#include "graphint.h"    
#include "shapefil.h"
 
typedef struct {
				short	Show, Level, Up, Next, Down, SymNum, ParNum, ID;
				char	SymName[34], SymDesc[14];
			   } SYMHIERARCHY;
typedef	SYMHIERARCHY	FAR	*LPSYMHIERARCHY;
#define	MAXHIERARCHYLEVELS	10   

#include "gmextern.h"

static	BYTE    InVisBits[400];
static	BYTE    Mask[8] = {128, 64, 32, 16, 8, 4, 2, 1};
static HANDLE	hDupDesc=0;
static BOOL16	*pDupDesc;
static HANDLE	LastParList=0;
static BOOL	DisableLoadVis=FALSE;

BOOL LayerIsVisible (int Layer)
{
    if (!CurVis || !CurVis->FileIsVisible[Layer])
    	return FALSE;
	return TRUE;
}

BOOL FileIsVisible (LPSTR FileName)
#if ENABLETRACE
{GSSiEnterProg (1377);
#endif
{   
	HFILE	Fid;   
	BOOL	rtn=TRUE;
	long	ORAHeaderType;
	
	switch (MapFileType (FileName))
	{
		case MT_INDEX:
	        rtn = MapIndexVisible (FileName);
	        break;
	        
        case MT_PLT: 
        	rtn = TRUE; 
        	break;
        case MT_SHP:
			if (OpenSHPFile (FileName))
        	{
				if ((SHPType = ReadSHPHeader (SHPFid,0,FileName)))
				{
					if (LoadSHPParm (FileName,SHPType,CurView->hWnd))
						rtn = IsSHPFileVisible ();
				}
				CloseSHPFile ();
			}
			break;
				 
        case MT_GMD:
			{
				HANDLE	hDB=0;
				LPSTR	pPar;

				*GMDWhere = 0;
				if ((pPar = strstr (FileName,".GMD(")))
				{
					pPar += 4;
					strncpy (GMDWhere,pPar,255);
					*pPar = 0;
				}
				if (OpenDataFile (FileName,"",BT_READ,&GMDHandle))
        		{
					if (LoadGMDParm (FileName,CurView->hWnd))
						rtn = IsGMDFileVisible ();
					CloseDataFile (TRUE,&GMDHandle); 
					if (!rtn)
						WantGMDNegGrid = FALSE;
				}
				strcat (FileName,GMDWhere);
			}
			break;
				 
        case MT_PERSONAL_GEO_DB:
			if ((SHPType = ReadPGDBHeader (FileName,0)))
			{
				if (CurView->PassID < 4)
				{
					if ((SHPType == SHPT_POLYGON || SHPType == SHPT_POLYGON_PGDB) && (!GetTypeVisibility(TYPE_AREA) || (CurView->PassID && CurView->PassID != 2)))
						rtn = FALSE;  
					else if ((SHPType == SHPT_POINT || SHPType == SHPT_POINTZ) && (!GetTypeVisibility(TYPE_POINT) || (CurView->PassID && CurView->PassID == 2)))
						rtn = FALSE;  
					else if (SHPType == SHPT_ARC && (!GetTypeVisibility(TYPE_LINECURVE) || (CurView->PassID && CurView->PassID == 2)))
						rtn = FALSE;  
					else if (SHPType == SHPT_TEXT && (!GetTypeVisibility(TYPE_TEXT) || (CurView->PassID && CurView->PassID != 3)))
						rtn = FALSE;  
				}
				if (rtn)
					if (LoadSHPParm (PltName,SHPType,CurView->hWnd))
						rtn = IsSHPFileVisible ();
			}
			else
				rtn = FALSE;
			break;
				 
        case MT_FILE_GEO_DB:
			if ((SHPType = ReadFGDBHeader (FileName,0)))
			{
				if (CurView->PassID < 4)
				{
					if ((SHPType == SHPT_POLYGON || SHPType == SHPT_POLYGON_PGDB) && (!GetTypeVisibility(TYPE_AREA) || (CurView->PassID && CurView->PassID != 2)))
						rtn = FALSE;  
					else if ((SHPType == SHPT_POINT || SHPType == SHPT_POINTZ) && (!GetTypeVisibility(TYPE_POINT) || (CurView->PassID && CurView->PassID == 2)))
						rtn = FALSE;  
					else if (SHPType == SHPT_ARC && (!GetTypeVisibility(TYPE_LINECURVE) || (CurView->PassID && CurView->PassID == 2)))
						rtn = FALSE;  
					else if (SHPType == SHPT_TEXT && (!GetTypeVisibility(TYPE_TEXT) || (CurView->PassID && CurView->PassID != 3)))
						rtn = FALSE;  
				}
				if (rtn && OpenMap((HWND)1,(HDC)1))
				{
					if (LoadSHPParm(PltName, SHPType, CurView->hWnd))
						rtn = IsSHPFileVisible();
					CloseMap(FALSE);
				}
			}
			else
				rtn = FALSE;
			break;
				 
        case MT_ORA:
        	Fid = GSSiOpenFile (FileName,0,OF_READ);
        	if (Fid != HFILE_ERROR)
        	{
				ORAType = ORATypeFromName (FileName);
				if (LoadORAParm (FileName,ORAType))
				{ 
					if ((ORAHeaderType = ReadORAHeader (Fid,0)))  
						rtn = IsORAFileVisible ();
				}
				GSSiClose (Fid);
			}
			break;
			
        case MT_DGN7:
        case MT_GPX:
		    
	    default:
	    	rtn = TRUE;
	}
{
#if ENABLETRACE
GSSiExitProg (1377);
#endif
        return rtn;
}
#if ENABLETRACE
}
#endif
}


BOOL MapIndexVisible (LPSTR IndexPathName)
{   
	HANDLE	hStr=GSSiGlobAlloc (0,GMEM_MOVEABLE,256);
	LPSTR	str=GlobalLock (hStr);
	LPSTR	lpBS;
	BOOL	rtn=TRUE;
	
	if (!CurVis)
		goto RtnTrue;
    _fstrcpy (str,IndexPathName);
	ExpandText (str);
    lpBS = _fstrrchr (str,'\\');
    if (!lpBS)
    	goto RtnTrue;
	lpBS++;
	_fstrcpy (lpBS,"symlist");
    FidMap = GSSiOpenFile (str,0,OF_READ); 
    if (FidMap == HFILE_ERROR)
		goto RtnTrue;
	MapVersion = 8;
    rtn = ProcessPrimarySeg (0,0,0,FALSE,1,HFILE_ERROR); 
    GSSiClose (FidMap);
    FidMap = HFILE_ERROR; 
RtnTrue:
	GSSiGlobUlFree (&hStr);
	return rtn;

} 

BOOL SelectVisList (BOOL Pick)
{    double xscale, yscale,  maxscale;
     long   rect_width, rect_height ;
     double bounds_width, bounds_height;
     LPVIEWPORT	pSaveVP=CurView;
     
     if (!CurView)
     	return FALSE;
     if (DoMapCopy == 4) // counts files in viewport
     	return TRUE;
     if (Pick)
     {
	     if (!_fstrnicmp (CurView->PickName,"FROM CMD",8))
		 {  
			char	SavePN = *CurView->PickName;

		 	if (CurView->ID != *pCommandViewport)
		 		SetViewport(*pCommandViewport);
			*CurView->PickName = 0;
		 	SelectVisList (Pick);
			*CurView->PickName = SavePN;
            CurView = pSaveVP;
            CurVis = pViewports[*pCommandViewport-1]->CurPick;  
            CurView = pSaveVP;
            return (FALSE);
		 }
         if (CurView->pPickListManual)
         {
            CurVis = CurView->pPickListManual;
		    CurView->CurPick = CurVis;
            return (FALSE);
         }
         if (!CurView->pPickList1)
         {
         	if (Pick == 2)
	      		CopyVisListToPickList ();
	        else
	         	goto UseVis; 
         }
         if (CurView->pPickListManual)
         {
            CurVis = CurView->pPickListManual;
		    CurView->CurPick = CurVis;
            return (FALSE);
         }
         CurVis = CurView->pPickList1;
     }
     else
     {
UseVis:
	     if (!_fstrnicmp (CurView->VisName,"FROM CMD",8))
		 {
            CurVis = pViewports[*pCommandViewport-1]->CurVis;  
            CurView = pSaveVP;
            return (FALSE);
		 }
		 if (CurView->pVisListManual)
         {
            CurVis = CurView->pVisListManual;  
			if (Pick)
			    CurView->CurPick = CurVis;
			else	 
		    	CurView->CurVis = CurVis;
            CurView = pSaveVP;
            return (FALSE);
         }
//         if (!CurView->pVisList1) return(FALSE);
         CurVis = CurView->pVisList1;
         if (!CurVis)
         {
             if (CurView->NumFiles)
             {
				HANDLE hVisList=GSSiGlobAlloc ( 709,GHND,sizeof(VISLIST));
				CurVis = (LPVISLIST)GlobalLock (hVisList); 
				CurVis->hVisList=hVisList;
				InitVis ();
                CurView->pVisList1 = CurVis; 
             }
             else  
             {
	            CurView = pSaveVP;
         	    return FALSE;
         	 }
         }
     }
     if (!CurView->HaveBounds || Pick)
     	goto MaxScale;
     rect_width = max(1,(long)CurView->DrawRect.right - (long)CurView->DrawRect.left);
     rect_height = max(1,(long)CurView->DrawRect.bottom - (long)CurView->DrawRect.top);
     bounds_width = CurView->NewBounds.xmx - CurView->NewBounds.xmn;
     bounds_height = CurView->NewBounds.ymx - CurView->NewBounds.ymn;
     xscale = bounds_width / rect_width;
     yscale = bounds_height / rect_height;
     maxscale = max (xscale,yscale); 
     if (maxscale)
     	CurView->BaseUnitsPerPixel = maxscale;
     else
     	CurView->BaseUnitsPerPixel = 1;
     if (!(Printing && !PrintMerging)|| !CurView->VisScale)
        CurView->VisScale = CurView->BaseUnitsPerPixel;
     
MaxScale:
     while ((CurView->VisScale > CurVis->MaxScale)|| !CurView->HaveBounds)
        if (CurVis->NextVisList)
            CurVis = (LPVISLIST)CurVis->NextVisList; 
        else
            break;

	 if (Pick)
	    CurView->CurPick = CurVis;
	 else	 
     	CurView->CurVis = CurVis;
     CurView = pSaveVP;
     return(TRUE);
} 


