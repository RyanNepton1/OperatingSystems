CC=gcc
#CFLAGS=-g -O0 -Wall #-DNDEBUG
CFLAGS=-DNDEBUG -Wall
LDFLAGS=-pthread

framesize ?= 18
varmemsize ?= 10
MEMDEFS=-DFRAME_STORE_SIZE=$(framesize) -DVAR_STORE_SIZE=$(varmemsize)

mysh: shell.c interpreter.c shellmemory.c
	$(CC) $(CFLAGS) $(MEMDEFS) -c shell.c interpreter.c shellmemory.c pcb.c queue.c schedule_policy.c
	$(CC) $(CFLAGS) -o mysh shell.o interpreter.o shellmemory.o pcb.o queue.o schedule_policy.o $(LDFLAGS)

clean: 
	rm mysh; rm *.o
