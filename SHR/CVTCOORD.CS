#include <math.h>
#include "shr.h"

static  Init=FALSE;
     
BOOL ConvertCoordInit (void)
{    long ok, ID; 
     char str[64];
     char Name[64];   
     
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
      
     _fstrcpy(Name, "baseproj");
     ID = 1;
     if (LoadProjection(ID,Name)!=0) return FALSE;
     _fstrcpy(Name,"latlongs");
     ID = 2;
     if (LoadProjection(ID,Name)!=0) return FALSE;
     ID = 3; 
     
     _fstrcpy(Name,"[%ALT_PROJECTION]");
     ExpandText (Name);
     if (Name[0])
     	if (LoadProjection(ID,Name)!=0) return FALSE;
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
     if (TranProjection((long)from, (long)to, &DPoint->x, &DPoint->y))
     	return FALSE;
     else
     	return TRUE;
}

BOOL ConvertCoordClose (void)
{   
    if (Init)
        CloseGCTPLibrary32();
    Init=FALSE;
    return TRUE;
}  
   
   

