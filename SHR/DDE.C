#include "shr.h"  
#include "ddemlsv.h"
#include "client.h"
#include "huge.h"

#define DEFAULT_ACK_TIME_OUT_MILLISEC 30000

#include "gmextern.h"

// GLOBALS 


static PXFERINFO pldaXferInfo;
static DWORD idInst;
static CONVCONTEXT CCFilter;
static  HWND hwndServer;

static  BOOL fAllBlocked;
static  BOOL fAllEnabled;
static  BOOL fEnableOneCB;
static  BOOL fBlockNextCB;
static  BOOL fTermNextCB;
static  BOOL fAppowned;
static  WORD cRunaway;
static  WORD RenderDelay;
static  DWORD count;
static  HSZ hszAppName;
//static  char szClass[];
static  char szTopic[MAX_TOPIC];
static  char szServer[MAX_TOPIC];
static  char szComment[MAX_COMMENT];
static  char szPoke[MAX_COMMENT];
static  char szExec[MAX_EXEC];
static  char *pszComment;
static  char *pszPoke;
static  WORD seed;
static  WORD cyText;
static  WORD cServers=0;
static  HDDEDATA hDataHelp[CFORMATS];
static  HDDEDATA hDataCount[CFORMATS];
 //HDDEDATA hDataRand[CFORMATS];
 //HDDEDATA hDataHuge[CFORMATS];
 //DWORD cbHuge;
static  char szDdeHelp[]= "GeoMaster DDE help:\r\n\n"\
    "Items supported under the 'Test' topic are:\r\n"\
    "\tAddress Location:\tThis Converts an address to coordinates.\r\n"\
    "\tLocation to Address:\tThis converts a coordinate location to an address.\r\n"\
    "\tVideo View:\tThis displays a video of the location\r\n"\
    "\r\n"\
    "The above items change after any request if in Runaway mode and \r\n"\
    "can bo POKEed in order to change their values.  POKEed Huge data \r\n"\
    "must be in a special format to verify the correctness of the data \r\n"\
    "or it will not be accepted.\r\n"\
    "If the server is set to use app owned data handles, all data sent \r\n"\
    "uses HDATA_APPOWNED data handles."\
    ;
static  FORMATINFO aFormats[CFORMATS];

static   ITEMLIST TestTopicItemList[CTESTITEMS] = 
  {
    { 0,GWIZXfer,      "Command" },
    { 0, ItemListXfer, "Items" },
    { 0, VideoXfer,    "Video" },
    { 0, HelpXfer,     "Help"}
  };

static   ITEMLIST TestTopicItemListHWY[CTESTITEMS] = 
  {
    { 0,GWIZXfer,      "Locate" },
    { 0, ItemListXfer, "Items" },
    { 0, VideoXfer,    "Video" },
    { 0, HelpXfer,     "Help"}
  };

static   ITEMLIST SystemTopicItemList[CSYSTEMITEMS] = 
  {
    { 0, TopicListXfer,  SZDDESYS_ITEM_TOPICS   },
    { 0, ItemListXfer,   SZDDESYS_ITEM_SYSITEMS },
    { 0, sysFormatsXfer, SZDDESYS_ITEM_FORMATS  },
    { 0, HelpXfer,       SZDDESYS_ITEM_HELP}
  };  
/* The system topic is always assumed to be first. */   
/*   HSZ   PROCEDURE            #ofITEMS        PSZ     */ 

static   TOPICLIST topicList[CTOPICS] = 
  {
    { 0, SystemTopicItemList,   CSYSTEMITEMS,   SZDDESYS_TOPIC},    // 0 index
    { 0, TestTopicItemList,     CTESTITEMS,     "Command"}           // 1 index
  };
static   TOPICLIST topicListHWY[CTOPICS] = 
  {
    { 0, SystemTopicItemList,   CSYSTEMITEMS,   SZDDESYS_TOPIC},    // 0 index
    { 0, TestTopicItemListHWY,     CTESTITEMS,     "Loc&Video"}           // 1 index
  };





static int nAckTimeOut,ii;
static BOOL   bInInitiate = FALSE, IsInRequest=TRUE;

BOOL ValidateContext(PCONVCONTEXT pCC);
BOOL AwaitingAck(HWND hwndClientDDE);
void WaitForAck (HWND hwndClientDDE);


/****************************************************************************

    FUNCTION: AwaitingAck

    PURPOSE:  Inform user if acknowledgement is required before further
              action.

****************************************************************************/
BOOL AwaitingAck(HWND hwndClientDDE)
{
    if (GetConvPendingAck(hwndClientDDE) == NONE)
    {
        return (FALSE);
    }
    MessageBox(hWndMain,
        "Previous DDE operation must be acknowledged first",
        "Client",
        MB_ICONEXCLAMATION | MB_OK);
    return (TRUE);
}

