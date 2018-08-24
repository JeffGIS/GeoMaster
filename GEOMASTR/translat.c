#include "graphint.h"   
#include "translat.h"
#include "extrndb.h"   
#include "pnet.h"

#include "gmextern.h"

static	TAGKEY TAGKey;
static HANDLE		hIntersect = 0;
static HANDLE	hTiger1BT;
static HFILE	hTiger1Data;
static	struct	{
			long Long, Lat, TLID;
		}	IntersectKey; 
static	struct	{
			long	Snum;
			long	OPLong, OPLat;
		}	IntersectData;

BOOL GuessProjection(HWND hWndDlg, UINT ProjCntl, UINT ProjUnits, LPMNMXCORD pFileBounds, BOOL UseFirst)
{
	MNMXCORD	BoundsInt, FileBoundsCVT, ProjBounds;
	short	PCTIn[32][2], UsedUnits[32][2], CVTUnits[32], CvtID = 0, UnitsID;
	short	nFound = 0, MaxFound = 0, MaxID, MaxUnits;
	BOOL	Err;
	char	SaveAltProj[MAX_PATH], str[4096];
	DPOINT	Points[4];
	LPSTR	pName;
	HFILE	Fid;
	int		CurProj;
	long	Loc;
	short	i;
	char	File[MAX_PATH];

	if (!hProjectionFile)
		return FALSE;
	pName = GlobalLock(hProjectionFile);
	Fid = GSSiOpenFile(pName, 0, OF_READ);
	GlobalUnlock(hProjectionFile);
	if (Fid == HFILE_ERROR)
		return FALSE;
	GetGlobalCVal("[%ALT_PROJECTION]", SaveAltProj, 0);
	GetGlobalCVal("[%PROJECTBOUNDS]", str, 0);
	ProjBounds = atobounds(str, &Err);
	if (!ValidBounds(&ProjBounds))
		return FALSE;

	while (SendDlgItemMessage(hWndDlg, ProjCntl, CB_GETLBTEXT, (WPARAM)CvtID, (LPARAM)str) != CB_ERR)
	{
		Loc = SendDlgItemMessage(hWndDlg, IDC_PROJECTION, CB_GETITEMDATA, (WPARAM)CvtID, (LPARAM)0);
		if (Loc < 0)
			_fstrcpy(str, "[%DL]baseproj.cvt");
		else
		{
			GSSillseek(Fid, Loc, 0);
			fgetstring(str, 250, Fid);
		}
		SetGlobalValue("%ALT_PROJECTION", str);
		PCTIn[CvtID][0] = 0;
		PCTIn[CvtID][1] = 0;
		UnitsID = 0;
		ConvertCoordClose();
		ConvertCoordInit();
		if (PRJ_UNITS[3] == 1 || PRJ_UNITS[3] == 2)
			CVTUnits[CvtID] = 1;
		else
			CVTUnits[CvtID] = PRJ_UNITS[3];
	BeginUnits:
		PRJ_UNITS[3] = CVTUnits[CvtID];
		UsedUnits[CvtID][UnitsID] = PRJ_UNITS[3] - 1;
		DBoundsInit(&FileBoundsCVT);
		Points[0].x = pFileBounds->xmn;
		Points[0].y = pFileBounds->ymn;
		Points[1].x = pFileBounds->xmn;
		Points[1].y = pFileBounds->ymx;
		Points[2].x = pFileBounds->xmx;
		Points[2].y = pFileBounds->ymx;
		Points[3].x = pFileBounds->xmx;
		Points[3].y = pFileBounds->ymn;
		for (i = 0; i<4; i++)
		{
			if (ConvertCoord(&Points[i], 3, 1))
			{
				goto NextUnits;
			}
			AddDPointToMinMax(&Points[i], &FileBoundsCVT);
		}
		if (IntersectBounds(&ProjBounds, &FileBoundsCVT, &BoundsInt))
		{
			PCTIn[CvtID][UnitsID] = IDNINT(10000 * BoundsArea(&BoundsInt) / BoundsArea(&FileBoundsCVT));
			if (PCTIn[CvtID][UnitsID])
			{
				if (PCTIn[CvtID][UnitsID] > MaxFound)
				{
					MaxFound = PCTIn[CvtID][UnitsID];
					MaxID = CvtID;
					MaxUnits = UnitsID;
				}
				nFound++;
			}
		}
	NextUnits:
		if (CVTUnits[CvtID] == 1)
		{
			CVTUnits[CvtID]++;
			UnitsID++;
			goto BeginUnits;
		}
		CvtID++;
	}
	GSSiClose(Fid);

	if (UseFirst)
		nFound = min(nFound, 1);
	switch (nFound)
	{
	case 1:
		SendDlgItemMessage(hWndDlg, ProjCntl, CB_SETCURSEL, (WPARAM)MaxID, (LPARAM)0);
		SendDlgItemMessage(hWndDlg, ProjUnits, CB_SETCURSEL, (WPARAM)min(2, UsedUnits[MaxID][MaxUnits]), (LPARAM)0);
		break;
	case 0:
		GSSiMsgBox(hWndDlg, "None of the supplied projections fit this data", "", MB_ICONEXCLAMATION, 0);
		break;
	default:
	{
			   char	PrjName[64], UnitsName[64];
			   short	i, j;
			   char	UnitsNames[4][8] = { "Feet", "Meters", "Degrees", "Degrees" };
			   *str = 0;
			   for (i = 0; i<CvtID; i++)
			   for (j = 0; j<2; j++)
			   if (PCTIn[i][j])
			   {
				   SendDlgItemMessage(hWndDlg, ProjCntl, CB_GETLBTEXT, (WPARAM)i, (LPARAM)PrjName);
				   sprintf(_fstrchr(str, 0), "%s - %s (%.0f percent)\r\n", PrjName, UnitsNames[UsedUnits[i][j]], (double)PCTIn[i][j] / 100);
			   }
			   GSSiMsgBox(hWndDlg, str, "Multiple projections fit this data", MB_ICONEXCLAMATION, 0);
	}
	}
	SetGlobalValue("%ALT_PROJECTION", SaveAltProj);
	return TRUE;
}
int GuessProjection2(LPSTR fileOfProjections,LPMNMXCORD pFileBounds, int useThisOne,LPSTR ChosenProjection)
{
	MNMXCORD	BoundsInt, FileBoundsCVT, ProjBounds;
	short	PCTIn[32][2], UsedUnits[32][2], CVTUnits[32], CvtID = 0, UnitsID;
	short	nFound = 0, MaxFound = 0, MaxID, MaxUnits;
	BOOL	Err;
	char	SaveAltProj[MAX_PATH], str[4096];
	DPOINT	Points[4];
	LPSTR	pName;
	HFILE	Fid;
	int		CurProj;
	long	Loc;
	short	i;
	char	File[MAX_PATH];

	GetGlobalCVal("[%ALT_PROJECTION]", SaveAltProj, 0);
	GetGlobalCVal("[%PROJECTBOUNDS]", str, 0);
	ProjBounds = atobounds(str, &Err);
	if (!ValidBounds(&ProjBounds))
		return FALSE;
	Fid = GSSiOpenFile(fileOfProjections, 0, OF_READ);
	if (Fid != HFILE_ERROR)
	while (fgetstring (str,255,Fid))
	{
		SetGlobalValue("%ALT_PROJECTION", str);
		PCTIn[CvtID][0] = 0;
		PCTIn[CvtID][1] = 0;
		UnitsID = 0;
		ConvertCoordClose();
		ConvertCoordInit();
		if (PRJ_UNITS[3] == 1 || PRJ_UNITS[3] == 2)
			CVTUnits[CvtID] = 1;
		else
			CVTUnits[CvtID] = PRJ_UNITS[3];
	BeginUnits:
		PRJ_UNITS[3] = CVTUnits[CvtID];
		UsedUnits[CvtID][UnitsID] = PRJ_UNITS[3] - 1;
		DBoundsInit(&FileBoundsCVT);
		Points[0].x = pFileBounds->xmn;
		Points[0].y = pFileBounds->ymn;
		Points[1].x = pFileBounds->xmn;
		Points[1].y = pFileBounds->ymx;
		Points[2].x = pFileBounds->xmx;
		Points[2].y = pFileBounds->ymx;
		Points[3].x = pFileBounds->xmx;
		Points[3].y = pFileBounds->ymn;
		for (i = 0; i<4; i++)
		{
			if (ConvertCoord(&Points[i], 3, 1))
			{
				goto NextUnits;
			}
			AddDPointToMinMax(&Points[i], &FileBoundsCVT);
		}
		if (IntersectBounds(&ProjBounds, &FileBoundsCVT, &BoundsInt))
		{
			PCTIn[CvtID][UnitsID] = IDNINT(10000 * BoundsArea(&BoundsInt) / BoundsArea(&FileBoundsCVT));
			if (PCTIn[CvtID][UnitsID])
			{
				if (PCTIn[CvtID][UnitsID] > MaxFound)
				{
					MaxFound = PCTIn[CvtID][UnitsID];
					MaxID = CvtID;
					MaxUnits = UnitsID;
				}
				nFound++;
			}
		}
	NextUnits:
		if (CVTUnits[CvtID] == 1)
		{
			CVTUnits[CvtID]++;
			UnitsID++;
			goto BeginUnits;
		}
		CvtID++;
	}
	GSSiClose(Fid);

	SetGlobalValue("%ALT_PROJECTION", SaveAltProj);
	return TRUE;
}

