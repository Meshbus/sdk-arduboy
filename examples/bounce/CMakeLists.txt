# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/toolchain.cmake)
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)
project(bounce CXX)
if(NOT MESHBUS_ARDUBOY_SDK_DIR)
 get_filename_component(MESHBUS_ARDUBOY_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
endif()
include(${MESHBUS_ARDUBOY_SDK_DIR}/meshbus_arduboy_llext.cmake)
option(ARDUBOY_TEST_STARTUP_RETRY "Exercise invalid-host retry before launch" OFF)
if(ARDUBOY_TEST_STARTUP_RETRY)
 set(retry_definition MESHBUS_ARDUBOY_TEST_STARTUP_RETRY=1)
endif()
option(ARDUBOY_TRACE_TIMING "Log application-thread waits" OFF)
if(ARDUBOY_TRACE_TIMING)
 list(APPEND retry_definition MESHBUS_ARDUBOY_TRACE_TIMING=1)
endif()
meshbus_arduboy_llext_add_app(bounce
 RUNTIME
 SOURCE ${PROJECT_SOURCE_DIR}/bounce_app.cpp
 SKETCH ${PROJECT_SOURCE_DIR}/upstream/Bounce.ino
 DEFINES MESHBUS_ARDUBOY_RUNTIME=1 ${retry_definition}
 INCLUDES ${LLEXT_EDK_INSTALL_DIR}/include/meshbus/include
)
