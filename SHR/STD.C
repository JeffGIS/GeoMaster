#include "graphint.h"      
#include "mapl.h"  
#include "hash.h"
#include "std.h"  

#include <stddef.h>

#define MXPARTS  21                                                                               
#define a1511 1511 

typedef struct
     {
         BOOL16      COMPAS[a1511],
                   NAMABV[a1511];
         short     STSPLN[a1511],         
                   STSPLC[a1511];
     } far lda5;
typedef lda5 far *STS;

void a_batch(void);   

#include "gmextern.h"

static HANDLE	hSTDNAMES=0;
static BOOL FIRST = TRUE;   
static char  dump[27];
static char *cp = dump;  
static char  TEMP[21], BLNK1= ' ';
static char  PART[21], BLNK40[41] = {"                                        "} ;
static char  HYPSPA[3]= "' ";
static char  HYPHEN[]= "'",   LASTCHAR ,    SAVE_NAME[41], NAME[42];  
static	short LPART;
static	long LENNAM, NPARTS, IBEG,I,J, IEND, LOCNNO, IDISST,
     NOTCH, NOLOC, 
     NORTH, ISTATE, do_nothing,  LOCCH, NXI, NUMNAM, NUMDIG, 
     IRC, LEN1, LEN2, LEN3, LEN4, LEN5, LEN6,
     LENGTH, SCSLENGTH, SCPLENGTH, INC,NUMLOC, ILOC, ISTAT, HWYLOC; 
static LPSTR pTEMP; 
static int NLEN;
static	LPSTR LOC; 
static   long LM,  *TAT, whats_left, iiioff=32, the_end  ;
static	LPSTR  pNAME =   NAME;  
static BOOL NONCCH, PRTNCH, NEEDNO, NABV,  LASTAB,
          NEEDST, LASTNC,  INIT_CALL=TRUE;
static short PARTBG[MXPARTS],   PARTLN[MXPARTS],                     
          ACNBEG, ACNEND,        PARTBGO[MXPARTS],   PARTLNO[MXPARTS],
          STNDLN, ST,  OLD_NPARTS;
static long	PARTID[MXPARTS];  
static	int	ID=0;
static short LAST_2_DIGITS, SAVEOR, ORDER[MXPARTS], NMLEN,
          INITOR[MXPARTS]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20}, ALT, REV; 
static LPSTR ARRAY;
static char SUFFIX[3], PROJ[9], STND_PATH[128], HNODE[19];
static	STS STSNC2=0; 
static	int	NumCustomConversions = 0;
static	HANDLE	hCustomConversions=0;
     
long  NCHK(LPSTR name,long starting_at,long for_how_long)
  // returns == 1 if numeric (integer)
{
   long results=0 ;
   long value; 
   LPSTR	pEnd, pStop;
   char		ptest[100];
   
   if (for_how_long <= 0)return 0;
   if(for_how_long > 98)return 0;
   _fstrncpy ( ptest , (char *) &name[starting_at-1] ,(size_t)(for_how_long)); 
   ptest[for_how_long]=0;
   pStop = _fstrchr (ptest,0);
   value = strtol(ptest, &pEnd,10);
   if (pStop == pEnd)
   	results = 1;
   return results;
}

BOOL LoadCustomStreenNameConversions (BOOL Reload)
{
	static	BOOL	Loaded=FALSE;
	char	FileName[MAX_PATH];
	HFILE	Fid;

	if (Reload)
	{
		Loaded = FALSE;
		GSSiGlobFree (&hCustomConversions);
		if (Reload == 2)
			return TRUE;
	}
	if (Loaded)
		return TRUE;
    SetAddressDir();
	NumCustomConversions = 0;
	strcpy (FileName,AddMatchDir); 
    strcat (FileName, "\\PreConversions.txt");
	Loaded = TRUE;
	Fid = GSSiOpenFile (FileName,0,OF_READ);
	if (Fid != HFILE_ERROR)
	{
		int		l = GSSifilelength (Fid);
		LPSTR	pConv;

		hCustomConversions = GSSiGlobAlloc(GAIDNO 1591,GHND,l+128);
		pConv = GlobalLock (hCustomConversions);
		while (fgetstring (pConv,64,Fid))
		{
			LPSTR	pTab = strchr (pConv,'\t');

			if (pTab)
			{
				pConv = strchr (pConv,0) + 1;
				*pTab = 0;
			}
			NumCustomConversions++;
		}
		GlobalUnlock (hCustomConversions);
		GSSiClose2 (&Fid);
	}
	return TRUE;
}

void    STNDST2(LPSTR INNAME, short  INLEN, 
               LPSTR STDNAM, 
               LPSTR NRONAM, 
               LPSTR NMONLY,
               LPSTR SANSCH, 
               LPSTR NANDCH, 
               LPSTR NCMPNM, 
               LPSTR ORIGNM,
               LPSTR SANSCP,
               LPSTR SANSCS,
               LPSTR PREFIXPART,
               LPSTR NAMEPART,
               LPSTR TYPEPART,
               LPSTR SUFFIXPART)
