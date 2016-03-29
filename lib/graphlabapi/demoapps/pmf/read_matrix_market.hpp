/* 
*   Matrix Market I/O example program
*
*   Read a real (non-complex) sparse matrix from a Matrix Market (v. 2.0) file.
*   and copies it to stdout.  This porgram does nothing useful, but
*   illustrates common usage of the Matrix Matrix I/O routines.
*   (See http://math.nist.gov/MatrixMarket for details.)
*
*   Usage:  a.out [filename] > output
*
*       
*   NOTES:
*
*   1) Matrix Market files are always 1-based, i.e. the index of the first
*      element of a matrix is (1,1), not (0,0) as in C.  ADJUST THESE
*      OFFSETS ACCORDINGLY offsets accordingly when reading and writing 
*      to files.
*
*   2) ANSI C requires one to use the "l" format modifier when reading
*      double precision floating point numbers in scanf() and
*      its variants.  For example, use "%lf", "%lg", or "%le"
*      when reading doubles, otherwise errors will occur.
*/
#ifndef READ_MARTIRX_MARKET
#define READ_MARTIRX_MARKET

#include <stdio.h>
#include <stdlib.h>
#include "pmf.h"
#include "../../libs/matrixmarket/mmio.h"
#include "../gabp/advanced_config.h"
#include <assert.h>

extern advanced_config ac;
extern problem_setup ps;
extern const char * testtypename[];
FILE * open_file(const char * name, const char * mode);

template<typename graph_type, typename vertex_data, typename edge_data>
void load_matrix_market(const char * filename, graph_type *_g, testtype data_type)
{
    int ret_code;
    MM_typecode matcode;
    int M, N, nz;   
    int i;

    printf("Loading %s %s\n", filename, testtypename[data_type]);
    FILE * f = fopen(filename, "r");
    if (data_type!=TRAINING && f == NULL){//skip optional files, if the file is missing
      printf("skipping file\n");
      return;
    }

    if(data_type==TRAINING && f== NULL)
	    logstream(LOG_FATAL) << " can not find input file. aborting " << std::endl;

    if (mm_read_banner(f, &matcode) != 0)
        logstream(LOG_FATAL) << "Could not process Matrix Market banner." << std::endl;

    /*  This is how one can screen matrix types if their application */
    /*  only supports a subset of the Matrix Market data types.      */
    if (mm_is_complex(matcode) && mm_is_matrix(matcode) && 
            mm_is_sparse(matcode) )
        logstream(LOG_FATAL) << "sOrry, this application does not support " << std::endl << 
          "Market Market type: " << mm_typecode_to_str(matcode) << std::endl;

    if (mm_is_array(matcode))
       logstream(LOG_FATAL) << "Only sparse matrix format is supported. It seems your input file has dense formati (array format)" << std::endl;

    if (mm_is_symmetric(matcode))
       logstream(LOG_FATAL) << "Symmetic matrix market matrices are not supported in pmf. " << std::endl;

    /* find out size of sparse matrix .... */
    if ((ret_code = mm_read_mtx_crd_size(f, &M, &N, &nz)) !=0)
       logstream(LOG_FATAL) << "failed to read matrix market cardinality size " << std::endl; 
   
   
    if (ac.K == 0) //no input from users about number of time bins, setting time bins to 1 (no tensor)
      ps.K = 1;  
    ps.M = M; ps.N = N; 
    ps.last_node = M+N;
    verify_size(data_type, M, N, 1);
    add_vertices<graph_type, vertex_data>(_g, data_type); 

    /* reseve memory for matrices */

    /* NOTE: when reading in doubles, ANSI C requires the use of the "l"  */
    /*   specifier as in "%lg", "%lf", "%le", otherwise errors will occur */
    /*  (ANSI C X3.159-1989, Sec. 4.9.6.2, p. 136 lines 13-15)            */

    int I,J; 
    double val;
    double dtime;

    edge_data edge;
    for (i=0; i<nz; i++)
    {
       if (ac.matrixmarkettokensperrow == 3){
        int rec = fscanf(f, "%d %d %lg\n", &I, &J, &val);
        if (rec != 3)
           logstream(LOG_FATAL)<<"Error reading input line " << i << std::endl;
      }
      else if (ac.matrixmarkettokensperrow == 4){
        int rec = fscanf(f, "%d %d %lg %lg\n", &I, &J, &val, &dtime);
        if (rec != 4)
           logstream(LOG_FATAL)<<"Error reading input line " << i << " returned tokens: " << rec << " I: " <<I<< "J:" <<J<<" val: " <<val << " dtime: " <<dtime <<std::endl;

      }
        if (I<=0 || J<= 0){
          logstream(LOG_FATAL) << "Matrix market values should be >= 1, observed values: " << I << " " << J << " In item number " << nz << std::endl;
        }
        I--;  /* adjust from 1-based to 0-based */
        J--;
        val /= ac.scalerating;
        val += ac.shiftrating;
        edge.weight = val;
        if (!ac.zero && val == 0)
	   logstream(LOG_FATAL)<<"Error in data line: " << i << " zero value is not allowed. Use --zero=true to allow zero value" << std::endl;
        if (I < 0 || I >=M)
           logstream(LOG_FATAL)<<"Error in data line: " << i << " 1st column value is: " << I+1 << " where it should be in the range 1 to " << M << std::endl;
        if (J < 0 || J >=N)
           logstream(LOG_FATAL)<<"Error in data line: " << i << " 2nd column value is: " << J+1 << " where it should be in the range 1 to " << N << std::endl;
        _g->add_edge(I,J+ps.M,edge);
        if (data_type == VALIDATION && ac.aggregatevalidation)
          ((graph_type*)ps.g<graph_type>(TRAINING))->add_edge(I,J+ps.M,edge);
    }

    if (data_type==TRAINING && ps.tensor && ps.K>1) 
    edges = new std::vector<edge_id_t>[ps.K]();

 
    set_num_edges(nz, data_type);
    logstream(LOG_INFO)<<"Loaded total edges: " << nz << std::endl;
    verify_edges<graph_type,edge_data>(_g, data_type);
 
    //add implicit edges if requested
    if (data_type == TRAINING && ac.implicitratingtype != "none")
       add_implicit_edges<graph_type, edge_data>(_g);

 
    if (data_type == TRAINING)
      count_all_edges(_g);
    else if (ac.aggregatevalidation && data_type == VALIDATION){
      count_all_edges<graph_type>((graph_type*)ps.g<graph_type>(TRAINING));
    }

    fclose(f);

}
template<>
 void load_matrix_market<graph_type_mult_edge, vertex_data, multiple_edges>(const char * filename, graph_type_mult_edge *_g, testtype data_type)
{
  assert(false);
}


