#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure to represent a pixel (RGB values)
typedef struct {
    unsigned char r, g, b;
} Pixel;

// Structure to represent an image
typedef struct {
    int width;
    int height;
    int max_color;
    Pixel **pixels;  // 2D array of pixels
} Image;

// Function to allocate memory for image pixels
Pixel** allocate_pixels(int width, int height) {
    Pixel **pixels = (Pixel**)malloc(height * sizeof(Pixel*));
    if (pixels == NULL) {   
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }
    
    for (int i = 0; i < height; i++) {
        pixels[i] = (Pixel*)malloc(width * sizeof(Pixel));
        if (pixels[i] == NULL) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            // Free previously allocated memory
            for (int j = 0; j < i; j++) {
                free(pixels[j]);
            }
            free(pixels);
            return NULL;
        }
    }
    return pixels;
}

// Function to free memory allocated for image pixels
void free_pixels(Pixel **pixels, int height) {
    if (pixels != NULL) {
        for (int i = 0; i < height; i++) {
            free(pixels[i]);
        }
        free(pixels);
    }
}

// Function to read a PPM file
Image* read_ppm(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return NULL;
    }
    
    char magic[3];
    if (fscanf(file, "%2s", magic) != 1 || strcmp(magic, "P6") != 0) {
        fprintf(stderr, "Error: Not a valid P6 PPM file\n");
        fclose(file);
        return NULL;
    }
    
    Image *img = (Image*)malloc(sizeof(Image));
    if (img == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        return NULL;
    }
    
    // Skip comments
    int c = fgetc(file);
    while (c == '#') {
        while (fgetc(file) != '\n');
        c = fgetc(file);
    }
    ungetc(c, file);
    
    // Read width, height, and max color value
    if (fscanf(file, "%d %d %d", &img->width, &img->height, &img->max_color) != 3) {
        fprintf(stderr, "Error: Invalid PPM header\n");
        free(img);
        fclose(file);
        return NULL;
    }
    
    // Skip whitespace after max_color
    fgetc(file);
    
    // Allocate memory for pixels
    img->pixels = allocate_pixels(img->width, img->height);
    if (img->pixels == NULL) {
        free(img);
        fclose(file);
        return NULL;
    }
    
    // Read pixel data
    for (int i = 0; i < img->height; i++) {
        for (int j = 0; j < img->width; j++) {
            img->pixels[i][j].r = fgetc(file);
            img->pixels[i][j].g = fgetc(file);
            img->pixels[i][j].b = fgetc(file);
        }
    }
    
    fclose(file);
    return img;
}

// Function to write a PPM file
int write_ppm(const char *filename, Image *img) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot create file %s\n", filename);
        return 0;
    }
    
    // Write PPM header
    fprintf(file, "P6\n%d %d\n%d\n", img->width, img->height, img->max_color);
    
    // Write pixel data
    for (int i = 0; i < img->height; i++) {
        for (int j = 0; j < img->width; j++) {
            fputc(img->pixels[i][j].r, file);
            fputc(img->pixels[i][j].g, file);
            fputc(img->pixels[i][j].b, file);
        }
    }
    
    fclose(file);
    return 1;
}

// Function to apply grayscale filter
void apply_grayscale(Image *img) {
    for (int i = 0; i < img->height; i++) {
        for (int j = 0; j < img->width; j++) {
            unsigned char gray = (img->pixels[i][j].r + 
                                 img->pixels[i][j].g + 
                                 img->pixels[i][j].b) / 3;
            img->pixels[i][j].r = gray;
            img->pixels[i][j].g = gray;
            img->pixels[i][j].b = gray;
        }
    }
}

// Function to apply invert filter
void apply_invert(Image *img) {
    for (int i = 0; i < img->height; i++) {
        for (int j = 0; j < img->width; j++) {
            img->pixels[i][j].r = 255 - img->pixels[i][j].r;
            img->pixels[i][j].g = 255 - img->pixels[i][j].g;
            img->pixels[i][j].b = 255 - img->pixels[i][j].b;
        }
    }
}

// Function to flip image horizontally
void flip_horizontal(Image *img) {
    for (int i = 0; i < img->height; i++) {
        for (int j = 0; j < img->width / 2; j++) {
            Pixel temp = img->pixels[i][j];
            img->pixels[i][j] = img->pixels[i][img->width - 1 - j];
            img->pixels[i][img->width - 1 - j] = temp;
        }
    }
}

