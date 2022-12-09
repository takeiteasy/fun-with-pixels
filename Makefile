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
		LIBS=-lX11 -lm
	else
		$(error OS not supported by this Makefile)
	endif
endif
SOURCES=$(wildcard src/*.c)

CC=clang

default: game
all: run

generate: $(SOURCES)
	ruby tools/generate.rb

library:
	$(CC) -shared -fpic $(LIBS) pp.c -o build/libpp.$(LIBEXT)

live: game
	$(CC) pp.c $(LIBS) -DPP_LIVE -o build/pp$(PROGEXT)

game:
	clang pp.c examples/live.c -shared -fpic $(LIBS) -I. -DPP_LIVE_LIBRARY -o build/libpplive.$(LIBEXT)

run: game live
	./build/pp$(PROGEXT) -p build/libpplive.$(LIBEXT) &

kill:
	pkill pp

web:
	emcc pp.c examples/basic.c -I. -o build/pp.html

webrun: web
	emrun build/pp.html

.PHONY: default all run game live library rebuild kill web webrun
