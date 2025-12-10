#include "graphint.h"    
#include "dibapi.h"  

#define IDNINTPOS(a)	((long)(a+0.5))

#include "gmextern.h"

static	char	LastSymName[34]=""; 

static BOOL		fromCloseDict = FALSE;
static HANDLE   cachedSymbolHandle[3200] = { 0 };
static HANDLE	hSymIndex=0;
static short	NumElementPoints=0;
static long	SymbolFileEnd;
static short	LastSymNum=0;
static HANDLE	hChildList=0;
static short	lChildList;
static double	DefaultSymbolFactor=150;
static short	ShowInvisibleLinesSymbol=0;
static  short    SymDictOpenMode=0;   
static	HFILE	FidSD=-1;
static	double	SymbolScales[7]={40,50,100,200,500,1000,0};
static	struct	{long npnts; short width, desc, OneWay, order; COLORREF color;} HollowLineHeader;          
static	short	SymDictVersion;
static	int		nElemPoints[MAX_SYMBOL_ELEMENTS];
static	HANDLE	hElemPoints[MAX_SYMBOL_ELEMENTS];
static	COLORREF	ElemLineColor[MAX_SYMBOL_ELEMENTS];
static	int		ElemLineWidth[MAX_SYMBOL_ELEMENTS];
static	DWORD	RastOpts[15]={SRCCOPY,SRCAND,SRCPAINT,SRCINVERT,SRCERASE,NOTSRCCOPY,NOTSRCERASE,MERGECOPY,
	           			  MERGEPAINT,PATCOPY,PATPAINT,PATINVERT,DSTINVERT,BLACKNESS,WHITENESS};



extern	int	ShowHollowStreet;

long    IDNINT (double X)
{
    if (X < 0)
        return ((long) fmax ((double)INT_MIN,(X - 0.5)));
    return ((long) fmin ((double)INT_MAX,(X + 0.5)));
}

void GetSymDictChildren (int Parent,LPSHORT pnChildren,LPHANDLE phChildren,int SymType,BOOL SearchSubPar)
{   
	LPCHILDLIST pChildList;
	short	n; 
	LPSHORT	pChildren;
	
	if (!OpenSymDict (OF_READ))
		return;   
	if (*IconDict)
	{   
		HANDLE	hMem=GSSiGlobAlloc ( 447,GMEM_MOVEABLE,256);
		LPSTR	str=GlobalLock (hMem); 
		long	CurLoc;
		
		*phChildren = GSSiGlobAlloc ( 448,GHND,USHRT_MAX); 
		pChildren = (LPSHORT)GlobalLock (*phChildren);
		*pnChildren = 0;
		CurLoc = GSSillseek (FidSD,0,0);
		while (fgetstring (str,128,FidSD))
		{
			*pChildren = (short)CurLoc+1;
			pChildren++; 
			(*pnChildren)++;
			CurLoc = GSSillseek (FidSD,0,1);
		}
		GSSiGlobUlFree (&hMem); 
		GlobalUnlock (*phChildren);
		return;
	}
	BuildChildList ();
	pChildList = (LPCHILDLIST)GlobalLock (hChildList);
	if (!*phChildren)
	{
		*pnChildren = 0; 
		if (lChildList)
			*phChildren = GSSiGlobAlloc ( 449,GMEM_MOVEABLE,(long)lChildList * sizeof(CHILDLIST));
		else
		{
			*phChildren = 0;
			GlobalUnlock (hChildList);
			return;         
		}
	}
	n = lChildList;
	while (n--) 
	{
		if (pChildList->SymNum == 29)
			ii = 1;
		if (pChildList->Parent == Parent)
		{
			if (!pChildList->Type && SearchSubPar)
				GetSymDictChildren (pChildList->SymNum,pnChildren,phChildren,SymType,SearchSubPar);
			if (SymType < 0 || pChildList->Type == SymType)
			{
				pChildren = (LPSHORT)GlobalLock (*phChildren);
				pChildren += *pnChildren;
				(*pnChildren)++;
				*pChildren++ = pChildList->SymNum; 
				GlobalUnlock (*phChildren);
			}
		}
		pChildList++;
	} 
	GlobalUnlock (hChildList);
	return; 
} 

void BuildChildList (void) 
{   
    LPSYMBOL    pSymbol; 
	HANDLE	hSymbol;  
	LPCHILDLIST pChildList;   
	short	i;

	if (hChildList)
		return;
	hChildList = GSSiGlobAlloc ( 450,GHND,USHRT_MAX);
	pChildList = (LPCHILDLIST)GlobalLock (hChildList);
	lChildList = 0;
	for (i=1;i<=NumSymbols;i++)
	{ 
		hSymbol = GetDictSymDesc (i,1);
		if (hSymbol)
		{ 
			pSymbol = (LPSYMBOL)GlobalLock (hSymbol);  
			if (pSymbol->Number == 29)
				ii = 1;
			pChildList->Type = pSymbol->Type;
			pChildList->Parent = pSymbol->Parent;
			pChildList++->SymNum = i;
			lChildList++;
         	GlobalUnlock (hSymbol); 
			DestroySymbol (hSymbol); 
		}
	} 
	GlobalUnlock (hChildList);
	
	return;
}

short SelectSymbol (HWND hWnd,short Type,LPSTR StartSym,short DialogOpt)
{   
	short	rtn=0;
	
	switch (Type)
	{   
		case 1:
		{   
			char	SymName[34]="[%NEW_POINT_SYM]", SymSize[64]="[%NEW_POINT_SIZE]", Rot[64]="[%NEW_POINT_ROT]", SymColor[64]="[%NEW_POINT_COLOR]";   
			float	size,rot;
			
			ExpandText (SymName);
			ExpandText (SymSize);
			ExpandText (SymColor);
			ExpandText (Rot);
			if ((rtn = SelectPointSymbol (hWnd,DialogOpt,SymName,StartSym,SymSize,Rot,SymColor,FALSE)))
			{   
				SetGlobalValue("%NEW_POINT_SYM",SymName);
				SetGlobalValue("%NEW_POINT_SIZE",SymSize);
				SetGlobalValue("%NEW_POINT_ROT",Rot);
				SetGlobalValue("%NEW_POINT_COLOR",SymColor);
			}
			break;
		}
		case 2:
		{
			char	SymName[34]="[%NEW_LINE_SYM]", SymSize[64]="[%NEW_LINE_WIDTH]", SymColor[64]="[%NEW_LINE_COLOR]";   
			
			ExpandText (SymName);
			ExpandText (SymSize);
			ExpandText (SymColor);
			if ((rtn=SelectLineSymbol (hWnd,DialogOpt,SymName,SymSize,SymColor,FALSE)))
			{   
				SetGlobalValue("%NEW_LINE_SYM",SymName);
				SetGlobalValue("%NEW_LINE_WIDTH",SymSize);
				SetGlobalValue("%NEW_LINE_COLOR",SymColor);
			} 
			break;
		}
		case 3:
		{
			char	SymName[34]="[%NEW_AREA_SYM]", SymColor[64]="[%NEW_AREA_COLOR]";   
			
			ExpandText (SymName);
			ExpandText (SymColor);
			if ((rtn=SelectAreaSymbol (hWnd,DialogOpt,SymName,SymColor,FALSE)))
			{   
				SetGlobalValue("%NEW_AREA_SYM",SymName);
				SetGlobalValue("%NEW_AREA_COLOR",SymColor);
			}
			break;
		}
	} 
	return rtn;
}  

short SelectFontSymbol (HWND hWnd,LPSTR FontSizeC, COLORREF *Color)
{
	DLGPROC lpfnGET_FONTSYMMsgProc;
	int	nRc;
	
	lpfnGET_FONTSYMMsgProc = MakeProcInstance((DLGPROC)GET_FONTSYMMsgProc, hInst);
	nRc = DialogBox(hInst, (LPSTR)"GET_FONTSYM", hWnd, lpfnGET_FONTSYMMsgProc);
	FreeProcInstance(lpfnGET_FONTSYMMsgProc); 
	return nRc;
} 

short FillParentList (HWND hWndDlg,UINT ControlID,short Type)
{
	char	str[128];
	char	Tab[4];
	short	nItem, nPar, nAll, SymLevel=0;	
	HANDLE	hLevel[32]; 
	long	i;
	short	Fun1,Fun2;
	
	nAll = GetDictSymbolNumber ("ALL"); // opens %SYM_DICT   
	if (!Type)
		_fstrcpy (Tab,"\t"); 
	else
		_fstrcpy (Tab,"  "); 
	nPar = AddToParentList (nAll,hWndDlg,Type,ControlID,Tab,str,&SymLevel);
	return nPar; 
}

short AddToParentList (int Parent,HWND hWndDlg,short ControlType,UINT ControlID,LPSTR Tab, LPSTR str,LPSHORT pSymLevel)
{   
	HANDLE	hChildren=0;
	short	nChildren; 
	short	nPar=0, nItem;
    LPSYMBOL    pSymbol; 
	HANDLE	hSymbol;  
	LPSHORT	Child; 
	short	i;   
	short	ii;
	
	if (!Parent)
		return 0;
	hSymbol = GetDictSymDesc (Parent,1);  
	if (!hSymbol)
		return 0;
	pSymbol = (LPSYMBOL)GlobalLock (hSymbol);  
	*str=0;
	for (i=0;i<*pSymLevel;i++)
		_fstrcat (str,Tab);
    _fstrcat (str,pSymbol->Name);
 	GlobalUnlock (hSymbol); 
	DestroySymbol (hSymbol); 
	if (!ControlType) 
	{
	   	nItem = SendDlgItemMessage (hWndDlg,ControlID,LB_ADDSTRING,(WPARAM)0,(LPARAM)str); 
	    SendDlgItemMessage (hWndDlg,ControlID,LB_SETITEMDATA,(WPARAM)nItem,(LPARAM)Parent); 
    }
    else
	{
	   	nItem = SendDlgItemMessage (hWndDlg,ControlID,CB_ADDSTRING,(WPARAM)0,(LPARAM)str); 
	    SendDlgItemMessage (hWndDlg,ControlID,CB_SETITEMDATA,(WPARAM)nItem,(LPARAM)Parent); 
    }
    (*pSymLevel)++;
	GetSymDictChildren (Parent,&nChildren,&hChildren,0,FALSE);
	if (hChildren)
	{  
		Child = (LPSHORT)GlobalLock (hChildren);
		while (nChildren--)
			nPar = AddToParentList (*Child++,hWndDlg,ControlType,ControlID,Tab,str,pSymLevel); 
		GSSiGlobUlFree (&hChildren);
	}
	else
		ii=1; 
	(*pSymLevel)--;
	return nPar;
} 

int CopySymbolParents (LPSTR SymName,LPSTR FromDict)
{
#define MAX_PARNAMES	8
	int parSymNum = 0;
	char	saveDict[MAX_PATH]; 
	BOOL	rtn = 0;
	int		n=0, ipar;
	char	parNames[MAX_PARNAMES][34];
	HANDLE	hSym[MAX_PARNAMES] = { 0 };
	int		nParNames=1;

    CloseSymDict(); 
    GetGlobalCVal ("[%SYM_DICT]",saveDict,0); 
    SetGlobalValue ("%SYM_DICT",FromDict);
	if (OpenSymDict (OF_READ))
	{
		n = GetDictSymbolNumber (SymName);
		ipar = GetDictSymParent (n);
		GetDictSymName (ipar,parNames[0]);
		hSym[0] = GetDictSymDesc (ipar,0);
		while (stricmp (parNames[nParNames-1],"ALL") && nParNames < MAX_PARNAMES)
		{
			ipar = GetDictSymParent (ipar);
			hSym[nParNames] = GetDictSymDesc (ipar,0);
			GetDictSymName (ipar,parNames[nParNames++]);
		}
	    SetGlobalValue ("%SYM_DICT",saveDict);
	    CloseSymDict(); 
		if (OpenSymDict (OF_READWRITE))
		{
			int	i=nParNames-1;
			int	lastPar=0;

			while (i>=0 )
			{
				ipar = GetDictSymbolNumber (parNames[i]);
				if (!ipar)
				{
					LPSYMBOL	pSym = (LPSYMBOL)GlobalLock (hSym[i]);

					pSym->Parent = lastPar; 
					GlobalUnlock (hSym[i]);
					ipar = SaveSymbol(hSym[i],n);   
				}
				lastPar = ipar;
				i--;
			}
			parSymNum = lastPar;
		}
		else
			parSymNum = -1;
	}
	for (n=0;n<nParNames;n++)
		DestroySymbol (hSym[n]);
	return parSymNum;
}

int CopySymbolFromDict (LPSTR FromDict,LPSTR SymName,LPSTR NewParent,LPSTR NewName,int FromSymNum,int ToSymNum)
{
	char	SaveDict[MAX_PATH]; 
	BOOL	rtn = 0;
	int		n=0;
	HANDLE	hSym=0; 
	BOOL	saveAllowCachedSymbols = allowCachedSymbols;


    CloseSymDict(); 
	allowCachedSymbols = FALSE;
    GetGlobalCVal ("[%SYM_DICT]",SaveDict,0); 
    SetGlobalValue ("%SYM_DICT",FromDict);
	if (OpenSymDict (OF_READ))
	{
		if (!FromSymNum)
			n = GetDictSymbolNumber (SymName);
		else
			n = FromSymNum;
		if (n) 
			hSym = GetDictSymDesc (n,0);
	    CloseSymDict(); 
	} 
    SetGlobalValue ("%SYM_DICT",SaveDict); 
    if (hSym)
    {
		if (OpenSymDict (OF_READWRITE))
		{
			short	parent;
			LPSYMBOL	pSym = (LPSYMBOL)GlobalLock (hSym);
			
			if (*NewParent)  
			{
				parent = GetDictSymbolNumber (NewParent);
				if (!parent)
					goto Exit;
				pSym->Parent = parent; 
			}
			if (*NewName)
				_fstrcpy (pSym->Name,NewName);
			GlobalUnlock (hSym);
			if (ToSymNum > 0)
				n = -ToSymNum;
			rtn = SaveSymbol(hSym,n);   
		}
Exit:
		DestroySymbol (hSym);
	    CloseSymDict(); 
	}
	allowCachedSymbols = saveAllowCachedSymbols;
	return rtn;
}

int CopyParentFromDict (LPSTR FromDict,LPSTR ParName,LPSTR NewParent,LPSTR NewParName)
{
	BOOL	rtn=0;
	char	SaveDict[MAX_PATH]; 
	int		i, ParentSym=0,NewParSymNum;
	short	nChildren=0;
	HANDLE	hSym=0, hChildren=0; 
	char	PName[66];
	
	if (!(NewParSymNum = CopySymbolFromDict (FromDict,ParName,NewParent,NewParName,0,0)))
		return 0;
	if (*NewParName)
		strcpy (PName,NewParName);
	else
		strcpy (PName,ParName);
    CloseSymDict(); 
    GetGlobalCVal ("[%SYM_DICT]",SaveDict,0); 
    SetGlobalValue ("%SYM_DICT",FromDict);
	if (OpenSymDict (OF_READ))
	{
		if ((ParentSym = GetDictSymbolNumber (ParName)))
			GetSymDictChildren (ParentSym,&nChildren,&hChildren,-1,FALSE);
	    CloseSymDict(); 
	} 
    SetGlobalValue ("%SYM_DICT",SaveDict);
	if (nChildren)
	{
		LPSHORT pChild = (LPSHORT)GlobalLock (hChildren);

		for (i=0;i<nChildren;i++)
			CopySymbolFromDict (FromDict,"",PName,"",*pChild++,0);
	}
	GSSiGlobUlFree (&hChildren);
	return rtn;
}

int GetSymbol (int SymNum,LPDPOINT pDPoint,HANDLE *hnPnts, HANDLE *hElements)
{   
	LPDPOINT	pDPoints; 
	LPHANDLE	phDPoints;
	LPSHORT		pnPnts; 
	int			NumElements=1; 
	long		offset=200;
    
    
	*hnPnts = GSSiGlobAlloc ( 451,GMEM_MOVEABLE,NumElements*sizeof(int));
	*hElements = GSSiGlobAlloc ( 452,GMEM_MOVEABLE,NumElements*sizeof(HANDLE));
	pnPnts = (LPSHORT)GlobalLock (*hnPnts);  
	*pnPnts = 4; 
	phDPoints = (LPHANDLE)GlobalLock (*hElements);
	*phDPoints = GSSiGlobAlloc ( 453,GMEM_MOVEABLE,*pnPnts*sizeof(DPOINT));
	pDPoints = (LPDPOINT)GlobalLock (*phDPoints);
	pDPoints->x = pDPoint->x - offset;
	pDPoints++->y = pDPoint->y + offset;
	pDPoints->x = pDPoint->x + offset;
	pDPoints++->y = pDPoint->y + offset;
	pDPoints->x = pDPoint->x + offset;
	pDPoints++->y = pDPoint->y - offset;
	pDPoints->x = pDPoint->x - offset;
	pDPoints->y = pDPoint->y - offset; 
	GlobalUnlock (*phDPoints);
	GlobalUnlock (*hElements);	
	GlobalUnlock (*hnPnts);
	return 1;
}


int GetDictSymbolNumber (LPSTR SymName)
{
	LPSTR pSymNames;
	LPSHORT	pSymDesc; 
	short	n=0;
	
	if (!*SymName)
		return 0;
	if (!_fstricmp (SymName,LastSymName))                   
		return LastSymNum; 
	*LastSymName = 0;
	if (FidSD == HFILE_ERROR)
	{
		if (!OpenSymDict (OF_READ))
			return 0; 
	}
	if (hSymNames)
	{ 
		pSymNames = GlobalLock (hSymNames); 
		while (*pSymNames)
		{
			if (!_fstricmp (SymName,pSymNames))
			{   
				short	l=_fstrlen (pSymNames);
				
				pSymNames += l+1;
				pSymDesc = (LPSHORT)pSymNames; 
				n = *pSymDesc;  
				LastSymNum = n;
				_fstrcpy (LastSymName,SymName);  
				break;
			} 
			pSymNames = _fstrchr (pSymNames,0);  
			pSymNames+=3;
		}  
		GlobalUnlock (hSymNames); 
	}
	else if (!n && hSymIndex)
	{                
		SYMBOL		Symbol; 
		LPSYMBOL	CurSymbol=&Symbol;
		LPLONG		pSymIndex; 
		short		i;
		
		pSymIndex = (LPLONG)GlobalLock (hSymIndex);
		for (i=0;i<NumSymbols;i++,pSymIndex++)
		{   
			if (*pSymIndex >=0)
			{
				GSSillseek (FidSD,*pSymIndex,0);
				BigRead (FidSD,(HPSTR)CurSymbol,sizeof(SYMBOL));   
				if (!_fstricmp(SymName,CurSymbol->Name))   
				{
					n = CurSymbol->Number;
					break;
				}
			}
		}
		GlobalUnlock (hSymIndex);
	}   
	return n;
}
 
BOOL GetDictSymName (int idesc,LPSTR Name)
{   
	HANDLE	hSymbol;
	LPSYMBOL pSymbol; 
	
	*Name = 0;
	hSymbol = GetDictSymDesc (idesc,1);
	if (hSymbol)
	{ 
		pSymbol = (LPSYMBOL)GlobalLock (hSymbol);  
		_fstrcpy (Name,pSymbol->Name);
     	GlobalUnlock (hSymbol); 
		DestroySymbol (hSymbol); 
		return TRUE;
	} 
	else
		return FALSE;
} 

BOOL SymbolIsVisible (int idesc) 
{
	LPSYMBOLATTRIBUTE	pSymAtt; 
	BOOL	rtn=TRUE;
	
	if (!OpenSymDict (OF_READ))
		return rtn;  
	if (hSymbolAttributes && idesc && idesc <= NumSymbols)
	{
		pSymAtt = (LPSYMBOLATTRIBUTE)GlobalLock (hSymbolAttributes);
		if (pSymAtt[idesc-1].InVisible)
			rtn=FALSE;
		GlobalUnlock (hSymbolAttributes);
	}
	return rtn;
}

BOOL SymbolIsSolidLine (int idesc) 
{
	LPSYMBOLATTRIBUTE	pSymAtt; 
	BOOL	rtn=FALSE;
	
	if (!OpenSymDict (OF_READ))
		return rtn;  
	if (hSymbolAttributes && idesc && idesc <= NumSymbols)
	{
		pSymAtt = (LPSYMBOLATTRIBUTE)GlobalLock (hSymbolAttributes);
		if (pSymAtt[idesc-1].SolidLine)
			rtn=TRUE;
		GlobalUnlock (hSymbolAttributes);
	}
	return rtn;
}  

BOOL SymbolIsReversed (int idesc) 
{
	LPSYMBOLATTRIBUTE	pSymAtt; 
	BOOL	rtn=FALSE;
	
	if (!OpenSymDict (OF_READ))
		return rtn;  
	if (hSymbolAttributes && idesc && idesc <= NumSymbols)
	{
		pSymAtt = (LPSYMBOLATTRIBUTE)GlobalLock (hSymbolAttributes);
		if (pSymAtt[idesc-1].Reversed)
			rtn=TRUE;
		GlobalUnlock (hSymbolAttributes);
	}
	return rtn;
}  

BOOL SymbolIsLayered (int idesc)
{
	LPSYMBOLATTRIBUTE	pSymAtt; 
	BOOL	rtn=FALSE;
	
	if (!OpenSymDict (OF_READ))
		return rtn;  
	if (hSymbolAttributes && idesc && idesc <= NumSymbols)
	{
		pSymAtt = (LPSYMBOLATTRIBUTE)GlobalLock (hSymbolAttributes);
		if (pSymAtt[idesc-1].Layered)
			rtn=TRUE;
		GlobalUnlock (hSymbolAttributes);
	}
	return rtn;
}  

int SymbolDisplayPos (int idesc)
{
	LPSYMBOLATTRIBUTE	pSymAtt; 
	int	rtn=0;
	
	if (!OpenSymDict (OF_READ))
		return 0;  
	if (hSymbolAttributes && idesc && idesc <= NumSymbols)
	{
		pSymAtt = (LPSYMBOLATTRIBUTE)GlobalLock (hSymbolAttributes);
		rtn = pSymAtt[idesc-1].DisplayPos;
		GlobalUnlock (hSymbolAttributes);
	}
	return rtn;
}

COLORREF GetSymbolColor (int idesc) 
{
	LPSYMBOLATTRIBUTE	pSymAtt; 
	COLORREF	rtn=0;
	
	if (!OpenSymDict (OF_READ))
		return 0;  
	if (hSymbolAttributes && idesc && idesc <= NumSymbols)
	{
		pSymAtt = (LPSYMBOLATTRIBUTE)GlobalLock (hSymbolAttributes);
		rtn = pSymAtt[idesc-1].Color;
		GlobalUnlock (hSymbolAttributes);
	}
	return rtn;
}

double GetSymbolWidth (int idesc) 
{
	LPSYMBOLATTRIBUTE	pSymAtt; 
	double	rtn=0;   
	double 	f;
	
	if (!OpenSymDict (OF_READ))
		return 0; 
//	f = GetGlobalDVal2 ("[%STREETWIDTHFACTOR]",1);
	f = PenWidthFactor; 
	if (hSymbolAttributes && idesc && idesc <= NumSymbols)
	{
		pSymAtt = (LPSYMBOLATTRIBUTE)GlobalLock (hSymbolAttributes);    
		if (pSymAtt[idesc-1].WidthIsMeters)
			rtn = f * AdjustWidth(-(double)pSymAtt[idesc-1].Width);  
		else
			rtn = f *  AdjustWidth((double)pSymAtt[idesc - 1].Width);
		if (pSymAtt[idesc-1].Type == 1)
			rtn /= 200;
		GlobalUnlock (hSymbolAttributes);
	}
	return rtn;
}

short GetDictSymbolType (int idesc) 
{
	LPSYMBOLATTRIBUTE	pSymAtt; 
	short	rtn=-1;
	
	if (!OpenSymDict (OF_READ))
		return rtn;  
	if (hSymbolAttributes && idesc && idesc <= NumSymbols)
	{
		pSymAtt = (LPSYMBOLATTRIBUTE)GlobalLock (hSymbolAttributes);
		rtn = pSymAtt[idesc-1].Type;
		GlobalUnlock (hSymbolAttributes);
	}
	return rtn;
}


BOOL PointIsVisible (int idesc)
{   
	HANDLE	hSymbol;
	LPSYMBOL pSymbol; 
	BOOL	rtn=FALSE;
	
	hSymbol = GetDictSymDesc (idesc,0);
	if (hSymbol)
	{ 
		pSymbol = (LPSYMBOL)GlobalLock (hSymbol); 
		if (pSymbol->NumElements)
			rtn = TRUE; 
     	GlobalUnlock (hSymbol); 
		DestroySymbol (hSymbol); 
	} 
	return rtn;
} 

short GetDictSymParent (int idesc)
{   
	HANDLE	hSymbol;
	LPSYMBOL pSymbol;
	short	parent=0; 
	
	hSymbol = GetDictSymDesc (idesc,1);
	if (hSymbol)
	{ 
		pSymbol = (LPSYMBOL)GlobalLock (hSymbol);  
		parent = pSymbol->Parent;
     	GlobalUnlock (hSymbol); 
		DestroySymbol (hSymbol); 
	} 
	return parent;
} 

BOOL GetDictSymDescription (int idesc,LPSTR Description)
{   
	HANDLE	hSymbol;
	LPSYMBOL pSymbol; 
	
	*Description = 0;  
	hSymbol = GetDictSymDesc (idesc,1);
	if (hSymbol)
	{ 
		pSymbol = (LPSYMBOL)GlobalLock (hSymbol);  
		_fstrcpy (Description,pSymbol->Desc);
     	GlobalUnlock (hSymbol); 
		DestroySymbol (hSymbol); 
		return TRUE;
	} 
	else
		return FALSE;
} 

/*void SetInvisFromDict(void)
{ 
	if (!OpenSymDict (OF_READ))
		return FALSE;
	if (!GetInVisibility(idesc)) ToggleInVisibility(idesc);
}*/

HANDLE GetDictSymDesc (int idesc,short opt)
{   
//opt = 0 gets all, opt=1 gets just header
	long	ii;  
	int		i;
	HANDLE	*phElement, hElement;
	LPSYMBOL pSymDesc; 
	HANDLE	handle;
	LPLONG	pSymIndex; 
	
	if (idesc <= 0)
		return 0;
	if (!OpenSymDict (OF_READ))
		return 0;
	if (idesc > NumSymbols)
		return 0;	
	if (cachedSymbolHandle[idesc])
		return cachedSymbolHandle[idesc];
	opt = 0;
	handle = GSSiGlobAlloc ( 454,GHND,sizeof(SYMBOL));
	pSymDesc = (LPSYMBOL)GlobalLock (handle);
	if (*IconDict)
	{
		GSSillseek (FidSD,idesc-1,0);
		fgetstring (pSymDesc->Name,32,FidSD);
		goto Exit;
	}
	pSymIndex = (LPLONG)GlobalLock (hSymIndex); 
	pSymIndex += idesc-1; 
	if (*pSymIndex < 0) 
	{
		GlobalUnlock (hSymIndex);
		goto ErrOut;
	}
	ii=GSSillseek (FidSD,*pSymIndex,0);
	GlobalUnlock (hSymIndex);
	if (SymDictVersion == 1)
	{   
		HANDLE	hSym=GSSiGlobAlloc (2,GMEM_MOVEABLE,SizeSYMBOL_V1);
		LPSYMBOL_V1	pSymV1=(LPSYMBOL_V1)GlobalLock (hSym);
		
		ii=GSSilread (FidSD,pSymV1,SizeSYMBOL_V1); 
		ConvertSymbol_V1_to_V2 (pSymDesc,pSymV1);
		GSSiGlobUlFree (&hSym);
	}
	else 
		ii=GSSilread (FidSD,pSymDesc,sizeof(SYMBOL)); 
	if (pSymDesc->Number != idesc)
	{
ErrOut: 
		GSSiGlobUlFree (&handle);
		return 0; 
	}  
	if (!_fstricmp (pSymDesc->Name,"ALL"))
		pSymDesc->Parent = 0;
	if (opt)
		pSymDesc->NumElements = 0;	
	if (pSymDesc->NumElements > 1)
	{   
		short	n=pSymDesc->NumElements;
		GlobalUnlock (handle);
		handle = GSSiGlobalReAlloc (0,handle,sizeof(SYMBOL)+n*sizeof(HANDLE),GMEM_MOVEABLE);
		pSymDesc = (LPSYMBOL)GlobalLock (handle); 
	} 
	for (i=0,phElement=(LPHANDLE)&pSymDesc->hElement;i<pSymDesc->NumElements;i++,phElement++)
	{ 
		LPELEMENT	pElement;
		
		*phElement = GSSiGlobAlloc ( 455,GMEM_MOVEABLE,sizeof(ELEMENT)+
									(long)(MAX_ELEMENT_VECTORS-1)*sizeof(VECTOR));
		pElement = (LPELEMENT)GlobalLock (*phElement); 
		GSSilread (FidSD,pElement,sizeof(ELEMENT)-sizeof(VECTOR));
		if (pElement->NumVectors < 0 || pElement->NumVectors > MAX_ELEMENT_VECTORS)
			pSymDesc->NumElements = i;
		else
		{
			if (pElement->Type > 4)
				pElement->Type = 3; 
	//		if (/*pSymDesc->Type == SYMTYPELINE && */pElement->Type == SVLINE)
	//			pElement->LineColorType = SVVARCOLOR;
	/*		if (pSymDesc->Type == SYMTYPEPOINT && pElement->Type == SVAREA)
			{
				pElement->FillColorType = SVNULLCOLOR;
				pElement->LineColorType = SVVARCOLOR;
			} */

			pElement->NumVectors = min (pElement->NumVectors,MAX_ELEMENT_VECTORS);
			GSSilread (FidSD,&pElement->Vector,pElement->NumVectors*sizeof(VECTOR)); 
			GlobalUnlock (*phElement); 
			*phElement = GSSiGlobalReAlloc (0,*phElement,sizeof(ELEMENT)+(pElement->NumVectors-1)*sizeof(VECTOR),GMEM_MOVEABLE);
		}
	}
Exit:  
    GlobalUnlock (handle);
	if (SymDictOpenMode == OF_READ && allowCachedSymbols)
		cachedSymbolHandle[idesc] = handle;
	return handle;
}

void DestroySymbol (HANDLE hSymbol)
{   
	int	i,ii;
	LPSYMBOL pSymDesc;
	HANDLE	*phElement, hElement;
	
	if (!fromCloseDict && allowCachedSymbols)
		return;
	if (!hSymbol)
		return;
	pSymDesc = (LPSYMBOL)GlobalLock (hSymbol);
	for (i=0,phElement=&pSymDesc->hElement;i<pSymDesc->NumElements;i++,phElement++)
		GSSiGlobFree (phElement);
	GSSiGlobUlFree (&hSymbol);
	return;
}

void DisplayLinearSymbol (HANDLE hSymbol, HDC hDC, double width, int nPnts, LPPOINT Points)
{
	return;
}

void DisplayAreaSymbol (HANDLE hSymbol, HDC hDC, int nPnts, LPPOINT Points)
{
	return;
}

HDIB32 GetSymbolImage (LPSTR SymName)
{
	HDIB32	hDib=0;
	HANDLE	hMem = GSSiGlobAlloc (1884,GMEM_MOVEABLE,512);
	LPSTR	pBS, BMPName = GlobalLock (hMem);
		
	_fstrcpy (BMPName,"[%SYM_DICT]");
	ExpandText (BMPName);
	if ((pBS = _fstrrchr (BMPName,'\\')))  
	{   
		pBS++;
		sprintf (pBS,"%s.bmp",SymName);
		hDib = LoadDIB32(BMPName,TRUE, 0);
		if (!hDib)
		{
			strcpy (BMPName,SymbolImage);
			ExpandText (BMPName);
			hDib = LoadDIB32(BMPName,TRUE, 0);
		}
	}
	GSSiGlobUlFree (&hMem);
	return hDib;
}

double GetAverageGreyScaleValue (HDIB32 hDib,HDIB32 hDibGS)
{
	double Value;
	UINT	TotGS=0;
	UINT	nrow = FreeImage_GetHeight (hDibGS);
	UINT	ncol = FreeImage_GetWidth (hDibGS);
	UINT	irow,icol;
	UINT	TotPix=0;
	LPBYTE	pGS;
	RGBTRIPLE	*p24Bit;

	for (irow = 0;irow < nrow;irow++)
	{
		pGS = FreeImage_GetScanLine (hDibGS,irow);
		p24Bit = (RGBTRIPLE	*)FreeImage_GetScanLine (hDib,irow);
		for (icol = 0;icol < ncol;icol++,pGS++,p24Bit++)
		{
			if (p24Bit->rgbtRed != 255 ||p24Bit->rgbtGreen != 255 ||p24Bit->rgbtBlue != 255 )
			{
				TotPix++;
				TotGS += *pGS;
			}
		}
	}
	Value = TotGS;
	Value /= TotPix;
	Value /= 255;

	return Value;
}

