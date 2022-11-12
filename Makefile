default:
	clang tests/main.c src/ppMac.c -Isrc -I. -Ilib/bitmap -framework Cocoa -o build/test

.PHONY: default
