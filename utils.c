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

void printWaiting(Queue* que) {
    printf("====== Waiting queue ======\n");
    Stack* temp = createStack();
    if (que->size == 0)
        return;
    
    printf("[");
    while (que->size > 0) {
        Node* iter = que->head;
        Task* tIter = iter->data;
        printf("(%d: priority = %d, remaining_time = %d)", tIter->taskID, tIter->priority, tIter->TTK);
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

        st->tail->next = node;
        st->tail = node;
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
    Stack* thPool = createStack();
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
        if (!strcmp(cmd, "print")) {
            int8_t sign = scanf(" %s", cmd);
            if (!strcmp(cmd, "waiting"))
                printWaiting(waiting);
        }
    }


    return;
}
