#include "test_utilities.h"
#include "../priority_queue.h"
#include <assert.h>
#include <stdlib.h>
#define NUMBER_TESTS 10
static void freeCharGeneric(PQElementPriority c)
{
    free(c);
}

static int compareCharsGeneric(PQElementPriority n1, PQElementPriority n2) {
    return (*(char *) n1 - *(char *) n2);
}

static PQElementPriority copyCharGeneric(PQElementPriority n) {
    if (!n) {
        return NULL;
    }
    char *copy = malloc(sizeof(*copy));
    if (!copy) {
        return NULL;
    }
    *copy = *(char *) n;
    return copy;
}

static PQElementPriority copyIntGeneric(PQElementPriority n) {
    if (!n) {
        return NULL;
    }
    int *copy = malloc(sizeof(*copy));
    if (!copy) {
        return NULL;
    }
    *copy = *(int *) n;
    return copy;
}

static void freeIntGeneric(PQElementPriority n) {
    free(n);
}

static int compareIntsGeneric(PQElementPriority n1, PQElementPriority n2) {
    return (*(int *) n1 - *(int *) n2);
}

static bool equalIntsGeneric(PQElementPriority n1, PQElementPriority n2) {
    return *(int *) n1 == *(int *) n2;
}

bool testChangePriorityByRef(){
    bool result = true;
    PriorityQueue pq = pqCreate(copyIntGeneric, freeIntGeneric, equalIntsGeneric, copyIntGeneric, freeIntGeneric, compareIntsGeneric);
 
    int elem1 = 5;
    ASSERT_TEST(pqInsert(pq, &elem1, &elem1) == PQ_SUCCESS, destroy);
 
    int* elem1_ptr =(int *)pqGetFirst(pq);
    ASSERT_TEST(*elem1_ptr == elem1, destroy);
 
    ASSERT_TEST(pqChangePriority(pq, elem1_ptr, elem1_ptr, elem1_ptr) == PQ_SUCCESS, destroy);
    elem1_ptr = (int *)pqGetFirst(pq);
    ASSERT_TEST(*elem1_ptr == elem1, destroy);
 
    destroy:
    pqDestroy(pq);
    return result;
}

bool testPQCreateDestroy() {
    bool result = true;

    PriorityQueue pq = pqCreate(copyIntGeneric, freeIntGeneric, equalIntsGeneric, copyIntGeneric, freeIntGeneric, compareIntsGeneric);
    ASSERT_TEST(pq != NULL, returnPQCreateDestroy);
    ASSERT_TEST(pqGetSize(pq) == 0,destroyPQCreateDestroy);
    ASSERT_TEST(pqGetFirst(pq) == NULL,destroyPQCreateDestroy);

destroyPQCreateDestroy:
    pqDestroy(pq);
returnPQCreateDestroy:
    return result;
}

bool testPQInsertAndSize() {
    bool result = true;
    PriorityQueue pq = pqCreate(copyIntGeneric, freeIntGeneric, equalIntsGeneric, copyIntGeneric, freeIntGeneric, compareIntsGeneric);
    ASSERT_TEST(pqGetSize(pq) == 0,destroyPQInsertAndSize);
    int to_add = 1;
    ASSERT_TEST(pqInsert(pq, &to_add, &to_add) == PQ_SUCCESS,destroyPQInsertAndSize);
    ASSERT_TEST(pqGetSize(pq) == 1,destroyPQInsertAndSize);

destroyPQInsertAndSize:
    pqDestroy(pq);
    return result;
}

bool testPQGetFirst() {
    bool result = true;
    PriorityQueue pq = pqCreate(copyIntGeneric, freeIntGeneric, equalIntsGeneric, copyIntGeneric, freeIntGeneric, compareIntsGeneric);
    ASSERT_TEST(pqGetSize(pq) == 0, destroyPQGetFirst);
    int to_add = 1;
    ASSERT_TEST(pqInsert(pq, &to_add, &to_add) == PQ_SUCCESS, destroyPQGetFirst);
    int* first_value = pqGetFirst(pq);
    ASSERT_TEST(first_value != NULL, destroyPQGetFirst);
    ASSERT_TEST(*first_value == to_add, destroyPQGetFirst);

destroyPQGetFirst:
    pqDestroy(pq);
    return result;
}

