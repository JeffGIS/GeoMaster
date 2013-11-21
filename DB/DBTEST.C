#include "DBTEST.h"
#include "DBTEST2.h"
#include "bt.h"
#include "shr.h"
#include "stdlib.h"
#include "math.h"
#include "extrndb.h"
#include "GWD.h" 

short	StretchMode;                  
extern	char gszFilter[256];
HCURSOR	curCursor, hCursor, OldCursor;
BOOL	ContinueProcessing, DoPaint, DoTime=FALSE;
char	PltName[128];
HBRUSH	hBackBrush1; 
extern BOOL CALLBACK EnumCtrlProc(HWND hCtrl,LONG lParam);  
BOOL FAR PASCAL SPLITMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam);
                  
char	InName[128], OFTitle[128];
int LoadBTree (HWND hWnd);
int LoadLMData (HWND hWnd);
int LoadStnames (HWND hWnd);
int LoadStnames2 (HWND hWnd);
HANDLE TestOpen (HWND hWnd);

int LoadGeospan (HWND hWnd)
{   char BTfname[100];
	BTVARDESC BTVar[8], *pVars;
	HANDLE hBT, hVars;
	time_t ltime;
	HDC	hDC;
	int	   st,  i, i2key;
	long	ikey, TotRecs, iseg;
	long	idata, nRecs, nLoaded;
	char	cdata[2];
	char	Text[512];
	char	Fname[256];
	OFSTRUCT	OFStruct;
	LPSTR	pInRec, lpStrName;
	int		strnum; 
	RECT	Rect;
	char	cTotRecs[7], VTYPE[2], vdir[8];
	int		NumVars, NumIndex, ioff;
	struct	{int	street_num;
			 long	faddl;
			 long	faddr;
			 long	taddl;
			 long	taddr;
			 }	data;
struct	{
			 int	street_num;
			 long	faddl, faddr, taddl, taddr;
			 long	fframe, tframe;
			 long	frlong, frlat, tolong, tolat;
			 long	CTBNAL, CTBNAR;
			 int	BGL, BGR;
			 BOOL	FixedOdd, FixedEven;
			 char	vdir;
		 }	SegdataOLD;

typedef struct	{
			 long	TLID;
			 long	street_num;
			 long	faddl, faddr, taddl, taddr;
			 long	fframe, tframe;
			 long	frlong, frlat, tolong, tolat;
			 char	CTBNAL[6], CTBNAR[6];
			 int	BGL, BGR;
			 long	FMCDL, FMCDR;
			 int	FixedFlag, ChangedFlag;
			 char	CFCC[3];
			 char	vdir;
			 char	Run[8];
			 int	FromFrameCycle;
			 int	DINumerator;
			 long	LoopFrame; 
			 char	CycleID,
			 		Spacer;
			 long	MALeft,
			 		MARight;
		 }	SEGDATAOLD2;  
		 
typedef struct	{
			 long	TLID;
			 long	street_num;
			 long	faddl, faddr, taddl, taddr;
			 long	fframe, tframe;
			 long	frlong, frlat, tolong, tolat;
			 char	CTBNAL[6], CTBNAR[6];
			 int	BGL, BGR;
			 long	FMCDL, FMCDR;
			 int	DeleteFlag, ChangedFlag;
			 char	CFCC[3];
			 char	vdir;
			 char	Run[8];
			 int	FromFrameCycle;
			 int	DINumerator;
			 long	LoopFrame; 
			 char	CycleID,
			 		Spacer;
			 long	MALeft,
			 		MARight;
			 long	ZIPLeft,
			 		ZIPRight;
		 }	SEGDATA;  
		 
		 

	char	TAG[30];
	int		FidData, ibeg;
	HFILE	Fid;
	struct {int		Beg;
			int		Len;
			int		Type;
			char	Name[34];
			} FldInfo;
	int		NumFields, Reclen, len;
	long	Refno, Offset;
	char	CRef[16];
	GWDHEADER GWDHead;
	GWFLDINFO GWFldInfo; 
	SEGDATA	Segdata; 
	int		l;
	char	ct[16],str[256];

/*	_fstrcpy (Fname,"c:\\data.pmq");*/
	_fstrcpy (Fname,"f:\\geospan.dat");
	Fid = OpenFile (Fname,&OFStruct,OF_READ);

/*	Get the total records in the file */
	_llseek (Fid,-8,2);
	i =_lread (Fid,cTotRecs,6);
	cTotRecs[6]='\0';
	TotRecs = atol (cTotRecs);
	_llseek (Fid,0,0);

/*	Get the number of fields and indexes */
	pInRec = NextRec (Fid); TotRecs--;
	NumVars = atoi (NextFld(&pInRec));
	NumIndex = atoi (NextFld(&pInRec));

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);

    for (i=0,ioff=0;i<NumVars;i++,pVars++)
    {
		pInRec = NextRec (Fid); TotRecs--;
		_fstrcpy (VTYPE,NextFld(&pInRec));
		if 		(VTYPE[0] == 'I') pVars->BT_VARTYP = BT_INTEGER;
		else if (VTYPE[0] == 'C') pVars->BT_VARTYP = BT_CHAR;
		else if (VTYPE[0] == 'R') pVars->BT_VARTYP = BT_REAL;
		pVars->BT_VARLEN = atoi (NextFld(&pInRec));

		pVars->BT_VAROFF = ioff;
		ioff += pVars->BT_VARLEN;
	}
    pVars = LocalUnlock(hVars);
    pVars = LocalLock(hVars);

    for (i=0,ioff=0;i<NumIndex;i++)
    {
		pInRec = NextRec (Fid); TotRecs--;
	}
	ltime = 0;

	_fstrcpy (Fname,"c:\\geospan.gwd");
	FidData = OpenFile (Fname,&OFStruct,OF_CREATE);
/*	FidData = OpenFile (Fname,&OFStruct,OF_READWRITE);*/
	GWDHead.NumFields=0;
	GWDHead.NumIndex=1;
	GWDHead.Version=1;
	GWDHead.Reclen=sizeof(Segdata);
	GWDHead.NumIndexFields[0]=1;
	GWDHead.IndexFields[0][0]=1;
	_lwrite (FidData,&GWDHead,sizeof(GWDHEADER));
	ibeg = 0;
           
	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"TLID");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"StreetNum");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"FRADDL");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"FRADDR");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"TOADDL");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"TOADDR");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"fframe");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"tframe");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"frlong");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"frlat");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"tolong");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"tolat");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 6;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"CTBNAL");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 6;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"CTBNAR");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"BGL");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"BGR");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"FMCDL");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"FMCDR");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"FixedFlag");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"ChangedFlag");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 3;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"CFCC");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 1;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"vdir");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"Run");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"FromFrameCycle");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"DINumerator");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"LoopFrame");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 1;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"CycleID");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 1;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"Spacer");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"MALeft");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"MARight");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"ZIPLeft");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"ZIPRight");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	 GWDHead.Reclen=ibeg; 
	 GWDHead.TimeStamp = time(NULL);
	 _llseek (FidData,0,0);
	_lwrite (FidData,&GWDHead,sizeof(GWDHEADER)-sizeof(GWFLDINFO));
	_llseek (FidData,0,2);

	NumVars = 1;
	NumIndex = 1;

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);


	BT_SET_PARMS (8,16,4,8); 
	_fstrcpy (BTfname, "c:\\geospan.in1");
	pVars->BT_VARLEN=4;
	pVars->BT_VARTYP=BT_INTEGER;
	pVars->BT_VAROFF=0;
	BT_CREATE (BTfname, 4, FALSE, 1, 1,pVars,FALSE, 0, GWDHead.TimeStamp, FALSE); 
	hBT = BT_OPEN (BTfname, GWDHead.TimeStamp, 1, 0); 
	_llseek(FidData,0,2);
     
     GetWindowRect(hWnd,&Rect);

	nRecs = TotRecs;
	nLoaded=0;
	while (TotRecs--)
	{
		pInRec = NextRec (Fid);
		ikey=atol (NextFld(&pInRec));
		Segdata.TLID = ikey;
		Segdata.street_num = atol (NextFld(&pInRec));
		Segdata.faddl = atol (NextFld(&pInRec));
		Segdata.faddr = atol (NextFld(&pInRec));
		Segdata.taddl = atol (NextFld(&pInRec));
		Segdata.taddr = atol (NextFld(&pInRec));
		Segdata.frlong = atol (NextFld(&pInRec));
		Segdata.frlat = atol (NextFld(&pInRec));
		Segdata.tolong = atol (NextFld(&pInRec));
		Segdata.tolat = atol (NextFld(&pInRec)); 
		_fstrcpy (ct,NextFld(&pInRec));  
		_fmemset (Segdata.CTBNAL,' ',6);
		l = _fstrlen (ct);
		if (l)
			_fmemmove(Segdata.CTBNAL,ct,l);
		_fstrcpy (ct,NextFld(&pInRec));  
		_fmemset (Segdata.CTBNAR,' ',6);
		l = _fstrlen (ct);
		if (l)
			_fmemmove(Segdata.CTBNAR,ct,l);
		Segdata.BGL = atoi (NextFld(&pInRec));
		Segdata.BGR = atoi (NextFld(&pInRec)); 
		Segdata.FMCDL = atol (NextFld(&pInRec));
		Segdata.FMCDR = atol (NextFld(&pInRec));
		_fmemmove (Segdata.CFCC,NextFld(&pInRec),3);
		
		Segdata.fframe = Segdata.tframe = Segdata.LoopFrame = 0;
		Segdata.CycleID = ' ';  
		Segdata.DINumerator = Segdata.FromFrameCycle = 0;
		_fmemset (Segdata.Run,' ',8); 
		Segdata.DeleteFlag = Segdata.ChangedFlag = 0;
		Segdata.vdir = ' '; 
		Segdata.MALeft = 0;
		Segdata.MARight = 0;  
		Segdata.ZIPLeft = atol (NextFld(&pInRec));
		Segdata.ZIPRight= atol (NextFld(&pInRec));
		

		Offset = _llseek (FidData,0,1);
		st = BT_PUT (hBT,(LPSTR)&ikey,(LPSTR)&Offset);
		len = sizeof(Segdata);
		_lwrite (FidData,&len,2);
		_lwrite (FidData,&Segdata,len);  
		ltoa (ikey,str,10);
//		SetWindowText (hWnd,str);
		if (!PctBox (hWnd, nRecs, ++nLoaded, -10)) TotRecs=0;
	}


	ReleaseDC(hWnd, hDC);
	BT_CLOSE (hBT);
	_lclose (FidData);
	_lclose (Fid);

	return (st);


} 

LPSTR ExpandText (LPSTR Text)
{
	return Text;
}

int LoadGeospanMNDot (HWND hWnd)
{   char BTfname[100];
	BTVARDESC BTVar[8], *pVars;
	HANDLE hBT, hVars;
	time_t ltime;
	HDC	hDC;
	int	   st,  i, i2key;
	long	ikey, TotRecs, iseg;
	long	idata, nRecs, nLoaded;
	char	cdata[2];
	char	Text[512];
	char	Fname[256];
	OFSTRUCT	OFStruct;
	LPSTR	pInRec, lpStrName;
	int		strnum;
	char	cTotRecs[7], VTYPE[2], vdir[8];
	int		NumVars, NumIndex, ioff;
	struct	{int	street_num;
			 long	faddl;
			 long	faddr;
			 long	taddl;
			 long	taddr;
			 }	data;
struct	{
			 int	street_num;
			 long	faddl, faddr, taddl, taddr;
			 long	fframe, tframe;
			 long	frlong, frlat, tolong, tolat;
			 long	CTBNAL, CTBNAR;
			 int	BGL, BGR;
			 long	FMCDL, FMCDR;
			 int	FixedFlag, ChangedFlag;
			 char	CFCC[3];
			 char	vdir;
		 }	Segdata;
	char	TAG[30];
	int		FidData, ibeg;
	FILE	*Fid;
	struct {int		Beg;
			int		Len;
			int		Type;
			char	Name[34];
			} FldInfo;
	int		NumFields, Reclen, len;
	long	Refno, Offset;
	char	CRef[16];
	GWDHEADER GWDHead;
	GWFLDINFO GWFldInfo;

/*	_fstrcpy (Fname,"c:\\data.pmq");*/
	_fstrcpy (Fname,"j:\\geospan.txt");
	Fid = OpenFile (Fname,&OFStruct,OF_READ);

/*	Get the total records in the file */
	_llseek (Fid,-8,2);
	i =_lread (Fid,cTotRecs,6);
	cTotRecs[6]='\0';
	TotRecs = atol (cTotRecs);
	_llseek (Fid,0,0);

/*	Get the number of fields and indexes */
	pInRec = NextRec (Fid); TotRecs--;
	NumVars = atoi (NextFld(&pInRec));
	NumIndex = atoi (NextFld(&pInRec));

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);

    for (i=0,ioff=0;i<NumVars;i++,pVars++)
    {
		pInRec = NextRec (Fid); TotRecs--;
		_fstrcpy (VTYPE,NextFld(&pInRec));
		if 		(VTYPE[0] == 'I') pVars->BT_VARTYP = BT_INTEGER;
		else if (VTYPE[0] == 'C') pVars->BT_VARTYP = BT_CHAR;
		else if (VTYPE[0] == 'R') pVars->BT_VARTYP = BT_REAL;
		pVars->BT_VARLEN = atoi (NextFld(&pInRec));

		pVars->BT_VAROFF = ioff;
		ioff += pVars->BT_VARLEN;
	}
    pVars = LocalUnlock(hVars);
    pVars = LocalLock(hVars);

    for (i=0,ioff=0;i<NumIndex;i++)
    {
		pInRec = NextRec (Fid); TotRecs--;
	}
	ltime = 0;

	_fstrcpy (Fname,"j:\\geospan.gwd");
/*	FidData = OpenFile (Fname,&OFStruct,OF_WRITE);*/
	FidData = OpenFile (Fname,&OFStruct,OF_CREATE);
	GWDHead.NumFields=0;
	GWDHead.NumIndex=1;
	GWDHead.Version=0;
	GWDHead.Reclen=sizeof(Segdata);
	GWDHead.NumIndexFields[0]=1;
	GWDHead.IndexFields[0][0]=1;
	_lwrite (FidData,&GWDHead,sizeof(GWDHEADER)-sizeof(GWFLDINFO));
	ibeg = 0;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"StreetNum");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"FRADDL");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"FRADDR");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"TOADDL");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"TOADDR");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"fframe");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"tframe");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"frlong");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"frlat");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"tolong");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"tolat");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"CTBNAL");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"CTBNAR");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"BGL");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"BGR");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"FMCDL");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"FMCDR");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"FixedFlag");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"ChangedFlag");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 3;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"CFCC");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 1;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"vdir");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	_llseek (FidData,0,0);
	_lwrite (FidData,&GWDHead,sizeof(GWDHEADER)-sizeof(GWFLDINFO));
	_llseek (FidData,0,2);

	NumVars = 1;
	NumIndex = 1;

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);

	ltime = 0;

	BT_SET_PARMS (8,16,4,8);
	strcpy (BTfname, "j:\\geospan.in1");
	pVars->BT_VARLEN=4;
	pVars->BT_VARTYP=BT_INTEGER;
	pVars->BT_VAROFF=0;
	BT_CREATE (BTfname, 4, FALSE, 1, 1,pVars,FALSE, 0, 0, FALSE);
	strcpy (BTfname, "j:\\geospan.in1");
	hBT = BT_OPEN (BTfname, ltime, 1, 0);

	hDC = GetDC(hWnd);
	nRecs = TotRecs;
	nLoaded=0;
	while (TotRecs)
	{
		pInRec = NextRec (Fid); TotRecs--;
		ikey=atol (NextFld(&pInRec));
		Segdata.street_num = atoi (NextFld(&pInRec));
		Segdata.faddl = atol (NextFld(&pInRec));
		Segdata.faddr = atol (NextFld(&pInRec));
		Segdata.taddl = atol (NextFld(&pInRec));
		Segdata.taddr = atol (NextFld(&pInRec));
		Segdata.fframe = 0;
		Segdata.tframe = 0;
		Segdata.vdir = '0';
		Segdata.frlong = atol (NextFld(&pInRec));
		Segdata.frlat = atol (NextFld(&pInRec));
		Segdata.tolong = atol (NextFld(&pInRec));
		Segdata.tolat = atol (NextFld(&pInRec));
		Segdata.CTBNAL = atol (NextFld(&pInRec));
		Segdata.CTBNAR = atol (NextFld(&pInRec));
		Segdata.BGL = atoi (NextFld(&pInRec));
		Segdata.BGR = atoi (NextFld(&pInRec));
		Segdata.FMCDL = atol (NextFld(&pInRec));
		Segdata.FMCDR = atol (NextFld(&pInRec));
		_fmemmove (Segdata.CFCC,NextFld(&pInRec),3);
		Segdata.FixedFlag = 0;
		Segdata.ChangedFlag = 0;
		if (Segdata.CFCC[0] == 'A')
		{
			Offset = _llseek (FidData,0,2);
			st = BT_PUT (hBT,(LPSTR)&ikey,(LPSTR)&Offset);
			len = sizeof(Segdata);
			_lwrite (FidData,&len,2);
			_lwrite (FidData,&Segdata,len);
		}
		if (!PctBox (hWnd, hDC, nRecs, ++nLoaded)) TotRecs=0;
	}


	ReleaseDC(hWnd, hDC);
	BT_CLOSE (hBT);
	_lclose (FidData);
	_lclose (Fid);

	return (st);


}

