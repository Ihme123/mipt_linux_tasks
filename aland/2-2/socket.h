
//initialize connection, use before fork();
int SOCKInit(struct connDesc *d, int csize, bool isDryer,const char *file);

//close connection; also it frees all ocupoed memory
int SOCKClose(struct connDesc *d);

int SOCKSend(struct connDesc *d, int data);

int SOCKRead(struct connDesc *d);
