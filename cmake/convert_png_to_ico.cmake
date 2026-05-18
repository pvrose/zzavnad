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

        # Function to process transparency
        function Process-Transparency {
            param($sourceBitmap, $threshold)

            $result = [System.Drawing.Bitmap]::new($sourceBitmap.Width, $sourceBitmap.Height, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)

            for ($y = 0; $y -lt $sourceBitmap.Height; $y++) {
                for ($x = 0; $x -lt $sourceBitmap.Width; $x++) {
                    $pixel = $sourceBitmap.GetPixel($x, $y)

                    # Check if pixel is white or near-white based on threshold
                    if ($pixel.R -ge $threshold -and $pixel.G -ge $threshold -and $pixel.B -ge $threshold) {
                        # Make it fully transparent
                        $newPixel = [System.Drawing.Color]::FromArgb(0, 0, 0, 0)
                        $result.SetPixel($x, $y, $newPixel)
                    } else {
                        # Keep the original pixel with its alpha channel
                        $result.SetPixel($x, $y, $pixel)
                    }
                }
            }

            return $result
        }

        # Function to resize bitmap with high quality
        function Resize-Bitmap {
            param($sourceBitmap, $newWidth, $newHeight)

            $destBitmap = [System.Drawing.Bitmap]::new($newWidth, $newHeight, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
            $graphics = [System.Drawing.Graphics]::FromImage($destBitmap)
            $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
            $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality
            $graphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
            $graphics.CompositingQuality = [System.Drawing.Drawing2D.CompositingQuality]::HighQuality
            $graphics.DrawImage($sourceBitmap, 0, 0, $newWidth, $newHeight)
            $graphics.Dispose()

            return $destBitmap
        }

        # Load the original PNG
        $png = [System.Drawing.Bitmap]::new('${PNG_FILE}')

        # Process transparency
        $threshold = ${WHITE_THRESHOLD}
        $transparentBitmap = Process-Transparency -sourceBitmap $png -threshold $threshold

        # Create multiple icon sizes for proper Windows taskbar display
        # Windows needs 16x16, 32x32, 48x48, and 256x256 for best results
        $sizes = @(16, 32, 48, 256)
        $icons = @()

        foreach ($size in $sizes) {
            if ($transparentBitmap.Width -ge $size -and $transparentBitmap.Height -ge $size) {
                $resized = Resize-Bitmap -sourceBitmap $transparentBitmap -newWidth $size -newHeight $size
                $icons += $resized
            }
        }

        # If no icons were created, use the original
        if ($icons.Count -eq 0) {
            $icons = @($transparentBitmap)
        }

        # Save as multi-resolution ICO file
        # Create a memory stream to build the ICO file manually
        $ms = [System.IO.MemoryStream]::new()
        $writer = [System.IO.BinaryWriter]::new($ms)

        # ICO header
        $writer.Write([UInt16]0)      # Reserved (must be 0)
        $writer.Write([UInt16]1)      # Type (1 = ICO)
        $writer.Write([UInt16]$icons.Count)  # Number of images

        # Calculate offset for image data
        $offset = 6 + ($icons.Count * 16)  # Header + directory entries

        # Write directory entries and collect image data
        $imageDataList = [System.Collections.ArrayList]::new()

        foreach ($icon in $icons) {
            # Create DIB (Device Independent Bitmap) data manually for proper alpha channel
            # This is necessary because standard BMP doesn't preserve alpha in ICO format
            $width = $icon.Width
            $height = $icon.Height

            # Lock bitmap data for direct pixel access
            $rect = [System.Drawing.Rectangle]::new(0, 0, $width, $height)
            $bmpData = $icon.LockBits($rect, [System.Drawing.Imaging.ImageLockMode]::ReadOnly, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)

            # Calculate sizes
            $stride = $bmpData.Stride
            $pixelDataSize = $stride * $height

            # Create DIB header (40 bytes = BITMAPINFOHEADER)
            $dibHeader = [byte[]]::new(40)
            $headerMs = [System.IO.MemoryStream]::new($dibHeader)
            $headerWriter = [System.IO.BinaryWriter]::new($headerMs)

            $headerWriter.Write([UInt32]40)              # Header size
            $headerWriter.Write([Int32]$width)           # Width
            $headerWriter.Write([Int32]($height * 2))    # Height * 2 (includes AND mask)
            $headerWriter.Write([UInt16]1)               # Planes
            $headerWriter.Write([UInt16]32)              # Bits per pixel (32 for ARGB)
            $headerWriter.Write([UInt32]0)               # Compression (0 = BI_RGB)
            $headerWriter.Write([UInt32]$pixelDataSize)  # Image size
            $headerWriter.Write([Int32]0)                # X pixels per meter
            $headerWriter.Write([Int32]0)                # Y pixels per meter
            $headerWriter.Write([UInt32]0)               # Colors used
            $headerWriter.Write([UInt32]0)               # Important colors

            $headerWriter.Dispose()
            $headerMs.Dispose()

            # Get pixel data (already in BGRA format which is what DIB expects)
            $pixelData = [byte[]]::new($pixelDataSize)
            [System.Runtime.InteropServices.Marshal]::Copy($bmpData.Scan0, $pixelData, 0, $pixelDataSize)
            $icon.UnlockBits($bmpData)

            # Flip pixel data vertically (BMP is stored bottom-to-top)
            $flippedPixelData = [byte[]]::new($pixelDataSize)
            for ($y = 0; $y -lt $height; $y++) {
                $srcOffset = $y * $stride
                $dstOffset = ($height - 1 - $y) * $stride
                [Array]::Copy($pixelData, $srcOffset, $flippedPixelData, $dstOffset, $stride)
            }

            # Create AND mask (for transparency - all zeros since we use alpha channel)
            $maskStride = [int](([int]$width + 31) / 32) * 4  # Align to 4-byte boundary
            $maskSize = $maskStride * $height
            $andMask = [byte[]]::new($maskSize)  # All zeros = no mask (use alpha channel)

            # Combine header + pixel data + AND mask
            $dibData = $dibHeader + $flippedPixelData + $andMask

            # Directory entry
            # Width and Height: 0 means 256 pixels
            $iconWidth = if ($width -eq 256) { 0 } else { $width }
            $iconHeight = if ($height -eq 256) { 0 } else { $height }

            $writer.Write([byte]$iconWidth)   # Width (0 = 256)
            $writer.Write([byte]$iconHeight)  # Height (0 = 256)
            $writer.Write([byte]0)           # Color palette (0 = no palette)
            $writer.Write([byte]0)           # Reserved
            $writer.Write([UInt16]1)         # Color planes
            $writer.Write([UInt16]32)        # Bits per pixel (32 for ARGB)
            $writer.Write([UInt32]$dibData.Length)  # Image data size
            $writer.Write([UInt32]$offset)   # Offset to image data

            $imageDataList.Add($dibData) | Out-Null
            $offset += $dibData.Length
        }

        # Write image data
        foreach ($dibData in $imageDataList) {
            $ms.Write($dibData, 0, $dibData.Length)
        }

        # Write to file
        $ms.Flush()
        $icoData = $ms.ToArray()
        [System.IO.File]::WriteAllBytes('${ICO_FILE}', $icoData)

        # Cleanup
        $writer.Dispose()
        $ms.Dispose()
        foreach ($icon in $icons) {
            $icon.Dispose()
        }
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
