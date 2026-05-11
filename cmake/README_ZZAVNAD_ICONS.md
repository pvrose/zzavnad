# Icon Generation for ZZAVNAD

This document describes how Windows icons are automatically generated for ZZAVNAD.

## Overview

ZZAVNAD uses a CMake-based icon generation system that:
1. Converts the PNG logo (`rose.png`) to Windows ICO format
2. Makes white/near-white backgrounds transparent
3. Embeds the icon and version information into the executable

## Files

- `cmake/icon_helper.cmake` - CMake functions for icon generation
- `cmake/convert_png_to_ico.cmake` - PowerShell script to convert PNG to ICO
- `rose.png` - Source logo image
- `zzavnad.rc.in` - Windows resource template file

## How It Works

During CMake configuration (when you run `cmake -S . -B build`):

1. **Icon Generation**: The `generate_windows_icon()` function:
   - Reads `rose.png`
   - Converts white/near-white pixels to transparent
   - Saves as `rose.ico` in the build directory

2. **Resource File Configuration**: The `configure_windows_resource()` function:
   - Reads `zzavnad.rc.in` template
   - Substitutes CMake variables (version, name, etc.)
   - Generates `rose.rc` in the build directory

3. **Build Integration**: The generated `.rc` file is added to the executable target

## Customization

### Adjusting Transparency Threshold

The `WHITE_THRESHOLD` parameter controls which pixels become transparent.
Edit `CMakeLists.txt` line 123:

```cmake
set(ZZAVNAD_ICON_WHITE_THRESHOLD 240)  # Default: 240
```

**Values:**
- `240` - Removes white and near-white (recommended for rose.png)
- `250` - More conservative (only very white pixels)
- `254` - Minimal (almost pure white only)
- `255` - Only pure white RGB(255,255,255)

### Using a Different Logo

To use a different PNG file:

1. Replace `rose.png` with your logo
2. Update `CMakeLists.txt` line 128:
   ```cmake
   PNG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/your_logo.png"
   ```
3. Update `zzavnad.rc.in` line 10 to reference the new icon:
   ```rc
   IDI_ICON1 ICON "your_logo.ico"
   ```

## Version Information

The resource file includes version information that appears in Windows file properties:

- **File Version**: From `project(ZZAVNAD VERSION x.y.z)` in CMakeLists.txt
- **Company**: `APP_VENDOR` variable (currently "GM3ZZA")
- **Product Name**: `APP_NAME` variable (currently "ZZAVNAD")
- **Description**: Hardcoded in `zzavnad.rc.in`

To update, edit the variables in `CMakeLists.txt` or modify `zzavnad.rc.in`.

## Troubleshooting

### Icon Not Transparent
- Lower the `WHITE_THRESHOLD` value
- Check that your PNG has actual white pixels (RGB 255,255,255 or near)
- Verify your source PNG already has transparency where needed

### Build Error About Icon File
- Ensure `rose.png` exists in the project root
- Check that PowerShell is available (standard on Windows)
- Verify CMake version >= 3.16

### Icon Not Showing in Executable
- Ensure you're building with MSVC (icon generation only works on Windows)
- Check that `WIN32` flag is present in `add_executable()`
- Rebuild completely: delete build directory and reconfigure

## For Developers

The icon generation uses the shared helper functions from `icon_helper.cmake`:

- `generate_windows_icon()` - Convert PNG to ICO
- `configure_windows_resource()` - Process .rc.in template  
- `generate_windows_icon_and_resource()` - Convenience function for both

See `cmake/README_ICON_HELPER.md` for detailed documentation of these functions.

## License

Copyright 2026, Philip Rose, GM3ZZA

This file is part of ZZAVNAD. VNA Analysis Software.

ZZAVNAD is distributed under the Lesser GNU General Public License v3 or later.
