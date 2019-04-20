/*
 * File: communication.h
 * Authors: Maxime Meurisse & Valentin Vermeylen
 *
 * This library allows you to manipulate System V's parallel programming
 * mechanisms.
 */

#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>

typedef struct {
    long mtype;
    long digit;
    long num_numbers;
    long write_pos;
} message;

/* ----------------------------------- */
/* ---------- Shared memory ---------- */
/* ----------------------------------- */

/*
 * This function creates a shared memory segment.
 *
 * Parameter(s)
 * ------------
 * size: the size of the shared memory segment
 * name: the unique name of the shared memory segment
 *
 * Return
 * ------
 * The ID of the shared memory segment.
 */
int shm_create(size_t size, char name);

/*
 * This function attaches a shared memory segment to a pointer.
 *
 * Parameter(s)
 * ------------
 * shm_id: the ID of the shared memory segment
 *
 * Return
 * ------
 * A pointer to the shared memory segment.
 */
long* shm_attach(int shm_id);

/*
 * This function allows to write a value in the shared memory segment
 * at a specific position.
 *
 * Parameter(s)
 * ------------
 * shm: a pointer to the shared memory segment
 * index: the position where to write
 * value: the value to write
 */
void shm_write(long* shm, size_t index, long value);

/*
 * This function allows to read a value in the shared memory segment at
 * a specific position.
 *
 * Parameter(s)
 * ------------
 * shm: a pointer to the shared memory
 * index: the position where the value to read is located
 *
 * Return
 * ------
 * The read value in the shared memory.
 */
long shm_read(long* shm, size_t index);

/*
 * This function allows to mark for deletion a shared memory segment.
 *
 * Parameter(s)
 * ------------
 * shm_id: the ID of the shared memory segment to delete
 */
void shm_remove(int shm_id);

/* ------------------------------- */
/* ---------- Semaphore ---------- */
/* ------------------------------- */

/*
 * This functio allows to create a set of semaphores.
 *
 * Parameter(s)
 * ------------
 * size: the size of the set of semaphores
 * name: the unique name of the set of semaphores
 *
 * Return
 * ------
 * The ID of the set of semaphores.
 */
int sem_create(size_t size, char name);

/*
 * This function decrements the semaphore if its value is > 0 or block
 * the process if not.
 *
 * Parameter(s)
 * ------------
 * sem_id: the ID of the set of semaphores
 * member: the index of the semaphore to lock in the set
 */
void sem_lock(int sem_id, size_t member);

/*
 * This function increment the semaphore.
 *
 * Parameter(s)
 * ------------
 * sem_id: the ID of the set of semaphores
 * member: the index of the semaphore to increment in the set
 */
void sem_unlock(int sem_id, size_t member);

/*
 * This function allows to mark for deletion a set of semaphores.
 *
 * Parameter(s)
 * ------------
 * sem_id: the ID of the set of semaphores to delete
 */
void sem_remove(int sem_id);

/* ----------------------------------- */
/* ---------- Message queue ---------- */
/* ----------------------------------- */

/*
 * This function creates a message queue.
 *
 * Parameter(s)
 * ------------
 * name: the name of the message queue
 *
 * Return
 * ------
 * The ID of the message queue.
 */
int msgq_create(char name);

/*
 * This function allows to send a message throw a message queue.
 *
 * Parameter(s)
 * ------------
 * msgq_id: the ID of the message queue
 * msg: the message to send
 */
void msgq_send(int msgq_id, message* msg);

/*
 * This function read a message from a message queue and remove it. If
 * there is no message, the process is blocked.
 *
 * Parameter(s)
 * ------------
 * msgq_id: the ID of the message queue
 * type: the type of the message
 * msg: the read message
 */
void msgq_read(int msgq_id, long type, message* msg);

/*
 * This function allows to mark for deletion a message queue.
 *
 * Parameter(s)
 * ------------
 * msgq_id: the ID of the message queue to delete
 */
void msgq_remove(int msgq_id);

#endif
