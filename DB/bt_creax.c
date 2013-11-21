#include <io.h>
#include <limits.h>
#include "BT.h"
#include "shr.h"     
#include <float.h>

HGLOBAL	DebugID=999;

static	char			DATA2[512]; 
static	LPSTR			pCIDATA, pCIDATA2; //pCIDATA2 used when pointed to by CS_PNT
static	LPLONG		pBlockCount;
static	LPBTHEAD		BT_HEAD;
static	LPBTREE			pBTree;
static	LPCB	CI_PNT, CS_PNT; 
static	LPBTBLOCK			BT_BLOCK;
static	LPBTBUFF1	BT_BUFFERS;
static	LPBTBUFINDEX	BT_BUFF_CURINDEX;
static	LPBTBUFF2	BT_BUFF_INDEX;
static	LPBTI		INDEX_ITEM_PNT;
static	LPSTR		INDEX_DATA_PNT;
static	LPBTFSI	BT_FSPACE_PNT;
static	LPBTVARDESC		pBtVarDesc;
static	BOOL		READ_SECIDX=FALSE, BT_UPDATE, BT_END, TIME_CHECK=TRUE;
static	BOOL		SET_PARMS=FALSE;
static	int			i, j, IOFF, N;
static	int			MPN, MDN, SPN, SDN;
static	int			BTLEV, bt_get_date;
static	BTPOS		POS, LAST_POS, SAVE_POS;
static	long		BT_BLOCK_NUM, BT_BLOCK_POS;
static	char		SAVEKEY[256];
static	LPSTR		SAVE_AREA; 
static	HANDLE		hSAVE_AREA;
static	int			Version=0;  
extern	HWND		hWndMain; 
extern	BOOL		DoPaint;   


LPSTR BT_PATH_KEY_BEG(int BTLEV);
LPSTR BT_PATH_KEY_END(int BTLEV);

void BT_GET_BLOCK(void);
void FIND_PRIOR_BTPOS(void);
void FIND_NEXT_BTPOS(void);
void UMDB_WRITE_JOURNAL (BOOL INDEX);
void BT_INSERT (LPSTR KEY,LPSTR DATA, BOOL LASTRC);
int  AllocateBTMem (HGLOBAL hBTree);
void DeallocateBTMem(HGLOBAL hBTree);
BOOL BT_ALLOCATE_BUFFERS (void);
LPCB FREE_BT_REC (int LREC,long *LOC, int Type);
void FREE_BT_REC_ADD (int LENGTH);
void PROPAGATE_BT_UP (LPSTR KEY,LPSTR DATE);
void PROPAGATE_BT_DOWN(void);
void PROPAGATE_SPLIT(void);
void PROPAGATE_DELETE_UP (long UP);
int BT_DELETE_internal(LPCSTR KEY,LPCSTR DATA,BOOL SECIDX);
void BTreeErrorMessage (void);


void SetReadSecIndex (BOOL val)
{   
	READ_SECIDX = val;
	return;
}
 
BOOL GetBTHeader (HGLOBAL IBTID,LPBTHEAD BTHead)
{

    if (!IBTID) return (FALSE);
    if (!AllocateBTMem(IBTID)) return FALSE;
    *BTHead = *BT_HEAD;
    DeallocateBTMem(IBTID); 
    return (TRUE);
} 

BOOL GetBTVarDesc (HGLOBAL IBTID,LPBTVARDESC BTVar)
{
    BTHEAD	BTHead;
	LPBTVARDESC BTVarFrom; 
	short	n;   
	
    if (!IBTID) return (FALSE);
    if (!AllocateBTMem(IBTID)) return FALSE;
    BTVarFrom = &BT_HEAD->BT_VARDESC; 
    n = BT_HEAD->BT_NVARS; 
    while (n--)
    	*BTVar++ = *BTVarFrom++;
    DeallocateBTMem(IBTID); 
    return (TRUE);
} 

short GetBTKeyLen (HGLOBAL IBTID)
{   
	short	len;
	
    if (!IBTID) return 0;
    if (!AllocateBTMem(IBTID)) return FALSE;
    len = BT_HEAD->BT_KEYLEN;
    DeallocateBTMem(IBTID); 
    return len;
} 


void BT_SET_VERSION (int V)
{
	Version = V;
	return;
}

int BT_GET_VERSION (HGLOBAL IBTID)
{	int	irc;

    if (!IBTID) return (-1);
    if (!AllocateBTMem(IBTID)) return -1;
    irc = BT_HEAD->BT_VERSION;
    DeallocateBTMem(IBTID);
    return (irc);
}

BOOL BT_CREATE (LPSTR FNAME, int DATLEN, BOOL DATED, int NVAR, int NVAR2,
        	   LPBTVARDESC	pVarDesc, BOOL DUPS, int DUPPOS,
		 	   time_t TIME, BOOL JOURNAL)
{
/*%include '/sys/ins/ms.ins.ftn'
%include 'bt.ins.ftn' {/umsc/include/bt.ins.ftn}

      INTEGER*4 BLKSIZE,  HEAD_ADD, INDEX_HEAD_LEN, ST, BADDR,
     +          LEN_MAPPED_HEAD, BTST
      INTEGER*2 KEYLEN, DATLEN, IBTID, MDN, MPN, SPN, SDN, NVAR, DATE,
     +          VARTYP(8), VARLEN(8), FNLEN, QLEN, ACCESS, DUPPOS,
     +          VARTYP2(8), VARLEN2(8), NVAR2
      CHARACTER FNAME*128, QUAL*(*), TIME*6, MAXKEY*(256),
     +          BT_FNAMES(YY)*128
      COMMON /BTFNMS/ BT_FNAMES
      LOGICAL*1 BTID_IN_USE(YY)
      LOGICAL SET_PARMS, EXTEND, DUPS, DATED, JOURNAL,
     +        TIME_CHECK, DOCHECK
      DATA BTID_IN_USE /YY*.FALSE./, SET_PARMS /.FALSE./,
     +     TIME_CHECK /.TRUE./
*/
      HGLOBAL	hBTree;
      OFSTRUCT	OFStruct;
      long		keylen; 
      BOOL		rtn;

      BT_UPDATE   = FALSE;
      READ_SECIDX = FALSE;
      for (i=0,pBtVarDesc=pVarDesc,keylen=0;i<NVAR;pBtVarDesc++,i++)
          keylen += pBtVarDesc->BT_VARLEN;
      hBTree = GSSiGlobAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,sizeof(BTREE) +
      				       (MAX_BTREE_LEVELS * 2 + 2) * keylen);
      pBTree = (LPBTREE)GlobalLock(hBTree);
      pBTree->hBT_HEAD = GSSiGlobAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,1024);
      BT_HEAD = (LPBTHEAD)GlobalLock(pBTree->hBT_HEAD);
      BT_HEAD->BT_HEADLEN = /*sizeof(BTHEAD)+(NVAR-1)*sizeof(BTVARDESC);*/ 1024;
      /*BT_HEAD = MS_$CRMAPL (FNAME,FNLEN,32,1024,MS_$COWRITERS,ST)
      IF (ST .NE. 0) GO TO 1500*/
      BT_HEAD->BT_VERSION	= Version;
      BT_HEAD->BT_LENGTH    = 0;
      BT_HEAD->BT_FIRST_POS = 0;
      BT_HEAD->BT_BLKSIZE   = 4096; 
      if (DATLEN > 256)
	      BT_HEAD->BT_BLKSIZE *= 2; 
      BT_HEAD->BT_DATED     = DATED;
      BT_HEAD->BT_JOURNAL   = JOURNAL;
      BT_HEAD->BT_KEYLEN    = 0;  
      if (DUPPOS)
	  	BT_HEAD->BT_DUPPOS    = DUPPOS;
	  else
      	BT_HEAD->BT_DUPPOS    = NVAR;
      BT_HEAD->BT_KEYLEN	= keylen;
      BT_HEAD->BT_KYLEN     = BT_HEAD->BT_KEYLEN;
      if (BT_HEAD->BT_DATED) BT_HEAD->BT_KEYLEN += 2;
      BT_HEAD->BT_DATLEN    = DATLEN;
      BT_HEAD->BT_PNTLEN    = BT_HEAD->BT_KEYLEN + 4;
      BT_HEAD->BT_PNTLEN += BT_HEAD->BT_PNTLEN%2;
      BT_HEAD->BT_RECLEN    = BT_HEAD->BT_KEYLEN + BT_HEAD->BT_DATLEN;
      BT_HEAD->BT_RECLEN += BT_HEAD->BT_RECLEN%2;
      BT_HEAD->BT_NUMRECS   = 0;
      BT_HEAD->BT_MAX_BLOCK = 0;
      BT_HEAD->BT_BLOCK_HEAD_LEN = 12; /* LasttFreeBlock,NextFreeBlock;	BT_FSPACE_BEG; Dirty;*/
      BT_HEAD->BT_DUPS      = DUPS;
      BT_HEAD->BT_NSPARSE_KEY = 0;
      BT_HEAD->BT_TIME_STAMP= TIME;
      BT_HEAD->FirstFreeBlock[0]=0;
      BT_HEAD->FirstFreeBlock[1]=0;
      if (SET_PARMS)
      {		BT_HEAD->CI_MAX_PNUM  = MPN;
            BT_HEAD->CI_MAX_DNUM  = MDN;
            BT_HEAD->SPLIT_PNT_1  = SPN;
            BT_HEAD->SPLIT_PNT_2  = MPN-SPN;
            BT_HEAD->SPLIT_DAT_1  = SDN;
            BT_HEAD->SPLIT_DAT_2  = MDN-SDN;
      }
      else
      {     BT_HEAD->CI_MAX_PNUM  = 8;
            BT_HEAD->CI_MAX_DNUM  = 16;
            BT_HEAD->SPLIT_PNT_1  = 5;
            BT_HEAD->SPLIT_PNT_2  = 3;
            BT_HEAD->SPLIT_DAT_1  = 10;
            BT_HEAD->SPLIT_DAT_2  = 6;
      }
      SET_PARMS = FALSE;
      BT_HEAD->CI_LENGTH_1  = 16 + BT_HEAD->CI_MAX_PNUM * BT_HEAD->BT_PNTLEN; 
      if (BT_HEAD->TrackCount)
      	BT_HEAD->CI_LENGTH_1 += 4;	
      BT_HEAD->CI_LENGTH_2  = 16 + BT_HEAD->CI_MAX_DNUM * BT_HEAD->BT_RECLEN;
      BT_HEAD->BT_NVARS = NVAR;
      for (i=0,pBtVarDesc=&BT_HEAD->BT_VARDESC,IOFF=0;i<NVAR;pBtVarDesc++,pVarDesc++,i++)
      	{	*pBtVarDesc = *pVarDesc;
      		pBtVarDesc->BT_VAROFF = IOFF;
      		IOFF += pBtVarDesc->BT_VARLEN;
            if (pVarDesc->BT_VARTYP == BT_CHAR || pVarDesc->BT_VARTYP == BT_RIGHT_CHAR)
                pBtVarDesc->BT_VARTYP = BT_CHAR;
            else if(pVarDesc->BT_VARTYP == BT_REAL && pVarDesc->BT_VARLEN == 4)
                pBtVarDesc->BT_VARTYP = BT_REAL4;
            else if(pVarDesc->BT_VARTYP == BT_REAL4 && pVarDesc->BT_VARLEN == 4)
                pBtVarDesc->BT_VARTYP = BT_REAL4;
            else if (pVarDesc->BT_VARTYP == BT_REAL8 && pVarDesc->BT_VARLEN == 8)
                pBtVarDesc->BT_VARTYP = BT_REAL8;
            else if (pVarDesc->BT_VARTYP == BT_REAL && pVarDesc->BT_VARLEN == 8)
                pBtVarDesc->BT_VARTYP = BT_REAL8;
            else if (pVarDesc->BT_VARTYP == BT_INTEGER && pVarDesc->BT_VARLEN == 2)
                pBtVarDesc->BT_VARTYP = BT_INT2;
            else if (pVarDesc->BT_VARTYP == BT_INT2 && pVarDesc->BT_VARLEN == 2)
                pBtVarDesc->BT_VARTYP = BT_INT2;
            else if (pVarDesc->BT_VARTYP == BT_INTEGER && pVarDesc->BT_VARLEN == 4)
                pBtVarDesc->BT_VARTYP = BT_INT4;
            else if (pVarDesc->BT_VARTYP == BT_INT4 && pVarDesc->BT_VARLEN == 4)
                pBtVarDesc->BT_VARTYP = BT_INT4;
            else
                GSSiERROR (pBTree->MessageWindow,1,"Bad type in BT_CREATE");
      	}
      BT_HEAD->BT_BEING_UPDATED = FALSE;
      pBTree->BtFid = GSSiOpenFile (FNAME,&OFStruct,OF_CREATE); 
      if (pBTree->BtFid == HFILE_ERROR)
		rtn=FALSE;
      else
      {
		_lwrite (pBTree->BtFid,(LPSTR)BT_HEAD,(long)1024);
		GSSiClose (pBTree->BtFid); 
		rtn=TRUE;   
	  }
      GlobalUnlock (pBTree->hBT_HEAD);
      GlobalFree (pBTree->hBT_HEAD);
      GlobalUnlock (hBTree);
      GlobalFree (hBTree);
      /*IF (FNLEN .GT. 0) CALL MS_$UNMAP (BT_HEAD,1024,ST)*/
	  return(rtn);
}

