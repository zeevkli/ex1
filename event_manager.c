#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "event_manager.h"
#include "date.h"
#include "priority_queue.h"


typedef struct Member_t
{
    int id;
    char *name;
}*Member;

typedef struct MemberList_t 
{
    Member member;
    struct MemberList_t *next;
}*MemberNode;

typedef struct Event_t
{
    Date date;
    int id;
    char *name;
    PriorityQueue memberPQ;
}*Event;

struct EventManager_t
{
    Date currentDate;
    PriorityQueue events;
	MemberNode members;
};

static PQElementPriority copyIdGeneric(PQElementPriority n);
static void freeIdGeneric(PQElementPriority n);
static int compareIdsGeneric(PQElementPriority n1, PQElementPriority n2);

static bool membersEqual(PQElement member1, PQElement member2);
static PQElement memberCopy(PQElement member);
static Member memberCreate(int id, char *name);
static void memberFree(PQElement member);

static Event eventCreate(Date date, int id, char* name);
static PQElement eventCopy(PQElement event);
static void eventDestroy(PQElement event);
static bool eventsEqual(PQElement event1, PQElement event2);

static PQElementPriority copyIdGeneric(PQElementPriority n) {
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

static void freeIdGeneric(PQElementPriority n) {
    free(n);
}

static int compareIdsGeneric(PQElementPriority n1, PQElementPriority n2) {
    return (*(int *) n1 - *(int *) n2);
}

static Event eventCreate(Date date, int id, char* name)
{
	if(!name)
	{
		return NULL;
	}
	Event event = malloc(sizeof(*event));
	if(!event)
	{
		return NULL;
	}
    event->name = malloc(strlen(name) + 1);
	if(!event->name)
	{
		free(event);
		return NULL;
	}
	strcpy(event->name, name);
	PriorityQueue pq = pqCreate(memberCopy, memberFree, membersEqual, 
					copyIdGeneric, freeIdGeneric, compareIdsGeneric);
	if(!pq)
	{
		free(event);
		free(event->name);
		return NULL;
	}
	return event;
}

static PQElement eventCopy(PQElement event)
{
    if(!event)
	{
		return NULL;
	}
	Event newEvent = (Event) event;
    
	Event eventCopy = eventCreate(newEvent->date, newEvent->id, newEvent->name);
	if(!eventCopy)
	{
		return NULL;
	}
	eventCopy->memberPQ = pqCopy(newEvent->memberPQ);
	if(!eventCopy->memberPQ)
	{
		eventDestroy(eventCopy);
		return NULL;
	}
	return (PQElement) eventCopy;
}

static void eventDestroy(PQElement event)
{
    Event newEvent = (Event) event;
    pqDestroy(newEvent->memberPQ);
    free(newEvent->name);
    free(event);
}
static bool eventsEqual(PQElement event1, PQElement event2)
{
    if(!event1 || !event2)
    {
        return true;
    }
    if (!event1 || !event2)
    {
        return false;
    }
    Event newEvent1 = (Event) event1;
    Event newEvent2 = (Event) event2;

    return newEvent1->id == newEvent2->id;
}

static Member memberCreate(int id, char *name)
{
    if(!name)
    {
        return NULL;
    }
    Member member = (Member) malloc(sizeof(*member));
    if(!member)
    {
        return NULL;
    }
    member->name = (char *) malloc(strlen(name) + 1);
    if(!member->name)
    {
        free(member);
        return NULL;
    }
    strcpy(member->name, name);
    return (PQElement) member;
}


static PQElement memberCopy(PQElement member)
{
    if(!member)
    {
        return NULL;
    }
    Member memberNew = (Member) member;
    if(!memberNew->name){
        return NULL;
    }

    return createMember(memberNew->id, memberNew->name);
}

static bool membersEqual(PQElement member1, PQElement member2)
{
    if(!member1 && !member2)
    {
        return true;
    }
    if (!member1 || !member2)
    {
        return false;
    }
    Member newMember1 = (Member) member1;
    Member newMember2 = (Member) member2;
    return newMember1->id == newMember2->id;
}

static void memberFree(PQElement member)
{
    Member memberNew = (Member) member;
    free(memberNew->name);
    free(memberNew);
}


EventManager createEventManager(Date date)
{
    if(!date)
    {
        return NULL;
    }
    
    EventManager em = (EventManager) malloc(sizeof(*em));
    if(!em)
    {
        return NULL;
    }

    em->currentDate = dateCopy(date);
    if(!em->currentDate)
    {
        free(em);
        return NULL;
    }
    em->events = pqCreate(eventCopy, eventDestroy, eventsEqual,
						dateCopy, dateDestroy, dateCompare);
    if(!em->events)
    {
        dateDestroy(em->currentDate);
        free(em);
        return NULL;
    }
    em->members = NULL;
    return em;
}

void destroyEventManager(EventManager em)
{
	if(!em)
	{
		return;
	}
	dateDestroy(em->currentDate);
	pqDestroy(em->events);
	pqDestroy(em->members);
}

