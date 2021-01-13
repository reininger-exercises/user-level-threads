/*
 * Reid Reininger, charles.reininger@wsu.edu
 *
 * Multi-tasking system using threads. Based on KC Wang's "Systems Programming
 * for Unix/Linux", 2018. Although a multi tasking system has already been
 * created this system was created to mirror the textbook to reduce
 * complications.
 */

#ifndef __THREADS_H_
#define __THREADS_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "queue.h"
#include "proc.h"

void init();
void texit(int value);
int create(void (*f)(), void *parm);
void func(void *parm);
int do_create();
int do_switch();
int do_exit();
int scheduler();
void tswitch(); // defined in ts.s
void tsleep(int event);
void twakeup(int event);
int join(int targetPid, int *status);
int do_join();

extern Proc *readyQueue;
extern Proc *running;

#endif
