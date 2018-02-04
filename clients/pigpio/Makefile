COMMENT ?=no comment

all: lib main

BASEDIR = ../..

LIBOBJS = $(BASEDIR)/lib/space.c
LIBOBJS += $(BASEDIR)/lib/cmds.c
LIBOBJS += $(BASEDIR)/lib/hands.c
LIBOBJS += $(BASEDIR)/lib/iosock.c
LIBOBJS += $(BASEDIR)/lib/iobuf.c
LIBOBJS += $(BASEDIR)/lib/node.c
LIBOBJS += $(BASEDIR)/lib/list.c
LIBOBJS += $(BASEDIR)/lib/group.c
LIBOBJS += $(BASEDIR)/lib/hvals.c

SRCOBJS = $(BASEDIR)/clients/pigpio/main.c
#SRCOBJS += ../src/tests.c

lib:
	gcc -g -Wall -Werror -fpic -shared -o $(BASEDIR)/lib/libthings.so $(LIBOBJS)
	#gcc -Werror -fpic -shared -o ../lib/libthings.so $(LIBOBJS)

main: lib
	gcc -g -Wall  -o main $(SRCOBJS) -I$(BASEDIR)/src -L $(BASEDIR)/lib -lthings
#$(LIBOBJS)
#pithings.o: pithings.c

clean:
	rm -f $(BASEDIR)/clients/pigpio/*.o $(BASEDIR)/lib/*.o
	rm -f pithings

git:
	git add runme *.c $(BASEDIR)/inc/*.h $(BASEDIR)/lib/*.c $(BASEDIR)/clients Makefile
	git commit -m "$(COMMENT)"
	git push
