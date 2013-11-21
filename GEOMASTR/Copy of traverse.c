#include "graphint.h"

#include "gmextern.h"
extern	BOOL CALLBACK EnumCtrlProc(HWND hCtrl,LONG lParam); 

#define	DEST_UPDATE	1
#define	DEST_TEST	2
#define DEST_WORKING	3

static	long	nTravPoints=0, nLegs=0,SubTravStartLeg;
static	HANDLE	hLegData=0, hTravPoints=0, hSnappedPoints=0;
static	HANDLE	hSnapStatus=0; 
static	HANDLE	hPointType=0; 
static	short	POBItem;
static	TRAVLEGDATA CurLegData;  
static	TRAVLEGDATA LastDANDBLegData;  
static	long	CurTravID=0;          
static	double	AZAdjust=0;  
static	int		TravDest=DEST_TEST;
static	BOOL	HaveClosure=FALSE; 
static	BOOL	HaveBoundaryLines=FALSE;  
static	MNMXCORD	TravBounds; 
static	HANDLE	hTravTran=0;

BOOL DecodeTYPE1CRVParm (LPSTR Parm,LPDOUBLE pRadius,LPDOUBLE pDist,LPDOUBLE pAZ,LPSHORT pDir,LPSHORT pDistOpt,LPSHORT pBearOpt);
BOOL ComputeTYPE1CRV (double Radius,double Dist,double AZ,short Dir,short DistOpt,short BearOpt,DPOINT PC,
					  LPDPOINT pPOC, LPDPOINT pPT,LPDPOINT pRP,
					  LPDOUBLE pArcDist,LPDOUBLE pTanDist,LPDOUBLE pChordDist,
					  LPDOUBLE pTanAZ, LPDOUBLE pRPAZ, LPDOUBLE pCrdAZ, LPDOUBLE pAngle);
BOOL ShowTrav (HWND hWndDlg,UINT IDCTRAVLIST,long CurTravID);
void LoadTravAddLegOptions (HWND hWndDlg,UINT IDCADDLEG,int Opt);
void TravShowSelectedLeg (HWND hWndDlg,UINT IDCTRAVLIST,UINT IDCCALLNO,int choice);
BOOL TravGetDefaultSymbol (LPSTR CurTAG,LPSTR SymName,int Which); 
void AdjustSubTraverse (HWND hWndDlg,UINT IDCTRAVLIST,int From,int Increment);
BOOL LoadTravTAGs (HWND hWndDlg,UINT IDCTAPREFIX);
BOOL TravAllowUpdateLayer (LPSTR CurTAG);
BOOL SaveTravTransformation (HANDLE hPoints,HANDLE hSnapStatus,long nPoints);
void RemoveTravTranFile (void);

BOOL LoadTravTAGs (HWND hWndDlg,UINT IDCTAPREFIX)
{
	HFILE Fid=GSSiOpenFile ("[%DL]travtags.txt",NULL,OF_READ);
	char	str[260]; 
	LPSTR	pComma;
	
	if (Fid == HFILE_ERROR)
	{
		MessageBox (0,"travtags.txt file not found",NULL,MB_ICONEXCLAMATION);
		return FALSE;
	}
	while (fgetstring (str,256,Fid))
	{   
		if (*str == '#')
			continue;
		if ((pComma = _fstrchr (str,',')))
			*pComma = 0;
    	SendDlgItemMessage (hWndDlg,IDCTAPREFIX,CB_ADDSTRING,0,(LPARAM)str);
     }
     GSSiClose (Fid);
     return TRUE;
}

BOOL TravGetDefaultSymbol (LPSTR CurTAG,LPSTR SymName,int Which)
{   
	HFILE	Fid; 
	LPSTR	pComma;
	char	line[260];    
	BOOL	rtn=FALSE;
			
	_fstrcpy (SymName,"");
	Fid = GSSiOpenFile("[%DL]travtags.txt",NULL,OF_READ);
	if (Fid != HFILE_ERROR)
	{   
		while (fgetstring (line,256,Fid))
		{   
			if (*line == '#')
				continue;
			if ((pComma=_fstrrchr (line,',')))
			{
				*pComma++ = 0;   
				rtn = atob (pComma);
			}
			if ((pComma=_fstrchr (line,','))) 
			{
				*pComma++=0; 
				_fstrcpy (SymName,pComma);
				if (Which > 1)
				{
					if ((pComma=_fstrchr (pComma,','))) 
					{
						*pComma++=0; 
						_fstrcpy (SymName,pComma);
						if (Which > 2) 
						{
							if ((pComma=_fstrchr (pComma,','))) 
							{
								*pComma++=0; 
								_fstrcpy (SymName,pComma); 
							}
						}
					}
				}
			} 
			if ((pComma=_fstrchr (SymName,','))) 
				*pComma++=0; 
			
			if (!_fstricmp (CurTAG,line))
				break;
		}
		GSSiClose (Fid);
	}  
	return rtn;
} 

BOOL TravAllowUpdateLayer (LPSTR CurTAG)
{   
	HFILE	Fid; 
	LPSTR	pComma;
	char	line[260];  
	BOOL	rtn=FALSE;
			
	Fid = GSSiOpenFile("[%DL]travtags.txt",NULL,OF_READ);
	if (Fid != HFILE_ERROR)
	{   
		while (fgetstring (line,256,Fid))
		{   
			if (*line != '#')
			{
				if ((pComma=_fstrrchr (line,',')))
					*pComma++ = 0; 
				rtn = atob (pComma);
				if ((pComma=_fstrchr (line,','))) 
					*pComma++=0; 
				if (!_fstricmp (CurTAG,line))
					break; 
			}
		}
		GSSiClose (Fid);
	}  
	return rtn;
} 

double GetAzAdjustment (DPOINT WPoint)
{   
	DPOINT	Point1, Point2;
	
	double Adjust; 
	
	ConvertCoord(&WPoint,1,3); 
	
	Point1 = Point2 = WPoint;
	Point1.y -= 0.25;
	Point2.y += 0.25;
	ConvertCoord(&Point1,3,1); 
	ConvertCoord(&Point2,3,1); 
	Adjust = getazd (&Point1,&Point2)-HALFPI;
	return Adjust;
}

BOOL FAR PASCAL TRAVTYPE1CRVMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 	int		st, choice, n,i, Maxval;
	char	str[256], DistUnits[16],DegC[16],MinC[16],SecC[16],RadUnits[16], BearOpt[16], DistOptC[16];
	static	char	PreDir[4]="N", PostDir[4]="E"; 
	static	char	CurDist[256]="",CrvDir[8]="Right",CurChordLen[64];
	static	short	CurDistUnitsA=0, CurDistUnitsCRD=0, CurDistUnitsT=0, CurDistUnitsR=0;  
	static	double	AZ;      
	static	UINT	SetVal;
	static	double	SEC=0; 
	static	short	DEG=0,MIN=0; 
	static	UINT	LastBType = IDC_USETANBEAR; 
	static	UINT	LastDType = IDC_USEARCDIST; 
	static	BOOL	First;
	BOOL	Err, NORTH,EAST;
	LPSTR	lpTAB, pParm, pEnd;   
	short	DistOpt,iDistOpt,iBearOpt,iCrvDir,IRC; 
	double	Dist,ROT, ChordLen, RadLen;  
	UINT	DistUnitsCntl[4]={IDC_DISTUNITSR,IDC_DISTUNITSCRD,IDC_DISTUNITST,IDC_DISTUNITSA}; 
	UINT	Next;  
	UINT	PreDirCntl,	BDegCntl, BMinCntl,	BSecCntl, PostDirCntl, DistUnCntl, DistCntl;


 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
    	 First = TRUE;
         SendDlgItemMessage (hWndDlg,IDC_DIRCURVE,CB_ADDSTRING,0,(LPARAM)"Right");
         SendDlgItemMessage (hWndDlg,IDC_DIRCURVE,CB_ADDSTRING,0,(LPARAM)"Left");
         for (i=0;i<4;i++)
         {
	         SendDlgItemMessage (hWndDlg,DistUnitsCntl[i],CB_ADDSTRING,0,(LPARAM)"Feet");
	         SendDlgItemMessage (hWndDlg,DistUnitsCntl[i],CB_ADDSTRING,0,(LPARAM)"Meters");
	         SendDlgItemMessage (hWndDlg,DistUnitsCntl[i],CB_ADDSTRING,0,(LPARAM)"Yards");
	         SendDlgItemMessage (hWndDlg,DistUnitsCntl[i],CB_ADDSTRING,0,(LPARAM)"Miles");
	         SendDlgItemMessage (hWndDlg,DistUnitsCntl[i],CB_ADDSTRING,0,(LPARAM)"Kilometers"); 
	     }
		 SendDlgItemMessage(hWndDlg,IDC_DISTUNITSR,CB_SETCURSEL,CurDistUnitsR,NULL); 
		 SendDlgItemMessage(hWndDlg,IDC_DISTUNITSCRD,CB_SETCURSEL,CurDistUnitsCRD,NULL); 
		 SendDlgItemMessage(hWndDlg,IDC_DISTUNITST,CB_SETCURSEL,CurDistUnitsT,NULL); 
		 SendDlgItemMessage(hWndDlg,IDC_DISTUNITSA,CB_SETCURSEL,CurDistUnitsA,NULL);
		 AZToBear (CurrentAZ,PreDir,DegC,MinC,SecC,PostDir);
		 SetDlgItemText (hWndDlg,IDC_PREDIRT,PreDir);
		 SetDlgItemText (hWndDlg,IDC_BDEGT,DegC);
		 SetDlgItemText (hWndDlg,IDC_BMINT,MinC);
		 SetDlgItemText (hWndDlg,IDC_BSECT,SecC);
		 SetDlgItemText (hWndDlg,IDC_POSTDIRT,PostDir);
		 SendDlgItemMessage (hWndDlg,LastDType,BM_SETCHECK,TRUE,0L);
		 SendDlgItemMessage (hWndDlg,LastBType,BM_SETCHECK,TRUE,0L);
	     SendMessage(hWndDlg, WM_COMMAND, LastDType, 0L); 
	     SendMessage(hWndDlg, WM_COMMAND, LastBType, 0L); 

    case GSSI_REINITDIALOG:
	{
		short	Deg, Min, ndp;
		double	Sec, decdeg, Angle, ArcDist, TanDist, ChordDist, TanAZ, RPAZ, CrdAZ; 
		DPOINT	PC={0,0}, POC, PT, RP; 
		
    	_fstrcpy (str,CurLegData.Parameters);   
		if (DecodeTYPE1CRVParm (str,&RadLen,&Dist,&AZ,&iCrvDir,&iDistOpt,&iBearOpt))
		{
			ComputeTYPE1CRV (RadLen,Dist,AZ+AZAdjust,iCrvDir,iDistOpt,iBearOpt,PC,
							 &POC,&PT,&RP,&ArcDist,&TanDist,&ChordDist,&TanAZ,&RPAZ,&CrdAZ,&Angle);
			  
			decdeg = Angle * DEGRAD;
			GetDMS (decdeg,&Deg,&Min,&Sec); 
			sprintf (str,"Angle = %2.2iD %2.2iM %.1fS",abs(Deg),Min,Sec);
			SetDlgItemText (hWndDlg,IDC_MESS,str);  
			if (iCrvDir == 1)
         		SendDlgItemMessage (hWndDlg,IDC_DIRCURVE,CB_SELECTSTRING,-1,(LPARAM)"Right");
			else
         		SendDlgItemMessage (hWndDlg,IDC_DIRCURVE,CB_SELECTSTRING,-1,(LPARAM)"Left");
         	switch (iBearOpt)
			{                
				case 1:
					SendDlgItemMessage (hWndDlg,IDC_USERADBEAR,BM_SETCHECK,FALSE,0L);
					SendDlgItemMessage (hWndDlg,IDC_USETANBEAR,BM_SETCHECK,TRUE,0L);
					SendDlgItemMessage (hWndDlg,IDC_USECHORDBEAR,BM_SETCHECK,FALSE,0L);
				    PostMessage(hWndDlg, WM_COMMAND, IDC_USETANBEAR, 0L);
			    break; 
			    case 2:
					SendDlgItemMessage (hWndDlg,IDC_USERADBEAR,BM_SETCHECK,TRUE,0L);
					SendDlgItemMessage (hWndDlg,IDC_USETANBEAR,BM_SETCHECK,FALSE,0L);
					SendDlgItemMessage (hWndDlg,IDC_USECHORDBEAR,BM_SETCHECK,FALSE,0L);
				    PostMessage(hWndDlg, WM_COMMAND, IDC_USERADBEAR, 0L); 
				break;
			    case 3:
					SendDlgItemMessage (hWndDlg,IDC_USECHORDBEAR,BM_SETCHECK,TRUE,0L);
					SendDlgItemMessage (hWndDlg,IDC_USETANBEAR,BM_SETCHECK,FALSE,0L);
					SendDlgItemMessage (hWndDlg,IDC_USERADBEAR,BM_SETCHECK,FALSE,0L);
				    PostMessage(hWndDlg, WM_COMMAND, IDC_USECHORDBEAR, 0L); 
				break;
				
			}
    	 		
			AZToBear (TanAZ,PreDir,DegC,MinC,SecC,PostDir);
			SetDlgItemText (hWndDlg,IDC_PREDIRT,PreDir);
	    	SetDlgItemText (hWndDlg,IDC_BDEGT,DegC); 
	    	SetDlgItemText (hWndDlg,IDC_BMINT,MinC);
	    	SetDlgItemText (hWndDlg,IDC_BSECT,SecC); 
			SetDlgItemText (hWndDlg,IDC_POSTDIRT,PostDir);  
			AZToBear (RPAZ,PreDir,DegC,MinC,SecC,PostDir);
			SetDlgItemText (hWndDlg,IDC_PREDIRR,PreDir);
	    	SetDlgItemText (hWndDlg,IDC_BDEGR,DegC); 
	    	SetDlgItemText (hWndDlg,IDC_BMINR,MinC);
	    	SetDlgItemText (hWndDlg,IDC_BSECR,SecC); 
			SetDlgItemText (hWndDlg,IDC_POSTDIRR,PostDir);  
			AZToBear (CrdAZ,PreDir,DegC,MinC,SecC,PostDir);
			SetDlgItemText (hWndDlg,IDC_PREDIRC,PreDir);
	    	SetDlgItemText (hWndDlg,IDC_BDEGC,DegC); 
	    	SetDlgItemText (hWndDlg,IDC_BMINC,MinC);
	    	SetDlgItemText (hWndDlg,IDC_BSECC,SecC); 
			SetDlgItemText (hWndDlg,IDC_POSTDIRC,PostDir);  
    	 	switch (iDistOpt)
			{   
				case 1:
					SendDlgItemMessage (hWndDlg,IDC_USEARCDIST,BM_SETCHECK,TRUE,0L);
					SendDlgItemMessage (hWndDlg,IDC_USECHORDDIST,BM_SETCHECK,FALSE,0L);
					SendDlgItemMessage (hWndDlg,IDC_USETANDIST,BM_SETCHECK,FALSE,0L); 
			        PostMessage(hWndDlg, WM_COMMAND, IDC_USEARCDIST, 0L);
					break; 
					
				case 2:
					SendDlgItemMessage (hWndDlg,IDC_USEARCDIST,BM_SETCHECK,FALSE,0L);
					SendDlgItemMessage (hWndDlg,IDC_USECHORDDIST,BM_SETCHECK,FALSE,0L);
					SendDlgItemMessage (hWndDlg,IDC_USETANDIST,BM_SETCHECK,TRUE,0L); 
			        PostMessage(hWndDlg, WM_COMMAND, IDC_USETANDIST, 0L);
					break;  
					
				case 3:
					SendDlgItemMessage (hWndDlg,IDC_USEARCDIST,BM_SETCHECK,FALSE,0L);
					SendDlgItemMessage (hWndDlg,IDC_USECHORDDIST,BM_SETCHECK,TRUE,0L);
					SendDlgItemMessage (hWndDlg,IDC_USETANDIST,BM_SETCHECK,FALSE,0L); 
			        PostMessage(hWndDlg, WM_COMMAND, IDC_USECHORDDIST, 0L);
					break;
            } 
			DistOpt=SendDlgItemMessage(hWndDlg,IDC_DISTUNITSR,CB_GETCURSEL,NULL,NULL); 
			RadLen = ConvertDist (RadLen,DistOpt+1);             
    	 	sprintf (str,"%.3f",RadLen);
	    	SetDlgItemText (hWndDlg,IDC_RADIUS,str);
			DistOpt=SendDlgItemMessage(hWndDlg,IDC_DISTUNITSA,CB_GETCURSEL,NULL,NULL); 
			ArcDist = ConvertDist (ArcDist,DistOpt+1);             
    	 	sprintf (str,"%.3f",ArcDist);
	    	SetDlgItemText (hWndDlg,IDC_ARCDISTANCE,str);
			DistOpt=SendDlgItemMessage(hWndDlg,IDC_DISTUNITSCRD,CB_GETCURSEL,NULL,NULL); 
			ChordDist = ConvertDist (ChordDist,DistOpt+1);             
    	 	sprintf (str,"%.3f",ChordDist);
	    	SetDlgItemText (hWndDlg,IDC_CHORDDISTANCE,str);
			DistOpt=SendDlgItemMessage(hWndDlg,IDC_DISTUNITST,CB_GETCURSEL,NULL,NULL); 
			TanDist = ConvertDist (TanDist,DistOpt+1);             
    	 	sprintf (str,"%.3f",TanDist);
	    	SetDlgItemText (hWndDlg,IDC_TANDISTANCE,str);  
	    	First = FALSE;
	     } 
	}
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
         {  
         	case IDC_USEARCDIST:  
         		 LastDType = IDC_USEARCDIST;
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_ARCDISTANCE),TRUE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_TANDISTANCE),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_CHORDDISTANCE),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_DISTUNITSA),TRUE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_DISTUNITSCRD),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_DISTUNITST),FALSE);  
            	 SetFocus (GetDlgItem(hWndDlg,IDC_ARCDISTANCE)); 
            	 SendDlgItemMessage(hWndDlg, IDC_ARCDISTANCE,EM_SETSEL, 0, MAKELONG(0, -1));  
         		 
         		 break;
         		 
         	case IDC_USECHORDDIST: 
         		 LastDType = IDC_USECHORDDIST;
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_ARCDISTANCE),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_TANDISTANCE),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_CHORDDISTANCE),TRUE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_DISTUNITSA),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_DISTUNITSCRD),TRUE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_DISTUNITST),FALSE); 
            	 if (!First)
            	 	SetFocus (GetDlgItem(hWndDlg,IDC_CHORDDISTANCE)); 
            	 SendDlgItemMessage(hWndDlg, IDC_CHORDDISTANCE,EM_SETSEL, 0, MAKELONG(0, -1));  
         		 break;
         		 
         	case IDC_USETANDIST: 
         		 LastDType = IDC_USETANDIST;
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_ARCDISTANCE),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_TANDISTANCE),TRUE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_CHORDDISTANCE),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_DISTUNITSA),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_DISTUNITSCRD),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_DISTUNITST),TRUE);
         		 if (!First) 
            	 	SetFocus (GetDlgItem(hWndDlg,IDC_TANDISTANCE)); 
            	 SendDlgItemMessage(hWndDlg, IDC_TANDISTANCE,EM_SETSEL, 0, MAKELONG(0, -1));  
         		 break;
         		 
         	case IDC_USETANBEAR:
         		 LastBType = IDC_USETANBEAR;
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_PREDIRT),TRUE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BDEGT),TRUE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BMINT),TRUE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BSECT),TRUE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_POSTDIRT),TRUE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_PREDIRR),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BDEGR),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BMINR),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BSECR),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_POSTDIRR),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_PREDIRC),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BDEGC),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BMINC),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BSECC),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_POSTDIRC),FALSE);
         		 if (!First) 
            	 	SetFocus (GetDlgItem(hWndDlg,IDC_PREDIRT)); 
            	 SendDlgItemMessage(hWndDlg, IDC_PREDIRT,EM_SETSEL, 0, MAKELONG(0, -1));  
         		 break;
         		 
         	case IDC_USERADBEAR: 
         		 LastBType = IDC_USERADBEAR;
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_PREDIRT),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BDEGT),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BMINT),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BSECT),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_POSTDIRT),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_PREDIRR),TRUE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BDEGR),TRUE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BMINR),TRUE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BSECR),TRUE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_POSTDIRR),TRUE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_PREDIRC),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BDEGC),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BMINC),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BSECC),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_POSTDIRC),FALSE);
         		 if (!First) 
            	 	SetFocus (GetDlgItem(hWndDlg,IDC_PREDIRR)); 
            	 SendDlgItemMessage(hWndDlg, IDC_PREDIRR,EM_SETSEL, 0, MAKELONG(0, -1));  
         		 break;
         	
         	case IDC_USECHORDBEAR:  
         		 LastBType = IDC_USECHORDBEAR;
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_PREDIRT),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BDEGT),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BMINT),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BSECT),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_POSTDIRT),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_PREDIRC),TRUE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BDEGC),TRUE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BMINC),TRUE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BSECC),TRUE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_POSTDIRC),TRUE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_PREDIRR),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BDEGR),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BMINR),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_BSECR),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_POSTDIRR),FALSE);
         		 if (!First) 
            	 	SetFocus (GetDlgItem(hWndDlg,IDC_PREDIRC)); 
            	 SendDlgItemMessage(hWndDlg, IDC_PREDIRC,EM_SETSEL, 0, MAKELONG(0, -1));  
         		 break;
         	
         	case IDC_DIRCURVE:
                 switch (HIWORD(lParam))
                 {      
		            case CBN_SELCHANGE:  
                    	SetFocus (GetDlgItem(hWndDlg,IDC_RADIUS)); 
                    	SendDlgItemMessage(hWndDlg, IDC_RADIUS,EM_SETSEL, 0, MAKELONG(0, -1));
                    break;
                 }
         		 break;
         	
         	case IDC_PREDIRR:
            case IDC_PREDIRT:  
            case IDC_PREDIRC:
                 switch (HIWORD(lParam))
                 {      
            		case EN_KILLFOCUS:
		            case EN_CHANGE:  
                        GetDlgItemText (hWndDlg,wParam,str,2);
                        switch (*str)
                        {
                        	case 'n': 
                        		SetDlgItemText (hWndDlg,wParam,"N"); 
                        		break;
                        	case 's':
                        		SetDlgItemText (hWndDlg,wParam,"S");
                        		break;
                        	case 'N':
                        	case 'S':
                        		break;
                        	default:
                				Sound (BAD_SOUND);
		                    	SetFocus (GetDlgItem(hWndDlg,wParam)); 
		                    	SendDlgItemMessage(hWndDlg, wParam,EM_SETSEL, 0, MAKELONG(0, -1));
                        		return TRUE;
                        }
                        if (wParam == IDC_PREDIRT)  
                        	Next = IDC_BDEGT;
                        else if (wParam == IDC_PREDIRR)
           	         		Next=IDC_BDEGR;	
           	         	else
           	         		Next=IDC_BDEGC;
           	         	if (HIWORD(lParam) == EN_CHANGE)
           	         	{ 	 
	                    	SetFocus (GetDlgItem(hWndDlg,Next)); 
	                    	SendDlgItemMessage(hWndDlg, Next,EM_SETSEL, 0, MAKELONG(0, -1));  
	                    }
                    break;
                 }
            break;
            
            case IDC_POSTDIRR:  
            case IDC_POSTDIRT:
            case IDC_POSTDIRC:  
                 switch (HIWORD(lParam))
                 {      
            		case EN_KILLFOCUS:
		            case EN_CHANGE:  
                        GetDlgItemText (hWndDlg,wParam,str,2);
                        switch (*str)
                        {
                        	case 'e': 
                        		SetDlgItemText (hWndDlg,wParam,"E");  
                        		break;
                        	case 'w':
                        		SetDlgItemText (hWndDlg,wParam,"W"); 
                        		break;
                        	case 'E':
                        	case 'W':
                        		break;
                        	default:
                				Sound (BAD_SOUND);
		                    	SetFocus (GetDlgItem(hWndDlg,wParam)); 
		                    	SendDlgItemMessage(hWndDlg, wParam,EM_SETSEL, 0, MAKELONG(0, -1));
                        		return TRUE;
                        }
           	         	if (HIWORD(lParam) == EN_CHANGE)
                    		SetFocus (GetDlgItem(hWndDlg,IDC_COMPUTE)); 
                    break;
                 }
            break;
            
            case IDC_BDEGR:
            case IDC_BDEGT:  
            case IDC_BDEGC:   
            	Maxval = 90; 
            	goto S20;
            case IDC_BMINT:  
            case IDC_BMINR: 
            case IDC_BMINC:
            	Maxval = 60; 
      S20:
                 switch (HIWORD(lParam))
                 {  
                 	case EN_KILLFOCUS:    
             		case EN_CHANGE:  
                    	n = GetDlgItemText (hWndDlg,wParam,str,8);
                    	if (HIWORD(lParam) == EN_KILLFOCUS || n == 2)
                    	{    
                    		n = strtol(str, &pEnd,10);
                    		if (*pEnd || n < 0 || n > Maxval)
                    		{
	            				Sound (BAD_SOUND);
		                    	SetFocus (GetDlgItem(hWndDlg,wParam)); 
		                    	SendDlgItemMessage(hWndDlg, wParam,EM_SETSEL, 0, MAKELONG(0, -1));
	                    		return TRUE;
	                    	}
	                    	switch (wParam)
	                    	{
					            case IDC_BDEGR:
		           	         		Next=IDC_BMINR;	 	 
		           	         		break;
					            case IDC_BDEGT: 
		                        	Next = IDC_BMINT;
					                break;
					            case IDC_BDEGC: 
		                        	Next = IDC_BMINC;
					                break;
					            case IDC_BMINT:  
	           	         			Next=IDC_BSECT;	 	 
	           	         			break;
					            case IDC_BMINR:   
					            	Next = IDC_BSECR;
					            	break;
					            case IDC_BMINC:   
					            	Next = IDC_BSECC;
					            	break;
					        } 
               	         	if (HIWORD(lParam) == EN_CHANGE)
               	         	{
	                    		SetFocus (GetDlgItem(hWndDlg,Next));
	                    		SendDlgItemMessage(hWndDlg, Next,EM_SETSEL, 0, MAKELONG(0, -1)); 
	                    	}
                    	}
                    break;
                 }
            break;
            
            case IDC_BSECT: 
            case IDC_BSECR:    
            case IDC_BSECC:
                 switch (HIWORD(lParam))
                 {      
             		case EN_CHANGE: 
             		case EN_KILLFOCUS: 
                    	n = GetDlgItemText (hWndDlg,wParam,str,8);
                    	if (wParam == IDC_BSECT)
                    		Next = IDC_POSTDIRT;
                    	else if (wParam == IDC_BSECR)
                    		Next = IDC_POSTDIRR; 
                    	else
                    		Next = IDC_POSTDIRC;  
                        if (n)
                        switch (str[n-1])
                        {
                        	case 'e': 
                        	case 'E':
                        		SetDlgItemText (hWndDlg,Next,"E"); 
                        		str[n-1] = 0;  
                        		SetDlgItemText (hWndDlg,wParam,str);
                        		n = 5;
                        		break;
                        	case 'W':
                        	case 'w':
                        		SetDlgItemText (hWndDlg,Next,"W");
                        		str[n-1] = 0;
                        		SetDlgItemText (hWndDlg,wParam,str);
                        		n = 5; 
                        		break;
                        	default: 
                        		break;
                        }
                    	if (HIWORD(lParam) == EN_KILLFOCUS || n == 5)
                    	{
                			double n = strtod(str, &pEnd);
	                		if (*pEnd || n < 0 || n > 60)
	                		{
	            				Sound (BAD_SOUND);
		                    	SetFocus (GetDlgItem(hWndDlg,wParam)); 
		                    	SendDlgItemMessage(hWndDlg, wParam,EM_SETSEL, 0, MAKELONG(0, -1));
	                    		return TRUE;
	                    	} 
	           	         	if (HIWORD(lParam) == EN_CHANGE)
		                		SetFocus (GetDlgItem(hWndDlg,Next)); 
	                	}
                    break;
                 }
            break;
            
         	case IDC_COMPUTE:
		 	case IDOK:      
				if (SendDlgItemMessage (hWndDlg,IDC_USETANBEAR,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)) 
				{
					PreDirCntl = IDC_PREDIRT;
					BDegCntl = IDC_BDEGT;
					BMinCntl = IDC_BMINT;
					BSecCntl = IDC_BSECT;
					PostDirCntl = IDC_POSTDIRT;   
					_fstrcpy (BearOpt,"TANBEAR");
				}
				else if (SendDlgItemMessage (hWndDlg,IDC_USECHORDBEAR,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)) 
				{
					PreDirCntl = IDC_PREDIRC;
					BDegCntl = IDC_BDEGC;
					BMinCntl = IDC_BMINC;
					BSecCntl = IDC_BSECC;
					PostDirCntl = IDC_POSTDIRC;   
					_fstrcpy (BearOpt,"CHORDBEAR");
				}
				else
				{
					PreDirCntl = IDC_PREDIRR;
					BDegCntl = IDC_BDEGR;
					BMinCntl = IDC_BMINR;
					BSecCntl = IDC_BSECR;
					PostDirCntl = IDC_POSTDIRR;
					_fstrcpy (BearOpt,"RADBEAR");
				}
                GetDlgItemText (hWndDlg,PreDirCntl,PreDir,2);
                DEG = GetDlgItemInt (hWndDlg,BDegCntl,&Err,FALSE);
			    if (DEG < 0 || DEG>90)
			    {
			    	MessageBox(hWndDlg,"Invalid degrees",0,MB_ICONEXCLAMATION); 
			    	break;
			    }  
                MIN = GetDlgItemInt (hWndDlg,BMinCntl,&Err,FALSE); 
			    if (MIN < 0 || MIN>60)
			    {
			    	MessageBox(hWndDlg,"Invalid minutes",0,MB_ICONEXCLAMATION); 
			    	break;
			    }  
                GetDlgItemText (hWndDlg,BSecCntl,str,14);
                SEC = atof (str);
			    if (SEC < 0 || SEC>60)
			    {
			    	MessageBox(hWndDlg,"Invalid seconds",0,MB_ICONEXCLAMATION); 
			    	break;
			    }  
                GetDlgItemText (hWndDlg,PostDirCntl,PostDir,2);     
                
				if (SendDlgItemMessage (hWndDlg,IDC_USEARCDIST,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)) 
				{
                	DistUnCntl = IDC_DISTUNITSA;
                	DistCntl = IDC_ARCDISTANCE;  
                	_fstrcpy (DistOptC,"ARCDIST");
                }
				else if (SendDlgItemMessage (hWndDlg,IDC_USECHORDDIST,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)) 
				{
                	DistUnCntl = IDC_DISTUNITSCRD;
                	DistCntl = IDC_CHORDDISTANCE;
                	_fstrcpy (DistOptC,"CHORDDIST");
                }
				else 
				{
                	DistUnCntl = IDC_DISTUNITST;
                	DistCntl = IDC_TANDISTANCE;
                	_fstrcpy (DistOptC,"TANDIST");
                }
			    GetDlgItemText (hWndDlg,DistCntl,str,sizeof(str));
			    Dist = FltAP (str,&IRC);
			    if (IRC || Dist < 0)
			    {
			    	MessageBox(hWndDlg,"Invalid distance",0,MB_ICONEXCLAMATION); 
			    	break;
			    }  
   			    GetDlgItemText (hWndDlg,DistUnCntl,DistUnits,sizeof(DistUnits));  
   			    
			    DistOpt=SendDlgItemMessage(hWndDlg,IDC_DISTUNITSR,CB_GETCURSEL,NULL,NULL); 
			    GetDlgItemText (hWndDlg,IDC_RADIUS,str,sizeof(str));
			    RadLen = FltAP (str,&IRC);
			    if (IRC || RadLen <= 0)
			    {
			    	MessageBox(hWndDlg,"Invalid radius length",0,MB_ICONEXCLAMATION); 
			    	break;
			    }  
   			    GetDlgItemText (hWndDlg,IDC_DISTUNITSR,RadUnits,sizeof(RadUnits));  
   			    
   			    
   			    GetDlgItemText (hWndDlg,IDC_DIRCURVE,CrvDir,sizeof(CrvDir));  
 			    sprintf (CurLegData.Parameters,"DIR=%s, RADIUS=%.3f %s, %s=%s %2.2iD %2.2iM %.1fS %s, %s=%.3f %s",
 			    		 CrvDir,RadLen,RadUnits,BearOpt,PreDir,DEG,MIN,SEC,PostDir,DistOptC,Dist,DistUnits);
				if (wParam == IDC_COMPUTE)
					PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
				else
	       			EndDialog(hWndDlg, TRUE);
		 	    break;   
		 	     
            case IDCANCEL:
                 EndDialog(hWndDlg, FALSE);
                 break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}  

