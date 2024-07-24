#include "graphint.h"
#include "extrndb.h" 
#include "std.h"  
#include "jansson.h"
#include "curl\curl.h"
#include "gmextern.h"   

static	double	IntersectionAverageDist=10;
static HANDLE	hNames;
static HANDLE	hCurStreets=0;
static HWND		hWndSecondaryIntInput = 0;


#define BUFFER_SIZE  (256 * 1024)  /* 256 KB */

#define URL_FORMAT   "http://github.com/api/v2/json/commits/list/%s/%s/master"
#define URL_SIZE     256

/* Return the offset of the first newline in text or the length of
text if there's no newline */
static int newline_offset(const char *text)
{
	const char *newline = strchr(text, '\n');
	if (!newline)
		return strlen(text);
	else
		return (int)(newline - text);
}

struct write_result
{
	char *data;
	int pos;
};

static size_t write_response(void *ptr, size_t size, size_t nmemb, void *stream)
{
	struct write_result *result = (struct write_result *)stream;

	if (result->pos + size * nmemb >= BUFFER_SIZE - 1)
	{
		fprintf(stderr, "error: too small buffer\n");
		return 0;
	}

	memcpy(result->data + result->pos, ptr, size * nmemb);
	result->pos += size * nmemb;

	return size * nmemb;
}

static BOOL wantStreet(int streetNum, int numSelected, LPINT pStreetNums)
{
	BOOL rtn = FALSE;
	if (!numSelected)
		return TRUE;
	for (int i = 0; i < numSelected; i++)
	{
		if (streetNum == pStreetNums[i])
			rtn = TRUE;
	}
	return rtn;
}
void DisplayCurStreets (BOOL Clear,int Flash,int numSelected,LPINT pStreetNums,LPMNMXCORD pBounds)
{
	if (Clear)
		GSSiGlobFree (&hCurStreets);
	else if (hCurStreets)
	{
		LPSTR pPolys;
		int	streetNum, nPnts, width, color, white=RGB(255,255,255);
		LPDPOINT pPoints;

		if (!pBounds)
		{
			GSSiDeleteObject(&CurView->hRgn);
			CurView->hRgn = CreateVPRgn(FALSE, FALSE);
			SelectClipRgn(CurView->hDC, CurView->hRgn);
			GSSiDeleteObject(&CurView->hRgn);
		}
		else
		{
			DBoundsInit(pBounds);
			Flash = 1;
		}
		Flash++;
		while (Flash--)
		{
			pPolys = GlobalLock (hCurStreets);
			while ((streetNum = *(LPINT)pPolys))
			{
				pPolys += sizeof(int);
				nPnts = *(LPINT)pPolys;
				pPolys += sizeof(int);
				width = *(LPINT)pPolys;
				pPolys += sizeof(int);
				color = *(LPINT)pPolys;
				if (Flash % 2)
					color = white;
				pPolys += sizeof(int);
				pPoints = (LPDPOINT)pPolys;
				pPolys += sizeof(DPOINT)*nPnts;
				if (wantStreet (streetNum, numSelected,pStreetNums))
				{
					if (pBounds)
					{
						for (int i=0;i<nPnts;i++)
							AddDPointToMinMax(&pPoints[i], pBounds);
					}
					else
					{
						HPEN	hPen = CreatePen(PS_SOLID, IDNINT(AdjustWidth(width)), color);
						HPEN	OldPen = SelectObject(CurView->hDC, hPen);

						GWPolylineD(CurView->hDC, pPoints, nPnts, 0);
						SelectObject(CurView->hDC, OldPen);
						GSSiDeleteObject(&hPen);
					}
				}
			}
			GlobalUnlock (hCurStreets);
			if (!pBounds)
				Sleep (50);
		}
	}
	return;
}

long AddToCurStreets (HWND hWndDlg,long CurPath,LPMNMXCORD TotMinMax, LPINT pDisplayedStreets)
{   
	char	str[256];
	COLORREF	DrawStreetColors[4]={RGB(255,0,0),RGB(0,255,0),RGB(0,255,255),RGB(255,255,0)}; 
	char	ColorNames[4][8]={"Red","Green","Lt Blue","Yellow"};
	BOOL	HighlightStreet;
	long	nSegs=0;
	     
	GetTrueStreetName (CurPath,str, 0,0); 
	SetDlgItemText (hWndDlg,IDC_STREET,str);
	_fstrcat (str,"\t");
	if (SendDlgItemMessage (hWndDlg,IDC_CURSTREETS,LB_FINDSTRING,(WPARAM)-1,(LPARAM) str) == LB_ERR)
	{
		sprintf (strchr(str,0),"%s\t%i", ColorNames[*pDisplayedStreets % 4],CurPath);
		SendDlgItemMessage (hWndDlg,IDC_CURSTREETS,LB_ADDSTRING,0,(LPARAM)str);
		HighlightStreet = SendDlgItemMessage (hWndDlg,IDC_HIGHLIGHT,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L);
		SetDlgItemText (hWndDlg,IDC_MESS,"");
		SetViewport(*pCommandViewport);
		GSSiDeleteObject(&CurView->hRgn);
		CurView->hRgn = CreateVPRgn(FALSE,FALSE);
  		SelectClipRgn (CurView->hDC,CurView->hRgn);
  		GSSiDeleteObject(&CurView->hRgn); 
		nSegs = DrawStreet_new (CurPath,DrawStreetColors[*pDisplayedStreets%4],3,HighlightStreet,TotMinMax,FALSE,&hCurStreets);
		(*pDisplayedStreets)++;
		sprintf (str,"%ld segments found",nSegs);
		SetDlgItemText (hWndDlg,IDC_MESS,str);
	}
	return nSegs;
}

long DrawStreet(long sNum, long WantLinkID, COLORREF Color, short Width, BOOL HighlightStreet, LPMNMXCORD pTotMinMax, BOOL LimToMinMax, LPHANDLE phPoly)
{
	BOOL	Opened, OpenedSP;
	NETREFSKEY		NetRefsKey, NetRefsKey2;
	NETREFSDATA		NetRefsData, NetRefsData2;
	short	pos = BT_FIRST, cond = BT_GT;
	HCURSOR	OldCursor;
	long	NumSegs = 0, LinkID;


	OpenStreetPolys(&OpenedSP);
	CreateSpecial();
	OldCursor = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
	NetRefsKey.Path = sNum;
	NetRefsKey.MP = WantLinkID * 1000000;
	while (!BT_FIND(hBTNetRefs, (LPSTR)&NetRefsKey, pos, cond, (LPSTR)&NetRefsData))
	{
		pos = BT_NEXT;
		cond = BT_ANY;
		LinkID = NetRefsKey.MP / 1000000;
		if (NetRefsKey.Path != sNum || (LinkID != WantLinkID && WantLinkID >= 0))
			break;
		NumSegs++;
		if (hStreetPolys)
		{
			long	Offset, Refno, nPnts, Size, EndPointNum;
			short	Desc;
			UINT	i;
			HPDPOINT	Points;
			double	AtDist;
			HANDLE	hPoly;
			STREETPOLYHEADER	Header;

			if (!BT_FIND(hStreetPolys, (LPSTR)&NetRefsData.Ref, BT_FIRST, BT_EQ, (LPSTR)&Offset))
			{
				GSSillseek(FidStreetPolys, Offset, 0);
				BigRead(FidStreetPolys, (HPSTR)&Header, sizeof(STREETPOLYHEADER));
				Size = Header.nPnts * sizeof(DPOINT);
				hPoly = GSSiGlobAlloc(0, GMEM_MOVEABLE, Size);
				Points = (HPDPOINT)GlobalLock(hPoly);
				BigRead(FidStreetPolys, (HPSTR)Points, Size);
				if (pTotMinMax)
				{
					for (i = 0; i < Header.nPnts; i++)
						if (!LimToMinMax || DPointInBounds(&Points[i], pTotMinMax))
							AddDPointToMinMax(&Points[i], pTotMinMax);
				}
				if (phPoly)
				{
					LPSTR pPolys;

					if (!*phPoly)
					{
						*phPoly = GSSiGlobAlloc(1730, GMEM_MOVEABLE, 4 * sizeof(int) + Header.nPnts * sizeof(DPOINT));
						pPolys = GlobalLock(*phPoly);
					}
					else
					{
						int lMem = GlobalSize(*phPoly);

						*phPoly = GlobalReAlloc(*phPoly, lMem + 3 * sizeof(int) + Header.nPnts * sizeof(DPOINT), GMEM_MOVEABLE);
						pPolys = GlobalLock(*phPoly);
						pPolys += lMem - sizeof(int);
					}
					*(LPINT)pPolys = Header.nPnts;
					pPolys += sizeof(int);
					*(LPINT)pPolys = Width;
					pPolys += sizeof(int);
					*(LPINT)pPolys = Color;
					pPolys += sizeof(int);
					memcpy(pPolys, Points, Header.nPnts * sizeof(DPOINT));
					pPolys += sizeof(DPOINT) * Header.nPnts;
					*(LPINT)pPolys = 0;
					GlobalUnlock(*phPoly);
				}
				GSSiGlobUlFree(&hPoly);
				NumSegs++;
			}
		}
		else if (PickByRefno(NetRefsData.Ref, 0, 0, -1))
		{
			if (pTotMinMax)
			{
				if (LimToMinMax)
				{
					if (!BoundsInBounds(pTotMinMax, &PickList[0].Rect, 1))
						continue;
				}
				else
					AddMinMaxD(pTotMinMax, &PickList[0].Rect);
			}
			if (HighlightStreet)
				AddToHighlightList(NetRefsData.Ref, &PickList[0], TRUE);
			else
				AddSpecial(NetRefsData.Ref, 1, Color, Width);
			ProcessPickedItem(0, 2);
		}
	}
	CloseStreetPolys(OpenedSP);
	GSSiSetCursor(OldCursor);
	return NumSegs;
}
long DrawStreet_new(long sNum, COLORREF Color, short Width, BOOL HighlightStreet, LPMNMXCORD pTotMinMax, BOOL LimToMinMax, LPHANDLE phPoly)
{
	BOOL	Opened, OpenedSP;
	short	pos = BT_FIRST, cond = BT_GT;
	HCURSOR	OldCursor;
	long	NumSegs = 0, StreetNumAndRefno[2], Offset ;


	OpenStreetPolys(&OpenedSP);
	//CreateSpecial();
	OldCursor = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
	LPGWDHEADER lpGWDHead = (LPGWDHEADER)GlobalLock(hDBStreetNumRefs);
	OldCursor = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
	//CreateSpecial();
Top:
	StreetNumAndRefno[0] = sNum;
	StreetNumAndRefno[1] = LONG_MIN;
	pos = BT_FIRST;
	cond = BT_GE;
	while (!BT_FIND(lpGWDHead->BTHandle[0], (LPSTR)StreetNumAndRefno, pos, cond, (LPSTR)&Offset))
	{
		pos = BT_NEXT;
		cond = BT_ANY;
		if (StreetNumAndRefno[0] != sNum)
			break;
		NumSegs++;
		if (hStreetPolys)
		{
			long	Offset, Refno, nPnts, Size, EndPointNum;
			short	Desc;
			UINT	i;
			HPDPOINT	Points;
			double	AtDist;
			HANDLE	hPoly;
			STREETPOLYHEADER	Header;

			if (!BT_FIND(hStreetPolys, (LPSTR)&StreetNumAndRefno[1], BT_FIRST, BT_EQ, (LPSTR)&Offset))
			{
				GSSillseek(FidStreetPolys, Offset, 0);
				BigRead(FidStreetPolys, (HPSTR)&Header, sizeof(STREETPOLYHEADER));
				Size = Header.nPnts * sizeof(DPOINT);
				hPoly = GSSiGlobAlloc(0, GMEM_MOVEABLE, Size);
				Points = (HPDPOINT)GlobalLock(hPoly);
				BigRead(FidStreetPolys, (HPSTR)Points, Size);
				if (pTotMinMax)
				{
					for (i = 0; i < Header.nPnts; i++)
						if (!LimToMinMax || DPointInBounds(&Points[i], pTotMinMax))
							AddDPointToMinMax(&Points[i], pTotMinMax);
				}
				if (phPoly)
				{
					LPSTR pPolys;

					if (!*phPoly)
					{
						*phPoly = GSSiGlobAlloc(1730, GMEM_MOVEABLE, 5 * sizeof(int) + Header.nPnts * sizeof(DPOINT));
						pPolys = GlobalLock(*phPoly);
					}
					else
					{
						int lMem = GlobalSize(*phPoly);

						*phPoly = GlobalReAlloc(*phPoly, lMem + 4 * sizeof(int) + Header.nPnts * sizeof(DPOINT), GMEM_MOVEABLE);
						pPolys = GlobalLock(*phPoly);
						pPolys += lMem - sizeof(int);
					}
					*(LPINT)pPolys = sNum;
					pPolys += sizeof(int);
					*(LPINT)pPolys = Header.nPnts;
					pPolys += sizeof(int);
					*(LPINT)pPolys = Width;
					pPolys += sizeof(int);
					*(LPINT)pPolys = Color;
					pPolys += sizeof(int);
					memcpy(pPolys, Points, Header.nPnts * sizeof(DPOINT));
					pPolys += sizeof(DPOINT) * Header.nPnts;
					*(LPINT)pPolys = 0;
					GlobalUnlock(*phPoly);
				}
				GSSiGlobUlFree(&hPoly);
				NumSegs++;
			}
		}
		else if (PickByRefno(StreetNumAndRefno[1], 0, 0, -1))
		{
			if (pTotMinMax)
			{
				if (LimToMinMax)
				{
					if (!BoundsInBounds(pTotMinMax, &PickList[0].Rect, 1))
						continue;
				}
				else
					AddMinMaxD(pTotMinMax, &PickList[0].Rect);
			}
			if (HighlightStreet)
				AddToHighlightList(StreetNumAndRefno[1], &PickList[0], TRUE);
			else
				AddSpecial(StreetNumAndRefno[1], 1, Color, Width);
			ProcessPickedItem(0, 2);
		}
	}
	GlobalUnlock(hDBStreetNumRefs);
	CloseStreetPolys(OpenedSP);
	GSSiSetCursor(OldCursor);
	return NumSegs;
}

int GetNumStreetSegs (long StreetNum,long WantZIP,LPMNMXCORD pTotMinMax,BOOL LimToMinMax, LPMNMXCORD pBounds, int DrawingOption,COLORREF Color,int Width)
{   
	BOOL	Opened;
	LPGWDHEADER	lpGWDHead; 
	long	NumSegs=0, LinkID, Offset,StreetNumAndRefno[2];  
	short	pos, cond, Index=1;
	BOOL	OpenedSP=FALSE;
	int		i;
	
    if (!OpenStreetSegmentTable (FALSE,&Opened))
		return 0;
	OpenStreetPolys (&OpenedSP);
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetSegments); 
    OldCursor = GSSiSetCursor (LoadCursor (0,IDC_WAIT));
Top:
	StreetNumAndRefno[0] = StreetNum;
	StreetNumAndRefno[1] = LONG_MIN;
	pos = BT_FIRST;
	cond = BT_GE;
	while (lpGWDHead->BTHandle[Index] && !BT_FIND (lpGWDHead->BTHandle[Index],(LPSTR)StreetNumAndRefno,pos,cond, (LPSTR)&Offset))
	{
		pos = BT_NEXT;
		cond = BT_ANY;  
		if (StreetNumAndRefno[0] == StreetNum) 
		{   
			if (WantZIP)
			{
				// SegMaxKey.ZIPCode == WantZIP)
			} 
			if (hStreetPolys)
			{   
    			long	Offset,Refno,nPnts,Size,EndPointNum;
    			short	Desc; 
    			HPDPOINT	Points; 
    			double	AtDist; 
    			HANDLE	hPoly;
				STREETPOLYHEADER	Header;
    			
    			if (!BT_FIND (hStreetPolys,(LPSTR)&StreetNumAndRefno[1],BT_FIRST,BT_EQ,(LPSTR)&Offset))
				{
    				GSSillseek (FidStreetPolys,Offset,0);
    				BigRead (FidStreetPolys,(HPSTR)&Header,sizeof(STREETPOLYHEADER));
    				Size = Header.nPnts * sizeof(DPOINT);
    				hPoly = GSSiGlobAlloc (0,GMEM_MOVEABLE,Size);
    				Points = (HPDPOINT)GlobalLock (hPoly);
    				BigRead (FidStreetPolys,(HPSTR)Points,Size); 
					if (pBounds)
					{
						for (i=0;i<Header.nPnts;i++)
							AddDPointToMinMax (&Points[i],pBounds);
					}
    				GSSiGlobUlFree (&hPoly);
					NumSegs++;
				}
			}
			else if (PickByRefno(StreetNumAndRefno[1],0,0,-1))
			{
				if (pTotMinMax)
				{
					if (LimToMinMax)
					{
						if (!BoundsInBounds (pTotMinMax,&PickList[0].Rect,1))
							continue;   
					}
					else
						AddMinMaxD (pTotMinMax,&PickList[0].Rect);
				}
				NumSegs++;
				AddSpecial (PickList[0].Refno,1,Color,Width);
				ProcessPickedItem (0,2); 
			}
		}
		else
			break;
	}
	if (Index++ < 4)
		goto Top;
    GlobalUnlock (hDBStreetSegments);
	CloseStreetPolys (OpenedSP);
    CloseStreetSegmentTable(Opened); 
	return NumSegs;
}

