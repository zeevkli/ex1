CC = gcc
OBJS = date.o priority_queue.o event_manager.o
EXEC1_MAIN = pq_example_tests.o
EXEC2_MAIN = event_manager_example_tests.o
EXEC1 = event_manager
EXEC2 = priority_queue
DEBUG_FLAG = -g -DNDEBUG
COMP_FLAG = -std=c99 -Wall -Werror -pedantic-errors

$(EXEC1) : $(OBJS) $(EXEC1_MAIN)
	$(CC) $(DEBUG_FLAG) $(OBJS) $(EXEC1_MAIN) -o $@

$(EXEC2) : $(OBJS) $(EXEC2_MAIN)
	$(CC) $(DEBUG_FLAG) $(OBJS) $(EXEC2_MAIN) -o $@

date.o : date.c date.h
	$(CC) -c $(DEBUG_FLAG) $(COMP_FLAG) $*.c

priority_queue.o : priority_queue.c priority_queue.h
	$(CC) -c $(DEBUG_FLAG) $(COMP_FLAG) $*.c

pq_example_tests.o: priority_queue.c priority_queue.h test_utilities.h pq_example_tests.c
	$(CC) -c $(DEBUG_FLAG) $(COMP_FLAG) $*.c

event_manager.o : priority_queue.c priority_queue.h date.c date.h event_manager.c event_manager.h
	$(CC) -c $(DEBUG_FLAG) $(COMP_FLAG) $*.c

event_manager_example_tests.o : priority_queue.c priority_queue.h date.c date.h event_manager.c event_manager.h test_utilities.h event_manager_example_tests.c
	$(CC) -c $(DEBUG_FLAG) $(COMP_FLAG) $*.c

clean:
	rm -f $(OBJS) $(EXEC1) $(EXEC2) $(EXEC1_MAIN) $(EXEC2_MAIN)
