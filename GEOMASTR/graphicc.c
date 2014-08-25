#include "graphint.h"
#include "extrndb.h"
 
#include "gmextern.h"


static	TAGKEY TAGKey;
static	char	RefIndexFile[MAX_PATH];
static	BOOL	InFixDupRef=FALSE;  
static	short	NumPrevLayers=0;
static	HANDLE	hPrevLayers[MAX_VIEWPORT_FILES];  
static	char	LayerRefIndexFile[MAX_VIEWPORT_FILES][MAX_PATH]; 
static	char	ReopenRefName[MAX_PATH], ReopenTAGName[MAX_PATH];
static	OFSTRUCTGM	OFStructOpenMap;
static	BOOL	FirstSNMess=TRUE;  
static	char	JLBPFile[256]="";
static	HANDLE	hDBJLBP=0;
static	char	GMDRefno[128]="[GMD.ControlNbr]";
static	char	GMDx[128]="[GMD.x]";
static	char	GMDy[128]="[GMD.y]";
static	char	GMDStartTime[128]="[GMD.BDate]";
static	char	GMDEndTime[128]="[GMD.EDate]";
static	char	GMDSymbol[128]="$SYMNUM(CRIMEPOINT)";
static	char	GMDSize[128]="5";
static	char	GMDTAG[128]="CONTROLN:[GMD.ControlNbr]", GMDTag[128];//"CASENUM:[GMD.CaseNbr]";
static	int		GMDXIndex=1, GMDYIndex=2;
static	int		GMDXField=7, GMDYField=8;
static	DPOINT	GMDPoint;
static	int		GMDPointSize;
static	long	GMDColor=-1;
static char		GMDBeginDate[256];
static char		GMDEndDate[256];
static time_t	GMDParmTime=0;
static	int		NumGMDParms=0;  
static short	HaveGMDSym=-1; 
static BOOL		GMDProjectionIsBase;
static char		GMDParms[4096]="";
static char		LastGMDFile[MAX_PATH]=""; 
static	int		GMDpos, GMDcond; 
static	HANDLE	hDisplayedGMDRefs=0;
static	BOOL	OpenMapOpenBP;

	typedef struct {int MSLNK,ONSTREETNUM;
					char ONSTREETNAM[64];
					double BPX,BPY,EPX,EPY;
					char BPITCOORD[12],EPITCOORD[12];
	}STREETSEGS;
	typedef STREETSEGS *LPSTREETSEGS;


short	FindConnectingRef (short WhichEnd,DPOINT OldEnd,LPGWDHEADER lpGWDHead,LPLONG pOffset,double Tol)
{
	short	ConnectingEnd = 0;
	LPDPOINT	pKey;  
	short	pos=BT_FIRST, cond=BT_GE;
	
	pKey = (LPDPOINT)lpGWDHead->pKeys[1];
	pKey->x = OldEnd.x - Tol;
	pKey->y = OldEnd.y - Tol;
	while (!BT_FIND (lpGWDHead->BTHandle[1],(LPSTR)lpGWDHead->pKeys[1],pos,cond,(LPSTR)pOffset))
	{   
		pos = BT_NEXT;
		cond = BT_ANY;
		if (pKey->x - OldEnd.x > Tol)
			break;
		if (ldistp (*pKey,OldEnd) <= Tol)
		{
			ConnectingEnd = 1;
			goto Exit;
		}
	}
	pos=BT_FIRST;
	cond=BT_GE;
	pKey = (LPDPOINT)lpGWDHead->pKeys[2];
	pKey->x = OldEnd.x - Tol;
	pKey->y = OldEnd.y - Tol;
	while (!BT_FIND (lpGWDHead->BTHandle[2],(LPSTR)lpGWDHead->pKeys[2],pos,cond,(LPSTR)pOffset))
	{
		pos = BT_NEXT;
		cond = BT_ANY;
		if (pKey->x - OldEnd.x > Tol)
			break;
		if (ldistp (*pKey,OldEnd) <= Tol)
		{
			ConnectingEnd = 2;
			goto Exit;
		}
	}

Exit:	
	return ConnectingEnd;
}

void ExtractInterleavedCoord (LPSTR intCoord,LPINT pX,LPINT pY)
{
	char cX[20]={0}, cY[20]={0};
	int ln=strlen (intCoord);
	int	i,j;

	for (i=ln,j=ln/2-1;i>0;i-=2,j--)
	{
		cY[j] = intCoord[i-1];
		cX[j] = intCoord[i-2];
	}
	*pX = atoi (cX);
	*pY = atoi (cY);
	return;
}

BOOL CreateInterleavedCoord (double x,double y,int nchar,LPSTR OutLoc)
{
	char cx[16], cy[16], fmt[12];
	int ix = IDNINT (x);
	int iy = IDNINT (y);
	int	i, nout=0;

	nchar += nchar % 2;
	nchar /= 2;
	sprintf (fmt,"%%%i.%ild",nchar,nchar);
	sprintf (cx,fmt,ix);
	sprintf (cy,fmt,iy);
	for (i=0;i<nchar;i++)
	{
		OutLoc[nout++] = cx[i];
		OutLoc[nout++] = cy[i];
	}
	OutLoc[nout] = 0;
	return TRUE;
}

BOOL SegAlreadyInList (int msLink,HFILE fidList)
{
	int		iLoc = GSSillseek (fidList,0,1);
	LPSTR	pTAB;
	char	str[64];
	BOOL	rtn = FALSE;
	int		imsl;

	GSSillseek (fidList,0,0);
	fgetstring (str,62,fidList);
	while (fgetstring (str,32,fidList))
	{
		if ((pTAB = strchr (str,'\t')))
		{
			pTAB++;
			imsl = atoi (pTAB);
			if (imsl == msLink)
			{
				rtn = TRUE;
				goto Exit;
			}
		}
	}
Exit:
	GSSillseek (fidList,iLoc,0);
	return rtn;
}

BOOL GetNearestUnusedSegment (int onStreetNum,LPSTR nextIntPoint,HFILE outFid,LPGWDHEADER lpGWDHead,LPINT startMSLink)
{
	int pos = BT_FIRST;
	int cond = BT_GE;
	int	iseq=0;
	int	x,y;
	long Offset;
	double dist, minDist=DBL_MAX, maxDist=350;
	char	minNextInt[14];
	int		minMSLink;
	LPSTREETSEGS pStreetSegs=(LPSTREETSEGS)&lpGWDHead->GWDData;;

	nextIntPoint[12] = 0;
	ExtractInterleavedCoord (nextIntPoint,&x,&y);

	SetFieldValFromCharAndName(lpGWDHead,"ONSTREETNUM",(LPSTR)&onStreetNum,TRUE);
	SetFieldValFromCharAndName(lpGWDHead,"BPITCOORD",(LPSTR)nextIntPoint,TRUE);
	GWDFormKey(lpGWDHead,3,TRUE,0,0);
	if (!BT_FIND (lpGWDHead->BTHandle[3],lpGWDHead->pKeys[3],BT_FIRST,BT_GE, (LPSTR)&Offset))
	{
	    FillGWDData (lpGWDHead,Offset); 
		if (pStreetSegs->MSLNK == *startMSLink)
		{
			if (BT_FIND (lpGWDHead->BTHandle[3],lpGWDHead->pKeys[3],BT_NEXT,BT_ANY, (LPSTR)&Offset))
				goto Next1;
			FillGWDData (lpGWDHead,Offset); 
		}
		if (SegAlreadyInList (pStreetSegs->MSLNK,outFid))
			goto Next1;
		if (pStreetSegs->ONSTREETNUM == onStreetNum) 
		{
			minDist = LDIST (pStreetSegs->BPX,pStreetSegs->BPY,x,y);
			minMSLink = pStreetSegs->MSLNK;
			strncpy0 (minNextInt,pStreetSegs->EPITCOORD,12);
		}
	}
Next1:
	SetFieldValFromCharAndName(lpGWDHead,"ONSTREETNUM",(LPSTR)&onStreetNum,TRUE);
	SetFieldValFromCharAndName(lpGWDHead,"BPITCOORD",(LPSTR)nextIntPoint,TRUE);
	GWDFormKey(lpGWDHead,3,TRUE,0,0);
	if (!BT_FIND (lpGWDHead->BTHandle[3],lpGWDHead->pKeys[3],BT_FIRST,BT_GE, (LPSTR)&Offset))
	{
		if (BT_FIND (lpGWDHead->BTHandle[3],lpGWDHead->pKeys[3],BT_PRIOR,BT_ANY, (LPSTR)&Offset))
			goto Next2;
	    FillGWDData (lpGWDHead,Offset); 
		if (pStreetSegs->MSLNK == *startMSLink)
		{
			if (BT_FIND (lpGWDHead->BTHandle[3],lpGWDHead->pKeys[3],BT_PRIOR,BT_ANY, (LPSTR)&Offset))
				goto Next2;
			FillGWDData (lpGWDHead,Offset); 
		}
		if (SegAlreadyInList (pStreetSegs->MSLNK,outFid))
			goto Next2;
		dist = LDIST (pStreetSegs->BPX,pStreetSegs->BPY,x,y);
		if (dist < minDist && pStreetSegs->ONSTREETNUM == onStreetNum) 
		{
			minMSLink = pStreetSegs->MSLNK;
			strncpy0 (minNextInt,pStreetSegs->EPITCOORD,12);
			minDist = dist;
		}
	}
Next2:
	SetFieldValFromCharAndName(lpGWDHead,"ONSTREETNUM",(LPSTR)&onStreetNum,TRUE);
	SetFieldValFromCharAndName(lpGWDHead,"EPITCOORD",(LPSTR)nextIntPoint,TRUE);
	GWDFormKey(lpGWDHead,4,TRUE,0,0);
	if (!BT_FIND (lpGWDHead->BTHandle[4],lpGWDHead->pKeys[4],BT_FIRST,BT_GE, (LPSTR)&Offset))
	{
	    FillGWDData (lpGWDHead,Offset); 
		if (pStreetSegs->MSLNK == *startMSLink)
		{
			if (BT_FIND (lpGWDHead->BTHandle[4],lpGWDHead->pKeys[4],BT_NEXT,BT_ANY, (LPSTR)&Offset))
				goto Next3;
			FillGWDData (lpGWDHead,Offset); 
		}
		if (SegAlreadyInList (pStreetSegs->MSLNK,outFid))
			goto Next3;
		dist = LDIST (pStreetSegs->EPX,pStreetSegs->EPY,x,y);
		if (dist < minDist && pStreetSegs->ONSTREETNUM == onStreetNum) 
		{
			minMSLink = pStreetSegs->MSLNK;
			strncpy0 (minNextInt,pStreetSegs->BPITCOORD,12);
			minDist = dist;
		}
	}
Next3:
	SetFieldValFromCharAndName(lpGWDHead,"ONSTREETNUM",(LPSTR)&onStreetNum,TRUE);
	SetFieldValFromCharAndName(lpGWDHead,"EPITCOORD",(LPSTR)nextIntPoint,TRUE);
	GWDFormKey(lpGWDHead,3,TRUE,0,0);
	if (!BT_FIND (lpGWDHead->BTHandle[4],lpGWDHead->pKeys[4],BT_FIRST,BT_GE, (LPSTR)&Offset))
	{
		if (BT_FIND (lpGWDHead->BTHandle[3],lpGWDHead->pKeys[4],BT_PRIOR,BT_ANY, (LPSTR)&Offset))
			goto Next4;
	    FillGWDData (lpGWDHead,Offset); 
		if (pStreetSegs->MSLNK == *startMSLink)
		{
			if (BT_FIND (lpGWDHead->BTHandle[4],lpGWDHead->pKeys[4],BT_PRIOR,BT_ANY, (LPSTR)&Offset))
				goto Next4;
			FillGWDData (lpGWDHead,Offset); 
		}
		if (SegAlreadyInList (pStreetSegs->MSLNK,outFid))
			goto Next4;
		dist = LDIST (pStreetSegs->EPX,pStreetSegs->EPY,x,y);
		if (dist < minDist && pStreetSegs->ONSTREETNUM == onStreetNum) 
		{
			minMSLink = pStreetSegs->MSLNK;
			strncpy0 (minNextInt,pStreetSegs->BPITCOORD,12);
			minDist = dist;
		}
	}
Next4:
	if (minDist < maxDist)
	{
		strncpy (nextIntPoint,minNextInt,12);
		*startMSLink = minMSLink;
		return TRUE;
	}
	return FALSE;
}


int TraceStreetLink (int recordNum,int onStreetNum,int startMSLink,LPSTR nextIntPointIn,LPSTR endIntPoint,HFILE outFid,LPSTR segFile)
{
	int n=0, nMatch;
	int pos = BT_FIRST;
	int cond = BT_GE;
	int	iseq=0;
	LPGWDHEADER lpGWDHead;
	long Offset;
	LPSTREETSEGS pStreetSegs;
	char	nextIntPoint[14]={0};
	char	nextIntPoints[8][14]={0};
	int		lastMSLinks[8];
	char	str[128];
	static	int nMult=0;

	HANDLE hDB = OpenGWDatabase (segFile,BT_READ);
	if (!hDB)
		return 0; 

	sprintf (str,"%i\t%i",iseq,startMSLink);
	fputstring (str,outFid);
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
	pStreetSegs = (LPSTREETSEGS)&lpGWDHead->GWDData;
	SetFieldValFromCharAndName(lpGWDHead,"MSLNK",(LPSTR)&startMSLink,TRUE);
	GWDFormKey(lpGWDHead,0,TRUE,0,0);
	if (!BT_FIND (lpGWDHead->BTHandle[0],lpGWDHead->pKeys[0],BT_FIRST,BT_EQ, (LPSTR)&Offset))
	{
	    FillGWDData (lpGWDHead,Offset); 
		if (!strnicmp (pStreetSegs->BPITCOORD,endIntPoint,12) ||
			!strnicmp (pStreetSegs->EPITCOORD,endIntPoint,12))
		{
			n = 1;
			goto Exit;
		}
	}
	memmove (nextIntPoint,nextIntPointIn,12);
NextSeg:
	nMatch = 0;
	SetFieldValFromCharAndName(lpGWDHead,"ONSTREETNUM",(LPSTR)&onStreetNum,TRUE);
	SetFieldValFromCharAndName(lpGWDHead,"BPITCOORD",nextIntPoint,TRUE);
	GWDFormKey(lpGWDHead,3,TRUE,0,0);
	pos = BT_FIRST;
	while (!BT_FIND (lpGWDHead->BTHandle[3],lpGWDHead->pKeys[3],pos,cond, (LPSTR)&Offset))
	{  
		pos = BT_NEXT;
	    FillGWDData (lpGWDHead,Offset); 
		if (pStreetSegs->ONSTREETNUM != onStreetNum || strnicmp (pStreetSegs->BPITCOORD,nextIntPoint,12))
			break;
		if (startMSLink != pStreetSegs->MSLNK)
		{
			lastMSLinks[nMatch] = pStreetSegs->MSLNK;
			memmove (nextIntPoints[nMatch++],pStreetSegs->EPITCOORD,12);
		}
	}
	SetFieldValFromCharAndName(lpGWDHead,"ONSTREETNUM",(LPSTR)&onStreetNum,TRUE);
	SetFieldValFromCharAndName(lpGWDHead,"EPITCOORD",nextIntPoint,TRUE);
	GWDFormKey(lpGWDHead,4,TRUE,0,0);
	pos = BT_FIRST;
	while (!BT_FIND (lpGWDHead->BTHandle[4],lpGWDHead->pKeys[4],pos,cond, (LPSTR)&Offset))
	{  
		pos = BT_NEXT;
	    FillGWDData (lpGWDHead,Offset); 
		if (pStreetSegs->ONSTREETNUM != onStreetNum || strnicmp (pStreetSegs->EPITCOORD,nextIntPoint,12))
			break;
		if (startMSLink != pStreetSegs->MSLNK)
		{
			lastMSLinks[nMatch] = pStreetSegs->MSLNK;
			memmove (nextIntPoints[nMatch++],pStreetSegs->BPITCOORD,12);
		}
	}
	switch (nMatch)
	{
	case 0: //find nearest (within 300 meters) unused segment for this on-street
		if (GetNearestUnusedSegment (onStreetNum,nextIntPoint,outFid,lpGWDHead,&startMSLink))
			goto GotNearest;
		break;
	case 1:
		memmove (nextIntPoint,nextIntPoints[0],12);
		startMSLink = lastMSLinks[0];
GotNearest:
		n--;
		iseq = -n;
		sprintf (str,"%i\t%i",iseq,startMSLink);
		fputstring (str,outFid);

		if (!strnicmp (nextIntPoint,endIntPoint,12))
		{
			n = abs (n);
			break;
		}
		goto NextSeg;
	default:
		nMult++;
		break;
	}
Exit:
	GlobalUnlock (hDB);
	CloseGWDatabase (hDB);
	if (n < 0)
		n = 0;
	return n;
}

int GetNearestIntPoint(LPSTR cX,LPSTR cY,LPSTR conStreetNum,LPSTR cnChar,LPSTR segFile,LPSTR OutLoc)
{
	int rtn=0;
	int pos = BT_FIRST;
	int cond = BT_GE;
	LPGWDHEADER lpGWDHead;
	long Offset;
	double x = atof (cX);
	double y = atof (cY);
	double maxDist=50;
	double startx = x - maxDist;
	double minDist=maxDist*2, dist;
	double minx, miny;
	int nChar = atoi (cnChar);
	int onStreetNum = atoi (conStreetNum);
	int snum;
	LPSTREETSEGS pStreetSegs;
	HANDLE hDB;

Top:
	hDB = OpenGWDatabase (segFile,BT_READ);
	if (!hDB)
		return 0; 
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
	pStreetSegs = (LPSTREETSEGS)&lpGWDHead->GWDData;
	SetFieldValFromCharAndName(lpGWDHead,"BPX",(LPSTR)&startx,TRUE);
	GWDFormKey(lpGWDHead,5,TRUE,0,0); 
	while (!BT_FIND (lpGWDHead->BTHandle[5],lpGWDHead->pKeys[5],pos,cond, (LPSTR)&Offset))
	{ 

		pos = BT_NEXT;
	    FillGWDData (lpGWDHead,Offset); 
		if (pStreetSegs->BPX-startx > maxDist * 2)
			break;
		dist = LDIST (pStreetSegs->BPX,pStreetSegs->BPY,x,y);
		snum = GetStreetNumFromRawName (pStreetSegs->ONSTREETNAM,FALSE);
		if ((!onStreetNum || snum == onStreetNum) && dist < minDist)
		{
			minDist = dist;
			minx = pStreetSegs->BPX;
			miny = pStreetSegs->BPY;
		}
	}

	SetFieldValFromCharAndName(lpGWDHead,"EPX",(LPSTR)&startx,TRUE);
	GWDFormKey(lpGWDHead,6,TRUE,0,0); 
	while (!BT_FIND (lpGWDHead->BTHandle[6],lpGWDHead->pKeys[6],pos,cond, (LPSTR)&Offset))
	{  
		pos = BT_NEXT;
	    FillGWDData (lpGWDHead,Offset); 
		if (pStreetSegs->EPX-startx > maxDist * 2)
			break;
		dist = LDIST (pStreetSegs->EPX,pStreetSegs->EPY,x,y);
		snum = GetStreetNumFromRawName (pStreetSegs->ONSTREETNAM,FALSE);
		if ((!onStreetNum || snum == onStreetNum) && dist < minDist)
		{
			minDist = dist;
			minx = pStreetSegs->EPX;
			miny = pStreetSegs->EPY;
		}
	}

	GlobalUnlock (hDB);
	CloseGWDatabase (hDB);
	if (minDist < maxDist)
	{
		CreateInterleavedCoord (minx,miny,nChar,OutLoc);
		return 1;
	}
	else if (onStreetNum)
	{
		onStreetNum = 0;
		goto Top;
	}
	return 0;
}

int GetStreetSegsBetweenPoints (LPSTR OutFile,LPSTR cRecordNum,LPSTR cFound,LPSTR cOnStreetNum,LPSTR wantSNam,LPSTR fromIntPoint,LPSTR toIntPoint,LPSTR segFile)
{ 
	int rtn=0;
	int pos = BT_FIRST;
	int cond = BT_GE;
	LPGWDHEADER lpGWDHead;
	long Offset;
	int onStreetNum = atoi (cOnStreetNum);
	int recordNum = atoi (cRecordNum);
	int iFound = atoi (cFound);
	LPSTREETSEGS pStreetSegs;
	int	nStartPoints = 0;
	int startMSLink[12];
	char	nextIntPoint[12][14];
	HFILE	outFid=HFILE_ERROR;
	int wantSNum;
	char	nameOnly[80];
	HANDLE hDB = OpenGWDatabase (segFile,BT_READ);

	GSSiRemove (OutFile);
	if (!hDB)
		return 0; 

    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
	pStreetSegs = (LPSTREETSEGS)&lpGWDHead->GWDData;
	SetFieldValFromCharAndName(lpGWDHead,"BPITCOORD",fromIntPoint,TRUE);
	GWDFormKey(lpGWDHead,1,TRUE,0,0); 
	while (!BT_FIND (lpGWDHead->BTHandle[1],lpGWDHead->pKeys[1],pos,cond, (LPSTR)&Offset))
	{  
		pos = BT_NEXT;
	    FillGWDData (lpGWDHead,Offset); 
		if (!strnicmp (pStreetSegs->BPITCOORD,fromIntPoint,12))
		{
			int snum = GetStreetNumFromRawName (pStreetSegs->ONSTREETNAM,FALSE);

			GetTrueStreetName (snum,nameOnly,0,3);
			if (snum == onStreetNum || (iFound < 0 && !stricmp (wantSNam,nameOnly)))
			{
				wantSNum = pStreetSegs->ONSTREETNUM;

				startMSLink[nStartPoints] = pStreetSegs->MSLNK;
				strncpy (nextIntPoint[nStartPoints++],pStreetSegs->EPITCOORD,12);
			}
		}
		else
			break;
	}
	SetFieldValFromCharAndName(lpGWDHead,"EPITCOORD",fromIntPoint,TRUE);
	GWDFormKey(lpGWDHead,2,TRUE,0,0); 
	pos = BT_FIRST;
	while (!BT_FIND (lpGWDHead->BTHandle[2],lpGWDHead->pKeys[2],pos,cond, (LPSTR)&Offset))
	{  
		pos = BT_NEXT;
	    FillGWDData (lpGWDHead,Offset); 
		if (!strnicmp (pStreetSegs->EPITCOORD,fromIntPoint,12))
		{
			int snum = GetStreetNumFromRawName (pStreetSegs->ONSTREETNAM,FALSE);

			GetTrueStreetName (snum,nameOnly,0,3);
			if (snum == onStreetNum || (iFound < 0 && !stricmp (wantSNam,nameOnly)))
			{
				wantSNum = pStreetSegs->ONSTREETNUM;

				startMSLink[nStartPoints] = pStreetSegs->MSLNK;
				strncpy (nextIntPoint[nStartPoints++],pStreetSegs->BPITCOORD,12);
			}
		}
		else
			break;
	}
	GlobalUnlock (hDB);
	CloseGWDatabase (hDB);
	if (nStartPoints)
	{
		outFid = GSSiOpenFile (OutFile,0,OF_CREATE);
		fputstring ("SEQINSEG\tMSLINKINSEG",outFid);
		if ((rtn = TraceStreetLink (recordNum,wantSNum,startMSLink[0],nextIntPoint[0],toIntPoint,outFid,segFile)))
			goto Exit;
		if (nStartPoints > 1)
		{
			GSSiClose (outFid);
			outFid = GSSiOpenFile (OutFile,0,OF_CREATE);
			fputstring ("SEQINSEG\tMSLINKINSEG",outFid);
			if ((rtn = TraceStreetLink (recordNum,wantSNum,startMSLink[1],nextIntPoint[1],toIntPoint,outFid,segFile)))
				goto Exit;
		}
	}
Exit:
	GSSiClose (outFid);
	return rtn;
}

//$JOINLINESBETWEENPOINTS (SETUP,maxlinelength)
// highlight list contains all lines to join
// current picklist contains points to snap to 
// current pickap contains snap tol
// maxlinelength contains max length of line to join (no max if not set)
//$JOINLINESBETWEENPOINTS (NEXT,reflistfile,maxlengthtag,pointlistglobal)
// returns 0 if done or 1 if line returned
// reflist global contains list of refnos linked in order of length (longest first)
// point list global contains handle to point list
long JoinLinesBetweenPoints (LPSTR Option,LPSTR Arg2,LPSTR Arg3,LPSTR Arg4)
{   
	long	rtn=0;  
static	double	MaxLineLength;
	char	DefStr[]="Length(R8),Refno(B4),Prefix(C8),UDI(C32),BPX(R8),BPY(R8),EPX(R8),EPY(R8)";
	LPGWDHEADER lpGWDHead;    
	HIGHLIGHTDATA	HighlightData;
	long	Refno, Offset;  
	short	pos=BT_FIRST;  
	LPSTR	pDot; 
	char	OldTAG[42];
typedef struct {
				double	Length;
				long	Refno;  
				char	Prefix[8];
				char	UDI[32];
				double	BPX,BPY,EPX,EPY;
				}JLBPDATA;
typedef JLBPDATA	FAR	*LPJLBPDATA;
LPJLBPDATA	pData;	

	if (!Option || !_fstricmp (Option,"CLOSE"))
	{
		if (hDBJLBP)
		{
			CloseGWDatabase (hDBJLBP); 
			GSSiRemove (JLBPFile);
		}
		return TRUE;
	}	
	if (!_fstricmp (Option,"SETUP"))
	{ 
		if (*Arg2)
			MaxLineLength = atof (Arg2);
		else
			MaxLineLength  = DBL_MAX;
	
		GSSiGetTempFileName (0,"gm",0,JLBPFile); 
		if ((pDot = _fstrrchr (JLBPFile,'.')))
			_fstrcpy (pDot,".gmd");
		

		CreateGWDDatabase (JLBPFile,1,FALSE,0,2,DefStr);
		{
			HANDLE	hKeyFields;
				
			GetFieldIDsFromNames (JLBPFile,&hKeyFields,0,"BPX;BPY",0);
			GWDAddIndex (JLBPFile,hKeyFields,FALSE,0);
			GSSiGlobFree (&hKeyFields);
			GetFieldIDsFromNames (JLBPFile,&hKeyFields,0,"EPX;EPY",0);
			GWDAddIndex (JLBPFile,hKeyFields,FALSE,0);
			GSSiGlobFree (&hKeyFields);
		} 
    	hDBJLBP = OpenGWDatabase (JLBPFile,BT_WRITE);  
    	lpGWDHead = (LPGWDHEADER)GlobalLock (hDBJLBP);
		pData = (LPJLBPDATA)&lpGWDHead->GWDData;
		while (ContinueProcessing && !BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData)) 
		{   
			pos = BT_NEXT;  
			PickList[0] = HighlightData.PD;
			if (PickList[0].Type == 2)
			{ 
				pData->Length = -PickList[0].Length;
				pData->Refno = Refno;  
				_fstrcpy (pData->Prefix,PickList[0].Prefix);
				_fstrncpy (pData->UDI,PickList[0].UDI,32);
				pData->BPX = PickList[0].BeginPoint.x;
				pData->BPY = PickList[0].BeginPoint.y;
				pData->EPX = PickList[0].EndPoint.x;
				pData->EPY = PickList[0].EndPoint.y; 
				if (ldistp (PickList[0].BeginPoint,PickList[0].EndPoint) > P_TOL)
	        		GWDAddRecord (lpGWDHead,0,0); 
	        }
	    }
    	rtn = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[0]);
 
	    GlobalUnlock (hDBJLBP);
	}
	else if (!_fstricmp (Option,"NEXT"))
	{   
		DPOINT	PickPoint, OldBP,OldEP,NewBP,NewEP; 
		LPDPOINT	OutPoints, Points, Points2;
		short	SaveMaxPick = MaxPick, nref=0, WhichEnd;  
		long	np,np2,i; 
		BOOL	HaveBP=FALSE, HaveEP=FALSE, Reverse;
		long	LinkedRefnos[64]; 
		HANDLE	hPoints=0, hPoints2,hTran, hOutPoints=0; 
		double	XFROM[2],YFROM[2],XTO[2],YTO[2];
		float	RSQMIN; 
		HFILE	FidRefs;
		
    	lpGWDHead = (LPGWDHEADER)GlobalLock (hDBJLBP);
		pData = (LPJLBPDATA)&lpGWDHead->GWDData;
		if (BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)lpGWDHead->pKeys[0],BT_FIRST,BT_ANY,(LPSTR)&Offset))
		{
		    GlobalUnlock (hDBJLBP);
			CloseGWDatabase (hDBJLBP);  
			hDBJLBP = 0;
			GSSiRemove (JLBPFile);
			return FALSE;
		} 
		MaxPick=1;  
	    FillGWDData (lpGWDHead,Offset); 
		OldBP.x = pData->BPX;
		OldBP.y = pData->BPY;
		OldEP.x = pData->EPX;
		OldEP.y = pData->EPY;
		sprintf (OldTAG,"%s:%s",pData->Prefix,pData->UDI);
		LinkedRefnos[nref++] = pData->Refno;
		GWDDeleteRecord (lpGWDHead,Offset);  
		if (PickByRefno(pData->Refno,0,0,-1)) 
		{
			GetPolyPnts ((LPPICKDATAHEADER)&PickList[0],FALSE,&np, &hPoints,TRUE);  
		    if (PickItems (CurView->hWnd,OldBP)) 
		    {
		    	HaveBP = TRUE;
		    	NewBP = PickList[0].BeginPoint;
		    }  
		    if (PickItems (CurView->hWnd,OldEP)) 
		    {
		    	HaveEP = TRUE;
		    	NewEP = PickList[0].BeginPoint;
		    }  
			if (fabs (pData->Length < MaxLineLength)) 
			{
				while (!HaveBP && nref < 64 && (WhichEnd=FindConnectingRef (1,OldBP,lpGWDHead,&Offset,PickApW)))
				{
				    FillGWDData (lpGWDHead,Offset);
				    if (WhichEnd == 1)
				    {
				    	Reverse = TRUE; 
						OldBP.x = pData->EPX;
						OldBP.y = pData->EPY;
					}
				    else
				    {
				    	Reverse = FALSE; 
						OldBP.x = pData->BPX;
						OldBP.y = pData->BPY;
					}
					LinkedRefnos[nref++] = pData->Refno;
					GWDDeleteRecord (lpGWDHead,Offset);  
					if (PickByRefno(pData->Refno,0,0,-1)) 
					{
						GetPolyPnts ((LPPICKDATAHEADER)&PickList[0],Reverse,&np2, &hPoints2,TRUE);
				    	hPoints2 = GSSiGlobalReAlloc (0,hPoints2 ,(np + np2)*sizeof(DPOINT),GMEM_MOVEABLE);
				    	Points2 = (LPDPOINT)GlobalLock (hPoints2);
				    	Points = (LPDPOINT)GlobalLock (hPoints);
				    	for (i=0;i<np;i++)
				    		Points2[np2+i] = Points[i];
				    	GlobalUnlock (hPoints2);
				    	GSSiGlobUlFree (&hPoints);
				    	hPoints = hPoints2;
				    	np += np2;
					}  
				    if (PickItems (CurView->hWnd,OldBP)) 
				    {
				    	HaveBP = TRUE;
				    	NewBP = PickList[0].BeginPoint;
				    }  
				}
				while (!HaveEP && nref < 64 && (WhichEnd=FindConnectingRef (2,OldEP,lpGWDHead,&Offset,PickApW)))
				{
				    FillGWDData (lpGWDHead,Offset);
				    if (WhichEnd == 2)
				    {
				    	Reverse = TRUE; 
						OldEP.x = pData->BPX;
						OldEP.y = pData->BPY;
					}
				    else
				    {
				    	Reverse = FALSE; 
						OldEP.x = pData->EPX;
						OldEP.y = pData->EPY;
					}
					LinkedRefnos[nref++] = pData->Refno;
					GWDDeleteRecord (lpGWDHead,Offset);  
					if (PickByRefno(pData->Refno,0,0,-1)) 
					{
						GetPolyPnts ((LPPICKDATAHEADER)&PickList[0],Reverse,&np2, &hPoints2,TRUE);
				    	hPoints = GSSiGlobalReAlloc (0,hPoints ,(np + np2)*sizeof(DPOINT),GMEM_MOVEABLE);
				    	Points2 = (LPDPOINT)GlobalLock (hPoints2);
				    	Points = (LPDPOINT)GlobalLock (hPoints);
				    	for (i=0;i<np2;i++)
				    		Points[np+i] = Points2[i];
				    	GlobalUnlock (hPoints);
				    	GSSiGlobUlFree (&hPoints2);
				    	np += np2;
					}  
				    if (PickItems (CurView->hWnd,OldEP)) 
				    {
				    	HaveEP = TRUE;
				    	NewEP = PickList[0].BeginPoint;
				    }  
				}
			}
			if (!HaveBP)
				NewBP = OldBP;
			if (!HaveEP)
				NewEP = OldEP; 
			XFROM[0] = OldBP.x;
			XFROM[1] = OldEP.x;
			YFROM[0] = OldBP.y;
			YFROM[1] = OldEP.y; 
			XTO[0] = NewBP.x;
			XTO[1] = NewEP.x;
			YTO[0] = NewBP.y;
			YTO[1] = NewEP.y;
			hTran = STRAN2 (1635,XFROM,YFROM,XTO,YTO,2,&RSQMIN,1,0);  
			hOutPoints = GSSiGlobAlloc (1549,GMEM_MOVEABLE,sizeof(DPOINT)*(1+np));
			Points = (LPDPOINT)GlobalLock (hPoints);
			OutPoints = (LPDPOINT)GlobalLock (hOutPoints);
			for (i=0;i<np;i++)
			{
				OutPoints[i] = TranPoint(&Points[i],hTran);
			}  
			OutPoints[np].x = DBL_MAX;
			GlobalUnlock (hOutPoints); 
			GSSiGlobUlFree (&hPoints);  
			CloseTRANS2 (&hTran);
			SetGlobalValueHandle (Arg4,hOutPoints);  
			FidRefs = GSSiOpenFile (Arg2,0,OF_CREATE);
			fputstring ("JOINEDREF",FidRefs);
			for (i=0;i<nref;i++)
			{   
				char	str[32];
				
				ltoa (LinkedRefnos[i],str,10);
				fputstring (str,FidRefs);
			}
			GSSiClose (FidRefs);
			SetGlobalValueLong (Arg2,LinkedRefnos[0]);
			SetGlobalValue (Arg3,OldTAG);
		}
		else
			SetGlobalValueHandle (Arg3,0);
				
		MaxPick = SaveMaxPick;
    	rtn = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[0])+1;
	    GlobalUnlock (hDBJLBP);
	}
	return rtn;
}

