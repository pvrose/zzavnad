#[=[
    Copyright 2026, Philip Rose, GM3ZZA

    This file is part of ZZALOG. Amateur Radio Logging Software.

    ZZALOG is free software: you can redistribute it and/or modify it under the
    terms of the Lesser GNU General Public License as published by the Free Software
    Foundation, either version 3 of the License, or (at your option) any later version.

    ZZALOG is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
    without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
    PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with ZZALOG. 
    If not, see <https://www.gnu.org/licenses/>. 

#]=]

# CMake helper functions for Windows icon generation
# Provides functions to convert PNG files to ICO format with transparency support

# Function to generate Windows icon from PNG file
# Usage: generate_windows_icon(
#   PNG_FILE path/to/logo.png
#   OUTPUT_ICO path/to/output.ico
#   [WHITE_THRESHOLD 250]  # Optional: threshold for white transparency (0-255)
# )
function(generate_windows_icon)
    if(NOT MSVC)
        message(STATUS "Icon generation: Skipping (not MSVC)")
        return()
    endif()

    set(options "")
    set(oneValueArgs PNG_FILE OUTPUT_ICO WHITE_THRESHOLD)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Validate required arguments
    if(NOT ARG_PNG_FILE)
        message(FATAL_ERROR "generate_windows_icon: PNG_FILE is required")
    endif()

    if(NOT ARG_OUTPUT_ICO)
        message(FATAL_ERROR "generate_windows_icon: OUTPUT_ICO is required")
    endif()

    # Set default white threshold if not provided
    if(NOT DEFINED ARG_WHITE_THRESHOLD)
        set(ARG_WHITE_THRESHOLD 250)
    endif()

    # Check if PNG file exists
    if(NOT EXISTS "${ARG_PNG_FILE}")
        message(WARNING "Icon generation: PNG file not found: ${ARG_PNG_FILE}")
        return()
    endif()

    # Find the convert_png_to_ico.cmake script
    set(CONVERTER_SCRIPT "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/convert_png_to_ico.cmake")
    if(NOT EXISTS "${CONVERTER_SCRIPT}")
        message(WARNING "Icon generation: Converter script not found: ${CONVERTER_SCRIPT}")
        return()
    endif()

    # Generate the icon
    message(STATUS "Icon generation: Converting ${ARG_PNG_FILE} to ${ARG_OUTPUT_ICO}")
    message(STATUS "Icon generation: White threshold = ${ARG_WHITE_THRESHOLD}")

    execute_process(
        COMMAND ${CMAKE_COMMAND}
            -DPNG_FILE=${ARG_PNG_FILE}
            -DICO_FILE=${ARG_OUTPUT_ICO}
            -DWHITE_THRESHOLD=${ARG_WHITE_THRESHOLD}
            -P ${CONVERTER_SCRIPT}
        RESULT_VARIABLE ICO_RESULT
        ERROR_VARIABLE ICO_ERROR
        OUTPUT_VARIABLE ICO_OUTPUT
    )

    if(NOT ICO_RESULT EQUAL 0)
        message(WARNING "Icon generation: Failed to generate ICO file: ${ICO_ERROR}")
    else()
        message(STATUS "Icon generation: Successfully created ${ARG_OUTPUT_ICO}")
    endif()

    # Export the output path to parent scope
    get_filename_component(ICO_FILENAME "${ARG_OUTPUT_ICO}" NAME)
    set(${PROJECT_NAME}_ICO_FILE "${ARG_OUTPUT_ICO}" PARENT_SCOPE)
endfunction()

# Function to configure Windows resource file with icon and version info
# Usage: configure_windows_resource(
#   RC_TEMPLATE path/to/template.rc.in
#   OUTPUT_RC path/to/output.rc
#   ICO_FILE path/to/icon.ico
# )
function(configure_windows_resource)
    if(NOT MSVC)
        message(STATUS "Resource file: Skipping (not MSVC)")
        return()
    endif()

    set(options "")
    set(oneValueArgs RC_TEMPLATE OUTPUT_RC ICO_FILE)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Validate required arguments
    if(NOT ARG_RC_TEMPLATE)
        message(FATAL_ERROR "configure_windows_resource: RC_TEMPLATE is required")
    endif()

    if(NOT ARG_OUTPUT_RC)
        message(FATAL_ERROR "configure_windows_resource: OUTPUT_RC is required")
    endif()

    # Check if template exists
    if(NOT EXISTS "${ARG_RC_TEMPLATE}")
        message(WARNING "Resource file: Template not found: ${ARG_RC_TEMPLATE}")
        return()
    endif()

    # Configure the resource file
    configure_file(
        ${ARG_RC_TEMPLATE}
        ${ARG_OUTPUT_RC}
        @ONLY
    )

    message(STATUS "Resource file: Generated ${ARG_OUTPUT_RC}")

    # Export the output path to parent scope
    set(${PROJECT_NAME}_RC_FILE "${ARG_OUTPUT_RC}" PARENT_SCOPE)
endfunction()

# Convenience function that generates both icon and resource file
# Usage: generate_windows_icon_and_resource(
#   PNG_FILE path/to/logo.png
#   RC_TEMPLATE path/to/template.rc.in
#   [WHITE_THRESHOLD 250]  # Optional
# )
function(generate_windows_icon_and_resource)
    if(NOT MSVC)
        return()
    endif()

    set(options "")
    set(oneValueArgs PNG_FILE RC_TEMPLATE WHITE_THRESHOLD)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Validate required arguments
    if(NOT ARG_PNG_FILE)
        message(FATAL_ERROR "generate_windows_icon_and_resource: PNG_FILE is required")
    endif()

    if(NOT ARG_RC_TEMPLATE)
        message(FATAL_ERROR "generate_windows_icon_and_resource: RC_TEMPLATE is required")
    endif()

    # Set default paths
    get_filename_component(PNG_NAME "${ARG_PNG_FILE}" NAME_WE)
    set(OUTPUT_ICO "${CMAKE_CURRENT_BINARY_DIR}/${PNG_NAME}.ico")
    set(OUTPUT_RC "${CMAKE_CURRENT_BINARY_DIR}/${PNG_NAME}.rc")

    # Set default threshold
    if(NOT DEFINED ARG_WHITE_THRESHOLD)
        set(ARG_WHITE_THRESHOLD 250)
    endif()

    # Generate icon
    generate_windows_icon(
        PNG_FILE ${ARG_PNG_FILE}
        OUTPUT_ICO ${OUTPUT_ICO}
        WHITE_THRESHOLD ${ARG_WHITE_THRESHOLD}
    )

    # Configure resource file
    configure_windows_resource(
        RC_TEMPLATE ${ARG_RC_TEMPLATE}
        OUTPUT_RC ${OUTPUT_RC}
        ICO_FILE ${OUTPUT_ICO}
    )

    # Export paths to parent scope
    set(${PROJECT_NAME}_ICO_FILE "${OUTPUT_ICO}" PARENT_SCOPE)
    set(${PROJECT_NAME}_RC_FILE "${OUTPUT_RC}" PARENT_SCOPE)
endfunction()
