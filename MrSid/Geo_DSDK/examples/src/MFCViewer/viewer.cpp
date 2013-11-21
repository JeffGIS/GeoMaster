/* $Id$ */
/* //////////////////////////////////////////////////////////////////////////
//                                                                         //
// This code is Copyright (c) 2006 LizardTech, Inc, 1008 Western Avenue,   //
// Suite 200, Seattle, WA 98104.  Unauthorized use or distribution         //
// prohibited.  Access to and use of this code is permitted only under     //
// license from LizardTech, Inc.  Portions of the code are protected by    //
// US and foreign patents and other filings. All Rights Reserved.          //
//                                                                         //
////////////////////////////////////////////////////////////////////////// */
/* PUBLIC */
#include "stdafx.h"
#include "viewer.h"
#include "assert.h"

// MRSID INCLUDES

// lib/base
#include "lti_pixel.h"
#include "lti_scene.h"
#include "lti_sceneBuffer.h"
#include "lti_navigator.h"
#include "lti_imagestage.h"

// lib/support
#include "lt_base.h"
#include "lt_fileSpec.h" 
#include "lt_status.h"

// lib/MrSIDreaders
#include "MrSIDImageReader.h"
#include "J2KImageReader.h"

// lib/filters
#include "lti_colorTransformer.h"
#include "lti_multiresFilter.h"
#include "lti_viewerImageFilter.h"

LT_USE_NAMESPACE(LizardTech);	

// CONSTANTS
#define MAX_LOADSTRING 100

// GLOBAL VARIABLES
HINSTANCE hInst;						      // current instance
HWND g_hwnd;                           // global window handle
TCHAR szTitle[MAX_LOADSTRING];			// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];	// the main window class name
TCHAR szFileName[MAX_PATH];            // name of currently open file
LTIImageStage* g_reader = NULL;        // Our sid image file reader
LTINavigator *g_navv = NULL;		      // does panning and zooming
RECT g_rect;                           // client width/height
lt_uint32 g_imgWidth,g_imgHeight;      // image width/height
double g_mag, g_magOld;                // current image magnification
double g_imgCntrX, g_imgCntrY;         // image center x/y
double g_scenePosY, g_scenePosX;       // UL of the rendered scene, in GDI coordinates
HBITMAP g_hbmp;                        //	handle to windows bitmap
BITMAPINFO* g_rgbBmpInfo;              //	bitmap info for RGB inputs
BITMAPINFO* g_grayBmpInfo;             //	bitmap info grayscale inputs
lt_int32 g_userX, g_userY;             // user mouse location
lt_int32 g_userXOld, g_userYOld;       // old mouse location