void BT_GET_DEF (HGLOBAL IBTID,LPINT DataLength,LPINT NumVar,LPBTVARDESC pVarDesc)
{   LPBTVARDESC pBtVarDesc;

    if (!AllocateBTMem(IBTID)) return;
    *NumVar = BT_HEAD->BT_NVARS;
    *DataLength = BT_HEAD->BT_DATLEN;
    for (i=0,pBtVarDesc=&BT_HEAD->BT_VARDESC,IOFF=0;
    	 i<BT_HEAD->BT_NVARS;
    	 pBtVarDesc++,pVarDesc++,i++)
      	{	*pVarDesc = *pBtVarDesc;
            if (pBtVarDesc->BT_VARTYP == BT_CHAR)
                pVarDesc->BT_VARTYP = BT_CHAR;
            else if(pBtVarDesc->BT_VARTYP == BT_REAL4)
                pVarDesc->BT_VARTYP = BT_REAL;
            else if (pBtVarDesc->BT_VARTYP == BT_REAL8)
                pVarDesc->BT_VARTYP = BT_REAL;
            else if (pBtVarDesc->BT_VARTYP == BT_INT2)
                pVarDesc->BT_VARTYP = BT_INTEGER;
            else if (pBtVarDesc->BT_VARTYP == BT_INT4)
                pVarDesc->BT_VARTYP = BT_INTEGER;
      	}

    DeallocateBTMem(IBTID);
    return;
}



HGLOBAL BT_OPEN (LPSTR FNAME, time_t TIME, int ACCESS, int DATE)
{     HGLOBAL 	hBTree;
	  long		flen;
	  HGLOBAL	hBTHEADER;
	  int		Fid, NumWait;
	  BOOL		Error=FALSE;
	  OFSTRUCT	OFStruct;
      unsigned frequency=1000, duration=100; 
      char		SaveText[144], str[256];

      BT_UPDATE   = TRUE;
      READ_SECIDX = FALSE;

      hBTHEADER = GSSiGlobAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,1024);
      BT_HEAD = (LPBTHEAD)GlobalLock(hBTHEADER);
	  if (!ACCESS)
	      Fid = GSSiOpenFile (FNAME,(LPOFSTRUCT) &OFStruct,OF_READ);
      else
	      Fid = GSSiOpenFile (FNAME,(LPOFSTRUCT) &OFStruct,OF_READWRITE);
	
	  if (Fid == HFILE_ERROR)
	  {   
		 if (OFStruct.nErrCode>3)
		 {
			sprintf (str,"Error number %i on open (mode %i) in BT_OPEN",
					 (int)OFStruct.nErrCode,ACCESS);
			MessageBox( NULL, FNAME,str, MB_OK|MB_ICONQUESTION|MB_APPLMODAL);
		 } 
     Error:
     	 GSSiGlobUlFree (&hBTHEADER);
         if (Fid != HFILE_ERROR)
         	GSSiClose(Fid);
         hBTree = NULL;
         goto Exit;
	  }
      
      flen = GSSifilelength (Fid);
      if (flen < 1024) goto Error;
      _lread (Fid,(LPSTR)BT_HEAD,1024); 
      BT_HEAD->BT_HEADLEN =1024;
      hBTree = GSSiGlobAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,sizeof(BTREE) +
      				       (MAX_BTREE_LEVELS * 2 + 2) * BT_HEAD->BT_KEYLEN);
      pBTree = (LPBTREE)GlobalLock(hBTree);
      pBTree->hBT_HEAD = hBTHEADER;
      pBTree->BtFid = Fid;
      
      _fullpath (pBTree->BT_FNAME,FNAME,256);
      pBTree->BTID_DATE = DATE;
      if (ACCESS ==BT_WRITE)
           pBTree->BTID_READ = FALSE;
      else
           pBTree->BTID_READ = TRUE;

      if (!BT_ALLOCATE_BUFFERS ())
      	goto Error;

/*      IF (FNLEN .GT. 0) THEN
          BT_HEAD = MS_$MAPL (FNAME,FNLEN,32,1024,
     +                        MS_$COWRITERS,ACCESS,EXTEND,
     +                        LEN_MAPPED_HEAD,ST)
          IF (ST .NE. 0) RETURN
          ENDIF*/

/*      if (TIME != BT_HEAD->BT_TIME_STAMP && TIME_CHECK)
      {		
          	MessageBox( NULL, "Time Stamp Error",pBTree->BT_FNAME, MB_OK|MB_ICONQUESTION|MB_APPLMODAL);
      		Error = TRUE;
      }*/

/*      IF (FNLEN .GT. 0) THEN
              BT_BLOCK = MS_$MAPL (FNAME,FNLEN,INT4(BT_HEAD_LEN),
     +                             BT_BLKSIZE,MS_$COWRITERS,ACCESS,
     +                             EXTEND,LEN_MAPPED,ST)
          ELSE
              BT_BLOCK = MS_$CRMAPL (FNAME,FNLEN,INT4(BT_HEAD_LEN),
     +                               BT_BLKSIZE,MS_$COWRITERS,ST)
              LEN_MAPPED = BT_BLKSIZE       {Set for ms_$advice call below.
          ENDIF
      IF (ST .NE. 0) THEN
          CALL MS_$UNMAP (BT_HEAD,LEN_MAPPED_HEAD,IST)
          RETURN
          ENDIF
      CALL MS_$ADVICE (BT_BLOCK,LEN_MAPPED,MS_$RANDOM,0,
     +                 BT_BLKSIZE,IST) */

      pBTree->BTID_FIRST_CALL = TRUE;
      pBTree->BT_PATH_CHANGE  = TRUE;
	  SetBTMaxKey (BT_MAXKEY());
	  pBTree->BTID_CUR_BLK    = 0;
      pBTree->BTID_CUR_BLK_PNT= BT_BLOCK;
      _lseek (pBTree->BtFid,1024,0);
      _lread (pBTree->BtFid,BT_BLOCK,BT_HEAD->BT_BLKSIZE);

      if (!pBTree->BTID_READ) BT_HEAD->BT_BEING_UPDATED = TRUE;
      DeallocateBTMem(hBTree);
      if (Error)
      {	BT_CLOSE (hBTree);
      	hBTree = 0;
      }

 Exit:
	  sprintf (str,"Open BTree: %s  ID: %ld",FNAME,(long)hBTree);
	  GSSiTrace(str);

      return (hBTree);
}
BOOL BT_ALLOCATE_BUFFERS (void)
{     
	long	NumBTBuffers=14;
	
	  if (BT_HEAD->BT_BLKSIZE > 256)
	  	NumBTBuffers = 7;
      pBTree->hBT_BLOCK = GSSiGlobAlloc (LMEM_MOVEABLE,
      								   2 + 4 +
      								   (long)BT_HEAD->BT_BLKSIZE * NumBTBuffers +
      								   2 + 2 + 4 +
      								   sizeof (BTBUFINDEX) * NumBTBuffers);

      BT_BUFFERS =(LPBTBUFF1) GlobalLock(pBTree->hBT_BLOCK);
      BT_BUFFERS->maxbufs = NumBTBuffers;
      BT_BUFFERS->index_offset = 2 + 4 + BT_BUFFERS->maxbufs * (long)BT_HEAD->BT_BLKSIZE;
      BT_BUFF_INDEX = (LPBTBUFF2)((LPSTR) BT_BUFFERS + BT_BUFFERS->index_offset);
      BT_BUFF_INDEX->usedbufs = 1;
      BT_BUFF_INDEX->nextuse = LONG_MIN;
      BT_BUFF_INDEX->curbuf = 0;
      BT_BUFF_CURINDEX = &BT_BUFF_INDEX->index0;
      BT_BUFF_CURINDEX->bufid = 0;
      BT_BUFF_CURINDEX->bufuse = LONG_MIN;
      BT_BLOCK = (LPBTBLOCK)&BT_BUFFERS->FirstBuf;
      return (TRUE);
}

int AllocateBTMem (HGLOBAL hBTree)
{
      pBTree = (LPBTREE)GlobalLock(hBTree);
      if (!pBTree)
      	return(0);
      BT_HEAD = (LPBTHEAD)GlobalLock(pBTree->hBT_HEAD);   
      if (!BT_HEAD)
      	return (0);
      BT_BUFFERS = (LPBTBUFF1)GlobalLock(pBTree->hBT_BLOCK);
      BT_BUFF_INDEX = (LPBTBUFF2)((LPSTR) BT_BUFFERS + BT_BUFFERS->index_offset);
      BT_BLOCK = (LPBTBLOCK)((LPSTR)(&BT_BUFFERS->FirstBuf) + BT_BUFF_INDEX->curbuf * (long)BT_HEAD->BT_BLKSIZE);
      pBTree->BTID_CUR_BLK_PNT= BT_BLOCK;
      return(1);
}

void DeallocateBTMem(HGLOBAL hBTree)
{
      GlobalUnlock(pBTree->hBT_HEAD);
      GlobalUnlock(pBTree->hBT_BLOCK);
      pBTree = 0;
      GlobalUnlock(hBTree);
      return;
}


void	SetBTMaxKey (LPSTR Maxkey)
{	BTVARDESC *pVarDesc;
	int i, IOFF;
	LPSTR	pMKEY;
	typedef float FAR *LPFLOAT;
	typedef double FAR *LPDOUBLE;

	pMKEY = Maxkey;

      for (i=0,pVarDesc=&BT_HEAD->BT_VARDESC,IOFF=0;
       	   i<BT_HEAD->BT_NVARS;pVarDesc++,i++)
      	{	if (pVarDesc->BT_VARTYP == BT_CHAR || pVarDesc->BT_VARTYP == BT_RIGHT_CHAR)
                _fmemset (pMKEY,UCHAR_MAX,pVarDesc->BT_VARLEN);
            else if(pVarDesc->BT_VARTYP == BT_REAL4)
                *(LPFLOAT)pMKEY = FLT_MAX;
            else if (pVarDesc->BT_VARTYP == BT_REAL8)
                *(LPDOUBLE)pMKEY = DBL_MAX;
            else if (pVarDesc->BT_VARTYP == BT_INT2)
                *(LPINT)pMKEY = SHRT_MAX;
            else if (pVarDesc->BT_VARTYP == BT_INT4)
                *(LPLONG)pMKEY = LONG_MAX;
            pMKEY += pVarDesc->BT_VARLEN;
      	}
}

void	SetBTMinKey (LPSTR Maxkey)
{	BTVARDESC *pVarDesc;
	int i, IOFF;
	LPSTR	pMKEY;
	typedef float FAR *LPFLOAT;
	typedef double FAR *LPDOUBLE;

	pMKEY = Maxkey;

      for (i=0,pVarDesc=&BT_HEAD->BT_VARDESC,IOFF=0;
       	   i<BT_HEAD->BT_NVARS;pVarDesc++,i++)
      	{	if (pVarDesc->BT_VARTYP == BT_CHAR || pVarDesc->BT_VARTYP == BT_RIGHT_CHAR)
                _fmemset (pMKEY,0,pVarDesc->BT_VARLEN);
            else if(pVarDesc->BT_VARTYP == BT_REAL4)
                *(LPFLOAT)pMKEY = -FLT_MAX;
            else if (pVarDesc->BT_VARTYP == BT_REAL8)
                *(LPDOUBLE)pMKEY = -DBL_MAX;
            else if (pVarDesc->BT_VARTYP == BT_INT2)
                *(LPINT)pMKEY = SHRT_MIN;
            else if (pVarDesc->BT_VARTYP == BT_INT4)
                *(LPLONG)pMKEY = LONG_MIN;
            pMKEY += pVarDesc->BT_VARLEN;
      	}
}
BOOL BT_SET_TIME_STAMP (HGLOBAL IBTID, time_t TIME)
{	int	irc;

    if (!IBTID) return (FALSE);
    if (!AllocateBTMem(IBTID)) return FALSE;
    BT_HEAD->BT_TIME_STAMP = TIME;
    DeallocateBTMem(IBTID);
    return (TRUE);
}
/*      ENTRY BT_SET_TIME_STAMP (IBTID,TIME)
      BTID = IBTID
      IF (BTID .LT. 0 .OR. BTID .GT. YY) RETURN
      IF (.NOT. BTID_IN_USE(BTID)) RETURN
      BT_HEAD = BTID_HEAD(BTID)
      BT_TIME_STAMP = TIME
      RETURN*/
