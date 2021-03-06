#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>
#define true 1
#define false 0

#include "binarr.h"
#define OOM(c) if(c==NULL){printf("Not enough memory! Terminating\n");exit(-1);}
#define BUFSIZE 512

#include "conn.h"
#include "socket.h"
#include "shmem.h"

#define DBG 1
#define CTYPE_SMEM 1
#define CTYPE_SOCK 2
#define CTYPE_UNDEF 0
//int main_parent(int argc, char **argv, char **env);
//int main_child(int argc, char **argv, char **env);

int main(int argc, char **argv, char **env)
{
	int i,j,l,n = 0;
	int ConnType = CTYPE_UNDEF;

	// Target environment variable name
	const char *strEnvPattern = "TABLE_LIMIT";

	char *strTmp;
	i=0;

	// Searching for strEnvPattern in environment variables
	l = strlen(strEnvPattern);
	while(env[i]!=NULL)
	{
		j = 0;
		while((strEnvPattern[j]==env[i][j])&&(j < l))
			j++;
		if (j==l) //We found it!
		{
			strTmp = env[i]+j+1;
			n = atoi(strTmp);
			break;
		};
		i++;
	};
	if(n<=0)
	{
		printf("ERROR: Wrong or missing environment variable %s\nUse ""export TABLE_LIMIT=x"" to initialize it properly\n", strEnvPattern);
		return -1;
	};



	// Read conf.files and sort resulting arrays;

	if (argc!=2) {printf("This application takes exactly 1 argument. Terminating\n"); return -1;}
	char c;
	char *StrDryConf, *StrWashConf, *StrWorkConf, *StrTmp;
	int StrTmpSize = BUFSIZE;
	StrDryConf = NULL;
	StrWashConf = NULL;
	StrTmp = calloc(BUFSIZE, sizeof(char));
	OOM(StrTmp);

	if(DBG>0) printf("Parsing conf file %s\n",argv[1]);
	i = 0;
	if(freopen(argv[1],"r",stdin)==NULL)
	{
		printf("Can\'t open file \'%s\'. Terminating\n",argv[1]);
		return -1;
	};
	i = j = 0; //i is reading stage, j is position in the line;
	while((c=getchar())!=EOF)
	{
		if((c=='#')&&(j==0))	//commentary
		{
			while((c!='\n')&&(c!=EOF))	c=getchar();
			//i++;
			if(DBG>1) printf("Commentary line passed\n");
			continue;
		};
		if(c!='\n')
		{
			StrTmp[j] = c;
			if(DBG>1) printf("Symbol : %c\n",c);
			j++;
			if(j==StrTmpSize)
			{
				StrTmpSize *= 2;
				StrTmp = realloc(StrTmp, StrTmpSize);
				OOM(StrTmp);
			};
			StrTmp[j] = '\0';
		}
		else	//we've ended the line, now we'll watch what that should be...
			switch(i)
			{
				case 0:		//first line, type of connection
					j = 0;
					if(!strcasecmp(StrTmp,"SHMEM"))
					{
						ConnType = CTYPE_SMEM;
						i++;
						continue;
					};
					if(!strcasecmp(StrTmp,"SOCKET"))
					{
						ConnType = CTYPE_SOCK;
						i++;
						continue;
					};
					i++;
					break;
				case 1:		//file with total work
					StrWashConf = calloc(strlen(StrTmp)+1,sizeof(char));
					OOM(StrWashConf);
					strcpy(StrWashConf, StrTmp);
					i++;
					j = 0;
					break;
				case 2:		//file with washer's timings
					StrDryConf = calloc(strlen(StrTmp)+1,sizeof(char));
					OOM(StrDryConf);
					strcpy(StrDryConf, StrTmp);
					i++;
					j = 0;
					break;
				case 3:		//file with dryer's timings
					StrWorkConf = calloc(strlen(StrTmp)+1,sizeof(char));
					OOM(StrWorkConf);
					strcpy(StrWorkConf, StrTmp);
					i++;
					j = 0;
					break;
			};
		if(i==4) break;
	};
	fclose(stdin);
	free(StrTmp);
	
	if(i<4)
	{
		free(StrWorkConf);
		free(StrWashConf);
		free(StrDryConf);
		printf("Wrong configuration file format. Terminating\n");
		return -1;
	};

	//Initialisung function pointers

	int (*cInit)(struct connDesc *, int, bool, const char *);
	int (*cClose)(struct connDesc *);
	int (*cSend)(struct connDesc *, int);
	int (*cRead)(struct connDesc *);

	switch(ConnType)
	{
		case CTYPE_UNDEF:
			printf("Unknown connection type. Terminating\n");
			free(StrWorkConf);
			free(StrWashConf);
			free(StrDryConf);
			return -1;
			break;
		case CTYPE_SMEM:
			cInit = SMEMInit;
			cClose = SMEMClose;
			cSend = SMEMSend;
			cRead = SMEMRead;
			break;
		case CTYPE_SOCK:
			cInit = SOCKInit;
			cClose = SOCKClose;
			cSend = SOCKSend;
			cRead = SOCKRead;
			break;
	};

	//Reading timings
	struct binarr *TDry;
	TDry = arrCreate();
	OOM(TDry);
	if(arrRead(StrDryConf, TDry)==-1)
	{
		printf("ERROR: Can't read file '%s'\n", StrDryConf);
        	return -1;
	};
	arrSort(TDry);

	//We've read all we need;

	if(DBG>0) printf("Reading succesful. Main conf file is '%s', washing timings in '%s', drying timings in '%s', and what we should wash is listed in '%s'\n",argv[1],StrWashConf, StrDryConf, StrWorkConf);


	struct connDesc *d;
	d = calloc(1, sizeof(struct connDesc));
	if (cInit(d, n, true, argv[1])==0)
	{
		printf("Error initialising connection. Terminating...\n");
		arrFree(TDry);
		free(d);
		free(StrWorkConf);
		free(StrWashConf);
		free(StrDryConf);
		return -1;
	}
	int a,p;
	while((a = cRead(d))!=-1)
	{
		p = arrFind(TDry, a);
		printf("+!Drying : %d (%d sec)\n",a,p);
		if(p>0) sleep(p);
		printf("+~Dryed  : %d\n",a);

	};


	cClose(d);

	printf("++We've dryed everything\n");

	//Freeing
	arrFree(TDry);
	free(d);
	free(StrWashConf);
	free(StrDryConf);
	free(StrWorkConf);
	return 0;
};

