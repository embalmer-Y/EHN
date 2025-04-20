#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

// NAPI simulation structure using condition variable
typedef struct {
    pthread_cond_t cond;         // Condition variable for notification
    pthread_mutex_t lock;        // Mutex for synchronization
    pthread_t poll_thread;       // Polling thread
    pthread_t interrupt_thread;  // Interrupt simulation thread
    int budget;                  // Max packets per poll cycle
    int packet_count;            // Pending packets count
    int napi_enabled;            // NAPI enable flag
    int running;                 // Running flag for thread control
} NapiCondSimulation;

// Initialize NAPI simulation with condition variable
NapiCondSimulation* napi_cond_init(int budget) {
    NapiCondSimulation* napi = malloc(sizeof(NapiCondSimulation));
    if (!napi) return NULL;

    // Initialize condition variable and mutex
    if (pthread_cond_init(&napi->cond, NULL) != 0) {
        perror("pthread_cond_init failed");
        free(napi);
        return NULL;
    }

    if (pthread_mutex_init(&napi->lock, NULL) != 0) {
        perror("pthread_mutex_init failed");
        pthread_cond_destroy(&napi->cond);
        free(napi);
        return NULL;
    }

    napi->budget = budget;
    napi->packet_count = 0;
    napi->napi_enabled = 1;  // Enable NAPI initially
    napi->running = 1;       // Set running flag

    return napi;
}

// Simulate packet processing
void process_packet(NapiCondSimulation* napi, int packet_id) {
    printf("Processing packet #%d\n", packet_id);
    usleep(10000);  // Simulate processing delay
}

// Poll thread function to simulate NAPI poll mechanism
void* poll_thread_func(void* arg) {
    NapiCondSimulation* napi = (NapiCondSimulation*)arg;
    int processed;
    int current_packet;

    printf("Poll thread started, budget=%d\n", napi->budget);

    while (napi->running) {
        pthread_mutex_lock(&napi->lock);
        
        // Wait for condition signal (spurious wakeup protected by while loop)
        while (napi->packet_count == 0 && napi->running) {
            pthread_cond_wait(&napi->cond, &napi->lock);
        }

        if (!napi->running) {
            pthread_mutex_unlock(&napi->lock);
            break;
        }

        // Disable NAPI (simulate interrupt off)
        napi->napi_enabled = 0;
        processed = 0;
        current_packet = napi->packet_count;

        printf("\n=== Condition signal received, starting poll ===\n");

        // Process packets with budget limit
        while (napi->packet_count > 0 && processed < napi->budget) {
            process_packet(napi, napi->packet_count);
            napi->packet_count--;
            processed++;
        }

        printf("=== Poll completed, processed %d packets, remaining %d ===\n", 
               processed, napi->packet_count);

        // Enable NAPI (simulate interrupt on)
        napi->napi_enabled = 1;
        pthread_mutex_unlock(&napi->lock);
    }

    printf("Poll thread exited\n");
    return NULL;
}

// Interrupt simulation thread to generate packets
void* interrupt_thread_func(void* arg) {
    NapiCondSimulation* napi = (NapiCondSimulation*)arg;
    int packet_id = 1;

    printf("Interrupt simulation thread started\n");

    while (napi->running) {
        // Simulate random packet arrival interval (1-3 seconds)
        sleep(rand() % 3 + 1);
        
        pthread_mutex_lock(&napi->lock);
        
        if (napi->napi_enabled) {
            // New packet arrival
            napi->packet_count++;
            printf("\n>>> Packet #%d arrived, total pending=%d\n", 
                   packet_id, napi->packet_count);
            
            // Signal poll thread
            if (pthread_cond_signal(&napi->cond) != 0) {
                perror("pthread_cond_signal failed");
            } else {
                printf(">>> Condition signal sent\n");
            }
            packet_id++;
        } else {
            // NAPI disabled, buffer the packet
            napi->packet_count++;
            printf("\n>>> NAPI disabled, packet #%d buffered, total pending=%d\n", 
                   packet_id, napi->packet_count);
            packet_id++;
        }
        
        pthread_mutex_unlock(&napi->lock);
    }

    printf("Interrupt thread exited\n");
    return NULL;
}

// Start NAPI simulation
int napi_cond_start(NapiCondSimulation* napi) {
    // Create poll thread
    if (pthread_create(&napi->poll_thread, NULL, poll_thread_func, napi) != 0) {
        perror("poll thread creation failed");
        return -1;
    }

    // Create interrupt simulation thread
    if (pthread_create(&napi->interrupt_thread, NULL, interrupt_thread_func, napi) != 0) {
        perror("interrupt thread creation failed");
        pthread_cancel(napi->poll_thread);
        return -1;
    }

    return 0;
}

// Stop and cleanup NAPI simulation resources
void napi_cond_cleanup(NapiCondSimulation* napi) {
    // Signal threads to exit
    pthread_mutex_lock(&napi->lock);
    napi->running = 0;
    pthread_cond_signal(&napi->cond);  // Wake up poll thread
    pthread_mutex_unlock(&napi->lock);

    // Join threads
    pthread_join(napi->poll_thread, NULL);
    pthread_join(napi->interrupt_thread, NULL);

    // Cleanup synchronization primitives
    pthread_cond_destroy(&napi->cond);
    pthread_mutex_destroy(&napi->lock);
    free(napi);
}

int main() {
    // Initialize NAPI simulation with budget 3
    NapiCondSimulation* napi = napi_cond_init(3);
    if (!napi) {
        fprintf(stderr, "NAPI condition variable simulation initialization failed\n");
        return 1;
    }

    // Start simulation
    if (napi_cond_start(napi) != 0) {
        fprintf(stderr, "NAPI condition variable simulation start failed\n");
        napi_cond_cleanup(napi);
        return 1;
    }

    // Run simulation for 15 seconds
    printf("Condition variable simulation running for 15 seconds...\n");
    sleep(60);

    // Cleanup and exit
    napi_cond_cleanup(napi);
    printf("Condition variable simulation ended\n");
    return 0;
}
