#include "graphint.h"   
#include "translat.h"

#define MAXATT	48 
static BOOL	Convert8to24=TRUE;
 
#include "gmextern.h"  
#include "dibapi.h"



BOOL ImportSSURGOTables (LPSTR Name,LPSTR DestDir);


void MoveHtoF (HPBYTE pTo, HPBYTE pFrom, DWORD n)
{
	while (n--)
		*pTo++ = *pFrom++;
	return;
}


BOOL OutputCatalogData (LPSTR Quad, LPSTR Volumn, BOOL HaveWorld)
{
    GWDHEADER GWDHead; 
    LPGWDHEADER lpGWDHead;
    HANDLE  hDB;   
    long	Offset;   
    short	len;  
    typedef struct {
    				char	Quad[8],
    						Vol[8],
    						DupVol[8];
    				BOOL	HaveWorld,
    						NeedClip;
    				short	ClipLeft,ClipRight,ClipTop,ClipBottom;
    				}CATDATA;
    typedef CATDATA	FAR	*LPCATDATA;
	LPCATDATA	pCatData;  
	BOOL	rtn=FALSE;
	char	Name[]="[%DL]attribut\\zn14cat.gmd"; 
	char	txt[128]; 

	hDB = OpenGWDatabase (Name,BT_WRITE);
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
    pCatData = (LPCATDATA)&lpGWDHead->GWDData;
    if (!BT_FIND (lpGWDHead->BTHandle[0],Quad,BT_FIRST,BT_EQ, (LPSTR)&Offset))
    {
        len=FillGWDData (lpGWDHead,Offset);
        _fstrncpy (pCatData->DupVol,Volumn,8);
		GWDReplaceRecord (lpGWDHead,len,NULL,Offset);
	}
	else
	{   
		_fmemset (pCatData,0,sizeof(CATDATA));
        _fstrncpy (pCatData->Quad,Quad,8);
        _fstrncpy (pCatData->Vol,Volumn,8); 
        pCatData->HaveWorld = HaveWorld;
        pCatData->ClipLeft = -1;
        GWDAddRecord (lpGWDHead,0,NULL);
	}
	GlobalUnlock (hDB);
    CloseGWDatabase (hDB); 
	return rtn;
}  

DPOINT OrthoToGround2(DPOINT InPoint,HANDLE hTran)
{   
	DPOINT OutPoint;
	
	TRANS2 (InPoint.x,InPoint.y,&OutPoint.x,&OutPoint.y,hTran);
	return OutPoint;
}

  
BOOL WriteQuadFile (LPQUADDATA pQuadDataIn)
{
	short		pos=BT_FIRST; 
	long		LastID;
    BTVARDESC  *pVars;
    short       NumFields, Reclen, len;
    GWDHEADER GWDHead; 
    LPGWDHEADER lpGWDHead;
    HANDLE  hVars, hDB;   
    long	Offset;
    short       FidData,ibeg,NumVars, NumSegs;
    OFSTRUCTGM    OFStruct;
    GWFLDINFO FldInfo;    
    char	PrimeIndex[128], Name[128];
    LPSTR	lpDot;  
    BOOL	Opened, rtn=FALSE;
    static	BOOL	First=FALSE;
	LPQUADDATA	pQuadData;    
    _fstrcpy (Name,"[%DL]quads.gmd");
    if (!ExistFile (Name))
    	First = TRUE;
    if (!First) goto OpenIt;
    First = FALSE;
    _fstrcpy (PrimeIndex,"[%DL]quads.in1");
    lpGWDHead = &GWDHead; 
    
    FidData = GSSiOpenFile (Name,&OFStruct,OF_CREATE);   
    _fmemset (lpGWDHead,0,sizeof(GWDHEADER));
    GWDHead.NumIndex=1;
    GWDHead.Version=1;
    GWDHead.NumIndexFields[0]=1;
    GWDHead.IndexFields[0][0]=0;
    BigWrite (FidData,(char *)&GWDHead,sizeof(GWDHEADER),-1);
    ibeg = 0;

    FldInfo.Len = 12;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"QUAD");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 16;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"VolLabel");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"Date");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"FltHeight");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 6;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"GSSiCD");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"GSSiFile");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"Status");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"Datum");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"GroundRefSys");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"ZoneCode");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"GroundResolution");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"nRows");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"nCols");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"ClipRectLeft");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"ClipRectRight");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"ClipRectTop");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"ClipRectBottom");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"SWPointX");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"SWPointY");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"NWPointX");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"NWPointY");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"NEPointX");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"NEPointY");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"SEPointX");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"SEPointY");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;


	GWDHead.Reclen=ibeg; 
	GWDHead.TimeStamp = time(0);
	GSSillseek (FidData,0,0);
	BigWrite (FidData,(char *)&GWDHead,sizeof(GWDHEADER),-1);
	GSSillseek (FidData,0,2);
	                 
	NumVars = 1;
	            
	hVars = LocalAlloc (LHND,NumVars * sizeof(BTVARDESC));
	pVars =(LPBTVARDESC) LocalLock(hVars);
	            
	pVars->BT_VARLEN=12;
	pVars->BT_VARTYP=BT_CHAR;
	pVars->BT_VAROFF=0;
	BT_CREATE (PrimeIndex, 4, FALSE, 1, 1,pVars,FALSE, 0, GWDHead.TimeStamp, FALSE);
	LocalUnlock(hVars);
	LocalFree(hVars); 
	GSSiClose (FidData);
	                 
	hDB = OpenGWDatabase (Name,BT_WRITE);
	if (!hDB) return (FALSE);
	CloseGWDatabase (hDB); 
OpenIt:	
	hDB = OpenGWDatabase (Name,BT_WRITE);
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
    pQuadData = (LPQUADDATA)&lpGWDHead->GWDData;
    *pQuadData = *pQuadDataIn;
    if (BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&pQuadDataIn->QuadID,BT_FIRST,BT_EQ, (LPSTR)&Offset))
    {
		GWDAddRecord (lpGWDHead,0,NULL);
		rtn = TRUE;
	}
	GlobalUnlock (hDB);
    CloseGWDatabase (hDB); 
	return rtn;
} 

BOOL OutputQuadData (HFILE OutFid, LPSTR Quad)
{
    GWDHEADER GWDHead; 
    LPGWDHEADER lpGWDHead;
    HANDLE  hDB;   
    long	Offset;
	LPQUADDATA	pQuadData;  
	BOOL	rtn=FALSE;
	char	Name[]="[%DL]quads.gmd"; 
	char	txt[128]; 

	hDB = OpenGWDatabase (Name,BT_READ);
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
    pQuadData = (LPQUADDATA)&lpGWDHead->GWDData;
    if (!BT_FIND (lpGWDHead->BTHandle[0],Quad,BT_FIRST,BT_EQ, (LPSTR)&Offset))
    {
        FillGWDData (lpGWDHead,Offset);
		rtn = TRUE; 
        sprintf (txt,"%i",pQuadData->ClipRect.left);
        fputstring (txt,OutFid);
        sprintf (txt,"%i",pQuadData->ClipRect.bottom);
        fputstring (txt,OutFid);
        sprintf (txt,"%i",pQuadData->ClipRect.right);
        fputstring (txt,OutFid);
        sprintf (txt,"%i",pQuadData->ClipRect.top);
        fputstring (txt,OutFid);
        sprintf (txt,"%i",pQuadData->ClipRect.left);
        fputstring (txt,OutFid);
        sprintf (txt,"%ld",pQuadData->nRows-pQuadData->ClipRect.bottom-1);
        fputstring (txt,OutFid);
        sprintf (txt,"%i",pQuadData->ClipRect.right);
        fputstring (txt,OutFid);
        sprintf (txt,"%ld",pQuadData->nRows-pQuadData->ClipRect.top-1);
        fputstring (txt,OutFid);
        sprintf (txt,"%f",pQuadData->SWClipPoint.x);
        fputstring (txt,OutFid);
        sprintf (txt,"%f",pQuadData->SWClipPoint.y);
        fputstring (txt,OutFid);
        sprintf (txt,"%f",pQuadData->NWClipPoint.x);
        fputstring (txt,OutFid);
        sprintf (txt,"%f",pQuadData->NWClipPoint.y);
        fputstring (txt,OutFid);
        sprintf (txt,"%f",pQuadData->NEClipPoint.x);
        fputstring (txt,OutFid);
        sprintf (txt,"%f",pQuadData->NEClipPoint.y);
        fputstring (txt,OutFid);
        sprintf (txt,"%f",pQuadData->SEClipPoint.x);
        fputstring (txt,OutFid);
        sprintf (txt,"%f",pQuadData->SEClipPoint.y);
        fputstring (txt,OutFid);
        sprintf (txt,"%f",pQuadData->Res);
        fputstring (txt,OutFid);
	}
	GlobalUnlock (hDB);
    CloseGWDatabase (hDB); 
	return rtn;
}  

BOOL MergeQuadData (short i)
{
    GWDHEADER GWDHead; 
    LPGWDHEADER lpGWDHead;
    HANDLE  hDB;   
    long	Offset;
    short	pos=BT_FIRST;
	LPQUADDATA	pQuadData;  
	char	Name[]="[%DL]quads2.gmd"; 
	char	QuadID[128]; 

	hDB = OpenGWDatabase (Name,BT_READ);
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
    pQuadData = (LPQUADDATA)&lpGWDHead->GWDData;
    while (!BT_FIND (lpGWDHead->BTHandle[0],QuadID,pos,BT_ANY, (LPSTR)&Offset))
    {   
    	pos = BT_NEXT;
        FillGWDData (lpGWDHead,Offset); 
		WriteQuadFile (pQuadData);
	}
	GlobalUnlock (hDB);
    CloseGWDatabase (hDB); 
	return TRUE;
}  

