// GraphicsPathTest.cpp : Defines the entry point for the application.
//
#include <windows.h>
#include "framework.h"
#include "GraphicsPathTest.h"
//#include <gdiplus.h>
//using namespace Gdiplus;
#define IDS_APP_TITLE			103
#define	GF_TEXTMODE		202  
#define GF_SCREENMODE	203

#define IDR_MAINFRAME			128
#define IDD_GRAPHICSPATHTEST_DIALOG	102
#define IDD_ABOUTBOX			103
//#define IDM_ABOUT				104
//#define IDM_EXIT				105
#define IDI_GRAPHICSPATHTEST			107
#define IDI_SMALL				108
#define IDC_GRAPHICSPATHTEST			109
#define IDC_MYICON				2
#ifndef IDC_STATIC
#define IDC_STATIC				-1
#endif

#define MAX_LOADSTRING 100
typedef struct { double x, y; } DPOINT;
typedef DPOINT* LPDPOINT;
typedef double* LPDOUBLE;
// Global Variables:
static HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
static ULONG_PTR           gdiplusToken = 0;

extern "C" short TempLineType;
extern "C" BOOL useGDIPlus;
extern "C" COLORREF TrackColor;
extern "C" BOOL FirstMoveSinceRedraw;
extern "C" int TrackWidth;
// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

extern "C" int SetDisplayMode(HDC hDC, short Mode);
extern "C" DPOINT WinPtToBasePtD(LPDPOINT WinPointD);
extern "C" DPOINT WinPtToBasePt(POINT Point);
extern "C" double getazd(LPDPOINT Point1, LPDPOINT Point2);
extern "C" DWORD DispText(HDC hDC, BOOL GetExtents, int left, int right, int y, int symsize, int hJust, int vJust, double Size, double SymSizeFactor, double SymbolTextFactor,
	int Weight, BOOL Italic, double AZ, LPSTR Text, int Symbol, BOOL TestRect,
	BOOL Shadow, COLORREF ShadowColor, long RemoveColor,
	long nPnts, HANDLE hAreaPoints, HANDLE hAreaAccelerator, int nPoly, HANDLE hPolyPartLen,
	short UseHalfTone, LPSTR ActualText, short MinSize, LPVOID CurTheme, LPRECT pTextRect, LPRECT pFullRect, LPRECT pFlagRect);
