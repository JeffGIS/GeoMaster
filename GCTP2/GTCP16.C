#include <io.h>
#include <stdio.h>
#include <windows.h>    /* required for all Windows applications */ 
#include <string.h>

typedef long (FAR PASCAL *FPROC)();

static FPROC DLL_1, DLL_2, DLL_3, DLL_4; 
HANDLE hlib=0;
    
BOOL FAR PASCAL __export OpenGCTP32( long FAR *var1)
{ 
    long rn;
    rn = DLL_1(var1);        
    if(rn == 0 )
    {
      MessageBox( 0,
            "Unable to OPENGCTP",
            "gctp16.c",  MB_ICONEXCLAMATION );
            return FALSE;
    }
    return TRUE;
}   
long FAR PASCAL __export NADCON( long FAR *var1, long FAR *var2)
{ // gotta reverse the order of the arguements, cuz C puts them on 
  // the stack right to left and Fortran
  // takes them off the stack in left to right
    return(DLL_2( var1, var2 )) ;
}

long FAR PASCAL __export GCTPZ0( long FAR *var1, long FAR *var2,
                                 long FAR *var3, long FAR *var4, long FAR *var5 )
{ // gotta reverse the order of the arguements, cuz C puts them on 
  // the stack right to left and Fortran
  // takes them off the stack in left to right
    return(DLL_3( var1, var2, var3, var4, var5 )) ;
}
// Next is a routine used by Watcom Fortran 32 bit DLL

long FAR PASCAL __export CloseGCTPLibrary32( void )
{ 
    long rn;
    if (!hlib)
    	return 0;  
    rn = DLL_4(&rn);//this closes the 16 bit C dll that lda32.dll uses
    FreeLibrary(hlib); 
    hlib = 0;
    return rn;
}

BOOL  FAR PASCAL __export LoadGCTPLibrary32( void )
{  
    hlib = LoadLibrary( "gctp32.dll" );
    if( hlib < 32 ) 
    {
      MessageBox( 0,
            "Make sure your PATH contains gctp32.DLL",
            "DLL16",  MB_ICONEXCLAMATION );
      return( FALSE );
    }

     DLL_1 = (FPROC) GetProcAddress( hlib, "DLL1" );
    if(DLL_1 == NULL )
    {
      MessageBox( 0,
            "Unable to GetProcAddress for DLL_1",
            "GCTP16.c",  MB_ICONEXCLAMATION );  
            return (FALSE);
    }

     DLL_2 = (FPROC) GetProcAddress( hlib, "DLL2" );
    if(DLL_2 == NULL )
    {
      MessageBox( 0,
            "Unable to GetProcAddress for DLL_2",
            "GCTP16.c",  MB_ICONEXCLAMATION );
            return (FALSE);
    }       

    DLL_3 = (FPROC) GetProcAddress( hlib, "DLL3" );
    if(DLL_3 == NULL )
    {
      MessageBox( 0,
            "Unable to GetProcAddress for DLL_3",
            "GCTP16.c",  MB_ICONEXCLAMATION );
            return (FALSE);
    }       

     DLL_4 = (FPROC) GetProcAddress( hlib, "DLL4" );
    if(DLL_4 == NULL )
    {
      MessageBox( 0,
            "Unable to GetProcAddress for DLL_4",
            "GCTP16.c",  MB_ICONEXCLAMATION );
            return (FALSE);
    }       

   return (TRUE);
}
BOOL FAR PASCAL LibMain( HANDLE hInstance, WORD wDataSegment,
             WORD wHeapSize, LPSTR lpszCmdLine )
{ 
//    MessageBox( 0,
//          "Opening 16 bit C DLL version 7.20",
//          "dll16.c",  MB_ICONEXCLAMATION );

    return( TRUE );
} 

 int FAR PASCAL _WEP(int j)
 {
   return 1;
 }