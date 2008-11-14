#define EOC (-1)
#define bool int
#define true 1
#define false 0
struct connDesc
{
	const char *file;
	int type;
	int size;
	void *param;
};

//int *(protoInit)(struct connDesc *, int, bool, const char *);
//int *(protoClose)(struct connDesc *);
//int *(protoSend)(struct connDesc *, int);
//int *(protoRead)(struct connDesc *d);

