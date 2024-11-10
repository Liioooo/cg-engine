cmake_minimum_required(VERSION 3.23)

project(CG_ENGINE VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE Header_Files "src/*.h")
source_group("Header Files" FILES ${Header_Files})
file(GLOB_RECURSE Source_Files "src/*cpp")
source_group("Source Files" FILES ${Source_Files})
set(All_Sources ${Header_Files} ${Source_Files})
message(STATUS "All_Sources_CG_ENGINE: ${All_Sources}")

add_library(${PROJECT_NAME} ${All_Sources})

# OpenGL
find_package(OpenGL REQUIRED)

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

################################################################################
# Dependencies
################################################################################
set(ADDITIONAL_LIBRARY_DEPENDENCIES)
set(ADDITIONAL_INCLUDE_DIRECTORIES)

# GLFW #########################################################################
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "" FORCE)
set(GLFW_USE_HYBRID_HPG ON CACHE BOOL "" FORCE)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/vendor/glfw)
list(APPEND ADDITIONAL_LIBRARY_DEPENDENCIES glfw)
list(APPEND ADDITIONAL_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/vendor/glfw/include")
################################################################################

# GLAD #########################################################################
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/vendor/glad)
list(APPEND ADDITIONAL_LIBRARY_DEPENDENCIES glad)
list(APPEND ADDITIONAL_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/vendor/glad/include")
################################################################################

# GLM ##########################################################################
list(APPEND ADDITIONAL_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/vendor/glm")
################################################################################

# SPDLOG #######################################################################
list(APPEND ADDITIONAL_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/vendor/spdlog/include")
################################################################################

# INIH #########################################################################
list(APPEND ADDITIONAL_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/vendor/inih")
################################################################################

# PUGIXML ######################################################################
set(PUGIXML_STATIC_CRT ON CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/vendor/pugixml)
list(APPEND ADDITIONAL_LIBRARY_DEPENDENCIES pugixml)
list(APPEND ADDITIONAL_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/vendor/pugixml/src")
################################################################################

# STBI #########################################################################
list(APPEND ADDITIONAL_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/vendor/stbi/include")
################################################################################

# ASSIMP #######################################################################
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(USE_STATIC_CRT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_VIEW  OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/vendor/assimp)
list(APPEND ADDITIONAL_LIBRARY_DEPENDENCIES assimp)
list(APPEND ADDITIONAL_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/vendor/assimp/include")
################################################################################

# FREETYPE #####################################################################
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/vendor/freetype)
list(APPEND ADDITIONAL_LIBRARY_DEPENDENCIES freetype)
list(APPEND ADDITIONAL_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/vendor/freetype/include")
################################################################################

# PHYSX ########################################################################
list(APPEND ADDITIONAL_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/include")
target_link_libraries(${PROJECT_NAME} PRIVATE debug "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/checked/PhysX_64.lib")
target_link_libraries(${PROJECT_NAME} PRIVATE debug "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/checked/PhysXCommon_64.lib")
target_link_libraries(${PROJECT_NAME} PRIVATE debug "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/checked/PhysXFoundation_64.lib")
target_link_libraries(${PROJECT_NAME} PRIVATE debug "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/checked/PhysXCooking_64.lib")
target_link_libraries(${PROJECT_NAME} PRIVATE debug "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/checked/PhysXExtensions_static_64.lib")
target_link_libraries(${PROJECT_NAME} PRIVATE debug "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/checked/PhysXCharacterKinematic_static_64.lib")
target_link_libraries(${PROJECT_NAME} PRIVATE debug "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/checked/PhysXPvdSDK_static_64.lib")
target_link_libraries(${PROJECT_NAME} PRIVATE optimized "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/release/PhysX_64.lib")
target_link_libraries(${PROJECT_NAME} PRIVATE optimized "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/release/PhysXCommon_64.lib")
target_link_libraries(${PROJECT_NAME} PRIVATE optimized "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/release/PhysXFoundation_64.lib")
target_link_libraries(${PROJECT_NAME} PRIVATE optimized "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/release/PhysXCooking_64.lib")
target_link_libraries(${PROJECT_NAME} PRIVATE optimized "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/release/PhysXExtensions_static_64.lib")
target_link_libraries(${PROJECT_NAME} PRIVATE optimized "${CMAKE_CURRENT_LIST_DIR}/vendor/PhysX/lib/release/PhysXCharacterKinematic_static_64.lib")
################################################################################

# OpenAL #######################################################################
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/vendor/openal-soft)
list(APPEND ADDITIONAL_LIBRARY_DEPENDENCIES OpenAL)
list(APPEND ADDITIONAL_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/vendor/openal-soft/include")
################################################################################

# DR_LIBS ######################################################################
list(APPEND ADDITIONAL_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/vendor/dr_libs")
################################################################################

target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/src/CgEngine"
        "${ADDITIONAL_INCLUDE_DIRECTORIES}"
)

target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

################################################################################
# Precompiled Header
################################################################################
target_precompile_headers(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/pch.h")
