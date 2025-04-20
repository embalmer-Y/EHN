#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"
#include "unistd.h"
#include "stdint.h"
#include "string.h"
#include "time.h"

// Simulate NAPI pattern: Multiple interrupt sources (producers) + One bottom-half thread (consumer)
// Core mechanism: Event Queue (FIFO) + Aggregate Condition Variable (notify bottom-half)

#define MAX_PRODUCERS 3       // Number of interrupt sources
#define QUEUE_SIZE 10         // Event queue capacity
#define NAPI_BUDGET 2         // NAPI budget: max events processed per poll
#define SIMULATE_DURATION 15  // Simulation runtime in seconds

// Interrupt source (producer) structure
typedef struct {
    int id;                   // Interrupt source ID
    int data;                 // Generated data
    int irq_enabled;          // Interrupt enable flag (simulate NAPI interrupt suppression)
    pthread_mutex_t irq_mutex;// Mutex for interrupt source synchronization
    pthread_cond_t irq_cond;  // Condition variable (wait for processing completion)
} IrqSource;

// Event queue structure (records interrupt trigger order)
typedef struct {
    IrqSource* items[QUEUE_SIZE];  // Event queue (stores interrupt source pointers)
    int front;                 // Queue front pointer
    int rear;                  // Queue rear pointer
    int count;                 // Number of events in queue
    pthread_mutex_t q_mutex;   // Queue mutex
    pthread_cond_t q_cond;     // Queue condition variable (aggregate notification)
} EventQueue;

// Global variables
EventQueue event_queue;
IrqSource irq_sources[MAX_PRODUCERS];
int simulation_running = 1;    // Simulation running flag

// Initialize event queue
void init_event_queue() {
    event_queue.front = 0;
    event_queue.rear = 0;
    event_queue.count = 0;
    pthread_mutex_init(&event_queue.q_mutex, NULL);
    pthread_cond_init(&event_queue.q_cond, NULL);
}

// Initialize interrupt sources
void init_irq_sources() {
    for (int i = 0; i < MAX_PRODUCERS; i++) {
        irq_sources[i].id = i + 1;
        irq_sources[i].data = 0;
        irq_sources[i].irq_enabled = 1;  // Enable interrupt by default
        pthread_mutex_init(&irq_sources[i].irq_mutex, NULL);
        pthread_cond_init(&irq_sources[i].irq_cond, NULL);
    }
}

// Enqueue event (called by producer)
int enqueue_event(IrqSource* irq) {
    pthread_mutex_lock(&event_queue.q_mutex);
    
    // Check if queue is full
    if (event_queue.count >= QUEUE_SIZE) {
        pthread_mutex_unlock(&event_queue.q_mutex);
        printf("Warning: Event queue full, dropping event from IRQ source %d\n", irq->id);
        return -1;
    }
    
    // Enqueue and update queue status
    event_queue.items[event_queue.rear] = irq;
    event_queue.rear = (event_queue.rear + 1) % QUEUE_SIZE;
    event_queue.count++;
    printf("IRQ source %d: Event enqueued (current queue size: %d)\n", irq->id, event_queue.count);
    
    // Notify consumer of new event
    pthread_cond_signal(&event_queue.q_cond);
    pthread_mutex_unlock(&event_queue.q_mutex);
    return 0;
}

// Dequeue event (called by consumer)
IrqSource* dequeue_event() {
    pthread_mutex_lock(&event_queue.q_mutex);
    
    // Wait for non-empty queue (prevent spurious wakeup)
    while (event_queue.count == 0 && simulation_running) {
        pthread_cond_wait(&event_queue.q_cond, &event_queue.q_mutex);
    }
    
    // Check if awakened due to simulation end
    if (!simulation_running) {
        pthread_mutex_unlock(&event_queue.q_mutex);
        return NULL;
    }
    
    // Dequeue and update queue status
    IrqSource* irq = event_queue.items[event_queue.front];
    event_queue.front = (event_queue.front + 1) % QUEUE_SIZE;
    event_queue.count--;
    printf("Event dequeued: IRQ source %d (remaining in queue: %d)\n", irq->id, event_queue.count);
    
    pthread_mutex_unlock(&event_queue.q_mutex);
    return irq;
}

// Simulate interrupt handler (bottom-half processing)
void process_irq_event(IrqSource* irq) {
    pthread_mutex_lock(&irq->irq_mutex);
    printf("\n=== Starting IRQ source %d event processing ===\n", irq->id);
    printf("Processing data: %d\n", irq->data);
    usleep(50000);  // Simulate processing delay (50ms)
    printf("=== IRQ source %d event processing completed ===\n", irq->id);
    
    // Re-enable interrupt (simulate NAPI interrupt restoration after processing)
    irq->irq_enabled = 1;
    pthread_cond_signal(&irq->irq_cond);  // Notify IRQ source it can trigger again
    pthread_mutex_unlock(&irq->irq_mutex);
}

