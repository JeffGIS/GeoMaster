
           
// lda addition
BOOL InitDdeStuff(HWND  In_handle);
// end lda addition
           

DWORD idInst = 0;
CONVCONTEXT CCFilter = { sizeof(CONVCONTEXT), 0, 0, 0, 0L, 0L };
HANDLE hInst;                       /* current instance                      */
HWND hwndServer;
RECT rcComment;
/*RECT rcRand;
RECT rcCount;
RECT rcExec;
RECT rcConnCount;
RECT rcRndrDelay;
RECT rcRunaway;
RECT rcAllBlock;
RECT rcNextAction;                             
RECT rcHugeSize;
RECT rcAppowned;
RECT rcTopics; */
BOOL fAllBlocked = FALSE;
BOOL fAllEnabled = TRUE;
BOOL fEnableOneCB = FALSE;
BOOL fBlockNextCB = FALSE;
BOOL fTermNextCB = FALSE;
BOOL fAppowned = FALSE;
WORD cRunaway = 0;
WORD RenderDelay = 0;
DWORD count = 0;
WORD seed = 0;
  HSZ hszAppName = NULL;
  char szClass[] = "ServerWClass";
  char szTopic[MAX_TOPIC] = "gwiz";
  char szServer[MAX_TOPIC] = "gwizServer";
  char szComment[MAX_COMMENT];
  char szExec[MAX_EXEC] = "";
  char *pszComment;
  WORD cyText;
  WORD cServers = 0;
 HDDEDATA hDataHelp[CFORMATS] = {0};
 HDDEDATA hDataCount[CFORMATS] = {0};
 HDDEDATA hDataRand[CFORMATS] = {0};
 HDDEDATA hDataHuge[CFORMATS] = {0};
 DWORD cbHuge = 0;

  char szDdeHelp[] = "DDEML test server help:\r\n\n"\
    "The 'Server'(service) and 'Test'(topic) names may change.\r\n\n"\
    "Items supported under the 'Test' topic are:\r\n"\
    "\tCount:\tThis value increments on each data change.\r\n"\
    "\tRand:\tThis value is randomly generated each data change.\r\n"\
    "\tHuge:\tThis is randomlly generated text data >64k that the\r\n"\
    "\t\tDDEML test client can verify.\r\n"\
    "The above items change after any request if in Runaway mode and \r\n"\
    "can bo POKEed in order to change their values.  POKEed Huge data \r\n"\
    "must be in a special format to verify the correctness of the data \r\n"\
    "or it will not be accepted.\r\n"\
    "If the server is set to use app owned data handles, all data sent \r\n"\
    "uses HDATA_APPOWNED data handles."\
    ;

  FORMATINFO aFormats[CFORMATS] = {
    { 0, "TEXT" },      // exception!  predefined format, note:
                        // we use the standard imposed by EXCEL of not including
                        // the "CF_" and use all UPPERCASE.
    { 0, "Dummy1"  },
    { 0, "Dummy2"  },
};    


/*
 *          Topic and Item tables supported by this application.
 */

/*   HSZ    PROCEDURE       PSZ        */

  ITEMLIST SystemTopicItemList[CSYSTEMITEMS] = {
    
    { 0, TopicListXfer,  SZDDESYS_ITEM_TOPICS   },
    { 0, ItemListXfer,   SZDDESYS_ITEM_SYSITEMS },
    { 0, sysFormatsXfer, SZDDESYS_ITEM_FORMATS  },
    { 0, HelpXfer,       SZDDESYS_ITEM_HELP},
  };


  ITEMLIST TestTopicItemList[CTESTITEMS] = 
  {
    
    { 0,GWIZXfer, "Wizard" },   // 0 index
   // { 0, TestCountXfer,  "Count"},   // 1 index
   // { 0, TestHugeXfer,   "Huge" },   // 2 index
   //  { 0, ItemListXfer,   SZDDESYS_ITEM_SYSITEMS },  // 3 index
  };


/* The system topic is always assumed to be first. */   
/*   HSZ   PROCEDURE            #ofITEMS        PSZ     */
  TOPICLIST topicList[CTOPICS] = 
  {
    { 0, SystemTopicItemList,   CSYSTEMITEMS,   SZDDESYS_TOPIC},    // 0 index
    { 0, TestTopicItemList,     CTESTITEMS,     szTopic},           // 1 index
  };


