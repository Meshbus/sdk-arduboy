# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/toolchain.cmake)
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)
project(isojourn CXX)
if(NOT MESHBUS_ARDUBOY_SDK_DIR)
 get_filename_component(MESHBUS_ARDUBOY_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
endif()
find_package(Python3 REQUIRED COMPONENTS Interpreter)
execute_process(COMMAND "${Python3_EXECUTABLE}" "${MESHBUS_ARDUBOY_SDK_DIR}/tools/fx_pack.py"
 --input "${PROJECT_SOURCE_DIR}/upstream/data.bin" --output "${PROJECT_BINARY_DIR}/fixture.bin" --page 0xFEBE COMMAND_ERROR_IS_FATAL ANY)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${MESHBUS_ARDUBOY_SDK_DIR}/tools/fx_pack.py" "${PROJECT_SOURCE_DIR}/upstream/data.bin")
include(${MESHBUS_ARDUBOY_SDK_DIR}/meshbus_arduboy_llext.cmake)
meshbus_arduboy_llext_add_app(isojourn RUNTIME FX
 SKETCH ${PROJECT_SOURCE_DIR}/upstream/isojourn.ino
 SOURCE ${PROJECT_SOURCE_DIR}/main.cpp
 RESOURCE ${PROJECT_BINARY_DIR}/fixture.bin
 RESOURCE_VERSION 1
 RESOURCE_DIRECTORY /extra/apps/arduboy_test
 INCLUDES ${LLEXT_EDK_INSTALL_DIR}/include/meshbus/include)
