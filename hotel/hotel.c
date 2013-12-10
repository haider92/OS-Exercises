/**
* CA321 - OS Design and Implementation
* Assignment 2 - Hotel Wakeup call system
*
* @author Ian Duffy, 11356066
* @author Darren Brogan, 11424362
* @author Peter Morgan, 11314976
*
* This project is our own work. We have not recieved assistance beyond what is
* normal, and we have cited any sources from which we have borrowed. We have
* not given a copy of our work, or a part of our work, to anyone. We are aware
* that copying or giving a copy may have serious consequences.
**/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

typedef struct node {
    int room_number;
    long expiry_time;
    struct node *next;
} Node;

typedef struct {
    Node *head;
    int pending;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} shared_data_t;

Node *insert(Node *head, int room_number, long expiry_time) {
    Node *newNode = (Node*) malloc(sizeof(Node));
    Node *previousNode, *currentNode;
    newNode->room_number = room_number;
    newNode->expiry_time = expiry_time;

    /* If head is empty. */
    if (head == NULL) {
        newNode->next = NULL;
        return newNode;
    }

    /* If the new expiry_time is smaller. */
    if (expiry_time < head->expiry_time) {
        newNode->next = head;
        return newNode;
    }

    previousNode = head;
    currentNode = head->next;

    while (currentNode != NULL && expiry_time > currentNode->expiry_time) {
        previousNode = currentNode;
        currentNode = currentNode->next;
    }

    /* Insert the newNode between previousNode and currentNode. */
    newNode->next = previousNode->next;
    previousNode->next = newNode;

    return head;
}

Node *removeFirst(Node *head) {
    /* reference to the next node */
    Node *tempNode = head->next;

    /* Free the head */
    free(head);

    /* The second node now becomes head */
    return tempNode;
}

void cleanup_guest(void * data_in) {
    printf("The guest thread is cleaning up...\n");
    printf("The guest thread says goodbye.\n");
}

void cleanup_waiter(void * data_in) {
    shared_data_t * data = (shared_data_t *) data_in;
    printf("The waiter thread is cleaning up...\n");

    /* Free up all nodes */
    while(data->head != NULL) {
        data->head = removeFirst(data->head);
        data->pending--;
    }

    /* Unlock the mutex */
    pthread_mutex_unlock(&data->mutex);
    printf("The waiter thread says goodbye\n");
}

void * guest(void *data_in) {
    shared_data_t *data = (shared_data_t *) data_in;
    int room_number;
    long expiry_time;
    unsigned int seed = time(NULL);

    /* Assign the cleanup handler */
    pthread_cleanup_push(cleanup_guest, (void *)NULL);

    while(1) {
        pthread_mutex_lock(&data->mutex);

        /* Log the new wakeup call and print it out */
        room_number = 1 + (rand_r(&seed) % 5000);
        expiry_time = time(NULL) + (rand_r(&seed) % 100);
        data->pending++;
        printf("Registering:\t%d %s\n", room_number, ctime(&expiry_time));
        data->head = insert(data->head, room_number, expiry_time);

        /* If the item at the top of the linkedlist is the newly added item
           Inform the waiter of its existence */
        if(expiry_time == data->head->expiry_time) {
            pthread_cond_signal(&data->cond);
        }

        pthread_mutex_unlock(&data->mutex);

        /* Sleep for a random amount of seconds under and including 5
           Cancelation point */
        sleep(rand_r(&seed) % 5);
    }

    /* Run the cleanup handler */
    pthread_cleanup_pop(0);

    return ((void *)NULL);
}

void * waiter(void *data_in) {
    shared_data_t *data = (shared_data_t *) data_in;
    int expired = 0, error;
    struct timespec timeout;

    /* Assign the cleanup handler */
    pthread_cleanup_push(cleanup_waiter, (void *)data);

    while(1) {
        pthread_mutex_lock(&data->mutex);

        /* Wait for data to be contained within the linkedlist */
        while(data->head == NULL) {
            /* Wait + Cancelation point */
            pthread_cond_wait(&data->cond, &data->mutex);
        }

        /* Setup an expiry time for the item at the top of the linkedlist */
        timeout.tv_sec = data->head->expiry_time;
        timeout.tv_nsec = 0;

        /* Wait + Cancelation point */
        error = pthread_cond_timedwait(&data->cond,
                                           &data->mutex, &timeout);

        /* Print out the alarm information */
        if(error == ETIMEDOUT) {
            expired++;
            data->pending--;
            printf("Wake up:\t%d %s\n", data->head->room_number,
                   ctime(&data->head->expiry_time));
            printf("Expired alarms:\t%d\n", expired);
            printf("Pending alarms:\t%d\n\n", data->pending);
            data->head = removeFirst(data->head);
        }

        pthread_mutex_unlock(&data->mutex);
    }

    /* Run the cleanup handler */
    pthread_cleanup_pop(0);

    return ((void *)NULL);
}


int main() {
    int sig;
    sigset_t set;
    pthread_t g, w;
    shared_data_t data;

    /* Setup the shared data */
    data.head = NULL;
    data.pending = 0;
    pthread_mutex_init(&data.mutex, NULL);
    pthread_cond_init(&data.cond, NULL);

    /* Start the threads with data passed in */
    pthread_create(&g, NULL, guest, (void *) &data);
    pthread_create(&w, NULL, waiter, (void *) &data);

    /* Listen for a SIGINT */
    sigaddset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, NULL);
    sigwait(&set, &sig);

    /* Cancel the threads */
    pthread_cancel(g);
    pthread_cancel(w);

    /* Wait for them to end */
    pthread_join(g, NULL);
    pthread_join(w, NULL);

    /* Destroy the mutex */
    pthread_mutex_destroy(&data.mutex);
    /* Show that all pending requests have been freed */
    printf("Pending: %d\n", data.pending);
    return 0;
}
