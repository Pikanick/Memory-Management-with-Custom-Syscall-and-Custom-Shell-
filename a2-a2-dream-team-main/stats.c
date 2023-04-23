#include "stats.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

typedef struct stats_t
{
    int candies_made;
    int candies_eaten;
    // in ms
    double total_delay, avg_delay, min_delay, max_delay;
} Stat; // alias name for typedef struct stats_t

Stat *statistics_buff;
// int STATS_BUFF_SIZE = 0;
int NUM_FACTORIES;
pthread_mutex_t mutex;

void stats_init(int num_producers)
{
    NUM_FACTORIES = num_producers;
    // 0 initialize everything
    statistics_buff = (Stat *)calloc(num_producers, sizeof(Stat));
    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        printf("\n mutex init failed\n");
    }
}

void stats_cleanup()
{
    free(statistics_buff);
    statistics_buff = NULL;
    if (pthread_mutex_destroy(&mutex) != 0)
    {
        printf("\n mutex destruction failed\n");
    }
}

void stats_record_produced(int factory_number)
{
    pthread_mutex_lock(&mutex);
    statistics_buff[factory_number].candies_made++;
    pthread_mutex_unlock(&mutex);
}

void stats_record_consumed(int factory_number, double delay_in_ms)
{
    pthread_mutex_lock(&mutex);
    // keep a running average, first multiply by old num to add new delay into sum
    statistics_buff[factory_number].avg_delay *= statistics_buff[factory_number].candies_eaten;
    statistics_buff[factory_number].avg_delay += delay_in_ms;
    statistics_buff[factory_number].candies_eaten++;
    statistics_buff[factory_number].avg_delay /= statistics_buff[factory_number].candies_eaten;

    statistics_buff[factory_number].total_delay += delay_in_ms;

    // before first candy eaten, min_delay is initialized to 0
    if (statistics_buff[factory_number].candies_eaten == 1 ||
        delay_in_ms < statistics_buff[factory_number].min_delay)
    {
        statistics_buff[factory_number].min_delay = delay_in_ms;
    }
    if (delay_in_ms > statistics_buff[factory_number].max_delay)
    {
        statistics_buff[factory_number].max_delay = delay_in_ms;
    }
    pthread_mutex_unlock(&mutex);
}

void stats_display()
{
    bool made_eaten_match = true;
    printf("%8s%10s%10s%20s%20s%20s\n", "Factory#", "#Made", "#Eaten", "Min Delay[ms]", "Avg Delay[ms]", "Max Delay[ms]"); // header
    for (int i = 0; i < NUM_FACTORIES; ++i)
    {
        printf("%8d%10d%10d%20.5f%20.5f%20.5f\n", i, statistics_buff[i].candies_made, statistics_buff[i].candies_eaten, statistics_buff[i].min_delay, statistics_buff[i].avg_delay, statistics_buff[i].max_delay); // rows
        if (statistics_buff[i].candies_made != statistics_buff[i].candies_eaten)
        {
            made_eaten_match = false;
        }
    }
    if (!made_eaten_match)
    {
        printf("ERROR: Mismatch between number made\n");
    }
}
