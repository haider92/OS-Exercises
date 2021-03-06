//
// CA321 - OS Design and Implementation
// Assignment 1 - POSIX threads and signal handling
//

#define _XOPEN_SOURCE 500
#define SLEEP 10000

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sysexits.h>

// Data structure for passing parameters to the threads.
typedef struct {
    int a, b;
    char* filename;
    pthread_t main;
} sharedData_t;

// Handler for the reader thread.
static void * reader(void *sharedData_in) {
    int sig = SIGUSR1;
    sigset_t set;
    sigaddset(&set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &set, NULL);

    // Cast the shared data back to type sharedData.
    sharedData_t *sharedData = (sharedData_t *)sharedData_in;

    // Open the file for reading.
    FILE *stream;
    stream = fopen(sharedData->filename, "r");
    if(stream == NULL) {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    while(!feof(stream)) {
        fscanf(stream, "%d %d", &sharedData->a, &sharedData->b);
        printf("Thread 1 submitting : %d %d\n", sharedData->a,sharedData->b);
        pthread_kill(sharedData->main, SIGUSR1);

        // Wait for main to tell to run.
        sigwait(&set, &sig);
        usleep(rand() % SLEEP);
    }
    fclose(stream);
    pthread_kill(sharedData->main, SIGUSR2);
    sigwait(&set, &sig);
    printf("Goodbye from Reader.\n");

    return ((void *)NULL);
}

// Handler for the calculator thread.
static void * calculator(void *sharedData_in) {
    int sig;
    sigset_t set;
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);
    sigprocmask(SIG_BLOCK, &set, NULL);

    // Cast the shared data back to type sharedData.
    sharedData_t *sharedData = (sharedData_t *)sharedData_in;

    while(sig != SIGUSR2) {
        if(sig == SIGUSR1) {
            int calculation = sharedData->a + sharedData->b;
            printf("Thread 2 calculated : %d\n", calculation);
            pthread_kill(sharedData->main, SIGUSR1);
        }
        // Wait for main to tell to run.
        sigwait(&set, &sig);
        usleep(rand() % SLEEP);
    }
    pthread_kill(sharedData->main, SIGUSR2);
    printf("Goodbye from Calculator.\n");
    return ((void *)NULL);
}

int main(int argc, char *argv[]) {
    pthread_t r, c;

    int sig;
    sigset_t set;
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);
    sigprocmask(SIG_BLOCK, &set, NULL);

    // Check that a valid command line argument was passed.
    if(argc != 2) {
        // Display an error to stderr, detailing valid usage.
        fprintf(stderr, "Usage: ./sigcalc file\n");

        // Exit returning the sysexits value for invalid command usage.
        exit(EX_USAGE);
    }

    sharedData_t sharedData;
    sharedData.main = pthread_self();
    sharedData.filename = argv[1];

    int calculateFlag = 0;

    pthread_create(&r, NULL, reader, (void *) &sharedData);
    pthread_create(&c, NULL, calculator, (void *) &sharedData);

    while(calculateFlag != -1) {
        sigwait(&set, &sig);
        if(sig == SIGUSR1) {
            if(calculateFlag == 1) {
                calculateFlag = 0;
                pthread_kill(c, SIGUSR1);
            } else if(calculateFlag == 0) {
                calculateFlag = 1;
                pthread_kill(r, SIGUSR1);
            }
        } else if(sig == SIGUSR2) {
            if(calculateFlag == 1) {
                calculateFlag = 0;
                pthread_kill(c, SIGUSR2);
            } else if(calculateFlag == 0) {
                calculateFlag = -1;
                pthread_kill(r, SIGUSR1);
            }
        }
    }

    pthread_join(r, NULL);
    pthread_join(c, NULL);

    return 0;
}
