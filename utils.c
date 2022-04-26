#include "utils.h"

// cauta primul id disponibil
int32_t assignTask(int8_t* idPool) {
    for (int i = 1; i < 32678; i++)
        if (idPool[i] == 0) {
            idPool[i] = 1;
            return i;
        }
    return 0;
}

Queue* createQueue() {
    Queue* que = calloc(1, sizeof(Queue));
    que->head = que->tail = NULL;
    que->size = 0;
    return que;
}

Stack* createStack() {
    Stack* st = calloc(1, sizeof(Stack));
    st->head = st->tail = NULL;
    st->size = 0;
    return st;
}

extern void moveInOrder();

// mutam primul nod din coada in stiva temporara
void dequeue(Queue* que, Stack* temp) {
    if (que->head == NULL)
        return;
    Node* addr = que->head->next;
    que->head->next = temp->head;
    temp->head = que->head;

    que->head = addr;
    que->size -= 1;
    temp->size += 1;
    return;
}

// mutam primul element din stiva inapoi in coada de unde provine
void enqueue(Queue* que, Stack* temp) {
    if (temp->head == NULL)
        return;

    if (que->head == NULL) {
        que->head = que->tail = temp->head;
        temp->head = temp->head->next;
        que->size += 1;
        temp->size -= 1;
        return;
    }

    Node* headCopy = que->head;
    que->head = temp->head;
    temp->head = temp->head->next;
    que->head->next = headCopy;
 
    que->size += 1;
    temp->size -= 1;
    return;
}

Node* getTask(Queue* waiting, Queue* running, Queue* finished, int32_t pID) {
    Node* iter = NULL, * srcRes = NULL;
    Stack* temp = createStack();
    int8_t flag = 0;
    while (waiting->size > 0) {
        iter = waiting->head;
        if (((Task*)iter->data)->taskID == pID) {
            // hai sa facem 80 de caractere pe rand
            printf("T" 
             "a" 
             "s" 
             "k"
             " " 
             "%d"
             " " 
             "i" 
             "s"
             " "
             "w" 
             "a"
             "i" 
             "t"
             "i"
             "n"
             "g"
             " " 
             "("
             "r"
             "e"
             "m"
             "a"
             "i"
             "n"
             "i"
             "n"
             "g"
             "_"
             "t"
             "i"
             "m"
             "e"
             " "
             "=" 
             " "
             "%d"
             ")"
             "."
             "\n", 
             pID, 
             ((Task*)iter->data)->TTK);
            // luati de va bucurati 🤡
            srcRes = iter;
            flag = 1;
            break;
        }
        dequeue(waiting, temp);
    }
    while (temp->size > 0)
        enqueue(waiting, temp);
    if (flag == 1) {
        free(temp);
        return srcRes;
    }

    while (running->size > 0) {
        iter = running->head;
        if (((Task*)iter->data)->taskID == pID) {
            printf("Task"
             " " 
             "%d"
             " "
             "is"
             " " 
             "running"
             " " 
             "(remaining_time = %d).\n",
             pID,
             ((Task*)iter->data)->TTK 
             - ((Task*)iter->data)->clk);
            // 🤡
            srcRes = iter;
            flag = 1;
            break;
        }
        dequeue(running, temp);
    }
    while (temp->size > 0)
        enqueue(running, temp);
    if (flag == 1) {
        free(temp);
        return srcRes;
    }

    while (finished->size > 0) {
        iter = finished->head;
        if (((Task*)iter->data)->taskID == pID) {
            printf("Task %d is finished (executed_time = %d).\n",
                   pID, ((Task*)iter->data)->TTK);
            srcRes = iter;
            flag = 1;
            break;
        }
        dequeue(finished, temp);
    }
    while (temp->size > 0)
        enqueue(finished, temp);
    
    if (!flag) {
        printf("Task %d not found.\n", pID);
    }

    if (temp->size > 0)
        printf("Warn: temporary stack is not emp"
        "ty at the end of the function getTask\n");
    free(temp);
    return srcRes;
}