BOOL BT_CLOSEANDDELETE (LPHANDLE pIBTID)
{   
	char	DeleteName[256];
	OFSTRUCT	OFStruct;
	
	if (!*pIBTID)
		return FALSE;  
    if (!AllocateBTMem(*pIBTID)) return FALSE;
    _fstrcpy (DeleteName,pBTree->BT_FNAME);
    DeallocateBTMem(*pIBTID);
	BT_CLOSE (*pIBTID);
	*pIBTID = 0;
	GSSiOpenFile (DeleteName,&OFStruct,OF_DELETE);    
	return TRUE;
}

BOOL BT_GETPATHNAME (HANDLE IBTID,LPSTR Name)
{   
	OFSTRUCT	OFStruct;
	
	if (!IBTID)
		return FALSE;  
    if (!AllocateBTMem(IBTID)) return FALSE;
    _fstrcpy (Name,pBTree->BT_FNAME);
    DeallocateBTMem(IBTID);
	return TRUE;
}

int BT_CLOSE (HGLOBAL IBTID)
{     int	ST;
	  char	str[256];

      if (!IBTID) return (0);
      if (!AllocateBTMem(IBTID)) return(0);
/*      MS_$UNMAP(pBTree->BTID_CUR_BLK_PNT,BT_HEAD->BT_BLKSIZE,BTST)*/;
      ST = 0;
	  sprintf (str,"Close BTree: %s  ID: %ld",pBTree->BT_FNAME,(long)IBTID);
	  GSSiTrace(str);
      if (BT_HEAD->BT_BEING_UPDATED && !pBTree->BTID_READ)
      {
          BT_HEAD->CI_MAX_PNUM  = 8;
          BT_HEAD->CI_MAX_DNUM  = 16;
          BT_HEAD->SPLIT_PNT_1  = 4;
          BT_HEAD->SPLIT_PNT_2  = 4;
          BT_HEAD->SPLIT_DAT_1  = 10;
          BT_HEAD->SPLIT_DAT_2  = 6;
          BT_HEAD->BT_MAX_BLOCK    = BT_HEAD->BT_LENGTH / BT_HEAD->BT_BLKSIZE;
          BT_HEAD->BT_BEING_UPDATED = FALSE;
	      _llseek (pBTree->BtFid,0,0);
    	  _lwrite (pBTree->BtFid,(LPSTR)BT_HEAD,1024);
        /*  chsize (pBTree->BtFid,BT_HEAD->BT_HEADLEN + BT_HEAD->BT_LENGTH + 32);*/
     	  for (i = 0,BT_BUFF_CURINDEX = &BT_BUFF_INDEX->index0;
     	  	   i<BT_BUFF_INDEX->usedbufs;
     	  	   i++,BT_BUFF_CURINDEX++)
     	  {
			  BT_BLOCK = (LPBTBLOCK)((LPSTR)&BT_BUFFERS->FirstBuf + i * (long)BT_HEAD->BT_BLKSIZE);

	     	  if (BT_BLOCK->Dirty)
	     	  {    _lseek  (pBTree->BtFid,1024+BT_BUFF_CURINDEX->bufid*(long)BT_HEAD->BT_BLKSIZE,0);
	     	  	   BT_BLOCK->Dirty = FALSE;
	     	       _lwrite (pBTree->BtFid,(LPSTR)BT_BLOCK,BT_HEAD->BT_BLKSIZE);
	     	  }
	      }
      }
      /*MS_$UNMAP (BT_HEAD,1024,IST)*/;
      ST = GSSiClose (pBTree->BtFid);
      GlobalUnlock(pBTree->hBT_HEAD);
      GlobalFree (pBTree->hBT_HEAD);
      GlobalUnlock(pBTree->hBT_BLOCK);
      GlobalFree(pBTree->hBT_BLOCK);
      pBTree = 0;
      GlobalUnlock(IBTID);
      GlobalFree(IBTID);
	  return(ST);
}

/*      ENTRY BT_CLOSE_CHECKPOINT (BTST)
      GO TO 115

      ENTRY BT_CLEAR (IBTID)
      BTID = IBTID
      IF (BTID .LT. 0 .OR. BTID .GT. YY) RETURN
      IF (.NOT. BTID_IN_USE(BTID)) RETURN
      BT_HEAD = BTID_HEAD(BTID)
      IF (BT_BEING_UPDATED .AND. .NOT. BTID_READ(BTID)) THEN
          BT_NUMRECS = 0
          BT_LENGTH = 0
          BT_MAX_BLOCK = 0
          BT_FIRST_POS = 0
          CALL MS_$TRUNCATE (BT_HEAD,
     +                       BT_HEAD_LEN + BT_LENGTH + 32,IST)
          ENDIF
      BT_PATH_CHANGE(BTID) = .TRUE.
      RETURN */

BOOL BT_CLEAR (HGLOBAL IBTID)
{     int	ST;

      if (!IBTID) return (FALSE);
      if (!AllocateBTMem(IBTID)) return(FALSE);
      if (BT_HEAD->BT_BEING_UPDATED && !pBTree->BTID_READ)
      {
          BT_HEAD->BT_NUMRECS = 0;
          BT_HEAD->BT_LENGTH = 0;
          BT_HEAD->BT_MAX_BLOCK = 0;
          BT_HEAD->BT_FIRST_POS = 0; 
          pBTree->BT_PATH_CHANGE = TRUE;  
		  BT_BUFF_INDEX->usedbufs = 1;
          GSSiChangeLength (pBTree->BtFid,BT_HEAD->BT_HEADLEN + BT_HEAD->BT_LENGTH + 32);
          return TRUE;
      }
      return FALSE;
}

void BT_SET_PARMS (int IMPN, int IMDN, int ISPN, int ISDN)
{
      MPN = IMPN;
      MDN = IMDN;
      SPN = ISPN;
      SDN = ISDN;
      SET_PARMS = TRUE;
      return;
}

void DISABLE_TIME_STAMP_CHECK(void)
{
      TIME_CHECK =FALSE;
      return;
}

/*void GET_BT_FNAME (IBTID,FNAME)
      FNAME = BT_FNAMES(IBTID)
      RETURN
      END*/  
      
void BTreeErrorMessage (void)
{   
	DoPaint = FALSE;
	MessageBox( GetFocus(), "Error in BTree index",pBTree->BT_FNAME, MB_OK|MB_ICONEXCLAMATION|MB_APPLMODAL);
    BlowOut (NULL);
}

long BT_NUM_IN_INDEX (HGLOBAL IBTID)
{   long num;
    
    if (!IBTID)
    	return 0;
    if (!AllocateBTMem(IBTID)) return 0;
    num = BT_HEAD->BT_NUMRECS;
    DeallocateBTMem(IBTID);
    return (num);
}

int BT_FIND (HGLOBAL IBTID,LPSTR KEY,int POSITION,int COND,LPSTR DATA)
{	int		irc;
	BOOL	LASTRC;

    if (!IBTID) return (BT_NOT_FOUND);
    if (!AllocateBTMem(IBTID)) return (BT_NOT_FOUND);
    irc = BT_FIND_internal (KEY, POSITION, COND, DATA, &LASTRC);
    DeallocateBTMem(IBTID);
    return (irc);
}

int BT_FIND_internal (LPSTR KEY,int POSITION,int COND,LPSTR DATA,LPINT LASTRC)
{	int 	BTST, Compare;
	BOOL    BACKWARDS, BEFORE_FIRST, FIRST_CALL;
	long	DOWN;    
	int	ii;
/*      INTEGER*4 BTST, DOWN, RECS_IN_BT
      INTEGER*2 COND, POSITION, IBTID, MKEY_INIT(128), BTLEV
      CHARACTER KEY*256, LAST_POS*6, DATA*256, MAXKEY*256,
     +          ERRMSG*128, FNAME*128
      COMMON /BTLASK/ ERRMSG, FNAME
      LOGICAL LASTRC, BEFORE_FIRST, BACKWARDS
       character*2 bt_Get_date
       common /btgd/ bt_get_date
      EQUIVALENCE (MAXKEY, MKEY_INIT)
      DATA MKEY_INIT/128*16#FFFF/
*/
      BACKWARDS = FALSE;
      *LASTRC = FALSE;
      if (BT_HEAD->BT_DUPS && !BT_UPDATE) READ_SECIDX =TRUE;
      if (!BT_HEAD->BT_NUMRECS) return (BT_NOT_FOUND);
      if (POSITION == BT_PRIOR)
      {
          if (pBTree->BTID_FIRST_CALL) return (BT_NOT_FOUND);
          POS = pBTree->BTID_CUR_POS;
          LAST_POS = POS;
          BT_GET_BLOCK ();
          FIRST_CALL = FALSE;
          FIND_PRIOR_BTPOS();
          BTLEV = BT_HEAD->BT_NUMLEVS;
          if (BT_END) goto S1100;
          goto S1000;
      }
      else if (POSITION == BT_LAST)
      {
          pBTree->BTID_FIRST_CALL  = FALSE;
          if (BT_HEAD->BT_DATED) _fmemmove (&pBTree->BTID_LASKEY,BT_MAXKEY(),BT_HEAD->BT_KYLEN);
          POS.POSA   = BT_HEAD->BT_FIRST_POS;
          POS.POSB   = 1;
          BTLEV  = 1;
          BT_GET_BLOCK();
          POS.POSB   = CI_PNT->LNCI;
          BT_GET_BLOCK();
          DOWN   = INDEX_ITEM_PNT->BT_DOWN;
          BT_END = FALSE;
          FIRST_CALL = TRUE;
          BACKWARDS =TRUE;
      }
      else if (POSITION == BT_FIRST || pBTree->BTID_FIRST_CALL)
      {
              pBTree->BTID_FIRST_CALL  = FALSE;
	          if (BT_HEAD->BT_DATED) _fmemmove (&pBTree->BTID_LASKEY,BT_MAXKEY(),BT_HEAD->BT_KYLEN);
              if (pBTree->BT_PATH_CHANGE || BT_HEAD->BT_KEYLEN > 64 || COND == BT_ANY) goto S20;
              for (BTLEV=BT_HEAD->BT_NUMLEVS-1; BTLEV; BTLEV--)
              {		if (BT_KEY_COMPARE(KEY,BT_PATH_KEY_BEG(BTLEV),BT_UPPER)<0) continue;
                    if (BT_KEY_COMPARE(KEY,BT_PATH_KEY_END(BTLEV),BT_UPPER)>0) continue;
                    goto S10;
 			  }
              goto S20;
S10:          POS = pBTree->BT_PATH_POS[BTLEV-1];
              BTLEV++;
              goto S30;
S20:          POS.POSA = BT_HEAD->BT_FIRST_POS;
              POS.POSB = 1;
              BEFORE_FIRST =TRUE;
              BTLEV = 1;
S30:          BT_GET_BLOCK();
              DOWN     = INDEX_ITEM_PNT->BT_DOWN;
              BT_END   = FALSE;
              FIRST_CALL = TRUE;

      }
      else
      {
              POS = pBTree->BTID_CUR_POS;
              LAST_POS = POS;
              BT_GET_BLOCK();
              FIRST_CALL = FALSE;
              FIND_NEXT_BTPOS();
              BTLEV = BT_HEAD->BT_NUMLEVS;
      }

S100: if (BT_END) goto S1100;
      if (COND == BT_ANY) goto S1000;
      Compare = BT_KEY_COMPARE (KEY,&INDEX_ITEM_PNT->INDEX_ENTRY,CI_PNT->CITYPE);
      switch (Compare)
      {		case 1:
				if (COND == BT_GE || COND == BT_GT || COND == BT_EQ) goto S900;
         		goto S1000;
         		break;
			case -1:
		    	if (COND == BT_GE || COND == BT_GT) goto S1000;
 			    goto S1100;
 			    break;
		   	case 0:
		   		if (COND == BT_GE || COND == BT_LE || COND == BT_EQ)
		        {	DOWN = INDEX_ITEM_PNT->BT_DOWN;
			        goto S1000;
			    }
		        else if (COND == BT_GT) goto S900;
	            goto S1100;
	  }
S900:  if (CI_PNT->CITYPE == BT_BOTTOM)
      {		LAST_POS = POS;
            BEFORE_FIRST = FALSE;
      }
      else
            DOWN = INDEX_ITEM_PNT->BT_DOWN;
      if (POSITION == BT_PRIOR)
      {   FIND_PRIOR_BTPOS ();
          if (BT_END) goto S1100;
          goto S1000;
      }
      else
      {   if (BACKWARDS)
              FIND_PRIOR_BTPOS();
          else
              FIND_NEXT_BTPOS();
      }

      goto S100;

S1000: 
	  if (CI_PNT->LNCI <=0)
	  	BTreeErrorMessage ();

	   
	   if (CI_PNT->CITYPE == BT_BOTTOM)
       {      if (BT_HEAD->BT_DATED)
       		  {   if (_fmemcmp(IADDR((LPSTR)&INDEX_ITEM_PNT->INDEX_ENTRY,BT_HEAD->BT_KYLEN+1),
                      &pBTree->BTID_DATE,2) < 0) goto S900;
                  if (!_fmemcmp (&INDEX_ITEM_PNT->INDEX_ENTRY,
                      &pBTree->BTID_LASKEY,BT_HEAD->BT_KYLEN)) goto S900;
                  _fmemmove (&pBTree->BTID_LASKEY,&INDEX_ITEM_PNT->INDEX_ENTRY,BT_HEAD->BT_KYLEN);
                  _fmemmove (&bt_get_date,IADDR((LPSTR)&INDEX_ITEM_PNT->INDEX_ENTRY,2),2);
              }
              BTST = 0;
              pBTree->BTID_CUR_POS = POS;     
              if (BT_HEAD->BT_KYLEN == 10)
              	ii=1;
              _fmemmove (DATA,INDEX_DATA_PNT ,BT_HEAD->BT_DATLEN);
              _fmemmove (KEY,&INDEX_ITEM_PNT->INDEX_ENTRY,BT_HEAD->BT_KYLEN);
              if (FIRST_CALL) pBTree->BT_PATH_CHANGE = FALSE;
       }
       else
S1005: {      POS.POSA = DOWN;
              POS.POSB = 1;
              if (BT_HEAD->BT_KEYLEN <= 64)
              {
                  if (BT_END)
			      	_fmemmove (BT_PATH_KEY_END(BTLEV),BT_MAXKEY(),BT_HEAD->BT_KEYLEN);
                  else
			      	_fmemmove (BT_PATH_KEY_END(BTLEV),&INDEX_ITEM_PNT->INDEX_ENTRY,BT_HEAD->BT_KEYLEN);
                  pBTree->BT_PATH_POS[BTLEV-1] = POS;
              }
              BT_GET_BLOCK();
              if (BACKWARDS)
              {
                  POS.POSB = CI_PNT->LNCI;
                  BT_GET_BLOCK();
              }
              DOWN = INDEX_ITEM_PNT->BT_DOWN;
              if (BT_HEAD->BT_KEYLEN <= 64)
			      _fmemmove (BT_PATH_KEY_BEG(BTLEV),&INDEX_ITEM_PNT->INDEX_ENTRY,BT_HEAD->BT_KEYLEN);

              BTLEV += 1;
              BT_END = FALSE;
              goto S100;
       }
       goto S2000;

S1100: if (CI_PNT->CITYPE == BT_BOTTOM)
       {	if (FIRST_CALL) pBTree->BT_PATH_CHANGE = FALSE;
            if (BT_END && CI_PNT->NEXTCI == BT_NIL) *LASTRC =TRUE;
            BTST   = BT_NOT_FOUND;
            if (BEFORE_FIRST)
            	pBTree->BTID_FIRST_CALL = TRUE;
            else
                pBTree->BTID_CUR_POS = LAST_POS;
       }
       else
            goto S1005;
S2000: if (BTLEV != BT_HEAD->BT_NUMLEVS)
			BTreeErrorMessage ();
      return (BTST);
}

