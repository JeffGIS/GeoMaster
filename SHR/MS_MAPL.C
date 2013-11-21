#include  "shr.h"
static	BOOL	Firstx=TRUE;
 
struct lda4
    {
#if WIN32
      char  *mem_ptr; 
#else
      char huge *mem_ptr; 
#endif
      HGLOBAL heap_ptr;
      char  filename[MAX_PATH];
      BOOL ms_w_or_ro;
      DWORD total;
    } Mfile[128];   
 
/**************************************************************************************************/   
#if WIN32
   char  *MS_MAPL( char *STND_PATH,  short STNDLN, long Ioff, long array_len,
            short MS_NR_XOR_1W,    short MS_R, BOOL BL , long *LM, long *TAT) 
#else
   char huge *MS_$MAPL( char *STND_PATH,  short STNDLN, long Ioff, long array_len,
            short MS_NR_XOR_1W,    short MS_R, BOOL BL , long *LM, long *TAT) 
#endif

{
 int i;

  
DWORD total;
HFILE hf;
OFSTRUCT openbuff;
OFSTRUCT FAR* lpOpenBuff = &openbuff;
BOOL written;

	if (Firstx)
		memset (Mfile,0,sizeof(Mfile));
	Firstx = FALSE;
   for (i = 0; i < 128, Mfile[i].heap_ptr != NULL; i++); 
   if(i == 128) return NULL;
   Mfile[i].total = (DWORD) array_len + Ioff + 1;  
   total = (DWORD) Mfile[i].total;
   _fstrncpy( Mfile[i].filename,  STND_PATH, (size_t) STNDLN) ;   
   Mfile[i].heap_ptr = GSSiGlobAlloc ( 295, GHND, (size_t)    (Mfile[i].total) ); 
   Mfile[i].mem_ptr  = GlobalLock(Mfile[i].heap_ptr); 
   if(Mfile[i].mem_ptr == NULL) return Mfile[i].mem_ptr; 
   hf = GSSiOpenFile ( Mfile[i].filename, lpOpenBuff, OF_READ);
       
      if (hf == HFILE_ERROR )
      { // unable to open the file
         *TAT = 0;
         return 0;
      }          //         FILE,  pointer to string, length of string
      written = BigRead( hf , (LPSTR) Mfile[i].mem_ptr,  total);
  
     if( Ioff > 0) Mfile[i].mem_ptr = (LPSTR) Mfile[i].mem_ptr + Ioff; // user wants me to offset the pointer
     GSSiClose(hf); 
     if(MS_R == 2)
     {
     Mfile[i].ms_w_or_ro = TRUE;  // the file will be written upon closing
     }
     else
     {
     Mfile[i].ms_w_or_ro = FALSE;
     }
     *TAT = 1;
    return   Mfile[i].mem_ptr;
}

/**************************************************************************************************/   
 
#if WIN32
   char   *MS_CRMAPL (char *FNAME, short  NMLEN, long IOFF, long AR, short MS_,
                                  short MS_WR,   long *ISTAT) 
#else
   char  huge *MS_$CRMAPL (char *FNAME, short  NMLEN, long IOFF, long AR, short MS_,
                                  short MS_WR,   long *ISTAT) 
#endif

{ 
//HGLOBAL ni;
// char  far  *mine;

 int i; 
 	if (Firstx)
		memset (Mfile,0,sizeof(Mfile));
	Firstx = FALSE;
      for (i = 0; i < 128, Mfile[i].heap_ptr != NULL; i++);
       if(i == 128) return NULL;
     _fstrncpy( Mfile[i].filename, FNAME, (size_t) NMLEN );
      Mfile[i].heap_ptr =  GSSiGlobAlloc ( 296, GHND,  AR ); 
      Mfile[i].total = (DWORD) AR;
      Mfile[i].mem_ptr  = GlobalLock(Mfile[i].heap_ptr);   
     
     if(MS_WR == 2)
        Mfile[i].ms_w_or_ro = TRUE;  // the file will be written upon closing
     else
        Mfile[i].ms_w_or_ro = FALSE;
     return Mfile[i].mem_ptr;
    // return mine;
 }  
    
/**************************************************************************************************/   
#if WIN32
   void  MS_UNMAP (char  *STSC2, long LM, long *ISTAT)
#else
   void  MS_$UNMAP (char huge *STSC2, long LM, long *ISTAT)
#endif   
{             

HFILE hf;
OFSTRUCT openbuff;
OFSTRUCT FAR* lpOpenBuff = &openbuff;
char holder[64]; 
char *cp = holder;
int i; 
    if (!STSC2) return;
	if (Firstx)
		return;
    for (i = 0; i < 128, Mfile[i].mem_ptr != STSC2 ; i++);
     if(i == 128) return;
/*   Ok, got the right file */   
  if( Mfile[i].ms_w_or_ro ) // user wants to update the file
  {                     
      hf = GSSiOpenFile ( Mfile[i].filename, lpOpenBuff, OF_CREATE); 
      if (hf == HFILE_ERROR )
      { // unable to open the file
         return;
      }          //         FILE,  pointer to string, length of string
      BigWrite( hf ,  Mfile[i].mem_ptr,  Mfile[i].total,-1);  
     //  fputs (file[i].mem_ptr, fptr);
       GSSiClose(hf);                                                           
   }
      GSSiGlobUlFree (&Mfile[i].heap_ptr);                           
      STSC2 =NULL;                                   
      return;
}                                         
/*
BOOL BigWrite (HFILE  Fid, LPSTR pMF, DWORD isize)
{  
    DWORD   remain, wrote;
    UINT  mx;
    LPSTR   lpMF;

    mx = 32767;
    lpMF = pMF;
    remain = isize;
    while (remain > 0)
        if (remain > mx)
        {   if (wrote = _lwrite (Fid, lpMF, mx)  !=  mx) return(FALSE);
            remain -= mx;                                                          
            lpMF += mx;
        }
        else
        {   wrote = _lwrite (Fid , lpMF,(UINT) remain);
            if (wrote != remain) return(FALSE);
            remain = 0;
        }

     return (TRUE); 
}
*/
     
/*BOOL BigRead (HFILE  Fid, LPSTR pMF, DWORD isize)
{  
    DWORD   remain;
    UINT  mx;
    LPSTR   lpMF;

    mx = 32767;
    lpMF = pMF;
    remain = isize;
    while (remain > 0)
        if (remain > mx)
        {   if (_lread (Fid, lpMF, mx)  !=  mx) return(FALSE);
            remain -= mx;                                                          
            lpMF += mx;
        }
        else
        {   if ( _lread (Fid , lpMF, (UINT) remain) != remain) return(FALSE);
            remain = 0;
        }

     return (TRUE);
}  */
        
