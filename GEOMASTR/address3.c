#include "graphint.h"
#include "extrndb.h"  
#include "std.h"

#define STORED_STREET_LEN 32
#define STORED_HOUSE_LEN 8
#define STORED_CITY_LEN 32
#define STORED_ZIP_LEN 8 

#define MAX_FLAGS	256    

#define	DT_TEXTCOLOR	1
#define DT_TEXTOUT		2  
#define DT_CREATEFONT	3  
#define DT_REMOVECOLOR	4

#include "gmextern.h"   

static	long	NumNoMatch, NumInvalid, NumMultMatch; 
static	RECT	txtRect;
static	char	DispTextText[4096], DispTextText2[4096]; 
static	HANDLE	hSTDMunic=0;
static	HFONT	hDelayedFont=0;
static	RECT	FlagRects[MAX_FLAGS];
static	int		nFlags;	

int GetGeocodeType(LPSTR Input) //returns 1 if intersection,2 if house-street,3 if all numeric(PID)
{
	char Street1[100], Street2[100], House[32];
	int	IHouse;
	LPSTR	Street;
	int	maxHouseLen = 4;

	if (IsInteger(Input) && strlen(Input) > maxHouseLen)
	{
		return 3;
	}
	else if (SeparateIntStreets(Input, Street1, Street2))
	{
		Truncate(Street1);
		Truncate(Street2);
		IHouse = 0;
		Street = Street1;
		if (*Street1 && *Street2)
			return 1;
	}
	else
	{
		Street = GetHouseAndStreet(Input, House);
		IHouse = atol(House);
		if (IHouse && *Street)
			return 2;
	}
	return 0;
}

BOOL FAR PASCAL GeocodeAlltypesMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{

	RECT	rect, crect, rect2;
	int		w, h;
	int		nRc;
	int		TabStops[2] = { 1000, 2000 };
	static HWND hSubWnd = 0;
	static int geocodeType = 1;
	static	char	input[256] = { 0 };
	char	txt[256];
	static	int fullHeight, fullWidth,collapsedHeight=56, collapsedWidth, border;
	static	RECT	collapsedCancelRect, expandedCancelRect;
	static	BOOL	collapsed = TRUE;
	static	int		currentGeocodeType = 0;

		int	BRtn;
		if ((BRtn = DIALOGSTYLEMsgProc(hWndDlg, Message, wParam, lParam)))
		{
			return (BRtn);
		}
		switch (Message)
		{
		case WM_INITDIALOG:
			hSubWnd = 0;
			SendDlgItemMessage(hWndDlg, IDC_MATCHLIST, LB_SETTABSTOPS, 2, (LPARAM)&TabStops);

			SendDlgItemMessage(hWndDlg, IDC_GEOCODE_TYPE, LB_ADDSTRING, 0, (LPARAM)"Intersection");
			SendDlgItemMessage(hWndDlg, IDC_GEOCODE_TYPE, LB_ADDSTRING, 0, (LPARAM)"House - Street Name");
			SendDlgItemMessage(hWndDlg, IDC_GEOCODE_TYPE, LB_ADDSTRING, 0, (LPARAM) "Parcel ID");
			SendDlgItemMessage(hWndDlg, IDC_GEOCODE_TYPE, LB_ADDSTRING, 0, (LPARAM)"Taxpayer Name");
			SendDlgItemMessage(hWndDlg, IDC_GEOCODE_TYPE, LB_ADDSTRING, 0, (LPARAM)"Common Name");
			SendDlgItemMessage(hWndDlg, IDC_GEOCODE_ACTION, LB_ADDSTRING, 0, (LPARAM) "Zoom to location at 200 scale");
			SendDlgItemMessage(hWndDlg, IDC_GEOCODE_ACTION, LB_ADDSTRING, 0, (LPARAM) "Place Pin at location");
			SendDlgItemMessage(hWndDlg, IDC_GEOCODE_ACTION, LB_ADDSTRING, 0, (LPARAM) "Create StreetView insert");
			SendDlgItemMessage(hWndDlg, IDC_GEOCODE_TYPE, LB_SETCURSEL, geocodeType - 1, 0);
			SendDlgItemMessage(hWndDlg, IDC_GEOCODE_ACTION, LB_SETSEL, TRUE, 0);

			//cwCenter(hWndDlg, 0);
			//SetDlgItemText(hWndDlg, IDC_WAITMESS, GMmess);
			//$TAGLOC(PINCA, 5, LASTPID, Locate Parcel, Primary Viewport)
			strcpy(TagLocPrefix, "PINCA");

			GetWindowRect(GetDlgItem(hWndDlg, IDOK), &rect);
			GetWindowRect(hWndDlg, &rect);
			GetClientRect(hWndDlg, &crect);
			border = max (4,(RECTWIDTH(&rect) - RECTWIDTH(&crect)) / 2);
			fullWidth = RECTWIDTH(&rect);
			fullHeight = RECTHEIGHT(&rect);
			GetWindowRect(GetDlgItem(hWndDlg, IDCANCEL), &rect2);
			expandedCancelRect = rect2;
			ScreenRectToClientRect(hWndDlg, &expandedCancelRect);
			w = RECTWIDTH(&rect2);
			h = RECTHEIGHT(&rect2);
			GetWindowRect(GetDlgItem(hWndDlg, IDOK), &rect2);
			ScreenRectToClientRect(hWndDlg, &rect2);
			collapsedWidth = rect2.right + w + border*2;
			MoveWindow(hWndDlg, rect.left, rect.top, collapsedWidth, collapsedHeight, TRUE);
			MoveWindow(GetDlgItem(hWndDlg, IDCANCEL), rect2.right + border, rect2.top, w, h,TRUE);
			GetWindowRect(GetDlgItem(hWndDlg, IDCANCEL), &collapsedCancelRect);
			ScreenRectToClientRect(hWndDlg, &collapsedCancelRect);

			//PostMessage(hWndDlg, WM_NEXTDLGCTL, IDC_INPUT, TRUE);
			SetFocus(GetDlgItem(hWndDlg, IDC_INPUT));
			return FALSE;
			break; /* End of WM_INITDIALOG                                 */

		case WM_TIMER:
			KillTimer(hWndDlg, 1);
//			if (!collapsed)
//				PostMessage(hWndDlg, WM_COMMAND, IDOK, 0);
			{
				HDC hDC = GetDC(GetDlgItem(hWndDlg, IDC_GEOCODE_MAP));
				RECT	rect;
				int		igray = 240;

				GetClientRect(GetDlgItem(hWndDlg, IDC_GEOCODE_MAP), &rect);
				FillRectPoly(hDC, &rect, RGB(igray, igray, igray));
				DisplayBMFileInRect(hDC, "C:\\Users\\jeffrey\\Pictures\\Untitled.bmp", rect, TRUE);
				ReleaseDC(GetDlgItem(hWndDlg, IDC_GEOCODE_MAP), hDC);
			}
			break;

		case WM_CLOSE:
			/* Closing the Dialog behaves the same as Cancel               */
			PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
			break; /* End of WM_CLOSE                                      */

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_MATCHLIST:
				break;

			case IDC_INPUT:
				switch (HIWORD(wParam))
				{
				case EN_CHANGE:
					if (GetDlgItemText(hWndDlg, IDC_INPUT, input,255))
					{
						int i = GetGeocodeType(input);
						SendDlgItemMessage(hWndDlg, IDC_GEOCODE_TYPE, LB_SETCURSEL, i - 1, 0);
						if (i)
						{
							geocodeType = i;
							GetDlgItemText(hWndDlg, IDC_INPUT, input, 255);
							switch (geocodeType)
							{
							case 1:
								PostMessage(hWndDlg, WM_COMMAND, IDC_LOCINTERSECTION, 0);
								break;
							case 2:
								PostMessage(hWndDlg, WM_COMMAND, IDC_LOCADDRESS, 0);
								break;
							case 3:
								PostMessage(hWndDlg, WM_COMMAND, IDC_LOCPID, 0);
								break;
							}

						}
					}
				}
				break;
			case IDCANCEL:
				if (hSubWnd)
					SendMessage(hSubWnd, WM_COMMAND, IDCANCEL, 0L);
				hSubWnd = 0;
				EndDialog(hWndDlg, FALSE);
				break;
			case IDOK:
				GetDlgItemText(hWndDlg, IDOK, txt, 4);
				if (*txt == 'V')
				{
					collapsed = FALSE;
					SetDlgItemText(hWndDlg, IDOK, "^");
					GetWindowRect(hWndDlg, &rect);
					MoveWindow(hWndDlg, rect.left, rect.top, fullWidth, fullHeight, TRUE);
					MoveWindow(GetDlgItem(hWndDlg, IDCANCEL), expandedCancelRect.left, expandedCancelRect.top, RECTWIDTH(&expandedCancelRect), RECTHEIGHT(&expandedCancelRect), TRUE);
					ShowWindow(GetDlgItem(hWndDlg, IDC_GEOCODE_TYPE), SW_SHOW);
					ShowWindow(GetDlgItem(hWndDlg, IDC_GEOCODE_ACTION), SW_SHOW);
					ShowWindow(GetDlgItem(hWndDlg, IDC_GEOCODE_MAP), SW_SHOW);
					ShowWindow(GetDlgItem(hWndDlg, IDC_GEOCODE_FROMFILE), SW_SHOW);

					switch (geocodeType)
					{
					case 1:
						PostMessage(hWndDlg, WM_COMMAND, IDC_LOCINTERSECTION, 0);
						break;
					case 2:
						PostMessage(hWndDlg, WM_COMMAND, IDC_LOCADDRESS, 0);
						break;
					}
					SetTimer(hWndDlg, 1, 100, (TIMERPROC)0);
				}
				else
				{
					collapsed = TRUE;
					if (hSubWnd)
						SendMessage(hSubWnd, WM_COMMAND, IDCANCEL, 0L);
					hSubWnd = 0;
					SetDlgItemText(hWndDlg, IDOK, "V");
					GetWindowRect(hWndDlg, &rect);
					MoveWindow(hWndDlg, rect.left, rect.top, collapsedWidth, collapsedHeight, TRUE);
					MoveWindow(GetDlgItem(hWndDlg, IDCANCEL), collapsedCancelRect.left, collapsedCancelRect.top, RECTWIDTH(&collapsedCancelRect), RECTHEIGHT(&collapsedCancelRect), TRUE);
					ShowWindow(GetDlgItem(hWndDlg, IDC_GEOCODE_TYPE), SW_HIDE);
					ShowWindow(GetDlgItem(hWndDlg, IDC_GEOCODE_ACTION), SW_HIDE);
					ShowWindow(GetDlgItem(hWndDlg, IDC_GEOCODE_MAP), SW_HIDE);
					ShowWindow(GetDlgItem(hWndDlg, IDC_GEOCODE_FROMFILE), SW_HIDE);
				}
				break;
			case IDC_LOCINTERSECTION:
			{
				char street1[100], street2[100];
				//nRc = DialogBox(hInst, (LPSTR)"LOC_INTERSECT1", hWndDlg, LOC_INTERSECTMsgProc);
				GetDlgItemText(hWndDlg, IDC_INPUT, input, 255);
				SeparateIntStreets(input, street1, street2);
				if (!collapsed || (*street1 && *street2))
				{
					if (currentGeocodeType != geocodeType)
					{
						currentGeocodeType = geocodeType;
						if (hSubWnd)
						{
							SendMessage(hSubWnd, WM_COMMAND, IDCANCEL, 0L);
							hSubWnd = 0;
						}
					}
					if (!hSubWnd)
					{
						hSubWnd = CreateDialog(hInst, (LPSTR)"LOC_INTERSECT1", hWndDlg,(DLGPROC) LOC_INTERSECTMsgProc);
						SetSecondaryIntInput(GetDlgItem(hWndDlg, IDC_INPUT));
					}
					GetDlgItemText(hWndDlg, IDOK, txt, 4);
					if (*txt == 'V')
						SetIntMatchControl(hWndDlg, IDC_MATCHLIST);
					else
					{
						SetIntMatchControl(hSubWnd, 0);
						ShowWindow(GetDlgItem(hWndDlg, IDC_MATCHLIST), SW_HIDE);
						ShowWindow(hSubWnd,SW_SHOW);
					}

					//SetFocus(GetDlgItem(hSubWnd, IDC_STREET2));
					SetFocus(GetDlgItem(hWndDlg, IDC_INPUT));
					SetDlgItemText(hSubWnd, IDC_STREET1, street1);
					SetDlgItemText(hSubWnd, IDC_STREET2, street2);
				}

			}
				break;
			case IDC_LOCADDRESS:
			{
					LPSTR Street;
					char House[32];

					if (currentGeocodeType != geocodeType)
					{
						currentGeocodeType = geocodeType;
						if (hSubWnd)
						{
							SendMessage(hSubWnd, WM_COMMAND, IDCANCEL, 0L);
							hSubWnd = 0;
						}
					}
					if (!hSubWnd)
					{
						hSubWnd = CreateDialog(hInst, (LPSTR)"ADDRESS3", hWndDlg, (DLGPROC)ADDRESSPIDMsgProc);
						SetSecondaryAddInput(GetDlgItem(hWndDlg, IDC_INPUT));
					}
					GetDlgItemText(hWndDlg, IDOK, txt, 4);
					if (*txt == 'V')
						SetAddMatchControl(hWndDlg, IDC_MATCHLIST);
					else
					{
						SetAddMatchControl(hSubWnd, 0);
						ShowWindow(GetDlgItem(hWndDlg, IDC_MATCHLIST), SW_HIDE);
						ShowWindow(hSubWnd, SW_SHOW);
					}
					SetFocus(GetDlgItem(hWndDlg, IDC_INPUT));
					GetDlgItemText(hWndDlg, IDC_INPUT, input, 255);
					Street = GetHouseAndStreet(input, House);

					SetDlgItemText(hSubWnd, IDM_HOUSE, House);
					SetDlgItemText(hSubWnd, IDM_STREET, Street);
			}
				break;

			case IDC_LOCPID:
			{
					if (!hSubWnd)
					{
						hSubWnd = CreateDialog(hInst, (LPSTR)"TAGLOC1", hWndDlg, (DLGPROC)TAGLOCMsgProc);
						SetSecondaryTAGInput(GetDlgItem(hWndDlg, IDC_INPUT));
					}
					SetFocus(GetDlgItem(hWndDlg, IDC_INPUT));

					SetDlgItemText(hSubWnd, IDC_TAGVALUE,input);
			}
				break;

			}
			break;    /* End of WM_COMMAND                                 */

		default:
			return FALSE;
		}
			return TRUE;
}

