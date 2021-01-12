mt: queue.c queue.h threads.c ts.s proc.h ts.s
	gcc -g -o threads -m32 ts.s threads.c queue.c