void load_matrix_market_matrix(const std::string & filename, mat & a){
    MM_typecode matcode;                        
    int i,I,J;
    double val;
    int rows, cols, nnz;
    FILE * f = open_file(filename.c_str() ,"r");
    int rc = mm_read_banner(f, &matcode); 
    if (rc != 0)
      logstream(LOG_FATAL)<<"Failed to load matrix market banner in file: " << filename << std::endl;

    if (mm_is_sparse(matcode)){
       int rc = mm_read_mtx_crd_size(f, &rows, &cols, &nnz);
       if (rc != 0)
          logstream(LOG_FATAL)<<"Failed to load matrix market banner in file: " << filename << std::endl;
    }
    else { //dense matrix
      rc = mm_read_mtx_array_size(f, &rows, &cols);
      if (rc != 0)
        logstream(LOG_FATAL)<<"Failed to load matrix market banner in file: " << filename << std::endl;
      nnz = rows * cols;
    }

    a = zeros(rows, cols);
    for (i=0; i<nnz; i++){
     if (mm_is_sparse(matcode) && ac.matrixmarkettokensperrow == 3){
        rc = fscanf(f, "%d %d %lg\n", &I, &J, &val);
        if (rc != 3)
           logstream(LOG_FATAL)<<"Error reading input line " << i << std::endl;
        I--; J--;
        assert(I >= 0 && I < rows);
        assert(J >= 0 && J < cols);
        set_val(a, I, J, val);
     }
     else {
        rc = fscanf(f, "%lg", &val);
        if (rc != 1)
           logstream(LOG_FATAL)<<"Error reading nnz " << i << std::endl;
        I = i / ac.D;
        J = i % cols;
        set_val(a, I, J, val);
     }
     }
     ASSERT_EQ(i, nnz);
     logstream(LOG_INFO) << "Loaded matrix of size " << rows << " x " << cols << " from file: " << filename << " total of " << nnz << " entries. " << std::endl;

}


