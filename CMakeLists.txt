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

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(project_svg PRIVATE -Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(project_svg PRIVATE -Wall -Wextra -Wpedantic)
elseif (MSVC)
    target_compile_options(project_svg PRIVATE /W4)
endif()

include(CTest)

if (BUILD_TESTING)
    if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/googletest/CMakeLists.txt")
        message(FATAL_ERROR "BUILD_TESTING=ON requires the GoogleTest submodule at third_party/googletest. Please run: git submodule update --init --recursive")
    endif()

    add_subdirectory(third_party/googletest)

    file(GLOB PROJECT_SVG_PARSER_TEST_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/parser/*.cpp"
    )
    file(GLOB PROJECT_SVG_INTERPRETER_TEST_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/interpreter/*.cpp"
    )
    file(GLOB PROJECT_SVG_DRAW_TEST_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/draw/*.cpp"
    )
    file(GLOB PROJECT_SVG_RASTERIZER_TEST_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/rasterizer/*.cpp"
    )
    file(GLOB PROJECT_SVG_API_TEST_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/api/*.cpp"
    )

    if (PROJECT_SVG_PARSER_TEST_SOURCES)
        add_executable(project_svg_parser_tests
            ${PROJECT_SVG_PARSER_TEST_SOURCES}
        )

        target_link_libraries(project_svg_parser_tests
            PRIVATE
                project_svg
                gtest_main
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
        )

        target_link_libraries(project_svg_interpreter_tests
            PRIVATE
                project_svg
                gtest_main
        )

        target_include_directories(project_svg_interpreter_tests
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/src
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

    if (PROJECT_SVG_DRAW_TEST_SOURCES)
        add_executable(project_svg_draw_tests
            ${PROJECT_SVG_DRAW_TEST_SOURCES}
        )

        target_link_libraries(project_svg_draw_tests
            PRIVATE
                project_svg
                gtest_main
        )

        target_include_directories(project_svg_draw_tests
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/src
        )

        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(project_svg_draw_tests PRIVATE -Wall -Wextra -Wpedantic)
        elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(project_svg_draw_tests PRIVATE -Wall -Wextra -Wpedantic)
        elseif (MSVC)
            target_compile_options(project_svg_draw_tests PRIVATE /W4)
        endif()

        enable_testing()
        add_test(NAME draw_tests COMMAND project_svg_draw_tests)
    endif()

    if (PROJECT_SVG_RASTERIZER_TEST_SOURCES)
        add_executable(project_svg_rasterizer_tests
            ${PROJECT_SVG_RASTERIZER_TEST_SOURCES}
        )

        target_link_libraries(project_svg_rasterizer_tests
            PRIVATE
                project_svg
                gtest_main
        )

        target_include_directories(project_svg_rasterizer_tests
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/src
        )

        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(project_svg_rasterizer_tests PRIVATE -Wall -Wextra -Wpedantic)
        elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(project_svg_rasterizer_tests PRIVATE -Wall -Wextra -Wpedantic)
        elseif (MSVC)
            target_compile_options(project_svg_rasterizer_tests PRIVATE /W4)
        endif()

        enable_testing()
        add_test(NAME rasterizer_tests COMMAND project_svg_rasterizer_tests)
    endif()

    if (PROJECT_SVG_API_TEST_SOURCES)
        add_executable(project_svg_api_tests
            ${PROJECT_SVG_API_TEST_SOURCES}
        )

        target_link_libraries(project_svg_api_tests
            PRIVATE
                project_svg
                gtest_main
        )

        target_include_directories(project_svg_api_tests
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/src
        )

        set(PROJECT_SVG_API_FIXTURE_PATH
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/api/fixtures/Line.svg"
        )

        target_compile_definitions(project_svg_api_tests
            PRIVATE
                PROJECT_SVG_API_FIXTURE_PATH="${PROJECT_SVG_API_FIXTURE_PATH}"
        )

        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(project_svg_api_tests PRIVATE -Wall -Wextra -Wpedantic)
        elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(project_svg_api_tests PRIVATE -Wall -Wextra -Wpedantic)
        elseif (MSVC)
            target_compile_options(project_svg_api_tests PRIVATE /W4)
        endif()

        enable_testing()
        add_test(NAME api_tests COMMAND project_svg_api_tests)

        if (MINGW)
            get_filename_component(
                PROJECT_SVG_MINGW_BIN_DIRECTORY
                "${CMAKE_CXX_COMPILER}"
                DIRECTORY
            )
            string(REPLACE ";" "\\;" PROJECT_SVG_TEST_PATH "$ENV{PATH}")
            set_tests_properties(
                api_tests
                PROPERTIES
                    ENVIRONMENT
                        "PATH=${PROJECT_SVG_MINGW_BIN_DIRECTORY}\\;${PROJECT_SVG_TEST_PATH}"
            )
        endif()
    endif()

endif()
