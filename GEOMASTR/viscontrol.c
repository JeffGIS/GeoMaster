#include "graphint.h"   
#include "dibapi.h"
#include <commctrl.h>

#include "gmextern.h"

extern int	nToolbars;
extern	HWND	ToolbarWindow[MAX_TOOLBARS];

static	int	symnumALL;

#define MENUWINDOWCLASS	"MenuWindowClass"

#define IDM_AUTOREDRAW			1
#define IDM_DISPLAYSYMBOLS		2
#define IDM_DISPLAYMODIFIERS	3
#define IDM_INTENSITYMOD		4
#define IDM_COLORMOD			5
#define IDM_SIZEMOD				6
#define IDM_RANDOMMOD			7
#define IDM_SETFACTOR			8
#define IDM_SETBKCOLOR			9
#define IDM_SETFONTTABS			10
#define IDM_SETFONTCMD			11
#define IDM_SETFONT				12
#define IDM_SETTXBKCOLOR		13
#define IDM_SETALPHABLEND		14
#define IDM_SETBUTTONH			15
#define IDM_SETBUTTONW			16
#define IDM_VERTTABS			17
#define IDM_TABPAD				18
#define IDM_SETBKCOLORCMD		19

#define MT_CATALOG				1
#define MT_AUTOVIS				2
#define MT_PICKABLE				3
#define MT_VISIBLE				4
#define MT_CMDMENU				5
#define	MT_ALLSYMBOLS			6
#define MT_VISSYMBOLS			7

typedef struct {
				int		expand; //0=no expand box,1=not expanded,2=expanded
				int		type;	//1=symbol,2=parent
				int		level;
				int		layer;
				int		parent;
				int		symnum;
				char	Name[66];
				}VCSYMBOL;
typedef	VCSYMBOL	*LPVCSYMBOL;

typedef struct {
				RECT	rect;
				int		viewport;
				int		level;
				int		layer;
				int		fromlayer;
				int		type;	//1=expandbox,2=onoffbox,3=text
				int		symnum;
				BOOL	AllowDoubleClick;
				}VCRECTANGLE;
typedef	VCRECTANGLE	*LPVCRECTANGLE;

typedef struct {int		nLayers;
				int		Config;
				int		VPID;
				BOOL	Pickability;
				BOOL	PickIsSame;
				BOOL	AutoRedraw;
				BOOL	DisplaySymbols;
				BOOL	DisplayModifiers;
				int		nLayerSyms[MAX_VIEWPORT_FILES+1];
				HANDLE	hLayerSyms[MAX_VIEWPORT_FILES+1];
				HANDLE	hSymOrder[MAX_VIEWPORT_FILES+1];
				BOOL	ExpandViewport;
				BOOL	ExpandLayer[MAX_VIEWPORT_FILES+1];
				int		nRectangles;
				HANDLE	hRectangles;
				int		nModifiers;
				HANDLE	hModifiers;
				int		iCurrentRect;
				}VISCONTROLHEADER;
typedef VISCONTROLHEADER	*LPVISCONTROLHEADER;

typedef struct {int		nAutoVis;
				int		Config;
				int		VPID;
				int		nRectangles;
				HANDLE	hRectangles;
				int		iCurrentRect;
				int		iCurrentAV;
				char	AVDir[MAX_PATH];
				char	AVFile[MAX_AUTOVIS_FILES][64];
				}AUTOVISHEADER;
typedef AUTOVISHEADER	*LPAUTOVISHEADER;

typedef struct {int		nCMD;
				int		Config;
				int		VPID;
				int		nRectangles;
				HANDLE	hRectangles;
				int		iCurrentRect;
				int		iCurrentCMD;
				BOOL	VScrollIsVisPrevious;
				BOOL	VScrollIsVis;
				char	CMDFile[MAX_PATH];
				HBITMAP	hBM[MAX_CMDMENU_COMMANDS];
				long	FileLoc[MAX_CMDMENU_COMMANDS];
				char	Label[MAX_CMDMENU_COMMANDS][64];
				}CMDMENUHEADER;
typedef CMDMENUHEADER	*LPCMDMENUHEADER;

typedef struct {
				RECT	rect;
				int		iAV;
				BOOL	AllowDoubleClick;
				}AVRECTANGLE;
typedef	AVRECTANGLE	*LPAVRECTANGLE;

typedef struct {
				RECT	rect;
				int		iCMD;
				BOOL	AllowDoubleClick;
				}CMDRECTANGLE;
typedef	CMDRECTANGLE	*LPCMDRECTANGLE;

static	int		ShowOnlyThisViewport,ShowOnlyThisLayer,ShowOnlyThisLevel,ShowOnlyThisSymbol;
static	VISLIST	ShowOnlyThisSaveVis;
static	BOOL	HaveShowOnlyVis=FALSE;
static	HANDLE	hInitData=0;
static	RECT	OrigRect;
static	HANDLE	hLevSymbols[MAXVCLEV];
static	int		nLevSymbols[MAXVCLEV];
static	int		Indent=19;
static	int		indent;
static	int		ButtonHeight=24, ButtonWidth=150;
static	int		buttonheight, buttonwidth;
static	COLORREF	CommandButtonBackgroundColor=4227200;//8454143;//RGB(255,255,128);
static	COLORREF	TypeColors[8]={5987163,16777215,0,8404992,16777215,0,0,0};//{RGB(64,196,64),RGB(196,64,64),0,RGB(64,64,196),0,0,0,0};
static	COLORREF	TypeColorsBk[8]={15987699,8404992,0,15921906,0,0,0,0};//RGB(240,240,240),0,0,0,0,0,0,0};
static	char	TypeFontName[8][32]={"Arial","Arial","Microsoft Sans Serif","Microsoft Sans Serif","Arial","Arial","Arial","Arial Rounded MT Bold"};
static	int		TypeFontWidth[8]={FW_BOLD,FW_BOLD,FW_NORMAL,FW_BOLD,FW_BOLD,FW_NORMAL,FW_NORMAL,FW_BOLD};
static	int		TypeFontSizeInc[8]={10,4,0,1,2,2,2,8};
static	DWORD	TypeFontUnderline[8]={0,0,0,0,0,0,0,0};
static	BOOL	floating;
static	int		menuy;
static	BOOL	HaveMoving;
static	int		viewport=1;//temp
static	char	ModifierID[16];
static	int		curviewport,curlayer,cursymbol;
static	HWND	hWndTab=0, hWndMenu=0;
static	BOOL	CreatingParentWindow=FALSE;
static	LPWINDOWMENUHEADER pWMH=0;
static	HANDLE	hWMH=0;
static	LPVISLIST	SaveVis;
static	LPVIEWPORT	SaveVP;
static	int			SaveCfg;
static	BOOL	FirstDisplay;
static	int		ToolbarID;
static	BOOL	InTrackMenu=FALSE;
static	int		VCTextLevel;
static	HANDLE	hInitVis;
int	AlphaBlendFactor=120;
static	BOOL	fromScroll = FALSE;

extern	HWND	g_hwndDisplay;


static	LPVISCONTROLHEADER	pVCHeader;
static	LPAUTOVISHEADER		pAVHeader;
static	LPCMDMENUHEADER		pCMDHeader;

BOOL DrawVisButton (HDC hDC,LPSTR ID,int type,int viewport,int layer,int symnum,LPINT px,LPINT py,int On,int Expand);
BOOL DisplayLayerSymbols (HDC hDC,int level,int parent,int type,int viewport,int layer,LPINT px,LPINT py,int nSyms,HANDLE hSyms,HANDLE hSymOrder);
int GetOnValue (int layer,int symnum,int type,int nSyms,HANDLE hSyms);
void SortLayerSymbols (int nSyms,HANDLE hSyms,LPHANDLE phSymOrder);
void SetVisibilityOfParentSymbol (int symnum,int nSyms,HANDLE hSyms,int status);
void AddVCRect (LPRECT prect,int level,int viewport,int layer,int symnum,int recttype,BOOL AllowDoubleClick);
BOOL FAR COLORINTENSITYMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam);
LONG FAR PASCAL CatalogMenuWndProc(HWND hWnd, int Message, WPARAM wParam, LONG lParam);
LONG FAR PASCAL AutoVisMenuWndProc(HWND hWnd, int Message, WPARAM wParam, LONG lParam);
LONG FAR PASCAL CMDMenuWndProc(HWND hWnd, int Message, WPARAM wParam, LONG lParam);
BOOL DrawAVButton (HDC hDC,LPSTR ID,int iAV,LPINT px,LPINT py,BOOL On);
int DrawCMDButton (HDC hDC,LPSTR ID,HBITMAP hBM,int iCMD,LPINT px,LPINT py,int h,int w);
HANDLE GetMenuWindowHandle (HWND hWnd);

#define TI_DOUBLE	1
#define TI_LONG		2
#define TI_CHAR		3

void ResetShowOnlyVis (void)
{
	HANDLE	handle;

	if (!HaveShowOnlyVis)
		return;
	SetViewport (ShowOnlyThisViewport);
	SelectVisList (0);
	handle = CurVis->hVisList;
	*CurVis = ShowOnlyThisSaveVis;
	CurVis->hVisList = handle;
	HaveShowOnlyVis = FALSE;
	return;
}
void InvertRectBorder (HDC hDC,LPRECT pRect)
{
	RECT	rect=*pRect;

	InvertRect (hDC,&rect);
	InflateRect (&rect,-1,-1);
	InvertRect (hDC,&rect);

	return;
}
void txtInOut (HFILE Fid,int ReadOrWrite,int type,LPVOID pData)
{
	char	str[300];

	if (ReadOrWrite)
	{
		switch (type)
		{
		case TI_DOUBLE:
			sprintf (str,"%f",*(LPDOUBLE)pData);
			break;
		case TI_LONG:
			sprintf (str,"%i",*(LPLONG)pData);
			break;
		case TI_CHAR:
			strcpy (str,(LPSTR)pData);
			break;
		}
		fputstring (str,Fid);
	}
	else
	{
		if (fgetstring (str,256,Fid))
			switch (type)
			{
			case TI_DOUBLE:
				*(LPDOUBLE)pData = atof (str);
				break;
			case TI_LONG:
				*(LPLONG)pData = atoi (str);
				break;
			case TI_CHAR:
				strcpy ((LPSTR)pData,str);
				break;
			}
	}
	return;
}

void TempLoadFloatMenuData (int ReadOrWrite) //read=0,write=1
{
	int	i;
	char	str[128];
	HFILE	Fid = GSSiOpenFile ("[%DL]tempmenudata.txt",0,ReadOrWrite?OF_CREATE:OF_READ);

	if (Fid == HFILE_ERROR)
		return;
	txtInOut (Fid,ReadOrWrite,TI_DOUBLE,&pWMH->Factor);
	txtInOut (Fid,ReadOrWrite,TI_LONG,&pWMH->Verticle);
	txtInOut (Fid,ReadOrWrite,TI_LONG,&pWMH->xpad);
	txtInOut (Fid,ReadOrWrite,TI_LONG,&pWMH->BackgroundColor);
	
	txtInOut (Fid,ReadOrWrite,TI_LONG,&ButtonHeight);
	txtInOut (Fid,ReadOrWrite,TI_LONG,&AlphaBlendFactor);
	txtInOut (Fid,ReadOrWrite,TI_LONG,&CommandButtonBackgroundColor);
	for (i=0;i<8;i++)
	{
		txtInOut (Fid,ReadOrWrite,TI_LONG,&TypeColors[i]);
		txtInOut (Fid,ReadOrWrite,TI_LONG,&TypeColorsBk[i]);
		txtInOut (Fid,ReadOrWrite,TI_LONG,&TypeFontWidth[i]);
		txtInOut (Fid,ReadOrWrite,TI_LONG,&TypeFontUnderline[i]);
		txtInOut (Fid,ReadOrWrite,TI_LONG,&TypeFontSizeInc[i]);
		txtInOut (Fid,ReadOrWrite,TI_CHAR,TypeFontName[i]);
	}
	GSSiClose2 (&Fid);
	return;
}

void TempSaveFloatMenuData (void)
{
//	TempLoadFloatMenuData (1);

	return;
}

void DisplayTabbedMenu (HWND hWnd)
{
	HMENU	EditMenu=CreatePopupMenu();   
	POINT	position;
return;
	AppendMenu (EditMenu,MF_ENABLED|MF_STRING,IDM_SETFONTTABS,"Set tab font");
	AppendMenu (EditMenu,MF_ENABLED|MF_STRING,IDM_TABPAD,"Set padding");
	AppendMenu (EditMenu,MF_ENABLED|MF_STRING|(pWMH->Verticle?MF_CHECKED:MF_UNCHECKED),IDM_VERTTABS,"Verticle tabs");
	AppendMenu (EditMenu,MF_ENABLED|MF_STRING,65003,"Cancel");
	GetCursorPos (&position);
	InTrackMenu = TRUE;
	TrackPopupMenu (EditMenu,TPM_LEFTBUTTON|TPM_CENTERALIGN|TPM_VCENTERALIGN,position.x,position.y,0,hWnd,0);
	InTrackMenu = FALSE;
	DestroyMenu (EditMenu); 
	return;
}

void EditVCText (HWND hWnd,int type)
{
	HMENU	EditMenu=CreatePopupMenu();   
	POINT	position;

	AppendMenu (EditMenu,MF_ENABLED|MF_STRING,IDM_SETFONT,"Set font");
	AppendMenu (EditMenu,MF_ENABLED|MF_STRING,IDM_SETTXBKCOLOR,"Set background color");
	AppendMenu (EditMenu,MF_ENABLED|MF_STRING,65003,"Cancel");
	VCTextLevel = type;
	GetCursorPos (&position);
	InTrackMenu = TRUE;
	TrackPopupMenu (EditMenu,TPM_LEFTBUTTON|TPM_CENTERALIGN|TPM_VCENTERALIGN,position.x,position.y,0,hWnd,0);
	InTrackMenu = FALSE;
	DestroyMenu (EditMenu); 
	return;
}

int GetHandleLockCount (HANDLE hMem)
{
	int rtn;
	UINT	gf = GlobalFlags (hMem);

	rtn = gf & GMEM_LOCKCOUNT;

	return rtn;
}

BOOL ProcessCMDMenuCmd (UINT CntlID)
{
	 BOOL rtn = RunGFCommandFromFileAtLoc (pCMDHeader->CMDFile,pCMDHeader->FileLoc[CntlID],TRUE,0);

	 return rtn;
}

int SwitchTabbedMenu (HWND hWnd,int ipage)
{
	int	rtn = 0;
	HANDLE	hWMH = GetMenuWindowHandle (hWnd);
	

	if (hWMH)
	{
		LPWINDOWMENUHEADER pWMH = GlobalLock (hWMH);

		pWMH->currentMenu = ipage;
		pWMH->HaveTrackMouseEvent = FALSE;
		DestroySavedScreen (&pWMH->hSaveScreen,0);
		rtn = 1;
		SetScrollPos (pWMH->hWndDisplay,SB_VERT,0,TRUE);
  		InvalidateRect (pWMH->hWndMenu,0,TRUE);
  		InvalidateRect (pWMH->hWndDisplay,0,TRUE);
		GlobalUnlock (hWMH);
	}
	return rtn;
}


BOOL FloatVisMenu (HWND hWnd)
{
	HANDLE hWMH = (HANDLE)GetWindowLong (hWnd,GWL_USERDATA);

	if (hWMH)
	{
		LPWINDOWMENUHEADER	pWMH = (LPWINDOWMENUHEADER)GlobalLock (hWMH);
		pWMH->Float = TRUE;
		pWMH->isDocked = 0;
		GlobalUnlock (hWMH);
		PostMessage (hWnd,WM_CLOSE,1,(LPARAM)hWMH);
		return TRUE;
	}
	return FALSE;
}

void SetVisibility (int symnum,int status)
{
	int curstatus = GetVisibility (symnum);

	if (status)
	{
		if (!curstatus)
			ToggleVisibility (symnum);
	}
	else
	{
		if (curstatus)
			ToggleVisibility (symnum);
	}
	return;
}

void SetLayerVis (int nSyms,HANDLE hSyms,int status)
{
	LPVCSYMBOL	pSyms;
	int			i;

	if (!nSyms)
		return;
	pSyms = GlobalLock (hSyms);
	for (i=1;i<nSyms;i++)
		SetVisibility (pSyms[i].symnum,status);
	GlobalUnlock (hSyms);
	return;
}

void SortLayerSymbols (int nSyms,HANDLE hSyms,LPHANDLE phSymOrder)
{
	LPWORD	pOrder;
	WORD	i, j, k, nSorted=1;
	LPVCSYMBOL	pSyms;

	if (!nSyms)
		return;
	pSyms = GlobalLock (hSyms);
	*phSymOrder = GSSiGlobAlloc (1682,GMEM_MOVEABLE,nSyms*sizeof(WORD));
	pOrder = GlobalLock (*phSymOrder);
	pOrder[0] = 0;

	for (i=1;i<nSyms;i++)
	{
		for (j=0;j<nSorted;j++)
		{
			if (stricmp (pSyms[i].Name,pSyms[pOrder[j]].Name) < 0)
			{
				for (k=nSorted;k>j;k--)
					pOrder[k] = pOrder[k-1];
				pOrder[j] = i;
				goto Inserted;
			}
		}
		pOrder[nSorted] = i;
Inserted:
		nSorted++;
	}
	GlobalUnlock (*phSymOrder);
	GlobalUnlock (hSyms);
	return;
}

int GetFromLayer(int symnum, int layer)
{
	LPVCSYMBOL	pSyms;
	int	i;

	if (layer != pVCHeader->nLayers)
		return layer;
	pSyms = (LPVCSYMBOL)GlobalLock (pVCHeader->hLayerSyms[pVCHeader->nLayers]);
	for (i=0;i<pVCHeader->nLayerSyms[pVCHeader->nLayers];i++)
	{
		if (pSyms[i].symnum == symnum)
		{
			layer = pSyms[i].layer;
			break;
		}
	}
	GlobalUnlock (pVCHeader->hLayerSyms[pVCHeader->nLayers]);
	return layer;
}

int AddSymbolToLayer (int layer,int symnum,int type,int level,LPINT pnSyms,LPHANDLE phSyms)
{
	LPVCSYMBOL	pSyms;
	int			parSymNum, isym, ii;

	if (symnum == 2420)
		ii=1; //TUNNELORINTERCEPTOR has wrong symnum in storm directroy
	if (type == 1 && pnSyms != &pVCHeader->nLayerSyms[pVCHeader->nLayers])
		AddSymbolToLayer (layer,symnum,1,level,&pVCHeader->nLayerSyms[pVCHeader->nLayers],&pVCHeader->hLayerSyms[pVCHeader->nLayers]);
	if ((parSymNum = GetDictSymParent (symnum)))
	{
		if (parSymNum != symnumALL)
			level = AddSymbolToLayer (layer,parSymNum,2,level+1,pnSyms,phSyms);
		else
			parSymNum = 0;
	}
	if (*pnSyms)
	{
		pSyms = (LPVCSYMBOL)GlobalLock (*phSyms);
		for (isym = 0;isym<*pnSyms;isym++)
		{
			if (pSyms[isym].symnum == symnum)
			{
				level = pSyms[isym].level;
				GlobalUnlock (*phSyms);
				return level;
			}
		}
		GlobalUnlock (*phSyms);
		*phSyms = GSSiGlobalReAlloc (1681,*phSyms,(*pnSyms+1)*sizeof(VCSYMBOL),GMEM_ZEROINIT);
	}
	else
		*phSyms = GSSiGlobAlloc (1704,GHND,sizeof(VCSYMBOL));

	pSyms = GlobalLock (*phSyms);
	pSyms[*pnSyms].level = level;
	pSyms[*pnSyms].type = type;
	pSyms[*pnSyms].parent = parSymNum;
	GetDictSymName (symnum,pSyms[*pnSyms].Name);
	if (type == 2)
		pSyms[*pnSyms].expand = 2;
	pSyms[*pnSyms].layer = layer;
	pSyms[(*pnSyms)++].symnum = symnum;
	GlobalUnlock (*phSyms);
	return level+1;
}

int GetModifier (int viewport,int layer,int symnum,int modnum,LPSTR *pParms)
{
	LPSTR	pMods, pLoc;
	char	ModHeader[32];
	int		type=0;

	if (!pVCHeader->hModifiers)
		return FALSE;

	pMods = GlobalLock (pVCHeader->hModifiers);	
	sprintf (ModHeader,"!%i|%i|%i|%i|",viewport,layer,symnum,modnum);
	if ((pLoc = strstr (pMods,ModHeader)))
	{
		pLoc += strlen (ModHeader);
		type = atoi (pLoc);
		if (pParms)
			*pParms = strchr (pLoc,'|') + 1;
	}
	GlobalUnlock (pVCHeader->hModifiers);	
	return type;
}

BOOL AddModifier (int viewport,int layer,int symnum,int type)
{
	LPSTR	pMods;
	int		modnum=0;

	while (GetModifier (viewport,layer,symnum,modnum,0))
		modnum++;

	if (!pVCHeader->hModifiers)
		pVCHeader->hModifiers = GSSiGlobAlloc (1686,GHND,MAXMODSLEN);
	
	pMods = GlobalLock (pVCHeader->hModifiers);	

	sprintf (strchr (pMods,0),"!%i|%i|%i|%i|%i|",viewport,layer,symnum,modnum,type);
	switch (type)
	{
	case IDM_INTENSITYMOD:
		break;
	}
	GlobalUnlock (pVCHeader->hModifiers);	
	pVCHeader->nModifiers++;
	return TRUE;
}

