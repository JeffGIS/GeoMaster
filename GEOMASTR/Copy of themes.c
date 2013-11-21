#include "graphint.h"
#include "extrndb.h"

#include "gmextern.h"

static LPTHEME	NewTheme=0;
static	char	DumpFileName[144]="";
static	char	MinClassValue[512];
static	char	OldValListFile[128];  
static	double	SaveCMin[MAX_THEME_CLASSES];
static	double	SaveCMax[MAX_THEME_CLASSES];
static	char	SaveCTitle[MAX_THEME_CLASSES][128];
static	UINT	IDC_Missing_Opts[5]={IDC_MISSING_OPT0,IDC_MISSING_OPT1,IDC_MISSING_OPT2,IDC_MISSING_OPT3,IDC_MISSING_OPT4};
static	UINT	ClassField[49]={IDC_UNASSIGNED,
					    	 	 IDC_CLASS1,
					    	 	 IDC_CLASS2,
					    	 	 IDC_CLASS3,
					    	 	 IDC_CLASS4,
					    	 	 IDC_CLASS5,
					    	 	 IDC_CLASS6,
					    	 	 IDC_CLASS7,
					    	 	 IDC_CLASS8,
					    	 	 IDC_CLASS9,
					    	 	 IDC_CLASS10,
					    	 	 IDC_CLASS11,
					    	 	 IDC_CLASS12,
					    	 	 IDC_CLASS13,
					    	 	 IDC_CLASS14,
					    	 	 IDC_CLASS15,
					    	 	 IDC_CLASS16,
					    	 	 IDC_CLASS17,
					    	 	 IDC_CLASS18,
					    	 	 IDC_CLASS19,
					    	 	 IDC_CLASS20,
					    	 	 IDC_CLASS21,
					    	 	 IDC_CLASS22,
					    	 	 IDC_CLASS23,
					    	 	 IDC_CLASS24,
					    	 	 IDC_CLASS25,
					    	 	 IDC_CLASS26,
					    	 	 IDC_CLASS27,
					    	 	 IDC_CLASS28,
					    	 	 IDC_CLASS29,
					    	 	 IDC_CLASS30,
					    	 	 IDC_CLASS31,
					    	 	 IDC_CLASS32,
					    	 	 IDC_CLASS33,
					    	 	 IDC_CLASS34,
					    	 	 IDC_CLASS35,
					    	 	 IDC_CLASS36,
					    	 	 IDC_CLASS37,
					    	 	 IDC_CLASS38,
					    	 	 IDC_CLASS39,
					    	 	 IDC_CLASS40,
					    	 	 IDC_CLASS41,
					    	 	 IDC_CLASS42,
					    	 	 IDC_CLASS43,
					    	 	 IDC_CLASS44,
					    	 	 IDC_CLASS45,
					    	 	 IDC_CLASS46,
					    	 	 IDC_CLASS47,
					    	 	 IDC_CLASS48};
static	UINT	TitleCntl[48]={IDC_TITLE1,
								IDC_TITLE2,
								IDC_TITLE3,
								IDC_TITLE4,
								IDC_TITLE5,
								IDC_TITLE6,
								IDC_TITLE7,
								IDC_TITLE8,
								IDC_TITLE9,
								IDC_TITLE10,
								IDC_TITLE11,
								IDC_TITLE12,
								IDC_TITLE13,
								IDC_TITLE14,
								IDC_TITLE15,
								IDC_TITLE16,
								IDC_TITLE17,
								IDC_TITLE18,
								IDC_TITLE19,
								IDC_TITLE20,
								IDC_TITLE21,
								IDC_TITLE22,
								IDC_TITLE23,
								IDC_TITLE24,
								IDC_TITLE25,
								IDC_TITLE26,
								IDC_TITLE27,
								IDC_TITLE28,
								IDC_TITLE29,
								IDC_TITLE30,
								IDC_TITLE31,
								IDC_TITLE32,
								IDC_TITLE33,
								IDC_TITLE34,
								IDC_TITLE35,
								IDC_TITLE36,
								IDC_TITLE37,
								IDC_TITLE38,
								IDC_TITLE39,
								IDC_TITLE40,
								IDC_TITLE41,
								IDC_TITLE42,
								IDC_TITLE43,
								IDC_TITLE44,
								IDC_TITLE45,
								IDC_TITLE46,
								IDC_TITLE47,
								IDC_TITLE48};

static	UINT	ClassIDCntl[48]={IDC_CLASSID1,
								IDC_CLASSID2,
								IDC_CLASSID3,
								IDC_CLASSID4,
								IDC_CLASSID5,
								IDC_CLASSID6,
								IDC_CLASSID7,
								IDC_CLASSID8,
								IDC_CLASSID9,
								IDC_CLASSID10,
								IDC_CLASSID11,
								IDC_CLASSID12,
								IDC_CLASSID13,
								IDC_CLASSID14,
								IDC_CLASSID15,
								IDC_CLASSID16,
								IDC_CLASSID17,
								IDC_CLASSID18,
								IDC_CLASSID19,
								IDC_CLASSID20,
								IDC_CLASSID21,
								IDC_CLASSID22,
								IDC_CLASSID23,
								IDC_CLASSID24,
								IDC_CLASSID25,
								IDC_CLASSID26,
								IDC_CLASSID27,
								IDC_CLASSID28,
								IDC_CLASSID29,
								IDC_CLASSID30,
								IDC_CLASSID31,
								IDC_CLASSID32,
								IDC_CLASSID33,
								IDC_CLASSID34,
								IDC_CLASSID35,
								IDC_CLASSID36,
								IDC_CLASSID37,
								IDC_CLASSID38,
								IDC_CLASSID39,
								IDC_CLASSID40,
								IDC_CLASSID41,
								IDC_CLASSID42,
								IDC_CLASSID43,
								IDC_CLASSID44,
								IDC_CLASSID45,
								IDC_CLASSID46,
								IDC_CLASSID47,
								IDC_CLASSID48};
								
static	UINT	FromCntl[16]=	{IDC_FROM1,
								IDC_FROM2,
								IDC_FROM3,
								IDC_FROM4,
								IDC_FROM5,
								IDC_FROM6,
								IDC_FROM7,
								IDC_FROM8,
								IDC_FROM9,
								IDC_FROM10,
								IDC_FROM11,
								IDC_FROM12,
								IDC_FROM13,
								IDC_FROM14,
								IDC_FROM15,
								IDC_FROM16};
static	UINT	ToCntl[16]=	{IDC_TO1,
								IDC_TO2,
								IDC_TO3,
								IDC_TO4,
								IDC_TO5,
								IDC_TO6,
								IDC_TO7,
								IDC_TO8,
								IDC_TO9,
								IDC_TO10,
								IDC_TO11,
								IDC_TO12,
								IDC_TO13,
								IDC_TO14,
								IDC_TO15,
								IDC_TO16};

static	UINT	SepCntl[16]=	{IDC_SEP1,
								IDC_SEP2,
								IDC_SEP3,
								IDC_SEP4,
								IDC_SEP5,
								IDC_SEP6,
								IDC_SEP7,
								IDC_SEP8,
								IDC_SEP9,
								IDC_SEP10,
								IDC_SEP11,
								IDC_SEP12,
								IDC_SEP13,
								IDC_SEP14,
								IDC_SEP15,
								IDC_SEP16};


short ConvertFontHeightFromPCTofVP (short PCT,short VPID)
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
	
short ConvertFontHeightToPCTofVP (short size,short VPID)
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
	pTheme->FidDelayedText = HFILE_ERROR;
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

BOOL PickThemeClass (short iclass,POINT MousePoint)
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

void DrawUnSelectedClass (short iclass,RECT ClassClrBox)
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
		HPEN	hPen = CreatePen (PS_SOLID,1,RGB(255,0,0));
		HPEN	hOldPen = SelectObject (CurView->hDC,hPen);  
		RECT	Rect = ClassClrBox;
				
		InflateRect (&Rect,2,2);
		MoveTo (CurView->hDC,Rect.left,Rect.bottom);
		LineTo (CurView->hDC,Rect.right,Rect.top);
		MoveTo (CurView->hDC,Rect.left,Rect.top);
		LineTo (CurView->hDC,Rect.right,Rect.bottom);
		SelectObject (CurView->hDC,hOldPen);
		DeleteObject (hPen);
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
			    	FARPROC lpfnLINETYPEMsgProc; 
			    	int		nRc,R,G,B,W;
				    	
					DoPaint = FALSE;  
					SetViewport(CurTheme->TargetViewport);
					NewColor = CurTheme->ClassColor[iclass];   
					NewWidth = CurTheme->ClassFactor[iclass];
					INewWidth = CurTheme->ClassFactor[iclass];
					if (INewWidth > 128)
						INewWidth -= 256;
					if (INewWidth > 0)
						INewWidth--;
			        lpfnLINETYPEMsgProc = MakeProcInstance((FARPROC)LINETYPEMsgProc, hInst);
			        nRc = DialogBox(hInst, (LPSTR)"LINETYPE", CurView->hWnd, lpfnLINETYPEMsgProc);
			        FreeProcInstance(lpfnLINETYPEMsgProc);
					DoPaint=TRUE;   
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
                    if (!GetTextString (GetFocus(),str,32,"Enter text factor",NULL,NULL,0,TRUE,TRUE))
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

BOOL ThemeSVChangeColor (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1231);
#endif
{
 POINT	MousePoint;
 static	HaveDown=FALSE;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		SetPrompt (PRMT_SET_CLASS_COLOR,TRUE);  
   		HaveDown=TRUE;
   		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    	
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint = MAKEPOINT(lParam);
		ThemeSVChangeClr (MousePoint);
		ThemeDisplayLegend(3,0);
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (1231);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1231);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}
 
BOOL ThemeChangeFactor (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1232);
#endif
{
 POINT	MousePoint;
 static	HaveDown=FALSE;
 char	str[260];

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		HaveDown=TRUE;
   		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    
    case WM_RBUTTONUP:
		GetGFFile (str,CurTheme->SQL,2); 
		EditTextFile (hWnd,str);
    	break;
    		
    case WM_LBUTTONUP:
	{	 
		int		iclass, isym, rtn=0; 
		char	SymName[34], SaveSymDict[128];
		
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint = MAKEPOINT(lParam);
	    
	    NumPicked = 0;
		if (!CurView->pTheme) break;
		CurTheme = CurView->pTheme;
		if (!CurTheme->IsActive) break;
		for (iclass=0;iclass<CurTheme->NumClass;iclass++)
		{	
			if (PickThemeClass(iclass,MousePoint)) 
			{   

	        	sprintf (str,"%f",CurTheme->ClassFactor[iclass]);
                if (!GetTextString (GetFocus(),str,32,"Enter symbol factor",NULL,NULL,0,TRUE,TRUE))
                	break;
                CurTheme->ClassFactor[iclass] = atof (str);

				break;
			}
		}
		ThemeDisplayLegend(3,0);
	}
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (1232);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1232);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

BOOL ThemeToggleClassStatus (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1232);
#endif
{
 POINT	MousePoint;
 static	HaveDown=FALSE;
 char	str[260];

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		HaveDown=TRUE;
   		break;

    case WM_RBUTTONDOWN:
    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
	{	 
		int		iclass, isym, rtn=0; 
		char	SymName[34], SaveSymDict[128];
		
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint = MAKEPOINT(lParam);
	    
	    NumPicked = 0;
		if (!CurView->pTheme) break;
		CurTheme = CurView->pTheme;
		if (!CurTheme->IsActive) break;
		for (iclass=0;iclass<CurTheme->NumClass;iclass++)
		{	
			if (PickThemeClass(iclass,MousePoint)) 
			{   

	        	if (CurTheme->ClassStatus[iclass])
	        		CurTheme->ClassStatus[iclass] = FALSE;
	        	else
	        		CurTheme->ClassStatus[iclass] = TRUE;
	        	if (Message == WM_RBUTTONUP)
	        	{
	        		BOOL Stat=CurTheme->ClassStatus[iclass];
	        		
					for (iclass=0;iclass<CurTheme->NumClass;iclass++)
	        			CurTheme->ClassStatus[iclass] = Stat; 
	        	}
				break;
			}
		}
		ThemeDisplayLegend(3,0);
	}
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (1232);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1232);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

BOOL ThemeChangeSymbol (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1232);
#endif
{
 POINT	MousePoint;
 static	HaveDown=FALSE;
 char	str[260];

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		HaveDown=TRUE;
   		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    
    case WM_RBUTTONUP:
		GetGFFile (str,CurTheme->SQL,2); 
		EditTextFile (hWnd,str);
    	break;
    		
    case WM_LBUTTONUP:
	{	 
		int		iclass, isym, rtn=0; 
		char	SymName[34], SaveSymDict[128];
		
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint = MAKEPOINT(lParam);
	    
	    NumPicked = 0;
		if (!CurView->pTheme) break;
		CurTheme = CurView->pTheme;
		if (!CurTheme->IsActive) break;
		for (iclass=0;iclass<CurTheme->NumClass;iclass++)
		{	
			if (PickThemeClass(iclass,MousePoint)) 
			{   
				if (*CurTheme->IconLibrary)
				{
					GetGlobalCVal ("[%SYM_DICT]",SaveSymDict,NULL);  
					sprintf (str,"%sfilelist.txt",CurTheme->IconLibrary);
					SetGlobalValue ("%SYM_DICT",str);
					CloseSymDict(); 
					if (OpenSymDict (OF_READ))
						rtn = SelectPointSymbol (hWnd,2,SymName,"",NULL,NULL,NULL,FALSE);
					SetGlobalValue ("%SYM_DICT",SaveSymDict);    
					CloseSymDict(); 
					if (rtn)
					{
						HFILE Fid;
						LPSTR	AtLoc;
						
						GetGFFile (str,CurTheme->SQL,2); 
						Fid = GSSiOpenFile (str,NULL,OF_READWRITE); 
                        GSSillseek (Fid,labs(CurTheme->ClassCount[iclass])-1,0);
						fgetstring (str,256,Fid);
						if ((AtLoc = _fstrchr (str,'&')))
						{ 
							GSSillseek (Fid,labs(CurTheme->ClassCount[iclass])-1+((long)AtLoc - (long)str),0);
							_fstrcpy (str,"&"); 
							_fstrcat (str,SymName);
							PadString (str,' ',13);
							BigWrite (Fid,str,13,-1);
						}
						GSSiClose (Fid);
					}
				}
				else if (*CurTheme->SymbolFont[0])
				{
					_fmemmove (CurSymbolFont,CurTheme->SymbolFont,sizeof(CurSymbolFont));
					CurTheme->ClassSymbol[iclass] = SelectFontSymbol (hWnd,CurTheme->SymSizeC,&CurTheme->ClassColor[iclass]);
				}
				else
				{
					GetDictSymName (CurTheme->ClassSymbol[iclass],SymName);
					SetGlobalValue("%NEW_POINT_SYM",SymName); 
					SetGlobalValue("%NEW_POINT_SIZE",CurTheme->SymSizeC); 
					SetGlobalValueLong("%NEW_POINT_COLOR",CurTheme->ClassColor[iclass]); 
					if (!(isym=SelectSymbol (hWnd,1,"ALL",1)))
{
#if ENABLETRACE
GSSiExitProg (1232);
#endif
						return TRUE;
}
					CurTheme->ClassSymbol[iclass]=isym;    
					GetGlobalCVal ("[%NEW_POINT_SIZE]",CurTheme->SymSizeC,NULL);
					CurTheme->ClassColor[iclass]=GetGlobalLVal ("[%NEW_POINT_COLOR]"); 
				}
				break;
			}
		}
		ThemeDisplayLegend(3,0);
	}
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (1232);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1232);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

BOOL ThemeRunMacro (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
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
    	MousePoint = MAKEPOINT(lParam);
    	NumPicked = ClassMacro (MousePoint,1,NULL,NULL,NULL);
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
							*phBox = YellowTextBox (CurView->hWnd,pTmp,MousePoint,0,FALSE);
							GSSiGlobUlFree (&hTmp);
						} 
					    else 
					    {
					    	*phBox = SaveScreen2 (CurView->hDC,ClassRect,0,NULL);
					  		SelectClipRgn (CurView->hDC,NULL);
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
					*phBox = YellowTextBox (CurView->hWnd,CurTheme->ClassBM[iclass],MousePoint,0,FALSE);  
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
		  		TrackPopupMenu (ApMenu,TPM_CENTERALIGN|TPM_LEFTBUTTON,position.x,position.y,NULL,CurView->hWnd,NULL);
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
			_fstrcpy (PickList[0].Prefix,"%CLASSYM");
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
					*phBox = YellowTextBox (CurView->hWnd,"Change Function List",MousePoint,0,FALSE);  
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
			PickList[0].ViewID = CurView->ID;
			_fstrcpy (PickList[0].Prefix,"%THEMETITLE");
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
			_fstrcpy (PickList[0].Prefix,"%THEMEBODY");
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
	
	if (!(CurTheme->IsActive || CurTheme->ID == PF_COORD_DISPLAY) || !CurTheme->VPDisplayed)
{
#if ENABLETRACE
GSSiExitProg (1237);
#endif
		return;
}
	
	if (CurTheme->TargetViewport > 0 && CurTheme->TargetViewport <= *pNumViewports)
	{
		if (!pViewports[CurTheme->TargetViewport-1]->Active)
{
#if ENABLETRACE
GSSiExitProg (1237);
#endif
			return;
}
	} else		
{
#if ENABLETRACE
GSSiExitProg (1237);
#endif
			return;
}
//	SetCurView (pViewports[CurTheme->DisplayViewport-1]); //tempdebu
	SetViewport (CurTheme->DisplayViewport);
	ClearFullWindowBitmap ();
	switch (CurTheme->ID)
	{   
        case GF_CONTEST_THEME: 
        case GF_DYNAMIC_SEG_THEME:
        case GF_POLYINFO_THEME: 
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
					SetTimer(hWndMain, STREETEDITTIMERID, 1, (FARPROC) NULL);  
				}
			}
			break;
			
        case GF_CACHE_DISPLAY_THEME:
        	CacheDisplayTheme (BeginOrEndDisplayPass);
        	break;
        	
        case GF_GRAPHICS_FUNCTION_THEME: 
//        	if (BeginOrEndDisplayPass && BeginOrEndDisplayPass != 3)
//        		break;  
		case GF_SINGLE_VALUE_THEME:
		case GF_SINGLE_NONNUM_VALUE_THEME:  
		case GF_POINT_IN_AREA_THEME:
			DisplaySVThemeLegend(BeginOrEndDisplayPass);
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
					SetTimer(hWndMain, NETMARKERTIMERID, 1, (FARPROC) NULL);  
				}
			}
			break;
	}
	DisplayCloseIcon (); 
	SetCurView (SaveView);
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

BOOL FAR PASCAL TRANTHEMEMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1238);
#endif
{	
 	FILE	*FidNoteType; 
 	char	str[130]; 
	LPSTR	lpBar, lpName;
	LPSHORT	pNumDocs;
	int		NumDocs=0;
   	int		nItems,i;     
   	COLORREF	Color;
   	static	COLORREF	CurColor1, CurColor2;


 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1238);
#endif
 	return (BRtn);
}
// if (ThemeCommonCode (hWndDlg, Message, wParam, lParam,0)) retrn TRUE;
 switch(Message)
   {
    case WM_INITDIALOG:
    	 SetDlgItemText (hWndDlg,IDC_FILENAME,CurTheme->DataFile); 
		 SendDlgItemMessage (hWndDlg,IDC_CLDISPLAY,BM_SETCHECK,CurTheme->ZeroIsMissing,0L);
		 SendDlgItemMessage (hWndDlg,IDC_EEDISPLAY,BM_SETCHECK,CurTheme->AddCommas,0L);
		 SendDlgItemMessage (hWndDlg,IDC_DISPLAYONLY,BM_SETCHECK,CurTheme->DisplayPointsOnly,0L);
		 
		 for (i=0;i<*pNumViewports;i++)
		 {
			SendDlgItemMessage (hWndDlg,SV_TARGET1,CB_ADDSTRING,NULL,(LPARAM)pViewports[i]->Name);
			SendDlgItemMessage (hWndDlg,SV_TARGET2,CB_ADDSTRING,NULL,(LPARAM)pViewports[i]->Name);
		 }
		 SendDlgItemMessage (hWndDlg,SV_TARGET1,CB_SETCURSEL,CurTheme->TargetViewport-1,NULL);
		 SendDlgItemMessage (hWndDlg,SV_TARGET2,CB_SETCURSEL,CurTheme->ReScan-1,NULL);
		 CurColor1 = CurTheme->ClassColor[0];
		 CurColor2 = CurTheme->ClassColor[1];
		 SendDlgItemMessage (hWndDlg,IDC_TRANTYPE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Least Squares"));
		 SendDlgItemMessage (hWndDlg,IDC_TRANTYPE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Average"));
		 SendDlgItemMessage (hWndDlg,IDC_TRANTYPE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Fixed (TIN)"));
		 SendDlgItemMessage (hWndDlg,IDC_CLWIDTH,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"1"));
		 SendDlgItemMessage (hWndDlg,IDC_CLWIDTH,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"2"));
		 SendDlgItemMessage (hWndDlg,IDC_CLWIDTH,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"3"));
		 SendDlgItemMessage (hWndDlg,IDC_EESTYLE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Opaque"));
		 SendDlgItemMessage (hWndDlg,IDC_EESTYLE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Transparent"));
		 SendDlgItemMessage (hWndDlg,IDC_TRANTYPE,CB_SETCURSEL,CurTheme->DispersePoints,NULL); 
		 SendDlgItemMessage (hWndDlg,IDC_CLWIDTH,CB_SETCURSEL,CurTheme->ValConv,NULL); 
		 SendDlgItemMessage (hWndDlg,IDC_EESTYLE,CB_SETCURSEL,CurTheme->MissOpt,NULL); 
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           {
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 EndDialog(hWndDlg, FALSE);
                 break; 
            
            case IDC_CLCOLOR: 
            	 Color = CurColor1;
            	 if (GetColor (hWndDlg,&Color))
            	 	CurColor1 = Color;
            	 break;
            	 
            case IDC_EECOLOR:
            	 Color = CurColor2;
            	 if (GetColor (hWndDlg,&Color))
            	 	CurColor2 = Color;
            	 break;
            	 
            case IDOK:
            {
            	GetDlgItemText (hWndDlg,IDC_FILENAME,CurTheme->DataFile,128);
				CurTheme->DisplayPointsOnly = SendDlgItemMessage (hWndDlg,IDC_DISPLAYONLY,BM_GETCHECK,0,0);
				CurTheme->ZeroIsMissing = SendDlgItemMessage (hWndDlg,IDC_CLDISPLAY,BM_GETCHECK,0,0);
				CurTheme->AddCommas = SendDlgItemMessage (hWndDlg,IDC_EEDISPLAY,BM_GETCHECK,0,0);
                CurTheme->TargetViewport=SendDlgItemMessage(hWndDlg,SV_TARGET1,CB_GETCURSEL,NULL,NULL)+1; 
                CurTheme->ReScan=SendDlgItemMessage(hWndDlg,SV_TARGET2,CB_GETCURSEL,NULL,NULL)+1; 
                CurTheme->DispersePoints=SendDlgItemMessage(hWndDlg,IDC_TRANTYPE,CB_GETCURSEL,NULL,NULL); 
				CurTheme->ClassColor[0]=CurColor1;
				CurTheme->ClassColor[1]=CurColor2;
                CurTheme->ValConv=SendDlgItemMessage(hWndDlg,IDC_CLWIDTH,CB_GETCURSEL,NULL,NULL);
                CurTheme->MissOpt=SendDlgItemMessage(hWndDlg,IDC_EESTYLE,CB_GETCURSEL,NULL,NULL);
               	EndDialog(hWndDlg, TRUE);
            }
                break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1238);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1238);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
} 


BOOL FAR PASCAL HOTSPOT_THEMEMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1239);
#endif
{	
 	FILE	*FidNoteType; 
 	char	str[130]; 
	LPSTR	lpBar, lpName;
	LPSHORT	pNumDocs;
	int		NumDocs=0;
   	int		nItems,i;  
   	short	item;   
   	COLORREF	Color;
   	static	COLORREF	CurColor1, CurColor2;  
	int	IDC_FieldName=SV_FIELD_NAME; 
	BOOL	True=TRUE;  


 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1239);
