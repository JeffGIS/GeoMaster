#include "graphint.h"      


#include "gmextern.h"


BOOL GetKeyWord (LPSTR str,LPSTR Key,LPSTR Value)
{   
	LPSTR	pKey, pSC;
	
	*Value = 0;
	pKey = _fstrstr (str,Key);
	if (!pKey)
		return FALSE;
	pKey += _fstrlen (Key);
	if ((pSC = _fstrchr (pKey,',')))
		*pSC = 0;
	_fstrcpy (Value,pKey);
	if (pSC)
		*pSC = ',';
	return TRUE;
}

BOOL FAR PASCAL GET_PARENTSYMMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 

	char	str[256],SymName[66],ParName[66];
	int		idesc,ipar,i,ii;
	LPSTR	pEnd;
    
 switch(Message)
   {
    case WM_INITDIALOG:  
	{
		for (i=0;i<3200;i++)
		{
			if (!GetDictSymbolType (i))
			{
				GetDictSymName (i,SymName);
				ipar = GetDictSymParent (i);
				GetDictSymName (ipar,ParName);
				sprintf (str,"%s under parent %s",SymName,ParName);
				ii=SendDlgItemMessage (hWndDlg,IDC_PARENTLIST,CB_ADDSTRING,(WPARAM)0,(LPARAM) str); 
			}
		
		}
	}
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
            case IDCANCEL: 
				*CurSymName = 0;
               EndDialog(hWndDlg, FALSE);
            break;

		    case IDOK: 
				GetDlgItemText(hWndDlg,IDC_PARENTLIST,str,sizeof(str));
				if ((pEnd = strstr (str," under parent")))
					*pEnd = 0;
				strcpy (CurSymName,str);
				EndDialog(hWndDlg, TRUE);
 		    	break;
    
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} 

short SelectParentSymbol (HWND hWnd,short DialogOpt,LPSTR SymName)
{
	int	nRc;
	
	strcpy (CurSymName,SymName);
	nRc = DialogBox(hInst, (LPSTR)"GET_PARENTSYM", hWnd, GET_PARENTSYMMsgProc);
	if (nRc)
		 _fstrcpy (SymName,CurSymName);
	return nRc;
} 

short SelectAreaSymbol (HWND hWnd,short DialogOpt,LPSTR SymName, LPSTR SymColor,BOOL AllowAdd)
{
	FARPROC lpfnGET_POINTSYMMsgProc;
	int	nRc;
	
	GetSymType = 3;
	AllowAddSym = AllowAdd && GetGlobalBVal2 ("[%SYMDICTUPDATEALLOWED]",TRUE);			
	_fstrcpy (CurSymName,SymName);
	if (SymColor)  
		_fstrcpy (CurSymColor,SymColor);   
	*CurSymRot=0;
	*CurSymSize=0;
	lpfnGET_POINTSYMMsgProc = MakeProcInstance((FARPROC)GET_POINTSYMMsgProc, hInst);
	switch (DialogOpt)
	{   
		default:
		case 1:
			nRc = DialogBox(hInst, (LPSTR)"GET_POINTSYM", hWnd, lpfnGET_POINTSYMMsgProc);
		break;
		
		case 2: 
			*CurSymName = 0;
			nRc = DialogBox(hInst, (LPSTR)"GET_POINTSYM2", hWnd, lpfnGET_POINTSYMMsgProc);
			if (nRc == SHRT_MAX)
				nRc = 0;
		break;  
	}
	FreeProcInstance(lpfnGET_POINTSYMMsgProc);   
	if (nRc)
	{
		 _fstrcpy (SymName,CurSymName);
		 if (SymColor)
		 	_fstrcpy (SymColor,CurSymColor);
	}
	return nRc;
} 