BOOL GetQuadVolLabel (LPSTR QuadID,LPSTR VolLabel)
{
    GWDHEADER GWDHead; 
    LPGWDHEADER lpGWDHead;
    HANDLE  hDB;   
    long	Offset;
	LPQUADDATA	pQuadData;  
	BOOL	rtn=FALSE;
	char	Name[]="[%DL]quads.gmd"; 
	char	txt[128]; 

	hDB = OpenGWDatabase (Name,BT_READ);   
	if (!hDB)
	{
		GSSiMessageBox ("Unable to open quads.gmd",NULL,MB_ICONEXCLAMATION,0);    
		return FALSE;
	}
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
    pQuadData = (LPQUADDATA)&lpGWDHead->GWDData;
    if (!BT_FIND (lpGWDHead->BTHandle[0],QuadID,BT_FIRST,BT_EQ, (LPSTR)&Offset))
    {
        FillGWDData (lpGWDHead,Offset); 
        strncpy0 (VolLabel,pQuadData->VolLabel,12);
		rtn = TRUE; 
	}
	GlobalUnlock (hDB);
    CloseGWDatabase (hDB); 
	return rtn;
}  

BOOL MarkQuadData (LPSTR Name,LPSTR Quad, LPSTR CDName, short FileNum)
{
    GWDHEADER GWDHead; 
    LPGWDHEADER lpGWDHead;
    HANDLE  hDB;   
    long	Offset;  
    short	len;
	LPCDDATA	pCDData;  
	BOOL	rtn=FALSE;

	hDB = OpenGWDatabase (Name,BT_WRITE); 
	if (!hDB)
	{   
	    BTVARDESC  *pVars;
	    short       NumFields, Reclen, len;
	    GWDHEADER GWDHead; 
	    LPGWDHEADER lpGWDHead;
	    HANDLE  hVars;
	    short       FidData,NumVars, NumSegs;
	    OFSTRUCTGM    OFStruct;
	    GWFLDINFO FldInfo;    
	    char	PrimeIndex[128];
	    LPSTR	lpDot;  
		short	ibeg=0;
        
        _fstrcpy (PrimeIndex,Name);
	    lpDot = _fstrrchr (PrimeIndex,'.');  
	    if (!lpDot)
	    	return FALSE;
	    *lpDot = 0;
	    _fstrcat (PrimeIndex,".in1");  
	    FidData = GSSiOpenFile (Name,&OFStruct,OF_CREATE); 
	    if (FidData == HFILE_ERROR)
	    	return FALSE;
	    _fmemset (&GWDHead,0,sizeof(GWDHEADER));
	    GWDHead.NumIndex=2;
	    GWDHead.Version=1;
	    GWDHead.NumIndexFields[0]=1;
	    GWDHead.IndexFields[0][0]=0;
	    GWDHead.NumIndexFields[1]=3;
	    GWDHead.IndexFields[1][0]=1;
	    GWDHead.IndexFields[1][1]=2;
	    GWDHead.IndexFields[1][2]=0;
	    BigWrite (FidData,(char *)&GWDHead,sizeof(GWDHEADER),-1);
	    ibeg = 0;
	
	    FldInfo.Len = 12;
	    FldInfo.Beg = ibeg;
	    ibeg += FldInfo.Len;
	    FldInfo.Type = BT_CHAR;
	    _fstrcpy (FldInfo.Name,"QUAD");
	    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
	    GWDHead.NumFields++;
	
	    FldInfo.Len = 6;
	    FldInfo.Beg = ibeg;
	    ibeg += FldInfo.Len;
	    FldInfo.Type = BT_CHAR;
	    _fstrcpy (FldInfo.Name,"GSSiCD");
	    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
	    GWDHead.NumFields++;
	
	    FldInfo.Len = 2;
	    FldInfo.Beg = ibeg;
	    ibeg += FldInfo.Len;
	    FldInfo.Type = BT_INTEGER;
	    _fstrcpy (FldInfo.Name,"GSSiFile");
	    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
	    GWDHead.NumFields++;
	
	    FldInfo.Len = 2;
	    FldInfo.Beg = ibeg;
	    ibeg += FldInfo.Len;
	    FldInfo.Type = BT_INTEGER;
	    _fstrcpy (FldInfo.Name,"Status");
	    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
	    GWDHead.NumFields++;
	
	    FldInfo.Len = 4;
	    FldInfo.Beg = ibeg;
	    ibeg += FldInfo.Len;
	    FldInfo.Type = BT_INTEGER;
	    _fstrcpy (FldInfo.Name,"TimeCreated");
	    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
	    GWDHead.NumFields++;
	
		GWDHead.Reclen=ibeg; 
		GWDHead.TimeStamp = time(0);
		GSSillseek (FidData,0,0);
		BigWrite (FidData,(char *)&GWDHead,sizeof(GWDHEADER),-1);
		GSSillseek (FidData,0,2);
		                 
		NumVars = 1;
		            
		hVars = LocalAlloc (LHND,NumVars * sizeof(BTVARDESC));
		pVars =(LPBTVARDESC) LocalLock(hVars);
		            
		pVars->BT_VARLEN=12;
		pVars->BT_VARTYP=BT_CHAR;
		pVars->BT_VAROFF=0;
		BT_CREATE (PrimeIndex, 4, FALSE, 1, 1,pVars,FALSE, 0, GWDHead.TimeStamp, FALSE);
		LocalUnlock(hVars);
		LocalFree(hVars); 
		GSSiClose (FidData);
		                 
		hDB = OpenGWDatabase (Name,BT_WRITE);
		if (!hDB) return (FALSE);
/*{
    GWDHEADER GWDHead1; 
    LPGWDHEADER lpGWDHead1;
    HANDLE  hDB1;   
    long	Offset; 
    short	pos=BT_FIRST;
	LPQUADDATA	pQuadData;  
	BOOL	rtn=FALSE;
	char	Name[]="[%DL]quads.gmd"; 
	char	txt[128]; 

    lpGWDHead = GlobalLock (hDB);
    pCDData = &lpGWDHead->GWDData;
	hDB1 = OpenGWDatabase (Name,BT_READ);
    lpGWDHead1 = GlobalLock (hDB1);
    pQuadData = &lpGWDHead1->GWDData;
    while (!BT_FIND (lpGWDHead1->BTHandle[0],txt,pos,BT_ANY, (LPSTR)&Offset))
    {   
    	pos = BT_NEXT;
        FillGWDData (lpGWDHead1,Offset); 
		_fstrncpy (pCDData->QuadID,pQuadData->QuadID,12);
        pCDData->GSSiFile = pQuadData->GSSiFile;
        _fstrncpy (pCDData->GSSiCD,pQuadData->GSSiCD,6);
	    GWDAddRecord (lpGWDHead,0,NULL); 
	}
	GlobalUnlock (hDB1);
    CloseGWDatabase (hDB1); 
	GlobalUnlock (hDB);
    CloseGWDatabase (hDB); 
	return TRUE;
}*/  
	}
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
    pCDData = (LPCDDATA)&lpGWDHead->GWDData;
    if (!BT_FIND (lpGWDHead->BTHandle[0],Quad,BT_FIRST,BT_EQ, (LPSTR)&Offset))
    {
        len = FillGWDData (lpGWDHead,Offset);
        pCDData->GSSiFile = FileNum;
        _fstrncpy (pCDData->GSSiCD,CDName,6);
		GWDReplaceRecord (lpGWDHead,len,NULL,Offset);
		rtn = TRUE; 
	} 
	else  
	{   
		_fstrncpy (pCDData->QuadID,Quad,12);
        pCDData->GSSiFile = FileNum;
        _fstrncpy (pCDData->GSSiCD,CDName,6);
	    GWDAddRecord (lpGWDHead,0,NULL); 
	    rtn=TRUE;
	}	
	GlobalUnlock (hDB);
    CloseGWDatabase (hDB); 
	return rtn;
}  

BOOL ImportAttributes (HANDLE hSQL)
{ 
	LPSHORT	lpVersion;
	LPWORD	lpLength;
	LPSTR	lpStr, lpEnd, lpTAB; 
    LPGWDHEADER	lpGWDHead; 
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr; 
    HANDLE	hTemp;  
    LPSTR	pTemp;
	  
    if (!hAttImport)
    	return TRUE;
   	lpVersion = (LPSHORT)GlobalLock (hAttImport);
	*lpVersion++; 
	lpLength = lpVersion++; 
	lpStr = (LPSTR)lpVersion;
	lpStr = _fstrchr (lpStr,0);

    SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
    FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
    lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle);  
    hTemp = GSSiGlobAlloc ( 535,GMEM_MOVEABLE,4096);
    pTemp = GlobalLock (hTemp);
	while (*++lpStr)
	{   
		_fstrcpy (pTemp,lpStr);
		lpEnd = _fstrchr (pTemp,0);
		if ((lpTAB = _fstrchr (pTemp,'\t')))
		{    
			*lpTAB++ = 0;
			 
			if ((lpTAB = _fstrrchr (lpTAB,'\t')))
				lpTAB++;
			else
				lpTAB = lpEnd;
		    SetFieldValFromCharAndName(lpGWDHead,pTemp,lpTAB,FALSE);
		}
    	lpStr = _fstrchr (lpStr,0);
	}
	GlobalUnlock (hAttImport);
	GWDAddRecord (lpGWDHead,0,NULL); 
	GlobalUnlock (FilePtr->FileHandle);
	GlobalUnlock (SQLPtr->OFHandle);
	GlobalUnlock (hSQL);
	GSSiGlobUlFree (&hTemp);
	return TRUE;
}

