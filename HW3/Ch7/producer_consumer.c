/*
 * Producer-Consumer Problem
 * Using counting semaphores (empty, full) and mutex lock
 * Compiled with: gcc -o producer_consumer a.c -lpthread
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

/* Buffer size */
#define BUFFER_SIZE 5

/* Number of producer and consumer threads */
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 2

/* Number of items to produce/consume */
#define NUM_ITEMS 10

/* Buffer structure */
typedef struct {
    int buffer[BUFFER_SIZE];
    int in;  /* Index where next item will be inserted */
    int out; /* Index where next item will be removed */
} Buffer;

/* Global variables */
Buffer buffer;
pthread_mutex_t mutex;
sem_t empty;  /* Semaphore for empty slots */
sem_t full;   /* Semaphore for full slots */

/* Function to insert an item into the buffer */
void insert_item(int item) {
    buffer.buffer[buffer.in] = item;
    buffer.in = (buffer.in + 1) % BUFFER_SIZE;
}

/* Function to remove an item from the buffer */
int remove_item() {
    int item = buffer.buffer[buffer.out];
    buffer.out = (buffer.out + 1) % BUFFER_SIZE;
    return item;
}

/* Producer thread function */
void* producer(void* arg) {
    int producer_id = (intptr_t)arg;
    
    for (int i = 0; i < NUM_ITEMS; i++) {
        int item = producer_id * 100 + i;
        
        /* Wait for empty slot */
        sem_wait(&empty);
        
        /* Acquire mutex before accessing buffer */
        pthread_mutex_lock(&mutex);
        
        insert_item(item);
        printf("[Producer %d] Produced item: %d\n", producer_id, item);
        
        /* Release mutex */
        pthread_mutex_unlock(&mutex);
        
        /* Signal that a slot is now full */
        sem_post(&full);
        
        /* Simulate production time */
        sleep(1);
    }
    
    pthread_exit(NULL);
}

/* Consumer thread function */
void* consumer(void* arg) {
    int consumer_id = (intptr_t)arg;
    
    for (int i = 0; i < NUM_ITEMS; i++) {
        /* Wait for full slot */
        sem_wait(&full);
        
        /* Acquire mutex before accessing buffer */
        pthread_mutex_lock(&mutex);
        
        int item = remove_item();
        printf("[Consumer %d] Consumed item:  %d\n", consumer_id, item);
        
        /* Release mutex */
        pthread_mutex_unlock(&mutex);
        
        /* Signal that a slot is now empty */
        sem_post(&empty);
        
        /* Simulate consumption time */
        sleep(2);
    }
    
    pthread_exit(NULL);
}

int main() {
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    
    /* Initialize buffer */
    buffer.in = 0;
    buffer.out = 0;
    
    /* Initialize mutex */
    pthread_mutex_init(&mutex, NULL);
    
    /* Initialize semaphores */
    sem_init(&empty, 0, BUFFER_SIZE);  /* Initially all slots are empty */
    sem_init(&full, 0, 0);             /* Initially no slots are full */
    
    printf("=== Producer-Consumer Problem ===\n");
    printf("Buffer Size: %d\n", BUFFER_SIZE);
    printf("Producers: %d, Consumers: %d\n", NUM_PRODUCERS, NUM_CONSUMERS);
    printf("Items per thread: %d\n\n", NUM_ITEMS);
    
    /* Create producer threads */
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_create(&producers[i], NULL, producer, (void*)(intptr_t)i);
    }
    
    /* Create consumer threads */
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_create(&consumers[i], NULL, consumer, (void*)(intptr_t)i);
    }
    
    /* Wait for all producer threads to complete */
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }
    
    /* Wait for all consumer threads to complete */
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }
    
    /* Clean up */
    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);
    
    printf("\n=== All threads completed ===\n");
    
    return 0;
}
