#include <stdlib.h>
#include <assert.h>
#include "priority_queue.h"

typedef struct ListNode
{
    PQElement element;
    PQElementPriority element_priority;
    struct ListNode* next;
}*Node;

struct PriorityQueue_t
{
    int size;
    Node list;
    Node iterator_current_position;

    CopyPQElement copyElementFunction;
    FreePQElement freeElementFunction;
    EqualPQElements isEqualElementFunction;
    CopyPQElementPriority copyPriorityFunction;
    FreePQElementPriority freeElementPriority; 
    ComparePQElementPriorities comparePrioritiesFunction;
};

static Node createNode(PQElement element, PQElementPriority element_priority);
static void destroyNode(PriorityQueue queue, Node node);
static Node copyNode(PriorityQueue queue, Node old);
static Node copyList(PriorityQueue queue);
static void destroyList(PriorityQueue queue, Node node);
static bool isNodeEqual(PriorityQueue queue, Node a, PQElement e, PQElementPriority p);

//allocates a node that contains an element and a priority
static Node createNode(PQElement element, PQElementPriority element_priority)
{
    Node node = malloc(sizeof(*node));
    if(node == NULL)
    {
        return NULL;
    }
    node->element = element;
    node->element_priority = element_priority;
    node->next = NULL;
    return node;
}

//frees given node using functions given by user
static void destroyNode(PriorityQueue queue, Node node)
{
    node->next = NULL;
    queue->freeElementFunction(node->element);
    queue->freeElementPriority(node->element_priority);
    free(node);
}

//copies given node using functions given by user
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
    new_node->element_priority = queue->copyPriorityFunction(old->element_priority);
    if(new_node->element_priority == NULL)
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
    queue->iterator_current_position = NULL;
    queue->copyElementFunction = copy_element;
    queue->freeElementFunction = free_element;
    queue->isEqualElementFunction = equal_elements;
    queue->copyPriorityFunction = copy_priority;
    queue->freeElementPriority = free_priority;
    queue->comparePrioritiesFunction = compare_priorities;
    return queue;
}

//frees the whole node tree
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
    if(!queue)
    {
        return;
    }
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
    queue->iterator_current_position = NULL;
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
    new_queue->iterator_current_position = NULL;
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
    queue->iterator_current_position = NULL;
    return PQ_SUCCESS; 
}

int pqGetSize(PriorityQueue queue)
{
    if (!queue)
    {
        return -1;
    }
    return queue->size;
}

PriorityQueueResult pqInsert(PriorityQueue queue, PQElement element, PQElementPriority priority)
{
    if(queue == NULL || element == NULL || priority == NULL)
    {
        return PQ_NULL_ARGUMENT;
    }
    queue->iterator_current_position = NULL;
    PQElement element_copy = queue->copyElementFunction(element);
    if (element_copy == NULL)
    {
        return PQ_OUT_OF_MEMORY;
    }
    PQElementPriority element_priority_copy = queue->copyPriorityFunction(priority);
    if (!element_priority_copy)
    {
        queue->freeElementFunction(element_copy);
        return PQ_OUT_OF_MEMORY;
    }
    Node node = createNode(element_copy, element_priority_copy);
    if (!node)
    {
        queue->freeElementFunction(element_copy);
        queue->freeElementPriority(element_priority_copy);
        return PQ_OUT_OF_MEMORY;
    }
    Node current = queue->list;
    if (!current) //the list is empty
    {
        queue->list = node;
        queue->size++;
        return PQ_SUCCESS;
    } 
    if(queue->comparePrioritiesFunction(node->element_priority,current->element_priority) > 0)
    {
        node->next = current;
        queue->list = node;
        queue->size++;
        return PQ_SUCCESS;
    }
    while(current->next != NULL && 
    queue->comparePrioritiesFunction(node->element_priority,current->next->element_priority) <= 0)
    {
        current = current->next;
    }
    node->next = current->next;  
    current->next = node;
    assert(node->next != current->next);
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
        current = current->next;
    }
    return false;
}

PriorityQueueResult pqRemoveElement(PriorityQueue queue, PQElement element)
{
    if(!queue || !element)
    {
        return PQ_NULL_ARGUMENT;
    }
    queue->iterator_current_position = NULL;
    Node first = queue->list;
    
    if(!first)
    {
        return PQ_ELEMENT_DOES_NOT_EXISTS;
    }

    //The case in which the first element is the element to remove is addressed differently
    if(queue->isEqualElementFunction(element, first->element))
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
        if(queue->isEqualElementFunction(next->element, element))
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

//compares two nodes
static bool isNodeEqual(PriorityQueue queue, Node a, PQElement e, PQElementPriority p)
{
    if(!a || !p || !e || !queue)
    {
        return false;
    }
    bool prioritySame = queue->comparePrioritiesFunction(a->element_priority, p) == 0;
    bool elementSame = queue->isEqualElementFunction(a->element, e);
    return prioritySame && elementSame;
}

PriorityQueueResult pqChangePriority(PriorityQueue     queue, 
									 PQElement     	   element,
                                     PQElementPriority old_priority, 
									 PQElementPriority new_priority)
{
    if(!queue || !element || !old_priority || !old_priority)
    {
        return PQ_NULL_ARGUMENT;
    }
    queue->iterator_current_position = NULL;
    Node first = queue->list;
    if(!first)
    {
        return PQ_ELEMENT_DOES_NOT_EXISTS;
    }
    if(isNodeEqual(queue, first, element, old_priority))
	{   
		queue->list = first->next;
		PriorityQueueResult pq_result = pqInsert(queue, element, new_priority);
		if(pq_result != PQ_SUCCESS)
		{
			queue->list = first;//reversing changes
			return pq_result;
		}
		else
		{
			destroyNode(queue, first);
			queue->size--;
			return pq_result;
		}
	}
    Node current = queue->list;
    Node next = current->next;	
    while (next)
    {
        if(isNodeEqual(queue, next, element, old_priority))
        {
            current->next = next->next;	
            PriorityQueueResult pq_result = pqInsert(queue, element, new_priority);
            if(pq_result != PQ_SUCCESS)
            {
                current->next = next;//reversing changes
                return pq_result;
            }
            else
            {
                destroyNode(queue, next);
                queue->size--;
                return pq_result;
            }
        }
        current = next; 
        next = next->next;
    }
    return PQ_ELEMENT_DOES_NOT_EXISTS;
}

PriorityQueueResult pqRemove(PriorityQueue queue)
{
    if (!queue)
    {
        return PQ_NULL_ARGUMENT;
    }
    queue->iterator_current_position = NULL;
    if(queue->list == NULL)
    {
        assert(queue->size == 0);
        return PQ_SUCCESS;
    }
    Node tmp = queue->list;
    queue->list = queue->list->next;
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
    queue->iterator_current_position = queue->list;
    return queue->iterator_current_position->element;
}

PQElement pqGetNext(PriorityQueue queue)
{
    if(queue == NULL || queue->size == 0)
    {
        return NULL;
    }
    if(queue->iterator_current_position == NULL)
    {
        return NULL;
    }
    queue->iterator_current_position = queue->iterator_current_position->next;
    if(queue->iterator_current_position == NULL)
    {
        return NULL;
    }
    return queue->iterator_current_position->element;
}