#include <stdlib.h>
#include <assert.h>

#include "priority_queue.h"

typedef struct ListNode
{
   PQElement element;
   PQElementPriority elementPriority;
   struct ListNode* next;
}*Node;


typedef struct PriorityQueue_t
{
    int size;
    Node list;

    Node iteratorCurrentPosition;

    CopyPQElement copyElementFunction;
    FreePQElement freeElementFunction;
    EqualPQElements isEqualElementFunction;
    CopyPQElementPriority copyPriorityFunction;
    FreePQElementPriority freeElementPriority; 
    ComparePQElementPriorities comparePrioritiesFunction;
}PQ;

static Node createNode(PQElement element, PQElementPriority elementPriority);
// static bool nodeValuesEqual(PriorityQueue queue, Node first, Node second);
static void destroyNode(PriorityQueue queue, Node node);
static Node copyNode(PriorityQueue queue, Node old);
static Node copyList(PriorityQueue queue);
static void destroyList(PriorityQueue queue, Node node);

static Node createNode(PQElement element, PQElementPriority elementPriority)
{
    Node node = malloc(sizeof(*node));
    if(node == NULL)
    {
        return NULL;
    }
    node->element = element;
    node->elementPriority = elementPriority;
    node->next = NULL;
    return node;
}

static void destroyNode(PriorityQueue queue, Node node)
{
    node->next = NULL;
    queue->freeElementFunction(node->element);
    queue->freeElementPriority(node->elementPriority);
    free(node);
}


// static bool nodeValuesEqual(PriorityQueue queue, Node first, Node second)
// {
//     return queue->isEqualElementFunction(first->element, second->element) &&
//      queue->comparePrioritiesFunction(first->elementPriority, second->elementPriority) == 0;
// }

static Node copyNode(PriorityQueue queue, Node old)
{
    Node new_node = malloc(sizeof(*new_node));
    if(new_node == NULL)
    {
        return NULL;
    }
    new_node->element = queue->copyElementFunction(old->element);
	if(new_node->element == NULL)
	{
		free(new_node);
		return NULL;
	}
    new_node->elementPriority = queue->copyPriorityFunction(old->elementPriority);
	if(new_node->elementPriority == NULL)
	{
		queue->freeElementFunction(new_node->element);
		free(new_node);
		return NULL;
	}
    new_node->next = NULL;
    return new_node;
}

//returns a pointer to a copy of queue->list
static Node copyList(PriorityQueue queue)
{
    if(queue == NULL || queue->list == NULL)
    {
        return NULL;
    }
    Node new_list = copyNode(queue, queue->list);
    if(new_list == NULL)
    {
        return NULL;// malloc failed in copyNode()
    }

    Node tmp = new_list;
    Node current = queue->list->next;  
    while(current)
    {
        assert(current != NULL);
        tmp->next = copyNode(queue, current);
        tmp = tmp->next;
        if(tmp == NULL)
        {
            destroyList(queue, new_list);
            return NULL;// malloc failed in copyNode()
        }
        current = current->next;
    }
    return new_list;
}





PriorityQueue pqCreate(CopyPQElement copy_element,
                       FreePQElement free_element,
                       EqualPQElements equal_elements,
                       CopyPQElementPriority copy_priority,
                       FreePQElementPriority free_priority,
                       ComparePQElementPriorities compare_priorities)
{
    PriorityQueue queue = (PriorityQueue) malloc(sizeof(*queue));
    if(queue == NULL)
    {
        return NULL;
    }
    queue->list = NULL;
    queue->size = 0;
    queue->iteratorCurrentPosition = NULL;
    queue->copyElementFunction = copy_element;
    queue->freeElementFunction = free_element;
    queue->isEqualElementFunction = equal_elements;
    queue->copyPriorityFunction = copy_priority;
    queue->freeElementPriority = free_priority;
    queue->comparePrioritiesFunction = compare_priorities;
    
    return queue;
}
static void destroyList(PriorityQueue queue, Node node)
{
    Node current = node;
    Node tmp;
    while(current)//NULL safe
    {
        tmp = current;
        current = current->next;
        destroyNode(queue, tmp);
    }
}
void pqDestroy(PriorityQueue queue)
{
    destroyList(queue, queue->list);
    free(queue);   
}


PriorityQueue pqCopy(PriorityQueue queue)
{
    if(queue == NULL)
    {
        return NULL;
    }
    PriorityQueue new_queue = pqCreate(queue->copyElementFunction,
            queue->freeElementFunction,
            queue->isEqualElementFunction,
            queue->copyPriorityFunction,
            queue->freeElementPriority,
            queue->comparePrioritiesFunction);
    if(new_queue == NULL)
    {
        return NULL;
    }
    if(queue->list != NULL)
    {
        new_queue->list = copyList(queue);
        if(new_queue->list == NULL)
        {
            pqDestroy(new_queue);
            return NULL; //malloc fail in copyNode()
        }
    }
    else
    {
        new_queue->list = NULL;
    }
    new_queue->size = queue->size;
    new_queue->iteratorCurrentPosition = new_queue->list;
    return new_queue;
}

PriorityQueueResult pqClear(PriorityQueue queue)
{
	if(queue == NULL)
	{
		return PQ_NULL_ARGUMENT;
	}
	destroyList(queue, queue->list);
	queue->list = NULL;
    queue->size = 0;
    queue->iteratorCurrentPosition = NULL;
	return PQ_SUCCESS; 
}

int pqGetSize(PriorityQueue queue)
{
    assert(queue);
    return queue->size;
}


