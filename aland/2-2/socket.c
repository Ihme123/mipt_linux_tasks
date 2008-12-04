#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

#define PORT 51000
#include "conn.h"

#define OOM(c) if(c==NULL){printf("Not enough memory! Terminating\n");exit(-1);}



struct stack
{
    int n;
    int p;
    int *s;
};

void stackinit(struct stack *s, int size)
{
    s->s = (int *)calloc(size, sizeof(int));
    s->n = size;
    s->p = 0;
    OOM(s->s);
};
void stackput(struct stack *s, int a)
{
    while(s->p>=s->n) usleep(50000);
    s->s[s->p] = a;
    s->p++;
}
int stackget(struct stack *s)
{
    while(s->p == 0) usleep(50000);
    s->p--;
    return s->s[s->p];
};

struct stack ss; //Sending stack, because i dunno want to write queue w/out STL


struct connSock
{
	int sockfd;
	int actsockfd;
	pthread_t waitthread;
	bool isInit, isFailed;
	int Cur;
};
struct sockSend
{
	int data;
	struct connSock *c;
};


void *WaitConnection(void *ptr)
{
	struct connSock *c;
	c = (struct connSock *) ptr;
	if((c->actsockfd = accept(c->sockfd, NULL, NULL))<0)
	{
		c->isFailed = true;
		close(c->sockfd);
		c->waitthread=0;
		return NULL;
	};
	c->isInit = true;
	//now we'll check stack and send all waiting data
//	c->waitthread = 0;

//	struct sockSend *s = (struct sockSend *) ptr;

	int data;
	while(true)
	{
		data = stackget(&ss);
		while((!(c->isInit))&&(!(c->isFailed)))
			usleep(500000);
		if(c->isFailed)
		{
	//		free(c);
			c->waitthread=0;
			return NULL;
		};
		if(write(c->actsockfd,&(data),sizeof(data))<0)
			c->isFailed = true;
		if(data==EOC)
		{
			c->waitthread=0;
			return NULL;
		};
	//	free(s);

	};
};


int SOCKInitServer(struct connSock *c)
{
	struct sockaddr_in servaddr;
	if ((c->sockfd = socket(PF_INET, SOCK_STREAM, 0))<0)
		return 0;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(c->sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr))<0)
	{
		close(c->sockfd);
		return 0;
	};
	if(listen(c->sockfd,1)<0)
	{
		close(c->sockfd);
		return 0;
	};
		
	if(pthread_create(&(c->waitthread),(pthread_attr_t *)NULL,WaitConnection,(void*)c)!=0)
	{
		close(c->sockfd);
		return 0;
	};

	return 1;
};
int SOCKInitClient(struct connSock *c)
{
	struct sockaddr_in servaddr;
	if ((c->sockfd = socket(PF_INET, SOCK_STREAM, 0))<0)
		return 0;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	inet_aton("127.0.0.1", &servaddr.sin_addr);
	c->actsockfd = c->sockfd;
	if(connect(c->sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr))<0)
	{
		c->isFailed = true;
		close(c->sockfd);
		return 0;
	};
	c->isInit = true;
	return 1;
};

int SOCKInit(struct connDesc *d, int csize, bool isDryer, const char *file)
{
	struct connSock *c;
	c = calloc(1, sizeof(struct connSock));
	stackinit(&ss, csize);
	d->param = (void*) c;
	c->isInit = false;
	c->isFailed = false;
	c->waitthread = 0;
	c->Cur = 0;
	d->file = file;
	d->size = csize;
	if(isDryer)
		return SOCKInitClient(c);
	else
		return SOCKInitServer(c);
};

int SOCKClose(struct connDesc *d)
{
	struct connSock *c;
	c = (struct connSock *) d->param;
	while(c->waitthread!=0) 
	{
		usleep(500000);
//		pthread_kill(c->waitthread,9);
	};
//	close(c->sockfd);
//	close(c->actsockfd);
	free(d->param);
	free((void *)ss.s);
	return 1;
};

int SOCKSend(struct connDesc *d, int data)
{
	struct connSock *c;
//	struct sockSend *sd;
//	pthread_t tt;
//	sd = calloc(1, sizeof(struct sockSend));
	int i;
	c = (struct connSock *) d->param;
//	sd->data = data;
//	sd->c = c;
	
	if(c->isFailed)
		return -1;

	//wait while we don't have enough space on table
	while(c->Cur >= d->size)
	{
		while((!(c->isInit))&&(!(c->isFailed)))
		usleep(500000);

		read(c->actsockfd, &i, sizeof(int));
		c->Cur--;
	};
	c->Cur++;
//	pthread_create(&tt,NULL,SendData,sd);
	
	stackput(&ss, data);
	return 1;

};
int SOCKRead(struct connDesc *d)
{
	struct connSock *c;
	int Val,i;
	i = 1;
	c = (struct connSock *) d->param;
	while((!(c->isInit))&&(!(c->isFailed)))
		usleep(500000);
	if(c->isFailed)
	{
		return -1;
	};
	if(read(c->actsockfd,&Val,sizeof(int))<=0)
	{
		return -1;
	}
	if(Val!=-1)
	{
		//Informing that we've taken one dish
		write(c->actsockfd,&i, sizeof(i));
	};
	return Val;
};
