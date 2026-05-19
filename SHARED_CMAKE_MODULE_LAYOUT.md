# Shared CMake module layout draft

Date: 19 May 2026

This draft assumes a reusable shared CMake layer for:
- `qbs`
- `zzalog`
- `zzatts`
- `zzavnad`
- future GM3ZZA projects

## Design goals

The shared layout should:
- keep each project `CMakeLists.txt` small;
- standardise configure, build, runtime-copy, and install behaviour;
- allow project-specific dependencies without forking the common flow;
- work with current `CMakePresets.json` usage;
- avoid writing generated files back into the source tree.

## Recommended repository shape

The cleanest structure is a dedicated shared repository or shared sibling directory, for example:

```text
zzacmake/
  cmake/
    Gm3zzaProjectDefaults.cmake
    Gm3zzaVersioning.cmake
    Gm3zzaZzacommon.cmake
    Gm3zzaTargetSetup.cmake
    Gm3zzaResources.cmake
    Gm3zzaWindowsRuntime.cmake
    Gm3zzaPresets.cmake
    Gm3zzaDocs.cmake
    deps/
      fftw_helper.cmake
      hamlib_helper.cmake
      piper_helper.cmake
      portaudio_helper.cmake
    windows/
      icon_helper.cmake
      convert_png_to_ico.cmake
  templates/
    CMakeLists.app.in
    CMakePresets.json.in
  docs/
    README.md
    migration-guide.md
```

If a separate repository is not wanted yet, the same structure can live temporarily in a sibling directory such as `../zzacmake/cmake`.

## Shared module distribution models

The shared module idea does not require a sibling checkout, but that is one valid delivery option.

### Model A: sibling repository

Example layout:

```text
workspace/
  qbs/
  zzacmake/
```

How projects consume it:
- append `../zzacmake/cmake` to `CMAKE_MODULE_PATH`.

Pros:
- fastest to iterate while designing the shared modules;
- easiest to update all projects locally.

Cons:
- requires two checkouts for users;
- less reproducible unless everyone pins the same `zzacmake` revision.

Best use:
- early development and migration.

### Model B: submodule or subtree vendoring

Example layout:

```text
qbs/
  cmake/
    shared/   # submodule or subtree of zzacmake/cmake
```

How projects consume it:
- include shared modules from inside the same repository.

Pros:
- single project checkout for users;
- project pins its own known-good shared module revision;
- easiest reproducibility per project.

Cons:
- updates to shared modules must be propagated to each repository;
- slightly more repository management overhead.

Best use:
- current phase for your projects if you want low friction for users.

### Model C: installable CMake package

How projects consume it:
- `find_package(Gm3zzaCMake CONFIG REQUIRED)` (or equivalent package name), then include exported modules.

Pros:
- cleanest long-term architecture;
- supports system or CI installation once and reuse everywhere;
- versioned package semantics are explicit.

Cons:
- highest initial setup effort (package config export, install rules, versioning strategy).

Best use:
- medium-to-long term once module APIs stabilise.

## Recommendation for your projects

Recommended rollout:

1. start with **Model A** while extracting and proving the modules;
2. move to **Model B** for `qbs`, `zzavnad`, `zzatts`, and `zzalog` so users only need one checkout per project;
3. optionally evolve to **Model C** later.

So your understanding is correct for the sibling-repository approach: users would need both the target project and `zzacmake` checked out as siblings.

## Module responsibilities

### 1. `Gm3zzaProjectDefaults.cmake`

Purpose:
- establish the common project policy layer.

Responsibilities:
- set minimum supported CMake policy behaviour;
- set `CMAKE_CXX_STANDARD 17`, `CMAKE_CXX_STANDARD_REQUIRED ON`, `CMAKE_CXX_EXTENSIONS OFF`;
- set `BUILD_SHARED_LIBS OFF` by default for current app projects;
- set `CMAKE_EXPORT_COMPILE_COMMANDS ON`;
- derive a portable `USER_HOME_DIR`;
- define a dedicated data install variable such as `APP_DATA_INSTALL_DIR`;
- define common options such as `GM3ZZA_ENABLE_PDB` or `GM3ZZA_ENABLE_DOCS`.