/*BOOL LoadPickList (LPSTR InName)
{   LPVISLIST   SaveVis=CurVis, LastVisList=0;   
	HANDLE  hVisList=0;
    short FidVis, nread;
    char     Name[128];
   
    _fstrcpy (Name,InName);
    FidVis = GSSiOpenFile (Name,0,OF_READ); 
    if (FidVis == HFILE_ERROR)
    {
        if (!_fstrchr(Name,'\\'))
        {
            _fstrcpy (Name,"piklists\\");
            _fstrcat (Name,InName);
        }
        if (!_fstrchr (Name,'.'))
            _fstrcat (Name,".pik"); 
	    FidVis = GSSiOpenFile (Name,0,OF_READ); 
	    if (FidVis == HFILE_ERROR)
            return FALSE;
    }

    CurVis = CurView->pPickList1;
    while (CurVis)
    {
		if (CurView->pPickListManual == CurVis)
			CurView->pPickListManual = 0;
        hVisList = CurVis->hVisList;
        CurVis =  (LPVISLIST) CurVis->NextVisList;
        GSSiGlobUlFree(&hVisList);
    }
    if (CurView->pPickListManual)
    {
		hVisList = CurView->pPickListManual->hVisList;
        GSSiGlobUlFree(&hVisList);
        CurView->pPickListManual=0;
    }
    CurView->NumPickList=0;
    CurView->pPickList1 = 0;
    if (FidVis != HFILE_ERROR)
    { 
       	HANDLE	PickSame;
        	
       	nread = BigRead (FidVis,&PickSame,2);  
       	if (nread == 2 && PickSame != 1)
       	{   
	       	GSSillseek (FidVis,0,0);
		    SetCurVal (Name,IDS_FILEPIK);
	        LastVisList = 0;
	        hVisList=GSSiGlobAlloc ( 710,GHND,sizeof(VISLIST));
	        CurVis =(LPVISLIST) GlobalLock (hVisList);
	        nread = BigRead (FidVis,CurVis,sizeof(VISLIST));
	        while (nread == sizeof(VISLIST))
	        {   
	            CurView->NumPickList++;
	            CurVis->hVisList = hVisList; 
	            if (CurVis->LastVisList == (char *)1)
	            {
	                CurView->pPickListManual = CurVis; 
	                if (!CurView->pPickList1)
	                {
	                 	HANDLE hVisList=GSSiGlobAlloc ( 719,GHND,sizeof(VISLIST));
					 	 
					 	CurVis->LastVisList = 0; 
					 	CurView->pPickList1 =(LPVISLIST) GlobalLock (hVisList);
					 	*CurView->pPickList1 = *CurVis;
		    		 	CurView->pPickList1->hVisList = hVisList;
		    		 	CurVis = CurView->pPickList1;
		    		 }
	            }
	            else if (!CurView->pPickList1)
	                CurView->pPickList1 = CurVis;
	            CurVis->LastVisList = (LPSTR) LastVisList; 
	            CurVis->NextVisList = 0;
	            if (LastVisList)
	            {
	                SaveVis = CurVis;
	                CurVis = LastVisList;
	                CurVis->NextVisList = (LPSTR)SaveVis;
	                CurVis = SaveVis;
	            }
	            LastVisList = CurVis; 
	            hVisList=GSSiGlobAlloc ( 711,GHND,sizeof(VISLIST));
	            CurVis =(LPVISLIST) GlobalLock (hVisList);
	            nread = BigRead (FidVis,CurVis,sizeof(VISLIST));
	            if (nread != sizeof(VISLIST))
	            	GSSiGlobUlFree (&hVisList);
	        }
	    } 
        GSSiClose (FidVis); 
        if (LastVisList)
        	CurVis = LastVisList; 
        else 
        	CurVis = SaveVis;
        GSSiGlobUlFree(&hVisList);
        SelectVisList (TRUE); 
    }
return TRUE;
} */
BOOL LoadPickList (LPSTR InName)
{   LPVISLIST   SaveVis=CurVis, LastVisList=0;   
	HANDLE  hVisList=0;
	short	nread,Signature,Version;
	long	lenVL;
    HFILE FidVis;
    char     Name[MAX_PATH];
   
    _fstrcpy (Name,InName);
    FidVis = GSSiOpenFile (Name,0,OF_READ); 
    if (FidVis == HFILE_ERROR)
    {
        if (!_fstrchr(Name,'\\'))
        {
            _fstrcpy (Name,"[%DL]piklists\\");
            _fstrcat (Name,InName);
        }
        if (!_fstrchr (Name,'.'))
            _fstrcat (Name,".pik"); 
	    FidVis = GSSiOpenFile (Name,0,OF_READ); 
	    if (FidVis == HFILE_ERROR)
            return FALSE;
    }

    CurVis = CurView->pPickList1;
    while (CurVis)
    {
		if (CurView->pPickListManual == CurVis)
			CurView->pPickListManual = 0;
        hVisList = CurVis->hVisList;
        CurVis =  (LPVISLIST) CurVis->NextVisList;
        GSSiGlobUlFree(&hVisList);
    }
    if (CurView->pPickListManual)
    {
		hVisList = CurView->pPickListManual->hVisList;
        GSSiGlobUlFree(&hVisList);
        CurView->pPickListManual=0;
    }
    CurView->NumPickList=0;
    CurView->pPickList1 = 0;
    if (FidVis != HFILE_ERROR)
    { 
       	HANDLE	PickSame;
        	
       	nread = BigRead (FidVis,(HPSTR)&PickSame,2);  
       	if (nread == 2 && PickSame != (HANDLE)1)
       	{   
	       	GSSillseek (FidVis,0,0);
		    SetCurVal (Name,IDS_FILEPIK);
	        LastVisList = 0;
	        hVisList=GSSiGlobAlloc ( 710,GHND,sizeof(VISLIST));
	        CurVis =(LPVISLIST) GlobalLock (hVisList);
			lenVL = GSSillseek (FidVis,0,2);
			GSSillseek(FidVis,lenVL-4,0);
			BigRead (FidVis,&Signature,2);
			BigRead (FidVis,&Version,2);
			GSSillseek(FidVis,0,0);
  			if (Version < 2)
				nread = ReadVISLIST16 (FidVis,CurVis); 
			else
				nread = BigRead (FidVis,(HPSTR)CurVis,sizeof(VISLIST)); 
	        while (nread == sizeof(VISLIST))
	        {   
	            CurView->NumPickList++;
	            CurVis->hVisList = hVisList; 
	            if (CurVis->LastVisList == (char *)1)
	            {
	                CurView->pPickListManual = CurVis; 
	                if (!CurView->pPickList1)
	                    CurView->pPickList1=CurVis;
	            }
	            else
	                if (!CurView->pPickList1) CurView->pPickList1 = CurVis;
	            CurVis->LastVisList = (LPSTR) LastVisList; 
	            CurVis->NextVisList = 0;
	            if (LastVisList)
	            {
	                SaveVis = CurVis;
	                CurVis = LastVisList;
	                CurVis->NextVisList = (LPSTR)SaveVis;
	                CurVis = SaveVis;
	            }
	            LastVisList = CurVis; 
	            hVisList=GSSiGlobAlloc ( 711,GHND,sizeof(VISLIST));
	            CurVis =(LPVISLIST) GlobalLock (hVisList);
  				if (Version < 2)
					nread = ReadVISLIST16 (FidVis,CurVis); 
				else
					nread = BigRead (FidVis,(HPSTR)CurVis,sizeof(VISLIST)); 
	            if (nread != sizeof(VISLIST))
	            	GSSiGlobUlFree (&hVisList);
	        }
	    } 
        GSSiClose (FidVis); 
        if (LastVisList)
        	CurVis = LastVisList; 
        else
        	CurVis = SaveVis;
        GSSiGlobUlFree(&hVisList); 
    }
return TRUE;   
}

