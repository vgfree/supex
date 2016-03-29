graphLab 之 RPC的基本用法
======
一旦distributed_control启动，它就可以用来调用远程的机器。前面已经举过例：
<!-- lang:c-->
```
if (dc.procid() == 0) {
  dc.remote_call(1, printf, "%d + %f = %s\n", 1, 2.0, "three");
}
```

异步的在机器0上调用机器1上的printf()函数。
在graphLab RPC的术语中，“调用”是指远程函数调用，“请求”是指有返回值的函数调用。“调用”是异步的并且会及时返回，“请求”会等待远程机器上的函数执行完成。
下面举个例子，机器1会打印“hello world”或者“world hello”：
<!-- lang:c-->
```
if (dc.procid() == 0) {
  dc.remote_call(1, printf, "hello ");
  dc.remote_call(1, printf, "world ");
}
```
远程调用立即完成，不管其他地方的函数调用花费多长时间。举个例子，0号处理器执行下面的代码可能不消耗执行时间：
```
if (dc.procid() == 0) {
  dc.remote_call(1, sleep, 1);
}
```
但是请求需要等待完成并且回复，这可能需要花费1秒钟。
```
if (dc.procid() == 0) {
  dc.remote_request(1, sleep, 1);
}
```
所有的参数和返回值都通过值传递，所有的参数类型和返回值类型都可以使用，只要他们是可序列化的。
###**分布式控制RPC**
graphlab::distributed_control 是基本的分布式RPC对象。这个类初始化分布式通信，以及基本的RPC程序和集体操作。
除了文档中列举的函数，也提供了下面的RPC程序。
`void distributed_control::remote_call(procid_t targetmachine, function, ...)`
`remote_call`以非阻塞的RPC方式运行远端目标机器上提供的函数指针。所有的参数都需要转化成值并且序列化。
**`targetmachine`**  程序运行的机器ID
**`function`**  目标机器上运行的函数
**`void distributed_control::fast_remote_call(procid_t targetmachine, function, ...)`**
`fast_remote_call`和`remote_call`是一样的，但是接收者是用接收线程来执行函数而不是用线程池。因此它只能被用作短小无阻塞的函数。
**`void distributed_control::control_call(procid_t targetmachine, function, ...)`**
和`remote_call`一样，程序使用`control_call`不影响调用程序并且对
`graphlab::async_consensus` 无影响。
**` RetType distributed_control::remote_request(procid_t targetmachine, 
function, ...) `**
`remote_request`  在远程机器上执行阻塞的RPC函数指针调用。所有的参数必须被转换成值并且序列化。只有目标机器完成函数调用才返回结果，返回值序列化后返回。
**`RetType distributed_control::fast_remote_request(procid_t targetmachine,
function, ...)`**
`fast_remote_request`  和`remote_request`一样，但是接收者使用接收线程完成函数调用而不是线程池。只能使用于短小无阻塞的函数。
**`void distributed_control::control_request(procid_t targetmachine, function, ...)`**
和`remote_request`相同，但是使用`control_request`对调用程序和
 `graphlab::async_consensus` 没有影响。

###**聚合操作**
除了常规的RPC操作，也提供了一系列类MPI的聚合操作。聚合操作是需要所有机器在继续执行程序前要执行的相同的函数。
###**Barrier**
一个最有用的操作是graphlab::distributed_control::barrier()，barrier()的功能和MPI_Barrier()的功能等价。它需要调用恢复前所有的机器都能执行到屏障。下面举个例子， 当0号处理器忙于计算π的时候，所有其他的处理器都将在屏障处等待0号处理器计算完成并到达屏障处，才开始执行下面的程序。
<!-- lang:c-->
```
if (dc.procid() == 0) {
  compute Pi to 1 million digits
}
dc.barrier();
```

###**Full Barrier**
全屏障也是通过`distributed_control::full_barrier()` 提供的。它保证所有的RPC操作必须都在屏障之前完成发送。
举个例子，全屏障保证调用 `set_a_to_1()` 必须在所有机器允许继续执行前完成。所有机器因此会打印‘1’：
<!-- lang:c-->
```
int a = 0;
void set_a_to_1() { a = 1; }

... /* in main after initialization */ ...
dc.remote_call( /* another machine */, set_a_to_1);
dc.full_barrier();
std::cout << a;
```

`full_barrier`大约是普通屏障2-3倍的消耗，所以应该慎重使用。

###**Other Collectives**
除了屏障和全屏障，也提供了像广播、集合和all_gather功能。注意这些操作的实现并不是像朴素的MPI实现一样特别高效，因为使用了过分简单化的算法。

###**Sequentialization**
GraphLab RPC有个稍微不同的特色是它可以对一个RPC调用序列实施顺序化，这在RPC库中尤其是异步操作是特别有用的，并且很多情况下能够简化代码。
举个例子：
<!-- lang:c-->
```
int a = 0;
void set_a_to_1() { a = 1; }
void print_a() { std::cout << a; }

... /* in main after initialization */ ...
targetmachine = (dc.procid() + 1) % dc.numprocs();
dc.remote_call(targetmachine, set_a_to_1);
dc.remote_call(targetmachine, print_a);
...
```

注意到由于异步自然的remote_call，很可能目标机在将变量a设置成1之前print_a()已经完成操作。因此，很可能输出是’0’。
一个可行的解决方案是将remote_calls改成remote_erquests。然而request会因为等待回复而带来大量的性能损失。所以我们选择使用顺序键系统：
<!-- lang:c-->
```
char oldkey = distributed_control::new_sequentialization_key();

dc.remote_call(targetmachine, set_a_to_1);
dc.remote_call(targetmachine, print_a);

distributed_control::set_sequentialization_key(oldkey);
```
它强制调用/请求在远程机器的线程池中的同一个线程上总是在执行前被设置，保证set_a_to_1和print_a调用的顺序。
顺序键在每个局部线程中是唯一的，以保证RPC在每个线程中调用的顺序对其他的线程没有影响。
键无需通过distributed_control::new_sequentialization_key() 创建。
distributed_control::set_sequentialization_key()也可以使用，只需给键任意一个非零值即可。
<!-- lang:c-->
```
char oldkey = distributed_control::set_sequentialization_key(123);

dc.remote_call(targetmachine, set_a_to_1);
dc.remote_call(targetmachine, print_a);

distributed_control::set_sequentialization_key(oldkey);
```

本来所有的RPC调用使用键值会有顺序，但是，自从**new_sequentialization_key()** 使用了一个非常简单的键选择系统，我们推荐使用 **set_sequentialization_key()** ，尤其是在多线程代码中。



