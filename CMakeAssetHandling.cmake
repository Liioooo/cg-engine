function(compile_shaders API SHADER_ROOT ASSET_TYPE)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs INCLUDE_DIRS)
    cmake_parse_arguments(COMPILE_SHADERS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    file(GLOB_RECURSE GLSL_SOURCE_FILES "${SHADER_ROOT}/*.glsl")

    if(NOT GLSL_SOURCE_FILES)
        message(WARNING "No shaders found in ${SHADER_ROOT}")
        return()
    endif()

    set(SPV_OUTPUT_DIR "${PROJECT_BINARY_DIR}/assets/${ASSET_TYPE}/compiled_shaders/${API}")
    set(SPV_BINARY_FILES "")

    set(INCLUDE_ARGS "")
    foreach(INC_DIR ${COMPILE_SHADERS_INCLUDE_DIRS})
        list(APPEND INCLUDE_ARGS -I${INC_DIR})
    endforeach()

    foreach(GLSL ${GLSL_SOURCE_FILES})
        file(RELATIVE_PATH REL_PATH "${SHADER_ROOT}" ${GLSL})
        get_filename_component(FILE_NAME ${REL_PATH} NAME_WE)
        get_filename_component(REL_DIR ${REL_PATH} DIRECTORY)

        set(SPV_DIR "${SPV_OUTPUT_DIR}/${REL_DIR}")
        set(SPV "${SPV_DIR}/${FILE_NAME}.spv")

        set(SHADER_STAGE "")
        if(FILE_NAME MATCHES "_vertex$")
            set(SHADER_STAGE "vert")
        elseif(FILE_NAME MATCHES "_fragment$")
            set(SHADER_STAGE "frag")
        elseif(FILE_NAME MATCHES "_comp$")
            set(SHADER_STAGE "comp")
        elseif(FILE_NAME MATCHES "_geometry$")
            set(SHADER_STAGE "geom")
        elseif(FILE_NAME MATCHES "_tcs$")
            set(SHADER_STAGE "tesc")
        elseif(FILE_NAME MATCHES "_tes$")
            set(SHADER_STAGE "tese")
        else()
            message(FATAL_ERROR "Unknown shader stage for file: ${FILE_NAME}")
        endif()

        add_custom_command(
                OUTPUT ${SPV}
                COMMAND ${CMAKE_COMMAND} -E make_directory ${SPV_DIR}
                COMMAND glslc --target-env=${API} -fshader-stage=${SHADER_STAGE} ${INCLUDE_ARGS} ${GLSL} -o ${SPV}
                DEPENDS ${GLSL}
                COMMENT "[${API}] Compiling Shader: ${REL_PATH} -> ${REL_DIR}/${FILE_NAME}.spv"
                VERBATIM
        )

        list(APPEND SPV_BINARY_FILES ${SPV})
    endforeach()

    add_custom_target(compile_shaders_${API}_${PROJECT_NAME}_${ASSET_TYPE} ALL DEPENDS ${SPV_BINARY_FILES})
endfunction()


function(handle_assets)
#    compile_shaders(vulkan "${CMAKE_CURRENT_SOURCE_DIR}/../CgEngine/assets/shaders" engine INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../CgEngine/assets/shaders_include")
    compile_shaders(opengl "${CMAKE_CURRENT_SOURCE_DIR}/../CgEngine/assets/shaders" engine INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../CgEngine/assets/shaders_include")

#    compile_shaders(vulkan "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders" game)
    compile_shaders(opengl "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders" game)

    file(GLOB_RECURSE ASSET_FILES_ENGINE RELATIVE ${CMAKE_CURRENT_LIST_DIR}/../CgEngine/assets ${CMAKE_CURRENT_LIST_DIR}/../CgEngine/assets/*)
    list(FILTER ASSET_FILES_ENGINE EXCLUDE REGEX "^shaders/")
    list(FILTER ASSET_FILES_ENGINE EXCLUDE REGEX "^shaders_include/")
    add_custom_target(copy_assets_${PROJECT_NAME}_engine ALL COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/assets/engine)
    foreach(file ${ASSET_FILES_ENGINE})
        add_custom_command(TARGET copy_assets_${PROJECT_NAME}_engine POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_LIST_DIR}/../CgEngine/assets/${file}
                ${PROJECT_BINARY_DIR}/assets/engine/${file}
        )
    endforeach()

    file(GLOB_RECURSE ASSET_FILES_GAME RELATIVE ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_LIST_DIR}/assets/*)
    list(FILTER ASSET_FILES_GAME EXCLUDE REGEX "^shaders/")
    list(FILTER ASSET_FILES_GAME EXCLUDE REGEX "^shaders_include/")
    add_custom_target(copy_assets_${PROJECT_NAME}_game ALL COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/assets/engine)
    foreach(file ${ASSET_FILES_GAME})
        add_custom_command(TARGET copy_assets_${PROJECT_NAME}_game POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_LIST_DIR}/assets/${file}
                ${PROJECT_BINARY_DIR}/assets/game/${file}
        )
    endforeach()

    add_dependencies(${PROJECT_NAME} copy_assets_${PROJECT_NAME}_engine)
#    add_dependencies(${PROJECT_NAME} compile_shaders_vulkan_${PROJECT_NAME}_engine)
    add_dependencies(${PROJECT_NAME} compile_shaders_opengl_${PROJECT_NAME}_engine)

    add_dependencies(${PROJECT_NAME} copy_assets_${PROJECT_NAME}_game)
    if(TARGET compile_shaders_vulkan_${PROJECT_NAME}_game)
        add_dependencies(${PROJECT_NAME} compile_shaders_vulkan_${PROJECT_NAME}_game)
    endif()
    if(TARGET compile_shaders_opengl_${PROJECT_NAME}_game)
        add_dependencies(${PROJECT_NAME} compile_shaders_opengl_${PROJECT_NAME}_game)
    endif()
endfunction()
