WORK_PATH=$(shell dirname `pwd`)

head_path   = -I ${WORK_PATH}/../../lib/json-c  -I ${WORK_PATH}/src
linklibs    = -L ${WORK_PATH}/../../lib  -Wl,-dn -lev  -Wl,-dy -lpthread 
options     = -g -Wall -D_GNU_SOURCE

file = async_comm.o jtt_body.o jtt_client.o pkg_escape.o cq_list.o jtt_comm.o jtt_package.o log.o pkg_crc.o pkg_sec.o     
obj = $(addprefix ../bin_releaseServer/,$(file))

testServer: main.o $(obj)
	gcc $^ -o $@  ${options} ${linklibs}
	rm *.o

.c.o:
	gcc -c $< -o $@  ${options} ${head_path} -g


clean:
	rm -f *.o 

