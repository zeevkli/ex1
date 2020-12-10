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
/**
typedef struct MemberList_t 
{
    Member member;
    struct MemberList_t *next;
}*MemberNode;
**/
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
	PriorityQueue members;//!!! change member functions below
};

static PQElementPriority copyIdGeneric(PQElementPriority n);
static void freeIdGeneric(PQElementPriority n);
static int compareIdsGeneric(PQElementPriority n1, PQElementPriority n2);

static bool membersEqual(PQElement member1, PQElement member2);
static PQElement memberCopy(PQElement member);
static Member memberCreate(int id, char *name);
static void memberFree(PQElement member);

static Event eventCreate(Date date, int id, char* name);
static Event createBlankEvent(int id);
static PQElement eventCopy(PQElement event);
static void eventDestroy(PQElement event);
static bool eventsEqual(PQElement event1, PQElement event2);

static bool eventNameAlreadyExistsInDate(EventManager em, Event event);
static EventManagerResult eventAdd(EventManager em, Event event, Date date);
static EventManagerResult emFindEvent(EventManager em, int id, Event *event_p);

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
	Event event = (Event) malloc(sizeof(*event));
	if(!event)
	{
		return NULL;
	}
    event->name = (char *) malloc(strlen(name) + 1);
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

static Event createBlankEvent(int id)
{
    Event event = (Event) malloc(sizeof(*event));
    if(!event)
    {
        return NULL;
    }
    event->id = id;
    event->memberPQ = NULL;
    event->date = NULL;
    event->name = NULL;
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
	emDestroyMembers(em);
}
static Member emGetMember(EventManager em, int id)
{
    assert(id >= 0);
    MemberNode current = em->members;
    while(current)
    {
        if(current->member->id == id)
        {
            return current->member;
        }
        current = current->next;
    }
    return NULL;
}
static void emDestroyMembers(EventManager em)
{
    if(!em)
    {
        return;
    }
    MemberNode tmp =  NULL;
    while(em->members)
    {
        tmp =  em->members;
        em->members = em->members->next;
        memberFree(tmp->member);
        tmp->next = NULL;
    }
}
static bool emMemberExists(EventManager em, int id)//can probaby be replaced with emGetMember
{
    if(!em)
    {
        return false;
    }
    MemberNode current = em->members;
    while(current)
    {
        if(current->member->id == id)
        {
            return true;
        }
        current = current->next;
    }
    return false;
}
EventManagerResult emAddMember(EventManager em, char* member_name, int member_id)
{
    if(!em || !member_name)
    {
        return EM_NULL_ARGUMENT;
    }
    if(member_id < 0)
    {
        return EM_INVALID_MEMBER_ID;
    }
    if(emMemberExists(em, member_id))
    {
        return EM_MEMBER_ID_ALREADY_EXISTS;
    }
    Member new_member = memberCreate(member_id, member_name);
    if(!new_member)
    {
        return EM_OUT_OF_MEMORY;
    }
    MemberNode new_node = malloc(sizeof(*new_node));
    if(!new_node)
    {
        memberFree(new_member);
        return EM_OUT_OF_MEMORY;
    }
    new_node->member = new_member;
    new_node->next = em->members;
    em->members = new_node;
    return EM_SUCCESS;
}
EventManagerResult emAddMemberToEvent(EventManager em, int member_id, int event_id)
{
    if(!em)
    {
        return EM_NULL_ARGUMENT;
    }
    if(member_id<0)
    {
        return EM_INVALID_MEMBER_ID;
    }
    if(event_id <0)
    {
        return EM_INVALID_EVENT_ID;
    }  
    Event event = NULL;
    EventManagerResult result = emFindEvent(em, event_id, &event);
    if(result == EM_OUT_OF_MEMORY)
    {
        return EM_OUT_OF_MEMORY;
    }
    if(result == EM_EVENT_ID_NOT_EXISTS)
    {
        return EM_EVENT_ID_NOT_EXISTS;
    }
    Member member = emGetMember(em, member_id);
    if(!member)
    {
        return EM_MEMBER_ID_NOT_EXISTS;
    }
    if(pqContains(event->memberPQ, member))
    {
        return EM_EVENT_AND_MEMBER_ALREADY_LINKED;
    }
    pqInsert(event->memberPQ, member, &member->id);
    return EM_SUCCESS;
}
static bool eventNameAlreadyExistsInDate(EventManager em, Event event)
{
    PQ_FOREACH(Event, iteratorEvent, em->events)
    {
        if(dateCompare(iteratorEvent->date, event->date) == 0 && strcmp(iteratorEvent->name, event->name) == 0)
        {
            return true;
        }
    }
    return false;
}