short SelectLineSymbol (HWND hWnd,short DialogOpt,LPSTR SymName, LPSTR SymSize, LPSTR SymColor,BOOL AllowAdd)
{
	FARPROC lpfnGET_POINTSYMMsgProc;
	int	nRc;
				
	GetSymType = 2;
	AllowAddSym = AllowAdd && GetGlobalBVal2 ("[%SYMDICTUPDATEALLOWED]",TRUE);		
	_fstrcpy (CurSymName,SymName);
	if (SymColor)   
		_fstrcpy (CurSymColor,SymColor);  
	if (SymSize)
		_fstrcpy (CurSymSize,SymSize); 
	*CurSymRot=0;
	lpfnGET_POINTSYMMsgProc = MakeProcInstance((FARPROC)GET_POINTSYMMsgProc, hInst);
	switch (DialogOpt)
	{   
		default:
		case 1:
			nRc = DialogBox(hInst, (LPSTR)"GET_POINTSYM", hWnd, lpfnGET_POINTSYMMsgProc);
		break;
		
		case 2: 
			*CurSymName = 0;
			nRc = DialogBox(hInst, (LPSTR)"GET_POINTSYM2", hWnd, lpfnGET_POINTSYMMsgProc);
			if (nRc == SHRT_MAX)
				nRc = 0;
		break;  
	}
	FreeProcInstance(lpfnGET_POINTSYMMsgProc);   
	if (nRc)
	{
		_fstrcpy (SymName,CurSymName);
		if (SymSize) 
			_fstrcpy (SymSize,CurSymSize);
		if (SymColor)
			_fstrcpy (SymColor,CurSymColor);
	}
	return nRc;
} 
short SelectPointSymbol (HWND hWnd,short DialogOpt,LPSTR SymName, LPSTR ParName,LPSTR SymSize, LPSTR SymRot, LPSTR SymColor,BOOL AllowAdd)
{
	FARPROC lpfnGET_POINTSYMMsgProc;
	int	nRc;
				
	GetSymType = 1;
	AllowAddSym = AllowAdd && GetGlobalBVal2 ("[%SYMDICTUPDATEALLOWED]",TRUE);			
	_fstrcpy (CurSymName,SymName);   
	if (!*CurParName)
		strcpy (CurParName,ParName);   
	if (SymColor && *SymColor)   
		_fstrcpy (CurSymColor,SymColor);  
	if (SymSize && *SymSize)
		_fstrcpy (CurSymSize,SymSize); 
	if (SymRot && *SymRot)
		_fstrcpy (CurSymRot,SymRot);
	DoPaint = FALSE; 
	lpfnGET_POINTSYMMsgProc = MakeProcInstance((FARPROC)GET_POINTSYMMsgProc, hInst);
	switch (DialogOpt)
	{   
		default:
		case 1:
			nRc = DialogBox(hInst, (LPSTR)"GET_POINTSYM", hWnd, lpfnGET_POINTSYMMsgProc);
		break;
		
		case 2: 
			*CurSymName = 0;
			nRc = DialogBox(hInst, (LPSTR)"GET_POINTSYM2", hWnd, lpfnGET_POINTSYMMsgProc);
			if (nRc == SHRT_MAX)
				nRc = 0;
		break;  
	}
	FreeProcInstance(lpfnGET_POINTSYMMsgProc);   
	DoPaint = TRUE;
	if (nRc)
	{
		_fstrcpy (SymName,CurSymName); 
		if (SymColor)   
			_fstrcpy (SymColor,CurSymColor);
		if (SymSize)
			_fstrcpy (SymSize,CurSymSize);
		if (SymRot)
			_fstrcpy (SymRot,CurSymRot);
	}
	return nRc;
} 

#define XBITMAP 80  
#define YBITMAP 20 
 
#define BUFFER MAX_PATH 
 
//HBITMAP hbmpPencil, hbmpCrayon, hbmpMarker, hbmpPen, hbmpFork; 
//HBITMAP hbmpPicture, hbmpOld; 
 








