#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define ONE_SECOND 1000000
#define RANGE 10
#define PERIOD 2
#define NUM_THREADS 4

// 定义数据结构描述停车场信息
struct Parking_Lot
{
    int *park;                 // 用一个整数数组buffer模拟停车场停车位
    int capacity;              // 停车场的车辆容量
    int occupied;              // 停车场现有车辆数目
    int next_in;               // 下一个进来的车的停车位置（用 park 数组代表的下标表示）
    int next_out;              // 下一个取走的车的停车位置（用 park 数组代表的下标表示）
    int sum_in;                // 记录停车场进入车辆的总和
    int sum_out;               //记录从停车场开出去的车辆总和
    int flag;                  //记录线程ID
    pthread_mutex_t lock;      //互斥量，保护该结构中的数据被线程互斥的方式使用
    pthread_cond_t space;      //条件变量，描述停车场是否有空位置
    pthread_cond_t car;        //条件变量，描述停车场是否有车
    pthread_barrier_t bar;     //线程屏障
};

static void * producer(void *cp_in);
static void * consumer(void *cp_in);
static void * monitor(void *cp_in);
static void initial(Parking_Lot *cp, int size);

static void initial(Parking_Lot *cp, int size)
{
    cp->occupied = cp->next_in = cp->next_out = cp->sum_in = cp->sum_out = 0;
    cp->capacity = size;        //设置停车场的大小

    cp->park = (int *)malloc(cp->capacity * sizeof(*cp->park));

    // 初始化线程屏障，NUM_THREADS 表示等待 NUM_THREADS = 4 个线程同步执行
    pthread_barrier_init(&cp->bar, NULL, NUM_THREADS);

    if (cp->park == NULL)
    {
        perror("malloc()");
        exit(1);
    }

    srand((unsigned int)getpid());

    pthread_mutex_init(&cp->lock, NULL);        // 初始化停车场的锁
    pthread_cond_init(&cp->space, NULL);        // 初始化描述停车场是否有空位的条件变量
    pthread_cond_init(&cp->car, NULL);          // 初始化描述停车场是否有车的条件变量
}

static void* producer(void *park_in)
 {
    Parking_Lot *temp;
    unsigned int seed;
    temp = (Parking_Lot *)park_in;

    // pthread_barrier_wait 函数表明，线程已完成工作，等待其他线程赶来
    pthread_barrier_wait(&temp->bar);
    while (1)
    {
        // 将线程随机挂起一段时间，模拟车辆到来的的随机性
        usleep(rand_r(&seed) % ONE_SECOND);

        pthread_mutex_lock(&temp->lock);

        // 循环等待直到有停车位
        while (temp->occupied == temp->capacity)
            pthread_cond_wait(&temp->space, &temp->lock);

        // 插入一个辆车（用随机数标识）
        temp->park[temp->next_in] = rand_r(&seed) % RANGE;

        // 各变量增量计算
        temp->flag = pthread_self();
        temp->occupied++;
        temp->next_in++;
        temp->next_in %= temp->capacity;         // 循环计数车辆停车位置
        temp->sum_in++;

        // 可能有的人在等有车可取（线程），这是发送 temp->car 条件变量
        pthread_cond_signal(&temp->car);

        // 释放锁
        pthread_mutex_unlock(&temp->lock);

    }
    return ((void *)NULL);

}

static void* consumer(void *park_out)
 {
    Parking_Lot *temp;
    unsigned int seed;
    temp = (Parking_Lot *)park_out;
    pthread_barrier_wait(&temp->bar);
    while(1)
    {
        // 将线程随机挂起一段时间，模拟车辆到来的的随机性
        usleep(rand_r(&seed) % ONE_SECOND);

        // 获取保护停车场结构的锁
        pthread_mutex_lock(&temp->lock);

        /* 获得锁后访问 temp->occupied 变量，此时如果车辆数为0（occupied ==0 ），
        pthread_cond_wait 进行的操作是忙等，释放锁（&temp->lock）供其它线程使用。
        直到 &temp->car 条件改变时再次将锁锁住 */
        while (temp->occupied == 0)
            pthread_cond_wait(&temp->car, &temp->lock);

        // 增加相应的增量
        temp->flag = pthread_self();
        temp->occupied--; // 现有车辆数目减1
        temp->next_out++;
        temp->next_out %= temp->capacity;
        temp->sum_out++;


        // 可能有的人在等有空空车位（线程），这是发送 temp->space 条件变量
        pthread_cond_signal(&temp->space);

        // 释放保护停车场结构的锁
        pthread_mutex_unlock(&temp->lock);

    }
    return ((void *)NULL);

}

// 监控停车场状况
static void *monitor(void *park_in)
{
    Parking_Lot *temp;
    temp = (Parking_Lot *)park_in;

    while(1)
    {
        sleep(PERIOD);
        // 获取锁
        pthread_mutex_lock(&temp->lock);
        printf("Delta: %d\n", temp->sum_in - temp->sum_out - temp->occupied);
        printf("Number of cars in park: %d\n", temp->occupied);
        // 释放锁
        pthread_mutex_unlock(&temp->lock);
    }

    return ((void *)NULL);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s parksize\n", argv[0]);
        exit(1);
    }
    Parking_Lot mypark;

    initial(&mypark, atoi(argv[1]));        // 初始化停车场数据结构

    pthread_t thread_in1, thread_out1, thread_m;               // 定义线程变量
    pthread_t thread_in2, thread_out2;

    pthread_create(&thread_in1, NULL, producer, (void *)&mypark);  // 创建往停车场停车线程（生产者1）
    pthread_create(&thread_out1, NULL, consumer, (void *)&mypark); // 创建从停车场取车线程（消费者1）
    pthread_create(&thread_in2, NULL, producer, (void *)&mypark); // 创建往停车场停车线程（生产者2）
    pthread_create(&thread_out2, NULL, consumer, (void *)&mypark); // 创建从停车场取车线程（消费者2）
    pthread_create(&thread_m, NULL, monitor, (void *)&mypark);  // 创建用于监控停车场状况的线程

    printf("The first producer ID is :%u\n",thread_in1);
    printf("The second producer ID is :%u\n",thread_in2);
    printf("The first consumer ID is :%u\n",thread_out1);
    printf("The second consumer ID is :%u\n",thread_out2);

    // pthread_join 的第二个参数设置为 NULL，表示并不关心线程的返回状态，仅仅等待指定线程（第一个参数）的终止
    pthread_join(thread_in1, NULL);
    pthread_join(thread_out1, NULL);
    pthread_join(thread_in2, NULL);
    pthread_join(thread_out2, NULL);
    pthread_join(thread_m, NULL);

    exit(0);
}
