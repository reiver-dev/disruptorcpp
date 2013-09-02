cmake_minimum_required (VERSION 2.8)

project(disruptor)

set(EXECUTABLE_OUTPUT_PATH ../bin)
set(LIBRARY_OUTPUT_PATH ../bin)

set(CFLAGS -pipe -Wall -pedantic)

if(DEBUG)
    set(CFLAGS ${CFLAGS} -g3 -DDEBUG)
else()
    set(CFLAGS ${CFLAGS} -O3 -g3)
endif()

include_directories(include)

add_definitions(${CFLAGS})
add_subdirectory(test)
add_subdirectory(perftest)



