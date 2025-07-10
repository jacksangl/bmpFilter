#include "helpers.h"
#include <math.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// Helper function to find minimum of two integers
int min(int a, int b)
{
    return (a < b) ? a : b;
}

// functions to calculate the avg rgb for pixels in the middle of an image

// Convert image to grayscale
// we can take the average of the red, green, and blue values to determine what shade of grey to make the new pixel.
// set each color value to the average
// how to test ./filter -g images/yard.bmp outfile.bmp
void grayscale(int height, int width, RGBTRIPLE image[height][width])
{
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            float average = ((image[i][j].rgbtRed + image[i][j].rgbtGreen + image[i][j].rgbtBlue) / 3.0);
            int intAverage = round(average);
            image[i][j].rgbtRed = intAverage;
            image[i][j].rgbtGreen = intAverage;
            image[i][j].rgbtBlue = intAverage;
        }
    }
    return;
}

// Reflect image horizontally
// any pixels on the left side of the image should end up on the right, and vice versa
void reflect(int height, int width, RGBTRIPLE image[height][width])
{
    RGBTRIPLE buffer;
    for (int i = 0; i < height; i++) {
        // maybe need to divide by 2??
        for (int j = 0; j < width / 2; j++) {
            // set the going to be swapped colors to a buffer
            buffer.rgbtRed = image[i][j].rgbtRed;
            buffer.rgbtGreen = image[i][j].rgbtGreen;
            buffer.rgbtBlue = image[i][j].rgbtBlue;

            // perform the swap
            image[i][j].rgbtRed = image[i][width - 1 - j].rgbtRed;
            image[i][j].rgbtGreen = image[i][width - 1 - j].rgbtGreen;
            image[i][j].rgbtBlue = image[i][width - 1 - j].rgbtBlue;

            image[i][width - 1 - j].rgbtRed = buffer.rgbtRed;
            image[i][width - 1 - j].rgbtGreen = buffer.rgbtGreen;
            image[i][width - 1 - j].rgbtBlue = buffer.rgbtBlue;
        }
    }
    return;
}

// Helper function to calculate blur for a single pixel at position (i, j)
void blurPixel(int i, int j, int height, int width, RGBTRIPLE original[height][width], RGBTRIPLE *result)
{
    int redSum = 0, greenSum = 0, blueSum = 0;
    int count = 0;
    
    // Check all pixels in 3x3 grid around current pixel
    for (int di = -1; di <= 1; di++) {
        for (int dj = -1; dj <= 1; dj++) {
            int ni = i + di;
            int nj = j + dj;
            
            // Check if neighbor is within bounds
            if (ni >= 0 && ni < height && nj >= 0 && nj < width) {
                redSum += original[ni][nj].rgbtRed;
                greenSum += original[ni][nj].rgbtGreen;
                blueSum += original[ni][nj].rgbtBlue;
                count++;
            }
        }
    }
    
    // Calculate averages and round
    result->rgbtRed = round((float)redSum / count);
    result->rgbtGreen = round((float)greenSum / count);
    result->rgbtBlue = round((float)blueSum / count);
}

// Blur image
// box blur, which works by taking each pixel and, for each color value, giving it a new value by averaging the color values of neighboring pixels
// new value of each pixel would be the average of the values of all of the pixels that are within 1 row and column of the original pixel (forming a 3x3 box)
// For a pixel along the edge or corner, we would still look for all pixels within 1 row and column
void blur(int height, int width, RGBTRIPLE image[height][width])
{
    // Create a copy of the original image
    RGBTRIPLE imageCopy[height][width];
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            imageCopy[i][j] = image[i][j];
        }
    }

    // Apply blur to each pixel
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            blurPixel(i, j, height, width, imageCopy, &image[i][j]);
        }
    }
    return;
}

