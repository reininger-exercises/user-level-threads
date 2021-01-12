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

// exit thread
int texit(int value)
{
	printf("task %d in texit value=%d\n", running->pid, running->pid);
	running->status = FREE;
	running->priority = 0;
	Enqueue(&freeList, running);
	PrintList("freeList", freeList);
	tswitch();
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
		printf("enter a key [c|s|q]: ");
		c = getchar(); getchar();
		switch(c) {
			case 'c': do_create(); break;
			case 's': do_switch(); break;
			case 'q': do_exit(); break;
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
	printf("Welcome to the MT User-Level Threads System\n");
	init();
	create((void*)func, 0);
	printf("P0 switch to P1\n");
	while(1) {
		if (readyQueue)
			tswitch();
	}
}

int scheduler()
{
	if (running->status == READY)
		Enqueue(&readyQueue, running);
	running = Dequeue(&readyQueue);
	printf("next running = %d\n", running->pid);
}
