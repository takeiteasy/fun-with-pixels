ifeq ($(OS),Windows_NT)
	LIBEXT=dll
	PROGEXT=.exe
	LIBS=-lgdi32
else
	UNAME:=$(shell uname -s)
	PROGEXT=
	ifeq ($(UNAME),Darwin)
		LIBEXT=dylib
		LIBS=-framework Cocoa -pthread
	else ifeq ($(UNAME),Linux)
		LIBEXT=so
		LIBS=-lX11
	else
		$(error OS not supported by this Makefile)
	endif
endif
CFLAGS=-I../ -I../src/bitmap/ ../pp.c $(LIBS)
APP=app.c
PROGRAM=pp$(PROGEXT)
LIBRARY=libapp.$(LIBEXT)

program: ppLive.c
	$(CC) $(CFLAGS) -I../extra $^ -o $(PROGRAM)

library: program
	$(CC) $(CFLAGS) $(APP) -shared -fpic -o $(LIBRARY)

clean:
	rm $(PROGRAM) $(LIBRARY) || true

all: library

.PHONY: program library run all default
