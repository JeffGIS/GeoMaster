#include "graphint.h"

typedef	struct {long	ID;
				char	Prefix[8], UDI[32], SymName[32];
				} TRAVIDDATA;
typedef TRAVIDDATA	FAR	*LPTRAVIDDATA;
typedef	struct {
				short	Type;
				DPOINT	Points[3];
				double	Dist;
				} LEGDATA;
typedef LEGDATA	FAR	*LPLEGDATA; 
typedef	struct {long	ID;
				short	LegNum;
				short	Type;
				long	SnapRef;
				short	SnapPos;
				char	Parameters[250];
				} TRAVLEGDATA;
typedef TRAVLEGDATA	FAR	*LPTRAVLEGDATA; 

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


BOOL DecodeTYPE1CRVParm (LPSTR Parm,LPDOUBLE pRadius,LPDOUBLE pDist,LPDOUBLE pAZ,LPSHORT pDir,LPSHORT pDistOpt,LPSHORT pBearOpt);
BOOL ComputeTYPE1CRV (double Radius,double Dist,double AZ,short Dir,short DistOpt,short BearOpt,DPOINT PC,
					  LPDPOINT pPOC, LPDPOINT pPT,LPDPOINT pRP,
					  LPDOUBLE pArcDist,LPDOUBLE pTanDist,LPDOUBLE pChordDist,
					  LPDOUBLE pTanAZ, LPDOUBLE pRPAZ, LPDOUBLE pCrdAZ, LPDOUBLE pAngle);
BOOL ShowTrav (HWND hWndDlg,UINT IDCTRAVLIST,long CurTravID);
void LoadTravAddLegOptions (HWND hWndDlg,UINT IDCADDLEG,int Opt);
void TravShowSelectedLeg (HWND hWndDlg,UINT IDCTRAVLIST,UINT IDCCALLNO,int choice);
void TravGetDefaultSymbol (LPSTR CurTAG,LPSTR SymName,int Which); 
void AdjustSubTraverse (HWND hWndDlg,UINT IDCTRAVLIST,int From,int Increment);

void TravGetDefaultSymbol (LPSTR CurTAG,LPSTR SymName,int Which)
{   
	HFILE	Fid; 
	LPSTR	pComma;
	char	line[260];
			
	_fstrcpy (SymName,"");
	Fid = GSSiOpenFile("[%DL]travsyms.txt",NULL,OF_READ);
	if (Fid != HFILE_ERROR)
	{   
		while (fgetstring (line,256,Fid))
		{
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
	return;
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
			  
			decdeg = Angle * RADIAN;
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
	if (st)
		SaveTrav (CurTravID,hWnd,IDC_TRAVLIST); 	
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
		if (                                                                                                                                                                                                                                               