int CreateOverlapMap(LPSTR OutImage, LPSTR InImages, COLORREF color)
{
	int rtn = 1;
	HDIB32 hOutBM=0;
	HDIB32 hInBM[32];
	RGBQUAD * pBitmap[32];
	int nInBM = 0;
	LPSTR pSC = strchr(InImages, ';');
	UINT	nrow=0, ncol=0;

	while (*InImages)
	{
		if (*pSC)
			*pSC++ = 0;
		hInBM[nInBM] = BMPHandleFromEXT(InImages);
		if (!hInBM[nInBM])
		{
			rtn = -(nInBM + 1);
			goto Exit;
		}
		nInBM++;
		InImages = pSC;
		pSC = strchr(InImages, ';');
		if (!pSC)
			pSC = strchr(InImages, 0);
	}
	if (nInBM > 1)
	{
		nrow = FreeImage_GetHeight(hInBM[0]);
		ncol = FreeImage_GetWidth(hInBM[0]);
		int		bpp = FreeImage_GetBPP(hInBM[0]);
		for (int i = 0; i < nInBM; i++)
		{
			if (nrow != FreeImage_GetHeight(hInBM[i]))
			{
				rtn = -(i + 101);
			}
			if (ncol != FreeImage_GetWidth(hInBM[i]))
			{
				rtn = -(i + 201);
			}
			if (bpp != FreeImage_GetBPP(hInBM[i]))
			{
				rtn = -(i + 301);
			}
		}
	}
	else
		rtn = -1000;
	if (rtn == 1)
	{
		hOutBM = FreeImage_Clone(hInBM[0]);
	}
	RGBQUAD blackval = { 0 };
	RGBQUAD whiteval;
	whiteval.rgbBlue = 255;
	whiteval.rgbGreen = 255;
	whiteval.rgbRed = 255;

	for (int irow = 0; irow < nrow; irow++)
	{
		int i;
		for (i = 0; i < nInBM; i++)
		{
			pBitmap[i] = (RGBQUAD	*)FreeImage_GetScanLine(hInBM[i], irow);
		}
		for (int icol = 0; icol < ncol; icol++)
		{
			int nHits = 0;
			for (i = 0; i < nInBM; i++)
			{
				COLORREF bitmapColor = RGBQUADToCOLORREF(*pBitmap[i]);
				if (bitmapColor == color)
					nHits++;
			}
			for (i = 0; i < nInBM; i++)
				pBitmap[i]++;
			if (hOutBM)
			{
				if (nHits > 1)
					FreeImage_SetPixelColor(hOutBM, icol, irow, &blackval);
				else
					FreeImage_SetPixelColor(hOutBM, icol, irow, &whiteval);
			}
		}
	}
	if (!GMFIBMPHandleToEXT(OutImage, hOutBM, 0))
		rtn = -2000;
	if (hOutBM)
		GSSiFreeImage_Unload(hOutBM);
Exit:	
	for (int i = 0; i < nInBM;i++)
		GSSiFreeImage_Unload(hInBM[i]);

	return rtn;
}
double GetPCTColorInBitmapWithMask(HDIB32 hBitmap, HDIB32 hMask, COLORREF color, COLORREF maskColor)
{
	UINT	nrow = FreeImage_GetHeight(hBitmap);
	UINT	ncol = FreeImage_GetWidth(hBitmap);
	int		bpp  = FreeImage_GetBPP(hBitmap);
	LPBYTE	pBitmap, pMask;
	int		totMaskPixels = 0;
	int		totColorPixels = 0;
	double  rtn=-3;

	if (FreeImage_GetWidth(hMask) != ncol ||
		FreeImage_GetHeight(hMask) != nrow)
		return -1;

	if (FreeImage_GetBPP(hMask) != bpp)
		return -2;

	if (bpp == 24)
	{
		for (int irow = 0; irow < nrow; irow++)
		{
			RGBTRIPLE * pBitmap = (RGBTRIPLE	*)FreeImage_GetScanLine(hBitmap, irow);
			RGBTRIPLE * pMask = (RGBTRIPLE	*)FreeImage_GetScanLine(hMask, irow);
			for (int icol = 0; icol < ncol; icol++, pBitmap++, pMask++)
			{
				COLORREF bitmapColor = RGBTRIPLEToCOLORREF(*pBitmap);
				COLORREF mColor = RGBTRIPLEToCOLORREF(*pMask);
				if (mColor == maskColor)
				{
					totMaskPixels++;
					if (bitmapColor == color)
						totColorPixels++;
				}
			}
		}
		rtn = (double)totColorPixels / (double)totMaskPixels;
	}
	else if (bpp == 32)
	{
		for (int irow = 0; irow < nrow; irow++)
		{
			RGBQUAD * pBitmap = (RGBQUAD	*)FreeImage_GetScanLine(hBitmap, irow);
			RGBQUAD * pMask = (RGBQUAD	*)FreeImage_GetScanLine(hMask, irow);
			for (int icol = 0; icol < ncol; icol++, pBitmap++, pMask++)
			{
				COLORREF bitmapColor = RGBQUADToCOLORREF(*pBitmap);
				COLORREF mColor = RGBQUADToCOLORREF(*pMask);
				if (mColor == maskColor)
				{
					totMaskPixels++;
					if (bitmapColor == color)
						totColorPixels++;
				}
			}
		}
		rtn = (double)totColorPixels / (double)totMaskPixels;

	}
	return rtn;
}

RGBTRIPLE NewColorValue (RGBTRIPLE *pColor,double intensitychange)
{
	RGBTRIPLE NewColor = *pColor, OldColor = *pColor;
	UINT	nLoops=0;
	double	ic3;
	double	dif;

	while (nLoops++ < 8 && fabs (intensitychange) > 0.001)
	{
		ic3 = intensitychange/3;
		dif = 255 * ic3/0.333;
		NewColor.rgbtRed = max (0,min (255,OldColor.rgbtRed + dif));
		intensitychange -= (0.333 * (double)(NewColor.rgbtRed - OldColor.rgbtRed))/255;
		dif = 255 * ic3/0.333;
		NewColor.rgbtGreen = max (0,min (255,OldColor.rgbtGreen + dif));
		intensitychange -= (0.333 * (double)(NewColor.rgbtGreen - OldColor.rgbtGreen))/255;
		dif = 255 * ic3/0.333;
		NewColor.rgbtBlue = max (0,min (255,OldColor.rgbtBlue + dif));
		intensitychange -= (0.333 * (double)(NewColor.rgbtBlue - OldColor.rgbtBlue))/255;
		OldColor = NewColor;
	}
	return NewColor;
}

HDIB32 ChangeBitmapColor (HDIB32 hDib,RGBTRIPLE *pNewColor)
{
//	HDIB32	hDibGS = GSSiFreeImage_ConvertTo8Bits (hDib);
	HDIB32	hDibGS = GSSiFreeImage_ConvertToGreyscale (hDib);
	HDIB32	hDibNewColor = GSSiFreeImage_ConvertTo24Bits (hDib);
	UINT	nrow = FreeImage_GetHeight (hDib);
	UINT	ncol = FreeImage_GetWidth (hDib);
	UINT	irow,icol,ii;
	LPBYTE	pGS;
	RGBTRIPLE	*p24Bit;
//	double	NewColorGrey = ((0.299 * pNewColor->rgbRed)/255 + (0.587 * pNewColor->rgbGreen)/255 + (0.114 * pNewColor->rgbBlue)/255);
	double	NewColorGrey = ((0.333 * pNewColor->rgbtRed)/255 + (0.333 * pNewColor->rgbtGreen)/255 + (0.333 * pNewColor->rgbtBlue)/255);
	double	AveGS = GetAverageGreyScaleValue (hDib,hDibGS);
	double	fac, x, v;

	if (AveGS)
	{
		fac = NewColorGrey / AveGS;

		for (irow = 0;irow < nrow;irow++)
		{
			pGS = FreeImage_GetScanLine (hDibGS,irow);
			p24Bit = (RGBTRIPLE	*)FreeImage_GetScanLine (hDibNewColor,irow);
			for (icol = 0;icol < ncol;icol++,pGS++,p24Bit++)
			{
				if (p24Bit->rgbtRed != 255 || p24Bit->rgbtGreen != 255 || p24Bit->rgbtBlue != 255 )
				{
					x = min (1.0,NewColorGrey * ((double)*pGS/255) / AveGS);
					*p24Bit = NewColorValue (pNewColor,x-NewColorGrey);
				}
			}
		}
		GSSiFreeImage_Unload(hDibGS);
	}
	return hDibNewColor;
}

BOOL DisplayTransparentBitmap(HDC hDC, HDIB32 *hDib, POINT Point, LPPOINT ptiePointBM,int width, LPRECT pBounds, LPCOLORREF pNewColor, LPCOLORREF pTranColor)
{
	HBITMAP hBMMask, hBMColor, hBMOld;
	BITMAP	bm;
	HDC		hDCMem;
	int		destw, desth, destx, desty;
	POINT	pt;
	int		OldMode=0;
	BITMAPINFOHEADER	lpbi,lpbim;
	UINT	ColorType = DIB_PAL_COLORS;
	UINT	RasterOpt=SRCCOPY;
	int		rtn,ii;
	LPVOID	pImage;
	BOOL	Printingx=TRUE;
	COLORREF	nColor;
	RGBTRIPLE	NewColor;
	COLORREF	OldColor;
	COLORREF	TranColor = RGB(255,255,255);
	int nTranColors = FreeImage_GetTransparencyCount (*hDib);
	BOOL	isTransparent = FreeImage_IsTransparent (*hDib);

//	GetColor (hWndMain,&nColor);
//	NewColor = RGBQUADFromCOLORREF (nColor);
	if (isTransparent)
	{
		int	iTran = FreeImage_GetTransparentIndex(*hDib);
		if (iTran >= 0)
		{
			RGBQUAD *pal = FreeImage_GetPalette(*hDib);
			TranColor = COLORREFFromRGBQUAD(pal[iTran]);
		}
	}
	if (pNewColor)
		NewColor = RGBTRIPLEFromCOLORREF (*pNewColor);
	if (pTranColor)
		TranColor = *pTranColor;
	GetBitmapInfoFromHandle (&lpbi,*hDib);
//    lpbi = (LPBITMAPINFOHEADER)GetDibHeader (hDib);
	if (lpbi.biBitCount != 24)
	{
		HDIB32 hDib24 = GSSiFreeImage_ConvertTo24Bits(*hDib);
		GSSiFreeImage_Unload(*hDib);
		*hDib = hDib24;
	}
//    pImage = FindDIBBits ((LPSTR)lpbi);
//    pImage = FreeImage_GetBits(hDib);
	hBMColor = DIB32ToBitmap(*hDib,(HPALETTE)0);
	GetObject(hBMColor, sizeof(bm), (LPSTR)&bm);
	if (pNewColor)
	{
		HDIB32 hDibNewColor = ChangeBitmapColor (*hDib,&NewColor);

		DeleteObject (hBMColor);
		hBMColor = DIB32ToBitmap(hDibNewColor,(HPALETTE)0);
		GSSiFreeImage_Unload(hDibNewColor);
	}

	hBMMask = CreateBitmapMask(hBMColor,TranColor);
	if (!width)
		width = bm.bmWidth;
	destw = width;
	desth = (width * bm.bmHeight)/bm.bmWidth;
	if (ptiePointBM)
	{
		destx = Point.x - ptiePointBM->x;
		desty = Point.y - ptiePointBM->y;
	}
	else
	{
		destx = Point.x -destw / 2;
		desty = Point.y -desth / 2;
	}
	if (pBounds)
	{
		POINT p;

		p.x = destx;
		p.y = desty;
		AddPointToRect (p,pBounds); 
		p.y += desth;
		AddPointToRect (p,pBounds); 
		p.x += destw;
		AddPointToRect (p,pBounds); 
		p.y = desty;
		AddPointToRect (p,pBounds); 
	}
	ii=1;
	if (Printing && ii)
	{
//		HDIB32	hDibMask = BitmapToDIB32 (hBMMask);
		HDIB32	hDib24 = GSSiFreeImage_ConvertTo24Bits (*hDib);
		HDIB32	hDib8 = GSSiFreeImage_ColorQuantize (hDib24,FIQ_NNQUANT);
//		HDIB32	hDibInvert = FreeImage_Allocate (bm.bmWidth,bm.bmHeight,1,0,0,0);
		HDIB32	hDibMask = GSSiFreeImage_Allocate (bm.bmWidth,bm.bmHeight,1,0,0,0);
		LPBYTE	pBits = FreeImage_GetBits (hDibMask);
		RGBQUAD *pal = FreeImage_GetPalette(hDibMask);
		BYTE	black=0,white=0xFF;
		int		i;

//		SaveDIB32 (hDib8,"c:\\temp\\temp8.bmp",-1,0);
//		SaveDIB32 (hDib,"c:\\temp\\temp.bmp",-1,0);
		if (bm.bmWidth % 32)
			i = (bm.bmWidth + 32 - bm.bmWidth % 32) / 8;
		else
			i = bm.bmWidth / 8;
		GetBitmapBits(hBMMask,i*bm.bmHeight,pBits);
/*for (i=0;i<(bm.bmWidth*bm.bmHeight)/8;i++)
 if (i%2)
	 *pBits++=black;
 else
	 *pBits++=white;*/
		pal++;
		pal->rgbBlue =  pal->rgbRed = pal->rgbGreen = pal->rgbReserved = 255;
		FreeImage_FlipVertical (hDibMask);
		pal = FreeImage_GetPalette(hDib8);
		for (i=0;i<256;i++,pal++)
			if (pal->rgbBlue == 255 && pal->rgbGreen == 255 && pal->rgbRed == 255)
				pal->rgbBlue =  pal->rgbRed = pal->rgbGreen = 0;
//SaveDIB32 (hDibMaskx,"c:\\tempx.bmp",-1,0);
//SaveDIB32 (hDibMask,"c:\\temp.bmp",-1,0);
//SaveDIB32 (hDibMask8,"c:\\temp8.bmp",-1,0);
		GetBitmapInfoFromHandle (&lpbim,hDib8);
//	    lpbim = (LPBITMAPINFOHEADER)GetDibHeader (hDib8);
//destw =lpbim->biWidth; desth= lpbim->biHeight;
//for (i=0;i<16;i++)
{
		rtn=StretchDIBitsFromHandle (hDC,destx,desty,//-i*(5+desth),
	                   destw, desth,
	                   0,0,
	                   lpbim.biWidth, lpbim.biHeight,
	                   hDibMask,
	                  (UINT)DIB_RGB_COLORS,
	                  (DWORD) SRCAND,1);
		rtn=StretchDIBitsFromHandle (hDC,destx,desty,//-i*(5+desth),
	                   destw, desth,
	                   0,0,
	                   lpbim.biWidth, lpbim.biHeight,
	                   hDib8,
	                  (UINT)DIB_RGB_COLORS,
	                  (DWORD) SRCPAINT,1);//RastOpts[i],1);
	}
//		DestroyDIB32(hDibInvert,FALSE);
		DestroyDIB32(hDibMask,FALSE);
		DestroyDIB32(hDib24,FALSE);
		DestroyDIB32(hDib8,FALSE);
	}
	else
	{
		hDCMem = CreateCompatibleDC(hDC); 
		if (Printing)
			hBMOld = SelectObject(hDCMem, hBMColor);
		else
		{

			hBMOld = SelectObject(hDCMem, hBMMask);
			OldMode = SetStretchBltMode(hDC,HALFTONE); 
			SetBrushOrgEx (hDC,0,0,&pt);
			OldColor = SetTextColor (hDC,0);
			StretchBlt(hDC, destx, desty,destw,desth, hDCMem, 0, 0,bm.bmWidth, bm.bmHeight,  SRCAND);
			SelectObject(hDCMem, hBMColor);
			if (OldColor != CLR_INVALID)
				SetTextColor (hDC,OldColor);
			RasterOpt = SRCPAINT;
		}
		StretchBlt(hDC, destx, desty,destw,desth, hDCMem, 0, 0,bm.bmWidth, bm.bmHeight,  RasterOpt);
		//BitBlt(hDC, 0, bm.bmHeight, bm.bmWidth, bm.bmHeight, hDCMem, 0, 0, SRCPAINT);
		SelectObject (hDCMem,hBMOld);
		DeleteDC (hDCMem);
		if (OldMode)
			SetStretchBltMode(hDC,OldMode); 
	}
	DeleteObject (hBMMask);
	DeleteObject (hBMColor);
	return TRUE;
}

BOOL DisplayTransparentBitmapInRect(HDC hDC, HDIB32 hDib, LPRECT pRect, BOOL MaintainAspect)
{
	HBITMAP hBMMask, hBMColor, hBMOld;
	BITMAP	bm;
	HDC		hDCMem;
	POINT	pt;
	int		OldMode=0;
	BITMAPINFOHEADER	lpbi,lpbim;
	UINT	ColorType = DIB_PAL_COLORS;
	UINT	RasterOpt=SRCCOPY;
	int		rtn,ii;
	LPVOID	pImage;
	BOOL	Printingx=TRUE;
	COLORREF	nColor;
	RGBTRIPLE	NewColor;
	COLORREF	OldColor;
	BITMAPINFOHEADER DibInfo;
	RECT Rect = *pRect;

//	GetColor (hWndMain,&nColor);
//	NewColor = RGBQUADFromCOLORREF (nColor);

	GetBitmapInfoFromHandle (&lpbi,hDib);
//    lpbi = (LPBITMAPINFOHEADER)GetDibHeader (hDib);
	if (lpbi.biBitCount > 8)
		ColorType = DIB_RGB_COLORS;
//    pImage = FindDIBBits ((LPSTR)lpbi);
//    pImage = FreeImage_GetBits(hDib);
	hBMColor = DIB32ToBitmap(hDib,(HPALETTE)0);
	GetObject(hBMColor, sizeof(bm), (LPSTR)&bm);
	GetBitmapInfoFromHandle(&DibInfo, hDib);

	hBMMask = CreateBitmapMask(hBMColor,RGB(255,255,255));
	if (MaintainAspect)
		ComputeBMLoc(*pRect, (LPBITMAPINFO)&DibInfo, MaintainAspect);
	else
	{
		destX = (int)Rect.left;
		destY = (int)Rect.top;
		destW = (int)(Rect.right - Rect.left + 1);
		destH = (int)(Rect.bottom - Rect.top + 1);
	}

	ii=1;
	if (Printing && ii)
	{
//		HDIB32	hDibMask = BitmapToDIB32 (hBMMask);
		HDIB32	hDib24 = GSSiFreeImage_ConvertTo24Bits (hDib);
		HDIB32	hDib8 = GSSiFreeImage_ColorQuantize (hDib24,FIQ_NNQUANT);
//		HDIB32	hDibInvert = FreeImage_Allocate (bm.bmWidth,bm.bmHeight,1,0,0,0);
		HDIB32	hDibMask = GSSiFreeImage_Allocate (bm.bmWidth,bm.bmHeight,1,0,0,0);
		LPBYTE	pBits = FreeImage_GetBits (hDibMask);
		RGBQUAD *pal = FreeImage_GetPalette(hDibMask);
		BYTE	black=0,white=0xFF;
		int		i;

//		SaveDIB32 (hDib8,"c:\\temp\\temp8.bmp",-1,0);
//		SaveDIB32 (hDib,"c:\\temp\\temp.bmp",-1,0);
		if (bm.bmWidth % 32)
			i = (bm.bmWidth + 32 - bm.bmWidth % 32) / 8;
		else
			i = bm.bmWidth / 8;
		GetBitmapBits(hBMMask,i*bm.bmHeight,pBits);
/*for (i=0;i<(bm.bmWidth*bm.bmHeight)/8;i++)
 if (i%2)
	 *pBits++=black;
 else
	 *pBits++=white;*/
		pal++;
		pal->rgbBlue =  pal->rgbRed = pal->rgbGreen = pal->rgbReserved = 255;
		FreeImage_FlipVertical (hDibMask);
		pal = FreeImage_GetPalette(hDib8);
		for (i=0;i<256;i++,pal++)
			if (pal->rgbBlue == 255 && pal->rgbGreen == 255 && pal->rgbRed == 255)
				pal->rgbBlue =  pal->rgbRed = pal->rgbGreen = 0;
//SaveDIB32 (hDibMaskx,"c:\\tempx.bmp",-1,0);
//SaveDIB32 (hDibMask,"c:\\temp.bmp",-1,0);
//SaveDIB32 (hDibMask8,"c:\\temp8.bmp",-1,0);
		GetBitmapInfoFromHandle (&lpbim,hDib8);
//	    lpbim = (LPBITMAPINFOHEADER)GetDibHeader (hDib8);
//destw =lpbim->biWidth; desth= lpbim->biHeight;
//for (i=0;i<16;i++)
{
		rtn=StretchDIBitsFromHandle (hDC,destX,destY,//-i*(5+desth),
	                   destW, destH,
	                   0,0,
	                   lpbim.biWidth, lpbim.biHeight,
	                   hDibMask,
	                  (UINT)DIB_RGB_COLORS,
	                  (DWORD) SRCAND,1);
		rtn=StretchDIBitsFromHandle (hDC,destX,destY,//-i*(5+desth),
	                   destW, destH,
	                   0,0,
	                   lpbim.biWidth, lpbim.biHeight,
	                   hDib8,
	                  (UINT)DIB_RGB_COLORS,
	                  (DWORD) SRCPAINT,1);//RastOpts[i],1);
	}
//		DestroyDIB32(hDibInvert,FALSE);
		DestroyDIB32(hDibMask,FALSE);
		DestroyDIB32(hDib24,FALSE);
		DestroyDIB32(hDib8,FALSE);
	}
	else
	{
		hDCMem = CreateCompatibleDC(hDC); 
		if (Printing)
			hBMOld = SelectObject(hDCMem, hBMColor);
		else
		{

			hBMOld = SelectObject(hDCMem, hBMMask);
			OldMode = SetStretchBltMode(hDC,HALFTONE); 
			SetBrushOrgEx (hDC,0,0,&pt);
			OldColor = SetTextColor (hDC,0);
			StretchBlt(hDC, destX, destY,destW,destH, hDCMem, 0, 0,bm.bmWidth, bm.bmHeight,  SRCAND);
			SelectObject(hDCMem, hBMColor);
			if (OldColor != CLR_INVALID)
				SetTextColor (hDC,OldColor);
			RasterOpt = SRCPAINT;
		}
		StretchBlt(hDC, destX, destY,destW,destH, hDCMem, 0, 0,bm.bmWidth, bm.bmHeight,  RasterOpt);
		//BitBlt(hDC, 0, bm.bmHeight, bm.bmWidth, bm.bmHeight, hDCMem, 0, 0, SRCPAINT);
		SelectObject (hDCMem,hBMOld);
		DeleteDC (hDCMem);
		if (OldMode)
			SetStretchBltMode(hDC,OldMode); 
	}
	DeleteObject (hBMMask);
	DeleteObject (hBMColor);
	return TRUE;
}

HANDLE DisplayPointSymbol (HANDLE hSymbol, HDC hDC, double Vsize, double Hsize, double rotation, LPPOINT pTiePoint,
						   LPRECT pBounds, BOOL Highlight, short nElementsToDisplay, HANDLE hElementsToDisplay,
						   BOOL SaveCurrentScreen,BOOL UseHalftoneVis,LPSTR txt,LPPOINT TextPoly)
{   
	int	i,j;
	long	NumElementPoints;
	HANDLE	*phElement;   
	DPOINT	TiePoint, Newpt;
	double	Vsizefactor, Hsizefactor, SymVSize, SymHSize, LineWidthFactor;  
	LPSYMBOL	lpSym;   
	HANDLE	hCurElement, hSymbolInvis=0, hSaveScreen=0;
	BOOL	Invis;
	RECT	Rect;
	short	idesc=0,ii, htdesc=-1; 
	int		OldRop=0, CurRop;
	HDIB32	hDib;

	nExportSymElements = 0;
	if (!hSymbol)
		return 0;  
	if (hDC > (HDC)100) //hDC >0 and <99 is FID - exports symbol to DXF block, 99 exports to current edit file
	{
		SaveDC(hDC);
		OldRop = SetROP2(hDC, R2_COPYPEN);
	}
	lpSym = (LPSYMBOL)GlobalLock (hSymbol);  
	idesc = lpSym->Number;
	if (lpSym->BlockRotation && CurView)
	{
		if (GetGraphicsMode(hDC) == GM_COMPATIBLE)
			rotation = 0;
		else
			rotation -= CurView->Rotation;
	}
	if (lpSym->Type != 1 || lpSym->InVisible || !lpSym->VSize || !lpSym->HSize)
	{   
		if (lpSym->Type != 1)
		{
			if (ShowBadSyms)
				goto Next;
		}
		if (CurVis) 
		{
			if (!CurVis->WantType[8] && !ItemIsHighlighted)
				goto Exit;
		}
Next:
		GlobalUnlock (hSymbol);   
		if (InvisiblePointSymbol)
		{
			hSymbol = hSymbolInvis = GetDictSymDesc (InvisiblePointSymbol,0);
		}
		else
		{
			InvisiblePointSymbol = GetDictSymbolNumber ("CIRCLE");
			if (!InvisiblePointSymbol)
				goto Exit2;
			hSymbol = hSymbolInvis = GetDictSymDesc (InvisiblePointSymbol,0);
		} 

		lpSym = (LPSYMBOL)GlobalLock (hSymbol);
		 
		Hsize *= InvisPointFactor;
		Vsize *= InvisPointFactor; 
	} 
	else if (ShowBadSyms)
		goto Exit2;  

	if (lpSym->BaseScale == 15)  
	{
		SymVSize = SymHSize =  1;
		SymHSize = SymVSize = (((FTM * lpSym->VSize)/fabs (lpSym->SizePointV[0].y - lpSym->SizePointV[1].y)) / CurView->BaseUnitsPerPixel)/DefaultSymbolFactor;
	}
	else if (lpSym->GetDimensionsFromSizePoints)
	{   
		if (lpSym->HSize)
			SymHSize = lpSym->SizePointH[1].x /lpSym->HSize;  
		else
			SymHSize = lpSym->SizePointH[1].x;
		if (lpSym->VSize)
			SymVSize = lpSym->SizePointV[1].y /lpSym->VSize;
		else
			SymVSize = lpSym->SizePointV[1].y;
	}
	else
	{
		SymVSize = lpSym->VSize;
		SymHSize = lpSym->HSize;
	}
	if (!SymVSize)
		SymVSize = 1;
	if (!SymHSize)
		SymHSize = 1;
	TiePoint.x = pTiePoint->x;
	TiePoint.y = pTiePoint->y;
	if (Vsize < 0)
	{
		Vsizefactor = Hsizefactor = -Vsize/SymVSize;
		if (SymHSize * Hsizefactor > Hsize) 
			Hsizefactor = Vsizefactor = Hsize/SymHSize;  
	}
	else if (!Vsize)
	{
		Hsizefactor = Vsizefactor = Hsize/SymHSize;  
	}
	else if (!Hsize)
	{
		Vsizefactor = Hsizefactor = Vsize/SymVSize;  
	}  
	else
	{
		Vsizefactor = Vsize/SymVSize;
		Hsizefactor = Hsize/SymHSize;
		Hsizefactor = Vsizefactor;
	} 
	LastBaseSize = Hsizefactor;
	if (lpSym->BaseSizeSet) 
		LineWidthFactor = Hsizefactor / lpSym->BaseSize;
	else
		LineWidthFactor = 1;
	if (SaveCurrentScreen)
	{   
		Rect.left = Rect.right = TiePoint.x;
		Rect.top = Rect.bottom = TiePoint.y;
		InflateRect (&Rect,(int)IDNINT(Vsizefactor*SymVSize),(int)IDNINT(Hsizefactor*SymHSize));
		hSaveScreen = SaveScreen2 (hWndMain,hDC,Rect,0,0);
	}
	if (lpSym->HasBitmap && !lpSym->NumElements)
	{
		if ((hDib = GetSymbolImage (lpSym->Name)))
		{
			if (lpSym->BaseScale == 15)
				DisplayTransparentBitmap (hDC,&hDib,*pTiePoint,0,0,pBounds,0,0);
			else
				DisplayTransparentBitmap (hDC,&hDib,*pTiePoint,0,IDNINT(Hsize*lpSym->HSize),pBounds,0,0);
			DestroyDIB32(hDib, FALSE);
		}
	}
	for (i=0,phElement=&lpSym->hElement;i<lpSym->NumElements;i++,phElement++)
	{ 
		LPELEMENT	pElement;
		LPVECTOR	pVector;
		LPPOINT		pPoint;
		HPEN		hPen=0, hCurPen=0;
		HBRUSH		hBrush=0, hCurBrush; 
		DPOINT		POC, PC;
		BOOL		HavePOC=FALSE;    
		short		Width=1;
		
		if (UseHalftoneVis)
			htdesc = idesc;
		else
			htdesc = -1;
		if (nElementsToDisplay && !ItemInList (i,nElementsToDisplay,hElementsToDisplay))
			goto SkipElement;
		pElement = (LPELEMENT)GlobalLock (*phElement);
		if (hDC > (HDC)100)
		{   
			hBrush = 0;
			if (pElement->Type == SVAREA)
			{
				if (Highlight)
				{   LOGBRUSH    NDB;
				    
			        if (PatternBrush || Printing)
			        {
			            NDB.lbStyle = BS_HATCHED;
			            NDB.lbColor = HighlightColor;
			            NDB.lbHatch = HS_DIAGCROSS;
			            hBrush =  CreateBrushIndirect(&NDB);
			        }
			        else
						hBrush = CreateSolidBrush (HighlightColor);
				}
				else if (pElement->FillColor < -9 || (pElement->FillColorType == SVVARCOLOR && HaveVarFillColor))
					hBrush = CreateGMBrush (GlobalColors[0],htdesc,hDC);
				else if (pElement->FillColor < 0)
					hBrush = CreateGMBrush (GlobalColors[labs(pElement->FillColor)],htdesc,hDC);
				else
					hBrush = CreateGMBrush (pElement->FillColor,htdesc,hDC);
			}  
			if (GetTypeVisibility (9))
				Width = max (1,pElement->Width) * LineWidthFactor;
			else
				Width = 1;
			if (hBrush)
				hCurBrush = SelectObject (hDC,hBrush); 
			if (Highlight)
				hPen = CreatePen (PS_SOLID,abs(HighlightWidth),HighlightColor); 
			else if  (pElement->Type == SVAREA && pElement->LineColorType == SVNULLCOLOR)  
				hPen = GetStockObject (NULL_PEN);
			else if (pElement->LineColor < -9 || (pElement->LineColorType == SVVARCOLOR && HaveVarFillColor))
				hPen = CreatePen (PS_SOLID,Width,ConvertColor(GlobalColors[0],idesc));
			else if (pElement->LineColor < 0)
				hPen = CreatePen (PS_SOLID,Width,ConvertColor(GlobalColors[min(9,labs(pElement->LineColor))],idesc));
			else
				hPen = CreatePen (PS_SOLID,Width,ConvertColor(pElement->LineColor,idesc)); 
			if (hPen)
				hCurPen = SelectObject (hDC,hPen);  
		}		
		
		NumElementPoints = 0;    
		hCurElement = GSSiGlobAlloc ( 456,GMEM_MOVEABLE,(long)MAX_POLY_POINTS*sizeof(POINT));
		pPoint = (LPPOINT)GlobalLock (hCurElement);
		for (j=0,pVector=&pElement->Vector;j<pElement->NumVectors;j++,pVector++)
		{   
			if (pVector->Type==2) 
			{
				if (HavePOC)
					pVector->Type = 1;
				else
					PC = Newpt;
			}
			Newpt = dnewpt (TiePoint,pVector->AZM+rotation,pVector->Dist);
			if (!hDC || hDC > (HDC)100) 
				Newpt.y = TiePoint.y - (Newpt.y - TiePoint.y); 
			Newpt.x = TiePoint.x + (Newpt.x - TiePoint.x) * Hsizefactor;
			Newpt.y = TiePoint.y + (Newpt.y - TiePoint.y) * Vsizefactor; 
			if (HavePOC) 
			{
				HavePOC = FALSE;
				CurvePoints (&PC,&POC, &Newpt, &NumElementPoints,&pPoint,2,MAX_POLY_POINTS-2); 
			} 
			else if (pVector->Type==2)
			{
				POC = Newpt;
				HavePOC = TRUE;
			}
			else
			{
				pPoint->x = IDNINT(Newpt.x);
				pPoint->y = IDNINT(Newpt.y);
				if (!NumElementPoints || !SamePoint (*pPoint,*(pPoint-1)))
				{
					pPoint++;
					NumElementPoints++; 
				}
			}
		} 
		GlobalUnlock (hCurElement); 
		pPoint = (LPPOINT)GlobalLock (hCurElement); 
		if (hDC > (HDC)100)// 4/20/04 && !pBounds)
		{   
			short	Type = pElement->Type;
			
			if (NumElementPoints < 3) 
			{
				Type = SVLINE; 
				if (NumElementPoints == 1) 
				{
					NumElementPoints = 2;
					pPoint[1] = pPoint[0];
					if (!CanDisplayZeroLenghtLines)
						pPoint[1].x++;
				}
			}
			CurRop = GetROP2(hDC);
			switch (Type)
			{
				case SVLINE:
					SetROP2 (hDC,OldRop);
					Polyline (hDC,pPoint,(int)NumElementPoints); 
					break;
				case SVAREA:
				{
					if (lpSym->HasBitmap && i+1 == lpSym->NumElements)
					{
						HANDLE	hPnt = 0;
						LPPOINT pPointScreen;
						if ((hDib = GetSymbolImage (lpSym->Name)))
						{ 
							if (CurView->ConvertToGray)
							{
								HDIB32	hDibGray = FreeImage_ConvertToGreyscale (hDib);
								HDIB32  hDib32 = FreeImage_ConvertTo32Bits(hDibGray);
								DestroyDIB32(hDib, FALSE);
								DestroyDIB32(hDibGray, FALSE);
								hDib = hDib32;
							}
							if (hPnt)
							{
								HRGN hRgn, hNewRgn;
								int	TypeRegion;

								if (GetGraphicsMode(hDC) == GM_ADVANCED)
								{
									UINT	j;
									
									hPnt = GSSiGlobAlloc (0,GMEM_MOVEABLE,sizeof(POINT)*NumElementPoints);
									pPointScreen = GlobalLock (hPnt);
									for (j=0;j<NumElementPoints;j++)
										pPointScreen[j] = TranPoint16 (pPoint[j],CurView->hTranVPToScreen);
								}
								else
									pPointScreen = pPoint;
								hRgn = CreatePolygonRgn (pPointScreen,(int)NumElementPoints,ALTERNATE);
                        
		    					if (CurView->hRgn)
									TypeRegion = CombineRgn (hRgn,CurView->hRgn,hRgn,RGN_AND);
						    	SelectClipRgn (hDC,hRgn); 
					        	GSSiDeleteObject (&hRgn);
								RectInit (&Rect);
						    	GetPolyRect (hCurElement,NumElementPoints,&Rect);
								if (!i)
									FillRect (hDC,&CurView->ScreenRect,GetStockObject (WHITE_BRUSH));
								DisplayBMInRect32 (hDC,hDib,Rect,TRUE);
								GSSiGlobUlFree (&hPnt);
							}
							else
							{
								if (ItemIsHighlighted)
								{
									COLORREF hColor = ConvertColor(HighlightColor, lpSym->Number);
									RECT rect = RectFromPointAndWidth (*pTiePoint, max(1, IDNINT(Hsize * lpSym->HSize)));
									InflateRect(&rect, 1, 1);
									FillRectColor(hDC, &rect, hColor);
								}
								if (pElement->FillColorType == SVVARCOLOR && HaveVarFillColor)
									DisplayTransparentBitmap (hDC,&hDib,*pTiePoint,0,max(1,IDNINT(Hsize*lpSym->HSize)),pBounds,&GlobalColors[0],0);
								else
									DisplayTransparentBitmap (hDC,&hDib,*pTiePoint,0,max(1,IDNINT(Hsize*lpSym->HSize)),pBounds,0,0);
							}
							DestroyDIB32(hDib,FALSE);
						}

					} 
					else if (hBrush) 
					{  
						Polygon (hDC,pPoint,(int)NumElementPoints); 
						if (CurRop == R2_MASKPEN)
						{
							SetROP2 (hDC,OldRop);
							Polyline (hDC,pPoint,(int)NumElementPoints); 
						} 
					}
					else
					{ 
						SetROP2 (hDC,OldRop);
						Polyline (hDC,pPoint,(int)NumElementPoints);
					} 
					break; 
				}
			} 
			if (pSymbolRect)
				AddPointsToSymbolRect (hDC,pPoint,NumElementPoints,Width);
		}
		else if (hDC && hDC < (HDC)99) // output symbol to dxf
		{   
			switch (pElement->Type)
			{
				case SVLINE:
					if (NumElementPoints == 2)
					{
						DPOINT	BP=PointToDPoint (pPoint[0]);
						DPOINT	EP=PointToDPoint (pPoint[1]);
						BP.x /= Hsize;
						BP.y /= Vsize;
						EP.x /= Hsize;
						EP.y /= Vsize;
						DXFOutLine (DXFHandle++,"0",(HFILE)hDC,&BP,&EP,pElement->Width);
					}
					else
					{                        
						HANDLE		hDPoint = GSSiGlobAlloc ( 457,GMEM_MOVEABLE,NumElementPoints*sizeof(DPOINT));
						HPDPOINT	pDPoint = (HPDPOINT)GlobalLock (hDPoint);
						for (j=0;j<NumElementPoints;j++) 
						{
							pDPoint[j] = PointToDPoint (pPoint[j]); 
							pDPoint[j].x /= Hsize;
							pDPoint[j].y /= Vsize;
						}
						DXFOutPolyline (&DXFHandle,"0",(HFILE)hDC,NumElementPoints,pDPoint,pElement->Width);
						GSSiGlobUlFree (&hDPoint);
					}
					break;
				case SVAREA:
					{                        
						HANDLE		hDPoint = GSSiGlobAlloc ( 458,GMEM_MOVEABLE,NumElementPoints*sizeof(DPOINT));
						HPDPOINT	pDPoint = (HPDPOINT)GlobalLock (hDPoint);
						for (j=0;j<NumElementPoints;j++)
						{
							pDPoint[j] = PointToDPoint (pPoint[j]); 
							pDPoint[j].x /= Hsize;
							pDPoint[j].y /= Vsize;
						}
						DXFOutPolygon (&DXFHandle,"0",(HFILE)hDC,NumElementPoints,pDPoint);
						GSSiGlobUlFree (&hDPoint);
					}
					break; 
			}
		}
		else if (hDC == (HDC)99) // output symbol to edit file
		{   
			HANDLE		hDPoint = GSSiGlobAlloc ( 458,GMEM_MOVEABLE,NumElementPoints*sizeof(DPOINT));
			HPDPOINT	pDPoint = (HPDPOINT)GlobalLock (hDPoint);   
			int			np=NumElementPoints;   
			short		LineSymNum=GetDictSymbolNumber ("PEN1"),AreaSymNum = GetDictSymbolNumber ("FILL1");
			long		Refno=((long)idesc)*1000+1; 
			char		UDI[66];

			for (j=0;j<NumElementPoints;j++)
			{
				pDPoint[j] = PointToDPoint (pPoint[j]); 
				pDPoint[j].x /= Hsize;
				pDPoint[j].y /= Vsize;
			} 
			GlobalUnlock (hDPoint);
			switch (pElement->Type)
			{
				case SVLINE:
					AddPolyToMap (1,&np, &hDPoint,1,Refno,0,-1,LineSymNum,0,"SYMDEF",UDI,-1,-1,-1,0,0,0,0,TRUE,0);
					break;
				case SVAREA:
					AddPolyToMap (1,&np, &hDPoint,0,Refno,0,-1,AreaSymNum,0,"SYMDEF",UDI,-1,-1,-1,0,0,0,0,TRUE,0);
					break; 
			}
			GSSiGlobFree (&hDPoint);
		} 
		if (pBounds)
			for (j=0;j<NumElementPoints;j++)
				AddPointToRect (pPoint[j],pBounds); 
		
		GSSiGlobUlFree (&hCurElement);    
		if (phElement)
			GlobalUnlock (*phElement);
		if (hPen && hPen != GetStockObject (NULL_PEN)) 
		{
			if (hCurPen)
				SelectObject (hDC,hCurPen);
			if (hPen != hCurPen)
				DeleteObject (hPen);   
		}
		if (hBrush)	
		{	
			SelectObject (hDC,hCurBrush);
			DeleteObject (hBrush);  
		}
SkipElement:;				
	} 
Exit:
	GlobalUnlock (hSymbol);
Exit2:
	DestroySymbol (hSymbolInvis); 
	if (hDC > (HDC)1)
		RestoreDC (hDC,-1);
	return hSaveScreen;
}

