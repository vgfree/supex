cmake_minimum_required(VERSION 2.8)

include (CheckFunctionExists)
include (CheckIncludeFiles)
include (CheckSymbolExists)

project (libcoro)
add_executable(test test.c)
add_executable(test1 test1.c)
add_library(coro coro.c coro.h coro_cluster.c coro_cluster.h)
#set_target_properties(coro PROPERTIES COMPILE_DEFINITIONS "CORO_ASM=1")

target_link_libraries (test coro pthread)
target_link_libraries (test1 coro pthread)

check_include_files (ucontext.h HAVE_UCONTEXT_H)
check_include_files (setjmp.h HAVE_SETJMP_H)
check_symbol_exists (sigaltstack "signal.h" HAVE_SIGALTSTACK)
check_symbol_exists (sigsetjmp "setjmp.h" HAVE_SIGSETJMP)

configure_file(config.h.in config.h)
include_directories("${PROJECT_BINARY_DIR}")
