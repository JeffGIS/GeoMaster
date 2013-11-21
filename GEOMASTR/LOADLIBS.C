#include <windows.h>  
#include <string.h>
#include "resource.h"

#define MaxNumLibs 5  
static  short i, OK;
static  char szString[128]; 
                  
static HINSTANCE  hi, HInstances[MaxNumLibs]={NULL,NULL,NULL,NULL,NULL}; 
      
BOOL LoadLibraries(HINSTANCE hInst)
{
  char MYLibs[MaxNumLibs][36];

     OK = 0;
     _fstrcpy((LPSTR)MYLibs, (LPSTR)"WINSOCK.DLL" );
     _fstrcpy((LPSTR)&MYLibs[1],(LPSTR)"COMMDLG.DLL");
     _fstrcpy((LPSTR)&MYLibs[2],(LPSTR)"ODBC.DLL");
     _fstrcpy((LPSTR)&MYLibs[3],(LPSTR)"CTL3DV2.DLL");
     _fstrcpy((LPSTR)&MYLibs[4],(LPSTR)"GCTP32.DLL");

     for (i=0;i < MaxNumLibs;i++)
     {  
       HInstances[i] = LoadLibrary((LPCSTR) &MYLibs[i]);
       if(HInstances[i] < HINSTANCE_ERROR)
       { 
         OK = -1; 
         hi = HInstances[i];       
         switch (hi)
         {
          case 0:
              LoadString(hInst, IDS_LOADLIB0, szString, sizeof(szString));
            break;
          case 2:
              LoadString(hInst, IDS_LOADLIB2, szString, sizeof(szString));
            break; 
          case 3:
              LoadString(hInst, IDS_LOADLIB3, szString, sizeof(szString));
            break; 
          case 5:
              LoadString(hInst, IDS_LOADLIB5, szString, sizeof(szString));
            break;
          case 6:
              LoadString(hInst, IDS_LOADLIB6, szString, sizeof(szString));
            break; 
          case 8:
              LoadString(hInst, IDS_LOADLIB8, szString, sizeof(szString));
            break; 
           case 10:
              LoadString(hInst, IDS_LOADLIB10, szString, sizeof(szString));
            break;
          case 12:
              LoadString(hInst, IDS_LOADLIB12, szString, sizeof(szString));
            break; 
          case 13:
              LoadString(hInst, IDS_LOADLIB13, szString, sizeof(szString));
            break; 
           case 14:
              LoadString(hInst, IDS_LOADLIB14, szString, sizeof(szString));
            break;
          case 15:
              LoadString(hInst, IDS_LOADLIB15, szString, sizeof(szString));
            break; 
          case 16:
              LoadString(hInst, IDS_LOADLIB16, szString, sizeof(szString));
            break; 
           case 19:
              LoadString(hInst, IDS_LOADLIB19, szString, sizeof(szString));
            break;
          case 20:
              LoadString(hInst, IDS_LOADLIB20, szString, sizeof(szString));
            break; 
          case 21:
              LoadString(hInst, IDS_LOADLIB21, szString, sizeof(szString));
            break;
          default:
              _fstrcpy(szString,"An Undocumented Error Occurred.");
               
           }
         MessageBox(NULL,szString,"Load Library Error Message", MB_ICONSTOP);
         MessageBox(NULL,(LPCSTR)&MYLibs[i],"Load Library Error Message", MB_ICONSTOP); 
         HInstances[i] = NULL;
       }
    }  
    if(OK == -1) return FALSE; 
    return TRUE;
 
 }
 
 void FreeLibraries(void)
 {
     for (i=0;i< MaxNumLibs;i++)
     {
       if(HInstances[i] != NULL) 
          {
             FreeLibrary(HInstances[i]);
             HInstances[i] = NULL;
          }
     }
     return;
 }           
     