// FORWARD DECLARATIONS
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void UpdateStatusBarText(HWND hwndStatus, LONG x, LONG y);
bool FileOpen(HWND hWnd);
bool OpenImage();
void PanBy(POINT* mouseOld,POINT* mouse);
void ZoomIn();
void ZoomOut();
void UpdateView();
unsigned char* CreateDIB(int numBands);


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_VIEWER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_VIEWER);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
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
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_VIEWER);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCTSTR)IDC_VIEWER;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
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
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent; 
   static HWND hwndStatus;       // handle to status bar
   static UINT wID;              // required for the status bar
   static POINT mouse,mouseOld;  // used for tracking panning distance
   static bool panning = false;
   static LONG prevX, prevY;     // used for updating status bar
	PAINTSTRUCT ps;
	HDC hdc,hdcBuffer;

	switch (message) 
	{
   case WM_CREATE:
      {
         InitCommonControls();

      	//	Color bitmap info
         g_rgbBmpInfo = new BITMAPINFO;
	
         //	Grayscale bitmap info:
         //	This is the official way to do this - we malloc enough for
         //	a BITMAPINFO struct + all of the colors we wish to include in
         //	our color look-up table. Then we initialize the look-up table
         g_grayBmpInfo = (BITMAPINFO*)new BYTE[sizeof(BITMAPINFO) + (255) * sizeof(RGBQUAD)];
	      for(WORD i = 0; i < 256; i++)
	      {
		      g_grayBmpInfo->bmiColors[i].rgbBlue = (BYTE)i;	
		      g_grayBmpInfo->bmiColors[i].rgbRed = (BYTE)i;	
		      g_grayBmpInfo->bmiColors[i].rgbGreen = (BYTE)i;	
		      g_grayBmpInfo->bmiColors[i].rgbReserved = (BYTE)i;	
	      }
         g_rect.left = 0;
         g_rect.top = 0;
         hwndStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE, "", hWnd, wID);
      }
      break;
   case WM_SIZE:      
      {
         // Status bar is not automatically notified of WM_SIZE message.
         // Send it manually
         SendMessage(hwndStatus,WM_SIZE,wParam,lParam);
         g_rect.right = LOWORD(lParam);
         g_rect.bottom = HIWORD(lParam);
         UpdateView();
         InvalidateRect(hWnd,&g_rect,false);
      }
      break;
   case WM_LBUTTONDOWN:
      {
         if(!g_reader)
            break;
         if( wParam & MK_SHIFT ||
            wParam & MK_CONTROL )
            ZoomOut();
         else
            ZoomIn();
         UpdateStatusBarText(hwndStatus,prevX,prevY);
         InvalidateRect(hWnd,&g_rect,false);
      }
      break;
   case WM_RBUTTONDOWN:
      {
         if(!g_reader)
            break;
         panning = true;
         mouseOld.x = mouse.x = LOWORD(lParam);
         mouseOld.y = mouse.y = HIWORD(lParam);
      }
      break;
   case WM_MOUSEMOVE:
      {
         if(!g_reader)
            break;
         prevX = LOWORD(lParam);
         prevY = HIWORD(lParam);
         if(panning)
         {
            mouse.x = prevX;
            mouse.y = prevY;
         }
         UpdateStatusBarText(hwndStatus,prevX,prevY);
      }
      break;
   case WM_RBUTTONUP:
      {
         if(!g_reader)
            break;
         panning = false;
         PanBy(&mouseOld,&mouse);
         InvalidateRect(hWnd,&g_rect,false);
      }
      break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// Parse the menu selections:
		switch (wmId)
		{
      case IDM_OPEN: 
         if(!FileOpen(hWnd))
         {
            MessageBox(hWnd,szFileName,"Failed to open image:", MB_OK);
         }
         break;
		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:   
		hdc = BeginPaint(hWnd, &ps);   
	   if (!g_hbmp)   
		   break;

      hdcBuffer = CreateCompatibleDC(hdc);
      SelectObject(hdcBuffer, g_hbmp);
      BitBlt(hdc,0,0,g_rect.right,g_rect.bottom,hdcBuffer,0,0,SRCCOPY);     

      DeleteDC(hdcBuffer);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
      if(g_reader != NULL)
         g_reader->release();
      g_reader = NULL;
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

// the common dlg File Open.
bool FileOpen(HWND hWnd)
{   
   OPENFILENAME ofn;
   TCHAR szFileFilter[] = TEXT("MrSID Files (*.sid)\0*.sid\0Jpeg2000 Files (*.jp2)\0*.jp2\0");

   ZeroMemory(&ofn, sizeof(ofn));

   ofn.lStructSize         = sizeof(ofn);
   ofn.hwndOwner           = hWnd;
   ofn.lpstrFilter         = szFileFilter;
   ofn.lpstrFile           = szFileName;
   ofn.nMaxFile            = MAX_PATH;
   ofn.Flags               = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
   ofn.lpstrDefExt         = TEXT("sid");
   ofn.lpstrInitialDir     = NULL;
   if( GetOpenFileName(&ofn) )
   {
      if( !OpenImage() )
         return false;
      
      SetWindowText(hWnd,szFileName);  
      RECT rect;
      GetClientRect(hWnd,&rect);
      InvalidateRect(hWnd,&rect,false);
   }
   return true;
}


// open the SID image and setup your pipeline.
bool OpenImage()
{   	
   LTIScene scene;
   if(g_reader != NULL)
      g_reader->release();
   g_reader = NULL;

   LT_STATUS sts = LT_STS_Uninit;
   LTFileSpec fs(szFileName);

   if( 0== strcmp("sid", fs.getSuffix()))
   {
      lt_uint8 generation;
      bool raster;
      if(LT_FAILURE(MrSIDImageReader::getMrSIDGeneration(fs, generation, raster)))
         goto CLEANUP;

      if(!raster)
      {
         MessageBox(g_hwnd, "this is a LiDAR file - you need to use the LiDAR SDK to work with it", szFileName, MB_OK);
         goto CLEANUP;
      }

      MrSIDImageReader *sidReader = MrSIDImageReader::create();

      if(!sidReader)
         goto CLEANUP;

      if(LT_FAILURE(sidReader->initialize(fs)))
      {
         sidReader->release();
         sidReader = NULL;
         goto CLEANUP;
      }

      g_reader = sidReader;
   }
   else if( 0== strcmp("jp2", fs.getSuffix()))
   {
      J2KImageReader *j2kReader = J2KImageReader::create();

      if(!j2kReader)
         goto CLEANUP;

      if(LT_FAILURE(j2kReader->initialize(fs)))
      {
         j2kReader->release();
         j2kReader = NULL;
         goto CLEANUP;
      }

      if(LT_FAILURE(j2kReader->setStripHeight(64)))
      {
         j2kReader->release();
         j2kReader = NULL;
         goto CLEANUP;
      }

      g_reader = j2kReader;
   }   
   else
   {
      assert( !"This file extension is not supported.  Only MrSID & Jpeg2000 files are supported." );
      return false;
   }
     
   // It's possible that the minimum magnification on some images
   // isn't small enough to fit into our client area.
   // for this, we use LTIMultiResFilter to force a smaller magnification
   {       
      LTIMultiResFilter *multiResFilter = LTIMultiResFilter::create();

      if(!multiResFilter)
         goto CLEANUP;

      if(LT_FAILURE(multiResFilter->initialize(g_reader)))
      {
         multiResFilter->release();
         multiResFilter = NULL;
         goto CLEANUP;
      }
      g_reader->release();
      g_reader = multiResFilter;
   }

   // add a viewer filter to the pipeline.  This will do some handy conversions to our images
   // to ensure that we can display them.  (ie. swap the blue and red bands -- RGB to BGR)
   {  
      LTIViewerImageFilter *viewerFilter = LTIViewerImageFilter::create();

      if(!viewerFilter)
         goto CLEANUP;

      if(LT_FAILURE(viewerFilter->initialize(g_reader, true, true)))
      {
         viewerFilter->release();
         viewerFilter = NULL;
         goto CLEANUP;
      }
      g_reader->release();
      g_reader = viewerFilter;
   }

   // create a scene and a navigator.  Using navigator,
   // find the best fit scene for our sid image.  
   g_navv = new LTINavigator(*g_reader);	
   sts = g_navv->bestFit( g_rect.right, g_rect.bottom, scene );
   assert(LT_SUCCESS(sts) && "error getting best fit");
   
   g_mag = scene.getMag();
   sts = g_reader->getDimsAtMag( g_mag, g_imgWidth, g_imgHeight );
   assert(LT_SUCCESS(sts) && "error getting dims at mag");

   g_imgCntrX = (double)g_reader->getWidth()  /2;
   g_imgCntrY = (double)g_reader->getHeight() /2;

   //center the image in the client window
   g_userX = (lt_int32)g_imgCntrX;
   g_userY = (lt_int32)g_imgCntrY;

   UpdateView();
   return true;

CLEANUP:
   if(g_reader != NULL)
      g_reader->release();
   g_reader = NULL;

   return false;
}


// Here we will setup our new scene and export that scene to BIP
// format and forward that into a DIB.  Our magnification is 
// stored in g_mag.
void UpdateView()
{   
   LT_STATUS sts = LT_STS_Uninit;   
   if(!g_reader)
      return;

   if(g_hbmp)
      DeleteObject(g_hbmp);
   g_hbmp = NULL;

   // setup our DIB
   unsigned char* bufAddr = CreateDIB(g_reader->getNumBands());   
	assert(bufAddr && g_hbmp && "error creating buffer"); 
   
   LTINavigator navv(*g_reader);
   sts = navv.setSceneAsCWH(g_userX*g_mag, g_userY*g_mag, g_rect.right, g_rect.bottom, g_mag);
   assert(LT_SUCCESS(sts) && "error setting scene");

   LTIScene unclippedScene = navv.getScene();
   
   bool clipOK = navv.clipToImage();
   if (!clipOK)
   {
      // the user did an invalid pan, e.g. beyond edge of image, so
      // restore to last-known-good state

      g_userX = g_userXOld;
      g_userY = g_userYOld;
      g_mag = g_magOld;

      sts = navv.setSceneAsCWH(g_userX*g_mag, g_userY*g_mag, g_rect.right, g_rect.bottom, g_mag);
      assert(LT_SUCCESS(sts) && "error setting scene");

      unclippedScene = navv.getScene();
   
      // clip as needed
      clipOK = navv.clipToImage();
      assert(clipOK && "Error clipping image");
   }

   const LTIScene clippedScene = navv.getScene();

   const lt_int32 offX = clippedScene.getUpperLeftCol() - unclippedScene.getUpperLeftCol();
   const lt_int32 offY = clippedScene.getUpperLeftRow() - unclippedScene.getUpperLeftRow();

   // the user scene will always fit within the bounds of our viewport
   {
      assert(clippedScene.getNumCols() <= g_rect.right);
      assert(clippedScene.getNumRows() <= g_rect.bottom);
   }

   LTISceneBuffer bufferData(g_reader->getPixelProps(),
                            clippedScene.getNumCols(),
                            clippedScene.getNumRows(),
                            NULL);

   // decode the image
   sts = g_reader->read(clippedScene, bufferData);
   if(LT_FAILURE(sts))
   {
      char str[1024] = {0};
      sprintf( str, "Error readung image [%d]", sts );
      MessageBox( g_hwnd, str, "error reading image", MB_OK );
      return;
   }

   const lt_uint32 pixelBytes = g_reader->getPixelProps().getNumBytes();
   const lt_uint32 rowBytesBase = g_rect.right * pixelBytes;
   const lt_uint32 rowBytesAligned = LTISceneBuffer::addAlignment(rowBytesBase, 4);

   // export the data to BIP format (writing the pixels to bufAddr -- our DIB)
   sts = bufferData.exportData(bufAddr + (offY*rowBytesAligned) + (offX*pixelBytes),
                               pixelBytes,
                               rowBytesAligned,
                               1);
   assert(LT_SUCCESS(sts) && "Error exporting data");

   g_userXOld = g_userX;
   g_userYOld = g_userY;
   g_magOld = g_mag;

   // the UL of the rendered scene, in GDI coordinates
   g_scenePosX = unclippedScene.getUpperLeftX();
   g_scenePosY = unclippedScene.getUpperLeftY();

   return;
}

// create a DIB section here.
unsigned char* CreateDIB(int numBands)
{
   unsigned char* bufAddr = NULL;

	if (numBands == 3)
	{
		//	color image - we set the fields of the m_colorBmpInfo
		//	which describes our bitmap
		g_rgbBmpInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		g_rgbBmpInfo->bmiHeader.biWidth = g_rect.right;
		g_rgbBmpInfo->bmiHeader.biHeight = -g_rect.bottom;	//	 negative indicates a top-down image
		g_rgbBmpInfo->bmiHeader.biPlanes = 1;
		g_rgbBmpInfo->bmiHeader.biBitCount = 24;
		g_rgbBmpInfo->bmiHeader.biCompression = BI_RGB;
		g_rgbBmpInfo->bmiHeader.biSizeImage = 0;
		g_rgbBmpInfo->bmiHeader.biXPelsPerMeter = 0;
		g_rgbBmpInfo->bmiHeader.biYPelsPerMeter = 0;
		g_rgbBmpInfo->bmiHeader.biClrUsed = 0;
		g_rgbBmpInfo->bmiHeader.biClrImportant = 0;

		//	create the bitmap - this allocates memory for the pixels, etc
		g_hbmp = CreateDIBSection(	NULL,
									      g_rgbBmpInfo,
									      DIB_RGB_COLORS,
									      (void**)(&bufAddr),
									      0,
									      0L );
	}
	else if (numBands == 1)
	{
		//	grayscale image - we set the fields of m_gsBmpInfo
		//	which describes our bitmap
		g_grayBmpInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		g_grayBmpInfo->bmiHeader.biWidth = g_rect.right;
		g_grayBmpInfo->bmiHeader.biHeight = -g_rect.bottom;	//	 negative indicates a top-down image
		g_grayBmpInfo->bmiHeader.biPlanes = 1;
		g_grayBmpInfo->bmiHeader.biBitCount = 8;
		g_grayBmpInfo->bmiHeader.biCompression = BI_RGB;
		g_grayBmpInfo->bmiHeader.biSizeImage = 0;
		g_grayBmpInfo->bmiHeader.biXPelsPerMeter = 0;
		g_grayBmpInfo->bmiHeader.biYPelsPerMeter = 0;
		g_grayBmpInfo->bmiHeader.biClrUsed = 0;
		g_grayBmpInfo->bmiHeader.biClrImportant = 0;

		//	create the bitmap - this allocates memory for the pixels, etc
		g_hbmp = CreateDIBSection(	NULL,
									 g_grayBmpInfo,
									 DIB_RGB_COLORS,
									 (void**)(&bufAddr),
									 0,
									 0L );
	}
   else
   {
      assert(!"Image is not RGB or Grayscale!");
      return NULL;
   }
   return bufAddr;
}

void PanBy(POINT* mouseOld,POINT* mouse)
{
   int difX = mouse->x - mouseOld->x;
   int difY = mouse->y - mouseOld->y;
   
   g_userX -= (lt_int32)(difX / g_mag);
   g_userY -= (lt_int32)(difY / g_mag);

   UpdateView();
}

void ZoomIn()
{   
   if (g_mag * 2.0 > g_reader->getMaxMagnification())
      return;

   g_mag *=2;

   UpdateView();
}

void ZoomOut()
{
   if (g_mag / 2.0 < g_reader->getMinMagnification())
      return;

   g_mag /= 2;
   UpdateView();
}

void UpdateStatusBarText(HWND hwndStatus, LONG x, LONG y)
{
   
   static char buf[128];

   // position in pixel space
   const double px = (g_scenePosX + x) / g_mag;
   const double py = (g_scenePosY + y) / g_mag;

   // position in geo space
   double gx=0.0, gy=0.0;
   g_reader->getGeoCoord().pixelToGeo(px, py, 1.0, gx, gy);

   sprintf(buf, "  i(%d,%d)   g(%g,%g)   mag=%g", (int)px, (int)py, gx, gy, g_mag);

   // send the text to the status bar
   SendMessage(hwndStatus, WM_SETTEXT, (WPARAM)NULL, (LPARAM)buf);
}
