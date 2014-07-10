#include "shr.h"     
#include "BT.h"
typedef	struct	{long			bufid;
				 long			bufuse;
				} BTBUFINDEX;
typedef	BTBUFINDEX FAR *LPBTBUFINDEX;

typedef	struct	{short			usedbufs;
				 USHORT			curbuf;
				 long			nextuse;
				 BTBUFINDEX		index0;
				} BTBUFF2;
typedef	BTBUFF2	FAR *LPBTBUFF2;

#pragma pack(2)
typedef	struct	{short			maxbufs;
				 long			index_offset;
				 BYTE			FirstBuf;
				} BTBUFF1;
typedef BTBUFF1	FAR *LPBTBUFF1;

typedef struct {short 	BT_VERSION;
				unsigned	short
						BT_DATED:1,
						TrackCount:1,   
						LongFreeBlock:1,
						Filler:13;
				BOOL16	BT_BEING_UPDATED,
						BT_DUPS,		BT_JOURNAL,		HaveStrings,
						Dirty;
                long	BT_TIME_STAMP;
				long	BT_LENGTH,		BT_NUMRECS,		BT_MAX_BLOCK;
				long	BT_FIRST_POS,	BT_LAST_POS,	BT_DUPVAL,
						BT_DUPBEG,		BT_DUPEND,		BT_DUPINC;
				USHORT	FirstFreeBlock[2]; /*this limits size of db - should be a DWORD*/
				WORD	BT_BLKSIZE;
				short	CI_LENGTH_1,	CI_LENGTH_2,
						CI_MAX_DNUM,	CI_MAX_PNUM,	SPLIT_DAT_1,
                        SPLIT_DAT_2,	SPLIT_PNT_1,	SPLIT_PNT_2,
                        BT_NUMLEVS,		BT_DATLEN,		BT_KYLEN,
                        BT_PNTLEN,		BT_KEYLEN,		BT_HEADLEN,
                        BT_DUPPOS,      BT_RECLEN,      BT_BLOCK_HEAD_LEN,
                        BT_NVARS,       BT_NSPARSE_KEY;
                BTVARDESC				BT_VARDESC;
				char	filler[1024-104];
                } BTHEAD;
typedef	BTHEAD FAR	*LPBTHEAD;

#pragma pack()

typedef struct	{long			POSA;
				 short			POSB;
				} BTPOS;

typedef struct	{long			LASTCI, NEXTCI, UPCI;
				 short			LNCI, CITYPE;
				 char			CIDATA;
				} CONTROLBLOCK;
typedef CONTROLBLOCK FAR *LPCB;

typedef	struct	{USHORT			LastFreeBlock,  /* should be DWORD*/
								NextFreeBlock;
				 short			Type;
				 short			RecLen;
				 short			BT_FSPACE_BEG;
				 BOOL16			Dirty;
				 char			BT_DATA;
				} BTREEPHYSREC;
typedef	BTREEPHYSREC FAR	*LPBTBLOCK;


typedef struct	{long		BT_DOWN;
				 char			INDEX_ENTRY;
				} BTreeItem;
typedef	BTreeItem FAR	*LPBTI;
typedef struct {short 			BT_FSPACE_NEXT, BT_FSPACE_LEN, BT_FSPACE_LAST;
			   } BTFREESPACEITEM;
typedef	BTFREESPACEITEM	FAR	*LPBTFSI;

typedef struct	{HGLOBAL		hBT_BLOCK;
				 HFILE			BtFid;
				 HWND			MessageWindow;
				 short			BTID_DATE;
				 short			BTLEV;
				 BTPOS			BTID_CUR_POS;
				 BOOL			BTID_FIRST_CALL, BT_PATH_CHANGE, BTID_READ;
				 long			BTID_CUR_BLK;
				 LPVOID			BTID_CUR_BLK_PNT;
				 BTPOS			BT_PATH_POS[MAX_BTREE_LEVELS];
				 char			BT_FNAME[256];
				 char			BTID_LASKEY; 
				 BTHEAD			BT_HEAD;
				 LPBTBUFF1		BT_BUFFERS;
				 LPBTBUFINDEX	BT_BUFF_CURINDEX;
				 LPBTBUFF2		BT_BUFF_INDEX;
				 LPBTI			INDEX_ITEM_PNT;
				 LPSTR			INDEX_DATA_PNT;
				 LPBTFSI		BT_FSPACE_PNT;
				 BOOL			READ_SECIDX;
				 BOOL			BT_UPDATE; 
				 LPCB			CI_PNT, CS_PNT;
				 BOOL			BT_END;
				 LPLONG			pBlockCount;
				 LPBTBLOCK		BT_BLOCK;
				 BTPOS			POS, LAST_POS, SAVE_POS;
				 long			BT_BLOCK_NUM, BT_BLOCK_POS;
				 /* special keys fit at end of this struct
				 	all are BT_HEAD->BT_KEYLEN in length

				    BTID_LASKEY;
				    BTID_MAXKEY;
				    BT_PATH_KEY_BEG[16];
				    BT_PATH_KEY_END[16];
				 */
				} BTREE;
typedef	BTREE	FAR	*LPBTREE;


static	char		SmallName[MAX_PATH]="";
static	char		DATA2[1024];
static	BOOL		TIME_CHECK=TRUE;
static	BOOL		SET_PARMS=FALSE;
static	short		MPN, MDN, SPN, SDN;
static	short		bt_get_date;
static	char		SAVEKEY[260];
static	short		Version=1;  
#include "gmextern.h"

BOOL GetBTHeader (HGLOBAL IBTID,LPBTHEAD BTHead);
void BT_INSERT (LPBTREE pBTree,LPSTR KEY,LPSTR DATA, BOOL LASTRC);
int BT_KEY_COMPARE (LPBTREE pBTree,LPSTR KEY1,LPSTR KEY2,int ITYPE);
LPSTR BT_MAXKEY(LPBTREE pBTree);
void SetBTMaxKey (LPBTREE pBTree,LPSTR Maxkey);
void SetBTMinKey (LPBTREE pBTree,LPSTR Minkey);
int BT_PUT_internal (LPBTREE pBTree,LPSTR KEY,LPSTR DATA, BOOL LRC, BOOL DO_FIND, int BTST);
BOOL GetBTVarDesc (HGLOBAL IBTID,LPBTVARDESC BTVar);
LPSTR BT_PATH_KEY_BEG(LPBTREE pBTree,int BTLEV);
LPSTR BT_PATH_KEY_END(LPBTREE pBTree,int BTLEV);
void FIND_NEXT_BTPOS(LPBTREE pBTree);
void BT_GET_BLOCK(LPBTREE pBTree);
void FIND_PRIOR_BTPOS(LPBTREE pBTree);
void UMDB_WRITE_JOURNAL (LPBTREE pBTree,BOOL INDEX);
LPBTREE  AllocateBTMem (HGLOBAL hBTree);
void DeallocateBTMem(LPBTREE pBTree,HGLOBAL hBTree);
BOOL BT_ALLOCATE_BUFFERS (HGLOBAL hBTree);
LPCB FREE_BT_REC (LPBTREE pBTree,int LREC,long *LOC, int Type);
void FREE_BT_REC_ADD (LPBTREE pBTree,int LENGTH);
void PROPAGATE_BT_UP (LPBTREE pBTree,LPSTR KEY,LPSTR DATE);
void PROPAGATE_BT_DOWN(LPBTREE pBTree);
void PROPAGATE_SPLIT(LPBTREE pBTree);
void PROPAGATE_DELETE_UP (LPBTREE pBTree,long UP);
int BT_DELETE_internal(LPBTREE pBTree,LPSTR KEY,LPSTR DATA,BOOL SECIDX);
void BTreeErrorMessage (LPBTREE pBTree,LPSTR Mess);
void btwrite (HFILE BtFid,LPSTR pData,UINT len, long seekloc);
int BT_FIND_internal (LPBTREE pBTREE,LPSTR KEY,int POSITION,int COND,LPSTR DATA,LPBOOL LASTRC);
 

void SetBT_HEAD_FirstFreeBlock (LPBTREE pBTree,UINT Type,long Loc) 
#if ENABLETRACE
{GSSiEnterProg (457);
#endif
{   
	LPLONG	pFFB;
	
	if (pBTree->BT_HEAD.LongFreeBlock)
	{   
		pFFB = (LPLONG)((LPSTR)&pBTree->BT_HEAD.BT_VARDESC + pBTree->BT_HEAD.BT_NVARS * sizeof (BTVARDESC)); 
		pFFB += Type;
		*pFFB = Loc;
    } 
    else
		pBTree->BT_HEAD.FirstFreeBlock[Type] = (UINT)Loc;
{
#if ENABLETRACE
GSSiExitProg (457);
#endif
    return;
}
#if ENABLETRACE
}
#endif
} 

long GetBT_HEAD_FirstFreeBlock (LPBTREE pBTree,UINT Type)
#if ENABLETRACE
{GSSiEnterProg (458);
#endif
{   
	long	FFB;
	LPLONG	pFFB;
	
	if (pBTree->BT_HEAD.LongFreeBlock)
	{
		pFFB = (LPLONG)((LPSTR)&pBTree->BT_HEAD.BT_VARDESC + pBTree->BT_HEAD.BT_NVARS * sizeof (BTVARDESC)); 
		pFFB += Type;
		FFB = *pFFB;
	}
	else
		FFB = pBTree->BT_HEAD.FirstFreeBlock[Type];
{
#if ENABLETRACE
GSSiExitProg (458);
#endif
	return FFB;
}
#if ENABLETRACE
}
#endif
}  

void btwrite (HFILE BtFid,LPSTR pData,UINT len, long seekloc)
#if ENABLETRACE
{GSSiEnterProg (459);
#endif
{
    UINT	nwritten;
    
	BigWrite (BtFid,pData,len,-1);
{
#if ENABLETRACE
GSSiExitProg (459);
#endif
		return;
}   
#if ENABLETRACE
}
#endif
}

