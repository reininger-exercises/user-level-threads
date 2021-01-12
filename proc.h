/*
 * Reid Reininger
 * charles.reininger@wsu.edu
 * 
 * This programs implements user level threads. Within a linux process, the
 * program multitasks between executing different threads.
 *
 * Adapted from KC Wang's "Systems Programming in Unix/Linux", 2018.
 */

#ifndef __TYPE_H_
#define __TYPE_H_

#define NPROC 9 // number of Proc structures
#define SSIZE 1024 // stack size

typedef enum procStatus {
	FREE, READY, SLEEP, BLOCK, ZOMBIE
} ProcStatus;

// Example Process control block structure (PCB).
// Conatains all of the information about a process.
// In linux systems, threads are managed as procs that share common resources.
typedef struct proc {
	struct proc *next; // next proc pointer, for arrainging in data structures
	int ksp; // saved stack pointer: at byte offset 4 in struct
	int pid; // process id
	ProcStatus status; // Proc status = FREE|READY, etc
	int priority; // scheduling priority, higher priority more important
	int event; // event value to sleep on
	int exitStatus; // exit value
	int joinPid; // join target pid
	struct proc *joinPtr; // join target Proc pointer
	int kstack[SSIZE]; // process execution stack
} Proc;

#endif

