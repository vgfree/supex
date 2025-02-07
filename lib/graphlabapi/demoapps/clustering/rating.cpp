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
 *  Any changes to the code must include this original license notice in full.
 */



#include "clustering.h"
#include "distance.h"
#include "../gabp/advanced_config.h"

extern advanced_config ac;
extern problem_setup ps;
extern const char * runmodesname[];

flt_dbl_vec wrap_answer(const vec& distances, const ivec& indices, int num);

void init_rating(){

   if (ac.distance_measure != EUCLIDEAN && ac.distance_measure != COSINE && ac.distance_measure != TANIMOTO)
     return;

   int start = 0;
   int end = ps.M;
  
   graph_type * training = ps.g<graph_type>(TRAINING);
   for (int i=start; i< end; i++){
      vertex_data & data = training->vertex_data(i);
      data.min_distance = sum_sqr(data.datapoint);
      assert( data.reported == ( nnz(data.datapoint) > 0));
   }

   int startv = 0;
   int endv = ps.M_validation;

   graph_type * validation = ps.g<graph_type>(VALIDATION);
   for (int i=startv; i< endv; i++){
      vertex_data & data = validation->vertex_data(i);
      data.min_distance = sum_sqr(data.datapoint);
   }

   logstream(LOG_INFO)<<"Training is: " << training << " validation is: " << validation << std::endl;
}


void rating_stats(){

   flt_dbl min=1e100, max=0, avg=0;
   int cnt = 0;
   graph_type * training = ps.g<graph_type>(TRAINING);
   int startv = 0;
   int endv = ps.M;

   for (int i=startv; i< endv; i++){
     vertex_data& data = training->vertex_data(i);
     if (data.distances.size() > 0){
       min = std::min(min, data.distances[0]);
       max = std::max(max, data.distances[0]);
       if (std::isnan(data.distances[0]))
          printf("bug: nan on %d\n", i);
       else {
         avg += data.distances[0];    
         cnt++;
       }
     }
   }

  printf("Distance statistics: min %g max %g avg %g\n", min, max, avg/cnt);
}

 /***
 * UPDATE FUNCTION
 */
void rating_update_function(gl_types::iscope &scope, 
			 gl_types::icallback &scheduler) {
    

  /* GET current vertex data */
  vertex_data& vdata = scope.vertex_data();

  int id = scope.vertex();
  bool toprint = ac.debug /*&& (id == 0 || (id == ps.M-1))*/; 
  
 /* print statistics */
  if (toprint){
    printf("Rating: entering user %u\n",  id);  
    print(vdata.datapoint); 
  }

  if (!vdata.reported) //this matrix row have no non-zero entries, and thus ignored
     return;

  graphlab::timer t; t.start();
  graph_type *validation = ps.g<graph_type>(VALIDATION);
  graph_type* train_ref = NULL;
  
  train_ref = ps.g<graph_type>(TEST);
  if (train_ref == NULL)
    logstream(LOG_FATAL)<<"Training graph was not supplied using the command line --training_ref" << std::endl;

  int start = 0;
  int end = ps.M_validation;
  int howmany = (end-start)*ac.knn_sample_percent;
  assert(howmany > 0 );
  vec distances(howmany);
  ivec indices = ivec(howmany);
  for (int i=0; i< howmany; i++){
    indices[i]= -2;
    distances[i] = 0;
  }
  vertex_data & thenode = train_ref->vertex_data(id);
  bool *curratings = new bool[ps.M_validation];
  FOR_ITERATOR_(j, thenode.datapoint){
  //no need to calculate this rating since it is given in the training data reference
    curratings[get_nz_index(thenode.datapoint, j)] = true;
  }
   if (ac.knn_sample_percent == 1.0){
     for (int i=start; i< end; i++){
        assert(id != ps.M + i);
        if (curratings[i])
          continue;
        vertex_data & other = validation->vertex_data(i);
        distances[i-start] = calc_distance(vdata.datapoint, other.datapoint, other.min_distance, vdata.min_distance);
        indices[i-start] = i;
     }
  }
  else for (int i=0; i<howmany; i++){
        int random_other = ::randi(start, end-1);
        vertex_data & other = validation->vertex_data(random_other);
        distances[i] = calc_distance(vdata.datapoint, other.datapoint, other.min_distance, vdata.min_distance);
        indices[i] = random_other;
  }
  delete [] curratings;
  vec out_dist(ps.K);
  ivec indices_sorted = reverse_sort_index2(distances, indices, out_dist, ps.K);
  vdata.distances = wrap_answer(out_dist, indices_sorted, ps.K);
  assert(vdata.distances.size() == ps.K*2);
  if (toprint)
    printf("Closest is: %d with distance %g\n", (int)vdata.distances[1], vdata.distances[0]);


  if (id % 100 == 0)
    printf("handling validation row %d at time: %g\n", id, ps.gt.current_time());
}

void copy_assignments(flt_dbl_mat &a, const flt_dbl_vec& distances, int i, graph_type* validation);

void copy_distances(flt_dbl_mat &a, const flt_dbl_vec& distances, int i, graph_type* validation);


void rating_prepare_output(){

  graph_type * training = ps.g<graph_type>(TRAINING);
  ps.output_assignements = zeros(ps.K, training->num_vertices());
  ps.output_clusters = zeros(ps.K, training->num_vertices());
  for (int i=0; i< (int)training->num_vertices(); i++){
     const vertex_data& data = training->vertex_data(i);
     copy_assignments(ps.output_assignements, data.distances, i, training);
     copy_distances(ps.output_clusters, data.distances, i, training); 
  }
  ps.output_clusters = transpose(ps.output_clusters);
  ps.output_assignements = transpose(ps.output_assignements);
}


 
void rating_main(){

    init_rating();
    ps.gt.start();
    ps.glcore->start();
    rating_stats();
    rating_prepare_output();
};