BOOL DisplayModifier (HDC hDC,int viewport,int layer,int symnum,int modnum,int type, int x,LPINT py,LPSTR pParms)
{
	HBITMAP	hBmp;
	HDIB	hDIB;
	char	BMName[32];
    RECT	Rect, Rect2, rect;
	HFONT	hFont,hOldFont;
	char	str[256];

	Rect.top = *py + 2;
	Rect.left = x;
	Rect.right = x + buttonheight;
	Rect.bottom = Rect.top + buttonheight;

	switch (type)
	{
	case IDM_INTENSITYMOD:
		strcpy (BMName,"LIGHTBULB");
		break;
	case IDM_COLORMOD:
		strcpy (BMName,"CRAYONS");
		break;
	case IDM_SIZEMOD:
		strcpy (BMName,"MEASURINGTAPE");
		break;
	case IDM_RANDOMMOD:
		strcpy (BMName,"DICE");
		break;
	}

    if ((hBmp = LoadBitmap (hInst,BMName)))
    {
		hDIB = BitmapToDIB (hBmp, 0,0);
		DeleteObject (hBmp);
		DisplayBMInRect2 (hDC,hDIB, Rect,0,0,0,0);
		DestroyDIB (hDIB); 
	 }
	switch (type)
	{
	case IDM_INTENSITYMOD:
		hFont  = CreateFont(-IDNINT((2*buttonheight)/4+TypeFontSizeInc[5]*pWMH->Factor), 0, 0, 0, TypeFontWidth[5],0, TypeFontUnderline[5], 0, 0, 0, 0, 0, 0,TypeFontName[5]);   
		hOldFont = SelectObject (hDC,hFont);
		SetTextColor (hDC,TypeColors[5]);
		Rect2 = Rect;
		Rect2.left = Rect.right + 5;
		Rect2.right = Rect2.left + 200;
		strcpy (str,"25% gray");
		DrawText(hDC, str, -1, &Rect2, DT_WORDBREAK | DT_LEFT | DT_VCENTER |DT_SINGLELINE |DT_NOCLIP |DT_CALCRECT );
		DrawText(hDC, str, -1, &Rect2, DT_WORDBREAK | DT_LEFT | DT_VCENTER |DT_SINGLELINE |DT_NOCLIP );
		UnionRect (&rect,&Rect,&Rect2);
		AddVCRect (&rect,IDM_INTENSITYMOD,viewport,layer,symnum,7,FALSE);
		SelectObject (hDC,hOldFont);
		DeleteObject (hFont);
		break;
	case IDM_COLORMOD:
		 
		break;
	case IDM_SIZEMOD:
		 
		break;
	case IDM_RANDOMMOD:
		 
		break;
	}
	(*py) += buttonheight + 4;
	return TRUE;
}

void AddVCRect (LPRECT prect,int level,int viewport,int layer,int symnum,int recttype,BOOL AllowDoubleClick)
{
	LPVCRECTANGLE	pVCRect;

	pWMH->MaxWidth = max (pWMH->MaxWidth,prect->right);
	if (pVCHeader->nRectangles)
		pVCHeader->hRectangles = GSSiGlobalReAlloc (1705,pVCHeader->hRectangles,(pVCHeader->nRectangles+1)*sizeof(VCRECTANGLE),GMEM_ZEROINIT);
	else
		pVCHeader->hRectangles = GSSiGlobAlloc (1706,GHND,sizeof(VCRECTANGLE));
	pVCRect = GlobalLock (pVCHeader->hRectangles);
	pVCRect[pVCHeader->nRectangles].rect = *prect;
	InflateRect (&pVCRect[pVCHeader->nRectangles].rect,1,1);
	pVCRect[pVCHeader->nRectangles].level = level;
	pVCRect[pVCHeader->nRectangles].viewport = viewport;
	pVCRect[pVCHeader->nRectangles].layer = layer;
	pVCRect[pVCHeader->nRectangles].fromlayer = GetFromLayer(symnum,layer);
	pVCRect[pVCHeader->nRectangles].AllowDoubleClick = AllowDoubleClick;
	pVCRect[pVCHeader->nRectangles].symnum = symnum;
	pVCRect[pVCHeader->nRectangles++].type = recttype;
	GlobalUnlock (pVCHeader->hRectangles);
	return;
}
void AddAVRect (LPRECT prect,int iAV,BOOL AllowDoubleClick)
{
	LPAVRECTANGLE	pAVRect;

	if (pAVHeader->nRectangles)
		pAVHeader->hRectangles = GSSiGlobalReAlloc (1707,pAVHeader->hRectangles,(pAVHeader->nRectangles+1)*sizeof(VCRECTANGLE),GMEM_ZEROINIT);
	else
		pAVHeader->hRectangles = GSSiGlobAlloc (1708,GHND,sizeof(VCRECTANGLE));
	pAVRect = GlobalLock (pAVHeader->hRectangles);
	pAVRect[pAVHeader->nRectangles].rect = *prect;
	InflateRect (&pAVRect[pAVHeader->nRectangles].rect,1,1);
	pAVRect[pAVHeader->nRectangles].iAV = iAV;
	pAVRect[pAVHeader->nRectangles++].AllowDoubleClick = AllowDoubleClick;
	GlobalUnlock (pAVHeader->hRectangles);
	return;
}
void AddCMDRect (LPRECT prect,int iCMD,BOOL AllowDoubleClick)
{
	LPCMDRECTANGLE	pCMDRect;

	if (pCMDHeader->nRectangles)
		pCMDHeader->hRectangles = GSSiGlobalReAlloc (1707,pCMDHeader->hRectangles,(pCMDHeader->nRectangles+1)*sizeof(CMDRECTANGLE),GMEM_ZEROINIT);
	else
		pCMDHeader->hRectangles = GSSiGlobAlloc (1708,GHND,sizeof(CMDRECTANGLE));
	pCMDRect = GlobalLock (pCMDHeader->hRectangles);
	pCMDRect[pCMDHeader->nRectangles].rect = *prect;
	InflateRect (&pCMDRect[pCMDHeader->nRectangles].rect,1,1);
	pCMDRect[pCMDHeader->nRectangles].iCMD = iCMD;
	pCMDRect[pCMDHeader->nRectangles++].AllowDoubleClick = AllowDoubleClick;
	GlobalUnlock (pCMDHeader->hRectangles);
	return;
}

int GetMaxVCRectRight (void)
{
	int	right=0;
	UINT	i;

	if (pVCHeader->nRectangles)
	{
		LPVCRECTANGLE	pVCRect = GlobalLock (pVCHeader->hRectangles);

		for (i=0;i<pVCHeader->nRectangles;i++)
			right = max (right,pVCRect[i].rect.right);
		GlobalUnlock (pVCHeader->hRectangles);
	}
	return right;
}
int GetMaxAVRectRight (void)
{
	int	right=0;
	UINT	i;

	if (pAVHeader->nRectangles)
	{
		LPAVRECTANGLE	pAVRect = GlobalLock (pAVHeader->hRectangles);

		for (i=0;i<pAVHeader->nRectangles;i++)
			right = max (right,pAVRect[i].rect.right);
		GlobalUnlock (pAVHeader->hRectangles);
	}
	return right;
}
int GetMaxCMDRectRight (void)
{
	int	right=0;
	UINT	i;

	if (pCMDHeader->nRectangles)
	{
		LPCMDRECTANGLE	pCMDRect = GlobalLock (pCMDHeader->hRectangles);

		for (i=0;i<pCMDHeader->nRectangles;i++)
			right = max (right,pCMDRect[i].rect.right);
		GlobalUnlock (pCMDHeader->hRectangles);
	}
	return right;
}

int GetMaxRectRight (void)
{
	int	right=0;
	int	rtn=0;

	switch (pWMH->menuType[pWMH->currentMenu])
	{
		case MT_VISIBLE:
		case MT_ALLSYMBOLS:
		case MT_VISSYMBOLS:
			ii=1;
		case MT_CATALOG:
		case MT_PICKABLE:
			if (pWMH->menuHandle[pWMH->currentMenu])
			{
				pVCHeader = (LPVISCONTROLHEADER)GlobalLock (pWMH->menuHandle[pWMH->currentMenu]);
				rtn = GetMaxVCRectRight ();
				GlobalUnlock (pWMH->menuHandle[pWMH->currentMenu]);
			}
			break;
		case MT_AUTOVIS:
			if (pWMH->menuHandle[pWMH->currentMenu])
			{
				pAVHeader = (LPAUTOVISHEADER)GlobalLock (pWMH->menuHandle[pWMH->currentMenu]);
				rtn = GetMaxAVRectRight ();
				GlobalUnlock (pWMH->menuHandle[pWMH->currentMenu]);
			}
			break;
		case MT_CMDMENU:
			if (pWMH->menuHandle[pWMH->currentMenu])
			{
				pCMDHeader = (LPCMDMENUHEADER)GlobalLock (pWMH->menuHandle[pWMH->currentMenu]);
				rtn = GetMaxCMDRectRight ();
				GlobalUnlock (pWMH->menuHandle[pWMH->currentMenu]);
			}
			break;
	}
	return rtn;
}

int GetMaxVCRectBottom (void)
{
	int	bottom=0;
	UINT	i;

	if (pVCHeader->nRectangles)
	{
		LPVCRECTANGLE	pVCRect = GlobalLock (pVCHeader->hRectangles);

		for (i=0;i<pVCHeader->nRectangles;i++)
			bottom = max (bottom,pVCRect[i].rect.bottom);
		GlobalUnlock (pVCHeader->hRectangles);
	}
	return bottom;
}
int GetMaxCMDRectBottom (void)
{
	int	bottom=0;
	UINT	i;

	if (pCMDHeader->nRectangles)
	{
		LPCMDRECTANGLE	pVCRect = GlobalLock (pCMDHeader->hRectangles);

		for (i=0;i<pCMDHeader->nRectangles;i++)
			bottom = max (bottom,pVCRect[i].rect.bottom);
		GlobalUnlock (pCMDHeader->hRectangles);
	}
	return bottom;
}

int GetMaxVCRectTop (void)
{
	int	top=0;
	UINT	i;

	if (pVCHeader->nRectangles)
	{
		LPVCRECTANGLE	pVCRect = GlobalLock (pVCHeader->hRectangles);

		for (i=0;i<pVCHeader->nRectangles;i++)
			top = min (top,pVCRect[i].rect.top);
		GlobalUnlock (pVCHeader->hRectangles);
	}
	return top;
}
int GetMaxAVRectBottom (void)
{
	int	bottom=0;
	UINT	i;

	if (pAVHeader->nRectangles)
	{
		LPAVRECTANGLE	pAVRect = GlobalLock (pAVHeader->hRectangles);

		for (i=0;i<pAVHeader->nRectangles;i++)
			bottom = max (bottom,pAVRect[i].rect.bottom);
		GlobalUnlock (pAVHeader->hRectangles);
	}
	return bottom;
}

void ClickExpandBox (int nclicks,int layer,int level,int symnum,int status)
{
	if (pVCHeader->nLayerSyms[layer])
	{
		LPVCSYMBOL	pSym = (LPVCSYMBOL)GlobalLock (pVCHeader->hLayerSyms[layer]);
		int	i,j;

		if (nclicks == 1)
		{
			for (i=0;i<pVCHeader->nLayerSyms[layer];i++,pSym++)
			{
				if (pSym->symnum == symnum)
				{
					if (pSym->expand == 1)
						pSym->expand = 2;
					else
						pSym->expand = 1;
					break;
				}
			}
		}
		else
		{
			switch (level)
			{
			case 1:
				for (j=0;j<pVCHeader->nLayerSyms[layer];j++)
				{
					if (pSym[j].parent == 0)
						ClickExpandBox (nclicks,layer,2,pSym[j].symnum,status);
				}
				break;
			case 2:
			case 3:
				for (i=0;i<pVCHeader->nLayerSyms[layer];i++)
				{
					if (pSym[i].symnum == symnum)
					{
						if (pSym[i].type == 1)
							pSym[i].expand = FALSE;
						else
						{
							pSym[i].expand = status;
							for (j=0;j<pVCHeader->nLayerSyms[layer];j++)
							{
								if (pSym[j].parent == symnum)
									ClickExpandBox (nclicks,layer,level,pSym[j].symnum,status);
							}
						}
						break;
					}
				}
				break;
			}
		}
		GlobalUnlock (pVCHeader->hLayerSyms[layer]);
	}
	return;
}

void ClickVisBox (int nclicks,int layer,int level,int symnum,int status)
{
	if (pVCHeader->nLayerSyms[layer])
	{
		LPVCSYMBOL	pSym = (LPVCSYMBOL)GlobalLock (pVCHeader->hLayerSyms[layer]);
		int	i,j;

		for (i=0;i<pVCHeader->nLayerSyms[layer];i++,pSym++)
		{
			switch (level)
			{
			case 0:
			case 1:
				SetVisibility (pSym->symnum,status);
				break;
			case 2:
				if (pSym->symnum == symnum)
				{
					if (pSym->type == 1)
						SetVisibility (symnum,status);
					else
						ClickVisBox (nclicks,layer,3,symnum,status);
				}
				break;
			case 3:
				if (pSym->parent == symnum)
					SetVisibility (pSym->symnum,status);
				break;
			}
		}
		GlobalUnlock (pVCHeader->hLayerSyms[layer]);
	}
	return;
}


HANDLE GetMenuWindowHandle (HWND hWnd)
{
	HANDLE	hWin=0;
	UINT	i;

	while (hWnd)
	{
		if (GetToolbarIDFromWnd (hWnd) > -1)
			return (HANDLE) GetWindowLong (hWnd,GWL_USERDATA);
		hWnd = GetParent (hWnd);
	}
	return hWin;
}

void DestroyFloatMenus (void)
{
	int imen, ilayer,i;

	TempSaveFloatMenuData ();
	DestroySavedScreen (&pWMH->hSaveScreen,0);
	GSSiDeleteObject (&pWMH->hFontTab);

	for (imen=0;imen<pWMH->nMenus;imen++)
	{
		if (pWMH->menuHandle[imen])
			switch (pWMH->menuType[imen])
			{
			case MT_VISIBLE:
			case MT_ALLSYMBOLS:
			case MT_VISSYMBOLS:
				pWMH->menuHandle[imen] = 0;
				break;
			case MT_CATALOG:
			case MT_PICKABLE:
				pVCHeader = (LPVISCONTROLHEADER)GlobalLock (pWMH->menuHandle[imen]);
				for (ilayer=0;ilayer<=pVCHeader->nLayers;ilayer++)
				{
					GSSiGlobFree (&pVCHeader->hLayerSyms[ilayer]);
					GSSiGlobFree (&pVCHeader->hSymOrder[ilayer]);
				}
				GSSiGlobFree (&pVCHeader->hRectangles);
				GSSiGlobFree (&pVCHeader->hModifiers);
				GSSiGlobUlFree (&pWMH->menuHandle[imen]);
				break;
			case MT_AUTOVIS:
				pAVHeader = (LPAUTOVISHEADER)GlobalLock (pWMH->menuHandle[imen]);
				GSSiGlobFree (&pAVHeader->hRectangles);
				GSSiGlobUlFree (&pWMH->menuHandle[imen]);
				break;
			case MT_CMDMENU:
				pCMDHeader = (LPCMDMENUHEADER)GlobalLock (pWMH->menuHandle[imen]);
				for (i=0;i<pCMDHeader->nCMD;i++)
					GSSiDeleteObject (&pCMDHeader->hBM[i]);

				GSSiGlobFree (&pCMDHeader->hRectangles);
				GSSiGlobUlFree (&pWMH->menuHandle[imen]);
				break;
			}
	}
	return;
}

void DisplayFloatMenu (BOOL CheckCursorPos)
{
	HDC	hDC;
	RECT	MainRect,MenuRectInMain,MenuClientRect,MenuWindowRect;
	LPVIEWPORT	SaveVP = CurView;
	int	xoff, yoff,ii;
	POINT	pt;

	if (!pWMH)
		return;
	//InvalidateRect (pWMH->hWndDisplay,0,TRUE);
	hDC = GetDC (pWMH->hWndDisplay);
//	hDC = GetWindowDC (pWMH->hWndMenu);
	SetGraphicsMode(hDC, GM_COMPATIBLE);
    SetMapMode    ( hDC, MM_ISOTROPIC );
	SetWindowOrgEx  ( hDC, 0, 0,0 );
	SetViewportOrgEx( hDC, 0, 0,0 );    
    SetWindowExtEx  ( hDC, 1024, 1024,0 ); 
	SetViewportExtEx( hDC, 1024, 1024,0 );  
	SelectClipRgn ( hDC,0);
	GetClientRect (pWMH->hWndDisplay,&MenuClientRect);
	GetWindowRect (pWMH->hWndMenu,&MenuWindowRect);
	RestoreScreen2 (hDC, pWMH->hSaveScreen,0,FALSE);
	GetWindowRect (hWndMain,&MainRect);
	GetCursorPos (&pt);
//	ScreenToClient (pWMH->hWndDisplay,&pt);
	if (!PtInRect (&MenuWindowRect,pt))
		pWMH->HaveTrackMouseEvent = FALSE;
	if (!CheckCursorPos || !PtInRect (&MenuWindowRect,pt))
	{
		GetWindowRect (pWMH->hWndDisplay,&MenuRectInMain);
//		GetWindowRect (pWMH->hWndMenu,&MenuRectInMain);
		ScreenRectToClientRect (hWndMain,&MenuRectInMain);
		xoff = MenuRectInMain.left;
		yoff = MenuRectInMain.top;
		if ((pWMH->Float && BufferedScreen && hDCScreenBuffer) && RectCompletelyInRect(&CurView->ScreenRect,&MenuRectInMain))
		{
			int	widthMenu=RECTWIDTH (&MenuClientRect), heightMenu=RECTHEIGHT (&MenuClientRect);
			BLENDFUNCTION bf;
       
			bf.BlendOp = AC_SRC_OVER;
			bf.BlendFlags = 0;
			bf.AlphaFormat = 0;
			bf.SourceConstantAlpha = pWMH->AlphaBlendFactor;
			ii=AlphaBlend(hDC,0,0,widthMenu,heightMenu, 
						hDCScreenBuffer,xoff,yoff,widthMenu,heightMenu,bf);
		}
	}
//	ReleaseDC (pWMH->hWndDisplay,hDC);
	ReleaseDC (pWMH->hWndMenu,hDC);
	CurView = SaveVP;
	return;
}


