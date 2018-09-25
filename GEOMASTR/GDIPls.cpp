

#define UNICODE
#ifndef STRICT
#define STRICT
#endif

#include <windows.h>
#include <gdiplus.h>
#include <math.h>
#include <stdio.h>
#include "gssitype.h"     

extern  "C" int defaultAreaTransparency;
extern  "C" int defaultLineTransparency;

#define GetIValue(rgb)      (LOBYTE((rgb)>>24))

extern "C" COLORREF RGBI(int r, int g, int b, int i);
extern "C" HGDIOBJ SelObject(HDC hdc, HGDIOBJ hobj);
extern "C" int CurvePointsD(LPDPOINT PC, LPDPOINT POC, LPDPOINT PT, LPLONG nPnts, HPDPOINT *Points, LPDOUBLE pBackAZ, long MaxPoints, double VectorizationFactor, short LoopFactor);
static ULONG_PTR           gdiplusToken=0;
using namespace Gdiplus;

void testit(HDC hdc)
{
	{
		using namespace Gdiplus;

		Gdiplus::Graphics graphic(hdc);
		DPOINT BP = { 150, 150 }, POC = { 550, 550 }, EP = { 150, 150 };
		long nPnt=0;
		LPDPOINT lpPoints=(LPDPOINT)malloc(4096*32);
		LPDPOINT lpPointsInit = lpPoints;
		double BackAZ;
		double CurveExpansionFactor = 1;

		CurvePointsD(&BP, &POC, &EP, &nPnt, &lpPoints, &BackAZ, USHRT_MAX, CurveExpansionFactor, 1);


		// Pen can also be constructed using a brush or another pen.  There is a second parameter - a width which defaults to 1.0f
		Pen	blue(Color(255, 0, 0, 255),5);
		Pen red(Color(255, 255, 0, 0),5);
		graphic.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighQuality);
		graphic.DrawLine(&blue, 0, 0, 1024, 1024);
		PointF pt1;
		PointF pt2;
		//GraphicsPath pth();
		Gdiplus::GraphicsPath pth;
		Gdiplus::GraphicsPath pth2;

		lpPoints = lpPointsInit;
		for (int i = 0; i < nPnt-1; i++)
		{
			pt1.X = lpPoints[i].x;
			pt1.Y = lpPoints[i].y;
			pt2.X = lpPoints[i + 1].x;
			pt2.Y = lpPoints[i + 1].y;
			pth.AddLine(pt1, pt2);
			pt1.X = lpPoints[i].x+50;
			pt1.Y = lpPoints[i].y;
			pt2.X = lpPoints[i + 1].x+50;
			pt2.Y = lpPoints[i + 1].y;
			pth2.AddLine(pt1, pt2);
		}
		graphic.DrawPath(&blue, &pth);
		Gdiplus::Size size(500, 500);
		Gdiplus::Point pt(100, 100);
		graphic.DrawEllipse(&blue, 200, 200, 400, 400);
		graphic.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighSpeed);
		graphic.DrawPath(&blue, &pth2);
		graphic.DrawLine(&blue, 32, 0, 1056, 1024);
		return;
	}
}


extern "C" void testGDIP(HDC hdc)
{
	
	HWND                hWnd;
	
	MSG                 msg;
	WNDCLASS            wndClass;
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartupOutput gdiplusStartupOutput;

	// Initialize GDI+.
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, &gdiplusStartupOutput);
	testit(hdc);
	GdiplusShutdown(gdiplusToken);

	return;
}

extern "C" void AAShutDown(void)
{
	if (gdiplusToken)
		GdiplusShutdown(gdiplusToken);
	return;
}

