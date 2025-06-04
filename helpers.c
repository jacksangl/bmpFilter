#include "helpers.h"
#include <math.h>

// functions to calculate the avg rgb for pixels in the middle of an image
int averageRed9(int topLeft, int top, int topRight, int middleLeft, int middle, int middleRight, int bottomLeft, int bottom, int bottomRight);
int averageGreen9(int topLeft, int top, int topRight, int middleLeft, int middle, int middleRight, int bottomLeft, int bottom, int bottomRight);
int averageBlue9(int topLeft, int top, int topRight, int middleLeft, int middle, int middleRight, int bottomLeft, int bottom, int bottomRight);
//functions to calculate on the side
int getColorChannel(RGBTRIPLE pixel, int channel);
void setColorChannel(RGBTRIPLE *pixel, int channel, int value);

//edge functions
double gyMatrix(int topLeft, int top, int topRight, int middleLeft, int middle, int middleRight, int bottomLeft, int bottom, int bottomRight);
double gxMatrix(int topLeft, int top, int topRight, int middleLeft, int middle, int middleRight, int bottomLeft, int bottom, int bottomRight);

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
    const int CAP = 255;
    double bufferX = 0;
    double bufferY = 0;
    double buffer = 0;
    RGBTRIPLE imageCopy[height][width];

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            imageCopy[i][j] = image[i][j];
        }
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            // if on top row
            if (i == 0) {
                // if top left corner
                if (j == 0) {
                    for (int color = 0; color < 3; color++) {
                        bufferX = gxMatrix(getColorChannel(imageCopy[i + 1][j + 1], color), getColorChannel(imageCopy[i + 1][j], color), 0,
                                getColorChannel(imageCopy[i][j + 1], color), getColorChannel(imageCopy[i][j], color), 0, 0,
                                0, 0);
                        bufferY = gyMatrix(getColorChannel(imageCopy[i + 1][j + 1], color), getColorChannel(imageCopy[i + 1][j], color), 0,
                                getColorChannel(imageCopy[i][j + 1], color), getColorChannel(imageCopy[i][j], color), 0, 0,
                                0, 0);
                        
                        buffer = sqrt(bufferX + bufferY);
                        if (buffer > CAP) {
                            buffer = CAP;
                        }
                        buffer = round(buffer);
                        setColorChannel(&image[i][j], color, buffer);
                    }
                }
                // if top right corner
                else if (j == width - 1) {
                    for (int color = 0; color < 3; color++) {
                        switch (color) {
                            case 0: 
                                bufferX = gxMatrix(0, imageCopy[i + 1][j].rgbtRed, imageCopy[i + 1][j - 1].rgbtRed,
                                        0, imageCopy[i][j].rgbtRed, imageCopy[i][j - 1].rgbtRed, 0,
                                        0, 0);
                                bufferY = gyMatrix(0, imageCopy[i + 1][j].rgbtRed, imageCopy[i + 1][j - 1].rgbtRed,
                                        0, imageCopy[i][j].rgbtRed, imageCopy[i][j - 1].rgbtRed, 0,
                                        0, 0);
                                break;

                            case 1: 
                                bufferX = gxMatrix(0, imageCopy[i + 1][j].rgbtGreen, imageCopy[i + 1][j - 1].rgbtGreen,
                                        0, imageCopy[i][j].rgbtGreen, imageCopy[i][j - 1].rgbtGreen, 0,
                                        0, 0);
                                bufferY = gyMatrix(0, imageCopy[i + 1][j].rgbtGreen, imageCopy[i + 1][j - 1].rgbtGreen,
                                        0, imageCopy[i][j].rgbtGreen, imageCopy[i][j - 1].rgbtGreen, 0,
                                        0, 0);
                                break;

                            case 2: 
                                bufferX = gxMatrix(0, imageCopy[i + 1][j].rgbtBlue, imageCopy[i + 1][j - 1].rgbtBlue,
                                        0, imageCopy[i][j].rgbtBlue, imageCopy[i][j - 1].rgbtBlue, 0,
                                        0, 0);
                                bufferY = gyMatrix(0, imageCopy[i + 1][j].rgbtBlue, imageCopy[i + 1][j - 1].rgbtBlue,
                                        0, imageCopy[i][j].rgbtBlue, imageCopy[i][j - 1].rgbtBlue, 0,
                                        0, 0);
                                break;
                        }
                        buffer = sqrt(bufferX + bufferY);
                        if (buffer > CAP) {
                            buffer = CAP;
                        }
                        buffer = round(buffer);
                        if (color == 0) {
                            image[i][j].rgbtRed = buffer;
                        } else if (color == 1) {
                            image[i][j].rgbtGreen = buffer;
                        } else {
                            image[i][j].rgbtBlue = buffer;
                        }
                    }
                }
                // if just top row
                else {
                    for (int color = 0; color < 3; color++) {
                        switch (color) {
                            case 0: 
                                bufferX = gxMatrix(imageCopy[i + 1][j + 1].rgbtRed, imageCopy[i + 1][j].rgbtRed, imageCopy[i + 1][j - 1].rgbtRed,
                                        imageCopy[i][j + 1].rgbtRed, imageCopy[i][j].rgbtRed, imageCopy[i][j - 1].rgbtRed, 0,
                                        0, 0);
                                bufferY = gyMatrix(imageCopy[i + 1][j + 1].rgbtRed, imageCopy[i + 1][j].rgbtRed, imageCopy[i + 1][j - 1].rgbtRed,
                                        imageCopy[i][j + 1].rgbtRed, imageCopy[i][j].rgbtRed, imageCopy[i][j - 1].rgbtRed, 0,
                                        0, 0);
                                break;

                            case 1: 
                                bufferX = gxMatrix(imageCopy[i + 1][j + 1].rgbtGreen, imageCopy[i + 1][j].rgbtGreen, imageCopy[i + 1][j - 1].rgbtGreen,
                                        imageCopy[i][j + 1].rgbtGreen, imageCopy[i][j].rgbtGreen, imageCopy[i][j - 1].rgbtGreen, 0,
                                        0, 0);
                                bufferY = gyMatrix(imageCopy[i + 1][j + 1].rgbtGreen, imageCopy[i + 1][j].rgbtGreen, imageCopy[i + 1][j - 1].rgbtGreen,
                                        imageCopy[i][j + 1].rgbtGreen, imageCopy[i][j].rgbtGreen, imageCopy[i][j - 1].rgbtGreen, 0,
                                        0, 0);
                                break;

                            case 2: 
                                bufferX = gxMatrix(imageCopy[i + 1][j + 1].rgbtBlue, imageCopy[i + 1][j].rgbtBlue, imageCopy[i + 1][j - 1].rgbtBlue,
                                        imageCopy[i][j + 1].rgbtBlue, imageCopy[i][j].rgbtBlue, imageCopy[i][j - 1].rgbtBlue, 0,
                                        0, 0);
                                bufferY = gyMatrix(imageCopy[i + 1][j + 1].rgbtBlue, imageCopy[i + 1][j].rgbtBlue, imageCopy[i + 1][j - 1].rgbtBlue,
                                        imageCopy[i][j + 1].rgbtBlue, imageCopy[i][j].rgbtBlue, imageCopy[i][j - 1].rgbtBlue, 0,
                                        0, 0);
                                break;
                        }
                        buffer = sqrt(bufferX + bufferY);
                        if (buffer > CAP) {
                            buffer = CAP;
                        }
                        buffer = round(buffer);
                        if (color == 0) {
                            image[i][j].rgbtRed = buffer;
                        } else if (color == 1) {
                            image[i][j].rgbtGreen = buffer;
                        } else {
                            image[i][j].rgbtBlue = buffer;
                        }
                    }
                }
            }
            // if on bottom row
            else if (i == height - 1) {
                // if bottom left corner
                if (j == 0) {
                    for (int color = 0; color < 3; color++) {
                        switch (color) {
                            case 0: 
                                bufferX = gxMatrix(0, 0, 0,
                                        imageCopy[i][j + 1].rgbtRed, imageCopy[i][j].rgbtRed, 0, imageCopy[i - 1][j + 1].rgbtRed,
                                        imageCopy[i - 1][j].rgbtRed, 0);
                                bufferY = gyMatrix(0, 0, 0,
                                        imageCopy[i][j + 1].rgbtRed, imageCopy[i][j].rgbtRed, 0, imageCopy[i - 1][j + 1].rgbtRed,
                                        imageCopy[i - 1][j].rgbtRed, 0);
                                break;

                            case 1: 
                                bufferX = gxMatrix(0, 0, 0,
                                        imageCopy[i][j + 1].rgbtGreen, imageCopy[i][j].rgbtGreen, 0, imageCopy[i - 1][j + 1].rgbtGreen,
                                        imageCopy[i - 1][j].rgbtGreen, 0);
                                bufferY = gyMatrix(0, 0, 0,
                                        imageCopy[i][j + 1].rgbtGreen, imageCopy[i][j].rgbtGreen, 0, imageCopy[i - 1][j + 1].rgbtGreen,
                                        imageCopy[i - 1][j].rgbtGreen, 0);
                                break;

                            case 2: 
                                bufferX = gxMatrix(0, 0, 0,
                                        imageCopy[i][j + 1].rgbtBlue, imageCopy[i][j].rgbtBlue, 0, imageCopy[i - 1][j + 1].rgbtBlue,
                                        imageCopy[i - 1][j].rgbtBlue, 0);
                                bufferY = gyMatrix(0, 0, 0,
                                        imageCopy[i][j + 1].rgbtBlue, imageCopy[i][j].rgbtBlue, 0, imageCopy[i - 1][j + 1].rgbtBlue,
                                        imageCopy[i - 1][j].rgbtBlue, 0);
                                break;
                        }
                        buffer = sqrt(bufferX + bufferY);
                        if (buffer > CAP) {
                            buffer = CAP;
                        }
                        buffer = round(buffer);
                        if (color == 0) {
                            image[i][j].rgbtRed = buffer;
                        } else if (color == 1) {
                            image[i][j].rgbtGreen = buffer;
                        } else {
                            image[i][j].rgbtBlue = buffer;
                        }
                    }
                }
                // if bottom right corner
                else if (j == width - 1) {
                    for (int color = 0; color < 3; color++) {
                        switch (color) {
                            case 0: 
                                bufferX = gxMatrix(0, 0, 0,
                                        0, imageCopy[i][j].rgbtRed, imageCopy[i][j - 1].rgbtRed, 0,
                                        imageCopy[i - 1][j].rgbtRed, imageCopy[i - 1][j - 1].rgbtRed);
                                bufferY = gyMatrix(0, 0, 0,
                                        0, imageCopy[i][j].rgbtRed, imageCopy[i][j - 1].rgbtRed, 0,
                                        imageCopy[i - 1][j].rgbtRed, imageCopy[i - 1][j - 1].rgbtRed);
                                break;

                            case 1: 
                                bufferX = gxMatrix(0, 0, 0,
                                        0, imageCopy[i][j].rgbtGreen, imageCopy[i][j - 1].rgbtGreen, 0,
                                        imageCopy[i - 1][j].rgbtGreen, imageCopy[i - 1][j - 1].rgbtGreen);
                                bufferY = gyMatrix(0, 0, 0,
                                        0, imageCopy[i][j].rgbtGreen, imageCopy[i][j - 1].rgbtGreen, 0,
                                        imageCopy[i - 1][j].rgbtGreen, imageCopy[i - 1][j - 1].rgbtGreen);
                                break;

                            case 2: 
                                bufferX = gxMatrix(0, 0, 0,
                                        0, imageCopy[i][j].rgbtBlue, imageCopy[i][j - 1].rgbtBlue, 0,
                                        imageCopy[i - 1][j].rgbtBlue, imageCopy[i - 1][j - 1].rgbtBlue);
                                bufferY = gyMatrix(0, 0, 0,
                                        0, imageCopy[i][j].rgbtBlue, imageCopy[i][j - 1].rgbtBlue, 0,
                                        imageCopy[i - 1][j].rgbtBlue, imageCopy[i - 1][j - 1].rgbtBlue);
                                break;
                        }
                        buffer = sqrt(bufferX + bufferY);
                        if (buffer > CAP) {
                            buffer = CAP;
                        }
                        buffer = round(buffer);
                        if (color == 0) {
                            image[i][j].rgbtRed = buffer;
                        } else if (color == 1) {
                            image[i][j].rgbtGreen = buffer;
                        } else {
                            image[i][j].rgbtBlue = buffer;
                        }
                    }
                }
                // just on bottom
                else {
                    for (int color = 0; color < 3; color++) {
                        switch (color) {
                            case 0: 
                                bufferX = gxMatrix(0, 0, 0,
                                        imageCopy[i][j + 1].rgbtRed, imageCopy[i][j].rgbtRed, imageCopy[i][j - 1].rgbtRed, imageCopy[i - 1][j + 1].rgbtRed,
                                        imageCopy[i - 1][j].rgbtRed, imageCopy[i - 1][j - 1].rgbtRed);
                                bufferY = gyMatrix(0, 0, 0,
                                        imageCopy[i][j + 1].rgbtRed, imageCopy[i][j].rgbtRed, imageCopy[i][j - 1].rgbtRed, imageCopy[i - 1][j + 1].rgbtRed,
                                        imageCopy[i - 1][j].rgbtRed, imageCopy[i - 1][j - 1].rgbtRed);
                                break;

                            case 1: 
                                bufferX = gxMatrix(0, 0, 0,
                                        imageCopy[i][j + 1].rgbtGreen, imageCopy[i][j].rgbtGreen, imageCopy[i][j - 1].rgbtGreen, imageCopy[i - 1][j + 1].rgbtGreen,
                                        imageCopy[i - 1][j].rgbtGreen, imageCopy[i - 1][j - 1].rgbtGreen);
                                bufferY = gyMatrix(0, 0, 0,
                                        imageCopy[i][j + 1].rgbtGreen, imageCopy[i][j].rgbtGreen, imageCopy[i][j - 1].rgbtGreen, imageCopy[i - 1][j + 1].rgbtGreen,
                                        imageCopy[i - 1][j].rgbtGreen, imageCopy[i - 1][j - 1].rgbtGreen);
                                break;

                            case 2: 
                                bufferX = gxMatrix(0, 0, 0,
                                        imageCopy[i][j + 1].rgbtBlue, imageCopy[i][j].rgbtBlue, imageCopy[i][j - 1].rgbtBlue, imageCopy[i - 1][j + 1].rgbtBlue,
                                        imageCopy[i - 1][j].rgbtBlue, imageCopy[i - 1][j - 1].rgbtBlue);
                                bufferY = gyMatrix(0, 0, 0,
                                        imageCopy[i][j + 1].rgbtBlue, imageCopy[i][j].rgbtBlue, imageCopy[i][j - 1].rgbtBlue, imageCopy[i - 1][j + 1].rgbtBlue,
                                        imageCopy[i - 1][j].rgbtBlue, imageCopy[i - 1][j - 1].rgbtBlue);
                                break;
                        }
                        buffer = sqrt(bufferX + bufferY);
                        if (buffer > CAP) {
                            buffer = CAP;
                        }
                        buffer = round(buffer);
                        if (color == 0) {
                            image[i][j].rgbtRed = buffer;
                        } else if (color == 1) {
                            image[i][j].rgbtGreen = buffer;
                        } else {
                            image[i][j].rgbtBlue = buffer;
                        }
                    }
                }
            }
            // if on left
            else if (j == 0) {
                for (int color = 0; color < 3; color++) {
                    switch (color) {
                        case 0: 
                            bufferX = gxMatrix(imageCopy[i + 1][j + 1].rgbtRed, imageCopy[i + 1][j].rgbtRed, 0,
                                    imageCopy[i][j + 1].rgbtRed, imageCopy[i][j].rgbtRed, 0, imageCopy[i - 1][j + 1].rgbtRed,
                                    imageCopy[i - 1][j].rgbtRed, 0);
                            bufferY = gyMatrix(imageCopy[i + 1][j + 1].rgbtRed, imageCopy[i + 1][j].rgbtRed, 0,
                                    imageCopy[i][j + 1].rgbtRed, imageCopy[i][j].rgbtRed, 0, imageCopy[i - 1][j + 1].rgbtRed,
                                    imageCopy[i - 1][j].rgbtRed, 0);
                            break;

                        case 1: 
                            bufferX = gxMatrix(imageCopy[i + 1][j + 1].rgbtGreen, imageCopy[i + 1][j].rgbtGreen, 0,
                                    imageCopy[i][j + 1].rgbtGreen, imageCopy[i][j].rgbtGreen, 0, imageCopy[i - 1][j + 1].rgbtGreen,
                                    imageCopy[i - 1][j].rgbtGreen, 0);
                            bufferY = gyMatrix(imageCopy[i + 1][j + 1].rgbtGreen, imageCopy[i + 1][j].rgbtGreen, 0,
                                    imageCopy[i][j + 1].rgbtGreen, imageCopy[i][j].rgbtGreen, 0, imageCopy[i - 1][j + 1].rgbtGreen,
                                    imageCopy[i - 1][j].rgbtGreen, 0);
                            break;

                        case 2: 
                            bufferX = gxMatrix(imageCopy[i + 1][j + 1].rgbtBlue, imageCopy[i + 1][j].rgbtBlue, 0,
                                    imageCopy[i][j + 1].rgbtBlue, imageCopy[i][j].rgbtBlue, 0, imageCopy[i - 1][j + 1].rgbtBlue,
                                    imageCopy[i - 1][j].rgbtBlue, 0);
                            bufferY = gyMatrix(imageCopy[i + 1][j + 1].rgbtBlue, imageCopy[i + 1][j].rgbtBlue, 0,
                                    imageCopy[i][j + 1].rgbtBlue, imageCopy[i][j].rgbtBlue, 0, imageCopy[i - 1][j + 1].rgbtBlue,
                                    imageCopy[i - 1][j].rgbtBlue, 0);
                            break;
                    }
                    buffer = sqrt(bufferX + bufferY);
                    if (buffer > CAP) {
                        buffer = CAP;
                    }
                    buffer = round(buffer);
                    if (color == 0) {
                        image[i][j].rgbtRed = buffer;
                    } else if (color == 1) {
                        image[i][j].rgbtGreen = buffer;
                    } else {
                        image[i][j].rgbtBlue = buffer;
                    }
                }
            }
            // if on right
            else if (j == width - 1) {
                for (int color = 0; color < 3; color++) {
                    switch (color) {
                        case 0: 
                            bufferX = gxMatrix(0, imageCopy[i + 1][j].rgbtRed, imageCopy[i + 1][j - 1].rgbtRed,
                                    0, imageCopy[i][j].rgbtRed, imageCopy[i][j - 1].rgbtRed, 0,
                                    imageCopy[i - 1][j].rgbtRed, imageCopy[i - 1][j - 1].rgbtRed);
                            bufferY = gyMatrix(0, imageCopy[i + 1][j].rgbtRed, imageCopy[i + 1][j - 1].rgbtRed,
                                    0, imageCopy[i][j].rgbtRed, imageCopy[i][j - 1].rgbtRed, 0,
                                    imageCopy[i - 1][j].rgbtRed, imageCopy[i - 1][j - 1].rgbtRed);
                            break;

                        case 1: 
                            bufferX = gxMatrix(0, imageCopy[i + 1][j].rgbtGreen, imageCopy[i + 1][j - 1].rgbtGreen,
                                    0, imageCopy[i][j].rgbtGreen, imageCopy[i][j - 1].rgbtGreen, 0,
                                    imageCopy[i - 1][j].rgbtGreen, imageCopy[i - 1][j - 1].rgbtGreen);
                            bufferY = gyMatrix(0, imageCopy[i + 1][j].rgbtGreen, imageCopy[i + 1][j - 1].rgbtGreen,
                                    0, imageCopy[i][j].rgbtGreen, imageCopy[i][j - 1].rgbtGreen, 0,
                                    imageCopy[i - 1][j].rgbtGreen, imageCopy[i - 1][j - 1].rgbtGreen);
                            break;

                        case 2: 
                            bufferX = gxMatrix(0, imageCopy[i + 1][j].rgbtBlue, imageCopy[i + 1][j - 1].rgbtBlue,
                                    0, imageCopy[i][j].rgbtBlue, imageCopy[i][j - 1].rgbtBlue, 0,
                                    imageCopy[i - 1][j].rgbtBlue, imageCopy[i - 1][j - 1].rgbtBlue);
                            bufferY = gyMatrix(0, imageCopy[i + 1][j].rgbtBlue, imageCopy[i + 1][j - 1].rgbtBlue,
                                    0, imageCopy[i][j].rgbtBlue, imageCopy[i][j - 1].rgbtBlue, 0,
                                    imageCopy[i - 1][j].rgbtBlue, imageCopy[i - 1][j - 1].rgbtBlue);
                            break;
                    }
                    buffer = sqrt(bufferX + bufferY);
                    if (buffer > CAP) {
                        buffer = CAP;
                    }
                    buffer = round(buffer);
                    if (color == 0) {
                        image[i][j].rgbtRed = buffer;
                    } else if (color == 1) {
                        image[i][j].rgbtGreen = buffer;
                    } else {
                        image[i][j].rgbtBlue = buffer;
                    }
                }
            }
            else {
                for (int color = 0; color < 3; color++) {
                    switch (color) {
                        case 0: 
                            bufferX = gxMatrix(imageCopy[i + 1][j + 1].rgbtRed, imageCopy[i + 1][j].rgbtRed, imageCopy[i + 1][j - 1].rgbtRed,
                                    imageCopy[i][j + 1].rgbtRed, imageCopy[i][j].rgbtRed, imageCopy[i][j - 1].rgbtRed, imageCopy[i - 1][j + 1].rgbtRed,
                                    imageCopy[i - 1][j].rgbtRed, imageCopy[i - 1][j - 1].rgbtRed);
                            bufferY = gyMatrix(imageCopy[i + 1][j + 1].rgbtRed, imageCopy[i + 1][j].rgbtRed, imageCopy[i + 1][j - 1].rgbtRed,
                                    imageCopy[i][j + 1].rgbtRed, imageCopy[i][j].rgbtRed, imageCopy[i][j - 1].rgbtRed, imageCopy[i - 1][j + 1].rgbtRed,
                                    imageCopy[i - 1][j].rgbtRed, imageCopy[i - 1][j - 1].rgbtRed);
                            break;

                        case 1: 
                            bufferX = gxMatrix(imageCopy[i + 1][j + 1].rgbtGreen, imageCopy[i + 1][j].rgbtGreen, imageCopy[i + 1][j - 1].rgbtGreen,
                                    imageCopy[i][j + 1].rgbtGreen, imageCopy[i][j].rgbtGreen, imageCopy[i][j - 1].rgbtGreen, imageCopy[i - 1][j + 1].rgbtGreen,
                                    imageCopy[i - 1][j].rgbtGreen, imageCopy[i - 1][j - 1].rgbtGreen);
                            bufferY = gyMatrix(imageCopy[i + 1][j + 1].rgbtGreen, imageCopy[i + 1][j].rgbtGreen, imageCopy[i + 1][j - 1].rgbtGreen,
                                    imageCopy[i][j + 1].rgbtGreen, imageCopy[i][j].rgbtGreen, imageCopy[i][j - 1].rgbtGreen, imageCopy[i - 1][j + 1].rgbtGreen,
                                    imageCopy[i - 1][j].rgbtGreen, imageCopy[i - 1][j - 1].rgbtGreen);
                            break;

                        case 2: 
                            bufferX = gxMatrix(imageCopy[i + 1][j + 1].rgbtBlue, imageCopy[i + 1][j].rgbtBlue, imageCopy[i + 1][j - 1].rgbtBlue,
                                    imageCopy[i][j + 1].rgbtBlue, imageCopy[i][j].rgbtBlue, imageCopy[i][j - 1].rgbtBlue, imageCopy[i - 1][j + 1].rgbtBlue,
                                    imageCopy[i - 1][j].rgbtBlue, imageCopy[i - 1][j - 1].rgbtBlue);
                            bufferY = gyMatrix(imageCopy[i + 1][j + 1].rgbtBlue, imageCopy[i + 1][j].rgbtBlue, imageCopy[i + 1][j - 1].rgbtBlue,
                                    imageCopy[i][j + 1].rgbtBlue, imageCopy[i][j].rgbtBlue, imageCopy[i][j - 1].rgbtBlue, imageCopy[i - 1][j + 1].rgbtBlue,
                                    imageCopy[i - 1][j].rgbtBlue, imageCopy[i - 1][j - 1].rgbtBlue);
                            break;
                    }
                    buffer = sqrt(bufferX + bufferY);
                    if (buffer > CAP) {
                        buffer = CAP;
                    }
                    buffer = round(buffer);
                    if (color == 0) {
                        image[i][j].rgbtRed = buffer;
                    } else if (color == 1) {
                        image[i][j].rgbtGreen = buffer;
                    } else {
                        image[i][j].rgbtBlue = buffer;
                    }
                }
            }
        }
    }
    return;
}

