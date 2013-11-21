
BOOL OffsetArea (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{
#undef lpldatablehome
//#undef lpPolyHome 

#include "offsetmn.h"
//#include "polycom.h" 

 HDC hDC;
 char key;
 POINT	MousePoint, TestPoint;
 DPOINT	BasePoint;
 BOOL	Cancel;
 LPTHEME	pTheme;
 int	np, *npt, i, st;
 long i4; 
 short size;
 LPDPOINT	lpDpoint, lpDNext;
 LPPOINT	lpPoint, lpPoly;
 int     NumCrvPts;
 char	cCor[32];
 HANDLE	hPoly; 
 LPSTR	lpColon;
 HPEN	hPen, OldPen;
  lpLArea OutArea;
  Area A;
  lpArea lpA = (lpArea) &A;
  lpDLine  lpLda;
  HGLOBAL heap_ptr; 
  lpA->type = 1; //indicates a standard area offset is requested
  lpA->whos_callen = 1;
  lpA->offset_dist = -50;
  lpA->LinkDesc = 351;
  lpA->fillet_desc = 1321;

 switch (Message)
   {
   	case GF_INIT:
   		break;

    case WM_LBUTTONUP:
    	MousePoint.x = LOWORD(lParam);
	    MousePoint.y = HIWORD(lParam);
	    if (!CurView->TranWinToBase) break;
        BasePoint=WinPtToBasePt(MousePoint);
        PickItems (hWnd,BasePoint);
        if (NumPicked > 0)
        {   
        	if (PickList[NumPicked-1].Type == 3)
        	{  lpPoint = lpPoints2;
	    		pTheme = AddTheme (GF_SAVEPOLY_THEME); 
	    		CurView->PassID = 1;
				ProcessPickedItem (NumPicked-1);        		
	    		DeleteTheme (pTheme);
	    		if (hSavePoly)
	    		{
		    		WaitCursor (1);
                    npt = (int *)GlobalLock (hSavePoly);
                    nPnts = *npt++;
                    lpDpoint = (LPDPOINT) npt;
                    lpA->NumPts = nPnts ;
   
          size = sizeof(DLine)*(lpA->NumPts); 
          heap_ptr = GlobalAlloc( GPTR, size); 
          if(heap_ptr == NULL)
          {   
             MessageBox(NULL,"Offset Out of Memory", "Offset Main Error",MB_ICONSTOP);
             return TRUE;
          }   
          lpA->lpDLineBase = (lpDLine) GlobalLock(heap_ptr);
          lpLda = lpA->lpDLineBase; 
          lpDNext = lpDpoint + 1;
          for(i=0; i < lpA->NumPts; i++,lpDpoint++,lpDNext++)
          { 
            lpLda = lpA->lpDLineBase + i;
            lpLda->F.x = lpDpoint->x;
            lpLda->F.y = lpDpoint->y;
            if(i < lpA->NumPts - 1)
            { 
              lpLda->T.x = lpDNext->x;
              lpLda->T.y = lpDNext->y;
            }  
            lpLda->Refn = i;
            lpLda->Desc = 21;
            lpLda->Type = 2;           
            lpLda->ID = 123+i;
          }
          lpDpoint = (LPDPOINT) npt;
          lpLda->T.x = lpDpoint->x;
          lpLda->T.y = lpDpoint->y;
          lpA->NumSides = lpA->NumPts;
          OFFSET_MAIN(lpA, &OutArea, &i4);          
          if(i4 != 0)
          {
             MessageBox(NULL,"Sorry... Unable to offset this item",
                       "Offset Error",MB_ICONINFORMATION);
          
          }
          GlobalUnlock(heap_ptr); 
          GlobalFree(heap_ptr);                           
		  GlobalUnlock(hSavePoly);
		  GlobalFree(hSavePoly);
		  hSavePoly = NULL;
		  if(i4 != 0)
		  {
                OffsetClose();
		     	WaitCursor (-1);
		        return TRUE;
		  }
		  BasePoint.x = 0e0;
		  lpLineBase = OutArea->Lines;
		  for (i=1,OutArea->Lines++; i <= OutArea->NumSides; i++,OutArea->Lines++)
		  {
		     if(OutArea->Lines->type == 3)
		     {
		       BasePoint.x = BasePoint.x + fabs(OutArea->Lines->lngth);
		     }
		  }
		  if(BasePoint.x > 0e0)//we found some curved lines
		  { //gotta figure out how many intermediate points these curves
		    //will be broken into. BasePoint.x has the length of the curves in feet.
		    //First I need to convert the length from Base coordinates to Window
		    //CurView->BaseUnitsPerPixel holds how many feet it takes per pixel
		    //I want a point approximately every 50 pixels
		    NumCrvPts = BasePoint.x / (CurView->BaseUnitsPerPixel * 50.0);
		    //POINT BasePtToWinPt (DPOINT WPoint)
            SetCurvPltCtol (CurView->BaseUnitsPerPixel * 50.0);
            //DPOINT WinPtToBasePt (POINT Point)
		    //Second I need to figure out how many pixels I want to include in each
		    //line segment.  Make a call to       void SetCurvPltCtol (double INCTOL);
            //so it's dividing the line properly
		  
		  }	
          size = sizeof(LPPOINT)*(OutArea->NumSides + NumCrvPts + 10);
          heap_ptr = NULL; 
          heap_ptr = GlobalAlloc( GPTR, size); 
          if(heap_ptr == NULL)
          {   
             MessageBox(NULL,"Offset Out of Memory", "Offset Main Error",MB_ICONSTOP);
             return TRUE;
          }   
          lpPoint = (LPPOINT) GlobalLock(heap_ptr);
          if(lpPoint == NULL)
          {   
             MessageBox(NULL,"Offset Out of Memory", "Offset Main Error",MB_ICONSTOP);
             return TRUE;
          }
          lpPoly = lpPoint;
               OutArea->Lines = lpLineBase + 1;   
               for (np = 0,i=1; i <= OutArea->NumSides; i++,OutArea->Lines++, lpPoint++)
               { //put the first point of the line in 
                    if(OutArea->Lines->type == 2)
                    {
			          BasePoint.x = OutArea->Lines->x1;
				   	  BasePoint.y = OutArea->Lines->y1;
					  *lpPoint = BasePtToWinPt(BasePoint);
					 // lpPoint->y = lpPoint->y - CurView->DrawRect.top;
					  if(i == 1)TestPoint = *lpPoint; 
			      /*    BasePoint.x = OutArea->Lines->x2;
				   	  BasePoint.y = OutArea->Lines->y2;
					  TestPoint = BasePtToWinPt(BasePoint); */
					  np++;
					}  
                    if(OutArea->Lines->type == 3)
                    {
			       /*   BasePoint.x = OutArea->Lines->x1;
				   	  BasePoint.y = OutArea->Lines->y1;
					  TestPoint = BasePtToWinPt(BasePoint); 
			          BasePoint.x = OutArea->Lines->x2;
				   	  BasePoint.y = OutArea->Lines->y2;
					  TestPoint = BasePtToWinPt(BasePoint); 
			          BasePoint.x = OutArea->Lines->rx;
				   	  BasePoint.y = OutArea->Lines->ry;
					  TestPoint = BasePtToWinPt(BasePoint);*/ 
                         st =  CURVPLT(OutArea->Lines, &np,  &lpPoint);
                    }
               }
               *lpPoint = TestPoint;
               lpPoint = lpPoly;
               np++;
            /*   for(i = 0;i <= np;i++,lpPoint++)
               {
                  st = 0;
               }   */ 
					SetDisplayMode (CurView->hDC, GF_TEXTMODE);
					SelectClipRgn (CurView->hDC,NULL);
               		hPen = CreatePen (PS_SOLID,6,RGB(255,0,0));
                    OldPen = SelectObject (CurView->hDC,hPen);
                    i = Polyline (CurView->hDC,lpPoly,np);
                    GlobalUnlock(heap_ptr); 
                    GlobalFree(heap_ptr);                           
                    SelectObject (CurView->hDC,OldPen);
                    DeleteObject (hPen); 
                    //GlobalUnlock (hPoly);
                    //GlobalFree (hPoly); 
                    //here I free up the memory used by the OutArea
                    OffsetClose();
		    		WaitCursor (-1);
	    		}
        	}
        }
		break;