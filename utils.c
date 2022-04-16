#include "utils.h"

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

    // que->tail->next = temp->head;
    // que->tail = temp->head;
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
            printf("Task %d is waiting (remaining_time = %d).\n", pID, ((Task*)iter->data)->TTK);
            srcRes = iter;
            flag = 1;
            break;
        }
        dequeue(waiting, temp);
    }
    while (temp->size > 0)
        enqueue(waiting, temp);

    while (running->size > 0) {
        iter = running->head;
        if (((Task*)iter->data)->taskID == pID) {
            printf("Task %d is running (remaining_time = %d).\n", pID, ((Task*)iter->data)->TTK);
            srcRes = iter;
            flag = 1;
            break;
        }
        dequeue(running, temp);
    }
    while (temp->size > 0)
        enqueue(running, temp);

    while (finished->size > 0) {
        iter = finished->head;
        if (((Task*)iter->data)->taskID == pID) {
            printf("Task %d is finished (remaining_time = %d).\n", pID, ((Task*)iter->data)->TTK);
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
        printf("Warn: temporary stack is not empty at the end of the function getTask\n");
    free(temp);
    return srcRes;
}

// caut in thPool, daca nu apare, atunci caut in running, pentru fiecare task, daca apartine de acel thread
Node* getThread(Stack* thPool, Queue* running, uint16_t pID) {
    Stack* temp = createStack();
    Node* node = NULL;

    temp->head = temp->tail = thPool->head;
    thPool->head = thPool->head->next;
    if (((Thread*)temp->head->data)->thID == pID) {
        // punem la loc pe raft
        Node* aux = thPool->head;
        thPool->head = temp->head;
        temp->head->next = aux;
        free(temp);
        printf("Thread %hd is idle.\n", ((Thread*)thPool->head->data)->thID);
        return thPool->head;
    }
    temp->size += 1;
    thPool->size -= 1;

    Node* val = thPool->head;
    while (val != NULL && thPool->size >= 0) {
        Thread* data = val->data;
        if (data->thID == pID) {
            node = val;
            printf("Thread %hd is idle.\n", ((Thread*)thPool->head->data)->thID);
            break;
        }
        Node* aux = temp->head;
        temp->head = thPool->head;
        val = thPool->head->next;
        temp->head->next = aux;
        // thPool->head = val->next;
        thPool->head = val;
        temp->size += 1; thPool->size -= 1;
    }

    if (thPool->size == 0) {
        // nu am gasit nimic in pool(a)
        // cautam acum in running, dupa ce refacem stiva
        val = temp->head;
        while (val != NULL) {
            Node* aux = thPool->head;
            thPool->head = temp->head;
            thPool->head->next = aux;
            temp->head = temp->head->next;
            val = temp->head;
            temp->size -= 1; thPool->size += 1;
        }
    }
    
    return node;
}

// atribuirea informatiei pe alta data, acum doar alocarea
void addNodeInOrder(void* dest, Node* node) {
    // todo inlocuire bucati de mutare noduri cu enqueue/dequeue
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
    for (Node* top = aux->head; top != NULL && counter < que->size; ) {
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
        counter++;
    }
    free(aux);
    return;
}

// 🤡
void printQueue(Queue* que, const int8_t qType) {
    switch (qType) {
        case 1:
            printf("====== Waiting queue ======\n");
            break;
        case 2:
            printf("====== Running in parallel ======\n");
            break;
        case 3:
            printf("====== Finished queue ======\n");
            break;
        default: break;
    }

    Stack* temp = createStack();
    if (que->size == 0)
        return;
    
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
    if (AT->TTK > BT->TTK)
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
    }

    char* cmd = calloc(S1, sizeof(char));
    while (scanf("%s", cmd) > 0) {
        if (!strcmp(cmd, "add_tasks")) {
            extern int32_t taskN;
            int16_t num = 0, priority = 0;
            int32_t TTK = 0;
            int8_t sign = scanf("%hd%d%hd", &num, &TTK, &priority);
            for (int i = taskN + 1; i <= taskN + num; i++) {
                Node* node = createNode(waiting, 2);
                node->data = calloc(1, sizeof(Task));
                ((Task*)node->data)->priority = priority;
                ((Task*)node->data)->state = 0;
                ((Task*)node->data)->taskID = i;
                ((Task*)node->data)->thID = -1;
                ((Task*)node->data)->TTK = TTK;
                ((Task*)node->data)->clk = 0;
                addNodeInOrder(waiting, node);
                printf("Task created successfully : ID %d.\n", ((Task*)node->data)->taskID);
            }
            taskN += num;
        }
        if (!strcmp(cmd, "get_task")) {
            int32_t pID = 0;
            int8_t sign = scanf("%d", &pID);
            Node* srcRes = getTask(waiting, running, finished, pID);
        }
        if (!strcmp(cmd, "get_thread")) {
            int32_t pID = 0;
            int8_t sign = scanf("%d", &pID);
            if (pID >= N)
                printf("Thread %d not found.\n", pID);
            else {
                Node* srcRes = getThread(thPool, running, pID);
            }
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
            int32_t vT = 0;
            int8_t sign = scanf("%d", &vT);
            Stack* temp = createStack();
            run(vT, Q, N, thPool, waiting, running, finished, temp);
        }
    }

    return;
}

// fiecare iteratie a functiei va acoperi bucata de timp ceruta
// va trebui mai folosesc o stiva pentru a pune threadurile folosite
// nu e nevoie sa o initializam din functia parser pentru ca la finalul
// unei iteratii, toate threadurile ajung din nou in pool
void run(volatile int32_t T, const int32_t Q, const int32_t N,
         Stack* thPool, Queue* waiting, Queue* running, Queue* finished, Stack* temp) {

    Stack* tempQ = createStack();
    Stack* tempQ2 = createStack();
    if (waiting->size == 0 && running->size == 0) {
        free(temp);
        return;
    }
    
    // aici incepem daca mai avem query-uri
    Node* nodeTh = thPool->head, * nodeTask = waiting->head;
    //printf("%d\n", T);
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
        Node* ranTask = running->tail;
        // setam valorile de mediu pentru thread
        ((Thread*)temp->head->data)->state = 1; // running
        ((Thread*)temp->head->data)->taskID = ((Task*)ranTask->data)->taskID;

        // setam valorile de mediu pentru task
        // (sunt cam multe, brace yourself)
        Task* value = ranTask->data;
        value->clk += Q * (Q <= T) + T * (Q > T);
        value->state = 1; // running
        value->thID = ((Thread*)temp->head->data)->thID;

        // aici s-a terminat partea de pus sclavii la munca
        // acum verificam sclavii deja assignati daca si-au terminat treaba
        // daca da, le luam treaba terminata si o punem in finished
        // si le dam alte treburi

        if CLKCheck(ranTask->data) {
            printf("sclavie incheiata cu succes la taskID %d\n", ((Task*)ranTask->data)->taskID);
            dequeue(running, tempQ2);
            enqueue(finished, tempQ2);
        
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
    
    if (T >= Q)
        run(T - Q, Q, N, thPool, waiting, running, finished, temp);
    // hehe
    while (0);
}