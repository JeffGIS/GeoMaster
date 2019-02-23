#include "graphint.h"
#include "dgnlib.h"      
//#include "dgn7.h"      

typedef DGNElemCore *LPDGNElementCore;

#define DGN_DESCRIPTION_NAME_LENGTH   34
#define DGN_DESCRIPTION_DESCRIPTION_LENGTH 68
#define MAX_DGN_DESC	70  
#define MAX_DGN_TAGS	8    
#define NUM_ATT_TYPES	9
typedef struct
{
 short   level;
 char   name        [ DGN_DESCRIPTION_NAME_LENGTH        ];
 char   description [ DGN_DESCRIPTION_DESCRIPTION_LENGTH ];
} DGN_Description;
typedef DGN_Description	FAR	*LPDGN_Description;
 
typedef struct       // Type 66 Level 6
{
 short   number_of_descriptions;
 DGN_Description description [ 1 ];
 
} DGN_Level_Names;  
typedef DGN_Level_Names	FAR	*LPDGN_Level_Names;

typedef struct	{char Name[8];
				 long	record;} CELLLIBRARYENTRY;
typedef CELLLIBRARYENTRY	FAR	* LPCELLLIBRARYENTRY;

static	CELLLIBRARYENTRY	CellLib[1024];
static	short	NumCells;

static	HANDLE	hLayerSym=0;
static	short	nLayerSym;
		 
static HANDLE			hDGN=0, hDGNCell=0;
static char				DGNParms[4096]="";     
static MNMXCORD			DGNLastBounds;   
static char				DGNTag[100]="", DGNTAG[100]="", DGNText[256]=""; 
static long				DGNBaseRefno=0, DGNMaxRefPerFile=0; 
static HANDLE			hElement=0;     
static LPDGNElementCore pElement; 
static DGN_Description	DGNDesc[MAX_DGN_DESC]; 
static short			DGNSymbolNum[MAX_DGN_DESC]; 
static short			DGNSymbolLevel[MAX_DGN_DESC];
static short			NumDGNSymbol=0;
static BOOL				WantDGNDesc=FALSE; 
static BOOL				AreasOnlyThisFile=FALSE;
static short			DGNBaseSymNum=3000; 
static COLORREF			DGNColorTable[256]; 
static long				FillColor;
static long				NumAttributes;
static long				Attributes[30];
static DWORD			MaxDGNElementSize=500000L;
static double			DGNPixelSize; 
static char				CurrentDGNFile[256];  
static short			DGNIndexType=0; 
static short			NumDGNTags=0;
static char				DefaultTAG[256];
static char				DGNTagPrefix[MAX_DGN_TAGS][10];
static char				DGNTagUDI[MAX_DGN_TAGS][66];
static char				DGNTagDatabase[MAX_DGN_TAGS][128];
static char				DGNTagDatabaseKeyField[MAX_DGN_TAGS][34];
static HANDLE			DGNTaghDB[MAX_DGN_TAGS]; 
static long				DGNTagType[MAX_DGN_TAGS]; 
static long				DGNTagEntity[MAX_DGN_TAGS]; 
static long				AttTypeID[NUM_ATT_TYPES]={DGNLT_DMRS,DGNLT_INFORMIX,DGNLT_ODBC,DGNLT_ORACLE,DGNLT_RIS,DGNLT_SYBASE,
												 DGNLT_XBASE,DGNLT_SHAPE_FILL,DGNLT_ASSOC_ID};
static char				AttTypeChar[NUM_ATT_TYPES+1][10]={"DMRS","INFORMIX","ODBC","ORACLE","RIS","SYBASE",
												      "XBASE","FILL","ASSID","UNKNOWN"};
												
static BYTE	ColorUsed[256];

#include "gmextern.h"

BOOL OpenDGNFile (LPSTR InName,LPMNMXCORD pBounds)
{   
	OFSTRUCTGM	OFStruct;
	HFILE		Fid;

//	SetWindowText (hWndMain,InName);	
	hDGN = 0;
	DGNNumElements = 0;
	OpenDGNCellLibrary ("[%CELLLIBRARY]");
	_fstrcpy (CurrentDGNFile,InName);
	ExpandText (CurrentDGNFile);
	_fmemset (DGNDesc,0,sizeof(DGNDesc));
	Fid = GSSiOpenFile (CurrentDGNFile,&OFStruct,OF_READ);
	if (Fid == HFILE_ERROR)
		return FALSE;
	GSSiClose2 (&Fid);	
	hDGN = DGN7Open (OFStruct.szPathName,0,pBounds);  
	if (!hDGN)
		return FALSE;
	DGNNumElements = DGNGetNumElements (hDGN);
	DBoundsInit (&DGNLastBounds); 
	if (!hElement)
		hElement = GSSiGlobAlloc (1,GMEM_MOVEABLE,MaxDGNElementSize);
	memset (ColorUsed,0,sizeof(ColorUsed));
	return TRUE;
}

int notype (int not)
{
	int j = not;

	return j;
}
BOOL CloseDGNFile (void)
{  
	BOOL	st = FALSE;
	if (hDGN)
		st = DGN7Close (hDGN);  
    hDGN = 0;  
    DGNNumElements = 0;
    GSSiGlobFree (&hElement);
    return st;
} 

BOOL LoadDGNColorFile (LPSTR InName)
{
	char	Name[MAX_PATH], str[66];
	LPSTR	pDot;
	HFILE	Fid;

	strcpy (Name,InName);
	pDot=strrchr (Name,'.');

	if (pDot)
		strcpy (pDot,".clr");
	else
		strcat (Name,".clr");
	Fid = GSSiOpenFile (Name,0,OF_READ);
	if (Fid == HFILE_ERROR)
		return FALSE;
	memset (DGNColorTable,0,sizeof(DGNColorTable));
	while (fgetstring (str,64,Fid))
	{
		int	index,r,g,b;

		if (sscanf (str,"%i,%i,%i,%i",&index,&r,&g,&b) == 4)
			DGNColorTable[index] = RGB (r,g,b);
	}
	
	GSSiClose2 (&Fid);
	return TRUE;
}