int GetNumStreetSegs_new(long StreetNum, long WantAllSegs, LPMNMXCORD pTotMinMax, BOOL LimToMinMax, LPMNMXCORD pBounds, int DrawingOption, COLORREF Color, int Width)
{
	LPGWDHEADER	lpGWDHead;
	long	NumSegs = 0, LinkID, Offset, StreetNumAndRefno[3];
	short	pos, cond, Index = 0;
	BOOL	OpenedSP = FALSE;
	int		i;

	if (!OpenStreetPolys(&OpenedSP))
		return 0;
	lpGWDHead = (LPGWDHEADER)GlobalLock(hDBStreetNumRefs);
	OldCursor = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
Top:
	StreetNumAndRefno[0] = StreetNum;
	StreetNumAndRefno[1] = LONG_MIN;
	StreetNumAndRefno[2] = LONG_MIN;
	pos = BT_FIRST;
	cond = BT_GE;
	while (lpGWDHead->BTHandle[Index] && !BT_FIND(lpGWDHead->BTHandle[Index], (LPSTR)StreetNumAndRefno, pos, cond, (LPSTR)&Offset))
	{
		pos = BT_NEXT;
		cond = BT_ANY;
		if (StreetNumAndRefno[0] == StreetNum)
		{
			if (hStreetPolys)
			{
				long	Offset, Refno, nPnts, Size, EndPointNum;
				short	Desc;
				HPDPOINT	Points;
				double	AtDist;
				HANDLE	hPoly;
				STREETPOLYHEADER	Header;

				if (!BT_FIND(hStreetPolys, (LPSTR)&StreetNumAndRefno[1], BT_FIRST, BT_EQ, (LPSTR)&Offset))
				{
					GSSillseek(FidStreetPolys, Offset, 0);
					BigRead(FidStreetPolys, (HPSTR)&Header, sizeof(STREETPOLYHEADER));
					Size = Header.nPnts * sizeof(DPOINT);
					hPoly = GSSiGlobAlloc(0, GMEM_MOVEABLE, Size);
					Points = (HPDPOINT)GlobalLock(hPoly);
					BigRead(FidStreetPolys, (HPSTR)Points, Size);
					if (pBounds)
					{
						for (i = 0; i < Header.nPnts; i++)
							AddDPointToMinMax(&Points[i], pBounds);
					}
					GSSiGlobUlFree(&hPoly);
					NumSegs++;
				}
			}
			else if (PickByRefno(StreetNumAndRefno[1], 0, 0, -1))
			{
				if (pTotMinMax)
				{
					if (LimToMinMax)
					{
						if (!BoundsInBounds(pTotMinMax, &PickList[0].Rect, 1))
							continue;
					}
					else
						AddMinMaxD(pTotMinMax, &PickList[0].Rect);
				}
				NumSegs++;
				AddSpecial(PickList[0].Refno, 1, Color, Width);
				ProcessPickedItem(0, 2);
			}
			if (NumSegs && !WantAllSegs)
				break;
		}
		else
			break;
	}
	GlobalUnlock(hDBStreetNumRefs);
	CloseStreetPolys(OpenedSP);
	return NumSegs;
}

long DrawStreet2(long StreetNum, long WantZIP, COLORREF Color, short Width, BOOL HighlightStreet, LPMNMXCORD pTotMinMax, BOOL LimToMinMax)
{
	BOOL	Opened;
	LPGWDHEADER	lpGWDHead;
	HCURSOR	OldCursor;
	long	NumSegs = 0, LinkID, Offset, StreetNumAndRefno[2];
	short	pos, cond, Index = 1;
	HDC		hDC;

	if (!OpenStreetSegmentTable(FALSE, &Opened))
		return 0;
	lpGWDHead = (LPGWDHEADER)GlobalLock(hDBStreetSegments);
	OldCursor = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
	CreateSpecial();
Top:
	StreetNumAndRefno[0] = StreetNum;
	StreetNumAndRefno[1] = LONG_MIN;
	pos = BT_FIRST;
	cond = BT_GE;
	while (!BT_FIND(lpGWDHead->BTHandle[Index], (LPSTR)StreetNumAndRefno, pos, cond, (LPSTR)&Offset))
	{
		pos = BT_NEXT;
		cond = BT_ANY;
		if (StreetNumAndRefno[0] == StreetNum)
		{
			if (WantZIP)
			{
				// SegMaxKey.ZIPCode == WantZIP)
			}
			if (PickByRefno(StreetNumAndRefno[1], 0, 0, -1))
			{
				if (pTotMinMax)
				{
					if (LimToMinMax)
					{
						if (!BoundsInBounds(pTotMinMax, &PickList[0].Rect, 1))
							continue;
					}
					else
						AddMinMaxD(pTotMinMax, &PickList[0].Rect);
				}
				NumSegs++;
				if (HighlightStreet)
					AddToHighlightList(PickList[0].Refno, &PickList[0], TRUE);
				else
					AddSpecial(PickList[0].Refno, 1, Color, Width);
				ProcessPickedItem(0, 2);
			}
		}
		else
			break;
	}
	if (Index++ < 4)
		goto Top;
	GSSiSetCursor(OldCursor);
	GlobalUnlock(hDBStreetSegments);
	CloseStreetSegmentTable(Opened);
	return NumSegs;
}
long DrawStreet2_new(long StreetNum, long WantZIP, COLORREF Color, short Width, BOOL HighlightStreet, LPMNMXCORD pTotMinMax, BOOL LimToMinMax)
{
	BOOL	Opened;
	LPGWDHEADER	lpGWDHead;
	HCURSOR	OldCursor;
	long	NumSegs = 0, LinkID, Offset, StreetNumAndRefno[2];
	short	pos, cond, Index = 1;
	HDC		hDC;

	if (!OpenStreetPolys(&Opened))
		return 0;
	lpGWDHead = (LPGWDHEADER)GlobalLock(hDBStreetNumRefs);
	OldCursor = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
	//CreateSpecial();
Top:
	StreetNumAndRefno[0] = StreetNum;
	StreetNumAndRefno[1] = LONG_MIN;
	pos = BT_FIRST;
	cond = BT_GE;
	while (!BT_FIND(lpGWDHead->BTHandle[0], (LPSTR)StreetNumAndRefno, pos, cond, (LPSTR)&Offset))
	{
		pos = BT_NEXT;
		cond = BT_ANY;
		if (StreetNumAndRefno[0] == StreetNum)
		{
			if (WantZIP)
			{
				// SegMaxKey.ZIPCode == WantZIP)
			}
			if (PickByRefno(StreetNumAndRefno[1], 0, 0, -1))
			{
				if (pTotMinMax)
				{
					if (LimToMinMax)
					{
						if (!BoundsInBounds(pTotMinMax, &PickList[0].Rect, 1))
							continue;
					}
					else
						AddMinMaxD(pTotMinMax, &PickList[0].Rect);
				}
				NumSegs++;
				if (HighlightStreet)
					AddToHighlightList(PickList[0].Refno, &PickList[0], TRUE);
				else
					AddSpecial(PickList[0].Refno, 1, Color, Width);
				ProcessPickedItem(0, 2);
			}
		}
		else
			break;
	}
	if (Index++ < 4)
		goto Top;
	GSSiSetCursor(OldCursor);
	GlobalUnlock(hDBStreetNumRefs);
	CloseStreetPolys(Opened);
	return NumSegs;
}

/*long DrawStreet2 (long StreetNum,long WantZIP,COLORREF Color,short Width,BOOL HighlightStreet,LPMNMXCORD pTotMinMax,BOOL LimToMinMax)
{   
	BOOL	Opened;
	short	pos=BT_FIRST,cond=BT_GT;
	HCURSOR	OldCursor;
	long	NumSegs=0, LinkID;  
	SEGMAXKEY	SegMaxKey; 
	long		MinHouseNum; 
	short		stMax, n,Side=1;
    long		MinAdd, MaxAdd, Dir, LowAdd, HighAdd; 
	
	if (!OpenSegMaxIndex (&Opened))
		return 0;
    OldCursor = GSSiSetCursor (LoadCursor (0,IDC_WAIT));
	CreateSpecial();
Top:
	SegMaxKey.StreetNum = StreetNum;
	SegMaxKey.ZIPCode = WantZIP;
	SegMaxKey.Side = Side;
	SegMaxKey.MaxHouseNum = 0;
	SegMaxKey.Segid = LONG_MIN;
	while (!BT_FIND (hSegMax,(LPSTR)&SegMaxKey,pos,cond,(LPSTR)&MinHouseNum))
	{
		pos = BT_NEXT;
		cond = BT_ANY;  
		if (SegMaxKey.StreetNum == StreetNum && (!WantZIP || SegMaxKey.ZIPCode == WantZIP)) 
		{
			if (PickByRefno(SegMaxKey.Segid,0,0,-1))
			{
				if (pTotMinMax)
				{
					if (LimToMinMax)
					{
						if (!BoundsInBounds (pTotMinMax,&PickList[0].Rect,1))
							continue;   
					}
					else
						AddMinMaxD (pTotMinMax,&PickList[0].Rect);
				}
				NumSegs++;
				if (HighlightStreet)
			    	AddToHighlightList (PickList[0].Refno,&PickList[0],TRUE);
			    else
					AddSpecial (PickList[0].Refno,1,Color,Width);
				ProcessPickedItem (0,2); 
			}
		}
		else
			break;
	}
	if (WantZIP && Side == 1)
	{
		Side = 2;
		pos=BT_FIRST;
		cond=BT_GT;
		goto Top;
	}
    GSSiSetCursor (OldCursor);  
    CloseSegMaxIndex (Opened);	
	return NumSegs;
}*/

long NumStreetSegs (long sNum)
{   
	BOOL	Opened;
	NETREFSKEY		NetRefsKey,NetRefsKey2;
	NETREFSDATA		NetRefsData,NetRefsData2;
	short	pos=BT_FIRST,cond=BT_GT;
	long	NumSegs=0, LinkID;  
	
	
	if (!OpenNetLinkAndRef (NetworkID,FALSE,&Opened))
		return 0;  
	NetRefsKey.Path =  sNum;
	NetRefsKey.MP = -100000000; 
	while (!BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey,pos,cond,(LPSTR)&NetRefsData))
	{
		pos = BT_NEXT;
		cond = BT_ANY;  
		if (NetRefsKey.Path != sNum)
			break;     
		NumSegs++;
	}
    CloseNetLinkAndRef (Opened);                 
	return NumSegs;
}

void CloseAllAddressFiles (void)
{
	STNDSN_INIT(TRUE);
	CloseNetLinkAndRef (TRUE);
    CloseNetMarkers(TRUE);                   
    CloseStreetNameTable();                  
    CloseStreetSegmentTable(TRUE);  
	CloseSegMaxIndex (TRUE);
	CloseStreetPolys (TRUE);
	return;
}

BOOL AddressLocation1 (HWND hWnd, HINSTANCE hInst,UINT iopt)
{
     {
      DLGPROC lpfnADDRESS1MsgProc;
      short	nRc;
      
	  if (*AltAddressDir)
	  {
		  strcpy (TempAddressDir,AltAddressDir);
		  CloseAllAddressFiles ();
	  }
      lpfnADDRESS1MsgProc = MakeProcInstance((DLGPROC)ADDRESS1MsgProc, hInst);
	  if (iopt == IDM_L_NET_ADDRESS)
		nRc = DialogBox(hInst, (LPCSTR)"ADDRESS1", hWnd, lpfnADDRESS1MsgProc);
	  else
 		nRc = DialogBox(hInst, (LPCSTR)"ADDRESS2", hWnd, lpfnADDRESS1MsgProc);
     FreeProcInstance(lpfnADDRESS1MsgProc);
	  if (*AltAddressDir)
	  {
		  CloseAllAddressFiles ();
		  *TempAddressDir = 0;
	  }
      switch (nRc)
	  {
	  case 1:
      	PickByRefno (AddRefno,AddPrefix[0],AddUDI,-1);
		break;
	  case 2:
      	HighlightStreet (hWnd,AddRefno);
		break;
      case 3:
		//UserSpecifiedBasePoint;
		break;
	  }
      return (nRc);
     }
}
BOOL LocatePID (HWND hWnd, HINSTANCE hInst)
{
     {
      DLGPROC lpfnLOCATEPIDMsgProc;
      short	nRc;
	  int	n = 0;

      lpfnLOCATEPIDMsgProc = MakeProcInstance((DLGPROC)LOCATEPIDMsgProc, hInst);
      nRc = DialogBox(hInst, (LPSTR)"LOCATEPID", hWnd, lpfnLOCATEPIDMsgProc);
      FreeProcInstance(lpfnLOCATEPIDMsgProc);
	  if (nRc)
	  {
		  while (n < 4 && *AddPrefix[n])
		  {
			  if ((AddRefno = PickByRefno(AddRefno, AddPrefix[n], AddUDI, -1)))
			  {
				  AddRefno = PickList[0].Refno;
				  return nRc;
			  }
			  n++;
		  }
		  GSSiMsgBox(hWndMain, "Graphic record not found for this parcel",
			  "Unable to Locate", MB_OK | MB_APPLMODAL, 0);
		  nRc = 0;
	  }

      return (nRc);
     }
}


BOOL AddressLocationPID (HWND hWnd, HINSTANCE hInst)
{     
	      DLGPROC lpfnADDRESSPIDMsgProc;
		  short	nRc;
		  int	n = 0;
		  char testTAG[128] = "Graphic record not found for this address";
	
		  AddToView = FALSE;
	      lpfnADDRESSPIDMsgProc = MakeProcInstance((DLGPROC)ADDRESSPIDMsgProc, hInst);
	      nRc = DialogBox(hInst, (LPSTR)"ADDRESS1", hWnd, lpfnADDRESSPIDMsgProc);
	      FreeProcInstance(lpfnADDRESSPIDMsgProc);
		  if (nRc == 1)
		  {
			  while (n < 4 && *AddPrefix[n])
			  {
				  sprintf(testTAG,"Graphic record not found for this address\n%s:%s", AddPrefix[n], AddUDI);
				  if (!PickByRefno(AddRefno, AddPrefix[n++], AddUDI, -1) || PickList[0].Type == 6)
					  continue;
				  AddRefno = PickList[0].Refno;
				  return nRc;
			  }
			  GSSiMsgBox(hWndMain, testTAG,
				  "Unable to Locate", MB_OK | MB_APPLMODAL, 0);
			  nRc = 0;
		  }
        
      	  return (nRc);
}

short DisplayStreets (HWND hDlg,LONG House, int OddEven, LPSTR InName,int nchar,USHORT EntryControl)
{
    short cond, pos, Index=1, NumDisplay=0;
    long    Offset;
    LPGWDHEADER lpGWDHead;   
    LPGWFLDINFO lpGWFldInfo;  
    HANDLE	hBT, hMatch=0;
    LPLONG	pSNum;
    char	str[128], NetAdd[128],MunName[64],MunAbv[32];
    SEGMAXKEY	SegMaxKey;
    short	NumMatch;
	LPADDMATCH	pMatch=0;  
	LPSTREETNAMETABLE	lpSNT;   
	LPSTR	lpTrueName;
	BOOL	GetNext = TRUE; 
	MSG		msg;
    
    SendDlgItemMessage (hDlg,IDM_STREET_MENU,LB_RESETCONTENT,0,0);
    if (nchar < 1 || !hDBStreetNames)
    	return 0;
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetNames);  
    lpSNT = (LPSTREETNAMETABLE)&lpGWDHead->GWDData;
    hBT = lpGWDHead->BTHandle[Index];
    lpGWFldInfo=lpGWDHead->pFldInfo; 
	SetFieldValFromChar(lpGWDHead, lpGWFldInfo, "0", FALSE, FALSE, TRUE);
    lpGWFldInfo+=Index;     
	SetFieldValFromChar(lpGWDHead, lpGWFldInfo, InName, FALSE, FALSE, TRUE);
    GWDFormKey(lpGWDHead,Index,TRUE,0,0);
    pos = BT_FIRST;
    cond = BT_GE;
    while (GetNext && !BT_FIND (hBT,lpGWDHead->pKeys[Index],pos,cond, (LPSTR)&Offset))
    {
    	pos = BT_NEXT;
    	cond = BT_ANY;
    	if (_fstrnicmp (lpGWDHead->pKeys[Index],InName,nchar))
    		break; 
    	FillGWDData (lpGWDHead,Offset);
    	pSNum = (LPLONG)&lpGWDHead->GWDData;
    	NumMatch = 0; 
		MatchAddress2(1, *pSNum, House, 0, 0, FALSE, &NumMatch, &hMatch, FALSE, TRUE);
		if (hMatch)
			pMatch = (LPADDMATCH)GlobalLock (hMatch);
		while (NumMatch--)
    	{   
    		if (lpGWDHead->Version >1 )
    			lpTrueName = (LPSTR)&lpSNT->TrueName;
    		else
    			lpTrueName = (LPSTR)&lpSNT->TrueNameUC;
			
			SetGlobalValueLong ("%NETHOUSE",pMatch->HouseNum);
			SetGlobalValue ("%NETSTREET",lpTrueName);  
			if (pMatch->ZIP)
				SetGlobalValueLong ("%NETZIP",pMatch->ZIP);
			else
				SetGlobalValue ("%NETZIP",""); 
			GetMunicName (pMatch->Munic,MunName,MunAbv);
			SetGlobalValue ("%NETMUNIC",MunName);	 
			GetGlobalCVal ("[%NETADDRESS]",NetAdd,0);	
	    	sprintf (str,"%s\t%ld:%ld:%ld|%f,%f",NetAdd,
	    								 *pSNum,pMatch->HouseNum,pMatch->ZIP,
	    								 pMatch->Point.x,pMatch->Point.y);
	        SendDlgItemMessage (hDlg,IDM_STREET_MENU,LB_ADDSTRING,0,(LPARAM)str); 
	        NumDisplay++;
	        pMatch++; 
	    } 
	    GSSiGlobUlFree (&hMatch);
		if (GSSiPeekMessage(&msg,GetDlgItem(hDlg,EntryControl),WM_KEYDOWN,WM_KEYDOWN,PM_NOREMOVE))
			GetNext = FALSE;	                                                             
    }
    GlobalUnlock (hDBStreetNames);
    return (NumDisplay);
}

/*
    DWORD idx;
    short stName, stMax, ch, stSeg, i;
    long    MinHouseNum;
    char    StrName[33];
    char FullAddress[50];
    short   StrNum;  
    SEGMAXKEY	SegMaxKey;

    ch = 0;
    idx = SendDlgItemMessage (hDlg,IDM_STREET_MENU,
                                LB_RESETCONTENT,0,0);

    if (nchar<Trigger) return 0;
    _fstrncpy (StrName,InName,nchar);
    for (i=nchar;i<32;i++)
        StrName[i]='\0';
    stName = BT_FIND (hNames,StrName,BT_FIRST,BT_GE,(LPSTR)&StrNum);
    while (!stName)
    {   if (_fstrncmp (InName,StrName,nchar))
            stName = 31;
        else if (House)
        {   SegMaxKey.StreetNum = abs(StrNum);
            SegMaxKey.MaxHouseNum = House;
            SegMaxKey.Segid = 0;
            stMax = BT_FIND (hSegMax,(LPSTR)&SegMaxKey,BT_FIRST,BT_GT,(LPSTR)&MinHouseNum);
            CheckSeg: if (!stMax && SegMaxKey.StreetNum == abs(StrNum))
            {   if (House < MinHouseNum) goto NextSeg;
                stSeg = BT_FIND (hSegData,(LPSTR)&SegMaxKey.Segid,BT_FIRST,BT_GE,(LPSTR)&Segdata);
                if ((House >= min(Segdata.faddl,Segdata.taddl)
                     && House <= max(Segdata.faddl,Segdata.taddl) &&
                    House%2 == Segdata.faddl%2) ||
                    (House >= min(Segdata.faddr,Segdata.taddr)
                     && House <= max(Segdata.faddr,Segdata.taddr) &&
                    House%2 == Segdata.faddr%2))
                {
                    GetFullAdd (House,(LPSTR)&StrName,(LPSTR)&FullAddress);
                    sprintf (FullAddress+_fstrlen(FullAddress),"\t%ld",SegMaxKey.Segid);
                    SendDlgItemMessage (hDlg,IDM_STREET_MENU,LB_ADDSTRING,0,
                                        (LPARAM)&FullAddress);
                }
                else
                {   NextSeg: if (StrNum > 0) goto NextStreet;
                    stMax = BT_FIND (hSegMax,(LPSTR)&SegMaxKey,BT_NEXT,BT_ANY,(LPSTR)&MinHouseNum);
                    goto CheckSeg;
                }
            }
            NextStreet: stName = BT_FIND (hNames,StrName,BT_NEXT,BT_ANY,(LPSTR)&StrNum);
        }
        else
        {   _fstrcpy (FullAddress,StrName);
            sprintf (FullAddress+_fstrlen(FullAddress),"\t%d",StrNum);
            SendDlgItemMessage (hDlg,IDM_STREET_MENU,LB_ADDSTRING,0,
                                (LPARAM)&FullAddress);
            goto NextStreet;
        }
    }

    return (0);
} 
*/ 
void SetSecondaryIntInput(HWND hWnd)
{
	hWndSecondaryIntInput = hWnd;
	return;
}