void CloseSymDict (void)
{   
	GSSiGlobFree (&hSymIndex); 
	GSSiGlobFree (&hChildList);    
	if (FidSD >= 0)
		GSSiClose2 (&FidSD);
//	if (SymDictOpenMode != OF_READ)  
   		GSSiGlobFree (&hSymbolAttributes); 
   		GSSiGlobFree (&hSymNames); 
		fromCloseDict = TRUE;
		for (int i = 0; i <= NumSymbols; i++)
		{
			if (cachedSymbolHandle[i])
			{
				DestroySymbol(cachedSymbolHandle[i]);
				cachedSymbolHandle[i] = 0;
			}
		}
		fromCloseDict = FALSE;
   	SymDictOpenMode = 0;
	*IconDict = 0;
	return;
} 

BOOL ReplaceSymbol (int SymNum,HANDLE hCurSymbol)
{   short	Signature, Version;
	LPSYMBOL	pCurSym, pNewSym;
	HANDLE	hBuf, hCurSym;
	HPSTR	pBuf;
	long	lBuf, lCurrent;
	LPLONG	pSymIndex;  
	
	if (!hCurSymbol)
		return FALSE;
	if (!OpenSymDict (OF_READWRITE))
		return FALSE;   
	hCurSym = GetDictSymDesc (SymNum,0);
	if (!hCurSym)
		return FALSE; 
	pCurSym = (LPSYMBOL)GlobalLock (hCurSym);
	lCurrent = SymbolToBuffer (pCurSym,&hBuf,0,0); 
	GlobalUnlock (hCurSym);
	DestroySymbol (hCurSym);
	GSSiGlobFree (&hBuf);
	pNewSym = (LPSYMBOL)GlobalLock (hCurSymbol);
	pNewSym->Number = SymNum;
	lBuf = SymbolToBuffer (pNewSym,&hBuf,0,0);
	pBuf = GlobalLock (hBuf);   
    pSymIndex = (LPLONG)GlobalLock (hSymIndex);
	pSymIndex += SymNum-1; 
	if (*pSymIndex >= 0 && lCurrent >= lBuf && SymNum <= NumSymbols)
	{
		GSSillseek (FidSD,*pSymIndex,0);
		BigWrite (FidSD,pBuf,lBuf,-1);
	}
	else
	{	 
		GSSillseek (FidSD,SymbolFileEnd,0);  
		*pSymIndex = SymbolFileEnd;
		BigWrite (FidSD,pBuf,lBuf,-1);
		GlobalUnlock (hSymIndex);
	    pSymIndex = (LPLONG)GlobalLock (hSymIndex);
	    SymbolFileEnd = GSSillseek(FidSD,0,1);
		BigWrite (FidSD,(HPSTR)pSymIndex,NumSymbols*sizeof(long),-1);
		Signature = 28051;
		Version = 1;
		BigWrite (FidSD,(HPSTR)&NumSymbols,2,-1);
	    BigWrite (FidSD,(HPSTR)&FirstSymbolNum,2,-1);
	    BigWrite (FidSD,(HPSTR)&Signature,2,-1);
	    BigWrite (FidSD,(HPSTR)&Version,2,-1);  
	}
	GlobalUnlock (hSymIndex); 
	GlobalUnlock (hCurSymbol);
	GSSiGlobUlFree (&hBuf);
	return TRUE;
} 

BOOL DeleteSymbol (int SymNum,BOOL Close)
{   short	Signature, Version;
	HANDLE	hBuf, hCurSym;
	HPSTR	pBuf;
	long	lBuf, lCurrent;
	LPLONG	pSymIndex; 
	BOOL	rtn = FALSE; 
	
	if (!SymNum)
		return FALSE;
	if (!OpenSymDict (OF_READWRITE))
		return FALSE;
	if (SymNum <= NumSymbols)
	{
		rtn = TRUE;   
	    pSymIndex = (LPLONG)GlobalLock (hSymIndex);
		pSymIndex += SymNum-1; 
		*pSymIndex = -1;
		GlobalUnlock (hSymIndex); 
		GSSillseek (FidSD,SymbolFileEnd,0);  
	    pSymIndex = (LPLONG)GlobalLock (hSymIndex);
		BigWrite (FidSD,(HPSTR)pSymIndex,NumSymbols*sizeof(long),-1);
		Signature = 28051;
		Version = 1;
		BigWrite (FidSD,(HPSTR)&NumSymbols,2,-1);
	    BigWrite (FidSD,(HPSTR)&FirstSymbolNum,2,-1);
	    BigWrite (FidSD,(HPSTR)&Signature,2,-1);
	    BigWrite (FidSD,(HPSTR)&Version,2,-1);  
		GlobalUnlock (hSymIndex); 
	} 
	if (Close)
		CloseSymDict(); 
	return TRUE;
} 

BOOL SetMaxSymNum (short MaxSym)
{
	LPLONG		pSymIndex;
    short	Signature, Version; 
    
	if (!OpenSymDict (OF_READWRITE))
		return FALSE;
	if (MaxSym <= NumSymbols)
		goto Exit; 
	if (hSymIndex)
	{
		GlobalUnlock (hSymIndex);	
		hSymIndex = GSSiGlobalReAlloc (0,hSymIndex,(MaxSym)*sizeof(long),GMEM_MOVEABLE);   
	}
	else 
		hSymIndex = GSSiGlobAlloc ( 459,GMEM_MOVEABLE,(MaxSym)*sizeof(long));   
    pSymIndex = (LPLONG)GlobalLock (hSymIndex);
	pSymIndex += NumSymbols;
	while (NumSymbols++ < MaxSym)
		*pSymIndex++ = -1;
	GlobalUnlock (hSymIndex);	
	GSSillseek (FidSD,SymbolFileEnd,0);
    pSymIndex = (LPLONG)GlobalLock (hSymIndex);
	BigWrite (FidSD,(HPSTR)pSymIndex,NumSymbols*sizeof(long),-1);
	Signature = 28051;
	Version = 1;
	BigWrite (FidSD,(HPSTR)&NumSymbols,2,-1);
    BigWrite (FidSD,(HPSTR)&FirstSymbolNum,2,-1);
    BigWrite (FidSD,(HPSTR)&Signature,2,-1);
    BigWrite (FidSD,(HPSTR)&Version,2,-1);  
	GlobalUnlock (hSymIndex); 
Exit:
    CloseSymDict();  
	return TRUE;
}

int SaveSymbol (HANDLE hCurSymbol,int isym)
{   short	Signature, Version; 
	int		i, CurSymNum;          
	HANDLE	*phElement;
	LPSYMBOL CurSymbol;   
	HANDLE	hBuf;
	HPSTR	pBuf;
	long	lBuf;
	LPLONG		pSymIndex;
	BOOL	ReplaceSym = FALSE;
	
	CurSymbol = (LPSYMBOL)GlobalLock (hCurSymbol);
	if (isym < 0)
	{
		ReplaceSym = TRUE;
		isym = -isym;
	}
	else if ((CurSymNum = GetDictSymbolNumber (CurSymbol->Name)))
	{
		ReplaceSym = TRUE;
		isym = CurSymNum;
	}
	if (!NumSymbols) 
    	hSymIndex = GSSiGlobAlloc ( 460,GMEM_MOVEABLE,2*sizeof(long));  
    pSymIndex = (LPLONG)GlobalLock (hSymIndex); 
    if (isym)
    {   
    	if (isym > NumSymbols)
    	{
		    GlobalUnlock (hSymIndex);
		    NumSymbols = isym;	
			hSymIndex = GSSiGlobalReAlloc (0,hSymIndex,(NumSymbols+2)*sizeof(long),GHND);
		    pSymIndex = (LPLONG)GlobalLock (hSymIndex); 
		}
    	pSymIndex += (isym-1);
    	if (*pSymIndex < 0 || ReplaceSym)
    		goto FoundUnusedSym;
    }
    GlobalUnlock (hSymIndex);
    pSymIndex = (LPLONG)GlobalLock (hSymIndex); 
    for (isym = 1;isym <= NumSymbols; isym++,pSymIndex++)
    {
    	if (*pSymIndex < 0)
    		goto FoundUnusedSym;
    }
    GlobalUnlock (hSymIndex);	
	hSymIndex = GSSiGlobalReAlloc (0,hSymIndex,(NumSymbols+2)*sizeof(long),GHND);
    pSymIndex = (LPLONG)GlobalLock (hSymIndex);
	pSymIndex += NumSymbols;
	NumSymbols++;   
	isym = NumSymbols;
FoundUnusedSym:	 
	*pSymIndex = SymbolFileEnd; 
	GSSillseek (FidSD,SymbolFileEnd,0);
	CurSymbol->Number=isym;
	if (CurSymbol->Type == 1 && !CurSymbol->GetDimensionsFromSizePoints)
	{  
		CurSymbol->VSize = ldistp (CurSymbol->SizePointV[0],CurSymbol->SizePointV[1]);
		CurSymbol->HSize = ldistp (CurSymbol->SizePointH[0],CurSymbol->SizePointH[1]); 
	}
	lBuf = SymbolToBuffer (CurSymbol,&hBuf,0,0);
	pBuf = GlobalLock (hBuf);   
	BigWrite (FidSD,pBuf,lBuf,-1);
	GSSiGlobUlFree (&hBuf);
	GlobalUnlock (hSymIndex);
    pSymIndex = (LPLONG)GlobalLock (hSymIndex);
    SymbolFileEnd = GSSillseek(FidSD,0,1);
	BigWrite (FidSD,(HPSTR)pSymIndex,NumSymbols*sizeof(long),-1);
	Signature = 28051;
	Version = 1;
	BigWrite (FidSD,(HPSTR)&NumSymbols,2,-1);
    BigWrite (FidSD,(HPSTR)&FirstSymbolNum,2,-1);
    BigWrite (FidSD,(HPSTR)&Signature,2,-1);
    BigWrite (FidSD,(HPSTR)&Version,2,-1);  
	GlobalUnlock (hSymIndex); 
	GlobalUnlock (hCurSymbol);
	return isym;
} 

BOOL CompressSymDict (void)
{   
	short	isym, ii; 
	long	lCurrent, EndLoc,Loc=0, MaxLen=0;
	HANDLE	hSymbol, hBuf;  
	HPSTR	pBuf;
	LPSYMBOL	pCurSym;
	HFILE	Fid=GSSiOpenFile ("[%DL]symdict.tmp",0,OF_CREATE);
	
    CloseSymDict(); 
	OpenSymDict (OF_READ);
	CreateStatusWind (hWndMain,1,"Unload Symbol Dictionary");
    for (isym = 1;isym <= NumSymbols; isym++)
    {   
    	if (isym == 230)
    		ii=1;
		hSymbol = GetDictSymDesc (isym,0);
		if (hSymbol) 
		{
			pCurSym = (LPSYMBOL)GlobalLock (hSymbol);
			lCurrent = SymbolToBuffer (pCurSym,&hBuf,0,0); 
			MaxLen = max (MaxLen,lCurrent);
			BigWrite (Fid,(HPSTR)&isym,2,-1);
			BigWrite (Fid,(HPSTR)&lCurrent,4,-1);
			pBuf = GlobalLock (hBuf);
			BigWrite (Fid,pBuf,lCurrent,-1); 
			GlobalUnlock (hSymbol);
			DestroySymbol (hSymbol);
			GSSiGlobUlFree (&hBuf);
		}
		StatusWindowUpdate (0,0, NumSymbols,isym);
	}
	EndLoc = GSSillseek (Fid,0,2);
	GSSillseek (Fid,0,0);
    CloseSymDict(); 
   	OpenSymDict (OF_DELETE); 
	OpenSymDict (OF_READWRITE);  
	hBuf = GSSiGlobAlloc ( 461,GMEM_MOVEABLE,MaxLen);
	pBuf = GlobalLock (hBuf); 
	StatusWindowUpdate ("Reload Symbol Dictionary",0, EndLoc,0);
	while (Loc < EndLoc)
	{
		BigRead (Fid,(HPSTR)&isym,2);
		BigRead (Fid,(HPSTR)&lCurrent,4);
		BigRead (Fid,pBuf,lCurrent); 
		hSymbol = BufferToSymbol (hBuf);
		pCurSym = (LPSYMBOL)GlobalLock (hSymbol);
		isym = pCurSym->Number;
		GlobalUnlock (hSymbol);
		SaveSymbol (hSymbol,-isym);
		DestroySymbol (hSymbol);
		Loc = GSSillseek (Fid,0,1);
		StatusWindowUpdate (0,0, EndLoc,Loc);
	}
    CloseSymDict(); 
	GSSiGlobUlFree (&hBuf);
	GSSiClose2 (&Fid);
	GSSiRemove ("[%DL]symdict.tmp");
	DestroyStatusWindow(0);  
	return TRUE;
}

float SymBaseScaleMetersPerPixel (int baseScale)
{
	float rtn = baseScale;

	if (baseScale >= 0 && baseScale < 7)
		rtn = (SymbolScales[baseScale] * 12 * FTM)/96;
	return rtn;
}

BOOL OpenSymDict (int Mode)
{   short	Signature, Version, isym,ii; 
	static	short	debugsym=473; 
	LPSYMBOLATTRIBUTE	pSymAtt;
	LPELEMENT pElement;  
	HANDLE	hSymbol;
	LPSYMBOL	pSymbol; 
	HANDLE	hNames=0;
    LPSTR	Name, AtName, NamesName, pDot;
	LPLONG	pSymIndex; 
	RECT	MaxRect;
	BOOL	SaveCP = ContinueProcessing, SaveAVEM = AllVarEqQuestionMark, FirstRect=TRUE;     
	BOOL	rtn = FALSE; 
	LPSTR	pSymNames; 
	HFILE	FidAt, FidNames;
    
    if (FidSD >= 0 && (SymDictOpenMode==Mode || SymDictOpenMode==OF_READWRITE))
        return TRUE; 
    if (Mode == OF_READWRITE && !GetGlobalBVal2 ("[%SYMDICTUPDATEALLOWED]",TRUE))
	{
		MessageBox (0,"You do not have permission to update the symbol dictionary",0,MB_ICONEXCLAMATION);
		return FALSE;
	}
    SetContinueProcessing ( TRUE);
    CloseSymDict();
    *LastSymName=0;                  
    SymDictOpenMode = Mode;
    AllVarEqQuestionMark = FALSE;  
    hNames = GSSiGlobAlloc ( 464,GMEM_MOVEABLE,1024);
    Name = GlobalLock (hNames);
    AtName = Name + 256; 
    NamesName = AtName + 256;
    _fstrcpy (Name,"[%SYM_DICT]");
    ExpandText (Name);           
    AllVarEqQuestionMark = SaveAVEM;   
    _fstrlwr (Name);
    if (_fstrstr (Name,".txt"))
    { 
    	FidSD = GSSiOpenFile (Name,0,OF_READ);   
    	if (FidSD != HFILE_ERROR)
    	{   
    		LPSTR	pBS;
    		
    		_fstrcpy (IconDict,Name);
    		if ((pBS = _fstrrchr (IconDict,'\\'))) 
    		{
    			pBS++;
    			*pBS = 0;
    		}
    		NumSymbols = GSSillseek (FidSD,0,2);
    		rtn = TRUE;
    	}
    	goto Exit;
    }
    if (!*Name)
    	_fstrcpy (Name,"[%DL]symdict.gsd");  
    _fstrcpy (AtName,Name);
    if (!(pDot = _fstrrchr (AtName,'.')))
    	pDot = _fstrchr (AtName,0);
    _fstrcpy (pDot,".att");
    _fstrcpy (NamesName,Name);
    if (!(pDot = _fstrrchr (NamesName,'.')))
    	pDot = _fstrchr (NamesName,0);
    _fstrcpy (pDot,".nms");
    if (Mode == OF_DELETE)
    {
    	GSSiRemove (Name);
    	rtn = TRUE;
    	goto Exit;
    }
Start: 
/*	if (Mode == OF_READ) 
		FidSD = GSSiOpenFileMem (Name);
	else*/ 
		FidSD = GSSiOpenFile (Name,0,Mode);
	if (FidSD == HFILE_ERROR) 
	{    
		if (Mode == OF_READWRITE)
		{   
			NumSymbols = 0;
			FirstSymbolNum = 1;
			Signature = 28051;
			Version = 1;
			FidSD = GSSiOpenFile (Name,0,OF_CREATE_NODELETE);
			BigWrite (FidSD,(HPSTR)&NumSymbols,2,-1);
		    BigWrite (FidSD,(HPSTR)&FirstSymbolNum,2,-1);
		    BigWrite (FidSD,(HPSTR)&Signature,2,-1);
		    BigWrite (FidSD,(HPSTR)&Version,2,-1);  
		    GSSiClose2 (&FidSD);
			goto Start;
		} 
		HaltMapDisplay(FALSE,TRUE);
		if (GSSiMsgBox( GetFocus(), "Symbol file not found",Name,
		    MB_OKCANCEL|MB_ICONEXCLAMATION,0) == IDCANCEL)
			BlowOut(0,0);
    	goto Exit;
	} 
	
    SymbolFileEnd = GSSillseek(FidSD,(LONG)-(8),2);

    GSSilread (FidSD,&NumSymbols,2);
    GSSilread (FidSD,&FirstSymbolNum,2);
    GSSilread (FidSD,&Signature,2);
    GSSilread (FidSD,&Version,2);
    if (Signature != 28051)
    {	GSSiClose2 (&FidSD);
		GSSiMsgBox( GetFocus(), "This is not a valid symbol file",Name, MB_OK,0);
    	goto Exit;
    }
    if (Version > 1)
    {	GSSiClose2 (&FidSD);
		GSSiMsgBox( GetFocus(), "This symbol file version is not recognized",Name, MB_OK,0);
    	goto Exit;
    }
    SymDictVersion = Version;
    if (NumSymbols)
    {   
    	hSymIndex = GSSiGlobAlloc ( 465,GMEM_MOVEABLE,NumSymbols*sizeof(long));
    	pSymIndex = (LPLONG)GlobalLock (hSymIndex); 
    	SymbolFileEnd = SymbolFileEnd-NumSymbols*sizeof(long);
	    GSSillseek(FidSD,SymbolFileEnd,0);
	    GSSilread (FidSD,pSymIndex,NumSymbols*sizeof(long));
	    GlobalUnlock (hSymIndex);
    } 
	SetContinueProcessing ( SaveCP);  
	if (NumSymbols && !hSymbolAttributes)
	{
		hSymbolAttributes = GSSiGlobAlloc ( 466,GHND,NumSymbols*sizeof(SYMBOLATTRIBUTE));
		pSymAtt = (LPSYMBOLATTRIBUTE)GlobalLock (hSymbolAttributes); 
		hSymNames = GSSiGlobAlloc ( 467,GHND,USHRT_MAX);
		pSymNames = GlobalLock (hSymNames); 
		pSymIndex = (LPLONG)GlobalLock (hSymIndex); 
		FidNames = GSSiOpenFile (NamesName,0,OF_READ); 
		if (FidNames == HFILE_ERROR)
			GSSiRemove (AtName);
		else
			GSSiClose2 (&FidNames); 
		if (GSSiLength (AtName) != NumSymbols*sizeof(SYMBOLATTRIBUTE))
			GSSiRemove (AtName);
		FidAt = GSSiOpenFile (AtName,0,OF_READ);
		if (FidAt == HFILE_ERROR)
		{
			BTVARDESC	BTVar[2]; 
			char		TempName[MAX_PATH], Name[66]; 
			HANDLE		hNamesBT,hNames2;
			short		pos = BT_FIRST;
			long		lNames=0;
			
			BTVar[0].BT_VARLEN=40;
			BTVar[0].BT_VARTYP=BT_CHAR;
			BTVar[0].BT_VAROFF=0;
			GSSiGetTempFileName (0,"gmu",0,(LPSTR)TempName);
			BT_CREATE (TempName, 2, FALSE, 1, 1,BTVar,FALSE, 0, 0, FALSE);
			hNamesBT = BT_OPEN (TempName,0, BT_WRITE, 0);
			for (isym=1;isym<=NumSymbols;isym++)
			{
				pSymAtt[isym-1].Type = -1;
				if ((hSymbol = GetDictSymDesc (isym,0)))
				{
					pSymbol = (LPSYMBOL)GlobalLock (hSymbol);
					pSymAtt[isym-1].Type = pSymbol->Type;
					pSymAtt[isym-1].InVisible = pSymbol->InVisible;   
					pSymAtt[isym-1].DisplayPos = pSymbol->DisplayPos;   
					pSymAtt[isym-1].Layered = pSymbol->Layered;   
					if (isym == debugsym)
						ii=1; 
					if (pSymIndex[isym-1] >= 0)
						BT_PUT (hNamesBT,pSymbol->Name,(LPSTR)&isym);
					switch (pSymbol->Type)
					{   
						case 1://point
						{
							POINT	Point={0,0};
							short	ii; 
							LPVISLIST	SaveVis = CurVis;
							RECT	Rect;
							
							CurVis = 0;
							RectInit (&Rect); 
							DisplayPointSymbol (hSymbol,0, 0, 200,0, &Point,&Rect,FALSE,0,0,FALSE,FALSE,0,0);   
							pSymAtt[isym-1].Rect = Rect32ToRect16 (Rect);
							if (pSymbol->BaseScale > 5)
							{
								pSymAtt[isym-1].WidthIsMeters = 0;
								pSymAtt[isym-1].Width = RECTWIDTH(&pSymAtt[isym-1].Rect);
							}
							else
							{
								pSymAtt[isym-1].WidthIsMeters = 1;
								pSymAtt[isym-1].Width = RECTWIDTH(&pSymAtt[isym-1].Rect);
								pSymAtt[isym-1].Width *= SymBaseScaleMetersPerPixel (pSymbol->BaseScale);
							}
							if (FirstRect)  
							{
								MaxRect = Rect16ToRect32 (pSymAtt[isym-1].Rect);
								FirstRect = FALSE;
							} 
							else
							{
								RECT	Rect2=Rect16ToRect32 (pSymAtt[isym-1].Rect);
								UnionRect (&MaxRect,&MaxRect,&Rect2);
							}
							CurVis = SaveVis;
							ii=1;
						}  
						break;
						
						case 2://line
						if (pSymbol->BlockRotation)
							pSymAtt[isym-1].Reversed = TRUE;
						else
							pSymAtt[isym-1].Reversed = FALSE;
						if (pSymbol->NumElements == 1)
						{
							pElement = (LPELEMENT)GlobalLock (pSymbol->hElement);  
							if (pElement->Width < 0)
							{
								pSymAtt[isym-1].WidthIsMeters = TRUE;
								pSymAtt[isym-1].Width = FTM * -pElement->Width; 
							}  
							else
							{
								pSymAtt[isym-1].WidthIsMeters = FALSE;
								pSymAtt[isym-1].Width = pElement->Width; 
							}  
							pSymAtt[isym-1].Color = pElement->LineColor;   
							pSymAtt[isym-1].Shadow = pElement->Shadow;   
							if (pElement->NumVectors < 3)
								pSymAtt[isym-1].SolidLine = TRUE;
							GlobalUnlock (pSymbol->hElement);
						} 
						else if (pSymbol->NumElements) 
						{
							short	i;
							HANDLE	*phElement = &pSymbol->hElement;
							
							for (i=0;i<pSymbol->NumElements;i++,phElement++)  
							{
								pElement = (LPELEMENT)GlobalLock (*phElement);  
								if (pElement->LineColorType == SVVARCOLOR)
								{
									if (pElement->Width < 0)
									{
										pSymAtt[isym-1].WidthIsMeters = TRUE;
										pSymAtt[isym-1].Width = FTM * -pElement->Width; 
									}  
									else if (pSymbol->BaseScale == 15)
									{
										pSymAtt[isym-1].WidthIsMeters = TRUE;
										pSymAtt[isym-1].Width = ((pSymbol->VSize * pElement->Width)/100) * FTM; 
									}  
									else
									{
										pSymAtt[isym-1].WidthIsMeters = FALSE;
										pSymAtt[isym-1].Width = pElement->Width; 
									}  
									pSymAtt[isym-1].Color = pElement->LineColor;
									GlobalUnlock (*phElement);
									break;
								}
								GlobalUnlock (*phElement);
							}
						}   
						if (!pSymbol->NumElements || !pSymbol->HSize || !pSymbol->VSize)
							pSymAtt[isym-1].SolidLine = TRUE;  
						break; 
					} 
					GlobalUnlock (hSymbol);
					DestroySymbol (hSymbol);
				}
			} 
			FidAt = GSSiOpenFile (AtName,0,OF_CREATE);
			BigWrite (FidAt,(HPSTR)pSymAtt,NumSymbols*sizeof(SYMBOLATTRIBUTE),-1);
			while (!BT_FIND (hNamesBT,Name,pos,BT_ANY,(LPSTR)&isym)) 
			{   
				pos = BT_NEXT;
				if (*Name)
				{
					_fstrcpy (&pSymNames[lNames],Name);
					lNames += _fstrlen (Name) + 1;
					_fmemmove (&pSymNames[lNames],&isym,2);
					lNames += 2; 
				}
			} 
			lNames++;
			BT_CLOSEANDDELETE (&hNamesBT); 
			FidNames = GSSiOpenFile (NamesName,0,OF_CREATE);
			BigWrite (FidNames,(HPSTR)pSymNames,(size_t)lNames,-1);
		}
		else
		{ 
			BigRead (FidAt,(HPSTR)pSymAtt,NumSymbols*sizeof(SYMBOLATTRIBUTE));
			FidNames = GSSiOpenFile (NamesName,0,OF_READ); 
			BigRead (FidNames,pSymNames,USHRT_MAX);
		}
		GSSiClose2 (&FidAt);
		GSSiClose2 (&FidNames);
		GlobalUnlock (hSymbolAttributes);
		GlobalUnlock (hSymNames);  
	    GlobalUnlock (hSymIndex);
	}  
    DefaultTextPointer= GetDictSymbolNumber ("%TEXTPNTR0"); 
    TXPntrSym[0]= GetDictSymbolNumber ("%TEXTPNTR1");
    TXPntrSym[1]= GetDictSymbolNumber ("%TEXTPNTR2");
	InvisiblePointSymbol = GetDictSymbolNumber ("INVISPNT");     
	if (SymDictOpenMode != OF_READ)
		GSSiRemove (AtName);	
	rtn = TRUE;   
Exit:
	memset(cachedSymbolHandle, 0, sizeof(cachedSymbolHandle));
	GSSiGlobUlFree (&hNames);
	return rtn;
} 