BOOL LoadDGNParm (LPSTR DGNFileName)
{
	char	Name[256], str[260], Projection[34], Units[34];
//	char	SymName[66], cWidth[64],cRot[64],cColor[64], cIF[128];
	LPSTR	pDot, pTAG,pWidth, pTAB, pParm=DGNParms, pBS; 
	short	l; 
	BOOL	FileIsIndex, Err=FALSE, rtn=FALSE; 
	HFILE	Fid;
    
    if (!DGNFileName)
    {   
    	while (NumDGNTags--)
    	{
    		CloseDataFile (TRUE,&DGNTaghDB[NumDGNTags]);
    	}
	    NumDGNTags = 0;
	    NumDGNSymbol = 0;
    	HaveIndexParmFile = FALSE;  
    	AreasOnlyThisFile = FALSE;
    	return TRUE;
    }
    if (HaveIndexParmFile) 
    {
    	DGNVisSetFromIndex = TRUE;
    	return TRUE; 
    }
    NumDGNTags = 0;
	_fstrcpy (Name,DGNFileName); 
	ExpandText (Name); 
	l = _fstrlen (Name);
	pBS = strrchr(Name, '\\');
	if (l > 4 && !_fstricmp(&Name[l - 5], "INDEX"))
	{
		HaveIndexParmFile = FALSE;
		FileIsIndex = TRUE;
		pDot = &Name[l];
	}
	else if (pBS && !strnicmp (++pBS,"INDEX",5))
	{
		HaveIndexParmFile = FALSE;
		FileIsIndex = TRUE;
		pDot = strchr(pBS, 0);
	}
	else
	{
		FileIsIndex = FALSE;
		pDot = _fstrrchr (Name,'.');   
		GetGlobalCVal("[%DefaultDGNProjection]", Projection, "BASEPROJ");
		LoadProjection(0, Projection);
		GetGlobalCVal("[%DefaultDGNUnits]", Units, "FEET");
		if (!_fstricmp(Units, "FEET"))
			PRJ_UNITS[0] = 1;
		else if (!_fstricmp(Units, "METERS"))
			PRJ_UNITS[0] = 2;
		else
			PRJ_UNITS[0] = 4;
	}
	if (!pDot)
		goto Exit;
	if (UseDGNColors)
		LoadDGNColorFile (Name);
	_fstrcpy (pDot,".gdp");
	Fid = GSSiOpenFile (Name,0,OF_READ); 
	if (Fid == HFILE_ERROR)
		goto Exit;
//    _fstat (Fid,&statParmFile);
//    DGNParmTime = statParmFile.st_mtime;    
	fgetstring (str,32,Fid);  
	AreasOnlyThisFile = atob (str);
	if (FileIsIndex)
		HaveIndexParmFile = TRUE;
	fgetstring (Projection,32,Fid); 
	if (!*Projection)
		GetGlobalCVal ("[%DefaultDGNProjection]",Projection,"BASEPROJ"); 
	LoadProjection(0,Projection); 
	fgetstring (Units,32,Fid);
	if (!*Units)
		GetGlobalCVal ("[%DefaultDGNUnits]",Units,"FEET"); 
	if (!_fstricmp (Units,"FEET"))
		PRJ_UNITS[0] = 1;
	else if (!_fstricmp (Units,"METERS"))
		PRJ_UNITS[0] = 2; 
	else
		PRJ_UNITS[0] = 4;
	fgetstring (str,32,Fid);  
	DGNBaseRefno = atol (str);
	if ((pTAB = _fstrchr (str,'\t')))
		DGNMaxRefPerFile = atol (++pTAB);
	else
		DGNMaxRefPerFile = 0;
	fgetstring (str,32,Fid); 
	DGNIndexType = atoi (str);
	_fmemset (DGNParms,0,sizeof(DGNParms));  
	while (fgetstring (str,256,Fid))
	{   
		LPSTR pLoc = str;
		
		pTAB = _fstrchr (pLoc,'\t');
		
		if (!_fstricmp (str,"-SYMBOLS-"))
			goto GetSymbols;
		DGNTaghDB[NumDGNTags] = 0;
		*DGNTagDatabase[NumDGNTags] = 0;
		if (pTAB)
			*pTAB++ = 0;
		_fstrcpy (DGNTagPrefix[NumDGNTags],str);  
		if (pTAB)
		{
			pLoc = pTAB;
			if ((pTAB = _fstrchr (pLoc,'\t')))
				*pTAB++ = 0;
			DGNTagType[NumDGNTags] = atol (pLoc); 
			if (pTAB)
			{
				pLoc = pTAB;
				if ((pTAB = _fstrchr (pLoc,'\t')))
					*pTAB++ = 0;
				DGNTagEntity[NumDGNTags] = atol (pLoc); 
				if (pTAB)
				{
					pLoc = pTAB;
					if ((pTAB = _fstrchr (pLoc,'\t')))
						*pTAB++ = 0;
					_fstrcpy (DGNTagDatabase[NumDGNTags],pLoc); 
					if (pTAB)
					{
						pLoc = pTAB;
						if ((pTAB = _fstrchr (pLoc,'\t')))
							*pTAB++ = 0;
						_fstrcpy (DGNTagDatabaseKeyField[NumDGNTags],pLoc); 
						if (pTAB)
						{   
							char	SQL[80];
							
							pLoc = pTAB;
							if ((pTAB = _fstrchr (pLoc,'\t')))
								*pTAB++ = 0;
							_fstrcpy (DGNTagUDI[NumDGNTags],pLoc);   
							sprintf (SQL,"%s = [%%MSLINK]",DGNTagDatabaseKeyField[NumDGNTags]);
							if (!OpenDataFile (DGNTagDatabase[NumDGNTags],SQL,BT_READ,&DGNTaghDB[NumDGNTags]))
								Err = TRUE;
						}
					}
				}
			}
		}
		else
			DGNTagType[NumDGNTags] = -1;    
		if (!_fstricmp (DGNTagPrefix[NumDGNTags],"REFNO"))
			DGNTagEntity[NumDGNTags] = -1;	
		else if (!_fstricmp (DGNTagPrefix[NumDGNTags],"ATTRIBUT"))
			DGNTagEntity[NumDGNTags] = -2;	
		NumDGNTags++;
	} 
GetSymbols:
	while (fgetstring (str,256,Fid))
	{   
		LPSTR pLoc = str;
		LPSTR pTAB = _fstrchr (pLoc,'\t');
		
		if (pTAB)
		{
			*pTAB++ = 0;
			DGNSymbolNum[NumDGNSymbol]=GetSymbolNum (str);    
			if (!DGNSymbolNum[NumDGNSymbol])  
				MessageBox (0,"Symbol not in dictionary",str,MB_ICONEXCLAMATION);
			DGNSymbolLevel[NumDGNSymbol++] = atoi (pTAB); 
		}
	}
	GSSiClose2 (&Fid);    
	rtn = TRUE;
Exit:
	if (GetGlobalCVal ("[%DefaultDGNTAG]",DefaultTAG,"REFNO") && strcmp (DefaultTAG,"REFNO"))
	{
		DGNTagType[NumDGNTags] = -1;    
		DGNTagEntity[NumDGNTags] = -3;
		DGNTaghDB[NumDGNTags] = 0;	
		_fstrcpy (DGNTagPrefix[NumDGNTags++],"MACRO");  
	}
	else
	{
		DGNTagType[NumDGNTags] = -1;    
		DGNTagEntity[NumDGNTags] = -1;
		DGNTaghDB[NumDGNTags] = 0;	
		_fstrcpy (DGNTagPrefix[NumDGNTags++],"REFNO");  
	}
	return rtn;
} 

BOOL ReadNextDGNRecord (LPMNMXCORD pBounds)
{   
	BOOL	rtn; 
	
	if (!hDGN)
		return FALSE;  
	if (pBounds)
	{
		MNMXCORD	Bounds=*pBounds; 
		
		if (IgnoreBounds)
			Bounds.xmn = Bounds.ymn = Bounds.xmx = Bounds.ymx = 0;   
		pBounds = &Bounds;
		if (_fmemcmp (pBounds,&DGNLastBounds,sizeof(MNMXCORD)))
		{   
			MNMXCORD	DGNBounds;
			
			if (pBounds->xmn || pBounds->xmx || pBounds->ymn || pBounds->ymx)
			{
				if (ConvertRectCoord (&DGNBounds,pBounds,1,0))
					DGNLibSetSpatialFilter(hDGN,&DGNBounds); 
			}
			else
				DGNLibSetSpatialFilter(hDGN,pBounds); 
			DGNLastBounds = *pBounds;
		}
	} 
	pElement = (DGNElemCore*)GlobalLock (hElement);
	rtn = DGNLibReadElement (hDGN,pElement,MaxDGNElementSize,&BaseDistToWinDist,&FillColor,&NumAttributes,Attributes);  
	CurrentDGNRec = pElement->element_id;
	GlobalUnlock (hElement);
	return rtn;
} 

BOOL GetDGNRecordBounds (DWORD Recno,LPMNMXCORD pBounds)
{   
	MNMXCORD	TempBounds;
	
	if (!hDGN)
		return FALSE;
	if (!DGN7GotoElement (hDGN,Recno))
		return FALSE;
	if (!DGNLibGetElementExtents (hDGN,pBounds))
		return FALSE;
	TempBounds = *pBounds; 
	ConvertRectCoord (pBounds, &TempBounds,0,1);
	GetFileMinMax (&CurrentItemMinMax,pBounds);
	return TRUE; 
}

