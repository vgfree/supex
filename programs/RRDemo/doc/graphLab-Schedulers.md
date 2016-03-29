graphLab 之 Schedulers
====
GraphLab框架提供了一系列内置的静态调度器，同时也提供了一些外来的特殊的动态调度器。下面，我们简要的描述下这些调度器以及每个调度器的选项设置。
有很多种方法来提供参数给每个调度器：
**命令行参数**： 如果graphLab使用了command_line_options 解析器，很多的参数可以通过命令行像这样传递参数：`--scheduler schedtype[key=value,key=value...]`
调度器的帮助选项也可以通过—schedhelp 来获得。
**graphlab::core::sched_options()** ： 如果使用了`graphlab::core`方法，调度器的选项可以通过`core::sched_options()`方法调用
`core.sched_options().add_option(“optionname”, VALUE)` 来设置，其中VALUE可以是任何类型。（sched_options 会自动根据需要转换成正确的类型）。
###**静态调度器**
静态调度器忽略了add_task调用，典型的重复预定义的序列直到终止。
####**Chromatic Scheduler [chromatic_scheduler]**
着色调度器（Chromatic Scheduler）支持单一的更新函数，并且应用于整个图的遍历。着色器先通过graphlab::graph::color()函数从每一个顶点读取颜色。调度器会在所有相同的颜色上执行，然后执行下一种颜色。在同一颜色上，执行器可能会在同一时刻在多个顶点上执行更新函数。但是着色调度器保证了在任何时候不同颜色的顶点都不会同时执行。
着色调度器使用graphlab::graph::compute_coloring()函数先计算出当前模型的一个颜色来获得Gauss-Seidel并行执行连续的算法。只要更新函数不修改邻接顶点的数据，并行执行函数就会让连续执行函数的顺序与每个顶点颜色的顺序一致。
参数：
**“max_iterations”:[integer, default=0]**设置迭代次数。
**“update_function”[update_function_type, default=set_on_add_task]**设置顶点上调用的更新函数。因为这里调用的是函数，所以这个参数不能通过命令行传入，只能通过程序代码。如果这个参数没有设置，更新函数会被设置成最近add_task调用的那个函数。尽管add_task在调度中并不起作用，这里着色调度器用它作为默认的更新函数来使用。
####**Round Robin Scheduler[round_robin]**
这种调度器循环执行所有的顶点。add_task函数可以用来设置每个顶点执行不同的更新函数。
####**Set_scheduler[set]**
这个还在试验阶段，不应该使用。不是官方发布的。

###**动态调度器**
动态调度器在更新函数中依赖graphlab::icallback来接收新任务（以及潜在的优先级）。接下来这些任务并入计划执行任务。它们之所以被称作动态调度器是因为他们依靠更新函数中的计算结果来解决何时来执行什么。
所有的动态调度器都有内部去重机制来阻止同一任务在队列中出现多次。调度器使用优先级来得到所有重复入口的最大优先级并且在队列中分配给单一的实例。
Graphlab中有不少动态调度器，下面列举了一些最流行的几个：
####**FiFo Scheduler[fifo]**
队列调度器把任务放入队列来保证先进先出的执行次序，它适合相对轻量级的同步更新函数。
####**Multiqueue FiFo Scheduler[multiqueue_fifo]**
多队列调度器FIFO调度器一样使用多个单一队列（2倍的CPU核数）并且任务是通过随机均衡策略来加入到队列。每个处理器从它自己的队列获取任务。
####**Priority Scheduler[priority]**
优先级调度器维护一个优先调度队列。优先级越高的任务总是被最先执行。如果add_task()被卷入当前已经存在的任务的优先级，那么这个任务优先级会被设置成这两个任务中的最大一个。
####**Multiqueue Priority Scheduler[multiqueue_priority]**
和优先调度器相同，只不过是维护了多个队列。
####**Clustered Priority Scheduler[clustered_priority]**
集群优先级调度器维护一个多块顶点的优先队列。这个调度器首先用分割方法将图分成多个块，每个块包含vert_per_part顶点。优先级在多个块之间维护，但是每个块有一个扫描调度器执行。
参数：
**`partition_method[string: metis/random/bfs, default=metis]`**设置分块方法。详细介绍可参看graphlab::partition_method。
**`vertices_per_partiton[integer, default=100]`**每个分块中顶点的数量。
####**Sweep Scheduler[sweep]**
这种调度器循环遍历所有的顶点，如果顶点上绑定任务则执行。每个顶点维护它自己本地的队列。
参数：
**`“ordering”：[string: linear/permute, default=linear]`** 设置使用的顺序。
###**特殊调度器**
####**Splash Scheduler[splash]**
我们目前只对每个CPU提供小规模伸展树然后有序化执行更新的Splash调度器。
参数：
**`“splash_size”:[integer, default=100]`**伸展树中顶点的个数。
**`“update_function”[update_function_type, default=开启 add_task_to_all]`**。 Splash更新使用的函数。它也不能通过命令行传递。如果这个参数没有提供，那么更新函数会被设置成add_task_to_all()最近调用的那个函数。

