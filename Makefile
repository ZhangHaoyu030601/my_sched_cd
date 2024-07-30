all: tasks

tasks: main.o executive.o tasks.o excstate.o busy_wait.o
	gcc -Wall -o tasks main.o executive.o busy_wait.o tasks.o excstate.o -lpthread -lrt

main.o: main.c
	gcc -Wall -c main.c -lpthread -lrt

executive.o: executive.c tasks.h
	gcc -Wall -c executive.c -DMULTIPROC

tasks.o: tasks.h busy_wait.h
	gcc -Wall -c tasks.c

excstate.o: excstate.c excstate.h
	gcc ${FLAGS} -c excstate.c -lpthread -lrt

busy_wait.o: busy_wait.c busy_wait.h
	gcc -Wall -c busy_wait.c

clean:
	rm -f *.o *~ tasks
