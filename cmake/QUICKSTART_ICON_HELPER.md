# Quick Start: Reusing Icon Helper in Your Project

## 1. Copy Files

Copy these two files to your project's `cmake/` directory:

```
YourProject/
├── cmake/
│   ├── icon_helper.cmake          ← Copy this
│   └── convert_png_to_ico.cmake   ← Copy this
├── your_logo.png                   ← Your PNG logo
├── yourapp.rc.in                   ← Your RC template
└── CMakeLists.txt
```

## 2. Create Resource Template (yourapp.rc.in)

Create `yourapp.rc.in` with this content:

```rc
IDI_ICON1 ICON "@PROJECT_NAME@.ico"

1 VERSIONINFO
FILEVERSION @PROJECT_VERSION_MAJOR@,@PROJECT_VERSION_MINOR@,@PROJECT_VERSION_PATCH@,@PROJECT_VERSION_TWEAK@
PRODUCTVERSION @PROJECT_VERSION_MAJOR@,@PROJECT_VERSION_MINOR@,@PROJECT_VERSION_PATCH@,@PROJECT_VERSION_TWEAK@
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904B0"
        BEGIN
            VALUE "CompanyName", "@APP_VENDOR@"
            VALUE "FileDescription", "@APP_NAME@"
            VALUE "FileVersion", "@APP_VERSION@"
            VALUE "ProductName", "@APP_NAME@"
            VALUE "ProductVersion", "@APP_VERSION@"
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x409, 1200
    END
END
```

## 3. Update Your CMakeLists.txt

Add this section to your `CMakeLists.txt`:

```cmake
# Windows icon generation
if(MSVC)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/icon_helper.cmake)

    generate_windows_icon_and_resource(
        PNG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/your_logo.png"
        RC_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/yourapp.rc.in"
        WHITE_THRESHOLD 250  # Optional
    )

    add_executable(yourapp WIN32 ${PROJECT_NAME}_RC_FILE)
endif()
```

## 4. Build

The icon will be automatically generated during CMake configuration:

```bash
cmake -S . -B build
cmake --build build
```

## Complete Example

```cmake
cmake_minimum_required(VERSION 3.16)
project(MyApp VERSION 1.0.0)

# Set app details
set(APP_VENDOR "Your Company")
set(APP_NAME "MyApp")
set(APP_VERSION "${PROJECT_VERSION}")

# Windows icon
if(MSVC)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/icon_helper.cmake)

    generate_windows_icon_and_resource(
        PNG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/logo.png"
        RC_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/myapp.rc.in"
        WHITE_THRESHOLD 240
    )

    add_executable(myapp WIN32 main.cpp ${MyApp_RC_FILE})
else()
    add_executable(myapp main.cpp)
endif()
```

## Done!

Your executable will now have:
- ✅ Custom icon with transparent background
- ✅ Version information embedded in the executable
- ✅ Proper Windows application properties

For more details, see `README_ICON_HELPER.md`.