//needs to be changed so it keeps the queue ordered
PriorityQueueResult pqInsert(PriorityQueue queue, PQElement element, PQElementPriority priority)
{
    //Check arguments are not null
    if(queue == NULL || element == NULL || priority == NULL)
    {
        return PQ_NULL_ARGUMENT;
    }

    //Copy element and priority and check they are not null
    PQElement elementCopy = queue->copyElementFunction(element);
    if (elementCopy == NULL)
    {
        return PQ_OUT_OF_MEMORY;
    }
    
    PQElementPriority elementPriorityCopy = queue->copyPriorityFunction(priority);

    if (elementPriorityCopy == NULL)
    {
        return PQ_OUT_OF_MEMORY;
    }

    //create node and check it's not null
    Node node = createNode(elementCopy, elementPriorityCopy);

    if (node == NULL)
    {
        return PQ_OUT_OF_MEMORY;
    }

    //Find the node that our node needs to be inserted after
    Node current = queue->list;
    
    //the list is empty
    if (!current)
    {
        queue->list = node;
    }
    else
    {
        //If the prioriy of the element added is bigger than the priority
        // of the current first element, make it the first element
        if(queue->comparePrioritiesFunction(node->elementPriority, current->element) > 0)
        {
            node->next = current;
            queue->list = node;
        }
        else
        {
            //while the next node is not null and it's priority is bigger or equal to the priority of
            // the added element, we advance current and next

            //!!!need to check about order of insertion and stuff....
            Node next = current->next;
            while(next && queue->comparePrioritiesFunction(node->elementPriority, next->element) <= 0)
            {
                current = next;
                next = next->next;
            }
            //if next is null, we want to add the element to the end of the list
            if(!next)
            {
                current->next = node;
            }
            //else, we add the node after current and before next
            else
            {
                current->next = node;
                node->next = next;
            }

        }
        
    }

    queue->size++;
    return PQ_SUCCESS;
}

bool pqContains(PriorityQueue queue, PQElement element)
{
    if(!queue || !element)
    {
        return false;
    }
    
    Node current = queue->list;
    
    while(current)
    {
        if(queue->isEqualElementFunction(current->element, element))
        {
            return true;
        }
    }
    
    return false;
}


PriorityQueueResult pqRemoveElement(PriorityQueue queue, PQElement element)
{
    if(!queue || !element)
    {
        return PQ_NULL_ARGUMENT;
    }
    Node first = queue->list;
    
    if(!first)
    {
        return PQ_ELEMENT_DOES_NOT_EXISTS;
    }

    //The case in which the first element is the element to remove is addressed differently
    if(queue->isEqualElementFunction(element, first))
    {
        queue->list = first->next;
        destroyNode(queue, first);
        queue->size--;
        return PQ_SUCCESS;
    }

    Node current = first;
    Node next = current->next;
    while(next)
    {
        if(queue->isEqualElementFunction(next, element))
        {
            current->next = next->next;
            destroyNode(queue, next);
            queue->size--;
            return PQ_SUCCESS;
        }
        current = next;
        next = next->next;
    }
    return PQ_ELEMENT_DOES_NOT_EXISTS;
    
}

//not implemented yet
PriorityQueueResult pqChangePriority(PriorityQueue queue, PQElement element,
                                     PQElementPriority old_priority, PQElementPriority new_priority)
    {
        if(!queue || !element || !old_priority || !old_priority)
        {
            return PQ_NULL_ARGUMENT;
        }

        Node first = queue->list;
        if(!first)
        {
            return PQ_ELEMENT_DOES_NOT_EXISTS;
        }
        if(queue->comparePrioritiesFunction(first->elementPriority, old_priority) == 0 && 
            queue->isEqualElementFunction(first->element, element))
            {
                queue->list = first->next;
                destroyNode(queue, first);
                queue->size--;
                return pqInsert(queue, element, new_priority);
            }

        Node current = queue->list;
        Node next = current->next;

        while (next)
        {
            if(queue->comparePrioritiesFunction(next->elementPriority, old_priority) == 0 && 
            queue->isEqualElementFunction(next->element, element))
            {
                current->next = next->next;
                destroyNode(queue, next);
                return pqInsert(queue, element, new_priority);
            }
        }

        return PQ_ELEMENT_DOES_NOT_EXISTS;
    }

PriorityQueueResult pqRemove(PriorityQueue queue)
{
    if (!queue)
    {
        return PQ_NULL_ARGUMENT;
    }
	if(queue->list == NULL)
	{
		assert(queue->size == 0);
		return PQ_SUCCESS;//!!!!! what should we return if the queue is empty? 
	}
	Node tmp = queue->list;
	queue->list = queue->list->next;
	if(queue->iteratorCurrentPosition == tmp)
	{
		queue->iteratorCurrentPosition = NULL;
	}
	destroyNode(queue, tmp);
	queue->size--;
	return PQ_SUCCESS;
}

PQElement pqGetFirst(PriorityQueue queue)
{
	if(queue == NULL || queue->size == 0)
	{
		return NULL;
	}
	queue->iteratorCurrentPosition = queue->list;
	return queue->iteratorCurrentPosition->element;
}

PQElement pqGetNext(PriorityQueue queue)
{
	 if(queue == NULL || queue->size == 0)
	{
		return NULL;
	}
	queue->iteratorCurrentPosition = queue->iteratorCurrentPosition->next;
	if(queue->iteratorCurrentPosition == NULL)
	{
		return NULL;
	}
	return queue->iteratorCurrentPosition->element;
}

