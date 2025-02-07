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
 */


#ifndef GRAPHLAB_SERIALIZE_UNORDERED_MAP_HPP
#define GRAPHLAB_SERIALIZE_UNORDERED_MAP_HPP

#include <boost/unordered_map.hpp>
#include <graphlab/serialization/iarchive.hpp>
#include <graphlab/serialization/oarchive.hpp>
#include <graphlab/serialization/iterator.hpp>

namespace graphlab {

  namespace archive_detail {
    /** Serializes a map */
    template <typename ArcType, typename T, typename U>
    struct serialize_impl<ArcType, boost::unordered_map<T,U>, false > {
      static void exec(ArcType& a, const boost::unordered_map<T,U>& vec){
        serialize_iterator(a,vec.begin(),vec.end(), vec.size());
      }
    };

    /** deserializes a map  */
      
    template <typename ArcType, typename T, typename U>
    struct deserialize_impl<ArcType, boost::unordered_map<T,U>, false > {
      static void exec(ArcType& a, boost::unordered_map<T,U>& vec){
        vec.clear();
        // get the number of elements to deserialize
        size_t length = 0;
        a >> length;    
        // iterate through and send to the output iterator
        for (size_t x = 0; x < length ; ++x){
          std::pair<T, U> v;
          a >> v;
          vec[v.first] = v.second;
        }
      }
    };

  } // archive_detail  
} // graphlab
#endif 

