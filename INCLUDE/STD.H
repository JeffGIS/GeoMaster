long  PATHNAME_TYPE(LPSTR STND_PATH,  short STNDLN);
//extern  void  ADD_TO_JUNK(LPSTR WHAT, long leng);
//extern void  GET_WORK_AREA( LPSTR PROJ, short ALT, short REV);
//extern void  UMDB_HOME_NODE(LPSTR PROJ, LPSTR NAME,short NMLEN,long *ST,long *ILOC) ;
void ERROr( long istat, char *message);

 

BOOL  LoadAbbreviations(LPSTR AddressDir);
void  ERROr(long ta, char *ERROR_IN_STNDSN);
void  PHONIC(LPSTR NAME, LPSHORT PART_LN);
BOOL  STNDSN_INIT(BOOL ReInit );
void  STNDNM(LPSTR INNAME, long INLEN, LPSTR STDNAM);
void  STREET_NAME_PARTS(LPSTR SNAME, LPSTR STYPE, LPSTR SCH, LPSTR SCHLOC);
void    STNDST(LPSTR INNAME, short  INLEN, 
               LPSTR STDNAM, 
               LPSTR NRONAM, 
               LPSTR NMONLY,
               LPSTR SANSCH, 
               LPSTR NANDCH, 
               LPSTR NCMPNM, 
               LPSTR ORIGNM,
               LPSTR SANSCP,
               LPSTR SANSCS,
               LPSTR PREFIX,
               LPSTR NAME,
               LPSTR TYPE,
               LPSTR SUFFIX);
void  STNDSN_CLEAR(void);      
BOOL GetSTDNamePart (LPSTR Name,LPSTR Type,LPSTR Part);
