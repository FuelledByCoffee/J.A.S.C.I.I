CPPFLAGS += -isystem external
CFLAGS   += -Wall -Wextra -Wshadow
LDFLAGS  += -lm

sources = $(wildcard *.c)
objects = $(sources:=.o)

config ?= debug

.PHONY: all
all: $(config)

.PHONY: debug
debug: CFLAGS += -g -fsanitize=undefined,address
debug: ascii

.PHONY: release
release: CFLAGS += -O2
release: CPPFLAGS += -DNDEBUG
release: ascii

ascii: $(objects)
	$(LINK.c) $^ -o $@ $(LDLIBS)

%.c.o: %.c
	$(COMPILE.c) $< -o $@

.PHONY: test
test: all
	./ascii googoo.png
	./ascii horse.png
	./ascii blossom.png
	./ascii flower.png
	./ascii spider.png
	./ascii avatar.png

.PHONY: clean
clean:
	rm -f ascii
	rm -rf $(objects)