extern "C" BOOL ShowTempLineType(HDC hDC, LPDPOINT pBasePoint, LPDPOINT lpDPoint, LPDOUBLE pTotDist);
extern "C" void TempPolyline(HDC hDC, LPPOINT lpPoints, short nPnts, LPSTR TopText, LPSTR BottomText);
void TestPath(HDC hdc);
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GRAPHICSPATHTEST, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GRAPHICSPATHTEST));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = 0;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GRAPHICSPATHTEST));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GRAPHICSPATHTEST);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void DisplayPath(HDC hdc,POINT *points, BYTE *types, int npts)
{
	int istart = 0;
	int np = 0;
	for (int i = 0; i < npts; i++)
	{
		if (types[i] == PT_MOVETO)
		{
			POINT ptsave = points[istart + np];
			points[istart + np] = points[istart];
			Polyline(hdc, &points[istart], np + 1);
			points[istart + np] = ptsave;
			np = 0;
			istart = i;
		}
		else
			np++;
	}
	POINT ptsave = points[istart + np];
	points[istart + np] = points[istart];
	Polyline(hdc, &points[istart], np + 1);
	points[istart + np] = ptsave;
	return;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
extern "C" void TempPolyline_new(HDC hDC, LPPOINT lpPoints, short nPnts, LPSTR TopText, LPSTR BottomText)
{
	HPEN hTrackPen, hOldPen;
	double	AZ, dist;
	char	txt[64];
	POINT	Point1, Point2, point;
	LPPOINT	pPoint1, pPoint2;
	DPOINT	DPoint1, DPoint2;
	float	size;

	hTrackPen = CreatePen(PS_SOLID, TrackWidth, 0);
	hOldPen = (HPEN)SelectObject(hDC, hTrackPen);
	point = lpPoints[1];
	//sprintf(txt, "\nTempPolyLine: %i %i %i %i", lpPoints->x, lpPoints->y, point.x, point.y);
	//OutputDebugString(txt);

	Polyline(hDC, lpPoints, nPnts);
	SelectObject(hDC, hOldPen);
	DeleteObject(hTrackPen);
	if (TempLineType == 1 && TopText)
	{
		size = -0.2;
		AZ = 0;
		nPnts--;
		pPoint2 = lpPoints;
		pPoint2 += nPnts;
		pPoint1 = pPoint2;
		pPoint1--;
		Point1 = *pPoint1;
		Point2 = *pPoint2;
		DPoint1 = WinPtToBasePt(Point1);
		DPoint2 = WinPtToBasePt(Point2);
		point.x = Point1.x + (Point2.x - Point1.x) / 2;
		point.y = Point1.y + (Point2.y - Point1.y) / 2;
		AZ = getazd(&DPoint1, &DPoint2);
		DispText(hDC, FALSE, point.x, point.x, point.y, 0, 4, 2, size, 1, 1, 100, FALSE, AZ, TopText, 0, FALSE, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	}
	
	return;
}

extern "C" void NotPolylineScreen_new(HDC hDC, LPPOINT Points, short nPnts, LPSTR TopText, LPSTR BottomText, LPDOUBLE pTotDist)
{
	static int ncalls = 0;
	short	OldMode;
	POINT points[1024];
	BYTE  types[1024];
	int st;

	int ii;
	//BOOL saveUseGDIPlus = useGDIPlus;
	//TestPath(hDC);
	useGDIPlus =  TRUE;
	//SaveDC(hDC);
	SetDisplayMode(hDC, GF_SCREENMODE);
	//SelectClipRgn(hDC, 0);
	SetBkMode(hDC, TRANSPARENT);

	if (ncalls > 100)
		ii = 1;
	OldMode = SetROP2(hDC, R2_NOT);
//	BeginPath(hDC);
	if (ncalls++ % 2)
		TrackColor = 0;
	else
		TrackColor = 0;// RGB(255, 0, 0);
	if (!FirstMoveSinceRedraw)
	{
		//OldMode = SetROP2(hDC,R2_NOT); 
		if (pTotDist)
		{
			DPOINT pt1 = WinPtToBasePt(Points[0]);
			DPOINT pt2 = WinPtToBasePt(Points[nPnts - 1]);
			ShowTempLineType(hDC, &pt1, &pt2, pTotDist);
		}
		else
			TempPolyline(hDC, Points, nPnts, 0, 0);
		//SetROP2(hDC,OldMode); 
	}
	FirstMoveSinceRedraw = FALSE;
//	EndPath(hDC);
	int npts = GetPath(hDC, points, types, 1024);
	st = StrokePath(hDC);
	GdiFlush();
	SetROP2(hDC, OldMode);
	//RestoreDC(hDC, -1);
	//useGDIPlus = saveUseGDIPlus;
	return;
}

void TempPolyline_del(HDC hDC, LPPOINT lpPoints, short nPnts, LPSTR TopText, LPSTR BottomText)
{
	HPEN hTrackPen, hOldPen;
	double	AZ, dist;
	char	txt[64];
	POINT	Point1, Point2, point;
	LPPOINT	pPoint1, pPoint2;
	DPOINT	DPoint1, DPoint2;
	float	size;

	hTrackPen = CreatePen(PS_SOLID, 1, 0);
	hOldPen = (HPEN)SelectObject(hDC, hTrackPen);
	point = lpPoints[1];
	//sprintf(txt, "\nTempPolyLine: %i %i %i %i", lpPoints->x, lpPoints->y, point.x, point.y);
	//OutputDebugString(txt);

	Polyline(hDC, lpPoints, nPnts);
	SelectObject(hDC, hOldPen);
	DeleteObject(hTrackPen);
/*	if (TempLineType == 1 && TopText)
	{
		size = -0.2;
		AZ = 0;
		nPnts--;
		pPoint2 = lpPoints;
		pPoint2 += nPnts;
		pPoint1 = pPoint2;
		pPoint1--;
		Point1 = *pPoint1;
		Point2 = *pPoint2;
		DPoint1 = WinPtToBasePt(Point1);
		DPoint2 = WinPtToBasePt(Point2);
		point.x = Point1.x + (Point2.x - Point1.x) / 2;
		point.y = Point1.y + (Point2.y - Point1.y) / 2;
		AZ = getazd(&DPoint1, &DPoint2);
		DispText(hDC, FALSE, point.x, point.x, point.y, 0, 4, 2, size, 1, 1, 100, FALSE, AZ, TopText, 0, FALSE, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	}*/
	return;
}


extern "C" LRESULT CALLBACK WndProcTest(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
               // DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
		BOOL st;
		//using namespace Gdiplus;
		//GdiplusStartupInput gdiplusStartupInput;
		//GdiplusStartupOutput gdiplusStartupOutput;
		//if (!gdiplusToken)
		//	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, &gdiplusStartupOutput);
		PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
			TestPath(hdc);
			EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void TestPath(HDC hdc)
{
	int st;
	//Gdiplus::Graphics graphic(hdc);
// TODO: Add any drawing code that uses hdc here...
	POINT pt[2];
	POINT points[1024];
	BYTE  types[1024];
	//PointF ptf[2];
	HFONT hFont = CreateFont((int)32, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, "Courier New");
	HPEN hPen = CreatePen(PS_SOLID, 1, 0);
	pt[0] = { 100,100 };
	pt[1] = { 500,500 };
	//ptf[0] = { 100,100 };
	//ptf[1] = { 500,500 };
	SetDisplayMode(hdc, GF_SCREENMODE);

	BeginPath(hdc);
	SelectObject(hdc, hFont);
	SelectObject(hdc, hPen);
	SelectObject(hdc, GetStockObject(BLACK_BRUSH));
	SetBkMode(hdc, TRANSPARENT);
	Polyline(hdc, pt, 2);
	st = TextOut(hdc, 200, 300, "1234567890.+-", 13);
	EndPath(hdc);
	//st = FlattenPath(hdc);
	int npts = GetPath(hdc, points, types, 1024);
	//DisplayPath(hdc, points, types,  npts);
	GdiFlush();
	//Sleep(1000);
	StrokePath(hdc);
	//npts = GetPath(hdc, points, types, 1024);
	Sleep(100);
	SetROP2(hdc, R2_NOT);
	//DisplayPath(hdc, points, types, npts);
	//Sleep(1000);
	BeginPath(hdc);
	SelectObject(hdc, hFont);
	SelectObject(hdc, hPen);
	SelectObject(hdc, GetStockObject(BLACK_BRUSH));
	SetBkMode(hdc, TRANSPARENT);
	Polyline(hdc, pt, 2);
	st = TextOut(hdc, 200, 300, "1234567890.+-", 13);
	EndPath(hdc);
	StrokePath(hdc);
	GdiFlush();
	//SelectClipPath(hdc, 5);

	// This generates the same result as SelectClipPath() 
	// SelectClipRgn(hdc, PathToRegion(hdc)); 

	// Fill the region with grayness 
	//FillRect(hdc, &ps.rcPaint,(HBRUSH) GetStockObject(BLACK_BRUSH));

	GdiFlush();
	//Sleep(2000);
	SetROP2(hdc, R2_NOT);
	//Polyline(hdc, pt, 2);
	//TextOut(hdc, 200, 300, "Hi There Fatty!!!", 17);
	GdiFlush();
	/*
	FontFamily  fontFamily(L"Times New Roman");
	Font        font(&fontFamily, 24, FontStyleRegular, UnitPixel);
	PointF      pointF(300.0f, 100.0f);
	SolidBrush  solidBrush(Color(255, 0, 0, 255));
	Gdiplus::GraphicsPath pth;
	//Gdiplus:StringFormat fmt;
	Pen pn(Color(255, 0, 0, 255), 5);
	pn.SetLineJoin(Gdiplus::LineJoin::LineJoinRound);
	pn.SetStartCap(LineCapRound);
	pn.SetEndCap(LineCapRound);
	pth.AddLine(ptf[0], ptf[1]);
	pth.AddString(L"Hello", -1, &fontFamily, FontStyleRegular,48,pointF,NULL);
		IN const WCHAR * string,
		IN INT                  length,
		IN const FontFamily * family,
		IN INT                  style,
		IN REAL                 emSize,  // World units
		IN const PointF & origin,
		IN const StringFormat * format
	)

	//graphic.DrawPath(&pn, &pth);
	Sleep(2000);
	SetROP2(hdc, R2_NOT);
	//graphic.DrawPath(&pn, &pth);

	//graphic.DrawString(L"Hello", -1, &font, pointF, &solidBrush);
	*/

}
// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}