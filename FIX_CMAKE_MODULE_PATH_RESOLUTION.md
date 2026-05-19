# Fix for Gm3zzaResources.cmake and icon_helper.cmake Path Issues

## Problem

When CMake modules define functions that reference other helper files, using `CMAKE_CURRENT_LIST_DIR` inside the function body resolves to the **calling** file's directory, not the module's directory.

### Original Issue 1: Gm3zzaResources.cmake
```cmake
function(gm3zza_enable_windows_icon)
	# This resolves to zzavnad/ not zzacmake/cmake/
	find_file(ICON_HELPER_PATH "icon_helper.cmake" 
			  PATHS "${CMAKE_CURRENT_LIST_DIR}/windows")
endfunction()
```

### Original Issue 2: icon_helper.cmake  
```cmake
function(generate_windows_icon)
	# This resolves to zzavnad/ not zzacmake/cmake/windows/
	set(CONVERTER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/convert_png_to_ico.cmake")
endfunction()
```

## Solution

Capture the module's directory path **at include time** (outside functions) into a module-scoped variable, then use that variable inside functions.

### Fix 1: Gm3zzaResources.cmake

**Added at top of file (line 11-12):**
```cmake
# Capture the directory where this module resides at include time
set(_GM3ZZA_RESOURCES_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}")
```

**Updated function (line 107):**
```cmake
function(gm3zza_enable_windows_icon)
	# ...
	# Use the captured module directory
	set(ICON_HELPER_PATH "${_GM3ZZA_RESOURCES_MODULE_DIR}/windows/icon_helper.cmake")
	if(NOT EXISTS "${ICON_HELPER_PATH}")
		message(FATAL_ERROR "GM3ZZA: icon_helper.cmake not found at ${ICON_HELPER_PATH}")
	endif()

	include("${ICON_HELPER_PATH}")
	# ...
endfunction()
```

### Fix 2: icon_helper.cmake

**Added after comments (line 22-23):**
```cmake
# Capture the directory where this helper resides at include time
set(_ICON_HELPER_DIR "${CMAKE_CURRENT_LIST_DIR}")
```

**Updated function (line 52):**
```cmake
function(generate_windows_icon)
	# ...
	# Use the captured helper directory
	set(CONVERTER_SCRIPT "${_ICON_HELPER_DIR}/convert_png_to_ico.cmake")
	if(NOT EXISTS "${CONVERTER_SCRIPT}")
		message(WARNING "Icon generation: Converter script not found: ${CONVERTER_SCRIPT}")
		return()
	endif()
	# ...
endfunction()
```

## Why This Works

### CMake Variable Scope Behaviour:
1. **`CMAKE_CURRENT_LIST_DIR` at module scope** (outside functions): Points to the directory containing the currently processing file
2. **`CMAKE_CURRENT_LIST_DIR` in functions**: Re-evaluated when the function is called, points to the **calling** file's directory
3. **Module-scoped variables** (set outside functions): Retain their value when accessed from within functions

### Our Pattern:
```cmake
# At include time: CMAKE_CURRENT_LIST_DIR = zzacmake/cmake/
set(_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}")  # Captures: zzacmake/cmake/

function(my_function)
	# At call time from zzavnad/CMakeLists.txt:
	# CMAKE_CURRENT_LIST_DIR would be: zzavnad/
	# _MODULE_DIR still holds: zzacmake/cmake/ ✓
	set(HELPER "${_MODULE_DIR}/helper.cmake")
endfunction()
```

## Testing

After fixes, icon generation warnings resolved:
```bash
$ cmake --preset x64-debug
-- GM3ZZA: CXX standard: 17
-- Icon generation: Converting rose.png to rose.ico
-- Icon generation: White threshold = 240
-- Icon generation: Successfully created rose.ico
-- Resource file: Generated rose.rc
```

## File Structure (Confirmed Working)

```
zzacmake/
  cmake/
	Gm3zzaResources.cmake          ← Fixed: captures module dir
	windows/
	  icon_helper.cmake             ← Fixed: captures helper dir
	  convert_png_to_ico.cmake      ← Referenced correctly

zzavnad/
  CMakeLists.txt                    ← Calls gm3zza_enable_windows_icon()
  rose.png
```

## Related CMake Variables

- `CMAKE_CURRENT_LIST_DIR` - Directory of the current file being processed
- `CMAKE_CURRENT_SOURCE_DIR` - Directory where CMakeLists.txt being processed is located
- `CMAKE_CURRENT_FUNCTION_LIST_DIR` - (CMake 3.17+) Directory where the function was defined (would have been ideal, but has issues in some contexts)

## Recommendation

This pattern should be used in all shared CMake modules that:
1. Define functions
2. Need to reference sibling helper files
3. Are included from other projects

Standard template:
```cmake
# Capture module directory at include time
set(_MY_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(my_function)
	# Use _MY_MODULE_DIR instead of CMAKE_CURRENT_LIST_DIR
	include("${_MY_MODULE_DIR}/helper.cmake")
endfunction()
```
