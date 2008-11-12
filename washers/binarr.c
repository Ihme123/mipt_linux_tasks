#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct pair
{
	int n;
	int x;
};
struct binarr
{
	int size, end;
	struct pair *p;
};

struct binarr *arrCreate()
{
    struct binarr *t;
    t = calloc(1, sizeof(struct binarr));
    if(t==NULL)
        return NULL;
    t->p = (struct pair *) calloc(1, sizeof(struct pair));
    t->size = 1;
    t->end = 0;
    return t;
};

int arrPush_back(struct binarr *arr, int n, int x)
{
    if(arr->end >= arr->size)
    {
        arr->p = (struct pair *) realloc(arr->p, 2*(arr->size)*sizeof(struct pair));
        if(arr->p == NULL) return(-1);
        (arr->size)*=2;
    };
    (arr->p[arr->end]).n = n;
    (arr->p[arr->end]).x = x;
    (arr->end)++;
    return (arr->end)-1;
};


void arrFree(struct binarr *arr)
{
    free(arr->p);
    free(arr);
};

int pairComp(const void *a, const void *b)
{
    struct pair *x = (struct pair *) a;
    struct pair *y = (struct pair *) b;
    return (x->n - y->n);
};

int arrFind(struct binarr *arr, int key)
{
    //printf("Looking for %d\n",key);
    struct pair *x, *y;
    x = calloc(1,sizeof(struct pair));
    x->n = key;
    x->x = 0;
    y = bsearch(x, arr->p, arr->end, sizeof(struct pair), pairComp);
    //printf("Search finished...%d\n",y->x);

    free(x);
    if(y==NULL)
        return 0;
    else
        return y->x;
};

void arrSort(struct binarr *arr)
{
    qsort(arr->p, arr->end, sizeof(struct pair), pairComp);
};

int arrRead(const char *filename, struct binarr *arr)
{
    if(freopen(filename, "r", stdin)==NULL)
    {return -1;}
    int x;
    int n;
    while(scanf("%d : %d",&n, &x)>0)
        arrPush_back(arr,n,x);
    fclose(stdin);
//    printf("succ, %d\n", arr->end);
    return (arr->end);
};
