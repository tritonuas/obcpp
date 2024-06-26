#ifndef INCLUDE_NETWORK_MOCK_PACKET_QUEUE_H_
#define INCLUDE_NETWORK_MOCK_PACKET_QUEUE_H_

#include <stdlib.h>
#include <semaphore.h>

#include "udp_squared/protocol.h"

/* 
 * Implementation of a thread-safe queue for the mock airdrop sockets code. 
 * Essentially, the mock sockets are using these queues in the background to 
 * "pass" around messages on the local system.
 */

#define MAX_PACKETS 100

typedef struct packet_queue {
    packet_t _arr[MAX_PACKETS];
    size_t _front;    // index of first element in the queue
    size_t _back;     // index of next element in the queue
    size_t _num_elems;

    sem_t _mutex;     // mutex to read/write any of the vars here
    sem_t _recv_sem;  // wait on this when
    size_t _num_waiting_for_recv;  // number of threads blocked on _recv_sem
} packet_queue_t;

// initialize the queue to have 0 items
void pqueue_init(packet_queue_t* queue);

// check whether queue is empty or not
int pqueue_empty(packet_queue_t* queue);

// check whether queue is full or not
int pqueue_full(packet_queue_t* queue);

// precondition: queue is not empty
void pqueue_push(packet_queue_t* queue, packet_t packet);

// remove front packet from queue and return
packet_t pqueue_pop(packet_queue_t* queue);

// remove front packet from queue, but if the queue is empty
// then wait until there is something to pop
packet_t pqueue_wait_pop(packet_queue_t* queue);

#endif  // INCLUDE_NETWORK_MOCK_PACKET_QUEUE_H_
