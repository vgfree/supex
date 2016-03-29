all : sample sample1

sample:
	gcc -g sample.c  ../../lib/http-parser/http_parser.c -I../../lib/http-parser/ -o sample

sample1:
	gcc -g sample1.c  ../../lib/http-parser/http_parser.c -I../../lib/http-parser/ -o sample1

clean:
	rm -f sample sample1
