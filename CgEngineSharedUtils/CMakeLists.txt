cmake_minimum_required(VERSION 3.24)

project(CG_ENGINE_SHARED_UTILS VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE Header_Files "src/*.h")
source_group("Header Files" FILES ${Header_Files})
file(GLOB_RECURSE Source_Files "src/*cpp")
source_group("Source Files" FILES ${Source_Files})
set(All_Sources ${Header_Files} ${Source_Files})
message(STATUS "All_Sources_CG_ENGINE_SHARED_UTILS: ${All_Sources}")

add_library(${PROJECT_NAME} STATIC ${All_Sources})

################################################################################
# Compile definitions
################################################################################
if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_WIN32)
endif ()
if (UNIX)
    target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_UNIX)
endif ()

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src")

target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
