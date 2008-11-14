//Task #1-1 (sem.3) by Alekseenko Andrey, 771

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include "list.h"

#define OOM(c,str) if(c==NULL) {printf("Not enough memory (at %s). Terminating\n",str); return 0;}

//default maximal argument count and max. length of one argument
const int DefArgCount=4, DefArgStrLen = 128;

const char *DefConfFileName = "useless.conf\0";


int main(int argc, char **argv, char **env)
{
    char c;
    int IsPause = 1;
    char **ArrTmp;
    int iArrTmp;
    
    time_t TimeStart = time(NULL);
    
    char *ConfFileName;
    ConfFileName = calloc(strlen(DefConfFileName)+1, sizeof(char));
    OOM(ConfFileName,"ConfFileName");
    strcpy(ConfFileName,DefConfFileName);
    
    int i, j, iDelay, iTime, iCurArg, iCurString, iRetVal;
    int ArgStrLen = DefArgStrLen;

    
    //Parsing command line...
    i=1;
    while(argv[i]!=0)
    {
	switch(argv[i][0])
	{
	    case '-':
		switch(argv[i][1])
		{
		    case 'p':
			IsPause = 0;
			break;
		    case '?':	
		    case 'h':
			printf("USELESS v0.1prebeta\nI won't describe here what this application does 'coz everyone who'll try to use it alredy know it. If you don't, I really sorry :(\n");
			printf("This application takes commandline parameters listed below:\n	-h  this message\n	-?  the same\n	-p  disable pause while waiting for next second, increase CPU usage\n	filename  (without '-') path to configuration file, default is '%s'\nAuthor: Alekseenko Andrey, MIPT771\n", DefConfFileName);
			return 0;
			break;
		    default:
			printf("Unrecognized commandline argument: %s\n",argv[i]);
		};
		break;
	    default:

		free(ConfFileName);
		ConfFileName = calloc(strlen(argv[i])+1,sizeof(char));
		OOM(ConfFileName,"New ConfFileName");
		strcpy(ConfFileName,argv[i]);
		break;
	};
	i++;
    };
    
    
    char *StrTmp, *StrTmp2;
    int iStrTmp2 = ArgStrLen;
    StrTmp2 = calloc(ArgStrLen, sizeof(char));
    OOM(StrTmp2, "StrTmp2");
    
    struct LIST *List;
    List  = ListCreate(-1,NULL,NULL,NULL);  //empty 1st element, just as pointer

    struct LIST *CurList; 
    CurList = List; 

    iCurString = 0; //just current string number of configuration file...

    //We won't read smth from keyboard, so we redirect input stream to file
    iRetVal = (int)freopen(ConfFileName,"r",stdin);
    
    if (iRetVal == 0)
    {
	printf("ERROR: Can't open configuration file %s. Terminating\n",ConfFileName);
	return 0;
    };

    while(scanf("%d ",&iDelay) == 1)    //we read incoming file
    {    
	iCurString++;

	CurList = ListInsert( ListCreate(iDelay,NULL,CurList,NULL) , CurList );
	
        if (iDelay <=0)
        {
    	    printf("ERROR: On line %d delay value should be positive (is %d). Terminating\n", iCurString, iDelay);
	    fclose(stdin);
    	    return 0;
        };	
	
        CurList->args = calloc(DefArgCount, sizeof(char*));
	OOM(CurList->args,"Argument array");

	iArrTmp = DefArgCount;
	iCurArg = 0;
	j=0;

	do
	{
	    i = getchar();
	    if (i==-1) break;
	    c = (char) i;

	    if(((c==' ')||(c=='\n'))&&(j > 0))
	    {
		StrTmp2[j]='\0';
//		CurList->args[iCurArg] = strcat(CurList->args[iCurArg], StrTmp2);
		CurList->args[iCurArg] = calloc( strlen(StrTmp2)+1, sizeof(char) );
		OOM(CurList->args[iCurArg],"New argument");
		strcpy(CurList->args[iCurArg],StrTmp2);
		j=0;
		iCurArg++;

		if(iCurArg == iArrTmp)
		{
		    ArrTmp = CurList->args;
		    CurList->args = calloc(2*iArrTmp, sizeof(char*));
		    OOM(CurList->args, "Extending **Args array");
		    memcpy(CurList->args, ArrTmp, iArrTmp*sizeof(char*));
		    free(ArrTmp);
		    iArrTmp*=2;
		};
		CurList->args[iCurArg] = NULL;
	    }
	    else if ((c!=' ')&&(c!='\n'))
	    {
		StrTmp2[j]=c;
		j++;
		if (j == iStrTmp2)   //we exceeded length of StrTmp2
		{
		    StrTmp = StrTmp2;
		    StrTmp2 = calloc( 2*iStrTmp2 , sizeof(char) );
		    OOM(StrTmp2,"Extending StrTmp2");
		    iStrTmp2*=2;
		    strcpy(StrTmp2,StrTmp);
		    free(StrTmp); 
//		    CurList->args[iCurArg] = strcat(CurList->args[iCurArg],StrTmp);
		    j = 0;
		};
	    };
	}while((c != '\n')&&(c != '\0'));
	CurList->next = NULL;
    };
    free(StrTmp2);
    fclose(stdin);
        
    //printf("DBG:File read!\n");
    //Now we have List of content of conf. file...
    
    List = ListSort(List);

    //printf("DBG:List sorted!\n ");
        
    while((List->next)!=NULL)
    {
//        while (iTime == (time() - TimeStart))
//	    if 	(IsPause) usleep(200);//wait until new second begin...
//	
//	iTime = (time(NULL) - TimeStart);
//

        CurList = List->next;
	
	iTime = time(0) - TimeStart;
	//printf("Sleeping %d (now %d)\n",List->next->Delay,iTime);
	sleep((List->next)->Delay - iTime);
	iTime = time(0) - TimeStart;
	
        while(CurList!=NULL)
        {
	    if(CurList->Delay <= iTime)   //the time has come...
	    {
	        //start necessery application
		if(fork()==0)
		{
		    if(execvp(CurList->args[0], CurList->args)==-1)
		    {
			printf("ERROR: Can't execute %s (Delay = %d)\n", CurList->args[0], CurList->Delay);
			return 0;
		    }
		};
		
		CurList = ListRemove(CurList);
	    }
	    else break;
	    CurList = CurList->next;
	};
    };
    
    //Well, we've started all what we had to. Now, let's wait 'till death of 'em all
    while(wait(NULL)!=-1);
    
    return 0;
}