#endif
 	return (BRtn);
}
 if (DATAFILEMsgProc(hWndDlg,Message,wParam,lParam,IDC_SQL,
 					 SV_SET_FILE, SV_DATABASE_LIST, SV_TABLE_NAMES,SV_TABLE_HEADING, &IDC_FieldName,1,
 					 CurTheme->DataFile, &CurTheme->DataFileType, &CurTheme->hThemeDB, &True,FALSE))
{
#if ENABLETRACE
GSSiExitProg (1247);
#endif
	return TRUE;
}
 if (ThemeCommonCode (hWndDlg, Message, wParam, lParam,CurTheme->hThemeDB))
{
#if ENABLETRACE
GSSiExitProg (1239);
#endif
 	return TRUE;
}
 switch(Message)
   {
    case WM_INITDIALOG:
    	 SetDlgItemText (hWndDlg,IDC_HSWEIGHT,CurTheme->IconLibrary);
    	 SetDlgItemText (hWndDlg,IDC_HSRADIUS,CurTheme->ClassDefValSQL);
//    	 itoa (CurTheme->HotSpotData.ColorMin,str,10); 
//    	 SetDlgItemText (hWndDlg,IDC_HSMINCOLOR,str);
    	 SetDlgItemText (hWndDlg,IDC_FILENAME,CurTheme->DataFile); 
         SetDlgItemText(hWndDlg,IDC_SQL,(LPCSTR)CurTheme->SQL); 
		 SendDlgItemMessage (hWndDlg,IDC_PASSTHROUGHNONE,BM_SETCHECK,FALSE,0L);
		 SendDlgItemMessage (hWndDlg,IDC_PASSTHROUGHTHEMES,BM_SETCHECK,FALSE,0L);
		 SendDlgItemMessage (hWndDlg,IDC_PASSTHROUGHDISPLAY,BM_SETCHECK,FALSE,0L);
         switch (CurTheme->HotSpotData.PassThrough)
         {
         	case 0:
		 		SendDlgItemMessage (hWndDlg,IDC_PASSTHROUGHTHEMES,BM_SETCHECK,TRUE,0L); 
		 	break;
         	case 1:
		 		SendDlgItemMessage (hWndDlg,IDC_PASSTHROUGHDISPLAY,BM_SETCHECK,TRUE,0L); 
		 	break;
         	case 2:
		 		SendDlgItemMessage (hWndDlg,IDC_PASSTHROUGHNONE,BM_SETCHECK,TRUE,0L); 
		 	break;
		 }
/*		 SendDlgItemMessage (hWndDlg,IDC_SAVEOPTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Do not save"));
		 SendDlgItemMessage (hWndDlg,IDC_SAVEOPTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Prompt for pathname"));
		 SendDlgItemMessage (hWndDlg,IDC_SAVEOPTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Get path from globals"));
    	 SendDlgItemMessage (hWndDlg,IDC_SAVEOPTION,CB_SETCURSEL,CurTheme->HotSpotData.PromptForSave,0);*/
		 SendDlgItemMessage (hWndDlg,IDC_HSGRAN,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"2"));
		 SendDlgItemMessage (hWndDlg,IDC_HSGRAN,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"3"));
		 SendDlgItemMessage (hWndDlg,IDC_HSGRAN,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"5"));
		 SendDlgItemMessage (hWndDlg,IDC_HSGRAN,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"7"));
