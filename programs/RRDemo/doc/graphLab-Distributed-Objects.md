graphLab 之 Distributed Objects
========
GraphLab提供了一个“分布式对象”系统来对分布式计算和存储提供简化的数据结构设计。
一个GraphLab的分布式对象是同时在所有的机器上都实例化的一个对象。这个对象内部包含一个dc_dist_object来提供分布式实例间的RPC通信。举个例子，我们用两台机器来运行下面程序：
<!--lang:c-->
```
... /* in main after initialization */ ...
graphlab::dht<std::string, std::string> str_map;
dc.barrier();

if (dc.procid() == 0) {
  str_map.set("hello", "world");
}
else if (dc.procid() == 1) {
  str_map.set("something", "other");
}
dc.barrier();
std::cout << str_map.get("hello").second;
std::cout << str_map.get("something").second;
```

DHT是一个提供键-值存储的分布式对象（分布式“Hash Table”）。每个入口都根据键值的hash值来存储在一台机器上，注意他们是同时在所有机器上创建的。创建后的barrier()保证了这个对象在使用前在所有的机器上已经合理的实例化了。
现在，初始化之后，dht的set()函数将键-值hash到正确的机器上执行。get()也是类似的。然而，就像例子中的分布式对象操作一样，很容易轻松的创建很多分布式对象。举个例子，下面的代码很容易创建50个不同的分布式键-值映射。当访问任意一台机器str_map[15]对应相同的DHT。
<!--lang:c-->
```
graphlab::dht<std::string, std::string> str_map[50];
dc.barrier();
```

###**Usage**
我们将用一个简单的分布式Hash Table来展示分布式对象系统的使用方法。注意这是非常简单的实现，因为它忽略了线程安全所以并不是完全正确的，但是它足够展示关键的概念。
<!--lang:c-->
```
class string_dht { 
 private:
  std::map<int, std::string> local_storage;
  mutable dc_dist_object<string_dht> rmi;
```

首先，每台机器需要局部数据存储。这里我们简单的使用std::map，关键提供分布式访问的是dc_dist_object<string_dht> rmi; ，这个对象创建一个远程函数调用的环境，允许正确的识别远程的实例。
我们看看string_dht构造函数，rmi构造函数需要一个distributed_control对象的引用，以及一个当前实例的指针：

<!--lang:c-->
```
public:
string_dht(distributed_control &dc): rmi(dc, this) {  }
```
 
现在，为了展示RMI对象怎么使用，我们看看set()函数：
<!--lang:c-->
```
void set(int key, const std::string &newval) {  
  procid_t owningmachine = key % rmi.numprocs();
  if (owningmachine == rmi.procid()) {
    local_storage[key] = newval;
  }
```

我们使用一个简单的hash函数来分辨键-值对存储在哪儿的。观察发现RMI对象和distributed_control对象提供了非常相似的功能，都有dc_dist_object::numprocs() 和dc_dist_object::procid()。如果数据存储在当前机器上，我们简单存储它，否则我们需要发送到远程机器上存储，下面是一个非常有趣的例子：
<!--lang:c-->
```
  else {
    rmi.remote_call(owningmachine,
                    &string_dht::set,
                    key,
                    newval);
  }
}
```

RMI对象支持和distributed_control  一样的调用/请求操作，但是它只支持成员函数指针操作。举个例子，我们将会调用与string_dht对象相匹配的机器上的set()成员函数（注意“&”符号是必须的，并且非常重要）。
get()函数也类似，但是我们必须远程请求：
<!--lang:c-->
```
std::string get(int key) {  
  procid_t owningmachine = key % rmi.numprocs();
  if (owningmachine == rmi.procid()) {
    return local_storage[key];
  }
  else {
    return rmi.remote_request(owningmachine,
                              &string_dht::get,
                              key);
  }
}
```

就像前面称述的一样，这种代码不应该被使用，因为它有很多限制，像local_storage对象是线程不安全的。因为将来的RPC调用通常是多线程的，锁是必须的。查看dht.hpp 得到等价的线程安全的DHT实例。

###**Context**
本质上，dc_dist_object对象和distributed_control对象支持相同的操作，但是严格限制“context”是单一对象实例。
它包含下面常规的调用操作：
<!--lang:c-->
```
•       remote_call
•       fast_remote_call
•       control_call
•       remote_request
•       fast_remote_request
•       control_request
```
另外，context完全与distributed_control对象独立，允许它自己的一系列集合操作：
<!--lang:c-->
```
dc_dist_object::broadcast, graphlab::dc_dist_object::barrier, 
graphlab::dc_dist_object::full_barrier 
```
等。

因为这些集合操作也是完全在实例化对象环境中操作，所以允许使用并行方式。举个例子，我可以有两个对象，每个对象内部都实例化线程去执行分布式计算，使用RMI对象执行收集操作是对象内可见的。
特别的，graphlab::dc_dist_object::full_barrier() 是值得注意的。
graphlab::distributed_control::full_barrier() 保证所有 RPC 包括分布式对象调用。它的屏障因此也是对于程序是全局可见的状态。graphlab::dc_dist_object::full_barrier() 只保证在所有对象实例中的RPC调用，它的屏障因此对分布式对象是局部的状态。这允许每个分布式对象运行自己的全屏障而不影响其他的分布式对象。

###**Final Notes**
最后，注意RMI对象只能调用成员函数指针，它不能调用其他全局函数（像printf()），全局环境可以通过dc_dist_object::dc()来访问并返回潜在的distributed_control对象，接下来可以通过它来调用全局函数。举例：
<!--lang:c-->
```
rmi.dc().remote_call(1, printf, "hello ");
```


