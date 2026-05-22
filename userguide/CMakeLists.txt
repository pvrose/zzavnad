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
  )

  set(DOXY_UG_IMAGES 
    ${CMAKE_CURRENT_SOURCE_DIR}/images/mag.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/randx.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/rvx.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/s11raw.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/smith.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/swr.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/tdr.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/tdr2.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/zpolar.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/zzavnad.png
  )



  # Copy Doxyfile
  configure_file(${DOXY_UG_IN} ${DOXY_UG_OUT} @ONLY)
 
  # Command for generating dox from config file
  message("Adding command to generate: ${CMAKE_CURRENT_BINARY_DIR}/html/index.html")
  
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
    DEPENDS ${DOXY_UG_FILES} ${DOXY_UG_OUT} ${DOXY_UG_IMAGES}
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXY_UG_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "ZZAVNAD: Generating userguide"
    VERBATIM
  )
  add_custom_target(ug_html ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
  )
  
  if(ZZAVNAD_PDF)
      # Command for generating PDF from userguide latex files
      if(MSVC)
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
                    ug_html
            COMMAND miktex-texworks.exe  ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.tex
            COMMENT "ZZAVNAD: Generating PDF Userguide"
            VERBATIM
        )
      else()
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
                    ug_html
            COMMAND make -C ${CMAKE_CURRENT_BINARY_DIR}/latex >> /dev/null
            COMMENT "ZZAVNAD: Generating PDF Userguide"
            VERBATIM
        )
      endif()
      add_custom_target(ug_pdf ALL
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf
      )
      # Copy from latex/refman.pdf to ZZAVNAD.pdf
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${APP_NAME}.pdf
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf
                ug_pdf
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf
        ${CMAKE_CURRENT_BINARY_DIR}/${APP_NAME}.pdf
        COMMENT "Copying PDF"
      )
      add_custom_target(
        pdf ALL
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${APP_NAME}.pdf
      )
  endif()
endif()
