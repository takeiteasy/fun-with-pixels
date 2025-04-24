ifeq ($(OS),Windows_NT)
	include windows.Makefile
else
	UNAME:=$(shell uname -s)
	ifeq ($(UNAME),Darwin)
		include macos.Makefile
	else ifeq ($(UNAME),Linux)
		include linux.Makefile
	else
		$(error OS not supported by this Makefile)
	endif
endif

default: all

BUILD=build
CFLAGS = -Isrc -Ideps
LINK = src/rng.c -L$(BUILD) -lpb

$(BUILD)/:
	mkdir -p $(BUILD)

libpb: $(BUILD)/
	$(CC) -shared -fpic $(CFLAGS) $(SYSFLAGS) src/$(BACKEND).c -o $(BUILD)/libpb.$(LIBEXT)

program: libpb
	$(CC) $(CFLAGS) src/fwp.c $(LINK) -o $(BUILD)/fwp$(PROGEXT)

test-web: libpb
	emcc $(CFLAGS) src/pb_emscripten.c templates/pb_boilerplate.c -o $(BUILD)/fwp_web.html

SRC := scenes
BIN := build
TARGETS := $(foreach file,$(foreach src,$(wildcard $(SRC)/*.c),$(notdir $(src))),$(patsubst %.c,$(BIN)/%.$(LIBEXT),$(file)))

.PHONY: FORCE scenes

FORCE: ;

$(BIN)/%.$(LIBEXT): $(SRC)/%.c FORCE | $(BIN)
	$(CC) -shared -fpic $(CFLAGS) $(LINK) -o $@ $<

scenes: $(TARGETS)

all: program scenes

clean:
	$(RM) -rf $(BUILD)

.PHONY: program libpb clean