BOOL FAR PASCAL TRAVTYPE2CRVMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 	int		st, choice, n;
	char	str[256], DistUnits[16],SecC[16],RadiusUnits[16];
	static	char	PreDir[4]="N", PostDir[4]="E"; 
	static	char	CurDist[256]="",CrvDir[8]="Right",CurRadius[64];
	static	short	CurDistUnits=0, CurRadiusUnits;  
	static	double	AZ;      
	static	UINT	SetVal;
	static	double	SEC=0; 
	static	short	DEG=0,MIN=0;
	BOOL	Err, NORTH,EAST;
	LPSTR	lpTAB, pParm, pEnd;   
	short	DistOpt,IRC; 
	double	Dist,ROT,Radius;

 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
         SendDlgItemMessage (hWndDlg,IDC_DIRCURVE,CB_ADDSTRING,0,(LPARAM)"Right");
         SendDlgItemMessage (hWndDlg,IDC_DIRCURVE,CB_ADDSTRING,0,(LPARAM)"Left");
         SendDlgItemMessage (hWndDlg,IDC_DIRCURVE,CB_SELECTSTRING,-1,(LPARAM)CrvDir);
         SendDlgItemMessage (hWndDlg,IDC_PREDIR,CB_ADDSTRING,0,(LPARAM)"N");
         SendDlgItemMessage (hWndDlg,IDC_PREDIR,CB_ADDSTRING,0,(LPARAM)"S");
         SendDlgItemMessage (hWndDlg,IDC_POSTDIR,CB_ADDSTRING,0,(LPARAM)"E");
         SendDlgItemMessage (hWndDlg,IDC_POSTDIR,CB_ADDSTRING,0,(LPARAM)"W");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITSR,CB_ADDSTRING,0,(LPARAM)"Feet");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITSR,CB_ADDSTRING,0,(LPARAM)"Meters");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITSR,CB_ADDSTRING,0,(LPARAM)"Yards");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITSR,CB_ADDSTRING,0,(LPARAM)"Miles");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITSR,CB_ADDSTRING,0,(LPARAM)"Kilometers");
		 SendDlgItemMessage(hWndDlg,IDC_DISTUNITSR,CB_SETCURSEL,CurDistUnits,NULL); 
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITS,CB_ADDSTRING,0,(LPARAM)"Feet");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITS,CB_ADDSTRING,0,(LPARAM)"Meters");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITS,CB_ADDSTRING,0,(LPARAM)"Yards");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITS,CB_ADDSTRING,0,(LPARAM)"Miles");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITS,CB_ADDSTRING,0,(LPARAM)"Kilometers");
		 SendDlgItemMessage(hWndDlg,IDC_DISTUNITS,CB_SETCURSEL,CurDistUnits,NULL); 

    case GSSI_REINITDIALOG:
    	 _fstrcpy (str,CurLegData.Parameters);
    	 pParm = _fstrstr (str,"BEAR=");
    	 if (pParm)
    	 {
    	 	pParm+=5;
    	 	*PreDir = *pParm++;
    	 	PreDir[1] = 0;
			SendDlgItemMessage (hWndDlg,IDC_PREDIR,CB_SELECTSTRING,-1,(LPARAM)PreDir);  
	    	pEnd = _fstrchr(pParm,'D');
	    	*pEnd++ = 0;
	    	SetDlgItemText (hWndDlg,IDC_BDEG,pParm); 
	    	pParm = pEnd;
	    	pEnd = _fstrchr(pParm,'M');
	    	*pEnd++ = 0;
	    	SetDlgItemText (hWndDlg,IDC_BMIN,pParm);
	    	pParm = pEnd;
	    	pEnd = _fstrchr(pParm,'S');
	    	*pEnd++ = 0; 
	    	pEnd++;
	    	SetDlgItemText (hWndDlg,IDC_BSEC,pParm);
	    	pParm = pEnd++;
	    	*pEnd = 0;
			SendDlgItemMessage (hWndDlg,IDC_POSTDIR,CB_SELECTSTRING,-1,(LPARAM)pParm);  
	     }
    	 _fstrcpy (str,CurLegData.Parameters);
    	 pParm = _fstrstr (str,"RADIUS=");
    	 if (pParm)
    	 {
    	 	pParm+=7;
    	 	pParm = GetDistAndUnits (pParm,&Radius,&CurRadiusUnits,TRUE);   
    	 	sprintf (str,"%.3f",Radius);
	    	SetDlgItemText (hWndDlg,IDC_RADIUS,str);
		 	SendDlgItemMessage(hWndDlg,IDC_DISTUNITSR,CB_SETCURSEL,CurRadiusUnits,NULL); 
	     }
    	 _fstrcpy (str,CurLegData.Parameters);
    	 pParm = _fstrstr (str,"DIST=");
    	 if (pParm)
    	 {
    	 	pParm+=5;
    	 	pParm = GetDistAndUnits (pParm,&Dist,&CurDistUnits,TRUE);   
    	 	sprintf (str,"%.3f",Dist);
	    	SetDlgItemText (hWndDlg,IDC_DISTANCE,str);
		 	SendDlgItemMessage(hWndDlg,IDC_DISTUNITS,CB_SETCURSEL,CurDistUnits,NULL); 
	     }
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
         { 
            case IDC_PREDIR:  
                 switch (HIWORD(lParam))
                 {  
                 	case CBN_KILLFOCUS:
                        if (!GetDlgItemText (hWndDlg,wParam,str,2))
                        	SetDlgItemText (hWndDlg,wParam,"N");
                        break; 
                        
		            case CBN_SELCHANGE:  
                    	SetFocus (GetDlgItem(hWndDlg,IDC_BDEG)); 
                    	SendDlgItemMessage(hWndDlg, IDC_BDEG,EM_SETSEL, 0, MAKELONG(0, -1));
                    break;
                 }
            break;
            
            case IDC_POSTDIR:  
                 switch (HIWORD(lParam))
                 {      
		             case CBN_SELCHANGE:  
                    	SetFocus (GetDlgItem(hWndDlg,IDC_RADIUS));
                    	SendDlgItemMessage(hWndDlg, IDC_RADIUS,EM_SETSEL, 0, MAKELONG(0, -1));
                    break;
                 }
            break;
            
            case IDC_BDEG:  
                 switch (HIWORD(lParam))
                 {      
             		case EN_CHANGE:  
                    	if (GetDlgItemText (hWndDlg,wParam,str,8)==2) 
                    	{
                    		SetFocus (GetDlgItem(hWndDlg,IDC_BMIN));
                    		SendDlgItemMessage(hWndDlg, IDC_BMIN,EM_SETSEL, 0, MAKELONG(0, -1));
                    	}
                    break;
                 }
            break;
            
            case IDC_BMIN:  
                 switch (HIWORD(lParam))
                 {      
             		case EN_CHANGE:  
                    	if (GetDlgItemText (hWndDlg,wParam,str,8)==2)
                    	{
                    		SetFocus (GetDlgItem(hWndDlg,IDC_BSEC));
                    		SendDlgItemMessage(hWndDlg, IDC_BSEC,EM_SETSEL, 0, MAKELONG(0, -1));
                    	}
                    break;
                 }
            break;
            
            case IDC_BSEC:  
                 switch (HIWORD(lParam))
                 {      
             		case EN_CHANGE:  
                    	if (GetDlgItemText (hWndDlg,wParam,str,8)>=2)
                    		SetFocus (GetDlgItem(hWndDlg,IDC_POSTDIR));
                    break;
                 }
            break;
            
		 	case IDOK: 
                GetDlgItemText (hWndDlg,IDC_PREDIR,PreDir,2);
                DEG = GetDlgItemInt (hWndDlg,IDC_BDEG,&Err,FALSE);
			    if (DEG < 0 || DEG>90)
			    {
			    	MessageBox(hWndDlg,"Invalid degrees",0,MB_ICONEXCLAMATION); 
			    	break;
			    }  
                MIN = GetDlgItemInt (hWndDlg,IDC_BMIN,&Err,FALSE); 
			    if (MIN < 0 || MIN>60)
			    {
			    	MessageBox(hWndDlg,"Invalid minutes",0,MB_ICONEXCLAMATION); 
			    	break;
			    }  
                GetDlgItemText (hWndDlg,IDC_BSEC,str,14);
                SEC = atof (str);
			    if (SEC < 0 || SEC>60)
			    {
			    	MessageBox(hWndDlg,"Invalid seconds",0,MB_ICONEXCLAMATION); 
			    	break;
			    }  
                GetDlgItemText (hWndDlg,IDC_POSTDIR,PostDir,2); 
                
			    DistOpt=SendDlgItemMessage(hWndDlg,IDC_DISTUNITSR,CB_GETCURSEL,NULL,NULL); 
			    GetDlgItemText (hWndDlg,IDC_RADIUS,CurRadius,sizeof(CurRadius));
			    Radius = FltAP (CurRadius,&IRC);
			    if (IRC || Radius < 0)
			    {
			    	MessageBox(hWndDlg,"Invalid radius",0,MB_ICONEXCLAMATION); 
			    	break;
			    }  
   			    GetDlgItemText (hWndDlg,IDC_DISTUNITS,RadiusUnits,sizeof(RadiusUnits));  
   			    
			    DistOpt=SendDlgItemMessage(hWndDlg,IDC_DISTUNITS,CB_GETCURSEL,NULL,NULL); 
			    GetDlgItemText (hWndDlg,IDC_DISTANCE,CurDist,sizeof(CurDist));
			    Dist = FltAP (CurDist,&IRC);
			    if (IRC || Dist < 0)
			    {
			    	MessageBox(hWndDlg,"Invalid distance",0,MB_ICONEXCLAMATION); 
			    	break;
			    }  
   			    GetDlgItemText (hWndDlg,IDC_DISTUNITS,DistUnits,sizeof(DistUnits));  
   			    
   			    
   			    
   			    GetDlgItemText (hWndDlg,IDC_DIRCURVE,CrvDir,sizeof(CrvDir));  
   			    sprintf (CurLegData.Parameters,"DIR=%s, RAD_BEAR=%s %2.2iD %2.2iM %.1fS %s, RADIUS=%.3f %s, DIST=%.3f %s",CrvDir,PreDir,DEG,MIN,SEC,PostDir,Radius,RadiusUnits,Dist,DistUnits);
	       		EndDialog(hWndDlg, TRUE);
		 	    break;   
		 	     
            case IDCANCEL:
                 EndDialog(hWndDlg, FALSE);
                 break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}  

