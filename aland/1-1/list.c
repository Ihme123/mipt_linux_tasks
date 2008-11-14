//Work with lists

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define OOM(c,str) if(c==NULL) {printf("ERROR: Not enough memory (for %s). Terminating\n",str); exit(EXIT_SUCCESS);}

struct LIST
{
	int Delay;
	char **args;
	struct LIST *prev, *next;
};

//creating new element
struct LIST *ListCreate(int Delay, char **args, struct LIST *prev, struct LIST *next)
{
	struct LIST *tmp;
	tmp = calloc(1, sizeof(struct LIST));
	OOM(tmp,"New list element");
	tmp->Delay = Delay;
	tmp->args = args;
	tmp->prev = prev;
	tmp->next = next;
	return tmp;
}

//Just remove element, hope it exists
void ListRemoveVoid(struct LIST *list)
{
	int i = 0;
	if(list->prev!=NULL) (list->prev)->next = list->next;
	if(list->next!=NULL) (list->next)->prev = list->prev;
	while(list->args[i]!=NULL)
		free(list->args[i++]);
	free(list->args);
	free(list);
};

//Remove element and return pointer to previous one
struct LIST *ListRemove(struct LIST *list)
{
	if (list==NULL) return 0;
	struct LIST *tmp;
	tmp = list->prev;
	ListRemoveVoid(list);
	return tmp;
};

//Insert element 'list' after 'prev' and return pointer to it
struct LIST *ListInsert(struct LIST *list, struct LIST *prev)
{
	list->prev = prev;
	if(prev!=NULL)
	{
		list->next = prev->next;	
		if(prev->next!=NULL) (prev->next)->prev = list;
		prev->next = list;
	}
	else
		list->next = NULL;
	return list;
};

//Swap 2 elements, return nothing; arguments must be non-zero pointers to elements to be swapped.
//this &xxx... not sure...
void ListSwap(struct LIST *a, struct LIST *b)
{
	struct LIST *tmp;
	
	tmp = a->next;
	a->next = b->next;
	if (a->next!=NULL) (a->next)->prev = a;
	b->next = tmp;
	if (b->next!=NULL) (b->next)->prev = b;
	
	tmp = a->prev;
	a->prev = b->prev;
	if (a->prev!=NULL) (a->prev)->next = a;
	b->prev = tmp;
	if (b->prev!=NULL) (b->prev)->next = b;
};

//Co-link 2 list elements
void ListMakePair(struct LIST *lprev,struct LIST *lnext)
{
//    printf("Pairing %x & %x\n",(int)lprev, (int)lnext);
    if(lprev!=NULL) 
    {	
	if (lnext!=NULL) 
	    (lprev->next) = lnext;
	else 
	    lprev->next = NULL;
    };
    if(lnext!=NULL) 
    {
	if (lprev!=NULL) 
	    (lnext->prev) = lprev;
	else
	    lnext->prev = NULL;
    };
};

//Q-sorts arr[0,n), where arr is array of pointers to list
/*
int ListSortQsort(struct LIST **arr, int fr, int to)
{
	int i,j,k;
	if(to < (1 + fr)) return 1;
	
//	printf("DBG:STARTING RECURSIVE SORT (%x)[%d,%d)\nLIST={",(int)arr,fr,to);
//	for(i=fr;i!=to;i++) printf("%d ",arr[i]->Delay);
//	printf("}\n");
	

	struct LIST *t;
	k = arr[to-1]->Delay;
//	printf("DBG:  CHOSEN ELEMENT %d\n",k);
	j = fr;
	for(i=fr;i!=to;i++)
	{
//		printf("DBG:    %d- COMPARING %d WITH k=%d\n",i,arr[i]->Delay,k);
		if((arr[i]->Delay)<=k)
		{
//			printf("DBG:      SWAPPING\n");
			t = arr[i];
			arr[i] = arr[j];
			arr[j] = t;
			
			j++;
		}
	}
	ListSortQsort(arr,fr,j-1);
	ListSortQsort(arr,j,to);
	return 0;
}
*/

static int CompareDelays(const void *x, const void *y)
{
	struct LIST **a = (struct LIST **) x;
	struct LIST **b = (struct LIST **) y;
	return ((*a)->Delay - (*b)->Delay);
}

//Sort list, receives NON-ZERO pointer to first element
struct LIST *ListSort(struct LIST *list)
{
	struct LIST *tmp;
	int i,n;
//	printf("DBG:SORTING INITIALISING\n");
	//create an array of pointers to list elements and sort it
	n = 1;
	tmp = list;
	while(tmp->next!=NULL)	{n++; tmp=tmp->next;}
	
	tmp = list;
	struct LIST *arr[n];
	//printf("\nDBG:\nLIST={");
	for(i=0;i!=n;i++)
	{
		arr[i] = tmp;
		tmp = tmp->next;
		//printf("%d ",arr[i]->Delay);
	};
	//printf("}\n");
	
	//ListSortQsort(arr,0,n);
	qsort(arr,n,sizeof(struct LIST *),CompareDelays);
	
	//printf("LIST={");
	//for(i=0;i!=n;i++) printf("%d ",arr[i]->Delay);
	//printf("}\n");
	
	ListMakePair(NULL,arr[i]);
	for(i=0;i!=n-1;i++) ListMakePair(arr[i],arr[i+1]);
	ListMakePair(arr[n-1],NULL);
	
//	printf("SORTING FINISHED\n");
	
	return arr[0];
};
