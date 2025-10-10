#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

int main(int argc, char *argv[]) {
    // Get file name from command line arguments
    if (argc != 2) {
        printf("Wrong usage\n");
        return 1;
    }
    const char* filename = argv[1];

    // Attempt to open the file
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Could not open file %s\n", filename);
        return 1;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file);

    // Compute number of elements
    size_t n = filesize / sizeof(int32_t);
    printf("Loading %zu elements (%.2f GiB)\n", n, filesize / (1024.0 * 1024.0 * 1024.0));

    // Allocate memory and read data
    int32_t *array = malloc(filesize);
    if (!array) {
        perror("Memory allocation failed");
        fclose(file);
        return 1;
    }

    // Parallel CUDA implementation here

    // Print largest element and time taken

    free(array);
    return 0;
}