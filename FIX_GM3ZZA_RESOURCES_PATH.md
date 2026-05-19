# Fix for Gm3zzaResources.cmake Path and Staging Issues

## Problems

Two issues were observed in `../zzacmake/cmake/Gm3zzaResources.cmake`.

### 1) Module helper resolution

The code used `find_file()` with `CMAKE_CURRENT_LIST_DIR` inside a function to locate `icon_helper.cmake`:

```cmake
find_file(ICON_HELPER_PATH "icon_helper.cmake" PATHS "${CMAKE_CURRENT_LIST_DIR}/windows")
```

Inside function scope, `CMAKE_CURRENT_LIST_DIR` can resolve in caller context rather than the module context. This can mis-resolve shared helper locations.

### 2) Runtime staging path duplication

`gm3zza_stage_runtime_files` always prefixed `CMAKE_CURRENT_SOURCE_DIR` when copying files, even when callers already passed absolute paths:

```cmake
"${CMAKE_CURRENT_SOURCE_DIR}/${F}"
```

That caused duplicated paths such as:

```text
/home/philip/dev/gm3zza/zzavnad//home/philip/dev/gm3zza/zzavnad/rose.png
```

## Solution

### 1) Capture module directory at include time

At module scope:

```cmake
set(_GM3ZZA_RESOURCES_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}")
```

Then inside `gm3zza_enable_windows_icon`, resolve helper path explicitly:

```cmake
set(ICON_HELPER_PATH "${_GM3ZZA_RESOURCES_MODULE_DIR}/windows/icon_helper.cmake")
if(NOT EXISTS "${ICON_HELPER_PATH}")
	message(FATAL_ERROR "GM3ZZA: icon_helper.cmake not found at ${ICON_HELPER_PATH}")
endif()
```

### 2) Handle absolute and relative staging inputs

In `gm3zza_stage_runtime_files`, normalise each source path:

```cmake
if(IS_ABSOLUTE "${F}")
	set(SOURCE_FILE "${F}")
else()
	set(SOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${F}")
endif()
```

Then copy from `SOURCE_FILE`.

## Why This Works

- Capturing `_GM3ZZA_RESOURCES_MODULE_DIR` at include time preserves the module location regardless of caller context.
- Using `EXISTS` on a direct expected path avoids `find_file()` cache side effects.
- `IS_ABSOLUTE` handling prevents duplicate path prefixes during runtime asset staging.
- Error and staging messages now show resolved full paths, improving diagnostics.

## File Structure

The fix assumes this structure (which is correct):

```
zzacmake/
  cmake/
	Gm3zzaResources.cmake          ← This file was fixed
	Gm3zzaProjectDefaults.cmake
	Gm3zzaVersioning.cmake
	Gm3zzaWindowsRuntime.cmake
	Gm3zzaZzacommon.cmake
	windows/
	  icon_helper.cmake             ← Referenced by the fixed code
	  convert_png_to_ico.cmake      ← Used by icon_helper.cmake
	deps/
	  fftw_helper.cmake
```

## Related Files

`../zzacmake/cmake/windows/icon_helper.cmake` also captures its own directory at include time (`_ICON_HELPER_DIR`) and resolves `convert_png_to_ico.cmake` from there.

## Testing

To test after clearing CMake cache:
```bash
rm -rf out/build/x64-debug/CMakeCache.txt
cmake --preset x64-debug
```

Note: There are separate build issues (x86/x64 library mismatches) that need to be resolved independently.

## Outcome

After these fixes, both QBS and ZZAVNAD were verified to build, run, install, and run from install on Windows in your test environment.
