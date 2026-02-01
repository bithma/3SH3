#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

int amount = 0;

pthread_mutex_t mutex;

// Semaphores
sem_t can_deposit;   // signals when deposit is allowed (amount < 400)
sem_t can_withdraw;  // signals when withdraw is allowed (amount > 0)

void* deposit(void* param) {
    int deposit_amount = *((int*)param);
    
    printf("Executing deposit function\n");
    
    // Wait on semaphore
    sem_wait(&can_deposit);
    
    // enter CS
    pthread_mutex_lock(&mutex);
    
    // CS 
    amount += deposit_amount;
    printf("Amount after deposit = %d\n", amount);
    
    // Check if withdraw allowed (amount > 0)
    if (amount > 0) {
        sem_post(&can_withdraw);
    }
    
    // Check if deposit allowed (amount < 400)
    if (amount < 400) {
        sem_post(&can_deposit);
    }
    
    // Exit CS
    pthread_mutex_unlock(&mutex);
    
    pthread_exit(0);
}

void* withdraw(void* param) {
    int withdraw_amount = *((int*)param);
    
    printf("Executing withdraw function\n");
    
    sem_wait(&can_withdraw);
    
    pthread_mutex_lock(&mutex);
    
    amount -= withdraw_amount;
    printf("Amount after Withdrawal = %d\n", amount);
    
    if (amount > 0) {
        sem_post(&can_withdraw);
    }
    
    if (amount < 400) {
        sem_post(&can_deposit);
    }
    
    pthread_mutex_unlock(&mutex);
    
    pthread_exit(0);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <amount>\n", argv[0]);
        return 1;
    }
    
    int transaction_amount = atoi(argv[1]);
    
    // Validate
    if (transaction_amount != 100) {
        fprintf(stderr, "Error: Amount must be 100\n");
        return 1;
    }
    
    // 7 deposit threads 3 withdraw threads
    pthread_t threads[10];
    
    // Initialize mutex lock
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        fprintf(stderr, "Error: Mutex initialization failed\n");
        return 1;
    }
    
    // Initialize semaphores
    // can_deposit --> set to 4 because we can deposit until amount gets to 400
    // can_withdraw --> set to 0 because amount initialized to 0 so no withdrawals allowed
    if (sem_init(&can_deposit, 0, 4) != 0) {
        fprintf(stderr, "Error: can_deposit semaphore initialization failed\n");
        return 1;
    }
    
    if (sem_init(&can_withdraw, 0, 0) != 0) {
        fprintf(stderr, "Error: can_withdraw semaphore initialization failed\n");
        return 1;
    }
    
    // 7 deposit threads
    for (int i = 0; i < 7; i++) {
        if (pthread_create(&threads[i], NULL, deposit, &transaction_amount) != 0) {
            fprintf(stderr, "Error: Failed to create deposit thread %d\n", i);
            return 1;
        }
    }
    
    // 3 withdraw threads
    for (int i = 7; i < 10; i++) {
        if (pthread_create(&threads[i], NULL, withdraw, &transaction_amount) != 0) {
            fprintf(stderr, "Error: Failed to create withdraw thread %d\n", i);
            return 1;
        }
    }
    
    // Wait for threads
    for (int i = 0; i < 10; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Error: Failed to join thread %d\n", i);
            return 1;
        }
    }
    
    // Destroy semaphores
    if (sem_destroy(&can_deposit) != 0) {
        fprintf(stderr, "Error: can_deposit semaphore destruction failed\n");
        return 1;
    }
    
    if (sem_destroy(&can_withdraw) != 0) {
        fprintf(stderr, "Error: can_withdraw semaphore destruction failed\n");
        return 1;
    }
    
    // Destroy mutex lock
    if (pthread_mutex_destroy(&mutex) != 0) {
        fprintf(stderr, "Error: Mutex destruction failed\n");
        return 1;
    }
    
    printf("Final amount = %d\n", amount);
    
    return 0;
}