BOOL DumpDGNSyms (LPSTR DGNFile,LPSTR DumpFile)
{   
	short	i;
	char	str[256];
	
	_fstrcpy (PltName,DGNFile);
	if (OpenMap ((HWND)1,0)) 
	{
		SetDGNVis (0,0,0,HFILE_ERROR);
		for (i=0;i<MAX_DGN_DESC;i++)
		{
			if (*DGNDesc[i].description)
			{
				sprintf (str,"%s\t%i",DGNDesc[i].name,(int)DGNDesc[i].level);
				AppendFile (DumpFile,str);
			}
		}
    	CloseMap (FALSE);
    } 
    return TRUE;
}		

BOOL IsDGNFileVisible (void)
{
	short	idesc,i; 
	
    if (!NumDGNSymbol)
		return TRUE;
	for (i=0;i<NumDGNSymbol;i++)
	{
		idesc = DGNSymbolNum[i];  
    	if(GetVisibility(idesc)) 
    		return TRUE;
	}
	return FALSE;
}

BOOL SetDGNVis (HWND hWndDlg, int DlgItemSym, int DlgItemPar,HFILE FidSymList)
{
	int		i,idesc, iparent,SymType=2; 
	char	DescName[100], SymDesc[100]; 
	BOOL	parent = FALSE;
    MNMXCORD	Bounds;    
    static	BOOL	Create=FALSE;
    int		iAll = GetDictSymbolNumber ("ALL");
    
    if (NumDGNSymbol)
    {   
    	for (i=0;i<NumDGNSymbol;i++)  
    	{   
    		idesc = DGNSymbolNum[i];
    		GetDictSymName (idesc,DescName); 
    		GetDictSymDescription (idesc,SymDesc);  
			iparent = GetDictSymParent (idesc); 
			sprintf (_fstrchr (DescName,0),"\t%s",SymDesc);
	    	if(GetVisibility(idesc))
	    		_fstrcat (DescName,"\t<on>\t");
	    	else
	    		_fstrcat (DescName,"\t\t");
	    	sprintf (_fstrchr(DescName,0),"%i\t%i",iparent,idesc);
			if (FidSymList != HFILE_ERROR)
			{
				sprintf (strchr (DescName,0),"\t%i",CurView->CurFile);
				fputstring (DescName,FidSymList);
			}
			else
			{
				if (SendDlgItemMessage ((HWND)hWndDlg,DlgItemSym,LB_FINDSTRING,-1,(LPARAM) DescName) ==  LB_ERR)
					SendDlgItemMessage ((HWND)hWndDlg,DlgItemSym,LB_ADDSTRING,0,(LPARAM) DescName); 
				idesc = iparent;
    			GetDictSymName (idesc,DescName); 
				ConvertSymName (DescName,1,TRUE,idesc);  
				iparent = GetDictSymParent (idesc); 
	    		if(GetVisibility(idesc))
	    			_fstrcat (DescName,"\t<on>\t");
	    		else
	    			_fstrcat (DescName,"\t\t");
	    		sprintf (_fstrchr(DescName,0),"%i\t%i",iparent,idesc);
				if (SendDlgItemMessage ((HWND)hWndDlg,DlgItemPar,LB_FINDSTRING,-1,(LPARAM) DescName) ==  LB_ERR)
					SendDlgItemMessage ((HWND)hWndDlg,DlgItemPar,LB_ADDSTRING,0,(LPARAM) DescName); 
			}
    	}
    	return TRUE;
    }
    Bounds.xmn = Bounds.ymn = 0;
    Bounds.xmx = Bounds.ymx = 1;
    WantDGNDesc = TRUE;
	while (ReadNextDGNRecord (&Bounds))
		ProcessDGNRecord (0,-1);
	WantDGNDesc = FALSE;
	
	if (!hWndDlg && !DlgItemSym)
		return TRUE;
	if (DlgItemSym<0)
	{       
		for (i=MAX_DGN_DESC-1;i>0;i--)
		{
			idesc = DGNDesc[i].level + DGNBaseSymNum;  
			if (DlgItemSym == -9999)
			{  
				if (!_fstricmp (CurrentSymName,DGNDesc[i].name)) 
				{
					CurrentSymNum = idesc; 
					return TRUE;
				}
			}
			else
			{
				if (idesc==-DlgItemSym) 
				{
					_fstrcpy (CurrentSymName,DGNDesc[i].name);
					CurrentSymParent = idesc + 100;   
					CurSymIsParent = FALSE; 
					return TRUE;
				}
			}
		}
		return FALSE;
	} 
	
    
	for (i=0;i<MAX_DGN_DESC;i++)
		if (*DGNDesc[i].description)
			goto HaveDesc;  
 	DGN7Rewind (hDGN);
    Bounds.xmn = Bounds.ymn = Bounds.xmx = Bounds.ymx = 0;
	while (ReadNextDGNRecord (&Bounds))
	{   
		if (pElement->level > 0)                        
			if (!*DGNDesc[pElement->level].description) 
			{ 
				sprintf (DGNDesc[pElement->level].description,"Level %i",(int)pElement->level);
			}
	}
HaveDesc:
	for (i=0;i<MAX_DGN_DESC;i++)
	{
//		idesc = DGNDesc[i].level + DGNBaseSymNum;  
//		iparent = idesc + 100; 
		if (*DGNDesc[i].description)
		{
			sprintf (DescName,"DGNLevel%2.2i",i);
			idesc = GetOrCreateSym (hWndDlg,DescName,0,0,Create,SymType);
			if (!idesc)
			{
				if (MessageBox (0,"DGN Level symbols not in dictionary\rDo you wish to create them?",0,MB_YESNO)==IDNO)
					return FALSE;
				idesc = GetOrCreateSym (hWndDlg,DescName,0,0,TRUE,SymType); 
				Create=TRUE;
			}
//			_fstrcpy (DescName,DGNDesc[i].name);
			sprintf (_fstrchr (DescName,0),"\t%s",DGNDesc[i].description);
	    	if(GetVisibility(idesc))
	    		_fstrcat (DescName,"\t<on>\t");
	    	else
	    		_fstrcat (DescName,"\t\t"); 
	    	iparent = GetDictSymParent (idesc);
	    	sprintf (_fstrchr(DescName,0),"%i\t%i",iparent,idesc);
			if (SendDlgItemMessage ((HWND)hWndDlg,DlgItemSym,LB_FINDSTRING,-1,(LPARAM) DescName) ==  LB_ERR)
				SendDlgItemMessage ((HWND)hWndDlg,DlgItemSym,LB_ADDSTRING,0,(LPARAM) DescName); 
			idesc = iparent;
			iparent = iAll; 
			GetDictSymName (idesc,DescName);
//			sprintf (DescName,"DGN Level %2i",DGNDesc[i].level);
	        ConvertSymName (DescName,1,TRUE,idesc);  
	    	if(GetVisibility(idesc))
	    		_fstrcat (DescName,"\t<on>\t");
	    	else
	    		_fstrcat (DescName,"\t\t");
	    	sprintf (_fstrchr(DescName,0),"%i\t%i",iparent,idesc);
			if (SendDlgItemMessage ((HWND)hWndDlg,DlgItemPar,LB_FINDSTRING,-1,(LPARAM) DescName) ==  LB_ERR)
				SendDlgItemMessage ((HWND)hWndDlg,DlgItemPar,LB_ADDSTRING,0,(LPARAM) DescName); 
		}
	}
	_fstrcpy (DescName,"ALL"); 
	iparent = 0; 
	idesc = iAll;
	if(GetVisibility(idesc))
		_fstrcat (DescName,"\t<on>\t");
	else
		_fstrcat (DescName,"\t\t");
	sprintf (_fstrchr(DescName,0),"%i\t%i",iparent,iAll);
	if (SendDlgItemMessage ((HWND)hWndDlg,DlgItemPar,LB_FINDSTRING,-1,(LPARAM) DescName) ==  LB_ERR)
		SendDlgItemMessage ((HWND)hWndDlg,DlgItemPar,LB_ADDSTRING,0,(LPARAM) DescName); 

	return TRUE;
}