BOOL OpenImportAttributesFile (LPHANDLE phDB)
{
	LPSHORT	lpVersion;
	LPWORD	lpLength;
	LPSTR	lpStr;  
	  
    if (!hAttImport)
    	return TRUE;
   	lpVersion = (LPSHORT)GlobalLock (hAttImport);
	*lpVersion++; 
	lpLength = lpVersion++; 
	lpStr = (LPSTR)lpVersion;
    _fstrcpy (AttImportDataFile,lpStr);
    if (!ExistFile (AttImportDataFile))
    {   
		HANDLE	hDefStr=GSSiGlobAlloc (1501,GHND,USHRT_MAX);
		LPSTR	pDefStr=GlobalLock (hDefStr); 
		BOOL	First = TRUE;  
		
        lpStr = _fstrchr (lpStr,0);
        while (*++lpStr) 
        {   
        	LPSTR	pVarName = lpStr, pVarType, pVarDef;
        	
	        lpStr = _fstrchr (lpStr,0);
	        pVarType = _fstrchr (pVarName,'\t');
	        *pVarType++ = 0; 
	        pVarDef = _fstrchr (pVarType,'\t');
	        *pVarDef = 0; 
	        if (!First)
	        	_fstrcat (pDefStr,",");
	        First = FALSE;
	        sprintf (_fstrchr (pDefStr,0),"%s(%s)",pVarName,pVarType);  
	        *--pVarType = '\t';
	        *pVarDef = '\t';
        }
		CreateGWDDatabase (AttImportDataFile,1,TRUE,0,1,pDefStr);
        GSSiGlobUlFree (&hDefStr); 
    }
	GlobalUnlock (hAttImport);
    if (!OpenDataFile (AttImportDataFile,"",BT_WRITE,phDB))
    {   
        MessageBox(GetFocus(),"Cannot open output attribute file",AttImportDataFile,MB_ICONQUESTION|MB_OK);
        return FALSE;
    }
	return TRUE;
}

void CloseImportAttributesFile (LPHANDLE phDB)
{
	CloseDataFile (FALSE,phDB);    
	*AttImportDataFile = 0;
	return;
}

long LoadStreetSegData (LPSHORT Stuff,DPOINT FirstPoint, DPOINT LastPoint,LPLONG StartRefno,BOOL UniqueRefno)
{

	LPGWDHEADER lpGWDHead; 
	HANDLE		hSegData;
    //TIGER1_MN   Tiger1PN; 
	LPTIGER1_MN	pTiger1PN;
    long	StreetNums[4], StreetNums2[4], Offset, NewRefno, speed;
	short	i, j, len, ns=0, n;
	LPSHORT lpVersion, lpLength, StuffLoc; 
	char	str[256]; 
	LPSTR	lpStr;
	BOOL	OpenedSeg=FALSE;
	static	long debugref=36076132;
	short	ii;	    
	
	lpVersion = (LPSHORT)GlobalLock (hStreetSegFields);
	*lpVersion++ = 1; 
	lpLength = lpVersion++; 
	lpStr = (LPSTR)lpVersion; 
	
	OpenStreetSegmentTable (TRUE,&OpenedSeg); 
							
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetSegments);  
    pTiger1PN = (LPTIGER1_MN)&lpGWDHead->GWDData;
 
	_fmemset (pTiger1PN,0,sizeof(TIGER1_MN)); 
	_fstrcpy (str,lpStr);lpStr = _fstrchr (lpStr,0);lpStr++;
	if (!_fstricmp (str,"[%INT_REFNO]")) 
	{
		pTiger1PN->TLID = GetNextRefno (StartRefno,UniqueRefno,TRUE);
		SetIntRefno (pTiger1PN->TLID);  
	}
	else 
	{
		ExpandText (str); 
    	pTiger1PN->TLID = pTiger1PN->TLID = atol (str);  
    } 
    if (pTiger1PN->TLID == debugref)
    	ii=1;
//    Offset = GSSillseek (lpGWDHead->Fid,0,2);
//    BT_PUT (lpGWDHead->BTHandle[0],(LPSTR)&pTiger1PN->TLID,(LPSTR)&Offset);  
	
	_fstrcpy (str,lpStr);lpStr = _fstrchr (lpStr,0);lpStr++;ExpandText (str); 
    _fstrncpy (pTiger1PN->CFCC,str,3);
	_fstrcpy (str,lpStr);lpStr = _fstrchr (lpStr,0);lpStr++;ExpandText (str);Truncate (str);  
	_fmemset (StreetNums2,0,sizeof(StreetNums2));
	if (*str) 
    	StreetNums2[ns++]=AddStreetName (str,0,"","","","");
	_fstrcpy (str,lpStr);lpStr = _fstrchr (lpStr,0);lpStr++;ExpandText (str);Truncate (str);  
	if (*str) 
    	StreetNums2[ns++]=AddStreetName (str,0,"","","","");
	_fstrcpy (str,lpStr);lpStr = _fstrchr (lpStr,0);lpStr++;ExpandText (str);Truncate (str);  
	if (*str) 
	    StreetNums2[ns++]=AddStreetName (str,0,"","","","");
	_fstrcpy (str,lpStr);lpStr = _fstrchr (lpStr,0);lpStr++;ExpandText (str); Truncate (str); 
	if (*str) 
    	StreetNums2[ns++]=AddStreetName (str,0,"","","","");   
	_fmemset (StreetNums,0,sizeof(StreetNums));
    StreetNums[0] = StreetNums2[0]; 
    n = 1;
    for (i=1;i<ns;i++)
    {
    	for (j=0;j<n;j++)
    	{
    		if (StreetNums2[i] == StreetNums[j])
    			goto GotDup;
    	}
    	StreetNums[n++] = StreetNums2[i];
GotDup:;
    }
    StuffLoc = Stuff;
    StuffLoc += (*Stuff/2)+1;
    (*Stuff) += 18;
    *StuffLoc++ = 10;
    _fmemmove (StuffLoc,StreetNums,16);
	
	if (PRJ_UNITS[1] == 4)
	{
		pTiger1PN->FRLONG = IDNINT (FirstPoint.x * 100000.0);
		pTiger1PN->FRLAT = IDNINT (FirstPoint.y * 100000.0);
		pTiger1PN->TOLONG = IDNINT (LastPoint.x * 100000.0);
		pTiger1PN->TOLAT = IDNINT (LastPoint.y * 100000.0);
	}                    
    for (i=0;i<4;i++)
    	pTiger1PN->StreetNum[i]=StreetNums[i];
	_fstrcpy (str,lpStr);lpStr = _fstrchr (lpStr,0);lpStr++;ExpandText (str); 
    pTiger1PN->faddl = atol (str);
	_fstrcpy (str,lpStr);lpStr = _fstrchr (lpStr,0);lpStr++;ExpandText (str); 
    pTiger1PN->faddr = atol (str);
	_fstrcpy (str,lpStr);lpStr = _fstrchr (lpStr,0);lpStr++;ExpandText (str); 
    pTiger1PN->taddl = atol (str);
	_fstrcpy (str,lpStr);lpStr = _fstrchr (lpStr,0);lpStr++;ExpandText (str); 
    pTiger1PN->taddr = atol (str);
	_fstrcpy (str,lpStr);lpStr = _fstrchr (lpStr,0);lpStr++;ExpandText (str); 
    pTiger1PN->STATEL = atol (str);
	_fstrcpy (str,lpStr);lpStr = _fstrchr (lpStr,0);lpStr++;ExpandText (str); 
    pTiger1PN->STATER = atol (str);
	_fstrcpy (str,lpStr);lpStr = _fstrchr (lpStr,0);lpStr++;ExpandText (str); 
    pTiger1PN->COUNTYL = atol (str);
	_fstrcpy (str,lpStr);lpStr = _fstrchr (lpStr,0);lpStr++;ExpandText (str); 
    pTiger1PN->COUNTYR = atol (str);
	_fstrcpy (str,lpStr);lpStr = _fstrchr (lpStr,0);lpStr++;ExpandText (str); 
    pTiger1PN->FMCDL = atol (str);
	_fstrcpy (str,lpStr);lpStr = _fstrchr (lpStr,0);lpStr++;ExpandText (str); 
    pTiger1PN->FMCDR = atol (str);
	_fstrcpy (str,lpStr);lpStr = _fstrchr (lpStr,0);lpStr++;ExpandText (str); 
    pTiger1PN->ZIPL = atol (str);
	_fstrcpy (str,lpStr);lpStr = _fstrchr (lpStr,0);lpStr++;ExpandText (str); 
    pTiger1PN->ZIPR = atol (str);
	_fstrcpy (str,lpStr);lpStr = _fstrchr (lpStr,0);lpStr++;ExpandText (str); 
    pTiger1PN->Width = atol (str);
	_fstrcpy (str,lpStr);lpStr = _fstrchr (lpStr,0);lpStr++;ExpandText (str); 
    speed = atol (str);     
    if (speed < 0)
    {
		switch (pTiger1PN->CFCC[1])
		{
			case '1':
				speed = 60;
				break;
			case '2':
				speed = 50;
				break;
			case '3':
				speed = 45;
				break;
			default:
				speed = 30;
				break;
		}
	}
    pTiger1PN->Speed = speed;
	_fstrcpy (str,lpStr);lpStr = _fstrchr (lpStr,0);lpStr++;ExpandText (str); 
    pTiger1PN->OneWay = atol (str);
    GWDAddRecord (lpGWDHead,0,0);
 /* 	len = sizeof(Tiger1PN);
   	BigWrite (lpGWDHead->Fid,(HPSTR)&len,2,-1);
   	BigWrite (lpGWDHead->Fid,(HPSTR)&Tiger1PN,len,-1);*/ 
	GlobalUnlock (hDBStreetSegments);
	GlobalUnlock (hStreetSegFields);      
	CloseStreetSegmentTable (OpenedSeg);
	return pTiger1PN->TLID;
}









BOOL ProcessImportFilter (void)
{   
	HANDLE	hStr;
	LPSTR	pStr;
	BOOL	rtn;  
	BOOL	IRC;
	LPIMPORTFILTER	pFilter; 
	
	if (!hImportFilter)
		return TRUE;
	hStr = GSSiGlobAlloc ( 541,GMEM_MOVEABLE,4096);
	pStr = GlobalLock (hStr);    
	pFilter = (LPIMPORTFILTER)GlobalLock (hImportFilter);
	_fstrcpy (pStr,pFilter->Filter);
	GlobalUnlock (hImportFilter);  
	rtn = LogicP (pStr,&IRC);      
	GSSiGlobUlFree (&hStr); 
	if (rtn)
		return TRUE;
	return rtn;
}
 
 



