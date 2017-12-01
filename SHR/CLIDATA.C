/****************************************************************************

    MODULE: CLIDATA.C

    PURPOSE: Maintains conversation data and paints data in the client
	     application window.

****************************************************************************/

//Order of INCLUDES matters for compile time ... with /YX
  
#include "shr.h"
//#include "clires.h"
#include "client.h"

#include "gmextern.h"

typedef struct CITEM
{
    char szItem[ITEM_MAX_SIZE+1];
    char szValue[VALUE_MAX_SIZE+1];
};

typedef struct CONV
{
    BOOL  bInTerminate;
    enum PENDINGACK ePendingAck;
    HWND  hwndClientDDE;
    HWND  hwndServerDDE;
    char  szApplication[APP_MAX_SIZE+1];
    char  szTopic[TOPIC_MAX_SIZE+1];
    int   nItemCount;
    struct CITEM Item[ITEMS_PER_CONV_MAX_COUNT];
};

static struct CONV Conv[CONV_MAX_COUNT];
static HWND  hwndNewClientDDE; /* used by SelectNewConvDlgProc */
static HWND  hwndNewServerDDE; /* new conversation selected by user */
static char  szSelectedApplication[APP_MAX_SIZE+1];
static char  szSelectedTopic[TOPIC_MAX_SIZE+1];
static char  szSelectedItem[ITEM_MAX_SIZE+1];
static char  szSelectedValue[VALUE_MAX_SIZE+1];

static struct CONV * FindConv(HWND);
static struct CITEM * FindItem(HWND, char *);


/*****************************************************************

    FUNCTION: AddItemToConv

    PURPOSE:  Add a data item to an existing conversation.

*****************************************************************/
BOOL AddItemToConv(hwndClientDDE, szItem)
    HWND   hwndClientDDE;
    char * szItem;
{
    struct CONV * pConv;
    struct CITEM * pItem;

    if (!(pConv = FindConv(hwndClientDDE)))
        return (FALSE);
    if (pConv->nItemCount >= ITEMS_PER_CONV_MAX_COUNT)
        return (FALSE);
   pItem =  pConv->Item + pConv->nItemCount++;
   _fstrcpy(pItem->szItem, szItem);
   pItem->szValue[0] = 0;
   return (TRUE);
}




/**********************************************************

    FUNCTION:  AtLeastOneConvActive

    PURPOSE:   Used during termination of application, to
	       determine whether pending DDE terminations
	       have completed.

***********************************************************/
BOOL AtLeastOneConvActive()
{
    return (nConvCount? TRUE: FALSE);
}



/*****************************************************************

    FUNCTION: AddConv

    PURPOSE:  Initialize items in CONV structure.

*****************************************************************/
BOOL AddConv(hwndClientDDE, hwndServerDDE, szApplication, szTopic)
    HWND  hwndClientDDE;
    HWND  hwndServerDDE;
    char * szApplication;
    char * szTopic;
{
    struct CONV * pConv; 
    char	mess[256];

    if (nConvCount >= CONV_MAX_COUNT)
        return (FALSE);
    pConv = Conv + nConvCount++;
    pConv->bInTerminate = FALSE;
    pConv->ePendingAck = NONE;
    pConv->hwndClientDDE = hwndClientDDE;
    pConv->hwndServerDDE = hwndServerDDE;
    _fstrcpy(pConv->szApplication, szApplication);
    _fstrcpy(pConv->szTopic, szTopic);  
    sprintf (mess,"DDE conversation %i:%s: %s",nConvCount,szApplication,szTopic);
    GSSiTrace (mess,0);
    pConv->nItemCount = 0;
    return (TRUE);
}



/*****************************************************************

    FUNCTION: DoesAdviseAlreadyExist

    PURPOSE:  Determines whether hot/warm link has already been
	      established for specified conversation item.

*****************************************************************/
BOOL DoesAdviseAlreadyExist(hwndClientDDE, szItem)
    HWND hwndClientDDE;
    char * szItem;
{
    return (FindItem(hwndClientDDE,szItem)==NULL?FALSE:TRUE);
}



