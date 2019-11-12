/* My402List
Author: harshit
*/



#include <stdio.h>
#include <stdlib.h>

#include "my402list.h"



/******************************************************/




int My402ListInit(My402List* list)
{
	list->num_members = 0;
	list->anchor.next = &(list->anchor);
	list->anchor.prev = &(list->anchor);
	return 0;
}



int My402ListLength(My402List* list)
{
	return list->num_members;
}



int My402ListEmpty(My402List* list)
{
	if (list->anchor.next == &(list->anchor))
		return TRUE;
	else
	return FALSE;

}



int My402ListAppend(My402List* list, void* new_obj)
{
	My402ListElem* node = (My402ListElem*) malloc(sizeof(My402ListElem));

	if(node == NULL)
	{
		return FALSE;
	}
	
	if(list->anchor.next == &(list->anchor))
	{
		node->prev = &(list->anchor);
		node->next = &(list->anchor);
		list->anchor.next = node;
		list->anchor.prev = node;
	}
	else
	{
		node->next = &(list->anchor);
		node->prev = (list->anchor.prev);
		(list->anchor.prev)->next = node;
		list->anchor.prev = node;
	}

	node->obj = new_obj;
	list->num_members++;

	return TRUE;

}


int My402ListPrepend(My402List* list, void* new_obj)
{
	My402ListElem* node = (My402ListElem*)malloc(sizeof(My402ListElem));

	if(node == NULL)
	{
		return FALSE;
	}

	if(list->anchor.next == &(list->anchor))
	{
		node->prev = &(list->anchor);
		node->next = &(list->anchor);
		list->anchor.next = node;
		list->anchor.prev = node;
	}
	else
	{
		node->next = (list->anchor.next);
		node->prev = &(list->anchor);
		(list->anchor.next)->prev = node;
		list->anchor.next = node;
	}

	node->obj = new_obj;
	list->num_members++;

	return TRUE;
}


void My402ListUnlink(My402List* list,My402ListElem* node)
{
	node->next->prev = node->prev;
	node->prev->next = node->next;
	free(node);
	list->num_members--;
}


void My402ListUnlinkAll(My402List* list)
{
	My402ListElem* elem;

	while(list->num_members!=0)
	{
		elem = list->anchor.next;
		list->anchor.next = elem->next;
		free(elem);
		list->num_members--;
	}
}




int  My402ListInsertAfter(My402List* list,void* new_obj,My402ListElem* node)
{
	if(list->anchor.next == &(list->anchor))
		My402ListAppend(list,new_obj);
	else
	{
		My402ListElem* new_node = (My402ListElem*)malloc(sizeof(My402ListElem));

		if(new_node == NULL)
		return FALSE;
		
		new_node->next = node->next;
		new_node->prev = node;
		(node->next)->prev = new_node;
		node->next = new_node;
		new_node->obj = new_obj;
		list->num_members++;
	}
	return TRUE;
}


int  My402ListInsertBefore(My402List* list,void* new_obj,My402ListElem* node)
{
	if (list->anchor.next == &(list->anchor))
		My402ListPrepend(list,new_obj);
	else
	{
		My402ListElem* new_node = (My402ListElem*)malloc(sizeof(My402ListElem));

		if(new_node == NULL)
			return FALSE;

		new_node->prev = (node->prev);
		new_node->next = node;
		node->prev = new_node;
		new_node->prev->next = new_node;
		new_node->obj = new_obj;
		list->num_members++;
	}
	return TRUE;	
}


My402ListElem *My402ListFirst(My402List* list)
{
	if(list->num_members==0)
		return NULL;
	else
		return (list->anchor.next);
}


My402ListElem *My402ListLast(My402List* list)
{
	if(list->anchor.next == &(list->anchor))
		return NULL;
	else
		return (list->anchor.prev);
}


My402ListElem *My402ListNext(My402List* list, My402ListElem* node)
{
	if(node->next == &(list->anchor))
		return NULL;
	else
	return(node->next);	
}


My402ListElem *My402ListPrev(My402List* list,My402ListElem* node)
{
	if(node->prev ==&(list->anchor))
		return NULL;
	else
	return(node->prev);	
}


My402ListElem *My402ListFind(My402List* list, void* new_obj)
{
        My402ListElem *elem=NULL;
        for (elem=My402ListFirst(list) ; elem != NULL ; elem=My402ListNext(list, elem))
        {
            if(new_obj == elem->obj)
            	return elem;
        }
        return NULL;
}
