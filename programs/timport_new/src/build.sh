# !/bin/sh
#gcc dispatch_data.c -lluajit-5.1
#gcc get_user_key.c common.c -lluajit-5.1
gcc get_start_time.c common.c -lluajit-5.1
mv a.out ../