HANDLE ReadVisList (LPSTR InName)
{   LPVISLIST   SaveVis, LastVisList, NewVisList, CurVis;
    HFILE	FidVis;
	short	nread,Signature,Version;
    OFSTRUCTGM    OFStruct;  
    HANDLE	hVisList,hNewVisList;
    char    Name[MAX_PATH];
	char	FullName[MAX_PATH];
	int		lenVL;
    
    _fstrcpy (Name,InName);
    ExpandText (Name);
    _fstrupr (Name);  
    Truncate (Name);
    if (_fstrstr (Name,".TXT"))
        return FALSE;
    FidVis = GSSiOpenFile (Name,(LPOFSTRUCTGM)&OFStruct,OF_READ);
    if (FidVis == HFILE_ERROR)
    {
        if (!_fstrchr(Name,'\\'))
        {
            _fstrcpy (Name,"[%DL]vislists\\");
            _fstrcat (Name,InName);
        }
        if (!_fstrchr (Name,'.'))
            _fstrcat (Name,".vis"); 
		_fullpath (FullName,Name,MAX_PATH);
	    FidVis = GSSiOpenFile (Name,(LPOFSTRUCTGM)&OFStruct,OF_READ);
	    if (FidVis == HFILE_ERROR)
	        return 0;
    }
    hNewVisList=GSSiGlobAlloc ( 712,GHND,sizeof(VISLIST));
    NewVisList =(LPVISLIST) GlobalLock (hNewVisList);
    lenVL = GSSillseek (FidVis,0,2);
    GSSillseek(FidVis,lenVL-4,0);
    BigRead (FidVis,&Signature,2);
    BigRead (FidVis,&Version,2);
	GSSillseek(FidVis,0,0);
  	if (Version < 2)
		nread = ReadVISLIST16 (FidVis,NewVisList); 
    else
		nread = BigRead (FidVis,(HPSTR)NewVisList,sizeof(VISLIST)); 
    if (nread < sizeof(VISLIST))
    {
    	GSSiGlobUlFree (&hNewVisList);
	    GSSiClose (FidVis);
    	return 0;
    }
    LastVisList = 0;
    hVisList = hNewVisList;
    NewVisList->hVisList = hVisList;
    GSSiClose (FidVis);
    GlobalUnlock(hNewVisList);
    
	return hNewVisList;
}


BOOL LoadVisList (LPSTR InName)
{   LPVISLIST   SaveVis, LastVisList, NewVisList;
    short FidVis, nread;  
    HANDLE	hMem=GSSiGlobAlloc ( 713,GMEM_MOVEABLE,1024);
    LPSTR   Name=GlobalLock (hMem);
    LPOFSTRUCTGM    pOFStruct = (LPOFSTRUCTGM)(Name+256);
    HANDLE	hVisList,hNewVisList; 
	long	lenVL;
	short	Version,Signature;
    
    if (DisableLoadVis || !CurView)
    	goto Exit;
    _fstrcpy (Name,InName);
    ExpandText (Name);
    _fstrupr (Name);  
    Truncate (Name);
    if (!_fstrnicmp (Name,"FROM CMD",8))
    	goto Exit;
    if (!_fstrchr (Name,'.'))
	    _fstrcat (Name,".VIS"); 
    if (_fstrstr (Name,".TXT"))
        goto Exit;
    FidVis = GSSiOpenFile (Name,(LPOFSTRUCTGM)pOFStruct,OF_READ);
    if (FidVis == HFILE_ERROR)
    {
        if (!_fstrchr(Name,'\\'))
        {
            _fstrcpy (Name,"[%DL]vislists\\");
            _fstrcat (Name,InName);
        }
        if (!_fstrchr (Name,'.'))
            _fstrcat (Name,".vis"); 
	    FidVis = GSSiOpenFile (Name,(LPOFSTRUCTGM)pOFStruct,OF_READ);
	    if (FidVis == HFILE_ERROR)
	    {   
//	    	sprintf (str,"Unable to open visibility file in viewport %i",CurView->ID);
//	        MessageBox( GetFocus(),Name, str, MB_OK);
	        goto Exit;
	    }
    }
    SetCurVal (Name,IDS_FILEVIS);
    hNewVisList=GSSiGlobAlloc (1727,GHND,sizeof(VISLIST));
    NewVisList =(LPVISLIST) GlobalLock (hNewVisList);
    lenVL = GSSillseek (FidVis,0,2);
    GSSillseek(FidVis,lenVL-4,0);

    GSSilread (FidVis,&Signature,2);
    GSSilread (FidVis,&Version,2);
	GSSillseek (FidVis,0,0);
  	if (Version < 2)
		nread = ReadVISLIST16 (FidVis,NewVisList); 
	else
		nread = BigRead (FidVis,(HPSTR)NewVisList,sizeof(VISLIST)); 
    CurVis = CurView->pVisList1;
    while (CurVis)
    {
		if (CurView->pVisListManual == CurVis)
			CurView->pVisListManual = 0;
        hVisList = CurVis->hVisList;
        CurVis =  (LPVISLIST) CurVis->NextVisList;
        GSSiGlobUlFree(&hVisList);
    }  
    if (CurView->pVisListManual)
    {
		hVisList = CurView->pVisListManual->hVisList;
        GSSiGlobUlFree(&hVisList);
        CurView->pVisListManual=0;
    }
    CurView->NumVisList=0;
    CurView->pVisList1 = 0;
    LastVisList = 0;
    hVisList = hNewVisList;
    CurVis = NewVisList;
    while (nread == sizeof(VISLIST))
    {   
        CurView->NumVisList++;
        CurVis->hVisList = hVisList;
        if (CurVis->LastVisList == (char *)1)
        {
            CurView->pVisListManual = CurVis; 
            if (!CurView->pVisList1)
                CurView->pVisList1=CurVis;
        }
        else
            if (!CurView->pVisList1) CurView->pVisList1 = CurVis;
        CurVis->LastVisList = (LPSTR) LastVisList; 
        CurVis->NextVisList = 0;
        if (LastVisList)
        {
            SaveVis = CurVis;
            CurVis = LastVisList;
            CurVis->NextVisList = (LPSTR)SaveVis;
            CurVis = SaveVis;
        }
        LastVisList = CurVis; 
        hVisList=GSSiGlobAlloc ( 715,GHND,sizeof(VISLIST));
        CurVis =(LPVISLIST) GlobalLock (hVisList);
  		if (Version < 2)
			nread = ReadVISLIST16 (FidVis,CurVis); 
		else
			nread = BigRead (FidVis,(HPSTR)CurVis,sizeof(VISLIST));  
	}
    GSSiClose (FidVis);
    GSSiGlobUlFree(&hVisList);
    CurVis = CurView->pVisList1; 
    _splitpath (pOFStruct->szPathName,0,0,CurView->CurVisibilityID,0);
Exit:
	GSSiGlobUlFree (&hMem);
	return TRUE;
}

BOOL    GetVisibility (int idesc)
{   short     bit, byte;
    BOOL    test;
    
    if (idesc <= 0 || idesc > 3199 || !CurVis)
    	return FALSE;
    byte = idesc/8;
    bit  = idesc%8;
    test = Mask[bit] & CurVis->VisBits[byte];

    if (test) return(TRUE); else return (FALSE);
}
BOOL    GetHalfToneVisibility (int idesc)
{   short     bit, byte;
    BOOL    test;
    
	if (CurView->AlwaysUseHalfTone)
		return TRUE;
    if (idesc <= 0 || idesc > 3199 || !CurView)
    	return FALSE;
    byte = idesc/8;
    bit  = idesc%8;
    test = Mask[bit] & CurView->HalfToneVisBits[byte];

    if (test) return(TRUE); else return (FALSE);
}

BOOL    ToggleVisibility (int idesc)
{   short     bit, byte;
    BOOL    test;
    
    if (!CurVis || !idesc)
    	return FALSE;
    byte = idesc/8;
    bit  = idesc%8;
    CurVis->VisBits[byte] = Mask[bit] ^ CurVis->VisBits[byte];
    test = Mask[bit] & CurVis->VisBits[byte];

    return(test);
}

BOOL    GetInVisibility (int idesc)
{   short     bit, byte;
    BOOL    test;  
    LPSYMBOLATTRIBUTE pSymAtt;
    
   	if (AlwaysUseSymDict)
   	{   
   		if (!hSymbolAttributes)
   			OpenSymDict (OF_READ);
		if (hSymbolAttributes && idesc && idesc <= NumSymbols)
		{
			pSymAtt = (LPSYMBOLATTRIBUTE)GlobalLock (hSymbolAttributes);
			test = pSymAtt[idesc-1].InVisible;
			GlobalUnlock (hSymbolAttributes); 
			return test;
		}
   	}

    byte = idesc/8;
    bit  = idesc%8;
    test = Mask[bit] & InVisBits[byte];

    if (test) return(TRUE); else return (FALSE);
}
BOOL    ToggleInVisibility (int idesc)
{   short     bit, byte;
    BOOL    test;

    byte = idesc/8;
    bit  = idesc%8;
    InVisBits[byte] = Mask[bit] ^ InVisBits[byte];
    test = Mask[bit] & InVisBits[byte];

    return(test);
}

void InvisInit (void)
{ 
	_fmemset (InVisBits,0,sizeof(InVisBits)); 
    return;
}

BOOL    ToggleFileVisibility (int ifile)
{   

    if (CurVis->FileIsVisible[ifile])
        CurVis->FileIsVisible[ifile]=FALSE;
    else
        CurVis->FileIsVisible[ifile]=TRUE;

    return(CurVis->FileIsVisible[ifile]);
}

