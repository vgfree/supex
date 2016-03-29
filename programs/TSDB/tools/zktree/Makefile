TARGET=zktree

all: $(TARGET)
	-@echo "all done!"

$(TARGET): zktree.c
	gcc -O2 -I../../lib/zookeeper-3.4.6/src/c/include -I../../lib/zookeeper-3.4.6/src/c/generated zktree.c -L../../lib -lzookeeper_mt -pthread -lm -o zktree

clean:
	-@rm -rf zktree
	-@echo "clean done!"
