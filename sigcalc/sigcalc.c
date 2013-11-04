//
// CA321 - OS Design and Implementation
// Assignment 1 - POSIX threads and signal handling
//
// @author Ian Duffy, 11356066
// @author Darren Brogan, 11424362
// @author Peter Morgan, 11314976
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
    // Accept SIGUSR1 and SIGUSR2 signals.
    sigset_t set;
    int sig;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);

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
            // When a SIGUSR1 is recieved read two numbers from the given file.
            if(sig == SIGUSR1)
                {
                    usleep(rand() % SLEEP);
                    fscanf(stream, "%d %d", &sharedData->a, &sharedData->b);

                    // When EOF is reached close the stream and end all the threads.
                    if(feof(stream))
                        {
                            fclose(stream);
                            // Tell the calculator thread to end.
                            pthread_kill(*sharedData->c, SIGUSR2);
                            sigwait(&set, &sig);
                            if(sig == SIGUSR2)
                                {
                                    printf("Goodbye from reader thread!\n");
                                    break;
                                }
                        }

                    printf("Thread 1 submitting : %d %d\n", sharedData->a, sharedData->b);
                    pthread_kill(*sharedData->c, SIGUSR1);
                }
        }

    return 0;

}


// Handler for the calculator thread.
static void * calculator(void *sharedData_in)
{
    // Accept SIGUSR1 and SIGUSR2 Signals.
    sigset_t set;
    int sig;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);

    // Cast the shared data back to type sharedData
    sharedData_t *sharedData = (sharedData_t *)sharedData_in;

    while(1)
        {
            sigwait(&set, &sig);
            // If a SIGUSR2 is recieved do a calculation.
	    if(sig == SIGUSR1)
                {
                    int calculation = sharedData->a + sharedData->b;
                    printf("Thread 2 calculated : %d\n", calculation);
                    usleep(rand() % SLEEP);
                    // Inform the reader that the calculation has been completed.
                    pthread_kill(*sharedData->r, SIGUSR1);
                }
            // If a SIGUSR2 is recieved end the thread.
            else if(sig == SIGUSR2)
                {
                    printf("Goodbye from calculator thread!\n");
                    // Inform the reader that the calculation thread has ended.
                    pthread_kill(*sharedData->r, SIGUSR2);
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
            fprintf(stderr, "Usage: ./sigcalc file\n");

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