short DisplayStreetsINT (HWND hDlg,USHORT iMenu, LPSTR InName,short nchar,UINT EntryControl,short FindOpt,BOOL CheckForSegs)
{
    short cond, pos, Index=TRUENAM_INDEX, NumDisplay=0,l;
    long    Offset;
    LPGWDHEADER lpGWDHead;   
    LPGWFLDINFO lpGWFldInfo;  
    LPSTREETNAMETABLE	lpSNT;
    HANDLE	hBT;
    LPLONG	pSNum; 
    BOOL	GetNext=TRUE;
    char	str[256], MatchName[64], Dir[4];  
    MSG		msg;  
    struct	{char	Name[32]; long SNum;}OrigNameKey; 
    LPSTR	pMatchName;
    
//    Index = SANSCH_INDEX;
    FindOpt = 0;//if had index on nameonly portion this might work
    if (!OpenStreetNameTable (FALSE))
    	return 0;
    SendDlgItemMessage (hDlg,iMenu,LB_RESETCONTENT,0,0);
    if (nchar < 1)
    	return 0;
    _fstrcpy (str,InName);
    _fstrupr (str);    
    _fstrncpy (MatchName,str,32);
	MatchName[32] = 0;
    if (FindOpt) 
    {   
    	short	lstr = _fstrlen (str);
    	
    	Index = NMONLY_INDEX;
    	PHONIC (str,&lstr);
    }
    OrigNameKey.SNum = LONG_MIN;
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetNames); 
    hBT = lpGWDHead->BTHandle[Index];
    lpGWFldInfo=lpGWDHead->pFldInfo; 
	SetFieldValFromChar(lpGWDHead, lpGWFldInfo, "0", FALSE, FALSE, TRUE);
    lpGWFldInfo+=Index; 
	SetFieldValFromChar(lpGWDHead, lpGWFldInfo, str, FALSE, FALSE, TRUE);
    GWDFormKey(lpGWDHead,Index,TRUE,0,0);   
    lpSNT = (LPSTREETNAMETABLE)&lpGWDHead->GWDData;
    pos = BT_FIRST;
    cond = BT_GE;
    while (GetNext && !BT_FIND (hBT,lpGWDHead->pKeys[Index],pos,cond, (LPSTR)&Offset))
    {
    	pos = BT_NEXT;
    	cond = BT_ANY;
    	if (_fstrnicmp (lpGWDHead->pKeys[Index],MatchName,nchar))
    		break; 
    	FillGWDData (lpGWDHead,Offset);
    	pSNum = (LPLONG)&lpGWDHead->GWDData;
    	sprintf (str,"%s\t%ld",lpSNT->TrueName,*pSNum);
		if (!CheckForSegs || GetNumStreetSegs_new (*pSNum,0,0,0,0,0,0,0))
		{
			if (SendDlgItemMessage (hDlg,iMenu,LB_FINDSTRINGEXACT,-1,(LPARAM)str) == LB_ERR) 
        		SendDlgItemMessage (hDlg,iMenu,LB_ADDSTRING,0,(LPARAM)str); 
			NumDisplay++;
		}
		if (GSSiPeekMessage(&msg,GetDlgItem(hDlg,EntryControl),WM_KEYDOWN,WM_KEYDOWN,PM_NOREMOVE))  
		{
			NumDisplay = -1;
			GetNext = FALSE;
		}
		if (hWndSecondaryIntInput)
		{
			if (GSSiPeekMessage(&msg, hWndSecondaryIntInput, WM_KEYDOWN, WM_KEYDOWN, PM_NOREMOVE))
			{
				NumDisplay = -1;
				GetNext = FALSE;
			}
		}
    }
    pos = BT_FIRST;
    cond = BT_GE; 
    l = _fstrlen (MatchName);  
    pMatchName = MatchName;
    if ( l > 2 &&
    	(!_fstrnicmp (MatchName,"N ",2) ||   
    	!_fstrnicmp (MatchName,"E ",2) ||                                          
    	!_fstrnicmp (MatchName,"S ",2) ||                                          
    	!_fstrnicmp (MatchName,"W ",2)))
    {
    	_fstrncpy (Dir,MatchName,1);
    	pMatchName = &MatchName[2];
    	nchar -=2;
    }
    else if (l > 3 &&
	    	(!_fstrnicmp (MatchName,"NE ",3) ||   
	    	!_fstrnicmp (MatchName,"SE ",3) ||                                          
	    	!_fstrnicmp (MatchName,"SW ",3) ||                                          
	    	!_fstrnicmp (MatchName,"NW ",3)))
    {
    	_fstrncpy (Dir,MatchName,2);
    	pMatchName = &MatchName[3];  
    	nchar -=3;
    } 
    _fstrncpy (OrigNameKey.Name,pMatchName,32);
    while (GetNext && !BT_FIND (hDBStreetNamesIndex9,(LPSTR)&OrigNameKey,pos,cond, (LPSTR)&Offset))
    {
    	pos = BT_NEXT;
    	cond = BT_ANY;
    	FillGWDData (lpGWDHead,Offset);
    	if (_fstrnicmp (lpSNT->OriginalNamePortion,pMatchName,nchar))
    		break; 
    	pSNum = (LPLONG)&lpGWDHead->GWDData;
    	sprintf (str,"%s\t%ld",lpSNT->TrueName,*pSNum);
 		if (!CheckForSegs || GetNumStreetSegs_new (*pSNum,0,0,0,0,0,0,0))
		{
			if (SendDlgItemMessage (hDlg,iMenu,LB_FINDSTRINGEXACT,-1,(LPARAM)str) == LB_ERR) 
        		SendDlgItemMessage (hDlg,iMenu,LB_ADDSTRING,0,(LPARAM)str); 
			NumDisplay++;
		}
		if (GSSiPeekMessage(&msg,GetDlgItem(hDlg,EntryControl),WM_KEYDOWN,WM_KEYDOWN,PM_NOREMOVE))  
		{
			NumDisplay = -1;
			GetNext = FALSE;
		}	                                                             
		if (hWndSecondaryIntInput)
		{
			if (GSSiPeekMessage(&msg, hWndSecondaryIntInput, WM_KEYDOWN, WM_KEYDOWN, PM_NOREMOVE))
			{
				NumDisplay = -1;
				GetNext = FALSE;
			}
		}
	}
    GlobalUnlock (hDBStreetNames);      
    
    return (NumDisplay);
}
/*void  OpenAddressFiles (HWND hWnd)
{   time_t      ltime;
    BTVARDESC BTVar[2];


    ltime = 0;
    hNames = BT_OPEN ("c:\\stnames2.btr", ltime, BT_READ, 0);
    hSegData = BT_OPEN ("c:\\lmdata.btr", ltime, BT_READ, 0);
    hSegMax = BT_OPEN ("c:\\segmax.btr",ltime, BT_READ, 0);
    if (!hSegMax) BuildSegMaxIndex (hWnd); */
/*  MarkStreetFile(hWnd);*/
/*
}

void    CloseAddressFiles (void)
{

    BT_CLOSE (hNames);
    hNames = 0;
    BT_CLOSE (hSegMax);
    hSegMax = 0;
    BT_CLOSE (hSegData);
    hSegData = 0;

} */


long PIDAddRefno (long Offset,LPSTR AddUDI)
{
    LPGWDHEADER lpGWDHead;
    LPGWFLDINFO lpGWFldInfo;
    long    iref;
    short       len, i;
    char    str[128];
    LPVOID  lpVal;
    
    lpGWDHead = (LPGWDHEADER) GlobalLock (hPIDAddDB); 
    GSSillseek (lpGWDHead->Fid,Offset,0);
    BigRead (lpGWDHead->Fid,(HPSTR)&len,2);
    BigRead (lpGWDHead->Fid,(HPSTR)&lpGWDHead->GWDData,len);
    SetGWDCurrentOffset (lpGWDHead,-1);
    
    iref = 0;  
    *AddUDI = 0;
    for (i=0,lpGWFldInfo=lpGWDHead->pFldInfo;i<lpGWDHead->NumFields;
         i++,lpGWFldInfo++)
    {   
        if (!_fstricmp(lpGWFldInfo->Name,AddUDIVar))
        {
            lpVal = &lpGWDHead->GWDData[lpGWFldInfo->Beg];
            switch (lpGWFldInfo->Type)
            {
                case BT_CHAR:  
                case BT_RIGHT_CHAR:
                    _fstrncpy (str,lpVal,lpGWFldInfo->Len);
                    str[lpGWFldInfo->Len]='\0';  
                    _fstrcpy(AddUDI,str);
                    iref = 1;
                break;
                                    
                case BT_INTEGER:
                    if (lpGWFldInfo->Len == 2)
                        iref = *(LPSHORT)lpVal;
                    else
                        iref = *(LPLONG)lpVal;
                    ltoa (iref,AddUDI,10);
                break;
            } 
            break;
        }
    }
    if (!*AddUDI && !iref)
    {   
        char    msg[128];
        
        sprintf (msg,"Field %s not in address location database", AddUDIVar);
        GSSiMsgBox( GetFocus(), msg,"Error in Address Location", MB_OK|MB_ICONEXCLAMATION,0);
        return (0);
    } 
    
    GlobalUnlock (hPIDAddDB); 
    return (iref);

}


void GetFullAdd (LONG House, LPSTR Street, LPSTR FullAddress)
{   
    FullAddress[0]='\0';
    wsprintf(FullAddress, "%6lu  %s",House,Street);
}

BOOL OpenSegMaxIndex (LPBOOL pOpened)
{   
	char	File[MAX_PATH];
	
	*pOpened = FALSE;
	if (hSegMax) return TRUE;
    SetAddressDir();
    sprintf (File,"%s\\segmax.btr",AddMatchDir);
    hSegMax = BT_OPEN (File, 0, BT_READ, 0); 
    if (hSegMax)
    {
    	*pOpened = TRUE;
    	return TRUE;  
    }
    else
    	return FALSE;
}

void CloseSegMaxIndex (BOOL Opened)
{   
	if (!Opened)
		return;
	BT_CLOSE (hSegMax);
	hSegMax = 0;
	return;
}

int AddSegMaxRecord (LPSEGMAXKEY pSegMaxKey,long SegMaxData)
{
	int	Overlap = 0;


	if (SegMaxData != 0 && pSegMaxKey->MaxHouseNum !=0)
	{
		long        SegMaxDataTest;
		SEGMAXKEY	SegMaxKeyTest=*pSegMaxKey;
		int			st=31;
		char		str[256];

		if (!BT_FIND (hSegMax,(LPSTR)&SegMaxKeyTest,BT_FIRST,BT_GE,(LPSTR)&SegMaxDataTest))
		{
			if (SegMaxKeyTest.StreetNum == pSegMaxKey->StreetNum && SegMaxKeyTest.StreetNum != 42389 &&
				SegMaxKeyTest.ZIPCode == pSegMaxKey->ZIPCode &&
				SegMaxKeyTest.Side == pSegMaxKey->Side &&
				SegMaxDataTest < pSegMaxKey->MaxHouseNum)
			{
				Overlap++;
				sprintf (str,"%i\t%i\t%i\t%i",pSegMaxKey->Segid,SegMaxKeyTest.Segid,SegMaxDataTest,pSegMaxKey->MaxHouseNum);
				AppendFile ("[%DL]address\\OverlappingSegments.txt",str);
			}
			st = BT_FIND (hSegMax,(LPSTR)&SegMaxKeyTest,BT_PRIOR,BT_ANY,(LPSTR)&SegMaxDataTest);
		}
		else
			st = BT_FIND (hSegMax,(LPSTR)&SegMaxKeyTest,BT_LAST,BT_ANY,(LPSTR)&SegMaxDataTest);
		if (!st && SegMaxKeyTest.StreetNum != 42389 &&
			SegMaxKeyTest.StreetNum == pSegMaxKey->StreetNum &&
			SegMaxKeyTest.ZIPCode == pSegMaxKey->ZIPCode &&
			SegMaxKeyTest.Side == pSegMaxKey->Side &&
			SegMaxData < SegMaxKeyTest.MaxHouseNum)
		{
			Overlap++;
			sprintf (str,"%i\t%i\t%i\t%i",pSegMaxKey->Segid,SegMaxKeyTest.Segid,SegMaxData,SegMaxKeyTest.MaxHouseNum);
			AppendFile ("[%DL]address\\OverlappingSegments.txt",str);
		}
		BT_PUT (hSegMax,(LPSTR)pSegMaxKey,(LPSTR)&SegMaxData);  
	}
	return Overlap;
}


BOOL BuildSegMaxIndex (HWND hWnd)
{   BTVARDESC   BTVar[5];
	LPGWDHEADER	lpGWDHead;
	NETLINKSKEY	NetLinksKey; 
    long        nRecs, nLoaded=0;
    HDC         hDC;
    short       stSeg;
    BOOL        RtnVal=FALSE, Opened, Reversed;
    long        SegMaxData=0;
    SEGMAXKEY	SegMaxKey;
    char    	File[MAX_PATH]; 
    LPSEGDATAGM	pSegdata; 
    long		Offset;  
    short		pos=BT_FIRST,i, Side1,Side2;
	NETLINKSDATA	NetLinksData; 
	BOOL		OpenedSeg; 
	DPOINT		DPoint;
	double		AZ, Length;   
	long		debugid=1800013461, ii;
	int			nOverlaps=0;
	char		str[128];

	if (!OpenStreetSegmentTable (FALSE,&OpenedSeg)) 
	{
    	GSSiMsgBox (GetFocus(),"Unable to open street segment table",0,MB_ICONEXCLAMATION,0);
		return FALSE;
	}
/*	if (!OpenNetLinkAndRef (NetworkID,FALSE,&Opened))
	{
    	GSSiMsgBox (GetFocus(),"Unable to open street network",0,MB_ICONEXCLAMATION);
		return FALSE; 
	}*/ 
	
	GSSiRemove ("[%DL]address\\OverlappingSegments.txt");
	sprintf (str,"Seg1\tSeg2\tAdd1\tAdd2");
	AppendFile ("[%DL]address\\OverlappingSegments.txt",str);

    sprintf (File,"%s\\segmax.btr",AddMatchDir);
    BTVar[0].BT_VARTYP=BT_INTEGER;
    BTVar[0].BT_VARLEN=4;
    BTVar[0].BT_VAROFF=0;
    BTVar[1].BT_VARTYP=BT_INTEGER;
    BTVar[1].BT_VARLEN=4;
    BTVar[1].BT_VAROFF=4;
    BTVar[2].BT_VARTYP=BT_INTEGER;
    BTVar[2].BT_VARLEN=2;
    BTVar[2].BT_VAROFF=8;
    BTVar[3].BT_VARTYP=BT_INTEGER;
    BTVar[3].BT_VARLEN=4;
    BTVar[3].BT_VAROFF=10;
    BTVar[4].BT_VARTYP=BT_INTEGER;
    BTVar[4].BT_VARLEN=4;
    BTVar[4].BT_VAROFF=14;
    BT_CREATE (File, 4, FALSE, 5, 1,(LPBTVARDESC) BTVar,FALSE, 0, 0, FALSE);
    hSegMax = BT_OPEN (File, 0, BT_WRITE, 0);

	lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetSegments); 
	pSegdata = (LPSEGDATAGM)&lpGWDHead->GWDData;
	nRecs = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[0]);
	CreateStatusWind (hWnd,1,"Create Address Index Table");
    while (ContinueProcessing && !BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&SegMaxKey.Segid,pos,BT_ANY,(LPSTR)&Offset))
    {   
    	pos = BT_NEXT;
		FillGWDData (lpGWDHead,Offset); 
		if (SegMaxKey.Segid == debugid)
			ii=1;
		for (i=0;i<4;i++)
		{   
			if (pSegdata->StreetNum[i])
			{
/*6/8/04				NetLinksKey.Ref = SegMaxKey.Segid;
				NetLinksKey.NetID = NetworkID;
				NetLinksKey.Path = pSegdata->StreetNum[0];  
				Reversed = FALSE;
				if (!BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_EQ,(LPSTR)&NetLinksData))
				{
					if (NetLinksData.Length < 0)
						Reversed = TRUE;
				}
				else
					goto Next; */
/*				if (Reversed) 
				{
					Side1=1;
					Side2=2;
				}
				else
				{ 
					Side1=2;
					Side2=1;
				} */ //   1/31/04
		    	SegMaxKey.StreetNum=pSegdata->StreetNum[i];
// 1/31/04		    	SegMaxKey.Side = Side1;
		    	SegMaxKey.ZIPCode = pSegdata->ZIPL;
		        SegMaxKey.MaxHouseNum = max (pSegdata->faddl,pSegdata->taddl); 
		        SegMaxKey.Side = SegMaxKey.MaxHouseNum % 2 + 1;
		        SegMaxData = min (pSegdata->faddl,pSegdata->taddl);
				nOverlaps += AddSegMaxRecord (&SegMaxKey,SegMaxData);
// 1/31/03		    	SegMaxKey.Side = Side2;
		    	SegMaxKey.ZIPCode = pSegdata->ZIPR;
		        SegMaxKey.MaxHouseNum = max (pSegdata->faddr,pSegdata->taddr);
		        SegMaxKey.Side = SegMaxKey.MaxHouseNum % 2 + 1;
		        SegMaxData = min (pSegdata->faddr,pSegdata->taddr);
				nOverlaps += AddSegMaxRecord (&SegMaxKey,SegMaxData);
		  Next:; 
		    } 
	    }
		StatusWindowUpdate (0,0, nRecs, ++nLoaded);
    }
    SetContinueProcessing ( TRUE);
    BT_CLOSE (hSegMax);  
    hSegMax = 0;
    GlobalUnlock (hDBStreetSegments);
    CloseStreetSegmentTable(OpenedSeg); 
//    CloseNetLinkAndRef (Opened); 
	DestroyStatusWindow(0); 
	if (nOverlaps)
	{
		char	mess[128];

		sprintf (mess,"%i overlaps detected",nOverlaps);
		MessageBox (0,mess,0,MB_ICONEXCLAMATION);
	}
    return TRUE;
}

