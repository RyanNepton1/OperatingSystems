CC=gcc
CFLAGS=
FMT=indent

mysh: shell.c interpreter.c shellmemory.c
	$(CC) $(CFLAGS) -c shell.c interpreter.c shellmemory.c pcb.c readystruct.c scheduler.c 
	$(CC) $(CFLAGS) -o mysh shell.o interpreter.o shellmemory.o pcb.o readystruct.o scheduler.o

style: shell.c shell.h interpreter.c interpreter.h shellmemory.c shellmemory.h pcb.c pcb.h readystruct.c readystruct.h scheduler.c scheduler.h
	$(FMT) $?

clean: 
	$(RM) mysh; $(RM) *.o; $(RM) *~

