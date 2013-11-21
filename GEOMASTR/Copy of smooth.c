#include <graphint.h>
typedef struct {
     double xo, yo, xm, ym, dm, fazm, tazm, azdf, moved,
            fdist, tdist,  WtAvgDiff, AvgDiff, 
            AvgDeltaAzm, WtAvgDeltaAzm,
            SumNearAzm, AbsolNearAzm;
     unsigned int PtNum, AzmSummed, TimesAdjusted;
     BOOL OkToMove;
     } PolyPt;
typedef PolyPt *lpPolyPt;  
void SmoothPolyLine(HGLOBAL HPP);
void PrintResults(lpPolyPt lpPP);
int NextWorstPoint(HGLOBAL HPP);
BOOL MoveThisOne(lpPolyPt lpPP);

static USHORT GlobNumPts;
static double GlobMaxDev;
static FILE *MyFile;
static char string[128], value[26];
static int MAXITERATIONS; 
static BOOL debug;

//***********************************************************
void TestSmooth(void)
{ int i;
HGLOBAL h;
LPDPOINT lpdp;
  h = GlobalAlloc(GHND,sizeof(DPOINT) * 12);
  lpdp = (LPDPOINT) GlobalLock(h);
  lpdp->x = 11.04;//int part is number of points. fraction part is iterations
  lpdp->y = 5.00; //max deviation from plotted point
  lpdp++;
  lpdp->x = 1000.0;  //1
  lpdp->y = 1000.0;   
  lpdp++;
  lpdp->x = 1000.0;  //2
  lpdp->y = 1100.0;
  lpdp++;
  lpdp->x = 1020.0;  //3
  lpdp->y = 1200.0;
  lpdp++;
  lpdp->x = 1050.0;  //4
  lpdp->y = 1300.0;
  lpdp++;
  lpdp->x = 1090.0;  //5
  lpdp->y = 1400.0;
  lpdp++;
  lpdp->x = 1110.0;  //6  this point is way out
  lpdp->y = 1420.0;
  lpdp++;
  lpdp->x = 1150.0;  //7
  lpdp->y = 1500.0;
  lpdp++;
  lpdp->x = 1220.0;  //8
  lpdp->y = 1600.0;
  lpdp++;
  lpdp->x = 1300.0;  //9  starts to break more right
  lpdp->y = 1700.0;
  lpdp++;
  lpdp->x = 1400.0;  //10
  lpdp->y = 1800.0;
  lpdp++;
  lpdp->x = 1450.0;  //11
  lpdp->y = 1820.0;
  
  GlobalUnlock(h);
  SmoothPolyLine(h);
  GlobalFree(h);
  return;
}  
  