int LoadCendata (HWND hWnd)
{   char BTfname[100];
	BTVARDESC BTVar[8], *pVars;
	HANDLE hBT, hVars;
	time_t ltime;
	HDC	hDC;
	int	   st,  i, i2key;
	long	ikey, TotRecs, iseg;
	long	idata, nRecs, nLoaded;
	char	cdata[2];
	char	Text[512];
	char	Fname[256];
	OFSTRUCT	OFStruct;
	LPSTR	pInRec, lpStrName;
	int		strnum;
	char	cTotRecs[7], VTYPE[2], vdir[8];
	int		NumVars, NumIndex, ioff;
struct	{
			char	TRACTBNA[6];
			int		BLCKGR;
		}	Key;

	char	TAG[30];
	int		FidData, ibeg;
	int		Fid;
	struct {int		Pct_0_4;
			int		Pct_5_9;
			int		Pct_10_19;
			int		Pct_20_49;
			int		Pct_50_64;
			int		Pct_65_UP;
			int		Pct_White;
			int		Pct_Black;
			int		Pct_Indian;
			int		Pct_Asian;
			int		Pct_Hispanic;
			int		Pct_Other;
			int		Pct_Owner;
			int		Pct_Renter;
			} data;
int		Type;
	struct {int		Beg;
			int		Len;
			int		Type;
			char	Name[34];
			} FldInfo;
	int		NumFields, Reclen, len;
	long	Refno, Offset;
	char	CRef[16], str[128];
	GWDHEADER GWDHead;
	GWFLDINFO GWFldInfo;

/*	_fstrcpy (Fname,"c:\\data.pmq");*/
	_fstrcpy (Fname,"c:\\cendata.txt");
	Fid = OpenFile (Fname,&OFStruct,OF_READ);

	NumVars = 2;
	NumIndex = 1;

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);

		BTVar[0].BT_VARTYP = BT_CHAR;
		BTVar[0].BT_VARLEN = 6;
		BTVar[0].BT_VAROFF = 0;
		BTVar[1].BT_VARTYP = BT_INTEGER;
		BTVar[1].BT_VARLEN = 2;
		BTVar[1].BT_VAROFF = 6;
	pInRec = NextRec (Fid); TotRecs--;
	ltime = 0;

	_fstrcpy (Fname,"c:\\cendata.gwd");
	FidData = OpenFile (Fname,&OFStruct,OF_CREATE);
	GWDHead.NumFields=0;
	GWDHead.NumIndex=1;
	GWDHead.Version=0;
	GWDHead.Reclen=sizeof(data);
	GWDHead.NumIndexFields[0]=2;
	GWDHead.IndexFields[0][0]=1;
	GWDHead.IndexFields[0][1]=2;
	_lwrite (FidData,&GWDHead,sizeof(GWDHEADER)-sizeof(GWFLDINFO));
	ibeg = 0;

	FldInfo.Len = 6;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"TRACTBNA");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"BLCKGR");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"Pct_0_4");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"Pct_5_9");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"Pct_10_19");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"Pct_20_49");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"Pct_50_64");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"Pct_65_UP");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"Pct_White");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"Pct_Black");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"Pct_Indian");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"Pct_Asian");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"Pct_Hispanic");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"Pct_Other");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"Pct_Owner");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"Pct_Renter");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;


	_llseek (FidData,0,0);
	_lwrite (FidData,&GWDHead,sizeof(GWDHEADER)-sizeof(GWFLDINFO));
	_llseek (FidData,0,2);

	NumVars = 1;
	NumIndex = 1;

	BT_SET_PARMS (8,16,4,8);
	strcpy (BTfname, "c:\\cendata.in1");
	BT_CREATE (BTfname, 4, FALSE, 2, 1,&BTVar,FALSE, 0, 0, FALSE);
	hBT = BT_OPEN (BTfname, ltime, 1, 0);

	hDC = GetDC(hWnd);
	TotRecs = 4444;
	nRecs = TotRecs;
	nLoaded=0;
	while (TotRecs)
	{
		pInRec = NextRec (Fid); TotRecs--;
		if (!_fstrstr(pInRec,",,"))
		{    
			if (*pInRec == '"') pInRec--;
			_fstrcpy (str,NextFld(&pInRec));
			_fstrncpy (Key.TRACTBNA,str,6);
			for (i=0;i<6;i++)
				if (Key.TRACTBNA[i] == '\0')
					Key.TRACTBNA[i] = ' ';
			Key.BLCKGR=atoi (NextFld(&pInRec));
			data.Pct_0_4 = 0.5+10*atof (NextFld(&pInRec));
			data.Pct_5_9 = 0.5+10*atof (NextFld(&pInRec));
			data.Pct_10_19 = 0.5+10*atof (NextFld(&pInRec));
			data.Pct_20_49 = 0.5+10*atof (NextFld(&pInRec));
			data.Pct_50_64 = 0.5+10*atof (NextFld(&pInRec));
			data.Pct_65_UP = 0.5+10*atof (NextFld(&pInRec));
			data.Pct_White = 0.5+10*atof (NextFld(&pInRec));
			data.Pct_Black = 0.5+10*atof (NextFld(&pInRec));
			data.Pct_Indian = 0.5+10*atof (NextFld(&pInRec));
			data.Pct_Asian = 0.5+10*atof (NextFld(&pInRec));
			data.Pct_Hispanic = 0.5+10*atof (NextFld(&pInRec));
			data.Pct_Other = 0.5+10*atof (NextFld(&pInRec));
			data.Pct_Owner = 0.5+10*atof (NextFld(&pInRec));
			data.Pct_Renter = 0.5+10*atof (NextFld(&pInRec));
			Offset = _llseek (FidData,0,1);
			st = BT_PUT (hBT,(LPSTR)&Key,(LPSTR)&Offset);
			len = sizeof(data);
			_lwrite (FidData,&len,2);
			_lwrite (FidData,&data,len);
		}
	/*	if (!PctBox (hWnd, nRecs, ++nLoaded,10)) TotRecs=0;*/
	}

    pVars = LocalUnlock(hVars);

	ReleaseDC(hWnd, hDC);
	BT_CLOSE (hBT);
	_lclose (FidData);
	_lclose (Fid);

	return (st);


}

int LoadGeospanCor (HWND hWnd)
{   char BTfname[100];
	BTVARDESC BTVar[8], *pVars;
	HANDLE hBT, hVars;
	time_t ltime;
	HDC	hDC;
	int	   st,  i, i2key;
	long	ikey, TotRecs, iseg;
	long	idata, nRecs, nLoaded;
	char	cdata[2];
	char	Text[512];
	char	Fname[256];
	OFSTRUCT	OFStruct;
	LPSTR	pInRec, lpStrName;
	int		strnum;
	char	cTotRecs[7], VTYPE[2], vdir[8];
	int		NumVars, NumIndex, ioff;
struct	{
			long	TRACTBNA;
			int		BLCKGR;
		}	Key;

	char	TAG[30];
	int		FidData, ibeg;
	FILE	*Fid;
	struct {
			int		product;
			long	X,Y;
			int		speed,
					azimuth;
			} data;
int		Type;
	struct {int		Beg;
			int		Len;
			int		Type;
			char	Name[34];
			} FldInfo;
	int		NumFields, Reclen, len;
	long	Refno, Offset, lastframe;
	char	CRef[16];
	GWDHEADER GWDHead;
	GWFLDINFO GWFldInfo;
	long	Frame;

/*	_fstrcpy (Fname,"c:\\data.pmq");*/
	_fstrcpy (Fname,InName);
	_fstrcat (Fname,".txt");
/*	Fid = OpenFile (Fname,&OFStruct,OF_READ);*/
	Fid = fopen (Fname,"r");

	NumVars = 1;
	NumIndex = 1;

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);

		BTVar[0].BT_VARTYP = BT_INTEGER;
		BTVar[0].BT_VARLEN = 4;
		BTVar[0].BT_VAROFF = 0;
	ltime = 0;

	_fstrcpy (Fname,InName);
	_fstrcat (Fname,".gwd");
	FidData = OpenFile (Fname,&OFStruct,OF_CREATE);
	GWDHead.NumFields=0;
	GWDHead.NumIndex=1;
	GWDHead.Version=0;
	GWDHead.Reclen=sizeof(data);
	GWDHead.NumIndexFields[0]=2;
	GWDHead.IndexFields[0][0]=1;
	GWDHead.IndexFields[0][1]=2;
	_lwrite (FidData,&GWDHead,sizeof(GWDHEADER)-sizeof(GWFLDINFO));
	ibeg = 0;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"Product");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"X");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"Y");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"SpeedX10");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"AzimuthX10");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	_llseek (FidData,0,0);
	_lwrite (FidData,&GWDHead,sizeof(GWDHEADER)-sizeof(GWFLDINFO));
	_llseek (FidData,0,2);

	NumVars = 1;
	NumIndex = 1;
	lastframe = 0;

	BT_SET_PARMS (8,16,4,8);
	_fstrcpy (Fname,InName);
	_fstrcat (Fname,".in1");
	BT_CREATE (Fname, 4, FALSE, 1, 1,&BTVar,FALSE, 0, 0, FALSE);
	hBT = BT_OPEN (Fname, ltime, 1, 0);

	hDC = GetDC(hWnd);
	TotRecs = 4444;
	nRecs = TotRecs;
	nLoaded=0;
	while (fgetss (Text,512,Fid))
	{
		_fstrncpy (CRef,Text,6);
		CRef[6]='\0';
		Frame = atol (CRef);
		if (Frame<lastframe)
		{
	      	 MessageBox( GetFocus(),"Reset Frame",
				 	    "Error", MB_OK);
		}
		lastframe=Frame;
		Offset = _llseek (FidData,0,1);
		st = BT_PUT (hBT,(LPSTR)&Frame,(LPSTR)&Offset);
		_fstrncpy (CRef,&Text[6],2);
		CRef[2]='\0';
		data.product = atol (CRef);
		_fstrncpy (CRef,&Text[8],8);
		CRef[8]='\0';
		data.X = atol (CRef);
		_fstrncpy (CRef,&Text[16],8);
		CRef[8]='\0';
		data.Y = atol (CRef);
		_fstrncpy (CRef,&Text[24],6);
		CRef[6]='\0';
		data.speed = atol (CRef);
		_fstrncpy (CRef,&Text[30],5);
		CRef[5]='\0';
		data.azimuth = atol (CRef);
		len = sizeof(data);
		_lwrite (FidData,&len,2);
		_lwrite (FidData,&data,len);
		if (!PctBox (hWnd, hDC, nRecs, ++nLoaded)) TotRecs=0;
	}

    pVars = LocalUnlock(hVars);
    LocalFree (hVars);

	ReleaseDC(hWnd, hDC);
	BT_CLOSE (hBT);
	_lclose (FidData);
	fclose (Fid);

	return (st);
}

int PASCAL WinMain(HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
 /***********************************************************************/
 /* HANDLE hInstance;       handle for this instance                    */
 /* HANDLE hPrevInstance;   handle for possible previous instances      */
 /* LPSTR  lpszCmdLine;     long pointer to exec command line           */
 /* int    nCmdShow;        Show code for main window display           */
 /***********************************************************************/

 MSG        msg;           /* MSG structure to store your messages        */
 int        nRc;           /* return value from Register Classes          */

 strcpy(szAppName, "DBTEST");
 hInst = hInstance;
 if(!hPrevInstance)
   {
    /* register window classes if first instance of application         */
    if ((nRc = nCwRegisterClasses()) == -1)
      {
       /* registering one of the windows failed                         */
       LoadString(hInst, IDS_ERR_REGISTER_CLASS, szString, sizeof(szString));
       MessageBox(NULL, szString, NULL, MB_ICONEXCLAMATION);
       return nRc;
      }
   }

 /* create application's Main window                                    */
 hWndMain = CreateWindow(
                szAppName,               /* Window class name           */
                "GeoMaster Utilities", /* Window's title             */
                WS_CAPTION      |        /* Title and Min/Max           */
                WS_SYSMENU      |        /* Add system menu box         */
                WS_MINIMIZEBOX  |        /* Add minimize box            */
                WS_MAXIMIZEBOX  |        /* Add maximize box            */
                WS_THICKFRAME   |        /* thick sizeable frame        */
                WS_CLIPCHILDREN |         /* don't draw in child windows areas */
                WS_OVERLAPPED,
                CW_USEDEFAULT, 0,        /* Use default X, Y            */
                CW_USEDEFAULT, 0,        /* Use default X, Y            */
                NULL,                    /* Parent window's handle      */
                NULL,                    /* Default to Class Menu       */
                hInst,                   /* Instance of window          */
                NULL);                   /* Create struct for WM_CREATE */


 if(hWndMain == NULL)
   {
    LoadString(hInst, IDS_ERR_CREATE_WINDOW, szString, sizeof(szString));
    MessageBox(NULL, szString, NULL, MB_ICONEXCLAMATION);
    return IDS_ERR_CREATE_WINDOW;
   }

 ShowWindow(hWndMain, nCmdShow);            /* display main window      */

 while(GetMessage(&msg, NULL, 0, 0))        /* Until WM_QUIT message    */
   {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
   }

 /* Do clean up before exiting from the application                     */
 CwUnRegisterClasses();
 return msg.wParam;
} /*  End of WinMain                                                    */
/************************************************************************/
/*                                                                      */
/* Main Window Procedure                                                */
/*                                                                      */
/* This procedure provides service routines for the Windows events      */
/* (messages) that Windows sends to the window, as well as the user     */
/* initiated events (messages) that are generated when the user selects */
/* the action bar and pulldown menu controls or the corresponding       */
/* keyboard accelerators.                                               */
/*                                                                      */
/************************************************************************/