// Producer thread (simulate interrupt source)
void* irq_source_thread(void* arg) {
    IrqSource* irq = (IrqSource*)arg;
    printf("IRQ source %d thread started (initial state: enabled)\n", irq->id);
    
    while (simulation_running) {
        // Random delay (1-3 seconds) to simulate interrupt trigger interval
        sleep(rand() % 3 + 1);
        
        pthread_mutex_lock(&irq->irq_mutex);
        if (irq->irq_enabled) {
            // Generate random data
            irq->data = rand() % 1000;
            
            // Disable interrupt (simulate NAPI interrupt suppression)
            irq->irq_enabled = 0;
            printf("\nIRQ source %d triggered (data: %d), disabling self-interrupt\n", irq->id, irq->data);
            
            // Enqueue event
            if (enqueue_event(irq) == 0) {
                // Wait for event processing completion (simulate NAPI poll loop)
                pthread_cond_wait(&irq->irq_cond, &irq->irq_mutex);
            } else {
                // Re-enable interrupt if enqueue failed
                irq->irq_enabled = 1;
            }
        }
        pthread_mutex_unlock(&irq->irq_mutex);
    }
    
    printf("IRQ source %d thread exiting\n", irq->id);
    return NULL;
}

// Consumer thread (simulate interrupt bottom-half/NAPI poll thread)
void* napi_poll_thread(void* arg) {
    int processed;
    printf("\nNAPI bottom-half thread started (budget: %d)\n", NAPI_BUDGET);
    
    while (simulation_running) {
        processed = 0;
        printf("\n--- NAPI polling started (budget: %d) ---\n", NAPI_BUDGET);
        
        // Batch process events according to budget
        while (processed < NAPI_BUDGET) {
            IrqSource* irq = dequeue_event();
            if (!irq) break;  // Queue empty or simulation ended
            
            process_irq_event(irq);
            processed++;
        }
        
        printf("--- NAPI polling ended, events processed: %d ---\n", processed);
        
        // If queue still has events, poll again immediately; otherwise sleep briefly
        pthread_mutex_lock(&event_queue.q_mutex);
        if (event_queue.count == 0) {
            pthread_mutex_unlock(&event_queue.q_mutex);
            usleep(10000);  // Short sleep to reduce CPU usage
        } else {
            pthread_mutex_unlock(&event_queue.q_mutex);
        }
    }
    
    printf("NAPI bottom-half thread exiting\n");
    return NULL;
}

int main() {
    pthread_t irq_threads[MAX_PRODUCERS], napi_thread;
    time_t start_time;
    
    // Initialize random seed
    srand(time(NULL));
    
    // Initialize synchronization structures
    init_event_queue();
    init_irq_sources();
    
    // Create interrupt source threads
    for (int i = 0; i < MAX_PRODUCERS; i++) {
        pthread_create(&irq_threads[i], NULL, irq_source_thread, &irq_sources[i]);
    }
    
    // Create NAPI bottom-half thread
    pthread_create(&napi_thread, NULL, napi_poll_thread, NULL);
    
    // Run simulation for specified duration
    start_time = time(NULL);
    while (time(NULL) - start_time < SIMULATE_DURATION) {
        sleep(1);
    }
    
    // Stop simulation
    simulation_running = 0;
    printf("\nSimulation time ended, starting cleanup...\n");
    
    // Wake blocked threads
    pthread_mutex_lock(&event_queue.q_mutex);
    pthread_cond_broadcast(&event_queue.q_cond);
    pthread_mutex_unlock(&event_queue.q_mutex);
    
    for (int i = 0; i < MAX_PRODUCERS; i++) {
        pthread_mutex_lock(&irq_sources[i].irq_mutex);
        pthread_cond_signal(&irq_sources[i].irq_cond);
        pthread_mutex_unlock(&irq_sources[i].irq_mutex);
    }
    
    // Wait for all threads to exit
    for (int i = 0; i < MAX_PRODUCERS; i++) {
        pthread_join(irq_threads[i], NULL);
    }
    pthread_join(napi_thread, NULL);
    
    // Cleanup resources
    pthread_mutex_destroy(&event_queue.q_mutex);
    pthread_cond_destroy(&event_queue.q_cond);
    for (int i = 0; i < MAX_PRODUCERS; i++) {
        pthread_mutex_destroy(&irq_sources[i].irq_mutex);
        pthread_cond_destroy(&irq_sources[i].irq_cond);
    }
    
    printf("All resources cleaned up, program exiting\n");
    return 0;
}
