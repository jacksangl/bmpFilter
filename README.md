# BMP Filter - Advanced Image Processing Tool

## Overview

This project began as an assignment for Harvard's CS50 course and I decided to add on seam carving capabilities. Written in C, this command-line tool implements multiple image filters and features a  seam carving algorithm for intelligent image compression.

## Features

### Standard Image Filters
- **Grayscale**: Convert color images to grayscale
- **Blur**: Apply Gaussian blur using a 3×3 kernel
- **Edge Detection**: Sobel operator-based edge detection
- **Reflection**: Horizontal image mirroring

### Advanced Seam Carving
- **Content-Aware Compression**: Removes least important vertical seams based on edge energy
- **Dynamic Programming**: Optimal seam detection using energy minimization
- **Memory Optimized**: In-place modification for large image processing
- **High Performance**: Handles HD images (1280×853) efficiently

## Technical Implementation

### Seam Carving Algorithm
The seam carving implementation uses:
- **Edge Energy Calculation**: Multi-channel Sobel operator for detecting image importance
- **Dynamic Programming**: Efficient seam path finding using cumulative energy matrices
- **Heap Memory Management**: Prevents stack overflow on large images
- **In-Place Optimization**: Eliminates redundant memory allocation for significant performance gains

### BMP Format Support
- **Universal Compatibility**: Supports BMP 3.0, 4.0, and 5.0 formats
- **Proper Header Management**: Accurate file size and dimension updates
- **Memory Safety**: Robust bounds checking and error handling

## Usage

### Compilation
```bash
make filter
```

### Standard Filters
```bash
# Apply grayscale filter
./filter -g input.bmp output.bmp

# Apply blur filter
./filter -b input.bmp output.bmp

# Apply edge detection
./filter -e input.bmp output.bmp

# Apply horizontal reflection
./filter -r input.bmp output.bmp
```

### Seam Carving (Content-Aware Compression)
```bash
# Compress image by 20% width using seam carving
./filter -s 20 input.bmp output.bmp

# Compress image by 10% width
./filter -s 10 input.bmp output.bmp
```

## Performance

- **Small Images** (600×400): ~1 second for 20% compression
- **HD Images** (1280×853): ~24 seconds for 10% compression
- **Memory Efficient**: O(width × height) space complexity
- **Optimized Algorithm**: In-place modification reduces memory overhead by 50%

## Technical Highlights

### Problem Solving
- **Fixed BMP Header Corruption**: Resolved incorrect `RGBTRIPLE` struct padding causing double file sizes
- **Stack Overflow Prevention**: Migrated large arrays from stack to heap allocation
- **Memory Leak Prevention**: Comprehensive memory management with proper cleanup

### Algorithm Optimization
- **Performance Enhancement**: Eliminated redundant memory allocation per seam removal
- **Space Complexity**: Reduced from O(n × width × height) to O(width × height)
- **Time Complexity**: Maintained O(n × width × height) while dramatically improving constants

## Troubleshooting & Bug Fixes

During development, several critical issues were identified and resolved:

### Issue 1: Memory Management in Working Image Buffer
**Problem**: The original implementation didn't properly handle the working image buffer as seams were removed, leading to memory corruption and incorrect results.

**Symptoms**: 
- Generated output files had corrupted pixel data
- Inconsistent image dimensions after processing
- Potential memory access violations

**Solution**: 
- Implemented a separate working buffer with original width dimensions
- Proper memory allocation and deallocation for the working image
- Clean separation between input processing and output generation

```c
// Fixed: Allocate working buffer for original width
RGBTRIPLE (*workingImage)[width] = malloc(height * width * sizeof(RGBTRIPLE));
```

### Issue 2: Seam Removal Logic with Variable Width Arrays
**Problem**: The separate `removeSeam()` function created complications with variable-length arrays as the image width changed during processing.

**Symptoms**:
- Array dimension mismatches causing undefined behavior
- Incorrect pixel shifting operations
- Memory boundary violations during seam removal

**Solution**:
- Integrated seam removal directly into the main `seamCarve()` function
- Eliminated the problematic separate function that caused dimension conflicts
- Direct pixel shifting with proper bounds checking

```c
// Fixed: Direct pixel shifting in seamCarve function
for (int j = seamCol; j < currentWidth - 1; j++) {
    workingImage[i][j] = workingImage[i][j + 1];
}
```

### Issue 3: Width Tracking Throughout Processing
**Problem**: Inconsistent tracking of the current image width as seams were progressively removed.

**Symptoms**:
- Seam detection algorithms operating on incorrect width values
- Energy calculations accessing invalid memory regions
- Output files with incorrect final dimensions

**Solution**:
- Implemented robust `currentWidth` variable tracking
- Proper width validation at each seam removal step
- Comprehensive bounds checking for all array operations

### Issue 4: Memory Safety and Error Handling
**Problem**: Insufficient error handling for memory allocation failures and invalid seam positions.

**Symptoms**:
- Potential crashes on large images or low memory conditions
- Invalid seam coordinates causing array out-of-bounds access
- Silent failures with corrupted output

**Solution**:
- Added comprehensive memory allocation error checking
- Implemented seam position validation with detailed error messages
- Graceful handling of edge cases (minimum width constraints)

```c
// Fixed: Robust error checking
if (seamCol < 0 || seamCol >= currentWidth) {
    fprintf(stderr, "ERROR: Invalid seam position %d at row %d\n", seamCol, i);
    continue;
}
```

### Verification Results
After implementing these fixes:
- ✅ Consistent file size reduction proportional to compression percentage
- ✅ Valid BMP files with correct headers and dimensions
- ✅ No memory leaks or access violations
- ✅ Proper handling of various image sizes and compression ratios

**Example**: 600×400 image with 20% compression:
- Input: 720,056 bytes → Output: 576,054 bytes
- Width: 600 pixels → 480 pixels (exactly 20% reduction)

## File Structure

```
bmpFilter/
├── filter.c          # Main program and argument parsing
├── helpers.c         # Image processing algorithms
├── helpers.h         # Function declarations
├── bmp.h            # BMP format definitions
├── Makefile         # Build configuration
└── images/          # Sample images for testing
```

## Requirements

- **Compiler**: GCC or Clang with C11 support
- **Platform**: Unix-like systems (macOS, Linux)
- **Memory**: Sufficient RAM for image dimensions (HD images ~10MB)

## Example Results

### Seam Carving Compression
- **Input**: 1280×853 HD image (3.2MB)
- **Output**: 1152×853 compressed image (2.9MB)
- **Compression**: 10% width reduction while preserving important visual content
- **Processing Time**: ~24 seconds

## Potential Future Improvements

- **PNG Support**: Extend file format support to include PNG images with transparency handling
- **JPEG Support**: Add JPEG file format compatibility with quality preservation
- **Horizontal Seam Carving**: Implement height reduction through horizontal seam removal
- **Multi-threaded Processing**: Parallelize energy calculation for improved performance on multi-core systems
- **Interactive Preview**: Real-time seam visualization before processing
- **Batch Processing**: Support for processing multiple images in a single command

---