/*****************************************************************

    FUNCTION: FindItem

    PURPOSE:  Return pointer to item structure given conversation and item.

*****************************************************************/
struct CITEM * FindItem(hwndClientDDE, szItem)
    HWND hwndClientDDE;
    char * szItem;
{
    struct CONV * pConv;
    struct CITEM * pItem;
    int           nItemIndex;

    if (!(pConv = FindConv(hwndClientDDE)))
        return (NULL);
    for (pItem = pConv->Item, nItemIndex = 0;
	 nItemIndex < pConv->nItemCount;
         pItem++, nItemIndex++)
    {
        if (!strcmpi(szItem, pItem->szItem))
        {
            return (pItem);
        }
    }
    return (NULL);
}


/*****************************************************************

    FUNCTION: FindConv

    PURPOSE:  Return pointer to conversation structure given handle
	      to client DDE window.

*****************************************************************/
struct CONV * FindConv(hwndClientDDE)
    HWND  hwndClientDDE;
{
    struct CONV * pConv;
    int 	  nConvIndex;

    for (pConv = Conv, nConvIndex = 0;
	 nConvIndex < nConvCount;
	 pConv++, nConvIndex++)
    {
	if (pConv->hwndClientDDE == hwndClientDDE)
	    return (pConv);
    }
    return (NULL);
}


/***************************************************************

    FUNCTION:  FindConvGivenAppTopic

    PURPOSE:   Find handle of client DDE window given
	       application and topic for an active conversation.

***************************************************************/
HWND FindConvGivenAppTopic(szApp, szTopic)
    char * szApp;
    char * szTopic;
{
    struct CONV * pConv;
    int 	  nConvIndex;

    for (pConv = Conv, nConvIndex = 0;
	 nConvIndex < nConvCount;
	 pConv++, nConvIndex++)
    {
	 if (!(stricmp(pConv->szApplication, szApp))
	     && !(stricmp(pConv->szTopic, szTopic)))
	 {
	     return pConv->hwndClientDDE;
	 }
    }
    return NULL;
}


/*****************************************************************

    FUNCTION: GetAppAndTopic

    PURPOSE:  Get conversation's application and topic strings.

*****************************************************************/
void GetAppAndTopic(hwndClientDDE, szApp, szTopic)
    HWND  hwndClientDDE;
    char * szApp;
    char * szTopic;
{
    struct CONV * pConv;

    if (!(pConv = FindConv(hwndClientDDE)))
        return;
    _fstrcpy(szApp, pConv->szApplication);
    _fstrcpy(szTopic, pConv->szTopic);
    return;
}



/****************************************************************

    FUNCTION: GetConvPendingAck

    PURPOSE:  Return state of acknowledgement for specified
	      conversation.

****************************************************************/
enum PENDINGACK GetConvPendingAck(hwndClientDDE)
    HWND hwndClientDDE;
{
    struct CONV * pConv;

    if (pConv = FindConv(hwndClientDDE))
	return pConv->ePendingAck;
    else
        return NONE;
}



/*****************************************************************

    FUNCTION: GetHwndServerDDE

    PURPOSE:  Gets the hwnd of the server, given the handle of
	      the client DDE window.

*****************************************************************/
HWND GetHwndServerDDE(hwndClientDDE)
    HWND  hwndClientDDE;
{
    struct CONV * pConv;
    int  nConvIndex;

    for (pConv = Conv, nConvIndex = 0;
	 nConvIndex < nConvCount;
	 pConv++, nConvIndex++)
    {
	if (pConv->hwndClientDDE == hwndClientDDE && !pConv->bInTerminate)
	    return (pConv->hwndServerDDE);
    }
    return (NULL);
}



/*****************************************************************

    FUNCTION: GetNextConv

    PURPOSE:  Get next conversation in the list of current conversations.
	      To get the first conversation, pass a NULL hwnd.

*****************************************************************/
HWND GetNextConv(hwndClientDDE)
    HWND hwndClientDDE;
{
    struct CONV * pConv;
    int 	  nConvIndex;

    if (hwndClientDDE)
    {
		for (nConvIndex = 0, pConv = Conv;
	     	nConvIndex < nConvCount;
	     	nConvIndex++, pConv++)
        {
		    if (pConv->hwndClientDDE == hwndClientDDE && !pConv->bInTerminate)
	        {
				if (++nConvIndex < nConvCount)
				    return (++pConv)->hwndClientDDE;
		        else
		            return (NULL);
	        }
        }
        return (NULL);
    }
    if (nConvCount > 0)
		return (Conv[0].hwndClientDDE);
    else
        return (NULL);
}



