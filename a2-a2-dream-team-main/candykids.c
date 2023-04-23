#include "bbuff.h"
#include "stats.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/sem.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

const int NUM_ARGS = 3;
bool factories_on = true;

typedef struct
{
    int factory_number;
    double creation_ts_ms;
} candy_t;

void tokenize_args(char *raws_arr[], int *final_arr)
{
    for (int i = 1; i <= NUM_ARGS; i++)
    {
        if (atoi(raws_arr[i]) < 0)
        {
            printf("Error: Please enter an integer greater than 0 \n");
            exit(1);
        }
        final_arr[i - 1] = atoi(raws_arr[i]);
    }
}

double current_time_in_ms(void)
{
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return now.tv_sec * 1000.0 + now.tv_nsec / 1000000.0;
}

void *producer(void *arg)
{
    int index = *((int *)arg);
    while (factories_on)
    {
        int wait = rand() % 4;
        printf("\tFactory %d ships candy & waits %ds\n", index, wait);
        candy_t *candy = malloc(sizeof(candy_t));
        candy->factory_number = index;
        candy->creation_ts_ms = current_time_in_ms();
        bbuff_blocking_insert(candy);
        stats_record_produced(index);
        sleep(wait);
    }
    printf("Candy-factory %d done\n", index);
    pthread_exit(NULL);
}

void *consumer(void *arg)
{
    while (true)
    {
        int wait = rand() % 2;
        candy_t *candy = bbuff_blocking_extract();
        // how long from candy made to kid ate
        double delay = current_time_in_ms() - candy->creation_ts_ms;
        stats_record_consumed(candy->factory_number, delay);
        free(candy);
        candy = NULL;
        sleep(wait);
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    // 1.  Extract arguments
    if (argc != NUM_ARGS + 1)
    {
        printf("Error: Please use the following format: ./candykids <#Factories>, <#Kids>, <#Seconds>\n");
        exit(1);
    }
    int args_processed[NUM_ARGS];
    tokenize_args(argv, args_processed);
    int TOT_FACTORIES = args_processed[0];
    int TOT_KIDS = args_processed[1];
    int MAX_TIME = args_processed[2]; // in seconds
    printf("%i factories, %i kids, %i seconds timeout\n", TOT_FACTORIES, TOT_KIDS, MAX_TIME);
    // 2.  Initialization of modules
    bbuff_init();
    stats_init(TOT_FACTORIES);
    // 3.  Launch candy-factory threads
    pthread_t factories[TOT_FACTORIES];
    int factory_indicies[TOT_FACTORIES];
    for (int i = 0; i < TOT_FACTORIES; i++)
    {
        factory_indicies[i] = i;
        if (pthread_create(&factories[i], NULL, &producer, &factory_indicies[i]) != 0)
        {
            perror("Failed to create thread");
        }
    }
    // 4.  Launch kid threads
    pthread_t kids[TOT_KIDS];
    for (int i = 0; i < TOT_KIDS; i++)
    {
        if (pthread_create(&kids[i], NULL, &consumer, &i) != 0)
        {
            perror("Failed to create thread");
        }
    }
    // 5.  Wait for requested time
    for (int i = 0; i < MAX_TIME; i++)
    {
        sleep(1);
        printf("Time %ds\n", i + 1);
    }
    // 6.  Stop candy-factory threads
    factories_on = false;
    for (int i = 0; i < TOT_FACTORIES; i++)
    {
        if (pthread_join(factories[i], NULL) != 0)
        {
            perror("Failed to join thread");
        }
    }
    // 7.  Wait until there is no more candies
    while (!bbuff_is_empty())
    {
        printf("Waiting for all candies to be consumed...\n");
        sleep(1);
    }
    // 8.  Stop kid threads
    for (int i = 0; i < TOT_KIDS; i++)
    {
        pthread_cancel(kids[i]);
        if (pthread_join(kids[i], NULL) != 0)
        {
            perror("Failed to join thread");
        }
    }
    // 9.  Print statistics
    stats_display();
    // 10. Cleanup any allocated memory
    stats_cleanup();
    return 0;
}