void SetTransferFileName (LPSTR Option,LPSTR Name)
{  
	*TransferFileRunCommand = 0;
	_fstrcpy (BuildTransferFileOption,Option);
	_fstrcpy (TransferFileName,Name);
	return;
} 

void RunTransferFileCommand (void)
{
	ExpandText (TransferFileRunCommand);
	return;
}

BOOL AddFileToTransferFile (HWND hWndStatus,HFILE FidTF,LPSTR FileToAdd,long MaxLength)
{   
	long	lRec;
    HANDLE	hRec = GSSiGlobAlloc (1550,GMEM_MOVEABLE,MaxLength);
    HPSTR	pRec = GlobalLock (hRec); 
    HANDLE	hCompressedRec = GSSiGlobAlloc (1551,GMEM_MOVEABLE,MaxLength*2);
    HPSTR	pCompressedRec = GlobalLock (hCompressedRec); 
    long	CompressedLength;  
	HFILE	Fid=GSSiOpenFile (FileToAdd,0,OF_READ);
	long	TotLen = GSSifilelength (Fid);
	
   	PctBox (hWndStatus,TotLen,GSSillseek (Fid,0,1),0); 
	while ((lRec=BigRead (Fid,pRec,MaxLength)))
	{
	 	lRec = CompressBinaryRecord (pRec,pCompressedRec,lRec); 	    
    	BigWrite (FidTF,(HPSTR)&lRec,4,-1);
    	BigWrite (FidTF,(HPSTR)pCompressedRec,lRec,-1);       
    	PctBox (hWndStatus,TotLen,GSSillseek (Fid,0,1),0); 
    }
    GSSiClose (Fid);
    GSSiGlobUlFree (&hCompressedRec); 
    GSSiGlobUlFree (&hRec);
    return TRUE; 
}

BOOL GetFileFromTransferFile (HWND hWndStatus,HFILE FidTF,LPSTR FileToGet,long LenToRead,long MaxLength)
{   
	long	lRec;
    HANDLE	hRec = GSSiGlobAlloc (1552,GMEM_MOVEABLE,MaxLength);
    HPSTR	pRec = GlobalLock (hRec); 
    HANDLE	hCompressedRec = GSSiGlobAlloc (1553,GMEM_MOVEABLE,MaxLength*2);
    HPSTR	pCompressedRec = GlobalLock (hCompressedRec); 
    long	CompressedLength, LenRead=0;  
	HFILE	Fid=GSSiOpenFile (FileToGet,0,OF_CREATE);
	
	if (Fid == HFILE_ERROR)
		return FALSE;
   	PctBox (hWndStatus,LenToRead,LenRead,0); 
	while (LenRead < LenToRead)
	{   
		BigRead (FidTF,(HPSTR)&CompressedLength,4);
		LenRead += CompressedLength+4;     
		BigRead (FidTF,pCompressedRec,CompressedLength);
		lRec = DecompressBinaryRecordUnsafe (pRec,pCompressedRec,CompressedLength);
    	BigWrite (Fid,(HPSTR)pRec,lRec,-1);
    	PctBox (hWndStatus,LenToRead,LenRead,0); 
    }
    GSSiClose (Fid);
    GSSiGlobUlFree (&hCompressedRec); 
    GSSiGlobUlFree (&hRec);
    return TRUE; 
}


BOOL UnmatchedRecInVis (HANDLE hDB2,LPSTR UDI,LPSHORT pCurrentDesc)
{
	BOOL	rtn=FALSE;
	long	Offset;
	int		pos=BT_FIRST, cond=BT_GE;
	int		WantCase, st;
	struct	{long Case,Seq;}Header;
	typedef struct	{long	Case,Seq;
			 short Severity;
			 long  Data;
			 char  CrimePart;
			 char  UCR[5];
			 char  OFFID[6];}DATA;
	typedef	DATA	*LPDATA;
	LPDATA	pData;
	char	CDesc[16];

	if (hDB2)
	{
	    LPGWDHEADER	lpGWDHead = (LPGWDHEADER)GlobalLock (hDB2); 

		pData = (LPDATA)&lpGWDHead->GWDData;
		WantCase = atoi (UDI);
		Header.Case = WantCase;
		Header.Seq = 0;
		do
		{
			if (!(st = BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&Header,pos,cond, (LPSTR)&Offset)))
			{   
			   	pos = BT_NEXT;
			   	cond = BT_ANY; 

				FillGWDData (lpGWDHead,Offset); 
  				CurrentRefno = Header.Seq; 
				sprintf (CDesc,"UCR_%c",pData->UCR[0]);
				*pCurrentDesc = CurrentDesc = GetDictSymbolNumber (CDesc);
				if ((rtn = GetVisibility (CurrentDesc)))
					break;
			}
		}
		while (!st && Header.Case == WantCase);
		GlobalUnlock (hDB2);
	}
	return rtn;
}

BOOL ProcessDisplayMacro (HFILE Fid,int Opt)
{   
	LPGWDHEADER lpGWDHead;
	static	HANDLE  hDB=0, hDB2=0;   
	long	Offset;
	static	short 	pos, cond; 
	BOOL	rtn=FALSE; 
	long	WantArea= 322814; 
	short	Zero=0;   
	char	str[128], Prefix[10],UDI[64];
	long	AreaRef, Refno;
	static	long	nFound;
	short	Symnum; 
	DPOINT	DPoint;
				
	switch (Opt)
	{
		case 1:
			hDB = OpenGWDatabase ("[%DL]attribut\\nomatch.gmd",BT_READ);
			hDB2 = OpenGWDatabase ("[%DL]attribut\\reports2.gmd",BT_READ);
			if (!hDB)
				return FALSE; 
			pos = BT_FIRST;
			cond = BT_GE;
			return TRUE;  
		case 4:
			Offset = CurrentMacroRec;
		    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
			goto ProcessRec;
		case 2:
			if (CurView->PassID == 2 || !hDB)
				return FALSE;
		    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
		    if (pos == BT_FIRST)
		    {   
		    	nFound=0;
		 		SetFieldValFromCharAndName(lpGWDHead,"AREAREF",(LPSTR)&WantArea,TRUE);
		 		SetFieldValFromCharAndName(lpGWDHead,"TIME",(LPSTR)&TimeRangeBeg,TRUE);
		 		SetFieldValFromCharAndName(lpGWDHead,"SYMNUM",(LPSTR)&Zero,TRUE);
	            GWDFormKey(lpGWDHead,1,TRUE,0,0); 
	        }
			if (!BT_FIND (lpGWDHead->BTHandle[1],lpGWDHead->pKeys[1],pos,cond, (LPSTR)&Offset))
			{   
			   	pos = BT_NEXT;
			   	cond = BT_ANY; 
		   		ItemSeg = Offset;  
ProcessRec:
			    FillGWDData (lpGWDHead,Offset); 
 				GMDGetCharFieldVal (lpGWDHead,0,str);
 				CurrentRefno = atol (str); 
 				GMDGetCharFieldVal (lpGWDHead,1,Prefix);
 				GMDGetCharFieldVal (lpGWDHead,2,UDI);
 				GMDGetCharFieldVal (lpGWDHead,3,str);
 				AreaRef = atol (str); 
 				GMDGetCharFieldVal (lpGWDHead,4,str);
 				GRStartTime = GREndTime = atol (str); 
 				GMDGetCharFieldVal (lpGWDHead,5,str);
 				CurrentDesc = atol (str); 
 				CurPointSize = 5;
 				if (AreaRef == WantArea && GRStartTime < TimeRangeEnd)
 				{   
 					char	Tag[80];
 					short	ltag;
 					
		    		rtn = TRUE;
					if (UnmatchedRecInVis (hDB2,UDI,&CurrentDesc))
					{
						SetSymNum (CurrentDesc);
						SelectClipRgn (CurView->hDC,0);
 						sprintf (Tag,"%s:%s",Prefix,UDI);
 						ltag = _fstrlen (Tag);
						if (ProcessRefAndTAG (TRUE,Tag,ltag))
						{  
	 						nFound++;
							DPoint = SubVPMidPointWorld;//MinMaxMidPointD (&ORARecordHeader.Bounds); 
							lpDCurPoints = &DPoint;  
							CurrentPoint = CurPointLocD = DPoint;
							LastElementBeginPoint = LastElementEndPoint = DPoint;
							nPnts = nCurPoints = 1;
							CurPointLoc = SubVPMidPointWin;//BasePtToWinPt(lpDCurPoints); 
			    			//if (!PointInMaskAreaWinCoord (CurPointLoc))
			    			//	goto RtnFalse;
			    			if (PointIsBlocked (&CurPointLocD,CurrentDesc))
			    				goto RtnFalse;
		    				InGraphicsProcessor = TRUE;
			       			HaveTXLoc = TRUE;
			    			CurrentType = GF_POINT; 
							if (CurPointSize < 0)
								CurPointSize = -CurPointSize * DeviceToScreenFactor;
							else
								CurPointSize /= CurView->BaseUnitsPerPixel;
							if ((Pick||PickingByRefno) && GetTypeVisibility(TYPE_POINT))
							{
								PickPointItemD (lpDCurPoints,(CurPointSize*ThemeWidthFactor)*CurView->BaseUnitsPerPixel,PTRot,CurrentDesc);			 		    
							} 
							else if (GetTypeVisibility(TYPE_POINT))
							{   
								short	iDesc=CurrentDesc;
								
								if (CurrentDesc > 0 && CurrentDesc < 3201)	
								{	
									if (TSize)
										CurView->CurVisType[CurrentDesc]=5;
									else
										CurView->CurVisType[CurrentDesc]=4;  
								}
								HighlightPointSym=FALSE;
								if (!GetTypeVisibility(6) && SymbolIsVisible (iDesc)) 
								{
									CurPointSize = 10*DeviceToScreenFactor;
									iDesc = InvisiblePointSymbol;
								}
								if (SetDisplayChar (CurView->hDC,GF_POINT,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI) > 0)
								{   
									double	size;
									
									if (ThemePointSym)
									{
										iDesc = ThemePointSym;
										if (ThemePointSize < 0)
											size = -ThemePointSize *DeviceToScreenFactor;
										else
											size = ThemePointSize / CurView->BaseUnitsPerPixel; 
										size *= ThemeWidthFactor;
										size = min(max (size,1),MaxPointSize);
									}
									else if (ItemSymbolWidth > 0)
										size = ItemSymbolWidth * CurPointSize*ThemeWidthFactor*GraphicsPointFactor;
									else if (ItemSymbolWidth < 0)
										size = -ItemSymbolWidth * BaseDistToWinDist * CurPointSize*ThemeWidthFactor*GraphicsPointFactor;
									else
										size = CurPointSize*ThemeWidthFactor*GraphicsPointFactor;
									if (iDesc < 0) 
									{   
										COLORREF	OldColor;
										
										if (ThemePointColor > -1)
											OldColor = 	SetTextColor (CurView->hDC,ConvertColor(ThemePointColor,ThemePointUseHalfTone));
										DisplayCharAtLoc (CurView->hDC,CurPointLoc,(short)IDNINT(size),-iDesc); 
										if (ThemePointColor > -1)
											SetTextColor (CurView->hDC,OldColor);
									}
									else
									{
										long	DisplayedWidth=0;

										DisplayPointItem (CurView->hDC,CurPointLoc,size,PTRot,iDesc,&DisplayedWidth);
										CurView->MaxSymbolWidth = max (CurView->MaxSymbolWidth,DisplayedWidth);
										CurView->MaxFileDisplayedPointWidth[FileNum] = max (CurView->MaxFileDisplayedPointWidth[FileNum],(DisplayedWidth/ FileDistToWinDist)-(((long)CurrentItemMinMax.xmx)-CurrentItemMinMax.xmn));
									}
								} 
							}
							InGraphicsProcessor = FALSE;
			   				TXLoc = CurPointLocD; 
			       			HaveTXLoc = 1;  
						}
					}
			    } 
			    if (Opt == 4)
			    	goto RtnFalse;
			}
RtnFalse:
			GlobalUnlock (hDB); 
			break;
		case 3:
			CloseGWDatabase (hDB); 
			hDB = 0;
			CloseGWDatabase (hDB2); 
			hDB2 = 0;
			return TRUE;
	}
	return rtn;
}

void GetGMDName (LPSTR Name)
{
	_fstrcpy (Name,LastGMDFile);
	return;
}                            

BOOL FAR PASCAL SETGMDPARAMMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
	char	SymName[66], cWidth[64],cRot[64],cColor[64],str[260], GSPName[256], cIF[128], UDI[66], SymStuff[256]; 
	short	i, Choice, rtn; 
	LPSTR	pDOT;  
	HFILE	Fid;  
	MNMXCORD	Bounds;  
	static	short	CurProj=-1,CurUnits=-1;
   	static	BOOL	Opened;
	long	ii;

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
 	return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:
		Opened = FALSE;
    	GetGMDName (str); 
    	SetDlgItemText (hWndDlg,IDC_GMDFILE,str);
        _fstrcpy (GSPName,str);   
        if ((pDOT = _fstrrchr (GSPName,'.')))
       		_fstrcpy (pDOT,".gmp");
        else
        	break;
    	SetCurVal (GSPName,IDS_FILEGSP); 
        LoadTAGDef ();   
        if (NumTAGDef)
         { 
            LPTAGDEF    lpTAGDef; 
            
            lpTAGDef = (LPTAGDEF)GlobalLock (hTAGDef);
            for (i=0;i<NumTAGDef;i++,lpTAGDef++) 
            {
                SendDlgItemMessage (hWndDlg,IDC_TAPREFIX,CB_ADDSTRING,0,(LPARAM)lpTAGDef->Prefix);
                SendDlgItemMessage (hWndDlg,IDC_REF_PREFIX,CB_ADDSTRING,0,(LPARAM)lpTAGDef->Prefix);
            }
            GlobalUnlock (hTAGDef);
         } 
         EnableWindow (GetDlgItem(hWndDlg,IDC_SAADD),TRUE);
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Feet");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Meters");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Degrees");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Degrees * 1000000");
         FillProjectionList (hWndDlg,IDC_PROJECTION,&CurProj,IDC_UNITS,&CurUnits);
//         _fstrcpy (str,"*.CVT");
//         DlgDirListComboBox (hWndDlg,str,IDC_PROJECTION,0,DDL_READWRITE); 
		 if (PRJ_UNITS[1] == 1)
 		 	SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_SETCURSEL,(WPARAM)0,(LPARAM)0); 
		 else if (PRJ_UNITS[1] == 2)
 		 	SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_SETCURSEL,(WPARAM)1,(LPARAM)0); 
 		 SendDlgItemMessage (hWndDlg,IDC_PROJECTION,CB_SELECTSTRING,(WPARAM)-1,(LPARAM)"baseproj"); 
		 SendDlgItemMessage (hWndDlg,IDC_INDEXSTANDARD,BM_SETCHECK,SHPIndexType == 0,0L);
		 SendDlgItemMessage (hWndDlg,IDC_INDEXSIMPLE,BM_SETCHECK,SHPIndexType == 1,0L);
		 SendDlgItemMessage (hWndDlg,IDC_INDEXQUAD,BM_SETCHECK,SHPIndexType == 2,0L);  
    	 GetGMDName (str);
		 if (GMDHandle)
		 {
		    LPOPENSQLDATA	SQLPtr = (LPOPENSQLDATA)GlobalLock (GMDHandle);
			LPOPENFILEDATA	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
			LPGWDHEADER		lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle);

			GMDFileBounds = lpGWDHead->FileBounds;
			if (lpGWDHead->SpatialIndex)
			{
				SetDlgItemText (hWndDlg,IDC_SHPMINX,ftoa(str,lpGWDHead->FileBounds.xmn));
				SetDlgItemText (hWndDlg,IDC_SHPMAXX,ftoa(str,lpGWDHead->FileBounds.xmx));
				SetDlgItemText (hWndDlg,IDC_SHPMINY,ftoa(str,lpGWDHead->FileBounds.ymn));
				SetDlgItemText (hWndDlg,IDC_SHPMAXY,ftoa(str,lpGWDHead->FileBounds.ymx));
			}
			GlobalUnlock (FilePtr->FileHandle);
			GlobalUnlock (SQLPtr->OFHandle); 
			GlobalUnlock (GMDHandle);
		 }
         goto LoadGSP;
         break;  

    case WM_COMMAND:

         switch(LOWORD(wParam))

         {  
            case IDC_SHOW_FIELDS: 
            {
            	 
            	if (!hSHPDBF)
            	{   
            		char	DBFName[256];
            		
			    	GetGMDName (str);
					sprintf (DBFName,"GMD=%s",str);
					OpenDataFile (DBFName,"",BT_READ,&hSHPDBF);
					Opened = TRUE; 
				}

            	DisplayFieldList (hWndDlg,hSHPDBF,0,0,0); 
            }	
                 break;
                      
            case IDC_COPYFROM: 
                 *GSPName=0;
                 if (!GetFileName3 (hWndDlg,GSPName,IDS_FILTERGMP,IDS_FILEGMP))
                 	break;
   LoadGSP:
				 SendDlgItemMessage (hWndDlg,IDC_SYMBOL_LIST,LB_RESETCONTENT,0,0);
				 SendDlgItemMessage (hWndDlg,IDC_SQL_LIST,LB_RESETCONTENT,0,0);
  				 {
   				 	HFILE	Fid = GSSiOpenFile (GSPName,0,OF_READ); 
   				 	short	SymNum=0;
   				 	
   				 	if (Fid == HFILE_ERROR)  
   				 	{   
   				 		short	type;
   				 		
				    	GetDlgItemText (hWndDlg,IDC_GMDFILE,str,256);
						GuessProjection (hWndDlg,IDC_PROJECTION,IDC_UNITS,&GMDFileBounds,TRUE); 
						_fstrcpy (cRot,"0D");
						_fstrcpy (cColor,"0");

						GetGlobalCVal ("[%DEFAULTPOINTSYMBOL]",SymName,"SQUARE"); 
						_fstrcpy (cWidth,"5P"); 
						if (!SymNum) 
						{
			                 *cWidth = 0;
			                 *cRot = 0;
			                 *cColor = 0; 
							 rtn = SelectPointSymbol (hWndDlg,1,SymName,"All",cWidth,cRot,cColor,TRUE);
			            }
	                 	sprintf (SymStuff,"%s;%s;%s;%s",SymName,cWidth,cRot,cColor);
	                 	SendDlgItemMessage (hWndDlg,IDC_SYMBOL_LIST,LB_ADDSTRING,0,(LPARAM)SymStuff);
	                	SendDlgItemMessage (hWndDlg,IDC_SQL_LIST,LB_ADDSTRING,0,(LPARAM)"ALL ROWS");
   				 		break; 
   				 	}
   				 	if (fgetstring (str,128,Fid))
					{
						HFILE	Fid2 = GSSiOpenFile (str,0,OF_READ);
						if (Fid2 != HFILE_ERROR)
						{   
							fgetstring (str,250,Fid2);
							SendDlgItemMessage (hWndDlg,IDC_PROJECTION,CB_SELECTSTRING,-1,(LPARAM)(str+1));
							GSSiClose (Fid2);
						}
					}
   				 	if (fgetstring (str,128,Fid))
			         	SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_SELECTSTRING,-1,(LPARAM)str);
   				 	if (fgetstring (str,128,Fid))
   				 		SetDlgItemText (hWndDlg,IDC_STARTNO,str);   
   				 	if (fgetstring (str,128,Fid))
   				 	{   
   				 		LPSTR	pSC = _fstrchr (str,':');
   				 		
   				 		if (pSC)
   				 		{
   				 			*pSC++ = 0;
   				 			SetDlgItemText (hWndDlg,IDC_UDI,pSC);
   				 		}
   				 		SetDlgItemText (hWndDlg,IDC_TAPREFIX,str); 
   				 	}
   
   				 	while (fgetstring (str,256,Fid))
   				 	{
						if (*str == '#')
							break;
						DecodeSHPParam (str,SymName,cIF,cColor,cWidth,cRot);
	                 	sprintf (str,"%s;%s;%s;%s",SymName,cWidth,cRot,cColor);
		                SendDlgItemMessage (hWndDlg,IDC_SYMBOL_LIST,LB_ADDSTRING,0,(LPARAM)str); 
		                if (*cIF)
		                	SendDlgItemMessage (hWndDlg,IDC_SQL_LIST,LB_ADDSTRING,0,(LPARAM)cIF);
		                else
		                	SendDlgItemMessage (hWndDlg,IDC_SQL_LIST,LB_ADDSTRING,0,(LPARAM)"ALL ROWS");
   				 	} 
					if (fgetstring (str,256,Fid))
	                	SetDlgItemText (hWndDlg,IDC_BEGINDATE,str);
					if (fgetstring (str,256,Fid))
	                	SetDlgItemText (hWndDlg,IDC_ENDDATE,str);
   				 	GSSiClose (Fid);
   				 }
   				 		   
                 break;
            
            case IDC_ASSIGNCVT:
		    	GetDlgItemText (hWndDlg,IDC_SHAPEFILE,str,128);
            	GuessShapeProjection (str,hWndDlg,IDC_PROJECTION,IDC_UNITS,FALSE);
            break;
            
            case IDC_SAADD:      
            {    
                 HANDLE hMem;
                 LPSTR  lpStr, lpWhere;   
                 
            	if (!hSHPDBF)
            	{   
            		char	DBFName[256];
            		
			    	GetGMDName (str);
					sprintf (DBFName,"SHP=%s",str);
					OpenDataFile (DBFName,"",BT_READ,&hSHPDBF);
					Opened = TRUE; 
				}
                 *SymName = 0;
                 *cWidth = 0;
                 *cRot = 0;
                 *cColor = 0; 
				 rtn = SelectPointSymbol (hWndDlg,1,SymName,"All",cWidth,cRot,cColor,FALSE);
                 if (!rtn)
                 	break;
                 hMem = GSSiGlobAlloc (1421,GHND,4096);
                 lpStr = GlobalLock (hMem);
                 if (GetSQLWhereClause (hWndDlg, hSHPDBF, lpStr))
                 {  
                 	sprintf (str,"%s;%s;%s;%s",SymName,cWidth,cRot,cColor);
	                SendDlgItemMessage (hWndDlg,IDC_SYMBOL_LIST,LB_ADDSTRING,0,(LPARAM)str);
	                SendDlgItemMessage (hWndDlg,IDC_SQL_LIST,LB_ADDSTRING,0,(LPARAM)lpStr);
                 }
                 GSSiGlobUlFree (&hMem);
                 break;
            }    
            case IDC_SQL_LIST:  
            case IDC_SYMBOL_LIST:
			{   
				WPARAM	OtherParam;
				
				switch(HIWORD(wParam))
			    {
			     case LBN_DBLCLK:
			          PostMessage(hWndDlg, WM_COMMAND, IDC_SAMODIFY, 0L); 
			          break;
			     case LBN_SELCHANGE:
			          Choice=SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_GETCURSEL,0,0);
			          if (LOWORD(wParam) == IDC_SQL_LIST)
			          	OtherParam = IDC_SYMBOL_LIST;
			          else
			          	OtherParam = IDC_SQL_LIST; 
			 		  SendDlgItemMessage(hWndDlg,OtherParam,LB_SETCURSEL,-1,0);
				 	  EnableWindow (GetDlgItem(hWndDlg,IDC_SADELETE),TRUE);
				 	  EnableWindow (GetDlgItem(hWndDlg,IDC_SAMODIFY),TRUE);
				 	  break; 
			    }
			}
				 break;
            case IDC_SADELETE:
		         Choice=max(SendDlgItemMessage(hWndDlg,IDC_SYMBOL_LIST,LB_GETCURSEL,0,0),
							SendDlgItemMessage(hWndDlg,IDC_SQL_LIST,LB_GETCURSEL,0,0));
	             SendDlgItemMessage (hWndDlg,IDC_SYMBOL_LIST,LB_DELETESTRING,Choice,0);
               	 SendDlgItemMessage (hWndDlg,IDC_SQL_LIST,LB_DELETESTRING,Choice,0);
                 break;
            
            case IDC_SAMODIFY:
		         if ((Choice=SendDlgItemMessage(hWndDlg,IDC_SYMBOL_LIST,LB_GETCURSEL,0,0)) >= 0)
		         {
	                 SendDlgItemMessage(hWndDlg,IDC_SYMBOL_LIST,LB_GETTEXT,Choice,(DWORD)SymStuff);
	                 SetSymParms (SymStuff,SymName,cWidth,cRot,cColor);            
					 rtn = SelectPointSymbol (hWndDlg,1,SymName,"All",cWidth,cRot,cColor,FALSE);
	                 if (!rtn)
	                 	break;
	                 SendDlgItemMessage (hWndDlg,IDC_SYMBOL_LIST,LB_DELETESTRING,Choice,0);
                 	 sprintf (SymStuff,"%s;%s;%s;%s",SymName,cWidth,cRot,cColor);
	                 SendDlgItemMessage (hWndDlg,IDC_SYMBOL_LIST,LB_INSERTSTRING,Choice,(LPARAM)SymStuff);
		         }
		         else
		         if ((Choice=SendDlgItemMessage(hWndDlg,IDC_SQL_LIST,LB_GETCURSEL,0,0)) >= 0)
		         {   
		         	 HANDLE	hMem;
		         	 LPSTR	lpStr;
		         	 
	                 hMem = GSSiGlobAlloc (1422,GHND,4096);
	                 lpStr = GlobalLock (hMem);
                	 SendDlgItemMessage(hWndDlg,IDC_SQL_LIST,LB_GETTEXT,Choice,(DWORD)lpStr); 
            		if (!hSHPDBF)
            		{   
            			char	DBFName[256];
            			
			    		GetGMDName (str);
						sprintf (DBFName,"SHP=%s",str);
						OpenDataFile (DBFName,"",BT_READ,&hSHPDBF);
						Opened = TRUE; 
					}
	                 if (GetSQLWhereClause (hWndDlg, hSHPDBF, lpStr))
	                 {
	                 	SendDlgItemMessage (hWndDlg,IDC_SQL_LIST,LB_DELETESTRING,Choice,0);
		                SendDlgItemMessage (hWndDlg,IDC_SQL_LIST,LB_INSERTSTRING,Choice,(LPARAM)lpStr);
	                 }
	                 GSSiGlobUlFree (&hMem);
		         }
		         
                 break;
            
            case IDCANCEL:   
            	SetGlobalValueBool ("%AUTOSHPPARM",FALSE);
            	if (Opened) 
            	{
					CloseDataFile (TRUE,&hSHPDBF); 
					DestroyFieldList();
				}
	            EndDialog(hWndDlg,FALSE);  
				break;
				 
            case IDOK: 
				GetGMDName (str); 
				_fstrcpy (GSPName,str);
		        if ((pDOT = _fstrrchr (GSPName,'.')))
		        {
	        		_fstrcpy (pDOT,".gmp");
		        }
		        else
		        	break;
				Fid = GSSiOpenFile (GSPName,0,OF_CREATE);
				if (Fid == HFILE_ERROR)
				{   
					MessageBox (hWndDlg,GSPName,"Unable to create file",MB_ICONEXCLAMATION);
					break;
				} 
		 		//GetDlgItemText (hWndDlg,IDC_PROJECTION,str,sizeof(str)-1);  
		 		GetProjectionFile (hWndDlg,IDC_PROJECTION,str);
		        SubstituteDL (str,TRUE);
				//ExpandText (str);
			 	fputstring (str,Fid); 
		 		GetDlgItemText (hWndDlg,IDC_UNITS,str,sizeof(str)-1);  
			 	fputstring (str,Fid); 
		 		GetDlgItemText (hWndDlg,IDC_STARTNO,str,sizeof(str)-1);  
			 	fputstring (str,Fid); 
		 		if (GetDlgItemText (hWndDlg,IDC_TAPREFIX,str,sizeof(str)-1))
		 		{
		 			_fstrcat (str,":");
		 			GetDlgItemText (hWndDlg,IDC_UDI,_fstrchr(str,0),sizeof(str)-_fstrlen(str)-1); 
		 		} 
			 	fputstring (str,Fid);  
			 	i = 0;
			 	while (SendDlgItemMessage (hWndDlg,IDC_SYMBOL_LIST,LB_GETTEXT,i,(LPARAM)str) != LB_ERR)
			 	{
	                SetSymParms (str,SymName,cWidth,cRot,cColor);            
	                SendDlgItemMessage (hWndDlg,IDC_SQL_LIST,LB_GETTEXT,i++,(LPARAM)cIF); 
	                if (!_fstrcmp (cIF,"ALL ROWS"))
	                	*cIF = 0; 
	                sprintf (str,"%s;%s;%s;%s;%s",SymName,cIF,cColor,cWidth,cRot);
				 	fputstring (str,Fid); 
			 	}
			 	fputstring ("#ENDOFLIST",Fid);  
		 		GetDlgItemText (hWndDlg,IDC_BEGINDATE,str,sizeof(str)-1);  
			 	fputstring (str,Fid); 
		 		GetDlgItemText (hWndDlg,IDC_ENDDATE,str,sizeof(str)-1);  
			 	fputstring (str,Fid); 
				GSSiClose (Fid);
            	if (Opened) 
            	{
					CloseDataFile (TRUE,&hSHPDBF); 
					DestroyFieldList ();
				}
	            EndDialog(hWndDlg,TRUE);  
				break;
          }
          break;

    default: 
        return FALSE;
   }
 return TRUE;    
}