/*      ENTRY BT_RESET_LAST_KEY (IBTID)
C******* CALLING THIS ENTRY PRIOR TO CALLING BT_FIND ALLOWS RETURN
C        OF MULTIPLE DATES FOR THE SAME KEY
      BTID   = IBTID
      BT_HEAD = BTID_HEAD(BTID)
      if (BT_DATED) BTID_LASKEY(BTID)(:BT_KYLEN) =
     +              MAXKEY(:BT_KYLEN)
      RETURN*/

void UMDB_UPDATE_BT_DATA (HGLOBAL IBTID,LPSTR DATA)
{
    if (AllocateBTMem(IBTID)) return;
	POS = pBTree->BTID_CUR_POS;
    BT_GET_BLOCK();
    UMDB_WRITE_JOURNAL (TRUE);
    _fmemmove (INDEX_DATA_PNT,DATA,BT_HEAD->BT_DATLEN);
    DeallocateBTMem(IBTID);
    return;
}

/*    ENTRY BT_GET_NUM_RECS (IBTID,RECS_IN_BT)
      BTID   = IBTID
      BT_HEAD = BTID_HEAD(BTID)
      RECS_IN_BT = BT_NUMRECS
      RETURN
      END */

int BT_KEY_COMPARE (LPSTR KEY1,LPSTR KEY2,int ITYPE)
{	LPBTVARDESC pVarDesc;
	LPSTR		pBT_CRVAR1, pBT_CRVAR2;
	LPFLOAT		pBT_R4VAR1, pBT_R4VAR2;
	LPDOUBLE	pBT_R8VAR1, pBT_R8VAR2;
	LPINT		pBT_I2VAR1, pBT_I2VAR2;
	LPLONG		pBT_I4VAR1, pBT_I4VAR2;

      if (READ_SECIDX)
          N = BT_HEAD->BT_DUPPOS;
      else
          N = BT_HEAD->BT_NVARS;
      for (i=0,pVarDesc=&BT_HEAD->BT_VARDESC;i<N;i++,pVarDesc++)
      {		pBT_CRVAR1 = KEY1 + pVarDesc->BT_VAROFF;
   	  		pBT_CRVAR2 = KEY2 + pVarDesc->BT_VAROFF;
      		switch (pVarDesc->BT_VARTYP)
   	  		{	case BT_CHAR:  
   	  			case BT_RIGHT_CHAR:
   	  				j = _fmemcmp (pBT_CRVAR1,pBT_CRVAR2,pVarDesc->BT_VARLEN);
   	  				if (j!=0) return (j);
			       	break;

   	  			case BT_REAL4:
   	  				pBT_R4VAR1 = (LPFLOAT)pBT_CRVAR1;
   	  				pBT_R4VAR2 = (LPFLOAT)pBT_CRVAR2;
   	  				if (*pBT_R4VAR1 < *pBT_R4VAR2) return (-1);
   	  				if (*pBT_R4VAR1 > *pBT_R4VAR2) return (1);
			       	break;

   	  			case BT_REAL8:
   	  				pBT_R8VAR1 = (LPDOUBLE)pBT_CRVAR1;
   	  				pBT_R8VAR2 = (LPDOUBLE)pBT_CRVAR2;
   	  				if (*pBT_R8VAR1 < *pBT_R8VAR2) return (-1);
   	  				if (*pBT_R8VAR1 > *pBT_R8VAR2) return (1);
			       	break;

   	  			case BT_INT2:
   	  				pBT_I2VAR1 = (LPINT)pBT_CRVAR1;
   	  				pBT_I2VAR2 = (LPINT)pBT_CRVAR2;
   	  				if (*pBT_I2VAR1 < *pBT_I2VAR2) return (-1);
   	  				if (*pBT_I2VAR1 > *pBT_I2VAR2) return (1);
			       	break;

   	  			case BT_INT4:
   	  				pBT_I4VAR1 = (LPLONG)pBT_CRVAR1;
   	  				pBT_I4VAR2 = (LPLONG)pBT_CRVAR2;
   	  				if (*pBT_I4VAR1 < *pBT_I4VAR2) return (-1);
   	  				if (*pBT_I4VAR1 > *pBT_I4VAR2) return (1);
			       	break;
              }
      } /* end for loop */

      if (BT_HEAD->BT_DATED && ITYPE == BT_UPPER)
      {
  	  		pBT_CRVAR2 = KEY2 + BT_HEAD->BT_KYLEN;
			pBT_I2VAR2 = (LPINT)pBT_CRVAR2;
			if (*pBT_I2VAR1 < *pBT_I2VAR2) return (-1);
			if (*pBT_I2VAR1 > *pBT_I2VAR2) return (1);
      }
      return (0);
}

void BT_GET_BLOCK(void)
{     int	inc;
	  long	OldestUse;
	  int	OldestBuf;
	  int	i;

      BT_BLOCK = pBTree->BTID_CUR_BLK_PNT;
      BT_BLOCK_NUM = POS.POSA / BT_HEAD->BT_BLKSIZE;
      if (BT_BLOCK_NUM != pBTree->BTID_CUR_BLK)
      {
/*          BT_BLOCK = MS_$REMAP (BT_BLOCK,
     +                          BT_HEAD_LEN+BT_BLOCK_NUM*BT_BLKSIZE,
     +                          BT_BLKSIZE,IDUM,ST)*/
     	  for (i = 0,BT_BUFF_CURINDEX = &BT_BUFF_INDEX->index0, OldestUse=LONG_MAX;
     	  	   i<BT_BUFF_INDEX->usedbufs;
     	  	   i++,BT_BUFF_CURINDEX++)
     	  {
      		if (BT_BLOCK_NUM == BT_BUFF_CURINDEX->bufid)
      		{	BT_BLOCK = (LPBTBLOCK)((LPSTR)&BT_BUFFERS->FirstBuf + i * (long)BT_HEAD->BT_BLKSIZE);
      			goto S100;
      		}
      		if (BT_BUFF_CURINDEX->bufuse < OldestUse)
      		{	OldestUse = BT_BUFF_CURINDEX->bufuse;
      			OldestBuf = i;
      		}
      	  }
      	  if (i == BT_BUFFERS->maxbufs)
      	  {
	          i = OldestBuf;
			  BT_BUFF_CURINDEX = &BT_BUFF_INDEX->index0 + i;
			  BT_BLOCK = (LPBTBLOCK)((LPSTR)&BT_BUFFERS->FirstBuf + i * (long)BT_HEAD->BT_BLKSIZE);

	     	  if (BT_BLOCK->Dirty)
	     	  {    _lseek  (pBTree->BtFid,1024+BT_BUFF_CURINDEX->bufid*(long)BT_HEAD->BT_BLKSIZE,0);
	     	  	   BT_BLOCK->Dirty = FALSE;
	     	       _lwrite (pBTree->BtFid,(LPSTR)BT_BLOCK,(UINT)BT_HEAD->BT_BLKSIZE);
	     	  }
	       }
	       else
	       {
	       	  BT_BUFF_INDEX->usedbufs++;
	       	  BT_BLOCK = (LPBTBLOCK)((LPSTR)&BT_BUFFERS->FirstBuf + (long)i * (long)BT_HEAD->BT_BLKSIZE);
	       }
		  _lseek (pBTree->BtFid,1024+BT_BLOCK_NUM*BT_HEAD->BT_BLKSIZE,0);
          _lread (pBTree->BtFid,BT_BLOCK,BT_HEAD->BT_BLKSIZE);

S100:	  BT_BUFF_CURINDEX->bufuse = BT_BUFF_INDEX->nextuse++;
     	  BT_BUFF_CURINDEX->bufid=BT_BLOCK_NUM;
      	  BT_BUFF_INDEX->curbuf = i;
          pBTree->BTID_CUR_BLK = BT_BLOCK_NUM;
          pBTree->BTID_CUR_BLK_PNT = BT_BLOCK;
      }
	  BT_BLOCK_POS   = (POS.POSA%BT_HEAD->BT_BLKSIZE)+1;
      CI_PNT         = (LPCB)IADDR ((LPSTR)&BT_BLOCK->BT_DATA,BT_BLOCK_POS);
	  pCIDATA = (LPSTR)&CI_PNT->CIDATA;
	  pBlockCount = NULL;
      INDEX_ITEM_PNT = (LPBTI)CI_PNT;
      if (CI_PNT->CITYPE == BT_UPPER)
      {
         inc = 16 + (POS.POSB-1) * BT_HEAD->BT_PNTLEN; 
         if (BT_HEAD->TrackCount) 
         {
         	pBlockCount = &CI_PNT->CIDATA;
         	pCIDATA += 4;
         	inc += 4;
         }
         INDEX_ITEM_PNT = (LPBTI)((LPSTR)INDEX_ITEM_PNT + inc);
      }
      else
      {  
         inc = 12 + (POS.POSB-1) * BT_HEAD->BT_RECLEN; /* no DOWN pointer on bottom lever*/
         INDEX_ITEM_PNT = (LPBTI)((LPSTR)INDEX_ITEM_PNT + inc);
         INDEX_DATA_PNT = (LPSTR)INDEX_ITEM_PNT;
         INDEX_DATA_PNT += BT_HEAD->BT_PNTLEN;
      }
      return;
}

