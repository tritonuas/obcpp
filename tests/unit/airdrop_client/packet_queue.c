// LINT_C_FILE

#include "packet_queue.h"
#include "airdrop/packet.h"

// initialize the queue to have 0 items
void pqueue_init(packet_queue_t* queue) {
    queue->_front = 0;
    queue->_back = 0;
    queue->_num_elems = 0;
}

// check whether queue is empty or not
int pqueue_empty(packet_queue_t* queue) {
    return queue->_num_elems == 0;
}

// precondition: queue is not empty
void pqueue_push(packet_queue_t* queue, ad_packet_t packet) {
    queue->_arr[queue->_back] = packet;
    queue->_back++;
    if (queue->_back == MAX_PACKETS) {
        queue->_back = 0;
    }
    queue->_num_elems++;
}

// remove front packet from queue and return
ad_packet_t pqueue_pop(packet_queue_t* queue) {
    ad_packet_t packet = queue->_arr[queue->_front];
    queue->_front++;
    if (queue->_front == MAX_PACKETS) {
        queue->_front = 0;
    }
    queue->_num_elems--;
    return packet;
}