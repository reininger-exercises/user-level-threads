#ifndef __MUTEX_H_
#define __MUTEX_H_

#include <stdlib.h>
#include <stdbool.h>
#include "queue.h"
#include "proc.h"
#include "threads.h"

typedef struct mutex {
	bool lock;
	Proc *owner; // pointer to owner of mutex
	Proc *queue; // FIFO queue of BLOCKED waiting Procs
} Mutex;

Mutex *CreateMutex();
void DestroyMutex(Mutex *mutex);
int LockMutex(Mutex *mutex);
int UnlockMutex(Mutex *mutex);

#endif