void HighlightStreet (HWND hWnd, long StrNum)
{   short       stMax;
    long    MinHouseNum;
    BOOL    AP;
    SEGMAXKEY	SegMaxKey;
	BOOL	OpenBP;

    CloseMap(FALSE);
    OpenPlotFile ();
//    OpenAddressFiles (hWnd);
    OpenRefIndex (FALSE);
    OpenBP = OpenBasePens();
    SegMaxKey.StreetNum = (short) abs((int)StrNum);
    SegMaxKey.MaxHouseNum = 0;
    SegMaxKey.Segid = 0;
    AP = SetAutoPan (FALSE);
    stMax = BT_FIND (hSegMax,(LPSTR)&SegMaxKey,BT_FIRST,BT_GE,(LPSTR)&MinHouseNum);
    while (!stMax && SegMaxKey.StreetNum == (short)abs((int)StrNum))
    {   PickByRefno (SegMaxKey.Segid,0,0,-1);
        stMax = BT_FIND (hSegMax,(LPSTR)&SegMaxKey,BT_NEXT,BT_ANY,(LPSTR)&MinHouseNum);
    }
    SetAutoPan (AP);
    ClosePlotFile ();
    CloseBasePens(OpenBP);
    CloseRefIndex (FALSE);  
    return;
}

BOOL CreateStreetNameTable (LPSTR Dir)
{
    BTVARDESC  *pVars;
    static  short       NumFields, Reclen, len;
    GWDHEADER16 GWDHead; 
    LPGWDHEADER16 lpGWDHead;
    HANDLE  hVars, hDB;
    int      FidData,ibeg,NumVars,i;
    OFSTRUCTGM    OFStruct;
    GWFLDINFO FldInfo;
    char    File[MAX_PATH], PrimeIndex[MAX_PATH], RemoveName[MAX_PATH];
    
    sprintf (File,"%s\\strname.gmd",Dir);
    sprintf (PrimeIndex,"%s\\strname.in1",Dir);
    for (i=2;i<9;i++)
    {
	    sprintf (RemoveName,"%s\\strname.in%i",Dir,i);   
	    GSSiRemove (RemoveName);
    }
    lpGWDHead = &GWDHead; 
    
    FidData = GSSiOpenFile (File,&OFStruct,OF_CREATE);   
    _fmemset (lpGWDHead,0,sizeof(GWDHEADER16));
    GWDHead.NumFields=0;
    GWDHead.NumIndex=8;
    GWDHead.Version=2;
    GWDHead.NumIndexFields[0]=1;
    GWDHead.IndexFields[0][0]=0;
    GWDHead.NumIndexFields[1]=2;
    GWDHead.IndexFields[1][0]=1;
    GWDHead.NumIndexFields[2]=2;
    GWDHead.IndexFields[2][0]=2;
    GWDHead.NumIndexFields[3]=2;
    GWDHead.IndexFields[3][0]=3;
    GWDHead.NumIndexFields[4]=2;
    GWDHead.IndexFields[4][0]=4;
    GWDHead.NumIndexFields[5]=2;
    GWDHead.IndexFields[5][0]=5;
    GWDHead.NumIndexFields[6]=2;
    GWDHead.IndexFields[6][0]=6;
    GWDHead.NumIndexFields[7]=2;
    GWDHead.IndexFields[7][0]=7;
    BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
    ibeg = 0;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"%STREET_NUM");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 32;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"TrueStreetNameUC");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 32;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"StandardStreetName");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 32;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"NamePortionOfStandardName");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 32;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"StandardNameWOHeading");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 32;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"StandardNameWOType");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 32;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"NonReorderedNameNoPrefixPortion");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 32;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"NonReorderedNameNoSuffixPortion");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 32;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"NonReorderedNamePortion");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 32;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"NonCompressedNamePortion");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 32;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"OriginalNamePortion");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"FEDIRP");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 30;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"FENAME");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"FETYPE");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"FEDIRS");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 32;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"TrueStreetName");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

     GWDHead.Reclen=ibeg; 
     GWDHead.TimeStamp = time(0);
     GSSillseek (FidData,0,0);
     BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
     GSSillseek (FidData,0,2);
                 
     NumVars = 1;
            
     hVars = GlobalAlloc (LHND,NumVars * sizeof(BTVARDESC));
     pVars =(LPBTVARDESC) GlobalLock(hVars);
            
     pVars->BT_VARLEN=4;
     pVars->BT_VARTYP=BT_INTEGER;
     pVars->BT_VAROFF=0;
     BT_CREATE (PrimeIndex, 4, FALSE, 1, 1,pVars,FALSE, 0, GWDHead.TimeStamp, FALSE);
     GlobalUnlock(hVars);
     GlobalFree(hVars); 
     GSSiClose2 (&FidData);
                 
     hDB = OpenGWDatabase (File,BT_WRITE);
     if (!hDB) return (FALSE);
     CloseGWDatabase (hDB); 
     return TRUE;
}

BOOL OpenPointAddressTable (BOOL Update)
{   
    char    File[MAX_PATH];
    static  BOOL    OpenMode;
    short       Mode;
    
    if (hPointAddress && (OpenMode==Update || OpenMode==BT_WRITE))
        return TRUE; 
    SetAddressDir();
    ClosePointAddressTable();                  
    
    sprintf (File,"%s\\pointadd.btr",AddMatchDir);
    if (!ExistFile(File))
    {   
    	DWORD	Err;
    	
    	if (GSSiMakeDir (AddMatchDir,&Err))
    		InitAddMatchDir (AddMatchDir);
        if (!CreatePointAddressTable (File))
        {
            return FALSE;
        } 
    }
    if (Update)
        Mode = BT_WRITE;
    else
        Mode = BT_READ;
    if (!(hPointAddress = BT_OPEN (File,0,Mode,0)))
    {
        return FALSE;
    }  
    OpenMode = Update;
    
    return TRUE;
} 

void ClosePointAddressTable (void)
{
	if (!hPointAddress)
		return;
	BT_CLOSE (hPointAddress);
	hPointAddress = 0;
	return;
}

BOOL CreatePointAddressTable (LPSTR Name)
{    
	BTVARDESC BTVar[2]; 

	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=4; 
	BT_CREATE (Name, sizeof(double), FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	return TRUE;
}

void CloseStreetNameTable (void)
{
    if (!hDBStreetNames) return; 
    CloseGWDatabase (hDBStreetNames);
    hDBStreetNames=0;
    BT_CLOSE (hDBStreetNamesIndex9); 
    hDBStreetNamesIndex9 = 0;
    return;
}

BOOL OpenStreetNameTable (BOOL Update)
{   
    char    File[MAX_PATH];
    static  BOOL    OpenMode;
    short       Mode, pos;
    LPGWDHEADER lpGWDHead;   
	BTVARDESC BTVar[2]; 
	LPSTREETNAMETABLE	lpSNT; 
	long	Offset;  
    
    if (hDBStreetNames && (OpenMode==Update || OpenMode==BT_WRITE))
        return TRUE; 
    SetAddressDir();
    CloseStreetNameTable();                  
    
    sprintf (File,"%s\\strname.gmd",AddMatchDir);
    if (!ExistFile(File))
    {   
    	DWORD	Err;
    	
    	if (GSSiMakeDir (AddMatchDir,&Err))
    		InitAddMatchDir (AddMatchDir);
        if (!CreateStreetNameTable (AddMatchDir))
        {
            return FALSE;
        } 
    }
    if (Update)
        Mode = BT_WRITE;
    else
        Mode = BT_READ;
    if (!(hDBStreetNames = OpenGWDatabase (File,Mode)))
    {
        return FALSE;
    }  
    OpenMode = Update;
    BlankStreet=GetStreetNumFromName ("",1,BT_FIRST, 0); 
    if (OpenMode == BT_READ)
    {
	    struct	{char	Name[32];
	    		 long	SNum;} Index9Key;

	    lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetNames); 
	    		  
	    sprintf (File,"%s\\strname.in9",AddMatchDir);
    	if ((hDBStreetNamesIndex9 = BT_OPEN (File,0,BT_READ,0)))
    	{
			if (BT_CHECK_TIME_STAMP (hDBStreetNamesIndex9, lpGWDHead->TimeStamp))
				goto Exit;  
			BT_CLOSEANDDELETE (&hDBStreetNamesIndex9);
		}
    	lpSNT = (LPSTREETNAMETABLE)&lpGWDHead->GWDData;

		BTVar[0].BT_VARTYP=BT_CHAR;
		BTVar[0].BT_VARLEN=32;
		BTVar[0].BT_VAROFF=0;
		BTVar[1].BT_VARTYP=BT_INTEGER;
		BTVar[1].BT_VARLEN=4;
		BTVar[1].BT_VAROFF=32; 
		BT_CREATE (File, 4, FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
    	hDBStreetNamesIndex9 = BT_OPEN (File,0,BT_WRITE,0);
	    pos = BT_FIRST;
	    while (!BT_FIND (lpGWDHead->BTHandle[0],lpGWDHead->pKeys[0],pos,BT_ANY, (LPSTR)&Offset))
	    {    
	    	
	    	pos = BT_NEXT;
	    	FillGWDData (lpGWDHead,Offset);    
	    	_fstrncpy (Index9Key.Name,lpSNT->OriginalNamePortion,32);
	    	Index9Key.SNum = lpSNT->StreetNum;
			BT_PUT (hDBStreetNamesIndex9,(LPSTR)&Index9Key,(LPSTR)&Offset);
		}
		BT_SET_TIME_STAMP (hDBStreetNamesIndex9, lpGWDHead->TimeStamp); 
		BT_CLOSE (hDBStreetNamesIndex9);
		hDBStreetNamesIndex9 = BT_OPEN (File,0,BT_READ,0);   
Exit:    
		GlobalUnlock (hDBStreetNames); 
	}
    return TRUE;
}

void InitAddMatchDir (LPSTR AddMatchDir)
{   
	char	FromFile[MAX_PATH]="[%INDIR]address\\abbrname.txt";
	char	ToFile[MAX_PATH];
	
	sprintf (ToFile,"%s\\abbrname.txt",AddMatchDir);
	ExpandText (FromFile);
	ExpandText (ToFile);
	if (!ExistFile(ToFile))
		copyfile (ToFile,FromFile,FALSE,0,0,0,0,0,0);
	return;
}  

BOOL ConvertSSTV2toV3 (LPSTR File)
{   
	char	OldName[MAX_PATH], IndexName[MAX_PATH];  
	short	pos=BT_FIRST, len;
	HANDLE	hDBold;
    LPGWDHEADER lpGWDHead, lpGWDHeadOld; 
    LPSEGDATAGM	pSegdata; 
    long	TLID, Offset, TotLen,CurLoc=0;
    BOOL	rtn=FALSE, OpenedSeg;   
    LPSTR	lpDot, lpDotIndex;
	
	_fstrcpy (OldName,File);
	_fstrlwr (OldName);
	_fstrcpy (IndexName,OldName);
	lpDot = _fstrstr (OldName,".gmd");
	*lpDot = 0;
	_fstrcat (OldName,"1.in1");
	lpDotIndex = _fstrstr (IndexName,".gmd");
	*lpDotIndex = 0;
	_fstrcat (IndexName,".in1");
	if (!GSSiRename (IndexName,OldName))
	{    
Mess1:  
		setDoPaint( FALSE);
    	GSSiMsgBox (GetFocus(),"Cannot convert street seg file - file exists",0,MB_ICONEXCLAMATION,0);
        BlowOut(0,0);
	}
	*lpDot = 0;
	_fstrcat (OldName,"1.gmd");
	if (!GSSiRename (File,OldName))
		goto Mess1;
	
	CreateStatusWind (hWndMain,1,0);
    hDBold= OpenGWDatabase (OldName,BT_READ);
	lpGWDHeadOld = (LPGWDHEADER)GlobalLock (hDBold); 
	OpenStreetSegmentTable (TRUE,&OpenedSeg);
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetSegments);  
	pSegdata = (LPSEGDATAGM)&lpGWDHead->GWDData;
	TotLen = GSSillseek (lpGWDHeadOld->Fid,0,2);
    while (!BT_FIND (lpGWDHeadOld->BTHandle[0],(LPSTR)&TLID,pos,BT_ANY,(LPSTR)&Offset))
    {   
    	pos = BT_NEXT;  
    	len = FillGWDData (lpGWDHeadOld,Offset);
	  	CurLoc += lpGWDHeadOld->Reclen;
        Offset = GSSillseek (lpGWDHead->Fid,0,2);
        BT_PUT (lpGWDHead->BTHandle[0],(LPSTR)&TLID,(LPSTR)&Offset);  
	  	len = sizeof(SEGDATAGM);
	   	if (BigWrite(lpGWDHead->Fid,(HPSTR)&len,2,-1)!=2)
	   		goto ErrOut; 
	   	pSegdata->Filler2 = 0;
	   	pSegdata->HighAddrL = pSegdata->HighAddrR = 0; 
	   	pSegdata->LowAddrL = pSegdata->LowAddrR = 0; 
	   	pSegdata->LowOffL = pSegdata->LowOffR = 0; 
	   	pSegdata->HighOffL = pSegdata->HighOffR = 0; 
	   	if (BigWrite (lpGWDHead->Fid,(HPSTR)pSegdata,len,-1) != len)
	   		goto ErrOut; 
		StatusWindowUpdate ("","Converting street segment table",TotLen,CurLoc);
    }
    rtn=TRUE;  
ErrOut: 
	GlobalUnlock (hDBStreetSegments);
    CloseGWDatabase (hDBold);   
    if (!rtn)
    {
		setDoPaint( FALSE);
    	GSSiMsgBox (GetFocus(),"Error converting street seg file - disk full",0,MB_ICONEXCLAMATION,0);
        BlowOut(0,0);
    }
	CloseStreetSegmentTable (OpenedSeg);
    GSSiRemove (OldName);
	*lpDot = 0;
	_fstrcat (OldName,"1.in1");
    GSSiRemove (OldName);  
	DestroyStatusWindow(0);  
    return TRUE;
}

BOOL ConvertSSTV3toV4 (LPSTR File)
{   
	char	OldName[MAX_PATH], IndexName[MAX_PATH];  
	short	pos=BT_FIRST, len;
	HANDLE	hDBold;
    LPGWDHEADER lpGWDHead, lpGWDHeadOld; 
    LPSEGDATAGM	pSegdata; 
    long	TLID, Offset, TotLen,CurLoc=0;
    BOOL	rtn=FALSE, OpenedSeg;   
    LPSTR	lpDot, lpDotIndex;
	
	_fstrcpy (OldName,File);
	_fstrlwr (OldName);
	_fstrcpy (IndexName,OldName);
	lpDot = _fstrstr (OldName,".gmd");
	*lpDot = 0;
	_fstrcat (OldName,"1.in1");
	lpDotIndex = _fstrstr (IndexName,".gmd");
	*lpDotIndex = 0;
	_fstrcat (IndexName,".in1");
	if (!GSSiRename (IndexName,OldName))
	{    
Mess1:  
		setDoPaint( FALSE);
    	GSSiMsgBox (GetFocus(),"Cannot convert street seg file - file exists",0,MB_ICONEXCLAMATION,0);
        BlowOut(0,0);
	}
	*lpDot = 0;
	_fstrcat (OldName,"1.gmd");
	if (!GSSiRename (File,OldName))
		goto Mess1;
	
	CreateStatusWind (hWndMain,1,"Converting street segment table");
    hDBold= OpenGWDatabase (OldName,BT_READ);
	lpGWDHeadOld = (LPGWDHEADER)GlobalLock (hDBold); 
	OpenStreetSegmentTable (TRUE,&OpenedSeg);
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetSegments);  
	pSegdata = (LPSEGDATAGM)&lpGWDHead->GWDData;
	TotLen = BT_NUM_IN_INDEX (lpGWDHeadOld->BTHandle[0]);
    while (!BT_FIND (lpGWDHeadOld->BTHandle[0],(LPSTR)&TLID,pos,BT_ANY,(LPSTR)&Offset))
    {   
    	pos = BT_NEXT;
	    GSSillseek (lpGWDHeadOld->Fid,Offset,0);
	    BigRead (lpGWDHeadOld->Fid,(HPSTR)&len,2);
	    BigRead (lpGWDHeadOld->Fid,(HPSTR)&lpGWDHead->GWDData,len);
        Offset = GSSillseek (lpGWDHead->Fid,0,2);
        BT_PUT (lpGWDHead->BTHandle[0],(LPSTR)&TLID,(LPSTR)&Offset);  
	  	len = sizeof(SEGDATAGM);
	   	if (BigWrite (lpGWDHead->Fid,(HPSTR)&len,2,-1)!=2)
	   		goto ErrOut; 
	   	pSegdata->UpdateTime = 0;
	   	pSegdata->UpdateID = 0;
	   	if (BigWrite (lpGWDHead->Fid,(HPSTR)pSegdata,len,-1) != len)
	   		goto ErrOut; 
		StatusWindowUpdate (0,0,TotLen,++CurLoc);
    }
    rtn=TRUE;  
ErrOut: 
	GlobalUnlock (hDBStreetSegments);
    CloseGWDatabase (hDBold);   
    if (!rtn)
    {
		setDoPaint( FALSE);
    	GSSiMsgBox (GetFocus(),"Error converting street seg file - disk full",0,MB_ICONEXCLAMATION,0);
        BlowOut(0,0);
    }
	CloseStreetSegmentTable (OpenedSeg);
    GSSiRemove (OldName);
	*lpDot = 0;
	_fstrcat (OldName,"1.in1");
    GSSiRemove (OldName);  
	DestroyStatusWindow(0);  
    return TRUE;
}

BOOL OpenStreetSegmentTable (BOOL Update,LPBOOL Opened)
{   
    char    File[MAX_PATH];
    static  BOOL    OpenMode;
    short       Mode, Version;
    LPGWDHEADER lpGWDHead;    
    
    *Opened = 0;
Top:						
    if (hDBStreetSegments && (OpenMode==Update || OpenMode==BT_WRITE))
        return TRUE; 
    SetAddressDir();
    CloseStreetSegmentTable(TRUE);                  
    
    sprintf (File,"%s\\strtseg.gmd",AddMatchDir);
    if (!ExistFile(File))
    {   
    	DWORD	Err;
    	
    	if (GSSiMakeDir (AddMatchDir,&Err))
    		InitAddMatchDir (AddMatchDir);
        if (!CreateStreetSegmentTable (File))
        {
            return FALSE;
        } 
    }
    if (Update)
        Mode = BT_WRITE;
    else
        Mode = BT_READ;
    if (!(hDBStreetSegments = OpenGWDatabase (File,Mode)))
    {
        return FALSE;
    }  
    *Opened = TRUE;
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetSegments);  
    Version = lpGWDHead->Version;
    GlobalUnlock (hDBStreetSegments);
