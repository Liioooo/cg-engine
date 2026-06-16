cmake_minimum_required(VERSION 3.24)

project(CG_ENGINE VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE Header_Files "src/*.h")
source_group("Header Files" FILES ${Header_Files})
file(GLOB_RECURSE Source_Files "src/*cpp")
source_group("Source Files" FILES ${Source_Files})
set(All_Sources ${Header_Files} ${Source_Files})
message(STATUS "All_Sources_CG_ENGINE: ${All_Sources}")

add_library(${PROJECT_NAME} STATIC ${All_Sources})

# OpenGL, Vulkan
find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)

################################################################################
# Compile definitions
################################################################################
if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_WIN32)
endif ()
if (UNIX)
    target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_UNIX)
endif ()

target_compile_definitions(${PROJECT_NAME} PUBLIC
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

# VULKAN #######################################################################
target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan)
target_compile_definitions(${PROJECT_NAME} PUBLIC VULKAN_HPP_NO_EXCEPTIONS)
################################################################################

# GLFW #########################################################################
target_compile_definitions(${PROJECT_NAME} PUBLIC GLFW_INCLUDE_NONE)
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
################################################################################

# GLAD #########################################################################
target_link_libraries(${PROJECT_NAME} PUBLIC glad)
################################################################################

# GLM ##########################################################################
target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)
################################################################################

# SPDLOG #######################################################################
target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog)
################################################################################

# INIH #########################################################################
target_link_libraries(${PROJECT_NAME} PUBLIC inih)
################################################################################

# PUGIXML ######################################################################
target_link_libraries(${PROJECT_NAME} PUBLIC pugixml)
################################################################################

# STBI #########################################################################
target_link_libraries(${PROJECT_NAME} PUBLIC stbi)
################################################################################

# VMA ##########################################################################
target_link_libraries(${PROJECT_NAME} PUBLIC vma)
################################################################################

# ASSIMP #######################################################################
target_link_libraries(${PROJECT_NAME} PUBLIC assimp)
################################################################################

# FREETYPE #####################################################################
target_link_libraries(${PROJECT_NAME} PUBLIC freetype)
################################################################################

# PHYSX ########################################################################
target_link_libraries(${PROJECT_NAME} PUBLIC PhysXSDK)
################################################################################

# OpenAL #######################################################################
target_link_libraries(${PROJECT_NAME} PUBLIC OpenAL)
################################################################################

# imgui ########################################################################
target_link_libraries(${PROJECT_NAME} PUBLIC imgui)
################################################################################

# DR_LIBS ######################################################################
target_link_libraries(${PROJECT_NAME} PUBLIC dr_libs)
################################################################################

# CG_ENGINE_SHARED_UTILS #######################################################
target_link_libraries(${PROJECT_NAME} PUBLIC CG_ENGINE_SHARED_UTILS)
################################################################################

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/CgEngine")
target_include_directories(${PROJECT_NAME} INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src")

if(MSVC)
    target_compile_options(${PROJECT_NAME} PUBLIC "/utf-8")
endif()

################################################################################
# Precompiled Header
################################################################################
target_precompile_headers(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/pch.h")
