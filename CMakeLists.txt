cmake_minimum_required(VERSION 3.9)
project(PPLS-CW2)

set(CMAKE_C_STANDARD 99)

include_directories( ${MY_SOURCE_DIR}/src )

add_executable(demo src/Cdemo.c)
add_executable(thread_example src/multiArgumentThreads.c)
add_executable(main src/ex2.c)

set_target_properties(main PROPERTIES
        COMPILE_FLAGS "-DNITEMS=10 -DNTHREADS=4 -DSHOWDATA=1 -lpthread")