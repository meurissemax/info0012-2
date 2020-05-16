/*
 * File: main.c
 * Authors: Maxime Meurisse & Valentin Vermeylen
 *
 * The main file of the project.
 *
 * Usage
 * -----
 * ./main (base) (size) (array)
 * example: ./main 10 5 4 54 21 32 3
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "headers/array.h"
#include "headers/communication.h"

/* ----- Union declaration ----- */
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short int *array;
    struct seminfo *__buf;
};

/* ----- Shared variables ----- */
long* shm_numbers;
long* shm_temp;
long* shm_sorted;

int sem_worker, sem_master;

int msgq_1, msgq_2;

/* ----- Prototypes ----- */
void worker(int id, long N, long base);
void master(long base, int iter, long N);

/* ----- Worker process ----- */
void worker(int id, long N, long base) {
    /* ----- Variable declaration ----- */
    // Message queue communication
    message msg;
    long write_pos;

    // Variable useful for execution
    long i, value, num, digit, divisor, nb, pos, begin, end;
    bool search;

    /* ----- Get worker informations ----- */
    search = true;

    // Where we must read in array.
    begin = floor(N / base) * id;
    end = floor(N / base) * (id + 1) - 1;

    divisor = 1;

    // Last worker
    if(id == base - 1)
        end = N - 1;

    if(N < base) {
        if(id >= N) // We are idle in the first step of execution.
            search = false;

        begin = id; // We have only one element to retrieve.
        end = id;
    }

    /* ----- Manipulation of the array ----- */
    while(shm_read(shm_sorted, 0) == 0) {
        if(search) {
            // Writing in the temporary array
            pos = begin; // to know where to write in temporary array, so it is not corrupted

            for(i = begin; i <= end; i++) {
                num = shm_read(shm_numbers, i);
                digit = (num / divisor) % base;

                shm_write(shm_temp, get_index(N, digit, pos), num);
                pos++;
            }
        }

        divisor *= base;

        // Semaphores management
        sem_unlock(sem_worker, 0);
        sem_lock(sem_master, 0);

        // Number of elements on the worker's line
        nb = 0;

        for(i = 0; i < N; i++)
            if(shm_read(shm_temp, get_index(N, id, i)) != -1)
                nb++;

        // Message queue communication
        msg.mtype = (long)(id + 1);
        msg.digit = (long)id;
        msg.num_numbers = nb;
        msg.write_pos = (long)0;

        msgq_send(msgq_1, &msg);
        msgq_read(msgq_2, (long)(id + 1), &msg); // Where to start writing back

        write_pos = msg.write_pos;

        // Writing in the main array
        for(i = 0; i < N; i++) {
            value = shm_read(shm_temp, get_index(N, id, i));

            if(value != -1) {
                shm_write(shm_numbers, write_pos, value);
                write_pos++;
            }
        }

        // Semaphores management
        sem_unlock(sem_worker, 0);
        sem_lock(sem_master, 1);
    }

    sem_unlock(sem_worker, 0);
}

/* ----- Master process ----- */
void master(long base, int iter, long N) {
    /* ----- Variable declaration ----- */
    // Message queue communication
    message* msg;

    // Variable useful for execution
    long i, j, to_write;

    /* ----- Process ----- */
    for(i = 0; i < iter; i++) {
        // We wait for all workers to put everything in the temporary array
        for(j = 0; j < base; j++)
            sem_lock(sem_worker, 0);

        // We signal them they can proceed to the next phase
        for(j = 0; j < base; j++)
            sem_unlock(sem_master, 0);

        msg = (message*)malloc(base * sizeof(message));

        if(msg == NULL) {
            printf("Error with malloc.\n");

            return;
        }

        for(j = 0; j < base; j++)
            msgq_read(msgq_1, (long)(j + 1), &msg[j]);

        // Find where each worker should start writing in the array
        to_write = 0;

        for(j = 0; j < base; j++) {
            msg[j].mtype = j + 1;
            msg[j].write_pos = to_write;
            to_write += msg[j].num_numbers;

            msgq_send(msgq_2, &msg[j]);
        }

        // We wait for everyone to write back in the main array
        for(j = 0; j < base; j++)
            sem_lock(sem_worker, 0);

        if(i == iter - 1) // Sorting is over
            shm_write(shm_sorted, 0, 1);

        // Reset the value of the temporary array
        for(j = 0; j < (long)get_size(base, N); j++)
            shm_write(shm_temp, j, -1);

        for(j = 0; j < base; j++)
            sem_unlock(sem_master, 1);
    }

    for(j = 0; j < base; j++)
        sem_lock(sem_worker, 0); // So no process will try to access an already deleted semaphore
}