BOOL FAR PASCAL TRAVTYPE3CRVMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 	int		st, choice, n;
	char	str[256], DistUnits[16],SecC[16],RadiusUnits[16],TanLenUnits[16];
	static	char	CurDist[256]="",CrvDir[8]="Right",CurRadius[64],CurTanLen[64];
	static	short	CurDistUnits=0, CurRadiusUnits,CurTanLenUnits;  
	static	double	AZ;      
	static	UINT	SetVal;
	static	double	SEC=0; 
	BOOL	Err;
	LPSTR	lpTAB, pParm, pEnd;   
	short	DistOpt,IRC; 
	double	Dist,ROT,Radius,TanLength;

 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
         SendDlgItemMessage (hWndDlg,IDC_DIRCURVE,CB_ADDSTRING,0,(LPARAM)"Right");
         SendDlgItemMessage (hWndDlg,IDC_DIRCURVE,CB_ADDSTRING,0,(LPARAM)"Left");
         SendDlgItemMessage (hWndDlg,IDC_DIRCURVE,CB_SELECTSTRING,-1,(LPARAM)CrvDir);
         SendDlgItemMessage (hWndDlg,IDC_PREDIR,CB_ADDSTRING,0,(LPARAM)"N");
         SendDlgItemMessage (hWndDlg,IDC_PREDIR,CB_ADDSTRING,0,(LPARAM)"S");
         SendDlgItemMessage (hWndDlg,IDC_POSTDIR,CB_ADDSTRING,0,(LPARAM)"E");
         SendDlgItemMessage (hWndDlg,IDC_POSTDIR,CB_ADDSTRING,0,(LPARAM)"W");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITSR,CB_ADDSTRING,0,(LPARAM)"Feet");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITSR,CB_ADDSTRING,0,(LPARAM)"Meters");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITSR,CB_ADDSTRING,0,(LPARAM)"Yards");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITSR,CB_ADDSTRING,0,(LPARAM)"Miles");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITSR,CB_ADDSTRING,0,(LPARAM)"Kilometers");
		 SendDlgItemMessage(hWndDlg,IDC_DISTUNITSR,CB_SETCURSEL,CurDistUnits,NULL); 
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITST,CB_ADDSTRING,0,(LPARAM)"Feet");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITST,CB_ADDSTRING,0,(LPARAM)"Meters");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITST,CB_ADDSTRING,0,(LPARAM)"Yards");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITST,CB_ADDSTRING,0,(LPARAM)"Miles");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITST,CB_ADDSTRING,0,(LPARAM)"Kilometers");
		 SendDlgItemMessage(hWndDlg,IDC_DISTUNITST,CB_SETCURSEL,CurDistUnits,NULL); 
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITS,CB_ADDSTRING,0,(LPARAM)"Feet");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITS,CB_ADDSTRING,0,(LPARAM)"Meters");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITS,CB_ADDSTRING,0,(LPARAM)"Yards");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITS,CB_ADDSTRING,0,(LPARAM)"Miles");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITS,CB_ADDSTRING,0,(LPARAM)"Kilometers");
		 SendDlgItemMessage(hWndDlg,IDC_DISTUNITS,CB_SETCURSEL,CurDistUnits,NULL); 

    case GSSI_REINITDIALOG:
    	 _fstrcpy (str,CurLegData.Parameters);
    	 pParm = _fstrstr (str,"TANLENGTH=");
    	 if (pParm)
    	 {
    	 	pParm+=10;
    	 	pParm = GetDistAndUnits (pParm,&TanLength,&CurTanLenUnits,TRUE);   
    	 	sprintf (str,"%.3f",TanLength);
	    	SetDlgItemText (hWndDlg,IDC_TANLENGTH,str);
		 	SendDlgItemMessage(hWndDlg,IDC_DISTUNITST,CB_SETCURSEL,CurTanLenUnits,NULL); 
	     }
    	 _fstrcpy (str,CurLegData.Parameters);
    	 pParm = _fstrstr (str,"RADIUS=");
    	 if (pParm)
    	 {
    	 	pParm+=7;
    	 	pParm = GetDistAndUnits (pParm,&Radius,&CurRadiusUnits,TRUE);   
    	 	sprintf (str,"%.3f",Radius);
	    	SetDlgItemText (hWndDlg,IDC_RADIUS,str);
		 	SendDlgItemMessage(hWndDlg,IDC_DISTUNITSR,CB_SETCURSEL,CurRadiusUnits,NULL); 
	     }
    	 _fstrcpy (str,CurLegData.Parameters);
    	 pParm = _fstrstr (str,"DIST=");
    	 if (pParm)
    	 {
    	 	pParm+=5;
    	 	pParm = GetDistAndUnits (pParm,&Dist,&CurDistUnits,TRUE);   
    	 	sprintf (str,"%.3f",Dist);
	    	SetDlgItemText (hWndDlg,IDC_DISTANCE,str);
		 	SendDlgItemMessage(hWndDlg,IDC_DISTUNITS,CB_SETCURSEL,CurDistUnits,NULL); 
	     }
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
         { 
		 	case IDOK: 
                
			    DistOpt=SendDlgItemMessage(hWndDlg,IDC_DISTUNITST,CB_GETCURSEL,NULL,NULL); 
			    GetDlgItemText (hWndDlg,IDC_TANLENGTH,CurTanLen,sizeof(CurTanLen));
			    TanLength = FltAP (CurTanLen,&IRC);
			    if (IRC || TanLength < 0)
			    {
			    	MessageBox(hWndDlg,"Invalid tangent length",0,MB_ICONEXCLAMATION); 
			    	break;
			    }  
   			    GetDlgItemText (hWndDlg,IDC_DISTUNITST,TanLenUnits,sizeof(TanLenUnits));  
   			    
			    DistOpt=SendDlgItemMessage(hWndDlg,IDC_DISTUNITSR,CB_GETCURSEL,NULL,NULL); 
			    GetDlgItemText (hWndDlg,IDC_RADIUS,CurRadius,sizeof(CurRadius));
			    Radius = FltAP (CurRadius,&IRC);
			    if (IRC || Radius < 0)
			    {
			    	MessageBox(hWndDlg,"Invalid radius",0,MB_ICONEXCLAMATION); 
			    	break;
			    }  
   			    GetDlgItemText (hWndDlg,IDC_DISTUNITS,RadiusUnits,sizeof(RadiusUnits));  
   			    
			    DistOpt=SendDlgItemMessage(hWndDlg,IDC_DISTUNITS,CB_GETCURSEL,NULL,NULL); 
			    GetDlgItemText (hWndDlg,IDC_DISTANCE,CurDist,sizeof(CurDist));
			    Dist = FltAP (CurDist,&IRC);
			    if (IRC || Dist < 0)
			    {
			    	MessageBox(hWndDlg,"Invalid distance",0,MB_ICONEXCLAMATION); 
			    	break;
			    }  
   			    GetDlgItemText (hWndDlg,IDC_DISTUNITS,DistUnits,sizeof(DistUnits));  
   			    
   			    
   			    
   			    GetDlgItemText (hWndDlg,IDC_DIRCURVE,CrvDir,sizeof(CrvDir));  
   			    sprintf (CurLegData.Parameters,"DIR=%s, TANLENGTH=%.3f %s, RADIUS=%.3f %s, DIST=%.3f %s",CrvDir,TanLength,TanLenUnits,Radius,RadiusUnits,Dist,DistUnits);
	       		EndDialog(hWndDlg, TRUE);
		 	    break;   
		 	     
            case IDCANCEL:
                 EndDialog(hWndDlg, FALSE);
                 break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}  

double atobasedist (LPSTR Dist,LPSHORT perr)
{
    double	DistVal,OutDist;
    short	Units;
    
    *perr = 0;
	GetDistAndUnits (Dist,&DistVal,&Units,TRUE);
	OutDist = ConvertInDist (DistVal,Units+1);
	return OutDist;
}


LPSTR GetDistAndUnits (LPSTR pParm,LPDOUBLE pDist,LPSHORT pCurDistUnits,BOOL UseDefault)
{   
	short i, lun; 
	char	savec;
	LPSTR	punits, pSave; 
	HANDLE	hTemp=0;
	long	l;
	LPSTR	pTemp; 
	LPSTR	pComma=0;
	
	l = _fstrlen (pParm);
	if (l)
	{
		hTemp = GSSiGlobAlloc (1079,GMEM_MOVEABLE,l+8);
		pTemp = GlobalLock (hTemp);
		_fstrcpy (pTemp,pParm);
		_fstrupr (pTemp);
		pParm = pTemp; 
		punits = FirstAlpha (pParm);
		if (punits)
		{ 
			if ((pComma = _fstrchr (punits,',')))
				*pComma = 0;
			lun = _fstrlen(punits); 
			if (lun)
			for (i=0;i<6;i++)
			{
				if (!_fstrnicmp (punits,DistUnitOpts[i],lun) || (!i && !_fstricmp (punits,"FOOT")))
				{   
					if (i == 5)
						i = PRJ_UNITS[1]-1;
					*pCurDistUnits=i;  
					savec = *punits; 
					pSave = punits;
					*punits = 0;
					punits += lun;
					*pDist = atof (pParm); 
					*pSave = savec;  
					if (pComma)
						*pComma = ',';
					GSSiGlobUlFree (&hTemp);
					return punits;
				}
			}
		}
	} 
	if (UseDefault)
		*pCurDistUnits = PRJ_UNITS[1]-1;
	else
		*pCurDistUnits = -1;
	*pDist = atof (pParm);
	if (pComma)
		*pComma = ',';
	GSSiGlobUlFree (&hTemp);
	return 0;
}    

double BearToAZ (char PreDir,LPSTR pDeg,LPSTR pMin,LPSTR pSec,char PostDir)
{   
	double AZ;
	double	DEG,MIN,SEC;
	BOOL	NORTH,EAST;
	
    switch (PreDir)
    {   
    	default:
    	case 'n': 
    	case 'N':
    		NORTH = TRUE;
    		break;
    	case 's':
    	case 'S':
    		NORTH = FALSE;
    		break;
    }
    switch (PostDir)
    {   
    	default:
    	case 'e':
    	case 'E':
    		EAST = TRUE;
    		break;
    	case 'w':
    	case 'W':
    		EAST = FALSE;
    		break;
    }
    DEG = atof (pDeg);
    MIN = atof (pMin); 
    SEC = atof (pSec);
	AZ = DEG + MIN / 60E0 + SEC / 3600E0;
	if (NORTH && EAST)
		AZ+=0;
	else if (NORTH && !EAST)
	  AZ = 360E0 - AZ; 
	else if (!NORTH && EAST)
	  AZ = 180E0 - AZ; 
	else if (!NORTH && !EAST) 
	  AZ = 180E0 + AZ;
	AZ *= RADDEG; 
	AZ = LTWOPI(HALFPI-AZ);
	return AZ;
}

BOOL DecodeBANDDParm (LPSTR Parm,LPDOUBLE pDist,LPDOUBLE pAZ,LPSHORT pDir,short Type)
{ 
	char	str[256], PreDir,PostDir,DegC[32],MinC[32],SecC[32]; 
	LPSTR	pParm, pEnd; 
	double	Dist,AZ;    
	short	inc;
	
	_fstrcpy (str,Parm);
	pParm = _fstrstr (str,"BEAR=");
	if (pParm)
	{
		pParm+=5;
		PreDir = *pParm++;
		pEnd = _fstrchr(pParm,'D');
		*pEnd++ = 0;
		_fstrcpy (DegC,pParm); 
		pParm = pEnd;
		pEnd = _fstrchr(pParm,'M');
		*pEnd++ = 0;
		_fstrcpy (MinC,pParm);
		pParm = pEnd;
		pEnd = _fstrchr(pParm,'S');
		*pEnd++ = 0; 
		pEnd++;
		_fstrcpy (SecC,pParm);  
		pParm = pEnd;
		pEnd++;
		*pEnd=0;
		PostDir = *pParm;
		*pAZ = BearToAZ (PreDir,DegC,MinC,SecC,PostDir);
	}
	else
		return FALSE;
	_fstrcpy (str,Parm);
	switch (Type)
	{
		default:
			pParm = _fstrstr (str,"DIST="); 
			inc = 5; 
			break;
		case 22:
			pParm = _fstrstr (str,"RADIUS=");
			inc = 7;  
			break;
	}
	if (pParm)
	{   
		short	CurDistUnits;
		
		pParm+=inc;
		pParm = GetDistAndUnits (pParm,&Dist,&CurDistUnits,TRUE);  
		*pDist = ConvertInDist (Dist,CurDistUnits+1); 
	}
	else
		return FALSE;
	_fstrcpy (str,Parm);
	pParm = _fstrstr (str,"DIR=");  
	if (pParm)
	{   
		short	CurDistUnits;
		
		pParm+=4; 
		if (!_fstrnicmp (pParm,"Right",5))
			*pDir = 1; 
		else
			*pDir = 2;
	}
	return TRUE;
}  

BOOL DecodeTYPE1CRVParm (LPSTR Parm,LPDOUBLE pRadius,LPDOUBLE pDist,LPDOUBLE pAZ,LPSHORT pDir,LPSHORT pDistOpt,LPSHORT pBearOpt)
{ 
	char	str[256], PreDir,PostDir,DegC[32],MinC[32],SecC[32]; 
	LPSTR	pParm, pEnd; 
	double	Dist,AZ;    
	short	inc;
	
	_fstrcpy (str,Parm);
	pParm = _fstrstr (str,"BEAR=");
	if (pParm)
	{
	 	if (!_fstrnicmp ((pParm-3),"TAN",3))
			*pBearOpt = 1;
	 	else if (!_fstrnicmp ((pParm-3),"RAD",3))
			*pBearOpt = 2;
		else
			*pBearOpt = 3;
		pParm+=5;
		PreDir = *pParm++;
		pEnd = _fstrchr(pParm,'D');
		*pEnd++ = 0;
		_fstrcpy (DegC,pParm); 
		pParm = pEnd;
		pEnd = _fstrchr(pParm,'M');
		*pEnd++ = 0;
		_fstrcpy (MinC,pParm);
		pParm = pEnd;
		pEnd = _fstrchr(pParm,'S');
		*pEnd++ = 0; 
		pEnd++;
		_fstrcpy (SecC,pParm);  
		pParm = pEnd;
		pEnd++;
		*pEnd=0;
		PostDir = *pParm;
		*pAZ = BearToAZ (PreDir,DegC,MinC,SecC,PostDir);
	}
	else
		return FALSE;
	_fstrcpy (str,Parm);
	pParm = _fstrstr (str,"DIST="); 
	inc = 5; 
	if (pParm)
	{   
		short	CurDistUnits;
		
	 	if (!_fstrnicmp ((pParm-3),"ARC",3))
			*pDistOpt = 1;
    	else if (!_fstrnicmp ((pParm-3),"TAN",3))
			*pDistOpt = 2;
		else 
			*pDistOpt = 3;
		pParm+=inc;
		pParm = GetDistAndUnits (pParm,&Dist,&CurDistUnits,TRUE);  
		*pDist = ConvertInDist (Dist,CurDistUnits+1); 
	}
	else
		return FALSE;
	pParm = _fstrstr (str,"RADIUS=");
	inc = 7;  
	if (pParm)
	{   
		short	CurDistUnits;
		
		pParm+=inc;
		pParm = GetDistAndUnits (pParm,&Dist,&CurDistUnits,TRUE);  
		*pRadius = ConvertInDist (Dist,CurDistUnits+1); 
	}
	else
		return FALSE;
	_fstrcpy (str,Parm);
	pParm = _fstrstr (str,"DIR=");  
	if (pParm)
	{   
		short	CurDistUnits;
		
		pParm+=4; 
		if (!_fstrnicmp (pParm,"Right",5))
			*pDir = 1; 
		else
			*pDir = 2;
	}
	return TRUE;
} 