LONG FAR PASCAL WndProc(HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{
 HMENU      hMenu=0;            /* handle for the menu                 */
 HBITMAP    hBitmap=0;          /* handle for bitmaps                  */
 HDC        hDC;                /* handle for the display device       */
 PAINTSTRUCT ps;                /* holds PAINT information             */
 int        nRc=0;              /* return code                         */
 static		HANDLE	hBT;

 switch (Message)
   {
    case WM_COMMAND:
         /* The Windows messages for action bar and pulldown menu items */
         /* are processed here.                                         */
         switch (wParam)
           {
            case IDM_F_NEW:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "New" here.                         */
                 break;

            case IDM_F_OPEN:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "Open" here.                        */
                 break;

            case IDM_F_DELETE:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "Delete" here.                      */
                 break;

            case IDM_F_EXIT:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "Exit" here.                        */
                 break;

            case IDM_SHOWRECORDS:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "Show Records" here.                */
                 break;

            case IDM_ADDRECORDS:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "Add Records" here.                 */
                 break;

            case IDM_IMPORT:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "Import" here.                      */
                 /*LoadBTree (hWnd);*/
                 /*LoadLMData (hWnd);*/
                 LoadStnames (hWnd);
                /*LoadAltNames (hWnd);*/
                 /*LoadStnames2 (hWnd);*/
                 /* LoadMunics(hWnd); */
                 /*LoadGIDs(hWnd);*/
                 /*LoadWells (hWnd);*/
                 /*LoadParcels (hWnd);*/
                 /*LoadParcelsSub (hWnd);*/
				 /*CreateDOTRoute (hWnd);*/
                 /*CreateProp(hWnd);*/
                 /*CreateClients(hWnd);*/
                 /*LoadCadZones(hWnd);*/
                 /*LoadBrkPrk(hWnd);*/
                 /*LoadHazem (hWnd);*/
                 /*LoadCwld (hWnd);*/
                 /*LoadAddressList(hWnd);*/
                 /*LoadGeospanAddr(hWnd);*/
                /*LoadGeospan(hWnd);*/
                /* LoadGeospanMNDot(hWnd);*/
                /* LoadCenData(hWnd);*/


                 break;      
            case IDM_CREATE_CMDID:
				{
					FILE	*Fid; 
					char	str[128];
					long	ID;
					LPSTR	lpSpace; 
					static	HFILE	FidOut=0;  
					OFSTRUCT	OFStruct;
					
					if (!FidOut)
						FidOut = GSSiOpenFile ("c:\\gssi\\prog\\gwizinst\\geomastr\\cmdid",&OFStruct,OF_CREATE);  
					Fid=fopen("c:\\gssi\\prog\\geomastr\\resource.h","rt");
					if (Fid)
					{
						while (fgetss(str,128,Fid))
						{
							OneSpace(str);
							if (!_fstrnicmp ("#define IDM_",str,12))
							{
								fputstring (&str[8],FidOut);
							}
						}
						fclose (Fid);
						fclose (FidOut);
					} 
				}
            	 break;
            case IDM_GMINSTAL:
            	{   
            		HFILE	Fid, Fid2;
            		char	str[32]="Hi There";
            		OFSTRUCT	OFStruct;
            		long	index[8], indexloc, lbytes, nbytes; 
            		HANDLE	hBuf;
            		LPSTR	pBuf;  
            		time_t	timestamp;
            		                  
            		time (&timestamp);
            		hBuf = GlobalAlloc (GMEM_MOVEABLE,UINT_MAX);
            		pBuf = GlobalLock (hBuf);
            		Fid = OpenFile ("c:\\install\\gminstal.exe",&OFStruct,OF_READWRITE);
            		index[0]=_llseek (Fid,0,2);
            		Fid2 = OpenFile ("c:\\install\\setup.exe",&OFStruct,OF_READ); 
            		lbytes=index[1]=_llseek (Fid2,0,2);
            		_llseek (Fid2,0,0);
		         	while (lbytes)
		         	{
		         		nbytes = _lread (Fid2,pBuf,min(UINT_MAX,lbytes)); 
		         		_lwrite (Fid,pBuf,nbytes);
		         		lbytes -= nbytes;
		         	}                              
		         	_lclose (Fid2);
            		index[2]=_llseek (Fid,0,2);
            		Fid2 = OpenFile ("c:\\install\\setupmn.arv",&OFStruct,OF_READ); 
            		lbytes=index[3]=_llseek (Fid2,0,2);
            		_llseek (Fid2,0,0);
		         	while (lbytes)
		         	{
		         		nbytes = _lread (Fid2,pBuf,min(UINT_MAX,lbytes)); 
		         		_lwrite (Fid,pBuf,nbytes);
		         		lbytes -= nbytes;
		         	}                              
		         	_lclose (Fid2);
            		index[4]=_llseek (Fid,0,2);
            		Fid2 = OpenFile ("c:\\install\\setup.arv",&OFStruct,OF_READ); 
            		lbytes=_llseek (Fid2,0,2);
            		index[5] = lbytes/2;
            		index[6] = 0;
            		index[7] = lbytes-index[5]; 
            		lbytes = index[5];
            		_llseek (Fid2,0,0);
		         	while (lbytes)
		         	{
		         		nbytes = _lread (Fid2,pBuf,min(UINT_MAX,lbytes)); 
		         		_lwrite (Fid,pBuf,nbytes);
		         		lbytes -= nbytes;
		         	}                              
            		_lwrite (Fid,index,sizeof(index));  
            		_lwrite (Fid,&timestamp,sizeof(timestamp));
            		_lclose (Fid); 
            		Fid = OpenFile ("c:\\install\\gminstal.002",&OFStruct,OF_CREATE);  
            		lbytes = index[7];
		         	while (lbytes)
		         	{
		         		nbytes = _lread (Fid2,pBuf,min(UINT_MAX,lbytes)); 
		         		_lwrite (Fid,pBuf,nbytes);
		         		lbytes -= nbytes;
		         	}                              
                    
                    _lwrite (Fid,&timestamp,sizeof(timestamp));
		         	_lclose (Fid2); 
		         	_lclose (Fid);
            		
            		GlobalUnlock (hBuf);
            		GlobalFree (hBuf);
            	}
            	break;
                 
            case IDM_SPLIT:
                 {
                  FARPROC lpfnSPLITMsgProc;

                  lpfnSPLITMsgProc = MakeProcInstance((FARPROC)SPLITMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"SPLIT", hWnd, lpfnSPLITMsgProc);
                  FreeProcInstance(lpfnSPLITMsgProc);
                 }
            	break;
            	

            case IDM_TESTLOAD:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "Test Load" here.                   */


                 {
                  FARPROC lpfnTESTLOADMsgProc;

                  lpfnTESTLOADMsgProc = MakeProcInstance((FARPROC)TESTLOADMsgProc, hInst);
                  nRc = DialogBox(hInst, MAKEINTRESOURCE(100), hWnd, lpfnTESTLOADMsgProc);
                  FreeProcInstance(lpfnTESTLOADMsgProc);
                 }
                 break;

            case IDM_COMPRESS:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "Compress" here.                    */


                 {
                  FARPROC lpfnCOMPRESSMsgProc;

                  lpfnCOMPRESSMsgProc = MakeProcInstance((FARPROC)COMPRESSMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"COMPRESS", hWnd, lpfnCOMPRESSMsgProc);
                  FreeProcInstance(lpfnCOMPRESSMsgProc);
                 }
                 break;

            default:
                return DefWindowProc(hWnd, Message, wParam, lParam);
           }
         break;        /* End of WM_COMMAND                             */

    case WM_CREATE:

		 CDInit (hWnd, hInst); /* Initialize Common Dialogs */     
        /* hBT = TestOpen (hWndMain);*/
         break;       /*  End of WM_CREATE                              */

    case WM_MOVE:     /*  code for moving the window                    */
         break;

    case WM_SIZE:     /*  code for sizing client area                   */
         break;       /* End of WM_SIZE                                 */

    case WM_PAINT:    /* code for the window's client area              */
         /* Obtain a handle to the device context                       */
         /* BeginPaint will sends WM_ERASEBKGND if appropriate          */
         memset(&ps, 0x00, sizeof(PAINTSTRUCT));
         hDC = BeginPaint(hWnd, &ps);

         /* Included in case the background is not a pure color         */
         SetBkMode(hDC, TRANSPARENT);

         /* Inform Windows painting is complete                         */
         EndPaint(hWnd, &ps);
         break;       /*  End of WM_PAINT                               */

    case WM_CLOSE:  /* close the window                                 */
         /* Destroy child windows, modeless dialogs, then, this window  */
		 /*BT_CLOSE (hBT);*/
         DestroyWindow(hWnd);
		 DeleteObject (hBackBrush1);
         if (hWnd == hWndMain)
           PostQuitMessage(0);  /* Quit the application                 */
        break;

    default:
         /* For any message for which you don't specifically provide a  */
         /* service routine, you should return the message to Windows   */
         /* for default message processing.                             */
         return DefWindowProc(hWnd, Message, wParam, lParam);
   }
 return 0L;
}     /* End of WndProc                                         */       
 void HaltMapDisplay(void)
 {
 	return;
 }
 int GetOpenERR00(void)
 {
 	return 1;
 }
/************************************************************************/
/*                                                                      */
/* Dialog Window Procedure                                              */
/*                                                                      */
/* This procedure is associated with the dialog box that is included in */
/* the function name of the procedure. It provides the service routines */
/* for the events (messages) that occur because the end user operates   */
/* one of the dialog box's buttons, entry fields, or controls.          */
/*                                                                      */
/************************************************************************/

BOOL FAR PASCAL TESTLOADMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{
    char BTfname[100];
	BTVARDESC BTVar[8];
	static HANDLE hBT;
	time_t ltime;
	HDC	hDC;
	static int	   st, indx, stSeg, SegMaxData,nLoaded,i;
	static long	ikey, lastkey=0;
	long	idata, nrecs;
	char	cdata[2];
	char	Text[100];
	BTPOS	POS;
	BOOL	Translated;
	static 	HANDLE	hSegData, hSegMax;
	struct	{int	street_num;
		 long	faddl;
		 long	faddr;
		 long	taddl;
		 long	taddr;
		 }	static Segdata;

	struct	{int	StreetNum;
		 long	MaxHouseNum;
		 long	Segid;
		 }	static SegMaxKey;



 switch(Message)
   {
    case WM_INITDIALOG:
         cwCenter(hWndDlg, 0);
         /* initialize working variables                                */
		BTVar[0].BT_VARTYP=BT_INTEGER;
		BTVar[0].BT_VARLEN=4;
		BTVar[0].BT_VAROFF=0;

		ltime = 0;

/*		strcpy (BTfname, "C:\\test.btr");
		strcpy (BTfname, "C:\\segmax.btr");*/
/*		BT_CREATE (BTfname, 4, FALSE, 1, 1,&BTVar,FALSE, 0, 0, FALSE);*/
/*		hBT = BT_OPEN (BTfname, ltime, 1, 0);*/
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=2;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=2;
	BTVar[2].BT_VARTYP=BT_INTEGER;
	BTVar[2].BT_VARLEN=4;
	BTVar[2].BT_VAROFF=6;
	BT_CREATE ("c:\\test.btr", 2, FALSE, 3, 1,&BTVar,FALSE, 0, 0, FALSE);
	ltime = 0;
	hSegMax = BT_OPEN ("c:\\test.btr", ltime, BT_WRITE, 0);
    hBT = hSegMax;
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         BT_CLOSE (hSegMax);
         BT_CLOSE (hSegData);
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           {
            case TL_NRECS: /* Edit Control                              */
                 break;

            case TL_LB: /* List box                                     */
                switch(HIWORD(lParam))
                    {
                     case CBN_DBLCLK:
						  indx=SendDlgItemMessage (hWndDlg,TL_LB,LB_GETCURSEL,NULL,NULL);
//		                  ShowBTree (hBT,hWndDlg,TL_LB,indx+1);
                          break;
                    }
                 break;

            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 EndDialog(hWndDlg, FALSE);
                 break;
            case TL_UP: /* Button text: "Up"                            */
//                 ShowBTree (hBT,hWndDlg,TL_LB,-4);
                 break;
            case TL_NEXT: /* Button text: "Next"                        */
//                 ShowBTree (hBT,hWndDlg,TL_LB,-3);
                 break;
            case TL_BACK: /* Button text: "Back"                        */
//                 ShowBTree (hBT,hWndDlg,TL_LB,-2);
                 break;
            case TL_FIRST: /* Button text: "First"                        */
//                 ShowBTree (hBT,hWndDlg,TL_LB,-1);
                 break;
            case TL_ADD: /* Button text: "Add"                          */
				nrecs = GetDlgItemInt(hWndDlg,TL_NRECS, &Translated,FALSE);
/*				for (ikey=lastkey,idata=lastkey+1000;ikey>lastkey-nrecs;ikey--,idata++)
				{
					st = BT_PUT (hBT,(LPSTR)&ikey,(LPSTR)&idata);

					if (ikey == 199)
						st = ikey;
				}
				 lastkey = ikey;*/

	nLoaded = 0; 
	for (i=0;i<1000;i++)
	{
    	SegMaxKey.StreetNum=i;
    	SegMaxKey.MaxHouseNum = 100+i;
    	SegMaxKey.Segid = 1000+i;
    	SegMaxData = 10+i;      
    
    	BT_PUT (hSegMax,(LPSTR)&SegMaxKey,(LPSTR)&SegMaxData);
    }
	for (i=900;i;i--)
	{   
    	
		if (i == 890)
			i = 890;
    	SegMaxKey.StreetNum=i;
    	SegMaxKey.MaxHouseNum = 100+i;
    	SegMaxKey.Segid = 1000+i;
    	SegMaxData = 10+i;      
    	BT_DELETE (hSegMax,(LPSTR)&SegMaxKey,(LPSTR)&SegMaxData,FALSE);   
    } 
    
    BT_FIND(hSegMax,(LPSTR)&SegMaxKey,BT_FIRST,BT_ANY,(LPSTR)&SegMaxData);  
    while (!BT_FIND(hSegMax,(LPSTR)&SegMaxKey,BT_NEXT,BT_ANY,(LPSTR)&SegMaxData))
    	i++;


       Done:    //      ShowBTree (hBT,hWndDlg,TL_LB,-1);
                 break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} /* End of TESTLOADMsgProc                                      */

/************************************************************************/
/*                                                                      */
/* Dialog Window Procedure                                              */
/*                                                                      */
/* This procedure is associated with the dialog box that is included in */
/* the function name of the procedure. It provides the service routines */
/* for the events (messages) that occur because the end user operates   */
/* one of the dialog box's buttons, entry fields, or controls.          */
/*                                                                      */
/************************************************************************/

BOOL FAR PASCAL COMPRESSMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{	char	str[128], Key[256], Data[256];
	static	char	BTFile[128], BTFileNew[128];
	int		nchar, isDir;
	int		USize, LSize, USplit, LSplit;
	int		DataLength, stFrom, stTo, NumVar;
	HANDLE	hFrom, hTo;
	BTVARDESC BTVar[8];
	time_t ltime;
	HDC	hDC;
	long	nLoaded, nRecs;   
	HFILE	FidList;
	OFSTRUCT	OFStruct;


 switch(Message)
   {
    case WM_INITDIALOG:
         cwCenter(hWndDlg, 0);
         /* initialize working variables                                */
         _fstrcpy (str,"*.txt");
         nchar=DlgDirListComboBox (hWndDlg,str,CMP_FILE_LB,CMP_DIR,0x4010);
       	 SetDlgItemText (hWndDlg,UPPER_SIZE,"8");
       	 SetDlgItemText (hWndDlg,LOWER_SIZE,"16");
       	 SetDlgItemText (hWndDlg,UPPER_SPLIT,"7");
       	 SetDlgItemText (hWndDlg,LOWER_SPLIT,"15");
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           {
            case UPPER_SIZE: /* Edit Control                            */
                 break;

            case LOWER_SIZE: /* Edit Control                            */
                 break;

            case UPPER_SPLIT: /* Edit Control                           */
                 break;

            case LOWER_SPLIT: /* Edit Control                           */
                 break;

            case CMP_FILE_LB: /* Combo Box                                 */
              {
                switch(HIWORD(lParam))
                    {
                     case CBN_DBLCLK:
                     case CBN_SELCHANGE:
                          isDir = DlgDirSelectComboBox(hWndDlg,
                     		      str,CMP_FILE_LB);
                          if (isDir)
                          	 {nchar = lstrlen (str);
                          	  if (nchar)
	                          	  {	if (str[nchar-1] == '\\')
	                          	  		  _fstrcat (str,"*.txt");
	                          	  	if (str[nchar-1] == '\:')
	                          	  		  _fstrcat (str,"*.txt");
	                          	  }
 					          DlgDirListComboBox (hWndDlg,str,CMP_FILE_LB,CMP_DIR,0x4010);
         					  }
         					  else
         					  	_fstrcpy (BTFile,str);

                          break;
                    }
              }
              break;

            case IDOK:
		       	 GetDlgItemText (hWndDlg,UPPER_SIZE,str,4);
		       	 USize=atoi(str);
		       	 GetDlgItemText (hWndDlg,LOWER_SIZE,str,4);
		       	 LSize=atoi(str);
		       	 GetDlgItemText (hWndDlg,UPPER_SPLIT,str,4);
		       	 USplit=atoi(str);
		       	 GetDlgItemText (hWndDlg,LOWER_SPLIT,str,4);
		       	 LSplit=atoi(str);
		       	 ltime = 0;  
		       	 FidList = GSSiOpenFile (BTFile,&OFStruct,OF_READ);
		       	 while (fgetstring (BTFile,128,FidList))
		       	 {   
		       	 	 SetDlgItemText (hWndDlg,IDC_MESS,BTFile);
				 	 hFrom = BT_OPEN (BTFile, ltime, BT_READ, 0);
				 	 if (!hFrom)
				 	 {	MessageBox(NULL,"Cannot open BTree file",
	                 				    "Error",MB_OK|MB_ICONQUESTION|MB_TASKMODAL);
	                 }
	                 else
	                 {	_fstrcpy(BTFileNew,BTFile);
	                 	BTFileNew[_fstrlen(BTFileNew)-3]='\0';
						_fstrcat (BTFileNew,"cmp");
						BT_SET_PARMS (USize,LSize,USplit,LSplit);
						BT_GET_DEF (hFrom,&DataLength,&NumVar,&BTVar);
						BT_CREATE (BTFileNew, DataLength, FALSE, NumVar, 1,&BTVar,FALSE, 0, 0, FALSE);
						ltime = 0;
						hTo = BT_OPEN (BTFileNew, ltime, BT_WRITE, 0);
						stFrom = BT_FIND (hFrom,(LPSTR)&Key,BT_FIRST,BT_ANY,(LPSTR)&Data);
						nLoaded = 0;
						nRecs = BT_NUM_IN_INDEX (hFrom);
						while (!stFrom)
						{
				    		BT_PUT (hTo,(LPSTR)&Key,(LPSTR)&Data);
							stFrom = BT_FIND (hFrom,(LPSTR)&Key,BT_NEXT,BT_ANY,(LPSTR)&Data);
						    if (!PctBox (GetDlgItem(hWndDlg,IDC_STATUS), nRecs, ++nLoaded,10)) goto Return;
						}
						Return: BT_CLOSE (hFrom);
						BT_CLOSE (hTo); 
						remove (BTFile);
						rename (BTFileNew,BTFile);  
					}
				 }
                 EndDialog(hWndDlg, TRUE);
                 break;
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 EndDialog(hWndDlg, FALSE);
                 break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} /* End of COMPRESSMsgProc                                      */

BOOL FAR PASCAL SPLITMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{	char	mess[256], File[256], Dir[256];
	HFILE	FidList;
	OFSTRUCT	OFStruct;


 switch(Message)
   {
    case WM_INITDIALOG:
         cwCenter(hWndDlg, 0); 
         SendDlgItemMessage (hWndDlg,IDC_FLOPPY,BM_SETCHECK,TRUE,0);
         /* initialize working variables                                */
       	 SetDlgItemText (hWndDlg,IDC_DIR,"C:\\SPLIT");
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           {
            case IDC_FIND:        
             	_fstrcpy (gszFilter,"All Files(*.*)|*.*|");
           		if (GetFileName (hWndDlg,File,0))
           			SetDlgItemText (hWndDlg,IDC_FILE,File);
                break;

            case IDOK: 
            {
            	long	nbytes, TotBytes, Done=0;
            	HFILE	Fid, Fid2;
            	LPSTR   pbuf;
            	HANDLE  hbuf; 
            	char	NewFile[256], Name[34], mess[256];
            	int		fnum=1, nread=0, MaxRead=22;
            	
                 if (SendDlgItemMessage (hWndDlg,IDC_1MEG,BM_GETCHECK,0,0)) MaxRead = 15;
                 if (SendDlgItemMessage (hWndDlg,IDC_2MEG,BM_GETCHECK,0,0)) MaxRead = 15 + 16;
                 if (SendDlgItemMessage (hWndDlg,IDC_3MEG,BM_GETCHECK,0,0)) MaxRead = 15 + 16*2;
                 if (SendDlgItemMessage (hWndDlg,IDC_4MEG,BM_GETCHECK,0,0)) MaxRead = 15 + 16*3;
                 if (SendDlgItemMessage (hWndDlg,IDC_FLOPPY,BM_GETCHECK,0,0)) MaxRead = 22;
		       	 GetDlgItemText (hWndDlg,IDC_FILE,File,sizeof(File));
		       	 GetDlgItemText (hWndDlg,IDC_DIR,Dir,sizeof(Dir));
		       	 mkdir (Dir); 
		       	 
		       	 _splitpath (File,NULL,NULL,Name,NULL);
		       	 hbuf = GSSiGlobAlloc (GHND,UINT_MAX);
		       	 pbuf = GlobalLock (hbuf);
		       	 Fid = GSSiOpenFile (File,&OFStruct,OF_READ); 
		       	 sprintf (NewFile,"%s\\%s.%3.3i",Dir,Name,fnum);
		       	 Fid2 = GSSiOpenFile (NewFile,&OFStruct,OF_CREATE); 
		       	 TotBytes = _llseek (Fid,0,2);
		       	 _llseek (Fid,0,0);
		       	 while ((nbytes = _lread (Fid,pbuf,UINT_MAX)))
		       	 {  
		       	 	Done+=nbytes; 
		       	 	nread++;
		       	 	if (nread > MaxRead)
		       	 	{   
		       	 		fnum++;
		       	 		_lclose (Fid2);
		       	 		nread = 1;  
		       	 		sprintf (mess,"%s written",NewFile);
		       	 		SetDlgItemText (hWndDlg,IDC_MESS,mess);
				       	sprintf (NewFile,"%s\\%s.%3.3i",Dir,Name,fnum);
				       	Fid2 = GSSiOpenFile (NewFile,&OFStruct,OF_CREATE);
		       	 	} 
		       	 	_lwrite (Fid2,pbuf,nbytes);   
		       	 	PctBox (GetDlgItem (hWndDlg,IDC_STATUS2), TotBytes,Done,0);
				 }
				 GlobalUnlock (hbuf);
				 GlobalFree (hbuf);
				 _lclose (Fid2);
				 _lclose (Fid);
                 EndDialog(hWndDlg, TRUE); 
            }
                 break;
            case IDC_RESTORE: 
            {
            	long	nbytes, TotBytes=LONG_MAX;
            	HFILE	Fid, Fid2;
            	LPSTR   pbuf;
            	HANDLE  hbuf; 
            	char	NewFile[256], Name[34];
            	int		fnum=1, nread=0; 
            	LPSTR	lpdot;
            	
		       	 if (!GetDlgItemText (hWndDlg,IDC_FILE,File,sizeof(File)))
		       	 {  
		       	 	MessageBox (hWndDlg,"Output file name missing",NULL,MB_ICONEXCLAMATION);
		       	 	break;
		       	 }
		       	 
		       	 Fid = GSSiOpenFile (File,&OFStruct,OF_CREATE); 
		       	 lpdot = _fstrrchr (File,'.');
		       	 *lpdot = 0;
		       	 hbuf = GSSiGlobAlloc (GHND,UINT_MAX);
		       	 pbuf = GlobalLock (hbuf);
		       	 sprintf (NewFile,"%s.%3.3i",File,fnum);
		       	 Fid2 = GSSiOpenFile (NewFile,&OFStruct,OF_READ); 
		       	 while (Fid2 != HFILE_ERROR)
		       	 {
			       	 while ((nbytes = _lread (Fid2,pbuf,UINT_MAX)))
			       	 {   
			       	 	_lwrite (Fid,pbuf,nbytes);
					 } 
					 _lclose (Fid2);
					 fnum++;  
			       	 sprintf (NewFile,"%s.%3.3i",File,fnum);
			       	 Fid2 = GSSiOpenFile (NewFile,&OFStruct,OF_READ); 
				 }
				 GlobalUnlock (hbuf);
				 GlobalFree (hbuf);
				 _lclose (Fid);
                 EndDialog(hWndDlg, TRUE); 
            }
                 break;
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 EndDialog(hWndDlg, FALSE);
                 break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} 

int nCwRegisterClasses(void)
{ HBITMAP hBmp;

 WNDCLASS   wndclass;    /* struct to define a window class             */
 memset(&wndclass, 0x00, sizeof(WNDCLASS));

 /* load WNDCLASS with window's characteristics                         */
 wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNWINDOW;
 wndclass.lpfnWndProc = WndProc;
 /* Extra storage for Class and Window objects                          */
 wndclass.cbClsExtra = 0;
 wndclass.cbWndExtra = 0;
 wndclass.hInstance = hInst;
 wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
 wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
 /* Create brush for erasing background                                 */
 hBmp = LoadBitmap (hInst,"BACKGROUND_1");
 hBackBrush1 = CreatePatternBrush (hBmp); 
 DeleteObject (hBmp);
 wndclass.hbrBackground = hBackBrush1 ;
 wndclass.lpszMenuName = szAppName;   /* Menu Name is App Name */
 wndclass.lpszClassName = szAppName; /* Class Name is App Name */
 if(!RegisterClass(&wndclass))
   return -1;


 return(0);
} /* End of nCwRegisterClasses                                          */

void cwCenter(hWnd, top)
HWND hWnd;
int top;
{
 POINT      pt;
 RECT       swp;
 RECT       rParent;
 int        iwidth;
 int        iheight;

 /* get the rectangles for the parent and the child                     */
 GetWindowRect(hWnd, &swp);
 GetClientRect(hWndMain, &rParent);

 /* calculate the height and width for MoveWindow                       */
 iwidth = swp.right - swp.left;
 iheight = swp.bottom - swp.top;

 /* find the center point and convert to screen coordinates             */
 pt.x = (rParent.right - rParent.left) / 2;
 pt.y = (rParent.bottom - rParent.top) / 2;
 ClientToScreen(hWndMain, &pt);

 /* calculate the new x, y starting point                               */
 pt.x = pt.x - (iwidth / 2);
 pt.y = pt.y - (iheight / 2);

 /* top will adjust the window position, up or down                     */
 if(top)
   pt.y = pt.y + top;

 /* move the window                                                     */
 MoveWindow(hWnd, pt.x, pt.y, iwidth, iheight, FALSE);
}

void CwUnRegisterClasses(void)
{
 WNDCLASS   wndclass;    /* struct to define a window class             */
 memset(&wndclass, 0x00, sizeof(WNDCLASS));

 UnregisterClass(szAppName, hInst);
}    /* End of CwUnRegisterClasses                                      */

int LoadWells (HWND hWnd)
{   char BTfname[100];
	BTVARDESC BTVar[8], *pVars;
	HANDLE hBT, hVars;
	time_t ltime;
	HDC	hDC;
	int	   st,  i, i2key;
	long	ikey, TotRecs, iseg;
	long	idata, nRecs, nLoaded;
	char	cdata[2];
	char	Text[512];
	char	Fname[256];
	OFSTRUCT	OFStruct;
	LPSTR	pInRec, lpStrName;
	int		strnum;
	char	cTotRecs[7], VTYPE[2];
	int		NumVars, NumIndex, ioff;
	struct	{int	street_num;
			 long	faddl;
			 long	faddr;
			 long	taddl;
			 long	taddr;
			 }	data;
	char	TAG[30];
	int		FidData;
	FILE	*Fid;
	struct {int		Beg;
			int		Len;
			int		Type;
			char	Name[34];
			} FldInfo;
	int		NumFields, Reclen, len;
	long	Refno, Offset;
	char	CRef[16];
	GWDHEADER GWDHead;
	GWFLDINFO GWFldInfo;

	_fstrcpy (Fname,"j:\\cwi.gwd");
	FidData = OpenFile (Fname,&OFStruct,OF_CREATE);
	Fid = fopen ("j:\\cwi.def","r");
	GWDHead.NumFields=0;
	GWDHead.NumIndex=1;
	GWDHead.Version=0;
	GWDHead.Reclen=0;
	GWDHead.NumIndexFields[0]=1;
	GWDHead.IndexFields[0][0]=1;
	_lwrite (FidData,&GWDHead,sizeof(GWDHEADER)-sizeof(GWFLDINFO));
	while (fgetss (Text,256,Fid))
	{
		Text[45]='\0';
		FldInfo.Len = atoi (&Text[42]);
		Text[38]='\0';
		FldInfo.Beg = atoi (&Text[33])-1;
		Text[32]='\0';
		_fstrcpy (FldInfo.Name,Text);
		GWDHead.NumFields++;
		GWDHead.Reclen = max (GWDHead.Reclen,FldInfo.Beg+FldInfo.Len-1);
		_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	}
	_llseek (FidData,0,0);
	_lwrite (FidData,&GWDHead,sizeof(GWDHEADER)-sizeof(GWFLDINFO));
	_llseek (FidData,0,2);

	fclose (Fid);

	NumVars = 1;
	NumIndex = 1;

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);
	ltime = 0;

	BT_SET_PARMS (8,16,4,8);
	strcpy (BTfname, "j:\\cwi.in1");
	pVars->BT_VARLEN=4;
	pVars->BT_VARTYP=BT_INTEGER;
	pVars->BT_VAROFF=0;
	BT_CREATE (BTfname, 4, FALSE, 1, 1,pVars,FALSE, 0, 0, FALSE);
	hBT = BT_OPEN (BTfname, ltime, 1, 0);

	hDC = GetDC(hWnd);
	nRecs = 50000;
	nLoaded=0;
	Fid = fopen ("j:\\cwi.txt","r");
	while (fgetss (Text,512,Fid))
	{
		_fstrncpy (CRef,Text,11);
		CRef[11]='\0';
		Refno = atol (CRef);
		Offset = _llseek (FidData,0,1);
		st = BT_PUT (hBT,(LPSTR)&Refno,(LPSTR)&Offset);
		len = _fstrlen(Text);
		_lwrite (FidData,&len,2);
		_lwrite (FidData,Text,len);
		if (!PctBox (hWnd, hDC, nRecs, ++nLoaded)) TotRecs=0;
	}

	ReleaseDC(hWnd, hDC);
	BT_CLOSE (hBT);
	_lclose (FidData);

	return (st);


}


int CreateDOTRoute (HWND hWnd)
{   char BTfname[100];
	BTVARDESC BTVar[8], *pVars;
	HANDLE hBT, hVars;
	time_t ltime;
	HDC	hDC;
	int	   st,  i, i2key;
	long	ikey, TotRecs, iseg;
	long	idata, nRecs, nLoaded;
	char	cdata[2];
	char	Text[512];
	char	Fname[256];
	OFSTRUCT	OFStruct;
	LPSTR	pInRec, lpStrName;
	int		strnum;
	char	cTotRecs[7], VTYPE[2];
	int		NumVars, NumIndex, ioff;
	struct	{
			 long	faddl;
			 long	faddr;
			 long	taddl;
			 long	taddr;
			 }	data;
	char	TAG[30];
	int		FidData;
	FILE	*Fid;
	struct {int		Beg;
			int		Len;
			int		Type;
			char	Name[34];
			} FldInfo;

	struct {
			    long	TLID;
			    char	Route[6];
			} Key;

	struct {
				int		Status;
				char	Desc[32];
				long	Price;
				long	TLID;
				long	Frame;
				long	PID;
				double	AddressPointx, AddressPointy;
			} PropData;

    struct {
    			int		Clip;
    			long	Frame;
    		}Dot2Key;
    struct {
    			char	Route[6];
    			char	DistMA[2];
    			char	Dir;
    			char	Date[6];
    			char	RefPoint[10];
    			char	Interval[4];
    		}Dot2Dat;

    struct {
    			int		Clip;
    			int		Segment;
    		}Dot3Key;
    struct {
    			long	FromFrame;
    			long	ToFrame;
    		}Dot3Dat;

    struct {
    			int		Clip;
    			int		Segment;
    			int		Sequence;
    		}Dot4Key;
    struct {
    			long	TLID;
    		}Dot4Dat;

    struct {
    			long	TLID;
    			int		Clip;
    		}Dot5Key;
    int	Dot5Dat;




	int		NumFields, Reclen, len;
	long	Refno, Offset;
	char	CRef[16];
	GWDHEADER GWDHead;
	GWFLDINFO GWFldInfo;
	int	Dot1Key;
	char	Dot1Dat[16];

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);
	ltime = 0;

	BT_SET_PARMS (8,16,4,8);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=2;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_CHAR;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=4;
	BT_SET_VERSION (1);
	BT_CREATE ("c:\\dot1.btr", sizeof(Dot1Dat), FALSE, 1, 1,&BTVar,FALSE, 0, 0, FALSE);
	ltime = 0;
	hBT = BT_OPEN ("c:\\dot1.btr", ltime, BT_WRITE, 0);
	Dot1Key=0;
	Dot1Dat[0]='\0';
		st = BT_PUT (hBT,(LPSTR)&Dot1Key,(LPSTR)&Dot1Dat);
	BT_CLOSE (hBT);

	BT_SET_PARMS (8,16,4,8);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=2;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=2;
	BT_SET_VERSION (1);
	BT_CREATE ("c:\\dot2.btr", sizeof(Dot2Dat), FALSE, 2, 1,&BTVar,FALSE, 0, 0, FALSE);
	ltime = 0;
	hBT = BT_OPEN ("c:\\dot2.btr", ltime, BT_WRITE, 0);
	Dot2Key.Clip=0;
	Dot2Key.Frame=0;
		st = BT_PUT (hBT,(LPSTR)&Dot2Key,(LPSTR)&Dot2Dat);
	BT_CLOSE (hBT);


	BT_SET_PARMS (8,16,4,8);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=2;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=2;
	BTVar[1].BT_VAROFF=2;
	BT_SET_VERSION (1);
	BT_CREATE ("c:\\dot3.btr", sizeof(Dot3Dat), FALSE, 2, 1,&BTVar,FALSE, 0, 0, FALSE);
	ltime = 0;
	hBT = BT_OPEN ("c:\\dot3.btr", ltime, BT_WRITE, 0);
	Dot3Key.Clip=0;
	Dot3Key.Segment=0;
		st = BT_PUT (hBT,(LPSTR)&Dot3Key,(LPSTR)&Dot3Dat);
	BT_CLOSE (hBT);


	BT_SET_PARMS (8,16,4,8);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=2;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=2;
	BTVar[1].BT_VAROFF=2;
	BTVar[2].BT_VARTYP=BT_INTEGER;
	BTVar[2].BT_VARLEN=2;
	BTVar[2].BT_VAROFF=4;
	BT_SET_VERSION (1);
	BT_CREATE ("c:\\dot4.btr", sizeof(Dot4Dat), FALSE, 3, 1,&BTVar,FALSE, 0, 0, FALSE);
	ltime = 0;
	hBT = BT_OPEN ("c:\\dot4.btr", ltime, BT_WRITE, 0);
	Dot4Key.Clip=0;
	Dot4Key.Segment=0;
		st = BT_PUT (hBT,(LPSTR)&Dot4Key,(LPSTR)&Dot4Dat);
	BT_CLOSE (hBT);


	BT_SET_PARMS (8,16,4,8);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=2;
	BTVar[1].BT_VAROFF=4;
	BT_SET_VERSION (1);
	BT_CREATE ("c:\\dot5.btr", sizeof(Dot5Dat), FALSE, 2, 1,&BTVar,FALSE, 0, 0, FALSE);
	ltime = 0;
	hBT = BT_OPEN ("c:\\dot5.btr", ltime, BT_WRITE, 0);
	Dot5Key.TLID=0;
		st = BT_PUT (hBT,(LPSTR)&Dot5Key,(LPSTR)&Dot5Dat);
	BT_CLOSE (hBT);

	return (st);


}



int CreateProp (HWND hWnd)
{   char BTfname[100];
	BTVARDESC BTVar[8], *pVars;
	HANDLE hBT, hVars;
	time_t ltime;
	HDC	hDC;
	int	   st,  i, i2key;
	long	ikey, TotRecs, iseg;
	long	idata, nRecs, nLoaded;
	char	cdata[2];
	char	Text[512];
	char	Fname[256];
	OFSTRUCT	OFStruct;
	LPSTR	pInRec, lpStrName;
	int		strnum;
	char	cTotRecs[7], VTYPE[2];
	int		NumVars, NumIndex, ioff;
	struct	{int	street_num;
			 long	faddl;
			 long	faddr;
			 long	taddl;
			 long	taddr;
			 }	data;
	char	TAG[30];
	int		FidData;
	FILE	*Fid;
	struct {int		Beg;
			int		Len;
			int		Type;
			char	Name[34];
			} FldInfo;

	struct {
			    long	AgentID;
			    int		Client;
			    int		sequence;
			} PropKey;

	struct {
				int		Status;
				char	Desc[32];
				long	Price;
				long	TLID;
				long	Frame;
				long	PID;
				double	AddressPointx, AddressPointy;
			} PropData;


	int		NumFields, Reclen, len;
	long	Refno, Offset;
	char	CRef[16];
	GWDHEADER GWDHead;
	GWFLDINFO GWFldInfo;

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);
	ltime = 0;

	BT_SET_PARMS (8,16,4,8);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=2;
	BTVar[1].BT_VAROFF=4;
	BTVar[2].BT_VARTYP=BT_INTEGER;
	BTVar[2].BT_VARLEN=2;
	BTVar[2].BT_VAROFF=6;
	BT_SET_VERSION (1);
	BT_CREATE ("c:\\property.btr", sizeof(PropData), FALSE, 3, 1,&BTVar,FALSE, 0, 0, FALSE);
	ltime = 0;
	hBT = BT_OPEN ("c:\\property.btr", ltime, BT_WRITE, 0);
	PropKey.AgentID=-1;
		st = BT_PUT (hBT,(LPSTR)&PropKey,(LPSTR)&PropData);
	BT_CLOSE (hBT);

	return (st);


}


HANDLE TestOpen (HWND hWnd)
{   char BTfname[100];
	BTVARDESC BTVar[8], *pVars;
	HANDLE hBT, hVars;
	time_t ltime;
	HDC	hDC;
	int	   st,  i, i2key;
	long	ikey, TotRecs, iseg;
	long	idata, nRecs, nLoaded;
	char	cdata[2];
	char	Text[512];
	char	Fname[256];
	OFSTRUCT	OFStruct;
	LPSTR	pInRec, lpStrName;
	int		strnum;
	char	cTotRecs[7], VTYPE[2];
	int		NumVars, NumIndex, ioff;
	struct	{int	street_num;
			 long	faddl;
			 long	faddr;
			 long	taddl;
			 long	taddr;
			 }	data;
	char	TAG[30];
	int		FidData;
	FILE	*Fid;
	struct {int		Beg;
			int		Len;
			int		Type;
			char	Name[34];
			} FldInfo;

	struct {
			    long	AgentID;
			    int		Client;
			    int		sequence;
			} PropKey;

	struct {
				int		Status;
				char	Desc[32];
				long	Price;
				long	TLID;
				long	Frame;
				long	PID;
				double	AddressPointx, AddressPointy;
			} PropData;


	int		NumFields, Reclen, len;
	long	Refno, Offset;
	char	CRef[16];
	GWDHEADER GWDHead;
	GWFLDINFO GWFldInfo;

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);
	ltime = 0;

	BT_SET_PARMS (8,16,4,8);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=2;
	BTVar[1].BT_VAROFF=4;
	BTVar[2].BT_VARTYP=BT_INTEGER;
	BTVar[2].BT_VARLEN=2;
	BTVar[2].BT_VAROFF=6;
	BT_SET_VERSION (1);
	ltime = 0;
	hBT = BT_OPEN ("d:\\gwizdata\\property.btr", ltime, BT_READ, 0);
	PropKey.AgentID=-1;
		st = BT_PUT (hBT,(LPSTR)&PropKey,(LPSTR)&PropData);

	return (hBT);


}

int CreateClients (HWND hWnd)
{   char BTfname[100];
	BTVARDESC BTVar[8], *pVars;
	HANDLE hBT, hVars;
	time_t ltime;
	HDC	hDC;
	int	   st,  i, i2key;
	long	ikey, TotRecs, iseg;
	long	idata, nRecs, nLoaded;
	char	cdata[2];
	char	Text[512];
	char	Fname[256];
	OFSTRUCT	OFStruct;
	LPSTR	pInRec, lpStrName;
	int		strnum;
	char	cTotRecs[7], VTYPE[2];
	int		NumVars, NumIndex, ioff;
	struct	{int	street_num;
			 long	faddl;
			 long	faddr;
			 long	taddl;
			 long	taddr;
			 }	data;
	char	TAG[30];
	int		FidData;
	FILE	*Fid;
	struct {int		Beg;
			int		Len;
			int		Type;
			char	Name[34];
			} FldInfo;

	struct {
			    long	AgentID;
			    char	Name[32];
			} ClientKey;

	struct {
				char	Status[8];
				long	TLID;
				long	Frame;
				double	AddressPointx, AddressPointy;
			} PropData;


	int		NumFields, Reclen, len, ClientID=0;
	long	Refno, Offset;
	char	CRef[16];
	GWDHEADER GWDHead;
	GWFLDINFO GWFldInfo;

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);
	ltime = 0;

	BT_SET_PARMS (8,16,4,8);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_CHAR;
	BTVar[1].BT_VARLEN=32;
	BTVar[1].BT_VAROFF=4;
	BT_CREATE ("c:\\clients.btr", 2, FALSE, 2, 1,&BTVar,FALSE, 0, 0, FALSE);
	ltime = 0;
	hBT = BT_OPEN ("c:\\clients.btr", ltime, BT_WRITE, 0);
	ClientKey.AgentID=-1;
		st = BT_PUT (hBT,(LPSTR)&ClientKey,(LPSTR)&ClientID);
	BT_CLOSE (hBT);

	return (st);


}





int LoadParcels (HWND hWnd)
{   char BTfname[100];
	BTVARDESC BTVar[8], *pVars;
	HANDLE hBT, hVars, hBT2, hBT3;
	time_t ltime;
	HDC	hDC;
	int	   st,  i, i2key;
	long	ikey, TotRecs, iseg;
	long	idata, nRecs, nLoaded;
	char	cdata[2];
	char	Text[512];
	char	Fname[256];
	OFSTRUCT	OFStruct;
	LPSTR	pInRec, lpStrName;
	int		strnum;
	char	cTotRecs[7], VTYPE[2];
	int		NumVars, NumIndex, ioff;
	struct	{int	street_num;
			 long	faddl;
			 long	faddr;
			 long	taddl;
			 long	taddr;
			 }	data;
	char	TAG[30];
	int		FidData;
	FILE	*Fid;
	struct {int		Beg;
			int		Len;
			int		Type;
			char	Name[34];
			} FldInfo;
	int		NumFields, Reclen;
	long	Refno, Offset;
	char	CRef[16];
	GWDHEADER GWDHead;
	GWFLDINFO GWFldInfo;
	char	Address[28];
	int		len;

	_fstrcpy (Fname,"j:\\parcels.gwd");
	FidData = OpenFile (Fname,&OFStruct,OF_CREATE);
	Fid = fopen ("j:\\parcels.def","r");
	GWDHead.NumFields=0;
	GWDHead.NumIndex=3;
	GWDHead.Version=0;
	GWDHead.Reclen=0;
	GWDHead.NumIndexFields[0]=1;
	GWDHead.IndexFields[0][0]=1;
	_lwrite (FidData,&GWDHead,sizeof(GWDHEADER)-sizeof(GWFLDINFO));
	while (fgetss (Text,256,Fid))
	{
		Text[45]='\0';
		FldInfo.Len = atoi (&Text[42]);
		Text[38]='\0';
		FldInfo.Beg = atoi (&Text[33])-1;
		Text[32]='\0';
		_fstrcpy (FldInfo.Name,Text);
		GWDHead.NumFields++;
		GWDHead.Reclen = max (GWDHead.Reclen,FldInfo.Beg+FldInfo.Len-1);
		_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	}
	_llseek (FidData,0,0);
	_lwrite (FidData,&GWDHead,sizeof(GWDHEADER)-sizeof(GWFLDINFO));
	_llseek (FidData,0,2);

	fclose (Fid);

	NumVars = 1;
	NumIndex = 1;

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);

    pVars = LocalUnlock(hVars);
    pVars = LocalLock(hVars);

	ltime = 0;

	BT_SET_PARMS (8,16,4,8);
	strcpy (BTfname, "j:\\parcels.in1");
	pVars->BT_VARLEN=4;
	pVars->BT_VARTYP=BT_INTEGER;
	pVars->BT_VAROFF=0;
	BT_CREATE (BTfname, 4, FALSE, 1, 1,pVars,FALSE, 0, 0, FALSE);
	hBT = BT_OPEN (BTfname, ltime, 1, 0);

	strcpy (BTfname, "j:\\parcels.in2");
	pVars->BT_VARLEN=28;
	pVars->BT_VARTYP=BT_CHAR;
	pVars->BT_VAROFF=0;
	BT_CREATE (BTfname, 4, FALSE, 1, 1,pVars,FALSE, 0, 0, FALSE);
	hBT2 = BT_OPEN (BTfname, ltime, 1, 0);

	strcpy (BTfname, "j:\\parcels.in3");
	pVars->BT_VARLEN=35;
	pVars->BT_VARTYP=BT_CHAR;
	pVars->BT_VAROFF=0;
	BT_CREATE (BTfname, 4, FALSE, 1, 1,pVars,FALSE, 0, 0, FALSE);
	hBT3 = BT_OPEN (BTfname, ltime, 1, 0);

	hDC = GetDC(hWnd);
	nRecs = 360000;
	nLoaded=0;
	Fid = fopen ("j:\\parcelsa.txt","r");
	while (fgetss (Text,512,Fid))
	{
		_fstrncpy (CRef,Text,12);
		CRef[12]='\0';
		Refno = atol (CRef);
		Offset = _llseek (FidData,0,1);
		/*st = BT_PUT (hBT,(LPSTR)&Refno,(LPSTR)&Offset);*/
		_fmemmove (Address,&Text[89],20);
		_fmemmove (&Address[20],&Text[81],8);
	/*	st = BT_PUT (hBT2,(LPSTR)Address,(LPSTR)&Offset)
		st = BT_PUT (hBT3,(LPSTR)&Text[33],(LPSTR)&Offset);*/
		len = _fstrlen(Text);
		_lwrite (FidData,&len,2);
		_lwrite (FidData,Text,len);
		if (!PctBox (hWnd, hDC, nRecs, ++nLoaded)) TotRecs=0;
	}

	ReleaseDC(hWnd, hDC);
	BT_CLOSE (hBT);
	BT_CLOSE (hBT2);
	BT_CLOSE (hBT3);
	_lclose (FidData);

	return (st);


}


int LoadParcelsSub (HWND hWnd)
{   char BTfname[100];
	BTVARDESC BTVar[8], *pVars;
	HANDLE hBT, hVars, hBT2, hBT3, hBTAll, hRefIdx;
	time_t ltime;
	HDC	hDC;
	int	   st,  i, i2key, FidDataAll, st2;
	long	ikey, TotRecs, iseg;
	long	idata, nRecs, nLoaded;
	char	cdata[2];
	char	Text[512];
	char	Fname[256];
	OFSTRUCT	OFStruct;
	LPSTR	pInRec, lpStrName;
	int		strnum;
	char	cTotRecs[7], VTYPE[2];
	int		NumVars, NumIndex, ioff;
	struct	{int	street_num;
			 long	faddl;
			 long	faddr;
			 long	taddl;
			 long	taddr;
			 }	data;
	char	TAG[30];
	int		FidData;
	FILE	*Fid;
	struct {int		Beg;
			int		Len;
			int		Type;
			char	Name[34];
			} FldInfo;
	int		NumFields, Reclen;
	long	Refno, Offset, OffsetAll;
	char	CRef[16];
	GWDHEADER GWDHead;
	GWFLDINFO GWFldInfo;
	char	Address[28];
	int		len;
	struct	{int	FileInIndex;
			 long	Segment;
			 WORD	Offset;
			 }	RefIdxData;


	_fstrcpy (Fname,"j:\\parcels.gwd");
	FidData = OpenFile (Fname,&OFStruct,OF_CREATE);
	Fid = fopen ("j:\\parcels.def","r");
	GWDHead.NumFields=0;
	GWDHead.NumIndex=3;
	GWDHead.Version=0;
	GWDHead.Reclen=0;
	GWDHead.NumIndexFields[0]=1;
	GWDHead.IndexFields[0][0]=1;
	_lwrite (FidData,&GWDHead,sizeof(GWDHEADER)-sizeof(GWFLDINFO));
	while (fgetss (Text,256,Fid))
	{
		Text[45]='\0';
		FldInfo.Len = atoi (&Text[42]);
		Text[38]='\0';
		FldInfo.Beg = atoi (&Text[33])-1;
		Text[32]='\0';
		_fstrcpy (FldInfo.Name,Text);
		GWDHead.NumFields++;
		GWDHead.Reclen = max (GWDHead.Reclen,FldInfo.Beg+FldInfo.Len-1);
		_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	}
	_llseek (FidData,0,0);
	_lwrite (FidData,&GWDHead,sizeof(GWDHEADER)-sizeof(GWFLDINFO));
	_llseek (FidData,0,2);

	fclose (Fid);

	NumVars = 1;
	NumIndex = 1;

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);

    pVars = LocalUnlock(hVars);
    pVars = LocalLock(hVars);

	ltime = 0;
	strcpy (BTfname, "j:\\gwiz\\attribut\\parcels.in1");
	hBTAll = BT_OPEN (BTfname, ltime, 0, 0);
	strcpy (Fname, "j:\\gwiz\\attribut\\parcels.gwd");
	FidDataAll = OpenFile (Fname,&OFStruct,OF_READ);

	BT_SET_PARMS (8,16,4,8);
	strcpy (BTfname, "j:\\parcels.in1");
	pVars->BT_VARLEN=4;
	pVars->BT_VARTYP=BT_INTEGER;
	pVars->BT_VAROFF=0;
	BT_CREATE (BTfname, 4, FALSE, 1, 1,pVars,FALSE, 0, 0, FALSE);
	hBT = BT_OPEN (BTfname, ltime, 1, 0);

	strcpy (BTfname, "j:\\parcels.in2");
	pVars->BT_VARLEN=28;
	pVars->BT_VARTYP=BT_CHAR;
	pVars->BT_VAROFF=0;
	BT_CREATE (BTfname, 4, FALSE, 1, 1,pVars,FALSE, 0, 0, FALSE);
	hBT2 = BT_OPEN (BTfname, ltime, 1, 0);

	strcpy (BTfname, "j:\\parcels.in3");
	pVars->BT_VARLEN=13;
	pVars->BT_VARTYP=BT_CHAR;
	pVars->BT_VAROFF=0;
	BT_CREATE (BTfname, 4, FALSE, 1, 1,pVars,FALSE, 0, 0, FALSE);
	hBT3 = BT_OPEN (BTfname, ltime, 1, 0);

	hDC = GetDC(hWnd);
	nRecs = 350000;
	nLoaded=0;

/*	strcpy (BTfname, "f:\\gwiz\\maplib\\baselayr\\refindex.rin");*/
	_fstrcpy (BTfname,"C:\\REFINDEX.RIN");
	hRefIdx = BT_OPEN (BTfname, ltime, 0, 0);
	st = BT_FIND (hRefIdx,(LPSTR)&Refno,BT_FIRST,BT_ANY,(LPSTR)&RefIdxData);
	while (!st)
	{
		st2 = BT_FIND (hBTAll,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)&OffsetAll);
		if (!st2)
		{
	    	_llseek (FidDataAll,OffsetAll,0);
	       	_lread (FidDataAll,&len,2);
	       	_lread (FidDataAll,Text,len);
			Offset = _llseek (FidData,0,1);
			st = BT_PUT (hBT,(LPSTR)&Refno,(LPSTR)&Offset);
			_fmemmove (Address,&Text[89],20);
			_fmemmove (&Address[20],&Text[81],8);
			st = BT_PUT (hBT2,(LPSTR)Address,(LPSTR)&Offset);
			st = BT_PUT (hBT3,(LPSTR)&Text[12],(LPSTR)&Offset);
			len = _fstrlen(Text);
			_lwrite (FidData,&len,2);
			_lwrite (FidData,Text,len);
		}
		st = BT_FIND (hRefIdx,(LPSTR)&Refno,BT_NEXT,BT_ANY,(LPSTR)&RefIdxData);
	}
	BT_CLOSE (hRefIdx);

