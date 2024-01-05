#include "graphint.h"
#include "gmextern.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

LONG Sequence=1;
static	LONG Actual[3];
static  PVOID h1,h2,h3;
static	char	msg[4096];
static	char	AbendFile[MAX_PATH];
static	BOOL	ignoreError=FALSE;

void SetIgnoreError (BOOL setting)
{
	ignoreError = setting;
	return;
}

void AbendWriter (LPSTR Message,LPSTR Title,long at,int type)
{
	char	LastF[MAX_PATH*2];
	LPSTR	LastAF = &LastF[MAX_PATH];
	BOOL	SaveSE=ShareEnabled;
	long	seconds; 
	time_t	systime; 
	RECT	WindRect,ClientRect; 
	char	DateTime[64];
	char	Winver[64];
	char	spaces[2]="";
	
	//MessageBox(0, "In Abend Writer", 0, MB_OK);
	if (!LogUsage || InPrintDriver || inOpenFileDialog || ignoreError)
		return;
	if (type == 0x6ba)
		return;
	ignoreError = TRUE; //limits to one time
	if (!Message)
		Message = spaces;
	if (!Title)
		Title = spaces;
	sprintf (LastF,"screendump.exe %s",AbendFile); 
//	ExpandText (LastF);
	WinExec (LastF,SW_HIDE);
	//Wait (2500);
	ShareEnabled = TRUE;
	strcpy (LastF,GetLastPathname ());
	strcpy (LastAF,GetLastAccessedFile ());

    Actual[0] = Sequence++;

		
	systime=time(&systime);
	
	strcpy (DateTime,ctime(&systime));
	*strchr(DateTime,'\n') = 0;   
	seconds = (GetTickCount64()-SysStartTime)/1000;
	GetWindowRect(hWndMain,&WindRect);
	GetClientRect(hWndMain,&ClientRect);
	ShareEnabled = TRUE;
	sprintf (msg,"\r\n\r\n%s\t%s\t%s\t%ld\t%ld\t%s\t%ld\t%lli\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i",
        DateTime,UserName,NodeName,seconds,NumScreensDisplayed,GMVersion,nTempFilesCleared,totAllocatedMem,
																			  		 WindRect.left,WindRect.top,WindRect.right,WindRect.bottom,ClientRect.left,ClientRect.top,ClientRect.right,ClientRect.bottom);
	AppendFile2 (AbendFile,msg);
	GetWindowsVersion (Winver);
	AppendFile2 (AbendFile,Winver);

	sprintf (msg,"%s %s",Message,Title);
	AppendFile2 (AbendFile,msg);
	sprintf (msg,"GeoMaster crash at %lx : type %lx",at,type);
	AppendFile2 (AbendFile,msg);
	sprintf (msg,"MemInUse:%ld MaxMem: %ld",TotMemAlloc,MaxMemAlloc);
	AppendFile2 (AbendFile,msg);

	char DriveID[6] = "C:\\";
	double FreeSpace = GetDriveFreeSpace(DriveID), mb = FreeSpace / ((double)1024 * (double)1024);
	sprintf(msg, "C Drive free space (MB):%.0f",mb);
	AppendFile2(AbendFile, msg);

	sprintf (msg,"Last opened file:%s\r\nLast accessed file:%s",LastF,LastAF);
	AppendFile2 (AbendFile,msg);
	sprintf (msg,"PickMFile:%s",PickMFile);
	AppendFile2 (AbendFile,msg);
	sprintf (msg,"LastFGSLoc = %i, LastFGSlRec=%i",LastFGSLoc,LastFGSlRec);   
	AppendFile2 (AbendFile,msg);
	sprintf (msg,"Last PMFid:%i",(int)FidPM);
	AppendFile2 (AbendFile,msg);
	sprintf (msg,"Last PMstr:%s",PMstr);
	AppendFile2 (AbendFile,msg);

	sprintf (msg,"CurrentConfig: %i",CurrentConfig);
	AppendFile2 (AbendFile,msg);
	if (CurView)
	{
		if (CurView->CurFile > 0)
			sprintf (msg,"Config:%s\r\nViewport:%s\r\nLayer:%i(%s)",CfgName,CurView->Name,CurView->CurFile,CurView->FileID[CurView->CurFile-1]);
		else
			sprintf (msg,"Config:%s\r\nViewport:%s\r\nLayer:%i",CfgName,CurView->Name,CurView->CurFile);
		AppendFile2 (AbendFile,msg);
	}
	if (CurTheme)
	{
		sprintf (msg,"Theme:%i %i %i",CurTheme->ID,CurTheme->DisplayViewport,CurTheme->TargetViewport);
		AppendFile2 (AbendFile,msg);
	}
	DumpTraceback (AbendFile);
	DumpOpenFiles(AbendFile);
    RemoveVectoredExceptionHandler(h1);
	ShareEnabled = SaveSE;
	return;
}