BOOL SetDGNParms ()
{   
	LPSTR	pDesc, pTAG, pClause, pC, pColor, pWidth, pRot; 
	short	rc;
	char	str[256], SymName[66];  
	short	ii,i,j,k=0,itype;
    
	pElement = (LPDGNElementCore)GlobalLock (hElement);
	CurPointSize = 1;
	CurrentDGNRec = pElement->element_id;
	CurrentRefno = DGNBaseRefno + (FileInIndex)*DGNMaxRefPerFile + pElement->element_id;
	DGNWidth = pElement->weight;
	DGNStyle = pElement->style;
	DGNColor = DGNColorTable[pElement->color]; 
	SetGlobalValueLong ("%DGNWidth",DGNWidth);
	SetGlobalValueLong ("%DGNStyle",DGNStyle);
	SetGlobalValueLong ("%DGNColor",DGNColor);
	ColorUsed[pElement->color] = 1;
	if (FillColor >= 0)
		DGNFillColor = DGNColorTable[FillColor];
	else
		DGNFillColor = -1; 
	if (NumAttributes)
		ii = Attributes[0];  
	for (i=0;i<NumDGNTags;i++)
	{
		if (DGNTagType[i] < 0)
			goto FoundTag;
		k = 0;
		for (j=0;j<NumAttributes;j++)
		{  
			if (DGNTagType[i] == Attributes[k] && DGNTagEntity[i] == Attributes[k+1])
				goto FoundTag;  
			k += 3; 
		}
	} 
	i=NumDGNTags-1;
FoundTag:
	strncpy0 (CurrentPrefix,DGNTagPrefix[i],MAX_PREFIX_LEN);
	switch (DGNTagEntity[i])
	{
		case -1://REFNO  
			ltoa (CurrentRefno,CurrentUDI,10);
		break;
		
		case -3://MACRO  
			{
				LPSTR pColon;

				strcpy (str,DefaultTAG);
				ExpandText (str);
				if ((pColon = strchr (str,':')))
				{
					*pColon++ = 0;
					strncpy0 (CurrentUDI,pColon,MAX_UDI_LEN);
					strncpy0 (CurrentPrefix,str,MAX_PREFIX_LEN);
				}
				else
					ltoa (CurrentRefno,CurrentUDI,10);
			}
		break;
		
		case -2://ATTRIBUT  
			*CurrentUDI = 0; 
			for (j=0;j<NumAttributes;j++)
			{   
				k = j*3;
				for (itype = 0;itype < NUM_ATT_TYPES;itype++)
				{
					if (Attributes[k] == AttTypeID[itype])
						goto FoundAttType;
				}
				itype = NUM_ATT_TYPES;
	FoundAttType:
				sprintf (_fstrchr(CurrentUDI,0),"(%s|%ld|%ld)",AttTypeChar[itype],Attributes[k+1],Attributes[k+2]);
			}
		break;
		
		default://all others  
			SetGlobalValueLong ("%MSLink",Attributes[k+2]);
			strncpy0 (CurrentUDI,DGNTagUDI[i],MAX_UDI_LEN);
			ExpandText (CurrentUDI); 
	}
	_fstrcpy (DGNTag,CurrentPrefix);
	_fstrcat (DGNTag,":");
	_fstrcat (DGNTag,CurrentUDI); 
	pDesc = DGNParms;
	for (i=0;i<NumDGNSymbol;i++)
	{
		if (pElement->level == DGNSymbolLevel[i])
		{
			CurrentDesc = DGNSymbolNum[i];
				goto Exit; 
		}
	}
	if (NumDGNSymbol)
		CurrentDesc = 0;
	else
	{
		CurrentDesc = pElement->level + DGNBaseSymNum; 
		sprintf (SymName,"DGNLevel%2.2i",(int)pElement->level);
		CurrentDesc = GetOrCreateSym (hWndMain,SymName,0,0,FALSE,2); 
	}
Exit:  
	GlobalUnlock (hElement);
	return TRUE;
}    