BOOL LoadGMDParm (LPSTR GMDFileName,HWND hWnd)
#if ENABLETRACE
{GSSiEnterProg (1374);
#endif
{   
	char	Name[MAX_PATH], str[260], Projection[34], Units[34];
	char	SymName[66], cWidth[64],cRot[64],cColor[64], cIF[128];
	LPSTR	pDot, pTAG,pWidth, pParm=GMDParms;  
	short	l;
	HFILE	Fid; 
	BOOL	FileIsIndex;
	struct _stati64   statParmFile; 

    if (!GMDFileName)
    {   
    	HaveIndexParmFile = FALSE;  
    	goto RtnTrue;
    }
    if (HaveIndexParmFile) 
    {
    	goto RtnTrue;
    }
	strcpy (Name,GMDFileName);
	strcpy (LastGMDFile,GMDFileName);
	*GMDBeginDate = 0;
	*GMDEndDate = 0;
	GMDParmTime = 0;
	NumGMDParms = 0;  
	GMDProjectionIsBase = TRUE;
	_fmemset (GMDParms,0,sizeof(GMDParms)); 
	GetGlobalCVal ("[%DefaultGMDPointSymbol]",GMDParms,"DUMMYPT");  
	pWidth = _fstrchr (GMDParms,0)+4;
	GetGlobalCVal ("[%DefaultGMDPointSize]",pWidth,"-5"); 
	GetGlobalCVal ("[%DefaultGMDProjection]",Projection,"BASEPROJ"); 
	LoadProjection(0,Projection); 
	GetGlobalCVal ("[%DefaultGMDUnits]",Units,"METERS"); 
	if (!_fstricmp (Units,"FEET"))
		PRJ_UNITS[0] = 1;
	else if (!_fstricmp (Units,"METERS"))
		PRJ_UNITS[0] = 2;
	else
		PRJ_UNITS[0] = 4;
	ExpandText (Name);
	l = _fstrlen (Name);
	if (l > 4 && !_fstricmp (&Name[l-5],"INDEX"))
	{
		HaveIndexParmFile = FALSE; 
		FileIsIndex = TRUE; 
		pDot = &Name[l];
	}
	else  
	{
		FileIsIndex = FALSE;
        pDot = _fstrrchr (Name,'.');
	}
	if (!pDot)
		goto RtnTrue;
//	goto RtnTrue;
	_fstrcpy (pDot,".gmp");
	Fid = GSSiOpenFile (Name,0,OF_READ); 
	if (Fid == HFILE_ERROR)  
	{

		if (FileIsIndex)
			goto RtnFalse;   
		{
			DialogBox(hInst, (LPSTR)"SETGMDPARAM", hWnd, SETGMDPARAMMsgProc);
			Fid = GSSiOpenFile (Name,0,OF_READ);  
			if (Fid == HFILE_ERROR)  
	        	goto RtnFalse; 
	    }
	}
	if (FileIsIndex)
		HaveIndexParmFile = TRUE;
    GSSifstat (Fid,&statParmFile);
    GMDParmTime = statParmFile.st_mtime;
	fgetstring (Projection,32,Fid); 
	if (!*Projection)
		GetGlobalCVal ("[%DefaultGMDProjection]",Projection,"BASEPROJ"); 
	LoadProjection(0,Projection); 
	GMDProjectionIsBase = IS_BASE[0];
	fgetstring (Units,32,Fid);
	if (!*Units)
		GetGlobalCVal ("[%DefaultGMDUnits]",Units,"FEET"); 
	if (!_fstricmp (Units,"FEET"))
		PRJ_UNITS[0] = 1;
	else if (!_fstricmp (Units,"METERS"))
		PRJ_UNITS[0] = 2; 
	else
		PRJ_UNITS[0] = 4;
	fgetstring (GMDRefno,126,Fid); 
	fgetstring (GMDTAG,99,Fid); 
	_fmemset (GMDParms,0,sizeof(GMDParms));
	while (fgetstring (str,256,Fid))
	{   
		if (*str == '#')
			break;
		DecodeSHPParam (str,SymName,cIF,cColor,cWidth,cRot);
		_fstrcpy (pParm,SymName);
		l = _fstrlen (SymName);
		pParm += l+1;
		_fstrcpy (pParm,cIF);
		l = _fstrlen (cIF);
		pParm += l+1;
		_fstrcpy (pParm,cColor);
		l = _fstrlen (cColor);
		pParm += l+1;
		_fstrcpy (pParm,cWidth);
		l = _fstrlen (cWidth);
		pParm += l+1;
		_fstrcpy (pParm,cRot);
		l = _fstrlen (cRot);
		pParm += l+1;
		NumGMDParms++;
	}
	if (NumGMDParms == 1)
	{
		sprintf (GMDSymbol,"$SYMNUM(%s)",SymName);
		ExpandText (GMDSymbol);
	}
	if (fgetstring (str,256,Fid))
		_fstrcpy (GMDBeginDate,str);
	if (fgetstring (str,256,Fid))
		_fstrcpy (GMDEndDate,str);
	
	GSSiClose (Fid); 
RtnTrue:
{
#if ENABLETRACE
GSSiExitProg (1374);
#endif
	return TRUE;
}
RtnFalse:
{
#if ENABLETRACE
GSSiExitProg (1374);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
} 

void CloseGMDMapFile (void)
{
	CloseDataFile (TRUE,&GMDHandle);
	WantGMDNegGrid = FALSE;
	BT_CLOSEANDDELETE (&hDisplayedGMDRefs);
	return;
}

BOOL OpenGMDMapFile (LPSTR Name)
{
	BOOL	rtn=FALSE;
	char	FileName[MAX_PATH];
	LPSTR	pPar;

	*GMDWhere = 0;
	BT_CLOSEANDDELETE (&hDisplayedGMDRefs);
	if ((pPar = strstr (Name,".GMD(")))
	{
		pPar += 4;
		strncpy (GMDWhere,pPar,255);
		*pPar = 0;
	}
	sprintf (FileName,"GMD=%s",Name);
	CloseDataFile (TRUE,&GMDHandle);
	rtn = OpenDataFile (FileName,"",OF_READ,&GMDHandle);
	if (rtn)
	{
		LPOPENSQLDATA	SQLPtr = (LPOPENSQLDATA)GlobalLock (GMDHandle);
		BTVARDESC	BTVar[2]; 
		char		TempName[MAX_PATH]; 
		
		SQLPtr->NumGlobals = 0;
		GlobalUnlock (GMDHandle);
		BTVar[0].BT_VARLEN=4;
		BTVar[0].BT_VARTYP=BT_INTEGER;
		BTVar[0].BT_VAROFF=0;
		GSSiGetTempFileName (0,"gmd",0,(LPSTR)TempName);
		BT_CREATE (TempName, 2, FALSE, 1, 1,BTVar,FALSE, 0, 0, FALSE);
		hDisplayedGMDRefs = BT_OPEN (TempName,0, BT_WRITE, 0);
	}
	strcpy (LastGMDFile,Name);
	ExpandText (LastGMDFile); 
	GMDpos = BT_FIRST;
	GMDcond = BT_ANY;
	return rtn;
}

int MonthInDateRange (LPSHORT pMonth)
{
	int	i,j;

	if (*pMonth < MonthBeg)
	{
		*pMonth = MonthBeg;
		return 2;
	}
	if (*pMonth > MonthEnd)
		return 0;
	for (i=0;i<*pNumViewports;i++)
	{
		if (!strnicmp (pViewports[i]->Name,"Year",5))
		{
			int	yr = 1900 + *pMonth / 12;

			if (pViewports[i]->pTheme && pViewports[i]->pTheme->IsActive && pViewports[i]->pTheme->VPDisplayed)
			{ 
				for (j=0;j<pViewports[i]->pTheme->NumClass;j++)
					if (!pViewports[i]->pTheme->ClassStatus[j])
					{
						if (yr >= (pViewports[i]->pTheme->ClassMin[j] + pViewports[i]->pTheme->RefValDbl) && yr <= (pViewports[i]->pTheme->ClassMax[j] + pViewports[i]->pTheme->RefValDbl))
							goto CheckMonth;
					}
					*pMonth -= *pMonth % 12;
					*pMonth += 12;
					return 2;
			}
		}
	}
CheckMonth:
	for (i=0;i<*pNumViewports;i++)
	{
		if (!strnicmp (pViewports[i]->Name,"Month",5))
		{
			if (pViewports[i]->pTheme && pViewports[i]->pTheme->IsActive && pViewports[i]->pTheme->VPDisplayed)
			{ 
				j = *pMonth % 12;
				if (pViewports[i]->pTheme->ClassStatus[j])
				{
					(*pMonth)++;
					return 2;
				}
				return 1;
			}
		}
	}
	return 1; 
}

int GridInWBounds (LPGWDHEADER lpGWDHead,LPSHORT pGrid)
{
	int			GridRow, GridCol;
	MNMXCORD	GridMnMx;
	DPOINT		pt;

	if (WantGMDNegGrid)
	{
		if (*pGrid == -1)
			return 1;
		if (*pGrid < -1)
		{
			*pGrid = -1;
			return 2;
		}
		*pGrid = -1;
		return 0;
	}
	if (hGMDKeyList)
		return 1;
	if (!BoundsInBounds (&lpGWDHead->GridBounds,&CurView->WBounds,1))
		return 0;
	if (*pGrid < 0)
	{
		*pGrid = 0;
		return 2;
	}
	GridRow = *pGrid / lpGWDHead->GridInc;
	GridCol = *pGrid % lpGWDHead->GridInc;
	GridMnMx.ymn = GridRow * lpGWDHead->GridYInc + lpGWDHead->GridBounds.ymn;
	if (GridMnMx.ymn > CurView->WBounds.ymx)
	{
		pt.x = max (CurView->WBounds.xmn,lpGWDHead->GridBounds.xmn);
		pt.y = max (CurView->WBounds.ymn,lpGWDHead->GridBounds.ymn);
		*pGrid = GridFromPoint (lpGWDHead,(LPDPOINT)&pt);
		return 0;
	}
	GridMnMx.ymx = GridMnMx.ymn + lpGWDHead->GridYInc;
	if (GridMnMx.ymx < CurView->WBounds.ymn)
	{
		pt.x = max (CurView->WBounds.xmn,lpGWDHead->GridBounds.xmn);
		pt.y = max (CurView->WBounds.ymn,lpGWDHead->GridBounds.ymn);
		*pGrid = GridFromPoint (lpGWDHead,(LPDPOINT)&pt);
		return 2;
	}
	GridMnMx.xmn = GridCol * lpGWDHead->GridXInc + lpGWDHead->GridBounds.xmn;
	GridMnMx.xmx = GridMnMx.xmn + lpGWDHead->GridXInc;
	if (GridMnMx.xmx < CurView->WBounds.xmn)
	{
		GridMnMx.xmn = CurView->WBounds.xmn;
		*pGrid = GridFromPoint (lpGWDHead,(LPDPOINT)&GridMnMx);
		return 2;
	}
	if (GridMnMx.xmn > CurView->WBounds.xmx)
	{
		GridMnMx.ymn += lpGWDHead->GridYInc;
		GridMnMx.xmn = CurView->WBounds.xmn;
		*pGrid = GridFromPoint (lpGWDHead,(LPDPOINT)&GridMnMx);
		return 2;
	}
	return 1;
}

BOOL GMDCodeInList (int code)
{
	char	str[32];

	if (!*GMDCodeList)
		return TRUE;
	sprintf (str,"|%i|",code);
	if (strstr (GMDCodeList,str))
		return TRUE;
	return FALSE;
}

short GMDNextCodeInList (int code)
{
	int		icode, nextcode = SHRT_MAX;
	LPSTR	pLoc = strchr (GMDCodeList,'|');
	LPSTR	pEnd = pLoc;

	if (!pLoc)
		return code+1;

	while (pEnd)
	{
		pEnd = strchr (++pLoc,'|');
		if (!pEnd)
			break;
		icode = atoi (pLoc);
		if (icode > code && icode < nextcode)
			nextcode = icode;
		pLoc = pEnd++;
	}
	return nextcode;
}
int GMDFindUsingKeyList(LPGWDHEADER	lpGWDHead, LPINT pGMDpos, LPINT pGMDcond, LPLONG pOffset)
{
	int rtn;
	int dummy;

	if (hGMDKeyList)
	{
Top:
		if (BT_FIND(hGMDKeyList, lpGWDHead->pKeys[0], GMDKeyListPos, BT_ANY, (LPSTR)&dummy))
		{
			rtn = 31;
			GMDKeyListPos = BT_FIRST;
		}
		else
		{
			int ii = *(LPINT)lpGWDHead->pKeys[0];
			short iii = *(LPSHORT)(lpGWDHead->pKeys[0] + 4);
			*(LPSHORT)(lpGWDHead->pKeys[0] + 4) = 1; //temp for MP only
			rtn = BT_FIND(lpGWDHead->BTHandle[0], lpGWDHead->pKeys[0], BT_FIRST, BT_EQ, (LPSTR)pOffset);
			GMDKeyListPos = BT_NEXT;
			if (rtn)
				goto Top;
		}
	}
	else
		rtn = BT_FIND(lpGWDHead->BTHandle[lpGWDHead->SpatialIndex], lpGWDHead->pKeys[lpGWDHead->SpatialIndex], *pGMDpos, *pGMDcond, (LPSTR)pOffset);

	return rtn;
}
long GetGMDRecordOffset (long record,BOOL UseBounds)
{
	LPOPENFILEDATA	FilePtr;
	LPOPENSQLDATA	SQLPtr;
	LPGWDHEADER		lpGWDHead;
	long			Offset=-2;
	double			xval,yval,StartTime,v;
	int				i,ii;
	LPVIEWPORT		SaveVP = CurView;
	
	if (!GMDHandle)
		return -1;
	if (CurView->DisplayInParent && CurView->Parent)            	
		SetViewport(CurView->Parent);

    SQLPtr = (LPOPENSQLDATA)GlobalLock (GMDHandle);
	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
    lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle);
	if (UseBounds && lpGWDHead->SpatialIndex)
	{
		LPSPATIALINDEXTYPE1	pSIIndex1 = (LPSPATIALINDEXTYPE1)lpGWDHead->pKeys[lpGWDHead->SpatialIndex];
		LPSPATIALINDEXTYPE2	pSIIndex2 = (LPSPATIALINDEXTYPE2)lpGWDHead->pKeys[lpGWDHead->SpatialIndex];

	/*	{
			SPATIALINDEXTYPE2	SIIndex2;
			HANDLE hBT = lpGWDHead->BTHandle[lpGWDHead->SpatialIndex];//BT_OPEN ("H:\\PoliceUpdate\\save\\New Folder\\offensexy.in2",0, BT_READ, 0);
			GMDpos = BT_FIRST;
			GMDcond = BT_ANY;
			while (!BT_FIND (hBT,&SIIndex2,GMDpos,GMDcond, (LPSTR)&Offset))
			{
				int	ii;
				GMDpos = BT_NEXT;
				GMDcond = BT_ANY;
				if (SIIndex2.PrimeIndex == 2845568)
					ii=1;
				if (Offset == 528887511)
					ii=1;
			}
			//BT_CLOSE (hBT);
		}*/
		while (!GMDFindUsingKeyList(lpGWDHead, &GMDpos, &GMDcond, &Offset))
		{
			static	int	dbval=987690,dboffset=570964760;
			if (Offset == dboffset)
				ii=1;
			if (pSIIndex2->PrimeIndex == dbval)
				ii=1;
			GMDpos = BT_NEXT;
			GMDcond = BT_ANY;
			switch (lpGWDHead->SpatialIndexType)
			{
				default:
				case 1:
				{
					switch (MonthInDateRange (&pSIIndex1->Month))
					{
						case 1: //want this month

							switch (GridInWBounds (lpGWDHead,&pSIIndex1->Grid))
							{
								case 1:	//want this grid
									SQLPtr->st = 0;
									SQLPtr->lastreadtime = NextVarTime ();
									memmove (&CurrentGMDRec,pSIIndex1->PrimeIndex,4);
									break;
								case 2: //Dont want this grid but not at end of bounds
									GMDpos = BT_FIRST;
									GMDcond = BT_GE;
									memset (pSIIndex1->PrimeIndex,0,lpGWDHead->lKeys[0]);
									continue;
								case 0: //At end of grid range
									pSIIndex1->Month++;
									GMDpos = BT_FIRST;
									GMDcond = BT_GE;
									memset (pSIIndex1->PrimeIndex,0,lpGWDHead->lKeys[0]);
									continue;
							}
							break;
					
						case 2:	//Dont want this month but not at end of range
							GMDpos = BT_FIRST;
							GMDcond = BT_GE;
							pSIIndex1->Grid = GridFromPoint (lpGWDHead,(LPDPOINT)&CurView->WBounds);
							memset (pSIIndex1->PrimeIndex,0,lpGWDHead->lKeys[0]);
							continue;
						case 0: //Past end of date range
						{
							Offset = -1;
							SQLPtr->st = 31;
							break;
						}
					}
				}
				break;
				case 2:
				{
					switch (MonthInDateRange (&pSIIndex2->Month))
					{
						case 1: //want this month
							
							if (pSIIndex2->Code < GMDMinCode)
							{
								GMDpos = BT_FIRST;
								GMDcond = BT_GE;
								pSIIndex2->Code = GMDMinCode;
								pSIIndex2->Grid = GridFromPoint (lpGWDHead,(LPDPOINT)&CurView->WBounds);
								pSIIndex2->PrimeIndex = LONG_MIN;
								continue;
							}
							else if (pSIIndex2->Code > GMDMaxCode)
							{
								GMDpos = BT_FIRST;
								GMDcond = BT_GE;
								pSIIndex2->Month++;
								pSIIndex2->Code = GMDMinCode;
								pSIIndex2->Grid = GridFromPoint (lpGWDHead,(LPDPOINT)&CurView->WBounds);
								pSIIndex2->PrimeIndex = LONG_MIN;
								continue;
							}
							else if (!GMDCodeInList (pSIIndex2->Code))
							{
								GMDpos = BT_FIRST;
								GMDcond = BT_GE;
								pSIIndex2->Code = GMDNextCodeInList (pSIIndex2->Code);
								pSIIndex2->Grid = GridFromPoint (lpGWDHead,(LPDPOINT)&CurView->WBounds);
								pSIIndex2->PrimeIndex = LONG_MIN;
								continue;
							}
							switch (GridInWBounds (lpGWDHead,&pSIIndex2->Grid))
							{
								case 1:	//want this grid
									SQLPtr->st = 0;
									SQLPtr->lastreadtime = NextVarTime ();
									CurrentGMDRec = pSIIndex2->PrimeIndex;
									break;
								case 2: //Dont want this grid but not at end of bounds
									GMDpos = BT_FIRST;
									GMDcond = BT_GE;
									pSIIndex2->PrimeIndex = LONG_MIN;
									continue;
								case 0: //At end of grid range
									pSIIndex2->Code = GMDNextCodeInList (pSIIndex2->Code);
									GMDpos = BT_FIRST;
									GMDcond = BT_GE;
									pSIIndex2->PrimeIndex = LONG_MIN;
									continue;
							}
							break;
					
						case 2:	//Dont want this month but not at end of range
							GMDpos = BT_FIRST;
							GMDcond = BT_GE;
							pSIIndex2->Code = GMDMinCode;
							pSIIndex2->Grid = GridFromPoint (lpGWDHead,(LPDPOINT)&CurView->WBounds);
							pSIIndex2->PrimeIndex = LONG_MIN;
							continue;
						case 0: //Past end of date range
						{
							Offset = -1;
							SQLPtr->st = 31;
							break;
						}
					}
				}
				break;
			}
			break;

/*			GMDGetNumericKeyVal (lpGWDHead,GMDYIndex,0,&yval);
			if (yval > CurView->WBounds.ymx)
			{
				Offset = -1;
				SQLPtr->st = 31;
				break;
			}
			GMDGetNumericKeyVal (lpGWDHead,GMDYIndex,1,&xval);
			GMDGetNumericKeyVal (lpGWDHead,GMDYIndex,2,&StartTime);
			if (xval >= CurView->WBounds.xmn && xval <= CurView->WBounds.xmx && StartTime >= TimeRangeBeg && StartTime < TimeRangeEnd)
			{
				SQLPtr->st = 0;
				SQLPtr->lastreadtime = NextVarTime ();
				memmove (&CurrentGMDRec,&lpGWDHead->GWDData,4);
				break;
			}*/
		}
	}
	else if (!UseBounds)
	{
		Offset = record;
		SQLPtr->st = 0;
		SQLPtr->lastreadtime = NextVarTime ();
	}
	SQLPtr->Offset = CurrentGMDRec = Offset;
	FillGWDData (lpGWDHead,Offset);
	GlobalUnlock (FilePtr->FileHandle);
	GlobalUnlock (SQLPtr->OFHandle); 
	GlobalUnlock (GMDHandle);
	CurView = SaveVP;
	return Offset;
}

BOOL ReadGMDRecordHeader (HFILE FidORA,long RecordOffset,LPMNMXCORD pMinMaxCoord)
{
	return TRUE;
}

BOOL IsGMDFileVisible (void)
{
	char	str[128]; 

	strcpy (str,GMDSymbol);
	ExpandText (str);
	CurrentDesc = atol (str);
	return (GetVisibility (CurrentDesc));
}

BOOL SetGMDVis (HWND hWndDlg, int DlgItemSym, int DlgItemPar,HFILE FidSymList)
{
	char	str[128]; 
	short	idesc;
	
	strcpy (str,GMDSymbol);
	ExpandText (str);
	idesc = atol (str);
	AddSymToList (hWndDlg,DlgItemSym,DlgItemPar,idesc,FidSymList);
	return TRUE;
}

BOOL SetGMDParms (void)
{   
	LPSTR	pDesc, pTAG, pClause, pC, pColor, pWidth, pRot; 
	BOOL	rc;
	char	str[1024];  
	BOOL	rtn = FALSE;
	
	strcpy (str,GMDRefno);
	ExpandText (str);
	CurrentRefno = atol (str);
	if (!GMDHandle)
		return FALSE; 
	SetUseOnlyOneDBHandle (hSHPDBF);
	pTAG = GMDTAG;
	if (*pTAG && (pC = _fstrchr (pTAG,':')))
	{   
		_fstrcpy (GMDTag,pTAG); 
		ExpandText (GMDTag);
		pC = _fstrchr (GMDTag,':');
		*pC++ = 0;
		strncpy0 (CurrentPrefix,GMDTag,MAX_PREFIX_LEN);
		strncpy0 (CurrentUDI,pC--,MAX_UDI_LEN);
		*pC = ':'; 
		ExpandText (CurrentUDI);
	}
	else
	{   
		*GMDTag = 0;
		*CurrentPrefix = 0;
		*CurrentUDI = 0;
	} 
	if (HaveGMDSym < 0)
	{
		LPOPENSQLDATA	SQLPtr = (LPOPENSQLDATA)GlobalLock (GMDHandle);
		LPOPENFILEDATA	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 

		pDesc = GMDParms;
		while (*pDesc)
		{
			pClause = _fstrchr (pDesc,0) + 1;
			pColor = _fstrchr (pClause,0) + 1;
			pWidth = _fstrchr (pColor,0) + 1;
			pRot = _fstrchr (pWidth,0) + 1;
	    	ConvertSQLToLogicP (str,pClause); 
			if (!*pClause || LogicPFile (SQLPtr,str,&rc))
			{
				break;
			}
			else
				pDesc = _fstrchr (pRot,0) + 1;
		}
		_fstrcpy (str,pDesc);
		ExpandText (str);
		CurrentDesc = GetDictSymbolNumber (str); 
		if (!*pDesc)
			goto Exit;
		if (*pColor) 
		{
			_fstrcpy (str,pColor);
			ExpandText (str);
			GMDColor = ConvertColor (atol (str),CurrentDesc);
		}
		else
			GMDColor = -1;
		if (*pWidth) 
		{
			_fstrcpy (str,pWidth);
			ExpandText (str);
			GMDPointSize = atol (str);
			switch (*LastChr (str)) 
			{
				case 'P':
				case 'p':
					GMDPointSize = -GMDPointSize;
					break;
				case 'F':
				case 'f':
					GMDPointSize *= FTM;
					break;
			}
			CurPointSize = GMDPointSize;
		}
		else
			GMDPointSize = 0;
		GlobalUnlock (SQLPtr->OFHandle); 
		GlobalUnlock (GMDHandle);
	}
	else
		CurrentDesc = HaveGMDSym; 
	if (*GMDBeginDate)
	{
		_fstrcpy (str,GMDBeginDate);
		ExpandText (str);
		GRStartTime  = GREndTime  = atol(str);
	}
	if (*GMDEndDate)
	{
		_fstrcpy (str,GMDEndDate);
		ExpandText (str);
		GREndTime  = atol(str);
	}
	rtn = TRUE;   
Exit: 
	SetUseOnlyOneDBHandle (0);
	return rtn;
}

BOOL ProcessGMDRecord (HDC hDC, HANDLE hDB,long Offset)
{   
	LPOPENFILEDATA	FilePtr;
	LPOPENSQLDATA	SQLPtr;
	LPGWDHEADER lpGWDHead;
	BOOL	rtn=FALSE; 
	char	str[128], Prefix[10],UDI[64];
	long	Refno;
	short	Symnum; 
	int		st;
	LPVIEWPORT	SaveVP=CurView;
	MNMXCORD	RecordBounds;
				
//	if (CurView->DisplayInParent && CurView->Parent)            	
//		SetViewport(CurView->Parent);
	if (CurView->PassID == 2 || !hDB)
		goto RtnFalse;
	InitRecord (hDC);
    SetGMDParms ();  
	strcpy (str,GMDSymbol);
	ExpandText (str);
	CurrentDesc = atol (str);
	if (!GetVisibility (CurrentDesc))
		goto RtnFalse;;
	if (*GMDWhere)
	{
		BOOL irc;
			
		if (!LogicP (GMDWhere,&irc))
			goto RtnFalse;
	}
    SQLPtr = (LPOPENSQLDATA)GlobalLock (GMDHandle);
	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
    lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle);
	FillGWDData (lpGWDHead,Offset);
	switch (lpGWDHead->SpatialIndexType)
	{
		case 1:
		case 2:
			GRStartTime = GMDGetIntegerFieldVal (lpGWDHead,lpGWDHead->FromDateField);
			GREndTime	 = GMDGetIntegerFieldVal (lpGWDHead,lpGWDHead->ToDateField);
			GMDPoint.x = GMDGetRealFieldVal (lpGWDHead,lpGWDHead->XField);
			GMDPoint.y = GMDGetRealFieldVal (lpGWDHead,lpGWDHead->YField);
			break;
	}					
	GlobalUnlock (FilePtr->FileHandle);
	GlobalUnlock (SQLPtr->OFHandle); 
	GlobalUnlock (GMDHandle);
	if (WantGMDNegGrid)
	{
		if (!GMDPoint.x)
		{
			SelectClipRgn(CurView->hDC, 0);
			GMDPoint = SubVPMidPointWorld;
		}
		else
			goto RtnFalse;
	}
	ConvertCoord(&GMDPoint,0,1);
	InGraphicsProcessor = TRUE;
	ShowValue (hDC,FALSE);  
	strcpy (str,GMDRefno);
	ExpandText (str);
	CurrentRefno = atol (str); 
 	PTRot = 0;
	if ((WantGMDNegGrid || PointInWBounds (&GMDPoint)) && GRStartTime >= TimeRangeBeg && GRStartTime < TimeRangeEnd)
 	{   
 		char	Tag[80];
 		short	ltag;
		short	Dummy;
 					
		strcpy (Tag,GMDTAG);
		ExpandText (Tag);
		SetSymNum (CurrentDesc);
		ltag = _fstrlen (Tag);
		if (ProcessRefAndTAG (TRUE,Tag,ltag))
		{  
			HiPrecis = TRUE;
			lpDCurPoints = &GMDPoint;  
			CurrentPoint = CurPointLocD = GMDPoint;
			ItemSeg = CurrentGMDRec;
			LastElementBeginPoint = LastElementEndPoint = GMDPoint;
			nPnts = nCurPoints = 1;
			CurPointLoc = BasePtToWinPt(lpDCurPoints); 
			if (!WantGMDNegGrid && !PointInMaskAreaWinCoordD (&CurPointLocD))
				goto RtnFalse;
			if (PointIsBlocked (&CurPointLocD,CurrentDesc))
			    goto RtnFalse;
		    InGraphicsProcessor = TRUE;
			HaveTXLoc = TRUE;
			CurrentType = GF_POINT; 
			if (CurPointSize < 0)
				CurPointSize = -CurPointSize * DeviceToScreenFactor;
			else
				CurPointSize /= CurView->BaseUnitsPerPixel;
			CurPointSize *= GraphicsPointFactor;
			if ((Pick||PickingByRefno) && GetTypeVisibility(TYPE_POINT))
			{
				PickPointItemD (lpDCurPoints,(CurPointSize*ThemeWidthFactor)*CurView->BaseUnitsPerPixel,PTRot,CurrentDesc);			 		    
			} 
			else if (GetTypeVisibility(TYPE_POINT))
			{   
				short	iDesc=CurrentDesc;
				
				if (CurrentDesc > 0 && CurrentDesc < 3201)	
				{	
					if (TSize)
						CurView->CurVisType[CurrentDesc]=5;
					else
						CurView->CurVisType[CurrentDesc]=4;  
				}
				HighlightPointSym=FALSE;
				if (!GetTypeVisibility(6) && SymbolIsVisible (iDesc)) 
				{
					CurPointSize = 10*DeviceToScreenFactor;
					iDesc = InvisiblePointSymbol;
				}
				if (BT_FIND (hDisplayedGMDRefs,(LPSTR)&CurrentRefno,BT_FIRST,BT_EQ,(LPSTR)&Dummy) &&
					(SetDisplayChar (CurView->hDC,GF_POINT,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI) > 0))
				{   
					double	size;
					
					BT_PUT (hDisplayedGMDRefs,(LPSTR)&CurrentRefno,(LPSTR)&CurrentDesc);
					if (ThemePointSym)
					{
						iDesc = ThemePointSym;
						if (ThemePointSize < 0)
							size = -ThemePointSize *DeviceToScreenFactor;
						else
							size = ThemePointSize / CurView->BaseUnitsPerPixel; 
						size *= ThemeWidthFactor;
						size = min(max (size*GraphicsPointFactor,1),MaxPointSize);
					}
					else if (ItemSymbolWidth > 0)
						size = ItemSymbolWidth * CurPointSize*ThemeWidthFactor*GraphicsPointFactor;
					else if (ItemSymbolWidth < 0)
						size = -ItemSymbolWidth * BaseDistToWinDist * CurPointSize*ThemeWidthFactor*GraphicsPointFactor;
					else
						size = CurPointSize*ThemeWidthFactor*GraphicsPointFactor;
					if (iDesc < 0) 
					{   
						COLORREF	OldColor;
						
						if (ThemePointColor > -1)
							OldColor = 	SetTextColor (CurView->hDC,ConvertColor(ThemePointColor,ThemePointUseHalfTone));
						DisplayCharAtLoc (CurView->hDC,CurPointLoc,(short)IDNINT(size),-iDesc); 
						if (ThemePointColor > -1)
							SetTextColor (CurView->hDC,OldColor);
					}
					else
					{
						long	DisplayedWidth=0;

						DisplayPointItem (CurView->hDC,CurPointLoc,size,PTRot,iDesc,&DisplayedWidth);
						CurView->MaxSymbolWidth = max (CurView->MaxSymbolWidth,DisplayedWidth);
						CurView->MaxFileDisplayedPointWidth[FileNum] = max (CurView->MaxFileDisplayedPointWidth[FileNum],(DisplayedWidth/ FileDistToWinDist)-(((long)CurrentItemMinMax.xmx)-CurrentItemMinMax.xmn));
					}
					DBoundsInit (&RecordBounds);
					AddDPointToMinMax (lpDCurPoints,&RecordBounds);
					InflateBounds (&RecordBounds,size);
					GetFileMinMax (&CurrentItemMinMax,&RecordBounds); 
				} 
			}
			InGraphicsProcessor = FALSE;
			TXLoc = CurPointLocD; 
			HaveTXLoc = 1;  
		}
	}
	ShowValue (hDC,FALSE);  
RtnFalse:
	CurView = SaveVP;
	InGraphicsProcessor = FALSE;
	return rtn;
}

long ReadGMDHeader (HFILE FidORA,LPMNMXCORD pMinMaxCoord)
{
	LPOPENFILEDATA	FilePtr;
	LPOPENSQLDATA	SQLPtr;
	LPGWDHEADER		lpGWDHead;
	long			Offset;
	double			rval;
	char			str[128];
	
	if (!GMDHandle)
		return FALSE;
	NumGMDRecs = NumSQLRows (GMDHandle); 
	if (!NumGMDRecs)
		return FALSE;
    SQLPtr = (LPOPENSQLDATA)GlobalLock (GMDHandle);
	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
    lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle);
	switch (lpGWDHead->SpatialIndexType)
	{
		case 1:
		case 2:
			*pMinMaxCoord = lpGWDHead->FileBounds;
			break;
	}

	if (pMinMaxCoord)
		ConvertRectCoord (&GMDFileBounds,pMinMaxCoord,0,1);
	GMDpos = BT_FIRST;
	GMDcond = BT_GE;
	GWDInitKeyValues (lpGWDHead,lpGWDHead->SpatialIndex);
   	GlobalUnlock (FilePtr->FileHandle);
   	GlobalUnlock (SQLPtr->OFHandle);
	GlobalUnlock (GMDHandle);
	return TRUE;
}  

BOOL ExpandGMDPointBounds (LPMNMXCORD pBounds)
{
	char	str[256];
	short	RectMax; 
	double	symsize;
	RECT	SymRect;
	LPOPENFILEDATA	FilePtr;
	LPOPENSQLDATA	SQLPtr;
	LPGWDHEADER		lpGWDHead;
	int		FromTime, ToTime;
	DPOINT	WPoint;
	static MNMXCORD ProjectBounds = { 0 };;
	static BOOL first = TRUE;

	if (first)
	{
		BOOL Err;
		first = FALSE;
		if (GetGlobalCVal("[%PROJECTBOUNDS]", str, 0))
		{
			ProjectBounds = atobounds(str, &Err);
		}
	}


    SQLPtr = (LPOPENSQLDATA)GlobalLock (GMDHandle);
	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
    lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle);
	switch (lpGWDHead->SpatialIndexType)
	{
		case 1:
		case 2:
			FromTime = GMDGetIntegerFieldVal (lpGWDHead,lpGWDHead->FromDateField);
			ToTime	 = GMDGetIntegerFieldVal (lpGWDHead,lpGWDHead->ToDateField);
			GMDPoint.x = GMDGetRealFieldVal (lpGWDHead,lpGWDHead->XField);
			GMDPoint.y = GMDGetRealFieldVal (lpGWDHead,lpGWDHead->YField);
			break;
	}
	if (WantGMDNegGrid && !PointInBounds(GMDPoint,&ProjectBounds))
		GMDPoint = SubVPMidPointWorld;
	ConvertCoord(&GMDPoint,0,1);
	strcpy (str,GMDSymbol);
	ExpandText (str);
	CurrentDesc = atol (str); 
	pBounds->xmn = pBounds->xmx = GMDPoint.x; 
	pBounds->ymn = pBounds->ymx = GMDPoint.y; 
	SymRect=GetSymRect(CurrentDesc);

	RectMax = max ((long)SymRect.right - (long)SymRect.left,(long)SymRect.bottom - (long)SymRect.top);   
	symsize = GMDPointSize*((double)RectMax)/200; 
	ExpandBounds (pBounds,symsize/3);
	GlobalUnlock (FilePtr->FileHandle);
	GlobalUnlock (SQLPtr->OFHandle); 
	GlobalUnlock (GMDHandle);
	return TRUE;
}

