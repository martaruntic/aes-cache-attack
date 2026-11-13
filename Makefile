CC      ?= gcc
CFLAGS  ?= -O2 -Wall -Wextra -std=gnu11
CFLAGS  += -Iinclude
LDLIBS  += -ldl

BUILD   := build
LIBS    := $(BUILD)/libaes_ttable.so
BINS    := $(BUILD)/test_aes

.PHONY: all test clean

all: $(LIBS) $(BINS)

src/aes_tables.c: tools/gen_tables.py
	python3 $< > $@

$(BUILD):
	mkdir -p $@

$(BUILD)/libaes_ttable.so: src/aes_ttable.c src/aes_tables.c include/aes.h | $(BUILD)
	$(CC) $(CFLAGS) -fPIC -shared -o $@ src/aes_ttable.c src/aes_tables.c

$(BUILD)/test_aes: tests/test_aes.c src/aes_loader.c include/aes_loader.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ tests/test_aes.c src/aes_loader.c $(LDLIBS)

test: $(LIBS) $(BUILD)/test_aes
	$(BUILD)/test_aes $(BUILD)/libaes_ttable.so

clean:
	rm -rf $(BUILD)
