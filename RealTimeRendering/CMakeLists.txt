cmake_minimum_required(VERSION 3.23)

project(RealTimeRendering VERSION 0.1.0)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE Header_Files "src/*.h")
source_group("Header Files" FILES ${Header_Files})
file(GLOB_RECURSE Source_Files "src/*cpp")
source_group("Source Files" FILES ${Source_Files})
set(All_Sources ${Header_Files} ${Source_Files})
message(STATUS "All_Sources_RealTimeRendering: ${All_Sources}")

add_executable(${PROJECT_NAME} ${All_Sources})

################################################################################
# MSVC runtime library
################################################################################
get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        $<$<CONFIG:Debug>:
        MultiThreadedDebug
        >
        $<$<OR:$<CONFIG:Release>,$<CONFIG:Dist>>:
        MultiThreaded
        >
        $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:Release>,$<CONFIG:Dist>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
)
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Compile definitions
################################################################################
if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_WIN32)
endif ()
if (UNIX)
    target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_UNIX)
endif ()

target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
        _DEBUG
        >
        $<$<CONFIG:Release>:
        CG_RELEASE
        >
        $<$<CONFIG:Dist>:
        CG_DIST
        >
        $<$<OR:$<CONFIG:Release>,$<CONFIG:Dist>>:
        NDEBUG
        >
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../CgEngine/src "${CMAKE_CURRENT_LIST_DIR}/src")
target_link_libraries(${PROJECT_NAME} PRIVATE CG_ENGINE)

################################################################################
# Copy DLLs
################################################################################
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:RealTimeRendering> $<TARGET_FILE_DIR:RealTimeRendering>
        COMMAND_EXPAND_LISTS
)

set(PHYSX_DLLS)
list(APPEND PHYSX_DEBUG_DLLS
        "${CMAKE_CURRENT_LIST_DIR}/../CgEngine/vendor/PhysX/lib/checked/PhysX_64.dll"
        "${CMAKE_CURRENT_LIST_DIR}/../CgEngine/vendor/PhysX/lib/checked/PhysXCommon_64.dll"
        "${CMAKE_CURRENT_LIST_DIR}/../CgEngine/vendor/PhysX/lib/checked/PhysXFoundation_64.dll"
        "${CMAKE_CURRENT_LIST_DIR}/../CgEngine/vendor/PhysX/lib/checked/PhysXCooking_64.dll"
)
set(PHYSX_RELEASE_DLLS)
list(APPEND PHYSX_RELEASE_DLLS
        "${CMAKE_CURRENT_LIST_DIR}/../CgEngine/vendor/PhysX/lib/release/PhysX_64.dll"
        "${CMAKE_CURRENT_LIST_DIR}/../CgEngine/vendor/PhysX/lib/release/PhysXCommon_64.dll"
        "${CMAKE_CURRENT_LIST_DIR}/../CgEngine/vendor/PhysX/lib/release/PhysXFoundation_64.dll"
        "${CMAKE_CURRENT_LIST_DIR}/../CgEngine/vendor/PhysX/lib/release/PhysXCooking_64.dll"
)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<IF:$<OR:$<CONFIG:Release>,$<CONFIG:Dist>>,${PHYSX_RELEASE_DLLS},${PHYSX_DEBUG_DLLS}>" $<TARGET_FILE_DIR:RealTimeRendering>
        COMMAND_EXPAND_LISTS
)

################################################################################
# Precompiled Header
################################################################################
target_precompile_headers(${PROJECT_NAME} REUSE_FROM CG_ENGINE)

################################################################################
# Assets handling
################################################################################
add_custom_target(copy_assets_cg_engine_real_time_rendering COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/../CgEngine/assets ${PROJECT_BINARY_DIR}/assets/engine)
add_dependencies(${PROJECT_NAME} copy_assets_cg_engine_real_time_rendering)

add_custom_target(copy_assets_real_time_rendering COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${PROJECT_BINARY_DIR}/assets/game)
add_dependencies(${PROJECT_NAME} copy_assets_real_time_rendering)

