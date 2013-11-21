#include <windows.h>
#include <string.h> 
#include <math.h>
#include <stdlib.h>
void main(void);

void main(void)
{
    static char szText  [] = "Hello Printer!" ;
    DOCINFO DocInfo;
	RECT	rcRect;
	HDC		hDC;
	int i; 
     short       xPage, yPage ;
	POINT point;
	char szTextStr[32];
     static char szPrinter [80] ;
     char        *szDevice, *szDriver, *szOutput ; 
                  GetProfileString ("windows", "device", ",,,", szPrinter, 80) ;

                 if (NULL != (szDevice = strtok (szPrinter, "," )) &&
                     NULL != (szDriver = strtok (NULL,      ", ")) &&
                     NULL != (szOutput = strtok (NULL,      ", ")))
                 {
                       hDC = CreateDC (szDriver, szDevice, szOutput, NULL) ;
                       if(GetDeviceCaps(hDC,RASTERCAPS) & RC_BANDING == 0)
                       { //Gets here if device doesn't support banding
                          i = 1;    
                       }
                       else
                       {// device supports banding
                          i = -1;
                          DocInfo.cbSize = sizeof(DOCINFO);
                          DocInfo.lpszDocName = "Test";
                          DocInfo.lpszOutput = (LPSTR) NULL;
                          StartDoc(hDC,&DocInfo);
                          xPage = GetDeviceCaps (hDC, HORZRES) ;
                          yPage = GetDeviceCaps (hDC, VERTRES) ;
                          for(;;)
                          {
                            Escape(hDC, NEXTBAND, 0, (LPSTR)NULL, &rcRect);
                            if(IsRectEmpty(&rcRect)) break;
                            DPtoLP(hDC, (POINT FAR*) &rcRect, 2);
                            Rectangle (hDC, rcRect.left,  rcRect.top,
                                            rcRect.right, rcRect.bottom) ;
                            SaveDC (hDC) ;
                            SetMapMode (hDC, MM_ISOTROPIC) ;
                            SetWindowExt   (hDC, 1000, 1000) ;
                            SetViewportExt (hDC, xPage / 2, -yPage / 2) ;
                            SetViewportOrg (hDC, xPage / 2,  yPage / 2) ;
                            Ellipse (hDC, -500, 200, 500, -200) ;
                            SetTextAlign (hDC, TA_BASELINE | TA_CENTER) ;
                            TextOut (hDC, 0, 0, szText, sizeof szText - 1) ;
                            RestoreDC (hDC, -1) ;
                          } 
                       }
                       EndDoc(hDC);
                       DeleteDC(hDC);
                 }      
}