void SetReadSecIndex (HGLOBAL IBTID,BOOL val)
#if ENABLETRACE
{GSSiEnterProg (460);
#endif
{   
	LPBTREE	pBTree;

    if (IBTID)
	{
		pBTree = (LPBTREE)GlobalLock(IBTID);
		pBTree->READ_SECIDX = val; 
		GlobalUnlock (IBTID);
	}
{
#if ENABLETRACE
GSSiExitProg (460);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}
 
BOOL GetBTHeader (HGLOBAL IBTID,LPBTHEAD BTHead)
#if ENABLETRACE
{GSSiEnterProg (461);
#endif
{
	LPBTREE	pBTree;

    if (!IBTID)
{
#if ENABLETRACE
GSSiExitProg (461);
#endif
    	return (FALSE);
}
    if (!(pBTree = AllocateBTMem(IBTID)))
{
#if ENABLETRACE
GSSiExitProg (461);
#endif
    	return FALSE;
}
	*BTHead = pBTree->BT_HEAD;
    DeallocateBTMem(pBTree,IBTID); 
{
#if ENABLETRACE
GSSiExitProg (461);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

HFILE GetBTFid (HGLOBAL IBTID)
{
	LPBTREE	pBTree;
	HFILE	rtn=HFILE_ERROR;

    if (IBTID)
	{
		pBTree = (LPBTREE)GlobalLock(IBTID);
		rtn = pBTree->BtFid; 
		GlobalUnlock (IBTID);
	}
	return rtn;
} 

BOOL GetBTVarDesc (HGLOBAL IBTID,LPBTVARDESC BTVar)
#if ENABLETRACE
{GSSiEnterProg (462);
#endif
{
	LPBTVARDESC BTVarFrom; 
	LPBTREE	pBTree;

	short	n;   
	
    if (!IBTID)
{
#if ENABLETRACE
GSSiExitProg (462);
#endif
    	return (FALSE);
}
    if (!(pBTree = AllocateBTMem(IBTID)))
{
#if ENABLETRACE
GSSiExitProg (462);
#endif
    	return FALSE;
}
    BTVarFrom = &pBTree->BT_HEAD.BT_VARDESC; 
    n = pBTree->BT_HEAD.BT_NVARS; 
    while (n--)
    	*BTVar++ = *BTVarFrom++;
    DeallocateBTMem(pBTree,IBTID); 
{
#if ENABLETRACE
GSSiExitProg (462);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

short GetBTKeyLen (HGLOBAL IBTID)
#if ENABLETRACE
{GSSiEnterProg (463);
#endif
{   
	short	len;
	LPBTREE	pBTree;
	
    if (!IBTID)
{
#if ENABLETRACE
GSSiExitProg (463);
#endif
    	return 0;
}
    if (!(pBTree = AllocateBTMem(IBTID)))
{
#if ENABLETRACE
GSSiExitProg (463);
#endif
    	return FALSE;
}
    len = pBTree->BT_HEAD.BT_KEYLEN;
    DeallocateBTMem(pBTree,IBTID); 
{
#if ENABLETRACE
GSSiExitProg (463);
#endif
    return len;
}
#if ENABLETRACE
}
#endif
} 

short GetBTDataLen (HGLOBAL IBTID)
#if ENABLETRACE
{GSSiEnterProg (464);
#endif
{   
	short	len;
	LPBTREE	pBTree;
	
    if (!IBTID)
{
#if ENABLETRACE
GSSiExitProg (464);
#endif
    	return 0;
}
    if (!(pBTree = AllocateBTMem(IBTID)))
{
#if ENABLETRACE
GSSiExitProg (464);
#endif
    	return FALSE;
}
    len = pBTree->BT_HEAD.BT_DATLEN;
    DeallocateBTMem(pBTree,IBTID); 
{
#if ENABLETRACE
GSSiExitProg (464);
#endif
    return len;
}
#if ENABLETRACE
}
#endif
} 

void BT_SET_VERSION (int V)
#if ENABLETRACE
{GSSiEnterProg (465);
#endif
{
	Version = V;
{
#if ENABLETRACE
GSSiExitProg (465);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

int BT_GET_VERSION (HGLOBAL IBTID)
#if ENABLETRACE
{GSSiEnterProg (466);
#endif
{	int	irc;
	LPBTREE	pBTree;

    if (!IBTID)
{
#if ENABLETRACE
GSSiExitProg (466);
#endif
    	return (-1);
}
    if (!(pBTree = AllocateBTMem(IBTID)))
{
#if ENABLETRACE
GSSiExitProg (466);
#endif
    	return -1;
}
    irc = pBTree->BT_HEAD.BT_VERSION;
    DeallocateBTMem(pBTree,IBTID); 
{
#if ENABLETRACE
GSSiExitProg (466);
#endif
    return (irc);
}
#if ENABLETRACE
}
#endif
}

BOOL BT_OPEN_FOR_WRITE (HGLOBAL IBTID)
#if ENABLETRACE
{GSSiEnterProg (466);
#endif
{	BOOL	irc;
	LPBTREE	pBTree;

    if (!IBTID)
{
#if ENABLETRACE
GSSiExitProg (466);
#endif
    	return (-1);
}
    if (!(pBTree = AllocateBTMem(IBTID)))
{
#if ENABLETRACE
GSSiExitProg (466);
#endif
    	return -1;
}
    irc = pBTree->BT_HEAD.BT_BEING_UPDATED;
    DeallocateBTMem(pBTree,IBTID); 
{
#if ENABLETRACE
GSSiExitProg (466);
#endif
    return (irc);
}
#if ENABLETRACE
}
#endif
}

BOOL BT_CREATE (LPSTR FNAME, int DATLEN, BOOL DATED, int NVARIn, int NVAR2,
        	   LPBTVARDESC	pVarDesc, BOOL DUPS, int DUPPOS,
		 	   time_t TIME, BOOL JOURNAL)
#if ENABLETRACE
{GSSiEnterProg (467);
#endif
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
	  OFSTRUCTGM	OFStruct;
      long		keylen; 
      BOOL		rtn;    
      int		NVAR = abs (NVARIn);
      int		i, IOFF;
	  LPBTREE	pBTree;
	  LPBTVARDESC	pBtVarDesc;
     
//      BT_UPDATE   = FALSE;
//      READ_SECIDX = FALSE;
      for (i=0,pBtVarDesc=pVarDesc,keylen=0;i<NVAR;pBtVarDesc++,i++)
          keylen += pBtVarDesc->BT_VARLEN;
      hBTree = GSSiGlobAlloc ( 100,GHND,sizeof(BTREE) + (MAX_BTREE_LEVELS * 2 + 2) * keylen);
      pBTree = (LPBTREE)GlobalLock(hBTree);
      //pBTree->hBT_HEAD = GSSiGlobAlloc ( 101,GHND,1024);
      //BT_HEAD = (LPBTHEAD)GlobalLock(pBTree->hBT_HEAD);
      pBTree->BT_HEAD.BT_HEADLEN = /*sizeof(BTHEAD)+(NVAR-1)*sizeof(BTVARDESC);*/ 1024;
      /*BT_HEAD = MS_$CRMAPL (FNAME,FNLEN,32,1024,MS_$COWRITERS,ST)
      IF (ST .NE. 0) GO TO 1500*/
      pBTree->BT_HEAD.BT_VERSION	= Version; 
      pBTree->BT_HEAD.LongFreeBlock = TRUE;
      pBTree->BT_HEAD.BT_LENGTH    = 0;
      pBTree->BT_HEAD.BT_FIRST_POS = 0;
      pBTree->BT_HEAD.BT_KEYLEN    = 0;  
      if (DUPPOS)
	  	pBTree->BT_HEAD.BT_DUPPOS    = DUPPOS;
	  else
      	pBTree->BT_HEAD.BT_DUPPOS    = NVAR;
      pBTree->BT_HEAD.BT_KEYLEN	= (short)keylen;
      pBTree->BT_HEAD.BT_KYLEN     = pBTree->BT_HEAD.BT_KEYLEN;
      if (pBTree->BT_HEAD.BT_DATED) pBTree->BT_HEAD.BT_KEYLEN += 2;
      pBTree->BT_HEAD.BT_DATLEN    = DATLEN;
      pBTree->BT_HEAD.BT_PNTLEN    = pBTree->BT_HEAD.BT_KEYLEN + 4;
      if (NVARIn < 0)
      {  
      	pBTree->BT_HEAD.TrackCount = TRUE;
      	pBTree->BT_HEAD.BT_PNTLEN += 4;
      }	
      pBTree->BT_HEAD.BT_PNTLEN += pBTree->BT_HEAD.BT_PNTLEN%2;
      pBTree->BT_HEAD.BT_RECLEN    = pBTree->BT_HEAD.BT_KEYLEN + pBTree->BT_HEAD.BT_DATLEN;
      pBTree->BT_HEAD.BT_RECLEN += pBTree->BT_HEAD.BT_RECLEN%2;
      if (SET_PARMS)
      {		pBTree->BT_HEAD.CI_MAX_PNUM  = MPN;
            pBTree->BT_HEAD.CI_MAX_DNUM  = MDN;
            pBTree->BT_HEAD.SPLIT_PNT_1  = SPN;
            pBTree->BT_HEAD.SPLIT_PNT_2  = MPN-SPN;
            pBTree->BT_HEAD.SPLIT_DAT_1  = SDN;
            pBTree->BT_HEAD.SPLIT_DAT_2  = MDN-SDN;
      }
      else
      {     pBTree->BT_HEAD.CI_MAX_PNUM  = 8;
            pBTree->BT_HEAD.CI_MAX_DNUM  = 16;
            pBTree->BT_HEAD.SPLIT_PNT_1  = 5;
            pBTree->BT_HEAD.SPLIT_PNT_2  = 3;
            pBTree->BT_HEAD.SPLIT_DAT_1  = 10;
            pBTree->BT_HEAD.SPLIT_DAT_2  = 6;
      }
      SET_PARMS = FALSE;
      pBTree->BT_HEAD.BT_BLKSIZE   = 4096; 
      if (DATLEN > 246)
	      pBTree->BT_HEAD.BT_BLKSIZE = (short)(sizeof (BTREEPHYSREC) + max (pBTree->BT_HEAD.CI_MAX_PNUM * (long)pBTree->BT_HEAD.BT_PNTLEN, 
	      													 pBTree->BT_HEAD.CI_MAX_DNUM * (long)pBTree->BT_HEAD.BT_RECLEN) + 32);
	  pBTree->BT_HEAD.BT_BLKSIZE += pBTree->BT_HEAD.BT_BLKSIZE % 2;
      pBTree->BT_HEAD.BT_DATED     = DATED;
      pBTree->BT_HEAD.BT_JOURNAL   = JOURNAL;
      pBTree->BT_HEAD.BT_NUMRECS   = 0;
      pBTree->BT_HEAD.BT_MAX_BLOCK = 0;
      pBTree->BT_HEAD.BT_BLOCK_HEAD_LEN = 12; /* LasttFreeBlock,NextFreeBlock;	BT_FSPACE_BEG; Dirty;*/
      pBTree->BT_HEAD.BT_DUPS      = DUPS;
      pBTree->BT_HEAD.BT_NSPARSE_KEY = 0;
      pBTree->BT_HEAD.BT_TIME_STAMP= TIME;
      pBTree->BT_HEAD.CI_LENGTH_1  = 16 + pBTree->BT_HEAD.CI_MAX_PNUM * pBTree->BT_HEAD.BT_PNTLEN;
      pBTree->BT_HEAD.CI_LENGTH_2  = 16 + pBTree->BT_HEAD.CI_MAX_DNUM * pBTree->BT_HEAD.BT_RECLEN;
      pBTree->BT_HEAD.BT_NVARS = NVAR;
      for (i=0,pBtVarDesc=&pBTree->BT_HEAD.BT_VARDESC,IOFF=0;i<NVAR;pBtVarDesc++,pVarDesc++,i++)
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
      pBTree->BT_HEAD.BT_BEING_UPDATED = FALSE;
      pBTree->BtFid = GSSiOpenFile (FNAME,&OFStruct,OF_CREATE); 
      if (pBTree->BtFid == HFILE_ERROR)
		rtn=FALSE;
      else
      {
		btwrite (pBTree->BtFid,(LPSTR)&pBTree->BT_HEAD,(long)1024,0);//sizeof(BTHEAD)
		GSSiClose (pBTree->BtFid); 
		pBTree->BtFid = HFILE_ERROR;
		rtn=TRUE;   
	  }
 //     GSSiGlobUlFree (&pBTree->hBT_HEAD);
      GSSiGlobUlFree (&hBTree);
{
#if ENABLETRACE
GSSiExitProg (467);
#endif
	  return(rtn);
}
#if ENABLETRACE
}
#endif
}

/*void BT_GET_DEF (HGLOBAL IBTID,LPSHORT DataLength,LPSHORT NumVar,LPBTVARDESC pVarDesc)
#if ENABLETRACE
{GSSiEnterProg (468);
#endif
{   LPBTVARDESC pBtVarDesc;

    if (!AllocateBTMem(IBTID))
{
#if ENABLETRACE
GSSiExitProg (468);
#endif
    	return;
}
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
{
#if ENABLETRACE
GSSiExitProg (468);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}*/



HGLOBAL BT_OPEN (LPSTR FNAME, time_t TIME, int ACCESS, int DATE)
#if ENABLETRACE
{GSSiEnterProg (469);
#endif
{
	  HGLOBAL 	hBTree=0;
	  LPBTREE	pBTree;
	  long		flen,ii;
	  HGLOBAL	hBTHEADER=0;
	  HFILE		Fid;
	  OFSTRUCTGM	OFStruct;
      unsigned frequency=1000, duration=100; 
	  LPBTHEAD	BT_HEAD;
      //char		str[256];

      //BT_UPDATE   = TRUE;
      //READ_SECIDX = FALSE;

      hBTHEADER = GSSiGlobAlloc ( 102,GHND,1024);
      BT_HEAD = (LPBTHEAD)GlobalLock(hBTHEADER);
	  if (!ACCESS)
	      Fid = GSSiOpenFile (FNAME,(LPOFSTRUCTGM) &OFStruct,OF_READ);
      else
		  Fid = GSSiOpenFile(FNAME, (LPOFSTRUCTGM)&OFStruct, OF_READWRITE);
	
	  if (Fid == HFILE_ERROR)
	  {   
		 if (OFStruct.nErrCode>3)
		 {  
		 	HANDLE	hMess=GSSiGlobAlloc ( 103,GMEM_MOVEABLE,512);
		 	LPSTR	pMess=GlobalLock (hMess);
			sprintf (pMess,"Error number %i on open (mode %i) in BT_OPEN",
					 (int)OFStruct.nErrCode,ACCESS);
			GSSiMsgBox( 0, FNAME,pMess, MB_OK|MB_ICONQUESTION|MB_APPLMODAL,0); 
			GSSiGlobUlFree (&hMess);
		 } 
     Error:
     	 GSSiGlobUlFree (&hBTHEADER);
         if (Fid != HFILE_ERROR)
         	GSSiClose(Fid);
         hBTree = NULL;
         goto Exit;
	  }
      
      flen = GSSifilelength (Fid);
      if (flen < 1024)
      	goto Error;
      ii = BigRead (Fid,(LPSTR)BT_HEAD,1024); 
      BT_HEAD->BT_HEADLEN =1024;
      hBTree = GSSiGlobAlloc ( 104,GHND,sizeof(BTREE) +
      				       (MAX_BTREE_LEVELS * 2 + 2) * BT_HEAD->BT_KEYLEN);
      pBTree = (LPBTREE)GlobalLock(hBTree);
	  pBTree->BT_HEAD = *BT_HEAD;
//      pBTree->hBT_HEAD = hBTHEADER;
      pBTree->BtFid = Fid;
      
      _fullpath (pBTree->BT_FNAME,OFStruct.szPathName,256);
      pBTree->BTID_DATE = DATE;
      if (ACCESS ==BT_WRITE)
           pBTree->BTID_READ = FALSE;
      else
           pBTree->BTID_READ = TRUE;

      if (!BT_ALLOCATE_BUFFERS (hBTree))
      	goto Error;


      pBTree->BTID_FIRST_CALL = TRUE;
      pBTree->BT_PATH_CHANGE  = TRUE;
	  SetBTMaxKey (pBTree,BT_MAXKEY(pBTree));
	  pBTree->BTID_CUR_BLK    = 0;
      pBTree->BTID_CUR_BLK_PNT= pBTree->BT_BLOCK;
      GSSillseek (pBTree->BtFid,1024,0);
      BigRead (pBTree->BtFid,(HPSTR)pBTree->BT_BLOCK,pBTree->BT_HEAD.BT_BLKSIZE);

      if (!pBTree->BTID_READ)
		  pBTree->BT_HEAD.BT_BEING_UPDATED = TRUE;
      DeallocateBTMem(pBTree,hBTree);

 Exit:
//	  sprintf (str,"Open BTree: %s  ID: %ld",FNAME,(long)hBTree);
//	  GSSiTrace(str);
	  GSSiGlobUlFree (&hBTHEADER);

{
#if ENABLETRACE
GSSiExitProg (469);
#endif
      return (hBTree);
}
#if ENABLETRACE
}
#endif
}
BOOL BT_ALLOCATE_BUFFERS (HGLOBAL hBTree)
#if ENABLETRACE
{GSSiEnterProg (470);
#endif
{
	LPBTREE	pBTree = GlobalLock (hBTree);
	long	NumBTBuffers=14;
	  
	  if (pBTree->BT_HEAD.BT_BLKSIZE)
	  {
		  NumBTBuffers = 64000L / pBTree->BT_HEAD.BT_BLKSIZE;
		  pBTree->hBT_BLOCK = GSSiGlobAlloc ( 105,GHND,
      			   2 + 4 + (long)pBTree->BT_HEAD.BT_BLKSIZE * NumBTBuffers + 2 + 2 + 4 + sizeof (BTBUFINDEX) * NumBTBuffers);




		  pBTree->BT_BUFFERS =(LPBTBUFF1) GlobalLock(pBTree->hBT_BLOCK);
		  pBTree->BT_BUFFERS->maxbufs = (short)NumBTBuffers;
		  pBTree->BT_BUFFERS->index_offset = 2 + 4 + pBTree->BT_BUFFERS->maxbufs * (long)pBTree->BT_HEAD.BT_BLKSIZE;
		  pBTree->BT_BUFF_INDEX = (LPBTBUFF2)((LPSTR) pBTree->BT_BUFFERS + pBTree->BT_BUFFERS->index_offset);
		  pBTree->BT_BUFF_INDEX->usedbufs = 1;
		  pBTree->BT_BUFF_INDEX->nextuse = LONG_MIN;
		  pBTree->BT_BUFF_INDEX->curbuf = 0;
		  pBTree->BT_BUFF_CURINDEX = &pBTree->BT_BUFF_INDEX->index0;
		  pBTree->BT_BUFF_CURINDEX->bufid = 0;
		  pBTree->BT_BUFF_CURINDEX->bufuse = LONG_MIN;
		  pBTree->BT_BLOCK = (LPBTBLOCK)&pBTree->BT_BUFFERS->FirstBuf;
		  GlobalUnlock (hBTree);
	  }
{
#if ENABLETRACE
GSSiExitProg (470);
#endif
      return (TRUE);
}
#if ENABLETRACE
}
#endif
}

LPBTREE AllocateBTMem (HGLOBAL hBTree)
#if ENABLETRACE
{GSSiEnterProg (471);
#endif
{
	LPBTREE	pBTree;

	if (!hBTree)
{
#if ENABLETRACE
GSSiExitProg (471);
#endif
      	return(0);
}     
     pBTree = (LPBTREE)GlobalLock(hBTree);
     if (!pBTree)
{
#if ENABLETRACE
GSSiExitProg (471);
#endif
      	return(0);
}     
	 // pBTree->SaveBTHEAD = BT_HEAD; 
	  //pBTree->SaveBTree = (LPVOID)SaveBTree;
	 // pBTree->SaveBT_BUFFERS = BT_BUFFERS;
	 // pBTree->SaveBT_BUFF_INDEX = BT_BUFF_INDEX;
      //BT_HEAD = (LPBTHEAD)GlobalLock(pBTree->hBT_HEAD);   
/*      if (!BT_HEAD)
{
#if ENABLETRACE
GSSiExitProg (471);
#endif
      	return (0);
}*/
      pBTree->BT_BUFFERS = (LPBTBUFF1)GlobalLock(pBTree->hBT_BLOCK);
      pBTree->BT_BUFF_INDEX = (LPBTBUFF2)((LPSTR) pBTree->BT_BUFFERS + pBTree->BT_BUFFERS->index_offset);
      pBTree->BT_BLOCK = (LPBTBLOCK)((LPSTR)(&pBTree->BT_BUFFERS->FirstBuf) + pBTree->BT_BUFF_INDEX->curbuf * (long)pBTree->BT_HEAD.BT_BLKSIZE);
      pBTree->BTID_CUR_BLK_PNT= pBTree->BT_BLOCK; 
{
#if ENABLETRACE
GSSiExitProg (471);
#endif
      return pBTree;
}
#if ENABLETRACE
}
#endif
}

void DeallocateBTMem(LPBTREE pBTree, HGLOBAL hBTree)
#if ENABLETRACE
{GSSiEnterProg (472);
#endif
{     
	if (pBTree)
	{
      //GlobalUnlock(pBTree->hBT_HEAD);
      GlobalUnlock(pBTree->hBT_BLOCK); 
//      BT_HEAD = pBTree->SaveBTHEAD; 
//	  BT_BUFFERS = pBTree->SaveBT_BUFFERS;
//	  BT_BUFF_INDEX = pBTree->SaveBT_BUFF_INDEX;
//      pBTree = (LPBTREE)pBTree->SaveBTree;
      GlobalUnlock(hBTree);
	}
{
#if ENABLETRACE
GSSiExitProg (472);
#endif
      return;
}
#if ENABLETRACE
}
#endif
}


void	SetBTMaxKey (LPBTREE pBTree,LPSTR Maxkey)
#if ENABLETRACE
{GSSiEnterProg (473);
#endif
{
	BTVARDESC *pVarDesc;
	int i, IOFF;
	LPSTR	pMKEY;

	pMKEY = Maxkey;

      for (i=0,pVarDesc=&pBTree->BT_HEAD.BT_VARDESC,IOFF=0;
       	   i<pBTree->BT_HEAD.BT_NVARS;pVarDesc++,i++)
      	{	if (pVarDesc->BT_VARTYP == BT_CHAR || pVarDesc->BT_VARTYP == BT_RIGHT_CHAR)
                _fmemset (pMKEY,UCHAR_MAX,pVarDesc->BT_VARLEN);
            else if(pVarDesc->BT_VARTYP == BT_REAL4)
                *(LPFLOAT)pMKEY = FLT_MAX;
            else if (pVarDesc->BT_VARTYP == BT_REAL8)
                *(LPDOUBLE)pMKEY = DBL_MAX;
            else if (pVarDesc->BT_VARTYP == BT_INT2)
                *(LPSHORT)pMKEY = SHRT_MAX;
            else if (pVarDesc->BT_VARTYP == BT_INT4)
                *(LPLONG)pMKEY = LONG_MAX;
            pMKEY += pVarDesc->BT_VARLEN;
      	}
{
#if ENABLETRACE
GSSiExitProg (473);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void	SetBTMinKey (LPBTREE pBTree,LPSTR Maxkey)
#if ENABLETRACE
{GSSiEnterProg (474);
#endif
{
	BTVARDESC *pVarDesc;
	int i, IOFF;
	LPSTR	pMKEY;

	pMKEY = Maxkey;

      for (i=0,pVarDesc=&pBTree->BT_HEAD.BT_VARDESC,IOFF=0;
       	   i<pBTree->BT_HEAD.BT_NVARS;pVarDesc++,i++)
      	{	if (pVarDesc->BT_VARTYP == BT_CHAR || pVarDesc->BT_VARTYP == BT_RIGHT_CHAR)
                _fmemset (pMKEY,0,pVarDesc->BT_VARLEN);
            else if(pVarDesc->BT_VARTYP == BT_REAL4)
                *(LPFLOAT)pMKEY = -FLT_MAX;
            else if (pVarDesc->BT_VARTYP == BT_REAL8)
                *(LPDOUBLE)pMKEY = -DBL_MAX;
            else if (pVarDesc->BT_VARTYP == BT_INT2)
                *(LPSHORT)pMKEY = SHRT_MIN;
            else if (pVarDesc->BT_VARTYP == BT_INT4)
                *(LPLONG)pMKEY = LONG_MIN;
            pMKEY += pVarDesc->BT_VARLEN;
      	}
#if ENABLETRACE
}
#endif
}
BOOL BT_SET_TIME_STAMP (HGLOBAL IBTID, time_t TIME)
#if ENABLETRACE
{GSSiEnterProg (475);
#endif
{	
	LPBTREE	pBTree;

    if (!IBTID)
{
#if ENABLETRACE
GSSiExitProg (475);
#endif
    	return (FALSE);
}
    if (!(pBTree = AllocateBTMem(IBTID)))
{
#if ENABLETRACE
GSSiExitProg (475);
#endif
    	return FALSE;
}
    pBTree->BT_HEAD.BT_TIME_STAMP = TIME;
    DeallocateBTMem(pBTree,IBTID); 
{
#if ENABLETRACE
GSSiExitProg (475);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL BT_CHECK_TIME_STAMP (HGLOBAL IBTID, time_t TIME)
#if ENABLETRACE
{GSSiEnterProg (475); 
#endif
{	
	BOOL	rtn;
	LPBTREE	pBTree;

    if (!IBTID)
{
#if ENABLETRACE
GSSiExitProg (475);
#endif
    	return (FALSE);
}
    if (!(pBTree = AllocateBTMem(IBTID)))
{
#if ENABLETRACE
GSSiExitProg (475);
#endif
    	return FALSE;
}
    rtn = (pBTree->BT_HEAD.BT_TIME_STAMP == TIME);
    DeallocateBTMem(pBTree,IBTID);
{
#if ENABLETRACE
GSSiExitProg (475);
#endif
    return (rtn);
}
#if ENABLETRACE
}
#endif
}

BOOL BT_CLOSEANDDELETE (LPHANDLE pIBTID)
#if ENABLETRACE
{GSSiEnterProg (476);
#endif
{ 
	LPBTREE	pBTree;
	HANDLE	hmem=GSSiGlobAlloc ( 106,GMEM_MOVEABLE,256);
	LPSTR	DeleteName=GlobalLock (hmem);
	
	if (!*pIBTID)
{   
		GSSiGlobUlFree (&hmem);
#if ENABLETRACE
GSSiExitProg (476);
#endif  
		return FALSE;  
}
    if (!(pBTree = AllocateBTMem(*pIBTID)))
{
		*pIBTID = 0;
		GSSiGlobUlFree (&hmem);
#if ENABLETRACE
GSSiExitProg (476);
#endif
    	return FALSE;
}
    _fstrcpy (DeleteName,pBTree->BT_FNAME);
    DeallocateBTMem(pBTree,*pIBTID);
	BT_CLOSE (*pIBTID);
	*pIBTID = 0;
	GSSiRemove (DeleteName); 
	GSSiGlobUlFree (&hmem);   
{
#if ENABLETRACE
GSSiExitProg (476);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL BT_GETPATHNAME (HANDLE IBTID,LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (477);
#endif
{   
	LPBTREE	pBTree;
	
	*Name = 0;
	if (!IBTID)
{
#if ENABLETRACE
GSSiExitProg (477);
#endif
		return FALSE;  
}   
	if (!GlobalSize (IBTID))
{
#if ENABLETRACE
GSSiExitProg (477);
#endif
		return FALSE;  
}   

    if (!(pBTree = AllocateBTMem(IBTID)))
{
#if ENABLETRACE
GSSiExitProg (477);
#endif
    	return FALSE;
}
    _fstrcpy (Name,pBTree->BT_FNAME);
    DeallocateBTMem(pBTree,IBTID);
{
#if ENABLETRACE
GSSiExitProg (477);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

int BT_CLOSE (HGLOBAL IBTID)
#if ENABLETRACE
{GSSiEnterProg (478);
#endif
{     int	ST, i;
	  LPBTREE	pBTree;

      if (!IBTID)
{
#if ENABLETRACE
GSSiExitProg (478);
#endif
      	return (0);
}
    if (!(pBTree = AllocateBTMem(IBTID)))
{
#if ENABLETRACE
GSSiExitProg (478);
#endif
      	return(0);
}
/*      MS_$UNMAP(pBTree->BTID_CUR_BLK_PNT,BT_HEAD->BT_BLKSIZE,BTST)*/;
      ST = 0;
	  if (TraceOn)
      {
      	HANDLE	hmem=GSSiGlobAlloc ( 107,GMEM_MOVEABLE,256);
      	LPSTR	str=GlobalLock (hmem);
	  	sprintf (str,"Close BTree: %s  ID: %ld",pBTree->BT_FNAME,(long)IBTID);
	  	GSSiTrace(str,0); 
	  	GSSiGlobUlFree (&hmem);
	  }
      if (pBTree->BT_HEAD.BT_BEING_UPDATED && !pBTree->BTID_READ)
      {
          pBTree->BT_HEAD.CI_MAX_PNUM  = 8;
          pBTree->BT_HEAD.CI_MAX_DNUM  = 16;
          pBTree->BT_HEAD.SPLIT_PNT_1  = 4;
          pBTree->BT_HEAD.SPLIT_PNT_2  = 4;
          pBTree->BT_HEAD.SPLIT_DAT_1  = 10;
          pBTree->BT_HEAD.SPLIT_DAT_2  = 6;
          pBTree->BT_HEAD.BT_MAX_BLOCK    = pBTree->BT_HEAD.BT_LENGTH / pBTree->BT_HEAD.BT_BLKSIZE;
          pBTree->BT_HEAD.BT_BEING_UPDATED = FALSE;
	      GSSillseek (pBTree->BtFid,0,0);
    	  btwrite (pBTree->BtFid,(LPSTR)&pBTree->BT_HEAD,1024,0);
        /*  chsize (pBTree->BtFid,pBTree->BT_HEAD.BT_HEADLEN + pBTree->BT_HEAD.BT_LENGTH + 32);*/
     	  for (i = 0,pBTree->BT_BUFF_CURINDEX = &pBTree->BT_BUFF_INDEX->index0;
     	  	   i<pBTree->BT_BUFF_INDEX->usedbufs;
     	  	   i++,pBTree->BT_BUFF_CURINDEX++)
     	  {
			  pBTree->BT_BLOCK = (LPBTBLOCK)((LPSTR)&pBTree->BT_BUFFERS->FirstBuf + i * (long)pBTree->BT_HEAD.BT_BLKSIZE);
	     	  if (pBTree->BT_BLOCK->Dirty) 
	     	  {    
	     	  	   long seekloc = 1024+pBTree->BT_BUFF_CURINDEX->bufid*(long)pBTree->BT_HEAD.BT_BLKSIZE; 
	     	  	   
	     	  	   GSSillseek  (pBTree->BtFid,seekloc,0);
	     	  	   pBTree->BT_BLOCK->Dirty = FALSE;
	     	       btwrite (pBTree->BtFid,(LPSTR)pBTree->BT_BLOCK,pBTree->BT_HEAD.BT_BLKSIZE,seekloc);
	     	  }
	      }
      }
      /*MS_$UNMAP (BT_HEAD,1024,IST)*/;
      ST = GSSiClose (pBTree->BtFid);
      //GSSiGlobUlFree (&pBTree->hBT_HEAD);
      GSSiGlobUlFree (&pBTree->hBT_BLOCK);
      pBTree = 0;
      GSSiGlobUlFree (&IBTID);
{
#if ENABLETRACE
GSSiExitProg (478);
#endif
	  return(ST);
}
#if ENABLETRACE
}
#endif
}

BOOL BT_CLEAR (HGLOBAL IBTID)
#if ENABLETRACE
{GSSiEnterProg (479);
#endif
{     
	short ii;
	LPBTREE	pBTree;
	 
      if (!IBTID)
{
#if ENABLETRACE
GSSiExitProg (479);
#endif
      	return (FALSE);
}
    if (!(pBTree = AllocateBTMem(IBTID)))
{
#if ENABLETRACE
GSSiExitProg (479);
#endif
      	return(FALSE);
}
      if (pBTree->BT_HEAD.BT_BEING_UPDATED && !pBTree->BTID_READ)
      {
          pBTree->BT_HEAD.BT_NUMRECS = 0;
          pBTree->BT_HEAD.BT_LENGTH = 0;
          pBTree->BT_HEAD.BT_MAX_BLOCK = 0;
          pBTree->BT_HEAD.BT_FIRST_POS = 0; 
          pBTree->BT_PATH_CHANGE = TRUE;  
		  pBTree->BT_BUFF_INDEX->usedbufs = 1;
          if (!GSSiChangeLength (pBTree->BtFid,pBTree->BT_HEAD.BT_HEADLEN + pBTree->BT_HEAD.BT_LENGTH + 32))
          {
          	GSSiClose (pBTree->BtFid);
          	CloseFidSmall ();
          	pBTree->BtFid = GSSiOpenFile (pBTree->BT_FNAME,0,OF_READWRITE);
          	if (!GSSiChangeLength (pBTree->BtFid,pBTree->BT_HEAD.BT_HEADLEN + pBTree->BT_HEAD.BT_LENGTH + 32))
          		ii=1;
          	GSSiClose (pBTree->BtFid);
          	CreateFidSmall ();
          	pBTree->BtFid = GSSiOpenFile (pBTree->BT_FNAME,0,OF_READWRITE);
          }
		  DeallocateBTMem(pBTree,IBTID); 
{
#if ENABLETRACE
GSSiExitProg (479);
#endif
          return TRUE;
}
      }
	  DeallocateBTMem(pBTree,IBTID); 
{
#if ENABLETRACE
GSSiExitProg (479);
#endif
      return FALSE;
}
#if ENABLETRACE
}
#endif
} 

void CreateFidSmall (void)
#if ENABLETRACE
{GSSiEnterProg (480);
#endif
{   
	HANDLE	hMem;
	LPOFSTRUCTGM	pOFStruct;
{
#if ENABLETRACE
GSSiExitProg (480);
#endif
	return;
}
	if (!*SmallName)
		GSSiGetTempFileName (0,"GMS",0,(LPSTR)SmallName);
	hMem = GSSiGlobAlloc(108, GMEM_MOVEABLE, sizeof(OFSTRUCTGM));
	pOFStruct = (LPOFSTRUCTGM)GlobalLock (hMem);	
//   	FidSmall = OpenFile (SmallName,pOFStruct,OF_CREATE);
   	FidSmall = _open (SmallName,_O_CREAT,_S_IREAD | _S_IWRITE);
   	GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (480);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}  

void CloseFidSmall (void)
{
	return;
}

void BT_SET_PARMS (int IMPN, int IMDN, int ISPN, int ISDN)
#if ENABLETRACE
{GSSiEnterProg (482);
#endif
{     
	if (!IMPN)
		SET_PARMS = FALSE;
	else
	{
      MPN = IMPN;
      MDN = IMDN;
      SPN = ISPN;
      SDN = ISDN;
      SET_PARMS = TRUE; 
    }
{
#if ENABLETRACE
GSSiExitProg (482);
#endif
      return;
}
#if ENABLETRACE
}
#endif
}

void DISABLE_TIME_STAMP_CHECK(void)
#if ENABLETRACE
{GSSiEnterProg (483);
#endif
{
      TIME_CHECK =FALSE;
{
#if ENABLETRACE
GSSiExitProg (483);
#endif
      return;
}
#if ENABLETRACE
}
#endif
}

      
void BTreeErrorMessage (LPBTREE pBTree,LPSTR Mess)
#if ENABLETRACE
{GSSiEnterProg (484);
#endif
{   
	char	str[128]="Error in BTree index:";
	
	_fstrcpy (str,Mess);
	DoPaint = FALSE;
	GSSiMsgBox( GetFocus(), str,pBTree->BT_FNAME, MB_OK|MB_ICONEXCLAMATION|MB_APPLMODAL,0);
    BlowOut (0,0);
#if ENABLETRACE
}
#endif
}
long BT_NUMLEVELS_IN_INDEX (HGLOBAL IBTID)
{
	LPBTREE	pBTree;
	long	num;
	
	if (!IBTID)
		return 0;
    if (!(pBTree = AllocateBTMem(IBTID)))
		return 0;
    num = pBTree->BT_HEAD.BT_NUMLEVS;
    DeallocateBTMem(pBTree,IBTID); 
	return num;
}
long GetBT_DATLEN (HGLOBAL IBTID)
{
	LPBTREE	pBTree;
	long	num;
	
	if (!IBTID)
		return 0;
    if (!(pBTree = AllocateBTMem(IBTID)))
		return 0;
    num = pBTree->BT_HEAD.BT_DATLEN;
    DeallocateBTMem(pBTree,IBTID); 
	return num;
}
long GetBT_LENGTH (HGLOBAL IBTID)
{
	LPBTREE	pBTree;
	long	num;
	
	if (!IBTID)
		return 0;
    if (!(pBTree = AllocateBTMem(IBTID)))
		return 0;
    num = pBTree->BT_HEAD.BT_LENGTH;
    DeallocateBTMem(pBTree,IBTID); 
	return num;
}

long BT_NUM_IN_INDEX (HGLOBAL IBTID)
#if ENABLETRACE
{GSSiEnterProg (485);
#endif
{   long num;
	LPBTREE	pBTree;
    
    if (!IBTID)
{
#if ENABLETRACE
GSSiExitProg (485);
#endif
    	return 0;
}
    if (!(pBTree = AllocateBTMem(IBTID)))
{
#if ENABLETRACE
GSSiExitProg (485);
#endif
    	return 0;
}
    num = pBTree->BT_HEAD.BT_NUMRECS;
    DeallocateBTMem(pBTree,IBTID); 
{
#if ENABLETRACE
GSSiExitProg (485);
#endif
    return (num);
}
#if ENABLETRACE
}
#endif
}

int BT_FIND (HGLOBAL IBTID,LPSTR KEY,int POSITION,int COND,LPSTR DATA)
#if ENABLETRACE
{GSSiEnterProg (486);
#endif
{	int		irc;
	BOOL	LASTRC;
	LPBTREE	pBTree;

    if (!IBTID)
{
#if ENABLETRACE
GSSiExitProg (486);
#endif
    	return (BT_NOT_FOUND);
}
    if (!(pBTree = AllocateBTMem(IBTID)))
{
#if ENABLETRACE
GSSiExitProg (486);
#endif
    	return (BT_NOT_FOUND);
}
    irc = BT_FIND_internal (pBTree,KEY, POSITION, COND, DATA, &LASTRC);
    DeallocateBTMem(pBTree,IBTID); 
{
#if ENABLETRACE
GSSiExitProg (486);
#endif
    return (irc);
}
#if ENABLETRACE
}
#endif
}

int BT_FIND_internal (LPBTREE pBTree,LPSTR KEY,int POSITION,int COND,LPSTR DATA,LPBOOL LASTRC)
#if ENABLETRACE
{GSSiEnterProg (487);
#endif
{	int 	BTST, Compare;
	BOOL    BACKWARDS, BEFORE_FIRST=FALSE, FIRST_CALL;  
	BOOL	CheckSecIndexEQ = FALSE;
	long	DOWN; 
	HANDLE	hInKey=0; 
	LPSTR	InKey;  
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
      if (pBTree->BT_HEAD.BT_DUPS && !pBTree->BT_UPDATE)     //pBTree->BT_HEAD.BT_VARDESC[2]
      	pBTree->READ_SECIDX =TRUE;
      if (!pBTree->BT_HEAD.BT_NUMRECS)
{
#if ENABLETRACE
GSSiExitProg (487);
#endif
      	return (BT_NOT_FOUND);
}     
	  if (pBTree->READ_SECIDX)
	  {
	  	if (POSITION == BT_FIRST && COND == BT_EQ)
	  	{
	  		CheckSecIndexEQ = TRUE;
	  		COND = BT_GE; 
	  		hInKey = GSSiGlobAlloc (0,GMEM_MOVEABLE,pBTree->BT_HEAD.BT_KYLEN);
	  		InKey = GlobalLock (hInKey);
	  		_fmemmove (InKey,KEY,pBTree->BT_HEAD.BT_KYLEN);
	  	}
	  	pBTree->READ_SECIDX = FALSE;
	  }
      if (POSITION == BT_PRIOR)
      {
          if (pBTree->BTID_FIRST_CALL) 
          {
          	GSSiGlobUlFree (&hInKey);
{
#if ENABLETRACE
GSSiExitProg (487);
#endif
          	return (BT_NOT_FOUND);
}          
		  }
          pBTree->POS = pBTree->BTID_CUR_POS;
          pBTree->LAST_POS = pBTree->POS;
          BT_GET_BLOCK (pBTree);
          FIRST_CALL = FALSE;
          FIND_PRIOR_BTPOS(pBTree);
          pBTree->BTLEV = pBTree->BT_HEAD.BT_NUMLEVS;
          if (pBTree->BT_END)
			  goto S1100;
          goto S1000;
      }
      else if (POSITION == BT_LAST)
      {
          pBTree->BTID_FIRST_CALL  = FALSE;
          if (pBTree->BT_HEAD.BT_DATED) _fmemmove (&pBTree->BTID_LASKEY,BT_MAXKEY(pBTree),pBTree->BT_HEAD.BT_KYLEN);
          pBTree->POS.POSA   = pBTree->BT_HEAD.BT_FIRST_POS;
          pBTree->POS.POSB   = 1;
          pBTree->BTLEV  = 1;
          BT_GET_BLOCK(pBTree);
          pBTree->POS.POSB   = pBTree->CI_PNT->LNCI;
          BT_GET_BLOCK(pBTree);
          DOWN   = pBTree->INDEX_ITEM_PNT->BT_DOWN;
          pBTree->BT_END = FALSE;
          FIRST_CALL = TRUE;
          BACKWARDS =TRUE;
      }
      else if (POSITION == BT_FIRST || pBTree->BTID_FIRST_CALL)
      {
              pBTree->BTID_FIRST_CALL  = FALSE;
	          if (pBTree->BT_HEAD.BT_DATED) _fmemmove (&pBTree->BTID_LASKEY,BT_MAXKEY(pBTree),pBTree->BT_HEAD.BT_KYLEN);
              if (pBTree->BT_PATH_CHANGE || pBTree->BT_HEAD.BT_KEYLEN > 64 || COND == BT_ANY) goto S20;
              for (pBTree->BTLEV=pBTree->BT_HEAD.BT_NUMLEVS-1; pBTree->BTLEV; pBTree->BTLEV--)
              {		if (BT_KEY_COMPARE(pBTree,KEY,BT_PATH_KEY_BEG(pBTree,pBTree->BTLEV),BT_UPPER)<0) continue;
                    if (BT_KEY_COMPARE(pBTree,KEY,BT_PATH_KEY_END(pBTree,pBTree->BTLEV),BT_UPPER)>0) continue;
                    goto S10;
 			  }
              goto S20;
S10:          pBTree->POS = pBTree->BT_PATH_POS[pBTree->BTLEV-1];
              pBTree->BTLEV++;
              goto S30;
S20:          pBTree->POS.POSA = pBTree->BT_HEAD.BT_FIRST_POS;
              pBTree->POS.POSB = 1;
              BEFORE_FIRST =TRUE;
              pBTree->BTLEV = 1;
S30:          BT_GET_BLOCK(pBTree);
              DOWN     = pBTree->INDEX_ITEM_PNT->BT_DOWN;
              pBTree->BT_END   = FALSE;
              FIRST_CALL = TRUE;

      }
      else if (POSITION == BT_CURPOS)
      {
              pBTree->POS = pBTree->BTID_CUR_POS;
              pBTree->LAST_POS = pBTree->POS;
              BT_GET_BLOCK(pBTree);
              FIRST_CALL = FALSE;
              pBTree->BTLEV = pBTree->BT_HEAD.BT_NUMLEVS;
	  }
      else
      {
              pBTree->POS = pBTree->BTID_CUR_POS;
              pBTree->LAST_POS = pBTree->POS;
              BT_GET_BLOCK(pBTree);
              FIRST_CALL = FALSE;
              FIND_NEXT_BTPOS(pBTree);
              pBTree->BTLEV = pBTree->BT_HEAD.BT_NUMLEVS;
      }

S100: if (pBTree->BT_END)
		  goto S1100;
      if (COND == BT_ANY)
		  goto S1000;
      Compare = BT_KEY_COMPARE (pBTree,KEY,&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,pBTree->CI_PNT->CITYPE);
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
		        {	DOWN = pBTree->INDEX_ITEM_PNT->BT_DOWN;
			        goto S1000;
			    }
		        else if (COND == BT_GT) goto S900;
	            goto S1100;
	  }
S900:  if (pBTree->CI_PNT->CITYPE == BT_BOTTOM)
      {		pBTree->LAST_POS = pBTree->POS;
            BEFORE_FIRST = FALSE;
      }
      else
            DOWN = pBTree->INDEX_ITEM_PNT->BT_DOWN;
      if (POSITION == BT_PRIOR)
      {   FIND_PRIOR_BTPOS (pBTree);
          if (pBTree->BT_END)
			  goto S1100;
          goto S1000;
      }
      else
      {   if (BACKWARDS)
              FIND_PRIOR_BTPOS(pBTree);
          else
              FIND_NEXT_BTPOS(pBTree);
      }

      goto S100;

S1000: 
	  if (pBTree->CI_PNT->LNCI <=0)
	  	BTreeErrorMessage (pBTree,"Invalid LNCI");

	   
	   if (pBTree->CI_PNT->CITYPE == BT_BOTTOM)
       {      if (pBTree->BT_HEAD.BT_DATED)
       		  {   if (_fmemcmp(IADDR((LPSTR)&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,pBTree->BT_HEAD.BT_KYLEN+1),
                      &pBTree->BTID_DATE,2) < 0) goto S900;
                  if (!_fmemcmp (&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,
                      &pBTree->BTID_LASKEY,pBTree->BT_HEAD.BT_KYLEN)) goto S900;
                  _fmemmove (&pBTree->BTID_LASKEY,&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,pBTree->BT_HEAD.BT_KYLEN);
                  _fmemmove (&bt_get_date,IADDR((LPSTR)&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,2),2);
              }
              BTST = 0;
              pBTree->BTID_CUR_POS = pBTree->POS;     
              	if (DATA)
              	_fmemmove (DATA,pBTree->INDEX_DATA_PNT ,pBTree->BT_HEAD.BT_DATLEN);
              _fmemmove (KEY,&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,pBTree->BT_HEAD.BT_KYLEN);
              if (FIRST_CALL) pBTree->BT_PATH_CHANGE = FALSE;
       }
       else
S1005: {      pBTree->POS.POSA = DOWN;
              pBTree->POS.POSB = 1;
              if (pBTree->BT_HEAD.BT_KEYLEN <= 64)
              {
                  if (pBTree->BT_END)
			      	_fmemmove (BT_PATH_KEY_END(pBTree,pBTree->BTLEV),BT_MAXKEY(pBTree),pBTree->BT_HEAD.BT_KEYLEN);
                  else
			      	_fmemmove (BT_PATH_KEY_END(pBTree,pBTree->BTLEV),&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,pBTree->BT_HEAD.BT_KEYLEN);
                  pBTree->BT_PATH_POS[pBTree->BTLEV-1] = pBTree->POS;
              }
              BT_GET_BLOCK(pBTree);
              if (BACKWARDS)
              {
                  pBTree->POS.POSB = pBTree->CI_PNT->LNCI;
                  BT_GET_BLOCK(pBTree);
              }
              DOWN = pBTree->INDEX_ITEM_PNT->BT_DOWN;
              if (pBTree->BT_HEAD.BT_KEYLEN <= 64)
			      _fmemmove (BT_PATH_KEY_BEG(pBTree,pBTree->BTLEV),&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,pBTree->BT_HEAD.BT_KEYLEN);

              pBTree->BTLEV += 1;
              pBTree->BT_END = FALSE;
              goto S100;
       }
       goto S2000;

S1100: if (pBTree->CI_PNT->CITYPE == BT_BOTTOM)
       {	if (FIRST_CALL) pBTree->BT_PATH_CHANGE = FALSE;
            if (pBTree->BT_END && pBTree->CI_PNT->NEXTCI == BT_NIL)
				*LASTRC =TRUE;
            BTST   = BT_NOT_FOUND;
            if (BEFORE_FIRST)
            	pBTree->BTID_FIRST_CALL = TRUE;
            else
                pBTree->BTID_CUR_POS = pBTree->LAST_POS;
       }
       else
            goto S1005;
S2000: if (pBTree->BTLEV != pBTree->BT_HEAD.BT_NUMLEVS)
			BTreeErrorMessage (pBTree,"Invalid BTLEV"); 
	   if (!BTST && CheckSecIndexEQ)
	   {
	   	  	pBTree->READ_SECIDX = TRUE;
			if (BT_KEY_COMPARE (pBTree,KEY,InKey,BT_BOTTOM))
				BTST = 31;
	   }   
	   GSSiGlobUlFree (&hInKey);
{
#if ENABLETRACE
GSSiExitProg (487);
#endif
      return (BTST);
}
#if ENABLETRACE
}
#endif
}


/*void UMDB_UPDATE_BT_DATA (HGLOBAL IBTID,LPSTR DATA)
#if ENABLETRACE
{GSSiEnterProg (488);
#endif
{
    if (AllocateBTMem(IBTID))
{
#if ENABLETRACE
GSSiExitProg (488);
#endif
    	return;
}
	POS = pBTree->BTID_CUR_POS;
    BT_GET_BLOCK();
    UMDB_WRITE_JOURNAL (TRUE);
    _fmemmove (INDEX_DATA_PNT,DATA,BT_HEAD->BT_DATLEN);
    DeallocateBTMem(IBTID);
{
#if ENABLETRACE
GSSiExitProg (488);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}  */

int BT_KEY_COMPARE (LPBTREE pBTree,LPSTR KEY1,LPSTR KEY2,int ITYPE)
#if ENABLETRACE
{GSSiEnterProg (489);
#endif
{
	LPBTVARDESC pVarDesc;
	LPSTR		pBT_CRVAR1, pBT_CRVAR2;
	LPFLOAT		pBT_R4VAR1, pBT_R4VAR2;
	LPDOUBLE	pBT_R8VAR1, pBT_R8VAR2;
	LPSHORT		pBT_I2VAR1, pBT_I2VAR2;
	LPLONG		pBT_I4VAR1, pBT_I4VAR2;
	int			i, j, N;

//	if (BTLEV <=0 || BTLEV > BT_HEAD->BT_NUMLEVS)
//		ii=1;
      if (pBTree->READ_SECIDX)
          N = pBTree->BT_HEAD.BT_DUPPOS;
      else
          N = pBTree->BT_HEAD.BT_NVARS;
      for (i=0,pVarDesc=&pBTree->BT_HEAD.BT_VARDESC;i<N;i++,pVarDesc++)
      {		pBT_CRVAR1 = KEY1 + pVarDesc->BT_VAROFF;
   	  		pBT_CRVAR2 = KEY2 + pVarDesc->BT_VAROFF;
      		switch (pVarDesc->BT_VARTYP)
   	  		{	case BT_CHAR:  
   	  			case BT_RIGHT_CHAR:
   	  				j = _fmemcmp (pBT_CRVAR1,pBT_CRVAR2,pVarDesc->BT_VARLEN);
   	  				if (j!=0)
{
#if ENABLETRACE
GSSiExitProg (489);
#endif
   	  					return (j);
}
			       	break;

   	  			case BT_REAL4:
   	  				pBT_R4VAR1 = (LPFLOAT)pBT_CRVAR1;
   	  				pBT_R4VAR2 = (LPFLOAT)pBT_CRVAR2;
   	  				if (*pBT_R4VAR1 < *pBT_R4VAR2)
{
#if ENABLETRACE
GSSiExitProg (489);
#endif
   	  					return (-1);
}
   	  				if (*pBT_R4VAR1 > *pBT_R4VAR2)
{
#if ENABLETRACE
GSSiExitProg (489);
#endif
   	  					return (1);
}
			       	break;

   	  			case BT_REAL8:
   	  				pBT_R8VAR1 = (LPDOUBLE)pBT_CRVAR1;
   	  				pBT_R8VAR2 = (LPDOUBLE)pBT_CRVAR2;
   	  				if (*pBT_R8VAR1 < *pBT_R8VAR2)
{
#if ENABLETRACE
GSSiExitProg (489);
#endif
   	  					return (-1);
}
   	  				if (*pBT_R8VAR1 > *pBT_R8VAR2)
{
#if ENABLETRACE
GSSiExitProg (489);
#endif
   	  					return (1);
}
			       	break;

   	  			case BT_INT2:
   	  				pBT_I2VAR1 = (LPSHORT)pBT_CRVAR1;
   	  				pBT_I2VAR2 = (LPSHORT)pBT_CRVAR2;
   	  				if (*pBT_I2VAR1 < *pBT_I2VAR2)
{
#if ENABLETRACE
GSSiExitProg (489);
#endif
   	  					return (-1);
}
   	  				if (*pBT_I2VAR1 > *pBT_I2VAR2)
{
#if ENABLETRACE
GSSiExitProg (489);
#endif
   	  					return (1);
}
			       	break;

   	  			case BT_INT4:
   	  				pBT_I4VAR1 = (LPLONG)pBT_CRVAR1;
   	  				pBT_I4VAR2 = (LPLONG)pBT_CRVAR2;
   	  				if (*pBT_I4VAR1 < *pBT_I4VAR2)
{
#if ENABLETRACE
GSSiExitProg (489);
#endif
   	  					return (-1);
}
   	  				if (*pBT_I4VAR1 > *pBT_I4VAR2)
{
#if ENABLETRACE
GSSiExitProg (489);
#endif
   	  					return (1);
}
			       	break;
              }
      } /* end for loop */

      if (pBTree->BT_HEAD.BT_DATED && ITYPE == BT_UPPER)
      {
  	  		pBT_CRVAR2 = KEY2 + pBTree->BT_HEAD.BT_KYLEN;
			pBT_I2VAR2 = (LPSHORT)pBT_CRVAR2;
			if (*pBT_I2VAR1 < *pBT_I2VAR2)
{
#if ENABLETRACE
GSSiExitProg (489);
#endif
				return (-1);
}
			if (*pBT_I2VAR1 > *pBT_I2VAR2)
{
#if ENABLETRACE
GSSiExitProg (489);
#endif
				return (1);
}
      }
{
#if ENABLETRACE
GSSiExitProg (489);
#endif
      return (0);
}
#if ENABLETRACE
}
#endif
}

void BT_GET_BLOCK(LPBTREE pBTree)
#if ENABLETRACE
{GSSiEnterProg (490);
#endif
{     int	inc;
	  long	OldestUse,ii;
	  int	OldestBuf;
	  int	i;
	  BOOL	CheckPtr=FALSE;

      pBTree->BT_BLOCK = pBTree->BTID_CUR_BLK_PNT;
      pBTree->BT_BLOCK_NUM = pBTree->POS.POSA / pBTree->BT_HEAD.BT_BLKSIZE;
      if (pBTree->BT_BLOCK_NUM != pBTree->BTID_CUR_BLK)
      {
/*          BT_BLOCK = MS_$REMAP (BT_BLOCK,
     +                          BT_HEAD_LEN+BT_BLOCK_NUM*BT_BLKSIZE,
     +                          BT_BLKSIZE,IDUM,ST)*/
     	  for (i = 0,pBTree->BT_BUFF_CURINDEX = &pBTree->BT_BUFF_INDEX->index0, OldestUse=LONG_MAX;
     	  	   i<pBTree->BT_BUFF_INDEX->usedbufs;
     	  	   i++,pBTree->BT_BUFF_CURINDEX++)
     	  {
      		if (pBTree->BT_BLOCK_NUM == pBTree->BT_BUFF_CURINDEX->bufid)
      		{	pBTree->BT_BLOCK = (LPBTBLOCK)((LPSTR)&pBTree->BT_BUFFERS->FirstBuf + i * (long)pBTree->BT_HEAD.BT_BLKSIZE);
      			goto S100;
      		}
      		if (pBTree->BT_BUFF_CURINDEX->bufuse < OldestUse)
      		{	OldestUse = pBTree->BT_BUFF_CURINDEX->bufuse;
      			OldestBuf = i;
      		}
      	  }
      	  if (i == pBTree->BT_BUFFERS->maxbufs)
      	  {
	          i = OldestBuf;
			  pBTree->BT_BUFF_CURINDEX = &pBTree->BT_BUFF_INDEX->index0 + i;
			  pBTree->BT_BLOCK = (LPBTBLOCK)((LPSTR)&pBTree->BT_BUFFERS->FirstBuf + i * (long)pBTree->BT_HEAD.BT_BLKSIZE);

	     	  if (pBTree->BT_BLOCK->Dirty)
	     	  {   
	     	  	   long	ii,seekloc = 1024+pBTree->BT_BUFF_CURINDEX->bufid*(long)pBTree->BT_HEAD.BT_BLKSIZE;
	     	  	   
	     	  	   ii = GSSillseek  (pBTree->BtFid,seekloc,0);
	     	  	   pBTree->BT_BLOCK->Dirty = FALSE;
	     	       btwrite (pBTree->BtFid,(LPSTR)pBTree->BT_BLOCK,(UINT)pBTree->BT_HEAD.BT_BLKSIZE,seekloc);
	     	  }
	       }
	       else
	       {
	       	  pBTree->BT_BUFF_INDEX->usedbufs++;
	       	  pBTree->BT_BLOCK = (LPBTBLOCK)((LPSTR)&pBTree->BT_BUFFERS->FirstBuf + (long)i * (long)pBTree->BT_HEAD.BT_BLKSIZE);
	       }
		  ii=GSSillseek (pBTree->BtFid,1024+pBTree->BT_BLOCK_NUM*pBTree->BT_HEAD.BT_BLKSIZE,0);
          ii=BigRead (pBTree->BtFid,(HPSTR)pBTree->BT_BLOCK,pBTree->BT_HEAD.BT_BLKSIZE);
		  CheckPtr = TRUE;

S100:	  pBTree->BT_BUFF_CURINDEX->bufuse = pBTree->BT_BUFF_INDEX->nextuse++;
     	  pBTree->BT_BUFF_CURINDEX->bufid=pBTree->BT_BLOCK_NUM;
      	  pBTree->BT_BUFF_INDEX->curbuf = i;
          pBTree->BTID_CUR_BLK = pBTree->BT_BLOCK_NUM;
          pBTree->BTID_CUR_BLK_PNT = pBTree->BT_BLOCK;
      }
	  pBTree->BT_BLOCK_POS   = (pBTree->POS.POSA%pBTree->BT_HEAD.BT_BLKSIZE)+1;
      pBTree->CI_PNT         = (LPCB)IADDR ((LPSTR)&pBTree->BT_BLOCK->BT_DATA,pBTree->BT_BLOCK_POS);
      pBTree->INDEX_ITEM_PNT = (LPBTI)pBTree->CI_PNT; 
      pBTree->pBlockCount = NULL;
      if (pBTree->CI_PNT->CITYPE == BT_UPPER)
      {
         inc = 16 + (pBTree->POS.POSB-1) * pBTree->BT_HEAD.BT_PNTLEN;
         pBTree->INDEX_ITEM_PNT = (LPBTI)((LPSTR)pBTree->INDEX_ITEM_PNT + inc); 
         if (pBTree->BT_HEAD.TrackCount)
         	pBTree->pBlockCount = (LPLONG)((LPSTR)&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY + pBTree->BT_HEAD.BT_KEYLEN);
      }
      else
      {
         inc = 12 + (pBTree->POS.POSB-1) * pBTree->BT_HEAD.BT_RECLEN; /* no DOWN pointer on bottom lever*/
         pBTree->INDEX_ITEM_PNT = (LPBTI)((LPSTR)pBTree->INDEX_ITEM_PNT + inc);
         pBTree->INDEX_DATA_PNT = (LPSTR)pBTree->INDEX_ITEM_PNT;
         pBTree->INDEX_DATA_PNT += (pBTree->BT_HEAD.BT_KEYLEN + 4);
      }
	  if (CheckPtr && pBTree->POS.POSA == pBTree->CI_PNT->NEXTCI)
	  {
			char	mess[128];

			sprintf (mess,"Forward pointer error:%ld",pBTree->POS.POSA);
			BTreeErrorMessage (pBTree,mess);
	  }
{
#if ENABLETRACE
GSSiExitProg (490);
#endif
      return;
}
#if ENABLETRACE
}
#endif
}

void FIND_PRIOR_BTPOS(LPBTREE pBTree)
#if ENABLETRACE
{GSSiEnterProg (491);
#endif
{

      pBTree->BT_END = FALSE;
      if (pBTree->POS.POSB > pBTree->CI_PNT->LNCI)
			BTreeErrorMessage (pBTree,"Invalid POSB");
      if (pBTree->POS.POSB == 1)
      {
          if (pBTree->CI_PNT->LASTCI == BT_NIL)
          {
              pBTree->BT_END = TRUE;
{
#if ENABLETRACE
GSSiExitProg (491);
#endif
              return;
}
          }
          pBTree->POS.POSA = pBTree->CI_PNT->LASTCI;
          BT_GET_BLOCK(pBTree);
          pBTree->POS.POSB = pBTree->CI_PNT->LNCI;
          BT_GET_BLOCK(pBTree);
       }
       else
       {
          pBTree->POS.POSB--;
          if (pBTree->CI_PNT->CITYPE == BT_UPPER)
              pBTree->INDEX_ITEM_PNT = (LPBTI)((LPSTR) pBTree->INDEX_ITEM_PNT - pBTree->BT_HEAD.BT_PNTLEN);
          else
          {
              pBTree->INDEX_ITEM_PNT = (LPBTI)((LPSTR) pBTree->INDEX_ITEM_PNT - pBTree->BT_HEAD.BT_RECLEN);
              pBTree->INDEX_DATA_PNT = pBTree->INDEX_DATA_PNT - pBTree->BT_HEAD.BT_RECLEN;
          }
       }
{
#if ENABLETRACE
GSSiExitProg (491);
#endif
       return;
}
#if ENABLETRACE
}
#endif
}

void FIND_NEXT_BTPOS(LPBTREE pBTree)
#if ENABLETRACE
{GSSiEnterProg (492);
#endif
{
      pBTree->BT_END = FALSE;
      if (pBTree->POS.POSB > pBTree->CI_PNT->LNCI)
			BTreeErrorMessage (pBTree,"Invalid POSB(2)");
      if (pBTree->POS.POSB == pBTree->CI_PNT->LNCI)
      {
          if (pBTree->CI_PNT->NEXTCI == BT_NIL)
          {
              pBTree->BT_END = TRUE;
              pBTree->POS.POSB++;
              if (pBTree->CI_PNT->CITYPE == BT_BOTTOM)
              {
                  pBTree->INDEX_ITEM_PNT = (LPBTI)((LPSTR) pBTree->INDEX_ITEM_PNT + pBTree->BT_HEAD.BT_RECLEN);
                  pBTree->INDEX_DATA_PNT = pBTree->INDEX_DATA_PNT + pBTree->BT_HEAD.BT_RECLEN;
              }
{
#if ENABLETRACE
GSSiExitProg (492);
#endif
              return;
}
           }
           pBTree->POS.POSA = pBTree->CI_PNT->NEXTCI;
           pBTree->POS.POSB = 1;
           BT_GET_BLOCK(pBTree);
      }
      else
      {
           pBTree->POS.POSB++;
           if (pBTree->CI_PNT->CITYPE == BT_UPPER)
              pBTree->INDEX_ITEM_PNT = (LPBTI)((LPSTR) pBTree->INDEX_ITEM_PNT + pBTree->BT_HEAD.BT_PNTLEN);
           else
           {
              pBTree->INDEX_ITEM_PNT = (LPBTI)((LPSTR) pBTree->INDEX_ITEM_PNT + pBTree->BT_HEAD.BT_RECLEN);
              pBTree->INDEX_DATA_PNT = pBTree->INDEX_DATA_PNT + pBTree->BT_HEAD.BT_RECLEN;
           }
	  }
{
#if ENABLETRACE
GSSiExitProg (492);
#endif
	  return;
}
#if ENABLETRACE
}
#endif
}

LPSTR BT_MAXKEY(LPBTREE pBTree)
#if ENABLETRACE
{GSSiEnterProg (493);
#endif
{   LPSTR 	ipnt;
	int 	off;
	ipnt = (LPSTR)pBTree;
	ipnt += sizeof(BTREE) -1;
	off = pBTree->BT_HEAD.BT_KEYLEN;
{
#if ENABLETRACE
GSSiExitProg (493);
#endif
	return (ipnt += off);
}
#if ENABLETRACE
}
#endif
}


LPSTR BT_PATH_KEY_BEG(LPBTREE pBTree,int BTLEV)
#if ENABLETRACE
{GSSiEnterProg (494);
#endif
{   LPSTR 	ipnt;
	int 	off,ii;  
//	if (BTLEV < 1 || BTLEV > 10)
//		ii=1;
	ipnt = (LPSTR)pBTree;
	ipnt += sizeof(BTREE) -1;
	off = pBTree->BT_HEAD.BT_KEYLEN * BTLEV;
{
#if ENABLETRACE
GSSiExitProg (494);
#endif
	return (ipnt += off);
}
#if ENABLETRACE
}
#endif
} 

/*void BT_GETDUPDATA (LPSTR DupData,short LenDupData)
#if ENABLETRACE
{GSSiEnterProg (495);
#endif
{
	_fmemcpy (DupData,DATA2,LenDupData);
{
#if ENABLETRACE
GSSiExitProg (495);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}*/

LPSTR BT_PATH_KEY_END(LPBTREE pBTree,int BTLEV)
#if ENABLETRACE
{GSSiEnterProg (496);
#endif
{
	LPSTR 	ipnt;
	int 	off;
	ipnt = (LPSTR)pBTree;
	ipnt += sizeof(BTREE) -1;
	off = pBTree->BT_HEAD.BT_KEYLEN*16 + pBTree->BT_HEAD.BT_KEYLEN * BTLEV;
{
#if ENABLETRACE
GSSiExitProg (496);
#endif
	return (ipnt += off);
}
#if ENABLETRACE
}
#endif
}

int BT_PUT (HGLOBAL hBTree, LPSTR KEY,LPSTR DATA)
#if ENABLETRACE
{GSSiEnterProg (497);
#endif
{	  int irc; 
	  static n=0;//,debugoff=5368;   
	  LPLONG	pOff;
	  short	ii;
	  LPBTREE	pBTree;
      
      if (!hBTree)
{
#if ENABLETRACE
GSSiExitProg (497);
#endif
      	return (1);
}
      pOff = (LPLONG)DATA;
//      if (*pOff == debugoff)
//      	ii=1;
    if (!(pBTree = AllocateBTMem(hBTree)))
{
#if ENABLETRACE
GSSiExitProg (497);
#endif
	  	return 1;
}
      irc = BT_PUT_internal (pBTree,KEY,DATA,FALSE,TRUE,0);
      DeallocateBTMem(pBTree,hBTree);
{
#if ENABLETRACE
GSSiExitProg (497);
#endif
      return (irc);
}
#if ENABLETRACE
}
#endif
}


/*int BT_PUT_NO_FIND (HGLOBAL hBTree,LPSTR KEY,LPSTR DATA, BOOL LRC, int BTST)
#if ENABLETRACE
{GSSiEnterProg (498);
#endif
{	  int irc; 
	if (!AllocateBTMem(hBTree))
{
#if ENABLETRACE
GSSiExitProg (498);
#endif
		return 1;
}
	irc = BT_PUT_internal (KEY,DATA,LRC,FALSE,BTST);
    DeallocateBTMem(hBTree);
{
#if ENABLETRACE
GSSiExitProg (498);
#endif
    return (irc);
}
#if ENABLETRACE
}
#endif
}*/

int BT_PUT_internal (LPBTREE pBTree,LPSTR KEY,LPSTR DATA, BOOL LRC, BOOL DO_FIND, int BTST)   //returns 0 if record replaced, 31 if added, -1 if not open for write
#if ENABLETRACE
{GSSiEnterProg (499);
#endif
{
	  BOOL LASTRC;
	  int ST, LREC,ii;

      pBTree->BT_UPDATE = TRUE;
      ST = BTST;
      LASTRC = LRC;  
      if (!pBTree->BT_HEAD.BT_BEING_UPDATED)
      	return -1;
/******* IF THIS IS A NEW B-TREE INITIALIZE IT*/
      if (!pBTree->BT_HEAD.BT_NUMRECS)
      {
          pBTree->BT_HEAD.BT_FIRST_POS = 0;
          pBTree->POS.POSA    = 0;
          pBTree->POS.POSB    = 1;
          BT_GET_BLOCK(pBTree);
          UMDB_WRITE_JOURNAL (pBTree,TRUE);
          pBTree->CI_PNT->LASTCI  = BT_NIL;
          pBTree->CI_PNT->NEXTCI  = BT_NIL;
          pBTree->CI_PNT->UPCI    = BT_NIL;
          pBTree->CI_PNT->LNCI    = 1;
          pBTree->CI_PNT->CITYPE  = BT_UPPER;
          pBTree->INDEX_ITEM_PNT = (LPBTI)((LPSTR)pBTree->CI_PNT + 16); 
          if (pBTree->BT_HEAD.TrackCount) 
          {
         	pBTree->pBlockCount = (LPLONG)((LPSTR)&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY + pBTree->BT_HEAD.BT_KEYLEN);
         	*pBTree->pBlockCount = 1;
          }
          
          pBTree->INDEX_ITEM_PNT->BT_DOWN = pBTree->BT_HEAD.BT_BLKSIZE;
          _fmemmove (&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,KEY,pBTree->BT_HEAD.BT_KYLEN);
          if (pBTree->BT_HEAD.BT_DATED)
     		  _fmemmove (IADDR(&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,pBTree->BT_HEAD.BT_KYLEN+1),
     		  			 &pBTree->BTID_DATE,2);
          LREC = pBTree->BT_HEAD.CI_LENGTH_1;
          pBTree->BT_BLOCK->BT_FSPACE_BEG = LREC + 1;
          pBTree->BT_BLOCK->Dirty = TRUE;
          pBTree->BT_BLOCK->NextFreeBlock = USHRT_MAX;
          pBTree->BT_BLOCK->LastFreeBlock = USHRT_MAX;
          pBTree->BT_BLOCK->Type = BT_UPPER;
          pBTree->BT_BLOCK->RecLen = pBTree->BT_HEAD.CI_LENGTH_1;
     	  SetBT_HEAD_FirstFreeBlock(pBTree,0,0);
          pBTree->BT_FSPACE_PNT = (LPBTFSI)IADDR ((LPSTR)&pBTree->BT_BLOCK->BT_DATA,LREC+1);
          pBTree->BT_FSPACE_PNT->BT_FSPACE_LEN =
          		 pBTree->BT_HEAD.BT_BLKSIZE - LREC - 6 - pBTree->BT_HEAD.BT_BLOCK_HEAD_LEN;
          pBTree->BT_FSPACE_PNT->BT_FSPACE_LAST = 0;
          pBTree->BT_FSPACE_PNT->BT_FSPACE_NEXT = -1;
          pBTree->BTID_CUR_POS = pBTree->POS;
          pBTree->BTID_FIRST_CALL  = FALSE;

          /* Insert lower segment */
          pBTree->POS.POSA    = pBTree->BT_HEAD.BT_BLKSIZE;
          pBTree->POS.POSB    = 1;
          BT_GET_BLOCK(pBTree);
          UMDB_WRITE_JOURNAL (pBTree,TRUE);
          pBTree->CI_PNT->LASTCI  = BT_NIL;
          pBTree->CI_PNT->NEXTCI  = BT_NIL;
          pBTree->CI_PNT->UPCI    = 0;
          pBTree->CI_PNT->LNCI    = 1;
          pBTree->CI_PNT->CITYPE  = BT_BOTTOM;
          pBTree->INDEX_ITEM_PNT = (LPBTI)((LPSTR) pBTree->CI_PNT + 12);
          pBTree->INDEX_DATA_PNT = (LPSTR) pBTree->INDEX_ITEM_PNT + (pBTree->BT_HEAD.BT_KEYLEN + 4);
          _fmemmove (&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,KEY,pBTree->BT_HEAD.BT_KYLEN);
          if (pBTree->BT_HEAD.BT_DATED)
     		  _fmemmove (IADDR(&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,pBTree->BT_HEAD.BT_KYLEN+1),
     		             &pBTree->BTID_DATE,2);
          _fmemmove (pBTree->INDEX_DATA_PNT,DATA,pBTree->BT_HEAD.BT_DATLEN);
     	  SetBT_HEAD_FirstFreeBlock(pBTree,1,1);
          pBTree->BT_HEAD.BT_LENGTH = pBTree->BT_HEAD.BT_BLKSIZE + pBTree->BT_HEAD.CI_LENGTH_2 +
          					   pBTree->BT_HEAD.BT_BLOCK_HEAD_LEN;
          pBTree->BT_HEAD.BT_NUMLEVS = 2;
          pBTree->BT_HEAD.BT_NUMRECS = 1;
          LREC = pBTree->BT_HEAD.CI_LENGTH_2;
          pBTree->BT_BLOCK->BT_FSPACE_BEG = LREC + 1;
          pBTree->BT_BLOCK->Dirty = TRUE;
          pBTree->BT_BLOCK->NextFreeBlock = USHRT_MAX;
          pBTree->BT_BLOCK->LastFreeBlock = USHRT_MAX;
          pBTree->BT_BLOCK->Type = BT_BOTTOM;
          pBTree->BT_BLOCK->RecLen = pBTree->BT_HEAD.CI_LENGTH_2;
          pBTree->BT_FSPACE_PNT = (LPBTFSI)IADDR ((LPSTR)&pBTree->BT_BLOCK->BT_DATA,LREC+1);
          pBTree->BT_FSPACE_PNT->BT_FSPACE_LEN =
          		 pBTree->BT_HEAD.BT_BLKSIZE - LREC - 6 - pBTree->BT_HEAD.BT_BLOCK_HEAD_LEN;
          pBTree->BT_FSPACE_PNT->BT_FSPACE_LAST = 0;
          pBTree->BT_FSPACE_PNT->BT_FSPACE_NEXT = -1;
          pBTree->BT_UPDATE = FALSE;
          pBTree->BT_HEAD.BT_MAX_BLOCK++;
          ST = 31;
{
#if ENABLETRACE
GSSiExitProg (499);
#endif
          return(ST);
}
      }

      if (DO_FIND)
          ST = BT_FIND_internal (pBTree,KEY,BT_FIRST,BT_EQ,DATA2,&LASTRC);
      if (!ST)
      {
          if (pBTree->BT_HEAD.BT_DATED)
              if (!_fmemcmp (IADDR(&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,pBTree->BT_HEAD.BT_KYLEN+1),
              			  &pBTree->BTID_DATE,2)) goto S100;

          UMDB_WRITE_JOURNAL (pBTree,TRUE);
          _fmemmove (pBTree->INDEX_DATA_PNT,DATA,pBTree->BT_HEAD.BT_DATLEN);
		  pBTree->BT_UPDATE = FALSE;
{
#if ENABLETRACE
GSSiExitProg (499);
#endif
		  return (ST);
}
	  }

S100:
	  BT_INSERT (pBTree,KEY,DATA,LASTRC);
      pBTree->BT_HEAD.BT_NUMRECS = pBTree->BT_HEAD.BT_NUMRECS + 1;  
      pBTree->BT_UPDATE = FALSE;
{
#if ENABLETRACE
GSSiExitProg (499);
#endif
      return (ST);
}
#if ENABLETRACE
}
#endif
}

void UMDB_WRITE_JOURNAL (LPBTREE pBTree,BOOL INDEX)
#if ENABLETRACE
{GSSiEnterProg (500);
#endif
{
	pBTree->BT_BLOCK->Dirty = TRUE;
{
#if ENABLETRACE
GSSiExitProg (500);
#endif
      return;
}
#if ENABLETRACE
}
#endif
}

void BT_INSERT (LPBTREE pBTree,LPSTR KEY,LPSTR DATA, BOOL LASTRC)
#if ENABLETRACE
{GSSiEnterProg (501);
#endif
{	int		 LN, IB, SPOSB;
	long	 NEWPOS, SAVE_POSA, SNEXT, SUP, SPOSA;
	LPSTR		SAVE_AREA; 
	HANDLE		hSAVE_AREA;
/*      CHARACTER KEY*256, DATA*256
      INTEGER*4 LN, SAVE_POS, SPOSA, NEWPOS, FREE_BT_REC, SNEXT, SUP
      INTEGER*2 IB, SPOSB
      LOGICAL   LASTRC*/

      if (LASTRC)
      {
          if (pBTree->POS.POSB <= pBTree->BT_HEAD.CI_MAX_DNUM)
          {
             BT_GET_BLOCK (pBTree);
             goto S100;
          }
          else
             goto S110;
      }
S10:  if (pBTree->CI_PNT->LNCI < pBTree->BT_HEAD.CI_MAX_DNUM)
	  {
          UMDB_WRITE_JOURNAL (pBTree,TRUE);
          IB = (pBTree->POS.POSB-1) * pBTree->BT_HEAD.BT_RECLEN + 1;
          LN = (pBTree->CI_PNT->LNCI - (pBTree->POS.POSB-1)) * pBTree->BT_HEAD.BT_RECLEN;
          if (LN > 0) _fmemmove (IADDR((LPSTR)&pBTree->CI_PNT->CIDATA,IB+pBTree->BT_HEAD.BT_RECLEN),
          						 IADDR((LPSTR)&pBTree->CI_PNT->CIDATA,IB),LN);
S100:     UMDB_WRITE_JOURNAL (pBTree,TRUE);
          pBTree->CI_PNT->LNCI += 1;
          _fmemmove (&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,KEY,pBTree->BT_HEAD.BT_KYLEN);
          if (pBTree->BT_HEAD.BT_DATED)
     		  _fmemmove (IADDR(&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,pBTree->BT_HEAD.BT_KYLEN+1),
     		  			 &pBTree->BTID_DATE,2);
          _fmemmove (pBTree->INDEX_DATA_PNT,DATA,pBTree->BT_HEAD.BT_DATLEN);
          pBTree->BTID_CUR_POS = pBTree->POS;
          if (pBTree->POS.POSB == 1) PROPAGATE_BT_UP(pBTree,KEY,(LPSTR)&pBTree->BTID_DATE);
       }
       else
       {
          if (pBTree->POS.POSB > pBTree->CI_PNT->LNCI)
          {
              if (pBTree->CI_PNT->NEXTCI == BT_NIL) goto S110;
              pBTree->POS.POSA = pBTree->CI_PNT->NEXTCI;
              pBTree->POS.POSB = 1;
              BT_GET_BLOCK(pBTree);
              goto S10;
          }
S110:     if (pBTree->POS.POSB == 1 && pBTree->CI_PNT->LASTCI != BT_NIL)
	      {
              SAVE_POSA = pBTree->POS.POSA;
              pBTree->POS.POSA     = pBTree->CI_PNT->LASTCI;
              BT_GET_BLOCK(pBTree);
              if (pBTree->CI_PNT->LNCI < pBTree->BT_HEAD.CI_MAX_DNUM)
              {
                  IB = pBTree->CI_PNT->LNCI * pBTree->BT_HEAD.BT_RECLEN + 1;
                  pBTree->POS.POSB = pBTree->CI_PNT->LNCI + 1;
                  BT_GET_BLOCK(pBTree);
                  goto S100;
              }
              else
              {
                  pBTree->POS.POSA = SAVE_POSA;
                  BT_GET_BLOCK(pBTree);
              }
          }
          UMDB_WRITE_JOURNAL (pBTree,TRUE);
          pBTree->CI_PNT->LNCI = pBTree->BT_HEAD.SPLIT_DAT_1;
          IB     = pBTree->BT_HEAD.SPLIT_DAT_1 * pBTree->BT_HEAD.BT_RECLEN + 1;
          LN     = pBTree->BT_HEAD.SPLIT_DAT_2 * pBTree->BT_HEAD.BT_RECLEN; 
          hSAVE_AREA = GSSiGlobAlloc ( 109,GMEM_MOVEABLE,pBTree->BT_HEAD.CI_LENGTH_2);
          SAVE_AREA = GlobalLock (hSAVE_AREA);
          _fmemmove (SAVE_AREA,IADDR((LPSTR)&pBTree->CI_PNT->CIDATA,IB),(UINT)LN);
          SNEXT  = pBTree->CI_PNT->NEXTCI;
          SUP    = pBTree->CI_PNT->UPCI;
          pBTree->CS_PNT = FREE_BT_REC (pBTree,pBTree->BT_HEAD.CI_LENGTH_2,&NEWPOS,BT_BOTTOM);
          _fmemmove ((LPSTR)&pBTree->CS_PNT->CIDATA,SAVE_AREA,(UINT)LN);
          GSSiGlobUlFree (&hSAVE_AREA);
          pBTree->CS_PNT->LASTCI = pBTree->POS.POSA;
          pBTree->CS_PNT->NEXTCI = SNEXT;
          pBTree->CS_PNT->UPCI   = SUP;
          pBTree->CS_PNT->LNCI   = pBTree->BT_HEAD.SPLIT_DAT_2;
          pBTree->CS_PNT->CITYPE = BT_BOTTOM;
          SPOSA  = pBTree->POS.POSA;
          if (SNEXT != BT_NIL)
          {
              pBTree->POS.POSA   = SNEXT;
              BT_GET_BLOCK(pBTree);
              UMDB_WRITE_JOURNAL (pBTree,TRUE);
              pBTree->CI_PNT->LASTCI = NEWPOS;
          }
          pBTree->POS.POSA   = SPOSA;
          BT_GET_BLOCK(pBTree);
          UMDB_WRITE_JOURNAL (pBTree,TRUE);
          pBTree->CI_PNT->NEXTCI = NEWPOS;
          if (pBTree->POS.POSB <= pBTree->BT_HEAD.SPLIT_DAT_1)
          {
              SPOSA  = pBTree->POS.POSA;
              SPOSB  = pBTree->POS.POSB;
          }
          else
          {
              SPOSA  = NEWPOS;
              SPOSB  = pBTree->POS.POSB - pBTree->BT_HEAD.SPLIT_DAT_1;
          }
          pBTree->POS.POSA   = NEWPOS;
          pBTree->POS.POSB   = 1;
          BT_GET_BLOCK(pBTree);
          PROPAGATE_SPLIT(pBTree);
          pBTree->POS.POSA   = SPOSA;
          pBTree->POS.POSB   = SPOSB;
          BT_GET_BLOCK(pBTree);
          goto S10;
      }

{
#if ENABLETRACE
GSSiExitProg (501);
#endif
      return;
}
#if ENABLETRACE
}
#endif
}

LPCB FREE_BT_REC (LPBTREE pBTree,int LREC,long *LOC, int Type)
#if ENABLETRACE
{GSSiEnterProg (502);
#endif
{ 	  long		I, LAST_BLOCK;
	  int		LAST, NEXT, LENS;
	  LPCB		pFREE_BT_REC;

      pBTree->SAVE_POS = pBTree->POS;
      if (Type != pBTree->BT_BLOCK->Type)
		  goto S100;

S5:   I = pBTree->BT_BLOCK->BT_FSPACE_BEG;
      LAST_BLOCK = pBTree->BT_BLOCK_NUM;
S10:      if (I < 0) goto S100;
          pBTree->BT_FSPACE_PNT = (LPBTFSI)IADDR ((LPSTR)&pBTree->BT_BLOCK->BT_DATA,I);
          if (pBTree->BT_FSPACE_PNT->BT_FSPACE_LEN >= LREC)
          {
              pFREE_BT_REC = (LPCB)pBTree->BT_FSPACE_PNT;
              LAST = pBTree->BT_FSPACE_PNT->BT_FSPACE_LAST;
              NEXT = pBTree->BT_FSPACE_PNT->BT_FSPACE_NEXT;
              LENS = pBTree->BT_FSPACE_PNT->BT_FSPACE_LEN - LREC;
              if (!LAST)
                  pBTree->BT_FSPACE_PNT = (LPBTFSI)&pBTree->BT_BLOCK->BT_FSPACE_BEG;
              else
                  pBTree->BT_FSPACE_PNT = (LPBTFSI)IADDR ((LPSTR)&pBTree->BT_BLOCK->BT_DATA,LAST);
              UMDB_WRITE_JOURNAL (pBTree,TRUE);
              if (LENS >= pBTree->BT_BLOCK->RecLen)
              {
                  pBTree->BT_FSPACE_PNT->BT_FSPACE_NEXT = (short)(I + LREC);
                  pBTree->BT_FSPACE_PNT = (LPBTFSI)IADDR ((LPSTR)&pBTree->BT_BLOCK->BT_DATA,pBTree->BT_FSPACE_PNT->BT_FSPACE_NEXT);
                  pBTree->BT_FSPACE_PNT->BT_FSPACE_NEXT = NEXT;
                  pBTree->BT_FSPACE_PNT->BT_FSPACE_LAST = LAST;
                  pBTree->BT_FSPACE_PNT->BT_FSPACE_LEN  = LENS;
               }
               else
               {
                  pBTree->BT_FSPACE_PNT->BT_FSPACE_NEXT = NEXT;
                  if (NEXT > 0)
                  {
                      pBTree->BT_FSPACE_PNT = (LPBTFSI)IADDR ((LPSTR)&pBTree->BT_BLOCK->BT_DATA,NEXT);
                      pBTree->BT_FSPACE_PNT->BT_FSPACE_LAST = LAST;
                  }
               }
               goto S200;
          }
          I = pBTree->BT_FSPACE_PNT->BT_FSPACE_NEXT;
          goto S10;

S100: pBTree->POS.POSA = GetBT_HEAD_FirstFreeBlock(pBTree,Type-1)*pBTree->BT_HEAD.BT_BLKSIZE;
      BT_GET_BLOCK(pBTree);
      if (pBTree->BT_BLOCK->BT_FSPACE_BEG >= 0 && pBTree->BT_BLOCK_NUM != LAST_BLOCK)
		  goto S5;
      /* Create new block */
      pBTree->BT_HEAD.BT_MAX_BLOCK++;
      pBTree->POS.POSA = pBTree->BT_HEAD.BT_MAX_BLOCK * pBTree->BT_HEAD.BT_BLKSIZE;
      pBTree->POS.POSB = 1;
      BT_GET_BLOCK(pBTree);
      UMDB_WRITE_JOURNAL (pBTree,TRUE);
      pBTree->BT_BLOCK->BT_FSPACE_BEG = LREC + 1;
      pBTree->BT_FSPACE_PNT = (LPBTFSI)IADDR ((LPSTR)&pBTree->BT_BLOCK->BT_DATA,(long)pBTree->BT_BLOCK->BT_FSPACE_BEG);
      pBTree->BT_FSPACE_PNT->BT_FSPACE_LEN = pBTree->BT_HEAD.BT_BLKSIZE - LREC - 6 -pBTree->BT_HEAD.BT_BLOCK_HEAD_LEN;
      pBTree->BT_FSPACE_PNT->BT_FSPACE_LAST = 0;
      pBTree->BT_FSPACE_PNT->BT_FSPACE_NEXT = -1;
      pFREE_BT_REC =(LPCB) &pBTree->BT_BLOCK->BT_DATA;
      pBTree->BT_HEAD.BT_LENGTH = pBTree->BT_BLOCK_NUM * pBTree->BT_HEAD.BT_BLKSIZE +
      					   LREC + pBTree->BT_HEAD.BT_BLOCK_HEAD_LEN + sizeof (BTFREESPACEITEM);
 	  pBTree->BT_BLOCK->Dirty = TRUE;
      pBTree->BT_BLOCK->NextFreeBlock = USHRT_MAX;
      pBTree->BT_BLOCK->LastFreeBlock = USHRT_MAX;
      pBTree->BT_BLOCK->Type = Type;
      if (Type == BT_UPPER)
      	pBTree->BT_BLOCK->RecLen = pBTree->BT_HEAD.CI_LENGTH_1;
      else
      	pBTree->BT_BLOCK->RecLen = pBTree->BT_HEAD.CI_LENGTH_2; 
      if (pBTree->BT_BLOCK_NUM > (long)USHRT_MAX -1 && !pBTree->BT_HEAD.LongFreeBlock) 
      		BlowOut ("B-Tree Limit Reached", pBTree->BT_FNAME);
      SetBT_HEAD_FirstFreeBlock(pBTree,Type-1,pBTree->BT_BLOCK_NUM);
      I = 1;
S200: *LOC = pBTree->BT_BLOCK_NUM * pBTree->BT_HEAD.BT_BLKSIZE + I - 1;
      if (*LOC >= pBTree->BT_HEAD.BT_LENGTH) pBTree->BT_HEAD.BT_LENGTH =
      		 *LOC + LREC + pBTree->BT_HEAD.BT_BLOCK_HEAD_LEN;
      pBTree->POS = pBTree->SAVE_POS;
{
#if ENABLETRACE
GSSiExitProg (502);
#endif
      return (pFREE_BT_REC);
}
#if ENABLETRACE
}
#endif
}

void PROPAGATE_BT_UP (LPBTREE pBTree,LPSTR KEY,LPSTR DATE)
#if ENABLETRACE
{GSSiEnterProg (503);
#endif
{	  long I;
/*      CHARACTER KEY*256, SAVE_POS*6, DATE*2
      INTEGER*4 I
*/
      pBTree->SAVE_POS = pBTree->POS;
      pBTree->BT_PATH_CHANGE = TRUE;
S10:  if (pBTree->CI_PNT->UPCI == BT_NIL) goto S20;
          I = pBTree->POS.POSA;
          pBTree->POS.POSA = pBTree->CI_PNT->UPCI;
          BT_GET_BLOCK(pBTree);
          while (pBTree->INDEX_ITEM_PNT->BT_DOWN != I)
          {   pBTree->INDEX_ITEM_PNT = (LPBTI)((LPSTR)pBTree->INDEX_ITEM_PNT + pBTree->BT_HEAD.BT_PNTLEN);
              pBTree->POS.POSB += 1;
              if (pBTree->POS.POSB > pBTree->CI_PNT->LNCI)
				BTreeErrorMessage (pBTree,"Invalid POSB(3)");
          }
          UMDB_WRITE_JOURNAL (pBTree,TRUE);
          _fmemmove (&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,KEY,pBTree->BT_HEAD.BT_KYLEN);
          if (pBTree->BT_HEAD.BT_DATED)
     		  _fmemmove (IADDR(&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,pBTree->BT_HEAD.BT_KYLEN+1),
     		  			 DATE,2);
          if (pBTree->POS.POSB != 1)
			  goto S20;
          goto S10;
S20:  pBTree->POS = pBTree->SAVE_POS;
{
#if ENABLETRACE
GSSiExitProg (503);
#endif
	  return;
}
#if ENABLETRACE
}
#endif
}

void PROPAGATE_SPLIT(LPBTREE pBTree)
#if ENABLETRACE
{GSSiEnterProg (504);
#endif
{	long	I, SPLIT_ADD, NEW_POSA, SAVE_POSA;
	long	NEWPOS, SUP, SNEXT, SPOSA, LN;
	int		IB;
	LPSTR		SAVE_AREA; 
	HANDLE		hSAVE_AREA;
/*      CHARACTER KEY*256
      INTEGER*4 I, FREE_BT_REC
      INTEGER*4 LN, SPLIT_ADD, NEW_POS, SAVE_POS, NEWPOS, SUP, SNEXT,
     +          SPOSA
      INTEGER*2 IB, SPLIT_POS
*/
      pBTree->BT_PATH_CHANGE = TRUE;

S10:  if (pBTree->CI_PNT->UPCI == BT_NIL)
	  {
          UMDB_WRITE_JOURNAL (pBTree,TRUE);
          hSAVE_AREA = GSSiGlobAlloc ( 110,GMEM_MOVEABLE,(pBTree->BT_HEAD.BT_PNTLEN-4));
          SAVE_AREA = GlobalLock (hSAVE_AREA);
          _fmemmove (SAVE_AREA,
          			 (LPSTR)&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,
          			 (UINT)(pBTree->BT_HEAD.BT_PNTLEN-4));
          pBTree->CS_PNT = FREE_BT_REC (pBTree,pBTree->BT_HEAD.CI_LENGTH_1,&I,BT_UPPER);
          pBTree->BT_HEAD.BT_FIRST_POS = I;
          pBTree->CS_PNT->LASTCI = BT_NIL;
          pBTree->CS_PNT->NEXTCI = BT_NIL;
          pBTree->CS_PNT->UPCI   = BT_NIL;
          pBTree->CS_PNT->LNCI   = 2;
          pBTree->CS_PNT->CITYPE = BT_UPPER;
          _fmemmove(IADDR(&pBTree->CS_PNT->CIDATA,pBTree->BT_HEAD.BT_PNTLEN+1),&pBTree->POS.POSA,4);
          _fmemmove(IADDR(&pBTree->CS_PNT->CIDATA,pBTree->BT_HEAD.BT_PNTLEN+5),SAVE_AREA,(pBTree->BT_HEAD.BT_PNTLEN-4));
          BT_GET_BLOCK(pBTree);
          UMDB_WRITE_JOURNAL (pBTree,TRUE);
          pBTree->CI_PNT->UPCI   = I;
          pBTree->POS.POSA   = pBTree->CI_PNT->LASTCI;
          SAVE_POSA  = pBTree->POS.POSA;
          BT_GET_BLOCK(pBTree);
          UMDB_WRITE_JOURNAL (pBTree,TRUE);
          pBTree->CI_PNT->UPCI   = I;
          _fmemmove(SAVE_AREA,&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,(pBTree->BT_HEAD.BT_PNTLEN-4));
          pBTree->POS.POSA   = I;
          BT_GET_BLOCK(pBTree);
          UMDB_WRITE_JOURNAL (pBTree,TRUE);
          _fmemmove(&pBTree->CS_PNT->CIDATA,&SAVE_POSA,4);
          _fmemmove(IADDR(&pBTree->CS_PNT->CIDATA,5),SAVE_AREA,(pBTree->BT_HEAD.BT_PNTLEN-4)); 
          GSSiGlobUlFree (&hSAVE_AREA);
          
          pBTree->BT_HEAD.BT_NUMLEVS++; 
          if (pBTree->BT_HEAD.BT_NUMLEVS > MAX_BTREE_LEVELS)
				BTreeErrorMessage (pBTree,"Max levels exceeded");
          	
{
#if ENABLETRACE
GSSiExitProg (504);
#endif
          return;
}
      }

      NEW_POSA   = pBTree->POS.POSA;
      SPLIT_ADD = pBTree->CI_PNT->LASTCI;
      _fmemmove(&SAVEKEY,&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,(pBTree->BT_HEAD.BT_PNTLEN-4));
      pBTree->POS.POSA      = pBTree->CI_PNT->UPCI;
      pBTree->POS.POSB      = 1;
      BT_GET_BLOCK(pBTree);

      while (SPLIT_ADD != pBTree->INDEX_ITEM_PNT->BT_DOWN)
      {   pBTree->INDEX_ITEM_PNT = (LPBTI)((LPSTR)pBTree->INDEX_ITEM_PNT + pBTree->BT_HEAD.BT_PNTLEN);
          pBTree->POS.POSB = pBTree->POS.POSB + 1;

              if (pBTree->POS.POSB > pBTree->CI_PNT->LNCI)
				BTreeErrorMessage (pBTree,"Invalid POSB(4)");
      }

      if (pBTree->CI_PNT->LNCI < pBTree->BT_HEAD.CI_MAX_PNUM)
      {
          UMDB_WRITE_JOURNAL (pBTree,TRUE);
          IB = pBTree->POS.POSB * pBTree->BT_HEAD.BT_PNTLEN + 1;
          LN = (pBTree->CI_PNT->LNCI - pBTree->POS.POSB) * pBTree->BT_HEAD.BT_PNTLEN;
S100:     if (LN > 0)
			  _fmemmove(IADDR(&pBTree->CI_PNT->CIDATA,IB+pBTree->BT_HEAD.BT_PNTLEN),
								IADDR(&pBTree->CI_PNT->CIDATA,IB),(size_t)LN);
          pBTree->CI_PNT->LNCI++;
          pBTree->INDEX_ITEM_PNT = (LPBTI)((LPSTR)pBTree->INDEX_ITEM_PNT + pBTree->BT_HEAD.BT_PNTLEN);
          pBTree->POS.POSB++;
          _fmemmove(&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY, &SAVEKEY,(pBTree->BT_HEAD.BT_PNTLEN-4));
          pBTree->INDEX_ITEM_PNT->BT_DOWN = NEW_POSA;
          if (pBTree->POS.POSB == 1)
              PROPAGATE_BT_UP (pBTree,(LPSTR)&SAVEKEY,IADDR(SAVEKEY,pBTree->BT_HEAD.BT_KYLEN+1));
{
#if ENABLETRACE
GSSiExitProg (504);
#endif
          return;
}
      }
      else
      {
          if (pBTree->POS.POSB == pBTree->BT_HEAD.CI_MAX_PNUM && pBTree->CI_PNT->NEXTCI != BT_NIL)
          {
              SAVE_POSA = pBTree->POS.POSA;
              pBTree->POS.POSA  = pBTree->CI_PNT->NEXTCI;
              BT_GET_BLOCK(pBTree);
              if (pBTree->CI_PNT->LNCI < pBTree->BT_HEAD.CI_MAX_PNUM)
              {
                  IB = 1;
                  LN = pBTree->CI_PNT->LNCI * pBTree->BT_HEAD.BT_PNTLEN;
                  SAVE_POSA = pBTree->POS.POSA;
                  pBTree->POS.POSA = NEW_POSA;
                  BT_GET_BLOCK(pBTree);
                  UMDB_WRITE_JOURNAL (pBTree,TRUE);
                  pBTree->CI_PNT->UPCI = SAVE_POSA;
                  pBTree->POS.POSA = SAVE_POSA;
                  pBTree->POS.POSB = 0;
                  BT_GET_BLOCK(pBTree);
                  UMDB_WRITE_JOURNAL (pBTree,TRUE);
                  goto S100;
              }
              else
              {
                  pBTree->POS.POSA = SAVE_POSA;
                  BT_GET_BLOCK(pBTree);
              }
          }
          UMDB_WRITE_JOURNAL (pBTree,TRUE);
          pBTree->CI_PNT->LNCI   = pBTree->BT_HEAD.SPLIT_PNT_1;
          IB     = pBTree->BT_HEAD.SPLIT_PNT_1 * pBTree->BT_HEAD.BT_PNTLEN + 1;
          LN     = pBTree->BT_HEAD.SPLIT_PNT_2 * pBTree->BT_HEAD.BT_PNTLEN;
          hSAVE_AREA = GSSiGlobAlloc ( 111,GMEM_MOVEABLE,pBTree->BT_HEAD.CI_LENGTH_2);
          SAVE_AREA = GlobalLock (hSAVE_AREA);
          _fmemmove(SAVE_AREA,IADDR(&pBTree->CI_PNT->CIDATA,IB),(size_t)LN);
          SNEXT  = pBTree->CI_PNT->NEXTCI;
          SUP    = pBTree->CI_PNT->UPCI;
          pBTree->CS_PNT = FREE_BT_REC (pBTree,pBTree->BT_HEAD.CI_LENGTH_1,&NEWPOS,BT_UPPER);
          _fmemmove(&pBTree->CS_PNT->CIDATA,SAVE_AREA,(size_t)LN);
          GlobalUnlock (hSAVE_AREA);
          GSSiGlobFree (&hSAVE_AREA);
          pBTree->CS_PNT->LASTCI = pBTree->POS.POSA;
          pBTree->CS_PNT->NEXTCI = SNEXT;
          pBTree->CS_PNT->UPCI   = SUP;
          pBTree->CS_PNT->LNCI   = pBTree->BT_HEAD.SPLIT_PNT_2;
          pBTree->CS_PNT->CITYPE = BT_UPPER;
          SPOSA  = pBTree->POS.POSA;
          if (SNEXT != BT_NIL)
          {
              pBTree->POS.POSA   = SNEXT;
              BT_GET_BLOCK(pBTree);
              UMDB_WRITE_JOURNAL (pBTree,TRUE);
              pBTree->CI_PNT->LASTCI = NEWPOS;
          }
          pBTree->POS.POSA   = SPOSA;
          BT_GET_BLOCK(pBTree);
          UMDB_WRITE_JOURNAL (pBTree,TRUE);
          pBTree->CI_PNT->NEXTCI = NEWPOS;
          if (pBTree->POS.POSB > pBTree->BT_HEAD.SPLIT_PNT_1)
          {
              pBTree->POS.POSB = pBTree->POS.POSB - pBTree->BT_HEAD.SPLIT_PNT_1;
              pBTree->POS.POSA = NEWPOS;
              BT_GET_BLOCK(pBTree);
          }
          UMDB_WRITE_JOURNAL (pBTree,TRUE);
          IB = pBTree->POS.POSB * pBTree->BT_HEAD.BT_PNTLEN + 1;
          LN = (pBTree->CI_PNT->LNCI - pBTree->POS.POSB) * pBTree->BT_HEAD.BT_PNTLEN;
          if (LN > 0) _fmemmove(IADDR(&pBTree->CI_PNT->CIDATA,IB+pBTree->BT_HEAD.BT_PNTLEN),
          						IADDR(&pBTree->CI_PNT->CIDATA,IB),(size_t)LN);
          pBTree->CI_PNT->LNCI++;
          pBTree->INDEX_ITEM_PNT =(LPBTI) ((LPSTR)pBTree->INDEX_ITEM_PNT + pBTree->BT_HEAD.BT_PNTLEN);
          pBTree->POS.POSB++;
          _fmemmove(&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,&SAVEKEY,(pBTree->BT_HEAD.BT_PNTLEN-4));
          pBTree->INDEX_ITEM_PNT->BT_DOWN = NEW_POSA;
          pBTree->POS.POSA    = NEWPOS;
          pBTree->POS.POSB    = 1;
          BT_GET_BLOCK(pBTree);
          PROPAGATE_BT_DOWN(pBTree);
          pBTree->POS.POSA    = NEWPOS;
          pBTree->POS.POSB    = 1;
          NEW_POSA = NEWPOS;
          BT_GET_BLOCK(pBTree);
          goto S10;
      }
#if ENABLETRACE
}
#endif
}

void PROPAGATE_BT_DOWN(LPBTREE pBTree)
#if ENABLETRACE
{GSSiEnterProg (505);
#endif
{	long	J[32], I;
	int		L, N;

      I = pBTree->POS.POSA;
      N = pBTree->CI_PNT->LNCI;

      for (L = 0;L < N; L++)
      {
          J[L] = pBTree->INDEX_ITEM_PNT->BT_DOWN;
          pBTree->INDEX_ITEM_PNT = (LPBTI)((LPSTR)pBTree->INDEX_ITEM_PNT + pBTree->BT_HEAD.BT_PNTLEN);
      }

      for (L = 0;L < N; L++)
      {
          pBTree->POS.POSA = J[L];
          BT_GET_BLOCK(pBTree);
          UMDB_WRITE_JOURNAL (pBTree,TRUE);
          pBTree->CI_PNT->UPCI = I;
      }

{
#if ENABLETRACE
GSSiExitProg (505);
#endif
      return;
}
#if ENABLETRACE
}
#endif
}

/*void ShowBTree (HGLOBAL hBT,HWND hWnd,int DlgItem,int iPOSB)
#if ENABLETRACE
{GSSiEnterProg (506);
#endif
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
	retrn;
#if ENABLETRACE
}
#endif
}
*/

int BT_DELETE (HGLOBAL hBTree,LPSTR KEY,LPSTR DATA,BOOL SECIDX)
#if ENABLETRACE
{GSSiEnterProg (507);
#endif
{
	int irc;
	LPBTREE	pBTree;

    if (!(pBTree = AllocateBTMem(hBTree)))
{
#if ENABLETRACE
GSSiExitProg (507);
#endif
	  	return 1;
}
	  
	  
      irc = BT_DELETE_internal (pBTree,KEY,DATA,SECIDX);
      DeallocateBTMem(pBTree,hBTree);
{
#if ENABLETRACE
GSSiExitProg (507);
#endif
      return (irc);
}
#if ENABLETRACE
}
#endif
}

int BT_DELETE_internal(LPBTREE pBTree,LPSTR KEY,LPSTR DATA,BOOL SECIDX) 
#if ENABLETRACE
{GSSiEnterProg (508);
#endif
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
      if (SECIDX)
		  pBTree->BT_UPDATE = TRUE;
      ST = BT_FIND_internal (pBTree,KEY,BT_FIRST,BT_EQ,DATA,&FRST); 
      
      if (pBTree->BT_HEAD.BT_DATED && ST == 0)
      {
		if (_fmemcmp(IADDR((LPSTR)&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,pBTree->BT_HEAD.BT_KYLEN+1),
                      &pBTree->BTID_DATE,2) != 0) ST = 1;
      }
      if (ST != 0)
      {
          pBTree->BTID_FIRST_CALL  = TRUE;
          goto S1000;
      }
      pBTree->BT_HEAD.BT_NUMRECS = pBTree->BT_HEAD.BT_NUMRECS - 1;
      if (pBTree->POS.POSB > 1)
      {
          UMDB_WRITE_JOURNAL (pBTree,TRUE);
          if (pBTree->POS.POSB == pBTree->CI_PNT->LNCI)
          {
              pBTree->CI_PNT->LNCI = pBTree->CI_PNT->LNCI - 1;
              pBTree->POS.POSB = pBTree->CI_PNT->LNCI;
              pBTree->BTID_CUR_POS = pBTree->POS;
              goto S1000;
          }
          LN = (pBTree->CI_PNT->LNCI - pBTree->POS.POSB) * pBTree->BT_HEAD.BT_RECLEN;
          IB = pBTree->POS.POSB * pBTree->BT_HEAD.BT_RECLEN + 1;
          _fmemmove(IADDR((LPSTR)&pBTree->CI_PNT->CIDATA,IB-pBTree->BT_HEAD.BT_RECLEN),IADDR(&pBTree->CI_PNT->CIDATA,IB),(size_t)LN);
          pBTree->CI_PNT->LNCI = pBTree->CI_PNT->LNCI - 1;
          pBTree->POS.POSB = pBTree->POS.POSB - 1;
          pBTree->BTID_CUR_POS = pBTree->POS;
          goto S1000;
      }
//******* REMOVING FIRST ITEM IN BLOCK CONTAINING MORE THAN 1 ITEM;
      if (pBTree->CI_PNT->LNCI > 1)
      {
	      LPSTR	PROP_KEY; 
	      HANDLE hPROP_KEY;
	      
	 	  hPROP_KEY = GSSiGlobAlloc ( 112,GMEM_MOVEABLE,256);
	 	  PROP_KEY = GlobalLock (hPROP_KEY);
          UMDB_WRITE_JOURNAL (pBTree,TRUE);
          pBTree->CI_PNT->LNCI = pBTree->CI_PNT->LNCI - 1;
          LN = pBTree->CI_PNT->LNCI * pBTree->BT_HEAD.BT_RECLEN;
          IB = pBTree->BT_HEAD.BT_RECLEN + 1;
          _fmemmove(&pBTree->CI_PNT->CIDATA,IADDR(&pBTree->CI_PNT->CIDATA,IB),(size_t)LN);
          _fmemmove(PROP_KEY,&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,pBTree->BT_HEAD.BT_KEYLEN);
          SAVE_POS = pBTree->POS;
          PROPAGATE_BT_UP (pBTree,PROP_KEY,IADDR(PROP_KEY,pBTree->BT_HEAD.BT_KYLEN+1));
		  GlobalUnlock(hPROP_KEY);
		  GSSiGlobFree (&hPROP_KEY);
          pBTree->POS  = SAVE_POS;
          pBTree->POS.POSB = pBTree->POS.POSB - 1;
          pBTree->BTID_CUR_POS = pBTree->POS;
          goto S1000;
      }
//******* REMOVING ONLY ITEM IN BLOCK;
      SAVE_POS = pBTree->POS;
      NEXT     = pBTree->CI_PNT->NEXTCI;
      LAST     = pBTree->CI_PNT->LASTCI;
      if (LAST != BT_NIL)
      {
          pBTree->POS.POSA = LAST;
          BT_GET_BLOCK(pBTree);
          UMDB_WRITE_JOURNAL (pBTree,TRUE);
          pBTree->CI_PNT->NEXTCI = NEXT;
          pBTree->POS.POSB = pBTree->CI_PNT->LNCI;
          pBTree->BTID_CUR_POS = pBTree->POS;
      }
      else
          pBTree->BTID_FIRST_CALL  = TRUE;
      if (NEXT != BT_NIL)
      {
          pBTree->POS.POSA = NEXT;
          BT_GET_BLOCK(pBTree);
          UMDB_WRITE_JOURNAL (pBTree,TRUE);
          pBTree->CI_PNT->LASTCI = LAST;
      }
      pBTree->POS     = SAVE_POS;
      BT_GET_BLOCK(pBTree);
      SAVE_UP = pBTree->CI_PNT->UPCI;
      FREE_BT_REC_ADD (pBTree,pBTree->BT_HEAD.CI_LENGTH_2);
      pBTree->POS     = SAVE_POS;
      PROPAGATE_DELETE_UP (pBTree,SAVE_UP);
S1000:pBTree->BT_UPDATE = FALSE; 
{
#if ENABLETRACE
GSSiExitProg (508);
#endif
      return ST;
}
#if ENABLETRACE
}
#endif
}

void PROPAGATE_DELETE_UP (LPBTREE pBTree,long UP)
#if ENABLETRACE
{GSSiEnterProg (509);
#endif
{     
	  
      long  LN, DELETE_POS, NEXT, LAST;
      int	IB;
	  BTPOS	SAVE_POS;
      

      pBTree->BT_PATH_CHANGE = TRUE;

S10:  DELETE_POS = pBTree->POS.POSA;
      if (UP == BT_NIL)
      {
          pBTree->BT_HEAD.BT_LENGTH    = 0;
          pBTree->BT_HEAD.BT_FIRST_POS = 0;
          pBTree->BT_HEAD.BT_NUMLEVS = 0;
          /*_chsize (pBTree->BtFid,BT_HEAD->BT_HEADLEN + BT_HEAD->BT_LENGTH + 32);*/
{
#if ENABLETRACE
GSSiExitProg (509);
#endif
          return;
}
      }
      pBTree->POS.POSA      = UP;
      pBTree->POS.POSB      = 1;
      BT_GET_BLOCK(pBTree);

      while (DELETE_POS != pBTree->INDEX_ITEM_PNT->BT_DOWN)
      {
          pBTree->INDEX_ITEM_PNT = (LPBTI)((LPSTR) pBTree->INDEX_ITEM_PNT + pBTree->BT_HEAD.BT_PNTLEN);
          pBTree->POS.POSB = pBTree->POS.POSB + 1;
		  if (pBTree->POS.POSB > pBTree->CI_PNT->LNCI)
			BTreeErrorMessage (pBTree,"Invalid POSB(5)");
      }

      if (pBTree->POS.POSB > 1)
      {
          UMDB_WRITE_JOURNAL (pBTree,TRUE);
          if (pBTree->POS.POSB == pBTree->CI_PNT->LNCI)
          {
              pBTree->CI_PNT->LNCI = pBTree->CI_PNT->LNCI - 1;
{
#if ENABLETRACE
GSSiExitProg (509);
#endif
              return;
}
          }
          LN = (pBTree->CI_PNT->LNCI - pBTree->POS.POSB) * pBTree->BT_HEAD.BT_PNTLEN;
          IB = pBTree->POS.POSB * pBTree->BT_HEAD.BT_PNTLEN + 1;
          _fmemmove(IADDR(&pBTree->CI_PNT->CIDATA,IB-pBTree->BT_HEAD.BT_PNTLEN),IADDR(&pBTree->CI_PNT->CIDATA,IB),(size_t)LN);
          pBTree->CI_PNT->LNCI = pBTree->CI_PNT->LNCI - 1;
{
#if ENABLETRACE
GSSiExitProg (509);
#endif
          return;
}
      }
      if (pBTree->CI_PNT->LNCI > 1)
      {   LPSTR KEY;
      	  HANDLE hKEY;
      	  
      	  hKEY = GSSiGlobAlloc ( 113,GMEM_MOVEABLE,256);
      	  KEY = GlobalLock(hKEY);
          UMDB_WRITE_JOURNAL (pBTree,TRUE);
          LN = pBTree->CI_PNT->LNCI * pBTree->BT_HEAD.BT_PNTLEN;
          IB = pBTree->BT_HEAD.BT_PNTLEN + 1;
          _fmemmove(&pBTree->CI_PNT->CIDATA,IADDR(&pBTree->CI_PNT->CIDATA,IB),(size_t)LN);
          pBTree->CI_PNT->LNCI = pBTree->CI_PNT->LNCI - 1;
          _fmemmove(KEY,&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,pBTree->BT_HEAD.BT_KEYLEN);
          PROPAGATE_BT_UP (pBTree,KEY,IADDR(KEY,pBTree->BT_HEAD.BT_KYLEN+1));
          GSSiGlobUlFree (&hKEY);
{
#if ENABLETRACE
GSSiExitProg (509);
#endif
          return;
}
      }
      SAVE_POS = pBTree->POS;
      NEXT     = pBTree->CI_PNT->NEXTCI;
      LAST     = pBTree->CI_PNT->LASTCI;
      if (LAST != BT_NIL)
      {
          pBTree->POS.POSA = LAST;
          BT_GET_BLOCK(pBTree);
          UMDB_WRITE_JOURNAL (pBTree,TRUE);
          pBTree->CI_PNT->NEXTCI = NEXT;
      }
      if (NEXT != BT_NIL)
      {
          pBTree->POS.POSA = NEXT;
          BT_GET_BLOCK(pBTree);
          UMDB_WRITE_JOURNAL (pBTree,TRUE);
          pBTree->CI_PNT->LASTCI = LAST;
      }
      pBTree->POS = SAVE_POS;
      BT_GET_BLOCK(pBTree);
      UP  = pBTree->CI_PNT->UPCI;
      FREE_BT_REC_ADD (pBTree,pBTree->BT_HEAD.CI_LENGTH_1);
      pBTree->POS = SAVE_POS;
      goto S10;
#if ENABLETRACE
}
#endif
}

void FREE_BT_REC_ADD (LPBTREE pBTree,int LENGTH)
#if ENABLETRACE
{GSSiEnterProg (510);
#endif
{
      int NEXT;

      UMDB_WRITE_JOURNAL (pBTree,TRUE);
      NEXT           = pBTree->BT_BLOCK->BT_FSPACE_BEG;
      pBTree->BT_BLOCK->BT_FSPACE_BEG  = (short)pBTree->BT_BLOCK_POS;
      pBTree->BT_FSPACE_PNT  = (LPBTFSI)IADDR ((LPSTR)&pBTree->BT_BLOCK->BT_DATA,pBTree->BT_BLOCK_POS);
      pBTree->BT_FSPACE_PNT->BT_FSPACE_NEXT = NEXT;
      pBTree->BT_FSPACE_PNT->BT_FSPACE_LAST = 0;
      pBTree->BT_FSPACE_PNT->BT_FSPACE_LEN  = LENGTH;
{
#if ENABLETRACE
GSSiExitProg (510);
#endif
      return;
}
#if ENABLETRACE
}
#endif
}

BOOL ReorgBTree (LPSTR Name, HWND hStatusWnd)
#if ENABLETRACE
{GSSiEnterProg (511);
#endif
{
    HANDLE	hBT, hBT2;
	//BTHEAD BTHead; 
	long	TotRecs,Done;
	short	pos;
	LPSTR	pKey, pData, lpDot;
	HANDLE	hKey, hData;   
	BTVARDESC   BTVar[8];
	char	NewName[256]; 
	BOOL	rtn;
	LPBTREE	pBTree;
     
	hBT = BT_OPEN (Name, 0, BT_READ, 0); 
	if (!hBT)
{
#if ENABLETRACE
GSSiExitProg (511);
#endif
		return FALSE;
}
	pBTree = (LPBTREE)GlobalLock (hBT);
	BT_SET_PARMS (8,16,7,15);
	//GetBTHeader (hBT,&BTHead);   
	GetBTVarDesc (hBT,BTVar);
	         		 
	_fstrcpy (NewName,Name); 
	lpDot = _fstrrchr (NewName,'.');
	*lpDot = 0;
	_fstrcat (NewName,".rbt");
	BT_CREATE (NewName, pBTree->BT_HEAD.BT_DATLEN, FALSE, pBTree->BT_HEAD.BT_NVARS, 1,(LPBTVARDESC) BTVar,FALSE, pBTree->BT_HEAD.BT_DUPPOS, 0, FALSE);
	hBT2 = BT_OPEN (NewName, 0, BT_WRITE, 0); 
	pos = BT_FIRST;
	Done = 0; 
	hKey  = GSSiGlobAlloc ( 114,GHND,1024);
	pKey = GlobalLock (hKey);
	hData  = GSSiGlobAlloc ( 115,GHND,1024);
	pData = GlobalLock (hData);    
	TotRecs = pBTree->BT_HEAD.BT_NUMRECS;
	while (ContinueProcessing && !BT_FIND (hBT,pKey,pos,BT_ANY,pData))
	{  
	 	pos = BT_NEXT;
	 	BT_PUT (hBT2,pKey,pData); 
	 	if (hStatusWnd)
	 		PctBox (hStatusWnd,TotRecs, ++Done,0);
	} 
	GSSiGlobUlFree (&hKey);
	GSSiGlobUlFree (&hData);
	GlobalUnlock (hBT);
	BT_CLOSE (hBT); 
	BT_CLOSE (hBT2); 
	if (ContinueProcessing)
	{
		GSSiRemove (Name);
		GSSiRename (NewName,Name);
	}
	else
		GSSiRemove (NewName); 
	rtn = ContinueProcessing;
	ContinueProcessing = TRUE;
{
#if ENABLETRACE
GSSiExitProg (511);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

int UpdateBTCounts (HGLOBAL IBTID)
#if ENABLETRACE
{GSSiEnterProg (512);
#endif
{	int		irc=BT_NOT_FOUND;
	BOOL	LASTRC;
	HANDLE	hMem=0;  
	LPSTR	pKey, pData;
	long	ii=0;
	BTPOS	START_POS; 
	short   N;
	HCURSOR	hcurSave;
	LPBTREE	pBTree;

    if (!IBTID)
{
#if ENABLETRACE
GSSiExitProg (512);
#endif
    	return (BT_NOT_FOUND);
}
    if (!(pBTree = AllocateBTMem(IBTID)))
{
#if ENABLETRACE
GSSiExitProg (512);
#endif
    	return (BT_NOT_FOUND);
}
    if (!pBTree->BT_HEAD.TrackCount)
    	goto Exit; 
    hMem = GSSiGlobAlloc ( 116,GMEM_MOVEABLE,pBTree->BT_HEAD.BT_KEYLEN+pBTree->BT_HEAD.BT_DATLEN);
    pKey = GlobalLock (hMem);
    pData = pKey + pBTree->BT_HEAD.BT_KEYLEN;
    if (BT_FIND_internal (pBTree,pKey, BT_FIRST, BT_ANY, pData, &LASTRC) == BT_NOT_FOUND)
    	goto Exit;

	hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
	irc = 0;
	pBTree->POS = pBTree->BTID_CUR_POS;
	BT_GET_BLOCK(pBTree); 
	pBTree->BT_END = FALSE;
	while (pBTree->CI_PNT->UPCI != BT_NIL)
	{
	  pBTree->POS.POSA = pBTree->CI_PNT->UPCI;  
	  pBTree->POS.POSB = 1; 
	  START_POS = pBTree->POS;
	  do
	  {
		  do
		  {
		  	BT_GET_BLOCK(pBTree);  
			pBTree->POS.POSB++;
			*pBTree->pBlockCount = 0;
		    UMDB_WRITE_JOURNAL (pBTree,TRUE);
		  } while (pBTree->POS.POSB <= pBTree->CI_PNT->LNCI); 
		  pBTree->POS.POSA = pBTree->CI_PNT->NEXTCI;
		  pBTree->POS.POSB = 1;
	   } while (pBTree->CI_PNT->NEXTCI != BT_NIL); 
	   pBTree->POS = START_POS;
	   BT_GET_BLOCK(pBTree);  
	}
    BT_FIND_internal (pBTree,pKey, BT_FIRST, BT_ANY, pData, &LASTRC);
	pBTree->POS = pBTree->BTID_CUR_POS;
	BT_GET_BLOCK(pBTree); 
	pBTree->BT_END = FALSE;
	while (!pBTree->BT_END)
	{   
	    N = pBTree->CI_PNT->LNCI;
		while (pBTree->CI_PNT->UPCI != BT_NIL)
		{
		  pBTree->LAST_POS = pBTree->POS;
		  pBTree->POS.POSA = pBTree->CI_PNT->UPCI;  
		  pBTree->POS.POSB = 1;
		  BT_GET_BLOCK(pBTree);  
		  while (pBTree->INDEX_ITEM_PNT->BT_DOWN != pBTree->LAST_POS.POSA)
		  {
			pBTree->POS.POSB++;
			BT_GET_BLOCK(pBTree);
		  }
		  (*pBTree->pBlockCount) += N;
		  UMDB_WRITE_JOURNAL (pBTree,TRUE);
		}
		pBTree->POS = pBTree->BTID_CUR_POS;  
		pBTree->POS.POSB = N;
		BT_GET_BLOCK(pBTree);
		FIND_NEXT_BTPOS(pBTree); 
		pBTree->BTID_CUR_POS = pBTree->POS;
		ii++;
	} 
	pBTree->POS.POSA = pBTree->BT_HEAD.BT_FIRST_POS;
	pBTree->POS.POSB = 1; 
	ii = 0;
	do
	{
		BT_GET_BLOCK(pBTree); 
		ii += *pBTree->pBlockCount;
		pBTree->POS.POSB++;
	} while (pBTree->POS.POSB <= pBTree->CI_PNT->LNCI);  
	
    BT_FIND_internal (pBTree,pKey, BT_FIRST, BT_ANY, pData, &LASTRC);
	GSSiSetCursor (hcurSave);
Exit:
    GSSiGlobUlFree (&hMem); 
    DeallocateBTMem(pBTree,IBTID);
{
#if ENABLETRACE
GSSiExitProg (512);
#endif
    return (irc);
}
#if ENABLETRACE
}
#endif
} 

int BT_FIND_RECORD_NUMBER (HGLOBAL IBTID,long RecordNum, LPSTR KEY,LPSTR DATA)
#if ENABLETRACE
{GSSiEnterProg (513);
#endif
{	int		irc=BT_NOT_FOUND, ii;
	DWORD	TotCount=0, LastCount;
	LPBTREE	pBTree;

    if (!IBTID)
{
#if ENABLETRACE
GSSiExitProg (513);
#endif
    	return (BT_NOT_FOUND);
}
    if (!(pBTree = AllocateBTMem(IBTID)))
{
#if ENABLETRACE
GSSiExitProg (513);
#endif
    	return (BT_NOT_FOUND);   
}
    if (RecordNum <= 0 || RecordNum > pBTree->BT_HEAD.BT_NUMRECS)
    	goto Exit;
    if (pBTree->BT_HEAD.TrackCount)
    {   
    	irc = 0;
		pBTree->POS.POSA = pBTree->BT_HEAD.BT_FIRST_POS;
		pBTree->POS.POSB = 1; 
		LastCount = 0; 
		do
		{
			do
			{   
				BT_GET_BLOCK(pBTree); 
//				if (POS.POSB > CI_PNT->LNCI)
//					ii=1;
				pBTree->POS.POSB++; 
				TotCount += LastCount;
				LastCount = *pBTree->pBlockCount;
			} while ((DWORD)RecordNum > TotCount + *pBTree->pBlockCount); 
			pBTree->POS.POSA = pBTree->INDEX_ITEM_PNT->BT_DOWN;
			pBTree->POS.POSB = 1;
			BT_GET_BLOCK(pBTree);
			LastCount = 0; 
		} while (pBTree->CI_PNT->CITYPE == BT_UPPER); 
		pBTree->POS.POSB = (short)((DWORD)RecordNum - TotCount);
        pBTree->BTID_CUR_POS = pBTree->POS;
		BT_GET_BLOCK(pBTree); 
		_fmemmove (DATA,pBTree->INDEX_DATA_PNT ,pBTree->BT_HEAD.BT_DATLEN);
		_fmemmove (KEY,&pBTree->INDEX_ITEM_PNT->INDEX_ENTRY,pBTree->BT_HEAD.BT_KYLEN);
		pBTree->BT_PATH_CHANGE = TRUE;
    }
Exit:
    DeallocateBTMem(pBTree,IBTID);
{
#if ENABLETRACE
GSSiExitProg (513);
#endif
    return (irc);
}
#if ENABLETRACE
}
#endif
}

HANDLE BT_FormKey(HANDLE hKeyList, LPSTR val)
{
	int i;
	BTHEAD btHead;
	HANDLE hKey = 0;
	LPSTR loc,nxtLoc=val;
	LPSTR pKey;

	if (GetBTHeader(hKeyList, &btHead))
	{
		LPBTVARDESC pFldInfo = &btHead.BT_VARDESC;

		hKey = GSSiGlobAlloc(0, GMEM_MOVEABLE, btHead.BT_KEYLEN + 2);
		pKey = GlobalLock(hKey);
		for (i = 0; i < btHead.BT_NVARS; i++, pFldInfo++)
		{
			loc = nxtLoc;
			nxtLoc = strchr(loc, ';');
			if (nxtLoc)
				*nxtLoc++ = 0;
			switch (pFldInfo->BT_VARTYP)
			{
			case BT_INTEGER:
			case BT_INT2:
			case BT_INT4:
				if (pFldInfo->BT_VARLEN == 2)
					*(LPSHORT)pKey = atoi(loc);
				else
					*(LPINT)pKey = atoi(loc);
				break;

			case BT_REAL:
			case BT_REAL4:
			case BT_REAL8:
				if (pFldInfo->BT_VARLEN == 4)
					*(LPFLOAT)pKey = atof(loc);
				else
					*(LPDOUBLE)pKey = atof(loc);
				break;

			default:
				_fmemmove(pKey, loc, pFldInfo->BT_VARLEN);
				break;
			case BT_CHAR:
				_fstrncpy(pKey,loc, pFldInfo->BT_VARLEN);
				break;
			}
			pKey += pFldInfo->BT_VARLEN;
		}
		GlobalUnlock(hKey);
	}
	return hKey;

}