/*	strcpy (BTfname, "j:\\gwiz\\maplib\\baselayr\\pc21868.rin");
	hRefIdx = BT_OPEN (BTfname, ltime, 1, 0);
	st = BT_FIND (hRefIdx,(LPSTR)&Refno,BT_FIRST,BT_ANY,(LPSTR)&RefIdxData);
	while (!st)
	{
		st2 = BT_FIND (hBTAll,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)&OffsetAll);
		if (!st2)
		{
	    	_llseek (FidDataAll,OffsetAll,0);
	       	_lread (FidDataAll,&len,2);
	       	_lread (FidDataAll,Text,len);
			Offset = _llseek (FidData,0,1);
			st = BT_PUT (hBT,(LPSTR)&Refno,(LPSTR)&Offset);
			_fmemmove (Address,&Text[89],20);
			_fmemmove (&Address[20],&Text[81],8);
			st = BT_PUT (hBT2,(LPSTR)Address,(LPSTR)&Offset);
			st = BT_PUT (hBT3,(LPSTR)&Text[12],(LPSTR)&Offset);
			len = _fstrlen(Text);
			_lwrite (FidData,&len,2);
			_lwrite (FidData,Text,len);
		}
		st = BT_FIND (hRefIdx,(LPSTR)&Refno,BT_NEXT,BT_ANY,(LPSTR)&RefIdxData);
	}
	BT_CLOSE (hRefIdx);

	strcpy (BTfname, "j:\\gwiz\\maplib\\baselayr\\pc21968.rin");
	hRefIdx = BT_OPEN (BTfname, ltime, 1, 0);
	st = BT_FIND (hRefIdx,(LPSTR)&Refno,BT_FIRST,BT_ANY,(LPSTR)&RefIdxData);
	while (!st)
	{
		st2 = BT_FIND (hBTAll,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)&OffsetAll);
		if (!st2)
		{
	    	_llseek (FidDataAll,OffsetAll,0);
	       	_lread (FidDataAll,&len,2);
	       	_lread (FidDataAll,Text,len);
			Offset = _llseek (FidData,0,1);
			st = BT_PUT (hBT,(LPSTR)&Refno,(LPSTR)&Offset);
			_fmemmove (Address,&Text[89],20);
			_fmemmove (&Address[20],&Text[81],8);
			st = BT_PUT (hBT2,(LPSTR)Address,(LPSTR)&Offset);
			st = BT_PUT (hBT3,(LPSTR)&Text[12],(LPSTR)&Offset);
			len = _fstrlen(Text);
			_lwrite (FidData,&len,2);
			_lwrite (FidData,Text,len);
		}
		st = BT_FIND (hRefIdx,(LPSTR)&Refno,BT_NEXT,BT_ANY,(LPSTR)&RefIdxData);
	}
	BT_CLOSE (hRefIdx);*/

	ReleaseDC(hWnd, hDC);
	BT_CLOSE (hBTAll);
	BT_CLOSE (hBT);
	BT_CLOSE (hBT2);
	BT_CLOSE (hBT3);
	_lclose (FidData);
	_lclose (FidDataAll);

	return (st);


}



