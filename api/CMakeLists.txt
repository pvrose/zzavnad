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

  setup_api_documentation(
      ZZAVNAD
      "${ZZAVNAD_HPPFILES}"
  )

  set(ZZAVNAD_API_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/html" PARENT_SCOPE)


endif()