RECT	GetSymRect (int idesc)
{   
	RECT	SymRect={-100,-100,100,100};
	LPSYMBOLATTRIBUTE	pSymAtt;
	
	if (!OpenSymDict (OF_READ))
		return SymRect;  
	if (hSymbolAttributes && idesc && idesc <= NumSymbols)
	{
		pSymAtt = (LPSYMBOLATTRIBUTE)GlobalLock (hSymbolAttributes);
		if (pSymAtt[idesc-1].Type == 1 && !pSymAtt[idesc-1].InVisible)
			SymRect = Rect16ToRect32(pSymAtt[idesc-1].Rect);
		GlobalUnlock (hSymbolAttributes);
	}
	return SymRect;
}

HANDLE AllocateNewElement (void)
{
	return GSSiGlobAlloc ( 468,GHND,sizeof(ELEMENT) + (long)sizeof(VECTOR) * MAX_ELEMENT_VECTORS);  
}
 
HANDLE AllocateNewSymbol (void)
{
	return GSSiGlobAlloc ( 469,GHND,sizeof(SYMBOL)+MAX_SYMBOL_ELEMENTS*sizeof(HANDLE));
}
    
int CurvePoints (LPDPOINT PC, LPDPOINT POC, LPDPOINT PT, LPLONG nPnts, HPPOINT *Points,double CurveExpansionFactor,long MaxPoints)
{

	DPOINT	RP, Newpt;
	double	CLEN, Circum, radius, AZ, azperpt; 
	long	nump=2, np, st;
	
	st = RCURVE(&PC->x,&PC->y,&POC->x,&POC->y,&PT->x,&PT->y,&RP.x,&RP.y,&CLEN); 
	if (st == 2)
	{             
		if (ldistp (*PC,*PT) < 0.0001)   
		{
			st = 0;
			RP = MidPointD (*PC,*POC);  
			radius = ldistp (*PC,RP);
			CLEN = 2 * PY * radius;
		}
	}
	(*Points)->x = IDNINT (PC->x);
	(*Points)++->y = IDNINT (PC->y);
	(*nPnts)++; 
	
	if (!st)
	{
		np = IDNINT (fabs(CLEN/2)) - 1;  
		np = max (np,0);       
		np *= CurveExpansionFactor; 
		if (nump+np > MaxPoints)
			np = MaxPoints - nump;
		nump+=np;
		if (np)
		{
			radius = ldistp (*PC,RP);
			Circum = 2 * PY * radius;
			azperpt = ((-CLEN / Circum) * TWOPI) / np;
			AZ = getazd (&RP,PC);
			(*nPnts) += np;
			while (np--)
			{   
				AZ += azperpt;
				Newpt = dnewpt (RP,AZ,radius);
				(*Points)->x = max((long)INT_MIN,min((long)INT_MAX,IDNINT (Newpt.x)));
				(*Points)++->y = max((long)INT_MIN,min((long)INT_MAX,IDNINT (Newpt.y)));
			}
		} 
	}
	(*Points)->x = IDNINT (PT->x);
	(*Points)++->y = IDNINT (PT->y);
	(*nPnts)++;
	return (nump);
}	

int CurvePointsS (LPDPOINT PC, LPDPOINT POC, LPDPOINT PT, LPLONG nPnts, HPPOINTS *Points,double CurveExpansionFactor,long MaxPoints)
{

	DPOINT	RP, Newpt;
	double	CLEN, Circum, radius, AZ, azperpt; 
	long	nump=2, np, st;
	
	st = RCURVE(&PC->x,&PC->y,&POC->x,&POC->y,&PT->x,&PT->y,&RP.x,&RP.y,&CLEN); 
	if (st == 2)
	{             
		if (ldistp (*PC,*PT) < 0.0001)   
		{
			st = 0;
			RP = MidPointD (*PC,*POC);  
			radius = ldistp (*PC,RP);
			CLEN = 2 * PY * radius;
		}
	}
	(*Points)->x = IDNINT (PC->x);
	(*Points)++->y = IDNINT (PC->y);
	(*nPnts)++; 
	
	if (!st)
	{
		np = IDNINT (fabs(CLEN/2)) - 1;  
		np = max (np,0);       
		np *= CurveExpansionFactor; 
		if (nump+np > MaxPoints)
			np = MaxPoints - nump;
		nump+=np;
		if (np)
		{
			radius = ldistp (*PC,RP);
			Circum = 2 * PY * radius;
			azperpt = ((-CLEN / Circum) * TWOPI) / np;
			AZ = getazd (&RP,PC);
			(*nPnts) += np;
			while (np--)
			{   
				AZ += azperpt;
				Newpt = dnewpt (RP,AZ,radius);
				(*Points)->x = max((long)SHRT_MIN,min((long)SHRT_MAX,IDNINT (Newpt.x)));
				(*Points)++->y = max((long)SHRT_MIN,min((long)SHRT_MAX,IDNINT (Newpt.y)));
			}
		} 
	}
	(*Points)->x = IDNINT (PT->x);
	(*Points)++->y = IDNINT (PT->y);
	(*nPnts)++;
	return (nump);
}	

int CurvePointsD (LPDPOINT PC, LPDPOINT POC, LPDPOINT PT, LPLONG nPnts, HPDPOINT *Points,LPDOUBLE pBackAZ,long MaxPoints,double VectorizationFactor,short LoopFactorIN)
#if ENABLETRACE
{GSSiEnterProg (1386);
#endif
{   
	short LoopFactor = abs(LoopFactorIN);
	DPOINT	RP, Newpt;   
	long	np4;
	double	CLEN, Circum, radius, AZ, azperpt; 
	int		st;
	long	np;
	BOOL	addPT = TRUE;
	int nPntsIN = *nPnts;
	if (LoopFactorIN < 0)
		addPT = FALSE;
	if (MaxPoints <= 0)
	{
		goto Exit;
	}
	st = RCURVE(&PC->x,&PC->y,&POC->x,&POC->y,&PT->x,&PT->y,&RP.x,&RP.y,&CLEN); 
	
	*(*Points)++ = *PC;
	(*nPnts)++; 
	if (!VectorizationFactor)
		VectorizationFactor = BaseDistToWinDist/2;
	else if (VectorizationFactor < 0)
		VectorizationFactor = -BaseDistToWinDist/VectorizationFactor;  
	VectorizationFactor /= LoopFactor;
	if (!st)
	{   
		*pBackAZ = getazd (&RP,PT);
		if (CLEN < 0)
			*pBackAZ = LTWOPI (*pBackAZ + HALFPI);
		else	
			*pBackAZ = LTWOPI (*pBackAZ - HALFPI);
//		np4 = IDNINT (VectorizationFactor*fabs(CLEN/2)) - 1;  
		np4 = IDNINT(fabs(CLEN / 2)/VectorizationFactor) - 1;
		np = max(np4, 0);
		if (np >= MaxPoints)
			np = MaxPoints;
		if (np)
		{
			radius = ldistp (*PC,RP);
			Circum = 2 * PY * radius;
			azperpt = ((-CLEN / Circum) * TWOPI) / np;
			AZ = getazd (&RP,PC); 
			np--;
			(*nPnts) += np;
			while (np--)
			{   
				AZ += azperpt;
				Newpt = dnewpt (RP,AZ,radius);
				*(*Points)++ = Newpt;
			}
		}
		else
		{
			*(*Points)++ = *POC;
			(*nPnts)++;
		} 
	}
	if (addPT)
	{
		*(*Points)++ = *PT;
		(*nPnts)++;
	}
Exit:;
	int npnts = *nPnts - nPntsIN;
{
#if ENABLETRACE
GSSiExitProg (1386);
#endif
	return (npnts);
}
#if ENABLETRACE
}
#endif
}

void SetSizePointsFromBounds (LPSYMBOL CurSymbol,LPMNMXCORD pBounds)
{
	CurSymbol->SizePointV[0] = 
	CurSymbol->SizePointV[1] = 
	CurSymbol->SizePointH[0] = 
	CurSymbol->SizePointH[1] = MinMaxMidPointD (pBounds);
	
	CurSymbol->SizePointV[0].y = pBounds->ymn;
	CurSymbol->SizePointV[1].y = pBounds->ymx;
	CurSymbol->SizePointH[0].x = pBounds->xmn;
	CurSymbol->SizePointH[1].x = pBounds->xmx; 
	return;
}

short GetUMSymbolPar (HFILE Fid, LPSTR SymName)
{
	HANDLE	hSym;
	LPSYMBOL	CurSymbol; 
	LPSTR	lpColon;
	short parnum;  
	char	str[70], Snam[16], Pnam[16];

	GSSillseek (Fid,0,0);
	while (fgetstring (str,64,Fid))
	{
		_fstrncpy (Snam,&str[1],8); 
		Snam[8] = 0;
		Truncate (Snam);
		lpColon = _fstrchr (&str[10],':');
		if (lpColon)
		{
			lpColon += 9;
			_fstrncpy (Pnam,lpColon,8); 
			Pnam[8] = 0;
			Truncate (Pnam);
			if (!_fstricmp (SymName,Snam))
				goto GotPar;
		}
	}                   
	_fstrcpy (Pnam,"ALL");
GotPar:
	parnum=GetDictSymbolNumber(Pnam);
	if (parnum)
		return parnum;		 
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock(hSym);
	_fstrcpy (CurSymbol->Name,Pnam);
	_fstrcpy (CurSymbol->Desc,"Imported from UltiMap");
	CurSymbol->Parent=GetUMSymbolPar (Fid, Pnam);	
	GlobalUnlock (hSym);	 
	parnum = SaveSymbol(hSym,0);   
	DestroySymbol (hSym);
		
	return parnum;
}

void NODE (short JNODE,LPDOUBLE X,LPDOUBLE Y)
{
      short INODE=abs(JNODE);
      short IY=INODE%151;
      short IX=INODE/151;
      
      if(IY) goto S50;
      IY=151;
      IX=IX-1;
S50:
      *X=IX/50.0;
      *Y=(IY-1)/50.0;
      return;

}

short JATTR (short NUMAT,short ISTART,short LENGTH) 
{
	short iJATTR, LL, SymNum;
    char  RATT[4]={0,0,0,0}, BTT[2];
    long  ILENGTH, MATTR, NNN;  
    LPLONG	pNNN;
	iJATTR = 0;
	LL=ISTART+LENGTH-1 ;
	_fmemmove (BTT,&NUMAT,2);
	pNNN = (LPLONG)RATT; 
	RATT[1] = BTT[0];
	RATT[0] = BTT[1];
	NNN = *pNNN;
	MATTR = NNN >> 16-LL;
	ILENGTH = (1 << LENGTH) - 1;
	iJATTR = MATTR & ILENGTH;
	return iJATTR;
}

short GetUMSymPoly (LPSTR Snam,short Snum,short View,double OFF,double Scale,double Rot,
					LPSHORT pnPoly,LPSHORT plPoly,LPHANDLE phPoly,LPSHORT PolyPen,HFILE Fid,HPDPOINT points,LPSHORT pnp,
					short MaxPoly)   
{ 
	char	snam[10];
	HANDLE	hStr=GSSiGlobAlloc ( 470,GMEM_MOVEABLE,USHRT_MAX);
	LPSTR	str=GlobalLock (hStr); 
	short	numc, loc, ipen;  
	HPDPOINT	pPoints; 
	double	Xoff=1.0, Yoff=1.0;
	
	
    GSSillseek (Fid,0,0); 
    _fstrcpy (snam,Snam);
    PadString(snam,' ',8);
    while (fgetstring (str,32000,Fid))
    { 
   		if (!_fstrnicmp (&str[1],snam,8))
   		{   
   			if (*str == 'P')
   				Xoff = Yoff = 5.0;
   			while (fgetstring (str,32000,Fid))
   			{
   				switch (*str)
   				{
   					case 'P': 
   					case 'L':
   						goto Exit;
   					case 'B': 
		         		plPoly[*pnPoly] = -1;  
		         		(*pnPoly)++;
   						break;
   					case 'V': 
		         		plPoly[*pnPoly] = -2;  
		         		(*pnPoly)++;
   						break;
   					case 'E': 
		         		plPoly[*pnPoly] = -3;  
		         		(*pnPoly)++;
   						break;
   					case 'C': 
   						ipen = ldread (&str[1],6);
   						numc = min (330,ldread (&str[7],6));
   						loc = 13;
   						if (ipen > 0)
   						{   
   							PolyPen[*pnPoly] = ipen;
			         		plPoly[*pnPoly] = numc;
			         		phPoly[*pnPoly] = GSSiGlobAlloc ( 471,GMEM_MOVEABLE,numc*sizeof(DPOINT));
			         		pPoints = (HPDPOINT)GlobalLock (phPoly[*pnPoly]);
			         		while (numc--)
			         		{
			         			pPoints->x = (double)ldread (&str[loc],6)/1000.0 - Xoff;
			         			loc += 6;
			         			pPoints++->y = (double)ldread (&str[loc],6)/1000.0 - Yoff;
			         			loc += 6; 
			         		}
			         		GlobalUnlock (phPoly[*pnPoly]);     
			         		(*pnPoly)++;
			         		if (*pnPoly >= MaxPoly)
			         			goto Exit;
			         	}
			         	else
			         	{
			         		numc /= 2;
			         		while (numc--)
			         		{
	   							PolyPen[*pnPoly] = -ipen;
				         		plPoly[*pnPoly] = 2;
				         		phPoly[*pnPoly] = GSSiGlobAlloc ( 472,GMEM_MOVEABLE,2*sizeof(DPOINT));
				         		pPoints = (HPDPOINT)GlobalLock (phPoly[*pnPoly]);   
			         			pPoints->x = (double)ldread (&str[loc],6)/1000.0 - Xoff;
			         			loc += 6;
			         			pPoints++->y = (double)ldread (&str[loc],6)/1000.0 - Yoff;
			         			loc += 6; 
			         			pPoints->x = (double)ldread (&str[loc],6)/1000.0 - Xoff;
			         			loc += 6;
			         			pPoints->y = (double)ldread (&str[loc],6)/1000.0 - Yoff;
			         			loc += 6; 
				         		GlobalUnlock (phPoly[*pnPoly]);     
				         		(*pnPoly)++;   
				         		if (*pnPoly >= MaxPoly)
				         			goto Exit;
			         		}
			         	}
			         	break;
   				}
   			}
   		}
   	}
Exit:
	GSSiGlobUlFree (&hStr);
   	return 0;
	
}
void GetUMSymDump (LPSTR Snam,HANDLE hSym,HFILE Fid)
{  
#define	MAXPOLY	500        
	LPSYMBOL	CurSymbol;
	LPELEMENT pElement;  
	LPVECTOR	pVector;   
	HANDLE	*phElement;
	DPOINT	Point1, Point2; 
	LPDPOINT	pPoint;
	MNMXCORD Bounds; 
	short	n, rec[200], lPoly[MAXPOLY], nPoly=0, ipoly,i,np=0, PolyPen[MAXPOLY];
	HANDLE	hPoly[MAXPOLY];
	DPOINT	points[MAXPOLY]; 
	PATBYTE	PatByte; 
	BYTE	W;
	
	GetUMSymPoly (Snam,0,1,0,1,0,&nPoly,lPoly,hPoly,PolyPen,Fid,points,&np,MAXPOLY); 
	if (!nPoly)
		return;
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	CurSymbol->NumElements=nPoly; 
	phElement = &CurSymbol->hElement;
	CurSymbol->TiePoint.x = CurSymbol->TiePoint.y = 0;
	for (ipoly=0;ipoly<nPoly;ipoly++,phElement++)
	{
		*phElement=GSSiGlobAlloc ( 473,GHND,sizeof(ELEMENT)+max(0,(lPoly[ipoly]-1))*sizeof(VECTOR));
		pElement = (LPELEMENT)GlobalLock (*phElement);
		pElement->NumVectors = max(0,lPoly[ipoly]); 
		if (lPoly[ipoly] > 0)
		{   
			PatByte.Pattern = 2;
			PatByte.Transparent = 1;  
			PatByte.BGOpt = 0;
			_fmemmove (&W,&PatByte,1);
		
			pElement->FillColor = RGBW(255,255,255,W);  
			pElement->LineColor = -PolyPen[ipoly]; 
			pElement->FillColorType = SVVARCOLOR; 
			pElement->Width = PolyPen[ipoly];
			pElement->Type = 2;
			pElement->Style = 0;
			pVector = &pElement->Vector;
			pPoint = (LPDPOINT)GlobalLock (hPoly[ipoly]); 
			if (pElement->NumVectors > 1 && !ldistp (*pPoint,pPoint[pElement->NumVectors-1]))
				pElement->Type = 3;
			for (i=0;i<pElement->NumVectors;i++,pVector++)
			{   
				Point1 = CurSymbol->TiePoint;
	//			AddDPointToMinMax (&Point2,&Bounds);  
				if (CurSymbol->Type == 2)
				{
					pVector->Dist = pPoint->x;    
					pVector->AZM = pPoint++->y; 
				}
				else
				{
					pVector->Dist = ldistp (Point1,*pPoint);    
					pVector->AZM = getazd (&Point1,pPoint++); 
				}
				pVector->Type = 1; 
			} 
			GSSiGlobUlFree (&hPoly[ipoly]); 
		}
		else
			pElement->Type = lPoly[ipoly];
		GlobalUnlock (*phElement); 
	}
	Bounds.xmn = Bounds.ymn = 0;
	Bounds.xmx = Bounds.ymx = 0.3;
	SetSizePointsFromBounds (CurSymbol,&Bounds);   
	GlobalUnlock (hSym);
	return;
}


void createsymbols (short type)
{   
	int	NumVectors, i;  
	HANDLE	hSym;
	LPSYMBOL	CurSymbol; 
	LPELEMENT pElement;  
	LPVECTOR	pVector;   
	HANDLE	*phElement;
	DPOINT	Point1, Point2;
	MNMXCORD Bounds;
	int		x1[19]={0,-6,-12,-13,-12,-11,-13,-8,-4,0,4,8,13,11,12,13,12,6,0};
	int		y1[19]={-15,-9,-7,-5,-3,3,10,15,12,15,12,15,10,3,-3,-5,-7,-9,-15};
	int		type1[19]={1,2,3,2,3,2,3,3,2,3,2,3,3,2,3,2,3,2,1};
	int		x2[8]={0,-10,-14,-12,12,14,10,0};
	int		y2[8]={-15,-9,-1,9,9,-1,-9,-15};
	int		type2[8]={1,2,1,1,1,1,2,1};
	int		x22[8]={-12,-8,-4,0,4,8,12,-12};
	int		y22[8]={9,15,13,15,13,15,9,9};
	int		type22[8]={1,3,2,3,2,3,3,1};
	int		x3[5]={-15,-15,15,15,-15};
	int		y3[5]={-15,15,15,-15,-15};
	int		type3[5]={1,1,1,1,1};
	int		x4[5]={0,-15,0,15,0};
	int		y4[5]={-15,0,15,0,-15};
	int		type4[5]={1,2,2,2,2};
	int		x5[5]={0,0,0,-15,15};
	int		y5[5]={-15,15,0,0,0};
	int		type5[5]={1,1,1,1,1};
	
	OpenSymDict (OF_READWRITE);     
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock(hSym);
	_fstrcpy (CurSymbol->Name,"ALL");
	_fstrcpy (CurSymbol->Desc,"Parent of all other symbols");
	CurSymbol->Parent=0;	
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	DestroySymbol (hSym);
	
	if (type == 2)
		goto SkipStdSyms;
			
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock(hSym);
	_fstrcpy (CurSymbol->Name,"SHIELDS");
	_fstrcpy (CurSymbol->Desc,"Hiway Shields");
	CurSymbol->Parent=GetDictSymbolNumber("ALL");	
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	DestroySymbol (hSym);
		
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	_fstrcpy (CurSymbol->Name,"CENSUSAR");
	_fstrcpy (CurSymbol->Desc,"Census areas");
	CurSymbol->Parent=GetDictSymbolNumber("ALL");		 
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	DestroySymbol (hSym);
		
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	_fstrcpy (CurSymbol->Name,"COUNTY");
	_fstrcpy (CurSymbol->Desc,"County area");
	CurSymbol->Type = 3;
	CurSymbol->NumElements=0;
	CurSymbol->Parent=GetDictSymbolNumber("CENSUSAR");		 
	NumVectors = 0;
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	DestroySymbol (hSym);
		
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	_fstrcpy (CurSymbol->Name,"COUNTYBL");
	_fstrcpy (CurSymbol->Desc,"County boundary line");
	CurSymbol->Type = 2;
	CurSymbol->NumElements=1;
	CurSymbol->Parent=GetDictSymbolNumber("CENSUSAR");		 
	NumVectors = 0;
	CurSymbol->hElement=GSSiGlobAlloc ( 474,GHND,sizeof(ELEMENT)+(NumVectors-1)*sizeof(VECTOR));
	pElement = (LPELEMENT)GlobalLock (CurSymbol->hElement);
	pElement->NumVectors = NumVectors;
	pElement->FillColor = 0;  
	pElement->LineColor = 0;  
	GlobalUnlock (CurSymbol->hElement); 
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	DestroySymbol (hSym);
		
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	_fstrcpy (CurSymbol->Name,"CENTRACT");
	_fstrcpy (CurSymbol->Desc,"Census tract area");
	CurSymbol->Type = 3;
	CurSymbol->NumElements=0;
	CurSymbol->Parent=GetDictSymbolNumber("CENSUSAR");		 
	NumVectors = 0;
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	DestroySymbol (hSym);
		
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	_fstrcpy (CurSymbol->Name,"CENTRABL");
	_fstrcpy (CurSymbol->Desc,"County boundary line");
	CurSymbol->Type = 2;
	CurSymbol->NumElements=1;
	CurSymbol->Parent=GetDictSymbolNumber("CENSUSAR");		 
	NumVectors = 0;
	CurSymbol->hElement=GSSiGlobAlloc ( 475,GHND,sizeof(ELEMENT)+(NumVectors-1)*sizeof(VECTOR));
	pElement = (LPELEMENT)GlobalLock (CurSymbol->hElement);
	pElement->NumVectors = NumVectors;
	pElement->FillColor = 0;  
	pElement->LineColor = 0;  
	GlobalUnlock (CurSymbol->hElement); 
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	DestroySymbol (hSym);
		
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	_fstrcpy (CurSymbol->Name,"BLOCKGRP");
	_fstrcpy (CurSymbol->Desc,"Census Block Group");
	CurSymbol->Type = 3;
	CurSymbol->NumElements=0;
	CurSymbol->Parent=GetDictSymbolNumber("CENSUSAR");		 
	NumVectors = 0;
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	DestroySymbol (hSym);
		
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	_fstrcpy (CurSymbol->Name,"BLOCKGBL");
	_fstrcpy (CurSymbol->Desc,"County boundary line");
	CurSymbol->Type = 2;
	CurSymbol->NumElements=1;
	CurSymbol->Parent=GetDictSymbolNumber("CENSUSAR");		 
	NumVectors = 0;
	CurSymbol->hElement=GSSiGlobAlloc ( 476,GHND,sizeof(ELEMENT)+(NumVectors-1)*sizeof(VECTOR));
	pElement = (LPELEMENT)GlobalLock (CurSymbol->hElement);
	pElement->NumVectors = NumVectors;
	pElement->FillColor = 0;  
	pElement->LineColor = 0;  
	GlobalUnlock (CurSymbol->hElement); 
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	DestroySymbol (hSym);
		
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	_fstrcpy (CurSymbol->Name,"CENBLOCK");
	_fstrcpy (CurSymbol->Desc,"Census Block");
	CurSymbol->Type = 3;
	CurSymbol->NumElements=0;
	CurSymbol->Parent=GetDictSymbolNumber("CENSUSAR");		 
	NumVectors = 0;
	CurSymbol->hElement=GSSiGlobAlloc ( 477,GHND,sizeof(ELEMENT)+(NumVectors-1)*sizeof(VECTOR));
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	DestroySymbol (hSym);
		
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	_fstrcpy (CurSymbol->Name,"CENPLACE");
	_fstrcpy (CurSymbol->Desc,"Place");
	CurSymbol->Type = 3;
	CurSymbol->NumElements=1;
	CurSymbol->Parent=GetDictSymbolNumber("CENSUSAR");		 
	NumVectors = 0;
	CurSymbol->hElement=GSSiGlobAlloc ( 478,GHND,sizeof(ELEMENT)+(NumVectors-1)*sizeof(VECTOR));
	pElement = (LPELEMENT)GlobalLock (CurSymbol->hElement);
	pElement->NumVectors = NumVectors;
	pElement->FillColor = -100;  
	pElement->LineColor = 0;  
	GlobalUnlock (CurSymbol->hElement); 
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	DestroySymbol (hSym);
		
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	_fstrcpy (CurSymbol->Name,"CENBLOBL");
	_fstrcpy (CurSymbol->Desc,"County boundary line");
	CurSymbol->Type = 2;
	CurSymbol->NumElements=1;
	CurSymbol->Parent=GetDictSymbolNumber("CENSUSAR");		 
	NumVectors = 0;
	CurSymbol->hElement=GSSiGlobAlloc ( 479,GHND,sizeof(ELEMENT)+(NumVectors-1)*sizeof(VECTOR));
	pElement = (LPELEMENT)GlobalLock (CurSymbol->hElement);
	pElement->NumVectors = NumVectors;
	pElement->FillColor = 0;  
	pElement->LineColor = 0;  
	GlobalUnlock (CurSymbol->hElement); 
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	DestroySymbol (hSym);
		
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	_fstrcpy (CurSymbol->Name,"MISC");
	_fstrcpy (CurSymbol->Desc,"Miscellaneous symbols");
	CurSymbol->Type = 0;
	CurSymbol->NumElements=0;
	CurSymbol->Parent=GetDictSymbolNumber("ALL");		 
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	DestroySymbol (hSym);
		
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	_fstrcpy (CurSymbol->Name,"USSHLD");
	_fstrcpy (CurSymbol->Desc,"US Highway Shield");
	CurSymbol->Parent=GetDictSymbolNumber("SHIELDS");
	CurSymbol->NumElements=1; 
	CurSymbol->Type=1;
	CurSymbol->TiePoint.x = 0;
	CurSymbol->TiePoint.y = 0;
	
	NumVectors = 19; 
	DBoundsInit (&Bounds);
	CurSymbol->hElement=GSSiGlobAlloc ( 480,GHND,sizeof(ELEMENT)+(NumVectors-1)*sizeof(VECTOR));
	pElement = (LPELEMENT)GlobalLock (CurSymbol->hElement);
	pElement->NumVectors = NumVectors;
	pElement->FillColor = RGB(255,255,255);  
	pElement->LineColor = -3;  
	pElement->Width = 0;
	pElement->Type = 3;
	pElement->Style = 0;
	pVector = &pElement->Vector; 
	for (i=0;i<pElement->NumVectors;i++,pVector++)
	{   
		Point1 = CurSymbol->TiePoint;
		Point2.x = x1[i];
		Point2.y = y1[i];
		AddDPointToMinMax (&Point2,&Bounds);
		pVector->Dist = ldistp (Point1,Point2);    
		pVector->AZM = getazd (&Point1,&Point2);
		pVector->Type = type1[i]; 
	}
    SetSizePointsFromBounds (CurSymbol,&Bounds);
    GlobalUnlock (CurSymbol->hElement);
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	DestroySymbol (hSym);
		
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	_fstrcpy (CurSymbol->Name,"INSTSHLD");
	_fstrcpy (CurSymbol->Desc,"Interstate Highway Shield");
	CurSymbol->Parent=GetDictSymbolNumber("SHIELDS");
	CurSymbol->NumElements=2; 
	CurSymbol->Type=1;
	CurSymbol->TiePoint.x = 0;
	CurSymbol->TiePoint.y = 0;
	
	NumVectors = 8; 
	DBoundsInit (&Bounds);
	phElement = &CurSymbol->hElement;
	*phElement=GSSiGlobAlloc ( 481,GHND,sizeof(ELEMENT)+(NumVectors-1)*sizeof(VECTOR));
	pElement = (LPELEMENT)GlobalLock (*phElement);
	pElement->NumVectors = NumVectors;
	pElement->FillColor = -1;  
	pElement->LineColor = -3;  
	pElement->Width = 0;
	pElement->Type = 3;
	pElement->Style = 0;
	pVector = &pElement->Vector; 
	for (i=0;i<pElement->NumVectors;i++,pVector++)
	{   
		Point1 = CurSymbol->TiePoint;
		Point2.x = x2[i];
		Point2.y = y2[i];
		AddDPointToMinMax (&Point2,&Bounds);
		pVector->Dist = ldistp (Point1,Point2);    
		pVector->AZM = getazd (&Point1,&Point2);
		pVector->Type = type2[i]; 
	}
	GlobalUnlock (*phElement++);
	NumVectors = 8;
	*phElement=GSSiGlobAlloc ( 482,GHND,sizeof(ELEMENT)+(NumVectors-1)*sizeof(VECTOR));
	pElement = (LPELEMENT)GlobalLock (*phElement);
	pElement->NumVectors = NumVectors;
	pElement->FillColor = -2;  
	pElement->LineColor = -3;  
	pElement->Width = 0;
	pElement->Type = 3;
	pElement->Style = 0;
	pVector = &pElement->Vector; 
	for (i=0;i<pElement->NumVectors;i++,pVector++)
	{   
		Point1 = CurSymbol->TiePoint;
		Point2.x = x22[i];
		Point2.y = y22[i];
		AddDPointToMinMax (&Point2,&Bounds);
		pVector->Dist = ldistp (Point1,Point2);    
		pVector->AZM = getazd (&Point1,&Point2);
		pVector->Type = type22[i]; 
	}
    SetSizePointsFromBounds (CurSymbol,&Bounds);
    GlobalUnlock (*phElement);
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	DestroySymbol (hSym);
		
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	_fstrcpy (CurSymbol->Name,"CHWYSHLD");
	_fstrcpy (CurSymbol->Desc,"County Highway Shield");
	CurSymbol->Parent=GetDictSymbolNumber("SHIELDS");
	CurSymbol->NumElements=1; 
	CurSymbol->Type=1;
	CurSymbol->TiePoint.x = 0;
	CurSymbol->TiePoint.y = 0;
	
	NumVectors = 5;
	DBoundsInit (&Bounds);
	CurSymbol->hElement=GSSiGlobAlloc ( 483,GHND,sizeof(ELEMENT)+(NumVectors-1)*sizeof(VECTOR));
	pElement = (LPELEMENT)GlobalLock (CurSymbol->hElement);
	pElement->NumVectors = NumVectors;
	pElement->FillColor = RGB(255,255,255);  
	pElement->LineColor = -3;  
	pElement->Width = 0;
	pElement->Type = 3;
	pElement->Style = 0;
	pVector = &pElement->Vector; 
	for (i=0;i<pElement->NumVectors;i++,pVector++)
	{   
		Point1 = CurSymbol->TiePoint;
		Point2.x = x3[i];
		Point2.y = y3[i];
		AddDPointToMinMax (&Point2,&Bounds);
		pVector->Dist = ldistp (Point1,Point2);    
		pVector->AZM = getazd (&Point1,&Point2);
		pVector->Type = type3[i]; 
	}
    SetSizePointsFromBounds (CurSymbol,&Bounds);
    GlobalUnlock (CurSymbol->hElement);
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	_fstrcpy (CurSymbol->Name,"CRDSHLD");
	_fstrcpy (CurSymbol->Desc,"County Road Shield");
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	_fstrcpy (CurSymbol->Name,"PRTSHLD");
	_fstrcpy (CurSymbol->Desc,"Province Route Shield");
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	_fstrcpy (CurSymbol->Name,"SQUARE");
	_fstrcpy (CurSymbol->Desc,"Square");
	CurSymbol->Parent=GetDictSymbolNumber("MISC");
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	DestroySymbol (hSym);
		
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	_fstrcpy (CurSymbol->Name,"STATSHLD");
	_fstrcpy (CurSymbol->Desc,"State Highway Shield");
	CurSymbol->Parent=GetDictSymbolNumber("SHIELDS");
	CurSymbol->NumElements=1; 
	CurSymbol->Type=1;
	CurSymbol->TiePoint.x = 0;
	CurSymbol->TiePoint.y = 0;
	
	NumVectors = 5;
	DBoundsInit (&Bounds);
	CurSymbol->hElement=GSSiGlobAlloc ( 484,GHND,sizeof(ELEMENT)+(NumVectors-1)*sizeof(VECTOR));
	pElement = (LPELEMENT)GlobalLock (CurSymbol->hElement);
	pElement->NumVectors = NumVectors;
	pElement->FillColor = RGB(255,255,255);  
	pElement->LineColor = -3;  
	pElement->Width = 0;
	pElement->Type = 3;
	pElement->Style = 0;
	pVector = &pElement->Vector; 
	for (i=0;i<pElement->NumVectors;i++,pVector++)
	{   
		Point1 = CurSymbol->TiePoint;
		Point2.x = x4[i];
		Point2.y = y4[i];
		AddDPointToMinMax (&Point2,&Bounds);
		pVector->Dist = ldistp (Point1,Point2);    
		pVector->AZM = getazd (&Point1,&Point2);
		pVector->Type = type4[i]; 
	}
    SetSizePointsFromBounds (CurSymbol,&Bounds);
    GlobalUnlock (CurSymbol->hElement);
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	_fstrcpy (CurSymbol->Name,"TRCSHLD");
	_fstrcpy (CurSymbol->Desc,"Trans-Canada Shield");
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	pElement->FillColor = -99;  
	_fstrcpy (CurSymbol->Name,"CIRCLE");
	_fstrcpy (CurSymbol->Desc,"Circle");
	CurSymbol->Parent=GetDictSymbolNumber("MISC");
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   

	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	pElement->FillColor = -99;  
	_fstrcpy (CurSymbol->Name,"INVISPNT");
	_fstrcpy (CurSymbol->Desc,"Invisible point symbol");
	CurSymbol->Parent=GetDictSymbolNumber("MISC");
	CurSymbol->NumElements=1; 
	CurSymbol->Type=1;
	CurSymbol->TiePoint.x = 0;
	CurSymbol->TiePoint.y = 0;
	
	NumVectors = 5;
	DBoundsInit (&Bounds);
	CurSymbol->hElement=GSSiGlobAlloc ( 485,GHND,sizeof(ELEMENT)+(NumVectors-1)*sizeof(VECTOR));
	pElement = (LPELEMENT)GlobalLock (CurSymbol->hElement);
	pElement->NumVectors = NumVectors;
	pElement->FillColor = RGB(255,255,255);  
	pElement->LineColor = -3;  
	pElement->Width = 0;
	pElement->Type = 2;
	pElement->Style = 0;
	pVector = &pElement->Vector; 
	for (i=0;i<pElement->NumVectors;i++,pVector++)
	{   
		Point1 = CurSymbol->TiePoint;
		Point2.x = x5[i];
		Point2.y = y5[i];
		AddDPointToMinMax (&Point2,&Bounds);
		pVector->Dist = ldistp (Point1,Point2);    
		pVector->AZM = getazd (&Point1,&Point2);
		pVector->Type = type5[i]; 
	}
    SetSizePointsFromBounds (CurSymbol,&Bounds);
    GlobalUnlock (CurSymbol->hElement);
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	DestroySymbol (hSym);
SkipStdSyms:		
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	_fstrcpy (CurSymbol->Name,"NEW");
	_fstrcpy (CurSymbol->Desc,"New automatically created symbols");
	CurSymbol->Parent=GetDictSymbolNumber("ALL");		 
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	DestroySymbol (hSym); 
	
	if (type)
	{
		HFILE	FidSymUse, FidSymPar, FidSymDump;
		OFSTRUCTGM	OFStruct;
		char	str[260], Snam[34];
		LPSTR	pStr;
		long	nrecs, nrecs2, TotLen, CurLoc;
		short	i, Type=1,ii;
		
		FidSymPar = GSSiOpenFile ("fromum\\symlist.txt",&OFStruct,OF_READ);
		FidSymUse = GSSiOpenFile ("fromum\\symuse.txt",&OFStruct,OF_READ);
		FidSymDump = GSSiOpenFile ("fromum\\symdump.txt",&OFStruct,OF_READ);
		if (FidSymUse != HFILE_ERROR)
		{   
			TotLen = GSSillseek (FidSymUse,0,2);
			GSSillseek (FidSymUse,0,0);
			CreateStatusWind (hWndMain,1,0);
			OpenSymDict (OF_READWRITE);     
			for (i=0;i<2;i++)
				fgetstring (str,256,FidSymUse);
			while (ContinueProcessing && fgetstring (str,256,FidSymUse))
			{
				CurLoc = GSSillseek (FidSymUse,0,1);
				hSym = AllocateNewSymbol ();
				CurSymbol = (LPSYMBOL)GlobalLock (hSym);
				_fstrcpy (CurSymbol->Desc,"Imported from UltiMap");
				_fstrncpy (Snam,&str[4],8);
				Snam[8] = 0;   
				Truncate (Snam);
				StatusWindowUpdate ("Create Symbol Dictionary",Snam, TotLen, CurLoc);
//				if (!GetDictSymbolNumber(Snam))
				{
					_fstrcpy (CurSymbol->Name,Snam);
					pStr = &str[14];
					nrecs = ldread (pStr,8);
					pStr+=8;
					nrecs2 = ldread (pStr,8);
					pStr+=8; 
					nrecs2 += ldread (pStr,8);
					pStr+=16; 
					Type = 1;
					if (nrecs2 > nrecs)
					{
						nrecs = nrecs2;
						Type = 2;
					}
					nrecs2 = ldread (pStr,8);
					if (nrecs2 > nrecs)
						Type = 3;
					CurSymbol->Type = Type; 
					CurSymbol->Parent = GetUMSymbolPar (FidSymPar,Snam);
					GlobalUnlock (hSym);  
					if (!_fstrnicmp ("WTR8",Snam,4))
						ii=1;
					if (!_fstricmp ("TRBKEEPR",Snam))
						ii=1;
					if (!_fstricmp ("KNOTE",Snam))
						ii=1;
					GetUMSymDump (Snam,hSym,FidSymDump);
					if (!CurSymbol->NumElements)
						CurSymbol->InVisible = TRUE;
						 
					SaveSymbol(hSym,0);   
				}
				DestroySymbol (hSym); 
			} 
			GSSiClose2 (&FidSymUse);   
			GSSiClose2 (&FidSymPar);   
			GSSiClose2 (&FidSymDump);
			DestroyStatusWindow(0);  
		}
	}
	SetContinueProcessing ( TRUE);	
 	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	_fstrcpy (CurSymbol->Name,"PROPERTY");
	_fstrcpy (CurSymbol->Desc,"Property data");
	CurSymbol->Parent=GetDictSymbolNumber("ALL");		 
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	DestroySymbol (hSym);
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	_fstrcpy (CurSymbol->Name,"PARCEL");
	_fstrcpy (CurSymbol->Desc,"Parcel Area");
	CurSymbol->Type = 3;
	CurSymbol->NumElements=0;
	CurSymbol->Parent=GetDictSymbolNumber("PROPERTY");		 
	NumVectors = 0;
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	DestroySymbol (hSym);
		
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	_fstrcpy (CurSymbol->Name,"USGS");
	_fstrcpy (CurSymbol->Desc,"USGS Data");
	CurSymbol->Parent=GetDictSymbolNumber("ALL");		 
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	DestroySymbol (hSym);
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
	_fstrcpy (CurSymbol->Name,"USGSQUAD");
	_fstrcpy (CurSymbol->Desc,"USGS Quad Area");
	CurSymbol->Type = 3;
	CurSymbol->NumElements=0;
	CurSymbol->Parent=GetDictSymbolNumber("USGS");		 
	NumVectors = 0;
	GlobalUnlock (hSym);	 
	SaveSymbol(hSym,0);   
	DestroySymbol (hSym); 
		
	CloseSymDict(); 

	return;
}   

