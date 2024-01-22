#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_SIZE 10000

void deleteTemporaryFiles() {
    for (int i = 1; i <= MAX_SIZE; i++) {
        char file_name[20];
        sprintf(file_name, "intermediate%d.txt", i);
        remove(file_name);
    }

    printf("Temporary files deleted.\n");
}

int compare(const void* a, const void* b) {
    return (*(int*)b - *(int*)a);
}

void writeRandomNumbersToFile(const char* fileName, int count) {
    FILE* file = fopen(fileName, "w");
    if (file == NULL) {
        perror("File opening error");
        exit(EXIT_FAILURE);
    }

    int i, *random_numbers;
    random_numbers = (int *)malloc(count * sizeof(int));


    srand((unsigned int)getpid());

    for (i = 0; i < count; i++) {
        random_numbers[i] = rand() % 1000 + 1;
    }

    qsort(random_numbers, count, sizeof(int), compare);


    for (i = 0; i < count; i++) {
        if (i == 0 || random_numbers[i] != random_numbers[i - 1]) {
            fprintf(file, "%d\n", random_numbers[i]);
        }
    }

    free(random_numbers);
    fclose(file);
}

int findKthLargest(const char* fileName, int k) {
    FILE* file = fopen(fileName, "r");
    if (file == NULL) {
        perror("File opening error");
        exit(EXIT_FAILURE);
    }

    int number, count = 0;
    int lastNumber = -1, lastCount = 0;

    while (fscanf(file, "%d", &number) == 1) {
        if (number != lastNumber) {
            lastCount++;
        }

        if (lastCount == k) {
            fclose(file);
            return lastNumber;
        }

        lastNumber = number;
    }

    fclose(file);
    return -1;
}

void mergeAndSortResults(int numChildProcesses, int k) {
    int* results = (int*)malloc(numChildProcesses * sizeof(int));


    for (int i = 1; i <= numChildProcesses; i++) {
        char file_name[20];
        sprintf(file_name, "intermediate%d.txt", i);
        results[i-1] = findKthLargest(file_name, k);
    }


    qsort(results, numChildProcesses, sizeof(int), compare);


    FILE* output_file = fopen("output.txt", "w");
    if (output_file == NULL) {
        perror("Output file opening error");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < numChildProcesses; i++) {
        fprintf(output_file, "%d\n", results[i]);
    }

    fclose(output_file);
    free(results);
}

void processFile(int process_id, int k) {
    char file_name[20];
    sprintf(file_name, "intermediate%d.txt", process_id);
    writeRandomNumbersToFile(file_name, 1000);
}

void createChildProcesses(int numChildProcesses, int k) {
    for (int i = 1; i <= numChildProcesses; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("Fork error");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            processFile(i, k);
            exit(EXIT_SUCCESS);
        }
    }

    for (int i = 0; i < numChildProcesses; i++) {
        wait(NULL);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <K> <N>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int K = atoi(argv[1]);
    int N = atoi(argv[2]);

    if (N < 1 || N > 5) {
        fprintf(stderr, "Invalid number of child processes. Should be between 1 and 5.\n");
        exit(EXIT_FAILURE);
    }

    K++; 

    if (K < 1 || K > 1000) {
        fprintf(stderr, "Invalid value of K. Should be between 1 and 1000.\n");
        exit(EXIT_FAILURE);
    }

    createChildProcesses(N, K);

    mergeAndSortResults(N, K);

    deleteTemporaryFiles();

    return 0;
}