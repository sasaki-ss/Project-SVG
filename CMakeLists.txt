cmake_minimum_required(VERSION 3.16)

project(ProjectSVG LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB PROJECT_SVG_LOADER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/loader/*.cpp"
)
file(GLOB PROJECT_SVG_PARSER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/parser/*.cpp"
)
file(GLOB PROJECT_SVG_INTERPRETER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/interpreter/*.cpp"
)
file(GLOB PROJECT_SVG_DRAW_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/draw/*.cpp"
)
file(GLOB PROJECT_SVG_RASTERIZER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/rasterizer/*.cpp"
)
file(GLOB PROJECT_SVG_API_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/api/*.cpp"
)

add_library(project_svg STATIC
    ${PROJECT_SVG_LOADER_SOURCES}
    ${PROJECT_SVG_PARSER_SOURCES}
    ${PROJECT_SVG_INTERPRETER_SOURCES}
    ${PROJECT_SVG_DRAW_SOURCES}
    ${PROJECT_SVG_RASTERIZER_SOURCES}
    ${PROJECT_SVG_API_SOURCES}
)

target_include_directories(project_svg
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

function(project_svg_set_warnings target)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    elseif (MSVC)
        target_compile_options(${target} PRIVATE /W4)
    endif()
endfunction()

project_svg_set_warnings(project_svg)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(PROJECT_SVG_IS_TOP_LEVEL ON)
else()
    set(PROJECT_SVG_IS_TOP_LEVEL OFF)
endif()
option(PROJECT_SVG_BUILD_TESTS "Build the Project SVG unit tests" ${PROJECT_SVG_IS_TOP_LEVEL})

if (PROJECT_SVG_BUILD_TESTS)
    include(CTest)
endif()

function(project_svg_add_test_target layer)
    file(GLOB test_sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/${layer}/*.cpp"
    )
    if (NOT test_sources)
        return()
    endif()

    set(target project_svg_${layer}_tests)
    add_executable(${target} ${test_sources})
    target_link_libraries(${target} PRIVATE project_svg gtest_main)
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    project_svg_set_warnings(${target})
    if (MINGW)
        # Another MinGW's libstdc++ DLL found earlier on PATH (e.g. Git for Windows) breaks the test run.
        target_link_options(${target} PRIVATE -static-libgcc -static-libstdc++)
    endif()
    add_test(NAME ${layer}_tests COMMAND ${target})
endfunction()

if (PROJECT_SVG_BUILD_TESTS AND BUILD_TESTING)
    if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/googletest/CMakeLists.txt")
        message(FATAL_ERROR "BUILD_TESTING=ON requires the GoogleTest submodule at third_party/googletest. Please run: git submodule update --init --recursive")
    endif()

    add_subdirectory(third_party/googletest)

    foreach(layer parser interpreter draw rasterizer api)
        project_svg_add_test_target(${layer})
    endforeach()

    if (TARGET project_svg_api_tests)
        target_compile_definitions(project_svg_api_tests
            PRIVATE
                PROJECT_SVG_API_FIXTURE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/tests/api/fixtures/Line.svg"
        )
    endif()
endif()
