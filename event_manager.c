#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "event_manager.h"
#include "date.h"
#include "priority_queue.h"

typedef enum {MEMBER_ADD_EVENT, MEMBER_REMOVE_EVENT} memberEnum;
typedef struct Member_t
{
    int id;
    char *name;
	int events_number; //matters only in members pq
}*Member;

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
	PriorityQueue members;
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

static bool eventNameAndDateinPQ(EventManager em, Event event);
static bool nameAndDateinPQEvent(EventManager em, char *name, Date date);
static EventManagerResult eventAdd(EventManager em, Event event, Date date);
static EventManagerResult emFindEvent(EventManager em, int id, Event *event_p);
static EventManagerResult emFindMember(EventManager em, int id, Member *member_p);

static EventManagerResult emMemberChangePriority(EventManager em, int member_id, memberEnum add_or_remove);
static int compareMemberPriority(PQElement memberA, PQElement memberB);

static void emPrintEvent(Event event, FILE* stream);
static void printDate(Date date, FILE* stream);
static void printEventMemberPq(PriorityQueue members, FILE* stream);
static int dateCompareEarliestFirst(Date date1, Date date2);

static int dateCompareEarliestFirst(Date date1, Date date2)
{
    return -dateCompare(date1, date2);
}
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
        free(event->name);
		free(event);
		return NULL;
	}
    event->memberPQ = pq;
    Date date_copy = dateCopy(date);
    if(!date_copy)
    {
        free(event->name);
        pqDestroy(event->memberPQ);
        free(event);
        return NULL;
    }
    event->date = date_copy;
    event->id = id;
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
    if(newEvent->memberPQ) //check if null
    {
        pqDestroy(newEvent->memberPQ);
    }
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

static int compareMemberPriority(PQElement memberA, PQElement memberB)
{
    Member member1 = (Member) memberA;
    Member member2 = (Member) memberB;
    int eventsDelta = member1->events_number - member2->events_number;
    if(eventsDelta)
    {
        return eventsDelta;
    }
    return member1->id - member2->id;
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
    member->id = id;
	member->events_number = 0;
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
    Member member_copy = (PQElement) memberCreate(memberNew->id, memberNew->name);
    if(!member_copy)
    {
        return NULL;
    }
    member_copy->events_number = memberNew->events_number;
    return member_copy;
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
    free(member);
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
        destroyEventManager(em);
        return NULL;
    }
    em->events = pqCreate(eventCopy, eventDestroy, eventsEqual,
						(PQElementPriority (*)(PQElementPriority)) dateCopy, 
                        (void (*) (PQElementPriority)) dateDestroy,
                        (int (*) (PQElementPriority, PQElementPriority)) dateCompareEarliestFirst);
    if(!em->events)
    {
        destroyEventManager(em);
        return NULL;
    }
    em->members = pqCreate(memberCopy, memberFree, membersEqual,
							memberCopy, memberFree, compareMemberPriority);
	if(!em->members)
	{
		destroyEventManager(em);
		return NULL;
	}
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
	Member new_member = memberCreate(member_id, member_name);
    if(!new_member)
    {
        return EM_OUT_OF_MEMORY;
    }
	
    if(pqContains(em->members, new_member))
    {
		memberFree(new_member);
        return EM_MEMBER_ID_ALREADY_EXISTS;
    }
    PriorityQueueResult pqResult = pqInsert(em->members, new_member, new_member);
	if(pqResult == PQ_OUT_OF_MEMORY)
	{
		memberFree(new_member);
		return EM_OUT_OF_MEMORY;
	}
	assert(pqResult == PQ_SUCCESS);
	memberFree(new_member);
    return EM_SUCCESS;
}