Suggested entry points:
- `gm3zza_project_defaults()`
- `gm3zza_define_standard_options()`
- `gm3zza_set_default_install_dirs(TARGET_NAME <name>)`

### 2. `Gm3zzaVersioning.cmake`

Purpose:
- normalise version metadata and generated source handling.

Responsibilities:
- split project version into major, minor, patch, tweak values;
- provide defaults for missing version components;
- generate `app.cpp` into the build tree, not the source tree;
- provide a consistent interface for Windows resource metadata.

Suggested entry points:
- `gm3zza_split_version(PROJECT_NAME <name>)`
- `gm3zza_generate_app_source(INPUT <template> OUTPUT_VAR <var>)`

Expected effect:
- replaces current repeated logic from [../qbs/CMakeLists.txt](../qbs/CMakeLists.txt#L24-L35), [../zzalog/CMakeLists.txt](../zzalog/CMakeLists.txt#L28-L43), and [CMakeLists.txt](CMakeLists.txt#L21-L35)
- standardises generated file placement across all projects

### 3. `Gm3zzaZzacommon.cmake`

Purpose:
- centralise `zzacommon` discovery and fallback fetch.

Responsibilities:
- search sibling `zzacommon` build and install directories;
- support both single-config and multi-config generators;
- accept requested `zzacommon` components as arguments;
- accept preferred tag, branch, or commit;
- optionally force documentation-related settings;
- expose a single project-facing function.

Suggested entry point:
- `gm3zza_use_zzacommon(COMPONENTS ... GIT_TAG <tag> [ENABLE_DOCS])`

Expected effect:
- replaces the repeated blocks in [../qbs/CMakeLists.txt](../qbs/CMakeLists.txt#L114-L136), [../zzalog/CMakeLists.txt](../zzalog/CMakeLists.txt#L321-L356), [../zzatts/CMakeLists.txt](../zzatts/CMakeLists.txt#L68-L94), and [CMakeLists.txt](CMakeLists.txt#L86-L111)

### 4. `Gm3zzaTargetSetup.cmake`

Purpose:
- provide the common executable creation pattern.

Responsibilities:
- create the application target;
- optionally attach a generated Windows `.rc` file;
- add source files;
- add include directories;
- add compile definitions;
- add link libraries;
- allow optional `WIN32` and `MACOSX_BUNDLE` target flags.

Suggested entry points:
- `gm3zza_add_application(TARGET <name> [WIN32] [MACOSX_BUNDLE] SOURCES ... INCLUDES ... LIBRARIES ... [RESOURCE_FILE <file>])`
- `gm3zza_add_standard_windows_defines(TARGET <name>)`

This module should stay thin. It should not know about FFTW, hamlib, or Piper.

### 5. `Gm3zzaResources.cmake`

Purpose:
- standardise runtime asset copy and install logic.

Responsibilities:
- define one consistent runtime asset copy method;
- install runtime data files to `APP_DATA_INSTALL_DIR`;
- install grouped subdirectories such as `contests/`;
- optionally stage runtime assets for development builds;
- optionally generate Windows icon and resource files.

Suggested entry points:
- `gm3zza_stage_runtime_files(TARGET <name> FILES ... [DEST_SUBDIR <dir>])`
- `gm3zza_install_runtime_files(FILES ... [DEST_SUBDIR <dir>] [COMPONENT <name>])`
- `gm3zza_enable_windows_icon(TARGET <name> PNG_FILE <png> RC_TEMPLATE <template> OUTPUT_VAR <var>)`

This would replace the current mixed `file(COPY ...)` and `reference_data` approaches used in:
- [../qbs/CMakeLists.txt](../qbs/CMakeLists.txt#L172-L173)
- [../zzalog/CMakeLists.txt](../zzalog/CMakeLists.txt#L478-L489)
- [../zzatts/CMakeLists.txt](../zzatts/CMakeLists.txt#L199-L204)
- [CMakeLists.txt](CMakeLists.txt#L221-L227)

### 6. `Gm3zzaWindowsRuntime.cmake`

Purpose:
- standardise Windows runtime DLL handling.

Responsibilities:
- implement the common `$<TARGET_RUNTIME_DLLS:...>` post-build copy block;
- install common runtime DLLs where needed;
- provide helper hooks to register extra DLLs from dependency-specific helpers.

Suggested entry points:
- `gm3zza_copy_runtime_dlls(TARGET <name>)`
- `gm3zza_register_runtime_dlls(FILES ... [COMPONENT <name>])`
- `gm3zza_install_registered_runtime_dlls()`

Expected effect:
- replaces the repeated post-build block in [../qbs/CMakeLists.txt](../qbs/CMakeLists.txt#L184-L197), [../zzalog/CMakeLists.txt](../zzalog/CMakeLists.txt#L458-L471), [../zzatts/CMakeLists.txt](../zzatts/CMakeLists.txt#L180-L194), and [CMakeLists.txt](CMakeLists.txt#L204-L217)

### 7. `Gm3zzaDocs.cmake`

Purpose:
- isolate optional documentation behaviour.

Responsibilities:
- add user guide and API subdirectories when enabled;
- install generated HTML and PDF output;
- optionally copy `zzacommon` API documentation into project docs.

Suggested entry points:
- `gm3zza_enable_docs(USERGUIDE_DIR <dir> API_DIR <dir>)`
- `gm3zza_install_docs([PDF_FILE <file>] HTML_DIRS ... )`

This exists mainly for `zzalog`-style projects and should remain optional.

### 8. `deps/*.cmake`

Purpose:
- keep dependency-specific integration separate from the common app flow.

Recommended contents:
- `deps/fftw_helper.cmake`
- `deps/hamlib_helper.cmake`
- `deps/piper_helper.cmake`
- `deps/portaudio_helper.cmake`

These should expose narrow functions only, for example:
- `find_fftw()`
- `install_fftw_dlls()`
- `find_hamlib()`
- `install_hamlib_dlls()`
- `find_piper()`
- `install_piper_dlls()`

These can later be renamed to a common prefix such as:
- `gm3zza_find_fftw()`
- `gm3zza_install_fftw_runtime()`

### 9. `windows/icon_helper.cmake` and `windows/convert_png_to_ico.cmake`

Purpose:
- keep Windows-only icon generation separate from general resource staging.

These should be shared once, not copied into each project.

## Project-level `CMakeLists.txt` template

Each project should eventually shrink to roughly this shape:

```cmake
cmake_minimum_required(VERSION 3.16)
project(MYAPP VERSION 1.2.3)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../zzacmake/cmake")

include(Gm3zzaProjectDefaults)
include(Gm3zzaVersioning)
include(Gm3zzaZzacommon)
include(Gm3zzaTargetSetup)
include(Gm3zzaResources)
include(Gm3zzaWindowsRuntime)

include(deps/fftw_helper OPTIONAL)
include(deps/hamlib_helper OPTIONAL)
include(deps/piper_helper OPTIONAL)

set(TARGET myapp)
set(APP_VENDOR "GM3ZZA")
set(APP_NAME "MYAPP")
set(APP_RC_VALUE "")

set(MYAPP_SOURCES
  src/main.cpp
  src/window.cpp
)

set(MYAPP_DATAFILES
  rose.png
)

gm3zza_project_defaults()
gm3zza_split_version(PROJECT_NAME ${PROJECT_NAME})
gm3zza_generate_app_source(
  INPUT  ${CMAKE_CURRENT_SOURCE_DIR}/src/app.cpp.in
  OUTPUT_VAR GENERATED_APP_CPP
)

gm3zza_use_zzacommon(
  COMPONENTS zzaf zzam zzad
  GIT_TAG v1.1.2.1
)

set(ALL_SOURCES ${GENERATED_APP_CPP} ${MYAPP_SOURCES})

if(MSVC)
  gm3zza_enable_windows_icon(
    TARGET ${TARGET}
    PNG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/rose.png
    RC_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/myapp.rc.in
    OUTPUT_VAR APP_RC_FILE
  )
endif()

gm3zza_add_application(
  TARGET ${TARGET}
  SOURCES ${ALL_SOURCES}
  INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/include
  LIBRARIES zzacommon::zzaf zzacommon::zzam zzacommon::zzad
  RESOURCE_FILE ${APP_RC_FILE}
)

gm3zza_stage_runtime_files(
  TARGET ${TARGET}
  FILES ${MYAPP_DATAFILES}
)

gm3zza_install_runtime_files(
  FILES ${MYAPP_DATAFILES}
  COMPONENT data
)

gm3zza_copy_runtime_dlls(TARGET ${TARGET})
install(TARGETS ${TARGET} RUNTIME COMPONENT applications)
```

## Preset layout draft

The shared presets strategy should be:
- one common `CMakePresets.json` template;
- per-project overrides only for cache variables such as `FLTK_DIR`, `FFTW_ROOT`, `PIPER_ROOT`, and `HAMLIB_ROOT`.

Recommended preset families:
- `linux-debug`
- `linux-release`
- `macos-debug`
- `macos-release`
- `windows-base`
- `x64-debug`
- `x64-release`

This would align with the current direction already present in:
- [../qbs/CMakePresets.json](../qbs/CMakePresets.json)
- [../zzatts/CMakePresets.json](../zzatts/CMakePresets.json)
- [CMakePresets.json](CMakePresets.json)

`CMakeSettings.json` should become legacy-only and eventually be removed.

## Migration map by current project

### `qbs`

Would use:
- `Gm3zzaProjectDefaults.cmake`
- `Gm3zzaVersioning.cmake`
- `Gm3zzaZzacommon.cmake`
- `Gm3zzaTargetSetup.cmake`
- `Gm3zzaResources.cmake`
- `Gm3zzaWindowsRuntime.cmake`
- `windows/icon_helper.cmake`

Project-specific items left in `qbs`:
- source list
- `zzacommon` component list
- `rose.png`
- `qbs.rc.in`

### `zzavnad`

Would use everything `qbs` uses, plus:
- `deps/fftw_helper.cmake`

Project-specific items left in `zzavnad`:
- source list
- FFTW dependency wiring
- `zzacommon` component list including `zzab`

### `zzatts`

Would use the common core, plus:
- `deps/fftw_helper.cmake`
- `deps/piper_helper.cmake`
- optionally `deps/portaudio_helper.cmake`

Project-specific items left in `zzatts`:
- Piper and ONNX include/link details
- DLL registration for Piper-specific binaries
- source list

### `zzalog`

Would use the common core, plus:
- `deps/hamlib_helper.cmake`
- `Gm3zzaDocs.cmake`
- `windows/icon_helper.cmake`

Project-specific items left in `zzalog`:
- docs options
- contest data layout
- hamlib setup
- API/user guide subdirectories
- larger source and header lists

## Immediate extraction order

Recommended order for implementation:

1. extract `Gm3zzaProjectDefaults.cmake`
2. extract `Gm3zzaVersioning.cmake`
3. extract `Gm3zzaWindowsRuntime.cmake`
4. extract `Gm3zzaZzacommon.cmake`
5. extract `Gm3zzaResources.cmake`
6. move shared Windows icon helpers
7. add `Gm3zzaDocs.cmake`
8. align presets

This order reduces risk because it starts with the most mechanical duplication.

## Minimal viable shared layer

If a smaller first pass is preferred, start with only these four files:
- `Gm3zzaProjectDefaults.cmake`
- `Gm3zzaVersioning.cmake`
- `Gm3zzaZzacommon.cmake`
- `Gm3zzaWindowsRuntime.cmake`

That would already remove a large fraction of the repeated boilerplate.

## Recommendation

The first implementation should target `qbs` and `zzavnad` together.

Reason:
- they are structurally close;
- both are simpler than `zzalog`;
- both already resemble the intended common pattern;
- they will validate the shared core before handling `zzalog` docs and `zzatts` Piper integration.

## Next step

The next practical step is to create the shared module skeleton and then refactor one pilot project against it.

The best pilot pair is:
- [../qbs/CMakeLists.txt](../qbs/CMakeLists.txt)
- [CMakeLists.txt](CMakeLists.txt)
