// LINT_C_FILE

#include "network/mock/packet_queue.h"

#include <semaphore.h>
#include <stdio.h>

#include "udp_squared/protocol.h"

void pqueue_init(packet_queue_t* q) {
    q->_front = 0;
    q->_back = 0;
    q->_num_elems = 0;

    // not trying to analyze at the error output of these...
    // these should just work (famous last words)
    sem_init(&q->_mutex, 0, 1);     // 1 => enforce mutual exclusion
    q->_num_waiting_for_recv = 0;   // start off no one waiting on recv_sem
    sem_init(&q->_recv_sem, 0, 0);  // 0 => any wait will block
}

int pqueue_empty(packet_queue_t* q) {
    sem_wait(&q->_mutex);
    int is_empty = q->_num_elems == 0;
    sem_post(&q->_mutex);
    return is_empty;
}

int pqueue_full(packet_queue_t* q) {
    sem_wait(&q->_mutex);
    int is_full = q->_num_elems == MAX_PACKETS;
    sem_post(&q->_mutex);
    return is_full;
}

void pqueue_push(packet_queue_t* q, packet_t packet) {
    sem_wait(&q->_mutex);
    q->_arr[q->_back] = packet;
    q->_back++;
    if (q->_back == MAX_PACKETS) {
        q->_back = 0;
    }
    q->_num_elems++;

    if (q->_num_waiting_for_recv > 0) {
        q->_num_waiting_for_recv--;
        sem_post(&q->_recv_sem);
    }

    sem_post(&q->_mutex);
}

packet_t pqueue_pop(packet_queue_t* q) {
    sem_wait(&q->_mutex);
    packet_t packet = q->_arr[q->_front];
    q->_front++;
    if (q->_front == MAX_PACKETS) {
        q->_front = 0;
    }
    q->_num_elems--;
    sem_post(&q->_mutex);
    return packet;
}


packet_t pqueue_wait_pop(packet_queue_t* q) {
    sem_wait(&q->_mutex);
    if (q->_num_elems == 0) {
        q->_num_waiting_for_recv++;
        sem_post(&q->_mutex);
        sem_wait(&q->_recv_sem);
    } else {
        sem_post(&q->_mutex);
    }
    return pqueue_pop(q);
}
