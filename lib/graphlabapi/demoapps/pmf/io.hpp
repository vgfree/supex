/**  
 * Copyright (c) 2009 Carnegie Mellon University. 
 *     All rights reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing,
 *  software distributed under the License is distributed on an "AS
 *  IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 *  express or implied.  See the License for the specific language
 *  governing permissions and limitations under the License.
 *
 * For more about this software visit:
 *
 *      http://www.graphlab.ml.cmu.edu
 *  
 *  Code written by Danny Bickson, CMU
 */


#ifndef _IO_HPP
#define _IO_HPP

#include "stats.hpp"
#include "../../libs/matrixmarket/mmio.h" //matrix market format support
#include "implicit.hpp"
#include <graphlab/macros_def.hpp>

extern advanced_config config;
extern problem_setup ps;


const static  int matlab_offset_user_movie = 1; //matlab array start from 1
static  int matlab_offset_time = 1; //matlab arrays start from 1
bool * flags = NULL;

FILE * open_file(const char * name, const char * mode){
  FILE * f = fopen(name, mode);
  if (f == NULL){
      perror("fopen failed");
      logstream(LOG_ERROR) <<" Failed to open file" << name << std::endl;
      exit(1);
   }
  return f;
}


template<typename graph_type, typename vertex_data>
void add_time_nodes(graph_type* _g){
    //init times
    ps.times = new vertex_data[ps.K];
    vec tones = ones(ac.D)*(ps.K==1?1:0.1);
    //add T time node (ps.tensor dim 3)
    for (int i=0; i<ps.K; i++){
      ps.times[i].pvec =tones;
      _g->add_vertex(ps.times[i]);
      if (ac.debug && (i <= 5 || i == ps.K-1))
        debug_print_vec("T: ", ps.times[i].pvec, ac.D);
    }
}; //nothing to be done here 

template<>
void add_time_nodes<graph_type_svdpp,vertex_data_svdpp>(graph_type_svdpp* _g){
    assert(ps.K > 1);
    ps.times_svdpp = new vertex_data_svdpp[ps.K];
    //add T time node (ps.tensor dim 3)
    for (int i=0; i<ps.K; i++){
      _g->add_vertex(ps.times_svdpp[i]);
    }
}
/**
 * Add the graph nodes. We have nodes for each row (user), column (movies) and time bins.
 * 
 */
template<typename graph_type, typename vertex_data>
void add_vertices(graph_type * _g, testtype data_type){
  vertex_data vdata;
  // add M user nodes (ps.tensor dim 1)
  for (int i=0; i<ps.M; i++){
    vdata.pvec = ac.debug? (ones(ac.D)*0.1) : (randu(ac.D)*(0.1/sqrt(ac.D)));
    _g->add_vertex(vdata);
    if (ac.debug && (i<= 5 || i == ps.M-1))
      debug_print_vec("U: ", vdata.pvec, ac.D);
  }
  
  // add N movie node (ps.tensor dim 2) 
  for (int i=0; i<ps.N; i++){
    vdata.pvec = ac.debug? (ones(ac.D)*0.1) : (randu(ac.D)*(0.1/sqrt(ac.D)));
    _g->add_vertex(vdata);
    if (ac.debug && (i<=5 || i==ps.N-1))
      debug_print_vec("V: ", vdata.pvec, ac.D);
  }
  
  //add time nodes (if needed)
  if (data_type==TRAINING && ps.tensor){
    add_time_nodes<graph_type, vertex_data>(_g);
  }
}

template<typename graph_type, typename edge_data>
void verify_edges(graph_type * _g, testtype data_type){

  //verify edges
  for (int i=ps.M; i < ps.M+ps.N; i++){
    foreach(graphlab::edge_id_t eid, _g->in_edge_ids(i)){          
      int from = _g->source(eid);
      int to = _g->target(eid);
      assert(from < ps.M);
      assert(to >= ps.M && to < ps.M+ps.N);

      const edge_data & data = _g->edge_data(eid);
	if (!ac.zero)
          assert(data.weight != 0);  
        if (ps.algorithm != WEIGHTED_ALS)
          assert(data.time < ps.K);
  
        if (ps.K > 1 && data_type==TRAINING && ps.tensor)
          edges[(int)data.time].push_back(eid);
    }
  }
}
template<>
void verify_edges<graph_type_mult_edge,multiple_edges>(graph_type_mult_edge * _g, testtype data_type){

  //verify edges
  for (int i=ps.M; i < ps.M+ps.N; i++){
    foreach(graphlab::edge_id_t eid, _g->in_edge_ids(i)){          
     const  multiple_edges & tedges= _g->edge_data(eid);
      int from = _g->source(eid);
      int to = _g->target(eid);
      assert(from < ps.M);
      assert(to >= ps.M && to < ps.M+ps.N);

      for (int j=0; j< (int)tedges.medges.size(); j++){
        const edge_data_mcmc & data= tedges.medges[j];
	if (!ac.zero)
          assert(data.weight != 0);  
        if (ps.algorithm != WEIGHTED_ALS)
          assert(data.time < ps.K);
  
        if (ps.K > 1 && data_type==TRAINING && ps.tensor)
          edges[(int)data.time].push_back(eid);
        }
    }
  }
}