/****************************************************************************

    FUNCTION: ClientAcknowledge

    PURPOSE:  Called when client application receives WM_DDE_ACK message
	      or WM_TIMER message (time out on wait for ACK).

****************************************************************************/
void ClientAcknowledge(hwndClientDDE, hwndServerDDE, lParam, bTimeOut)
    HWND hwndClientDDE;
    HWND hwndServerDDE;
    LONG lParam;    /* lParam of WM_DDE_ACK message */
    BOOL bTimeOut;  /* TRUE if NACK is due to time-out */
{
    enum PENDINGACK ePendingAck;
    char szApplication[APP_MAX_SIZE+1];
    char szTopic[TOPIC_MAX_SIZE+1];
    char szItem[ITEM_MAX_SIZE+1];
    char message[80];
    short	ii;

    ePendingAck = GetConvPendingAck(hwndClientDDE);
    SetConvPendingAck(hwndClientDDE, NONE);
    ii=1;
    if (hwndClientDDE && hwndServerDDE)
    	KillTimer(hwndClientDDE, (UINT)hwndServerDDE);
    if (bInInitiate)
    {
        GlobalGetAtomName(LOWORD(lParam),
            szApplication,
            APP_MAX_SIZE);
        GlobalGetAtomName(HIWORD(lParam),
            szTopic,
            TOPIC_MAX_SIZE);
		if (!AddConv(hwndClientDDE, hwndServerDDE, szApplication, szTopic))
	        {
		    MessageBox(hWndMain,
			"Maximum conversation count exceeded",
	                "Client",
	                MB_ICONEXCLAMATION | MB_OK);
        }
	/*
	GlobalDeleteAtom(LOWORD(lParam));
        GlobalDeleteAtom(HIWORD(lParam));
	*/
        return;
    }
    if ((ePendingAck == ADVISE) && (LOWORD(lParam) & 0x8000))
    {	/* received positive ACK in response to ADVISE */
        GlobalGetAtomName(HIWORD(lParam), szItem, ITEM_MAX_SIZE);
		AddItemToConv(hwndClientDDE, szItem);

	/* Conversation item is established: now get current value */
	/* and update screen					  */
		SendRequest(hwndClientDDE, hwndServerDDE, szItem);
    }
    if ((ePendingAck == UNADVISE) && (LOWORD(lParam) & 0x8000))
    {	/* received positive ACK in response to UNADVISE */
        GlobalGetAtomName(HIWORD(lParam), szItem, ITEM_MAX_SIZE);
		RemoveItemFromConv(hwndClientDDE, szItem);
    }
    if (!(LOWORD(lParam) & 0x8000))    /* NACK */
    {
        _fstrcpy(message, "DDE ");
        _fstrcat(message, ePendingAck == ADVISE?    "ADVISE "
                      : ePendingAck == UNADVISE?  "UNADVISE "
                      : ePendingAck == POKE?      "POKE "
                      : ePendingAck == REQUEST?   "REQUEST "
		      : ePendingAck == EXECUTE?   "EXECUTE "
                      : " ");
        _fstrcat(message, bTimeOut? "acknowledge time out"
                                : "operation failed");
		MessageBox(hWndMain,
	            message,
	            "Client",
	            MB_ICONEXCLAMATION | MB_OK);
    }
    switch (ePendingAck)
    { 
	case POKE:    
//		if (hwndPokeDlg)
//			PostMessage(hwndPokeDlg,WM_COMMAND,IDC_ACK,MAKELPARAM(hwndClientDDE,hwndServerDDE));
	
		GlobalDeleteAtom(HIWORD(lParam));  
		
		break;    
	case ADVISE:
	case UNADVISE:
	case REQUEST:
	    if (HIWORD(lParam))  /* will not be available for time-out */
		GlobalDeleteAtom(HIWORD(lParam));
	    break;
	case EXECUTE: 
	{
		HANDLE	hMess;     
		LPSTR	pMess;
		DWORD	ii;
		char	mess[512];
		
		if (lParam)
		{
			hMess = (HANDLE)HIWORD(lParam);  
			pMess = GlobalLock (hMess);   
		    SetGlobalValue ("%DDE_RESPONSE",pMess);
		    sprintf (mess,"DDE Response %ld %ld: %s",(long)hwndClientDDE,(long)hwndServerDDE,pMess);
		    GSSiTrace (mess,0);
		    GSSiGlobUlFree (&hMess);
		}
	}
	    break;
    }
    return;
}



/****************************************************************************

    FUNCTION: ClientReceiveData

    PURPOSE:  Called when client application receives WM_DDE_DATA message.

****************************************************************************/
void ClientReceiveData(hwndClientDDE, hwndServerDDE, lParam)
    HWND  hwndClientDDE;
    HWND  hwndServerDDE;
    LONG  lParam;
{
    DDEDATA FAR * lpDDEData;
    char          szItem[ITEM_MAX_SIZE+1];
    BOOL          bRelease;
    BOOL          bAck;

    if (IsConvInTerminateState(hwndClientDDE, hwndServerDDE))
    { /* Terminate in progress: do not receive data */
        GlobalFree ((HANDLE)LOWORD(lParam));
        GlobalDeleteAtom(HIWORD(lParam));
        return;
    }

    if (GetConvPendingAck(hwndClientDDE) == REQUEST)
    {
		SetConvPendingAck(hwndClientDDE, NONE);
		KillTimer(hwndClientDDE, (int)hwndServerDDE);
    }

    if (!(lpDDEData = (DDEDATA FAR *)GlobalLock((HANDLE)LOWORD(lParam)))
        || (lpDDEData->cfFormat != CF_TEXT))
    {
		PostMessage(hwndServerDDE,
	            WM_DDE_ACK,
		   (WPARAM) hwndClientDDE,
	            MAKELONG(0, HIWORD(lParam)));  /* Negative ACK */
    }
    bAck = FALSE;
    if (IsInRequest)
    {
	/* Update REQUEST dialog box value */
//		RequestSatisfied(lpDDEData->Value);
        bAck = TRUE;
    }
    else
    {
        GlobalGetAtomName(HIWORD(lParam), szItem, ITEM_MAX_SIZE);
		bAck = SetConvItemValue(hwndClientDDE, szItem, lpDDEData->Value);
    }
    if (lpDDEData->fAckReq)
    {
	/* return ACK or NACK */
		PostMessage(hwndServerDDE,
	            WM_DDE_ACK,
		   (WPARAM) hwndClientDDE,
		    MAKELONG( (bAck? 0x8000:0), HIWORD(lParam)));
    }
    bRelease = lpDDEData->fRelease;
    GlobalUnlock((HANDLE)LOWORD(lParam));
    if (bRelease)
        GlobalFree ((HANDLE)LOWORD(lParam));
    return;
}


/****************************************************************************

    FUNCTION: ClientTerminate

    PURPOSE:  Called when client application receives WM_DDE_TERMINATE
	      message.

****************************************************************************/
void ClientTerminate(HWND hwndClientDDE,HWND hwndServerDDE)
{
    if (!IsConvInTerminateState(hwndClientDDE, hwndServerDDE))
    { /* Server has requested terminate: respond with terminate */
		PostMessage(hwndServerDDE, WM_DDE_TERMINATE,(WPARAM) hwndClientDDE, 0L);
    }
    RemoveConv(hwndClientDDE, hwndServerDDE);
    if (!IsHwndClientDDEUsed(hwndClientDDE))
		DestroyWindow(hwndClientDDE);
    return;
}

BOOL RegisterDDEClient(void)
{
    WNDCLASS  wc; 
    static	Called=FALSE;

    if (Called) return TRUE;
    Called = TRUE;
    wc.style = NULL;
    wc.lpfnWndProc = DDEWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName =  NULL;
    wc.lpszClassName = "ClientDDEWndClass";

    return (RegisterClass(&wc));
}

long FAR PASCAL  DDEWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_DDE_ACK:
	    ClientAcknowledge(hwnd,(HWND)wParam, lParam, FALSE);
	    return (0L);

	case WM_TIMER:
	    /* Negative ACK because of time out */
	    ClientAcknowledge(hwnd,(HWND)wParam, 0L, TRUE);
	    return (0L);

	 case WM_DDE_DATA:
	     ClientReceiveData(hwnd,(HWND)wParam, lParam);
	     return (0L);

	 case WM_DDE_TERMINATE:
	      ClientTerminate(hwnd,(HWND)wParam);
	      return (0L);

	 default:
	      return (DefWindowProc(hwnd, message, wParam, lParam));
    }
}

/****************************************************************************

    FUNCTION: InitAckTimeOut

    PURPOSE:  Get DDE timeout value from win.ini.  Value is in milliseconds.

****************************************************************************/
void InitAckTimeOut(void)
{


   if (!(nAckTimeOut = GetGlobalLVal("[%DDE_ACK_TIMEOUT]")))
		nAckTimeOut = DEFAULT_ACK_TIME_OUT_MILLISEC;
   return;
}

