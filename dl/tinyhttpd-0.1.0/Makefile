all: httpd showvars

httpd: httpd.c
	#gcc -W -Wall -lsocket -lpthread -o httpd httpd.c
	gcc -W -Wall -lrt -pthread -o httpd httpd.c

showvars: showvars.c
	#gcc -W -Wall -lsocket -lpthread -o httpd httpd.c
	gcc -W -Wall -lrt -pthread -o $@ $<
	cp $@ htdocs/showvars.cgi

clean:
	rm httpd