#include "read_matrix_market.hpp"
/**
 * fill data structures used for writing output to file
 */
template<typename graph_type, typename vertex_data>
void fill_factors_uvt(){

 if (ps.algorithm != LANCZOS && ps.algorithm != SVD){
   //clear the graph only at the end of the run
   if (ac.bptf_additional_output != true)
     ((graph_type*)ps.g<graph_type>(TRAINING))->reduce_mem_consumption();
   ps.U = zeros(ps.M,ac.D);
   ps.V = zeros(ps.N,ac.D);

   for (int i=0; i< ps.M+ps.N; i++){ 
      const vertex_data & data = ps.g<graph_type>(TRAINING)->vertex_data(i);
      if (i < ps.M){
        set_row(ps.U, i, data.pvec);
      }
      else {
        set_row(ps.V, i-ps.M, data.pvec);
      }
   }

   if (ps.tensor){ 
     ps.T = zeros(ps.K,ac.D);
     for (int i=0; i<ps.K; i++){
        set_row(ps.T, i, ps.times[i].pvec);
     }
    } 
  }
} 

/* prepare output for SVD++ algorithm */
template<>
void fill_factors_uvt<graph_type_svdpp,vertex_data_svdpp>(){

   if (ps.algorithm == SVD_PLUS_PLUS || ps.algorithm == BIAS_SGD){
   ps.U = zeros(ps.M,ac.D);
   ps.V = zeros(ps.N,ac.D);
   ps.svdpp_usr_bias = zeros(ps.M);
   ps.svdpp_movie_bias = zeros(ps.N);
      
   for (int i=0; i< ps.M+ps.N; i++){ 
      const vertex_data_svdpp & data = ps.g<graph_type_svdpp>(TRAINING)->vertex_data(i);
      if (i < ps.M){
         set_row(ps.U, i, data.pvec+ data.weight);
         ps.svdpp_usr_bias[i] = data.bias;
      }
      else {
        set_row(ps.V, i-ps.M, data.pvec);
        ps.svdpp_movie_bias[i-ps.M] = data.bias;
      }
   }
   }
   else if (ps.algorithm == TIME_SVD_PLUS_PLUS){
      ps.timesvdpp_out.ptemp = zeros(ps.M,ac.D);
      ps.timesvdpp_out.x = zeros(ps.M,ac.D);
      ps.timesvdpp_out.pu = zeros(ps.M,ac.D);
      ps.timesvdpp_out.q = zeros(ps.N,ac.D);
      ps.timesvdpp_out.z = zeros(ac.K,ac.D);
      ps.timesvdpp_out.pt = zeros(ac.K,ac.D);
      for (int i=0; i< ps.M; i++){
        const vertex_data_svdpp & data = ps.g<graph_type_svdpp>(TRAINING)->vertex_data(i);
        set_row(ps.timesvdpp_out.ptemp, i, tail(data.weight, ac.D));
        set_row(ps.timesvdpp_out.x, i, head(data.weight, ac.D));
        set_row(ps.timesvdpp_out.pu, i, tail(data.pvec, ac.D));
      }
      for (int i=ps.M; i < ps.M+ps.N; i++){
        const vertex_data_svdpp & data = ps.g<graph_type_svdpp>(TRAINING)->vertex_data(i);
        set_row(ps.timesvdpp_out.q, i-ps.M, data.pvec);
      }
      for (int i=0; i< ac.K; i++){
        const vertex_data_svdpp & data = ps.times_svdpp[i];
        set_row(ps.timesvdpp_out.z, i, data.pvec);
        set_row(ps.timesvdpp_out.pt, i, data.weight);
      }

   }
   else if (ps.algorithm == RBM){ //TODO

   } else assert(false);
} 


template<typename edgedata, typename graph_type, typename edge_data>
int read_mult_edges(FILE * f, int nodes, testtype type, graph_type * g, graph_type * _g, bool symmetry = false);
 //write an output vector to file
template <typename T>
void write_vec(FILE * f, const int len, const T * array){
  assert(f != NULL && array != NULL);
  int total = 0;
  
  while(true){
    int rc = fwrite(array+total, sizeof(T), len-total, f);
    if (rc <= 0){
      if (errno == EINTR){
         logstream(LOG_WARNING) << "Interrupted system call, trying agin " << std::endl;
         continue;
      }
      perror("write failed");
      logstream(LOG_FATAL) << "Failed to write vector!" << std::endl;
    }
    total += rc;
    if (total >= len)
      break;
  }
}