void FIND_PRIOR_BTPOS()
{

      BT_END = FALSE;
      if (POS.POSB > CI_PNT->LNCI)
			BTreeErrorMessage ();
      if (POS.POSB == 1)
      {
          if (CI_PNT->LASTCI == BT_NIL)
          {
              BT_END = TRUE;
              return;
          }
          POS.POSA = CI_PNT->LASTCI;
          BT_GET_BLOCK();
          POS.POSB = CI_PNT->LNCI;
          BT_GET_BLOCK();
       }
       else
       {
          POS.POSB--;
          if (CI_PNT->CITYPE == BT_UPPER)
              INDEX_ITEM_PNT = (LPBTI)((LPSTR) INDEX_ITEM_PNT + BT_HEAD->BT_PNTLEN);
          else
          {
              INDEX_ITEM_PNT = (LPBTI)((LPSTR) INDEX_ITEM_PNT - BT_HEAD->BT_RECLEN);
              INDEX_DATA_PNT = INDEX_DATA_PNT - BT_HEAD->BT_RECLEN;
          }
       }
       return;
}

void FIND_NEXT_BTPOS()
{
      BT_END = FALSE;
      if (POS.POSB > CI_PNT->LNCI)
			BTreeErrorMessage ();
      if (POS.POSB == CI_PNT->LNCI)
      {
          if (CI_PNT->NEXTCI == BT_NIL)
          {
              BT_END = TRUE;
              POS.POSB++;
              if (CI_PNT->CITYPE == BT_BOTTOM)
              {
                  INDEX_ITEM_PNT = (LPBTI)((LPSTR) INDEX_ITEM_PNT + BT_HEAD->BT_RECLEN);
                  INDEX_DATA_PNT = INDEX_DATA_PNT + BT_HEAD->BT_RECLEN;
              }
              return;
           }
           POS.POSA = CI_PNT->NEXTCI;
           POS.POSB = 1;
           BT_GET_BLOCK();
      }
      else
      {
           POS.POSB++;
           if (CI_PNT->CITYPE == BT_UPPER)
              INDEX_ITEM_PNT = (LPBTI)((LPSTR) INDEX_ITEM_PNT + BT_HEAD->BT_PNTLEN);
           else
           {
              INDEX_ITEM_PNT = (LPBTI)((LPSTR) INDEX_ITEM_PNT + BT_HEAD->BT_RECLEN);
              INDEX_DATA_PNT = INDEX_DATA_PNT + BT_HEAD->BT_RECLEN;
           }
	  }
	  return;
}

LPSTR BT_MAXKEY()
{   LPSTR 	ipnt;
	int 	off;
	ipnt = (LPSTR)pBTree;
	ipnt += sizeof(BTREE) -1;
	off = BT_HEAD->BT_KEYLEN;
	return (ipnt += off);
}


LPSTR BT_PATH_KEY_BEG(int BTLEV)
{   LPSTR 	ipnt;
	int 	off,ii;  
	if (BTLEV < 1 || BTLEV > 10)
		ii=1;
	ipnt = (LPSTR)pBTree;
	ipnt += sizeof(BTREE) -1;
	off = BT_HEAD->BT_KEYLEN * BTLEV;
	return (ipnt += off);
} 

void BT_GETDUPDATA (LPSTR DupData,short LenDupData)
{
	_fmemcpy (DupData,DATA2,LenDupData);
	return;
}

LPSTR BT_PATH_KEY_END(int BTLEV)
{   LPSTR 	ipnt;
	int 	off;
	ipnt = (LPSTR)pBTree;
	ipnt += sizeof(BTREE) -1;
	off = BT_HEAD->BT_KEYLEN*16 + BT_HEAD->BT_KEYLEN * BTLEV;
	return (ipnt += off);
}

int BT_PUT (HGLOBAL hBTree, LPSTR KEY,LPSTR DATA)
{	  int irc; 
	  static n=0,debugoff=5368;   
	  LPLONG	pOff;
	  short	ii;
      
      pOff = DATA;
      if (*pOff == debugoff)
      	ii=1;
	  if (!AllocateBTMem(hBTree)) return 1;
      irc = BT_PUT_internal (KEY,DATA,FALSE,TRUE,0);
      DeallocateBTMem(hBTree);
      return (irc);
}


int BT_PUT_NO_FIND (HGLOBAL hBTree,LPSTR KEY,LPSTR DATA, BOOL LRC, int BTST)
{	  int irc; 
	if (!AllocateBTMem(hBTree)) return 1;
	irc = BT_PUT_internal (KEY,DATA,LRC,FALSE,BTST);
    DeallocateBTMem(hBTree);
    return (irc);
}

int BT_PUT_internal (LPSTR KEY,LPSTR DATA, BOOL LRC, BOOL DO_FIND, int BTST)
{     BOOL LASTRC;
	  int ST, LREC;

      BT_UPDATE = TRUE;
      ST = BTST;
      LASTRC = LRC;
/******* IF THIS IS A NEW B-TREE INITIALIZE IT*/
      if (!BT_HEAD->BT_NUMRECS)
      {
          BT_HEAD->BT_FIRST_POS = 0;
          POS.POSA    = 0;
          POS.POSB    = 1;
          BT_GET_BLOCK();
          UMDB_WRITE_JOURNAL (TRUE);
          CI_PNT->LASTCI  = BT_NIL;
          CI_PNT->NEXTCI  = BT_NIL;
          CI_PNT->UPCI    = BT_NIL;
          CI_PNT->LNCI    = 1;
          CI_PNT->CITYPE  = BT_UPPER;
          INDEX_ITEM_PNT = (LPBTI)((LPSTR)CI_PNT + 16);
          INDEX_ITEM_PNT->BT_DOWN = BT_HEAD->BT_BLKSIZE;
          _fmemmove (&INDEX_ITEM_PNT->INDEX_ENTRY,KEY,BT_HEAD->BT_KYLEN);
          if (BT_HEAD->BT_DATED)
     		  _fmemmove (IADDR(&INDEX_ITEM_PNT->INDEX_ENTRY,BT_HEAD->BT_KYLEN+1),
     		  			 &pBTree->BTID_DATE,2);
          LREC = BT_HEAD->CI_LENGTH_1;
          BT_BLOCK->BT_FSPACE_BEG = LREC + 1;
          BT_BLOCK->Dirty = TRUE;
          BT_BLOCK->NextFreeBlock = UINT_MAX;
          BT_BLOCK->LastFreeBlock = UINT_MAX;
          BT_BLOCK->Type = BT_UPPER;
          BT_BLOCK->RecLen = BT_HEAD->CI_LENGTH_1;
     	  BT_HEAD->FirstFreeBlock[0]=0;
          BT_FSPACE_PNT = (LPBTFSI)IADDR ((LPSTR)&BT_BLOCK->BT_DATA,LREC+1);
          BT_FSPACE_PNT->BT_FSPACE_LEN =
          		 BT_HEAD->BT_BLKSIZE - LREC - 6 - BT_HEAD->BT_BLOCK_HEAD_LEN;
          BT_FSPACE_PNT->BT_FSPACE_LAST = 0;
          BT_FSPACE_PNT->BT_FSPACE_NEXT = -1;
          pBTree->BTID_CUR_POS = POS;
          pBTree->BTID_FIRST_CALL  = FALSE;

          /* Insert lower segment */
          POS.POSA    = BT_HEAD->BT_BLKSIZE;
          POS.POSB    = 1;
          BT_GET_BLOCK();
          UMDB_WRITE_JOURNAL (TRUE);
          CI_PNT->LASTCI  = BT_NIL;
          CI_PNT->NEXTCI  = BT_NIL;
          CI_PNT->UPCI    = 0;
          CI_PNT->LNCI    = 1;
          CI_PNT->CITYPE  = BT_BOTTOM;
          INDEX_ITEM_PNT = (LPBTI)((LPSTR) CI_PNT + 12);
          INDEX_DATA_PNT = (LPSTR) INDEX_ITEM_PNT + BT_HEAD->BT_PNTLEN;
          _fmemmove (&INDEX_ITEM_PNT->INDEX_ENTRY,KEY,BT_HEAD->BT_KYLEN);
          if (BT_HEAD->BT_DATED)
     		  _fmemmove (IADDR(&INDEX_ITEM_PNT->INDEX_ENTRY,BT_HEAD->BT_KYLEN+1),
     		             &pBTree->BTID_DATE,2);
          _fmemmove (INDEX_DATA_PNT,DATA,BT_HEAD->BT_DATLEN);
     	  BT_HEAD->FirstFreeBlock[1]=1;
          BT_HEAD->BT_LENGTH = BT_HEAD->BT_BLKSIZE + BT_HEAD->CI_LENGTH_2 +
          					   BT_HEAD->BT_BLOCK_HEAD_LEN;
          BT_HEAD->BT_NUMLEVS = 2;
          BT_HEAD->BT_NUMRECS = 1;
          LREC = BT_HEAD->CI_LENGTH_2;
          BT_BLOCK->BT_FSPACE_BEG = LREC + 1;
          BT_BLOCK->Dirty = TRUE;
          BT_BLOCK->NextFreeBlock = UINT_MAX;
          BT_BLOCK->LastFreeBlock = UINT_MAX;
          BT_BLOCK->Type = BT_BOTTOM;
          BT_BLOCK->RecLen = BT_HEAD->CI_LENGTH_2;
          BT_FSPACE_PNT = (LPBTFSI)IADDR ((LPSTR)&BT_BLOCK->BT_DATA,LREC+1);
          BT_FSPACE_PNT->BT_FSPACE_LEN =
          		 BT_HEAD->BT_BLKSIZE - LREC - 6 - BT_HEAD->BT_BLOCK_HEAD_LEN;
          BT_FSPACE_PNT->BT_FSPACE_LAST = 0;
          BT_FSPACE_PNT->BT_FSPACE_NEXT = -1;
          BT_UPDATE = FALSE;
          BT_HEAD->BT_MAX_BLOCK++;
          ST = 31;
          return(ST);
      }

      if (DO_FIND)
          ST = BT_FIND_internal (KEY,BT_FIRST,BT_EQ,DATA2,&LASTRC);
      if (!ST)
      {
          if (BT_HEAD->BT_DATED)
              if (!_fmemcmp (IADDR(&INDEX_ITEM_PNT->INDEX_ENTRY,BT_HEAD->BT_KYLEN+1),
              			  &pBTree->BTID_DATE,2)) goto S100;

          UMDB_WRITE_JOURNAL (TRUE);
          _fmemmove (INDEX_DATA_PNT,DATA,BT_HEAD->BT_DATLEN);
		  BT_UPDATE = FALSE;
		  return (ST);
	  }

S100: BT_INSERT (KEY,DATA,LASTRC);
      BT_HEAD->BT_NUMRECS = BT_HEAD->BT_NUMRECS + 1;
      BT_UPDATE = FALSE;
      return (ST);
}

void UMDB_WRITE_JOURNAL (BOOL INDEX)
{
	BT_BLOCK->Dirty = TRUE;
}

