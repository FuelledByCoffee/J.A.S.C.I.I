CPPFLAGS += -isystem external
CFLAGS   += -Wall -Wextra -Wshadow
LDFLAGS  += -lm

sources = $(wildcard *.c)
objects = $(sources:=.o)

config ?= debug

ifneq ($(findstring emcc,$(CC)),)
	target = a.out.js
	config = release
	LDFLAGS += -O3 \
         -sINVOKE_RUN=0 \
         -sEXIT_RUNTIME=0 \
         -sALLOW_MEMORY_GROWTH=1 \
         -sEXPORTED_RUNTIME_METHODS=FS,callMain,cwrap
else
	target = asciiart
endif

.PHONY: all
all: $(config)

.PHONY: debug
debug: CFLAGS += -g -fsanitize=undefined,address
debug: $(target)

.PHONY: release
release: CFLAGS += -O2
release: CPPFLAGS += -DNDEBUG
release: $(target)

$(target): $(objects)
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
	rm -f $(target)
	rm -rf $(objects)