HANDLE GRTextFromTextHeader (LPGRTEXTHEADER	pGRTextHead,LPSTR Text) 
{
	HANDLE		hGRText = GSSiGlobAlloc (0,GHND,sizeof(GRTEXT));
	LPGRTEXT	pGRText;   
	double		dHeight;   
	
	pGRText = (LPGRTEXT)GlobalLock (hGRText);  

	pGRText->ltext = max (0,pGRTextHead->lText);
	pGRText->UltiMapStyle = pGRTextHead->UltiMapStyle;
	if (pGRTextHead->UltiMapStyle)
	{   
		switch (pGRTextHead->hJust)
		{
			case 2:
				pGRText->hJust = -7;
				break;
			case 0:
				pGRText->hJust = 0;
				break;
			default:
				pGRText->hJust = 1; 
		}
	}   
	else
	{
		pGRText->hJust = pGRTextHead->hJust;
	}			
	pGRText->vJust = pGRTextHead->vJust;   
	pGRText->italic = pGRTextHead->italic;
	pGRText->weight = pGRTextHead->Weight;
	pGRText->FlipForEasyReading = pGRTextHead->FlipForEasyReading;
	dHeight = GetTextHeadSize (pGRTextHead); 
	ftoa (pGRText->cHeight,dHeight);
	if (pGRTextHead->HeightIsPixels)
		_fstrcat (pGRText->cHeight,"P");
	pGRText->FontNum = pGRTextHead->FontNum;              

	if (pGRTextHead->lText <= 0)
		pGRTextHead->lText = max (2,_fstrlen(Text)); 
	pGRText->ltext = pGRTextHead->lText;    
    _fstrcpy (pGRText->Text,Text);
	GlobalUnlock (hGRText); 
	
	return hGRText;
}

 


   
    
void DecodePointSym (LPSTR Stuff, LPSTR SymName,LPSTR CSize,LPSTR CRot,LPSTR CColor)
{   
	LPSTR	lpStr, lpV, lpVar[3]={CSize,CRot,CColor};
	short	i=0;
	
	*SymName=*CSize = *CRot= *CColor = 0; 
	
	lpStr=Stuff;
	lpV = SymName;
	while (*lpStr)
	{
		if (*lpStr == ';' && i<3) 
		{
			*lpV=0;
			lpV = lpVar[i++]; 
		}
		else
			*lpV++ = *lpStr;
		lpStr++;
	}
	*lpV = 0;
	return;
} 
   	
void DecodeLineSym (LPSTR Stuff, LPSTR SymName,LPSTR CSize,LPSTR CColor)
{   
	LPSTR	lpStr, lpV, lpVar[2]={CSize,CColor};
	short	i=0;
	
	*SymName=*CSize = *CColor = 0; 
	
	lpStr=Stuff;
	lpV = SymName;
	while (*lpStr)
	{
		if (*lpStr == ';' && i<2) 
		{
			*lpV=0;
			lpV = lpVar[i++]; 
		}
		else
			*lpV++ = *lpStr;
		lpStr++;
	}
	return;
} 
   	
void DecodeAreaSym (LPSTR Stuff, LPSTR SymName,LPSTR CColor)
{   
	LPSTR	lpStr, lpV, lpVar[1]={CColor};
	short	i=0;
	
	*SymName= *CColor = 0; 
	
	lpStr=Stuff;
	lpV = SymName;
	while (*lpStr)
	{
		if (*lpStr == ';' && i<1) 
		{
			*lpV=0;
			lpV = lpVar[i++]; 
		}
		else
			*lpV++ = *lpStr;
		lpStr++;
	}
	return;
} 
   	
LPSTR GetTIGERFileExtension(char FileType)
{
	LPSTR lpType, lpEnd; 
	static	char	RtnType[6];
	char	File1Ext[32]="[%TIGERFILE1EXT]";
	
	ExpandText (File1Ext);
	
	lpEnd = LastChr (File1Ext);
	*lpEnd = FileType; 
	_fstrcpy (RtnType,File1Ext); 
	_fstrlwr (RtnType);
	return RtnType;
}   

BOOL GetDOQCoord (LPSTR Name,LPMNMXCORD pBounds, LPDOUBLE pResolution,LPRECT32 ClipRect,HFILE *Fid,
							 LPHANDLE phDibInfo, LPLONG ImageOffset)
{
	OFSTRUCTGM OFStruct;
	HANDLE	hTxt=GSSiGlobAlloc ( 545,GMEM_MOVEABLE,USHRT_MAX);
	LPSTR	txt=GlobalLock (hTxt);  
	long	BeginBitmap, l, EndFile;
	LPBITMAPINFO    pDibInfo; 
	short	i;
	COLORREF	*pColor;
	
	ClipRect->left = -1;
	*Fid = GSSiOpenFile (Name,&OFStruct,OF_READ);
	if (*Fid == HFILE_ERROR) goto RtnFalse;
	
	l = (long)fgetstring (txt,82,*Fid);
	if (_fstrncmp (txt,"BEGIN_USGS_DOQ_HEADER",21))
		goto RtnFalse;  
	*phDibInfo = GSSiGlobAlloc ( 546,GHND,sizeof(BITMAPINFO)+256*sizeof(COLORREF));
	pDibInfo = (LPBITMAPINFO)GlobalLock (*phDibInfo); 
	pDibInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pDibInfo->bmiHeader.biPlanes = 1;
	pDibInfo->bmiHeader.biCompression = BI_RGB;
	while (l && _fstrncmp (txt,"END_USGS_HEADER",15))
	{   
		if (!_fstrncmp (txt,"BITS_PER_PIXEL",14))
		{
	        pDibInfo->bmiHeader.biBitCount = atoi (&txt[14]); 
	        if (pDibInfo->bmiHeader.biBitCount == 8)
	        	pDibInfo->bmiHeader.biClrUsed = 256;
		} 
	    else if (!_fstrncmp (txt,"SAMPLES_AND_LINES",17))
	    {
			pDibInfo->bmiHeader.biWidth = atoi(&txt[17]);
	        pDibInfo->bmiHeader.biHeight= atoi(&txt[24]); 
	    }     
	    else if (!_fstrncmp (txt,"XY_ORIGIN",9))
	    {
			pBounds->xmn = atof(&txt[9]);
			pBounds->ymx = atof(&txt[23]);
	    }     
	    else if (!_fstrncmp (txt,"HORIZONTAL_RESOLUTION",21))
	    	*pResolution = atof (&txt[21]);
		l = (long)fgetstring (txt,82,*Fid);
    }		
	*ImageOffset = GSSillseek (*Fid,0,1);   
	EndFile = GSSillseek (*Fid,0,2);
	pDibInfo->bmiHeader.biSizeImage = EndFile - *ImageOffset;
	pBounds->xmx = pBounds->xmn + (pDibInfo->bmiHeader.biWidth-1) * (*pResolution); 
	pBounds->ymn = pBounds->ymx - (pDibInfo->bmiHeader.biHeight-1) * (*pResolution); 
	if (pDibInfo->bmiHeader.biClrUsed == 256)
	{
		pColor = (COLORREF*)pDibInfo->bmiColors;
		for (i=0;i<256;i++)
			*pColor++ = RGB(i,i,i);
	}
	GlobalUnlock (*phDibInfo);
	if (!l)
		goto RtnFalse; 
	GSSiGlobUlFree (&hTxt);
	return TRUE;
	
RtnFalse:  
	if (*Fid != HFILE_ERROR)
		GSSiClose (*Fid);
	GSSiGlobUlFree (&hTxt);
	return FALSE;
}

