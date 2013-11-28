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
#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>

typedef struct {
    int roomNumber;
    time_t wakeupTime;
} q_elem_t;

typedef struct {
    q_elem_t *buf;
    int size, expired, alloc;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} pri_queue_t;

pri_queue_t* priq_new(int size) {
    if (size < 4) size = 4;

    pri_queue_t* q = malloc(sizeof(pri_queue_t));
    q->buf = malloc(sizeof(q_elem_t) * size);
    q->alloc = size;

    q->size = 1;

    return q;
}

void priq_push(pri_queue_t* q, int roomNumber, time_t wakeupTime) {
    q_elem_t *b;
    int n, m;

    if (q->size >= q->alloc) {
        q->alloc *= 2;
        b = q->buf = realloc(q->buf, sizeof(q_elem_t) * q->alloc);
    } else
        b = q->buf;

    n = q->size++;

    while ((m = n / 2) && wakeupTime < b[m].wakeupTime) {
        b[n] = b[m];
        n = m;
    }

    b[n].roomNumber = roomNumber;
    b[n].wakeupTime = wakeupTime;
}

int priq_pop(pri_queue_t* q, time_t *wakeupTime) {
    int out;
    if (q->size == 1) return 0;

    q_elem_t *b = q->buf;

    out = b[1].roomNumber;
    *wakeupTime = b[1].wakeupTime;

    --q->size;

    int n = 1, m;
    while ((m = n * 2) < q->size) {
        if (m + 1 < q->size && b[m].wakeupTime > b[m + 1].wakeupTime) m++;

        if (b[q->size].wakeupTime <= b[m].wakeupTime) break;
        b[n] = b[m];
        n = m;
    }

    b[n] = b[q->size];
    if (q->size < q->alloc / 2 && q->size >= 16)
        q->buf = realloc(q->buf, (q->alloc /= 2) * sizeof(b[0]));

    return out;
}

int priq_top(pri_queue_t* q, time_t *wakeupTime) {
    if (q->size == 1) return 0;
    if (wakeupTime) *wakeupTime = q->buf[1].wakeupTime;
    return q->buf[1].roomNumber;
}

int priq_size(pri_queue_t *q) {
    return q->size - 1;
}

static void cleanup_handler(void *arg) {
    pthread_mutex_t* mutex = (pthread_mutex_t*) arg;
    printf("Called clean-up handler\n");
    pthread_mutex_unlock(mutex);
}


void * generator(void *q_in) {
    pri_queue_t * q = (pri_queue_t *) q_in;

    pthread_cleanup_push(cleanup_handler, &q->mutex);

    while(1) {
        pthread_mutex_lock(&q->mutex);

        int roomNumber = rand() % 5000;

        time_t wakeupTime = (time(NULL) + rand() % 100);
        struct tm * timeinfo = localtime( &wakeupTime );
        printf("Registering: \t%d %s\n", roomNumber, asctime(timeinfo));

        priq_push(q, roomNumber, wakeupTime);

        pthread_cond_signal(&q->cond);

        struct timespec timeout;
        timeout.tv_sec = time(0) + (rand() % 5);
        pthread_cond_timedwait(&q->cond, &q->mutex, &timeout);

        pthread_mutex_unlock(&q->mutex);
    }

    pthread_cleanup_pop(1);
    return ((void *)NULL);
}

static void * reader(void *q_in) {
    pri_queue_t * q = (pri_queue_t *) q_in;
    time_t p;

    pthread_cleanup_push(cleanup_handler, &q->mutex);

    while(1) {
        pthread_mutex_lock(&q->mutex);

        while(priq_size(q) == 0) {
            pthread_cond_wait(&q->cond, &q->mutex);
        }

        while(priq_top(q, &p) && p == time(0)) {
            q->expired = q->expired + 1;
            int roomNumber = priq_pop(q, &p);
            struct tm * timeinfo = localtime( &p );
            printf("Wake up: \t%d %s\n", roomNumber, asctime(timeinfo));
            printf("Expired alarms:\t%d\n", q->expired);
            printf("Pending alarms:\t%d\n\n", priq_size(q));
        }

        struct timespec timeout;
        timeout.tv_sec = (long) p;
        pthread_cond_timedwait(&q->cond, &q->mutex, &timeout);

        pthread_mutex_unlock(&q->mutex);
    }

    pthread_cleanup_pop(1);
    return ((void *)NULL);
}

int main() {
    pthread_t g, r;
    int sig;
    sigset_t set;
    sigaddset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, NULL);

    pri_queue_t q = *priq_new(0);
    pthread_mutex_init(&q.mutex, NULL);
    pthread_cond_init(&q.cond, NULL);

    pthread_create(&g, NULL, generator, (void *) &q);
    pthread_create(&r, NULL, reader, (void *) &q);

    sigwait(&set, &sig);
    pthread_cancel(g);
    pthread_cancel(r);
    pthread_join(g, NULL);
    pthread_join(r, NULL);
    printf("ending...");
    pthread_mutex_lock(&q.mutex);

    return 0;
}