void truncate_and_scale(float & prediction){
  if (prediction<ac.minval)
     prediction=ac.minval;
  else if (prediction>ac.maxval)
     prediction=ac.maxval; 

  if (ac.scalerating != 1)
     prediction *= ac.scalerating;
}
	
template<typename graph_type, typename vertex_data, typename edge_data>
void common_prediction(const graph_type &g, const graph_type & _g, const vertex_data& data,int i, int &lineNum, double& sumPreds, vec& test_predictions, bool dosave){
  
  foreach(edge_id_t iedgeid, _g.out_edge_ids(i)) {
    const vertex_data & pdata = g.vertex_data(_g.target(iedgeid)); 
	  const edge_data & edge = _g.edge_data(iedgeid);
          
    if (!ac.zero)
			assert(edge.weight != 0);

		float prediction = 0;
    predict(data, 
			pdata, 
			ps.algorithm == WEIGHTED_ALS ? &edge : NULL, 
			ps.tensor? (&ps.times[(int)edge.time]):NULL, 
			edge.weight, 
			prediction);
    truncate_and_scale(prediction);      
    if (ac.debug && (i== 0 || i == ps.M))
			cout<<lineNum<<") prediction:"<<prediction<<endl; 
    if (dosave)
			test_predictions[lineNum] = prediction;
	  
    sumPreds += prediction;
 	  lineNum++; 
  }
}

//compute predictions for SVD++
void test_predict(vertex_data_svdpp & user, int i, int& lineNum, double & sumPreds, vec& test_predictions, bool dosave, const graph_type_svdpp &g, const graph_type_svdpp & _g){
  if (ps.algorithm == SVD_PLUS_PLUS){
		int n = user.num_edges; //+1.0 ? //regularization
		memset(&user.weight[0], 0, ac.D*sizeof(double));
		if (n > 0 ){
			foreach(edge_id_t oedgeid, g.out_edge_ids(i)) {
				vertex_data_svdpp & movie = (vertex_data_svdpp&)g.vertex_data(g.target(oedgeid)); 
				user.weight += movie.weight;
			}
			float usrnorm = float(1.0/sqrt(n));
			user.weight *= usrnorm;
		}
		else { //cold start, we did not encounter this user in training!
			memset(&user.pvec[0], 0, ac.D*sizeof(double));
			user.bias = 0;
		}
  }
  else if (ps.algorithm == BIAS_SGD){
		int n = user.num_edges; //+1.0 ? //regularization
    if (n == 0){
			memset(&user.pvec[0], 0, ac.D*sizeof(double));
			user.bias = 0;
    }	
  }
  else if (ps.algorithm == TIME_SVD_PLUS_PLUS){

  }
  else if (ps.algorithm == RBM){

  }
  else assert(false);
  common_prediction<graph_type_svdpp,vertex_data_svdpp,edge_data>(g, _g,user,i,lineNum, sumPreds, test_predictions, dosave);
}

//compute predictions for all others
void test_predict(vertex_data & data, int i, int& lineNum, double& sumPreds, vec& test_predictions, bool dosave, const graph_type &g, const graph_type& _g){
  common_prediction<graph_type,vertex_data,edge_data>(g,_g,data,i,lineNum, sumPreds, test_predictions, dosave);
}

//compute predictions for BPTF/PMF
void test_predict(vertex_data & data, int i, int&lineNum, double& sumPreds, vec& test_predictions, bool dosave, const graph_type_mcmc& g, const graph_type_mcmc &_g){
      foreach(edge_id_t iedgeid, _g.out_edge_ids(i)) {
        const vertex_data & pdata = g.vertex_data(_g.target(iedgeid)); 
	  edge_data_mcmc & edge = (edge_data_mcmc&)_g.edge_data(iedgeid);
          
          if (!ac.zero)
           	assert(edge.weight != 0);

          float prediction = 0;
          predict(data, 
                  pdata, 
                  NULL, 
                  ps.tensor? (&ps.times[(int)edge.time]):NULL, 
                  edge.weight, 
                  prediction);
          
          if (ps.BPTF && ps.iiter > ac.bptf_burn_in){
             edge.avgprd += prediction;
             //add = powf((edge.avgprd / (iiter - bptf_burn_in)) - edge.weight, 2);
              prediction = (edge.avgprd / (ps.iiter - ac.bptf_burn_in));
           }
          
	         truncate_and_scale(prediction);
          if (ac.debug && (i== 0 || i == ps.M))
            cout<<lineNum<<") prediction:"<<prediction<<endl; 
          if (dosave)
            test_predictions[lineNum] = prediction;
	        sumPreds += prediction;
 	        lineNum++; 
       }
}