int LoadHazem (HWND hWnd)
{   char BTfname[100];
	BTVARDESC BTVar[8], *pVars;
	HANDLE hBT, hVars, hBT2;
	time_t ltime;
	HDC	hDC;
	int	   st,  i, i2key;
	long	ikey, TotRecs, iseg;
	long	idata, nRecs, nLoaded;
	char	cdata[2];
	char	Text[512];
	char	Fname[256];
	OFSTRUCT	OFStruct;
	LPSTR	pInRec, lpStrName;
	int		strnum;
	char	cTotRecs[7], VTYPE[2];
	int		NumVars, NumIndex, ioff;
	struct	{int	street_num;
			 long	faddl;
			 long	faddr;
			 long	taddl;
			 long	taddr;
			 }	data;
	char	TAG[30];
	int		FidData;
	FILE	*Fid;
	struct {int		Beg;
			int		Len;
			int		Type;
			char	Name[34];
			} FldInfo;
	int		NumFields, Reclen, len;
	long	Refno, Offset;
	char	CRef[16];
	GWDHEADER GWDHead;
	GWFLDINFO GWFldInfo;

	_fstrcpy (Fname,"j:\\hazem.gwd");
	FidData = OpenFile (Fname,&OFStruct,OF_CREATE);
	Fid = fopen ("j:\\hazem.def","r");
	GWDHead.NumFields=0;
	GWDHead.NumIndex=2;
	GWDHead.Version=0;
	GWDHead.Reclen=0;
	GWDHead.NumIndexFields[0]=1;
	GWDHead.IndexFields[0][0]=1;
	_lwrite (FidData,&GWDHead,sizeof(GWDHEADER)-sizeof(GWFLDINFO));
	while (fgetss (Text,256,Fid))
	{
		Text[45]='\0';
		FldInfo.Len = atoi (&Text[42]);
		Text[38]='\0';
		FldInfo.Beg = atoi (&Text[33])-1;
		Text[32]='\0';
		_fstrcpy (FldInfo.Name,Text);
		GWDHead.NumFields++;
		GWDHead.Reclen = max (GWDHead.Reclen,FldInfo.Beg+FldInfo.Len-1);
		_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	}
	_llseek (FidData,0,0);
	_lwrite (FidData,&GWDHead,sizeof(GWDHEADER)-sizeof(GWFLDINFO));
	_llseek (FidData,0,2);

	fclose (Fid);

	NumVars = 1;
	NumIndex = 1;

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);

    pVars = LocalUnlock(hVars);
    pVars = LocalLock(hVars);

	ltime = 0;

	BT_SET_PARMS (8,16,4,8);
	strcpy (BTfname, "j:\\hazem.in1");
	pVars->BT_VARLEN=4;
	pVars->BT_VARTYP=BT_INTEGER;
	pVars->BT_VAROFF=0;
	BT_CREATE (BTfname, 4, FALSE, 1, 1,pVars,FALSE, 0, 0, FALSE);
	hBT = BT_OPEN (BTfname, ltime, 1, 0);

	hDC = GetDC(hWnd);
	nRecs = 350000;
	nLoaded=0;
	Fid = fopen ("j:\\hazem.txt","r");
	while (fgetss (Text,512,Fid))
	{
		_fstrncpy (CRef,Text,11);
		CRef[11]='\0';
		Refno = atol (CRef);
		Offset = _llseek (FidData,0,1);
		st = BT_PUT (hBT,(LPSTR)&Refno,(LPSTR)&Offset);
		len = _fstrlen(Text);
		_lwrite (FidData,&len,2);
		_lwrite (FidData,Text,len);
		if (!PctBox (hWnd, hDC, nRecs, ++nLoaded)) TotRecs=0;
	}

	ReleaseDC(hWnd, hDC);
	BT_CLOSE (hBT);
	_lclose (FidData);

	return (st);


}

