typedef struct 
    { 
      double x,y;
    } DPoint;
typedef DPoint far *lpDPoint; 

typedef struct 
    { DPoint F,
             T;
      long Refn;
      double lngth;
      unsigned int Desc, ID, Type;
    } DLine;
typedef DLine far *lpDLine;

typedef struct
    { unsigned int NumPts, NumSides;
      int type, whos_callen, LinkDesc,fillet_desc;
      double offset_dist;
      lpDLine lpDLineBase;
      lpDPoint lpPtBase;
    } Area;
typedef Area far *lpArea;
         
typedef struct
    {unsigned int NumSides, Type, Desc;
     COLORREF color;
     HDC hDC;
     double MinX, MinY, MaxX, MaxY, CentX, CentY;
     lpLine Lines;
    } LArea;
typedef LArea far *lpLArea;