BOOL GuessShapeProjection (LPSTR Name,HWND hWnd,UINT ProjCntl,UINT ProjUnits,BOOL UseFirst)
{
	SHPHEADER	SHPHeader;
	HFILE		FidSHP;
	MNMXCORD	SHPBounds;
	double		factor;
	LPSTR		nvp = 0;
	
	switch (MapFileType(Name, 0, 0))
	{
		case MT_SHP:
			FidSHP=GSSiOpenFile (Name,0,OF_READ);
			if (FidSHP == HFILE_ERROR)  
				return FALSE;  
			BigRead (FidSHP,(HPSTR)&SHPHeader,(UINT)sizeof(SHPHeader));
			GSSiClose (FidSHP);      
			SHPBounds = *(LPMNMXCORD)&SHPHeader.Xmin;
			nvp = SHPGetNVP(Name, &factor);
			if (nvp)
			{
				free(nvp);
				return TRUE;
			}
		break;
		
		case MT_PERSONAL_GEO_DB: 
			ReadPGDBHeader (Name,&SHPBounds);
		break;
		case MT_FILE_GEO_DB: 
			ReadFGDBHeader (Name,&SHPBounds);
		break;
	}  
	return GuessProjection (hWnd,ProjCntl,ProjUnits,&SHPBounds,UseFirst);

}  

 

