OBJ = libscco.a \
      sample


all:$(OBJ)
	-@echo "OK!"

sample:scco_example.o libscco.a
	gcc $< -lscco -L. -o $@

libscco.a:scco.o
ifeq ($(SCCO_STACK_TYPE),)
	@echo -e "\x1B[0;31m\t""don't make at libscco directory!""\x1B[m"
else
	ar -rcs $@ $^
	nm -s $@ > /dev/null
endif

%.o:%.c
	#@if [ ! $(EXPORT_CFLAGS) ];then echo -e "\x1B[0;31m\t""don't make at here!""\x1B[m";else gcc $(EXPORT_CFLAGS) -c $< ; fi
	gcc $(EXPORT_CFLAGS) $(SCCO_STACK_TYPE) -c $<

clean:
	rm -rf *.o $(OBJ)