LONG FAR PASCAL FloatMenuWndProc(HWND hWnd, int Message, WPARAM wParam, LONG lParam)
{
	BOOL	rtn, ii;
	UINT	ilayer;
	BOOL	Unlock;
	UINT	i,j;
	UINT	imen;
	HANDLE	savehWMH,hWMH2;
	LPWINDOWMENUHEADER savepWMH;

extern POINT	ToolBarStartPoint;
	savehWMH = hWMH;
	savepWMH = pWMH;

	if (InTrackMenu)
		return DefWindowProc(hWnd, Message, wParam, lParam);
	switch (Message)
   {
		case GSSI_REINITDIALOG:
		{
			POINT	pt=ToolBarStartPoint;
			RECT	rect;
			HWND	hWndMenu;

			ToolbarID = GetToolbarIDFromWnd (hWnd);
			if (ToolbarID < 0)
				break;
			ClientToScreen (hWndMain,(LPPOINT)&pt);
//			ScreenToClient (hWndMain,(LPPOINT)&pt);
/*			GetWindowRect (hWnd,&rect);
			ii = MoveWindow(hWnd,pt.x,pt.y,
							rect.right-rect.left,rect.bottom-rect.top,TRUE);
			SetToolbarConfig (ToolbarID,0);*/
			hWMH = GetMenuWindowHandle (hWnd);
			pWMH = GlobalLock (hWMH);
			pWMH->Float = lParam;
			pWMH->ScreenStartPoint = pt;
			DestroySavedScreen (&pWMH->hSaveScreen,0);
			hWndMenu = pWMH->hWndMenu;
			GlobalUnlock (hWMH);
			PostMessage (hWndMenu,WM_CLOSE,3,(LPARAM)hWMH);
			break;
		}
//	case WM_SYSCOMMAND:
//		return DefWindowProc(hWnd, Message, wParam, lParam);

	case WM_NOTIFY:
		 return OnWMNotify(hWnd,lParam);

	case WM_HSCROLL:
		{
			int nScrollCode = (int) LOWORD(wParam);  // scroll bar value 
			int	nPos, iPos = (short int) HIWORD(wParam);   // scroll box position 
			HWND	hwndScrollBar = (HWND) lParam;       // handle to scroll bar 
			RECT	rect;

			GetClientRect (hWnd,&rect);
			nPos = GetScrollPos (hWnd,SB_HORZ);

			switch (nScrollCode)
			{
			case SB_THUMBPOSITION:
				SetScrollPos (hWnd,SB_HORZ,iPos,TRUE);
				DestroySavedScreen (&pWMH->hSaveScreen,0);
	     		InvalidateRect (hWnd,0,TRUE);
				break;
			case SB_PAGEDOWN:
				SetScrollPos (hWnd,SB_HORZ,nPos+RECTWIDTH(&rect)/2,TRUE);
				DestroySavedScreen (&pWMH->hSaveScreen,0);
	     		InvalidateRect (hWnd,0,TRUE);
				break;
			case SB_PAGEUP:
				SetScrollPos (hWnd,SB_HORZ,nPos-RECTWIDTH(&rect)/2,TRUE);
				DestroySavedScreen (&pWMH->hSaveScreen,0);
	     		InvalidateRect (hWnd,0,TRUE);
				break;
			case SB_LINEDOWN:
				SetScrollPos (hWnd,SB_HORZ,nPos+RECTWIDTH(&rect)/10,TRUE);
				DestroySavedScreen (&pWMH->hSaveScreen,0);
	     		InvalidateRect (hWnd,0,TRUE);
				break;
			case SB_LINEUP:
				SetScrollPos (hWnd,SB_HORZ,nPos-RECTWIDTH(&rect)/10,TRUE);
				DestroySavedScreen (&pWMH->hSaveScreen,0);
	     		InvalidateRect (hWnd,0,TRUE);
				break;
			}
		}
		break;
	case WM_VSCROLL:
		{
			int nScrollCode = (int) LOWORD(wParam);  // scroll bar value 
			int	nPos,iPos = (short int) HIWORD(wParam);   // scroll box position 
			HWND	hwndScrollBar = (HWND) lParam;       // handle to scroll bar 
			RECT	rect;
			int		minx, maxx;

			GetClientRect (hWnd,&rect);
			nPos = GetScrollPos (hWnd,SB_VERT);
			GetScrollRange (hWnd,SB_VERT,&minx,&maxx);

			switch (nScrollCode)
			{
			case SB_THUMBTRACK:
				SetScrollPos (hWnd,SB_VERT,iPos,TRUE);
				DestroySavedScreen (&pWMH->hSaveScreen,0);
				fromScroll = TRUE;
	     		InvalidateRect (hWnd,0,TRUE);
				break;
			case SB_THUMBPOSITION:
				SetScrollPos (hWnd,SB_VERT,iPos,TRUE);
				DestroySavedScreen (&pWMH->hSaveScreen,0);
				fromScroll = TRUE;
	     		InvalidateRect (hWnd,0,TRUE);
				break;
			case SB_PAGEDOWN:
				SetScrollPos (hWnd,SB_VERT,nPos+RECTHEIGHT(&rect)/2,TRUE);
				DestroySavedScreen (&pWMH->hSaveScreen,0);
				fromScroll = TRUE;
	     		InvalidateRect (hWnd,0,TRUE);
				break;
			case SB_LINEDOWN:
				SetScrollPos (hWnd,SB_VERT,min(maxx,nPos+RECTHEIGHT(&rect)/10),TRUE);
				DestroySavedScreen (&pWMH->hSaveScreen,0);
				fromScroll = TRUE;
	     		InvalidateRect (hWnd,0,TRUE);
				break;
			case SB_PAGEUP:
				SetScrollPos (hWnd,SB_VERT,nPos-RECTHEIGHT(&rect)/2,TRUE);
				DestroySavedScreen (&pWMH->hSaveScreen,0);
				fromScroll = TRUE;
	     		InvalidateRect (hWnd,0,TRUE);
			case SB_LINEUP:
				SetScrollPos (hWnd,SB_VERT,nPos-RECTHEIGHT(&rect)/10,TRUE);
				DestroySavedScreen (&pWMH->hSaveScreen,0);
				fromScroll = TRUE;
	     		InvalidateRect (hWnd,0,TRUE);
				break;
			}
		}
		break;
	    case WM_NCCREATE:
		{
			
			if (CreatingParentWindow)
			{
				LPWINDOWMENUHEADER pWMH = GlobalLock (hInitData);

				CreatingParentWindow = FALSE;
				SetWindowLong (hWnd,GWL_USERDATA,(long)hInitData);
				pWMH->hWndMenu = hWnd;
				GlobalUnlock (hInitData);

				return TRUE;
			}
			else
			{
				int	isavemen;

				hWMH = GetMenuWindowHandle (hWnd);
				pWMH = GlobalLock (hWMH);
				isavemen = pWMH->currentMenu;
				pWMH->hWndDisplay =  hWnd;
				for (imen=0;imen<pWMH->nMenus;imen++)
				{
					if (!pWMH->menuHandle[imen])
					{
						pWMH->currentMenu = imen;
						switch (pWMH->menuType[imen])
						{
						case MT_CATALOG:
						case MT_PICKABLE:
							rtn = CatalogMenuWndProc (hWnd,Message, wParam, lParam);
							break;
						case MT_ALLSYMBOLS:
						case MT_VISSYMBOLS:
						case MT_VISIBLE:
							{
								int j;

								for (j=0;j<pWMH->nMenus;j++)
									if (pWMH->menuType[j] == MT_CATALOG)
										pWMH->menuHandle[imen] = pWMH->menuHandle[j];
							}
							break;
						case MT_AUTOVIS:
							rtn = AutoVisMenuWndProc (hWnd,Message, wParam, lParam);
							break;
						case MT_CMDMENU:
							rtn = CMDMenuWndProc (hWnd,Message, wParam, lParam);
							break;
						default:
							rtn = 0;
						}
					}

				}
				pWMH->currentMenu = isavemen;
				GlobalUnlock (hWMH);
				hWMH = savehWMH;
				pWMH = savepWMH;

				//return rtn;
				return DefWindowProc(hWnd, Message, wParam, lParam);
			}
		}
		break;
		case WM_DESTROY:
			ii=1;
			return DefWindowProc(hWnd, Message, wParam, lParam);
		case WM_NCDESTROY:
			hWMH = (HANDLE)GetWindowLong (hWnd,GWL_USERDATA);
			if (hWMH)
			{
				int	nl;

				pWMH = GlobalLock (hWMH);
				ToolbarID = GetToolbarIDFromWnd (pWMH->hWndMenu);
				DestroyToolbar (ToolbarID);
				RemoveToolbar (ToolbarID,TRUE);
				DestroyFloatMenus ();
				nl = GetHandleLockCount (hWMH);
				while (nl--)
					GlobalUnlock (hWMH);
				GSSiGlobFree (&hWMH);
				pWMH = 0;
				SetWindowLong (hWnd,GWL_USERDATA,0);
			}
				hWMH = savehWMH;
				pWMH = savepWMH;
			return DefWindowProc(hWnd, Message, wParam, lParam);
		break;

		case WM_PAINT:
			if ((hWMH = GetMenuWindowHandle (hWnd)))
			{
				pWMH = (LPWINDOWMENUHEADER)GlobalLock (hWMH);

				if (pWMH->hSaveScreen)
				{
					if (hWnd == pWMH->hWndDisplay)
					{
						PAINTSTRUCT	ps;
						HDC	hDC;

						memset(&ps, 0x00, sizeof(PAINTSTRUCT));
						hDC = BeginPaint(hWnd, &ps);
						if (!GetCapture ())
							DisplayFloatMenu (TRUE);
						EndPaint (hWnd,&ps);
						GlobalUnlock (hWMH);
						hWMH = savehWMH;
						pWMH = savepWMH;
						return 0;
					}
					else
						InvalidateRect (pWMH->hWndTab,0,TRUE);
				}
				GlobalUnlock (hWMH);
			}

		break;
		case WM_ERASEBKGND:
			if (GetCapture() && GetToolbarIDFromWnd(GetCapture()) >= 0)
				return 0;
			if ((hWMH = GetMenuWindowHandle (hWnd)))
			{
				pWMH = (LPWINDOWMENUHEADER)GlobalLock (hWMH);

				if (hWnd == pWMH->hWndMenu)
				{
					RECT	rect;
					HDC	hDC = GetWindowDC (hWnd);
					int	w,h;

					GetWindowRect (hWnd,&rect);
					w = RECTWIDTH (&rect);
					h = RECTHEIGHT (&rect);
					rect.left = rect.top = 0;
					rect.right = w;
					rect.bottom = h;
					FillRectPoly (hDC,&rect,RGB(227,227,227));
					FrameRect (hDC,&rect,GetStockObject (DKGRAY_BRUSH));
					ReleaseDC (hWnd,hDC);
				}
				else
				{
					RECT	Rect;
					HDC	hDC = GetDC (hWnd);

					GetClientRect (hWnd,&Rect);
					FillRectPoly (hDC,&Rect,RGB(255,255,255));
					ReleaseDC (hWnd,hDC);
				}
				GlobalUnlock (hWMH);
			}
			break;
		case WM_CLOSE:
			if ((hWMH = GetMenuWindowHandle (hWnd)))
			{
				pWMH = (LPWINDOWMENUHEADER)GlobalLock (hWMH);

				{
					RECT	rect;
					char	VPConfigAndName[40]="";
					char	appliesToVP[40];
					BOOL	Err;
					int		nl;
					HWND	hWndDestroy;
					BOOL	Float = pWMH->Float;
					double	fac = pWMH->Factor;

					DestroySavedScreen (&pWMH->hSaveScreen,0);
					strcpy (appliesToVP,pWMH->appliesToVP);
					if (wParam)
					{
						strcpy (VPConfigAndName,"%RECT%");
						GetWindowRect (pWMH->hWndMenu,&rect);
						if (wParam == 1)
							rect.right = max (rect.right,rect.left+GetMaxRectRight ()+32);
					}
					else
					{
						strcpy (VPConfigAndName,"%RECT%");
						rect = pWMH->FixedRect;
					}
					nl = GetHandleLockCount (pWMH->menuHandle[pWMH->currentMenu]);
					while (nl--)
						GlobalUnlock (pWMH->menuHandle[pWMH->currentMenu]);
					//if (lParam)
					{
						if (hWnd == pWMH->hWndMenu)
						{
							ToolbarID = GetToolbarIDFromWnd (hWnd);
							pWMH->isDocked = GetToolbarDockingStatus (ToolbarID,&pWMH->dockWidth);
							DestroyToolbar (ToolbarID);
							RemoveToolbar (ToolbarID,lParam==0);
							SetWindowLong (pWMH->hWndMenu,GWL_USERDATA,0);
							SetWindowLong (pWMH->hWndDisplay,GWL_USERDATA,0);
						}
					}
					if (!lParam)
						DestroyFloatMenus ();
					nl = GetHandleLockCount (hWMH);
					hWndDestroy = pWMH->hWndMenu;
					if (pWMH->pVP)
						pWMH->pVP->hWndDlg = 0;
					while (nl--)
						GlobalUnlock (hWMH);
					if (!lParam)
						GSSiGlobFree (&hWMH);
					pWMH = 0;
					DestroyWindow (hWndDestroy);
					SetConfig (1);
					SetCurView (SetVPFromName (appliesToVP,&Err));
					if (lParam)
					{
						VisibilityControl (CurView->hWnd,hInst,VPConfigAndName,&rect,fac,(HANDLE)lParam,FALSE,!Float,wParam!=3);
						if (Float)
						{
							//ShowWindow (pWMH->hWndMenu,SW_SHOW);
							//cwCenter(pWMH->hWndMenu, 0);
						}
					}
					else
						AdjustToolbarPositions ();
				}
			}
		hWMH = savehWMH;
		pWMH = savepWMH;
		return DefWindowProc(hWnd, Message, wParam, lParam);
	}

	hWMH = GetMenuWindowHandle (hWnd);
	if (!hWMH)
	{
		hWMH = savehWMH;
		pWMH = savepWMH;
		return DefWindowProc(hWnd, Message, wParam, lParam);
	}
	pWMH = (LPWINDOWMENUHEADER)GlobalLock (hWMH);

	switch (Message)
	{
		case 161:
				hWMH = savehWMH;
				pWMH = savepWMH;
			return DefWindowProc(hWnd, Message, wParam, lParam);
			break;

		case WM_MOUSEMOVE:

		if (hWnd == pWMH->hWndDisplay)
		{
			if (!pWMH->HaveTrackMouseEvent)
			{
				TRACKMOUSEEVENT EventTrack;
				HDC hDC = GetDC (pWMH->hWndDisplay);

				RestoreScreen2 (hDC, pWMH->hSaveScreen,0,FALSE);
				ReleaseDC (pWMH->hWndDisplay,hDC);
				EventTrack.dwFlags = TME_LEAVE;
				EventTrack.cbSize = sizeof(TRACKMOUSEEVENT);
				EventTrack.hwndTrack = hWnd;
				EventTrack.dwHoverTime = 0;
				TrackMouseEvent(&EventTrack);
				pWMH->HaveTrackMouseEvent = TRUE;
			}
		}
		break;

		case WM_EXITSIZEMOVE:
			{
				POINT	pt;

				AdjustToolbarWindowRect (hWnd);
				GetCursorPos (&pt);
				//DisplayAllToolbars (FALSE);
				ToolbarID = GetToolbarIDFromWnd (hWnd);
				SetToolbarConfig (ToolbarID,0);
				SeeIfToolbarShouldBeDocked (hWnd,&pt);
			}
			break;

		case WM_MOVING:
//			DisplayAllToolbars (FALSE);
			break;

		case WM_MOUSELEAVE:
			pWMH->HaveTrackMouseEvent =FALSE;
			if (pWMH->Float)
				DisplayFloatMenu (FALSE);
		break;

		case WM_COMMAND:
		{         
			 switch(LOWORD(wParam))
    		{
			 case IDM_SETFACTOR:
				 {
					 char newfac[64];

					 sprintf (newfac,"$GETVAL(Enter new size factor:,%f)",pWMH->Factor);


					 ExpandText (newfac);
					 pWMH->Factor = atof (newfac);
	     			 InvalidateRect (hWnd,0,TRUE);
					 PostMessage (pWMH->hWndMenu,WM_CLOSE,1,(LPARAM)hWMH);
					 rtn = TRUE;//DefWindowProc(hWnd, Message, wParam, lParam);
					 break;
				 }
			 case IDM_SETALPHABLEND:
				 {
					 char newfac[64];

					 sprintf (newfac,"$GETVAL(Enter new alpha blend factor(0 to 255):,%i)",pWMH->AlphaBlendFactor);

					 ExpandText (newfac);
					 pWMH->AlphaBlendFactor = atoi (newfac);
					 AlphaBlendFactor = pWMH->AlphaBlendFactor;
	     			 InvalidateRect (hWnd,0,TRUE);
					 PostMessage (pWMH->hWndMenu,WM_CLOSE,1,(LPARAM)hWMH);
					 rtn = TRUE;//DefWindowProc(hWnd, Message, wParam, lParam);
					 break;
				 }
				break;
			 case IDM_SETBUTTONH:
				 {
					 char newfac[64];

					 sprintf (newfac,"$GETVAL(Enter new button height:,%i)",ButtonHeight);

					 ExpandText (newfac);
					 ButtonHeight = min (40,max (10,atoi (newfac)));
	     			 InvalidateRect (hWnd,0,TRUE);
					 PostMessage (pWMH->hWndMenu,WM_CLOSE,1,(LPARAM)hWMH);
					 rtn = TRUE;//DefWindowProc(hWnd, Message, wParam, lParam);
					 break;
				 }
				break;
//	buttonheight = ButtonHeight * pWMH->Factor;
//	buttonwidth  = ButtonWidth  * pWMH->Factor;

			 case IDM_SETBKCOLOR:
				 if (GetColor (hWnd,&pWMH->BackgroundColor))
				 {
	     			 InvalidateRect (hWnd,0,TRUE);
					 rtn = TRUE;
				 }
				 break;
			 case IDM_SETBKCOLORCMD:
				 if (GetColor (hWnd,&CommandButtonBackgroundColor))
				 {
	     			 InvalidateRect (hWnd,0,TRUE);
					 rtn = TRUE;
				 }
				 break;
			 case IDM_VERTTABS:
				 pWMH->Verticle = !pWMH->Verticle;
				 PostMessage (pWMH->hWndMenu,WM_CLOSE,1,(LPARAM)hWMH);
				 rtn = TRUE;
				 break;
			 case IDM_TABPAD:
				 {
					 char newfac[64];

					 sprintf (newfac,"$GETVAL(Enter padding value:,%i)",pWMH->xpad);

					 ExpandText (newfac);
					 pWMH->xpad = pWMH->ypad = atoi (newfac);
					PostMessage (pWMH->hWndMenu,WM_CLOSE,1,(LPARAM)hWMH);
					rtn = TRUE;
				 }
				 break;
			 case IDM_SETFONTTABS:
				 {
					 HFONT	hFont=0;
					 COLORREF	Color;

					 if (!pWMH->hFontTab)
					 {
						 HFONT hFontTab = (HFONT)SendMessage (pWMH->hWndTab,WM_GETFONT,0,0); 
						 
						 GetObject (hFontTab,sizeof(LOGFONT),&pWMH->LogFontTab);
					 }
					 if (GetFont (hWnd, &pWMH->LogFontTab, &pWMH->FontColorTab,0,&pWMH->hFontTab)) 
					 {
						 RECT	rect;
						
						 GetClientRect (pWMH->hWndMenu,&rect);
						 SendMessage (pWMH->hWndTab,WM_SETFONT,(WPARAM)pWMH->hFontTab,MAKELPARAM(TRUE, 0));    
						 TabCtrl_AdjustRect(pWMH->hWndTab, FALSE, &rect); 
					     SetWindowPos(pWMH->hWndDisplay, HWND_TOP, rect.left, rect.top, 
				                                               rect.right - rect.left, rect.bottom - rect.top, 0); 
						 rtn = TRUE;
					 }
				 }
				 break;

			 case IDM_SETFONTCMD:
				 {
					 LOGFONT	LogFont;
					 HFONT hFont = CreateFont(-IDNINT((2*buttonheight)/4+TypeFontSizeInc[4]*pWMH->Factor), 0, 0, 0, TypeFontWidth[4],0, TypeFontUnderline[4], 0, 0, 0, 0, 0, 0,TypeFontName[4]);   
					 
					 GetObject (hFont,sizeof(LOGFONT),&LogFont);
					 if (GetFont (hWnd, &LogFont, &TypeColors[4],0,&hFont)) 
					 {
	 					 GetObject (hFont,sizeof(LOGFONT),&LogFont);
						 TypeFontWidth[4] = LogFont.lfWeight;
						 TypeFontUnderline[4] = LogFont.lfUnderline;
						 strcpy (TypeFontName[4],LogFont.lfFaceName);
		     			 InvalidateRect (hWnd,0,TRUE);
					 }
					 GSSiDeleteObject (&hFont);
					 rtn = TRUE;
				 }
				 break;

			 case IDM_SETFONT:
				 {
					 LOGFONT	LogFont,LogFont2;
					 HFONT hFont = CreateFont(-IDNINT((2*buttonheight)/4+TypeFontSizeInc[VCTextLevel]*pWMH->Factor), 0, 0, 0, TypeFontWidth[VCTextLevel],0, TypeFontUnderline[VCTextLevel], 0, 0, 0, 0, 0, 0,TypeFontName[VCTextLevel]);   
					 
					 GetObject (hFont,sizeof(LOGFONT),&LogFont);
					 LogFont2 = LogFont;
					 if (GetFont (hWnd, &LogFont2, &TypeColors[VCTextLevel],0,&hFont)) 
					 {
	 					 GetObject (hFont,sizeof(LOGFONT),&LogFont2);
						 TypeFontWidth[VCTextLevel] = LogFont2.lfWeight;
						 TypeFontUnderline[VCTextLevel] = LogFont2.lfUnderline;
						 strcpy (TypeFontName[VCTextLevel],LogFont2.lfFaceName);
						 TypeFontSizeInc[VCTextLevel] += abs (LogFont2.lfHeight) - abs (LogFont.lfHeight);
		     			 InvalidateRect (hWnd,0,TRUE);
					 }
					 GSSiDeleteObject (&hFont);
					 rtn = TRUE;
				 }
				 break;
			 case IDM_SETTXBKCOLOR:
				 if (GetColor (hWnd,&TypeColorsBk[VCTextLevel]))
				 {
	     			 InvalidateRect (hWnd,0,TRUE);
					 rtn = TRUE;
				 }
				 break;

			 }

		}
	}
if (Message == WM_LBUTTONDOWN)
	ii=1;

	if (TabbedWindowProc (hWnd,Message, wParam, lParam))
		rtn = TRUE;
	else
	{
		SaveVis = CurVis;
		SaveVP = CurView;
		SaveCfg = CurrentConfig;

		switch (pWMH->menuType[pWMH->currentMenu])
		{
			case MT_VISIBLE:
			case MT_ALLSYMBOLS:
			case MT_VISSYMBOLS:
				ii=1;
			case MT_CATALOG:
			case MT_PICKABLE:
				if (pWMH->menuHandle[pWMH->currentMenu])
				{
					pVCHeader = (LPVISCONTROLHEADER)GlobalLock (pWMH->menuHandle[pWMH->currentMenu]);
					SetConfig (pVCHeader->Config);
					SetViewport (pVCHeader->VPID);
					SelectVisList (pVCHeader->Pickability);
					if (!CurView->pPickListManual && !CurView->pPickList1)
						pVCHeader->PickIsSame = TRUE;
					else
						pVCHeader->PickIsSame = FALSE;

					GlobalUnlock (pWMH->menuHandle[pWMH->currentMenu]);
				}
				{
					LPWINDOWMENUHEADER savepWMH2 = pWMH;
					HANDLE	savehWMH2=hWMH;

					rtn = CatalogMenuWndProc (hWnd,Message, wParam, lParam);
					pWMH = savepWMH2;
					hWMH = savehWMH2;
				}
				break;
			case MT_AUTOVIS:
				{
					LPWINDOWMENUHEADER savepWMH2 = pWMH;
					HANDLE	savehWMH2=hWMH;

					pAVHeader = (LPAUTOVISHEADER)GlobalLock (pWMH->menuHandle[pWMH->currentMenu]);
					rtn = AutoVisMenuWndProc (hWnd,Message, wParam, lParam);
					pWMH = savepWMH2;
					hWMH = savehWMH2;
					GlobalUnlock (pWMH->menuHandle[pWMH->currentMenu]);
				}
				break;
			case MT_CMDMENU:
				{
					LPWINDOWMENUHEADER savepWMH2 = pWMH;
					HANDLE	savehWMH2=hWMH;

					pCMDHeader = (LPCMDMENUHEADER)GlobalLock (pWMH->menuHandle[pWMH->currentMenu]);
					rtn = CMDMenuWndProc (hWnd,Message, wParam, lParam);
					pWMH = savepWMH2;
					hWMH = savehWMH2;
					GlobalUnlock (pWMH->menuHandle[pWMH->currentMenu]);
				}
				break;
			default:
				rtn = DefWindowProc(hWnd, Message, wParam, lParam);
				break;
		}
		SetConfig (SaveCfg);
		CurView = SaveVP;
		CurVis = SaveVis;
	}
	if (Message == WM_PAINT && hWnd == pWMH->hWndDisplay)
	{
		RECT	rect;
		HDC		hDC = GetDC (pWMH->hWndDisplay);

		DestroySavedScreen (&pWMH->hSaveScreen,0);
		if (GetClientRect (pWMH->hWndDisplay,&rect))
			pWMH->hSaveScreen = SaveScreen2 ((HWND)-1,hDC,rect,0,0);
		ReleaseDC (hWnd,hDC);
	}
	hWMH2 = GetMenuWindowHandle (hWnd);
	if (hWMH && hWMH == hWMH2)
		GlobalUnlock (hWMH);
	pWMH = savepWMH;
	hWMH = savehWMH;
	return rtn;
}