// caut in thPool, daca nu apare, atunci caut in temp, pentru fiecare thread
// am chiar tupeul pervers sa cred ca stivele sunt bine facute si nu lipseste
// nimic
Node* getThread(Stack* thPool, Stack* temp, Queue* running, uint16_t pID) {
    Stack* exchange = createStack();
    extern void moveStoS();
    while (thPool->head) {
        moveStoS(exchange, thPool);
        Thread* thIt = exchange->head->data;
        if (thIt->thID == pID)
            printf("Thread %d is idle.\n", pID);
    }
    
    while (exchange->head)
        moveStoS(thPool, exchange);

    while (temp->head) {
        moveStoS(exchange, temp);
        Thread* thIt = exchange->head->data;
        if (thIt->thID == pID)
            printf("Thread %d is running task %d (remaining_time = %d).\n",
                   pID, ((Task*)thIt->tPtr->data)->taskID,
                   ((Task*)thIt->tPtr->data)->TTK
                   - ((Task*)thIt->tPtr->data)->clk);
        // recomand sa se scoata limita de 80 de caractere pe rand,
        // am auzit ca monitoarele de acum nu mai sunt cu tub
        // destul de crazy
    }
    
    while (exchange->head)
        moveStoS(temp, exchange);

    free(exchange);
    return NULL;
}

// atribuirea informatiei pe alta data, acum doar alocarea
// deprecated; mai utila este functia din testbed de adaugare direct din stiva
void addNodeInOrder(void* dest, Node* node) {
    // todo inlocuire bucati de mutare noduri cu enqueue/dequeue
    // edit: yeah I don't think I will
    Queue* que = dest;
    Stack* aux = createStack();

    if (que->tail == NULL) {
        que->head = node;
        que->tail = node;
        que->size = 1;
        free(aux);
        return;
    }
    Node* iter = que->head;
    int8_t flag = 0;
    que->size++;

    if (compareQueueNodes(node, iter) <= 0) {
        node->next = aux->head;
        aux->head = node;
        flag = 1;
    }
    while (que->head->next != NULL) {
        iter = que->head;
        Node* queFollowUp = iter->next;
        iter->next = aux->head;
        aux->head = iter;
        if (compareQueueNodes(node, queFollowUp) <= 0 && !flag) {
            node->next = aux->head;
            aux->head = node;
            flag = 1;
        }

        que->head = queFollowUp;
        iter = queFollowUp;
    }

    que->head->next = aux->head;
    aux->head = que->head;
    que->head = que->tail = NULL;
    if (!flag) {
        node->next = aux->head;
        aux->head = node;
    }
    int32_t counter = 0;
    for (Node* top = aux->head; top != NULL && counter < que->size; counter++) {
        Node* followUp = top->next;
        if (que->head == NULL) {
            que->head = que->tail = top;
            top->next = NULL;
        }
        else {
            top->next = que->head;
            que->head = top;
        }
        top = followUp;
        
    }
    free(aux);
    return;
}

// 🤡
// in ref-uri e un egal in plus la antete fata de enunt
void printQueue(Queue* que, const int8_t qType) {
    switch (qType) {
        case 1:
            printf("====== Waiting queue =======\n");
            break;
        case 2:
            printf("====== Running in parallel =======\n");
            break;
        case 3:
            printf("====== Finished queue =======\n");
            break;
        default: break;
    }

    Stack* temp = createStack();
    // if (que->size == 0)
    //     return;
    
    printf("[");
    while (que->size > 0) {
        Node* iter = que->head;
        Task* tIter = iter->data;
        switch (qType) {
            case 1:
                printf("(%d: priority = %d, remaining_time = %d)", tIter->taskID, tIter->priority, tIter->TTK);
                break;
            case 2:
                printf("(%d: priority = %d, remaining_time = %d, running_thread = %d)", tIter->taskID, tIter->priority, tIter->TTK - tIter->clk, tIter->thID);
                break;
            case 3:
                printf("(%d: priority = %d, executed_time = %d)", tIter->taskID, tIter->priority, tIter->TTK);
                break;
            default: break;
        }
        if (que->size > 1)
            printf(",\n");
        dequeue(que, temp);
    }

    while (temp->size > 0)
        enqueue(que, temp);

    free(temp);
    printf("]\n");
    return;
}