/****************************************************************************

    FUNCTION: SendAdvise

    PURPOSE:  Send advise message to server.


****************************************************************************/
void SendAdvise(hwndClientDDE, hwndServerDDE, szItem)
    HWND  hwndClientDDE;
    HWND  hwndServerDDE;
    char * szItem;
{
    ATOM            atomItem;
    HANDLE          hOptions;
    DDEADVISE FAR * lpOptions;

    /* don't send another message requiring an ACK until first message */
    /* is acknowledged						       */
    if (AwaitingAck(hwndClientDDE))
        return;

    if (!(hOptions
          = GSSiGlobAlloc ( 402,GMEM_MOVEABLE | GMEM_DDESHARE, (LONG)sizeof(DDEADVISE))))
        return;
    if (!(lpOptions
          = (DDEADVISE FAR *)GlobalLock(hOptions)))
    {
	GSSiGlobFree (&hOptions);
        return;
    }
    lpOptions->cfFormat = CF_TEXT;
    lpOptions->fAckReq = TRUE;
    lpOptions->fDeferUpd = FALSE;
    GlobalUnlock(hOptions);
    atomItem = GlobalAddAtom((LPSTR)szItem);
    SetConvPendingAck(hwndClientDDE, ADVISE); 
    InitAckTimeOut();
    if (!SetTimer(hwndClientDDE, hwndServerDDE, nAckTimeOut, 0))
    	ii=1;
    if (!PostMessage(hwndServerDDE,
            WM_DDE_ADVISE,
	    hwndClientDDE,
            MAKELONG(hOptions, atomItem)))
    {
        GlobalDeleteAtom(atomItem);
        GSSiGlobFree (&hOptions);
    }
    return;
}



/****************************************************************************

    FUNCTION: SendExecute

    PURPOSE:  Send execute string to server.


****************************************************************************/
void SendExecute(HWND hwndClientDDE, HWND hwndServerDDE, LPSTR szExecuteString)
{
    HANDLE	hExecuteString;
    LPSTR	lpExecuteString;  
    LPSTR	mess;
    HANDLE	hMess;

    /* don't send another message requiring an ACK until first message */
    /* is acknowledged						       */
    if (AwaitingAck(hwndClientDDE))
        return;

    if (!(hExecuteString
          = GSSiGlobAlloc ( 403,GMEM_MOVEABLE | GMEM_DDESHARE,
			(DWORD)lstrlen(szExecuteString) + 1)))
        return;
    if (!(lpExecuteString
	  = GlobalLock(hExecuteString)))
    {
	GSSiGlobFree (&hExecuteString);
        return;
    }
    hMess = GSSiGlobAlloc ( 404,GMEM_MOVEABLE,1024);
    mess = GlobalLock (hMess);  
    sprintf (mess,"DDE Execute %ld %ld: %s",(long)hwndClientDDE,(long)hwndServerDDE,szExecuteString);
    GSSiTrace (mess,0);
    GSSiGlobUlFree (&hMess);
    _fstrcpy(lpExecuteString, szExecuteString);
    GlobalUnlock(hExecuteString);
    SetConvPendingAck(hwndClientDDE, EXECUTE);
    InitAckTimeOut();
    if (!SetTimer(hwndClientDDE, hwndServerDDE, nAckTimeOut, 0))
    	ii=0;
    if (!PostMessage(hwndServerDDE,
	    WM_DDE_EXECUTE,
	    hwndClientDDE,
	    MAKELONG(0, hExecuteString)))
    {
	GSSiGlobFree (&hExecuteString);
    }
	WaitForAck (hwndClientDDE);
    return;
}

/****************************************************************************

    FUNCTION: SendInitiate

    PURPOSE:  Sends initiate message to all windows.  By the time this
	      function returns, all servers matching the app/topic will
	      have acknowledged, and this client applicaiton will have
	      temporarily registered the new conversations.   If more
	      than one server responded, then this client application
	      asks the user which conversation to keep; all other
	      conversations will then be terminated.   This function
	      returns the handle of the hidden DDE window used to
	      initiate the conversation with server(s).

****************************************************************************/
HWND SendDDEInitiate(LPSTR szApplication, LPSTR szTopic)
{
    HWND  hwndClientDDE;
    ATOM  atomApplication;
    ATOM  atomTopic;  
    short	nConvBeg;

	if (!RegisterDDEClient()) return (0);
    if (!(hwndClientDDE = CreateWindow(
	    "ClientDDEWndClass",
	    "ClientDDE",
	    WS_CHILD,	/* not visible */
	    0, 0, 0, 0, /* no position or dimensions */
	    hWndMain,	/* parent */
	    0,	/* no menu */
	    hInst,
	    0)))
    {
		return (0);
    }

    atomApplication
        = *szApplication == 0 ? NULL : GlobalAddAtom((LPSTR)szApplication);
    atomTopic
        = *szTopic == 0 ? NULL : GlobalAddAtom((LPSTR)szTopic);

    /* flag bIniInitiate is queried when client processes the server's ACK */
    bInInitiate = TRUE;
    nConvBeg = nConvCount;
    SendMessage(HWND_BROADCAST,WM_DDE_INITIATE,hwndClientDDE,MAKELONG(atomApplication, atomTopic));
    bInInitiate = FALSE;
    if (atomApplication != NULL)
        GlobalDeleteAtom(atomApplication);
    if (atomTopic != NULL)
        GlobalDeleteAtom(atomTopic);
    if (nConvBeg == nConvCount)
    {
		DestroyWindow(hwndClientDDE); 
		hwndClientDDE = NULL;
	}
	else if (nConvCount - nConvBeg > 1)  
	{
		TerminateConversations(hwndClientDDE); 
		MessageBox (GetFocus(),"Multiple responses to DDE intitiate",0,MB_ICONEXCLAMATION);
		hwndClientDDE = NULL;
	}
		
    return (hwndClientDDE);
}
/****************************************************************************

    FUNCTION: SendPoke

    PURPOSE:  Send poke message to server.


****************************************************************************/
void SendPoke(hwndClientDDE, hwndServerDDE, szItem, szValue)
    HWND  hwndClientDDE;
    HWND  hwndServerDDE;
    char * szItem;
    char * szValue;
{
    ATOM        atomItem;
    HANDLE      hPokeData;
    DDEPOKE FAR * lpPokeData;

    /* don't send another message requiring an ACK until first message */
    /* is acknowledged						       */
    if (AwaitingAck(hwndClientDDE))
        return;

    /* Allocate size of DDE data header, plus the data:  a string   */
    /* terminated by <CR> <LF> <NULL>.  The <NULL> is counted by    */
    /* by DDEPOKE.Value[1].                                         */

    if (!(hPokeData
          = GSSiGlobAlloc ( 405,GMEM_MOVEABLE | GMEM_DDESHARE,
                        (LONG)sizeof(DDEPOKE) + lstrlen(szValue) + 2)))
        return;
    if (!(lpPokeData
          = (DDEPOKE FAR*)GlobalLock(hPokeData)))
    {
	GSSiGlobFree (&hPokeData);
        return;
    }
    lpPokeData->fRelease = TRUE;
    lpPokeData->cfFormat = CF_TEXT;
    _fstrcpy((LPSTR)lpPokeData->Value, (LPSTR)szValue);
    /* each line of CF_TEXT data is terminated by CR/LF */
    lstrcat((LPSTR)lpPokeData->Value, (LPSTR)"\r\n");
    GlobalUnlock(hPokeData);
    atomItem = GlobalAddAtom((LPSTR)szItem);
    SetConvPendingAck(hwndClientDDE, POKE);
    InitAckTimeOut();
    if (!SetTimer(hwndClientDDE, hwndServerDDE, nAckTimeOut, 0))
    	ii=1;
    if (!PostMessage(hwndServerDDE,
            WM_DDE_POKE,
	    hwndClientDDE,
            MAKELONG(hPokeData, atomItem)))
    {
        GlobalDeleteAtom(atomItem);
        GSSiGlobFree (&hPokeData);
    }
    return;
}



