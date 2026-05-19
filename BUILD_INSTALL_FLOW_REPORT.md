# CMake build and install flow review

Date: 19 May 2026

Projects reviewed:
- [../qbs/CMakeLists.txt](../qbs/CMakeLists.txt)
- [../zzalog/CMakeLists.txt](../zzalog/CMakeLists.txt)
- [../zzatts/CMakeLists.txt](../zzatts/CMakeLists.txt)
- [CMakeLists.txt](CMakeLists.txt)

## Executive summary

Yes, you can standardise most of the build and install flow across these four projects, but not as a single shared top-level `CMakeLists.txt` dropped unchanged into every repository.

The workable approach is:
- keep a small project-level `CMakeLists.txt` in each repository;
- move the repeated logic into a shared CMake module set;
- keep only project metadata, source lists, dependency lists, and project-specific extras in each repository.

My judgement is:
- **about 70–85% of the current flow is reusable**;
- **the remaining 15–30% should stay project-specific**.

## What is already common

### 1. Common preset structure

`qbs`, `zzatts`, and `zzavnad` already use the same basic preset layout:
- Linux and macOS debug presets with Ninja and `out/build` + `out/install` layout in [../qbs/CMakePresets.json](../qbs/CMakePresets.json#L5-L40), [../zzatts/CMakePresets.json](../zzatts/CMakePresets.json#L5-L40), and [CMakePresets.json](CMakePresets.json#L5-L40)
- Windows `windows-base`, `x64-debug`, and `x64-release` presets in [../qbs/CMakePresets.json](../qbs/CMakePresets.json#L35-L72), [../zzatts/CMakePresets.json](../zzatts/CMakePresets.json#L35-L77), and [CMakePresets.json](CMakePresets.json#L35-L73)

This is already close to a reusable family pattern.

### 2. Common `zzacommon` acquisition flow

All four projects:
- extend `CMAKE_PREFIX_PATH` to look for a sibling `zzacommon` build or install;
- fall back to `FetchContent`;
- link named `zzacommon::...` targets.

References:
- [../qbs/CMakeLists.txt](../qbs/CMakeLists.txt#L114-L136)
- [../zzalog/CMakeLists.txt](../zzalog/CMakeLists.txt#L329-L356)
- [../zzatts/CMakeLists.txt](../zzatts/CMakeLists.txt#L68-L94)
- [CMakeLists.txt](CMakeLists.txt#L86-L111)

This is a strong candidate for a single helper function.

### 3. Common executable assembly pattern

All four projects follow the same broad pattern:
- set project metadata and version values;
- define a target name;
- add an executable;
- add include directories;
- add source files;
- link third-party and `zzacommon` libraries;
- install the executable and data files.

References:
- [../qbs/CMakeLists.txt](../qbs/CMakeLists.txt#L145-L206)
- [../zzalog/CMakeLists.txt](../zzalog/CMakeLists.txt#L403-L536)
- [../zzatts/CMakeLists.txt](../zzatts/CMakeLists.txt#L126-L214)
- [CMakeLists.txt](CMakeLists.txt#L152-L237)

### 4. Common Windows runtime DLL copy pattern

All four projects use the same post-build runtime DLL copy based on `$<TARGET_RUNTIME_DLLS:...>`:
- [../qbs/CMakeLists.txt](../qbs/CMakeLists.txt#L184-L197)
- [../zzalog/CMakeLists.txt](../zzalog/CMakeLists.txt#L458-L471)
- [../zzatts/CMakeLists.txt](../zzatts/CMakeLists.txt#L180-L194)
- [CMakeLists.txt](CMakeLists.txt#L204-L217)

This should be one shared helper.

### 5. Shared helper modules already exist

There are already identical or near-identical helper files:
- `icon_helper.cmake` is identical in `zzalog` and `zzavnad`
- `convert_png_to_ico.cmake` is identical in `zzalog` and `zzavnad`
- `fftw_helper.cmake` is identical in `zzatts` and `zzavnad`

So a shared helper repository or sibling `cmake` module directory would reduce immediate duplication.

## Main differences that block a single drop-in file

### 1. Install destination is not standardised

`qbs`, `zzalog`, and `zzavnad` use `etc/${TARGET}` through `CMAKE_INSTALL_RPATH`:
- [../qbs/CMakeLists.txt](../qbs/CMakeLists.txt#L66-L71)
- [../zzalog/CMakeLists.txt](../zzalog/CMakeLists.txt#L297-L302)
- [CMakeLists.txt](CMakeLists.txt#L64-L67)

`zzatts` uses absolute OS-specific application data paths instead:
- [../zzatts/CMakeLists.txt](../zzatts/CMakeLists.txt#L36-L39)

This is the biggest behavioural mismatch.

Also, `CMAKE_INSTALL_RPATH` is being used as a data install directory variable, which is confusing because the variable normally refers to runtime search paths for binaries, not data destinations.

**Recommendation:** replace this in all projects with a dedicated variable such as `APP_DATA_INSTALL_DIR`.

### 2. Generated `app.cpp` handling is inconsistent

`qbs`, `zzatts`, and `zzavnad` generate `src/app.cpp` directly in the source tree:
- [../qbs/CMakeLists.txt](../qbs/CMakeLists.txt#L138-L142)
- [../zzatts/CMakeLists.txt](../zzatts/CMakeLists.txt#L102-L106)
- [CMakeLists.txt](CMakeLists.txt#L113-L117)

`zzalog` writes the generated file to the build tree instead:
- [../zzalog/CMakeLists.txt](../zzalog/CMakeLists.txt#L358-L362)

The `zzalog` approach is the better reusable pattern because it avoids modifying tracked source files during configuration.

### 3. `zzacommon` version policy is inconsistent

`qbs`, `zzatts`, and `zzavnad` fetch `zzacommon` from `master`:
- [../qbs/CMakeLists.txt](../qbs/CMakeLists.txt#L130-L135)
- [../zzatts/CMakeLists.txt](../zzatts/CMakeLists.txt#L85-L90)
- [CMakeLists.txt](CMakeLists.txt#L105-L110)

`zzalog` fetches a pinned tag:
- [../zzalog/CMakeLists.txt](../zzalog/CMakeLists.txt#L350-L355)

The required component sets also differ per project.

This is fine at the project level, but it means the shared layer needs parameters for:
- required components;
- preferred `zzacommon` tag or revision;
- whether docs from `zzacommon` should be copied.

### 4. Reference data copy flow is inconsistent

Three different approaches are in use:
- `qbs` copies data with `file(COPY ...)` during configure time: [../qbs/CMakeLists.txt](../qbs/CMakeLists.txt#L172-L173)
- `zzalog` copies data and contest files with `file(COPY ...)` during configure time, despite also declaring `reference_data`: [../zzalog/CMakeLists.txt](../zzalog/CMakeLists.txt#L478-L489)
- `zzatts` and `zzavnad` use an `ALL` custom target with post-build copy commands: [../zzatts/CMakeLists.txt](../zzatts/CMakeLists.txt#L199-L204) and [CMakeLists.txt](CMakeLists.txt#L221-L227)

This should be unified.

**Recommendation:** standardise on one helper that either:
- copies runtime assets with a post-build command; and
- installs the same asset list with `install(FILES ...)`.

### 5. Windows icon/resource handling is not fully aligned

`qbs`, `zzalog`, and `zzavnad` all generate Windows icons and resource files:
- [../qbs/CMakeLists.txt](../qbs/CMakeLists.txt#L80-L112)
- [../zzalog/CMakeLists.txt](../zzalog/CMakeLists.txt#L378-L401)
- [CMakeLists.txt](CMakeLists.txt#L125-L150)

`zzatts` currently does not use the same icon helper flow.

This does not prevent reuse, but it means the shared module should make icon/resource generation optional.

### 6. Presets and settings are not fully standardised

`qbs`, `zzatts`, and `zzavnad` use CMake presets, but `zzalog` does not currently have them.

Also, `qbs` and `zzavnad` still carry `CMakeSettings.json`, and they are not aligned:
- `qbs` uses Visual Studio generator settings in [../qbs/CMakeSettings.json](../qbs/CMakeSettings.json#L3-L31)
- `zzavnad` uses Ninja settings in [CMakeSettings.json](CMakeSettings.json#L3-L11)

If the goal is one repeatable flow, the cleanest path is to standardise on `CMakePresets.json` and retire `CMakeSettings.json` where possible.

### 7. Project-specific extras vary a lot

These must remain optional modules, not forced common behaviour:
- `zzalog` documentation build, PDF output, PDB option, API and user guide install flow: [../zzalog/CMakeLists.txt](../zzalog/CMakeLists.txt#L53-L59), [../zzalog/CMakeLists.txt](../zzalog/CMakeLists.txt#L493-L559)
- `zzalog` hamlib integration: [../zzalog/CMakeLists.txt](../zzalog/CMakeLists.txt#L313-L318) and [../zzalog/cmake/hamlib_helper.cmake](../zzalog/cmake/hamlib_helper.cmake)
- `zzatts` piper and ONNX runtime integration: [../zzatts/CMakeLists.txt](../zzatts/CMakeLists.txt#L48-L66), [../zzatts/CMakeLists.txt](../zzatts/CMakeLists.txt#L156-L166), and [../zzatts/cmake/piper_helper.cmake](../zzatts/cmake/piper_helper.cmake)
- `zzatts` and `zzavnad` FFTW integration: [../zzatts/CMakeLists.txt](../zzatts/CMakeLists.txt#L117-L121), [CMakeLists.txt](CMakeLists.txt#L119-L123), and [cmake/fftw_helper.cmake](cmake/fftw_helper.cmake)

## Recommended shared structure

I would not try to share one whole `CMakeLists.txt`.

I would create a shared CMake module package, for example in a new sibling repository such as `zzacmake`, or as a shared subdirectory used by all projects.

Suggested modules:

1. `Gm3zzaProjectDefaults.cmake`
   - C++ standard
   - common compiler flags
   - common options such as PDB/docs switches
   - default install directory variables

2. `Gm3zzaZzacommon.cmake`
   - find local sibling install/build of `zzacommon`
   - fall back to `FetchContent`
   - accept component list and tag as arguments

3. `Gm3zzaResources.cmake`
   - runtime asset copy helper
   - install helper for data files
   - optional icon/resource generation

4. `Gm3zzaWindowsRuntime.cmake`
   - shared `$<TARGET_RUNTIME_DLLS:...>` post-build copy helper
   - wrapper hooks for project-specific DLL installers

5. `Gm3zzaDocs.cmake`
   - optional docs and API install flow for `zzalog`-style projects only

6. dependency-specific helpers kept separate
   - `fftw_helper.cmake`
   - `hamlib_helper.cmake`
   - `piper_helper.cmake`

## Proposed standard per-project shape

Each project-level `CMakeLists.txt` should ideally only contain:
- project name and version;
- target name;
- source and header lists;
- data file list;
- dependency declarations;
- optional feature toggles;
- calls into shared helper functions.

That would make each repository-specific `CMakeLists.txt` much smaller and much more consistent.

## Concrete standardisation steps

### Phase 1: safe alignment

1. Standardise data install variable naming.
   - Replace `CMAKE_INSTALL_RPATH` as a data path variable.
   - Introduce `APP_DATA_INSTALL_DIR` in all four projects.

2. Standardise generated file placement.
   - Move `configure_file(... app.cpp.in ...)` outputs into the binary tree everywhere.

3. Standardise runtime data copy behaviour.
   - Pick either configure-time copy or post-build copy and use it everywhere.
   - I recommend post-build copy for runtime assets.

4. Standardise runtime DLL copy helper.
   - Replace the duplicated `$<TARGET_RUNTIME_DLLS>` blocks with one helper.

5. Standardise on `CMakePresets.json`.
   - Add presets to `zzalog`.
   - Keep a common preset shape and only vary project-specific cache variables.

### Phase 2: extract shared modules

6. Move `icon_helper.cmake`, `convert_png_to_ico.cmake`, and `fftw_helper.cmake` to a shared location.

7. Extract the common `zzacommon` discovery/fetch logic into one helper.

8. Add a small common helper for executable setup and asset installation.

### Phase 3: optional higher-level template

9. After the shared helpers settle, create a thin project template for future repositories.
   - This is the point where new projects can reuse the same flow with minimal edits.

## Recommendation on feasibility

**Can you have a single reusable set of CMake files?**

Yes, with this interpretation:
- a **shared module set** reused by all projects: **yes**;
- a **single identical top-level `CMakeLists.txt`** for all current projects: **no**;
- a **common project skeleton plus per-project metadata**: **yes, and this is the best fit**.

## Bottom line

A common reusable CMake layer is practical now.

The main items preventing immediate unification are:
- inconsistent install destination semantics;
- inconsistent generated-file location;
- inconsistent runtime asset copy method;
- inconsistent preset/settings story;
- project-specific optional features.

If you want, the next sensible step is for me to draft the shared module layout and show what each of the four `CMakeLists.txt` files would shrink to under that scheme.