void SetTypeVisibility (int type, BOOL on)
{   
	switch (type)
	{
    	case 10:
   	    	SetBit (7,(LPSTR)&CurVis->WantType[7],on);
   	    break;
   	    case 11:
           	SetBit (6,(LPSTR)&CurVis->WantType[7],on);
        break;
        case 12:
           	SetBit (5,(LPSTR)&CurVis->WantType[7],on);  
        break; 
        case 1:
        	if (CurVis->WantType[1] == 1)
        	{
        		CurVis->WantType[1] = 0;  
           		SetBit (3,(LPSTR)&CurVis->WantType[1],1);  
           		SetBit (2,(LPSTR)&CurVis->WantType[1],on); 
           	}
           	else 
           		SetBit (2,(LPSTR)&CurVis->WantType[1],on); 
        break;  
        case 101:
        	if (CurVis->WantType[1] == 1)
        	{
        		CurVis->WantType[1] = 0;  
           		SetBit (2,(LPSTR)&CurVis->WantType[1],1);  
           		SetBit (3,(LPSTR)&CurVis->WantType[1],on); 
           	}
           	else 
           		SetBit (3,(LPSTR)&CurVis->WantType[1],on); 
        break;
		default:
    		CurVis->WantType[type]=on;
    	break;
    }  
    TurnOffAutoVis (TRUE);		   
    return;
}
short GetTypeVisibility (int type)
{   
//CurVis->WantType[0] = area
//					1	= POINT, 101=LINE/CURVE (value of 1 = both, bit2 = point, bit3 = linecurve
//					2	= Text
//					3	= Contours
//					4	=  
//					5	= Orthos 
//					6	= Symbols 
//					7	= Area hatch,fill,border 
//					8	= Invis 
//					9	= Width 

	if (!CurVis || ProcessSingleItem)
		return (1);
	if (type == TYPE_TEXT)
		type = 2;
	switch (type)
	{

    	case 10:
   	    	return GetBit (7,(LPSTR)&CurVis->WantType[7]);
   	    break;
   	    case 11:
           	return GetBit (6,(LPSTR)&CurVis->WantType[7]);
        break;
        case 12:
           	return GetBit (5,(LPSTR)&CurVis->WantType[7]);  
        break;
        case 1:
        	if (CurVis->WantType[1] == 1)
        		return 1;
           	return GetBit (2,(LPSTR)&CurVis->WantType[1]);  
        break;
        case 101:
        	if (CurVis->WantType[1] == 1)
        		return 1;
           	return GetBit (3,(LPSTR)&CurVis->WantType[1]);  
        break;
		default:
		    if (CurVis->WantType[type])
		        return (1);
		    else
		        return (0); 
		break;
    }
}
void GetVisList (HWND hWndDlg,int DlgItemSym, int DlgItemPar, int DlgItemFile,int Layer,HFILE FidSymList)
{   
    short       ifile, Signature, LastCurFile=Layer, ii; 
    char        str[256];
    char        ifilec[16]; 
    LPSTR		lpBS;
    BOOL		First;  
    BOOL	SaveUseRefOrTAGIndex=UseRefOrTAGIndex;   
    BOOL	SaveIgnoreBounds = IgnoreBounds, SaveDisplay = Display;
     
    Display = FALSE;
    if (CurView->Type ==  VPTYPE_PROFILE)
    	IgnoreBounds = TRUE;
    UseRefOrTAGIndex = FALSE;
    DuplicateDescInit();
    if (DlgItemPar>0)
    {   SendDlgItemMessage (hWndDlg,DlgItemSym,LB_RESETCONTENT,0,0);
        SendDlgItemMessage (hWndDlg,DlgItemPar,LB_RESETCONTENT,0,0);
        if (DlgItemFile > 0)
        SendDlgItemMessage (hWndDlg,DlgItemFile,LB_RESETCONTENT,0,0);
    }
    CurView->CurFile=max (-1,Layer-2); 
    CloseAllIndexes ();
    CurView->PassID = 4;
	if (Layer <= 0)
		CurView->CurFile=-1;
//	LastCurFile = CurView->CurFile;
//    IncrementFile ();
    while (GetNextViewportFile (TRUE))
    {   
    	if (PltType > 4 && PltType != 9)
    		goto NextFile;
    	if (LastCurFile < FileNum)
    	{
    		First = TRUE;
    		if (Layer > -1 && LastCurFile >= Layer-1)
    			goto Done; 
    	} 
    	else
    		First = FALSE;
    	if (PltType == 4 || PltType == 9)// && First)
    	{   
    		_fstrcpy (str,CurView->lpFiles[CurView->CurFile]);
    		lpBS = _fstrrchr (str,'\\');
    		if (!lpBS)
    			goto FromPltFile;
    		lpBS++;
			if (stricmp (lpBS,"INDEX") && stricmp (lpBS,"INDEX.TIN"))
    			goto FromPltFile;
    		_fstrcpy (lpBS,"symlist");
            FidMap = GSSiOpenFile (str,0,OF_READ); 
            if (FidMap == HFILE_ERROR)
    			goto FromPltFile;
			MapVersion = 8;
            ProcessPrimarySeg (hWndDlg,DlgItemSym,DlgItemPar,FALSE,0,FidSymList); 
            GSSiClose (FidMap);
            FidMap = HFILE_ERROR; 
	        CloseMapIndex (CurView->lpFiles[CurView->CurFile],
	                       CurView->hlpIndex[CurView->CurFile],FALSE,TRUE);
	        CurView->hlpIndex[CurView->CurFile] = 0; 
			//IncrementFile (); 
			First = TRUE;
    		goto NextFile;
    	}
FromPltFile:
        if (PltType < 5)
        {   
        	switch (MapFileType (PltName))
        	{   
         		case MT_SHP:
	        	{    
	        		long	SaveSHPRec = CurrentSHPRec;
	        		
					if (OpenMap ((HWND)1,0)) 
					{
						SetSHPVis (hWndDlg,DlgItemSym, DlgItemPar,FidSymList);
		            	CloseMap (FALSE);
		            }
		            CurrentSHPRec = SaveSHPRec;
	        	}
				break;

         		case MT_GMD:
	        	{    
	        		long	SaveGMDRec = CurrentGMDRec;
	        		
					if (OpenMap ((HWND)1,0)) 
					{
						SetGMDVis (hWndDlg,DlgItemSym, DlgItemPar,FidSymList);
		            	CloseMap (FALSE);
		            }
		            CurrentGMDRec = SaveGMDRec;
	        	}
	        	break;  
				
         		case MT_GPX:
	        	{    
	        		long	SaveGPXRec = CurrentGPXRec;
	        		
					if (OpenMap ((HWND)1,0)) 
					{
						SetGPXVis (hWndDlg,DlgItemSym, DlgItemPar,FidSymList);
		            	CloseMap (FALSE);
		            }
		            CurrentGPXRec = SaveGPXRec;
	        	}
	        	break;   
	        	
        		case MT_PERSONAL_GEO_DB:
	        	{    
	        		long	SaveSHPRec = CurrentSHPRec;
	        		
					if (LoadSHPParm (PltName,0,CurView->hWnd))
						SetSHPVis (hWndDlg,DlgItemSym, DlgItemPar,FidSymList);
		            CurrentSHPRec = SaveSHPRec;
	        	}
	        	break; 

        		case MT_FILE_GEO_DB:
	        	{    
	        		long	SaveSHPRec = CurrentSHPRec;
	        		
					if (LoadSHPParm (PltName,0,CurView->hWnd))
						SetSHPVis (hWndDlg,DlgItemSym, DlgItemPar,FidSymList);
		            CurrentSHPRec = SaveSHPRec;
	        	}
	        	break;   
	        	
	        	case MT_DTM:
	        	break;
	        	
	        	case MT_ORA:
	        	{
					if (OpenMap ((HWND)1,0)) 
					{
						SetORAVis (hWndDlg,DlgItemSym, DlgItemPar,FidSymList);
		            	CloseMap (FALSE);
		            }
	        	}
	        	break;
	        	
	        	case MT_DGN7:
	        	{
					if (OpenMap ((HWND)1,0)) 
					{   
						DGNVisSetFromIndex = FALSE;
						SetDGNVis (hWndDlg,DlgItemSym, DlgItemPar,FidSymList);
		            	CloseMap (FALSE); 
		            	if (DGNVisSetFromIndex && CurView->FileType[CurView->CurFile] == 4 && CurView->hlpIndex[CurView->CurFile])
		            	{
		            		LPFILEINDEX lpIndex = (LPFILEINDEX)GlobalLock(CurView->hlpIndex[CurView->CurFile]);
                            
    						lpIndex->FileInIndex = (long)lpIndex->NumFiles;
                            GlobalUnlock(CurView->hlpIndex[CurView->CurFile]); 
                        }
		            }
	        	}
	        	break;
	        	
	        	default:
	        	{
		            CloseMap (FALSE);
		            FidMap = GSSiOpenFile (PltName,0,OF_READ); 
		            if (FidMap != HFILE_ERROR)
		            {
					    GSSillseek(FidMap,(LONG)-(6),2);
					    BigRead (FidMap,(HPSTR)&Signature,2);
					    if (Signature == 32349)
					    {   
					    	BYTE	MV;
					    	
						    BigRead (FidMap,&MV,1); 
						    MapVersion = MV;
			                GSSillseek(FidMap,(LONG)-(6+12),2);
			                BigRead (FidMap,(HPSTR)&PrimeOffset,4);
			                GSSillseek(FidMap,PrimeOffset,0); 
			                UsedDescOffset = -1;
			                ProcessPrimarySeg (0, 0, 0,FALSE,0,HFILE_ERROR);
			                if (UsedDescOffset < 0)
			                	ii=1;
			                else
			                {
				                GSSillseek(FidMap,UsedDescOffset,0);
				                ProcessPrimarySeg (hWndDlg,DlgItemSym,DlgItemPar,FALSE,0,FidSymList);
				            } 
			            }
			            else
			            	ii=1;
		                GSSiClose (FidMap); 
		            } 
		        }
		        break;
		    }
            FidMap = HFILE_ERROR;
        }  
NextFile:
		LastCurFile = FileNum;
    } 
Done:
    DuplicateDescClose(); 
    if (hWndDlg && DlgItemFile)
    for (ifile=0;ifile<CurView->NumFiles;ifile++)
    {
        _fstrcpy (str,CurView->FileID[ifile]);
		ExpandText (str);
        if(CurVis && CurVis->FileIsVisible[ifile])
            _fstrcat (str,"\t<on>\t");
        else
            _fstrcat (str,"\t\t");
        itoa(ifile,ifilec,10);
        _fstrcat (str,ifilec);
        SendDlgItemMessage (hWndDlg,DlgItemFile,LB_ADDSTRING,0,(LPARAM)str);
    }
    UseRefOrTAGIndex = SaveUseRefOrTAGIndex;
    IgnoreBounds = SaveIgnoreBounds;  
    Display = SaveDisplay;
    return;
}