BOOL WriteAdvancedOpts (HFILE FidSave)
{ 
     if (hAttImport)
     {  
     	UINT	Length;
     	short	id=TR_ATTIMPORT;
     	LPSHORT	lpVersion=(LPSHORT)GlobalLock (hAttImport);

     	BigWrite (FidSave,(HPSTR)&id,2,-1);  
     	*lpVersion = 1;
     	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
     	lpVersion++;
     	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
     	Length = *lpVersion++;
     	BigWrite (FidSave,(HPSTR)lpVersion,Length-4,-1);
     	GlobalUnlock (hAttImport);
     }
     if (hStreetSegFields)
     {  
     	short	Length, id=TR_STREET_SEG_FIELDS;
     	LPSHORT	lpVersion=(LPSHORT)GlobalLock (hStreetSegFields);

     	BigWrite (FidSave,(HPSTR)&id,2,-1);  
     	*lpVersion = 1;
     	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
     	lpVersion++;
     	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
     	Length = *lpVersion++;
     	BigWrite (FidSave,(HPSTR)lpVersion,Length-4,-1);
     	GlobalUnlock (hStreetSegFields);
     }
	 if (hImportLimits)
	 {  
	 	short	Length, id=TR_IMPORT_LIMITS;
	 	LPSHORT	lpVersion=(LPSHORT)GlobalLock (hImportLimits);
	
	 	BigWrite (FidSave,(HPSTR)&id,2,-1);  
	 	*lpVersion = 1;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	Length = *lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,Length-4,-1);
	 	GlobalUnlock (hImportLimits);
	 }
	 if (hImportFilter)
	 {  
	 	short	Length, id=TR_IMPORT_FILTER;
	 	LPSHORT	lpVersion=(LPSHORT)GlobalLock (hImportFilter);
	
	 	BigWrite (FidSave,(HPSTR)&id,2,-1);  
	 	*lpVersion = 1;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	Length = *lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,Length-4,-1);
	 	GlobalUnlock (hImportFilter);
	 }
	 if (hGRCommand)
	 {  
	 	short	Length, id=TR_GRCOMMAND;
	 	LPSHORT	lpVersion=(LPSHORT)GlobalLock (hGRCommand);
	
	 	BigWrite (FidSave,(HPSTR)&id,2,-1);  
	 	*lpVersion = 1;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	Length = *lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,Length-4,-1);
	 	GlobalUnlock (hGRCommand);
	 }
	 if (hGRText)
	 {  
	 	short	Length, id=TR_GRTEXT;
	 	LPSHORT	lpVersion=(LPSHORT)GlobalLock (hGRText);
	
	 	BigWrite (FidSave,(HPSTR)&id,2,-1);  
	 	*lpVersion = 1;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	Length = *lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,Length-4,-1);
	 	GlobalUnlock (hGRText);
	 }
	 if (hTimeStamp)
	 {  
	 	short	Length, id=TR_TIMESTAMP;
	 	LPSHORT	lpVersion=(LPSHORT)GlobalLock (hTimeStamp);
	
	 	BigWrite (FidSave,(HPSTR)&id,2,-1);  
	 	*lpVersion = 1;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	Length = *lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,Length-4,-1);
	 	GlobalUnlock (hTimeStamp);
	 }
	 if (hImportRefno)
	 {  
	 	short	Length, id=TR_IMPORT_REFNO;
	 	LPSHORT	lpVersion=(LPSHORT)GlobalLock (hImportRefno);
	
	 	BigWrite (FidSave,(HPSTR)&id,2,-1);  
	 	*lpVersion = 1;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	Length = *lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,Length-4,-1);
	 	GlobalUnlock (hImportRefno);
	 }
	 if (hImportPreSet)
	 {  
	 	short	Length, id=TR_IMPORT_SETUP;
	 	LPSHORT	lpVersion=(LPSHORT)GlobalLock (hImportPreSet);
	
	 	BigWrite (FidSave,(HPSTR)&id,2,-1);  
	 	*lpVersion = 1;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	Length = *lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,Length-4,-1);
	 	GlobalUnlock (hImportPreSet);
	 }
	 return TRUE;
}

void DestroyAdvancedOpts (void)
{
     GSSiGlobFree (&hAttImport);
     GSSiGlobFree (&hStreetSegFields);
	 GSSiGlobFree (&hImportLimits);
	 GSSiGlobFree (&hImportFilter);
	 GSSiGlobFree (&hGRCommand);
	 GSSiGlobFree (&hGRText);
	 GSSiGlobFree (&hTimeStamp);
	 GSSiGlobFree (&hImportRefno);
	 GSSiGlobFree (&hImportPreSet);
	 return;
} 