/*    	 switch (CurTheme->HotSpotData.Granularity)
    	 {
    	 	case 2:
    	 		item=0;
    	 		break;
    	 	case 3:
    	 		item=1;
    	 		break;
    	 	case 5:
    	 		item=2;
    	 		break;
    	 	case 7:
    	 		item=3;
    	 		break;
    	 } */
    	 SendDlgItemMessage (hWndDlg,IDC_HSGRAN,CB_SETCURSEL,item,0);
		 SendDlgItemMessage (hWndDlg,IDC_HSDECAY,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Linear"));
		 SendDlgItemMessage (hWndDlg,IDC_HSDECAY,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Square"));
		 SendDlgItemMessage (hWndDlg,IDC_HSDECAY,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Square Root"));
		 SendDlgItemMessage (hWndDlg,IDC_HSDECAY,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"None"));
    	 SendDlgItemMessage (hWndDlg,IDC_HSDECAY,CB_SETCURSEL,CurTheme->HotSpotData.DecayOpt,0);
		 SendDlgItemMessage (hWndDlg,IDC_HSCOLORDISPLAY,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Linear"));
		 SendDlgItemMessage (hWndDlg,IDC_HSCOLORDISPLAY,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Square Root"));
		 SendDlgItemMessage (hWndDlg,IDC_HSCOLORDISPLAY,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Square"));
		 SendDlgItemMessage (hWndDlg,IDC_HSCOLORDISPLAY,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Log"));
		 SendDlgItemMessage (hWndDlg,IDC_HSCOLORDISPLAY,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Exponential"));
    	 SendDlgItemMessage (hWndDlg,IDC_HSCOLORDISPLAY,CB_SETCURSEL,CurTheme->HotSpotData.ColorOpt,0);
		 
		 for (i=0;i<*pNumViewports;i++)
		 {
			SendDlgItemMessage (hWndDlg,SV_TARGET1,CB_ADDSTRING,NULL,(LPARAM)pViewports[i]->Name);
			SendDlgItemMessage (hWndDlg,SV_TARGET2,CB_ADDSTRING,NULL,(LPARAM)pViewports[i]->Name);
		 }
		 SendDlgItemMessage (hWndDlg,SV_TARGET1,CB_SETCURSEL,CurTheme->TargetViewport-1,NULL);
		 SendDlgItemMessage (hWndDlg,SV_TARGET2,CB_SETCURSEL,CurTheme->ReScan-1,NULL);
		 CurColor1 = CurTheme->ClassColor[0];
		 CurColor2 = CurTheme->ClassColor[1];
		 SendDlgItemMessage (hWndDlg,IDC_TRANTYPE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Least Squares"));
		 SendDlgItemMessage (hWndDlg,IDC_TRANTYPE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Average"));
		 SendDlgItemMessage (hWndDlg,IDC_TRANTYPE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Fixed (TIN)"));
		 SendDlgItemMessage (hWndDlg,IDC_CLWIDTH,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"1"));
		 SendDlgItemMessage (hWndDlg,IDC_CLWIDTH,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"2"));
		 SendDlgItemMessage (hWndDlg,IDC_CLWIDTH,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"3"));
		 SendDlgItemMessage (hWndDlg,IDC_EESTYLE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Opaque"));
		 SendDlgItemMessage (hWndDlg,IDC_EESTYLE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Transparent"));
		 SendDlgItemMessage (hWndDlg,IDC_TRANTYPE,CB_SETCURSEL,CurTheme->DispersePoints,NULL); 
		 SendDlgItemMessage (hWndDlg,IDC_CLWIDTH,CB_SETCURSEL,CurTheme->ValConv,NULL); 
		 SendDlgItemMessage (hWndDlg,IDC_EESTYLE,CB_SETCURSEL,CurTheme->MissOpt,NULL);   
    	 SetDlgItemText (hWndDlg,IDC_COMPARETO,CurTheme->HotSpotCompareTo);
    	 SetDlgItemText (hWndDlg,IDC_SAVETO,CurTheme->HotSpotSaveTo);
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           {
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 EndDialog(hWndDlg, FALSE);
                 break; 
            
            case IDC_CLCOLOR: 
            	 Color = CurColor1;
            	 if (GetColor (hWndDlg,&Color))
            	 	CurColor1 = Color;
            	 break;
            	 
            case IDC_EECOLOR:
            	 Color = CurColor2;
            	 if (GetColor (hWndDlg,&Color))
            	 	CurColor2 = Color;
            	 break;
            	 
            case IDC_SHOW_FIELDS: 
            	 
            	 DisplayFieldList (hWndDlg,CurTheme->hThemeDB,NULL,0);
                 break;
                      
            case IDC_OPEN_DB:
                 EnableWindow (GetDlgItem(hWndDlg,IDC_SETSQL),TRUE);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_SHOW_FIELDS),TRUE);
                 break;  
            
            case IDOK:
            {
           	 	GetDlgItemText (hWndDlg,SV_DATABASE_LIST,CurTheme->DataFile,sizeof(CurTheme->DataFile));
       	 		GetDlgItemText (hWndDlg,IDC_SQL,CurTheme->SQL,256); 
		    	GetDlgItemText (hWndDlg,IDC_COMPARETO,CurTheme->HotSpotCompareTo,128);
		    	GetDlgItemText (hWndDlg,IDC_SAVETO,CurTheme->HotSpotSaveTo,128);
		    	GetDlgItemText (hWndDlg,IDC_HSWEIGHT,CurTheme->IconLibrary,128);
		    	GetDlgItemText (hWndDlg,IDC_HSRADIUS,CurTheme->ClassDefValSQL,128);
		    	CurTheme->HotSpotData.Radius = atof (CurTheme->ClassDefValSQL);
				CurTheme->HotSpotData.PassThrough = 0;
				if (SendDlgItemMessage (hWndDlg,IDC_PASSTHROUGHTHEMES,BM_GETCHECK,0,0))
					CurTheme->HotSpotData.PassThrough = 1;
				else if (SendDlgItemMessage (hWndDlg,IDC_PASSTHROUGHNONE,BM_GETCHECK,0,0))
					CurTheme->HotSpotData.PassThrough = 2;
                CurTheme->TargetViewport=SendDlgItemMessage(hWndDlg,SV_TARGET,CB_GETCURSEL,NULL,NULL)+1; 
				CurTheme->ClassColor[0]=CurColor1;
				CurTheme->ClassColor[1]=CurColor2;
//                CurTheme->HotSpotData.PromptForSave=SendDlgItemMessage(hWndDlg,IDC_SAVEOPTION,CB_GETCURSEL,NULL,NULL);
                CurTheme->HotSpotData.DecayOpt=SendDlgItemMessage(hWndDlg,IDC_HSDECAY,CB_GETCURSEL,NULL,NULL);
                CurTheme->HotSpotData.ColorOpt=SendDlgItemMessage(hWndDlg,IDC_HSCOLORDISPLAY,CB_GETCURSEL,NULL,NULL);
//		    	GetDlgItemText (hWndDlg,IDC_HSMINCOLOR,str,32);
//		    	CurTheme->HotSpotData.ColorMin = atoi(str); 
		    	GetDlgItemText (hWndDlg,IDC_HSGRAN,str,32);
//		    	CurTheme->HotSpotData.Granularity = atoi(str);   
		    	
               	EndDialog(hWndDlg, TRUE);
            }
                break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1239);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1239);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL FAR PASCAL COORDDISPLAYMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1240);
#endif
{	
 char	str[128];
 UINT	idc_units[5]={IDC_UNITS1,IDC_UNITS2,IDC_UNITS3,IDC_UNITS4,IDC_UNITS5};
 UINT	idc_precis[5]={IDC_PRECISION1,IDC_PRECISION2,IDC_PRECISION3,IDC_PRECISION4,IDC_PRECISION5};
 LPCOORDINATEDISPLAY	CD;  
 short	i;
 
 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1240);
#endif
 	return (BRtn);
}
 CD=(LPCOORDINATEDISPLAY)CurTheme;
 switch(Message)
   {
    case WM_INITDIALOG: 
       	 SendDlgItemMessage (hWndDlg,IDC_LINE1,BM_SETCHECK,CD->DisplayLine[0],0L);
       	 SendDlgItemMessage (hWndDlg,IDC_LINE2,BM_SETCHECK,CD->DisplayLine[1],0L);
       	 SendDlgItemMessage (hWndDlg,IDC_LINE3,BM_SETCHECK,CD->DisplayLine[2],0L);  
       	 SendDlgItemMessage (hWndDlg,IDC_LINE4,BM_SETCHECK,CD->DisplayLine[3],0L); 
       	 SendDlgItemMessage (hWndDlg,IDC_SHOWELEVATION,BM_SETCHECK,CD->DisplayElevation,0L); 
       	 SendDlgItemMessage (hWndDlg,IDC_REFRESH,BM_SETCHECK,CD->Refresh,0L);
       	 SendDlgItemMessage (hWndDlg,IDC_COMMAS1,BM_SETCHECK,CD->Commas[0],0L);
       	 SendDlgItemMessage (hWndDlg,IDC_COMMAS2,BM_SETCHECK,CD->Commas[1],0L);
       	 SendDlgItemMessage (hWndDlg,IDC_COMMAS3,BM_SETCHECK,CD->Commas[2],0L);  
       	 SendDlgItemMessage (hWndDlg,IDC_COMMAS4,BM_SETCHECK,CD->ElevCommas,0L);  
       	 SetDlgItemText (hWndDlg,IDC_ID1,CD->LineID[0]);
       	 SetDlgItemText (hWndDlg,IDC_ID2,CD->LineID[1]);
       	 SetDlgItemText (hWndDlg,IDC_ID3,CD->LineID[2]);
       	 SetDlgItemText (hWndDlg,IDC_ID4,CD->LineID[3]);
       	 SetDlgItemText (hWndDlg,IDC_ELEVATIONID,CD->ElevationID);
         _fstrcpy (str,"*.CVT");
         DlgDirListComboBox (hWndDlg,str,IDC_PROJECTION,0,DDL_READWRITE);
         SetDlgItemText (hWndDlg,IDC_PROJECTION,CD->AltCVTFile);
         SetDlgItemText (hWndDlg,IDC_PRINTTEXT,CD->PrintText);
         SetDlgItemText (hWndDlg,IDC_ELEVATIONSURFACE,CD->ElevSurface);
         
         for (i=0;i<5;i++)
         {   
	         SendDlgItemMessage (hWndDlg,idc_units[i],CB_ADDSTRING,0,(LPARAM)"Feet");
	         SendDlgItemMessage (hWndDlg,idc_units[i],CB_ADDSTRING,0,(LPARAM)"Meters"); 
	         SendDlgItemMessage (hWndDlg,idc_units[i],CB_ADDSTRING,0,(LPARAM)"DM");
	         SendDlgItemMessage (hWndDlg,idc_units[i],CB_ADDSTRING,0,(LPARAM)"DMS");
	         SendDlgItemMessage (hWndDlg,idc_units[i],CB_ADDSTRING,0,(LPARAM)"Decimal Degrees"); 
	         SendDlgItemMessage (hWndDlg,idc_units[i],CB_ADDSTRING,0,(LPARAM)"Inches");
	         SendDlgItemMessage (hWndDlg,idc_units[i],CB_ADDSTRING,0,(LPARAM)"Centimeters"); 
	         SendDlgItemMessage (hWndDlg,idc_units[i],CB_ADDSTRING,0,(LPARAM)"MGRS"); 
	         SendDlgItemMessage (hWndDlg,idc_precis[i],CB_ADDSTRING,0,(LPARAM)"1");
	         SendDlgItemMessage (hWndDlg,idc_precis[i],CB_ADDSTRING,0,(LPARAM)"0.1");
	         SendDlgItemMessage (hWndDlg,idc_precis[i],CB_ADDSTRING,0,(LPARAM)"0.01");
	         SendDlgItemMessage (hWndDlg,idc_precis[i],CB_ADDSTRING,0,(LPARAM)"0.001");
	         SendDlgItemMessage (hWndDlg,idc_precis[i],CB_ADDSTRING,0,(LPARAM)"0.0001");
	         SendDlgItemMessage (hWndDlg,idc_precis[i],CB_ADDSTRING,0,(LPARAM)"0.00001");
	         SendDlgItemMessage (hWndDlg,idc_precis[i],CB_ADDSTRING,0,(LPARAM)"0.000001");
	         SendDlgItemMessage (hWndDlg,idc_precis[i],CB_ADDSTRING,0,(LPARAM)"0.0000001");
	         if (i<4)
	         {
				 SendDlgItemMessage(hWndDlg,idc_precis[i],CB_SETCURSEL,CD->Precision[i]-1,NULL);
				 SendDlgItemMessage(hWndDlg,idc_units[i],CB_SETCURSEL,CD->Units[i]-1,NULL);  
			 } 
			 else
	         {
				 SendDlgItemMessage(hWndDlg,idc_precis[i],CB_SETCURSEL,CD->ElevPrecision-1,NULL);
				 SendDlgItemMessage(hWndDlg,idc_units[i],CB_SETCURSEL,CD->ElevUnits-1,NULL);  
			 }
         } 
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           {
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 EndDialog(hWndDlg, FALSE);
                 break; 
            
            case IDC_COORDFONT:  
            	 CD->Font.lfHeight = ConvertFontHeightFromPCTofVP (CD->Font.lfHeight,CD->DisplayViewport);
            	 GetFont (hWndDlg, &CD->Font, &CD->FontColor);   
             	 CD->Font.lfHeight = ConvertFontHeightToPCTofVP (abs(CD->Font.lfHeight),CD->DisplayViewport);

            	 break;   
            
            case IDC_FIND_ELEVATIONSURFACE:
				 if (GetFileName3(hWndDlg,CD->ElevSurface,IDS_FILTERSURFACE,IDS_FILESURFACE))
			         SetDlgItemText (hWndDlg,IDC_ELEVATIONSURFACE,CD->ElevSurface);
            	 break;	 
            case IDOK:
            {
            	 CD->DisplayLine[0] = SendDlgItemMessage (hWndDlg,IDC_LINE1,BM_GETCHECK,0,0L);  
            	 CD->DisplayLine[1] = SendDlgItemMessage (hWndDlg,IDC_LINE2,BM_GETCHECK,0,0L);  
            	 CD->DisplayLine[2] = SendDlgItemMessage (hWndDlg,IDC_LINE3,BM_GETCHECK,0,0L);  
            	 CD->DisplayLine[3] = SendDlgItemMessage (hWndDlg,IDC_LINE4,BM_GETCHECK,0,0L);  
            	 CD->DisplayElevation = SendDlgItemMessage (hWndDlg,IDC_SHOWELEVATION,BM_GETCHECK,0,0L);  
            	 CD->Refresh = SendDlgItemMessage (hWndDlg,IDC_REFRESH,BM_GETCHECK,0,0L);  
            	 CD->Commas[0] = SendDlgItemMessage (hWndDlg,IDC_COMMAS1,BM_GETCHECK,0,0L);  
            	 CD->Commas[1] = SendDlgItemMessage (hWndDlg,IDC_COMMAS2,BM_GETCHECK,0,0L);  
            	 CD->Commas[2] = SendDlgItemMessage (hWndDlg,IDC_COMMAS3,BM_GETCHECK,0,0L);  
            	 CD->ElevCommas = SendDlgItemMessage (hWndDlg,IDC_COMMAS4,BM_GETCHECK,0,0L);  
		       	 GetDlgItemText (hWndDlg,IDC_ID1,CD->LineID[0],32);
		       	 GetDlgItemText (hWndDlg,IDC_ID2,CD->LineID[1],32);
		       	 GetDlgItemText (hWndDlg,IDC_ID3,CD->LineID[2],32);
		       	 GetDlgItemText (hWndDlg,IDC_ID4,CD->LineID[3],32);
		       	 GetDlgItemText (hWndDlg,IDC_ELEVATIONID,CD->ElevationID,32);
         		 GetDlgItemText (hWndDlg,IDC_PRINTTEXT,CD->PrintText,sizeof(CD->PrintText));
         		 GetDlgItemText (hWndDlg,IDC_ELEVATIONSURFACE,CD->ElevSurface,sizeof(CD->ElevSurface));
		       	 for (i=0;i<4;i++) 
		       	 {
		       	 	CD->Units[i]=SendDlgItemMessage(hWndDlg,idc_units[i],CB_GETCURSEL,NULL,NULL)+1;
		       	 	CD->Precision[i]=SendDlgItemMessage(hWndDlg,idc_precis[i],CB_GETCURSEL,NULL,NULL)+1;
		       	 }
		       	 CD->ElevUnits=SendDlgItemMessage(hWndDlg,idc_units[4],CB_GETCURSEL,NULL,NULL)+1;
		       	 CD->ElevPrecision=SendDlgItemMessage(hWndDlg,idc_precis[4],CB_GETCURSEL,NULL,NULL)+1;
                 DTMClose (&CD->hSurf);
		       	 if (CD->DisplayLine[2])
		       	 {  
		       	 	GetDlgItemText (hWndDlg,IDC_PROJECTION,CD->AltCVTFile,sizeof(CD->AltCVTFile));
	                SetGlobalValue("%ALT_PROJECTION",CD->AltCVTFile);
				    ConvertCoordClose ();
					ConvertCoordInit();  
		       	 }
		       	 
               	 EndDialog(hWndDlg, TRUE);
               	 
            }
                break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1240);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1240);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
} 

HANDLE CreateClassValueList(void)
#if ENABLETRACE
{GSSiEnterProg (1241);
#endif
{

	BTVARDESC	BTVar[3];
	int	i, ifield, len;
	HANDLE	hBT;
						
	BT_CLOSEANDDELETE (&CurTheme->hScatterFile);
	if (CurTheme->FieldFun)
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

BOOL FAR PASCAL ASSIGN_NONNUMERICMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1243);
#endif
{
	COLORREF	Color; 
	static	BOOL	FirstAfterScroll=FALSE, InRedraw=FALSE;
	int		pos, ClassNo, i,index, x,y,width,height,margin,bwidth,bheight,idwidth,idheight,twidth,theight;
	char	Value[256]; 
	static	int	from=0,to=0, FirstOnScreen, NumOnScreen; 
	RECT	Rect, Border, DlgRect; 
	static	WORD	CurFocus=0;
	short	lCurKey, lNewKey;
   	HCURSOR	hcurSave; 
    char	str[512];
	char	SymName[36];
    
 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1243);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG: 
        
        FirstOnScreen = 0; 
        *DumpFileName = 0;
        _fstrcpy (OldValListFile,CurTheme->ScatterFile);
        
		BT_CLOSE (CurTheme->hScatterFile);
		CurTheme->hScatterFile = 0;
		if (CurTheme->NumDesiredClass < 17) 
	   		ShowWindow (GetDlgItem(hWndDlg,IDC_SCROLL),SW_HIDE);
	   	else
	   		SetScrollRange (GetDlgItem(hWndDlg,IDC_SCROLL),SB_CTL,0,CurTheme->NumDesiredClass-16,TRUE);  
		for (i=0;i<CurTheme->NumDesiredClass;i++)
		{
			SetDlgItemText(hWndDlg,TitleCntl[i],CurTheme->ClassBM[i]); 
		   	itoa (i+1,Value,10);
		   	SetDlgItemText (hWndDlg,ClassIDCntl[i],Value); 
    	}
    case GSSI_REINITDIALOG:   
    	InRedraw = TRUE;
    	for (i=0;i<48;i++)
    	{	
		   	ShowWindow (GetDlgItem(hWndDlg,ClassField[i+1]),SW_HIDE);
		   	ShowWindow (GetDlgItem(hWndDlg,ClassIDCntl[i]),SW_HIDE);
		   	ShowWindow (GetDlgItem(hWndDlg,TitleCntl[i]),SW_HIDE); 
		}
        NumOnScreen = min (16,CurTheme->NumDesiredClass - FirstOnScreen);
		GetWindowRect(hWndDlg,&DlgRect);
		GetWindowRect(GetDlgItem(hWndDlg,IDC_BORDER),&Rect);
		Rect.left -= DlgRect.left;
		Rect.right -= DlgRect.left;
		Rect.top -= DlgRect.top;
		Rect.bottom -= DlgRect.top;
		Border = Rect;
		bwidth = Rect.right - Rect.left + 1;
		bheight = Rect.bottom - Rect.top + 1;
		GetWindowRect(GetDlgItem(hWndDlg,IDC_CLASSID1),&Rect);
		idwidth = Rect.right - Rect.left + 1;
		idheight = Rect.bottom - Rect.top + 1;
		GetWindowRect(GetDlgItem(hWndDlg,IDC_TITLE1),&Rect);
		twidth = Rect.right - Rect.left + 1;
		theight = Rect.bottom - Rect.top + 1;
		margin = 4; 
		y = Border.top - theight/2 + margin;
		x = Border.left + margin;
		if (CurTheme->NumDesiredClass > 12)
		{   
			width = (bwidth - margin*10) / 4;
			height = (bheight - margin*12) / 4;
			for (i = 0; i<16; i++)
			{   POINT	pt;
			
				pt.x = x;
				pt.y = y+(theight+2);
				MoveWindow(GetDlgItem(hWndDlg,ClassField[i+1+FirstOnScreen]),pt.x,pt.y, width, height-(theight+2), FALSE);
				MoveWindow(GetDlgItem(hWndDlg,ClassIDCntl[i+FirstOnScreen]),pt.x,y, idwidth, idheight, FALSE);
				MoveWindow(GetDlgItem(hWndDlg,TitleCntl[i+FirstOnScreen]),pt.x+idwidth,y, width-idwidth, theight, FALSE);
				x += (width + margin*2);
				if (i == 3 || i == 7 || i == 11)
				{
					x = Border.left + margin;
					y += (height + margin*2);
				} 
			}
		}
		else if (NumOnScreen < 5)
		{   
			width = (bwidth - margin*5) / 2;
			height = (bheight - margin*8) / 2;
			for (i = 0; i<5; i++)
			{   POINT	pt;
			
				pt.x = x;
				pt.y = y+(theight+2);
				MoveWindow(GetDlgItem(hWndDlg,ClassField[i+1+FirstOnScreen]),pt.x,pt.y, width, height-(theight+2), FALSE);
				MoveWindow(GetDlgItem(hWndDlg,ClassIDCntl[i+FirstOnScreen]),pt.x,y, idwidth, idheight, FALSE);
				MoveWindow(GetDlgItem(hWndDlg,TitleCntl[i+FirstOnScreen]),pt.x+idwidth,y, width-idwidth, theight, FALSE);
				x += (width + margin*2);
				if (i == 1)
				{
					x = Border.left + margin;
					y += (height + margin*2);
				} 
			}
		}
		else if (NumOnScreen < 7)
		{   
			width = (bwidth - margin*7) / 3;
			height = (bheight - margin*8) / 2;
			for (i = 0; i<7; i++)
			{   POINT	pt;
			
				pt.x = x;
				pt.y = y+(theight+2);
				MoveWindow(GetDlgItem(hWndDlg,ClassField[i+1+FirstOnScreen]),pt.x,pt.y, width, height-(theight+2), FALSE);
				MoveWindow(GetDlgItem(hWndDlg,ClassIDCntl[i+FirstOnScreen]),pt.x,y, idwidth, idheight, FALSE);
				MoveWindow(GetDlgItem(hWndDlg,TitleCntl[i+FirstOnScreen]),pt.x+idwidth,y, width-idwidth, theight, FALSE);
				x += (width + margin*2);
				if (i == 2)
				{
					x = Border.left + margin;
					y += (height + margin*2);
				} 
			}
		}
		else if (NumOnScreen < 10)
		{   
			width = (bwidth - margin*7) / 3;
			height = (bheight - margin*10) / 3;
			for (i = 0; i<10; i++)
			{   POINT	pt;
			
				pt.x = x;
				pt.y = y+(theight+2);
				MoveWindow(GetDlgItem(hWndDlg,ClassField[i+1+FirstOnScreen]),pt.x,pt.y, width, height-(theight+2), FALSE);
				MoveWindow(GetDlgItem(hWndDlg,ClassIDCntl[i+FirstOnScreen]),pt.x,y, idwidth, idheight, FALSE);
				MoveWindow(GetDlgItem(hWndDlg,TitleCntl[i+FirstOnScreen]),pt.x+idwidth,y, width-idwidth, theight, FALSE);
				x += (width + margin*2);
				if (i == 2 || i == 5)
				{
					x = Border.left + margin;
					y += (height + margin*2);
				} 
			}
		}
		else if (NumOnScreen < 13)
		{   
			width = (bwidth - margin*7) / 3;
			height = (bheight - margin*12) / 4;
			for (i = 0; i<13; i++)
			{   POINT	pt;
			
				pt.x = x;
				pt.y = y+(theight+2);
				MoveWindow(GetDlgItem(hWndDlg,ClassField[i+1+FirstOnScreen]),pt.x,pt.y, width, height-(theight+2), FALSE);
				MoveWindow(GetDlgItem(hWndDlg,ClassIDCntl[i+FirstOnScreen]),pt.x,y, idwidth, idheight, FALSE);
				MoveWindow(GetDlgItem(hWndDlg,TitleCntl[i+FirstOnScreen]),pt.x+idwidth,y, width-idwidth, theight, FALSE);
				x += (width + margin*2);
				if (i == 2 || i == 5 || i == 8)
				{
					x = Border.left + margin;
					y += (height + margin*2);
				} 
			}
		}
    	for (i=0;i<48;i++)
    	{	
    		if (i >= FirstOnScreen && i <= FirstOnScreen + NumOnScreen - 1)
    		{
			   	ShowWindow (GetDlgItem(hWndDlg,ClassField[i+1]),SW_SHOW);
			   	ShowWindow (GetDlgItem(hWndDlg,ClassIDCntl[i]),SW_SHOW);
			   	ShowWindow (GetDlgItem(hWndDlg,TitleCntl[i]),SW_SHOW);
			}  
		} 
		InRedraw = FALSE;
		if (Message == GSSI_REINITDIALOG)
			break;
Update:
		for (i=0;i<CurTheme->NumDesiredClass+1;i++)
			SendDlgItemMessage (hWndDlg,ClassField[i],LB_RESETCONTENT,NULL,NULL);  
			
		SendDlgItemMessage (hWndDlg,ClassField[CurTheme->AllValueClass],LB_ADDSTRING,NULL,(LPARAM)"(All Other Values)");
		if (*CurTheme->ScatterFile)
		{    
		 	 CurTheme->hScatterFile = BT_OPEN (CurTheme->ScatterFile, 0, BT_READ, 0); 
			 pos = BT_FIRST;
	    	 while (!BT_FIND (CurTheme->hScatterFile,Value,pos,BT_ANY,(LPSTR)&ClassNo))
	    	 {
	    	 	pos = BT_NEXT;
	    	 	Value[GetBTKeyLen(CurTheme->hScatterFile)]='\0';  
	    	 	if (!*Value)
	    	 		_fstrcpy (Value,"(NULL Field)"); 
	    	 	if (ClassNo <0 || ClassNo > CurTheme->NumDesiredClass)
	    	 		ClassNo = 0;
				SendDlgItemMessage (hWndDlg,ClassField[ClassNo],LB_ADDSTRING,NULL,(LPARAM)Value);
			 }
       		 BT_CLOSE (CurTheme->hScatterFile);
       		 CurTheme->hScatterFile = 0;
       		 if (!_fstricmp (CurTheme->ScatterFile,OldValListFile))
       		 	*CurTheme->ScatterFile = 0;
    	 } 
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

 	case WM_VSCROLL: 
 		switch (wParam)
 		{
			case SB_BOTTOM: 
				FirstOnScreen = CurTheme->NumDesiredClass - 16;
				break;
			case SB_LINEDOWN: 
				FirstOnScreen++;
				break;
			case SB_LINEUP: 
				FirstOnScreen--;
				break;
			case SB_PAGEDOWN:
				FirstOnScreen += 4;
				break; 
			case SB_PAGEUP:
				FirstOnScreen -= 4; 
			case SB_THUMBPOSITION://	Scroll to absolute position. The current position is specified by the nPos parameter. 
			case SB_THUMBTRACK:	 //Drag scroll box (thumb) to specified position. The current position is specified by the nPos parameter. 
				FirstOnScreen = LOWORD (lParam);
				break;
			case SB_TOP:
				FirstOnScreen = 0;
				break;
		}
		FirstOnScreen = max (0,min (CurTheme->NumDesiredClass - 16,FirstOnScreen));
		FirstAfterScroll = TRUE;
		SetScrollPos (GetDlgItem(hWndDlg,IDC_SCROLL),SB_CTL,FirstOnScreen,TRUE);	 		 
		PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
		break;
		
    case WM_COMMAND: 
    	 switch (wParam)
    	 {  
    	 	case IDC_UNASSIGNED:
    	 	case IDC_CLASS1:
    	 	case IDC_CLASS2:
    	 	case IDC_CLASS3:
    	 	case IDC_CLASS4:
    	 	case IDC_CLASS5:
    	 	case IDC_CLASS6:
    	 	case IDC_CLASS7:
    	 	case IDC_CLASS8:
    	 	case IDC_CLASS9:
    	 	case IDC_CLASS10:
    	 	case IDC_CLASS11:
    	 	case IDC_CLASS12:
    	 	case IDC_CLASS13:
    	 	case IDC_CLASS14:
    	 	case IDC_CLASS15:
    	 	case IDC_CLASS16:
    	 	case IDC_CLASS17:
    	 	case IDC_CLASS18:
    	 	case IDC_CLASS19:
    	 	case IDC_CLASS20:
    	 	case IDC_CLASS21:
    	 	case IDC_CLASS22:
    	 	case IDC_CLASS23:
    	 	case IDC_CLASS24:
    	 	case IDC_CLASS25:
    	 	case IDC_CLASS26:
    	 	case IDC_CLASS27:
    	 	case IDC_CLASS28:
    	 	case IDC_CLASS29:
    	 	case IDC_CLASS30:
    	 	case IDC_CLASS31:
    	 	case IDC_CLASS32:
    	 	case IDC_CLASS33:
    	 	case IDC_CLASS34:
    	 	case IDC_CLASS35:
    	 	case IDC_CLASS36:
    	 	case IDC_CLASS37:
    	 	case IDC_CLASS38:
    	 	case IDC_CLASS39:
    	 	case IDC_CLASS40:
    	 	case IDC_CLASS41:
    	 	case IDC_CLASS42:
    	 	case IDC_CLASS43:
    	 	case IDC_CLASS44:
    	 	case IDC_CLASS45:
    	 	case IDC_CLASS46:
    	 	case IDC_CLASS47:
    	 	case IDC_CLASS48:
	    	{
	            switch(HIWORD(lParam)) 
	            {
				 case LBN_DBLCLK:   
	    	 		from = wParam;  
	    	 		to = 0;
				 	PostMessage(hWndDlg,WM_COMMAND,IDC_EDIT,0L);
				 	break;
				 	
				 break;
				 
	              case LBN_SELCHANGE:
	              	if (to)
	              	{
						SendDlgItemMessage (hWndDlg,wParam,LB_SETSEL,FALSE,(LPARAM)-1);
						to = 0;
					} 
	              	else
	              	{
						short n = SendDlgItemMessage(hWndDlg,wParam,LB_GETSELCOUNT,NULL,NULL);
						if (n)
						{
		            		EnableWindow (GetDlgItem(hWndDlg,IDC_DESELECT),TRUE); 
		            		EnableWindow (GetDlgItem(hWndDlg,IDC_DELETE),TRUE); 
		            	}
		            	else
		            		EnableWindow (GetDlgItem(hWndDlg,IDC_EDIT),FALSE); 
		            	if (n == 1)
		            		EnableWindow (GetDlgItem(hWndDlg,IDC_EDIT),TRUE); 
		            }
	/*	    	 	if (!from && CurFocus == wParam)
		    	 		from = wParam;  
		    	 	else
		    	 		from = 0;*/
		    	 	break;
		    	 	
		    	  case LBN_KILLFOCUS: 
		    	  	if (InRedraw && !FirstAfterScroll)
		    	  		break;
/*	    	 		if (FirstAfterScroll)
	    	 		{
	    	 			FirstAfterScroll = FALSE;
	    	 			break;
	    	 		} */
    	 			FirstAfterScroll = FALSE;
		    	  	CurFocus = 0;
	    	 		from = wParam;  
	    	 		to = 0;
		    	  	break;
		    	  	
		    	  case LBN_SETFOCUS:
		    	 	{   
			    	  	if (InRedraw)
			    	  		break;
		    	 	/*	if (FirstAfterScroll)
		    	 		{
		    	 			break;
		    	 		}   */
		    	 		CurFocus = wParam; 
		    	 		if (hVALUELIST)
		    	 		{   
			           		LPSTR pVals = GlobalLock (hVALUELIST);
			           		
			           		while (*pVals)
			           		{
								SendDlgItemMessage (hWndDlg,CurFocus,LB_ADDSTRING,NULL,(LPARAM)pVals); 
								pVals = _fstrchr (pVals,0);
								pVals++;
			           		}
			           		GSSiGlobUlFree (&hVALUELIST);
			    	 		from = 0; 
			    	 		to = 0;
			           		break; 
		    	 		}
		    	 		if (!from)
		    	 			break;  
		    	 		if (from != wParam)
		    	 		{ 
		    	 			int nItems;
		    	 			HANDLE	hItems;
		    	 			LPSHORT	lpItems;
		    	 				    	 			
			    	 		CurFocus = 0;
							nItems=SendDlgItemMessage(hWndDlg,from,LB_GETSELCOUNT,NULL,NULL); 
							if (!nItems) break;
			    	 		to = wParam;
							hItems=GSSiGlobAlloc ( 634,GHND,nItems*2);
							lpItems=  (LPSHORT) GlobalLock(hItems);
							SendDlgItemMessage(hWndDlg,from,LB_GETSELITEMS,nItems,(LPARAM)lpItems); 
							for (i=0;i<nItems;i++,lpItems++)
							{
						        SendDlgItemMessage(hWndDlg,from,LB_GETTEXT, 
						         		  	     	    *lpItems,(LPARAM)((LPSTR)Value)); 
								SendDlgItemMessage (hWndDlg,to,LB_ADDSTRING,NULL,(LPARAM)Value); 
							}
							GlobalUnlock (hItems);
							lpItems=  (LPSHORT) GlobalLock(hItems);
							lpItems += (nItems-1);
							for (i=0;i<nItems;i++)
							{
						        SendDlgItemMessage(hWndDlg,from,LB_DELETESTRING,*lpItems--,NULL); 
							} 
							GSSiGlobUlFree (&hItems);
			    	 		from = 0; 
			    	 		to = 0;
		    	 		}
		    	 	}	   
	    	 	  break;
	    	 	}    
    	 	                 
    	 	} 
    	 	break;
	    	case IDC_DESELECT: 
	    	 	if (from)
					SendDlgItemMessage (hWndDlg,from,LB_SETSEL,FALSE,(LPARAM)-1); 
	            EnableWindow (GetDlgItem(hWndDlg,IDC_DESELECT),FALSE);
	            EnableWindow (GetDlgItem(hWndDlg,IDC_DELETE),FALSE);  
	            EnableWindow (GetDlgItem(hWndDlg,IDC_EDIT),FALSE);  
	            from = 0;
	    	 break;    
	    	 
           	case IDC_COLOR:
           		 Color = NewColor;  
            	 if (GetColor(hWndDlg,&Color))
            	 	NewColor = Color;
           		 break;
           	
           	case IDC_DELETE:
	 		{ 
	 			int nItems;
	 			HANDLE	hItems;
	 			LPSHORT	lpItems;
	    	 				    	 			
				nItems=SendDlgItemMessage(hWndDlg,from,LB_GETSELCOUNT,NULL,NULL); 
				if (!nItems) break;
				hItems=GSSiGlobAlloc ( 635,GHND,nItems*2);
				lpItems=  (LPSHORT) GlobalLock(hItems);
				SendDlgItemMessage(hWndDlg,from,LB_GETSELITEMS,nItems,(LPARAM)lpItems); 
				lpItems += (nItems-1);
				for (i=0;i<nItems;i++)
				{
			        SendDlgItemMessage(hWndDlg,from,LB_DELETESTRING,*lpItems--,NULL); 
				} 
				GSSiGlobUlFree (&hItems);
		        PostMessage(hWndDlg, WM_COMMAND, IDC_DESELECT, 0L);
           	}
           		break;
           	
           	case IDC_EDIT:
	 		{ 
	 			int nItems;
	 			short	item;
	    	 				    	 			
				nItems=SendDlgItemMessage(hWndDlg,from,LB_GETSELCOUNT,NULL,NULL); 
				if (nItems != 1) break;
				SendDlgItemMessage(hWndDlg,from,LB_GETSELITEMS,1,(LPARAM)&item); 
		        SendDlgItemMessage(hWndDlg,from,LB_GETTEXT,item,(LPARAM)((LPSTR)Value));
				if (GetTextString (hWndDlg,Value,256,"Enter new value",NULL,NULL,0,TRUE,TRUE)) 		         
				{
			        SendDlgItemMessage(hWndDlg,from,LB_DELETESTRING,item,NULL); 
					SendDlgItemMessage (hWndDlg,from,LB_ADDSTRING,NULL,(LPARAM)Value);
				} 
		        PostMessage(hWndDlg, WM_COMMAND, IDC_DESELECT, 0L);
           	}
           		break;
           	
           	case IDC_CLEAR:
				for (i=0;i<CurTheme->NumDesiredClass+1;i++)
					SendDlgItemMessage (hWndDlg,ClassField[i],LB_RESETCONTENT,NULL,NULL);
           		break;
           		
           	case IDC_MANADD:
           		if (GetDlgItemText (hWndDlg,IDC_MANVAL,Value,sizeof(Value)))
					SendDlgItemMessage (hWndDlg,IDC_UNASSIGNED,LB_ADDSTRING,NULL,(LPARAM)Value); 
		        break;
		    
		    case IDC_MANVAL:
	            EnableWindow (GetDlgItem(hWndDlg,IDC_MANADD),
           			GetDlgItemText (hWndDlg,IDC_MANVAL,Value,sizeof(Value)));
           		break;
           	
           	case IDC_FILLFROM:
           		
           		GetValsFromTable (hWndDlg);
           		break;
           			
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */ 
        		 if (_fstricmp (CurTheme->ScatterFile,OldValListFile))
           		 	GSSiRemove (CurTheme->ScatterFile);
        		 _fstrcpy (CurTheme->ScatterFile,OldValListFile);
				 DestroySELECTVALUES();
				 GSSiRemove (DumpFileName);
                 EndDialog(hWndDlg, FALSE);
                 break;
            
            case IDC_PRINTCLASSDEF:
            {
            	 HFILE	Fid;
            	 
            	 GetWindowText (GetDlgItem(hWndDlg,IDC_PRINTCLASSDEF),str,32);
            	 if (!_fstricmp (str,"Reload File"))
            	 {
	                 Fid = GSSiOpenFile (DumpFileName,NULL,OF_READ);
	                 ClassNo = -1;
	                 while (fgetstring (str,500,Fid))
	                 {  
	                 	if (*str == '\t')
	                 		*str = ' ';
	                 	if (*str == ' ')
	                 	{   
	                 		LPSTR	pNB = FirstNonBlank(str);
	                 		
	                 		if (pNB && ClassNo >= 0 && ClassNo <= MAX_THEME_CLASSES) 
								SendDlgItemMessage(hWndDlg,ClassField[ClassNo],LB_ADDSTRING,NULL,(LPARAM)pNB); 
	                 	}
	                 	else
	                 	{   
	                 		ClassNo++;
	                 		if (ClassNo) 
	                 		{   
	                 			LPSTR	pSC = _fstrchr (str,';');
	                 			
	                 			if (*pSC)
	                 			{   
	                 				SetGlobalValueLong ("%COLOR",0);
	                 				SetGlobalValue ("%FONT",""); 
	                 				SetGlobalValue ("%CHR",""); 
	                 				SetGlobalValue ("%SYMNAME",""); 
	                 				*pSC++ = 0; 
	                 				ExpandText (pSC);
	                 				CurTheme->ClassColor[ClassNo-1] = GetGlobalLVal ("[%COLOR]");
	                 				if (GetGlobalCVal ("[%SYMNAME]",SymName,0))
	                 				{   
	                 					*CurTheme->SymbolFont[0] = 0;
	                 					CurTheme->ClassSymbol[ClassNo-1] = GetDictSymbolNumber (SymName);
	                 				}
	                 			}
								SetDlgItemText(hWndDlg,TitleCntl[ClassNo-1],str);
							}
					        SendDlgItemMessage (hWndDlg,ClassField[ClassNo],LB_RESETCONTENT,NULL,NULL);
	                 	}
	                 }
	                 CurTheme->NumDesiredClass = ClassNo; 
					 GSSiClose (Fid);
					 SetWindowText (GetDlgItem(hWndDlg,IDC_PRINTCLASSDEF),"Dump to File"); 
            	 }
            	 else
            	 {
	       	 		 GSSiGetTempFileName (NULL,"gm",NULL,(LPSTR)DumpFileName);
	                 Fid = GSSiOpenFile (DumpFileName,NULL,OF_CREATE);
					 for (ClassNo=0;ClassNo<CurTheme->NumDesiredClass+1;ClassNo++) 
					 {  
					    if (ClassNo)
					    {
							short r = GetRValue(CurTheme->ClassColor[ClassNo-1]);
							short g = GetGValue(CurTheme->ClassColor[ClassNo-1]);
							short b = GetBValue(CurTheme->ClassColor[ClassNo-1]);
							GetDlgItemText(hWndDlg,TitleCntl[ClassNo-1],str,128);
							if (CurTheme->DataType == 1)
							{
								
								if (*CurTheme->SymbolFont[0])
								{
									short ifont = CurTheme->ClassSymbol[ClassNo-1] / 1000;
									sprintf (_fstrchr (str,0),";[%%FONT]=%s;[%%CHR]=%i",CurTheme->SymbolFont[ifont],CurTheme->ClassSymbol[ClassNo-1]);
								}
								else
								{
									GetDictSymName (CurTheme->ClassSymbol[ClassNo-1],SymName);
									sprintf (_fstrchr (str,0),";[%%SYMNAME]=%s",SymName);
								} 
								sprintf (_fstrchr (str,0),";[%%COLOR]=$RGB(%i,%i,%i)",r,g,b); 
							}
						}
						else
							_fstrcpy (str,"Unassigned");
						fputstring (str,Fid);
						index = 0;   
						*str = '\t';
						while (SendDlgItemMessage(hWndDlg,ClassField[ClassNo],LB_GETTEXT, 
						         		  	      index++,(LPARAM)((LPSTR)&str[1])) != LB_ERR)
							fputstring (str,Fid);
					 }
					 GSSiClose (Fid);
					 EditTextFile (NULL,DumpFileName); 
					 SetWindowText (GetDlgItem(hWndDlg,IDC_PRINTCLASSDEF),"Reload File"); 
				 }  
			}
            	 break;
            	       
           	case IDC_GET_UNASSIGNED:
            case IDOK:
                 
				 hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
    			 CurTheme->hScatterFile = CreateClassValueList(); 
    			 CurTheme->AllValueClass = 0;
				 for (ClassNo=0;ClassNo<CurTheme->NumDesiredClass+1;ClassNo++) 
				 {  
				    
				    if (ClassNo)
				    {
				    	CurTheme->ClassCount[ClassNo-1]=0;
						GetDlgItemText(hWndDlg,TitleCntl[ClassNo-1],CurTheme->ClassBM[ClassNo-1],128);
					}
					index = 0;
					while (SendDlgItemMessage(hWndDlg,ClassField[ClassNo],LB_GETTEXT, 
					         		  	      index,(LPARAM)((LPSTR)str)) != LB_ERR)
					{   
			    	 	if (!_fstrcmp (str,"(NULL Field)"))
			    	 		*str = 0;
			    	 	if (!_fstrcmp (str,"(All Other Values)"))
			    	 		CurTheme->AllValueClass = ClassNo;
			    	 	else
			    	 	{
							_fstrncpy (Value,str,GetBTKeyLen(CurTheme->hScatterFile)); 
			    			BT_PUT (CurTheme->hScatterFile,Value,(LPSTR)&ClassNo);
			    		} 
						index++;
					    if (ClassNo)
					    	CurTheme->ClassCount[ClassNo-1]=1; //indicates we have values
					} 

                 }
				 GSSiSetCursor(hcurSave);
           		 if (wParam == IDC_GET_UNASSIGNED)
           		 {
	    			UpdateClassValueList();
					BT_CLOSE (CurTheme->hScatterFile);
					CurTheme->hScatterFile = 0;
	    			goto Update;
	    		 }
           		 BT_CLOSE (CurTheme->hScatterFile);
           		 CurTheme->hScatterFile = 0;
           		 GSSiRemove (OldValListFile);
				 DestroySELECTVALUES();
				 GSSiRemove (DumpFileName);
                 EndDialog(hWndDlg, TRUE);
                 break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1243);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1243);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}
    
BOOL FAR PASCAL CLASS_RANGESMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1244);
#endif
{
	int		i,ndp; 
	char	str[128]; 
	double	rndp; 
	static	TopClass;

 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1244);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG: 
        TopClass = 0;   
        for (i=0;i<CurTheme->NumDesiredClass;i++)
        {
        	SaveCMin[i] = CurTheme->ClassMin[i];
        	SaveCMax[i] = CurTheme->ClassMax[i];  
        	_fstrcpy (SaveCTitle[i],CurTheme->ClassBM[i]);
        }
        SetDlgItemText (hWndDlg,IDC_REFERENCEVAL,CurTheme->RefValChar);
       	SendDlgItemMessage (hWndDlg,IDC_RANGEAREPCT,BM_SETCHECK,CurTheme->RefIsPCT,0L); 
       	if (CurTheme->NumDesiredClass > 16) 
       	{
       		ShowWindow (GetDlgItem(hWndDlg,IDC_UP),SW_SHOW);
       		ShowWindow (GetDlgItem(hWndDlg,IDC_DOWN),SW_SHOW);  
       	}
    case GSSI_REINITDIALOG:  
    	if (TopClass)  
    	{
       		EnableWindow (GetDlgItem(hWndDlg,IDC_UP),TRUE);
       		EnableWindow (GetDlgItem(hWndDlg,IDC_AUTOINC),FALSE); 
       	}
       	else 
       	{
       		EnableWindow (GetDlgItem(hWndDlg,IDC_AUTOINC),TRUE); 
       		EnableWindow (GetDlgItem(hWndDlg,IDC_UP),FALSE);
       	} 
    	if (TopClass + 16 < CurTheme->NumDesiredClass)	
       		EnableWindow (GetDlgItem(hWndDlg,IDC_DOWN),TRUE);
       	else 
       		EnableWindow (GetDlgItem(hWndDlg,IDC_DOWN),FALSE); 
    	for (i=0;i<16;i++)
    	{	
    		BOOL	show=SW_SHOW;
    		
    		if (TopClass + i >= CurTheme->NumDesiredClass)
    			show = SW_HIDE;
		   	ShowWindow (GetDlgItem(hWndDlg,TitleCntl[i]),show);
		   	ShowWindow (GetDlgItem(hWndDlg,ClassIDCntl[i]),show);
		   	ShowWindow (GetDlgItem(hWndDlg,FromCntl[i]),show);
		   	ShowWindow (GetDlgItem(hWndDlg,ToCntl[i]),show);
		   	ShowWindow (GetDlgItem(hWndDlg,SepCntl[i]),show);
		}
		
		for (i=0;i<16;i++) 
		{   
			short	j = TopClass + i;
			 
			if (j < CurTheme->NumDesiredClass)
			{
				SetDlgItemText(hWndDlg,TitleCntl[i],CurTheme->ClassBM[j]);
				if (CurTheme->RoundTo<1)
				{
		            rndp = log10(CurTheme->RoundTo); 
		            ndp = abs((short)IDNINT(rndp))+1; 
		        }
				else
					ndp = 0;				
				RWRITE (CurTheme->ClassMin[j],ndp,str);
				SetDlgItemText(hWndDlg,FromCntl[i],str);
				RWRITE (CurTheme->ClassMax[j],ndp,str);
				SetDlgItemText(hWndDlg,ToCntl[i],str); 
				itoa (j+1,str,10);
				SetDlgItemText(hWndDlg,ClassIDCntl[i],str); 
			}
		}
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
    	 switch(wParam)
           {
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
		        for (i=0;i<CurTheme->NumDesiredClass;i++)
		        {
		        	CurTheme->ClassMin[i] = SaveCMin[i];
		        	CurTheme->ClassMax[i] = SaveCMax[i];  
		        	_fstrcpy (CurTheme->ClassBM[i],SaveCTitle[i]);
		        }
                EndDialog(hWndDlg, FALSE);
                break;
            
            case IDC_AUTOINC: 
            {
            	double	inc;
            	
            	*str = 0;
				if (!GetTextString (hWndDlg,str,16,"Enter the auto increment value",NULL,NULL,0,TRUE,TRUE))
					break;
				inc = atof (str); 
				GetDlgItemText(hWndDlg,FromCntl[0],str,32);
				rread (str,&CurTheme->ClassMin[0],&ndp);
				GetDlgItemText(hWndDlg,ToCntl[0],str,32);
				rread (str,(LPDOUBLE)&CurTheme->ClassMax[0],&ndp); 
				for (i=0;i<CurTheme->NumDesiredClass;i++)
				{
					CurTheme->ClassMin[i] = CurTheme->ClassMin[0] + inc * i;
					CurTheme->ClassMax[i] = CurTheme->ClassMax[0] + inc * i;
					if (CurTheme->RoundTo<1)
					{
			            rndp = log10(CurTheme->RoundTo); 
			            ndp = abs((short)IDNINT(rndp))+1; 
			        }
					else
						ndp = 0;				
					RWRITE (CurTheme->ClassMin[i],ndp,str);
					_fstrcat (str," to ");
					RWRITE (CurTheme->ClassMax[i],ndp,_fstrchr(str,0));
					_fstrcpy (CurTheme->ClassBM[i],str);
				}	
		        PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
			}
            	break;
            	
            case IDC_UP:
            case IDC_DOWN:      
            case IDOK:
				 for (i=0;i<16;i++) 
				 {  char	str[256]; 
				 	int		ndp;
					short	j = TopClass + i;
					 
					if (j < CurTheme->NumDesiredClass)
					{
						GetDlgItemText(hWndDlg,TitleCntl[i],CurTheme->ClassBM[j],128);
						GetDlgItemText(hWndDlg,FromCntl[i],str,32);
						rread (str,&CurTheme->ClassMin[j],&ndp);
						GetDlgItemText(hWndDlg,ToCntl[i],str,32);
						rread (str,(LPDOUBLE)&CurTheme->ClassMax[j],&ndp); 
					}
                 }
                 if (wParam == IDC_UP)
                 {
                 	TopClass = max (0,TopClass - 16);
			        PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
			        break;
			     }
                 if (wParam == IDC_DOWN)
                 {
                 	TopClass = min (CurTheme->NumDesiredClass-16,TopClass + 16);
			        PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
			        break;
			     }
                 	
            	 CurTheme->RefIsPCT = SendDlgItemMessage (hWndDlg,IDC_RANGEAREPCT,BM_GETCHECK,0,0L); 
                 GetDlgItemText (hWndDlg,IDC_REFERENCEVAL,CurTheme->RefValChar,35);
                 Truncate (CurTheme->RefValChar);
                 EndDialog(hWndDlg, TRUE);
                 break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1244);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1244);
