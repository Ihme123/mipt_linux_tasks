//Work with lists
//#include <stdio.h>

struct LIST
{
	int Delay;
	char **args;
	struct LIST *prev, *next;
};

//creating new element
struct LIST *ListCreate(int Delay, char **args, struct LIST *prev, struct LIST *next);

//Just remove element, hope it exists
void ListRemoveVoid(struct LIST *list);

//Remove element and return pointer to next one
struct LIST *ListRemove(struct LIST *list);

//Insert element 'list' after 'prev' and return pointer to it
struct LIST *ListInsert(struct LIST *list, struct LIST *prev);

//Swap 2 elements, return nothing; arguments must be non-zero pointers to elements to be swapped
void ListSwap(struct LIST *a, struct LIST *b);

struct LIST *ListSort(struct LIST *list);