BOOL AddMapToDir (HWND hWnd,LPSTR File, LPSTR Dir, LPFILEINDEX lpFI, short Type,
                  BOOL AviOut, BOOL UseFreeImage,HWND StatusWnd,
                  HFILE FidIndex, short SizeOpt, LPSTR cOrthoRes,BOOL UsesTimes,
                  LPLONG pLastHeaderOffset,LPBOOL CurHeaderWritten,LPMNMXCORD pFileBounds,BOOL UseCPT,BOOL UseExCmp)
{   
	FILEINDEXENTRY 		FileIndexEntry;
	LPFILEINDEXENTRY	lpEntry=&FileIndexEntry;
	LPMNMXCORD pBounds=&lpFI->Bounds;
	LPDOUBLE lpOrthoRes=&lpFI->OrthoRes;
	LPLONG IndexLength=&lpFI->Length;
	unsigned short FAR* NumFiles = &lpFI->NumFiles;
    HPBYTE  pRow,pWrite,pWrite2;
    HPBYTE	OrigRowData, pTIFFStrip;
    MNMXCORD    BoundSP;
    BITMAPFILEHEADER bmfHead;
    LPBITMAPINFO    pDibInfo, pDibInfoOut;
    short  width, height, nrow, ncol, irow, icol, 
		   last_height, last_width, FileNum, inc, jrow,
		   approx_width=80, approx_height=60, ires, numres=3;
	long	ClipWidth, ClipHeight, i, OrigWidth, OrigHeight;
    HANDLE  hDibInfo, hRow=0,hOutRow=0,h0s, hDibInfoOut;
    HANDLE	hTIFFOffsets=0, hTIFFLengths=0; 
    double  Resolution, MinRow, MinCol, BytesPerPel; 
    WORD    HeadLen;  
    UINT	n,m,BPP;
    OFSTRUCTGM    OFStruct;
    HFILE   FidBM, FidBMOrig=HFILE_ERROR;
    char	ClipFile[MAX_PATH];
    char    fNameBM[MAX_PATH], NameOnly[64], Ext[5], leaf[34], daterange[32],TempDir[MAX_PATH]="",IndexName[MAX_PATH];
    long    rowinc, rowinc2, colinc, rowlen, OutLen, 
			NumFile, ImageOffset, OrigBMRowLen, lenSourceRow;
    HPSTR   p0s;
    BOOL    First=TRUE, ImageIsFliped=FALSE, ImageIsTiff=FALSE;  
    RECT32	ClipRect;  
    short	iOrthoRes, iInc,sizefac[4]={1,2,4,8};
    long	FirstBufRow, OrigBMHeight,OrigRow, OrigCol, AutoClip=0,TIFFrowinc;
    short	OrigRowInc, ii, BitCount; 
    HANDLE	hRowBufs=0, hAllRows=0, hTran=0, hTIFFStrip=0;
    LPHANDLE	phOrigRow=0; 
    long	Offset, Frame=-1, Length; 
    short	TIFFCompression=0;
    short	ThisHeight,RowsPerStrip=1,PhotoInterp=1,SamplesPerPixel,PlanarConfig,BitsPerSample[8];
    BOOL	UsedAllRows, CreateOutputFile=TRUE, PCXFile=FALSE, BWTIFFile = FALSE, BWBMPFile = FALSE; 
    short	SkipBits;
    BOOL	rtn=FALSE, FileIsBMP, HaveClipFile;
   	HFILE	FidClip;
   	LPSTR	pDot;  
   	char	str[132];
	HDIB32	hDibFactored = 0;
    BITMAPINFOHEADER DibInfo;     
    double	conversion=1;
	BOOL	DoDelete=FALSE, oldMethod=FALSE;
    
	if (SizeOpt == 1)
	{
		approx_width = approx_height = 256;
	}
	else if (SizeOpt == 3)
	{
		approx_width = approx_height = 512;
	}
	else
	{
		approx_width *= sizefac[SizeOpt];
		approx_height*= sizefac[SizeOpt];
	}
	iOrthoRes = max(1, atoi(cOrthoRes));

    switch (Type)
    {
        case 5: 
		    if (PRJ_UNITS[3] == 1 && PRJ_UNITS[1] == 2)
		    	conversion = FTM;
		    else if (PRJ_UNITS[3] == 2 && PRJ_UNITS[1] == 1)
		    	conversion = MFT; 
		case 6:
        case 4: 
        case 1:
        {
        	BOOL	SaveNRI;
        	short	lDir=_fstrlen (Dir);
        	
        	sprintf (IndexName,"%s\\INDEX",Dir);
        	LoadIndexParm (IndexName);
            _fstrcpy (PltName,File);
            CurView->HaveBounds=FALSE;
            PltType = 4; 
            SaveNRI = NoRefIndex;
            NoRefIndex = TRUE;
            OpenMap (0,CurView->hDC);
            CurView->FileMNMX.xmn *= conversion;
            CurView->FileMNMX.xmx *= conversion;
            CurView->FileMNMX.ymn *= conversion;
            CurView->FileMNMX.ymx *= conversion;
            lpEntry->Bounds = CurView->FileMNMX;  
            if (Type == 4)
            {   
            	char	FName[34],cval[10];
            	short	irow,icol;
            	
		    	GetBitmapInfoFromHandle (&DibInfo,hCurImageMapDib);
	            lpEntry->BMWidth = DibInfo.biWidth;
	            lpEntry->BMHeight = DibInfo.biHeight;
	            lpEntry->BMBitCount = DibInfo.biBitCount; 
            	_splitpath (PltName,NULL,NULL,FName,NULL); 
	            strncpy0 (cval,FName,3);
	            irow = atoi (cval);
	            strncpy0 (cval,&FName[3],3);
	            icol = atoi (cval);
	            lpEntry->Bounds.xmn += icol * lpEntry->BMWidth;
	            lpEntry->Bounds.ymn += irow * lpEntry->BMHeight; 
	            lpEntry->Bounds.xmx = lpEntry->Bounds.xmn + lpEntry->BMWidth; 
	            lpEntry->Bounds.ymx = lpEntry->Bounds.ymn + lpEntry->BMHeight; 
            }
            AddMinMaxD (pBounds,&lpEntry->Bounds);  
            AddMinMaxD (pFileBounds,&lpEntry->Bounds);
            if (MapType == MT_IMAGE && Type != 3)
            {
		    	GetBitmapInfoFromHandle (&DibInfo,hCurImageMapDib);
	            lpEntry->BMWidth = DibInfo.biWidth;
	            lpEntry->BMHeight = DibInfo.biHeight;
	            lpEntry->BMBitCount = DibInfo.biBitCount; 
            }
			else if (MapType == MT_SID)
			{
	            lpEntry->BMWidth = MrSidWidth;;
	            lpEntry->BMHeight = MrSidHeight;
	            lpEntry->BMBitCount = MrSidBitCount; 
			}
            else
            { 
	            lpEntry->BMWidth = 0;
	            lpEntry->BMHeight = 0;
	            lpEntry->BMBitCount = 0; 
            }
            if (!_fstrnicmp (Dir,File,lDir))
            {  
/*            	if (*LastChr (Dir) != '\\')
	            	_fstrcpy (lpEntry->Name,&File[lDir+1]);
            	else */
	            	_fstrcpy (lpEntry->Name,&File[lDir]);
            }
            else
            {
	            _splitpath(File,0,0,NameOnly,Ext);
	            
	            _fstrcpy (lpEntry->Name,NameOnly);
	            _fstrcat (lpEntry->Name,Ext); 
	        }
            if (UsesTimes)
            {
            	sprintf (daterange,"(%ld %ld)",MinFileTime,MaxFileTime);
            	_fstrcat (lpEntry->Name,daterange);
            }
            lpEntry->Len = _fstrlen (lpEntry->Name) + 1 +
                                sizeof(FILEINDEXENTRY) - sizeof(lpEntry->Name);
            *IndexLength += lpEntry->Len;
            (*NumFiles)++;
            BigWrite (FidIndex,(LPSTR)lpEntry,lpEntry->Len,-1);
             
            CloseMap (FALSE);
            NoRefIndex = SaveNRI;
            if (*IndexLength >(USHRT_MAX - sizeof(FILEINDEXENTRY)))
            { 
                MessageBox(0,"Too many files in index - use smaller tile size",
                           0,MB_OK|MB_ICONQUESTION|MB_TASKMODAL);
                return FALSE;
            } 
            rtn = TRUE;
      }
            
            break; 
        
        case 3:
            if (!GetDOQCoord (File,&BoundSP, &Resolution,&ClipRect,&FidBMOrig,
            					   &hDibInfo,&ImageOffset)) break;
            goto ProcessBitmap;
        case 2:
		    _fstrcpy (ClipFile,File);
		    FileIsBMP = FALSE; 
    		if ((pDot = _fstrrchr (ClipFile,'.'))) 
    		{
    			if (!_fstricmp (pDot,".bpw"))
    				FileIsBMP = TRUE;
		    	_fstrcpy (pDot,".clp"); 
		    }
            HaveClipFile = ExistFile (ClipFile); 
            if (!FileIsBMP && HaveClipFile)
            	UseFreeImage = TRUE;
            if (!GetBMCoord (File,&BoundSP, &Resolution,&ClipRect,&hTran,UseCPT)) break;
            
            if (UseFreeImage)
            {   
				if (oldMethod)
				{
            		char	OutFile[MAX_PATH];
            	
					GSSiGetTempFileName (0,"gm",0,OutFile); 
            	
					if (!BMPFileFromEXT(File, OutFile, 1))
					{
						UseFreeImage = FALSE;
	//				if (!LoadBMP (hWnd,File,OutFile,TRUE,".tif"))
						break; 
					} 
					_fstrcpy (File,OutFile);
					DoDelete = TRUE;
				}
				else
				{
					HDIB32 hDib = BMPHandleFromEXT(File);
					LPBITMAPINFOHEADER pDibInfo;
					BITMAPINFOHEADER dibInfo;
					double factor;

					GetBitmapInfoFromHandle(&dibInfo, hDib);
					if (dibInfo.biBitCount == 8)
					{
						HDIB32 hDib24 = FreeImage_ConvertTo24Bits(hDib);
						GetBitmapInfoFromHandle(&dibInfo, hDib24);
						FreeImage_Unload(hDib);
						hDib = hDib24;
					}
					hDibInfo = GSSiGlobAlloc(1000,GMEM_MOVEABLE,sizeof(BITMAPINFOHEADER));
					pDibInfo = GlobalLock(hDibInfo);
					*pDibInfo = dibInfo;
					GlobalUnlock(hDibInfo);
					factor = 1.0 / iOrthoRes;
					hDibFactored = FreeImage_Rescale(hDib, dibInfo.biWidth*factor, dibInfo.biHeight*factor, FILTER_CATMULLROM);
					FreeImage_Unload(hDib);

				}
			}
            else if (_fstrstr (File,".pcx")) 
            {    
            	
				hDibInfo = BMPFromPCX (File);
            	ImageOffset = 0;   
            	approx_width = SHRT_MAX;
            	approx_height = SHRT_MAX;
				CreateOutputFile = FALSE;
				AviOut = FALSE; 
				PCXFile = TRUE;
            	goto ProcessBitmap;
            }
            FidBMOrig = GSSiOpenFile(File,&OFStruct,OF_READ);
            if (FidBMOrig == HFILE_ERROR)
            {
                MessageBox( GetFocus(),File, "Unable to open ortho bitmap", MB_OK);
                goto Exit;
            }
			if (!UseFreeImage && _fstrstr(File, ".tif"))
            {   
            	short	NumStrips;
            	
            	if (!SetupTIFHeader (FidBMOrig, &hDibInfo,&NumStrips,&hTIFFOffsets,&hTIFFLengths,
            						 &RowsPerStrip,&PhotoInterp,
									 &SamplesPerPixel,&PlanarConfig,&TIFFCompression,
									 BitsPerSample,NULL)) 
				{   
					GSSiClose (FidBMOrig);
					if (GSSiMessageBox ("Failed to open Tiff file",File,MB_ICONEXCLAMATION|MB_OKCANCEL,0) ==  IDCANCEL)
						goto Exit; 
					rtn = TRUE;
					goto Exit;
				}  
				
	            pDibInfo = (LPBITMAPINFO)GlobalLock (hDibInfo); 
	            if (pDibInfo->bmiHeader.biBitCount == 1)
	            {   
	            	GSSiClose (FidBMOrig); 
	            	FidBMOrig = HFILE_ERROR;
					GlobalUnlock (hDibInfo);
	            	ImageOffset = 0;   
	            	approx_width = SHRT_MAX;
	            	approx_height = SHRT_MAX;
					CreateOutputFile = FALSE;
					AviOut = FALSE; 
					BWTIFFile = TRUE;
	            	goto ProcessBitmap;
	            }
            	ImageOffset = 0;
	            if (RowsPerStrip == pDibInfo->bmiHeader.biHeight && TIFFCompression < 2)
	            {   
            		HPULONG	pOffset=(HPULONG)GlobalLock (hTIFFOffsets);  
            		
            		ImageOffset = *pOffset;
	            	GSSiGlobUlFree (&hTIFFOffsets);
	            	GSSiGlobFree (&hTIFFLengths);
	            }
	            GlobalUnlock (hDibInfo);     
	            ImageIsTiff = TRUE;
            	ImageIsFliped = TRUE;
            }
			else if (oldMethod)
			{            
            	ReadBitMapHeader (FidBMOrig, &hDibInfo,&ImageOffset);
	            pDibInfo = (LPBITMAPINFO)GlobalLock (hDibInfo); 
	            if (pDibInfo->bmiHeader.biBitCount == 1)
	            {   
	            	approx_width = SHRT_MAX;
	            	approx_height = SHRT_MAX;
					CreateOutputFile = FALSE;
					AviOut = FALSE; 
					BWBMPFile = TRUE;
	            } 
	            GlobalUnlock (hDibInfo);     
	        }
            
ProcessBitmap:
            if (!hDibInfo)
            	goto Exit;
            pDibInfo = (LPBITMAPINFO)GlobalLock (hDibInfo);
            OrigBMHeight = pDibInfo->bmiHeader.biHeight; 
            BitCount = pDibInfo->bmiHeader.biBitCount;
            if ((AutoClip = GetGlobalLVal2 ("[%AUTOCLIP]",0)))
            {
            	ClipRect.left = ClipRect.bottom = AutoClip;
            	ClipRect.top = pDibInfo->bmiHeader.biHeight - AutoClip;
            	ClipRect.right = pDibInfo->bmiHeader.biWidth - AutoClip;
            	ClipWidth = ClipRect.right - ClipRect.left + 1;
            	ClipHeight = ClipRect.top - ClipRect.bottom + 1;
            }
    		else if ((FidClip = GSSiOpenFile (ClipFile,NULL,OF_READ)) != HFILE_ERROR)
    		{ 
		    	double	minx, miny, maxx, maxy, x, y, WX[4], WY[4], BMPX[4], BMPY[4]; 
		    	HANDLE	hTran=0;
                float	RSQMIN;
                double	conversion=1;
                
/*			    if (PRJ_UNITS[3] == 1 && PRJ_UNITS[1] == 2)
			    	conversion = FTM;
			    else if (PRJ_UNITS[3] == 2 && PRJ_UNITS[1] == 1)
			    	conversion = MFT; */
		    	fgetstring (str,128,FidClip);
			    n=sscanf (str,"%Flf %Flf %Flf %Flf",&minx, &miny,&maxx, &maxy);
	    		GSSiClose (FidClip);
			    WX[0] = WX[1] = BoundSP.xmn*conversion;
			    WX[2] = WX[3] = BoundSP.xmn*conversion + (pDibInfo->bmiHeader.biWidth-1) * fabs (Resolution);  
			    if (Resolution < 0)
			    {
				    WY[1] = WY[2] = BoundSP.ymn*conversion; 
				    WY[0] = WY[3] = BoundSP.ymn*conversion + (pDibInfo->bmiHeader.biHeight-1)* Resolution; 
				}
			    else 
			    {
				    WY[0] = WY[3] = BoundSP.ymn*conversion; 
				    WY[1] = WY[2] = BoundSP.ymn*conversion + (pDibInfo->bmiHeader.biHeight-1)* Resolution; 
				}
			    BMPX[0] = BMPX[1] = 0;
			    BMPX[2] = BMPX[3] = pDibInfo->bmiHeader.biWidth;
			    BMPY[0] = BMPY[3] = pDibInfo->bmiHeader.biHeight;
			    BMPY[1] = BMPY[2] = 0;
		        hTran = STRAN2 (1667,WX,WY,BMPX,BMPY,4,(LPFLOAT)&RSQMIN,1,NULL); 
	        	TRANS2 (minx,miny,&x,&y,hTran); 
	        	ClipRect.left = IDNINT (x);
	        	ClipRect.bottom = IDNINT (pDibInfo->bmiHeader.biHeight-y);
	        	TRANS2 (maxx,maxy,&x,&y,hTran);  
        	    CloseTRANS2 (&hTran);
	        	ClipRect.right = IDNINT (x);
	        	ClipRect.top = IDNINT (pDibInfo->bmiHeader.biHeight-y);
            	ClipWidth = ClipRect.right - ClipRect.left + 1;
            	ClipHeight = ClipRect.top - ClipRect.bottom + 1;
	    	}
            else if (ClipRect.left < 0)
            {
            	ClipWidth = pDibInfo->bmiHeader.biWidth;
            	ClipHeight =pDibInfo->bmiHeader.biHeight; 
            	ClipRect.left = ClipRect.bottom = 0;
            	ClipRect.top = pDibInfo->bmiHeader.biHeight;
            	ClipRect.right = pDibInfo->bmiHeader.biWidth;
            }
            else
            {
            	ClipWidth = ClipRect.right - ClipRect.left + 1;
            	ClipHeight = ClipRect.top - ClipRect.bottom + 1;
            }  
            OrigWidth = ClipWidth;
            OrigHeight = ClipHeight;
            ClipWidth /= iOrthoRes;
            ClipHeight /= iOrthoRes;
            OrigBMRowLen = pDibInfo->bmiHeader.biSizeImage/pDibInfo->bmiHeader.biHeight;
            if (Resolution < 0) 
            {   
	            BoundSP.ymx = BoundSP.ymn;
	            BoundSP.ymn += (pDibInfo->bmiHeader.biHeight-1) * Resolution;
	            Resolution = fabs (Resolution);
	        }
	        else 
	        {
	            BoundSP.ymx = BoundSP.ymn + (pDibInfo->bmiHeader.biHeight-1) * Resolution; 
	        }
            Resolution *= iOrthoRes;  
	        BoundSP.xmn += ClipRect.left * Resolution/iOrthoRes;
            BoundSP.xmx = BoundSP.xmn + (ClipWidth-1) * Resolution; 
	        BoundSP.ymn += ClipRect.bottom * Resolution/iOrthoRes;
            BoundSP.ymx = BoundSP.ymn + (ClipHeight-1) * Resolution; 
            if (UseCPT)
            {   
            	HANDLE	hTran;  
            	char	CPTFile[128];
            	LPSTR	pExt;
            	
            	_fstrcpy (CPTFile,File);
            	
            	pExt = _fstrrchr (CPTFile,'.');
            	_fstrcpy (pExt,".cpt");
            	hTran = LoadTranFile (CPTFile,1,1,NULL,NULL);
		        Resolution = TranScale (hTran,&BoundSP);
		        TranBounds (hTran,&BoundSP); 
		        CloseTRANS2 (&hTran);
            }
            *lpOrthoRes = ComputeRes (ClipHeight,ClipWidth,BoundSP);
            BytesPerPel = (double)pDibInfo->bmiHeader.biBitCount/4;  //times 2 for 16 color files
            rowinc = pDibInfo->bmiHeader.biSizeImage / pDibInfo->bmiHeader.biHeight;
            if (ImageIsTiff)
            	TIFFrowinc = IDNINT(pDibInfo->bmiHeader.biWidth*BytesPerPel)/2; 
            else
            	TIFFrowinc = rowinc;
            ncol = 1+(ClipWidth-1) / approx_width;
            nrow = 1+(ClipHeight-1) / approx_height;
            hRow = GSSiGlobAlloc ( 547,GMEM_MOVEABLE,OrigBMRowLen);
            h0s = GSSiGlobAlloc ( 548,GHND,OrigBMRowLen);
            p0s = GlobalLock (h0s);
            width  = IDNINT(BytesPerPel*approx_width)/2;  
            
            colinc = width;
            height = approx_height;
            last_width  = IDNINT(ClipWidth * BytesPerPel)/2 - ((ncol-1) * (long)width);
            last_height = ClipHeight - ((nrow-1) * (long)height); 
            
            HeadLen = sizeof(BITMAPINFOHEADER)+pDibInfo->bmiHeader.biClrUsed*sizeof(RGBQUAD);
            hDibInfoOut = GSSiGlobAlloc ( 549,GHND,HeadLen);
            pDibInfoOut = (LPBITMAPINFO)GlobalLock (hDibInfoOut); 
            _fmemmove (pDibInfoOut,pDibInfo,HeadLen);  
            if (!pDibInfoOut->bmiHeader.biSize)
				pDibInfoOut->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            
            _splitpath (File,0,0,NameOnly,0);
            FileNum = 0;  
            NumFile = nrow * ncol;
            if (AviOut)
            {
            	GetTempDir (TempDir);
            	sprintf (_fstrchr(TempDir,0),"\\%s",NameOnly); 
            	makedirectories (TempDir,TRUE,FALSE);
            }
            else
                _fstrcpy (TempDir,Dir);

            for (irow = 0; irow < nrow; irow++)
            {    
            	ii=1; 
				for (icol = 0; icol < ncol; icol++)
				{
					FileNum++;
					/*_fstrcpy (fNameBM,Dir);
					_fstrcat (fNameBM,"\\");
					_fstrcat (fNameBM,NameOnly);*/
					sprintf(fNameBM, "%s\\%s.%3.3x", TempDir, NameOnly, FileNum % 4096);
					if (PCXFile)
					{
						short	l = _fstrlen(Dir);

						if (!_fstrnicmp(Dir, File, l))
							_fstrcpy(leaf, &File[l]);
						else
							sprintf(leaf, "%s.pcx", NameOnly);
					}
					else if (BWTIFFile)
						sprintf(leaf, "%s.tif", NameOnly);
					else if (BWBMPFile)
						sprintf(leaf, "%s.bmp", NameOnly);
					else
						sprintf(leaf, "%s.%3.3x", NameOnly, FileNum % 4096);
					if (oldMethod)
						iInc = iOrthoRes;
					else
						iInc = 1;
					{
					if (icol == ncol - 1)
					{
						inc = last_width % 4;
						if (inc) inc = 4 - inc;
						rowlen = (last_width + inc);
						if (AviOut)
							pDibInfoOut->bmiHeader.biWidth = approx_width;
						else if (BytesPerPel)
							pDibInfoOut->bmiHeader.biWidth = IDNINT(2 * last_width / BytesPerPel);
					}
					else
					{
						/*    rowlen = colinc;*/
						inc = width % 4;
						if (inc) inc = 4 - inc;
						rowlen = (width + inc);

						pDibInfoOut->bmiHeader.biWidth = IDNINT(2 * width / BytesPerPel);
					}
					if (!CreateOutputFile)
						goto WriteIndex;
					OutLen = rowlen;
					GSSiGlobFree(&hOutRow);
					hOutRow = GSSiGlobAlloc(550, GMEM_MOVEABLE, OutLen);
					if (irow == nrow - 1 && !AviOut)
						pDibInfoOut->bmiHeader.biHeight = last_height;
					else
						pDibInfoOut->bmiHeader.biHeight = height;

					pDibInfoOut->bmiHeader.biSizeImage =
						IDNINT((long)pDibInfoOut->bmiHeader.biHeight *
						(long)pDibInfoOut->bmiHeader.biWidth *
						(long)BytesPerPel) / 2;

					FidBM = GSSiOpenFile(fNameBM, &OFStruct, OF_CREATE);
					bmfHead.bfType = 19778;
					bmfHead.bfSize = sizeof(BITMAPFILEHEADER)+HeadLen +
						pDibInfoOut->bmiHeader.biSizeImage;
					bmfHead.bfReserved1 = 0;
					bmfHead.bfReserved2 = 0;
					bmfHead.bfOffBits = HeadLen + sizeof(BITMAPFILEHEADER);
					if (pDibInfoOut->bmiHeader.biClrUsed == 2)
					{
						RGBQUAD clrtab[] = { { 0, 0, 0, 0 },
						{ 255, 255, 255, 0 } };
						if (PhotoInterp == 1)
						{
							pDibInfoOut->bmiColors[0] = clrtab[0];
							pDibInfoOut->bmiColors[1] = clrtab[1];
						}
						else
						{
							pDibInfoOut->bmiColors[1] = clrtab[0];
							pDibInfoOut->bmiColors[0] = clrtab[1];
						}
					}
					BigWrite(FidBM, (char *)&bmfHead, sizeof(BITMAPFILEHEADER), -1);
					BigWrite(FidBM, (char *)pDibInfoOut, HeadLen, -1);
					OrigRow = ClipRect.bottom + irow * (long)height * iInc;
					OrigCol = ClipRect.left + icol * IDNINT((2 * (long)width) / BytesPerPel) * iInc;
					MinRow = OrigRow;
					MinCol = OrigCol;
					OrigRowInc = iInc;
					if (ImageIsFliped)
					{
						rowinc2 = -rowinc;
						OrigRow = OrigBMHeight - OrigRow - 1;
						OrigRowInc = -iInc;
					}
					else
						rowinc2 = rowinc;
					FirstBufRow = max(0, min(OrigRow, OrigRow + (pDibInfoOut->bmiHeader.biHeight - 1)*OrigRowInc));

				SkipAllRows:
					for (jrow = 0; jrow < pDibInfoOut->bmiHeader.biHeight; jrow++)
					{
						pRow = GlobalLock(hRow);
						pWrite = GlobalLock(hOutRow);
						phOrigRow = 0;
						UsedAllRows = FALSE;
						if (irow == nrow - 1)
							ThisHeight = last_height;
						else
							ThisHeight = height;

						if (irow == nrow - 1 && jrow >= last_height)
							pWrite = p0s;
						else
						{
							if (hAllRows)
							{
								HPBYTE pAllRows = GlobalLock(hAllRows);

								pAllRows += ((long)(OrigRow - FirstBufRow)*rowinc);
								OrigRowData = pAllRows;
								UsedAllRows = TRUE;
							}
							else if (oldMethod)
							{
								long	RowInc = 1, ThisRow = FirstBufRow;
								long	CurTIFFStrip;

								if (!hRowBufs)
								{
									hRowBufs = GSSiGlobAlloc(551, GHND, sizeof(HANDLE)*height);
									phOrigRow = (LPHANDLE)GlobalLock(hRowBufs);
									if (ImageIsFliped)
									{
										phOrigRow += (ThisHeight - 1);
										RowInc = -1;
									}
									CurTIFFStrip = -1;
									for (i = 0; i < ThisHeight; i++)
									{
										DWORD	OffsetD;
										if (ThisRow < OrigBMHeight)
										{
											*phOrigRow = GSSiGlobAlloc(552, GMEM_MOVEABLE, rowinc);
											OrigRowData = GlobalLock(*phOrigRow);
											if (hTIFFOffsets)
											{
												HPULONG	pOffset = (HPULONG)GlobalLock(hTIFFOffsets);
												HPLONG	pLength = (HPLONG)GlobalLock(hTIFFLengths);
												long	Strip = ThisRow / RowsPerStrip,
													StripOffset = (ThisRow%RowsPerStrip)*TIFFrowinc;

												if (Strip != CurTIFFStrip)
												{
													pOffset += Strip;
													pLength += Strip;
													OffsetD = (DWORD)*pOffset;// + StripOffset; 
													Length = *pLength;
													GSSiGlobFree(&hTIFFStrip);
													hTIFFStrip = GSSiGlobAlloc(553, GMEM_MOVEABLE, max(rowinc*RowsPerStrip, Length));
													pTIFFStrip = GlobalLock(hTIFFStrip);
													GSSillseek2(FidBMOrig, OffsetD, 0);
													BigRead(FidBMOrig, pTIFFStrip, Length);
													DecompressTIFF(pTIFFStrip, Length, rowinc*RowsPerStrip, TIFFCompression);
													GlobalUnlock(hTIFFStrip);
												}
												GlobalUnlock(hTIFFOffsets);
												GlobalUnlock(hTIFFLengths);
												CurTIFFStrip = Strip;
												pTIFFStrip = GlobalLock(hTIFFStrip);
												hmemmove(OrigRowData, (HPBYTE)(pTIFFStrip + StripOffset), TIFFrowinc);
												GlobalUnlock(hTIFFStrip);
											}
											else
											{
												Length = TIFFrowinc;
												OffsetD = (DWORD)ImageOffset + (DWORD)ThisRow * (DWORD)TIFFrowinc;
												GSSillseek2(FidBMOrig, OffsetD, 0);
												BigRead(FidBMOrig, OrigRowData, Length);
											}
											GlobalUnlock(*phOrigRow);
											CheckForContinue(TRUE,0);
										}
										ThisRow += iInc;
										phOrigRow += RowInc;
									}
									GlobalUnlock(hRowBufs);
									GSSiGlobFree(&hTIFFStrip);
								}
								phOrigRow = (LPHANDLE)GlobalLock(hRowBufs);
								phOrigRow += jrow;
								OrigRowData = GlobalLock(*phOrigRow);
								lenSourceRow = OrigBMRowLen;
							}
							else
							{
								BITMAPINFOHEADER dibInfo;

								GetBitmapInfoFromHandle(&dibInfo, hDibFactored);
								OrigRowData = FreeImage_GetScanLine(hDibFactored, OrigRow);
								lenSourceRow = dibInfo.biSizeImage / dibInfo.biHeight;
								OrigWidth = dibInfo.biWidth;
							}
							OrigRow += OrigRowInc;
							OrigRowData += OrigCol*IDNINT(BytesPerPel / 2);
							MoveHtoF(pRow, OrigRowData, lenSourceRow - (OrigCol*IDNINT(BytesPerPel / 2)));
							if (phOrigRow)
							{
								GlobalUnlock(*phOrigRow);
								GlobalUnlock(hRowBufs);
							}
							if (UsedAllRows)
								GlobalUnlock(hAllRows);
							BPP = max(1, IDNINT(BytesPerPel / 2));
							pWrite2 = pWrite;
							hmemset(pWrite2, 0, OutLen);
							{
								UINT	i, j, jinc = iInc*BPP;
								if (ImageIsTiff)
								for (i = 0, j = 0; i < OutLen - BPP + 1 && j < OrigWidth*BPP; i += BPP, j += jinc, pRow += jinc)
								{
									n = BPP;
									while (n--)
										*pWrite2++ = *(HPBYTE)(pRow + n);
								}
								else
								for (i = 0, j = 0; i < OutLen - BPP + 1 && j < OrigWidth*BPP; i += BPP, j += jinc, pRow += jinc)
								{
									n = BPP;
									m = 0;
									while (n--)
										*pWrite2++ = *(HPBYTE)(pRow + m++);
								}
							}
						}
						if (BigWrite(FidBM, pWrite, OutLen, -1) != OutLen)
						{
							char	mess[128];

							sprintf(mess, "Error writing %ld bytes to bitmap on chan %i - disk may be full", (long)OutLen, FidBM);
							MessageBox(GetFocus(), mess, OFStruct.szPathName, MB_OK | MB_ICONQUESTION | MB_TASKMODAL);
							GSSiClose(FidBM);
							GSSiClose(FidBMOrig);
							GSSiGlobUlFree(&h0s);
							GSSiGlobUlFree(&hRow);
							GSSiGlobUlFree(&hOutRow);
							GSSiGlobUlFree(&hDibInfo);
							GSSiGlobUlFree(&hDibInfoOut);
							GSSiGlobFree(&hTIFFOffsets);
							GSSiGlobFree(&hTIFFLengths);
							goto Exit;
						}
						GlobalUnlock(hOutRow);
						GlobalUnlock(hRow);
						if (AviOut && rowlen < width)
							BigWrite(FidBM, p0s, width - rowlen, -1);
						CheckForContinue(TRUE,0);
					}
					GSSiClose(FidBM);
				}
                  if (AviOut)
                  {  
                     LPBITMAPINFOHEADER pDIB;
                     HDIB   hDIB, hNewDIB;
                     char AVIFile[MAX_PATH];
                     BOOL   st;  
                     long	nc;
                     char	OrthoImageExtension[6]=".gci";
                     
                     _fstrcpy (AVIFile,Dir);
                     sprintf (AVIFile,"%s\\orthos%s%s",Dir,cOrthoRes,OrthoImageExtension);
                      
                     hDIB = LoadDIB (fNameBM);
//                     nc = NumDIBColors (hDIB);
                     pDIB = (LPBITMAPINFOHEADER)GlobalLock (hDIB);
                     if (pDIB->biBitCount == 8 && Convert8to24)
                     {
					 	hNewDIB = ConvertBitmap8To24 (pDIB); 
					 	GlobalUnlock (hDIB);
					 	DestroyDIB (hDIB);
					 	pDIB = (LPBITMAPINFOHEADER)GlobalLock (hNewDIB);
					 	hDIB = hNewDIB;
					 }                       
                     st = AVIOut (AVIFile,pDIB,&hAVIFile,&Frame,UseExCmp);
				 	 GlobalUnlock (hDIB);
                     DestroyDIB (hDIB);
                     GSSiRemove (fNameBM);
                     if (!st)
                     {
                        MessageBox(0,"Error writing output file - disk may be full",
                                     AVIFile,MB_OK|MB_ICONQUESTION|MB_TASKMODAL);
                     	ContinueProcessing = FALSE;
                     	goto Exit;
                     }
                     if (st == -1)
                     {
                        MessageBox(0,"Error writing output file - maximum file size exceeded",
                                     AVIFile,MB_OK|MB_ICONQUESTION|MB_TASKMODAL);
                     	ContinueProcessing = FALSE;
                     	goto Exit;
                     }
                     if (st == -2)
                     {
                        MessageBox(0,"Error writing output file - exit program and retry using external compression",
                                     AVIFile,MB_OK|MB_ICONQUESTION|MB_TASKMODAL);
                     	ContinueProcessing = FALSE;
                     	goto Exit;
                     }
                  }
       WriteIndex:                  
                  if (irow == nrow-1)
                    lpEntry->BMHeight = last_height; 
                  else
                    lpEntry->BMHeight = height; 
                    
                  if (icol == ncol-1)
                    lpEntry->BMWidth = (2*(short)IDNINT(last_width/BytesPerPel)); 
                  else
                    lpEntry->BMWidth = (2*(short)IDNINT(width/BytesPerPel));
                  lpEntry->BMBitCount = BitCount; 
                  
                  lpEntry->Bounds.xmn = BoundSP.xmn + (icol * 2 * (double)width/BytesPerPel) * (*lpOrthoRes);
                  lpEntry->Bounds.ymn = BoundSP.ymn + (irow * (double)height) * (*lpOrthoRes);  
                  if (hTran)
					TRANS2 (MinCol,MinRow,&lpEntry->Bounds.xmn,&lpEntry->Bounds.ymn,hTran);                   	
                  lpEntry->Bounds.xmx = lpEntry->Bounds.xmn + 
                                           ((double)lpEntry->BMWidth - 1) * (*lpOrthoRes);
                  lpEntry->Bounds.ymx = lpEntry->Bounds.ymn + 
                                           ((double)lpEntry->BMHeight - 1) * (*lpOrthoRes);
                  AddMinMaxD (pBounds,&lpEntry->Bounds);
		          AddMinMaxD (pFileBounds,&lpEntry->Bounds); 
                  if (PCXFile || BWTIFFile || BWBMPFile)
                  	sprintf (lpEntry->Name,"%s",leaf);
                  else if (First)
                  	sprintf (lpEntry->Name,"%s(%i)",leaf,UserDefinedImageQuality/100);
                  else
                  	sprintf (lpEntry->Name,".%3.3x",FileNum%4096);
                  First = FALSE;
                  if (Frame >= 0)
                  	sprintf (_fstrchr(lpEntry->Name,0),"@%ld",Frame);
                  lpEntry->Len = _fstrlen (lpEntry->Name) + 1 +
                                sizeof(FILEINDEXENTRY) - sizeof(lpEntry->Name);
                  *IndexLength += lpEntry->Len;
                    if (*IndexLength >(USHRT_MAX - sizeof(FILEINDEXENTRY)))
                    { 
                        MessageBox(0,"Too many files in index - use larger tile size",
                                   0,MB_OK|MB_ICONQUESTION|MB_TASKMODAL);
                        goto Exit;
                    }
                  (*NumFiles)++;
                  BigWrite (FidIndex,(HPSTR)lpEntry,lpEntry->Len,-1);  

				 if (Type > 1 && lpFI->NumFiles > 255)
				 {   
	             	long CurOffset = GSSillseek (FidIndex,0,1);
					lpFI->NextHeaderOffset = CurOffset;
					GSSillseek (FidIndex,*pLastHeaderOffset,0);
					BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1);
					GSSillseek (FidIndex,CurOffset,0);
					DBoundsInit (&lpFI->Bounds);   
					lpFI->NumFiles=0;
					lpFI->Length=0; 
					*pLastHeaderOffset = CurOffset;
					BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1);
					*CurHeaderWritten = TRUE;
                  } 
                  else
                  	*CurHeaderWritten = FALSE;
                  
                  PctBox (StatusWnd, NumFile,FileNum,0);    
                  if (!ContinueProcessing)
                  	goto Exit;
                }
                FreeRowBuffers (&hRowBufs,height);     
                GSSiGlobFree (&hAllRows); 
            } 
            
		    if (ContinueProcessing)
		    	rtn = TRUE;  
