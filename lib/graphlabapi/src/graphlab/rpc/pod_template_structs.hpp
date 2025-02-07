#ifndef POD_TEMPLATE_STRUCTS_HPP
#define POD_TEMPLATE_STRUCTS_HPP
#include <graphlab/serialization/serialization_includes.hpp>
#include <graphlab/serialization/is_pod.hpp>

namespace graphlab {
namespace dc_impl {
namespace pod_template_detail {

template <typename F>
struct pod_call_struct0 : public IS_POD_TYPE{
  size_t dispatch_function;
  size_t objid;
  F remote_function;
};


template <typename F, typename T0>
struct pod_call_struct1 : public IS_POD_TYPE{
  size_t dispatch_function;
  size_t objid;
  F remote_function;
  T0 t0;
};


template <typename F, typename T0, typename T1>
struct pod_call_struct2 : public IS_POD_TYPE{
  size_t dispatch_function;
  size_t objid;
  F remote_function;
  T0 t0; T1 t1;
};

template <typename F, typename T0, typename T1, typename T2>
struct pod_call_struct3 : public IS_POD_TYPE{
  size_t dispatch_function;
  size_t objid;
  F remote_function;
  T0 t0; T1 t1; T2 t2;
};


template <typename F, typename T0, typename T1, typename T2,
          typename T3>
struct pod_call_struct4 : public IS_POD_TYPE{
  size_t dispatch_function;
  size_t objid;
  F remote_function;
  T0 t0; T1 t1; T2 t2; T3 t3;
};


template <typename F, typename T0, typename T1, typename T2,
          typename T3, typename T4>
struct pod_call_struct5 : public IS_POD_TYPE{
  size_t dispatch_function;
  size_t objid;
  F remote_function;
  T0 t0; T1 t1; T2 t2; T3 t3; T4 t4;
};


template <typename F, typename T0, typename T1, typename T2,
          typename T3, typename T4, typename T5>
struct pod_call_struct6 : public IS_POD_TYPE{
  size_t dispatch_function;
  size_t objid;
  F remote_function;
  T0 t0; T1 t1; T2 t2; T3 t3; T4 t4; T5 t5;
};


template <typename F, typename T0, typename T1, typename T2,
          typename T3, typename T4, typename T5,
          typename T6>
struct pod_call_struct7 : public IS_POD_TYPE{
  size_t dispatch_function;
  size_t objid;
  F remote_function;
  T0 t0; T1 t1; T2 t2; T3 t3; T4 t4; T5 t5; T6 t6;
};


template <typename F, typename T0, typename T1, typename T2,
          typename T3, typename T4, typename T5,
          typename T6, typename T7>
struct pod_call_struct8 : public IS_POD_TYPE{
  size_t dispatch_function;
  size_t objid;
  F remote_function;
  T0 t0; T1 t1; T2 t2; T3 t3; T4 t4; T5 t5; T6 t6; T7 t7;
};

}
}
}

#endif