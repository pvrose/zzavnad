# Rose Color Conversion - Red to Peach

## Summary

The rose.png logo has been converted from red/pink colors to peach tones to give it a warmer, softer appearance.

## Files

- **`rose.png`** - Updated logo with peach-colored rose (13,785 bytes)
- **`rose_original_backup.png`** - Original red/pink rose backup (18,989 bytes)
- **`convert_rose_to_peach.ps1`** - PowerShell script used for conversion

## What Was Changed

The conversion script:
1. Identifies red/pink/magenta pixels (where red component > green and blue)
2. Converts them to peach tones (RGB: 255, 204, 170)
3. Preserves brightness levels (darker reds → darker peach, lighter pinks → lighter peach)
4. Maintains full transparency for background
5. Keeps non-red colors unchanged (stems, shadows, etc.)

**Result**: 5,761 red/pink pixels were converted to peach

## Peach Color Used

**Target Peach**: RGB(255, 204, 170)
- R: 255 (full red)
- G: 204 (80% green - adds warmth)
- B: 170 (67% blue - creates peachy tone)

This creates a warm, peachy-coral tone that's softer and more approachable than bright red.

## Brightness Preservation

The script maintains the original brightness of each pixel:
- Dark red shadows → Dark peach shadows
- Bright pink highlights → Bright peach highlights
- Medium red petals → Medium peach petals

This preserves the 3D appearance and shading of the rose.

## Icon Generation

The peach rose will automatically be used for Windows icon generation:
- Icon file: `build/rose.ico`
- Embedded in: `zzavnad.exe`
- Transparency: White background removed (threshold: 240)

## To Revert to Original

If you want to go back to the red/pink rose:

```powershell
# Restore from backup
Copy-Item ".\rose_original_backup.png" ".\rose.png" -Force

# Then rebuild
cmake -S . -B build
cmake --build build
```

## To Adjust Peach Color

Edit `convert_rose_to_peach.ps1` lines 24-26:

```powershell
# Current peach (warm coral)
$peachR = 255
$peachG = 204
$peachB = 170

# For lighter peach:
# $peachR = 255
# $peachG = 218
# $peachB = 185

# For coral peach:
# $peachR = 255
# $peachG = 195
# $peachB = 160
```

Then run:
```powershell
powershell -ExecutionPolicy Bypass -File ".\convert_rose_to_peach.ps1"
```

## Next Steps

The peach rose will be used automatically in the next build:

```bash
# Clean rebuild to regenerate icon
rm -r build
cmake -S . -B build
cmake --build build
```

The executable will now show the peach-colored rose icon!

## Visual Comparison

- **Before**: Red/pink rose - classic, bold, traditional
- **After**: Peach rose - warm, soft, modern, approachable

The peach color maintains the rose aesthetic while giving the application a distinctive, gentler appearance.

---

**Date**: 2026-05-11  
**Tool**: `convert_rose_to_peach.ps1` (PowerShell + System.Drawing)  
**Pixels Converted**: 5,761  
**Original Preserved**: `rose_original_backup.png`
