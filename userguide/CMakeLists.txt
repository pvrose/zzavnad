#[=[
	Copyright 2026, Philip Rose, GM3ZZA
	
    This file is part of ZZAVNAD. VNA Analysis Software.

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
  set(DOXY_UG_INDEX_FILE index.html)
  set(DOXY_UG_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set(DOXY_UG_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile_ug)

  set(DOXY_UG_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/index.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/data.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/displays.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/howto.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/overview.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/release_notes.dox
)

  set(DOXY_UG_IMAGES 
    ${CMAKE_CURRENT_SOURCE_DIR}/images/overview.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/overview_calib.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/overview_connect.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/overview_disp.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/overview_help.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/overview_mark.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/overview_scan.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/overview_srcs.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/calibration_1.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/calibration_2.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/calibration_3.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/calibration_4.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/compare_1.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/compare_2.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/compare_3.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/compare_4.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/connect_1.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/connect_2.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/display_1.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/display_2.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/display_3.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/display_4.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/display_5.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/display_6.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/display_7.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/display_8.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/files_1.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/files_2.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/marker_1.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/marker_2.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/scan_1.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/scan_2.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/scan_3.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/zoom_1.png
  )



  # Copy Doxyfile
  configure_file(${DOXY_UG_IN} ${DOXY_UG_OUT} @ONLY)
 
  # Define the main output file that Doxygen produces
  set(DOXY_OUTPUT_INDEX "${CMAKE_CURRENT_BINARY_DIR}/html/index.html")

  # The Custom Command: This handles the actual dependency checking
  add_custom_command(
    OUTPUT ${DOXY_OUTPUT_INDEX}
    DEPENDS ${DOXY_UG_FILES} ${DOXY_UG_OUT} ${DOXY_UG_IMAGES}
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXY_UG_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "ZZALOG: Generating Userguide documentation (smart tracking)"
    VERBATIM
  )

  # The Custom Target: This gives you the clickable menu item in Visual Studio
  add_custom_target(ug_html
    DEPENDS ${DOXY_OUTPUT_INDEX}
  )
 
      # Command for generating PDF from userguide latex files
      if(MSVC)
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf
            COMMAND miktex-texworks.exe  ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.tex
            COMMENT "ZZALOG: Generating PDF Userguide"
            VERBATIM
        )
      else()
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf
            COMMAND make -C ${CMAKE_CURRENT_BINARY_DIR}/latex >> /dev/null
            COMMENT "ZZALOG: Generating PDF Userguide"
            VERBATIM
        )
      endif()

      add_custom_target(ug_pdf
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf ug_html
      )
      
      # Copy from latex/refman.pdf to ZZALOG.pdf
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${APP_NAME}.pdf
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf
        ${CMAKE_CURRENT_BINARY_DIR}/${APP_NAME}.pdf
        COMMENT "Copying PDF"
      )
      add_custom_target(pdf
         DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${APP_NAME}.pdf ug_pdf
      )
endif()