/*    if (Version == 1)
    {
	    CloseStreetSegmentTable();   
	    ConvertSSTV1toV2 (File);
	    goto Top;
	} */              
    if (Version == 2)
    {
	    CloseStreetSegmentTable(TRUE);   
	    ConvertSSTV2toV3 (File);
	    goto Top;
	}               
    if (Version == 3)
    {
	    CloseStreetSegmentTable(TRUE);   
	    ConvertSSTV3toV4 (File);
	    goto Top;
	}               
    
    OpenMode = Update;
    return TRUE;
}   


void CloseStreetSegmentTable (BOOL Opened)
{
    if (!Opened || !hDBStreetSegments) return; 
    CloseGWDatabase (hDBStreetSegments);
    hDBStreetSegments=0; 
    return;
}

BOOL CreateStreetSegmentTable (LPSTR File)
{
 	int		i;
	BTVARDESC BTVar[2], *pVars;
	static	int		NumFields, Reclen, len;
	long	Refno, Offset;
	long	TotFileLen;
	GWDHEADER16 GWDHead; 
	LPGWDHEADER16	lpGWDHead;
	GWFLDINFO GWFldInfo;
	LPGWFLDINFO	lpGWFldInfo, lpCTField, lpMCDField, lpMAField;
	HANDLE hBT, hVars, hDB, hBlock, hFldInLen,hBTOld, hDBma, hDBct;
	int	FidOld;
	time_t ltime;
	int		FidData;
	int		ibeg,NumVars,NumIndex;
	OFSTRUCTGM	OFStruct;
	LPVOID	lpVal;
	LPSTR	pName;
	GWFLDINFO FldInfo;
	long	SaveFrame, TLID, Offsetct, Offsetsa;
	int		st, lenct, lenma; 
	char	FileIn1[MAX_PATH]; 
	LPSTR	lpDot;  

	if (ExistFile(File))
	{
		HaltMapDisplay(FALSE,FALSE);
		GSSiMsgBox( GetFocus(),File,"Trying to create existing file", MB_OK,0);
        BlowOut(0,0);
	}
	 _fstrcpy (FileIn1,File);
	 lpDot = _fstrrchr (FileIn1,'.');
	 if (lpDot)
	 	*lpDot = 0;
	 _fstrcat (FileIn1,".in1");	
	 lpGWDHead = &GWDHead; 

	 FidData = GSSiOpenFile (File,&OFStruct,OF_CREATE);
	 GWDHead.NumFields=0;
	 GWDHead.NumIndex=5;
	 GWDHead.Unused=0;
	 GWDHead.Version=4;
	 GWDHead.NumIndexFields[0]=1;
	 GWDHead.IndexFields[0][0]=0;  
	 GWDHead.lKeys[0]=4;
	 GWDHead.NumIndexFields[1]=2;
	 GWDHead.IndexFields[1][0]=1;  
	 GWDHead.IndexFields[1][1]=0;  
	 GWDHead.NumIndexFields[2]=2;
	 GWDHead.IndexFields[2][0]=2;  
	 GWDHead.IndexFields[2][1]=0;  
	 GWDHead.NumIndexFields[3]=2;
	 GWDHead.IndexFields[3][0]=3;  
	 GWDHead.IndexFields[3][1]=0;  
	 GWDHead.NumIndexFields[4]=2;
	 GWDHead.IndexFields[4][0]=4;  
	 GWDHead.IndexFields[1][1]=0;  
	 GWDHead.lKeys[1]=8;
	 GWDHead.lKeys[2]=8;
	 GWDHead.lKeys[3]=8;
	 GWDHead.lKeys[4]=8;
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
	ibeg = 0;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"TLID");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"StreetNum1");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"StreetNum2");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"StreetNum3");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"StreetNum4");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"FRADDL");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"FRADDR");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"TOADDL");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"TOADDR");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"ZIPL");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"ZIPR");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"STATEL");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"STATER");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"COUNTYL");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"COUNTYR");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"FPLL");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"FPLR");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 6;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"CTBNAL");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 6;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"CTBNAR");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"BLKL");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"BLKR");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"FMCDL");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"FMCDR");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;


	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"TrafficVol");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"FromTLID");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"NumLanes");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"UpdateTime");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"UpdateID");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"ChangedFlag");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_REAL;
	_fstrcpy (FldInfo.Name,"Width");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"Speed");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"OneWay");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 3;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"CFCC");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 1;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"Filler2");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"LowAddL");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"LowAddR");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"HighAddL");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"HighAddR");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"LowOffL");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"LowOffR");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"HighOffL");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"HighOffR");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	 GWDHead.Reclen=ibeg; 
	 GWDHead.TimeStamp = time(0);
	 GSSillseek (FidData,0,0);
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
 	 GSSillseek (FidData,0,2);
			     
	 NumVars = 1;
	 NumIndex = 1;
			
	 hVars = LocalAlloc (LHND,NumVars * sizeof(BTVARDESC));
     pVars = (LPBTVARDESC)LocalLock(hVars);
			
	 pVars->BT_VARLEN=4;
	 pVars->BT_VARTYP=BT_INTEGER;
	 pVars->BT_VAROFF=0;
	 BT_CREATE (FileIn1, 4, FALSE, 1, 1,pVars,FALSE, 0, GWDHead.TimeStamp, FALSE);
     LocalUnlock(hVars);
     LocalFree(hVars);
 	 GSSiClose2 (&FidData);
     return TRUE;
} 

BOOL CreateINT_MATCHTable (LPSTR File,LPSTR BadNames,short OrigKeyLen)
{
 	int		i;
	BTVARDESC BTVar[2], *pVars;
	static	int		NumFields, Reclen, len;
	long	Refno, Offset;
	long	TotFileLen;
	GWDHEADER16 GWDHead; 
	LPGWDHEADER16	lpGWDHead;
	GWFLDINFO GWFldInfo;
	LPGWFLDINFO	lpGWFldInfo, lpCTField, lpMCDField, lpMAField;
	HANDLE hBT, hVars, hDB, hBlock, hFldInLen,hBTOld, hDBma, hDBct;
	int	FidOld;
	time_t ltime;
	int		FidData;
	int		ibeg,NumVars,NumIndex;
	OFSTRUCTGM	OFStruct;
	LPVOID	lpVal;
	LPSTR	pName;
	GWFLDINFO FldInfo;
	long	SaveFrame, TLID, Offsetct, Offsetsa;
	int		st, lenct, lenma; 
	char	FileIn1[MAX_PATH], FileIn2[MAX_PATH]; 
	LPSTR	lpDot;  

	 _fstrcpy (FileIn1,File);
	 lpDot = _fstrrchr (FileIn1,'.');
	 if (lpDot)
	 	*lpDot = 0;
	 _fstrcpy (FileIn2,FileIn1);
	 _fstrcpy (BadNames,FileIn1);
	 _fstrcat (FileIn1,".in1");	
	 _fstrcat (FileIn2,".in2");	  
	 _fstrcat (BadNames,".bnf");	  
	 GSSiRemove (FileIn2);
	 lpGWDHead = &GWDHead; 
    _fmemset (lpGWDHead,0,sizeof(GWDHEADER16));

	 FidData = GSSiOpenFile (File,&OFStruct,OF_CREATE);
	 GWDHead.NumFields=0;
	 GWDHead.NumIndex=2;
	 GWDHead.Version=1;
	 GWDHead.NumIndexFields[0]=1;
	 GWDHead.IndexFields[0][0]=0;
	 GWDHead.NumIndexFields[1]=-2; // was 2
	 GWDHead.IndexFields[1][0]=1;
	 GWDHead.IndexFields[1][1]=0;
	 GWDHead.lKeys[1]=6;//SHRT_MIN;  //creates as non-unique
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
	ibeg = 0;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"RecordNumber");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"MatchCode");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"LocationCode");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"StreetNum");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"HouseNum");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"IntersectionID");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"StreetNum1");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"StreetNum2");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"MunicNum");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"ZIP");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_REAL;
	_fstrcpy (FldInfo.Name,"X");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_REAL;
	_fstrcpy (FldInfo.Name,"Y");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"MunicChanged");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"ZIPChanged");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_REAL;
	_fstrcpy (FldInfo.Name,"Dist");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_REAL;
	_fstrcpy (FldInfo.Name,"Direction");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_REAL;
	_fstrcpy (FldInfo.Name,"Offset");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 32;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"StreetA");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 32;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"StreetB");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = OrigKeyLen;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"OriginalFileKey");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 40;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"Symbol");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 32;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"FromDate");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 32;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"ToDate");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	 GWDHead.Reclen=ibeg; 
	 GWDHead.TimeStamp = time(0);
	 GSSillseek (FidData,0,0);
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
 	 GSSillseek (FidData,0,2);
			     
	 NumVars = 1;
	 NumIndex = 2;
			
	 hVars = LocalAlloc (LHND,NumVars * sizeof(BTVARDESC));
     pVars = (LPBTVARDESC)LocalLock(hVars);
			
	 pVars->BT_VARLEN=4;
	 pVars->BT_VARTYP=BT_INTEGER;
	 pVars->BT_VAROFF=0;
	 BT_CREATE (FileIn1, 4, FALSE, 1, 1,pVars,FALSE, 0, GWDHead.TimeStamp, FALSE);
     LocalUnlock(hVars);
     LocalFree(hVars);
 	 GSSiClose2 (&FidData);

     hDB = OpenGWDatabase (File,BT_WRITE);
     if (!hDB) return (FALSE);
     CloseGWDatabase (hDB); 
	 BTVar[0].BT_VARLEN=40;
	 BTVar[0].BT_VARTYP=BT_CHAR;
	 BTVar[0].BT_VAROFF=0;
	 BTVar[1].BT_VARLEN=4;
	 BTVar[1].BT_VARTYP=BT_INTEGER;
	 BTVar[1].BT_VAROFF=40;
	 BT_CREATE (BadNames, 2, FALSE, 2, 1,BTVar,FALSE, 0, GWDHead.TimeStamp, FALSE);
     return TRUE;


} 

BOOL CreateADD_MATCHTable (LPSTR File,LPSTR BadNames,LPSTR KeyDef,short AddUDILength)
{
 	int		i;
	BTVARDESC BTVar[2], *pVars;
	static	int		NumFields, Reclen, len;
	long	Refno, Offset;
	long	TotFileLen;
	GWDHEADER16 GWDHead; 
	LPGWDHEADER16	lpGWDHead;
	GWFLDINFO GWFldInfo;
	LPGWFLDINFO	lpGWFldInfo, lpCTField, lpMCDField, lpMAField;
	HANDLE hBT, hVars, hDB, hBlock, hFldInLen,hBTOld, hDBma, hDBct;
	int	FidOld;
	time_t ltime;
	int		FidData;
	int		ibeg,NumVars,NumIndex;
	OFSTRUCTGM	OFStruct;
	LPVOID	lpVal;
	LPSTR	pName;
	GWFLDINFO FldInfo;
	long	SaveFrame, TLID, Offsetct, Offsetsa;
	int		st, lenct, lenma; 
	char	FileIn1[MAX_PATH], FileIn2[MAX_PATH]; 
	LPSTR	lpDot;
	BOOL	rtn=FALSE; 
	char	DefStr[]=",MatchCode(B2),LocationCode(B2),StreetNum(B4),HouseNum(B4),IntID(B4),StreetNum1(B4),StreetNum2(B4),Street1MP(R8),Street2MP(R8),MunicNum(B4),ZIP(B4),X(R8),Y(R8),MunicChanged(B2),ZIPChanged(B2),NumPartsRemoved(B2),AddressTAGPrefix(C8),AddressTAGUDI(C64),OnStreetNum(B4),IntID2(B4),X2(R8),Y2(R8),Street(C80),House(C12),City(C32),ZIPCODE(C12),Symbol(C64),FromDate(B4),ToDate(B4),Note(C100)";
	HANDLE	hStr=GSSiGlobAlloc (0,GMEM_MOVEABLE,4096);
	LPSTR	str=GlobalLock (hStr);

	sprintf (str,"%s%s,FromDateC(C32),ToDateC(C32),OriginalFileKey(C100)",KeyDef,DefStr);
	 _fstrcpy (FileIn1,File);
	 lpDot = _fstrrchr (FileIn1,'.');
	 if (lpDot)
	 	*lpDot = 0;
	 _fstrcpy (FileIn2,FileIn1);
	 _fstrcpy (BadNames,FileIn1);
	 _fstrcat (FileIn1,".in1");	
	 _fstrcat (FileIn2,".in2");	  
	 _fstrcat (BadNames,".bnf");	  
	 GSSiRemove (FileIn2);
	 lpGWDHead = &GWDHead; 
    _fmemset (lpGWDHead,0,sizeof(GWDHEADER16));

	 rtn = CreateGWDDatabase (File,1,TRUE,0,1,str);  
	 GSSiGlobUlFree (&hStr);
/*	 FidData = GSSiOpenFile (File,&OFStruct,OF_CREATE);
	 GWDHead.NumFields=0;
	 GWDHead.NumIndex=2;
	 GWDHead.Version=1;
	 GWDHead.Compressed = 1;
	 GWDHead.NumIndexFields[0]=1;
	 GWDHead.IndexFields[0][0]=0;
	 GWDHead.NumIndexFields[1]=-2; // was 2
	 GWDHead.IndexFields[1][0]=1;
	 GWDHead.IndexFields[1][1]=0;
	 GWDHead.lKeys[1]=6; 
//	 GWDHead.lKeys[1]=SHRT_MIN;  //creates as non-unique
//	 GWDHead.lKeys[1]=1; 
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
	ibeg = 0;
      
	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"RecordNumber");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"MatchCode");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"LocationCode");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"StreetNum");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"HouseNum");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"IntID");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"StreetNum1");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"StreetNum2");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_REAL;
	_fstrcpy (FldInfo.Name,"Street1MP");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_REAL;
	_fstrcpy (FldInfo.Name,"Street2MP");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"MunicNum");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"ZIP");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_REAL;
	_fstrcpy (FldInfo.Name,"X");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_REAL;
	_fstrcpy (FldInfo.Name,"Y");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"MunicChanged");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"ZIPChanged");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"NumPartsRemoved");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"AddressTAGPrefix");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 64;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"AddressTAGUDI");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 64;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"Street");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 12;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"House");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 32;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"City");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 12;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"ZIPCODE");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 40;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"Symbol");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 32;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"FromDate");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 32;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"ToDate");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 100;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"Note");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = OrigKeyLen;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"OriginalFileKey");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	 GWDHead.Reclen=ibeg; 
	 GWDHead.TimeStamp = time(0);
	 GSSillseek (FidData,0,0);
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
 	 GSSillseek (FidData,0,2);
			     
	 NumVars = 1;
	 NumIndex = 2;
			
	 hVars = LocalAlloc (LHND,NumVars * sizeof(BTVARDESC));
     pVars = (LPBTVARDESC)LocalLock(hVars);
			
	 pVars->BT_VARLEN=4;
	 pVars->BT_VARTYP=BT_INTEGER;
	 pVars->BT_VAROFF=0;
	 BT_CREATE (FileIn1, 4, FALSE, 1, 1,pVars,FALSE, 0, GWDHead.TimeStamp, FALSE);
     LocalUnlock(hVars);
     LocalFree(hVars);
 	 GSSiClose2 (&FidData);

     hDB = OpenGWDatabase (File,BT_WRITE);
     if (!hDB) return (FALSE);
     CloseGWDatabase (hDB); 
	 BTVar[0].BT_VARLEN=40;
	 BTVar[0].BT_VARTYP=BT_CHAR;
	 BTVar[0].BT_VAROFF=0;
	 BTVar[1].BT_VARLEN=4;
	 BTVar[1].BT_VARTYP=BT_INTEGER;
	 BTVar[1].BT_VAROFF=40;
//	 BT_CREATE (BadNames, 2, FALSE, 2, 1,BTVar,FALSE, 0, GWDHead.TimeStamp, FALSE);
     return TRUE;*/
	 return rtn;

} 

BOOL SetAddressDir (void)
{
    if (*TempAddressDir)
		strcpy (AddMatchDir,TempAddressDir);
	else if (*AltAddressDir == '|')
		strcpy (AddMatchDir,&AltAddressDir[1]);
	else
		strcpy (AddMatchDir,"[%DL]address");
    ExpandText (AddMatchDir);
    return TRUE;
}  

long GetStreetNumFromName (LPSTR Name,short Index, short FirstOrNext, LPSTR TrueName)
{
    LPGWFLDINFO lpGWFldInfo;
    LPGWDHEADER lpGWDHead;
    HANDLE      hBT;
    long        Offset,SNum=0; 
    short       st,  j,  len, ifield; 
    LPVOID      lpVal;
    
    if (!hDBStreetNames)
    {
	    if (!OpenStreetNameTable(FALSE))
	        return 0; 
	}
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetNames);  
    if (Index >= lpGWDHead->NumIndex) 
    	goto NotFound;
    hBT = lpGWDHead->BTHandle[Index];
    ifield = Index + 1;
    lpGWFldInfo=lpGWDHead->pFldInfo; 
	SetFieldValFromChar(lpGWDHead, lpGWFldInfo, "0", FALSE, FALSE, TRUE);
    lpGWFldInfo+=Index;   
    if (!_fstricmp (Name,"##COUNT##"))  
    {
    	SNum = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[Index]);
    	goto NotFound;
    } 
	SetFieldValFromChar(lpGWDHead, lpGWFldInfo, Name, FALSE, FALSE, TRUE);
    GWDFormKey(lpGWDHead,Index,TRUE,0,0);
    if (FirstOrNext == BT_FIRST)
    {
//      _fstrncpy (lpGWDHead->pKeys[Index],Name,32);
        st = BT_FIND (hBT,lpGWDHead->pKeys[Index],BT_FIRST,BT_GE, (LPSTR)&Offset);
    }
    else
        st = BT_FIND (hBT,lpGWDHead->pKeys[Index],BT_NEXT,BT_ANY, (LPSTR)&Offset);
    if (st) goto NotFound; 
    if (FirstOrNext)
    	if (_fstrncmp (lpGWDHead->pKeys[Index],Name,32)) goto NotFound;
    GSSillseek (lpGWDHead->Fid,Offset,0);
    BigRead (lpGWDHead->Fid,(HPSTR)&len,2);
    BigRead (lpGWDHead->Fid,(HPSTR)&lpGWDHead->GWDData,len);
    for (j=len;j<lpGWDHead->Reclen;j++) lpGWDHead->GWDData[j]='\0'; 
                    
    lpGWFldInfo=lpGWDHead->pFldInfo;      
    
    lpVal = &lpGWDHead->GWDData[lpGWFldInfo->Beg];  
    SNum = *(LPLONG)lpVal;
    if (TrueName)
    {    
        lpGWFldInfo++;
	    if (lpGWDHead->Version > 1)
		    lpGWFldInfo+=14;
        lpVal = &lpGWDHead->GWDData[lpGWFldInfo->Beg];  
        strncpy0 (TrueName,lpVal,lpGWFldInfo->Len);
    }