void save_matrix_market_matrix(const char * filename, const mat & a, std::string comment, bool integer, bool issparse){
    MM_typecode matcode;                        
    int i,j;

    mm_initialize_typecode(&matcode);
    mm_set_matrix(&matcode);
    mm_set_coordinate(&matcode);
    if (issparse)
      mm_set_sparse(&matcode);
    else mm_set_dense(&matcode);  

    if (!integer)
       mm_set_real(&matcode);
    else
       mm_set_integer(&matcode);

    FILE * f = open_file(filename,"w");
    if(f == NULL)
       logstream(LOG_FATAL)<<"Failed to open file: " << filename << " for writing." << std::endl;

    mm_write_banner(f, matcode); 
    if (comment.size() > 0)
      fprintf(f, "%s%s", "%", comment.c_str());

    if (issparse)
    mm_write_mtx_crd_size(f, a.rows(), a.cols(), a.size());
    else mm_write_mtx_array_size(f, a.rows(), a.cols());

    for (i=0; i<a.rows(); i++){
       for (j=0; j<a.cols(); j++){
          if (issparse){
            if (get_val(a,i,j) != 0){
               if (integer)
                 fprintf(f, "%d %d %d\n", i+1, j+1, (int)get_val(a,i,j));
               else  fprintf(f, "%d %d %10.13g\n", i+1, j+1, (double)get_val(a,i,j)); 
            }
          } else { //dense
             if (integer)
                fprintf(f, "%d ", (int)get_val(a,i,j));
             else fprintf(f, "%10.13g ", (double)get_val(a,i,j));
	     if (j == a.cols() -1 )
                fprintf(f, "\n");
          }
       }
    }
    logstream(LOG_INFO) << "Saved output matrix to file: " << filename << std::endl;
    logstream(LOG_INFO) << "You can read it with Matlab/Octave using the script mmread.m found on http://graphlab.org/mmread.m" << std::endl;

}



void save_matrix_market_vector(const char * filename, const vec & a, std::string comment, bool integer,bool issparse){
    MM_typecode matcode;                        
    int i;

    mm_initialize_typecode(&matcode);
    mm_set_matrix(&matcode);
    if (issparse){
      mm_set_sparse(&matcode);
      mm_set_coordinate(&matcode);
    }
    else {
      mm_set_dense(&matcode);
      mm_set_array(&matcode);
    }
    if (!integer)
      mm_set_real(&matcode);
    else
      mm_set_integer(&matcode);

    FILE * f = open_file(filename,"w");
    mm_write_banner(f, matcode); 
    if (comment.size() > 0)
      fprintf(f, "%s%s", "%", comment.c_str());
    
    if (issparse)
      mm_write_mtx_crd_size(f, a.size(), 1, a.size());
    else 
      mm_write_mtx_array_size(f, a.size(), 1);

    for (i=0; i<a.size(); i++){
      if (issparse){
        if (integer)
          fprintf(f, "%d %d %d\n", i+1, 1, (int)a[i]);
        else fprintf(f, "%d %d %10.13g\n", i+1, 1, a[i]);
      }
      else {//dense
        if (integer)
          fprintf(f,"%d ", (int)a[i]);
        else fprintf(f, "%10.13g\n", a[i]);
      }
    }

    fclose(f);
    logstream(LOG_INFO) << "Saved output vector to file: " << filename << " vector size: " << a.size() << std::endl;
    logstream(LOG_INFO) << "You can read it with Matlab/Octave using the script mmread.m found on http://graphlab.org/mmread.m" << std::endl;
}