// Function to flip image vertically
void flip_vertical(Image *img) {
    for (int i = 0; i < img->height / 2; i++) {
        for (int j = 0; j < img->width; j++) {
            Pixel temp = img->pixels[i][j];
            img->pixels[i][j] = img->pixels[img->height - 1 - i][j];
            img->pixels[img->height - 1 - i][j] = temp;
        }
    }
}

// Function to rotate image 90 degrees clockwise
Image* rotate_90_clockwise(Image *img) {
    Image *rotated = (Image*)malloc(sizeof(Image));
    if (rotated == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }
    
    // Swap width and height for 90-degree rotation
    rotated->width = img->height;
    rotated->height = img->width;
    rotated->max_color = img->max_color;
    
    rotated->pixels = allocate_pixels(rotated->width, rotated->height);
    if (rotated->pixels == NULL) {
        free(rotated);
        return NULL;
    }
    
    // Rotate pixels
    for (int i = 0; i < img->height; i++) {
        for (int j = 0; j < img->width; j++) {
            rotated->pixels[j][rotated->width - 1 - i] = img->pixels[i][j];
        }
    }
    
    return rotated;
}

// Function to rotate image 90 degrees counter-clockwise
Image* rotate_90_counterclockwise(Image *img) {
    Image *rotated = (Image*)malloc(sizeof(Image));
    if (rotated == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }
    
    // Swap width and height for 90-degree rotation
    rotated->width = img->height;
    rotated->height = img->width;
    rotated->max_color = img->max_color;
    
    rotated->pixels = allocate_pixels(rotated->width, rotated->height);
    if (rotated->pixels == NULL) {
        free(rotated);
        return NULL;
    }
    
    // Rotate pixels
    for (int i = 0; i < img->height; i++) {
        for (int j = 0; j < img->width; j++) {
            rotated->pixels[rotated->height - 1 - j][i] = img->pixels[i][j];
        }
    }
    
    return rotated;
}

// Function to rotate image 180 degrees
void rotate_180(Image *img) {
    // Rotate 180 degrees by flipping both horizontally and vertically
    flip_horizontal(img);
    flip_vertical(img);
}

// Function to free image memory
void free_image(Image *img) {
    if (img != NULL) {
        free_pixels(img->pixels, img->height);
        free(img);
    }
}

// Function to print usage information
void print_usage(const char *program_name) {
    printf("Usage: %s <input.ppm> <output.ppm> <filter>\n", program_name);
    printf("\nFilters:\n");
    printf("  grayscale  - Convert image to grayscale\n");
    printf("  invert     - Invert image colors\n");
    printf("  flip-h     - Flip image horizontally\n");
    printf("  flip-v     - Flip image vertically\n");
    printf("  rotate-90  - Rotate image 90 degrees clockwise\n");
    printf("  rotate-90ccw - Rotate image 90 degrees counter-clockwise\n");
    printf("  rotate-180 - Rotate image 180 degrees\n");
    printf("\nExample: %s input.ppm output.ppm grayscale\n", program_name);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char *input_file = argv[1];
    const char *output_file = argv[2];
    const char *filter = argv[3];
    
    // Read the input image
    Image *img = read_ppm(input_file);
    if (img == NULL) {
        return 1;
    }
    
    // Apply the requested filter
    if (strcmp(filter, "grayscale") == 0) {
        apply_grayscale(img);
    } else if (strcmp(filter, "invert") == 0) {
        apply_invert(img);
    } else if (strcmp(filter, "flip-h") == 0) {
        flip_horizontal(img);
    } else if (strcmp(filter, "flip-v") == 0) {
        flip_vertical(img);
    } else if (strcmp(filter, "rotate-90") == 0) {
        Image *rotated = rotate_90_clockwise(img);
        if (rotated == NULL) {
            free_image(img);
            return 1;
        }
        free_image(img);
        img = rotated;
    } else if (strcmp(filter, "rotate-90ccw") == 0) {
        Image *rotated = rotate_90_counterclockwise(img);
        if (rotated == NULL) {
            free_image(img);
            return 1;
        }
        free_image(img);
        img = rotated;
    } else if (strcmp(filter, "rotate-180") == 0) {
        rotate_180(img);
    } else {
        fprintf(stderr, "Error: Unknown filter '%s'\n", filter);
        print_usage(argv[0]);
        free_image(img);
        return 1;
    }
    
    // Write the output image
    if (write_ppm(output_file, img)) {
        printf("Successfully applied '%s' filter and saved to %s\n", filter, output_file);
    } else {
        free_image(img);
        return 1;
    }
    
    // Free memory
    free_image(img);
    
    return 0;
}