//compute predictions of tensors with multiple edges between same pair of nodes
void test_predict(vertex_data & data, int i, int&lineNum, double & sumPreds, vec& test_predictions, bool dosave, const graph_type_mult_edge&g, const graph_type_mult_edge &_g){
      foreach(edge_id_t iedgeid, _g.out_edge_ids(i)) {
        const multiple_edges & edges = _g.edge_data(iedgeid);
        const vertex_data & pdata = g.vertex_data(_g.target(iedgeid)); 
        for (int j=0; j< (int)edges.medges.size(); j++){  
          edge_data_mcmc & edge = (edge_data_mcmc&)edges.medges[j];
          
          if (!ac.zero)
           	assert(edge.weight != 0);

          float prediction = 0;
          predict(data, 
                  pdata, 
                  NULL, 
                  ps.tensor? (&ps.times[(int)edge.time]):NULL, 
                  edge.weight, 
                  prediction);
          if (ps.BPTF && ps.iiter > ac.bptf_burn_in){
             edge.avgprd += prediction;
              prediction = (edge.avgprd / (ps.iiter - ac.bptf_burn_in));
           }
	        truncate_and_scale(prediction);
          if (ac.debug && (i== 0 || i == ps.M))
            cout<<lineNum<<") prediction:"<<prediction<<endl; 
          if (dosave)
           test_predictions[lineNum] = prediction;
	        sumPreds += prediction;
 	        lineNum++; 
         }
       }
 }


template<typename graph_type, typename vertex_data, typename edge_data>
void export_test_file(const graph_type & _g, testtype type, bool dosave) {
       
  const graph_type * g = ps.g<graph_type>(TRAINING);
  double sumPreds = 0;
  int lineNum = 0;
  if (!dosave)
    assert(ps.BPTF);	
  if (ps.Lt == 0 && type == TEST)
    logstream(LOG_FATAL) << "Empty or missing test data file, can not compute predictions!" << std::endl;
  if (ps.Lt2 == 0 && type == TEST2)
    logstream(LOG_FATAL) << "Empty or missing test data file, can not compute predictions!" << std::endl;

  int size = 0;
  string suffix = "";
  string comment = "";
  switch(type){
  
    case VALIDATION: 
       size = ps.Le; 
       suffix = ".validation.predictions"; 
       comment = "output predictions for validation data\n";
       break;
  
    case TEST: 
       size = ps.Lt; 
       suffix = ".test.predictions";
       comment = "output predictions for test data\n";
       break;

    case TEST2: 
       size = ps.Lt2; 
       suffix = ".test2.predictions"; 
       comment = "output predictions for test2 data\n";
       break;

    case TRAINING:
    default:
      assert(false);
  }
  vec out_predictions = zeros(size);
  for (int i=0; i< ps.M; i++){ 
      vertex_data & data = (vertex_data&)g->vertex_data(i);
      test_predict(data, i, lineNum, sumPreds, out_predictions, dosave, *g, _g);
  }

  ASSERT_EQ(lineNum, size);
  logstream(LOG_INFO)<< "**Completed successfully (mean prediction: " << (sumPreds/lineNum)-ac.shiftrating << std::endl;
  if (dosave)
    save_matrix_market_vector((ac.datafile + suffix).c_str(), 
     out_predictions, comment, false, false);
}



//OUTPUT: SAVE FACTORS U,V,T to a binary file

// FORMAT:  M N K D (4 x ints)
// MATRIX U ( M x D doubles)
// MATRIX V ( N x D doubles)
// MATRIX K ( K x D doubles - optional, only for ps.tensor)
// TOTAL FILE SIZE: 4 ints + (M+N+K)*D - for ps.tensor
//                  4 ints + (M+N)*D - for matrix
template<typename graph_type, typename vertex_data>
void export_uvt_to_binary_file(){

  char dfile[256] = {0};
  sprintf(dfile,"%s-%d-%d.out",ac.datafile.c_str(),ac.D,ps.iiter);
  FILE * f = open_file(dfile, "w");

  logstream(LOG_INFO)<<"Saving output in binary format to: " << dfile << std::endl;

  int rc = fwrite(&ps.M, 1, 4, f);
  assert(rc == 4);
  rc = fwrite(&ps.N, 1, 4, f);
  assert(rc == 4);
  rc = fwrite(&ps.K, 1, 4, f);
  assert(rc == 4);
  rc = fwrite(&ac.D, 1, 4, f);
  assert(rc == 4);

  for (int i=0; i< ps.M+ps.N; i++){ 
      const vertex_data & vdata = ps.g<graph_type>(TRAINING)->vertex_data(i);
      if (i < ps.M){
        write_vec(f, vdata.pvec.size(), data(vdata.pvec));
      }
      else {
        write_vec(f, vdata.pvec.size(), data(vdata.pvec));
      }
   }

   if (ps.tensor){ 
     for (int i=0; i<ps.K; i++){
        write_vec(f, ps.times[i].pvec.size(), data(ps.times[i].pvec));
     }
    } 

  fclose(f); 

}

