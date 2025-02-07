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


#ifndef OBJECT_BROADCAST_ISSUE_HPP
#define OBJECT_BROADCAST_ISSUE_HPP
#include <iostream>
#include <graphlab/serialization/serialization_includes.hpp>
#include <graphlab/rpc/dc_types.hpp>
#include <graphlab/rpc/dc_internal_types.hpp>
#include <graphlab/rpc/dc_send.hpp>
#include <graphlab/rpc/object_call_dispatch.hpp>
#include <graphlab/rpc/object_call_issue.hpp>
#include <graphlab/rpc/is_rpc_call.hpp>
#include <boost/preprocessor.hpp>
#include <graphlab/rpc/mem_function_arg_types_def.hpp>

namespace graphlab{
namespace dc_impl {

/**
\ingroup rpc_internal
\file
 This is an internal function and should not be used directly

Marshalls a object function broadcast to a remote machine. 

\code
template<typename T, 
        typename F , 
        typename T0> class object_call_issue1
{
    public: static void exec(dc_send* sender, 
                            unsigned char flags, 
                            procid_t target, 
                            size_t objid, 
                            F remote_function , 
                            const T0 &i0 )
    {
        boost::iostreams::stream<resizing_array_sink> strm(128);
        oarchive arc(strm);
        dispatch_type d = dc_impl::OBJECT_NONINTRUSIVE_DISPATCH1<distributed_control,T,F , T0 >;
        arc << reinterpret_cast<size_t>(d);
        serialize(arc, (char*)(&remote_function), sizeof(F));
        arc << objid;
        arc << i0;
        strm.flush();
        sender->send_data(target,flags , strm->str, strm->len);
    }
};
\endcode
*/

#define GENARGS(Z,N,_)  BOOST_PP_CAT(const T, N) BOOST_PP_CAT(&i, N)
#define GENI(Z,N,_) BOOST_PP_CAT(i, N)
#define GENT(Z,N,_) BOOST_PP_CAT(T, N)
#define GENARC(Z,N,_) arc << BOOST_PP_CAT(i, N);


#define REMOTE_BROADCAST_ISSUE_GENERATOR(Z,N,FNAME_AND_CALL) \
template<typename Iterator, typename T, typename F BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_PARAMS(N, typename T)> \
class  BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2,0,FNAME_AND_CALL), N) { \
  public: \
  static void exec(dc_dist_object_base* rmi, std::vector<dc_send*> sender, unsigned char flags, \
                    Iterator target_begin, Iterator target_end, size_t objid, F remote_function BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM(N,GENARGS ,_) ) {  \
    boost::iostreams::stream<resizing_array_sink_ref> &strm = get_thread_local_stream();    \
    oarchive arc(strm);                         \
    strm->advance(sizeof(packet_hdr));            \
    dispatch_type d = BOOST_PP_CAT(dc_impl::OBJECT_NONINTRUSIVE_DISPATCH,N)<distributed_control,T,F BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM(N, GENT ,_) >;   \
    arc << reinterpret_cast<size_t>(d);                                 \
    serialize(arc, (char*)(&remote_function), sizeof(F));               \
    arc << objid;                                                       \
    BOOST_PP_REPEAT(N, GENARC, _)                                       \
    strm.flush();                                                       \
    Iterator iter = target_begin;                                       \
    while(iter != target_end) { \
      Iterator nextiter = iter; ++nextiter; \
      if (nextiter != target_end) { \
        char* newbuf = (char*)malloc(strm->size()); memcpy(newbuf, strm->c_str(), strm->size()); \
        sender[(*iter)]->send_data((*iter),flags , newbuf, strm->size());    \
      } else {    \
        sender[(*iter)]->send_data((*iter),flags , strm->c_str(), strm->size());    \
      } \
      if ((flags & CONTROL_PACKET) == 0) {                                 \
        rmi->inc_bytes_sent((*iter), strm->size()); \
      } \
      iter = nextiter;  \
    } \
    strm->relinquish(); \
  }  \
}; 



/**
Generates a function call issue. 3rd argument is a tuple (issue name, dispacther name)
*/
BOOST_PP_REPEAT(7, REMOTE_BROADCAST_ISSUE_GENERATOR,  (object_broadcast_issue, _) )



#undef GENARC
#undef GENT
#undef GENI
#undef GENARGS
#undef REMOTE_BROADCAST_ISSUE_GENERATOR

} // namespace dc_impl
} // namespace graphlab

#include <graphlab/rpc/mem_function_arg_types_undef.hpp>

#endif

