

#if WIN32
   extern  char    *MS_MAPL( char *STND_PATH,  short STNDLN, long Ioff,
          long array_len, short share,  short mode, BOOL BL, 
          long *LM, long *TAT); 
   extern  char    *MS_CRMAPL (char *FNAME, short  NMLEN, long IOFF, long AR,
             short MS_,   short mode,   long *TAT); 
   extern void  MS_UNMAP (char   *STSC2, long LM, long *TAT);
#else
   extern  char  huge  *MS_MAPL( char *STND_PATH,  short STNDLN, long Ioff,
          long array_len, short share,  short mode, BOOL BL, 
          long *LM, long *TAT); 
   extern  char  huge  *MS_CRMAPL (char *FNAME, short  NMLEN, long IOFF, long AR,
             short MS_,   short mode,   long *TAT); 
   extern void  MS_UNMAP (char huge  *STSC2, long LM, long *TAT);
#endif             

#define MS_NR_XOR_1W 1 
#define MS_R 1
#define MS_WR 2