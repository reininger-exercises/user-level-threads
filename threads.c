/*
 * Reid Reininger, charles.reininger@wsu.edu
 *
 * Multi-tasking system using threads. Based on KC Wang's "Systems Programming
 * for Unix/Linux", 2018. Although a multi tasking system has already been
 * created this system was created to mirror the textbook to reduce
 * complications.
 */

#include "threads.h"

Proc procs[NPROC];
Proc *freeList;
Proc *readyQueue;
Proc *sleepList;
Proc *running;

// initialize multitasking system for threads
void init()
{
	int i, j;
	Proc *p;
	for (i=0; i<NPROC; i++) {
		p = &procs[i];
		p->pid = i;
		p->priority = 0;
		p->status = FREE;
		p->event = 0;
		p->joinPid = 0;
		p->joinPtr = 0;
		p->next = p+1;
	}

	procs[NPROC-1].next = 0;
	freeList = &procs[0]; // all Procs in freeList
	readyQueue = 0;
	sleepList = 0;
	// create P0 as initial running task
	running = p = Dequeue(&freeList);
	p->status = READY;
	p->priority = 0;
	PrintList("freeList", freeList);
	printf("init complete: P0 running\n");
}

// exit thread. This funciton never returns.
void texit(int value)
{
	Proc *p;
	printf("task%d is in texit\n", running->pid);
	// try to find any task wanting to join with running task
	for (p = sleepList; p && p->joinPid != running->pid; p=p->next);
	if (!p) { // if none waiting
		printf("No task waiting for task%d to terminate, exit as FREE\n",
			running->pid);
		running->status = FREE;
		running->priority = 0;
		running->joinPid = 0;
		running->joinPtr = 0;
		Enqueue(&freeList, running);
		PrintList("freeList", freeList);
		do_switch();
	}

	// there exits a waiting task
	printf("tasks are waiting for task%d, exiting as ZOMBIE\n", running->pid);
	running->exitStatus = value;
	running->status = ZOMBIE;
	twakeup(running->pid);
	do_switch();
}

// Create a new thread to run the passed in function.
int create(void (*f)(), void *parm)
{
	int i;
	Proc *p = Dequeue(&freeList);
	if (!p) {
		printf("create failed\n");
		return -1;
	}
	p->status = READY;
	p->priority = 1;
	p->joinPid = 0;
	p->joinPtr = 0;

	// initialize new task stack for it to resume to f(parm)
	for (i=1; i<13; i++) { // zero out stack cells
		p->kstack[SSIZE-i] = 0;
	}
	p->kstack[SSIZE-1] = (int)parm; // funciton parameter
	p->kstack[SSIZE-2] = (int)do_exit; // function return address
	p->kstack[SSIZE-3] = (int)f; // function entry
	p->ksp = (int)&p->kstack[SSIZE-12]; // ksp -> stack top
	Enqueue(&readyQueue, p);
	PrintList("readyQueue", readyQueue);
	printf("task %d created a new task %d\n", running->pid, p->pid);
	return p->pid;
}

// Function all threads run for demonstration purposes.
void func(void *parm)
{
	int c;
	printf("task %d start: parm = %d\n", running->pid, (int)parm);
	while (true) {
		printf("task %d running\n", running->pid);
		PrintList("readyQueue", readyQueue);
		printf("\033[0;32menter a key [c|s|q|j]: \033[0m");
		c = getchar(); getchar();
		switch(c) {
			case 'c': do_create(); break;
			case 's': do_switch(); break;
			case 'q': do_exit(); break;
			case 'j': do_join(); break;
			case EOF:
				putchar('\n');
				exit(0);
				break;
		}
	}
}

// Create a new thread running func.
int do_create()
{
	int pid = create(func, (void*)running->pid); // parm = pid
}

// Calling thread gives up the CPU.
int do_switch()
{
	tswitch();
}

// Call texit().
int do_exit()
{
	texit(running->pid); // for simplicity: exit with pid value
}

int main()
{
	printf("Welcome to the User-Level Threads System\n");
	init();
	create((void*)func, 0);
	printf("P0 switch to P1\n");
	while(1) {
		if (readyQueue)
			do_switch();
		else {
			printf("All tasks ended, exiting\n");
			break;
		}
	}
}

int scheduler()
{
	if (running->status == READY)
		Enqueue(&readyQueue, running);
	running = Dequeue(&readyQueue);
	printf("next running = %d\n", running->pid);
}

// Thread waits to be awakened on event.
void tsleep(int event)
{
	printf("task%d waits for event%d\n", running->pid, event);
	running->event = event;
	running->status = SLEEP;
	Enqueue(&sleepList, running);
	PrintList("sleepList", sleepList);
	do_switch();
}

// Wakes up all tasks sleeping on event.
void twakeup(int event)
{
	if (!sleepList) return;
	Proc *prev = NULL, *cur = sleepList, *next = cur->next;
	while (cur) {
		if (cur->event == event) {
			cur->status = READY;
			Enqueue(&readyQueue, cur);
			printf("task%d woke up task%d\n", running->pid, cur->pid);
			
			// remove from sleepList
			if (!prev) {
				sleepList = next;
			}
			else {
				prev->next = next;
			}
		}
		else {
			prev = cur;
		}
		cur = next;
		if (next) next = next->next;
	}
}

// Wait for a thread with targetPid to terminate.
int join(int targetPid, int *status)
{
	Proc *p, *target = &procs[targetPid];
	while (true) {
		if (target->status == FREE) {
			printf("could not find task%d\n", targetPid);
			return -1; // NOPID error
		}
		// check for deadlock
		for (p = target; p; p = p->joinPtr) {
			if (p == running) {
				printf("joining with task%d leads to deadlock\n", targetPid);
				return -2; // DEADLOCK error
			}
		}
		// set running join fields
		running->joinPid = targetPid;
		running->joinPtr = target;

		// if found targets zombie
		if (target->status == ZOMBIE) {
			printf("task%d found taks%d's ZOMBIE\n", running->pid, target->pid);
			*status = target->exitStatus;
			target->status = FREE;
			target->priority = 0;
			target->joinPid = 0;
			target->joinPtr = 0;
			Enqueue(&freeList, target);
			return target->pid;
		}
		tsleep(targetPid);
	}
}

int do_join()
{
	int c, status;
	printf("enter a pid to join with: ");
	c = getchar(); getchar();
	c -= '0';
	printf("task%d try to join with task%d\n", running->pid, c);
	join(c, &status);
}
