ifeq ($(OS),Windows_NT)
	LIBEXT=dll
	PROGEXT=.exe
	LIBS=-lgdi32
else
	UNAME:=$(shell uname -s)
	PROGEXT=
	ifeq ($(UNAME),Darwin)
		LIBEXT=dylib
		LIBS=-framework Cocoa
	else ifeq ($(UNAME),Linux)
		LIBEXT=so
		LIBS=-lX11
	else
		$(error OS not supported by this Makefile)
	endif
endif
SOURCES=$(wildcard src/*.c)
SOURCE=pp.c
CC=clang

all: library
default: library

$(SOURCE): $(SOURCES)
	mkdir build/ || true
	ruby tools/generate.rb

library: $(SOURCE)
	$(CC) -shared -fpic $(LIBS) -Isrc/bitmap $^ -o build/libpp.$(LIBEXT)

.PHONY: default library all