Exit:       
            if (AviOut && *TempDir)
            	DeleteDirAndContents (TempDir);
            FreeRowBuffers (&hRowBufs,height);
            GSSiGlobFree (&hAllRows); 
            if (FidBMOrig != HFILE_ERROR)
            	GSSiClose (FidBMOrig); 
			if (hDibFactored)
				FreeImage_Unload(hDibFactored);

            GSSiGlobFree (&hRow);
            GSSiGlobFree (&hOutRow);
            GSSiGlobUlFree (&h0s);
            GSSiGlobUlFree (&hDibInfo);
            GSSiGlobUlFree (&hDibInfoOut);
			GSSiGlobFree (&hTIFFOffsets);  
			GSSiGlobFree (&hTIFFLengths);                      
			CloseTRANS2 (&hTran); 
            break;
    } 
    if (DoDelete)
    	GSSiRemove (File);
    return rtn;
}

void FreeRowBuffers (LPHANDLE phRowBufs, short nrows)
{   
	LPHANDLE	phBuf;
	
	if (!*phRowBufs)
		return;
	phBuf = (LPHANDLE)GlobalLock (*phRowBufs);
	while (nrows--)
		GSSiGlobFree (phBuf++); 
	GSSiGlobUlFree (phRowBufs);
		
	return;
}  

