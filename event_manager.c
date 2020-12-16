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
    PriorityQueue member_pq; 
}*Event;

struct EventManager_t
{
    Date current_date;
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
static Event eventCreateNoEmptyPQ(Date date, int id, char* name);
static Event createBlankEvent(int id);
static PQElement eventCopy(PQElement event);
static void eventDestroy(PQElement event);
static bool eventsEqual(PQElement event1, PQElement event2);

static bool eventNameAndDateinPQ(EventManager em, Event event);
static bool nameAndDateinPQEvent(EventManager em, char *name, Date date);
static EventManagerResult eventAdd(EventManager em, Event event, Date date);
static EventManagerResult emFindEvent(EventManager em, int id, Event *event_p);
static EventManagerResult emFindMember(EventManager em, int id, Member *member_p);
static EventManagerResult emRemoveAllMembersFromEvent(EventManager em, Event event);

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
    return -(*(int *) n1 - *(int *) n2);
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
        eventDestroy(event);
		return NULL;
	}
    event->member_pq = pq;
    Date date_copy = dateCopy(date);
    if(!date_copy)
    {
        eventDestroy(event);
        return NULL;
    }
    event->date = date_copy;
    event->id = id;
	return event;
}

static Event eventCreateNoEmptyPQ(Date date, int id, char* name)
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

    Date date_copy = dateCopy(date);
    if(!date_copy)
    {
        eventDestroy(event);
        return NULL;
    }
    event->member_pq = NULL;
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
    event->member_pq = NULL;
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
	Event new_event = (Event) event;
    
	Event event_copy = eventCreateNoEmptyPQ(new_event->date, new_event->id, new_event->name);
	if(!event_copy)
	{
		return NULL;
	}
    if(new_event->member_pq == NULL)
    {
        return (PQElement) event_copy;
    }
    event_copy->member_pq = pqCopy(new_event->member_pq);
    if(!event_copy->member_pq)
    {
        eventDestroy(event_copy);
        return NULL;
    }
	return (PQElement) event_copy;
}

static void eventDestroy(PQElement event)
{
    Event new_event = (Event) event;
    pqDestroy(new_event->member_pq);
	dateDestroy(new_event->date);
    free(new_event->name);
    free(new_event);
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
    Event new_event1 = (Event) event1;
    Event new_event2 = (Event) event2;

    return new_event1->id == new_event2->id;
}

static int compareMemberPriority(PQElement memberA, PQElement memberB)
{
    Member member1 = (Member) memberA;
    Member member2 = (Member) memberB;
    int events_delta = member1->events_number - member2->events_number;
    if(events_delta)
    {
        return events_delta;
    }
    return -(member1->id - member2->id);
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
    Member member_new = (Member) member;
    if(!member_new->name){
        return NULL;
    }
    Member member_copy = (PQElement) memberCreate(member_new->id, member_new->name);
    if(!member_copy)
    {
        return NULL;
    }
    member_copy->events_number = member_new->events_number;
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
    Member new_member1 = (Member) member1;
    Member new_member2 = (Member) member2;
    return new_member1->id == new_member2->id;
}

static void memberFree(PQElement member)
{
    Member member_new = (Member) member;
    free(member_new->name);
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
    em->current_date = dateCopy(date);
    if(!em->current_date)
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
	dateDestroy(em->current_date);
	pqDestroy(em->events);
	pqDestroy(em->members);
    free(em);
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
    PriorityQueueResult pq_result = pqInsert(em->members, new_member, new_member);
	if(pq_result == PQ_OUT_OF_MEMORY)
	{
		memberFree(new_member);
		return EM_OUT_OF_MEMORY;
	}
	assert(pq_result == PQ_SUCCESS);
	memberFree(new_member);
    return EM_SUCCESS;
}

