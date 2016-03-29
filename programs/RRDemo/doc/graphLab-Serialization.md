graphLab 之 Serialization
======
我们设计了一个定制的序列化方案来提高性能而不是通用性的。它不执行类型检测，它不执行指针跟踪，并且只对跨平台支持做了限制。经过测试，它仅兼容X86平台。
有两个序列化的类：graphlab::oarchive 和graphlab::iarchive。前者负责输出，后者负责输入。为了包含所有序列化头，需要include <graphlab/serialization/serialization_includes.hpp> 。
###**基本的序列化/反序列化**
为了将数据序列化到磁盘，你只需要创建一个输出存档，然后将它与一个文件流关联即可。
<!--lang:c-->
```
std::ofstream fout(“file.bin”, std::fstream::binary);
Graphlab::oarchive oarc(fout);
```

流操作符接下来会将数据写入存档。
<!--lang:c-->
```
int I = 10;
double j = 20;
std::vector v(10, 1.0);
oarc << i << j << v;
```
读回数据，只需将输入存档与输入文件流关联，然后按照相同的顺序读取变量即可：
<!--lang:c-->
```
std::ifstream fin(“fin.bin”, std::fstream::binary);
graphlab::iarchive iarc(fout);
int i;
double j;
std::vector v;
iarc >> i >> j >> v;
```

所有的基本数据类型都支持。四大STL容器：std::map, std::set, std::list 和std::vector也是支持的，只要容器包含的类型是可序列化的。也就是说，它可以正确序列化一个vector类型中包含的集合类型中的整型。
###**用户自定义结构体和类**
序列化一个结构体或者类，你只需要定义一个共有的 load()/save()函数即可：
<!--lang:c-->
```
class TestClass{
   public:
   int I, j;
   std::vector<int> k;
   
   void save(oarchive& oarc) const{
      oarc << i << j << k;
}
void load(iarchive& iarc) {
    iarc >> I >> j >> k;
}
}
```

之后，上面段落描述的标准流操作符就可以正常工作。 TestClass STL容器也可以很好的工作。

###**POD(Plain Old Data)types**
POD类型是在内存中占据了一段连续区域的数据类型。坦白的说，基本类型（double , int 等等），或者结构体类型都只包含基本类型。这种类型可以用一个简单的内存拷贝操作来拷贝或者复制数据，并且是序列化、反序列化期间非常推荐的加速方式。
不幸的是，并没有一种简单的方式去检测或者测试任何一个给定的类型是不是POD。C++ TR1定义了一个is_pod特性，但是这种特性已经在部分编译器中实现。因此我们定义了自己的 gl_is_pod< … >特性来让用户明确地指出一个特定的类型是POD类型。gl_is_pod 在将来编译器更好的支持std::is_pod的时候，可以进行深入扩展。
使用gl_is_pod, 我们假设如下的一个坐标结构体：
<!--lang:c-->
```
struct Coordinate{
   int x, y, z;
};
```

这个结构体可以使用一个加速序列化工具来定义成一个POD类型：
<!--lang:c-->
```
namespace graphlab{
  struct gl_is_pod<Coordinate> {
  BOOST_STATIC_CONSTANT(bool, value=true);
};
}
```

现在，Coordinate结构体变量，甚至是vector<Coordinate>变量 序列化、反序列化就会充分利用直接内存拷贝而更快。
特别说明的是这个快速序列化的方案可能因为它强制了特定整数类型在结构体中的字节长度而不能很好的跨平台。相同的存档在不同的编译器之间也可能存在争议。

###**Any**
graphlab::any 是源自Boost Any的变种，但是它扩展到可以支持graphLab的序列化系统。详细请参看：graphlab::any。