static EventManagerResult emMemberChangePriority(EventManager em, int member_id, memberEnum add_or_remove)
{
	Member memberTmp = NULL;	
	emFindMember(em, member_id, &memberTmp);//member should exist 
    Member member = (Member) memberCopy(memberTmp);
	if(!member)
	{
		return EM_OUT_OF_MEMORY;
	}
    PriorityQueueResult result = pqRemoveElement(em->members, memberTmp);//delete the member in PQ  
	memberTmp = NULL; //before this line points to garbage
    assert(result == PQ_SUCCESS);
	assert(member->id == member_id);
    int new_number;
	switch(add_or_remove)
	{
		case MEMBER_ADD_EVENT:
			new_number = member->events_number + 1;
            break;
		case MEMBER_REMOVE_EVENT:
			new_number = member->events_number - 1;
            break;
		default:
			assert(1 == 0);// I probably fucked up using the enum
	};
	member->events_number = new_number;
	assert(0 <= new_number);
    
    result = pqInsert(em->members, member, member);
	if(result == PQ_NULL_ARGUMENT)
	{
        memberFree(member);
		return EM_NULL_ARGUMENT;
	}
	assert(result == PQ_SUCCESS);

    memberFree(member);
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
	assert(result == EM_SUCCESS);
	
    Member member = NULL;
	result = emFindMember(em, member_id, &member);
    if(result == EM_MEMBER_ID_NOT_EXISTS)
    {
        return EM_MEMBER_ID_NOT_EXISTS;
    
    }
	assert(result == EM_SUCCESS);
	
    if(pqContains(event->memberPQ, member))
    {
        return EM_EVENT_AND_MEMBER_ALREADY_LINKED;
    }
	
	PriorityQueueResult pqResult = pqInsert(event->memberPQ, member, &member->id);
    if(pqResult == PQ_OUT_OF_MEMORY)
	{
		return EM_OUT_OF_MEMORY;
	}
	assert(pqResult == PQ_SUCCESS);
	result = emMemberChangePriority(em, member_id, MEMBER_ADD_EVENT);
	if(result == EM_OUT_OF_MEMORY)
    {
        return EM_OUT_OF_MEMORY;
    }
	assert(result == EM_SUCCESS);
	return EM_SUCCESS;
}

EventManagerResult emRemoveMemberFromEvent(EventManager em, int member_id, int event_id)
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
	assert(result == EM_SUCCESS);
    Member member = NULL;
	result = emFindMember(em, member_id, &member);
    if(result == EM_MEMBER_ID_NOT_EXISTS)
    {
        return EM_MEMBER_ID_NOT_EXISTS;
    }
	assert(result == EM_SUCCESS);
	
    if(!pqContains(event->memberPQ, member))
    {
        return EM_EVENT_AND_MEMBER_NOT_LINKED;
    }
	PriorityQueueResult pqResult = pqRemoveElement(event->memberPQ, member);
    if(pqResult == PQ_OUT_OF_MEMORY)
    {
        return EM_OUT_OF_MEMORY;
    }
	assert(pqResult == PQ_SUCCESS);
	result = emMemberChangePriority(em, member_id, MEMBER_REMOVE_EVENT);
	if(result == EM_OUT_OF_MEMORY)
    {
        return EM_OUT_OF_MEMORY;
    }
	assert(result == EM_SUCCESS);
	return EM_SUCCESS;
}
static bool eventNameAndDateinPQ(EventManager em, Event event)
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
//like above but uses external date 
static bool nameAndDateinPQEvent(EventManager em, char *name, Date date)
{
    PQ_FOREACH(Event, iteratorEvent, em->events)
    {
        if(dateCompare(iteratorEvent->date, date) == 0 && strcmp(iteratorEvent->name, name) == 0)
        {
            return true;
        }
    }
    return false;
}
static EventManagerResult eventAdd(EventManager em, Event event, Date date)
{
    if(eventNameAndDateinPQ(em, event))
    {
        return EM_EVENT_ALREADY_EXISTS;
    }
    if(pqContains(em->events, event))
    {
        return EM_EVENT_ID_ALREADY_EXISTS;
    }

    PriorityQueueResult pqResult = pqInsert(em->events, event, date);
    //arguments cant be null because we already checked them
    assert(pqResult != PQ_NULL_ARGUMENT);
    switch(pqResult)
    {
        case PQ_OUT_OF_MEMORY:
            return EM_OUT_OF_MEMORY;
        case PQ_SUCCESS:
            return EM_SUCCESS;
        default:
            return EM_ERROR;
    }
}

EventManagerResult emAddEventByDate(EventManager em, char* event_name, Date date, int event_id)
{
    if(!em || !event_name || !date)
    {
        return EM_NULL_ARGUMENT;
    }
    if(dateCompare(date, em->currentDate) < 0)
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

    EventManagerResult emResult = eventAdd(em, event, date); 
    eventDestroy(event);
    return emResult;
}

EventManagerResult emAddEventByDiff(EventManager em, char* event_name, int days, int event_id)
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
    
    EventManagerResult emResult = eventAdd(em, event, newDate);
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
        case PQ_SUCCESS:
            return EM_SUCCESS;
        default:
            return EM_ERROR;

    }
}

static EventManagerResult emFindEvent(EventManager em, int id, Event* event_p)
{
    assert(event_p != NULL);//can't derefence NULL
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
            eventDestroy(blankEvent);
            *event_p = iteratorEvent;
            return EM_SUCCESS;
        }
    }
    eventDestroy(blankEvent);
    return EM_EVENT_ID_NOT_EXISTS;
}

