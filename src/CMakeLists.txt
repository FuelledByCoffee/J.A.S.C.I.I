cmake_minimum_required(VERSION 4.1)
project(asciiart)

add_executable(asciiart main.cpp stb_impl.c)
target_include_directories(asciiart PRIVATE external)
target_compile_features(asciiart PRIVATE cxx_std_26)
target_compile_options(asciiart PRIVATE -Wall -Wextra -Wshadow)
