# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/toolchain.cmake)
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)
project(resource_fixture CXX)
if(NOT MESHBUS_ARDUBOY_SDK_DIR)
 get_filename_component(MESHBUS_ARDUBOY_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
endif()
find_package(Python3 REQUIRED COMPONENTS Interpreter)
execute_process(COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/generate.py"
 "${PROJECT_BINARY_DIR}/fixture.bin" COMMAND_ERROR_IS_FATAL ANY)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/generate.py")
include(${MESHBUS_ARDUBOY_SDK_DIR}/meshbus_arduboy_llext.cmake)
meshbus_arduboy_llext_add_app(resource_fixture RUNTIME
 SOURCE ${PROJECT_SOURCE_DIR}/main.cpp
 RESOURCE ${PROJECT_BINARY_DIR}/fixture.bin
 RESOURCE_VERSION 1
 RESOURCE_DIRECTORY /extra/apps/arduboy_test
 INCLUDES ${LLEXT_EDK_INSTALL_DIR}/include/meshbus/include)
