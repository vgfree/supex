Distributed GraphLab
=======
为了使用分布式GraphLab，需要MPI。我们已经在OS X系统上测试了MPICH2和MPI分布式，然而，其他的分布式MPI(Open MPI)也应该好使。
分布式graphLab与常规的GraphLab的功能是一样的，只是因为缺乏抽象规则，用户需要考虑一些问题。
首先，图不能凭空创建，它必须实现创建好并且从原子索引文件中加载（Distributed Graph Creation ）。
其次，因为多实例程序跨网络启动，需要协调保证所有实例表现一致。我们强烈鼓励使用graphlab::core （它的分布式版本是[graphlab::distributed_core](http://www.select.cs.cmu.edu/code/graphlab/doxygen/html/classgraphlab_1_1distributed__core.html)）因为它简化了大量的管理操作。
**注意**：
常规规则是任何操作，任何影响全局规模的操作（例如设置配置）都应该被所有实例同时调用。
用户如果已经阅读了共享内存的详细例子，或者已经熟悉了共享内存的GraphLab，可以轻松的继续阅读。否则这里有一个分布式版本的共享内存的详细例子：[Distributed Detailed Example ](http://www.select.cs.cmu.edu/code/graphlab/doxygen/html/distributed_detailed_example.html)。

根据你程序的复杂程度，从共享内存版本挪到分布式版本是非常机械的，接下来列举一些关键的不同点。
一个非常简单的方式来看出从共享内存到分布式版本你需要修改哪些地方，就是对比：
tests/demo/demo.cpp 和 tests/dist_demo/dist_demo.cpp的不同。

###**Types**
不是包含graphlab.hpp, 你应该包含distributed_graphlab.hpp。类似的，分布式版本的核心是dgraphlab::istributed_core 并且分布式版本的类型结构是graphlab::distributed_types。我们推荐使用graphlab::distributed_types系统：
<!-- lang:c -->
```
typedef graphlab::distributed_graph<vertex_data, edge_data> graph_type;
typedef graphlab::distributed_types<graph_type> gl;
```

因为它可以让你很容易从共享内存的实现复制粘贴到分布式版本而不用改动后面的实现。
举个例子，你的更新函数不应该使用graphlab::edge_list， 而应该使用gl::edge_list因为共享内存的图和分布式的图类型是不同的。
类似的，应该使用gl::distributed_core而不是core,，应该使用gl::distributed_glshared而不是glshared。

###**Use The Disk Graph**
不像共享内存版本，图不可能凭空创建。用户必须先创建一个磁盘图，接下来从磁盘图构建
distributed_core和distributed_graph。
####**Distributed Execution**
用户需要始终记住在分布式设置中，有一系列的独立拷贝程序被执行，所有的通信都通过MPI或者graphLab RPC系统。
所有的分布式核心和引擎通常需要所有机器在相同的代码路径下，同时运行相同的函数。举个例子：
<!-- lang:c -->
```
  core.set_sync(...);
  core.set_engine_options(clopts);
  core.build_engine();
  ```
  
它需要被所有机器同时按照顺序执行。有些函数（像core.build_engine()）内部有分布式屏障来保证所有机器在开始之前都到达了build_engine()函数。这个规则有一些例外（像add_task()），他们都已经在文档中写到。

####** Distributed Graph Limitations **
普通的图有通过引用来获取顶点和边数据的功能，分布式图也有相似的功能，但是有些浅显的原因，它严格限制了顶点和边的数据只能被本地划分（local partition）的区域拥有。应该使用替代的函数get_vertex_data()/ set_vertex_data()函数，对于边也是同样的。这些返回值可以跨网络get/set边和顶点的数据。
用户需要记住远程网络的gets/sets操作本来就慢，。如果需要大量的读写顶点或边，强烈建议用户充分利用RPC操作来批量收集或散射信息，而不是每次读取一条数据。选择性的，可以将图存在磁盘上，并且用一台机器加载数据来充分利用磁盘图的类。
####**Build The Engine**
共享内存核心总动销毁然后根据参数的改变重建引擎。不幸的是在分布式设置中很难调整，因此，一旦所有的参数都设置完成，用户必须明确的通过
graphlab::distributed_core::build_engine()  函数来构建引擎。
Distributed Command Line Options
graphlab::command_line_options对象自动为共享内存引擎从加载参数，为分布式引擎加载参数：

<!-- lang:c -->
```opts.use_distributed_options();```

必须必opts.parse()优先调用。

####**Distributed Engine Types**
用户会注意到新的命令行选项包含一个—engine选项，Distributed GraphLab使用了完全不同的特征来实现这两种引擎。一个被称作彩色引擎(--engine=dist_chromatic)， 另外一个被称作锁引擎（--engine=dist_locking），彩色引擎是默认的。

#####**Chromatic Engine**
彩色引擎是在每个顶点上绑定一个更新任务，然后充分利用图的着色来并行执行更新任务。举个例子，如果我的图有三种颜色，它将并行的在所有颜色为0的顶点上执行更新任务，停止后，接下来再在所有颜色为1的顶点上并行执行…
彩色引擎因此不需要调度器，因为它本来就建立了一个彩色调度器。然而，它假设图上的颜色在请求范围内是合法的，换句话说，如果一条边的范围(scope)被请求到，那么土的颜色必须保证邻接顶点的颜色不能与之相同。如果违反了颜色约束，尽管引擎仍旧能够顺利的跑，然是一致性并不能够得到保证。
Engine Options  最大迭代次数以及是否需要交换顶点顺序是可以设置的，有两个引擎参数，他们可以通过命令行设置：“max_iterations=N”,“randomize_schedule=0 或1”。
<!--lang:c-->
```
--engine="dist_chromatic(max_iterations=10,randomize_schedule=1)"
```

#####**Limitations**
彩色引擎因此在某种程度上受限于它的调度能力（只支持色彩调度），只支持一种更新类型，并且需要一个合法的图的颜色。然而，它的好处是比其他更多通用的锁引擎有更小的局限性，因此它是默认的引擎。

####**Locking Engine**
加锁引擎是从共享内存到分布式设置的广义化实现，它充分利用锁来保证一致性，并且使用了大量的线性方式来保证大量的操作在任何时候都正常，它可以和一系列共享内存调度器配合工作。
加锁引擎更普遍并且操作几乎和共享内存引擎等价。然而，复杂的实现导致了一些效率的损失。
#####**Engine Options**
加锁引擎有一个参数:max_deferred_task_per_node，它在锁管道里保持运行状态的最大更新任务数，默认值是1000，可以通过命令行设置：

<!--lang:c -->
```
--engine=dist_locking(max_deferred_tasks_per_node=10)"
```

#####**Incomplete[未完成] Implementations**
distributed_glshared_const 还没有实现。用户可以仿照这个简单定义一个普通全局变量，保证所有的程序在启动的时候设置这个全局变量的值。
你也可以选择使用distributed_glshared 。
引擎/核心的 sync_now()操作还没有实现。


