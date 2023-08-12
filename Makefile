ifeq ($(OS),Windows_NT)
	LIBEXT=dll
	PROGEXT=.exe
	CFLAGS=-lgdi32
else
	UNAME:=$(shell uname -s)
	PROGEXT=
	ifeq ($(UNAME),Darwin)
		LIBEXT=dylib
		CFLAGS=-framework Cocoa
	else ifeq ($(UNAME),Linux)
		LIBEXT=so
		CFLAGS=-lX11 -lm
	else
		$(error OS not supported by this Makefile)
	endif
endif

default: library
	$(CC) -Lbuild -lpp basic.c -o build/pp

library:
	$(CC) $(CFLAGS) -Ibackends pp.c -shared -fpic -o build/libpp.$(LIBEXT)

web:
	emcc -Ibackends basic.c pp.c -o build/pp.html

.PHONY: default all library web
