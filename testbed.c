#include "utils.h"

void moveStoS(Stack* st1, Stack* st2);

// move q tail to s head
void moveQtoS(Queue* que, Stack* st) {
    Node* N = NULL, * aux = NULL;
    if (que->head)
        N = que->head, aux = que->head->next;
    else
        return;

    N->next = st->head;
    st->head = N;
    que->head = aux;
    que->size -= 1;
    st->size += 1; 

    // parcurge coada ca sa modifice tail
    Stack* exchange = createStack();
    N = que->head;
    if (que->size) {
        que->tail = NULL;
        return;
    }
    if (N)
        while(N->next) {
            aux = N->next;
            if (aux == que->tail) {
                que->tail = N;
                N->next = NULL;
                break;
            }
            moveStoS(exchange, (Stack*)que);
        }
    while (exchange->head)
        moveStoS((Stack*)que, exchange);

    free(exchange);
    return;
}

// move s head to q tail
void moveStoQ(Queue* que, Stack* st) {
    Node* N = NULL, * aux = NULL;
    if (st->head)
        N = st->head, aux = st->head->next;
    else
        return;

    que->tail->next = N;
    que->tail = N;
    N->next = NULL;
    st->head = aux;
    st->size -= 1;
    que->size += 1;
    return;
}

// move s2 head to s1 head
void moveStoS(Stack* st1, Stack* st2) {
    Node* N = NULL, * aux = NULL;
    if (st2->head)
        N = st2->head, aux = st2->head->next;
    else
        return;

    N->next = st1->head;
    st1->head = N;
    st2->head = aux;
    st1->size += 1;
    st2->size -= 1;
    return;
}

// move q2 head to q1 head
void headsMoveQtoQ(Queue* que1, Queue* que2) {
    Stack* exchange = createStack();
    dequeue(que2, exchange);
    enqueue(que1, exchange);
    free(exchange);
    return;
}

// move q2 tail to q1 head
void moveQtoQ(Queue* que1, Queue* que2) {
    Node* N = NULL, * aux = NULL;
    if (que2->head)
        return;
    else
        return;
    que2->tail->next = que1->head;
    que1->head = que2->tail;

    // parcurge coada ca sa modifice tail
    Stack* exchange = createStack();
    N = que2->head;
    if (que2->size == 0) {
        que2->tail = NULL;
        return;
    }
    if (N)
        while(N->next) {
            aux = N->next;
            if (aux == que2->tail) {
                que2->tail = N;
                N->next = NULL;
                break;
            }
            moveStoS((Stack*)que1, exchange);
        }
    while (exchange->head) {
        moveStoS((Stack*)que2, exchange);
    }

    free(exchange);
    return;
}

Node* findNodeInQ(Queue* que, Stack* exchange, int32_t tID, int8_t* headFlag) {
    Node* N = que->head;
    *headFlag = 0;
    if (!N)
        return NULL;
    
    if (((Task*)N->data)->taskID == tID) {
        *headFlag = 1;
        return NULL;
    }
    
    while (N) {
        if (((Task*)N->data)->taskID == tID) {
            return N;
        }

        Node* aux = N->next;
        moveStoS(exchange, (Stack*)que);
        que->head = aux;
        N = que->head;
    }
    return NULL;
}

void run2(volatile int32_t T, const int32_t Q, const int32_t N,
         Stack* thPool, Queue* waiting, Queue* running, Queue* finished, Stack* temp) {
    Stack* exchangeSR = createStack();
    Stack* exchangeSW = createStack();
    Stack* finishedTh = createStack();
    Stack* exchangeQ = createStack();

    //printf("%d | %d\n", temp->size, running->size);
    //printf("waiting size: %d\n", waiting->size);
    while (thPool->head) {
        moveStoS(exchangeSW, thPool);
        Node* node = exchangeSW->head;
        if (waiting->size) {
            headsMoveQtoQ(running, waiting);
            ((Thread*)node->data)->tPtr = running->head;
            ((Thread*)node->data)->taskID = ((Task*)((Thread*)node->data)->tPtr->data)->taskID;
            ((Thread*)node->data)->state = 1;
            ((Task*)((Thread*)node->data)->tPtr->data)->thID = ((Thread*)node->data)->thID;

            moveStoS(temp, exchangeSW);
        }
    }

    //printf("%d | %d\n", temp->size, running->size);
    while (exchangeSW->size)
        moveStoS(thPool, exchangeSW);
    
    //printf("%d | %d | %d | %d\n", exchangeSR->size, temp->size, thPool->size, running->size);
    while (temp->head) {
        moveStoS(exchangeSR, temp);
        
        Node* node = exchangeSR->head;
        Node* assocTask = ((Thread*)node->data)->tPtr;
        ((Task*)assocTask->data)->clk += T * (T <= Q) + Q * (T > Q);
        
        if CLKCheck(assocTask->data) {
            int8_t headFlag = 0;
            findNodeInQ(running, exchangeQ, ((Task*)assocTask->data)->taskID, &headFlag);
            // printf ("found node %d\n", ((Task*)running->head->data)->taskID);
            // printf ("in exchange: %p -> %d\n", exchangeQ->head, exchangeQ->size);
            
            headsMoveQtoQ(finished, running);
            
            Node* finTask = finished->head;
            ((Task*)finTask->data)->state = 0;
            ((Task*)finTask->data)->thID = 0;
            
            // if (!headFlag)
                while (exchangeQ->head)
                    moveStoS((Stack*)running, exchangeQ);

            // printf ("in exchange: %p -> %d\n", exchangeQ->head, exchangeQ->size);

            ((Thread*)node->data)->tPtr = NULL;
            ((Thread*)node->data)->state = 0;
            ((Thread*)node->data)->usedF = 1;
            ((Thread*)node->data)->taskID = 0;
            
            moveStoS(finishedTh, exchangeSR);
        }
    }
    //printf("%d | %d | %d | %d\n", exchangeSR->size, temp->size, thPool->size, running->size);
    while (exchangeSR->size)
        moveStoS(temp, exchangeSR);
    //printf("%d | %d | %d | %d\n", exchangeSR->size, temp->size, thPool->size, running->size);
    
    while (finishedTh->size) {
        moveStoS(thPool, finishedTh);
        ((Thread*)thPool->head->data)->usedF = 0;
        moveStoS(exchangeSW, thPool);
        Node* node = exchangeSW->head;
        if (waiting->size) {
            headsMoveQtoQ(running, waiting);
            ((Thread*)node->data)->tPtr = running->head;
            ((Thread*)node->data)->taskID = ((Task*)((Thread*)node->data)->tPtr->data)->taskID;
            ((Thread*)node->data)->state = 1;
            ((Task*)((Thread*)node->data)->tPtr->data)->thID = ((Thread*)node->data)->thID;

            moveStoS(temp, exchangeSW);
        }
    }

    while (exchangeSW->size)
        moveStoS(thPool, exchangeSW);
    //printf("%d | %d | %d | %d\n", exchangeSR->size, temp->size, thPool->size, running->size);
    if (T >= Q)
        run2(T - Q, Q, N, thPool, waiting, running, finished, temp);
}
