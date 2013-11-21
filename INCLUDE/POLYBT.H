extern int PolyBTGetThis (const HGLOBAL bid6,LPSTR record);
extern int PolyBTGetFirst (const HGLOBAL bid1,LPSTR record);
extern int PolyBTClose (HGLOBAL *bid3);
extern int PolyBTClear (const HGLOBAL bid4);
extern HGLOBAL PolyBTInit (short lrec,short nkeyfld,short fldtyp[8],short fldlen[8]);
extern int PolyBTPut(const HGLOBAL bid, LPSTR rec);
extern int PolyBTGetNext (const HGLOBAL bid2,LPSTR record);