LONG FAR PASCAL AutoVisMenuWndProc(HWND hWnd, int Message, WPARAM wParam, LONG lParam)
{
	HDC	hDC;
	long	rtn=0;
	int	ilayer,i,nClicks=0;
	static	BOOL	HaveTimer;
	char	mess[256];
	int		status,ii;
	BOOL	nRc;
	static	HWND	hWndTab=0,hWndDisplay=0;
	RECT	TabClientRect;
	HANDLE	savehWMH;
	LPWINDOWMENUHEADER savepWMH;
	LPAUTOVISHEADER	savepAVHeader;

	if (InTrackMenu)
		return DefWindowProc(hWnd, Message, wParam, lParam);

    if (Message == WM_NCCREATE)
	{
		char	TempFile[MAX_PATH];
		int		layer, symnum, ilayer;
		HFILE	Fid;
		char	str[300],str2[512];
		
		hInitData = GetMenuWindowHandle (hWnd);
		HaveTimer = FALSE;
		HaveMoving = FALSE;
		pWMH = (LPWINDOWMENUHEADER)GlobalLock (hInitData);
		
		if (!pWMH->menuHandle[pWMH->currentMenu])
		{
			LPSTR	pBS;
			long	loc;
			int		n=0;

			pWMH->menuHandle[pWMH->currentMenu]=GSSiGlobAlloc (1687,GHND,sizeof(AUTOVISHEADER));
			pAVHeader = (LPAUTOVISHEADER)GlobalLock (pWMH->menuHandle[pWMH->currentMenu]);
			strcpy (str,CurView->VisName);
			if ((pBS = strrchr (str,'.')))
        		*pBS = 0;
			if ((pBS = strrchr (str,'\\')))
				SetDlgItemText (hWnd,IDC_LISTHEADER,pBS+1);
			Fid = OpenTempNamedFile (); 
			loc = GSSillseek (Fid,0,1);  
			GetGlobalCVal ("[%AVDIR]",str,"[%DL]autovisibility");
			strcpy (pAVHeader->AVDir,str);
			SearchFilesInDir (str,".txt", Fid,&n,"*.txt",-1,TRUE,TRUE); 
			GSSillseek (Fid,loc,0);
			while (fgetstring (str,256,Fid))
			{    
				pBS = _fstrrchr (str,'\\');
				if (pBS) 
					pBS++;
				else
					pBS = str;
				_fstrcpy (str2,pBS);
				if ((pBS = _fstrrchr (str2,'.')))
					*pBS = 0;
				strcpy (pAVHeader->AVFile[pAVHeader->nAutoVis++],str2);
			}  
		  	CloseAndDeleteFile (&Fid); 
			pAVHeader->iCurrentRect = -1;
			SetConfig (SaveCfg);
			CurView = SaveVP;
			CurVis = SaveVis;
		}
		else
		{
			pAVHeader = (LPAUTOVISHEADER)GlobalLock (pWMH->menuHandle[pWMH->currentMenu]);

		}

		pWMH->Float = floating;
		if (!floating)
			pWMH->FixedRect = OrigRect;
		SetScrollRange (hWnd,SB_VERT,0,10000,FALSE);
		SetScrollRange (hWnd,SB_HORZ,0,500,FALSE);
		GlobalUnlock (pWMH->menuHandle[pWMH->currentMenu]);
		GlobalUnlock (hInitData);
		return TRUE;
		return DefWindowProc(hWnd, Message, wParam, lParam);
	}

	savehWMH = hWMH;
	savepWMH = pWMH;
	hWMH = GetMenuWindowHandle (hWnd);
	if (!hWMH)
	{
		hWMH = savehWMH;
		return DefWindowProc(hWnd, Message, wParam, lParam);
	}
	pWMH = (LPWINDOWMENUHEADER)GlobalLock (hWMH);

	if (!pWMH->menuHandle[pWMH->currentMenu])
	{
		GlobalUnlock (hWMH);
		hWMH = savehWMH;
		pWMH = savepWMH;
		return DefWindowProc(hWnd, Message, wParam, lParam);
	}

	savepAVHeader = pAVHeader;

	pAVHeader = (LPAUTOVISHEADER)GlobalLock (pWMH->menuHandle[pWMH->currentMenu]);

	switch (Message)
   {
	case WM_CREATE:
		ii=1;
	break;

    case WM_CLOSE:
		{
			RECT	rect;
			char	VPConfigAndName[40]="";
			char	appliesToVP[40];
			BOOL	Err;
			int		nl;
			HWND	hWndDestroy;
			double	fac = pWMH->Factor;
			BOOL	Float = pWMH->Float;

			strcpy (appliesToVP,pWMH->appliesToVP);
			if (wParam == 3)
				ii=1;
			else if (wParam)
			{
				GetWindowRect (pWMH->hWndMenu,&rect);
				rect.right = max (rect.right,rect.left+GetMaxRectRight ()+32);
			}
			else
			{
				strcpy (VPConfigAndName,"%RECT%");
				rect = pWMH->FixedRect;
			}
			nl = GetHandleLockCount (pWMH->menuHandle[pWMH->currentMenu]);
			while (nl--)
				GlobalUnlock (pWMH->menuHandle[pWMH->currentMenu]);
			if (lParam)
			{
				SetWindowLong (pWMH->hWndMenu,GWL_USERDATA,0);
				SetWindowLong (pWMH->hWndDisplay,GWL_USERDATA,0);
			}
			nl = GetHandleLockCount (hWMH);
			hWndDestroy = pWMH->hWndMenu;
			pWMH->pVP->hWndDlg = 0;
			while (nl--)
				GlobalUnlock (hWMH);
			pWMH = 0;
			DestroyWindow (hWndDestroy);
			SetConfig (1);
			SetCurView (SetVPFromName (appliesToVP,&Err));
			if (lParam)
				VisibilityControl (CurView->hWnd,hInst,VPConfigAndName,&rect,fac,(HANDLE)lParam,FALSE,!Float,wParam!=3);
		}
		break;

	case WM_MOVING:
		HaveMoving = pWMH->Float;
		break;
    case WM_MOVE:     /*  code for moving the window                    */
		if (HaveMoving)
		{
			i=1;
		}
		//HaveMoving = FALSE;
        break;
    case GSSI_REPOSITION:
		 ii=1;
		 break;

	case WM_LBUTTONDOWN:
		 if (pAVHeader->iCurrentRect < 0)
			 break;
		 {
				LPAVRECTANGLE	pAVRect = GlobalLock (pAVHeader->hRectangles);
				BOOL	AllowDoubleClick;
				
				pAVRect += pAVHeader->iCurrentRect;
				AllowDoubleClick = pAVRect->AllowDoubleClick;
				GlobalUnlock (pAVHeader->hRectangles);
				if (!AllowDoubleClick)
				{
					nClicks = 0;
					goto SingleClick;
				}
		 }
		 if (HaveTimer)
			 KillTimer (hWnd,1);
		 SetTimer (hWnd,1,GetDoubleClickTime(),0);
		 HaveTimer = TRUE;
		 break;

	case WM_LBUTTONDBLCLK:
		nClicks++;
	case WM_TIMER:
SingleClick:
		KillTimer (hWnd,1);
		HaveTimer = FALSE;
		nClicks++;
		if (pAVHeader->iCurrentRect > -1)
		{
			if (pAVHeader->hRectangles)
			{
				LPAVRECTANGLE	pAVRect = GlobalLock (pAVHeader->hRectangles);
				
				pAVRect += pAVHeader->iCurrentRect;
				pAVHeader->iCurrentAV = pAVRect->iAV;
				GlobalUnlock (pAVHeader->hRectangles);
				DestroySavedScreen (&pWMH->hSaveScreen,0);
	     		InvalidateRect (hWnd,0,TRUE);
			}
		}
		break;

	case WM_NCHITTEST:
		{
			rtn = DefWindowProc(hWnd, Message, wParam, lParam);
			if (HaveMoving)
			{
		   		POINT	CursorPoint = POINTStoPOINT(MAKEPOINTS (lParam)); 

				HaveMoving = FALSE;
				if (PtInRect (&pWMH->FixedRect,CursorPoint))
				{
					PostMessage (hWnd,WM_CLOSE,0,(LPARAM)hWMH);
				}
			}
			goto Exit;
		}
		break;
	case WM_RBUTTONDOWN:
		break;

	case WM_RBUTTONUP:
		break;

	case WM_MBUTTONUP:
		break;
 
	case WM_MOUSELEAVE:
		hDC = GetDC (hWndMain);
		ClearMeterPrompts (hDC);
		ReleaseDC (hWndMain,hDC);
		hDC = GetDC (hWnd);
		if (pVCHeader->hRectangles)
		{
			LPVCRECTANGLE	pVCRect = GlobalLock (pVCHeader->hRectangles);

			if (pVCHeader->iCurrentRect > -1)
				InvertRectBorder (hDC,&pVCRect[pVCHeader->iCurrentRect].rect);
			pVCHeader->iCurrentRect = -1;
			GlobalUnlock (pVCHeader->hRectangles);
		}
		break;
				
	case WM_MOUSEMOVE:
		{
    		POINT	MovePoint = POINTStoPOINT(MAKEPOINTS (lParam)); 
			if (HaveTimer)
				break;
			if (hWnd != pWMH->hWndDisplay)
				break;

			hDC = GetDC (hWndMain);
			ClearMeterPrompts (hDC);
			ReleaseDC (hWndMain,hDC);
			hDC = GetDC (hWnd);
			if (pAVHeader->hRectangles)
			{
				LPAVRECTANGLE	pAVRect = GlobalLock (pAVHeader->hRectangles);

				for (i=0;i<pAVHeader->nRectangles;i++)
				{
					if (PtInRect (&pAVRect[i].rect,MovePoint))
					{
						if (i != pAVHeader->iCurrentRect)
						{
							if (pAVHeader->iCurrentRect > -1)
								InvertRectBorder (hDC,&pAVRect[pAVHeader->iCurrentRect].rect);
							InvertRectBorder (hDC,&pAVRect[i].rect);
						}
						pAVHeader->iCurrentRect = i;
						break;
					}
				}
				if (i == pAVHeader->nRectangles) //not in rect
				{
					if (pAVHeader->iCurrentRect > -1)
						InvertRectBorder (hDC,&pAVRect[pAVHeader->iCurrentRect].rect);
					pAVHeader->iCurrentRect = -1;
					SetSysMess ("");
				}
				else
				{
					sprintf (mess,"Single click to select %s",pAVHeader->AVFile[i]);
					SetSysMess (mess);

				}
				GlobalUnlock (pAVHeader->hRectangles);
			}
			ReleaseDC (hWnd,hDC);
		}
		break;

	case WM_ERASEBKGND:
		{
			RECT	Rect;

			hDC = GetDC (hWnd);
			GetClientRect (hWnd,&Rect);
			FillRectPoly (hDC,&Rect,RGB(255,255,255));
			ReleaseDC (hWnd,hDC);
		}
		break;

	case WM_HSCROLL:
		{
			int nScrollCode = (int) LOWORD(wParam);  // scroll bar value 
			int	nPos, iPos = (short int) HIWORD(wParam);   // scroll box position 
			HWND	hwndScrollBar = (HWND) lParam;       // handle to scroll bar 
			RECT	rect;

			GetClientRect (hWnd,&rect);
			nPos = GetScrollPos (hWnd,SB_HORZ);

			switch (nScrollCode)
			{
			case SB_THUMBPOSITION:
				SetScrollPos (hWnd,SB_HORZ,iPos,TRUE);
	     		InvalidateRect (hWnd,0,TRUE);
				break;
			case SB_PAGEDOWN:
				SetScrollPos (hWnd,SB_HORZ,nPos+RECTWIDTH(&rect)/2,TRUE);
	     		InvalidateRect (hWnd,0,TRUE);
				break;
			case SB_PAGEUP:
				SetScrollPos (hWnd,SB_HORZ,nPos-RECTWIDTH(&rect)/2,TRUE);
	     		InvalidateRect (hWnd,0,TRUE);
				break;
			case SB_LINEDOWN:
				SetScrollPos (hWnd,SB_HORZ,nPos+RECTWIDTH(&rect)/10,TRUE);
	     		InvalidateRect (hWnd,0,TRUE);
				break;
			case SB_LINEUP:
				SetScrollPos (hWnd,SB_HORZ,nPos-RECTWIDTH(&rect)/10,TRUE);
	     		InvalidateRect (hWnd,0,TRUE);
				break;
			}
		}
		break;

	case WM_MOUSEWHEEL:
		{
			int	nPos = GetScrollPos (hWnd,SB_VERT);
			int	inc  = (short) HIWORD(wParam);    

			nPos -= inc;
			SetScrollPos (hWnd,SB_VERT,nPos,TRUE);
     		InvalidateRect (hWnd,0,TRUE);

		}
		break;

	case WM_VSCROLL:
		{
			int nScrollCode = (int) LOWORD(wParam);  // scroll bar value 
			int	nPos,iPos = (short int) HIWORD(wParam);   // scroll box position 
			HWND	hwndScrollBar = (HWND) lParam;       // handle to scroll bar 
			RECT	rect;

			GetClientRect (hWnd,&rect);
			nPos = GetScrollPos (hWnd,SB_VERT);

			switch (nScrollCode)
			{
			case SB_THUMBPOSITION:
				SetScrollPos (hWnd,SB_VERT,iPos,TRUE);
				DestroySavedScreen (&pWMH->hSaveScreen,0);
	     		InvalidateRect (hWnd,0,TRUE);
				break;
			case SB_PAGEDOWN:
				SetScrollPos (hWnd,SB_VERT,nPos+RECTHEIGHT(&rect)/2,TRUE);
				DestroySavedScreen (&pWMH->hSaveScreen,0);
	     		InvalidateRect (hWnd,0,TRUE);
				break;
			case SB_LINEDOWN:
				SetScrollPos (hWnd,SB_VERT,nPos+RECTHEIGHT(&rect)/10,TRUE);
				DestroySavedScreen (&pWMH->hSaveScreen,0);
	     		InvalidateRect (hWnd,0,TRUE);
				break;
			case SB_PAGEUP:
				SetScrollPos (hWnd,SB_VERT,nPos-RECTHEIGHT(&rect)/2,TRUE);
				DestroySavedScreen (&pWMH->hSaveScreen,0);
	     		InvalidateRect (hWnd,0,TRUE);
			case SB_LINEUP:
				SetScrollPos (hWnd,SB_VERT,nPos-RECTHEIGHT(&rect)/10,TRUE);
				DestroySavedScreen (&pWMH->hSaveScreen,0);
	     		InvalidateRect (hWnd,0,TRUE);
				break;
			}
		}
		break;

    case WM_PAINT:    /* code for the window's client area              */
         /* Obtain a handle to the device context                       */
         /* BeginPaint will sends WM_ERASEBKGND if appropriate          */
		{
			PAINTSTRUCT	ps;
			int		x=3;
			int		y=3;
			int		ystart;
			POINT	Point;
			RECT	clientrect;
			int		iAV;

			if (hWnd != pWMH->hWndDisplay || !pAVHeader)
				goto ExitDefault;
			GetClientRect (hWnd,&clientrect);

			memset(&ps, 0x00, sizeof(PAINTSTRUCT));
			hDC = BeginPaint(hWnd, &ps);
			pAVHeader->iCurrentRect = -1;
			GSSiGlobFree (&pAVHeader->hRectangles);
			pAVHeader->nRectangles = 0;
			menuy = y++;
			x = 2*pWMH->Factor - GetScrollPos (hWnd,SB_HORZ);
			y += (- GetScrollPos (hWnd,SB_VERT));
			ystart = y;
			for (iAV=0;iAV<pAVHeader->nAutoVis;iAV++)
			{
				DrawAVButton (hDC,pAVHeader->AVFile[iAV],iAV,&x,&y,(iAV==pAVHeader->iCurrentAV));
			}
			if (!ShowScrollBar (hWnd,SB_VERT,GetMaxAVRectBottom() > clientrect.bottom ||ystart < 0))
				ii=1;
			ShowScrollBar (hWnd,SB_HORZ,!pWMH->Float);
			GetCursorPos (&Point);
			SetCursorPos (Point.x,Point.y);
			EndPaint(hWnd, &ps);
		}
         break;       /*  End of WM_PAINT                               */

    case WM_COMMAND:
    {         
         switch(LOWORD(wParam))
    	{
		 case IDM_AUTOREDRAW:
			pVCHeader->AutoRedraw = !pVCHeader->AutoRedraw;
			break;
		 case IDM_DISPLAYSYMBOLS:
			pVCHeader->DisplaySymbols = !pVCHeader->DisplaySymbols;
			break;
		 case IDM_DISPLAYMODIFIERS:
			pVCHeader->DisplayModifiers = !pVCHeader->DisplayModifiers;
			break;
		 case IDM_INTENSITYMOD:
			AddModifier (curviewport,curlayer,cursymbol,IDM_INTENSITYMOD);
			break;
		 case IDM_COLORMOD:
			AddModifier (curviewport,curlayer,cursymbol,IDM_COLORMOD);
			break;
		 case IDM_RANDOMMOD:
			AddModifier (curviewport,curlayer,cursymbol,IDM_RANDOMMOD);
			break;
		 case IDM_SIZEMOD:
			AddModifier (curviewport,curlayer,cursymbol,IDM_SIZEMOD);
			break;
        }
		InvalidateRect (hWnd,0,TRUE);
        break;   	 
    			
    }
    	break;
    default:
         /* For any message for which you don't specifically provide a  */
         /* service routine, you should return the message to Windows   */
         /* for default message processing  */
//		if (pWMH->menuHandle[pWMH->currentMenu])
//			GlobalUnlock (pWMH->menuHandle[pWMH->currentMenu]);
ExitDefault:
		SetConfig (SaveCfg);
		CurView = SaveVP;
		CurVis = SaveVis;
		GlobalUnlock (pWMH->menuHandle[pWMH->currentMenu]);
		GlobalUnlock (hWMH);
		pAVHeader = savepAVHeader;
		hWMH = savehWMH;
		pWMH = savepWMH;
        return DefWindowProc(hWnd, Message, wParam, lParam);
   }
Exit:
	SetConfig (SaveCfg);
	CurView = SaveVP;
	CurVis = SaveVis;
	if (hWMH)
	{
		GlobalUnlock(pWMH->menuHandle[pWMH->currentMenu]);
		GlobalUnlock(hWMH);
	}
	pAVHeader = savepAVHeader;
	hWMH = savehWMH;
	pWMH = savepWMH;
	return rtn;
} 
                              