#endif
 return TRUE;
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
		 Fid = GSSiOpenFile (SaveName,NULL,OF_CREATE);  
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
			CurTheme->Version = 103;
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
			if (hBT)
			{                             
				HPSTR	pRecs;
				HANDLE	hRecs;
				int		ClassNo, pos;  
				LPSHORT	lpint;
				long	NumRecs, lrec;
							
				NumRecs = BT_NUM_IN_INDEX (hBT);                    
				BigWrite(Fid,(HPSTR)&NumRecs,4,-1);
				if (NumRecs)
				{   
					lrec = GetBTKeyLen(hBT);
					hRecs = GlobalAlloc (GMEM_MOVEABLE,NumRecs*(2+lrec));
					pRecs = GlobalLock(hRecs);  
					pos = BT_FIRST;
					while (!BT_FIND(hBT,pRecs,pos,BT_ANY,(LPSTR)&ClassNo))
					{
						pos = BT_NEXT;
						pRecs += lrec;
						lpint = (LPSHORT) pRecs;
						*lpint++ = ClassNo;
						pRecs = (LPSTR) lpint;
									
					}  
					GlobalUnlock(hRecs);
					pRecs = GlobalLock(hRecs);
					BigWrite (Fid,pRecs,NumRecs*(2+lrec),-1);
					GSSiGlobUlFree (&hRecs); 
				}
				BT_CLOSE (hBT);
			}
			else
			{
				long NumRecs=0;
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

BOOL FAR PASCAL SV_THEME1MsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1247);
#endif
{	HWND	hCheckBox;
	RECT	rect;
	HDC		hDC;
	char	str[256],  *ptr;
LPSTR lpSTRING;
static  FIELDINFO FIELD;
static	LPFIELDINFO lpmess = &FIELD;
//lda addition
static	LPFIELDINFO lpFieldInfo = &FIELD;
HENV henv;
HDBC hdbc;
SWORD iptr, outlen, deslen;
UCHAR namel[256];
SDWORD namelen;
RETCODE rc;
int IRC, dlgitem,n;
LPSHORT irc = &IRC;  
static int i, NumFieldNames;
static LPVIEWPORT SaveView;   
LPTHEME	pSaveTheme;
static	HANDLE	hSaveTheme=0;
// end of lda addition
//FIELDINFO FAR *LPFIELDINFO ;
	long	icount;
	char index[] = "refno", keydata[] = "    -97492987" ;
	LPVOID LPIndex = &index, LPKeydata = &keydata;
	char ANSWER[64];
	char *answer = ANSWER;
	int	Choice;  
	BOOL	True=TRUE, Error;  
	int	IDC_FieldName=SV_FIELD_NAME; 
	COLORREF	Color;    
	long	ii;  
	UINT	nPrompts=42;
	UINT	PrmtDat[42*3] = {
				SV_TARGET,PRMT_SV_TARGET,0,
				SV_CB_ZEROBASED, PRMT_SV_CB_ZEROBASED,0,
				SV_CONTENTS_LIST, PRMT_SV_CONTENTS_LIST,MORE_SV_CONTENTS_LIST,
				SV_ITEM_TYPE, PRMT_SV_ITEM_TYPE,0,
				SV_DATABASE_LIST, PRMT_SV_DATABASE_LIST,0,
				SV_TABLE_NAMES, PRMT_SV_TABLE_NAMES,0,
				SV_FIELDFUNCTION, PRMT_SV_FIELDFUNCTION,MORE_SV_FIELDFUNCTION,
				SV_FIELD_NAME, PRMT_SV_FIELD_NAME,0,
				SV_CORRECTION, PRMT_SV_CORRECTION,0,
				SV_FIELD_VALUE, PRMT_SV_FIELD_VALUE,MORE_SV_FIELD_VALUE,
				IDC_SETSQL, PRMT_IDC_SETSQL, MORE_IDC_SETSQL,
				IDC_SQL, PRMT_IDC_SQL, MORE_IDC_SQL,
				SV_TITLE, PRMT_SV_TITLE, 0, 
				SV_NUM_CLASSES, PRMT_SV_NUM_CLASSES,MORE_SV_NUM_CLASSES,
				SV_MAXVAL, PRMT_SV_MAXVAL,0,
				IDC_ROUND_TO, PRMT_IDC_ROUND_TO,0,
				SV_CB_PERCENTILES, PRMT_SV_CB_PERCENTILES,0,
				SV_CB_EVENRANGES, PRMT_SV_CB_EVENRANGES, 0,
				SV_CB_MANUAL, PRMT_SV_CB_MANUAL,0,
				IDC_EDITRANGES, PRMT_IDC_EDITRANGES,0,
				IDC_MISSING_OPT0, PRMT_IDC_MISSING_OPT0,MORE_IDC_MISSING_OPT0,
				IDC_MISSING_OPT1, PRMT_IDC_MISSING_OPT1,0,
				IDC_MISSING_OPT2, PRMT_IDC_MISSING_OPT2,0,
				IDC_MISSING_OPT3, PRMT_IDC_MISSING_OPT3, MORE_IDC_MISSING_OPT3,
				SV_DISPLAY_DATAPOINTS, PRMT_SV_DISPLAY_DATAPOINTS, MORE_SV_DISPLAY_DATAPOINTS,
				SV_INSERT_COMMAS, PRMT_SV_INSERT_COMMAS, 0,
				SV_DISPLAY_VALUE, PRMT_SV_DISPLAY_VALUE, MORE_SV_DISPLAY_VALUE,
				SV_CB_ZEROASMISS, PRMT_SV_CB_ZEROASMISS, 0,
				IDC_MARK_INVALID, PRMT_IDC_MARK_INVALID, MORE_IDC_MARK_INVALID,
				SV_CB_RECOMPUTE, PRMT_SV_CB_RECOMPUTE, MORE_SV_CB_RECOMPUTE,
				SV_DISPLAY_PCT, PRMT_SV_DISPLAY_PCT, MORE_SV_DISPLAY_PCT,
				IDC_SAVE_THEME, PRMT_IDC_SAVE_THEME, MORE_IDC_SAVE_THEME,
				IDC_LOAD_THEME, PRMT_IDC_LOAD_THEME, MORE_IDC_LOAD_THEME,
				SV_COLOR_SCHEME,PRMT_SV_COLOR_SCHEME, MORE_SV_COLOR_SCHEME,
				IDC_DEFINE_COLOR_SCHEME, PRMT_IDC_DEFINE_COLOR_SCHEME, 0,
				IDC_BGCOLOR, PRMT_IDC_BGCOLOR, MORE_IDC_BGCOLOR,
				IDC_TITBGCOLOR, PRMT_IDC_TITBGCOLOR, MORE_IDC_TITBGCOLOR,
				IDC_VALBGCOLOR, PRMT_IDC_VALBGCOLOR, MORE_IDC_VALBGCOLOR,
				IDC_TITSIZE, PRMT_IDC_TITSIZE, MORE_IDC_TITSIZE,
				IDC_MARGIN, PRMT_IDC_MARGIN, MORE_IDC_MARGIN,
				SV_DISPERSE, PRMT_SV_DISPERSE,0,
				SV_PCTBYAREA, PRMT_SV_PCTBYAREA, 0
			};
  
 int	BRtn;
 if (Message==WM_INITDIALOG)
 {  
 	UINT	pw=0, pn=1, pm=2; 
 	HWND	hwnd;
 	short	ii;
 	
 	InitDlgPrompts (hWndDlg);
 	while (nPrompts--)
 	{   
 		hwnd = GetDlgItem(hWndDlg,PrmtDat[pw]);
 		if (!hwnd)
 			ii=1;
	 	SetDlgPrompt (hwnd,PrmtDat[pn],PrmtDat[pm]);
	 	pw+=3;
	 	pn+=3; 
	 	pm+=3;
	}
 }
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1247);
#endif
 	return (BRtn);
}
 if (CurTheme->ID != GF_GRAPHICS_FUNCTION_THEME)
 if (DATAFILEMsgProc(hWndDlg,Message,wParam,lParam,IDC_SQL,
 					 SV_SET_FILE, SV_DATABASE_LIST, SV_TABLE_NAMES,SV_TABLE_HEADING, &IDC_FieldName,1,
 					 CurTheme->DataFile, &CurTheme->DataFileType, &CurTheme->hThemeDB, &True,FALSE))
{
#if ENABLETRACE
GSSiExitProg (1247);
#endif
	return TRUE;
}
 if (ThemeCommonCode (hWndDlg, Message, wParam, lParam,CurTheme->hThemeDB))
{
#if ENABLETRACE
GSSiExitProg (1247);
#endif
 	return TRUE;
}
 switch(Message)
   { 
    case WM_INITDIALOG:  
    	ClearDlgPrompts (); 
    	DoPaint = FALSE; 
    	if (CurTheme->ID == GF_GRAPHICS_FUNCTION_THEME) 
    	{
        	SetDlgItemText (hWndDlg,SV_DATABASE_LIST,CurTheme->DataFile); 
	   	 	ShowWindow (GetDlgItem(hWndDlg,SV_TABLE_NAMES),SW_SHOW);
        	SetDlgItemText (hWndDlg,SV_TABLE_NAMES,CurTheme->IconLibrary);
        }
    	hSaveTheme = GSSiGlobAlloc ( 636,GHND,sizeof(THEME));
    	pSaveTheme = (LPTHEME)GlobalLock (hSaveTheme);
    	*pSaveTheme = *CurTheme;
    	GlobalUnlock (hSaveTheme);
		for (i=0;i<MAX_THEME_CLASSES;i++)
		{
			itoa (i+1,str,10);
			SendDlgItemMessage (hWndDlg,SV_COLUMNS,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)str)); 
		}
		SendDlgItemMessage (hWndDlg,SV_COLUMNS,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Circular")); 
		SendDlgItemMessage (hWndDlg,SV_COLUMNS,CB_SETCURSEL,CurTheme->NumCols,NULL); 
        if (CurTheme->FieldFun==6 && !*CurTheme->Value)
         	sprintf (CurTheme->Value,"[%s]",CurTheme->Field.name); 
    	SaveView = CurView;
	    NumFieldNames = 0;									       
		SendDlgItemMessage (hWndDlg,SV_FIELDFUNCTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Value of"));
		SendDlgItemMessage (hWndDlg,SV_FIELDFUNCTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Count of"));
		SendDlgItemMessage (hWndDlg,SV_FIELDFUNCTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Average of"));
		SendDlgItemMessage (hWndDlg,SV_FIELDFUNCTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Sum of")); 
		SendDlgItemMessage (hWndDlg,SV_FIELDFUNCTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Min of")); 
		SendDlgItemMessage (hWndDlg,SV_FIELDFUNCTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Max of")); 
		SendDlgItemMessage (hWndDlg,SV_FIELDFUNCTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Expression")); 
		SendDlgItemMessage (hWndDlg,SV_FIELDFUNCTION,CB_SETCURSEL,CurTheme->FieldFun,NULL); 
		
		SetFieldCorrectionOpts (hWndDlg,CurTheme->DataType); 
		SendDlgItemMessage (hWndDlg,SV_ITEM_TYPE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Areas"));
		SendDlgItemMessage (hWndDlg,SV_ITEM_TYPE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Points"));
		SendDlgItemMessage (hWndDlg,SV_ITEM_TYPE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Lines")); 
		SendDlgItemMessage (hWndDlg,SV_ITEM_TYPE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Text")); 
		SendDlgItemMessage (hWndDlg,SV_ITEM_TYPE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"All")); 
		SendDlgItemMessage (hWndDlg,SV_ITEM_TYPE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Pixels")); 
		SendDlgItemMessage (hWndDlg,SV_ITEM_TYPE,CB_SETCURSEL,CurTheme->DataType,NULL); 
		
		SendDlgItemMessage (hWndDlg,IDC_ROUND_TO,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"100,000"));
		SendDlgItemMessage (hWndDlg,IDC_ROUND_TO,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"10,000"));
		SendDlgItemMessage (hWndDlg,IDC_ROUND_TO,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"1,000"));
		SendDlgItemMessage (hWndDlg,IDC_ROUND_TO,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"100"));
		SendDlgItemMessage (hWndDlg,IDC_ROUND_TO,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"10"));
		SendDlgItemMessage (hWndDlg,IDC_ROUND_TO,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"1"));
		SendDlgItemMessage (hWndDlg,IDC_ROUND_TO,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"0.1"));
		SendDlgItemMessage (hWndDlg,IDC_ROUND_TO,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"0.01"));
		SendDlgItemMessage (hWndDlg,IDC_ROUND_TO,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"0.001")); 
		SendDlgItemMessage (hWndDlg,IDC_ROUND_TO,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"0.0001")); 
		SendDlgItemMessage (hWndDlg,IDC_ROUND_TO,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"0.00001")); 

    case GSSI_REINITDIALOG:

		 sprintf (str,"%f",CurTheme->RoundTo);
    	 SetDlgItemText(hWndDlg,IDC_ROUND_TO,str);
    	 SetDlgItemText(hWndDlg,SV_TITLE,(LPSTR)CurTheme->Title);
       	 SendDlgItemMessage (hWndDlg,SV_DISPLAY_DATAPOINTS,BM_SETCHECK,CurTheme->DisplayScatterDiagram,0L);
       	 SendDlgItemMessage (hWndDlg,SV_INSERT_COMMAS,BM_SETCHECK,CurTheme->AddCommas,0L);
       	 SendDlgItemMessage (hWndDlg,SV_CB_ZEROBASED,BM_SETCHECK,CurTheme->ZeroBased,0L);
       	 SendDlgItemMessage (hWndDlg,SV_CB_ZEROASMISS,BM_SETCHECK,CurTheme->ZeroIsMissing,0L);
       	 SendDlgItemMessage (hWndDlg,IDC_MARK_INVALID,BM_SETCHECK,CurTheme->MarkInvalid,0L);
       	 SendDlgItemMessage (hWndDlg,IDC_SKIP_INVALID,BM_SETCHECK,CurTheme->SkipInvalid,0L);
       	 SendDlgItemMessage (hWndDlg,SV_DISPLAY_VALUE,BM_SETCHECK,CurTheme->ShowValue,0L); 
       	 SendDlgItemMessage (hWndDlg,SV_DELAY_VALUE,BM_SETCHECK,CurTheme->DelayTextDisplay,0L); 
		 SendDlgItemMessage (hWndDlg,SV_DISPERSE,BM_SETCHECK,FALSE,0L);
 		 SendDlgItemMessage (hWndDlg,SV_ACCUMULATE,BM_SETCHECK,FALSE,0L);
       	 switch (CurTheme->DispersePoints)
       	 {
       	 	case 1:
       	 		SendDlgItemMessage (hWndDlg,SV_DISPERSE,BM_SETCHECK,TRUE,0L);
       	 		break;
       	 	case 2:
       	 		SendDlgItemMessage (hWndDlg,SV_ACCUMULATE,BM_SETCHECK,TRUE,0L);
       	 		break;
       	 }
	   	 ShowWindow (GetDlgItem(hWndDlg,IDC_EDITRANGES),SW_HIDE);
       	 if (CurTheme->ClassType ==1) dlgitem = SV_CB_EVENRANGES;
       	 if (CurTheme->ClassType ==2) dlgitem = SV_CB_PERCENTILES;
       	 if (CurTheme->ClassType ==3)
       	 {
       	 	dlgitem = SV_CB_MANUAL;
		   	ShowWindow (GetDlgItem(hWndDlg,IDC_EDITRANGES),SW_SHOW);
//10/17/03			CurTheme->Recompute=FALSE;
       	 }
       	 SendDlgItemMessage (hWndDlg,SV_CB_RECOMPUTE,BM_SETCHECK,CurTheme->Recompute,0L);
	     SendDlgItemMessage (hWndDlg,dlgitem,BM_SETCHECK,TRUE,0L);
       	 itoa (CurTheme->NumDesiredClass,str,10);
       	 SetDlgItemText (hWndDlg,SV_NUM_CLASSES,str);
       	 if (CurTheme->YLimit < DBL_MAX)
       	 	sprintf (str,"%f",CurTheme->YLimit);
       	 else
       	 	str[0]='\0';
       	 SetDlgItemText (hWndDlg,SV_MAXVAL,str);  
       	 for (i=0;i<5;i++)
       	 	 SendDlgItemMessage (hWndDlg,IDC_Missing_Opts[i],BM_SETCHECK,FALSE,0L);

   		 SendDlgItemMessage (hWndDlg,IDC_Missing_Opts[CurTheme->MissOpt],BM_SETCHECK,TRUE,0L);

         SetDlgItemText(hWndDlg,SV_FIELD_NAME,(LPCSTR)&CurTheme->Field.name); 
         SetDlgItemText(hWndDlg,IDC_SQL,(LPCSTR)CurTheme->SQL);
         
         SetDlgItemInt (hWndDlg,IDC_TITSIZE,CurTheme->TitleHeight,TRUE);
         SetDlgItemInt (hWndDlg,IDC_INMARGIN,CurTheme->InnerMargin,TRUE);
         SetDlgItemInt (hWndDlg,IDC_MARGIN,CurTheme->Margin,TRUE);
         SetDlgItemInt (hWndDlg,IDC_BOXSIZE,CurTheme->ColorsWidth,TRUE);
         if (CurTheme->FieldFun<6)  
		 {
			ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_VALUE),SW_HIDE);
			ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_NAME),SW_SHOW); 
			if (CurTheme->DataFileType != GMCENSUS_DATAFILE)
				ShowWindow (GetDlgItem(hWndDlg,IDC_SHOW_FIELDS),SW_HIDE); 
		 }
		 else
		 {
			ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_VALUE),SW_SHOW);
			ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_NAME),SW_HIDE);
			ShowWindow (GetDlgItem(hWndDlg,IDC_SHOW_FIELDS),SW_SHOW); 
		 }
         SetDlgItemText (hWndDlg,SV_FIELD_VALUE,CurTheme->Value);
         if (ThemeEditStartCmd)
	         PostMessage(hWndDlg, WM_COMMAND, ThemeEditStartCmd, 0L);

		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           {
            case IDC_THEME_HELP:
              //  WinHelp(hWndDlg,"gwizhelp\\gwizhelp.hlp",HELP_CONTEXT,IDD_SV_THEME);
              //  WinHelp(hWndDlg,"gwizhelp\\gwizhelp.hlp",HELP_CONTEXTPOPUP,IDD_SV_THEME);
              WinHelp(hWndDlg,"gwizhelp\\gwizhelp.hlp",HELP_PARTIALKEY,(DWORD)"Theme Editing");
              //  WinHelp(hWndDlg,"gwizhelp\\gwizhelp.hlp",HELP_KEY,(DWORD)"Theme Editing");
              //  WinHelp(hWndDlg,"gwizhelp\\gwizhelp.hlp",HELP_MULTIKEY,(DWORD)"Theme Editing");
               break;
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 CloseThemeDataFile(TRUE);   
		    	 pSaveTheme = (LPTHEME)GlobalLock (hSaveTheme);
		    	 *CurTheme = *pSaveTheme;  
		    	 CurTheme->hThemeDB = 0;
		    	 GSSiGlobUlFree (&hSaveTheme); 
			     SetCurView (SaveView);
                 DestroyFieldList ();
    			 DoPaint = TRUE;
                 EndDialog(hWndDlg, FALSE);
                 break;  
                 
            case SV_FIELDFUNCTION:
              switch(HIWORD(lParam))
              {
               case CBN_DBLCLK:
               case CBN_SELCHANGE:
                  Choice=SendDlgItemMessage(hWndDlg,SV_FIELDFUNCTION,
									       CB_GETCURSEL,NULL,NULL);
				  if (Choice<6)
		          {
		         	ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_VALUE),SW_HIDE);
		         	ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_NAME),SW_SHOW); 
		         	ShowWindow (GetDlgItem(hWndDlg,IDC_SHOW_FIELDS),SW_HIDE); 
		          }
		          else
		          {
		         	ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_VALUE),SW_SHOW);
		         	ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_NAME),SW_HIDE);
		         	ShowWindow (GetDlgItem(hWndDlg,IDC_SHOW_FIELDS),SW_SHOW); 
		          }
				  break;
			  }
			  break;
			   
            case IDC_SHOW_FIELDS: 
            	 
            	 CensusValueField = SV_FIELD_VALUE;
            	 CensusValueWnd = hWndDlg;
            	 CensusTitleField = SV_TITLE;
            	 DisplayFieldList (hWndDlg,CurTheme->hThemeDB,NULL,0);
            	 CensusValueField = 0;
            	 CensusTitleField = 0;     
            	 if (ThemeEditStartCmd)
			         PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
            	 	
                 break;
                      
            case SV_ITEM_TYPE:
              switch(HIWORD(lParam))
              {
               case CBN_DBLCLK:
               case CBN_SELCHANGE:
                  Choice=SendDlgItemMessage(hWndDlg,SV_ITEM_TYPE,
									       CB_GETCURSEL,NULL,NULL);
				  CurTheme->FieldCorrection=0;
				  SetFieldCorrectionOpts (hWndDlg,Choice); 
				  break;
			  }
			  break; 
			  
			case IDC_TITTEXTFONT:
				break;
			 
            case SV_FIELD_NAME:
              switch(HIWORD(lParam))
              {
               case CBN_DBLCLK:
               case CBN_SELCHANGE:
            
                  Choice=SendDlgItemMessage(hWndDlg,SV_FIELD_NAME,
									       CB_GETCURSEL,NULL,NULL); 
				  if(Choice >= 0)
			      {					        
		               SendDlgItemMessage(hWndDlg,SV_FIELD_NAME,CB_GETLBTEXT,
		         		  		    Choice,(DWORD)&CurTheme->Field.name); 
		           }
		           break;  
		      }// end of the switch       		  		    
              break;
            
            case IDC_EDITRANGES:
		       	 GetDlgItemText (hWndDlg,SV_NUM_CLASSES,str,3);
		       	 CurTheme->NumDesiredClass = atoi (str);
            	 GetDlgItemText (hWndDlg,IDC_ROUND_TO,str,10); 
            	 Strip (str,',');
            	 CurTheme->RoundTo = atof(str);
		         {
			          FARPROC lpfnCLASS_RANGESMsgProc;
			          lpfnCLASS_RANGESMsgProc = MakeProcInstance((FARPROC)CLASS_RANGESMsgProc, hInst);
			          DialogBox(hInst, (LPSTR)"CLASS_RANGES", hWndDlg, lpfnCLASS_RANGESMsgProc);
			          FreeProcInstance(lpfnCLASS_RANGESMsgProc);
		         }
              	break;
              
            case SV_CB_EVENRANGES:
       	 	case SV_CB_PERCENTILES:
			   	ShowWindow (GetDlgItem(hWndDlg,IDC_EDITRANGES),SW_HIDE);
				CurTheme->Recompute=TRUE;
			   	break;
			   	
			case SV_CB_MANUAL:
			   	ShowWindow (GetDlgItem(hWndDlg,IDC_EDITRANGES),SW_SHOW);
				CurTheme->Recompute=FALSE;
       	 		SendDlgItemMessage (hWndDlg,SV_CB_RECOMPUTE,BM_SETCHECK,FALSE,0L);
			  	break;
            
            case SV_DISPERSE:
				SendDlgItemMessage (hWndDlg,SV_ACCUMULATE,BM_SETCHECK,FALSE,0L);
                break;
                
            case SV_ACCUMULATE:
				SendDlgItemMessage (hWndDlg,SV_DISPERSE,BM_SETCHECK,FALSE,0L);
            	if (SendDlgItemMessage (hWndDlg,SV_ACCUMULATE,BM_GETCHECK,0,0L))
		        {
			    	FARPROC lpfnACCUMPOINTOPTMsgProc; 
				    	
					DoPaint = FALSE; 
			        lpfnACCUMPOINTOPTMsgProc = MakeProcInstance((FARPROC)ACCUMPOINTOPTMsgProc, hInst);
			        DialogBox(hInst, (LPSTR)"ACCUMPOINTOPT", hWndDlg, lpfnACCUMPOINTOPTMsgProc);
			        FreeProcInstance(lpfnACCUMPOINTOPTMsgProc);
					DoPaint=TRUE;
				}
                break;
                
            case IDC_OPEN_DB:  
            	 if (CurTheme->DataFileType == 8)
            	 {
	                 ShowWindow (GetDlgItem(hWndDlg,IDC_SQL),SW_HIDE);  
	                 ShowWindow (GetDlgItem(hWndDlg,IDC_SETSQL),SW_HIDE);  
	                 ShowWindow (GetDlgItem(hWndDlg,IDC_SEGMENTTITLE),SW_SHOW);  
	                 ShowWindow (GetDlgItem(hWndDlg,IDC_SEGMENT),SW_SHOW);
	                 ltoa ((long)CurTheme->DynSegMaxFixedIncrement,str,10);  
            	 	 SetDlgItemText (hWndDlg,IDC_SEGMENT,str);
            	 }
            	 else
            	 {   
            	 	 SetDlgItemText (hWndDlg,IDC_SEGMENT,"");
	                 ShowWindow (GetDlgItem(hWndDlg,IDC_SQL),SW_SHOW);  
	                 ShowWindow (GetDlgItem(hWndDlg,IDC_SETSQL),SW_SHOW);  
	                 EnableWindow (GetDlgItem(hWndDlg,IDC_SETSQL),TRUE);
	                 EnableWindow (GetDlgItem(hWndDlg,IDC_SHOW_FIELDS),TRUE);
	             }
                 break;  
            
            case IDC_SAVE_THEME:  
 
            case IDOK: 
            	 if (CurTheme->DataFileType == UMIFS_DATAFILE || CurTheme->DataFileType == GMCENSUS_DATAFILE)
            	 	GetDlgItemText (hWndDlg,SV_DATABASE_LIST,CurTheme->DataFile,sizeof(CurTheme->DataFile));
                 CurTheme->DataType=SendDlgItemMessage(hWndDlg,SV_ITEM_TYPE,
									       CB_GETCURSEL,NULL,NULL);
		    	 if (CurTheme->ID == GF_GRAPHICS_FUNCTION_THEME) 
		        	GetDlgItemText (hWndDlg,SV_TABLE_NAMES,CurTheme->IconLibrary,128);
            	 GetDlgItemText (hWndDlg,SV_FIELD_VALUE,CurTheme->Value,sizeof(CurTheme->Value));
                 CurTheme->FieldFun=SendDlgItemMessage(hWndDlg,SV_FIELDFUNCTION,
									       CB_GETCURSEL,NULL,NULL);
                 CurTheme->FieldCorrection=SendDlgItemMessage(hWndDlg,SV_CORRECTION,
									       CB_GETCURSEL,NULL,NULL);
            	 GetDlgItemText (hWndDlg,IDC_SEGMENT,str,10); 
				 CurTheme->DynSegMaxFixedIncrement = atol (str);
            	 GetDlgItemText (hWndDlg,IDC_ROUND_TO,str,10); 
            	 Strip (str,',');
            	 CurTheme->RoundTo = atof(str);
  				 CurTheme->Field.type = SQL_VARCHAR;
                 Choice=SendDlgItemMessage(hWndDlg,SV_FIELD_NAME,
									       CB_GETCURSEL,NULL,NULL);
		         SendDlgItemMessage(hWndDlg,SV_FIELD_NAME,CB_GETLBTEXT,
		         		  		    Choice,(DWORD)&CurTheme->Field.name);
                 
                 CurTheme->NumCols=SendDlgItemMessage(hWndDlg,SV_COLUMNS,CB_GETCURSEL,NULL,NULL);
         		 CurTheme->TitleHeight=GetDlgItemInt (hWndDlg,IDC_TITSIZE,&Error,TRUE);
         		 CurTheme->InnerMargin=GetDlgItemInt (hWndDlg,IDC_INMARGIN,&Error,TRUE);
         		 CurTheme->Margin=GetDlgItemInt (hWndDlg,IDC_MARGIN,&Error,TRUE);
         		 CurTheme->ColorsWidth=GetDlgItemInt (hWndDlg,IDC_BOXSIZE,&Error,TRUE);
                 CurTheme->ColorScheme = SendDlgItemMessage(hWndDlg,SV_COLOR_SCHEME,LB_GETCURSEL,NULL,NULL);
                 SetThemeColorsFromScheme ();
       	 		 GetDlgItemText (hWndDlg,SV_TITLE,CurTheme->Title,256);
       	 		 GetDlgItemText (hWndDlg,IDC_SQL,CurTheme->SQL,256);
            	 CurTheme->DisplayScatterDiagram = SendDlgItemMessage (hWndDlg,SV_DISPLAY_DATAPOINTS,BM_GETCHECK,0,0L);  
            	 for (i=0;i<5;i++)
            	 	if (SendDlgItemMessage (hWndDlg,IDC_Missing_Opts[i],BM_GETCHECK,0,0L))
            	 		CurTheme->MissOpt = i;
            	 CurTheme->AddCommas = SendDlgItemMessage (hWndDlg,SV_INSERT_COMMAS,BM_GETCHECK,0,0L); 
            	 CurTheme->ZeroBased = SendDlgItemMessage (hWndDlg,SV_CB_ZEROBASED,BM_GETCHECK,0,0L); 
            	 CurTheme->ShowValue = SendDlgItemMessage (hWndDlg,SV_DISPLAY_VALUE,BM_GETCHECK,0,0L); 
            	 CurTheme->DispersePoints = 0;
            	 if (SendDlgItemMessage (hWndDlg,SV_DISPERSE,BM_GETCHECK,0,0L))
            	 	CurTheme->DispersePoints = 1;
            	 if (SendDlgItemMessage (hWndDlg,SV_ACCUMULATE,BM_GETCHECK,0,0L))
            	 	CurTheme->DispersePoints = 2;
            	 if ((CurTheme->Recompute = SendDlgItemMessage (hWndDlg,SV_CB_RECOMPUTE,BM_GETCHECK,0,0L)))
                 {   
                	CurTheme->WantDataPass = TRUE;
                    if (CurTheme->ClassType != 3)
                    	CurTheme->ComputeClassBoundaries = TRUE;                    
                 }
             	 CurTheme->ZeroIsMissing = SendDlgItemMessage (hWndDlg,SV_CB_ZEROASMISS,BM_GETCHECK,0,0L); 
            	 CurTheme->MarkInvalid = SendDlgItemMessage (hWndDlg,IDC_MARK_INVALID,BM_GETCHECK,0,0L); 
            	 CurTheme->SkipInvalid = SendDlgItemMessage (hWndDlg,IDC_SKIP_INVALID,BM_GETCHECK,0,0L); 

		       	 if (SendDlgItemMessage (hWndDlg,SV_CB_EVENRANGES,BM_GETCHECK,0,0L)) CurTheme->ClassType =1;
		       	 if (SendDlgItemMessage (hWndDlg,SV_CB_PERCENTILES,BM_GETCHECK,0,0L)) CurTheme->ClassType =2;
		       	 if (SendDlgItemMessage (hWndDlg,SV_CB_MANUAL,BM_GETCHECK,0,0L)) CurTheme->ClassType =3;
		       	 if (CurTheme->ClassType!=1) CurTheme->DisplayScatterDiagram = FALSE;
		       	 GetDlgItemText (hWndDlg,SV_NUM_CLASSES,str,3);
		       	 CurTheme->NumDesiredClass = atoi (str);
		       	 if (CurTheme->ClassType == 3) 
		       	 	CurTheme->NumClass = CurTheme->NumDesiredClass;
		       	 GetDlgItemText (hWndDlg,SV_MAXVAL,str,16);
		       	 Truncate (str);
		       	 if (str[0])
		       	 	CurTheme->YLimit = atof (str); 
		       	 else
		       	 	CurTheme->YLimit = DBL_MAX;
		       	 
		       	 if (wParam == IDC_SAVE_THEME)
		       	   	SaveCurTheme(hWndDlg);
		       	 else
		       	 {
			       	 SetCurView (SaveView);
	                 CloseThemeDataFile(TRUE);
		    	 	 GSSiGlobFree (&hSaveTheme);
				     DoPaint = FALSE;
	                 EndDialog(hWndDlg, TRUE); 
	             }
                 break;

          /*  case SV_DATA_FILE_ICON:
            	 CloseThemeDataFile(TRUE);
            	 SetFilterString (IDS_FILTERDB);

    no longer   if (GetOpenFileCD (hWndDlg,CurTheme->DataFile,"C:"))
    use this    {
           	    	 SetDlgItemText(hWndDlg,SV_DATA_FILE,CurTheme->DataFile);
           	    	 if (_fstrstr (CurTheme->DataFile,".GWD")) CurTheme->DataFileType = UMIFS_DATAFILE;
           	    	 if (_fstrstr (CurTheme->DataFile,".DBF")) CurTheme->DataFileType = FOXPRO_DATAFILE;
           	    	 if (_fstrstr (CurTheme->DataFile,".MDB")) CurTheme->DataFileType = MSACCESS_DATAFILE;
           	    	 goto LoadFields;
                 }  */


           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1247);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1247);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

