1, recv_message.
   现有的实现message_send_queue 中的comm_send(,fd, ) is ok.
   但comm_recv(,fd,), 表示有疑问， 业务逻辑层有太多的fd, 而不应该的去遍历fd而调用comm_recv接口吧， 
   如此太多的无用工， 影响性能。
   是不是message_recv_queue 是如下形式比较好， 
   ----------------
   | fd  | message|
   ----------------
   | fd  | message|
   ----------------

   是不是提供一个pop_message_from_recv_queue()比较好？

2, 是不是得加一个回调callback_for_accept(fd, ip) 专门传递客户端链上来的(ip 可以不用， 但是不是得注意防止同一个客户端多次连接本主机)?

3，是不是得加一个回调callback_for_closed(fd) 专门通知业务线程并同步哪个（fd）已经掉线？

4, 现在是不是在libcomm 中没有实现mfptp的验证， 得保证包的正确性和完整性？

5, 每次epoll_wait 之后对需要监听的事件， 还需要remove吗？


6, 根据第2,3点， 我们可不可以直接用一个callback, callback中有command参数， 由command 决定回调将会执行那些功能？

7, 如何最好性能的把每一个fd接收的数据解析存到 message queue, epoll_wait(timeout) 超时， 当IO很密集时， 可能会出现数据解析饿死的状况
  是不是可以有其他的schedule 方案， 比如(1,定时调用， 2,计数，接收一定event 后， 调用).

8, 接收message queue 是不是没必要每一个fd有一个， 只要所有的fd 共用一块接收message queue 就OK 了。
 
9, epoll_wait(timeout); timeout = -1 , 非阻塞就可以了， 用时间等待，看不出有什么好处？

10, 我认为外部打包， 我们可以用proto_buffer, 谷歌开源的消息打包协议， 里面再封装mfptp, 消息路由没必要对mfptp理解太透彻。 
