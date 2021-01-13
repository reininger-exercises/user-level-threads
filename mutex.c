#include "mutex.h"

// create a mutex and initialize it
Mutex *CreateMutex() 
{
	Mutex *mutex = (Mutex*)malloc(sizeof(Mutex));
	mutex->lock = mutex->owner = mutex->queue = 0;
	return mutex;
}

void DestroyMutex(Mutex *mutex)
{
	free(mutex);
}

int LockMutex(Mutex *mutex)
{
	if (!mutex->lock) {
		mutex->lock = true;
		mutex->owner = running;
	}
	else {
		running->status = BLOCK;
		Enqueue(&mutex->queue, running);
	}
}

int UnlockMutex(Mutex *mutex)
{
	// if cannot unlock, return an error
	if (!mutex->lock || (mutex->lock && mutex->owner != running)) {
		return -1;
	}
	
	if (!mutex->queue) {
		mutex->lock = false;
		mutex->owner = 0;
	}
	else {
		Proc *proc = Dequeue(&mutex->queue);
		mutex->owner = proc;
		proc->status = READY;
		Enqueue(&readyQueue, proc);
	}
}