//OUTPUT: SAVE FACTORS U,V,T to a binary file

// FORMAT:  M N K D (4 x ints)
// MATRIX U ( M x D doubles)
// MATRIX V ( N x D doubles)
// usr_bias ( M x 1 doubles)
// movie bias (N x 1 doubles)
// global mean (1 double)
// TOTAL FILE SIZE: 4 ints + (M+N)*D + M + N + 1
template<>
void export_uvt_to_binary_file<graph_type_svdpp,vertex_data_svdpp>(){

  if (ps.algorithm == TIME_SVD_PLUS_PLUS){
    logstream(LOG_FATAL) <<"time-svd++ does not support binary output format" << std::endl;
  }

  char dfile[256] = {0};
  sprintf(dfile,"%s-%d-%d.out",ac.datafile.c_str(),ac.D,ps.iiter);
  FILE * f = open_file(dfile, "w");

  logstream(LOG_INFO)<<"Saving output in binary format to: " << dfile << std::endl;

  int rc = fwrite(&ps.M, 1, 4, f);
  assert(rc == 4);
  rc = fwrite(&ps.N, 1, 4, f);
  assert(rc == 4);
  rc = fwrite(&ps.K, 1, 4, f);
  assert(rc == 4);
  rc = fwrite(&ac.D, 1, 4, f);
  assert(rc == 4);

  ps.svdpp_usr_bias = zeros(ps.M);
  ps.svdpp_movie_bias = zeros(ps.N);
 
  for (int i=0; i< ps.M+ps.N; i++){ 
      const vertex_data_svdpp & vdata = ps.g<graph_type_svdpp>(TRAINING)->vertex_data(i);
      if (i < ps.M){
        write_vec(f, vdata.pvec.size(), data(vdata.pvec));
        ps.svdpp_usr_bias[i] = vdata.bias;
      }
      else {
        write_vec(f, vdata.pvec.size(), data(vdata.pvec));
        ps.svdpp_movie_bias[i-ps.M] = vdata.bias;
      }
   }

  write_vec(f, ps.svdpp_usr_bias.size(), data(ps.svdpp_usr_bias));
  write_vec(f, ps.svdpp_movie_bias.size(), data(ps.svdpp_movie_bias));
  write_vec(f, 1, ps.globalMean); 
  fclose(f); 
}



//OUTPUT: SAVE FACTORS U,V,T TO IT++ FILE
template<typename graph_type, typename vertex_data>
void export_uvt_to_itpp_file(){

  if (ps.algorithm != LANCZOS && ps.algorithm != SVD) 
     fill_factors_uvt<graph_type, vertex_data>();
  char dfile[256] = {0};
  sprintf(dfile,"%s-%d-%d.out",ac.datafile.c_str(), ac.D, ps.iiter);
  remove(dfile);
  it_file output(dfile);
  /* for all other algos */
  if (ps.algorithm != SVD){
  output << Name("User");
  output << ps.U;
  output << Name("Movie");
  output << ps.V;
  if (ps.tensor){
    output << Name("Time");
    output << ps.T;
  }
  }
  else{ /* for SVD */
    output << Name("U"); /* for conforming to wikipeida conversion i swap u and v*/
    output << ps.V;
    output << Name("V");
    output << ps.U;
    output << Name("EigenValues_AAT");
    output << get_col(ps.T,0);
    output << Name("EigenValues_ATA");
    output << get_col(ps.T, 1);
  }
  
  output.close();
  logstream(LOG_INFO) << "Saved output to file: " << dfile << " You can read it using the script itload.m available from http://graphloab.org/pmf.html " << std::endl;

}
//OUTPUT: SAVE FACTORS U,V,T TO IT++ FILE
template<>
void export_uvt_to_itpp_file<graph_type_svdpp,vertex_data_svdpp>(){

  fill_factors_uvt<graph_type_svdpp, vertex_data_svdpp>();

  char dfile[256] = {0};
  sprintf(dfile,"%s-%d-%d.out",ac.datafile.c_str(), ac.D, ps.iiter);
  remove(dfile);
  it_file output(dfile);

  if (ps.algorithm == SVD_PLUS_PLUS){
    output << Name("User");
    output << ps.U;
    output << Name("Movie");
    output << ps.V;
 }
  else if (ps.algorithm == TIME_SVD_PLUS_PLUS){
    output << Name("User_ptemp");
    output << ps.timesvdpp_out.ptemp;
    output << Name("User_x");
    output << ps.timesvdpp_out.x;
    output << Name("User_pu");
    output << ps.timesvdpp_out.pu;
    output << Name("Movie_q");
    output << ps.timesvdpp_out.q;
    output << Name("Time_z");
    output << ps.timesvdpp_out.z;
    output << Name("Time_pt");
    output << ps.timesvdpp_out.pt;
  }
  else assert(false);

  output << Name("UserBias");
  output << ps.svdpp_usr_bias;
  output << Name("MovieBias");
  output << ps.svdpp_movie_bias;
  output << Name("GlobalMean");
  output << ps.globalMean[0];
 
  output.close();
}