/****************************************************************

    FUNCTION: HexToInt

    PURPOSE:  Convert from ascii to integer up to 4 chars
	      representing a hexidecimal number.  The hex number
	      is considered terminated by a null or any non-hex
	      digit.

****************************************************************/
int HexToInt(szHex)
    char * szHex;
{
    unsigned int  nReturn;
    int 	  nDigit;
    int 	  nIndex;

    nReturn = 0;
    for (nIndex = 0; nIndex < 4; nIndex++)
    {
	if (*szHex >= '0' && *szHex <= '9')
	    nDigit = *szHex - '0';
	else if (*szHex >= 'A' && *szHex <= 'F')
	    nDigit = *szHex - 'A' + 10;
	else if (*szHex >= 'a' && *szHex <= 'f')
	    nDigit = *szHex - 'a' + 10;
	else
	    return ((int)nReturn);
	nReturn = (nReturn<<4) + nDigit;
	szHex++;
    }
    return ((int)nReturn);
}




/****************************************************************

    FUNCTION: IsConvInTerminateState

    PURPOSE:  Determine whether conversation is in process of
	      being terminated.   In identifying the conversation,
	      it is necessary to compare both the hwndClientDDE
	      and hwndServerDDE, because during an initiate,
	      there may be multiple unwanted conversations sharing
	      the same hwndClientDDE.

****************************************************************/
BOOL IsConvInTerminateState(hwndClientDDE, hwndServerDDE)
    HWND hwndClientDDE;
    HWND hwndServerDDE;
{
    struct CONV * pConv;
    int  nConvIndex;

    for (nConvIndex = 0, pConv = Conv;
	 nConvIndex < nConvCount;
	 nConvIndex++, pConv++)
    {
	if (pConv->hwndClientDDE == hwndClientDDE
	    && pConv->hwndServerDDE == hwndServerDDE)
	    return pConv->bInTerminate;
    }
    return (TRUE);  /* If conversation not found, assume terminate state */
		    /* to avoid possible endless terminate feedback loop */
}



/****************************************************************

    FUNCTION: IsHwndClientDDEUsed

    PURPOSE:  Determine whether hwndClientDDE is still being
	      used in some second conversation after the
	      first conversation has been terminated.  This
	      determination is necessary during an initiate
	      in which the same hwndClientDDE is used in
	      multiple WM_DDE_INITIATE messages.
	      being terminated.

****************************************************************/
BOOL IsHwndClientDDEUsed(hwndClientDDE)
    HWND  hwndClientDDE;
{
    struct CONV * pConv;
    int  nConvIndex;

    for (pConv = Conv, nConvIndex = 0;
	 nConvIndex < nConvCount;
	 pConv++, nConvIndex++)
    {
	if (pConv->hwndClientDDE == hwndClientDDE)
	    return (TRUE);
    }
    return (FALSE);
}





/*****************************************************************

    FUNCTION: RemoveItemFromConv

    PURPOSE:  Remove an item (advise) from an existing conversation.

*****************************************************************/
BOOL RemoveItemFromConv(hwndClientDDE, szItem)
    HWND   hwndClientDDE;
    char * szItem;
{
    struct CONV * pConv;
    int           nItemIndex;
    struct CITEM * pItem;

    if (!(pConv = FindConv(hwndClientDDE)))
        return (FALSE);
    pItem = pConv->Item;
    nItemIndex = 0;
    while (nItemIndex < pConv->nItemCount)
    {
        if (!(strcmpi(pItem->szItem, szItem)))
            break;
        pItem++;
        nItemIndex++;
    }
    pConv->nItemCount--;
    while (nItemIndex < pConv->nItemCount)
    {
        *pItem = *(pItem+1);
        pItem++;
        nItemIndex++;
    }
    return (TRUE);
}