void DuplicateDescInit(void)
{   
//    hCursor = LoadCursor (0,IDC_WAIT);
//    OldCursor = GSSiSetCursor (hCursor);
    
    hDupDesc = GSSiGlobAlloc ( 716,GHND,3200*sizeof(BOOL16));
    pDupDesc = (LPSHORT) GlobalLock (hDupDesc);
    return;
}

BOOL DuplicateDesc(int idesc, BOOL DupSet)
{   BOOL16    *bpnt, rtn;

    if (!hDupDesc || idesc <= 0 || idesc > 3201)
    	return(TRUE);
    bpnt = pDupDesc + (idesc-1);
    rtn = *bpnt;
    *bpnt = DupSet;
    return (rtn);
}

void DuplicateDescClose(void)
{
    GSSiGlobUlFree (&hDupDesc);
//    hCursor = 0; 
//    GSSiSetCursor (OldCursor);
    return;
}  

void SetPickSame (LPVIEWPORT CurView)
{    
	 LPVISLIST	CurVis; 
	 HANDLE		hVisList, savedb;
	 
	 if (!CurView)
		return;
	 CurVis  = CurView->pPickList1;
	 if (CurView->pPickListManual == CurVis)
		CurView->pPickListManual = NULL;
	 while (CurVis)
	 {
		hVisList = CurVis->hVisList; 
		savedb = hVisList;
		CurVis =  (LPVISLIST) CurVis->NextVisList;
		GSSiGlobUlFree (&hVisList);
	 }  
	 CurView->NumPickList=0; 
	 if (CurView->pPickListManual)
	 {
	 	hVisList = CurView->pPickListManual->hVisList;
		GSSiGlobUlFree (&hVisList);
     }
     CurView->pPickListManual = NULL;
     CurView->pPickList1=NULL;   
     SelectVisList (FALSE);
     return;
}

void TurnOffAutoVis (BOOL OnOff)
{  
	if (InZoomMacro || Pickability)
		return;
	if (!CurView)
		return; 
	if (OnOff)
	{
		if (!*CurView->VisName)
			return;
		CurView->DisableZoomMacro = TRUE; 
	}
	else
		CurView->DisableZoomMacro = FALSE; 
	return;
}		   

BOOL SetParentVisibility (int Parent, BOOL Vis, short Layer)
{   OFSTRUCTGM    OFStruct;
    HANDLE      hParList;
    PARLIST *pParList;
    HANDLE  CurParList;
    short	Signature, LastCurFile=Layer;
    LPSTR		lpBS;
    BOOL		First; 
    char	str[MAX_PATH];  
    short	SaveCurFile, SaveSubFile, SaveRestoreFile, SavePassID; 
    char	SaveOrigFile[MAX_PATH], SaveFile[MAX_PATH];
    BOOL	SaveUseRefOrTAGIndex=UseRefOrTAGIndex;
    BOOL	rtn=TRUE;

    if (!CurView)
    	return FALSE;
	if (Vis == 3)
		rtn = FALSE;
    SaveCurFile  = CurView->CurFile;
	if (AlwaysUseSymDict)
	{
		LPSHORT	Child;
		HANDLE	hChildren=0;
		short	nChildren=0;
		
		GetSymDictChildren (Parent,&nChildren,&hChildren,-1,TRUE); 
		if (hChildren)
		{		
			Child = (LPSHORT)GlobalLock (hChildren);
			while (nChildren--)  
			{ 
				if (*Child == 29)
					ii = 1;
				if (Vis == 3)
				{
					if (GetVisibility(*Child))
						rtn = TRUE;
				}
				else if (GetVisibility(*Child) != Vis)
						ToggleVisibility (*Child);
				Child++;
			}
			GSSiGlobUlFree (&hChildren);
		}
		return rtn;
	}
    UseRefOrTAGIndex = FALSE;
    if (CurView->SubFile)
    {
        _fstrcpy (SaveOrigFile,CurView->OrigFile);
        _fstrcpy (SaveFile,CurView->lpFiles[CurView->RestoreFile]); 
        _fstrcpy (CurView->lpFiles[CurView->RestoreFile],CurView->OrigFile);  
        SaveRestoreFile = CurView->RestoreFile;
    }   
    SavePassID = CurView->PassID;
	CurView->PassID = 4;
    GSSiGlobFree (&CurView->hBinFileList);
   	SaveSubFile = CurView->SubFile;
    CurView->SubFile = 0;
    DuplicateDescInit();
    CurView->CurFile=max (-1,Layer-1);
    while (GetNextViewportFile (TRUE))
    {   
    	if (PltType > 4)
    		goto NextFile;
    	if (LastCurFile < FileNum)
    	{
    		First = TRUE;
    		if (Layer > -1)
    			goto Done; 
    	} 
    	else
    		First = FALSE;
    	if (PltType == 4)// && First)
    	{   
    		_fstrcpy (str,PltName);
    		lpBS = _fstrrchr (str,'\\');
    		if (!lpBS)
    			goto FromPltFile;
    		lpBS++;
    		_fstrcpy (lpBS,"symlist");
            FidMap = GSSiOpenFile (str,(LPOFSTRUCTGM)&OFStruct,OF_READ); 
            if (FidMap == HFILE_ERROR)
    			goto FromPltFile;
			MapVersion = 8;
            ParIsVisible = Vis;
            LastParList = 0;
            CurParList = AddParToList (Parent);
            
            while (CurParList)
            {
                pParList=(LPPARLIST) GlobalLock(CurParList);
                SetVisByPar=pParList->Parent;  
                GSSillseek (FidMap,0,0);
                ProcessPrimarySeg (0, 0, 0,FALSE,0,HFILE_ERROR);
                hParList = CurParList;
                CurParList = pParList->Next;
                GSSiGlobUlFree (&hParList);
            }
            SetVisByPar = -1;
            GSSiClose (FidMap);
            FidMap = HFILE_ERROR; 
	        CloseMapIndex (CurView->lpFiles[CurView->CurFile],
	                       CurView->hlpIndex[CurView->CurFile],FALSE,TRUE);
	        CurView->hlpIndex[CurView->CurFile] = 0; 
	//		IncrementFile ();
    		goto NextFile;
    	}
FromPltFile:
    	if (PltType < 5)
        {
            CloseMap (FALSE);
            FidMap = GSSiOpenFile (PltName,(LPOFSTRUCTGM)&OFStruct,OF_READ);
            if (FidMap != HFILE_ERROR)
            {   
            	BYTE	MV;
            	
			    GSSillseek(FidMap,(LONG)-(6),2);
			    BigRead (FidMap,(HPSTR)&Signature,2);
				if (Signature != 32349)
					goto Next;
				BigRead (FidMap,&MV,1); 
				MapVersion = MV;
                ParIsVisible = Vis;
                LastParList = 0;
                CurParList = AddParToList (Parent);
            
                while (CurParList)
                {
                    pParList=(LPPARLIST) GlobalLock(CurParList);
                    SetVisByPar=pParList->Parent;
                    GSSillseek(FidMap,(LONG)-(6+12),2);
                    BigRead (FidMap,(HPSTR)&PrimeOffset,4);
                    GSSillseek(FidMap,PrimeOffset,0);
                    ProcessPrimarySeg (0, 0, 0,FALSE,0,HFILE_ERROR);
                    GSSillseek(FidMap,UsedDescOffset,0);
                    ProcessPrimarySeg (0,0,0,FALSE,0,HFILE_ERROR);
                    hParList = CurParList;
                    CurParList = pParList->Next;
                    GSSiGlobUlFree (&hParList);
                }
	Next:
                SetVisByPar = -1;
                GSSiClose (FidMap); 
            }
            FidMap = HFILE_ERROR;
        }
NextFile:
		LastCurFile = FileNum;
    } 
Done:
    DuplicateDescClose();
    if (CurView->SubFile)
        _fstrcpy (CurView->lpFiles[CurView->RestoreFile],CurView->OrigFile); 
    if (SaveSubFile && SaveCurFile >= 0)
    {
    	CurView->SubFile = SaveSubFile;  
        CurView->RestoreFile = SaveRestoreFile;
        _fstrcpy (CurView->OrigFile,SaveOrigFile);
        _fstrcpy (CurView->lpFiles[CurView->RestoreFile],SaveFile); 
    }
    else
    	CurView->SubFile = 0;
    CurView->CurFile = SaveCurFile;   
    CurView->PassID = SavePassID;   
    CurView->CurFile = SaveCurFile;
    UseRefOrTAGIndex = SaveUseRefOrTAGIndex;
    return rtn;
}