void SetFieldCorrectionOpts (HWND hWndDlg,short Type)
#if ENABLETRACE
{GSSiEnterProg (1248);
#endif
{
	SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_RESETCONTENT,NULL,NULL);
	SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Unadjusted")); 
	switch (Type)
	{
		case 0:
		SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Per Square Foot")); 
		SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Per Square Meter")); 
		SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Per Square Yard")); 
		SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Per Square Mile")); 
		SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Per Square Kilometer")); 
		SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Per Acre")); 
		break; 
		case 1:
		break; 
		case 2:
		SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Per Foot")); 
		SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Per Meter")); 
		SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Per Yard")); 
		SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Per Mile")); 
		SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Per Kilometer")); 
		break; 
	}
	SendDlgItemMessage (hWndDlg,SV_CORRECTION,CB_SETCURSEL,CurTheme->FieldCorrection,NULL);
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

BOOL FAR PASCAL ACCUMPOINTOPTMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1249);
#endif
{	

 int	SymControls[3]={IDC_ASCIRCLE,IDC_ASSQUARE,IDC_ASNOCHANGE};
 int	SizeControls[5]={IDC_APSIZE1,IDC_APSIZE2,IDC_APSIZE3,IDC_APSIZE4,IDC_APSIZE5};  
 int	BaseSizeControls[3]={IDC_BSORIGNAL,IDC_BSPIXELS,IDC_BSWORLD};
 int	BRtn; 
 char	str[128]; 
 short	i;  
 static	HANDLE	hSaveTheme; 
 LPTHEME	pSaveTheme;
 
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1249);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:
         cwCenter(hWndDlg, 0); 
         hSaveTheme = GSSiGlobAlloc ( 637,GMEM_MOVEABLE,sizeof(THEME));
         pSaveTheme = (LPTHEME)GlobalLock (hSaveTheme);
         *pSaveTheme = *CurTheme;
         GlobalUnlock (hSaveTheme);
		 SendDlgItemMessage (hWndDlg,SymControls[CurTheme->AccumPointSymbolOpt],BM_SETCHECK,TRUE,0L);
		 SendDlgItemMessage (hWndDlg,SizeControls[CurTheme->AccumPointSizeOpt],BM_SETCHECK,TRUE,0L);
		 if (CurTheme->AccumPointBaseSize < 0)
		 {
		 	i=2;  
		 	sprintf (str,"%f",-CurTheme->AccumPointBaseSize);
    	 	SetDlgItemText (hWndDlg,IDC_BSWORLDSIZE,str);
    	 } 
		 else if (CurTheme->AccumPointBaseSize > 0) 
		 {
		 	i=1; 
		 	ltoa (IDNINT(CurTheme->AccumPointBaseSize),str,10);
    	 	SetDlgItemText (hWndDlg,IDC_BSPIXSIZE,str);
    	 } 
    	 else
    	 	i=0;
		 SendDlgItemMessage (hWndDlg,BaseSizeControls[i],BM_SETCHECK,TRUE,0L);
		 SendDlgItemMessage (hWndDlg,IDC_APLINK,BM_SETCHECK,CurTheme->AccumPointLink,0L);
		 SendDlgItemMessage (hWndDlg,IDC_ACCUMTEXT,BM_SETCHECK,CurTheme->AccumPointText,0L);
 		 SendDlgItemMessage (hWndDlg,IDC_DISPERSNONE,BM_SETCHECK,CurTheme->ACCDisperse==2,0L);
 		 SendDlgItemMessage (hWndDlg,IDC_DISPERSPARTIAL,BM_SETCHECK,CurTheme->ACCDisperse==1,0L);
 		 SendDlgItemMessage (hWndDlg,IDC_DISPERSEFULL,BM_SETCHECK,CurTheme->ACCDisperse==0,0L);
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           {
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
		         pSaveTheme =(LPTHEME)GlobalLock (hSaveTheme);
		         *CurTheme = *pSaveTheme;
		         GlobalUnlock (hSaveTheme); 
		         GlobalFree (hSaveTheme);
                 EndDialog(hWndDlg, FALSE);
                 break; 
            
            case IDC_BSORIGNAL:
            case IDC_BSPIXELS:
            case IDC_BSWORLD:
            	SetDlgItemText (hWndDlg,IDC_BSPIXSIZE,"");
    	 		SetDlgItemText (hWndDlg,IDC_BSWORLDSIZE,"");
    	 		break;
    	 		
            case IDOK:
            {   
            	for (i=0;i<3;i++)
            		if (SendDlgItemMessage (hWndDlg,SymControls[i],BM_GETCHECK,0,0L))
            			CurTheme->AccumPointSymbolOpt = i;
            	for (i=0;i<5;i++)
            		if (SendDlgItemMessage (hWndDlg,SizeControls[i],BM_GETCHECK,0,0L))
            			CurTheme->AccumPointSizeOpt = i;
           		if (SendDlgItemMessage (hWndDlg,IDC_BSPIXELS,BM_GETCHECK,0,0L))
           		{
           			GetDlgItemText (hWndDlg,IDC_BSPIXSIZE,str,sizeof(str));
           			CurTheme->AccumPointBaseSize = atol (str);
           		}
           		else if (SendDlgItemMessage (hWndDlg,IDC_BSWORLD,BM_GETCHECK,0,0L))
           		{
           			GetDlgItemText (hWndDlg,IDC_BSWORLDSIZE,str,sizeof(str));
           			CurTheme->AccumPointBaseSize = -atof (str);
           		} 
           		else
           			CurTheme->AccumPointBaseSize = 0;
           		
		 		CurTheme->AccumPointLink = SendDlgItemMessage (hWndDlg,IDC_APLINK,BM_GETCHECK,0,0L);
		 		CurTheme->AccumPointText = SendDlgItemMessage (hWndDlg,IDC_ACCUMTEXT,BM_GETCHECK,0,0L);
		 		if (SendDlgItemMessage (hWndDlg,IDC_DISPERSNONE,BM_GETCHECK,0,0L))
		 			CurTheme->ACCDisperse = 2;
		 		else if (SendDlgItemMessage (hWndDlg,IDC_DISPERSPARTIAL,BM_GETCHECK,0,0L))
		 			CurTheme->ACCDisperse = 1;
		 		else
		 			CurTheme->ACCDisperse = 0;
                GlobalFree (hSaveTheme);
               	EndDialog(hWndDlg, TRUE);
            }
                break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1249);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1249);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL FAR PASCAL SV_THEME2MsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1250);
