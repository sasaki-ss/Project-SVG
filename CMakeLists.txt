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

add_library(project_svg STATIC
    ${PROJECT_SVG_LOADER_SOURCES}
    ${PROJECT_SVG_PARSER_SOURCES}
)

target_include_directories(project_svg
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(project_svg PRIVATE -Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(project_svg PRIVATE -Wall -Wextra -Wpedantic)
elseif (MSVC)
    target_compile_options(project_svg PRIVATE /W4)
endif()

include(CTest)

if (BUILD_TESTING)
    add_subdirectory(third_party/googletest)

    file(GLOB PROJECT_SVG_PARSER_TEST_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/parser/*.cpp"
    )
    file(GLOB PROJECT_SVG_INTERPRETER_TEST_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/interpreter/*.cpp"
    )

    if (PROJECT_SVG_PARSER_TEST_SOURCES)
        add_executable(project_svg_parser_tests
            ${PROJECT_SVG_PARSER_TEST_SOURCES}
        )

        target_link_libraries(project_svg_parser_tests
            PRIVATE
                project_svg
        )

        target_include_directories(project_svg_parser_tests
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/src
                ${CMAKE_CURRENT_SOURCE_DIR}/src/parser
        )

        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(project_svg_parser_tests PRIVATE -Wall -Wextra -Wpedantic)
        elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(project_svg_parser_tests PRIVATE -Wall -Wextra -Wpedantic)
        elseif (MSVC)
            target_compile_options(project_svg_parser_tests PRIVATE /W4)
        endif()

        enable_testing()
        add_test(NAME parser_tests COMMAND project_svg_parser_tests)
    endif()

    if (PROJECT_SVG_INTERPRETER_TEST_SOURCES)
        add_executable(project_svg_interpreter_tests
            ${PROJECT_SVG_INTERPRETER_TEST_SOURCES}
            ${CMAKE_CURRENT_SOURCE_DIR}/src/interpreter/SvgElementInterpreter.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/interpreter/SvgShapeInterpreter.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/interpreter/SvgInterpreterUtility.cpp
        )

        target_include_directories(project_svg_interpreter_tests
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/src
                ${CMAKE_CURRENT_SOURCE_DIR}/src/interpreter
                ${CMAKE_CURRENT_SOURCE_DIR}/src/parser
        )

        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(project_svg_interpreter_tests PRIVATE -Wall -Wextra -Wpedantic)
        elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(project_svg_interpreter_tests PRIVATE -Wall -Wextra -Wpedantic)
        elseif (MSVC)
            target_compile_options(project_svg_interpreter_tests PRIVATE /W4)
        endif()

        enable_testing()
        add_test(NAME interpreter_tests COMMAND project_svg_interpreter_tests)
    endif()
endif()