BOOL ImportCommonCode (HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam,HANDLE hDB) 
{
  HANDLE	hStr=GSSiGlobAlloc ( 427,GMEM_MOVEABLE,256);
  LPSTR	str=GlobalLock (hStr);
  int   TabStops[2]={10,1300},i; 
  BOOL	RemoveOpt, rtn=FALSE; 
  
  switch(Message)
   {
    case WM_INITDIALOG:
         if (GetDlgItem(hWndDlg,IDC_ADVANCED_OPTS))
         	SendDlgItemMessage (hWndDlg,IDC_ADVANCED_OPTS,LB_SETTABSTOPS,1,(LPARAM)&TabStops); 
         LoadTAGDef (); 
    case GSSI_REINITDIALOG:
    {    
    	 char	CheckMark, str[256];
    	 
         ContinueProcessing=TRUE;  
         
         if (!GetDlgItem(hWndDlg,IDC_ADVANCED_OPTS))
        	 break; 
	     SetWindowText (GetDlgItem(hWndDlg,IDC_AVOADD),"Add");
		 EnableWindow (GetDlgItem(hWndDlg,IDC_AVOADD),FALSE); 
		 EnableWindow (GetDlgItem(hWndDlg,IDC_AVOREMOVE),FALSE);
         SendDlgItemMessage (hWndDlg,IDC_ADVANCED_OPTS,LB_RESETCONTENT,0,0);  
         if (hImportRefno)
         	CheckMark = 'X';
         else
         	CheckMark = ' ';
         sprintf (str,"%c\tRefno",CheckMark);
         SendDlgItemMessage (hWndDlg,IDC_ADVANCED_OPTS,LB_ADDSTRING,0,(LPARAM)str);
          if (hImportPreSet)
         	CheckMark = 'X';
         else
         	CheckMark = ' ';
         sprintf (str,"%c\tPre-Processing Setup",CheckMark);
         SendDlgItemMessage (hWndDlg,IDC_ADVANCED_OPTS,LB_ADDSTRING,0,(LPARAM)str);
          if (hImportFilter)
         	CheckMark = 'X';
         else
         	CheckMark = ' ';
         sprintf (str,"%c\tFilter",CheckMark);
         SendDlgItemMessage (hWndDlg,IDC_ADVANCED_OPTS,LB_ADDSTRING,0,(LPARAM)str);
         if (hImportLimits)
         	CheckMark = 'X';
         else
         	CheckMark = ' ';
         sprintf (str,"%c\tSet Geographic Limits",CheckMark);
         SendDlgItemMessage (hWndDlg,IDC_ADVANCED_OPTS,LB_ADDSTRING,0,(LPARAM)str);
         if (hGRText)
         	CheckMark = 'X';
         else
         	CheckMark = ' ';
         sprintf (str,"%c\tText",CheckMark);
         SendDlgItemMessage (hWndDlg,IDC_ADVANCED_OPTS,LB_ADDSTRING,0,(LPARAM)str);
         if (hGRCommand)
         	CheckMark = 'X';
         else
         	CheckMark = ' ';
         sprintf (str,"%c\tImbedded Command",CheckMark);
         SendDlgItemMessage (hWndDlg,IDC_ADVANCED_OPTS,LB_ADDSTRING,0,(LPARAM)str);
         if (hTimeStamp)
         	CheckMark = 'X';
         else
         	CheckMark = ' ';
         sprintf (str,"%c\tTime Stamp",CheckMark);
         SendDlgItemMessage (hWndDlg,IDC_ADVANCED_OPTS,LB_ADDSTRING,0,(LPARAM)str);
         
	}
         goto Exit;

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
         rtn = TRUE;     
         goto Exit;

    case WM_COMMAND:
#if WIN32
         switch(LOWORD(wParam))
#else
         switch(wParam)
#endif
         {  
         	case IDC_EXIT:  
         		Processing = FALSE;
             	SetContinueProcessing ( TRUE); 
             	DestroyAdvancedOpts();
				DestroyFieldList ();
             	ForceRefIndex = ForceTAGIndex = FALSE;
         		goto Exit;  
         		
         	case IDC_AVOADD: 
         		RemoveOpt = FALSE;
         		goto AddAvancedOpt; 
         		
         	case IDC_AVOREMOVE:
         		RemoveOpt = TRUE;
         		goto AddAvancedOpt; 
         		
            case IDC_ADVANCED_OPTS:
			{   
				short	Choice;
				
				switch(HIWORD(wParam))
			    { 
			     case LBN_SELCHANGE:
			     {
			    	BOOL	Enable=TRUE; 
			    	
			     	if ((Choice = SendDlgItemMessage(hWndDlg,IDC_ADVANCED_OPTS,LB_GETCURSEL,0,0)) == LB_ERR)
			     		Enable = FALSE;
			     	else
			     	{
	                	  SendDlgItemMessage(hWndDlg,IDC_ADVANCED_OPTS,LB_GETTEXT,Choice,(DWORD)str); 
	                	  if (*str == 'X')
	                	  	SetWindowText (GetDlgItem(hWndDlg,IDC_AVOADD),"Edit");
			     	}
	                EnableWindow (GetDlgItem(hWndDlg,IDC_AVOADD),Enable);
	                EnableWindow (GetDlgItem(hWndDlg,IDC_AVOREMOVE),Enable);
	                break;
	             }
			     case LBN_DBLCLK: 
			     	RemoveOpt=FALSE;
	AddAvancedOpt:
					{
	        	  	  short	nRc;
	                  DLGPROC lpfnMsgProc=0;
	                  char	DLGName[32]; 
	                  
			          Choice=SendDlgItemMessage(hWndDlg,IDC_ADVANCED_OPTS,LB_GETCURSEL,0,0);
			          if (Choice != LB_ERR)
			          {
	                	  SendDlgItemMessage(hWndDlg,IDC_ADVANCED_OPTS,LB_GETTEXT,Choice,(DWORD)str); 
	                	  if (_fstrstr (str,"Set Geog"))
	                	  {    
	                	  	  if (RemoveOpt) 
	                	  	  	  GSSiGlobFree (&hImportLimits);
	                	  	  else
	                	  	  {
				                  lpfnMsgProc = MakeProcInstance((DLGPROC)IMPORT_LIMITSMsgProc, hInst); 
				                  _fstrcpy (DLGName,"IMPORT_LIMITS");  
				              }
			              }
	                	  else if (_fstrstr (str,"Filter"))
	                	  {   
	                	  	  if (RemoveOpt) 
	                	  	  	  GSSiGlobFree (&hImportFilter);
	                	  	  else
	                	  	  {
				                  lpfnMsgProc = MakeProcInstance((DLGPROC)IMPORT_FILTERMsgProc, hInst); 
				                  _fstrcpy (DLGName,"IMPORT_FILTER");
				              }
			              }
	                	  else if (_fstrstr (str,"Text"))
	                	  {   
	                	  	  if (RemoveOpt) 
	                	  	  	  GSSiGlobFree (&hGRText);
	                	  	  else
	                	  	  {
				                  lpfnMsgProc = MakeProcInstance((DLGPROC)GRTEXTMsgProc, hInst); 
				                  _fstrcpy (DLGName,"GRTEXT");
				              }
			              }
	                	  else if (_fstrstr (str,"Imbedded"))
	                	  {   
	                	  	  if (RemoveOpt) 
	                	  	  	  GSSiGlobFree (&hGRCommand);
	                	  	  else
	                	  	  {
				                  lpfnMsgProc = MakeProcInstance((DLGPROC)GRCOMMANDMsgProc, hInst); 
				                  _fstrcpy (DLGName,"GRCOMMAND");
				              }
			              }
	                	  else if (_fstrstr (str,"Import Att"))
	                	  {   
	                	  	  if (RemoveOpt) 
	                	  	  	  GSSiGlobFree (&hAttImport);
	                	  	  else
	                	  	  {
				                  lpfnMsgProc = MakeProcInstance((DLGPROC)IMPORT_ATTRIBUTESMsgProc, hInst); 
				                  _fstrcpy (DLGName,"IMPORT_ATTRIBUTES");
				              }
			              }
	                	  else if (_fstrstr (str,"Time Stamp"))
	                	  {   
	                	  	  if (RemoveOpt) 
	                	  	  	  GSSiGlobFree (&hTimeStamp);
	                	  	  else
	                	  	  {
				                  lpfnMsgProc = MakeProcInstance((DLGPROC)TIMESTAMPMsgProc, hInst); 
				                  _fstrcpy (DLGName,"TIMESTAMP");
				              }
			              }
	                	  else if (_fstrstr (str,"Street S"))
	                	  {   
	                	  	  if (RemoveOpt) 
	                	  	  	  GSSiGlobFree (&hStreetSegFields);
	                	  	  else
	                	  	  {
				                  lpfnMsgProc = MakeProcInstance((DLGPROC)STREETSEG_FIELDSMsgProc, hInst); 
				                  _fstrcpy (DLGName,"STREETSEG_FIELDS");
				              }
			            
			              }
	                	  else if (_fstrstr (str,"Pre-"))
	                	  {   
	                	  	  if (RemoveOpt) 
	                	  	  	  GSSiGlobFree (&hImportPreSet);
	                	  	  else
	                	  	  {   
	                	  	  	  LPSTR	pStr;
	                	  	  	  LPSETREFNO	lpSetRef; 

	                	  	  	  if (!hImportPreSet)
	                	  	  	  	hImportPreSet = GSSiGlobAlloc ( 428,GHND,sizeof(SETREFNO));
	                	  	  	  lpSetRef = (LPSETREFNO)GlobalLock (hImportPreSet);
	                	  	  	  
       	  	  	  			   	  GetTextString (hWndDlg,lpSetRef->SetRefno,256,"Enter Pre-load settings",0,0,0,TRUE,TRUE);
       	  	  	  			   	  if (*lpSetRef->SetRefno)
       	  	  	  			   	  {
       	  	  	  			   	  	lpSetRef->length = 4 + sizeof(SETREFNO);
       	  	  	  			   	  	GlobalUnlock (hImportPreSet); 
								  }       	  	  	  			   	  	
       	  	  	  			   	  else
       	  	  	  			   	  	GSSiGlobUlFree (&hImportPreSet);
				              }
			              }
	                	  else if (_fstrstr (str,"Refno"))
	                	  {   
	                	  	  if (RemoveOpt) 
	                	  	  	  GSSiGlobFree (&hImportRefno);
	                	  	  else
	                	  	  {   
	                	  	  	  LPSTR	pStr;
	                	  	  	  LPSETREFNO	lpSetRef; 

	                	  	  	  
	                	  	  	  if (!hImportRefno)
	                	  	  	  	hImportRefno = GSSiGlobAlloc ( 429,GHND,sizeof(SETREFNO));
	                	  	  	  lpSetRef = (LPSETREFNO)GlobalLock (hImportRefno);
	                	  	  	  
       	  	  	  			   	  GetTextString (hWndDlg,lpSetRef->SetRefno,256,"Set Refno From",0,0,0,TRUE,TRUE);
       	  	  	  			   	  if (*lpSetRef->SetRefno)
       	  	  	  			   	  {
       	  	  	  			   	  	lpSetRef->length = 4 + sizeof(SETREFNO);
       	  	  	  			   	  	GlobalUnlock (hImportRefno); 
								  }       	  	  	  			   	  	
       	  	  	  			   	  else
       	  	  	  			   	  	GSSiGlobUlFree (&hImportRefno);
				              }
			              }
			              if (lpfnMsgProc)
			              {   
			              	  ImportAttSourcehDB = hDB;
			                  nRc = DialogBox(hInst, (LPSTR)DLGName, hWndDlg, lpfnMsgProc);
			                  FreeProcInstance(lpfnMsgProc);
	                	  }
	                  } 
	                  PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
                	}
				 	  break; 
			    }
			} 
			rtn = TRUE;
			break;
    default:
        break;
   }
   default:
        break;
   }
Exit:
   GSSiGlobUlFree (&hStr);
   return rtn;    
}    