// Helper function to calculate edge detection for a single pixel at position (i, j)
void edgePixel(int i, int j, int height, int width, RGBTRIPLE original[height][width], RGBTRIPLE *result)
{
    const int CAP = 255;
    
    // For each color channel (0=red, 1=green, 2=blue)
    for (int color = 0; color < 3; color++) {
        // Get the 3x3 grid of color values around the current pixel
        // Treat out-of-bounds pixels as 0
        int grid[9];
        int idx = 0;
        
        // Fill grid in order: topLeft, top, topRight, middleLeft, middle, middleRight, bottomLeft, bottom, bottomRight
        for (int di = -1; di <= 1; di++) {
            for (int dj = -1; dj <= 1; dj++) {
                int ni = i + di;
                int nj = j + dj;
                
                if (ni >= 0 && ni < height && nj >= 0 && nj < width) {
                    grid[idx] = getColorChannel(original[ni][nj], color);
                } else {
                    grid[idx] = 0; 
                }
                idx++;
            }
        }
        
        // Calculate gx and gy using the grid values
        double bufferX = gxMatrix(grid[0], grid[1], grid[2], grid[3], grid[4], grid[5], grid[6], grid[7], grid[8]);
        double bufferY = gyMatrix(grid[0], grid[1], grid[2], grid[3], grid[4], grid[5], grid[6], grid[7], grid[8]);
        
        // Calculate final edge value using proper Sobel magnitude: sqrt(gx² + gy²)
        double buffer = sqrt(bufferX * bufferX + bufferY * bufferY);
        if (buffer > CAP) {
            buffer = CAP;
        }
        buffer = round(buffer);
        
        // Set the color channel
        setColorChannel(result, color, (int)buffer);
    }
}

// Detect edges
// create a 3x3 grid around pixel // for border pixes treat any pixel past the border having all 0 values
// gx
// -1, 0, 1
// -2, 0, 2
// -1, 0, 1
// gy
// -1, -2, -1
//  0,  0,  0,
//  1,  2,  1
// compute each new channel value as the square root of gx^2 + gy^2
void edges(int height, int width, RGBTRIPLE image[height][width])
{
    // Create a copy of the original image
    RGBTRIPLE imageCopy[height][width];
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            imageCopy[i][j] = image[i][j];
        }
    }

    // Apply edge detection to each pixel
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            edgePixel(i, j, height, width, imageCopy, &image[i][j]);
        }
    }
    return;
}

// edge functions
double gxMatrix(int topLeft, int top, int topRight, int middleLeft, int middle, int middleRight, int bottomLeft, int bottom, int bottomRight)
{
    double gx = (topLeft * -1 + top * 0 + topRight * 1 + middleLeft * -2 + middle * 0 + middleRight * 2 + bottomLeft * -1 + bottom * 0 + bottomRight * 1);
    return gx;  // Return gx directly, don't square it here
}

double gyMatrix(int topLeft, int top, int topRight, int middleLeft, int middle, int middleRight, int bottomLeft, int bottom, int bottomRight)
{
    double gy = (topLeft * -1 + top * -2 + topRight * -1 + middleLeft * 0 + middle * 0 + middleRight * 0 + bottomLeft * 1 + bottom * 2 + bottomRight * 1);
    return gy;  // Return gy directly, don't square it here
}

// function definitions
int averageRed9(int topLeft, int top, int topRight, int middleLeft, int middle, int middleRight, int bottomLeft, int bottom, int bottomRight)
{
    float average = (topLeft + top + topRight + middleLeft + middle + middleRight + bottomLeft + bottom + bottomRight) / 9.0;
    int averageReturn = round(average);
    return averageReturn;
}

int averageGreen9(int topLeft, int top, int topRight, int middleLeft, int middle, int middleRight, int bottomLeft, int bottom, int bottomRight)
{
    float average = (topLeft + top + topRight + middleLeft + middle + middleRight + bottomLeft + bottom + bottomRight) / 9.0;
    int averageReturn = round(average);
    return averageReturn;
}

int averageBlue9(int topLeft, int top, int topRight, int middleLeft, int middle, int middleRight, int bottomLeft, int bottom, int bottomRight)
{
    float average = (topLeft + top + topRight + middleLeft + middle + middleRight + bottomLeft + bottom + bottomRight) / 9.0;
    int averageReturn = round(average);
    return averageReturn;
}