BOOL GeocodeAlltypes(HWND hWnd,LPSTR OutLoc, LPSTR Arg1)
{
	int nRc = DialogBox(hInst, (LPSTR)"GEOCODE_ALLTYPES", hWnd, (DLGPROC)GeocodeAlltypesMsgProc);

	return TRUE;
}

BOOL ReverseGeocodeCommand (int nArgs,LPSTR *Arg,LPSTR OutLoc)
{
	short	SaveMaxPick = MaxPick; 
	LPVISLIST	SaveVis = CurVis; 
	int		SaveVP = CurView->ID;
	BOOL rtn=FALSE;
	BOOL err;
	DPOINT DPoint = atopt (Arg[1],&err);
	int		formatID = atoi (Arg[2]);

	*OutLoc = 0;
	if (err)
		return FALSE;
	SetViewport(*pCommandViewport);
	MaxPick=1;  
	SaveVis = CurVis;
//	UseUserPickAp =FALSE; 
//	SystemPickAp = 0;
//	PickLimit = 5 * MetersPerMile;	
//	LoadPickList ("piklists\\revgeocode.pik"); 
	PickItems (CurView->hWnd,DPoint); 
	if (NumPicked)
	{
		double ActualDist = GetBaseDist (&DPoint,&PickList[0].PickedPoint)*MFT/5280;

		switch (formatID)
		{
			case 3://peoplenet
			{ 
				int iAdd1, iAdd2;
				LPSTR pAdd1, pAdd2;

			    CurState = PickedStreets[0][4];
				OpenGSStreetNames (BT_READ,1);
				ProcessText ("$OPEN(PNDATA=%.pnd,REFNO=@[%PICKED_REFNO]);$FETCH(PNDATA)");

				if (PickList[0].OffDist < 0)
					strcpy (OutLoc,"On:[PNDATA.STREETNAME];BetweenStreets:[PNDATA.FROMSTREET] and [PNDATA.TOSTREET];BetweenAddresses:[PNDATA.FROMADDRESSL] and [PNDATA.TOADDRESSL];InZIP:[PNDATA.ZIPL];");
				else
					strcpy (OutLoc,"On:[PNDATA.STREETNAME];BetweenStreets:[PNDATA.FROMSTREET] and [PNDATA.TOSTREET];BetweenAddresses:[PNDATA.FROMADDRESSR] and [PNDATA.TOADDRESSR];InZIP:[PNDATA.ZIPR];");
				ExpandText (OutLoc);
				pAdd1 = strstr(OutLoc, "BetweenAddresses:") +strlen("BetweenAddresses:");
#pragma warning(suppress: 6387)
				pAdd2 = strchr (pAdd1,' ') + 5;
				iAdd1 = atoi (pAdd1);
				iAdd2 = atoi (pAdd2);
				sprintf (strchr (OutLoc,0),"InterpolatedAddress:%i",iAdd1 + IDNINT((iAdd2-iAdd1)*PickList[0].PCT));
				ProcessText ("$CLOSE(PNDATA)");
				CloseGSStreetNames ();
			}
				break;
			default:
				break;
		}
	}

	CurVis = SaveVis;
	MaxPick = SaveMaxPick;   
	PickLimit = DBL_MAX;
	UseUserPickAp = TRUE; 
	SetViewport(SaveVP);
	return rtn;
}

long GetStreetNumFromRawName (LPSTR Street,LPSTR TrueName)
{
	long StreetNum;
	
	STNDSN_INIT(FALSE);
	STNDST(Street, (short)_fstrlen(Street),STDNAMv,NRONAMv,NMONLYv,
	                        SANSCHv,NANDCHv,NCMPNMv,ORIGNMv,SANSCPv,SANSCSv,NULL,NULL,NULL,NULL);
    StreetNum = GetStreetNumFromName (STDNAMv,2,BT_FIRST,TrueName);
	return StreetNum;
}

