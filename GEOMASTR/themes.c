#include "graphint.h"
#include "extrndb.h"

#include "gmextern.h"

static	char	MinClassValue[512];


short ConvertFontHeightFromPCTofVP (int PCT,int VPID)
#if ENABLETRACE
{GSSiEnterProg (1226);
#endif
{
	LPVIEWPORT	SaveVP=CurView;
	short	size;
	long	height;
	
	if (PCT < 0)
{
#if ENABLETRACE
GSSiExitProg (1226);
#endif
		return PCT;
}
	SetViewport (VPID);
	height = (long)CurView->DrawRect.bottom - (long)CurView->DrawRect.top;
	size = 1+IDNINT ((double)(height * PCT) / 100);
	SetCurView (SaveVP);
{
#if ENABLETRACE
GSSiExitProg (1226);
#endif
	return size;
}
#if ENABLETRACE
}
#endif
}
	
short ConvertFontHeightToPCTofVP (int size,int VPID)
#if ENABLETRACE
{GSSiEnterProg (1227);
#endif
{
	LPVIEWPORT	SaveVP=CurView;
	short	PCT; 
	long	height;
	
	SetViewport (VPID);
	height = (long)CurView->DrawRect.bottom - (long)CurView->DrawRect.top;
	if (height < 1)
{
#if ENABLETRACE
GSSiExitProg (1227);
#endif
		return 6;
}
	PCT = IDNINT ((double)(size * 100) / height);
	SetCurView (SaveVP);
{
#if ENABLETRACE
GSSiExitProg (1227);
#endif
	return PCT;
}
#if ENABLETRACE
}
#endif
}
	
LPTHEME CreateNewTheme (int Choice)
#if ENABLETRACE
{GSSiEnterProg (1229);
#endif
{	HANDLE	handle;
	LPTHEME	pTheme;

	if (Choice == 25) //coordinate display
	{
		LPCOORDINATEDISPLAY CD;

		handle=GSSiGlobAlloc ( 133,GHND,sizeof(THEME));
		CD = (LPCOORDINATEDISPLAY)GlobalLock(handle);
		pTheme = (LPTHEME)CD; 
		CD->Version = 104;
		CD->TargetViewport=*pCommandViewport;;
		CD->DisplayViewport=4;
		CD->ID = PF_COORD_DISPLAY;
		CD->DisplayLine[0] = TRUE;
		CD->Units[0] = 1;
		CD->Precision[0] = 3;
		CD->Commas[0] = TRUE;
		CD->handle = handle;
        AddPassiveFun (CD);
	}
	else
	{
		handle=GSSiGlobAlloc ( 630,GHND,sizeof(THEME));
		pTheme=(LPTHEME)GlobalLock(handle);
		pTheme->handle = handle; 
		pTheme->TargetViewport = *pCommandViewport;
		pTheme->ClassType=1;   
		pTheme->RoundTo = 1;
		pTheme->MarkInvalid=TRUE;   
		pTheme->Recompute=TRUE;
		pTheme->ClassColor[0]=RGB(50,150,255);
		pTheme->ClassColor[1]=RGB(150,255,250);
		pTheme->ClassColor[2]=RGB(250,150,250);
		pTheme->ClassColor[3]=RGB(250,50,150);
		pTheme->ClassColor[4]=RGB(50,255,150);
		pTheme->ClassColor[5]=RGB(150,155,255);
		pTheme->ClassColor[6]=RGB(150,250,255);
		pTheme->ClassColor[7]=RGB(250,150,255);
		pTheme->ClassColor[8]=RGB(250,50,175);
		pTheme->ClassColor[9]=RGB(50,255,126);
		pTheme->NumDesiredClass=16;
		pTheme->XLimit = DBL_MAX;
		pTheme->YLimit = DBL_MAX;
		pTheme->Margin = 0;
		pTheme->ScatterWidth=15;
		pTheme->ColorsWidth=15;
		pTheme->InnerMargin=4;
		pTheme->TitleHeight=10;
		pTheme->BGColor=RGB(255,255,255);
		pTheme->ScatterColor=RGB(0,0,0);
		pTheme->ScatterBoxBG=RGB(255,255,255);
		pTheme->TitleBoxBG=RGB(255,255,255);
		pTheme->IBBGColor=RGB(255,255,255);  
		pTheme->FidDelayedText = pTheme->FidAreas = HFILE_ERROR;
		_fstrcpy (pTheme->TitleFont.lfFaceName,"Arial Rounded MT Bold");
		_fstrcpy (pTheme->ClassFont1.lfFaceName,"Arial");
		_fstrcpy (pTheme->ClassFont2.lfFaceName,"Arial");

		switch (Choice)
		{
			case 0: 
				pTheme->ID = GF_SINGLE_VALUE_THEME;
				break;
				
			case 1: 
				pTheme->ID = GF_SINGLE_NONNUM_VALUE_THEME; 
				pTheme->WantDataPass = FALSE; 
				pTheme->Recompute = FALSE;
				pTheme->ClassType = 3;
				pTheme->InnerMargin=8;
				break;
				
	/*		case 2:
				pTheme->ID = GF_TWO_VALUE_THEME;
				break;
				
			case 3:
				pTheme->ID = GF_COMPARATIVE_THEME;
				break;     */
				
			case 2:
				pTheme->ID = GF_NETMARKER_THEME;
				pTheme->Recompute=FALSE;
				pTheme->WantDataPass = FALSE;
				pTheme->NumDesiredClass=1; 
				pTheme->NumClass=1;
				pTheme->ClassColor[0]=RGB(0,0,255);
				pTheme->Xmove = 1;
				break;
				
			case 3:
				pTheme->ID = GF_DOCUMENTS_THEME;
				pTheme->NumDesiredClass=1; 
				pTheme->NumClass=1;
				_fstrcpy (pTheme->Title,"Document location theme");
				break;   
				
			case 4:
				pTheme->ID = GF_STREET_TEXT_THEME;
				pTheme->Recompute=FALSE;
				pTheme->WantDataPass = FALSE;
				pTheme->ClassType = 3;
				pTheme->NumDesiredClass=0; 
				pTheme->NumClass=0;
				pTheme->Xmove = 1;
				break;
				
			case 5:
				pTheme->ID = GF_STREET_ADDRESS_THEME;
				pTheme->Recompute=FALSE;
				pTheme->WantDataPass = FALSE;
				pTheme->ClassType = 3;
				pTheme->NumDesiredClass=0; 
				pTheme->NumClass=0;
				pTheme->Xmove = 2;
				pTheme->Ymove = 1;
				break;
				
			case 6:
				pTheme->ID = GF_CONTEST_THEME;
				pTheme->Recompute=FALSE;
				pTheme->NumDesiredClass=2; 
				pTheme->NumClass=2;
				_fstrcpy (pTheme->Title,"Network Connectivity");
				break;   

			case 7:
				pTheme->ID = GF_TRANSFORM_THEME;
				pTheme->Recompute=FALSE;
				pTheme->WantDataPass = FALSE;
				pTheme->NumDesiredClass=0; 
				pTheme->NumClass=0;
				break;
				
			case 8:
				pTheme->ID = GF_BOUNDS_DISPLAY_THEME;
				pTheme->Recompute=FALSE;
				pTheme->WantDataPass = FALSE;
				pTheme->NumDesiredClass=0; 
				pTheme->NumClass=0;
				break;
				
			case 9:
				pTheme->ID = GF_DISTANCE_THEME;
				pTheme->Recompute=FALSE;
				pTheme->WantDataPass = FALSE;
				pTheme->NumDesiredClass=0; 
				pTheme->NumClass=0;
				break;

			case 10:
				pTheme->ID = GF_COORDGRID_THEME;
				pTheme->Recompute=FALSE;
				pTheme->WantDataPass = FALSE;
				pTheme->NumDesiredClass=0; 
				pTheme->NumClass=0;
				break;

			case 11:
				pTheme->ID = GF_DYNAMIC_SEG_THEME;
				pTheme->Recompute=FALSE;
				pTheme->WantDataPass = FALSE;
				pTheme->NumDesiredClass=0; 
				pTheme->NumClass=0;   
				pTheme->RoundTo = 10;
				break;
				
			case 12:
				pTheme->ID = GF_PROFILE_THEME;
				pTheme->Recompute=FALSE;
				pTheme->WantDataPass = FALSE;
				pTheme->NumDesiredClass=0; 
				pTheme->NumClass=0;   
				break;
				
			case 13:
				pTheme->ID = GF_2D_THEME;
				pTheme->Recompute=FALSE;
				pTheme->WantDataPass = FALSE;
				pTheme->NumDesiredClass=0; 
				pTheme->NumClass=0;   
				break;
				
			case 14:
				pTheme->ID = GF_HOTSPOT_THEME;
				pTheme->Recompute=TRUE;
				pTheme->WantDataPass = TRUE;
				pTheme->NumDesiredClass=0; 
				pTheme->NumClass=0;
	//	        pTheme->HotSpotData.Granularity = 3;
				pTheme->HotSpotData.Radius = 1000; 
				_fstrcpy (pTheme->IconLibrary,"1.0");
				_fstrcpy (CurTheme->ClassDefValSQL,"1000.0");
	//			pTheme->HotSpotData.ColorMin = 0;
	//			pTheme->HotSpotData.ColorMax = 100;
				break;
				
			case 15:
				pTheme->ID = GF_POLYINFO_THEME;
				pTheme->Recompute=FALSE;
				pTheme->NumDesiredClass=0; 
				pTheme->NumClass=0;
				_fstrcpy (pTheme->Title,"Polygon Info");
				break;  
				 
			case 16:
				pTheme->ID = GF_GRAPHICS_FUNCTION_THEME;
				pTheme->Recompute=FALSE;
				pTheme->WantDataPass = FALSE;
				pTheme->NumDesiredClass=0; 
				pTheme->NumClass=0; 
				pTheme->ClassType = 3;  
				break;
				
			case 17:
				pTheme->ID = GF_CACHE_DISPLAY_THEME;
				pTheme->Recompute=FALSE;
				pTheme->WantDataPass = FALSE;
				pTheme->NumDesiredClass=0; 
				pTheme->NumClass=0; 
				break;
				
			case 18:
				pTheme->ID = GF_COMPARE_VIEWPORTS_THEME;
				pTheme->Recompute=FALSE;
				pTheme->WantDataPass = FALSE;
				pTheme->NumDesiredClass=0; 
				pTheme->NumClass=0;   
				break;
				
			case 19:
				pTheme->ID = GF_TIME_DISPLAY_THEME;
				pTheme->Recompute=FALSE;
				pTheme->WantDataPass = FALSE;
				pTheme->NumDesiredClass=0; 
				pTheme->NumClass=0;   
				break;
				
			case 20:
				pTheme->ID = GF_POINT_IN_AREA_THEME;
				pTheme->Recompute=FALSE;
				pTheme->WantDataPass = FALSE;
				pTheme->NumDesiredClass=0; 
				pTheme->NumClass=0;   
				break;
			case 21:
				pTheme->ID = GF_NORTH_ARROW_THEME;
				pTheme->Recompute=FALSE;
				pTheme->WantDataPass = FALSE;
				pTheme->NumDesiredClass=0; 
				pTheme->NumClass=0;   
				break;
			case 22:
				pTheme->ID = GF_PROFILE_LINK_THEME;
				pTheme->Recompute=FALSE;
				pTheme->WantDataPass = FALSE;
				pTheme->NumDesiredClass=0; 
				pTheme->NumClass=0;   
				break;
			case 23:
				pTheme->ID = GF_OFFSETAREA_THEME;
				pTheme->Recompute=FALSE;
				pTheme->WantDataPass = TRUE;
				pTheme->NumDesiredClass=0; 
				pTheme->NumClass=0;   
				break;
			case 24:
				pTheme->ID = GF_CONNECTION_LINE_THEME;
				pTheme->Recompute=FALSE;
				pTheme->WantDataPass = TRUE;
				pTheme->NumDesiredClass=0; 
				pTheme->NumClass=0;   
				break;
			case 26:
				pTheme->ID = GF_CITY_THEME;
				pTheme->Recompute = FALSE;
				pTheme->WantDataPass = FALSE;
				pTheme->NumDesiredClass = 1;
				pTheme->NumClass = 1;
				break;
			case 27:
				pTheme->ID = GF_AREA_IN_MASK_THEME;
				pTheme->Recompute = FALSE;
				pTheme->WantDataPass = FALSE;
				pTheme->NumDesiredClass=1; 
				pTheme->NumClass=1;  
				break;
				
		}
	}
	
{
#if ENABLETRACE
GSSiExitProg (1229);
#endif
	return (pTheme);
}
#if ENABLETRACE
}
#endif
}  