/****************************************************************************

    FUNCTION: SendRequest

    PURPOSE:  Send request message to server.


****************************************************************************/
void SendRequest(hwndClientDDE, hwndServerDDE, szItem)
    HWND  hwndClientDDE;
    HWND  hwndServerDDE;
    char * szItem;
{
    ATOM  atomItem;

    /* don't send another message requiring an ACK until first message */
    /* is acknowledged						       */
    if (AwaitingAck(hwndClientDDE))
        return;

    atomItem = GlobalAddAtom((LPSTR)szItem);
    SetConvPendingAck(hwndClientDDE, REQUEST);
    InitAckTimeOut();
    if (!SetTimer(hwndClientDDE, hwndServerDDE, nAckTimeOut, 0))	
    	ii=1;
    if (!PostMessage(hwndServerDDE,
            WM_DDE_REQUEST,
	    hwndClientDDE,
            MAKELONG(CF_TEXT,atomItem)))
    {
        GlobalDeleteAtom(atomItem);
    }
    return;
}


/****************************************************************************

    FUNCTION: SendTerminate

    PURPOSE:  Send terminate message to server.

****************************************************************************/
void SendTerminate(hwndClientDDE, hwndServerDDE)
    HWND  hwndClientDDE;
    HWND  hwndServerDDE;
{
    SetConvInTerminateState(hwndClientDDE, hwndServerDDE);
    PostMessage(hwndServerDDE, WM_DDE_TERMINATE, hwndClientDDE, 0L);
    return;
}




/****************************************************************************

    FUNCTION: SendUnadvise

    PURPOSE:  Send unadvise message to server.


****************************************************************************/
void SendUnadvise(hwndClientDDE, hwndServerDDE, szItem)
    HWND  hwndClientDDE;
    HWND  hwndServerDDE;
    char * szItem;
{
    ATOM  atomItem;

    /* don't send another message requiring an ACK until first message */
    /* is acknowledged						       */
    if (AwaitingAck(hwndClientDDE))
        return;

    atomItem = GlobalAddAtom((LPSTR)szItem);
    SetConvPendingAck(hwndClientDDE, UNADVISE);
    InitAckTimeOut();
    if (!SetTimer(hwndClientDDE, hwndServerDDE, nAckTimeOut, 0))
    	ii=1;
    if (!PostMessage(hwndServerDDE,
            WM_DDE_UNADVISE,
	    hwndClientDDE,
            MAKELONG(0,atomItem)))
    {
        GlobalDeleteAtom(atomItem);
    }
    return;
}


/****************************************************************************

    FUNCTION: TerminateConversations

    PURPOSE:  Processes WM_DESTROY message, terminates all conversations.

****************************************************************************/
void TerminateConversations(HWND hWndClient)
{
   HWND  hwndClientDDE;
   HWND  hwndServerDDE;
   LONG  lTimeOut;
   MSG   msg;


   /* Terminate each active conversation */
   hwndClientDDE = hWndClient;
   while (hwndClientDDE = GetNextConv(hwndClientDDE))
   {
		hwndServerDDE = GetHwndServerDDE(hwndClientDDE);
		if (IsWindow(hwndServerDDE)) /* if server window still alive */
		    SendTerminate(hwndClientDDE, hwndServerDDE);
		else
		{
		    SetConvInTerminateState(hwndClientDDE, hwndServerDDE);
			ClientTerminate(hwndClientDDE,hwndServerDDE);
		}
   }

   /* Wait for all conversations to terminate or for time out */
   InitAckTimeOut();
   lTimeOut = GetTickCount() + (LONG)nAckTimeOut;
   while (GSSiPeekMessage(&msg, 0, WM_DDE_FIRST, WM_DDE_LAST, PM_REMOVE))
   {
#if ENABLETRACE
SetLastMessage(-1*(long)msg.message);
#endif
         DispatchMessage (&msg);
		 if (msg.message == WM_DDE_TERMINATE)
		 {
		     if (!AtLeastOneConvActive())
			 break;
		 }
         if (GetTickCount() > (DWORD)lTimeOut)
             break;
   }

   return;
}

    //following added by lda
void DdeBye(void)
{  
	UnHszize();
   DdeNameService(idInst, 0, 0, DNS_UNREGISTER);
   DdeUninitialize(idInst) ;
} 

BOOL InitDdeStuff(HWND  In_handle, BOOL Highways)
{
  FARPROC my_handle;
  UINT K, i;
 HGLOBAL LDA;     
 
 if (Highways)
 	topicList[1] = topicListHWY[1];
 // added by LDA 
    aFormats[0].atom = CF_TEXT; // exception - predefined.
    for (i = 1; i < CFORMATS; i++) 
    {
        aFormats[i].atom = RegisterClipboardFormat(aFormats[i].sz);
    } // end of lda addition
    // LDA = GlobalAlloc(GMEM_DDESHARE ,sizeof(XFERINFO));            
    // pldaXferInfo   =   (PXFERINFO) GlobalLock(LDA);
     CCFilter.iCodePage = CP_WINANSI;   // initial default codepage
     my_handle = MakeProcInstance((FARPROC)DdeCallback, In_handle);        
    if (!DdeInitialize(&idInst,(PFNCALLBACK) my_handle, APPCMD_FILTERINITS, 0)) 
    {
        Hszize();
        DdeNameService(idInst, hszAppName, 0, DNS_REGISTER|DNS_FILTEROFF	);
       return (TRUE);
    }  // end of lda addition
 return (FALSE);
}