BOOL CheckSymChanged (HWND hWndDlg,BOOL SymChanged,LPHANDLE phSymbol,short SymNum,BOOL Prompt)
{   
	char	str[128], SymName[64];
	LPSYMBOL	pSym; 
	short	rc=IDYES;
	int	nItems; 
	HANDLE	hItems;
	HANDLE	*phElement;  
	LPELEMENT	pElement;
	LPINT	pItem; 
	int	i, j,Choice; 
	LPSTR	pTab;
	
	if (!SymChanged || !*phSymbol || !SymNum)
		return TRUE; 
	nItems = GetLBSelectedItems (hWndDlg,IDC_SYMLIST,&hItems);
	switch (nItems)
	{
		case 0:
			return TRUE;
		case 1:
			GetDlgItemText (hWndDlg,IDC_NAME,SymName,63);
			//GetDictSymName (SymNum,SymName);
			pSym = GlobalLock (*phSymbol);
			strncpy (pSym->Name,SymName,sizeof(pSym->Name));
			GlobalUnlock (*phSymbol);
			sprintf (str,"Save changes to symbol %s",SymName);
			if (Prompt)
				rc = GSSiMsgBox(hWndMain,str,"Verify Changes",MB_YESNO,0);
			if (rc != IDYES)
				break;
			if (SymChanged == 99)
			{
				int	item;

				ReplaceSymbol (SymNum,*phSymbol);
				DestroySymbol (*phSymbol); 
				*phSymbol = 0;
				item = SendDlgItemMessage(hWndDlg,IDC_SYMLIST,LB_GETTOPINDEX,0,0); 
				PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, item);
				break;
			}
		default:  
			pItem = (LPINT)GlobalLock (hItems);
			for (i=0;i<nItems;i++,pItem++)
			{
				SendDlgItemMessage(hWndDlg,IDC_SYMLIST,LB_GETTEXT,*pItem,(DWORD)str); 
				pTab = _fstrrchr (str,'\t');
				pTab++;
				SymNum = atoi (pTab);  
				*phSymbol = GetDictSymDesc (SymNum,0); 
				pSym = (LPSYMBOL)GlobalLock (*phSymbol); 
				if (!pSym->InVisible && !pSym->NumElements)
				{
					pSym->NumElements = 1; 
					pSym->hElement = AllocateNewElement ();   
					pElement = (LPELEMENT)GlobalLock (pSym->hElement);
					pElement->Type = max (2,pSym->Type); 
					GlobalUnlock (pSym->hElement);
				}
				if (GetBit (2,(LPSTR)&SymChanged))                             
            	{
					Choice=SendDlgItemMessage(hWndDlg,IDC_PARLIST,CB_GETCURSEL,0,0); 
					SendDlgItemMessage(hWndDlg,IDC_PARLIST,CB_GETLBTEXT,Choice,(DWORD)str); 
	                OneSpace (str);
					pSym->Parent = GetDictSymbolNumber (str);
			    }
				if (GetBit (3,(LPSTR)&SymChanged))                             
					pSym->InVisible = SendDlgItemMessage (hWndDlg,IDC_INVISIBLE,BM_GETCHECK,0,0);
				if (GetBit (4,(LPSTR)&SymChanged))                             
					GetDlgItemText (hWndDlg,IDC_DESC,pSym->Desc,sizeof(pSym->Desc)-1); 
				if (GetBit (5,(LPSTR)&SymChanged)) 
					pSym->Type=SendDlgItemMessage(hWndDlg,IDC_TYPELIST2,CB_GETCURSEL,0,0); 
				if (GetBit (6,(LPSTR)&SymChanged)) 
					pSym->BaseScale=SendDlgItemMessage(hWndDlg,IDC_DEFAULTSCALE,CB_GETCURSEL,0,0); 
				if (GetBit (7,(LPSTR)&SymChanged)) 
				{
					GetDlgItemText (hWndDlg,IDC_HSIZE,str,16);
                    pSym->HSize = atof (str); 
                }  
				if (GetBit (8,(LPSTR)&SymChanged))
				{ 
					GetDlgItemText (hWndDlg,IDC_VSIZE,str,16);
                    pSym->VSize = atof (str); 
                }  
				if (GetBit (9,(LPSTR)&SymChanged))
				{
					pSym->NoSizeLimit = SendDlgItemMessage (hWndDlg,IDC_NOSIZELIMIT,BM_GETCHECK,0,0);
					if (pSym->NoSizeLimit)pSym->BaseSizeSet = 0;
				}
				if (GetBit (10,(LPSTR)&SymChanged) || GetBit (11,(LPSTR)&SymChanged) || GetBit (15,(LPSTR)&SymChanged))                             
				{
					phElement=&pSym->hElement;  
					for (j=0;j<pSym->NumElements;j++)
					{
						pElement = (LPELEMENT)GlobalLock (phElement[j]);
						switch (pElement->Type)
						{   
							case 3:
								if (!SendDlgItemMessage (hWndDlg,IDC_LINEDISPLAY,BM_GETCHECK,0,0))
									pElement->LineColorType = SVNULLCOLOR;
								else if (SendDlgItemMessage (hWndDlg,IDC_VARLINECOLOR,BM_GETCHECK,0,0))
									pElement->LineColorType = SVVARCOLOR;
								else
									pElement->LineColorType = 0;
								break;
							case 2:
								if (SendDlgItemMessage (hWndDlg,IDC_VARLINECOLOR,BM_GETCHECK,0,0))
									pElement->LineColorType = SVVARCOLOR;
								else
									pElement->LineColorType = 0;
								pElement->Squared = SendDlgItemMessage (hWndDlg,IDC_SQUAREENDS,BM_GETCHECK,0,0);
								pElement->Shadow = SendDlgItemMessage (hWndDlg,IDC_SHADOWLINE,BM_GETCHECK,0,0);
							break;
						}
						GlobalUnlock (phElement[j]);
					} 
				}
				if (GetBit (12,(LPSTR)&SymChanged))                             
				{
					phElement=&pSym->hElement;  
					for (j=0;j<pSym->NumElements;j++)
					{
						pElement = (LPELEMENT)GlobalLock (phElement[j]);
						switch (pElement->Type)
						{   
							case 3:
								if (SendDlgItemMessage (hWndDlg,IDC_VARFILLCOLOR,BM_GETCHECK,0,0))
									pElement->FillColorType = SVVARCOLOR;
								else
									pElement->FillColorType = 0;  
								break;
							break;
						}
						GlobalUnlock (phElement[j]);
					} 
				}
				if (GetBit (13,(LPSTR)&SymChanged))                             
					pSym->Layered = SendDlgItemMessage (hWndDlg,IDC_SYMLAYERED,BM_GETCHECK,0,0);
				if (GetBit (14,(LPSTR)&SymChanged))                             
					pSym->HasBitmap = SendDlgItemMessage (hWndDlg,IDC_HASBITMAP,BM_GETCHECK,0,0);
				if (GetBit (16,(LPSTR)&SymChanged))                             
					pSym->BlockRotation = SendDlgItemMessage (hWndDlg,IDC_BLOCKROTATION,BM_GETCHECK,0,0);
				if (GetBit (17,(LPSTR)&SymChanged))   
				{
					if (SendDlgItemMessage (hWndDlg,IDC_TRUESCALE,BM_GETCHECK,0,0))  
						pSym->BaseScale = 15;  
					else
						pSym->BaseScale = 0;  
				}
 				if (GetBit (18,(LPSTR)&SymChanged))                             
					pSym->DisplayPos=SendDlgItemMessage(hWndDlg,IDC_DISPLAYPOS,CB_GETCURSEL,0,0); 
 				if (GetBit (19,(LPSTR)&SymChanged)) 
				{
					phElement=&pSym->hElement;  
					for (j=0;j<pSym->NumElements;j++)
					{
						pElement = (LPELEMENT)GlobalLock (phElement[j]);
						switch (pElement->Type)
						{   
							case 2:
								if (SendDlgItemMessage (hWndDlg,IDC_SHADOWLINE,BM_GETCHECK,0,0))
									pElement->Shadow = 1;
								else
									pElement->Shadow = 0;  
								break;
							break;
						}
						GlobalUnlock (phElement[j]);
					} 
				}
                GlobalUnlock (*phSymbol);
				ReplaceSymbol (SymNum,*phSymbol);
				DestroySymbol (*phSymbol);
				*phSymbol = 0;
			}
			GSSiGlobUlFree (&hItems);
		break;
	}
	GSSiGlobFree (&hItems);
	return TRUE;
} 

