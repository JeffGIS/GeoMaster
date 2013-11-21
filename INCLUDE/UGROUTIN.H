/***********************************************************

  Common General Purpose Routines

************************************************************/

void DrawBorder(HDC hdc,RECT far *rect,int thick,int updn);

int CopyCellInfo(UG_CellInfo *src,UG_CellInfo *dest);
UG_CellInfo * CreateCellInfo();
void DeleteCellInfo(UG_CellInfo * cell);