cmake_minimum_required(VERSION 2.8.4)

# Set -DTEST=ON to build unit tests.
option(TEST "Build all tests." OFF)
option(DEBUG "Debug mode." OFF)

project(DSHIN)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(OpenGL REQUIRED)
find_package(Boost COMPONENTS program_options filesystem system REQUIRED)
find_package(YamlCpp REQUIRED)
find_package(Armadillo REQUIRED)
find_package(GLEW REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

set(CORELIBS ${GLEW_LIBRARIES} ${GLFW_LIBRARIES} ${ARMADILLO_LIBRARIES} ${Boost_LIBRARIES} ${OPENGL_LIBRARY} ${YAMLCPP_LIBRARY})
set(INCLUDE_DIRS ${GLFW_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})

if (DEBUG)
    set(CMAKE_CXX_FLAGS "-g -Wall")
    add_definitions(-DDEBUG)
else()
    set(CMAKE_CXX_FLAGS "-Ofast")
endif()

add_definitions(-std=c++11)

# Used by both src and test
include_directories(${DSHIN_SOURCE_DIR}/src)
include_directories(${DSHIN_SOURCE_DIR}/src/third_party/)
include_directories(${GLEW_INCLUDE_DIRS})

add_subdirectory(src)

if (TEST)
    # This adds 'project(gmock)'
    add_subdirectory(${DSHIN_SOURCE_DIR}/lib/gmock-1.7.0)
    add_subdirectory(test)

    enable_testing()
    add_test(NAME RunTests COMMAND runUnitTests)
endif()
