#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#define MAX_FILENAME_LENGTH 50
#define MAX_THREADS 5
#define NUM_RANDOM_NUMBERS 1000


char filenames[MAX_THREADS][MAX_FILENAME_LENGTH];

char mainOutputFileName[MAX_FILENAME_LENGTH];

int K;

pthread_mutex_t mutex;

int kthValues[MAX_THREADS];

int compare(const void *a, const void *b) {
    return (*(int *)b - *(int *)a);

void *threadFunction(void *arg);

int main(int argc, char *argv[]) {
    if (argc < 4 || argc > MAX_THREADS + 3) {
        printf("Usage: %s <K> <N> <infile1> ... <infileN> <outfile>\n", argv[0]);
        return 1;
    }

    K = atoi(argv[1]);
    int threadCount = atoi(argv[2]);

    if (threadCount < 1 || threadCount > MAX_THREADS || K < 1) {
        printf("Invalid input. K and N should be positive integers, and N should be between 1 and %d.\n", MAX_THREADS);
        return 1;
    }

    snprintf(mainOutputFileName, MAX_FILENAME_LENGTH, "%s", argv[argc - 1]);

    if (pthread_mutex_init(&mutex, NULL) != 0) {
        fprintf(stderr, "Mutex creation error\n");
        return 1;
    }
    pthread_t threads[MAX_THREADS];

    for (int i = 0; i < threadCount; ++i) {
        snprintf(filenames[i], MAX_FILENAME_LENGTH, "%s", argv[i + 3]);

        if (strstr(filenames[i], ".txt") == NULL) {
            strcat(filenames[i], ".txt");
        }

        if (pthread_create(&threads[i], NULL, threadFunction, (void *)(intptr_t)(i + 1)) != 0) {
            fprintf(stderr, "Thread creation error\n");
            return 1;
        }
    }

    for (int i = 0; i < threadCount; ++i) {
        pthread_join(threads[i], NULL);
    }

    qsort(kthValues, threadCount, sizeof(int), compare);

    FILE *mainOutputFile = fopen(mainOutputFileName, "w");
    if (mainOutputFile == NULL) {
        fprintf(stderr, "Error opening the main output file\n");
        return 1;
    }

    for (int i = 0; i < threadCount; ++i) {
        fprintf(mainOutputFile, "%d\n", kthValues[i]);
    }

    fclose(mainOutputFile);

    pthread_mutex_destroy(&mutex);

    printf("Program completed successfully.\n");

    return 0;
}

void *threadFunction(void *arg) {
    int threadID = (int)(intptr_t)arg;

    char filename[MAX_FILENAME_LENGTH];
    snprintf(filename, MAX_FILENAME_LENGTH, "%s", filenames[threadID - 1]);

    if (strstr(filename, ".txt") == NULL) {
        strcat(filename, ".txt");
    }

    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error creating the file\n");
        pthread_exit(NULL);
    }

    for (int i = 0; i < NUM_RANDOM_NUMBERS; ++i) {
        fprintf(file, "%d\n", rand() % 1000 + 1);
    }

    fclose(file);

    int numbers[NUM_RANDOM_NUMBERS];
    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening the file\n");
        pthread_exit(NULL);
    }

    for (int i = 0; i < NUM_RANDOM_NUMBERS; ++i) {
        fscanf(file, "%d", &numbers[i]);
    }

    fclose(file);

    qsort(numbers, NUM_RANDOM_NUMBERS, sizeof(int), compare);

    int uniqueNumbers[NUM_RANDOM_NUMBERS];
    int uniqueCount = 1;
    uniqueNumbers[0] = numbers[0];

    for (int i = 1; i < NUM_RANDOM_NUMBERS; ++i) {
        if (numbers[i] != numbers[i - 1]) {
            uniqueNumbers[uniqueCount] = numbers[i];
            uniqueCount++;
        }
    }

    file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening the file\n");
        pthread_exit(NULL);
    }

    for (int i = 0; i < uniqueCount; ++i) {
        fprintf(file, "%d\n", uniqueNumbers[i]);
    }

    fclose(file);

    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening the file\n");
        pthread_exit(NULL);
    }

    int kthValue;
    for (int i = 0; i < K; ++i) {
        if (fscanf(file, "%d", &kthValue) != 1) {
            fprintf(stderr, "Error reading Kth value from the file\n");
            fclose(file);
            pthread_exit(NULL);
        }
    }

    fclose(file);

    pthread_mutex_lock(&mutex);
    kthValues[threadID - 1] = kthValue;
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}