// Helper function to get color channel value by index (0=red, 1=green, 2=blue)
int getColorChannel(RGBTRIPLE pixel, int channel)
{
    switch (channel) {
        case 0: return pixel.rgbtRed;
        case 1: return pixel.rgbtGreen;
        case 2: return pixel.rgbtBlue;
        default: return 0;
    }
}

// Helper function to set color channel value by index (0=red, 1=green, 2=blue)
void setColorChannel(RGBTRIPLE *pixel, int channel, int value)
{
    switch (channel) {
        case 0: pixel->rgbtRed = value; break;
        case 1: pixel->rgbtGreen = value; break;
        case 2: pixel->rgbtBlue = value; break;
    }
}

// EDGE ENERGY FOR SEAM DETECTION
double edgeEnergy(int i, int j, int height, int currentWidth, RGBTRIPLE image[height][currentWidth])
{
    // For each color channel (0=red, 1=green, 2=blue)
    double totalEnergy = 0.0;
    
    for (int color = 0; color < 3; color++) {
        // Get the 3x3 grid of color values around the current pixel
        // Treat out-of-bounds pixels as edge pixels (replicate edge values)
        int grid[9];
        int idx = 0;
        
        // Fill grid in order: topLeft, top, topRight, middleLeft, middle, middleRight, bottomLeft, bottom, bottomRight
        for (int di = -1; di <= 1; di++) {
            for (int dj = -1; dj <= 1; dj++) {
                int ni = i + di;
                int nj = j + dj;
                
                // Clamp to image boundaries (replicate edge pixels)
                if (ni < 0) ni = 0;
                if (ni >= height) ni = height - 1;
                if (nj < 0) nj = 0;
                if (nj >= currentWidth) nj = currentWidth - 1;
                
                grid[idx] = getColorChannel(image[ni][nj], color);
                idx++;
            }
        }
        
        // Calculate gx and gy using sobel operator
        int gx = -1*grid[0] + 0*grid[1] + 1*grid[2] +
                 -2*grid[3] + 0*grid[4] + 2*grid[5] +
                 -1*grid[6] + 0*grid[7] + 1*grid[8];
                 
        int gy = -1*grid[0] + -2*grid[1] + -1*grid[2] +
                  0*grid[3] +  0*grid[4] +  0*grid[5] +
                  1*grid[6] +  2*grid[7] +  1*grid[8];
        
        // Calculate energy for this channel and add to total
        double channelEnergy = sqrt((double)gx * gx + (double)gy * gy);
        totalEnergy += channelEnergy;
    }
    
    // Return total energy (not average) - this is more standard for seam carving
    return totalEnergy;
}

void findSeam(int height, int currentWidth, RGBTRIPLE image[height][currentWidth], int *seam)
{
    
    // Input validation
    if (currentWidth <= 0 || height <= 0) {
        fprintf(stderr, "Invalid dimensions: %dx%d\n", height, currentWidth);
        return;
    }
    
    // Allocate energy matrix on heap to avoid stack overflow with large images
    double (*M)[currentWidth] = malloc(height * currentWidth * sizeof(double));
    if (M == NULL) {
        fprintf(stderr, "Failed to allocate memory for energy matrix\n");
        return;
    }
    
    // 1. Calculate energy and initialize first row
    for (int j = 0; j < currentWidth; j++) {
        M[0][j] = edgeEnergy(0, j, height, currentWidth, image);
    }
    
    // 2. Fill DP table
    for (int i = 1; i < height; i++) {
        for (int j = 0; j < currentWidth; j++) {
            double current_energy = edgeEnergy(i, j, height, currentWidth, image);
            double min_prev = M[i-1][j]; // directly above
            
            // Check diagonal left (only if j > 0)
            if (j > 0 && M[i-1][j-1] < min_prev) {
                min_prev = M[i-1][j-1];
            }
            // Check diagonal right (only if j < width-1)
            if (j < currentWidth-1 && M[i-1][j+1] < min_prev) {
                min_prev = M[i-1][j+1];
            }
            
            M[i][j] = current_energy + min_prev;
        }
    }
    
    // 3. Find minimum in last row
    int min_col = 0;
    for (int j = 1; j < currentWidth; j++) {
        if (M[height-1][j] < M[height-1][min_col]) {
            min_col = j;
        }
    }
    
    // 4. Trace back the seam
    seam[height-1] = min_col;
    
    for (int i = height-2; i >= 0; i--) {
        int j = seam[i+1];
        int best_j = j;
        double best_energy = M[i][j];
        
        // Check left diagonal (only if valid)
        if (j > 0 && M[i][j-1] < best_energy) {
            best_j = j - 1;
            best_energy = M[i][j-1];
        }
        
        // Check right diagonal (only if valid)
        if (j < currentWidth-1 && M[i][j+1] < best_energy) {
            best_j = j + 1;
            best_energy = M[i][j+1];
        }
        
        seam[i] = best_j;
    }
    
    // Validate the computed seam
    for (int i = 0; i < height; i++) {
        if (seam[i] < 0 || seam[i] >= currentWidth) {
            fprintf(stderr, "ERROR: Computed invalid seam at row %d: %d (width=%d)\n", 
                    i, seam[i], currentWidth);
            // Set to a safe value
            seam[i] = (seam[i] < 0) ? 0 : currentWidth-1;
        }
    }
    
    // Free the energy matrix
    free(M);
}