bool testPQIterator() {
    bool result = true;
    PriorityQueue pq = pqCreate(copyIntGeneric, freeIntGeneric, equalIntsGeneric, copyIntGeneric, freeIntGeneric, compareIntsGeneric);

    int max_value = 10;

    for(int i=0; i< max_value; i++){
        ASSERT_TEST(pqInsert(pq, &i, &i) == PQ_SUCCESS, destroyPQIterator);
    }

    int i = 0;
    PQ_FOREACH(int*, iter, pq) {
        if (i != max_value) {
            ASSERT_TEST(iter != NULL,destroyPQIterator);
        } else {
            ASSERT_TEST(iter == NULL,destroyPQIterator);
        }
        i++;
    }

destroyPQIterator:
    pqDestroy(pq);
    return result;
}
/////////NEW 
bool testPQCreateDestroy2() {
    bool result = true;

    PriorityQueue pq = pqCreate(copyIntGeneric, freeIntGeneric, equalIntsGeneric, copyCharGeneric, freeCharGeneric, compareCharsGeneric);
    ASSERT_TEST(pq != NULL, returnPQCreateDestroy);
    ASSERT_TEST(pqGetSize(pq) == 0,destroyPQCreateDestroy);
    ASSERT_TEST(pqGetFirst(pq) == NULL,destroyPQCreateDestroy);

destroyPQCreateDestroy:
    pqDestroy(pq);
returnPQCreateDestroy:
    return result;
}

bool testPQInsertAndSize2() {
    bool result = true;
    PriorityQueue pq = pqCreate(copyIntGeneric, freeIntGeneric, equalIntsGeneric, copyCharGeneric, freeCharGeneric, compareCharsGeneric);
    ASSERT_TEST(pqGetSize(pq) == 0,destroyPQInsertAndSize);
    int to_add = 1;
    ASSERT_TEST(pqInsert(pq, &to_add, &to_add) == PQ_SUCCESS,destroyPQInsertAndSize);
    ASSERT_TEST(pqGetSize(pq) == 1,destroyPQInsertAndSize);

destroyPQInsertAndSize:
    pqDestroy(pq);
    return result;
}

bool testPQGetFirst2() {
    bool result = true;
    PriorityQueue pq = pqCreate(copyIntGeneric, freeIntGeneric, equalIntsGeneric, copyCharGeneric, freeCharGeneric, compareCharsGeneric);
    ASSERT_TEST(pqGetSize(pq) == 0, destroyPQGetFirst);
    int to_add = 1;
    ASSERT_TEST(pqInsert(pq, &to_add, &to_add) == PQ_SUCCESS, destroyPQGetFirst);
    int* first_value = pqGetFirst(pq);
    ASSERT_TEST(first_value != NULL, destroyPQGetFirst);
    ASSERT_TEST(*first_value == to_add, destroyPQGetFirst);

destroyPQGetFirst:
    pqDestroy(pq);
    return result;
}

