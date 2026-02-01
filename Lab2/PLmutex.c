#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// Shared data
int amount = 0;

// Mutex lock for CS (critical section)
pthread_mutex_t mutex;

void* deposit(void* param) {
    int deposit_amount = *((int*)param);
    
    // Enter CS
    pthread_mutex_lock(&mutex);
    
    // CS - modify shared data
    amount += deposit_amount;
    printf("Deposit amount = %d\n", amount);


    // Exit CS
    pthread_mutex_unlock(&mutex);
    
    pthread_exit(0);
}

void* withdraw(void* param) {
    int withdraw_amount = *((int*)param);
    
    pthread_mutex_lock(&mutex);
    
    amount -= withdraw_amount;
    printf("Withdrawal amount = %d\n", amount);
    
    pthread_mutex_unlock(&mutex);
    
    pthread_exit(0);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <deposit_amount> <withdraw_amount>\n", argv[0]);
        return 1;
    }
    
    int deposit_amount = atoi(argv[1]);
    int withdraw_amount = atoi(argv[2]);
    
    // validate
    if (deposit_amount <= 0 || withdraw_amount <= 0) {
        fprintf(stderr, "Error: Both deposit and withdraw amounts must be positive integers\n");
        return 1;
    }
    
    // Threading
    pthread_t deposit_threads[3];
    pthread_t withdraw_threads[3];
    
    // Initialize mutex lock
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        fprintf(stderr, "Error: Mutex initialization failed\n");
        return 1;
    }
    
    printf("Initial Balance: $%d\n", amount);
    
    // make 3 deposit threads
    for (int i = 0; i < 3; i++) {
        if (pthread_create(&deposit_threads[i], NULL, deposit, &deposit_amount) != 0) {
            fprintf(stderr, "Error: Failed to make deposit thread %d\n", i);
            return 1;
        }
    }
    
    // make 3 withdraw threads
    for (int i = 0; i < 3; i++) {
        if (pthread_create(&withdraw_threads[i], NULL, withdraw, &withdraw_amount) != 0) {
            fprintf(stderr, "Error: Failed to make withdraw thread %d\n", i);
            return 1;
        }
    }
    
    // Wait for deposit threads to finsih
    for (int i = 0; i < 3; i++) {
        if (pthread_join(deposit_threads[i], NULL) != 0) {
            fprintf(stderr, "Error: Failed to join deposit thread %d\n", i);
            return 1;
        }
    }
    
    // Wait for withdraw threads to finish
    for (int i = 0; i < 3; i++) {
        if (pthread_join(withdraw_threads[i], NULL) != 0) {
            fprintf(stderr, "Error: Failed to join withdraw thread %d\n", i);
            return 1;
        }
    }
    
    // kill mutex lock
    if (pthread_mutex_destroy(&mutex) != 0) {
        fprintf(stderr, "Error: Mutex destruction failed\n");
        return 1;
    }
    
    printf("\nAll operations completed.\n");
    printf("Final Balance: $%d\n", amount);
    
    return 0;
}