CPPFLAGS += -isystem external
CFLAGS   += -Wall -Wextra -Wshadow
LDFLAGS  += -lm

sources = $(wildcard *.c)
objects = $(sources:=.o)

config ?= debug

ifneq ($(findstring emcc,$(CC)),)
	target = a.out.js
	config = release
	objects = $(sources:%.c=%.c.js.o)
	obj_pattern = %.c.js.o
	CFLAGS += -O3
	LDFLAGS += \
         -sINVOKE_RUN=0 \
         -sEXIT_RUNTIME=0 \
         -sALLOW_MEMORY_GROWTH=1 \
         -sEXPORTED_RUNTIME_METHODS=FS,callMain,cwrap,ccall
else
	target = asciiart
	obj_pattern = %.c.o
endif

.PHONY: all
all: $(config)

.PHONY: debug
debug: CFLAGS += -g -fsanitize=undefined,address -fno-sanitize=alignment
debug: $(target)

.PHONY: release
release: CFLAGS += -O2
release: CPPFLAGS += -DNDEBUG
release: $(target)

$(target): $(objects)
	$(LINK.c) $^ -o $@ $(LDLIBS)

$(objects): $(obj_pattern): %.c
	$(COMPILE.c) $< -o $@

.PHONY: test
test: all
	./$(target) -i ../assets/hornet.jpg
	./$(target) -i ../assets/hornet.jpg -c

.PHONY: clean
clean:
	rm -f $(target)
	rm -rf $(objects)