LONG FAR PASCAL CMDMenuWndProc(HWND hWnd, int Message, WPARAM wParam, LONG lParam)
{
	HDC	hDC;
	long	rtn=0;
	int	ilayer,i,nClicks=0;
	static	BOOL	HaveTimer;
	char	mess[256];
	int		status,ii;
	BOOL	nRc;
	static	BOOL	InTrackMenu=FALSE;
	static	HWND	hWndTab=0,hWndDisplay=0;
	RECT	TabClientRect;
	HANDLE	savehWMH;
	LPWINDOWMENUHEADER savepWMH;
	LPCMDMENUHEADER	savepCMDHeader;

	if (InTrackMenu)
		return DefWindowProc(hWnd, Message, wParam, lParam);

    if (Message == WM_NCCREATE)
	{
		char	TempFile[MAX_PATH];
		int		layer, symnum, ilayer;
		HFILE	Fid;
		char	str[256],str2[512];
		
		hInitData = GetMenuWindowHandle (hWnd);
		HaveTimer = FALSE;
		HaveMoving = FALSE;
		pWMH = (LPWINDOWMENUHEADER)GlobalLock (hInitData);
		
		if (!pWMH->menuHandle[pWMH->currentMenu])
			return DefWindowProc(hWnd, Message, wParam, lParam);
		else
			pCMDHeader = (LPCMDMENUHEADER)GlobalLock (pWMH->menuHandle[pWMH->currentMenu]);

		GlobalUnlock (pWMH->menuHandle[pWMH->currentMenu]);
		GlobalUnlock (hInitData);
		return DefWindowProc(hWnd, Message, wParam, lParam);
	}

	savehWMH = hWMH;
	savepWMH = pWMH;
	hWMH = GetMenuWindowHandle (hWnd);
	if (!hWMH)
	{
		hWMH = savehWMH;
		return DefWindowProc(hWnd, Message, wParam, lParam);
	}
	pWMH = (LPWINDOWMENUHEADER)GlobalLock (hWMH);

	if (!pWMH->menuHandle[pWMH->currentMenu])
	{
		GlobalUnlock (hWMH);
		hWMH = savehWMH;
		pWMH = savepWMH;
		return DefWindowProc(hWnd, Message, wParam, lParam);
	}

	savepCMDHeader = pCMDHeader;

	pCMDHeader = (LPCMDMENUHEADER)GlobalLock (pWMH->menuHandle[pWMH->currentMenu]);

	switch (Message)
   {
	case WM_CREATE:
		ii=1;
	break;

    case WM_CLOSE:
		{
			RECT	rect;
			char	VPConfigAndName[40]="";
			char	appliesToVP[40];
			BOOL	Err;
			int		nl;
			HWND	hWndDestroy;
			double	fac=pWMH->Factor;
			BOOL	Float = pWMH->Float;

			strcpy (appliesToVP,pWMH->appliesToVP);
			if (wParam == 3)
				ii=1;
			else if (wParam)
			{
				GetWindowRect (hWnd,&rect);
				rect.right = max (rect.right,rect.left+GetMaxRectRight ()+32);
			}
			else
			{
				strcpy (VPConfigAndName,"%RECT%");
				rect = pWMH->FixedRect;
			}
			nl = GetHandleLockCount (pWMH->menuHandle[pWMH->currentMenu]);
			while (nl--)
				GlobalUnlock (pWMH->menuHandle[pWMH->currentMenu]);
			if (lParam)
			{
				SetWindowLong (pWMH->hWndMenu,GWL_USERDATA,0);
				SetWindowLong (pWMH->hWndDisplay,GWL_USERDATA,0);
			}
			nl = GetHandleLockCount (hWMH);
			hWndDestroy = pWMH->hWndMenu;
			while (nl--)
				GlobalUnlock (hWMH);
			pWMH = 0;
			DestroyWindow (hWndDestroy);
			SetConfig (1);
			SetCurView (SetVPFromName (appliesToVP,&Err));
			if (lParam)
				VisibilityControl (CurView->hWnd,hInst,VPConfigAndName,&rect,fac,(HANDLE)lParam,FALSE,!Float,wParam!=3);
		}
		break;

	case WM_MOVING:
		HaveMoving = pWMH->Float;
		break;
    case WM_MOVE:     /*  code for moving the window                    */
		if (HaveMoving)
		{
			i=1;
		}
		//HaveMoving = FALSE;
        break;
    case GSSI_REPOSITION:
		 ii=1;
		 break;

	case WM_LBUTTONDOWN:
		 if (pCMDHeader->iCurrentRect < 0)
			 break;
		 {
				LPCMDRECTANGLE	pCMDRect = GlobalLock (pCMDHeader->hRectangles);
				BOOL	AllowDoubleClick;
				
				pCMDRect += pCMDHeader->iCurrentRect;
				AllowDoubleClick = pCMDRect->AllowDoubleClick;
				GlobalUnlock (pCMDHeader->hRectangles);
				if (!AllowDoubleClick)
				{
					nClicks = 0;
					goto SingleClick;
				}
		 }
		 if (HaveTimer)
			 KillTimer (hWnd,1);
		 SetTimer (hWnd,1,GetDoubleClickTime(),0);
		 HaveTimer = TRUE;
		 break;

	case WM_LBUTTONDBLCLK:
		nClicks++;
	case WM_TIMER:
SingleClick:
		KillTimer (hWnd,1);
		HaveTimer = FALSE;
		nClicks++;
		if (pCMDHeader->iCurrentRect > -1)
		{
			if (pCMDHeader->hRectangles)
			{
				LPCMDRECTANGLE	pCMDRect = GlobalLock (pCMDHeader->hRectangles);
				
				pCMDRect += pCMDHeader->iCurrentRect;
				pCMDHeader->iCurrentCMD = pCMDRect->iCMD;
				GlobalUnlock (pCMDHeader->hRectangles);
				ProcessCMDMenuCmd (pCMDHeader->iCurrentCMD);
				DestroySavedScreen (&pWMH->hSaveScreen,0);
				InvalidateRect (hWnd,0,TRUE);
			}
		}
		break;

	case WM_RBUTTONDOWN:
		break;

	case WM_RBUTTONUP:
		break;

	case WM_MBUTTONUP:
		break;
 
	case WM_MOUSELEAVE:
		hDC = GetDC (hWndMain);
		ClearMeterPrompts (hDC);
		ReleaseDC (hWndMain,hDC);
		hDC = GetDC (hWnd);
		if (pCMDHeader->hRectangles)
		{
			LPCMDRECTANGLE	pVCRect = GlobalLock (pCMDHeader->hRectangles);

			if (pCMDHeader->iCurrentRect > -1)
				InvertRectBorder (hDC,&pVCRect[pCMDHeader->iCurrentRect].rect);
			pCMDHeader->iCurrentRect = -1;
			GlobalUnlock (pCMDHeader->hRectangles);
		}
		break;
				
	case WM_MOUSEMOVE:
		{
    		POINT	MovePoint = POINTStoPOINT(MAKEPOINTS (lParam)); 
			if (HaveTimer)
				break;
			if (hWnd != pWMH->hWndDisplay)
				break;

			hDC = GetDC (hWndMain);
			ClearMeterPrompts (hDC);
			ReleaseDC (hWndMain,hDC);
			hDC = GetDC (hWnd);
			if (pCMDHeader->hRectangles)
			{
				LPCMDRECTANGLE	pCMDRect = GlobalLock (pCMDHeader->hRectangles);

				for (i=0;i<pCMDHeader->nRectangles;i++)
				{
					if (PtInRect (&pCMDRect[i].rect,MovePoint))
					{
						if (i != pCMDHeader->iCurrentRect)
						{
							if (pCMDHeader->iCurrentRect > -1)
								InvertRectBorder (hDC,&pCMDRect[pCMDHeader->iCurrentRect].rect);
							InvertRectBorder (hDC,&pCMDRect[i].rect);
						}
						pCMDHeader->iCurrentRect = i;
						break;
					}
				}
				if (i == pCMDHeader->nRectangles) //not in rect
				{
					if (pCMDHeader->iCurrentRect > -1)
						InvertRectBorder (hDC,&pCMDRect[pCMDHeader->iCurrentRect].rect);
					pCMDHeader->iCurrentRect = -1;
					SetSysMess ("");
				}
				else
				{
					sprintf (mess,"Single click to select %s",pCMDHeader->CMDFile);
					SetSysMess (mess);

				}
				GlobalUnlock (pCMDHeader->hRectangles);
			}
			ReleaseDC (hWnd,hDC);
		}
		break;

	case WM_ERASEBKGND:
		{
			RECT	Rect;

			hDC = GetDC (hWnd);
			GetClientRect (hWnd,&Rect);
			FillRectPoly (hDC,&Rect,RGB(255,255,255));
			ReleaseDC (hWnd,hDC);
		}
		break;


    case WM_PAINT:    /* code for the window's client area              */
         /* Obtain a handle to the device context                       */
         /* BeginPaint will sends WM_ERASEBKGND if appropriate          */
		{
			PAINTSTRUCT	ps;
			int		x=3;
			int		y=3, ystart;
			int		w, maxw=0;
			POINT	Point;
			RECT	clientrect;
			BOOL	DoRedisplay = FALSE;
			int		iCMD;

			if (hWnd != pWMH->hWndDisplay || !pCMDHeader)
				goto ReturnDefault;
			ShowScrollBar (hWnd,SB_VERT,pCMDHeader->VScrollIsVis);
			GetClientRect (hWnd,&clientrect);
			w = RECTWIDTH (&clientrect)-4;
			memset(&ps, 0x00, sizeof(PAINTSTRUCT));
			hDC = BeginPaint(hWnd, &ps);
			pCMDHeader->iCurrentRect = -1;
			GSSiGlobFree (&pCMDHeader->hRectangles);
			pCMDHeader->nRectangles = 0;
			if (!pWMH->Float)
			{
				x = -4;
				y = 3;
				//DrawCmdButton (hDC,"Float",2,&x,&y,clientrect.right);
			}
			menuy = y++;
			x = 2*pWMH->Factor - GetScrollPos (hWnd,SB_HORZ);
			y += (- GetScrollPos (hWnd,SB_VERT));
			ystart = y;
			for (iCMD=0;iCMD<pCMDHeader->nCMD;iCMD++)
			{
				maxw = max (maxw,DrawCMDButton (hDC,pCMDHeader->Label[iCMD],pCMDHeader->hBM[iCMD],iCMD,&x,&y,-buttonheight,w));
			}
			y = ystart;
			if (pWMH->Float)
				w = maxw;
			for (iCMD=0;iCMD<pCMDHeader->nCMD;iCMD++)
			{
				DrawCMDButton (hDC,pCMDHeader->Label[iCMD],pCMDHeader->hBM[iCMD],iCMD,&x,&y,buttonheight,w);
			}
			pCMDHeader->VScrollIsVis = GetMaxCMDRectBottom() > clientrect.bottom || ystart < 0;
			ShowScrollBar (hWnd,SB_VERT,pCMDHeader->VScrollIsVis);
			if (pCMDHeader->VScrollIsVisPrevious != pCMDHeader->VScrollIsVis)
				DoRedisplay = TRUE;
			pCMDHeader->VScrollIsVisPrevious = pCMDHeader->VScrollIsVis;
			ShowScrollBar (hWnd,SB_HORZ,FALSE);//!pWMH->Float);
			SetScrollRange (hWnd,SB_VERT,0,y-(clientrect.bottom+ystart),TRUE);
			GetCursorPos (&Point);
			SetCursorPos (Point.x,Point.y);
			EndPaint(hWnd, &ps);
			if (pWMH->Float && abs (clientrect.right - GetMaxRectRight()) > 32)
			{
				RECT	winRect;
				int		w;

				GetWindowRect (pWMH->hWndMenu,&winRect);
				w = RECTWIDTH(&winRect)+(GetMaxRectRight()-clientrect.right);
				if (pCMDHeader->VScrollIsVis) w += 32;
				SetWindowPos(pWMH->hWndMenu, 0, winRect.left,winRect.top,w,RECTHEIGHT(&winRect),
					SWP_DRAWFRAME|SWP_NOZORDER|SWP_SHOWWINDOW);
				PostMessage (pWMH->hWndMenu,WM_CLOSE,2,(LPARAM)hWMH);
			}
			else if (DoRedisplay)
				InvalidateRect (hWnd,0,TRUE);
			else
				ShowWindow (pWMH->hWndMenu,SW_SHOW);
		}
         break;       /*  End of WM_PAINT                               */

    case WM_COMMAND:
    {         
/*         switch(LOWORD(wParam))
    	{
		 case IDM_AUTOREDRAW:
			pVCHeader->AutoRedraw = !pVCHeader->AutoRedraw;
			break;
		 case IDM_DISPLAYSYMBOLS:
			pVCHeader->DisplaySymbols = !pVCHeader->DisplaySymbols;
			break;
		 case IDM_DISPLAYMODIFIERS:
			pVCHeader->DisplayModifiers = !pVCHeader->DisplayModifiers;
			break;
		 case IDM_INTENSITYMOD:
			AddModifier (curviewport,curlayer,cursymbol,IDM_INTENSITYMOD);
			break;
		 case IDM_COLORMOD:
			AddModifier (curviewport,curlayer,cursymbol,IDM_COLORMOD);
			break;
		 case IDM_RANDOMMOD:
			AddModifier (curviewport,curlayer,cursymbol,IDM_RANDOMMOD);
			break;
		 case IDM_SIZEMOD:
			AddModifier (curviewport,curlayer,cursymbol,IDM_SIZEMOD);
			break;
        }*/
		DestroySavedScreen (&pWMH->hSaveScreen,0);
		InvalidateRect (hWnd,0,TRUE);
        break;   	 
    			
    }
    	break;
    default:
         /* For any message for which you don't specifically provide a  */
         /* service routine, you should return the message to Windows   */
         /* for default message processing  */
//		if (pWMH->menuHandle[pWMH->currentMenu])
//			GlobalUnlock (pWMH->menuHandle[pWMH->currentMenu]);
ReturnDefault:
		SetConfig (SaveCfg);
		CurView = SaveVP;
		CurVis = SaveVis;
		GlobalUnlock (pWMH->menuHandle[pWMH->currentMenu]);
		GlobalUnlock (hWMH);
		pCMDHeader = savepCMDHeader;
		hWMH = savehWMH;
		pWMH = savepWMH;

        return DefWindowProc(hWnd, Message, wParam, lParam);
   } 
	SetConfig (SaveCfg);
	CurView = SaveVP;
	CurVis = SaveVis;
	GlobalUnlock (pWMH->menuHandle[pWMH->currentMenu]);
	GlobalUnlock (hWMH);
	pCMDHeader = savepCMDHeader;
	hWMH = savehWMH;
	pWMH = savepWMH;
//	if (pWMH->menuHandle[pWMH->currentMenu])
//		GlobalUnlock (pWMH->menuHandle[pWMH->currentMenu]);
	return rtn;
}   

SIZE DisplayCatalog (HWND hWnd,HDC hDC,LPINT pYstart)
{
	SIZE	size={0,0};
	int		x=3;
	int		y=buttonheight+2;
	POINT	Point;
	int		ilayer;

	pVCHeader->iCurrentRect = -1;
	GSSiGlobFree (&pVCHeader->hRectangles);
	pVCHeader->nRectangles = 0;
	menuy = y++;
	x = 55*pWMH->Factor - GetScrollPos (hWnd,SB_HORZ);
	y += (- GetScrollPos (hWnd,SB_VERT));
	*pYstart = y;
	size.cx = max (size.cx,x);
	size.cy = max (size.cy,y);
	if (pWMH->menuType[pWMH->currentMenu] == MT_PICKABLE)
	{
		int xt = 16;

		DrawVisButton (hDC,"Same as Visibility",7,CurView->ID,0,0,&xt,&y,MAKELONG(pVCHeader->PickIsSame,1),FALSE);
		y+=8;
		size.cx = max (size.cx,x);
		size.cy = max (size.cy,y);
	}
	if (pWMH->menuType[pWMH->currentMenu] != MT_PICKABLE || !pVCHeader->PickIsSame)
	{
		if (pWMH->menuType[pWMH->currentMenu] != MT_ALLSYMBOLS && pWMH->menuType[pWMH->currentMenu] != MT_VISSYMBOLS)
			DrawVisButton (hDC,CurView->Name,0,CurView->ID,0,0,&x,&y,MAKELONG(CurView->Active,1),pVCHeader->ExpandViewport);
		x += indent;
		size.cx = max (size.cx,x);
		size.cy = max (size.cy,y);
		if (pVCHeader->ExpandViewport == 2)
		{
			if (pWMH->menuType[pWMH->currentMenu] == MT_ALLSYMBOLS || pWMH->menuType[pWMH->currentMenu] == MT_VISSYMBOLS)
			{
				ilayer = pVCHeader->nLayers;
				DisplayLayerSymbols (hDC,1,0,0,CurView->ID,ilayer,&x,&y,pVCHeader->nLayerSyms[ilayer],pVCHeader->hLayerSyms[ilayer],pVCHeader->hSymOrder[ilayer]);
				size.cx = max (size.cx,x);
				size.cy = max (size.cy,y);
			}
			else for (ilayer=0;ilayer<pVCHeader->nLayers;ilayer++)
			{
				int	On = GetOnValue (ilayer,0,1,pVCHeader->nLayerSyms[ilayer],pVCHeader->hLayerSyms[ilayer]);

				DrawVisButton (hDC,CurView->FileID[ilayer],1,CurView->ID,ilayer,0,&x,&y,On,pVCHeader->ExpandLayer[ilayer]);
				if (pVCHeader->ExpandLayer[ilayer] == 2)
					DisplayLayerSymbols (hDC,1,0,0,CurView->ID,ilayer,&x,&y,pVCHeader->nLayerSyms[ilayer],pVCHeader->hLayerSyms[ilayer],pVCHeader->hSymOrder[ilayer]);
				size.cx = max (size.cx,x);
				size.cy = max (size.cy,y);
			}
		}
	}
	size.cy -= *pYstart; 
	return size;
}

BOOL InitCatalogData (HANDLE hInitData,BOOL Pickability)
{
	char	TempFile[MAX_PATH];
	int		layer, symnum, ilayer;
	HFILE	Fid;
	char	str[260];

	symnumALL = GetDictSymbolNumber("ALL");
	pWMH = (LPWINDOWMENUHEADER)GlobalLock (hInitData);
	
		pWMH->menuHandle[pWMH->currentMenu] = GSSiGlobAlloc (1684,GHND,sizeof(VISCONTROLHEADER));
		pVCHeader = GlobalLock (pWMH->menuHandle[pWMH->currentMenu]);
		pVCHeader->iCurrentRect = -1;
		pVCHeader->Config = CurrentConfig;
		pVCHeader->VPID = CurView->ID;
		pVCHeader->AutoRedraw = TRUE;
		pVCHeader->Pickability = FALSE;
		pVCHeader->DisplayModifiers = TRUE;
		pVCHeader->DisplaySymbols = TRUE;
		pVCHeader->nLayers = CurView->NumFiles;
		pVCHeader->ExpandViewport = 2;
		pVCHeader->Pickability = pWMH->menuType[pWMH->currentMenu] == MT_PICKABLE;
		GSSiGetTempFileName (0,"gm",0,TempFile);
		DumpVisibilityToFile (TempFile,TRUE,0);
		Fid = GSSiOpenFile (TempFile,0,OF_READ);
		fgetstring (str,256,Fid);
		while (fgetstring (str,256,Fid))
		{
			LPSTR	pSpace=strrchr (str,'\t');
			
			*pSpace++ = 0;
			layer = atoi (pSpace);
			pSpace=strrchr (str,'\t');
			symnum = atoi (pSpace);
			if (GetDictSymbolType (symnum))
				AddSymbolToLayer (layer,symnum,1,0,&pVCHeader->nLayerSyms[layer],&pVCHeader->hLayerSyms[layer]);
		}
		GSSiClose2 (&Fid);
		GSSiRemove (TempFile);
		SetConfig (SaveCfg);
		CurView = SaveVP;
		CurVis = SaveVis;
		for (ilayer=0;ilayer<=pVCHeader->nLayers;ilayer++)
		{
			if (pVCHeader->nLayerSyms[ilayer])
			{
				pVCHeader->ExpandLayer[ilayer] = 2;
				SortLayerSymbols (pVCHeader->nLayerSyms[ilayer],pVCHeader->hLayerSyms[ilayer],&pVCHeader->hSymOrder[ilayer]);
			}
			else
				pVCHeader->ExpandLayer[ilayer] = 0;
		}
	GlobalUnlock (pWMH->menuHandle[pWMH->currentMenu]);	
	GlobalUnlock (hInitData);
	return TRUE;
}


