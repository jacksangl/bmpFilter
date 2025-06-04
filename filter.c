#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "helpers.h"

int main(int argc, char *argv[])
{
    // Define allowable filters (s: means s takes an argument)
    char *filters = "begrs:";
    int compressPercent = 0;
    int seamCarving = 0;
    char filter = 0;

    // Get filter flag and check validity
    int opt;
    while ((opt = getopt(argc, argv, filters)) != -1) {
        switch (opt) {
            case 'b':
            case 'e':
            case 'g':
            case 'r':
                filter = opt;
                break;
            case 's':
                filter = opt;
                seamCarving = 1;
                compressPercent = atoi(optarg);  // optarg contains the argument after -s
                if (compressPercent < 1 || compressPercent > 99) {
                    printf("Compression percentage must be between 1 and 99.\n");
                    return 8;
                }
                break;
            case '?':
                printf("Invalid filter.\n");
                return 1;
        }
    }

    // Check if a filter was selected
    if (filter == 0) {
        printf("Must specify a filter.\n");
        return 1;
    }

    // Ensure proper usage
    if (seamCarving) {
        // For seam carving: ./filter -s 50 infile outfile
        if (argc != optind + 2) {
            printf("Usage for seam carving: ./filter -s percentage infile outfile\n");
            return 3;
        }
    } else {
        // For other filters: ./filter -flag infile outfile
        if (argc != optind + 2) {
            printf("Usage: ./filter [flag] infile outfile\n");
            printf("Usage for seam carving: ./filter -s percentage infile outfile\n");
            return 3;
        }
    }

    // Remember filenames
    char *infile = argv[optind];
    char *outfile = argv[optind + 1];

    // Open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        printf("Could not open %s.\n", infile);
        return 4;
    }

    // Open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        printf("Could not create %s.\n", outfile);
        return 5;
    }

    // Read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // Read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // Ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        printf("Unsupported file format.\n");
        return 6;
    }

    // Get image's dimensions
    int height = abs(bi.biHeight);
    int width = bi.biWidth;

    // Allocate memory for image
    RGBTRIPLE(*image)[width] = calloc(height, width * sizeof(RGBTRIPLE));
    if (image == NULL)
    {
        printf("Not enough memory to store image.\n");
        fclose(outptr);
        fclose(inptr);
        return 7;
    }

    // Determine padding for scanlines
    int padding = (4 - (width * sizeof(RGBTRIPLE)) % 4) % 4;

    // Iterate over infile's scanlines
    for (int i = 0; i < height; i++)
    {
        // Read row into pixel array
        fread(image[i], sizeof(RGBTRIPLE), width, inptr);

        // Skip over padding
        fseek(inptr, padding, SEEK_CUR);
    }

    // Filter image
    switch (filter)
    {
        // Blur
        case 'b':
            blur(height, width, image);
            break;

        // Edges
        case 'e':
            edges(height, width, image);
            break;

        // Grayscale
        case 'g':
            grayscale(height, width, image);
            break;

        // Reflect
        case 'r':
            reflect(height, width, image);
            break;
            
        // Seam carving
        case 's':
            seamCarve(height, width, image, compressPercent);
            break;
    }

    // Write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // Write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // Write new pixels to outfile
    for (int i = 0; i < height; i++)
    {
        // Write row to outfile
        fwrite(image[i], sizeof(RGBTRIPLE), width, outptr);

        // Write padding at end of row
        for (int k = 0; k < padding; k++)
        {
            fputc(0x00, outptr);
        }
    }

    // Free memory for image
    free(image);

    // Close files
    fclose(inptr);
    fclose(outptr);
    return 0;
} 