short GetOrCreateSym (HWND hWnd,LPSTR InSymName,LPSHORT pNumSyms,LPHANDLE phSymDesc,int Create, int Type)
{    
	//Create=0	symbol not created if missing
	//Create=1	symbol created from [%NEW_ variables
	//Create=2	create symbol dialog displayed
	//Create=3	update existing symbol
	BOOL	Opened;
    short	SymNum, ParentSymbol=0;
	short		TemplateSymbol=0;
	char	SymName[128],NewDesc[66],NewPar[34];
	char		NewSymbolTemplate[4][64]={
	                                     "[%NEW_PARENT_SYMBOL_TEMPLATE]",
										 "[%NEW_POINT_SYMBOL_TEMPLATE]",
	                                     "[%NEW_LINE_SYMBOL_TEMPLATE]",
	                                     "[%NEW_AREA_SYMBOL_TEMPLATE]"};
	char		NewSymbolParent[4][64]={
	                                     "[%NEW_PARENT_SYMBOL_PARENT]",
										 "[%NEW_POINT_SYMBOL_PARENT]",
	                                     "[%NEW_LINE_SYMBOL_PARENT]",
	                                     "[%NEW_AREA_SYMBOL_PARENT]"};
    HANDLE  hSymbol;
	LPSYMBOL	CurSymbol; 
	LPSTR	pSC;
	 
    
    _fstrcpy (SymName,InSymName);
    ExpandText (SymName);
    GetKeyWord (SymName,"DES=",NewDesc);
    GetKeyWord (SymName,"PAR=",NewPar);
    if ((pSC=_fstrchr (SymName,',')))
		*pSC = 0;
    Truncate (SymName);
    if (!*SymName)
    	_fstrcpy (SymName,"NULLSYMBOL");
	SymNum = GetDictSymbolNumber (SymName);

AddToList:
	if (SymNum && Create != 3)
	{   
		if (Type && phSymDesc)
			AddToSymList (SymNum,pNumSyms,phSymDesc);  
		return SymNum;
	}
	if (!Create)
		return 0;
	if (Create == 2)
	{
		if (!(SymNum = CreateNewSymbol (hWnd,SymName,Type)))
			return 0;
		Create = 0;
		goto AddToList;
	}
	if (!*NewPar)
		_fstrcpy (NewPar,NewSymbolParent[Type]);
	ExpandText (NewPar); 
	if (!_fstricmp (NewPar,SymName))
		*NewPar = 0;
	if (!_fstricmp (NewPar,"-none-"))
		ParentSymbol = -1;
	else if (*NewPar)
 		ParentSymbol = GetOrCreateSym (hWnd,NewPar,pNumSyms,phSymDesc,min (1,Create),0);
 
	Opened = OpenSymDict (OF_READWRITE);
	ExpandText (NewSymbolTemplate[Type]);
 	TemplateSymbol = GetDictSymbolNumber (NewSymbolTemplate[Type]); 
	if (TemplateSymbol)
	{
		hSymbol = GetDictSymDesc (TemplateSymbol,0); 
		CurSymbol = (LPSYMBOL)GlobalLock (hSymbol);  
	}
	else
	{	
		hSymbol = AllocateNewSymbol (); 
		CurSymbol = (LPSYMBOL)GlobalLock (hSymbol);
		CurSymbol->Parent=GetDictSymbolNumber("NEW");
	}
 	if (ParentSymbol)
 		CurSymbol->Parent = ParentSymbol;
 	if (!CurSymbol->Parent)
		CurSymbol->Parent=GetDictSymbolNumber("ALL");
	CurSymbol->Type = Type;         
	_fstrcpy (CurSymbol->Name,SymName);
	if (*NewDesc)
		_fstrcpy (CurSymbol->Desc,NewDesc);
	else
		_fstrcpy (CurSymbol->Desc,"New automatically created symbol");
	GlobalUnlock (hSymbol);	
	if (SymNum)
		ReplaceSymbol (SymNum,hSymbol);
	else	 
		SymNum = SaveSymbol(hSymbol,0);   
	DestroySymbol (hSymbol);  
	if (phSymDesc)
		AddToSymList (SymNum,pNumSyms,phSymDesc);  
	if (Opened)
		CloseSymDict();
	return SymNum;
} 