BOOL PickThemeClass (int iclass,POINT MousePoint)
{   
	if (CurTheme->NumCols == MAX_THEME_CLASSES)
	{ 
		HANDLE	hPoints=GSSiGlobAlloc (0,GMEM_MOVEABLE,360*sizeof(POINT));
		LPPOINT	Points = (LPPOINT)GlobalLock (hPoints);
		short	nPnts;
		BOOL	rtn;
		    			
		nPnts = GetPieSlice (CurTheme->RadiusPoint,CurTheme->Radius,CurTheme->NumClass,iclass,Points);
		Polygon (CurView->hDC,Points,nPnts); 
		rtn = POINT_IN_AREA (MousePoint, nPnts, Points);
		GSSiGlobUlFree (&hPoints); 
		return rtn;
    }
    else
		return (PtInRect(&CurTheme->ClassClrBox[iclass],MousePoint));
}

void DrawUnSelectedClass (int iclass,RECT ClassClrBox)
{
	if (CurTheme->NumCols == MAX_THEME_CLASSES)
	{ 
		HANDLE	hPoints=GSSiGlobAlloc (0,GMEM_MOVEABLE,360*sizeof(POINT));
		LPPOINT	Points = (LPPOINT)GlobalLock (hPoints);
		short	nPnts;
		LOGBRUSH    NDB;
		HBRUSH	hBrush, OldBrush;
				    			
		nPnts = GetPieSlice (CurTheme->RadiusPoint,CurTheme->Radius,CurTheme->NumClass,iclass,Points);
        NDB.lbStyle = BS_HATCHED;
        NDB.lbColor = HighlightColor;
        NDB.lbHatch = HS_DIAGCROSS;
        hBrush =  CreateBrushIndirect(&NDB);  
        OldBrush = SelectObject (CurView->hDC,hBrush);
		Polygon (CurView->hDC,Points,nPnts); 
        SelectObject (CurView->hDC,OldBrush);
        GSSiDeleteObject (&hBrush);
		GSSiGlobUlFree (&hPoints); 
    }
    else
	{
		RECT	Rect = ClassClrBox;
		InflateRect (&Rect,2,2);
		{
			HPEN	hPen = CreatePen (PS_SOLID,3,RGB(255,255,255));
			HPEN	hOldPen = SelectObject (CurView->hDC,hPen);  
					
			MoveToEx (CurView->hDC,Rect.left,Rect.bottom,0);
			LineTo (CurView->hDC,Rect.right,Rect.top);
			MoveToEx (CurView->hDC,Rect.left,Rect.top,0);
			LineTo (CurView->hDC,Rect.right,Rect.bottom);
			SelectObject (CurView->hDC,hOldPen);
			DeleteObject (hPen);
		}
		{
			HPEN	hPen = CreatePen (PS_SOLID,1,RGB(255,0,0));
			HPEN	hOldPen = SelectObject (CurView->hDC,hPen);  

			MoveToEx (CurView->hDC,Rect.left,Rect.bottom,0);
			LineTo (CurView->hDC,Rect.right,Rect.top);
			MoveToEx (CurView->hDC,Rect.left,Rect.top,0);
			LineTo (CurView->hDC,Rect.right,Rect.bottom);
			SelectObject (CurView->hDC,hOldPen);
			DeleteObject (hPen);
		}
	}
	return;
}

