#include "distance.h"
#include "clustering.h"
#include "../gabp/advanced_config.h"

extern advanced_config ac;
const char * distance_measure_name[] = {"EUCLIDEAN", "CHEBYCHEV", "MANAHOLIS", "MANHATTAN", "MINKOWSKI", "TANIMOTO", "WEIGTED", "WEIGHTED_MANAHOLIS", "COSINE", "LOGLIKELIHOOD"};





double safeLog(double d) {
    return d <= 0.0 ? 0.0 : log(d);
}
double logL(double p, double k, double n) {
    return k * safeLog(p) + (n - k) * safeLog(1.0 - p);
}

double twoLogLambda(double k1, double k2, double n1, double n2) {
    double p = (k1 + k2) / (n1 + n2);
    return 2.0 * (logL(k1 / n1, k1, n1) + logL(k2 / n2, k2, n2) - logL(p, k1, n1) - logL(p, k2, n2));
}

flt_dbl calc_loglikelihood_distance( sparse_flt_dbl_vec & datapoint, sparse_flt_dbl_vec & cluster, flt_dbl sqr_sum, flt_dbl sqr_sum_datapoint){ 
   flt_dbl intersection = dot_prod(datapoint , cluster);
   flt_dbl logLikelihood = twoLogLambda(intersection,
                                        sqr_sum - intersection,
                                        sqr_sum_datapoint,
                                        datapoint.size() - sqr_sum_datapoint);
    return 1.0 - 1.0 / (1.0 + logLikelihood);
}

flt_dbl calc_loglikelihood_distance( sparse_flt_dbl_vec & datapoint,  flt_dbl_vec &cluster, flt_dbl sqr_sum, flt_dbl sqr_sum_datapoint){
  flt_dbl intersection = dot_prod(datapoint, cluster);
  flt_dbl logLikelihood = twoLogLambda(intersection,
                                        sqr_sum - intersection,
                                        sqr_sum_datapoint,
                                        datapoint.size() - sqr_sum_datapoint);
   return 1.0 - 1.0 / (1.0 + logLikelihood);
}



flt_dbl calc_tanimoto_distance( sparse_flt_dbl_vec & datapoint, sparse_flt_dbl_vec & cluster, flt_dbl sqr_sum, flt_dbl sqr_sum_datapoint){ 
  flt_dbl a_mult_b = dot_prod(datapoint , cluster);
  flt_dbl div = (sqr_sum + sqr_sum_datapoint - a_mult_b);
  if (ac.debug && (div == 0 || a_mult_b/div < 0)){
     logstream(LOG_ERROR) << "divisor is zeo: " << sqr_sum<< " " << sqr_sum_datapoint << " " << a_mult_b << " " << std::endl;
     print(datapoint);
     print(cluster);
     exit(1);
  }
  return 1.0 - a_mult_b/div;
}

flt_dbl calc_tanimoto_distance( sparse_flt_dbl_vec & datapoint,  flt_dbl_vec &cluster, flt_dbl sqr_sum, flt_dbl sqr_sum_datapoint){
  flt_dbl a_mult_b = dot_prod(datapoint, cluster);
  flt_dbl div = (sqr_sum + sqr_sum_datapoint - a_mult_b);
  if (ac.debug && (div == 0 || a_mult_b/div < 0)){
     logstream(LOG_ERROR) << "divisor is zeo: " << sqr_sum << " " << sqr_sum_datapoint << " " << a_mult_b << " " << std::endl;
     print(datapoint);
     debug_print_vec("cluster", cluster, cluster.size());
     exit(1);
  }
  return 1.0 - a_mult_b/div;
}

flt_dbl calc_euclidian_distance( sparse_flt_dbl_vec & datapoint,  sparse_flt_dbl_vec &cluster, flt_dbl sqr_sum, flt_dbl sqr_sum_datapoint){
  //sparse_flt_dbl_vec diff = minus(datapoint , cluster);
  //return sqrt(sum_sqr(diff));
  sparse_flt_dbl_vec mult = elem_mult(datapoint, cluster);
  flt_dbl diff = (sqr_sum + sqr_sum_datapoint - 2*sum(mult));
  return sqrt(fabs(diff)); //because of numerical errors, diff may be negative
}