static EventManagerResult eventAdd(EventManager em, Event event, Date date)
{
    PriorityQueueResult pqResult = pqInsert(em, event, date);
    //arguments cant be null because we already checked them
    assert(pqResult != PQ_NULL_ARGUMENT);
    switch(pqResult)
    {
        case PQ_OUT_OF_MEMORY:
            return EM_OUT_OF_MEMORY;
        default:
            return EM_SUCCESS;
    }
}

EventManagerResult emAddEventByDate(EventManager em, char* event_name, Date date, int event_id)
{
    if(!em || !event_name || !date)
    {
        return EM_NULL_ARGUMENT;
    }
    if(dateCompare(date, em->currentDate) > 0)
    {
        return EM_INVALID_DATE;
    }
    
    if(event_id < 0)
    {
        return EM_INVALID_EVENT_ID;
    }

    Event event = eventCreate(date, event_id, event_name);
    if(!event)
    {
        return EM_OUT_OF_MEMORY;
    }
    if(pqContains(em->events, event))
    {
        eventDestroy(event);
        return EM_EVENT_ID_ALREADY_EXISTS;
    }
    
    if(eventNameAlreadyExistsInDate(em, event))
    {
        eventDestroy(event);
        return EM_EVENT_ALREADY_EXISTS;
    }

    EventManagerResult emResult = eventAdd(em->events, event, date); 
    eventDestroy(event);
    return emResult;
}

EventManagerResult emAddEventByDiff(EventManager em, char* event_name, int days,
int event_id)
{
    if(!em || !event_name)
    {
        return EM_NULL_ARGUMENT;
    }
    if(days < 0)
    {
        return EM_INVALID_DATE;
    }
    if(event_id < 0)
    {
        return EM_INVALID_EVENT_ID;
    }
    //copy current date so it won't change when we tick
    Date newDate = dateCopy(em->currentDate);
    if(!newDate)
    {
        return EM_OUT_OF_MEMORY;
    }
    
    for (int i = 0; i < days; i++)
    {
        dateTick(newDate);
    }

    Event event = eventCreate(newDate, event_id, event_name);
    if(!event)
    {
        dateDestroy(newDate);
        return EM_OUT_OF_MEMORY;
    }
    if(eventNameAlreadyExistsInDate(em, event))
    {
        dateDestroy(newDate);
        eventDestroy(event);
        return EM_EVENT_ALREADY_EXISTS;
    }
    
    EventManagerResult emResult = eventAdd(em->events, event, newDate);
    dateDestroy(newDate);
    eventDestroy(event);
    return emResult;
}

EventManagerResult emRemoveEvent(EventManager em, int event_id)
{
    if(!em)
    {
        return EM_NULL_ARGUMENT;
    }
    if(event_id < 0)
    {
        return EM_INVALID_EVENT_ID;
    }
    
    Event eventToRemove = createBlankEvent(event_id);
    if(!eventToRemove)
    {
        return EM_OUT_OF_MEMORY;
    }

    PriorityQueueResult pqResult = pqRemoveElement(em->events, eventToRemove);
    eventDestroy(eventToRemove);
    //we already checked for null args
    assert(pqResult != PQ_NULL_ARGUMENT);
    switch(pqResult)
    {
        case PQ_ELEMENT_DOES_NOT_EXISTS:
            return EM_EVENT_NOT_EXISTS;
        default:
            return EM_SUCCESS;

    }
}