Node* createNode(void* dest, const uint8_t type) {
    Node* node = calloc(1, sizeof(Node));
    node->next = NULL;

    if (type == 1) {
        Stack* st = dest;

        if (st->tail == NULL) {
            st->head = st->tail = node;
            st->size = 1;
            return node;
        }

        node->next = st->head;
        st->head = node;
        st->size++;
        return node;
    } else {
        return node;
    }
    return node;
}

int8_t compareQueueNodes(Node* A, Node* B) {
    Task* AT = A->data;
    Task* BT = B->data;
    if (AT->priority > BT->priority)
        return -1;
    if (AT->priority < BT->priority)
        return 1;
    if (AT->TTK - AT->clk > BT->TTK - BT->clk)
        return 1;
    if (AT->TTK < BT->TTK)
        return -1;
    if (AT->taskID > BT->taskID)
        return 1;
    if (AT->taskID < BT->taskID)
        return -1;
    return 0;
}   // magic, do not touch

// main_actual
void parser(const int32_t Q, const int32_t N) {
    Stack* thPool = createStack(); thPool->size = N;
    Queue* running = createQueue();
    Queue* waiting = createQueue();
    Queue* finished = createQueue();
    for (int i = N - 1; i >= 0; i--) {
        Node* node = createNode(thPool, 1);
        node->data = calloc(1, sizeof(Thread*));
        ((Thread*)node->data)->state = 0;
        ((Thread*)node->data)->taskID = -1;
        ((Thread*)node->data)->thID = i;
        ((Thread*)node->data)->usedF = 0;
        ((Thread*)node->data)->tPtr = NULL;
    }

    char* cmd = calloc(S1, sizeof(char));
    Stack* temp = createStack();
    int8_t* idPool = calloc(32780, 1);
    int32_t elapsed = 0, taskN = 0;
    while (scanf("%s", cmd) > 0) {
        if (!strcmp(cmd, "add_tasks")) {
            
            int16_t num = 0, priority = 0;
            int32_t TTK = 0;
            int8_t sign = scanf("%hd%d%hd", &num, &TTK, &priority);
            Stack* loading = createStack();
            for (int i = 1; i <= num; i++) {
                Node* node = createNode(waiting, 2);
                node->data = calloc(1, sizeof(Task));
                ((Task*)node->data)->priority = priority;
                ((Task*)node->data)->state = 0;
                ((Task*)node->data)->taskID = assignTask(idPool);
                ((Task*)node->data)->thID = -1;
                ((Task*)node->data)->TTK = TTK;
                ((Task*)node->data)->clk = 0;
                taskN++;
                
                node->next = loading->head;
                loading->head = node;
                loading->size += 1;
                printf("Task created successfully : ID %d.\n", ((Task*)node->data)->taskID);
            }

            while (loading->head)
                moveInOrder(waiting, loading);
            free(loading);
        }
        if (!strcmp(cmd, "get_task")) {
            int32_t pID = 0;
            int8_t sign = scanf("%d", &pID);
            Node* srcRes = getTask(waiting, running, finished, pID);
        }
        if (!strcmp(cmd, "get_thread")) {
            int32_t pID = 0;
            int8_t sign = scanf("%d", &pID);
            // if (pID >= N)
                // printf("Thread %d not found.\n", pID);
            // else {
            if (pID < N) {
                Node* srcRes = getThread(thPool, temp, running, pID);
            }
            //}
        }
        if (!strcmp(cmd, "print")) {
            int8_t sign = scanf(" %s", cmd);
            if (!strcmp(cmd, "waiting"))
                printQueue(waiting, 1);
            if (!strcmp(cmd, "running"))
                printQueue(running, 2);
            if (!strcmp(cmd, "finished"))
                printQueue(finished, 3);
        }
        if (!strcmp(cmd, "run")) {
            int32_t vT = 0, maxT = 0;
            int8_t sign = scanf("%d", &vT), final = 1;
            printf("Running tasks for %d ms...\n", vT);
            extern int32_t run2();
            while (vT >= Q && taskN > finished->size && final) {
                maxT = run2(vT, Q, N, thPool, waiting, running, finished, temp, &final, idPool);
                vT -= Q;
                elapsed += maxT;
                final = 1; // numai cuvinte bune pentru fisierul ref
                // ba da macar v-a trecut prin acei 4 neuroni gandul ca ar trebui verificat ca
                // raspunsurile corecte sa corespunda cu enuntul
            }

            while (taskN > finished->size && vT) {
                // printf ("w: %d r: %d f: %d tot: %d\n", waiting->size, running->size, finished->size, taskN);
                run2(1, Q, N, thPool, waiting, running, finished, temp, &final, idPool);
                elapsed += 1;
                vT -= 1;
            }
        }
        if (!strcmp(cmd, "finish")) {
            int final = 0;
            extern int32_t run2();
            while (taskN > finished->size) {
                int32_t maxT = 0;
                // printf ("w: %d r: %d f: %d tot: %d\n", waiting->size, running->size, finished->size, taskN);
                maxT = run2(Q, Q, N, thPool, waiting, running, finished, temp, &final, idPool);
                elapsed += maxT;
            }
            printf ("Total time: %d", elapsed);
        }
    }

    Stack* unload = createStack();
    while (waiting->head && waiting->size) {
        dequeue(waiting, unload);
    }
    while (running->head && running->size) {
        dequeue(running, unload);    
    }
    while (finished->head && finished->size) {
        dequeue(finished, unload);
    }
    fflush(stdout);
    free(waiting);
    free(running);
    free(finished);

    while (unload->head && unload->size) {
        Node* nHead = unload->head->next;
        Node* crt = unload->head;
        free((Task*)crt->data);
        free(crt);
        unload->head = nHead;
    }
    free(unload);
    while (thPool->head) {
        free((Thread*)thPool->head->data);
        Node* node = thPool->head->next;
        free(thPool->head);
        thPool->head = node;
    }
    free(thPool);

    while (temp->head) {
        free((Thread*)temp->head->data);
        Node* node = temp->head->next;
        free(temp->head);
        temp->head = node;
    }

    free(idPool);
    free(temp);
    free(cmd);
    return;
}

