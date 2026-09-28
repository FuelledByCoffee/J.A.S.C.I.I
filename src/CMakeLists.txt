cmake_minimum_required(VERSION 4.1)
project(asciiart)

add_executable(asciiart main.cpp stb_impl.c)
target_include_directories(asciiart PRIVATE external)
target_compile_features(asciiart PRIVATE cxx_std_26)
target_compile_options(asciiart PRIVATE -Wall -Wextra -Wshadow)

# Emscripten specific compiler/linker flags
if(EMSCRIPTEN)
    target_compile_options(my_project PRIVATE "-O3")

    # Target linker options for output format and features
    target_link_options(
        my_project
        PRIVATE
            "-sWASMDATASECTIONIFY=1"
            # Change suffix to .html if you want Emscripten to generate a default HTML page
            # Otherwise, it defaults to producing .js and .wasm files
            "-o"
            "${CMAKE_BINARY_DIR}/a.out.js"
    )
endif()