BOOL SetPointSize (LPDOUBLE pSize, LPSTR NewPointSize)
{ 
	char	txt[128];
	LPSTR	lpType; 
	double	size; 
	BOOL	First = TRUE;

	if (NewPointSize)
		_fstrcpy (txt,NewPointSize);
	else
		_fstrcpy (txt,"[%NEW_POINT_SIZE]"); 
	ExpandText (txt);
	if (!*txt)
		_fstrcpy (txt,"2P");
	if (!ContinueProcessing)
	{
		SetContinueProcessing ( TRUE);
		return FALSE;
	}
Top:
	size = strtod (txt,&lpType);
	if (!size || !lpType) 
	{
		if (First) 
		{
			First = FALSE;
			_fstrcpy (txt,"2P");
			goto Top;
		}
		goto BadSymSizeType;
	}
	else if (*lpType == 'P')
		*pSize = -size;
	else if (*lpType == 'p')
		*pSize = size / BaseDistToWinDist;
	else if (*lpType == 'F' || *lpType == 'f')
		*pSize = ConvertInDist (size,1);
	else if (!*lpType || *lpType == 'M' || *lpType == 'm')
		*pSize = ConvertInDist (size,2);
	else 
	{
		char	mess[256]; 
				
BadSymSizeType:
		sprintf (mess,"Invalid point symbol size %s(%s)",NewPointSize,txt);
		GSSiMsgBox (GetFocus(),mess,0,MB_ICONEXCLAMATION,0);  
		return FALSE;
	} 
	return TRUE;
}
BOOL SetPointRot (LPDOUBLE pRot, LPSTR NewSymRot)
{   
	double	factor=1;
	char	txt[128];     
	LPSTR	pEnd;
	double  deg;
	
	if (NewSymRot)
		_fstrcpy (txt,NewSymRot);
	else
		_fstrcpy (txt,"[%NEW_POINT_ROT]");
	ExpandText (txt);
	if (!ContinueProcessing)
	{
		SetContinueProcessing ( TRUE);
		return FALSE;
	} 
	pEnd = LastChr(txt);
	if (*pEnd == 'D')
	{
		*pEnd = 0;
		deg = atof(txt);
		factor = RADDEG;
		deg = (360 - deg);
		*pRot = deg * factor;
	}
	else
		*pRot = atof(txt) * factor;
	*pRot = LTWOPI(*pRot);
	return TRUE;
}

BOOL SetPointColor (long *pColor, LPSTR NewSymColor)
{   
	char	txt[128];   
	
	if (NewSymColor)
		_fstrcpy (txt,NewSymColor);
	else
		_fstrcpy (txt,"[%NEW_POINT_COLOR]");
	ExpandText (txt);
	if (!ContinueProcessing)
	{
		SetContinueProcessing ( TRUE);
		return FALSE;
	}
	*pColor = atol(txt);
	return TRUE;
}

BOOL SetLineWidth (LPFLOAT pWidth, LPSTR NewLineWidth)
{   
	char	txt[128];   
	
	if (NewLineWidth)
		_fstrcpy (txt,NewLineWidth);
	else
		_fstrcpy (txt,"[%NEW_LINE_WIDTH]");
	ExpandText (txt);
	if (!ContinueProcessing)
	{
		SetContinueProcessing ( TRUE);
		return FALSE;
	}
	*pWidth = atof(txt);
	return TRUE;
}
BOOL SetRouteWidth (LPDOUBLE pWidth, LPSTR NewLineWidth)
{   
	char	txt[128];   
	
	if (NewLineWidth)
		_fstrcpy (txt,NewLineWidth);
	else
		_fstrcpy (txt,"[%NEW_ROUTE_WIDTH]");
	ExpandText (txt);
	if (!ContinueProcessing)
	{
		SetContinueProcessing ( TRUE);
		return FALSE;
	}
	*pWidth = atof(txt);
	return TRUE;
}

BOOL SetAreaColor (long *pColor,LPSTR NewAreaColor)
{   
	char	txt[128];   
	
	if (NewAreaColor)
		_fstrcpy (txt,NewAreaColor);
	else
		_fstrcpy (txt,"[%NEW_AREA_COLOR]");
	ExpandText (txt);
	if (!ContinueProcessing)
	{
		SetContinueProcessing ( TRUE);
		return FALSE;
	}
	*pColor = atol(txt);
	return TRUE;
}

BOOL SetLineColor (long *pColor,LPSTR NewLineColor)
{   
	char	txt[128];   
	
	if (NewLineColor)
		_fstrcpy (txt,NewLineColor);
	else
		_fstrcpy (txt,"[%NEW_LINE_COLOR]");
	ExpandText (txt);
	if (!ContinueProcessing)
	{
		SetContinueProcessing ( TRUE);
		return FALSE;
	}
	*pColor = atol(txt);
	return TRUE;
}   

BOOL SetLineSymbol (LPSHORT	pSym,LPSTR cSym) 
{
	char	txt[128];   
	
	if (cSym)
		_fstrcpy (txt,cSym);
	else
		_fstrcpy (txt,"[%NEW_LINE_SYM]");
	ExpandText (txt);
	if (!ContinueProcessing)
	{
		SetContinueProcessing ( TRUE);
		return FALSE;
	}
	*pSym = GetDictSymbolNumber(txt);   
	if (*pSym)
		return TRUE;
	else
	{
		char	mess[128];
		sprintf (mess,"Line symbol %s not in symbol dictionary",txt);
		GSSiMsgBox (GetFocus(),mess,0,MB_ICONEXCLAMATION,0);
	} 
	return FALSE;
			

}   

BOOL SetAreaSymbol (LPSHORT	pSym,LPSTR cSym) 
{
	char	txt[128];   
	
	if (cSym)
		_fstrcpy (txt,cSym);
	else
		_fstrcpy (txt,"[%NEW_AREA_SYM]");
	ExpandText (txt);
	if (!ContinueProcessing)
	{
		SetContinueProcessing ( TRUE);
		return FALSE;
	}
	*pSym = GetDictSymbolNumber(txt);
	if (*pSym)
		return TRUE;
	else
	{
		char	mess[128];
		sprintf (mess,"Area symbol %s not in symbol dictionary",txt);
		GSSiMsgBox (GetFocus(),mess,0,MB_ICONEXCLAMATION,0);
	} 
	return FALSE;
}   

BOOL SetPointSymbol (LPSHORT	pSym,LPSTR cSym) 
{
	char	txt[128];   
	
	if (cSym)
		_fstrcpy (txt,cSym);
	else
		_fstrcpy (txt,"[%NEW_POINT_SYM]");
	ExpandText (txt);  
	if (!ContinueProcessing)
	{
		SetContinueProcessing ( TRUE);
		return FALSE;
	}
	*pSym = GetDictSymbolNumber(txt);
	if (*pSym)
		return TRUE;
	else
	{
		char	mess[128];
		sprintf (mess,"Point symbol %s not in symbol dictionary",txt);
		GSSiMsgBox (GetFocus(),mess,0,MB_ICONEXCLAMATION,0);
	} 
	return FALSE;
} 
 
HPEN DrawSymbolBetweenPoints (HDC hDC,LPFPOINT pPoint1F, LPFPOINT pPoint2F, LPSYMBOL lpSym,
							  LPDOUBLE pCurSymDist, LPSHORT pCurElem,LPDOUBLE pSymFactor,LPSHORT pWantType,
							  short nElementsToDisplay, HANDLE hElementsToDisplay,BOOL IsLastPoint)
{   
	DPOINT		Point1D = FPointToDPoint (*pPoint1F);
	DPOINT		Point2D = FPointToDPoint (*pPoint2F);
	double		len=ldistp (Point1D,Point2D);
	static		double	Totlen=0;
	static		int		TotnPoints=0;
	double		AZ=getazd (&Point1D,&Point2D);  
	double		LoopLen, EndSymbolDist, EndSymDist, a, slope;  
	UINT		i,j;
	LPELEMENT	pElement;
	LPVECTOR	pVector, pVector1, pVector2;  
	HANDLE		hPlotPoint=GSSiGlobAlloc ( 486,GMEM_MOVEABLE,2048*sizeof(POINT));
	LPPOINT		pPoint, PlotPoint=(LPPOINT)GlobalLock (hPlotPoint), Points;
//	POINT		PlotPoint[1000];
	LPHANDLE	phElement; 
	DPOINT		VecPoint1, VecPoint2;
	DPOINT		SymBeginPoint;   
	int			InType=-2, np, Width=0;   
	BOOL		WantEndDist=FALSE, PreSymOutside=TRUE, PostSymOutside=TRUE;  
	long		nLoops=0,ii;
	short		UseHalfTone = lpSym->Number;
	COLORREF	LineColor=ConvertColor(0,UseHalfTone); 
	double		VectorOffset=0, SymbolLength; 
	double		SymFactor = DefaultSymbolFactor * (*pSymFactor);
	double		VSymFactor = lpSym->VSize * SymFactor, HSymFactor = lpSym->HSize * SymFactor;
	SIZE		size;

//	SetWindowText (hWndMain,"DSBP 1");
//	SaveDC (hDC);
//	GetWindowExtEx (hDC,&size);
//	SetWindowExtEx  ( hDC, size.cx*100, size.cy*100,0 ); 
	
	if (lpSym->BaseScale == 15)
		VSymFactor = SymFactor * (lpSym->SizePointH[1].x-lpSym->SizePointH[0].x)/3;
	*pCurElem = 0;
	SymBeginPoint = Point1D; 
	if (*pWantType == -3)
	{
		*pCurSymDist = HSymFactor; 
		SymBeginPoint = Point2D; 
	}
	else
		SymBeginPoint = dnewpt (SymBeginPoint,AZ,-*pCurSymDist); 
	SymbolLength = lpSym->TiePoint.x;
	if (SymbolLength <= 0)
		SymbolLength = 3;
//	SetWindowText (hWndMain,"DSBP 2");
	while (nLoops < 1000 && len > 0)
	{   
		nLoops++;
//		if (nLoops >995)
//			ii=1;
		LoopLen = min (len,(SymbolLength*HSymFactor)- *pCurSymDist); 
		EndSymbolDist = *pCurSymDist + LoopLen;
		phElement=&lpSym->hElement;
		phElement += *pCurElem;
//	SetWindowText (hWndMain,"DSBP 3");
		while (*pCurElem < lpSym->NumElements)
		{ 
			HPEN		hPen=0, hCurPen;
			HBRUSH		hBrush=0, hCurBrush; 
			DPOINT		POC, PC;
			BOOL		HavePOC=FALSE; 
			
//	SetWindowText (hWndMain,"DSBP 5");
			pElement = (LPELEMENT)GlobalLock (*phElement);  
			pVector1=&pElement->Vector;
			pVector2=pVector1+1;
			if (InType == -2 && pElement->NumVectors)
			{

				if (pElement->Type > 0 && pElement->Vector.Dist * HSymFactor> EndSymbolDist)
					goto SkipElement; //pElement->Vector[1]
			}
			switch (pElement->Type)
			{   
				case SVPRESYM:   //PRESYM
					InType = -1;
					break;
					
				case SVEND:    //END
					InType = -2;
					break;
					
				case SVPOSTSYM:    //POSTSYM
					InType = -3; 
					WantEndDist = TRUE;
					break;
					
				case SVLINE: 
				{   
					double	Fac;
					
					Width = GetLineElementWidthAndColor (lpSym,pElement,&LineColor,pSymFactor);
					hPen = CreatePen (PS_SOLID,(int)Width,LineColor); 
					if (hPen)
					{
						if (*pWantType ==  -5) 
						{
							GlobalUnlock (*phElement); 
							GSSiGlobUlFree (&hPlotPoint);
							//RestoreDC (hDC,-1);
							return hPen;
						}
						else
							hCurPen = SelectObject (hDC,hPen);
					} 
				}
					 
					break;
				case SVAREA:
					if (HighlightThisItem)
					{   LOGBRUSH    NDB;
					    
				        if (PatternBrush || Printing)
				        {
				            NDB.lbStyle = BS_HATCHED;
				            NDB.lbColor = ConvertColor(HighlightColor,lpSym->Number);
				            NDB.lbHatch = HS_DIAGCROSS;
				            hBrush =  CreateBrushIndirect(&NDB);
				        }
				        else
							hBrush = CreateSolidBrush (ConvertColor(HighlightColor,lpSym->Number));
					}
					else if (pElement->FillColor < -9 || (pElement->FillColorType == SVVARCOLOR && HaveVarFillColor))
						hBrush = 0;
					else if (pElement->FillColor < 0)
						hBrush = CreateGMBrush (GlobalColors[labs(pElement->FillColor)],lpSym->Number,hDC);
					else
						hBrush = CreateGMBrush (pElement->FillColor,lpSym->Number,hDC);
					if (hBrush)
						hCurBrush = SelectObject (hDC,hBrush); 
					hPen = GetStockObject(NULL_PEN);
					hCurPen = SelectObject (hDC,hPen); 
					break; 
			}
			if (InType == -2)  
				*pWantType = min (InType,*pWantType);  
			np = 0;
			if (InType == *pWantType)
			for (j=1,pVector1=&pElement->Vector,pVector2=pVector1+1;j<pElement->NumVectors;j++,pVector1++,pVector2++)
			{   
				
				if (WantEndDist)
				{
					EndSymDist = *pCurSymDist;
					WantEndDist = FALSE;
					VectorOffset = SymbolLength;
				}
				VecPoint1.x = (pVector1->Dist - VectorOffset) * HSymFactor;
				VecPoint1.y = -pVector1->AZM  * VSymFactor;
				VecPoint2.x = (pVector2->Dist - VectorOffset) * HSymFactor;
				VecPoint2.y = -pVector2->AZM  * VSymFactor;
/*				if (InType == -3)
				{
					VecPoint1.x += SymBeginPoint.x;
					VecPoint2.x += SymBeginPoint.x;
				} */
				if ((InType != -2 && InType != -3) && (VecPoint2.x > EndSymbolDist))
					break; 
				if ((InType != -1 && InType != -3) && ((VecPoint1.x < *pCurSymDist && VecPoint2.x < *pCurSymDist) ||
									 (VecPoint1.x > EndSymbolDist && VecPoint2.x > EndSymbolDist)))
					goto NextVector;
				if (InType != -3 && VecPoint1.y == VecPoint2.y)
				{
					VecPoint1.x = max (*pCurSymDist,VecPoint1.x);
					VecPoint2.x = min (EndSymbolDist,VecPoint2.x);
				}
				else if (VecPoint1.x != VecPoint2.x) 
				{   
					slope = (double)(VecPoint2.y - VecPoint1.y) / (double)(VecPoint2.x - VecPoint1.x);  
					a = VecPoint1.y - slope * VecPoint1.x;
					if (InType != -3 && VecPoint1.x < *pCurSymDist)
					{
						VecPoint1.x = max (*pCurSymDist,VecPoint1.x);
					}
					if (InType != -3 && VecPoint2.x > EndSymbolDist) 
					{
						VecPoint2.x = min (EndSymbolDist,VecPoint2.x);
				    }
				    VecPoint1.y = a + slope * VecPoint1.x;
				    VecPoint2.y = a + slope * VecPoint2.x;
				}
				if (InType == -2 || InType == -3 || (VecPoint1.x >= (*pCurSymDist) && VecPoint2.x <= (EndSymbolDist)))
				{ 
					DPOINT	DPoint;
					//PlotPoint[np] = DPointToPoint (dnewpt (SymBeginPoint,AZ,VecPoint1.x));
					//PlotPoint[np++] = newpt (PlotPoint[np],AZ+HALFPI,VecPoint1.y);
					DPoint = dnewpt (SymBeginPoint,AZ,VecPoint1.x);
					PlotPoint[np++] = DPointToPoint (dnewpt (DPoint,AZ+HALFPI,VecPoint1.y));
					if (np > 1 && PlotPoint[np-1].x == PlotPoint[np-2].x && PlotPoint[np-1].y == PlotPoint[np-2].y)
						np--; 
					//PlotPoint[np] = DPointToPoint (dnewpt (SymBeginPoint,AZ,VecPoint2.x));
					//PlotPoint[np++] = newpt (PlotPoint[np],AZ+HALFPI,VecPoint2.y);
					DPoint = dnewpt (SymBeginPoint,AZ,VecPoint2.x);
					PlotPoint[np++] = DPointToPoint (dnewpt (DPoint,AZ+HALFPI,VecPoint2.y));
					if (np > 1 && PlotPoint[np-1].x == PlotPoint[np-2].x && PlotPoint[np-1].y == PlotPoint[np-2].y)
						np--; 
				} 
NextVector:;
			} 
			if (np > 0)
			{   
				if (!nElementsToDisplay || ItemInList (*pCurElem,nElementsToDisplay,hElementsToDisplay))  
				{
					short	Type = pElement->Type; 
				    
				    if (np < 3)
				    	Type = SVLINE;
				    if (np == 1)
				    	SetPixel (hDC,PlotPoint[0].x,PlotPoint[0].y,LineColor);
				    else
					switch (Type)
					{
						case SVLINE: 
							if (pElement->Shadow)
							{
								HPEN	hShadowPen, hCPen;
								LOGPEN	lPen;
								
								hCPen = SelectObject (hDC,GetStockObject(BLACK_PEN));
								GetObject (hCPen,sizeof(LOGPEN),&lPen);
								SelectObject (hDC,hCPen);
								hShadowPen = CreatePen (PS_SOLID,lPen.lopnWidth.x+2,RGB(255,255,255));
								hCurPen = SelectObject (hDC,hShadowPen);
								Polyline (hDC,PlotPoint,np);
								SelectObject (hDC,hCurPen);
								GSSiDeleteObject (&hShadowPen);
							}
							if (pElement->Squared)// && UseFlatEndPolyline)
							{
								if (IsLastPoint)
								{
									if (!nElemPoints[*pCurElem])
										DrawLineWithFlatEnd (hDC,np,PlotPoint,Width,LineColor);
									else
									{
										Points = GlobalLock (hElemPoints[*pCurElem]);
										if (SamePoint (Points[nElemPoints[*pCurElem]-1],PlotPoint[0]))
										{
											Points[nElemPoints[*pCurElem]++] = PlotPoint[1];
											DrawLineWithFlatEnd (hDC,nElemPoints[*pCurElem],Points,ElemLineWidth[*pCurElem],ElemLineColor[*pCurElem]);
											GSSiGlobUlFree (&hElemPoints[*pCurElem]);
											nElemPoints[*pCurElem] = 0;
										}
										else
										{
											DrawLineWithFlatEnd (hDC,nElemPoints[*pCurElem],Points,ElemLineWidth[*pCurElem],ElemLineColor[*pCurElem]);
											GSSiGlobUlFree (&hElemPoints[*pCurElem]);
											nElemPoints[*pCurElem] = 0;
											DrawLineWithFlatEnd (hDC,np,PlotPoint,Width,LineColor);
										}
									}
								}
								else if (nElemPoints[*pCurElem])
								{
									Points = GlobalLock (hElemPoints[*pCurElem]);
									if (SamePoint (Points[nElemPoints[*pCurElem]-1],PlotPoint[0]))
									{
										Points[nElemPoints[*pCurElem]++] = PlotPoint[1];
									}
									else
									{
										DrawLineWithFlatEnd (hDC,nElemPoints[*pCurElem],Points,ElemLineWidth[*pCurElem],ElemLineColor[*pCurElem]);
										Points[0] = PlotPoint[0];
										Points[1] = PlotPoint[1];
										nElemPoints[*pCurElem] = 2;
										ElemLineWidth[*pCurElem] = Width;
										ElemLineColor[*pCurElem] = LineColor;
									}
									GlobalUnlock (hElemPoints[*pCurElem]);
								}
								else
								{
									GSSiGlobFree (&hElemPoints[*pCurElem]);
									hElemPoints[*pCurElem] = GSSiGlobAlloc (1572,GMEM_MOVEABLE,MAX_ELEMENT_VECTORS*sizeof(POINT));
									Points = GlobalLock (hElemPoints[*pCurElem]);
									Points[0] = PlotPoint[0];
									Points[1] = PlotPoint[1];
									nElemPoints[*pCurElem] = 2;
									ElemLineWidth[*pCurElem] = Width;
									ElemLineColor[*pCurElem] = LineColor;
									GlobalUnlock (hElemPoints[*pCurElem]);
								}
							}
							else
								Polyline (hDC,PlotPoint,np);  //PlotPoint[1]      

							break;
						case SVAREA:
							Polygon (hDC,PlotPoint,np); 
							break; 
					} 
					if (pSymbolRect)
						AddPointsToSymbolRect (hDC,PlotPoint,np,Width);
				}
			}
			if (hPen) 
			{
				SelectObject (hDC,hCurPen); 
				if (hPen != GetStockObject(NULL_PEN))
					GSSiDeleteObject(&hPen);
			}
			if (hBrush)	
			{	
				SelectObject (hDC,hCurBrush);
				GSSiDeleteObject(&hBrush);
			}
SkipElement:				
			GlobalUnlock (*phElement); 
			if (nElemPoints[*pCurElem])
			{
				Points = GlobalLock (hElemPoints[*pCurElem]);
				DrawLineWithFlatEnd (hDC,nElemPoints[*pCurElem],Points,ElemLineWidth[*pCurElem],ElemLineColor[*pCurElem]);
				nElemPoints[*pCurElem] = 0;
				GSSiGlobUlFree (&hElemPoints[*pCurElem]);
			}
			(*pCurElem)++; 
			phElement++;
		} 
EndSymbol:
		if (*pWantType ==  -5) 
			break;
		*pCurSymDist = EndSymbolDist;
		len -= LoopLen;
		if (*pCurSymDist >= HSymFactor || len > 0)// || *pCurElem >= lpSym->NumElements)  
		{
			*pCurSymDist = 0;  
			if (*pWantType != -3)
				WantEndDist = FALSE;
		}
/*		for (i=0;i<*pCurElem+1;i++)
		{
			GSSiGlobFree (&hElemPoints[i]);
			nElemPoints[i] = 0;
		}*/
		*pCurElem = 0;
		SymBeginPoint = dnewpt (SymBeginPoint,AZ,EndSymbolDist);   
		if (*pWantType == -3)
			len = 0; 
		else
			InType = -2;
	}
	GSSiGlobUlFree (&hPlotPoint);
	if (IsLastPoint)
	{
		TotnPoints = 0;
		Totlen = 0;
	}
	//RestoreDC (hDC,-1);
	return 0;
}

void AddPointsToSymbolRectF(HDC	hDC, HPFPOINT lpPoints, long npnts, int Width)
{
	HANDLE handle = GSSiGlobAlloc(0, GMEM_MOVEABLE, npnts*sizeof(POINT) + 4);
	HPPOINT	pPoints = (HPPOINT)GlobalLock(handle);

	for (int i = 0; i < npnts; i++)
	{
		pPoints[i].x = IDNINT(lpPoints[i].x);
		pPoints[i].y = IDNINT(lpPoints[i].y);
	}
	AddPointsToSymbolRect(hDC, pPoints, npnts, Width);
	GSSiGlobUlFree(&handle);
}
void AddFPointsToSymbolRect(HDC	hDC, HPFPOINT lpPoints, long npnts, int Width)
{
	HANDLE hPoints = GSSiGlobAlloc(0, GMEM_MOVEABLE, npnts * sizeof(POINT)+4);
	LPPOINT pPoints = GlobalLock(hPoints);

	for (int i = 0; i < npnts; i++)
		pPoints[i] = FPointToPoint(lpPoints[i]);
	AddPointsToSymbolRect(hDC,pPoints, npnts, Width);
	GSSiGlobUlFree(&hPoints);
}

void AddPointsToSymbolRect(HDC	hDC, HPPOINT lpPoints, long npnts, int Width)
{
	RECT	Rect;
	
	if (!pSymbolRect)
		return; 
	Rect.left = Rect.right = lpPoints->x;
	Rect.top = Rect.bottom = (lpPoints++)->y;    
	npnts--;
	while (npnts--)
		AddPointToRect (*(lpPoints++),&Rect); 
	switch (Width)
	{
		case 0: 
			if (hDC)
			{
				HPEN	hPen=SelectObject (hDC,GetStockObject (NULL_PEN));    
				LOGPEN	LogPen;
				
				GetObject(hPen, sizeof(LOGPEN), (LPSTR) &LogPen);
				Width = LogPen.lopnWidth.x;
				if (Width > 1)
					InflateRect (&Rect,Width/2,Width/2); 
				SelectObject (hDC,hPen); 
			} 
			break; 
		case 1:
			break;
		default:
			InflateRect (&Rect,Width/2,Width/2);  
			break;
	}
	if (pSymbolRect->right < pSymbolRect->left)
		*pSymbolRect = Rect;
	else
		UnionRect (pSymbolRect,pSymbolRect,&Rect);
	return;
}
	  
