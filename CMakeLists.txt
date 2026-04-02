cmake_minimum_required(VERSION 3.16)

project(ZZAVNAD VERSION 0.0.1) 

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

set(CPPFILES 
  ${ZZAVNAD_SOURCE_DIR}/src/display.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/display_control.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/display_legend.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/main.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/nvna_control.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/nvna_iface.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/source_control.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/sp_data.cpp
  ${ZZAVNAD_SOURCE_DIR}/src/window.cpp
)

# set directory to install executable, reference data and documents.
if(MSVC)
  set(CMAKE_INSTALL_RPATH C:/ProgramData/${APP_VENDOR}/${APP_NAME})
else()
  set(CMAKE_INSTALL_RPATH /etc/${APP_VENDOR}/${APP_NAME})
endif()

# ser compiler options
if (MSVC)
#s  add_compile_options(/MD)
else()
  add_compile_options(-g)
endif()

  # Add ZZACOMMON
add_subdirectory(zzacommon)
message(STATUS "FLTK include dir ${FLTK_INCLUDE_DIR}")
message(STATUS "FLTK libs - ${FLTK_LIBRARIES}")

## Reference data
set(DATAFILES
  ${ZZAVNAD_SOURCE_DIR}/rose.png 
)

add_executable(${TARGET})
if (MSVC)
  add_dependencies(${TARGET} zzacommon ${ZC_APP_LIBRARY})
else()
  add_dependencies(${TARGET} zzacommon ${ZC_APP_LIBRARY})
endif()

target_include_directories(${TARGET} PRIVATE 
  ${ZZAVNAD_SOURCE_DIR}/include
  ${ZZACOMMON_INCLUDE_DIR}
  ${ZC_APP_LIBRARY}
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

target_sources(${TARGET} PRIVATE ${CPPFILES}  "src/nvna_iface.cpp")

target_link_libraries(${TARGET} PRIVATE
    ${ZZACOMMON_LIBRARIES}
    ${ZC_APP_LIBRARY}
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
  # The following has been included from zzacommon.
  copy_zlib_dlls()    

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