void BuildTAGIndex (LPSTR Prefix, LPSTR UDI, int len, long Refno,BOOL Deleted)
#if ENABLETRACE
{GSSiEnterProg (1020);
#endif
{   
	REFINDEXDATA SaveRID;
	REFINDEXDATA	RefIdxData;

    if (!Prefix || !*Prefix || !_fstricmp (Prefix,"REFNO"))
		goto Exit;
    if (!len)
    	len = 33;
	if (ForceTAGIndex)
	{	
		char str[34];
		short	keylen = GetBTKeyLen (hTAGIdx);
		  
		len = min (len,33);
		_fstrncpy (str,UDI,len);
		str[len]=0;
		RefIdxData.FileInIndex = FileInIndex; 
		if (Deleted) //can be 1 or 2 
			RefIdxData.Deleted = 1;
		else
			RefIdxData.Deleted = 0;
		
		RefIdxData.Segment = ItemSeg;
		RefIdxData.Offset = CurrentItem;
		RefIdxData.MinMax = *(LPRECT16)&CurrentItemMinMax;    
		_fstrncpy(TAGKey.PREFIX,Prefix,8); 
		_fstrncpy(TAGKey.UDI,str,32); 
		TAGKey.Refno = Refno;  
/*		SaveRID = RefIdxData;
		while (!BT_FIND (hTAGIdx,(LPSTR)&TAGKey,BT_FIRST,BT_EQ,(LPSTR)&SaveRID))
		{
			if (keylen < 44)
				break; 
			TAGKey.Sequence++;
		}*/ //sizeof(REFINDEXDATA)
		BT_PUT (hTAGIdx,(LPSTR)&TAGKey,(LPSTR)&RefIdxData);
	}
Exit:
{
#if ENABLETRACE
GSSiExitProg (1020);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

int AddAdditionalUDI (LPSTR IndexPath,LPSTR FileID,LPSTR Prefix, LPSTR OldUDIvar, LPSTR NewUDIvar)
{
	REFINDEXDATA	RefIdxData;
	short	len=32;
	long	Refno;
	int		NumAdded=0;

	CloseTAGIndex ();
	if ((hTAGIdx = BT_OPEN (IndexPath, 0, BT_WRITE, 0)))
	{	
		char	str[34], FetchStr[64], OldUDIstr[66],NewUDIstr[66];
		short	rc,keylen = GetBTKeyLen (hTAGIdx);
		int		tot, loc=0;
		
		sprintf (FetchStr,"$NUMROWS(%s)",FileID);
		ExpandText (FetchStr);
		tot = atol (FetchStr);
		CreateStatusWind (hWndMain,1,"Adding additional TAGs");
		if (!hTAGIdx)
			hTAGIdx = BT_OPEN (IndexPath, 0, BT_WRITE, 0);
		sprintf (FetchStr,"$FETCH(%s)",FileID);
		ExpandText (FetchStr);
		rc = atoi (FetchStr);
		while (rc)
		{
			strncpy(TAGKey.PREFIX,Prefix,8); 
			sprintf (OldUDIstr,"[%s.%s]",FileID,OldUDIvar);
			ExpandText (OldUDIstr);
			strncpy(TAGKey.UDI,OldUDIstr,32); 
			TAGKey.Refno = LONG_MIN;  
			*NewUDIstr=0;
			if (!BT_FIND (hTAGIdx,(LPSTR)&TAGKey,BT_FIRST,BT_GE,(LPSTR)&RefIdxData))
			{
				if (!strcmp (TAGKey.UDI,OldUDIstr))
				{
					sprintf (NewUDIstr,"[%s.%s]",FileID,NewUDIvar);
					ExpandText (NewUDIstr);
					strncpy(TAGKey.UDI,NewUDIstr,32); 
					BT_PUT (hTAGIdx,(LPSTR)&TAGKey,(LPSTR)&RefIdxData);
					NumAdded++;
				}
			}
			sprintf (FetchStr,"$FETCH(%s)",FileID);
			ExpandText (FetchStr);
			rc = atoi (FetchStr);
			StatusWindowUpdate (0,NewUDIstr,tot,loc++);
			if (!ContinueProcessing)
			{
				rc = 0;
				ContinueProcessing = TRUE;
			}
		}
		CloseTAGIndex ();
		DestroyStatusWindow(0);  
	}
	return NumAdded;
} 

BOOL DeleteFromTAGList (LPSTR Prefix,LPSTR UDI,long Refno)
#if ENABLETRACE
{GSSiEnterProg (1021);
#endif
{   
	REFINDEXDATA	RefIdxData;

	if (!Prefix)
{
#if ENABLETRACE
GSSiExitProg (1021);
#endif
		return FALSE;
}
    if (!*Prefix)
{
#if ENABLETRACE
GSSiExitProg (1021);
#endif
    	return FALSE;
}
    if (hTAGIdx)
    { 
		char str[34];
		short	keylen = GetBTKeyLen (hTAGIdx);
		  
		_fstrncpy (str,UDI,33);
		str[33]=0;
		_fstrncpy(TAGKey.PREFIX,Prefix,8); 
		_fstrncpy(TAGKey.UDI,str,32); 
		TAGKey.Refno = Refno;  
		if (!BT_FIND (hTAGIdx,(LPSTR)&TAGKey,BT_FIRST,BT_EQ,(LPSTR)&RefIdxData))
		{
			BT_DELETE (hTAGIdx,(LPSTR)&TAGKey,(LPSTR)&RefIdxData,FALSE);
{
#if ENABLETRACE
GSSiExitProg (1021);
#endif
			return TRUE;
}
		}
	}
{
#if ENABLETRACE
GSSiExitProg (1021);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
} 


void BuildRefIndex (BOOL DeleteExistingRef,BOOL DeletedItem)
#if ENABLETRACE
{GSSiEnterProg (1022);
#endif
{   int ii;
	REFINDEXDATA	RefIdxData, RefIdxDataCur; 
	PICKDATA		SavePick0=PickList[0];
    
    if (CurrentRefno > LONG_MAX-1000) //fixes problem in Kaufman where single maxrefno value causes fix dup ref to start at LONG_MIN
{
#if ENABLETRACE
GSSiExitProg (1022);
#endif
    	return;
}
	if (ForceRefIndex)
	{	
		if (DeleteExistingRef)
		{ 
			if (!BT_FIND (hRefIdx,(LPSTR)&CurrentRefno,BT_FIRST,BT_EQ,(LPSTR)&RefIdxData))
			{
				PickList[0].Segment = RefIdxData.Segment;  
				PickList[0].Offset = RefIdxData.Offset;  
				PickList[0].FileInIndex = RefIdxData.FileInIndex;  
				PickList[0].SubFile = 0;
				PickList[0].ViewID = 0;
				PickList[0].ConfigID = CurrentConfig;
				*PickList[0].Prefix = 0;
				DeletePickedItem (0,12,92);
			}
		}
		RefIdxData.FileInIndex = FileInIndex;
		RefIdxData.Deleted = DeletedItem;
		RefIdxData.Segment = ItemSeg;
		RefIdxData.Offset = CurrentItem;
		RefIdxData.MinMax = *(LPRECT16)&CurrentItemMinMax;    
//		if (!BT_FIND (hRefIdx,(LPSTR)&CurrentRefno,BT_FIRST,BT_EQ,(LPSTR)&RefIdxDataCur))
//			ii=1;
		if (!BT_PUT (hRefIdx,(LPSTR)&CurrentRefno,(LPSTR)&RefIdxData) && !DeletedItem)  
		{   
//			if (!RefIdxDataCur.Deleted)
//			{
//				AddToDupRefList (CurrentRefno,&RefIdxDataCur,TRUE);
//				AddToDupRefList (CurrentRefno,&RefIdxData,FALSE);
//			}
		}
	} 
	PickList[0] = SavePick0;
{
#if ENABLETRACE
GSSiExitProg (1022);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

HANDLE CreateDupRefTable (void)
#if ENABLETRACE
{GSSiEnterProg (1023);
#endif
{
	BTVARDESC	BTVar[2];
	char	Name[256], Name2[256]="";
	HANDLE	handle;  
	LPSTR	lpDot, pDupFiles;
	
	BT_GETPATHNAME (hRefIdx,Name);
    lpDot = _fstrrchr (Name,'.');
    if (lpDot)
    	_fstrcpy (lpDot,".dup");  
    else
{
#if ENABLETRACE
GSSiExitProg (1023);
#endif
    	return 0;  
}
    if (hDupRef)
   		BT_GETPATHNAME (hDupRef,Name2);
   	if (!_fstricmp (Name,Name2))
{
#if ENABLETRACE
GSSiExitProg (1023);
#endif
   		return hDupRef;
}
   	BT_CLOSE (hDupRef); 
   	hDupRef = 0;
   	pDupFiles = GlobalLock (hDupFiles);
   	while (*pDupFiles)
   	{
   		if (!_fstricmp (pDupFiles,Name))
   		{
			handle = BT_OPEN (Name, 0, BT_WRITE, 0);    
			GlobalUnlock (hDupFiles);
{
#if ENABLETRACE
GSSiExitProg (1023);
#endif
   			return handle;
}
   		} 
   		pDupFiles = _fstrchr (pDupFiles,0);
   		pDupFiles++;
   	} 
   	_fstrcpy (pDupFiles,Name);
	GlobalUnlock (hDupFiles);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=2;
	BTVar[1].BT_VAROFF=0;
	BT_CREATE (Name, sizeof(DUPREFDATA), FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	handle = BT_OPEN (Name, 0, BT_WRITE, 0);
{
#if ENABLETRACE
GSSiExitProg (1023);
#endif
	return handle;
}
#if ENABLETRACE
}
#endif
}


BOOL AddToDupRefList (long Refno)
#if ENABLETRACE
{GSSiEnterProg (1024);
#endif
{   
	short	st, Dummy=0;
	DUPREFDATA  DupRefDat, OldDRD; 
	DUPREFKEY	DupRefKey; 
	
	if (!hDupRef)
{
#if ENABLETRACE
GSSiExitProg (1024);
#endif
		return TRUE;	
}
//	BT_GETDUPDATA ((LPSTR)&DupRefDat,sizeof(REFINDEXDATA)); // REFINDEXDATA should be same as first part of DUPREFDATA  
	DupRefDat.FileNum = FileNum;
	DupRefDat.SubFile = SubFile;
	DupRefDat.Desc = CurrentDesc;
	DupRefDat.FileInIndex = FileInIndex;
	DupRefDat.Segment = CurrentSeg;
	DupRefDat.Offset = CurrentItem;
	DupRefKey.Refno = Refno+1;
	DupRefKey.dupnum = 0; 
	
	if ((st = BT_FIND (hDupRef,(LPSTR)&DupRefKey,BT_FIRST,BT_GT,(LPSTR)&OldDRD)))
		st = BT_FIND (hDupRef,(LPSTR)&DupRefKey,BT_LAST,BT_ANY,(LPSTR)&OldDRD);
	else
		st = BT_FIND (hDupRef,(LPSTR)&DupRefKey,BT_PRIOR,BT_ANY,(LPSTR)&OldDRD);
	if (!st && DupRefKey.Refno == Refno)
	{
		BT_PUT (hDupRef2,(LPSTR)&Refno,(LPSTR)&Dummy);
		DupRefKey.dupnum++; 
	}
	else
	{
		DupRefKey.Refno = Refno;
		DupRefKey.dupnum = 1; 
	}
	BT_PUT (hDupRef,(LPSTR)&DupRefKey,(LPSTR)&DupRefDat);
	FastMapCopynRecs++;
	if (!(FastMapCopynRecs % 100))
	{
		char	mess[64];

		sprintf (mess,"%i records processed",FastMapCopynRecs);
		//SetWindowText (CurView->hWnd,mess);
		SetSysMess (mess);
		SetPrompt2 (0,PRMT_SYSMESS);
	}
{
#if ENABLETRACE
GSSiExitProg (1024);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}

BOOL FixDupRef (void)
#if ENABLETRACE
{GSSiEnterProg (1025);
#endif
{
	BOOL	rtn=FALSE, SaveFRI=ForceRefIndex;      
/*	long	NumDup = BT_NUM_IN_INDEX (hDupRef);
	char	mess[256], Name[256];    
	long	MaxRef, Refno, SaveFPFN, SaveFPFII;
	short	pos = BT_FIRST, PickFile; 
	HANDLE	hBTDup=hDupRef,hSaveVP=GSSiGlobAlloc ( 962,GMEM_MOVEABLE,sizeof(VIEWPORT));
	LPVIEWPORT	pSaveVP = CurView, SaveVP=(LPVIEWPORT)GlobalLock (hSaveVP);
	LPSTR	lpDot;
	REFINDEXDATA	RefIdxData;
	
 	hDupRef = 0; // stops reiterative calls    
 	InFixDupRef = TRUE;
	ForceRefIndex =  FALSE; 
	SetViewport(*pCommandViewport);
	*SaveVP = *CurView;
	sprintf (mess,"There are %ld items with duplicate reference numbers. \r\nDo you wish to fix them?",NumDup);
 	if (MessageBox (GetFocus(),mess,"Duplicate reference numbers found",MB_YESNO) == IDNO) 
	{
		char	DefStr[]="Refno(B4),Sequence(B2),SymName(C64),File(C256)"; 
		char	SymName[66];
		HANDLE	hDB=0;
		LPGWDHEADER lpGWDHead;
		int		nmiss=0;

	 	if (MessageBox (GetFocus(),"Do you wish to create a duplicate reference database?","",MB_YESNO) == IDNO) 
	 		goto Exit;
		strcpy (Name,"[%DL]attribut\\duprefs.gmd");
		if (!CreateGWDDatabase (Name,1,FALSE,0,2,DefStr))
			goto Exit;
		hDB = OpenGWDatabase (Name,BT_WRITE);
	    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
		while (!BT_FIND (hBTDup,(LPSTR)&DupRefKey,pos,BT_ANY,(LPSTR)&DupRefDat))
		{
			pos = BT_NEXT;
    		FastPick = TRUE; 
			PickList[0].FileNum = DupRefDat.FileNum;
			PickList[0].SubFile = DupRefDat.SubFile;
		 	SetFieldValFromCharAndName(lpGWDHead,"Refno",(LPSTR)&DupRefKey.Refno,TRUE);
		 	SetFieldValFromCharAndName(lpGWDHead,"Sequence",(LPSTR)&DupRefKey.dupnum,TRUE);
			GetDictSymName (DupRefDat.Desc,SymName);
		 	SetFieldValFromCharAndName(lpGWDHead,"SymName",SymName,FALSE);
			GetPickName (0);
 		 	SetFieldValFromCharAndName(lpGWDHead,"File",PickName,FALSE);
			GWDAddRecord (lpGWDHead,0,0);
		}
		GlobalUnlock (hDB);  
		CloseGWDatabase (hDB); 
		sprintf (mess,"%i",nmiss);
		MessageBox (0,mess,"",MB_OK);
		goto Exit; 
	}
	BT_GETPATHNAME (hBTDup,Name);
	lpDot = _fstrchr (Name,'.');
	_fstrcpy (lpDot,".rin"); 
	hRefIdx = BT_OPEN (Name, 0, BT_WRITE, 0);
	BT_FIND (hRefIdx,(LPSTR)&Refno,BT_LAST,BT_ANY,(LPSTR)&RefIdxData);
	SetGlobalValueLong ("%INITIALREFNO",Refno); 
	MaxRef = GetNewRefno(Name,0,0,0,0); 
	MaxRef = max (MaxRef,Refno); 
	SetNewRefno (Name,MaxRef);
	while (!BT_FIND (hBTDup,(LPSTR)&DupRefKey,pos,BT_ANY,(LPSTR)&DupRefDat))
	{
		Refno = GetNewRefno(Name,0,0,0,0); 
		pos = BT_NEXT;
    	PickFile = DupRefDat.FileNum + CurView->ID*256;    
    	SaveFPFN = FastPickFileNum;
    	SaveFPFII = FastPickFII;
		FastPickFileNum = FileNum; 
		FastPickFII = 0;//DupRefDat.FileInIndex;
    	FastPick = TRUE; 
    	if (PickByRefno (DupRefKey.Refno,"%DUPREF%",(LPSTR)&DupRefDat,PickFile))
    	{
			FastPick = FALSE; 
			FastPickFII = SaveFPFII;
			FastPickFileNum = SaveFPFN;
			ChangePickedItemRefno (0, Refno);
			BT_PUT (hRefIdx,(LPSTR)&Refno,(LPSTR)&DupRefDat);
		}
	}   
	BT_CLOSE (hRefIdx);
	hRefIdx = 0;
	rtn = TRUE;
Exit:   
	FastPick = FALSE;
	BT_CLOSEANDDELETE (&hBTDup);  
	ForceRefIndex = SaveFRI;
	InFixDupRef = FALSE;    
	SetCurView ( pSaveVP);
	*CurView = *SaveVP;
	GSSiGlobUlFree (&hSaveVP);
	*/
{
#if ENABLETRACE
GSSiExitProg (1025);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

long PickByTagOrRefno (LPSTR TagOrRefno,short PickFile)
{
	LPSTR	lpColon=0;
	long	Refno=0;
	char	TorR[128];
	LPSTR	pTag = TorR;

	strcpy (TorR,TagOrRefno);
	
	if ((lpColon = _fstrchr (TorR,':')))
		*lpColon++=0;
	else 
	{
		Refno = atol (TorR);
		pTag = 0;
	}
	return (PickByRefno (Refno,pTag,lpColon,-1));
}

long PickByRefno (long Refno,LPSTR InPrefix, LPSTR InUDI,short PickFile)
#if ENABLETRACE
{GSSiEnterProg (1027);
#endif
{   int	st, iview;
	BOOL RIOpened, SavePick,PrefixIsDupref;
	long	rtn=0, nPnts;
	HANDLE	hPoly;
	LPVIEWPORT	SaveView;
	LPVISLIST	SaveVis; 
	HANDLE		hVisList=0; 
	short	WantView;
	long	FileListLoc=0;
	BOOL	SaveUseRORTI = UseRefOrTAGIndex;    
	long	Sequence=LONG_MIN;
	LPSTR	pBrace;   
	HFILE	FidFL; 
	HANDLE	hMem=GSSiGlobAlloc ( 963,GMEM_MOVEABLE,2*sizeof(REFINDEXDATA)+64);
	LPSTR	UDI=GlobalLock (hMem);
	LPREFINDEXDATA	pRefIdxData=(LPREFINDEXDATA)(UDI+64);
	LPREFINDEXDATA	pRefIdxDataTest=pRefIdxData+1;
    LPSTR	Prefix=InPrefix;  
    BOOL	SaveWDB = WantDescBlock; 
    BOOL	CheckForLargestPiece;  
    PICKDATA	PD;
    char	SavePltName[MAX_PATH];
	int		SavePltType = PltType;

	strcpy (SavePltName,PltName);
    WantDescBlock = FALSE;
    if (Prefix && !_fstricmp (Prefix,"REFNO")) 
    {
    	Prefix = 0;
    	Refno = atol (InUDI);
    } 
    SaveView = CurView;
    SaveVis = CurVis;
    SubFile = 0;
    NumPicked = 0; 
	if (PickFile >= -100) 
	{
		hVisList=GSSiGlobAlloc ( 964,GHND,sizeof(VISLIST));
		CurVis = (LPVISLIST)GlobalLock (hVisList);
		CurVis->hVisList=hVisList;
		InitVis ();
	}
	else
		SelectVisList (TRUE); 
	
	if (!FastPick)
	{
    	CloseMap(FALSE);
    	CloseRefIndex (FALSE); 
 	}
	UseRefOrTAGIndex=TRUE; 
	if (PickFile <= -100)
		WantView = CurView->ID;
	else if (PickFile < 0)
		WantView = 0;
	else
	{
		WantView = PickFile / 256;
		PickFile = PickFile%256;
	}
	for (iview=0;iview<*pNumViewports;iview++)
	{   
    	SetCurView ( pViewports[iview]);
    	if (CurView)
    	{   
    		if (!WantView || CurView->ID == WantView)   
		    for (FileNum=0;FileNum<CurView->NumFiles;FileNum++)
		    {    
	    	 	 if (PickFile >= 0 && FileNum != PickFile)
	    	 	 	goto NextFile; 
	    	 	 if (!CurVis->FileIsVisible[FileNum])
	    	 	 	goto NextFile;
		    	 CurView->CurFile = FileNum;
		    	 PltType = CurView->FileType[CurView->CurFile]; 
		    	 if (PltType == 3 || PltType == 6)
		    	 	goto NextFile;
				 if (PltType<4)
				     _fstrcpy (PltName,CurView->lpFiles[CurView->CurFile]);
		         else if (PltType == 4) 
		         {
		         	if (_fstrstr(CurView->lpFiles[CurView->CurFile],"FILELIST.TXT")) 
		         	{
		         		FidFL = GSSiOpenFile (CurView->lpFiles[CurView->CurFile],0,OF_READ);  
		         		SubFile = 0;
		         		if (FidFL == HFILE_ERROR)
		         			goto NextFile;  
NextFileInList:    		GSSillseek (FidFL,FileListLoc,0);
						SubFile++;
		         		if (fgetstring (PltName,MAX_PATH-1,FidFL)) 
						{
							LPSTR	pPar;

		         			FileListLoc = GSSillseek (FidFL,0,1);
							if ((pPar = strrchr (PltName,'(')))
							{
								*pPar = 0;//geofence entry
								Truncate (PltName);
							}
						}
		         		else
		         		{
		         			FileListLoc = 0;
		         			SubFile = 0; 
		         			GSSiClose (FidFL);
		         			goto NextFile;
		         		} 
		         	}     
		         	else
				    	_fstrcpy (PltName,CurView->lpFiles[CurView->CurFile]);
				 }
		         else
		         	goto NextFile;
		        if (!FastPick || FileNum != FastPickFileNum || !hRefIdx)
		        {
		        	CloseRefIndex (FALSE);
		    		OpenRefIndex (FALSE);
		    	}
		    	CheckForLargestPiece=FALSE;
		    	if (Prefix && *Prefix)
		    	{   
		    		PrefixIsDupref = !_fstrcmp (Prefix,"%DUPREF%");
		    		if (PrefixIsDupref || hTAGIdx)
		    		{
			    		strncpy0 (UDI,InUDI,32);
			    		if (*LastChr (UDI) == '}')
			    		{
			    			if ((pBrace=_fstrrchr (UDI,'{')))
			    			{
			    				*pBrace++ = 0;
			    				Sequence = atol (pBrace);
			    			}
			    		} 
			    		if (PrefixIsDupref)
			    			_fmemcpy (pRefIdxData,InUDI,sizeof(REFINDEXDATA));
	                    else
	                    {
							short	keylen = GetBTKeyLen (hTAGIdx);  
							BOOL	CheckNextTAG=TRUE;
			    			short	pos=BT_FIRST, cond=BT_GE;
							
							if (GetBTDataLen (hTAGIdx) > 8)
								CheckForLargestPiece=TRUE;	
				    		_fstrncpy(TAGKey.PREFIX,Prefix,8);
				    		_fstrncpy(TAGKey.UDI,UDI,32); 
				    		if (keylen == 42)
				    			TAGKey.Refno = 0;
				    		else
				    			TAGKey.Refno = Sequence;  
				    		while (CheckNextTAG)
				    		{
				    			CheckNextTAG = FALSE;
								st = BT_FIND (hTAGIdx,(LPSTR)&TAGKey,pos,cond,(LPSTR)pRefIdxData);
								if (!st)
								{
						    		if (_fstrncmp(TAGKey.PREFIX,Prefix,8) || 
						    			_fstrncmp(TAGKey.UDI,UDI,32))
						    			st=31;
								}
								if (!st && pRefIdxData->Deleted) 
								{
									pos = BT_NEXT;
									cond = BT_ANY; 
									CheckNextTAG = TRUE;
								}
							}
						}
					}
					else
						st = 31;
		    	}
		    	else
					st = BT_FIND (hRefIdx,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)pRefIdxData);
			    if (st && !FastPick)
			    	CloseRefIndex (FALSE);
				else
			    {
			    	if (CheckForLargestPiece)
			    	{   
			    		double Size = RectArea16 (&pRefIdxData->MinMax);
NextPiece:
	    				st = BT_FIND (hTAGIdx,(LPSTR)&TAGKey,BT_NEXT,BT_ANY,(LPSTR)pRefIdxDataTest);
						if (!st)
						{
				    		if (_fstrncmp(TAGKey.PREFIX,Prefix,8) || 
				    			_fstrncmp(TAGKey.UDI,UDI,32))
				    			st=31;
						}
						if (!st && pRefIdxData->Deleted) 
							goto NextPiece; 
						if (!st)
						{
							double TestSize = RectArea16 (&pRefIdxDataTest->MinMax); 
							
							if (TestSize > Size)
							{
								Size = TestSize;
								*pRefIdxData = *pRefIdxDataTest;
							}
							goto NextPiece;
						}
                    }
					PickingByRefno=TRUE; 
					SavePick = Pick;
					Pick = TRUE;
				    NumPicked = 0; 
				    _fmemset (&PickList[0],0,sizeof(PICKDATA));
				    PickList[0].ViewID = CurView->ID;
					PickList[0].ConfigID = CurrentConfig;
					PickList[0].FileNum = FileNum;  
					PickList[0].SubFile = SubFile; 
					PickList[0].FileInIndex = pRefIdxData->FileInIndex;
					PickList[0].Segment = pRefIdxData->Segment;
					PickList[0].Refno = Refno;
					PickList[0].Desc = 0;
					if (!FastPick)
						CloseRefIndex (FALSE);
					PickList[0].Offset = pRefIdxData->Offset; 
					PickList[0].Element = 0; 
					CurView->SubFile = SubFile; 
					if (SubFile)
					{
						strcpy (CurView->OrigFile,CurView->lpFiles[FileNum]);
						CurView->RestoreFile = FileNum;
					}
					PD=PickList[0];
					if (!PickedItemMinMax (NumPicked,&PD.Rect))
					{       
						Pick = SavePick;
						PickingByRefno=FALSE;
						CurView->SubFile = 0;
						goto NextFile;  
					}
					CurView->SubFile = 0;
				    NumPicked = 1;
				    PickList[0] = PD;
					PickList[0].HiPrecis = PolyIsHiPrecis;
				    PickList[0].PickedPoint = MinMaxMidPointD (&PickList[0].Rect);
					PD=PickList[0];
					PickList[0].FileInIndex = pRefIdxData->FileInIndex;
					PickList[0].Segment = pRefIdxData->Segment;
					PickList[0].Offset = pRefIdxData->Offset; 
					PickList[0].Desc = CurrentDesc;
					PickList[0].Refno = CurrentRefno;
					_fstrcpy (PickList[0].Prefix,CurrentPrefix);
					_fstrcpy (PickList[0].UDI,CurrentUDI);
					PickList[0].Type = 2; 
					if (CurrentType == GF_LINE || CurrentType == GF_POLYLINE) 
					{
						PickList[0].Type = 2; 
						if (GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&nPnts,&hPoly))  
						{   
							HPDPOINT lpPoints=(HPDPOINT)GlobalLock (hPoly);
							
							PickList[0].NumPoints = nPnts;
							PickList[0].BeginPoint = lpPoints[0];
							PickList[0].EndPoint = lpPoints[nPnts-1];
							PickList[0].Length = GetPolyLengthD (lpPoints,nPnts);
                            GSSiGlobUlFree (&hPoly);
						}
					}
					else if (CurrentType == GF_AREA) 
					{
						PickList[0].Type = 3; 
						if (GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&nPnts,&hPoly))  
						{   
							HPDPOINT lpPoints=(HPDPOINT)GlobalLock (hPoly);
							
							PickList[0].Area = ComputeProjectedAreaAreaD (lpPoints,nPnts,&PickList[0].Length);
                            GSSiGlobUlFree (&hPoly);
						}
					}
					else if (CurrentType == GF_POINT) 
					{
						PickList[0].Type = 1; 
						PickList[0].BeginPoint = PickList[0].EndPoint = PickList[0].PickedPoint = CurPointLocD;
					} 
					else if (CurrentType == GF_TEXT)
						PickList[0].Type = 4; 
					else if (CurrentType == GF_CURVE)
						PickList[0].Type = 5; 
					else if (CurrentType == GF_DELETE || ItemIsDeleted)
						PickList[0].IsDeleted = TRUE; 
					PD=PickList[0];
					rtn = 1; 
					PickingByRefno=FALSE;
					Pick = SavePick;
					goto Exit;
				}
		NextFile:
				if (FileListLoc)
					goto NextFileInList;
			}
		}
    }
	SetCurView ( SaveView);
	rtn = FALSE;
Exit: 
	if (FileListLoc)
		GSSiClose (FidFL);                      
	SetCurView ( SaveView);
	CurVis = SaveVis;
	GSSiGlobUlFree (&hVisList);	
	UseRefOrTAGIndex=SaveUseRORTI; 
	WantDescBlock = SaveWDB; 
	GSSiGlobUlFree (&hMem);
	strcpy (PltName,SavePltName);
	PltType = SavePltType;
	{
#if ENABLETRACE
GSSiExitProg (1027);
#endif
	return (rtn);
}
#if ENABLETRACE
}
#endif
} 

void OpenRefIndex (BOOL Delete)
#if ENABLETRACE
{GSSiEnterProg (1075);
#endif
{
	if (NoRefIndex && !Delete)
{
#if ENABLETRACE
GSSiExitProg (1075);
#endif
		return; 
}
	OpenRefIndex2 (Delete);
	OpenTAGIndex (Delete,StoreTAGBounds);
{
#if ENABLETRACE
GSSiExitProg (1075);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}
void OpenRefIndex2 (BOOL Delete)
#if ENABLETRACE
{GSSiEnterProg (1076);
#endif
{	BTVARDESC	BTVar[1];
	time_t ltime; 
	short	ii;  
	LPSTR	pLoc;

	if (hRefIdx && !Delete)  
	{
		if (!ForceRefIndex || BT_OPEN_FOR_WRITE (hRefIdx))
{
#if ENABLETRACE
GSSiExitProg (1076);
#endif
			return;
}
	}
	if (PltType == 3 || PltType == 5)
{
#if ENABLETRACE
GSSiExitProg (1076);
#endif
		return;
}   
	if (hRefIdx && PltType == 4 && ForceRefIndex && InRebuildRefIndex)  
{
#if ENABLETRACE
GSSiExitProg (1076);
#endif
		return;
}   
	BT_CLOSE (hRefIdx);
	hRefIdx = 0;
	
	if (PltType != 4 &&(PltType < 4 || MapFileType (PltName) == MT_PLT))
	{
		_fstrcpy (RefIndexFile,PltName); 
		ExpandText (RefIndexFile); 
		if ((pLoc = _fstrrchr (RefIndexFile,'.')))
			_fstrcpy (pLoc,".rin");
		else
{
#if ENABLETRACE
GSSiExitProg (1076);
#endif
			return;
}
    }
    else
    {   
    	char	Name[MAX_PATH];
    	
    	_fstrcpy (Name,PltName);
    	ExpandText (Name);
    	_fullpath (RefIndexFile,Name,sizeof(RefIndexFile));
		if ((pLoc = _fstrrchr (RefIndexFile,'\\')))
			_fstrcpy (pLoc,"\\refindex.rin");
		else
{
#if ENABLETRACE
GSSiExitProg (1076);
#endif
			return;
}
    }
    if (Delete && ForceRefIndex)
    {   
    	OFSTRUCTGM OFStruct;
    	
    	GSSiRemove (RefIndexFile);  
    	//retrn;
    }
	ltime = 0;
	if (ForceRefIndex)
	{   
		hRefIdx = BT_OPEN (RefIndexFile, ltime, BT_WRITE, 0);
	}
	else 
	{   
		if (CurView != PrevLayerVP)
		{   
			UINT	i;
			
			PrevLayerVP = CurView;
			for (i=0;i<MAX_VIEWPORT_FILES;i++)
				*LayerRefIndexFile[i] = 0;	
		}
		if (ExistFile (RefIndexFile))
		{
			if (*ReopenRefName)
				ii=1;
			hRefIdx = BT_OPEN (RefIndexFile, ltime, BT_READ, 0);  
		}
		if (CurView->CurFile >=0 && CurView->CurFile <MAX_VIEWPORT_FILES)
		{
			if (hRefIdx)
				_fstrcpy (LayerRefIndexFile[CurView->CurFile],RefIndexFile);
			else
				*LayerRefIndexFile[CurView->CurFile] = 0;
		}
		else
			ii=1;
	}
	if (!hRefIdx && ForceRefIndex)
	{
		BTVar[0].BT_VARTYP=BT_INTEGER;
		BTVar[0].BT_VARLEN=4;
		BTVar[0].BT_VAROFF=0;
		BT_CREATE (RefIndexFile, sizeof(REFINDEXDATA), FALSE, 1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
		ltime = 0;
		hRefIdx = BT_OPEN (RefIndexFile, ltime, BT_WRITE, 0);
	}
{
#if ENABLETRACE
GSSiExitProg (1076);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void CloseRefIndex (BOOL ForceClose)
#if ENABLETRACE
{GSSiEnterProg (1077);
#endif
{   
	if (InFixDupRef)
{
#if ENABLETRACE
GSSiExitProg (1077);
#endif
		return;
}
	if (hRefIdx && PltType == 4 && InRebuildRefIndex && !ForceClose)  
{
#if ENABLETRACE
GSSiExitProg (1077);
#endif
		return;
}
	*ReopenRefName=0;
	*ReopenTAGName=0;
	if (!hDupRef && !hRefIdx && !hTAGIdx)
{
#if ENABLETRACE
GSSiExitProg (1077);
#endif
		return;
}
//	BT_CLOSE (hDupRef);
//	hDupRef = 0;
	BT_CLOSE (hRefIdx);
	hRefIdx = 0;
	BT_CLOSE (hTAGIdx);
	hTAGIdx = 0;
{
#if ENABLETRACE
GSSiExitProg (1077);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 


short RefInPrevLayer (long Refno)//returns 0 if not, 1 if ref used by nondeleted item or 2 if by deleted item
#if ENABLETRACE
{GSSiEnterProg (1036);
#endif
{   
	short	i,Dummy, rtn=1;   
	REFINDEXDATA	RefIdxData;
	LPREFINDEXDATA	pRefIdxData=&RefIdxData;
	
	if (hDupRef || ForceRefIndex || ForceTAGIndex || !UseRefOrTAGIndex)
{
#if ENABLETRACE
GSSiExitProg (1036);
#endif
		return FALSE;
}
	if (hDisplayedRefs)
	{
		if (!BT_FIND(hDisplayedRefs,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)&Dummy))
{
#if ENABLETRACE
GSSiExitProg (1036);
#endif
			return 1;
}
	}		
	if (IgnorePrevLayers)
{
#if ENABLETRACE
GSSiExitProg (1036);
#endif
		return FALSE; 
}
	for (i=0;i<NumPrevLayers;i++)
	{
		if (!BT_FIND(hPrevLayers[i],(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)pRefIdxData))
		{   
			if (FidBlockedRefs != HFILE_ERROR)
			{
				BigWrite (FidBlockedRefs,(HPSTR)&Refno,4,-1);
				BigWrite (FidBlockedRefs,(HPSTR)&CurrentDesc,2,-1);
				BigWrite (FidBlockedRefs,(HPSTR)&i,2,-1);
				BigWrite (FidBlockedRefs,(HPSTR)pRefIdxData,sizeof(REFINDEXDATA),-1);
				BigWrite (FidBlockedRefs,(HPSTR)&FileNum,2,-1);
				pRefIdxData->FileInIndex = FileInIndex;
				pRefIdxData->Segment = ItemSeg;
				pRefIdxData->Offset = CurrentItem;
				BigWrite (FidBlockedRefs,(HPSTR)pRefIdxData,sizeof(REFINDEXDATA),-1);
			} 
			if (pRefIdxData->Deleted)
				rtn = 2;
{
#if ENABLETRACE
GSSiExitProg (1036);
#endif
			return rtn;
}
		}
	}
{
#if ENABLETRACE
GSSiExitProg (1036);
#endif
	return 0;
}
#if ENABLETRACE
}
#endif
} 

BOOL OpenPrevLayers (short CurLayer, short CurLayerID)
#if ENABLETRACE
{GSSiEnterProg (1037);
#endif
{   
	short	i;
	
	ClosePrevLayers ();
	NumPrevLayers = 0;
	if (IgnorePrevLayers)
{
#if ENABLETRACE
GSSiExitProg (1037);
#endif
		return FALSE; 
}
	if (DisplayAllRefs)
{
#if ENABLETRACE
GSSiExitProg (1037);
#endif
		return FALSE;
}
	for (i=0;i<CurLayer;i++)
		if (*LayerRefIndexFile[i])
			hPrevLayers[NumPrevLayers++] = BT_OPEN (LayerRefIndexFile[i], 0, BT_READ, 0);
{
#if ENABLETRACE
GSSiExitProg (1037);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL ClosePrevLayers (void)
#if ENABLETRACE
{GSSiEnterProg (1038);
#endif
{   
	if (NumPrevLayers>0)
	while (NumPrevLayers--) 
	{
		BT_CLOSE (hPrevLayers[NumPrevLayers]);
		hPrevLayers[NumPrevLayers] = 0;
	}
{
#if ENABLETRACE
GSSiExitProg (1038);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

void ReopenMap (BOOL Open)
#if ENABLETRACE
{GSSiEnterProg (1074);
#endif
{    
	static		OFSTRUCTGM	OFStruct;
	short		ii; 
	LPSTR		pDot;
	char		str[MAX_PATH],str2[MAX_PATH];
	extern		BOOL	ShareEnabled;   
	static		short	ReopenFNum, ReopenLayer; 
	
	if (!ShareEnabled)
{
#if ENABLETRACE
GSSiExitProg (1074);
#endif
		return;  
}
	if (Open)
	{ 
		if (FidMap>=1000 && FidMap < 2000) 
		{
			FidMap = GSSiOpenFile (OFStructOpenMap.szPathName,&OFStruct,OF_READ);
			OpenPrevLayers (ReopenFNum,ReopenLayer);
		} 
		if (*ReopenRefName)
			hRefIdx = BT_OPEN (ReopenRefName, 0, BT_READ, 0);
		if (*ReopenTAGName)
			hTAGIdx = BT_OPEN (ReopenTAGName, 0, BT_READ, 0);
		*ReopenRefName=0;
		*ReopenTAGName=0;
	}
	else
	{ 
		*ReopenRefName=0;
		*ReopenTAGName=0;
		if (FidMap != HFILE_ERROR) 
		{
			GSSiClose (FidMap); 
			ReopenFNum = FileNum;
			ReopenLayer = LayerID;  
			if (hRefIdx)
			{
				BT_GETPATHNAME (hRefIdx,ReopenRefName);
				BT_CLOSE (hRefIdx);
				hRefIdx = 0;
			}
			if (hTAGIdx)
			{
				BT_GETPATHNAME (hTAGIdx,ReopenTAGName);
				BT_CLOSE (hTAGIdx);
				hTAGIdx = 0;
			}
            _fstrcpy (str,PltName);
            ExpandText (str);
            _fstrupr (str);
            if ((pDot=_fstrstr (str,".PLT")))
            	*pDot = 0;
            _fstrcpy (str2,ReopenRefName);
            ExpandText (str2);
            _fstrupr (str2);
            if ((pDot=_fstrstr (str2,".RIN")))
            	*pDot = 0; 
            if (_fstricmp (str,str2))
            	ii=1;
            _fstrcpy (str2,ReopenTAGName);
            ExpandText (str2);
            _fstrupr (str2);
            if ((pDot=_fstrstr (str2,".TIN")))
            	*pDot = 0; 
            if (_fstricmp (str,str2))  
            	ii=1;
			ClosePrevLayers (); 
			FidMap += 1000; //signifies temp closed plt
		}
	}
{
#if ENABLETRACE
GSSiExitProg (1074);
#endif
	return;    
}
#if ENABLETRACE
}
#endif
} 

BOOL GetImageBounds (LPSTR PathName, HDIB32 hdib,LPMNMXCORD pBitmapBounds,LPMNMXCORD pWBounds)
{
	BITMAPINFOHEADER DibInfo={0}; 
    double	ScaleX, ScaleY;
    DPOINT	BitmapPoint,WorldPoint; 
    char	WorldFile[256],str[130];  
    LPSTR	pDot;  
    HFILE	Fid;
	HANDLE	hTran;
	double factor = 1.0;
	char units[32];


	if (hdib)
	{
		if (!GetBitmapInfoFromHandle (&DibInfo,hdib))
			return FALSE;  
		pBitmapBounds->xmn = pBitmapBounds->ymn = 0;
		pBitmapBounds->xmx = DibInfo.biWidth-1;
		pBitmapBounds->ymx = DibInfo.biHeight-1;
		if (GetGeoTiffData (hdib,&ScaleX,&ScaleY,&BitmapPoint, &WorldPoint))  
		{
			GetGlobalCVal ("[%GEOTIFFUNITS]",units,0);
			if (!stricmp (units,"FEET"))
				factor = FTM;
    		pWBounds->ymx = WorldPoint.y * factor;
    		pWBounds->xmn = WorldPoint.x * factor;  
    		pWBounds->xmx = (WorldPoint.x + ScaleX * (DibInfo.biWidth-1)) * factor;
    		pWBounds->ymn = (WorldPoint.y - ScaleY * (DibInfo.biHeight-1))* factor; 
    		return TRUE;
		} 
	}
    _fstrcpy (WorldFile,PathName);
    if ((pDot = _fstrrchr (WorldFile,'.')))
    {
		if (!_fstricmp(pDot, ".tif"))
			_fstrcpy(pDot, ".tfw");
		else if (!_fstricmp(pDot, ".jpg"))
			_fstrcpy(pDot, ".jpw");
		else if (!_fstricmp(pDot, ".bmp"))
		{
			LPBITMAPINFO pDibInfo;
			HANDLE hDibInfo;
			int ImageOffset;
			BOOL rc;

    		if ((Fid = GSSiOpenFile(PathName,0,OF_READ)) == HFILE_ERROR)
				return FALSE;
			rc = ReadBitMapHeader (Fid,&hDibInfo, &ImageOffset);
			GSSiClose (Fid);
			if (!rc)
				return FALSE;
			pDibInfo=(LPBITMAPINFO)GlobalLock (hDibInfo);
			DibInfo = pDibInfo->bmiHeader;
			GSSiGlobUlFree (&hDibInfo);
    		_fstrcpy (pDot,".bpw");
			GetGlobalCVal ("[%BPWUNITS]",units,0);
			if (!stricmp (units,"FEET"))
				factor = FTM;
		}
		else
			goto Exit;
		if (!ExistFile (WorldFile))
		{
			strcpy (pDot,".trn");
			if (!ExistFile (WorldFile))
				goto Exit;
			hTran = LoadTranFile (WorldFile,1,2,0,0);
			if (!hTran)
				goto Exit;
			*pWBounds = *pBitmapBounds;
			TranBounds (hTran,pWBounds);
			CloseTRANS2 (&hTran);
			return TRUE;
		}
    	if ((Fid = GSSiOpenFile(WorldFile,0,OF_READ)) == HFILE_ERROR)
    		goto Exit;
    	fgetstring (str,128,Fid);
    	ScaleX = atof (str);
    	fgetstring (str,128,Fid);
    	BitmapPoint.x = atof (str);
    	fgetstring (str,128,Fid);
    	BitmapPoint.y = atof (str);
    	fgetstring (str,128,Fid);
    	ScaleY = atof (str);
    	fgetstring (str,128,Fid);
    	WorldPoint.x = atof (str);
    	fgetstring (str,128,Fid);
    	WorldPoint.y = atof (str);
    	GSSiClose (Fid);
    	pWBounds->ymx = WorldPoint.y * factor;
    	pWBounds->xmn = WorldPoint.x * factor;  
    	pWBounds->xmx = (WorldPoint.x + ScaleX * (DibInfo.biWidth-1)) * factor;
    	pWBounds->ymn = (WorldPoint.y + ScaleY * (DibInfo.biHeight-1)) * factor; 
    	
    	return TRUE;
    }
Exit:	
   	pWBounds->xmn = pWBounds->ymn = 0;
   	pWBounds->ymx = DibInfo.biHeight-1;
   	pWBounds->xmx = DibInfo.biWidth-1;
    return TRUE;
} 

void ExpandPltName (LPSTR PltName)
{
	char	Where[256]="";
	LPSTR	pPar=strstr (PltName,".gmd(");

	if (!pPar)
		pPar=strstr (PltName,".GMD(");
	if (pPar)
	{
		pPar+=4;
		strncpy (Where,pPar,255);
		*pPar = 0;
	}
	ExpandText (PltName);
	strcat (PltName,Where);
	return;
}

BOOL OpenMap (HWND hWnd, HDC hDC)
#if ENABLETRACE
{GSSiEnterProg (1073);
#endif
{   //hWnd,hDC used only as flags
	short     Signature, i;   
    OFSTRUCTGM    OFStruct; 
    static		UINT	OpenMode=1000;   
    UINT		WantMode;
    char        mess[MAX_PATH];
    POINT       Point;      
    clock_t     starttime;       
    DWORD		FileSize;
    long		ii;
	DPOINT		Points[4]; 
	double		Dist1, Dist2;    
	MNMXCORD	BitmapBounds;
	LPSTR		pPar,pDot;
    struct	{short	Sig;
    		 BYTE	MV,SN1;
    		 UINT	SN2;} Trailer;
 //   char	str[256];

	InOpenMap = TRUE;
	ii=KillTimer (hWndMain,OKTOCONTINUETIMER);
    
    if (FidMap != HFILE_ERROR)
    {   
    	if (hDC || hWnd == (HWND)1)
    		WantMode = OF_READ;
    	else
    		WantMode = OF_READWRITE;
    	if (WantMode != OpenMode)
    		CloseMap (FALSE);
    	else
			goto RtnTrue;
    } 
    CloseBasePens(OpenMapOpenBP);   
	OpenMapOpenBP = FALSE;
	ExpandPltName (PltName);
    MapType = MapFileType (PltName);
	SetGlobalValue("%MAPFILE", PltName);
	switch (MapType)
	{
		default:
		{
			char drive[16],dir[MAX_PATH],leaf[128],File[MAX_PATH];

		    _splitpath (PltName,drive,dir,leaf,0); 
	    	sprintf (File,"%s%sglobal.ini",drive,dir); 
		    if (*drive)
				LoadGlobalInit (File,FALSE);
	    	if (*drive)
				sprintf (File,"%s%s%s.ini",drive,dir,leaf); 
		    LoadGlobalInit (File,FALSE);
		}
		break;
		case MT_HGF:
			PltType = 1;
		case MT_INDEX:
		case MT_DTM:
		case MT_HIGHLIGHTLIST:
		break;
	}

    if (MapType == MT_DTM && hDTM)
		goto RtnTrue;
    ClosePrevLayers ();
    if (PltType == 6)
		goto RtnTrue;
    if (!OkToContinue(FALSE))
		goto RtnFalse;
    if (!ReorgFile)
    {
	    MinFileTime = 0;
	    MaxFileTime = LONG_MAX;
	}
	else
	{
		if (!CreateReorgBackupFile (PltName))
			goto RtnFalse;
	}
    FoundInvalidRec = FALSE;  
    if (DoTime)
    	starttime=GetTickCount();
//    DescScan (-3,0); 
    if (PltType == 3)
    {   
	    if (MapFileType (PltName) == MT_SID)  
	    {
	        MapType = MT_IMAGE;
	    	goto DoSid;
	    } 
ProcessImageFile:  
		if (!InLoadBinaryFileList)
		{
			hCurImageMapDib = LoadDIB32 (PltName,TRUE); 
    		if (!hCurImageMapDib)
    			goto RtnFalse;  
		}
		else
			ii=1;
    	/*if (CurView->Rotation)
    	{
    		HDIB32	hDibRotated = GMRotateImageClassic (hCurImageMapDib,CurView->Rotation*DEGRAD);
    		
			DestroyDIB32 (hCurImageMapDib,FALSE); 
			hCurImageMapDib = hDibRotated;
		}
    	if (!hCurImageMapDib)
    		goto RtnTrue;  */ 

    	GetImageBounds (PltName,hCurImageMapDib,&BitmapBounds,&CurView->FileMNMX);
		CreateFileTranD (&BitmapBounds,&CurView->FileMNMX); 
    	MapType = MT_IMAGE;
    	goto S100;
    } 
    if (PltType == 7)
    {
	    FidMap = GSSiOpenFile (PltName,(LPOFSTRUCTGM)&OFStructOpenMap,OpenMode);
	    if (FidMap == HFILE_ERROR) 
	        goto RtnFalse;  
       	ProcessDisplayMacro (FidMap,1);
    	goto RtnTrue; 
    }
    if (PltType == 5 && MapType != MT_SID)
    {   
 /*    if (CurView->WindowZoomedToOrtho)
        {
            SetNewBoundsToOrtho();
            SetBounds(CurView->hWnd,CurView->hDC);
        }
        else if (CurView->WindowIsZoomed)
            SetNewBoundsToBounds();
        else if (!CurView->HaveBounds)
        {
            SaveView = CurView;
            CurView = pViewports[0];
            Bounds = CurView->WBounds;
            CurView = SaveView;
            CurView->NewBounds = Bounds;
            SetBounds(CurView->hWnd,CurView->hDC); 
        }*/
        goto RtnTrue;
    }
    FirstError = TRUE;
    maxbrush = -1; 
    sprintf (mess,"Opening %s Pass=%i",PltName,CurView->PassID);
    GSSiTrace (mess,0); 
    if (DisplayFiles==1)
    	SetWindowText (hWndMain,mess);   
    if (hDC || hWnd == (HWND)1) 
    	OpenMode = OF_READ;
    else
    	OpenMode = OF_READWRITE;
	if ((pPar = strstr (PltName,".GMD(")))
	{
		pPar += 4;
		*pPar = 0;
	}
	else if ((pPar = strstr (PltName,".gmd(")))
	{
		pPar += 4;
		*pPar = 0;
	}
	else if ((pDot = strrchr (PltName,'.')))
	{
		if ((pPar = _fstrrchr (pDot,'(')))
			*pPar = 0;
	}
	else
	{
		pDot = PltName;
		pPar = 0;
	}
/*	if ((pDot = strrchr (PltName,'.')))
	{
		if (strnicmp (pDot,".MDB",4))
		{
			if ((pPar = _fstrrchr (pDot,'(')))
				*pPar = 0;
		}
		else
		{
			pDot = PltName;
			pPar = 0;
		}
	}
	else
	{
		pDot = PltName;
		pPar = 0;
	}*/
	if (!strstr (PltName,".GDB") && !strstr (PltName,".gdb")) 
	{
		FidMap = GSSiOpenFile (PltName,(LPOFSTRUCTGM)&OFStructOpenMap,OpenMode); 
		if (pPar)
    		*pPar = '(';
		if (FidMap == HFILE_ERROR) 
			goto RtnFalse;
	    MapType = MapFileType (OFStructOpenMap.szPathName);
	}
	else
	{
		if (FileType (PltName) != 2)
			goto RtnFalse;
		FidMap = HFILE_ERROR;
		MapType = MT_FILE_GEO_DB;
		if (pPar)
    		*pPar = '(';
	}
    if (MapType == MT_DGN7)
    {
    	GSSiClose (FidMap);
    	FidMap = HFILE_ERROR;
//    	Fid = GSSiOpenFileMem (PltName);
    } 
    if (MapType == MT_IMAGE)
    {
    	GSSiClose (FidMap);
    	FidMap = HFILE_ERROR;
    	PltType = 3;
    	goto ProcessImageFile;
    }
    if (MapType == MT_SID)
    {
		DWORD NumMetaRecords;
		DWORD ColorSpace,NumBands,DataType,IsLocked;
		double MinMag,MaxMag,ULX,ULY,XRes,YRes,XRot,YRot,Mag=1, SidMag;  
DoSid:				
		if (MrSIDImageHandle) 
			MrSidClose (MrSIDImageHandle);

		SetGlobalValue("%LAYER_PROJECTION","");
		MrSIDImageHandle=MrSidOpen (PltName);  
		if (MrSIDImageHandle)
		{
    		char	GIFile[MAX_PATH];  
		    LPSTR	pBS;   
		
		    _fstrcpy (GIFile,PltName);
    		_fstrlwr (GIFile);
    		if ((pBS = _fstrrchr (GIFile,'\\')))
		    {
		    	_fstrcpy (pBS,"\\global.ini");
		    	LoadGlobalInit (GIFile,FALSE); 
				LayerUnits = PRJ_UNITS[3];  
			}
		    if (LayerUnits == 1 && PRJ_UNITS[1] == 2)
		    	MrSidConversion = FTM;
		    else if (LayerUnits == 2 && PRJ_UNITS[1] == 1)
		    	MrSidConversion = MFT;
		    else
		    	MrSidConversion = 1;

			MrSidGetInfo (MrSIDImageHandle,&MrSidWidth,&MrSidHeight,&ColorSpace,&NumBands,&DataType,&MinMag,&MaxMag,
							   		 &IsLocked,&ULX,&ULY,&XRes,&YRes,&XRot,&YRot,&NumMetaRecords);
			ULX *= MrSidConversion;
			ULY *= MrSidConversion;
			XRes *= MrSidConversion;
			YRes *= MrSidConversion;
			if (NumBands == 3)
				MrSidBitCount = 24;
			else
				MrSidBitCount = 8;
	    	MrSidBounds.xmx = ULX + (MrSidWidth-1) * XRes;
	    	MrSidBounds.ymn = ULY + (MrSidHeight-1) * YRes;
	    	MrSidBounds.ymx = ULY;
	    	MrSidBounds.xmn = ULX;  
			GetGlobalCVal ("[%LAYER_PROJECTION]",MrSidProjection,0);
			if (*MrSidProjection && !InLoadBinaryFileList)
			{
				char	SaveAltProj[MAX_PATH];

				ConvertCoordClose ();
				LoadProjection(0,MrSidProjection); 
				ConvertRectCoord (&CurView->FileMNMX,&MrSidBounds,0,1);
				ConvertRectCoord (&MrSidBounds,&CurView->WBounds,1,0);
			}
			else
			{
				CurView->FileMNMX = MrSidBounds;
				MrSidBounds = CurView->WBounds;
			}
	    	goto S100;
	    }
	    else
	    	goto RtnFalse;
    }
    if (!PeopleNet) 
    {
    	SetPltNameGlobals ();
	    if (!ReorgFile && hDC)
	    	OpenPrevLayers (FileNum,LayerID);
	}
	if (hDC || hWnd == (HWND)1)
	{  
	    if (LoadCompiledTran)
	    {   
	    	HFILE	TranFid;
	    	HANDLE	hTranName=GSSiGlobAlloc ( 984,GMEM_MOVEABLE,256);
	    	LPSTR	pDot,TranName=GlobalLock (hTranName);
	    	
			CloseTRANS2 (&hTranProjection);
			CloseTRANS2 (&hTranProjectionReverse);
	    	_fstrcpy (TranName,PltName);
	    	if ((pDot = _fstrrchr (TranName,'.')))
	    	{   
	    		_fstrcpy (pDot,".ctr");
				TranFid = GSSiOpenFile (TranName,(LPOFSTRUCTGM)&OFStruct,OF_READ); 
				if (TranFid != HFILE_ERROR)
				{
					if ((hTranProjection=ReadTranData(TranFid)))
					{
						hTranProjectionReverse=ReadTranData(TranFid);  
					}
					GSSiClose (TranFid);
				}
			}
			GSSiGlobUlFree (&hTranName);
	    }
	    //else
	    //	SetFileRotation (); 
	}
    
    switch (MapType)
    {
    	case 0: //GeoMaster plot file
	    {
		    FileSize = GSSillseek(FidMap,(LONG)-(6),2) + 6;
		    BigRead (FidMap,(HPSTR)&Trailer,6);  
		    Signature = Trailer.Sig;
		    if (Trailer.MV > 100)
		    {   
		    	DWORD	MapSerNo=0; 
		    	char	SN[4];
		    	
		    	if (!hDC)
		    	{
				    CloseMap(FALSE);
		    		goto RtnFalse;
		    	}
		    	_fmemmove (&MapSerNo,&Trailer.SN1,3); 
		    	MapSerNo -= FileSize%10000;
		    	if (MapSerNo != SerNo)   
		    	{   
		    		ExpandText (PltName);
		    		if (FirstSNMess)
				    	GSSiMessageBox ("Invalid serial number in map file",PltName,MB_ICONEXCLAMATION,0);  
				    FirstSNMess = FALSE;
				    CloseMap(FALSE);
				    goto RtnFalse;
				}
		    	Trailer.MV -= 100;
		    } 
		    MapVersion = Trailer.MV;
		    if (Signature != 32349)
		    {   BadMap: CloseMap (FALSE); 
		    	ExpandText (PltName);
		        _fullpath(mess,PltName,MAX_PATH);
				if (GSSiMessageBox("This is not a valid graphics file",mess,
				    MB_OKCANCEL|MB_ICONEXCLAMATION,0) == IDCANCEL)
					BlowOut(0,0);
		        goto RtnFalse;
		    }
		    if (MapVersion > 9 || MapVersion == 3)
		    {   CloseMap (FALSE);
		        GSSiMessageBox("This graphics file version is not recognized",PltName, MB_OK,0);
		        goto RtnFalse;
		    }
		    if (MapVersion < 2) goto BadMap; 
		    
		    if (ReorgFile)
		        ReorgFileOpen (PltName);
		    ii=GSSillseek(FidMap,(LONG)-(6+12),2);
		    DescBlockOffset = 0; 
		    BigRead (FidMap,(HPSTR)&PrimeOffset,4);
		    BigRead (FidMap,(HPSTR)&MinMax,8);
		    
		    {
		    	long save=MinMax.xmn;
		    	
		    	if (MinMax.xmn > MinMax.xmx || MinMax.ymn > MinMax.ymx)
		    	{
		    		MinMax.xmn = MinMax.xmx;
		    		MinMax.xmx = save;   
		    		MinMax.xmn = -32767;
		    		MinMax.ymn = -32767;
		    		MinMax.xmx = 32767;
		    		MinMax.ymx = 32767;
		    	}
		    }
		    Point.x = MinMax.xmn;
		    Point.y = MinMax.ymn;
		    ProjectFilePt (&Point);
		    MinMax.xmn = (short)Point.x;
		    MinMax.ymn = (short)Point.y;
		    Point.x = MinMax.xmx;
		    Point.y = MinMax.ymx;
		    ProjectFilePt (&Point);
		    MinMax.xmx = (short)Point.x;
		    MinMax.ymx = (short)Point.y;  
		    CurFileMinMax = MinMax; 
		    FileDateOffset = -1;
		    
		    GSSillseek(FidMap,PrimeOffset,0);
		    ProcessPrimarySeg (0, 0, 0,TRUE,0,HFILE_ERROR);   
		    if (ReorgFile)
		    {   
		        HANDLE  hSpace;
		        HPSTR   Space;
		        WORD    nbytes;
		        long	lnquad; 
		        PRIMEOFFSETS NewPrimeOffs; 
		        
		        ExpandText (PltName);
		        SetDlgItemText (ReorghWnd,ReorgFileTxt,PltName);
		        NewPrimeOffs.Code101 = 101;                 
		        NewPrimeOffs.Code102 = 102;                 
		        NewPrimeOffs.Code103 = 103;                 
		        NewPrimeOffs.Code200 = 200;                 
		        NewPrimeOffs.Code201 = 201;                 
		        NewPrimeOffs.Term = 0;              
		        NewPrimeOffs.DescBlockOffset = 0;  
		        NewPrimeOffs.Code202 = 202;                 
		        NewPrimeOffs.MinTime=MinFileTime;
		        NewPrimeOffs.MaxTime=MaxFileTime;
		        GSSillseek(FidMap,UsedDescOffset,0);
		        BigRead (FidMap,(HPSTR)&nbytes,2);
		        hSpace = GSSiGlobAlloc ( 985,GMEM_MOVEABLE,(DWORD)nbytes);
		        Space = GlobalLock (hSpace);
		        BigRead (FidMap,Space,(WORD)nbytes);  
		        GlobalUnlock (hSpace);
		        if (MapVersion < 8)
		        	nbytes = ConvertSymTable (&hSpace);
		        MapVersion = 8;
		        Space = GlobalLock (hSpace);  
		        CreateSymConversionTable (hWnd,Space,nbytes);
		        ConvertSymsInSymTable (Space,nbytes);
		        NewPrimeOffs.UsedDescOffset = ReorgOut (Space,nbytes);
		        GSSiGlobUlFree (&hSpace);
		        
		        GSSillseek(FidMap,TranPointOffset,0);
		        BigRead (FidMap,(HPSTR)&nbytes,2);
		        hSpace = GSSiGlobAlloc ( 986,GMEM_MOVEABLE,(DWORD)nbytes);
		        Space = GlobalLock (hSpace);
		        BigRead (FidMap,Space,(WORD)nbytes);
		        NewPrimeOffs.TranPointOffset = ReorgOut (Space,nbytes);
		        GSSiGlobUlFree (&hSpace);
		        
		        GSSillseek(FidMap,ColorPaletteOffset,0);
		        BigRead (FidMap,(HPSTR)&nbytes,2);
		        hSpace = GSSiGlobAlloc ( 987,GMEM_MOVEABLE,(DWORD)nbytes);
		        Space = GlobalLock (hSpace);
		        BigRead (FidMap,Space,(WORD)nbytes);
		        NewPrimeOffs.ColorPaletteOffset = ReorgOut (Space,nbytes);
		        GSSiGlobUlFree (&hSpace);
		        
		        GSSillseek(FidMap,GraphicsOffset+14,0);
		        BigRead (FidMap,(HPSTR)&lnquad,4);
		        hSpace = GSSiGlobAlloc ( 988,GMEM_MOVEABLE,lnquad+16);
		        Space = GlobalLock (hSpace);
		        GSSillseek(FidMap,GraphicsOffset+2,0);
		        BigRead (FidMap,Space,lnquad+16);
			    nbytes = 0; 
		        NewPrimeOffs.GraphicsOffset = GSSillseek (ReorgfileFID,0,1);  
				BigWrite (ReorgfileFID,(HPSTR)&nbytes,2,-1); //not used
				BigWrite (ReorgfileFID,(HPSTR)Space,lnquad+16,-1);
		        ReorgStartQuad = NewPrimeOffs.GraphicsOffset;
		        GSSiGlobUlFree (&hSpace);
		        
		        NewPrimeOffset = (long)ReorgOut ((char *)&NewPrimeOffs,sizeof(NewPrimeOffs));
		    }
		     
		    InvisInit ();
		    if (!AlwaysUseSymDict && !ReorgFile/* && !Pick && hDC*/)
		    {
		        GSSillseek(FidMap,UsedDescOffset,0);
				CreateOpenFileSymName();
		        ProcessPrimarySeg (0,0,-1,FALSE,0,HFILE_ERROR);   
		    } 
		    TRANS2 ((double) MinMax.xmn,(double) MinMax.ymn,
		            &CurView->FileMNMX.xmn,&CurView->FileMNMX.ymn,hTranFileToBase);
		    TRANS2 ((double) MinMax.xmx,(double) MinMax.ymx,
		            &CurView->FileMNMX.xmx,&CurView->FileMNMX.ymx,hTranFileToBase); 
		}
		break;
		
		case MT_SHP: //shape file  
		{   
			
			CloseTRANS2 (&hTranFileToBase); 
			CloseTRANS2 (&hTranBaseToFile);  
			CloseTRANS2 (&hTranFileToVP);  
			if (!(SHPType = ReadSHPHeader (FidMap,&CurView->FileMNMX,PltName)))
			{
		    	CloseMap(FALSE);
		    	goto RtnFalse;
		    }
			LoadSHPParm (PltName,SHPType,CurView->hWnd); 
			Points[0].x = ClipCoordToProjection (CurView->FileMNMX.xmn,1,0,1);   
			Points[0].y = ClipCoordToProjection (CurView->FileMNMX.ymn,2,0,1);   
			Points[1].x = ClipCoordToProjection (CurView->FileMNMX.xmn,1,0,1);   
			Points[1].y = ClipCoordToProjection (CurView->FileMNMX.ymx,2,0,1);   
			Points[2].x = ClipCoordToProjection (CurView->FileMNMX.xmx,1,0,1);   
			Points[2].y = ClipCoordToProjection (CurView->FileMNMX.ymx,2,0,1);   
			Points[3].x = ClipCoordToProjection (CurView->FileMNMX.xmx,1,0,1);   
			Points[3].y = ClipCoordToProjection (CurView->FileMNMX.ymn,2,0,1); 
			DBoundsInit (&CurView->FileMNMX);
			Dist1 = ldistp (Points[0],Points[2]); 
			for (i=0;i<4;i++)
			{
				if (ConvertCoord(&Points[i],0,1))
				{   
				    MessageBox(GetFocus(),"Unable to convert coordinates as specified", 0,MB_ICONQUESTION|MB_OK);
				    goto RtnFalse;
				}
				AddDPointToMinMax (&Points[i],&CurView->FileMNMX); 
			}
			Dist2 = ldistp (Points[0],Points[2]); 
			NonPltFileDistToBaseDist = Dist2/Dist1;
			if (CurView->FileMNMX.xmx - CurView->FileMNMX.xmn >
				CurView->FileMNMX.ymx - CurView->FileMNMX.ymn)
			{
				MinMax.xmn = -32000;
				MinMax.xmx = 32000;
				MinMax.ymn = -32000 * ((CurView->FileMNMX.ymx - CurView->FileMNMX.ymn)/(CurView->FileMNMX.xmx - CurView->FileMNMX.xmn));
				MinMax.ymx = -MinMax.ymn;
			} 
			else
			{
				MinMax.ymn = -32000;
				MinMax.ymx = 32000;
				MinMax.xmn = -32000 * ((CurView->FileMNMX.xmx - CurView->FileMNMX.xmn)/(CurView->FileMNMX.ymx - CurView->FileMNMX.ymn));
				MinMax.xmx = -MinMax.xmn;
			} 
			CreateFileTran (&MinMax,&CurView->FileMNMX); 
			NextSHPRec = 0;
			OpenSHPFileIndex (PltName,FidMap); 
        }
        break;
		case MT_PERSONAL_GEO_DB:
		{   
			
			CloseTRANS2 (&hTranFileToBase); 
			CloseTRANS2 (&hTranBaseToFile);  
			CloseTRANS2 (&hTranFileToVP);  
			if (!(SHPType = ReadPGDBHeader (PltName,&CurView->FileMNMX)))
			{
		    	CloseMap(FALSE);
		    	goto RtnFalse;
		    }
			LoadSHPParm (PltName,SHPType,CurView->hWnd); 
			Points[0].x = CurView->FileMNMX.xmn;   
			Points[0].y = CurView->FileMNMX.ymn;   
			Points[1].x = CurView->FileMNMX.xmn;   
			Points[1].y = CurView->FileMNMX.ymx;   
			Points[2].x = CurView->FileMNMX.xmx;   
			Points[2].y = CurView->FileMNMX.ymx;   
			Points[3].x = CurView->FileMNMX.xmx;   
			Points[3].y = CurView->FileMNMX.ymn; 
			DBoundsInit (&CurView->FileMNMX);
			Dist1 = ldistp (Points[0],Points[2]); 
			for (i=0;i<4;i++)
			{
				if (ConvertCoord(&Points[i],0,1))
				{   
				    MessageBox(GetFocus(),"Unable to convert coordinates as specified", 0,MB_ICONQUESTION|MB_OK);
				    goto RtnFalse;
				}
				AddDPointToMinMax (&Points[i],&CurView->FileMNMX); 
			}
			Dist2 = ldistp (Points[0],Points[2]); 
			NonPltFileDistToBaseDist = Dist2/Dist1;
			if (CurView->FileMNMX.xmx - CurView->FileMNMX.xmn >
				CurView->FileMNMX.ymx - CurView->FileMNMX.ymn)
			{
				MinMax.xmn = -32000;
				MinMax.xmx = 32000;
				MinMax.ymn = -32000 * ((CurView->FileMNMX.ymx - CurView->FileMNMX.ymn)/(CurView->FileMNMX.xmx - CurView->FileMNMX.xmn));
				MinMax.ymx = -MinMax.ymn;
			} 
			else
			{
				MinMax.ymn = -32000;
				MinMax.ymx = 32000;
				MinMax.xmn = -32000 * ((CurView->FileMNMX.xmx - CurView->FileMNMX.xmn)/(CurView->FileMNMX.ymx - CurView->FileMNMX.ymn));
				MinMax.xmx = -MinMax.xmn;
			} 
			CreateFileTran (&MinMax,&CurView->FileMNMX); 
			NextSHPRec = 0;
			if (hDC)
				OpenPGDBFileIndex (PltName,&CurView->WBounds); 
        }
        break;
		case MT_FILE_GEO_DB:
		{   
			
			FidMap = HFILE_FGDB;
			CloseTRANS2 (&hTranFileToBase); 
			CloseTRANS2 (&hTranBaseToFile);  
			CloseTRANS2 (&hTranFileToVP);  
			strcpy (ShapeFieldName,"SHAPE");
			if (hDC)
			{
				if (!(SHPType = ReadFGDBHeader(PltName, &CurView->FileMNMX)))
				{
					CloseMap(FALSE);
					goto RtnFalse;
				}
				LoadSHPParm(PltName, SHPType, CurView->hWnd);
				Points[0].x = CurView->FileMNMX.xmn;
				Points[0].y = CurView->FileMNMX.ymn;
				Points[1].x = CurView->FileMNMX.xmn;
				Points[1].y = CurView->FileMNMX.ymx;
				Points[2].x = CurView->FileMNMX.xmx;
				Points[2].y = CurView->FileMNMX.ymx;
				Points[3].x = CurView->FileMNMX.xmx;
				Points[3].y = CurView->FileMNMX.ymn;
				DBoundsInit(&CurView->FileMNMX);
				Dist1 = ldistp(Points[0], Points[2]);
				for (i = 0; i < 4; i++)
				{
					if (ConvertCoord(&Points[i], 0, 1))
					{
						MessageBox(GetFocus(), "Unable to convert coordinates as specified", 0, MB_ICONQUESTION | MB_OK);
						goto RtnFalse;
					}
					AddDPointToMinMax(&Points[i], &CurView->FileMNMX);
				}
				Dist2 = ldistp(Points[0], Points[2]);
				NonPltFileDistToBaseDist = FTM;//Dist2/Dist1;
				if (CurView->FileMNMX.xmx - CurView->FileMNMX.xmn >
					CurView->FileMNMX.ymx - CurView->FileMNMX.ymn)
				{
					MinMax.xmn = -32000;
					MinMax.xmx = 32000;
					MinMax.ymn = -32000 * ((CurView->FileMNMX.ymx - CurView->FileMNMX.ymn) / (CurView->FileMNMX.xmx - CurView->FileMNMX.xmn));
					MinMax.ymx = -MinMax.ymn;
				}
				else
				{
					MinMax.ymn = -32000;
					MinMax.ymx = 32000;
					MinMax.xmn = -32000 * ((CurView->FileMNMX.xmx - CurView->FileMNMX.xmn) / (CurView->FileMNMX.ymx - CurView->FileMNMX.ymn));
					MinMax.xmx = -MinMax.xmn;
				}
				CreateFileTran(&MinMax, &CurView->FileMNMX);
				OpenFGDBFileIndex(PltName, &CurView->WBounds);
			}
			else
			{
				LoadSHPParm(PltName, SHPType, CurView->hWnd);
				if (!(SHPType = ReadFGDBHeader(PltName, 0)))
				{
					CloseMap(FALSE);
					goto RtnFalse;
				}
			}
			NextSHPRec = 0;
		}
        break;

		case MT_ORA: //oracle export file  
		{   
			long	ORAHeaderType;
			
			CloseTRANS2 (&hTranFileToBase); 
			CloseTRANS2 (&hTranBaseToFile); 
			CloseTRANS2 (&hTranFileToVP); 
			ORAType = ORATypeFromName (PltName);
			LoadORAParm (PltName,ORAType); 
			if (!(ORAHeaderType = ReadORAHeader (FidMap,&CurView->FileMNMX)))
			{
		    	CloseMap(FALSE);
		    	goto RtnFalse;
		    }
		    if (!ORAType)
		    	ORAType = ORAHeaderType;
			ExpandORAPointBounds (&CurView->FileMNMX);  
			if (CurView->FileMNMX.xmx - CurView->FileMNMX.xmn >
				CurView->FileMNMX.ymx - CurView->FileMNMX.ymn)
			{
				MinMax.xmn = -32000;
				MinMax.xmx = 32000;
				MinMax.ymn = -32000 * ((CurView->FileMNMX.ymx - CurView->FileMNMX.ymn)/(CurView->FileMNMX.xmx - CurView->FileMNMX.xmn));
				MinMax.ymx = -MinMax.ymn;
			} 
			else
			{
				MinMax.ymn = -32000;
				MinMax.ymx = 32000;
				MinMax.xmn = -32000 * ((CurView->FileMNMX.xmx - CurView->FileMNMX.xmn)/(CurView->FileMNMX.ymx - CurView->FileMNMX.ymn));
				MinMax.xmx = -MinMax.xmn;
			} 
			CreateFileTran (&MinMax,&CurView->FileMNMX); 
			NextORARec = 0;
			switch (OpenORAFileIndex (FidMap,PltName))
			{   
				case 0:
		    		CloseMap(FALSE);
		    		goto RtnFalse;
				case ORAT_POINT: 
					if (!GetTypeVisibility(TYPE_POINT))
					{
		    			CloseMap(FALSE);
						goto RtnFalse;
					}
                    break;
				case ORAT_ARC:
					if (!GetTypeVisibility(TYPE_LINECURVE))
					{
		    			CloseMap(FALSE);
						goto RtnFalse;
					}
                    break;
				case ORAT_TEXT:
					if (!GetTypeVisibility(2))
					{
		    			CloseMap(FALSE);
						goto RtnFalse;
					}
                    break;
				case ORAT_POLYGON:
					if (!GetTypeVisibility(TYPE_AREA))
					{
		    			CloseMap(FALSE);
						goto RtnFalse;
					}
                    break;
				default:
					break;
		    }
        }
        break;
		case MT_GMD: //GMD Point file  
		{   
			
			CloseTRANS2 (&hTranFileToBase); 
			CloseTRANS2 (&hTranBaseToFile); 
			CloseTRANS2 (&hTranFileToVP); 
	    	GSSiClose (FidMap);
			if (!OpenGMDMapFile (PltName))
		    	goto RtnFalse;
			FidMap = HFILE_GMD;
			LoadGMDParm (PltName,hWndMain); 
			if (!ReadGMDHeader (FidMap,&CurView->FileMNMX))
			{
		    	CloseMap(FALSE);
		    	goto RtnFalse;
		    }
			//ExpandGMDPointBounds (&CurView->FileMNMX);  should get max bounds for all possible gmd symbols
			if (CurView->FileMNMX.xmx - CurView->FileMNMX.xmn >
				CurView->FileMNMX.ymx - CurView->FileMNMX.ymn)
			{
				MinMax.xmn = -32000;
				MinMax.xmx = 32000;
				MinMax.ymn = -32000 * ((CurView->FileMNMX.ymx - CurView->FileMNMX.ymn)/(CurView->FileMNMX.xmx - CurView->FileMNMX.xmn));
				MinMax.ymx = -MinMax.ymn;
			} 
			else
			{
				MinMax.ymn = -32000;
				MinMax.ymx = 32000;
				MinMax.xmn = -32000 * ((CurView->FileMNMX.xmx - CurView->FileMNMX.xmn)/(CurView->FileMNMX.ymx - CurView->FileMNMX.ymn));
				MinMax.xmx = -MinMax.xmn;
			} 
			CreateFileTran (&MinMax,&CurView->FileMNMX); 
			NextGMDRec = 0;
			if (!GetTypeVisibility(TYPE_POINT))
			{
		    	CloseMap(FALSE);
				goto RtnFalse;
			}
        }
        break;
		case MT_DGN7: //DGN file  
		{   
			
			CloseTRANS2 (&hTranFileToBase); 
			CloseTRANS2 (&hTranBaseToFile);  
			CloseTRANS2 (&hTranFileToVP); 
			if (!(OpenDGNFile (PltName,&CurView->FileMNMX)))
			{
		    	goto RtnFalse;
		    } 
		    FidMap = HFILE_DGN;
			LoadDGNParm (PltName); 
			Points[0].x = CurView->FileMNMX.xmn;   
			Points[0].y = CurView->FileMNMX.ymn;   
			Points[1].x = CurView->FileMNMX.xmn;   
			Points[1].y = CurView->FileMNMX.ymx;   
			Points[2].x = CurView->FileMNMX.xmx;   
			Points[2].y = CurView->FileMNMX.ymx;   
			Points[3].x = CurView->FileMNMX.xmx;   
			Points[3].y = CurView->FileMNMX.ymn; 
			DBoundsInit (&CurView->FileMNMX);  
			Dist1 = ldistp (Points[0],Points[2]);
			for (i=0;i<4;i++)
			{
				if (ConvertCoord(&Points[i],0,1))
				{   
				    MessageBox(GetFocus(),"Unable to convert coordinates as specified", 0,MB_ICONQUESTION|MB_OK);
				    goto RtnFalse;
				}
				AddDPointToMinMax (&Points[i],&CurView->FileMNMX); 
			}
			Dist2 = ldistp (Points[0],Points[2]); 
			NonPltFileDistToBaseDist = Dist2/Dist1;
			if (CurView->FileMNMX.xmx - CurView->FileMNMX.xmn >
				CurView->FileMNMX.ymx - CurView->FileMNMX.ymn)
			{
				MinMax.xmn = -32000;
				MinMax.xmx = 32000;
				MinMax.ymn = -32000 * ((CurView->FileMNMX.ymx - CurView->FileMNMX.ymn)/(CurView->FileMNMX.xmx - CurView->FileMNMX.xmn));
				MinMax.ymx = -MinMax.ymn;
			} 
			else
			{
				MinMax.ymn = -32000;
				MinMax.ymx = 32000;
				MinMax.xmn = -32000 * ((CurView->FileMNMX.xmx - CurView->FileMNMX.xmn)/(CurView->FileMNMX.ymx - CurView->FileMNMX.ymn));
				MinMax.xmx = -MinMax.xmn;
			} 
			CreateFileTran (&MinMax,&CurView->FileMNMX); 
//			NextSHPRec = 0;
//			OpenSHPFileIndex (PltName); 
        }
        break;
		case MT_GPX: //GPX file  
		{   
			
			GSSiClose (FidMap);
			FidMap = HFILE_ERROR;
			CloseTRANS2 (&hTranFileToBase); 
			CloseTRANS2 (&hTranBaseToFile);  
			CloseTRANS2 (&hTranFileToVP); 
			if (!(hGPX = OpenGPXFile (PltName,&CurView->FileMNMX)))
			{
		    	goto RtnFalse;
		    } 
		    FidMap = HFILE_GPX;
			LoadGPXParm (PltName); 
			Points[0].x = CurView->FileMNMX.xmn;   
			Points[0].y = CurView->FileMNMX.ymn;   
			Points[1].x = CurView->FileMNMX.xmn;   
			Points[1].y = CurView->FileMNMX.ymx;   
			Points[2].x = CurView->FileMNMX.xmx;   
			Points[2].y = CurView->FileMNMX.ymx;   
			Points[3].x = CurView->FileMNMX.xmx;   
			Points[3].y = CurView->FileMNMX.ymn; 
			DBoundsInit (&CurView->FileMNMX);  
			Dist1 = ldistp (Points[0],Points[2]);
			for (i=0;i<4;i++)
			{
				if (ConvertCoord(&Points[i],2,1))
				{   
				    MessageBox(GetFocus(),"Unable to convert coordinates as specified", 0,MB_ICONQUESTION|MB_OK);
				    goto RtnFalse;
				}
				AddDPointToMinMax (&Points[i],&CurView->FileMNMX); 
			}
			Dist2 = ldistp (Points[0],Points[2]); 
			NonPltFileDistToBaseDist = Dist2/Dist1;
			if (CurView->FileMNMX.xmx - CurView->FileMNMX.xmn >
				CurView->FileMNMX.ymx - CurView->FileMNMX.ymn)
			{
				MinMax.xmn = -32000;
				MinMax.xmx = 32000;
				MinMax.ymn = -32000 * ((CurView->FileMNMX.ymx - CurView->FileMNMX.ymn)/(CurView->FileMNMX.xmx - CurView->FileMNMX.xmn));
				MinMax.ymx = -MinMax.ymn;
			} 
			else
			{
				MinMax.ymn = -32000;
				MinMax.ymx = 32000;
				MinMax.xmn = -32000 * ((CurView->FileMNMX.xmx - CurView->FileMNMX.xmn)/(CurView->FileMNMX.ymx - CurView->FileMNMX.ymn));
				MinMax.xmx = -MinMax.xmn;
			} 
			CreateFileTran (&MinMax,&CurView->FileMNMX); 
//			NextSHPRec = 0;
//			OpenSHPFileIndex (PltName); 
        }
        break;
		case MT_KML: //KML file  
		{   
			
			GSSiClose (FidMap);
			FidMap = HFILE_ERROR;
			CloseTRANS2 (&hTranFileToBase); 
			CloseTRANS2 (&hTranBaseToFile);  
			CloseTRANS2 (&hTranFileToVP); 
			if (!(hKML = OpenKMLFile (PltName,&CurView->FileMNMX)))
			{
		    	goto RtnFalse;
		    } 
		    FidMap = HFILE_KML;
			LoadKMLParm (PltName); 
			Points[0].x = CurView->FileMNMX.xmn;   
			Points[0].y = CurView->FileMNMX.ymn;   
			Points[1].x = CurView->FileMNMX.xmn;   
			Points[1].y = CurView->FileMNMX.ymx;   
			Points[2].x = CurView->FileMNMX.xmx;   
			Points[2].y = CurView->FileMNMX.ymx;   
			Points[3].x = CurView->FileMNMX.xmx;   
			Points[3].y = CurView->FileMNMX.ymn; 
			DBoundsInit (&CurView->FileMNMX);  
			Dist1 = ldistp (Points[0],Points[2]);
			for (i=0;i<4;i++)
			{
				if (ConvertCoord(&Points[i],2,1))
				{   
				    MessageBox(GetFocus(),"Unable to convert coordinates as specified", 0,MB_ICONQUESTION|MB_OK);
				    goto RtnFalse;
				}
				AddDPointToMinMax (&Points[i],&CurView->FileMNMX); 
			}
			Dist2 = ldistp (Points[0],Points[2]); 
			NonPltFileDistToBaseDist = Dist2/Dist1;
			if (CurView->FileMNMX.xmx - CurView->FileMNMX.xmn >
				CurView->FileMNMX.ymx - CurView->FileMNMX.ymn)
			{
				MinMax.xmn = -32000;
				MinMax.xmx = 32000;
				MinMax.ymn = -32000 * ((CurView->FileMNMX.ymx - CurView->FileMNMX.ymn)/(CurView->FileMNMX.xmx - CurView->FileMNMX.xmn));
				MinMax.ymx = -MinMax.ymn;
			} 
			else
			{
				MinMax.ymn = -32000;
				MinMax.ymx = 32000;
				MinMax.xmn = -32000 * ((CurView->FileMNMX.xmx - CurView->FileMNMX.xmn)/(CurView->FileMNMX.ymx - CurView->FileMNMX.ymn));
				MinMax.xmx = -MinMax.xmn;
			} 
			CreateFileTran (&MinMax,&CurView->FileMNMX); 
//			NextSHPRec = 0;
//			OpenSHPFileIndex (PltName); 
        }
        break;
        
        case MT_DTM:
        {   
        	if (CurView->DTMRenderAs[CurView->CurFile] != DTM_RENDER_SLOPE_POLYGONS &&
        		(CurView->PassID != 3 || !GetTypeVisibility (TYPE_CONTOUR)))
        		goto RtnFalse;
        	if (CurView->DTMRenderAs[CurView->CurFile] == DTM_RENDER_SLOPE_POLYGONS &&
        		(CurView->PassID && CurView->PassID != 2))
        		goto RtnFalse;
	       	hDTM = DTMOpen (PltName,DBL_MAX,BT_READ,0);  
	       	if (!GetNextDTMSegment (TRUE))
	       		goto RtnFalse;
        }
        break;
	}
//    SetInvisFromDict ();
S100:
    if (!CurView->HaveBounds)  
    	CurView->NewBounds = CurView->FileMNMX;
    SetRezoomBounds (CurView->NewBounds);

    if (!CurView->HaveBounds)
    {   
        BOOL    SaveDisableHalt;
		char	SavePltName[MAX_PATH];

		strcpy (SavePltName,PltName);
        
        if (CurView->WindowZoomedToOrtho)
            SetNewBoundsToOrtho();
        else if (CurView->WindowIsZoomed)
            SetNewBoundsToBounds();
        SaveDisableHalt = DisableHalt;
        DisableHalt = TRUE;
		SetScaleAndMidpointFromBounds (CurView);
        SetBounds(CurView->hWnd,CurView->hDC);
        DisableHalt = SaveDisableHalt;
		strcpy (PltName,SavePltName);
    }
    
    if (!MapType)
    {
	    GSSillseek(FidMap,GraphicsOffset,0);
	    LoadQuadTree();  
	    if (ReorgFile)
	        AllocateDescBlocks(); 
	//    else  
	    LoadDescBlocks (DescBlockOffset);
    }
    if ((hWnd > (HWND)1 || hDC)  && !SetFileBounds())
    {   
    	CloseMap(FALSE);
    	goto RtnFalse;
    }
    if (Display && !Pick)
    {    
	    if (!MapType)
	    {
	        GSSillseek(FidMap,ColorPaletteOffset,0);
			for (i=1;i<maxbrush+1;i++)
			{
    			GSSiDeleteObject (&brushes[i]);
				GSSiDeleteObject (&pens[i]);
			}
	        ProcessPrimarySeg (0, 0, 0,FALSE,0,HFILE_ERROR);
        }
        if (MapType != MT_DTM)
        {
        	OpenMapOpenBP = OpenBasePens (); 
        	OpenNewObjects();
        }
    }
    if (ForceRefIndex || ForceTAGIndex || UseRefOrTAGIndex)
    {
        OpenRefIndex (FALSE);
        nPltBytesRead = 0;  
        switch (MapType)
        {
        	case MT_PLT:
        	case MT_ORA:
        	case MT_SHP:
        		CurPltFileLen = GSSillseek (FidMap,0,2);
        		break;
        	case MT_DGN7:
        		CurPltFileLen = DGNNumElements;
        		break;
        	case MT_GPX:
        		CurGPXFileLen = GPXNumElements;
        		break;
        	case MT_KML:
        		CurKMLFileLen = KMLNumElements;
        		break;
        }
        if (hWnd && (ForceRefIndex || ForceTAGIndex))
        	StatusWindowUpdate2 (0,CurPltFileLen,nPltBytesRead); 
	}
    ContinuationOffset=-1;
/*    OpenHighlightList();*/
/*    WantType[0]=FALSE;*/
RtnTrue: 
	if (DoTime)
    	OpenMapTime += GetTickCount()-starttime;
	InOpenMap = FALSE;

{
#if ENABLETRACE
GSSiExitProg (1073);
#endif  
//	sprintf (str,"%s T",PltName);
//	SetWindowText (hWndMain,str);
//	if (Pick)
//		Wait (100);
    return (TRUE);  
}
RtnFalse:  
	InOpenMap = FALSE;
	if (DoTime)
    	OpenMapTime += GetTickCount()-starttime;
{
#if ENABLETRACE
GSSiExitProg (1073);
#endif
//	sprintf (str,"%s F",PltName);
//	SetWindowText (hWndMain,str); 
//	if (Pick)
//		Wait (100); 
	return (FALSE);
}
    
#if ENABLETRACE
}
#endif
}

void CloseMap (BOOL Update)
#if ENABLETRACE
{GSSiEnterProg (8);
#endif
{   short i,ii;

	SetGlobalValue("%MAPFILE", "");
	if (InOpenMap)
		ii=1;
	ClosePrevLayers ();
	SetSavedGraphicsFid (0);

    if (idTimer)
    {
        if (!MemMap)
        {
		    if (Display && !Pick)
		    {   
		    	idTimer=0;
		    	if (CurrentConfig)
		    	{
		        	GSSiSetCursor (VPCursor (hWndMain));
		        	PostMessage(hWndMain, WM_SETCURSOR, 0, 0L); 
		        }
	        }
	        else
	        	ii=1;
        }
	    idTimer = 0; 
        KillTimer(hWndMain, 1); 
        if (TrapKillTimer)
        	ii=1;
    }
    if (Update  && FidMap != HFILE_ERROR && FidMap < 1000)
    {
    	if (hDescBlock)
    	{
			LPDESCBLOCK	pDescBlock;
			
		    GSSillseek(FidMap,DescBlockOffset,0); 
		    BigRead (FidMap,(HPSTR)&DescBlockLen,4);
			pDescBlock = (LPDESCBLOCK)GlobalLock (hDescBlock);
			{
				LPDESCBLOCK	pDescBlock2 = pDescBlock + 1324;
				
				ii=1;
			}

			BigWrite (FidMap,(HPSTR)pDescBlock,DescBlockLen,-1);
			GlobalUnlock (hDescBlock); 
		}
		if (FileDateOffset >= 0)
		{   
			short	ID;
			GSSillseek(FidMap,FileDateOffset,0);
			BigRead (FidMap,(HPSTR)&ID,2); 
			if (ID == 202)  
			{
		    	if (BigWrite (FidMap,(HPSTR)&MinFileTime,4,-1) != 4)
		    		ii=1;
		    	BigWrite (FidMap,(HPSTR)&MaxFileTime,4,-1);
		    } 
		}
	}
    CloseRefIndex (FALSE);
    CloseBasePens(OpenMapOpenBP);   
	OpenMapOpenBP = FALSE;
	DestroyOpenFileSymNames ();
    ReorgFileClose (PltName);
    if (hQuadTree2 && FidMap != HFILE_ERROR && FidMap < 1000)
    {   
        HPSTR   pQuadTree, pQuadTree2;
        LPQUAD  pQuad, pQuad2;
        long    QuadOff, NumQuadSegs;
        
        pQuadTree  = GlobalLock (hQuadTree);
        pQuadTree2 = GlobalLock (hQuadTree2);
        GSSillseek (FidMap,QuadTreeOffset,0);
        BigRead (FidMap,pQuadTree,LenQuad); 
        NumQuadSegs = LenQuad/LenQuadSeg;
        for (QuadOff=0;QuadOff<NumQuadSegs;QuadOff++)
        {   
        	if (QuadOff > 2046)
        		ii=1;
            pQuad     = (LPQUAD)(pQuadTree + QuadOff*LenQuadSeg);
            pQuad2    = (LPQUAD)(pQuadTree2 + QuadOff*LenQuadSeg); 
            pQuad->MinMax = pQuad2->MinMax;  
        }
        GSSillseek (FidMap,QuadTreeOffset,0);
        BigWrite (FidMap,(HPSTR)pQuadTree,LenQuad,-1);  
        GSSiGlobUlFree (&hQuadTree2);
        GlobalUnlock (hQuadTree);
    } 
    if (KeepTranFileToBase && hTranFileToBase)
    {
    	KeepTranFileToBase = hTranFileToBase;
    	hTranFileToBase = 0;
    }
    CloseTRANS2 (&hTranFileToBase);               //of the server from opening
    CloseTRANS2 (&hTranBaseToFile);
    CloseTRANS2 (&hTranFileToVP);
    GSSiGlobFree (&hQuadTree);
    GSSiGlobFree (&hQuadOffset);
    hQuadTree2 = 0;
    if (hDescBlock)
        GSSiGlobFree (&hDescBlock);  
    CloseNewObjects();
    for (i=1;i<maxbrush+1;i++)
    {
    	GSSiDeleteObject (&brushes[i]);
		GSSiDeleteObject (&pens[i]);
    }
    maxbrush = 0;
   	if (hFileSymList)
	{
		HANDLE	hSymDesc=0;
		short	NumSyms=0;
		LPSHORT	pDesc=(LPSHORT)GlobalLock (hFileSymList);
		
		while (*pDesc)  
			AddToSymList (*pDesc++,&NumSyms,&hSymDesc); 
		GSSiGlobUlFree (&hFileSymList);
		AddSymToMap ((short)-NumSyms,hSymDesc,0,0); 
        DestroySymList (&NumSyms,&hSymDesc);
	} 
	if (MapType == MT_SHP)
		OpenSHPFileIndex (0,HFILE_ERROR);  
	else if (MapType == MT_PERSONAL_GEO_DB)
		OpenPGDB (0,0,0);
	else if (MapType == MT_FILE_GEO_DB)
		OpenFGDB (0,0,0);
	else if (MapType == MT_GMD)
		CloseGMDMapFile (); 
	else if (MapType == MT_ORA)
		OpenORAFileIndex (HFILE_ERROR,0);
	else if (MapType == MT_DGN7)  
		CloseDGNFile ();
	else if (MapType == MT_GPX)  
		CloseGPXFile (&hGPX);
	else if (MapType == MT_KML)  
		CloseKMLFile (&hKML);
	else if (MapType == MT_DTM) 
	{   
		InCloseMap=TRUE;
		DTMClose (&hDTM); 
		InCloseMap=FALSE; 
		DisplayContourLabels (TRUE);
	}
	else if (MapType == MT_IMAGE)
	{
		DestroyDIB32 (hCurImageMapDib,FALSE); 
		hCurImageMapDib = 0;
	}
	else if (MapType == MT_SID)
	{
		MrSidClose (MrSIDImageHandle);
		MrSIDImageHandle = 0;
	}
    if ((FidMap>=0 && FidMap < 1000) || FidMap >= 2000)                    
    {   
    	if (PltType == 7)
    		ProcessDisplayMacro (FidMap,3); 
        i=GSSiClose (FidMap);
        if (i == HFILE_ERROR)
        {
            char    str[256];
            
            sprintf (str,"Error closing map: %i %s",FidMap,PltName);
            GSSiTrace (str,0);
        }
    } 
    FidMap = HFILE_ERROR;
    MapType = 0;
    if (DisplayFileParam) DebugWait = TRUE;    
{
#if ENABLETRACE
GSSiExitProg (8);
#endif
    return;
}
/*    CloseHighlightList();*/
#if ENABLETRACE
}
#endif
}


BOOL GetPolyPnts (LPPICKDATAHEADER PickData,BOOL Reverse,LPLONG pnPnts, LPHANDLE pHandle,BOOL WantUS)
{   
	BOOL SaveUS = WantUnsplinedPoints, rtn; 
    
    WantUnsplinedPoints = WantUS;
	rtn =GetPolyPoints (PickData,Reverse,pnPnts, pHandle);
	WantUnsplinedPoints = SaveUS;
	return rtn;
}

BOOL GetPolyPoints (LPPICKDATAHEADER PickData,BOOL Reverse,LPLONG pnPnts, LPHANDLE pHandle)
{
	LPTHEME	pTheme, SaveTheme=CurTheme; 
	LPVIEWPORT	SaveVP=CurView;
	HPDPOINT pPolyPoints,lpDPoint;
	BOOL	rtn=FALSE; 
	short	SavePass;
    LPVISLIST	SaveVis = CurVis;
	HANDLE		hVisList=GSSiGlobAlloc ( 964,GHND,sizeof(VISLIST));
	
	CurVis = (LPVISLIST)GlobalLock (hVisList);
	CurVis->hVisList=hVisList;
	InitVis ();
	*pnPnts = 0;  
	*pHandle = 0;
	*(LPPICKDATAHEADER)&PickList[0] = *PickData;
	if (PickList[0].Type == 2 || PickList[0].Type == 3 || PickList[0].Type == 5)
	{   
	    SetConfig (PickList[0].ConfigID);
	    SetViewport (PickList[0].ViewID);
		pTheme = AddTheme (GF_SAVEPOLY_THEME); 
		SavePass = CurView->PassID;
		CurView->PassID = 4;
		ProcessSelectedTheme = CurView->NumThemes;
		WantSegmentID = PickList[0].PolyID;
		ProcessPickedItem (0,FALSE);
		WantSegmentID = 0;
		CurView->PassID = SavePass; 
		ProcessSelectedTheme = 0;       		
		DeleteTheme (pTheme); 
		GetSavedPolys ();
		if (hSavePoly)
		{   LPMNMXCORD lpRect;
			HPDPOINT	lpDpoint;
	    		    
            *pnPnts = nSavePoly; 
            lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
            lpRect++;
            lpDpoint = (LPDPOINT) lpRect;  
            *pHandle = GSSiGlobAlloc ( 623,GMEM_MOVEABLE,(nSavePoly+1)*sizeof(DPOINT));
            pPolyPoints = (HPDPOINT)GlobalLock (*pHandle);
            hmemmove ((HPSTR)pPolyPoints,(HPSTR)lpDpoint,nSavePoly*sizeof(DPOINT));
            if (PickList[0].Type == 3 && *pnPnts > 2 && ldistp (pPolyPoints[0],pPolyPoints[nSavePoly-1]) > P_TOL)
            {   
            	(*pnPnts)++;
            	pPolyPoints[nSavePoly] = pPolyPoints[0];
            }   
            GlobalUnlock (*pHandle); 
            GlobalUnlock (hSavePoly);
			DestroySavedPolys ();  
			if (Reverse)
				*pHandle = ReversePoints (*pnPnts,*pHandle);
            rtn = TRUE;
        }
        else
			rtn = FALSE;
	} 
	CurTheme = SaveTheme;
	CurVis = SaveVis;
	GSSiGlobUlFree (&hVisList);	
	SetCurView ( SaveVP);
	return rtn;
}

BOOL GetPolyPoints3D (LPPICKDATAHEADER PickData,BOOL Reverse,LPLONG pnPnts, LPHANDLE pHandle)
{
	LPTHEME	pTheme, SaveTheme=CurTheme; 
	LPVIEWPORT	SaveVP=CurView;
	HPDPOINT3D pPolyPoints,lpDPoint;
	BOOL	rtn=FALSE; 
	short	SavePass;   
	HPFLOAT	pElev;  
	long	i;
	
	*pnPnts = 0;  
	*pHandle = 0;
	*(LPPICKDATAHEADER)&PickList[0] = *PickData;
	if (PickList[0].Type == 2 || PickList[0].Type == 3 || PickList[0].Type == 5)
	{   
	    SetConfig (PickList[0].ConfigID);
	    SetViewport (PickList[0].ViewID);
		pTheme = AddTheme (GF_SAVEPOLY_THEME); 
		SavePass = CurView->PassID;
		CurView->PassID = 4;
		ProcessSelectedTheme = CurView->NumThemes;
		ProcessPickedItem (0,FALSE);
		CurView->PassID = SavePass; 
		ProcessSelectedTheme = 0;       		
		DeleteTheme (pTheme); 
		GetSavedPolys ();
		if (hSavePoly)
		{   LPMNMXCORD lpRect;
			HPDPOINT	lpDpoint;
	    		    
            *pnPnts = nSavePoly; 
            lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
            lpRect++;
            lpDpoint = (LPDPOINT) lpRect;  
            *pHandle = GSSiGlobAlloc ( 623,GMEM_MOVEABLE,(nSavePoly+1)*sizeof(DPOINT3D));
            pPolyPoints = (HPDPOINT3D)GlobalLock (*pHandle); 
            if (hSavePolyElev)
            	pElev = (HPFLOAT)GlobalLock (hSavePolyElev);
            for (i=0;i<nSavePoly;i++)
            { 
            	pPolyPoints[i] = DPointToDPoint3D (lpDpoint[i]);
   	            if (hSavePolyElev)  
   	            	pPolyPoints[i].z = pElev[i];
            }
            if (PickList[0].Type == 3 && ldistpp ((HPDPOINT)&pPolyPoints[0],(HPDPOINT)&pPolyPoints[nSavePoly-1]) > P_TOL)
            {   
            	(*pnPnts)++;
            	pPolyPoints[nSavePoly] = pPolyPoints[0];
            }   
            GlobalUnlock (*pHandle); 
            GlobalUnlock (hSavePoly);
            if (hSavePolyElev)
            	GlobalUnlock (hSavePolyElev);
			DestroySavedPolys ();  
			if (Reverse)
				*pHandle = ReversePoints3D (*pnPnts,*pHandle);
            rtn = TRUE;
        }
        else
			rtn = FALSE;
	} 
	CurTheme = SaveTheme;
	SetCurView ( SaveVP);
	return rtn;
}

short GetPolyPoints2 (LPPICKDATAHEADER PickData,LPINT pNumPoints, LPHANDLE phPoints,short MaxLoops)
{
	LPTHEME	pTheme, SaveTheme=CurTheme; 
	LPVIEWPORT	SaveVP=CurView;
	HPDPOINT pPolyPoints,lpDPoint;  
	LPINT	pPolyParts;
	short	SavePass; 
	int		nLoops=0, nParts; 
	int		nPParts;    
	BOOL	FirstLoop;
	
	*(LPPICKDATAHEADER)&PickList[0] = *PickData;
	if (PickList[0].Type == 2 || PickList[0].Type == 3 || PickList[0].Type == 5)
	{   
	    SetConfig (PickList[0].ConfigID);
	    SetViewport (PickList[0].ViewID);
		pTheme = AddTheme (GF_SAVEPOLYPARTS_THEME); 
		SavePass = CurView->PassID;
		CurView->PassID = 4;
		ProcessSelectedTheme = CurView->NumThemes;
		ProcessPickedItem (0,FALSE);
		CurView->PassID = SavePass; 
		ProcessSelectedTheme = 0;       		
		DeleteTheme (pTheme); 
		nParts = GetSavedPolys ();
		if (hSavePoly)
		{   LPMNMXCORD lpRect;
			HPDPOINT	lpDpoint;
			short	nareas;
	    		    
            if (hSavePolyParts)
            {
            	pPolyParts = (LPINT)GlobalLock (hSavePolyParts);
            	nareas = *pPolyParts++; 
            }
            else
            { 
                nareas=1; 
                nPParts = nSavePoly; 
                pPolyParts = &nPParts;
            }
			nLoops = nareas;				                            
            lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
            lpDpoint = (LPDPOINT) (lpRect+1);  
            FirstLoop = TRUE; 
            if (MaxLoops)
	            while (nareas--)
	            {   
	            	DPOINT	FirstPoint, LastPoint;   
	            	HPDPOINT	pNewPoint;
								                        	
	            	*pNumPoints = *pPolyParts;
	            	*phPoints= GSSiGlobAlloc ( 421,GMEM_MOVEABLE,(long)*pPolyParts*sizeof(DPOINT));
	            	pNewPoint = (HPDPOINT)GlobalLock (*phPoints);
	                while ((*pPolyParts)--)
	                    *pNewPoint++ = *lpDpoint++;
	                GlobalUnlock (*phPoints++);
	                pPolyParts++; 
	                pNumPoints++; 
	                if (!FirstLoop)
	                	lpDpoint++;
	                FirstLoop = FALSE;
	            }


/*            if (PickList[0].Type == 3 && ldistp (pPolyPoints[0],pPolyPoints[nSavePoly-1]) > P_TOL)
            {   
            	(*pnPnts)++;
            	pPolyPoints[nSavePoly] = pPolyPoints[0];
            }*/     
            if (hSavePolyParts)
            	GlobalUnlock (hSavePolyParts);
			GlobalUnlock (hSavePoly);
			DestroySavedPolys ();  
//			if (Reverse)
//				*pHandle = ReversePoints (*pnPnts,*pHandle);
        }
	} 
	CurTheme = SaveTheme;
	SetCurView ( SaveVP);
	return nLoops;
}

int GetPolyPointsWithParts (LPPICKDATAHEADER PickData,LPLONG pnPnts,LPHANDLE phPoints,LPHANDLE phPolyPartLen)//returns npoly, all points in phPoints preceeded by bounds
{
	LPTHEME	pTheme, SaveTheme=CurTheme; 
	LPVIEWPORT	SaveVP=CurView;
	HPDPOINT pPolyPoints,lpDPoint;  
	LPINT	pPolyParts;
	short	SavePass; 
	int		nLoops=0, nParts; 
	int		nPParts;    
	BOOL	FirstLoop;
	
	*phPolyPartLen = 0;
	*(LPPICKDATAHEADER)&PickList[0] = *PickData;
	if (PickList[0].Type == 2 || PickList[0].Type == 3 || PickList[0].Type == 5)
	{   
	    SetConfig (PickList[0].ConfigID);
	    SetViewport (PickList[0].ViewID);
		pTheme = AddTheme (GF_SAVEPOLYPARTS_THEME); 
		SavePass = CurView->PassID;
		CurView->PassID = 4;
		ProcessSelectedTheme = CurView->NumThemes;
		ProcessPickedItem (0,FALSE);
		CurView->PassID = SavePass; 
		ProcessSelectedTheme = 0;       		
		DeleteTheme (pTheme); 
		nParts = GetSavedPolys ();
		if (hSavePoly)
		{   LPMNMXCORD lpRect;
			HPDPOINT	lpDpoint;

            *pnPnts = nSavePoly; 
            if (hSavePolyParts)
            {
            	pPolyParts = (LPINT)GlobalLock (hSavePolyParts);
            	nLoops = *pPolyParts++; 
				GlobalUnlock (hSavePolyParts);
				*phPolyPartLen = hSavePolyParts;
				hSavePolyParts = 0;
            }
            else
            { 
                nLoops=1; 
            }
			*phPoints = hSavePoly;
			hSavePoly = 0;
        }
	} 
	CurTheme = SaveTheme;
	SetCurView ( SaveVP);
	return nLoops;
}

long GetPointsBetweenDist (long nPnts,HPDPOINT Points,double FromDist, double ToDist, HPDPOINT OutPoints)
{
	long	nOutPnts=0;
	double	dist=0;
	double	FDist = min (FromDist, ToDist);
	double	TDist = max (FromDist, ToDist);
	DPOINT	LastPoint = *Points++;
	
	nPnts--;
	while (nPnts--)
	{       
		dist += ldistp (LastPoint,*Points);
		if (dist - P_TOL > FDist && dist + P_TOL < TDist)
			OutPoints[nOutPnts++] = *Points;
		LastPoint = *Points++;
	}
	if (ToDist < FromDist)
		ReversePoints2 (nOutPnts,OutPoints);
	return nOutPnts;
} 

BOOL RecoverPLTFromRIN (LPSTR PltFile)
{
	REFINDEXDATA	RefIdxData;
	char	RINFile[MAX_PATH];
	HANDLE	hRefIdx;  
	long	TotRecs=0, Refno;
	LPSTR	pDot;  
	short	pos=BT_FIRST;
	
	_fstrcpy (RINFile,PltFile);
	pDot = _fstrrchr (RINFile,'.');
	_fstrcpy (pDot,".rin");
	
	hRefIdx = BT_OPEN (RINFile, 0, BT_READ, 0);  
	while (!BT_FIND (hRefIdx,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&RefIdxData))
	{
		pos=BT_NEXT; 
		if (RefIdxData.Segment < 21000)
			TotRecs++;
	}
	BT_CLOSE (hRefIdx);
	return TRUE;
}

void CreateTranRec (LPSTR Coords,MNMXCORD MinMaxD,LPMNMXCORD MinMaxCoord)
{
	char	mmcc1[32],mmcc2[32],mmcc3[32],mmcc4[32];
			
	sprintf (mmcc1,"%16f",MinMaxCoord->xmn);
	mmcc1[16] = 0;
	sprintf (mmcc2,"%16f",MinMaxCoord->xmx);
	mmcc2[16] = 0;
	sprintf (mmcc3,"%16f",MinMaxCoord->ymn);
	mmcc3[16] = 0;
	sprintf (mmcc4,"%16f",MinMaxCoord->ymx);
	mmcc4[16] = 0;
    sprintf (Coords,"%16f%16f%16f%16f%16s%16s%16s%16s",
                MinMaxD.xmn,  
                MinMaxD.xmx,  
                MinMaxD.ymn,  
                MinMaxD.ymx, 
				mmcc1,
				mmcc2,
				mmcc3,
				mmcc4);
	return;
}

BOOL CreateNewMap (LPSTR NewName,LPMNMXCORD MinMaxCoord,short NumSyms,HANDLE hSymDesc,
                    short NumPens,LPPENDESC pPenDesc,long StartTime,long EndTime,BOOL ClearBounds)
{   
    LPSYMDESC pSymDesc;
    LPSYMBOL    pSymbol; 
    char    Name[MAX_PATH], Coords[130];  
    long    NewPrimeOffset;
    HANDLE  hMem;
    LPSHORT   pInt;  
    HFILE   Fid;
    OFSTRUCTGM    OFStruct;
    short     lMem, Signature, Version, i2, i, n, NumParent;  
    MNMXCORD    MinMaxD;
    mnmxCor     MinMax;  
    double  Hinc, Vinc;
    PRIMEOFFSETS NewPrimeOffs;
    char	Drive[4], Dir[MAX_PATH],name[34],Ext[8], TempName[MAX_PATH];
    double	MinSize = GetGlobalDVal2 ("[%MINMAPWIDTH]",1000000 * P_TOL)/2;
    
    if (MinMaxCoord->xmn == MinMaxCoord->xmx)
    {
    	MinMaxCoord->xmn -= MinSize;
    	MinMaxCoord->xmx += MinSize;
    }  
    if (MinMaxCoord->ymn == MinMaxCoord->ymx)
    {
    	MinMaxCoord->ymn -= MinSize;
    	MinMaxCoord->ymx += MinSize;
    }  
    if (MinMaxCoord->xmn >= MinMaxCoord->xmx ||  
    	MinMaxCoord->ymn >= MinMaxCoord->ymx)
    {   
    	GSSiMsgBox (GetFocus(),"Unable to create output map\r\nInvalid coordinate extents",0,MB_ICONEXCLAMATION,0);
    	return FALSE;
    }  
    NewPrimeOffs.Code101 = 101;                 
    NewPrimeOffs.Code102 = 102;                 
    NewPrimeOffs.Code103 = 103;                 
    NewPrimeOffs.Code200 = 200;                 
    NewPrimeOffs.Code201 = 201;
    NewPrimeOffs.DescBlockOffset = 0;                 
    NewPrimeOffs.Term = 0;              
    NewPrimeOffs.Code202 = 202;
    if (!EndTime)
    {                 
	    NewPrimeOffs.MinTime=0;
	    NewPrimeOffs.MaxTime=LONG_MAX; 
	}
	else
    {                 
	    NewPrimeOffs.MinTime=StartTime;
	    NewPrimeOffs.MaxTime=EndTime; 
	}
    
    _fstrcpy (Name,NewName);
    ExpandText (Name);     
    if (!*Name)
		return FALSE;
    if (!makedirectories (Name,FALSE,TRUE))
    	return FALSE;
    Fid = GSSiOpenFile (Name,&OFStruct,OF_CREATE);   
    if (Fid == HFILE_ERROR)
    	return FALSE;
    _splitpath (Name,Drive,Dir,name,Ext);
    sprintf (TempName,"%s%s%s.rin",Drive,Dir,name);
    GSSiRemove (TempName);
    sprintf (TempName,"%s%s%s.tin",Drive,Dir,name);
    GSSiRemove (TempName);
    
    
    Hinc = (MinMaxCoord->xmx - MinMaxCoord->xmn) * 0.025;
    Vinc = (MinMaxCoord->ymx - MinMaxCoord->ymn) * 0.025; 
    MinMaxCoord->xmn -= Hinc;
    MinMaxCoord->xmx += Hinc;
    MinMaxCoord->ymn -= Vinc;
    MinMaxCoord->ymx += Vinc;
    if (MinMaxCoord->xmx - MinMaxCoord->xmn > MinMaxCoord->ymx - MinMaxCoord->ymn)
    {
        MinMaxD.xmn = -32000;   
        MinMaxD.xmx = 32000;
        MinMaxD.ymn = MinMaxD.xmn * (MinMaxCoord->ymx - MinMaxCoord->ymn) /
                                    (MinMaxCoord->xmx - MinMaxCoord->xmn);
        MinMaxD.ymx = MinMaxD.xmx * (MinMaxCoord->ymx - MinMaxCoord->ymn) /
                                    (MinMaxCoord->xmx - MinMaxCoord->xmn);
    }
    else
    {   
        MinMaxD.ymn = -32000;   
        MinMaxD.ymx = 32000;    
        MinMaxD.xmn = MinMaxD.ymn * (MinMaxCoord->xmx - MinMaxCoord->xmn) /
                                    (MinMaxCoord->ymx - MinMaxCoord->ymn);
        MinMaxD.xmx = MinMaxD.ymx * (MinMaxCoord->xmx - MinMaxCoord->xmn) /
                                    (MinMaxCoord->ymx - MinMaxCoord->ymn);
    }  
    MinMax.xmn = IDNINT (MinMaxD.xmn);
    MinMax.ymn = IDNINT (MinMaxD.ymn);
    MinMax.xmx = IDNINT (MinMaxD.xmx);
    MinMax.ymx = IDNINT (MinMaxD.ymx);
    NewPrimeOffs.TranPointOffset = GSSillseek (Fid,0,1);   
    lMem = 16*4*2 + 2 + 2 ;  
    BigWrite (Fid,(HPSTR)&lMem,2,-1);
	CreateTranRec (Coords,MinMaxD,MinMaxCoord);
    BigWrite (Fid,Coords,128,-1);
    i2 = 0;  
    BigWrite (Fid,(char *)&i2,2,-1); 
    n = NumSyms; 
    NumParent = 0;
    if (hSymDesc)
    {
	    pSymDesc = (LPSYMDESC)GlobalLock (hSymDesc);
	    while (n--)
	    {   
			if (!pSymDesc->Handle)
			    pSymDesc->Handle = GetDictSymDesc (pSymDesc->Number,0); 
	        pSymbol = (LPSYMBOL)GlobalLock (pSymDesc->Handle);
	        if (!pSymbol->Type)
	            NumParent++;
	        GlobalUnlock (pSymDesc->Handle);
	    	DestroySymbol (pSymDesc->Handle); 
	    	pSymDesc++->Handle = 0;
	    }
	    GlobalUnlock (hSymDesc); 
	}
    NewPrimeOffs.UsedDescOffset = GSSillseek (Fid,0,1);    

	WriteSymList (Fid, NumParent, NumSyms,hSymDesc);
	    
    lMem = NumPens*10+2+2;
    hMem = GSSiGlobAlloc ( 445,GHND,lMem);
    pInt = (LPSHORT)GlobalLock (hMem);
    *pInt++ = NumPens*10+2;  
    while (NumPens--)
    {
        *pInt++ = 1;  
        *pInt++ = pPenDesc->PenNum;
        *pInt++ = GetRValue(pPenDesc->Color);
        *pInt++ = GetGValue(pPenDesc->Color);
        *pInt++ = GetBValue(pPenDesc++->Color);
    }
    *pInt = 0;
    GlobalUnlock (hMem); 
    NewPrimeOffs.ColorPaletteOffset = GSSillseek (Fid,0,1);    
    pInt = (LPSHORT)GlobalLock (hMem);
    BigWrite (Fid,(char *)pInt,lMem,-1);  
    GSSiGlobUlFree (&hMem); 
    MaxNewType = 2;
    NewPrimeOffs.GraphicsOffset=AddSimpleQuadTree (Fid);
    
    NewPrimeOffset = GSSillseek (Fid,0,1); 
    i2 = sizeof(NewPrimeOffs);
    BigWrite (Fid,(char *)&i2,2,-1);
    BigWrite (Fid,(char *)&NewPrimeOffs,sizeof(NewPrimeOffs),-1);
    BigWrite (Fid,(char *)&NewPrimeOffset,4,-1);
    BigWrite (Fid,(char *)&MinMax,8,-1);
    Signature = 32349;       
    BigWrite (Fid,(char *)&Signature,2,-1);
    Version = 8;            
    BigWrite (Fid,(char *)&Version,2,-1);
    Version = 0;
    BigWrite (Fid,(char *)&Version,2,-1);
    GSSiClose (Fid);  
    
    if (ClearBounds)
    {
	    ClearAllBounds();  
	    _fstrcpy (PltName,NewName);
	    OpenMap (0,CurView->hDC);
		EditBounds = CurView->FileMNMX;
	    CloseMap (FALSE);
	}
	CurView->FileProjectionType=0;
    return TRUE;
}   

BOOL WriteSymList (HFILE Fid, short NumParent, short NumSyms, HANDLE hSymDesc)
{ 
    LPSYMDESC pSymDesc;
    LPSYMBOL    pSymbol; 
	short	i2, n,ActualNum=0;
	long	lMem, loc, RtnLoc;
	
    lMem = (1+NumSyms)*36+2+2+2+2+2;  
    BigWrite (Fid,(char *)&lMem,2,-1);
    i2 = 11; 
    BigWrite (Fid,(char *)&i2,2,-1);
    i2 = NumSyms-NumParent; 
    loc = GSSillseek (Fid,0,1);
    BigWrite (Fid,(char *)&i2,2,-1); 
    if (hSymDesc)
    { 
	    pSymDesc = (LPSYMDESC)GlobalLock (hSymDesc);
	    n=NumSyms;
	    while (n--)
	    {   
	    	if (!pSymDesc->Handle)
			    pSymDesc->Handle = GetDictSymDesc (pSymDesc->Number,0); 
	        pSymbol = (LPSYMBOL)GlobalLock (pSymDesc->Handle);
	        if (pSymbol->Type)
	        {
	            BigWrite (Fid,(char *)&pSymDesc->Number,2,-1);
	            BigWrite (Fid,(char *)&pSymbol->Parent,2,-1);
	            BigWrite (Fid,(char *)&pSymbol++->Name,32,-1); 
	            ActualNum++;
	        } 
			GlobalUnlock (pSymDesc->Handle);
	    	DestroySymbol (pSymDesc->Handle); 
	    	pSymDesc++->Handle = 0;
	    }
	    GlobalUnlock (hSymDesc); 
	}
    i2 = NumParent; 
    RtnLoc = GSSillseek (Fid,0,1); 
    GSSillseek (Fid,loc,0);
    BigWrite (Fid,(HPSTR)&ActualNum,2,-1);
    GSSillseek (Fid,RtnLoc,0);
    BigWrite (Fid,(char *)&i2,2,-1);  
    ActualNum = 0;  
    if (hSymDesc)
    {
	    pSymDesc = (LPSYMDESC)GlobalLock (hSymDesc);
	    n=NumSyms;
	    while (n--)
	    {   
	    	if (!pSymDesc->Handle)
			    pSymDesc->Handle = GetDictSymDesc (pSymDesc->Number,0); 
			if (pSymDesc->Handle)
			{
		        pSymbol = (LPSYMBOL)GlobalLock (pSymDesc->Handle);
		        if (!pSymbol->Type)
		        {
		            BigWrite (Fid,(char *)&pSymDesc->Number,2,-1);
		            BigWrite (Fid,(char *)&pSymbol->Parent,2,-1);
		            BigWrite (Fid,(char *)&pSymbol++->Name,32,-1);
		            ActualNum++;
		        }
		        GlobalUnlock (pSymDesc->Handle); 
		    	DestroySymbol (pSymDesc->Handle); 
		    }
	    	pSymDesc++->Handle = 0;
	    }
	    GlobalUnlock (hSymDesc);
	}
    i2 = 0;  
    BigWrite (Fid,(char *)&i2,2,-1); 
    BigWrite (Fid,(char *)&i2,2,-1);
    loc = GSSillseek (Fid,0,1); 
    GSSillseek (Fid,RtnLoc,0);
    BigWrite (Fid,(HPSTR)&ActualNum,2,-1);
    GSSillseek (Fid,loc,0);
    return TRUE; 
}

void AddToSymList (int idesc,LPSHORT NumSyms, LPHANDLE hSymDesc)
{
    LPSYMDESC   pSymDesc;
    LPSYMBOL    pSymbol; 
    short     i; 
    
    if (!idesc)
    	return;
Top:if (*hSymDesc)
    {
        pSymDesc = (LPSYMDESC)GlobalLock(*hSymDesc);
        for (i=0;i<*NumSyms;i++,pSymDesc++)
            if (idesc == pSymDesc->Number)
            {
                GlobalUnlock (*hSymDesc);
                return;
            }
        GlobalUnlock (*hSymDesc);
        (*NumSyms)++;
//        *hSymDesc = GlobalReAlloc (*hSymDesc,(long)*NumSyms*sizeof(SYMDESC),0); 
    }
    else                                        
    {
        *NumSyms=1;
        *hSymDesc = GSSiGlobAlloc ( 446,GMEM_MOVEABLE,3200*sizeof(SYMDESC));
    }
    pSymDesc = (LPSYMDESC)GlobalLock(*hSymDesc);
    pSymDesc += *NumSyms-1;
    pSymDesc->Number = idesc;
    pSymDesc->Handle = GetDictSymDesc (idesc,0); 
    if (pSymDesc->Handle)
    {  
    	pSymbol = (LPSYMBOL) GlobalLock (pSymDesc->Handle);
    	idesc = pSymbol->Parent;
    	GlobalUnlock (pSymDesc->Handle); 
    	DestroySymbol (pSymDesc->Handle); 
    	pSymDesc->Handle = 0;
    }
    else
    {
    	idesc = 0;
        (*NumSyms)--;
    }
    GlobalUnlock (*hSymDesc);    
    if (idesc) goto Top;  
    if (!*NumSyms)
    	GSSiGlobFree (hSymDesc);
    return;
}

void DestroySymList (LPSHORT NumSyms,LPHANDLE hSymDesc)
{ 
    LPSYMDESC   pSymDesc;
    LPSYMBOL    pSymbol; 
    
    if (!*hSymDesc) return; 
    
    pSymDesc = (LPSYMDESC) GlobalLock(*hSymDesc);
    while ((*NumSyms)--) 
    {
		DestroySymbol (pSymDesc->Handle); 
        pSymDesc++;
    }   
    GSSiGlobUlFree (hSymDesc);  
    return;
}

HRGN GetVPRgn (BOOL Invert,LPRECT pClipRect)
{   
	HRGN	NewRgn=0, OvrLapRgn, MaskRgn, hRgn; 
    short	TypeRegion;
    
	if (IsRectEmpty (pClipRect))
		return 0;
	if (Invert)
		return (CreateRectRgnIndirect (&CurView->Rect));
	if (*MaskAreaFile)
		NewRgn = SetOrthoMask (MaskAreaFile);
	*MaskAreaFile = 0;
	if (CurView->Transparent && CurView->hMaskArea)
	{
		LPDPOINT	lpBasePoint;
		HANDLE		hPoints;
		UINT		i, Nump=0;
		LPPOINT		Points;
		LPMNMXCORD	lpRect;
		
	    lpRect = (LPMNMXCORD) GlobalLock (CurView->hMaskArea);
		if (RectInWBounds (lpRect,1))  
			Nump=CurView->NumMaskPoints; 
		if (Nump)
		{ 
			lpRect++;
			lpBasePoint = (LPDPOINT) lpRect;
		    hPoints = GSSiGlobAlloc (  44,GMEM_MOVEABLE,(Nump+1)*sizeof(POINT));
		    Points = (HPPOINT) GlobalLock (hPoints);
		    for (i=0;i<Nump;i++,lpBasePoint++)
				Points[i] = BasePtToWinPt(lpBasePoint);
			MaskRgn = CreatePolygonRgn (Points,Nump,ALTERNATE);
			GSSiGlobUlFree (&hPoints); 
			if (NewRgn) 
				TypeRegion = CombineRgn (NewRgn,MaskRgn,NewRgn,RGN_AND);
			else
				NewRgn = MaskRgn;
		    OvrLapRgn = CreateRectRgnIndirect (&CurView->ScreenRect);  
			TypeRegion = CombineRgn (NewRgn,OvrLapRgn,NewRgn,RGN_AND);
	        DeleteObject (OvrLapRgn);
		} 
		GlobalUnlock (CurView->hMaskArea);
		return NewRgn;
	}
	hRgn = CreateRectRgnIndirect (pClipRect);
	if (NewRgn) 
	{
		int RgnType = CombineRgn (hRgn,NewRgn,hRgn,RGN_AND);  
        DeleteObject (NewRgn); 
    }
	return hRgn;
	
}

HRGN CreateVPRgn (BOOL Invert,BOOL InReset)
#if ENABLETRACE
{GSSiEnterProg (11);
#endif
{   HRGN    NewRgn, OvrLapRgn, ShadowRgn; 
    RECT    NewRect, VPRect, OVRect, ClipRect;
    short     iview, StartVP,TypeRegion;     
    BOOL	SkipOverlaps=FALSE;
    LPVIEWPORT  SaveVP;
    static	BOOL	testspeed=FALSE;
	static	int	ncalls=0;
	
	if (!CurView)
		return 0;

	//if (BufferedScreen && hDCScreenBuffer == CurView->hDC)
	//	return 0;

//    if (Printing)
//    	return 0;
//return 0; 
	if (testspeed)
		return 0;
	ncalls++;
    if (InReset || Invert == 2)
    {
	    ClipRect = CurView->Rect;
    	VPRect = CurView->Rect; 
	    Invert = 0;
	}
	else 
	{
	    ClipRect = CurView->ScreenRect;
	    VPRect = CurView->ScreenRect;
	} 
/*    if (Invert)
    {
    	NewRgn = CreateRectRgnIndirect (&CurView->Rect);  
	    OvrLapRgn = CreateRectRgnIndirect (&MainRect);  
		CombineRgn (NewRgn,OvrLapRgn,NewRgn,RGN_DIFF);
        DeleteObject (OvrLapRgn);
{
#if ENABLETRACE
GSSiExitProg (11);
#endif
	    return(NewRgn);
}
    } */
	if (InReset)
		InflateRect (&ClipRect,1,1);
	else
	{
		ClipRect.right++;
		ClipRect.bottom++;
	}
    SaveVP = CurView;
    StartVP = 0; 
    for (iview=0;iview<*pNumViewports;iview++)
    { 
    	if (pViewports[iview]->ID != CurView->ID && pViewports[iview]->DisplayedFullScreen)
    	{
    		if (CurView->DisplayInParent)
    		{
    			if (CurView->Parent == pViewports[iview]->ID)  
    			{   
    				SkipOverlaps = TRUE;
    				ClipRect = pViewports[iview]->ScreenRect;
    				break;                                 
    			}
    		}
    		SetRectEmpty (&ClipRect);
    	}
    }
    while (StartVP < *pNumViewports && pViewportsD[StartVP]->ID != CurView->ID) //pViewportsD[0]
    	StartVP++;
    StartVP++;
    if (CurView->DisplayedFullScreen)
    	StartVP = *pNumViewports; 
    if (Invert)
    {
//    	NewRgn = CreateRectRgnIndirect (&CurView->Rect);     
		NewRgn = GetVPRgn (Invert,&ClipRect);
		if (NewRgn)
		{
		    OvrLapRgn = CreateRectRgnIndirect (&MainClipRect);  
			TypeRegion = CombineRgn (NewRgn,OvrLapRgn,NewRgn,RGN_DIFF);
	        DeleteObject (OvrLapRgn);
	    }
	    else
	    	NewRgn = CreateRectRgnIndirect (&MainClipRect); 
    }
    else
    {
//    	NewRgn = CreateRectRgnIndirect (&ClipRect); 
		NewRgn = GetVPRgn (Invert,&ClipRect);
    	if (CurrentConfig)
    	{   
    		RECT	MCR=MainClipRect;
    		
    		MCR.right++;
    		MCR.bottom++;
    		if (NewRgn)
    		{
			    OvrLapRgn = CreateRectRgnIndirect (&MCR);  
				TypeRegion = CombineRgn (NewRgn,OvrLapRgn,NewRgn,RGN_AND);
		        DeleteObject (OvrLapRgn);
		    }
		    else
		    	NewRgn = CreateRectRgnIndirect (&MCR); 
	    }
    }
    if (!SkipOverlaps && !SaveVP->DisplayedFullScreen && !ComputePCTTheme && (InReset || CurView->Type>-1))
	for (iview=StartVP;iview<*pNumViewports;iview++)
    {   
        SetCurView ( pViewportsD[iview]);
        if (CurViewActive() && !CurView->DisplayInParent && !CurView->Transparent)
        {  
        	if (!SaveVP->DisplayInParent || CurView->ID != SaveVP->Parent)
        	{  
	            OVRect = CurView->Rect;
				OVRect.right++;
				OVRect.bottom++;
		        if ((OvrLapRgn = GetVPRgn (FALSE,&OVRect)))
		        {
		            if (IntersectRect (&NewRect,&VPRect,&CurView->Rect)) 
		                TypeRegion = CombineRgn (NewRgn,NewRgn,OvrLapRgn,RGN_DIFF);
		            GSSiDeleteObject (&OvrLapRgn);
		        } 
		    }
        }
    } 
    if (hLastBox)
    {
    	if (!ScreenIsRegistered(hLastBox,0))
    		hLastBox = 0; 
    	else
		{
			LPSAVESCREEN	pSaveScreen=(LPSAVESCREEN)GlobalLock (hLastBox);
			HRGN hBoxRgn=CreateRectRgnIndirect (&pSaveScreen->Rect);		
	
			GlobalUnlock (hLastBox);
	        TypeRegion = CombineRgn (NewRgn,NewRgn,hBoxRgn,RGN_DIFF);
	        DeleteObject (hBoxRgn);
		} 
	}
    if (pZoomOutExclusionArea)
    {
		HRGN hBoxRgn=CreateRectRgnIndirect (pZoomOutExclusionArea);		
	
        TypeRegion = CombineRgn (NewRgn,NewRgn,hBoxRgn,RGN_DIFF);
	    DeleteObject (hBoxRgn);
	}
    SetCurView (SaveVP);
//	NewRgn = 0;
{
#if ENABLETRACE
GSSiExitProg (11);
#endif
    return(NewRgn);
}
#if ENABLETRACE
}
#endif
}

int GetNameParts (LPSTR Field,LPSTR *pLoc,LPINT pLen,int MaxParts)
#if ENABLETRACE
{GSSiEnterProg (1399);
#endif
{
	int	nParts=0;
	LPSTR	pWord = Field, pEnd;

	strupr (Field);
	ReplaceChar (Field,'-',' '); 
	ReplaceChar (Field,'/',' '); 
	ReplaceChar (Field,'(',' '); 
	ReplaceChar (Field,')',' '); 
	ReplaceChar (Field,'&',' '); 
	ReplaceChar (Field,',',' '); 
	ReplaceChar (Field,'\'',' '); 
	ReplaceChar (Field,'#',' '); 
	ReplaceChar (Field,'"',' '); 
	ReplaceChar (Field,'.',' '); 
	OneSpace (Field);  
	while (*pWord && nParts < MaxParts)
	{   
		if (*pWord == ' ')
			pWord++;
		if ((pEnd = _fstrchr (pWord,' ')))
			*pEnd++ = 0;
		else
			pEnd = _fstrchr (pWord,0);
		*(pLoc + nParts) = pWord;
		*(pLen + nParts++) = strlen (pWord);
		pWord = pEnd;
	}
{
#if ENABLETRACE
GSSiExitProg (1399);
#endif
    return nParts;
}
#if ENABLETRACE
}
#endif
}

BOOL CreateWordIndex (LPSTR FromFile,LPSTR FromField,LPSTR ToFile)
#if ENABLETRACE
{GSSiEnterProg (1400);
#endif
{   
	char	DefStr[]="Word(C8),Sequence(B2),nOffsets(B2),Offsets(C400)"; 
	char	DefStrTmp[128];
	short	NumFields=4, NumIndexFields=2;
	long	Offset, nRecs, nLoaded=0;
	BOOL	st, rtn=FALSE; 
	HANDLE	hDB=0,hDBOut=0, hDBOutTmp=0;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr;
	LPGWDHEADER lpGWDHead;
    LPOPENFILEDATA  FilePtrTmp;
    LPOPENSQLDATA   SQLPtrTmp;
	LPGWDHEADER lpGWDHeadTmp;
	short	WordMax = 8; 
	char	Field[256],Word[256];  
	LPWORDINDEX	pRec, pRecTmp;
	UINT	len;     
	char	ToFileTmp[256];
	LPSTR	pEnd;
	LPSTR	pWord[32];
	int		WordLen[32];
	int		nParts, iword,ii;
	
	_fstrcpy (DefStrTmp,DefStr);
    if (!OpenDataFile (FromFile,"",BT_READ,&hDB))
    	goto Exit;
    _fstrcpy (ToFileTmp,ToFile); 
    pEnd = _fstrrchr (ToFileTmp,'.');
    if (!pEnd)
    	pEnd = _fstrchr (ToFileTmp,0);
    _fstrcpy (pEnd,"tmp.gmd");
	if (!CreateGWDDatabase (ToFileTmp,1,FALSE,NumFields,NumIndexFields,DefStrTmp))
	{
		CloseDataFile (TRUE,&hDB); 
		goto Exit; 
	}  
    SQLPtr = (LPOPENSQLDATA) GlobalLock (hDB);
    FilePtr = (LPOPENFILEDATA) GlobalLock (SQLPtr->OFHandle);
//	GMDSwitchToMemFile (FilePtr->FileHandle,0,0);
	GlobalUnlock (SQLPtr->OFHandle);
	GlobalUnlock (hDB);
	OpenDataFile (ToFileTmp,"",BT_WRITE,&hDBOutTmp);
    SQLPtrTmp = (LPOPENSQLDATA) GlobalLock (hDBOutTmp);
    FilePtrTmp = (LPOPENFILEDATA) GlobalLock (SQLPtrTmp->OFHandle);
	GMDSwitchToMemFile (FilePtrTmp->FileHandle,50000000,10000000);
	lpGWDHeadTmp = (LPGWDHEADER)GlobalLock (FilePtrTmp->FileHandle);                             
    pRecTmp = (LPWORDINDEX)&lpGWDHeadTmp->GWDData;
	nRecs = NumSQLRows (hDB);
	CreateStatusWind (hWndMain,1,"Create Word Index");
	StatusWindowUpdate (0,"Step 1", nRecs, 0);
    while (FetchDBRec (hDB) && ContinueProcessing)
    {   
    	LPOPENSQLDATA   SQLPtr = (LPOPENSQLDATA) GlobalLock (hDB);
	    LPOPENFILEDATA  FilePtr = (LPOPENFILEDATA) GlobalLock (SQLPtr->OFHandle);
	    long	FileOffset=SQLPtr->Offset;
    	
    	GlobalUnlock (SQLPtr->OFHandle);
    	GlobalUnlock (hDB);
		strcpy (Field,FromField);
		ExpandText (Field); 
		nParts = GetNameParts (Field,pWord,WordLen,32);
		for (iword=0;iword < nParts;iword++)
		{   
			if (WordLen[iword] > 1)
			{
				_fstrncpy (pRecTmp->Word,pWord[iword],WordMax);  
				pRecTmp->Seq = 0;
NextSeq:		pRecTmp->Seq--;  
				if (-pRecTmp->Seq <= MAXWISEQ)
				{
					GWDFormKey(lpGWDHeadTmp,0,TRUE,0,0);
					if (!BT_FIND (lpGWDHeadTmp->BTHandle[0],lpGWDHeadTmp->pKeys[0],BT_FIRST,BT_EQ, (LPSTR)&Offset))
					{
						len=FillGWDData (lpGWDHeadTmp,Offset);
						if (pRecTmp->nOffsets == MAXWIOFFSETS)
			        		goto NextSeq;
						pRecTmp->Offsets[pRecTmp->nOffsets++] = FileOffset;
						if (!FileOffset)
							ii=1;
						GWDReplaceRecord (lpGWDHeadTmp,0,0,Offset);
					}
					else
					{   
						_fmemset (pRecTmp->Offsets,0,sizeof(pRecTmp->Offsets)); 
						pRecTmp->nOffsets = 0;
						pRecTmp->Offsets[pRecTmp->nOffsets++] = FileOffset;
						if (!FileOffset)
							ii=1;
						GWDAddRecord (lpGWDHeadTmp,0,0);
					} 
				}
			}
		}
Next:
		ContinueProcessing = StatusWindowUpdate (0,0, nRecs, ++nLoaded);
	}
	if (ContinueProcessing)
	{   
		short	pos=BT_FIRST;
		
		CreateGWDDatabase (ToFile,1,FALSE,NumFields,NumIndexFields,DefStr);
		OpenDataFile (ToFile,"",BT_WRITE,&hDBOut);
	    SQLPtr = (LPOPENSQLDATA) GlobalLock (hDBOut);
	    FilePtr = (LPOPENFILEDATA) GlobalLock (SQLPtr->OFHandle);
		lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle);                             
	    pRec = (LPWORDINDEX)&lpGWDHead->GWDData; 
	    nRecs = BT_NUM_IN_INDEX (lpGWDHeadTmp->BTHandle[0]);     
	    nLoaded = 0;
   		StatusWindowUpdate (0,"Step 2", nRecs, nLoaded);
	    while (!BT_FIND (lpGWDHeadTmp->BTHandle[0],lpGWDHeadTmp->pKeys[0],pos,BT_ANY, (LPSTR)&Offset))
	    {   
	    	pos = BT_NEXT;
	        len=FillGWDData (lpGWDHeadTmp,Offset);
	        if (pRecTmp->Seq <= -MAXWISEQ) 
	        {
	        	while (pRecTmp->Seq < -1) 
	        	{
	        		BT_FIND (lpGWDHeadTmp->BTHandle[0],lpGWDHeadTmp->pKeys[0],pos,BT_ANY, (LPSTR)&Offset);
			        len=FillGWDData (lpGWDHeadTmp,Offset); 
			        nLoaded++;
			    }
	        }
	        else
	        {
	        	*pRec = *pRecTmp;
	        	len = 12 + pRec->nOffsets * sizeof (long);
		        GWDAddRecord (lpGWDHead,len,0); 
		    }
    		StatusWindowUpdate (0,0, nRecs, ++nLoaded);
		}
		GlobalUnlock (FilePtr->FileHandle);  
		GlobalUnlock (SQLPtr->OFHandle);
		GlobalUnlock (hDBOut);
		CloseDataFile (TRUE,&hDBOut); 
	}
    ContinueProcessing = TRUE;
	DestroyStatusWindow(0); 
	GlobalUnlock (FilePtrTmp->FileHandle);  
	GlobalUnlock (SQLPtrTmp->OFHandle);
	GlobalUnlock (hDBOutTmp);
	CloseDataFile (TRUE,&hDBOutTmp); 
	GSSiRemove (ToFileTmp);
    pEnd = _fstrrchr (ToFileTmp,'.');
    _fstrcpy (pEnd,".in1");
	GSSiRemove (ToFileTmp);
	CloseDataFile (TRUE,&hDB); 
	rtn = TRUE;
Exit:
{
#if ENABLETRACE
GSSiExitProg (1400);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

DPOINT ScreenPointToVPPoint (POINT Point)
{
	POINT NewPoint=Point,PtFrom,PtTo; 
	RECT FromRect; 
	HANDLE	hTran;   
	MNMXCORD	Bounds;
	DPOINT	OutPoint={Point.x,Point.y},FromPoint;
	
	if (CurView)
	{   
		FromRect = CurView->Rect;
		ClientRectToScreenRect (CurView->hWnd,&FromRect);
		Bounds.xmn = CurView->TagPoint.x;
		Bounds.ymn = CurView->TagPoint.y;  
		Bounds.xmx = CurView->Width;
		Bounds.ymx = CurView->Height;
		hTran = STRANRectToBounds (&FromRect,&Bounds);   
		FromPoint.x = Point.x;
		FromPoint.y = Point.y;
		OutPoint = TranPoint (&FromPoint,hTran);
		CloseTRANS2 (&hTran); 
	}
	return OutPoint;
}

BOOL AddPointToBuffer (DPOINT DPoint,long NewRefno,HANDLE hTimeStamp,int idesc,double size, double rot,LPSHORT Stuff,HANDLE hGRText,HANDLE hTextTPL,
					LPSTR Prefix, LPSTR UDI,long AreaColor,long PenColorIn, short PenWidth,BOOL HiPrecis,LPHANDLE phBuf,LPLONG plbuf,int Opt)
{ // opt=0 write full record, 1 create new record with text but do not close, 2 add text data to open record, 3 add text and close record
	long	lbuftemp=0;
	short	i, j, id,  ii, nPoly;
	DPOINT	pt1, pt2;
	static	short	item_len;
	short	item_len2;
	static	mnmxCor	PtMinMax; 
	long	Offset, CurRecLenLoc, loc, endsegloc, PenColor, StartTime, EndTime;
	static	long	lBufBeg;      
	short		CurRecLen, ltag;  
	long	TextColor=0;
	LPGRTEXT		pGRText;
	LPUMTEXTTPL		pTextTPL;
	GRTEXTHEADER	GRTextHead;
	HPSTR	pBuf, pBufBeg, MinMaxLoc; 
	char	Text[256];
	char	tag[80];
	short	TxtType=1;  
	static	short	BufType, BufDesc=0;  
	struct {
			short	id;
			float	size,
				  	rot;
			POINT	point;} PointData;
	struct {
			short	id;
			double	size,
				  	rot;
			DPOINT	point;} PointDataD;
			
	if (hGRText && !SymbolIsVisible (idesc))
		TxtType = 2;

//	CurView->HaveBounds=FALSE;
	
	CurView->FileProjectionType=0;
	PenColor = PenColorIn;
	    
	pBuf = GlobalLock (*phBuf);
	pBuf += *plbuf;
	if (Opt < 2)
	{
		MinMaxInit (&PtMinMax);
		ltag = 0;
		if (Prefix) 
		{   
			if (*Prefix)
			{
				_fstrcpy (tag,Prefix);
				_fstrcat (tag,":");
				_fstrcat (tag,UDI);
				ltag = _fstrlen (tag);
				if (ltag%2) ltag++;  
			}
		}
		item_len = 2+4+ltag + 2+2;  
		if (hTimeStamp)
			item_len += 10; 
		if (Stuff)
			item_len += *Stuff;  
		if (AreaColor != -1 || PenColor != -1)
			item_len += 2;
		if (AreaColor != -1)
			item_len += 12;
		if (PenColorIn != -1)
			item_len += 10;  
	}
	if (hGRText)
	{   
		short	ltxt;
		
		pGRText = (LPGRTEXT)GlobalLock (hGRText);   
		if (pGRText->ltext > 0)
			ltxt = pGRText->ltext; 
		else
		{
			_fstrcpy (Text,pGRText->Text);
			ExpandText (Text);
			ltxt = max (2,_fstrlen (Text));
		} 
		ltxt += ltxt % 2;
		item_len += (2 + sizeof(GRTEXTHEADER) + ltxt);  
		_fstrcpy (Text,pGRText->cColor);
		ExpandText (Text);
        TextColor = atol (Text);
		GlobalUnlock (hGRText);
		if (TextColor > 0)
			item_len += 6;  
	}
	if (hTextTPL)
		item_len += (2 + sizeof(UMTEXTTPL)); 
	if (HiPrecis)
		item_len += sizeof(PointDataD); 
	else
		item_len += sizeof(PointData); 

	if (Opt < 2)
	{
		id = 12;
		pBufBeg = pBuf;
		lBufBeg = *plbuf;
		BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
		
		MinMaxLoc = pBuf;
		BufWrite(&pBuf,plbuf,(HPSTR)&PtMinMax,8);
		item_len /= 2;
		item_len2 = -item_len;
		BufWrite(&pBuf,plbuf,(HPSTR)&item_len2,2); 
		id = 9 + 256 * ltag;
		BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
		BufWrite(&pBuf,plbuf,(HPSTR)&NewRefno,4); 
		if (ltag)
			BufWrite(&pBuf,plbuf,tag,ltag);
		id = 8;
		BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
		BufWrite(&pBuf,plbuf,(HPSTR)&idesc,2); 
		if (hTimeStamp)
		{   
			char	str[256];
			LPTIMESTAMP	lpTimeStamp=(LPTIMESTAMP)GlobalLock (hTimeStamp); 
			
			id = 37;
			BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
			_fstrcpy (str,lpTimeStamp->StartTime);
			ExpandText (str);
			StartTime = atol (str);
			if (StartTime == 1)     
			{
				time_t	systime;     
				time (&systime);
				StartTime = systime;
			}
			BufWrite(&pBuf,plbuf,(HPSTR)&StartTime,4);
			_fstrcpy (str,lpTimeStamp->EndTime);
			ExpandText (str);
			EndTime = atol (str); 
			if (EndTime <= StartTime)
				EndTime = StartTime;
			BufWrite(&pBuf,plbuf,(HPSTR)&EndTime,4);  
			GlobalUnlock (hTimeStamp);   
			MinFileTime = min (MinFileTime,StartTime);
			MaxFileTime = max (MaxFileTime,EndTime);		
		} 
		else
		{
			MinFileTime = min (MinFileTime,0);
			MaxFileTime = LONG_MAX;
		}
		if (Stuff)
		{   
			if (*Stuff)
			{
				item_len = *Stuff++;
				BufWrite(&pBuf,plbuf,(HPSTR)Stuff,item_len); 
			}
		}
		if (AreaColor != -1)
		{
			id = 22;
			BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
			id = 0;
			BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
			BufWrite(&pBuf,plbuf,(HPSTR)&AreaColor,4);
			BufWrite(&pBuf,plbuf,(HPSTR)&AreaColor,4);
		}
		if (PenColor != -1)
		{    
			if (HiPrecis)
				id = 121;
			else
				id = 21;
			BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
			BufWrite(&pBuf,plbuf,(HPSTR)&PenColor,4);
			BufWrite(&pBuf,plbuf,(HPSTR)&PenWidth,2);
			id = 0;
			BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
		}
	}
    if (HiPrecis)
    {
		PointDataD.id = 120;
		PointDataD.size = size;
		PointDataD.rot = rot;
		PointDataD.point = DPoint;
		BufWrite(&pBuf,plbuf,(HPSTR)&PointDataD,sizeof(PointDataD));
	}
	if (hTextTPL)
	{   
		DPOINT	pt1, pt2;
		
		id = 192;
		BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
		pTextTPL = (LPUMTEXTTPL)GlobalLock (hTextTPL);
		BufWrite(&pBuf,plbuf,(HPSTR)pTextTPL,sizeof(UMTEXTTPL));
		GlobalUnlock (hTextTPL);    
	}
	if (TextColor > 0)
	{
		id = 25;
		BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
		BufWrite(&pBuf,plbuf,(HPSTR)&TextColor,4);
	}
    
	if (hGRText)
	{   
		LPSTR	pText;
		double	dHeight, twidth, xmove, ymove;
		UINT	isize;  
		short	NumLines, iline, NumUpLines, MaxLineLen; 
		
		id = 19;
		BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
		pGRText = (LPGRTEXT)GlobalLock (hGRText);  
		_fmemset (&GRTextHead,0,sizeof(GRTEXTHEADER)); 
		GRTextHead.lText = max (0,pGRText->ltext);
		if (pGRText->UltiMapStyle)
		{
			if (pGRText->hJust < -6)
				GRTextHead.hJust = 2;
			else if (pGRText->hJust > 6)
				GRTextHead.hJust = 0;
			else
				GRTextHead.hJust = 1; 
			GRTextHead.hJust2 = 15 - IDNINT (fabs((double)pGRText->hJust * 15)/99); 
		}   
		else
		{
			GRTextHead.hJust = 2 - pGRText->hJust;
			GRTextHead.hJust2 = 0;
		}			
		GRTextHead.vJust = pGRText->vJust;   
		GRTextHead.italic = pGRText->italic;
		GRTextHead.Weight = pGRText->weight;
		GRTextHead.UltiMapStyle = pGRText->UltiMapStyle;
		GRTextHead.FlipForEasyReading = pGRText->FlipForEasyReading;
		_fstrcpy (Text,pGRText->cHeight);
		ExpandText (Text);
		dHeight = atof (Text);
		GRTextHead.FontNum = pGRText->FontNum;              
		GRTextHead.HeightIsPixels = 0;              
		switch (*LastChr (Text))
		{
			case 'F':
			case 'f':
				dHeight = ConvertInDist (dHeight,1);
			break;
			case 'M':
			case 'm':
				dHeight = ConvertInDist (dHeight,2);
			break;
			case 'p': 
				dHeight = dHeight / BaseDistToWinDist;
			break;
			case 'P':
				GRTextHead.HeightIsPixels = 1;              
			break; 
		}
		if (fabs(dHeight) < 64)
		{
			GRTextHead.HeightPrecision = 3;
			isize = IDNINT (dHeight * 1000);
		}    
		else if (fabs(dHeight) < 640)
		{
			GRTextHead.HeightPrecision = 2;
			isize = IDNINT (dHeight * 100);
		}    
		else if (fabs(dHeight) < 6400)
		{
			GRTextHead.HeightPrecision = 1;
			isize = IDNINT (dHeight * 10);
		}  
		else  
			isize = IDNINT(dHeight);
		GRTextHead.Height = isize;

		_fstrcpy (Text,pGRText->Text);
		ExpandText (Text);
		if (GRTextHead.lText <= 0)
			GRTextHead.lText = max (2,_fstrlen(Text)); 
		GRTextHead.lText += GRTextHead.lText % 2;
		BufWrite(&pBuf,plbuf,(HPSTR)&GRTextHead,sizeof(GRTEXTHEADER)); 
		BufWrite(&pBuf,plbuf,Text,GRTextHead.lText);
		GlobalUnlock (hGRText); 
        
	}
	if (!Opt || Opt == 3)
	{
		if (AreaColor != -1 || PenColorIn != -1)
		{
			id = 23;
			BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
		} 
	}
	GlobalUnlock (*phBuf);
	pBuf = GlobalLock (*phBuf);
	pBuf += lBufBeg + 10;
	item_len2 = -item_len;
	BufWrite(&pBuf,&lbuftemp,(HPSTR)&item_len2,2); 
	GlobalUnlock (*phBuf);
    
{
#if ENABLETRACE
GSSiExitProg (740);
#endif
	return(TRUE);
}
}					

BOOL AddPolyToBuffer (int nPolyIn, LPINT lpnPntsIn,LPHANDLE phDPoints,short Type, long NewRefno,HANDLE hTimeStamp,short ipen, int idesc,LPSHORT Stuff,
					LPSTR Prefix, LPSTR UDI,long AreaColor,long PenColorIn, short PenWidth,
					HANDLE hGRText,HANDLE hTextTPL,short nCurvePoints, LPSHORT CurvePoints,short HiPrecis,LPHANDLE phBuf,LPLONG plbuf)
#if ENABLETRACE
{GSSiEnterProg (737);
#endif
{   
	HANDLE	hPoints, hDPoints, hhPoints=0,hnPoints=0; 
	LPHANDLE	phPoints, lphDPoints; 
	LPINT	pnPnts, pnPoints;
	LPINT	lpnPnts;
	HPPOINT	lpPoints, lpPoint;
    HPDPOINT lpDPoint;  
   	DPOINT LastDPoint, FirstDPoint, TXPoint, DPoint; 
   	double	rot;
   	POINT	LastPoint, Point;
    HANDLE	hRec;
    HPSTR	pBuf, pBufBeg, MinMaxLoc;
	char	Text[256];
	unsigned short i, j;
	short id, ii, itemlen;
	int		nPoly;
	mnmxCor	MinMax; 
	int		TotPoints=0; 
	long	TotP=0, nElevPts=0;
	long	Offset, CurRecLenLoc, loc, endsegloc, PenColor,RecLen, item_len, PointSize=4;      
	static	long	lastloc,iii=-1;
	int		CurRecLen, ltag,  nsame=0;  
	char	tag[80];
	LPSTR	Rec; 
	BOOL	First; 
	LPGRTEXT		pGRText;
	LPUMTEXTTPL		pTextTPL;
	GRTEXTHEADER	GRTextHead;
   	DPOINT POC,PT;
	long	lbuftemp=0, TextColor=0;   
	HANDLE	hTemp=0;
	BOOL	Deleted=FALSE; 
	short	TypeLevel=Type;
	HANDLE	hPnts;
	
	if (Type == 4) //shapes that go on line level
	{
		TypeLevel = 1;
		Type = 0;
	}
		
    if (HiPrecis)
    	PointSize = 16;

//	CurView->HaveBounds=FALSE; 
	CurView->FileProjectionType=0;
	PenColor = PenColorIn;
	nPoly = abs (nPolyIn);
	hPnts = GSSiGlobAlloc (0,GMEM_MOVEABLE,max (1,nPoly) * sizeof (int));
	lpnPnts = GlobalLock (hPnts);
	for (i=0;i<max (1,nPoly);i++)
	{
		if (lpnPntsIn[i] > USHRT_MAX)
			MessageBox (0,"Num points in poly island exceeds 64K",0,MB_ICONEXCLAMATION);
		lpnPnts[i] = lpnPntsIn[i];
	}
	TotP = 0; 
	if (nPolyIn)
	{
		if (nPolyIn < 0) 
		{
			for (i=0,pnPnts=lpnPnts;i<nPoly;i++,pnPnts++) 
				TotPoints += *pnPnts;
			TotPoints += nPoly - 1;
			PenColor = -1;
			nPoly = 1; 
			lpnPnts = &TotPoints;   
		}
		lphDPoints = phDPoints;
		hhPoints = GSSiGlobAlloc ( 347,GHND,(nPoly+1)*sizeof(HANDLE));
		phPoints = (LPHANDLE)GlobalLock (hhPoints);
		hnPoints = GSSiGlobAlloc ( 348,GHND,nPoly*sizeof(int));
		pnPoints = (LPINT)GlobalLock (hnPoints);
		for (i=0,pnPnts=lpnPnts;i<nPoly;i++,pnPnts++,phPoints++,phDPoints++,pnPoints++) 
		{
			*phPoints = GSSiGlobAlloc ( 349,GHND,(long)*pnPnts*sizeof(POINT));
			lpPoints = (LPPOINT) GlobalLock(*phPoints);
			lpPoint = lpPoints; 
			if (HiPrecis == 2) 
			{
				HPDPOINT3D	lpDPoint3D = (HPDPOINT3D)GlobalLock(*phDPoints); 
				long	np = *pnPnts; 
				
				nElevPts = np;
				hTemp = GSSiGlobAlloc ( 350,GMEM_MOVEABLE,np*sizeof(DPOINT));
				lpDPoint = (HPDPOINT)GlobalLock(hTemp);
				while (np--)
				{
					lpDPoint->x = lpDPoint3D->x;
					lpDPoint++->y = lpDPoint3D++->y;
				}
				GlobalUnlock (hTemp);
				lpDPoint = (HPDPOINT)GlobalLock(hTemp);
			}
			else
				lpDPoint = (HPDPOINT)GlobalLock(*phDPoints);
			if (!TotP)
				FirstDPoint = *lpDPoint;
			for (j=0;j<*pnPnts;j++,lpDPoint++)
			{   
				switch (TotP)
				{
					case 1:
						POC = *lpDPoint;
						break;
					case 2:
						PT = *lpDPoint;
						break;
					default:
						break;
				}
				if (!i && j==1)
				{
					TXPoint = MidPointD (FirstDPoint,*lpDPoint);
					rot = getazd (&FirstDPoint,lpDPoint);
				}
				*lpPoint = BasePtToFilePt(*lpDPoint); 
	//			if (lpDPoint->x <= 0 || lpDPoint->y <=0)
	//				ii=1;
				if (Type && (i || j) && !HiPrecis &&
					(lpPoint->x == LastPoint.x && lpPoint->y == LastPoint.y))
					nsame++;
				else 
				{
					TotP++; 
					(*pnPoints)++;
					LastPoint = *lpPoint;   
					lpPoint++; 
				}
			} 
			GlobalUnlock (*phDPoints); 
			GlobalUnlock(*phPoints);
		    GSSiGlobUlFree (&hTemp);
	    }
	    GlobalUnlock (hhPoints);  
	    GlobalUnlock (hnPoints); 
	    pnPoints = (LPINT)GlobalLock (hnPoints); //new npoints array needed if dup points removed  
	    lpnPnts = pnPoints;
	}
	ltag=0; 
	if (Prefix) 
	{   
		if (*Prefix && _fstricmp (Prefix,"REFNO"))
		{
			_fstrcpy (tag,Prefix);
			_fstrcat (tag,":");
			_fstrcat (tag,UDI);
			ltag = _fstrlen (tag);
			if (ltag%2) ltag++;
		}
	}
	item_len = 2+4+ltag + 2+2; 
	if (hTimeStamp)
		item_len += 10; 
	if (Stuff)
		item_len += *Stuff;  
	if (AreaColor != -1 || PenColor != -1)
		item_len += 2;
	if (AreaColor != -1)
		item_len += 12;
	if (PenColorIn != -1)
		item_len += 10;  
	if (nCurvePoints)
		item_len += 2 + nCurvePoints * 2;
	if (hGRText)
	{   
		short	ltxt;
		
		pGRText = (LPGRTEXT)GlobalLock (hGRText); 
		if (pGRText->ltext > 0)
			ltxt = pGRText->ltext; 
		else
		{
			_fstrcpy (Text,pGRText->Text);
			ExpandText (Text);
			ltxt = max (2,_fstrlen (Text));
		} 
		ltxt += ltxt % 2;
		item_len += (2 + sizeof(GRTEXTHEADER) + ltxt);
		_fstrcpy (Text,pGRText->cColor);
		ExpandText (Text);
        TextColor = atol (Text);
		GlobalUnlock (hGRText);
		if (TextColor > 0)
			item_len += 6;  
	}
	if (hTextTPL)
		item_len += (2 + sizeof(UMTEXTTPL));
	if (nPoly)
	{ 
		if (nPoly > 1)
			if (!Type)
				item_len += (4 + 2*nPoly);
		for (i=0,pnPnts=lpnPnts;i<nPoly;i++,pnPnts++)
			item_len += 4 + (long)*pnPnts*PointSize; 
		if (nPolyIn < 0) 
		{
			for (i=0,pnPnts=pnPoints;i<abs(nPolyIn);i++,pnPnts++)
				item_len += 4 + (long)*pnPnts*PointSize; 
		}		
		if (!Type)
			item_len+=2*nPoly; 
		if (Type == 1 && TotP == 2 && HiPrecis)
			item_len -= 2; // type is GF_LINE
	}
	else
		item_len += 2;  
	if (HiPrecis == 2)
		item_len += (2 + 2 + nElevPts*4);
	pBuf = GlobalLock (*phBuf);
	pBuf += *plbuf;
	pBufBeg = pBuf;
	
	id = 12 + 256 * ipen;
	BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
	MinMaxLoc = pBuf;
	BufWrite(&pBuf,plbuf,(HPSTR)&MinMax,8);
	item_len /= 2;
	if (item_len > 16000)
		itemlen=0;
	else
		itemlen = -item_len;
	BufWrite(&pBuf,plbuf,(HPSTR)&itemlen,2); 
	id = 9 + 256 * ltag;
	BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
	BufWrite(&pBuf,plbuf,(HPSTR)&NewRefno,4); 
	if (ltag)
		BufWrite(&pBuf,plbuf,tag,ltag);
	id = 8;
	BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
	BufWrite(&pBuf,plbuf,(HPSTR)&idesc,2);  
	if (hTimeStamp)
	{   
		char	str[256]; 
		long	StartTime, EndTime;
		LPTIMESTAMP	lpTimeStamp=(LPTIMESTAMP)GlobalLock (hTimeStamp); 
		
		id = 37;
		BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
		_fstrcpy (str,lpTimeStamp->StartTime);
		ExpandText (str);
		StartTime = atol (str); 
		if (StartTime == 1)     
		{
			time_t	systime;     
			time (&systime);
			StartTime = systime;
		}
		BufWrite(&pBuf,plbuf,(HPSTR)&StartTime,4);
		_fstrcpy (str,lpTimeStamp->EndTime);
		ExpandText (str);
		EndTime = atol (str); 
		if (EndTime <= StartTime)
			EndTime = StartTime;
		BufWrite(&pBuf,plbuf,(HPSTR)&EndTime,4);  
		GlobalUnlock (hTimeStamp);
		MinFileTime = min (MinFileTime,StartTime);
	    MaxFileTime = max (MaxFileTime,EndTime);		
	} 
	else
	{
		MinFileTime = min (MinFileTime,0);
		MaxFileTime = LONG_MAX;
	}
	if (Stuff)
	{   
		item_len = *Stuff++;
		BufWrite(&pBuf,plbuf,(HPSTR)Stuff,item_len);
	}
	if (AreaColor != -1)
	{
		id = 22;
		BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
		id = 0;
		BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
		BufWrite(&pBuf,plbuf,(HPSTR)&AreaColor,4);
		BufWrite(&pBuf,plbuf,(HPSTR)&AreaColor,4);
	}
	if (PenColor != -1)
	{
		if (HiPrecis)
			id = 121;
		else
			id = 21;
		BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
		BufWrite(&pBuf,plbuf,(HPSTR)&PenColor,4);
		BufWrite(&pBuf,plbuf,(HPSTR)&PenWidth,2);
		id = 0;
		BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
	}
    
    if (nCurvePoints)
    {   
		id = 28;
		BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
		BufWrite(&pBuf,plbuf,(HPSTR)&nCurvePoints,2); 
		BufWrite(&pBuf,plbuf,(HPSTR)CurvePoints,2*nCurvePoints); 
    }
    
	if (nPoly > 1 && !Type)
	{   
		if (HiPrecis)
			id = 227;
		else
			id = 228;
		BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
		BufWrite(&pBuf,plbuf,(HPSTR)&nPoly,4);
		BufWrite(&pBuf,plbuf,(HPSTR)lpnPnts,4*nPoly);
	}
    
    if (HiPrecis)
    {   
    	HANDLE	hElev=0;
    	HPFLOAT	pElev;
    	
		phDPoints = lphDPoints;
		
		for (i=0,pnPnts=lpnPnts;i<nPoly;i++,pnPnts++,phDPoints++)
		{   
			if (HiPrecis == 2) 
			{
				HPDPOINT3D	lpDPoint3D = (HPDPOINT3D)GlobalLock(*phDPoints); 
				long	np = *pnPnts;
				hTemp = GSSiGlobAlloc ( 352,GMEM_MOVEABLE,np*sizeof(DPOINT));
				lpDPoint = (HPDPOINT)GlobalLock(hTemp);
				hElev= GSSiGlobAlloc ( 353,GMEM_MOVEABLE,np*sizeof(float));
				pElev = (HPFLOAT)GlobalLock(hElev);
				while (np--)
				{
					lpDPoint->x = lpDPoint3D->x;
					lpDPoint++->y = lpDPoint3D->y; 
					*pElev++ = lpDPoint3D++->z;
				} 
				GlobalUnlock (hElev);
				GlobalUnlock (hTemp);
				lpDPoint = (HPDPOINT)GlobalLock(hTemp);
			}
			else
				lpDPoint = (HPDPOINT)GlobalLock(*phDPoints);
			if (hElev)
			{
				pElev = (HPFLOAT)GlobalLock(hElev);
				id = 38; 
				BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
				BufWrite(&pBuf,plbuf,(HPSTR)pnPnts,2);
				BufWrite(&pBuf,plbuf,(HPSTR)pElev,(long)*pnPnts*4);   
				GSSiGlobUlFree (&hElev);
			}  
			switch (Type)
			{   
				case 1:  
					if (nPoly == 1 && *pnPnts == 2)
					{
						id = 41;
						BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
					}
					else
					{
						id = 61;
						BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
						BufWrite(&pBuf,plbuf,(HPSTR)pnPnts,2); 
					}
					BufWrite(&pBuf,plbuf,(HPSTR)lpDPoint,(long)*pnPnts*16);   
				break; 
				
				case 3:
					id = 172; 
					TypeLevel = Type = 1;
					BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
					BufWrite(&pBuf,plbuf,(HPSTR)pnPnts,2);
					BufWrite(&pBuf,plbuf,(HPSTR)lpDPoint,(long)*pnPnts*16);   
				break;
				 
				case 0:		
					id = 51;
					BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
					BufWrite(&pBuf,plbuf,(HPSTR)&ipen,2);
					BufWrite(&pBuf,plbuf,(HPSTR)pnPnts,2);
					BufWrite(&pBuf,plbuf,(HPSTR)lpDPoint,(long)*pnPnts*16); 
				break;  
			} 
			GlobalUnlock (*phDPoints);  
			GSSiGlobUlFree (&hTemp);
		}  
    }
    else if (nPolyIn)
    {
		phPoints = (LPHANDLE)GlobalLock (hhPoints);
		for (i=0,pnPnts=lpnPnts;i<nPoly;i++,pnPnts++,phPoints++)
		{   
			lpPoints = (LPPOINT)GlobalLock (*phPoints);
			switch (Type)
			{   
				case 1:
					id = 6;
					BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
					BufWrite(&pBuf,plbuf,(HPSTR)pnPnts,2);
					BufWrite(&pBuf,plbuf,(HPSTR)lpPoints,(long)*pnPnts*4);   
				break; 
				
				case 3:
					id = 171; 
					TypeLevel = Type = 1;
					BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
					BufWrite(&pBuf,plbuf,(HPSTR)pnPnts,2);
					BufWrite(&pBuf,plbuf,(HPSTR)lpPoints,(long)*pnPnts*4);   
				break;
				 
				case 0:		
					id = 5;
					BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
					BufWrite(&pBuf,plbuf,(HPSTR)&ipen,2);
					BufWrite(&pBuf,plbuf,(HPSTR)pnPnts,2);
					BufWrite(&pBuf,plbuf,(HPSTR)lpPoints,(long)*pnPnts*4); 
				break;  
			}
			GlobalUnlock (*phPoints); 
		}  
		GlobalUnlock (hhPoints);
    }
    
	if (nPolyIn < 0) 
	{
		if (PenColorIn != -1)
		{   
			if (HiPrecis)
				id = 121;
			else
				id = 21;
			BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
			BufWrite(&pBuf,plbuf,(HPSTR)&PenColorIn,4);
			BufWrite(&pBuf,plbuf,(HPSTR)&PenWidth,2);
			id = 0;
			BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
		}
  
		phPoints = (LPHANDLE)GlobalLock (hhPoints);
		lpPoints = (LPPOINT)GlobalLock (*phPoints);
		for (i=0,pnPnts=pnPoints;i<abs(nPolyIn);i++,pnPnts++)
		{   
			id = 6;
			BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
			BufWrite(&pBuf,plbuf,(HPSTR)pnPnts,2);
			BufWrite(&pBuf,plbuf,(HPSTR)lpPoints,(long)*pnPnts*4); 
			lpPoints += (*pnPnts);
			if (i)
				lpPoints++;  
		}  
		GlobalUnlock (*phPoints); 
		GlobalUnlock (hhPoints);
    } 
    if (!nPolyIn)
    {
		id = 24;
		BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
	}
	if (hTextTPL)
	{   
		DPOINT	pt1, pt2;
		
		id = 192;
		BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
		pTextTPL = (LPUMTEXTTPL)GlobalLock (hTextTPL);
		BufWrite(&pBuf,plbuf,(HPSTR)pTextTPL,sizeof(UMTEXTTPL));
		GlobalUnlock (hTextTPL);    
	}
	if (TextColor > 0)
	{
		id = 25;
		BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
		BufWrite(&pBuf,plbuf,(HPSTR)&TextColor,4);
	}
    
	if (hGRText)
	{   
		LPSTR	pText;
		double	dHeight, twidth, xmove, ymove;
		UINT	isize;  
		long	lbuftemp=0;
		DPOINT	pt1, pt2;
		short	NumLines, iline, NumUpLines, MaxLineLen; 
		
		id = 19;
		BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
		pGRText = (LPGRTEXT)GlobalLock (hGRText);  
		_fmemset (&GRTextHead,0,sizeof(GRTEXTHEADER));
		GRTextHead.lText = max (0,pGRText->ltext);
		if (pGRText->UltiMapStyle)
		{
			if (pGRText->hJust < -6)
				GRTextHead.hJust = 2;
			else if (pGRText->hJust > 6)
				GRTextHead.hJust = 0;
			else
				GRTextHead.hJust = 1; 
			GRTextHead.hJust2 = 15 - IDNINT (fabs((double)pGRText->hJust * 15)/99); 
		}   
		else
		{
			GRTextHead.hJust = 2 - pGRText->hJust;
			GRTextHead.hJust2 = 0;
		}			
		GRTextHead.vJust = pGRText->vJust;          
		GRTextHead.italic = pGRText->italic;
		GRTextHead.Weight = pGRText->weight;
		GRTextHead.UltiMapStyle = pGRText->UltiMapStyle;
		GRTextHead.FlipForEasyReading = pGRText->FlipForEasyReading;
		_fstrcpy (Text,pGRText->cHeight);
		ExpandText (Text);
		dHeight = atof (Text);
		GRTextHead.FontNum = pGRText->FontNum;              
		GRTextHead.HeightIsPixels = 0;              
		switch (*LastChr (Text))
		{
			case 'F':
			case 'f':
				dHeight = ConvertInDist (dHeight,1);
			break;
			case 'M':
			case 'm':
				dHeight = ConvertInDist (dHeight,2);
			break;
			case 'p': 
				dHeight = dHeight / BaseDistToWinDist;
			break;
			case 'P':
				GRTextHead.HeightIsPixels = 1;              
			break; 
		}
		if (fabs(dHeight) < 64)
		{
			GRTextHead.HeightPrecision = 3;
			isize = IDNINT (dHeight * 1000);
		}    
		else if (fabs(dHeight) < 640)
		{
			GRTextHead.HeightPrecision = 2;
			isize = IDNINT (dHeight * 100);
		}    
		else if (fabs(dHeight) < 6400)
		{
			GRTextHead.HeightPrecision = 1;
			isize = IDNINT (dHeight * 10);
		}  
		else  
			isize = IDNINT(dHeight);
		GRTextHead.Height = isize;
		_fstrcpy (Text,pGRText->Text);
		if (GRTextHead.lText <= 0)
		{
			ExpandText (Text);
			GRTextHead.lText = max (2,_fstrlen(Text)); 
		}
		GRTextHead.lText += GRTextHead.lText % 2;
		if (GRTextHead.lText == 92)
			ii=1;	
		BufWrite(&pBuf,plbuf,(HPSTR)&GRTextHead,sizeof(GRTEXTHEADER)); 
		BufWrite(&pBuf,plbuf,(HPSTR)Text,GRTextHead.lText);
		GlobalUnlock (hGRText); 
	}
	if (AreaColor != -1 || PenColorIn != -1)
	{
		id = 23;
		BufWrite(&pBuf,plbuf,(HPSTR)&id,2);
	} 
	if (hhPoints)
	{
		phPoints = (LPHANDLE)GlobalLock (hhPoints);
		while (*phPoints)
			GSSiGlobFree (phPoints++);
		GSSiGlobUlFree (&hhPoints);
	}
    GSSiGlobUlFree (&hnPoints);
	GlobalUnlock (*phBuf);
	GSSiGlobUlFree (&hPnts);
{
#if ENABLETRACE
GSSiExitProg (737);
#endif
	return(TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL NotifyFunction (LPVIEWPORT pVP,UINT message)
#if ENABLETRACE
{GSSiEnterProg (170);
#endif
{   
	BOOL	st=0;
	LPVIEWPORT	SaveVP=CurView;
	int	ii;
	
	if (!pVP)
{
#if ENABLETRACE
GSSiExitProg (170);
#endif
		return FALSE;          
}
	if (message == GF_EXIT_VIEWPORT)
		SetSysMess ("");
	if ((int)pVP == -1)
	{
		int iview;

		for (iview=0;iview<*pNumViewports;iview++)
		{
			SetCurView ( pViewportsD[iview]); 
	        if (!CurView->DisplayInParent && CurViewActive())
			{
				st = ProcessGraphicsFunction2 (CurView->CurrentFunction,CurView->hWnd,message,0,0L); 
			}
		}
	}
	else
	{
		SetCurView ( pVP);
		st = ProcessGraphicsFunction2 (CurView->CurrentFunction,CurView->hWnd,message,0,0L); 
	}
	SetCurView ( SaveVP);
{
#if ENABLETRACE
GSSiExitProg (170);
#endif
	return st;
}
#if ENABLETRACE
}
#endif
}
	
void EscapeFunction (BOOL DoHalt)
{
	LPVIEWPORT SaveVP = CurView;
	int	i;

	MergeImageIntoViewport(0, 0, 0, 0);
	SetConfig(1);
	NumTAGDef = -1;  
	BlockVehicleDisplay = 0;
	BlockSocketProcessing (FALSE);
	VehicleStatusChanged = TRUE;
	for (i=0;i<*pNumViewports;i++)
	{
		CurView = pViewports[i];
		CloseThemeFiles();
	}
	CloseBufferedMacros ();
	if (DoHalt)
	{
		ResetFunStack (TRUE);
		HaltMapDisplay (TRUE,TRUE);
	}
    CloseSymDict();  
	CloseOrthos(TRUE);
	GetSymAttrFile (0,0,0,0,0,0);
	AddBMPToCache (0,0);
	AddBMPToCache32 (0,0);
	LoadCustomStreenNameConversions (TRUE);
	ClosePassiveFunctions ();
	DTMClose (0);
	CloseAllRequestedFiles(FALSE); 
   	CacheAlreadyChecked (0,0,0);
	DeleteCacheDir ();
	ODBCTerminate (TRUE);
	WriteToHBird (0,0);
	ConvertToNewLocation (0,0);
	RemoveVPBitmaps ();
	RemoveFromMacroStack (-1);
	CurView = SaveVP;

	return;
}