/*
 * This function verifies that the incomming conversation context fits the
 * server's context filter's requirements.
 */
BOOL ValidateContext(PCONVCONTEXT pCC)
{
    // make sure our CCFilter allows it...mock security, language support
    // old DDE app client case...pCC == NULL
    if (pCC == (PCONVCONTEXT) NULL &&
            CCFilter.dwSecurity == 0 &&      // were nonsecure
            CCFilter.iCodePage == CP_WINANSI) // were normal cp
                  return(TRUE);
    if (pCC &&
            pCC->wFlags == CCFilter.wFlags && // no special flags needed
            pCC->iCodePage == CCFilter.iCodePage && // codepages match
            pCC->dwSecurity == CCFilter.dwSecurity) // security passes
        // dont care about language and country.
                   return(TRUE);
    
    return(FALSE);  // disallow no match
}


/***************************** Public  Function ****************************\
*
* This function is called by the DDE manager DLL and passes control onto
* the apropriate function pointed to by the global topic and item arrays.
* It handles all DDE interaction generated by external events.   segment
*                        
\***************************************************************************/

HDDEDATA EXPENTRY DdeCallback(WORD wType, WORD wFmt, HCONV hConv,HSZ hszTopic,
         HSZ hszItem,HDDEDATA hData,DWORD lData1,DWORD lData2)
{  
    WORD i, j ,ii;
    register ITEMLIST *pItemList;
    WORD cItems, iFmt;
    HDDEDATA hDataRet, mydde;  
   /*
     * Block this callback if its blockable and we are supposed to.
     */          // CS == DX before I start moving things around
 /*    _asm { _asm MOV CX, DS
            _asm MOV DX, SS
            _asm MOV DS, DX
            _asm MOV DX, CS  }   */
   if (fBlockNextCB && !(wType & XTYPF_NOBLOCK)) 
    {
        fBlockNextCB = FALSE;
        fAllEnabled = FALSE;
        return(CBR_BLOCK);                      
    }

    /*
     * Block this callback if its associated with a conversation and we
     * are supposed to.
     */
    if (fTermNextCB && hConv) 
    {
        fTermNextCB = FALSE;
        DdeDisconnect(hConv);
        wType = XTYP_DISCONNECT;
    }

    /*
     * Keep a count of connections
     */ 
    if (wType == WM_DDE_INITIATE)
    	ii=1;
    if (wType == XTYP_CONNECT_CONFIRM) 
    {
        cServers++;
       // InvalidateRect(hwndServer, &rcConnCount, TRUE);
        return(0);
    }
    if (wType == XTYP_DISCONNECT) 
    {
        cServers--;
      //  InvalidateRect(hwndServer, &rcConnCount, TRUE);
        return(0);
    }
    
    
    /*
     * only allow transactions on the formats we support if they have a format.
     */
    if (wFmt) 
    {
        for (iFmt = 0; iFmt < CFORMATS; iFmt++) 
        {
            if (wFmt == aFormats[iFmt].atom)break;
        }
        if (iFmt == CFORMATS) return(0); // illegal format - ignore now.
    }

    /*
     * Executes are allowed only on the system topic.  This is a general
     * convention, not a requirement.
     *
     * Any executes received result in the execute text being shown in
     * the server client area.  No real action is taken.
     */                                                   
    if (wType == XTYP_EXECUTE) 
    {
        if (hszTopic == topicList[0].hszTopic) 
        {   // must be on system topic
            // Format is assumed to be CF_TEXT.
            HDDEDATA hNew;  
            
            DdeGetData(hData, (LPBYTE)szExec, MAX_EXEC, 0);
            szExec[MAX_EXEC - 1] = '\0';     
	        ExecuteCommandString (szExec); 
	          
	        if (LogOn)
	        {
	        	HFILE	Fid;
	        	char	str[256];
	        	OFSTRUCT	OFStruct;
	        	
	        	Fid = GSSiOpenFile ("ddelog.txt",&OFStruct,OF_READWRITE);
	        	if (Fid == HFILE_ERROR)
		        	Fid = GSSiOpenFile ("ddelog.txt",&OFStruct,OF_CREATE);  
	        	if (Fid != HFILE_ERROR)
	        	{   
	        		LPSTR CmdMess = GlobalLock (hCmdMess);
	        		BigWrite (Fid,szExec,_fstrlen(szExec),-1);
                    BigWrite (Fid,CmdMess,_fstrlen(CmdMess),-1);
	        		GSSiClose (Fid);  
	        		GlobalUnlock (hCmdMess);
	        	}
		        
	        }
            
            // InvalidateRect(hwndServer, &rcExec, TRUE); 
//		       hDataRet = DdeCreateDataHandle(idInst, Mess, sizeof(Mess), 0L, 0, wFmt, 0);
//			   return(hDataRet);            
//            hDataRet = TRUE;
            goto ReturnSpot;
        }
        pszComment = "Execute received on non-system topic - ignored";
        //  InvalidateRect(hwndServer, &rcComment, TRUE);
        return(0);
    }

    /*
     * Process wild initiates here
     */
    if (wType == XTYP_WILDCONNECT) 
    {
        HSZ ahsz[(CTOPICS + 1) * 2];
        /*
         * He wants a hsz list of all our available app/topic pairs
         * that conform to hszTopic and hszItem(App).
         */

        if (!ValidateContext((PCONVCONTEXT)lData1)) return(FALSE);
        
        if (hszItem != hszAppName && hszItem != 0) return(0); 
        // scan the topic table and create hsz pairs 
        j = 0;
        for (i = 0; i < CTOPICS; i++) 
        {
            if (hszTopic == NULL || hszTopic == topicList[i].hszTopic) 
            {
                ahsz[j++] = hszAppName;
                ahsz[j++] = topicList[i].hszTopic; 
            }
        }

        // cap off the list with 0s
        ahsz[j++] = ahsz[j++] = 0L;

        // send it back
       mydde = DdeCreateDataHandle(idInst, (LPBYTE)&ahsz[0], sizeof(HSZ) * j, 0L, 0, wFmt, 0);
       return mydde;
    }

    /*
     * Check our hsz tables and send to the apropriate proc. to process.
     * We use DdeCmpStringHandles() which is the portable case-insensitive
     * method of comparing string handles.  (this is a macro on windows so
     * there is no real speed hit.)  On WINDOWS, HSZs are case-insensitive
     * anyway, but this may not be the case on other platforms.
     */
    for (i = 0; i < CTOPICS; i++) 
    {
        if (DdeCmpStringHandles(topicList[i].hszTopic, hszTopic) == 0) 
        {

            /*
             * connections must be on a topic we support.
             */
            if (wType == XTYP_CONNECT)
            {   
            	char pTopic[64], pService[64];
            	DWORD	len; 
            	BOOL	rtn=TRUE;
            	if (lData1)
            		return(ValidateContext((PCONVCONTEXT)lData1));   
            	
            	len = DdeQueryString(idInst, hszTopic, pTopic, 64, CP_WINANSI);	 
            	len = DdeQueryString(idInst, hszItem, pService, 64, CP_WINANSI);
            	if (_fstricmp (pService,"GeoMaster"))
            		rtn = FALSE;	 
//            	DdeFreeStringHandle (idInst,hszTopic);
//            	DdeFreeStringHandle (idInst,hszItem);  
            	return rtn;
            }
                      
            pItemList = topicList[i].pItemList;
            cItems = topicList[i].cItems;
            for (j = 0; j < cItems; j++) 
            {
                if (DdeCmpStringHandles(pItemList[j].hszItem, hszItem) == 0) 
                {
                     //* Make call to worker function here...
                    hDataRet = (*pItemList[j].npfnCallback) ((PXFERINFO)&lData2, iFmt);
ReturnSpot:                    
                    /*
                     * The table functions return a boolean or data.
                     * It gets translated here.
                     */
                    switch (wType & XCLASS_MASK) 
                    {
                    case XCLASS_DATA:
                        return(hDataRet);
                        break;
                    case XCLASS_FLAGS:
                        return(hDataRet ? DDE_FACK : DDE_FNOTPROCESSED);
                        break;
                    case XCLASS_BOOL:
                        return(TRUE);
                    default: // XCLASS_NOTIFICATION
                        return(0);
                        break;
                    }
                    break;
                }
            }
            break;
        }
    }

    /*
     * anything else fails - DDEML is designed so that a 0 return is ALWAYS ok.
     */ 

    return(0);
}