BOOL ProcessDGNRecord (HDC hDC,long Recno)
{
    HPDPOINT    pPoints, pFirstPoint;
    LPLONG      pPartIndex;   
    ULONG		i,j,n;
    short		nPoly, Type, ltag;
    HANDLE		hPoints, hPartIndex, hPolyPartLen;  
    LPINT		pNumPoints;
    long		NumPoints=0; 
    HPEN		hOldPen=0, hTempPen=0;
	HBRUSH		hOldBrush=0, hDeletePen=0, hTempBrush=0;  
	DPOINT		DPoint;
	double		size;
	HANDLE		hElemInfo=0;
	HANDLE		hOrd=0;   
	LPLONG		pElemInfo;
	LPDOUBLE	pOrd; 
	long		ElemInfoLoc;
	short		ElemType, Interp, NumParts=1; 
	long		loopfactor=1; 
	static		int		jj=0;  
	static		long		debugitem=1365;
	DGNPoint*	pvertices; 
	long num_vertices = 0;
	LPLONG		pnum_vertices=&num_vertices;
	MNMXCORD	Rect;
	LPFLOAT		pElev;
	int MaxMemSize = 1024 * 1024;

	pElement = (LPDGNElementCore)GlobalLock (hElement);
	if (Recno >= 0)  
	{   
		DGNLibSetSpatialFilter(hDGN,0); 
		if (!DGN7GotoElement (hDGN,Recno))
			goto RtnFalse;
		DGNLibReadElement (hDGN,pElement,MaxDGNElementSize,&DGNPixelSize,&FillColor,&NumAttributes,Attributes);  
	}		
	ItemSeg = CurrentDGNRec;  
	if (ForceRefIndex || ForceTAGIndex)
    	StatusWindowUpdate2 (0,CurPltFileLen,CurrentDGNRec); 
	if (ItemSeg != debugitem && jj)
	{
		GlobalUnlock(hElement);
		return FALSE;
	}
	switch (pElement->type)
	{
		case 33: //??
		case 56: //??
			goto RtnFalse;
		case DGNT_DIGITIZER_SETUP:       
		case DGNT_TCB: 
			goto RtnFalse;
		case DGNT_LEVEL_SYMBOLOGY: 
			goto RtnFalse;
		case DGNT_3DSURFACE_HEADER:     
		case DGNT_3DSOLID_HEADER:       
		case DGNT_CONE:                 
			goto RtnFalse;
		case DGNT_SHARED_CELL_DEFN:  
			goto RtnFalse;   
		case DGNT_SHARED_CELL_ELEM:     
			goto RtnFalse;
		case DGNT_TAG_VALUE:  
			goto RtnFalse;          
		case DGNT_GROUP_DATA:
			if (pElement->level == DGN_GDL_COLOR_TABLE)  
			{
				DGNElemColorTable FAR	*pRec=(DGNElemColorTable*)pElement; 
				
				if (UseDGNColors)
					for (i=0;i<256;i++)
						DGNColorTable[i] = RGB (pRec->color_info[i][0],pRec->color_info[i][1],pRec->color_info[i][2]);
				else
					_fmemset (DGNColorTable,0,sizeof(DGNColorTable));
			}
			goto RtnFalse;           
		case DGNT_APPLICATION_ELEM:
		{
			short	plnoff=108,i=0; 
			LPDGN_Level_Names	pln=(LPDGN_Level_Names)((LPSTR)pElement + plnoff);  
			switch (pElement->level)
			{
				case 0:
				break;

				case 6:
				if (WantDGNDesc)
				{
					
					while (pln->number_of_descriptions--)
					{   
						
						if (pln->description[i].level < MAX_DGN_DESC-1)
							DGNDesc[pln->description[i].level] = pln->description[i++];
						else
							i++;
					}
				} 
				break;
				
				default:
					ii=1;
			}
			goto RtnFalse;  
		}
		default:
		break;
	}
	ItemIsDeleted = FALSE; 
   	ItemIsRemoved = FALSE;              	
	if (pElement->deleted)
	{
		if (!ShowDeletedOpt)
			goto RtnFalse; 
		ItemIsDeleted = TRUE;
	}
	else if (ShowDeletedOpt)
		goto RtnFalse;
	ShowValue (hDC,FALSE);
	InGraphicsProcessor = TRUE;
	InitRecord (hDC); 
    SetDGNParms ();  
    if (!CurrentDesc)
		goto RtnFalse;
	SetSymNum (CurrentDesc);
    ltag = _fstrlen (DGNTag);
	if (!ProcessRefAndTAG (GetVisibility (CurrentDesc),DGNTag,ltag))
		goto RtnFalse; 
	if (Pick)
    	GetDGNRecordBounds (CurrentDGNRec,&Rect);
	else if (hDC && Display)
	{
		SetROP2(hDC,DisplayRasterOpt);
		if (HaveVarFillColor)
			SetTextColor (hDC,ConvertColor(GlobalColors[0],CurrentDesc));
		else if (UseDGNColors)
			SetTextColor (hDC,ConvertColor(DGNColor,CurrentDesc));
		else
			SetTextColor (hDC,ConvertColor(DefaultTextColor,CurrentDesc));
		SelectObject (hDC,GetStockObject(BLACK_PEN));
		GSSiDeleteObject (&hBlackPen);
		hBlackPen = CreatePen (PS_SOLID,0,ConvertColor(0,CurrentDesc));		
		SelectObject (hDC,hBlackPen);
	}
	HiPrecis = TRUE; 
	CurrentPen = 0; 
//	else
//		goto RtnFalse; 
	if (pElement->stype == DGNST_MULTIPOINT)
	{
		DGNElemMultiPoint FAR	*pRec=(DGNElemMultiPoint*)pElement;   
		num_vertices = pRec->num_vertices;
		pvertices = pRec->vertices; 
	}
	switch (pElement->type)
	{   
		default:  
			notype (pElement->stype);
			ii=1;
			break;
		case DGNT_CELL_LIBRARY:
			break;          
		case DGNT_CELL_HEADER:
		{
			DGNElemCellHeader FAR	*pRec=(DGNElemCellHeader*)pElement; 
			break;          
		} 
		case DGNT_COMPLEX_SHAPE_HEADER:  
		{
			DGNElemComplexHeader FAR	*pRec=(DGNElemComplexHeader*)pElement;  
			long	NumElems = pRec->numelems;
			
			if (CurView->PassID && CurView->PassID != 4 && CurView->PassID != 2)
				goto RtnFalse; 
			if (!GetTypeVisibility(TYPE_AREA))
				goto RtnFalse;
		    if (CurrentDesc < 3201)
				CurView->CurVisType[CurrentDesc]=3;  
	        nPoly = 1;  
	        NumPoints = 0; 
//	        if (pElement->type == DGNT_COMPLEX_SHAPE_HEADER)
//	        	NumPoints = NumPoints+nPoly-1;
	        hPartIndex = GSSiGlobAlloc (1418,GMEM_MOVEABLE,sizeof(long)*(nPoly+1));
	        hPolyPartLen = GSSiGlobAlloc (1419,GMEM_MOVEABLE,sizeof(USHORT)*(nPoly+1));
			hPoints = GSSiGlobAlloc (1420,GMEM_MOVEABLE,sizeof(DPOINT)*(long)USHRT_MAX); 
	        pPartIndex = (LPLONG)GlobalLock (hPartIndex);
	        *pPartIndex = 0;

			pPoints = pFirstPoint = (LPDPOINT)GlobalLock (hPoints); 
			while (ReadNextDGNRecord (0))
			{   
				NumElems--;  
				switch (pElement->stype) 
				{
					case DGNST_MULTIPOINT:
					{
						DGNElemMultiPoint FAR	*pRec=(DGNElemMultiPoint*)pElement; 

						for (i=0;i<pRec->num_vertices;i++)
						{
							pPoints[NumPoints].x = pRec->vertices[i].x;
							pPoints[NumPoints].y = pRec->vertices[i].y;
							ConvertCoord(&pPoints[NumPoints],0,1);
							if (!NumPoints || !SameDPoint (&pPoints[NumPoints-1],&pPoints[NumPoints]))
								NumPoints++;
						}
					}
					break;
					 
					case DGNST_ARC:              
					{
						int  size = sizeof(DGNElemArc);

						DGNElemArc FAR	*pRec=(DGNElemArc*)pElement;
						DGNPoint	*pvertices;
						DGNPoint	TestPointsDGN[5];
						DPOINT		TestPoints[5];
						int			MaxPoints = (MaxMemSize - size - 4) / sizeof (DGNPoint);
						double		dlen;
						static int displayArc = 1;
					/*	DGNStrokeArc(hDGN, (DGNElemArc *)pElement, 5, TestPointsDGN);
						for (int i = 0; i < 5; i++)
						{
							TestPoints[i].x = TestPointsDGN[i].x;
							TestPoints[i].y = TestPointsDGN[i].y;
							ConvertCoord(&TestPoints[i], 0, 1);
						}
						dlen = GetPolyLengthD(TestPoints, 5);
						num_vertices = min(MaxPoints - 1, max(2, (int)(dlen / CurView->MetersPerPixel))) + 1;*/
						num_vertices = 5;
						pvertices = (DGNPoint*)malloc((num_vertices + 2) * sizeof(DGNPoint));

						//convert quaternion to counter-clockwise rotation in degrees on z axis.
						double w = (pRec->quat[0] / (double)INT_MAX);
						double x = (pRec->quat[1] / (double)INT_MAX);
						double y = (pRec->quat[2] / (double)INT_MAX);
						double z = (pRec->quat[3] / (double)INT_MAX);

						double t0 = (2.0 * ((w * z) + (x * y)));
						double t1 = (1.0 - (2.0 * ((y * y) + (z * z))));
						double rotation = (atan2(t0, t1) * RADtoDEG);
						pRec->rotation = -rotation;
						
						DGNStrokeArc(hDGN, (DGNElemArc *)pElement, num_vertices, pvertices);
						
						if (!displayArc)
							num_vertices = 0;

						for (i = 0; i < num_vertices; i++)
						{
							pPoints[NumPoints + i].x = pvertices[i].x;
							pPoints[NumPoints + i].y = pvertices[i].y;
							ConvertCoord(&pPoints[NumPoints + i], 0, 1);
						}
						free(pvertices);
						NumPoints += num_vertices;
						/*for (i = 0; i < num_vertices; i++)
						{
							if (!NumPoints || !SameDPoint(&pPoints[NumPoints - 1], &pPoints[NumPoints]))
							NumPoints++;
						}*/
						break;
					/*case DGNT_CURVE:
					{
						DGNElemMultiPoint *pRec = (DGNElemMultiPoint *)pDGNElemCore;
						dlen = PolyLength(pRec->num_vertices, pRec->vertices);
						*pnum_vertices = min(MaxPoints - 1, max(2, (int)(dlen / (*pPixelSize)))) + 1;
						DGNStrokeCurve(hDGN, (DGNElemMultiPoint *)pDGNElemCore, *pnum_vertices, pvertices);
					}*/
					}
					break;

					case DGNST_COLORTABLE:
						ii=1;
						break;
					
					default:
						ii=1;
						break;

				}

				if (!NumElems)
					break;
			}
	        pPartIndex[NumParts]= NumPoints;
            pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
	        for (i=0;i<nPoly;i++)
	        {   
	            long    numpoints, startpoint,ii; 
	                        
	            startpoint = *pPartIndex++;
	            numpoints = *pPartIndex-startpoint; 
	            *pNumPoints++ = numpoints; 
	            pPoints += numpoints;   
	            if (i)
	            	*pPoints++ = *pFirstPoint; 
	        } 
	        GlobalUnlock (hPoints); 
	        GlobalUnlock (hPolyPartLen);
	        GSSiGlobUlFree (&hPartIndex);  
			pPoints = (HPDPOINT)GlobalLock (hPoints); 
    		CurrentType = GF_AREA; 
			goto DoPoly;
		}
		case DGNT_ELLIPSE:              
		case DGNT_ARC:  
		{
			break;
    		CurrentType = GF_LINE; 
			int  size = sizeof(DGNElemArc);

			hPoints = GSSiGlobAlloc(1420, GMEM_MOVEABLE, sizeof(DPOINT)*(long)USHRT_MAX);
			//pPartIndex = (LPLONG)GlobalLock(hPartIndex);
			//*pPartIndex = 0;

			pPoints = pFirstPoint = (LPDPOINT)GlobalLock(hPoints);
			DGNElemArc FAR	*pRec = (DGNElemArc*)pElement;
			DGNPoint	*pvertices = (DGNPoint *)&pPoints[NumPoints];
			DGNPoint	TestPoints[5];
			int			MaxPoints = (MaxMemSize - size - 4) / sizeof (DGNPoint);
			double		dlen;

			{
				DGNStrokeArc(hDGN, (DGNElemArc *)pElement, 5, TestPoints);
				dlen = PolyLength(5, TestPoints);
				num_vertices = min(MaxPoints - 1, max(2, (int)(dlen / CurView->MetersPerPixel))) + 1;
				DGNStrokeArc(hDGN, (DGNElemArc *)pElement, num_vertices, pvertices);
			}

			//						pnum_vertices = (LPLONG)((LPSTR)pRec + sizeof (DGNElemArc));
			//						pvertices = (HPDGNPoint) (pnum_vertices+1);
			for (i = 0; i<num_vertices; i++)
			{
				//							pPoints[NumPoints].x = pvertices[i].x;
				//							pPoints[NumPoints].y = pvertices[i].y;
				ConvertCoord((LPDPOINT)&pvertices[i], 0, 1);
				//if (!NumPoints || !SameDPoint (&pPoints[NumPoints-1],&pPoints[NumPoints]))
				//	NumPoints++;
			}


			goto ProcessMultipoint;
		}
		case DGNT_CURVE:
		{
			DGNElemMultiPoint FAR	*pRec=(DGNElemMultiPoint*)pElement;   
    		CurrentType = GF_AREA;   
    		num_vertices = pRec->num_vertices;
    		pvertices = pRec->vertices; 
    		CurrentType = GF_LINE; 
		}
    		goto ProcessMultipoint;
		case DGNT_SHAPE:
		{
			DGNElemMultiPoint FAR	*pRec=(DGNElemMultiPoint*)pElement;

    		CurrentType = GF_AREA;   
    		num_vertices = pRec->num_vertices;
    		pvertices = pRec->vertices; 
    		if (DGNFillColor < 0)
    			DGNFillColor = DGNColor;
    	}
    		goto ProcessMultipoint;
/*		case DGNT_BSPLINE: 
		{
			DGNElemMultiPoint FAR	*pRec=(DGNElemMultiPoint*)pElement;  
			HANDLE	hOldPnts = GSSiGlobAlloc (0,GMEM_MOVEABLE,pRec->num_vertices*sizeof(DPOINT));
			HANDLE	hNewPnts = GSSiGlobAlloc (0,GMEM_MOVEABLE,(long)(UINT_MAX/4)*sizeof(DPOINT)); 
			HPDPOINT	OldPnts=(HPDPOINT)GlobalLock (hOldPnts);
			HPDPOINT	NewPnts=(HPDPOINT)GlobalLock (hNewPnts);
			
    		CurrentType = GF_LINE; 
    		for (i=0;i<pRec->num_vertices;i++)
    		{
    			OldPnts[i].x = pRec->vertices[i].x; 
    			OldPnts[i].y = pRec->vertices[i].y;
    		} 
    		pnum_vertices = (LPLONG)&pRec->vertices[pRec->num_vertices];
    		pvertices = (HPDGNPoint)(pnum_vertices + 1); 
			*pnum_vertices = 0;
			SplinePointsD (1,pRec->num_vertices,OldPnts,pnum_vertices,NewPnts,CurView->BaseUnitsPerPixel*2,0,0,UINT_MAX/4); 
    		for (i=0;i<*pnum_vertices;i++)
    		{
    			pvertices[i].x = NewPnts[i].x; 
    			pvertices[i].y = NewPnts[i].y;
    		} 
			GSSiGlobUlFree (&hNewPnts);
			GSSiGlobUlFree (&hOldPnts);
		}
    		goto ProcessMultipoint;  */
		case DGNT_LINE:                  
		case DGNT_LINE_STRING:  
		case DGNT_BSPLINE: 
		{
			DGNElemMultiPoint FAR	*pRec=(DGNElemMultiPoint*)pElement;  
			
    		CurrentType = GF_LINE;  
    		num_vertices = pRec->num_vertices;
    		pvertices = pRec->vertices; 
    	}
ProcessMultipoint:
		{	
			if (CurrentType != GF_AREA)
			{
				if (CurView->PassID == 2 || AreasOnlyThisFile)
					goto RtnFalse;
				if (!GetTypeVisibility(TYPE_LINECURVE))
					goto RtnFalse; 
			}
		    if (CurrentDesc < 3201)
				CurView->CurVisType[CurrentDesc]=2; 
	        if (*pnum_vertices > USHRT_MAX)
	        	goto RtnFalse;
	        if (!*pnum_vertices)
	        	goto RtnFalse;
	        nPoly = 1;  
	        NumPoints = *pnum_vertices; 
	        if (pElement->type == DGNT_COMPLEX_SHAPE_HEADER)
	        	NumPoints = NumPoints+nPoly-1;
	        hPartIndex = GSSiGlobAlloc (1418,GMEM_MOVEABLE,sizeof(long)*(nPoly+1));
	        hPolyPartLen = GSSiGlobAlloc (1419,GMEM_MOVEABLE,sizeof(USHORT)*(nPoly+1));
			hPoints = GSSiGlobAlloc (1420,GMEM_MOVEABLE,sizeof(DPOINT)*NumPoints); 
	        pPartIndex = (LPLONG)GlobalLock (hPartIndex);
	        *pPartIndex = 0; 
	        pPartIndex[NumParts]= NumPoints;
	                    
            pPoints = pFirstPoint = (LPDPOINT)GlobalLock (hPoints); 
            pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
	        for (i=0;i<nPoly;i++)
	        {   
	            long    numpoints, startpoint,ii; 
	                        
	            startpoint = *pPartIndex++;
	            numpoints = *pPartIndex-startpoint; 
	            *pNumPoints++ = numpoints; 
	            pPoints += numpoints;   
	            if (i && (pElement->type == DGNT_COMPLEX_SHAPE_HEADER))
	            	*pPoints++ = *pFirstPoint; 
	        } 
	        GlobalUnlock (hPoints); 
	        GlobalUnlock (hPolyPartLen);
	        GSSiGlobUlFree (&hPartIndex);  
			pPoints = (HPDPOINT)GlobalLock (hPoints);
			GSSiGlobFree (&hElevBuffer);
		    hElevBuffer = GSSiGlobAlloc ( 312,GMEM_MOVEABLE,NumPoints*sizeof(float));
		    pElev = (HPFLOAT)GlobalLock (hElevBuffer);
			for (i=0;i<NumPoints;i++)  
			{   
				pPoints[i].x = pvertices[i].x;
				pPoints[i].y = pvertices[i].y; 
				pElev[i] = pvertices[i].z;
				ConvertCoord(&pPoints[i],0,1);
			}
		    GlobalUnlock (hElevBuffer);
DoPoly:
			if (CurrentType != GF_AREA)
			{
				if (Pick)
				{   
					PickPolylineD (pPoints,NumPoints,0,2,0,0,0);
				}
				else
				{   
					short	lStyle=PS_SOLID;
					
					lpDCurPoints = pPoints;
					nPnts = nCurPoints = NumPoints;
                    if (!NumDGNSymbol)
                    	switch (DGNStyle)
	                    {
	                    	default:
	                    		lStyle = PS_SOLID;
	                    		break;
							case DGNS_DOTTED:
								lStyle = PS_DOT;
								break;
							case DGNS_MEDIUM_DASH:
							case DGNS_LONG_DASH:
							case DGNS_SHORT_DASH:
								lStyle = PS_DASH;
								break;
							case DGNS_DOT_DASH:
								lStyle = PS_DASHDOT;
								break;
							case DGNS_LONG_DASH_SHORT_DASH:
							case DGNS_DASH_DOUBLE_DOT:
								lStyle = PS_DASHDOTDOT;
								break;
	                    }
					if (PolyInMaskAreaFileCoord (GF_LINE,&NumPoints,0,0,&pPoints,TRUE))
					{   
						HPEN hNewPen=0;
						
						hOldPen  = SelectObject(hDC,GetStockObject(BLACK_PEN));
						TempLineColor = DGNColor; 
						TempLineWidth = DGNWidth; 
						if (TempLineColor >= 0 || TempLineWidth != 0)
						{
							hNewPen = CreatePen(lStyle, (short)IDNINT(AdjustWidth(TempLineWidth)), TempLineColor);
							SelectObject(hDC,hNewPen);
						}
						if (SetDisplayChar (hDC,GF_LINE,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI) > 0)
						{   
							pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
			        		for (i=0;i<nPoly;i++)
			        		{   
								if (*pNumPoints > 300)
									ii=1;
								GWPolylineD (hDC,lpDCurPoints,*pNumPoints,CurrentDesc); 
								lpDCurPoints+=*pNumPoints++;
			        		}   
			        		GlobalUnlock (hPolyPartLen); 
						} 
						if (hNewPen)
						{
							SelectObject(hDC,GetStockObject(BLACK_PEN)); 
							GSSiDeleteObject (&hNewPen);
						}
					}
				}
			}
			else
			{   
				BOOL	ShowBorder = GetBit (5,(LPSTR)&CurVis->WantType[7]); 
				HPEN	hBorderPen=0;
				
				if (Pick)
				{   
					PickPolygonD (pPoints,NumPoints,nPoly,hPolyPartLen,9999999,0);
				}
				else if (CopyRec)
				{
					pNumPoints = (LPINT)GlobalLock(hPolyPartLen);//pNumPoints[1]
					if (nPoly > 1)
					{
						HANDLE	hhPoly = GSSiGlobAlloc(418, GMEM_MOVEABLE, sizeof(HANDLE)*nPoly);
						LPHANDLE phPoly = (LPHANDLE)GlobalLock(hhPoly);
						HPDPOINT	pPoints1 = (HPDPOINT)GlobalLock(hPoints), pPoints2;

						for (i = 0; i<nPoly; i++)
						{
							phPoly[i] = GSSiGlobAlloc(420, GMEM_MOVEABLE, sizeof(DPOINT)*(long)pNumPoints[i]);
							pPoints2 = (HPDPOINT)GlobalLock(phPoly[i]);
							hmemmove((HPSTR)pPoints2, (HPSTR)pPoints1, sizeof(DPOINT)*(long)pNumPoints[i]);
							GlobalUnlock(phPoly[i]);
							pPoints1 += pNumPoints[i];
							if (i)
								pPoints1++;
						}
						GlobalUnlock(hPoints);
						AddPolyToBuffer(nPoly, pNumPoints, phPoly, TYPE_AREA, CurrentRefno, 0, -1, CurrentDesc, 0, CurrentPrefix, CurrentUDI,
							-1, -1, 0, 0, 0, 0, 0, TRUE, &hUpdateBuf, &lUpdateBuf);
						for (i = 0; i<nPoly; i++)
							GSSiGlobFree(&phPoly[i]);
						GSSiGlobUlFree(&hhPoly);
					}
					else
						AddPolyToBuffer(nPoly, pNumPoints, &hPoints, TYPE_AREA, CurrentRefno, 0, -1, CurrentDesc, 0, CurrentPrefix, CurrentUDI,
						-1, -1, 0, 0, 0, 0, 0, TRUE, &hUpdateBuf, &lUpdateBuf);
					GlobalUnlock(hPolyPartLen);
				}
				else
				{    
					if (Display)
					{
						hOldPen  = SelectObject(hDC,h0Pen);
						if (ShowBorder)
		    				hBorderPen = hAreaBorderPen[TRUE];
			        	//if (GetInVisibility(CurrentDesc))
			        	if (GetBit (7,(LPSTR)&CurVis->WantType[7]) && DGNFillColor < 0)
			        	{
			        		if (CurView->HaveLayerColor[FileNum])
			        		{
			        			hTempBrush = CreateSolidBrush(CurView->LayerColor[FileNum]);
								hOldBrush = SelectObject (hDC,hTempBrush);
								hTempPen = CreatePen (PS_SOLID,0,CurView->LayerColor[FileNum]);
                                SelectObject (hDC,hTempPen);
                            }
			        		else
								hOldBrush = SelectRandomBrush (hDC,CurrentRefno,&hBorderPen,CurrentDesc); 
						}  
						if (hBorderPen)
							SelectObject(hDC,hBorderPen); 
					}
					lpDCurPoints = pPoints;
					nPnts = NumPoints;

					if (PolyInMaskAreaFileCoord(GF_AREA, &NumPoints, 0, 0, &pPoints, TRUE))
					{
						int	SDCrtn = SetDisplayChar(hDC, GF_AREA, CurrentRefno, CurrentDesc, CurrentPrefix, CurrentUDI);

						if (SDCrtn > 0)
							ProcessPolygon(hDC, ShowBorder, PltType, hBorderPen, hTempPen, 0);
				/*		else if (SDCrtn < 0)
							HaveTXLoc = TRUE;
						if (PolyInMaskAreaFileCoord(GF_AREA, &NumPoints, 0, 0, &pPoints, TRUE))
					{
						if (SetDisplayChar (hDC,GF_AREA,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI) > 0)
						{   
							BOOL DoBorder = FALSE, DoFill;
							
							if (Display)
							{
								if (nPoly > 1 && !ShowLinkLines)
									SelectObject(hDC,h0Pen); 
								DoFill = (FillAreas && (GetBit (7,(LPSTR)&CurVis->WantType[7]) ||
														GetBit (6,(LPSTR)&CurVis->WantType[7])));
								hDeletePen = GWPolygonD (hDC,pPoints, NumPoints, nPoly, hPolyPartLen,CurrentDesc,ShowBorder,DoFill,0);
					        }
						}*/
					}
				}
			}
			GSSiGlobFree (&hPolyPartLen);
			GSSiGlobUlFree (&hPoints);
		}
			break;                 
		case DGNT_TEXT_NODE:             
			break;                 
		case DGNT_COMPLEX_CHAIN_HEADER:
			ii=1;
			break;               
		case DGNT_TEXT:                 
		{
			DGNElemText FAR	*pRec=(DGNElemText*)pElement; 
			short	nchar = _fstrlen (pRec->string);   
			GRTEXTHEADER	GRTextHeader;  
						
			if (CurView->PassID == 2 || AreasOnlyThisFile)
				goto RtnFalse; 
			if (!CurVis->WantType[2])
				goto RtnFalse;
		    if (CurrentDesc < 3201)
				CurView->CurVisType[CurrentDesc]=5; 
			CurrentType = GF_POINT;	
			CurPointLocD.x = pRec->origin.x; 
			CurPointLocD.y = pRec->origin.y; 
			HaveTXLoc = TRUE;
			ConvertCoord(&CurPointLocD,0,1);
    		if (HaveNewPoint)
    			CurPointLocD = NewPointD;   
    		LastElementBeginPoint = CurPointLocD;   
    		if (PickingByRefno)
				PickList[0].BeginPoint = PickList[0].EndPoint = CurPointLocD;
    			
    		PTRot = pRec->rotation * RADDEG;  
    		_fmemset (&GRTextHeader,0,sizeof(GRTextHeader));
    		GRTextHeader.lText = nchar;
			GRTextHeader.FontNum = 8;  
 
/*			GRTextHeader.vJust=1;      //0=above,1=baseline,2=center,3=below
			GRTextHeader.hJust=1;      //0=left,1=center,2=right
			switch (pRec->justification)
			{
				case DGNJ_LEFT_TOP:
					GRTextHeader.hJust=0;
					GRTextHeader.vJust=3;
					break;
				case DGNJ_LEFT_CENTER:
					GRTextHeader.hJust=0;
					GRTextHeader.vJust=2;
					break;
				case DGNJ_LEFT_BOTTOM:
					GRTextHeader.hJust=0;
					GRTextHeader.vJust=1;
					break;
				case DGNJ_LEFTMARGIN_TOP:    // text node header only 
				case DGNJ_LEFTMARGIN_CENTER: // text node header only 
				case DGNJ_LEFTMARGIN_BOTTOM: // text node header only 
				case DGNJ_CENTER_TOP:
					GRTextHeader.vJust=3;
					break;
				case DGNJ_CENTER_CENTER:
					GRTextHeader.vJust=2;
					break;
				case DGNJ_CENTER_BOTTOM:
					GRTextHeader.vJust=1;
					break;
				case DGNJ_RIGHTMARGIN_TOP:   // text node header only 
				case DGNJ_RIGHTMARGIN_CENTER:// text node header only 
				case DGNJ_RIGHTMARGIN_BOTTOM:// text node header only 
				case DGNJ_RIGHT_TOP:
					GRTextHeader.hJust=2;
					GRTextHeader.vJust=3;
					break;
				case DGNJ_RIGHT_CENTER:
					GRTextHeader.hJust=2;
					GRTextHeader.vJust=2;
					break;
				case DGNJ_RIGHT_BOTTOM: 
					GRTextHeader.hJust=2;
					GRTextHeader.vJust=1;
					break;
				break;
			}*/ 
			{
				static short	vjus=0;
				GRTextHeader.vJust=vjus;//0=above,1=baseline,2=center,3=below
			}
			GRTextHeader.hJust=0;      //0=left,1=center,2=right
			SetTextHeadSize (&GRTextHeader,pRec->height_mult*1.8*NonPltFileDistToBaseDist);
		    SetTextLocVars (&TLSet); 
		    if (CopyRec)
		    {   
		    	HANDLE hGRText=0;
                
				if (UpdateItem == 20)
        			CurPointLocD = NewPointD;
				if (UpdateItem == 201)
        			AdjustPointRotation (&PTRot);
				if (UpdateItem == 19)
				{   
					LPGRTEXTHEADER pGRTextHeader = (LPGRTEXTHEADER)GlobalLock (hPickedTextHeader);	 
					LPSTR	pText = (LPSTR) (pGRTextHeader+1);
					
		    		nchar = pGRTextHeader->lText;  
            		hGRText = GRTextFromTextHeader (pGRTextHeader,pText);
					GlobalUnlock (hPickedTextHeader);	
            	}
            	else
            		hGRText = GRTextFromTextHeader (&GRTextHeader,pRec->string);
		    	AddPointToBuffer (CurPointLocD,CurrentRefno,0,CurrentDesc,10, PTRot,0,hGRText,0,
								  CurrentPrefix,CurrentUDI,0,0,-1,TRUE,&hUpdateBuf,&lUpdateBuf,0);
				GSSiGlobFree (&hGRText);
			}
        	else if (CurVis->WantType[2] && HaveTXLoc && (CurView->PassID || Pick)) 
        	{   
        		COLORREF	OldColor = -1;
        		
        		if (hPickedTextHeader)
			    {
			    	LPGRTEXTHEADER pPickedTextHeader = (LPGRTEXTHEADER)GlobalLock (hPickedTextHeader);
			    	_fmemmove (pPickedTextHeader,&GRTextHeader,sizeof(GRTEXTHEADER)); 
			    	pPickedTextHeader++;
			    	_fstrcpy ((LPSTR)pPickedTextHeader,pRec->string);
			    	GlobalUnlock (hPickedTextHeader);
			    }

				ProcessTextObject (hDC,&GRTextHeader,pRec->string,nchar,0,0,0,0,0);
				if (OldColor >= 0)
					SetTextColor (hDC,OldColor); 
				if (CurVis->WantType[8])
					DisplayPointItem (hDC,BasePtToWinPt(&CurPointLocD),10,PTRot,InvisiblePointSymbol,0);
			}
		} 
			break; 
	}
	if (hOldBrush)
		SelectObject (hDC,hOldBrush);
	if (hOldPen)
		SelectObject (hDC,hOldPen); 
   	GSSiDeleteObject (&hDeletePen);
   	GSSiDeleteObject (&hTempBrush);
   	GSSiDeleteObject (&hTempPen);
	ShowValue (hDC,FALSE);  
	GlobalUnlock (hElement);
	ItemIsDeleted = FALSE;     
	InGraphicsProcessor = FALSE;
	return TRUE;  
RtnFalse:
	ItemIsDeleted = FALSE;     
	GlobalUnlock (hElement);
	InGraphicsProcessor = FALSE;
	return FALSE;
}