void BT_INSERT (LPSTR KEY,LPSTR DATA, BOOL LASTRC)
{	int		 LN, IB, SPOSB;
	long	 NEWPOS, SAVE_POSA, SNEXT, SUP, SPOSA;

/*      CHARACTER KEY*256, DATA*256
      INTEGER*4 LN, SAVE_POS, SPOSA, NEWPOS, FREE_BT_REC, SNEXT, SUP
      INTEGER*2 IB, SPOSB
      LOGICAL   LASTRC*/

      if (LASTRC)
      {
          if (POS.POSB <= BT_HEAD->CI_MAX_DNUM)
          {
             BT_GET_BLOCK ();
             goto S100;
          }
          else
             goto S110;
      }
S10:  if (CI_PNT->LNCI < BT_HEAD->CI_MAX_DNUM)
	  {
          UMDB_WRITE_JOURNAL (TRUE);
          IB = (POS.POSB-1) * BT_HEAD->BT_RECLEN + 1;
          LN = (CI_PNT->LNCI - (POS.POSB-1)) * BT_HEAD->BT_RECLEN;
          if (LN > 0) _fmemmove (IADDR((LPSTR)&CI_PNT->CIDATA,IB+BT_HEAD->BT_RECLEN),
          						 IADDR((LPSTR)&CI_PNT->CIDATA,IB),LN);
S100:     UMDB_WRITE_JOURNAL (TRUE);
          CI_PNT->LNCI += 1;
          _fmemmove (&INDEX_ITEM_PNT->INDEX_ENTRY,KEY,BT_HEAD->BT_KYLEN);
          if (BT_HEAD->BT_DATED)
     		  _fmemmove (IADDR(&INDEX_ITEM_PNT->INDEX_ENTRY,BT_HEAD->BT_KYLEN+1),
     		  			 &pBTree->BTID_DATE,2);
          _fmemmove (INDEX_DATA_PNT,DATA,BT_HEAD->BT_DATLEN);
          pBTree->BTID_CUR_POS = POS;
          if (POS.POSB == 1) PROPAGATE_BT_UP(KEY,(LPSTR)&pBTree->BTID_DATE);
       }
       else
       {
          if (POS.POSB > CI_PNT->LNCI)
          {
              if (CI_PNT->NEXTCI == BT_NIL) goto S110;
              POS.POSA = CI_PNT->NEXTCI;
              POS.POSB = 1;
              BT_GET_BLOCK();
              goto S10;
          }
S110:     if (POS.POSB == 1 && CI_PNT->LASTCI != BT_NIL)
	      {
              SAVE_POSA = POS.POSA;
              POS.POSA     = CI_PNT->LASTCI;
              BT_GET_BLOCK();
              if (CI_PNT->LNCI < BT_HEAD->CI_MAX_DNUM)
              {
                  IB = CI_PNT->LNCI * BT_HEAD->BT_RECLEN + 1;
                  POS.POSB = CI_PNT->LNCI + 1;
                  BT_GET_BLOCK();
                  goto S100;
              }
              else
              {
                  POS.POSA = SAVE_POSA;
                  BT_GET_BLOCK();
              }
          }
          UMDB_WRITE_JOURNAL (TRUE);
          CI_PNT->LNCI = BT_HEAD->SPLIT_DAT_1;
          IB     = BT_HEAD->SPLIT_DAT_1 * BT_HEAD->BT_RECLEN + 1;
          LN     = BT_HEAD->SPLIT_DAT_2 * BT_HEAD->BT_RECLEN; 
          hSAVE_AREA = GSSiGlobAlloc (GMEM_MOVEABLE,BT_HEAD->CI_LENGTH_2);
          SAVE_AREA = GlobalLock (hSAVE_AREA);
          _fmemmove (SAVE_AREA,IADDR((LPSTR)&CI_PNT->CIDATA,IB),(UINT)LN);
          SNEXT  = CI_PNT->NEXTCI;
          SUP    = CI_PNT->UPCI;
          CS_PNT = FREE_BT_REC (BT_HEAD->CI_LENGTH_2,&NEWPOS,BT_BOTTOM);
          _fmemmove ((LPSTR)&CS_PNT->CIDATA,SAVE_AREA,(UINT)LN);
          GSSiGlobUlFree (&hSAVE_AREA);
          CS_PNT->LASTCI = POS.POSA;
          CS_PNT->NEXTCI = SNEXT;
          CS_PNT->UPCI   = SUP;
          CS_PNT->LNCI   = BT_HEAD->SPLIT_DAT_2;
          CS_PNT->CITYPE = BT_BOTTOM;
          SPOSA  = POS.POSA;
          if (SNEXT != BT_NIL)
          {
              POS.POSA   = SNEXT;
              BT_GET_BLOCK();
              UMDB_WRITE_JOURNAL (TRUE);
              CI_PNT->LASTCI = NEWPOS;
          }
          POS.POSA   = SPOSA;
          BT_GET_BLOCK();
          UMDB_WRITE_JOURNAL (TRUE);
          CI_PNT->NEXTCI = NEWPOS;
          if (POS.POSB <= BT_HEAD->SPLIT_DAT_1)
          {
              SPOSA  = POS.POSA;
              SPOSB  = POS.POSB;
          }
          else
          {
              SPOSA  = NEWPOS;
              SPOSB  = POS.POSB - BT_HEAD->SPLIT_DAT_1;
          }
          POS.POSA   = NEWPOS;
          POS.POSB   = 1;
          BT_GET_BLOCK();
          PROPAGATE_SPLIT();
          POS.POSA   = SPOSA;
          POS.POSB   = SPOSB;
          BT_GET_BLOCK();
          goto S10;
      }

      return;
}

LPCB FREE_BT_REC (int LREC,long *LOC, int Type)
{ 	  long		I, LAST_BLOCK;
	  int		LAST, NEXT, LENS;
	  LPCB		pFREE_BT_REC;

      SAVE_POS = POS;
      if (Type != BT_BLOCK->Type) goto S100;

S5:   I = BT_BLOCK->BT_FSPACE_BEG;
      LAST_BLOCK = BT_BLOCK_NUM;
S10:      if (I < 0) goto S100;
          BT_FSPACE_PNT = (LPBTFSI)IADDR ((LPSTR)&BT_BLOCK->BT_DATA,I);
          if (BT_FSPACE_PNT->BT_FSPACE_LEN >= LREC)
          {
              pFREE_BT_REC = (LPCB)BT_FSPACE_PNT;
              LAST = BT_FSPACE_PNT->BT_FSPACE_LAST;
              NEXT = BT_FSPACE_PNT->BT_FSPACE_NEXT;
              LENS = BT_FSPACE_PNT->BT_FSPACE_LEN - LREC;
              if (!LAST)
                  BT_FSPACE_PNT = (LPBTFSI)&BT_BLOCK->BT_FSPACE_BEG;
              else
                  BT_FSPACE_PNT = (LPBTFSI)IADDR ((LPSTR)&BT_BLOCK->BT_DATA,LAST);
              UMDB_WRITE_JOURNAL (TRUE);
              if (LENS >= BT_BLOCK->RecLen)
              {
                  BT_FSPACE_PNT->BT_FSPACE_NEXT = I + LREC;
                  BT_FSPACE_PNT = (LPBTFSI)IADDR ((LPSTR)&BT_BLOCK->BT_DATA,BT_FSPACE_PNT->BT_FSPACE_NEXT);
                  BT_FSPACE_PNT->BT_FSPACE_NEXT = NEXT;
                  BT_FSPACE_PNT->BT_FSPACE_LAST = LAST;
                  BT_FSPACE_PNT->BT_FSPACE_LEN  = LENS;
               }
               else
               {
                  BT_FSPACE_PNT->BT_FSPACE_NEXT = NEXT;
                  if (NEXT > 0)
                  {
                      BT_FSPACE_PNT = (LPBTFSI)IADDR ((LPSTR)&BT_BLOCK->BT_DATA,NEXT);
                      BT_FSPACE_PNT->BT_FSPACE_LAST = LAST;
                  }
               }
               goto S200;
          }
          I = BT_FSPACE_PNT->BT_FSPACE_NEXT;
          goto S10;

S100: POS.POSA = (long)BT_HEAD->FirstFreeBlock[Type-1]*BT_HEAD->BT_BLKSIZE;
      BT_GET_BLOCK();
      if (BT_BLOCK->BT_FSPACE_BEG >= 0 && BT_BLOCK_NUM != LAST_BLOCK) goto S5;
      /* Create new block */
      BT_HEAD->BT_MAX_BLOCK++;
      POS.POSA = BT_HEAD->BT_MAX_BLOCK * BT_HEAD->BT_BLKSIZE;
      POS.POSB = 1;
      BT_GET_BLOCK();
      UMDB_WRITE_JOURNAL (TRUE);
      BT_BLOCK->BT_FSPACE_BEG = LREC + 1;
      BT_FSPACE_PNT = (LPBTFSI)IADDR ((LPSTR)&BT_BLOCK->BT_DATA,(long)BT_BLOCK->BT_FSPACE_BEG);
      BT_FSPACE_PNT->BT_FSPACE_LEN = BT_HEAD->BT_BLKSIZE - LREC - 6 -BT_HEAD->BT_BLOCK_HEAD_LEN;
      BT_FSPACE_PNT->BT_FSPACE_LAST = 0;
      BT_FSPACE_PNT->BT_FSPACE_NEXT = -1;
      pFREE_BT_REC =(LPCB) &BT_BLOCK->BT_DATA;
      BT_HEAD->BT_LENGTH = BT_BLOCK_NUM * BT_HEAD->BT_BLKSIZE +
      					   LREC + BT_HEAD->BT_BLOCK_HEAD_LEN + sizeof (BTFREESPACEITEM);
 	  BT_BLOCK->Dirty = TRUE;
      BT_BLOCK->NextFreeBlock = UINT_MAX;
      BT_BLOCK->LastFreeBlock = UINT_MAX;
      BT_BLOCK->Type = Type;
      if (Type == BT_UPPER)
      	BT_BLOCK->RecLen = BT_HEAD->CI_LENGTH_1;
      else
      	BT_BLOCK->RecLen = BT_HEAD->CI_LENGTH_2;
      BT_HEAD->FirstFreeBlock[Type-1]=BT_BLOCK_NUM;
      I = 1;
S200: *LOC = BT_BLOCK_NUM * BT_HEAD->BT_BLKSIZE + I - 1;
      if (*LOC >= BT_HEAD->BT_LENGTH) BT_HEAD->BT_LENGTH =
      		 *LOC + LREC + BT_HEAD->BT_BLOCK_HEAD_LEN;
      POS = SAVE_POS;
      return (pFREE_BT_REC);
}

void PROPAGATE_BT_UP (LPSTR KEY,LPSTR DATE)
{	  long I;
/*      CHARACTER KEY*256, SAVE_POS*6, DATE*2
      INTEGER*4 I
*/
      SAVE_POS = POS;
      pBTree->BT_PATH_CHANGE = TRUE;
S10:  if (CI_PNT->UPCI == BT_NIL) goto S20;
          I = POS.POSA;
          POS.POSA = CI_PNT->UPCI;
          BT_GET_BLOCK();
          while (INDEX_ITEM_PNT->BT_DOWN != I)
          {   INDEX_ITEM_PNT = (LPBTI)((LPSTR)INDEX_ITEM_PNT + BT_HEAD->BT_PNTLEN);
              POS.POSB += 1;
              if (POS.POSB > CI_PNT->LNCI)
				BTreeErrorMessage ();
          }
          UMDB_WRITE_JOURNAL (TRUE);
          _fmemmove (&INDEX_ITEM_PNT->INDEX_ENTRY,KEY,BT_HEAD->BT_KYLEN);
          if (BT_HEAD->BT_DATED)
     		  _fmemmove (IADDR(&INDEX_ITEM_PNT->INDEX_ENTRY,BT_HEAD->BT_KYLEN+1),
     		  			 DATE,2);
          if (POS.POSB != 1) goto S20;
          goto S10;
S20:  POS = SAVE_POS;
	  return;
}