/***************************** Private Function ****************************\
* This passes out a standard tab-delimited list of topic names for this
* application.
*
* This support is required for other apps to be able to
* find out about us.  This kind of support should be in every DDE application.
*
\***************************************************************************/
HDDEDATA TopicListXfer(PXFERINFO pXferInfo, WORD iFmt)
{
    WORD cbAlloc, i;
    LPSTR pszTopicList;
    HDDEDATA hData;

    if (pXferInfo->wType == XTYP_ADVSTART)  
        return(TRUE);
    
    if (pXferInfo->wType != XTYP_REQUEST &&
            pXferInfo->wType != XTYP_ADVREQ)
        return(0);
    /*
     * construct the list of topics we have
     */
    cbAlloc = 0;
    for (i = 0; i < CTOPICS; i++)
        cbAlloc += lstrlen(topicList[i].pszTopic) + 1;  // 1 for tab

    // allocate a data handle big enough for the list.
    hData = DdeCreateDataHandle(idInst, 0, 0, cbAlloc, pXferInfo->hszItem,
            pXferInfo->wFmt, 0);
    pszTopicList = (LPSTR)DdeAccessData(hData, 0);
    if (pszTopicList) {
        for (i = 0; i < CTOPICS; i++) {
            _fstrcpy(pszTopicList, topicList[i].pszTopic);
            pszTopicList += _fstrlen(topicList[i].pszTopic);
            *pszTopicList++ = '\t';
        }
        *--pszTopicList = '\0';
        DdeUnaccessData(hData);
        return(hData);
    }
    return(0);
}




/***************************** Private Function ****************************\
* This passes out a standard tab-delimited list of item names for the
* specified topic.
*
* This support is required for other apps to be able to
* find out about us.  This kind of support should be in every DDE
* application.
*
\***************************************************************************/
HDDEDATA ItemListXfer(
PXFERINFO pXferInfo,
WORD iFmt)
{
    WORD cbAlloc, i, iItem, cItems;
    ITEMLIST *pItemList = 0;
    LPSTR pszItemList;
    HDDEDATA hData;

    if (pXferInfo->wType == XTYP_ADVSTART) return(TRUE);
    
    if (pXferInfo->wType != XTYP_REQUEST && pXferInfo->wType != XTYP_ADVREQ) return(0);
    /*
     * construct the list of items we support for this topic - this supports
     * more than the minimum standard which would support SysItems only on
     * the system topic.
     */

    // locate the requested topic item table
    for (i = 0; i < CTOPICS; i++) {
        if (pXferInfo->hszTopic == topicList[i].hszTopic) {
            pItemList = topicList[i].pItemList;
            cItems = topicList[i].cItems;
            break;
        }
    }
    
    if (!pItemList)
        return(0);  // item not found
        
    cbAlloc = 0;
    for (iItem = 0; iItem < cItems; iItem++)
        cbAlloc += lstrlen(pItemList[iItem].pszItem) + 1; // 1 for tab
        
    // allocate a data handle big enough for the list.
    hData = DdeCreateDataHandle(idInst, 0, 0, cbAlloc, pXferInfo->hszItem,
            pXferInfo->wFmt, 0);
    pszItemList = (LPSTR)DdeAccessData(hData, 0);
    if (pszItemList) {
        for (i = 0; i < cItems; i++) {
            _fstrcpy(pszItemList, pItemList[i].pszItem);
            pszItemList += _fstrlen(pItemList[i].pszItem);
            *pszItemList++ = '\t';
        }
        *--pszItemList = '\0';
        DdeUnaccessData(hData);
        return(hData);
    }
    return(0);
}





/***************************** Private Function ****************************\
* Gives out a 0 terminated array of dde format numbers supported by this app.
*
* This support is required for other apps to be able to
* find out about us.  This kind of support should be in every DDE application.
*
\***************************************************************************/
HDDEDATA sysFormatsXfer(PXFERINFO pXferInfo,WORD iFmt)
{
    int i, cb;
    LPSTR psz, pszT;
    HDDEDATA hData;

    if (pXferInfo->wType == XTYP_ADVSTART)
        return(TRUE);
    
    if (pXferInfo->wType != XTYP_REQUEST &&
            pXferInfo->wType != XTYP_ADVREQ) 
        return(0);

    for (i = 0, cb = 0; i < CFORMATS; i++) 
        cb += _fstrlen(aFormats[i].sz) + 1;
        
    hData = DdeCreateDataHandle(idInst, 0, (DWORD)cb,
            0L, pXferInfo->hszItem, pXferInfo->wFmt, 0);
    psz = pszT = DdeAccessData(hData, 0);
    for (i = 0; i < CFORMATS; i++) {
        _fstrcpy(pszT, aFormats[i].sz);
        pszT += _fstrlen(pszT);
        *pszT++ = '\t';
    }
    *(--pszT) = '\0';
    DdeUnaccessData(hData);
    return(hData);
}




