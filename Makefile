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

CC=clang

default: game
all: run

generate: $(SOURCES)
	mkdir build/ || true
	ruby tools/generate.rb
	headerdoc2html -udpb pp.h -o docs/
	gatherheaderdoc docs
	mv docs/masterTOC.html docs/index.html

library:
	$(CC) -shared -fpic $(LIBS) pp.c -o build/libpp.$(LIBEXT)

live: game
	$(CC) pp.c $(LIBS) -DPP_LIVE -o build/pp$(PROGEXT)

game:
	clang pp.c examples/live.c -shared -fpic $(LIBS) -I. -DPP_LIVE -o build/libpplive.$(LIBEXT)

run: game live
	./build/pp$(PROGEXT) -p build/libpplive.$(LIBEXT) &

kill:
	pkill pp

.PHONY: default all run game live library rebuild kill
