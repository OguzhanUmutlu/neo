cmake_minimum_required(VERSION 3.22)
project(neo_api_debug C)

set(CMAKE_C_STANDARD 11)

find_library(GMP_LIB gmp)
find_library(MPFR_LIB mpfr)

if(NOT GMP_LIB)
    message(FATAL_ERROR "GMP library not found.")
endif()

if(NOT MPFR_LIB)
    message(FATAL_ERROR "MPFR library not found.")
endif()

include_directories(.)
include_directories(types)
include_directories(include)

add_executable(neo_api_debug
        types/neoarray.c
        types/neobigfloat.c
        types/neobigint.c
        types/neoboolean.c
        types/neodouble.c
        types/neofunction.c
        types/neoint.c
        types/neoobject.c
        types/neostring.c
        tests/test.c
        neo.c)

target_link_libraries(neo_api_debug m)

target_link_libraries(neo_api_debug ${GMP_LIB} ${MPFR_LIB})
