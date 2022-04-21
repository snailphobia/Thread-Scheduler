#include "utils.h"

void moveStoS(Stack* st1, Stack* st2);

// src este mask pentru Stack* / Queue*
// ne intereseaza doar primul element
Node* moveInOrder(Queue* que, void* src) {
    Stack* ptr = src;
    Stack* exchange = createStack();
    Node* retVal = ptr->head;
    if (ptr->head == NULL) {
        printf("Warn: %p (src) is empty, can not retrieve node\n", ptr);
        free(exchange);
        return NULL;
    }

    if (que->head == NULL) {
        free(exchange);
        Node* aux = ptr->head->next;
        que->head = que->tail = ptr->head;
        ptr->head = aux;
        que->size = 1; ptr->size -= 1;
        que->head->next = NULL; que->tail->next = NULL;
        return retVal;
    }
    
    //verificam sa fie / nu fie primul element
    if (compareQueueNodes(que->head, ptr->head) >= 1) {
        Node* aux = ptr->head->next;
        ptr->head->next = que->head;
        que->head = ptr->head;
        ptr->head = aux;
        
        que->size += 1;
        ptr->size -= 1;
        free(exchange);
        return retVal;
    }

    while (que->size) {
        Node* aux = que->head, * nAux = que->head->next;
        
        if (compareQueueNodes(que->head, ptr->head) >= 1) {
            Node* nPtr = ptr->head->next;
            ptr->head->next = exchange->head;
            exchange->head = ptr->head;
            ptr->head = nPtr;
            ptr->size -= 1; exchange->size += 1;
            break;
        }

        que->head->next = exchange->head;
        exchange->head = que->head;
        que->head = nAux;
        exchange->size += 1; que->size -= 1;
    }

    // daca suntem la finalul cozii inseamna ca trebuie pus ultimul nodul dat
    if (que->size == 0) {
        Node* auxH = ptr->head->next;
        ptr->head->next = NULL;
        que->head = que->tail = ptr->head;
        ptr->head = auxH;
        que->size = 1; ptr->size -= 1;
    }

    while (exchange->head) {
        Node* hd = exchange->head->next;
        exchange->head->next = que->head;
        
        // made a severe lapse in judgement
        if (que->head == NULL) {
            que->head = que->tail = exchange->head; 
        }
        else
            que->head = exchange->head;
        exchange->head = hd;

        exchange->size -= 1; que->size += 1;
    }

    if (exchange->size != 0)
        printf ("Warn (in moveInOrder/testbed.c): exchange stack not empty at exit\n");

    free(exchange);
    return retVal;
}

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

    if (que->head == NULL) {
        st->head->next = NULL;
        que->head = que->tail = st->head;
        st->head = aux;
        st->size -= 1; que->size += 1;
        return;
    }
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
        return N;
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
         Stack* thPool, Queue* waiting, Queue* running, Queue* finished, Stack* temp, int8_t* final) {
    Stack* exchangeSR = createStack();
    Stack* exchangeSW = createStack();
    Stack* finishedTh = createStack();
    Stack* exchangeQ = createStack();
    Stack* SRMLE = createStack();
    fflush(stdout);

    while (thPool->head) {
        moveStoS(exchangeSW, thPool);
        Node* node = exchangeSW->head;
        if (waiting->size) {
            Node* slaved = moveInOrder(running, waiting);

            ((Thread*)node->data)->tPtr = slaved;
            ((Thread*)node->data)->taskID = ((Task*)((Thread*)node->data)->tPtr->data)->taskID;
            ((Thread*)node->data)->state = 1;
            ((Task*)((Thread*)node->data)->tPtr->data)->thID = ((Thread*)node->data)->thID;

            moveStoS(temp, exchangeSW);
        }
    }
    
    while (exchangeSW->size)
        moveStoS(thPool, exchangeSW);

    while (temp->head) {
        moveStoS(exchangeSR, temp);

        Node* node = exchangeSR->head;
        Node* assocTask = ((Thread*)node->data)->tPtr;
        ((Task*)assocTask->data)->clk += T * (T <= Q) + Q * (T > Q);
    }

    while (exchangeSR->head) {
        Node* node = exchangeSR->head;
        Node* auxH = node->next;
        Node* assocTask = ((Thread*)node->data)->tPtr;
        if CLKCheck(assocTask->data) {
            int8_t headFlag = 0;
            findNodeInQ(running, exchangeQ, ((Task*)assocTask->data)->taskID, &headFlag);
            moveStoQ(finished, running);
            Node* finTask = finished->tail;
            ((Task*)finTask->data)->state = 0;
            ((Task*)finTask->data)->thID = 0;
            // if (!headFlag)
            while (exchangeQ->head)
                moveInOrder(running, exchangeQ);
            ((Thread*)node->data)->tPtr = NULL;
            ((Thread*)node->data)->state = 0;
            ((Thread*)node->data)->usedF = 1;
            ((Thread*)node->data)->taskID = 0;
            
            moveStoS(finishedTh, exchangeSR);
        }
        else
            moveStoS(SRMLE, exchangeSR);
        // exchangeSR->head = auxH;
    }

    while(SRMLE->head)
        moveStoS(exchangeSR, SRMLE);

    while (exchangeSR->size)
        moveStoS(temp, exchangeSR);
    
    while (finishedTh->size) {
        moveStoS(thPool, finishedTh);
        ((Thread*)thPool->head->data)->usedF = 0;
        moveStoS(exchangeSW, thPool);
        Node* node = exchangeSW->head;
        if (waiting->size) {
            Node* slaved = moveInOrder(running, waiting);
            ((Thread*)node->data)->tPtr = slaved;
            ((Thread*)node->data)->taskID = ((Task*)((Thread*)node->data)->tPtr->data)->taskID;
            ((Thread*)node->data)->state = 1;
            ((Task*)((Thread*)node->data)->tPtr->data)->thID = ((Thread*)node->data)->thID;

            moveStoS(temp, exchangeSW);
        }
    }

    while (exchangeSW->size)
        moveStoS(thPool, exchangeSW);

    if (T >= Q)
        run2(T - Q, Q, N, thPool, waiting, running, finished, temp, final);
    // if (*final == 0) {
    //     *final = 1;
    //     run2(0, Q, N, thPool, waiting, running, finished, temp, final);
    // }
    return;
}
