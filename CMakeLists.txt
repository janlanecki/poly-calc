cmake_minimum_required(VERSION 3.0)
project(polynomials C)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif ()

# No meassages about the progress of the compilation.
set_property(GLOBAL PROPERTY RULE_MESSAGES OFF)

# Make messages.
set(CMAKE_VERBOSE_MAKEFILE ON)

# Compilation flags.
set(CMAKE_C_FLAGS "-std=c11")
# set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
# set(CMAKE_C_FLAGS_DEBUG "-g")

find_library(CMOCKA cmocka)

if (NOT CMOCKA)
    message(FATAL_ERROR "Could not find cmocka.")
endif ()

enable_testing()

set(SOURCE_FILES
    src/poly.c
    src/poly.h
    src/stack_poly.c
    src/stack_poly.h
    src/calc_poly.c
    src/utils.h
)

add_executable(calc_poly ${SOURCE_FILES})
add_executable(unit_tests_poly src/unit_tests_poly.c ${SOURCE_FILES})

set_target_properties(
    unit_tests_poly
    PROPERTIES
    COMPILE_DEFINITIONS UNIT_TESTING=1)

target_link_libraries(unit_tests_poly ${CMOCKA})
add_test(unit_tests_poly ${CMAKE_CURRENT_BINARY_DIR}/unit_tests_poly)

find_package(Doxygen)
if (DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile @ONLY)
    # using make doc creates Doxygen documentation in the specified directory with the specified configuration file
    add_custom_target(doc
	${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/doc
        COMMENT "Generating API documentation with Doxygen"
    )
endif (DOXYGEN_FOUND)
