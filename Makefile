ifeq ($(OS),Windows_NT)
	LIBEXT=dll
	PROGEXT=.exe
	CFLAGS=-lgdi32
	SRC=ppWindows
else
	UNAME:=$(shell uname -s)
	PROGEXT=
	ifeq ($(UNAME),Darwin)
		LIBEXT=dylib
		CFLAGS=-framework Cocoa
		SRC=ppMac
	else ifeq ($(UNAME),Linux)
		LIBEXT=so
		CFLAGS=-lX11 -lm
		SRC=ppLinux
	else
		$(error OS not supported by this Makefile)
	endif
endif

.PHONY: default all library web

default:
	$(CC) -Isrc src/$(SRC).c examples/basic.c $(CFLAGS) -o build/$(SRC)$(PROGEXT)

library:
	$(CC) -shared -fpic -Isrc src/$(SRC).c examples/basic.c $(CFLAGS) -o build/lib$(SRC).$(LIBEXT)

web:
	emcc -DPP_EMSCRIPTEN -Isrc src/ppEmscripten.c examples/basic.c -o build/pp.html

all: default library web