LONG FAR PASCAL CatalogMenuWndProc(HWND hWnd, int Message, WPARAM wParam, LONG lParam)
{
	HDC	hDC;
	long	rtn=0;
	int	ilayer,i,nClicks=0;
	static	BOOL	HaveTimer;
	char	mess[256];
	int		status,ii;
	BOOL	nRc;
	static	BOOL	InTrackMenu=FALSE;
	static	HWND	hWndTab=0,hWndDisplay=0;
	RECT	TabClientRect;
	static	BOOL	VScrollIsVisPrevious=FALSE;
	HANDLE	savehWMH;
	LPWINDOWMENUHEADER savepWMH;
	LPVISCONTROLHEADER	savepVCHeader;


	if (InTrackMenu)
		return DefWindowProc(hWnd, Message, wParam, lParam);

    if (Message == WM_NCCREATE)
	{
		
		HaveTimer = FALSE;
		HaveMoving = FALSE;
		hInitData = GetMenuWindowHandle (hWnd);
		pWMH = GlobalLock (hInitData);
		if (!pWMH->menuHandle[pWMH->currentMenu])
		{
			HaltMapDisplay (FALSE,TRUE);
			InitCatalogData (hInitData,0);
		}
		pVCHeader = GlobalLock (pWMH->menuHandle[pWMH->currentMenu]);

		pWMH->Float = floating;
		if (!floating)
			pWMH->FixedRect = OrigRect;
		SetScrollRange (hWnd,SB_VERT,0,10000,FALSE);
		SetScrollRange (hWnd,SB_HORZ,0,500,FALSE);
		GlobalUnlock (pWMH->menuHandle[pWMH->currentMenu]);
		GlobalUnlock (hInitData);
		return TRUE;
		//return DefWindowProc(hWnd, Message, wParam, lParam);
	}
	savehWMH = hWMH;
	savepWMH = pWMH;
	hWMH = GetMenuWindowHandle (hWnd);
	if (!hWMH)
	{
		hWMH = savehWMH;
		return DefWindowProc(hWnd, Message, wParam, lParam);
	}
	pWMH = (LPWINDOWMENUHEADER)GlobalLock (hWMH);

	if (!pWMH->menuHandle[pWMH->currentMenu])
	{
		GlobalUnlock (hWMH);
		hWMH = savehWMH;
		pWMH = savepWMH;
		return DefWindowProc(hWnd, Message, wParam, lParam);
	}

	savepVCHeader = pVCHeader;

	pVCHeader = (LPVISCONTROLHEADER)GlobalLock (pWMH->menuHandle[pWMH->currentMenu]);
	switch (Message)
   {
	case WM_CREATE:
		VScrollIsVisPrevious = TRUE;
	break;

    case WM_CLOSE:
		break;

	case WM_MOVING:
		HaveMoving = pWMH->Float;
		break;
    case WM_MOVE:     /*  code for moving the window                    */
		if (HaveMoving)
		{
			i=1;
		}
		//HaveMoving = FALSE;
        break;
    case GSSI_REPOSITION:
		 ii=1;
		 break;

	case WM_LBUTTONDOWN:
		 if (pVCHeader->iCurrentRect < 0)
			 break;
		 {
				LPVCRECTANGLE	pVCRect = GlobalLock (pVCHeader->hRectangles);
				BOOL	AllowDoubleClick;
				
				pVCRect += pVCHeader->iCurrentRect;
				AllowDoubleClick = pVCRect->AllowDoubleClick;
				GlobalUnlock (pVCHeader->hRectangles);
				if (!AllowDoubleClick)
				{
					nClicks = 0;
					goto SingleClick;
				}
		 }
		 if (HaveTimer)
			 KillTimer (hWnd,1);
		 SetTimer (hWnd,1,GetDoubleClickTime(),0);
		 HaveTimer = TRUE;
		 break;

	case WM_LBUTTONDBLCLK:
		nClicks++;
	case WM_TIMER:
//	case WM_RBUTTONDOWN:
SingleClick:
		KillTimer (hWnd,1);
		HaveTimer = FALSE;
		nClicks++;
		if (pVCHeader->iCurrentRect > -1)
		{
			if (pVCHeader->hRectangles)
			{
				LPVCRECTANGLE	pVCRect = GlobalLock (pVCHeader->hRectangles);
				LPVCSYMBOL		pSym;
				
				hDC = GetDC (hWnd);
				InvertRectBorder (hDC,&pVCRect[pVCHeader->iCurrentRect].rect);
				ReleaseDC (hWnd,hDC);
				pVCRect += pVCHeader->iCurrentRect;
				curviewport = pVCRect->viewport;
				curlayer = pVCRect->layer;
				cursymbol = pVCRect->symnum;

				switch (pVCRect->type)
				{
				case 1:	//expandbox
					switch (pVCRect->level)
					{
					case 0:	//viewport
						if (pVCHeader->ExpandViewport == 1)
							pVCHeader->ExpandViewport = 2;
						else
							pVCHeader->ExpandViewport = 1;
						if (nClicks == 2)
							for (ilayer=0;ilayer<pVCHeader->nLayers;ilayer++)
							{
								pVCHeader->ExpandLayer[ilayer] = pVCHeader->ExpandViewport;
								ClickExpandBox (nClicks,ilayer,1,0,pVCHeader->ExpandViewport);
							}
						DestroySavedScreen (&pWMH->hSaveScreen,0);
	     				InvalidateRect (hWnd,0,TRUE);
						break;
					case 1:	//layer
						if (pVCHeader->ExpandLayer[pVCRect->layer] == 1)
							pVCHeader->ExpandLayer[pVCRect->layer] = 2;
						else
							pVCHeader->ExpandLayer[pVCRect->layer] = 1;
						if (nClicks == 2)
							ClickExpandBox (nClicks,pVCRect->layer,1,0,pVCHeader->ExpandLayer[pVCRect->layer]);
						DestroySavedScreen (&pWMH->hSaveScreen,0);
	     				InvalidateRect (hWnd,0,TRUE);
						break;
					case 3:	//parent symbol
						pSym = (LPVCSYMBOL)GlobalLock (pVCHeader->hLayerSyms[pVCRect->layer]);
						for (i=0;i<pVCHeader->nLayerSyms[pVCRect->layer];i++,pSym++)
							if (pSym->symnum == pVCRect->symnum)
							{
								int expand=1;
								
								if (pSym->expand == 1)
									expand = 2;
								ClickExpandBox (nClicks,pVCRect->layer,pSym->level,pSym->symnum,expand);
								DestroySavedScreen (&pWMH->hSaveScreen,0);
			     				InvalidateRect (hWnd,0,TRUE); 
								break;
							}
						GlobalUnlock (pVCHeader->hLayerSyms[pVCRect->layer]);
						break;
					}
					break;
				case 2:	//onoffbox
 					switch (pVCRect->level)
					{
					case 0:	//viewport
						if (CurView->Active)
							status = 0;
						else
							status = 1;
						CurView->Active = status;
						if (nClicks == 2)
							for (ilayer = 0; ilayer<pVCHeader->nLayers; ilayer++)
							{
								CurVis->FileIsVisible[ilayer] = status;
								ClickVisBox (nClicks,ilayer,1,0,status);
							}
						break;
					case 1:	//layer
						if (CurVis->FileIsVisible[pVCRect->layer])
							status = FALSE;
						else
							status = TRUE;
						CurVis->FileIsVisible[pVCRect->layer] = status;
						if (nClicks == 2)
							SetLayerVis (pVCHeader->nLayerSyms[pVCRect->layer],pVCHeader->hLayerSyms[pVCRect->layer],status);
						break;
					case 2:	//symbol
						ToggleVisibility (pVCRect->symnum);
						break;
					case 3:	//parent symbol
						if (GetVisibilityOfParentSymbol (pVCRect->symnum,pVCHeader->nLayerSyms[pVCRect->layer],pVCHeader->hLayerSyms[pVCRect->layer]))
							status = 0;
						else
							status = 1;
						SetVisibilityOfParentSymbol (pVCRect->symnum,pVCHeader->nLayerSyms[pVCRect->layer],pVCHeader->hLayerSyms[pVCRect->layer],status);

						break;
					case 7: //Pick same as vis
						if (!CurView->pPickList1)
				 		{
				      		CopyVisListToPickList ();  
				      		if (CurView->pPickListManual)
				      			CurVis =  CurView->pPickListManual;
				      		else
		            			CurVis = CurView->pPickList1;
							pVCHeader->PickIsSame = FALSE;
						}
						else
						{
							SetPickSame (CurView);
							pVCHeader->PickIsSame = TRUE;
						}
						break;
					}
 					DestroySavedScreen (&pWMH->hSaveScreen,0);
   					InvalidateRect (hWnd,0,TRUE); 
					if (pVCHeader->AutoRedraw)
						RedisplayWindow ();

					break;
				case 3:	//text
					{
						if (Message == WM_RBUTTONDOWN)
							EditVCText (hWnd,pVCRect->level);
						else /*if (pVCRect->level > 1)
						{
							char	VCSelMacro[]="[%VCSELMACRO]";

							SetGlobalValueLong ("%VCSELLAYER",pVCRect->layer);
							SetGlobalValueLong ("%VCSELSYMNUM",pVCRect->symnum);
							ProcessText (VCSelMacro);
						}*/
						{
							HMENU	EditMenu=CreatePopupMenu(),hModMenu=CreateMenu ();   
							POINT	position;

							ResetShowOnlyVis ();
							ShowOnlyThisViewport = pVCRect->viewport;
							ShowOnlyThisLayer = pVCRect->layer;
							ShowOnlyThisLevel = pVCRect->level;
							ShowOnlyThisSymbol = pVCRect->symnum;
							pVCHeader->iCurrentRect = -1;
							if (ShowOnlyThisLayer)
							{
								AppendMenu (EditMenu,MF_ENABLED|MF_STRING,65002,"Show only this");
								AppendMenu (EditMenu,MF_ENABLED|MF_STRING,65005,"Change color and width");
							}
							else
								AppendMenu (EditMenu,MF_ENABLED|MF_STRING,65004,"Set background color");
							AppendMenu (EditMenu,MF_ENABLED|MF_STRING,65003,"Cancel");
							GetCursorPos (&position);
							InTrackMenu = TRUE;
							TrackPopupMenu (EditMenu,TPM_LEFTBUTTON|TPM_CENTERALIGN|TPM_VCENTERALIGN,position.x,position.y,0,hWnd,0);
							InTrackMenu = FALSE;
							DestroyMenu (EditMenu); 
						}
					}
					break;

				case 6: // symbol
					{
						HMENU	EditMenu=CreatePopupMenu(),hModMenu=CreateMenu ();   
						POINT	position;

						pVCHeader->iCurrentRect = -1;
						AppendMenu (hModMenu,MF_ENABLED|MF_STRING,IDM_INTENSITYMOD,"Add Intensity Modifier");
						AppendMenu (hModMenu,MF_ENABLED|MF_STRING,IDM_COLORMOD,"Add Color Modifier");
						AppendMenu (hModMenu,MF_ENABLED|MF_STRING,IDM_SIZEMOD,"Add Size Modifier");
						AppendMenu (hModMenu,MF_ENABLED|MF_STRING,IDM_RANDOMMOD,"Add Random Colors Modifier");
						AppendMenu (EditMenu,MF_ENABLED|MF_POPUP,(UINT)hModMenu,"Add Modifiers");
						AppendMenu (EditMenu,MF_ENABLED|MF_STRING,65003,"Cancel");
						GetCursorPos (&position);
						InTrackMenu = TRUE;
						TrackPopupMenu (EditMenu,TPM_LEFTBUTTON|TPM_CENTERALIGN|TPM_VCENTERALIGN,position.x,position.y,0,hWnd,0);
						InTrackMenu = FALSE;
						DestroyMenu (EditMenu); 
					}
					break;

				case 5:	//command
					switch (pVCRect->level)
					{
					case 1://redisplay
						RedisplayWindow ();
						break;
					case 2://float
/*						{
							HANDLE	hMem =  GSSiGlobAlloc (1683,GHND,sizeof(VISCONTROLHEADER)+MAXMODSLEN);
							LPVISCONTROLHEADER	pHead=GlobalLock (hMem);

							*pHead = *pVCHeader;
							pHead->hRectangles = 0;
							for (ilayer=0;ilayer<pVCHeader->nLayers;ilayer++)
							{
								pVCHeader->hLayerSyms[ilayer] = 0;
								pVCHeader->hSymOrder[ilayer] = 0;
							}
							if (pVCHeader->hModifiers)
							{
								LPBYTE	pModsTo = (LPBYTE)(pHead + 1);
								LPBYTE	pModsFrom = (LPBYTE)GlobalLock (pVCHeader->hModifiers);

								memmove (pModsTo,pModsFrom,strlen(pModsFrom));
								GlobalUnlock (pVCHeader->hModifiers);
							}
							GlobalUnlock (hMem);
							PostMessage (hWnd,WM_CLOSE,1,(LPARAM)hMem);
						}*/
						{
/*							HANDLE	hMem =  GSSiGlobAlloc (1683,GHND,sizeof(WINDOWMENUHEADER));
							LPWINDOWMENUHEADER	pHead=GlobalLock (hMem);

							*pHead = *pWMH;
							GlobalUnlock (hMem);*/
							pWMH->Float = TRUE;
							pWMH->isDocked = 0;
							//PostMessage (pWMH->hWndMenu,WM_CLOSE,1,(LPARAM)hWMH);
							PostMessage (pWMH->hWndMenu,WM_CLOSE,0,0);
						}
						break;

					case -2:	//dock
							pWMH->Float = FALSE;
							PostMessage (pWMH->hWndMenu,WM_CLOSE,0,(LPARAM)hWMH);
							break;

					case 3: //options
						{
							HMENU	EditMenu=CreatePopupMenu();   
							POINT	position;

							pVCHeader->iCurrentRect = -1;
					/*		if (Message == WM_RBUTTONDOWN)
							{
								AppendMenu (EditMenu,MF_ENABLED|MF_STRING,IDM_SETFACTOR,"Set size factor");
								AppendMenu (EditMenu,MF_ENABLED|MF_STRING,IDM_SETBUTTONH,"Set button height");
								AppendMenu (EditMenu,MF_ENABLED|MF_STRING,IDM_SETALPHABLEND,"Set alpha blend factor");
								AppendMenu (EditMenu,MF_ENABLED|MF_STRING,IDM_SETFONTCMD,"Set command font");
								AppendMenu (EditMenu,MF_ENABLED|MF_STRING,IDM_SETBKCOLOR,"Set window background color");
								AppendMenu (EditMenu,MF_ENABLED|MF_STRING,IDM_SETBKCOLORCMD,"Set command button background color");
							}
							else*/
							{
								if (pVCHeader->AutoRedraw)
									AppendMenu (EditMenu,MF_ENABLED|MF_STRING|MF_CHECKED,IDM_AUTOREDRAW,"Auto redraw");
								else
									AppendMenu (EditMenu,MF_ENABLED|MF_STRING,IDM_AUTOREDRAW,"Auto redraw");
								if (pVCHeader->DisplaySymbols)
									AppendMenu (EditMenu,MF_ENABLED|MF_STRING|MF_CHECKED,IDM_DISPLAYSYMBOLS,"Display symbols");
								else
									AppendMenu (EditMenu,MF_ENABLED|MF_STRING,IDM_DISPLAYSYMBOLS,"Display symbols");
								if (pVCHeader->DisplayModifiers)
									AppendMenu (EditMenu,MF_ENABLED|MF_STRING|MF_CHECKED,IDM_DISPLAYMODIFIERS,"Display modifiers");
								else
									AppendMenu (EditMenu,MF_ENABLED|MF_STRING,IDM_DISPLAYMODIFIERS,"Display modifiers");
							}
							AppendMenu (EditMenu,MF_SEPARATOR,0,0);
							AppendMenu (EditMenu,MF_ENABLED|MF_STRING,65003,"Cancel");
							GetCursorPos (&position);
							InTrackMenu = TRUE;
							TrackPopupMenu (EditMenu,TPM_LEFTBUTTON|TPM_CENTERALIGN|TPM_VCENTERALIGN,position.x,position.y,0,pWMH->hWndDisplay,0);
							InTrackMenu = FALSE;
							DestroyMenu (EditMenu); 
						}
						break;
					}
					break;
				case 7:	//modifier
					switch (pVCRect->level)
					{
						case IDM_INTENSITYMOD:
							GlobalUnlock (pVCHeader->hRectangles);
							if (DialogBox(hInst,"MODIFIER_COLORINTENSITY", hWnd, (DLGPROC)COLORINTENSITYMsgProc))
							{
							}
							DestroySavedScreen (&pWMH->hSaveScreen,0);
		     		  		InvalidateRect (pWMH->hWndMenu,0,TRUE);
							goto SkipUnlock;
					}
					break;

				}
				GlobalUnlock (pVCHeader->hRectangles);
SkipUnlock:;
			}
		}
		break; 

/*	case WM_NCHITTEST:
		{
			rtn = DefWindowProc(hWnd, Message, wParam, lParam);
			if (HaveMoving)
			{
		   		POINT	CursorPoint = POINTStoPOINT(MAKEPOINTS (lParam)); 

				HaveMoving = FALSE;
				if (PtInRect (&pWMH->FixedRect,CursorPoint))
				{
					PostMessage (pWMH->hWndMenu,WM_CLOSE,0,(LPARAM)hWMH);
				}
			}
			goto Exit;
		}
		break;
*/
	case WM_MBUTTONUP:
		break;
	case WM_MOUSELEAVE:
		hDC = GetDC (hWndMain);
		ClearMeterPrompts (hDC);
		ReleaseDC (hWndMain,hDC);
		hDC = GetDC (hWnd);
		if (pVCHeader->hRectangles)
		{
			LPVCRECTANGLE	pVCRect = GlobalLock (pVCHeader->hRectangles);

			if (pVCHeader->iCurrentRect > -1)
				InvertRectBorder (hDC,&pVCRect[pVCHeader->iCurrentRect].rect);
			pVCHeader->iCurrentRect = -1;
			GlobalUnlock (pVCHeader->hRectangles);
		}
		ReleaseDC (hWnd,hDC);
		break;
				
	case WM_MOUSEMOVE:
		{
    		POINT	MovePoint = POINTStoPOINT(MAKEPOINTS (lParam)); 
			if (HaveTimer)
				break;
			if (hWnd != pWMH->hWndDisplay)
				break;

			hDC = GetDC (hWndMain);
			ClearMeterPrompts (hDC);
			ReleaseDC (hWndMain,hDC);
			hDC = GetDC (hWnd);
			if (pVCHeader->hRectangles)
			{
				LPVCRECTANGLE	pVCRect = GlobalLock (pVCHeader->hRectangles);

				for (i=0;i<pVCHeader->nRectangles;i++)
				{
					if (PtInRect (&pVCRect[i].rect,MovePoint))
					{
						if (i != pVCHeader->iCurrentRect)
						{
							if (pVCHeader->iCurrentRect > -1)
								InvertRectBorder (hDC,&pVCRect[pVCHeader->iCurrentRect].rect);
							InvertRectBorder (hDC,&pVCRect[i].rect);
						}
						pVCHeader->iCurrentRect = i;
						break;
					}
				}
				if (i == pVCHeader->nRectangles) //not in rect
				{
					if (pVCHeader->iCurrentRect > -1)
						InvertRectBorder (hDC,&pVCRect[pVCHeader->iCurrentRect].rect);
					pVCHeader->iCurrentRect = -1;
					SetSysMess ("");
				}
				else
				{
					*mess = 0;
					switch (pVCRect[pVCHeader->iCurrentRect].type)
					{
						case 2:
						{
							char	SymType[8]="Parent";
							int	On = GetOnValue (pVCRect[pVCHeader->iCurrentRect].layer,pVCRect[pVCHeader->iCurrentRect].symnum,pVCRect[pVCHeader->iCurrentRect].level,pVCHeader->nLayerSyms[pVCRect[pVCHeader->iCurrentRect].layer],pVCHeader->hLayerSyms[pVCRect[pVCHeader->iCurrentRect].layer]);
							char	SymName[66],ParName[66],SymDesc[64];

							switch (pVCRect[pVCHeader->iCurrentRect].level)
							{
								case 0:
									if (LOWORD(On))
										sprintf (mess,"Single click to turn off viewport '%s' or double click to turn off viewport '%s' and all its layers and symbols ",CurView->Name,CurView->Name);
									else
										sprintf (mess,"Single click to turn on viewport '%s' or double click to turn on viewport '%s' and all its layers and symbols ",CurView->Name,CurView->Name);
									break;
								case 1:
									if (LOWORD(On))
										sprintf (mess,"Single click to turn off layer '%s' or double click to turn off layer '%s' and all its symbols",CurView->FileID[pVCRect[pVCHeader->iCurrentRect].layer],CurView->FileID[pVCRect[pVCHeader->iCurrentRect].layer]);
									else
										sprintf (mess,"Single click to turn on layer '%s' or double click to turn on layer '%s' and all its symbols",CurView->FileID[pVCRect[pVCHeader->iCurrentRect].layer],CurView->FileID[pVCRect[pVCHeader->iCurrentRect].layer]);
									break;
								case 2:
									GetDictSymName (pVCRect[pVCHeader->iCurrentRect].symnum,SymName);
									if (LOWORD(On))
										sprintf (mess,"Single click to turn off symbol '%s'",SymName);
									else
										sprintf (mess,"Single click to turn on symbol '%s'",SymName);
									break;
								case 3:
									GetDictSymName (pVCRect[pVCHeader->iCurrentRect].symnum,SymName);
									if (LOWORD(On))
										sprintf (mess,"Single click to turn off parent symbol '%s' and all its children",SymName);
									else
										sprintf (mess,"Single click to turn on parent symbol '%s' and all its children",SymName);
									break;
							}
						}
						break;
						case 3:
						{
							char	SymType[8]="Parent";

							switch (pVCRect[pVCHeader->iCurrentRect].level)
							{
							case 0:
								sprintf (mess,"Viewport:%s",CurView->Name);
								break;
							case 1:
								sprintf (mess,"Layer:%s",CurView->FileID[pVCRect[pVCHeader->iCurrentRect].layer]);
								break;
							case 2:
								strcpy (SymType,"Symbol");
							case 3:
								{
									char	SymName[66],ParName[66],SymDesc[64],LayerName[64];

									strcpy (LayerName,CurView->FileID[pVCRect[pVCHeader->iCurrentRect].fromlayer]);
									GetDictSymName (pVCRect[pVCHeader->iCurrentRect].symnum,SymName);
									GetDictSymName (GetDictSymParent(pVCRect[pVCHeader->iCurrentRect].symnum),ParName);
									GetDictSymDescription (pVCRect[pVCHeader->iCurrentRect].symnum,SymDesc);
									sprintf (mess,"%s:%s (%s) under parent %s in layer %s",SymType,SymName,SymDesc,ParName,LayerName);
								}
								break;
							}
						}
						break;
					
						case 5:
						{
							switch (pVCRect[pVCHeader->iCurrentRect].level)
							{
							case 1:
								sprintf (mess,"Click here to redraw the '%s' viewport",CurView->Name);
								break;
							case 2:
								sprintf (mess,"Click here to allow this menu to float");
								break;
							case -2:
								sprintf (mess,"Click here to dock this menu");
								break;
							case 3:
								sprintf (mess,"Click here to display the options menu");
								break;
							}
							break;
						}
						break;

						case 6:
							sprintf (mess,"Click here to add display modifiers");
						break;

						default:
							sprintf (mess,"type %i prompt not defined",pVCRect[pVCHeader->iCurrentRect].type);
						break;
					}
					SetSysMess (mess);

				}
				GlobalUnlock (pVCHeader->hRectangles);
			}
			ReleaseDC (hWnd,hDC);
		}
		break;

	case WM_ERASEBKGND:
		{
			RECT	Rect;

			hDC = GetDC (hWnd);
			GetClientRect (hWnd,&Rect);
			FillRectPoly (hDC,&Rect,pWMH->BackgroundColor);
			ReleaseDC (hWnd,hDC);
		}
		break;


	case WM_MOUSEWHEEL:
		{
			int	nPos = GetScrollPos (hWnd,SB_VERT);
			int	inc  = (short) HIWORD(wParam);    

			if (!VScrollIsVisPrevious)
				break;
			nPos -= inc;
			SetScrollPos (hWnd,SB_VERT,nPos,TRUE);
 			DestroySavedScreen (&pWMH->hSaveScreen,0);
    		InvalidateRect (hWnd,0,TRUE);

		}
		break;


    case WM_PAINT:    /* code for the window's client area              */
         /* Obtain a handle to the device context                       */
         /* BeginPaint will sends WM_ERASEBKGND if appropriate          */
		{
			PAINTSTRUCT	ps;
			RECT	clientrect;
			BOOL	VScrollIsVis;
			BOOL	DoRedisplay = FALSE, DidDisplay=FALSE;
			POINT	Point;
			SIZE	size;
			int		x,y,ystart, maxrectright=0;

			if (hWnd != pWMH->hWndDisplay || !pVCHeader)
				goto ExitDefault;
			GetClientRect (hWnd,&clientrect);

			memset(&ps, 0x00, sizeof(PAINTSTRUCT));
			hDC = BeginPaint(hWnd, &ps);
//			if (ps.fErase || !IsRectEmpty (&ps.rcPaint))
			{
				DidDisplay = TRUE;
				if (ps.fErase)
					FillRectPoly (hDC,&ps.rcPaint,pWMH->BackgroundColor);
				size = DisplayCatalog (hWnd,hDC,&ystart);
				VScrollIsVis = GetMaxVCRectBottom() > clientrect.bottom || ystart < 0;
				ShowScrollBar (hWnd,SB_VERT,VScrollIsVis);
				if (VScrollIsVisPrevious && !VScrollIsVis)
					DoRedisplay = TRUE;
				VScrollIsVisPrevious = VScrollIsVis;
				ShowScrollBar (hWnd,SB_HORZ,!pWMH->Float); 
				SetScrollRange (hWnd,SB_VERT,0,size.cy,TRUE);
				GetCursorPos (&Point);
				SetCursorPos (Point.x,Point.y);
				GetClientRect (hWnd,&clientrect);
				x=y=3;
				DrawCmdButton (hDC,"Options",3,&x,&y,clientrect.right);
				//if (!pVCHeader->AutoRedraw)
				{
					x += 6;
					y = 3;
					DrawCmdButton (hDC,"Redraw",1,&x,&y,clientrect.right);
				}
				maxrectright = GetMaxVCRectRight ();
				if (pWMH->Float)
				{
					x = -4;
					y = 3;
					//DrawCmdButton (hDC,"Dock",-2,&x,&y,clientrect.right);
				}
				else
				{
					x = -4;
					y = 3;
					DrawCmdButton (hDC,"Close",2,&x,&y,clientrect.right);
				}
			}
			EndPaint(hWnd, &ps);
			GetClientRect (hWnd,&clientrect);
			if (!fromScroll && DidDisplay && pWMH->Float && abs (clientrect.right - maxrectright) > 32)
			{
				RECT	winRect;

				GetWindowRect (pWMH->hWndMenu,&winRect);
				SetWindowPos(pWMH->hWndMenu, 0, winRect.left,winRect.top,RECTWIDTH(&winRect)+(maxrectright-clientrect.right),RECTHEIGHT(&winRect),
					SWP_DRAWFRAME|SWP_NOZORDER|SWP_SHOWWINDOW);
				PostMessage (pWMH->hWndMenu,WM_CLOSE,2,(LPARAM)hWMH);
			}
		/*	else if (FirstDisplay)
			{
				PostMessage (hWnd,WM_CLOSE,2,(LPARAM)hWMH);
				FirstDisplay =FALSE;
			}*/
			else if (DoRedisplay)
			{
				DestroySavedScreen (&pWMH->hSaveScreen,0);
				InvalidateRect (hWnd,0,TRUE);
			}
			else
				ShowWindow (pWMH->hWndMenu,SW_SHOW);
			fromScroll = FALSE;
		}
         break;       /*  End of WM_PAINT                               */

    case WM_COMMAND:
    {         
         switch(LOWORD(wParam))
    	{
		 case IDM_AUTOREDRAW:
			pVCHeader->AutoRedraw = !pVCHeader->AutoRedraw;
			break;
		 case IDM_DISPLAYSYMBOLS:
			pVCHeader->DisplaySymbols = !pVCHeader->DisplaySymbols;
			break;
		 case IDM_DISPLAYMODIFIERS:
			pVCHeader->DisplayModifiers = !pVCHeader->DisplayModifiers;
			break;
		 case IDM_INTENSITYMOD:
			AddModifier (curviewport,curlayer,cursymbol,IDM_INTENSITYMOD);
			break;
		 case IDM_COLORMOD:
			AddModifier (curviewport,curlayer,cursymbol,IDM_COLORMOD);
			break;
		 case IDM_RANDOMMOD:
			AddModifier (curviewport,curlayer,cursymbol,IDM_RANDOMMOD);
			break;
		 case IDM_SIZEMOD:
			AddModifier (curviewport,curlayer,cursymbol,IDM_SIZEMOD);
			break;
		 case 65004:
			 {
				 COLORREF Color=CurView->BackGroundColor;
			 
				 SetViewport (ShowOnlyThisViewport);
				 if (GetColor (hWnd,&Color))
				 {
					CurView->BackGroundColor = Color;
					RedisplayWindow ();
				 }
			 }
			 break;

		 case 65005:
			 {
				NumPicked = 1;
				PickList[0].Desc = ShowOnlyThisSymbol;
				PickList[0].Type = GetDictSymbolType (ShowOnlyThisSymbol); 
				SetViewport (ShowOnlyThisViewport);
				sprintf (mess, "$RESET(%s,ALL);$CMD(GF_LINE_TYPE_CLASS(P),%s)",CurView->Name,CurView->Name);
				ProcessText (mess);
			 }
			 break;

		 case 65002:
			
			ShowOnlyThisSaveVis = *CurVis;
			HaveShowOnlyVis = TRUE;
			InitVis ();
			memset (CurVis->VisBits,0,sizeof(CurVis->VisBits));
			memset (CurVis->FileIsVisible,0,sizeof(CurVis->FileIsVisible));
			CurVis->FileIsVisible[ShowOnlyThisLayer] = TRUE;
			if (ShowOnlyThisLevel == 2)
				ToggleVisibility (ShowOnlyThisSymbol);
			else if (ShowOnlyThisLevel == 3)
			//	SetVisibilityOfParentSymbol (ShowOnlyThisSymbol,pVCHeader->nLayerSyms[pVCRect->layer],pVCHeader->hLayerSyms[pVCRect->layer],1);
				SetParentVisibility (ShowOnlyThisSymbol,1,-1); 
			TurnOffAutoVis (TRUE);
			RedisplayWindow ();

			break;
        }
		DestroySavedScreen (&pWMH->hSaveScreen,0);
		InvalidateRect (hWnd,0,TRUE);
        break;   	 
    			
    }
    	break;
    default:
         /* For any message for which you don't specifically provide a  */
         /* service routine, you should return the message to Windows   */
         /* for default message processing  */
//		if (pWMH->menuHandle[pWMH->currentMenu])
//			GlobalUnlock (pWMH->menuHandle[pWMH->currentMenu]);
ExitDefault:
		SetConfig (SaveCfg);
		CurView = SaveVP;
		CurVis = SaveVis;
		GlobalUnlock (pWMH->menuHandle[pWMH->currentMenu]);
		GlobalUnlock (hWMH);
		pVCHeader = savepVCHeader;
		hWMH = savehWMH;
		pWMH = savepWMH;
        return DefWindowProc(hWnd, Message, wParam, lParam);
   } 
Exit:
    GlobalUnlock (pWMH->menuHandle[pWMH->currentMenu]);
	GlobalUnlock (hWMH);
	pVCHeader = savepVCHeader;
	hWMH = savehWMH;
	pWMH = savepWMH;
	SetConfig (SaveCfg);
	CurView = SaveVP;
	CurVis = SaveVis;
//	if (pWMH->menuHandle[pWMH->currentMenu])
//		GlobalUnlock (pWMH->menuHandle[pWMH->currentMenu]);
	return rtn;
}                               