// fiecare iteratie a functiei va acoperi bucata de timp ceruta
// threadurile folosite intra in temp
// la finalul unei executii, "schedulerul" ramane pe pauza

void run(volatile int32_t T, const int32_t Q, const int32_t N,
         Stack* thPool, Queue* waiting, Queue* running, Queue* finished, Stack* temp) {

    Stack* tempQ = createStack();
    Stack* tempQ2 = createStack();
    Stack* swapSt = createStack();
    Stack* usedEnded = createStack();

    if (waiting->size == 0 && running->size == 0) {
        free(temp);
        free(tempQ);
        free(swapSt);
        free(usedEnded);
        return;
    }
    
    // aici incepem daca mai avem query-uri
    Node* nodeTh = thPool->head, * nodeTask = waiting->head;
    //printf("%d\n", T);
    int16_t swapped = 0;
    Stack* ranToF = createStack();
    while (running->head && running->size) {
        dequeue(running, ranToF);
        // printf("%d %d\n", running->size, ranToF->size);
        fflush(stdout);
        Node* nd = ranToF->head;
        ((Task*)nd->data)->clk += T * (T <= Q) + Q * (T > Q);
        // daca vreununul dintre taskuri care se afla deja in running s-a terminat
        // mutam threadul inapoi in pool
        if CLKCheck(nd->data) {
            swapped++;
            // printf("sclavie incheiata cu succes la task id %d\n", ((Task*)nd->data)->taskID);
            Node* auxSw = nd->next;
            // printf("in clkcheck . . . %p -> %p\n", nd, auxSw);
            // ((Task*)nd->data)->tPtr->next = usedEnded->head;
            // usedEnded->head = ((Task*)nd->data)->tPtr;
            ((Thread*)usedEnded->head->data)->tPtr = NULL;
            ((Thread*)usedEnded->head->data)->taskID = 0;
            ((Thread*)usedEnded->head->data)->usedF = 0;
            enqueue(finished, ranToF);
            // printf("  %d\n", finished->size);
        }
    }
    // printf("size of rantof is %d | start is at %p\n", ranToF->size, ranToF->head);
    while (ranToF->head) {
        enqueue(running, ranToF);
        // printf("%d %d\n", running->size, ranToF->size);
        fflush(stdout);
    }
    while (thPool->head && waiting->head) {
        //printf("in loop: thread %p task %p\n", thPool->head, waiting->head);
        if (temp->head == NULL) {
            temp->head = temp->tail = nodeTh;
            thPool->head = thPool->head->next;
            temp->size += 1;
            thPool->size -= 1;
        } else {
            Node* aux = temp->head;
            temp->head = thPool->head;
            nodeTh = thPool->head->next;
            temp->head->next = aux;
            thPool->head = nodeTh;
            temp->size += 1;
            thPool->size -= 1;
        }

        // mutam obiectul nodeTask din waiting in running
        // why are you running?
        
        dequeue(waiting, tempQ);
        enqueue(running, tempQ);
        Node* ranTask = running->head;
        // mai trebuiesc si ordonate nodurile din cozi la final

        // setam valorile de mediu pentru thread
        ((Thread*)temp->head->data)->state = 1; // running
        ((Thread*)temp->head->data)->taskID = ((Task*)ranTask->data)->taskID;
        ((Thread*)temp->head->data)->tPtr = ranTask;

        // setam valorile de mediu pentru task
        // (sunt cam multe, brace yourself)
        Task* value = ranTask->data;
        value->clk += Q * (Q <= T) + T * (Q > T);
        value->state = 1; // running
        value->thID = ((Thread*)temp->head->data)->thID;
        // value->tPtr = temp->head;
        // aici s-a terminat partea de pus sclavii la munca
        // acum verificam sclavii deja assignati daca si-au terminat treaba
        // daca da, le luam treaba terminata si o punem in finished
        // si le dam alte treburi

        if CLKCheck(ranTask->data) {
            // printf("sclavie incheiata cu succes la taskID %d\n", ((Task*)ranTask->data)->taskID);
            fflush(stdout);
            dequeue(running, tempQ2);
            enqueue(finished, tempQ2);
            Node* freedTh = NULL; // ((Task*)ranTask->data)->tPtr;

            while (temp->head) {
                if (((Thread*)temp->head->data)->thID == ((Task*)ranTask->data)->thID)
                    break;
                Node* aux = temp->head->next;
                temp->head->next = swapSt->head;
                swapSt->head = temp->head;
                temp->head = aux;
            }
            // am gasit threadul terminat
            swapped++;

            Node* aux = temp->head->next;
            temp->head->next = usedEnded->head;
            usedEnded->head = temp->head;
            temp->head = aux;
            usedEnded->size += 1;
            ((Thread*)usedEnded->head->data)->usedF = 1;
            // il punem inapoi in pool

            while (swapSt->head) {
                aux = swapSt->head->next;
                swapSt->head->next = temp->head;
                temp->head = swapSt->head;
                swapSt->head = aux;
            }
            // nodeTh = temp->head;
            // ((Thread*)nodeTh->data)->state = 0; // terminated
            // ((Thread*)nodeTh->data)->taskID = 0; // terminated
            // Node* aux = thPool->head;
            // thPool->head = temp->head;
            // thPool->head->next = aux;
            // temp->head = temp->head->next;
            // nodeTh = temp->head;
            // temp->size -= 1; thPool->size += 1;
        }
    }
    
    while(swapped) {
        // printf("%d : %p -> %p\n", swapped, usedEnded->head, usedEnded->head->next);
        fflush(stdout);
        if (usedEnded->head) {
            Node* aux = usedEnded->head->next;
            usedEnded->head->next = thPool->head;
            thPool->head = usedEnded->head;
            usedEnded->head = aux;
            usedEnded->size -= 1;
            thPool->size += 1;
            ((Thread*)thPool->head->data)->usedF = 0;
            ((Thread*)thPool->head->data)->tPtr = NULL;
            swapped--;
        }
        // il punem inapoi in pool
    }
    free(usedEnded);
    fflush(stdout);

    if (T >= Q)
        run(T - Q, Q, N, thPool, waiting, running, finished, temp);

    return;
}