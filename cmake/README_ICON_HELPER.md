# Windows Icon Generation Helper

CMake helper functions for generating Windows icons (.ico) from PNG files with transparency support.

## Files

- **`icon_helper.cmake`** - Main CMake helper functions
- **`convert_png_to_ico.cmake`** - PowerShell-based PNG to ICO converter

## Features

- ✅ Converts PNG files to ICO format
- ✅ Makes white backgrounds transparent
- ✅ Configurable white threshold (0-255)
- ✅ Generates Windows resource files (.rc)
- ✅ Reusable across multiple projects

## Usage

### Basic Usage (All-in-One)

```cmake
# Include the helper
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/icon_helper.cmake)

# Generate both icon and resource file
generate_windows_icon_and_resource(
    PNG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/logo.png"
    RC_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/app.rc.in"
    WHITE_THRESHOLD 240  # Optional, default: 250
)

# The function exports these variables:
# ${PROJECT_NAME}_ICO_FILE - Path to generated .ico file
# ${PROJECT_NAME}_RC_FILE  - Path to generated .rc file
```

### Advanced Usage (Separate Steps)

```cmake
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/icon_helper.cmake)

# Step 1: Generate icon only
generate_windows_icon(
    PNG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/logo.png"
    OUTPUT_ICO "${CMAKE_CURRENT_BINARY_DIR}/myapp.ico"
    WHITE_THRESHOLD 250
)

# Step 2: Configure resource file
configure_windows_resource(
    RC_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/myapp.rc.in"
    OUTPUT_RC "${CMAKE_CURRENT_BINARY_DIR}/myapp.rc"
    ICO_FILE "${CMAKE_CURRENT_BINARY_DIR}/myapp.ico"
)

# Use the generated resource file in your executable
add_executable(myapp WIN32 ${MYAPP_RC_FILE})
```

## Parameters

### `generate_windows_icon()`

| Parameter | Required | Description |
|-----------|----------|-------------|
| `PNG_FILE` | Yes | Path to source PNG file |
| `OUTPUT_ICO` | Yes | Path for output ICO file |
| `WHITE_THRESHOLD` | No | RGB threshold for transparency (default: 250) |

### `configure_windows_resource()`

| Parameter | Required | Description |
|-----------|----------|-------------|
| `RC_TEMPLATE` | Yes | Path to .rc.in template file |
| `OUTPUT_RC` | Yes | Path for output .rc file |
| `ICO_FILE` | No | Path to icon file (for reference) |

### `generate_windows_icon_and_resource()`

| Parameter | Required | Description |
|-----------|----------|-------------|
| `PNG_FILE` | Yes | Path to source PNG file |
| `RC_TEMPLATE` | Yes | Path to .rc.in template file |
| `WHITE_THRESHOLD` | No | RGB threshold for transparency (default: 250) |

## White Threshold Guide

The `WHITE_THRESHOLD` parameter controls which pixels become transparent:

- **240** - More aggressive (removes near-white and off-white pixels)
- **250** - Recommended default (removes white and very light gray)
- **254** - Conservative (only removes nearly pure white)
- **255** - Only pure white (RGB 255,255,255)

**Rule**: Pixels with RGB values **all >= threshold** become transparent.

## Example: ZZALOG Project

```cmake
# In CMakeLists.txt
if(MSVC)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/icon_helper.cmake)

  set(ZZALOG_ICON_WHITE_THRESHOLD 240)

  generate_windows_icon_and_resource(
    PNG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/rose.png"
    RC_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/zzalog.rc.in"
    WHITE_THRESHOLD ${ZZALOG_ICON_WHITE_THRESHOLD}
  )

  add_executable(zzalog WIN32 ${ZZALOG_RC_FILE})
endif()
```

## Reusing in Other Projects

To use in another project:

1. **Copy these files** to your project's `cmake/` directory:
   - `icon_helper.cmake`
   - `convert_png_to_ico.cmake`

2. **Include the helper** in your CMakeLists.txt:
   ```cmake
   include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/icon_helper.cmake)
   ```

3. **Call the function**:
   ```cmake
   generate_windows_icon_and_resource(
       PNG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/my_logo.png"
       RC_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/myapp.rc.in"
   )
   ```

## Requirements

- **Platform**: Windows (MSVC only)
- **PowerShell**: Built-in on Windows
- **CMake**: Version 3.16 or higher
- **.NET Framework**: System.Drawing assembly (standard on Windows)

## Troubleshooting

**Icon not transparent:**
- Lower the `WHITE_THRESHOLD` value (try 240)
- Check your PNG has actual white pixels (RGB 255,255,255)

**Build error "FLTK_DIR not set":**
- This is unrelated to icon generation
- Set FLTK_DIR if your project uses FLTK

**Script not found:**
- Ensure both `icon_helper.cmake` and `convert_png_to_ico.cmake` are in the `cmake/` directory

## License

Copyright 2026, Philip Rose, GM3ZZA

This software is part of ZZALOG and is distributed under the Lesser GNU General Public License v3 or later.