BOOL ComputeTYPE1CRV (double Radius,double Dist,double AZ,short Dir,short DistOpt,short BearOpt,DPOINT PC,
					  LPDPOINT pPOC, LPDPOINT pPT,LPDPOINT pRP,
					  LPDOUBLE pArcDist,LPDOUBLE pTanDist,LPDOUBLE pChordDist,
					  LPDOUBLE pTanAZ, LPDOUBLE pRPAZ, LPDOUBLE pCrdAZ,LPDOUBLE pAngle)
{   
	short	rc;      
	double	clen;
	
	if (!Radius)
	{
    	MessageBox(GetFocus(),"Invalid radius",0,MB_ICONEXCLAMATION); 
		return FALSE;
	}
	switch (DistOpt)
	{
		case 1:
			*pArcDist = Dist;
			*pAngle = *pArcDist/Radius;
			break; 
		case 2:  
			*pTanDist = Dist; 
			*pAngle = atan2 (*pTanDist,Radius) * 2;
			break;
		case 3: 
			*pChordDist = Dist;
			*pAngle = asin ((Dist/2)/Radius) * 2; 
			break;
	}
	switch (BearOpt)
	{   
		case 1:
			if (Dir == 1)
				*pRPAZ = LTWOPI(AZ-HALFPI);	
			else
				*pRPAZ = LTWOPI(AZ+HALFPI);
			break;	 
		
		case 2:
			*pRPAZ = AZ; 
			break;
				 
		case 3:
			if (Dir == 1) 
			{ 
				AZ = LTWOPI(AZ-HALFPI);
				*pRPAZ = LTWOPI(AZ+*pAngle/2);
			}	
			else
			{ 
				AZ = LTWOPI(AZ+HALFPI);
				*pRPAZ = LTWOPI(AZ-*pAngle/2);
			}	
			break;	 
	} 
	*pArcDist = *pAngle * Radius; 
	*pTanDist = tan (*pAngle/2) * Radius;
	*pChordDist = sin (*pAngle/2) * Radius * 2;
	*pRP = dnewpt (PC,*pRPAZ,Radius);  
	if (Dir == 1)
		*pTanAZ = LTWOPI(*pRPAZ+HALFPI);	
	else
		*pTanAZ = LTWOPI(*pRPAZ-HALFPI);
	if (Dir == 1)
		*pCrdAZ = LTWOPI(*pRPAZ - *pAngle/2 + PY - HALFPI);	
	else
		*pCrdAZ = LTWOPI(*pRPAZ + *pAngle/2 + PY + HALFPI);	
	if (Dir == 2)
		clen = -(*pArcDist);  
	else
		clen = *pArcDist;
	rc = PCURVE(&PC.x,&PC.y,&pPOC->x,&pPOC->y,&pPT->x,&pPT->y,&pRP->x,&pRP->y,&clen); 
	return TRUE;
} 

BOOL FAR PASCAL TRAVDANDBMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 	int		st, choice, n;
	char	str[256],DegC[8],MinC[8],SecC[16], PreDir[4], PostDir[4], DistUnits[16]; 
	static	char	CurDist[256]="";
	static	short	CurDistUnits=0;  
	static	double	AZ;      
	static	UINT	SetVal;
	double	SEC; 
	short	DEG,MIN;
	BOOL	Err, NORTH,EAST;
	LPSTR	lpTAB, pParm, pEnd;   
	short	DistOpt,IRC; 
	double	Dist,ROT;

 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
         SendDlgItemMessage (hWndDlg,IDC_PREDIR,CB_ADDSTRING,0,(LPARAM)"N");
         SendDlgItemMessage (hWndDlg,IDC_PREDIR,CB_ADDSTRING,0,(LPARAM)"S");
         SendDlgItemMessage (hWndDlg,IDC_POSTDIR,CB_ADDSTRING,0,(LPARAM)"E");
         SendDlgItemMessage (hWndDlg,IDC_POSTDIR,CB_ADDSTRING,0,(LPARAM)"W");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITS,CB_ADDSTRING,0,(LPARAM)"Feet");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITS,CB_ADDSTRING,0,(LPARAM)"Meters");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITS,CB_ADDSTRING,0,(LPARAM)"Yards");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITS,CB_ADDSTRING,0,(LPARAM)"Miles");
         SendDlgItemMessage (hWndDlg,IDC_DISTUNITS,CB_ADDSTRING,0,(LPARAM)"Kilometers");
		 SendDlgItemMessage(hWndDlg,IDC_DISTUNITS,CB_SETCURSEL,CurDistUnits,NULL); 
    case GSSI_REINITDIALOG:  
    	 _fstrcpy (str,CurLegData.Parameters);
    	 if (!*str)
    	 	_fstrcpy (str,LastDANDBLegData.Parameters);
    	 pParm = _fstrstr (str,"BEAR=");
    	 if (pParm)
    	 {
    	 	pParm+=5;
    	 	*PreDir = *pParm++;
    	 	PreDir[1] = 0;
			SendDlgItemMessage (hWndDlg,IDC_PREDIR,CB_SELECTSTRING,-1,(LPARAM)PreDir);  
	    	pEnd = _fstrchr(pParm,'D');
	    	*pEnd++ = 0;
	    	SetDlgItemText (hWndDlg,IDC_BDEG,pParm); 
	    	pParm = pEnd;
	    	pEnd = _fstrchr(pParm,'M');
	    	*pEnd++ = 0;
	    	SetDlgItemText (hWndDlg,IDC_BMIN,pParm);
	    	pParm = pEnd;
	    	pEnd = _fstrchr(pParm,'S');
	    	*pEnd++ = 0; 
	    	pEnd++;
	    	SetDlgItemText (hWndDlg,IDC_BSEC,pParm);  
	    	pParm = pEnd;
	    	pEnd++;
	    	*pEnd=0;
			SendDlgItemMessage (hWndDlg,IDC_POSTDIR,CB_SELECTSTRING,-1,(LPARAM)pParm);  
	     }
    	 _fstrcpy (str,CurLegData.Parameters);
    	 if (!*str)
    	 	_fstrcpy (str,LastDANDBLegData.Parameters);
    	 pParm = _fstrstr (str,"DIST=");
    	 if (pParm)
    	 {
    	 	pParm+=5;
    	 	pParm = GetDistAndUnits (pParm,&Dist,&CurDistUnits,TRUE);   
    	 	sprintf (str,"%.4f",Dist);
	    	SetDlgItemText (hWndDlg,IDC_DISTANCE,str);
		 	SendDlgItemMessage(hWndDlg,IDC_DISTUNITS,CB_SETCURSEL,CurDistUnits,NULL); 
	     }
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           { 
            case IDC_PREDIR:  
                 switch (HIWORD(lParam))
                 {      
                 	case CBN_KILLFOCUS:
                        if (!GetDlgItemText (hWndDlg,wParam,str,2))
							SendDlgItemMessage (hWndDlg,wParam,CB_SELECTSTRING,-1,(LPARAM)"N");  
                        break; 
                        
		            case CBN_SELCHANGE:  
                    	SetFocus (GetDlgItem(hWndDlg,IDC_BDEG)); 
                    	SendDlgItemMessage(hWndDlg, IDC_BDEG,EM_SETSEL, 0, MAKELONG(0, -1));
                    break;
                 }
            break;
            
            case IDC_POSTDIR:  
                 switch (HIWORD(lParam))
                 {      
                 	case CBN_KILLFOCUS:
                        if (!GetDlgItemText (hWndDlg,wParam,str,2))
							SendDlgItemMessage (hWndDlg,wParam,CB_SELECTSTRING,-1,(LPARAM)"E");  
                        break; 
                        
		             case CBN_SELCHANGE:  
                    	SetFocus (GetDlgItem(hWndDlg,IDC_DISTANCE));
                    	SendDlgItemMessage(hWndDlg, IDC_DISTANCE,EM_SETSEL, 0, MAKELONG(0, -1));
                    break;
                 }
            break;
            
            case IDC_BDEG:  
                 switch (HIWORD(lParam))
                 {      
             		case EN_CHANGE:  
                    	if (GetDlgItemText (hWndDlg,wParam,str,8)==2) 
                    	{
                    		SetFocus (GetDlgItem(hWndDlg,IDC_BMIN));
                    		SendDlgItemMessage(hWndDlg, IDC_BMIN,EM_SETSEL, 0, MAKELONG(0, -1));
                    	}
                    break;
                 }
            break;
            
            case IDC_BMIN:  
                 switch (HIWORD(lParam))
                 {      
             		case EN_CHANGE:  
                    	if (GetDlgItemText (hWndDlg,wParam,str,8)==2)
                    	{
                    		SetFocus (GetDlgItem(hWndDlg,IDC_BSEC));
                    		SendDlgItemMessage(hWndDlg, IDC_BSEC,EM_SETSEL, 0, MAKELONG(0, -1));
                    	}
                    break;
                 }
            break;
            
            case IDC_BSEC:  
                 switch (HIWORD(lParam))
                 {      
             		case EN_CHANGE:  
                    	if (GetDlgItemText (hWndDlg,wParam,str,8)>=2)
                    		SetFocus (GetDlgItem(hWndDlg,IDC_POSTDIR));
                    break;
                 }
            break;
            
		 	case IDOK: 
                GetDlgItemText (hWndDlg,IDC_PREDIR,PreDir,2);
                DEG = GetDlgItemInt (hWndDlg,IDC_BDEG,&Err,FALSE);
			    if (DEG < 0 || DEG>90)
			    {
			    	MessageBox(hWndDlg,"Invalid degrees",0,MB_ICONEXCLAMATION); 
			    	break;
			    }  
                MIN = GetDlgItemInt (hWndDlg,IDC_BMIN,&Err,FALSE); 
			    if (MIN < 0 || MIN>60)
			    {
			    	MessageBox(hWndDlg,"Invalid minutes",0,MB_ICONEXCLAMATION); 
			    	break;
			    }  
                GetDlgItemText (hWndDlg,IDC_BSEC,str,14);
                SEC = atof (str);
			    if (SEC < 0 || SEC>60)
			    {
			    	MessageBox(hWndDlg,"Invalid seconds",0,MB_ICONEXCLAMATION); 
			    	break;
			    }  
                GetDlgItemText (hWndDlg,IDC_POSTDIR,PostDir,2);
			    DistOpt=SendDlgItemMessage(hWndDlg,IDC_DISTUNITS,CB_GETCURSEL,NULL,NULL); 
			    GetDlgItemText (hWndDlg,IDC_DISTANCE,CurDist,sizeof(CurDist));
			    Dist = FltAP (CurDist,&IRC);
			    if (IRC || Dist < 0)
			    {
			    	MessageBox(hWndDlg,"Invalid distance",0,MB_ICONEXCLAMATION); 
			    	break;
			    }  
   			    GetDlgItemText (hWndDlg,IDC_DISTUNITS,DistUnits,sizeof(DistUnits));  
   			    sprintf (CurLegData.Parameters,"BEAR=%s %2.2iD %2.2iM %5.2fS %s, DIST=%12.4f %s",PreDir,DEG,MIN,SEC,PostDir,Dist,DistUnits);
	       		_fstrcpy (LastDANDBLegData.Parameters,CurLegData.Parameters);
	       		EndDialog(hWndDlg, TRUE);
		 	    break;   
		 	     
            case IDCANCEL:
                 EndDialog(hWndDlg, FALSE);
                 break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}  