short GetImportFileType (HWND hWndDlg)
{ 
	short	rtn=0;

    if (SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0))
    	rtn = 1;
    if (SendDlgItemMessage (hWndDlg,IDC_DIRECTORY,BM_GETCHECK,0,0))
    	rtn = 2;
    if (SendDlgItemMessage (hWndDlg,IDC_CHRONDIR,BM_GETCHECK,0,0))
    	rtn = 3;
    return rtn;
} 

void SetImportFileType (HWND hWndDlg, short Type)
{
    SendDlgItemMessage (hWndDlg,IDC_EXISTFILE,BM_SETCHECK,FALSE,0L);
    SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_SETCHECK,FALSE,0L);
    SendDlgItemMessage (hWndDlg,IDC_DIRECTORY,BM_SETCHECK,FALSE,0L);
    SendDlgItemMessage (hWndDlg,IDC_CHRONDIR,BM_SETCHECK,FALSE,0L);  
    switch (Type)
    {
    	case 0:
		    SendDlgItemMessage (hWndDlg,IDC_EXISTFILE,BM_SETCHECK,TRUE,0L);
		    break;
    	case 1:
		    SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_SETCHECK,TRUE,0L);
		    break;
    	case 2:
		    SendDlgItemMessage (hWndDlg,IDC_DIRECTORY,BM_SETCHECK,TRUE,0L);
		    break;
    	case 3:
		    SendDlgItemMessage (hWndDlg,IDC_CHRONDIR,BM_SETCHECK,TRUE,0L);
		    break;
	}
	return;
}
			
 

BOOL SetSymParms (LPSTR SymStuff,LPSTR SymName,LPSTR CSymSize,LPSTR CSymRot,LPSTR CSymColor)
{   
	LPSTR pSize, pRot, pColor;
	
    if ((pSize = _fstrchr (SymStuff,';')))
    {
		*pSize++ = 0;
	    if ((pRot = _fstrchr (pSize,';')))
	    {
			*pRot++ = 0;
		    if ((pColor = _fstrchr (pRot,';')))
		    {
				*pColor++ = 0;
			}
			else
				pColor = _fstrchr (SymStuff,0);
		}
		else
			pRot = pColor = _fstrchr (SymStuff,0);
	}
	else
		pSize = pRot = pColor = _fstrchr (SymStuff,0);
	_fstrcpy (SymName,SymStuff);	                
	_fstrcpy (CSymSize,pSize);	                
	_fstrcpy (CSymRot,pRot);	                
	_fstrcpy (CSymColor,pColor);
	return TRUE;
}	                

