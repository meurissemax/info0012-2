/*
 * File: communication.c
 * Authors: Maxime Meurisse & Valentin Vermeylen
 *
 * This library allows you to manipulate System V's parallel programming
 * mechanisms.
 */

#include "headers/communication.h"

/* ----------------------------------- */
/* ---------- Shared memory ---------- */
/* ----------------------------------- */
int shm_create(size_t size, char name) {
    assert(size > 0);

    key_t shm_key;
    int shm_id;

    shm_key = ftok(".", name);

    // Create shared memory segment
    if((shm_id = shmget(shm_key, size, IPC_CREAT | IPC_EXCL | 0666)) == -1) {
        printf("Shared memory segment exists - opening as client.\n");

        // Segment probably already exists - try as client
        if((shm_id = shmget(shm_key, size, 0)) == -1) {
            perror("shmget");

            exit(errno);
        }
    }

    return shm_id;
}

long* shm_attach(int shm_id) {
    long* shm;

    if((shm = (long*)shmat(shm_id, 0, 0)) == (long*)-1) {
        perror("shmat");

        exit(errno);
    }

    return shm;
}

void shm_write(long* shm, size_t index, long value) {
    shm[index] = value;

    fflush(stdout);
}

long shm_read(long* shm, size_t index) {
    return shm[index];
}

void shm_remove(int shm_id) {
    if(shmctl(shm_id, IPC_RMID, 0) == -1) {
        perror("shmctl");

        exit(errno);
    }
}

/* ------------------------------- */
/* ---------- Semaphore ---------- */
/* ------------------------------- */
int sem_create(size_t size, char name) {
    assert(size > 0);

    key_t sem_key;
    int sem_id;

    sem_key = ftok(".", name);

    if((sem_id = semget(sem_key, size, IPC_CREAT | IPC_EXCL | 0666)) == -1) {
        printf("Semaphore set already exists.\n");

        exit(errno);
    }

    return sem_id;
}

void sem_lock(int sem_id, size_t member) {
    struct sembuf lock = {member, -1, 0};

    if((semop(sem_id, &lock, 1)) == -1) {
        perror("semop");

        exit(errno);
    }
}

void sem_unlock(int sem_id, size_t member) {
    struct sembuf unlock = {member, 1, 0};

    if((semop(sem_id, &unlock, 1)) == -1) {
        perror("semop");

        exit(errno);
    }
}

void sem_remove(int sem_id) {
    if(semctl(sem_id, 0, IPC_RMID, 0) == -1) {
        perror("semctl");

        exit(errno);
    }
}

/* ----------------------------------- */
/* ---------- Message queue ---------- */
/* ----------------------------------- */
int msgq_create(char name) {
    key_t msgq_key;
    int msgq_id;

    msgq_key = ftok(".", name);
    msgq_id = 0;

    // Open the queue - create if necessary
    if((msgq_id = msgget(msgq_key, IPC_CREAT | 0660)) == -1) {
        perror("msgget");

        exit(errno);
    }

    return msgq_id;
}

void msgq_send(int msgq_id, message* msg) {
    assert(msg != NULL);

    int length;

    if(msg == NULL) {
        perror("msgsnd");

        exit(errno);
    }

    length = sizeof(message) - sizeof(long);

    if((msgsnd(msgq_id, msg, length, 0)) == -1) {
        perror("msgsnd");

        exit(errno);
    }
}

void msgq_read(int msgq_id, long type, message* msg) {
    assert(msg != NULL);

    int length;

    if(msg == NULL) {
        perror("readmsg");

        exit(errno);
    }

    length = sizeof(message) - sizeof(long);

    if((msgrcv(msgq_id, msg, length, type, 0)) == -1) {
        perror("readmsg");

        exit(errno);
    }
}

void msgq_remove(int msgq_id) {
    if(msgctl(msgq_id, IPC_RMID, 0) == -1) {
        perror("msgctl");

        exit(errno);
    }
}