static EventManagerResult emFindMember(EventManager em, int id, Member *member_p)
{
    if(!em)
    {
        return EM_NULL_ARGUMENT;
    }
    PQ_FOREACH(Member, iterator, em->members)
    {
        if(id == iterator->id)
        {
            *member_p = iterator;
            return EM_SUCCESS;
        }
    }
    return EM_MEMBER_ID_NOT_EXISTS;
}

EventManagerResult emChangeEventDate(EventManager em, int event_id, Date new_date)
{
    if(!em || !new_date)
    {
        return EM_NULL_ARGUMENT;
    }
    if(dateCompare(new_date, em->currentDate) < 0)
    {
        return EM_INVALID_DATE;
    }
    if(event_id < 0)
    {
        return EM_INVALID_EVENT_ID;
    }
    Event eventToChange = NULL;
    EventManagerResult emResult = emFindEvent(em, event_id, &eventToChange);
    
    //args can't be null
    assert(emResult != EM_NULL_ARGUMENT);
    
    switch(emResult)
    {
        case EM_OUT_OF_MEMORY:
            return EM_OUT_OF_MEMORY;
            
        case EM_SUCCESS: ;//intentional semicolon
            //ZEEV i rewrote this part but i'm not 100% sure it doesn't have weird side effects
            //check that there isn't already an event by that name
            Date old_date = eventToChange->date;
            if(nameAndDateinPQEvent(em, eventToChange->name, new_date))
            {
                return EM_EVENT_ALREADY_EXISTS;
            }
            //change date on element
            eventToChange->date = new_date;
            
            //change the date
            PriorityQueueResult pqResult = pqChangePriority(em->events, eventToChange,
            old_date, new_date);
            //args wont be null cause we already checked them
            assert(pqResult != PQ_NULL_ARGUMENT);
            //element exists beacuse we already found it
            assert(pqResult != PQ_ELEMENT_DOES_NOT_EXISTS);
            
            
            switch(pqResult)
            {
                case PQ_OUT_OF_MEMORY:
                    eventToChange->date = old_date;
                    return EM_OUT_OF_MEMORY;
                case PQ_SUCCESS:
                    dateDestroy(old_date);
                    return EM_SUCCESS;
                default:
                    eventToChange->date = old_date;
                    return EM_ERROR;
            }
            
        case EM_EVENT_ID_NOT_EXISTS:
            return EM_EVENT_ID_NOT_EXISTS;
        default:
            return EM_ERROR;
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
        Event first = (Event) pqGetFirst(em->events);
        while(first && dateCompare(first->date, em->currentDate) < 0)
        {
            pqRemove(em->events);
            first = (Event) pqGetFirst(em->events);
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
    Event next = pqGetFirst(em->events);
    if(!next)
    {
        return NULL;
    }
    char *name = (char *) malloc(strlen(next->name) + 1);
    strcpy(name, next->name);
    return name;
}


void emPrintAllResponsibleMembers(EventManager em, const char* file_name)
{
    int member_counter = 0;
    int memberPQSize = pqGetSize(em->members);
    FILE* stream = fopen(file_name, "w");
    if(!stream)
    {
        return;
    }
    PQ_FOREACH(Member, iterator, em->members)
    {
        member_counter++;
        fprintf(stream, "%s,%d", iterator->name, iterator->events_number);
        if(member_counter != memberPQSize)
        {
            fprintf(stream, "\n");
        }
        
    }
    fclose(stream);
}
void emPrintAllEvents(EventManager em, const char* file_name)
{
    int event_counter = 0;
    int eventPQSize = pqGetSize(em->events);
    FILE* stream = fopen(file_name, "w");
    if(!stream)
    {
        return;
    }
    PQ_FOREACH(Event, iteratorEvent, em->events)
    {
        event_counter++;
        emPrintEvent(iteratorEvent, stream);
        if(event_counter != eventPQSize)
        {
            fprintf(stream, "\n");
        }
    }
    fclose(stream);
}

static void emPrintEvent(Event event, FILE* stream)
{
    fprintf(stream, "%s,", event->name);
    printDate(event->date, stream);
    printEventMemberPq(event->memberPQ, stream);
}

static void printDate(Date date, FILE* stream)
{
    int day, month, year;
    dateGet(date, &day, &month, &year);
    fprintf(stream, "%d.%d.%d", day, month, year);
}

static void printEventMemberPq(PriorityQueue members, FILE* stream)
{
    // int size = pqGetSize(members);
    // int memberCounter = 0;
    PQ_FOREACH(Member, iteratorMember, members)
    {
        fprintf(stream, ",");
        fprintf(stream, "%s", iteratorMember->name);
    }
}