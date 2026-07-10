cmake_minimum_required(VERSION 3.16)

# Add shared CMake modules
# Add shared CMake modules
# First look in sibling path for development, then in installed location for users
include(FetchContent)

# Try to find zzacmake as a sibling project first
set(ZZACMAKE_SIBLING_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../zzacmake")
if(EXISTS "${ZZACMAKE_SIBLING_DIR}/cmake")
    set(zzacmake_SOURCE_DIR "${ZZACMAKE_SIBLING_DIR}")
    message(STATUS "Using zzacmake from sibling project: ${zzacmake_SOURCE_DIR}")
else()
    # Fall back to fetching from GitHub
    FetchContent_Declare(
        zzacmake
        GIT_REPOSITORY https://github.com/pvrose/zzacmake.git
        GIT_TAG main
    )
    FetchContent_MakeAvailable(zzacmake)
    message(STATUS "Fetched zzacmake from GitHub: ${zzacmake_SOURCE_DIR}")
endif()

list(APPEND CMAKE_MODULE_PATH "${zzacmake_SOURCE_DIR}/cmake")

include(Gm3zzaProjectDefaults)
include(Gm3zzaVersioning)
include(Gm3zzaZzacommon)
include(Gm3zzaResources)
include(Gm3zzaWindowsRuntime)
include(Gm3zzaDocs)
include(deps/fftw_helper OPTIONAL)

gm3zza_get_git_version(MY_DYNAMIC_VERSION)

project(ZZAVNAD VERSION ${MY_DYNAMIC_VERSION})
message(STATUS "Project version: ${ZZALOG_VERSION}")

## Set target executable and program metadata
set(TARGET zzavnad)
set(APP_VENDOR "GM3ZZA")
set(APP_NAME "ZZAVNAD")
set(APP_RC_VALUE "")
string(TIMESTAMP APP_TIMESTAMP UTC)
set(APP_SOURCE_DIR ${ZZAVNAD_SOURCE_DIR})

## Initialise defaults and settings
gm3zza_project_defaults()
gm3zza_set_default_install_dirs("${APP_NAME}")
gm3zza_split_version(${PROJECT_NAME})

## Set version string for app.cpp.in
set(APP_VERSION "${ZZAVNAD_VERSION}${APP_RC_VALUE}")

## Set source files (excluding app.cpp which is generated)
set(ZZAVNAD_CPPFILES 
  ${ZZAVNAD_SOURCE_DIR}/src/calib_data.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/display.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/display_control.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/display_legend.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/main.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/marker_table.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/markers.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/nvna_control.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/nvna_iface.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/source_control.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/sp_data.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/window.cpp
)

## Set include files for API documentation generation
set(ZZAVNAD_HPPFILES
  ${ZZAVNAD_SOURCE_DIR}/include/calib_data.hpp
  ${ZZAVNAD_SOURCE_DIR}/include/display.hpp
  ${ZZAVNAD_SOURCE_DIR}/include/display_control.hpp
  ${ZZAVNAD_SOURCE_DIR}/include/display_legend.hpp
  ${ZZAVNAD_SOURCE_DIR}/include/main.hpp
  ${ZZAVNAD_SOURCE_DIR}/include/marker_table.hpp
  ${ZZAVNAD_SOURCE_DIR}/include/markers.hpp
  ${ZZAVNAD_SOURCE_DIR}/include/nvna_control.hpp
  ${ZZAVNAD_SOURCE_DIR}/include/nvna_iface.hpp
  ${ZZAVNAD_SOURCE_DIR}/include/source_control.hpp
  ${ZZAVNAD_SOURCE_DIR}/include/sp_data.hpp
  ${ZZAVNAD_SOURCE_DIR}/include/window.hpp
  ${ZZAVNAD_SOURCE_DIR}/include/displays/s11_ma.hpp
  ${ZZAVNAD_SOURCE_DIR}/include/displays/s11_raw.hpp
  ${ZZAVNAD_SOURCE_DIR}/include/displays/s11_rj.hpp
  ${ZZAVNAD_SOURCE_DIR}/include/displays/s11_rx.hpp
  ${ZZAVNAD_SOURCE_DIR}/include/displays/s11_smith.hpp
  ${ZZAVNAD_SOURCE_DIR}/include/displays/s11_swr.hpp
  ${ZZAVNAD_SOURCE_DIR}/include/displays/s11_tdr.hpp
  ${ZZAVNAD_SOURCE_DIR}/include/displays/s11_z_polar.hpp
  ${ZZAVNAD_SOURCE_DIR}/include/displays/s21_gain.hpp
)

## Data files
set(DATAFILES
  ${ZZAVNAD_SOURCE_DIR}/rose.png 
  ${ZZAVNAD_SOURCE_DIR}/pdf.png
)

## Generate Windows icon and resource file
if(MSVC)
  if(NOT DEFINED ZZAVNAD_ICON_WHITE_THRESHOLD)
    set(ZZAVNAD_ICON_WHITE_THRESHOLD 240)
  endif()
  
  gm3zza_enable_windows_icon(
    TARGET ${TARGET}
    PNG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/rose.png"
    RC_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/zzavnad.rc.in"
    WHITE_THRESHOLD ${ZZAVNAD_ICON_WHITE_THRESHOLD}
    OUTPUT_VAR APP_RC_FILE
  )
endif()

## FFTW library integration
gm3zza_find_fftw()
message(STATUS "FFTW include dir: ${FFTW3_INCLUDE_DIRS}")   
message(STATUS "FFTW libraries: ${FFTW3_LIBRARIES}")


## Source file generation in binary tree
gm3zza_generate_app_source(
  INPUT "${ZZAVNAD_SOURCE_DIR}/src/app.cpp.in"
  OUTPUT_VAR GENERATED_APP_CPP
)

set(ALL_CPPFILES ${GENERATED_APP_CPP} ${ZZAVNAD_CPPFILES})

## Use shared zzacommon (includes zzaf, zzam, zzad, zzab, zzafb)
gm3zza_use_zzacommon(COMPONENTS zzaf zzam zzad zzab zzafb GIT_TAG v1.2.8)


# Define the zzavnad executable
if(MSVC)
  add_executable(${TARGET} ${APP_RC_FILE})
else()
  add_executable(${TARGET})
endif()

if(MSVC)
  add_dependencies(${TARGET} 
    zzacommon::zzaf
    zzacommon::zzam
    zzacommon::zzad
    zzacommon::zzab
    zzacommon::zzafb
  )
endif()


add_subdirectory(${ZZAVNAD_SOURCE_DIR}/userguide EXCLUDE_FROM_ALL)
add_subdirectory(${ZZAVNAD_SOURCE_DIR}/api EXCLUDE_FROM_ALL)

add_custom_target(docs
    COMMENT "Building all project documentation"
)

add_dependencies(docs ug_html pdf api_html zzacommon_api_html)

target_include_directories(${TARGET} PRIVATE 
  ${ZZAVNAD_SOURCE_DIR}/include
  ${ZZACOMMON_INCLUDE_DIR}
  ${FLTK_INCLUDE_DIR}
  ${FFTW3_INCLUDE_DIRS}
)

if(MSVC)
  message(STATUS "Including other libraries for MSVC from ${USER_HOME_DIR}")
  target_include_directories(${TARGET} PRIVATE 
    "${USER_HOME_DIR}\\source\\repos\\json\\include"
  )
endif()

target_sources(${TARGET} PRIVATE ${ALL_CPPFILES})

target_link_libraries(${TARGET} PRIVATE
  zzacommon::zzaf
  zzacommon::zzam
  zzacommon::zzad
  zzacommon::zzab
  zzacommon::zzafb
  ${FLTK_LIBRARIES}
  FFTW3::fftw3
)

## Stage runtime assets for development
gm3zza_stage_runtime_files(TARGET ${TARGET} FILES ${DATAFILES})

## Copy runtime DLLs for development
gm3zza_copy_runtime_dlls(TARGET ${TARGET})

## Install
install(TARGETS ${TARGET} RUNTIME COMPONENT applications)
gm3zza_install_runtime_files(FILES ${DATAFILES} COMPONENT data)

if(MSVC)
  gm3zza_register_fftw_dlls()
  zzacommon_install_runtime_dlls(TARGET ${TARGET} DESTINATION bin COMPONENT application)
  gm3zza_install_registered_runtime_dlls()
endif()


# Install userguide and API documentation
    if(ZZAVNAD_USERGUIDE_HTML_DIR)
      gm3zza_install_runtime_files(
        DIRECTORIES ${ZZAVNAD_USERGUIDE_HTML_DIR}
        DEST_SUBDIR userguide
        COMPONENT documentation
      )
    endif()

    if(ZZAVNAD_USERGUIDE_PDF_FILE)
      gm3zza_install_runtime_files(
        FILES ${ZZAVNAD_USERGUIDE_PDF_FILE}
        DEST_SUBDIR userguide
        COMPONENT documentation
      )
    endif()

    if(ZZAVNAD_API_HTML_DIR)
      gm3zza_install_runtime_files(
        DIRECTORIES ${ZZAVNAD_API_HTML_DIR}
        DEST_SUBDIR api
        COMPONENT documentation
      )
    endif()
