#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <semaphore.h>

#define ONE_SECOND 1000000
#define RANGE 10
#define PERIOD 2
#define NUM_THREADS 4

typedef struct
{
    int *park;
    int capacity;
    int occupied;
    int next_in;
    int next_out;
    int sum_in;
    int sum_out;
    int flag;
    int pa_f;
    pthread_mutex_t lock;
    pthread_cond_t space;
    pthread_cond_t car;
    pthread_barrier_t bar;
}Parking_Lot;

void * producer(void *park_in);
void * consumer(void *park_out);
void * monitor(void *park_in);
void initial(Parking_Lot *cp, int size);

void* producer(void *park_in)
 {
    Parking_Lot *temp;
    unsigned int seed;
    temp = (Parking_Lot *)park_in;

    pthread_barrier_wait(&temp->bar);
    while (1)
    {
       // usleep(rand_r(&seed) % ONE_SECOND);
        sleep(rand_r(&seed)%5);
        pthread_mutex_lock(&temp->lock);

        while (temp->occupied == temp->capacity)
            pthread_cond_wait(&temp->space, &temp->lock);

        temp->park[temp->next_in] = rand_r(&seed) % RANGE;
        temp->pa_f = 1;
        temp->flag = pthread_self();
        temp->occupied++;
        temp->next_in++;
        temp->next_in %= temp->capacity;
        temp->sum_in++;

        pthread_cond_signal(&temp->car);
        pthread_mutex_unlock(&temp->lock);
    }
    return ((void *)NULL);
}

void* consumer(void *park_out)
 {
    Parking_Lot *temp;
    unsigned int seed;
    temp = (Parking_Lot *)park_out;
    pthread_barrier_wait(&temp->bar);
    while(1)
    {
       // usleep(rand_r(&seed) % ONE_SECOND);
        sleep(rand_r(&seed)%5);
        pthread_mutex_lock(&temp->lock);
        while (temp->occupied == 0)
            pthread_cond_wait(&temp->car, &temp->lock);

        temp->flag = pthread_self();
        temp->occupied--;
        temp->next_out++;
        temp->next_out %= temp->capacity;
        temp->sum_out++;
        temp->pa_f = 0;

        pthread_cond_signal(&temp->space);
        pthread_mutex_unlock(&temp->lock);

    }
    return ((void *)NULL);

}

void *monitor(void *park_in)
{
    unsigned int seed;
    Parking_Lot *temp;
    temp = (Parking_Lot *)park_in;

    while(1)
    {
        // sleep(PERIOD);
        usleep(rand_r(&seed) % ONE_SECOND);
        pthread_mutex_lock(&temp->lock);
        if(temp->pa_f == 1||temp->pa_f == 0)
        {
            printf("Delta: %d\n", temp->sum_in - temp->sum_out - temp->occupied);
            printf("Number of cars in park: %d\n", temp->occupied);
        }
        pthread_mutex_unlock(&temp->lock);
    }

    return ((void *)NULL);
}

void initial(Parking_Lot *cp, int size)
{
    cp->occupied = cp->next_in = cp->next_out = cp->sum_in = cp->sum_out = 0;
    cp->capacity = size;
    cp->pa_f = -1;
    cp->park = (int *)malloc(cp->capacity * sizeof(*cp->park));

    pthread_barrier_init(&cp->bar, NULL, NUM_THREADS);

    if (cp->park == NULL)
    {
        perror("Initialize Error !");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));

    int rel = pthread_mutex_init(&cp->lock, NULL);
    if(rel != 0)
    {
        perror("Initialize failed !");
        exit(EXIT_FAILURE);
    }
    int res = pthread_cond_init(&cp->space, NULL);
    if(res != 0)
    {
        perror("Initialize failed !");
        exit(EXIT_FAILURE);
    }
    int rec = pthread_cond_init(&cp->car, NULL);
    if(rec != 0)
    {
        perror("Initialize failed !");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    int res_in1,res_in2,res_out1,res_out2,res_m;
    if (argc != 2)
    {
        printf("Usage: %s parksize!\n", argv[0]);
        exit(1);
    }
    Parking_Lot mypark;

    initial(&mypark, atoi(argv[1]));

    pthread_t thread_in1, thread_out1, thread_m;
    pthread_t thread_in2, thread_out2;

    res_in1 = pthread_create(&thread_in1, NULL, producer, (void *)&mypark);
    if(res_in1 != 0)
    {
        perror("thread_in1 creation failed !");
        exit(EXIT_FAILURE);
    }
    res_out1 = pthread_create(&thread_out1, NULL, consumer, (void *)&mypark);
     if(res_out1 != 0)
    {
        perror("thread_out1 creation failed !");
        exit(EXIT_FAILURE);
    }
    res_in2 = pthread_create(&thread_in2, NULL, producer, (void *)&mypark);
     if(res_in2 != 0)
    {
        perror("thread_in2 creation failed !");
        exit(EXIT_FAILURE);
    }
    res_out2 = pthread_create(&thread_out2, NULL, consumer, (void *)&mypark);
     if(res_out2 != 0)
    {
        perror("thread_out2 creation failed !");
        exit(EXIT_FAILURE);
    }
    res_m = pthread_create(&thread_m, NULL, monitor, (void *)&mypark);
     if(res_m != 0)
    {
        perror("thread_m creation failed !");
        exit(EXIT_FAILURE);
    }

    printf("The first producer ID is :%u\n",thread_in1);
    printf("The second producer ID is :%u\n",thread_in2);
    printf("The first consumer ID is :%u\n",thread_out1);
    printf("The second consumer ID is :%u\n",thread_out2);

    pthread_join(thread_in1, NULL);
    pthread_join(thread_out1, NULL);
    pthread_join(thread_in2, NULL);
    pthread_join(thread_out2, NULL);
    pthread_join(thread_m, NULL);

    pthread_exit(0);
}
