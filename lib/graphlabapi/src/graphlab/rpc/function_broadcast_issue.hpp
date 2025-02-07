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


#ifndef FUNCTION_BROADCAST_ISSUE_HPP
#define FUNCTION_BROADCAST_ISSUE_HPP
#include <iostream>
#include <graphlab/serialization/serialization_includes.hpp>
#include <graphlab/rpc/dc_types.hpp>
#include <graphlab/rpc/dc_internal_types.hpp>
#include <graphlab/rpc/dc_send.hpp>
#include <graphlab/rpc/function_call_dispatch.hpp>
#include <graphlab/rpc/function_call_issue.hpp>
#include <graphlab/rpc/is_rpc_call.hpp>
#include <boost/preprocessor.hpp>
#include <graphlab/rpc/function_arg_types_def.hpp>

namespace graphlab{
namespace dc_impl {

/**
\ingroup rpc_internal
\file

This is an internal function and should not be used directly

A "call" is an RPC which is performed asynchronously.
There are 2 types of calls. A "basic" call calls a standard C/C++ function
and does not require the function to be modified.
while the "regular" call requires the first 2 arguments of the function 
to be "distributed_control &dc, procid_t source".

An "issue" is a wrapper function on the sending side of an RPC
which encodes the packet and transmits it to the other side. 
(I realized later this is called a "Marshaller")

Native Call Formats \n
=================== \n
The format of a "call" packet is in the form of an archive and is as follows

\li (dispatch_type*) -- pointer to target machine's dispatcher function
\li (void*)          -- pointer to target function
\li fn::arg1_type    -- target function's 1st argument
\li fn::arg2_type    -- target function's 2nd argument
\li  ...
\li fn::argN_type    -- target function's Nth argument

Argument casting is deferred to as late as possible. So the data type of
arguments are the data types that the caller use to call the function. 
A dispatcher function will be instantiated with the input types, which will
then perform the type cast.


The code below generates the following for different number of arguments. Here, we 
demonstrate the 1 argument version.
\code
namespace function_call_issue_detail
{
    template <typename BoolType, 
            typename F , 
            typename T0> struct dispatch_selector1
    {
        static dispatch_type dispatchfn()
        {
            return dc_impl::NONINTRUSIVE_DISPATCH1<distributed_control,F , T0 >;
        }
    };
    template <typename F , typename T0> struct dispatch_selector1<boost::mpl::bool_<true>, F , T0>
    {
        static dispatch_type dispatchfn()
        {
            return dc_impl::DISPATCH1<distributed_control,F , T0 >;
        }
    };
}


template<Iterator, typename F , typename T0> class remote_call_issue1
{
    public: static void exec(dc_send* sender, 
                            unsigned char flags, 
                            Iterator target_begin,
                            Iterator target_end,
                            F remote_function , 
                            const T0 &i0 )
    {
        boost::iostreams::stream<resizing_array_sink> strm(128);
        oarchive arc(strm);
        dispatch_type d = function_call_issue_detail::dispatch_selector1<typename is_rpc_call<F>::type, F , T0 >::dispatchfn();
        arc << reinterpret_cast<size_t>(d);
        arc << reinterpret_cast<size_t>(remote_function);
        arc << i0;
        strm.flush();
        Iterator iter = target_begin;
        while(iter != target_end) {
          Iteratator nextiter = iter; ++nextiter;
          if (nextiter != target_end) {
            char* newbuf = malloc(strm->len); memcpy(newbuf, strm->str, strm->len);
            sender->send_data((*iter),flags , newbuf, strm->len);
          }
          else {
            sender->send_data((*iter),flags , strm->str, strm->len);
          }
          iter = nextiter;
          strm->relinquish();
        }
    }
};

\endcode

The basic idea of the code is straightforward.
The receiving end cannot call the target function (remote_function) directly, since it has 
no means of understanding how to deserialize or to construct the stack for the remote_function.
So instead, we generate a "dispatch" function on the receiving side. The dispatch function
is constructed according to the type information of the remote_function, and therefore knows
how to deserialize the data, and issue the function call. That is the "dispatch_type".

However, since we defined two families of receiving functions:
 a non-intrusive version which does not take (dc, procid) as an argument
 and an intrusive version which does, the dispatch function must therefore be slightly different
 for each of them. That is what the dispatch_selector class performs.
 The first template argument of the dispatch_selector family of classes is a boolean flag which
 denotes whether the function is a non-intrusive call or not. This boolean flag itself
 is determined using the is_rpc_call<F>::type template.
*/



#define GENARGS(Z,N,_)  BOOST_PP_CAT(const T, N) BOOST_PP_CAT(&i, N)
#define GENI(Z,N,_) BOOST_PP_CAT(i, N)
#define GENT(Z,N,_) BOOST_PP_CAT(T, N)
#define GENARC(Z,N,_) arc << BOOST_PP_CAT(i, N);


/**
The dispatch_selectorN structs are used to pick between the standard dispatcher and the nonintrusive dispatch
by checking if the function is a RPC style call or not.
*/
#define REMOTE_BROADCAST_ISSUE_GENERATOR(Z,N,FNAME_AND_CALL) \
template<typename Iterator, typename F BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_PARAMS(N, typename T)> \
class  BOOST_PP_CAT(FNAME_AND_CALL, N) { \
  public: \
  static void exec(std::vector<dc_send*>& sender, unsigned char flags, Iterator target_begin, Iterator target_end, F remote_function BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM(N,GENARGS ,_) ) {  \
    boost::iostreams::stream<resizing_array_sink_ref> &strm = get_thread_local_stream();    \
    strm->advance(sizeof(packet_hdr));            \
    oarchive arc(strm);                         \
    dispatch_type d = BOOST_PP_CAT(function_call_issue_detail::dispatch_selector,N)<typename is_rpc_call<F>::type, F BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_PARAMS(N, T) >::dispatchfn();   \
    arc << reinterpret_cast<size_t>(d);       \
    arc << reinterpret_cast<size_t>(remote_function); \
    BOOST_PP_REPEAT(N, GENARC, _)                \
    strm.flush();           \
    Iterator iter = target_begin; \
    while(iter != target_end) { \
      Iterator nextiter = iter; ++nextiter; \
      if (nextiter != target_end) { \
        char* newbuf = (char*)malloc(strm->size()); memcpy(newbuf, strm->c_str(), strm->size()); \
        sender[(*iter)]->send_data((*iter),flags , newbuf, strm->size());    \
      } \
      else {  \
        sender[(*iter)]->send_data((*iter),flags , strm->c_str(), strm->size());    \
      } \
      iter = nextiter;  \
    } \
    strm->relinquish(); \
  }\
}; 



/**
Generates a function call issue. 3rd argument is the issue name
*/
BOOST_PP_REPEAT(6, REMOTE_BROADCAST_ISSUE_GENERATOR,  remote_broadcast_issue )



#undef GENARC
#undef GENT
#undef GENI
#undef GENARGS
#undef REMOTE_BROADCAST_ISSUE_GENERATOR

} // namespace dc_impl
} // namespace graphlab

#include <graphlab/rpc/function_arg_types_undef.hpp>

#endif