static EventManagerResult emFindEvent(EventManager em, int id, Event *event_p)
{
    if(!em)
    {
        return EM_NULL_ARGUMENT;
    }
    Event blankEvent = createBlankEvent(id);
    if(!blankEvent)
    {
        return EM_OUT_OF_MEMORY;
    }

    PQ_FOREACH(Event, iteratorEvent, em->events)
    {
        if(eventsEqual(iteratorEvent, blankEvent))
        {
            free(blankEvent);
            *event_p = &iteratorEvent;
            return EM_SUCCESS;
        }
    }
    eventDestroy(blankEvent);
    return EM_EVENT_ID_NOT_EXISTS;
}
EventManagerResult emChangeEventDate(EventManager em, int event_id, Date new_date)
{
    if(!em || !new_date)
    {
        return EM_NULL_ARGUMENT;
    }
    if(dateCompare(new_date, em->currentDate) > 0)
    {
        return EM_INVALID_DATE;
    }
    if(event_id < 0)
    {
        return EM_INVALID_EVENT_ID;
    }
    Event *eventToChange;
    EventManagerResult emResult = emFindEvent(em, event_id, eventToChange);
    
    //args can't be null
    assert(emResult != EM_NULL_ARGUMENT);
    
    switch(emResult)
    {
        case EM_OUT_OF_MEMORY:
            return EM_OUT_OF_MEMORY;
            
        case EM_SUCCESS:
            //first we check if the date of the event found is not the same as the new date
            if(dateCompare((*eventToChange)->date, new_date) == 0)
            {
                return EM_EVENT_ALREADY_EXISTS;
            }
            //copy the event found from the pointer
            Event copyOfEvent = (Event) eventCopy(*eventToChange);
            if(!copyOfEvent)
            {
                return EM_OUT_OF_MEMORY;
            }
            //change the date
            PriorityQueueResult pqResult = pqChangePriority(em->events, copyOfEvent,
            copyOfEvent->date, new_date);
            eventDestroy(copyOfEvent);
            
            //args wont be null cause we already checked them
            assert(pqResult != PQ_NULL_ARGUMENT);
            //element exists beacuse we already found it
            assert(pqResult != PQ_ELEMENT_DOES_NOT_EXISTS);
            
            switch(pqResult)
            {
                case PQ_OUT_OF_MEMORY:
                    return EM_OUT_OF_MEMORY;
                case PQ_SUCCESS:
                    return EM_SUCCESS;
            }
            
        case EM_EVENT_ID_NOT_EXISTS:
            return EM_EVENT_ID_NOT_EXISTS;
    }
}

EventManagerResult emTick(EventManager em, int days)
{
    if(!em)
    {
        return EM_NULL_ARGUMENT;
    }

    if(days <= 0)
    {
        return EM_INVALID_DATE;
    }
    for(int i = 0; i < days; i++)
    {
        dateTick(em->currentDate);
        Event first = pqGetFirst(em->events);
        while(first && dateCompate(first, em->currentDate) < 0)
        {
            PriorityQueueResult pqResult = pqRemove(em->events);
            assert(pqResult != PQ_NULL_ARGUMENT);
            first = pqGetFirst(em->events);
        }
    }
    return EM_SUCCESS;
}

int emGetEventsAmount(EventManager em)
{
    if(!em)
    {
        return -1;
    }
    return pqGetSize(em->events);
}

char* emGetNextEvent(EventManager em)
{
    if(!em)
    {
        return NULL;
    }
    return (pqGetFirst(em->events))->name;
}




