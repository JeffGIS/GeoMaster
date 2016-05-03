#include "graphint.h"   
#include "gmextern.h"

// format fits Avery 5160 labels

static void DrawStuff(HDC hdc, HDIB32 hDIB32, int x, int y, int width, int height, char lines[4][256],BOOL wantImage);
static void InitDocStruct(DOCINFO* di, char* docname);
static BOOL CALLBACK AbortProcML(HDC hDC, int Error);
static HDC GetPrinterdc(void);

static BOOL displayImage(HDC hDC, HDIB32 hDIB32, int x, int y,int width, int height)
{
	BOOL rtn = FALSE;
	RECT rect;
	int border = height * 0.05;
	if (hDIB32)
	{
		BITMAPINFOHEADER DibInfo;

		GetBitmapInfoFromHandle(&DibInfo, hDIB32);
		rect.left = x;
		rect.top = y+border;
		rect.right = rect.left + width;
		rect.bottom = rect.top + height - border;
		rtn = DisplayBMInRect32(hDC, hDIB32, rect, FALSE);
	}
	return rtn;
}
/*===============================*/
/* The Abort Procudure           */
/* ==============================*/
static BOOL CALLBACK AbortProcML(HDC hDC, int Error)
{
	MSG   msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return TRUE;
}

/*==============================================*/
/* Sample code :  Typical printing process      */
/* =============================================*/
BOOL PrintMailLabels(int nArgs, LPSTR *Arg, LPSTR OutLoc)//print to 5160 labels
{
	HWND hWndParent = hWndMain;
	BOOL rtn = FALSE;
	HDC        hDC;
	DOCINFO    di;
	HANDLE	hDB = 0;
	char ImagePath[MAX_PATH]="[%DL]icons\\maillabel.bmp";

	// Need a printer DC to print to.
	hDC = GetPrinterdc();

	// Did you get a good DC?
	if (!hDC)
	{
//		MessageBox(NULL, "Error creating DC", "Error",
//			MB_APPLMODAL | MB_OK);
		return FALSE;
	}
	else if (OpenDataFile(Arg[1], "", BT_READ, &hDB))
	{
		int pageHeight = GetDeviceCaps(hDC, VERTRES);
		int pageHeightmm = GetDeviceCaps(hDC, VERTSIZE);
		int pageWidth = GetDeviceCaps(hDC, HORZRES);
		int pageWidthmm = GetDeviceCaps(hDC, HORZSIZE);
		double pageWidthInch = 0.0393701 * pageWidthmm;
		double pageHeightInch = 0.0393701 * pageHeightmm;
		double pixPerInchH = pageHeight / pageHeightInch;
		double pixPerInchW = pageWidth / pageWidthInch;
		int physWidth = GetDeviceCaps(hDC, PHYSICALWIDTH);
		int physHeight = GetDeviceCaps(hDC, PHYSICALHEIGHT);
		int physOffsetX = GetDeviceCaps(hDC, PHYSICALOFFSETX);
		int physOffsetY = GetDeviceCaps(hDC, PHYSICALOFFSETY);
		double xInch = 0.25, yInch = 0.5;
		int x = xInch * pixPerInchW - physOffsetX;
		int y = yInch * pixPerInchH - physOffsetY;
		int startY = y;
		int height = (pageHeight - y*2) / 10;
		int width = pixPerInchW * 2.75;// (pageWidth - x * 2) / 3;
		HFONT hFont, OldFont;
		int PointSize = 8;
		int FontSize = -MulDiv(PointSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
		int nLabels = NumSQLRows(hDB);
		HDIB32 hDIB32 = LoadDIB32(ImagePath, FALSE);
		char lines[4][256];

		// You always have to use an AbortProc().
		if (SetAbortProc(hDC, AbortProcML) == SP_ERROR)
		{
			MessageBox(NULL, "Error setting up AbortProc",
				"Error", MB_APPLMODAL | MB_OK);
			return FALSE;
		}

		// Init the DOCINFO and start the document.
		InitDocStruct(&di, "MyDoc");
		StartDoc(hDC, &di);

		// Print one page.
		StartPage(hDC);
		hFont = CreateFont(FontSize, 0, 0, 0, FW_NORMAL,0, 0, 0, 0, 0, 0, 0, 0, "Arial");

		OldFont = SelectObject(hDC, hFont);
		//SetTextColor(hDC, RGB(128, 0, 0));
		SetTextColor(hDC, 0);
		while (nLabels > 0)
		{
			y = startY;
			for (int irow = 0; irow < 10; irow++)
			{
				for (int icol = 0; icol < 3; icol++)
				{
					if (!nLabels--)
					{
						//EndPage(hDC);
						goto Exit;
					}
					FetchDBRec(hDB);
					for (int line = 0; line < 4; line++)
					{
						sprintf(lines[line], "[LABELLINE%i]", line + 1);
						ExpandText(lines[line]);
					}
					DrawStuff(hDC, hDIB32, x + (icol*width), y, width, height,lines,FALSE);
				}
				y += height;
			}
			if (nLabels)
				EndPage(hDC);
		}

Exit:
		// Indicate end of document.
		EndDoc(hDC);

		// Clean up
		SelectObject(hDC, OldFont);
		DeleteObject(hFont);
		DeleteDC(hDC);
		DestroyDIB32(hDIB32, FALSE);
		CloseDataFile(TRUE, &hDB);
		rtn = TRUE;
	}

	return rtn;
}

/*===============================*/
/* Obtain printer device context */
/* ==============================*/
static HDC GetPrinterdc(void)
{
	PRINTDLG pdlg;

	// Initialize the PRINTDLG structure.
	memset(&pdlg, 0, sizeof(PRINTDLG));
	pdlg.lStructSize = sizeof(PRINTDLG);
	// Set the flag to return printer DC.
	pdlg.Flags = PD_RETURNDC;

	// Invoke the printer dialog box.
	if (PrintDlg(&pdlg))
		// hDC member of the PRINTDLG structure contains
		// the printer DC.
		return pdlg.hDC;
	else
		return 0;
}


/*===============================*/
/* Initialize DOCINFO structure  */
/* ==============================*/
void InitDocStruct(DOCINFO* di, char* docname)
{
	// Always zero it before using it.
	memset(di, 0, sizeof(DOCINFO));
	// Fill in the required members.
	di->cbSize = sizeof(DOCINFO);
	di->lpszDocName = docname;
}

/*===============================*/
/* Drawing on the DC             */
/* ==============================*/
static void DrawStuff(HDC hdc, HDIB32 hDIB32, int x, int y, int width, int height,char lines[4][256],BOOL wantImage)
{
	char str[1024];
	int w = width*(0.7 / 2.72);
	static int n = 1;
	POINT p[2];
	SIZE siz;

	p[0].y = p[1].y = y;
	p[0].x = x;
	p[1].x = x + width;
	//Polyline(hdc, p, 2);
	p[0].y = y;
	p[1].y = y + height;
	p[0].x = x + w;
	p[1].x = x + w;
	//Polyline(hdc, p, 2);


	SetTextAlign(hdc, TA_LEFT | TA_TOP);
	sprintf(str, "Name%i", n);
	if (wantImage)
		displayImage(hdc, hDIB32, x, y, w, height);
	else
		w = w / 5;
	for (int line = 0; line < 4; line++)
	{
		if (*lines[line])
		{
			TextOut(hdc, x + w, y + height / 4, lines[line], lstrlen(lines[line]));
			GetTextExtentPoint32(hdc, lines[line], lstrlen(lines[line]), &siz);
			y += siz.cy;
		}
	}
	/*
	TextOut(hdc, x + w, y + height / 4, str, lstrlen(str));
	GetTextExtentPoint32(hdc, str, lstrlen(str), &siz);
	y += siz.cy;
	sprintf(str, "Address%i", n);
	TextOut(hdc, x + w, y + height / 4, str, lstrlen(str));
	GetTextExtentPoint32(hdc, str, lstrlen(str), &siz);
	y += siz.cy;
	sprintf(str, "City State Sip Code%i", n++);
	TextOut(hdc, x + w, y + height / 4, str, lstrlen(str));
	GetTextExtentPoint32(hdc, str, lstrlen(str), &siz);
	y += siz.cy;*/

}
