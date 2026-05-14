cmake_minimum_required(VERSION 3.16)

project(ZZAVNAD VERSION 0.1) 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

## Set home directory per OS.
if(MSVC)
  set(USER_HOME_DIR $ENV{USERPROFILE})
else()
  set(USER_HOME_DIR $ENV{HOME})
endif()

## Set program details
set(APP_VENDOR "GM3ZZA")
set(APP_NAME "ZZAVNAD")
string(TIMESTAMP APP_TIMESTAMP UTC)
set(ZZAVNAD_RC_VALUE "")
set(APP_VERSION "${ZZAVNAD_VERSION}${ZZAVNAD_RC_VALUE}")
set (TARGET zzavnad )
if (NOT MSVC)
  set(BUILD_SHARED_LIBS OFF)
endif()

## Split version into components for Windows resource file
string(REPLACE "." ";" VERSION_LIST ${ZZAVNAD_VERSION})
list(LENGTH VERSION_LIST VERSION_LIST_LENGTH)
list(GET VERSION_LIST 0 ZZAVNAD_VERSION_MAJOR)
list(GET VERSION_LIST 1 ZZAVNAD_VERSION_MINOR)
# Handle PATCH component (default to 0 if not provided)
if(VERSION_LIST_LENGTH GREATER 2)
    list(GET VERSION_LIST 2 ZZAVNAD_VERSION_PATCH)
else()
    set(ZZAVNAD_VERSION_PATCH 0)
endif()
# Handle TWEAK component (default to 0 if not provided)
if(VERSION_LIST_LENGTH GREATER 3)
    list(GET VERSION_LIST 3 ZZAVNAD_VERSION_TWEAK)
else()
    set(ZZAVNAD_VERSION_TWEAK 0)
endif()



set(CPPFILES 
  ${ZZAVNAD_SOURCE_DIR}/src/app.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/calib_data.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/display.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/display_control.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/display_legend.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/main.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/markers.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/nvna_control.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/nvna_iface.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/source_control.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/sp_data.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/window.cpp
)

# set directory to install executable, reference data and documents.
if(MSVC)
  set(CMAKE_INSTALL_RPATH etc)
else()
  set(CMAKE_INSTALL_RPATH etc/${TARGET})
endif()

# ser compiler options
if (MSVC)
#s  add_compile_options(/MD)
else()
  add_compile_options(-g)
endif()

# Add ZZACOMMON
# Try to find zzacommon locally first (sibling directory), fall back to downloading
if(CMAKE_BUILD_TYPE)
  # Single-config generator (Unix Makefiles, Ninja): use specific build type
  string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE_LOWER)
  list(APPEND CMAKE_PREFIX_PATH 
    "${CMAKE_CURRENT_SOURCE_DIR}/../zzacommon/out/build/x64-${BUILD_TYPE_LOWER}"
    "${CMAKE_CURRENT_SOURCE_DIR}/../zzacommon/out/install/x64-${BUILD_TYPE_LOWER}"
  )
endif()
# Multi-config generator (Visual Studio) or fallback: include both
list(APPEND CMAKE_PREFIX_PATH 
  "${CMAKE_CURRENT_SOURCE_DIR}/../zzacommon/out/build/x64-debug"
  "${CMAKE_CURRENT_SOURCE_DIR}/../zzacommon/out/build/x64-release"
  "${CMAKE_CURRENT_SOURCE_DIR}/../zzacommon/out/install/x64-debug"
  "${CMAKE_CURRENT_SOURCE_DIR}/../zzacommon/out/install/x64-release"
  "${CMAKE_CURRENT_SOURCE_DIR}/../zzacommon/build"
  "${CMAKE_CURRENT_SOURCE_DIR}/../zzacommon/install"
)

include(FetchContent)
FetchContent_Declare(
  zzacommon
  GIT_REPOSITORY https://github.com/pvrose/zzacommon.git
  GIT_TAG v1.1.3
  FIND_PACKAGE_ARGS NAMES zzacommon COMPONENTS zzaf zzam zzad zzab zzafb
)
FetchContent_MakeAvailable(zzacommon)