//LONG WINAPI
LONG WINAPI
VectoredHandler1(struct _EXCEPTION_POINTERS *pExceptionInfo)
{
	if (pExceptionInfo->ExceptionRecord->ExceptionCode == 1073807366)
		return EXCEPTION_CONTINUE_SEARCH;
	if (pExceptionInfo->ExceptionRecord->ExceptionCode == 3765269347)
		return EXCEPTION_CONTINUE_SEARCH;
	if (pExceptionInfo->ExceptionRecord->ExceptionCode == -536870911)
		return EXCEPTION_CONTINUE_SEARCH;
	AbendWriter ("From vectored handler",0,(long)pExceptionInfo->ExceptionRecord->ExceptionAddress,pExceptionInfo->ExceptionRecord->ExceptionCode);
   return EXCEPTION_CONTINUE_SEARCH;
}

LONG WINAPI
VectoredHandler2(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
    Actual[1] = Sequence++;
	AppendFile ("c:\\temp\\testexc.txt","GeoMaster crash");
	_exit (1);
    return EXCEPTION_CONTINUE_SEARCH;
}
LONG WINAPI
VectoredHandler3(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
    Actual[2] = Sequence++;
    return EXCEPTION_CONTINUE_SEARCH;
}

LONG WINAPI
VectoredHandlerSkip1(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
    PCONTEXT Context;
    
    Sequence++;
    Context = ExceptionInfo->ContextRecord;
    Actual[0] = 0xcc;
    Context->Eip++;
    return EXCEPTION_CONTINUE_EXECUTION;
}

LONG WINAPI
VectoredHandlerSkip2(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
    PCONTEXT Context;
    
    Sequence++;
    Context = ExceptionInfo->ContextRecord;
    Actual[1] = 0xcc;
    Context->Eip++;
    return EXCEPTION_CONTINUE_EXECUTION;
}

LONG WINAPI
VectoredHandlerSkip3(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
    PCONTEXT Context;
    
    Sequence++;
    Context = ExceptionInfo->ContextRecord;
    Actual[2] = 0xcc;
    Context->Eip++;
    return EXCEPTION_CONTINUE_EXECUTION;
}

BOOL
CheckTest(
    char *Variation,
    PLONG e,
    PLONG a
    )
{
    int i;
    BOOL Pass = TRUE;

    for(i=0;i<3;i++) 
    {
        if (e[i] != a[i]) 
        {
            if (Variation) 
            {
                printf("%s Failed at %d Expected %d vs Actual %d\n", 
                        Variation, i, e[i], a[i]);
            }
            Pass = FALSE;
        }

        // Clear actual for next pass.
        a[i] = 0;
    }

    // Get ready for next pass.
    Sequence = 1;

    if (Variation) 
    {
        printf("Variation %s %s\n", Variation, 
                Pass ? "Passed" : "Failed");
    }
    return Pass;
}

void
CheckAllClear()
{
    LONG e[3];
    BOOL b;

    e[0]=0;e[1]=0;e[2]=0;
    __try 
    {
        RaiseException(1,0,0,NULL);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        b = CheckTest(NULL,e,Actual);
    }
    if (!b) 
    {
        printf("Fatal error, handlers still registered.\n");
    }
}

