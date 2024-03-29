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

// todo: actual threads
typedef struct {
    // https://imgur.com/Tz9IKSJ
    pthread_t* tID;
    clock_t clk;
} ElementPThread;

// states:
// 0 = waiting
// 1 = running
// Task: 2 = finished
typedef struct {
    int16_t thID; // id thread
    int8_t state; // flag pentru starea thread
    int32_t taskID; // id task pe care il ruleaza
    int8_t usedF; // flag daca a fost folosit deja in ciclul curent
    struct _Node* tPtr; // pentru task pe care il ruleaza
    // ElementPThread tiedThread; // todo
} Thread;

typedef struct {
    int16_t taskID; // id task
    int16_t state; // flag pentru starea task (deprecated)
    int16_t thID; // id thread care il ruleaza
    int16_t priority;
    int32_t TTK; // timpul necesar rularii complete
    int32_t clk; // timpul in care a fost rulat pana in prezent
} Task;

#define CLKCheck(N) (((Task*)N)->clk >= ((Task*)N)->TTK)
// verificare daca timpul curent depaseste timpul necesar
#define FCLKCheck(N, Q)  (((Task*)N)->TTK - ((Task*)N)->clk >= Q)
// verificare daca mai avem rulari de cuanta Q dupa cea curenta

Queue* createQueue();
Stack* createStack();
Node* createNode(void* dest, const uint8_t type);
void parser(const int32_t Q, const int32_t N);
int8_t compareQueueNodes(Node* A, Node* B);
void addNodeInOrder(void* dest, Node* node);
void dequeue(Queue* que, Stack* temp);
void enqueue(Queue* que, Stack* temp);
void printQueue(Queue* que, const int8_t qType);
Node* getTask(Queue* waiting, Queue* running, Queue* finished, int32_t pID);
Node* getThread(Stack* thPool, Stack* temp, Queue* running, uint16_t pID);
void run(volatile int32_t T, const int32_t Q, const int32_t N,
         Stack* thPool, Queue* waiting, Queue* running, Queue* finished, Stack* temp);

#endif