void PROPAGATE_SPLIT()
{	long	I, SPLIT_ADD, NEW_POSA, SAVE_POSA;
	long	NEWPOS, SUP, SNEXT, SPOSA, LN;
	int		IB;
/*      CHARACTER KEY*256
      INTEGER*4 I, FREE_BT_REC
      INTEGER*4 LN, SPLIT_ADD, NEW_POS, SAVE_POS, NEWPOS, SUP, SNEXT,
     +          SPOSA
      INTEGER*2 IB, SPLIT_POS
*/
      pBTree->BT_PATH_CHANGE = TRUE;

S10:  if (CI_PNT->UPCI == BT_NIL)
	  {
          UMDB_WRITE_JOURNAL (TRUE);
          hSAVE_AREA = GSSiGlobAlloc (GMEM_MOVEABLE,BT_HEAD->BT_KEYLEN);
          SAVE_AREA = GlobalLock (hSAVE_AREA);
          _fmemmove (SAVE_AREA,
          			 (LPSTR)&INDEX_ITEM_PNT->INDEX_ENTRY,
          			 (UINT)BT_HEAD->BT_KEYLEN);
          CS_PNT = FREE_BT_REC (BT_HEAD->CI_LENGTH_1,&I,BT_UPPER);
          BT_HEAD->BT_FIRST_POS = I;
          CS_PNT->LASTCI = BT_NIL;
          CS_PNT->NEXTCI = BT_NIL;
          CS_PNT->UPCI   = BT_NIL;
          CS_PNT->LNCI   = 2;
          CS_PNT->CITYPE = BT_UPPER; 
          pCIDATA2 = &CS_PNT->CIDATA;
          if (BT_HEAD->TrackCount)
          	pCIDATA2 += 4;
          _fmemmove(IADDR(pCIDATA2,BT_HEAD->BT_PNTLEN+1),&POS.POSA,4);
          _fmemmove(IADDR(pCIDATA2,BT_HEAD->BT_PNTLEN+5),SAVE_AREA,BT_HEAD->BT_KEYLEN);
          BT_GET_BLOCK();
          UMDB_WRITE_JOURNAL (TRUE);
          CI_PNT->UPCI   = I;
          POS.POSA   = CI_PNT->LASTCI;
          SAVE_POSA  = POS.POSA;
          BT_GET_BLOCK();
          UMDB_WRITE_JOURNAL (TRUE);
          CI_PNT->UPCI   = I;
          _fmemmove(SAVE_AREA,&INDEX_ITEM_PNT->INDEX_ENTRY,BT_HEAD->BT_KEYLEN);
          POS.POSA   = I;
          BT_GET_BLOCK();
          UMDB_WRITE_JOURNAL (TRUE);
          _fmemmove(pCIDATA2,&SAVE_POSA,4);
          _fmemmove(IADDR(pCIDATA2,5),SAVE_AREA,BT_HEAD->BT_KEYLEN); 
          GSSiGlobUlFree (&hSAVE_AREA);
          
          BT_HEAD->BT_NUMLEVS = BT_HEAD->BT_NUMLEVS + 1;
          return;
      }

      NEW_POSA   = POS.POSA;
      SPLIT_ADD = CI_PNT->LASTCI;
      _fmemmove(&SAVEKEY,&INDEX_ITEM_PNT->INDEX_ENTRY,BT_HEAD->BT_KEYLEN);
      POS.POSA      = CI_PNT->UPCI;
      POS.POSB      = 1;
      BT_GET_BLOCK();

      while (SPLIT_ADD != INDEX_ITEM_PNT->BT_DOWN)
      {   INDEX_ITEM_PNT = (LPBTI)((LPSTR)INDEX_ITEM_PNT + BT_HEAD->BT_PNTLEN);
          POS.POSB = POS.POSB + 1;

              if (POS.POSB > CI_PNT->LNCI)
				BTreeErrorMessage ();
      }

      if (CI_PNT->LNCI < BT_HEAD->CI_MAX_PNUM)
      {
          UMDB_WRITE_JOURNAL (TRUE);
          IB = POS.POSB * BT_HEAD->BT_PNTLEN + 1;
          LN = (CI_PNT->LNCI - POS.POSB) * BT_HEAD->BT_PNTLEN;
S100:     pCIDATA2 = &CS_PNT->CIDATA;
          if (BT_HEAD->TrackCount)
          	pCIDATA2 += 4;

		  if (LN > 0) _fmemmove(IADDR(pCIDATA2,IB+BT_HEAD->BT_PNTLEN),
								IADDR(pCIDATA2,IB),(size_t)LN);
          CI_PNT->LNCI++;
          INDEX_ITEM_PNT = (LPBTI)((LPSTR)INDEX_ITEM_PNT + BT_HEAD->BT_PNTLEN);
          POS.POSB++;
          _fmemmove(&INDEX_ITEM_PNT->INDEX_ENTRY, &SAVEKEY,BT_HEAD->BT_KEYLEN);
          INDEX_ITEM_PNT->BT_DOWN = NEW_POSA;
          if (POS.POSB == 1)
              PROPAGATE_BT_UP ((LPSTR)&SAVEKEY,IADDR(SAVEKEY,BT_HEAD->BT_KYLEN+1));
          return;
      }
      else
      {
          if (POS.POSB == BT_HEAD->CI_MAX_PNUM && CI_PNT->NEXTCI != BT_NIL)
          {
              SAVE_POSA = POS.POSA;
              POS.POSA  = CI_PNT->NEXTCI;
              BT_GET_BLOCK();
              if (CI_PNT->LNCI < BT_HEAD->CI_MAX_PNUM)
              {
                  IB = 1;
                  LN = CI_PNT->LNCI * BT_HEAD->BT_PNTLEN;
                  SAVE_POSA = POS.POSA;
                  POS.POSA = NEW_POSA;
                  BT_GET_BLOCK();
                  UMDB_WRITE_JOURNAL (TRUE);
                  CI_PNT->UPCI = SAVE_POSA;
                  POS.POSA = SAVE_POSA;
                  POS.POSB = 0;
                  BT_GET_BLOCK();
                  UMDB_WRITE_JOURNAL (TRUE);
                  goto S100;
              }
              else
              {
                  POS.POSA = SAVE_POSA;
                  BT_GET_BLOCK();
              }
          }
          UMDB_WRITE_JOURNAL (TRUE);
          CI_PNT->LNCI   = BT_HEAD->SPLIT_PNT_1;
          IB     = BT_HEAD->SPLIT_PNT_1 * BT_HEAD->BT_PNTLEN + 1;
          LN     = BT_HEAD->SPLIT_PNT_2 * BT_HEAD->BT_PNTLEN;
          hSAVE_AREA = GSSiGlobAlloc (GMEM_MOVEABLE,BT_HEAD->CI_LENGTH_2);
          SAVE_AREA = GlobalLock (hSAVE_AREA);
          _fmemmove(SAVE_AREA,IADDR(pCIDATA,IB),(size_t)LN);
          SNEXT  = CI_PNT->NEXTCI;
          SUP    = CI_PNT->UPCI;
          CS_PNT = FREE_BT_REC (BT_HEAD->CI_LENGTH_1,&NEWPOS,BT_UPPER);
          pCIDATA2 = &CS_PNT->CIDATA;
          if (BT_HEAD->TrackCount)
          	pCIDATA2 += 4;
          _fmemmove(pCIDATA2,SAVE_AREA,(size_t)LN);
          GSSiGlobUlFree (&hSAVE_AREA);
          CS_PNT->LASTCI = POS.POSA;
          CS_PNT->NEXTCI = SNEXT;
          CS_PNT->UPCI   = SUP;
          CS_PNT->LNCI   = BT_HEAD->SPLIT_PNT_2;
          CS_PNT->CITYPE = BT_UPPER;
          SPOSA  = POS.POSA;
          if (SNEXT != BT_NIL)
          {
              POS.POSA   = SNEXT;
              BT_GET_BLOCK();
              UMDB_WRITE_JOURNAL (TRUE);
              CI_PNT->LASTCI = NEWPOS;
          }
          POS.POSA   = SPOSA;
          BT_GET_BLOCK();
          UMDB_WRITE_JOURNAL (TRUE);
          CI_PNT->NEXTCI = NEWPOS;
          if (POS.POSB > BT_HEAD->SPLIT_PNT_1)
          {
              POS.POSB = POS.POSB - BT_HEAD->SPLIT_PNT_1;
              POS.POSA = NEWPOS;
              BT_GET_BLOCK();
          }
          UMDB_WRITE_JOURNAL (TRUE);
          IB = POS.POSB * BT_HEAD->BT_PNTLEN + 1;
          LN = (CI_PNT->LNCI - POS.POSB) * BT_HEAD->BT_PNTLEN;
          if (LN > 0) _fmemmove(IADDR(pCIDATA,IB+BT_HEAD->BT_PNTLEN),
          						IADDR(pCIDATA,IB),(size_t)LN);
          CI_PNT->LNCI++;
          INDEX_ITEM_PNT =(LPBTI) ((LPSTR)INDEX_ITEM_PNT + BT_HEAD->BT_PNTLEN);
          POS.POSB++;
          _fmemmove(&INDEX_ITEM_PNT->INDEX_ENTRY,&SAVEKEY,BT_HEAD->BT_KEYLEN);
          INDEX_ITEM_PNT->BT_DOWN = NEW_POSA;
          POS.POSA    = NEWPOS;
          POS.POSB    = 1;
          BT_GET_BLOCK();
          PROPAGATE_BT_DOWN();
          POS.POSA    = NEWPOS;
          POS.POSB    = 1;
          NEW_POSA = NEWPOS;
          BT_GET_BLOCK();
          goto S10;
      }
}

void PROPAGATE_BT_DOWN()
{	long	J[32], I;
	int		L, N;

      I = POS.POSA;
      N = CI_PNT->LNCI;

      for (L = 0;L < N; L++)
      {
          J[L] = INDEX_ITEM_PNT->BT_DOWN;
          INDEX_ITEM_PNT = (LPBTI)((LPSTR)INDEX_ITEM_PNT + BT_HEAD->BT_PNTLEN);
      }

      for (L = 0;L < N; L++)
      {
          POS.POSA = J[L];
          BT_GET_BLOCK();
          UMDB_WRITE_JOURNAL (TRUE);
          CI_PNT->UPCI = I;
      }

}

/*void ShowBTree (HGLOBAL hBT,HWND hWnd,int DlgItem,int iPOSB)
{
	char	Text[256];
	HDC		hDC;
	long	ikey;
	int		TabStops[5]={30,60,90,120,150};
	typedef struct	{int	StreetNum;
					 long	MaxHouseNum;
					 long	Segid;
					 }	SegMaxKey;

	typedef SegMaxKey	FAR *LPSMK;
	LPSMK	lpSMK;


	hDC = GetDC(hWnd);
    AllocateBTMem(hBT);
	if (iPOSB ==-1)
	{
		POS.POSA = BT_HEAD->BT_FIRST_POS;
		POS.POSB = 1;
	}
	else if (iPOSB ==-2)
	{
		POS.POSA = CI_PNT->LASTCI;
		POS.POSB = 1;
	}
	else if (iPOSB ==-3)
	{
		POS.POSA = CI_PNT->NEXTCI;
		POS.POSB = 1;
	}
	else if (iPOSB ==-4)
	{
		POS.POSA = CI_PNT->UPCI;
		POS.POSB = 1;
	}
	else
	{
		POS.POSB = iPOSB;
		BT_GET_BLOCK();
		POS.POSA = INDEX_ITEM_PNT->BT_DOWN;
		POS.POSB = 1;
	}
	BT_GET_BLOCK();

	SAVE_POS = POS;

	wsprintf(Text, "POSA: %ld      ",POS.POSA);
	TextOut (hDC,5,5,Text,_fstrlen(Text));
	wsprintf(Text, "POSB: %d      ",POS.POSB);
	TextOut (hDC,5,25,Text,_fstrlen(Text));
	wsprintf(Text, "Num recs: %ld ",BT_HEAD->BT_NUMRECS);
	TextOut (hDC,5,45,Text,_fstrlen(Text));
	wsprintf(Text, "Num levels: %d ",BT_HEAD->BT_NUMLEVS);
	TextOut (hDC,5,65,Text,_fstrlen(Text));

	wsprintf(Text, "LNCI: %d           ",CI_PNT->LNCI);
	TextOut (hDC,155,10,Text,_fstrlen(Text));
	wsprintf(Text, "UPCI: %ld          ",CI_PNT->UPCI);
	TextOut (hDC,155,30,Text,_fstrlen(Text));
	wsprintf(Text, "LASTCI: %ld        ",CI_PNT->LASTCI);
	TextOut (hDC,155,50,Text,_fstrlen(Text));
	wsprintf(Text, "NEXTCI: %ld        ",CI_PNT->NEXTCI);
	TextOut (hDC,155,70,Text,_fstrlen(Text));
	wsprintf(Text, "CITYPE: %d         ",CI_PNT->CITYPE);
	TextOut (hDC,155,90,Text,_fstrlen(Text));

    SendDlgItemMessage (hWnd,DlgItem,LB_RESETCONTENT,NULL,NULL);
	SendDlgItemMessage (hWnd,DlgItem,LB_SETTABSTOPS,5,(LPARAM)&TabStops);

    for (POS.POSB=1;POS.POSB<CI_PNT->LNCI+1;POS.POSB++)
    {
    	BT_GET_BLOCK();
//    	ikey = *(LPLONG)&INDEX_ITEM_PNT->INDEX_ENTRY;
//        wsprintf(Text, "KEY: %ld  DOWN: %ld ",ikey,INDEX_ITEM_PNT->BT_DOWN);
    	lpSMK = (LPSMK)&INDEX_ITEM_PNT->INDEX_ENTRY;
        wsprintf(Text, "KEY: %d\t %ld\t %ld\t  \tDOWN: %ld ",lpSMK->StreetNum,lpSMK->MaxHouseNum,lpSMK->Segid,INDEX_ITEM_PNT->BT_DOWN);

    	SendDlgItemMessage (hWnd,DlgItem,LB_ADDSTRING,NULL,(DWORD)&Text);
    }
    POS = SAVE_POS;
    BT_GET_BLOCK();
	DeallocateBTMem(hBT);

	ReleaseDC(hWnd, hDC);
	return;
}
*/

