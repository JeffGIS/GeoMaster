     int CURVPLT(lpLine L1,int *np, POINT **lpPoint)
{
//C******* SPECIFICATIONS **********************************************
//C*                                                                   *
//C*       PROGRAM SUMMARY                                             *
//C*       ------- -------                                             *
//C*       THIS ROUTINE PLOTS A CURVE OF LENGTH (L) WITH BEGINNING     *
//C*       POINT OF (X1,Y1) AND RADIUS POINT OF (X2,Y2).               *
//          
//         Arguments
//  input  lpLine L1 A structure containing information about a line
//  input  HGLOBAL *heap_ptr is a pointer to an area of memory supplied
//                   by the calling program, used to contain a series
//                   of coordinate set structures;
//  function return value: 0 if it added points to *heap_ptr
//                         -1 if it didn't do anything.

      double  DTH, A1, LenRad, LenCrv;
      DPOINT TestPoint, HoldPt;
      POINT  StartPoint, EndPoint, RadPt; 
      short size, Npts;
      POINT MyPt;
    //  XB=L1->x1; //X1;
    //  YB=L1->y1; //Y1;
//C******* COMPUTE RADIUS
     if( L1->rad <= 0) return -1;  
//C******* CALCULATE AZIMUTH OF BEGINNING & ENDING ARC RADII 
	 	    TestPoint.x = (CurView->WBounds.xmn + CurView->WBounds.xmx)/2.0;
	 	    HoldPt.x    = TestPoint.x;
		    TestPoint.y = (CurView->WBounds.ymn + CurView->WBounds.xmx)/2.0;
            StartPoint  = BasePtToWinPt (TestPoint);		    
            TestPoint.x = HoldPt.x + L1->rad;
		    EndPoint    = BasePtToWinPt(TestPoint);
		    LenRad      = EndPoint.x - StartPoint.x;  
            TestPoint.x = HoldPt.x + L1->lngth;
		    EndPoint    = BasePtToWinPt(TestPoint);
		    LenCrv      = EndPoint.x - StartPoint.x;  
            TestPoint.x = L1->rx;
            TestPoint.y = L1->ry;
            RadPt       = BasePtToWinPt(TestPoint);
     // C= L1->x1 - L1->rx; //C=X1-X2
     // S = L1->y1 - L1->ry;//S=Y1-Y2
     // R = sqrt(C*C+S*S);
     // if (R == 0) return -1;
//C******* CALCULATE AZIMUTH OF BEGINNING & ENDING ARC RADII
      A1 = L1->azm; //atan2(S,C);
             
      //A2 = L1->eazm;//A1 - L1->lngth / R;
      if(LenRad <= CTOL) goto S110;
      DTH = acos ((LenRad - CTOL)/LenRad);
       if(DTH == 0) goto S110;
//C******* CALCULATE NUMBER OF VECTORS IN CURVE
      Npts = (short)(fabs(LenCrv) / (DTH * LenRad));
      Npts++; // to plot at least the beginning of the curve
      if(LenCrv < 0)DTH = -DTH;
      if(Npts < 2)goto S110;
//C******* PLOT REMAINING VECTORS
      for (lpPoly1->l = 2;lpPoly1->l <= Npts;lpPoly1->l++)
      {
         MyPt.x = LenRad * cos(A1) + RadPt.x;
         MyPt.y = RadPt.y - (LenRad * sin(A1));
		// MyPt.y = MyPt.y - CurView->DrawRect.top;
         **lpPoint = MyPt;
         *lpPoint = *lpPoint + 1;
         *np = *np + 1;
         A1 -=  DTH;
      }
//C******* FINISH CURVE
 S110:  *lpPoint = *lpPoint - 1;

      return 0;
}
      void SetCurvPltCtol (double INCTOL)
   {
      CTOL = INCTOL;
      return;
    }
