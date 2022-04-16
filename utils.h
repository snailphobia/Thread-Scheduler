#ifndef UTILS_DEF
#define UTILS_DEF

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <stdint.h>

#define S1 100
#define S2 256

#define TTKCheck(Q, v) ((double)(clock() - v) / CLOCKS_PER_SEC > Q)

typedef struct _Node {
    void* data;
    struct _Node* next;
} Node;

typedef struct _st_que{
    int32_t size;
    Node* head, * tail;
} Stack, Queue;

// states:
// 0 = waiting
// 1 = running
// Task: 2 = finished
typedef struct {
    int16_t thID;
    int16_t state;
    int32_t taskID;
} Thread;

typedef struct {
    int32_t taskID;
    int16_t state;
    int16_t thID;
    int32_t priority;
    int32_t TTK;
} Task;

// todo: actual threads
typedef struct {
    // https://imgur.com/Tz9IKSJ
    pthread_t* tID;
    clock_t clk;
} ElementPThread;

Queue* createQueue();
Stack* createStack();
Node* createNode(void* dest, const uint8_t type);
void parser(const int32_t Q, const int32_t N);
int8_t compareQueueNodes(Node* A, Node* B);
void addNodeInOrder(void* dest, Node* node);
void dequeue(Queue* que, Stack* temp);
void enqueue(Queue* que, Stack* temp);
Node* getTask(Queue* waiting, Queue* running, Queue* finished, int32_t pID);
#endif