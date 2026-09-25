# SPDX-License-Identifier: Apache-2.0
include_guard(GLOBAL)
if(NOT DEFINED MESHBUS_ARDUBOY_SDK_DIR)
    get_filename_component(MESHBUS_ARDUBOY_SDK_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
endif()
get_filename_component(MESHBUS_ARDUBOY_SDK_DIR "${MESHBUS_ARDUBOY_SDK_DIR}" ABSOLUTE)
set(MESHBUS_ARDUBOY_COMMON_INCLUDE_DIR "${MESHBUS_ARDUBOY_SDK_DIR}/include")

function(meshbus_arduboy_llext_add_app target_name)
    cmake_parse_arguments(APP "RUNTIME;COMPATIBILITY;FX" "SOURCE;SKETCH;RESOURCE;RESOURCE_VERSION;RESOURCE_DIRECTORY" "SOURCES;DEFINES;DEPENDS;FLAGS;INCLUDES;REQUIRES" ${ARGN})
    if(APP_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unknown Arduboy arguments: ${APP_UNPARSED_ARGUMENTS}")
    endif()
    if((APP_SOURCE AND APP_SOURCES) OR (NOT APP_SOURCE AND NOT APP_SOURCES))
        message(FATAL_ERROR "Specify exactly one of SOURCE or SOURCES")
    endif()
    if(NOT DEFINED LLEXT_EDK_INSTALL_DIR)
        set(LLEXT_EDK_INSTALL_DIR "$ENV{LLEXT_EDK_INSTALL_DIR}")
    endif()
    if(NOT LLEXT_EDK_INSTALL_DIR)
        message(FATAL_ERROR "LLEXT_EDK_INSTALL_DIR is not set")
    endif()
    include("${LLEXT_EDK_INSTALL_DIR}/cmake.cflags")
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    set(capability_options)
    if(APP_RESOURCE)
        if(NOT APP_RUNTIME OR NOT APP_RESOURCE_VERSION OR NOT APP_RESOURCE_DIRECTORY)
            message(FATAL_ERROR "RESOURCE requires RUNTIME, RESOURCE_VERSION and RESOURCE_DIRECTORY")
        endif()
        get_filename_component(resource_source "${APP_RESOURCE}" ABSOLUTE BASE_DIR "${PROJECT_SOURCE_DIR}")
        execute_process(COMMAND "${Python3_EXECUTABLE}" "${MESHBUS_ARDUBOY_SDK_DIR}/tools/resources.py"
            --source "${resource_source}" --output "${CMAKE_CURRENT_BINARY_DIR}"
            --identity "${target_name}" --version "${APP_RESOURCE_VERSION}" --directory "${APP_RESOURCE_DIRECTORY}"
            COMMAND_ERROR_IS_FATAL ANY)
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
            "${resource_source}" "${MESHBUS_ARDUBOY_SDK_DIR}/tools/resources.py")
        list(APPEND APP_DEFINES MESHBUS_ARDUBOY_RESOURCES=1)
        list(APPEND APP_INCLUDES "${CMAKE_CURRENT_BINARY_DIR}")
        list(APPEND capability_options --resources)
    else()
        # Removing RESOURCE must not reuse the previous configure's manifest.
        file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/arduboy-resources.json")
    endif()
    if(APP_FX)
        if(NOT APP_RESOURCE OR NOT APP_RUNTIME)
            message(FATAL_ERROR "FX requires RESOURCE and RUNTIME")
        endif()
        list(APPEND APP_DEFINES MESHBUS_ARDUBOY_FX=1)
        list(APPEND capability_options --fx)
    endif()
    if(APP_RUNTIME)
        list(APPEND capability_options --runtime)
    endif()
    if(APP_COMPATIBILITY)
        list(APPEND capability_options --compatibility)
    endif()
    execute_process(COMMAND "${Python3_EXECUTABLE}" "${MESHBUS_ARDUBOY_SDK_DIR}/tools/capabilities.py"
        --sdk "${MESHBUS_ARDUBOY_SDK_DIR}" --source "${PROJECT_SOURCE_DIR}"
        --edk "${LLEXT_EDK_INSTALL_DIR}" --output "${CMAKE_CURRENT_BINARY_DIR}/arduboy-capabilities.json"
        ${capability_options} --requires ${APP_REQUIRES}
        COMMAND_ERROR_IS_FATAL ANY)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        "${MESHBUS_ARDUBOY_SDK_DIR}/capabilities.json"
        "${MESHBUS_ARDUBOY_SDK_DIR}/tools/capabilities.py"
        "${LLEXT_EDK_INSTALL_DIR}/edk-release.json")
    file(READ "${CMAKE_CURRENT_BINARY_DIR}/arduboy-capabilities.json" capability_report)
    string(JSON managed_stop GET "${capability_report}" actual managed_stop status)
    if(managed_stop STREQUAL "implemented")
        list(APPEND APP_DEFINES MESHBUS_ARDUBOY_MANAGED_STOP=1)
    endif()
    string(JSON relocatable_resources GET "${capability_report}" actual relocatable_resources status)
    if(relocatable_resources STREQUAL "implemented")
        list(APPEND APP_DEFINES MESHBUS_ARDUBOY_RELOCATABLE_RESOURCES=1)
    endif()
    set(sources ${APP_SOURCE} ${APP_SOURCES})
    if(APP_SKETCH)
        if(NOT APP_RUNTIME)
            message(FATAL_ERROR "SKETCH requires RUNTIME")
        endif()
        get_filename_component(sketch_main "${APP_SKETCH}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
        get_filename_component(sketch_dir "${sketch_main}" DIRECTORY)
        file(GLOB sketch_inputs CONFIGURE_DEPENDS "${sketch_dir}/*.ino" "${sketch_dir}/*.pde")
        file(GLOB sketch_cpp CONFIGURE_DEPENDS "${sketch_dir}/*.cpp")
        find_package(Python3 REQUIRED COMPONENTS Interpreter)
        set(generated_sketch "${CMAKE_CURRENT_BINARY_DIR}/${target_name}.generated/sketch.cpp")
        add_custom_command(OUTPUT "${generated_sketch}"
            COMMAND Python3::Interpreter "${MESHBUS_ARDUBOY_SDK_DIR}/tools/sketch.py"
                --main "${sketch_main}" --output "${generated_sketch}"
            DEPENDS ${sketch_inputs} "${MESHBUS_ARDUBOY_SDK_DIR}/tools/sketch.py" ${APP_DEPENDS}
            VERBATIM)
        list(APPEND sources "${generated_sketch}" ${sketch_cpp})
        list(APPEND APP_INCLUDES "${sketch_dir}")
    endif()
    if(APP_COMPATIBILITY)
        list(APPEND APP_DEFINES MESHBUS_ARDUBOY_COMPATIBILITY=1)
        message(WARNING "sdk-arduboy ${target_name}: explicit compatibility mode; legacy timing/audio/SPI/GPIO placeholders remain")
    endif()
    set(needs_eeprom FALSE)
    foreach(definition IN LISTS APP_DEFINES)
        if(definition MATCHES "MESHBUS_ARDUBOY_LLEXT_GAME_ENABLE_EEPROM")
            set(needs_eeprom TRUE)
        endif()
    endforeach()
    if(APP_RUNTIME)
        list(APPEND sources "${MESHBUS_ARDUBOY_SDK_DIR}/src/runtime.cpp")
        list(APPEND APP_DEFINES MESHBUS_ARDUBOY_RUNTIME=1)
        set(needs_eeprom TRUE)
    endif()
    if(needs_eeprom)
        list(APPEND sources "${MESHBUS_ARDUBOY_SDK_DIR}/src/eeprom_runtime.cpp")
    endif()
    set(cxxflags)
    foreach(flag IN LISTS LLEXT_CFLAGS)
        # EDK flags escape quotes for shell command generation; VERBATIM handles it here.
        string(REPLACE "\\\"" "\"" flag "${flag}")
        list(APPEND cxxflags "${flag}")
    endforeach()
    list(FILTER cxxflags EXCLUDE REGEX "^-std=.*")
    list(APPEND cxxflags -std=gnu++17 -fno-exceptions -fno-rtti
        -fno-threadsafe-statics -fno-use-cxa-atexit -fno-unwind-tables
        -fno-asynchronous-unwind-tables)
    # Real COMDAT/weak linkage is required when independent TUs share inline state.
    # Retain the previous legacy single-source behavior until its port is migrated.
    if(APP_SOURCE AND NOT APP_RUNTIME)
        list(APPEND cxxflags -fno-weak)
    endif()
    foreach(definition IN LISTS APP_DEFINES)
        if(definition MATCHES "^-D")
            list(APPEND cxxflags "${definition}")
        else()
            list(APPEND cxxflags "-D${definition}")
        endif()
    endforeach()
    list(APPEND cxxflags "-I${MESHBUS_ARDUBOY_COMMON_INCLUDE_DIR}")
    foreach(include_dir IN LISTS APP_INCLUDES)
        list(APPEND cxxflags "-I${include_dir}")
    endforeach()
    list(APPEND cxxflags ${APP_FLAGS})
    set(objects)
    set(seen)
    foreach(source IN LISTS sources)
        get_filename_component(source "${source}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
        if(source IN_LIST seen)
            message(FATAL_ERROR "Duplicate source: ${source}")
        endif()
        list(APPEND seen "${source}")
        string(SHA1 key "${source}")
        get_filename_component(name "${source}" NAME)
        set(object "${CMAKE_CURRENT_BINARY_DIR}/${target_name}.objects/${key}-${name}.o")
        add_custom_command(OUTPUT "${object}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/${target_name}.objects"
            COMMAND ${CMAKE_CXX_COMPILER} ${cxxflags} -MMD -MF "${object}.d" -MT "${object}"
                -c "${source}" -o "${object}"
            DEPENDS "${source}" ${APP_DEPENDS} "${LLEXT_EDK_INSTALL_DIR}/cmake.cflags"
            DEPFILE "${object}.d"
            VERBATIM COMMAND_EXPAND_LISTS)
        list(APPEND objects "${object}")
    endforeach()
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-prog-name=ld
        OUTPUT_VARIABLE relocatable_linker OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY)
    if(NOT IS_ABSOLUTE "${relocatable_linker}" OR NOT EXISTS "${relocatable_linker}")
        message(FATAL_ERROR "Compiler did not identify its cross linker: ${relocatable_linker}")
    endif()
    set(extension "${CMAKE_CURRENT_BINARY_DIR}/${target_name}.llext")
    add_custom_command(OUTPUT "${extension}" "${CMAKE_CURRENT_BINARY_DIR}/${target_name}.inc"
        COMMAND "${relocatable_linker}" -r --build-id=none --force-group-allocation
            -T "${MESHBUS_ARDUBOY_SDK_DIR}/cmake/relocatable.ld"
            ${objects} -o "${extension}"
        COMMAND xxd -ip "${extension}" "${CMAKE_CURRENT_BINARY_DIR}/${target_name}.inc"
        DEPENDS ${objects} "${MESHBUS_ARDUBOY_SDK_DIR}/cmake/relocatable.ld"
        VERBATIM COMMAND_EXPAND_LISTS)
    add_custom_target(${target_name} ALL DEPENDS "${extension}" "${CMAKE_CURRENT_BINARY_DIR}/${target_name}.inc")
endfunction()
