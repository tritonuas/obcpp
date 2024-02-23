#ifndef PACKET_QUEUE_H_
#define PACKET_QUEUE_H_

#include "airdrop/packet.h"

#include <stdlib.h>

/* 
 * Implementation of a queue for the mock airdrop sockets code. Essentially,
 * the mock sockets are using these queues in the background to "pass" around
 * messages on the local system.
 */

#define MAX_PACKETS 100

typedef struct packet_queue {
    ad_packet_t _arr[MAX_PACKETS];
    size_t _front; // index of first element in the queue
    size_t _back;  // index of next element in the queue
    size_t _num_elems;
} packet_queue_t;

// initialize the queue to have 0 items
void pqueue_init(packet_queue_t* queue);

// check whether queue is empty or not
int pqueue_empty(packet_queue_t* queue);

// precondition: queue is not empty
void pqueue_push(packet_queue_t* queue, ad_packet_t packet);

// remove front packet from queue and return
ad_packet_t pqueue_pop(packet_queue_t* queue);

#endif  // PACKET_QUEUE_H_