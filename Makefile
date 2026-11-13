CC      ?= gcc
CFLAGS  ?= -O2 -Wall -Wextra -std=gnu11
CFLAGS  += -Iinclude
LDLIBS  += -ldl

BUILD   := build
LIBS    := $(BUILD)/libaes_ttable.so $(BUILD)/libaes_ni.so
BINS    := $(BUILD)/test_aes $(BUILD)/bench $(BUILD)/victim

.PHONY: all test clean

all: $(LIBS) $(BINS)

src/aes_tables.c: tools/gen_tables.py
	python3 $< > $@

$(BUILD):
	mkdir -p $@

$(BUILD)/libaes_ttable.so: src/aes_ttable.c src/aes_tables.c include/aes.h | $(BUILD)
	$(CC) $(CFLAGS) -fPIC -shared -o $@ src/aes_ttable.c src/aes_tables.c

$(BUILD)/libaes_ni.so: src/aes_ni.c include/aes.h | $(BUILD)
	$(CC) $(CFLAGS) -maes -fPIC -shared -o $@ $<

$(BUILD)/test_aes: tests/test_aes.c src/aes_loader.c include/aes_loader.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ tests/test_aes.c src/aes_loader.c $(LDLIBS)

$(BUILD)/bench: tests/bench.c src/aes_loader.c include/aes_loader.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ tests/bench.c src/aes_loader.c $(LDLIBS)

$(BUILD)/victim: src/victim.c src/aes_loader.c include/aes_loader.h include/protocol.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ src/victim.c src/aes_loader.c $(LDLIBS)

test: $(LIBS) $(BUILD)/test_aes
	$(BUILD)/test_aes $(BUILD)/libaes_ni.so $(BUILD)/libaes_ttable.so

clean:
	rm -rf $(BUILD)
