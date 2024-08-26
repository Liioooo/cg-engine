cmake_minimum_required(VERSION 3.22)

project(glad VERSION 0.1.36)
add_library(glad
        "include/glad/glad.h"
        "include/KHR/khrplatform.h"
        "src/glad.c"
        )

target_include_directories(glad PRIVATE "include")
