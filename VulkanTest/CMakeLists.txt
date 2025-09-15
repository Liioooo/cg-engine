cmake_minimum_required(VERSION 3.23)
include(../CMakeAssetHandling.cmake)

project(VulkanTest VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE Header_Files "src/*.h")
source_group("Header Files" FILES ${Header_Files})
file(GLOB_RECURSE Source_Files "src/*cpp")
source_group("Source Files" FILES ${Source_Files})
set(All_Sources ${Header_Files} ${Source_Files})
message(STATUS "All_Sources_VulkanTest: ${All_Sources}")

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
#    target_sources(${PROJECT_NAME} PRIVATE rtr.rc)
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
if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:VulkanTest> $<TARGET_FILE_DIR:VulkanTest>
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
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<IF:$<OR:$<CONFIG:Release>,$<CONFIG:Dist>>,${PHYSX_RELEASE_DLLS},${PHYSX_DEBUG_DLLS}>" $<TARGET_FILE_DIR:VulkanTest>
            COMMAND_EXPAND_LISTS
    )
endif()
if (UNIX)
    set(PHYSX_DLLS)
    list(APPEND PHYSX_DEBUG_DLLS
            "${CMAKE_CURRENT_LIST_DIR}/../CgEngine/vendor/PhysX/lib/linux/checked/libPhysX_64.so"
            "${CMAKE_CURRENT_LIST_DIR}/../CgEngine/vendor/PhysX/lib/linux/checked/libPhysXCommon_64.so"
            "${CMAKE_CURRENT_LIST_DIR}/../CgEngine/vendor/PhysX/lib/linux/checked/libPhysXFoundation_64.so"
            "${CMAKE_CURRENT_LIST_DIR}/../CgEngine/vendor/PhysX/lib/linux/checked/libPhysXCooking_64.so"
    )
    set(PHYSX_RELEASE_DLLS)
    list(APPEND PHYSX_RELEASE_DLLS
            "${CMAKE_CURRENT_LIST_DIR}/../CgEngine/vendor/PhysX/lib/linux/release/libPhysX_64.so"
            "${CMAKE_CURRENT_LIST_DIR}/../CgEngine/vendor/PhysX/lib/linux/release/libPhysXCommon_64.so"
            "${CMAKE_CURRENT_LIST_DIR}/../CgEngine/vendor/PhysX/lib/linux/release/libPhysXFoundation_64.so"
            "${CMAKE_CURRENT_LIST_DIR}/../CgEngine/vendor/PhysX/lib/linux/release/libPhysXCooking_64.so"
    )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<IF:$<OR:$<CONFIG:Release>,$<CONFIG:Dist>>,${PHYSX_RELEASE_DLLS},${PHYSX_DEBUG_DLLS}>" $<TARGET_FILE_DIR:VulkanTest>
            COMMAND_EXPAND_LISTS
    )
endif()
################################################################################
# Precompiled Header
################################################################################
target_precompile_headers(${PROJECT_NAME} REUSE_FROM CG_ENGINE)

################################################################################
# Assets handling
################################################################################
handle_assets()

