
//initialize connection, use before fork();
int SMEMInit(struct connDesc *d, int csize, bool isDryer,const char *file);

//close connection; also it frees all ocupoed memory
int SMEMClose(struct connDesc *d);

int SMEMSend(struct connDesc *d, int data);

int SMEMRead(struct connDesc *d);