void save_matrix_market_format(const char * filename, mat &U, mat& V)
{
    if (ps.algorithm != SVD && ps.algorithm != SVD_PLUS_PLUS && ps.algorithm != TIME_SVD_PLUS_PLUS){
      save_matrix_market_matrix((std::string(filename) + ".V").c_str(),V, "%%GraphLab Collaborative filtering library. This file holds the matrix V. Row i holds the feature vector for movie i. You can compute prediction in matlab for user i movie j using U(i,:)*V(j,:)'\n", false, false);
      save_matrix_market_matrix((std::string(filename) + ".U").c_str(),U, "%%GraphLab Collaborative filtering library. This file holds the matrix U. Row i holds the feature vector for user i. You can compute prediction in matlab for user i movie j using U(i,:)*V(j,:)'\n", false, false);
      return;
    }
    
    if (ps.algorithm == SVD){
      save_matrix_market_matrix((std::string(filename) + ".V").c_str(),U, "%%GraphLab collaborative filtering library. This file holds the matrix V which is the output of SVD\n", false, false); /* for conforming to wikipedia convention, I swap U and V*/
      save_matrix_market_matrix((std::string(filename) + ".U").c_str(),V, "%%GraphLab collaborative filtering library. This file holds the matrix U which is the output of SVD\n", false, false);
      save_matrix_market_vector((std::string(filename) + ".EigenValues_AAT").c_str(),get_col(ps.T,0), "%%GraphLab collaborative filtering library. This file holds eigenvalues of the matrix A*A'\n", false, false);
      save_matrix_market_vector((std::string(filename) + ".EigenValues_ATA").c_str(),get_col(ps.T,1), "%%GraphLab collaborative filtering library. This file holds eigenvalues of the matrix A'*A\n", false, false);
      return;
    }

    if (ps.algorithm == SVD_PLUS_PLUS){
      save_matrix_market_vector((std::string(filename) + ".UserBias").c_str(),ps.svdpp_usr_bias, "%%GraphLab collaborative filtering library. This file holds user bias vector. In row i we have bias of user i.\n", false, false);
      save_matrix_market_vector((std::string(filename) + ".MovieBias").c_str(),ps.svdpp_movie_bias, "%%GraphLab collaborative filtering library. This file holds user bias vector. In row i we have bias of movie i.\n", false, false);
      save_matrix_market_matrix((std::string(filename) + ".Users").c_str(),ps.U,"%%GraphLab Collaborative filtering library. This file holds the matrix U. Row i holds the feature vector for user i.\n", false, false);
      save_matrix_market_matrix((std::string(filename) + ".Movies").c_str(),ps.V,"%%GraphLab Collaborative filtering library. This file holds the matrix V. Row i holds the feature vector for movie i.\n", false, false);
      mat gmean = mat(1,1);
      set_val(gmean,0,0,ps.globalMean[0]);
      save_matrix_market_matrix((std::string(filename) + ".GlobalMean").c_str(),gmean, "%%GraphLab collaborative filtering library. This file holds the global mean value.\n", false, false);
      return;
    }

    if (ps.algorithm == TIME_SVD_PLUS_PLUS){
      save_matrix_market_vector((std::string(filename) + ".UserBias").c_str(),ps.svdpp_usr_bias,"%%GraphLab collaborative filtering library. This file holds user bias vector. In row i we have bias of user i.\n", false, false);
      save_matrix_market_vector((std::string(filename) + ".MovieBias").c_str(),ps.svdpp_movie_bias,"%%GraphLab collaborative filtering library. This file holds user bias vector. In row i we have bias of movie i.\n", false, false);
      save_matrix_market_matrix((std::string(filename) + ".Users_ptemp").c_str(),ps.timesvdpp_out.ptemp,"%%GraphLab collaborative filtering library. This file holds ptemp array for time-SVD++\n", false, false);
      save_matrix_market_matrix((std::string(filename) + ".Users_x").c_str(),ps.timesvdpp_out.x,"%%GraphLab collaborative filtering library. This file holds x array for time-SVD++\n", false, false);
      save_matrix_market_matrix((std::string(filename) + ".Users_pu").c_str(),ps.timesvdpp_out.pu,"%%GraphLab collaborative filtering library. This file holds pu array for time-SVD++\n", false, false);
      save_matrix_market_matrix((std::string(filename) + ".Movies_q").c_str(),ps.timesvdpp_out.q,"%%GraphLab collaborative filtering library. This file holds q array for time-SVD++\n", false, false);
      save_matrix_market_matrix((std::string(filename) + ".Time_z").c_str(),ps.timesvdpp_out.z,"%%GraphLab collaborative filtering library. This file holds z array for time-SVD++\n", false, false);
      save_matrix_market_matrix((std::string(filename) + ".Time_pt").c_str(),ps.timesvdpp_out.pt,"%%GraphLab collaborative filtering library. This file holds pt array for time-SVD++\n", false, false);
      mat gmean = mat(1,1);
      set_val(gmean,0,0,ps.globalMean[0]);
      save_matrix_market_matrix((std::string(filename) + ".GlobalMean").c_str(),gmean,"%%GraphLab collaborative filtering library. This file holds the global mean value.\n",false,false);
      return;
    }
}
#endif //READ_MARTIRX_MARKET
