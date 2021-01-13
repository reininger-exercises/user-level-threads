threads: queue.c queue.h threads.c ts.s proc.h ts.s mutex.c mutex.h
	gcc -g -o threads -m32 ts.s threads.c queue.c mutex.c