configure_file(
  ${ZZAVNAD_SOURCE_DIR}/src/app.cpp.in
  ${ZZAVNAD_SOURCE_DIR}/src/app.cpp
  @ONLY
)

## Reference data
set(DATAFILES
  ${ZZAVNAD_SOURCE_DIR}/rose.png 
)

# Generate Windows icon and resource file
if (MSVC)
  # Include the icon helper functions
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/icon_helper.cmake)

  # Optional: Set white threshold for transparency (default: 250, range: 0-255)
  # Higher values = more aggressive white removal (e.g., 240 removes near-white)
  # Lower values = only pure white removed (e.g., 254 removes almost pure white)
  if(NOT DEFINED ZZAVNAD_ICON_WHITE_THRESHOLD)
    set(ZZAVNAD_ICON_WHITE_THRESHOLD 240)
  endif()

  # Generate icon and resource file using the helper function
  generate_windows_icon_and_resource(
    PNG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/rose.png"
    RC_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/zzavnad.rc.in"
    WHITE_THRESHOLD ${ZZAVNAD_ICON_WHITE_THRESHOLD}
  )

  # The function exports ${PROJECT_NAME}_RC_FILE which we'll use below
  # For project ZZAVNAD, this will be ${ZZAVNAD_RC_FILE}
  message(STATUS "Using resource file: ${ZZAVNAD_RC_FILE}")
endif()


# Define the zzavnad executable
if (MSVC)
  add_executable(${TARGET} ${ZZAVNAD_RC_FILE})
else()
  add_executable(${TARGET})
endif()
if (MSVC)
  add_dependencies(${TARGET} 
  zzacommon::zzaf
  zzacommon::zzam
  zzacommon::zzad
  zzacommon::zzab
  zzacommon::zzafb
  )
else()
#  add_dependencies(${TARGET} zzacommon ${ZC_APP_LIBRARY})
endif()

target_include_directories(${TARGET} PRIVATE 
  ${ZZAVNAD_SOURCE_DIR}/include
  ${ZZACOMMON_INCLUDE_DIR}
  ${FLTK_INCLUDE_DIR}
)
# Needed for boost::asio on Windows.
# This is set in zzacommon, but we need it here too for the executable target.
# TODO: How can we set this in zzacommon/CMakeLists.txt to propagate it to this target?
target_compile_definitions(${TARGET} PRIVATE
  _WIN32_WINNT=0x0601
)
## Extra include directories required on MSVC
if (MSVC)
  message(STATUS "Including other libraries for MSVC from ${USER_HOME_DIR}")
  target_include_directories(${TARGET} PRIVATE 
    "${USER_HOME_DIR}\\source\\repos\\json\\include"
  )
endif()

target_sources(${TARGET} PRIVATE ${CPPFILES})

target_link_libraries(${TARGET} PRIVATE
  zzacommon::zzaf
  zzacommon::zzam
  zzacommon::zzad
  zzacommon::zzab
  zzacommon::zzafb
    ${FLTK_LIBRARIES}
)

# Copy runtime DLLs to build directory for MSVC
if (MSVC)
  # Get the parent project's binary directory
  set(RUNTIME_OUTPUT_DIR ${CMAKE_BINARY_DIR})

  # Use CMake's automatic runtime DLL detection
  add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_RUNTIME_DLLS:${TARGET}>
      ${RUNTIME_OUTPUT_DIR}
    COMMAND_EXPAND_LISTS
    COMMENT "Copying runtime DLLs to ${RUNTIME_OUTPUT_DIR}"
  )

endif()

# copy reference data
add_custom_target(reference_data ALL)
message(STATUS "Reference data: ${DATAFILES}")
foreach(F ${DATAFILES})
  add_custom_command(TARGET reference_data POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${F} ${ZZAVNAD_BINARY_DIR}
    COMMENT "Copying ${F} to ${ZZAVNAD_BINARY_DIR}"
  )
endforeach()

install(TARGETS ${TARGET})
install(FILES ${DATAFILES}
    DESTINATION ${CMAKE_INSTALL_RPATH})

    # install DLLs
if (MSVC)
    zzacommon_install_runtime_dlls(DESTINATION bin COMPONENT application)
endif()
