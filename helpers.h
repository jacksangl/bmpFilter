#include "bmp.h"

// Convert image to grayscale
void grayscale(int height, int width, RGBTRIPLE image[height][width]);

// Reflect image horizontally
void reflect(int height, int width, RGBTRIPLE image[height][width]);

// Detect edges
void edges(int height, int width, RGBTRIPLE image[height][width]);

// Blur image
void blur(int height, int width, RGBTRIPLE image[height][width]);

// Seam carving
int seamCarve(int height, int width, RGBTRIPLE image[height][width], int compressPercent);

// Helper functions for blur
void blurPixel(int i, int j, int height, int width, RGBTRIPLE original[height][width], RGBTRIPLE *result);

// Helper functions for edge detection
void edgePixel(int i, int j, int height, int width, RGBTRIPLE original[height][width], RGBTRIPLE *result);

// Seam carving helper functions
void findSeam(int height, int width, RGBTRIPLE image[height][width], int *seam);
void removeSeam(int height, int width, RGBTRIPLE image[height][width], int *seam);
double edgeEnergy(int i, int j, int height, int width, RGBTRIPLE image[height][width]);

// Matrix calculation functions for edge detection
double gxMatrix(int topLeft, int top, int topRight, int middleLeft, int middle, int middleRight, int bottomLeft, int bottom, int bottomRight);
double gyMatrix(int topLeft, int top, int topRight, int middleLeft, int middle, int middleRight, int bottomLeft, int bottom, int bottomRight);

// Color averaging functions
int averageRed9(int topLeft, int top, int topRight, int middleLeft, int middle, int middleRight, int bottomLeft, int bottom, int bottomRight);
int averageGreen9(int topLeft, int top, int topRight, int middleLeft, int middle, int middleRight, int bottomLeft, int bottom, int bottomRight);
int averageBlue9(int topLeft, int top, int topRight, int middleLeft, int middle, int middleRight, int bottomLeft, int bottom, int bottomRight);

// Color channel utility functions
int getColorChannel(RGBTRIPLE pixel, int channel);
void setColorChannel(RGBTRIPLE *pixel, int channel, int value);

// Utility functions
int min(int a, int b);

// Edge case handling function - DEPRECATED (no longer used)
void edgeCase(int i, int j, int width, int height, RGBTRIPLE image[height][width]);
