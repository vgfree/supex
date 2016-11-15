/**
 * @file   macros.h
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date   Sun Dec 27 23:32:50 2009
 *
 * @brief  Header file for global macros
 *
 *
 */

#ifndef RINOO_MACROS_H_
#define RINOO_MACROS_H_

#if defined(__GNUC__) && (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96))
#define	likely(x)	__builtin_expect((x), 1)
#define	unlikely(x)	__builtin_expect((x), 0)
#else
#define	likely(x)	(x)
#define	unlikely(x)	(x)
#endif

#if defined(__GNUC__)
#define unused(x)	x __attribute__((unused))
#else
#define unused(x)	x
#endif

#define	container_of(ptr, type, member) ({ \
	const typeof(((type *) 0)->member) *__mptr = (ptr); \
	(type *)( (char *)__mptr - offsetof(type,member) );})

#define FOR_EACH_0(func, arg, ...)	func(arg)
#define FOR_EACH_1(func, arg, x)	func(arg, x)
#define FOR_EACH_2(func, arg, x, ...)	func(arg, x); FOR_EACH_1(func, arg, __VA_ARGS__)
#define FOR_EACH_3(func, arg, x, ...)	func(arg, x); FOR_EACH_2(func, arg, __VA_ARGS__)
#define FOR_EACH_4(func, arg, x, ...)	func(arg, x); FOR_EACH_3(func, arg, __VA_ARGS__)
#define FOR_EACH_5(func, arg, x, ...)	func(arg, x); FOR_EACH_4(func, arg, __VA_ARGS__)
#define FOR_EACH_6(func, arg, x, ...)	func(arg, x); FOR_EACH_5(func, arg, __VA_ARGS__)
#define FOR_EACH_7(func, arg, x, ...)	func(arg, x); FOR_EACH_6(func, arg, __VA_ARGS__)
#define FOR_EACH_8(func, arg, x, ...)	func(arg, x); FOR_EACH_7(func, arg, __VA_ARGS__)
#define FOR_EACH_9(func, arg, x, ...)	func(arg, x); FOR_EACH_8(func, arg, __VA_ARGS__)
#define FOR_EACH_10(func, arg, x, ...)	func(arg, x); FOR_EACH_9(func, arg, __VA_ARGS__)
#define FOR_EACH_11(func, arg, x, ...)	func(arg, x); FOR_EACH_10(func, arg, __VA_ARGS__)
#define FOR_EACH_12(func, arg, x, ...)	func(arg, x); FOR_EACH_11(func, arg, __VA_ARGS__)
#define FOR_EACH_13(func, arg, x, ...)	func(arg, x); FOR_EACH_12(func, arg, __VA_ARGS__)
#define FOR_EACH_14(func, arg, x, ...)	func(arg, x); FOR_EACH_13(func, arg, __VA_ARGS__)
#define FOR_EACH_15(func, arg, x, ...)	func(arg, x); FOR_EACH_14(func, arg, __VA_ARGS__)
#define FOR_EACH_16(func, arg, x, ...)	func(arg, x); FOR_EACH_15(func, arg, __VA_ARGS__)
#define FOR_EACH_17(func, arg, x, ...)	func(arg, x); FOR_EACH_16(func, arg, __VA_ARGS__)
#define FOR_EACH_18(func, arg, x, ...)	func(arg, x); FOR_EACH_17(func, arg, __VA_ARGS__)
#define FOR_EACH_19(func, arg, x, ...)	func(arg, x); FOR_EACH_18(func, arg, __VA_ARGS__)
#define FOR_EACH_20(func, arg, x, ...)	func(arg, x); FOR_EACH_19(func, arg, __VA_ARGS__)
#define FOR_EACH_21(func, arg, x, ...)	func(arg, x); FOR_EACH_20(func, arg, __VA_ARGS__)
#define FOR_EACH_22(func, arg, x, ...)	func(arg, x); FOR_EACH_21(func, arg, __VA_ARGS__)
#define FOR_EACH_23(func, arg, x, ...)	func(arg, x); FOR_EACH_22(func, arg, __VA_ARGS__)
#define FOR_EACH_24(func, arg, x, ...)	func(arg, x); FOR_EACH_23(func, arg, __VA_ARGS__)
#define FOR_EACH_25(func, arg, x, ...)	func(arg, x); FOR_EACH_24(func, arg, __VA_ARGS__)
#define FOR_EACH_26(func, arg, x, ...)	func(arg, x); FOR_EACH_25(func, arg, __VA_ARGS__)
#define FOR_EACH_27(func, arg, x, ...)	func(arg, x); FOR_EACH_26(func, arg, __VA_ARGS__)
#define FOR_EACH_28(func, arg, x, ...)	func(arg, x); FOR_EACH_27(func, arg, __VA_ARGS__)
#define FOR_EACH_29(func, arg, x, ...)	func(arg, x); FOR_EACH_28(func, arg, __VA_ARGS__)
#define FOR_EACH_30(func, arg, x, ...)	func(arg, x); FOR_EACH_29(func, arg, __VA_ARGS__)
#define FOR_EACH_31(func, arg, x, ...)	func(arg, x); FOR_EACH_30(func, arg, __VA_ARGS__)
#define FOR_EACH_32(func, arg, x, ...)	func(arg, x); FOR_EACH_31(func, arg, __VA_ARGS__)
#define FOR_EACH_33(func, arg, x, ...)	func(arg, x); FOR_EACH_32(func, arg, __VA_ARGS__)
#define FOR_EACH_34(func, arg, x, ...)	func(arg, x); FOR_EACH_33(func, arg, __VA_ARGS__)
#define FOR_EACH_35(func, arg, x, ...)	func(arg, x); FOR_EACH_34(func, arg, __VA_ARGS__)
#define FOR_EACH_36(func, arg, x, ...)	func(arg, x); FOR_EACH_35(func, arg, __VA_ARGS__)
#define FOR_EACH_37(func, arg, x, ...)	func(arg, x); FOR_EACH_36(func, arg, __VA_ARGS__)
#define FOR_EACH_38(func, arg, x, ...)	func(arg, x); FOR_EACH_37(func, arg, __VA_ARGS__)
#define FOR_EACH_39(func, arg, x, ...)	func(arg, x); FOR_EACH_38(func, arg, __VA_ARGS__)
#define FOR_EACH_40(func, arg, x, ...)	func(arg, x); FOR_EACH_39(func, arg, __VA_ARGS__)
#define FOR_EACH_41(func, arg, x, ...)	func(arg, x); FOR_EACH_40(func, arg, __VA_ARGS__)
#define FOR_EACH_42(func, arg, x, ...)	func(arg, x); FOR_EACH_41(func, arg, __VA_ARGS__)
#define FOR_EACH_43(func, arg, x, ...)	func(arg, x); FOR_EACH_42(func, arg, __VA_ARGS__)
#define FOR_EACH_44(func, arg, x, ...)	func(arg, x); FOR_EACH_43(func, arg, __VA_ARGS__)
#define FOR_EACH_45(func, arg, x, ...)	func(arg, x); FOR_EACH_44(func, arg, __VA_ARGS__)
#define FOR_EACH_46(func, arg, x, ...)	func(arg, x); FOR_EACH_45(func, arg, __VA_ARGS__)
#define FOR_EACH_47(func, arg, x, ...)	func(arg, x); FOR_EACH_46(func, arg, __VA_ARGS__)
#define FOR_EACH_48(func, arg, x, ...)	func(arg, x); FOR_EACH_47(func, arg, __VA_ARGS__)
#define FOR_EACH_49(func, arg, x, ...)	func(arg, x); FOR_EACH_48(func, arg, __VA_ARGS__)
#define FOR_EACH_50(func, arg, x, ...)	func(arg, x); FOR_EACH_49(func, arg, __VA_ARGS__)
#define FOR_EACH_51(func, arg, x, ...)	func(arg, x); FOR_EACH_50(func, arg, __VA_ARGS__)
#define FOR_EACH_52(func, arg, x, ...)	func(arg, x); FOR_EACH_51(func, arg, __VA_ARGS__)
#define FOR_EACH_53(func, arg, x, ...)	func(arg, x); FOR_EACH_52(func, arg, __VA_ARGS__)
#define FOR_EACH_54(func, arg, x, ...)	func(arg, x); FOR_EACH_53(func, arg, __VA_ARGS__)
#define FOR_EACH_55(func, arg, x, ...)	func(arg, x); FOR_EACH_54(func, arg, __VA_ARGS__)
#define FOR_EACH_56(func, arg, x, ...)	func(arg, x); FOR_EACH_55(func, arg, __VA_ARGS__)
#define FOR_EACH_57(func, arg, x, ...)	func(arg, x); FOR_EACH_56(func, arg, __VA_ARGS__)
#define FOR_EACH_58(func, arg, x, ...)	func(arg, x); FOR_EACH_57(func, arg, __VA_ARGS__)
#define FOR_EACH_59(func, arg, x, ...)	func(arg, x); FOR_EACH_58(func, arg, __VA_ARGS__)
#define FOR_EACH_60(func, arg, x, ...)	func(arg, x); FOR_EACH_59(func, arg, __VA_ARGS__)
#define FOR_EACH_61(func, arg, x, ...)	func(arg, x); FOR_EACH_60(func, arg, __VA_ARGS__)
#define FOR_EACH_62(func, arg, x, ...)	func(arg, x); FOR_EACH_61(func, arg, __VA_ARGS__)
#define FOR_EACH_63(func, arg, x, ...)	func(arg, x); FOR_EACH_62(func, arg, __VA_ARGS__)

