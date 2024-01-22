#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <string.h>

struct message {
    long mtype;
    char mtext[100];
};

int compare(const void *a, const void *b) {
    return (*(int *)b - *(int *)a);
}

int main(int argc, char *argv[]) {
    if (argc < 6) {
        fprintf(stderr, "Kullanım: %s <K> <Num_message> <infile1> ... <infileN> <outfile>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int K = atoi(argv[1]);
    int num_messages = atoi(argv[2]);
    char output_filename[100];
    strcpy(output_filename, argv[argc - 1]);


    int *k_values = (int *)malloc(num_messages * sizeof(int));
    if (k_values == NULL) {
        perror("Bellek tahsisi hatası");
        exit(EXIT_FAILURE);
    }

    key_t key = ftok(".", 'a');
    int msgid = msgget(key, 0666 | IPC_CREAT);

    if (msgid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    pid_t child_pid = fork();

    if (child_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0) {

        for (int i = 0; i < num_messages; ++i) {
            struct message msg;
            msg.mtype = 1;

            strcpy(msg.mtext, argv[i + 3]);

            FILE *file = fopen(msg.mtext, "w");
            if (file == NULL) {
                perror("Dosya açma hatası");
                exit(EXIT_FAILURE);
            }

            for (int j = 0; j < 1000; ++j) {
                int random_value = rand() % 1000 + 1;
                fprintf(file, "%d\n", random_value);
            }

            fclose(file);

            if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
        }

        exit(EXIT_SUCCESS);
    } else {

        wait(NULL);

        for (int i = 0; i < num_messages; ++i) {
            struct message received_msg;
            if (msgrcv(msgid, &received_msg, sizeof(received_msg.mtext), 1, 0) == -1) {
                perror("msgrcv");
                exit(EXIT_FAILURE);
            }

            FILE *file = fopen(received_msg.mtext, "r");
            if (file == NULL) {
                perror("Dosya açma hatası");
                exit(EXIT_FAILURE);
            }

            int values[1000];
            int count = 0;


            while (fscanf(file, "%d", &values[count]) != EOF && count < 1000) {
                count++;
            }

            fclose(file);

            qsort(values, count, sizeof(int), compare);

            int filtered_values[1000];
            int filtered_count = 0;

            for (int j = 0; j < count; ++j) {
                if (j == 0 || values[j] != values[j - 1]) {
                    filtered_values[filtered_count++] = values[j];
                }
            }

            k_values[i] = filtered_values[K - 1];

            file = fopen(received_msg.mtext, "w");
            if (file == NULL) {
                perror("Dosya açma hatası");
                exit(EXIT_FAILURE);
            }

            for (int j = 0; j < filtered_count; ++j) {
                fprintf(file, "%d\n", filtered_values[j]);
            }

            fclose(file);
        }

        qsort(k_values, num_messages, sizeof(int), compare);

        FILE *output_file = fopen(output_filename, "w");
        if (output_file == NULL) {
            perror("Dosya açma hatası");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < num_messages; ++i) {
            fprintf(output_file, "%d\n", k_values[i]);
        }

        fclose(output_file);

        free(k_values);

        if (msgctl(msgid, IPC_RMID, NULL) == -1) {
            perror("msgctl");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}