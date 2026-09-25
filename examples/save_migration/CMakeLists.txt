# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/toolchain.cmake)
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)
project(save_migration CXX)
if(NOT MESHBUS_ARDUBOY_SDK_DIR)
 get_filename_component(MESHBUS_ARDUBOY_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
endif()
include(${MESHBUS_ARDUBOY_SDK_DIR}/meshbus_arduboy_llext.cmake)
meshbus_arduboy_llext_add_app(save_migration
 RUNTIME
 SOURCE ${PROJECT_SOURCE_DIR}/save_migration.cpp
 DEFINES MESHBUS_ARDUBOY_RUNTIME=1
 INCLUDES ${LLEXT_EDK_INSTALL_DIR}/include/meshbus/include)