NotFound: 

    GlobalUnlock (hDBStreetNames);
    return (SNum);
}
BOOL GetTrueStreetName (long SNum, LPSTR TrueName, long State,int index)
{
    LPGWFLDINFO lpGWFldInfo;
    LPGWDHEADER lpGWDHead;
    HANDLE      hBT;
    long        Offset; 
    short       st, j,  len,  ifield; 
    LPVOID      lpVal;
    short       Index=0; 
    BOOL		rtn=FALSE;
    LPSTR		lpAND,pSpace;
	BOOL		FieldInc=0;
	
	if (!TrueName)
		return FALSE;
	*TrueName = 0;
	if (SNum >= 1000000000) 
	{
		SNum -= 1000000000;
		FieldInc = 9; //gets name only
	}
    if (ByState)
    {
        if (HaveState != State)
        {   
            char    str[8];
                            
            CloseGSStreetNames();
            sprintf (str,"%2.2i",State);
            SetGlobalValue ("%STATE",str);   
            if (!OpenGSStreetNames (BT_READ,3)) 
                return FALSE;
            HaveState = State;
            BT_FIND (hNames2,(LPSTR)TrueName,BT_FIRST,BT_ANY,(LPSTR)&BlankStreet); 
            if (*TrueName)
                BlankStreet = 0;
        }
        if (SNum < 0)
        	return FALSE;  
        if (!BT_FIND (hNames1,(LPSTR)&SNum,BT_FIRST,BT_EQ,TrueName))
        {   
        	if ((lpAND = _fstrchr(TrueName,'&')))
        		*lpAND = 0; //fixes Canada roads where mult names in single field    
        	if (FieldInc)
        	{
        		if ((pSpace = _fstrrchr (TrueName,' '))) 
        		{
        			*pSpace = 0;
        			return TRUE;
        		}
        		else
        			return FALSE;
        	}
        	else
	            return TRUE;
        }
        else
            return FALSE;
    }
    if (!OpenStreetNameTable(FALSE))
        return FALSE;
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetNames); 
    hBT = lpGWDHead->BTHandle[Index];
    ifield = 0;
    lpGWFldInfo=lpGWDHead->pFldInfo; 
	SetFieldValFromChar(lpGWDHead, lpGWFldInfo, "0", FALSE, FALSE, TRUE);
    lpGWFldInfo+=Index;     
    SetFieldValFromLong(lpGWDHead,lpGWFldInfo,SNum); 
    GWDFormKey(lpGWDHead,Index,TRUE,0,0);
    st = BT_FIND (hBT,lpGWDHead->pKeys[Index],BT_FIRST,BT_EQ, (LPSTR)&Offset);
    if (st) goto NotFound; 
    rtn = TRUE; 
    len = FillGWDData (lpGWDHead,Offset);
                    
    lpGWFldInfo=lpGWDHead->pFldInfo;      
    
    lpVal = &lpGWDHead->GWDData[lpGWFldInfo->Beg];  
    SNum = *(LPLONG)lpVal;
	if (index)
		lpGWFldInfo += index;
	else
	{
		lpGWFldInfo++;
		if (lpGWDHead->Version > 1 && State <= 0)
			lpGWFldInfo+=14; 
		lpGWFldInfo += FieldInc;
	}
    lpVal = &lpGWDHead->GWDData[lpGWFldInfo->Beg];  
    _fstrncpy (TrueName,lpVal,lpGWFldInfo->Len);
    TrueName+=lpGWFldInfo->Len;
    *TrueName=0;
NotFound: 
	if (!rtn)
//		_fstrcpy (TrueName,"???");
		_fstrcpy (TrueName,"");
    GlobalUnlock (hDBStreetNames);
    return rtn;
}

long AddStreetName (LPSTR InTrueName,long num,LPSTR FEDIRP,LPSTR FENAME,LPSTR FETYPE, LPSTR FEDIRS)
{        
    LPGWDHEADER lpGWDHead;
    long    SNum=0, Offset;   
    HANDLE  hBT;  
    short       st;   
    LPSTREETNAMETABLE   pSN;  
    char	TrueName[128];

	if (ByState)
		return (GetGSStreetNum (InTrueName));
    
    if (!OpenStreetNameTable(TRUE))
        return 0;   
    _fstrcpy (TrueName,InTrueName);
    OneSpace (TrueName);  
    if (!*TrueName)
    	return 0;
    _fstrupr (TrueName);
    if (num >= 0) 
    {
	    SNum=GetStreetNumFromName (TrueName,1,BT_FIRST, 0);
	    if (SNum)
	        return SNum;
	}
	else
		num = -num; // loads dup names with different numbers
STNDSN_INIT(FALSE);
    STNDST(TrueName, (short)_fstrlen(TrueName),STDNAMv,NRONAMv,NMONLYv,
                             SANSCHv,NANDCHv,NCMPNMv,ORIGNMv,SANSCPv,SANSCSv,0,0,0,0);   
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetNames); 
    if (num)
    	SNum = num;
    else
    {
	    hBT = lpGWDHead->BTHandle[0];
	    st = BT_FIND (hBT,(LPSTR)&SNum,BT_LAST,BT_ANY, (LPSTR)&Offset);
	    if (st)
	        SNum=1;
	    else
	        SNum++;  
	}     
        
    pSN = (STREETNAMETABLE *)&lpGWDHead->GWDData;
    pSN->StreetNum = SNum;
    _fstrncpy (pSN->TrueName,InTrueName,32);
    _fstrncpy (pSN->TrueNameUC,TrueName,32);  
    _fstrncpy (pSN->StdName,STDNAMv,32);
    _fstrncpy (pSN->NamePortionOfStdName,NMONLYv,32);
    _fstrncpy (pSN->StdNameWOHeading,SANSCHv,32);
    _fstrncpy (pSN->StdNameWOType,NANDCHv,32);
    _fstrncpy (pSN->NonReorderedNamePortion,NRONAMv,32);
    _fstrncpy (pSN->NonCompressedNamePortion,NCMPNMv,32);
    _fstrncpy (pSN->OriginalNamePortion,ORIGNMv,32);
    _fstrncpy (pSN->NonReorderedNameNoPrefixPortion,SANSCPv,32);
    _fstrncpy (pSN->NonReorderedNameNoSuffixPortion,SANSCSv,32);
    _fstrncpy (pSN->FEDIRP,FEDIRP,2);
    _fstrncpy (pSN->FENAME,FENAME,30);
    _fstrncpy (pSN->FETYPE,FETYPE,4);
    _fstrncpy (pSN->FEDIRS,FEDIRS,2);
    GWDAddRecord (lpGWDHead,0,0);
            
    GlobalUnlock (hDBStreetNames);
    return (SNum);   
    
}

BOOL DeleteStreetNetwork (void)
{
	NetworkID=1;
    CloseNetLinkAndRef(TRUE);                    
	CloseNetIntersect (TRUE);
    CloseNetMarkers(TRUE);                   
    CloseNetVideoIndex(TRUE);                    
    SetNetworkDir();   
	DeleteDirAndContents (NetworkDir);
    return TRUE;
}

BOOL OpenNetLinkAndRef  (short NetID,BOOL Update,LPBOOL Opened)
{   
    char    File[MAX_PATH];
    static  BOOL    OpenMode;
    short       Mode;   
    DWORD	Err;
    
	*Opened = 0;
	NetworkID=NetID;
	if (hBTNetLinks && (OpenMode==Update || OpenMode==BT_WRITE)) 
		return TRUE;
    SetNetworkDir();
    CloseNetLinkAndRef(TRUE);                    
    
    sprintf (File,"%s\\netlinks.btr",NetworkDir);
    if (!ExistFile(File))
    {   
    	if (!Update)
    		return FALSE;
        GSSiMakeDir (NetworkDir,&Err);
        if (!CreateNetLinkAndRef (NetworkDir,NetworkID))
        {
            return FALSE;
        } 
    }
    if (Update)
        Mode = BT_WRITE;
    else
        Mode = BT_READ;
    if (!(hBTNetLinks = BT_OPEN (File,0,Mode,0)))
    {
        return FALSE;
    }  
    sprintf (File,"%s\\net%i\\netrefs.btr",NetworkDir,(int)NetworkID);
    if (!(hBTNetRefs = BT_OPEN (File,0,Mode,0)))
    {
        return FALSE;
    }  
    OpenMode = Update;  
    *Opened = 1;
    
    return TRUE;
}
  
void CloseNetLinkAndRef (BOOL Opened)
{  
    if (!Opened) return;
    BT_CLOSE (hBTNetLinks);
    BT_CLOSE (hBTNetRefs);
    hBTNetLinks = 0;
    hBTNetRefs = 0;
    return;
}

BOOL SetNetworkDir (void)
{
    _fstrcpy (NetworkDir,"[%DL]networks");
    ExpandText (NetworkDir);
    return TRUE;
}

