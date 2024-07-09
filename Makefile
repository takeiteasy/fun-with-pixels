ifeq ($(OS),Windows_NT)
	LIBEXT=dll
	PROGEXT=.exe
	CFLAGS=-lgdi32
	SRC=pb_winapi
else
	UNAME:=$(shell uname -s)
	PROGEXT=
	ifeq ($(UNAME),Darwin)
		LIBEXT=dylib
		CFLAGS=-framework Cocoa
		SRC=pb_cocoa
	else ifeq ($(UNAME),Linux)
		LIBEXT=so
		CFLAGS=-lX11 -lm
		SRC=pb_x11
	else
		$(error OS not supported by this Makefile)
	endif
endif

.PHONY: default all library web

default:
	$(CC) -Isrc src/fwp.c src/pb_cocoa.c src/rng.c -framework Cocoa -o test

all: default