//*************************************************  
void SmoothPolyLine(HGLOBAL HPP)
{ int decpt,sign, WtAvgDiv, NextWorst, LastWorst, Iterates;
 double LastX, LastY, MaxDev;
 unsigned int NumPts, i, j,NumLeft,Num2Read;
 LPDPOINT lpDP,lpDNP,lpDPFirst, lpDLP;
 lpPolyPt lpPP,lpPP2, lpPPFirst, lpPrior, lpNext;
 HGLOBAL HPOINTS;
 lpDPFirst = (LPDPOINT) GlobalLock(HPP);
 lpDP = lpDPFirst + 1;
 lpDNP = lpDP + 1;
 GlobNumPts = NumPts = (unsigned int) lpDPFirst->x;//how many points in this polyline 
 MAXITERATIONS = (lpDPFirst->x - GlobNumPts) * 100;
 GlobMaxDev = MaxDev = lpDPFirst->y;
 HPOINTS = GlobalAlloc(GHND,(long) sizeof(PolyPt)*NumPts);
 lpPP = (lpPolyPt) GlobalLock(HPOINTS);
 debug = FALSE;
 if(debug) MyFile = fopen("c:\\curves.txt","wt");
 lpPPFirst = lpPP;
 for(i = 0;i < NumPts;i++,lpPP++,lpDP++,lpDNP++)
 {
   LastX = lpPP->xo = lpPP->xm = lpDP->x;
   LastY = lpPP->yo = lpPP->ym = lpDP->y;
   if(i < NumPts - 1)
   {
     lpPP->tdist = LDIST(lpDP->x, lpDP->y,lpDNP->x,lpDNP->y);
     lpPP->tazm = LGETAZ(lpDP->x, lpDP->y,lpDNP->x,lpDNP->y);
   }  
   lpPP->PtNum = i;
   if(i > 0)
   { 
     lpDLP = lpDP - 1;
     lpPP->fdist = LDIST(lpDLP->x, lpDLP->y,lpDP->x,lpDP->y);
     lpPP->fazm = LGETAZ(lpDLP->x, lpDLP->y,lpDP->x,lpDP->y);
     lpPP->azdf = AZDF(LTWOPI(lpPP->fazm+PY),lpPP->tazm,(double) 1e0) - PY;
   }
 }
    Iterates = 0;
    LastWorst = -1;
Again: lpPP = lpPPFirst;
 for(NumLeft=NumPts,i = 0;i <  NumPts;i++,lpPP++,NumLeft--)
 {
   switch (i)
   {
     case 0:
       if(NumLeft > 3)
         Num2Read = 3;
       else
         Num2Read = NumLeft;
         lpPP2 = lpPP;
     break;
     case 1:
       if(NumLeft > 4)
         Num2Read = 4;
       else
         Num2Read = NumLeft + 1;
         lpPP2 = lpPP - 1;
     break;   
     default:
       if(NumLeft > 3)
         Num2Read = 5;
       else
         Num2Read = NumLeft + 2;
         lpPP2 = lpPP - 2;
   } //end of switch
       lpPP->AzmSummed = Num2Read;
       lpPP->SumNearAzm = 0;
       lpPP->AbsolNearAzm = 0;
       lpPP->AvgDeltaAzm = 0; 
       lpPP->WtAvgDeltaAzm = 0;
       WtAvgDiv = 0;
       for(j=0;j < Num2Read;j++,lpPP2++)
       {
         lpPP->SumNearAzm += lpPP2->azdf;
         lpPP->AbsolNearAzm += fabs(lpPP2->azdf);
         switch((long)lpPP->PtNum - (long)lpPP2->PtNum)
         {
           case -2: 
           case 2:
             WtAvgDiv++;
             lpPP->AvgDeltaAzm   += lpPP2->azdf;
             lpPP->WtAvgDeltaAzm += lpPP2->azdf;
           break;
           case -1:
           case 1:
             WtAvgDiv += 3;
             lpPP->AvgDeltaAzm   += lpPP2->azdf;
             lpPP->WtAvgDeltaAzm += lpPP2->azdf* 3;
           break;
           case 0:
             WtAvgDiv += 5;
             lpPP->AvgDeltaAzm   += lpPP2->azdf;
             lpPP->WtAvgDeltaAzm += lpPP2->azdf * 5;
           
         }//end of the switch
       }

    //  WtAvgDiff, AvgDiff, AvgDeltaAzm, WtAvgDeltaAzm,
       lpPP->WtAvgDeltaAzm = lpPP->WtAvgDeltaAzm / (double) WtAvgDiv;
       lpPP->WtAvgDiff =  lpPP->WtAvgDeltaAzm - lpPP->azdf ;
       lpPP->AvgDeltaAzm = lpPP->AvgDeltaAzm/(double)Num2Read;
       lpPP->AvgDiff = lpPP->AvgDeltaAzm - lpPP->azdf;
       lpPP->moved = LDIST(lpPP->xo,lpPP->yo,lpPP->xm,lpPP->ym);
       if(fabs(lpPP->moved) < GlobMaxDev)
          lpPP->OkToMove = TRUE;
       else
          lpPP->OkToMove = FALSE;
     // PrintResults(lpPP);
 }

      GlobalUnlock(HPOINTS);   
      
      NextWorst = NextWorstPoint(HPOINTS);
      if(NextWorst == 0)goto Ending;
      if(NextWorst == LastWorst)
      {
        if( Iterates++ > MAXITERATIONS)  goto Ending;
      }
      else
      {
        Iterates = 0;
        LastWorst = NextWorst;
      }
      lpPP = (lpPolyPt) GlobalLock(HPOINTS);
      lpPPFirst = lpPP;
      lpPP += NextWorst;
      lpPrior = lpPP - 1;
      lpNext = lpPP + 1;
      if(MoveThisOne(lpPP))
         goto Again;

Ending: if(debug)fclose(MyFile);
GlobalUnlock(HPP);
GlobalUnlock(HPOINTS);
lpDP = (LPDPOINT)GlobalLock(HPP);
lpDP++;
lpPP = (lpPolyPt)GlobalLock(HPOINTS);
for(i=0;i<GlobNumPts;i++,lpDP++,lpPP++)
{
  lpDP->x = lpPP->xm;
  lpDP->y = lpPP->ym;
}
GlobalUnlock(HPP);
GlobalUnlock(HPOINTS);
GlobalFree(HPOINTS);
return;
}
//*******************************************************                                   
BOOL MoveThisOne(lpPolyPt lpPP)
{ lpPolyPt lpNext,lpPrior;
  short n1, n2, icd, i;
  double HalfAzm, dx, dy, maxx, maxy, MaxAzm, Azm[3], NewAzm,x[3],y[3],
        dmn;
  DPOINT NewPoint, OldPoint, New2;
    if(!lpPP->OkToMove) return FALSE;
    //must not exceed the maxdev limitation
    HalfAzm = LTWOPI(lpPP->fazm + (lpPP->azdf/2.0) + HALFPI ) ;
    lpPP->moved = -1.00;
    if(lpPP->azdf > lpPP->WtAvgDiff)   
    {
        HalfAzm = LTWOPI(HalfAzm + PY); //gotta move point to the right
        lpPP->moved = 1.00;
    }    
    OldPoint.x = lpPP->xo;
    OldPoint.y = lpPP->yo;
    NewPoint = dnewpt(OldPoint,HalfAzm,GlobMaxDev);
    lpPrior = lpPP - 1;
    lpNext  = lpPP + 1;
    Azm[0] = LGETAZ(lpPrior->xm, lpPrior->ym,NewPoint.x, NewPoint.y);
    Azm[2] = AZDF( lpPrior->tazm,Azm[0],lpPP->moved);
    if(fabs(Azm[2]) > fabs(2.0*(lpPP->WtAvgDiff/3.0)))
    {//gotta go less than the max
       MaxAzm = 2.0*(lpPP->WtAvgDiff/3.0);//but move which direction?
       if(lpPP->moved < 0)// moving the point to the left
         NewAzm = LTWOPI(lpPrior->tazm + fabs(MaxAzm));
       else //moving the point to the right 
         NewAzm = LTWOPI(lpPrior->tazm - fabs(MaxAzm));
       //from the prior point I shoot a line out on this azimuth
       //and intersect the line used to define NewPoint.  The intersection
       //point becomes lpPP->xm and lpPP->ym.
       OldPoint.x = lpPrior->xm;
       OldPoint.y = lpPrior->ym;
       New2 = dnewpt(OldPoint,NewAzm,lpPrior->tdist+100.0);
       XLL(&lpPP->xm, &lpPP->ym, &NewPoint.x, &NewPoint.y,
           &lpPrior->xm, &lpPrior->ym, &New2.x, &New2.y,
           &x[0], &y[0], &x[1], &y[1], &x[2], &y[2],
           &n1, &n2, &dmn, &icd);
       if(icd == 3 || icd == 5 || icd == 6)//gotta an intersection
       {
          lpPP->xm = x[n1-1];
          lpPP->ym = y[n1-1];
       }
       else
       {
         MessageBox(NULL,"Got Trouble Smoothing","Smoother",
            MB_ICONSTOP);
            return FALSE;
       }    
    }
    else
    {//ok to move
     lpPP->xm = NewPoint.x;
     lpPP->ym = NewPoint.y;
    }                                       
    //now I adjust the dimensions
     lpPP->moved   *= LDIST(lpPP->xm, lpPP->ym,lpPP->xo,lpPP->yo);
   if(debug)
   {
     i = sprintf(string,"moving Pt   %d\t%f\n",lpPP->PtNum,lpPP->moved);
     fputs(string,MyFile);
     i = sprintf(string,"From X %f  From Y %f\n",lpPP->xo,lpPP->yo);
     fputs(string,MyFile);
     i = sprintf(string,"To X %f  To Y %f\n",lpPP->xm,lpPP->ym);
     fputs(string,MyFile);
     fputs(" \n",MyFile);
   }  
     lpPP->TimesAdjusted++; 
     lpPP->fdist    = LDIST(lpPrior->xm, lpPrior->ym,lpPP->xm,lpPP->ym);
     lpPrior->tdist = lpPP->fdist;
     lpNext->fdist  = LDIST(lpNext->xm, lpNext->ym,lpPP->xm,lpPP->ym);
     lpPP->tdist    = lpNext->fdist;
     lpPP->fazm     = LGETAZ(lpPrior->xm, lpPrior->ym,lpPP->xm,lpPP->ym);
     lpPrior->tazm  = lpPP->fazm;
     lpPP->tazm     = LGETAZ(lpPP->xm, lpPP->ym,lpNext->xm,lpNext->ym);
     lpNext->fazm   = lpPP->tazm;
     lpPP->azdf     = AZDF(LTWOPI(lpPP->fazm+PY),lpPP->tazm,(double) 1e0) - PY;
     lpPrior->azdf  = AZDF(LTWOPI(lpPrior->fazm+PY),lpPrior->tazm,(double) 1e0) - PY;
     lpNext->azdf   = AZDF(LTWOPI(lpNext->fazm+PY),lpNext->tazm,(double) 1e0) - PY;
  return TRUE; 
}
//***************************************************
int NextWorstPoint(HGLOBAL HPP)
{ int i, WorstOne = 0;
  double DWorst;
  lpPolyPt lpPP;
  lpPP = (lpPolyPt) GlobalLock(HPP);
  lpPP += 1; //skip the first pt cuz its not relevate
  
  for (DWorst = 0.0, i = 1;i < GlobNumPts-1; i++, lpPP++)
  {
      if(fabs(lpPP->WtAvgDeltaAzm) > fabs(DWorst) && lpPP->OkToMove &&
        lpPP->TimesAdjusted <= MAXITERATIONS)    
      {
        WorstOne = lpPP->PtNum;
        DWorst = lpPP->WtAvgDeltaAzm;
      }  
  } 
  GlobalUnlock(HPP);      
  return WorstOne;
}


            
void PrintResults(lpPolyPt lpPP)
{
   if(!debug)return;
      fputs(" \n",MyFile);
      sprintf(string,"Point # %d\n",lpPP->PtNum);
      fputs(string,MyFile);

      sprintf(string,"X = %f\tY = %f\n", lpPP->xo,lpPP->yo);
      fputs(string,MyFile);

      sprintf(string,"Deflection = %f\n",lpPP->azdf);
      fputs(string,MyFile);

      sprintf(string,"Average Deflection = ",lpPP->AvgDeltaAzm);
      fputs(string,MyFile);
      
      sprintf(string,"Weighted Average Deflection = %f\n",lpPP->WtAvgDeltaAzm);
      fputs(string,MyFile);
      
      sprintf(string,"AvgDiff - AZDF = %f\n",lpPP->AvgDiff );
      fputs(string,MyFile);
 
       sprintf(string,"WtAvgDiff - AZDF  = %f\n",lpPP->WtAvgDiff );
      fputs(string,MyFile);
      
     return;
}        