/* ----- Main process ----- */
int main(int argc, char* argv[]) {
    /* --------------------------------------- */
    /* ---------- Preparation phase ---------- */
    /* --------------------------------------- */

    /* ----- Variable declaration ----- */
    // User parameters
    int N, base;
    long* numbers;
    char* endp;

    // Shared memory segments ID
    int id_numbers, id_temp, id_sorted;

    // Process management
    int id;
    pid_t pid;

    // Variable useful for execution
    int max, iter;
    long i, value;

    /* ----- Verification and get the user parameters ----- */
    // Check the number of parameters
    if(argc < 4) {
        printf("Not enough argument.\n");

        return EXIT_FAILURE;
    }

    // Retrieving parameter base
    base = strtol(argv[1], &endp, 10);

    if(errno != 0 || strlen(endp) > 0) {
        printf("The base is not a number or is too large.\n");

        return EXIT_FAILURE;
    }

    if(base <= 1) {
        printf("Base should be a positive number greater than 1.\n");

        return EXIT_FAILURE;
    }

    // Retrieving parameter N
    N = strtol(argv[2], &endp, 10);

    if(errno != 0 || strlen(endp) > 0) {
        printf("The size is not a number or is too large.\n");

        return EXIT_FAILURE;
    }

    if(N <= 0) {
        printf("The size should be a strictly positive number.\n");

        return EXIT_FAILURE;
    } else {
        if(N != (argc - 3)) {
            printf("The size should match the real size of the array.\n");

            return EXIT_FAILURE;
        }
    }

    // Allocating array of numbers
    numbers = (long*)malloc(N * sizeof(long));

    if(numbers == NULL) {
        printf("Problem with malloc.\n");

        return EXIT_FAILURE;
    }

    // Retrieving numbers to sort
    for(i = 0; i < N; i++) {
        numbers[i] = strtol(argv[i + 3], &endp, 10);

        if(errno != 0 || strlen(endp) > 0) {
            printf("An argument is not a number or is too large.\n");

            return EXIT_FAILURE;
        } else {
            if(numbers[i] < 0) {
                printf("An argument is not positive.\n");

                return EXIT_FAILURE;
            }
        }
    }

    /* ----- Creation of the shared memory elements ----- */
    // Array of numbers
    id_numbers = shm_create(N * sizeof(long), 'N');
    shm_numbers = shm_attach(id_numbers);

    for(i = 0; i < N; i++)
        shm_write(shm_numbers, i, numbers[i]);

    // Temporary array
    id_temp = shm_create(get_size(base, N) * sizeof(long), 'T');
    shm_temp = shm_attach(id_temp);

    for(i = 0; i < (long)get_size(base, N); i++)
        shm_write(shm_temp, i, -1);

    // Variable sorted
    id_sorted = shm_create(sizeof(long), 'S');
    shm_sorted = shm_attach(id_sorted);

    shm_write(shm_sorted, 0, 0); // initialization to the value 0

    /* ----- Creation of the semaphores ----- */
    // Worker
    sem_worker = sem_create(1, 'W');

    // Master
    sem_master = sem_create(2, 'M');

    // Initialization to the value 0
    union semun semopts;

    semopts.val = 0;
    semctl(sem_worker, 0, SETVAL, semopts);
    semctl(sem_master, 0, SETVAL, semopts);
    semctl(sem_master, 1, SETVAL, semopts);

    /* ----- Creation of the message queue ----- */
    msgq_1 = msgq_create('1');
    msgq_2 = msgq_create('2');

    /* ----------------------------------- */
    /* ---------- Sorting phase ---------- */
    /* ----------------------------------- */

    /* ----- Finding the maximum value ----- */
    max = -1;

    for(i = 0; i < N; i++) {
        value = shm_read(shm_numbers, i);

        if(value > max)
            max = value;
    }

    /* ----- Calculating the number of iterations ----- */
    iter = 0;

    while(max > 0) {
        max /= base;
        iter++;
    }

    if(iter == 0)
        iter = 1;

    /* ----- Creation of the different processes ----- */
    id = 0;

    for(i = 0; i < base; i++) {
        pid = fork();

        if(pid < 0) {
            printf("Error while creating process.\n");

            return EXIT_FAILURE;
        }

        if(pid == 0) {
            worker(id, N, base);
            i = base;
        } else {
            id++;
        }
    }

    if(pid > 0)
        master(base, iter, N);

    /* --------------------------------------- */
    /* ---------- Termination phase ---------- */
    /* --------------------------------------- */

    /* ----- End of the program and display of the result ----- */
    // Only the master process
    if(pid > 0) {
        // Display the sorted array
        printf("Sorted array: ");

        for(i = 0; i < N; i++)
            printf("%ld ", shm_read(shm_numbers, i));

        printf("\n");

        // Remove IPC elements
        shm_remove(id_numbers);
        shm_remove(id_temp);
        shm_remove(id_sorted);

        sem_remove(sem_worker);
        sem_remove(sem_master);

        msgq_remove(msgq_1);
        msgq_remove(msgq_2);

        // Free allocated elements
        array_free(numbers);

        // Return of the master process
        return 0;
    }

    // Return of the worker process
    return 0;
}
