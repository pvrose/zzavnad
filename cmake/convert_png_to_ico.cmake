# convert_png_to_ico.cmake
# CMake script to convert PNG to ICO format on Windows with transparency support
# Usage: cmake -DPNG_FILE=input.png -DICO_FILE=output.ico [-DWHITE_THRESHOLD=250] -P convert_png_to_ico.cmake
#
# Features:
# - Converts white or near-white backgrounds to fully transparent
# - WHITE_THRESHOLD: RGB threshold for white detection (default: 250, range: 0-255)
# - Preserves original alpha channel for non-white pixels
# - Creates Windows icons with proper transparency

if(NOT PNG_FILE OR NOT ICO_FILE)
    message(FATAL_ERROR "PNG_FILE and ICO_FILE must be specified")
endif()

if(NOT EXISTS "${PNG_FILE}")
    message(FATAL_ERROR "PNG file not found: ${PNG_FILE}")
endif()

# Set default white threshold if not specified
if(NOT DEFINED WHITE_THRESHOLD)
    set(WHITE_THRESHOLD 250)
endif()

message(STATUS "Converting ${PNG_FILE} to ${ICO_FILE}")
message(STATUS "White threshold: ${WHITE_THRESHOLD} (pixels with RGB >= ${WHITE_THRESHOLD} will be transparent)")

# Use PowerShell to convert PNG to ICO with transparency
execute_process(
    COMMAND powershell -NoProfile -ExecutionPolicy Bypass -Command "
        Add-Type -AssemblyName System.Drawing

        # Load the original PNG
        $png = [System.Drawing.Bitmap]::new('${PNG_FILE}')

        # Create a new bitmap with the same dimensions
        $transparentBitmap = [System.Drawing.Bitmap]::new($png.Width, $png.Height)

        # White threshold for transparency
        $threshold = ${WHITE_THRESHOLD}

        # Process each pixel to make white background transparent
        for ($y = 0; $y -lt $png.Height; $y++) {
            for ($x = 0; $x -lt $png.Width; $x++) {
                $pixel = $png.GetPixel($x, $y)

                # Check if pixel is white or near-white based on threshold
                if ($pixel.R -ge $threshold -and $pixel.G -ge $threshold -and $pixel.B -ge $threshold) {
                    # Make it fully transparent
                    $transparentPixel = [System.Drawing.Color]::FromArgb(0, 255, 255, 255)
                    $transparentBitmap.SetPixel($x, $y, $transparentPixel)
                } else {
                    # Keep the original pixel with its alpha channel
                    $transparentBitmap.SetPixel($x, $y, $pixel)
                }
            }
        }

        # Convert to icon
        $icon = [System.Drawing.Icon]::FromHandle($transparentBitmap.GetHicon())
        $stream = [System.IO.FileStream]::new('${ICO_FILE}', [System.IO.FileMode]::Create)
        $icon.Save($stream)
        $stream.Close()

        # Cleanup
        $transparentBitmap.Dispose()
        $png.Dispose()
    "
    RESULT_VARIABLE RESULT
    ERROR_VARIABLE ERROR_OUTPUT
)

if(NOT RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to convert PNG to ICO: ${ERROR_OUTPUT}")
endif()

message(STATUS "Successfully created ${ICO_FILE}")
