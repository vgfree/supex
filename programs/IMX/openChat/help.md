 strace -f  -e socket,listen,epoll_ctl,epoll_wait -o 11111111111111111 ./openChat
strace -fp 3093  -e epoll_ctl,epoll_wait -o 11111111111111111
