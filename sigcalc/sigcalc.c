//
// CA321 - OS Design and Implementation
// Assignment 1 - POSIX threads and signal handling
//
// @author Ian Duffy
// @author Darren Brogan
// @author Peter Morgan
//
// This project is our own work. We have not recieved assistance beyond what is
// normal, and we have cited any sources from which we have borrowed. We have
// not given a copy of our work, or a part of our work, to anyone. We are aware
// that copying or giving a copy may have serious consequences.
//

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sysexits.h>

#define SLEEP 10000

// Data structure for passing parameters to the threads.
typedef struct
{
    int a, b;
    char* filename;
    pthread_t *r, *c;
} sharedData_t;

// Handler for the reader thread.
static void * reader(void *sharedData_in)
{
    // Accept SIGUSR1 signals.
    sigset_t set;
    int sig;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGINT);

    // Cast the shared data back to type sharedData.
    sharedData_t *sharedData = (sharedData_t *)sharedData_in;

    // Open the file for reading.
    FILE *stream;
    stream = fopen(sharedData->filename, "r");
    if(stream == NULL)
        {
            perror("Error");
            exit(EXIT_FAILURE);
        }

    while(1)
        {
            sigwait(&set, &sig);
            if(sig == SIGUSR1)
                {
                    usleep(rand() % SLEEP);
                    fscanf(stream, "%d %d", &sharedData->a, &sharedData->b);

                    if(feof(stream))
                        {
                            fclose(stream);
                            pthread_kill(*sharedData->c, SIGINT);
                            sigwait(&set, &sig);
                            if(sig == SIGINT)
                                {
                                    printf("Goodbye from reader thread!\n");
                                    break;
                                }
                        }

                    printf("Thread 1 submitting : %d %d\n", sharedData->a, sharedData->b);
                    pthread_kill(*sharedData->c, SIGUSR2);
                }
        }

    return 0;

}


// Handler for the calculator thread.
static void * calculator(void *sharedData_in)
{
    // Accept SIGUSR2 Signals.
    sigset_t set;
    int sig;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR2);
    sigaddset(&set, SIGINT);

    // Cast the shared data back to type sharedData
    sharedData_t *sharedData = (sharedData_t *)sharedData_in;

    while(1)
        {
            sigwait(&set, &sig);
            if(sig == SIGUSR2)
                {
                    int calculation = sharedData->a + sharedData->b;
                    printf("Thread 2 calculated : %d %d\n", calculation, sig);
                    usleep(rand() % SLEEP);
                    pthread_kill(*sharedData->r, SIGUSR1);
                }
            else if(sig == SIGINT)
                {
                    printf("Goodbye from calculator thread!\n");
                    pthread_kill(*sharedData->r, SIGINT);
                    break;
                }
        }

    return 0;
}

int main(int argc, char *argv[])
{
    // Block Signals.
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);
    sigprocmask(SIG_BLOCK, &set, NULL);

    // Check that a valid command line argument was passed.
    if(argc != 2)
        {
            // Display an error to stderr, detailing valid usage.
            fputs("Usage: ./sigcalc file\n", stderr);

            // Exit returning the sysexits value for invalid command usage.
            exit(EX_USAGE);
        }

    pthread_t r, c;

    // Create shared data.
    sharedData_t sharedData;
    sharedData.filename = argv[1];
    sharedData.r = &r;
    sharedData.c = &c;

    // Create the threads.
    pthread_create(&r, NULL, reader, (void *) &sharedData);
    pthread_create(&c, NULL, calculator, (void *) &sharedData);

    // Kick of the reading thread by sending SIGUSR1.
    pthread_kill(r, SIGUSR1);

    // Wait for the threads to finish.
    pthread_join(r, NULL);
    pthread_join(c, NULL);

    return 0;
}
