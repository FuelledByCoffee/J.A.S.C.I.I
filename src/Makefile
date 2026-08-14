CPPFLAGS += -isystem external
LDFLAGS  += -lm

ifeq ($(CC),clang)
	COMPILE.cc += -MJ $@.json
	COMPILE.c += -MJ $@.json
endif

warnings += -Wall -Wextra -Wshadow
CFLAGS   += $(warnings)
CXXFLAGS += $(warnings) -std=c++26

sources = $(wildcard *.c *.cpp)
objects = $(sources:=.o)
json    = $(objects:=.json)

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

ifeq ($(CC),clang)
all: compile_commands.json
endif

compile_commands.json: $(json)
	@sed -e '1s/^/[\n/' -e '$$s/,$$/\n]/' $^ > $@

.PHONY: debug
debug: CFLAGS += -g -fsanitize=undefined,address
debug: CXXFLAGS += -g -fsanitize=undefined,address
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