BOOL FAR PASCAL SUBTRAVERSEMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{
 	int		st, choice, n, Legno;
	char	str[256];       
	LPSTR	pParm;
	int		TabStopsAddLeg[2]={9000,9100};
	

 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
       	 SendDlgItemMessage (hWndDlg,IDC_TRAVLISTSUB,LB_SETTABSTOPS,2,(LPARAM)TabStopsAddLeg);
    case GSSI_REINITDIALOG:  
		 ShowTrav (hWndDlg,IDC_TRAVLISTSUB,CurTravID);
    	 _fstrcpy (str,CurLegData.Parameters);
    	 pParm = _fstrstr (str,"StartLeg=");
    	 if (pParm)
    	 {
    	 	pParm+=9;
    	 	Legno = atoi(pParm);
			SendDlgItemMessage (hWndDlg,IDC_POSTDIR,CB_SELECTSTRING,-1,(LPARAM)pParm);  
	     }
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           { 
		 	case IDOK: 
			    choice=SendDlgItemMessage(hWndDlg,IDC_TRAVLISTSUB,LB_GETCURSEL,NULL,NULL); 
			    if (choice == LB_ERR)
			    	break;
   			    sprintf (CurLegData.Parameters,"StartLeg=%i",choice-POBItem);
	       		EndDialog(hWndDlg, TRUE);
		 	    break;   
		 	     
            case IDCANCEL:
                 EndDialog(hWndDlg, FALSE);
                 break;
           	case IDC_TRAVLISTSUB:
              	switch(HIWORD(lParam))
                {    
 		             case CBN_SELCHANGE:
						ClearHighlightList (TRUE);
 		                SetDlgItemText (hWndDlg,IDC_CALLNOSUB,""); 
		                choice=SendDlgItemMessage(hWndDlg,IDC_TRAVLISTSUB, LB_GETCURSEL,NULL,NULL);
		                if (choice > 0) 
		                {
 		             		TravShowSelectedLeg (hWndDlg,IDC_TRAVLISTSUB,IDC_CALLNOSUB,choice);
 		             	}
 		             	break; 
		             case LBN_DBLCLK: 
         			 	PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
					 break;
				}
			break;    
			
           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}  

BOOL EncodeTravLegData (LPTRAVLEGDATA pLegData,LPSTR str)
{
	switch (pLegData->Type)
	{
		case 0: //start point
			sprintf (str,"Starting at %s\t%i",pLegData->Parameters,pLegData->Type);
		break;               
		
		case 1: //POB
			sprintf (str,"*** Point of Beginning ***\t%i",pLegData->Type);
		break;               
		
		case 3: //Close
			sprintf (str,"*** Close Traverse ***\t%i",pLegData->Type);
		break;               
		
		case 11: //distance and bearing  	
		{ 
			sprintf (str,"Bearing and Distance (%s)\t%i",pLegData->Parameters,pLegData->Type);
		}
		break;
		
		case 21: //type 1 curve
			sprintf (str,"Type 1 curve (%s)\t%i",pLegData->Parameters,pLegData->Type);
		break;
		case 22: //type 2 curve
			sprintf (str,"Type 2 curve (%s)\t%i",pLegData->Parameters,pLegData->Type);
		break;
		case 23: //type 3 curve
			sprintf (str,"Type 3 curve (%s)\t%i",pLegData->Parameters,pLegData->Type);
		break;
		case 31: //sub-traverse
			sprintf (str,"Sub-Traverse (%s)\t%i",pLegData->Parameters,pLegData->Type);
		break;
	}
	return TRUE;
}

BOOL DecodeTravLegData (long TravID,short LegNum,LPTRAVLEGDATA pLegData,LPSTR str,LPDPOINT pPoints,LPDOUBLE pDist)
{   
	LPSTR	pParm,pEnd,pTAB; 
	double	Dist, AZ,Radius, ArcDist, TanDist, ChordDist, Angle, TanAZ, RPAZ, CrdAZ;   
	DPOINT	RP; 
	short	Dir,DistOpt,BearOpt;  
	LPSTR	pEQ;
	 
 	pTAB = _fstrchr(str,'\t'); 
 	if (pTAB)
 		*pTAB++=0;
 	else	
 		pTAB = _fstrchr (str,0);
    pLegData->Type=atoi(pTAB);
	pLegData->ID = TravID;
	pLegData->LegNum = LegNum;
	switch (pLegData->Type)
	{   
		case 0: 
			pParm = _fstrstr (str," at ");
			pParm+=4;
			_fstrcpy (pLegData->Parameters,pParm);   
			if (!pPoints)
				break;
			sscanf (pParm,"%Flf %Flf",&pPoints->x,&pPoints->y); 
			pPoints[2]=pPoints[0];
			break; 
		case 2://not used
		case 3://CLOSURE
		case 1://POB
			*pLegData->Parameters = 0;
			break;
		case 11: //bear and dist
			pParm = _fstrchr (str,'(');
			pParm++;
			pEnd = _fstrchr (pParm,')');
			*pEnd = 0;
			_fstrcpy (pLegData->Parameters,pParm); 
			if (!pPoints)
				break;
			DecodeBANDDParm (pParm,&Dist,&AZ,&Dir,pLegData->Type);
			CurrentAZ = AZ; 
			pPoints[0]=pPoints[2];
			pPoints[2]=dnewpt (pPoints[0],AZ+AZAdjust,Dist); 
			break;
		case 21: //type 1 curve
			pParm = _fstrchr (str,'(');
			pParm++;
			pEnd = _fstrchr (pParm,')');
			*pEnd = 0;
			_fstrcpy (pLegData->Parameters,pParm); 
			if (!pPoints)
				break;
			pPoints[0]=pPoints[2];
			DecodeTYPE1CRVParm (pParm,&Radius,&Dist,&AZ,&Dir,&DistOpt,&BearOpt);
			ComputeTYPE1CRV (Radius,Dist,AZ+AZAdjust,Dir,DistOpt,BearOpt,pPoints[0],
							 &pPoints[1],&pPoints[2],&RP,&ArcDist,&TanDist,&ChordDist,&TanAZ,&RPAZ,&CrdAZ,&Angle);
			break;
		case 31: //sub-traverse
			pParm = _fstrchr (str,'(');
			pParm++;
			pEnd = _fstrchr (pParm,')');
			*pEnd = 0;
			_fstrcpy (pLegData->Parameters,pParm);  
			if ((pEQ = _fstrchr (pParm,'=')))
				SubTravStartLeg = atol (++pEQ);
			else
				SubTravStartLeg = -1;
			break;
		default:
			pParm = _fstrchr (str,'(');
			pParm++;
			pEnd = _fstrchr (pParm,')');
			*pEnd = 0;
			_fstrcpy (pLegData->Parameters,pParm); 
			break;
	}
	if (pDist)
		*pDist = Dist;
	return TRUE;
}  

BOOL EditTravLegData (HWND hWnd, LPTRAVLEGDATA pLegData)
{   
	BOOL	st;
	CurLegData = *pLegData;
	switch (pLegData->Type)
	{
		case 0: //start point
		break;               
		
		case 11: //distance and bearing  	
		{
			FARPROC lpfnTRAVDANDBMsgProc;

			lpfnTRAVDANDBMsgProc = MakeProcInstance((FARPROC)TRAVDANDBMsgProc, hInst);
			st = DialogBox(hInst, (LPSTR)"TRAVDANDB", hWnd, lpfnTRAVDANDBMsgProc);
			FreeProcInstance(lpfnTRAVDANDBMsgProc);
		}
		break;
		
		case 21: //type 1 curve   
		{
			FARPROC lpfnTRAVTYPE1CRVMsgProc;

			lpfnTRAVTYPE1CRVMsgProc = MakeProcInstance((FARPROC)TRAVTYPE1CRVMsgProc, hInst);
			st = DialogBox(hInst, (LPSTR)"TRAVTYPE1CRV", hWnd, lpfnTRAVTYPE1CRVMsgProc);
			FreeProcInstance(lpfnTRAVTYPE1CRVMsgProc);
		}
		break;
		case 22: //type 2 curve
		{
			FARPROC lpfnTRAVTYPE2CRVMsgProc;

			lpfnTRAVTYPE2CRVMsgProc = MakeProcInstance((FARPROC)TRAVTYPE2CRVMsgProc, hInst);
			st = DialogBox(hInst, (LPSTR)"TRAVTYPE2CRV", hWnd, lpfnTRAVTYPE2CRVMsgProc);
			FreeProcInstance(lpfnTRAVTYPE2CRVMsgProc);
		}
		break;
		case 23: //type 3 curve
		{
			FARPROC lpfnTRAVTYPE3CRVMsgProc;

			lpfnTRAVTYPE3CRVMsgProc = MakeProcInstance((FARPROC)TRAVTYPE3CRVMsgProc, hInst);
			st = DialogBox(hInst, (LPSTR)"TRAVTYPE3CRV", hWnd, lpfnTRAVTYPE3CRVMsgProc);
			FreeProcInstance(lpfnTRAVTYPE3CRVMsgProc);
		}
		break;
		case 31: //sub-traverse   
		{
			FARPROC lpfnSUBTRAVERSEMsgProc;

			lpfnSUBTRAVERSEMsgProc = MakeProcInstance((FARPROC)SUBTRAVERSEMsgProc, hInst);
			st = DialogBox(hInst, (LPSTR)"SUBTRAVERSE", hWnd, lpfnSUBTRAVERSEMsgProc);
			FreeProcInstance(lpfnSUBTRAVERSEMsgProc);
		}
		break;
	} 
//	if (st)
//		SaveTrav (CurTravID,hWnd,IDC_TRAVLIST); 	
	return st;
}

BOOL SaveTrav (long TravID,HWND hWndDlg,UINT ControlID)
{	
	HANDLE	hDB;
    LPGWDHEADER lpGWDHead; 
	short	item=0,st;
	char	str[256];  
	long	Offset;
	LPTRAVLEGDATA	pLegData;     
	LPSTR	pTAB;   
	
	_fstrcpy (str,"[%TRAVDATADB]");
	hDB = OpenGWDatabase (str,BT_WRITE);
	if (!hDB)
	{   
    	MessageBox(GetFocus(),"Unable to open traverse database",str,MB_ICONEXCLAMATION|MB_OK);
		return FALSE; 
	}
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
	pLegData = (LPTRAVLEGDATA)&lpGWDHead->GWDData;

NextDelete:	 
	pLegData->ID = TravID;
	pLegData->LegNum = -1;     
	GWDFormKey(lpGWDHead,0,TRUE,0);
	st = BT_FIND (lpGWDHead->BTHandle[0],lpGWDHead->pKeys[0],BT_FIRST,BT_GE, (LPSTR)&Offset);
	if (!st)
	{   
		FillGWDData (lpGWDHead,Offset);  
	    if (pLegData->ID == TravID)
	    {
	    	BT_DELETE (lpGWDHead->BTHandle[0],(LPSTR)lpGWDHead->pKeys[0],(LPSTR)&Offset,FALSE);
	    	goto NextDelete;
	    }
	}
    while (SendDlgItemMessage(hWndDlg,ControlID,LB_GETTEXT,item,(DWORD)str) != LB_ERR)
    {
		DecodeTravLegData (TravID,item,pLegData,str,NULL,NULL);  
		if (pLegData->Type == 1)
			POBItem = item;
		GWDReplaceRecord (lpGWDHead,0,NULL,-1);  
		item++;
	}
	GlobalUnlock (hDB);  
	CloseGWDatabase (hDB);   
	return TRUE;
}

void TravCompute (HWND hWndDlg)
{    
	double	size,Perim,area;
	DPOINT	Points[3],POB;
	short	choice; 
	BOOL	HavePOB=FALSE;   
	char	str[256]; 
	HPDPOINT	pPoints;  
	double		CloseErr; 
	HANDLE	hExpandedPoints;
	WORD	nExpandedPoints; 
	LPBYTE	pType;
	LPLEGDATA	pLegData;  
	MNMXCORD	TravBounds;
	
	HaveClosure=FALSE;			
//	SetDlgItemText (hWndDlg,IDC_SNAPDIST,0); 
	choice=0;    
	GSSiGlobFree (&hTravPoints);
	GSSiGlobFree (&hLegData);
	GSSiGlobFree (&hSnappedPoints);
	GSSiGlobFree (&hPointType);
	GSSiGlobFree (&hSnapStatus);  
	DBoundsInit (&TravBounds);
	nLegs = nTravPoints = 0;
	EnableWindow (GetDlgItem(hWndDlg,IDC_POB),TRUE);
	EnableWindow (GetDlgItem(hWndDlg,IDC_CLOSETRAV),FALSE);
	EnableWindow (GetDlgItem(hWndDlg,IDC_GENERATE),FALSE);    
 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_SNAPDIST),FALSE);
 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_SNAPDISTTITLE),FALSE);
 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_SNAPDISTTITLE2),FALSE);
	//EnableWindow (GetDlgItem(hWndDlg,IDC_ADDLEG),TRUE);
	LoadTravAddLegOptions (hWndDlg,IDC_ADDLEG,1);
	*str=0;
	SetDlgItemText (hWndDlg,IDC_MESS,str);

    while (SendDlgItemMessage(hWndDlg,IDC_TRAVLIST,LB_GETTEXT,choice++,(DWORD)str)!=LB_ERR)
    {   
    	double	Dist;
			    	
      	DecodeTravLegData (CurTravID,choice,&CurLegData,str,Points,&Dist);
		if (choice == 1)
		{
	 		char	curproject[32], SaveProj[32]; 
				 		
	 		GetGlobalCVal ("[%ALT_PROJECTION]",SaveProj,NULL);
			if (TravDest == DEST_TEST) 
	 			_fstrcpy (curproject,"baseproj.cvt"); 
	 		else
            	GetGlobalCVal ("[%TRAVPROJECTION]",curproject,"latlon83.cvt");
            SetGlobalValue("%ALT_PROJECTION",curproject);
			ConvertCoordClose ();
			ConvertCoordInit(); 
			AZAdjust = 0;
			if (TravDest != DEST_TEST)
			{
				if (SendDlgItemMessage (hWndDlg,IDC_ADJUSTAZ,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
					AZAdjust = GetAzAdjustment (Points[0]);
				else
				{
					GetDlgItemText (hWndDlg,IDC_ROTATEDEG,str,32);
					AZAdjust = atof (str) * DEGtoRAD;
				}
			}   
            SetGlobalValue("%ALT_PROJECTION",SaveProj);
			ConvertCoordClose ();
		}
      	switch (CurLegData.Type)
      	{
      		case 0: 
      			SetGlobalValueDPoint ("%STARTPOINT",Points[2]);
      		break;
      		case 1: //POB  
      			HavePOB = TRUE; 
      			POBItem = choice - 1;
      			POB = Points[2]; 
      			AddDPointToMinMax (&POB,&TravBounds);
      			SetGlobalValueDPoint ("%POB",POB);
      			EnableWindow (GetDlgItem(hWndDlg,IDC_POB),FALSE);
				EnableWindow (GetDlgItem(hWndDlg,IDC_CLOSETRAV),TRUE); 
				GSSiGlobFree (&hLegData);
				GSSiGlobFree (&hTravPoints); 
				GSSiGlobFree (&hSnappedPoints);
				GSSiGlobFree (&hPointType);
				GSSiGlobFree (&hSnapStatus);
				hTravPoints = GSSiGlobAlloc (1080,GMEM_MOVEABLE,(long)UINT_MAX*sizeof(DPOINT));
				hLegData = GSSiGlobAlloc (1081,GMEM_MOVEABLE,(long)UINT_MAX); 
				nLegs = 0;
				pPoints = (HPDPOINT)GlobalLock (hTravPoints);  
				*pPoints++ = POB;
				nTravPoints=1;    
				GlobalUnlock (hTravPoints);
				hPointType = GSSiGlobAlloc (1082,GHND,(long)UINT_MAX);
      		break;
      		case 3: //closure   
      			HaveClosure = TRUE;
				if (TravDest == DEST_UPDATE) 
				{
					EnableWindow (GetDlgItem(hWndDlg,IDC_GENERATE),TRUE);
				 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_SNAPDIST),TRUE);
				 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_SNAPDISTTITLE),TRUE);
				 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_SNAPDISTTITLE2),TRUE);
				}    
				EnableWindow (GetDlgItem(hWndDlg,IDC_CLOSETRAV),FALSE);
				//EnableWindow (GetDlgItem(hWndDlg,IDC_ADDLEG),FALSE);
				LoadTravAddLegOptions (hWndDlg,IDC_ADDLEG,2);
				if (HavePOB)
				{   
					long	icoord;
					HPDPOINT	pSnapCoord;
								
					CloseErr = ldistp (POB,Points[2]);
					CloseErr = ConvertDist (CloseErr,1);
/*					if (SendDlgItemMessage (hWndDlg,IDC_FORCECLOSE,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
					{
						pPoints = (HPDPOINT)GlobalLock (hTravPoints);
						pPoints[nTravPoints++]=Points[2]; 
						GlobalUnlock (hTravPoints);
					} */
					hExpandedPoints = ExpandTravPoints (hTravPoints,hPointType,nTravPoints,&nExpandedPoints);
					pPoints = (HPDPOINT)GlobalLock (hExpandedPoints); 
					size = ComputeAreaAreaD (pPoints,nExpandedPoints,&Perim);
					GSSiGlobUlFree (&hExpandedPoints);
					area = ConvertArea (size,6); 
					sprintf (str,"Closure error = %.4f Feet, Acres = %.4f",CloseErr,area);
					SetDlgItemText (hWndDlg,IDC_MESS,str);
					GSSiGlobFree (&hSnapStatus);      
					hSnapStatus = GSSiGlobAlloc (1083,GMEM_MOVEABLE,(long)sizeof(DPOINT)*nTravPoints);
					pSnapCoord = (HPDPOINT)GlobalLock (hSnapStatus);
					for (icoord=0;icoord<nTravPoints;icoord++,pSnapCoord++)
						pSnapCoord->x=DBL_MAX;
					GlobalUnlock (hSnapStatus);
					pLegData = (LPLEGDATA)GlobalLock (hLegData); 
					pLegData += nLegs;
					pLegData->Type = 13;
					GlobalUnlock (hLegData);
       				nLegs++;

				}  
			break;
			case 11://distance and bearing  
			{
				if (!HavePOB)
					break;
				if (!HaveClosure)
				{
					pPoints = (HPDPOINT)GlobalLock (hTravPoints);
					pPoints+=nTravPoints; 
					*pPoints++ = Points[2];
					nTravPoints++;    
					GlobalUnlock (hTravPoints);
				} 
				pLegData = (LPLEGDATA)GlobalLock (hLegData); 
				pLegData += nLegs;
				pLegData->Type = 2;
				pLegData->Points[0] = Points[0];
				pLegData->Points[1] = Points[2]; 
      			AddDPointToMinMax (&Points[0],&TravBounds);
      			AddDPointToMinMax (&Points[2],&TravBounds);
				pLegData->Dist = Dist;
				GlobalUnlock (hLegData);  
				nLegs++;
			}
			break;
			case 21: //type 1 curve   
			case 22: //type 2 curve 
			case 23: //type 3 curve 
			{
							
				if (!HavePOB)
					break;
				if (!HaveClosure)
				{
					pPoints = (HPDPOINT)GlobalLock (hTravPoints);
					pPoints+=nTravPoints;     
					pType = GlobalLock (hPointType);
					pType += nTravPoints;
					*pPoints++ = Points[1]; 
					*pType = 1;
					GlobalUnlock (hPointType);
					*pPoints++ = Points[2];
					nTravPoints+=2;    
					GlobalUnlock (hTravPoints);
				}  
				pLegData = (LPLEGDATA)GlobalLock (hLegData); 
				pLegData += nLegs;
				pLegData->Type = 3;
				pLegData->Points[0] = Points[0];
				pLegData->Points[1] = Points[1];   
				pLegData->Points[2] = Points[2];   
      			AddDPointToMinMax (&Points[0],&TravBounds);
      			AddDPointToMinMax (&Points[1],&TravBounds);
      			AddDPointToMinMax (&Points[2],&TravBounds);
				pLegData->Dist = Dist;
				GlobalUnlock (hLegData);  
				nLegs++;
			}
			break;
			case 31:  
			{
				pLegData = (LPLEGDATA)GlobalLock (hLegData); 
				LoadTravAddLegOptions (hWndDlg,IDC_ADDLEG,3);
				pLegData[nLegs].Type = 31;
				pLegData[nLegs].Points[0] = pLegData[SubTravStartLeg].Points[0];   
				Points[1] = pLegData[nLegs].Points[1] = pLegData[SubTravStartLeg-1].Points[0];   
				Points[2] = pLegData[nLegs].Points[2] = pLegData[SubTravStartLeg-1].Points[1];   
				GlobalUnlock (hLegData);
				nLegs++; 
			}
			break; 
		} 
    } 
    SetGlobalValueBounds ("%TRAVBOUNDS",&TravBounds);
    return;
}  

BOOL ShowTrav (HWND hWndDlg,UINT IDCTRAVLIST,long CurTravID)
{  
	HANDLE	hDB;
    LPGWDHEADER lpGWDHead; 
    LPGWFLDINFO lpGWFldInfo; 
	LPTRAVLEGDATA	pLegData;    
	char	str[256]; 
    long	Offset;   
	short	st, choice;
	
    SendDlgItemMessage (hWndDlg,IDCTRAVLIST,LB_RESETCONTENT,0,0);
	hDB = OpenGWDatabase ("[%TRAVDATADB]",BT_READ);
	if (!hDB)
	{   
		MessageBox(GetFocus(),"Unable to open traverse database", 0,MB_ICONEXCLAMATION|MB_OK);
		return FALSE; 
	}
	EnableWindow (GetDlgItem(hWndDlg,IDC_POB),TRUE);
	EnableWindow (GetDlgItem(hWndDlg,IDC_NEXT),TRUE);
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
	pLegData = (LPTRAVLEGDATA)&lpGWDHead->GWDData;
	pLegData->ID = CurTravID;
	pLegData->LegNum = -1;     
	GWDFormKey(lpGWDHead,0,TRUE,0);
	st = BT_FIND (lpGWDHead->BTHandle[0],lpGWDHead->pKeys[0],BT_FIRST,BT_GE, (LPSTR)&Offset);
	while (!st)
	{   
		FillGWDData (lpGWDHead,Offset);  
		if (pLegData->ID != CurTravID) 
			break;     
		EncodeTravLegData (pLegData,str);   
		if (pLegData->Type == 3)
			EnableWindow (GetDlgItem(hWndDlg,IDC_POB),FALSE);
		choice = SendDlgItemMessage (hWndDlg,IDCTRAVLIST,LB_ADDSTRING,0,(LPARAM)str);  
		st = BT_FIND (lpGWDHead->BTHandle[0],lpGWDHead->pKeys[0],BT_NEXT,BT_ANY, (LPSTR)&Offset);
	}   
	GlobalUnlock (hDB);  
	CloseGWDatabase (hDB);   
    return TRUE;
}  

void LoadTravAddLegOptions (HWND hWndDlg,UINT IDCADDLEG,int Opt)
{
   	int	nItems = SendDlgItemMessage (hWndDlg,IDCADDLEG,LB_GETCOUNT,0,0);
   	int	Choice = SendDlgItemMessage (hWndDlg,IDCADDLEG,LB_GETCURSEL,0,0); 
   	
   	SendDlgItemMessage (hWndDlg,IDCADDLEG,LB_RESETCONTENT,0,0);
    if (Opt != 2)
    {
	 	SendDlgItemMessage (hWndDlg,IDCADDLEG,LB_ADDSTRING,NULL,
	 						(LPARAM)"Bearing and Distance\t11");
	 	SendDlgItemMessage (hWndDlg,IDCADDLEG,LB_ADDSTRING,NULL,
	 						(LPARAM)"Distance along curve\t21"); 
	}
 	if (Opt > 1)
 		SendDlgItemMessage (hWndDlg,IDCADDLEG,LB_ADDSTRING,NULL,
 						(LPARAM)"Begin Sub-Traverse\t31");
//	 	SendDlgItemMessage (hWndDlg,IDC_ADDLEG,LB_ADDSTRING,NULL,
//	 						(LPARAM)"Distance along type 2 curve (Direction, radius point bearing and radius)\t22");
//	 	SendDlgItemMessage (hWndDlg,IDC_ADDLEG,LB_ADDSTRING,NULL,
//	 						(LPARAM)"Distance along type 3 curve (Direction, radius and tangent length)\t23");
	SendDlgItemMessage (hWndDlg,IDCADDLEG,LB_SETCURSEL,0,0);
	return;
}

void TravShowSelectedLeg (HWND hWndDlg,UINT IDCTRAVLIST,UINT IDCCALLNO,int choice)
{   
	long	Refno;
	char	str[256];
	
	if (choice > POBItem)    
	{   
		if (!CurrentConfig)
			SetConfig (1);
 		SetViewport (*pCommandViewport);
		Refno = choice-POBItem;
	    ltoa (Refno,str,10);
	    SetDlgItemText (hWndDlg,IDCCALLNO,str); 
//		if (SendDlgItemMessage (hWndDlg,IDC_TESTMODE,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)) 
		if (TravDest == DEST_TEST)
		{   
	    	if (PickByRefno(Refno,NULL,NULL,GetPickFile (-1)))
	    	{
	    		AddToHighlightList (Refno,&PickList[0],TRUE);
				ProcessPickedItem (0,TRUE);
//				RedisplayViewport(FALSE,FALSE);
			} 
	    }
	}
	return;
} 

void AdjustSubTraverse (HWND hWndDlg,UINT IDCTRAVLIST,int From,int Increment)
{   
	int	choice = 0;
	TRAVLEGDATA	LegData;   
	char	str[256];
	int	ModLeg=INT_MAX;
	
	while (SendDlgItemMessage(hWndDlg,IDC_TRAVLIST,LB_GETTEXT,choice,(DWORD)str) != LB_ERR)
	{
     	DecodeTravLegData (CurTravID,choice,&LegData,str,NULL,NULL);
    	if (LegData.Type==1)
    		ModLeg = From - choice;
    	else if (LegData.Type==31 && SubTravStartLeg >= ModLeg)
    	{
			SubTravStartLeg += Increment;    	
		    sprintf (LegData.Parameters,"StartLeg=%i",SubTravStartLeg);
		    EncodeTravLegData (&LegData,str); 
			SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_DELETESTRING,choice,0); 
			SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_INSERTSTRING,choice,(LPARAM)str); 
		}
		choice++;
	} 
	return;
}   

BOOL FAR PASCAL TRAVERSE_ENTRYMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 	int		st, choice, n, nItems;
	int		TabStopsAddLeg[2]={9000,9100};
	long	Refno; 
	char	str[256]; 
	char	SymName[36];  
	HANDLE	hDB;
    LPGWDHEADER lpGWDHead; 
    LPGWFLDINFO lpGWFldInfo; 
    long	Offset;   
    LPSTR	pTAB; 