int BT_DELETE (HGLOBAL hBTree,LPCSTR KEY,LPCSTR DATA,BOOL SECIDX)
{	  int irc;

	  if (!AllocateBTMem(hBTree)) return 1;
	  
	  
      irc = BT_DELETE_internal (KEY,DATA,SECIDX);
      DeallocateBTMem(hBTree);
      return (irc);
}

int BT_DELETE_internal(LPCSTR KEY,LPCSTR DATA,BOOL SECIDX) 
{
       
      int	ST, IB;
	  long	LAST, NEXT, SAVE_UP, LN;
	  BTPOS	SAVE_POS;
      BOOL	FRST; 
      
/*      INTEGER*4 ST, LN, NEXT, LAST, SAVE_UP
      INTEGER*2 IBTID, IB
      CHARACTER KEY*256, DATA*256, SAVE_POS*6, PROP_KEY*256
      LOGICAL FRST, SECIDX
 */   
      if (SECIDX) BT_UPDATE = TRUE;
      ST = BT_FIND_internal (KEY,BT_FIRST,BT_EQ,DATA,&FRST); 
      
      if (BT_HEAD->BT_DATED && ST == 0)
      {
		if (_fmemcmp(IADDR((LPSTR)&INDEX_ITEM_PNT->INDEX_ENTRY,BT_HEAD->BT_KYLEN+1),
                      &pBTree->BTID_DATE,2) != 0) ST = 1;
      }
      if (ST != 0)
      {
          pBTree->BTID_FIRST_CALL  = TRUE;
          goto S1000;
      }
      BT_HEAD->BT_NUMRECS = BT_HEAD->BT_NUMRECS - 1;
      if (POS.POSB > 1)
      {
          UMDB_WRITE_JOURNAL (TRUE);
          if (POS.POSB == CI_PNT->LNCI)
          {
              CI_PNT->LNCI = CI_PNT->LNCI - 1;
              POS.POSB = CI_PNT->LNCI;
              pBTree->BTID_CUR_POS = POS;
              goto S1000;
          }
          LN = (CI_PNT->LNCI - POS.POSB) * BT_HEAD->BT_RECLEN;
          IB = POS.POSB * BT_HEAD->BT_RECLEN + 1;
          _fmemmove(IADDR((LPSTR)pCIDATA,IB-BT_HEAD->BT_RECLEN),IADDR(pCIDATA,IB),LN);
          CI_PNT->LNCI = CI_PNT->LNCI - 1;
          POS.POSB = POS.POSB - 1;
          pBTree->BTID_CUR_POS = POS;
          goto S1000;
      }
//******* REMOVING FIRST ITEM IN BLOCK CONTAINING MORE THAN 1 ITEM;
      if (CI_PNT->LNCI > 1)
      {
	      LPSTR	PROP_KEY; 
	      HANDLE hPROP_KEY;
	      
	 	  hPROP_KEY = GSSiGlobAlloc(LMEM_MOVEABLE,256);
	 	  PROP_KEY = GlobalLock (hPROP_KEY);
          UMDB_WRITE_JOURNAL (TRUE);
          CI_PNT->LNCI = CI_PNT->LNCI - 1;
          LN = CI_PNT->LNCI * BT_HEAD->BT_RECLEN;
          IB = BT_HEAD->BT_RECLEN + 1;
          _fmemmove(pCIDATA,IADDR(pCIDATA,IB),LN);
          _fmemmove(PROP_KEY,&INDEX_ITEM_PNT->INDEX_ENTRY,BT_HEAD->BT_KEYLEN);
          SAVE_POS = POS;
          PROPAGATE_BT_UP (PROP_KEY,IADDR(PROP_KEY,BT_HEAD->BT_KYLEN+1));
		  GlobalUnlock(hPROP_KEY);
		  GlobalFree (hPROP_KEY);
          POS  = SAVE_POS;
          POS.POSB = POS.POSB - 1;
          pBTree->BTID_CUR_POS = POS;
          goto S1000;
      }
//******* REMOVING ONLY ITEM IN BLOCK;
      SAVE_POS = POS;
      NEXT     = CI_PNT->NEXTCI;
      LAST     = CI_PNT->LASTCI;
      if (LAST != BT_NIL)
      {
          POS.POSA = LAST;
          BT_GET_BLOCK();
          UMDB_WRITE_JOURNAL (TRUE);
          CI_PNT->NEXTCI = NEXT;
          POS.POSB = CI_PNT->LNCI;
          pBTree->BTID_CUR_POS = POS;
      }
      else
          pBTree->BTID_FIRST_CALL  = TRUE;
      if (NEXT != BT_NIL)
      {
          POS.POSA = NEXT;
          BT_GET_BLOCK();
          UMDB_WRITE_JOURNAL (TRUE);
          CI_PNT->LASTCI = LAST;
      }
      POS     = SAVE_POS;
      BT_GET_BLOCK();
      SAVE_UP = CI_PNT->UPCI;
      FREE_BT_REC_ADD (BT_HEAD->CI_LENGTH_2);
      POS     = SAVE_POS;
      PROPAGATE_DELETE_UP (SAVE_UP);
S1000:BT_UPDATE = FALSE; 
      return ST;
}

void PROPAGATE_DELETE_UP (long UP)
{     
	  
      long  LN, DELETE_POS, NEXT, LAST;
      int	IB;
      BOOL FRST; 
      LPSTR	KEY;
	  BTPOS	SAVE_POS;
      

      pBTree->BT_PATH_CHANGE = TRUE;

S10:  DELETE_POS = POS.POSA;
      if (UP == BT_NIL)
      {
          BT_HEAD->BT_LENGTH    = 0;
          BT_HEAD->BT_FIRST_POS = 0;
          BT_HEAD->BT_NUMLEVS = 0;
          /*chsize (pBTree->BtFid,BT_HEAD->BT_HEADLEN + BT_HEAD->BT_LENGTH + 32);*/
          return;
      }
      POS.POSA      = UP;
      POS.POSB      = 1;
      BT_GET_BLOCK();

      while (DELETE_POS != INDEX_ITEM_PNT->BT_DOWN)
      {
          INDEX_ITEM_PNT = (LPSTR) INDEX_ITEM_PNT + BT_HEAD->BT_PNTLEN;
          POS.POSB = POS.POSB + 1;
		  if (POS.POSB > CI_PNT->LNCI)
			BTreeErrorMessage ();
      }

      if (POS.POSB > 1)
      {
          UMDB_WRITE_JOURNAL (TRUE);
          if (POS.POSB == CI_PNT->LNCI)
          {
              CI_PNT->LNCI = CI_PNT->LNCI - 1;
              return;
          }
          LN = (CI_PNT->LNCI - POS.POSB) * BT_HEAD->BT_PNTLEN;
          IB = POS.POSB * BT_HEAD->BT_PNTLEN + 1;
          _fmemmove(IADDR(pCIDATA,IB-BT_HEAD->BT_PNTLEN),IADDR(pCIDATA,IB),LN);
          CI_PNT->LNCI = CI_PNT->LNCI - 1;
          return;
      }
      if (CI_PNT->LNCI > 1)
      {   LPSTR KEY;
      	  HANDLE hKEY;
      	  
      	  hKEY = GSSiGlobAlloc(LMEM_MOVEABLE,256);
      	  KEY = GlobalLock(hKEY);
          UMDB_WRITE_JOURNAL (TRUE);
          LN = CI_PNT->LNCI * BT_HEAD->BT_PNTLEN;
          IB = BT_HEAD->BT_PNTLEN + 1;
          _fmemmove(pCIDATA,IADDR(pCIDATA,IB),LN);
          CI_PNT->LNCI = CI_PNT->LNCI - 1;
          _fmemmove(KEY,&INDEX_ITEM_PNT->INDEX_ENTRY,BT_HEAD->BT_KEYLEN);
          PROPAGATE_BT_UP (KEY,IADDR(KEY,BT_HEAD->BT_KYLEN+1));
          GlobalUnlock(hKEY);
          GlobalFree(hKEY);
          return;
      }
      SAVE_POS = POS;
      NEXT     = CI_PNT->NEXTCI;
      LAST     = CI_PNT->LASTCI;
      if (LAST != BT_NIL)
      {
          POS.POSA = LAST;
          BT_GET_BLOCK();
          UMDB_WRITE_JOURNAL (TRUE);
          CI_PNT->NEXTCI = NEXT;
      }
      if (NEXT != BT_NIL)
      {
          POS.POSA = NEXT;
          BT_GET_BLOCK();
          UMDB_WRITE_JOURNAL (TRUE);
          CI_PNT->LASTCI = LAST;
      }
      POS = SAVE_POS;
      BT_GET_BLOCK();
      UP  = CI_PNT->UPCI;
      FREE_BT_REC_ADD (BT_HEAD->CI_LENGTH_1);
      POS = SAVE_POS;
      goto S10;
}

void FREE_BT_REC_ADD (int LENGTH)
{
      int NEXT;

      UMDB_WRITE_JOURNAL (TRUE);
      NEXT           = BT_BLOCK->BT_FSPACE_BEG;
      BT_BLOCK->BT_FSPACE_BEG  = BT_BLOCK_POS;
      BT_FSPACE_PNT  = IADDR ((LPSTR)&BT_BLOCK->BT_DATA,BT_BLOCK_POS);
      BT_FSPACE_PNT->BT_FSPACE_NEXT = NEXT;
      BT_FSPACE_PNT->BT_FSPACE_LAST = 0;
      BT_FSPACE_PNT->BT_FSPACE_LEN  = LENGTH;
      return;
}

BOOL ReorgBTree (LPSTR Name, HWND hStatusWnd)
{
    HANDLE	hBT, hBT2;
	BTHEAD BTHead; 
	long	TotRecs,Done;
	short	pos;
	LPSTR	pKey, pData, lpDot;
	HANDLE	hKey, hData;   
	BTVARDESC   BTVar[8],NewName[128];
     
	BT_SET_PARMS (8,16,7,15);
	hBT = BT_OPEN (Name, 0, BT_READ, 0); 
	if (!hBT) return FALSE;
	GetBTHeader (hBT,&BTHead);   
	GetBTVarDesc (hBT,BTVar);
	         		 
	_fstrcpy (NewName,Name); 
	lpDot = _fstrrchr (NewName,'.');
	*lpDot = 0;
	_fstrcat (NewName,".rbt");
	BT_CREATE (NewName, BTHead.BT_DATLEN, FALSE, BTHead.BT_NVARS, 1,(LPBTVARDESC) BTVar,FALSE, 0, 0, FALSE);
	hBT2 = BT_OPEN (NewName, 0, BT_WRITE, 0); 
	pos = BT_FIRST;
	Done = 0; 
	hKey  = GSSiGlobAlloc (GHND,1024);
	pKey = GlobalLock (hKey);
	hData  = GSSiGlobAlloc (GHND,1024);
	pData = GlobalLock (hData);    
	TotRecs = BTHead.BT_NUMRECS;
	while (!BT_FIND (hBT,pKey,pos,BT_ANY,pData))
	{  
	 	pos = BT_NEXT;
	 	BT_PUT (hBT2,pKey,pData); 
	 	PctBox (hStatusWnd,TotRecs, Done++,0);
	} 
	GSSiGlobUlFree (&hKey);
	GSSiGlobUlFree (&hData);
	BT_CLOSE (hBT); 
	BT_CLOSE (hBT2); 
	GSSiRemove (Name);
	GSSiRename (NewName,Name);
	return TRUE;
}