/***************************** Private Function ****************************\
*  This creates often used global hszs from standard global strings.
*  It also fills the hsz fields of the topic and item tables.
*
\***************************************************************************/
void Hszize()
{    

    register ITEMLIST *pItemList;
    WORD iTopic, iItem;
    _fstrcpy(szServer,"GeoMaster");
    hszAppName = DdeCreateStringHandle(idInst, szServer, CP_WINANSI);

    for (iTopic = 0; iTopic < CTOPICS; iTopic++) 
    {
        topicList[iTopic].hszTopic =
                DdeCreateStringHandle(idInst, topicList[iTopic].pszTopic, 0);
        pItemList = topicList[iTopic].pItemList;
        for (iItem = 0; iItem < topicList[iTopic].cItems; iItem++) 
        {
            pItemList[iItem].hszItem =
                    DdeCreateStringHandle(idInst, pItemList[iItem].pszItem, 0);
        }
    }
}





/***************************** Private Function ****************************\
*  This destroys often used global hszs from standard global strings.
*
\***************************************************************************/
void UnHszize()
{
    register ITEMLIST *pItemList;
    WORD iTopic, iItem;

    DdeFreeStringHandle(idInst, hszAppName);

    for (iTopic = 0; iTopic < CTOPICS; iTopic++) {
        DdeFreeStringHandle(idInst, topicList[iTopic].hszTopic);
        pItemList = topicList[iTopic].pItemList;
        for (iItem = 0; iItem < topicList[iTopic].cItems; iItem++) {
            DdeFreeStringHandle(idInst, pItemList[iItem].hszItem);
        }
    }
}
/*******************************************************************\
* 
* Here's where we place the code for the service requests supported
* by our GeoMaster Address applications. lda
*
\*******************************************************************/

HDDEDATA GWIZXfer(PXFERINFO pXferInfo, WORD iFmt)
{
  //  char szT[128];   // SS==DS!
    LPSTR pszData;
    HDDEDATA hData;
    WORD i;
    int	ii;
    char *ptr;
    char *presponse, response[] = "OK from GeoMaster Address";
    char szX[24], szY[24], savepoke[256];
    double X,Y;
   	clock_t	starttime, endtime, TotTime;
   	
    presponse = response;
    ptr = szPoke;
    switch (pXferInfo->wType) 
    {
    case XTYP_POKE:
        {// we expect an ascii number to replace the current count.
       // pszComment = "GWIZ poke received";
       // InvalidateRect(hwndServer, &rcComment, TRUE);
       // InvalidateRect(hwndServer, &rcCount, TRUE);
        if (DdeGetData(pXferInfo->hData, ptr, 127, 0))
        ptr = _fstrchr(szPoke,'\r');
        if(ptr) *ptr = '\0';    
        _fstrcpy (savepoke,szPoke);  
		OpenFileTime=NumFilesOpened=lBlocksRead=nBlocksRead=nBlocksIn=nBlocksOut=TotDisplayTime=MapIOTime= OpenMapTime= TextTime=TotTextDisplayTime=PickTime=CloseTime=IntersectTime=DistTime=0;
        
	    if (DoTime)
	        starttime=GetTickCount();
        ExecuteCommandString (szPoke);
        if (DoTime)
	        endtime=GetTickCount();   
        TotTime = endtime-starttime;
	        if (LogOn)
	        {
	        	HFILE	Fid;
	        	char	str[512];
	        	OFSTRUCT	OFStruct; 
	        	long	TotBlocks;
	        	
	        	Fid = GSSiOpenFile ("ddelog.txt",&OFStruct,OF_READWRITE);
	        	if (Fid == HFILE_ERROR)
		        	Fid = GSSiOpenFile ("ddelog.txt",&OFStruct,OF_CREATE);  
	        	if (Fid != HFILE_ERROR)
	        	{   
	        		LPSTR CmdMess = GlobalLock (hCmdMess);
	        		GSSillseek (Fid,0,2);
	        		BigWrite (Fid,savepoke,_fstrlen(savepoke),-1); 
        			BigWrite (Fid,"\r\n",2,-1);
                    BigWrite (Fid,CmdMess,_fstrlen(CmdMess),-1);
        			BigWrite (Fid,"\r\n",2,-1);
        			if (DoTime)
        			{
		        		sprintf (str,"%f",CurView->ZMScale);
		        		BigWrite (Fid,str,_fstrlen(str),-1); 
	        			BigWrite (Fid,"\r\n",2,-1);
	        			TotBlocks = max (1,nBlocksIn+nBlocksOut);  
	        			TotTime = max (TotTime,1);
						sprintf (str,"%ld %ld %ld %ld %ld\r\n"
									 "Total Time\t%4.1f\r\n"
									 "Pct Blocks Displayed\t%6.2f\r\n"
									 "Pct Display Time\t%6.2f\r\n"
									 "Pct Map IO Time\t\t%6.2f\r\n"
									 "Pct Map Open Time\t%6.2f\r\n"
									 "Pct Text Time\t\t%6.2f\r\n"
									 "Pct Text Display\t%6.2f\r\n"
									 "Pct Pick Time\t\t%6.2f\r\n"
									 "Pct Intersect Time\t%6.2f\r\n"
									 "Pct Close Time\t\t%6.2f\r\n"
									 "Pct Dist Time\t\t%6.2f\r\n"
									 "Pct File Open\t\t%6.2f\r\n"
									 "Pct Unaccounted\t\t%6.2f",
	   					 		 lBlocksRead,nBlocksRead,nBlocksIn,nBlocksOut,NumFilesOpened,
	        					 (double)TotTime/CLOCKS_PER_SEC,
	        					 100.0 * ((double)nBlocksIn/TotBlocks),
	        					 100.0 * ((double)TotDisplayTime/TotTime),
	        					 100.0 * ((double)MapIOTime/TotTime),
	        					 100.0 * ((double)OpenMapTime/TotTime),
	        					 100.0 * ((double)TextTime/TotTime),
	        					 100.0 * ((double)TotTextDisplayTime/TotTime),
	        					 100.0 * ((double)PickTime/TotTime),
	        					 100.0 * ((double)IntersectTime/TotTime),
	        					 100.0 * ((double)CloseTime/TotTime),
	        					 100.0 * ((double)DistTime/TotTime),
	        					 100.0 * ((double)OpenFileTime/TotTime),
	        					 100.0 * ((double)(TotTime-(/*TotDisplayTime+MapIOTime+OpenMapTime+TextTime+*/PickTime+CloseTime+IntersectTime))/TotTime));
		        		BigWrite (Fid,str,_fstrlen(str),-1); 
	        			BigWrite (Fid,"\r\n",2,-1);
                    }
	        		GSSiClose (Fid); 
	        		GlobalUnlock (hCmdMess);
	        	}
		        
	        }
/*        ptr = _fstrchr(szPoke,':');
        if(ptr)
        {
           *ptr++ = '\0';
           _fstrcpy(szX,szPoke);
           X = atof(szX);
           _fstrcpy(szY,ptr); 
           Y = atof(szY);
           _fstrcpy(szPoke,szX);
           _fstrcat(szPoke,"  ");
           _fstrcat(szPoke,szY);
        } */
            for (i = 0; i < CFORMATS; i++) 
            {
                if (hDataCount[i]) DdeFreeDataHandle(hDataCount[i]);
                hDataCount[i] = 0;
            }
            DdePostAdvise(idInst, pXferInfo->hszTopic, pXferInfo->hszItem);
//            MessageBox(0,szPoke,"GeoMaster Locate",MB_ICONINFORMATION);
            return(1);
        }
        break;
    case XTYP_EXECUTE:  
    	ii=1;
       // pszComment = "Hi Wizard!";
       // InvalidateRect(hwndServer, &rcComment, TRUE);
        // notice no break here.  It goes right to work on the acknowledgement 
        break;
    case XTYP_REQUEST: 
    {
		LPSTR CmdMess = GlobalLock (hCmdMess); 
		DWORD	lCmd=_fstrlen(CmdMess)+1;
	       hData = DdeCreateDataHandle(idInst, CmdMess, lCmd, 0L, pXferInfo->hszItem, CF_TEXT, 0);
	       GlobalUnlock (hCmdMess);
		   return(hData); 
	}           
    	
       // pszComment = "Hi Wizard!";
       // InvalidateRect(hwndServer, &rcComment, TRUE);
        // notice no break here.  It goes right to work on the acknowledgement 
    case XTYP_ADVREQ:
        Delaydde(RenderDelay, FALSE);// RenderDelay initial set to zero
        if (!hDataCount[iFmt]) 
        {   // gets a global memory data handle
            hDataCount[iFmt] = DdeCreateDataHandle(idInst, 0, 0, 10, pXferInfo->hszItem,
                    pXferInfo->wFmt, fAppowned ? HDATA_APPOWNED : 0);
            if (pszData = DdeAccessData(hDataCount[iFmt], 0))//lock the global handle 
            {  // gets a pointer to the global memory and writes count value into it 
            
                wsprintf(pszData, "%s", presponse);
                DdeUnaccessData(hDataCount[iFmt]); // releases the global handle
            }
        }
        hData = hDataCount[iFmt];
        if (!fAppowned) hDataCount[iFmt] = 0;
        return(hData);
        break;
        
    case XTYP_ADVSTART:
        return(1);
    }
    return(0);
}  

