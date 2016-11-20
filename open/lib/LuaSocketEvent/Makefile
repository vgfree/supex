
all:about
	gcc -fPIC --shared -o socketevent.so socketevent.c -lpthread `pkg-config  --cflags --libs luajit`


clean:
	rm -f socketevent.so

about:
	@echo -e "One connect use one pthread!\nIt's not good!"