template<typename graph_type, typename vertex_data>
void export_uvt_to_matrixmarket(){
  if (ps.algorithm != LANCZOS && ps.algorithm != SVD)
     fill_factors_uvt<graph_type, vertex_data>();
  char dfile[256] = {0};
  sprintf(dfile,"%s-%d-%d.out",ac.datafile.c_str(), ac.D,ps.iiter);
  if (ps.tensor)
    logstream(LOG_WARNING)<<" matrix market IO does not support tensor mode" << std::endl;
  remove(dfile);
  save_matrix_market_format(dfile, ps.U, ps.V);  
 
}

//LOAD FACTORS FROM FILE
template<typename graph_type>
void import_uvt_from_file(){
 
 const graph_type * g =  ps.g<graph_type>(TRAINING);
 mat U,V,T;
 if (!ac.matrixmarket){
 char dfile[256] = {0};
 sprintf(dfile,"%s%d.out",ac.datafile.c_str(), ac.D);
 printf("Loading factors U,V,T from file\n");
 it_file input(dfile);
 input >> Name("User");
 input >> U;
 input >> Name("Movie");
 input >> V;
  if (ps.tensor){
    input >> Name("Time");
    input >> T;
 }
 input.close();
 }
 else {
   load_matrix_market_matrix(ac.datafile + ".U", U);
   load_matrix_market_matrix(ac.datafile + ".V", V);
   if (ps.tensor)
     load_matrix_market_matrix(ac.datafile + ".T", T); 
   ASSERT_EQ(U.rows(), ps.M);
   ASSERT_EQ(V.rows(), ps.N);
   ASSERT_EQ(U.cols(), V.cols());
   ASSERT_EQ(U.cols(), ac.D);
 }
 

 //initalizing feature vectors from file
//#pragma omp parallel for
 for (int i=0; i< ps.M+ps.N; i++){ 
    vertex_data & data = (vertex_data&)g->vertex_data(i);
    if (i < ps.M)
        data.pvec = get_row(U, i); 
   else
        data.pvec = get_row(V, i-ps.M);
 }

 if (ps.tensor){ 
    for (int i=0; i<ps.K; i++){
        ps.times[i].pvec = get_row(T, i);
    }
 } 
}


void set_num_edges(int val, testtype data_type){
  switch(data_type){
    case TRAINING: 
      ps.L = val; break;
    
    case VALIDATION: 
      ps.Le = val; 
      if (ac.aggregatevalidation)
	ps.L+=  ps.Le; //add edges of validation dataset into the training data set as well.
	break; 
     
    case TEST: 
      ps.Lt = val; break;

    case TEST2:
      ps.Lt2 = val; break;
  }  
}

/**
 * Verify that matrix size is consistent between training, validation and test files
 */
void verify_size(testtype data_type, int _M, int _N, int _K){
 if (data_type != TRAINING && ps.M != _M)
	logstream(LOG_WARNING) << " wrong number of users: " << _M << " instead of " << ps.M << " in " << testtypename[data_type] << std::endl;
  if (data_type != TRAINING && ps.N != _N)
	logstream(LOG_WARNING) << " wrong number of movies: " << _N << " instead of " << ps.N << " in " << testtypename[data_type] << std::endl;
  if (data_type != TRAINING && ps.K != _K && ac.K == 0)
	logstream(LOG_WARNING) << " wrong number of time bins: " << _K << " instead of " << ps.K << " in " << testtypename[data_type] <<std::endl;

  printf("Matrix size is: USERS %d MOVIES %d TIME BINS %d\n", ps.M, ps.N, ps.K);
}




/* function that reads the ps.tensor from file */
/* Input format is:
 * M - number of users (int)
 * N - number of movies (int)
 * K - number of time bins (int), in case of weighted ALS this value is ignored
 * e - number of edges (int)
 * A list of edges in the format
 * [from] [to] [ time] [weight]  (4 floats)
 * where [from] is an integeter from 1 to M
 * [to] is an interger from 1 to N
 * [time] is an integer from 1 to K (for weighted ALS this is the weight, which is float)
 * [weight] - this is the rating, which is float. Rating is assumed non-zero unless the --zero=true flas is on 
 */