short StreetNameType (LPSTR STName)
{
    short l, Type;
    
    l = _fstrlen (STName);
    if (!_fstrnicmp (STName,"I-",2))
        Type = 1;
    else if (_fstrstr (STName,"US Hwy"))
        Type = 2;
    else if (_fstrstr (STName,"United States Highway"))
        Type = 2;
    else if (_fstrstr (STName,"US Rte"))
        Type = 2;
    else if (_fstrstr (STName,"State Hwy"))
        Type = 3;
    else if (_fstrstr (STName,"State Highway"))
        Type = 3;
    else if (_fstrstr (STName,"State Rte"))
        Type = 3;
    else if (_fstrstr (STName,"County Hwy"))
        Type = 4;     
    else if (_fstrstr (STName,"County Highway"))
        Type = 4;     
    else if (_fstrstr (STName,"County Rte"))
        Type = 4;
    else if (_fstrstr (STName,"Twp Hwy"))
        Type = 5;
    else if (_fstrstr (STName,"Township Hwy"))
        Type = 5;
    else if (!_fstricmp (&STName[max(0,l-4)],"Frwy"))
        Type = 6;
    else if (!_fstricmp (&STName[max(0,l-3)],"Hwy"))
        Type = 7;
    else if (!_fstricmp (&STName[max(0,l-4)],"Mtwy"))
        Type = 7;  
    else if (!_fstricmp (&STName[max(0,l-5)],"Thoro"))
        Type = 7;  
    else if (!_fstricmp (&STName[max(0,l-4)],"Thwy"))
        Type = 7;  
    else if (!_fstricmp (&STName[max(0,l-4)],"Tpke"))
        Type = 7;  
    else if (_fstrstr (STName,"State Rd"))
        Type = 8;
    else if (_fstrstr (STName,"County Rd"))
        Type = 8;
    else if (_fstrstr (STName,"Twp Rd"))
        Type = 9;
    else if (_fstrstr (STName,"Township"))
        Type = 9;
    else if (_fstrstr (STName,"State"))
        Type = 9;
    else if (_fstrstr (STName,"County"))
        Type = 9;
    else
        Type = 100;
    return Type; 
}
    




 

long GetGSStreetNum (LPSTR Name)
{   
    char    StrName[34], LastName[34];  
    long    StrNum;
    
    if (!hNames2)
   		OpenGSStreetNames (BT_WRITE,3);

    _fstrncpy (StrName,Name,34);
    if (!BT_FIND (hNames2,StrName,BT_FIRST,BT_EQ,(LPSTR)&StrNum))
        return StrNum;
    if (!BT_FIND (hNames1,(LPSTR)&StrNum,BT_LAST,BT_ANY,(LPSTR)&LastName))
        StrNum++;
    else
        StrNum=1;
    BT_PUT (hNames1,(LPSTR)&StrNum,(LPSTR)StrName);
    BT_PUT (hNames2,(LPSTR)StrName,(LPSTR)&StrNum);
    return StrNum;
} 
BOOL PointInFileBounds (LPDPOINT lpDPoint,LPMNMXCORD MinMaxCoord, short Pass)
{
	SetViewport(*pCommandViewport);
    if (!Pass)
    {   
        MinMaxCoord->xmn = min (MinMaxCoord->xmn,lpDPoint->x);
        MinMaxCoord->xmx = max (MinMaxCoord->xmx,lpDPoint->x);
        MinMaxCoord->ymn = min (MinMaxCoord->ymn,lpDPoint->y);
        MinMaxCoord->ymx = max (MinMaxCoord->ymx,lpDPoint->y);
        return FALSE;
    } 
    if (Pass < 0) return FALSE;
    if (lpDPoint->x < CurView->FileMNMX.xmn ||   
        lpDPoint->x > CurView->FileMNMX.xmx ||
        lpDPoint->y < CurView->FileMNMX.ymn ||
        lpDPoint->y > CurView->FileMNMX.ymx)
        return FALSE;    
    else
        return TRUE;
}