BOOL RegisterFloatMenuClass(BOOL UnRegister)
{
    WNDCLASS  wc; 
    static	Called=FALSE;

	if (UnRegister)
	{
		 if (Called)
		 {
			 UnregisterClass(MENUWINDOWCLASS, hInst);
			 Called = FALSE;
		 }
		 return TRUE;
	}
    if (Called) return TRUE;
    Called = TRUE;
    wc.style = CS_OWNDC|CS_DBLCLKS;
    wc.lpfnWndProc = (WNDPROC)FloatMenuWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_HAND);
    wc.hbrBackground = 0;
    wc.lpszMenuName =  NULL;
    wc.lpszClassName = MENUWINDOWCLASS;

    return (RegisterClass(&wc));
}

int DrawOnOffRadioButton (HDC hDC,int level,int viewport,int layer,int symnum,int x,int y,int height,int On)
{  
	RECT	Rect; 
	HPEN	hPen, hSavePen, hOldPen;
	HBRUSH	hBrush, hOldBrush=0;
	int		i;
	COLORREF	Color, PenColor=0;
	
	if (!HIWORD(On))
		PenColor = RGB(255,0,0);
//	Rect.bottom = y - 5;  
//	Rect.top = Rect.bottom - (height/2 + 1);
//	Rect.right = x - 2;
//	Rect.left = Rect.right - (height/2 + 1);

 	Rect.top = y + height/8;
	Rect.right = x - 3;
	Rect.bottom = y + (height - height/4);  
	Rect.left = Rect.right - (height - height/3); 
	AddVCRect (&Rect,level,viewport,layer,symnum,2,TRUE);
	if (hDC)
	{
		//FillRect (hDC,&Rect,GetStockObject(LTGRAY_BRUSH));

		switch (LOWORD (On))
		{
		case 1://all on
			if (HIWORD(On))
				Color = 0;
			else
				Color = RGB (255,127,127);
			InflateRect (&Rect,-height/8,-height/8);
			hBrush = CreateSolidBrush (Color);
			hOldBrush = SelectObject (hDC,hBrush);
			RoundRect (hDC,Rect.left,Rect.top,Rect.right,Rect.bottom,height-height/8,height-height/8);
			InflateRect (&Rect,height/8,height/8);
		break;
		case 2://partial on
			hBrush = CreateSolidBrush (RGB(204,204,204));
			hOldBrush = SelectObject (hDC,hBrush);
			RoundRect (hDC,Rect.left,Rect.top,Rect.right,Rect.bottom,height,height);
		break;
		case 3://layer on but no symbols on
			hBrush = CreateSolidBrush (RGB(220,204,204));
			hOldBrush = SelectObject (hDC,hBrush);
			RoundRect (hDC,Rect.left,Rect.top,Rect.right,Rect.bottom,height,height);
		break;
		}
		hPen = CreatePen(PS_SOLID,1,PenColor);
		hOldPen = SelectObject (hDC,hPen);
		if (hOldBrush)
		{
			SelectObject (hDC,hOldBrush);
			DeleteObject (hBrush);
		}
		hOldBrush = SelectObject (hDC,GetStockObject (NULL_BRUSH));
		RoundRect (hDC,Rect.left,Rect.top,Rect.right,Rect.bottom,height,height);
		SelectObject (hDC,hOldBrush);
		SelectObject (hDC,hOldPen);
		DeleteObject (hPen);
	}
	return Rect.left;
} 

BOOL DrawExpandCheckBox (HDC hDC,int level,int viewport,int layer,int symnum,int x,int y,int height,int IsExpanded)
{  
	RECT	Rect=CurView->Rect; 
	HPEN	hSavePen, hBlackPenDW = CreatePen (PS_SOLID,1,0);
	
	if (!hDC)
		return TRUE;
	Rect.bottom = y - 3;  
	Rect.top = Rect.bottom - (3*height)/4;
	Rect.right = x - 2;
	Rect.left = Rect.right - (3*height)/4;
	//FillRect (hDC,&Rect,GetStockObject(LTGRAY_BRUSH));
	FrameRect (hDC,&Rect,GetStockObject(BLACK_BRUSH));
	AddVCRect (&Rect,level,viewport,layer,symnum,1,TRUE);
	
	hSavePen = SelectObject (hDC,hBlackPenDW);
	if (IsExpanded == 1)
	{
		MoveToEx (hDC, (Rect.left+Rect.right)/2, Rect.top+2,0);
		LineTo (hDC, (Rect.left+Rect.right)/2,Rect.bottom-2);
	}
	MoveToEx (hDC, Rect.left+2, (Rect.bottom+Rect.top)/2,0);
	LineTo (hDC, Rect.right-2,  (Rect.bottom+Rect.top)/2);
	SelectObject (hDC,hSavePen);
	GSSiDeleteObject(&hBlackPenDW);
	
//	Draw3DBorder(hDC, &Rect,-UP_3D, FALSE); 
	return TRUE;
} 

BOOL DrawCommandCheckBox (HDC hDC,int viewport,int layer,int symnum,int type,LPSTR txt,int x,int y,int height,int IsChecked)
{  
	RECT	Rect=CurView->Rect; 
	HPEN	hSavePen, hBlackPenDW = CreatePen (PS_SOLID,1,0);
	
	if (!hDC)
		return TRUE;
	Rect.bottom = y - 5;  
	Rect.top = Rect.bottom - (height/2 + 1);
	Rect.right = x - 2;
	Rect.left = Rect.right - (height/2 + 1);
	//FillRect (hDC,&Rect,GetStockObject(LTGRAY_BRUSH));
	FrameRect (hDC,&Rect,GetStockObject(BLACK_BRUSH));
	AddVCRect (&Rect,0,viewport,layer,symnum,1,TRUE);
	
	hSavePen = SelectObject (hDC,hBlackPenDW);
	if (IsChecked == 1)
	{
		MoveToEx (hDC, (Rect.left+Rect.right)/2, Rect.top+2,0);
		LineTo (hDC, (Rect.left+Rect.right)/2,Rect.bottom-2);
	}
	MoveToEx (hDC, Rect.left+2, (Rect.bottom+Rect.top)/2,0);
	LineTo (hDC, Rect.right-2,  (Rect.bottom+Rect.top)/2);
	TextOut (hDC,x+height+2,y,txt,strlen(txt));
	SelectObject (hDC,hSavePen);
	GSSiDeleteObject(&hBlackPenDW);
	
//	Draw3DBorder(hDC, &Rect,-UP_3D, FALSE); 
	return TRUE;
} 

int DrawSymbolInBox (HDC hDC,int level,int viewport,int layer,int symnum,int x,int y,int height)
{  
	RECT	Rect=CurView->Rect; 
			
	Rect.bottom = y;  
	Rect.top = Rect.bottom - height;
	Rect.left = x;
	Rect.right = Rect.left + height*2;
	//FillRect (hDC,&Rect,GetStockObject(LTGRAY_BRUSH));
	//FrameRect (hDC,&Rect,GetStockObject(LTGRAY_BRUSH));
	AddVCRect (&Rect,level,viewport,layer,symnum,6,FALSE);
	DisplaySymInRect (hDC,symnum,Rect,1,FALSE);
	
//	Draw3DBorder(hDC, &Rect,-UP_3D, FALSE); 
	return Rect.right+1;
} 

BOOL DrawVisButton (HDC hDC,LPSTR inID,int type,int viewport,int layer,int symnum,LPINT px,LPINT py,int On,int Expand)
{
	HBRUSH	hBrush=0;
	RECT	rect, saverect;
	HFONT	hFont, hOldFont=0;
	int		left, right, w;
	int		modnum=0;
	LPSTR	pParms;
	char	ID[256];

	if ((pWMH->menuType[pWMH->currentMenu] == MT_VISIBLE || pWMH->menuType[pWMH->currentMenu] == MT_VISSYMBOLS) && (!HIWORD (On) || !LOWORD (On)))
		return FALSE;
	if ((pWMH->menuType[pWMH->currentMenu] == MT_ALLSYMBOLS || pWMH->menuType[pWMH->currentMenu] == MT_VISSYMBOLS) && type != 2)
		return FALSE;
	strcpy (ID,inID);
	ExpandText (ID);
	rect.left = *px;
	rect.top  = *py;
	rect.bottom = *py + buttonheight;
	rect.right = *px + buttonwidth;
	(*py) += buttonheight + 3;

	if (rect.top < menuy)
		return FALSE;
	saverect = rect;
	right = rect.right;
	if (pVCHeader->DisplaySymbols && type == 2)
	{
		right = DrawSymbolInBox (hDC,type,viewport,layer,symnum,rect.left,rect.bottom,buttonheight);
		w = RECTWIDTH (&rect);
		rect.left = right;
		rect.right = rect.left + w;
	}
	if (hDC)
	{
		hBrush = CreateSolidBrush (TypeColorsBk[type]);

		hFont  = CreateFont(-IDNINT((2*buttonheight)/4+TypeFontSizeInc[type]*pWMH->Factor), 0, 0, 0, TypeFontWidth[type],0, TypeFontUnderline[type], 0, 0, 0, 0, 0, 0,TypeFontName[type]);   
		hOldFont = SelectObject (hDC,hFont);
		SetTextColor (hDC,TypeColors[type]);
		DrawText(hDC, ID, -1, &rect, DT_WORDBREAK | DT_LEFT | DT_VCENTER |DT_SINGLELINE |DT_NOCLIP |DT_CALCRECT );
		if (TypeColorsBk[type])
			FillRect (hDC,&rect,hBrush);
		SetBkMode (hDC,TRANSPARENT);
		DrawText(hDC, ID, -1, &rect, DT_WORDBREAK | DT_LEFT | DT_VCENTER |DT_SINGLELINE |DT_NOCLIP );
	}
	AddVCRect (&rect,type,viewport,layer,symnum,3,FALSE);
	if (hDC)
	{
		SelectObject (hDC,hOldFont);
		DeleteObject (hFont);
	}
	rect = saverect;
	left = rect.left;
	left = DrawOnOffRadioButton (hDC,type,viewport,layer,symnum,left,rect.top,buttonheight, On);
	if (Expand)
		DrawExpandCheckBox (hDC,type,viewport,layer,symnum,left,rect.bottom,buttonheight,Expand);
	//DrawFocusRect(hDC, &rect);
	GSSiDeleteObject (&hBrush);
	if (pVCHeader->DisplayModifiers)
		while ((type = GetModifier (viewport,layer,symnum,modnum,&pParms)))
		{
			DisplayModifier (hDC,viewport,layer,symnum,modnum,type,rect.left,py,pParms);
			modnum++;
		}

	return TRUE;
}
BOOL DrawCmdButton (HDC hDC,LPSTR ID,int cmdid,LPINT px,LPINT py,int windowright)
{
	HBRUSH	hBrush=CreateSolidBrush (CommandButtonBackgroundColor);
	RECT	rect, txrect;
	HFONT	hFont, hOldFont;

	rect.left = *px;
	rect.top  = *py;
	rect.bottom = *py + buttonheight;
	rect.right = *px + buttonwidth;
	(*py) += buttonheight + 3;

	hFont = CreateFont(-IDNINT((2*buttonheight)/4+TypeFontSizeInc[4]*pWMH->Factor), 0, 0, 0, TypeFontWidth[4],0, TypeFontUnderline[4], 0, 0, 0, 0, 0, 0,TypeFontName[4]);   
	hOldFont = SelectObject (hDC,hFont);
	SetTextColor (hDC,TypeColors[4]);
	DrawText(hDC, ID, -1, &rect, DT_WORDBREAK | DT_LEFT | DT_VCENTER |DT_SINGLELINE |DT_NOCLIP |DT_CALCRECT );
	if (*px < 0)
	{
		int	width = RECTWIDTH (&rect);
		rect.right = windowright + *px;
		rect.left  = rect.right - width;
	}
	txrect = rect;
	InflateRect (&rect,3,3);
	FillRect (hDC,&rect,hBrush);
	SetBkMode (hDC,TRANSPARENT);
	DrawText(hDC, ID, -1, &txrect, DT_WORDBREAK | DT_LEFT | DT_VCENTER |DT_SINGLELINE |DT_NOCLIP );

	AddVCRect (&rect,cmdid,0,0,0,5,FALSE);
	(*px) = rect.right + 1;
	SelectObject (hDC,hOldFont);
	DeleteObject (hFont);
//	DrawFocusRect(hDC, &rect);
	DeleteObject (hBrush);
	DrawEdge(hDC, &rect,EDGE_ETCHED,BF_RECT);
	return TRUE;
}

BOOL ParentsAreVisible (int symnum)
{
	return TRUE;
}

int GetVisibilityOfParentSymbol (int symnum,int nSyms,HANDLE hSyms)
{
	int rtn=-1, rtn2, i;

	if (nSyms)
	{
		LPVCSYMBOL	pSym  = (LPVCSYMBOL)GlobalLock (hSyms);

		for (i=0;i<nSyms;i++)
			if (pSym[i].parent == symnum)
			{
				if (pSym[i].type == 2)
					rtn2 = GetVisibilityOfParentSymbol (pSym[i].symnum,nSyms,hSyms);
				else
					rtn2 = GetVisibility (pSym[i].symnum);
				if ((!rtn && rtn2) || (rtn > 0 && !rtn2) || rtn2 == 2)
				{
					rtn = 2;
					goto Exit;
				}
				rtn = rtn2;
			}
Exit:
		GlobalUnlock (hSyms);
	}
	return rtn;
}

