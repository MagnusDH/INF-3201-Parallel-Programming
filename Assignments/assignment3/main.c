#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

double get_time_sec();

double get_time_sec() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(int argc, char *argv[]) {
    // Get file name from command line arguments
    if (argc != 2) {
        printf("Wrong usage\n");
        return EXIT_FAILURE;
    }
    const char* filename = argv[1];

    // Attempt to open the file
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Could not open file %s\n", filename);
        return EXIT_FAILURE;
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

    fread(array, sizeof(int32_t), n, file);
    fclose(file);

    // Start timing
    double start = get_time_sec();

    // Find the largest element
    int32_t max_value = array[0];
    for (size_t i = 1; i < n; i++) {
        if (array[i] > max_value){
            max_value = array[i];
        }
    }

    // End timing
    double end = get_time_sec();
    double time_spent = end - start;

    // Print largest element and time taken
    printf("Largest element: %d\n", max_value);
    printf("Time taken: %.8f seconds\n", time_spent);

    free(array);
    return 0;
}