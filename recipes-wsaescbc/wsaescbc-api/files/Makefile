SRCFILE := wsaescbc.c
OBJFILE := wsaescbc.o
LIBFILE := libwsaescbc.a
TESTFILE := wsaescbc_api_test.c
TESTEXEC := wsaescbc_api_test
all: lib test

# Shared Library
##lib:
##	gcc -Wall -g -fPIC -c -o $(OBJFILE) $(SRCFILE)
##	gcc -shared -o $(LIBFILE) $(OBJFILE)

# Static Library 
lib:
	gcc -Wall -g -c -o $(OBJFILE) $(SRCFILE) -fPIC
	ar -cvq $(LIBFILE) $(OBJFILE)

test: lib
	gcc -Wall -o wsaescbc_api_test $(TESTFILE) $(LIBFILE)

clean: 
	rm -f *.o *.so *.a $(TESTEXEC)