HANDLE AddParToList(int Parent)
{   HANDLE  hParList;
    PARLIST *pParList;

    hParList = GSSiGlobAlloc ( 717,GMEM_MOVEABLE,sizeof(PARLIST));
    pParList =(LPPARLIST) GlobalLock (hParList);
    pParList->Parent = Parent;
    pParList->Next = 0;
    if (LastParList)
    {   pParList=(LPPARLIST) GlobalLock(LastParList);
        pParList->Next = hParList;
        GlobalUnlock(LastParList);
    }
    LastParList = hParList;
    GlobalUnlock(hParList);

    return (hParList);

}


void InitVis (void)
{   short i;

    for (i=0;i<400;i++){ CurVis->VisBits[i]=255;}
    for (i=0;i<10;i++) CurVis->WantType[i]=TRUE;
//    CurVis->WantType[3]=0; not sure why this is here - JS 3/15/97
    CurVis->WantType[8]=0; /* 9 is Width visibility, 8 is Invisible Visibility */ 
	CurVis->WantType[9]=1;
    CurVis->MinPointSize=0;
    CurVis->MaxPointSize=9999;
    CurVis->MinScale=0;
    CurVis->MaxScale=999999.0;
    for (i=0;i<MAX_VIEWPORT_FILES;i++) CurVis->FileIsVisible[i]=TRUE;
}

BOOL GetTextVisibility (float TSize,short from)
{   float PointSize;
    
    if (!TSize)
    	return FALSE;
    if (from)
    	PointSize = TSize * PixelsPerHInch;
    else
    	PointSize = TSize;  
    if (!CurVis)
    	return FALSE;
    if (PointSize >= CurVis->MinPointSize && PointSize <= CurVis->MaxPointSize)
        return (TRUE);
    else
        return (FALSE);
}


BOOL CopyVisListToPickList (void)
{
    LPVISLIST   FromVis, LastVis;
    HANDLE		hVisList;
    
    if (CurView->pVisListManual)
    {
        hVisList=GSSiGlobAlloc ( 718,GHND,sizeof(VISLIST));
        CurVis =(LPVISLIST) GlobalLock (hVisList);
        *CurVis = *CurView->pVisListManual; 
        CurVis->hVisList = hVisList; 
    	CurView->pPickListManual = CurVis;  
    	return TRUE;
    }
    LastVis = 0;
    CurView->NumPickList = CurView->NumVisList;
    FromVis = CurView->pVisList1;
    while (FromVis)
    {
        hVisList=GSSiGlobAlloc ( 718,GHND,sizeof(VISLIST));
        CurVis =(LPVISLIST) GlobalLock (hVisList);
        *CurVis = *FromVis; 
        if (!LastVis) CurView->pPickList1 = CurVis;
        CurVis->LastVisList = (LPSTR)  LastVis;
        CurVis->NextVisList = 0;
        CurVis->hVisList = hVisList; 
        if (LastVis) LastVis->NextVisList = (LPSTR) CurVis;
        LastVis = CurVis; 
        FromVis = (LPVISLIST) FromVis->NextVisList;
    }
    return(TRUE);
}


BOOL SetTypeVisByName (LPSTR Name,int OnOrOff)
{ 
	char	TypeNames[15][8]={"AREA","POINTS","TEXT","CONTOUR","notused","ORTHO","SYMBOL","notused","INVIS","WIDTH","FILL","HATCH","BORDER","LINES","PLC"};
	short	i;
	BOOL	Reverse;
	
	for (i=0;i<15;i++)
		if (!stricmp (Name,"ALL") || !strnicmp (Name,TypeNames[i],min(4,strlen(TypeNames[i]))))
		{
			int	j=i;

			if (i == 13)
				j = 101;
			if (i == 14)
				j = 1;
			switch (OnOrOff)
			{
			case 2:
				Reverse = !GetTypeVisibility (j);
				SetTypeVisibility (j,Reverse);
				break;
			case 3:
				return GetTypeVisibility (j);
				break;
			default:
				SetTypeVisibility (j,OnOrOff);
			}
			if (i == 14)
			{
				j = 101;
				switch (OnOrOff)
				{
				case 2:
					Reverse = !GetTypeVisibility (j);
					SetTypeVisibility (j,Reverse);
					break;
				case 3:
					return GetTypeVisibility (j);
					break;
				default:
					SetTypeVisibility (j,OnOrOff);
				}
			}
			if (stricmp (Name,"ALL"))
				return TRUE;
		}
	if (!stricmp (Name,"ALL"))
		return TRUE;
	return FALSE;
}


BOOL SaveVisFile (LPSTR SaveName,BOOL Pickability,LPSTR desc)
{
	HFILE Fid;
	LPVISLIST	SaveVis=CurVis;
	int	Signature=28051, Version=2;
	short	PickSame = 0;
	
	Fid = GSSiOpenFile (SaveName,(LPOFSTRUCTGM) NULL,OF_CREATE);
	if (Fid == HFILE_ERROR)
		return FALSE;
	if (!Pickability)
	{
		if (CurView->pVisListManual)
		{
			CurVis = CurView->pVisListManual; 
			CurVis->NextVisList=NULL;
			CurVis->LastVisList=(LPSTR)1; //indicates manual
		}
		else
			CurVis = CurView->pVisList1; 
	}
	else 
	{
		if (CurView->pPickListManual) 
		{
			CurVis = CurView->pPickListManual;
			CurVis->NextVisList=NULL;
			CurVis->LastVisList=(LPSTR)1; //indicates manual
		}
		else if (CurView->pPickList1)
			CurVis = CurView->pPickList1; 
		else
		{
			PickSame = 1;
			CurVis = CurView->pVisList1; 
		}
	}					 		
	while (CurVis)
	{   
		HANDLE	SaveHandle = CurVis->hVisList;
		
		CurVis->hVisList = (HANDLE)PickSame;
		BigWrite (Fid,(HPSTR)CurVis,sizeof(VISLIST),-1);  
		CurVis->hVisList = SaveHandle;
		CurVis =(LPVISLIST) CurVis->NextVisList;
	}
	BigWrite (Fid,(HPSTR)desc,100,-1);
	BigWrite (Fid,(HPSTR)&Signature,2,-1);   
	BigWrite (Fid,(HPSTR)&Version,2,-1);
	GSSiClose (Fid);
	CurVis = SaveVis;
	
	return TRUE;
}



BOOL GetFileVisList (HWND hWndDlg,WORD iList)
{   
	short	i, on; 
	char	str[256], OnOff[2][6]={"<on>",""};
	
    SendDlgItemMessage (hWndDlg,iList,LB_RESETCONTENT,0,0); 
    
    for (i=0;i<CurView->NumFiles;i++)
    {   
    	if (CurVis->FileIsVisible[i])
    		on = 0;
    	else
    		on = 1;
    	sprintf (str,"%s\t%s",OnOff[on],CurView->FileID[i]);
	    SendDlgItemMessage (hWndDlg,iList,LB_ADDSTRING,0,(LPARAM)str);
    }
    return TRUE;  
}
 
HANDLE BuildSymHierarchy (HWND hWndDlg,UINT iList, LPINT pStartSym, LPINT pnSym)
{
	HANDLE	handle, hPar = GSSiGlobAlloc ( 726,GMEM_MOVEABLE,USHRT_MAX);
	LPSYMHIERARCHY pSH, pSHLast, pSHPar, pSHBeg;  
	short	nsym;
	short	i, nPar=0, np, ParID, Lev; 
	LPSHORT	pPar;
	char	str[256];   
	LPSTR	lpTAB, lpBeg; 
	
	
    nsym=(long)SendDlgItemMessage (hWndDlg,iList,LB_GETCOUNT,0,0); 
    *pnSym = nsym; 
    if (!nsym)
    	return 0;
    handle = GSSiGlobAlloc ( 727,GHND,(long)nsym*sizeof(SYMHIERARCHY));
    pSH = (LPSYMHIERARCHY)GlobalLock (handle);
    for (i=0;i<nsym;i++,pSH++)
    {
		SendDlgItemMessage(hWndDlg,iList,LB_GETTEXT,i,(DWORD)str); 
		lpBeg = str;
		lpTAB = _fstrchr (lpBeg,'\t');
		*lpTAB++=0;
		_fstrcpy (pSH->SymName,lpBeg);
		lpBeg = lpTAB;
		lpTAB = _fstrchr (lpBeg,'\t');
		*lpTAB++=0;
		lpBeg = lpTAB;
		lpTAB = _fstrchr (lpBeg,'\t');
		*lpTAB++=0;
		pSH->ParNum = atoi (lpBeg); 
		pSH->Show = 2;
		pPar = (LPSHORT)GlobalLock (hPar);
		np = nPar;
		while (np)
		{
			if (*pPar == pSH->ParNum)
				break;
			pPar++; 
			np--;
		}
		if (!np && pSH->ParNum)
		{
			*pPar = pSH->ParNum;
			nPar++;
		}
		GlobalUnlock (hPar); 
		if (!pSH->ParNum)
			*pStartSym = i+1; 
		pSH->ID = i+1;
		pSH->SymNum = atoi (lpTAB);
    }
    GlobalUnlock (handle);
	pPar = (LPSHORT)GlobalLock (hPar);  
	np = nPar;
	while (np--)
	{   
		ParID = GetSymID (*pPar,nsym,handle);
		Lev = GetSymLev (ParID,nsym,handle);
		pSHLast = 0;
		pSH = pSHBeg = (LPSYMHIERARCHY)GlobalLock (handle); 
		pSHPar = pSHBeg + (ParID-1);
		pSHPar->Show = 1;
		for (i=0;i<nsym;i++,pSH++)
		{
			if (pSH->ParNum == *pPar)
			{   
				pSH->Level = Lev+1;
				pSH->Up = ParID;
				if (pSHLast)
					pSHLast->Next = pSH->ID;
				else
					pSHPar->Down = pSH->ID;
				pSHLast = pSH;
			}
		}  
		GlobalUnlock (handle);
		pPar++;
	}
    GlobalUnlock (hPar);
	pPar = (LPSHORT)GlobalLock (hPar);  
	np = nPar;
	while (np--)
	{   
		short	NumChildren=0;
		
		ParID = GetSymID (*pPar,nsym,handle);
		pSH = (LPSYMHIERARCHY)GlobalLock (handle); 
		pSH += (ParID-1);
		if (GetChildrenVis (pSH->Down,&NumChildren,handle)) 
			pSH->Show = 1;
		else
			pSH->Show =0;
		GlobalUnlock (handle); 
		pPar++;
	}
    GSSiGlobUlFree (&hPar);
	return handle;
}  

