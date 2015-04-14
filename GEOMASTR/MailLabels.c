#include "graphint.h"   
#include "gmextern.h"

static void DrawStuff(HDC hdc);
static void InitDocStruct(DOCINFO* di, char* docname);
static BOOL CALLBACK AbortProcML(HDC hDC, int Error);
static HDC GetPrinterDC(void);

static BOOL displayImage(HDC hDC, HDIB32 hDIB32, int x, int y,int width, int height)
{
	BOOL rtn = FALSE;
	RECT rect;
	if (hDIB32)
	{
		BITMAPINFOHEADER DibInfo;

		GetBitmapInfoFromHandle(&DibInfo, hDIB32);
		rect.left = x;
		rect.top = y;
		rect.right = rect.left + width;
		rect.bottom = rect.top + height;
		rtn = DisplayBMInRect32(hDC, hDIB32, rect, TRUE);
		DestroyDIB32(hDIB32, FALSE);
	}

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
BOOL PrintMailLabels(int nArgs, LPSTR *Arg, LPSTR OutLoc)
{
	HWND hWndParent = hWndMain;
	BOOL rtn = TRUE;
	HDC        hDC;
	DOCINFO    di;
	char ImagePath[MAX_PATH]="[%DL]icons\\maillabel.bmp";

	// Need a printer DC to print to.
	hDC = GetPrinterDC();

	// Did you get a good DC?
	if (!hDC)
	{
		MessageBox(NULL, "Error creating DC", "Error",
			MB_APPLMODAL | MB_OK);
		return FALSE;
	}
	else
	{
		int pageHeight = GetDeviceCaps(hDC, VERTRES);
		int pageWidth = GetDeviceCaps(hDC, HORZRES);
		int height = pageHeight / 10;
		int width = pageWidth / 3;

		HDIB32 hDIB32 = LoadDIB32(ImagePath, FALSE);

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
		DrawStuff(hDC);
		EndPage(hDC);

		// Indicate end of document.
		EndDoc(hDC);

		// Clean up
		DeleteDC(hDC);
	}
	return rtn;
}

/*===============================*/
/* Obtain printer device context */
/* ==============================*/
static HDC GetPrinterDC(void)
{
	PRINTDLG pdlg;

	// Initialize the PRINTDLG structure.
	memset(&pdlg, 0, sizeof(PRINTDLG));
	pdlg.lStructSize = sizeof(PRINTDLG);
	// Set the flag to return printer DC.
	pdlg.Flags = PD_RETURNDC;

	// Invoke the printer dialog box.
	PrintDlg(&pdlg);
	// hDC member of the PRINTDLG structure contains
	// the printer DC.
	return pdlg.hDC;
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
static void DrawStuff(HDC hdc)
{
	// This is the function that does draws on a given DC.
	// You are printing text here.
	TextOut(hdc, 0, 0, "Test Printing", lstrlen("Test Printing"));
}
