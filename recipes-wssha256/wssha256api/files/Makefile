SRCFILE := wssha.c
OBJFILE := wssha.o
LIBFILE := libwssha.so
TESTFILE := wsshatest.c
TESTEXEC := wsshatest

all: lib test

lib:
	gcc -Wall -g -fPIC -c -o $(OBJFILE) $(SRCFILE)
	gcc -shared -o $(LIBFILE) $(OBJFILE)

test: lib
	gcc -L$(shell pwd) -Wall -g $(TESTFILE) -o $(TESTEXEC) -lwssha

clean: 
	rm -f *.o *.so $(TESTEXEC)
