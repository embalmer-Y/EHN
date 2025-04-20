/*
 * Dynamic Producer-Consumer MPSC Queue Example
 * Features:
 * 1. Dynamically managed producer count
 * 2. Continuous random data generation
 * 3. User-controlled graceful exit (Ctrl+C)
 * 4. Safe resource cleanup with remaining data handling
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>

// Global atomic exit flag
static _Atomic bool g_running = true;

// ------------------------------ Queue Data Structures ------------------------------
typedef struct Node {
    int data;                   // Stores the actual data
    _Atomic(struct Node*) next; // Atomic pointer to next node
} Node;

typedef struct {
    _Atomic(Node*) head;         // Consumer dequeue pointer
    _Atomic(Node*) tail;         // Producer enqueue pointer
    Node* dummy;                 // Sentinel node for empty queue handling
    _Atomic(int) producer_count; // Active producer counter
} MPSCQueue;

// ------------------------------ Signal Handling ------------------------------
// Handles SIGINT (Ctrl+C) for graceful exit
void handle_exit(int signum) {
    if (signum == SIGINT) {
        printf("\nReceived exit signal. Performing graceful shutdown...\n");
        atomic_store(&g_running, false);
    }
}

// ------------------------------ Queue Operations ------------------------------
// Initialize MPSC queue
void mpsc_queue_init(MPSCQueue* q) {
    q->dummy = malloc(sizeof(Node));
    if (!q->dummy) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    q->dummy->data = 0;
    atomic_init(&q->dummy->next, NULL);
    atomic_init(&q->head, q->dummy);
    atomic_init(&q->tail, q->dummy);
    atomic_init(&q->producer_count, 0); // Initialize with 0 active producers
}

// Enqueue operation (thread-safe for multiple producers)
void mpsc_enqueue(MPSCQueue* q, int data) {
    Node* new_node = malloc(sizeof(Node));
    if (!new_node) {
        perror("malloc failed");
        return;
    }
    new_node->data = data;
    atomic_init(&new_node->next, NULL);

    Node* old_tail = atomic_exchange(&q->tail, new_node);
    atomic_store(&old_tail->next, new_node); // Release semantics
}

// Dequeue operation (single consumer only)
int* mpsc_dequeue(MPSCQueue* q) {
    Node* head = atomic_load(&q->head);
    Node* next = atomic_load(&head->next);

    while (next == NULL) {
        // Exit condition: exit signal received and all producers terminated
        if (!atomic_load(&g_running) && atomic_load(&q->producer_count) == 0) {
            return NULL;
        }
        usleep(1000); // Short sleep to reduce CPU usage
        next = atomic_load(&head->next);
    }

    int* data = malloc(sizeof(int));
    *data = next->data;
    atomic_store(&q->head, next); // Acquire semantics
    free(head);
    return data;
}

// Clean up queue resources
void mpsc_queue_destroy(MPSCQueue* q) {
    Node* current = atomic_load(&q->head);
    while (current != NULL) {
        Node* next = atomic_load(&current->next);
        free(current);
        current = next;
    }
}

// ------------------------------ Thread Functions ------------------------------
// Producer thread: continuously generates random data until exit signal
void* producer_thread(void* arg) {
    MPSCQueue* q = (MPSCQueue*)arg;
    int producer_id = (int)(intptr_t)pthread_self() % 1000; // Simplified ID generation

    // Increment active producer count
    atomic_fetch_add(&q->producer_count, 1);
    printf("Producer %d started (active: %d)\n", 
           producer_id, atomic_load(&q->producer_count));

    while (atomic_load(&g_running)) {
        // Generate random data (1-1000) and random delay (100-1000ms)
        int data = rand() % 1000 + 1;
        int delay_ms = rand() % 900 + 100; // Random delay between 100-1000ms

        mpsc_enqueue(q, data);
        printf("Producer %d generated data: %d (delay: %dms)\n", producer_id, data, delay_ms);
        
        usleep(delay_ms * 1000); // Convert ms to microseconds
    }

    // Decrement active producer count
    atomic_fetch_sub(&q->producer_count, 1);
    printf("Producer %d exited (active: %d)\n", 
           producer_id, atomic_load(&q->producer_count));
    return NULL;
}

// Consumer thread: processes data until exit signal and all producers terminated
void* consumer_thread(void* arg) {
    MPSCQueue* q = (MPSCQueue*)arg;
    int processed_count = 0;

    printf("Consumer started\n");
    // Continue processing while: running flag is true OR producers are still active
    while (atomic_load(&g_running) || atomic_load(&q->producer_count) > 0) {
        int* data = mpsc_dequeue(q);
        if (data) {
            processed_count++;
            printf("Consumer processed data: %d (total: %d)\n", *data, processed_count);
            free(data);
        } else {
            usleep(10000); // Longer sleep when queue is empty
        }
    }

    // Process remaining data after exit signal
    printf("Consumer processing remaining data...\n");
    while (1) {
        int* data = mpsc_dequeue(q);
        if (!data) break;
        processed_count++;
        printf("Consumer processed remaining data: %d (total: %d)\n", *data, processed_count);
        free(data);
    }

    printf("Consumer exited. Total processed: %d items\n", processed_count);
    return NULL;
}

// ------------------------------ Main Function ------------------------------
int main() {
    srand(time(NULL));
    MPSCQueue q;
    pthread_t consumer, producers[3]; // Create 3 producer threads in this example

    // Initialize signal handler for Ctrl+C
    signal(SIGINT, handle_exit);
    mpsc_queue_init(&q);

    // Create consumer thread
    if (pthread_create(&consumer, NULL, consumer_thread, &q) != 0) {
        perror("pthread_create failed");
        return 1;
    }

    // Create producer threads
    for (int i = 0; i < 3; i++) {
        if (pthread_create(&producers[i], NULL, producer_thread, &q) != 0) {
            perror("pthread_create failed");
            // Cleanup if thread creation fails
            atomic_store(&g_running, false);
            for (int j = 0; j < i; j++) {
                pthread_join(producers[j], NULL);
            }
            pthread_join(consumer, NULL);
            mpsc_queue_destroy(&q);
            return 1;
        }
    }

    // Wait for all producer threads to complete
    for (int i = 0; i < 3; i++) {
        pthread_join(producers[i], NULL);
    }

    // Wait for consumer to finish processing remaining data
    pthread_join(consumer, NULL);

    // Clean up resources
    mpsc_queue_destroy(&q);
    printf("Program exited successfully\n");
    return 0;
}