extern "C" void AAPolyLine(HDC hdc, LPPOINT pPoints, int np, COLORREF ColorRef, float w)
{
	using namespace Gdiplus;
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartupOutput gdiplusStartupOutput;
	HPEN hpn = (HPEN)SelObject(hdc, GetStockObject(BLACK_PEN));
	HBRUSH hbr = (HBRUSH)SelObject(hdc, GetStockObject(BLACK_BRUSH));
	LineCap lincap = LineCapFlat;

	if (!gdiplusToken)
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, &gdiplusStartupOutput);
	{
		int intensity = GetIValue(ColorRef);
		if (!intensity)
			intensity = defaultLineTransparency;

		Gdiplus::Graphics graphic(hdc);
		graphic.SetPageUnit(UnitPixel);
		graphic.SetCompositingQuality(CompositingQualityHighQuality);
		int r = GetRValue(ColorRef), g = GetGValue(ColorRef), b = GetBValue(ColorRef);
		Pen pn(Color(intensity, r,g,b), w);
		graphic.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighQuality);
		//graphic.DrawLine(&blue, 0, 0, 1024, 1024);
		PointF pt1;
		PointF pt2;
		Gdiplus::GraphicsPath pth;
		pn.SetLineJoin(Gdiplus::LineJoin::LineJoinRound);
		pn.SetStartCap(lincap);
		pn.SetEndCap(lincap);
		for (int i = 0; i < np - 1; i++)
		{
			pt1.X = pPoints[i].x;
			pt1.Y = pPoints[i].y;
			pt2.X = pPoints[i + 1].x;
			pt2.Y = pPoints[i + 1].y;
			pth.AddLine(pt1, pt2);
		}
		graphic.DrawPath(&pn, &pth);
	}
	SelObject(hdc, hpn);
	SelObject(hdc, hbr);
}
extern "C" void AAPolyLineF(HDC hdc, LPFPOINT pPoints, int np, COLORREF ColorRef, float w)
{
	using namespace Gdiplus;
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartupOutput gdiplusStartupOutput;
	HPEN hpn = (HPEN)SelObject(hdc, GetStockObject(BLACK_PEN));
	HBRUSH hbr = (HBRUSH)SelObject(hdc, GetStockObject(BLACK_BRUSH));
	LineCap lincap = LineCapFlat;

	if (!gdiplusToken)
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, &gdiplusStartupOutput);
	{

		Gdiplus::Graphics graphic(hdc);
		graphic.SetPageUnit(UnitPixel);
		graphic.SetCompositingQuality(CompositingQualityHighQuality);
		Pen pn(Color(255, GetRValue(ColorRef), GetGValue(ColorRef), GetBValue(ColorRef)), w);
		graphic.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighQuality);
		//graphic.DrawLine(&blue, 0, 0, 1024, 1024);
		PointF pt1;
		PointF pt2;
		Gdiplus::GraphicsPath pth;
		pn.SetLineJoin(Gdiplus::LineJoin::LineJoinRound);
		pn.SetStartCap(lincap);
		pn.SetEndCap(lincap);
		for (int i = 0; i < np - 1; i++)
		{
			pt1.X = pPoints[i].x;
			pt1.Y = pPoints[i].y;
			pt2.X = pPoints[i + 1].x;
			pt2.Y = pPoints[i + 1].y;
			pth.AddLine(pt1, pt2);
		}
		graphic.DrawPath(&pn, &pth);
	}
	SelObject(hdc, hpn);
	SelObject(hdc, hbr);
}

extern "C" void AAPolygon(HDC hdc, LPPOINT pPoints, int np, LOGPEN *lp, LOGBRUSH *lb)
{

	using namespace Gdiplus;
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartupOutput gdiplusStartupOutput;
	HPEN hpn = (HPEN)SelObject(hdc, GetStockObject(BLACK_PEN));
	HBRUSH hbr = (HBRUSH)SelObject(hdc, GetStockObject(BLACK_BRUSH));
	LineCap lincap = LineCapRound;

	if (!gdiplusToken)
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, &gdiplusStartupOutput);
	{

		Gdiplus::Graphics graphic(hdc);
		graphic.SetPageUnit(UnitPixel);
		graphic.SetCompositingQuality(CompositingQualityHighQuality);

		PointF pt1;
		PointF pt2;
		Gdiplus::GraphicsPath pth;
		for (int i = 0; i < np - 1; i++)
		{
			pt1.X = pPoints[i].x;
			pt1.Y = pPoints[i].y;
			pt2.X = pPoints[i + 1].x;
			pt2.Y = pPoints[i + 1].y;
			pth.AddLine(pt1, pt2);
		}
		if (np && (pt2.X != pPoints[0].x || pt2.Y != pPoints[0].y))
		{
			pt1.X = pPoints[0].x;
			pt1.Y = pPoints[0].y;
			pth.AddLine(pt2, pt1);
		}
		if (lb->lbStyle != BS_NULL)
		{
			if (lb->lbStyle == BS_HATCHED)
			{
				HatchBrush hbr((HatchStyle)lb->lbHatch, Color(255, GetRValue(lb->lbColor), GetGValue(lb->lbColor), GetBValue(lb->lbColor)), Color(64, GetRValue(lb->lbColor), GetGValue(lb->lbColor), GetBValue(lb->lbColor)));
				graphic.FillPath(&hbr, &pth);
			}
			else if (lb->lbStyle == BS_PATTERN)
			{
			}
			else
			{
				extern BOOL maxIntensity;
				int intensity = GetIValue(lb->lbColor);
				if (!intensity)
					intensity = defaultAreaTransparency;
				if (maxIntensity)
					intensity = 255;
				SolidBrush sbr(Color(intensity, GetRValue(lb->lbColor), GetGValue(lb->lbColor), GetBValue(lb->lbColor)));
				graphic.FillPath(&sbr, &pth);
				graphic.Flush();
			}
		}
		if (lp && lp->lopnStyle != PS_NULL)
		{
			Pen pn(Color(255, GetRValue(lp->lopnColor), GetGValue(lp->lopnColor), GetBValue(lp->lopnColor)), lp->lopnWidth.x);
			graphic.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighQuality);
			pn.SetLineJoin(Gdiplus::LineJoin::LineJoinRound);
			pn.SetStartCap(lincap);
			pn.SetEndCap(lincap);
			graphic.DrawPath(&pn, &pth);
		}
	}
	SelObject(hdc, hpn);
	SelObject(hdc, hbr);
}

