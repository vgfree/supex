graphLab 之 Spawning & Initialization
====
Spawning是在不同的机器上启动GraphLab RPC的过程。GraphLab RPC支持两种实例化方式：MPI或者rpcexec.py（tools/目录下的一个脚本）。强烈推荐使用MPI方式，尽管它需要所有机器共享访问普通文件系统。

###**通过MPI实例化**
GraphLab已经使用MPICH2测试通过，它也应该使用OpenMPI测试。使用MPICH2或者OpenMPI文档材料启动MPI以确保你可以运行基本的MPI测试程序。（MPICH2附有一个mpdringtest程序）
实例化GraphLab RPC程序不需要额外使用MPI进行配置。
GraphLab RPC程序需要用以下方式开始：
<!--lang:c-->
```
#include <graphlab/rpc/dc.hpp>
#include <graphlab/rpc/dc_init_from_mpi.hpp>

using namespace graphlab;

int main(int argc, char ** argv) {
  mpi_tools::init(argc, argv);

  dc_init_param param;
  // set additional param options here
  if (init_param_from_mpi(param) == false) {
    return 0;
  }
  distributed_control dc(param);
  ...
}
```
在这个例子中，init_param_from_mpi使用MPI环交换端口数并且启动RPC通信层。查看
 dc_init_param 得到额外详细的配置参数。

###**通过rpcexec.py实例化**
rpcexec.py提供了一种简单的方式来运行一系列的机器，通过SSH来通信。 rpcexec.py –help提供基本的帮助。
你需要先创建一个主机配置文件，里面包含host name 和IP地址：
<!--lang:c-->
```
localhost
192.168.1.5
node2
node3
localhost
192.168.1.5
node2
node3
```

运行 rpcexec.py –n [启动的数量] –f [主机文件列表] `命令` 就会读取hostsfile中的前N个主机并执行命令。举个例子：
<!--lang:c-->
```
rpcexec.py –n 5 –f hostsfile ls
```
它将会在本地执行两次`ls`命令，在192.168.1.5，node2,node3这三台机器上分别执行一次。
rpcexec.py 也支持屏幕模式（GNU 屏）。运行：
<!--lang:c-->
```
rpcexec.py –s lsscreen –n 3 –f hostsfile ls
```
将会创建三个屏幕会话窗口，一个显示本地的`ls`， 其他的两个窗口分别登陆到192.168.1.5和node2上然后在上面分别执行`ls`命令。
rpcexec.py 创建完屏幕会话后会立即终止。
<!--lang:c-->
```
screen –r lsscreen
```
这个命令将显示并回复屏幕会话。

如果使用rpcexec.py来实例化程序，GraphLab RPC程序启动方式：
<!--lang:c-->
```
#include <graphlab/rpc/dc.hpp>
#include <graphlab/rpc/dc_init_from_env.hpp>

using namespace graphlab;

int main(int argc, char ** argv) {
  dc_init_param param;
  // set additional param options here
  if (init_param_from_env(param) == false) {
    return 0;
  }
  distributed_control dc(param);
  ...
}
```

因为不像MPI实例化，机器间没有通信端口。rpcexec.py因此使用环境变量来给GraphLab RPC程序传递信息。使用以下两个环境变量：
**SPAWNNODES** 用逗号分隔的分布式环境下的主机名称。
**SPAWNID**  SPAWNNODES中当前机器的索引。 第一台机器的索引值为0，机器将会监听10000+SPAWNID端口。
查看 dc_init_param  获得详细的附加配置参数。
这个实例化系统因为固定的端口号而缺乏灵活性。举个例子，当一个进程挂掉之后，端口会保持**TIMED_WAIT**分钟，阻止下一个RPC进程的启动。这也阻止了同一集合中机器上的GraphLab RPC程序的启动。
因此，MPI实例化是推荐使用的方式来启动RPC系统。


