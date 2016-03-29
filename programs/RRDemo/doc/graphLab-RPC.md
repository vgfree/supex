graphLab RPC
========

 GraphLab RPC最初的设计目标是提供方便简洁的方式来完成分布式网络中不同机器的异步通信。它因此提供了类似MPI和RPC结合体的功能。graphLab的分布式实现建立在这个RPC库之上。
GraphLab RPC使用了广泛的模板元编程技术来提供一种IDL-free 的RPC系统，允许远程的机器任意的调用函数（注意所有的机器必须运行相同的二进制代码）。
举个例子，这是个非常有趣的例子：
<!-- lang:c-->
```
`#include <iostream>`
`#include <graphlab/rpc/dc.hpp>`
`#include <graphlab/rpc/dc_init_from_mpi.hpp>`

using namespace graphlab;

int main(int argc, char ** argv) {
  mpi_tools::init(argc, argv);

  dc_init_param param;
  if (init_param_from_mpi(param) == false) {
    return 0;
  }
  distributed_control dc(param);
  
  if (dc.procid() == 0 && dc.numprocs() >= 2) {
    dc.remote_call(1, printf, "%d + %f = %s\n", 1, 2.0, "three");
  }
  dc.barrier();
}
```
一旦distributed_control对象被创建，dc.procid()提供当前机器的编号，dc.numprocs()提供机器总数。
if条件因此只进入第一台机器，执行一个二号机器上的远程调用（remote_call的第一个参数是目标机器的ID）。等价于第二台机器执行：

`printf("%d + %f = %s\n", 1, 2.0, "three");`

我们将分别讨论RPC库的不同方面：
###**Spawning and Initialization**
使用graphLab RPC初始化并启动一个分布式程序。
###**Basic RPC Usage**
RPC库的基本使用。简单函数调用。
Distributed Objects
RPC库的高级用法。创建和管理分布式对象环境。

举例：
test/目录包含了7个RPC的例子来证明所有的关键特性。
•       [RPC Example 1: Basic Synchronous RPC rpc_example1.cpp](http://www.select.cs.cmu.edu/code/graphlab/doxygen/html/rpc__example1_8cpp_source.html)
•       [RPC Example 2: Asynchronous RPC with Built-in Serialization rpc_example2.cpp](http://www.select.cs.cmu.edu/code/graphlab/doxygen/html/rpc__example2_8cpp_source.html)
•       [RPC Example 3: Asynchronous RPC with Struct POD Serialization rpc_example3.cpp](http://www.select.cs.cmu.edu/code/graphlab/doxygen/html/rpc__example3_8cpp_source.html)
•       [RPC Example 4: Asynchronous RPC with Manual Serialization rpc_example4.cpp](http://www.select.cs.cmu.edu/code/graphlab/doxygen/html/rpc__example4_8cpp_source.html)
•       [RPC Example 4: Asynchronous RPC to printf(打印函数) rpc_example5.cpp](http://www.select.cs.cmu.edu/code/graphlab/doxygen/html/rpc__example5_8cpp_source.html)
•       [RPC Example 4: Asynchronous RPC with any rpc_example6.cpp](http://www.select.cs.cmu.edu/code/graphlab/doxygen/html/rpc__example6_8cpp_source.html)
•       [RPC Example 4: Distributed Object rpc_example7.cpp](http://www.select.cs.cmu.edu/code/graphlab/doxygen/html/rpc__example1_8cpp_source.html)