// edge functions
double gxMatrix(int topLeft, int top, int topRight, int middleLeft, int middle, int middleRight, int bottomLeft, int bottom, int bottomRight)
{
    double gx = (topLeft * -1 + top * 0 + topRight * 1 + middleLeft * -2 + middle * 0 + middleRight * 2 + bottomLeft * -1 + bottom * 0 + bottomRight * 1);
    gx = pow(gx, 2.0);
    return gx;
}

double gyMatrix(int topLeft, int top, int topRight, int middleLeft, int middle, int middleRight, int bottomLeft, int bottom, int bottomRight)
{
    double gy = (topLeft * -1 + top * -2 + topRight * -1 + middleLeft * 0 + middle * 0 + middleRight * 0 + bottomLeft * 1 + bottom * 2 + bottomRight * 1);
    gy = pow(gy, 2.0);
    return gy;
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

void edgeCase(int i, int j, int width, int height, RGBTRIPLE image[height][width])
{
    if (i == 0) {
        if (j == 0) {
            return;
        } else if (j == width - 1) {
            return;
        } else {
            return;
        }   
    } else if (i == height - 1) {   
        if (j == 0) {
            return;
        } else if (j == width - 1) {
            return;
        } else {
            return;
        }
    } else if (j == 0) {
        return;
    } else if (j == width - 1) {
        return;
    }
}