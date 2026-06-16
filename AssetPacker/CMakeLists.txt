cmake_minimum_required(VERSION 3.23)
include(../CMakeAssetHandling.cmake)

project(AssetPacker VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE Header_Files "src/*.h")
source_group("Header Files" FILES ${Header_Files})
file(GLOB_RECURSE Source_Files "src/*cpp")
source_group("Source Files" FILES ${Source_Files})
set(All_Sources ${Header_Files} ${Source_Files})
message(STATUS "All_Sources_AssetPacker: ${All_Sources}")

add_executable(${PROJECT_NAME} ${All_Sources})

################################################################################
# Compile definitions
################################################################################
if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_WIN32)
endif ()
if (UNIX)
    target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_UNIX)
endif ()

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src")

target_link_libraries(${PROJECT_NAME} PRIVATE inih)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
target_link_libraries(${PROJECT_NAME} PRIVATE assimp)
target_link_libraries(${PROJECT_NAME} PRIVATE pugixml)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog)
target_link_libraries(${PROJECT_NAME} PRIVATE CG_ENGINE_SHARED_UTILS)

if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            COMMAND_EXPAND_LISTS
    )
endif()