/*****************************************************************

    FUNCTION: RemoveConv

    PURPOSE:  Remove active conversation.   It is necessary to
	      specify both the client and server DDE windows,
	      since during an initiate, only the server DDE
	      window handles are distinguishable.

*****************************************************************/
BOOL RemoveConv(hwndClientDDE, hwndServerDDE)
    HWND  hwndClientDDE;
    HWND  hwndServerDDE;
{
    struct CONV * pRemoveConv;
    struct CONV * pConv;
    int 	  nConvIndex;

    for (pRemoveConv = Conv, nConvIndex = 0;
	 nConvIndex < nConvCount;
	 pRemoveConv++, nConvIndex++)
    {
		if (pRemoveConv->hwndClientDDE == hwndClientDDE
		    && pRemoveConv->hwndServerDDE == hwndServerDDE)
		    break;
    }
    if (nConvIndex >= nConvCount)
        return (FALSE);
    nConvIndex = 0;
    pConv = Conv;
    while (pConv != pRemoveConv)
    {
		if (++nConvIndex >= nConvCount)
	            return (FALSE);
		pConv++;
    }
    while (++nConvIndex < nConvCount)
    {
		*pConv = *(pConv+1);
		pConv++;
    }
    nConvCount--;
    return (TRUE);
}




/****************************************************************

    FUNCTION: SetConvInTerminateState

    PURPOSE:  Set conversation terminate state to TRUE.

****************************************************************/
void SetConvInTerminateState(hwndClientDDE, hwndServerDDE)
    HWND hwndClientDDE;
    HWND hwndServerDDE;
{
    struct CONV * pConv;
    int  nConvIndex;

    for (nConvIndex = 0, pConv = Conv;
	 nConvIndex < nConvCount;
	 nConvIndex++, pConv++)
    {
	if (pConv->hwndClientDDE == hwndClientDDE
	    && pConv->hwndServerDDE == hwndServerDDE)
	{
	    pConv->bInTerminate = TRUE;
	    return;
	}
    }
    return;
}



/****************************************************************

    FUNCTION: SetConvItemValue

    PURPOSE:  Find server data item and set value.

****************************************************************/
BOOL SetConvItemValue(hwndClientDDE, szItem, lpszValue)
    HWND hwndClientDDE;
    char * szItem;
    LPSTR  lpszValue;
{
   struct CITEM * pItem;
   char        * pcValue;

    if (pItem = FindItem(hwndClientDDE, szItem))
    {
        pcValue = pItem->szValue;
        /* copy until <CR> in CF_TEXT data */
        while (*lpszValue != '\r' && *lpszValue)
        {
            *pcValue++ = *lpszValue++;
        }
        *pcValue++ = 0;
	/* Repaint client application window */
	InvalidateRect(hWndMain, NULL, TRUE);
        return (TRUE);
    }
    return (FALSE);
}



/****************************************************************

    FUNCTION: SetConvPendingAck

    PURPOSE:  Set the state of acknowledgement for a specified
	      conversation.

****************************************************************/
void SetConvPendingAck(HWND hwndClientDDE, int ePendingAck)
{
    struct	CONV * pConv;  
    
    if (pConv = FindConv(hwndClientDDE))
    {
		pConv->ePendingAck = ePendingAck;
    }
    return;
} 

void WaitForAck (HWND hwndClientDDE)
{
    struct	CONV * pConv;  
    MSG		msg; 
//    char	str[64];

    if ((pConv = FindConv(hwndClientDDE)))
    {
		while (pConv->ePendingAck)
		{
	    	while (pConv->ePendingAck && GSSiPeekMessage(&msg,0,0,0,PM_REMOVE))
		    { 
//		      sprintf (str,"%ld %ld",(long)msg.hwnd,(long)msg.message);
//		      GSSiTrace (str);
#if ENABLETRACE
SetLastMessage(-1*(long)msg.message,msg.wParam);
#endif
		      if (msg.hwnd == hwndClientDDE ||
		      	  msg.message == WM_PAINT ||
		      		!IsChild (hWndMain,msg.hwnd))
		      {
		          TranslateMessage(&msg);
		          DispatchMessage(&msg);
		      }
		    }
		}
	}
	return;
} 
