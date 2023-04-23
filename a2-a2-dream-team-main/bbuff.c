#include "bbuff.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <semaphore.h>
#include <pthread.h>

#define BUFFER_SIZE 10

void *candies_buff[BUFFER_SIZE];
int pos_in, pos_out;
sem_t full, empty;
pthread_mutex_t crit_lock;

void bbuff_init(void)
{
    pos_in = 0;
    pos_out = 0;
    // semaphores to keep track if buffer is full or slots available
    // no slots available
    sem_init(&full, 0, 0);
    // all slots available
    sem_init(&empty, 0, BUFFER_SIZE);
    if (pthread_mutex_init(&crit_lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
    }
}

_Bool bbuff_is_empty(void)
{
    int n_empty, n_full;
    sem_getvalue(&empty, &n_empty);
    sem_getvalue(&full, &n_full);
    if (n_full == 0 && n_empty == BUFFER_SIZE)
        return true;
    else
        return false;
}

void bbuff_blocking_insert(void *item)
{
    // decrement semaphore; if semaphore = 0 then there are no available slots and we should wait until one is free.
    sem_wait(&empty);
    pthread_mutex_lock(&crit_lock);

    // critical section should go here
    // what should BUFFER_SIZE be? its specified in the doc to be 10
    candies_buff[pos_in] = item;
    // already waited until >= 1 empty slot
    // just go in a circle through and through array again and again
    pos_in = (pos_in + 1) % BUFFER_SIZE;

    pthread_mutex_unlock(&crit_lock);
    sem_post(&full); // increment semaphore to reflect inserted element
}

void *bbuff_blocking_extract(void)
{
    // wait until at least one element in buffer to consume. if 0 then wait until semaphore has something
    sem_wait(&full);
    pthread_mutex_lock(&crit_lock);

    void *item;
    item = candies_buff[pos_out];
    // already waited until >= 1 filled slot
    // just go in a circle through and through array again and again
    pos_out = (pos_out + 1) % BUFFER_SIZE;

    pthread_mutex_unlock(&crit_lock);
    sem_post(&empty);

    return item;
}