//	static	short	symopt=0, lsymopt; 
	static	char	CurTAG[10]="", CurUDI[34]="";  
	LPTRAVIDDATA	pTravIDData;
	LPTRAVLEGDATA	pLegData;    
	DPOINT			Points[3],POB; 
	HANDLE	hExpandedPoints;
	WORD	nExpandedPoints;   
	LPVIEWPORT	SaveVP=CurView;  
	RECT	TravWndRect; 
	BOOL	MustClose;
	
	
 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
	{    
    	 UndoAddWindow (hWndDlg);
 		 ClearHighlightList (FALSE); 
 		 TravDest = DEST_TEST;
      	 SetGlobalValueBool ("%USEFULLMAP",FALSE);
 		 SendDlgItemMessage (hWndDlg,IDC_AUTOLINES,BM_SETCHECK,TRUE,0L);
 		 SendDlgItemMessage (hWndDlg,IDC_SHOWNODES,BM_SETCHECK,FALSE,0L);
  
//		 CurTravID = 0;  
//		 *CurUDI = 0; 
		 GSSiGlobFree (&hSnappedPoints);
		 GSSiGlobFree (&hTravPoints);
		 GSSiGlobFree (&hLegData);
		 nLegs = nTravPoints=0;
		 SetDlgItemTextGlobal (hWndDlg,IDC_SNAPDIST,"[%TRAVSNAPDIST]","");
// 		 SendDlgItemMessage (hWndDlg,IDC_TESTMODE,BM_SETCHECK,UseTestMode,0L);
//		 SendDlgItemMessage (hWndDlg,IDC_FORCECLOSE,BM_SETCHECK,TRUE,0L);
       	 SendDlgItemMessage (hWndDlg,IDC_ADDLEG,LB_SETTABSTOPS,2,(LPARAM)TabStopsAddLeg);
       	 SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_SETTABSTOPS,2,(LPARAM)TabStopsAddLeg);
		 LoadTravAddLegOptions (hWndDlg,IDC_ADDLEG,1);  
		 if (!LoadTravTAGs (hWndDlg,IDC_TAPREFIX))
		 {
			PostMessage(hWndDlg, WM_CLOSE, 0, 0L);
		 	break;
		 }
		SendDlgItemMessage (hWndDlg,IDC_TAPREFIX,CB_SELECTSTRING,-1,(LPARAM)CurTAG);  
		SetDlgItemText (hWndDlg,IDC_UDI,CurUDI); 
//        FillCBList (hWndDlg,IDC_SYMBOL,"[%DL]travsyms.txt",symopt,0);
//        FillCBList (hWndDlg,IDC_SYMBOLLINE,"[%DL]travsyml.txt",lsymopt,0);
		SetDlgItemTextGlobal (hWndDlg,IDC_TAPREFIX,"[%TRAVPREFIX]",NULL);
		SetDlgItemTextGlobal (hWndDlg,IDC_UDI,"[%TRAVUDI]",NULL);
//		SetDlgItemTextGlobal (hWndDlg,IDC_SYMBOL,"[%TRAVSYM]",NULL);
//		SetDlgItemTextGlobal (hWndDlg,IDC_SYMBOLLINE,"[%TRAVSYML]",NULL);  
		if (SetConfig (0))
		{   
			RECT	ClientRect;
			
			SetViewport(*pCommandViewport);
			ClientRect  = CurView->Rect;
			ClientRectToScreenRect (CurView->hWnd,&ClientRect);
			SetWindowPos(hWndDlg,HWND_TOP,ClientRect.left+1,ClientRect.top+1,0,0,SWP_NOSIZE|SWP_NOZORDER);
        }
        CurView = SaveVP;
        SetConfig (1);
		SetFocus (GetDlgItem(hWndDlg,IDC_UDI)); 
		if (GetDlgItemText (hWndDlg,IDC_UDI,str,64))
		{
			ShowTrav (hWndDlg,IDC_TRAVLIST,CurTravID);
		} 
//		if (TravDest == DEST_TEST) 
	 	GetWindowRect(hWndDlg, &TravWndRect); 
		SetGlobalValueRect ("%TRAVRECT",TravWndRect);
		PostMessage(hWndDlg, WM_COMMAND, IDC_TESTMODE, 0L);
//		if (SendDlgItemMessage (hWndDlg,IDC_AUTOLINES,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)) 
//			PostMessage(hWndDlg, WM_COMMAND, IDC_CREATELINES, 0L);  
	} 

        break; /* End of WM_INITDIALOG                                 */
    case WM_SIZE:     /*  code for sizing client area                   */  
         switch (wParam)
           {
            case SIZE_MINIMIZED: 
            	SetGlobalValueBool ("%USEFULLMAP",TRUE);
                 break;
			case SIZE_MAXIMIZED:
            case SIZE_RESTORED:  
            	SetGlobalValueBool ("%USEFULLMAP",FALSE);
           		 break;
            default: 
                 break;
           }
		    sprintf (str,"[C]=$MACRO([%%DL]macros\\travmode.txt,%i)",4); 
		    ExpandText (str);
         break;

	case GF_UNDOCOMPLETED:
		 ShowTrav (hWndDlg,IDC_TRAVLIST,CurTravID);
		 break;
		 
    case WM_CLOSE: 
		 CloseTRANS2 (&hTravTran);
		 GSSiGlobFree (&hTravPoints);
		 GSSiGlobFree (&hLegData);
		 GSSiGlobFree (&hSnappedPoints);
		 GSSiGlobFree (&hPointType);
		 GSSiGlobFree (&hSnapStatus);
    	 GetDlgItemText (hWndDlg,IDC_UDI,CurUDI,sizeof(CurUDI)-1);  
    	 GetDlgItemText (hWndDlg,IDC_TAPREFIX,CurTAG,9);   
 		 GetDlgItemTextGlobal (hWndDlg,IDC_SNAPDIST,"[%TRAVSNAPDIST]");
 		 GetDlgItemTextGlobal (hWndDlg,IDC_TAPREFIX,"[%TRAVPREFIX]");
 		 GetDlgItemTextGlobal (hWndDlg,IDC_UDI,"[%TRAVUDI]");
//		 GetDlgItemTextGlobal (hWndDlg,IDC_SYMBOL,"[%TRAVSYM]");
//		 GetDlgItemTextGlobal (hWndDlg,IDC_SYMBOLLINE,"[%TRAVSYML]");
//		 UseTestMode = SendDlgItemMessage (hWndDlg,IDC_TESTMODE,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L); 

    	 if (CurTravID)
    	 	SaveTrav (CurTravID,hWndDlg,IDC_TRAVLIST);  
//	     symopt =(short)SendDlgItemMessage(hWndDlg,IDC_SYMBOL,CB_GETCURSEL,0,0); 
//	     lsymopt =(short)SendDlgItemMessage(hWndDlg,IDC_SYMBOLLINE,CB_GETCURSEL,0,0);    
	     DestroyWindow(hWndDlg); 
       	 SetGlobalValueBool ("%USEFULLMAP",TRUE);
	     sprintf (str,"[C]=$MACRO([%%DL]macros\\travmode.txt,%i)",1); 
	     if (*CfgName)
	     	ExpandText (str);
         hWndTraverseEntry = 0; 
		 FreeProcInstance(lpfnTRAVERSE_ENTRYMsgProc); 
     	 UndoRemoveWindow (hWndDlg);
		 if (*CfgName)
		 	RedisplayWindow ();
	     break;
	      
    case WM_COMMAND:
         switch(wParam)
           {
			
			case IDOK:
				goto AddLeg;
           	case IDC_ADDLEG:
              	switch(HIWORD(lParam))
                {
		             case CBN_SELCHANGE:  
		    AddLeg:     
		    			//set CurrentAZ from previous leg if there is one
		                choice=SendDlgItemMessage(hWndDlg,IDC_TRAVLIST, LB_GETCURSEL,NULL,NULL); 
		                if (choice == LB_ERR)
		                {
		                	choice=SendDlgItemMessage(hWndDlg,IDC_TRAVLIST, LB_GETCOUNT,NULL,NULL);
		                	choice--;
		                }
		                if (choice != LB_ERR)
		                {
				         	SendDlgItemMessage(hWndDlg,IDC_TRAVLIST,LB_GETTEXT,choice,(DWORD)str);
				         	DecodeTravLegData (CurTravID,choice,&CurLegData,str,Points,NULL);
				        }
		             	_fmemset (&CurLegData,0,sizeof(TRAVLEGDATA));
		             	choice = SendDlgItemMessage(hWndDlg,IDC_ADDLEG, LB_GETCURSEL,NULL,NULL);
		             	if (choice == LB_ERR)
		             		break; 
		             		
			         	SendDlgItemMessage(hWndDlg,IDC_ADDLEG,LB_GETTEXT,choice,(DWORD)str);
			         	pTAB = _fstrchr(str,'\t');
			         	pTAB++;
		                CurLegData.Type=atoi(pTAB);
		                if (!EditTravLegData (hWndDlg,&CurLegData))
		                	break;
						CreateCheckPoint ("Add Traverse Leg");
		                EncodeTravLegData (&CurLegData,str);  
		                if (CurLegData.Type==31)
		                	choice = -1;
		                else 
		                {
			                choice=SendDlgItemMessage(hWndDlg,IDC_TRAVLIST, LB_GETCURSEL,NULL,NULL);
			                if (choice >= 0)
			                	choice++; 
			            }
						choice = SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_INSERTSTRING,choice,(LPARAM)str);  
		 		        AdjustSubTraverse (hWndDlg,IDC_TRAVLIST,choice,1);
						nItems = SendDlgItemMessage(hWndDlg,IDC_TRAVLIST, LB_GETCOUNT,NULL,NULL);
						if (choice + 1 > nItems)
							choice = -1;
						if (choice < 0)
							SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_SETTOPINDEX,nItems-1,0);
						else
							SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_SETTOPINDEX,choice,0);
						SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_SETCURSEL,choice,0);
						SetFocus (GetDlgItem(hWndDlg,IDOK));
						ClearHighlightList (FALSE);
						if (SendDlgItemMessage (hWndDlg,IDC_AUTOLINES,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)) 
         					PostMessage(hWndDlg, WM_COMMAND, IDC_CREATELINES, 0L);  
         				else
							TravCompute (hWndDlg);
					 break;
				}
			break;
			
           	case IDC_TRAVLIST:
              	switch(HIWORD(lParam))
                {    
 		             case LBN_SELCHANGE:
						ClearHighlightList (TRUE);
 		                SetDlgItemText (hWndDlg,IDC_CALLNO,""); 
		                choice=SendDlgItemMessage(hWndDlg,IDC_TRAVLIST, LB_GETCURSEL,NULL,NULL);
		                if (choice > 0) 
		                {
 		             		EnableWindow (GetDlgItem(hWndDlg,IDC_REMOVELEG),TRUE);
 		             		EnableWindow (GetDlgItem(hWndDlg,IDC_EDLEG),TRUE); 
 		             		TravShowSelectedLeg (hWndDlg,IDC_TRAVLIST,IDC_CALLNO,choice);
 		             	}
 		             	else 
 		             	{
 		             		EnableWindow (GetDlgItem(hWndDlg,IDC_REMOVELEG),FALSE);
 		             		EnableWindow (GetDlgItem(hWndDlg,IDC_EDLEG),FALSE); 
 		             	}
						SetFocus (GetDlgItem(hWndDlg,IDOK));
 		             	break; 
		             case LBN_DBLCLK: 
         			 	PostMessage(hWndDlg, WM_COMMAND, IDC_EDLEG, 0L);
					 break;
				}
			break;    
			
			case IDC_EDLEG:
                choice=SendDlgItemMessage(hWndDlg,IDC_TRAVLIST, LB_GETCURSEL,NULL,NULL);
	         	SendDlgItemMessage(hWndDlg,IDC_TRAVLIST,LB_GETTEXT,choice,(DWORD)str);
	         	DecodeTravLegData (CurTravID,choice,&CurLegData,str,Points,NULL);
                if (!EditTravLegData (hWndDlg,&CurLegData))
                	break;
				CreateCheckPoint ("Edit Traverse Leg");
                EncodeTravLegData (&CurLegData,str);
				SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_DELETESTRING,choice,(LPARAM)str);
				choice=SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_INSERTSTRING,choice,(LPARAM)str);
				SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_SETCURSEL,choice,0); 
				SetFocus (GetDlgItem(hWndDlg,IDOK));
				if (SendDlgItemMessage (hWndDlg,IDC_AUTOLINES,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)) 
 					PostMessage(hWndDlg, WM_COMMAND, IDC_CREATELINES, 0L);  
 				else
 					TravCompute (hWndDlg);

			    break;
			    
			case IDC_TRAVCOMPUTE: 
				TravCompute (hWndDlg);
			break;             
			
			case IDC_NEXT:
				EnableWindow (GetDlgItem(hWndDlg,IDC_NEXT),TRUE);
		        choice=(short)SendDlgItemMessage(hWndDlg,IDC_UDI, CB_GETCURSEL,0,0); 
		        if (choice == CB_ERR)
		        	break;  
		        if (SendDlgItemMessage(hWndDlg,IDC_UDI, CB_SETCURSEL,choice+1,0) != CB_ERR)
					goto ShowNext;
				break;
					
        	case IDC_UDI:
        	{
              switch(HIWORD(lParam))
              {
	             case CBN_SELCHANGE:
	             case CBN_DBLCLK:   
	       ShowNext:
		            choice=(short)SendDlgItemMessage(hWndDlg,IDC_UDI, CB_GETCURSEL,0,0);
		            SendDlgItemMessage(hWndDlg,IDC_UDI,CB_GETLBTEXT,choice, (LPARAM)CurUDI); 
                	CurTravID = SendDlgItemMessage(hWndDlg,IDC_UDI,CB_GETITEMDATA,choice, (LPARAM) 0); 
           ShowTrav:
		 			ClearHighlightList (FALSE);
           			ShowTrav (hWndDlg,IDC_TRAVLIST,CurTravID);
					//EnableWindow (GetDlgItem(hWndDlg,IDC_ADDLEG),TRUE);
					LoadTravAddLegOptions (hWndDlg,IDC_ADDLEG,1);
				    EnableWindow (GetDlgItem(hWndDlg,IDC_REMOVELEG),FALSE);
	    	 		GetDlgItemText (hWndDlg,IDC_TAPREFIX,CurTAG,9);  
/*	                if (PickByRefno(0,CurTAG,CurUDI,-1)) 
	                {
				 		SetViewport (*pCommandViewport);
				 		if (SendDlgItemMessage (hWndDlg,IDC_SHOWNODES,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
				 			ShowNodesRef = PickList[0].Refno;
				 		else
				 			ShowNodesRef=LONG_MAX; 
				 		ClearSpecial ();
				 		CreateSpecial ();
 						AddSpecial (PickList[0].Refno,1,RGB(255,255,0),3);
	                	SetWindowText (hWndDlg,"Traverse Entry");
			    		ZoomToPickedItem(0,100,TRUE,TRUE,FALSE); 
			    	}
			    	else
			    		SetWindowText (hWndDlg,"Not located");*/
					TravDest = DEST_TEST; 
					RemoveTravTranFile ();
					TravCompute (hWndDlg);
	         		PostMessage(hWndDlg, WM_COMMAND, IDC_TESTMODE, 0L);
					RedisplayWindow ();
/*					if (SendDlgItemMessage (hWndDlg,IDC_AUTOLINES,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)) 
     					PostMessage(hWndDlg, WM_COMMAND, IDC_CREATELINES, 0L);  
     				else*/
              		break;
              	 case CBN_DROPDOWN: 
              	 {
              	 	short	n;
					hDB = OpenGWDatabase ("[%TRAVIDDB]",BT_WRITE);
					if (!hDB)
					{   
						MessageBox(GetFocus(),"Unable to open traverse database", 0,MB_ICONEXCLAMATION|MB_OK);
						break; 
					}
					lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
					pTravIDData = (LPTRAVIDDATA)&lpGWDHead->GWDData;
				    lpGWFldInfo=lpGWDHead->pFldInfo; 
				    lpGWFldInfo++;     
			    	n=0;//GetDlgItemText (hWndDlg,IDC_UDI,CurUDI,33);  
	    	 		GetDlgItemText (hWndDlg,IDC_TAPREFIX,CurTAG,9);  
	    			SetFieldValFromChar(lpGWDHead,lpGWFldInfo,CurTAG,FALSE,FALSE); 
				    lpGWFldInfo++;     
	    			SetFieldValFromChar(lpGWDHead,lpGWFldInfo,"",FALSE,FALSE); 
	    			GWDFormKey(lpGWDHead,1,TRUE,0);
			        SendDlgItemMessage (hWndDlg,IDC_UDI,CB_RESETCONTENT,0,0);
	    			st = BT_FIND (lpGWDHead->BTHandle[1],lpGWDHead->pKeys[1],BT_FIRST,BT_GE, (LPSTR)&Offset);
					while (!st)
					{   
						FillGWDData (lpGWDHead,Offset); 
						if (_fstricmp (CurTAG,pTravIDData->Prefix))
							break;
						if (n && _fstrnicmp (pTravIDData->UDI,CurUDI,n))
							break; 
						strncpy0 (str,(LPSTR)(lpGWDHead->pKeys[1]+8),32);
						choice = SendDlgItemMessage (hWndDlg,IDC_UDI,CB_ADDSTRING,0,(LPARAM)str);  
	         			SendDlgItemMessage (hWndDlg,IDC_UDI,CB_SETITEMDATA,(WPARAM)choice,(LPARAM)pTravIDData->ID);
	    				st = BT_FIND (lpGWDHead->BTHandle[1],lpGWDHead->pKeys[1],BT_NEXT,BT_ANY, (LPSTR)&Offset);
					}   
					SendDlgItemMessage (hWndDlg,IDC_UDI,CB_SELECTSTRING,-1,(LPARAM)CurUDI);
					GlobalUnlock (hDB);  
					CloseGWDatabase (hDB);   
              	  	break;      
              	  }
            	}
            	break;
            }
             
            
			case IDC_NEWTRAV:  
				if (CurTravID)
					SaveTrav (CurTravID,hWndDlg,IDC_TRAVLIST);  
				if (!HaveTravStartPoint)
                { 
					if (MessageBox (GetFocus(),"Start point not set - Use assumed start point?","No Start Point",MB_YESNO|MB_ICONQUESTION) == IDYES)
					{
						GetGlobalPVal ("[%ASSUMEDSTARTPOINT]",NULL,&TravStartPoint);
					}
					else
					{
                		MessageBox(GetFocus(),"Start point not set", 0,MB_ICONEXCLAMATION|MB_OK);
                		break;
                	}
                } 
				hDB = OpenGWDatabase ("[%TRAVIDDB]",BT_WRITE);
				if (!hDB)
				{   
                	MessageBox(GetFocus(),"Unable to open traverse database", 0,MB_ICONEXCLAMATION|MB_OK);
					break; 
				}
		        SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_RESETCONTENT,0,0);
		    	GetDlgItemText (hWndDlg,IDC_UDI,CurUDI,33);  
    	 		GetDlgItemText (hWndDlg,IDC_TAPREFIX,CurTAG,9);  