int seamCarve(int height, int width, RGBTRIPLE image[height][width], int compressPercent)
{
    if (compressPercent <= 0 || compressPercent >= 100) {
        printf("Invalid compression percentage: %d\n", compressPercent);
        return width;
    }
    
    // Calculate how many seams to remove based on compression percentage
    int seamsToRemove = (width * compressPercent) / 100;
    // these are stupid and are just here incase the top input validation fails
    if (seamsToRemove >= width) {
        seamsToRemove = width - 1; // Don't remove all columns
    }
    if (seamsToRemove <= 0) {
        return width; // No seams to remove
    }
    
    printf("Removing %d seams from image of width %d\n", seamsToRemove, width);
    
    // Allocate a separate working buffer that we'll use for computation
    // This is sized to the original width
    RGBTRIPLE (*workingImage)[width] = malloc(height * width * sizeof(RGBTRIPLE));
    if (workingImage == NULL) {
        fprintf(stderr, "Failed to allocate memory for working image\n");
        return width;
    }

    // Copy the original image to working buffer
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            workingImage[i][j] = image[i][j];
        }
    }
    
    int currentWidth = width;
    int *seam = malloc(height * sizeof(int));
    if (seam == NULL) {
        fprintf(stderr, "Failed to allocate memory for seam\n");
        free(workingImage);
        return width;
    }
    
    // Remove seams one by one
    for (int n = 0; n < seamsToRemove; n++) {
        printf("Removing seam %d/%d (current width: %d)\n", n+1, seamsToRemove, currentWidth);
        
        // Find the minimum energy seam
        findSeam(height, currentWidth, workingImage, seam);
        
        // Remove the seam - shift pixels left
        for (int i = 0; i < height; i++) {
            int seamCol = seam[i];
            
            // Validate seam position
            if (seamCol < 0 || seamCol >= currentWidth) {
                fprintf(stderr, "ERROR: Invalid seam position %d at row %d (currentWidth=%d)\n", 
                        seamCol, i, currentWidth);
                continue;
            }
            
            // Shift all pixels to the left starting from the seam position
            for (int j = seamCol; j < currentWidth - 1; j++) {
                workingImage[i][j] = workingImage[i][j + 1];
            }
        }
        // I could put this in a function but I'm not sure if it's worth it
        
        currentWidth--;
        
        // Sanity check
        if (currentWidth <= 1) {
            printf("Reached minimum width, stopping\n");
            break;
        }
    }
    
    // Copy the carved result back to the original image array
    for (int i = 0; i < height; i++) {
        // Copy the remaining pixels
        for (int j = 0; j < currentWidth; j++) {
            image[i][j] = workingImage[i][j];
        }
        // Clear the removed pixels (set to black)
        for (int j = currentWidth; j < width; j++) {
            image[i][j].rgbtRed = 0;
            image[i][j].rgbtGreen = 0;
            image[i][j].rgbtBlue = 0;
        }
    }

    free(seam);
    free(workingImage);
    return currentWidth; // Return the new width after seam removal
}
