CPPFLAGS += -isystem external
LDFLAGS  += -lm

warnings += -Wall -Wextra -Wshadow
CFLAGS   += $(warnings)
CXXFLAGS += $(warnings)

sources = $(wildcard *.c *.cpp)
objects = $(sources:=.o)

config ?= debug

ifneq ($(findstring emcc,$(CC)),)
	target = a.out.js
	config = release
	objects = $(sources:=.js.o)
	c_obj_pattern = %.c.js.o
	cpp_obj_pattern = %.cpp.js.o
	LDFLAGS += \
         -sINVOKE_RUN=0 \
         -sEXIT_RUNTIME=0 \
         -sALLOW_MEMORY_GROWTH=1 \
         -sEXPORTED_RUNTIME_METHODS=FS,callMain,cwrap,ccall
else
	target = asciiart
	c_obj_pattern = %.c.o
	cpp_obj_pattern = %.cpp.o
endif

.PHONY: all
all: $(config)

.PHONY: debug
debug: CFLAGS += -g -fsanitize=undefined,address -fno-sanitize=alignment
debug: CXXFLAGS += -g -fsanitize=undefined,address -fno-sanitize=alignment
debug: $(target)

.PHONY: release
release: CFLAGS += -O2
release: CXXFLAGS += -O2
release: CPPFLAGS += -DNDEBUG
release: $(target)

$(target): $(objects)
	$(LINK.cc) $^ -o $@ $(LDLIBS)

$(c_obj_pattern): %.c
	$(COMPILE.c) $< -o $@

$(cpp_obj_pattern): %.cpp
	$(COMPILE.cc) $< -o $@

.PHONY: test
test: all
	./$(target) -i ../assets/hornet.jpg
	./$(target) -i ../assets/hornet.jpg -c

.PHONY: clean
clean:
	rm -f $(target)
	rm -rf $(objects)