/*			    GetDlgItemText (hWndDlg,IDC_SYMBOL,SymName,34);
			    if (!_fstricmp (SymName,"DEFAULT")) */
			    TravGetDefaultSymbol (CurTAG,SymName,1); 
				lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
			    lpGWFldInfo=lpGWDHead->pFldInfo; 
			    lpGWFldInfo++;     
    			SetFieldValFromChar(lpGWDHead,lpGWFldInfo,CurTAG,FALSE,FALSE); 
			    lpGWFldInfo++;     
    			SetFieldValFromChar(lpGWDHead,lpGWFldInfo,CurUDI,FALSE,FALSE); 
    			GWDFormKey(lpGWDHead,1,TRUE,0);
    			if (!BT_FIND (lpGWDHead->BTHandle[1],lpGWDHead->pKeys[1],BT_FIRST,BT_EQ, (LPSTR)&Offset))
				{   
                	MessageBox(GetFocus(),"This traverse already exists", 0,MB_ICONEXCLAMATION|MB_OK);
					GlobalUnlock (hDB);  
					CloseGWDatabase (hDB);   
					break; 
				}
				CreateCheckPoint ("Create New Traverse");
    			if (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&CurTravID,BT_LAST,BT_ANY, (LPSTR)&Offset))
    				CurTravID++;
    			else
    				CurTravID = 1;
				pTravIDData = (LPTRAVIDDATA)&lpGWDHead->GWDData;
				pTravIDData->ID = CurTravID;
			    lpGWFldInfo=lpGWDHead->pFldInfo; 
			    lpGWFldInfo++;     
    			SetFieldValFromChar(lpGWDHead,lpGWFldInfo,CurTAG,FALSE,FALSE); 
			    lpGWFldInfo++;     
    			SetFieldValFromChar(lpGWDHead,lpGWFldInfo,CurUDI,FALSE,FALSE); 
			    lpGWFldInfo++;     
    			SetFieldValFromChar(lpGWDHead,lpGWFldInfo,SymName,FALSE,FALSE); 
				GWDReplaceRecord (lpGWDHead,0,NULL,-1);
				GlobalUnlock (hDB);  
				CloseGWDatabase (hDB);   
	            EnlargeScreen (0,0);
                sprintf (str,"Starting at %.4lf %.4lf\t0",TravStartPoint.x,TravStartPoint.y); 
				SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_ADDSTRING,NULL,(LPARAM)str);
				EnableWindow (GetDlgItem(hWndDlg,IDC_POB),TRUE);
				//EnableWindow (GetDlgItem(hWndDlg,IDC_ADDLEG),TRUE);
				LoadTravAddLegOptions (hWndDlg,IDC_ADDLEG,1);
	            EnableWindow (GetDlgItem(hWndDlg,IDC_REMOVELEG),FALSE); 
				TravDest = DEST_TEST;
				HaveClosure = FALSE; 
				RemoveTravTranFile ();
//				if (SendDlgItemMessage (hWndDlg,IDC_AUTOLINES,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)) 
// 					PostMessage(hWndDlg, WM_COMMAND, IDC_CREATELINES, 0L);  
				SetFocus (hWndDlg);
				SetFocus (GetDlgItem(hWndDlg,IDOK));
         		PostMessage(hWndDlg, WM_COMMAND, IDC_TESTMODE, 0L);

			break;
			
			case IDC_TRAVDELETE: 
				GetDlgItemText (hWndDlg,IDC_UDI,CurUDI,33);
				sprintf (str,"Do you really wish to delete the traverse for %s?",CurUDI);
				if (MessageBox (GetFocus(),str,"Verify Delete",MB_YESNO|MB_ICONQUESTION) != IDYES)
					break;
				CreateCheckPoint ("Delete Traverse");
			break;
			
//			case IDC_FORCECLOSE:
//         		PostMessage(hWndDlg, WM_COMMAND, IDC_TRAVCOMPUTE, 0L);
//			break;
			
			case IDC_STARTPOINT:
				if (!HaveTravStartPoint) 
				{
					if (MessageBox (GetFocus(),"Start point not set - Use assumed start point?","No Start Point",MB_YESNO|MB_ICONQUESTION) == IDYES)
					{
						GetGlobalPVal ("[%ASSUMEDSTARTPOINT]",NULL,&TravStartPoint);
					}
					else
					{
                		MessageBox(GetFocus(),"Start point not set", 0,MB_ICONEXCLAMATION|MB_OK);
                		break;
                	} 
                }
				CreateCheckPoint ("Set Traverse Start Point");
	            EnlargeScreen (0,0);
                sprintf (str,"Starting at %.4lf %.4lf\t0",TravStartPoint.x,TravStartPoint.y); 
				SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_DELETESTRING,0,0L);
				SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_INSERTSTRING,0,(LPARAM)str);
				TravCompute (hWndDlg);
				SetFocus (GetDlgItem(hWndDlg,IDOK));
				break;   
				
			case IDC_REMOVELEG:
				CreateCheckPoint ("Remove Traverse Leg");
                choice=SendDlgItemMessage(hWndDlg,IDC_TRAVLIST, LB_GETCURSEL,NULL,NULL);
				SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_DELETESTRING,choice,(LPARAM)str);
 		        EnableWindow (GetDlgItem(hWndDlg,IDC_REMOVELEG),FALSE);
 		        AdjustSubTraverse (hWndDlg,IDC_TRAVLIST,choice,-1);
				SetFocus (GetDlgItem(hWndDlg,IDOK));
				if (SendDlgItemMessage (hWndDlg,IDC_AUTOLINES,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)) 
 					PostMessage(hWndDlg, WM_COMMAND, IDC_CREATELINES, 0L);  
 				else
					TravCompute (hWndDlg);
          		break;
				
			case IDC_POB:
				CreateCheckPoint ("Set Traverse POB");
                choice=SendDlgItemMessage(hWndDlg,IDC_TRAVLIST, LB_GETCURSEL,NULL,NULL);
                if (choice >= 0)
                	choice++;
				SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_INSERTSTRING,choice,(LPARAM)"*** Point of Beginning ***\t1");
				nItems = SendDlgItemMessage(hWndDlg,IDC_TRAVLIST, LB_GETCOUNT,NULL,NULL);
				if (choice + 1 > nItems)
					choice = -1;
				if (choice < 0)
					SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_SETTOPINDEX,nItems-1,0);
				else
					SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_SETTOPINDEX,choice,0);
				SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_SETCURSEL,choice,0);
				SetFocus (GetDlgItem(hWndDlg,IDOK));
				TravCompute (hWndDlg);
			break;
			     
			case IDC_CLOSETRAV:
				SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_INSERTSTRING,-1,(LPARAM)"*** Close Traverse ***\t3");
				TravCompute (hWndDlg);
			break;
			     
		 	case IDC_GENERATE: 
		 	{    
		 		short	SymNum=0, NumSyms=0,PickFile; 
		 		HANDLE	hSymDesc=0; 
		 		long	Refno;
		 		double	SnapDist; 
		 		HCURSOR	hcurSave; 
		 		
		    	if (CurTravID)
		    	 	SaveTrav (CurTravID,hWndDlg,IDC_TRAVLIST);  
		 		if (!nTravPoints)
		 			break; 
				if (lParam != 1)
					TravCompute (hWndDlg);
				hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
				if (!CurrentConfig)
					SetConfig (1);
		 		SetViewport (*pCommandViewport);
		   		if (!CurView->UpdateFile)
		   		{
			 		MessageBox(GetFocus(), "No update file in this viewport", NULL,MB_ICONEXCLAMATION|MB_OK);
		            break;
		   		} 
		   		else
		   			_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
   	            
   	            if (!ExistFile(EditName))
   	            	copyfile (EditName,"[%NULLMAP]",FALSE,0,0,0,0,0,0);

		    	GetDlgItemText (hWndDlg,IDC_UDI,CurUDI,33); 
	    		SetGlobalValue ("%TRAVUDI",CurUDI);
		    	GetDlgItemText (hWndDlg,IDC_TAPREFIX,CurTAG,9); 
		    	SetGlobalValue ("%TRAVPREFIX",CurTAG);
   	 		    if (lParam != 1)
   	 		    {
   	 		    	GetDlgItemText (hWndDlg,IDC_SNAPDIST,str,64);
   	 		    	SnapDist = atof (str) * FTM;
   	 		    }
   	 		    else
   	 		    	SnapDist = 0;
			    TravGetDefaultSymbol (CurTAG,SymName,1);  
   		    	SetGlobalValue ("%TRAVSYMBOL",SymName);
/*			    symopt =(short)SendDlgItemMessage(hWndDlg,IDC_SYMBOL,CB_GETCURSEL,0,0); 
			    if (symopt>=0)
			    {
				    GetDlgItemText (hWndDlg,IDC_SYMBOL,SymName,34);*/ 
	                SymNum = GetOrCreateSym (SymName,&NumSyms,&hSymDesc,FALSE,0);
	                if (!SymNum)
	                { 
	                	MessageBox(GetFocus(),"Symbol not found", SymName,MB_ICONEXCLAMATION|MB_OK);
	                	break;
	                } 
/*	            }
	            else  
		   		{
			 		MessageBox(GetFocus(), "Symbol not set", NULL,MB_ICONEXCLAMATION|MB_OK);
		            break;
		   		} */
/*                if (SendDlgItemMessage (hWndDlg,IDC_FORCECLOSE,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
                {
                	HPDPOINT	pPoint1=GlobalLock (hTravPoints);
                	HPDPOINT	pPointLast = pPoint1 + (nTravPoints-1);
                	
                	*pPointLast = *pPoint1;
                	GlobalUnlock (hTravPoints);
                }*/  
				CreateCheckPoint ("Generate Traverse Area");
				PickFile = GetPickFile (-1);
				if (PickByRefno(0,CurTAG,CurUDI,PickFile))
				{				 
					Refno = PickList[0].Refno;
					DeletePickedItem (0,12,92); 
				}
				else
					Refno = GetNewRefno (EditName,NULL,NULL,NULL,NULL);
				DoNotPickThisRefno=Refno;
				GSSiGlobFree (&hSnappedPoints);
				hSnappedPoints = SnapTravPoints (hTravPoints,hSnapStatus,nTravPoints,SnapDist,HaveClosure);
//					(BOOL)SendDlgItemMessage (hWndDlg,IDC_FORCECLOSE,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L));   
				SaveTravTransformation (hTravPoints,hSnappedPoints,nTravPoints);

				DoNotPickThisRefno=LONG_MAX;
				hExpandedPoints = ExpandTravPoints (hSnappedPoints,hPointType,nTravPoints,&nExpandedPoints);
				AddToSymList (SymNum,&NumSyms,&hSymDesc); 
	    		_fstrcpy (PltName,EditName);
	    		PltType = 2;
				OpenMap (CurView->hWnd,CurView->hDC);
				EditBounds = CurView->FileMNMX; 
				CloseMap (FALSE);
				AddPolyToMap (1,&nExpandedPoints, &hExpandedPoints,0,Refno,NULL,-1,SymNum,NULL,CurTAG,CurUDI,-1,-1,-1,0,0,0,0,TRUE);
				GSSiGlobFree (&hExpandedPoints);
			    CloseMap(TRUE);  
				AddSymToMap (NumSyms,hSymDesc,0,NULL); 
	            DestroySymList (&NumSyms,&hSymDesc); 
			    SelectVisList (FALSE);
				if (!GetVisibility(SymNum))
					ToggleVisibility (SymNum);
				GSSiSetCursor (hcurSave);    
	             
                if (PickByRefno(Refno,NULL,NULL,PickFile))
                {
			 		SetViewport (*pCommandViewport);
			 		if (SendDlgItemMessage (hWndDlg,IDC_SHOWNODES,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
			 			ShowNodesRef = PickList[0].Refno;
			 		else
			 			ShowNodesRef=LONG_MAX; 
			 		ClearSpecial ();
			 		CreateSpecial ();
					AddSpecial (PickList[0].Refno,2,RGB(255,255,0),3);
                	SetWindowText (hWndDlg,"Traverse Entry");
		    		ZoomToPickedItem(0,0,TRUE,TRUE,FALSE); 
		    	} 
		    	else
					ZoomToRect(TravBounds,FALSE);
	 			EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_CREATEBOUNDARYLINES),TRUE);  
//				if (TravDest != DEST_TEST && MinimizeWindowIfOverMain (hWndDlg))
//		    		RedisplayWindow ();
			    SetFocus (hWndDlg);
	            
			}
		 	     break; 
		 	case IDC_TRAVDEST:
		 	{   
                switch (HIWORD(lParam))
                {      
		            case CBN_SELCHANGE:  
                     	choice=(short)SendDlgItemMessage(hWndDlg,IDC_TRAVDEST, CB_GETCURSEL,0,0);
                     	SendDlgItemMessage(hWndDlg ,IDC_TRAVDEST,CB_GETLBTEXT,choice,(LPARAM)str); 
				 		if (!_fstricmp (str,"Test Mode"))
				 			TravDest = DEST_TEST;
				 		else if (!_fstricmp (str,"Update Layer"))
				 			TravDest = DEST_UPDATE;
				 		else if (!_fstricmp (str,"Working Layer"))
				 			TravDest = DEST_WORKING;  
						PostMessage(hWndDlg, WM_COMMAND, IDC_TESTMODE, 0L);
				 	default:
				 		break;
				 }
		 	}
		 		break;
		 	
		 	case IDC_TESTMODE:
		 	{
//		 		short	icfg=1; 
			    HaltMapDisplay (FALSE);	
//		 		SetViewport (*pCommandViewport);
//		 		RezoomRect = CurView->WBounds;
//				if (SendDlgItemMessage (hWndDlg,IDC_TESTMODE,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)) 
//			        icfg = 2;
		        SendDlgItemMessage (hWndDlg,IDC_TRAVDEST,CB_RESETCONTENT,0,0);
//		        if (TravAllowUpdateLayer (CurTAG))
		        SendDlgItemMessage (hWndDlg,IDC_TRAVDEST,CB_ADDSTRING,0,(LPARAM)"Update Layer");
//		        else if (TravDest == DEST_UPDATE)
//		        	TravDest = DEST_TEST; 
		        SendDlgItemMessage (hWndDlg,IDC_TRAVDEST,CB_ADDSTRING,0,(LPARAM)"Test Mode");
		        SendDlgItemMessage (hWndDlg,IDC_TRAVDEST,CB_ADDSTRING,0,(LPARAM)"Working Layer");
			 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_GENERATE),FALSE);
			 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_SNAPDIST),FALSE);
			 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_SNAPDISTTITLE),FALSE);
			 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_SNAPDISTTITLE2),FALSE);
	 			EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_CREATEBOUNDARYLINES),FALSE);
	 			EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_CREATELINES),TRUE);
		        if (TravDest == DEST_UPDATE)
		        {
	   			    TravGetDefaultSymbol (CurTAG,SymName,1); 
		        	SendDlgItemMessage (hWndDlg,IDC_TRAVDEST,CB_SELECTSTRING,-1,(LPARAM)"Update Layer");
			 		if (HaveClosure && _fstricmp (SymName,"NONE")) 
			 		{
			 			EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_GENERATE),TRUE);
					 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_SNAPDIST),TRUE);
					 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_SNAPDISTTITLE),TRUE);
					 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_SNAPDISTTITLE2),TRUE);
			 		}
	   			    MustClose = TravGetDefaultSymbol (CurTAG,SymName,2); 
			 		if ((HaveClosure || !MustClose) && _fstricmp (SymName,"NONE"))
			 			EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_CREATEBOUNDARYLINES),TRUE);
		 			EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_CREATELINES),FALSE);
		        }
		        else if (TravDest == DEST_WORKING)  
		        	SendDlgItemMessage (hWndDlg,IDC_TRAVDEST,CB_SELECTSTRING,-1,(LPARAM)"Working Layer");
		        else
		        	SendDlgItemMessage (hWndDlg,IDC_TRAVDEST,CB_SELECTSTRING,-1,(LPARAM)"Test Mode");
			    sprintf (str,"[C]=$MACRO([%%DL]macros\\travmode.txt,%i)",TravDest); 
			    ExpandText (str);    
//		        PostMessage(hWndMain, WM_COMMAND, IDM_Z_REZOOM, 0L);
		    }  
		 		break;
			case IDC_CREATEBOUNDARYLINES:
		 	case IDC_CREATELINES:
		 	{    
		 		short	SymNum=0, NumSyms=0,PickFile, iLeg; 
		 		HANDLE	hSymDesc=0; 
		 		long	Refno;
		 		MNMXCORD	Rect;
				LPLEGDATA	pLegData; 
				short	LineType=2;
							 
		    	if (CurTravID)
		    	 	SaveTrav (CurTravID,hWndDlg,IDC_TRAVLIST);  
				TravCompute (hWndDlg);
		 		ClearHighlightList (TRUE);
		 		if (!nLegs)
		 			break; 
			/*	if (TravDest == DEST_TEST) 
			    {
			    	sprintf (str,"[C]=$MACRO([%%DL]macros\\travmode.txt,%i)",2); 
			    	ExpandText (str);
			    }*/
				if (!CurrentConfig)
					SetConfig (1);
		 		SetViewport (*pCommandViewport);
		   		if (!CurView->UpdateFile)
		   		{
			 		MessageBox(GetFocus(), "No update file in this viewport", NULL,MB_ICONEXCLAMATION|MB_OK);
		            break;
		   		} 
		   		else
		   			_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
				CloseTRANS2 (&hTravTran);
                if (TravDest != DEST_UPDATE) 
                {
                	LineType = 3;
				    sprintf (str,"[C]=$MACRO([%%DL]macros\\travmode.txt,%i)",TravDest); 
				    ExpandText (str);
                }
                else
                {     
					if (GetGlobalCVal ("[%TRAVTRANFILE]",str,NULL))
					{
                		_fstrcat (str,"(F,3)");
						hTravTran = LoadTranFileWithDandT (str);
                    }
                }
   			    TravGetDefaultSymbol (CurTAG,SymName,LineType); 
   		    	SetGlobalValue ("%TRAVSYMBOL",SymName);
/*			    lsymopt =(short)SendDlgItemMessage(hWndDlg,IDC_SYMBOLLINE,CB_GETCURSEL,0,0); 
			    if (lsymopt>=0)
			    {
				    GetDlgItemText (hWndDlg,IDC_SYMBOLLINE,SymName,34); */
	                SymNum = GetOrCreateSym (SymName,&NumSyms,&hSymDesc,FALSE,2);
	                if (!SymNum)
	                { 
	                	MessageBox(GetFocus(),"Symbol not found", SymName,MB_ICONEXCLAMATION|MB_OK);
	                	break;
	                } 
/*	            }
	            else  
		   		{
			 		MessageBox(GetFocus(), "Symbol not set", NULL,MB_ICONEXCLAMATION|MB_OK);
		            break;
		   		} */
   	            DBoundsInit (&Rect);
//				AddToSymList (SymNum,&NumSyms,&hSymDesc); 
	    		_fstrcpy (PltName,EditName);
	    		PltType = 2;
				OpenMap (CurView->hWnd,CurView->hDC);
				EditBounds = CurView->FileMNMX; 
				CloseMap (FALSE); 
				pLegData = (LPLEGDATA)GlobalLock (hLegData); 
				for (iLeg = 0;iLeg < nLegs; iLeg++,pLegData++)
				{   
					if (pLegData->Type < 4)
					{
						WORD	nPnts=pLegData->Type; 
						short	Type = nPnts,i;
						HANDLE	hPnts=GSSiGlobAlloc (1084,GMEM_MOVEABLE,1024);
						LPDPOINT	pPoints = (HPDPOINT)GlobalLock (hPnts);
						
						for (i=0;i<nPnts;i++)
						{
							pPoints[i] = TranPoint (&pLegData->Points[i],hTravTran);
							AddDPointToMinMax (&pPoints[i],&Rect);
                        }
						GlobalUnlock (hPnts); 
						if (Type == 2)
							Type = 1; 
						if (TravDest == DEST_TEST) 
							Refno = iLeg + 1;
						else
							Refno = GetNewRefno (EditName,NULL,NULL,NULL,NULL);
						AddPolyToMap (1,&nPnts,&hPnts,Type,Refno,NULL,-1,SymNum,NULL,NULL,NULL,-1,-1,-1,0,0,0,0,TRUE);
						GSSiGlobFree (&hPnts);
					}
					else if (pLegData->Type == 31)
					{   
						if (TravDest == DEST_UPDATE)
							break;
		   			    TravGetDefaultSymbol (CurTAG,SymName,3); 
		                SymNum = GetOrCreateSym (SymName,&NumSyms,&hSymDesc,FALSE,2);
		                if (!SymNum)
		                { 
		                	MessageBox(GetFocus(),"Symbol not found", SymName,MB_ICONEXCLAMATION|MB_OK);
		                	break;
		                } 
					}
				}
				CloseTRANS2 (&hTravTran);
			    CloseMap(TRUE); 
			    GlobalUnlock (hLegData); 
				AddSymToMap (NumSyms,hSymDesc,0,NULL); 
	            DestroySymList (&NumSyms,&hSymDesc);  
			    CurView->CurZoomAreaRef = 0;
				SelectVisList (FALSE);
				if (!GetVisibility(SymNum))
					ToggleVisibility (SymNum);
				ZoomToRect(Rect,FALSE);   
				if (TravDest == DEST_TEST)
				{
	                choice=SendDlgItemMessage(hWndDlg,IDC_TRAVLIST, LB_GETCURSEL,NULL,NULL);
             		TravShowSelectedLeg (hWndDlg,IDC_TRAVLIST,IDC_CALLNO,choice);
				}
				SetFocus (GetDlgItem(hWndDlg,IDOK));
//				else if	(TravDest == DEST_UPDATE && MinimizeWindowIfOverMain (hWndDlg))
//		    		RedisplayWindow ();
	        }    
		 	break;  
		 	     
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
		         PostMessage(hWndDlg, WM_CLOSE, 0, 0L);
                 break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} 