int LoadCadZones (HWND hWnd)
{   char BTfname[100];
	BTVARDESC BTVar[8], *pVars;
	HANDLE hBT, hVars, hBT2;
	time_t ltime;
	HDC	hDC;
	int	   st,  i, i2key;
	long	ikey, TotRecs, iseg;
	long	idata, nRecs, nLoaded;
	char	cdata[2];
	char	Text[512];
	char	Fname[256];
	OFSTRUCT	OFStruct;
	LPSTR	pInRec, lpStrName;
	int		strnum;
	char	cTotRecs[7], VTYPE[2];
	int		NumVars, NumIndex, ioff;
	struct	{int	street_num;
			 long	faddl;
			 long	faddr;
			 long	taddl;
			 long	taddr;
			 }	data;
	char	TAG[30];
	int		FidData;
	FILE	*Fid;
	struct {int		Beg;
			int		Len;
			int		Type;
			char	Name[34];
			} FldInfo;
	int		NumFields, Reclen, len;
	long	Refno, Offset;
	char	CRef[16];
	GWDHEADER GWDHead;
	GWFLDINFO GWFldInfo;

	_fstrcpy (Fname,"j:\\cadzones.gwd");
	FidData = OpenFile (Fname,&OFStruct,OF_CREATE);
	Fid = fopen ("j:\\tag.def","r");
	GWDHead.NumFields=0;
	GWDHead.NumIndex=2;
	GWDHead.Version=0;
	GWDHead.Reclen=0;
	GWDHead.NumIndexFields[0]=1;
	GWDHead.IndexFields[0][0]=1;
	_lwrite (FidData,&GWDHead,sizeof(GWDHEADER)-sizeof(GWFLDINFO));
	while (fgetss (Text,256,Fid))
	{
		Text[45]='\0';
		FldInfo.Len = atoi (&Text[42]);
		Text[38]='\0';
		FldInfo.Beg = atoi (&Text[33])-1;
		Text[32]='\0';
		_fstrcpy (FldInfo.Name,Text);
		GWDHead.NumFields++;
		GWDHead.Reclen = max (GWDHead.Reclen,FldInfo.Beg+FldInfo.Len-1);
		_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	}
	_llseek (FidData,0,0);
	_lwrite (FidData,&GWDHead,sizeof(GWDHEADER)-sizeof(GWFLDINFO));
	_llseek (FidData,0,2);

	fclose (Fid);

	NumVars = 1;
	NumIndex = 1;

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);

    pVars = LocalUnlock(hVars);
    pVars = LocalLock(hVars);

	ltime = 0;

	BT_SET_PARMS (8,16,4,8);
	strcpy (BTfname, "j:\\cadzones.in1");
	pVars->BT_VARLEN=4;
	pVars->BT_VARTYP=BT_INTEGER;
	pVars->BT_VAROFF=0;
	BT_CREATE (BTfname, 4, FALSE, 1, 1,pVars,FALSE, 0, 0, FALSE);
	hBT = BT_OPEN (BTfname, ltime, 1, 0);

	hDC = GetDC(hWnd);
	nRecs = 350000;
	nLoaded=0;
	Fid = fopen ("j:\\cadzones.txt","r");
	while (fgetss (Text,512,Fid))
	{
		_fstrncpy (CRef,Text,11);
		CRef[11]='\0';
		Refno = atol (CRef);
		Offset = _llseek (FidData,0,1);
		st = BT_PUT (hBT,(LPSTR)&Refno,(LPSTR)&Offset);
		len = _fstrlen(Text);
		_lwrite (FidData,&len,2);
		_lwrite (FidData,Text,len);
		if (!PctBox (hWnd, hDC, nRecs, ++nLoaded)) TotRecs=0;
	}

	ReleaseDC(hWnd, hDC);
	BT_CLOSE (hBT);
	_lclose (FidData);

	return (st);


}

int LoadBrkPrk(HWND hWnd)
{   char BTfname[100];
	BTVARDESC BTVar[8], *pVars;
	HANDLE hBT, hVars, hBT2;
	time_t ltime;
	HDC	hDC;
	int	   st,  i, i2key;
	long	ikey, TotRecs, iseg;
	long	idata, nRecs, nLoaded;
	char	cdata[2];
	char	Text[512];
	char	Fname[256];
	OFSTRUCT	OFStruct;
	LPSTR	pInRec, lpStrName;
	int		strnum;
	char	cTotRecs[7], VTYPE[2];
	int		NumVars, NumIndex, ioff;
	struct	{int	street_num;
			 long	faddl;
			 long	faddr;
			 long	taddl;
			 long	taddr;
			 }	data;
	char	TAG[30];
	int		FidData;
	FILE	*Fid;
	struct {int		Beg;
			int		Len;
			int		Type;
			char	Name[34];
			} FldInfo;
	int		NumFields, Reclen, len;
	long	Refno, Offset;
	char	CRef[16];
	GWDHEADER GWDHead;
	GWFLDINFO GWFldInfo;

	_fstrcpy (Fname,"j:\\brkprk.gwd");
	FidData = OpenFile (Fname,&OFStruct,OF_CREATE);
	Fid = fopen ("j:\\brkprk.def","r");
	GWDHead.NumFields=0;
	GWDHead.NumIndex=2;
	GWDHead.Version=0;
	GWDHead.Reclen=0;
	GWDHead.NumIndexFields[0]=1;
	GWDHead.IndexFields[0][0]=1;
	_lwrite (FidData,&GWDHead,sizeof(GWDHEADER)-sizeof(GWFLDINFO));
	while (fgetss (Text,256,Fid))
	{
		Text[45]='\0';
		FldInfo.Len = atoi (&Text[42]);
		Text[38]='\0';
		FldInfo.Beg = atoi (&Text[33])-1;
		Text[32]='\0';
		_fstrcpy (FldInfo.Name,Text);
		GWDHead.NumFields++;
		GWDHead.Reclen = max (GWDHead.Reclen,FldInfo.Beg+FldInfo.Len-1);
		_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	}
	_llseek (FidData,0,0);
	_lwrite (FidData,&GWDHead,sizeof(GWDHEADER)-sizeof(GWFLDINFO));
	_llseek (FidData,0,2);

	fclose (Fid);

	NumVars = 1;
	NumIndex = 1;

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);

    pVars = LocalUnlock(hVars);
    pVars = LocalLock(hVars);

	ltime = 0;

	BT_SET_PARMS (8,16,4,8);
	strcpy (BTfname, "j:\\brkprk.in1");
	pVars->BT_VARLEN=4;
	pVars->BT_VARTYP=BT_INTEGER;
	pVars->BT_VAROFF=0;
	BT_CREATE (BTfname, 4, FALSE, 1, 1,pVars,FALSE, 0, 0, FALSE);
	hBT = BT_OPEN (BTfname, ltime, 1, 0);

	hDC = GetDC(hWnd);
	nRecs = 350000;
	nLoaded=0;
	Fid = fopen ("j:\\brkprk.txt","r");
	while (fgetss (Text,512,Fid))
	{
		_fstrncpy (CRef,Text,11);
		CRef[11]='\0';
		Refno = atol (CRef);
		Offset = _llseek (FidData,0,1);
		st = BT_PUT (hBT,(LPSTR)&Refno,(LPSTR)&Offset);
		len = _fstrlen(Text);
		_lwrite (FidData,&len,2);
		_lwrite (FidData,Text,len);
		if (!PctBox (hWnd, hDC, nRecs, ++nLoaded)) TotRecs=0;
	}

	ReleaseDC(hWnd, hDC);
	BT_CLOSE (hBT);
	_lclose (FidData);

	return (st);


}




int LoadCwld (HWND hWnd)
{   char BTfname[100];
	BTVARDESC BTVar[8], *pVars;
	HANDLE hBT, hVars, hBT2;
	time_t ltime;
	HDC	hDC;
	int	   st,  i, i2key;
	long	ikey, TotRecs, iseg;
	long	idata, nRecs, nLoaded;
	char	cdata[2];
	char	Text[512];
	char	Fname[256];
	OFSTRUCT	OFStruct;
	LPSTR	pInRec, lpStrName;
	int		strnum;
	char	cTotRecs[7], VTYPE[2];
	int		NumVars, NumIndex, ioff;
	struct	{int	street_num;
			 long	faddl;
			 long	faddr;
			 long	taddl;
			 long	taddr;
			 }	data;
	char	TAG[30];
	int		FidData;
	FILE	*Fid;
	struct {int		Beg;
			int		Len;
			int		Type;
			char	Name[34];
			} FldInfo;
	int		NumFields, Reclen, len;
	long	Refno, Offset;
	char	CRef[16];
	GWDHEADER GWDHead;
	GWFLDINFO GWFldInfo;

	_fstrcpy (Fname,"j:\\cwld.gwd");
	FidData = OpenFile (Fname,&OFStruct,OF_CREATE);
	Fid = fopen ("j:\\cwld.def","r");
	GWDHead.NumFields=0;
	GWDHead.NumIndex=2;
	GWDHead.Version=0;
	GWDHead.Reclen=0;
	GWDHead.NumIndexFields[0]=1;
	GWDHead.IndexFields[0][0]=1;
	_lwrite (FidData,&GWDHead,sizeof(GWDHEADER)-sizeof(GWFLDINFO));
	while (fgetss (Text,256,Fid))
	{
		Text[45]='\0';
		FldInfo.Len = atoi (&Text[42]);
		Text[38]='\0';
		FldInfo.Beg = atoi (&Text[33])-1;
		Text[32]='\0';
		_fstrcpy (FldInfo.Name,Text);
		GWDHead.NumFields++;
		GWDHead.Reclen = max (GWDHead.Reclen,FldInfo.Beg+FldInfo.Len-1);
		_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	}
	_llseek (FidData,0,0);
	_lwrite (FidData,&GWDHead,sizeof(GWDHEADER)-sizeof(GWFLDINFO));
	_llseek (FidData,0,2);

	fclose (Fid);

	NumVars = 1;
	NumIndex = 1;

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);

    pVars = LocalUnlock(hVars);
    pVars = LocalLock(hVars);

	ltime = 0;

	BT_SET_PARMS (8,16,4,8);
	strcpy (BTfname, "j:\\cwld.in1");
	pVars->BT_VARLEN=8;
	pVars->BT_VARTYP=BT_CHAR;
	pVars->BT_VAROFF=0;
	BT_CREATE (BTfname, 4, FALSE, 1, 1,pVars,FALSE, 0, 0, FALSE);
	hBT = BT_OPEN (BTfname, ltime, 1, 0);

	hDC = GetDC(hWnd);
	nRecs = 55000;
	nLoaded=0;
	Fid = fopen ("j:\\cwld.txt","r");
	while (fgetss (Text,512,Fid))
	{
		Offset = _llseek (FidData,0,1);
		st = BT_PUT (hBT,(LPSTR)&Text,(LPSTR)&Offset);
		len = _fstrlen(Text);
		_lwrite (FidData,&len,2);
		_lwrite (FidData,Text,len);
		if (!PctBox (hWnd, hDC, nRecs, ++nLoaded)) TotRecs=0;
	}

	ReleaseDC(hWnd, hDC);
	BT_CLOSE (hBT);
	_lclose (FidData);

	return (st);


}



