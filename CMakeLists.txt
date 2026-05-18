cmake_minimum_required(VERSION 3.20)

project(rvc_ai LANGUAGES CXX)

option(BUILD_PYTHON_BINDINGS "Build pybind11 module for simulator integration" ON)
option(BUILD_UNIT_TESTS "Build GoogleTest unit tests" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(rvc_controller
    src/AdapterTimer.cpp
    src/CleaningManager.cpp
    src/LegacyAdapters.cpp
    src/LeftPriorityAvoidanceStrategy.cpp
    src/MovementManager.cpp
    src/RvcController.cpp
    src/RvcStates.cpp
    src/SensorSubjects.cpp
    src/SimulatorApi.cpp
)

target_include_directories(rvc_controller
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(rvc_controller
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

enable_testing()

if(BUILD_UNIT_TESTS)
    find_package(GTest REQUIRED)
    include(GoogleTest)

    add_executable(rvc_unit_tests tests/rvc_unit_tests.cpp)
    target_link_libraries(rvc_unit_tests PRIVATE rvc_controller GTest::gtest_main)
    gtest_discover_tests(rvc_unit_tests)
endif()

if(BUILD_PYTHON_BINDINGS)
    find_package(pybind11 QUIET)
    if(NOT pybind11_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            pybind11
            GIT_REPOSITORY https://github.com/pybind/pybind11.git
            GIT_TAG        v2.13.6
        )
        FetchContent_MakeAvailable(pybind11)
    endif()

    pybind11_add_module(rvc bindings/rvc_module.cpp)
    target_link_libraries(rvc PRIVATE rvc_controller)
    set_target_properties(rvc PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bindings
    )
endif()