BOOL CreateNetLinkAndRef (LPSTR NetworkDir,short NetworkID)
{   char    File[MAX_PATH];
    BTVARDESC   BTVar[3]; 
    DWORD	Err;

    BTVar[0].BT_VARTYP=BT_INTEGER;
    BTVar[0].BT_VARLEN=4;
    BTVar[0].BT_VAROFF=0;
    BTVar[1].BT_VARTYP=BT_INTEGER;
    BTVar[1].BT_VARLEN=2;
    BTVar[1].BT_VAROFF=4;
    BTVar[2].BT_VARTYP=BT_INTEGER;
    BTVar[2].BT_VARLEN=4;
    BTVar[2].BT_VAROFF=6;
    sprintf (File,"%s\\netlinks.btr",NetworkDir);
    BT_CREATE (File, sizeof(NETLINKSDATA), FALSE, 3, 1,
        (LPBTVARDESC) BTVar,FALSE, 0, 0, FALSE);
  
    sprintf (File,"%s\\net%i",NetworkDir,(int)NetworkID);
    GSSiMakeDir (File,&Err);
    
    BTVar[0].BT_VARTYP=BT_INTEGER;
    BTVar[0].BT_VARLEN=4;
    BTVar[0].BT_VAROFF=0;
    BTVar[1].BT_VARTYP=BT_REAL;
    BTVar[1].BT_VARLEN=8;
    BTVar[1].BT_VAROFF=4;
    sprintf (File,"%s\\net%i\\netrefs.btr",NetworkDir,(int)NetworkID);
    BT_CREATE (File, sizeof(NETREFSDATA), FALSE, 2, 1,
        (LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
    return TRUE;
}

void PutNetRefAndLink (int NetID,long Path,long Ref,int RefDir,double MP,double Length,int NetDir)
{
    NETREFSKEY  NetRefsKey;
    NETREFSDATA NetRefsData;  
    NETLINKSKEY NetLinksKey; 
    NETLINKSDATA    NetLinksData;
    
    NetRefsKey.Path = Path;
    NetRefsKey.MP   = MP;
    NetRefsData.Ref = Ref;
    NetRefsData.Dir = RefDir;
    if (NetDir < 0)
    { 
    	if (NetRefsData.Dir == 1)
    		NetRefsData.Dir = 2;
    	else
    		NetRefsData.Dir = 1;
    } 
    BT_PUT (hBTNetRefs,(LPSTR)&NetRefsKey,(LPSTR)&NetRefsData);  
    NetLinksKey.Ref = Ref;
    NetLinksKey.NetID = NetID;
    NetLinksKey.Path = Path;  
    if (NetRefsData.Dir == 1)
   		NetLinksData.Length = Length;  
   	else
   		NetLinksData.Length = -Length; 
   	NetLinksData.MP = MP;
    BT_PUT (hBTNetLinks,(LPSTR)&NetLinksKey,(LPSTR)&NetLinksData);
    return;
} 

BOOL CreateNetMarkers (LPSTR NetworkDir,short NetworkID)
{   char    File[MAX_PATH];
    BTVARDESC   BTVar[5]; 
    DWORD	Err;

    BTVar[0].BT_VARTYP=BT_INTEGER;
    BTVar[0].BT_VARLEN=2;
    BTVar[0].BT_VAROFF=0;
    BTVar[1].BT_VARTYP=BT_INTEGER;
    BTVar[1].BT_VARLEN=4;
    BTVar[1].BT_VAROFF=2;
    BTVar[2].BT_VARTYP=BT_CHAR;
    BTVar[2].BT_VARLEN=2;
    BTVar[2].BT_VAROFF=6;
    BTVar[3].BT_VARTYP=BT_CHAR;
    BTVar[3].BT_VARLEN=2;
    BTVar[3].BT_VAROFF=8;
    BTVar[4].BT_VARTYP=BT_REAL;
    BTVar[4].BT_VARLEN=8;
    BTVar[4].BT_VAROFF=10;
    sprintf (File,"%s\\net%i",NetworkDir,(int)NetworkID);
    GSSiMakeDir (File,&Err);
    sprintf (File,"%s\\net%i\\markers1.btr",NetworkDir,(int)NetworkID);
    BT_CREATE (File, sizeof(NETMARKERSDATA1), FALSE, 5, 1,
        (LPBTVARDESC) BTVar,FALSE, 0, 0, FALSE);
  
    BTVar[0].BT_VARTYP=BT_INTEGER;
    BTVar[0].BT_VARLEN=2;
    BTVar[0].BT_VAROFF=0;
    BTVar[1].BT_VARTYP=BT_INTEGER;
    BTVar[1].BT_VARLEN=4;
    BTVar[1].BT_VAROFF=2;
    BTVar[2].BT_VARTYP=BT_REAL;
    BTVar[2].BT_VARLEN=8;
    BTVar[2].BT_VAROFF=6;
    sprintf (File,"%s\\net%i\\markers2.btr",NetworkDir,(int)NetworkID);
    BT_CREATE (File, sizeof(NETMARKERSDATA2), FALSE, 3, 1,
            (LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
    return TRUE;
}


BOOL OpenNetMarkers  (short NetworkID,BOOL Update,LPBOOL Opened)
{   
    char    File[MAX_PATH];
    static  BOOL    OpenMode;
    short       Mode;
    DWORD	Err;
    
	*Opened = 0;
    SetNetworkDir();
    if (hBTNetMarkers1 && (OpenMode==Update || OpenMode==BT_WRITE))
        return TRUE; 
    CloseNetMarkers(TRUE);                   
    
    sprintf (File,"%s\\net%i\\markers1.btr",NetworkDir,(int)NetworkID);
    if (!ExistFile(File) || Update == 2)
    {   
        GSSiMakeDir (NetworkDir,&Err);
        if (!CreateNetMarkers (NetworkDir,NetworkID))
        {
            return FALSE;
        } 
    }
    if (Update)
        Mode = BT_WRITE;
    else
        Mode = BT_READ;
    if (!(hBTNetMarkers1 = BT_OPEN (File,0,Mode,0)))
    {
        return FALSE;
    }  
    sprintf (File,"%s\\net%i\\markers2.btr",NetworkDir,(int)NetworkID);
    if (!(hBTNetMarkers2 = BT_OPEN (File,0,Mode,0)))
    {
        return FALSE;
    }  
    OpenMode = Update; 
    *Opened = 1;
    
    return TRUE;
}
  
void CloseNetMarkers (BOOL Opened)
{  
    if (!Opened) return;
    BT_CLOSE (hBTNetMarkers1);
    BT_CLOSE (hBTNetMarkers2);
    hBTNetMarkers1 = 0;
    hBTNetMarkers2 = 0;
    return;
}

BOOL CreateNetRefIntersect (LPSTR NetworkDir,short NetworkID)
{   char    File[MAX_PATH];
    BTVARDESC   BTVar[5];  
    short	pos=BT_FIRST;
	NETINTREFKEY	NetIntRefKey;
	NETINTREFDATA	NetIntRefData; 
	NETREFINTKEY	NetRefIntKey;
	NETREFINTDATA	NetRefIntData;  
	long	nRecs, nLoaded=0;    
	DWORD	Err;

    BTVar[0].BT_VARTYP=BT_INTEGER;
    BTVar[0].BT_VARLEN=4;
    BTVar[0].BT_VAROFF=0;
    BTVar[1].BT_VARTYP=BT_INTEGER;
    BTVar[1].BT_VARLEN=4;
    BTVar[1].BT_VAROFF=4;
    sprintf (File,"%s\\net%i",NetworkDir,(int)NetworkID);
    GSSiMakeDir (File,&Err);
    sprintf (File,"%s\\net%i\\refint.btr",NetworkDir,(int)NetworkID);
    BT_CREATE (File, sizeof(NETREFINTDATA), FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE); 
	nRecs = BT_NUM_IN_INDEX (hBTNetIntRef);
	hBTNetRefInt = BT_OPEN (File, 0, BT_WRITE, 0);
	CreateStatusWind (hWndMain,1,"Create Intersection Ref Table");
	while (!BT_FIND (hBTNetIntRef,(LPSTR)&NetIntRefKey,pos,BT_ANY,(LPSTR)&NetIntRefData))
	{
		pos = BT_NEXT;
		NetRefIntKey.IntID = NetIntRefKey.IntID;
		NetRefIntKey.Refno = NetIntRefKey.Refno;  
		NetRefIntData.WhichEnd = NetIntRefData.WhichEnd;
		NetRefIntData.Coord = NetIntRefData.Coord;
		BT_PUT (hBTNetRefInt,(LPSTR)&NetRefIntKey,(LPSTR)&NetRefIntData);
		StatusWindowUpdate (0,0, nRecs, ++nLoaded);
    }
    BT_CLOSE (hBTNetRefInt);
	DestroyStatusWindow(0); 
	return TRUE;
}
  
BOOL CreateNetIntersect (LPSTR NetworkDir,short NetworkID)
{   char    File[MAX_PATH];
    BTVARDESC   BTVar[5]; 
    DWORD	Err;

    BTVar[0].BT_VARTYP=BT_INTEGER;
    BTVar[0].BT_VARLEN=4;
    BTVar[0].BT_VAROFF=0;
    BTVar[1].BT_VARTYP=BT_INTEGER;
    BTVar[1].BT_VARLEN=4;
    BTVar[1].BT_VAROFF=4;
    sprintf (File,"%s\\net%i",NetworkDir,(int)NetworkID);
    GSSiMakeDir (File,&Err);
    sprintf (File,"%s\\net%i\\intref.btr",NetworkDir,(int)NetworkID);
    BT_CREATE (File, sizeof(NETINTREFDATA), FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
  
    BTVar[0].BT_VARTYP=BT_INTEGER;
    BTVar[0].BT_VARLEN=4;
    BTVar[0].BT_VAROFF=0;
    BTVar[1].BT_VARTYP=BT_REAL;
    BTVar[1].BT_VARLEN=8;
    BTVar[1].BT_VAROFF=4;
    sprintf (File,"%s\\net%i\\intmp.btr",NetworkDir,(int)NetworkID);
    BT_CREATE (File, 4, FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);    
    
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=4;
	BTVar[2].BT_VARTYP=BT_INTEGER;
	BTVar[2].BT_VARLEN=4;
	BTVar[2].BT_VAROFF=8;
    sprintf (File,"%s\\net%i\\intpaths.btr",NetworkDir,(int)NetworkID);
	BT_CREATE (File, sizeof(DPOINT), FALSE, 3, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
    return TRUE;
}

BOOL OpenNetIntersect (short NetworkID,BOOL Update,LPBOOL Opened)
{   
    char    File[MAX_PATH];
    static  BOOL    OpenMode;
    short       Mode; 
    DWORD	Err;
    
	*Opened = 0;
    SetNetworkDir();
    if (hBTNetIntRef && (OpenMode==Update || OpenMode==BT_WRITE))
        return TRUE; 
    CloseNetIntersect(TRUE);                   
    
    sprintf (File,"%s\\net%i\\intref.btr",NetworkDir,(int)NetworkID);
    if (!ExistFile(File))
    {   
        GSSiMakeDir (NetworkDir,&Err);
        if (!CreateNetIntersect (NetworkDir,NetworkID))
        {
            return FALSE;
        } 
    }
    if (Update)
        Mode = BT_WRITE;
    else
        Mode = BT_READ;
    if (!(hBTNetIntRef = BT_OPEN (File,0,Mode,0)))
    {
        return FALSE;
    }  
    sprintf (File,"%s\\net%i\\intmp.btr",NetworkDir,(int)NetworkID);
    if (!(hBTNetIntMP = BT_OPEN (File,0,Mode,0)))
    {
        return FALSE;
    }  
    sprintf (File,"%s\\net%i\\intpaths.btr",NetworkDir,(int)NetworkID);
    if (!(hBTNetIntPaths = BT_OPEN (File,0,Mode,0)))
    {
        return FALSE;
    }
    if (!Update)
    	hBTNetIntPaths = AddMunicsToInts (hBTNetIntPaths,File);  
    sprintf (File,"%s\\net%i\\refint.btr",NetworkDir,(int)NetworkID);
    if (!ExistFile(File))
    {   
        GSSiMakeDir (NetworkDir,&Err);
        if (!CreateNetRefIntersect (NetworkDir,NetworkID))
        {
            return FALSE;
        } 
    }
    if (!(hBTNetRefInt = BT_OPEN (File,0,Mode,0)))
    {
        return FALSE;
    } 
    OpenMode = Update; 
    *Opened = 1;
    
    return TRUE;
}
  
void CloseNetIntersect (BOOL Opened)
{  
    if (!Opened) return;
    BT_CLOSE (hBTNetIntRef);
    BT_CLOSE (hBTNetIntMP);
	BT_CLOSE (hBTNetRefInt);
	BT_CLOSE (hBTNetIntPaths);
	hBTNetRefInt = 0;
	hBTNetIntPaths = 0;
    hBTNetIntRef = 0;
    hBTNetIntMP = 0;
    return;
}
 
BOOL CreateTurnTable (LPSTR NetworkDir)
{   char    File[MAX_PATH];
    BTVARDESC   BTVar[5];

    BTVar[0].BT_VARTYP=BT_INTEGER;
    BTVar[0].BT_VARLEN=4;
    BTVar[0].BT_VAROFF=0;
    BTVar[1].BT_VARTYP=BT_INTEGER;
    BTVar[1].BT_VARLEN=2;
    BTVar[1].BT_VAROFF=4;
    BTVar[2].BT_VARTYP=BT_INTEGER;
    BTVar[2].BT_VARLEN=4;
    BTVar[2].BT_VAROFF=6;
    BTVar[3].BT_VARTYP=BT_INTEGER;
    BTVar[3].BT_VARLEN=2;
    BTVar[3].BT_VAROFF=10;
    BTVar[4].BT_VARTYP=BT_INTEGER;
    BTVar[4].BT_VARLEN=4;
    BTVar[4].BT_VAROFF=12;
    sprintf (File,"%s\\turndata.btr",NetworkDir);
    BT_CREATE (File, sizeof(double), FALSE, 5, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
    return TRUE;
}

BOOL OpenTurnTable (BOOL Update,LPBOOL Opened)
{   
    char    File[MAX_PATH];
    static  BOOL    OpenMode;
    short       Mode;  
    DWORD	Err;
    
	*Opened = 0;
    SetNetworkDir();
    if (hBTTurnTable && (OpenMode==Update || OpenMode==BT_WRITE))
        return TRUE; 
    CloseTurnTable (TRUE);                   
    
    sprintf (File,"%s\\turndata.btr",NetworkDir);
    if (!ExistFile(File))
    {   
        GSSiMakeDir (NetworkDir,&Err);
        if (!CreateTurnTable (NetworkDir))
        {
            return FALSE;
        } 
    }
    if (Update)
        Mode = BT_WRITE;
    else
        Mode = BT_READ;
    if (!(hBTTurnTable = BT_OPEN (File,0,Mode,0)))
    {
        return FALSE;
    }  
    OpenMode = Update; 
    *Opened = 1;
    
    return TRUE;
}
  
void CloseTurnTable (BOOL Opened)
{  
    if (!Opened) return;
    BT_CLOSE (hBTTurnTable);
    hBTTurnTable = 0;
    return;
}
 
BOOL CreateNetVideoIndex (LPSTR NetworkDir,short NetworkID)
{   char    File[MAX_PATH];
    BTVARDESC   BTVar[8];
    DWORD	Err;

    BTVar[0].BT_VARTYP=BT_INTEGER;
    BTVar[0].BT_VARLEN=4;
    BTVar[0].BT_VAROFF=0;
    BTVar[1].BT_VARTYP=BT_INTEGER;
    BTVar[1].BT_VARLEN=2;
    BTVar[1].BT_VAROFF=4;
    BTVar[2].BT_VARTYP=BT_INTEGER;
    BTVar[2].BT_VARLEN=2;
    BTVar[2].BT_VAROFF=6;
    BTVar[3].BT_VARTYP=BT_INTEGER;
    BTVar[3].BT_VARLEN=2;
    BTVar[3].BT_VAROFF=8;
    BTVar[4].BT_VARTYP=BT_CHAR;
    BTVar[4].BT_VARLEN=2;
    BTVar[4].BT_VAROFF=10;
    BTVar[5].BT_VARTYP=BT_CHAR;
    BTVar[5].BT_VARLEN=2;
    BTVar[5].BT_VAROFF=12;
    BTVar[6].BT_VARTYP=BT_REAL;
    BTVar[6].BT_VARLEN=8;
    BTVar[6].BT_VAROFF=14;
    sprintf (File,"%s\\net%i",NetworkDir,(int)NetworkID);
    GSSiMakeDir (File,&Err);
    sprintf (File,"%s\\net%i\\netvid1.btr",NetworkDir,(int)NetworkID);
    BT_CREATE (File, sizeof(NETVIDEODATA1), FALSE, 7, 1,
        (LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
  
    BTVar[0].BT_VARTYP=BT_CHAR;
    BTVar[0].BT_VARLEN=16;
    BTVar[0].BT_VAROFF=0;
    BTVar[1].BT_VARTYP=BT_INTEGER;
    BTVar[1].BT_VARLEN=4;
    BTVar[1].BT_VAROFF=8;
    sprintf (File,"%s\\net%i\\netvid2.btr",NetworkDir,(int)NetworkID);
    BT_CREATE (File, sizeof(NETVIDEODATA2), FALSE, 2, 1,
        (LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
    return TRUE;
}


BOOL OpenNetVideoIndex  (short NetworkID,BOOL Update,LPBOOL Opened)
{   
    char    File[MAX_PATH];
    static  BOOL    OpenMode;
    short       Mode;
    DWORD	Err;
    
	*Opened = 0;
    SetNetworkDir();
    if (hBTNetVideo1 && (OpenMode==Update || OpenMode==BT_WRITE))
        return TRUE; 
    CloseNetVideoIndex(TRUE);                    
    
    sprintf (File,"%s\\net%i\\netvid1.btr",NetworkDir,(int)NetworkID);
    if (!ExistFile(File) || Update)
    {   
        GSSiMakeDir (NetworkDir,&Err);
        if (!CreateNetVideoIndex (NetworkDir,NetworkID))
        {
            return FALSE;
        } 
    }
    if (Update)
        Mode = BT_WRITE;
    else
        Mode = BT_READ;
    if (!(hBTNetVideo1 = BT_OPEN (File,0,Mode,0)))
    {
        return FALSE;
    }  
    sprintf (File,"%s\\net%i\\netvid2.btr",NetworkDir,(int)NetworkID);
    if (!(hBTNetVideo2 = BT_OPEN (File,0,Mode,0)))
    {
        return FALSE;
    }  
    OpenMode = Update;
    *Opened = 1;
    
    return TRUE;
}
  
void CloseNetVideoIndex (BOOL Opened)
{  
    if (!Opened) return;
    BT_CLOSE (hBTNetVideo1);
    BT_CLOSE (hBTNetVideo2);
    hBTNetVideo1 = 0;
    hBTNetVideo2 = 0;
    return;
}
long ChangeSimilarStreets (LPSTR DestName,LPSTR OrigName,long RecNum,LPHANDLE hChangeRecs)
{
	char	BadNameFile[MAX_PATH];   
	LPSTR	lpDot; 
	HANDLE	hBTBadNames;
    BADNAMEKEY	BadNameKey;
    short	pos=BT_FIRST, cond=BT_GE, sn, Choice;
    long	nFixed=0;  
    LPLONG	pRecs;
	
    *hChangeRecs = GSSiGlobAlloc ( 122,GHND,USHRT_MAX);
    pRecs = (LPLONG)GlobalLock (*hChangeRecs); 
    *pRecs++ = RecNum;
    nFixed++;
    _fstrcpy (BadNameFile,DestName);
    lpDot = _fstrrchr (BadNameFile,'.'); 
    if (!lpDot)
    	return 0;
    *lpDot = 0;
    _fstrcat (BadNameFile,".bnf");
    hBTBadNames = BT_OPEN (BadNameFile,0,BT_READ,0);          
	_fstrncpy (BadNameKey.Name,OrigName,40);
	BadNameKey.RecNum = 0;  
	while (!BT_FIND (hBTBadNames,(LPSTR)&BadNameKey,pos,cond,(LPSTR)&sn))
	{
		pos = BT_NEXT;
		cond = BT_ANY; 
		if (_fstrcmp (BadNameKey.Name,OrigName) != 0 || nFixed > (USHRT_MAX/4-2))
			break; 
		if (BadNameKey.RecNum != RecNum)
		{
			nFixed++; 
			*pRecs++ = BadNameKey.RecNum;
		}
	}
	GlobalUnlock (*hChangeRecs);
	if (!nFixed)
		GSSiGlobFree (hChangeRecs);	
    BT_CLOSE (hBTBadNames);
    return nFixed;
}



void ShowStreetMatches (LPSTR StreetBuf,HWND hWndDlg,short idc_list,LPMNMXCORD pBounds)
{   
	char	TrueName[40], str[128];
	LPLONG	pList;
	HANDLE	hList;
	short	pos, nAdded, nList=0;
	
	STNDSN_INIT(FALSE);
	STNDST(StreetBuf, (short)_fstrlen(StreetBuf),STDNAMv,NRONAMv,NMONLYv,
	                        SANSCHv,NANDCHv,NCMPNMv,ORIGNMv,SANSCPv,SANSCSv,0,0,0,0);
	pos = BT_FIRST;  
	SendDlgItemMessage (hWndDlg,idc_list,LB_RESETCONTENT,0,0);
	hList = GSSiGlobAlloc ( 123,GMEM_MOVEABLE,USHRT_MAX);
    nAdded = GetNameTypeList (1,&nList,hList,STDNAMv,NRONAMv,NMONLYv,
	                        	SANSCHv,NANDCHv,NCMPNMv,ORIGNMv,SANSCPv,SANSCSv); 
    nAdded += GetNameTypeList (2,&nList,hList,STDNAMv,NRONAMv,NMONLYv,
	                        	SANSCHv,NANDCHv,NCMPNMv,ORIGNMv,SANSCPv,SANSCSv);
	pList = (LPLONG)GlobalLock (hList); 
	while (nList--)
	{   
		GetTrueStreetName (*pList, TrueName, 0,0);
		if (GetNumStreetSegs (*pList,0,0,0,pBounds,0,0,0))
		{
			sprintf (str,"%s\t%ld",TrueName,*pList);
			pList++;
    		SendDlgItemMessage (hWndDlg,idc_list,LB_ADDSTRING,0,(LPARAM)str);
		}
	} 
	GSSiGlobUlFree (&hList);
	return;
}

short OFT_MATCH (LPSTR OnStreet,LPSTR FromStreet, LPSTR ToStreet, LPSTR City, long ZIP, short MOPT, LPHANDLE pHandle,
				 LPLONG pOnStreetNum, LPLONG pFromStreetNum, LPLONG pToStreetNum,LPLONG pMunicNum,LPINT pNumMatch1,LPINT pNumMatch2,LPHANDLE phMatch1,LPHANDLE phMatch2)
{
	int	OnStreetNum2;
	HANDLE	hMatch1=0, hMatch2=0;
	int match1 = INT_MATCH (OnStreet,FromStreet,City,0,MOPT,&hMatch1,pOnStreetNum,pFromStreetNum,pMunicNum); 
	int	match2 = INT_MATCH (OnStreet,ToStreet,City,0,MOPT,&hMatch2,&OnStreetNum2,pToStreetNum,pMunicNum);
	
	if (match1 == 1 && match2 == 1)
	{
		LPADDMATCH	pMatch1=GlobalLock (hMatch1);
		LPADDMATCH	pMatch2=GlobalLock (hMatch2);

		if (pMatch1->StreetNum1 == pMatch2->StreetNum1)
		{
			pMatch1->MatchCode = max (pMatch1->MatchCode,pMatch2->MatchCode);
			pMatch1->LocationCode = 11;
			pMatch1->IntIDTo = pMatch2->IntID;
			pMatch1->OnStreetNum = pMatch1->StreetNum1;
			pMatch1->StreetNum1 = pMatch1->StreetNum2;
			pMatch1->StreetNum2 = pMatch2->StreetNum2;
			pMatch1->Point2 = pMatch2->Point;
			GlobalUnlock (hMatch1);
			*pHandle = hMatch1;
			GSSiGlobUlFree (&hMatch2);

			return 1;
		}
		else
		{
			GlobalUnlock (hMatch1);
			GlobalUnlock (hMatch2);
		}
	}
	if (pNumMatch1)
	{
		*pNumMatch1 = match1;
		*pNumMatch2 = match2;
		*phMatch1 = hMatch1;
		*phMatch2 = hMatch2;
	}
	else
	{
		GSSiGlobFree (&hMatch1);
		GSSiGlobFree (&hMatch2);
	}
	return 0;
}

short INT_MATCH (LPSTR Street1, LPSTR Street2, LPSTR City, long ZIP, short MOPT,LPHANDLE hMatch,
				 LPLONG	pStreetNum1, LPLONG pStreetNum2, LPLONG pMunic)
{   
	HANDLE	hList1=0, hList2=0; 
	LPLONG	pList1, pList2; 
	long	WantMunic;
	BOOL	Opened; 
	char	Street[256];
	short	nList1=0, nList2=0, rtn=0, nAdded1, nAdded2;
	char	STDNAM1[42], NRONAM1[42], NMONLY1[42], SANSCH1[42], NANDCH1[42], NCMPNM1[42], ORIGNM1[42], SANSCP1[42], SANSCS1[42] ;	
	char	STDNAM2[42], NRONAM2[42], NMONLY2[42], SANSCH2[42], NANDCH2[42], NCMPNM2[42], ORIGNM2[42], SANSCP2[42], SANSCS2[42] ;	
	
STNDSN_INIT(FALSE);
    if (!OpenNetIntersect (NetworkID,FALSE,&Opened))
      	return 0;
	*hMatch = 0;
	*pStreetNum1 = 0;
	*pStreetNum2 = 0;  
	_fstrcpy (Street,Street1);
	Truncate (Street);
	_fstrcat (Street,"/");
	_fstrcat (Street,Street2);
	Truncate (Street);
	if (FoundUserAssignedAddress (0,Street,*pMunic,FALSE,hMatch))
	{
		rtn = 1;
		goto Exit;
	}
 	WantMunic = GetMunicFromName (City);
	hList1 = GSSiGlobAlloc ( 124,GMEM_MOVEABLE,USHRT_MAX);
	hList2 = GSSiGlobAlloc ( 125,GMEM_MOVEABLE,USHRT_MAX);
    STNDST(Street1, (short)_fstrlen(Street1),STDNAM1,NRONAM1,NMONLY1,
                             SANSCH1,NANDCH1,NCMPNM1,ORIGNM1,SANSCP1,SANSCS1,0,0,0,0); 
    nAdded1 = GetNameTypeList (1,&nList1,hList1,STDNAM1,NRONAM1,NMONLY1,
                             					 SANSCH1,NANDCH1,NCMPNM1,ORIGNM1,SANSCP1,SANSCS1); 
	STNDST(Street2, (short)_fstrlen(Street2),STDNAM2,NRONAM2,NMONLY2,
								 SANSCH2,NANDCH2,NCMPNM2,ORIGNM2,SANSCP2,SANSCS2,0,0,0,0);   
	//	Find all type 1 matches (full standardized name), if any found return.
	nAdded2 = GetNameTypeList (1,&nList2,hList2,STDNAM2,NRONAM2,NMONLY2,
	                         						 SANSCH2,NANDCH2,NCMPNM2,ORIGNM2,SANSCP2,SANSCS2); 
	if (nList1 == 1)
	{
		pList1 = (LPLONG)GlobalLock (hList1);
		*pStreetNum1 = *pList1;
		GlobalUnlock (hList1);
	}
	if (nList2 == 1)
	{
		pList2 = (LPLONG)GlobalLock (hList2);
		*pStreetNum2 = *pList2;
		GlobalUnlock (hList2);
	}
    if (nAdded1 || nAdded2)
    	rtn = MatchIntLists (1,nList1,nList2,hList1,hList2,WantMunic,hMatch,0,0);
    if (rtn || MOPT <2)
    	goto Exit;

//	Next try all type 2 changes (Add direction if none present, remove if it is;
//	Add type if none present, remove if it is
    nAdded1 = GetNameTypeList (2,&nList1,hList1,STDNAM1,NRONAM1,NMONLY1,
                             					 SANSCH1,NANDCH1,NCMPNM1,ORIGNM1,SANSCP1,SANSCS1); 
    nAdded2 = GetNameTypeList (2,&nList2,hList2,STDNAM2,NRONAM2,NMONLY2,
                             					 SANSCH2,NANDCH2,NCMPNM2,ORIGNM2,SANSCP2,SANSCS2); 
    if (nAdded1 || nAdded2)
	    rtn = MatchIntLists (2,nList1,nList2,hList1,hList2,WantMunic,hMatch,0,0);
    if (rtn || MOPT <3)
    	goto Exit;
//	Next try all type 3 changes (Change direction if present, change type if none present
    nAdded1 = GetNameTypeList (3,&nList1,hList1,STDNAM1,NRONAM1,NMONLY1,
                             					 SANSCH1,NANDCH1,NCMPNM1,ORIGNM1,SANSCP1,SANSCS1); 
    nAdded2 = GetNameTypeList (3,&nList2,hList2,STDNAM2,NRONAM2,NMONLY2,
                             					 SANSCH2,NANDCH2,NCMPNM2,ORIGNM2,SANSCP2,SANSCS2); 
    if (nAdded1 || nAdded2 || WantMunic)
	    rtn = MatchIntLists (3,nList1,nList2,hList1,hList2,WantMunic,hMatch,0,0);
    if (rtn)
    	goto Exit;
Exit:                        
	GSSiGlobFree (&hList1);
	GSSiGlobFree (&hList2);
	if (rtn > 1) // check for multiple ints at same location (i.e STATE ST and 3RD AVE matching
				 // int of STATE ST and 3RD AVE N and STATE ST and 3RD AVE S. Return as 1 match.
	{   
		LPADDMATCH	pMatch, pFirstMatch;
		short	n=rtn,i; 
		long	IntID;  
		DPOINT	AvePoint;
		         
		pMatch = pFirstMatch = (LPADDMATCH)GlobalLock (*hMatch);  
		AvePoint = pMatch->Point;
		for (i=1;i<n;i++)
		{
			AvePoint.x += pMatch[i].Point.x;
			AvePoint.y += pMatch[i].Point.y;
		}
		AvePoint.x /= n;
		AvePoint.y /= n;
		IntID = pMatch->IntID; 
		while (n--)
		{   
			if (ldistp (AvePoint,pMatch++->Point) > GetGlobalDVal2 ("[%AddMatchAverageDist]",25))
				goto NotSame;
		} 
		rtn = 1;
NotSame:
		GlobalUnlock (*hMatch);		
	}
	if (rtn == 1)
	{
		LPADDMATCH	pMatch = (LPADDMATCH)GlobalLock (*hMatch);
		
		*pStreetNum1 = pMatch->StreetNum1;
		*pStreetNum2 = pMatch->StreetNum2;
		GlobalUnlock (*hMatch);		
	}
	CloseNetIntersect (Opened);				 
	return rtn;
}   

short INT_MATCH_DLG (HWND hWndDlg,UINT List1, UINT List2, long Munic, long ZIP, short MOPT,LPHANDLE phMatch,
					 LPLONG	pStreetNum1, LPLONG pStreetNum2,HWND hwnddlg1,HWND hwnddlg2)
{   
	HANDLE	hList1, hList2; 
	LPLONG	pList1, pList2; 
	char	str[256];
	LPSTR	lpTAB;
	short	nList1=0, nList2=0, rtn=0, item;
	
	*phMatch = 0;
	*pStreetNum1 = 0;
	*pStreetNum2 = 0;
    item = 0;
	hList1 = GSSiGlobAlloc ( 126,GMEM_MOVEABLE,USHRT_MAX);
	hList2 = GSSiGlobAlloc ( 127,GMEM_MOVEABLE,USHRT_MAX); 
	pList1 = (LPLONG)GlobalLock (hList1);
    while (SendDlgItemMessage(hWndDlg,List1,LB_GETTEXT,item++,(DWORD)&str) != LB_ERR)
	{
		lpTAB = _fstrchr (str,'\t')+1;
        *pList1++ = atol (lpTAB);
        nList1++;
    }  
    GlobalUnlock (hList1); 
	if (!List2)
	{
		rtn = GetAllInts (1, 1,hList1,Munic,phMatch,hwnddlg1,hwnddlg2);
		GSSiGlobFree (&hList1);
		goto Exit;
	}
	pList2 = (LPLONG)GlobalLock (hList2);
	item = 0;
    while (SendDlgItemMessage(hWndDlg,List2,LB_GETTEXT,item++,(DWORD)&str) != LB_ERR)
	{
		lpTAB = _fstrchr (str,'\t')+1;
        *pList2++ = atol (lpTAB);
        nList2++;
    }  
    GlobalUnlock (hList2); 
	if (nList1 == 1 && nList2 == 0)
	{
		rtn = GetAllInts (1, 1,hList1,Munic,phMatch,hwnddlg1,hwnddlg2);
		GSSiGlobFree (&hList1);
		GSSiGlobFree (&hList2);
		goto Exit;
	}

	if (nList1 == 1)
	{
		pList1 = (LPLONG)GlobalLock (hList1);
		*pStreetNum1 = *pList1;
		GlobalUnlock (hList1);
	}
	if (nList2 == 1)
	{
		pList2 = (LPLONG)GlobalLock (hList2);
		*pStreetNum2 = *pList2;
		GlobalUnlock (hList2);
	}
   	rtn = MatchIntLists_new (MOPT,nList1,nList2,hList1,hList2,Munic,phMatch,hwnddlg1,hwnddlg2);
	GSSiGlobFree (&hList1);
	GSSiGlobFree (&hList2);
	if (rtn > 1) // check for multiple ints at same location (i.e STATE ST and 3RD AVE matching
				 // int of STATE ST and 3RD AVE N and STATE ST and 3RD AVE S. Return as 1 match.
	{   
		LPADDMATCH	pMatch;
		short	n=rtn; 
		long	IntID;
		
		pMatch = (LPADDMATCH)GlobalLock (*phMatch);
		IntID = pMatch++->IntID; 
		n--;
		while (n--)
		{
			if (pMatch++->IntID != IntID)
				goto NotSame;
		} 
		rtn = 1;
NotSame:
		GlobalUnlock (*phMatch);		
	}
Exit:		 
	return rtn;
}   

short GetNameTypeList (short Type,LPSHORT nList,HANDLE hList,
								  LPSTR STDNAM,LPSTR NRONAM,LPSTR NMONLY,
                            	  LPSTR SANSCH,LPSTR NANDCH,LPSTR NCMPNM,LPSTR ORIGNM, LPSTR SANSCP, LPSTR SANSCS)
{                 
	short	nAdded=0, n;  
	LPLONG	pList, pListBeg, pList2;
	
	switch (Type)
	{
		case 1:
			nAdded = GetMatchingNames (STDNAM_INDEX, STDNAM,nList, hList);
		break;
		
		case 2: 
			if (!_fstrcmp (STDNAM,SANSCH))// if no compass heading match against all names minus ch
				nAdded += GetMatchingNames (SANSCH_INDEX, STDNAM, nList,hList);
			else //match name wo ch against all full standard names
				nAdded += GetMatchingNames (STDNAM_INDEX, SANSCH, nList,hList);
			if (!_fstrcmp (STDNAM,NANDCH))// if no type match against all names minus type
				nAdded += GetMatchingNames (NANDCH_INDEX, STDNAM,nList,hList);
			else//match name wo type against all full standard names
				nAdded += GetMatchingNames (STDNAM_INDEX, NANDCH, nList,hList);
			if (_fstrcmp (STDNAM,SANSCP))
				nAdded += GetMatchingNames (SANSCS_INDEX, STDNAM,nList,hList);
			if (_fstrcmp (STDNAM,SANSCS))
				nAdded += GetMatchingNames (SANSCP_INDEX, STDNAM,nList,hList);  
			if (!_fstrcmp (STDNAM,NMONLY))
				nAdded += GetMatchingNames (NMONLY_INDEX, NMONLY, nList,hList);

		break;
		
		case 3:
				nAdded += GetMatchingNames (NMONLY_INDEX, NMONLY, nList,hList);
		break;
	}

	return nAdded;
}

int GetMapQuestLocation(LPSTR FullAddressIN, LPSTR Quality, LPDPOINT pPoint, int MaxAcceptableQuality)
{
	int		rtn = 0;
	char	CMD[512] = "http://www.mapquestapi.com/geocoding/v1/address?key=Fmjtd%7Cluu72q01nq%2Crl%3Do5-5yb20&callback=renderOptions&inFormat=kvp&outFormat=json&location=";
	char	TempFile[MAX_PATH], FullAddress[256];

	strcpy(FullAddress, FullAddressIN);
	REPLAC(FullAddress, "&", "AND", 256);
	REPLAC(FullAddress, "/", " AND ", 256);
	strcat(CMD, FullAddress);
	GSSiGetTempFileName(0, "gmt", 0, TempFile);
	if (URLToFile(CMD, TempFile))
	{
		int	lFile = GSSiLength(TempFile);

		if (lFile > 0)
		{
			HANDLE	hMem = GSSiGlobAlloc(0, GMEM_MOVEABLE, lFile);
			LPSTR	pFile = GlobalLock(hMem);
			HFILE	Fid = GSSiOpenFile(TempFile, 0, OF_READ);
			DPOINT	LatLng = { 0, 0 };

			if (Fid != HFILE_ERROR)
			{
				LPSTR	ql, pEnd, plat, plng;

				BigRead(Fid, pFile, lFile);
				GSSiClose2 (&Fid);
				ql = strstr(pFile, "\"geocodeQuality\":\"");
				if (ql)
				{
					ql += 18;
					pEnd = strchr(ql, '"');
					if (pEnd)
					{
						*pEnd = 0;
						strcpy(Quality, ql);
						if (!stricmp(Quality, "ADDRESS"))
							rtn = 1;
						else if (!stricmp(Quality, "INTERSECTION"))
							rtn = 2;
						else if (!stricmp(Quality, "POINT"))
							rtn = 3;
						else if (!stricmp(Quality, "STREET"))
							rtn = 4;
						if (rtn && rtn <= MaxAcceptableQuality)
						{
							plat = strstr(pFile, "\"lat\":");
							plng = strstr(pFile, "\"lng\":");
							if (plat)
							{
								if ((pEnd = strchr(plat, ',')))
								{
									*pEnd++ = 0;
									plat += 6;
									LatLng.y = atof(plat);
									if (plng)
									{
										plng += 6;
										if ((pEnd = strchr(plng, '}')))
										{
											*pEnd = 0;
											LatLng.x = atof(plng);
										}
									}
								}
							}
							if (LatLng.x != 0 && LatLng.y != 0)
							{
								if (!ConvertCoord(&LatLng, 2, 1))
									*pPoint = LatLng;
								else
									rtn = 0;
							}
							else
								rtn = 0;
						}
						else
							rtn = 0;
					}
				}
			}
			GSSiGlobUlFree(&hMem);
		}

	}
	GSSiRemove(TempFile);
	return rtn;
}

static int decodeGoogleLocation(LPSTR url, LPDPOINT pLocPoint,LPBOOL pHaveVPPoints, LPDPOINT pVPPoints, char * formattedAddress,LPSTR locType,LPSTR types)
{
	unsigned int i;
	char *text;
	double latitude, longitude;
	int rtn = -1, nResults = 0;

	json_t *root;
	json_error_t error;
	json_t *status;
	json_t *results;
	const char * status_text;
	//add bounds to restrict results, return only results in bounds
	//&bounds = 34.172684, -118.604794 | 34.236144, -118.50093
	*pHaveVPPoints = FALSE;
	text = requestFromURL(url);
	if (!text)
		return -1;

	root = json_loads(text, 0, &error);
	free(text);

	if (!root)
	{
		fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
		return -1;
	}

	status = json_object_get(root, "status");
	status_text = json_string_value(status);
	if (!stricmp(status_text, "OK"))
	{
		results = json_object_get(root, "results");
		if (!json_is_array(results))
		{
			fprintf(stderr, "error: results is not an array\n");
			goto Exit;
		}

		for (i = 0; i < json_array_size(results); i++)
		{
			json_t *result, *formatted_address, *message, *geometry, *location, *location_type, *lat, *lng;
			const char *message_text, *formattedadd, *locationtype;

			result = json_array_get(results, i);
			if (!json_is_object(result))
			{
				fprintf(stderr, "error: result %d is not an object\n", i + 1);
				goto Exit;
			}

			formatted_address = json_object_get(result, "formatted_address");
			if (!json_is_string(formatted_address))
			{
				fprintf(stderr, "error: formatted_address %d: id is not a string\n", i + 1);
				goto Exit;
			}

			geometry = json_object_get(result, "geometry");
			if (!json_is_object(geometry))
			{
				fprintf(stderr, "error: geometry %d: message is not an object\n", i + 1);
				goto Exit;
			}
			location = json_object_get(geometry, "location");
			location_type = json_object_get(geometry, "location_type");
			lat = json_object_get(location, "lat");
			lng = json_object_get(location, "lng");
			formattedadd = json_string_value(formatted_address);
			locationtype = json_string_value(location_type);
			if (stricmp(locationtype, "APPROXIMATE") &&
				stricmp(locationtype, "GEOMETRIC_CENTERx"))
			{
				strcpy(formattedAddress, formattedadd);
				strcpy(locType, locationtype);
				pLocPoint->y = json_real_value(lat);
				pLocPoint->x = json_real_value(lng);
				nResults++;
			}
		}
	}
	if (!nResults)
		strcpy(formattedAddress, status_text);
	rtn = nResults;
Exit:
	json_decref(root);
	return rtn;
}


int GetGoogleLocation(LPSTR FullAddressIN, int wantMatch, LPSTR formattedAddress, LPDPOINT pLocPoint, LPBOOL pHaveVPPoints, LPDPOINT pVPPoints, LPSTR locType, LPSTR types)
//returns num matches found, -1 if request fails, -2 if unable to convert coord.
{
	int		rtn = 0;
	char	CMD[512], fmt[] = "https://maps.googleapis.com/maps/api/geocode/json?address=%s&sensor=false&key=%s";
	//char	CMD[512], fmt[] = "https://maps.googleapis.com/maps/api/geocode/json?address=%s&sensor=true";
	char	TempFile[MAX_PATH], FullAddress[256];

	strcpy(FullAddress, FullAddressIN);
	REPLAC(FullAddress, "&", "AND", 256);
	REPLAC(FullAddress, "/", " AND ", 256);
	REPLAC(FullAddress, " ", "+", 256);
	sprintf(CMD, fmt, FullAddress, GOOGLE_SERVER_KEY);
	//sprintf(CMD, fmt, FullAddress);
	rtn = decodeGoogleLocation(CMD, pLocPoint, pHaveVPPoints, pVPPoints, formattedAddress, locType, types);
	if (rtn > 0)
	{
		if (ConvertCoord(pLocPoint, 2, 1))
			rtn = -2;
	}
	return rtn;
}
static int retrieveHistoricWeather(LPSTR url,int refno,int time,LPSTR cDate,HFILE outFid)
{
	char* text;
	int rtn = -1;
	char outRec[1024];

	text = requestFromURL(url);
	if (!text)
		return -1;
	int textLength = strlen(text);
	sprintf(outRec, "%i\t%i\t%s\t%s", refno, time,cDate, text);
	fputstring(outRec, outFid);
	free(text);
	return textLength;
}
 int decodeHistoricWeather(LPSTR text,LPSTR precipVar,LPSTR windMinVar,LPSTR windMaxVar,LPSTR tempMinVar,LPSTR tempMaxVar)
{
	unsigned int i;
	char setVars[256];
	double latitude, longitude;
	int rtn = -1, nResults = 0;

	json_t* root;
	json_error_t error;
	json_t* status;
	json_t* precipTotal;
	json_t* results;
	const char* status_text;
	//add bounds to restrict results, return only results in bounds
	//&bounds = 34.172684, -118.604794 | 34.236144, -118.50093
	
	int textLength = strlen(text);
	root = json_loads(text, 0, &error);

	if (!root)
	{
		fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
		return -1;
	}

	status = json_object_get(root, "precipitation");
	precipTotal = json_object_get(status, "total");
	float precip = json_real_value(precipTotal);
	sprintf(setVars, "[%s]=%f", precipVar, precip);
	ProcessText(setVars);
	nResults = 1;
/*	if (!stricmp(status_text, "OK"))
	{
		results = json_object_get(root, "results");
		if (!json_is_array(results))
		{
			fprintf(stderr, "error: results is not an array\n");
			goto Exit;
		}

		for (i = 0; i < json_array_size(results); i++)
		{
			json_t* result, * formatted_address, * message, * geometry, * location, * location_type, * lat, * lng;
			const char* message_text, * formattedadd, * locationtype;

			result = json_array_get(results, i);
			if (!json_is_object(result))
			{
				fprintf(stderr, "error: result %d is not an object\n", i + 1);
				goto Exit;
			}

			formatted_address = json_object_get(result, "formatted_address");
			if (!json_is_string(formatted_address))
			{
				fprintf(stderr, "error: formatted_address %d: id is not a string\n", i + 1);
				goto Exit;
			}

			geometry = json_object_get(result, "geometry");
			if (!json_is_object(geometry))
			{
				fprintf(stderr, "error: geometry %d: message is not an object\n", i + 1);
				goto Exit;
			}
			location = json_object_get(geometry, "location");
			location_type = json_object_get(geometry, "location_type");
			lat = json_object_get(location, "lat");
			lng = json_object_get(location, "lng");
			formattedadd = json_string_value(formatted_address);
			locationtype = json_string_value(location_type);
			if (stricmp(locationtype, "APPROXIMATE") &&
				stricmp(locationtype, "GEOMETRIC_CENTERx"))
			{
				//strcpy(formattedAddress, formattedadd);
				//strcpy(locType, locationtype);
				//pLocPoint->y = json_real_value(lat);
				//pLocPoint->x = json_real_value(lng);
				nResults++;
			}
		}
	}
	*/
//	if (!nResults)
//		strcpy(formattedAddress, status_text);
	rtn = nResults;
Exit:
	json_decref(root);
	return rtn;
}

int GetHistoricWeatherData(DPOINT point,int refno,int time,LPSTR outFile)
//returns num matches found, -1 if request fails, -2 if unable to convert coord. 1712898000
{
	int		rtn = 0;
	//char	CMD[512], fmt[] = "https://maps.googleapis.com/maps/api/geocode/json?address=%s&sensor=false&key=%s";
	char	CMD[512], fmt[] = "https://api.openweathermap.org/data/3.0/onecall/day_summary?lat=%f&lon=%f&units=imperial&appid=38dde0e75b863c1dad14cb6d2a7f423e&date=%s";
	char	TempFile[MAX_PATH], FullAddress[256];
	char	cDate[32];
	DPOINT	latlon = point;
	HFILE fid;
	if (ConvertCoord(&latlon, 1, 2))
		rtn = -2;
	else
	{
		if (ExistFile(outFile))
		{
			fid = GSSiOpenFile(outFile, 0, OF_WRITE);
			GSSillseek(fid, 0, 2);
		}
		else
		{
			fid = GSSiOpenFile(outFile, 0, OF_CREATE);
			fputstring("REFNO\tTIME\tDATE\tDATA", fid);
		}
		sprintf(cDate, "$BEFORE($CAL(%i, 3), )", time);
		ExpandText(cDate);
		sprintf(CMD, fmt, latlon.y,latlon.x,cDate);
		//sprintf(CMD, fmt, FullAddress);
		rtn = retrieveHistoricWeather(CMD, refno, time,cDate, fid);
		//rtn = decodeHistoricWeather(CMD);
		GSSiClose(fid);
	}
	return rtn;
}

