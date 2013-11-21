
 void  OffsetEliminator(int node);
 int OffsetMessageBox (HWND hWnd,LPSTR Text,LPSTR Title,UINT Style);   

 void  offset_find_perimeter(long *ST);
 void  OFFSET_MAIN(lpArea InArea, lpLArea *LA,long *ST);
 void  OffsetClose(void);
 void  OffsetGet(lpLArea *lpA, long *st);
 void  offset_merge_areas(BOOL *deletions,long *st);
 void  OffsetDeleteArea(int i);

 void  offset_validate_area(long *ST);
 void  offset_intx_these_two(lpWS lpLLast,lpWS lpL,
               double far **zx1, double far **zy1,double *gap,long *irc); 
 void  offset_intx_all_lines(lpArea lpa, BOOL one_side, long *st); 
 void  offset_delete_area(long island_ref);
 void  offset_validate_area(long *st);
 void  offset_delete_area(long i);
  int  OffsetThese( lpArea lpA);
  int  OffsetCreateFillet (lpWS lpFrom,lpWS lpTo,lpWS lpSpare, 
               int whos_callen,int fillet_desc, 
               BOOL one_side); 
  int  OffsetAlignment(lpArea lpA) ;
 void  offset_find_link_partners(long *st);
 int   offset_check_coincident(lpWS lpWork,lpWS lpNext);
 
  
 int   PolyFindNextNode(double *sx,double *sy,char direction); 
 int   PolyEliminateDeadends(void) ;
 int   DeleteNode(int iway);
 BOOL  poly_load_lda_coords(void);
 BOOL  PolyPurge(void)  ;
 int   PolyAreas(void);
 void  PolyMinValidLine(void);
 
 void  PolyTieBreaker(double *a3, double *a2, long L1, long L2);
 void  PolyFindOcean(long *island_ref,long *ocean_ref);
double PolyCalcDeltaAzm(double *old_azm, BOOL old_grain,
                        double *azimuth, BOOL the_grain,
                        double *smallest_change);
 int   PolyFindStart(BOOL *true,double *sx,double *sy);
 void  poly_check_for_islands(void);
 void  poly_bt_close(short id1);
 void  poly_bt_init (short *id1,short int238,short nkeyfld, short *fldtyp,short *fldlen);
 void  poly_bt_clear (short *id1);
 int   poly_load_lda_line(double x1,double y1,lpWS lpWork,lpWS lpNext,short int2);
 int   PolyEliminateCoincidence(lpWS lpWork,lpWS lpNext,double **x1,double **y1,int ireturn);
 int   PolyEliminator(int ibad);
 void  set_alignment_ends_square(void);
 void  set_left_side_only(void);
 void  set_right_side_only(void);
 void  hashc(short id2);
 void  hashf(short id3,char *keyword, long *loc);
 void  hashg(short id3,char *keyword,long loc);
 void OFFSET_MAIN(lpArea InArea, lpLArea *OutArea, long *ST);
 void OffsetClose(void);
 void SetCurvPltCtol (double INCTOL);
 int  CURVPLT(lpLine L1,LPLONG np, HPDPOINT *lpPoint);
 void OffsetCloseOutArea(void);
 int ArcToPoints(lpLine L1,int *np, LPDPOINT *lpPoint);
#define radical 3201