void
IllegalInst()
{
    _asm {cli};
}
int AddVectoredHandlers (LPSTR UName,LPSTR Dir)
{
	time_t	starttime;
	char	noDir=0;

	if (!Dir)
		Dir = &noDir;

	time (&starttime);  
	
	if (UName)
		sprintf (AbendFile,"%sabends\\%s_%I64i.txt",Dir,UName,starttime);
	else
	{
     
//    h2 = AddVectoredExceptionHandler(1,VectoredHandler2);
//    h3 = AddVectoredExceptionHandler(0,VectoredHandler3);
		sprintf (AbendFile,"%sabends\\beforenodeparms_%I64i.txt",Dir,starttime);
//	strcpy (AbendFile,"[%%DL]abends.txt");
//	ExpandText (AbendFile);
		h1 = AddVectoredExceptionHandler(0,VectoredHandler1);
	}
	_fullpath (AbendFile,AbendFile,MAX_PATH);
	return 1;
}
Test1()
{
    
    PVOID h1,h2,h3;
    LONG e[3];
     
    e[0]=1;e[1]=2;e[2]=3;
    h2 = AddVectoredExceptionHandler(1,VectoredHandler2);
    h3 = AddVectoredExceptionHandler(0,VectoredHandler3);
    h1 = AddVectoredExceptionHandler(1,VectoredHandler1);

    __try 
    {
        RaiseException(1,0,0,NULL);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        CheckTest("Test1a",e,Actual);
    }

    RemoveVectoredExceptionHandler(h1);
    RemoveVectoredExceptionHandler(h3);
    RemoveVectoredExceptionHandler(h2);
    CheckAllClear();
}

void
Test2()
{
    
    PVOID h1,h2,h3;
    LONG e[3];
     
    e[0]=0xcc;e[1]=0;e[2]=0;
    h1 = AddVectoredExceptionHandler(1,VectoredHandlerSkip1);
    IllegalInst();
    CheckTest("Test2a",e,Actual);
    RemoveVectoredExceptionHandler(h1);
    CheckAllClear();

    e[0]=1;e[1]=2;e[2]=0xcc;
    h2 = AddVectoredExceptionHandler(1,VectoredHandler2);
    h3 = AddVectoredExceptionHandler(0,VectoredHandlerSkip3);
    h1 = AddVectoredExceptionHandler(1,VectoredHandler1);
    IllegalInst();
    CheckTest("Test2b",e,Actual);
    RemoveVectoredExceptionHandler(h1);
    RemoveVectoredExceptionHandler(h2);
    RemoveVectoredExceptionHandler(h3);
    CheckAllClear();

    e[0]=1;e[1]=0xcc;e[2]=0;
    h1 = AddVectoredExceptionHandler(0,VectoredHandler1);
    h2 = AddVectoredExceptionHandler(0,VectoredHandlerSkip2);
    h3 = AddVectoredExceptionHandler(0,VectoredHandler3);
    IllegalInst();
    CheckTest("Test2c",e,Actual);
    RemoveVectoredExceptionHandler(h1);
    RemoveVectoredExceptionHandler(h2);
    RemoveVectoredExceptionHandler(h3);
    CheckAllClear();

    e[0]=2;e[1]=0xcc;e[2]=1;
    h1 = AddVectoredExceptionHandler(0,VectoredHandler1);
    h3 = AddVectoredExceptionHandler(1,VectoredHandler3);
    h2 = AddVectoredExceptionHandler(0,VectoredHandlerSkip2);
    __try 
    {
        IllegalInst();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        // Should not make it to here.
        e[0]=0;e[1]=0;e[2]=0;
        CheckTest("Test2d-1",e,Actual);
    }
    CheckTest("Test2d-2",e,Actual);
    if (h1) RemoveVectoredExceptionHandler(h1);
	if (h2) RemoveVectoredExceptionHandler(h2);
	if (h3) RemoveVectoredExceptionHandler(h3);
    CheckAllClear();
}

int testexception(int i )
{
    Test1();
    Test2();
	return 1;
}