LPSTR GetMSLinkType (LPSTR hextype)
{   
	short	i;
	static	char	Types[8][10]={"DMRS","INFORMIX","ODBC","ORACLE","RIS","SYBASE","XBASE","????"};
	char	HexVal[7][6]={"0000","3848","5e62","6091","71Fb","4f58","1971"}; 
	
	for (i=0;i<6;i++)
		if (!_fstricmp (hextype,HexVal[i]))
			return Types[i];
	return Types[7];
}
	
	

BOOL SetDGNAttribute (HFILE Fid, LPSTR str, LPSTR pENum, LPSTR WantENum,LPSTR MSLinkVals)
{
//  short	la = _fstrlen (str);
//	char	MSLinkC[10];   
	long	MSLink,ii; 
	LPSTR	pENumLoc, pMSLinkLoc, pTypeLoc;
	
    SetGlobalValueLong ("%MSLINK",-1);
    *pENum = 0; 
    *MSLinkVals = 0;
    while (fgetstring (str,256,Fid)) 
    {
    	if (!*str)
    		return TRUE;
    	if ((pTypeLoc = _fstrstr (str,"Type=0x")))
    	{
    		LPSTR pEnd = _fstrchr (pTypeLoc+7,',');
    		
    		if (pEnd)
    		{
	    		*pEnd++ = 0;
		    	if ((pENumLoc = _fstrstr (pEnd,"EntityNum=")))
		    	{   
		    		LPSTR pEnd2 = _fstrchr (pENumLoc+10,','); 
		    		
		    		if (pEnd2)
		    		{
			    		*pEnd2++ = 0;
				    	if ((pMSLinkLoc = _fstrstr (pEnd2,"MSLink=")))
				    	{   
				    		sprintf (pENum,"%s:%s",pTypeLoc+7,pENumLoc+10); 
				    		if (*MSLinkVals)
				    			_fstrcat (MSLinkVals,"|");
				    		sprintf (_fstrchr (MSLinkVals,0),"%s:%s:%s",GetMSLinkType(pTypeLoc+7),pENumLoc+10,pMSLinkLoc+7);
			    			if (!_fstricmp (pENum,WantENum))
				        	{
						    		MSLink = atol (pMSLinkLoc+7);
					if(MSLink==79556)
					ii=1;
					        		SetGlobalValueLong ("%MSLINK",MSLink);
				        	}
			        	}
			        }
		        }
		    }
    	}
    }
/*    if (la > 16 && *AttImportDataFile)
    {
	    sprintf (MSLinkC,"%c%c%c%c%c%c%c%c",str[la-10],str[la-9],str[la-4],str[la-3],str[la-6],str[la-5],str[la-8],str[la-7]);
	    MSLinkC[8] = 0;
        sscanf (MSLinkC,"%Flx",&MSLink); 
    } */
    return FALSE;
}

BOOL ConvertUsingSymlist (LPSTR SymName,short SymType) 
{
	char	str[128],SymListFile[128]="[%DL]symlists\\[MAPID].txt";
	HFILE	Fid = GSSiOpenFile (SymListFile,NULL,OF_READ);
	LPSTR	pPar;
	char	TypeC[3][6]={"POINT","LINE","AREA"};
	
	if (Fid == HFILE_ERROR)
		return FALSE;
	
	while (fgetstring (str,120,Fid))
	{
		if ((pPar = _fstrchr (str,',')))
		{
			*pPar++ = 0;
			if (!_fstrcmp (SymName,str))
			{
				sprintf (SymName,"%s_%s",pPar,TypeC[SymType-1]);
				break;
			}
		}		
	}
	GSSiClose (Fid);
	return TRUE;
}