short GetSymID (int SymNum,int nsym,HANDLE handle)
{
	short ID=0;
	LPSYMHIERARCHY pSH;  
	
	pSH = (LPSYMHIERARCHY)GlobalLock (handle);
	while (nsym--)
		if (pSH->SymNum == SymNum)
		{
			ID = pSH->ID;
			break;
		}
		else
			pSH++;
	GlobalUnlock (handle);
	return ID;
}

short GetSymLev (int SymID, int nsym,HANDLE handle)
{
	LPSYMHIERARCHY pSH, pSHBeg;  
	short	Lev=0;
	
	if (!SymID)
		return -1;
	pSHBeg = (LPSYMHIERARCHY)GlobalLock (handle);
	pSH = pSHBeg + (SymID-1);
	while (pSH->ParNum)
	{
		Lev++;
		SymID = GetSymID (pSH->ParNum,nsym,handle);
		pSH = pSHBeg + (SymID-1);
	}
	GlobalUnlock (handle);
	
	return Lev;
}

BOOL DisplaySymHierarchy (HWND hWndDlg,UINT iList,HANDLE hSymHierarchy, int start)
{
	LPSYMHIERARCHY pSH, pBeg, pLev[MAXHIERARCHYLEVELS];
	short	lev=0, i, t=4;  
	int   	TabStops[MAXHIERARCHYLEVELS+3]; 
	char	str[256], str2[32]; 
	char	PlusMinus[4]="+- ";
	
	if (!hSymHierarchy)
		return FALSE;
	TabStops[0] = 10;
	TabStops[1] = 40;
	for (i=2;i<MAXHIERARCHYLEVELS+2;i++,t+=8)
		TabStops[i] = t;
	TabStops[i++] = t+2000;
    SendDlgItemMessage (hWndDlg,iList,LB_SETTABSTOPS,MAXHIERARCHYLEVELS+3,(LPARAM)&TabStops);
    SendDlgItemMessage (hWndDlg,iList,LB_RESETCONTENT,0,0);  
    pBeg = (LPSYMHIERARCHY)GlobalLock (hSymHierarchy);
    pLev[0] = pBeg + (start-1);

Top:
	str[0] = PlusMinus[pLev[lev]->Show];
	str[1] = 0;
	_fstrcat (str,"\t");
	_fstrcat (str,AllNoneSome(pLev[lev]->ID,hSymHierarchy));
	_fstrcat (str,"\t");
	for (i=0;i<pLev[lev]->Level;i++)
		_fstrcat (str,"\t");
	_fstrcat (str,pLev[lev]->SymName);
	for (i=pLev[lev]->Level;i<MAXHIERARCHYLEVELS;i++)
		_fstrcat (str,"\t"); 
	if (GetVisibility(pLev[lev]->SymNum))
		_fstrcat (str,"<on>\t");
	else
		_fstrcat (str,"\t");
	if (pLev[lev]->Down)
		_fstrcat (str,itoa (-pLev[lev]->SymNum,str2,10)); 
	else
		_fstrcat (str,itoa (pLev[lev]->SymNum,str2,10));
    SendDlgItemMessage (hWndDlg,iList,LB_ADDSTRING,0,(LPARAM)str);
    if (pLev[lev]->Show)
    {
    	if (pLev[lev]->Down)
    	{
    		lev++;
    		pLev[lev] = pBeg + (pLev[lev-1]->Down-1);
    		goto Top;
    	}
    }
NextSibling:
	if (pLev[lev]->Next)
	{ 
		pLev[lev] = pBeg + (pLev[lev]->Next-1);
		goto Top;
	}
	else if (!lev)
		goto Exit; 
	lev--;
	goto NextSibling;
Exit:
	GlobalUnlock (hSymHierarchy);
    
	return TRUE;
}    

BOOL ToggleShow (int SymNum,int nsym, HANDLE hSymHierarchy)
{
	short ID;
	LPSYMHIERARCHY pSH, pSHBeg;  
	
	if (!hSymHierarchy)
		return FALSE;   
	ID = GetSymID (SymNum,nsym,hSymHierarchy);
	pSHBeg = (LPSYMHIERARCHY)GlobalLock (hSymHierarchy); 
	pSH = pSHBeg + (ID - 1);
	switch (pSH->Show)
	{
		case 0:
			pSH->Show = 1;
			break;
		case 1:
			pSH->Show = 0; 
			CloseChildren (pSH->Down,hSymHierarchy);
			break;
		case 2:
			GlobalUnlock (hSymHierarchy);
			return FALSE; 
	}
	GlobalUnlock (hSymHierarchy);
	return TRUE;
}

LPSTR AllNoneSome(short ID,HANDLE hSymHierarchy)
{   
	static char ANS[5][8]={"<all>","","<some>","<on>","<most>"};
	LPSYMHIERARCHY pSH, pSHBeg;
	LPSTR	rtn=ANS[1];
	short	NumChildren, NumChildrenVis;  
	
	pSHBeg = (LPSYMHIERARCHY)GlobalLock (hSymHierarchy);
	pSH = pSHBeg + (ID - 1);
	if (pSH->Down)
	{
		if (!pSH->Show)
		{   
			NumChildren = 0;
			NumChildrenVis = GetChildrenVis (pSH->Down,&NumChildren,hSymHierarchy); 
			if (NumChildrenVis == NumChildren)
				rtn = ANS[0];
			else if (NumChildrenVis > NumChildren/2)
				rtn = ANS[4];
			else if (NumChildrenVis)
				rtn = ANS[2];
		}
	}
	else
	{
		if (GetVisibility (pSH->SymNum))
			rtn = ANS[3];
	}
	GlobalUnlock (hSymHierarchy); 
	return rtn;
}

BOOL CloseChildren (short ID,HANDLE hSymHierarchy)
{
	LPSYMHIERARCHY pSH, pSHBeg;  
	
	pSHBeg =(LPSYMHIERARCHY)GlobalLock (hSymHierarchy);
	while (ID)
	{ 
		pSH = pSHBeg + (ID - 1);
		if (pSH->Down)    
		{
			pSH->Show = 0;
			CloseChildren (pSH->Down,hSymHierarchy);
		}
		ID = pSH->Next;   
	}
	GlobalUnlock (hSymHierarchy);
	return TRUE;
} 

short GetChildrenVis (short ID,LPSHORT pNumChildren,HANDLE hSymHierarchy)
{
	LPSYMHIERARCHY pSH, pSHBeg; 
	short	NumVis=0; 
	
	pSHBeg = (LPSYMHIERARCHY)GlobalLock (hSymHierarchy);
	while (ID)
	{ 
		pSH = pSHBeg + (ID - 1);
		if (pSH->Down)
			NumVis += GetChildrenVis (pSH->Down,pNumChildren,hSymHierarchy);
		else
		{
			(*pNumChildren)++;
			if (GetVisibility (pSH->SymNum))
				NumVis++; 
		}
		ID = pSH->Next;
	}
	GlobalUnlock (hSymHierarchy);
	return NumVis;
} 

BOOL CursorOnExpandButton (HWND hWndDlg,WORD LBCntl)
{   
	RECT	Rect; 
	POINT	pnt;
	
	GetWindowRect (GetDlgItem (hWndDlg,LBCntl),&Rect);
	GetCursorPos (&pnt);
	if (abs (pnt.x-Rect.left) < 20)
		return TRUE;
	else
		return FALSE;
}


BOOL GetPenRedefColor(int ipen,COLORREF *Color)
{   COLORREF	*lpNewColors;
            
	if (!CurView->hPenRedef)
		return FALSE;
	lpNewColors = (COLORREF *)GlobalLock (CurView->hPenRedef);
	lpNewColors += ipen;   
	if (*lpNewColors != ULONG_MAX)
	{
		*Color = *lpNewColors;
		GlobalUnlock (CurView->hPenRedef);
		return TRUE;
	}
	GlobalUnlock (CurView->hPenRedef);
	return FALSE;
} 

