//
// CA321 - OS Design and Implementation
// Assignment 2 - Hotel Wakeup call system
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
    newNode->room_number = room_number;
    newNode->expiry_time = expiry_time;

    // If head is empty.
    if (head == NULL) {
        newNode->next = NULL;
        return newNode;
    }

    // If the new expiry_time is smaller.
    if (expiry_time < head->expiry_time) {
        newNode->next = head;
        return newNode;
    }

    Node *previousNode = head;
    Node *currentNode = head->next;

    while (currentNode != NULL && expiry_time > currentNode->expiry_time) {
        previousNode = currentNode;
        currentNode = currentNode->next;
    }

    // Insert the newNode between previousNode and currentNode.
    newNode->next = previousNode->next;
    previousNode->next = newNode;

    return head;
}

Node *removeFirst(Node *head) {
    // reference to the next node
    Node *tempNode = head->next;

    // Free the head
    free(head);

    // The second node now becomes head
    return tempNode;
}

void cleanup_guest(void * data_in) {
    printf("The guest thread is cleaning up...\n");
    printf("The guest thread says goodbye.\n");
}

void cleanup_waiter(void * data_in) {
    shared_data_t * data = (shared_data_t *) data_in;
    printf("The waiter thread is cleaning up...\n");
    while(data->head != NULL) {
        data->head = removeFirst(data->head);
        data->pending--;
    }
    printf("The waiter thread says goodbye\n");
}

void * guest(void *data_in) {
    shared_data_t *data = (shared_data_t *) data_in;

    pthread_cleanup_push(cleanup_guest, (void *)NULL);

    while(1) {
        pthread_mutex_lock(&data->mutex);

        int room_number = 1 + (rand() % 5000);
        long expiry_time = time(NULL) + (rand() % 100);

        data->pending++;

        printf("Registering:\t%d %s\n", room_number, ctime(&expiry_time));
        data->head = insert(data->head, room_number, expiry_time);

        if(expiry_time == data->head->expiry_time) {
            pthread_cond_signal(&data->cond);
        }
        pthread_mutex_unlock(&data->mutex);

        sleep(rand() % 5);
    }

    pthread_cleanup_pop(0);

    return ((void *)NULL);
}

void * waiter(void *data_in) {
    shared_data_t *data = (shared_data_t *) data_in;
    int error, expired = 0;
    struct timespec timeout;

    pthread_cleanup_push(cleanup_waiter, (void *)data);

    while(1) {
        pthread_mutex_lock(&data->mutex);

        while(data->head == NULL) {
            pthread_cond_wait(&data->cond, &data->mutex);
        }

        timeout.tv_sec = data->head->expiry_time;
        timeout.tv_nsec = 0;
        error = pthread_cond_timedwait(&data->cond, &data->mutex, &timeout);

        if(error == ETIMEDOUT) {
            expired++;
            data->pending--;
            printf("Wake up:\t%d %s\n", data->head->room_number, ctime(&data->head->expiry_time));
            printf("Expired alarms:\t%d\n", expired);
            printf("Pending alarms:\t%d\n\n", data->pending);
            data->head = removeFirst(data->head);
        }

        pthread_mutex_unlock(&data->mutex);
    }

    pthread_cleanup_pop(0);

    return ((void *)NULL);
}


int main() {

    shared_data_t data;
    data.head = NULL;
    data.pending = 0;

    pthread_mutex_init(&data.mutex, NULL);
    pthread_cond_init(&data.cond, NULL);

    pthread_t g, w;

    pthread_create(&g, NULL, guest, (void *) &data);
    pthread_create(&w, NULL, waiter, (void *) &data);

    sigset_t set;
    sigaddset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, NULL);

    sigwait(&set, NULL);
    pthread_cancel(g);
    pthread_cancel(w);
    pthread_join(g, NULL);
    pthread_join(w, NULL);

    pthread_mutex_destroy(&data.mutex);
    printf("Pending: %d\n", data.pending);
    return 0;
}