void RemoveTravTranFile (void)
{   
	char	File[256];
	
	if (!GetGlobalCVal ("[%TRAVTRANFILE]",File,NULL)) 
		return;
	GSSiRemove (File);
	return;
}

BOOL SaveTravTransformation (HANDLE hPoints,HANDLE hSnapStatus,long nPoints)
{
	HPDPOINT	SnapCoord, Point;    
	long	i;
	BOOL	HaveAdjust = FALSE;
	char	str[256], File[256];  
	HFILE	Fid;
	
	if (!GetGlobalCVal ("[%TRAVTRANFILE]",File,NULL))
	{
		GSSiGetTempFileName (0,"gmt",0,File); 
		SetGlobalValue("%TRAVTRANFILE",File);
	}
	Fid = GSSiOpenFile (File,NULL,OF_CREATE);
	Point = (HPDPOINT)GlobalLock (hPoints);
	SnapCoord = (HPDPOINT)GlobalLock (hSnapStatus);
	for (i=0;i<nPoints-1;i++)
	{
		if (SnapCoord[i].x < DBL_MAX)
		{
			sprintf (str,"%Flf %Flf %Flf %Flf",Point[i].x,Point[i].y,SnapCoord[i].x,SnapCoord[i].y);
			HaveAdjust = TRUE;
		}
		else
			sprintf (str,"%Flf %Flf %Flf %Flf",Point[i].x,Point[i].y,Point[i].x,Point[i].y); 
		fputstring (str,Fid);
	} 
	GlobalUnlock (hPoints);
	GlobalUnlock (hSnapStatus);
	GSSiClose (Fid);
	if (!HaveAdjust)
		GSSiRemove (File);    
	return HaveAdjust;
}

HANDLE SnapTravPoints (HANDLE hPoints,HANDLE hSnapStatus,long nPoints,double SnapDist,BOOL ForceClose)
{   
	//Snaps traverse points to existing linework. Uses snapped points to transformed un-snapped
	// points.
	
	HPDPOINT	pPoint1,pPoint2; 
	HPDPOINT	pSnapCoord;    
	long	i, NumNotSnapped=0, MaxTran = min (MAXTRANPOINTS,nPoints);
	HANDLE handle=GSSiGlobAlloc (1085,GHND,nPoints*(long)sizeof(DPOINT));   
	HANDLE	hTran=0, hTemp;
	short	SaveMaxPick=MaxPick, nTran=0;
	float	RSQMIN;	    
	char	PickFile[128];
	static	char	SavePickFile[128]="";
    LPDOUBLE  XT1, YT1, XT2, YT2; 
    LPVISLIST	pSaveVis=CurVis;
    
    if (!*SavePickFile)
		 GSSiGetTempFileName (0,"gmb",0,SavePickFile);
    SaveVisFile (SavePickFile,TRUE,"");
    GetGlobalCVal ("[%SNAPPICKLIST]",PickFile,NULL);
    if (*PickFile)
    	LoadPickList (PickFile);
    hTemp = GSSiGlobAlloc (1086,GMEM_MOVEABLE,MaxTran*16*2);
    XT1 = (LPDOUBLE)GlobalLock (hTemp);
    XT2 = XT1 + MaxTran;
    YT1 = XT2 + MaxTran;
    YT2 = YT1 + MaxTran;
	pPoint1 = (HPDPOINT)GlobalLock (hPoints);
	pPoint2 = (HPDPOINT)GlobalLock (handle);  
	pSnapCoord = (HPDPOINT)GlobalLock (hSnapStatus);    
	UseUserPickAp =FALSE;
	SystemPickAp = -SnapDist;	
	MaxPick=1;
	SetPickAp(0);  

	for (i=0;i<nPoints-1;i++,pPoint1++,pPoint2++,pSnapCoord++)
	{             
		NumPicked = 0; 
		if (pSnapCoord->x < DBL_MAX)
		{
			NumPicked = 1;
			PickList[0].PickedPoint = *pSnapCoord; 
		} 
		else 
		{
			short SavePP = PickPerim;

			PickPerim = 1;  
			PickOnlyNodePoints = TRUE;
			if (SnapDist > 0)
				PickItems2 (CurView->hWnd,*pPoint1,FALSE,TRUE,TRUE); 
			PickOnlyNodePoints = FALSE;
			if (!NumPicked && SnapDist > 0)
				PickItems2 (CurView->hWnd,*pPoint1,FALSE,TRUE,TRUE);  
			PickPerim = SavePP; 
			if (!NumPicked)
			{
				NumPicked = 1;
				PickList[0].PickedPoint = *pPoint1; 
			}
		}
		if (NumPicked)
		{
			*pPoint2 =  *pSnapCoord = PickList[0].PickedPoint;
			XT1[nTran] = pPoint1->x;
			YT1[nTran] = pPoint1->y;
			XT2[nTran] = pPoint2->x;
			YT2[nTran] = pPoint2->y;
			if (i+1 < nPoints)
				nTran++;   
		}
		else  
		{
			NumNotSnapped++;
			pPoint2->x = -DBL_MAX;
		}
	}
	if (nTran > 3)
		hTran = STRAN2 (XT1,YT1,XT2,YT2,nTran,&RSQMIN,3,NULL);
	GSSiGlobUlFree (&hTemp);   
	GlobalUnlock (hPoints);
	GlobalUnlock (handle);
	GlobalUnlock (hSnapStatus);  
	if (NumNotSnapped)   
	{
		pPoint1 = (HPDPOINT)GlobalLock (hPoints);
		pPoint2 = (HPDPOINT)GlobalLock (handle);  
		for (i=0;i<nPoints;i++,pPoint1++,pPoint2++)
		{   
//					MessageBox (0,"pastpickap",NULL,MB_OK);
			if (pPoint2->x == -DBL_MAX)
				*pPoint2 = TranPoint (pPoint1,hTran);
	    }
		GlobalUnlock (hPoints);
		GlobalUnlock (handle);
	}
	pPoint1 = (HPDPOINT)GlobalLock (hPoints);
	pPoint2 = (HPDPOINT)GlobalLock (handle);
	if (ForceClose)
		pPoint2[nPoints-1] = pPoint2[0];
	else  
		pPoint2[nPoints-1] = TranPoint (&pPoint1[nPoints-1],hTran);
	GlobalUnlock (hPoints);
	GlobalUnlock (handle);
	CloseTRANS2 (&hTran);
	UseUserPickAp =TRUE;
	MaxPick = SaveMaxPick;  
	CurVis = pSaveVis;
    if (*PickFile)
    	LoadPickList (SavePickFile);
    CurVis = pSaveVis;
	return handle;
}  

BOOL TravCreateDimText (void)
{
	short	SymNum=0, NumSyms=0,PickFile,i; 
	HANDLE	hSymDesc=0; 
	long	Refno;
	char	SymName[66]; 
	BYTE	LastType;  
	LPBYTE	pPointType;
	HPDPOINT	pSnappedPoint, pOrigPoint;  
	DPOINT	Points[2], OrigPoints[2];
//	LPLEGDATA pLegData;
	    
    if (!nTravPoints || !hTravPoints || !hSnappedPoints || !hPointType)
    {
 		MessageBox(GetFocus(), "No current traverse", NULL,MB_ICONEXCLAMATION|MB_OK);
        return FALSE;
	} 
	GetGlobalCVal ("[%LOTDIMSYMBOL]",SymName,"LOTDIMENSION_POINT");
	SymNum = GetDictSymbolNumber (SymName);
	if (!SymNum)
	{
 		MessageBox(GetFocus(), "Lot dimension symbol not found", SymName,MB_ICONEXCLAMATION|MB_OK);
        return FALSE;
	} 
	if (!CurrentConfig)
		SetConfig (1);
	SetViewport (*pCommandViewport);
	if (!CurView->UpdateFile)
	{
 		MessageBox(GetFocus(), "No update file in this viewport", NULL,MB_ICONEXCLAMATION|MB_OK);
        return FALSE;
	} 
	else
		_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
   	            
    if (!ExistFile(EditName))
    	copyfile (EditName,"[%NULLMAP]",FALSE,0,0,0,0,0,0);
	AddToSymList (SymNum,&NumSyms,&hSymDesc); 
	_fstrcpy (PltName,EditName);
	PltType = 2;
	OpenMap (CurView->hWnd,CurView->hDC);
	EditBounds = CurView->FileMNMX; 
	CloseMap (FALSE); 
	pPointType = GlobalLock (hPointType); 
	pSnappedPoint = (HPDPOINT)GlobalLock (hSnappedPoints); 
	pOrigPoint = (HPDPOINT)GlobalLock (hTravPoints); 
	Points[0] = *pSnappedPoint++; 
	OrigPoints[0] = *pOrigPoint++; 
	LastType = *pPointType++;
	for (i=1;i<nTravPoints;i++,pSnappedPoint++,pPointType++,pOrigPoint++)
	{   
		
		Points[1] = *pSnappedPoint;
		OrigPoints[1] = *pOrigPoint; 
		if (!LastType && !*pPointType)
		{   
			DPOINT	DPoint; 
			double	AZ,d; 
			double	SymSize=20;
			HANDLE		hGRText; 
			LPGRTEXT	lpGRText; 
			char	txt[32];
			
			d = ldistp (OrigPoints[0],OrigPoints[1]);  
			d = CvtDist(d,(short)PRJ_UNITS[1],1);
			RWRITE (d, 2, txt); 
			_fstrcat (txt,"'");
			DPoint = MidPointD (Points[0],Points[1]);
			AZ = getazd (&Points[0],&Points[1]);

            hGRText = GSSiGlobAlloc (1087,GHND,sizeof(GRTEXT));
            lpGRText = (LPGRTEXT)GlobalLock (hGRText);   
            lpGRText->UltiMapStyle = 1;  
            lpGRText->version = 1;    
            lpGRText->length = sizeof(GRTEXT);   
            lpGRText->ltext = _fstrlen (txt);
            lpGRText->ltext += lpGRText->ltext % 2;
            TSize = 6.5;  
            sprintf (lpGRText->cHeight,"%f",CvtDist(TSize,(short)PRJ_UNITS[3],(short)PRJ_UNITS[1])); 
        	lpGRText->FontNum = 0;  
        	lpGRText->hJust = 0;
            lpGRText->vJust = 3; 
        	lpGRText->FlipForEasyReading = 1;
            _fstrcpy (lpGRText->Text,txt);
            GlobalUnlock (hGRText);
            
            if (d > 10)
            {
				Refno = GetNewRefno (EditName,NULL,NULL,NULL,NULL);
	           	AddPointToMap (DPoint,Refno,0,SymNum,SymSize,AZ,NULL,hGRText,0,NULL,NULL,-1,-1,-1,FALSE,HiPrecis,NULL,NULL); 
	        }
           	GSSiGlobFree (&hGRText);
//			AddPolyToMap (1,&nPoints, &hPoints,0,Refno,NULL,-1,SymNum,NULL,NULL,NULL,-1,-1,-1,0,0,0,0,TRUE); 
		}
		Points[0] = Points[1];
		OrigPoints[0] = OrigPoints[1];
		LastType = *pPointType;
	} 
	GlobalUnlock (hSnappedPoints);
	GlobalUnlock (hPointType);  
	GlobalUnlock (hTravPoints);
    CloseMap(TRUE);  
	AddSymToMap (NumSyms,hSymDesc,0,NULL); 
    DestroySymList (&NumSyms,&hSymDesc);  
	return TRUE;
}

HANDLE ExpandTravPoints (HANDLE hPoints,HANDLE hPointTypes,long nPoints,LPWORD pnExpandedPoints)
{
	HANDLE handle=GSSiGlobAlloc (1088,GMEM_MOVEABLE,(long)UINT_MAX*(long)sizeof(DPOINT));
	long	nNewPoints=0;
	HPDPOINT	pPoint1= (HPDPOINT)GlobalLock (hPoints);
	HPDPOINT	pPoint2= (HPDPOINT)GlobalLock (handle);  
	LPBYTE	pType=GlobalLock (hPointTypes); 
	BOOL 	HavePOC=FALSE;   
	DPOINT	PC, POC, PT; 
	double	BackAZ;
	
	nNewPoints = 0; 
	while (nPoints--) 
	{
		if (*pType)
		{
			HavePOC = TRUE;  
			POC = *pPoint1++;
		}
		else if (HavePOC)
		{
		    PT = *pPoint1++;
            CurvePointsD(&PC,&POC,&PT, &nNewPoints, &pPoint2,&BackAZ,UINT_MAX-4-nNewPoints-nPoints,CurveChordDist,1);
            HavePOC = FALSE;  
            PC = PT;
	    }
	    else   
	    {
			PC = *pPoint2++ = *pPoint1++;  
			nNewPoints++;
		} 
		pType++;
	}
	GlobalUnlock (hPoints);
	GlobalUnlock (handle);   
	GlobalUnlock (hPointTypes);          
	*pnExpandedPoints = nNewPoints;
	return handle; 
}
  
BOOL AdjustTravPoint (HWND hWnd, WORD Message, WORD wParam, LONG lParam,short Function)
{
 char key;
 static	HBITMAP	hShowPoint=NULL;  
 static long	mini;
 short	rtn=1, iSym; 
 static	BOOL	HavePoint, DoTrack=TRUE; 
 static	PICKDATA	PickData;
 static	POINT	TrackPoints[5], LastPoint;  
 POINT	MousePoint;
 DPOINT	BasePoint;    
 HPDPOINT pStatus;
 
 switch (Message)
   {
   	case GF_INIT:     
   		if (!hWndTraverseEntry ||!hSnapStatus || !hSnappedPoints)
   			return FALSE;
       	AddLBUTTON = FALSE; 
       	HavePoint = FALSE;    
   		SetPrompt (PRMT_TRAVADJUST1,TRUE);  
   		break;

   	case GF_REDRAW:
    case GF_CLOSE: 
		DestroySavedScreen (&hShowPoint,0);
    	return FALSE;
    	
    case WM_RBUTTONUP:
    {    
    	MousePoint = MAKEPOINT(lParam); 
	    if (CursorIsLocked)
	    { 
	    	BasePoint = CurrentPoint;
	    	UnlockCursor ();
			MousePoint = BasePtToWinPt (&BasePoint);
	    } 
	    else
        	BasePoint=WinPtToBasePt (MousePoint); 
    	EnlargeScreen (0,0);
   		if (!hWndTraverseEntry ||!hSnapStatus) 
   		{
		    PostMessage(hWnd, GF_CLOSE, 0, 0L); 
			break;
		}
	    pStatus = (HPDPOINT)GlobalLock (hSnapStatus);
	    pStatus[mini] = BasePoint;
	    GlobalUnlock (hSnapStatus);	
        PostMessage(hWndTraverseEntry, WM_COMMAND, IDC_GENERATE, 1L);
		PostMessage(hWnd, GF_CLOSE,0, 0L);
   		SetPrompt (PRMT_TRAVADJUST1,TRUE);  
		HavePoint = FALSE;
    }
    	break;
    	
    case WM_LBUTTONUP:
    {   
    	if (HavePoint)
    		HavePoint = FALSE; 
    	if (!hShowPoint)
    		break;
    	HavePoint = TRUE;   
   		SetPrompt (PRMT_TRAVADJUST2,TRUE);  

    }
    	break;

    case WM_MOUSEMOVE: 
    {
		long	i; 
		DPOINT	BasePoint;
		HPDPOINT	DPoint;
		double	MinDist,d; 
		POINT	WinPoint;
        
   		if (!hWndTraverseEntry ||!hSnapStatus) 
   		{
		    PostMessage(hWnd, GF_CLOSE, 0, 0L); 
			break;
		}
        if (HavePoint)
        	break;
        if (hShowPoint)
        {
	   		HDC	hDC=GetDC(hWnd);
			RestoreScreen2 (hDC, hShowPoint,0,FALSE);
			ReleaseDC (hWnd,hDC); 
			DestroySavedScreen (&hShowPoint,0);
		}
		if (!hSnappedPoints)
			break;
    	MousePoint = MAKEPOINT(lParam); 
	    BasePoint=WinPtToBasePt(MousePoint); 
	    DPoint = (HPDPOINT)GlobalLock (hSnappedPoints);  
	    MinDist = ldistp (BasePoint,DPoint[0]);
	    mini = 0;
	    for (i=1;i<nTravPoints;i++)
	    {
	    	d = ldistp (BasePoint,DPoint[i]);
	    	if (d<MinDist)
	    	{
	    		MinDist=d;
	    		mini = i;
	    	}
	    }  
	    WinPoint = BasePtToWinPt (&DPoint[mini]);
	    iSym = GetDictSymbolNumber ("CIRCLE");
		hShowPoint = DisplayPointItem2 (CurView->hDC,WinPoint,5,0,iSym) ;
	    GlobalUnlock (hSnappedPoints);
    }
    	break;  
    default:
    	return FALSE;
    	
    }
    return (TRUE);
} 

BOOL IdentifyTraverseLeg (HWND hWnd, WORD Message, WORD wParam, LONG lParam,short Function)
{    
	char	str[128];
	
 switch (Message)
   {
   	case GF_INIT:
   		if (!hWndTraverseEntry ||!hSnapStatus)
   			return FALSE; 
		if (TravDest != DEST_TEST)
		{
			GSSiMessageBox ("Traverse entry must be in test mode to use this function",NULL,MB_OK);
			return FALSE;
		} 
       	AddLBUTTON = TRUE; 
       	return GF_READY_TO_PROCESS;
   		break;

    case WM_LBUTTONUP:
    {
		short	Item; 
		POINT	MousePoint, FilePoint;
		DPOINT	BasePoint;
    	short	ID;  
    	HFILE	Fid; 
    	OFSTRUCT	OFStruct;

 		if (CursorIsLocked)  
 		{
 		 	BasePoint = CurrentPoint;
	    	UnlockCursor ();
	    }
	    else
	    { 
	    	MousePoint = MAKEPOINT(lParam);
		    BasePoint=WinPtToBasePt(MousePoint);
		}
		EnlargeScreen (0,0); 
	    PickItems (hWnd,BasePoint);
	    if (!NumPicked)
	    	break;  
	    Item = NumPicked-1;
		SendDlgItemMessage (hWndTraverseEntry,IDC_TRAVLIST,LB_SETCURSEL,(WPARAM)(PickList[Item].Refno+POBItem),0);
	    ltoa (PickList[Item].Refno,str,10);
	    SetDlgItemText (hWndTraverseEntry,IDC_CALLNO,str);
 		EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_REMOVELEG),TRUE);
 		EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_EDLEG),TRUE); 
		SetFocus (GetDlgItem(hWndTraverseEntry,IDOK));
		ShowWindow (hWndTraverseEntry,SW_RESTORE);
    }
    	break;
    default:
    	return (FALSE);
    }
    return (TRUE);
} 