void OpenMunicFromName (BOOL Close)
{   
	static	BOOL	Opened=FALSE;  
	char	File[256];
	BTVARDESC BTVar[2];  
	ALTMUNICNAME	ALTMunName; 
	short	Dummy,pos=BT_FIRST,cond=BT_ANY;
	long	STDMunic;
	
	if (Close)
	{
		BT_CLOSE (hBTMFN);
		hBTMFN = 0;  
		BT_CLOSEANDDELETE (&hSTDMunic); 
		Opened = FALSE;
		return;
    }
	if (!Opened && !hBTMFN)
	{   
		HANDLE	hBT;
		
		Opened = TRUE;
	    SetAddressDir();
	    sprintf (File,"%s\\munnames.btr",AddMatchDir);
		hBTMFN = BT_OPEN (File,0,BT_READ,0); 
		if (hBTMFN)
		{
			BTVar[0].BT_VARTYP=BT_INTEGER;
			BTVar[0].BT_VARLEN=4;
			BTVar[0].BT_VAROFF=0;
			GSSiGetTempFileName (0,"gm",0,(LPSTR)File); 
			BT_CREATE (File, 4, FALSE, 1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
		    hBT = BT_OPEN (File,0,BT_WRITE,0); 
		    while (!BT_FIND (hBTMFN,(LPSTR)&ALTMunName,BT_FIRST,cond,(LPSTR)&Dummy))
		    {   
		    	cond = BT_GT;
		    	STDMunic = GetMunicFromName (ALTMunName.ALTName);
		    	BT_PUT (hBT,(LPSTR)&ALTMunName.Munic,(LPSTR)&STDMunic);
		    }
		    hSTDMunic = hBT; 
		}
	}	
	return;
}

long StandardizedMunic (long Munic)
{
	char	File[MAX_PATH]; 
	long	STDMunic;
    
    OpenMunicFromName (FALSE);
	if (!hSTDMunic) 
		return Munic;
	if (BT_FIND (hSTDMunic,(LPSTR)&Munic,BT_FIRST,BT_GE,(LPSTR)&STDMunic))
		return Munic;
	return STDMunic;
}

long GetMunicFromName (LPSTR MName)
{  
	ALTMUNICNAME	ALTMunName;
	long	Munic=0;
	char	File[MAX_PATH];  
	short	st, Dummy;
//	MUNICNAME	MunName;
	
	if (!MName)
	{   
		OpenMunicFromName (TRUE);
		return 0;
	}
	OpenMunicFromName (FALSE);
	OneSpace (MName);
	if (!_fstricmp (MName,"MPLS"))
		return 135;
    _fstrncpy (ALTMunName.ALTName,MName,sizeof(ALTMunName.ALTName));   
    _fstrupr (ALTMunName.ALTName);
    ALTMunName.Munic = 0; 
	st = BT_FIND (hBTMFN,(LPSTR)&ALTMunName,BT_FIRST,BT_GE,(LPSTR)&Dummy);
	if (st)
		return 0;
	if (!_fstricmp (MName,ALTMunName.ALTName))
		return StandardizedMunic (ALTMunName.Munic);
	
	return 0;
}  

long DumpMunicNameTable (LPSTR FileName)
{  
	ALTMUNICNAME	ALTMunName;
	long	Munic=0;
	char	str[256],File[256];  
	short	st, Dummy, pos = BT_FIRST;
	MUNICNAME	MunName; 
	HANDLE	hBTMFN;
	long	rtn=0;
	HFILE	Fid=GSSiOpenFile (FileName,NULL,OF_CREATE);
	
	SetAddressDir();
	sprintf (File,"%s\\munnames.btr",AddMatchDir);
	hBTMFN = BT_OPEN (File,0,BT_READ,0);
	while (!BT_FIND (hBTMFN,(LPSTR)&ALTMunName,pos,BT_ANY,(LPSTR)&Dummy))
	{
		pos = BT_NEXT;
		sprintf (str,"%s\t%ld",ALTMunName.ALTName,ALTMunName.Munic);
		fputstring (str,Fid);  
		rtn++;
	}
	GSSiClose2 (&Fid);   
	BT_CLOSE (hBTMFN);
	return rtn;
}

BOOL ReloadMunicNameTable (LPSTR FileName)
{  
	BTVARDESC BTVar[2];  
	ALTMUNICNAME	ALTMunName;
	char	str[256],File[256];  
	LPSTR	pTab;
	HFILE	Fid=GSSiOpenFile (FileName,NULL,OF_READ);   
	HANDLE	hBT; 
	short	Dummy=0;
	
	
	if (Fid == HFILE_ERROR)
		return FALSE;
	SetAddressDir();
	sprintf (File,"%s\\munnames.btr",AddMatchDir);
	BTVar[0].BT_VARTYP=BT_CHAR;
	BTVar[0].BT_VARLEN=64;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=64;
	BT_CREATE (File, 2, FALSE, 2, 1,(LPBTVARDESC)&BTVar,FALSE, 0, 0, FALSE);
	hBT = BT_OPEN (File,0,BT_WRITE,0);
	while (fgetstring (str,128,Fid))
	{
        pTab = _fstrchr (str,'\t');
        if (pTab)
        {
        	*pTab++ = 0;
        	ALTMunName.Munic = atol (pTab);
        	Truncate (str);
        	_fstrupr (str);
	        _fstrncpy (ALTMunName.ALTName,str,sizeof(ALTMunName.ALTName)); 
	    	BT_PUT (hBT,(LPSTR)&ALTMunName,(LPSTR)&Dummy);    
	    }
	}
	GSSiClose2 (&Fid);   
	BT_CLOSE (hBT);
	return TRUE;
}

COLORREF SetTextColorDelayed (HDC hDC,COLORREF OldColor,LPTHEME CurTheme)
{   
	short	Type=DT_TEXTCOLOR;
	COLORREF	LastColor = SetTextColor (hDC,OldColor); 
	
	if (!CurTheme || CurTheme->FidDelayedText == HFILE_ERROR)
		return LastColor;
	BigWrite (CurTheme->FidDelayedText,(HPSTR)&Type,2,-1);
	BigWrite (CurTheme->FidDelayedText,(HPSTR)&OldColor,sizeof(COLORREF),-1);    
	return LastColor;
}

BOOL TextOutDelayed (HDC hDC,int x,int y,LPSTR Text,short ltext,LPTHEME CurTheme)
{   
	short	Type=DT_TEXTOUT; 
	int		BKMode;
	COLORREF	Color;
	
	if (!CurTheme || CurTheme->FidDelayedText == HFILE_ERROR)
		return TextOut (hDC,x,y,Text,ltext);     
	BKMode = GetBkMode (hDC); 
	Color = GetTextColor (hDC);
	BigWrite (CurTheme->FidDelayedText,(HPSTR)&Type,2,-1);
	BigWrite (CurTheme->FidDelayedText,(HPSTR)&BKMode,sizeof(int),-1);
	BigWrite (CurTheme->FidDelayedText,(HPSTR)&Color,sizeof(COLORREF),-1);
	BigWrite (CurTheme->FidDelayedText,(HPSTR)&x,sizeof(int),-1);
	BigWrite (CurTheme->FidDelayedText,(HPSTR)&y,sizeof(int),-1);
	BigWrite (CurTheme->FidDelayedText,(HPSTR)&ltext,sizeof(short),-1);
	BigWrite (CurTheme->FidDelayedText,(HPSTR)Text,ltext,-1);
	return TRUE;
}

HFONT CreateFontIndirectDelayed(LPLOGFONT pLogFont,LPTHEME CurTheme)
{   
	short	Type=DT_CREATEFONT;
	
	pLogFont->lfOrientation = pLogFont->lfEscapement;
	if (!CurTheme || CurTheme->FidDelayedText == HFILE_ERROR)
		return CreateFontIndirect (pLogFont);
	BigWrite (CurTheme->FidDelayedText,(HPSTR)&Type,2,-1);
	BigWrite (CurTheme->FidDelayedText,(HPSTR)pLogFont,sizeof(LOGFONT),-1);
	return CreateFontIndirect (pLogFont);
} 

void RemoveColorFromRectDelayed (HDC hDC,COLORREF RemoveColor,LPRECT pRect,LPTHEME CurTheme) 
{
	short	Type=DT_REMOVECOLOR;
	
	if (!CurTheme || CurTheme->FidDelayedText == HFILE_ERROR)
		RemoveColorFromRect (hDC,RemoveColor,pRect);
	else
	{	
		BigWrite (CurTheme->FidDelayedText,(HPSTR)&Type,2,-1);
		BigWrite (CurTheme->FidDelayedText,(HPSTR)&RemoveColor,sizeof(COLORREF),-1);   
		BigWrite (CurTheme->FidDelayedText,(HPSTR)pRect,sizeof(RECT),-1);   
	}
	return ;
} 
	
BOOL DrawDelayedText (void)
{   
	int		RegionType;   
	short	l, Type;
	COLORREF	Color;  
	short	ltext;
	int		x,y, BKMode; 
	long	ii;
	LPSTR	Text;
	HANDLE	hMem;
	LPLOGFONT pLogFont;
    
    if (!CurTheme)
    	return FALSE;
	if (CurTheme->FidDelayedText <= 0)
		return FALSE;
    SaveDC (CurView->hDC);
    SetViewport (CurTheme->TargetViewport); 
    SetDisplayMode (CurView->hDC, GF_SCREENMODE);
    GSSiDeleteObject(&CurView->hRgn);
    CurView->hRgn = CreateVPRgn (FALSE,FALSE);
    RegionType = SelectClipRgn (CurView->hDC,CurView->hRgn);
    GSSiDeleteObject(&CurView->hRgn);  
    if (PrintingToMF || RegionType != NULLREGION)
    {
		GSSillseek (CurTheme->FidDelayedText,0,0);
		BigRead (CurTheme->FidDelayedText,(HPSTR)&l,2);
  		ii=GSSillseek (CurTheme->FidDelayedText,l,1);
  		while (BigRead (CurTheme->FidDelayedText,(HPSTR)&Type,2) == 2)
  		{
  			switch (Type)
  			{
				case DT_TEXTCOLOR:
					BigRead (CurTheme->FidDelayedText,(HPSTR)&Color,sizeof(COLORREF));
					SetTextColor (CurView->hDC,Color);
					break;
				case DT_TEXTOUT: 
					if (CurTheme->hDelayedFont)
						SelectObject (CurView->hDC,CurTheme->hDelayedFont);
					BigRead (CurTheme->FidDelayedText,(HPSTR)&BKMode,sizeof(int));
					BigRead (CurTheme->FidDelayedText,(HPSTR)&Color,sizeof(COLORREF)); 
					SetBkMode (CurView->hDC,BKMode);
					SetTextColor (CurView->hDC,Color);
					BigRead (CurTheme->FidDelayedText,(HPSTR)&x,sizeof(int));
					BigRead (CurTheme->FidDelayedText,(HPSTR)&y,sizeof(int));
					BigRead (CurTheme->FidDelayedText,(HPSTR)&ltext,sizeof(short));
					hMem = GSSiGlobAlloc (0,GMEM_MOVEABLE,ltext+1);
					Text = GlobalLock (hMem);
					BigRead (CurTheme->FidDelayedText,Text,ltext);  
					Text[ltext] = 0;					
					TextOut (CurView->hDC,x,y,Text,ltext);  
					GSSiGlobUlFree (&hMem);
					break;
				case DT_CREATEFONT: 
					SelectObject (CurView->hDC,GetStockObject(SYSTEM_FONT)); 
					GSSiDeleteObject (&CurTheme->hDelayedFont);
					hMem = GSSiGlobAlloc (0,GMEM_MOVEABLE,sizeof(LOGFONT));
					pLogFont = (LPLOGFONT)GlobalLock (hMem);
					BigRead (CurTheme->FidDelayedText,(HPSTR)pLogFont,sizeof(LOGFONT));
					pLogFont->lfOrientation = pLogFont->lfEscapement;
					CurTheme->hDelayedFont = CreateFontIndirect (pLogFont); 
					GSSiGlobUlFree (&hMem); 
					break;
				case DT_REMOVECOLOR:
					break;      
  			} 
  		}	 
  	}
  	CloseAndDeleteFile (&CurTheme->FidDelayedText);
	ShowBufferedScreen(TRUE, TRUE, 0, 0);
    return TRUE;
}

void ClearFlags (void)
{
	nFlags = 0;
	return;
}

BOOL HaveFlagOverlap (LPRECT pRect)
{
	UINT	i;
	RECT	IRect;

	for (i=0;i<nFlags;i++)
		if (IntersectRect (&IRect,pRect,&FlagRects[i]))
			return TRUE;

	return FALSE;
}

DWORD DispText (HDC hDC,BOOL GetExtents,int left, int right, int y, int symsize,int hJust, int vJust, double InSize,double SymbolSizeFactor,double SymbolTextFactor,
			   int InWeight, BOOL Italic, double AZIN, LPSTR InText,int shield,BOOL TestRect,
			   BOOL Shadow, COLORREF ShadowColor,long RemoveColor,long nPnts,HANDLE hAreaPoints,HANDLE hAreaAccelerator,
			   int nPoly,HANDLE hPolyPartLen,short UseHalfTone,LPSTR ActualText,short MinSize,LPTHEME CurTheme,LPRECT pTextRect,LPRECT pFullRect,LPRECT pFlagRect)
#if ENABLETRACE
{GSSiEnterProg (781);
#endif
{    
	int		Weight=InWeight;
	LPSTR	Text;
	LPSTR	pText2, pText3;
	DWORD	TextExt=0;
	short	nchar, NumLines, iline, ltext,theight,maxtheight=0, twidth, NumUpLines, MaxLineLen;
	int		Twidth, Theight,   xt, start; 
	static	short	lastheight=0, lastrot=9999;
	DWORD	i;
	static	HFONT	OldFont=0, TagFont=0;
	static	TAGBOX	TAGBox;  
	BOOL	Invert, DoInvert;  
	DPOINT	pt, pt2;    
	double	xmove=0, ymove;
	POINT	SymPoint; 
	COLORREF	OldColor;  
	BOOL	ChangedColor=FALSE, StretchSymbol=FALSE, DoRestore=FALSE;
	double	wfac=1;
	short	Y;
	clock_t	starttime;   
	double	AZ, AZInc;                                    
	double	Size; 
	short	symbol=0; 
	static	BOOL	First=TRUE;   
	double	PixelSize=0;    
	BOOL	UseFlag = FALSE;
	int		PoleLength = 12;
	int		TextFlagWidth, TextFlagHeight;
	RECT	FlagPointRect;
	LPRECT	pSymbolRectIn = pSymbolRect;
	double	tFac = 1;
	SIZE	txSize;
	LPSYMBOL	pSymbol;
	char	SymName[66]="";
	double	FlagDotShrink=0.65;

	if (GetExtents == 2)
		RectInit (&txtRect);
	if (PeopleNet)
		StretchSymbol=TRUE;
//	if (hDC) retrn	TextExt;
	if (shield < 0)
	{   
	    GSSiDeleteObject(&TagFont); 
//	    GSSiDeleteObject(&ShadowFont); 
		lastrot = 9999; 
		First = TRUE;
{
#if ENABLETRACE
GSSiExitProg (781);
#endif
	    return (TextExt);
}
	}  
	ClearFullWindowBitmap (0);
	Size=SmallFontLargeFontFactor*fabs((double)InSize); 
	Text = DispTextText; 
	if (First)
	{
		First=FALSE;
		InfoBoxInit (&TAGBox); 
		GetGlobalCVal ("[%LABELFONT]",TAGBox.LogFont.lfFaceName,"Arial");
	}
	AZ = AZIN;
	if (AZ > 1000000)
	{
		AZ = AZ - 2000000;
		DoInvert = FALSE;   
	}
	else
		DoInvert = TRUE;
	if (shield >= 20000)
	{
		symbol = shield - 20000;
		shield = 0;
		UseFlag = TRUE;
	} 
	else if (shield >= 10000)
	{
		symbol = shield - 10000;
		shield = 0;
	} 
	else
		symbol = GetShieldSymbol (shield); 
	if (UseFlag)
	{
		pSymbolRect = &FlagPointRect;
		RectInit (pSymbolRect);
	}
//	starttime=clock();  
	start = GetSymbolTextStart (shield);
	if (start && _fstrlen(&InText[start])>6)
	{
		start = 0;
		shield = 0;
		symbol = 0;
	}    
	_fstrcpy (Text,&InText[start]); 
	ProcessShieldText (shield,Text);    
    SymPoint.x = (right + left)/2;   
    SymPoint.y = y;
	if (shield) 
	{
		vJust=2;
		AZ = 0;//LTWOPI(-CurView->Rotation);
		DoInvert = FALSE;   
	    SaveDC (hDC);
		DoRestore = TRUE;
	    SetDisplayMode (hDC, GF_SCREENMODE);  
		SymPoint = TranPoint16 (SymPoint,CurView->hTranVPToScreen);
	}
	else if (InSize>=0)
	{
	   SaveDC (hDC);
	   DoRestore = TRUE;
	   SetDisplayMode (hDC, GF_TEXTMODE);  
	} 

TryAgain:

   if (abs(InWeight)>1)
   {   
   	   if (InWeight == 2) 
   	   {
   	   		TAGBox.LogFont.lfHeight =  max (1,IDNINT (Size));   
   	   		Weight = FW_BOLD;
   	   }
   	   else
   	   {		 
		   TAGBox.CoordStyle=3; 
		   PixelSize = TAGFontHtToPixels (Size,TAGBox.CoordStyle);
		   TAGBox.LogFont.lfHeight = max (1,IDNINT(PixelSize)); 
	   }
	    if (GetGraphicsMode (hDC) == GM_COMPATIBLE)
			AZInc = 0;
		else
			AZInc = CurView->Rotation;

		if (DoInvert && LTWOPI(AZ+AZInc) > HALFPI && LTWOPI(AZ+AZInc) < 3*HALFPI)
		{
			Invert=TRUE;
			AZ = LTWOPI (AZ+PY);
		}
		else
			Invert=FALSE;
	   TAGBox.LogFont.lfEscapement = TAGBox.LogFont.lfOrientation = (AZ * 10) / RADDEG;  
	   if (!TagFont || 
	   		TAGBox.LogFont.lfHeight != lastheight || TAGBox.LogFont.lfEscapement != lastrot || TAGBox.LogFont.lfWeight != abs(Weight))
	   {   
		   lastheight = TAGBox.LogFont.lfHeight;
		   lastrot = TAGBox.LogFont.lfEscapement;  
			if (TagFont)
			{   
				if (OldFont)
					SelectObject(hDC, OldFont);
				OldFont = 0;
			    GSSiDeleteObject(&TagFont); 
			}
		   
		   TAGBox.Just = hJust;  
		   if (abs(InWeight) < 2)
		   	TAGBox.LogFont.lfWeight = FW_DONTCARE;
		   else
		   	TAGBox.LogFont.lfWeight = abs(Weight);
		   TAGBox.LogFont.lfItalic = Italic;
		   TAGBox.LogFont.lfCharSet = ANSI_CHARSET;
		   TAGBox.LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
		   TAGBox.LogFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		   TAGBox.LogFont.lfQuality = PROOF_QUALITY;
		   TAGBox.LogFont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		   GetGlobalCVal ("[%LABELFONT]",TAGBox.LogFont.lfFaceName,"Arial");
		   TagFont = CreateFontIndirectDelayed((LPLOGFONT)&TAGBox.LogFont,CurTheme); 
	   }
	   if (TagFont)
	   	OldFont = SelectObject(hDC, TagFont); 
   }
   else if (InWeight==-1)
   		goto Exit; 
   	nchar = _fstrlen (Text);
	pText2 = DispTextText2;
	strncpy0 (pText2,Text,nchar);
   NumLines = GetNumTextLines (pText2,&NumUpLines,&MaxLineLen,FALSE);   
   Twidth = Theight = 0;
   for (iline =0;iline < NumLines;iline++)
   {    
   		pText3 = pText2;
		pText2 = _fstrchr (pText2,0);
		pText2++;
		if (*pText2 == '\n')
			pText2++; 
		if (strcmp (pText3," "))
			Truncate (pText3);
		ltext = _fstrlen(pText3);
		GetTextExtentPoint32 (hDC,pText3,ltext,&txSize);
		twidth = txSize.cx; 
		Twidth = max (twidth,Twidth);
		theight = txSize.cy;
		maxtheight = max (maxtheight,theight);
		Theight += theight;
   } 
   theight = maxtheight; 
   TextExt = MAKELPARAM (Twidth,Theight);   
   if (GetExtents == 1) 
   {
		txtRect.right = txtRect.bottom = -1;
		txtRect.left = txtRect.top = 0;
   		goto GetOut;                
   } 
   switch (hJust)
   {
   		case 1:
   			xmove = (right - left)/2;   
   			break;
   		case 3:
   			xmove = -(right - left)/2;
   			break;
   		case 2:
   			xmove = -Twidth/2;
   			break;
   		case 4:
   			xmove = -Twidth;
   			break;            
   	}  
   switch (vJust)
   {
   	case 1:
   		ymove = Theight + max (0,symsize)/2;
   		break;
   	case 2:
   		ymove = Theight/2;
   		break;
	case 3:
		ymove = -max(0, symsize) / 2;
		break;
	case 4:
		ymove = -Theight / 2;
		break;
   }
   	
    if (hAreaPoints)
    {    
   		HPDPOINT	pAreaPoints = (HPDPOINT)GlobalLock (hAreaPoints);
   		DPOINT		TextPoints[5], VPPoints[4], IntPoint;
   		BOOL		Int;
   		double		az = LTWOPI(TWOPI - AZ);
		LPHANDLE	pAccelerator=0;
   		
		if (hAreaAccelerator)
			pAccelerator = &hAreaAccelerator;
		pt.x = SymPoint.x;
		pt.y = SymPoint.y;
		pt2 = dnewpt (pt,az,xmove); 
//		pt2.y = pt2.y-(pt.y-pt2.y); 
		TextPoints[0] = dnewpt (pt2,az+HALFPI,-ymove); 
//		TextPoints[0].y = TextPoints[0].y - (pt2.y - TextPoints[0].y);
   		TextPoints[1] = dnewpt (TextPoints[0],az,Twidth);
//		TextPoints[1].y = TextPoints[1].y - (TextPoints[0].y - TextPoints[1].y);
		TextPoints[2] = dnewpt (TextPoints[1],az+HALFPI,Theight);
//		TextPoints[2].y = TextPoints[2].y - (TextPoints[1].y - TextPoints[2].y);
   		TextPoints[3] = dnewpt (TextPoints[2],az,-Twidth);  
   		TextPoints[4] = TextPoints[0];
//		TextPoints[3].y = TextPoints[3].y - (TextPoints[2].y - TextPoints[3].y);
		for (i=0;i<4;i++)
			if (!POINT_IN_AREAD (TextPoints[i], nPnts,pAreaPoints,1,0,0,pAccelerator))
			{
				GlobalUnlock (hAreaPoints);
				TextExt = 0;
				goto GetOut;
			}
		//if (!pAccelerator)
		{
			Int = IntersectPolys1 (GF_AREA,GF_AREA,4,TextPoints,NULL,nPnts,pAreaPoints,nPoly,hPolyPartLen,NULL,NULL,NULL,NULL,0);  //pAreaPoints[8]
			if (Int)
			{   
				TextExt = 0;
				GlobalUnlock (hAreaPoints);
				goto GetOut; 
			}
			if (!pAccelerator)
			{
				for (i=0;i<nPnts;i++)
					if (POINT_IN_AREAD (pAreaPoints[i], 4,TextPoints,1,0,0,0))
					{
						TextExt = 0;
						GlobalUnlock (hAreaPoints);
						goto GetOut;
					}
			}
		}
		GlobalUnlock (hAreaPoints);
		RectToDPoints (&CurView->ScreenRect,VPPoints);
		Int = IntersectPolys1 (GF_AREA,GF_AREA,4,TextPoints,NULL,4,VPPoints,0,NULL,NULL,NULL,NULL,NULL,0); 
		if (Int)
		{   
			TextExt = 0;
		    goto GetOut; 
		}
    }
//	pt.x = (double)(left + right)/2;
//	pt.y = y;
	pt = PointToDPoint (SymPoint);
	pt2 = dnewpt (pt,AZ,xmove);  
	pt2.y = IDNINT(pt.y-(pt2.y-pt.y));
	pt = dnewpt (pt2,AZ+HALFPI,ymove);
	xt = IDNINT(pt.x);
	Y = IDNINT(pt2.y-(pt.y-pt2.y));
	   
   if (TestRect || GetExtents==2)
   {    
   		DPOINT	p[4];
   		double	mnx=DBL_MAX,mny=DBL_MAX,mxx=-DBL_MAX,mxy=-DBL_MAX;
   		int		i;
   		
   		p[0].x = xt;
   		p[0].y = Y;
   		p[1] = dnewpt (p[0],AZ,Twidth);
   		p[1].y = IDNINT(p[0].y-(p[1].y-p[0].y));
   		p[2] = dnewpt (p[0],AZ+PY/2,-Theight);
   		p[2].y = IDNINT(p[0].y-(p[2].y-p[0].y));
   		p[3] = dnewpt (p[2],AZ,Twidth);
   		p[3].y = IDNINT(p[2].y-(p[3].y-p[2].y));
   		
   		for (i=0;i<4;i++)
   		{
   			mnx = min (mnx,p[i].x);
   			mny = min (mny,p[i].y);
   			mxx = max (mxx,p[i].x);
   			mxy = max (mxy,p[i].y); 
   		}
		txtRect.left = IDNINT(mnx);
		txtRect.right = IDNINT(mxx);
		txtRect.top = IDNINT(mny);
		txtRect.bottom = IDNINT(mxy);
//		InflateRect(&Rect,3,3);
		if (symbol)
		{   
			HANDLE hSymbol;  
			short	l = _fstrlen(Text);   
			short	SaveTwidth=Twidth, SaveTheight=Theight;	
			               
			wfac = 1;	               
			if (l < 2)
				wfac = 2; 
			else if (l > 2)
				wfac *= ((double)2.0)/l;
			if (!shield)
			{
				Theight*=1.5;
				Twidth*=1.5;
			} 
			else
			{
				Theight*=SymbolSizeFactor*1.5;
				Twidth*=SymbolSizeFactor*1.5;
			} 


/*
			if (l < 2)
				wfac = 2; 
			else if (l > 2)
				wfac *= ((double)2.0)/l;
			if (!shield)
			{
				Theight*=1.6;
				Twidth*=1.6;
			} 
			else
			{
				Theight*=SymbolSizeFactor*1.6;
				Twidth*=SymbolSizeFactor*1.6;
			}   */
			
			hSymbol = GetDictSymDesc (symbol,0);
			pSymbol = GlobalLock (hSymbol);
			strcpy (SymName,pSymbol->Name);
			GlobalUnlock (hSymbol);
			if (symsize < 0)
			{
				if (pFlagRect)
					DisplayPointSymbol (hSymbol, 0, -symsize*FlagDotShrink, -symsize*FlagDotShrink,AZIN, &SymPoint,&txtRect,FALSE,0,0,FALSE,FALSE,0,0);
				else
					DisplayPointSymbol (hSymbol, 0, -symsize, -symsize,AZIN, &SymPoint,&txtRect,FALSE,0,0,FALSE,FALSE,0,0);
			}
			else if (StretchSymbol) 
				DisplayPointSymbol (hSymbol, 0, Theight+2, wfac * Twidth+2,AZ, &SymPoint,&txtRect,FALSE,0,0,FALSE,FALSE,0,0);  
			else 
			{   
				double	fac=wfac*Twidth;
				if (fac && fac < MinSize)
				{
					Size *= MinSize/fac;
					MinSize = 0;
					tFac = SymbolTextFactor;
					DestroySymbol (hSymbol);  
					goto TryAgain;
				}
				DisplayPointSymbol (hSymbol, 0, wfac*Twidth, wfac * Twidth,AZ, &SymPoint,&txtRect,FALSE,0,0,FALSE,FALSE,0,0);     
			}
			DestroySymbol (hSymbol);  
			Twidth = SaveTwidth;
			Theight = SaveTheight;
		}
		if (GetGraphicsMode (hDC) == GM_COMPATIBLE)
		{
			if (!RectCompletelyInRect(&CurView->ScreenRect,&txtRect))
			{
				TextExt = 0;
				goto GetOut;
			}
		}
		else
		{
			if (!RectInRect(&CurView->DrawRect,&txtRect))
			{
				TextExt = 0;
				goto GetOut;
			}
		}
		if (TestRect!=2)
		{
			if (!AddTextRect(&txtRect))
			{
				TextExt = 0;
				goto GetOut;
			}
		}
//RemoveColor = -1;
		if (RemoveColor >= 0)
			RemoveColorFromRectDelayed (hDC,RemoveColor,&txtRect,CurTheme);
   }   
   
	if (symbol>0)
	{   
		HANDLE hSymbol;
		short	l = _fstrlen(Text);
		
		wfac = 1;	               
		if (l < 2)
			wfac = 2; 
		else if (l > 2)
			wfac *= ((double)2.0)/l;
		if (!shield)
		{
			Theight*=1.5;
			Twidth*=1.5;
		} 
		else
		{
			Theight*=SymbolSizeFactor*1.5;
			Twidth*=SymbolSizeFactor*1.5;
		} 
		hSymbol = GetDictSymDesc (symbol,0);
		if (symsize < 0)
		{
			if (pFlagRect)
				DisplayPointSymbol (hSymbol, hDC, -symsize*FlagDotShrink, -symsize*FlagDotShrink,AZIN, &SymPoint,pFullRect,FALSE,0,0,FALSE,TRUE,0,0);  
			else
				DisplayPointSymbol (hSymbol, hDC, -symsize, -symsize,AZIN, &SymPoint,pFullRect,FALSE,0,0,FALSE,TRUE,0,0);  
		}
		else if (StretchSymbol) 
			DisplayPointSymbol (hSymbol, hDC, Theight+2, wfac * Twidth+2,AZ, &SymPoint,pFullRect,FALSE,0,0,FALSE,TRUE,0,0);
		else 
		{
			double f=((double)2.0)/max (3,l);

			switch (l)
			{
			case 1:
			case 2:
			case 3:
				f *= Theight;
				break;
			default:
				f *= Twidth;
				break;
			}
		//	if (SymbolTextFactor && SymbolTextFactor < 1)
		//		f /= SymbolTextFactor;
			RectInit (&txtRect);
//			DisplayPointSymbol (hSymbol, 0, f, f,AZ, &SymPoint,&txtRect,FALSE,0,0,FALSE,TRUE,0,0);  
			DisplayPointSymbol (hSymbol, hDC, f, f,AZ, &SymPoint,&txtRect,FALSE,0,0,FALSE,TRUE,0,0);  
		}
		//SetPixel (hDC,SymPoint.x,SymPoint.y,0);
		DestroySymbol (hSymbol);    
		if (ActualText && *ActualText && (txtRect.right - txtRect.left > 10))
		{   
			RECT	Rect = txtRect;  
			short	x,y, Twidth, Theight, RectWidth, RectHeight; 
			HBRUSH	hBrush = SelectObject (hDC,GetStockObject (WHITE_BRUSH));
			HBRUSH	hPen = SelectObject (hDC,GetStockObject (BLACK_PEN));  
			POINT	Points[4];
			COLORREF	TextColor;    
			LOGFONT	LogFont = TAGBox.LogFont;  
			HFONT	hOldFont, hFont=0;
			
//			FillRectPoly(hDC, &Rect, RGB(255,255,0));
			LogFont.lfHeight = max (1, LogFont.lfHeight*0.85);	    
			Rect.bottom = Rect.top-DeviceToScreenFactor();
			RectWidth = Rect.right - Rect.left - 2;
			Rect.left++;
			TextColor = SetTextColor (hDC,0); 
			do
			{
				GSSiDeleteObject (&hFont);
				hFont = CreateFontIndirectDelayed((LPLOGFONT)&LogFont,0); 
				hOldFont = SelectObject (hDC,hFont);
				GetTextExtentPoint32 (hDC,ActualText, _fstrlen(ActualText),&txSize);  
				Twidth = txSize.cx;   
				Theight = txSize.cy;   
				LogFont.lfHeight--;  
				SelectObject (hDC,hOldFont);
			} while (Twidth > RectWidth  && LogFont.lfHeight > 0);
			LogFont.lfHeight++;
			TextExt = MAKELPARAM (Twidth,Theight);   
			hOldFont = SelectObject (hDC,hFont);
			Rect.top = Rect.bottom - LogFont.lfHeight; 
			RectHeight = Rect.bottom - Rect.top - 2;
			RectToPoints (&Rect,Points); 
			SetROP2(hDC,DisplayRasterOpt);
			Polygon (hDC, Points,4);	
			y = Rect.bottom -1 - LogFont.lfHeight - (RectHeight - LogFont.lfHeight)/2;
			x = Rect.left + (RectWidth - Twidth)/2;       
			if (!GetExtents)
				TextOut(hDC, x,y, ActualText, _fstrlen(ActualText)); 
			SelectObject (hDC,hPen);
			SelectObject (hDC,hBrush); 
			SelectObject (hDC,hOldFont);
			SetTextColor (hDC,TextColor); 
			GSSiDeleteObject (&hFont);
		}
		if (!stricmp (SymName,"MSASHIELD") || !stricmp (SymName,"MNSTATESHIELD"))
		{
			txtRect.top += (txtRect.bottom - txtRect.top)/3;
			txtRect = PctRect (txtRect,-5);
		}
		else if (!stricmp (SymName,"HENNCOUNTYSHIELD"))
		{
			txtRect.top += (txtRect.bottom - txtRect.top)/5;
			txtRect.bottom -= (txtRect.bottom - txtRect.top)/5;
			txtRect = PctRect (txtRect,-10);
		}
		else
			txtRect = PctRect (txtRect,-5);
		if (!UseFlag && symsize > 0)
		{
			SymPoint = RectMid (&txtRect);   
			hJust = vJust = 2;
			nchar = strlen (Text);
			GetTextExtentPoint32 (hDC,Text,nchar,&txSize);  
			tFac = min (((double)txtRect.bottom - txtRect.top) / txSize.cy,((double)txtRect.right - txtRect.left) / txSize.cx);
			theight = TAGBox.LogFont.lfHeight *= tFac;
			SelectObject (hDC,OldFont);
			GSSiDeleteObject (&TagFont);
			TagFont = CreateFontIndirectDelayed((LPLOGFONT)&TAGBox.LogFont,CurTheme); 
			OldFont = SelectObject(hDC, TagFont); 
		}
		tFac = 1;
	}
	ChangedColor = SetShieldTextColor (hDC,shield,&OldColor,UseHalfTone);
   	nchar = _fstrlen (Text);
	pText2 = DispTextText2;;
	strncpy0 (pText2,Text,nchar);
    NumLines = GetNumTextLines (pText2,&NumUpLines,&MaxLineLen,FALSE);   
    Twidth = Theight = 0;
    for (iline =0;iline < NumLines;iline++)
    {   
    	pText3 = pText2;
		pText2 = _fstrchr (pText2,0);
		pText2++;
		if (*pText2 == '\n')
			pText2++; 
    	Truncate (pText3);
		ltext = _fstrlen(pText3); 
		if (ltext)
		{
			GetTextExtentPoint32 (hDC,pText3,ltext,&txSize);  
			Twidth = txSize.cx; 
			Theight = txSize.cy; 
			TextExt = MAKELPARAM (Twidth,Theight);   
			if (NumLines > 1 ||symbol>0)
			{
			   switch (hJust)
			   {
			   		case 1:
			   			xmove = (right - left)/2;   
			   			break;
			   		case 3:
			   			xmove = -(right - left)/2;
			   			break;
			   		case 2:
			   			xmove = -Twidth/2;
			   			break;
			   		case 4:
			   			xmove = -Twidth;
			   			break;            
			   	}  
			   switch (vJust)
			   {
			   	case 1:
			   		ymove = Theight*(NumLines-iline) + max (0,symsize)/2;
			   		break;
			   	case 2:
					ymove = (((double) NumLines) / 2) * theight - iline * theight; 
//			   		ymove = Theight/2 + (NumLines-iline)*Theight;
			   		break;
			   	case 3:
			   		ymove =  -max (0,symsize)/2;
			   		break;    
			   	}
			   	
			//	pt.x = (double)(left + right)/2;
			//	pt.y = y;
				pt = PointToDPoint (SymPoint);
				pt2 = dnewpt (pt,AZ,xmove);  
				pt2.y = IDNINT(pt.y-(pt2.y-pt.y));
				pt = dnewpt (pt2,AZ+HALFPI,ymove);
				xt = IDNINT(pt.x);
				Y = IDNINT(pt2.y-(pt.y-pt2.y));
			} 
			if (_fstrcmp (pText3,"%%%")) 
			{   
				if (ActualText && !symbol) 
				{
					pText3 = ActualText;
					ExpandText (pText3); 
					ltext = _fstrlen (pText3);
				}
				if (!GetExtents && !UseFlag && (Shadow == 1 && !symbol || Shadow > 1))
				{   
					short	i, inc=max(1,(TAGBox.LogFont.lfHeight-5)/10); 
					int		iDeviceToScreenFactor = max(1, DeviceToScreenFactor() + 0.5);//IDNINT(DeviceToScreenFactor());
					
					OldColor = SetTextColorDelayed (hDC,ConvertColor(ShadowColor,UseHalfTone),CurTheme);  
					i = 1;
					while (i <= iDeviceToScreenFactor)//*2)
					{
						TextOutDelayed (hDC,xt+i,Y+i,pText3,ltext,CurTheme); 
						TextOutDelayed (hDC,xt-i,Y+i,pText3,ltext,CurTheme); 
						TextOutDelayed (hDC,xt-i,Y-i,pText3,ltext,CurTheme); 
						TextOutDelayed (hDC,xt+i,Y-i,pText3,ltext,CurTheme); 
						i++;
					} 
					SetTextColorDelayed (hDC,OldColor,CurTheme);
				}
				LoadDynamicText (xt,Y,AZ,PixelSize,pText3,ltext);
				if (UseFlag)
				{
					RECT	FlagRect;
					POINT	MidPoint = RectMid (&CurView->ScreenRect);
					POINT	PolePoints[2];
					HPEN	hPolePen, hOldPen;
					int		Attempts=1;
					
				    //SetDisplayMode (hDC, GF_TEXTMODE);  
					TextFlagHeight = PixelSize + 4;
					TextFlagWidth  = Twidth + 4;
					if (nFlags < MAX_FLAGS)
						FlagRects[nFlags++] = *pSymbolRect;
NextAttempt:
					switch (Attempts)
					{
					case 1:
						FlagRect.left  = SymPoint.x + PoleLength;
						FlagRect.right = FlagRect.left + TextFlagWidth;
						FlagRect.bottom = SymPoint.y - PoleLength;
						FlagRect.top   = FlagRect.bottom - TextFlagHeight;
						break;
					case 2:
						FlagRect.right = SymPoint.x - PoleLength;
						FlagRect.left  = FlagRect.right - TextFlagWidth;
						FlagRect.bottom = SymPoint.y - PoleLength;
						FlagRect.top   = FlagRect.bottom - TextFlagHeight;
						break;
					case 3:
						FlagRect.right = SymPoint.x - PoleLength;
						FlagRect.left  = FlagRect.right - TextFlagWidth;
						FlagRect.top = SymPoint.y + PoleLength;
						FlagRect.bottom   = FlagRect.top + TextFlagHeight;
						break;
					case 4:
						FlagRect.left  = SymPoint.x + PoleLength;
						FlagRect.right = FlagRect.left + TextFlagWidth;
						FlagRect.top = SymPoint.y + PoleLength;
						FlagRect.bottom   = FlagRect.top + TextFlagHeight;
						break;
					}
					if (HaveFlagOverlap (&FlagRect) && Attempts < 4)
					{
						Attempts++;
						goto NextAttempt;
					}
					if (nFlags < MAX_FLAGS)
						FlagRects[nFlags++] = FlagRect;
					//FillRectColor (hDC,&FlagRect,RGB(255,0,0));
					hPolePen = CreatePen (PS_SOLID,1,RGB(255,0,0));
					hOldPen = SelectObject (hDC,hPolePen);
					PolePoints[0] = SymPoint;
					PolePoints[1] = RectMid (&FlagRect);
					Polyline (hDC,PolePoints,2);
					SelectObject (hDC,hOldPen);
					GSSiDeleteObject (&hPolePen);
					xt = FlagRect.left + 2;
					Y = FlagRect.top + 2;
					SetTextColor (hDC,RGB(0,0,128)); 
					if (pFlagRect)
						*pFlagRect = FlagRect;
					//TextOutDelayed (hDC,xt,Y,pText3,ltext,CurTheme);
					if (!GetExtents)
						TextOutWithShadow (hDC,xt,Y,pText3,ltext,1,RGB(255,255,255));
				}
				else
				{
					if (tFac != 1)
					{
						if (TagFont)
						{   
							if (OldFont)
								SelectObject(hDC, OldFont);
							OldFont = 0;
							GSSiDeleteObject(&TagFont); 
						}
					   TAGBox.LogFont.lfHeight *= tFac;
					   TagFont = CreateFontIndirectDelayed((LPLOGFONT)&TAGBox.LogFont,CurTheme); 
					   OldFont = SelectObject(hDC, TagFont); 
					   GetTextExtentPoint32 (hDC,pText3,ltext,&txSize);  
					}
					if (!GetExtents)
						TextOutDelayed (hDC,xt,Y,pText3,ltext,CurTheme);
				}
			}
		}
    }  
	if (ChangedColor)
		SetTextColorDelayed (hDC,OldColor,CurTheme);
GetOut:
	if (InWeight<=0)
		goto ByeBye;

Exit: 
	if (OldFont)
		SelectObject(hDC, OldFont);
	OldFont = 0;
ByeBye:
//	TotTextDisplayTime+=(clock()-starttime);
	if (DoRestore)
    	RestoreDC (hDC,-1);
	if (pTextRect)
		*pTextRect = txtRect;
	if (pFullRect)
	{
		if (IsRectEmpty (pFullRect))
		{
			if (!IsRectEmpty (&txtRect))
				*pFullRect = txtRect;
		}
		else if (!IsRectEmpty (&txtRect))
			UnionRect (pFullRect,pFullRect,&txtRect);
	}
	pSymbolRect = pSymbolRectIn;
	lastheight = 0;
{
#if ENABLETRACE
GSSiExitProg (781);
#endif
	GdiFlush();
    return (TextExt);
}
#if ENABLETRACE
}
#endif
}   

BOOL DisplayStreetAddresses(BOOL Init)
#if ENABLETRACE
{GSSiEnterProg (764);
#endif
{   
	LPSHORT	pnRef;  
	LPLONG	pRef;
	long	nRef;
	long	Offset; 
	static	long	AtRef; 
	static	long	MaxPerCall;  
	LPVIEWPORT	SaveView=CurView;
	LPGWDHEADER	lpGWDHead; 
	BOOL	OpenedSeg=FALSE;     
	HRGN	hRgn;    
	double	size;
	static	BOOL	HaveInit=FALSE,OpenedSP=FALSE; 
	static	LPTHEME	DisplayAddTheme=NULL;
	
	if (Init)
		DisplayAddTheme = CurTheme;
	else
		CurTheme = DisplayAddTheme;
	if (!CurTheme)
		goto RtnFalse;
	if (CurTheme->ID != GF_STREET_ADDRESS_THEME || !CurTheme->hScatterFile)
		goto RtnFalse;    
	if (CurTheme->TargetViewport)
		SetCurView ( pViewports[CurTheme->TargetViewport-1]);
	size=AddressTextSize / CurView->Scale;
	if (size < 0.04)
		goto RtnFalse;             
	if (Init)
	{
		HaveInit = TRUE;  
		DisplayAddTheme = CurTheme;
	 	CloseEditRect(CurView);
	 	AtRef = 0;
	 	if (!Printing)
	 	{   
	 		MaxPerCall = 5;
		 	EditRectInit (sizeof(EDITRECTINFO));  
{
#if ENABLETRACE
GSSiExitProg (764);
#endif
			return TRUE;
}
		} 
		else 
			MaxPerCall = LONG_MAX;
	} 
	if (!HaveInit)
		goto RtnFalse;
	
    SetDisplayMode (CurView->hDC, GF_SCREENMODE);
	GSSiDeleteObject(&CurView->hRgn);
	CurView->hRgn = CreateVPRgn(FALSE,FALSE);
	SelectClipRgn (CurView->hDC,CurView->hRgn);
	GSSiDeleteObject(&CurView->hRgn); 
	pnRef = (LPSHORT)GlobalLock (CurTheme->hScatterFile); 
	nRef = *pnRef++;
	if (AtRef >= nRef)
		goto Exit;
	pRef = (LPLONG)pnRef;
	pRef += AtRef;
	if (!OpenStreetSegmentTable (FALSE,&OpenedSeg))
		goto Exit;
	OpenStreetPolys (&OpenedSP);
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetSegments); 
	nRef = min (nRef - AtRef,MaxPerCall);
	while(nRef--)
	{   
		DisplaySegmentAdds (lpGWDHead,pRef,CurTheme->Xmove,CurTheme->Ymove,CurTheme->UseHalfTone);
		pRef++; 
		AtRef++; 
    }
	CloseStreetPolys (OpenedSP);
    GlobalUnlock (hDBStreetSegments);
    CloseStreetSegmentTable(OpenedSeg); 
	if (Printing)
		goto Exit;
	GlobalUnlock (CurTheme->hScatterFile); 
	SetCurView ( SaveView);  
{
#if ENABLETRACE
GSSiExitProg (764);
#endif
	return TRUE;
}
Exit: 
	GSSiGlobUlFree (&CurTheme->hScatterFile);
RtnFalse:
	SetCurView ( SaveView);  
	HaveInit = FALSE;   
	DisplayAddTheme = NULL;
{
#if ENABLETRACE
GSSiExitProg (764);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}  

void DisplaySegmentAdds (LPGWDHEADER lpGWDHead,LPLONG pRef, short DisplayOpt, short AllowAdd,short UseHalfTone)
#if ENABLETRACE
{GSSiEnterProg (765);
#endif
{
    POINT	WP1={100,100}, WP2={101,100};
    DPOINT	ENDPoint,CLPoint, ADDPoint, MIDPoint;
    LPSEGDATAGM	pSegdata = (LPSEGDATAGM)&lpGWDHead->GWDData;
    double	AZ=0, Length, PCT;
    double	OffDist1=18, OffDist2=6;
    long	Offset;  
    char	txt[80];
    short	i, Theight, Twidth;   
    DWORD	TextExt; 
    DPOINT	BP1, BP2;
    double	Factor;   
    COLORREF	OldColor;
	HDC		hDC;
    
	hDC = ScreenBufferDC (CurView->hWnd,CurView->hDC);
	TextExt = DispText (hDC,TRUE,100,-100, -100,0, 2,2,
						AddressTextSize / CurView->BaseUnitsPerPixel,1,1,100, FALSE,AZ,"ABCDEFGHIJKL",0,2,0,0,-1,0,NULL,0,0,NULL,UseHalfTone,0,0,0,0,0,0); 
    Twidth = LOWORD(TextExt);     
    Theight = HIWORD(TextExt);     
    BP1 = ScreenPtToBasePt (WP1);
    BP2 = ScreenPtToBasePt (WP2);
    Factor = ldistp (BP1,BP2);  
    OffDist1 = Twidth/2 * Factor;
    OffDist2 = 1.5 * Theight * Factor;
	OldColor = SetTextColor (hDC,0);
    if (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)pRef,BT_FIRST,BT_EQ,(LPSTR)&Offset))
    {   
		FillGWDData (lpGWDHead,Offset);
		PCT = 0; 
		if (DisplayOpt == 1 || DisplayOpt == 3)
		{
			if (GetSegCoorByPct (*pRef,PCT,&ENDPoint,&AZ,&Length,FALSE)) 
			{ 
				CLPoint = dnewpt (ENDPoint,AZ,OffDist1);
				ADDPoint = dnewpt (CLPoint,AZ-PIHALF,OffDist2/2);
				DisplayAddress (hDC,&ADDPoint,AZ,ltoa(pSegdata->faddl,txt,10),*pRef,0,UseHalfTone);
				ADDPoint = dnewpt (CLPoint,AZ+PIHALF,OffDist2/2);
				DisplayAddress (hDC,&ADDPoint,AZ,ltoa(pSegdata->faddr,txt,10),*pRef,1,UseHalfTone);
			}
			PCT = 1;
			if (GetSegCoorByPct (*pRef,PCT,&ENDPoint,&AZ,&Length,FALSE)) 
			{ 
				CLPoint = dnewpt (ENDPoint,AZ,-OffDist1);
				ADDPoint = dnewpt (CLPoint,AZ-PIHALF,OffDist2/2);
				DisplayAddress (hDC,&ADDPoint,AZ,ltoa(pSegdata->taddl,txt,10),*pRef,2,UseHalfTone);
				ADDPoint = dnewpt (CLPoint,AZ+PIHALF,OffDist2/2);
				DisplayAddress (hDC,&ADDPoint,AZ,ltoa(pSegdata->taddr,txt,10),*pRef,3,UseHalfTone);
			}
		}
		PCT = 0.5;
		if (GetSegCoorByPct (*pRef,PCT,&MIDPoint,&AZ,&Length,FALSE))
		{   
			if (DisplayOpt == 2 || DisplayOpt == 3)
			{
				for (i=0;i<4;i++)
				{   
					ADDPoint = dnewpt (MIDPoint,AZ+PIHALF,i*OffDist2+OffDist2/2);
					if (pSegdata->StreetNum[i])
						GetTrueStreetName (pSegdata->StreetNum[i],txt,0,0);
					else  
					{
						if (!AllowAdd)
							break;
						_fstrcpy (txt,"*****");
					}
					DisplayAddress (hDC,&ADDPoint,AZ,txt,*pRef,4+i,UseHalfTone);
					if (!pSegdata->StreetNum[i])
						break;
				}   
			}
			else if (DisplayOpt == 4)
			{ 
				itoa (pSegdata->Speed,txt,10);
				ADDPoint = dnewpt (MIDPoint,AZ+PIHALF,OffDist2/2);
				DisplayAddress (hDC,&ADDPoint,AZ,txt,*pRef,8,UseHalfTone);
			}
			else if (DisplayOpt == 5)
			{ 
				ltoa (pSegdata->TrafVol,txt,10);
				ADDPoint = dnewpt (MIDPoint,AZ+PIHALF,OffDist2/2);
				DisplayAddress (hDC,&ADDPoint,AZ,txt,*pRef,9,UseHalfTone);
			}
			else if (DisplayOpt == 7)
			{ 
				switch (pSegdata->OneWay)
				{
					case 0:
						_fstrcpy (txt,"<>");
						break;
					case 1:
						_fstrcpy (txt,"----->");
						break;
					case 2:
						_fstrcpy (txt,"<-----");
						break;
					default:
						_fstrcpy (txt,"???");
						break;
						
				}
				ADDPoint = dnewpt (MIDPoint,AZ+PIHALF,OffDist2/2);
				DisplayAddress (hDC,&ADDPoint,AZ+2000000,txt,*pRef,10,UseHalfTone);
			}
			else if (DisplayOpt == 8)
			{ 
				itoa (pSegdata->Lanes,txt,10);
				ADDPoint = dnewpt (MIDPoint,AZ+PIHALF,OffDist2/2);
				DisplayAddress (hDC,&ADDPoint,AZ,txt,*pRef,11,UseHalfTone);
			}
		}
	}
	SetTextColor (hDC,OldColor);
	ShowBufferedScreen (TRUE,TRUE,0,0);

{
#if ENABLETRACE
GSSiExitProg (765);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}


void DisplayAddress (HDC hDC,LPDPOINT DPoint, double AZ, LPSTR txt,long Ref,int ID,short UseHalfTone)
#if ENABLETRACE
{GSSiEnterProg (766);
#endif
{
	POINT	point; 
	EDITRECTINFO Info;
	DWORD	TextExt;
	double 	size= AddressTextSize / CurView->BaseUnitsPerPixel;

	point = BasePtToScreenPt (DPoint);
	if (PtInRect (&CurView->ScreenRect,point))
	{
		TextExt = DispText (hDC,FALSE,point.x,point.x, point.y, 0,2,2,
					size,1,1,100, FALSE,AZ,txt,0,2,0,0,-1,0,NULL,0,0,NULL,UseHalfTone,0,0,0,0,0,0); 
		Info.Width = LOWORD(TextExt); 
		Info.Height = HIWORD(TextExt);
		Info.AZ = LTWOPI(AZ) * 1000; 
		Info.MidPoint = point;
		Info.Refno = Ref;
		Info.AddID = ID; 
		AddEditRect(&txtRect, &Info);
	}
{
#if ENABLETRACE
GSSiExitProg (766);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL CreateStnameFilesUpdate (void)
{	BTVARDESC	BTVar[3];
	long		nRecs, nLoaded;
	HDC			hDC;
	int			stSeg, st2;
	BOOL		RtnVal=FALSE;
	long			SegMaxData=0, Offset;
	time_t ltime;
	HANDLE		hStname1, hStname2, hStname,hAddList;
	char		name[34];

	ltime = 0;
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BT_CREATE ("[%GEOSPAN_LOC]stname1.btr", 34, FALSE, 1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	hStname1 = BT_OPEN ("[%GEOSPAN_LOC]stname1.btr", ltime, BT_WRITE, 0);
	BTVar[0].BT_VARTYP=BT_CHAR;
	BTVar[0].BT_VARLEN=32;
	BTVar[0].BT_VAROFF=0;
	BT_CREATE ("[%GEOSPAN_LOC]stname2.btr", 4, FALSE, 1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	ltime = 0;
	hStname2 = BT_OPEN ("[%GEOSPAN_LOC]stname2.btr", ltime, BT_WRITE, 0);
	BT_CLOSE (hStname1);
	BT_CLOSE (hStname2);
	return (TRUE);
}
/*void ReadSegData (long Offset, LPSEGDATA pSegdata)
{   UINT	len;

  	_llseek (SegDataFid,Offset,0);
   	BigRead (SegDataFid,&len,2);
   	BigRead (SegDataFid,pSegdata,len);
   	return;

}

void WriteSegData (long Offset, LPSEGDATA pSegdata)
{   UINT	len;

  	_llseek (SegDataFid,Offset,0);  
  	len = sizeof(Segdata);
   	_lwrite (SegDataFid,&len,2);
   	_lwrite (SegDataFid,pSegdata,len);
   	LocalSegdata = *pSegdata;
   	return;

}  

BOOL UpdateSegData (long TLID, LPSEGDATA pSegdata)
{   UINT	len;  
	long	Offset, ii; 
	int		st;

	st = BT_FIND (hSegData,(LPSTR)&TLID,BT_FIRST,BT_EQ,(LPSTR)&Offset);
	if (st) return FALSE;
  	_llseek (SegDataFid,Offset,0);
  	len = sizeof(Segdata);
   	ii=_lwrite (SegDataFid,&len,2);
   	ii=_lwrite (SegDataFid,pSegdata,len);
   	LocalSegdata = *pSegdata;
   	return TRUE;

} 

void	CloseAddressFiles (BOOL Final)
{
    if (!Final) return; 
    BT_CLOSE (hBTMFN);
    hBTMFN = 0;
    BT_CLOSE (hBTNZINM);
    hBTNZINM = 0;
	BT_CLOSE (hNames1);
	hNames1 = 0;
	BT_CLOSE (hNames2);
	hNames2 = 0;
	BT_CLOSE (hSegMax);
	hSegMax = 0;
    CloseGWDatabase (hDBSegdata);
    hDBSegdata = 0;
	hSegData = 0;
	GSSiTrace ("Closing");
	BT_CLOSE (hActAdd);
	hActAdd = 0;
	BT_CLOSE (hStreetAdd);
	hStreetAdd = 0;
	BT_CLOSE (hIntersect);
	hIntersect = 0;
	BT_CLOSE(hSubFrames);
    hSubFrames=NULL; 
    return;
} */

BOOL CreateSEGBETWEEN_INT_MATCHTable (LPSTR File,char OrigKeyType,short OrigKeyLen)
{
	char	DefStr1[]="RecordNumber(B4),MatchCodeFrom(B2),MatchCodeTo(B2),LocationCodeFrom(B2),LocationCodeTo(B2),IntIDFrom(B4),IntIDTo(B4),StreetNumOn(B4),StreetNumFrom(B4),StreetNumTo(B4),FromMPRef(B4),FromMPPCT(R4),ToMPRef(B4),ToMPPCT(R4),";
	char	DefStr2[]="MunicFrom(B4),MunicTo(B4),ZIPFrom(B4),ZIPTo(B4),FromPointX(R8),FromPointY(R8),ToPointX(R8),ToPointY(R8),ChangedMunicFrom(B2),ChangedMunicTo(B2),ChangedZIPFrom(B2),ChangedZIPTo(B2),Direction(R4),Offset(R8),NumSegs(B4),";
	char	DefStr3[]="OnStreet(C32),FromStreet(C32),ToStreet(C32),Symbol(C32),OriginalKey(%c%i)";
	HANDLE	hDB;
	LPGWDHEADER lpGWDHead;  
	HANDLE	hMem=GSSiGlobAlloc (0,GMEM_MOVEABLE,4096);
	LPSTR	DefStr=GlobalLock (hMem); 
	LPSTR	Format=DefStr+2048;
	BOOL	rtn;
	
	_fstrcpy (Format,DefStr1);
	_fstrcat (Format,DefStr2);
	_fstrcat (Format,DefStr3); 
	
	sprintf (DefStr,Format,OrigKeyType,OrigKeyLen);
	
	rtn = CreateGWDDatabase (File,1,FALSE,0,1,DefStr);  
	GSSiGlobUlFree (&hMem);
	return rtn;
}

BOOL CreateSEGBETWEEN_INT_SEGS (LPSTR File,char OrigKeyType,short OrigKeyLen)
{
	char	DefStr[128];
	HANDLE	hDB;
	LPGWDHEADER lpGWDHead;  
	BOOL	rtn;
	sprintf (DefStr,"SegRefno(B4),Value(%c%i),PCTFrom(B2),PCTTo(B2)",OrigKeyType,(int)OrigKeyLen);
	rtn = CreateGWDDatabase (File,1,FALSE,0,2,DefStr);  
	return rtn;
}  

BOOL GetFieldInfoFromName (HANDLE hDB,LPSTR FldName,LPSTR pFldType,LPSHORT pFldLen) 
{
	FIELDINFO	FieldInfo;
	
	*pFldType = 'C';
	if (*FldName == '[' && *LastChr (FldName) == ']')
	{
    	strncpy (FieldInfo.name,&FldName[1],sizeof(FieldInfo.name)-1);
    	*LastChr (FieldInfo.name) = 0;
    }
    else
    	strncpy (FieldInfo.name,FldName,sizeof(FieldInfo.name)-1);
	if (GetDBFieldInfo (&FieldInfo,hDB))
	{   
		*pFldLen = FieldInfo.length;
		switch (FieldInfo.type)
		{
			default:
			case BT_CHAR:
			break;
				
			case SQL_INTEGER:
			case SQL_SMALLINT:
			case BT_INTEGER:
			case BT_INT2:
			case BT_INT4:
			*pFldType = 'B';
			break;
					
			case SQL_NUMERIC:
			case SQL_DECIMAL:
			case SQL_FLOAT:
			case SQL_REAL:
			case SQL_DOUBLE:
			case BT_REAL:
			case BT_REAL4:
			case BT_REAL8:
			*pFldType = 'R';
			break;
		}
		return TRUE;
	}
	else
		return FALSE;
}



 

LPSTR GetHouseAndStreet (LPSTR pStreet,LPSTR HouseNum)
{   
	 LPSTR	pSpace,pSpace2; 
	 int	ln;
	 char	Street1[256], Street2[256], Street3[256];
	                 	 
	 *HouseNum = 0;
	 if (!SeparateIntStreets (pStreet,Street1,Street2) && !SeparateOnFromToStreets (pStreet,Street1,Street2,Street3)) 
	 {   
	 	 LPSTR	pStart = pStreet;
				 	 	 	 
	     if (isdigit (*pStreet))
	     {
	         if ((pSpace = _fstrchr (pStreet,' ')))
	         {
	         	if ((pSpace2 = _fstrchr ((pSpace+1),' ')))
	         	{   
	         		*pSpace2 = 0;
	         		if (!_fstricmp ((pSpace+1),"BLOCK") || 
	         			!_fstricmp ((pSpace+1),"BLK"))
	         		{
	         			pSpace = pSpace + 1; 
	         			pStreet = pSpace2+1;
	         		}
	         		else
	         		{
	         			*pSpace++ = 0;
	         			*pSpace2++ = ' ';
						ln = strlen (pStreet);
						if (ln > 2 && (!stricmp (pStreet+ln-2,"ST") || !stricmp (pStreet+ln-2,"ND") || !stricmp (pStreet+ln-2,"RD")))
						{
							*--pSpace = ' ';
							return pStreet;
						}
	         			pStreet = pSpace;  
	         		}
	         	}
	         	else
	         	{
	         		*pSpace++ = 0; 
					ln = strlen (pStreet);
					if (ln > 2 && (!stricmp (pStreet+ln-2,"ST") || !stricmp (pStreet+ln-2,"ND") || !stricmp (pStreet+ln-2,"RD")))
					{
						*--pSpace = ' ';
						return pStreet;
					}
	             	pStreet = pSpace;
	         	}
	         	_fstrcpy (HouseNum,pStart);
	         }
	     }
	 }  
	 return pStreet;
} 

  

short AddMatchSingle (LPSTR HouseAndStreet,LPSTR City,LPSTR ZIP,LPSTR OutAddress,LPDPOINT pOutCoord,LPADDMATCH pMatchOut,int matchOpt)
{ 
	char	Street[512],Street1[256],Street2[256], OnStreet[256], HouseNumC[32],TrueStreet1[64],TrueStreet2[64],TrueCity[64],CityAbv[16];
	LPSTR	pStreet=Street; 
	short	match=0,nPartsRemoved=0,rtn=0;
	HANDLE	hMatch=0;
	long	MunicNum, StreetNum1, StreetNum2, StreetNum, OnStreetNum; 
	BOOL	RemoveLastPart=FALSE;
	
	if (!matchOpt)
		matchOpt = MOPT;
	*OutAddress = 0;
	pOutCoord->x = pOutCoord->y = 0;
	_fstrcpy (pStreet,HouseAndStreet);
	if (strstr (pStreet,"(PLOW TO CREEKSIDE)"))
		ii=1;
	ExpandText(pStreet);
	OneSpace (pStreet);
   	pStreet = GetHouseAndStreet (pStreet,HouseNumC);
	MunicNum = GetMunicFromName (City);    
	if (*HouseNumC || !*pStreet)
	{   
TryAgain:
		match = ADD_MATCH (pStreet,HouseNumC,City,ZIP,2,&hMatch,&StreetNum,&MunicNum,FALSE,TRUE,0,0,0); 
		if (!match && RemoveLastPart && nPartsRemoved < 2)
		{
			LPSTR	pSpace=_fstrrchr (pStreet,' '); 
									 	
			if (pSpace)
			{
				*pSpace = 0;  
				GSSiGlobFree (&hMatch);
				nPartsRemoved++;
				goto TryAgain;
			}
		}
	}
	else if (SeparateOnFromToStreets (pStreet,OnStreet,Street1,Street2))
		match = OFT_MATCH (OnStreet,Street1,Street2,City,0,matchOpt,&hMatch,&OnStreetNum,&StreetNum1,&StreetNum2,&MunicNum,0,0,0,0); 
	else if (SeparateIntStreets (pStreet,Street1,Street2))
		match = INT_MATCH (Street1,Street2,City,0,matchOpt,&hMatch,&StreetNum1,&StreetNum2,&MunicNum); 
	else if (FoundUserAssignedAddress (0,Street,MunicNum,FALSE,&hMatch))
		match = 1;

	if (match == 1)
	{
 		LPADDMATCH pMatch = (LPADDMATCH)GlobalLock (hMatch); 
        
        if (pMatchOut)
        	*pMatchOut = *pMatch;
		GetMunicName (pMatch->Munic,TrueCity,CityAbv);
        switch (pMatch->LocationCode) //1=House-Street, 2=Intersection match,3=Block Center,6 = User locate questionable, 7=User located, 8=User selected from list, 9=match to zip, 10=unmatchable
		{
			case 1:
				GetTrueStreetName (pMatch->StreetNum, TrueStreet1, 0,0);    
				sprintf (OutAddress,"%ld %s, %s %ld",pMatch->HouseNum,TrueStreet1,TrueCity,pMatch->ZIP); 
				*pOutCoord = pMatch->Point; 
				rtn=1;
				break;
			case 2:
				GetTrueStreetName (pMatch->StreetNum1, TrueStreet1, 0,0);    
				GetTrueStreetName (pMatch->StreetNum2, TrueStreet2, 0,0);    
				sprintf (OutAddress,"%s at %s in %s",TrueStreet1,TrueStreet2,TrueCity); 
				*pOutCoord = pMatch->Point; 
				rtn=2;
			    break;
			case 7:
				*pOutCoord = pMatch->Point; 
				rtn=7;  
				break; 
			default:
				rtn=0;
				break;
		} 
		GlobalUnlock (hMatch);
	}
	GSSiGlobFree (&hMatch);
	return rtn;
}

BOOL CreateUserAddressPlot (LPSTR OutFile)
{
    short	Type=1, i, ii;
	HANDLE	hGRText, hBT; 
	LPGRTEXT	lpGRText;  
	char	str[128];
	BTVARDESC	BTVar[2];   
	LPSHORT	pChild; 
	short	SymNum, pos=BT_FIRST;  
	long	Refno=1;
	HANDLE	hSymDesc=0;
	short	NumSyms=0; 
	MNMXCORD	Bounds;
	BOOL	rtn=FALSE;  
	DPOINT	DP={0,0}; 
	char	SymName[66];
	USERLOCATEDADDRESSKEY	ULAddKey;
	ADDMATCH	Match;  
	long	n=0; 
	BOOL	OpenedUAA;
    
	if (!OpenUserDefinedAddress (FALSE,&OpenedUAA))
		return FALSE;

	hGRText = GSSiGlobAlloc (1258,GHND,sizeof(GRTEXT));
	lpGRText = (LPGRTEXT)GlobalLock (hGRText); 
	lpGRText->version = 1;    
	lpGRText->length = sizeof(GRTEXT);
    lpGRText->FontNum = 0;
    _fstrcpy (lpGRText->cHeight,"10M"); 
   	lpGRText->hJust = 1;
	lpGRText->vJust = 3;  
	GlobalUnlock (hGRText); 
	SymNum = GetDictSymbolNumber ("CIRCLE");
	AddToSymList (SymNum,&NumSyms,&hSymDesc); 
	DBoundsInit (&Bounds);
	while (!BT_FIND (hBTUserDefinedAddress,(LPSTR)&ULAddKey,pos,BT_ANY,(LPSTR)&Match))  
	{ 
		pos = BT_NEXT;
		AddDPointToMinMax (&Match.Point,&Bounds);
	}
    pos = BT_FIRST;
	if (!CreateNewMap (OutFile,&Bounds,0,NULL,0,NULL,0,0,FALSE))
		goto Exit;  
	_fstrcpy (PltName,OutFile);
	while (!BT_FIND (hBTUserDefinedAddress,(LPSTR)&ULAddKey,pos,BT_ANY,(LPSTR)&Match))  
	{ 
		pos = BT_NEXT;
		lpGRText = (LPGRTEXT)GlobalLock (hGRText);
	    sprintf (lpGRText->Text,"%ld %s\r\n%ld",ULAddKey.HouseNum,ULAddKey.Street,ULAddKey.Munic); 
	    lpGRText->ltext = _fstrlen (lpGRText->Text)+1;
	    lpGRText->ltext += lpGRText->ltext % 2;  
	    GlobalUnlock (hGRText);
	    Refno++;
	    AddPointToMap (Match.Point,Refno,0,SymNum,10,0,NULL,hGRText,NULL,NULL,NULL,-1,-1,-1,TRUE,TRUE,NULL,NULL);
	}
	CloseUserDefinedAddress (OpenedUAA);    
	AddPointToMap (DP,0,0,0,0,0,0,0,0,0,0,0,0,0,TRUE,TRUE,NULL,NULL);
    CloseMap(TRUE);  
	AddSymToMap (NumSyms,hSymDesc,0,NULL); 
    DestroySymList (&NumSyms,&hSymDesc); 
    ForceRefIndex = ForceTAGIndex = FALSE;
	rtn = TRUE;
	 
Exit:
	GSSiGlobFree (&hGRText);
	return rtn;
}

BOOL DumpUserAdd (void)
{
	USERLOCATEDADDRESSKEY	ULAddKey;
//	ADDMATCH_ver1	Match;   
	ADDMATCH	Match;   
	long	n=0; 
	BOOL	OpenedUAA;
	short	pos=BT_FIRST;
	HFILE	Fid;   
	char	str[256];
	
	if (!OpenUserDefinedAddress (FALSE,&OpenedUAA))
		return FALSE;
	Fid = GSSiOpenFile ("[%DL]address\\userdump.txt",NULL,OF_CREATE); 
	fputstring ("STREET\tHOUSE\tMUNIC\tX\tY",Fid);
	while (!BT_FIND (hBTUserDefinedAddress,(LPSTR)&ULAddKey,pos,BT_ANY,(LPSTR)&Match))  
	{ 
		pos = BT_NEXT;
		n++;  
        sprintf (str,"%s\t%ld\t%ld\t%f\t%f",ULAddKey.Street,ULAddKey.HouseNum,ULAddKey.Munic,Match.Point.x,Match.Point.y);
     	fputstring (str,Fid);
	}
	GSSiClose2 (&Fid);
	CloseUserDefinedAddress (OpenedUAA);    
	CreateUserAddressPlot ("[%DL]maplib\\useradd.plt");
	sprintf (str,"%ld records written to %s",n,"[%DL]address\\userdump.txt");     
	ExpandText (str);
	GSSiMsgBox (0,str,"",MB_OK,0);
	return TRUE;
}

BOOL CreateMatchByAreaDB (LPSTR File,char OrigKeyType,short OrigKeyLen)
{
	char	DefStr[256];
	HANDLE	hDB;
	LPGWDHEADER lpGWDHead;  
	BOOL	rtn;
	sprintf (DefStr,"KeyValue(%c%i),TotRecords(B4),NumMatched(B4),NumNotMatched(B4),NumMultiMatched(B4),PCTMatched(R4)",OrigKeyType,OrigKeyLen);
	rtn = CreateGWDDatabase (File,1,FALSE,0,1,DefStr);  
	return rtn;
}  

BOOL CreateAddMatchByArea (LPSTR InFile,LPSTR OutFile,LPSTR AreaName)
{
	HANDLE	hDBIn, hDBOut;
	LPGWDHEADER lpGWDHead, lpGWDHeadOut;  
	long	Offset, Offset2,len;
	LPADDMATCHEDITREC pAMER; 
    LPADDMATCHBYAREA pMatchByArea; 
    short	pos=BT_FIRST; 
    long	ZIP;   
    long	TotRecs,NumRecs=0;

	if (!CreateMatchByAreaDB (OutFile,'B',4))
		return FALSE;
	hDBIn = OpenGWDatabase (InFile,BT_READ);
	if (!hDBIn)
		return FALSE;
	hDBOut = OpenGWDatabase (OutFile,BT_WRITE);
	if (!hDBOut)
		return FALSE;
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDBIn); 
	pAMER = (LPADDMATCHEDITREC)&lpGWDHead->GWDData;
	lpGWDHeadOut = (LPGWDHEADER)GlobalLock (hDBOut); 
	pMatchByArea = (LPADDMATCHBYAREA)&lpGWDHeadOut->GWDData;
    TotRecs = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[0]);
	CreateStatusWind (hWndMain,1,"Create Address Match by Area");
	while (ContinueProcessing && !BT_FIND (lpGWDHead->BTHandle[0],lpGWDHead->pKeys[1],pos,BT_ANY,(LPSTR)&Offset))
	{   
		pos = BT_NEXT;
		len = FillGWDData (lpGWDHead,Offset); 
		ZIP = atol (pAMER->ZIP);
		if (BT_FIND (lpGWDHeadOut->BTHandle[0],(LPSTR)&ZIP,BT_FIRST,BT_EQ,(LPSTR)&Offset2))
		{
			_fmemset (pMatchByArea,0,sizeof(ADDMATCHBYAREA));
			pMatchByArea->Key = ZIP;
		}
		else
			len = FillGWDData (lpGWDHeadOut,Offset2);
		if (pAMER->AM.LocationCode)
			pMatchByArea->NumMatched++;
		else
		{
			pMatchByArea->NumNotMatched++;
			if (pAMER->AM.MatchCode == 4)
				pMatchByArea->NumMultiMatched++;
		}
		pMatchByArea->TotRecords++;
		pMatchByArea->PCTMatched = (100 * (double)pMatchByArea->NumMatched)/pMatchByArea->TotRecords;
		GWDReplaceRecord (lpGWDHeadOut,0,NULL,-1); 
		NumRecs++;
		StatusWindowUpdate (NULL,NULL, TotRecs,NumRecs);
	}
	GlobalUnlock (hDBIn);
	GlobalUnlock (hDBOut);
	CloseGWDatabase (hDBIn);
	CloseGWDatabase (hDBOut);
	DestroyStatusWindow(0);  
	return TRUE;
}  
	

BOOL OpenStreetPolys (LPBOOL pOpened)
{
	char	FileName[MAX_PATH];

	*pOpened = FALSE;
	if (hStreetPolys)
		return TRUE;
	strcpy(FileName, "[%DL]maplib\\centerline\\[CLINEDATE]\\streetnumrefs.gmd");
	ExpandText(FileName);
	hDBStreetNumRefs = OpenGWDatabase(FileName, BT_READ);
	if (!hDBStreetNumRefs)
		return FALSE;
	sprintf(FileName, "[%DL]maplib\\centerline\\[CLINEDATE]\\streetpoly.in1");
	hStreetPolys = BT_OPEN (FileName,0,BT_READ,0);
	if (!hStreetPolys)
		return FALSE;
	sprintf(FileName, "[%DL]maplib\\centerline\\[CLINEDATE]\\streetpoly.bin");
	FidStreetPolys = GSSiOpenFile (FileName,NULL,OF_READ);
	*pOpened = TRUE;
	return TRUE;
}

void CloseStreetPolys (BOOL Opened)
{
	if (Opened)
	{
		BT_CLOSE (hStreetPolys);
		hStreetPolys = 0;
		GSSiClose2 (&FidStreetPolys);
		CloseGWDatabase(hDBStreetNumRefs);
	}
	return;
}

BOOL SaveStreetPolys (LPSTR dir)
{    
	short	pos = BT_FIRST;
	long	Refno;
	HIGHLIGHTDATA	HighlightData;   
	long	nPnts, Offset;
	HANDLE	hPnts, hBT;
	HPDPOINT	Points;
	HFILE	Fid;
	char	FileName[MAX_PATH];
	BTVARDESC BTVar[2]; 
	long	TotRecs, NumLoaded=0;   
	double	Length;
	BOOL wasReordered;

	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	
    sprintf (FileName,"%s\\streetpoly.in1",dir);
	BT_CREATE (FileName, 4, FALSE, 1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
    hBT = BT_OPEN (FileName,0,BT_WRITE,0); 
    sprintf (FileName,"%sstreetpoly.bin",dir);
    Fid = GSSiOpenFile (FileName,NULL,OF_CREATE);
   	TotRecs = BT_NUM_IN_INDEX (hHighlight); 
	CreateStatusWind (hWndMain,1,"Writing output file");
	while (StatusWindowUpdate(NULL, NULL, TotRecs, NumLoaded++) && !BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData))
	{   
		pos = BT_NEXT;
		if (HighlightData.PD.Type == 2)
		{
			if (GetPolyPoints ((LPPICKDATAHEADER)&HighlightData.PD,FALSE,&nPnts,&hPnts,&wasReordered))
			{  
				Points = (HPDPOINT)GlobalLock (hPnts); 
				Offset = GSSillseek (Fid,0,1);
				BigWrite (Fid,(HPSTR)&Refno,4,-1);
				BigWrite (Fid,(HPSTR)&HighlightData.PD.Desc,2,-1);
				BigWrite (Fid,(HPSTR)&nPnts,4,-1); 
				Length = GetPolyLengthD (Points,nPnts);
				BigWrite (Fid,(HPSTR)&Length,8,-1);
				BigWrite (Fid,(HPSTR)Points,nPnts*sizeof(DPOINT),-1);
				GSSiGlobUlFree (&hPnts);  
				BT_PUT (hBT,(LPSTR)&Refno,(LPSTR)&Offset);
				if (wasReordered)
				{
					//AppendFile("c:\\temp\\reordercline.txt", HighlightData.PD.UDI);
				}
			}
		}
		
	} 
	SetContinueProcessing ( TRUE); 
	DestroyStatusWindow(0);  
	GSSiClose2 (&Fid);
	BT_CLOSE (hBT);
	return TRUE;
}

static int intFac = 2;
int GetIntersectionID(LPDPOINT pt)
{
	MNMXCORD cityBounds = { 156000, 40000, 168000, 60000 };
	double xFac = (cityBounds.xmx - cityBounds.xmn) * intFac / USHRT_MAX;
	double yFac = (cityBounds.ymx - cityBounds.ymn) * intFac / USHRT_MAX;

	USHORT x = IDNINT((pt->x - cityBounds.xmn) / xFac);
	USHORT y = IDNINT((pt->y - cityBounds.ymn) / yFac);

	DWORD drtn = MAKELONG(x, y);
	int rtn = *(LPINT)&drtn;
	DPOINT dbpt = GetIntersectionPoint(rtn);
	return rtn;
}

DPOINT GetIntersectionPoint(int ID)
{
	MNMXCORD cityBounds = { 156000, 40000, 168000, 60000 };
	double xFac = (cityBounds.xmx - cityBounds.xmn) * intFac / USHRT_MAX;
	double yFac = (cityBounds.ymx - cityBounds.ymn) * intFac / USHRT_MAX;
	DPOINT rtn = { 0,0 };
	LONG dwID = *(LPLONG) &ID;
	USHORT x, y;
	x = LOWORD(dwID);
	y = HIWORD(dwID);
	rtn.y = (y * yFac) + cityBounds.ymn;
	rtn.x = (x * xFac) + cityBounds.xmn;
	return rtn;
}




	