flt_dbl calc_euclidian_distance( sparse_flt_dbl_vec & datapoint,  flt_dbl_vec &cluster, flt_dbl sqr_sum, flt_dbl sqr_sum_datapoint){
  flt_dbl dist = sqr_sum + sqr_sum_datapoint;
  //for (int i=0; i< datapoint.nnz(); i++){
  FOR_ITERATOR_(i, datapoint){
      flt_dbl val = get_nz_data(datapoint, i);
      int pos = get_nz_index(datapoint, i);
      dist -= 2*val*cluster[pos];
   }
  if (ac.debug && dist < 0 && fabs(dist) > 1e-8){
     logstream(LOG_WARNING)<<"Found a negative distance: " << dist << " initial sum: " << sqr_sum_datapoint + sqr_sum << std::endl;
     logstream(LOG_WARNING)<<"sqr sum: " << sqr_sum << " sqr_sum_datapoint: " <<sqr_sum_datapoint<<std::endl;
     FOR_ITERATOR_(i, datapoint){
        int pos = get_nz_index(datapoint, i);
        logstream(LOG_WARNING)<<"Data: " << get_nz_data(datapoint, i) << " Pos: " << get_nz_index(datapoint, i) <<" cluster valu: " << cluster[pos] 
            << "reduction: " << 2*get_nz_data(datapoint,i)*cluster[pos] << std::endl;
     } 
     dist = 0;
  
  }
  return sqrt(fabs(dist)); //should not happen, but distance is sometime negative because of the shortcut we make to calculate it..
}

flt_dbl calc_chebychev_distance( sparse_flt_dbl_vec & datapoint,  sparse_flt_dbl_vec &cluster){
   sparse_flt_dbl_vec diff = minus(datapoint , cluster);
   flt_dbl ret = 0;
   FOR_ITERATOR(i, diff){
      ret = std::max(ret, (flt_dbl)fabs(get_nz_data(diff, i)));
   }
   return ret;

}
flt_dbl calc_chebychev_distance( sparse_flt_dbl_vec & datapoint,  flt_dbl_vec &cluster){
   flt_dbl_vec diff = minus(datapoint , cluster);
   flt_dbl ret = 0;
   for (int i=0; i< diff.size(); i++)
      ret = std::max(ret, (flt_dbl)fabs(diff[i]));

   return ret;

}

flt_dbl calc_manhatten_distance( sparse_flt_dbl_vec & datapoint,  sparse_flt_dbl_vec &cluster){
   sparse_flt_dbl_vec diff = minus(datapoint , cluster);
   sparse_flt_dbl_vec absvec = fabs(diff);
   flt_dbl ret = sum(absvec);
   return ret;

}
flt_dbl calc_manhatten_distance( sparse_flt_dbl_vec & datapoint,  flt_dbl_vec &cluster){
   flt_dbl_vec diff = minus(datapoint , cluster);
   flt_dbl ret = sum(fabs(diff));
   return ret;

}

flt_dbl calc_cosine_distance( sparse_flt_dbl_vec & datapoint,  sparse_flt_dbl_vec & cluster, flt_dbl sum_sqr, flt_dbl sum_sqr0){
   flt_dbl dotprod = dot_prod(datapoint,cluster);
   flt_dbl denominator = sqrt(sum_sqr0)*sqrt(sum_sqr);
   return 1.0 - dotprod / denominator; 
}

flt_dbl calc_cosine_distance( sparse_flt_dbl_vec & datapoint,  flt_dbl_vec & cluster, flt_dbl sum_sqr, flt_dbl sum_sqr0){
   flt_dbl dotprod = dot_prod(datapoint,cluster);
   flt_dbl denominator = sqrt(sum_sqr0)*sqrt(sum_sqr);
   return 1.0 - dotprod / denominator; 
}


