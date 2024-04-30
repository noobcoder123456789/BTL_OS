#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
        return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
        
        /* Our group's code */
        if(q->size >= MAX_QUEUE_SIZE) {
                printf("Queue is full!\n");
                return;
        }

        int prio_position, i;
        for(prio_position = 0; prio_position < q->size && q->proc[prio_position]->prio <= proc->prio; prio_position++);
        for(i = q->size; i > prio_position; q->proc[i] = q->proc[i - 1], i--);

        q->proc[prio_position] = proc;
        q->size++;
        /* Our group's code */
}

struct pcb_t * dequeue(struct queue_t * q) {
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */

        /* Our group's code */
        if(!q || empty(q)) {
                printf("Queue is empty!\n");
                return NULL;
        }

        struct pcb_t *highest_pcb_priority = q->proc[0];
        for(int i = 0; i + 1 < q->size; q->proc[i] = q->proc[i + 1], i ++);
        q->size--;
	return highest_pcb_priority;
        /* Our group's code */
}