void SetVisibilityOfParentSymbol (int symnum,int nSyms,HANDLE hSyms,int status)
{
	int i;

	if (nSyms)
	{
		LPVCSYMBOL	pSym  = (LPVCSYMBOL)GlobalLock (hSyms);

		for (i=0;i<nSyms;i++)
			if (pSym[i].parent == symnum)
			{
				if (pSym[i].type == 2)
					SetVisibilityOfParentSymbol (pSym[i].symnum,nSyms,hSyms,status);
				else
					SetVisibility (pSym[i].symnum,status);
			}
		GlobalUnlock (hSyms);
	}
}

int GetOnValue (int layer,int symnum,int type,int nSyms,HANDLE hSyms)
{
	// returns visibility of upper layers in HIWORD, vis of item in LOWORD
	int	hi=1, lo=1;

	switch (type)
	{
	case 0://viewport
		return MAKELONG (CurView->Active,1);
		break;
	case 1://layer
		if (!CurVis->FileIsVisible[layer])
			lo = 0;
//not sure what next 4 lines area for
/*		else if (CurView->FileType[layer] != 1 && CurView->FileType[layer] != 4)
			lo = 1;
		else
			lo = GetVisibilityOfParentSymbol (0,nSyms,hSyms);*/
		return MAKELONG (lo,CurView->Active);
	case 2://symbol
		if (!CurVis->FileIsVisible[layer] || !CurView->Active)
			hi = 0;
		lo = GetVisibility(symnum);
		return MAKELONG (lo,hi);
	case 3://parent
		if (!CurVis->FileIsVisible[layer] || !CurView->Active)
			hi = 0;
		lo = GetVisibilityOfParentSymbol (symnum,nSyms,hSyms);
		return MAKELONG (lo,hi);
	}
	return 0;
}

BOOL DisplayLayerSymbols (HDC hDC,int level,int iparent,int type,int viewport,int layer,LPINT px,LPINT py,int nSyms,HANDLE hSyms,HANDLE hSymOrder)
{
	if (nSyms)
	{
		LPVCSYMBOL	pSym, pSyms  = (LPVCSYMBOL)GlobalLock (hSyms);
		LPWORD	pOrder = GlobalLock (hSymOrder);
		int		isym=0;

		*px += indent;

		while (isym < nSyms)
		{
			pSym = &pSyms[pOrder[isym++]];
			if (pWMH->menuType[pWMH->currentMenu] == MT_ALLSYMBOLS)
			{
				int	On = GetOnValue (pSym->layer,pSym->symnum,pSym->type+1,nSyms,hSyms);
				DrawVisButton (hDC,pSym->Name,pSym->type+1,viewport,layer,pSym->symnum,px,py,On,pSym->expand);
			}
			else if (pWMH->menuType[pWMH->currentMenu] == MT_VISSYMBOLS)
			{
				int	On = GetOnValue (pSym->layer,pSym->symnum,pSym->type+1,nSyms,hSyms);

				if (On)
					DrawVisButton (hDC,pSym->Name,pSym->type+1,viewport,layer,pSym->symnum,px,py,On,pSym->expand);
			}
			else if (pSym->parent == iparent)
			{
				int	On = GetOnValue (layer,pSym->symnum,pSym->type+1,nSyms,hSyms);

				DrawVisButton (hDC,pSym->Name,pSym->type+1,viewport,layer,pSym->symnum,px,py,On,pSym->expand);
				if (pSym->expand == 2)
					DisplayLayerSymbols (hDC,level+1,pSym->symnum,pSym->type,viewport,layer,px,py,nSyms,hSyms,hSymOrder);
			}
		}
		*px -= indent;
		GlobalUnlock (hSyms);
		GlobalUnlock (hSymOrder);
	}
	return TRUE;
}

HWND VisibilityControl (HWND hWnd,HINSTANCE hInst,LPSTR ConnectToVP,LPRECT pRect,double Factor,HANDLE hInit,BOOL First,BOOL Docked,BOOL CheckForDock)
{
	HMENU	hMenu=0;
	HWND	hwndTT;
	RECT	rect;
	LPVIEWPORT	pVP,pSaveVP=CurView;
	int		ii, SaveConfig = CurrentConfig, vpID=0;
	BOOL	Float=!Docked, err;
	LPWINDOWMENUHEADER pWMH;
	static nFirst=0;

	if (!hInit)
	{
		hInit = hInitData= GSSiGlobAlloc (1683,GHND,sizeof(WINDOWMENUHEADER));
		pWMH = (LPWINDOWMENUHEADER)GlobalLock (hInit);
		pWMH->Factor = 0.8;
		pWMH->ScreenStartPoint.x = pRect->left;
		pWMH->ScreenStartPoint.y = pRect->top;
		pWMH->AlphaBlendFactor = 120;
		pWMH->xpad = pWMH->ypad = 1;
		pWMH->BackgroundColor  = 16777215;//RGB(255,255,255);
		if (!nFirst)
		{
			//nFirst++;

			pWMH->menuType[pWMH->nMenus]=MT_CATALOG;
			strcpy (pWMH->menuTitle[pWMH->nMenus++],"Catalog");
			pWMH->menuType[pWMH->nMenus]=MT_VISIBLE;
			strcpy (pWMH->menuTitle[pWMH->nMenus++],"Visible Items");
			pWMH->menuType[pWMH->nMenus]=MT_ALLSYMBOLS;
			strcpy (pWMH->menuTitle[pWMH->nMenus++],"All Symbols");
			pWMH->menuType[pWMH->nMenus]=MT_VISSYMBOLS;
			strcpy (pWMH->menuTitle[pWMH->nMenus++],"Visible Symbols");
			pWMH->menuType[pWMH->nMenus]=MT_AUTOVIS;
			strcpy (pWMH->menuTitle[pWMH->nMenus++],"Auto Visibility");
			pWMH->menuType[pWMH->nMenus]=MT_PICKABLE;
			strcpy (pWMH->menuTitle[pWMH->nMenus++],"Pickability");
		}
		else
		{
			//pWMH->Verticle = TRUE;
			pWMH->Flat = FALSE;
			pWMH->xpad = pWMH->ypad = 3;
			pWMH->menuType[pWMH->nMenus]=MT_CMDMENU;
			pWMH->menuHandle[pWMH->nMenus]=GSSiGlobAlloc (1709,GHND,sizeof(CMDMENUHEADER));
			pCMDHeader = (LPCMDMENUHEADER)GlobalLock (pWMH->menuHandle[pWMH->nMenus]);
/*	strcpy (pCMDHeader->CMDFile,"[%DL]fundir\\base.txt");
			LoadGFFile (hWnd,pCMDHeader->CMDFile,4,FALSE);
			GlobalUnlock (pWMH->menuHandle[pWMH->nMenus]);
			strcpy (pWMH->menuTitle[pWMH->nMenus++],"Base Menu");
			pWMH->menuType[pWMH->nMenus]=MT_CMDMENU;
			pWMH->menuHandle[pWMH->nMenus]=GSSiGlobAlloc (1709,GHND,sizeof(CMDMENUHEADER));
			pCMDHeader = (LPCMDMENUHEADER)GlobalLock (pWMH->menuHandle[pWMH->nMenus]);*/
	strcpy (pCMDHeader->CMDFile,"[%DL]fundir\\sewersetup.txt");
			LoadGFFile (hWnd,pCMDHeader->CMDFile,4,FALSE);
			GlobalUnlock (pWMH->menuHandle[pWMH->nMenus]);
			strcpy (pWMH->menuTitle[pWMH->nMenus++],"Sewer Menu");
			pWMH->menuType[pWMH->nMenus]=MT_CMDMENU;
			pWMH->menuHandle[pWMH->nMenus]=GSSiGlobAlloc (1709,GHND,sizeof(CMDMENUHEADER));
			pCMDHeader = (LPCMDMENUHEADER)GlobalLock (pWMH->menuHandle[pWMH->nMenus]);
	strcpy (pCMDHeader->CMDFile,"[%DL]fundir\\makearea.txt");
			LoadGFFile (hWnd,pCMDHeader->CMDFile,4,FALSE);
			GlobalUnlock (pWMH->menuHandle[pWMH->nMenus]);
			strcpy (pWMH->menuTitle[pWMH->nMenus++],"Make Area Menu");
		}
	}
	else
	{
		hInitData = hInit; //GetMenuWindowHandle (hWnd);
		pWMH = (LPWINDOWMENUHEADER)GlobalLock (hInitData);

	}
/*	if (!pWMH->menuHandle[pWMH->currentMenu])
	{
		InitCatalogData (hInitData);
	}
	else
	{
		pVCHeader = GlobalLock (pWMH->menuHandle[pWMH->currentMenu]);

	}*/
	FirstDisplay = First;
	pVP = SetVPFromName (ConnectToVP,&err);
	vpID = pVP->ID;
//	if (!*ConnectToVP)
//		Float = TRUE;
	rect = *pRect;
	if (!First && !Float)
	{
		int w=RECTWIDTH(&rect);
		int h=RECTHEIGHT(&rect);

		rect.left = pWMH->ScreenStartPoint.x;
		rect.top = pWMH->ScreenStartPoint.y;
		rect.right = rect.left+w;
		rect.bottom = rect.top+h;
	}
	SetConfig (SaveConfig);
	CurView = pSaveVP;
	/*hMenu = CreateMenu ();
	AppendMenu (hMenu,MF_ENABLED|MF_STRING,65001,"Refresh Screen");
	AppendMenu (hMenu,MF_ENABLED|MF_STRING,65001,"Float");*/

	pWMH->HaveTrackMouseEvent = FALSE;
	floating  = Float;
	RegisterFloatMenuClass(FALSE);
	pWMH->hWnd = hWnd;
	if (First)
	{
	//	TempLoadFloatMenuData (0);
	//	GlobalUnlock (hInit);
	}
	hInitData = hInit;
	OrigRect = rect;
		//hWndDisplay = DoCreateDisplayWindow (hWndTab);
		//hWndDisplay = DoCreateDisplayWindow(hWndTab);
	CreatingParentWindow = TRUE;
    if (Float)
	{
		hWndMenu = CreateWindowEx(WS_EX_TOOLWINDOW|WS_EX_WINDOWEDGE,
	    MENUWINDOWCLASS,
	    "Set Visibility",
	    //WS_VISIBLE|WS_POPUP|WS_CLIPCHILDREN,  
		WS_CAPTION|WS_POPUP|WS_BORDER|
        WS_SYSMENU|WS_VISIBLE,//WS_CLIPCHILDREN|WS_CLIPSIBLINGS|
	    rect.left,rect.top,RECTWIDTH(&rect),RECTHEIGHT(&rect), 
	    hWnd,	/* parent */
	    hMenu,	/* no menu */
	    hInst,
	    0);
		pWMH->Float = TRUE;
	}
	else
	{
		//ScreenRectToClientRect (hWndMain,&rect);
		hWndMenu = CreateWindowEx(WS_EX_TOOLWINDOW,//|WS_EX_WINDOWEDGE,
	   MENUWINDOWCLASS,
	   //"STATIC",
	    "",
	    //WS_VISIBLE|WS_POPUP|WS_CLIPCHILDREN,  
		//WS_CAPTION|WS_BORDER|WS_POPUP|WS_SYSMENU|
        //WS_CHILDWINDOW|WS_VISIBLE|WS_SIZEBOX ,
		WS_POPUP|WS_VISIBLE|WS_BORDER,//|WS_SIZEBOX,//|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
	    rect.left,rect.top,RECTWIDTH(&rect),RECTHEIGHT(&rect), 
	    hWnd,	/* parent */
	    hMenu,	/* no menu */
	    hInst,
	    0);
		pVP->hWndDlg = hWndMenu;
		pWMH->pVP = pVP;
		//ShowWindow (hWndMenu,SW_HIDE);
	}
	if (!hWndMenu)
		return FALSE;
	ToolbarID = LoadToolbar (hWndMenu,"","VIS",0,1,"0 0",CheckForDock,pWMH->Float,0,0,vpID);
	SetToolbarDockingStatus (ToolbarID,pWMH->isDocked,pWMH->dockWidth);
	if (!pWMH->Factor)
		pWMH->Factor = 1;
	buttonheight = ButtonHeight * pWMH->Factor;
	buttonwidth  = ButtonWidth  * pWMH->Factor;
	indent = Indent * pWMH->Factor;
	GetClientRect (hWndMenu,&rect);
	hWndTab = DoCreateTabControl(hWndMenu,pWMH->nMenus,(LPSTR)pWMH->menuTitle,32,&rect,pWMH->Verticle,pWMH->Flat,pWMH->xpad,pWMH->ypad);
	if (!pWMH->hFontTab)
	{
		pWMH->hFontTab = CreateFont(14, 0, 0, 0, FW_BOLD,0, 0, 0, 0, 0, 0, 0, 0,"Arial"); 
		GetObject (pWMH->hFontTab,sizeof(LOGFONT),&pWMH->LogFontTab);
	}
	SendMessage (hWndTab,WM_SETFONT,(WPARAM)pWMH->hFontTab,MAKELPARAM(FALSE, 0)); 

    TabCtrl_AdjustRect(hWndTab, FALSE, &rect); 
	g_hwndDisplay = CreateWindowEx(WS_EX_TOPMOST,
	    MENUWINDOWCLASS,
	    "",
	    //WS_VISIBLE|WS_POPUP|WS_CLIPCHILDREN,  
		//WS_CAPTION|WS_BORDER|WS_POPUP|WS_SYSMENU|
        WS_CHILDWINDOW|WS_VISIBLE,//|WS_VSCROLL|WS_HSCROLL,
	    rect.left,rect.top,RECTWIDTH(&rect),RECTHEIGHT(&rect), 
	    hWndTab,	/* parent */
	    0,	/* no menu */
	    hInst,
	    0);
/*	{
		long g_OldProc = GetWindowLong (hWndVisMenu,GWL_WNDPROC);
		long g_OldProc2 = GetWindowLong (hWndMenu,GWL_WNDPROC);

		if (g_OldProc == g_OldProc2)
			SetWindowLong (hWndVisMenu,GWL_WNDPROC,g_OldProc2);
	}*/
    SetWindowPos(g_hwndDisplay, HWND_TOP, rect.left, rect.top, 
				rect.right - rect.left, rect.bottom - rect.top, 0); 
	ToolbarWindow[ToolbarID] = hWndTab;//g_hwndDisplay;
	hwndTT = DoCreateDialogTooltip(ToolbarID); 
	ToolbarWindow[ToolbarID] = hWndMenu;
	pWMH->hWndTab = hWndTab;
	TabCtrl_SetToolTips(hWndTab,hwndTT);

	TabCtrl_SetCurSel(hWndTab,pWMH->currentMenu);  
	GlobalUnlock (hInitData);
	SetToolbarConfig (ToolbarID,0);
	if (Float)
	{
		POINT	pt;

		pt.x = rect.left;
		pt.y = rect.top;
		if (SeeIfToolbarShouldBeDocked (ToolbarWindow[ToolbarID],&pt))
			AdjustToolbarPositions ();
	}
	setDoPaint(TRUE);
	PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);
	return hWndMenu;
}

BOOL FAR COLORINTENSITYMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
  switch(Message)
   {
    case WM_INITDIALOG:
         cwCenter(hWndDlg, -2);
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:

         switch(LOWORD(wParam))

         {
		 case IDCANCEL:
			 EndDialog(hWndDlg, FALSE);
			 break;
			 
		 case IDOK:
			 EndDialog(hWndDlg, TRUE);
			 break;

		 case IDC_CONVERTTOGRAY:
			 break;

		 }
		 break;
	 default:  
		return FALSE;
	}
  return TRUE;
}

BOOL DrawAVButton (HDC hDC,LPSTR ID,int iAV,LPINT px,LPINT py,int On)
{
	HBRUSH	hBrush;
	RECT	rect, saverect;
	HFONT	hFont, hOldFont;
	int		left, right, w;
	int		modnum=0;
	LPSTR	pParms;
	int		type=6;

	rect.left = *px;
	rect.top  = *py;
	rect.bottom = *py + buttonheight;
	rect.right = *px + buttonwidth;
	(*py) += buttonheight + 3;
	if (rect.top < menuy)
		return FALSE;
	saverect = rect;
	right = rect.right;
	hBrush = CreateSolidBrush (RGB(240,240,0));
	hFont  = CreateFont(-IDNINT((2*buttonheight)/4+TypeFontSizeInc[type]*pWMH->Factor), 0, 0, 0, TypeFontWidth[type],0, TypeFontUnderline[type], 0, 0, 0, 0, 0, 0,TypeFontName[type]);   
	hOldFont = SelectObject (hDC,hFont);
	SetTextColor (hDC,TypeColors[type]);
	DrawText(hDC, ID, -1, &rect, DT_WORDBREAK | DT_LEFT | DT_VCENTER |DT_SINGLELINE |DT_NOCLIP |DT_CALCRECT );
	if (On)
	{
		FillRect (hDC,&rect,hBrush);
		SetBkMode (hDC,TRANSPARENT);
	}
	DrawText(hDC, ID, -1, &rect, DT_WORDBREAK | DT_LEFT | DT_VCENTER |DT_SINGLELINE |DT_NOCLIP );

	AddAVRect (&rect,iAV,FALSE);
	SelectObject (hDC,hOldFont);
	DeleteObject (hFont);
	DeleteObject (hBrush);
	return TRUE;
}

int DrawCMDButton (HDC hDC,LPSTR ID,HBITMAP hBM,int iCMD,LPINT px,LPINT py,int h,int w)
{
	HBRUSH	hBrush, hOldBrush;
	HPEN	hPen, hOldPen;
	RECT	rect, rect2, saverect;
	HFONT	hFont, hOldFont, hFontHotKey;
	int		left, right;
	int		modnum=0;
	LPSTR	pParms;
	int		type=6;
	BOOL	display=TRUE;

	if (h < 0)
	{
		h = -h;
		display = FALSE;
	}
	rect.left = *px + 2;
	rect.top  = *py + 3;
	*py += h + 3;
	rect.bottom = *py;
	rect.right = *px + w-3;
	if (rect.top < menuy)
		return FALSE;
	right = rect.right;
	if (hBM)
		DisplayBitmapInRect (hDC,rect,hBM,1,SRCCOPY);
	else
	{
		RECT	rectHotKey = rect;
		LPSTR	pHotKey = strchr (ID,'^');

		if (pHotKey)
			*pHotKey++ = 0;
		hFontHotKey  = CreateFont(-2*buttonheight/3, 0, 0, 0, FW_BOLD,0, FALSE, 0, 0, 0, 0, 0, 0,"Courier New");   
		hOldFont = SelectObject (hDC,hFontHotKey);
		DrawText(hDC, "A", -1, &rectHotKey, DT_LEFT |DT_CALCRECT );
		SelectObject (hDC,hOldFont);
		rect.left = rectHotKey.right + 6;
		hFont  = CreateFont(-IDNINT((2*buttonheight)/4+TypeFontSizeInc[type]*pWMH->Factor), 0, 0, 0, TypeFontWidth[type],0, TypeFontUnderline[type], 0, 0, 0, 0, 0, 0,TypeFontName[type]);   
		hOldFont = SelectObject (hDC,hFont);
		SetTextColor (hDC,RGB(0,0,64));//TypeColors[type]);
		if (!display)
		{
			DrawText(hDC, ID, -1, &rect, DT_WORDBREAK | DT_LEFT |DT_CALCRECT );
		}
		else
		{
			if (pHotKey)
			{
				hOldFont = SelectObject (hDC,hFontHotKey);
				DrawText(hDC, pHotKey, -1, &rectHotKey, DT_LEFT);
				SelectObject (hDC,hOldFont);
			}
			hBrush = CreateSolidBrush (RGB(232,232,255));
			hPen = CreatePen (PS_SOLID,1,RGB(0,0,128));
			hOldBrush = SelectObject (hDC,hBrush);
			hOldPen = SelectObject (hDC,hPen);
			rect2 = rect;
			DrawText(hDC, ID, -1, &rect2, DT_WORDBREAK | DT_LEFT |DT_CALCRECT );
			rect.bottom = rect2.bottom;
			saverect = rect;
			InflateRect (&rect,3,3);
//			FillRect (hDC,&rect,hBrush);
			RoundRect (hDC,rect.left,rect.top,rect.right,rect.bottom,h/2,h/2);
//			FrameRect (hDC,&rect,GetStockObject(BLACK_BRUSH));
			SetBkMode (hDC,TRANSPARENT);
			rect = saverect;
			DrawText(hDC, ID, -1, &rect, DT_WORDBREAK | DT_CENTER );
			InflateRect (&rect,3,3);
			SelectObject (hDC,hOldBrush);
			SelectObject (hDC,hOldPen);
			DeleteObject (hBrush);
			DeleteObject (hPen);
			AddCMDRect (&rect,iCMD,FALSE);
		}
		if (pHotKey)
			*(pHotKey-1) = '^';
		SelectObject (hDC,hOldFont);
		DeleteObject (hFont);
		DeleteObject (hFontHotKey);
	}
	(*py) = rect.bottom + 3;
	return rect.right+3;
}

int AddButtonToCMDMenu (HWND hWndDlg,LPSTR BMPath,LPSTR ButtonText,int filepos)
{
	HBITMAP	hBM = GetToolBitmap(BMPath); 

	if (!pCMDHeader)
		return -1;
	if (pCMDHeader->nCMD >= MAX_CMDMENU_COMMANDS)
		return -1;
	pCMDHeader->hBM[pCMDHeader->nCMD] = hBM;
	pCMDHeader->FileLoc[pCMDHeader->nCMD] = filepos;
	strncpy0 (pCMDHeader->Label[pCMDHeader->nCMD++],ButtonText,63);
	return 0;
}

