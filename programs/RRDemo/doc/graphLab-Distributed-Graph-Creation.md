Distributed Graph Creation
===
分布式GraphLab的目标不仅是能够处理超出内存容量的图计算，而且比单机更能充分利用处理器资源。因为机器的数量可能会根据性能的要求而改变，图必须能够支持根据集群的数量变化动态加载数据图。为了解决这个问题，我们开发了基于两相分割计划的磁盘图。
###**Atom Partitioning**
最开始用户必须将图分割成比用来部署数据的最大集群更多的部分，每个部分称作一个原子，每个原子不仅包含这个部分中的顶点和边，而且还包含与这个部分直接相邻的顶点和边的信息（也被称作这个部分的镜像）。
一系列的原子组成一个完整的图叫做磁盘图，原子文件名的格式是“[basename].0”,“[basename].1”，“[basename].2”等等。每个原子文件存储的是一个二进制格式的[Kyoto Cabinet](http://fallabs.com/kyotocabinet/)哈希表，用户不应该让原子直接互相直接影响，格式在 graphlab::disk_atom 文档中已经声明。
一个合适的磁盘图应该包含一个原子索引文件，并且文件名是“[basename].idx”。原子索引是一个附加的人类可读/可修改的文本文件，它描述了图的基本信息（顶点数，边数），原子的相邻结构，最重要的，这些原子文件的文件路径或者URL。（当前只支持file://格式的URL协议，将来可能会增加其他协议支持）。
**Note**:
假如原子索引文件丢失，graphlab::disk_graph 将会通过构造函数来从磁盘图中加载数据而不需要原子索引，最后将会重新生成一个原子索引文件。

###**Graph Creation**
有三种不同的方式创建磁盘图，用于三种不同的情况：
Conversion of In Memory Graph To Disk Graph: 图足够小，完全可以在一台机器的内存中装下。这是用户测试分布式GraphLab实现的普遍情况，或者计算时间比图大小要大。
**Direct Creation of Disk Graph**: 可以在一台机器上创建的足够小的图，这种情况是介于上面和下面方法的情况。这个图足够小，以至于它能够在合理的时间内能够写入到一台机器的磁盘。
**Distributed Graph Creation**: 图太大而不能在一台机器上创建，所以需要在可接受的时间内分布式创建图，这是最复杂的情况，需要用户花费比上面两种更多的精力。

**Conversion of Memory Graph to Disk Graph**
这是最简单的一种情况，用户只需要使用graphlab::graph来实例化一个内存图，接下来可以用graphlab::graph中的任意一种划分方法来生成一个原子划分（atom partition）。调用graph::graph_partition_to_atomindex可以将图转换成磁盘图。

**Direct Creation of Disk Graph**
基于磁盘的GraphLab图包含顶点和边的类型模板。
这个类或者或少的都和graphlab::graph接口相同，只是有一些受限的不同。首先，用户会发现边的ID不再分配，这个设计是故意的，因为图中的边数比顶点数多，并且随着图大小的增长，通过单一的索引表来映射所有的数据表变的不切实际。替而代之，我们通过它的开始和结束顶点来自然的确定每一条边。
**Graph Creation**
顶点和边的添加可以使用 disk_graph::add_vertex() 和 graph::add_edge() 成员函数：
<!--lang:c-->
```
vertex_it disk_graph::add_vertex(const VertexData& vdata = VertexData()) 
  void disk_graph::add_edge(vertex_id_t source, vertex_id_t target, const EdgeData& edata = EdgeData()) 
```

而且添加顶点还有提示的版本，它允许用户提供使用哪个原子来存储顶点的信息。
vertex_it disk_graph::add_vertex(const VertexData& vdata, size_t locationhint)
然而，一次添加一个顶点或者一条边会很慢，创建顶点和边经常使用并行的创建函数。

<!--lang:c-->
```
Parallel Graph Creation
vertex_id_t add_vertex_collection(std::string collectionname,
std::vector<VertexData> & vdata)

vertex_id_t add_vertex_collection(std::string collectionname,
size_t numvertices,
boost::function<VertexData (vertex_id_t)> generator)
```

第一种格式是在集合中插入一系列的顶点，为集合分配一个名字并且从vdata vector中读取顶点数据，函数返回第一个插入的顶点索引。记住，顶点被分配了连续的顶点ID。
边的插入可以使用下面的函数：
<!--lang:c-->
```
 void add_edge_indirect(std::string collectionname,
                         boost::function<void (vertex_id_t,
                         const VertexData&,
                         std::vector<vertex_id_t>&,
                         std::vector<EdgeData>&,
                         std::vector<vertex_id_t>&, 
                         std::vector<EdgeData>&)> efn) 
```

函数原型看起来难懂实际上是非常简单的。
这个提供给用户的函数会在集合中的每个顶点上调用，传入顶点数据(vdata)以及顶点ID。函数接下来应该返回下面提到的参数：出度顶点集合以及它们对应的边数据，以及入度顶点和他们对应的边数据。
函数的模型：
<!--lang:c-->
```
void efn(vertex_id_t vid,          // input: vertex ID of the vertex being queried
            const VertexData& vdata,   // input: vertex data of the vertex being queried
            std::vector<vertex_id_t>& inv,    // output: Incoming vertices of the vertex being queried
            std::vector<EdgeData>& inedata,   // output: Data on the in-edges of the vertex. Paired with inv.
            std::vector<vertex_id_t>& outv,   // output: Outgoing vertices of the vertex being queried
            std::vector<EdgeData>& outedata); // output: Data on the out-edges of the vertex. Paired with outv.
```

####**Finalization**
finalize()函数完成磁盘IO同步以及重新生成原子索引文件，finalize()在disk_graph析构函数时自动调用。
####**Performance**
简单的综合标准是每秒钟插入100k到200k条边，顶点数据插入速率是每秒500k个顶点。

###**Distributed Graph Creation**
MapReduce类型的分布式构建方法由graphlab::mr_disk_graph_construction提供。最基本的要求是所有的机器都必须共享文件存储（NFS / SMBFS /等）。
用户需要先创建graphlab::igraph_constructor的子类，它对于图的顶点或边的任意子集提供了一套迭代机制，图构造函数的一些限制是必要的，因为它必须分布式可知（distributed-aware）。
之后，graphlab::mr_disk_graph_construction将会使用图的构造器执行高并行、分布式的磁盘的构建。
[distributed_dg_construction_test.cpp](http://www.select.cs.cmu.edu/code/graphlab/doxygen/html/distributed__dg__construction__test_8cpp_source.html) 是一个简单明了的例子。