void GetTLIDShapePoints (long TLID,LPSHORT NumShapes,HPDPOINT lpShapes,HFILE FidTIGER2)
{   
    static  short     CurNumPoints;
    static	LPTIGER2  Tiger2;  
    static	HANDLE	hTiger2=0;
    short     i, WantEvery=1;
    BTVARDESC   BTVar[2];  
    static  char    TigerShapeIndex[144]=""; 
    static  HANDLE  hIndex;
    long    Offset, CurTLID;
    struct {long    TLID;
            short     seq;} Index;
    
    *NumShapes=0;
    if (TLID == -1)
    {
        hTiger2 = GSSiGlobAlloc ( 444,GMEM_MOVEABLE,sizeof(TIGER2)+4);
        Tiger2 = (LPTIGER2)GlobalLock (hTiger2);
        if (!TigerShapeIndex[0])
            GSSiGetTempFileName (0,"gmt",0,(LPSTR)TigerShapeIndex);
        BTVar[0].BT_VARTYP=BT_INTEGER;
        BTVar[0].BT_VARLEN=4;
        BTVar[0].BT_VAROFF=0;
        BTVar[1].BT_VARTYP=BT_INTEGER;
        BTVar[1].BT_VARLEN=2;
        BTVar[1].BT_VAROFF=4;
        BT_CREATE (TigerShapeIndex, 4, FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
        hIndex= BT_OPEN (TigerShapeIndex, 0, BT_WRITE, 0); 
        Offset = GSSillseek(FidTIGER2,0,1);
        while (fgetstring((LPSTR)Tiger2,sizeof(TIGER2),FidTIGER2))  
        {
            Index.TLID = ldread(Tiger2->TLID,10);
            Index.seq = ldread(Tiger2->RTSQ,3); 
            BT_PUT (hIndex,(LPSTR)&Index,(LPSTR)&Offset);  
            Offset = GSSillseek(FidTIGER2,0,1);
        }
        BT_CLOSE (hIndex);
        hIndex= BT_OPEN (TigerShapeIndex, 0, BT_READ, 0); 
        return;
    }
    if (TLID == -2)
    {   
    	GSSiGlobUlFree (&hTiger2);
        BT_CLOSE (hIndex);
        GSSiRemove (TigerShapeIndex);
        return;
    } 
    Index.TLID = TLID;
    Index.seq = 1;
    
    if (!BT_FIND (hIndex,(LPSTR)&Index,BT_FIRST,BT_EQ,(LPSTR)&Offset)) 
    {
        CurTLID = Index.TLID;      
        GSSillseek (FidTIGER2,Offset,0); 
        fgetstring((LPSTR)Tiger2,sizeof(TIGER2),FidTIGER2);       
    }
    else
        CurTLID = LONG_MAX;
    while (CurTLID == TLID)  
    {   
        for (i=0;i<10;i++)
        {
            lpShapes->x = dread((LPSTR)&Tiger2->LONGLAT[i].Longitude,10)/1000000;
            lpShapes->y = dread((LPSTR)&Tiger2->LONGLAT[i].Latitude,9)/1000000;
            if (lpShapes->x != 0 && ((i+1)%WantEvery) == 0)
            {
                (*NumShapes)++;
                lpShapes++;
            }
        }
        if (fgetstring((LPSTR)Tiger2,sizeof(TIGER2),FidTIGER2))
            CurTLID = ldread(Tiger2->TLID,10); 
        else
            CurTLID = LONG_MAX;
    }
    return;
}

BOOL WantThisTLID (long TLID,LPTIGER1 Tiger1,LPBOOL WantAreas, short Pass,HANDLE hCFCC)
{   
    LPSHORT   pNumCFCC;
    LPSTR   lpCFCC; 
    short     i;
    
    if (!Pass) return TRUE; 
    if (Pass<0)
    {
        if (Tiger1->CFCC[0]=='A')
            return TRUE;
        else
            return FALSE;
    }
    if (!hCFCC)
    	return TRUE;
    pNumCFCC = (LPSHORT)GlobalLock (hCFCC);
    lpCFCC = (LPSTR)pNumCFCC;
    lpCFCC+=2;
    for (i=0;i<*pNumCFCC;i++,lpCFCC+=3)
    {
        if (!_fstrncmp(Tiger1->CFCC,lpCFCC,3))
        {
            GlobalUnlock (hCFCC);
            return TRUE;         
        }
    }
    GlobalUnlock (hCFCC);
    return FALSE;
}

BOOL LoadCFCCTable (void)
{   HANDLE hDB;
    LPGWFLDINFO lpGWFldInfo;
    LPGWDHEADER lpGWDHead;
    HANDLE      hBT, hSQL, hSym;
    long        Offset; 
    double      rtn;
    LPVOID      lpVal; 
    char        str1[16], str2[64];
    short         st, i, len,ifield;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr;
    LPFIELDINFO lpFieldInfo; 
    HANDLE      SaveHandle;
    LPFILEPATH  FilePathPtr; 
    BOOL        More;
    short         rc;   
    LPSYMBOL    CurSymbol;
    
    hSQL = 0;
    if (!OpenDataFile ("[%DL]cfccall.txt","",BT_READ,&hSQL))
        return FALSE;      
    
    if (!OpenSymDict (OF_READWRITE))
	{
		CloseDataFile (TRUE, &hSQL);   
		return FALSE;
	}	
    SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
    FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
    
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
    _fstrcpy (CurSymbol->Name,"ALL");
    _fstrcpy (CurSymbol->Desc,"Parent of All");  
    GlobalUnlock (hSym);
    SaveSymbol(hSym,0);
	DestroySymbol (hSym);

	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
    _fstrcpy (CurSymbol->Name,"TIGER");
    _fstrcpy (CurSymbol->Desc,"TIGER file data");
    CurSymbol->Type = 0;
    CurSymbol->NumElements=0;
    CurSymbol->Parent=GetDictSymbolNumber("ALL");        
    GlobalUnlock (hSym);
    SaveSymbol(hSym,0);
	DestroySymbol (hSym);
    More=TRUE;
    while (FetchDBRec (hSQL))
    {
		hSym = AllocateNewSymbol ();
		CurSymbol = (LPSYMBOL)GlobalLock (hSym);
        GetValFromOpenFiles ("CFCC",CurSymbol->Name,64);
        GetValFromOpenFiles ("DESC",CurSymbol->Desc,64);
        CurSymbol->Parent=GetDictSymbolNumber("TIGER");
        CurSymbol->Type = 2;         
	    GlobalUnlock (hSym);
	    SaveSymbol(hSym,0);
		DestroySymbol (hSym);
    }
    GlobalUnlock (SQLPtr->OFHandle);  
    GlobalUnlock (hSQL);
    CloseDataFile (TRUE, &hSQL);   
    CloseSymDict();
    return TRUE;
} 

void DisplayTIGERName (long refno)
{
    HANDLE      hBTTiger1Out;
    HFILE   FidTiger1Out;
    long    Offset;  
    OFSTRUCTGM    OFStruct;
    TIGER1_PEOPLENET    Tiger1PN;
    char    Name[34];  
    HMENU   NameMenu;
    POINT   position;  
    short     i; 
    char    str[128];  
    BOOL    HaveName=FALSE;
    long    SNum;
    
    NameMenu = CreatePopupMenu();  
    for (i=0;i<4;i++)
        if ((SNum=labs(CurStreetNumbers[i])))
            if (GetTrueStreetName (SNum, Name, CurState,0))
            {
                AppendMenu (NameMenu,MF_ENABLED|MF_STRING,0,Name); 
                HaveName = TRUE;
            }
    if (!HaveName)
        AppendMenu (NameMenu,MF_ENABLED|MF_STRING,0,"Unnamed Street"); 
    if (GetTrueStreetName (FromStreet, Name, CurState,0))
    {
        sprintf (str,"From: %s",Name);
        AppendMenu (NameMenu,MF_ENABLED|MF_STRING,0,str);  
    }
    if (GetTrueStreetName (ToStreet, Name, CurState,0))
    {
        sprintf (str,"To:   %s",Name);
        AppendMenu (NameMenu,MF_ENABLED|MF_STRING,0,str);  
    }
        
    CloseGSStreetNames();
	CloseStreetNameTable();
    GetCursorPos (&position);  
    TrackPopupMenu (NameMenu,TPM_LEFTBUTTON,position.x,position.y,0,CurView->hWnd,0);
    DestroyMenu (NameMenu);
    return;
} 

void ReadSegData (long Offset, LPSEGDATA pSegdata)
{   UINT	len;

  	GSSillseek (SegDataFid,Offset,0);
   	BigRead (SegDataFid,(HPSTR)&len,2);
   	BigRead (SegDataFid,(HPSTR)pSegdata,len);
   	return;

}

void CreateIntersectionFile (BOOL GeoMaster,HWND hWndStatus)
{
	BTVARDESC	BTVar[3];
	long		nRecs, nLoaded, TLID;
	HDC			hDC;
	int			stSeg, st2;
	short		len;
	BOOL		RtnVal=FALSE;
	long			SegMaxData=0, Offset, TotLen, Processed=0;
	time_t ltime;
	HANDLE		hStname1, hStname2, hStname;
	char		name[34], File[128], State[32], mess[128]; 
	HFILE		Tiger1FID; 
	TIGER1		Tiger1;
	TIGER1_MN	Tiger1PN;
	SEGDATA		Segdata;

	HCURSOR	hcurSave;

	if (!hWndStatus)
		hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=4;
	BTVar[2].BT_VARTYP=BT_INTEGER;
	BTVar[2].BT_VARLEN=4;
	BTVar[2].BT_VAROFF=8;   
	if (GeoMaster)	
		_fstrcpy (File,"[%DL][%STATE]\\intersec.btr");
	else
		_fstrcpy (File,"[%GEOSPAN_LOC]intersec.btr");
	BT_CREATE (File, sizeof(IntersectData), FALSE, 3, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	ltime = 0;
	hIntersect = BT_OPEN (File, ltime, BT_WRITE, 0);  
	if (GeoMaster) 
	{   
		OFSTRUCTGM	OFStruct;
		
		hSegData = BT_OPEN ("[%DL][%STATE]\\tiger1.btr", 0, BT_READ, 0); 
		Tiger1FID = GSSiOpenFile("[%DL][%STATE]\\tiger1.dat",&OFStruct,OF_READ);
		_fstrcpy (State,"[%STATE]");
		ExpandText (State);
		TotLen = GSSillseek (Tiger1FID,0,2);
	}
    stSeg = BT_FIND (hSegData,(LPSTR)&TLID,BT_FIRST,BT_ANY,(LPSTR)&Offset);
    while (!stSeg)
    {    
    	if (GeoMaster)
    	{
/*    		GSSillseek (Tiger1FID,Offset,0);
    		BigRead (Tiger1FID,&Tiger1,sizeof(TIGER1));
    		Segdata.frlong = ldread(Tiger1.FRLONG,10);
    		Segdata.frlat = ldread(Tiger1.FRLAT,10);
    		Segdata.tolong = ldread(Tiger1.TOLONG,10);
    		Segdata.tolat = ldread(Tiger1.TOLAT,10); 
    		Segdata.street_num = 0; 
*/
    		GSSillseek (Tiger1FID,Offset,0);
    		Processed+=BigRead (Tiger1FID,(HPSTR)&len,2); 
    		Processed+=BigRead (Tiger1FID,(HPSTR)&Tiger1PN,sizeof(Tiger1PN));
    		Segdata.frlong = Tiger1PN.FRLONG;
    		Segdata.frlat = Tiger1PN.FRLAT;
    		Segdata.tolong = Tiger1PN.TOLONG;
    		Segdata.tolat = Tiger1PN.TOLAT; 
    		Segdata.street_num = Tiger1PN.StreetNum[0]; 
            PctBox (hWndStatus, TotLen, Processed,0);   
    	}
    	else
   			ReadSegData (Offset,&Segdata);
   		IntersectKey.Long = Segdata.frlong; 
   		IntersectKey.Lat = Segdata.frlat; 
   		IntersectKey.TLID = TLID;  
   		IntersectData.Snum = Segdata.street_num;
   		IntersectData.OPLong = Segdata.tolong;
   		IntersectData.OPLat = Segdata.tolat;
    	BT_PUT (hIntersect,(LPSTR)&IntersectKey,(LPSTR)&IntersectData);
   		IntersectKey.Long = Segdata.tolong; 
   		IntersectKey.Lat = Segdata.tolat; 
   		IntersectKey.TLID = TLID;
   		IntersectData.Snum = Segdata.street_num;
   		IntersectData.OPLong = Segdata.frlong;
   		IntersectData.OPLat = Segdata.frlat;
    	BT_PUT (hIntersect,(LPSTR)&IntersectKey,(LPSTR)&IntersectData);
	    stSeg = BT_FIND (hSegData,(LPSTR)&TLID,BT_NEXT,BT_ANY,(LPSTR)&Offset);
	}
	BT_CLOSE (hIntersect);
	if (GeoMaster)
	{
		BT_CLOSE (hSegData); 
		hIntersect = 0;
		GSSiClose (Tiger1FID);
	}  
	else
		hIntersect = BT_OPEN (File,ltime, BT_READ, 0);
	
	if (!hWndStatus)
		GSSiSetCursor (hcurSave);
	return;
} 