/*	HANDLE	hMem;
	DWORD	st,
	LPDGNElementCore pElement;       
    st = DGN7GetExtents (hDGN,Extents);
    hMem = GSSiGlobAlloc (0,GMEM_MOVEABLE,2000000L);
    pElement = GlobalLock (hMem);
    while (DGN7ReadElement (hDGN,pElement))
    {
    	ii = pElement->type;  
    }
    DGN7Close (hDGN);  
    GSSiGlobUlFree (&hMem);
    }
*/   

BOOL OpenDGNCellLibrary (LPSTR InName)
{
	LPDGNElementCore pElement; 
	long	NumAttributes;
	long	Attributes[30];   
	char	Name[256];  
	long	recno=0;
	
	return FALSE;
	if (hDGNCell)
		return TRUE;
	
	NumCells = 0;
	_fstrcpy (Name,InName);
	ExpandText (Name);
	if (!*Name)
		return FALSE;	
	hDGNCell = DGN7Open (Name,0,0);  
	if (!hDGNCell)
		return FALSE;
	hElement = GSSiGlobAlloc (0,GMEM_MOVEABLE,MaxDGNElementSize);
	pElement = (LPDGNElementCore)GlobalLock (hElement);
	while (DGNLibReadElement (hDGNCell,pElement,MaxDGNElementSize,0,&FillColor,&NumAttributes,Attributes))
	{
		if (pElement->type == DGNT_CELL_LIBRARY)    
		{
			DGNElemCellLibrary FAR	*pRec=(DGNElemCellLibrary*)pElement; 
			
			_fstrcpy (CellLib[NumCells].Name,pRec->name);
			CellLib[NumCells++].record = recno;
		} 
		recno++;
	}
    GSSiGlobUlFree (&hElement);
	return TRUE;
} 

