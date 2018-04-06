# Minimum cmake required
cmake_minimum_required(VERSION 3.2)

# Project
project(map VERSION 0.1 LANGUAGES C CXX)

# Compiler flags
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-fPIC")
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-fPIC")


#
include(GNUInstallDirs)

#
add_subdirectory(src) # resursive, modular

#
add_library(map SHARED map.cpp
	$<TARGET_OBJECTS:map_core>
	$<TARGET_OBJECTS:map_front>
	$<TARGET_OBJECTS:map_inter>
	$<TARGET_OBJECTS:map_back>
	$<TARGET_OBJECTS:map_runt>
	$<TARGET_OBJECTS:map_file>
	$<TARGET_OBJECTS:map_visit>
	$<TARGET_OBJECTS:map_util>
)
target_link_libraries(map tiff pthread OpenCL)

#
set_target_properties(map PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	PUBLIC_HEADER map.hpp
)

configure_file(map.pc.in map.pc @ONLY)

target_include_directories(map PRIVATE .)

install(TARGETS map LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${CMAKE_BINARY_DIR}/map.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

#[[# https://bloerg.net/2012/11/10/cmake-and-distutils.html

find_program(PYTHON "python")

if (PYTHON)
    set(SETUP_PY_IN "${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in")
    set(SETUP_PY    "${CMAKE_CURRENT_BINARY_DIR}/setup.py")
    set(DEPS        "${CMAKE_CURRENT_SOURCE_DIR}/module/__init__.py")
    set(OUTPUT      "${CMAKE_CURRENT_BINARY_DIR}/build/timestamp")

    configure_file(${SETUP_PY_IN} ${SETUP_PY})

    add_custom_command(OUTPUT ${OUTPUT}
                       COMMAND ${PYTHON} ${SETUP_PY} build
                       COMMAND ${CMAKE_COMMAND} -E touch ${OUTPUT}
                       DEPENDS ${DEPS})

    add_custom_target(target ALL DEPENDS ${OUTPUT})

    install(CODE "execute_process(COMMAND ${PYTHON} ${SETUP_PY} install)")
endif()

]]#

#[[

# Options
option(map_DEBUG "Enable for debug mode" OFF)
option(map_INTERPRETER "Enable to deactivate compiler approach" OFF)
option(map_CODE_FUSION "Enable to Activates fusion" ON)
option(map_INMEM_CACHE "Enable to Activates in-memory caching" ON)
option(map_COMPIL_CACHE "Enable to Activates compilation cache" ON)
option(map_PREDICTION "Enable to Activates predicton" ON)

# required for unit testing using CMake's ctest command
include(CTest)
enable_testing()

# required modules for our task
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckIncludeFiles)
include(ExternalProject)

# External dependencies
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/thirdparty)

# Recursively build the...
add_subdirectory(src) # shared library
add_subdirectory(examples) # binaries
add_subdirectory(python) # package

]]#
