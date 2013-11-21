#include <math.h>
#include "shr.h"

static  Init=FALSE;
     
BOOL ConvertCoordInit (void)
{    long ok, ID; 
     char str[64];
     char Name[20];   
     
     if (Init) return TRUE;
     if(!LoadGCTPLibrary32( ))
     {
       MessageBox(0,"Unable to Open GCTP Library",
         "Coordinate Conversion Error",MB_ICONSTOP);
         return FALSE;
     
     }  
     if(!OpenGCTP32(&ok))
     {
       MessageBox(0,"Unable to Open GCTP",
         "Coordinate Conversion Error",MB_ICONSTOP);
         return FALSE;
     
     }       
      
     _fstrcpy(Name, "hennco");
     ID = 1;
     LoadProjection(ID,Name);
     _fstrcpy(Name, "utm15");
     ID = 2;
     LoadProjection(ID,Name);
     _fstrcpy(Name,"latlongs");
     ID = 3;
     LoadProjection(ID,Name); 
     Init=TRUE;
    return TRUE;
} 
     
BOOL ConvertCoord (LPDPOINT DPoint, int from, int to)
{
TranP TP;
lpTranP lpTP; 
     
     extern HWND    hWndMain;
     
     if (!ConvertCoordInit()) return FALSE;
     lpTP = &TP;
     TranProjection((long)from, (long)to, &DPoint->x, &DPoint->y);
     return TRUE;
}

BOOL ConvertCoordClose (void)
{   
    if (Init)
        CloseGCTPLibrary32();
    Init=FALSE;
    return TRUE;
}  
   
   

