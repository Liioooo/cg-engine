cmake_minimum_required(VERSION 3.23)

project(Main)

# MSVC runtime library
set(MSVC_RUNTIME_LIBRARY_STR "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR} CACHE STRING "" FORCE)

# GLFW #########################################################################
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "" FORCE)
set(GLFW_USE_HYBRID_HPG ON CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)
set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
add_subdirectory(vendor/glfw)
################################################################################

# GLAD #########################################################################
add_subdirectory(vendor/glad)
################################################################################

# GLM ##########################################################################
add_subdirectory(vendor/glm)
################################################################################

# SPDLOG #######################################################################
set(SPDLOG_USE_STD_FORMAT OFF CACHE BOOL "" FORCE)
set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "" FORCE)
set(SPDLOG_FMT_EXTERNAL_HO OFF CACHE BOOL "" FORCE)
set(SPDLOG_MSVC_UTF8 ON CACHE BOOL "" FORCE)
add_subdirectory(vendor/spdlog)
target_compile_definitions(spdlog PRIVATE SPDLOG_COMPILED_LIB)
target_compile_options(spdlog PRIVATE $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/utf-8>)
set_target_properties(spdlog PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})
################################################################################

# INIH #########################################################################
add_library(inih INTERFACE)
target_include_directories(inih INTERFACE "${CMAKE_CURRENT_LIST_DIR}/vendor/inih")
################################################################################

# PUGIXML ######################################################################
set(PUGIXML_STATIC_CRT ON CACHE BOOL "" FORCE)
add_subdirectory(vendor/pugixml)
################################################################################

# STBI #########################################################################
add_library(stbi INTERFACE)
target_include_directories(stbi INTERFACE "${CMAKE_CURRENT_LIST_DIR}/vendor/stbi/include")
################################################################################

# ASSIMP #######################################################################
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(USE_STATIC_CRT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_VIEW  OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/assimp)
################################################################################

# FREETYPE #####################################################################
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
add_subdirectory(vendor/freetype)
################################################################################

# OpenAL #######################################################################
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/openal-soft)
################################################################################

# imgui ########################################################################
set(IMGUI_PATH vendor/imgui)
file(GLOB IMGUI_GLOB
        ${IMGUI_PATH}/imgui.h
        ${IMGUI_PATH}/imgui.cpp
        ${IMGUI_PATH}/imconfig.h
        ${IMGUI_PATH}/imgui_demo.cpp
        ${IMGUI_PATH}/imgui_draw.cpp
        ${IMGUI_PATH}/imgui_internal.h
        ${IMGUI_PATH}/imstb_rectpack.h
        ${IMGUI_PATH}/imstb_textedit.h
        ${IMGUI_PATH}/imstb_truetype.h
        ${IMGUI_PATH}/imgui_tables.cpp
        ${IMGUI_PATH}/imgui_widgets.cpp

        # specific bindings...
        ${IMGUI_PATH}/backends/imgui_impl_glfw.h
        ${IMGUI_PATH}/backends/imgui_impl_glfw.cpp
        ${IMGUI_PATH}/backends/imgui_impl_opengl3.h
        ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp
        ${IMGUI_PATH}/backends/imgui_impl_vulkan.h
        ${IMGUI_PATH}/backends/imgui_impl_vulkan.cpp
)
add_library(imgui STATIC ${IMGUI_GLOB})
target_include_directories(imgui PUBLIC ${IMGUI_PATH})
target_link_libraries(imgui PRIVATE glfw)
set_target_properties(imgui PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})
if (UNIX)
    find_package(X11 REQUIRED)
    target_link_libraries(imgui PRIVATE ${X11_LIBRARIES})
endif()
################################################################################

# DR_LIBS ######################################################################
add_library(dr_libs INTERFACE)
target_include_directories(dr_libs INTERFACE "${CMAKE_CURRENT_LIST_DIR}/vendor/dr_libs")
################################################################################

# VMA ##########################################################################
add_library(vma INTERFACE)
target_include_directories(vma INTERFACE "${CMAKE_CURRENT_LIST_DIR}/vendor/vma/include")
################################################################################

# PHYSX ########################################################################
add_library(PhysXSDK INTERFACE)
target_include_directories(PhysXSDK INTERFACE "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/include")