/*******************************************************************\
* 
* Here's where we place the code for the service requests supported
* by our GeoMaster Video applications. lda
*
\*******************************************************************/

HDDEDATA VideoXfer(PXFERINFO pXferInfo, WORD iFmt)
{
  //  char szT[128];   // SS==DS!
    LPSTR pszData;
    HDDEDATA hData;
    WORD i;
    char *ptr;
    char *presponse, response[] = "OK From GeoMaster Video";
    presponse = response;
    ptr = szPoke;
    switch (pXferInfo->wType) 
    {
    case XTYP_POKE:
        // we expect an ascii number to replace the current count.
       // pszComment = "GWIZ poke received";
       // InvalidateRect(hwndServer, &rcComment, TRUE);
       // InvalidateRect(hwndServer, &rcCount, TRUE);
        if (DdeGetData(pXferInfo->hData, ptr, 127, 0))
        ptr = _fstrchr(szPoke,'\r');
        if(ptr) *ptr = '\0';
       // ptr = _fstrchr(pszComment,'\r');
       // *ptr = '\0'; 
        {
          //  *pszPoke[127] = '\0';  // just incase we overran.
          //  sscanf(szT, "%ld", &count);
            for (i = 0; i < CFORMATS; i++) 
            {
                if (hDataCount[i]) DdeFreeDataHandle(hDataCount[i]);
                hDataCount[i] = 0;
            }
            DdePostAdvise(idInst, pXferInfo->hszTopic, pXferInfo->hszItem);
            MessageBox(0,szPoke,"GeoMaster",MB_ICONINFORMATION);
            return(1);
        }
        break;
    case XTYP_EXECUTE:
       // pszComment = "Hi Wizard!";
       // InvalidateRect(hwndServer, &rcComment, TRUE);
        // notice no break here.  It goes right to work on the acknowledgement 
        break;
        
    case XTYP_REQUEST:
       // pszComment = "Hi Wizard!";
       // InvalidateRect(hwndServer, &rcComment, TRUE);
        // notice no break here.  It goes right to work on the acknowledgement 
    case XTYP_ADVREQ:
        Delaydde(RenderDelay, FALSE);// RenderDelay initial set to zero
        if (!hDataCount[iFmt]) 
        {   // gets a global memory data handle
            hDataCount[iFmt] = DdeCreateDataHandle(idInst, 0, 0, 10, pXferInfo->hszItem,
                    pXferInfo->wFmt, fAppowned ? HDATA_APPOWNED : 0);
            if (pszData = DdeAccessData(hDataCount[iFmt], 0))//lock the global handle 
            {  // gets a pointer to the global memory and writes count value into it 
            
                wsprintf(pszData, "%s", presponse);
                DdeUnaccessData(hDataCount[iFmt]); // releases the global handle
            }
        }
        hData = hDataCount[iFmt];
        if (!fAppowned) hDataCount[iFmt] = 0;
        return(hData);
        break;
        
    case XTYP_ADVSTART:
        return(1);
    }
    return(0);
}  
//*************************************************
HDDEDATA HelpXfer(PXFERINFO pXferInfo,WORD iFmt)
{
    HDDEDATA hData;
    
    switch (pXferInfo->wType) {
    case XTYP_REQUEST:
        pszComment = "Help text requested.";
      //  InvalidateRect(hwndServer, &rcComment, TRUE);
    case XTYP_ADVREQ:
        if (!hDataHelp[iFmt]) 
        {
            hDataHelp[iFmt] = DdeCreateDataHandle(idInst, szDdeHelp, _fstrlen(szDdeHelp) + 1,
                    0, pXferInfo->hszItem, pXferInfo->wFmt, fAppowned ? HDATA_APPOWNED : 0);
        }
        hData = hDataHelp[iFmt];
        if (!fAppowned)
            hDataHelp[iFmt] = 0;
        return(hData);
        break;
        
    case XTYP_ADVSTART:
        return(1);
    }
    return(0);
}
//*************************************************
 void Delaydde(DWORD delay, BOOL fModal)
{
    MSG msg;
    delay = GetCurrentTime() + delay;
    while (GetCurrentTime() < delay) 
    { 
        if (fModal && GSSiPeekMessage(&msg,0,0,0,PM_REMOVE)) 
        {
#if ENABLETRACE
SetLastMessage(-1*(long)msg.message);
#endif
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}
