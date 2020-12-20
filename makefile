CC = gcc
EXEC1_OBJS = date.o event_manager.o
EXEC2_OBJS = priority_queue.o
EXEC1_MAIN = tests/event_manager_tests.o
EXEC2_MAIN = tests/priority_queue_tests.o
EXEC1 = event_manager
EXEC2 = priority_queue
DEBUG_FLAG = -DNDEBUG
COMP_FLAG = -std=c99 -Wall -Werror -pedantic-errors

$(EXEC1) : $(EXEC1_OBJS) $(EXEC1_MAIN)
	$(CC) $(DEBUG_FLAG) $(EXEC1_OBJS) $(EXEC1_MAIN) -L. -lpriority_queue -o $@

$(EXEC2) : $(EXEC2_OBJS) $(EXEC2_MAIN)
	$(CC) $(DEBUG_FLAG) $(EXEC2_OBJS) $(EXEC2_MAIN) -o $@

date.o : date.c date.h
	$(CC) -c $(DEBUG_FLAG) $(COMP_FLAG) $*.c

priority_queue.o : priority_queue.c priority_queue.h
	$(CC) -c $(DEBUG_FLAG) $(COMP_FLAG) $*.c

$(EXEC2_MAIN): priority_queue.c priority_queue.h tests/priority_queue_tests.c
	$(CC) -c -o $(EXEC2_MAIN) $(DEBUG_FLAG) $(COMP_FLAG) $*.c

event_manager.o : priority_queue.c priority_queue.h date.c date.h event_manager.c event_manager.h
	$(CC) -c $(DEBUG_FLAG) $(COMP_FLAG) $*.c

$(EXEC1_MAIN) : priority_queue.c priority_queue.h date.c date.h event_manager.c event_manager.h tests/event_manager_tests.c
	$(CC) -c  -o $(EXEC1_MAIN) $(DEBUG_FLAG) $(COMP_FLAG) $*.c

clean:
	rm -f $(EXEC1_OBJS) $(EXEC2_OBJS) $(EXEC1) $(EXEC2) $(EXEC1_MAIN) $(EXEC2_MAIN)
