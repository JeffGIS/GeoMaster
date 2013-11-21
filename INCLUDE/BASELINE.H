#include <windows.h>
#define MaxTextLen 128 
                   // set it to what you want
//***************************************************//

typedef struct tagCurve
{
  double  Radx, 
          Rady, 
          CLen,     // length of the curve in radians
          RadLen,   // distance from radius to the curve
          RtoPCAzm; // rad to PC azimuth
  HGLOBAL HeapPtr;
} Curve;
typedef Curve FAR* LPCurve;

//*****************************************************//
typedef struct tagText
{ 
  LOGFONT HFont;
  HPEN    HPen;
  LPSTR   pText;
  HGLOBAL HeapPtr; 
  char    Text[MaxTextLen];
} Text;
typedef Text FAR *LPText;
 
//************************************************************//
typedef struct tagLine
{
  double BX,   BY, 
         EX,   EY;
     //  MinX, MinY, //don't know as we really need these cuz
     //  MaxX, MaxY; //its so easy to use min(BX,EX) and max(BX,EX)
  HPEN           HPen;
  unsigned long  Refno;                                 
  unsigned short Desc, 
                 ParentDesc; 
  unsigned  Width      :6; // Pen weight
  unsigned  Valid      :2; // is this line valid for what I want to use it for? [ t | f ]
  unsigned  Type       :2; // point, line, curve
  unsigned  tfText     :1; // text present? [ true | false ]
  unsigned  tfVisible  :1; // is it visible? [ true | false ]
  unsigned  tfBlinking :1; // is it blinking? [ true | false ]
  unsigned  tfEnabled  :1; // Pickable or Editable? [ true | false ] 
  unsigned  Changed    :2; //0 = no, 1 = yes, 3 = Deleted
  LPCurve   Rad;           //leave this out and we handle only straight lines
  LPText    Text;          //if NULL, no text present
} Line;
  typedef Line FAR *LPLine; 
  
//***********************************************************//
typedef struct tagArea
{
   double MinX, 
          MinY, 
          MaxX, 
          MaxY; 
   unsigned long  Refno;
   unsigned short Desc,
                  NumSides;
   LOGBRUSH BackColor;
   Text     Title;
   unsigned Pos_Neg  :1; //positive or negative area
   unsigned Clipped  :1; //it the area clipped by something?
   unsigned Visible  :1; //is it visible?
   unsigned Blinking :1; //is it blinking?
   unsigned Enabled  :1; //is it pickable?
   unsigned Changed  :2; //0 = no, 1 = yes, 3 = deleted
   unsigned Valid    :1; //is this a valid area?
   unsigned Type     :8; //area types (256 different types)
} Area;
typedef Area FAR *LPArea;

//************************************************************//
typedef struct tagLTable
{
  unsigned short   TOT,  LMAX;
  BOOL    Text,
          Curves;
  LPLine  Side;  //use to move around in Sides     
  LPSTR   Sides; //points to the chunk of memory ((TOT+20) * sizeof(Line))
  LPArea  AInfo;
  HGLOBAL TableHeapPtr,
          SidesHeapPtr,
          AInfoHeapPtr;
} LTable;
typedef LTable FAR *LPLTable; 





// Address information necessary for each line
// could be added to the table structure as well 
//***************************************
typedef struct tagAddr
{
   long   IntxStr1, 
          IntxStr2,
          Munic, 
          House, 
          Street, 
          InMmunic,
          VarPnts[12], 
          Zip;
   short  IntxNo, 
          EndCode;
   double PtOff[2], 
          IntxTol;
   HFONT  FontInfo;     
} Addr;
typedef Addr FAR *LPAddress;