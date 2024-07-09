ifeq ($(OS),Windows_NT)
	LIBEXT=dll
	PROGEXT=.exe
	SYSFLAGS=-lgdi32
	BACKEND=pb_winapi
else
	UNAME:=$(shell uname -s)
	PROGEXT=
	ifeq ($(UNAME),Darwin)
		LIBEXT=dylib
		SYSFLAGS=-framework Cocoa
		BACKEND=pb_cocoa
	else ifeq ($(UNAME),Linux)
		LIBEXT=so
		SYSFLAGS=-lX11 -lm
		BACKEND=pb_x11
	else
		$(error OS not supported by this Makefile)
	endif
endif

.PHONY: default all library web

default: all

BUILD=build
LIBSRC = src/$(BACKEND).c src/rng.c
CFLAGS = -Isrc -Ideps
LINK = -L$(BUILD) -lpb

$(BUILD)/:
	mkdir -p $(BUILD)
	
libpb: $(BUILD)/
	$(CC) -shared -fpic $(CFLAGS) $(SYSFLAGS) $(LIBSRC) -o $(BUILD)/libpb.$(LIBEXT)

fwp: libpb
	$(CC) $(CFLAGS) src/fwp.c $(LINK) -o $(BUILD)/fwp$(PROGEXT)

test-lib: libpb
	$(CC) $(CFLAGS) templates/pb_boilerplate.c $(LINK) -o $(BUILD)/test_lib$(PROGEXT)

test-scene: libpb
	$(CC) $(CFLAGS) -shared -fpic templates/fwp_boilerplate.c $(LINK) -o $(BUILD)/test_scene.$(LIBEXT)

rebuild-test:
	$(CC) $(CFLAGS) -shared -fpic examples/basic.c $(LINK) -o $(BUILD)/rebuild_test.$(LIBEXT)

test-web: libpb
	emcc $(CFLAGS) src/pb_emscripten.c templates/pb_boilerplate.c -o $(BUILD)/fwp_web.html

tests: test-lib test-scene test-web

SRC := examples
BIN := build
TARGETS := $(foreach file,$(foreach src,$(wildcard $(SRC)/*.c),$(notdir $(src))),$(patsubst %.c,$(BIN)/fwp_example_%.$(LIB_EXT),$(file)))

$(BIN)/fwp_example_%.$(LIB_EXT): $(SRC)/%.c | $(BIN)
	$(CC) -shared -fpic $(CFLAGS) $(LINK) -o $@ $^

examples: $(TARGETS)

all: fwp tests examples

clean:
	$(RM) -rf $(BUILD)

.PHONY: default all clean libpb fwp test-lib test-scene test-web tests examples
