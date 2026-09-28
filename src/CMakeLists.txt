cmake_minimum_required(VERSION 4.1)
project(asciiart)

add_executable(asciiart main.cpp stb_impl.c)
target_include_directories(asciiart PRIVATE external)
target_compile_features(asciiart PRIVATE cxx_std_26)
target_compile_options(asciiart PRIVATE -Wall -Wextra -Wshadow)

# Emscripten specific compiler/linker flags
if(EMSCRIPTEN)
    set_target_properties(asciiart PROPERTIES OUTPUT_NAME "a.out" SUFFIX ".js")
    target_link_options(
        asciiart
        PUBLIC
            -sINVOKE_RUN=0
            -sEXIT_RUNTIME=0
            -sALLOW_MEMORY_GROWTH=1
            -sEXPORTED_RUNTIME_METHODS=FS,callMain,cwrap,ccall
    )
endif()
