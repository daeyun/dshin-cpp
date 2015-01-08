if (DEBUG OR TEST)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DSHIN_SOURCE_DIR}/bin-dev)
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DSHIN_SOURCE_DIR}/bin)
endif()

# Get all files except for ./src/main.cc and make them a library.
file(GLOB_RECURSE INCLUDES "*.h" "*.cpp" "*.cc")
list(REMOVE_ITEM INCLUDES ${DSHIN_SOURCE_DIR}/src/main.cc)

# Also used in ./test/CMakeLists.txt
add_library (dshin_lib ${INCLUDES})

add_executable(dshin main.cc)
target_link_libraries(dshin dshin_lib ${CORELIBS})