void CloseDGNCellLibrary (void)
{
	if (hDGNCell)
		DGN7Close (hDGNCell); 
	hDGNCell = 0; 
	return;
} 

/*HANDLE GetCellLibSymbol (LPSTR Name)
{
	USHORT	i;
	
	for (i=0;i<NumCells;i++)
		if (!_fstricmp (Name,CellLib[i].Name))
			goto FoundSym;
	return 0;
FoundSym:
	return 1;
}*/

short	DGNFontToGMFont (short DGNFont)
{    
  	LPSTR	pDTGFont;
  	LPSTR	pSpace, pEnd;
  	short	font, gmfont=0;
  	
  	if (!hDGNtoGMFont)
  		return 0;
  	pDTGFont  = GlobalLock (hDGNtoGMFont);
  	while (*pDTGFont) 
  	{
  		font = atoi (pDTGFont);
  		if (font == DGNFont)
  		{
  			if ((pSpace = _fstrrchr (pDTGFont,' ')))
	  			gmfont = atoi (pSpace);
	  		else
	  			gmfont = 0;
  			break;
  		}
  		pDTGFont = _fstrchr (pDTGFont,0);
  		pDTGFont++;
  	}
  	GlobalUnlock (hDGNtoGMFont); 
  	return gmfont;
}

 

			