void AddPenRedef (int ipen, COLORREF Color)
{   COLORREF	*lpNewColors; 

	if (!CurView->hPenRedef)
	{   int	i;
	
		CurView->hPenRedef = GSSiGlobAlloc (1757,GMEM_MOVEABLE,sizeof(COLORREF)*MAXPENS);
		lpNewColors = (COLORREF *)GlobalLock (CurView->hPenRedef);
		for (i=0;i<MAXPENS;i++,lpNewColors++)   
			*lpNewColors = ULONG_MAX;
		GlobalUnlock (CurView->hPenRedef);
	}
	lpNewColors = (COLORREF *)GlobalLock (CurView->hPenRedef);
	lpNewColors += ipen;   
	*lpNewColors = Color;
	GlobalUnlock (CurView->hPenRedef);
	
	return;
} 

BOOL LoadDisplayRedefFile (LPSTR Name)
{   COLORREF	*lpNewColors;
	short	HavePenRedef;
	OFSTRUCTGM	OFStruct;
	HFILE	Fid;  
//	NEWOBJECT_v0	OldNewObject; 
	short		i,version=0,Signature;
	
	if (!*Name) return FALSE;
	Fid = GSSiOpenFile (Name,&OFStruct,OF_READ);
	if (Fid == HFILE_ERROR)
		return FALSE; 
	GSSillseek (Fid,-(4),2);
	BigRead (Fid,(HPSTR)&Signature,2);  
	if (Signature == 20852) 
	{
		BigRead (Fid,(HPSTR)&version,2);
	    if (version > 2)
	    {	GSSiClose(Fid);
			MessageBox( GetFocus(), "This redef file version is not recognized",Name, MB_OK);
			return(FALSE);
	    }
	}
	GSSillseek (Fid,0,0);
	BigRead (Fid,(HPSTR)&HavePenRedef,2); 
	if (HavePenRedef)
	{
		if (!CurView->hPenRedef)
			CurView->hPenRedef = GSSiGlobAlloc (1758,GMEM_MOVEABLE,sizeof(COLORREF)*MAXPENS);
		lpNewColors = (COLORREF *)GlobalLock (CurView->hPenRedef); 
		BigRead (Fid,(HPSTR)lpNewColors,sizeof(COLORREF)*MAXPENS);
		GlobalUnlock (CurView->hPenRedef);
	}
	BigRead (Fid,(HPSTR)&CurView->NumNewObjects,2);
	CurView->NumNewObjects = min (CurView->NumNewObjects,100);
	if (CurView->NumNewObjects) 
	{
		BigRead (Fid,(HPSTR)&CurView->NewObjectMap,3201*2); 
		if (version==1)
		{
			for (i=0;i<CurView->NumNewObjects;i++)
			{
				CurView->NewObject[i] = ReadNewObject16 (Fid);
			}
		}
		else if (version == 2)
			BigRead (Fid,(HPSTR)&CurView->NewObject,CurView->NumNewObjects*sizeof(NEWOBJECT));
		else
		{	
			for (i=0;i<CurView->NumNewObjects;i++)
			{
				CurView->NewObject[i] = ReadNewObject_V0 (Fid);
				CurView->NewObjectTextFactor[i] = 1;
				CurView->NewObjectSetTextColor[i] = 0;
			}
		}
	}
	GSSiClose (Fid);
	for (i=0;i<CurView->NumNewObjects;i++)
		CurView->NewObject[i].Handle = 0;
	return TRUE;
} 

BOOL SaveDisplayRedefFile (LPSTR Name)
{   COLORREF	*lpNewColors;
OFSTRUCTGM	OFStruct;
	short	HavePenRedef;  
	int		Signature, Version=2;
	HFILE	Fid;
	
	if (!*Name) return FALSE;   
	CurView->NumNewObjects = min (CurView->NumNewObjects,MAX_NEW_OBJECTS);
	Fid = GSSiOpenFile (Name,&OFStruct,OF_CREATE);
	if (Fid == HFILE_ERROR) return FALSE;
	if (CurView->hPenRedef)
	{          
		HavePenRedef = 1;
		BigWrite (Fid,(HPSTR)&HavePenRedef,2,-1); 
		lpNewColors = (COLORREF *)GlobalLock (CurView->hPenRedef); 
		BigWrite (Fid,(HPSTR)lpNewColors,sizeof(COLORREF)*MAXPENS,-1);
		GlobalUnlock (CurView->hPenRedef);
	}
	else
	{
		HavePenRedef = 0;
		BigWrite (Fid,(HPSTR)&HavePenRedef,2,-1); 
	}  
	BigWrite (Fid,(HPSTR)&CurView->NumNewObjects,2,-1);
	if (CurView->NumNewObjects) 
	{
		BigWrite (Fid,(HPSTR)&CurView->NewObjectMap,3201*2,-1);
		BigWrite (Fid,(HPSTR)&CurView->NewObject,CurView->NumNewObjects*sizeof(NEWOBJECT),-1); 
	}
	Signature = 20852;
    BigWrite (Fid,(HPSTR)&Signature,2,-1);
    BigWrite (Fid,(HPSTR)&Version,2,-1);
	
	GSSiClose (Fid);
	return TRUE;
}








BOOL SetLayerVisibility (LPSTR Name,short setopt)
{   
	short	i, WantVP=-1;
	BOOL	rtn=FALSE;
	 
	if (!CurView || !CurVis)
		return FALSE;
	if (*Name == '#')
	{
		i = atoi (Name+1);
		if (i && i <= CurView->NumFiles)
		{
			WantVP = i-1;
		}
	}
	for (i=0;i<CurView->NumFiles;i++)
	{
		char	str[MAX_PATH];

		strcpy (str,CurView->FileID[i]);
		ExpandText (str);
		if (!_fstricmp (str,Name) || i == WantVP || !_fstricmp ("ALL",Name))
		{
			if (setopt == 2)
			{
				if (CurVis->FileIsVisible[i])
					CurVis->FileIsVisible[i]=FALSE;
				else
					CurVis->FileIsVisible[i]=TRUE;
			}
			else if (setopt == 3)
			{
				if (CurVis->FileIsVisible[i])
					return TRUE;
				else
					return FALSE;
			}
			else
				CurVis->FileIsVisible[i] = setopt;
			rtn = TRUE;   
			Pickability = FALSE;
		    TurnOffAutoVis (TRUE);		   
		}
	}
	return rtn;
}   

BOOL SetLayerSymbolsVisibility (LPSTR Name,short setopt)
{   
	short	i;
	BOOL	rtn=FALSE;    
	HFILE	Fid;  
	char	FileName[MAX_PATH], str[130];   
	short	idesc;
	
	GSSiGetTempFileName (0,"gms",0,FileName); 
	Fid = GSSiOpenFile (FileName,0,OF_CREATE);
	for (i=0;i<CurView->NumFiles;i++)
		if (!_fstricmp (CurView->FileID[i],Name) || !_fstricmp ("ALL",Name))
		 	GetVisList (0,0,0,0,i+1,Fid); 
	GSSillseek (Fid,0,0);
	while (fgetstring (str,128,Fid))
	{  
		LPSTR	lpTAB = _fstrrchr (str,'\t'); 
		
		idesc = 0;
		if (lpTAB)
		{
			*lpTAB = 0;
			lpTAB = _fstrrchr (str,'\t'); 
			if (lpTAB)
				idesc = atoi (++lpTAB);
		}
		if (GetVisibility(idesc) != setopt)
			ToggleVisibility (idesc);
	}   
	GSSiClose (Fid);
	GSSiRemove (FileName);
    Pickability = FALSE;
    TurnOffAutoVis (TRUE);		   
	rtn = TRUE;
	return rtn;
}   

BOOL DumpVisibilityToFile (LPSTR FileName,BOOL ShowAllLayers,BOOL Pickability)
{   
	short	i,j;
	HFILE	Fid;  
	char	str[130];   
	short	idesc;
	short	SaveType[10];
	LPVIEWPORT	SavePVP=CurView;
	VIEWPORT	SaveVP=*CurView;
	LPVISLIST	SavePVL;
	VISLIST		SaveVL;
	HANDLE	handle;

	Fid = GSSiOpenFile (FileName,0,OF_CREATE); 
	if (Fid == HFILE_ERROR)
		return FALSE;
	WaitCursor (1);
	SaveViewports (0);
	SelectVisList (Pickability);
	SavePVL=CurVis;
	SaveVL=*CurVis;
	fputstring ("SYMNAME\tONOFF\tPARENTSYMNUM\tSYMNUM\tLAYER",Fid);
	for (i=0;i<CurView->NumFiles;i++)
	{
		BOOL SaveFileVis = CurVis->FileIsVisible[i];

		if (ShowAllLayers)
		{
			CurVis->FileIsVisible[i] = TRUE;
			for (j=0;j<10;j++)
			{
				SaveType[j] = CurVis->WantType[j];
				CurVis->WantType[j] = 1;
			}
		}
		if (LayerIsVisible (i))
	 		GetVisList (0,0,0,0,i+1,Fid);
		CurVis->FileIsVisible[i] = SaveFileVis;
		if (ShowAllLayers)
		{
			for (j=0;j<10;j++)
				CurVis->WantType[j] = SaveType[j];
		}
	}
	GSSiClose (Fid);
	WaitCursor (-1);
	RestoreViewports ();
	CurView = SavePVP;
	SelectVisList (Pickability);
	handle = CurVis->hVisList;
	*CurVis = SaveVL;
	CurVis->hVisList = handle;

/*	*CurView = SaveVP;
	CurVis = SavePVL;
	*CurVis = SaveVL;*/
	return TRUE;
}   

