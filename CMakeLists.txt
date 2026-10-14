cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED VERSION)
    execute_process(COMMAND git describe --tags --abbrev=0
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if("${VERSION}" STREQUAL "")
        set(VERSION "0.0.1")
        message(AUTHOR_WARNING "VERSION is not set and failed to get from git. Defaulting to ${VERSION}.")
    endif()
endif()
string(REGEX REPLACE "^v" "" VERSION "${VERSION}")

project(m3colours VERSION ${VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(PROJECT_IS_TOP_LEVEL)
    add_compile_options(
        -Wall -Wextra -Wpedantic -Wshadow -Wconversion
        -Wold-style-cast -Wnull-dereference -Wdouble-promotion
        -Wformat=2 -Wfloat-equal -Woverloaded-virtual
        -Wsign-conversion -Wredundant-decls -Wswitch
        -Wunreachable-code
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(-Wunused-lambda-capture -Wno-nonportable-include-path)
    endif()
endif()

add_library(m3colours
    src/quantise/lab.cpp
    src/quantise/wu.cpp
)
add_library(m3colours::m3colours ALIAS m3colours)

target_include_directories(m3colours PUBLIC include)
target_compile_features(m3colours PUBLIC cxx_std_20)

if(PROJECT_IS_TOP_LEVEL)
    option(BUILD_TESTING "Build tests" OFF)
    if(BUILD_TESTING)
        enable_testing()
        add_subdirectory(tests)
    endif()
endif()