int LoadStnames2 (HWND hWnd)
{   char BTfname[100];
	BTVARDESC BTVar[8], *pVars;
	HANDLE hBT, hVars;
	time_t ltime;
	HDC	hDC;
	int	   st, Fid, i, i2key;
	long	ikey, TotRecs, iseg;
	long	idata, nRecs, nLoaded;
	char	cdata[2];
	char	Text[100];
	char	Fname[256];
	OFSTRUCT	OFStruct;
	LPSTR	pInRec, lpStrName;
	int		strnum;
	char	cTotRecs[7], VTYPE[2];
	int		NumVars, NumIndex, ioff;
	struct	{int	street_num;
			 long	faddl;
			 long	faddr;
			 long	taddl;
			 long	taddr;
			 }	data;
	char	TAG[30];

/*	_fstrcpy (Fname,"c:\\data.pmq");*/
	_fstrcpy (Fname,"c:\\stnames.txt");
	Fid = OpenFile (Fname,&OFStruct,OF_READ);

/*	Get the total records in the file */
	_llseek (Fid,-8,2);
	i =_lread (Fid,cTotRecs,6);
	cTotRecs[6]='\0';
	TotRecs = atol (cTotRecs);
	_llseek (Fid,0,0);

/*	Get the number of fields and indexes */
	pInRec = NextRec (Fid); TotRecs--;
	NumVars = atoi (NextFld(&pInRec));
	NumIndex = atoi (NextFld(&pInRec));

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);

    for (i=0,ioff=0;i<NumVars;i++,pVars++)
    {
		pInRec = NextRec (Fid); TotRecs--;
		_fstrcpy (VTYPE,NextFld(&pInRec));
		if 		(VTYPE[0] == 'I') pVars->BT_VARTYP = BT_INTEGER;
		else if (VTYPE[0] == 'C') pVars->BT_VARTYP = BT_CHAR;
		else if (VTYPE[0] == 'R') pVars->BT_VARTYP = BT_REAL;
		pVars->BT_VARLEN = atoi (NextFld(&pInRec));

		pVars->BT_VAROFF = ioff;
		ioff += pVars->BT_VARLEN;
	}
    pVars = LocalUnlock(hVars);
    pVars = LocalLock(hVars);

    for (i=0,ioff=0;i<NumIndex;i++)
    {
		pInRec = NextRec (Fid); TotRecs--;
	}
	ltime = 0;

	BT_SET_PARMS (8,16,4,8);
	strcpy (BTfname, "C:\\stnames2.btr");
	pVars->BT_VARLEN=32;
	pVars->BT_VARTYP=BT_CHAR;
	BT_CREATE (BTfname, 2, FALSE, 1, 1,pVars,FALSE, 0, 0, FALSE);
	hBT = BT_OPEN (BTfname, ltime, 1, 0);

	hDC = GetDC(hWnd);
	nRecs = TotRecs;
	nLoaded=0;
	while (TotRecs)
	{
		pInRec = NextRec (Fid); TotRecs--;
		strnum = atoi (NextFld(&pInRec));
		lpStrName=NextFld(&pInRec);
		st = BT_PUT (hBT,lpStrName,(LPSTR)&strnum);
		if (!PctBox (hWnd, hDC, nRecs, ++nLoaded)) TotRecs=0;
	}

	ReleaseDC(hWnd, hDC);
	BT_CLOSE (hBT);
	_lclose (Fid);

	return (st);


}

int LoadStnames (HWND hWnd)
{   char BTfname[100];
	BTVARDESC BTVar[8], *pVars;
	HANDLE hBT, hBT2, hVars;
	time_t ltime;
	HDC	hDC;
	int	   st, Fid, i, i2key;
	long	ikey, TotRecs, iseg;
	long	idata, nRecs, nLoaded;
	char	cdata[2];
	char	Text[100];
	char	Fname[256];
	OFSTRUCT	OFStruct;
	LPSTR	pInRec, lpStrName, lpStrNameEnd;
	long	strnum;
	char	cTotRecs[7], VTYPE[2];
	int		NumVars, NumIndex, ioff; 
	RECT	Rect;
	struct	{int	street_num;
			 long	faddl;
			 long	faddr;
			 long	taddl;
			 long	taddr;
			 }	data;
	char	TAG[30];

/*	_fstrcpy (Fname,"c:\\data.pmq");*/
	_fstrcpy (Fname,"f:\\stnames.txt");
	Fid = OpenFile (Fname,&OFStruct,OF_READ);

/*	Get the total records in the file */
	_llseek (Fid,-8,2);
	i =_lread (Fid,cTotRecs,6);
	cTotRecs[6]='\0';
	TotRecs = atol (cTotRecs);
	_llseek (Fid,0,0);

/*	Get the number of fields and indexes */
	pInRec = NextRec (Fid); TotRecs--;
	NumVars = atoi (NextFld(&pInRec));
	NumIndex = atoi (NextFld(&pInRec));

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);

    for (i=0,ioff=0;i<NumVars;i++,pVars++)
    {
		pInRec = NextRec (Fid); TotRecs--;
		_fstrcpy (VTYPE,NextFld(&pInRec));
		if 		(VTYPE[0] == 'I') pVars->BT_VARTYP = BT_INTEGER;
		else if (VTYPE[0] == 'C') pVars->BT_VARTYP = BT_CHAR;
		else if (VTYPE[0] == 'R') pVars->BT_VARTYP = BT_REAL;
		pVars->BT_VARLEN = atoi (NextFld(&pInRec));

		pVars->BT_VAROFF = ioff;
		ioff += pVars->BT_VARLEN;
	}
    pVars = LocalUnlock(hVars);
    pVars = LocalLock(hVars);

    for (i=0,ioff=0;i<NumIndex;i++)
    {
		pInRec = NextRec (Fid); TotRecs--;
	}

		BTVar[0].BT_VARTYP = BT_INTEGER;
		BTVar[0].BT_VARLEN = 4;
		BTVar[0].BT_VAROFF = 0;
		BTVar[1].BT_VARTYP = BT_INTEGER;
		BTVar[1].BT_VARLEN = 2;
		BTVar[1].BT_VAROFF = 2;
	ltime = 0;

	BT_SET_PARMS (8,16,4,8);
	strcpy (BTfname, "C:\\stname1.btr");
/*	BT_CREATE (BTfname, sizeof(data), FALSE, 1, 1,pVars,FALSE, 0, 0, FALSE);*/
	BT_CREATE (BTfname, 34, FALSE, 1, 1,BTVar,FALSE, 0, 0, FALSE);
	hBT = BT_OPEN (BTfname, ltime, 1, 0);
	strcpy (BTfname, "C:\\stname2.btr");
		BTVar[0].BT_VARTYP = BT_CHAR;
		BTVar[0].BT_VARLEN = 32;
		BTVar[0].BT_VAROFF = 0;
		BTVar[1].BT_VARTYP = BT_INTEGER;
		BTVar[1].BT_VARLEN = 2;
		BTVar[1].BT_VAROFF = 32;
	BT_CREATE (BTfname, 4, FALSE, 1, 1,BTVar,FALSE, 0, 0, FALSE);
	hBT2 = BT_OPEN (BTfname, ltime, 1, 0);

	nRecs = TotRecs;
	nLoaded=0; 
	GetWindowRect(hWnd,&Rect);
 hWnd = CreateWindow(
                "STATIC",               /* Window class name           */
                "", /* Window's title             */
                WS_POPUP | WS_BORDER | WS_VISIBLE, 
                (Rect.left+Rect.right)/2,(Rect.top+Rect.bottom)/2,
                200,20,NULL,                    /* Parent window's handle      */
                NULL,                    /* Default to Class Menu       */
                hInst,                   /* Instance of window          */
                NULL);                   /* Create struct for WM_CREATE */
	while (TotRecs)
	{
		pInRec = NextRec (Fid); TotRecs--;
		strnum = atol (NextFld(&pInRec));
		lpStrName=NextFld(&pInRec);
		lpStrNameEnd = lpStrName + 32;
		st = BT_PUT (hBT,(LPSTR)&strnum,lpStrName);
		st = BT_PUT (hBT2,lpStrName,(LPSTR)&strnum);
		if (!PctBox (hWnd, nRecs, ++nLoaded, 10)) TotRecs=0;
	}

	ReleaseDC(hWnd, hDC);
	BT_CLOSE (hBT);
	BT_CLOSE (hBT2);
	_lclose (Fid);

	return (st);


}

int LoadAltNames (HWND hWnd)
{   char BTfname[100];
	BTVARDESC BTVar[8], *pVars;
	HANDLE hBT, hBT2, hVars;
	time_t ltime;
	HDC	hDC;
	int	   st, Fid, i, i2key;
	long	ikey, TotRecs, iseg;
	long	idata, nRecs, nLoaded;
	char	cdata[2];
	char	Text[100];
	char	Fname[256];
	OFSTRUCT	OFStruct;
	LPSTR	pInRec, lpStrName, lpStrNameEnd;
	int		strnum;
	char	cTotRecs[7], VTYPE[2];
	int		NumVars, NumIndex, ioff;
	struct	{
			 long	TLID;
			 int	strnum;
			 } Key;
	char	TAG[30];

/*	_fstrcpy (Fname,"c:\\data.pmq");*/
	_fstrcpy (Fname,"j:\\altstrt.txt");
	Fid = OpenFile (Fname,&OFStruct,OF_READ);

/*	Get the total records in the file */
	_llseek (Fid,-8,2);
	i =_lread (Fid,cTotRecs,6);
	cTotRecs[6]='\0';
	TotRecs = atol (cTotRecs);
	_llseek (Fid,0,0);

/*	Get the number of fields and indexes */
	pInRec = NextRec (Fid); TotRecs--;
	NumVars = atoi (NextFld(&pInRec));
	NumIndex = atoi (NextFld(&pInRec));

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);

    for (i=0,ioff=0;i<NumVars;i++,pVars++)
    {
		pInRec = NextRec (Fid); TotRecs--;
		_fstrcpy (VTYPE,NextFld(&pInRec));
		if 		(VTYPE[0] == 'I') pVars->BT_VARTYP = BT_INTEGER;
		else if (VTYPE[0] == 'C') pVars->BT_VARTYP = BT_CHAR;
		else if (VTYPE[0] == 'R') pVars->BT_VARTYP = BT_REAL;
		pVars->BT_VARLEN = atoi (NextFld(&pInRec));

		pVars->BT_VAROFF = ioff;
		ioff += pVars->BT_VARLEN;
	}
    pVars = LocalUnlock(hVars);
    pVars = LocalLock(hVars);

    for (i=0,ioff=0;i<NumIndex;i++)
    {
		pInRec = NextRec (Fid); TotRecs--;
	}

		BTVar[0].BT_VARTYP = BT_INTEGER;
		BTVar[0].BT_VARLEN = 4;
		BTVar[0].BT_VAROFF = 0;
		BTVar[1].BT_VARTYP = BT_INTEGER;
		BTVar[1].BT_VARLEN = 2;
		BTVar[1].BT_VAROFF = 2;
	ltime = 0;

	BT_SET_PARMS (8,16,4,8);
	strcpy (BTfname, "C:\\altnames.btr");
/*	BT_CREATE (BTfname, 2, FALSE, 2, 1,BTVar,FALSE, 0, 0, FALSE);*/
	hBT = BT_OPEN (BTfname, ltime, 1, 0);

	hDC = GetDC(hWnd);
	nRecs = TotRecs;
	nLoaded=0;
	while (TotRecs)
	{
		pInRec = NextRec (Fid); TotRecs--;
		Key.TLID = atol (NextFld(&pInRec));
		Key.strnum = atoi (NextFld(&pInRec));
		st = BT_PUT (hBT,(LPSTR)&Key,(LPSTR)&nLoaded);
		if (!PctBox (hWnd, hDC, nRecs, ++nLoaded)) TotRecs=0;
	}

	ReleaseDC(hWnd, hDC);
	BT_CLOSE (hBT);
	_lclose (Fid);

	return (st);


}

int LoadLMData (HWND hWnd)
{   char BTfname[100];
	BTVARDESC BTVar[8], *pVars;
	HANDLE hBT, hVars;
	time_t ltime;
	HDC	hDC;
	int	   st, Fid, i, i2key;
	long	ikey, TotRecs, iseg;
	long	idata, nRecs, nLoaded;
	char	cdata[2];
	char	Text[100];
	char	Fname[256];
	OFSTRUCT	OFStruct;
	LPSTR	pInRec, lpStrName;
	int		strnum;
	char	cTotRecs[7], VTYPE[2];
	int		NumVars, NumIndex, ioff;
	struct	{int	street_num;
			 long	faddl;
			 long	faddr;
			 long	taddl;
			 long	taddr;
			 }	data;
	char	TAG[30];

/*	_fstrcpy (Fname,"c:\\data.pmq");*/
	_fstrcpy (Fname,"c:\\temp.txt");
	Fid = OpenFile (Fname,&OFStruct,OF_READ);

/*	Get the total records in the file */
	_llseek (Fid,-8,2);
	i =_lread (Fid,cTotRecs,6);
	cTotRecs[6]='\0';
	TotRecs = atol (cTotRecs);
	_llseek (Fid,0,0);

/*	Get the number of fields and indexes */
	pInRec = NextRec (Fid); TotRecs--;
	NumVars = atoi (NextFld(&pInRec));
	NumIndex = atoi (NextFld(&pInRec));

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);

    for (i=0,ioff=0;i<NumVars;i++,pVars++)
    {
		pInRec = NextRec (Fid); TotRecs--;
		_fstrcpy (VTYPE,NextFld(&pInRec));
		if 		(VTYPE[0] == 'I') pVars->BT_VARTYP = BT_INTEGER;
		else if (VTYPE[0] == 'C') pVars->BT_VARTYP = BT_CHAR;
		else if (VTYPE[0] == 'R') pVars->BT_VARTYP = BT_REAL;
		pVars->BT_VARLEN = atoi (NextFld(&pInRec));

		pVars->BT_VAROFF = ioff;
		ioff += pVars->BT_VARLEN;
	}
    pVars = LocalUnlock(hVars);
    pVars = LocalLock(hVars);

    for (i=0,ioff=0;i<NumIndex;i++)
    {
		pInRec = NextRec (Fid); TotRecs--;
	}
	ltime = 0;

	BT_SET_PARMS (8,16,4,8);
	strcpy (BTfname, "C:\\lmdata.btr");
/*	BT_CREATE (BTfname, sizeof(data), FALSE, 1, 1,pVars,FALSE, 0, 0, FALSE);*/
	hBT = BT_OPEN (BTfname, ltime, BT_WRITE, 0);

	hDC = GetDC(hWnd);
	nRecs = TotRecs;
	nLoaded=0;
	while (TotRecs)
	{
		pInRec = NextRec (Fid); TotRecs--;
		ikey=atol (NextFld(&pInRec));
		data.street_num = atoi (NextFld(&pInRec));
		data.faddl = atol (NextFld(&pInRec));
		data.faddr = atol (NextFld(&pInRec));
		data.taddl = atol (NextFld(&pInRec));
		data.taddr = atol (NextFld(&pInRec));
		st = BT_PUT (hBT,(LPSTR)&ikey,&data);
		if (!PctBox (hWnd, hDC, nRecs, ++nLoaded)) TotRecs=0;


	}

	ReleaseDC(hWnd, hDC);
	BT_CLOSE (hBT);

	return (st);


}