bool testPQIterator2() {
    bool result = true;
    PriorityQueue pq = pqCreate(copyIntGeneric, freeIntGeneric, equalIntsGeneric, copyCharGeneric, freeCharGeneric, compareCharsGeneric);

    int max_value = 10;
	char c = 'a';
    for(int i=0; i< max_value; i++){
		c = (char)i + 'a';
        ASSERT_TEST(pqInsert(pq, &i,&c) == PQ_SUCCESS, destroyPQIterator);
    }

    int i = 0;
    PQ_FOREACH(int*, iter, pq) {
        if (i != max_value) {
            ASSERT_TEST(iter != NULL,destroyPQIterator);
        } else {
            ASSERT_TEST(iter == NULL,destroyPQIterator);
        }
        i++;
    }

destroyPQIterator:
    pqDestroy(pq);
    return result;
}
static void printList(PriorityQueue pq)
{
	int i =0;
	PQ_FOREACH(int*, iter, pq) 
	{
		printf("VALUE %d: %d\n",i, *iter);
		i++;
	}
	assert(i == pqGetSize(pq));
	printf("-------------------------\n");
}
bool newTest() {
    bool result = true;
    PriorityQueue pq = pqCreate(copyIntGeneric, freeIntGeneric, equalIntsGeneric, copyCharGeneric, freeCharGeneric, compareCharsGeneric);
    int max_value = 10;
	char a ='a';
	char b ='b';
	char c ='c';
	char d ='d';
	char z  ='z';
	char t = 't';
    int i = 0;
	printf("\n");
    ASSERT_TEST(pqInsert(pq, &i, &d) == PQ_SUCCESS, destroyPQIterator);
	i++;
	printList(pq);
	ASSERT_TEST(pqInsert(pq, &i, &b) == PQ_SUCCESS, destroyPQIterator);
	i++;
	printList(pq);
	ASSERT_TEST(pqInsert(pq, &i, &c) == PQ_SUCCESS, destroyPQIterator);
	i++;
	printList(pq);
	ASSERT_TEST(pqInsert(pq, &i, &a) == PQ_SUCCESS, destroyPQIterator);
	printList(pq);
	i++;
	int n = 11;
	ASSERT_TEST(pqInsert(pq, &n, &b) == PQ_SUCCESS, destroyPQIterator);
	i++;
	n+=11;
	printList(pq);
	ASSERT_TEST(pqInsert(pq, &n, &b) == PQ_SUCCESS, destroyPQIterator);
	i++;
	n+=11;
	printList(pq);
	ASSERT_TEST(pqInsert(pq, &n, &b) == PQ_SUCCESS, destroyPQIterator);
	i++;
	n+=11;
	printList(pq);
	ASSERT_TEST(pqInsert(pq, &n, &z) == PQ_SUCCESS, destroyPQIterator);
	i++;
	n+=11;
	printList(pq);
	ASSERT_TEST(pqInsert(pq, &n, &z) == PQ_SUCCESS, destroyPQIterator);
	i++;
	n+=11;
	printList(pq);
	ASSERT_TEST(pqInsert(pq, &n, &z) == PQ_SUCCESS, destroyPQIterator);
	i++;
	n+=11;
	printList(pq);
	ASSERT_TEST(pqInsert(pq, &n, &z) == PQ_SUCCESS, destroyPQIterator);
	i++;
	n+=11;
	printList(pq);
	ASSERT_TEST(pqInsert(pq, &n, &t) == PQ_SUCCESS, destroyPQIterator);
	printList(pq);
	ASSERT_TEST(pqInsert(pq, &n, &t) == PQ_SUCCESS, destroyPQIterator);
	printList(pq);
	ASSERT_TEST(pqInsert(pq, &n, &z) == PQ_SUCCESS, destroyPQIterator);
	printList(pq);
	ASSERT_TEST(pqInsert(pq, &n, &z) == PQ_SUCCESS, destroyPQIterator);
	printList(pq);
	ASSERT_TEST(pqChangePriority(pq, &n, &z,&a) == PQ_SUCCESS, destroyPQIterator);
	printList(pq);
	ASSERT_TEST(pqChangePriority(pq, &n, &z,&a) == PQ_SUCCESS, destroyPQIterator);
	printList(pq);
	ASSERT_TEST(pqChangePriority(pq, &n, &t,&a) == PQ_SUCCESS, destroyPQIterator);
	printList(pq);
	ASSERT_TEST(pqChangePriority(pq, &n, &t,&a) == PQ_SUCCESS, destroyPQIterator);
	printList(pq);
	PriorityQueue pq2 = pqCopy(pq);
	printList(pq2);
	int r = 0;
	ASSERT_TEST(pqRemoveElement(pq2, &r) == PQ_SUCCESS, destroyPQIterator);
	printList(pq2);
	ASSERT_TEST(pqRemoveElement(pq2, &n) == PQ_SUCCESS, destroyPQIterator);
	printList(pq2);
	ASSERT_TEST(pqRemoveElement(pq2, &n) == PQ_SUCCESS, destroyPQIterator);
	printList(pq2);
	ASSERT_TEST(pqRemoveElement(pq2, &n) == PQ_SUCCESS, destroyPQIterator);
	printList(pq2);
	ASSERT_TEST(pqRemove(pq2) == PQ_SUCCESS, destroyPQIterator);
	printList(pq2);
	ASSERT_TEST(pqRemove(pq2) == PQ_SUCCESS, destroyPQIterator);
	printList(pq2);
	ASSERT_TEST(pqRemove(pq2) == PQ_SUCCESS, destroyPQIterator);
	printList(pq2);
    PQ_FOREACH(int*, iter, pq) {
        if (i != max_value) {
            ASSERT_TEST(iter != NULL,destroyPQIterator);
        } else {
            ASSERT_TEST(iter == NULL,destroyPQIterator);
        }
        i++;
    }
	printf("\n");
	printList(pq);

	destroyPQIterator:
		pqDestroy(pq);
		return result;
}
bool (*tests[]) (void) = {
        testPQCreateDestroy,
        testPQInsertAndSize,
        testPQGetFirst,
        testPQIterator,
        testPQCreateDestroy2,
        testPQInsertAndSize2,
        testPQGetFirst2,
        testPQIterator2,
        testChangePriorityByRef,
		newTest
};

const char* testNames[] = {
        "testPQCreateDestroy",
        "testPQInsertAndSize",
        "testPQGetFirst",
        "testPQIterator",
		"testPQCreateDestroy2",
        "testPQInsertAndSize2",
        "testPQGetFirst2",
        "testPQIterator2",
        "testChangePriorityByRef",
		"newTest"
};

int main(int argc, char *argv[]) {
    if (argc == 1) {
        for (int test_idx = 0; test_idx < NUMBER_TESTS; test_idx++) {
            RUN_TEST(tests[test_idx], testNames[test_idx]);
        }
        return 0;
    }
    if (argc != 2) {
        fprintf(stdout, "Usage: priority_queue_tests <test index>\n");
        return 0;
    }

    int test_idx = strtol(argv[1], NULL, 10);
    if (test_idx < 1 || test_idx > NUMBER_TESTS) {
        fprintf(stderr, "Invalid test index %d\n", test_idx);
        return 0;
    }

    RUN_TEST(tests[test_idx - 1], testNames[test_idx - 1]);


    system("pause");
    return 0;
}