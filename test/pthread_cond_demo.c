#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
 
/* 静态方式初始化一个互斥锁和一个条件变量 */ 
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
 
/* 指向线程控制块的指针 */
static pthread_t tid1;
static pthread_t tid2;
static pthread_t tid3;
 
/* 函数返回值检查 */
static void check_result(char* str,int result)
{
    if (0 == result)
    {
        printf("%s successfully!\n",str);
    }
    else
    {
        printf("%s failed! error code is %d\n",str,result);
    }
}
 
/* 生产者生产的结构体数据，存放在链表里 */
struct node 
{
    int n_number;
    struct node* n_next;
};
struct node* head = NULL; /* 链表头,是共享资源 */
 
/* 消费者线程入口函数 */
static void* consumer(void* parameter) 
{
    struct node* p_node = NULL;
 
    pthread_mutex_lock(&mutex);    /* 对互斥锁上锁 */
 
    while (1)
    {
        while (head == NULL)    /* 判断链表里是否有元素 */
        {
            pthread_cond_wait(&cond,&mutex); /* 尝试获取条件变量 */
        }
        /* 
        pthread_cond_wait()会先对mutex解锁，
        然后阻塞在等待队列，直到获取条件变量被唤醒，
        被唤醒后，该线程会再次对mutex上锁，成功进入临界区。
        */
 
        p_node = head;    /* 拿到资源 */
        head = head->n_next;    /* 头指针指向下一个资源 */
        /* 打印输出 */
        printf("consume %d\n",p_node->n_number);
 
        free(p_node);    /* 拿到资源后释放节点占用的内存 */
    }
    pthread_mutex_unlock(&mutex);    /* 释放互斥锁 */
    return 0;
}
/* 生产者线程入口函数 */
static void* product(void* patameter)
{
    int count = 0;
    struct node *p_node;
 
    while(1)
    {
        /* 动态分配一块结构体内存 */
        p_node = (struct node*)malloc(sizeof(struct node));
        if (p_node != NULL)
        {
            p_node->n_number = count++;    
            pthread_mutex_lock(&mutex);    /* 需要操作head这个临界资源，先加锁 */
 
            p_node->n_next = head;
            head = p_node;    /* 往链表头插入数据 */
 
            pthread_mutex_unlock(&mutex);    /* 解锁 */
            printf("produce %d\n",p_node->n_number);
 
            pthread_cond_signal(&cond);    /* 发信号唤醒一个线程 */
 
            sleep(2);    /* 休眠2秒 */
        }
        else
        {
            printf("product malloc node failed!\n");
            break;
        }
    }
}

void usleep_posix(useconds_t usec) {
    struct timespec req = {0};
    req.tv_sec = usec / 1000000;          // 秒数
    req.tv_nsec = (usec % 1000000) * 1000; // 剩余纳秒数
    nanosleep(&req, NULL);
}

static void *loop(void *arg)
{
    uint64_t i = 0;

    while (1) {
        // sleep(1);
        // printf("loop %llu\n", i++);
        i++;
        if (i % 10 == 0) {
            continue;
        }
        sleep(1);
    }
}

 
int rt_application_init() 
{
    int result;
 
    /* 创建生产者线程,属性为默认值，入口函数是product，入口函数参数为NULL*/
    // result = pthread_create(&tid1,NULL,product,NULL);
    // check_result("product thread created ",result);
 
    /* 创建消费者线程,属性为默认值，入口函数是consumer，入口函数参数是NULL */
    // result = pthread_create(&tid2,NULL,consumer,NULL);
    // check_result("consumer thread created ",result);

    result = pthread_create(&tid3,NULL,loop,NULL);
    check_result("loop thread created ",result);


    return 0;
}

int main() 
{
    rt_application_init();
    
    /* 等待线程结束 */
    // pthread_join(tid1, NULL);
    // pthread_join(tid2, NULL);

    pthread_join(tid3, NULL);
    /* 销毁互斥锁和条件变量 */
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    
    return 0;
}