template<typename graph_type, typename gl_types, typename vertex_data, typename edge_data>
void load_pmf_graph(const char* filename, graph_type * g, graph_type * _g, testtype data_type) {


  if (ac.matrixmarket){
      printf("Loading Matrix Market file %s %s\n", filename, testtypename[data_type]);
      load_matrix_market<graph_type, vertex_data, edge_data>(filename, _g, data_type);
      return;
  }

  printf("Loading %s %s\n", filename, testtypename[data_type]);
  FILE * f = fopen(filename, "r");
  if (data_type!=TRAINING && f == NULL){//skip optional files, if the file is missing
    printf("skipping file\n");
    return;
  }

  if(data_type==TRAINING && f== NULL){
        perror("failed fopen() with error: ");
	logstream(LOG_ERROR) << " can not load input file: " << filename << " . aborting " << std::endl;
	exit(1);
  }

  int _M,_N,_K;
  int rc = fread(&_M,1,4,f);//movies
  assert(rc==4); 
  rc=fread(&_N,1,4,f);//users/
  assert(rc==4); 
  rc=fread(&_K,1,4,f);//time
  assert(rc==4); 
  assert(_K>=1);
  assert(_M>=1 && _N>=1); 

 if (_M == 1632445733){
     logstream(LOG_FATAL)<<"Detected matrix market input file. Please rerun using --matrixmarket=true flag" << std::endl;
   }


  if (data_type == TRAINING){
  	ps.M=_M; ps.N= _N; ps.K= _K;
        ps.last_node = ps.M+ps.N;

	if (ac.datafile == "kddcup" || ac.datafile == "kddcup2")// DB: ugly - kdd cup data has more time bins for test data than in training data. can fix this buy setting the time bins in training data to 6649.
		ps.K=6649;
     ps.K=ceil((ps.K-ac.truncating)/ac.scaling);
  }
  verify_size(data_type, _M,_N,_K);
  add_vertices<graph_type, vertex_data>(_g, data_type);
 
  // read tensor non zero edges from file
  int val = 0; 
  if (!ac.FLOAT) 
     val = read_mult_edges<edge_double, graph_type, edge_data>(f, ps.M+ps.N, data_type, g, _g);
  else 
     val = read_mult_edges<edge_float, graph_type, edge_data>(f,ps.M+ps.N, data_type, g, _g);

  if (data_type==TRAINING && ps.tensor && ps.K>1) 
    edges = new std::vector<edge_id_t>[ps.K]();

  set_num_edges(val, data_type);
  logstream(LOG_INFO)<<"Loaded total ratings: " << val <<std::endl;
  verify_edges<graph_type, edge_data>(_g, data_type);

  fclose(f);
  
  //add implicit edges if requested
  if (data_type == TRAINING && ac.implicitratingtype != "none")
     add_implicit_edges<graph_type, edge_data>(_g);

 //store number of edges for each node 
  if (data_type == TRAINING || (ac.aggregatevalidation && data_type == VALIDATION)){
    count_all_edges<graph_type>(g);
  }
 
  //verify correct number of edges encourntered
  if (data_type==TRAINING && ps.tensor && ps.K>1){
    int cnt = 0;
    for (int i=0; i<ps.K; i++){
      cnt+= edges[i].size();
    }
    assert(cnt == ps.L);
  }

}

template<typename edgedata, typename edge_data>
void verify_edge(edgedata & ed, edge_data& edge, int i, testtype type){
    if (!ac.zero) //usually we do not allow zero ratings, unless --zero=true flag is set.
	 assert(ed.weight != 0); 
      //verify node ids are in allowed range
      if (i == 0 && ((int)ed.from < matlab_offset_user_movie || (int)ed.from > ps.last_node))
          logstream(LOG_FATAL) << " Wrong intput file format. Did you try to use --float=true " << endl;
      if ((int)ed.from < matlab_offset_user_movie || (int)ed.from > ps.last_node)
          logstream(LOG_FATAL) << " Edge from node " << ed.from << " where the allowed node range is [" <<
         matlab_offset_user_movie << "-" << ps.last_node << "]. In input line " << i << endl;
      if ((int)ed.to < matlab_offset_user_movie || (int)ed.to > ps.last_node)
          logstream(LOG_FATAL) << " Edge to node " << ed.to << " where the allowed node range is [" <<
         matlab_offset_user_movie << "-" << ps.last_node << "]. In input line " << i << endl;
       //no self edges
      if ((int)ed.to == (int)ed.from)
        logstream(LOG_FATAL) << " Self edge between node " << ed.to << " to itself is not allowed. In input line " << i << endl;

      edge.weight = (double)ed.weight;
    
      //if sacling of rating values is requested to it here.
      if (ac.scalerating != 1.0)
	     edge.weight /= ac.scalerating;
      ps.globalMean[type] += edge.weight;
     
      //if scaling of time bins request do it here
      double time  = ((ed.time - matlab_offset_time - ac.truncating)/(double)ac.scaling);
      edge.time = time;
}



