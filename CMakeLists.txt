cmake_minimum_required(VERSION 3.23)
project(UnitTest C)
add_compile_definitions(CQUEUE_USE_MALLOC)
add_compile_definitions(CQUEUE_UNIT_TEST)
set(CMAKE_C_STANDARD 99)
file(GLOB SOURCES
        src/cQueue.c
        src/main.c
        src/mleak.c
        src/cQueue.h
        src/mleak.h
        )
add_executable(UnitTest src/main.c ${SOURCE_FILES} ${SOURCES})


