#[=[
	Copyright 2026, Philip Rose, GM3ZZA
	
    This file is part of ZZAVNAD. Amateur Radio Logging Software.

    ZZAVNAD is free software: you can redistribute it and/or modify it under the
	terms of the Lesser GNU General Public License as published by the Free Software
	Foundation, either version 3 of the License, or (at your option) any later version.

    ZZAVNAD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
	PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with ZZAVNAD. 
	If not, see <https://www.gnu.org/licenses/>. 

#]=]
find_package(Doxygen)

if(DOXYGEN_FOUND)

  message(STATUS "Doxygen found: ${DOXYGEN_EXECUTABLE}")

  # Set project files
  set(DOXY_API_INDEX_FILE index.html)
  set(DOXY_API_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set(DOXY_API_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  # Set source files
  set(DOXY_API_FILES 
    ${HPPFILES}
  )

  # Copy Doxyfile
  configure_file(${DOXY_API_IN} ${DOXY_API_OUT} @ONLY)

  # Set "real" dependency
  set(DOXY_OUTPUT_INDEX "${CMAKE_CURRENT_BINARY_DIR}/html/index.html")

  # Custom Command: This is the actual command that will run Doxygen
  add_custom_command(
    OUTPUT ${DOXY_OUTPUT_INDEX}
    DEPENDS ${DOXY_API_FILES} ${DOXY_API_OUT} zzacommon_api_html
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${ZZACOMMON_API_HTML_DIR}"
      "${CMAKE_CURRENT_BINARY_DIR}/html/zzacommon"
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXY_API_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "ZZAVNAD: Generating API documentation (smart tracking)"
    VERBATIM
  )

  #The Custom Target: This gives you the clickable menu item in Visual Studio
  add_custom_target(api_html
    DEPENDS ${DOXY_OUTPUT_INDEX}
  )

  # Set output directory for API HTML
  set(ZZAVNAD_API_HTML_DIR ${CMAKE_CURRENT_BINARY_DIR}/html PARENT_SCOPE)

endif()
