
      #define ET_RCL  26
      #define DEF_MX_CORD_LENGTH  0e0
      #define DEF_MX_CORD_ANGLE   3e1 //{Max. angle for arc cords
      #define DEF_MX_CORD_ANGLE_VAR 5e0 //{Max. variation in arc cord angles
      
      #define ULIM_MX_CORD_ANGLE  45e0      // {Upper limit on max. cord angle
      #define LLIM_MX_CORD_ANGLE  0e0       // {Lower limit on max. cord angle
      #define ULIM_MX_CORD_ANGLE_VAR  15e0  // {Upper limit on max. cord angle variation
      #define LLIM_MX_CORD_ANGLE_VAR  0e0   // {Lower limit on max. cord angle variation   
      
/*
*       **********************************
*       MSDICT COMMON  - SW
*       **********************************
*
******** CONTAINS VARIABLES FOR primary dictionary mapping
*        new consolidated dictionary only
        lda note: the following structure is left commented out
        for informational purposes only */ 
/*typedef struct
   {
      long   DICT_LEN, REC_INDEX[3200], END_ARRAY;
      short  REC_ARRAY, REC_TO_DESC[3200], DESC_TO_REC[3851], DHNX, DHN,
             REC_TO_PNUM[3200], REC_TO_UOM[3200];
      char   EDSC[3851][9];
   }  MSDict;
typedef MSDict far *lpMSDict; */
typedef struct
   {
         int  Flags,
              Align,
              NumPts,
              Color,
              GeoSymNum;
         char GeoSymbol[32],
              Name[32];
   } PtDesc; 
typedef PtDesc far *lpPtDesc;     
typedef struct
   {
     char  Name[64];
         int  Num49s,
              Flags,
              NumLines,
              Color,
              Align,
              GeoSymNum;
        float TotLen,
              DashLen[24];
         char Pattern[96],
              GeoSymbol[32];
   } lType; 
typedef lType far *lplType;   

typedef struct
   {  
      long far  *lpREC;
      long      DICT_LEN, END_ARRAY;
      short far *lpREC_TO_DESC, *lpDESC_TO_REC,
                *lpREC_TO_PNUM, *lpREC_TO_UOM;
      short     DHNX, DHN, REC_ARRAY;
      char far  *lpEDSC;
      char      EDSC[9];
   }  MSDict;
typedef MSDict far *lpMSDict; 
/*      EQUIVALENCE
     +            (ET_RECORD(1),  DXF_TYPE)
     +          , (ET_RECORD(2),  DXF_SYMBOL)
     +          , (ET_RECORD(12), DXF_LAYER)
     +          , (ET_RECORD(22), UM_SYMBOL)
     +          , (ET_RECORD(26), UM_NDESC)
    */  
 /*     POINTER/ENTRY_PNTR/DICT_LEN,END_ARRAY,DHNX,EDSC(3851),
     +                   DESC_TO_REC(3851),REC_TO_DESC(3200),
     +                   REC_TO_PNUM(3200),REC_TO_UOM(3200),
     +                   REC_INDEX(3200),REC_ARRAY(1)
      COMMON/MSDICT/ENTRY_PNTR,DHN  */
/*C   
C*        DXFIN_ARC_CONTROL_VARS common block contains variables which control
C*        the automatic creation of arcs where appropriate from 3 or more line
C*        segments of a polyline for the DXF input translator.  The user can
C*        control these variables with the command line option -CF (curve fit).
C*
C*        AUTHOR: Jerry Magin  Aug 1992 */


typedef struct
   {
        BOOL          INBLOCK, DONE, GR_CHANGE[80], BLOCKS,
                      ENTITIES, NEST,  UM_INIT,
                      CHECK, CONTINUE_POLY, POLY_CLOSE,
                      Ignore, HEADER, DB, 
                      ARG_MISSING, FLAG,
                      CURVE_FIT_OPTN,      //{Set true if curve_fit option enabled.                          , CURVE_FIT_OPTN     */
                      TINVERT, 
                      PREPROCESSING;

        double         XY[4][1000], BULGE[1000], ROTATION, THICK, 
                       ELEV, RADIUS,  TSIZE, STANG, ENDANG, 
                       TSZFAC,  ARC[4][4],  PI, CX, CY, DIS, AZM,
                       Radian,  SCALE, OFFSET[3],
                       SWEEP,   SYM_SCALE,   ANGLE[10], 
                       FL_VALUE[10], XXX[10], YYY[10], ZZZ[10],
                       EXTENTS[3][3], TPL[3][3];
                       
      double MX_CORD_ANGLE;          //{Max. angle for arc cords
      double MX_CORD_ANGLE_VAR;      //{Max. variation in arc cord angles
      double MX_CORD_LENGTH;         //{Max. length for arc cords

        long          NV, COLOR, GRCODE, CNTR, I_VALUE[9], IST, IST2,
                      ET_RECORD_LOC,  ET_RECORD_ADDR[1000], LINE_COUNT;
        unsigned long REFNO2;              

        short         GR_CHANGE_INDEX[80], N_GR_CHANGE, EtRcl,
                      TJUST, NTXT, TFONT, LATTVAL, ET_HASHID, ET_NUM,
                      I_NTXT;
        char      CSTRING[256], INFILE[128], TXT_STYLE[68],  LTYPE[64],
                      CURRENT_SECTION[62], LAYER[68], BLNAME[64],
                      ENLAYER[64], ATTVAL[256], LINE[256],
                      TEXT[260], ATTAG[64],  COMMAND[64], DUMMY[264],
                      ATTFILE[128], GR_DATA[40][80],OUT_SYM[64],
                      VARNAME[68], NAME[4][96],  
                      ErrMsg[64], MESS[256], C1000[256];
   } DxfToPC;