#endif
{	HWND	hCheckBox;
	RECT	rect;
	HDC		hDC;
	char	str[256],  *ptr;
LPSTR lpSTRING;
static  FIELDINFO FIELD;
static	LPFIELDINFO lpmess = &FIELD;
//lda addition
static	LPFIELDINFO lpFieldInfo = &FIELD;
HENV henv;
HDBC hdbc;
SWORD iptr, outlen, deslen;
UCHAR namel[256];
SDWORD namelen;
RETCODE rc;
int IRC, dlgitem;   
BOOL	Error;
LPSHORT irc = &IRC;  
static int i, NumFieldNames;
static LPVIEWPORT SaveView;
// end of lda addition
//FIELDINFO FAR *LPFIELDINFO ;
	long	icount;
	char index[] = "refno", keydata[] = "    -97492987" ;
	LPVOID LPIndex = &index, LPKeydata = &keydata;
	char ANSWER[64];
	char *answer = ANSWER;
	int	Choice;      
	BOOL	True=TRUE;  
	int	IDC_FieldName = SV_FIELD_NAME;
	static		HANDLE	hSaveBM=0;
	

  
 int	BRtn; 
 if (Message == WM_INITDIALOG)
 {
 	    SaveView = CurView;
        hSaveBM = EnterBlockingWindow (hWndDlg);
 }
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1250);
#endif
 	return (BRtn);
}
 if (DATAFILEMsgProc(hWndDlg,Message,wParam,lParam,IDC_SQL,
 					 SV_SET_FILE, SV_DATABASE_LIST, SV_TABLE_NAMES,SV_TABLE_HEADING, &IDC_FieldName,1,
 					 CurTheme->DataFile, &CurTheme->DataFileType, &CurTheme->hThemeDB, &True,FALSE))
{
#if ENABLETRACE
GSSiExitProg (1250);
#endif
 					 	return TRUE;
}
 if (ThemeCommonCode (hWndDlg, Message, wParam, lParam,CurTheme->hThemeDB))
{
#if ENABLETRACE
GSSiExitProg (1250);
#endif
 	return TRUE;
}
 switch(Message)
   {
    case WM_INITDIALOG:  
    case GSSI_REINITDIALOG:
        SendDlgItemMessage (hWndDlg,SV_FIELDFUNCTION,CB_RESETCONTENT,NULL,NULL);
		SendDlgItemMessage (hWndDlg,SV_FIELDFUNCTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Value of"));
		SendDlgItemMessage (hWndDlg,SV_FIELDFUNCTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Expression")); 
		SendDlgItemMessage (hWndDlg,SV_FIELDFUNCTION,CB_SETCURSEL,CurTheme->FieldFun,NULL); 
        if (CurTheme->FieldFun<1)  
        {
         	ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_VALUE),SW_HIDE);
         	ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_NAME),SW_SHOW);
        }
        else
        {
         	ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_VALUE),SW_SHOW);
         	ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_NAME),SW_HIDE);
        }
        SetDlgItemText (hWndDlg,SV_FIELD_VALUE,CurTheme->Value);
		
        SendDlgItemMessage (hWndDlg,SV_ITEM_TYPE,CB_RESETCONTENT,NULL,NULL);
		SendDlgItemMessage (hWndDlg,SV_ITEM_TYPE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Areas"));
		SendDlgItemMessage (hWndDlg,SV_ITEM_TYPE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Points"));
		SendDlgItemMessage (hWndDlg,SV_ITEM_TYPE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Lines")); 
		SendDlgItemMessage (hWndDlg,SV_ITEM_TYPE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Text")); 
		SendDlgItemMessage (hWndDlg,SV_ITEM_TYPE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"All")); 
		SendDlgItemMessage (hWndDlg,SV_ITEM_TYPE,CB_SETCURSEL,CurTheme->DataType,NULL); 
		
		for (i=0;i<MAX_THEME_CLASSES;i++)
		{
			itoa (i+1,str,10);
			SendDlgItemMessage (hWndDlg,SV_COLUMNS,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)str)); 
		}
		SendDlgItemMessage (hWndDlg,SV_COLUMNS,CB_SETCURSEL,CurTheme->NumCols,NULL); 
		
       	 SendDlgItemMessage (hWndDlg,IDC_MARK_INVALID,BM_SETCHECK,CurTheme->MarkInvalid,0L);
       	 SendDlgItemMessage (hWndDlg,IDC_SKIP_INVALID,BM_SETCHECK,CurTheme->SkipInvalid,0L);
		 SendDlgItemMessage (hWndDlg,SV_DISPERSE,BM_SETCHECK,FALSE,0L);
 		 SendDlgItemMessage (hWndDlg,SV_ACCUMULATE,BM_SETCHECK,FALSE,0L);
       	 switch (CurTheme->DispersePoints)
       	 {
       	 	case 1:
       	 		SendDlgItemMessage (hWndDlg,SV_DISPERSE,BM_SETCHECK,TRUE,0L);
       	 		break;
       	 	case 2:
       	 		SendDlgItemMessage (hWndDlg,SV_ACCUMULATE,BM_SETCHECK,TRUE,0L);
       	 		break;
       	 }
    	 SetDlgItemText(hWndDlg,SV_TITLE,(LPSTR)&CurTheme->Title);
       	 SendDlgItemMessage (hWndDlg,IDC_AUTOCLASSDEF,BM_SETCHECK,CurTheme->AutoClassDef,0L);
       	 SendDlgItemMessage (hWndDlg,IDC_USEFIRSTSYMBOL,BM_SETCHECK,CurTheme->UseFirstSymbol,0L);
       	 SendDlgItemMessage (hWndDlg,SV_CB_RECOMPUTE,BM_SETCHECK,CurTheme->ComputeStoredCounts,0L);
       	 SendDlgItemMessage (hWndDlg,SV_HIDENOCOUNT,BM_SETCHECK,CurTheme->ClearIfNoCount,0L);
       	 SendDlgItemMessage (hWndDlg,SV_DISPLAY_VALUE,BM_SETCHECK,CurTheme->ShowValue,0L);
       	 SendDlgItemMessage (hWndDlg,SV_DELAY_VALUE,BM_SETCHECK,CurTheme->DelayTextDisplay,0L); 
       	 SendDlgItemMessage (hWndDlg,SV_DISPLAY_COUNT,BM_SETCHECK,CurTheme->DisplayCount,0L);
       	 itoa (CurTheme->NumDesiredClass,str,10);
       	 SetDlgItemText (hWndDlg,SV_NUM_CLASSES,str);
       	 if (CurTheme->YLimit < DBL_MAX)
       	 	sprintf (str,"%f",CurTheme->YLimit);
       	 else
       	 	str[0]='\0';
       	 for (i=0;i<5;i++)
       	 	 SendDlgItemMessage (hWndDlg,IDC_Missing_Opts[i],BM_SETCHECK,FALSE,0L);

   		 SendDlgItemMessage (hWndDlg,IDC_Missing_Opts[CurTheme->MissOpt],BM_SETCHECK,TRUE,0L);
         
         SetDlgItemText(hWndDlg,SV_FIELD_NAME,(LPCSTR)&CurTheme->Field.name); 
         SetDlgItemText(hWndDlg,IDC_SQL,(LPCSTR)CurTheme->SQL);
         SetDlgItemInt (hWndDlg,IDC_TITSIZE,CurTheme->TitleHeight,TRUE);
         SetDlgItemInt (hWndDlg,IDC_INMARGIN,CurTheme->InnerMargin,TRUE);
         SetDlgItemInt (hWndDlg,IDC_MARGIN,CurTheme->Margin,TRUE);
         SetDlgItemInt (hWndDlg,IDC_BOXSIZE,CurTheme->ColorsWidth,TRUE);
		 EnableWindow (GetDlgItem(hWndDlg,IDC_CLASSES_FROM_TABLE),!CurTheme->AutoClassDef); 
		 EnableWindow (GetDlgItem(hWndDlg,IDC_DEFINE_CLASSES),!CurTheme->AutoClassDef);  
         SendDlgItemMessage (hWndDlg,IDC_MULTIVALUEOPT,CB_RESETCONTENT,NULL,NULL);
		 SendDlgItemMessage (hWndDlg,IDC_MULTIVALUEOPT,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Use First Value")); 
		 SendDlgItemMessage (hWndDlg,IDC_MULTIVALUEOPT,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Use Lowest Class")); 
		 SendDlgItemMessage (hWndDlg,IDC_MULTIVALUEOPT,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Use Highest Class")); 
		 SendDlgItemMessage (hWndDlg,IDC_MULTIVALUEOPT,CB_SETCURSEL,CurTheme->GetFirstClass,NULL); 
         if (ThemeEditStartCmd)
	         PostMessage(hWndDlg, WM_COMMAND, ThemeEditStartCmd, 0L);
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           {
            case IDC_THEME_HELP:
              //  WinHelp(hWndDlg,"gwizhelp\\gwizhelp.hlp",HELP_CONTEXT,IDD_SV_THEME);
              //  WinHelp(hWndDlg,"gwizhelp\\gwizhelp.hlp",HELP_CONTEXTPOPUP,IDD_SV_THEME);
              WinHelp(hWndDlg,"gwizhelp\\gwizhelp.hlp",HELP_PARTIALKEY,(DWORD)"Theme Editing");
              //  WinHelp(hWndDlg,"gwizhelp\\gwizhelp.hlp",HELP_KEY,(DWORD)"Theme Editing");
              //  WinHelp(hWndDlg,"gwizhelp\\gwizhelp.hlp",HELP_MULTIKEY,(DWORD)"Theme Editing");
               break;  
            
            case IDC_SHOW_FIELDS:
            	 DisplayFieldList (hWndDlg,CurTheme->hThemeDB,NULL,0);
                 break;
                      
            case IDC_OPEN_DB:
                 EnableWindow (GetDlgItem(hWndDlg,IDC_SETSQL),TRUE);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_SHOW_FIELDS),TRUE);
                 break;  
            
            	break;   
            case SV_FIELDFUNCTION:
              switch(HIWORD(lParam))
              {
               case CBN_DBLCLK:
               case CBN_SELCHANGE:
                  Choice=SendDlgItemMessage(hWndDlg,SV_FIELDFUNCTION,
									       CB_GETCURSEL,NULL,NULL);
				  if (Choice<1)
		          {
		         	ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_VALUE),SW_HIDE);
		         	ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_NAME),SW_SHOW);
		          }
		          else
		          {
		         	ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_VALUE),SW_SHOW);
		         	ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_NAME),SW_HIDE);
		          }
				  break;
			  }
			  break;
			
			case IDC_AUTOCLASSDEF:  
			{
				BOOL On = SendDlgItemMessage (hWndDlg,IDC_AUTOCLASSDEF,BM_GETCHECK,0,0L);
        		EnableWindow (GetDlgItem(hWndDlg,IDC_CLASSES_FROM_TABLE),!On); 
        		EnableWindow (GetDlgItem(hWndDlg,IDC_DEFINE_CLASSES),!On);  
        	}
			break;
			
			case IDC_CLASSES_FROM_TABLE:
			{
				BOOL	st;
		        FARPROC lpfnCLASSESFROMTABLEMsgProc;
	            lpfnCLASSESFROMTABLEMsgProc = MakeProcInstance((FARPROC)CLASSESFROMTABLEMsgProc, hInst);
				st = DialogBox(hInst, (LPSTR)"CLASSESFROMTABLE", hWndDlg, lpfnCLASSESFROMTABLEMsgProc);
				FreeProcInstance(lpfnCLASSESFROMTABLEMsgProc); 
				if (st)
					SetDlgItemInt (hWndDlg,SV_NUM_CLASSES,CurTheme->NumClass,TRUE);
			}
			break;
			     
            case IDC_DEFINE_CLASSES:
		       	 GetDlgItemText (hWndDlg,SV_NUM_CLASSES,str,3); 
		       	 GetDlgItemText (hWndDlg,SV_FIELD_NAME,CurTheme->Field.name,62); 
		       	 GetDBFieldInfo (&CurTheme->Field,CurTheme->hThemeDB);
		       	 CurTheme->NumDesiredClass = atoi (str);
		         {
			          FARPROC lpfnASSIGN_NONNUMERICMsgProc;
			          lpfnASSIGN_NONNUMERICMsgProc = MakeProcInstance((FARPROC)ASSIGN_NONNUMERICMsgProc, hInst);
			          DialogBox(hInst, (LPSTR)"ASSIGN_NONNUMERIC", hWndDlg, lpfnASSIGN_NONNUMERICMsgProc);
			          FreeProcInstance(lpfnASSIGN_NONNUMERICMsgProc);
		         }
		         i = CurTheme->NumDesiredClass;  
		         while (i--)
		         {
		         	if (CurTheme->ClassBM[i][0] || CurTheme->ClassCount[i])  
		         	{
		         		itoa (CurTheme->NumDesiredClass,str,10);
		         		SetDlgItemText (hWndDlg,SV_NUM_CLASSES,str);
		         		break;                                      
		         	}
		         	CurTheme->NumDesiredClass--;
		         }
		         
				break;
               
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
		       	 BT_CLOSE (CurTheme->hScatterFile);
		       	 CurTheme->hScatterFile = 0;
                 CloseThemeDataFile(TRUE);
              	 GSSiEndDialog(hWndDlg,FALSE,hSaveBM);    
                 break;
                 
            case SV_FIELD_NAME:
              switch(HIWORD(lParam))
              {
               case CBN_DBLCLK:
               case CBN_SELCHANGE:
            
                  Choice=SendDlgItemMessage(hWndDlg,SV_FIELD_NAME,
									       CB_GETCURSEL,NULL,NULL); 
				  if(Choice >= 0)
				  {
						LPFIELDINFO	lpFieldInfo;
						LPOPENFILEDATA	FilePtr;
						LPOPENSQLDATA	SQLPtr;
						LPHANDLE	lpFileHandle;    
						
						if (!OpenThemeDataFile())
							break;
						SQLPtr = (LPOPENSQLDATA)GlobalLock(CurTheme->hThemeDB);
						FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
						lpFieldInfo = &FilePtr->FldInfo;
						lpFieldInfo += Choice;
						CurTheme->Field = *lpFieldInfo;   
                  		CurTheme->Field.length = GetValListFieldLen (&CurTheme->Field);
  				 		CurTheme->Field.type = SQL_VARCHAR; 
  				 		GlobalUnlock (SQLPtr->OFHandle); 
  				 		GlobalUnlock (CurTheme->hThemeDB);
				   }
		           break;  
		      
		      }// end of the switch       		  		    
              break;
              
            case SV_DISPERSE:
				SendDlgItemMessage (hWndDlg,SV_ACCUMULATE,BM_SETCHECK,FALSE,0L);
                break;
                
            case SV_ACCUMULATE:
				SendDlgItemMessage (hWndDlg,SV_DISPERSE,BM_SETCHECK,FALSE,0L);
            	if (SendDlgItemMessage (hWndDlg,SV_ACCUMULATE,BM_GETCHECK,0,0L))
		        {
			    	FARPROC lpfnACCUMPOINTOPTMsgProc; 
				    	
					DoPaint = FALSE; 
			        lpfnACCUMPOINTOPTMsgProc = MakeProcInstance((FARPROC)ACCUMPOINTOPTMsgProc, hInst);
			        DialogBox(hInst, (LPSTR)"ACCUMPOINTOPT", hWndDlg, lpfnACCUMPOINTOPTMsgProc);
			        FreeProcInstance(lpfnACCUMPOINTOPTMsgProc);
					DoPaint=TRUE;
				}
                break;
                
            case IDC_SAVE_THEME:  
 
            case IDOK: 
            	 GetDlgItemText (hWndDlg,SV_FIELD_VALUE,CurTheme->Value,sizeof(CurTheme->Value));  
            	 CurTheme->GetFirstClass = SendDlgItemMessage(hWndDlg,IDC_MULTIVALUEOPT,
									       CB_GETCURSEL,NULL,NULL);
                 CurTheme->FieldFun=SendDlgItemMessage(hWndDlg,SV_FIELDFUNCTION,
									       CB_GETCURSEL,NULL,NULL);
            	 if (CurTheme->DataFileType == UMIFS_DATAFILE || CurTheme->DataFileType == GMCENSUS_DATAFILE)
            	 	GetDlgItemText (hWndDlg,SV_DATABASE_LIST,CurTheme->DataFile,sizeof(CurTheme->DataFile));
                 CurTheme->DataType=SendDlgItemMessage(hWndDlg,SV_ITEM_TYPE,
									       CB_GETCURSEL,NULL,NULL);
       	 		 GetDlgItemText (hWndDlg,IDC_SQL,CurTheme->SQL,256); 
                 CurTheme->NumCols=SendDlgItemMessage(hWndDlg,SV_COLUMNS,CB_GETCURSEL,NULL,NULL);
                 Choice=SendDlgItemMessage(hWndDlg,SV_FIELD_NAME,
									       CB_GETCURSEL,NULL,NULL);
		         SendDlgItemMessage(hWndDlg,SV_FIELD_NAME,CB_GETLBTEXT,
		         		  		    Choice,(DWORD)&CurTheme->Field.name);

         		 CurTheme->TitleHeight=GetDlgItemInt (hWndDlg,IDC_TITSIZE,&Error,TRUE);
         		 CurTheme->Margin=GetDlgItemInt (hWndDlg,IDC_MARGIN,&Error,TRUE);
         		 CurTheme->InnerMargin=GetDlgItemInt (hWndDlg,IDC_INMARGIN,&Error,TRUE);
         		 CurTheme->ColorsWidth=GetDlgItemInt (hWndDlg,IDC_BOXSIZE,&Error,TRUE);
                
       	 		 GetDlgItemText (hWndDlg,SV_TITLE,CurTheme->Title,256);
       	 		 for (i=0;i<4;i++)
            	 	if (SendDlgItemMessage (hWndDlg,IDC_Missing_Opts[i],BM_GETCHECK,0,0L))
            	 		CurTheme->MissOpt = i;
            	 if ((CurTheme->AutoClassDef = SendDlgItemMessage (hWndDlg,IDC_AUTOCLASSDEF,BM_GETCHECK,0,0L)))
            	 {
                    CurTheme->hScatterFile = CreateClassValueList();
			       	BT_CLOSE (CurTheme->hScatterFile);
			       	CurTheme->hScatterFile = 0;
            	 }
            	 CurTheme->UseFirstSymbol = SendDlgItemMessage (hWndDlg,IDC_USEFIRSTSYMBOL,BM_GETCHECK,0,0L);
            	 if (!(CurTheme->ComputeStoredCounts = SendDlgItemMessage (hWndDlg,SV_CB_RECOMPUTE,BM_GETCHECK,0,0L)))
            	 	CurTheme->UseStoredCounts = FALSE;
            	 CurTheme->ClearIfNoCount = SendDlgItemMessage (hWndDlg,SV_HIDENOCOUNT,BM_GETCHECK,0,0L); 
            	 CurTheme->ShowValue = SendDlgItemMessage (hWndDlg,SV_DISPLAY_VALUE,BM_GETCHECK,0,0L); 
            	 CurTheme->DelayTextDisplay = SendDlgItemMessage (hWndDlg,SV_DELAY_VALUE,BM_GETCHECK,0,0L); 
            	 CurTheme->MarkInvalid = SendDlgItemMessage (hWndDlg,IDC_MARK_INVALID,BM_GETCHECK,0,0L); 
            	 CurTheme->SkipInvalid = SendDlgItemMessage (hWndDlg,IDC_SKIP_INVALID,BM_GETCHECK,0,0L); 
            	 CurTheme->DispersePoints = 0;
            	 if (SendDlgItemMessage (hWndDlg,SV_DISPERSE,BM_GETCHECK,0,0L))
            	 	CurTheme->DispersePoints = 1;
            	 if (SendDlgItemMessage (hWndDlg,SV_ACCUMULATE,BM_GETCHECK,0,0L))
            	 	CurTheme->DispersePoints = 2;

		       	 GetDlgItemText (hWndDlg,SV_NUM_CLASSES,str,3);
		       	 CurTheme->NumDesiredClass = atoi (str);
				 CurTheme->NumClass = CurTheme->NumDesiredClass;    
		       	 if (str[0]) CurTheme->YLimit = atof (str); 
		       	 if (wParam == IDC_SAVE_THEME)
		       	   	SaveCurTheme(hWndDlg);
		       	 else
		       	 {
			       	 SetCurView (SaveView);
			       	 BT_CLOSE (CurTheme->hScatterFile);
			       	 CurTheme->hScatterFile = 0;
	                 CloseThemeDataFile(TRUE);
	              	 GSSiEndDialog(hWndDlg,TRUE,hSaveBM);    
	             }
                 break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1250);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1250);