#if ENABLETRACE
{GSSiEnterProg (1373);
#endif
{   
/*C******* SPECIFICATIONS ************************************************/
/*C*                                                                    **/
/*C*       PROGRAM SUMMARY                                              **/
/*C*       ------- -------                                              **/
/*C*    THIS ROUTINE STANDARDIZES STREET NAMES BY CONVERTING COMMON     **/
/*C*    ABREVIATIONS/MISSPELLINGS TO STANDARD FORM AND ORDERING THE     **/
/*C*    NAME IN A STANDARD WAY. THIS ROUTINE ALSO REMOVES AND PERIODS   **/
/*C*    QUOTES AND EXTRA BLANKS FROM THE NAME AND COMPRESSES MULTIPART  **/
/*C*    NAMES TO A SINGLE COMPOUND NAME (RIVER VIEW BECOMES RIVERVIEW). **/
/*C*    FOUR 20 BYTE char STRINGS ARE RETURNED. THEY ARE THE            **/
/*C*    STANDARDIZED NAME, THE NAME-ONLY PORTION OF THE STANDARDIZED    **/
/*C*    NAME, THE STANDARDIZED NAME MINUS THE COMPASS HEADING AND THE   **/
/*C*    NAME-ONLY PORTION PLUS THE COMPASS HEADING (OR THE STANDARDIZED **/
/*C*    NAME MINUS THE STREET TYPE).                                    **/
/*C*                                                                    **/
/*C*       ARGUMENT DESCRIPTION                                         **/
/*C*       -------- -----------                                         **/
/*C*    INNAME  (CHAR*INLN) THE INPUT STREET NAME                       **/
/*C*    INLN    I*4         THE LENGTH OF THE INPUT STREET NAME IN BYTES**/
/*C*    STDNAM  (CHAR*40)   THE STANDARDIZED STREET NAME                **/
/*C*    NRONAM  (CHAR*40)   THE NON-REORDERED STANDARDIZED NAME         **/
/*C*    NMONLY  (CHAR*40)   THE NAME-ONLY PORTION OF THE STANDARD NAME  **/
/*C*    SANSCH  (CHAR*40)   THE STANDARD NAME MINUS THE COMPASS HEADING **/
/*C*    NANDCH  (CHAR*40)   THE NAME-ONLY PLUS THE COMPASS HEADING      **/
/*C*    NCMPNM  (CHAR*40)   THE NON-COMPRESSED STANDARDIZED NAME        **/
/*C*    ORIGNM  (CHAR*40)   THE NAME ONLY PORTION IN ORIGINAL FORM      **/
/*C*                                                                    **/
/*C*       ERROR HANDLING                                               **/
/*C*       ----- --------                                               **/
/*C*    NO ERROR CODE IS returnED BY THIS ROUTINE, NOR ARE ANY ERROR    **/
/*C*    MESSAGES PRINTED BY IT. IT SHOULD HANDLE ANY char STRING   **/
/*C*    OF 30 OR LESS BYTES.                                            **/
/*C*                                                                    **/
/*C*       AUTHOR                                                       **/
/*C*       ------                                                       **/
/*C*     JEFF SMITH                                                     **/
/*C***********************************************************************/
//#include '/SYS/INS/MS.INS.FTN'
  long i, j, ii; 
  short	l,NumNameParts; 
  LPSTR	pLoc, pSpace;


/*C*    {*******************************************************/
/* A former entry point  */
/*C*    {*******************************************************/  
	LoadCustomStreenNameConversions (FALSE);
	if(INIT_CALL)STNDSN_INIT(FALSE);
 	if (PREFIXPART)
 	{ 
 		*PREFIXPART = 0;
 		*NAMEPART = 0;
 		*TYPEPART = 0;
 		*SUFFIXPART = 0;
 	} 
      _fstrncpy(STDNAM, BLNK40, 40);
      STDNAM[40] = '\0';
      _fstrncpy(NRONAM, BLNK40, 40);
      NRONAM[40] = '\0';
      _fstrncpy(NMONLY, BLNK40, 40);
      NMONLY[40] = '\0';
      _fstrncpy(SANSCH, BLNK40, 40);
      SANSCH[40] = '\0';
      _fstrncpy(NANDCH, BLNK40, 40);
      NANDCH[40] = '\0';
      _fstrncpy(NCMPNM, BLNK40, 40);
      NCMPNM[40] = '\0';
      _fstrncpy(ORIGNM, BLNK40, 40);
      ORIGNM[40] = '\0';
      _fstrncpy(SANSCP, BLNK40, 40);
      SANSCP[40] = '\0';
      _fstrncpy(SANSCS, BLNK40, 40);
      SANSCS[40] = '\0';
      NAME[0] = ' '; 
                               
//      _fmemmoveRO(INNAME(1),NAME(2:),INLEN);
//      _fstrncpy(pNAME+1 ,INNAME, INLEN);
	  while (*INNAME == '0') //remove leading zeros
		  INNAME++;
      _fstrcpy(pNAME+1 ,INNAME);
    //  for(I = INLEN+1; I < 40;I++, *pNAME = ' ');
    //  NAME[40] = '\0';
//      NAME[INLEN+2:] = " "; 
	  _fstrupr (pNAME);
      REPLAC (pNAME, ".", " ",40);
      OneSpace (&NAME[1]); 
      ACNBEG = 0;
      ACNEND = 0;
/*C******* BLANK OUT ANY PERIODS.*/
 //     REPLAC (NAME,INLEN,"."," "); 
      LOC = pNAME;
      NLEN   = _fstrlen (pNAME);
      if (!NLEN)
      	goto Exit;
	  if (NLEN > 2 && !strcmp (&pNAME[NLEN-2],"NO"))
	  {
		  pNAME[NLEN-1] = 0;
		  NLEN--;
	  }
	  if (NLEN > 2 && !strcmp (&pNAME[NLEN-2],"SO"))
	  {
		  pNAME[NLEN-1] = 0;
		  NLEN--;
	  }
/*C******* CHANGE MULTIPLE BLANKS TO SINGLE BLANK*/
      _fstrcat (NAME," "); 
      NLEN++;
      LENNAM = NLEN;
      LASTAB = FALSE;  
//	Change " NOnum " to " num " and " NO num " to " num "
	if ((pLoc = _fstrstr (NAME," NO")))
	{   
		pLoc += 3;
		if (*pLoc == ' ')
		{
			if ((pSpace = _fstrchr (pLoc+1,' ')))
			{
				if ((i=pSpace - pLoc) > 0)
				{
					if (*pLoc != '-' && NCHK (pLoc,1,i) == 1)
						*pLoc = '-';
				}
			}
		}
		else if ((pSpace = _fstrchr (pLoc,' ')))
		{
			if ((i=pSpace - pLoc) > 0)
			{
				if (*pLoc != '-' && NCHK (pLoc,1,i) == 1)
				{   
					l = _fstrlen (pLoc);
					_fmemmove (pLoc+1,pLoc,l+1);
					*pLoc = '-';
				}
			}
		}
	    REPLAC (NAME, " NO-", "",40);
	}
//	Change " Inum " to " I-num " and " I num " to " I-num "
	if ((pLoc = _fstrstr (NAME," I")))
	{   
		pLoc += 2;
		if (*pLoc == ' ')
		{
			if ((pSpace = _fstrchr (pLoc+1,' ')))
			{
				if ((i=pSpace - pLoc) > 0)
				{
					if (*pLoc != '-' && NCHK (pLoc,1,i) == 1)
						*pLoc = '-';
				}
			}
		}
		else if ((pSpace = _fstrchr (pLoc,' ')))
		{
			if ((i=pSpace - pLoc) > 0)
			{
				if (*pLoc != '-' && NCHK (pLoc,1,i) == 1)
				{   
					l = _fstrlen (pLoc);
					_fmemmove (pLoc+1,pLoc,l+1);
					*pLoc = '-';
				}
			}
		}
	}
//	Change " COnum " to " CO-num " and " CO num " to " CO-num "
	if ((pLoc = _fstrstr (NAME," CO")))
	{   
		pLoc += 3;
		if (*pLoc == ' ')
		{
			if ((pSpace = _fstrchr (pLoc+1,' ')))
			{
				if ((i=pSpace - pLoc) > 0)
				{
					if (*pLoc != '-' && NCHK (pLoc,1,i) == 1)
						*pLoc = '-';
				}
			}
		}
		else if ((pSpace = _fstrchr (pLoc,' ')))
		{
			if ((i=pSpace - pLoc) > 0)
			{
				if (*pLoc != '-' && NCHK (pLoc,1,i) == 1)
				{   
					l = _fstrlen (pLoc);
					_fmemmove (pLoc+1,pLoc,l+1);
					*pLoc = '-';
				}
			}
		}
	}
//	Change " COUNTYnum " to " CO-num " and " COUNTY num " to " CO-num "
	if ((pLoc = _fstrstr (NAME," COUNTY")))
	{   
		pLoc += 7;
		if (*pLoc == ' ')
		{
			if ((pSpace = _fstrchr (pLoc+1,' ')))
			{
				if ((i=pSpace - pLoc) > 0)
				{
					if (*pLoc != '-' && NCHK (pLoc,1,i) == 1)
						*pLoc = '-';
				}
			}
		}
		else if ((pSpace = _fstrchr (pLoc,' ')))
		{
			if ((i=pSpace - pLoc) > 0)
			{
				if (*pLoc != '-' && NCHK (pLoc,1,i) == 1)
				{   
					l = _fstrlen (pLoc);
					_fmemmove (pLoc+1,pLoc,l+1);
					*pLoc = '-';
				}
			}
		}
	    REPLAC (NAME, " COUNTY-", " CO-",40);
	}
//	Change " CNTYnum " to " CO-num " and " CNTY num " to " CO-num "
	if ((pLoc = _fstrstr (NAME," CNTY")))
	{   
		pLoc += 5;
		if (*pLoc == ' ')
		{
			if ((pSpace = _fstrchr (pLoc+1,' ')))
			{
				if ((i=pSpace - pLoc) > 0)
				{
					if (*pLoc != '-' && NCHK (pLoc,1,i) == 1)
						*pLoc = '-';
				}
			}
		}
		else if ((pSpace = _fstrchr (pLoc,' ')))
		{
			if ((i=pSpace - pLoc) > 0)
			{
				if (*pLoc != '-' && NCHK (pLoc,1,i) == 1)
				{   
					l = _fstrlen (pLoc);
					_fmemmove (pLoc+1,pLoc,l+1);
					*pLoc = '-';
				}
			}
		}
	    REPLAC (NAME, " CNTY-", " CO-",40);
	}
	if (hCustomConversions)
	{
		LPSTR pFrom = GlobalLock (hCustomConversions);
		LPSTR pTo	= strchr (pFrom,0)+1;

		for (i=0;i<NumCustomConversions;i++)
		{
			REPLAC (NAME,pFrom,pTo,40);
			pFrom = strchr (pTo,0)+1;
			pTo	  = strchr (pFrom,0)+1;
		}
		GlobalUnlock (hCustomConversions);
	}
/*C******* CHANGE " U S " TO " US "*/
      REPLAC (NAME, " U S ", " US ",40);
/*C******* CHANGE " HIGHWAY " TO " HWY "*/
      REPLAC (NAME, " HIGHWAY ", " HWY ",40);
/*C******* CHANGE " US HWY" TO " HWY "*/
      REPLAC (NAME, " US HWY ", " HWY ",40);
/*C******* CHANGE " I-" TO " INTERSTATE "*/
      REPLAC (NAME, " I-", " INTERSTATE ",40);
/*C******* CHANGE " INTERSTATE HWY " TO " INTERSTATE "*/
      REPLAC (NAME, " INTERSTATE HWY ", " INTERSTATE ",40);
/*C******* CHANGE " CO-" TO " CO RD"*/
      REPLAC (NAME, " CO-", " CO RD ",40);
/*C******* CHANGE " HWY-" TO " HWY "*/
      REPLAC (NAME, " HWY-", " HWY ",40);
/*C******* CHANGE " N E " TO " NE "*/
      REPLAC (NAME, " N E ", " NE ",40);
/*C******* CHANGE " N W " TO " NW "*/
      REPLAC (NAME, " N W ", " NW ",40);
/*C******* CHANGE " S W " TO " SW "*/
      REPLAC (NAME, " S W ", " SW ",40);
/*C******* CHANGE " S E " TO " SE "*/
      REPLAC (NAME, " S E ", " SE ",40);
/*C******* CHANGE " TH " TO "TH " i.e 5 TH ST to 5TH ST*/
      REPLAC (NAME, " TH ", "TH ",40);
/*C******* CHANGE " ND " TO "ND " i.e 2 ND ST to 2ND ST*/
      REPLAC (NAME, " ND ", "ND ",40);
/*C******* CHANGE " MC " TO " MC" i.e MC DONALD to MCDONALD*/
      REPLAC (NAME, " MC ", " MC",40);
/*C******* CHANGE "#" TO " "*/  
      REPLAC (NAME,"#"," ",40); 
/*C******* CHANGE "#" TO " "*/  
      REPLAC (NAME,"½","1/2",40); 
	  
/*C******* CHANGE HYPHEN-SPACE TO HYPHEN*/
      REPLAC (NAME, HYPSPA, HYPHEN, 40);

      OneSpace (NAME); 
      NLEN   = _fstrlen (pNAME);
      LENNAM = NLEN;
 
/*C******* REMOVE HYPHENS*/
      STRIPR (NAME, &NLEN, HYPHEN, 1);
      if (!NLEN) goto S1500;
/*C******* if NAME CONTAINS 1/2 REMOVE SPACE BETWEEN 1/2 AND PRECEEDING*/
/*C        PART THEN REMOVE "ST", "RD", ECT BEFORE 1/2 (1ST1/2 ST S)*/
       
      if (!_fstrstr(pNAME,"1/2")) goto S10;
      REPLAC ( pNAME, " 1/2","1/2", 40);
      REPLAC ( pNAME,"ST1/2","1/2", 40);
      REPLAC ( pNAME,"ND1/2","1/2", 40);
      REPLAC ( pNAME,"RD1/2","1/2", 40);
      REPLAC ( pNAME,"TH1/2","1/2", 40);
      NLEN   = _fstrlen (pNAME);
      LENNAM = NLEN;
S10:  _fstrcpy ( &SAVE_NAME[0], pNAME);
/*C******* FIND ALL PARTS OF NAME.*/
      NPARTS = 0;
      IBEG   = 0; //used to be 1;
  S110:   while(NAME[IBEG] == BLNK1)IBEG++;
          if (IBEG >= NLEN) goto S200;
          NPARTS++;
          ORDER  [NPARTS] = (short) NPARTS;
          PARTBG [NPARTS] = IBEG;
          PARTBGO[NPARTS] = IBEG;
  /*      IEND = INDEX (NAME[IBEG:NLEN)," ") + IBEG - 1;   */
          LOC = _fstrchr(&NAME[IBEG],' ');
          if(LOC == NULL)
             IEND = NLEN;  
          else
             IEND = (long) (ptrdiff_t)( LOC - &NAME[IBEG]) + IBEG;
         
          PARTLN [NPARTS] = IEND - PARTBG[NPARTS];
          PARTLNO[NPARTS] = IEND - PARTBG[NPARTS];
          if (IEND >= NLEN) goto S200;
          IBEG  = IEND + 1 ;
          goto S110;
            

/*C******* CHECK EACH PART AGAINST THE STANDARD SPELLING TABLE.*/
  S200: NEEDNO = FALSE;
      LOCNNO = 0;
      IDISST = 0;
  for  (I = 1;I <= NPARTS; I++)
  {//     DO 205 I = 1, NPARTS;
    //    PART = NAME[PARTBG[I]:PARTBG[I]+PARTLN[I]-1);
          _fstrncpy (PART,&NAME[PARTBG[I]],(size_t)PARTLN[I]);
          LPART = PARTLN[I];
          if (LPART > 20) LPART= 20;
          PART[LPART] = '\0';
          PHONIC ( PART, &LPART);                 
              
              
          PARTLN[I] = LPART ;
   //       NAME[PARTBG[I]:PARTBG[I]+PARTLN[I]-1] = PART; 
          for (i = LPART; i < 21; PART[i] = '\0', i++);
          _fstrncpy( &NAME[PARTBG[I]], PART, (size_t) PARTLN[I]);
          HASHF (ID, PART, &PARTID[I]);
  }
  for (I=1; I<=NPARTS; I++)
  {//       DO 230 I = 1, NPARTS;
          if (PARTID[I] <= 0) goto S220;
          if (!STSNC2->NAMABV[PARTID[I]]) goto S207;
          PARTID[I] = STSNC2->STSPLC[PARTID[I]]; 
          if (PARTID[I] == HWYLOC) 
          	goto S207;
          goto S220;
/*C      ******* if THIS PART IS "NO" DETERMINE if IT IS THE ABBREV*/
/*C              FOR NORTH OR NUMBER BY DETERMINING if IT IS FOLLOWED*/
/*C              BY A NUMERIC VALUE. if IT IS THE FIRST OR LAST PART IT*/
/*C              IS NORTH.*/
  S207:   if (PARTID[I] != NOLOC) goto S210;
          if (I == 1 || I == NPARTS) goto S208;
          PARTID[I] = -1;
        if (NCHK (&NAME[PARTBG[I+1]],1,PARTLN[I+1]) == 1)
        {
         PARTID[I+1] = -2; // {INDICATES NUMERIC PART OF NAME};
         goto S230;
        }   
  S208:  PARTID[I] = NORTH;
         goto S230;
/*C      ******** if THE STANDARD SPELLING OF THIS PART IS HWY OR CO*/
/*C               (HIWAY OR COUNTY) SET THE NEEDNO PARAMETER TO TRUE.*/
/*C               if IT IS # SET THE NEEDNO PARAMETER TO FALSE. if*/
/*C               THIS PART IS NUMERIC AND THE NEEDNO PARARETER IS*/
/*C               TRUE SAVE THE PARAMETER NUMBER IN LOCNNO.*/
/*C            ****** IDISST INDICATES if A ST PRECEEDS A NAME*/
/*  S210:   if (ARRAY[PARTID[I]] != "ST          ") goto S215;   */
  S210:   if (_fstrcmp( &ARRAY[PARTID[I]*13],"ST")!= 0) goto S215; 
         // _fmemcpy(dump,&ARRAY[PARTID[I]*13],26);   
          if (I > 1) // check for '1 ST AVE' type name
          {
        	if (NCHK (&NAME[PARTBG[I-1]],1,PARTLN[I-1]) == 1)
          		ii=1;
          }
          if (I == NPARTS) goto S216;
          if (PARTID[I+1] == 0) goto S216;
          if (_fstrcmp(&ARRAY[STSNC2->STSPLC[PARTID[I+1]]*13],"HWY")!= 0)
//          if (ARRAY[STSPLC[PARTID[I+1]]] != "HWY")
                                goto S216;
          PARTID[I] = ISTATE;
          goto S230;
  S216:   IDISST = I;
  S215:   PARTID[I] = STSNC2->STSPLC[PARTID[I]];
/*          if (ARRAY[PARTID[I]] == "HWY         " !!
              ARRAY[PARTID[I]] == "CO          ") NEEDNO = TRUE;
          if (ARRAY[PARTID[I]] == "#           ") NEEDNO = FALSE;  */
      //    _fmemcpy(dump,&ARRAY[PARTID[I]*13],26);
          if (_fstrcmp(&ARRAY[PARTID[I]*13],"HWY") ==0)
          	NEEDNO = TRUE;
          if (_fstrcmp(&ARRAY[PARTID[I]*13],"CO") ==0)
          	NEEDNO = TRUE;
          if (_fstrcmp(&ARRAY[PARTID[I]*13], "#") ==0)
          	NEEDNO = FALSE;
          goto S230;
  S220:   if (IDISST != 0) PARTID[IDISST] = 0;
          if (!NEEDNO||NCHK(&NAME[PARTBG[I]],1,PARTLN[I])         != 1) goto S230;
          LOCNNO = I;
  S230:   do_nothing = 0; 
  } //END OF A FOR LOOP
/*C******* if LOCNNO IS NOT 0 INSERT A # AT LOCNO.*/
/*C      if (LOCNNO == 0) goto S300*/
/*C          DO 250 J = LOCNNO, NPARTS*/
/*C          I = NPARTS - J + LOCNNO*/
/*C          PARTID(I+1) = PARTID(I)*/
/*C          PARTBG(I+1) = PARTBG(I)*/
/*C  250     PARTLN(I+1) = PARTLN(I)*/
/*C      PARTID(LOCNNO) = NUMLOC*/
/*C      NPARTS  = NPARTS + 1*/
/*C      ORDER(NPARTS)  = NPARTS*/
/*C******* SET THE FIRST PART OF ACTUAL NAME TO THE FIRST PART NOT IN*/
/*C        THE ABBREVIATION TABLE AND LAST PART TO THE LAST CONSECUTIVE*/
/*C        PART NOT IN TABLE. TREAT ABBREVIATED NAMES AS if THEY WERE*/
/*C        NOT IN THE TABLE.*/
    NOTCH = 0;
    for (I=1; I<=NPARTS; I++)  
    { //          DO 325 I = 1, NPARTS;
          if (PARTID[I] < 0) goto S325;
          if (PARTID[I] == 0) goto S305;
          if (!STSNC2->NAMABV[PARTID[I]]) goto S320;
  S305:   ACNBEG = I;
          ACNEND = I;
/*C         if A NONCH IMMEDIATLY PRECEEDS THE FIRST NAME PART*/
/*C          SET THE NAME BEGINNING TO THIS NONCH*/
          if (NOTCH != 0 && NOTCH == I-1)
           {  
            ACNBEG = I - 1;
            PARTID[I-1] = 0;
           }   
  S310:    if (ACNEND >= NPARTS) goto S400;
           if (PARTID[ACNEND+1] <= 0) goto S315;
           if (!STSNC2->NAMABV[PARTID[ACNEND+1]]) goto S400;
  S315:    ACNEND = ACNEND + 1;
           goto S310;
  S320:    if (STSNC2->COMPAS[PARTID[I]] || NOTCH != 0) goto S325;
           NOTCH  = I;
  S325: do_nothing = 0; 
  } // END OF THE FOR LOOP
/*C            if NO ACTUAL NAME FOUND DO THE FOLLOWING:*/
/*C            if ONE OR LESS NONCH PAR FOUND SET THE ACTUAL NAME TO THE*/
/*C            COMPASS HEADINGS BEFORE THE FIRST NON-COMPASS HEADING.*/
/*C            if NO COMPASS HEADINGS USE THE ORIGINAL SPELLING OF THE*/
/*C            FIRST PART.*/
/*C            if MORE THAN ONE NONCH (STREET TYPE) FOUND SET THE ACTUAL*/
/*C            NAME TO THE FIRST NONCH.*/
      if (NOTCH == 0) goto S3265;
      LASTNC = FALSE;
    for (I=NOTCH; I<=NPARTS;I++)
    { //  DO 3262 I = NOTCH, NPARTS;
          if (PARTID[I] < 0) goto S3262;
          if (STSNC2->COMPAS[PARTID[I]]) goto S3262;
          if (LASTNC)
              {  
                  ACNBEG = I - 1;
                  ACNEND = ACNBEG;
                  goto S3266;
              }
              else
              {
                  LASTNC = TRUE;
              }  
 S3262:     do_nothing = 0; 
   } /*end of the for loop   */
 S3265: if (NOTCH <= 1) NOTCH = 2;
      ACNBEG = 1;
      ACNEND = NOTCH - 1;
 S3266:   for (I=ACNBEG;I<=ACNEND;I++)
   { //  DO 327 I = ACNBEG, ACNEND;
          if (PARTID[I] > 0) PARTID[I] = 0;
      do_nothing = 0;
   } //end of the for loop
/*C******* if A NUMERIC PART FOLLOWS THE END OF NAME, SET END OF NAME*/
/*C        TO THIS PART (S FARM RD 111)*/
S400:   for (I=ACNEND+1; I<=NPARTS;I++)
        { // DO I = ACNEND + 1, NPARTS;
          if (NCHK(&NAME[PARTBG[I]],1,PARTLN[I]) == 1) { 
              ACNEND = I;
              goto S4000;
          }  //
        } // ENDDO;
/*C******* REMOVE ANY PARTS NOT IN THE NAME AND NOT A TYPE OR DIRECTION*/
/*C        WHICH OCCURE AT THE END OF THE STRING AND PLACE INTO THE JUNK*/
/*C        FIELD*/
 S4000: OLD_NPARTS = NPARTS;
      for (I = OLD_NPARTS; I>=0;I--)
      { //DO I = OLD_NPARTS, 2, -1;
          if (PARTID[I] == 0 && I > ACNEND && ACNEND > 0)
             NPARTS--;
          else
             goto S401;
      } // ENDDO;
  S401: if (OLD_NPARTS > NPARTS)
        {  
          for (I=NPARTS+1;I<=OLD_NPARTS;I++)
          { // DO I = NPARTS + 1, OLD_NPARTS;
        //    ADD_TO_JUNK (&NAME[PARTBG[I]] ,PARTLN[I]-1);
          } //ENDDO;
        }  
      for ( I=1 ; I<=NPARTS ; I++)
      { //DO I = 1, NPARTS;
          if (PARTID[I] == -2) PARTID[I] = 0;
      } //ENDDO;
/*C*       if THERE ARE 2 CONTIGUOUS COMPASS HEADINGS SET THEM AS ONE*/
/*C        PART. if THERE ARE NO NON-CONTIGUOUS COMPASS HEADINGS MOVE*/
/*C        THE CH TO THE LAST PART.*/
      if (NPARTS == 1) goto S500;
      LOCCH  = -1;
      NONCCH = FALSE;
/*C     PRTNCH = FALSE*/
       for (I=1;I<=NPARTS;I++)
       {//   DO 420 I = 1, NPARTS;
          if (PARTID[I] <= 0) goto S420;
          NXI = I;
  S402:   NXI = NXI + 1;
          if (NXI <= NPARTS && PARTID[NXI] < 0) goto S402;
/*C         if (.NOT. STSNC2->COMPAS(PARTID(I)) && .NOT. NAMABV(PARTID(I)))*/
/*C    +                                                   PRTNCH = TRUE*/
          if (I == NPARTS) goto S410;
          if (PARTID[NXI] == 0) goto S410;
          if (!STSNC2->COMPAS[PARTID[I]] || !STSNC2->COMPAS[PARTID[NXI]]
              || NXI > NPARTS) goto S410;
/*          TEMP = ARRAY[PARTID[I]]    [1:1*STSPLN[PARTID[I]]] //
                 ARRAY[PARTID[NXI]]  [1:1*STSPLN[PARTID[NXI]]];   */
          pTEMP = _fmemcpy (TEMP, &ARRAY[PARTID[I]*13], (size_t)PARTLN[I]);
          pTEMP = _fmemcpy (&TEMP[PARTLN[I]], &ARRAY[PARTID[NXI]*13], (size_t)PARTLN[NXI] ); 
          for (j = (PARTLN[I]+PARTLN[NXI]); j < 12; j++, pTEMP[j] = ' ');
          pTEMP[12] = '\0';
          HASHF (ID,TEMP,&NOLOC);
          if (NOLOC == 0) goto S410;
          PARTID[I] = NOLOC;
          PARTID[I+1] = -1;
  S410:     if (!STSNC2->COMPAS[PARTID[I]]) goto S420;
          if (LOCCH == I - 1) goto S420;
          if (LOCCH != -1) NONCCH = TRUE;
          LOCCH  = I;
  S420:   do_nothing = 0; 
    } // end of the for loop
/*C     if (NONCCH !! .NOT. PRTNCH !! LOCCH == -1. OR.*/
      if (NONCCH || LOCCH == -1 || LOCCH == NPARTS) goto S500;
      J     = LOCCH + 1;
      SAVEOR = ORDER[LOCCH];
      for (I=J;I<=NPARTS;I++)
      { //    DO 450 I = J, NPARTS;
       ORDER[I-1] = ORDER[I];
      } //end of the for loop
      ORDER[NPARTS] = SAVEOR;
 S500: do_nothing = 0;
/*C*    GENERATE THE NON-REORDERED NAME*/
/*C******* if THERE IS ONLY 1 TYPE 0 (NAME) PART AND IT IS NUMERIC ADD THE*/
/*C        APPROPRIATE EXTENSION TO IT (I.E.  12 BECOMES 12TH, 101 BECOMES 101ST)*/
      NUMNAM = 0;
     for (I=1;I<=NPARTS;I++)
     {// DO I = 1, NPARTS;
          if (PARTID[I]== 0)
          {  
              if (NUMNAM != 0) goto S1000;
              NUMNAM = I;
          }
          else
          {
            if (PARTID[I] > 0)
            {  
              if (STSNC2->NAMABV[PARTID[I]] && _fstrcmp(&ARRAY[PARTID[I]*13],"HWY"))
              	goto S1000;
            }
          }
               
      } //ENDDO;

      if (NUMNAM > 0)
      {
          if (NCHK (&NAME[PARTBG[NUMNAM]],1,PARTLN[NUMNAM]) != 1)
              goto S1000; 
          if (NUMNAM < NPARTS && !_fstrcmp(&ARRAY[PARTID[NUMNAM+1]*13],"HWY")) //switch 65 HWY NE to HWY 65 NE
          {
          	short SaveOrder = ORDER[NUMNAM];
          	ORDER[NUMNAM]=ORDER[NUMNAM+1];
          	ORDER[NUMNAM+1] = SaveOrder; 
          	if (ACNBEG == ACNEND)
          		ACNEND++;
          	goto S1000;
          }
          if (NEEDNO)
          	goto S1000;
          if (PARTLN[NUMNAM] == 1)
          { 
              NUMDIG = 1;
          }
          else
          {
              NUMDIG = 2;
          }  
          IBEG = PARTBG[NUMNAM] + PARTLN[NUMNAM] - NUMDIG;
          IRC = -1;
          LAST_2_DIGITS = (short) IREAD (&NAME[IBEG], NUMDIG, &IRC);
          if (IRC != 0) goto S1000;
          _fstrcpy (SUFFIX,"TH");
          if (LAST_2_DIGITS < 10 || LAST_2_DIGITS > 20)
          {
              LAST_2_DIGITS =  LAST_2_DIGITS % 10;
              switch (LAST_2_DIGITS)
              {
                case 1:                                                     
                 {                                  
                  _fstrcpy(SUFFIX, "ST");
                  break;
                 }
                case 2:
                {
                  _fstrcpy(SUFFIX, "ND");
                  break;
                }  
                case 3:
                  _fstrcpy(SUFFIX, "RD");
             } // end of the switch  
          }   
          for (I= NUMNAM+1; I<= NPARTS;I++)
          { //DO I = NUMNAM + 1, NPARTS;
             // _fmemmoveRO(NAME[PARTBG[I]:],NAME[PARTBG[I]+2:],PARTLN[I]);
             _fmemmove (&NAME[PARTBG[I]+2], &NAME[PARTBG[I]],(size_t)PARTLN[I]);
              PARTBG[I] = PARTBG[I] + 2;
          } // ENDDO;
         // NAME[PARTBG[NUMNAM]+PARTLN[NUMNAM]:PARTBG[NUMNAM]+PARTLN[NUMNAM]+1] = SUFFIX;
          _fstrncpy(&NAME[PARTBG[NUMNAM]+PARTLN[NUMNAM]],SUFFIX,2);
          PARTLN[NUMNAM] = PARTLN[NUMNAM] + 2;
      }  //

S1000:  LENGTH = -1; 
		SCPLENGTH = -1;
		SCSLENGTH = -1;
     for (I=1;I<=NPARTS;I++)
     { //DO 1150 I = 1, NPARTS;
          INC    = 1;
          if (I >= ACNBEG && I < ACNEND) INC = 0;
          if (PARTID[I] < 0) goto  S1150;
          if (PARTID[I] == 0) goto S1110;
          goto S1120;
 //S1110:   NRONAM[LENGTH+1:LENGTH+PARTLN[I]] = NAME[PARTBG[I]:];
S1110:   _fstrncpy(&NRONAM[LENGTH+1], &NAME[PARTBG[I]],(size_t) PARTLN[I]);
		  _fstrncpy(&SANSCS[SCSLENGTH+1], &NAME[PARTBG[I]],(size_t) PARTLN[I]);
		  _fstrncpy(&SANSCP[SCPLENGTH+1], &NAME[PARTBG[I]],(size_t) PARTLN[I]);
          LENGTH += PARTLN[I] + INC;
          SCSLENGTH += PARTLN[I] + INC;
          SCPLENGTH += PARTLN[I] + INC;
          goto S1150;
// S1120:   NRONAM[LENGTH+1:LENGTH+STSPLN[PARTID[I]]] =  ARRAY[PARTID[I]];
 S1120:   _fstrncpy(&NRONAM[LENGTH+1], &ARRAY[PARTID[I]*13],(size_t) STSNC2->STSPLN[PARTID[I]]);
          LENGTH += STSNC2->STSPLN[PARTID[I]] + INC;
  		  if (!STSNC2->COMPAS[PARTID[I]] || I < ACNBEG)
  		  {
  		  	_fstrncpy(&SANSCS[SCSLENGTH+1], &ARRAY[PARTID[I]*13],(size_t) STSNC2->STSPLN[PARTID[I]]);
          	SCSLENGTH += STSNC2->STSPLN[PARTID[I]] + INC; 
          }
  		  if (!STSNC2->COMPAS[PARTID[I]] || I > ACNEND)
  		  {
  		  	_fstrncpy(&SANSCP[SCPLENGTH+1], &ARRAY[PARTID[I]*13],(size_t) STSNC2->STSPLN[PARTID[I]]);
          	SCPLENGTH += STSNC2->STSPLN[PARTID[I]] + INC; 
          }
 S1150: do_nothing = 0;
     } //end of the for loop  
     NRONAM[max(LENGTH,0)]=0;
     SANSCS[max(SCSLENGTH,0)]=0;
     SANSCP[max(SCPLENGTH,0)]=0;
/*C*    GENERATE THE NAME ONLY, THE NAME PLUS COMPASS HEADING, THE*/
/*C        STANDARDIZED NAME AND THE NAME MINUS THE COMPASS HEADING*/
      LEN1   = -1;
      LEN2   = -1;
      LEN3   = -1;
      LEN4   = -1;
      LEN5   = -1;                  
      LEN6   = -1;                  
      NEEDST = TRUE;
      for(I=0;I<=NPARTS;I++)
      { 
       if( ORDER[I] != INITOR[I]) goto S1200; 
      } 
     //  if (ICMP (ORDER[1],INITOR[1],NPARTS*2) != 0) goto S1200;
      _fstrcpy(STDNAM,NRONAM);
      NEEDST = FALSE; 
      
      
 S1200: NumNameParts = ACNEND - ACNBEG + 1;
 		for (J=1;J<=NPARTS;J++)
        {//    DO 1250 J = 1, NPARTS;
          I      = ORDER[J];
          INC    = 1;
//          if (J >= ACNBEG && J < ACNEND) INC = 0; 
//was 
//          if (I >= ACNBEG && I < ACNEND) INC = 0;//5/19/2005 for 65 HWY NE to HWY65 NE
          if (J < NumNameParts)
          	INC = 0;
          if (PARTID[I]< 0) goto S1250;
          if (PARTID[I] == 0) goto S1210;
          goto S1220;
 /* S1210:  NMONLY[LEN1+1:LEN1+PARTLN[I]] = NAME[PARTBG[I]:];
          NANDCH[LEN2+1:LEN2+PARTLN[I]] = NAME[PARTBG[I]:];
          SANSCH[LEN3+1:LEN3+PARTLN[I]] = NAME[PARTBG[I]:];
          NCMPNM[LEN4+1:LEN4+PARTLN[I]] = NAME[PARTBG[I]:];  */
  S1210:  _fstrncpy(&NMONLY[LEN1+1], &NAME[PARTBG[I]],(size_t)PARTLN[I]);
          _fstrncpy(&NANDCH[LEN2+1], &NAME[PARTBG[I]],(size_t)PARTLN[I]);
          _fstrncpy(&SANSCH[LEN3+1], &NAME[PARTBG[I]],(size_t)PARTLN[I]);
          _fstrncpy(&NCMPNM[LEN4+1], &NAME[PARTBG[I]],(size_t)PARTLN[I]);
          LEN1   = LEN1 + PARTLN[I] + INC;
          LEN2   = LEN2 + PARTLN[I] + INC;
          LEN3   = LEN3 + PARTLN[I] + INC;
          LEN4   = LEN4 + PARTLN[I] + 1;
          if (!NEEDST) goto S1250;
          _fstrncpy(&STDNAM[LEN5+1], &NAME[PARTBG[I]],(size_t)PARTLN[I]);
        //  STDNAM[LEN5+1:LEN5+PARTLN[I]] = NAME[PARTBG[I]:];
          LEN5   = LEN5 + PARTLN[I] + INC;
          goto S1250;
 S1220:   if (STSNC2->NAMABV[PARTID[I]]) goto S1225;
          if (!STSNC2->COMPAS[PARTID[I]]) goto S1245;
        //  NANDCH[LEN2+1:LEN2+STSPLN[PARTID[I]]] = ARRAY[PARTID[I]];
       //   NCMPNM[LEN4+1:LEN4+STSPLN[PARTID[I]]] = ARRAY[PARTID[I]];
          _fstrncpy(&NANDCH[LEN2+1], &ARRAY[PARTID[I]*13],(size_t)STSNC2->STSPLN[PARTID[I]]);
          _fstrncpy(&NCMPNM[LEN4+1], &ARRAY[PARTID[I]*13],(size_t)STSNC2->STSPLN[PARTID[I]]);
          LEN2   = LEN2 + STSNC2->STSPLN[PARTID[I]] + INC;
          LEN4   = LEN4 + STSNC2->STSPLN[PARTID[I]] + 1;
          if (!NEEDST) goto S1250;
          _fstrncpy(&STDNAM[LEN5+1], &ARRAY[PARTID[I]*13],(size_t)STSNC2->STSPLN[PARTID[I]]);
          LEN5   = LEN5 + STSNC2->STSPLN[PARTID[I]] + INC;
          goto S1250;
 /*S1225:   NMONLY[LEN1+1:LEN1+STSPLN[PARTID[I]]] = ARRAY[PARTID[I]];
          NANDCH[LEN2+1:LEN2+STSPLN[PARTID[I]]] = ARRAY[PARTID[I]];
          SANSCH[LEN3+1:LEN3+STSPLN[PARTID[I]]] = ARRAY[PARTID[I]];
          NCMPNM[LEN4+1:LEN4+STSPLN[PARTID[I]]] = ARRAY[PARTID[I]];  */
 S1225:   _fstrncpy(&NMONLY[LEN1+1], &ARRAY[PARTID[I]*13], (size_t)STSNC2->STSPLN[PARTID[I]]);
          _fstrncpy(&NANDCH[LEN2+1], &ARRAY[PARTID[I]*13], (size_t)STSNC2->STSPLN[PARTID[I]]);
          _fstrncpy(&SANSCH[LEN3+1], &ARRAY[PARTID[I]*13], (size_t)STSNC2->STSPLN[PARTID[I]]);
          _fstrncpy(&NCMPNM[LEN4+1], &ARRAY[PARTID[I]*13], (size_t)STSNC2->STSPLN[PARTID[I]]);
          LEN1   = LEN1 + STSNC2->STSPLN[PARTID[I]] + INC;
          LEN2   = LEN2 + STSNC2->STSPLN[PARTID[I]] + INC;
          LEN3   = LEN3 + STSNC2->STSPLN[PARTID[I]] + INC;
          LEN4   = LEN4 + STSNC2->STSPLN[PARTID[I]] + 1;
          if (!NEEDST) goto S1250;
          _fstrncpy(&STDNAM[LEN5+1], &ARRAY[PARTID[I]*13], (size_t)STSNC2->STSPLN[PARTID[I]]);
          LEN5   = LEN5 + STSNC2->STSPLN[PARTID[I]] + INC;
          goto S1250;
 S1245:   _fstrncpy(&SANSCH[LEN3+1], &ARRAY[PARTID[I]*13], (size_t)STSNC2->STSPLN[PARTID[I]]);
          _fstrncpy(&NCMPNM[LEN4+1], &ARRAY[PARTID[I]*13], (size_t)STSNC2->STSPLN[PARTID[I]]);
          LEN3   = LEN3 + STSNC2->STSPLN[PARTID[I]] + INC;
          LEN4   = LEN4 + STSNC2->STSPLN[PARTID[I]] + 1;
          if (!NEEDST) goto S1250;
          _fstrncpy(&STDNAM[LEN5+1], &ARRAY[PARTID[I]*13], (size_t)STSNC2->STSPLN[PARTID[I]]);
          LEN5   = LEN5 + STSNC2->STSPLN[PARTID[I]] + INC;
 S1250:     do_nothing = 0; 
   } //end of the for loop
/*C******* GENERATE NONE ABREVIATED, NON PHONICED NAME ONLY*/
	 NumNameParts = ACNEND - ACNBEG + 1;
     for (J=1;J<=NPARTS;J++)
     {// DO  J = 1, NPARTS;
          I      = ORDER[J];
          if (I >= ACNBEG && I <= ACNEND) { // 
              _fstrncpy(&ORIGNM[LEN6+1], &SAVE_NAME[PARTBGO[I]], (size_t)PARTLNO[I]);
              LEN6 = LEN6 + PARTLNO[I]+1;
          }  //
          if (I > ACNEND && PARTID[I] == 0) { //
              _fstrncpy(&ORIGNM[LEN6+1], &SAVE_NAME[PARTBGO[I]], (size_t)PARTLNO[I]);
              LEN6 = LEN6 + PARTLNO[I]+1;
          }  //
     }// ENDDO;
 S1500:
 	if (PREFIXPART)
 	{ 
		for (J=1;J<=NPARTS;J++) 
		{
			I      = ORDER[J];
			if (I >= ACNBEG && I <= ACNEND)
			{ 
			  _fstrncat(NAMEPART,&SAVE_NAME[PARTBGO[I]], (size_t)PARTLNO[I]);
			  _fstrcat (NAMEPART," ");
			}
			else if (I < ACNBEG)
			{  
			  _fstrncat(PREFIXPART,&SAVE_NAME[PARTBGO[I]], (size_t)PARTLNO[I]);
			  _fstrcat (PREFIXPART," ");
			}
			else if (!STSNC2->COMPAS[PARTID[I]])
			{  
			  _fstrncat(TYPEPART,&SAVE_NAME[PARTBGO[I]], (size_t)PARTLNO[I]);
			  _fstrcat (TYPEPART," ");
			}
			else  
			{  
			  _fstrncat(SUFFIXPART,&SAVE_NAME[PARTBGO[I]], (size_t)PARTLNO[I]);
			  _fstrcat (SUFFIXPART," ");
			}
		}
 	} 
 	STDNAM[max(0,LEN5)]=0;
 	NMONLY[max(0,LEN1)]=0;
 	NANDCH[max(0,LEN2)]=0;
 	SANSCH[max(0,LEN3)]=0;
 	NCMPNM[max(0,LEN4)]=0;
 	ORIGNM[max(0,LEN6)]=0; 
 Exit:
{
#if ENABLETRACE
GSSiExitProg (1373);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

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
               LPSTR PREFIXPART,
               LPSTR NAMEPART,
               LPSTR TYPEPART,
               LPSTR SUFFIXPART)
{
	char INNAME2[42];

	strncpy0 (INNAME2,INNAME,min(INLEN,40));

	STNDST2(INNAME2,strlen(INNAME2), 
           STDNAM, 
           NRONAM, 
           NMONLY,
           SANSCH, 
           NANDCH, 
           NCMPNM, 
           ORIGNM,
           SANSCP,
           SANSCS,
           PREFIXPART,
           NAMEPART,
           TYPEPART,
           SUFFIXPART);
	return;

}
 
 /***************************************************************************/
 /* A former entry point  */
 /***************************************************************************/
void   STREET_NAME_PARTS(LPSTR SNAME, LPSTR STYPE, LPSTR SCH, LPSTR SCHLOC)  
 {      
  //   char SNAME[33], STYPE[13], SCH[3], SCHLOC; 
  
long  LEN1 = 0;
    
      _fstrset(SNAME ,' ');
      _fstrset(STYPE,' ');
      _fstrset(SCH,' ');
      _fstrset(SCHLOC,' ');
     
      for (I=1; I<=NPARTS ; I++)
      {//     DO I = 1, NPARTS;
          if (PARTID[I] < 0) goto S1621;
          if (PARTID[I] > 0 &&(I<ACNBEG || I>ACNEND))
          {  
              if (STSNC2->COMPAS[PARTID[I]])
              { 
                  if (NONCCH)
                  {  
                      NONCCH = FALSE;
                      goto S1620;
                  }  
                  _fstrncpy((char *)SCH, &ARRAY[PARTID[I]*13],(size_t)PARTLN[I]);
                  if (I < ACNBEG)
                       SCHLOC = "B";
                  else
                      SCHLOC ="E";
              }
              else
              {
                if (!STSNC2->NAMABV[PARTID[I]]) 
                 _fstrncpy((char *)STYPE, &ARRAY[PARTID[I]*13],(size_t) PARTLN[I]);
              }  //end of if (COMPAS[PARTID[I]])   
          }
          else
          {
S1620:        _fstrncpy( &SNAME[LEN1+1], &SAVE_NAME[PARTBGO[I]],(size_t) PARTLNO[I]);
              LEN1 = LEN1 + PARTLNO[I] + 1;
             // SNAME[LEN1:] = " ";
          } // end of  if (PARTID[I] > 0 &&(I<ACNBEG !! I>ACNEND))
S1621:  do_nothing = 0;            
     }// ENDDO;
      return ;
}       
/*C*    {***********************************************************************/
/* A former entry point  */
 /*C*    {***********************************************************************/
void   STNDNM (LPSTR INNAME,long INLEN, LPSTR STDNAM) 
{ 
long I,J, LENNAM, NPARTS, IBEG, IEND, LOCNNO, IDISST, LENGTH  ;      
int   NLEN;      

      _fstrncpy ((char *)&STDNAM, BLNK40, 40);
      NAME[0] = BLNK1;
     // _fmemmoveRO(INNAME[1],NAME[2:],INLEN);
      _fmemmove ((char*)&NAME[1],(char *)INNAME ,(size_t)INLEN);
      _fstrset (&NAME[INLEN+2],' ');
/*C******* BLANK OUT ANY PERIODS.*/ 
      LOC = pNAME;
      while (*pNAME++ == *pNAME) if(*pNAME == '.') *pNAME = ' ';
      pNAME = LOC;  
      // REPLAC (NAME,INLEN,"."," ");
      NLEN   = _fstrlen ((char *)&NAME) + 1;
      if (NLEN == 1) return;
/*C******* CHANGE MULTIPLE BLANKS TO SINGLE BLANK*/
      LENNAM = NLEN;
      LASTAB = FALSE; 
/*C******* CHANGE "#" TO " "*/
      REPLAC (pNAME, "#", " ", 40);
      
       for(I=1;I<=NLEN;I++)
       {// DO 5008 I = 1, NLEN;
          J = NLEN - I + 1;
          if (NAME[J] == ' ') goto S5006;
          LASTAB = FALSE;
          goto S5008;
S5006:    if (LASTAB) goto S5007;
          LASTAB = TRUE;
          goto S5008;
S5007:    _fmemmove((char *)&NAME[J],(char *)&NAME[J+1],(size_t) (LENNAM-J));
          LENNAM = LENNAM - 1;
S5008:    do_nothing = 0; 
        } //end of the for loop

      NLEN = LENNAM;
/*C******* CHANGE HYPHEN-SPACE TO HYPHEN*/
      REPLAC ((LPSTR)NAME, HYPSPA, HYPHEN, NLEN);
/*C******* REMOVE HYPHENS*/
      STRIPR (NAME, &NLEN, HYPHEN, 1);
      if (NLEN == 0) goto S1500;
       _fstrncpy((char *)&SAVE_NAME, (char *) &NAME,(size_t) NLEN);
/*C******* FIND ALL PARTS OF NAME.*/
      NPARTS = 0;
      IBEG   = 0;
S5110:     if (NAME[IBEG] != BLNK1) goto S5115;
          IBEG = IBEG + 1;
          if (IBEG > NLEN) goto S5200;
          goto S5110;
 S5115:     NPARTS  = NPARTS + 1;
          ORDER[NPARTS]  = (short) NPARTS;
          PARTBG[NPARTS] = IBEG;
          PARTBGO[NPARTS] = IBEG;
          LOC =(char *) _fstrchr (&NAME[IBEG],' ');
          IEND =  LOC - &NAME[IBEG]  - 1;
          if (IEND < IBEG) IEND = NLEN + 1;
          PARTLN[NPARTS] = IEND - PARTBG[NPARTS];
          PARTLNO[NPARTS] = IEND - PARTBG[NPARTS];
          if (IEND >= NLEN) goto S5200;
          IBEG   = IEND + 1;
          goto S5110;
/*C******* CHECK EACH PART AGAINST THE STANDARD SPELLING TABLE.*/
 S5200: NEEDNO = FALSE;
      LOCNNO = 0;
      IDISST = 0;
      for (I=1;I<=NPARTS;I++)
      { //DO I = 1, NPARTS;
          PHONIC ( &NAME[PARTBG[I]] , &PARTLN[I]);
          _fstrncpy((char *) &PART, (char *) &NAME[PARTBG[I]],(size_t) PARTLN[I]-1);
          HASHF (ID,PART, &PARTID[I]);
      } //ENDDO;
      for (I=1;I<=NPARTS;I++)
      { //DO I = 1, NPARTS;    
          if (PARTID[I] != 0)
          { 
              if (!STSNC2->NAMABV[PARTID[I]]) 
                  PARTID[I] = 0;
              else 
                  PARTID[I] = STSNC2->STSPLC[PARTID[I]];
          }  
      } //ENDDO;
/*C******* GENERATE THE STANDARD NAME*/
      LENGTH = 0;
      for (I=1;I<=NPARTS;I++)
      { //DO I = 1, NPARTS;    
          if (PARTID[I] == 0) 
          { 
          //    STDNAM[LENGTH+1:LENGTH+PARTLN[I]] = NAME[PARTBG[I]];
             _fstrncpy((char *) &STDNAM[LENGTH+1], (char *) &NAME[PARTBG[I]] ,(size_t)( LENGTH+PARTLN[I]));
              LENGTH += PARTLN[I];
          } 
          else 
          {
           //   STDNAM[LENGTH+1:LENGTH+STSPLN[PARTID[I]]] =     ARRAY[PARTID[I]];
              _fstrncpy(&STDNAM[LENGTH+1], &ARRAY[PARTID[I]*13],(size_t) LENGTH+STSNC2->STSPLN[PARTID[I]])       ;
              LENGTH = LENGTH + STSNC2->STSPLN[PARTID[I]];
          }   
      } //ENDDO;
S1500: return; 
   }   
   
/*****************************************************************************************************/
BOOL   STNDSN_INIT(BOOL ReInit)
{ 
    char TestString[13];
    short i; 
    
    if (ReInit) 
    {
    	STNDSN_CLEAR ();
    	INIT_CALL=TRUE; 
    }
    if(!INIT_CALL)return TRUE;   
    INIT_CALL = FALSE;
    FIRST  = FALSE;   
    SetAddressDir();

Start:    
    _fstrcpy (STND_PATH,AddMatchDir); 
    _fstrcat (STND_PATH, "\\stndsn.hsh");
    STNDLN = _fstrlen(STND_PATH);      
    
    if (!ExistFile (STND_PATH))
    {
    	if (LoadAbbreviations (AddMatchDir))
    		goto Start;
    	else
    		return FALSE;
    } 
       
       
      HASHS(&ID, 0L, 0L, 0L,(char *)STND_PATH,(long)STNDLN,"READ");
      HASHA (ID, &ARRAY);
     // ARRAY = STSNC1;
      _fstrncpy(&STND_PATH[STNDLN-3], "ARY",4);
     

// extern char  *MS_MAPL(LPSTR STND_PATH,  short STNDLN, long j, long array_ln, short MS_NR_XOR_1W, 
//                                        short MS_R, BOOL tf , long *LM, long *TAT);

      STSNC2 = (STS) MS_MAPL ((LPSTR) STND_PATH, (short) STNDLN , 0, (long)( a1511*(sizeof(BOOL16)+sizeof(BOOL16)+sizeof(short)+sizeof(short))),
                         MS_NR_XOR_1W, MS_R,TRUE, &LM, &ISTAT);
      if(ISTAT < 1)MessageBox(0,"ERROR IN INIT_STNDSN","std.c",MB_OK);
      _fstrncpy(TestString,"NO",13);
      HASHF (ID,TestString,&NOLOC);
      _fstrncpy(TestString,"#",13);
      HASHF (ID,TestString,&NUMLOC);
      _fstrncpy(TestString,"N",13);

      HASHF (ID,TestString,&NORTH);
      _fstrncpy(TestString,"STATE",13);
      HASHF (ID,TestString,&ISTATE); 
      _fstrncpy(TestString,"HWY",13);
      HASHF (ID,TestString,&HWYLOC); 
      
      hSTDNAMES = GSSiGlobAlloc(GAIDNO 395,GMEM_MOVEABLE,41*14);
      STDNAMv=GlobalLock(hSTDNAMES);
      NRONAMv=STDNAMv+41;
      NMONLYv=NRONAMv+41;
      SANSCHv=NMONLYv+41;
      NANDCHv=SANSCHv+41;
      NCMPNMv=NANDCHv+41;
      ORIGNMv=NCMPNMv+41;
      SANSCPv=ORIGNMv+41;
      SANSCSv=SANSCPv+41;
      PREFIXv=SANSCSv+41;
      NAMEv=PREFIXv+41;
      TYPEv=NAMEv+41;
      SUFFIXv=TYPEv+41;	

    // INIT_CALL = FALSE;
    //  if (INIT_CALL) return; 
     
//   INLEN =_fstrlen(INNAME);    
  
      return TRUE; 
  }  
  
BOOL GetSTDNamePart (LPSTR Name,LPSTR Type,LPSTR Part)
{
STNDSN_INIT(FALSE);
    STNDST(Name, (short)_fstrlen(Name),STDNAMv,NRONAMv,NMONLYv,
                             SANSCHv,NANDCHv,NCMPNMv,ORIGNMv,SANSCPv,SANSCSv,PREFIXv,NAMEv,TYPEv,SUFFIXv);   
	if (!_fstricmp (Type,"STDNAM"))
	{
		_fstrcpy (Part,STDNAMv);
		return TRUE;
	} 
	if (!_fstricmp (Type,"PREFIX"))
	{
		_fstrcpy (Part,PREFIXv);
		return TRUE;
	} 
	if (!_fstricmp (Type,"NAME"))
	{
		_fstrcpy (Part,NAMEv);
		return TRUE;
	} 
	if (!_fstricmp (Type,"TYPE"))
	{
		_fstrcpy (Part,TYPEv);
		return TRUE;
	} 
	if (!_fstricmp (Type,"SUFFIX"))
	{
		_fstrcpy (Part,SUFFIXv);
		return TRUE;
	} 
	*Part = 0;
	return FALSE;
}
  
   
/*C*    {***********************************************************************/
/* A former entry point  */
 /*C*    {***********************************************************************/
void   STNDSN_CLEAR(void)
{
  long stat;       
      HASHC (ID);
      MS_UNMAP ((char *)STSNC2,LM,&stat);
      FIRST = TRUE;
      INIT_CALL = TRUE;
      ID = 0;
      GSSiGlobUlFree (&hSTDNAMES);
      return; 
}      
/**************************************************/      
/*void  REPLAC(char  *mess, long *len, char cthis[], char with_this[])
{                             
char *the_end;    
LPSTR  loc_it;
long  len_this, len_with_this, idiff, whats_left ;
     
      len_this = _fstrlen (cthis);
      len_with_this = _fstrlen(with_this);
      idiff = len_this - len_with_this; 
      the_end =    mess + *len - 1 ; 
      
r10:  loc_it = _fstrstr (mess, cthis);
 
      while (loc_it!= NULL)                                                      
      {
        whats_left = (long)(the_end - loc_it);
        if(idiff < 0)
        { // the new stuff is longer... i gotta make more room
          _fmemmove (loc_it+idiff,loc_it,(size_t)(whats_left));
          the_end -= idiff;
        }
        else
        {
          if(idiff > 0)
          { // the new stuff is shorter... I gotta shrink it up
            _fmemmove (loc_it,loc_it+idiff,(size_t)(whats_left-idiff));
            the_end -= idiff;
          }
        } 
//        ok, I'm ready to place the new stuff in the mess
          _fstrncpy (loc_it,with_this,(size_t)len_with_this);
          goto r10;  
      } 
      *len = _fstrlen( mess )  ;     
return;
}      */
/*********************************************************************/
/****************************************************/
/************************************************************************/

 void  PHONIC (LPSTR NAME, LPSHORT NLEN)
{ 
 char LASTCHAR, *cp, *next, *last;
 long   NEWLEN, I; 
 LPSTR	lpName2;
 
   if (*NLEN <= 1)return;
// strip leading zeros 
	lpName2 = NAME + 1;
	while (*NAME == '0')
		_fmemmove (NAME,lpName2,(*NLEN)--);
 
/*C******* CHANGE EY SUFFIX TO Y */
        REPLAC (NAME,"EY ","Y ",*NLEN);
/*C******* CHANGE IE SUFFIX TO Y*/
        REPLAC (NAME,"IE ","Y ",*NLEN);  
        
/*C****** CHANGE AE SUFFIX TO AY*/
        REPLAC (NAME,"AE ","AY ",*NLEN);;
/*C******* CHANGE PH TO F*/
        REPLAC (NAME,"PH","F",*NLEN);
/*C******* REMOVE REPEATING, ADJACENT CHARACTERS IF NOT NUMERIC*/
      cp = NAME; 
      next = cp + 1;
      *NLEN = _fstrlen(NAME);
      if (*NLEN <= 2)return;
      while(*next != '\0')
      {
        if(*cp >= '0' && *cp <= '9')
        {//found a number
          cp++;
          next++;
        } 
        else
        {
          if(*cp == *next)//got a adjacent repeating character
          {
            last   = _fstrchr(cp,'\0');
            _fmemmove(cp,next,(size_t)(last - cp));
          }
          else
          {
             cp++;
             next++;
          }
        }
      }
      if(*cp == ' ') *cp = '\0';
      *NLEN = _fstrlen(NAME);
      return;
}      
/*          if (*NAME  < '0' || *NAME  > '9')
               LASTCHAR = *NAME ;
           else
                LASTCHAR = ' ';
          NEWLEN = 0;
          NAME++; 
          for (I = 1; I < *NLEN ; I++)
          {
            if ( *NAME   != LASTCHAR)
            {   
                NEWLEN++;
                *(&cp[NEWLEN]) =  *NAME ;
                if (  *NAME  < '0' ||   *NAME > '9')
                    LASTCHAR =  *NAME ;
                 else
                    LASTCHAR = ' '; 
             }       
            NAME++;

           } //end of the for loop  9         CONTINUE
          *NLEN = NEWLEN +1 ; 
          *(&cp[NEWLEN+1]) = '\0';
          NAME = cp;*/
 /************************************************************************/
//*****************************************************************
/*void a_batch(void)
{
FILE *fptr, *fptr2;
char inname[41], stdnam[41], nronam[41], nmonly[41], sansch[41], nandch[41], ncmpnm[41], orignm[41], sanscp[41], sanscs[41];
LPSTR INNAME = inname,  STDNAM = stdnam,    NRONAM = nronam,   NMONLY = nmonly,
     SANSCH = sansch,   NANDCH = nandch,      NCMPNM = ncmpnm,     ORIGNM = orignm, SANSCP = sanscp, SANSCS = sanscs  ;  
char convert[35], match_this[21], *pchar; 
short INLEN;
fptr = fopen("..\\addr\\street.txt","rt");
fptr2 = fopen("..\\addr\\output.txt", "wt");

        while(!feof(fptr))
{                  
  pchar = fgets(convert, 34, fptr);
      pchar = fgets(match_this, 34, fptr); 
      INLEN = (short) _fstrlen(convert); 
STNDSN_INIT(FALSE);
      STNDST( convert ,  INLEN,  STDNAM,   NRONAM, NMONLY,
                        SANSCH,  NANDCH,   NCMPNM,   ORIGNM, SANSCP, SANSCS); 
      fputs(STDNAM, fptr2);
      fputs("  ",fptr2);
      fputs(match_this,fptr2);
      
} 
   fclose(fptr);
   fclose(fptr2);    
return;
} */

BOOL LoadAbbreviations(LPSTR AddressDir)
{    
     BOOL NABV, compass, initial_load = TRUE, FirstLine=TRUE;
      long   STNDSP, LOC, IOFF, NUM, AR,  LM=0, ISTAT;
      char file2[MAX_PATH];
      char VALS[88], ABREV[13],  outval[104]  ;  
      OFSTRUCTGM OF;
      HFILE fptr;
      UINT pchar;
	  int	ID;
      short	NMLEN,  MS_ = 0, i,  lengths[8],LENABV ;
//      short MS_NR_XOR_1W,  MS_R=1, MS_RW = 2;
      long j, got_len, num_of_em = 0;
      DWORD	Err;
	  int	nloaded=0;
	  int	wanti=31, ii;
      
typedef struct 
      {
        BOOL16  COMPAS[a1511],
              NAMABV[a1511];
        short STSPLN[a1511],
              STSPLC[a1511];
           } lda55;
typedef lda55 far *NPntr;
NPntr NPNT; 
HANDLE hGlobal;
VARPNT VarPnt;
          _fstrcpy(file2,AddressDir);
          _fstrcat (file2, "\\AbbrName.txt");
       
       if (!ExistFile (file2))
       {
       		GSSiMakeDir (AddressDir,&Err);
       		copyfile (file2,"[%INDIR]address\\abbrname.txt",FALSE,0,0,0,0,0,0);                          
       }
        fptr = GSSiOpenFile(file2,&OF,OF_READ); 
       if(fptr == HFILE_ERROR)
       {
          MessageBox(GetFocus(),"Load Abbreviations *** ERROR *** Unable to open input file",
          file2, MB_ICONSTOP);
          return FALSE;
       } 
       
       IOFF = 0; 
     //  initial_load = FALSE;
       AR = a1511*(sizeof(BOOL16)+sizeof(BOOL16)+sizeof(short)+sizeof(short));
       _fstrcpy(file2,AddressDir); 
       _fstrcat (file2, "\\stndsn.hsh");

       if(initial_load)
         HASHS(&ID, 13, 13, a1511,file2, (short)_fstrlen(file2), "NEW"); 
       else   
         HASHS(&ID,0, 0, 0,file2, (short)_fstrlen(file2), "UPDATE");  
       if(ID == 0)
       {
          MessageBox(GetFocus(),"*** Error *** Unable to get memory for MS_CRMAPL",
              "Load Abbreviations",MB_ICONSTOP);
         return FALSE;
       }
       _fstrcpy(file2,AddressDir);
       _fstrcat(file2,"\\stndsn.ary"); 
       if(initial_load)
       { 
         NPNT = (NPntr)MS_CRMAPL(file2, (short)_fstrlen(file2),
                                  IOFF,  AR,  MS_, MS_WR,    &ISTAT);
         NABV = FALSE;
         compass = TRUE;
       }
       else                            
       {
         NPNT = (NPntr) MS_MAPL (file2, (short)_fstrlen(file2) ,
                                  0, AR, MS_NR_XOR_1W, MS_WR, TRUE, &LM, &ISTAT);
         NABV = TRUE;
         compass = FALSE;
       } 
       if(NPNT == NULL)
       {
          MessageBox(GetFocus(),"*** Error *** Unable to get memory for MS_CRMAPL",
              "Load Abbreviations",MB_ICONSTOP);
          return FALSE;
       }   

        pchar = 82; 
    while(pchar == 82)                                               
    {                  
s12:  if (!fgetstring (VALS,86,fptr))
		goto s14;
        //  _fstrset(outval,' ');  <- stops filling at the first \0 it finds
      //  for (j = 0; j <= 87; outval[j] = ' ', j++ ); 
        VALS[80] = '\0';
        got_len = _fstrlen(VALS);    
        if(VALS[0]== '*')//found the line dividing compass heading from street names
        {   
        	if (!FirstLine)
			{
				if (!compass)
					NABV = TRUE;
	            compass = FALSE;
			}
	        FirstLine=FALSE;
            goto s12; 
        } 
		nloaded++;
		if (wanti ==  nloaded)
			ii=0;
        NUM = 0;   
        for (i = 0; i < got_len; i+=12)
        {                                    
          lengths[i/12] = 0;
          if(got_len - i < 12)
             LENABV = (got_len - i);
          else 
            LENABV = 12;
           
          _fmemmove(ABREV, &VALS[i],(size_t) LENABV);
          ABREV[LENABV] = '\0';  
          
          PHONIC(ABREV, &LENABV); 
          //CHECK FOR BLANK LINES
          if ((LENABV == 0 || _fstrcmp(ABREV," ") == 0) && NUM == 0 ) goto s12;
          if (LENABV == 0 || _fstrcmp(ABREV," ") == 0) goto s13;

          NUM++;
        //  for (j = LENABV; j <= 12;  ABREV[j] = '\0', j++ );
        //  ABREV[12] = '\0'; //fatten out the name
         //HASHF(ID, ABREV, &LOC); 
          _fmemmove(&outval[(i/12)*13], ABREV, 13);
          lengths[i/12] = (short) LENABV; 
          lengths[(i+12)/12] = 0;
        } //end of the for loop
            
s13:  HASHP	(ID, &outval[13], &STNDSP);
      num_of_em++;
        for (i = 0,j = 0; j < NUM; j++, i+=13)
        {  
           if(lengths[j] == 0) break;
            HASHP(ID, &outval[i], &LOC);
            NPNT->COMPAS[LOC] = compass;
            NPNT->NAMABV[LOC] = NABV;
            NPNT->STSPLN[LOC] = lengths[j];
            NPNT->STSPLC[LOC] = (short) STNDSP ;        
            if(num_of_em % 10 == 0)
            {
              num_of_em++;
            }   
            
        } //end of the for loop
 
}  //end of the while (!feof(fptr))  

s14:  GSSiClose2 (&fptr);
      HASHC(ID);
      MS_UNMAP((char *)NPNT,LM,&ISTAT);
return TRUE;
}