void ThemeSVChangeClr (POINT MousePoint)
#if ENABLETRACE
{GSSiEnterProg (1230);
#endif
{	HBRUSH	BkBrush;
	int		iclass;
	BOOL	Cancel; 
	LPVIEWPORT	SaveVP=CurView; 
	char	str[128];

	if (!CurView->pTheme)
{
#if ENABLETRACE
GSSiExitProg (1230);
#endif
		return;
}
	CurTheme = CurView->pTheme;
	if (!CurTheme->IsActive)
{
#if ENABLETRACE
GSSiExitProg (1230);
#endif
		return;
}
	for (iclass=0;iclass<CurTheme->NumClass;iclass++)
	{	
		if (PickThemeClass (iclass,MousePoint)) 
		{
			switch (CurTheme->DataType)
			{
				case 2:
		        {
			    	DLGPROC lpfnLINETYPEMsgProc; 
			    	int		nRc,R,G,B,W;
				    	
					setDoPaint( FALSE);  
					SetViewport(CurTheme->TargetViewport);
					NewColor = CurTheme->ClassColor[iclass];   
					NewWidth = CurTheme->ClassFactor[iclass];
					INewWidth = CurTheme->ClassFactor[iclass];
					if (INewWidth > 128)
						INewWidth -= 256;
					if (INewWidth > 0)
						INewWidth--;
			        lpfnLINETYPEMsgProc = MakeProcInstance((DLGPROC)LINETYPEMsgProc, hInst);
			        nRc = DialogBox(hInst, (LPSTR)"LINETYPE", CurView->hWnd, lpfnLINETYPEMsgProc);
			        FreeProcInstance(lpfnLINETYPEMsgProc);
					setDoPaint( TRUE);   
					SetCurView (SaveVP);
			        if (nRc)
			        {
						CurTheme->ClassColor[iclass]=NewColor;
						R = GetRValue (NewColor);
						G = GetGValue (NewColor);
						B = GetBValue (NewColor);
						W = max (-127,IDNINT(NewWidth));
						if (NewWidth < 0) 
							CurTheme->ClassFactor[iclass] = NewWidth; 
						else
							CurTheme->ClassFactor[iclass] = NewWidth+1;
						CurTheme->ClassColor[iclass]=RGB(R,G,B);
						FillRectPoly (CurView->hDC,&CurTheme->ClassClrBox[iclass],ConvertColor(ColorWOWidth (CurTheme->ClassColor[iclass]),CurTheme->UseHalfTone));
			        }
		        } 
		        break;
		        
		        case 3:
		        {   
		        	sprintf (str,"%f",CurTheme->ClassFactor[iclass]);
                    if (!GetTextString (GetFocus(),str,32,"Enter text factor",0,0,0,TRUE,TRUE))
                    	break;
                    CurTheme->ClassFactor[iclass] = atof (str);
		        	NewColor = CurTheme->ClassColor[iclass];
					if (GetColor(hWndMain,&NewColor))
					{   
						CurTheme->ClassColor[iclass]=NewColor;
						FillRectPoly (CurView->hDC,&CurTheme->ClassClrBox[iclass],ConvertColor(ColorWOWidth (CurTheme->ClassColor[iclass]),CurTheme->UseHalfTone));
					} 
				} 
		        break;
		        
		        default:
		        {
		        	NewColor = CurTheme->ClassColor[iclass];
					if (GetColor(hWndMain,&NewColor))
					{   
						CurTheme->ClassColor[iclass]=NewColor;
						FillRectPoly (CurView->hDC,&CurTheme->ClassClrBox[iclass],ConvertColor(ColorWOWidth (CurTheme->ClassColor[iclass]),CurTheme->UseHalfTone));
					} 
				} 
			}
{
#if ENABLETRACE
GSSiExitProg (1230);
#endif
			return;
}
		}
	}
	if (PtInRect(&CurTheme->TitleBox,MousePoint)) 
	{   
		NewColor = CurTheme->TitleBoxBG;
		if (GetColor(hWndMain,&NewColor))
		{   
			CurTheme->TitleBoxBG=NewColor;
			FillRectPoly (CurView->hDC,&CurTheme->TitleBox,CurTheme->TitleBoxBG);
		}  
	}
	else if (PtInRect(&CurTheme->InfoBox,MousePoint)) 
	{   
		NewColor = CurTheme->IBBGColor;
		if (GetColor(hWndMain,&NewColor))
		{   
			CurTheme->IBBGColor=NewColor;
			FillRectPoly (CurView->hDC,&CurTheme->InfoBox,CurTheme->IBBGColor);
		}  
	}
	else 
	{   
		NewColor = CurTheme->BGColor;
		if (GetColor(hWndMain,&NewColor))
		{   
			CurTheme->BGColor=NewColor;
			FillRectPoly (CurView->hDC,&CurTheme->Rect,CurTheme->BGColor);
		}  
	}
{
#if ENABLETRACE
GSSiExitProg (1230);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}


 




 

BOOL ThemeRunMacro (HWND hWnd, int Message, WPARAM wParam,LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1233);
#endif
{
 POINT	MousePoint;
 static	HaveDown=FALSE;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		HaveDown=TRUE;
   		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    	
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
    	NumPicked = ClassMacro (MousePoint,1,0,0,0);
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (1233);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1233);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

short ClassMacro (POINT MousePoint,short opt,LPHANDLE phBox,LPVIEWPORT *ppVP,LPHANDLE phCmd)
#if ENABLETRACE
{GSSiEnterProg (1234);
#endif
{	 
	short	iclass, colw, vpid; 
	LPSTR	pCmd, pAmp, pCarrot;
	LPSTR	pSemiColon;
   	LPSTR	pDesc;
	LPVIEWPORT	SaveVP = CurView;   
	RECT	ClassRect; 
	char	VPName[34];
	LPSAVESCREEN	pSaveScreen;
// 6/28/03 to fix hanging yellow box prob    *ppVP = 0;
	if (!CurView->pTheme)
{
#if ENABLETRACE
GSSiExitProg (1234);
#endif
		return 0;
}
	CurTheme = CurView->pTheme;
	if (!CurTheme->IsActive)
{
#if ENABLETRACE
GSSiExitProg (1234);
#endif
		return 0;
}
	colw = (CurTheme->InfoBox.right - CurTheme->InfoBox.left + 1) / (CurTheme->NumCols + 1);
	for (iclass=0;iclass<CurTheme->NumClass;iclass++)
	{	
		ClassRect = CurTheme->ClassClrBox[iclass];
		ClassRect.right = ClassRect.left + colw - CurTheme->ActualXMargin;
		if (PtInRect(&ClassRect,MousePoint)) 
		{   
			if (CurTheme->ID == GF_GRAPHICS_FUNCTION_THEME)
			{   

		    	if (*phBox) 
		    	{   
		    		short	UserID;
		    		
		    		pSaveScreen = (LPSAVESCREEN)GlobalLock (*phBox); 
		    		UserID = pSaveScreen->UserID; 
		    		GlobalUnlock (*phBox);
		    		if (UserID == iclass+1)
		    			goto Exit;
		    		else
		    		{
			    		RestoreScreen2 (CurView->hDC, *phBox,0,FALSE);
			    		DestroySavedScreen (phBox,0); 
			    	}
		    	}
		 		if ((pSemiColon = _fstrchr (CurTheme->ClassBM[iclass],';')))
		 			*pSemiColon = 0;
			    if ((pDesc=_fstrchr (CurTheme->ClassBM[iclass],'/')))
			    	*pDesc = 0;
		    	if ((pAmp = _fstrchr (CurTheme->ClassBM[iclass],'&')))
		    		*pAmp = 0;  
		    	if ((pCarrot = _fstrchr (CurTheme->ClassBM[iclass],'^')))
		    		*pCarrot = 0; 
		    	if (!CurTheme->ColorsWidth) 
		    	{   
		    		if (CurTheme->ClassCount[iclass])
                    {   
                    	
			    		SaveDC (CurView->hDC);
					    SetDisplayMode (CurView->hDC, GF_TEXTMODE); 
					    if (pDesc) 
					    {
					    	HANDLE hTmp=GSSiGlobAlloc ( 631,GMEM_MOVEABLE,1024);
					    	LPSTR pTmp=GlobalLock (hTmp); 
					    	LPSTR	pEnd;
					    	
					    	_fstrcpy (pTmp,pDesc+1);
					    	if ((pEnd = _fstrchr (pTmp,'/')))
					    		*pEnd = 0;
							*phBox = YellowTextBox (CurView->hWnd,pTmp,MousePoint,0,0,FALSE,0);
							GSSiGlobUlFree (&hTmp);
						} 
					    else 
					    {
					    	*phBox = SaveScreen2 (CurView->hWnd,CurView->hDC,ClassRect,0,0);
					  		SelectClipRgn (CurView->hDC,0);
			    			InvertRect (CurView->hDC,&ClassRect); 
			    			if (pSemiColon)
			    			{
			    				SetGlobalValue("%P",(pSemiColon+1));
			    			}
			    			else
			    				SetGlobalValue("%P","");
			    		}                           
						RestoreDC (CurView->hDC,-1);
					}
		    	}
		    	else
//					*phBox = YellowTextBox (CurView->hWnd,CurTheme->ClassBM[iclass],MousePoint,0,0,FALSE,4);  
					*phBox = YellowTextBox (CurView->hWnd,CurTheme->ClassBM[iclass],RectMid(&ClassRect),0,0,FALSE,4);  
	    		if (*phBox)
	    		{
		    		pSaveScreen = (LPSAVESCREEN)GlobalLock (*phBox);
		    		pSaveScreen->UserID = iclass+1;
	                GlobalUnlock (*phBox); 
	            }
				if (ppVP)
					*ppVP = CurView;
				if (pAmp)
					*pAmp = '&'; 
				if (pCarrot)
					*pCarrot = '^';
				if (pDesc)
					*pDesc = '/';
				if (pSemiColon)
					*pSemiColon = ';';
				if (!*phCmd)
					*phCmd = GSSiGlobAlloc ( 632,GMEM_MOVEABLE,256);
				pCmd = GlobalLock (*phCmd);
				*pCmd = 0; 
				vpid=SelectViewport (CurrentLBUTDOWNLoc,FALSE,FALSE,TRUE);
				if (CurrentConfig && vpid)
				{
			    	SetViewport (vpid);  
			    	_fstrcpy (VPName,CurView->Name);
					SetCurView (SaveVP);
				} 
				else if (CurrentConfig)
				{
			    	SetViewport (CurTheme->TargetViewport);  
			    	_fstrcpy (VPName,CurView->Name);
					SetCurView (SaveVP);
				} 
				else
					_fstrcpy (VPName,"#CMD"); 
				LastMenuConfig = CurrentConfig;
	        	LastMenuVPID = CurView->ID;
			    if (CurTheme->ClassCount[iclass])
					sprintf (pCmd,"$GFCMD(%s,%s,%ld,N,2)",VPName,CurTheme->SQL,labs(CurTheme->ClassCount[iclass])-1);  
				GlobalUnlock (*phCmd);
/*				HMENU	ApMenu = CreatePopupMenu(); 
				POINT	position;
			    UINT	CmdID=63800; 
				AppendMenu (ApMenu,MF_ENABLED|MF_STRING,CmdID,CurTheme->ClassBM[iclass]); 
			   	GetCursorPos (&position);  
		  		TrackPopupMenu (ApMenu,TPM_CENTERALIGN|TPM_LEFTBUTTON,position.x,position.y,0,CurView->hWnd,0);
			  	DestroyMenu (ApMenu); */
Exit:
{
#if ENABLETRACE
GSSiExitProg (1234);
#endif
				return 0;
}
			}
			SetGlobalValueLong ("%CLASSNO",iclass+1); 
			SetGlobalValue ("%CLASSID",CurTheme->ClassBM[iclass]); 
			PickList[0].Desc = -1;
			PickList[0].ViewID = CurView->ID;
			PickList[0].ConfigID = CurrentConfig;
			PickList[0].Type = 1;
			PickList[0].Refno = iclass+1;
			RectToBounds (&ClassRect,&PickList[0].Rect);
			SetGlobalValueBounds ("%CLASSBOUNDS",&PickList[0].Rect);
/*			if (debugvalue > -1)
				debugvalue++;
			if (debugvalue == 3)
				debugvalue = -5;*/
			PickList[0].PickedPoint = PickList[0].BeginPoint = PickList[0].EndPoint = PointToDPoint(MousePoint);
			_fstrcpy (PickList[0].Prefix,"%CLASSYM");
			_fstrcpy (PickList[0].UDI,CurTheme->ClassBM[iclass]);
{
#if ENABLETRACE
GSSiExitProg (1234);
#endif
			return 1;
}
		}
	}
	if (PtInRect(&CurTheme->TitleBox,MousePoint)) 
	{       
			if (CurTheme->ID == GF_GRAPHICS_FUNCTION_THEME)
			{
		    	if (*phBox) 
		    	{   
		    		short	UserID;
		    		
		    		pSaveScreen = (LPSAVESCREEN)GlobalLock (*phBox); 
		    		UserID = pSaveScreen->UserID; 
		    		GlobalUnlock (*phBox);
		    		if (UserID == iclass+1)
		    			goto Exit;
		    		else
		    		{
			    		RestoreScreen2 (CurView->hDC, *phBox,0,FALSE);
			    		DestroySavedScreen (phBox,0); 
			    	}
		    	}
				if (opt == 2)  
				{
					*phBox = YellowTextBox (CurView->hWnd,"Change Function List",MousePoint,0,0,FALSE,0);  
		    		pSaveScreen = (LPSAVESCREEN)GlobalLock (*phBox);
		    		pSaveScreen->UserID = iclass+1;
	                GlobalUnlock (*phBox);
					if (!*phCmd)
						*phCmd = GSSiGlobAlloc ( 633,GMEM_MOVEABLE,256);
					pCmd = GlobalLock (*phCmd);
					*pCmd = 0; 
					sprintf (pCmd,"$GFLIST(THEME,%s)",CurView->Name);  
					GlobalUnlock (*phCmd);
					if (ppVP)
						*ppVP = CurView;
{
#if ENABLETRACE
GSSiExitProg (1234);
#endif
					return 0;
}  
				}
				else
				{ 
					DisplayGFList ();  
{
#if ENABLETRACE
GSSiExitProg (1234);
#endif
					return 0;
}
				}
			}
			PickList[0].Desc = -1;
			PickList[0].Refno = 0;
			PickList[0].ViewID = CurView->ID;
			PickList[0].ConfigID = CurrentConfig;
			PickList[0].PickedPoint = PickList[0].BeginPoint = PointToDPoint(MousePoint);
			_fstrcpy (PickList[0].Prefix,"%THEMETITLE");
			_fstrcpy (PickList[0].UDI,CurView->Name);
{
#if ENABLETRACE
GSSiExitProg (1234);
#endif
			return 1;
}
	}
	else if (PtInRect(&CurTheme->InfoBox,MousePoint)) 
	{
			PickList[0].Desc = -1;
			PickList[0].ViewID = CurView->ID;
			PickList[0].ConfigID = CurrentConfig;
			PickList[0].Refno = -1;
			PickList[0].PickedPoint = PickList[0].BeginPoint = PointToDPoint(MousePoint);
			_fstrcpy (PickList[0].Prefix,"%THEMEBODY");
			_fstrcpy (PickList[0].UDI,CurView->Name);
{
#if ENABLETRACE
GSSiExitProg (1234);
#endif
			return 1;
}
	}
	else 
	{
	}
{
#if ENABLETRACE
GSSiExitProg (1234);
#endif
	return 0;
}
#if ENABLETRACE
}
#endif
}

BOOL DisplayVPThemeLegends (void)
#if ENABLETRACE
{GSSiEnterProg (1235);
#endif
{
	int	itheme, iclass; 
    LPTHEME SaveTheme=CurTheme;
    LPVIEWPORT	SaveVP=CurView;
     
                       
	for (itheme=0;itheme<CurView->NumThemes;itheme++)
	{   
		if (!CurView->pThemes[itheme])
			goto NextTheme;
		CurTheme=CurView->pThemes[itheme]; 
		if (VPIsActive (CurTheme->DisplayViewport))
			ThemeDisplayLegend2(2,SaveVP->ID);
NextTheme:; 
	}
	SetCurView (SaveVP);
{
#if ENABLETRACE
GSSiExitProg (1235);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL DisplayVPThemeDelayedAndInfobox (void)
#if ENABLETRACE
{GSSiEnterProg (1235);
#endif
{
	int	itheme, iclass; 
    LPTHEME SaveTheme=CurTheme;
    LPVIEWPORT	SaveVP=CurView;
     
                       
	for (itheme=0;itheme<CurView->NumThemes;itheme++)
	{   
		if (!CurView->pThemes[itheme])
			goto NextTheme;
		CurTheme=CurView->pThemes[itheme];
		if (VPIsActive (CurTheme->DisplayViewport))
			ThemeDisplayLegend2(5,SaveVP->ID);
NextTheme:; 
	}
	SetCurView (SaveVP);
{
#if ENABLETRACE
GSSiExitProg (1235);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

void ThemeDisplayLegend(short BeginOrEndDisplayPass,short FromVPID)
#if ENABLETRACE
{GSSiEnterProg (1236);
#endif
{   
	LPTHEME	SaveTheme = CurTheme;
	
	if (!CurView->pTheme || ComputePCTTheme)
{
#if ENABLETRACE
GSSiExitProg (1236);
#endif
		return;
}
	CurTheme = CurView->pTheme;  
	ThemeDisplayLegend2(BeginOrEndDisplayPass,FromVPID);
	CurTheme = SaveTheme;
{
#if ENABLETRACE
GSSiExitProg (1236);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void ThemeDisplayLegend2(short BeginOrEndDisplayPass,short FromVPID)
#if ENABLETRACE
{GSSiEnterProg (1237);
#endif
{   
	LPVIEWPORT	SaveView = CurView;  
	short	ii;
	BOOL	SaveInDisplayProcessing=InDisplayProcessing;

	InDisplayProcessing = TRUE;
	
	if (!(VPIsActive (CurTheme->DisplayViewport) || CurTheme->ID == PF_COORD_DISPLAY) || !CurTheme->VPDisplayed)
		goto Exit;
	
	if (CurTheme->TargetViewport > 0 && CurTheme->TargetViewport <= *pNumViewports)
	{
		if (!pViewports[CurTheme->TargetViewport-1]->Active)
			goto Exit;
	}
	else		
		goto Exit;
//	SetCurView (pViewports[CurTheme->DisplayViewport-1]); //tempdebu
	SetViewport (CurTheme->DisplayViewport);
	ClearFullWindowBitmap (0);
	switch (CurTheme->ID)
	{   
        case GF_CONTEST_THEME: 
        case GF_DYNAMIC_SEG_THEME:
        case GF_POLYINFO_THEME: 
		case GF_PROFILE_LINK_THEME:
		case GF_CITY_THEME:
			break;

		case GF_STREET_TEXT_THEME: 
			if (BeginOrEndDisplayPass == 2)
				DisplayStreetText();
			break;
			
		case GF_STREET_ADDRESS_THEME:
			if (BeginOrEndDisplayPass != 2) break;
			if (DisplayStreetAddresses(TRUE))
			{
				if (InImediate)
					DisplayFinOpt = 5;
				else 
				{
					StreetEditTimer = STREETEDITTIMERID;
					SetTimer(hWndMain, STREETEDITTIMERID, 1, (TIMERPROC) 0);  
				}
			}
			break;
			
        case GF_CACHE_DISPLAY_THEME:
        	CacheDisplayTheme (BeginOrEndDisplayPass);
        	break;

		case GF_CONNECTION_LINE_THEME:			
		case GF_SINGLE_NONNUM_VALUE_THEME: 
		{
			if (BeginOrEndDisplayPass == 1)
			{
				LPTHEME		SaveTheme = CurTheme;

				ThemeBeginDisplayPass(FALSE,CurView->ID);
				CurTheme = SaveTheme;
			}
		}
        case GF_GRAPHICS_FUNCTION_THEME: 
//        	if (BeginOrEndDisplayPass && BeginOrEndDisplayPass != 3)
//        		break;  
		case GF_SINGLE_VALUE_THEME:
		case GF_POINT_IN_AREA_THEME:
		case GF_OFFSETAREA_THEME:
			DisplaySVThemeLegend(BeginOrEndDisplayPass);
			break;
		case GF_AREA_IN_MASK_THEME:
			break;
		case GF_TWO_VALUE_THEME:
//			DisplayTwoVThemeLegend();
			break;

		case GF_MULT_BITMAPS_THEME:
			DisplayBMThemeLegend();
			break;  
			
		case GF_PROFILE_THEME:
			DisplayProfileThemeLegend(BeginOrEndDisplayPass);
			break;  
			
		case GF_BOUNDS_DISPLAY_THEME:  
//020924			BoundsDisplayTheme2 (FALSE);   
			break;
			
        case GF_TRANSFORM_THEME: 
        	DisplayTransformThemeLegend(BeginOrEndDisplayPass);
			break;
		
        case GF_HOTSPOT_THEME: 
        	DisplayHotSpotThemeLegend(BeginOrEndDisplayPass);
			break;
		
		case GF_DISTANCE_THEME:
			DisplayDistanceThemeLegend (1,0,0,0);
			break;

		case GF_NORTH_ARROW_THEME:
			DisplayNorthArrowLegend (BeginOrEndDisplayPass);
			break;
							 
		case GF_COORDGRID_THEME:
			DisplayCoordGridThemeLegend (BeginOrEndDisplayPass);
			break;
		
		case GF_TIME_DISPLAY_THEME:
			DisplayTimeLegend (BeginOrEndDisplayPass);	  
			break;
			
		case GF_2D_THEME:
			Display2DThemeLegend (BeginOrEndDisplayPass);	  
			break;
			
		case GF_COMPARE_VIEWPORTS_THEME:
			CompareViewportsThemeLegend (BeginOrEndDisplayPass,FromVPID);	  
			break;
			
		case PF_COORD_DISPLAY: 
			if (Printing && BeginOrEndDisplayPass == 3)
				DisplayCoordPrintText ();
			break;
							 
		case GF_NETMARKER_THEME:
			if (BeginOrEndDisplayPass != 2) break;
			if (DisplayNetMarkerThemeLegend(TRUE)) 
			{
				if (InImediate)
					DisplayFinOpt = 4;
				else 
				{
					NetMarkTimer = NETMARKERTIMERID;
					SetTimer(hWndMain, NETMARKERTIMERID, 1, (TIMERPROC) 0);  
				}
			}
			break;
	}
	DisplayCloseIcon (); 
	SetCurView (SaveView);
	SaveFullWindowBitmap (CurView->hWnd);
Exit:
	InDisplayProcessing = SaveInDisplayProcessing;
{
#if ENABLETRACE
GSSiExitProg (1237);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}








HANDLE CreateClassValueList(HANDLE hBTOld)
#if ENABLETRACE
{GSSiEnterProg (1241);
#endif
{

	BTVARDESC	BTVar[3];
	int	i, ifield, len;
	HANDLE	hBT;
						
	BT_CLOSEANDDELETE (&hBTOld);
	if (CurTheme->FieldFun || CurTheme->MultiValOption == 4)
		len =  128;
	else     
		len = GetValListFieldLen (&CurTheme->Field);
	hBT = CreateUniqueList (len,CurTheme->ScatterFile); 
	CurTheme->NextValueColor = 0;

{
#if ENABLETRACE
GSSiExitProg (1241);
#endif
	return hBT;
}
#if ENABLETRACE
}
#endif
}                                             

short GetValListFieldLen (LPFIELDINFO pField)
#if ENABLETRACE
{GSSiEnterProg (1242);
#endif
{   
	short	len;
	
	switch (pField->type)
	{
        case SQL_SMALLINT:
        case SQL_INTEGER:  
        case SQL_TINYINT: 
        case SQL_BIT:
		case BT_INTEGER:
			len = 12;
			break;
      	case SQL_DECIMAL:         
      	case SQL_NUMERIC:         
      	case SQL_REAL:         
      	case SQL_FLOAT:         
      	case SQL_DOUBLE:
		case BT_REAL:
			len = 20;
			break;
		default:
			len = pField->length;
	} 
	if (!len)
		len = 32;
{
#if ENABLETRACE
GSSiExitProg (1242);
#endif
	return len;
}
#if ENABLETRACE
}
#endif
}


    

    
BOOL SaveCurTheme (HWND hWnd)
#if ENABLETRACE
{GSSiEnterProg (1245);
#endif
{ 
	char	Ext[16], SaveChr;
	char	InitDir[128], SaveName[128], str[128]; 
	int		SaveDrive;
	HFILE	Fid;
	BOOL	rtn=FALSE; 
	
	if (GetSaveName2 (hWnd,SaveName,IDS_FILTERTHEMES,".THM",IDS_FILETHM))
	{
		 Fid = GSSiOpenFile (SaveName,0,OF_CREATE);  
		 WriteCurTheme (Fid);
	 	 GSSiClose (Fid);
	 	 rtn =TRUE;
	}
{
#if ENABLETRACE
GSSiExitProg (1245);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
} 

BOOL WriteCurTheme (HFILE Fid)
#if ENABLETRACE
{GSSiEnterProg (1246);
#endif
{
	HANDLE	hBT, SaveHandle;
	char	SaveChr;
	
	switch (CurTheme->ID)
	{   
		case 0:
{
#if ENABLETRACE
GSSiExitProg (1246);
#endif
			return FALSE;
}
		case PF_COORD_DISPLAY: 
			CurTheme->Version = 104;
			BigWrite (Fid,(HPSTR)CurTheme,sizeof(COORDINATEDISPLAY),-1);
			break;
		
		case PF_BOUNDS_DISPLAY: 
			CurTheme->Version = CUR_THEME_VERSION;
			BoundsDisplayWrite (CurTheme,Fid);
			break;
		
		default:
		
		CurTheme->Version = CUR_THEME_VERSION;
		SaveChr = CurTheme->ScatterFile[0];
		if (CurTheme->ID == GF_SINGLE_NONNUM_VALUE_THEME) 
		{
			if (!CurTheme->hScatterFile && CurTheme->ScatterFile[0])  
				CurTheme->hScatterFile = BT_OPEN (CurTheme->ScatterFile,0, BT_READ, 0);
			hBT = CurTheme->hScatterFile; 
			CurTheme->ScatterFile[0]=0; 
		}
		else 
		{
			CurTheme->ScatterFile[0]=0; 
			hBT = 0;
			SaveChr = 0;  
		}
		CurTheme->hScatterFile = 0; 
		SaveHandle = CurTheme->hDisperseFileName;
		CurTheme->hDisperseFileName = 0;				 
		BigWrite (Fid,(HPSTR)CurTheme,sizeof(THEME),-1); 
		CurTheme->ScatterFile[0] = SaveChr;
		CurTheme->hDisperseFileName = SaveHandle;
		if (CurTheme->ID == GF_SINGLE_NONNUM_VALUE_THEME)
		{
			short	lrec=0;
			
			if (hBT)
			{                             
				HPSTR	pRecs;
				HANDLE	hRecs;
				int		ClassNo, pos;  
				LPINT	lpint;
				long	NumRecs;
							
				NumRecs = BT_NUM_IN_INDEX (hBT);                    
				BigWrite(Fid,(HPSTR)&NumRecs,4,-1);
				lrec = GetBTKeyLen(hBT);
				BigWrite(Fid,(HPSTR)&lrec,2,-1);
				if (NumRecs)
				{   
					hRecs = GSSiGlobAlloc (1748,GMEM_MOVEABLE,NumRecs*(4+lrec));
					pRecs = GlobalLock(hRecs);  
					pos = BT_FIRST;
					while (!BT_FIND(hBT,pRecs,pos,BT_ANY,(LPSTR)&ClassNo))
					{
						pos = BT_NEXT;
						pRecs += lrec;
						lpint = (LPINT) pRecs;
						*lpint++ = ClassNo;
						pRecs = (LPSTR) lpint;
									
					}  
					GlobalUnlock(hRecs);
					pRecs = GlobalLock(hRecs);
					BigWrite (Fid,pRecs,NumRecs*(4+lrec),-1);
					GSSiGlobUlFree (&hRecs); 
				}
				BT_CLOSE (hBT);
			}
			else
			{
				long NumRecs=0;

				BigWrite(Fid,(HPSTR)&lrec,2,-1);
				BigWrite(Fid,(HPSTR)&NumRecs,4,-1);
            }
		}
		if (CurTheme->ID == GF_DOCUMENTS_THEME)
		{
			if (hDesiredDocs)
			{                             
				int		NumDocs;
				LPSHORT	pNumDocs;
				LPSTR	pName;
				
				pNumDocs = (LPSHORT)GlobalLock (hDesiredDocs);  
				NumDocs = *pNumDocs;
				BigWrite(Fid,(HPSTR)&NumDocs,sizeof(short),-1);
				pNumDocs++;
				pName = (LPSTR)pNumDocs; 
				BigWrite (Fid,pName,NumDocs*14,-1);
				GlobalUnlock (hDesiredDocs);
			}
		} 
		break;
	}
{
#if ENABLETRACE
GSSiExitProg (1246);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}  

BOOL SetThemeColorsFromScheme (void)
{   
	int	R1,R2,G1,G2,B1,B2;
	short	i; 
	COLORREF	StartColor[5]={RGB(240,240,240),RGB(255,240,240),RGB(240,255,240),RGB(240,240,255),RGB(255,240,240)};
	COLORREF	EndColor[5]  ={RGB(64,64,64),RGB(255,64,64),RGB(64,255,64),RGB(64,64,255),RGB(64,64,255)};
	COLORREF	R,G,B;
	
	if (CurTheme->ColorScheme <= 0 || CurTheme->ColorScheme > 5|| !CurTheme->NumClass)
		return FALSE; 
	if (CurTheme->ColorScheme == 5)
	{
		R1 = GetRValue (StartColor[1]);
		R2 = GetRValue (EndColor[4]);
		G1 = GetGValue (StartColor[1]);
		G2 = GetGValue (EndColor[4]);
		B1 = GetBValue (StartColor[1]);
		B2 = GetBValue (EndColor[4]);
		for (i=0;i<CurTheme->NumClass;i++) 
		{   
			R = R1 + (i * (R2 - R1))/CurTheme->NumClass;
			G = G1 + (i * (G2 - G1))/CurTheme->NumClass;
			B = B1 + (i * (B2 - B1))/CurTheme->NumClass;
			CurTheme->ClassColor[i] = RGB(R,G,B);   
		} 
	}
	else
	{
		R1 = GetRValue (StartColor[CurTheme->ColorScheme-1]);
		R2 = GetRValue (EndColor[CurTheme->ColorScheme-1]);
		G1 = GetGValue (StartColor[CurTheme->ColorScheme-1]);
		G2 = GetGValue (EndColor[CurTheme->ColorScheme-1]);
		B1 = GetBValue (StartColor[CurTheme->ColorScheme-1]);
		B2 = GetBValue (EndColor[CurTheme->ColorScheme-1]);
		for (i=0;i<CurTheme->NumClass;i++) 
		{   
			R = R1 + (i * (R2 - R1))/CurTheme->NumClass;
			G = G1 + (i * (G2 - G1))/CurTheme->NumClass;
			B = B1 + (i * (B2 - B1))/CurTheme->NumClass;
			CurTheme->ClassColor[i] = RGB(R,G,B);   
		} 
	}
	return TRUE;
}



void SetFieldCorrectionOpts (HWND hWndDlg,int Type)
#if ENABLETRACE
{GSSiEnterProg (1248);
#endif
{
	SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_RESETCONTENT,0,0);
	SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_ADDSTRING,0,(LPARAM)((LPSTR)"Unadjusted")); 
	switch (Type)
	{
		case 0:
		SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_ADDSTRING,0,(LPARAM)((LPSTR)"Per Square Foot")); 
		SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_ADDSTRING,0,(LPARAM)((LPSTR)"Per Square Meter")); 
		SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_ADDSTRING,0,(LPARAM)((LPSTR)"Per Square Yard")); 
		SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_ADDSTRING,0,(LPARAM)((LPSTR)"Per Square Mile")); 
		SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_ADDSTRING,0,(LPARAM)((LPSTR)"Per Square Kilometer")); 
		SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_ADDSTRING,0,(LPARAM)((LPSTR)"Per Acre")); 
		break; 
		case 1:
		break; 
		case 2:
		SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_ADDSTRING,0,(LPARAM)((LPSTR)"Per Foot")); 
		SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_ADDSTRING,0,(LPARAM)((LPSTR)"Per Meter")); 
		SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_ADDSTRING,0,(LPARAM)((LPSTR)"Per Yard")); 
		SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_ADDSTRING,0,(LPARAM)((LPSTR)"Per Mile")); 
		SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_ADDSTRING,0,(LPARAM)((LPSTR)"Per Kilometer")); 
		break; 
	}
	SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_SETCURSEL,CurTheme->FieldCorrection,0);
{
#if ENABLETRACE
GSSiExitProg (1248);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

short EditTheme (HWND hWnd,UINT StartCmd)
#if ENABLETRACE
{GSSiEnterProg (1251);
#endif
{	short nRc;
    
    if (!CurTheme)
{
#if ENABLETRACE
GSSiExitProg (1251);
#endif
    	return 0;
}
    ThemeEditStartCmd = StartCmd;
Top:
	switch (CurTheme->ID)
	{   
		case GF_AREA_IN_MASK_THEME:
			nRc = DialogBox(hInst, (LPCSTR)"CREATE_MASKED_AREA_THEME", hWnd,(DLGPROC)AreaInMaskThemeMsgProc);
			break;
		case GF_GRAPHICS_FUNCTION_THEME:			 
		case GF_OFFSETAREA_THEME:
		case GF_SINGLE_VALUE_THEME:
	         {
	          DLGPROC lpfnSV_THEME1MsgProc;
	          setDoPaint( FALSE);
	          lpfnSV_THEME1MsgProc = MakeProcInstance((DLGPROC)SV_THEME1MsgProc, hInst);
	          nRc = DialogBox(hInst, (LPSTR)"SV_THEME1", hWnd, lpfnSV_THEME1MsgProc);
	          FreeProcInstance(lpfnSV_THEME1MsgProc);
	          setDoPaint( TRUE);
			  if (nRc == 2)
				  goto Top;
	         }

		break; 
				
		case GF_CONNECTION_LINE_THEME:
	         {
	          DLGPROC lpfnSV_THEME2MsgProc;
	          setDoPaint( FALSE);
	          lpfnSV_THEME2MsgProc = MakeProcInstance((DLGPROC)SV_THEME2MsgProc, hInst);
	          nRc = DialogBox(hInst, (LPSTR)"CONNECTION_LINE", hWnd, lpfnSV_THEME2MsgProc);
	          FreeProcInstance(lpfnSV_THEME2MsgProc);
	          setDoPaint( TRUE);
			  if (nRc == 2)
				  goto Top;
	         }

		break; 
				
		case GF_SINGLE_NONNUM_VALUE_THEME:
	         {
	          DLGPROC lpfnSV_THEME2MsgProc;
	          setDoPaint( FALSE);
	          lpfnSV_THEME2MsgProc = MakeProcInstance((DLGPROC)SV_THEME2MsgProc, hInst);
	          nRc = DialogBox(hInst, (LPSTR)"SV_THEME2", hWnd, lpfnSV_THEME2MsgProc);
	          FreeProcInstance(lpfnSV_THEME2MsgProc);
	          setDoPaint( TRUE);
			  if (nRc == 2)
				  goto Top;
	         }

		break; 

		case GF_STREET_ADDRESS_THEME:
	         {
	          DLGPROC lpfnEDIT_ADDS_THEMEMsgProc;
	          setDoPaint( FALSE);
	          lpfnEDIT_ADDS_THEMEMsgProc = MakeProcInstance((DLGPROC)EDIT_ADDS_THEMEMsgProc, hInst);
	          nRc = DialogBox(hInst, (LPSTR)"EDIT_ADDS_THEME", hWnd, lpfnEDIT_ADDS_THEMEMsgProc);
	          FreeProcInstance(lpfnEDIT_ADDS_THEMEMsgProc);
	          setDoPaint( TRUE);
	         }   
	    break;
			         
		case GF_CITY_THEME:
	         {
	          setDoPaint( FALSE);
	          nRc = DialogBox(hInst, (LPSTR)"CITY_THEME", hWnd, (DLGPROC)CITY_THEMEMsgProc);
	          setDoPaint( TRUE);
	         }   
	    break;
			         
		case GF_TRANSFORM_THEME:
	         {
	          DLGPROC lpfnTRANTHEMEMsgProc;
	          setDoPaint( FALSE);
	          lpfnTRANTHEMEMsgProc = MakeProcInstance((DLGPROC)TRANTHEMEMsgProc, hInst);
	          nRc = DialogBox(hInst, (LPSTR)"TRANTHEME", hWnd, lpfnTRANTHEMEMsgProc);
	          FreeProcInstance(lpfnTRANTHEMEMsgProc);
	          setDoPaint( TRUE);
	         } 
	    break;  
			         
		case GF_POLYINFO_THEME:
	         {
	          DLGPROC lpfnTRANTHEMEMsgProc;
	          setDoPaint( FALSE);
	          lpfnTRANTHEMEMsgProc = MakeProcInstance((DLGPROC)TRANTHEMEMsgProc, hInst);
	          nRc = DialogBox(hInst, (LPSTR)"TRANTHEME", hWnd, lpfnTRANTHEMEMsgProc);
	          FreeProcInstance(lpfnTRANTHEMEMsgProc);
	          setDoPaint( TRUE);
	         } 
	    break;  
			         
		case GF_HOTSPOT_THEME:
	         {
	          DLGPROC lpfnHOTSPOT_THEMEMsgProc;
	          setDoPaint( FALSE);
	          lpfnHOTSPOT_THEMEMsgProc = MakeProcInstance((DLGPROC)HOTSPOT_THEMEMsgProc, hInst);
	          nRc = DialogBox(hInst, (LPSTR)"HOTSPOT_THEME", hWnd, lpfnHOTSPOT_THEMEMsgProc);
	          FreeProcInstance(lpfnHOTSPOT_THEMEMsgProc);
	          setDoPaint( TRUE);
	         } 
	    break;  
			         
		case PF_COORD_DISPLAY:
	         {
	          DLGPROC lpfnCOORDDISPLAYMsgProc;
	          setDoPaint( FALSE);
	          lpfnCOORDDISPLAYMsgProc = MakeProcInstance((DLGPROC)COORDDISPLAYMsgProc, hInst);
	          nRc = DialogBox(hInst, (LPSTR)"COORDDISPLAY", hWnd, lpfnCOORDDISPLAYMsgProc);
	          FreeProcInstance(lpfnCOORDDISPLAYMsgProc);
	          setDoPaint( TRUE);
	         } 
	    break;  
			    
		case GF_STREET_TEXT_THEME:
	    {   
	    	LPVIEWPORT	SaveView; 
	    	short		n,i;
			DLGPROC lpfnSTREET_TEXTMsgProc;
			    	
			setDoPaint( FALSE);
			lpfnSTREET_TEXTMsgProc = MakeProcInstance((DLGPROC)STREET_TEXTMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"STREET_TEXT", hWnd, lpfnSTREET_TEXTMsgProc);
			FreeProcInstance(lpfnSTREET_TEXTMsgProc);
			setDoPaint( TRUE);
        }  
		break;
				
		case GF_PROFILE_THEME:
	    {   
	    	LPVIEWPORT	SaveView; 
	    	short		n,i;
			DLGPROC lpfnPROFILEMsgProc;
			    	
			setDoPaint( FALSE);
			lpfnPROFILEMsgProc = MakeProcInstance((DLGPROC)PROFILEMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"PROFILE", hWnd, lpfnPROFILEMsgProc);
			FreeProcInstance(lpfnPROFILEMsgProc);
			setDoPaint( TRUE);
        }  
		break;
		
		case GF_NORTH_ARROW_THEME:
		case GF_DISTANCE_THEME:
	    {    
	    	LPVIEWPORT	SaveView; 
	    	short		n,i;
			DLGPROC lpfnDISTANCEMsgProc;
			    	
			setDoPaint( FALSE);
			lpfnDISTANCEMsgProc = MakeProcInstance((DLGPROC)DISTANCEMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"DISTANCE", hWnd, lpfnDISTANCEMsgProc);
			FreeProcInstance(lpfnDISTANCEMsgProc);
			setDoPaint( TRUE);
        }  
		break;
				
		case GF_COORDGRID_THEME:
	    {    
	    	LPVIEWPORT	SaveView; 
	    	short		n,i;
			DLGPROC lpfnCOORDGRIDMsgProc;
			    	
			setDoPaint( FALSE);
			lpfnCOORDGRIDMsgProc = MakeProcInstance((DLGPROC)COORDGRIDMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"COORDGRID", hWnd, lpfnCOORDGRIDMsgProc);
			FreeProcInstance(lpfnCOORDGRIDMsgProc);
			setDoPaint( TRUE);
        }  
		break;
				
		case GF_2D_THEME:
	    {    
	    	LPVIEWPORT	SaveView; 
	    	short		n,i;
			DLGPROC lpfnDISPLAY2DMsgProc;
			    	
			setDoPaint( FALSE);
			lpfnDISPLAY2DMsgProc = MakeProcInstance((DLGPROC)DISPLAY2DMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"DISPLAY2D", hWnd, lpfnDISPLAY2DMsgProc);
			FreeProcInstance(lpfnDISPLAY2DMsgProc);
			setDoPaint( TRUE);
        }  
		break;
				
		case GF_TIME_DISPLAY_THEME:
	    {    
	    	LPVIEWPORT	SaveView; 
	    	short		n,i;
			DLGPROC lpfnTIME_DISPLAYMsgProc;
			    	
			setDoPaint( FALSE);
			lpfnTIME_DISPLAYMsgProc = MakeProcInstance((DLGPROC)TIME_DISPLAYMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"TIME_DISPLAY", hWnd, lpfnTIME_DISPLAYMsgProc);
			FreeProcInstance(lpfnTIME_DISPLAYMsgProc);
			setDoPaint( TRUE);
        }  
		break;
				
		case GF_POINT_IN_AREA_THEME:
	    {    
	    	LPVIEWPORT	SaveView; 
	    	short		n,i;
			DLGPROC lpfnPOINT_IN_AREAMsgProc;
			    	
			setDoPaint( FALSE);
			lpfnPOINT_IN_AREAMsgProc = MakeProcInstance((DLGPROC)POINT_IN_AREAMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"POINT_IN_AREA", hWnd, lpfnPOINT_IN_AREAMsgProc);
			FreeProcInstance(lpfnPOINT_IN_AREAMsgProc);
			setDoPaint( TRUE);
        }  
		break;
				
		case GF_COMPARE_VIEWPORTS_THEME:
	    {    
	    	LPVIEWPORT	SaveView; 
	    	short		n,i;
			DLGPROC lpfnCOMPARE_VIEWPORTSMsgProc;
			    	
			setDoPaint( FALSE);
			lpfnCOMPARE_VIEWPORTSMsgProc = MakeProcInstance((DLGPROC)COMPARE_VIEWPORTSMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"COMPARE_VIEWPORTS", hWnd, lpfnCOMPARE_VIEWPORTSMsgProc);
			FreeProcInstance(lpfnCOMPARE_VIEWPORTSMsgProc);
			setDoPaint( TRUE);
        }  
		break;
				
	    case GF_BOUNDS_DISPLAY_THEME:     
    	case GF_CONTEST_THEME:
	    case GF_NETMARKER_THEME:  
	    case GF_DYNAMIC_SEG_THEME:
        case GF_CACHE_DISPLAY_THEME:
		case GF_PROFILE_LINK_THEME:
	    {   // dont remove without documenting why!!
	    	LPVIEWPORT	SaveView; 
	    	short		n,i;
			DLGPROC lpfnBOUNDS_DISPLAYMsgProc;
			    	
			setDoPaint( FALSE);
			lpfnBOUNDS_DISPLAYMsgProc = MakeProcInstance((DLGPROC)BOUNDS_DISPLAYMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"BOUNDS_DISPLAY", hWnd, lpfnBOUNDS_DISPLAYMsgProc);
			FreeProcInstance(lpfnBOUNDS_DISPLAYMsgProc);
			setDoPaint( TRUE);
        }  
		break;
				
		case GF_DOCUMENTS_THEME:
	         {
	          DLGPROC lpfnDOCUMENTSMsgProc;
	          setDoPaint( FALSE);
	          lpfnDOCUMENTSMsgProc = MakeProcInstance((DLGPROC)DOCUMENTSMsgProc, hInst);
	          nRc = DialogBox(hInst, (LPSTR)"DOCUMENTS", hWnd, lpfnDOCUMENTSMsgProc);
	          FreeProcInstance(lpfnDOCUMENTSMsgProc);
	          setDoPaint( TRUE);
	         }  
	    break;     
				 
	} 
	ThemeEditStartCmd = 0;
{
#if ENABLETRACE
GSSiExitProg (1251);
#endif
    return nRc;
}
#if ENABLETRACE
}
#endif
}










void CloseTheme(void)
#if ENABLETRACE
{GSSiEnterProg (1256);
#endif
{
	if (CurView->pTheme)
	{   
		LPVIEWPORT	SaveView;
		LPTHEME		CurTheme;
		int	i,n;
					
		CurTheme = CurView->pTheme;
		ClearCompareDC (CurTheme,TRUE);
		GSSiGlobFree (&CurTheme->hVisList);
		SaveView = CurView;
		SetCurView (pViewports[CurTheme->TargetViewport-1]);
		RemoveThemeFromVP (CurView,CurTheme);
		SetCurView (SaveView);
    	CloseObject (CurView->pTheme); 
    	CurView->pTheme=0;
    } 
{
#if ENABLETRACE
GSSiExitProg (1256);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}


   
BOOL OpenThemeDataFile (LPSTR DataFile)
#if ENABLETRACE
{GSSiEnterProg (1258);
#endif
{ 
	LPVIEWPORT	SaveVP=CurView;
	char	Name[MAX_PATH+64];
	BOOL	rtn;

    if (CurTheme->hThemeDB)
{
#if ENABLETRACE
GSSiExitProg (1258);
#endif
    	return(TRUE); 
}

  	_fstrcpy (ThemeDB,DataFile);  
	if (StringEndsWith (ThemeDB,".ORA"))
		CurTheme->DataFileType = ORA_DATAFILE;
	SetCurView (pViewports[CurTheme->DisplayViewport-1]); 
	strncpy0 (CurTheme->DataFileID,CurView->Name,32);
	CurView = SaveVP;
	if (*DataFile && (!strchr (DataFile,'=') || !strnicmp (DataFile,"ODBC|",5)))
		sprintf (Name,"%s=%s",CurTheme->DataFileID,DataFile);
	else
		strcpy (Name,DataFile);
  	//CurTheme->Statement = 0;
	rtn = OpenDataFile (Name,CurTheme->SQL,BT_READ,&CurTheme->hThemeDB);
{
#if ENABLETRACE
GSSiExitProg (1258);
#endif
    return rtn;
}
    					  
#if ENABLETRACE
}
#endif
}

BOOL CloseThemeDataFile (BOOL Final)
#if ENABLETRACE
{GSSiEnterProg (1259);
#endif
{ 
	BOOL	rtn;

	if (!CurTheme)
{
#if ENABLETRACE
GSSiExitProg (1259);
#endif
		return(TRUE);
}
    if (!CurTheme->hThemeDB)
{
#if ENABLETRACE
GSSiExitProg (1259);
#endif
    	return(TRUE);  
}
    if (CurTheme->DataFileType == 10)
    {   
    	GSSiGlobUlFree (&CurTheme->hThemeDB);
    	CurTheme->hThemeDB = 0;
{
#if ENABLETRACE
GSSiExitProg (1259);
#endif
    	return TRUE;
}
    }
    else 
	{ 
		rtn = CloseDataFile (FALSE,&CurTheme->hThemeDB);
{
#if ENABLETRACE
GSSiExitProg (1259);
#endif
    	return rtn;
}
    }
#if ENABLETRACE
}
#endif
}
   
BOOL StartAutoClassDef (void)
{
	UINT	i;
	
	if (!CurTheme->AutoClassDef)
		return FALSE;
	for (i=0;i<MAX_THEME_CLASSES;i++)
		if (CurTheme->ClassStatus[i])
			return FALSE;
	return TRUE;
} 

void ProcessShowValMacro (BOOL Begin)
{   
	if (*CurTheme->ShowValMacro)
	{
		HANDLE	hMem=GSSiGlobAlloc (0,GMEM_MOVEABLE,1024);
		LPSTR	pMacro = GlobalLock (hMem);
		
		sprintf (pMacro,"$MACRO(%s,%i)",CurTheme->ShowValMacro,Begin);
		ExpandText (pMacro);
		GSSiGlobUlFree (&hMem);
	}
	return;
}

void ThemeBeginDisplayPass(BOOL PixelThemesOnly,short FromVPID)
#if ENABLETRACE
{GSSiEnterProg (1260);
#endif
{   int	itheme, iclass, i;
    LOGBRUSH	NDB;
    HBITMAP		hBM; 
    char		str[128]; 
    LPVIEWPORT	SaveView=CurView;
	LPPROFILETHEMEDATA	lpProfileData;
	static	long	LastDisplayCycle = -1;

	if (GetGlobalBVal2 ("[%ONEPASS]",FALSE))
		CurView->PassID = 5;
	CurView->HaveOrthos = SetHaveOrthos();
	SetTransparency (-1);
    if (DisplayCycle != LastDisplayCycle)
    {
    	FirstClasslessTheme = 0;
    	LastDisplayCycle = DisplayCycle;
    }
	RemoveDataDisplayRect (CurView);
	for (itheme=0;itheme<CurView->NumThemes;itheme++) 
	{
		CurTheme=CurView->pThemes[itheme];
	    if (CurTheme->ID == GF_CACHE_DISPLAY_THEME) 
	    {
			SetViewport (CurTheme->DisplayViewport);
			ThemeDisplayLegend(0,SaveView->ID);
			SetCurView (SaveView);
			if (CurView->PassID == 99) 
{
#if ENABLETRACE
GSSiExitProg (1260);
#endif
				return;            
}
			break;
		}
	}
	ThemeDisplayPass = 1;
	for (itheme=0;itheme<CurView->NumThemes;itheme++)
	{
		CurTheme=CurView->pThemes[itheme];
		if (!CurTheme->IsActive ||
		    !CurTheme->VPDisplayed ||
		    CurTheme->ID == GF_BOUNDS_DISPLAY_THEME ||
		    CurTheme->ID == GF_CACHE_DISPLAY_THEME ||
			CurTheme->ID == GF_PROFILE_LINK_THEME ||
		    CurTheme->ID == GF_GRAPHICS_FUNCTION_THEME)
		    goto NextTheme;
		if (PixelThemesOnly && CurTheme->DataType != THEMEDATATYPE_PIXEL)
			goto NextTheme;
		if (!PixelThemesOnly && CurTheme->DataType == THEMEDATATYPE_PIXEL)
			goto NextTheme;
		if (CurTheme->UseFirstSymbol)
			_fmemset (CurTheme->ClassSymbol,0,sizeof(CurTheme->ClassSymbol));    
		ProcessShowValMacro (TRUE);
		if (CurTheme->SymNum > 0)  
		{
			GSSiGlobFree (&CurTheme->hVisList);
			CurTheme->hVisList = SetThemeVisList (CurTheme->SymNum);  
		} 
		else if (CurTheme->SymNum == -2) 
		{
			GSSiGlobFree (&CurTheme->hVisList);
			CurTheme->hVisList = ReadVisList (&CurTheme->Contents[1]);  
		}
		if (ProcessPickItemDesc)
		{
			if (!ItemProcessedByTheme (ProcessPickItemPrefix,ProcessPickItemDesc))
				goto NextTheme; 
		}	
		if (ComputePCTTheme == CurTheme)
			BeginThemePCTByArea ();
		else if (CurTheme->DisplayViewport)
		{
//			SetCurView (pViewports[CurTheme->DisplayViewport-1]); //tempdebu
			SetViewport (CurTheme->DisplayViewport);
			ThemeDisplayLegend(1,SaveView->ID);
		}
		SetCurView (SaveView);
		if (*CurTheme->RefValChar) 
		{ 
			_fstrcpy (str,CurTheme->RefValChar);
			ExpandText (str);  
			CurTheme->RefValDbl = atof (str); 
		}
		if (!CurTheme->WantDataPass && !CurTheme->AutoClassDef)
			CurTheme->NumClass = CurTheme->NumDesiredClass;

/*		NDB.lbStyle = BS_HATCHED;
		NDB.lbColor = RGB(255,0,0);
		NDB.lbHatch	= HS_DIAGCROSS; 
		if (PatternBrush || Printing)
			CurTheme->NoDataBrush =  CreateBrushIndirect(&NDB);
		else
			CurTheme->NoDataBrush =  CreateSolidBrush(RGB(120,120,120));*/ 
		if (CurTheme->DataType <= 1)
		{
			hBM = LoadBitmap (hInst,MAKEINTRESOURCE(IDB_BITMAP2));
			CurTheme->InvalidDataBrush = CreatePatternBrush (hBM);
			i=DeleteObject (hBM);
			hBM = LoadBitmap (hInst,MAKEINTRESOURCE(IDB_BITMAP3));
			CurTheme->NoDataBrush = CreatePatternBrush (hBM);
			i=DeleteObject (hBM);
		}
		else
		{
			CurTheme->InvalidDataBrush= (HBRUSH)CreatePen(PS_DASH,1,RGB(255,0,0));
			CurTheme->NoDataBrush= (HBRUSH)CreatePen(PS_DOT,1,RGB(255,0,0));
		}
		if (!CurTheme->PCTByArea && !CurTheme->UseStoredCounts)
		for (iclass=0;iclass<MAX_THEME_CLASSES;iclass++)
			CurTheme->ClassCount[iclass] = 0; 
        CurTheme->NumMissing = 0;
        CurTheme->NumInvalid = 0;  
        if (CurTheme->ShowValue)
		{
        	BeginDelayedText (CurTheme->FidDelayedText);
			OpenShowValInfoboxFile ();
		}
		switch (CurTheme->ID)
		{  
			case GF_OFFSETAREA_THEME:
				ThemeDisplayOffsetAreas ();
				break;
			case GF_AREA_IN_MASK_THEME:
				ThemeCreateAreaInMask(TRUE);
				break;
			case GF_NORTH_ARROW_THEME:
				break;

			case GF_PROFILE_THEME:
				lpProfileData = (LPPROFILETHEMEDATA)&CurTheme->ClassBM; 
	    		lpProfileData->MinSeq = LONG_MAX;
	    		lpProfileData->MaxSeq = LONG_MIN;
		    	break;
			
			case GF_DYNAMIC_SEG_THEME: 
				CurTheme->RoundTo = GetGlobalDVal2 ("[%DYNAMICSEGDIST]",50);
				break;   
				
			case GF_STREET_TEXT_THEME:
			{
				int	i;
			    LPSTREETTEXTDATA	pStreetData=(LPSTREETTEXTDATA)CurTheme->ClassBM;
				BTVARDESC	BTVar[2];
			    
			    ResetStreetLabels ();
 				OpenThemeHighlightFile (BT_WRITE);
               
                {
                	LPVIEWPORT SaveVP=CurView;
                	
                	SetViewport(CurTheme->DisplayViewport);
                	if (!_fstricmp (CurView->Name,"Depth Labels"))
                		pStreetData->IgnoreShields = TRUE;
                	CurView = SaveVP;
                }
			    if (pStreetData->NameSource)
			    {
					OpenThemeDataFile(CurTheme->DataFile);
			
					GSSiGetTempFileName (0,"gma",0,(LPSTR)pStreetData->NameFile1);
					GSSiGetTempFileName (0,"gmb",0,(LPSTR)pStreetData->NameFile2); 
			
					BTVar[0].BT_VARTYP=BT_CHAR;
					BTVar[0].BT_VARLEN=pStreetData->NameLength;
					BTVar[0].BT_VAROFF=0;
					BT_CREATE (pStreetData->NameFile1, 4, FALSE, 1, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
					pStreetData->hNameFile1 = BT_OPEN (pStreetData->NameFile1,0, BT_WRITE, 0);
					BTVar[0].BT_VARTYP=BT_INTEGER;
					BTVar[0].BT_VARLEN=4;
					BTVar[0].BT_VAROFF=0;
					BT_CREATE (pStreetData->NameFile2, pStreetData->NameLength, FALSE, 1, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
					pStreetData->hNameFile2 = BT_OPEN (pStreetData->NameFile2,0, BT_WRITE, 0);
			    }
			    else
			    	pStreetData->hNameFile1 = 0;
			    if (*pStreetData->ListFile)
			    	pStreetData->hListDB = CreateListFile (pStreetData->ListFile,pStreetData->ListLen,pStreetData->SortOnData);                        
				pStreetData->SavePAE = ProcessAllElements;
				ProcessAllElements = pStreetData->ShowAllElements;
		    	GSSiGlobFree (&CurTheme->hScatterFile); 
				CurTheme->hScatterFile = GSSiGlobAlloc ( 638,GMEM_MOVEABLE,(long)sizeof(MIDPOINT)*(long)MaxMidpoints); 
				CurTheme->NumMidpoint=0;
				_fmemset (HaveStates,0,74*sizeof(BYTE));
			}                                   
				break;
				
			case GF_STREET_ADDRESS_THEME:
			{
				LPSHORT	pInt;
			    
			    GSSiGlobFree (&CurTheme->hScatterFile);
				CurTheme->hScatterFile = GSSiGlobAlloc ( 639,GMEM_MOVEABLE,USHRT_MAX); 
				pInt = (LPSHORT)GlobalLock(CurTheme->hScatterFile);
				*pInt = 0;
				GlobalUnlock (CurTheme->hScatterFile);   
			}
				break;
				
			case GF_CITY_THEME:
			{
				BTVARDESC	BTVar[3];
				int	i;
			
				OpenThemeDataFile(CurTheme->DataFile);
				CurTheme->CityUniqueInc = 0;
				if (CurTheme->CompareAttributes)//auto max pop option
					CurTheme->MaxCityPopOnScreen = 0;
				BT_CLOSE (CurTheme->hScatterFile);
				if (CurTheme->ScatterFile[0])
					GSSiRemove (CurTheme->ScatterFile);
				GSSiGetTempFileName (0,"gmt",0,(LPSTR)CurTheme->ScatterFile);
			
				BTVar[0].BT_VARTYP=BT_INTEGER;
				BTVar[0].BT_VARLEN=4;
				BTVar[0].BT_VAROFF=0;
				BTVar[1].BT_VARTYP=BT_INTEGER;
				BTVar[1].BT_VARLEN=2;
				BTVar[1].BT_VAROFF=4;
				BT_CREATE (CurTheme->ScatterFile, sizeof(CITY_NAME_DATA), FALSE, 2, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
				CurTheme->hScatterFile = BT_OPEN (CurTheme->ScatterFile,0, BT_WRITE, 0);
			}
				break;

			case GF_SINGLE_NONNUM_VALUE_THEME: 
				if (CurTheme->ScatterFile[0])  
				{   
					short	Mode=BT_READ;
					if (StartAutoClassDef () && (!CurTheme->PCTByArea || ComputePCTTheme)) 
					{
						CurTheme->NumClass = 0;
						Mode = BT_WRITE;  
					}
					if (CurTheme->AutoClassDef && !CurTheme->NumDesiredClass && FirstClasslessTheme != 0 &&!UseSymnumColor) 
					{
						CurTheme->NextValueColor = FirstClasslessTheme->NextValueColor;
						CurTheme->hScatterFile = BT_OPEN (FirstClasslessTheme->ScatterFile, 0, Mode, 0);
					} 
					else
						CurTheme->hScatterFile = BT_OPEN (CurTheme->ScatterFile, 0, Mode, 0); 
					if (StartAutoClassDef ()) 
					{
						if (CurTheme->NumDesiredClass > 0 || FirstClasslessTheme == 0) 
						{
							CurTheme->NextValueColor = 0;
							BT_CLEAR (CurTheme->hScatterFile);
						}	 
						if (!CurTheme->NumDesiredClass && FirstClasslessTheme == 0)
							FirstClasslessTheme = CurTheme;
					}
					else if (CurTheme->FieldFun)
						CurTheme->hScatterFile = ExpandThemeValues (CurTheme->hScatterFile,CurTheme->ScatterFile);	
				}
				else
					CurTheme->hScatterFile = 0;	
			case GF_CONNECTION_LINE_THEME:
			case GF_SINGLE_VALUE_THEME:
			case GF_TIME_DISPLAY_THEME:
			case GF_TWO_VALUE_THEME: 
				OpenThemeHighlightFile (BT_WRITE);
				OpenPointDispersionFile (BT_WRITE);
				OpenThemeDataFile(CurTheme->DataFile);
                if (CurTheme->hThemeDB && GetDBType (CurTheme->hThemeDB) != GMTEXT_DATAFILE)
	            	 GetDBFieldInfo (&CurTheme->Field,CurTheme->hThemeDB);
			    break;  
			    
	        case GF_CONTEST_THEME:
	        {
				int			width;
	         
	        	width = 2*WidthFactor;
				CurTheme->ClassPen[0]= CreatePen(PS_SOLID,width,RGB(255,0,0));
				CurTheme->ClassPen[1]= CreatePen(PS_SOLID,width,RGB(0,255,0));
	        	hBTNetCon = BT_OPEN ("contest.wor",0, BT_READ, 0);
	        }
			    break;  
			    
			case GF_NETMARKER_THEME:
			{
				BTVARDESC	BTVar[3];
				int	i;
			
				BT_CLOSE (CurTheme->hScatterFile);
				if (CurTheme->ScatterFile[0])
					GSSiRemove (CurTheme->ScatterFile);
				GSSiGetTempFileName (0,"gmt",0,(LPSTR)CurTheme->ScatterFile);
			
				BTVar[0].BT_VARTYP=BT_INTEGER;
				BTVar[0].BT_VARLEN=4;
				BTVar[0].BT_VAROFF=0;
				BTVar[1].BT_VARTYP=BT_REAL;
				BTVar[1].BT_VARLEN=8;
				BTVar[1].BT_VAROFF=4;
				BT_CREATE (CurTheme->ScatterFile, 8, FALSE, 2, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
				CurTheme->hScatterFile = BT_OPEN (CurTheme->ScatterFile,0, BT_WRITE, 0);
 				OpenNetLinkAndRef (CurTheme->Xmove,FALSE,&CurTheme->ReScan); 
			
			}
				break;  
			
			case GF_HOTSPOT_THEME:  
				DisplayHotSpots ();
				break;
				
			case GF_POLYINFO_THEME:
				DisplayCurveFactor = 0.1;
				break;  
				
			case GF_POINT_IN_AREA_THEME:
					if (!CurTheme->Pass)
				{
					if (CurTheme->FidAreas != HFILE_ERROR)
						GSSiClose (CurTheme->FidAreas);
					if (!CurTheme->ScatterFile[0])
						GSSiGetTempFileName (0,"gmt",0,(LPSTR)CurTheme->ScatterFile);
					CurTheme->FidAreas = GSSiOpenFile (CurTheme->ScatterFile,0,OF_CREATE);  
					CurTheme->NumAreas = 0;  
				}
				else
				{
					OpenThemeHighlightFile (BT_WRITE);
					OpenPointDispersionFile (BT_WRITE);
					OpenThemeDataFile(CurTheme->DataFile);
	                if (CurTheme->hThemeDB && GetDBType (CurTheme->hThemeDB) != GMTEXT_DATAFILE)
		            	 GetDBFieldInfo (&CurTheme->Field,CurTheme->hThemeDB);
				}
				break;
		} 
NextTheme:
	;
	} 
{
#if ENABLETRACE
GSSiExitProg (1260);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

HANDLE SetThemeVisList (short SymNum)
#if ENABLETRACE
{GSSiEnterProg (1261);
#endif
{
    LPVISLIST	SaveVis=CurVis; 
    LPVIEWPORT	SaveView=CurView; 
    LPTHEME		SaveTheme=CurTheme;
    HANDLE		handle;
		    
	SetCurView (pViewports[CurTheme->TargetViewport-1]);
    handle=GSSiGlobAlloc ( 640,GHND,sizeof(VISLIST));
    CurVis = (LPVISLIST)GlobalLock (handle); 
    if (SaveVis) 
    	*CurVis = *SaveVis;
    CurVis->hVisList=handle;
    _fmemset (CurVis->VisBits,0,sizeof(CurVis->VisBits));
	ToggleVisibility (SymNum);
	SetParentVisibility (SymNum,TRUE,-1); 
	GlobalUnlock (handle); 
	if (SaveVis)
		CurVis = SaveVis;   
	SetCurView (SaveView);      
	CurTheme = SaveTheme;
{
#if ENABLETRACE
GSSiExitProg (1261);
#endif
	return (handle);
}
#if ENABLETRACE
}
#endif
}


void GetClassMinMax (int iclass,LPDOUBLE pClassMin,LPDOUBLE pClassMax)
#if ENABLETRACE
{GSSiEnterProg (1263);
#endif
{
	if (*CurTheme->RefValChar)
	{   
		if (CurTheme->RefIsPCT)
		{
			*pClassMin = CurTheme->RefValDbl * (1 + CurTheme->ClassMin[iclass]/100);  
			*pClassMax = CurTheme->RefValDbl * (1 + CurTheme->ClassMax[iclass]/100);
		}
		else
		{
			*pClassMin = CurTheme->RefValDbl + CurTheme->ClassMin[iclass];  
			*pClassMax = CurTheme->RefValDbl + CurTheme->ClassMax[iclass];
		}   
	}
	else
	{
		*pClassMin = CurTheme->ClassMin[iclass]-CurTheme->RoundTo/3;  
		*pClassMax = CurTheme->ClassMax[iclass]+CurTheme->RoundTo/3;   
	} 
{
#if ENABLETRACE
GSSiExitProg (1263);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

BOOL ItemProcessedByTheme (LPSTR Prefix,int desc) 
{   
	BOOL	rtn=FALSE;
	
	if (!CurTheme)
		return FALSE;
		
	if (CurTheme->SymNum == -1)
	{
		if (!_fstricmp (Prefix,&CurTheme->Contents[1])) 
			rtn = TRUE;
	}
	else if (CurTheme->hVisList)
	{   
		LPVISLIST	SaveVis=CurVis; 
		BOOL		WantDesc;
				
		CurVis = (LPVISLIST)GlobalLock (CurTheme->hVisList);
		rtn = GetVisibility (desc);
		GlobalUnlock (CurTheme->hVisList);
		CurVis = SaveVis;
	}
	else if (desc == CurTheme->SymNum)
		rtn = TRUE;
	return rtn;
}

short ThemeTestChar (int Type, long iref, int desc, LPSTR TAG, LPSTR UDI)
#if ENABLETRACE
{GSSiEnterProg (1264);
#endif
{
// Returns -1 if element not processed by this theme
//          0 if element not in selected class
//			1 if element in selected class or no classes selected
	int	 iclass, ClassNo, ClassNo2;
	double	ValD, Rem,ClassMin, ClassMax;
	short	status;
	LPSTR	Value, KeyVal, VarVal;
	int		st, st1, st2, Case, ii, ipos,lnKey;
	BOOL	InClass; 
	HANDLE	hMEM=0;     
	UINT	i;
    
    if (!UseTestChar)
    	return -1;
    if (!CurTheme)
{
#if ENABLETRACE
GSSiExitProg (1264);
#endif
    	return (-1);
}
	if (CurTheme->ID == PF_COORD_DISPLAY || CurTheme->ID == PF_BOUNDS_DISPLAY ||
		CurTheme->ID == GF_NORTH_ARROW_THEME || CurTheme->ID == GF_CITY_THEME ||
		!CurTheme->IsActive|| !CurTheme->VPDisplayed)
{
#if ENABLETRACE
GSSiExitProg (1264);
#endif
		return (-1);
}
    if (CurTheme->SkipInvalid || CurTheme->MissOpt == 1)
    	goto CheckTheme;
	for (i=0;i<CurTheme->NumClass;i++)
		if (CurTheme->ClassStatus[i])
			goto CheckTheme;
{
#if ENABLETRACE
GSSiExitProg (1264);
#endif
	return -1; 
}
CheckTheme:
	*CurTheme->CurValue = 0;
	hMEM = GSSiGlobAlloc ( 641,GMEM_MOVEABLE,2048);  
	Value = GlobalLock(hMEM);
	KeyVal = Value+512;
	VarVal = KeyVal+512;
    HaltReport = FALSE;
	switch (CurTheme->ID)    
	{   
		case GF_OFFSETAREA_THEME:
			if (!ThemeCheckOffsetAreas ())
				goto RtnNoDisplay;
		break;
		
        case GF_POINT_IN_AREA_THEME: 
        	if (!CurTheme->Pass)    
        		goto RtnNoDisplay;
        case GF_TIME_DISPLAY_THEME:
		case GF_SINGLE_VALUE_THEME: 
			if (!ItemProcessedByTheme (TAG,desc))
					goto RtnNotProcessed;
			SwitchThemeSHPFile ();		
			ResetFileChangeTime (CurTheme->hThemeDB);
			ValD = GetNumericFieldData (CurTheme->hThemeDB,
										&CurTheme->Field,CurTheme->FieldFun,CurTheme->MultiValOption,CurTheme->Value,iref,Type,&status,CurTheme->DataFileID);
			InClass=FALSE; 
			if (status == 1 && CurTheme->MissOpt == 4)
			{
				status = 0;
				ValD = 0;
			}
			if (status==1 || (CurTheme->ZeroIsMissing && ValD == 0))
			{   
ProcessMissing:
				goto RtnNoDisplay;
			}
			if (status==-1)
			{
InvalidSV1:     
				goto RtnNotProcessed;
			}   
			if (CurTheme->MissOpt==3)
				goto RtnNoDisplay;
			if (ValD && CurTheme->FieldCorrection)
			{   
				double	Area,Dist;
				
				switch (CurTheme->DataType)
				{
					case 0:
						if (HiPrecis)
							Area = ComputeProjectedAreaAreaD (lpDCurPoints,nCurPoints,&Dist); 
						else 
							Area = ComputeProjectedAreaArea (lpCurPoints,nCurPoints,&Dist);  
						Area = ConvertArea (Area,CurTheme->FieldCorrection); 
						if (!Area)
							goto InvalidSV1;
						ValD /= Area;
					break;
					default:
					case 1:
					break;
					case 2:
						if (HiPrecis)
							ComputeProjectedAreaAreaD (lpDCurPoints,-nCurPoints,&Dist); 
						else 
							ComputeProjectedAreaArea (lpCurPoints,-nCurPoints,&Dist);  
						Dist = ConvertDist (Dist,CurTheme->FieldCorrection); 
						if (!Dist)
							goto InvalidSV1;
						ValD /= Dist;
					break;
				}
			}
			ValD = Round (ValD,CurTheme->RoundTo);
			
			if (ValD > CurTheme->YLimit) ValD = CurTheme->YLimit;
			ftoa (CurTheme->CurValue,ValD);
			for (iclass=0;iclass<CurTheme->NumClass;iclass++)
			{	
				GetClassMinMax (iclass,&ClassMin,&ClassMax);
								
				if (ValD>=ClassMin && ValD<=ClassMax)
			    {   
					if (CurTheme->ClassStatus[iclass])
						goto RtnNoDisplay;  
					goto RtnProcessed;
			    }
			}
			if (CurTheme->Recompute)
				goto RtnProcessed;
			goto ProcessMissing;
			break;

		case GF_CONNECTION_LINE_THEME:
		case GF_SINGLE_NONNUM_VALUE_THEME:
			if (CurTheme->LayerID && (LayerID != CurTheme->LayerID))
				goto RtnNotProcessed;
			if (CurTheme->SymNum == -1)
			{
				if (_fstricmp (TAG,&CurTheme->Contents[1]))
				{
					if (CurTheme == ComputePCTTheme)
						goto RtnNoDisplay;
					else
						goto RtnNotProcessed;
				}
			}
			else if (CurTheme->hVisList)
			{   
				LPVISLIST	SaveVis=CurVis; 
				BOOL		WantDesc;
				
				CurVis = (LPVISLIST)GlobalLock (CurTheme->hVisList);
				WantDesc = GetVisibility (desc);
				GlobalUnlock (CurTheme->hVisList);
				CurVis = SaveVis;
				if (!WantDesc)
				{
					if (CurTheme == ComputePCTTheme)
						goto RtnNoDisplay;
					else
						goto RtnNotProcessed;
				}
			}
			else if (CurTheme->SymNum > 0 && CurTheme->SymNum != desc)
			{   
				if (CurTheme == ComputePCTTheme)
					goto RtnNoDisplay;
				else
					goto RtnNotProcessed;
			}
			ResetFileChangeTime (CurTheme->hThemeDB);
			if (CurTheme->MultiValOption && CurTheme->MultiValOption < 3/*All values*/)
			{  
				short MinClass=MAX_THEME_CLASSES+1;
NextValue:		
				SwitchThemeSHPFile ();		
				status = GetCharFieldData (CurTheme->hThemeDB,
								  &CurTheme->Field,iref,CurTheme->FieldFun,CurTheme->Value,Value,256,CurTheme->DataFileID,CurTheme->MultiValOption); 
CheckStatus:
				if (status)
				{
					SetVarChangeTimes (1);
					if (MinClass > MAX_THEME_CLASSES)
						goto ProcessMissing;
					_fstrcpy (Value,MinClassValue);
					status = 0;
				}
				else
				{
					strcpy (CurTheme->CurValue,Value);
					if (!CurTheme->hScatterFile && CurTheme->ScatterFile[0])  
						CurTheme->hScatterFile = BT_OPEN (CurTheme->ScatterFile,0, BT_READ, 0);
					lnKey = GetBTKeyLen(CurTheme->hScatterFile); 
					ClassNo = 0; 
					_fstrncpy (KeyVal,Value,lnKey);
					KeyVal[lnKey]=0;
        			if (!BT_FIND (CurTheme->hScatterFile,KeyVal,BT_FIRST,BT_EQ,(LPSTR)&ClassNo))
					{
						if (ClassNo < MinClass)   
						{
							_fstrcpy (MinClassValue,Value);
							MinClass = ClassNo;
						}
					}
					else if (CurTheme->AllValueClass && CurTheme->AllValueClass < CurTheme->NumClass+1)
					{
						ClassNo = CurTheme->AllValueClass;
						goto GotClass;
					}
					if (FetchDBRec (CurTheme->hThemeDB))
						goto NextValue;
					else
					{
						status = 1;
						goto CheckStatus;
					}
				}
            }
			else
			{
KeepLooking:
				SwitchThemeSHPFile ();		
				status = GetCharFieldData (CurTheme->hThemeDB,
								  &CurTheme->Field,iref,CurTheme->FieldFun,CurTheme->Value,Value,256,CurTheme->DataFileID,CurTheme->MultiValOption); 
			}
			if (status==1)
				goto ProcessMissing; 
			
			if (status==-1)  //pViewports[0]
			{   
				goto RtnNotProcessed;
			}
			if (!CurTheme->hScatterFile && CurTheme->ScatterFile[0])  
				CurTheme->hScatterFile = BT_OPEN (CurTheme->ScatterFile,0, BT_READ, 0);
			_fstrncpy (KeyVal,Value,GetBTKeyLen(CurTheme->hScatterFile));
			strcpy (CurTheme->CurValue,Value);
			ClassNo = 0;
			if (!BT_FIND (CurTheme->hScatterFile,KeyVal,BT_FIRST,BT_EQ,(LPSTR)&ClassNo))
			{   
				if (!ClassNo || ClassNo > MAX_THEME_CLASSES)
					goto ProcessMissing;
	GotClass:
				if (CurTheme->ClassStatus[ClassNo-1])
					goto RtnNoDisplay;  
				goto RtnProcessed;
			}
			else
			{
				_fstrncpy (VarVal,"[",GetBTKeyLen(CurTheme->hScatterFile));  
				st = BT_FIND (CurTheme->hScatterFile,VarVal,BT_FIRST,BT_GE,(LPSTR)&ClassNo2);
				while (!st)
				{
					if (*VarVal == '[')
					{   
						ExpandText (VarVal);
						if (!_fstrcmp (VarVal,Value))
						{
							ClassNo = ClassNo2;
							goto GotClass;
						}
					}
					else
						st = 1;
					st = BT_FIND (CurTheme->hScatterFile,VarVal,BT_NEXT,BT_ANY,(LPSTR)&ClassNo2);
				}
			}
			if (!ClassNo && !CurTheme->AutoClassDef)
			{
				if (CurTheme->AllValueClass && CurTheme->AllValueClass < CurTheme->NumClass+1)
				{
					ClassNo = CurTheme->AllValueClass;
					goto GotClass;
				}
				if (CurTheme->SkipInvalid)
					goto RtnNoDisplay;
				if (!CurTheme->MultiValOption && CurTheme->DataFileType != SHAPE_DATAFILE)
					goto KeepLooking;
				goto ProcessMissing; 
			}
			goto RtnProcessed;
			break;    		
	}

RtnNotProcessed:
	if (HaltReport)
	{
		CloseThemeDataFile(TRUE);
		CurTheme->IsActive = FALSE; 
	}
	GSSiGlobUlFree (&hMEM);
{
#if ENABLETRACE
GSSiExitProg (1264);
#endif
	return -1;
}
RtnNoDisplay:
	if (HaltReport)
	{
		CloseThemeDataFile(TRUE);
		CurTheme->IsActive = FALSE; 
	}
	GSSiGlobUlFree (&hMEM);
{
#if ENABLETRACE
GSSiExitProg (1264);
#endif
	return  0;
}
RtnProcessed:
	if (HaltReport)
	{
		CloseThemeDataFile(TRUE);
		CurTheme->IsActive = FALSE; 
	} 
	else if (!CurTheme->DataType)
		HaveVarFillColor = TRUE; 
	GSSiGlobUlFree (&hMEM);
{
#if ENABLETRACE
GSSiExitProg (1264);
#endif
	return	1;  
}
	
#if ENABLETRACE
}
#endif
}

BOOL GetThemeIDText (int ID,LPSTR Name)
{

	switch (ID)
	{
	case GF_SINGLE_VALUE_THEME:
		strcpy (Name,"Single Numeric Value");
		break;
	case GF_MOVE_POLY_THEME:
	case GF_UNDRAW_THEME:	
	case GF_TWO_VALUE_THEME:
	case GF_COMPARATIVE_THEME:
	case GF_SAVEPOLY_THEME:
	case GF_CRIME_THEME:
	case GF_SINGLE_NONNUM_VALUE_THEME:   
		strcpy (Name,"Single Non-Numeric Value");
		break;
	case GF_NETMARKER_THEME:
	case GF_SNAP_POLY_THEME:
	case GF_DOCUMENTS_THEME:   
	case GF_STREET_TEXT_THEME:
	case GF_CONTEST_THEME:
	case GF_STREET_ADDRESS_THEME:
	case GF_TRANSFORM_THEME:
	case GF_SAVEPOLYFILE_THEME:
	case GF_SAVEPOLYPARTS_THEME:
	case GF_BOUNDS_DISPLAY_THEME: 
	case GF_DISTANCE_THEME:
	case GF_COORDGRID_THEME:
	case GF_DYNAMIC_SEG_THEME:
	case GF_PROFILE_THEME:
	case GF_2D_THEME:
	case GF_HOTSPOT_THEME:
	case GF_POLYINFO_THEME:   
	case GF_GRAPHICS_FUNCTION_THEME:
	case GF_CACHE_DISPLAY_THEME: 
	case GF_COMPARE_VIEWPORTS_THEME:  
	case GF_TIME_DISPLAY_THEME:
	case GF_POINT_IN_AREA_THEME:
	case GF_NORTH_ARROW_THEME:
	case GF_PROFILE_LINK_THEME:
	case GF_OFFSETAREA_THEME:
	case GF_AREA_IN_MASK_THEME:
	case GF_WHEELZOOM:
	case GF_CONNECTION_LINE_THEME:
	case GF_CITY_THEME:
		strcpy (Name,"Other");
		break;
	default:
		*Name = 0;
		return FALSE;
	}
	return TRUE;
}