#endif
 return TRUE;
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
	switch (CurTheme->ID)
	{   
		
		case GF_GRAPHICS_FUNCTION_THEME:			 
		case GF_SINGLE_VALUE_THEME:
	         {
	          FARPROC lpfnSV_THEME1MsgProc;
	          DoPaint = FALSE;
	          lpfnSV_THEME1MsgProc = MakeProcInstance((FARPROC)SV_THEME1MsgProc, hInst);
	          nRc = DialogBox(hInst, (LPSTR)"SV_THEME1", hWnd, lpfnSV_THEME1MsgProc);
	          FreeProcInstance(lpfnSV_THEME1MsgProc);
	          DoPaint = TRUE;
	         }

		break; 
				
		case GF_SINGLE_NONNUM_VALUE_THEME:
	         {
	          FARPROC lpfnSV_THEME2MsgProc;
	          DoPaint = FALSE;
	          lpfnSV_THEME2MsgProc = MakeProcInstance((FARPROC)SV_THEME2MsgProc, hInst);
	          nRc = DialogBox(hInst, (LPSTR)"SV_THEME2", hWnd, lpfnSV_THEME2MsgProc);
	          FreeProcInstance(lpfnSV_THEME2MsgProc);
	          DoPaint = TRUE;
	         }

		break; 
				
		case GF_STREET_ADDRESS_THEME:
	         {
	          FARPROC lpfnEDIT_ADDS_THEMEMsgProc;
	          DoPaint = FALSE;
	          lpfnEDIT_ADDS_THEMEMsgProc = MakeProcInstance((FARPROC)EDIT_ADDS_THEMEMsgProc, hInst);
	          nRc = DialogBox(hInst, (LPSTR)"EDIT_ADDS_THEME", hWnd, lpfnEDIT_ADDS_THEMEMsgProc);
	          FreeProcInstance(lpfnEDIT_ADDS_THEMEMsgProc);
	          DoPaint = TRUE;
	         }   
	    break;
			         
		case GF_TRANSFORM_THEME:
	         {
	          FARPROC lpfnTRANTHEMEMsgProc;
	          DoPaint = FALSE;
	          lpfnTRANTHEMEMsgProc = MakeProcInstance((FARPROC)TRANTHEMEMsgProc, hInst);
	          nRc = DialogBox(hInst, (LPSTR)"TRANTHEME", hWnd, lpfnTRANTHEMEMsgProc);
	          FreeProcInstance(lpfnTRANTHEMEMsgProc);
	          DoPaint = TRUE;
	         } 
	    break;  
			         
		case GF_POLYINFO_THEME:
	         {
	          FARPROC lpfnTRANTHEMEMsgProc;
	          DoPaint = FALSE;
	          lpfnTRANTHEMEMsgProc = MakeProcInstance((FARPROC)TRANTHEMEMsgProc, hInst);
	          nRc = DialogBox(hInst, (LPSTR)"TRANTHEME", hWnd, lpfnTRANTHEMEMsgProc);
	          FreeProcInstance(lpfnTRANTHEMEMsgProc);
	          DoPaint = TRUE;
	         } 
	    break;  
			         
		case GF_HOTSPOT_THEME:
	         {
	          FARPROC lpfnHOTSPOT_THEMEMsgProc;
	          DoPaint = FALSE;
	          lpfnHOTSPOT_THEMEMsgProc = MakeProcInstance((FARPROC)HOTSPOT_THEMEMsgProc, hInst);
	          nRc = DialogBox(hInst, (LPSTR)"HOTSPOT_THEME", hWnd, lpfnHOTSPOT_THEMEMsgProc);
	          FreeProcInstance(lpfnHOTSPOT_THEMEMsgProc);
	          DoPaint = TRUE;
	         } 
	    break;  
			         
		case PF_COORD_DISPLAY:
	         {
	          FARPROC lpfnCOORDDISPLAYMsgProc;
	          DoPaint = FALSE;
	          lpfnCOORDDISPLAYMsgProc = MakeProcInstance((FARPROC)COORDDISPLAYMsgProc, hInst);
	          nRc = DialogBox(hInst, (LPSTR)"COORDDISPLAY", hWnd, lpfnCOORDDISPLAYMsgProc);
	          FreeProcInstance(lpfnCOORDDISPLAYMsgProc);
	          DoPaint = TRUE;
	         } 
	    break;  
			    
		case GF_STREET_TEXT_THEME:
	    {   
	    	LPVIEWPORT	SaveView; 
	    	short		n,i;
			FARPROC lpfnSTREET_TEXTMsgProc;
			    	
			DoPaint = FALSE;
			lpfnSTREET_TEXTMsgProc = MakeProcInstance((FARPROC)STREET_TEXTMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"STREET_TEXT", hWnd, lpfnSTREET_TEXTMsgProc);
			FreeProcInstance(lpfnSTREET_TEXTMsgProc);
			DoPaint = TRUE;
        }  
		break;
				
		case GF_PROFILE_THEME:
	    {   
	    	LPVIEWPORT	SaveView; 
	    	short		n,i;
			FARPROC lpfnPROFILEMsgProc;
			    	
			DoPaint = FALSE;
			lpfnPROFILEMsgProc = MakeProcInstance((FARPROC)PROFILEMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"PROFILE", hWnd, lpfnPROFILEMsgProc);
			FreeProcInstance(lpfnPROFILEMsgProc);
			DoPaint = TRUE;
        }  
		break;
				
		case GF_DISTANCE_THEME:
	    {    
	    	LPVIEWPORT	SaveView; 
	    	short		n,i;
			FARPROC lpfnDISTANCEMsgProc;
			    	
			DoPaint = FALSE;
			lpfnDISTANCEMsgProc = MakeProcInstance((FARPROC)DISTANCEMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"DISTANCE", hWnd, lpfnDISTANCEMsgProc);
			FreeProcInstance(lpfnDISTANCEMsgProc);
			DoPaint = TRUE;
        }  
		break;
				
		case GF_COORDGRID_THEME:
	    {    
	    	LPVIEWPORT	SaveView; 
	    	short		n,i;
			FARPROC lpfnCOORDGRIDMsgProc;
			    	
			DoPaint = FALSE;
			lpfnCOORDGRIDMsgProc = MakeProcInstance((FARPROC)COORDGRIDMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"COORDGRID", hWnd, lpfnCOORDGRIDMsgProc);
			FreeProcInstance(lpfnCOORDGRIDMsgProc);
			DoPaint = TRUE;
        }  
		break;
				
		case GF_2D_THEME:
	    {    
	    	LPVIEWPORT	SaveView; 
	    	short		n,i;
			FARPROC lpfnDISPLAY2DMsgProc;
			    	
			DoPaint = FALSE;
			lpfnDISPLAY2DMsgProc = MakeProcInstance((FARPROC)DISPLAY2DMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"DISPLAY2D", hWnd, lpfnDISPLAY2DMsgProc);
			FreeProcInstance(lpfnDISPLAY2DMsgProc);
			DoPaint = TRUE;
        }  
		break;
				
		case GF_TIME_DISPLAY_THEME:
	    {    
	    	LPVIEWPORT	SaveView; 
	    	short		n,i;
			FARPROC lpfnTIME_DISPLAYMsgProc;
			    	
			DoPaint = FALSE;
			lpfnTIME_DISPLAYMsgProc = MakeProcInstance((FARPROC)TIME_DISPLAYMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"TIME_DISPLAY", hWnd, lpfnTIME_DISPLAYMsgProc);
			FreeProcInstance(lpfnTIME_DISPLAYMsgProc);
			DoPaint = TRUE;
        }  
		break;
				
		case GF_POINT_IN_AREA_THEME:
	    {    
	    	LPVIEWPORT	SaveView; 
	    	short		n,i;
			FARPROC lpfnPOINT_IN_AREAMsgProc;
			    	
			DoPaint = FALSE;
			lpfnPOINT_IN_AREAMsgProc = MakeProcInstance((FARPROC)POINT_IN_AREAMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"POINT_IN_AREA", hWnd, lpfnPOINT_IN_AREAMsgProc);
			FreeProcInstance(lpfnPOINT_IN_AREAMsgProc);
			DoPaint = TRUE;
        }  
		break;
				
		case GF_COMPARE_VIEWPORTS_THEME:
	    {    
	    	LPVIEWPORT	SaveView; 
	    	short		n,i;
			FARPROC lpfnCOMPARE_VIEWPORTSMsgProc;
			    	
			DoPaint = FALSE;
			lpfnCOMPARE_VIEWPORTSMsgProc = MakeProcInstance((FARPROC)COMPARE_VIEWPORTSMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"COMPARE_VIEWPORTS", hWnd, lpfnCOMPARE_VIEWPORTSMsgProc);
			FreeProcInstance(lpfnCOMPARE_VIEWPORTSMsgProc);
			DoPaint = TRUE;
        }  
		break;
				
	    case GF_BOUNDS_DISPLAY_THEME:     
    	case GF_CONTEST_THEME:
	    case GF_NETMARKER_THEME:  
	    case GF_DYNAMIC_SEG_THEME:
        case GF_CACHE_DISPLAY_THEME:
	    {   // dont remove without documenting why!!
	    	LPVIEWPORT	SaveView; 
	    	short		n,i;
			FARPROC lpfnBOUNDS_DISPLAYMsgProc;
			    	
			DoPaint = FALSE;
			lpfnBOUNDS_DISPLAYMsgProc = MakeProcInstance((FARPROC)BOUNDS_DISPLAYMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"BOUNDS_DISPLAY", hWnd, lpfnBOUNDS_DISPLAYMsgProc);
			FreeProcInstance(lpfnBOUNDS_DISPLAYMsgProc);
			DoPaint = TRUE;
        }  
		break;
				
		case GF_DOCUMENTS_THEME:
	         {
	          FARPROC lpfnDOCUMENTSMsgProc;
	          DoPaint = FALSE;
	          lpfnDOCUMENTSMsgProc = MakeProcInstance((FARPROC)DOCUMENTSMsgProc, hInst);
	          nRc = DialogBox(hInst, (LPSTR)"DOCUMENTS", hWnd, lpfnDOCUMENTSMsgProc);
	          FreeProcInstance(lpfnDOCUMENTSMsgProc);
	          DoPaint = TRUE;
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

BOOL ThemeEdit (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1252);
#endif
{	int nRc;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = FALSE;
		PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
   		break;
    
    case GF_REINIT:
    case WM_LBUTTONUP:
    case GF_EXECUTE:  
   	    if (CurView->pTheme)
   	    {
   	    	CurTheme=CurView->pTheme; 
   	    	nRc = EditTheme (hWnd,0);
			switch (CurTheme->ID)
			{   
					 
				case GF_SINGLE_VALUE_THEME:    
				case GF_HOTSPOT_THEME:
				case GF_STREET_ADDRESS_THEME:
				case GF_SINGLE_NONNUM_VALUE_THEME:
				case GF_TRANSFORM_THEME: 
				case GF_2D_THEME:   
				case GF_COMPARE_VIEWPORTS_THEME:
				case GF_DOCUMENTS_THEME:
				case GF_POLYINFO_THEME: 
				case GF_GRAPHICS_FUNCTION_THEME:
			          if (nRc && CurTheme->IsActive)
			          {
						HaveCurrentLBUTTON=TRUE;
			          	AddGraphicsFunction (CurView->hWnd, GF_THEME_ACTIVATE,0);
			          }	
				break; 
				
				case GF_DISTANCE_THEME:  
				case GF_COORDGRID_THEME:
				case GF_STREET_TEXT_THEME:
			    case GF_BOUNDS_DISPLAY_THEME:     
			    case GF_PROFILE_THEME:     
	        	case GF_CONTEST_THEME:
			    case GF_NETMARKER_THEME: 
			    case GF_DYNAMIC_SEG_THEME: 
		        case GF_CACHE_DISPLAY_THEME:
			    {   // dont remove without documenting why!!
			    	if (CurView->CurrentFunction == GF_THEME_EDIT)  
			        	PostMessage(hWnd, GF_CLOSE,0, 0L); 
                }  
				break;
				
			}
		} 
		if (nRc)
{
#if ENABLETRACE
GSSiExitProg (1252);
#endif
			return GF_INCREASE_SUCCESS_COUNT; 
}
   		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (1252);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1252);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}
BOOL ThemeActivate (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1253);
#endif
{	LPVIEWPORT SaveView, DisplayView=0;
	int	iview, i, n;

 switch (Message)
   {
    case GF_EXECUTE:
    case WM_LBUTTONUP:
		HaltReport=0;
   	    if (CurView->pTheme)
   	    {
   	    	CurTheme=CurView->pTheme;
   	    	CurTheme->IsActive = TRUE; 
		    UnloadReport (&CurView->hReport);  
   	    	
			if (CurTheme->ClassType < 3 &&
			    CurTheme->ID != GF_BOUNDS_DISPLAY_THEME && 
			    CurTheme->ID != GF_DISTANCE_THEME &&
			    CurTheme->ID != GF_COORDGRID_THEME &&  
			    CurTheme->ID != GF_DYNAMIC_SEG_THEME &&
			    CurTheme->ID != GF_PROFILE_THEME &&
				CurTheme->ID != GF_TRANSFORM_THEME && 
				CurTheme->ID != GF_POLYINFO_THEME && 
				CurTheme->ID != GF_GRAPHICS_FUNCTION_THEME &&
				CurTheme->ID != GF_2D_THEME)
			{   
				*CurTheme->RefValChar = 0;
		   		CurTheme->WantDataPass=TRUE;
				CurTheme->ComputeClassBoundaries=TRUE;
			}
			else
			{   
			   	CurTheme->WantDataPass=CurTheme->Recompute;//10/17/03
				CurTheme->ComputeClassBoundaries=FALSE;
			}
			SaveView=CurView;
			for (iview=0;iview<*pNumViewports;iview++)
			{
				SetCurView (pViewports[iview]);
				CurView->Display=FALSE;
			}
			SetCurView (SaveView);
			CurView->Display = TRUE; 
			if (CurTheme->TargetViewport)
			{
		    	if (CurTheme->TargetViewport > *pNumViewports)
		    		CurTheme->TargetViewport = 1;
				SetCurView (pViewports[CurTheme->TargetViewport-1]);  
				DisplayView = CurView;
				for (i=0,n=0;i<CurView->NumThemes;i++)
				{ 
					if (CurView->pThemes[i]==CurTheme)
						n++;
				}
				if (!n)
					CurView->pThemes[CurView->NumThemes++]=CurTheme;
				CurView->Display = TRUE;  
				SetCurView (SaveView);
			}

		} 
        PostMessage(hWnd, GF_CLOSE,0, 0L); 
        if (DisplayView)    
//        	InvalidateRect (hWndMain,NULL,TRUE);
        	PostMessage(hWndMain, WM_COMMAND,IDM_REDISPLAYVIEWPORTS, 0L); 
		break;

   	case GF_INIT:
        AddLBUTTON = TRUE;  
   		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (1253);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1253);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL ThemeDeActivate (HWND hWnd, WORD Message, WORD wParam, LONG lParam,short Function)
#if ENABLETRACE
{GSSiEnterProg (1254);
#endif
{

 switch (Message)
   {
   	case GF_INIT: 
   	{
   		LPTHEME	SaveTheme = CurTheme;
   		
   	    if (CurView->pTheme)
   	    {   
   	    	CurTheme=CurView->pTheme;  
   	    	if (SaveTheme == CurTheme)
   	    		SaveTheme = 0;
   	    	if (Function == GF_THEME_REMOVE)
   	    	{
	    		LPVIEWPORT	SaveView = CurView; 
	    		short	i,n;
	    		
	    		SetCurView (pViewports[CurTheme->TargetViewport-1]);
	    		for (i=0,n=0;i<CurView->NumThemes;i++)
	    		{
	    			if (CurView->pThemes[i]!=CurTheme)
	    				CurView->pThemes[n++]=CurView->pThemes[i];
	    		}	
	    		CurView->NumThemes=n;
	    		SetCurView (SaveView);
	   	    	CloseObject (CurView->pTheme);
	   	    	CurView->pTheme = 0; 
	   	    	SetCurView (SaveView);
   	    	}
   	    	else
   	    	{
	   	    	CurTheme->IsActive = FALSE;
			   	CurTheme->WantDataPass=FALSE;
				CurTheme->ComputeClassBoundaries=FALSE; 
				DeletePointDispersionFile ();
				ResetViewport (TRUE,FALSE); 
				ResetOvrlapViewport (CurView->Rect,CurView->ID);
            }
            CurTheme = SaveTheme;
		}
        PostMessage(hWnd, GF_CLOSE,0, 0L); 
		PostMessage(hWndMain, WM_COMMAND, IDM_Z_REDRAW, 0L); 
	}
   		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (1254);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1254);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL LoadTheme (HWND hWnd)
{
    FARPROC lpfnLOADTHEMEMsgProc; 
	LPVIEWPORT	SaveView;
    int	nRc, i, n; 
    BOOL	rtn=FALSE;
        
	DoPaint=FALSE;
    lpfnLOADTHEMEMsgProc = MakeProcInstance((FARPROC)LOADTHEMEMsgProc, hInst);
    nRc = DialogBox(hInst, (LPSTR)"LOADTHEME", hWnd, lpfnLOADTHEMEMsgProc);
    FreeProcInstance(lpfnLOADTHEMEMsgProc);
    DoPaint=TRUE;
    if (nRc)
    {   
    	rtn = nRc; 
		if (CurView->pTheme)
		{   
			CurTheme = CurView->pTheme;
    		SaveView = CurView;
    		SetCurView (pViewports[CurTheme->TargetViewport-1]);
    		for (i=0,n=0;i<CurView->NumThemes;i++)
    		{
    			if (CurView->pThemes[i]!=CurTheme)
    				CurView->pThemes[n++]=CurView->pThemes[i];
    		}	
    		CurView->NumThemes=n;
    		SetCurView (SaveView);
   	    	CloseObject (CurView->pTheme);
   	    } 
   	    CurTheme = NewTheme;
    	CurView->pTheme = NewTheme;
        CurView->pTheme->DisplayViewport = CurView->ID;
      	if (nRc == 1)
      	{
    		SaveView = CurView;
    		SetCurView (pViewports[CurTheme->TargetViewport-1]);
    		CurView->pThemes[CurView->NumThemes]=NewTheme;
    		CurView->NumThemes++;
    		SetCurView (SaveView); 
    		if (NewTheme->IsActive) 
    		{
			    UnloadReport (&CurView->hReport);  
    		}
    	}
    	else 
    	{
			HaveCurrentLBUTTON=TRUE;
      		AddGraphicsFunction (CurView->hWnd, GF_THEME_EDIT,0);
		} 
    }
    return rtn;
}

BOOL ThemeLoad (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1255);
#endif
{	int nRc;

 switch (Message)
   {
   	case GF_INIT: 
       	AddLBUTTON = FALSE;
		PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
   		break;
    
    case GF_COMPLETE:
    	PostMessage(hWnd, GF_CLOSE,0, 0L); 
		break;
	
	case GF_REINIT:	
    case WM_LBUTTONUP:
    case GF_EXECUTE:   
    	if (LoadTheme (hWnd))
    		if (CurView->pTheme->IsActive)
    			PostMessage(hWndMain, WM_COMMAND, IDM_Z_REDRAW, 0L);
    	PostMessage(hWnd, GF_CLOSE,0, 0L); 

   		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (1255);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1255);
#endif
    return (TRUE);
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
		GSSiGlobFree (&CurTheme->hVisList);
		SaveView = CurView;
		SetCurView (pViewports[CurTheme->TargetViewport-1]);
		for (i=0,n=0;i<CurView->NumThemes;i++)
		{
			if (CurView->pThemes[i]!=CurTheme)
				CurView->pThemes[n++]=CurView->pThemes[i];
		}	
		CurView->NumThemes=n;
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

BOOL FAR PASCAL LOADTHEMEMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1257);
#endif
{
	COLORREF	Color; 
	HFILE		Fid;
	int			Choice, rtn; 
	LPSTR		lpBS;
	char		Name[128], str[128];   
	static		HANDLE	hSaveBM=0;
	

 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1257);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:  
         hSaveBM = EnterBlockingWindow (hWndDlg);
   	     _getcwd (CurDir,128);
       	 SendDlgItemMessage (hWndDlg,IDC_ACTIVATE,BM_SETCHECK,TRUE,0L);
       	 SendDlgItemMessage (hWndDlg,IDC_GOTOEDIT,BM_SETCHECK,FALSE,0L);
  	     SaveDrive = _getdrive();
		 SendDlgItemMessage (hWndDlg,IDC_NEW_THEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Single Numeric Value"));
		 SendDlgItemMessage (hWndDlg,IDC_NEW_THEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Single Non-Numeric Value"));
//		 SendDlgItemMessage (hWndDlg,IDC_NEW_THEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Two Numeric Value"));
//		 SendDlgItemMessage (hWndDlg,IDC_NEW_THEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Compare to Reference"));
		 SendDlgItemMessage (hWndDlg,IDC_NEW_THEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Network Mile Markers"));
		 SendDlgItemMessage (hWndDlg,IDC_NEW_THEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Attached Document"));
		 SendDlgItemMessage (hWndDlg,IDC_NEW_THEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Street Name Theme"));
		 SendDlgItemMessage (hWndDlg,IDC_NEW_THEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Street Segment Edit Helper"));
		 SendDlgItemMessage (hWndDlg,IDC_NEW_THEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Network Connectivity Theme"));
		 SendDlgItemMessage (hWndDlg,IDC_NEW_THEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Transformation Theme"));
		 SendDlgItemMessage (hWndDlg,IDC_NEW_THEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Bounds Display Theme"));
		 SendDlgItemMessage (hWndDlg,IDC_NEW_THEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Distance Display Theme"));
		 SendDlgItemMessage (hWndDlg,IDC_NEW_THEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Coordinate Grid Theme"));
		 SendDlgItemMessage (hWndDlg,IDC_NEW_THEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Dynamic Segmentation Theme"));
		 SendDlgItemMessage (hWndDlg,IDC_NEW_THEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Profile Theme"));
		 SendDlgItemMessage (hWndDlg,IDC_NEW_THEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"2 Dimension Legend Theme"));
		 SendDlgItemMessage (hWndDlg,IDC_NEW_THEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Hot Spot Theme"));
		 SendDlgItemMessage (hWndDlg,IDC_NEW_THEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Polygon Info Theme"));
		 SendDlgItemMessage (hWndDlg,IDC_NEW_THEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Graphics Function Theme"));
		 SendDlgItemMessage (hWndDlg,IDC_NEW_THEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Display Cache Theme"));
		 SendDlgItemMessage (hWndDlg,IDC_NEW_THEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Compare Viewports Theme"));
		 SendDlgItemMessage (hWndDlg,IDC_NEW_THEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Time Display Theme"));
		 SendDlgItemMessage (hWndDlg,IDC_NEW_THEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Point In Area Theme"));
     case GSSI_REINITDIALOG:
		 SendDlgItemMessage (hWndDlg,IDC_AVAIL_THEMES,LB_RESETCONTENT,NULL,NULL);  
		 GetCurVal (str,sizeof(str),IDS_FILETHM);
		 _fullpath (Name,str,sizeof(Name));   
		 if (*LastChr(str) == '\\')
		 	_fstrcat (Name,"\\*.thm");
		 else
		 {
			 if ((lpBS = _fstrrchr (Name,'\\')))
			 	_fstrcpy (lpBS,"\\*.thm");
		 }
		 DlgDirList (hWndDlg,Name,IDC_AVAIL_THEMES,NULL,DDL_READWRITE);

		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           {
           	case IDC_SWITCHDIR:
             	_fstrcpy (gszFilter,"GeoMaster Themes(*.thm)|*.THM|"); 
             	*str = 0;
				if (GetFileName3(hWndDlg,str,0,IDS_FILETHM))
           	    	PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
           		break;
           		
            case IDC_AVAIL_THEMES:
				switch(HIWORD(lParam))
				{
				 case LBN_DBLCLK:   
//					IgnoreLbutton = TRUE;
				 	PostMessage(hWndDlg,WM_COMMAND,IDOK,0L);
				 break;
				 
				 case LBN_SELCHANGE:
				 SendDlgItemMessage(hWndDlg,IDC_NEW_THEME, LB_SETCURSEL,-1,NULL);
				 Choice=SendDlgItemMessage(hWndDlg,IDC_AVAIL_THEMES, LB_GETCURSEL,NULL,NULL);
				 SendDlgItemMessage(hWndDlg,IDC_AVAIL_THEMES,LB_GETTEXT,
				 		  		    Choice,(DWORD)&Name);
				 Fid = GSSiOpenFile (Name,NULL,OF_READ); 
				 ReadObject (&Fid,TRUE,&NewTheme,NULL); 
				 GSSiClose (Fid);
				 SetDlgItemText (hWndDlg,IDC_TITLE,NewTheme->Title);
	             NewTheme->DisplayViewport = CurView->ID;
				 if (NewTheme->TargetViewport <= *pNumViewports)
	    		 	pViewports[NewTheme->TargetViewport-1]->NumThemes--;
				 CloseObject (NewTheme);
				}
			     break; 
			     
            case IDC_NEW_THEME:
				switch(HIWORD(lParam))
				{
				 case LBN_SELCHANGE:
				 	SendDlgItemMessage(hWndDlg,IDC_AVAIL_THEMES, LB_SETCURSEL,-1,NULL);
				 break;
				 
 				 case LBN_DBLCLK: 
//					IgnoreLbutton = TRUE;
				 	PostMessage(hWndDlg,WM_COMMAND,IDOK,0L);
 				 break;

				}
			     break; 
			     
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
			   	 _chdir (CurDir);
           	  	 _chdrive (SaveDrive);
              	 GSSiEndDialog(hWndDlg, FALSE,hSaveBM);    
                 break;
                  
            case IDOK:
				 Choice=SendDlgItemMessage(hWndDlg,IDC_AVAIL_THEMES, LB_GETCURSEL,NULL,NULL);
				 if (Choice >= 0)
				 {
					 SendDlgItemMessage(hWndDlg,IDC_AVAIL_THEMES,LB_GETTEXT,
					 		  		    Choice,(DWORD)&Name);
					 Fid = GSSiOpenFile (Name,NULL,OF_READ); 
					 ReadObject (&Fid,TRUE,&NewTheme,0); 
					 GSSiClose (Fid);
		             NewTheme->DisplayViewport = CurView->ID;
				 	 if (NewTheme->TargetViewport > *pNumViewports) 
				 	 	NewTheme->TargetViewport = 1; 
				 	 if (pViewports[NewTheme->TargetViewport-1]->NumThemes)
	    		 	 	pViewports[NewTheme->TargetViewport-1]->NumThemes--;
					 NewTheme->IsActive=FALSE;
					 rtn = 1;
					 NewTheme->IsActive=SendDlgItemMessage (hWndDlg,IDC_ACTIVATE,BM_GETCHECK,0,0L);
					 if (SendDlgItemMessage (hWndDlg,IDC_GOTOEDIT,BM_GETCHECK,0,0L))
					 	rtn = 2;  					 
				 }
				 else 
				 {
					 Choice=SendDlgItemMessage(hWndDlg,IDC_NEW_THEME, LB_GETCURSEL,NULL,NULL);
					 if (Choice >= 0)
					 {
						 SendDlgItemMessage(hWndDlg,IDC_NEW_THEME,LB_GETTEXT,
						 		  		    Choice,(DWORD)&Name);
						 NewTheme = CreateNewTheme (Choice); 
						 NewTheme->IsActive=SendDlgItemMessage (hWndDlg,IDC_ACTIVATE,BM_GETCHECK,0,0L);
						 rtn =2;
					 }
					  
					 else
					 	break;
				 }
		   	     _chdir (CurDir);
       	  	     _chdrive (SaveDrive);
              	 GSSiEndDialog(hWndDlg,rtn,hSaveBM);    
                 break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1257);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1257);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
} /* End of LOADTHEMEMsgProc                                      */
   
BOOL OpenThemeDataFile ()
#if ENABLETRACE
{GSSiEnterProg (1258);
#endif
{ 
    if (CurTheme->hThemeDB)
{
#if ENABLETRACE
GSSiExitProg (1258);
#endif
    	return(TRUE); 
}
  	_fstrcpy (ThemeDB,CurTheme->DataFile);  
  	CurTheme->Statement = NULL;
{
#if ENABLETRACE
GSSiExitProg (1258);
#endif
    return (OpenDataFile (CurTheme->DataFile,CurTheme->SQL,BT_READ,&CurTheme->hThemeDB));
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
    	GlobalFree (CurTheme->hThemeDB);
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
		if (CurTheme->Statement)
			SQLFreeStmt(CurTheme->Statement, SQL_DROP);
	  	CurTheme->Statement = NULL;
{
#if ENABLETRACE
GSSiExitProg (1259);
#endif
    	return (CloseDataFile (FALSE,&CurTheme->hThemeDB));
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

    CurView->HaveOrthos = FALSE; 
    if (DisplayCycle != LastDisplayCycle)
    {
    	FirstClasslessTheme = NULL;
    	LastDisplayCycle = DisplayCycle;
    }
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
			CurTheme->InvalidDataBrush= CreatePen(PS_DASH,1,RGB(255,0,0));
			CurTheme->NoDataBrush= CreatePen(PS_DOT,1,RGB(255,0,0));
		}
		if (!CurTheme->PCTByArea && !CurTheme->UseStoredCounts)
		for (iclass=0;iclass<MAX_THEME_CLASSES;iclass++)
			CurTheme->ClassCount[iclass] = 0; 
        CurTheme->NumMissing = 0;
        CurTheme->NumInvalid = 0;  
        if (CurTheme->ShowValue)
        	BeginDelayedText (CurTheme->FidDelayedText);
		switch (CurTheme->ID)
		{   
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
                
                {
                	LPVIEWPORT SaveVP=CurView;
                	
                	SetViewport(CurTheme->DisplayViewport);
                	if (!_fstricmp (CurView->Name,"Depth Labels"))
                		pStreetData->IgnoreShields = TRUE;
                	CurView = SaveVP;
                }
			    if (pStreetData->NameSource)
			    {
			    	OpenThemeDataFile ();
			
					if (pStreetData->NameFile1[0]) 
					{
						GSSiRemove (pStreetData->NameFile1);
						GSSiRemove (pStreetData->NameFile2);  
					}
					else
					{
						GSSiGetTempFileName (NULL,"gm1",NULL,(LPSTR)pStreetData->NameFile1);
						GSSiGetTempFileName (NULL,"gm2",NULL,(LPSTR)pStreetData->NameFile2); 
					}
			
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
				CurTheme->hScatterFile = GSSiGlobAlloc ( 639,GMEM_MOVEABLE,UINT_MAX); 
				pInt = (LPSHORT)GlobalLock(CurTheme->hScatterFile);
				*pInt = 0;
				GlobalUnlock (CurTheme->hScatterFile);   
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
					if (CurTheme->AutoClassDef && !CurTheme->NumDesiredClass && FirstClasslessTheme != NULL) 
					{
						CurTheme->NextValueColor = FirstClasslessTheme->NextValueColor;
						CurTheme->hScatterFile = BT_OPEN (FirstClasslessTheme->ScatterFile, 0, Mode, 0);
					} 
					else
						CurTheme->hScatterFile = BT_OPEN (CurTheme->ScatterFile, 0, Mode, 0); 
					if (StartAutoClassDef ()) 
					{
						if (CurTheme->NumDesiredClass > 0 || FirstClasslessTheme == NULL) 
						{
							CurTheme->NextValueColor = 0;
							BT_CLEAR (CurTheme->hScatterFile);
						}	 
						if (!CurTheme->NumDesiredClass && FirstClasslessTheme == NULL)
							FirstClasslessTheme = CurTheme;
					}
					else if (CurTheme->FieldFun)
						CurTheme->hScatterFile = ExpandThemeValues (CurTheme->hScatterFile);	
				}
				else
					CurTheme->hScatterFile = NULL;			
			case GF_SINGLE_VALUE_THEME:
			case GF_TIME_DISPLAY_THEME:
			case GF_TWO_VALUE_THEME: 
				OpenThemeHighlightFile (BT_WRITE);
				OpenPointDispersionFile (BT_WRITE);
			    OpenThemeDataFile ();
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
				else
					GSSiGetTempFileName (NULL,"gmt",NULL,(LPSTR)CurTheme->ScatterFile);
			
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
						GSSiGetTempFileName (NULL,"gmt",NULL,(LPSTR)CurTheme->ScatterFile);
					CurTheme->FidAreas = GSSiOpenFile (CurTheme->ScatterFile,NULL,OF_CREATE);  
					CurTheme->NumAreas = 0;  
				}
				else
				{
					OpenThemeHighlightFile (BT_WRITE);
					OpenPointDispersionFile (BT_WRITE);
				    OpenThemeDataFile ();
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


void GetClassMinMax (short iclass,LPDOUBLE pClassMin,LPDOUBLE pClassMax)
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

BOOL ItemProcessedByTheme (LPSTR Prefix,short desc) 
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
	int		status;
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
	hMEM = GSSiGlobAlloc ( 641,GMEM_MOVEABLE,2048);  
	Value = GlobalLock(hMEM);
	KeyVal = Value+512;
	VarVal = KeyVal+512;
    HaltReport = FALSE;
	switch (CurTheme->ID)    
	{   
        case GF_POINT_IN_AREA_THEME: 
        	if (!CurTheme->Pass)    
        		goto RtnNoDisplay;
        case GF_TIME_DISPLAY_THEME:
		case GF_SINGLE_VALUE_THEME: 
			if (!ItemProcessedByTheme (TAG,desc))
					goto RtnNotProcessed;
			SwitchThemeSHPFile ();		
			ValD = GetNumericFieldData (CurTheme->hThemeDB,&CurTheme->Statement,
										&CurTheme->Field,CurTheme->FieldFun,CurTheme->Value,iref,Type,&status);
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
			if (CurTheme->GetFirstClass)
			{  
				short MinClass=MAX_THEME_CLASSES+1;
NextValue:		
				SwitchThemeSHPFile ();		
				status = GetCharFieldData (CurTheme->hThemeDB,&CurTheme->Statement,
								  &CurTheme->Field,iref,CurTheme->FieldFun,CurTheme->Value,Value); 
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
				SwitchThemeSHPFile ();		
				status = GetCharFieldData (CurTheme->hThemeDB,&CurTheme->Statement,
								  &CurTheme->Field,iref,CurTheme->FieldFun,CurTheme->Value,Value); 
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






