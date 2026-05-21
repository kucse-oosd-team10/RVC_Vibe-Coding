cmake_minimum_required(VERSION 3.22)

project(rvc_ai LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# compile_commands.json for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------- Options ----------
option(ENABLE_TESTING "Build unit tests" ON)
option(ENABLE_COVERAGE "Enable coverage flags" OFF)
option(BUILD_UNIT_TESTS "Build GoogleTest unit tests" ${ENABLE_TESTING})
option(BUILD_PYTHON_BINDINGS "Build pybind11 module for simulator integration" OFF)

# ---------- Coverage flags ----------
if(ENABLE_COVERAGE)
    add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
    add_link_options(-fprofile-instr-generate -fcoverage-mapping)
endif()

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

set_target_properties(rvc_controller PROPERTIES POSITION_INDEPENDENT_CODE ON)

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

if(ENABLE_TESTING)
    enable_testing()
endif()

if(ENABLE_TESTING AND BUILD_UNIT_TESTS)
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        v1.15.2
        )
        FetchContent_MakeAvailable(googletest)
    endif()

    include(GoogleTest)

    add_executable(rvc_tests tests/rvc_unit_tests.cpp)
    target_link_libraries(rvc_tests PRIVATE rvc_controller GTest::gtest_main)
    set_target_properties(rvc_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
    )
    gtest_discover_tests(rvc_tests)
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
