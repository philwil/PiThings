COMMENT ?=no comment

all: pithings lib main

pithings: pithings.c
	cc -g pithings.c -o pithings

LIBOBJS = ../lib/space.c
LIBOBJS += ../lib/cmds.c
LIBOBJS += ../lib/hands.c
LIBOBJS += ../lib/iosock.c
LIBOBJS += ../lib/iobuf.c
LIBOBJS += ../lib/node.c
LIBOBJS += ../lib/list.c
LIBOBJS += ../lib/group.c

SRCOBJS = ../src/main.c
SRCOBJS += ../src/tests.c

lib:
	gcc -Wall -Werror -fpic -shared -o ../lib/libthings.so $(LIBOBJS)
	#gcc -Werror -fpic -shared -o ../lib/libthings.so $(LIBOBJS)

main: lib
	gcc -Wall  -o main $(SRCOBJS) -L ../lib -lthings $(LIBOBJS)
#pithings.o: pithings.c

clean:
	rm -f ../src/*.o ../lib/*.o
	rm -f pithings

git:
	git add runme *.c ../inc/*.h ../lib/*.c Makefile
	git commit -m "$(COMMENT)"
	git push
