cmake_minimum_required(VERSION 3.23)
include(../CMakeAssetHandling.cmake)

project(GAME VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE Header_Files "src/*.h")
source_group("Header Files" FILES ${Header_Files})
file(GLOB_RECURSE Source_Files "src/*cpp")
source_group("Source Files" FILES ${Source_Files})
set(All_Sources ${Header_Files} ${Source_Files})
message(STATUS "All_Sources_GAME: ${All_Sources}")

add_executable(${PROJECT_NAME} ${All_Sources})

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src")
target_link_libraries(${PROJECT_NAME} PRIVATE CG_ENGINE)

################################################################################
# Copy DLLs
################################################################################
if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMAND_EXPAND_LISTS
    )
endif()

get_target_property(PHYSX_DEBUG_DLLS PhysXSDK INTERFACE_PHYSX_DEBUG_DLLS)
get_target_property(PHYSX_RELEASE_DLLS PhysXSDK INTERFACE_PHYSX_RELEASE_DLLS)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<$<CONFIG:Debug>:${PHYSX_DEBUG_DLLS}>"
        "$<$<CONFIG:Release>:${PHYSX_RELEASE_DLLS}>"
        "$<$<CONFIG:Dist>:${PHYSX_RELEASE_DLLS}>"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMAND_EXPAND_LISTS
)

################################################################################
# Precompiled Header
################################################################################
target_precompile_headers(${PROJECT_NAME} REUSE_FROM CG_ENGINE)

################################################################################
# Assets handling
################################################################################
handle_assets()

