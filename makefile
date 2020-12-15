CC = gcc
OBJS = date.o priority_queue.o event_manager.o
EXEC1_MAIN = tests/event_manager_tests.o
EXEC2_MAIN = tests/pq_tests.o
EXEC1 = event_manager
EXEC2 = priority_queue
DEBUG_FLAG = -g 
COMP_FLAG = -std=c99 -Wall -Werror -pedantic-errors

$(EXEC1) : $(OBJS) $(EXEC1_MAIN)
	$(CC) $(DEBUG_FLAG) $(OBJS) $(EXEC1_MAIN) -o $@

$(EXEC2) : priority_queue.o $(EXEC2_MAIN)
	$(CC) $(DEBUG_FLAG) priority_queue.o $(EXEC2_MAIN) -o $@

date.o : date.c date.h
	$(CC) -c $(DEBUG_FLAG) $(COMP_FLAG) $*.c

priority_queue.o : priority_queue.c priority_queue.h
	$(CC) -c $(DEBUG_FLAG) $(COMP_FLAG) $*.c

$(EXEC2_MAIN): priority_queue.c priority_queue.h tests/pq_tests.c
	$(CC) -c -o $(EXEC2_MAIN) $(DEBUG_FLAG) $(COMP_FLAG) $*.c

event_manager.o : priority_queue.c priority_queue.h date.c date.h event_manager.c event_manager.h
	$(CC) -c $(DEBUG_FLAG) $(COMP_FLAG) $*.c

$(EXEC1_MAIN) : priority_queue.c priority_queue.h date.c date.h event_manager.c event_manager.h tests/event_manager_tests.c
	$(CC) -c  -o $(EXEC1_MAIN) $(DEBUG_FLAG) $(COMP_FLAG) $*.c

clean:
	rm -f $(OBJS) $(EXEC1) $(EXEC2) $(EXEC1_MAIN) $(EXEC2_MAIN)