BOOL BigPolyline (HDC hDC, HPPOINT lpPoints, long npnts,int Width)
#if ENABLETRACE
{GSSiEnterProg (963);
#endif
{   
	BOOL	rtn;    
	LPPOINT	pPoints;  
	HANDLE	hMem;   
	long	mxp=MaxDisplayPoints/2;
	int		np,ii=npnts-1;
	POINT	pt[100];

	for (int i = 0; i < min(100, npnts); i++)
		pt[i] = lpPoints[i];
	if (pSymbolRect)
		AddPointsToSymbolRect (hDC,lpPoints,npnts,Width); 
	rtn = Polyline(hDC, lpPoints, min(npnts, mxp)); lpPoints[ii];
	if (!rtn)
	{
/*		char	mess[128];
		OFSTRUCTGM	OFStruct;
		HFILE	Fid; 
		short	n=0;
		
		Fid = GSSiOpenFile ("temp.txt",&OFStruct,OF_CREATE);
		while (npnts--)
		{
			sprintf (mess,"%i %i %i",n++,lpPoints->x,lpPoints->y); 
			lpPoints++;
			fputstring (mess,Fid);
		}   
		GSSiClose2 (&Fid);*/
		ii=1;     
{
#if ENABLETRACE
GSSiExitProg (963);
#endif
		return FALSE;
}
	}
	npnts -= mxp;                 
	while (npnts > 0)
	{   
		lpPoints += mxp;
		hMem = GSSiGlobAlloc ( 760,GMEM_MOVEABLE,USHRT_MAX);
		pPoints = (LPPOINT)GlobalLock (hMem);  
		np = min (npnts,mxp);
		_fmemmove (pPoints,lpPoints,np*sizeof(POINT));
		if (np > 1)
		{
			rtn = Polyline (hDC,pPoints,np);    
			if (!rtn)
				ii=1;
		}
		GSSiGlobUlFree (&hMem);
		npnts -= np; 
	}
{
#if ENABLETRACE
GSSiExitProg (963);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 
   
BOOL FlatEndPolylineI(HDC hDC, HPPOINT lpPoints, long npnts, int Width, COLORREF Color)
{
	HANDLE handle = GSSiGlobAlloc(0, GMEM_MOVEABLE, npnts*sizeof(FPOINT)+4);
	HPFPOINT	pPoints = (HPFPOINT)GlobalLock(handle);

	for (int i = 0; i < npnts; i++)
	{
		pPoints[i].x = lpPoints[i].x;
		pPoints[i].y = lpPoints[i].y;
	}
	BOOL rtn = FlatEndPolyline(hDC, pPoints, npnts, Width, Color);
	GSSiGlobUlFree (&handle);
	return rtn;
}

BOOL FlatEndPolyline (HDC hDC, HPFPOINT lpPoints, long npnts,int Width,COLORREF Color)
#if ENABLETRACE
{GSSiEnterProg (963);
#endif
{   
	BOOL	rtn;    
	LPFPOINT	pPoints;  
	HANDLE	hMem;   
	long	mxp=MaxDisplayPoints/2;
	unsigned short	np,ii;
	
	if (pSymbolRect)
		AddPointsToSymbolRectF (hDC,lpPoints,npnts,Width); 
	rtn = DrawLineWithFlatEndF (hDC,(short)min (npnts,mxp),lpPoints,Width,Color);  
//	rtn = Polyline (hDC,lpPoints,(short)min (npnts,mxp));
	if (!rtn)
	{
/*		char	mess[128];
		OFSTRUCTGM	OFStruct;
		HFILE	Fid; 
		short	n=0;
		
		Fid = GSSiOpenFile ("temp.txt",&OFStruct,OF_CREATE);
		while (npnts--)
		{
			sprintf (mess,"%i %i %i",n++,lpPoints->x,lpPoints->y); 
			lpPoints++;
			fputstring (mess,Fid);
		}   
		GSSiClose2 (&Fid);*/
		ii=1;     
{
#if ENABLETRACE
GSSiExitProg (963);
#endif
		return FALSE;
}
	}
	npnts -= mxp;                 
	while (npnts > 0)
	{   
		lpPoints += mxp;
		hMem = GSSiGlobAlloc ( 760,GMEM_MOVEABLE,USHRT_MAX*2);
		pPoints = (LPFPOINT)GlobalLock (hMem);  
		np = min (npnts,mxp);
		_fmemmove (pPoints,lpPoints,np*sizeof(FPOINT));
		if (np > 1)
		{   
			rtn = DrawLineWithFlatEndF (hDC,np,pPoints,Width,Color);  
//			rtn = Polyline (hDC,pPoints,np);    
			if (!rtn)
				ii=1;
		}
		GSSiGlobUlFree (&hMem);
		npnts -= np; 
	}
{
#if ENABLETRACE
GSSiExitProg (963);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}    

BOOL BigFPolyline (HDC hDC, HPDPOINT lpPoints, long npnts,double Width)
#if ENABLETRACE
{GSSiEnterProg (964);
#endif
{
	HANDLE handle = GSSiGlobAlloc ( 761,GMEM_MOVEABLE,(npnts+1)*sizeof(POINT));
	HPPOINT	pPoints = (HPPOINT)GlobalLock (handle);
	HPPOINT	pPointsBeg = pPoints;
	BOOL	rtn=1; 
	DWORD	np=npnts;
	HPEN	hCPen, hPen=0;
	LOGPEN	lPen;

	hCPen = SelectObject(hDC, GetStockObject(BLACK_PEN));
	GetObject(hCPen, sizeof(LOGPEN), &lPen);
	if (Width != 0)
	{
		int	width;
		
		width = IDNINT(AdjustWidth(Width));
		hPen = CreatePen(PS_SOLID, width, lPen.lopnColor);
		SelectObject (hDC,hPen);
	}
	else
		SelectObject(hDC, hCPen);

	while (np--)
	{
		pPoints->x = IDNINT(lpPoints->x);
		pPoints++->y = IDNINT(lpPoints++->y);
	}
	if (npnts > 1) 
	{	 
//		pPointsBeg[npnts]=pPointsBeg[npnts-2];
		rtn = BigPolyline (hDC,pPointsBeg,npnts,0); 
//		BigPolyline (hDC,&pPointsBeg[npnts-1],2,0); 
	}
	if (hPen)
	{
		SelectObject (hDC,hCPen);
		DeleteObject (hPen);
	}
   	//SetPixel (hDC,pPointsBeg->x,pPointsBeg->y,0);//debug
   	SetPixel (hDC,pPointsBeg[npnts-1].x,pPointsBeg[npnts-1].y,AutoYellow(lPen.lopnColor));

	GSSiGlobUlFree (&handle);
{
#if ENABLETRACE
GSSiExitProg (964);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

BOOL BigSPolyline (HDC hDC, HPPOINTS lpPoints, long npnts,int Width)
#if ENABLETRACE
{GSSiEnterProg (964);
#endif
{
	HANDLE handle = GSSiGlobAlloc ( 761,GMEM_MOVEABLE,(npnts+1)*sizeof(POINT));
	HPPOINT	pPoints = (HPPOINT)GlobalLock (handle);
	HPPOINT	pPointsBeg = pPoints;
	BOOL	rtn=1; 
	DWORD	np=npnts;
	
	while (np--)
	{
		pPoints->x = lpPoints->x;
		pPoints++->y = lpPoints++->y;
	}
	if (npnts > 1)
	{	 
		pPointsBeg[npnts]=pPointsBeg[npnts-1];
		rtn = BigPolyline (hDC,pPointsBeg,npnts+1,Width); 
	}
//   	SetPixel (hDC,pPointsBeg->x,pPointsBeg->y,0);
//   	SetPixel (hDC,pPointsBeg[npnts-1].x,pPointsBeg[npnts-1].y,AutoYellow(0));

	GSSiGlobUlFree (&handle);
{
#if ENABLETRACE
GSSiExitProg (964);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

int GWPolyline2 (HDC hDC, HPPOINT Points, long npnts,int idesc)
{
    HANDLE	hPoints = GSSiGlobAlloc (0,GMEM_MOVEABLE,npnts*sizeof(DPOINT));
    HPDPOINT	DPoints=(HPDPOINT)GlobalLock (hPoints);
    long	i; 
    int	rtn;
    
    for (i=0;i<npnts;i++)
    	DPoints[i] = WinPtToBasePt (Points[i]);
	rtn = GWPolylineD (CurView->hDC, DPoints, npnts,idesc); 
	GSSiGlobUlFree (&hPoints);
	return rtn;
}

BOOL DrawOneWayArrows (HDC hDC, int OneWay,HPFPOINT Points, int npnts,int Width)
{
	int		nArrows = 2;
	COLORREF lineColor = GetGlobalLVal2("[%1WAYARROWLINECOLOR]", RGB(255,255,255));
	COLORREF fillColor = GetGlobalLVal2("[%1WAYARROWFILLCOLOR]", RGB(160, 160, 160));
	double   widthFactor = GetGlobalDVal2("[%1WAYARROWWIDTHFACTOR]", 1.0);
	double   lengthFactor = GetGlobalDVal2("[%1WAYARROWLENGTHFACTOR]", 1.0);
	double   gapFactor = GetGlobalDVal2("[%1WAYARROWGAPFACTOR]", 1.0);

	Width *= widthFactor;

	if (!OneWay)
		return FALSE;
	{
		double	ArrowLength=Width*4;
		double	PolyLen = GetPolyLengthF (Points,npnts);
		double	GapLength = 8;
		
		GapLength *= gapFactor;
		ArrowLength *= lengthFactor;
		nArrows = max (1,PolyLen / (GapLength * ArrowLength));
		GapLength = (PolyLen - nArrows * ArrowLength) / (nArrows+1);

		if (PolyLen < ArrowLength || nArrows < 1)
			return FALSE;
		{
			int			NumArrowPoints,i, iarrow, nap;
			HANDLE		hDPoly=GSSiGlobAlloc (0,GMEM_MOVEABLE,npnts * sizeof (DPOINT));
			HPDPOINT	pArrowDPoints, pDPoints=GlobalLock (hDPoly);
			HANDLE		hArrowPoints, hArrowDPoints;
			double		StartArrow, EndArrow;
			HPPOINT		pArrowPoints;
			int			ArrowLineWidth = Width/4+1;
			int			ArrowHeadWidth = Width;
			HPEN		hOldPen, hPen = CreatePen (PS_SOLID,ArrowLineWidth,fillColor);
			HPEN		hWhitePen1 = CreatePen (PS_SOLID,ArrowLineWidth+2,lineColor);
			HPEN		hWhitePen2 = CreatePen (PS_SOLID,3,lineColor);
			DPOINT		ArrowHeadD[3];
			POINT		ArrowHead[3];
			HBRUSH		hOldBrush, hBrush = CreateSolidBrush(fillColor);
			double		Az;
			DPOINT		pt;

			hOldBrush = SelectObject (hDC,hBrush);
			hOldPen = SelectObject (hDC,hPen);
			for (i=0;i<npnts;i++)
				pDPoints[i] = FPointToDPoint (Points[i]);
			StartArrow = GapLength;//max (0,(PolyLen - (nArrows * 2 -1)*ArrowLength)/2);
			if (StartArrow < 0)
			{
				ArrowLength /= 2;
				StartArrow = max (0,(PolyLen - (nArrows * 2 -1)*ArrowLength)/2);
				if (StartArrow < 0)
					nArrows = 0;
			}
			EndArrow = StartArrow + ArrowLength;
			for (iarrow=0; iarrow<nArrows; iarrow++,StartArrow += (GapLength+ArrowLength), EndArrow += (GapLength+ArrowLength))
			{
				hArrowDPoints = GetPolyBetweenDist (pDPoints,npnts,StartArrow,EndArrow,&NumArrowPoints,FALSE,FALSE); 
				if (NumArrowPoints > 1)
				{
					pArrowDPoints = GlobalLock (hArrowDPoints);
					hArrowPoints = GSSiGlobAlloc (0,GMEM_MOVEABLE,NumArrowPoints*sizeof(POINT));
					pArrowPoints = GlobalLock (hArrowPoints);
					switch (OneWay)
					{
					case 1:
						Az = getazd (&pArrowDPoints[NumArrowPoints-1],&pArrowDPoints[0]);
						ArrowHeadD[0] = dnewpt (pArrowDPoints[NumArrowPoints-1],Az,-ArrowLineWidth);
						pt = dnewpt (ArrowHeadD[0],Az,ArrowHeadWidth*2.5);
						ArrowHeadD[1] = dnewpt (pt,Az+HALFPI,ArrowHeadWidth);
						ArrowHeadD[2] = dnewpt (pt,Az-HALFPI,ArrowHeadWidth);
						break;
					default:
						ArrowHeadD[0] = pArrowDPoints[0];
						Az = getazd(&ArrowHeadD[0], &pArrowDPoints[NumArrowPoints - 1]);
						pt = dnewpt (ArrowHeadD[0],Az,ArrowHeadWidth*2.5);
						ArrowHeadD[1] = dnewpt (pt,Az+HALFPI,ArrowHeadWidth);
						ArrowHeadD[2] = dnewpt (pt,Az-HALFPI,ArrowHeadWidth);
						break;
					}
					nap = 0;
					for (i = 0; i<NumArrowPoints; i++)
					{
						pArrowPoints[nap] = DPointToPoint(pArrowDPoints[i]);
						if (!i || !SamePoint(pArrowPoints[nap], pArrowPoints[nap - 1]))
							nap++;
					}
					NumArrowPoints = nap;
					for (i = 0; i < 3; i++)
					{
						ArrowHead[i] = DPointToPoint(ArrowHeadD[i]);
					}
					SelectObject(hDC, hBrush);
					SelectObject (hDC,hWhitePen1);
					Polyline (hDC,pArrowPoints,NumArrowPoints);
					SelectObject (hDC,hWhitePen2);
					Polygon (hDC,ArrowHead,3);
					SelectObject (hDC,hPen);
					Polyline (hDC,pArrowPoints,NumArrowPoints);
					SelectObject (hDC,GetStockObject (NULL_PEN));
					Polygon (hDC,ArrowHead,3);
					SelectObject (hDC,hPen);
					GSSiGlobUlFree (&hArrowDPoints);
					GSSiGlobUlFree (&hArrowPoints);
				}
			}
			GSSiGlobUlFree (&hDPoly); 
			SelectObject (hDC,hOldPen);
			GSSiDeleteObject (&hPen);
			SelectObject (hDC,hOldBrush);
			GSSiDeleteObject (&hBrush);
			GSSiDeleteObject (&hWhitePen1);
			GSSiDeleteObject (&hWhitePen2);
		}
	}
	return TRUE;
}

void DisplayHollowLines (BOOL Clear)
{   
	HPEN	hPen, OldPen; 
	LPSTR	pFile; 
	short 	Width;   
	short	pass;
	
    if (FidHollowLines != HFILE_ERROR)
    {   
    	if (!Clear && ShowHollowStreet)
    	{
		    SaveDC (CurView->hDC);
			InitRecord (CurView->hDC); 
			SetROP2(CurView->hDC,DisplayRasterOpt);
			SetDisplayMode (CurView->hDC, GF_TEXTMODE); 
			GSSiDeleteObject(&CurView->hRgn);
			CurView->hRgn = CreateVPRgn(FALSE,FALSE);
			SelectClipRgn (CurView->hDC,CurView->hRgn);
			GSSiDeleteObject(&CurView->hRgn); 
			for (pass=0;pass<2;pass++)
			{ 
				GSSillseek (FidHollowLines,0,0);
			    while (BigRead (FidHollowLines,(HPSTR)&HollowLineHeader,sizeof(HollowLineHeader)) == sizeof(HollowLineHeader))
			    {
				    HANDLE	hPoints = GSSiGlobAlloc (0,GMEM_MOVEABLE,HollowLineHeader.npnts*sizeof(FPOINT));
				    HPFPOINT	SPoints=(HPFPOINT)GlobalLock (hPoints); 
				    
				    BigRead (FidHollowLines,(HPSTR)SPoints,HollowLineHeader.npnts*sizeof(FPOINT));
				    Width = max(0,HollowLineHeader.width-2*DeviceToScreenFactor());
				    if (HollowLineHeader.desc > 0 && HollowLineHeader.width > 2*DeviceToScreenFactor())
				    {   
				    	if (pass == SymbolInUseShieldsList (HollowLineHeader.desc))
				    	{
						    if (Width > 0)
						    {
							    if (UseFlatEndPolyline)
									FlatEndPolyline (CurView->hDC, SPoints, HollowLineHeader.npnts,Width,HollowLineHeader.color);  
								else
								{
								    hPen = CreatePen (PS_SOLID,Width,HollowLineHeader.color);  
								    OldPen = SelectObject (CurView->hDC,hPen); 
									BigFPolyline (CurView->hDC, SPoints, HollowLineHeader.npnts,Width); 
								    SelectObject (CurView->hDC,OldPen);
								    GSSiDeleteObject (&hPen); 
								}
							}
						}

					}
					DrawOneWayArrows (CurView->hDC, HollowLineHeader.OneWay,SPoints, HollowLineHeader.npnts,Width); 
				    GSSiGlobUlFree (&hPoints);  
				}
			}
			RestoreDC (CurView->hDC,-1);
		}
	    GSSiClose2 (&FidHollowLines); 
	    FidHollowLines = HFILE_ERROR;
		pFile = GlobalLock (hHollowLinesFile);
	    GSSiRemove (pFile);
	    GlobalUnlock (hHollowLinesFile);
	}
	return;
}  

DPOINT RectIntersect (LPRECT pRect,DPOINT P1, DPOINT P2)
{
	DPOINT IntPoint;
	double	AZ=getazd (&P1,&P2); 
	double	Intx, Inty;
	short	ii;
	
	if (min (P1.x,P2.x) <= pRect->left && max (P1.x,P2.x) >= pRect->left)
	{
		if (LINSEC (P1.x,P1.y,AZ,pRect->left,pRect->bottom,HALFPI,&Intx,&Inty) == 1) 
		{
			if (Inty <= pRect->bottom && Inty >= pRect->top)
				goto Exit;
		}
	}
	if (min (P1.x,P2.x) <= pRect->right && max (P1.x,P2.x) >= pRect->right)
	{
		if (LINSEC (P1.x,P1.y,AZ,pRect->right,pRect->bottom,HALFPI,&Intx,&Inty) == 1) 
		{
			if (Inty <= pRect->bottom && Inty >= pRect->top)
				goto Exit;
		}
	}
	if (min (P1.y,P2.y) <= pRect->top && max (P1.y,P2.y) >= pRect->top)
	{
		if (LINSEC (P1.x,P1.y,AZ,pRect->left,pRect->top,0,&Intx,&Inty) == 1) 
		{
			if (Intx >= pRect->left && Intx <= pRect->right)
				goto Exit;
		}
	}
	if (min (P1.y,P2.y) <= pRect->bottom && max (P1.y,P2.y) >= pRect->bottom)
	{
		if (LINSEC (P1.x,P1.y,AZ,pRect->left,pRect->bottom,0,&Intx,&Inty) == 1) 
		{
			if (Intx >= pRect->left && Intx <= pRect->right)
				goto Exit;
		}
	} 
	ii=1;
Exit:
	IntPoint.x = Intx;
	IntPoint.y = Inty;
	return IntPoint;
}

int RectIntersect2 (LPRECT pRect,DPOINT P1, DPOINT P2, LPDPOINT IntPoints)
{
	double	AZ=getazd (&P1,&P2); 
	double	Intx, Inty;
	short	ii; 
	int		nint=0;
	
	if (min (P1.x,P2.x) <= pRect->left && max (P1.x,P2.x) >= pRect->left)
	{
		if (LINSEC (P1.x,P1.y,AZ,pRect->left,pRect->bottom,HALFPI,&Intx,&Inty) == 1) 
		{
			if (Inty <= pRect->bottom && Inty >= pRect->top) 
			{
				IntPoints[nint].x = Intx;
				IntPoints[nint++].y = Inty; 
			}
		}
	}
	if (min (P1.x,P2.x) <= pRect->right && max (P1.x,P2.x) >= pRect->right)
	{
		if (LINSEC (P1.x,P1.y,AZ,pRect->right,pRect->bottom,HALFPI,&Intx,&Inty) == 1) 
		{
			if (Inty <= pRect->bottom && Inty >= pRect->top)
			{
				IntPoints[nint].x = Intx;
				IntPoints[nint++].y = Inty; 
			}
		}
	}
	if (min (P1.y,P2.y) <= pRect->top && max (P1.y,P2.y) >= pRect->top)
	{
		if (LINSEC (P1.x,P1.y,AZ,pRect->left,pRect->top,0,&Intx,&Inty) == 1) 
		{
			if (Intx >= pRect->left && Intx <= pRect->right)
			{
				IntPoints[nint].x = Intx;
				IntPoints[nint++].y = Inty;
			}
		}
	}
	if (min (P1.y,P2.y) <= pRect->bottom && max (P1.y,P2.y) >= pRect->bottom)
	{
		if (LINSEC (P1.x,P1.y,AZ,pRect->left,pRect->bottom,0,&Intx,&Inty) == 1) 
		{
			if (Intx >= pRect->left && Intx <= pRect->right)
			{
				IntPoints[nint].x = Intx;
				IntPoints[nint++].y = Inty;
			}
		}
	} 
	if (nint > 2)
		ii=1;
	return nint;
}

int GWPolylineScreen2 (HDC hDC, HPDPOINT Points, long npnts,int idesc)
{
    long	i, n=0, npm1=npnts-1; 
    DPOINT	ScreenPoint,PreviousPoint;  
    BOOL	PrevPointIn, PointIn=FALSE;   
    HANDLE	hPoints = GSSiGlobAlloc (0,GMEM_MOVEABLE,npnts*sizeof(DPOINT));
    HPDPOINT	SPoints=(HPDPOINT)GlobalLock (hPoints);
    int		rtn=0, w;
	double	dw;
	RECT	SaveRect = CurView->DrawRect;
    
    if (StreetWidth)
    	dw=(double)StreetWidth/CurView->MetersPerPixel;   
    else
		dw = GetSymbolWidth(abs(idesc));
	dw *= ThemeWidthFactor * StreetWidthFactor;
	w = IDNINT (dw);
	InflateRect (&CurView->DrawRect,w,w);
    SaveDC (hDC);
	SetDisplayMode (hDC, GF_TEXTMODE);
   	PreviousPoint = Points[0];   
	PrevPointIn = DPointInRect(&PreviousPoint,&CurView->DrawRect);
    for (i=1;i<npnts;i++)
    {   
    	ScreenPoint = Points[i];
		PointIn = DPointInRect(&ScreenPoint,&CurView->DrawRect);
   		if (PrevPointIn && PointIn)
    		SPoints[n++] = PreviousPoint;
    	else if (PrevPointIn && !PointIn)   
    	{
	   		SPoints[n++] = PreviousPoint;
    		SPoints[n++] = RectIntersect (&CurView->DrawRect,PreviousPoint,ScreenPoint);
    		rtn = DisplayScreenLineSegment (hDC, SPoints,n,idesc,w);
    		n=0; 
    	}
    	else if (!PrevPointIn && PointIn)
    	    SPoints[n++] = RectIntersect (&CurView->DrawRect,PreviousPoint,ScreenPoint);
    	else if (RectIntersect2 (&CurView->DrawRect,PreviousPoint,ScreenPoint,SPoints) == 2)
    		rtn = DisplayScreenLineSegment (hDC, SPoints,2,idesc,w);        //SPoints[1]
    	PreviousPoint = ScreenPoint;
    	PrevPointIn = PointIn;
    }
    if (PointIn)
    	SPoints[n++] = ScreenPoint;
    if (n > 1)
		rtn = DisplayScreenLineSegment (hDC, SPoints,n,idesc,w);
	RestoreDC (hDC,-1);
    GSSiGlobUlFree (&hPoints);
	CurView->DrawRect = SaveRect;
	return rtn;
} 

int GWPolylineScreen(HDC hDC, HPFPOINT Points, long npnts, int idesc)
{
	HANDLE	hPoints = GSSiGlobAlloc(0, GMEM_MOVEABLE, npnts*sizeof(DPOINT) + 4);
	HPDPOINT	SPoints = (HPDPOINT)GlobalLock(hPoints);
	int		rtn;
	long	i;

	for (i = 0; i<npnts; i++)
	{
		DPOINT	p;
		p.x = Points[i].x;
		p.y = Points[i].y;
		ProjectFilePtD(&p);
		SPoints[i] = FilePtToBasePtD(p);
	}
	rtn = GWPolylineScreen2(hDC, SPoints, npnts, idesc);
	GSSiGlobUlFree(&hPoints);
	return rtn;
}
int GWPolylineScreenI(HDC hDC, HPPOINT Points, long npnts, int idesc)
{
	HANDLE	hPoints = GSSiGlobAlloc(0, GMEM_MOVEABLE, npnts*sizeof(DPOINT) + 4);
	HPDPOINT	SPoints = (HPDPOINT)GlobalLock(hPoints);
	int		rtn;
	long	i;

	for (i = 0; i<npnts; i++)
	{
		DPOINT	p;
		p.x = Points[i].x;
		p.y = Points[i].y;
		ProjectFilePtD(&p);
		SPoints[i] = FilePtToBasePtD(p);
	}
	rtn = GWPolylineScreen2(hDC, SPoints, npnts, idesc);
	GSSiGlobUlFree(&hPoints);
	return rtn;
}
int GWPolylineScreenS(HDC hDC, HPPOINTS Points, long npnts, int idesc)
{
	HANDLE	hPoints = GSSiGlobAlloc(0, GMEM_MOVEABLE, npnts*sizeof(DPOINT) + 4);
	HPDPOINT	SPoints = (HPDPOINT)GlobalLock(hPoints);
	int		rtn;
	long	i;

	for (i = 0; i<npnts; i++)
	{
		DPOINT	p;
		p.x = Points[i].x;
		p.y = Points[i].y;
		ProjectFilePtD(&p);
		SPoints[i] = FilePtToBasePtD(p);
	}
	rtn = GWPolylineScreen2(hDC, SPoints, npnts, idesc);
	GSSiGlobUlFree(&hPoints);
	return rtn;
}

int DisplayScreenLineSegment (HDC hDC, HPFPOINT SPoints,long n,int idesc,int w4)
{
    COLORREF	Color=RGB(255,255,255), OutlineColor=4;   
    HPEN	hPen, OldPen; 
    int		rtn=0,  w=w4;     
    LOGPEN	LogPen;
    
    if (SymbolIsLayered (abs(idesc)))
    	_fmemset (CurStreetNumbers,0,sizeof(CurStreetNumbers));
	if (ItemIsHighlighted)
	{
		w = HighlightWidth;
		Color = HighlightColor;
	}
	else if (SpecialThisItem)
	{
		w = SpecialWidth;
		Color = SpecialColor;
	}
    else if (HaveVarFillColor)
	{
    	Color = ConvertColor(GlobalColors[0],idesc);
	}
    else if (CurView->NewObjectMap[abs(idesc)]) 
    {   
		LPNEWOBJECT	pObj=&CurView->NewObject[CurView->NewObjectMap[abs(idesc)]-1]; 
		
		Color = RGB (pObj->R,pObj->G,pObj->B);
    }
    else
    	Color = GetSymbolColor (abs(idesc));
//    if (GetObject (OldPen,sizeof(LOGPEN),(LPSTR)&LogPen))
//    	Color = LogPen.lopnColor;
	if (!GetTypeVisibility (9))
		w = 1;
	if (FillStreetWithThisColor > 0)
	{
		OutlineColor = Color;
		Color = FillStreetWithThisColor;
	}
    else if (!Color) 
    {
    	Color = RGB(255,255,255); 
    	OutlineColor = 4;
    }
    else
    	OutlineColor = 4;//Color;
	Color = ConvertColor(Color,idesc);
	OutlineColor = ConvertColor(OutlineColor,idesc);
    if (AddToStreetSegmentList (SPoints,n,w,CurThemeClass,Color,OutlineColor))
		goto Exit;
    if (idesc < 0)
    	goto AddToFile;
    if (UseFlatEndPolyline)
		rtn = FlatEndPolyline (hDC,SPoints,n,w,ConvertColor(OutlineColor,CurTheme->UseHalfTone));  
	else   
	{
	    hPen = CreatePen (PS_SOLID,IDNINT(w/**DeviceToScreenFactor()*/),ConvertColor(OutlineColor,-1));  
	    OldPen = SelectObject (hDC,hPen);  
		rtn = BigFPolyline (hDC, SPoints, n,w); 
	    SelectObject (hDC,OldPen);
	    GSSiDeleteObject (&hPen);  
	}
AddToFile:
    if (FidHollowLines == HFILE_ERROR)
    {    
    	LPSTR	pFile;
    	
    	if (!hHollowLinesFile) 
    	{
    		hHollowLinesFile = GSSiGlobAlloc (0,GHND,256);
    		pFile = GlobalLock (hHollowLinesFile);
   			GSSiGetTempFileName (0,"gm",0,pFile);
   		}
   		else
   			pFile = GlobalLock (hHollowLinesFile);
   		FidHollowLines	 = GSSiOpenFile (pFile,0,OF_CREATE); 
   		GlobalUnlock (hHollowLinesFile);
   	}

    if (FidHollowLines != HFILE_ERROR)
    {
	    HollowLineHeader.npnts = n;
	    HollowLineHeader.width = w;
	    HollowLineHeader.desc = idesc;
		HollowLineHeader.order = CurThemeClass;
	    HollowLineHeader.OneWay = StreetOneWay;
	    HollowLineHeader.color = ConvertColor(Color,-1);
	    BigWrite (FidHollowLines,(HPSTR)&HollowLineHeader,sizeof(HollowLineHeader),-1);
	    BigWrite (FidHollowLines,(HPSTR)SPoints,n*sizeof(FPOINT),-1);  
	}
Exit:
	return rtn;
} 

/*int GWPolyline3 (HDC hDC, HPPOINTS Points, long npnts,int idesc)    
#if ENABLETRACE
{GSSiEnterProg (1438);
#endif
{
    int	rtn;

	if (UseShortSymbols)// && (!SymbolIsSolidLine (idesc) || GetSymbolWidth (idesc) > 1))
	{
	    HANDLE	hPoints = GSSiGlobAlloc (0,GMEM_MOVEABLE,abs(npnts)*sizeof(DPOINT));
	    HPDPOINT	DPoints=(HPDPOINT)GlobalLock (hPoints);
	    ULONG	i; 
	    
	    for (i=0;i<abs(npnts);i++)
		{
			POINT	p=POINTStoPOINT(Points[i]);
			//ProjectFilePt (&p);
	    	DPoints[i] = FilePtToBasePt (p);
		}
		rtn = GWPolylineD (hDC, DPoints, npnts,idesc); 
		GSSiGlobUlFree (&hPoints);
	}  
	else
		rtn = GWPolylineShort (hDC,Points,abs(npnts),idesc);
{
#if ENABLETRACE
GSSiExitProg (1438);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}*/ 

int GWPolyline (HDC hDC, HPPOINTS Points, long npnts,int idesc)    
#if ENABLETRACE
{GSSiEnterProg (1384);
#endif
{
	int	rtn;

	//if (CurView->Rotation)
	{
		HANDLE	Handle = GSSiGlobAlloc ( 756,GMEM_MOVEABLE,(long)abs(npnts)*sizeof(DPOINT));
		HPDPOINT pPolyPointsD = (HPDPOINT) GlobalLock (Handle);
		UINT	i;

		for (i=0;i<abs(npnts);i++)
			pPolyPointsD[i] = FilePtToBasePt (POINTStoPOINT(Points[i]));   
		SaveDC (hDC);
		SetDisplayMode (hDC, GF_TEXTMODE);
		rtn = GWPolylineD (hDC, pPolyPointsD, npnts,idesc); 
		GSSiGlobUlFree (&Handle);
		RestoreDC (hDC,-1);
	}
	//else
	//	rtn = GWPolyline3 (hDC,Points, npnts,idesc);
{
#if ENABLETRACE
GSSiExitProg (1384);
#endif
	return rtn;
}

#if ENABLETRACE
}
#endif
} 

/*int GWPolylineShort (HDC hDC, HPPOINTS lpPoints, long npnts,int idesc)
#if ENABLETRACE
{GSSiEnterProg (984);
#endif
{
	HPPOINTS	lpNewPoints, lpPntNew;
	DWORD	i,nNewPts; 
	

	if (!Display || !hDC || abs(npnts) < 2 || ShowBadSyms)
{
#if ENABLETRACE
GSSiExitProg (984);
#endif
		return (0);
}
//    if (!GetTypeVisibility(8) && SymIsInvis (CurrentDesc))
//    	return (0);
//	if (CurVis && CurVis->WantType[9])
	TotPointsProcessed += npnts;  
	if (StreetCenterline)
	{
		LPTHEME	SaveTheme = CurTheme;
		
		CurTheme = StreetCenterline;

		i = GWPolylineScreenS (hDC,lpPoints,abs(npnts),idesc);
		CurTheme = SaveTheme;
		goto Exit;
	}
	if (!CurView->FileFactor)
		CurView->FileFactor=1; 
	if (!CurView->FileProjectionType && (FileMode || CurView->FileFactor <= 1))
	{
		if (DoGraphics && abs(npnts) > 1)
			i = BigSPolyline (hDC,lpPoints,npnts,0);
			if (!i)
				i=1;
	}
	else if (CurView->FileFactor == 1)
	{
		lpNewPoints = (HPPOINTS)pCommonMem;
		nNewPts=1;  
		lpPntNew = lpNewPoints;
		ProjectFilePtS (lpPoints);
		lpPntNew[0] = *lpPoints;
		lpPoints++;
		for (i=1;i<npnts;i++,lpPoints++)
		{   
			ProjectFilePtS (lpPoints);
			lpPntNew[nNewPts] = *lpPoints;
			if (!SamePointS (lpPntNew[nNewPts],lpPntNew[nNewPts-1]))
				nNewPts++;
		}  
		if (DoGraphics && npnts > 1)
			i = BigSPolyline (hDC,lpNewPoints,nNewPts,0);
	}
	else 
	{
		lpNewPoints = (HPPOINTS)pCommonMem;
		nNewPts=1;  
		lpPntNew = lpNewPoints;
		ProjectFilePtS (lpPoints);
		lpPntNew->x = lpPoints->x / CurView->FileFactor;
		lpPntNew->y = lpPoints->y / CurView->FileFactor;  
		lpPoints++;
		for (i=1;i<npnts;i++,lpPoints++)
		{   
			if (CurView->FileProjectionType)
				ProjectFilePtS (lpPoints);
			lpPntNew[nNewPts].x = lpPoints->x / CurView->FileFactor;
			lpPntNew[nNewPts].y = lpPoints->y / CurView->FileFactor;  
			if (lpPntNew[nNewPts].x != lpPntNew[nNewPts-1].x || lpPntNew[nNewPts].y != lpPntNew[nNewPts-1].y)
			//if (!SamePoint (lpPntNew[nNewPts],lpPntNew[nNewPts-1]))
				nNewPts++;
		}  
		if (DoGraphics && npnts > 1)
			i = BigSPolyline (hDC,lpNewPoints,nNewPts,0);
	} 
Exit:
{
#if ENABLETRACE
GSSiExitProg (984);
#endif
	return (i);
}

#if ENABLETRACE
}
#endif
} */

BOOL PointInWBoundsPTOL (HPDPOINT Point,LPMNMXCORD pWBounds)
#if ENABLETRACE
{GSSiEnterProg (985);
#endif
{    
	 BOOL	rtn=TRUE;
	 
	 if (Point->x - P_TOL > pWBounds->xmx ||
	     Point->y - P_TOL > pWBounds->ymx ||
	     Point->x + P_TOL < pWBounds->xmn ||
	     Point->y + P_TOL < pWBounds->ymn)
			rtn=FALSE;
{
#if ENABLETRACE
GSSiExitProg (985);
#endif
	 return rtn;
}
#if ENABLETRACE
}
#endif
}

int GetScreenInt (LPDPOINT pPoint1,LPDPOINT pPoint2,LPDOUBLE pIntDist)
#if ENABLETRACE
{GSSiEnterProg (986);
#endif
{   
	short	nint=0;
	double	AZ, AZ2;
	double	linelen;
	double	SavePTOL = P_TOL; 
	MNMXCORD	Bounds,ScreenBounds;
	double	maxptol;
	DPOINT	IntPoints[4];
	LPDPOINT	pIntPoints=IntPoints;
	
	Bounds.xmn = min (pPoint1->x,pPoint2->x);
	Bounds.xmx = max (pPoint1->x,pPoint2->x);
	Bounds.ymn = min (pPoint1->y,pPoint2->y);
	Bounds.ymx = max (pPoint1->y,pPoint2->y);
    RectToBounds (&CurView->ScreenRect,&ScreenBounds);  
	if (!BoundsInBounds (&Bounds,&ScreenBounds,1)) 
{
#if ENABLETRACE
GSSiExitProg (986);
#endif
		return 0;
}    
	linelen = ldistp (*pPoint1,*pPoint2);
	AZ = getazd (pPoint1,pPoint2);   
	maxptol  = 0.1 * min (ScreenBounds.xmx - ScreenBounds.xmn,ScreenBounds.ymx - ScreenBounds.ymn);
	P_TOL = min (P_TOL,maxptol);
	if (LINSEC (pPoint1->x,pPoint1->y,AZ,
				ScreenBounds.xmn,ScreenBounds.ymn,HALFPI,
				&pIntPoints->x,&pIntPoints->y) == 1)
	{   
		if (PointInWBoundsPTOL (pIntPoints,&ScreenBounds) &&
			ldistp (*pIntPoints,*pPoint1) <= linelen && 
			ldistp (*pIntPoints,*pPoint2) <= linelen)
		{
			pIntDist[nint++] = ldistp (*pPoint1,*pIntPoints);
			pIntPoints++;
		}
	}
	if (LINSEC (pPoint1->x,pPoint1->y,AZ,
				ScreenBounds.xmn,ScreenBounds.ymn,0,
				&pIntPoints->x,&pIntPoints->y) == 1)
		if (PointInWBoundsPTOL (pIntPoints,&ScreenBounds) &&
			ldistp (*pIntPoints,*pPoint1) <= linelen && 
			ldistp (*pIntPoints,*pPoint2) <= linelen)
		{
			pIntDist[nint++] = ldistp (*pPoint1,*pIntPoints);
			pIntPoints++;
		}
	if (LINSEC (pPoint1->x,pPoint1->y,AZ,
				ScreenBounds.xmx,ScreenBounds.ymx,HALFPI,
				&pIntPoints->x,&pIntPoints->y) == 1)
		if (PointInWBoundsPTOL (pIntPoints,&ScreenBounds) &&
			ldistp (*pIntPoints,*pPoint1) <= linelen && 
			ldistp (*pIntPoints,*pPoint2) <= linelen)
		{
			pIntDist[nint++] = ldistp (*pPoint1,*pIntPoints);
			pIntPoints++;
		}
	if (LINSEC (pPoint1->x,pPoint1->y,AZ,
				ScreenBounds.xmx,ScreenBounds.ymx,0,
				&pIntPoints->x,&pIntPoints->y) == 1)
		if (PointInWBoundsPTOL (pIntPoints,&ScreenBounds) &&
			ldistp (*pIntPoints,*pPoint1) <= linelen && 
			ldistp (*pIntPoints,*pPoint2) <= linelen)
		{
			pIntDist[nint++] = ldistp (*pPoint1,*pIntPoints);
			pIntPoints++;
		}
	P_TOL = SavePTOL; 
/*	if (nint == 2) //retain orig az
	{ 
		AZ2=getazd (&pIntPointsOrig[0],&pIntPointsOrig[1]);
		if (fabs (DeltaAZ (AZ,AZ2)) > 1)
		{
			DPOINT SavePt=pIntPointsOrig[0];
			
			pIntPointsOrig[0] = pIntPointsOrig[1];
			pIntPointsOrig[1] = SavePt;
		}	
	}*/
{
#if ENABLETRACE
GSSiExitProg (986);
#endif
	return nint;
}
#if ENABLETRACE
}
#endif
}



short GetWBoundsInt (LPDPOINT pPoint1,LPDPOINT pPoint2,LPDPOINT pIntPoints)
#if ENABLETRACE
{GSSiEnterProg (986);
#endif
{   
	LPDPOINT pIntPointsOrig = pIntPoints;
	short	nint=0;
	double	AZ, AZ2;
	double	linelen;
	double	SavePTOL = P_TOL; 
	MNMXCORD	Bounds;
	double	maxptol;
	
	Bounds.xmn = min (pPoint1->x,pPoint2->x);
	Bounds.xmx = max (pPoint1->x,pPoint2->x);
	Bounds.ymn = min (pPoint1->y,pPoint2->y);
	Bounds.ymx = max (pPoint1->y,pPoint2->y);
	if (!BoundsInBounds (&Bounds,&CurView->WBounds,1)) 
{
#if ENABLETRACE
GSSiExitProg (986);
#endif
		return 0;
}    
	linelen = ldistp (*pPoint1,*pPoint2);
	AZ = getazd (pPoint1,pPoint2);   
	maxptol  = 0.1 * min (CurView->WBounds.xmx - CurView->WBounds.xmn,CurView->WBounds.ymx - CurView->WBounds.ymn);
	P_TOL = min (P_TOL,maxptol);
	if (LINSEC (pPoint1->x,pPoint1->y,AZ,
				CurView->WBounds.xmn,CurView->WBounds.ymn,HALFPI,
				&pIntPoints->x,&pIntPoints->y) == 1)
	{   
		if (PointInWBoundsPTOL (pIntPoints,&CurView->WBounds) &&
			ldistp (*pIntPoints,*pPoint1) <= linelen && 
			ldistp (*pIntPoints,*pPoint2) <= linelen)
		{
			nint++;
			pIntPoints++;
		}
	}
	if (LINSEC (pPoint1->x,pPoint1->y,AZ,
				CurView->WBounds.xmn,CurView->WBounds.ymn,0,
				&pIntPoints->x,&pIntPoints->y) == 1)
		if (PointInWBoundsPTOL (pIntPoints,&CurView->WBounds) &&
			ldistp (*pIntPoints,*pPoint1) <= linelen && 
			ldistp (*pIntPoints,*pPoint2) <= linelen)
		{
			nint++;
			pIntPoints++;
		}
	if (LINSEC (pPoint1->x,pPoint1->y,AZ,
				CurView->WBounds.xmx,CurView->WBounds.ymx,HALFPI,
				&pIntPoints->x,&pIntPoints->y) == 1)
		if (PointInWBoundsPTOL (pIntPoints,&CurView->WBounds) &&
			ldistp (*pIntPoints,*pPoint1) <= linelen && 
			ldistp (*pIntPoints,*pPoint2) <= linelen)
		{
			nint++;
			pIntPoints++;
		}
	if (LINSEC (pPoint1->x,pPoint1->y,AZ,
				CurView->WBounds.xmx,CurView->WBounds.ymx,0,
				&pIntPoints->x,&pIntPoints->y) == 1)
		if (PointInWBoundsPTOL (pIntPoints,&CurView->WBounds) &&
			ldistp (*pIntPoints,*pPoint1) <= linelen && 
			ldistp (*pIntPoints,*pPoint2) <= linelen)
		{
			nint++;
			pIntPoints++;
		}
	P_TOL = SavePTOL; 
	if (nint == 2) //retain orig az
	{ 
		AZ2=getazd (&pIntPointsOrig[0],&pIntPointsOrig[1]);
		if (fabs (DeltaAZ (AZ,AZ2)) > 1)
		{
			DPOINT SavePt=pIntPointsOrig[0];
			
			pIntPointsOrig[0] = pIntPointsOrig[1];
			pIntPointsOrig[1] = SavePt;
		}	
	}
{
#if ENABLETRACE
GSSiExitProg (986);
#endif
	return nint;
}
#if ENABLETRACE
}
#endif
}

short GetBoundsInt (LPPOINT pPoint1,LPPOINT pPoint2,LPDPOINT pIntPoints)
#if ENABLETRACE
{GSSiEnterProg (986);
#endif
{   
	LPDPOINT pIntPointsOrig = pIntPoints;
	short	nint=0;
	double	AZ, AZ2;
	double	linelen;
	double	SavePTOL = P_TOL; 
	MNMXCORL	Bounds;
	double	maxptol;  
	DPOINT	Point1, Point2;
	
	Bounds.xmn = min (pPoint1->x,pPoint2->x);
	Bounds.xmx = max (pPoint1->x,pPoint2->x);
	Bounds.ymn = min (pPoint1->y,pPoint2->y);
	Bounds.ymx = max (pPoint1->y,pPoint2->y);
	if (!Bounds4InBounds4 (&Bounds,&CurView->Bounds,1)) 
{
#if ENABLETRACE
GSSiExitProg (986);
#endif
		return 0;
}   
	Point1 = FilePtToBasePt(*pPoint1);
	Point2 = FilePtToBasePt(*pPoint2);
	linelen =ldistp (Point1,Point2);
	AZ = getazd (&Point1,&Point2);   
	maxptol  = 0.1 * min (CurView->WBounds.xmx - CurView->WBounds.xmn,CurView->WBounds.ymx - CurView->WBounds.ymn);
	P_TOL = min (P_TOL,maxptol);
	if (LINSEC (Point1.x,Point1.y,AZ,
				CurView->WBounds.xmn,CurView->WBounds.ymn,HALFPI,
				&pIntPoints->x,&pIntPoints->y) == 1)
	{   
		if (PointInWBoundsPTOL (pIntPoints,&CurView->WBounds) &&
			ldistp (*pIntPoints,Point1) <= linelen && 
			ldistp (*pIntPoints,Point2) <= linelen)
		{
			nint++;
			pIntPoints++;
		}
	}
	if (LINSEC (Point1.x,Point1.y,AZ,
				CurView->WBounds.xmn,CurView->WBounds.ymn,0,
				&pIntPoints->x,&pIntPoints->y) == 1)
		if (PointInWBoundsPTOL (pIntPoints,&CurView->WBounds) &&
			ldistp (*pIntPoints,Point1) <= linelen && 
			ldistp (*pIntPoints,Point2) <= linelen)
		{
			nint++;
			pIntPoints++;
		}
	if (LINSEC (Point1.x,Point1.y,AZ,
				CurView->WBounds.xmx,CurView->WBounds.ymx,HALFPI,
				&pIntPoints->x,&pIntPoints->y) == 1)
		if (PointInWBoundsPTOL (pIntPoints,&CurView->WBounds) &&
			ldistp (*pIntPoints,Point1) <= linelen && 
			ldistp (*pIntPoints,Point2) <= linelen)
		{
			nint++;
			pIntPoints++;
		}
	if (LINSEC (Point1.x,Point1.y,AZ,
				CurView->WBounds.xmx,CurView->WBounds.ymx,0,
				&pIntPoints->x,&pIntPoints->y) == 1)
		if (PointInWBoundsPTOL (pIntPoints,&CurView->WBounds) &&
			ldistp (*pIntPoints,Point1) <= linelen && 
			ldistp (*pIntPoints,Point2) <= linelen)
		{
			nint++;
			pIntPoints++;
		}
	P_TOL = SavePTOL; 
	if (nint == 2) //retain orig az
	{ 
		AZ2=getazd (&pIntPointsOrig[0],&pIntPointsOrig[1]);
		if (fabs (DeltaAZ (AZ,AZ2)) > 1)
		{
			DPOINT SavePt=pIntPointsOrig[0];
			
			pIntPointsOrig[0] = pIntPointsOrig[1];
			pIntPointsOrig[1] = SavePt;
		}	
	}
{
#if ENABLETRACE
GSSiExitProg (986);
#endif
	return nint;
}
#if ENABLETRACE
}
#endif
}

BOOL EliminateDupPoints (long np,HPFPOINT pPoints)
#if ENABLETRACE
{GSSiEnterProg (987);
#endif
{   
	long	CurWidth=0, CurWidthPlus1=0;//CurWidth+1;
	HPFPOINT	pLastPoint, lpPntNew;
	return FALSE;
	if (np<1)
{
#if ENABLETRACE
GSSiExitProg (987);
#endif
		return FALSE;
}
	else
	{
		pLastPoint = lpPntNew = pPoints + (np-1);
		lpPntNew++;
		if (abs (IDNINT(lpPntNew->x) - IDNINT(pLastPoint->x)) > CurWidthPlus1 ||
			abs (IDNINT(lpPntNew->y) - IDNINT(pLastPoint->y)) > CurWidthPlus1)
{
#if ENABLETRACE
GSSiExitProg (987);
#endif
			return FALSE;
}
	}   
{
#if ENABLETRACE
GSSiExitProg (987);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

int GetLineElementWidthAndColor (LPSYMBOL lpSym,LPELEMENT pElement,LPCOLORREF pColor,LPDOUBLE pSymFactor)
{
	int	Width;
	int	WantWidth = 1;
	double	LineWidthFactor = *pSymFactor * ThemeWidthFactor;
	short	UseHalfTone = lpSym->Number;
	COLORREF	LineColor=ConvertColor(0,UseHalfTone);

	if (CurVis && !ItemIsHighlighted)
		WantWidth = CurVis->WantType[9];
	if (pElement->Type == SVLINE)
	{
		double	Fac;
		 
		Width = pElement->Width;  
		if (TempLineWidth)
			Width = TempLineWidth;
		if (WantWidth)
		{
			if (!ItemSymbolWidth) 
			{
				if (Width > 256)
					Width = 1;  
				if (lpSym->BaseScale == 15)
					Width = ((lpSym->VSize * Width)/100) * FTM * BaseDistToWinDist * PenWidthFactor;
				else if (Width < 0)
					Width = -Width * FTM * BaseDistToWinDist * PenWidthFactor * LineWidthFactor;
				else
				{ 
					Fac = LineWidthFactor * PenWidthFactor;
					Width = IDNINTPOS (Width * Fac); 
				}
			}
			else if (ItemSymbolWidth > 0)
				Width = ItemSymbolWidth * PenWidthFactor * DeviceToScreenFactor();
			else
				Width = -ItemSymbolWidth * BaseDistToWinDist * PenWidthFactor;
		}
		else
			Width = 1;//7/1/2005 DeviceToScreenFactor();
		if (pElement->LineColor < -10)
			LineColor = 0; 
		else
			LineColor = pElement->LineColor;
		if (HighlightThisItem)
		{
			if (HighlightWidth < 0)
				Width = IDNINT(-HighlightWidth * BaseDistToWinDist);
			else
				Width = abs(HighlightWidth) * DeviceToScreenFactor(); 
			if (HighlightThisItem == 1 || !CurView)
				LineColor = HighlightColor;
			else   
				LineColor = ConvertColor(CurView->BackGroundColor,-1);
		}
		else if (SpecialThisItem)
		{
			Width = abs(SpecialWidth) * DeviceToScreenFactor(); 
			LineColor = SpecialColor;
		}
		else
		{
			if (CurView && CurView->NewObjectMap[lpSym->Number])
			{   
				short	iobj=CurView->NewObjectMap[lpSym->Number]-1;
				UseHalfTone=0;
				if (CurView->NewObject[iobj].Width<0)
	        		Width = (short)IDNINT((double)-CurView->NewObject[iobj].Width / CurView->BaseUnitsPerPixel); 
				else 
					Width = IDNINT((CurView->NewObject[iobj].Width+1) * PenWidthFactor * DeviceToScreenFactor());
				
				if (CurView->NewObjectMap[lpSym->Number] < CurView->HalfToneNewObjectStart)
					UseHalfTone = -1;
				LineColor = RGB(CurView->NewObject[iobj].R,
								CurView->NewObject[iobj].G,
								CurView->NewObject[iobj].B);
			}
			else if (pElement->LineColorType == SVVARCOLOR && HaveVarFillColor)
			{    
				LineColor = ColorWOWidth (GlobalColors[0]);  
				if (GetWValue(GlobalColors[0]))
					Width = IDNINT(GetWValue(GlobalColors[0]) * PenWidthFactor * DeviceToScreenFactor());
			}
			else if (TempLineColor >= 0)    
				LineColor = TempLineColor;
			else if (pElement->LineColor < 0)
				LineColor = GlobalColors[labs(pElement->LineColor)];  
			LineColor = ConvertColor(LineColor,UseHalfTone); 
		}
	}
	else
		Width = 1;
	*pColor = LineColor;
	return max (1,Width);
}

BOOL GetSymbolDashPattern (LPSYMBOL lpSym,LPINT pWidth,LPINT pnStyle,LPDWORD Style,LPDOUBLE pSymFactor,LPCOLORREF pLineColor)
{
	double		SymFactor = DefaultSymbolFactor * (*pSymFactor);
	double		VSymFactor = lpSym->VSize * SymFactor, HSymFactor = lpSym->HSize * SymFactor;
	int	i;
	LPELEMENT	pElement;
	LPHANDLE	phElement;
	int			Width=1, Last=0, j;
	
	*pnStyle = 0;
	phElement=&lpSym->hElement;
	for (i=0;i<lpSym->NumElements;i++,phElement++)
	{ 
		pElement = (LPELEMENT)GlobalLock (*phElement);  
		if (pElement->Squared && pElement->NumVectors == 2)
		{
			LPVECTOR pVector1=&pElement->Vector;
			LPVECTOR pVector2=pVector1+1;
			
			if (pVector1->Type != 1 || pVector2->Type != 1 ||
				pVector1->AZM > 0.000001 || pVector2->AZM > 0.000001)
			{
				GlobalUnlock (*phElement);
				return FALSE;
			}
			Width = max (Width,GetLineElementWidthAndColor (lpSym,pElement,pLineColor,pSymFactor));
			if (pVector1->Dist * HSymFactor >= 1)
			{
				if (!i)
					Style[(*pnStyle)++] = 0;
				j = IDNINT (pVector1->Dist * HSymFactor * BaseDistToWinDist);
				Style[(*pnStyle)++] = max (1,j - Last);
				Last = j;
			}
			j = IDNINT (pVector2->Dist * HSymFactor * BaseDistToWinDist);
			Style[(*pnStyle)++] = max (1,j - Last);
			Last = j;
		}
		GlobalUnlock (*phElement);
	}
	*pWidth = Width;
	if (*pnStyle > 2 && *pnStyle % 2)
		Style[(*pnStyle)++] = Style[(*pnStyle)-2];
	for (i=0;i<*pnStyle;i++)
		if (Style[i])
			return TRUE;
	return FALSE;
}

int PlotLinearItem (HDC hDC,HPFPOINT lpPoints,long nPnts,int idescIn,HANDLE hSymbol,short nElementsToDisplay, HANDLE hElementToDisplay,BOOL WantPreSym,BOOL WantPostSym,double Factor)
#if ENABLETRACE
{GSSiEnterProg (988);
#endif
{   
	BOOL	Invis=FALSE, SingleLine=FALSE, Shadow=FALSE, DoDestroy=FALSE;
	int		ii; 
	HPFPOINT	lpEndPoint;
	LPSYMBOLATTRIBUTE	pSymAtt;
	LPSYMBOL	CurSymbol; 
	UINT	i; 
	short	CurElem=0, WantType, Type, idesc = abs (idescIn);
	double	SymDist=0, SymFac;   
	FPOINT	Point1, Point2;
	int		rtn=0; 
	BOOL	IsLastPoint,UseBmpSym=TRUE;
	double	BaseDistPerPixel, BaseDistPerInch;
	int		Width;
	int		nStyle;
	DWORD	Style[100];
	COLORREF	LineColor;
	
	if (!OpenSymDict (OF_READ))
{
#if ENABLETRACE
GSSiExitProg (988);
#endif
		return 0;  
}
	if (nPnts < 2)
{
#if ENABLETRACE
GSSiExitProg (988);
#endif
		return 0;
}
	if (!idesc && ShowBadSyms)
{
#if ENABLETRACE
GSSiExitProg (988);
#endif
		return 0;
}
	if (!hSymbol && hSymbolAttributes && idesc && idesc <= NumSymbols)
	{   
//		SetWindowText (hWndMain,"Step 1");
		pSymAtt = (LPSYMBOLATTRIBUTE)GlobalLock (hSymbolAttributes);
		Invis = pSymAtt[idesc-1].InVisible; 
		SingleLine = pSymAtt[idesc-1].SolidLine;  
		Shadow = pSymAtt[idesc-1].Shadow;  
		Type = pSymAtt[idesc-1].Type;
		GlobalUnlock (hSymbolAttributes);
		if (Type != 2)
			SingleLine = TRUE;  
		else if (ShowBadSyms) 
{
#if ENABLETRACE
GSSiExitProg (988);
#endif
			return 0;
}
		if (!CurVis)
{
#if ENABLETRACE
GSSiExitProg (988);
#endif
			return 0;
}
		if (Invis && !CurVis->WantType[8])
{
#if ENABLETRACE
GSSiExitProg (988);
#endif
			return 0;
}
	}
//	DeviceRes = GetDeviceCaps(hDC, LOGPIXELSX); 
//	SymFac = (double) DeviceRes;  
	if (!hSymbol && (Invis || !GetTypeVisibility(6))) 
{
#if ENABLETRACE
GSSiExitProg (988);                                      //lpPoints[1]
#endif
		return BigFPolyline (hDC,lpPoints,nPnts,ItemSymbolWidth);
}   
	if (!hSymbol)
	{
		DoDestroy = TRUE;
		hSymbol = GetDictSymDesc (idesc,0);
	}
	if (!hSymbol)	
{
#if ENABLETRACE
GSSiExitProg (988);
#endif
		return BigFPolyline (hDC,lpPoints,nPnts,ItemSymbolWidth);  
}
        
	CurSymbol = (LPSYMBOL)GlobalLock (hSymbol);  
	if (CurSymbol->NumElements == 1)
	{
		LPELEMENT	pElement=(LPELEMENT)GlobalLock (CurSymbol->hElement); 
		if (!pElement->NumVectors)
			SingleLine = TRUE;
		Shadow = pElement->Shadow;
		GlobalUnlock (CurSymbol->hElement);
	}
	SaveDC (hDC);
	if (SaveContourElev && !strnicmp (CurSymbol->Name,"LKC",3))
	{
		SaveContourElevPoints (hDC,CurSymbol->Name,lpPoints,nPnts);
	}
	if (idescIn > 0 && CurSymbol->Layered)
	{
		AddToLayeredSymbolList (lpPoints,nPnts,idesc);    
		if (!HighlightThisItem)
			goto Exit;
	}
	if (!Factor)
	{ 
//		SetWindowText (hWndMain,"Step 2");
//	    BaseDistPerPixel = (CurView->NewBounds.xmx - CurView->NewBounds.xmn)/
//	    				   ((long)CurView->DrawRect.right - (long)CurView->DrawRect.left);	
//		BaseDistPerInch = BaseDistPerPixel * DevicePixelsPerInch;  
		BaseDistPerInch = CurView->MetersPerPixel * DevicePixelsPerInch;   
		if (CurSymbol->BaseScale == 15)    
			SymFac = (((FTM * CurSymbol->VSize)/fabs (CurSymbol->SizePointV[0].y - CurSymbol->SizePointV[1].y)) / CurView->BaseUnitsPerPixel)/DefaultSymbolFactor;
		else
		{
			SymFac = FTM * SymbolScales[CurSymbol->BaseScale] / BaseDistPerInch;    //CurView->BaseUnitsPerPixel
			if (!CurSymbol->NoSizeLimit)
				SymFac = min (1,SymFac); 
			if (!SymFac)
				SymFac = DeviceToScreenFactor();
			else
			{ 
				SymFac *= DevicePixelsPerInch / 96;
			} 
		}
		SymFac *= LineSymbolFactor;
		//SymFac *= DeviceToScreenFactor(); 
	}
	else if (CurSymbol->BaseScale == 15 && CurSymbol->SizePointV[0].y - CurSymbol->SizePointV[1].y)
		SymFac = ((Factor*FTM * CurSymbol->VSize)/fabs (CurSymbol->SizePointV[0].y - CurSymbol->SizePointV[1].y)) /DefaultSymbolFactor;
	else
		SymFac = Factor/DefaultSymbolFactor;
	if (SingleLine)
	{   
		HPEN	hCurPen, hPen=0;
		
//		SetWindowText (hWndMain,"Step 3");
		if (CurSymbol->NumElements)
		{
//			SetWindowText (hWndMain,"Step 6");
			WantType = -5;    
/*			{
				char	mess[512];
				sprintf (mess,"%20.10g",SymDist);
				SetWindowText (hWndMain,mess);
			}*/
			hPen = DrawSymbolBetweenPoints (hDC,&lpPoints[0],&lpPoints[1],CurSymbol,&SymDist,&CurElem,&SymFac,&WantType,nElementsToDisplay,hElementToDisplay,FALSE);	
		}
		if (Shadow)
		{
			HPEN	hShadowPen, hCPen=hPen;
			LOGPEN	lPen;
			
			if (!hPen)
				hCPen = SelectObject (hDC,GetStockObject(BLACK_PEN));
			GetObject (hCPen,sizeof(LOGPEN),&lPen);
			if (!hPen)
				SelectObject (hDC,hCPen);
			hShadowPen = CreatePen (PS_SOLID,lPen.lopnWidth.x+2,RGB(255,255,255));
			hCurPen = SelectObject (hDC,hShadowPen);
			rtn = BigFPolyline (hDC,lpPoints,nPnts,ItemSymbolWidth); 
			SelectObject (hDC,hCurPen);
			GSSiDeleteObject (&hShadowPen);
		}
		if (hPen)
		{
//			SetWindowText (hWndMain,"Step 7");
			hCurPen = SelectObject (hDC,hPen);
			rtn = BigFPolyline (hDC,lpPoints,nPnts,ItemSymbolWidth); 
			SelectObject (hDC,hCurPen);
			GSSiDeleteObject (&hPen);
		}
		else 
			rtn = BigFPolyline (hDC,lpPoints,nPnts,ItemSymbolWidth); 
		goto Exit;
	}
//	GSSiTrace (CurSymbol->Name);
/*	if (UseBmpSym)
	{
		HPEN	hPen, OldPen;
		LOGBRUSH	lb;
		DWORD	rtn;
		HDIB	hDib;
		HBITMAP	hBM;
		LPBITMAPINFOHEADER pdib;

  	    hBM = LoadBitmap (hInst,MAKEINTRESOURCE(IDB_PENTEST));
		lb.lbStyle = BS_PATTERN;
		lb.lbColor = RGB(255,0,0);
		lb.lbHatch = (LONG)hBM;
		hPen = ExtCreatePen (PS_GEOMETRIC|PS_SOLID|PS_ENDCAP_FLAT|PS_JOIN_BEVEL,30,&lb,0,0);
		OldPen = SelectObject (hDC,hPen);
		BigFPolyline (hDC,lpPoints,nPnts,0); 
		SelectObject (hDC,OldPen);
		DeleteObject (hPen);
		DeleteObject (hBM);
	}*/
	UseBmpSym = FALSE;
	if (GetSymbolDashPattern (CurSymbol,&Width,&nStyle,Style,&SymFac,&LineColor))
	{
		HPEN	hPen, OldPen;
		LOGBRUSH	lb;
		DWORD	rtn;
		lb.lbStyle = BS_SOLID;
		lb.lbColor = LineColor;
		lb.lbHatch = 0;
		hPen = ExtCreatePen (PS_GEOMETRIC|PS_USERSTYLE|PS_ENDCAP_FLAT|PS_JOIN_BEVEL,Width,&lb,nStyle,Style);
	//	hPen = ExtCreatePen (PS_GEOMETRIC|PS_ENDCAP_FLAT|PS_JOIN_BEVEL,30,&lb,0,0);
		OldPen = SelectObject (hDC,hPen);
		BigFPolyline (hDC,lpPoints,nPnts,0); 
		SelectObject (hDC,OldPen);
		DeleteObject (hPen);
	}
	else if (CurSymbol->NumElements && CurSymbol->HSize)
	{
		int	MaxElem = 0;

//		SetWindowText (hWndMain,"Step 4");
	    lpEndPoint = lpPoints + 1;  
	    if (CurSymbol->Number == 109)
	    	ii=1;
	    if (WantPreSym)
	    	WantType = -1;
	    else
	    	WantType = -2;
		IsLastPoint = FALSE;
		for (i=1;i<nPnts;i++,lpEndPoint++,lpPoints++)
		{   
			Point1 = *lpPoints;
			Point2 = *lpEndPoint;
			IsLastPoint = !(i+1 < nPnts);
			DrawSymbolBetweenPoints (hDC,&Point1,&Point2,CurSymbol,&SymDist,&CurElem,&SymFac,&WantType,nElementsToDisplay,hElementToDisplay,IsLastPoint);			
			MaxElem = max (CurElem,MaxElem);
		}
		if (WantPostSym)
		{
			WantType = -3;
			DrawSymbolBetweenPoints (hDC,&Point1,&Point2,CurSymbol,&SymDist,&CurElem,&SymFac,&WantType,nElementsToDisplay, hElementToDisplay,FALSE);			
			MaxElem = max (CurElem,MaxElem);
		}
	}
	else
		BigFPolyline (hDC,lpPoints,nPnts,ItemSymbolWidth); 
Exit: 
 	GlobalUnlock (hSymbol);
 	RestoreDC (hDC,-1); 
 	if (DoDestroy)
		DestroySymbol (hSymbol); 
{
#if ENABLETRACE
GSSiExitProg (988);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

void DisplayPoint (HDC hDC,POINT Point)
{   
	RECT	Rect;
	int	psize=2;
	
    SaveDC (hDC);
	SetDisplayMode (hDC, GF_TEXTMODE);
	Rect.left = Point.x-psize;
	Rect.right = Point.x+psize;
	Rect.top = Point.y-psize;
	Rect.bottom = Point.y+psize;
	FillRectPoly(hDC,&Rect,RGB(255,0,0));
	RestoreDC (hDC,-1);
	return;
}
void RectAtPoint(HDC hDC, HPFPOINT pt, COLORREF color)
{
	RECT rect;
	rect.left = pt->x;
	rect.top = pt->y;
	rect.right = pt->x + 8;
	rect.bottom = pt->y + 8;
	FillRectColor(hDC, &rect, color);
}
int GWPolylineD (HDC hDC, HPDPOINT lpPoints, long npnts,int idesc)
#if ENABLETRACE
{GSSiEnterProg (989);
#endif
{
	BOOL showNodes = FALSE;
	if (idesc == -1)
	{
		showNodes = TRUE;
		idesc = 0;
	}
	HANDLE Handle;
	HPFPOINT	lpNewPoints, lpPntNew;   
	HPDPOINT lpPointsIn = lpPoints;
	DWORD	i;
	int	rtn=0;
	long	np=0, nSplinePoints=0, MaxSplinePoints=4096;  
	FPOINT	LastPoint; 
	BOOL	PinB, LastPinB=TRUE, WantPreSym=TRUE,WantPostSym=TRUE; 
	DPOINT	IntPoints[4],LastDPoint;
	MNMXCORD	SaveBounds;
	short	nint, j;  
//	BOOL	Spline=GetGlobalBVal2 ("[%SPLINE]",FALSE);
	HANDLE	hReverse=0;
	int		ii;
	static	int	debugi=10838;
//	HPDPOINT	pSplinePoints;   
//	double	d1,d2;
	double saveItemSymbolWidth = ItemSymbolWidth;
	
	if (PickingByRefno)
	{   
		npnts = labs (npnts);
		PickList[0].BeginPoint = lpPoints[0];
		PickList[0].EndPoint = lpPoints[npnts-1];
		PickList[0].NumPoints = npnts;
		PickList[0].Length = GetPolyLengthD (lpPoints,npnts);
{
#if ENABLETRACE
GSSiExitProg (989);
#endif
		return (0);
}   
	}
	TotPointsProcessed += abs(npnts);  
	if (!Display || !hDC || !npnts)
{
#if ENABLETRACE
GSSiExitProg (989);
#endif
		return (0);
}
//    if (!GetTypeVisibility(8) && SymIsInvis (CurrentDesc))
//    	retrn (0);  
/*	if (Spline)
	{   
		hSpline = GSSiGlobAlloc ( 767,GMEM_MOVEABLE,MaxSplinePoints * sizeof(DPOINT));
		pSplinePoints = (HPDPOINT)GlobalLock (hSpline);
		SplinePointsD (1,npnts,lpPoints,&nSplinePoints, pSplinePoints,CurView->BaseUnitsPerPixel,CurView->BaseUnitsPerPixel/4,0,MaxSplinePoints);
		npnts = nSplinePoints;
		lpPoints = pSplinePoints;
    }*/ 
	if (CurView && CurView->LineSymbolOveride)
		idesc = CurView->LineSymbolOveride;
	if (!idesc)
		ItemSymbolWidth = 0;
	if (npnts < 0)
	{   
		npnts = -npnts;  
		hReverse =  ReversePoints3 (npnts,lpPoints);
		lpPoints = (HPDPOINT)GlobalLock (hReverse);
    } 
	if (SymbolIsReversed (idesc))
	{
		hReverse =  ReversePoints3 (npnts,lpPoints);
		lpPoints = (HPDPOINT)GlobalLock (hReverse);
	}
	if (ShowLineDirection)
	{
		HANDLE hFPoints = GSSiGlobAlloc(0, GMEM_MOVEABLE, npnts * sizeof(FPOINT)+4);
		HPFPOINT FPoints = GlobalLock(hFPoints);
		for (i = 0; i<npnts; i++)
			FPoints[i] = BasePtToWinPtF(&lpPoints[i]);

		SaveDC(hDC);
		SetDisplayMode(hDC, GF_TEXTMODE);
		DrawOneWayArrows(hDC, 1, FPoints, npnts, 5);
		AAPolyLineF(hDC, FPoints, npnts, 0, 2);
		RestoreDC(hDC, -1);
		GSSiGlobUlFree(&hFPoints);
		goto Exit;
	}
	if (StreetCenterline && SymbolIsSolidLine (idesc))
	{   
		short	desc=idesc;
		HPDPOINT	lpPoints16;
		LPTHEME	SaveTheme = CurTheme;
		
		CurTheme = StreetCenterline;
		if (ShowHollowStreet != 1)
			desc = -desc;
		Handle = GSSiGlobAlloc ( 768,GMEM_MOVEABLE,((long)npnts+16L) * (long)sizeof(DPOINT));
		lpPoints16 = (HPDPOINT)GlobalLock (Handle);
		for (i=0;i<npnts;i++,lpPoints++)
			lpPoints16[i] = BasePtToWinPtD (lpPoints);   
	    rtn = GWPolylineScreen2 (hDC,lpPoints16,npnts,desc); 
	    GSSiGlobUlFree (&Handle);
		CurTheme = SaveTheme;
	    goto Exit; 
	}

 	SaveBounds = CurView->WBounds;
	CurView->WBounds = FactorBounds (&CurView->WBounds,1.15);
    SaveDC (hDC);
	SetDisplayMode (hDC, GF_TEXTMODE);
	Handle = GSSiGlobAlloc ( 768,GMEM_MOVEABLE,((long)npnts+16L) * (long)sizeof(FPOINT));
	lpPntNew = lpNewPoints = (HPFPOINT)GlobalLock (Handle);

//d1=ldistp (lpPoints[0],lpPoints[1]);
	for (i=0;i<npnts;i++,lpPoints++)
	{   
		if ((PinB = PointInWBounds (lpPoints)))
		{
			if (!LastPinB)
			{ 
				nint = GetWBoundsInt (&LastDPoint,lpPoints,IntPoints);
				for (j=0;j<nint;j++)
				{
					*lpPntNew = BasePtToWinPtF (&IntPoints[j]); 
				    if (!EliminateDupPoints (np,lpNewPoints))
				    {
				    	lpPntNew++;
				    	np++;
				    }  
				}
			} 
			*lpPntNew = BasePtToWinPtF (lpPoints); 
			if (showNodes)
			{
				if (np== textEpoint)
					RectAtPoint(hDC, lpPntNew, RGB(255, 0, 0));
				if (np== textBpoint)
					RectAtPoint(hDC, lpPntNew, RGB(0,255, 0));
			}
			LastPoint = *lpPntNew;
		    if (!EliminateDupPoints (np,lpNewPoints))
		    {
		    	lpPntNew++;
		    	np++;
		    }  
		}
		else if (i)
		{
			if (i==debugi)
				ii=1;
			nint = GetWBoundsInt (&LastDPoint,lpPoints,IntPoints);
			for (j=0;j<nint;j++)
			{
				*lpPntNew = BasePtToWinPtF (&IntPoints[j]); 
			    if (!EliminateDupPoints (np,lpNewPoints))
			    {
			    	lpPntNew++;
			    	np++;
			    }                                     // lpNewPoints[1]
			}
			if (DoGraphics && np > 1)
				PlotLinearItem (hDC,lpNewPoints,np,idesc,0,0,0,WantPreSym,FALSE,0);   
			WantPreSym = FALSE;
			np = 0;
			lpPntNew = lpNewPoints;
		}
		else
			WantPreSym = FALSE;
		LastDPoint = *lpPoints;
		LastPinB = PinB;
	}
	if (!PinB)
		WantPostSym = FALSE;   
//d2=ldistp (FPointToDPoint (lpNewPoints[0]),FPointToDPoint (lpNewPoints[1]));
	if (DoGraphics && np > 1)
		PlotLinearItem (hDC,lpNewPoints,np,idesc,0,0,0,WantPreSym,WantPostSym,0);  
	if (DisplayLineBPEP && DoGraphics)
	{   
		if (PointInWBounds (lpPointsIn))
			DisplayPoint (hDC,BasePtToWinPt (lpPointsIn)); 
		if (PointInWBounds (&lpPointsIn[npnts-1]))
			DisplayPoint (hDC,BasePtToWinPt (&lpPointsIn[npnts-1])); 
	}
	if (DisplayLinePoints)
	{   
		for (i=0;i<npnts;i++)
		{
			if (PointInWBounds (&lpPointsIn[i]))
				DisplayPoint (hDC,BasePtToWinPt (&lpPointsIn[i])); 
		}
	}
	GSSiGlobUlFree (&Handle);  
	GSSiGlobUlFree (&hReverse);
	RestoreDC (hDC,-1);
	if (StreetCenterline && (CurrentType == GF_LINE	|| CurrentType == GF_POLYLINE || CurrentType == GF_CURVE))
	{   
		HPFPOINT	lpPoints16;
		LPTHEME	SaveTheme = CurTheme;
		
		CurTheme = StreetCenterline;
		
		Handle = GSSiGlobAlloc ( 768,GMEM_MOVEABLE,((long)npnts+16L) * (long)sizeof(FPOINT));
		lpPoints16 = (HPFPOINT)GlobalLock (Handle); 
		lpPoints = lpPointsIn;
		for (i=0;i<npnts;i++,lpPoints++)
			lpPoints16[i] = BasePtToWinPtF (lpPoints);   
	    rtn = GWPolylineScreen2 (hDC,lpPoints16,npnts,-idesc); 
	    GSSiGlobUlFree (&Handle);
		CurTheme = SaveTheme;
	}
	CurView->WBounds = SaveBounds;
Exit:
	ItemSymbolWidth = saveItemSymbolWidth;
{
#if ENABLETRACE
GSSiExitProg (989);
#endif
	return (rtn);
}

#if ENABLETRACE
}
#endif
}



BOOL DisplaySymInRect2 (HDC hDC,HANDLE hSymbol,RECT Rect,short nElement,HANDLE hElement,double Factor,BOOL UseHalfTone) 
{
    LPVISLIST	SaveVis=CurVis;  
    HANDLE	hVisList=0;  
    short	width, height, type, symnum;
    POINT	Points[5];
	FPOINT	FPoints[5]; 
//	DPOINT	DPoints[5];
    double	hsize, SaveBDTWD=BaseDistToWinDist;
	LPSYMBOLATTRIBUTE	pSymAtt;
    LPSYMBOL    CurSymbol; 
    short	LineInRectOpt = GetGlobalLVal2 ("[%LINEINRECTOPT]",0);
    
    if (!hSymbol)
    	return FALSE;
	SaveDC (hDC);
	SetDisplayMode (hDC, GF_SCREENMODE);
	hVisList=GSSiGlobAlloc ( 487,GHND,sizeof(VISLIST));
	CurVis = (LPVISLIST)GlobalLock (hVisList); 
	CurVis->hVisList=hVisList;
	InitVis ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSymbol);    
//	SetWindowText (hWndMain,CurSymbol->Name);
	type = CurSymbol->Type; 
	symnum = CurSymbol->Number;  
	hsize = CurSymbol->HSize;
	GlobalUnlock (hSymbol);  
//	InflateRect (&Rect,-15,-15);
    width = Rect.right - Rect.left -2;
    Points[0].y=Points[1].y=FPoints[0].y=FPoints[1].y=(Rect.top + Rect.bottom)/2;  
	switch (CurSymbol->Type)                                     
	{   
		case 2:
		{
			short	np = 2;
	    	FPoints[0].x=Rect.left; 
	    	FPoints[1].x=Rect.right;
	    	switch (LineInRectOpt)
	    	{   
	    		case 1:
		    		np = 4;
		    		FPoints[0].y = Rect.bottom;
		    		FPoints[1].x = Rect.left + (Rect.right - Rect.left) / 3;
		    		FPoints[1].y = Rect.top;
		    		FPoints[2].x = Rect.left + 2 * (Rect.right - Rect.left) / 3;
		    		FPoints[2].y = Rect.bottom;
		    		FPoints[3].x = Rect.right;
		    		FPoints[3].y = Rect.top; 
		    		break;
	    		case 2:
		    		np = 2;
		    		FPoints[0].y = Rect.bottom;
		    		FPoints[1].x = Rect.right;
		    		FPoints[1].y = Rect.top; 
		    		break;
	    	} 
	    	height = width;  
	    	{
	    		double SaveDefaultSymbolFactor = DefaultSymbolFactor;
	    		
		    	if (!Factor && hsize && LineSymbolFactor) 
		    		BaseDistToWinDist = (width / hsize) / (LineSymbolFactor * 76.2); 
		    	else if (CurSymbol->BaseScale == 15)  
		    	{
	//	    		DefaultSymbolFactor = 1;
		    		BaseDistToWinDist = 1;
		    		Factor = 1;  
		    	}
		    	else
		    		BaseDistToWinDist = 1;  
				PlotLinearItem (hDC,FPoints,np,-symnum,hSymbol,nElement,hElement,TRUE,TRUE,Factor);
				DefaultSymbolFactor = SaveDefaultSymbolFactor;
			} 
    	}
    	break; 
    	case 3:
    	{   
    		if (!CurSymbol->InVisible && CurSymbol->NumElements)
    		{   
    			HPEN	hPen=0,hOldPen;
				HBRUSH	hBrush = 0, hOldBrush = GetStockObject(BLACK_BRUSH);
				int		BorderSymNum;
    			
/*    			LPELEMENT pElement = (LPELEMENT)GlobalLock (CurSymbol->hElement);
    			FillRectPoly (hDC,&Rect,ConvertColor(pElement->FillColor));
    			GlobalUnlock (CurSymbol->hElement);*/   
    			RectToPoints (&Rect,Points);
/*				nPnts = 4;
				nPoly = 0;
				HiPrecis = FALSE;
				CurView->FileFactor = 1;
				lpCurPoints = Points;
				CurrentDesc = CurSymbol->Number;
				ProcessPolygon (hDC,TRUE,PltType,0,0,0);*/
    			SetAreaPenAndBrush (hDC,CurSymbol,0,FALSE,TRUE,&hPen,&hBrush,&BorderSymNum);
			    if (hBrush)
					hOldBrush = SelectObject (hDC,hBrush);
				if (hPen) 
					hOldPen = SelectObject (hDC,hPen); 
				Polygon (hDC, Points, 4);  
			    if (hBrush)
					SelectObject (hDC,hOldBrush);
				if (hPen) 
					SelectObject (hDC,hOldPen); 
				if (hPen != GetStockObject (NULL_PEN))
					GSSiDeleteObject (&hPen);
				if (hBrush != GetStockObject(NULL_BRUSH))
					GSSiDeleteObject (&hBrush);
				if (BorderSymNum)
 	    		{
	    			double SaveDefaultSymbolFactor = DefaultSymbolFactor;

					int i, np = 5;
					RECT	Rect2=Rect;
	    			
		   			InflateRect (&Rect2,-1,-1);
					RectToPoints (&Rect2,Points);
					for (i=0;i<np;i++)
						FPoints[i] = PointToFPoint (Points[i]);
					FPoints[4] = FPoints[0];
		    		if (!Factor && hsize && LineSymbolFactor) 
		    			BaseDistToWinDist = (width / hsize) / (LineSymbolFactor * 76.2); 
		    		else if (CurSymbol->BaseScale == 15)  
		    		{
		//	    		DefaultSymbolFactor = 1;
		    			BaseDistToWinDist = 1;
		    			Factor = 1;  
		    		}
		    		else
		    			BaseDistToWinDist = 1;  
					PlotLinearItem (hDC,FPoints,np,-BorderSymNum,0,0,0,TRUE,TRUE,Factor);
					DefaultSymbolFactor = SaveDefaultSymbolFactor;
				} 
   			}
    	} 
    	break;
    	default:
	    {   
	    	double pfac=1;
	    	
	    	Points[0].x=(Rect.left + Rect.right)/2;
		    height = Rect.bottom - Rect.top -2; 
		    if (hSymbolAttributes)
		    {   long	div1, div2;
		   		pSymAtt = (LPSYMBOLATTRIBUTE)GlobalLock (hSymbolAttributes); 
		   		div1 = pSymAtt[symnum-1].Rect.right - pSymAtt[symnum-1].Rect.left; 
		   		div2 = pSymAtt[symnum-1].Rect.bottom - pSymAtt[symnum-1].Rect.top;
		   		if (div1 && div2)
			    	pfac = min(200e0 / div2,200e0 / div1);
			    GlobalUnlock (hSymbolAttributes);
			} 
			DisplayPointSymbol (hSymbol,hDC, -height*pfac/2, width*pfac/2,0, &Points[0],0,FALSE,nElement,hElement,FALSE,UseHalfTone,0,0);  
		}
	}
	GSSiGlobUlFree (&hVisList);
    CurVis = SaveVis; 
    BaseDistToWinDist = SaveBDTWD;
    RestoreDC (hDC,-1); 
    return TRUE;
}

BOOL DisplaySymInRect (HDC hDC,int SymNumIN,RECT Rect,double Factor,BOOL UseHalfTone)
{   
    HANDLE	hSymbol=0;
    BOOL	rtn=FALSE;
	int		SymNum=abs (SymNumIN);
	
	if (!SymNum)
		return FALSE; 
	Factor *= DefaultSymbolFactor;
	SaveDC (hDC);
	if (SymNumIN > 0)
	{
		hRgn = CreateRectRgn(Rect.left,Rect.top,Rect.right,Rect.bottom);
  		SelectClipRgn (hDC,hRgn);
  		DeleteObject(hRgn);
	}
	if (SymNumIN < 0)
	{
		rtn = DisplayCharInRect (hDC,SymNum,Rect);
	}
	else
	{
		if ((hSymbol = GetDictSymDesc (SymNum,0)))
			rtn = DisplaySymInRect2 (hDC,hSymbol,Rect,0,0,Factor,UseHalfTone);   
		DestroySymbol (hSymbol);
	} 
	RestoreDC (hDC,-1);
	return rtn;
}  

BOOL DisplayCharInRect (HDC hDC,int chr,RECT Rect)
{   
	LOGFONT	LogFont;    
   	HFONT	OldFont,hFont;
	char	txt[2];
	short	FontNum = chr / 1000;
	
	txt[0] = chr%1000;
	txt[1] = 0;

	_fmemset (&LogFont,0,sizeof(LOGFONT));
	LogFont.lfHeight = -min((Rect.right - Rect.left)/0.8,(Rect.bottom - Rect.top)); 
	LogFont.lfEscapement = 0;   
	LogFont.lfWeight = FW_NORMAL;    
	LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	LogFont.lfQuality = PROOF_QUALITY; 
	_fstrcpy (LogFont.lfFaceName,CurSymbolFont[FontNum]);  
	hFont = CreateFontIndirect((LPLOGFONT)&LogFont);
	OldFont = SelectObject (hDC,hFont);
	TextOut (hDC,Rect.left,Rect.top,txt,1);	
	SelectObject (hDC,OldFont);
	DeleteObject (hFont);
	return TRUE;
}
	
BOOL DisplayCharAtLoc (HDC hDC,POINT Point,int size,int chr)
{   
	LOGFONT	LogFont;    
   	HFONT	OldFont,hFont;
	char	txt[2];
	short	FontNum = chr / 1000;
	
	SaveDC (hDC);
	SetDisplayMode (hDC, GF_TEXTMODE);
	txt[0] = chr%1000;
	txt[1] = 0;

	_fmemset (&LogFont,0,sizeof(LOGFONT));
	LogFont.lfHeight = size; 
	LogFont.lfEscapement = 0;   
	LogFont.lfWeight = FW_NORMAL;    
	LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	LogFont.lfQuality = PROOF_QUALITY; 
	_fstrcpy (LogFont.lfFaceName,CurSymbolFont[FontNum]);  
	hFont = CreateFontIndirect((LPLOGFONT)&LogFont);
	OldFont = SelectObject (hDC,hFont);
	TextOut (hDC,Point.x-size/2,Point.y-size/2,txt,1);	
	SelectObject (hDC,OldFont);
	DeleteObject (hFont);
	RestoreDC (hDC,-1);
	return TRUE;
}

BOOL DisplayCharAtPoint (HDC hDC,POINT Point,double az,int size,char chr,int cno,int seqno,int Elev)
{   
	LOGFONT	LogFont;    
   	HFONT	OldFont,hFont;
	char	txt[2];
	int		angle;
	POINT	Point2;
	SIZE	txSize;
	int		twidth, theight;
	char	GridPrefix[10], GridUDI[16];
	static	int	Seq, LastGridID=-1;
	DPOINT	WPoint;
	BOOL	isUnified = GetGlobalBVal2 ("[%UNIFIED]",FALSE);
	
	if (seqno)
		Seq = seqno;
	txt[0] = chr;
	txt[1] = 0;

	if (az)	 
		angle  = IDNINT(3600-((LTWOPI(az)/RADDEG)*10));
	else
		angle = 0; 
	if (angle >= 3600)
		angle = 0;
	if (!hDC || SaveContourElev)
	{
		HANDLE	hDBSaveContourText;

		if ((hDBSaveContourText = GetOpenHandleFromID ("CONTTEXT",UMIFS_DATAFILE)))
		{
			LPGWDHEADER	lpGWDHead = (LPGWDHEADER)GlobalLock (hDBSaveContourText);
			if (isUnified)
			{
				LPSAVECONTEXT2	pSaveC = (LPSAVECONTEXT2)&lpGWDHead->GWDData; 
				char	QuadKey[22];
				int	pixelX, pixelY;

				GetGlobalCVal ("[%QUADKEY]",QuadKey,0);
				strncpy0 (pSaveC->QuadKey,QuadKey,20);
				pSaveC->depth = Elev;
				pSaveC->size = size;
				pSaveC->Seq	= Seq++;
				pSaveC->x = Point.x;
				pSaveC->y = Point.y;
				TileXYToPixelXY (CurrentGridCol,CurrentGridRow,&pixelX,&pixelY);
				pixelX += Point.x;
				pixelY += Point.y;
				PixelXYToLatLong(pixelX,pixelY,CurrentGridLevel,&pSaveC->latitude, &pSaveC->longitude);
				angle = (256 * angle)/3600;
				pSaveC->rot = angle;
				pSaveC->az = az;
			}
			else
			{
				LPSAVECONTEXT	pSaveC = (LPSAVECONTEXT)&lpGWDHead->GWDData; 

				GetGlobalCVal ("[GRIDPREFIX]",GridPrefix,0);
				GetGlobalCVal ("[GRIDCELLID]",GridUDI,0);
				pSaveC->GridID = atoi (GridUDI);
				if (pSaveC->GridID != LastGridID)
				{
					Seq = 0;
					LastGridID = pSaveC->GridID;
				}
				//WPoint = ScreenPtToBasePt (Point);
				pSaveC->Seq	= Seq++;
				pSaveC->CharNo = cno;
				pSaveC->chr[0] = chr;
				pSaveC->chr[1] = 0;
				if (stricmp (GridPrefix,"VECTOR"))
				{
					pSaveC->x = (10000 * (Point.x - CurView->ScreenRect.left))/(CurView->ScreenRect.right - CurView->ScreenRect.left);
					pSaveC->y = (10000 * (Point.y - CurView->ScreenRect.top))/(CurView->ScreenRect.bottom - CurView->ScreenRect.top);
				}
				else
				{
					pSaveC->x = Point.x;
					pSaveC->y = Point.y;
					angle = (256 * angle)/3600;
					pSaveC->GridID = Elev;
				}
				pSaveC->size = size;
				pSaveC->rot = angle;
				pSaveC->isym = CurrentDesc;
			}
			GWDAddRecord (lpGWDHead,0,0);
			GlobalUnlock (hDBSaveContourText);
			return TRUE;
		}
		return FALSE;
	}
	SaveDC (hDC);
	SetDisplayMode (hDC, GF_TEXTMODE);
	_fmemset (&LogFont,0,sizeof(LOGFONT));
	LogFont.lfHeight = size; 
	LogFont.lfEscapement = LogFont.lfOrientation = angle; 
	LogFont.lfWeight = FW_NORMAL;    
	LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	LogFont.lfQuality = PROOF_QUALITY; 
	_fstrcpy (LogFont.lfFaceName,"Arial Rounded MT Bold");  
	hFont = CreateFontIndirect((LPLOGFONT)&LogFont);
	OldFont = SelectObject (hDC,hFont);
	GetTextExtentPoint32 (hDC,txt,1,&txSize); 
	twidth = txSize.cx;
	theight = txSize.cy;
	Point2 = newpt (Point,az,twidth/2);
	Point2 = newpt (Point2,az+HALFPI,-theight/2);
	SetTextColor (hDC,0);
	TextOut (hDC,Point2.x,Point2.y,txt,1);	
	SelectObject (hDC,OldFont);
	DeleteObject (hFont);
	RestoreDC (hDC,-1);
	return TRUE;
}

BOOL DisplaySymInDlg (HWND hWndDlg,UINT Control,LPSTR SymName,COLORREF Color)
{
    short	symnum, nc; 
	BOOL	rtn=FALSE;
    HANDLE	hSymbol; 
    HBRUSH	hBrush, hOldBrush=0;
    HWND	hWnd=GetDlgItem(hWndDlg,Control);
    HDC		hDC = GetDC (hWnd);     
    BOOL	SaveHVFC = HaveVarFillColor;   
    double	SaveLSF = LineSymbolFactor;
    double	SaveGPF = GraphicsPointFactor;
    
	symnum = GetDictSymbolNumber (SymName); 
	if (symnum)
	{
		hSymbol = GetDictSymDesc (symnum,0);
		if (Color >= 0)
		{
			HaveVarFillColor = TRUE;  
			GlobalColors[0]=Color;
			hBrush = CreateSolidBrush(Color);  
			hOldBrush = SelectObject (hDC,hBrush);
		} 
		else
			HaveVarFillColor = FALSE;  
        
		LineSymbolFactor = -1;  
		GraphicsPointFactor = 1;
		DisplaySymInWindow (hWnd,hDC,hSymbol,0,0,hBackBrush1,-2); 
		DestroySymbol (hSymbol);    
		LineSymbolFactor = SaveLSF;
		HaveVarFillColor = SaveHVFC; 
		GraphicsPointFactor = SaveGPF;
		if (hOldBrush) 
		{
			GSSiDeleteObject (&hBrush);
			SelectObject (hDC,hOldBrush);
		}
		ReleaseDC (hWnd,hDC); 
		rtn = TRUE;
	}
	return rtn;
}
	
BOOL DisplaySymInWindow (HWND hWnd,HDC hDCin,HANDLE hSymbol,short nElement,HANDLE hElement,HBRUSH FillBrush,short Offset)
{   
	RECT	Rect;
	HDC hDC;  
    BOOL	rtn=FALSE;
    HRGN	hRgn;   
	HANDLE	hVP=0;
	LPVIEWPORT	pSaveVP;
    
    if (!hSymbol)
    	return FALSE;
	hVP = GSSiGlobAlloc (1880,GMEM_MOVEABLE,sizeof(VIEWPORT));
	pSaveVP = GlobalLock (hVP);
	*pSaveVP = *CurView;
	CurView->HalfTone = 0;
	CurView->ConvertToGray = FALSE;
	if (!hDCin)
		hDC = GetDC (hWnd);
	else
		hDC = hDCin;
	GetClientRect(hWnd,&Rect); 
	hRgn = CreateRectRgn(Rect.left,Rect.top,Rect.right,Rect.bottom);
  	SelectClipRgn (hDC,hRgn);
  	DeleteObject(hRgn);
	if (!FillBrush)
		FillBrush = GetStockObject(WHITE_BRUSH);
	FillRect (hDC,&Rect,FillBrush);
	InflateRect (&Rect,Offset,Offset);   
	rtn = DisplaySymInRect2 (hDC,hSymbol,Rect,nElement,hElement,DefaultSymbolFactor,FALSE);
	if (!hDCin)
		ReleaseDC (hWnd,hDC);  
	*CurView = *pSaveVP;
	GSSiGlobUlFree (&hVP);
	return rtn;
}
	
BOOL ListElements (HWND hWndDlg,UINT idclist, HANDLE hSymbol)
{   
	POINT	Point;
	RECT	Rect;
	HDC		hDC;  
	short	symnum, size;
	int		i,j;
    LPSYMBOL    pSym; 
    char	str[128];
	LPHANDLE	phElement;  
	double	MaxLength=0;
	char	Type[6][16]={"Text","Polyline","Polygon","Border","XHatch",""};
	char	NegType[4][16]={"PreSym","End","PostSym",""};
	
	SendDlgItemMessage (hWndDlg,idclist,LB_RESETCONTENT,0,0);
	pSym = (LPSYMBOL)GlobalLock (hSymbol);
	for (i=0,phElement=&pSym->hElement;i<pSym->NumElements;i++,phElement++)
	{ 
		LPELEMENT	pElement;
		
		pElement = (LPELEMENT)GlobalLock (*phElement);
		if (pSym->Type == SVLINE)
		{ 
			LPVECTOR	pVector=&pElement->Vector;  
				 
			for (j=0;j<pElement->NumVectors;j++,pVector++)
				MaxLength = max (MaxLength,pVector->Dist); 
		}
		if (pElement->Type > 0) 
			sprintf (str,"%3i: %s",i+1,Type[pElement->Type-1]);
		else
			sprintf (str,"%3i: %s",i+1,NegType[abs(pElement->Type)-1]);
	    SendDlgItemMessage (hWndDlg,idclist,LB_ADDSTRING,0,(LPARAM)str);     
		GlobalUnlock (*phElement);
	}
//	pSym->TiePoint.x = MaxLength;
	GlobalUnlock (hSymbol);  

	return TRUE;
}
FLTPOINT BasePtToWinPtFLT(LPDPOINT WPoint)
{
	DPOINT WinPointD, Intmod;
	POINT  WinPoint;
	FLTPOINT	WinPointF;

	if (!CurView->hTranBaseToVP)
		CreateBaseToVPTran(CurView->DrawRect);
	switch (CurView->FileProjectionType)
	{
		DPOINT PPoint;

	case 2:
		PPoint = *WPoint;
		ProjectBasePt(&PPoint);
		TRANS2(PPoint.x, PPoint.y, &WinPointD.x, &WinPointD.y, CurView->hTranBaseToVP);
		break;
	case 1:
		PPoint = *WPoint;
		TRANS2(PPoint.x, PPoint.y, &WinPointD.x, &WinPointD.y, CurView->hTranBaseToVP);
		break;
	default:
	case 0:
		TRANS2(WPoint->x, WPoint->y, &WinPointD.x, &WinPointD.y, CurView->hTranBaseToVP);
		break;
	case 3:
		PPoint = *WPoint;
		ConvertCoord(&PPoint, 1, GOOGLEMAPSPROJECTION);
		TRANS2(PPoint.x, PPoint.y, &WinPointD.x, &WinPointD.y, CurView->hTranProjectionToScreen);
		break;
	}
	WinPointF.x = WinPointD.x;
	WinPointF.y = WinPointD.y;
	return (WinPointF);
}

FPOINT BasePtToWinPtF (LPDPOINT WPoint)
#if ENABLETRACE
{GSSiEnterProg (1369);
#endif
{    DPOINT WinPointD, Intmod;
     POINT  WinPoint; 
     FPOINT	WinPointF;
     
     if(!CurView->hTranBaseToVP) 
		CreateBaseToVPTran (CurView->DrawRect);
/*     if (CurView->hTranFormat)  
     {
     		TRANS2 (WPoint->x,WPoint->y,&WinPointD.x,&WinPointD.y,CurView->hTranFormat); 
     		WPoint = &WinPointD;
     } */
     switch (CurView->FileProjectionType)
     {  
     	DPOINT PPoint;
     	
     	case 2:
			PPoint = *WPoint;
		    ProjectBasePt (&PPoint);
		    TRANS2 (PPoint.x,PPoint.y,&WinPointD.x,&WinPointD.y,CurView->hTranBaseToVP); 
     		break;
     	case 1:
			PPoint = *WPoint;
		    TRANS2 (PPoint.x,PPoint.y,&WinPointD.x,&WinPointD.y,CurView->hTranBaseToVP); 
			break;
		default:
     	case 0:
     		TRANS2 (WPoint->x,WPoint->y,&WinPointD.x,&WinPointD.y,CurView->hTranBaseToVP);
     		break;
 		case 3:
			PPoint = *WPoint;
			ConvertCoord(&PPoint, 1, GOOGLEMAPSPROJECTION);
     		TRANS2 (PPoint.x,PPoint.y,&WinPointD.x,&WinPointD.y,CurView->hTranProjectionToScreen);
			break;
    } 
     WinPointF.x = WinPointD.x;
     WinPointF.y = WinPointD.y;
{
#if ENABLETRACE
GSSiExitProg (1369);
#endif
     return (WinPointF);
}
#if ENABLETRACE
}
#endif
}