flt_dbl calc_distance(sparse_flt_dbl_vec &datapoint,  sparse_flt_dbl_vec & cluster, flt_dbl sqr_sum, flt_dbl sqr_sum_datapoint){
   switch(ac.distance_measure){
      case EUCLIDEAN:          
          return calc_euclidian_distance(datapoint, cluster, sqr_sum, sqr_sum_datapoint);
      case CHEBYCHEV:
          return calc_chebychev_distance(datapoint, cluster);
      case COSINE:
	  return calc_cosine_distance(datapoint, cluster, sqr_sum, sqr_sum_datapoint);  
      case MANHATTAN:
          return calc_manhatten_distance(datapoint, cluster);
      case TANIMOTO:
          return calc_tanimoto_distance(datapoint, cluster, sqr_sum , sqr_sum_datapoint);
      case LOGLIKELIHOOD:
          return calc_loglikelihood_distance(datapoint, cluster, sqr_sum, sqr_sum_datapoint);
      case MANAHOLIS:
      case WEIGHTED_MANAHOLIS:
      case WEIGHTED:
      default:
          logstream(LOG_ERROR)<< "distance measure " << ac.distance_measure<< "  not implemented yet" << std::endl;
    }
    return -1;

}


flt_dbl calc_distance(sparse_flt_dbl_vec &datapoint,  flt_dbl_vec & cluster, flt_dbl sqr_sum, flt_dbl sqr_sum_datapoint){
   switch(ac.distance_measure){
      case EUCLIDEAN:          
          return calc_euclidian_distance(datapoint, cluster, sqr_sum, sqr_sum_datapoint);
      case CHEBYCHEV:
          return calc_chebychev_distance(datapoint, cluster);
      case COSINE:
	  return calc_cosine_distance(datapoint, cluster, sqr_sum, sqr_sum_datapoint);  
      case MANHATTAN:
          return calc_manhatten_distance(datapoint, cluster);
      case TANIMOTO:
          return calc_tanimoto_distance(datapoint, cluster, sqr_sum , sqr_sum_datapoint);
      case LOGLIKELIHOOD:
          return calc_loglikelihood_distance(datapoint, cluster, sqr_sum, sqr_sum_datapoint);
      case MANAHOLIS:
      case WEIGHTED_MANAHOLIS:
      case WEIGHTED:
      default:
          logstream(LOG_ERROR)<< "distance measure " << ac.distance_measure<< "  not implemented yet" << std::endl;
    }
    return -1;

}

/**
 *

v1 =
         0    1.0000   -3.5000         0   0
v2 =

         1    2        3               4   5
v3 =
         0    0.5000   0               4   0


 *
 *
 *
 * */
void test_distance(){
  sparse_flt_dbl_vec v1;
  set_size(v1, 5);
  set_new(v1,1,1.0);
  set_new(v1,2,-3.5);

  sparse_flt_dbl_vec v3;
  set_size(v3, 5);
  set_new(v3,1,0.5);
  set_new(v3,3,4);
  flt_dbl_vec v2 = init_vec("1 2 3 4 5", 5);
  ac.distance_measure = EUCLIDEAN;
  flt_dbl ret = calc_distance(v1, v2, sum_sqr(v2), sum_sqr(v1));
  assert(powf(ret - 9.233092656309694,2) < 1e-10);

  ret = calc_distance(v1, v1, sum_sqr(v1), sum_sqr(v1));
  assert(powf(ret - 0, 2) < 1e-10);

  ret = calc_distance(v1, v3, sum_sqr(v1), sum_sqr(v3));
  assert(powf(ret - 5.3385,2) <1e-8);

  ac.distance_measure = COSINE;
  ret = calc_distance(v1, v2, sum_sqr(v1), sum_sqr(v2));
  assert(powf(ret - 1.3149, 2)<1e-8);

  ret = calc_distance(v1, v3, sum_sqr(v1), sum_sqr(v3));
  assert(powf(ret - (1 - .0341), 2)<1e-8);

  ac.distance_measure = TANIMOTO;
  sparse_flt_dbl_vec v4;
  set_size(v4,5);
  set_new(v4, 1, 1);
  set_new(v4, 3, 1);
  flt_dbl_vec v5 = init_vec("0 1 1 1 1", 5);
  ret = calc_distance(v4, v5, sum_sqr(v4), sum_sqr(v5));
  assert(powf(ret - 0.5,2) <1e-8);

  //ac.distance_measure = LOGLIKELIHOOD;
  //assert(powf(calc_distance(v4, v4, sum_sqr(v4), sum_sqr(v4)),1)<1e-8);  

}