int LoadGeospanAddr (HWND hWnd)
{   char BTfname[100];
	BTVARDESC BTVar[8], *pVars;
	HANDLE hBT, hVars;
	time_t ltime;
	HDC	hDC;
	int	   st, Fid, i, i2key;
	long	ikey, TotRecs, iseg;
	long	idata, nRecs, nLoaded;
	char	cdata[2];
	char	Text[100];
	char	Fname[256];
	OFSTRUCT	OFStruct;
	LPSTR	pInRec, lpStrName;
	int		strnum;
	char	cTotRecs[7], VTYPE[2];
	int		NumVars, NumIndex, ioff;
	struct	{int	street_num;
			 long	faddl;
			 long	faddr;
			 long	taddl;
			 long	taddr;
			 }	data;
	struct	{
				long	TLID;
				long	House;
			}	key;
	long	Frame;
	char	TAG[30];

/*	_fstrcpy (Fname,"c:\\data.pmq");*/
	_fstrcpy (Fname,"c:\\geoaddr.txt");
	Fid = OpenFile (Fname,&OFStruct,OF_READ);

/*	Get the total records in the file */
	_llseek (Fid,-8,2);
	i =_lread (Fid,cTotRecs,6);
	cTotRecs[6]='\0';
	TotRecs = atol (cTotRecs);
	_llseek (Fid,0,0);

/*	Get the number of fields and indexes */
	pInRec = NextRec (Fid); TotRecs--;
	NumVars = atoi (NextFld(&pInRec));
	NumIndex = atoi (NextFld(&pInRec));

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);

    for (i=0,ioff=0;i<NumVars;i++,pVars++)
    {
		pInRec = NextRec (Fid); TotRecs--;
		_fstrcpy (VTYPE,NextFld(&pInRec));
		if 		(VTYPE[0] == 'I') pVars->BT_VARTYP = BT_INTEGER;
		else if (VTYPE[0] == 'C') pVars->BT_VARTYP = BT_CHAR;
		else if (VTYPE[0] == 'R') pVars->BT_VARTYP = BT_REAL;
		pVars->BT_VARLEN = atoi (NextFld(&pInRec));

		pVars->BT_VAROFF = ioff;
		ioff += pVars->BT_VARLEN;
	}
    pVars = LocalUnlock(hVars);
    pVars = LocalLock(hVars);

    for (i=0,ioff=0;i<NumIndex;i++)
    {
		pInRec = NextRec (Fid); TotRecs--;
	}
	ltime = 0;

	BT_SET_PARMS (8,16,4,8);
	strcpy (BTfname, "C:\\actaddr.btr");
	BT_CREATE (BTfname, 4, FALSE, 2, 1,pVars,FALSE, 0, 0, FALSE);
	hBT = BT_OPEN (BTfname, ltime, BT_WRITE, 0);

	hDC = GetDC(hWnd);
	nRecs = TotRecs;
	nLoaded=0;
	while (TotRecs)
	{
		pInRec = NextRec (Fid); TotRecs--;
		key.TLID=atol (NextFld(&pInRec));
		key.House = atol (NextFld(&pInRec));
		Frame = atol (NextFld(&pInRec));
		st = BT_PUT (hBT,(LPSTR)&key,&Frame);
		if (!PctBox (hWnd, hDC, nRecs, ++nLoaded)) TotRecs=0;


	}

	ReleaseDC(hWnd, hDC);
	BT_CLOSE (hBT);

	return (st);


}

int LoadMunics(HWND hWnd)
{   char BTfname[100];
	BTVARDESC BTVar[8], *pVars;
	HANDLE hBT, hBT2, hVars;
	time_t ltime;
	HDC	hDC;
	int	   st, Fid, i, i2key;
	long	ikey, TotRecs, iseg;
	long	idata, nRecs, nLoaded;
	char	cdata[2];
	char	Text[100];
	char	Fname[256];
	OFSTRUCT	OFStruct;
	LPSTR	pInRec, lpMunName, lpStrNameEnd;
	int		strnum;
	char	cTotRecs[7], VTYPE[2];
	int		NumVars, NumIndex, ioff;
	struct	{char	Name[32];
			 long	Refno;
			 }	data1;
	struct	{char	Name[32];
			 long	FIPS;
			 long	x,y;
			 }	data2;
	char	TAG[30];

/*	_fstrcpy (Fname,"c:\\data.pmq");*/
	_fstrcpy (Fname,"c:\\munics.dat");
	Fid = OpenFile (Fname,&OFStruct,OF_READ);

/*	Get the total records in the file */
	_llseek (Fid,-8,2);
	i =_lread (Fid,cTotRecs,6);
	cTotRecs[6]='\0';
	TotRecs = atol (cTotRecs);
	_llseek (Fid,0,0);

/*	Get the number of fields and indexes */
	pInRec = NextRec (Fid); TotRecs--;
	NumVars = atoi (NextFld(&pInRec));
	NumIndex = atoi (NextFld(&pInRec));

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);

    for (i=0,ioff=0;i<NumVars;i++,pVars++)
    {
		pInRec = NextRec (Fid); TotRecs--;
		_fstrcpy (VTYPE,NextFld(&pInRec));
		if 		(VTYPE[0] == 'I') pVars->BT_VARTYP = BT_INTEGER;
		else if (VTYPE[0] == 'C') pVars->BT_VARTYP = BT_CHAR;
		else if (VTYPE[0] == 'R') pVars->BT_VARTYP = BT_REAL;
		pVars->BT_VARLEN = atoi (NextFld(&pInRec));

		pVars->BT_VAROFF = ioff;
		ioff += pVars->BT_VARLEN;
	}
    pVars = LocalUnlock(hVars);
    pVars = LocalLock(hVars);

    for (i=0,ioff=0;i<NumIndex;i++)
    {
		pInRec = NextRec (Fid); TotRecs--;
	}

		BTVar[0].BT_VARTYP = BT_INTEGER;
		BTVar[0].BT_VARLEN = 4;
		BTVar[0].BT_VAROFF = 0;
	ltime = 0;

	BT_SET_PARMS (8,16,4,8);
	strcpy (BTfname, "c:\\munics1.btr");
	BT_CREATE (BTfname, sizeof(data1), FALSE, 1, 1,pVars,FALSE, 0, 0, FALSE);
	/*BT_CREATE (BTfname, 34, FALSE, 1, 1,BTVar,FALSE, 0, 0, FALSE);*/
	hBT = BT_OPEN (BTfname, ltime, 1, 0);
	strcpy (BTfname, "c:\\munics2.btr");
		BTVar[0].BT_VARTYP = BT_INTEGER;
		BTVar[0].BT_VARLEN = 4;
		BTVar[0].BT_VAROFF = 0;
	BT_CREATE (BTfname,sizeof(data2), FALSE, 1, 1,BTVar,FALSE, 0, 0, FALSE);
	hBT2 = BT_OPEN (BTfname, ltime, 1, 0);

	hDC = GetDC(hWnd);
	nRecs = TotRecs;
	nLoaded=0;
	while (TotRecs)
	{
		pInRec = NextRec (Fid); TotRecs--;
		data2.FIPS = atol (NextFld(&pInRec));
		data1.Refno = atol (NextFld(&pInRec));
		_fstrcpy(data1.Name,NextFld(&pInRec));
		_fstrcpy(data2.Name,data1.Name);
		data2.x = atof (NextFld(&pInRec));
		data2.y = atof (NextFld(&pInRec));
		st = BT_PUT (hBT,(LPSTR)&data2.FIPS,&data1);
		st = BT_PUT (hBT2,&data1.Refno,(LPSTR)&data2);
		if (!PctBox (hWnd, hDC, nRecs, ++nLoaded)) TotRecs=0;
	}

	ReleaseDC(hWnd, hDC);
	BT_CLOSE (hBT);
	BT_CLOSE (hBT2);
	_lclose (Fid);

	return (st);


}

int LoadGIDs(HWND hWnd)
{   char BTfname[100];
	BTVARDESC BTVar[8], *pVars;
	HANDLE hBT, hBT2, hVars;
	time_t ltime;
	HDC	hDC;
	int	   st, Fid, i, i2key;
	long	ikey, TotRecs, iseg;
	long	idata, nRecs, nLoaded;
	char	cdata[2];
	char	Text[100];
	char	Fname[256];
	OFSTRUCT	OFStruct;
	LPSTR	pInRec, lpMunName, lpStrNameEnd;
	int		strnum;
	char	cTotRecs[7], VTYPE[2];
	int		NumVars, NumIndex, ioff;
	struct	{char	Name[8];
			 long	Refno;
			 }	data1;
	struct	{char	Name[8];
			 long	GIDid;
			 long	x,y;
			 }	data2;
	char	TAG[30];

/*	_fstrcpy (Fname,"c:\\data.pmq");*/
	_fstrcpy (Fname,"c:\\gids.dat");
	Fid = OpenFile (Fname,&OFStruct,OF_READ);

/*	Get the total records in the file */
	_llseek (Fid,-8,2);
	i =_lread (Fid,cTotRecs,6);
	cTotRecs[6]='\0';
	TotRecs = atol (cTotRecs);
	_llseek (Fid,0,0);

/*	Get the number of fields and indexes */
	pInRec = NextRec (Fid); TotRecs--;
	NumVars = atoi (NextFld(&pInRec));
	NumIndex = atoi (NextFld(&pInRec));

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);

    for (i=0,ioff=0;i<NumVars;i++,pVars++)
    {
		pInRec = NextRec (Fid); TotRecs--;
		_fstrcpy (VTYPE,NextFld(&pInRec));
		if 		(VTYPE[0] == 'I') pVars->BT_VARTYP = BT_INTEGER;
		else if (VTYPE[0] == 'C') pVars->BT_VARTYP = BT_CHAR;
		else if (VTYPE[0] == 'R') pVars->BT_VARTYP = BT_REAL;
		pVars->BT_VARLEN = atoi (NextFld(&pInRec));

		pVars->BT_VAROFF = ioff;
		ioff += pVars->BT_VARLEN;
	}
    pVars = LocalUnlock(hVars);
    pVars = LocalLock(hVars);

    for (i=0,ioff=0;i<NumIndex;i++)
    {
		pInRec = NextRec (Fid); TotRecs--;
	}

		BTVar[0].BT_VARTYP = BT_INTEGER;
		BTVar[0].BT_VARLEN = 4;
		BTVar[0].BT_VAROFF = 0;
	ltime = 0;

	BT_SET_PARMS (8,16,4,8);
	strcpy (BTfname, "c:\\gids1.btr");
	BT_CREATE (BTfname, sizeof(data1), FALSE, 1, 1,pVars,FALSE, 0, 0, FALSE);
	/*BT_CREATE (BTfname, 34, FALSE, 1, 1,BTVar,FALSE, 0, 0, FALSE);*/
	hBT = BT_OPEN (BTfname, ltime, 1, 0);
	strcpy (BTfname, "c:\\gids2.btr");
		BTVar[0].BT_VARTYP = BT_INTEGER;
		BTVar[0].BT_VARLEN = 4;
		BTVar[0].BT_VAROFF = 0;
	BT_CREATE (BTfname,sizeof(data2), FALSE, 1, 1,BTVar,FALSE, 0, 0, FALSE);
	hBT2 = BT_OPEN (BTfname, ltime, 1, 0);

	hDC = GetDC(hWnd);
	nRecs = TotRecs;
	nLoaded=0;
	while (TotRecs)
	{
		pInRec = NextRec (Fid); TotRecs--;
		data2.GIDid = atol (NextFld(&pInRec));
		data1.Refno = atol (NextFld(&pInRec));
		_fstrcpy(data1.Name,NextFld(&pInRec));
		_fstrcpy(data2.Name,data1.Name);
		data2.x = atof (NextFld(&pInRec));
		data2.y = atof (NextFld(&pInRec));
		st = BT_PUT (hBT,(LPSTR)&data2.GIDid,&data1);
		st = BT_PUT (hBT2,&data1.Refno,(LPSTR)&data2);
	}

	ReleaseDC(hWnd, hDC);
	BT_CLOSE (hBT);
	BT_CLOSE (hBT2);
	_lclose (Fid);

	return (st);


}

int LoadAddressList (HWND hWnd)
{   char BTfname[100];
	BTVARDESC BTVar[8], *pVars;
	HANDLE hBT, hVars;
	time_t ltime;
	HDC	hDC;
	int	   st, Fid, i, i2key;
	long	ikey, TotRecs, iseg;
	long	idata, nRecs, nLoaded;
	char	cdata[2];
	char	Text[100];
	char	Fname[256];
	OFSTRUCT	OFStruct;
	LPSTR	pInRec, lpStrName;
	int		strnum;
	char	cTotRecs[7], VTYPE[2];
	int		NumVars, NumIndex, ioff;
	struct	{
			 long	x;
			 long	y;
			 }	data;
	struct	{
				int		Street;
				long	House;
			}	key;
	long	Frame;
	char	TAG[30];
	RECT	Rect;

/*	_fstrcpy (Fname,"c:\\data.pmq");*/
	_fstrcpy (Fname,"c:\\addlist.txt");
	Fid = OpenFile (Fname,&OFStruct,OF_READ);

/*	Get the total records in the file */
	_llseek (Fid,-8,2);
	i =_lread (Fid,cTotRecs,6);
	cTotRecs[6]='\0';
	TotRecs = atol (cTotRecs);
	_llseek (Fid,0,0);

/*	Get the number of fields and indexes */
	pInRec = NextRec (Fid); TotRecs--;
	NumVars = atoi (NextFld(&pInRec));
	NumIndex = atoi (NextFld(&pInRec));

	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
    pVars = LocalLock(hVars);

    for (i=0,ioff=0;i<NumVars;i++,pVars++)
    {
		pInRec = NextRec (Fid); TotRecs--;
		_fstrcpy (VTYPE,NextFld(&pInRec));
		if 		(VTYPE[0] == 'I') pVars->BT_VARTYP = BT_INTEGER;
		else if (VTYPE[0] == 'C') pVars->BT_VARTYP = BT_CHAR;
		else if (VTYPE[0] == 'R') pVars->BT_VARTYP = BT_REAL;
		pVars->BT_VARLEN = atoi (NextFld(&pInRec));

		pVars->BT_VAROFF = ioff;
		ioff += pVars->BT_VARLEN;
	}
    pVars = LocalUnlock(hVars);
    pVars = LocalLock(hVars);

    for (i=0,ioff=0;i<NumIndex;i++)
    {
		pInRec = NextRec (Fid); TotRecs--;
	}
	ltime = 0;

	BT_SET_PARMS (8,16,4,8);
	strcpy (BTfname, "c:\\addlist.btr");
	BT_CREATE (BTfname, sizeof(data), FALSE, 2, 1,pVars,FALSE, 0, 0, FALSE);
	hBT = BT_OPEN (BTfname, ltime, BT_WRITE, 0);

     GetWindowRect(hWnd,&Rect);
 hWnd = CreateWindow(
                "STATIC",               /* Window class name           */
                "", /* Window's title             */
                WS_POPUP | WS_BORDER | WS_VISIBLE, 
                (Rect.left+Rect.right)/2,(Rect.top+Rect.bottom)/2,
                200,20,NULL,                    /* Parent window's handle      */
                NULL,                    /* Default to Class Menu       */
                hInst,                   /* Instance of window          */
                NULL);                   /* Create struct for WM_CREATE */
	nRecs = TotRecs;
	nLoaded=0;
	while (TotRecs)
	{
		pInRec = NextRec (Fid); TotRecs--;
		key.Street=atoi (NextFld(&pInRec));
		key.House = atol (NextFld(&pInRec));
		data.x = atol (NextFld(&pInRec));
		data.y = atol (NextFld(&pInRec));
		st = BT_PUT (hBT,(LPSTR)&key,&data);
		if (!PctBox (hWnd, nRecs, ++nLoaded, 10)) TotRecs=0;


	}

	ReleaseDC(hWnd, hDC);
	BT_CLOSE (hBT);

	return (st);


}

LPSTR NextRec (int Fid)
{
	static	char	Buf[32767];
	static	int		lBuf=-1, nread;
	static	LPSTR   bpos=&Buf;
	LPSTR	rtnval;

	if (lBuf<0 || !*bpos)
	{
		lBuf=_lread(Fid,&Buf,32767);
		if (lBuf != 32767) Buf[lBuf]='\0';
		bpos = &Buf;
	}
	if (*bpos == '\n') bpos++;
	rtnval = bpos;
	while (*bpos != '\r' && *bpos != '\n')
	{   if (!*bpos)
		{
			nread=bpos-rtnval;
			_fmemmove (&Buf,rtnval,nread);
			rtnval=&Buf;
			bpos=(LPSTR)&Buf+nread;
			nread=32767-nread;
			lBuf=_lread(Fid,bpos,nread);
			if (lBuf != nread) *(bpos+nread)='\0';
		}
		else bpos++;
	}
	*bpos++ = '\0';
	return (rtnval);
}

LPSTR NextFld (LPSTR *ipos)
{   LPSTR  rtnval;
	char	EndChar;

	EndChar = ',';
    (*ipos)++;
	if (**ipos == ',') (*ipos)++;
	if (**ipos == '"') {(*ipos)++; EndChar='"';}
	rtnval = *ipos;
	while (**ipos && **ipos != EndChar) (*ipos)++;
	**ipos='\0';
	return(rtnval);
}
 
BOOL GetTextString (LPSTR str,int l,LPSTR title)
{
	return FALSE; 
}
BOOL AVIFrameToDIB (LPSTR File, long frame,LPHANDLE phDibInfo, LPHANDLE phImage, int Bitcount)
{
	return FALSE;
}

int OppositeView(int view)
{
	return 0;
}

BOOL FAR PASCAL TAGLOCMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam){return FALSE;}
BOOL LoadVisList (LPSTR Name){return FALSE;};
BOOL LoadPickList (LPSTR Name){return FALSE;};
LPVOID	ReadObject (int	Fid){return NULL;}; 
int	ZoomedView (int view){return view;};

DWORD GetDlgItemPrompt (HWND hWnd){return 0; }
int GetGlobalBVal (int i){return 0;}
void SetGlobalValue (LPSTR Name, LPSTR Value){return;}
void GetCurVal (LPSTR Val,short maxval, UINT StringID){return;}
void SetCurVal (LPSTR Val,UINT StringID){return;}
int SetPromptDlg (int i){return 0;}