if (WIN32)
    target_link_libraries(PhysXSDK INTERFACE debug "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/checked/PhysX_64.lib")
    target_link_libraries(PhysXSDK INTERFACE debug "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/checked/PhysXCommon_64.lib")
    target_link_libraries(PhysXSDK INTERFACE debug "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/checked/PhysXFoundation_64.lib")
    target_link_libraries(PhysXSDK INTERFACE debug "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/checked/PhysXCooking_64.lib")
    target_link_libraries(PhysXSDK INTERFACE debug "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/checked/PhysXExtensions_static_64.lib")
    target_link_libraries(PhysXSDK INTERFACE debug "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/checked/PhysXCharacterKinematic_static_64.lib")
    target_link_libraries(PhysXSDK INTERFACE debug "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/checked/PhysXPvdSDK_static_64.lib")
    target_link_libraries(PhysXSDK INTERFACE optimized "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/release/PhysX_64.lib")
    target_link_libraries(PhysXSDK INTERFACE optimized "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/release/PhysXCommon_64.lib")
    target_link_libraries(PhysXSDK INTERFACE optimized "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/release/PhysXFoundation_64.lib")
    target_link_libraries(PhysXSDK INTERFACE optimized "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/release/PhysXCooking_64.lib")
    target_link_libraries(PhysXSDK INTERFACE optimized "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/release/PhysXExtensions_static_64.lib")
    target_link_libraries(PhysXSDK INTERFACE optimized "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/release/PhysXCharacterKinematic_static_64.lib")

    set(PHYSX_DEBUG_DLLS
        "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/checked/PhysX_64.dll"
        "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/checked/PhysXCommon_64.dll"
        "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/checked/PhysXFoundation_64.dll"
        "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/checked/PhysXCooking_64.dll"
    )
    set(PHYSX_RELEASE_DLLS
        "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/release/PhysX_64.dll"
        "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/release/PhysXCommon_64.dll"
        "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/release/PhysXFoundation_64.dll"
        "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/release/PhysXCooking_64.dll"
    )
endif()
if (UNIX)
    set(PHYSX_LIBS
            "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/linux/release/libPhysX_static_64.a"
            "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/linux/release/libPhysXCommon_static_64.a"
            "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/linux/release/libPhysXExtensions_static_64.a"
            "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/linux/release/libPhysXPvdSDK_static_64.a"
            "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/linux/release/libPhysXCooking_static_64.a"
            "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/linux/release/libPhysXFoundation_static_64.a"
            "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/linux/release/libPhysXCharacterKinematic_static_64.a"
    )
    target_link_libraries(PhysXSDK INTERFACE optimized "$<LINK_GROUP:RESCAN,${PHYSX_LIBS}>")

    set(PHYSX_LIBS_DEBUG
            "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/linux/checked/libPhysX_static_64.a"
            "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/linux/checked/libPhysXCommon_static_64.a"
            "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/linux/checked/libPhysXExtensions_static_64.a"
            "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/linux/checked/libPhysXPvdSDK_static_64.a"
            "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/linux/checked/libPhysXCooking_static_64.a"
            "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/linux/checked/libPhysXFoundation_static_64.a"
            "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/linux/checked/libPhysXCharacterKinematic_static_64.a"
    )
    target_link_libraries(PhysXSDK INTERFACE debug "$<LINK_GROUP:RESCAN,${PHYSX_LIBS_DEBUG}>")

    set(PHYSX_DEBUG_DLLS
        "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/linux/checked/libPhysX_64.so"
        "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/linux/checked/libPhysXCommon_64.so"
        "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/linux/checked/libPhysXFoundation_64.so"
        "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/linux/checked/libPhysXCooking_64.so"
    )
    set(PHYSX_RELEASE_DLLS
        "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/linux/release/libPhysX_64.so"
        "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/linux/release/libPhysXCommon_64.so"
        "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/linux/release/libPhysXFoundation_64.so"
        "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/linux/release/libPhysXCooking_64.so"
    )
endif ()

set_property(TARGET PhysXSDK APPEND PROPERTY INTERFACE_PHYSX_DEBUG_DLLS "${PHYSX_DEBUG_DLLS}")
set_property(TARGET PhysXSDK APPEND PROPERTY INTERFACE_PHYSX_RELEASE_DLLS "${PHYSX_RELEASE_DLLS}")
################################################################################

add_subdirectory(CgEngine)
add_subdirectory(CgEngineSharedUtils)
add_subdirectory(AssetPacker)
add_subdirectory(Game)
add_subdirectory(RealTimeRendering)
add_subdirectory(VulkanTest)