template<typename edgedata, typename graph_type>
void add_edge(int i, edgedata &ed, graph_type *g, graph_type *_g, multiple_edges edges, testtype type){ 
      
      edge_data_mcmc edge;
     
      verify_edge<edgedata,edge_data_mcmc>(ed, edge, i, type); 
       std::pair<bool, edge_id_t> ret;
      if (flags[(int)ed.from-matlab_offset_user_movie] == true && flags[(int)ed.to-matlab_offset_user_movie] == true){
        ret = _g->find((int)ed.from-matlab_offset_user_movie, (int)ed.to-matlab_offset_user_movie);
      }
      else ret.first = false;

      if (ret.first == false){
        edges.medges.push_back(edge); 
        _g->add_edge((int)ed.from-matlab_offset_user_movie, (int)ed.to-matlab_offset_user_movie, edges); // Matlab export has ids starting from 1, ours start from 0
        if (type == VALIDATION && ac.aggregatevalidation)//add validation edges into training dataset as well
           g->add_edge((int)ed.from-matlab_offset_user_movie, (int)ed.to-matlab_offset_user_movie, edges); // Matlab export has ids starting from 1, ours start from 0
          flags[(int)ed.from-matlab_offset_user_movie] = true;
          flags[(int)ed.to-matlab_offset_user_movie] = true;
      }
      else {
        _g->edge_data(ret.second).medges.push_back(edge);
      }
 }





template<typename edgedata, typename graph_type>
void add_edge(int i, edgedata &ed, graph_type *g, graph_type *_g, edge_data edge, testtype type){ 
  verify_edge<edgedata, edge_data>(ed, edge, i,type);
  _g->add_edge((int)ed.from-matlab_offset_user_movie, (int)ed.to-matlab_offset_user_movie, edge);
  if (type == VALIDATION && ac.aggregatevalidation)//add validation edges into training dataset as well
  g->add_edge((int)ed.from-matlab_offset_user_movie, (int)ed.to-matlab_offset_user_movie, edge);
}

template<typename edgedata, typename graph_type>
void add_edge(int i, edgedata &ed, graph_type *g, graph_type *_g, edge_data_mcmc edge, testtype type){ 
  verify_edge<edgedata, edge_data_mcmc>(ed, edge, i,type);
  _g->add_edge((int)ed.from-matlab_offset_user_movie, (int)ed.to-matlab_offset_user_movie, edge);
  if (type == VALIDATION && ac.aggregatevalidation)//add validation edges into training dataset as well
  g->add_edge((int)ed.from-matlab_offset_user_movie, (int)ed.to-matlab_offset_user_movie, edge);
}



/**
 * read edges from file, with support with multiple edges between the same pair of nodes (in different times)
 */
template<typename edgedata, typename graph_type, typename edge_data>
int read_mult_edges(FILE * f, int nodes, testtype type, graph_type *g, graph_type * _g, bool symmetry = false){
     
  //typedef typename graph::edge_data_type edge_data;
  if (ps.algorithm == BPTF_TENSOR_MULT || ps.algorithm == ALS_TENSOR_MULT){
    flags = new bool[nodes];
    memset(flags, 0, sizeof(bool)*nodes);
  }

 if (ps.algorithm == WEIGHTED_ALS)
    matlab_offset_time = 0; //for weighted ALS there are no time bins which are integers, so no need to convert them

  unsigned int e;
  int rc = fread(&e,1,4,f);
  assert(rc == 4);
  printf("Creating %d edges (observed ratings)...\n", e);
  assert(e>0);
  int total = 0;
  edgedata* ed = new edgedata[200000];
  int edgecount_in_file = e;
  while(true){
    //memset(ed, 0, 200000*sizeof(edge_float));
    rc = (int)fread(ed, sizeof(edgedata), std::min(200000, edgecount_in_file - total), f);
    total += rc;

    //go over each rating (edges)
    for (int i=0; i<rc; i++){
      edge_data edge;
      add_edge<edgedata, graph_type>(total - rc + i, ed[i], g, _g, edge, type);
    } 
   printf(".");
    fflush(0);
    if (rc == 0 || total >= edgecount_in_file)
      break;
  }
  if (total != (int)e){
      logstream(LOG_ERROR) << "Missing edges in " << testtypename[type] << "file. Should be " << e << edges << " but in file we counted only " << total << " edges. Please check your conversion script and verify the file is not truncated and edges are not missing. " << endl;
  }
  assert(total == (int)e);
  ps.globalMean[type] /= e;
  delete [] ed; ed = NULL;
  if (flags != NULL)
    delete[] flags;
  return e;
}


template<typename graph_type, typename vertex_data>
void write_output(){
  //write output matrices U,V,T to file
  if (ac.binaryoutput)
     export_uvt_to_binary_file<graph_type, vertex_data>();
  else if (ac.matrixmarket)
     export_uvt_to_matrixmarket<graph_type, vertex_data>();
  else // it++ output
   export_uvt_to_itpp_file<graph_type, vertex_data>();

}
#include <graphlab/macros_undef.hpp>
#endif