/*VOID OnPaint(HDC hdc)
{
	Gdiplus::Graphics graphics(hdc);

	Bitmap b(L"Calvin.jpg");
	Bitmap* b2;

	INT iWidth = b.GetWidth();
	INT iHeight = b.GetHeight();

	Rect rect(0, 0, iWidth, iHeight);
	b2 = b.Clone(rect, PixelFormat24bppRGB);


	BitmapData bmData;
	BitmapData bmData2;

	b.LockBits(&rect, ImageLockModeRead | ImageLockModeWrite, PixelFormat24bppRGB, &bmData);
	b2->LockBits(&rect, ImageLockModeRead | ImageLockModeWrite, PixelFormat24bppRGB, &bmData2);

	int stride = bmData.Stride;

	unsigned char * p = (unsigned char *)bmData.Scan0;
	unsigned char * p2 = (unsigned char *)bmData2.Scan0;


	int nOffset = stride - iWidth * 3;
	int nWidth = iWidth * 3;

	int nPixel = 0, nPixelMax = 0;

	p += stride;
	p2 += stride;
	int nThreshold = 0;

	for (int y = 1; y<iHeight - 1; ++y)
	{
		p += 3;
		p2 += 3;

		for (int x = 3; x < nWidth - 3; ++x)
		{


			nPixelMax = abs((p2 - stride + 3)[0] - (p2 + stride - 3)[0]);
			nPixel = abs((p2 + stride + 3)[0] - (p2 - stride - 3)[0]);
			if (nPixel>nPixelMax) nPixelMax = nPixel;

			nPixel = abs((p2 - stride)[0] - (p2 + stride)[0]);
			if (nPixel>nPixelMax) nPixelMax = nPixel;

			nPixel = abs((p2 + 3)[0] - (p2 - 3)[0]);
			if (nPixel>nPixelMax) nPixelMax = nPixel;

			if (nPixelMax < nThreshold)
				nPixelMax = 0;

			p[0] = (byte)nPixelMax;

			++p;
			++p2;

		}

		p += 3 + nOffset;
		p2 += 3 + nOffset;
	}



	b.UnlockBits(&bmData);
	b2->UnlockBits(&bmData2);

	graphics.DrawImage(b2, 0, 0, iWidth, iHeight);
	graphics.DrawImage(&b, iWidth + 10, 0, iWidth, iHeight);

}


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT iCmdShow)
{
	HWND                hWnd;
	MSG                 msg;
	WNDCLASS            wndClass;
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	// Initialize GDI+.
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = TEXT("Edge Detection");

	RegisterClass(&wndClass);

	hWnd = CreateWindow(
		TEXT("Edge Detection"),   // window class name
		TEXT("Edge Detection"),  // window caption
		WS_OVERLAPPEDWINDOW,      // window style
		CW_USEDEFAULT,            // initial x position
		CW_USEDEFAULT,            // initial y position
		CW_USEDEFAULT,            // initial x size
		CW_USEDEFAULT,            // initial y size
		NULL,                     // parent window handle
		NULL,                     // window menu handle
		hInstance,                // program instance handle
		NULL);                    // creation parameters

	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	GdiplusShutdown(gdiplusToken);
	return msg.wParam;
}  // WinMain


LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	HDC          hdc;
	PAINTSTRUCT  ps;

	switch (message)
	{
	case WM_CREATE:
		//OnCreate();
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		OnPaint(hdc);
		EndPaint(hWnd, &ps);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
} // WndProc

*/