static EventManagerResult emMemberChangePriority(EventManager em, int member_id, memberEnum add_or_remove)
{
	Member memberTmp = NULL; //Shay, maybe you have an ides for another name to this variable?
	emFindMember(em, member_id, &memberTmp);//member should exist 
    Member member = (Member) memberCopy(memberTmp);
	if(!member)
	{
		return EM_OUT_OF_MEMORY;
	}
    PriorityQueueResult result = pqRemoveElement(em->members, memberTmp);//delete the member in PQ  
	memberTmp = NULL; //before this line points to garbage SHAY - why is this neccesary?
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
	
    if(pqContains(event->member_pq, member))
    {
        return EM_EVENT_AND_MEMBER_ALREADY_LINKED;
    }
	
	PriorityQueueResult pq_result = pqInsert(event->member_pq, member, &member->id);
    if(pq_result == PQ_OUT_OF_MEMORY)
	{
		return EM_OUT_OF_MEMORY;
	}
	assert(pq_result == PQ_SUCCESS);
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
	
    if(!pqContains(event->member_pq, member))
    {
        return EM_EVENT_AND_MEMBER_NOT_LINKED;
    }
	PriorityQueueResult pq_result = pqRemoveElement(event->member_pq, member);
    if(pq_result == PQ_OUT_OF_MEMORY)
    {
        return EM_OUT_OF_MEMORY;
    }
	assert(pq_result == PQ_SUCCESS);
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
    PQ_FOREACH(Event, iterator_event, em->events)
    {
        if(dateCompare(iterator_event->date, event->date) == 0 && strcmp(iterator_event->name, event->name) == 0)
        {
            return true;
        }
    }
    return false;
}
//like above but uses external date 
static bool nameAndDateinPQEvent(EventManager em, char *name, Date date)
{
    PQ_FOREACH(Event, iterator_event, em->events)
    {
        if(dateCompare(iterator_event->date, date) == 0 && strcmp(iterator_event->name, name) == 0)
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

    PriorityQueueResult pq_result = pqInsert(em->events, event, date);
    //arguments cant be null because we already checked them
    assert(pq_result != PQ_NULL_ARGUMENT);
    switch(pq_result)
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
    if(dateCompare(date, em->current_date) < 0)
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

    EventManagerResult em_result = eventAdd(em, event, date); 
    eventDestroy(event);
    return em_result;
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
    Date new_date = dateCopy(em->current_date);
    if(!new_date)
    {
        return EM_OUT_OF_MEMORY;
    }
    
    for (int i = 0; i < days; i++)
    {
        dateTick(new_date);
    }

    Event event = eventCreate(new_date, event_id, event_name);
    if(!event)
    {
        dateDestroy(new_date);
        return EM_OUT_OF_MEMORY;
    }
    
    EventManagerResult em_result = eventAdd(em, event, new_date);
    dateDestroy(new_date);
    eventDestroy(event);
    return em_result;
}

static EventManagerResult emRemoveAllMembersFromEvent(EventManager em, Event event)
{
    if(!em || !event)
    {
        return EM_NULL_ARGUMENT;
    }
    Member first = pqGetFirst(event->member_pq);
    while(first)
    {
        EventManagerResult em_result = emRemoveMemberFromEvent(em, first->id, event->id);
        if(em_result == EM_OUT_OF_MEMORY)
        {
            return EM_OUT_OF_MEMORY;
        }
        assert(em_result == EM_SUCCESS);
        first = pqGetFirst(event->member_pq);
    }
    return EM_SUCCESS;
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
    Event event_to_remove = createBlankEvent(event_id);
    if(!event_to_remove)
    {
        return EM_OUT_OF_MEMORY;
    }

    bool event_found = false;
    EventManagerResult em_result;

    //find event in pq to remove it's members
    PQ_FOREACH(Event, iterator_event, em->events)
    {
        if(eventsEqual(iterator_event, event_to_remove))
        {
            em_result = emRemoveAllMembersFromEvent(em, iterator_event);
            if(em_result == EM_OUT_OF_MEMORY)
            {
                eventDestroy(event_to_remove);
                return EM_OUT_OF_MEMORY;
            }
            assert(em_result == EM_SUCCESS);
            event_found = true;
            break;
        }
    }
    if(!event_found)
    {
        eventDestroy(event_to_remove);
        return EM_EVENT_NOT_EXISTS;
    }
    assert(em_result == EM_SUCCESS);


    PriorityQueueResult pq_result = pqRemoveElement(em->events, event_to_remove);
    eventDestroy(event_to_remove);
    //we already checked for null args
    assert(pq_result != PQ_NULL_ARGUMENT);
    //if the element does not exist we alresy know it from going over the pq
    assert(pq_result != PQ_ELEMENT_DOES_NOT_EXISTS);
    switch(pq_result)
    {
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
    Event blank_event = createBlankEvent(id);
    if(!blank_event)
    {
        return EM_OUT_OF_MEMORY;
    }

    PQ_FOREACH(Event, iterator_event, em->events)
    {
        if(eventsEqual(iterator_event, blank_event))
        {
            eventDestroy(blank_event);
            *event_p = iterator_event;
            return EM_SUCCESS;
        }
    }
    eventDestroy(blank_event);
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
    if(dateCompare(new_date, em->current_date) < 0)
    {
        return EM_INVALID_DATE;
    }
    if(event_id < 0)
    {
        return EM_INVALID_EVENT_ID;
    }
    Event event_to_change = NULL;
    EventManagerResult em_result = emFindEvent(em, event_id, &event_to_change);
    if(em_result != EM_SUCCESS)
    {
        return em_result;
    }
    assert(em_result == EM_SUCCESS || em_result == EM_EVENT_ID_NOT_EXISTS || em_result == EM_OUT_OF_MEMORY);
    if(nameAndDateinPQEvent(em, event_to_change->name, new_date))
    {
        return EM_EVENT_ALREADY_EXISTS;
    }
    Date old_date = event_to_change->date;
    PriorityQueueResult pq_result = pqChangePriority(em->events, event_to_change,
    old_date, new_date);//change the date
    if(pq_result == PQ_OUT_OF_MEMORY)
    {
        return EM_OUT_OF_MEMORY;
    }
    assert(pq_result == PQ_SUCCESS);
    //now we change the date in the event elemnt itself
    event_to_change = NULL;
    em_result = emFindEvent(em, event_id, &event_to_change);
    if(em_result != EM_SUCCESS)
    {
        return em_result;
    }
    assert(em_result == EM_SUCCESS || em_result == EM_EVENT_ID_NOT_EXISTS || em_result == EM_OUT_OF_MEMORY);
    dateDestroy(event_to_change->date);
    Date date_copy = dateCopy(new_date);
    if(!date_copy)
    {
        return EM_OUT_OF_MEMORY;
    }
    event_to_change->date = date_copy;
    return EM_SUCCESS;
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
        dateTick(em->current_date);
        Event first = (Event) pqGetFirst(em->events);
        while(first && dateCompare(first->date, em->current_date) < 0)
        {
            EventManagerResult em_result = emRemoveAllMembersFromEvent(em, first);
            if(em_result == EM_OUT_OF_MEMORY)
            {
                return EM_OUT_OF_MEMORY;
            }
            assert(em_result == EM_SUCCESS);

            // Commented this code becuase it does not compile with DNDEBUG, because pq_result is not used
            // PriorityQueueResult pq_result =  pqRemove(em->events);
            // assert(pq_result == PQ_SUCCESS);
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
    return next->name;
}


void emPrintAllResponsibleMembers(EventManager em, const char* file_name)
{
    int member_counter = 0;
    int member_pq_size = pqGetSize(em->members);
    FILE* stream = fopen(file_name, "w");
    if(!stream)
    {
        return;
    }
    PQ_FOREACH(Member, iterator, em->members)
    {
        member_counter++;
        if(iterator->events_number > 0)
        {
            fprintf(stream, "%s,%d", iterator->name, iterator->events_number);
            if(member_counter != member_pq_size)
            {
                fprintf(stream, "\n");
            }
        }
        
    }
    fclose(stream);
}
void emPrintAllEvents(EventManager em, const char* file_name)
{
    int event_counter = 0;
    int event_pq_size = pqGetSize(em->events);
    FILE* stream = fopen(file_name, "w");
    if(!stream)
    {
        return;
    }
    PQ_FOREACH(Event, iterator_event, em->events)
    {
        event_counter++;
        emPrintEvent(iterator_event, stream);
        if(event_counter != event_pq_size)
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
    printEventMemberPq(event->member_pq, stream);
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
    PQ_FOREACH(Member, iterator_member, members)
    {
        fprintf(stream, ",");
        fprintf(stream, "%s", iterator_member->name);
    }
}