#define __VA_NARG__(...)	__VA_NARG_(_0, ## __VA_ARGS__, __RSEQ_N())
#define __VA_NARG_(...)		__VA_ARG_N(__VA_ARGS__)
#define __VA_ARG_N( \
	_1, _2, _3, _4, _5, _6, _7, _8, _9, _10,  \
	_11, _12, _13, _14, _15, _16, _17, _18, _19, _20,  \
	_21, _22, _23, _24, _25, _26, _27, _28, _29, _30,  \
	_31, _32, _33, _34, _35, _36, _37, _38, _39, _40,  \
	_41, _42, _43, _44, _45, _46, _47, _48, _49, _50,  \
	_51, _52, _53, _54, _55, _56, _57, _58, _59, _60,  \
	_61, _62, _63, N, ...) N
#define __RSEQ_N() \
	62, 61, 60, \
	59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
	49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
	39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
	29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
	19, 18, 17, 16, 15, 14, 13, 12, 11, 10, \
	9,  8,  7,  6,  5,  4,  3,  2,  1,  0

#define CONCAT(arg1, arg2)   CONCAT1(arg1, arg2)
#define CONCAT1(arg1, arg2)  CONCAT2(arg1, arg2)
#define CONCAT2(arg1, arg2)  arg1##arg2

#define FOR_EACH_(N, func, arg, ...) CONCAT(FOR_EACH_, N)(func, arg, __VA_ARGS__)
#define FOR_EACH(func, arg, ...) FOR_EACH_(__VA_NARG__(__VA_ARGS__), func, arg, __VA_ARGS__)

#endif /* !RINOO_MACROS_H_ */