typedef DxfToPC far *lpDxfToPC;

typedef struct
  {
     unsigned long REFNO, REFINC;
      char   GMRFILE[100], TABLEFILE[100], OUTFILE[100], 
             INFILE[100], NEW_OLD[4];
  } DxfOptions;

typedef DxfOptions far *lpDxfOptions;  

//lda note: Found these two types of equivalence statements
//I'm guessing they're used in different routines to 
//retrieve data from different locations.


    

 /*          EQUIVALENCE
     +            (ET_RECORD(1),  DXF_TYPE)
     +          , (ET_RECORD(2),  UM_SYMBOL)
     +          , (ET_RECORD(6),  DXF_SYMBOL)
     +          , (ET_RECORD(16), DXF_LAYER)
*/
typedef struct
   {
     char Text[256];
     unsigned short NumText;
     double X,Y,Rot,Tpl[4],Size, HScale, VScale,Italics;
     long FColor, BColor;
     int  Font, Just;
     char FontName[32];
     BOOL Invert;
   } LetterStr;
   
typedef LetterStr far *lpLetterStr;
typedef struct
   {
     long Refno;
     int NumLetterStr;
     lpLetterStr lpLtrStr;
   }  TxtStr;
typedef TxtStr far *lpTxtStr;   
typedef struct
   {
      double XY[160], TEXT_SIZE;
      long   TYPE, TEXT_NCHRS, TEXT_JUST;
      char  DESCRIPTION[8], TEXT_STRING[388];
   } DxfOutData;
typedef DxfOutData far *lpDxfOutData;
   
 /*     COMMON /DXFOUT_DATA/ TYPE, DESCRIPTION, XY
     +         , TEXT_NCHRS, TEXT_STRING, TEXT_SIZE
     +         , TEXT_JUST */

void DxfIn(void);
  
void  DxfCloseMem(void);
void  dxfin_doline(void);
void  dxfin_dopoint(LPDPOINT LPD);
short dxfin_set_symbol(const char L);
void  dxfin_docircle(LINE *l);
void  dxfin_dosolid(LPDPOINT LPD);
void  dxfin_doarc(LINE *l);
void  dxfin_dotrace(LPDPOINT LPD);
void  dxfin_dotext(void);
void  dxfin_doinsert(LPDPOINT LPD);
void  dxfin_doattrib(lpLetterStr lpLtrStr);
void  dxfin_dopoly(BOOL *dopoly,  long *i); 
void  dxfin_check_combos(char TYPE,long *IST);
void  dxfin_cvt_3pt_arc(double **XY, long *IST);
void  dxfin_grcode_loop(long *IST);
void  dxfin_get_options(long *IST);
void  dxfin_load_symbol_table(char *TABLE_FNAME,long *IST);
void  dxfin_header_substitute(long *IST);
void  dxfin_cmd_processor(char far *COMMAND,BOOL FLAG,long *IST);
BOOL  dxfin_header_processor(char *COMMAND,BOOL FLAG,long *IST);
void  DxfInTableProcessor(char *COMMAND,BOOL FLAG,long *IST);
void  dxfin_list_combos(long *IST);
void  UM_ACTIVATE_WRITE_TO_DB(void);
void UM_INIT_SGR(char *NEW_OLD,unsigned long REFNO, 
               long REFINC, char *OUTFILE, double SCALE,
               char *SGRFILE, double *EXTENTS,long *IST);
void UM_CLOSE_SGR(unsigned long LAST_REFN,long NUM_RECS,long *ISTX); 
void UM_CLOSE_DB(void); 
void CLOSE_SYMBOL_EXCHANGE_TABLE(long *ISTX);
void CloseDxfMem();

lpDxfOptions      DxfInitActions(void);

