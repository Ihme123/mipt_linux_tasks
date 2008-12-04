#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <stdlib.h>

#include "conn.h"

//signal that we donn't have any more disshes to wash...

#define OOM(c) if(c==NULL){printf("Not enough memory! Terminating\n");exit(-1);}

struct connSmem
{
    int sem;
    int shmid;
    void *smem;
};

int my_semop(int semid, int sem_num, int sem_op)
{
//	printf("%d\n",sem_op);
	struct sembuf t[1];
	t[0].sem_num = sem_num;
	t[0].sem_op = sem_op;
	t[0].sem_flg = 0;
	return semop(semid, t, 1);
};

void * get_shared_memory_block(const char *file, int *shmid_res, int size) {
	int shmid;
	if ( (shmid = shmget(ftok(file, 1), size, 0666|IPC_CREAT))<0 ) {
		printf("Can not get shmid for block of %d bytes\n", size);
		return NULL;
	}
	if ( shmid_res )
		*shmid_res = shmid;
	return shmat(shmid, NULL, 0);
};

int SMEMInit(struct connDesc *d, int csize, bool isDryer, const char *file)
{
//	printf("Initialising shared-memory connection for %s, keyfile is %s, size is %d\n",(isDryer)?"Dryer":"Washer",file,csize);
	d->file = file;
	d->param = calloc(1, sizeof(struct connSmem));
	OOM(d->param);
	((struct connSmem*)d->param)->sem = semget(ftok(file, 2), 1, ((isDryer)?0:IPC_CREAT)|0600);
	if(((struct connSmem*)d->param)->sem==0)
	{
		printf("Failed to get semaphores. Terminatiing\n");
		free(d->param);
		return -1;
	};

	d->size = csize;
//	d->type = 0;  my_semop	printf
	// my_semop(((struct connSmem*)d->param)->sem, 1, +1);
	struct connSmem *dd;
	dd = (struct connSmem *) d->param;
	semctl(dd->sem,0,SETVAL,0);
	if(isDryer)
	{
		dd->smem = get_shared_memory_block(d->file, NULL, (d->size + 3)*sizeof(int));
		if(dd->smem==NULL) {free(d->param); return 0;}
		return 1;
	}
	else
	{
		dd->smem = get_shared_memory_block(d->file, &(dd->shmid), (d->size + 3)*sizeof(int));
//		OOM(dd->smem)
		//SHARED MEMORY STRUCTURE: ARRAY OF INT
		//[0] - qty of dishes on the table
		//[1] - false if there might be more dishes, true otherwise ("have the washer finished his part of work?")
		//[2+]- type of dish

		if(dd->smem==NULL) {free(d->param); return 0;}
		my_semop(dd->sem,0,+1);
		memset(dd->smem, 0, ((d->size)+3)*sizeof(int));
		((int*)(dd->smem))[0] = 0;
		((int*)(dd->smem))[1] = false;
		my_semop(dd->sem,0,-1);
		//mark that shared memory is initialized
		//my_semop(dd->sem,1,-1);
		return (int) dd->smem;
	};
};

int SMEMClose(struct connDesc *d)
{
    free(d->param);
    //free(d);
    return 1;
};

int SMEMSend(struct connDesc *d, int data)
{
	//printf("--Sending...\n");
	//int f;
	struct connSmem *dd;

	dd = (struct connSmem *) d->param;
	my_semop(dd->sem,0,0);
	my_semop(dd->sem,0,+1);
	//printf("We've got acces to the structure!\n");
//	printf ("SIZE IS %d",d->size);

    //If we got the flag that washing is over,,,
	if (data == EOC)
		{
			((int*)dd->smem)[1] = true;
			my_semop(dd->sem,0,-1);
			return 0;
		};
	//wait until we have enough space on the table;
	//printf("--SEND:Now on the table %d\n",((int*)dd->smem)[0]);
	while(((int*) dd->smem)[0] >= d->size)
	{
		//printf("--SEND:Now on the table %d\n",((int*)dd->smem)[0]);
		my_semop(dd->sem,0,-1);
		//go for a walk for 1/2 sec.
		usleep(500000);
		my_semop(dd->sem,0,0);
		my_semop(dd->sem,0,+1);
	};
	((int*)dd->smem)[0]++;
	((int*)dd->smem) [((int*)dd->smem)[0]+1] = data;
//	printf("**Added dish %d to place %d\n",data,((int*)dd->smem)[0]+1);
	my_semop(dd->sem,0,-1);

	return 0;
};
int SMEMRead(struct connDesc *d)
{
  //  	printf("--Reading...\n");
	int Val;
	Val = 0;
	struct connSmem *dd;
	dd = (struct connSmem *) d->param;
	//Waiting initialisation...
	my_semop(dd->sem,1,0);
//	printf("SHMEM is intialised!\n");
	my_semop(dd->sem,0,0);
	my_semop(dd->sem,0,+1);
//	printf("And we've got acces to it!\n");


	if ((((int*)dd->smem)[0]==0)&&(((int*)dd->smem)[1]==true))
	{
		my_semop(dd->sem,0,-1);
		return -1;
	};

	//wait until we have smth on the table
	while((((int*)dd->smem)[0])<=0)
	{
		my_semop(dd->sem,0,-1);
		//go for a walk for 1/2 sec.
		usleep(500000);
		my_semop(dd->sem,0,0);
		my_semop(dd->sem,0,+1);
	};

//	printf("--READ!!\n");
	Val = ((int*)dd->smem)[(((int*)dd->smem)[0])+1];
//	printf("--READ!!!!! %d\n", Val);
	(((int*)dd->smem)[0])--;
	my_semop(dd->sem,0,-1);

	return Val;
};
