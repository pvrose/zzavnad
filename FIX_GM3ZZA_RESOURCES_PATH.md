# Fix for Gm3zzaResources.cmake Path Issue

## Problem

In `../zzacmake/cmake/Gm3zzaResources.cmake` around line 103, the code was using `find_file()` to locate `icon_helper.cmake`:

```cmake
find_file(ICON_HELPER_PATH "icon_helper.cmake" PATHS "${CMAKE_CURRENT_LIST_DIR}/windows")
```

This approach had potential issues:
1. `find_file()` caches results, which can cause stale failures
2. Less explicit about the expected file location
3. Harder to debug when the path is incorrect

## Solution

Replaced `find_file()` with a direct path construction using `CMAKE_CURRENT_LIST_DIR`:

```cmake
set(ICON_HELPER_PATH "${CMAKE_CURRENT_LIST_DIR}/windows/icon_helper.cmake")
if(NOT EXISTS "${ICON_HELPER_PATH}")
	message(FATAL_ERROR "GM3ZZA: icon_helper.cmake not found at ${ICON_HELPER_PATH}")
endif()
```

## Why This Works

- `CMAKE_CURRENT_LIST_DIR` points to the directory containing the currently executing CMake file (`zzacmake/cmake/`)
- Directly constructing the path as `${CMAKE_CURRENT_LIST_DIR}/windows/icon_helper.cmake` gives us the full path
- Using `EXISTS` check is more reliable than cached `find_file()` results
- Error message now shows the full path that was checked, making debugging easier

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

The `icon_helper.cmake` itself uses `CMAKE_CURRENT_FUNCTION_LIST_DIR` to find `convert_png_to_ico.cmake`, which is the correct approach for function-scoped includes.

## Testing

To test after clearing CMake cache:
```bash
rm -rf out/build/x64-debug/CMakeCache.txt
cmake --preset x64-debug
```

Note: There are separate build issues (x86/x64 library mismatches) that need to be resolved independently.
