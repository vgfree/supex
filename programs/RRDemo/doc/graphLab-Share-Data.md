graphLab 之 Share Data
=======
共享数据系统提供了线程安全访问全局变量的控制机制。它主要提供了两种功能：
全局共享变量：允许所有的graphLab计算有控制的访问“全局变量”。实际上共享内存并行机制可以说是累赘的。然后，这里提供的抽象扩展到分布式内存机制。
同步：一个后台执行所有顶点数据的累计的reduce框架。
[detailed example](http://www.select.cs.cmu.edu/code/graphlab/doxygen/html/detailed_example.html) 提供了一个很好的使用例子。

###**Globally Shared Variables**
所有的共享数据必须用下面的语法定义成全局变量：
<!--lang:c-->
```
gl::glshared<T> var;
```
T是变量的数据类型。任意数据类型都可以保证线程安全。
通过值读取变量的值：
<!--lang:c-->
```
T val = var.get_val();
```
通过引用读取变量的值：
<!--lang:c-->
```
boost::shared_ptr<T> ptr = var.get_ptr();
```

get_ptr()函数返回数据的指针。boost::shared_ptr格式的指针可以像一般的指针一样用过“*”操作符来引用。然而指针仍旧在范围内，保证从指针中读取的值是没变化的。在特殊情况下，控制共享指针可以阻止写变量。指针因此需要尽快的移除范围或者调用ptr.release()释放。
通过使用set_val()函数修改变量，这是非常明显的。
<!--lang:c-->
```
var.set_val(T newval);
```

共享数据类型也提供了原子操作。
###**原子操作**
除了常规的get/set函数，共享变量也提供了两种基本的原子操作：
<!--lang:c-->
```
T oldval = glshared<T>::exchange(T newval);
```
写入新的值的同时拷贝并返回之前的值。
<!--lang:c-->
```
void glshared<T>::apply(apply_function_type fun, const any& closure);
```
通过引用调用fun()函数的共享变量。函数类型：
<!--lang:c-->
```
void (*apply_function_type)(any& current_value, const any& closure);
```

不幸的是，由于当前设计的限制，current_value必须用any类型来传入。然而，值的类型在any容器中是原始的共享变量类型。坦白的说，如果共享变量如下声明：
<!--lang:c-->
```
gl::glshared<size_t> shared_counts;
```
显然current_value可以通过graphlab::any::as()来访问，并且传入给as()函数的类型和共享变量的类型匹配。apply()函数允许改变current_type，将来读取这个变量的时候会返回新的值。

###**同步**
递增的在图的所有顶点上执行同步函数并且将结果写入到关联的共享数据表的接口中。同步操作定义成一对函数。一个同步函数和一个应用函数。这个概念最好通过一个例子来说明。简单点，假设图的每个顶点是一个double类型：
<!--lang:c-->
```
typedef graphlab::graph<double , double> graph_type;
typedef graphlab::types<graph_type> gl;
```

我想要计算所有整数的L2 Norm(欧几里得距离，平方和的平方根)。我会定义一个共享变量来存储最终结果：
<!--lang:c-->
```
gl::glshared<double> l2norm_value;
```

以及下面的同步和应用函数：
<!--lang:c-->
```
void squared_sum_sync(gl::iscope& scope, graphlab::any& accumulator){
   accumulator.as<double>() += scope.const_vertex_data() * scope.const_vertex_data();
}

void square_root_apply(graphlab::any& current_data, 
const graphlab::any& new_data){
    current_data.as<double>() = sqrt(new_data.as<double>());
}
```

变量与sync/apply关联：
<!--lang:c-->
```
core.set_sync(l2norm_value,  //shared variable
            squared_sum_sync, //sync function
            square_root_apply, //apply function
            double(0.0), //initial sync value
             100);   //sync frequency
```

当求完值后，同步函数（squared_sum_sync）就会在scope范围内的所有顶点上轮流调用。累加器从一个函数传递到下一个函数调用，不断的累加值。squared_sum()函数传递共享数据表中的关联入口索引，以及数据表的引用和当前顶点将要计算的范围。
当所有的顶点都完成了，应用函数（square_root_apply）被应用在累加的结果上。new_data变量包含有累加器在累加阶段计算的最终值。另外current_data是共享变量l2norm_value的引用。这就是说：current_data的值与l2norm_value.get_val()的值相等。修改current_data将会作用到l2norm_value上。
创建同步可以使用引擎中的graphlab::iengine::set_sync()成员函数或者核心中的graphlab::core::set_sync()。

###**common Syncs and Applyies**

一系列普通的同步和应用操作已经在gl::glshared_sync_ops 和 gl::glshared_apply_ops。使用glshared_sync_ops，你必须提供一个AccumulationType类型的访问器（const vertex_data& v）。
接下来你可以充分利用下面的同步函数：
<!--lang:c-->
```
graphlab::glshared_sync_ops::sum  用访问器函数读取顶点数据然后求所有顶点的和。
graphlab::glshared_sync_ops::l1_sum 用访问器求所有顶点的绝对值之和。
graphlab::glshared_sync_ops::l2_sum 用访问器求所有顶点数据的平方和。
graphlab::glshared_sync_ops::max    计算所有顶点数据的最大值。
graphlab::glshared_apply_ops::identity 将累加的结果存入共享变量。
graphlab::glshared_apply_ops::identity_print 将累加的结果存入共享数据入口并打印屏幕。
graphlab::glshared_apply_ops::increment  将最终累加的结果与关联的共享变量相加。
graphlab::glshared_apply_ops::decrement 从关联的共享变量中减掉最终累加的结果。
graphlab::glshared_apply_ops::sqrt  将累加结果的平方根存入共享变量。
```
下面的代码创建一个同步函数作为例子：
<!--lang:c-->
```
double accessor(const double& v)
{
    return v;
}
shared_data.set_sync(l2norm_value,
                  gl::shared_sync_ops::l2sum<double, accessor>,
                   gl::shared_apply_ops::sqrt<double>,
                   double(0.0),
100);
```

###**合并**
上面描述的同步操作不能并行化处理。为了允许并行化，又定义了一个额外的同步合并结果的合并函数。
如果定义了合并函数，顶点集合会被分割成互不相交的集合。同步函数会在每个集合上并行的执行，产生一系列的部分结果。这些部分结果会通过合并函数进行合并。最终，应用函数在最后的结果上执行并写入到共享变量中。
简单的说，在前面的同步实例中图的顶点数据L2 Norm已经计算完了，接下来结果可以通过如下方式合并：
<!--lang:c-->
```
static void sum_merge(any& dest,
                  const any& src){
      dest.as<double> += src.as<double>();
}
```

集合也可以的进行更新：
<!--lang:c-->
```
core.set_sync(l2norm_value,  //shared variable
            squared_sum_sync,   //sync function
            squared_root_apply,  //apply function
           double(0.0),       //initial sync value
           100,            //sync frequency
           sum_merge);   //merge function
```

类似的，一系列的普通合并函数也在gl::glshared_merge_ops中提供了：
<!--lang:c-->
```
graphlab::glshared_merge_ops::sum 合并中间结果。
graphlab::glshared_merge_ops::l1_sum  合并中间结果的绝对值。
graphlab::glshared_merge_ops::l2_sum 合并中间结果的平方和。
graphlab::glshared_merge_ops::max  返回中间结果的最大值。
```

实例：
<!--lang:c-->
```
share_data.set_sync(l2norm_value,
                 gl::glshared_sync_ops::l2sum<double, accessor>,
                 gl::shared_apply_ops::sqrt<double>,
                 double(0.0),
                 gl::glshared_merge_ops::sum<double>);
```



