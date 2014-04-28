#include "graphint.h"      
#include "hash.h"   
#include "dxfcom.h"

#define ftm 1.0
#define FILL 0
#define NOFILL 1 
#define ARCPCPOCPT 3

#include "gmextern.h"

static	BOOL	HiPrecis=TRUE;
static	HANDLE	hDXFLayers=0;
static	short   NumSyms=0;  
static  HANDLE	hSymDesc=0; 
static	DPOINT	LWPoint[4096];
static	long	CurLWPoint=0, DebugItem=32561;
static char DXFTABLE[64],NAME[128],  LAYER[24], OUTNOTES[64],DXFOutFile[64],ATTDEFS[32][24];
static BOOL Importing,  BlocksOnly, Gotta1000, Hold1000;  
static HANDLE hTranFile=0;
static HANDLE LineMem = 0, PointMem = 0;
static char DefaultPrefix[32], DXFPrefix[128], DXFUDI[128];
static	short  ET_RECORD[26], NumAttDefs;
static	BOOL	IgnoreExtents, SymConvTableChanged,ScanForExtents;  
static	BOOL	HaveIgnore;   
static HANDLE hDxf[12]={0,0,0,0,0,0,0,0,0,0,0,0};
static	char	IgnoreType[5][12]={"MTEXT","LEADER","ELLIPSE","SPLINE","SHAPE"};
static char GeoSymName[32], GeoLineName[32], GeoHashString[50], LastDxfSymName[32];
static	HANDLE	hLayerSym=0;
static	short	nLayerSym;  
static 	BOOL	UniqueRefno;
static 	long	StartRefno;   
static	double	DXFTextSizeAdjust=1.4;
static double	ARCTOL=5e-1;
static HFILE	DXFDXF;
static HWND	hWnd;
static BOOL	HaveExtents=FALSE;
static OFSTRUCTGM	OBF;
static OFSTRUCTGM	OUTF;
static LPOFSTRUCTGM	lpOBF = &OBF;
static LPOFSTRUCTGM	lpOUTF = &OUTF;
static short	Hid=0;
static double	SCALE=1e0;
static double	TSZFAC=1e0;
static double	REFNO=1e0;
static double	REFINC=0e0;
static double	XOffset;
static double	YOffset;
static char	*DXF_TYPE=(char*)&ET_RECORD[1];
static char	*DXF_SYMBOL=(char*)&ET_RECORD[2];
static char	*DXF_LAYER=(char*)&ET_RECORD[12];
static char	*UM_SYMBOL=(char*)&ET_RECORD[22];
static LPSTR	cptr;
static short	*UM_NDESC=&ET_RECORD[26];
static int	nStore;
static BOOL	AutoDXFSymbols;
static short	NumDXFLayers=0;
static short	idesc;
static short	wantdesc;
static long	lineno;
static LPINT	lpnStore=&nStore;
static short	EntitiesChecked;
static short	BlocksChecked;
static short	HeaderChecked;
static short	CurveFitting;
static short	ContentsOnly;
static short	NumLines;
static short	NumPoints;
static double	DXFTextScale;
static double	DXFScale;
static long	PrimeRefno;
static long	RefnoInc;
static long	DXFOffset;
static long	NewRefno;
static long	CurLoc;
static long	ItemBegin;
static long	HashLoc;
static lpPtDesc	lpPointDesc;
static lpPtDesc	lpPtStart;
static lplType	lpLT;
static lplType	lpStart;
static BOOL	LinkDXFToPLT;
static BOOL	DoInserts=TRUE;
static BOOL	DoAttributes=TRUE;
static MNMXCORD	Bounds;
static short	GeoSymNum;
static short	GeoLineNum;
static short	PointLineCurve;
static lpDxfOptions	lpDxfOpts;
static lpDxfOutData	lpDxfOutput;
static lpMSDict	lpSymDict;
static lpDxfToPC	lpDxf2GM;
static LPSTR	KeepGoing;
static long	size;
static LPDPOINT	lpDpoint;
static LPDPOINT	lpPoly;
static short	NumCrvPts;
static int		nPoints;
static short	st;
static short	itype;
static short	SymType;
static HANDLE	heap_ptr=0;
static HANDLE	hPoly;
static LPHANDLE	lpHeapPtr=&heap_ptr;
static LINE	Lines;
static BOOL	Store;
static BOOL	GotTables;
static BOOL	GotHeader;
static BOOL	GotBlocks;
static BOOL	GotLines;
static HWND	DXFhWndDlg;
static BOOL	LinesOnly=FALSE;
static	long	Ignore[5];   

BOOL SkipToSeqend (void);
BOOL SkipToEndSec(HWND hWnd, long TotLen, LPSTR str);
short GetLayerSymNum (LPSTR LayerName, LPSTR Type);
int ArcToPoints(lpLine L1,int *np, LPDPOINT *lpPoint);

     int ArcToPoints(lpLine L1,int *np, LPDPOINT *lpPoint)
{
//C******* SPECIFICATIONS **********************************************
//C*                                                                   *
//C*       PROGRAM SUMMARY                                             *
//C*       ------- -------                                             *
//C*       THIS ROUTINE PLOTS A CURVE OF LENGTH (L) WITH BEGINNING     *
//C*       POINT OF (X1,Y1) AND RADIUS POINT OF (X2,Y2).               *

//  function return value: 0 if it added points to *heap_ptr
//                         -1 if it didn't do anything.

      double  DTH, A1, LenRad, LenCrv;
      POINT  StartPoint, EndPoint;
      DPOINT  RadPt, TestPoint,  HoldPt; 
      short size, Npts, l;
      DPOINT MyPt;
    //  XB=L1->x1; //X1;
    //  YB=L1->y1; //Y1;
//C******* COMPUTE RADIUS
     if( L1->rad <= 0) return -1;  
//C******* CALCULATE AZIMUTH OF BEGINNING & ENDING ARC RADII 

       /*     TestPoint.x = (CurView->WBounds.xmn + CurView->WBounds.xmx)/2.0;
            HoldPt.x    = TestPoint.x;
            TestPoint.y = (CurView->WBounds.ymn + CurView->WBounds.xmx)/2.0;
            StartPoint  = BasePtToWinPt (TestPoint); //center of the screen         
            TestPoint.x = HoldPt.x + L1->rad;
            EndPoint    = BasePtToWinPt(TestPoint);
            LenRad      = EndPoint.x - StartPoint.x;//length of rad in pixels  
            TestPoint.x = HoldPt.x + L1->lngth;
            EndPoint    = BasePtToWinPt(TestPoint);
            LenCrv      = EndPoint.x - StartPoint.x; //length of the curve in pixels 
            RadPt.x = L1->rx;
            RadPt.y = L1->ry;  */
            
//C******* CALCULATE AZIMUTH OF BEGINNING & ENDING ARC RADII
      A1 = L1->azm; //atan2(S,C);
             
      //A2 = L1->eazm;//A1 - L1->lngth / R;
      if(L1->rad <= FileDistToBaseDist) goto S110; 
    //  if(L1->lngth <= CTOL) goto S100;
      DTH = acos ((L1->rad - FileDistToBaseDist)/L1->rad);
       if(DTH == 0) goto S110;
//C******* CALCULATE NUMBER OF VECTORS IN CURVE
      Npts = (short)(fabs(L1->lngth) / (DTH * L1->rad));
      Npts++; // to plot at least the beginning of the curve
      if(L1->lngth < 0)DTH = -DTH;
      if(Npts < 2)goto S110;
//C******* PLOT REMAINING VECTORS
      for (l = 1;l < Npts;l++)
      {
         MyPt.x = L1->rad * cos(A1) + L1->rx;
         MyPt.y = L1->rad * sin(A1) + L1->ry;
        // MyPt.y = MyPt.y - CurView->DrawRect.top;
         **lpPoint = MyPt;
         *lpPoint = *lpPoint + 1;
         *np = *np + 1;
         A1 -=  DTH;
      }
      if(A1 != L1->eazm)
      {
        MyPt.x = L1->rad * cos(L1->eazm) + L1->rx;
        MyPt.y = L1->rad * sin(L1->eazm) + L1->ry;
        // MyPt.y = MyPt.y - CurView->DrawRect.top;
        **lpPoint = MyPt;
        *lpPoint = *lpPoint + 1;
        *np = *np + 1;
      }  
//C******* FINISH CURVE
 S110:  *lpPoint = *lpPoint - 1;

      return 0;     
} 

short GetLayerSymNum2 (lpDxfToPC lpDxf2GM)
{   
	short	idesc;
   	char	SymName[34], Command[34];
		        	
	_fstrcpy (SymName,lpDxf2GM->LAYER);
/*	if(lpDxf2GM->GR_CHANGE[2])    
    	_fstrcpy(SymName, lpDxf2GM->NAME[1]);*/
	if (!_fstricmp(lpDxf2GM->COMMAND,"INSERT"))
		_fstrcpy (Command,"POINT");
	else
		_fstrcpy (Command,lpDxf2GM->COMMAND); 
		
	idesc = GetLayerSymNum (SymName,Command); 
	return idesc;
}
 
BOOL AddPolyToMap2 (int nPolyIn, LPINT lpnPntsIn,LPHANDLE phDPoints,int Type, long NewRefno,int ipen, int idesc,LPSHORT Stuff,
					LPSTR Prefix, LPSTR UDI,long AreaColor,long PenColorIn, int PenWidth,BOOL HiPrecis)
{ 
    HANDLE	hPoints;
    LPDPOINT	pDpoints, pDpoints2;   
    int		nPnts, n2=2;
    
  	if (idesc>0)
		AddToSymList (idesc,&NumSyms,&hSymDesc);
    if (LinkDXFToPLT)
    	NewRefno = ItemBegin;
    if (!Type)
    {
    	if (*lpnPntsIn < 3)
    		Type = 1;
		else if (GetDictSymbolType (idesc) == 2) 
		{
			Type = 1; 
			 
			pDpoints = (LPDPOINT)GlobalLock (*phDPoints); 
			pDpoints[*lpnPntsIn] = *pDpoints;
			GlobalUnlock (*phDPoints);  
			(*lpnPntsIn)++;
		}
	}
  
	if (LinesOnly && nPolyIn == 1)
	{   
		hPoints = GSSiGlobAlloc ( 989,GMEM_MOVEABLE,2*sizeof(DPOINT));
		nPnts = *lpnPntsIn;
		pDpoints = (LPDPOINT)GlobalLock (*phDPoints);   
		nPnts--;
		while (nPnts--)
		{   
			pDpoints2 = (LPDPOINT)GlobalLock (hPoints);
			*pDpoints2++=*pDpoints++;
			*pDpoints2=*pDpoints;
			GlobalUnlock (hPoints);
			AddPolyToMap (nPolyIn,&n2,&hPoints,Type, GetNewRefno(PltName,0,0,0,0),0,ipen, idesc, Stuff,
						  Prefix, UDI,AreaColor,PenColorIn, PenWidth,0,0,0,0,HiPrecis,0);
		} 
		GlobalUnlock (*phDPoints);  
		return TRUE;
	}
	else
		return AddPolyToMap (nPolyIn,lpnPntsIn,phDPoints,Type, NewRefno,0,ipen, idesc, Stuff,
							 Prefix, UDI,AreaColor,PenColorIn, PenWidth,0,0,0,0,HiPrecis,0);

}

short GetLayerSymNum (LPSTR LayerName, LPSTR InType)
{   
	short	Choice=0, isym, SymType=1, i;
	LPSTR	lpTab, lpType;
	char	str[256], Type[32]; 
	HANDLE	hSymbol;
	LPSYMBOL	CurSymbol; 
typedef	struct	{char	Name[64], Type[16];
				 short	isym;}	LSREC;
typedef LSREC	FAR	*LPLSREC;
	LPLSREC	pRec; 
	
	if (!_fstricmp (LayerName,"SDASH3"))
		ii=1;
	if (!_fstricmp (InType,"POLYLINE")   ||
		!_fstricmp (InType,"LWPOLYLINE") ||
		!_fstricmp (InType,"ARC")		 ||
		!_fstricmp (InType,"TRACE")		 ||
		!_fstricmp (InType,"CIRCLE"))
		
		_fstrcpy (Type,"LINE");
	else
		_fstrcpy (Type,InType); 
	if (!hLayerSym)
	{
		hLayerSym = GSSiGlobAlloc ( 990,GHND,USHRT_MAX);
		nLayerSym = 0; 
		pRec = (LPLSREC)GlobalLock (hLayerSym);
		while (SendDlgItemMessage(DXFhWndDlg,IDC_LAYER_LIST,LB_GETTEXT,Choice++,(DWORD)str)!=LB_ERR)
		{
			
			lpTab = _fstrrchr (str,'\t');
			lpTab++;
			isym=atoi(lpTab);
			lpTab=_fstrchr (str,'\t');
			*lpTab++=0;
			lpTab=_fstrchr (lpTab,'\t');
			*lpTab++=0;  
			lpType = lpTab;  
			lpTab=_fstrchr (lpType,'\t');
			*lpTab++=0;
			_fstrcpy (pRec->Name,str);
			_fstrcpy (pRec->Type,lpType);
			pRec++->isym = isym;
			nLayerSym++;
		}
		GlobalUnlock (hLayerSym);
	}
	pRec = (LPLSREC)GlobalLock (hLayerSym); 
	isym = 0;
	for (i=0;i<nLayerSym;i++,pRec++)
	{  
		if (!_fstricmp (LayerName,pRec->Name) && !_fstricmp(Type,pRec->Type)) 
		{   
			if (pRec->isym > 0)
				isym = pRec->isym;
			break;
		}
	}
	GlobalUnlock (hLayerSym);
	return isym;
} 

BOOL SetDXFTAG (LPSTR DXFPrefix,LPSTR DXFUDI,lpDxfToPC lpDxf2GM)
{   
	GetDlgItemText (DXFhWndDlg,IDC_TAPREFIX,DXFPrefix,128);
	GetDlgItemText (DXFhWndDlg,IDC_UDI,DXFUDI,128);  
	SetGlobalValue("%DXFLAYER", lpDxf2GM->LAYER);  
	ExpandText (DXFPrefix);
	ExpandText (DXFUDI);
	return TRUE;
}




void DxfIn(void)
{
/*
%include '/umsc/include/user_data.ftn'
%include '/umsc/include/user_proj_alt_rev.ftn'
%include '/umsc/include/project_data.ftn' {/umsc/include/project_data.ftn}
%include 'dxfin_commons.ftn'
%include 'dxfin_units.ftn'
%include 'dxfin_options_common.ftn'  */

//lpDxfToPC lpDxf;

      char *lptr;
      long IST, NUM_RECS=0,I,ISTX,TotLen;  
      short	i;
  //    int    st;
      unsigned long LAST_REFN=LONG_MAX, Count ;
      MSG msg;
      MSG far *lpMSG;   
      BOOL MoreFiles=FALSE; 
      HFILE	FidFileList;
      char	FileName[MAX_PATH];
      char  mess[256];
   
     static BOOL  PREPROCESSING = FALSE;    
     
	 lineno=0;   
	 KeepGoing = (LPSTR)1;   
	 Canceled = FALSE;
     LinkDXFToPLT = GetGlobalBVal ("[%LINKDXF]");  
     ItemBegin=0;
     NumDXFLayers = 0;
     GSSiGlobFree (&hDXFLayers);
     hDXFLayers = GSSiGlobAlloc ( 993,GHND,USHRT_MAX);
     
      lpMSG = &msg;
      DxfInitActions() ;// get memory for all the variables;
      lpDxf2GM = (lpDxfToPC) GlobalLock(hDxf[1]);
      dxfin_get_options(&IST);
      lpDxfOpts = (lpDxfOptions) GlobalLock(hDxf[0]);
      lpDxf2GM->OFFSET[1] = XOffset;
      lpDxf2GM->OFFSET[2] = YOffset;
      lpDxf2GM->Radian = asin(1e0) / 9e1 ; //{ compute degrees -> radians factor }
      CurLoc = 0;
      _fstrcpy (FileName,lpDxfOpts->INFILE);

      if(ContentsOnly )    
      {
         HASHS((LPINT)&Hid,48,22,256," ",0,"NEW");
         lpDxf2GM->CHECK = TRUE   ; //{ preprocess to determine extents }
         lpDxf2GM->PREPROCESSING = TRUE;
      }
      else
      {
         lpDxf2GM->CHECK = FALSE   ; //{ preprocess to determine extents }
         lpDxf2GM->PREPROCESSING = FALSE;
      }    
        ExpandText (PltName);
        PltType = 2;
        ForceRefIndex = ForceTAGIndex = TRUE;
S1000:      	
       DXFDXF = GSSiOpenFile (FileName,lpOBF,OF_READ);
       if(DXFDXF == HFILE_ERROR)
       { 
              
              sprintf (mess,"Error opening dxf file %s",FileName);
              MessageBox(0,mess,"DXF Reader",MB_ICONEXCLAMATION);
              return ;
       } 

       if (ContentsOnly)
       		sprintf (mess,"Scanning %s",FileName);
       else
       		sprintf (mess,"Loading %s",FileName); 
       SetDlgItemText (hWnd,IDC_PROCESS_MESS,mess);
       TotLen = GSSillseek (DXFDXF,0,2);
       GSSillseek (DXFDXF,0,0);

     // DXFOUT = GSSiOpenFile (lpDxfOpts->OUTFILE,lpOUTF,OF_WRITE|OF_CREATE);
     //           OpenSymDict ("TIGER.gsd", OF_READWRITE);
      _fstrcpy(DXFOutFile,lpDxfOpts->OUTFILE);
S5000: IST = 0;
       lpDxf2GM->COMMAND[0] = '\0';
       lpDxf2GM->CURRENT_SECTION[0] = '\0'; 
       Count = 0;
       Importing = TRUE;
      while (IST == 0) //{ main DXF processing loop }
      { 
         if (Count > 10110)
         	ii=1;
         dxfin_grcode_loop(&IST); //{ handle begin and ends }     
         if(KeepGoing == 0) break;
         if(IST == 6)  break;
         Count++;
             if(Canceled)
             {
               IST = -99;
               goto S9000;
             }
         if(_fstrnicmp(lpDxf2GM->COMMAND,"ENDSEC",6)== 0)    
         {
           // fputstring(lpDxf2GM->COMMAND,DXFOUT) ;
            goto S1;
         }   
         if (_fstrnicmp(lpDxf2GM->COMMAND,"ENDBLK",6) == 0) 
         {
          //  fputstring(lpDxf2GM->COMMAND,DXFOUT) ;
            lpDxf2GM->INBLOCK = FALSE;
            goto S1;
         }   
         if (_fstrnicmp(lpDxf2GM->COMMAND,"SECTION",7)== 0)    
         {
             _fstrcpy(lpDxf2GM->CURRENT_SECTION,&(lpDxf2GM->NAME[1][0]));
             if(_fstrnicmp(lpDxf2GM->CURRENT_SECTION,"HEADER",6) == 0)
             	GotHeader = TRUE;
             if(_fstrnicmp(lpDxf2GM->CURRENT_SECTION,"BLOCKS",6) == 0)
             {
             	GotBlocks = TRUE;
             	SkipToEndSec(hWnd,TotLen,lpDxf2GM->CSTRING);     
//				_fstrcpy (lpDxf2GM->CURRENT_SECTION,"ENTITIES");
             }
             if(_fstrnicmp(lpDxf2GM->CURRENT_SECTION,"TABLES",6) == 0)
             {
             	GotTables = TRUE;
             	SkipToEndSec(hWnd,TotLen,lpDxf2GM->CSTRING); 
             }
             if(_fstrnicmp(lpDxf2GM->CURRENT_SECTION,"ENTITIES",8) == 0)
             {   
             	GotTables=FALSE; // forces to scan data for layers - js 4/2/97
                if(lpDxf2GM->CHECK &&GotHeader&&GotBlocks&&GotTables)
                {
                  IST = -99;
                  goto S9000;
                }   
             }
         }
         else if( _fstrnicmp(lpDxf2GM->CURRENT_SECTION,"ENTITIES",8) == 0)
         {
         	if (lpDxf2GM->ENTITIES)
	        {   
	        	short	ii; 
	        	static	long	lastlineno;
	        	
	        	if (lineno >= 115664)//840504
	        		ii=1;
				lastlineno = lineno;
				dxfin_cmd_processor(lpDxf2GM->COMMAND,lpDxf2GM->FLAG,&IST);  
				
	        }
	        else if (!_fstricmp(lpDxf2GM->COMMAND,"TEXT") || 
	        		 !_fstricmp(lpDxf2GM->COMMAND,"POLYLINE") ||  
	        		 !_fstricmp(lpDxf2GM->COMMAND,"LWPOLYLINE") ||  
	        		 !_fstricmp(lpDxf2GM->COMMAND,"LINE")||
	        		 !_fstricmp(lpDxf2GM->COMMAND,"CIRCLE")||
	        		 !_fstricmp(lpDxf2GM->COMMAND,"TRACE")||
	        		 !_fstricmp(lpDxf2GM->COMMAND,"INSERT")||
	        		 !_fstricmp(lpDxf2GM->COMMAND,"POINT")||
	        		 !_fstricmp(lpDxf2GM->COMMAND,"3DLINE"))
	        {   
	        	char	SymName[34], SubName[34]="",Command[34];
		        
		        if (!_fstricmp(lpDxf2GM->COMMAND,"TEXT"))
		        	ii=1;
	            lpDxf2GM->POLY_CLOSE = FALSE;
	            if(lpDxf2GM->GR_CHANGE[70] && lpDxf2GM->I_VALUE[1] == 1)   // check first bit 
	                    lpDxf2GM->POLY_CLOSE = TRUE;
		        _fstrcpy (Command,lpDxf2GM->COMMAND);
		        if (!_fstricmp (Command,"POLYLINE") || !_fstricmp (Command,"LWPOLYLINE"))   
		        {
		        	if (lpDxf2GM->POLY_CLOSE)
		        		_fstrcpy (Command,"AREA");
		        	else
		        		_fstrcpy (Command,"LINE"); 
		        }
		        	
	        	if(lpDxf2GM->GR_CHANGE[2])    
		        	_fstrcpy(SubName, lpDxf2GM->NAME[1]);
	        	if (!_fstricmp(lpDxf2GM->COMMAND,"INSERT"))
	        		_fstrcpy (Command,"POINT");  
	   SetLayer:
	        	_fstrcpy (SymName,lpDxf2GM->LAYER);
	        	if ((idesc = GetDictSymbolNumber (SymName)))
	        	{
	        		short itype = GetDictSymbolType (idesc);
	        		switch (itype)
	        		{
	        			case 1: //point
	        				if (!_fstrcmp (Command,"POINT") ||
	        				    !_fstrcmp (Command,"TEXT"))
	        				break;
	        				*SymName = 0;
	        				break;
	        			case 2: //line
	        				if (!_fstrcmp (Command,"LINE") ||
	        				    !_fstrcmp (Command,"BORDER"))
	        				break;
	        				*SymName = 0;
	        				break;
	        			case 3: //line
	        				if (!_fstrcmp (Command,"AREA"))
	        				break;
	        				*SymName = 0;
	        				break; 
	        		}
	        	}
	        	else
	        		*SymName = 0; 
	        	if (!*SymName)
	        		idesc = 0;
		        pLayer = (LPLAYERDESC)GlobalLock (hDXFLayers);
		        for (i=0;i<NumDXFLayers;i++,pLayer+=sizeof(LAYERDESC))
		        {   

		        	if (!_fstrcmp(pLayer->Name,lpDxf2GM->LAYER) &&
		        		!_fstrcmp(pLayer->SubName,SubName) &&
		        		!_fstricmp(pLayer->Command,Command))
		        		goto GotLayer;
		        }
		        NumDXFLayers++;  
		        _fstrcpy (pLayer->Name,lpDxf2GM->LAYER);
		        _fstrcpy (pLayer->Command,Command); 
		        _fstrcpy (pLayer->SubName,SubName);
		        _fstrcpy (pLayer->SymName,SymName); 
		        pLayer->SymNum = idesc;
		GotLayer: 
				GlobalUnlock (hDXFLayers); 
				if (!_fstrcmp (Command,"AREA"))
				{
					_fstrcpy (Command,"BORDER");
					goto SetLayer;
				}
			}     
	     }
         else if ( (_fstrnicmp(lpDxf2GM->CURRENT_SECTION,"BLOCKS",6) == 0)
                &&   lpDxf2GM->BLOCKS)
         {     
                  dxfin_cmd_processor(lpDxf2GM->COMMAND,lpDxf2GM->FLAG,&IST);
                  if(_fstrnicmp(lpDxf2GM->COMMAND,"BLOCK",5) == 0 &&
                                lpDxf2GM->BLOCKS && 
                                !_fstrstr(&(lpDxf2GM->NAME[2][0]),"ESRI") )
                  {
                          //fputstring(&(lpDxf2GM->NAME[2][0]),DXFOUT);
                          NumPoints++;
                  }       
          }
          else if (_fstrnicmp(lpDxf2GM->CURRENT_SECTION,"HEADER",6) == 0 
                 && lpDxf2GM->HEADER)
          { 
                 if (!dxfin_header_processor(lpDxf2GM->COMMAND,lpDxf2GM->FLAG,&IST))
                 	break;
          }
          else if (_fstrnicmp(lpDxf2GM->CURRENT_SECTION,"TABLES",6) == 0 
                 && lpDxf2GM->CHECK)// && !lpDxf2GM->CHECK)    
                 DxfInTableProcessor(lpDxf2GM->COMMAND,lpDxf2GM->FLAG,&IST);
S1:       if(IST == -1)    
          {    
          	   char mess[128];
          	   
          	   sprintf (mess,"Invalid command %s in DXF at line %ld - Continue???",lpDxf2GM->CSTRING,lineno);
               if (MessageBox(0,mess,"DXF Importer",MB_YESNO) == IDYES)
               		IST = 0; //{ attempt to continue processing }
          } 
          if (IST == -99) // { SGR file full }
               MessageBox(0,"Maximum GMR file size exceeded.",
                                 "DXF In",MB_ICONINFORMATION);
                 
          if(_fstrnicmp(lpDxf2GM->CSTRING,"EOF",3)== 0) IST = -99 ; //{ check for normal completion }  
           
         _fstrcpy(lpDxf2GM->COMMAND,lpDxf2GM->CSTRING); //{ save command for next time }
         if(KeepGoing == 0)break;
         CurLoc = GSSillseek (DXFDXF,0,1); 
         ItemBegin = CurLoc; 
         if (ItemBegin >= DebugItem)
         	ii=1;     
         PctBox (GetDlgItem(hWnd,IDC_STATUS), TotLen, CurLoc,1);
      }

       if(lpDxf2GM->CHECK && !lpDxf2GM->HEADER)    
       {
         lpDxf2GM->CHECK = FALSE;
         IST = 0;
         _fstrset(lpDxf2GM->CURRENT_SECTION,' ');
         GSSiClose (DXFDXF) ;
       //  fputstring("Beginning actual processing...",DXFOUT);
       //  GSSiClose (DXFOUT);
         DXFDXF = GSSiOpenFile (lpDxfOpts->INFILE,lpOBF,OF_READ);
//         DXFOUT = GSSiOpenFile (lpDxfOpts->GMRFILE,lpOBF,OF_READ);
         for (I = 1;I <= 2; I++) //DO I=1,2;
         {
            lpDxf2GM->EXTENTS[1][I] = lpDxf2GM->EXTENTS[1][I] - lpDxf2GM->OFFSET[1];
            lpDxf2GM->EXTENTS[2][I] = lpDxf2GM->EXTENTS[2][I] - lpDxf2GM->OFFSET[2]; 
         } //ENDDO;
         lpDxf2GM->CNTR = 0;
         goto S5000;
       }    

//C----
S9000:  GSSiClose (DXFDXF);

     if (HaveExtents)
     {
     	DXFMinMax.xmn = lpDxf2GM->EXTENTS[1][1];
     	DXFMinMax.xmx = lpDxf2GM->EXTENTS[1][2];
     	DXFMinMax.ymn = lpDxf2GM->EXTENTS[2][1];
     	DXFMinMax.ymx = lpDxf2GM->EXTENTS[2][2];
     }
      Importing = FALSE;

      PctBox (GetDlgItem(hWnd,IDC_STATUS), TotLen, CurLoc,1);
      if (ContentsOnly)
      	SetDlgItemText (hWnd,IDC_PROCESS_MESS,"Scan complete");
      else
      	SetDlgItemText (hWnd,IDC_PROCESS_MESS,"Load complete");
      if(IST == -99) 
      {//     { successfully processed DXF }
        // WRITE (DXFOUT,9001) ((EXTENTS(I,J),I=1,2),J=1,2);
          if(lpDxf2GM->CHECK)    
             dxfin_list_combos(&IST);
          else 
          { 
            ltoa(LAST_REFN, lpDxf2GM->DUMMY,10);
          //  fputstring(lpDxf2GM->DUMMY,DXFOUT);
            if(lpDxf2GM->DB)   ;//   UM_CLOSE_DB();
            ltoa(NUM_RECS,lpDxf2GM->DUMMY,10);
           // fputstring(lpDxf2GM->DUMMY,DXFOUT);
          }    
      } 
      else 
      {
           // ltoa(IST,lpDxf2GM->DUMMY,10);
           // fputstring(lpDxf2GM->DUMMY,DXFOUT);
      } 
/*        DXFMinMax.xmn = lpDxf2GM->EXTENTS[1][1]*FTM;
        DXFMinMax.ymn = lpDxf2GM->EXTENTS[2][1]*FTM;
        DXFMinMax.xmx = lpDxf2GM->EXTENTS[1][2]*FTM;
        DXFMinMax.ymx = lpDxf2GM->EXTENTS[2][2]*FTM;
*/      
        CloseRefIndex(TRUE);           
//        GSSiClose (DXFOUT);
        if (hDxf[0])
			GlobalUnlock(hDxf[0]); 
		if (hDxf[1])
        	GlobalUnlock(hDxf[1]);
        CLOSE_SYMBOL_EXCHANGE_TABLE(&ISTX); 
        DxfCloseMem(); 
        HASHC(Hid); 
        Hid = 0;
        {
			DPOINT Dpoint={0,0};
			AddPointToMap (Dpoint,0,0,0,0,0,0,0,0,0,0,0,0,0,TRUE,HiPrecis,0,0);
		}
		ForceRefIndex = ForceTAGIndex = FALSE;
        return;
 }
  

lpDxfOptions DxfInitActions(void)
{

short i;
    for(i = 0;i < 12;i++){hDxf[i] = 0; }
    
    hDxf[0] = GSSiGlobAlloc(1766,GHND,(long)sizeof(DxfOptions));

    hDxf[1] = GSSiGlobAlloc(1766,GHND,(long)sizeof(DxfToPC));
    lpDxf2GM = (lpDxfToPC) GlobalLock(hDxf[1]);
    lpDxf2GM->EtRcl = ET_RCL;
    GlobalUnlock(hDxf[1]);
    
    hDxf[2] = GSSiGlobAlloc(1766,GHND,(long)sizeof(DxfOutData));
    
    hDxf[3] = GSSiGlobAlloc(1766,GHND,(long)sizeof(DxfOutData));

    hDxf[4] = GSSiGlobAlloc(1766,GHND,(long)sizeof(MSDict));

    hDxf[5] = GSSiGlobAlloc(1766,GHND,6400);

    hDxf[6] = GSSiGlobAlloc(1766,GHND,7800);

    hDxf[7] = GSSiGlobAlloc(1766,GHND,6400);

    hDxf[8] = GSSiGlobAlloc(1766,GHND,6400);

    hDxf[9] = GSSiGlobAlloc(1766,GHND,12800);
    
    hDxf[10] = GSSiGlobAlloc(1766,GHND,(long)(3900*(sizeof(lpSymDict->EDSC))));
       
    return lpDxfOpts;
}

void DxfCloseMem(void)
{ 
 short i;
 for (i = 0; i < 12; i++)
   GSSiGlobFree(&hDxf[i]);
 return;
}
void dxfin_check_combos(char TYPE,long *IST)
{
  static BOOL FIRST = TRUE;


//C...first time through and no exchange table specified
       if(FIRST && lpDxfOpts->TABLEFILE[0] == '\0')    
       {
         HASHS((LPINT)&lpDxf2GM->ET_HASHID,
               lpDxf2GM->EtRcl*2,
               lpDxf2GM->EtRcl*2-10,
               997,
               " ",
               0,
               "NEW");
         FIRST = FALSE;
      }    

      *DXF_TYPE = TYPE;
      if(lpDxf2GM->GR_CHANGE[2])    
         _fstrcpy(DXF_SYMBOL, NAME)  ; //{ block name if applicable }
      else 
         _fstrcpy(DXF_SYMBOL,"                      ");
      _fstrcpy(DXF_LAYER, LAYER);

      lpDxf2GM->ET_RECORD_LOC = 0;
       HASHF(lpDxf2GM->ET_HASHID,
             (char *) &ET_RECORD[1],
             &lpDxf2GM->ET_RECORD_LOC);
       if(lpDxf2GM->ET_RECORD_LOC == 0)    
       {
         _fstrcpy(UM_SYMBOL,"       ");
         HASHP(lpDxf2GM->ET_HASHID,
               (char *) &ET_RECORD[1],
               &lpDxf2GM->ET_RECORD_LOC);
         lpDxf2GM->ET_NUM++;
         lpDxf2GM->ET_RECORD_ADDR[lpDxf2GM->ET_NUM] = lpDxf2GM->ET_RECORD_LOC;
       }    

      return;
 }
//***********************************************************
void dxfin_cmd_processor(char far *COMMAND,BOOL FLAG, long *IST)
{
      short   NDESC, I,J;  
      double	POCX,POCY;
      char  UM_SYMBOL[8], SYM_TYPE;
      DPOINT DP, CP;
                    DPOINT  DPoints[2];  
                    BOOL    Store=TRUE;  
                    int     i; 
      short	Stuff[32]; 
      long	StreetNums[4];   
      LPSTR	pPrefix, pUDI;
      
		StreetNums[0]=0;
		StreetNums[1]=0;
		StreetNums[2]=0;
		StreetNums[3]=0;
		Stuff[0]=18;
		Stuff[1]=10;
		_fmemmove (&Stuff[2],StreetNums,16);
      

      lpDxf2GM->BLNAME[0] = '\0'; //{ reset block name }
      lpDxf2GM->NTXT = 0;   
      if (_fstricmp (COMMAND,"LWPOLYLINE"))
      	lpDxf2GM->NV = 0;
      *IST = 0;
   if(_fstricmp(COMMAND,"ENTITY")== 0)    
       goto T1000;
      
   if(_fstricmp(COMMAND,"LINE")== 0 || _fstricmp(COMMAND,"3DLINE")==0)    
   {
          size = sizeof(DPOINT)*(2);
          heap_ptr = 0; 
          heap_ptr = (HANDLE) GSSiGlobAlloc ( 994, GHND, size); 
          lpDpoint = (LPDPOINT) GlobalLock(heap_ptr);
          lpPoly = lpDpoint;
         dxfin_doline();
         SYM_TYPE = 'L';
     //    if(_fstrnicmp(lpDxf2GM->CURRENT_SECTION,"BLOCKS",6) == 0)
     //       fputstring(COMMAND,DXFOUT);
          if(!lpDxf2GM->CHECK)    
          {

          if(_fstricmp(LastDxfSymName,&(lpDxf2GM->NAME[1][0])) == 0)
            goto R121;
          _fstrset(GeoHashString,'\0');
          _fstrcpy(GeoHashString ,&(lpDxf2GM->NAME[1][0]));
          HASHF(Hid,GeoHashString,&HashLoc);
          if(HashLoc)
          {
             HASHG(Hid,GeoHashString,HashLoc);
             _fmemcpy(&SymType,&(GeoHashString[44]),2);
             _fstrcpy(GeoSymName,&(GeoHashString[22]));
             _fstrcpy(LastDxfSymName,&(lpDxf2GM->NAME[1][0]));
             _fmemcpy(&GeoSymNum, &(GeoHashString[42]),2);
              PointLineCurve = 1;//a point
          }
          else
          {//user never assigned a symbol for this item.
          
          }
                    
R121:               lpDpoint->x = lpDxf2GM->XY[1][1] ;
                    lpDpoint->y = lpDxf2GM->XY[2][1] ; 
                    lpDpoint++;
                    lpDpoint->x = lpDxf2GM->XY[1][2] ;
                    lpDpoint->y = lpDxf2GM->XY[2][2] ;
                    
                    for (i=0,lpDpoint--;i<2;i++,lpDpoint++)
                    {   
                    	ConvertAndTranCoord (lpDpoint,hTranFile);
                        if (lpDpoint->x < CurView->FileMNMX.xmn ||   
                            lpDpoint->x > CurView->FileMNMX.xmx ||
                            lpDpoint->y < CurView->FileMNMX.ymn ||
                            lpDpoint->y > CurView->FileMNMX.ymx)
                            Store=FALSE;    
                    }   
                    
                    nStore = 2;
                    if (Store)
                    {
	                    if ((idesc = GetLayerSymNum2 (lpDxf2GM)))
	                    {
/*			                if(Gotta1000 && lpDxf2GM->C1000[0] != ' ')
			                        AddPolyToMap2 ((int) 1,  lpnStore,  lpHeapPtr, NOFILL,
			                              NewRefno,  1, idesc,0,DefaultPrefix,lpDxf2GM->C1000,-1L,-1L,-1,HiPrecis);
				                      else */
				                        SetDXFTAG (DXFPrefix,DXFUDI,lpDxf2GM);
										NewRefno = GetNextRefno (&StartRefno,UniqueRefno,TRUE);
				                        AddPolyToMap2 ((int) 1,  lpnStore,  lpHeapPtr, NOFILL,
				                              NewRefno, -1, idesc,Stuff,DXFPrefix,DXFUDI,-1L,-1L,-1,HiPrecis); 
				        }
			        }
                       
// (int nPoly, LPSHORT lpnPnts,LPHANDLE phDPoints,int Type, long NewRefno,int ipen, int idesc,
//                    LPSTR Prefix, LPSTR UDI,long AreaColor,long PenColor, int PenWidth)

          }    
          GSSiGlobUlFree (&heap_ptr);
          lpDxf2GM->NV = 2;
         goto T1000;
      } 
  if (_fstricmp(COMMAND,"POINT") == 0 || _fstricmp(COMMAND,"3DPOINT")==0)    
  {
         Store = TRUE;
          size = sizeof(DPOINT)*(4);
          heap_ptr = 0; 
          heap_ptr = (HANDLE)GSSiGlobAlloc ( 995, GHND, size); 
          lpDpoint = (LPDPOINT) GlobalLock(heap_ptr);
          lpPoly = lpDpoint;
          dxfin_dopoint(lpDpoint);
         SYM_TYPE = 'P';
         if(!lpDxf2GM->CHECK)    
         {
          if(_fstricmp(LastDxfSymName,&(lpDxf2GM->NAME[1][0])) == 0)
            goto T121;
          _fstrset(GeoHashString,'\0');
          _fstrcpy(GeoHashString ,&(lpDxf2GM->NAME[1][0]));
          HASHF(Hid,GeoHashString,&HashLoc);
          if(HashLoc)
          {
             HASHG(Hid,GeoHashString,HashLoc);
             _fmemcpy(&SymType,&(GeoHashString[44]),2);
             _fstrcpy(GeoSymName,&(GeoHashString[22]));
             _fstrcpy(LastDxfSymName,&(lpDxf2GM->NAME[1][0]));
             _fmemcpy(&GeoSymNum, &(GeoHashString[42]),2);
              PointLineCurve = 1;//a point
          }
          else
          {//user never assigned a symbol for this item.
          
          }
T121:        if(!lpDxf2GM->Ignore)  
             {
               DP = *lpPoly;
               ConvertAndTranCoord (&DP,hTranFile);
               if (DP.x < CurView->FileMNMX.xmn ||   
                   DP.x > CurView->FileMNMX.xmx ||
                   DP.y < CurView->FileMNMX.ymn ||
                   DP.y > CurView->FileMNMX.ymx)
                        Store=FALSE;    
                    
                if (Store)
                {
                    if ((idesc = GetLayerSymNum2 (lpDxf2GM)))
                    {
	                    if (LinkDXFToPLT)
	                    	NewRefno = ItemBegin;
	                    else
	                    	NewRefno = GetNextRefno (&StartRefno,UniqueRefno,TRUE); 

                        SetDXFTAG (DXFPrefix,DXFUDI,lpDxf2GM);
	                    AddPointToMap (DP,NewRefno,0,idesc,lpDxf2GM->TSIZE,lpDxf2GM->ROTATION*RADDEG,0,0,0,DXFPrefix,DXFUDI,-1,-1,-1,TRUE,HiPrecis,0,0);
                      	if (idesc>0)
                			AddToSymList (idesc,&NumSyms,&hSymDesc);
	                 }
		        } 
// (int nPoly, LPSHORT lpnPnts,LPHANDLE phDPoints,int Type, long NewRefno,int ipen, int idesc,
//                    LPSTR Prefix, LPSTR UDI,long AreaColor,long PenColor, int PenWidth)
             }   
         }    
          GSSiGlobUlFree (&heap_ptr);
          lpDxf2GM->NV = 1;
         goto T1000;
      }
       
 if (_fstricmp(COMMAND,"CIRCLE") == 0)    
 {

     //    if(_fstrnicmp(lpDxf2GM->CURRENT_SECTION,"BLOCKS",6) == 0)
     //       fputstring(COMMAND,DXFOUT);

         Store = TRUE;
         dxfin_docircle(&Lines);
         SYM_TYPE = 'L';
          if(!lpDxf2GM->CHECK)    
          {

            NDESC = 251;//dxfin_set_symbol(76);
             if(!lpDxf2GM->Ignore)    
             {
             
                   if (Lines.x1 < CurView->FileMNMX.xmn ||   
                       Lines.x1 > CurView->FileMNMX.xmx ||
                       Lines.y1 < CurView->FileMNMX.ymn ||
                       Lines.y1 > CurView->FileMNMX.ymx)
                            goto T1000;    
                   if(Lines.lngth == 0)goto T1000;
             
          NumCrvPts =  fabs(Lines.lngth) / FileDistToBaseDist; 
          if(NumCrvPts < 3) NumCrvPts = 3;
          if(fabs(Lines.lngth) > 20e0)
           st = 0;
          size = USHRT_MAX;
          heap_ptr = 0; 
          heap_ptr = (HANDLE)  GSSiGlobAlloc ( 996, GHND, size); 
          lpDpoint = (LPDPOINT) GlobalLock(heap_ptr);
          lpPoly = lpDpoint;
          nPoints = 0;
           st =  ArcToPoints(&Lines, &nPoints,  &lpDpoint);
          if(nPoints == 0)Store = FALSE;

           if (Store)
           { 
				if ((idesc = GetLayerSymNum2 (lpDxf2GM)))
				{
                  nStore = nPoints;
                  SetDXFTAG (DXFPrefix,DXFUDI,lpDxf2GM);
				  NewRefno = GetNextRefno (&StartRefno,UniqueRefno,TRUE);
                  AddPolyToMap2 ((int) 1, lpnStore, lpHeapPtr,FILL,NewRefno,-1,idesc,0,
                                   0,0,-1,-1,-1,HiPrecis);
                }
           }   
          GSSiGlobUlFree (&heap_ptr);
                     
              if(*IST == 3) *IST = 0; //{ zero radius - just Ignore it }
             }    
           }    
           lpDxf2GM->NV = 4;
           goto T1000;
      }
       
 if (_fstricmp(COMMAND,"ARC") == 0 || _fstricmp(COMMAND ,"3DARC") == 0)    
 {

         Store = TRUE;
     //    if(_fstrnicmp(lpDxf2GM->CURRENT_SECTION,"BLOCKS",6) == 0)
     //       fputstring(COMMAND,DXFOUT);
         dxfin_doarc(&Lines);

         SYM_TYPE = 'L';
     if(!lpDxf2GM->CHECK)    
     {

          NDESC = 251;//dxfin_set_symbol(76);
       if(!lpDxf2GM->Ignore)    
       {
/*          NumCrvPts =  fabs(Lines.lngth) / FileDistToBaseDist; 
          if(NumCrvPts < 3) NumCrvPts = 3;
          if(fabs(Lines.lngth) > 20e0)
           st = 0;  */
          size = 1024;
          heap_ptr = GSSiGlobAlloc ( 997, GMEM_MOVEABLE, size); 
          lpDpoint = (LPDPOINT) GlobalLock(heap_ptr);
          lpPoly = lpDpoint;  
          
          PCURVE(&Lines.x1,&Lines.y1,&POCX,&POCY,
	             &Lines.x2,&Lines.y2,&Lines.rx,&Lines.ry,&Lines.lngth);

          nPoints = 3;  

		  DBoundsInit (&Bounds);
          lpDpoint->x = Lines.x1;
          lpDpoint->y = Lines.y1;
          ConvertAndTranCoord (lpDpoint,hTranFile);
		  AddDPointToMinMax (lpDpoint,&Bounds);
          lpDpoint++;
          lpDpoint->x = POCX;
          lpDpoint->y = POCY;
          ConvertAndTranCoord (lpDpoint,hTranFile);
		  AddDPointToMinMax (lpDpoint,&Bounds);
          lpDpoint++;
          lpDpoint->x = Lines.x2;
          lpDpoint->y = Lines.y2;
          ConvertAndTranCoord (lpDpoint,hTranFile);
		  AddDPointToMinMax (lpDpoint,&Bounds);
          Store = BoundsInBounds (&Bounds,&CurView->FileMNMX,0);

           if (Store)
           {     
                if ((idesc = GetLayerSymNum2 (lpDxf2GM)))
                {
	                nStore = nPoints;
                    SetDXFTAG (DXFPrefix,DXFUDI,lpDxf2GM);
	                NewRefno = GetNextRefno (&StartRefno,UniqueRefno,TRUE);
	                AddPolyToMap2 ((int) 1, lpnStore,lpHeapPtr,ARCPCPOCPT,NewRefno,-1,idesc, 0,
	                                DXFPrefix,DXFUDI, -1L,-1L,(int) -1,HiPrecis);
                }
           }       
           GSSiGlobUlFree (&heap_ptr);
         }
         }    
         goto T1000;
      } 

  if (_fstricmp(COMMAND,"TRACE") == 0)  
  {

         Store = TRUE;
      //   if(_fstrnicmp(lpDxf2GM->CURRENT_SECTION,"BLOCKS",6) == 0)
      //      fputstring(COMMAND,DXFOUT);

          size = sizeof(DPOINT)* 6;
          heap_ptr = 0; 
          heap_ptr = GSSiGlobAlloc ( 998, GHND, size+1); 
          lpDpoint = (LPDPOINT) GlobalLock(heap_ptr);
          lpPoly = lpDpoint;

          dxfin_dotrace(lpDpoint);
 

         SYM_TYPE = 'L';
          if(!lpDxf2GM->CHECK)    
          {
            NDESC = 251;// dxfin_set_symbol(76);
            if(!lpDxf2GM->Ignore)    
            {

               if (lpPoly->x < CurView->FileMNMX.xmn ||   
                   lpPoly->x > CurView->FileMNMX.xmx ||
                   lpPoly->y < CurView->FileMNMX.ymn ||
                   lpPoly->y > CurView->FileMNMX.ymx)  Store=FALSE;    
                nPoints = 4;           
                if (Store)
                {
                    if ((idesc = GetLayerSymNum2 (lpDxf2GM)))
                    {
		                nStore = nPoints;
                        SetDXFTAG (DXFPrefix,DXFUDI,lpDxf2GM);
		                NewRefno = GetNextRefno (&StartRefno,UniqueRefno,TRUE);
		                AddPolyToMap2 ((int)1, lpnStore, lpHeapPtr,FILL,NewRefno,-1,idesc,0,
		                              DXFPrefix,DXFUDI, -1L, -1L, (int) -1,HiPrecis); 
                    }
                } 
            }    
         }    
          GSSiGlobUlFree (&heap_ptr);
         lpDxf2GM->NV = 2;
         goto T1000;
  } 
  if (_fstricmp(COMMAND,"SOLID") == 0)    
  {

         Store = TRUE;
      //   if(_fstrnicmp(lpDxf2GM->CURRENT_SECTION,"BLOCKS",6) == 0)
      //      fputstring(COMMAND,DXFOUT);

          size = sizeof(DPOINT)* 7;
          heap_ptr = 0; 
          heap_ptr = GSSiGlobAlloc ( 999, GHND, size); 
          lpDpoint = (LPDPOINT) GlobalLock(heap_ptr);
          lpPoly = lpDpoint;


          SYM_TYPE = 'L';
          if(!lpDxf2GM->CHECK)    
          {
          dxfin_dotrace(lpDpoint);
          NDESC = 251;//dxfin_set_symbol(76);
             if(!lpDxf2GM->Ignore)    
             {

               if (lpPoly->x < CurView->FileMNMX.xmn ||   
                   lpPoly->x > CurView->FileMNMX.xmx ||
                   lpPoly->y < CurView->FileMNMX.ymn ||
                   lpPoly->y > CurView->FileMNMX.ymx)    Store=FALSE;    
               nPoints = 4;
               Store=FALSE;  
               if (Store)
               {  
                    if ((idesc = GetLayerSymNum2 (lpDxf2GM)))
                    {
		                nStore = nPoints;   
                        SetDXFTAG (DXFPrefix,DXFUDI,lpDxf2GM);
		                NewRefno = GetNextRefno (&StartRefno,UniqueRefno,TRUE);                 
		                AddPolyToMap2 ((int) 1, lpnStore,lpHeapPtr,FILL,NewRefno,-1,idesc,0,
		                                DXFPrefix,DXFUDI, -1L, -1L, (int) 1,HiPrecis); 
		            }
               }   
            }    
          }    
          GSSiGlobUlFree (&heap_ptr);
          lpDxf2GM->NV = 4;
          goto T1000;
  } 
  if (_fstricmp(COMMAND,"TEXT") == 0)    
  {
		LetterStr LStr;
		lpLetterStr lpLStr = &LStr;
		TxtStr    TS; 
		
      TS.lpLtrStr = lpLStr;
         dxfin_dotext();
         SYM_TYPE = 'T';
          if(!lpDxf2GM->CHECK)    
          {
             if(!lpDxf2GM->Ignore)    
             {
               lpDxf2GM->TPL[1][1] = lpDxf2GM->XY[1][1] + 
                                     0.35 * lpDxf2GM->TSIZE * 
                                     cos( (lpDxf2GM->ROTATION - 90.0) * 
                                     PY / 180.0 );
               lpDxf2GM->TPL[2][1] = lpDxf2GM->XY[2][1] + 
                                     (0.35 * lpDxf2GM->TSIZE) * 
                                     sin( (lpDxf2GM->ROTATION - 90.0) * 
                                     PY / 180.0 );
               lpDxf2GM->TPL[1][2] = lpDxf2GM->TPL[1][1];
               lpDxf2GM->TPL[2][2] = lpDxf2GM->TPL[2][1];
               DP.x = lpDxf2GM->XY[1][1] ;
               DP.y = lpDxf2GM->XY[2][1] ; 
	           ConvertAndTranCoord (&DP,hTranFile);
               if (DP.x < CurView->FileMNMX.xmn ||   
                   DP.x > CurView->FileMNMX.xmx ||
                   DP.y < CurView->FileMNMX.ymn ||
                   DP.y > CurView->FileMNMX.ymx)
                        Store=FALSE;    
                    
                if (Store)
                {
                    if ((idesc = GetLayerSymNum2 (lpDxf2GM)))
                    {
						HANDLE	hGRText=GSSiGlobAlloc (1000,GHND,sizeof(GRTEXT)); 
						LPGRTEXT	lpGRText = (LPGRTEXT)GlobalLock (hGRText); 
						
		            	lpGRText->version = 1;    
		            	lpGRText->length = sizeof(GRTEXT);
		            	if (!_fstrnicmp (lpDxf2GM->TEXT,"106TH",5))
		            		ii=1;   
			            _fstrcpy(lpGRText->Text,lpDxf2GM->TEXT); 
			            lpGRText->FontNum = lpDxf2GM->TFONT;
	                    lpGRText->ltext = _fstrlen (lpGRText->Text)+1;
	                    lpGRText->ltext += lpGRText->ltext % 2;  
	                    sprintf (lpGRText->cHeight,"%f",CvtDist(lpDxf2GM->TSIZE*DXFTextSizeAdjust,(short)PRJ_UNITS[3],(short)PRJ_UNITS[1])); 
	                    if (lpDxf2GM->GR_CHANGE[72])
	                    { 
	                    	lpGRText->hJust = lpDxf2GM->I_VALUE[72-69]; 
	                    	if (lpGRText->hJust == 0)
	                    		lpGRText->hJust = 2;
	                    	else if (lpGRText->hJust == 2)
	                    		lpGRText->hJust = 0; 
	                    	else
	                    		lpGRText->hJust = 1; 
	                    }
	                    else
	                    	lpGRText->hJust = 2;
	                    if (lpDxf2GM->GR_CHANGE[73]) 
	                    {   
	                    	switch (lpDxf2GM->I_VALUE[73-69])
	                    	{
	                    		case 0:
	                    		default:
	                    			lpGRText->vJust = 0;
	                    			break;
	                    		case 1:
	                    			lpGRText->vJust = 3;
	                    			break;
	                    		case 2:
	                    			lpGRText->vJust = 2;
	                    			break;
	                    		case 3:
	                    			lpGRText->vJust = 1;
	                    			break;
	                    	}
	                    }
	                    else
	                    	lpGRText->vJust = 0;
				        GlobalUnlock (hGRText);
	                    if (LinkDXFToPLT)
	                    	NewRefno = ItemBegin; 
	                    else
	                    	NewRefno = GetNextRefno (&StartRefno,UniqueRefno,TRUE);
                        SetDXFTAG (DXFPrefix,DXFUDI,lpDxf2GM);
	                    AddPointToMap (DP,NewRefno,0,idesc,lpDxf2GM->TSIZE,lpDxf2GM->ROTATION*RADDEG,0,hGRText,0,DXFPrefix,DXFUDI,-1,-1,-1,TRUE,HiPrecis,0,0);
                      	if (idesc>0)
                			AddToSymList (idesc,&NumSyms,&hSymDesc);
	                    GSSiGlobFree (&hGRText);
	                 }
		        } 
          }    
      /*   if(_fstrnicmp(lpDxf2GM->CURRENT_SECTION,"BLOCKS",6) == 0)
          {
            fputstring(COMMAND,DXFOUT);
            fputstring(lpDxf2GM->TEXT,DXFOUT);
          }  */
           _fstrcpy(TS.lpLtrStr->Text,lpDxf2GM->TEXT);
     /*     TS.lpLtrStr->NumText = lpDxf2GM->NTXT;
          TS.lpLtrStr->X = lpDxf2GM->XY[1][1];
          TS.lpLtrStr->Y = lpDxf2GM->XY[2][1];
          TS.lpLtrStr->Font = lpDxf2GM->TFONT;
          TS.lpLtrStr->Size = lpDxf2GM->TSIZE;
          TS.lpLtrStr->Rot = lpDxf2GM->ROTATION;
          TS.lpLtrStr->Invert = lpDxf2GM->TINVERT;
          TS.lpLtrStr->Tpl[0] = lpDxf2GM->TPL[1][1];
          TS.lpLtrStr->Tpl[1] = lpDxf2GM->TPL[2][1];
          TS.lpLtrStr->Tpl[2] = lpDxf2GM->TPL[1][2];
          TS.lpLtrStr->Tpl[3] = lpDxf2GM->TPL[2][2];   */
         // fputstring("Adding Text",DXFOUT);
        //  fputstring(lpDxf2GM->TEXT,DXFOUT);
          lpDxf2GM->NTXT = 0;
          lpDxf2GM->NV = 1;
          goto T1000;
  	}    
  }
  if (_fstricmp(COMMAND,"BLOCK") == 0)
  {   
          lpDxf2GM->INBLOCK = TRUE;
          goto T900;
  } 
      if (_fstricmp(COMMAND,"INSERT") == 0)    
      {
      //   if(_fstrnicmp(lpDxf2GM->CURRENT_SECTION,"BLOCKS",6) == 0)
      //      fputstring(COMMAND,DXFOUT);
      	  if (!DoInserts)
          { 
          	SkipToSeqend ();
          	goto T1000;
          }
          Store = TRUE;
          size = sizeof(DPOINT)*(4);
          heap_ptr = 0; 
          heap_ptr = (HANDLE)GSSiGlobAlloc (1001, GHND, size); 
          lpDpoint = (LPDPOINT) GlobalLock(heap_ptr);
          lpPoly = lpDpoint;
          
           dxfin_doinsert(lpDpoint);
           
         SYM_TYPE = 'P';
         if(!lpDxf2GM->CHECK && !lpDxf2GM->BLOCKS && !lpDxf2GM->Ignore) 
/*         {//{ don't process if loading block section }
           lpDpoint->x = lpDxf2GM->XY[1][1];
           lpDpoint->y = lpDxf2GM->XY[2][1];
           lpDpoint++;
           lpDpoint->x = lpDxf2GM->XY[1][1] + 1;
           lpDpoint->y = lpDxf2GM->XY[2][1];
           lpDpoint++;
           lpDpoint->x = lpDxf2GM->XY[1][1] ;
           lpDpoint->y = lpDxf2GM->XY[2][1] -1;
           lpDpoint++;
           lpDpoint->x = lpDxf2GM->XY[1][1] -1;
           lpDpoint->y = lpDxf2GM->XY[2][1] ;
          if(_fstricmp(LastDxfSymName,&(lpDxf2GM->NAME[1][0])) == 0)goto H122;
          _fstrset(GeoHashString,'\0');
          _fstrcpy(GeoHashString ,&(lpDxf2GM->NAME[1][0]));
          HASHF(Hid,GeoHashString,&HashLoc);
          if(HashLoc)
          {
             HASHG(Hid,GeoHashString,HashLoc);
             _fmemcpy(&SymType,&(GeoHashString[44]),2);
             _fstrcpy(GeoSymName,&(GeoHashString[22]));
             _fstrcpy(LastDxfSymName,lpDxf2GM->ATTAG);
             _fmemcpy(&GeoSymNum, &(GeoHashString[42]),2);
          }
          else
          {//user never assigned a symbol for this item
          
          }
H122:  
	           if ((idesc = GetLayerSymNum2 (lpDxf2GM)))
	           {
                      nStore = 4;
                      if(Gotta1000 && lpDxf2GM->C1000[0] != ' ')
                        AddPolyToMap2 ((int) 1,  lpnStore, lpHeapPtr, FILL,  NewRefno,
                                     1,  idesc,0,DefaultPrefix, lpDxf2GM->C1000,  -1L,  -1L, (int) 1,HiPrecis); 
                      else              
                        AddPolyToMap2 ((int) 1,  lpnStore, lpHeapPtr, FILL,  NewRefno,  
                                     4,  idesc,0, 0, 0,  -1L,  -1L, (int) 1,HiPrecis);
                } 
          }*/
          
         {
           DP = *lpPoly;
           ConvertAndTranCoord (&DP,hTranFile);
           if (DP.x < CurView->FileMNMX.xmn ||   
               DP.x > CurView->FileMNMX.xmx ||
               DP.y < CurView->FileMNMX.ymn ||
               DP.y > CurView->FileMNMX.ymx)
                    Store=FALSE;    
                    
            if (Store)
            {
                if ((idesc = GetLayerSymNum2 (lpDxf2GM)))
                {
                    if (LinkDXFToPLT)
                    	NewRefno = ItemBegin;
                    else
                    	NewRefno = GetNextRefno (&StartRefno,UniqueRefno,TRUE);
                    SetDXFTAG (DXFPrefix,DXFUDI,lpDxf2GM);
                    AddPointToMap (DP,NewRefno,0,idesc,10,lpDxf2GM->ROTATION*RADDEG,0,0,0,DXFPrefix,DXFUDI,-1,-1,-1,TRUE,HiPrecis,0,0);
                   	if (idesc>0)
              			AddToSymList (idesc,&NumSyms,&hSymDesc);
                 }
	        } 
         }   
          
              
          GSSiGlobUlFree (&heap_ptr);
         lpDxf2GM->ROTATION=0.0;
         lpDxf2GM->NV = 1;
         goto T900;
  } 
  if (!_fstricmp(COMMAND,"ATTDEF") || !_fstricmp(COMMAND,"AttributeDefinition"))
  { //this identifies 1: an attribute definition
                        //2: The field name in the database
                        //3: The value to find in the database 
         if(!ContentsOnly)goto T1000;
         _fstrset(GeoHashString,'\0');
         cptr = _fstrchr(&(lpDxf2GM->NAME[1][0]),'-');
         if (cptr)
         {
	         cptr++;
	         if(cptr)
	            _fstrcpy(GeoHashString,cptr);
	         else
	            _fstrcpy(GeoHashString,&(lpDxf2GM->NAME[1][0]));
	         HASHF(Hid,GeoHashString,&HashLoc);
	         if(!HashLoc)
	         {
	           HASHP(Hid,GeoHashString,&HashLoc);
	           _fstrcpy(&(ATTDEFS[NumAttDefs++][0]),GeoHashString);
	         }
	     }  
         *IST = -2;
         goto T1000;
      } 
      if (!_fstricmp(COMMAND,"ATTRIB"))    
      {LetterStr LStr;
lpLetterStr lpLtrStr = &LStr;
      BOOL Store = TRUE;
      int Len, OddLen,i;
       //  if(_fstrnicmp(lpDxf2GM->CURRENT_SECTION,"BLOCKS",6) == 0)
       //     fputstring(COMMAND,DXFOUT);
          if (!DoAttributes)
          { 
          	SkipToSeqend ();
          	goto T1000;
          }
          dxfin_doattrib(lpLtrStr );
          if(_fstricmp(lpDxf2GM->ATTAG,DefaultPrefix) == 0)goto T1000;
          Len = _fstrlen(lpDxf2GM->ATTVAL);
          if(Len == 0)goto T1000;
          for(i = 0; i < Len; i++)
          {
                if(lpDxf2GM->ATTVAL[i] != ' ') goto G11;
          }
          goto T1000;
G11:      SYM_TYPE = 'A';
          if(!lpDxf2GM->CHECK)    
          {
            NDESC = 251; //dxfin_set_symbol('T');
             if(!lpDxf2GM->Ignore)    
             {
               lpDxf2GM->TPL[1][1] = lpDxf2GM->XY[1][1] + (3.5e-1*lpDxf2GM->TSIZE) * 
                          cos((lpDxf2GM->ROTATION-9e1)*PY/1.8e2);
               lpDxf2GM->TPL[2][1] = lpDxf2GM->XY[2][1] + (double)(3.5e-1*lpDxf2GM->TSIZE) * 
                          sin((lpDxf2GM->ROTATION-9e1)*PY/1.8e2);
               lpDxf2GM->TPL[1][2] = lpDxf2GM->TPL[1][1];
               lpDxf2GM->TPL[2][2] = lpDxf2GM->TPL[2][1];
               if (lpLtrStr->X < CurView->FileMNMX.xmn ||   
                   lpLtrStr->X > CurView->FileMNMX.xmx ||
                   lpLtrStr->Y < CurView->FileMNMX.ymn ||
                   lpLtrStr->Y > CurView->FileMNMX.ymx)    Store=FALSE;    
                  
                 Len = _fstrlen(lpDxf2GM->ATTVAL) + _fstrlen(lpDxf2GM->ATTAG) ;
                 if(Len > 46)  Store  = FALSE;
                 
                if (Store)     
                {      
          size = sizeof(DPOINT)* 6;
          heap_ptr = 0; 
          heap_ptr = GSSiGlobAlloc (1002, GHND, size); 
          lpDpoint = (LPDPOINT) GlobalLock(heap_ptr);
          
          if(_fstricmp(LastDxfSymName,lpDxf2GM->ATTAG) == 0)goto S122;
          _fstrset(GeoHashString,'\0');
          _fstrcpy(GeoHashString ,lpDxf2GM->ATTAG);
          HASHF(Hid,GeoHashString,&HashLoc);
          if(HashLoc)
          {
             HASHG(Hid,GeoHashString,HashLoc);
             _fmemcpy(&SymType,&(GeoHashString[44]),2);
             _fstrcpy(GeoSymName,&(GeoHashString[22]));
             _fstrcpy(LastDxfSymName,lpDxf2GM->ATTAG);
             _fmemcpy(&GeoSymNum, &(GeoHashString[42]),2);
          }
          else
          {//user never assigned a symbol for this item
          
          }

S122:            lpPoly = lpDpoint;
                 lpDpoint->x = lpLtrStr->X - 5e-1;
                 lpDpoint->y = lpLtrStr->Y - 5e-1;
                 lpDpoint++;
                 lpDpoint->x = lpLtrStr->X - 5e-1;
                 lpDpoint->y = lpLtrStr->Y + 5e-1 ;
                 lpDpoint++;
                 lpDpoint->x = lpLtrStr->X + 5e-1;
                 lpDpoint->y = lpLtrStr->Y + 5e-1 ;
                 lpDpoint++;
                 lpDpoint->x = lpLtrStr->X + 5e-1;
                 lpDpoint->y = lpLtrStr->Y - 5e-1 ;
                      if(_fstrstr(lpDxf2GM->ATTAG,DefaultPrefix) != 0)
                      {
                         for(i=0,lpDpoint = lpPoly;i<4;i++,lpDpoint++)
                         {//raise everything up 2 meters
                           lpDpoint->y += 2e0;
                         }
                      }   
                      nStore = 4;
	                  if ((idesc = GetLayerSymNum2 (lpDxf2GM)))
	                  {
	                      /*if(Gotta1000 && lpDxf2GM->C1000[0] != ' ')
	                        AddPolyToMap2 ((int) 1,  lpnStore, lpHeapPtr,  FILL,  NewRefno,  1,  idesc,0,
	                                    DefaultPrefix, lpDxf2GM->C1000,  -1L, -1L, (int) 1,HiPrecis); 
	                      else
	                      {
	                        cptr = _fstrchr(lpDxf2GM->ATTAG,'-');
	                        if(cptr)
	                        {
		                       cptr++;
	                           AddPolyToMap2 ((int) 1,  lpnStore, lpHeapPtr,  FILL,  NewRefno,  4,  idesc,0,
	                                    cptr, lpDxf2GM->ATTVAL,  -1L, -1L, (int) 1,HiPrecis);
	                        }
	                        else
	                           AddPolyToMap2 ((int) 1,  lpnStore, lpHeapPtr, FILL,  NewRefno,  -1,  idesc,0,
	                                    lpDxf2GM->ATTAG, lpDxf2GM->ATTVAL,  -1L, -1L, (int) 1,HiPrecis);
                           }*/
                           //replaces above 12/04/01         
		                       SetDXFTAG (DXFPrefix,DXFUDI,lpDxf2GM); 
		                       NewRefno = GetNextRefno (&StartRefno,UniqueRefno,TRUE);
	                           AddPolyToMap2 ((int) 1,  lpnStore, lpHeapPtr, FILL,  NewRefno,  -1,  idesc,0,
	                                    DXFPrefix,DXFUDI,  -1L, -1L, (int) 1,HiPrecis);
                      }
                      
                }

             }    
             NDESC = 251;//dxfin_set_symbol('A');
             if(!lpDxf2GM->Ignore) ;
              //  UM_ATTRIBUTES(1L,lpDxf2GM->ATTVAL,lpDxf2GM->LATTVAL,IST);
         }    
          GSSiGlobUlFree (&heap_ptr);
          lpDxf2GM->NV = 1;
         goto T1000;
      } 
  if (!_fstricmp(COMMAND,"POLYLINE") || !_fstricmp(COMMAND,"LWPOLYLINE"))    
  {
     int     nPoly, nPoints,lastnpoints, itype;
     long    nVertex;
     LPDPOINT    lpDPoint, lpDPoints;
     DPOINT  LinkPoint;
     LPSTR   lpSpace;
     BOOL    Store, FirstPoly;
                    
          Store=TRUE;  
          hPoly = 0; 
          nPoints = 0;
          FirstPoly=TRUE;
          lpDxf2GM->POLY_CLOSE = FALSE;
          if(lpDxf2GM->GR_CHANGE[70] && lpDxf2GM->I_VALUE[1] == 1)   // check first bit 
                    lpDxf2GM->POLY_CLOSE = TRUE;
       //  if(_fstrnicmp(lpDxf2GM->CURRENT_SECTION,"BLOCKS",6) == 0)
       //     fputstring(COMMAND,DXFOUT);
       Hold1000 = Gotta1000;
S5000:   dxfin_dopoly(&lpDxf2GM->CONTINUE_POLY, IST);
         Gotta1000 = Hold1000;
         SYM_TYPE = 'L';
        if(!lpDxf2GM->CHECK)    
          {

          if(_fstricmp(LastDxfSymName,&(lpDxf2GM->NAME[1][0])) == 0)
            goto S121;
          _fstrset(GeoHashString,'\0');
          _fstrcpy(GeoHashString ,&(lpDxf2GM->NAME[1][0]));
          HASHF(Hid,GeoHashString,&HashLoc);
          if(HashLoc)
          {
             HASHG(Hid,GeoHashString,HashLoc);
             _fmemcpy(&SymType,&(GeoHashString[44]),2);
             _fstrcpy(GeoSymName,&(GeoHashString[22]));
             _fstrcpy(LastDxfSymName,&(lpDxf2GM->NAME[1][0]));
             _fmemcpy(&GeoSymNum, &(GeoHashString[42]),2);
              PointLineCurve = 1;//a point
          }
          else
          {//user never assigned a symbol for this item.
          
          }
S121:        lpDxf2GM->Ignore = 0; //dxfin_set_symbol('L'); currently
             //sets it to true;
             NDESC = 251;//dxfin_set_symbol('L');
             if(!lpDxf2GM->Ignore)    
             {

                         nVertex = lpDxf2GM->NV;
                         lastnpoints = nPoints;
                         nPoints += (int)nVertex;
                         if (hPoly)
                         {  
                            nPoints++;
                            hPoly = GSSiGlobalReAlloc (0,hPoly,(DWORD)(1+nPoints)*sizeof(DPOINT),GMEM_MOVEABLE); 
                            lpDPoints = (LPDPOINT)GlobalLock(hPoly); 
                            lpDPoint = lpDPoints;
                            lpDPoint += lastnpoints;
                         }
                         else
                         {
                             hPoly = GSSiGlobAlloc (1003,GMEM_MOVEABLE,(DWORD)(1+nPoints)*sizeof(DPOINT));
                             lpDPoint = (LPDPOINT)GlobalLock (hPoly); 
                             lpDPoints = lpDPoint;
                         } 
//C
//C              Either  polyln_to_arc_and_line if the curve fit option is
//C              selected or handle lines and bulge factor here.
//C
                if(lpDxf2GM->CURVE_FIT_OPTN)    
                {
                  /* POLYLN_TO_ARC_AND_LINE(&lpDxf2GM->XY[1][1], 
                                          lpDxf2GM->NV, 
                                          UM_SYMBOL)*/;
                } 
                else 
                {  
               SetGlobalValueReal("%DXFELEV",lpDxf2GM->XY[3][1]);
               for ( I=1; I <= lpDxf2GM->NV;I++)
               {
                   if(I == lpDxf2GM->NV/2)    
                      lpDxf2GM->I_NTXT = lpDxf2GM->NTXT;
                   else  
                     lpDxf2GM->I_NTXT = 0;
                   
                      
                   if(fabs(lpDxf2GM->BULGE[I]) < 1e4)    
                   {
                            lpDPoint->x = lpDxf2GM->XY[1][I] ;
                            lpDPoint->y = lpDxf2GM->XY[2][I] ; 
	           				ConvertAndTranCoord (lpDPoint,hTranFile);
                            if (lpDPoint->x < CurView->FileMNMX.xmn ||   
                                lpDPoint->x > CurView->FileMNMX.xmx ||
                                lpDPoint->y < CurView->FileMNMX.ymn ||
                                lpDPoint->y > CurView->FileMNMX.ymx)
                                Store=FALSE;    
                            lpDPoint++;
                   } 
                   else 
                   { // apply bulge factor and place arc
                     lpDxf2GM->ARC[1][1] = lpDxf2GM->XY[1][I];
                     lpDxf2GM->ARC[2][1] = lpDxf2GM->XY[2][I]; // first point
                     lpDxf2GM->ARC[3][1] = lpDxf2GM->XY[3][I];
                     CP.x = (lpDxf2GM->XY[1][I] + lpDxf2GM->XY[1][I+1]) / 2e0;
                     CP.y = (lpDxf2GM->XY[2][I] + lpDxf2GM->XY[2][I+1]) / 2e0;
                     lpDxf2GM->AZM =  LGETAZ(lpDxf2GM->XY[1][I],
                                             lpDxf2GM->XY[2][I],
                                             lpDxf2GM->XY[1][I+1],
                                             lpDxf2GM->XY[2][I+1])
                                     - DSIGN(HALFPI, lpDxf2GM->BULGE[I])  ; // right or left determined by sign
                     lpDxf2GM->BULGE[I] = fabs(lpDxf2GM->BULGE[I])         ; // now shown by angle
                     lpDxf2GM->DIS = sqrt( (lpDxf2GM->XY[1][I] - lpDxf2GM->XY[1][I+1])*(lpDxf2GM->XY[1][I] - lpDxf2GM->XY[1][I+1]) +
                             (lpDxf2GM->XY[2][I] - lpDxf2GM->XY[2][I+1])*(lpDxf2GM->XY[2][I] - lpDxf2GM->XY[2][I+1]) );
                     lpDxf2GM->DIS = lpDxf2GM->BULGE[I] * lpDxf2GM->DIS / 2e0;
                     DP =  dnewpt (CP, lpDxf2GM->AZM, lpDxf2GM->DIS );
                     lpDxf2GM->ARC[1][2] = DP.x;
                     lpDxf2GM->ARC[2][2] = DP.y; // mid point
                     lpDxf2GM->ARC[1][3] = lpDxf2GM->XY[1][I+1];
                     lpDxf2GM->ARC[2][3] = lpDxf2GM->XY[2][I+1]     ; // end point
                     lpDxf2GM->ARC[3][3] = lpDxf2GM->XY[3][I+1];
                     if(*IST==3) *IST = 0; // zero radius - just Ignore it 
                   }    

               }//end of the for loop
               }  //if(CURVE_FIT_OPTN)  


                if (FirstPoly && nPoints)
                {
                  lpDPoint--;
                  LinkPoint = *lpDPoint;
                }  
                else
                  *lpDPoint = LinkPoint;
                   
                if (Store && !lpDxf2GM->CONTINUE_POLY)     
                {
                    nStore = nPoints;   
/*	                if(Gotta1000)
	                {
	                	pPrefix = DefaultPrefix;
	                	pUDI = lpDxf2GM->C1000;
	                }
	                else
	            		pPrefix = 0; */                       
                    SetDXFTAG (DXFPrefix,DXFUDI,lpDxf2GM);
                    pPrefix = DXFPrefix;
                    pUDI = DXFUDI;
                    if (nStore)
                    {
		                if(lpDxf2GM->POLY_CLOSE) 
		                {   
		                	*(lpDPoints + nStore) = *lpDPoints;
		                	nStore++;
			                if ((idesc = GetLayerSymNum (lpDxf2GM->LAYER,"BORDER")))
			                    AddPolyToMap2 ((int) 1, lpnStore, &hPoly, NOFILL,GetNextRefno (&StartRefno,UniqueRefno,TRUE), -1, idesc, Stuff,
			                                      pPrefix,pUDI, -1L, -1L, (int) 1,HiPrecis); 
			                nStore--;
			                if ((idesc = GetLayerSymNum (lpDxf2GM->LAYER,"AREA")))
			                    AddPolyToMap2 ((int) 1, lpnStore, &hPoly, FILL, GetNextRefno (&StartRefno,UniqueRefno,TRUE), -1, idesc, Stuff,
			                                      pPrefix,pUDI, -1L, -1L, (int) 1,HiPrecis); 
		                }
		                else
		                {
			                if ((idesc = GetLayerSymNum (lpDxf2GM->LAYER,"LINE")))
			                    AddPolyToMap2 ((int) 1, lpnStore, &hPoly, NOFILL, GetNextRefno (&StartRefno,UniqueRefno,TRUE), -1, idesc, Stuff,
			                                      pPrefix,pUDI, -1L, -1L, (int) 1,HiPrecis); 
		                } 
		             }
                  }      
                  GlobalUnlock (hPoly);
                  if (!lpDxf2GM->CONTINUE_POLY)  GSSiGlobFree (&hPoly);  
                  FirstPoly=FALSE;
            }    
         }    
          if(lpDxf2GM->CONTINUE_POLY && *IST == 0) goto S5000;
          goto T1000;
      } 
      if (_fstricmp(COMMAND,"SEQEND") == 0)    
      {
         *IST = -2;
          goto T1000;
      } 
	if (!_fstricmp(COMMAND,"MTEXT"))
	{
		Ignore[0]++;
		goto T1000;
	}    
	if (!_fstricmp(COMMAND,"LEADER"))
	{
		Ignore[1]++;
		goto T1000;
	}    
	if (!_fstricmp(COMMAND,"ELLIPSE"))
	{
		Ignore[2]++;
		goto T1000;
	}    
	if (!_fstricmp(COMMAND,"SPLINE")) 
	{
		Ignore[3]++;
		goto T1000;
	}    
    if (_fstricmp(COMMAND,"SHAPE")==0)    
    {
		Ignore[4]++;
        *IST = -2;
        goto T1000;
    } 
      if (_fstrnicmp(COMMAND,"END",3) == 0) //{ ENDSEC ENDBLK }
          *IST = -2;
      else  
          *IST = -1;
 
T900:  if( (_fstricmp(COMMAND,"BLOCK") == 0  && lpDxf2GM->BLOCKS) ||
           (_fstricmp(COMMAND,"INSERT") == 0 && lpDxf2GM->BLOCKS && 
           lpDxf2GM->ENTITIES))
                 
       {
          if(lpDxf2GM->CHECK && !lpDxf2GM->PREPROCESSING)    
          {
             dxfin_check_combos(SYM_TYPE,IST);
             if(SYM_TYPE == 'A')  dxfin_check_combos('T',IST);
          }    
          for( I=1;I <= lpDxf2GM->NV;I++)
          {
            for( J=1; J <= 2; J++)
            {
                if(lpDxf2GM->XY[J][I] < lpDxf2GM->EXTENTS[J][1]) lpDxf2GM->EXTENTS[J][1] = lpDxf2GM->XY[J][I];
                if(lpDxf2GM->XY[J][I] > lpDxf2GM->EXTENTS[J][2]) lpDxf2GM->EXTENTS[J][2] = lpDxf2GM->XY[J][I];
            }
           }
          *IST = 0;
        }    

T1000:   if(*IST == -2) *IST = 0  ; //{ condition internal to this routine }
//C----
      return;
  }
//******************************************************************
      void dxfin_cvt_3pt_arc(double **XY, long *IST)
{

     double AZ12, AZ23,AZP12, AZP23, X12, Y12, X23, Y23,
            AZ1, AZ2, AZ3, AZM12, AZM13, CL, RAD,RPX, RPY;
      short K;      

      AZ12 = LGETAZ(lpDxf2GM->XY[1][1],lpDxf2GM->XY[2][1],
                  lpDxf2GM->XY[1][2],lpDxf2GM->XY[2][2]);
      AZ23 = LGETAZ(lpDxf2GM->XY[1][2],lpDxf2GM->XY[2][2],
                  lpDxf2GM->XY[1][3],lpDxf2GM->XY[2][3]);
      AZP12 = LTWOPI(AZ12+HALFPI);
      AZP23=LTWOPI(AZ23+HALFPI);
      X12=(lpDxf2GM->XY[1][1]+lpDxf2GM->XY[1][2])/2e0;
      Y12=(lpDxf2GM->XY[2][1]+lpDxf2GM->XY[2][2])/2e0;
      X23=(lpDxf2GM->XY[1][3]+lpDxf2GM->XY[1][2])/2e0;
      Y23=(lpDxf2GM->XY[2][3]+lpDxf2GM->XY[2][2])/2e0;
      SECLIN8( &X12,&Y12,&AZP12,&X23,&Y23,&AZP23,&RPX,&RPY,&K);
      if(K!=1)   
      {
         RAD=LDIST(lpDxf2GM->XY[1][1],lpDxf2GM->XY[2][1],RPX,RPY);
         AZ1=LGETAZ(RPX,RPY,XY[1][1],XY[2][1]);
         AZ2=LGETAZ(RPX,RPY,XY[1][2],XY[2][2]);
         AZ3=LGETAZ(RPX,RPY,XY[1][3],XY[2][3]);
         AZM12=LTWOPI(AZ1-AZ2);
         AZM13=LTWOPI(AZ1-AZ3);
         if(AZM12<AZM13)   
         {
//C        *** CLOCKWISE CURVE
            CL=AZM13*RAD;
         } 
         else 
         { 
           if(AZM12 > AZM13)   
           {
//C        *** COUNTER CLOCKWISE CURVE
            CL=(AZM13 - HALFPI) * RAD;
           } 
           else 
           { 
//C        *** POC AND PT THE SAME, IT IS AN ERROR CONDIDTION
//C           PRINT *,'**ERROR** POC & PT COINCIDED'
              RPX=XY[1][1];
              RPY=XY[2][1];
              CL=0e0;
              RAD=0e0; 
           }  
         }    
      } 
      else 
      { 
//C     *** 3 POINTS COLINEAR
//C         PRINT *,'**ERROR** THREE POINTS CO-LINEAR'
         RPX=XY[1][1];
         RPY=XY[2][1];
         CL = 0e0;
         RAD = 0e0;                  
      }           
      XY[1][2]=RPX;
      XY[2][2]=RPY;
      XY[1][3]=CL;
      XY[2][3]=RAD;

      return;
  }
//***********************************************************************
       void dxfin_doarc(LINE *Lines)
{     short i;
      double Sweep, azdf;
      Lines->rad = lpDxf2GM->FL_VALUE[1] * ftm; // in meters 
      lpDxf2GM->SWEEP = lpDxf2GM->ANGLE[2] - lpDxf2GM->ANGLE[1]; //the curves length in radians
      Sweep = lpDxf2GM->SWEEP * lpDxf2GM->Radian;
      if(Sweep <= 0) 
            Sweep += TWOPI;
      Lines->lngth = -1e0* Sweep * Lines->rad;  //in radians
      if(fabs(Lines->lngth) > TWOPI*Lines->rad - 1e-5) 
        lpDxf2GM->POLY_CLOSE = TRUE;
      else
        lpDxf2GM->POLY_CLOSE = FALSE;
        
      Lines->azm  = lpDxf2GM->ANGLE[1] * lpDxf2GM->Radian; //rad -> PC azimuth
      Lines->eazm = lpDxf2GM->ANGLE[2] * lpDxf2GM->Radian; //rad -> PT azimuth
      azdf = AZDF(Lines->azm,Lines->eazm,Lines->lngth);
      if(azdf > Sweep + 1e-5 || azdf < Sweep - 1e-5)
           Lines->lngth *= -1e0;
      
      Lines->rx = (lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1]) * ftm;
      Lines->ry = (lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2]) * ftm; 
      //here we calculate the coords of the PC
      Lines->x1 = (lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1]) * ftm + Lines->rad * cos(Lines->azm);
      Lines->y1 = (lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2]) * ftm + Lines->rad * sin(Lines->azm);

      //Here we calculate the coords of the PT
      Lines->x2 = (lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1]) * ftm + Lines->rad * cos(Lines->eazm);
      Lines->y2 = (lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2]) * ftm + Lines->rad * sin(Lines->eazm);

    //  lpDxf2GM->XY[3][1] = lpDxf2GM->ZZZ[1];
    //  lpDxf2GM->XY[3][3] = lpDxf2GM->ZZZ[1]; 

          
  /*    lpDxf2GM->RADIUS = lpDxf2GM->FL_VALUE[1];
      lpDxf2GM->SWEEP = lpDxf2GM->ANGLE[2] - lpDxf2GM->ANGLE[1];
       if(lpDxf2GM->SWEEP <= 0) lpDxf2GM->SWEEP = lpDxf2GM->SWEEP + 36e1;
      lpDxf2GM->STANG  = lpDxf2GM->ANGLE[1];
      lpDxf2GM->ENDANG = lpDxf2GM->STANG + lpDxf2GM->SWEEP;
      lpDxf2GM->SWEEP = lpDxf2GM->STANG + (lpDxf2GM->SWEEP / 2e0);

//C      IF (ENDANG-STANG<0.) ENDANG = ENDANG + 360.
//C      IF (ENDANG-STANG>360.) ENDANG = ENDANG - 360.
      lpDxf2GM->STANG  = lpDxf2GM->STANG * lpDxf2GM->Radian;
      lpDxf2GM->SWEEP  = lpDxf2GM->SWEEP * lpDxf2GM->Radian;
      lpDxf2GM->ENDANG = lpDxf2GM->ENDANG * lpDxf2GM->Radian;
      lpDxf2GM->XY[1][1] = lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1] + lpDxf2GM->RADIUS*cos(lpDxf2GM->STANG);
      lpDxf2GM->XY[2][1] = lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2] + lpDxf2GM->RADIUS*sin(lpDxf2GM->STANG);
      lpDxf2GM->XY[1][2] = lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1] + lpDxf2GM->RADIUS*cos(lpDxf2GM->SWEEP);
      lpDxf2GM->XY[2][2] = lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2] + lpDxf2GM->RADIUS*sin(lpDxf2GM->SWEEP);
      lpDxf2GM->XY[1][3] = lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1] + lpDxf2GM->RADIUS*cos(lpDxf2GM->ENDANG);
      lpDxf2GM->XY[2][3] = lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2] + lpDxf2GM->RADIUS*sin(lpDxf2GM->ENDANG);
      lpDxf2GM->XY[3][1] = lpDxf2GM->ZZZ[1];
      lpDxf2GM->XY[3][3] = lpDxf2GM->ZZZ[1];   */



      return;
     }
//***********************************************************************;
        void dxfin_doattrib(lpLetterStr lpLtrStr)
{
       if(lpDxf2GM->GR_CHANGE[2])    
       {
         _fstrcpy(lpDxf2GM->ATTAG,&(lpDxf2GM->NAME[1][0]))  ; //{ is this necessary anymore? }
         _fstrcpy(lpDxf2GM->BLNAME, &(lpDxf2GM->NAME[1][0]));
       } 
       else  
         lpDxf2GM->ATTAG[0] = '\0';
          

       if(lpDxf2GM->GR_CHANGE[1])    
       {
         _fstrcpy(lpDxf2GM->ATTVAL,lpDxf2GM->TEXT);
         lpDxf2GM->LATTVAL = _fstrlen(lpDxf2GM->TEXT);
       } 
       else  
         lpDxf2GM->ATTVAL[0] = '\0';
          

      lpDxf2GM->NTXT = _fstrlen(lpDxf2GM->TEXT);

//C...Ignore style and other flags
      lpDxf2GM->TSIZE = lpDxf2GM->FL_VALUE[1] * lpDxf2GM->TSZFAC;

//C...assume font number one
      lpDxf2GM->TFONT = 1;

//C...assume left justification
      lpDxf2GM->TJUST = -99;

//C...assume assume text not inverted
      lpDxf2GM->TINVERT = FALSE;

       if(lpDxf2GM->GR_CHANGE[50]) //    { rotation set ? }
        lpDxf2GM->ROTATION = lpDxf2GM->ANGLE[1];
       else  
        lpDxf2GM->ROTATION = 0e0;
          

      lpDxf2GM->XY[1][1] = (lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1])*ftm;
      lpDxf2GM->XY[2][1] = (lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2])*ftm;
      lpDxf2GM->XY[3][1] = (lpDxf2GM->ZZZ[1])*ftm;
          lpLtrStr->NumText = lpDxf2GM->NTXT;
          lpLtrStr->X = lpDxf2GM->XY[1][1];
          lpLtrStr->Y = lpDxf2GM->XY[2][1];
          lpLtrStr->Font = lpDxf2GM->TFONT;
          lpLtrStr->Size = lpDxf2GM->TSIZE;
          lpLtrStr->Rot = lpDxf2GM->ROTATION;
          lpLtrStr->Invert = lpDxf2GM->TINVERT;
          lpLtrStr->Tpl[0] = lpDxf2GM->TPL[1][1];
          lpLtrStr->Tpl[1] = lpDxf2GM->TPL[2][1];
          lpLtrStr->Tpl[2] = lpDxf2GM->TPL[1][2];
          lpLtrStr->Tpl[3] = lpDxf2GM->TPL[2][2]; 
      return;
    }

//***********************************************************************;
   void dxfin_docircle(LINE *Lines)
   {  short I;
       lpDxf2GM->RADIUS = lpDxf2GM->FL_VALUE[1];
       Lines->rx = (lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1]) * ftm;
       Lines->ry = (lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2]) * ftm;
       Lines->x1 = (lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1] + lpDxf2GM->RADIUS) * ftm;
       Lines->y1 = (lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2] + lpDxf2GM->RADIUS) * ftm;
       Lines->x2 = Lines->x1;
       Lines->y2 = Lines->y1;
       Lines->lngth = TWOPI * lpDxf2GM->RADIUS * ftm;
       Lines->rad = lpDxf2GM->RADIUS * ftm;
       Lines->azm = 0e0;
       Lines->eazm = 0e0;
   /*   lpDxf2GM->RADIUS = lpDxf2GM->FL_VALUE[1];

      lpDxf2GM->XY[1][1] = lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1];
      lpDxf2GM->XY[2][1] = lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2] + lpDxf2GM->RADIUS;
      lpDxf2GM->XY[1][2] = lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1] + lpDxf2GM->RADIUS;
      lpDxf2GM->XY[2][2] = lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2];
      lpDxf2GM->XY[1][3] = lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1];
      lpDxf2GM->XY[2][3] = lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2] - lpDxf2GM->RADIUS;

      lpDxf2GM->XY[1][4] = lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1] - lpDxf2GM->RADIUS;
      lpDxf2GM->XY[2][4] = lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2];
      lpDxf2GM->XY[1][5] = lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1];
      lpDxf2GM->XY[2][5] = lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2] + lpDxf2GM->RADIUS;

      for( I=1; I <= 5;I++)
      {
         lpDxf2GM->XY[3][I] = lpDxf2GM->ZZZ[1];
      }    */
      return;
   }
//***********************************************************************;
        void dxfin_doinsert(LPDPOINT Dpoint)
{
       if(lpDxf2GM->GR_CHANGE[2])    
         _fstrcpy(lpDxf2GM->BLNAME, &lpDxf2GM->NAME[1][0]);
       else 
         _fstrset(lpDxf2GM->BLNAME,' ');
      Dpoint->x = lpDxf2GM->XY[1][1] = (lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1])*ftm;
      Dpoint->y = lpDxf2GM->XY[2][1] = (lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2])*ftm;

       if(lpDxf2GM->GR_CHANGE[50]) 
         lpDxf2GM->ROTATION = lpDxf2GM->ANGLE[1];
       else 
         lpDxf2GM->ROTATION = 0e0;

      return;
     }
//**************************************************************;
        void dxfin_doline(void)
  {      
     short I;
      for( I = 1; I <= 2; I++)
      {
        lpDxf2GM->XY[1][I] = lpDxf2GM->XXX[I] + lpDxf2GM->OFFSET[1];
        lpDxf2GM->XY[2][I] = lpDxf2GM->YYY[I] + lpDxf2GM->OFFSET[2];
        lpDxf2GM->XY[3][I] = lpDxf2GM->ZZZ[1];
      }

      return;
    }
//***********************************************************************;
         void dxfin_dopoint(LPDPOINT Dpoint)
{        
       if(lpDxf2GM->GR_CHANGE[2])    
         _fstrcpy(lpDxf2GM->BLNAME, &lpDxf2GM->NAME[1][0]);
       else  
         lpDxf2GM->BLNAME[0] = '\0';
      Dpoint->x = (lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1]) * ftm;
      Dpoint->y = (lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2]) * ftm;
          
    /*  lpDxf2GM->XY[1][1] = lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1];
      lpDxf2GM->XY[2][1] = lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2];
      lpDxf2GM->XY[3][1] = lpDxf2GM->ZZZ[1]; */

       if(lpDxf2GM->GR_CHANGE[50]) //               { rotation set ? }
         lpDxf2GM->ROTATION = lpDxf2GM->ANGLE[1];
       else  
         lpDxf2GM->ROTATION = 0e0;
      return;
    }
//***********************************************************************;
        void dxfin_dopoly(BOOL *CONTINUE_POLY, long *IST)
{
//C
//C                 NOTE][ IgnoreS ELEV OR THICKNESS 38-39
//C                       CURRENTLY IgnoreS GRCODE 40 - 41 LINEWIDTH
//C                       IgnoreS VERTEX FLAGS GRCODE 70
//C                       IgnoreS VERTEX CURVE FITTING

      char COMMAND[20]; 
      USHORT	i;   
      static	n=0, debugn=4764;
      
      n++;
      if (n==debugn)
      	i=1;
      lpDxf2GM->DONE = FALSE;
      _fstrcpy(COMMAND, lpDxf2GM->CSTRING);

       
      if (!_fstricmp (COMMAND,"LWPOLYLINE"))
      {
          *CONTINUE_POLY = FALSE;
          for (i=0;i<lpDxf2GM->NV;i++) 
          {
              lpDxf2GM->XY[1][i+1] = LWPoint[i].x + lpDxf2GM->OFFSET[1];
              lpDxf2GM->XY[2][i+1] = LWPoint[i].y + lpDxf2GM->OFFSET[2];
              lpDxf2GM->XY[3][i+1] = 0;
          }
//          dxfin_grcode_loop(IST);
         if(KeepGoing)
         {
	         if(!_fstricmp(lpDxf2GM->CSTRING,"SEQEND") || !_fstricmp(lpDxf2GM->CSTRING,"ENDSEC"))    
	         {
	            lpDxf2GM->DONE = TRUE;
	         } 
	         else 
	         { 
	            _fstrcpy(COMMAND, lpDxf2GM->CSTRING);
	         }
         }    
      }    
      else if (!_fstricmp(COMMAND,"ENDSEC"))
            lpDxf2GM->DONE = TRUE;
      else
      {
		lpDxf2GM->NV = 0;
        if(*CONTINUE_POLY)    
        {
         lpDxf2GM->XY[1][1] = lpDxf2GM->XY[1][1000];
         lpDxf2GM->XY[2][1] = lpDxf2GM->XY[2][1000];
         lpDxf2GM->NV = 1;
         *CONTINUE_POLY = FALSE;
        }

        while (! lpDxf2GM->DONE)
        {
          dxfin_grcode_loop(IST);
         if(KeepGoing == 0) break;
          if(_fstricmp(COMMAND,"VERTEX") == 0)    
          {
//C            WRITE (DXFOUT,'(A)') COMMAND
            lpDxf2GM->NV++;
            lpDxf2GM->XY[1][lpDxf2GM->NV] = lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1];
            lpDxf2GM->XY[2][lpDxf2GM->NV] = lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2];
            lpDxf2GM->XY[3][lpDxf2GM->NV] = lpDxf2GM->ZZZ[1];
             if(lpDxf2GM->GR_CHANGE[42])    
               lpDxf2GM->BULGE[lpDxf2GM->NV] = lpDxf2GM->FL_VALUE[3];
             else  
               lpDxf2GM->BULGE[lpDxf2GM->NV] = 0e0;
         }             

         else 
         { 
            char mess[128];
            int answer;
            sprintf (mess,"UNKNOWN COMMAND IN A POLYLINE at line %ld = %s",lineno,COMMAND);
            _fstrcat(mess,"\r\nTo bypass this problem pick YES, otherwise pick NO"); 
            answer = MessageBox(0,mess,"DXF Reader", MB_YESNO);
            if(answer == 6) lpDxf2GM->DONE = TRUE;
         }    
         if(!_fstricmp(lpDxf2GM->CSTRING,"SEQEND") || !_fstricmp(lpDxf2GM->CSTRING,"ENDSEC"))    
         {
            lpDxf2GM->DONE = TRUE;
         } 
         else 
         { 
            _fstrcpy(COMMAND, lpDxf2GM->CSTRING);
         }    
         if(lpDxf2GM->NV >= 1000)    
         {
            *CONTINUE_POLY = TRUE;
            return;
         }    
      }//end of the while loop
    }
    return;
   }
//***********************************************************************;
        void dxfin_dosolid(LPDPOINT lpD)
{  
           dxfin_dotrace(lpD);
           
/*   short I;
      for ( I=1; I <= 4; I++)
      {
        lpDxf2GM->XY[1][I] = lpDxf2GM->XXX[I] + lpDxf2GM->OFFSET[1];
        lpDxf2GM->XY[2][I] = lpDxf2GM->YYY[I] + lpDxf2GM->OFFSET[2];
        lpDxf2GM->XY[3][I] = lpDxf2GM->ZZZ[I];
      }
//C
//C...close it
      lpDxf2GM->XY[1][5] = lpDxf2GM->XY[1][1];
      lpDxf2GM->XY[2][5] = lpDxf2GM->XY[2][1];   */

      return;
   }
//***********************************************************************;
        void dxfin_dotext(void)
{
//C                 DOES NOT HANDLE ELEVATION, STYLE OR OTHER FLAGS
//C
      lpDxf2GM->NTXT = _fstrlen(lpDxf2GM->TEXT);
//       _fstrupr(lpDxf2GM->TEXT);

//C...Ignore style and other flags
      lpDxf2GM->TSIZE = lpDxf2GM->FL_VALUE[1]*lpDxf2GM->TSZFAC;

//C...assume font number one
      lpDxf2GM->TFONT = 1;

//C...assume left justification
      lpDxf2GM->TJUST = -99;

//C...assume assume text not inverted
      lpDxf2GM->TINVERT = FALSE;

       if(lpDxf2GM->GR_CHANGE[50]) // { rotation set ? }
         lpDxf2GM->ROTATION = lpDxf2GM->ANGLE[1];
      else 
         lpDxf2GM->ROTATION = 0e0;

      lpDxf2GM->XY[1][1] = (lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1]);
      lpDxf2GM->XY[2][1] = (lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2]);
      lpDxf2GM->XY[3][1] = lpDxf2GM->ZZZ[1];

      return;
    }
//***********************************************************************;
        void dxfin_dotrace(LPDPOINT lpD)
{
//C                 WIDE LINES NOT HANDLED
//C                 LOAD UP FOUR END POINTS AND PRINT
int NumPts;
      lpDxf2GM->NV = 5;
      for(NumPts = 1;NumPts < 5;NumPts++,lpD++)
      {
        lpD->x = (lpDxf2GM->XXX[NumPts] + lpDxf2GM->OFFSET[1]) * ftm;
        lpD->y = (lpDxf2GM->YYY[NumPts] + lpDxf2GM->OFFSET[2]) * ftm;
      }
     
   /*   lpDxf2GM->XY[1][1] = lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1];
      lpDxf2GM->XY[2][1] = lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2];
      lpDxf2GM->XY[1][2] = lpDxf2GM->XXX[2] + lpDxf2GM->OFFSET[1];
      lpDxf2GM->XY[2][2] = lpDxf2GM->YYY[2] + lpDxf2GM->OFFSET[2];
      lpDxf2GM->XY[1][3] = lpDxf2GM->XXX[4] + lpDxf2GM->OFFSET[1];
      lpDxf2GM->XY[2][3] = lpDxf2GM->YYY[4] + lpDxf2GM->OFFSET[2];
      lpDxf2GM->XY[1][4] = lpDxf2GM->XXX[3] + lpDxf2GM->OFFSET[1];
      lpDxf2GM->XY[2][4] = lpDxf2GM->YYY[3] + lpDxf2GM->OFFSET[2];
      lpDxf2GM->XY[1][5] = lpDxf2GM->XY[1][1];
      lpDxf2GM->XY[2][5] = lpDxf2GM->XY[2][1];   */

      return;
     }
//**********************************************************;
      void dxfin_get_options(long *IST)
      {
      char far *ptr;
      if(ContentsOnly)
      { lpDxf2GM->BLOCKS = TRUE;
        lpDxf2GM->HEADER = TRUE;
        lpDxf2GM->ENTITIES = FALSE; }
      else
      { lpDxf2GM->BLOCKS = FALSE;
        lpDxf2GM->HEADER = FALSE;
        lpDxf2GM->ENTITIES = TRUE; }
      
      lpDxf2GM->DB = FALSE;
      lpDxf2GM->CURVE_FIT_OPTN = FALSE; //{Disable curve fit option.
      lpDxf2GM->ARG_MISSING = FALSE; //{ for branching in curve fit option }


      lpDxfOpts = (lpDxfOptions) GlobalLock(hDxf[0]);
T100:    if (DXFTABLE[0] != '\0')
               _fstrcpy (lpDxfOpts->TABLEFILE,DXFTABLE);
         else 
            //   MessageBox(0,"Invalid or non-existant table filename","DXF Reader",MB_ICONSTOP);

               _fstrcpy(lpDxfOpts->INFILE, NAME);
               lpDxf2GM->TSZFAC = DXFTextScale ; //       { -text_scale_factor }
               lpDxf2GM->SCALE= DXFScale;
               if( lpDxf2GM->SCALE==0e0)  lpDxf2GM->SCALE = 1e0;
               lpDxf2GM->OFFSET[2] = DXFOffset;
            //   lpDxfOpts->REFNO = PrimeRefno;
           //    lpDxfOpts->REFINC= RefnoInc;
               lpDxf2GM->OFFSET[1] = XOffset;
               lpDxf2GM->OFFSET[2] = YOffset;
               if(DXFOutFile[0] == '\0')
                  _fstrcpy(lpDxfOpts->OUTFILE,NAME);
               else
                   _fstrcpy(lpDxfOpts->OUTFILE,DXFOutFile);
               
               ptr = _fstrchr(lpDxfOpts->OUTFILE,'.');
               if(!ptr)
               {
                 _fstrcat(lpDxfOpts->OUTFILE,".RIN");
               }
               else
               {  
                 ptr++;
                 *ptr = '\0';
                 _fstrcat(lpDxfOpts->OUTFILE,"TXT");
               }
               _fstrcpy(lpDxfOpts->GMRFILE, lpDxfOpts->OUTFILE);
               ptr = _fstrchr(lpDxfOpts->GMRFILE,'.');
               ptr++;
               *ptr = '\0';
               _fstrcat(lpDxfOpts->GMRFILE,"RIN");
               _fstrcpy(lpDxfOpts->NEW_OLD,"OLD");

//C
//C        Curve Fit Option][ Create arcs where appropriate to replace
//C                          3 or more line segments of a polyline
//C.
//C               -FC max_cord_length, max_cord_angle, min_cord_angle,
//C                   max_deviation_in_cord_angles
//C       
//C           Set default values.                                       
         if(CurveFitting)
         {
            lpDxf2GM->CURVE_FIT_OPTN = TRUE; //{Enable curve fit option.
            lpDxf2GM->MX_CORD_LENGTH = DEF_MX_CORD_LENGTH; //{Default maximum arc cord length
            lpDxf2GM->MX_CORD_ANGLE = DEF_MX_CORD_ANGLE; //{Default maximum arc cord angle
            lpDxf2GM->MX_CORD_ANGLE_VAR = DEF_MX_CORD_ANGLE_VAR; //{Default max. arc cord angle deviation
//C
//C           Accept up to 3 parameter values.
//C                                  
//C           Look for maximum arc chord length.
//C
               lpDxf2GM->MX_CORD_LENGTH = 3e2 ;
               if(lpDxf2GM->MX_CORD_LENGTH < 0e0)
               {
                 MessageBox(0,"Error in  maximum arc chord length.","DXF Reader",MB_ICONSTOP);
                 goto T100;
               }//end of while loop
//C
//C           Look for maximum arc chord angle.
//C
                   lpDxf2GM->MX_CORD_ANGLE = 
                     (ULIM_MX_CORD_ANGLE + LLIM_MX_CORD_ANGLE)/2;
                  if (lpDxf2GM->MX_CORD_ANGLE > ULIM_MX_CORD_ANGLE ||
                            lpDxf2GM->MX_CORD_ANGLE <=LLIM_MX_CORD_ANGLE)
                  {
                    MessageBox(0,"Error in  maximum arc chord angle.",
                    "DXF Reader",MB_ICONSTOP);
                    goto T100;
                  }
//C           
//C              Look for maximum arc chord angle deviation.
//C           
                     lpDxf2GM->MX_CORD_ANGLE_VAR = 
                        (ULIM_MX_CORD_ANGLE_VAR + LLIM_MX_CORD_ANGLE_VAR)/2; 
                      if (lpDxf2GM->MX_CORD_ANGLE_VAR >
                          ULIM_MX_CORD_ANGLE_VAR || 
                          lpDxf2GM->MX_CORD_ANGLE_VAR < LLIM_MX_CORD_ANGLE_VAR)
                     {
                        MessageBox(0,"Error in  maximum arc chord angle deviation.",
                          "DXF Reader",MB_ICONSTOP);
                        goto T100;
                     }
//C           Convert all angle parameters from degrees to Radians

            lpDxf2GM->MX_CORD_ANGLE = lpDxf2GM->MX_CORD_ANGLE * 1.74532925199433e-2 ; //{RADDEG
            lpDxf2GM->MX_CORD_ANGLE_VAR = lpDxf2GM->MX_CORD_ANGLE_VAR * 1.74532925199433e-2 ; //{RADDEG
         }// end of if(CurveFitting)

 // if(lpDxf2GM->CHECK && !lpDxf2GM->HEADER) lpDxf2GM->HEADER = TRUE  ; //{ -NOHEADER has no meaning with -CHECK }
      *IST = 0;
      GlobalUnlock(hDxf[0]);
      return;
    }
//**********************************************************************;
      void dxfin_grcode_loop(long *IST)
      {
//C
      char cGR[16];
      static	char	CurEntityType[66]="";
      short I,i; 
      BOOL	InEntities;

       if(_fstricmp(lpDxf2GM->CURRENT_SECTION,"BLOCKS") == 0 ||
          _fstricmp(lpDxf2GM->CURRENT_SECTION,"ENTITIES") == 0 ||
          _fstricmp(lpDxf2GM->CURRENT_SECTION,"HEADER") == 0)    
     {

         for (I=1;I <= lpDxf2GM->N_GR_CHANGE; I++)
         {
           lpDxf2GM->GR_CHANGE[lpDxf2GM->GR_CHANGE_INDEX[I]] = FALSE  ; //{ clear all flags for new entity }
         }
         lpDxf2GM->N_GR_CHANGE = 0;

      }    
      Gotta1000 = FALSE;
S100: if (!KeepGoing)
		goto S1000;
	  KeepGoing = fgetstring(cGR,12,DXFDXF); 
	  lineno++;
      CurLoc  += _fstrlen(cGR) + 2; 
      lpDxf2GM->CNTR++;
      lpDxf2GM->GRCODE = atoi(cGR);
      if(lpDxf2GM->GRCODE == 0 || lpDxf2GM->GRCODE == 9)    
      {
        KeepGoing = fgetstring(lpDxf2GM->CSTRING,64,DXFDXF);
	    lineno++;
        CurLoc  += _fstrlen(lpDxf2GM->CSTRING) + 2; 
        if (!lpDxf2GM->GRCODE)
        	_fstrcpy (CurEntityType,lpDxf2GM->CSTRING);
        lpDxf2GM->CNTR++;
        return; //{exit at start of next entity
      } 
      else if(lpDxf2GM->GRCODE == 100)    
      { 
      	char	TempCmd[66];
        KeepGoing = fgetstring(lpDxf2GM->CSTRING,64,DXFDXF);
	    lineno++;
        CurLoc  += _fstrlen(lpDxf2GM->CSTRING) + 2;  
        goto S100;  
        _fstrupr (lpDxf2GM->CSTRING);
        _fstrcpy (TempCmd,lpDxf2GM->CSTRING);
        if (!_fstrnicmp (&TempCmd[4],"2d",2))
        	_fstrcpy (lpDxf2GM->CSTRING,&TempCmd[6]);
        else
        	_fstrcpy (lpDxf2GM->CSTRING,&TempCmd[4]);
        _fstrcpy (CurEntityType,lpDxf2GM->CSTRING);
        lpDxf2GM->CNTR++;
        return; //{exit at start of next entity
      } 
      else 
      {
/*      	if (lpDxf2GM->GRCODE == 100)
      	{
      		SkipToSeqend ();
      		goto S100;
      	} */
       if (lpDxf2GM->GRCODE < 0 || lpDxf2GM->GRCODE > 90)
       {
         if(lpDxf2GM->GRCODE < 1000 || lpDxf2GM->GRCODE > 1071)  goto S99;
       }
        if (!_fstrnicmp(lpDxf2GM->CURRENT_SECTION,"ENTITIES",8))
        {
        	InEntities = TRUE;
        	goto MainSwitch;
        }
        InEntities=FALSE;
       if(_fstrnicmp(lpDxf2GM->CURRENT_SECTION,"BLOCKS",6) != 0 &&
          _fstrnicmp(lpDxf2GM->CURRENT_SECTION,"HEADER",6) != 0)    
       {
          if(lpDxf2GM->GRCODE == 2)    
          {
             KeepGoing = fgetstring (&(lpDxf2GM->NAME[1][0]),72,DXFDXF) ;
			 lineno++;
             CurLoc  += _fstrlen(&(lpDxf2GM->NAME[1][0])) + 2;          
          }
          else
          { 
            KeepGoing = fgetstring (lpDxf2GM->DUMMY,255,DXFDXF) ; 
		    lineno++;
            CurLoc  += _fstrlen(lpDxf2GM->DUMMY) + 2;          
          }  
            lpDxf2GM->CNTR++;
          goto S100;
       }    

 /*   goto (
          1, 2, 2, 2, 2, 6, 7, 8, 9,10
        ,10,10,10,10,10,10,10,10,99,20
        ,20,20,20,20,20,20,20,20,99,30
        ,30,30,30,30,30,30,99,38,39,40
        ,40,40,40,40,40,40,40,40,49,50
        ,50,50,50,50,50,50,50,50,99,99
        ,99,62,99,99,99,66,99,99,99,70
        ,70,70,70,70,70,70,70,70
        ), GRCODE; */ 
MainSwitch:
  switch(lpDxf2GM->GRCODE)
  {
    case 1:     
       KeepGoing = fgetstring(lpDxf2GM->TEXT,255,DXFDXF);
	   lineno++;
       CurLoc  += _fstrlen(lpDxf2GM->TEXT) + 2;          
       break;
    case 2:
    case 3:
    case 4:
    case 5:
       KeepGoing = fgetstring( &(lpDxf2GM->NAME[lpDxf2GM->GRCODE-1][0]), 72,DXFDXF);
	   lineno++;
       CurLoc  += _fstrlen(&(lpDxf2GM->NAME[lpDxf2GM->GRCODE-1][0])) + 2;          
       break;
    case 6:
       KeepGoing = fgetstring( lpDxf2GM->DUMMY,64,DXFDXF);
	   lineno++;
       CurLoc  += _fstrlen( lpDxf2GM->DUMMY) + 2;
       if(ContentsOnly && !_fstrstr(lpDxf2GM->LTYPE,"BYLAYER"))
       {
          _fstrset(GeoHashString,'\0');
          _fstrcpy(GeoHashString,lpDxf2GM->DUMMY);
          HASHF(Hid,GeoHashString,&HashLoc);
          if(!HashLoc)HASHP(Hid,GeoHashString,&HashLoc);
       }       
       break;
    case 7:
       KeepGoing = fgetstring( lpDxf2GM->TXT_STYLE,64,DXFDXF);
	   lineno++;
       CurLoc  += _fstrlen(lpDxf2GM->TXT_STYLE) + 2;          
       break;
    case 8:
        KeepGoing = fgetstring(lpDxf2GM->LAYER,64,DXFDXF);
	    lineno++;
        CurLoc  += _fstrlen(lpDxf2GM->LAYER) + 2;   
       break;
    case 9:
       KeepGoing = fgetstring(lpDxf2GM->VARNAME,64,DXFDXF);
	   lineno++;
       CurLoc  += _fstrlen(lpDxf2GM->VARNAME) + 2;          
       break;
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 210:
       KeepGoing = fgetstring(lpDxf2GM->DUMMY,64,DXFDXF);
	   lineno++;
       CurLoc  += _fstrlen(lpDxf2GM->DUMMY) + 2;          
       lpDxf2GM->XXX[lpDxf2GM->GRCODE-9] = atof(lpDxf2GM->DUMMY);
       if (InEntities && lpDxf2GM->GRCODE == 11 && !_fstricmp (CurEntityType,"TEXT"))
	       lpDxf2GM->XXX[lpDxf2GM->GRCODE-10] = atof(lpDxf2GM->DUMMY);
       if (InEntities && lpDxf2GM->GRCODE == 10 && !_fstricmp (CurEntityType,"LWPOLYLINE"))
	       LWPoint[CurLWPoint].x = atof(lpDxf2GM->DUMMY);
       if (InEntities && !HaveExtents &&
       		(!_fstricmp (CurEntityType,"VERTEX") ||
       		 !_fstricmp (CurEntityType,"TEXT") ||  
       		 !_fstricmp (CurEntityType,"LWPOLYLINE") ||
       		 !_fstricmp (CurEntityType,"LINE"))) 
       {    
			DXFMinMax.xmn = min(DXFMinMax.xmn,lpDxf2GM->XXX[lpDxf2GM->GRCODE-9]) ;
			DXFMinMax.xmx = max(DXFMinMax.xmx,lpDxf2GM->XXX[lpDxf2GM->GRCODE-9]) ;
	   }
       break;
    case 19:
    case 29:
    case 60:
    case 61:
    case 63:
    case 64:
    case 65:
    case 67:
    case 68:
    case 69:
       goto S99;
       break;
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 220:
       KeepGoing = fgetstring(lpDxf2GM->DUMMY,64,DXFDXF);
	   lineno++;
       CurLoc  += _fstrlen(lpDxf2GM->DUMMY) + 2;          
       lpDxf2GM->YYY[lpDxf2GM->GRCODE-19] = atof(lpDxf2GM->DUMMY);
       if (InEntities && lpDxf2GM->GRCODE == 21 && !_fstricmp (CurEntityType,"TEXT"))
	       lpDxf2GM->YYY[lpDxf2GM->GRCODE-20] = atof(lpDxf2GM->DUMMY);
       if (InEntities && lpDxf2GM->GRCODE == 20 && !_fstricmp (CurEntityType,"LWPOLYLINE"))
	       LWPoint[CurLWPoint++].y = atof(lpDxf2GM->DUMMY);
       if (InEntities && !HaveExtents &&
       		(!_fstricmp (CurEntityType,"VERTEX") ||
       		 !_fstricmp (CurEntityType,"TEXT") ||
       		 !_fstricmp (CurEntityType,"LWPOLYLINE") ||
       		 !_fstricmp (CurEntityType,"LINE")))
       {
			DXFMinMax.ymn = min(DXFMinMax.ymn,lpDxf2GM->YYY[lpDxf2GM->GRCODE-19]) ;
			DXFMinMax.ymx = max(DXFMinMax.ymx,lpDxf2GM->YYY[lpDxf2GM->GRCODE-19]) ;
	   }
       break;
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:  
    case 230:
       KeepGoing = fgetstring(lpDxf2GM->DUMMY,64,DXFDXF);
	   lineno++;
       CurLoc  += _fstrlen(lpDxf2GM->DUMMY) + 2;          
      lpDxf2GM->ZZZ[lpDxf2GM->GRCODE-29] = atof(lpDxf2GM->DUMMY);
      break;
   case 38:
       KeepGoing = fgetstring(lpDxf2GM->DUMMY,64,DXFDXF);
	   lineno++;
       CurLoc  += _fstrlen(lpDxf2GM->DUMMY) + 2;          
      lpDxf2GM->ELEV = atof(lpDxf2GM->DUMMY);
      break;
   case 39:
       KeepGoing = fgetstring(lpDxf2GM->DUMMY,64,DXFDXF);
	   lineno++;
       CurLoc  += _fstrlen(lpDxf2GM->DUMMY) + 2;          
      lpDxf2GM->THICK = atof(lpDxf2GM->DUMMY);
      break;
   case 40:
   case 41:
   case 42:
   case 43:
   case 44:
   case 45:
   case 46:
   case 47:
   case 48:
      KeepGoing = fgetstring(lpDxf2GM->DUMMY,64,DXFDXF);
	   lineno++;
      CurLoc  += _fstrlen(lpDxf2GM->DUMMY) + 2;          
      lpDxf2GM->FL_VALUE[lpDxf2GM->GRCODE-39]  = atof(lpDxf2GM->DUMMY);
      break;
   case 49:
       KeepGoing = fgetstring(lpDxf2GM->CSTRING,96,DXFDXF);
	   lineno++;
       CurLoc  += _fstrlen(lpDxf2GM->CSTRING) + 2;          
      break;
   case 50:
   case 51:
   case 52:
   case 53:
   case 54:
   case 55:
   case 56:
   case 57:
   case 58:
   case 59:
     KeepGoing = fgetstring(lpDxf2GM->DUMMY,64,DXFDXF);
	   lineno++;
     CurLoc  += _fstrlen(lpDxf2GM->DUMMY) + 2;          
     lpDxf2GM->ANGLE[lpDxf2GM->GRCODE-49]  = atof(lpDxf2GM->DUMMY);
     break;
   case 62:  
      KeepGoing = fgetstring(lpDxf2GM->DUMMY,64,DXFDXF);
	   lineno++;
      CurLoc  += _fstrlen(lpDxf2GM->DUMMY) + 2;          
      lpDxf2GM->COLOR  = atol(lpDxf2GM->DUMMY);
      break;
    case 66:  
      KeepGoing = fgetstring(lpDxf2GM->CSTRING,96,DXFDXF);
	   lineno++;
      CurLoc  += _fstrlen(lpDxf2GM->CSTRING) + 2;          
      lpDxf2GM->NEST = TRUE; //{ flag for 66 group }
      break;
   case 70:
   case 71:
   case 72:
   case 73:
   case 74:
   case 75:
   case 76:
   case 77:
   case 78:
     KeepGoing = fgetstring(lpDxf2GM->DUMMY,64,DXFDXF);
	   lineno++;
     CurLoc  += _fstrlen(lpDxf2GM->DUMMY) + 2;          
     lpDxf2GM->I_VALUE[lpDxf2GM->GRCODE-69]  = atol(lpDxf2GM->DUMMY);
     break;  
   case 999:  //A COMMENT LINE
     KeepGoing = fgetstring(lpDxf2GM->DUMMY,255,DXFDXF);
	   lineno++;
     CurLoc  += _fstrlen(lpDxf2GM->DUMMY) + 2;          
     break;
     
   case 1000: //An ASCII string < 255 bytes in XDATA
        KeepGoing = fgetstring(lpDxf2GM->DUMMY,256,DXFDXF);
	   lineno++;
        _fstrcpy(lpDxf2GM->MESS,lpDxf2GM->DUMMY);
        CurLoc  += _fstrlen(lpDxf2GM->DUMMY) + 2;          
        if(_fstrstr(lpDxf2GM->DUMMY,"FMSACTS=") != 0)
        {
          _fstrcpy(lpDxf2GM->C1000,&(lpDxf2GM->DUMMY[8]));
          Gotta1000 = TRUE;
        }
      break; 
    case 90:
	     KeepGoing = fgetstring(lpDxf2GM->DUMMY,64,DXFDXF);
		 lineno++;
	     CurLoc  += _fstrlen(lpDxf2GM->DUMMY) + 2; 
	     if (InEntities && !_fstricmp (CurEntityType,"LWPOLYLINE"))
	     {
	     	lpDxf2GM->NV  = atol(lpDxf2GM->DUMMY);
	     	CurLWPoint = 0;
	     }
      break;  
    case 1001: //Registered application name < 31 bytes
    case 1002: //XDATA control string ("{" or "}") (fixed)
    case 1003: //Layer name in XDATA
    case 1004: //Chunk of bytes < 127 in XDATA
    case 1005: //Entity handle in XDATA
    case 1010: //A point in XDATA
    case 1011: //a 3D World space position in XDATA
    case 1012: //A 3D World space displacment in XDATA
    case 1013: //A 3D World space direction in XDATA
    case 1020: //the Y
    case 1021:
    case 1022:
    case 1023:
    case 1030: //the Z
    case 1031:
    case 1032:
    case 1033:
    case 1040: //Floating point value in XDATA
    case 1041: //Distance value in XDATA
    case 1042: //Scale factor in XDATA
    case 1070: //16 bit integer in XDATA
    case 1071: //32 BIT signed long integer in XDATA
        KeepGoing = fgetstring(lpDxf2GM->DUMMY,255,DXFDXF);
	   lineno++;
        CurLoc  += _fstrlen(lpDxf2GM->DUMMY) + 2;          
        break;
    default:  
        goto S99;

   } //end of the switch
      lpDxf2GM->CNTR++;
      lpDxf2GM->GR_CHANGE[lpDxf2GM->GRCODE] = TRUE; 
      lpDxf2GM->N_GR_CHANGE++;
      lpDxf2GM->N_GR_CHANGE = min (lpDxf2GM->N_GR_CHANGE,79);
      lpDxf2GM->GR_CHANGE_INDEX[lpDxf2GM->N_GR_CHANGE] =(short) lpDxf2GM->GRCODE;  
      if (lpDxf2GM->N_GR_CHANGE > 79)
      	ii=1;
      goto S100 ; //{ main loop }
 }
S99:  _fstrcpy(lpDxf2GM->ErrMsg,"Invalid group identifier value = ");
      ltoa(lpDxf2GM->GRCODE,lpDxf2GM->CSTRING,10);
      _fstrcat(lpDxf2GM->ErrMsg,lpDxf2GM->CSTRING);
//      fputstring(lpDxf2GM->ErrMsg, DXFOUT);
      lpDxf2GM->CNTR++;
      KeepGoing = fgetstring(lpDxf2GM->DUMMY,255,DXFDXF);
	   lineno++;
      CurLoc  += _fstrlen(lpDxf2GM->DUMMY) + 2;          
      goto S100; //{ attempt to continue }
/*
S999:  MessageBox(0,"Premature end of DXF data","DXF Reader",MB_OKCANCEL);
     *IST = lpDxf2GM->CNTR;
      return; */

S1000: lpDxf2GM->CNTR++;
      *IST = lpDxf2GM->CNTR;
      _fstrcpy(lpDxf2GM->ErrMsg,"Error reading DXF file on line ");
      ltoa(lpDxf2GM->GRCODE,lpDxf2GM->CSTRING,10);
      _fstrcat(lpDxf2GM->ErrMsg,lpDxf2GM->CSTRING);
      MessageBox(0,lpDxf2GM->ErrMsg,"DXF Reader",MB_ICONSTOP);
      return;  
    }
 
//************************************************
      BOOL dxfin_header_processor(char *COMMAND,BOOL FLAG,long *IST)
  {
      if(_fstricmp(COMMAND,"$EXTMIN") == 0)
      {
         lpDxf2GM->EXTENTS[1][1] = lpDxf2GM->XXX[1];
         lpDxf2GM->EXTENTS[2][1] = lpDxf2GM->YYY[1];
      } 
      if (_fstricmp(COMMAND,"$EXTMAX") == 0 && !IgnoreExtents)
      {
         lpDxf2GM->EXTENTS[1][2] = lpDxf2GM->XXX[1];
         lpDxf2GM->EXTENTS[2][2] = lpDxf2GM->YYY[1];
	     HaveExtents = TRUE;    
	     if (ScanForExtents)
	     	return FALSE;
      }
       
      return TRUE;
  }
//*********************************************************;
      void dxfin_header_substitute(long *IST)
      {


      MessageBox(0,"*** WARNING *** No extents in HEADER section",
      "DXF Reader",MB_ICONSTOP);

//C...prompt for data range for SGR setup
     //HERE WE activate a popup that gets values
     //for the following fields      
                                  /*  lpDxf2GM->EXTENTS[1][1],
                                      lpDxf2GM->EXTENTS[2][1],
                                      lpDxf2GM->EXTENTS[1][2],
                                      lpDxf2GM->EXTENTS[2][2];*/
      *IST = 0;
//      return;

//S999:  *IST = -1;
      return;
   }
//***********************************************************;
      void dxfin_list_combos(long *IST)
  {
    short I;

/*S2:    FORMAT(//
         , '#!symbol type (P -point, L -line, T -text, A -attributes)'
         ,/'#! !AutoCad layer name (20 chrs. max.)'
         ,/'#! !                    !AutoCad block name '
         , '(20 chrs. max.)'
         ,/'#! !                    !                   '
         , ' !UltiMap symbol name (8 chrs. max.)'
         ,/'#! !                    !                    !'
         ,/'#-------------------------------------------'
         , '------------------------------------');  */
 //      if(lpDxf2GM->ET_NUM > 0) WRITE (6,2)
      for( I=1;I <= lpDxf2GM->ET_NUM; I++)
      {
    /*    HASHG(lpDxf2GM->ET_HASHID,
             (char *)&ET_RECORD[0],
             (long)(lpDxf2GM->ET_RECORD_ADDR[I]));  */
/*S1:    FORMAT(1X,A1,1X,A20,1X,A20,1X,A8);
        WRITE (6,1) DXF_TYPE,
                    DXF_LAYER,
                    DXF_SYMBOL,
                    UM_SYMBOL;  */
       // fputstring(DXF_TYPE, DXFOUT);
       // fputstring("  ", DXFOUT);            
       // fputstring(DXF_LAYER, DXFOUT);
       // fputstring("  ", DXFOUT);            
       // fputstring(DXF_SYMBOL, DXFOUT);
       // fputstring("  ", DXFOUT);            
       // fputstring(UM_SYMBOL, DXFOUT);
       // fputstring("\n\r", DXFOUT);            
      }

      return;
  }


//******************************************************;
 void dxfin_load_symbol_table(char *TABLE_FNAME,long *IST)
{

 
      return;
}
//C=======================================================================

      void CLOSE_SYMBOL_EXCHANGE_TABLE(long *IST)
 {
      // HASHC(lpDxf2GM->ET_HASHID);
       DxfCloseMem();

      return;
    }
//*****************************************************************;
     short dxfin_set_symbol( const char TYPE)
     {


      short NDESC=0;
//C
      *DXF_TYPE   = TYPE;
      _fstrcpy(DXF_SYMBOL,lpDxf2GM->BLNAME);
      _fstrcpy(DXF_LAYER,lpDxf2GM->LAYER);

  /*    HASHF(lpDxf2GM->ET_HASHID,
            (char *)&ET_RECORD[0],
            &lpDxf2GM->ET_RECORD_LOC);
      if(lpDxf2GM->ET_RECORD_LOC!= 0) 
      {//{ found match - get UM symbol }
          HASHG(lpDxf2GM->ET_HASHID,
                (char *)&ET_RECORD[0],
                lpDxf2GM->ET_RECORD_LOC);
      } 
      else 
      { 
          if(*DXF_TYPE == 'P') 
          {// { try block only - universal layer }
             _fstrset( DXF_LAYER,' ') ;
             HASHF(lpDxf2GM->ET_HASHID,
                   (char *)&ET_RECORD[0],
                   &lpDxf2GM->ET_RECORD_LOC);
             if(lpDxf2GM->ET_RECORD_LOC != 0)
                HASHG(lpDxf2GM->ET_HASHID,
                      (char *)&ET_RECORD[0],
                      lpDxf2GM->ET_RECORD_LOC);
            _fstrcpy(DXF_LAYER,LAYER);
          }    
          if(lpDxf2GM->ET_RECORD_LOC == 0)    
          {
            *DXF_TYPE = ' ' ; //{ try universal symbol type }
             HASHF(lpDxf2GM->ET_HASHID,
                   (char *)&ET_RECORD[0],
                   &lpDxf2GM->ET_RECORD_LOC);
             if(lpDxf2GM->ET_RECORD_LOC==0) ; //{ last chance - get last symbol }
               lpDxf2GM->ET_RECORD_LOC = lpDxf2GM->ET_RECORD_ADDR[lpDxf2GM->ET_NUM];
             HASHG(lpDxf2GM->ET_HASHID,
                   (char *)&ET_RECORD[0],
                    lpDxf2GM->ET_RECORD_LOC);
         }    
      }           */
//C
//C Ignore data for output if UM symbol is left out of table
      lpDxf2GM->Ignore = FALSE;
       if(UM_SYMBOL[0] == '\0')     
      {
         lpDxf2GM->Ignore = TRUE;
         return 0L;
      }    
      else
      {
         _fstrcpy(lpDxf2GM->OUT_SYM,UM_SYMBOL);
         return NDESC;
      }   
   }
void  DxfInTableProcessor(char *COMMAND,BOOL FLAG,long *IST)
{

  if(_fstrnicmp(&(lpDxf2GM->NAME[1][0]),"LTYPE",5) == 0)
  { //Got me another line type
    int i, casetype, Num49s; 
   if(GotLines)return;
    GotLines = TRUE;
    Num49s = -1; 
    NumLines = atoi(lpDxf2GM->DUMMY);
    if(NumLines == 0)return;
    GSSiGlobFree (&LineMem);
    GSSiGlobFree (&PointMem);

    LineMem = GSSiGlobAlloc(1766,GHND, sizeof(lType)*NumLines);
    lpLT = (lplType) GlobalLock(LineMem); 
    lpLT->NumLines = NumLines;
    for(i = 0,lpLT->Num49s = 0; i < NumLines;i++,lpLT++)
    {
S1:   KeepGoing = fgetstring(lpDxf2GM->CSTRING,96,DXFDXF);  //CASETYPE
	   lineno++;
      CurLoc  += _fstrlen(lpDxf2GM->CSTRING) + 2;          
      casetype = atoi(lpDxf2GM->CSTRING);
      KeepGoing = fgetstring(lpDxf2GM->CSTRING,96,DXFDXF);  //THE VALUE
	   lineno++;
      CurLoc  += _fstrlen(lpDxf2GM->CSTRING) + 2;          
      switch (casetype)
      {
         case 2:
           _fstrcpy(lpLT->Name,lpDxf2GM->CSTRING);
           break;
         case 3:
           _fstrcpy(lpLT->Pattern,lpDxf2GM->CSTRING);
           break;
         case 40:
            lpLT->TotLen = (float)atof(lpDxf2GM->CSTRING);
           break;
         case 49:
           lpLT->DashLen[lpLT->Num49s++] = (float)atof(lpDxf2GM->CSTRING);
           break;
         case 70:
            lpLT->Flags = atoi(lpDxf2GM->CSTRING);
           break;
         case 72:
            lpLT->Align = atoi(lpDxf2GM->CSTRING);
           break;
         case 73:
           ;// lpLT->Num49s = atoi(lpDxf2GM->CSTRING);
           break;
         case 0:
           goto S21;
      }
      goto S1;
S21: ;  //THE VALUE;
    }
  GlobalUnlock(LineMem);
  
  //to test
  /*   lpLT = (lplType) GlobalLock(LineMem);
     for(i = 0;i<NumLines;i++)
       {
         lpLT++;
       }
     GlobalUnlock(LineMem);   */
  }
return;
}
BOOL SkipToSeqend (void)
{
	char	str[260];
	
	while (fgetstring(str,256,DXFDXF))
	{
		lineno++;
		if (!_fstricmp (str,"SEQEND"))
			return TRUE;
	}
	KeepGoing = 0;
    return FALSE;
} 

BOOL SkipToEndSec(HWND hWnd, long TotLen,LPSTR str)
{
	 
	long	CurLoc;
	
	if (!_fstricmp (str,"ENDSEC"))
		return TRUE;
	while (fgetstring(str,252,DXFDXF) && KeepGoing)
	{
		lineno++;
		if (!_fstricmp (str,"ENDSEC"))
			return TRUE;
        CurLoc = GSSillseek (DXFDXF,0,1); 
        PctBox (GetDlgItem(hWnd,IDC_STATUS), TotLen, CurLoc,1);
	}
	KeepGoing = 0;
    return FALSE;
}

void DXFOutPoint (LPDPOINT pPoint,short Code,HFILE Fid)
{   
	char	str[256];
	
	sprintf (str,"%3i\r\n%f\r\n%3i\r\n%f\r\n%3i\r\n0.0",
				  Code,pPoint->x,Code+10,pPoint->y,Code+20); 
	fputstring (str,Fid);
	return;
}

void DXFOutVertex (LPDPOINT pPoint,long DXFHandle, LPSTR SymName,HFILE Fid)
{
	char	str[256];
	
	sprintf (str,"  0\r\nVERTEX\r\n  5\r\n%lX\r\n100\r\nAcDbEntity\r\n  8\r\n%s\r\n100\r\nAcDbVertex\r\n100\r\nAcDb2dVertex\r\n 10\r\n%f\r\n 20\r\n%f\r\n 30\r\n0.0"
			 ,DXFHandle,SymName,pPoint->x,pPoint->y); 
	fputstring (str,Fid); 
	return;
} 

void DXFOutLine (long DXFHandle,LPSTR SymName,HFILE Fid,LPDPOINT BP, LPDPOINT EP,short Width)
{   
	char	str[256];
	
	if (PointExportType)
	{   
		sprintf (str,"-2 2 %i",(int)Width);
		fputstring (str,Fid);
		sprintf (str,"%f %f",BP->x,BP->y);
		fputstring (str,Fid); 
		sprintf (str,"%f %f",EP->x,EP->y);
		fputstring (str,Fid);  
		nExportSymElements++;
		return;
	}
	
	
	sprintf (str,"  0\r\nLINE\r\n  5\r\n%lX\r\n100\r\nAcDbEntity\r\n  8\r\n%s\r\n100\r\nAcDbLine",
			 DXFHandle,SymName); 
	fputstring (str,Fid); 
	if (Width)
	{    
		sprintf (str," 39\r\n%i",(int)Width);
		fputstring (str,Fid); 
	}
    DXFOutPoint (BP,10,Fid);
    DXFOutPoint (EP,11,Fid);
    return;
}

void DXFOutPolyline (LPLONG pDXFHandle,LPSTR SymName,HFILE Fid,long np, HPDPOINT lpDpoint,short Width)
{   
	char	str[256];
	long	i;

	if (PointExportType)
	{   
		sprintf (str,"-2 %ld %i",np,(int)Width);
		fputstring (str,Fid);
		for (i=0;i<np;i++)
		{
			sprintf (str,"%f %f",lpDpoint[i].x,lpDpoint[i].y);
			fputstring (str,Fid); 
		}
		nExportSymElements++;
		return;
	}
	
	sprintf (str,"  0\r\nPOLYLINE\r\n  5\r\n%lX\r\n100\r\nAcDbEntity\r\n  8\r\n%s\r\n100\r\nAcDb2dPolyline\r\n 66\r\n     1",
			 (*pDXFHandle)++,SymName); 
	fputstring (str,Fid);
	if (Width)
	{    
		sprintf (str," 39\r\n%i",(int)Width);
		fputstring (str,Fid); 
	}
	for (i=0;i<np;i++)
		DXFOutVertex (lpDpoint++,(*pDXFHandle)++,SymName,Fid); 
	sprintf (str,"  0\r\nSEQEND\r\n  5\r\n%lX\r\n100\r\nAcDbEntity\r\n  8\r\n%s",
			 (*pDXFHandle)++,SymName);
	fputstring (str,Fid);
	return;
}
 
void DXFOutPolygon (LPLONG pDXFHandle,LPSTR SymName,HFILE Fid,long np, HPDPOINT lpDpoint)
{   
	char	str[256];
	long	i;

	if (PointExportType)
	{   
		sprintf (str,"-3 %ld",np);
		fputstring (str,Fid);
		for (i=0;i<np;i++)
		{
			sprintf (str,"%f %f",lpDpoint[i].x,lpDpoint[i].y);
			fputstring (str,Fid); 
		}
		nExportSymElements++;
		return;
	}
	
	
	sprintf (str,"  0\r\nPOLYLINE\r\n  5\r\n%lX\r\n100\r\nAcDbEntity\r\n  8\r\n%s\r\n100\r\nAcDb2dPolyline\r\n 66\r\n     1\r\n 70\r\n     1",
			 (*pDXFHandle)++,SymName); 
	fputstring (str,Fid);
	for (i=0;i<np;i++)
		DXFOutVertex (lpDpoint++,(*pDXFHandle)++,SymName,Fid); 
	sprintf (str,"  0\r\nSEQEND\r\n  5\r\n%lX\r\n100\r\nAcDbEntity\r\n  8\r\n%s",
			 (*pDXFHandle)++,SymName);
	fputstring (str,Fid);
	return;
}
 

void DXFOutBlocks (HFILE Fid,LPLONG pDXFHandle,short NumPointSymbols,LPSHORT PointSymbols)
{   
	char	str[300], Name[64], FontPathName[64];     
	short	i;

	sprintf (str,"  0\r\nSECTION\r\n  2\r\nBLOCKS\r\n  0\r\nBLOCK\r\n  5\r\n%lX",(*pDXFHandle)++);
	fputstring (str,Fid);
	fputstring ("100\r\nAcDbEntity\r\n  8\r\n0\r\n100\r\nAcDbBlockBegin\r\n  2\r\n*MODEL_SPACE\r\n 70\r\n     0\r\n 10\r\n0.0\r\n 20\r\n0.0\r\n 30\r\n0.0\r\n  3\r\n*MODEL_SPACE\r\n  1\r\n\r\n  0\r\nENDBLK",Fid);
	sprintf (str,"  5\r\n%lX",(*pDXFHandle)++);
	fputstring (str,Fid);
	sprintf (str,"100\r\nAcDbEntity\r\n  8\r\n0\r\n100\r\nAcDbBlockEnd\r\n  0\r\nBLOCK\r\n  5\r\n%lX",(*pDXFHandle)++);
	fputstring (str,Fid);
	fputstring ("100\r\nAcDbEntity\r\n 67\r\n     1\r\n  8\r\n0\r\n100\r\nAcDbBlockBegin\r\n  2\r\n*PAPER_SPACE\r\n 70\r\n     0\r\n 10\r\n0.0\r\n 20\r\n0.0\r\n 30\r\n0.0\r\n  3\r\n*PAPER_SPACE\r\n  1\r\n\r\n  0\r\nENDBLK",Fid);
	sprintf (str,"  5\r\n%lX\r\n100\r\nAcDbEntity\r\n 67\r\n     1\r\n  8\r\n0\r\n100\r\nAcDbBlockEnd",(*pDXFHandle)++);
	fputstring (str,Fid);   
	
	for (i=0;i<NumPointSymbols;i++)
	{   
		char	SymbolName[64]; 
		HANDLE	hSymbol; 
		POINT	SymPoint = {0,0};
		
    	GetDictSymName (PointSymbols[i],SymbolName); 
    	DXFConvertSymName (SymbolName);
    	if (!_fstrchr (SymbolName,'%'))
    	{
			sprintf (str,"  0\r\nBLOCK\r\n  5\r\n%lX\r\n100\r\nAcDbEntity\r\n  8\r\n0\r\n100\r\nAcDbBlockBegin\r\n  2\r\n%s\r\n 70\r\n     0\r\n 10\r\n0.0\r\n 20\r\n0.0\r\n 30\r\n0.0\r\n  3\r\n%s\r\n  1\r\n",
	        		(*pDXFHandle)++,SymbolName,SymbolName);
			fputstring (str,Fid); 
		
/*	sprintf (str,"  1\r\n\r\n  0\r\nCIRCLE\r\n  5\r\n%1X\r\n100\r\nAcDbEntity\r\n  8\r\n0\r\n100\r\nAcDbCircle\r\n 10\r\n0.0\r\n 20\r\n0.0\r\n 30\r\n0.0\r\n 40\r\n3.1",
	     (*pDXFHandle)++);
	fputstring (str,Fid); */

			hSymbol = GetDictSymDesc (PointSymbols[i],0);
			DisplayPointSymbol (hSymbol,(HANDLE)Fid, 1000, 1000,0, &SymPoint,0,FALSE,0,0,FALSE,FALSE,0,0);  
			DestroySymbol (hSymbol); 
			sprintf (str,"  0\r\nENDBLK\r\n  5\r\n%lX\r\n100\r\nAcDbEntity\r\n  8\r\n0\r\n100\r\nAcDbBlockEnd",(*pDXFHandle)++);
			fputstring (str,Fid);
		}
	}
	
	fputstring ("  0\r\nENDSEC",Fid);
	
	return;
}
  

void DXFOutTables (HFILE Fid,LPLONG pDXFHandle,short NumPointSymbols,LPSHORT PointSymbols)
{   
	char	str[300], Name[64], FontPathName[64], FontFileName[34];     
	short	NumFonts=0, i;
	
	fputstring ("  0\r\nSECTION\r\n  2\r\nCLASSES\r\n  0\r\nENDSEC\r\n  0\r\nSECTION\r\n  2\r\nTABLES",Fid); 
	sprintf (str,"  0\r\nTABLE\r\n  2\r\nAPPID\r\n  5\r\n%lX\r\n100\r\nAcDbSymbolTable\r\n 70\r\n     1\r\n  0\r\nAPPID",
					(*pDXFHandle)++);			
    fputstring (str,Fid); 
	sprintf (str,"  5\r\n%lX\r\n100\r\nAcDbSymbolTableRecord\r\n100\r\nAcDbRegAppTableRecord\r\n  2\r\nACAD\r\n 70\r\n     0\r\n  0\r\nENDTAB",
					(*pDXFHandle)++);			
    fputstring (str,Fid); 

	for (i=0;i<MAXFONTS;i++)
		if (*FontNames[i])
			NumFonts++;

	if (NumFonts)
	{ 
		sprintf (str,"  0\r\nTABLE\r\n  2\r\nSTYLE\r\n  5\r\n%lX\r\n100\r\nAcDbSymbolTable\r\n 70\r\n%i",(*pDXFHandle)++,NumFonts);
	    fputstring (str,Fid); 
	    for (i=0;i<MAXFONTS;i++) 
	    {
			if (*FontNames[i]) 
			{
				if (!i)
					_fstrcpy (Name,"STANDARD");
				else
					sprintf (Name,"FONT%i",(int)i);   
				sprintf (str,"  0\r\nSTYLE\r\n  5\r\n%lX\r\n100\r\nAcDbSymbolTableRecord\r\n100\r\nAcDbTextStyleTableRecord\r\n  2\r\n%s\r\n 70\r\n     0",
						(*pDXFHandle)++,Name);
			    fputstring (str,Fid);
			    GetFontFileName (FontNames[i],FontFileName); 
				sprintf (str," 40\r\n0.0\r\n 41\r\n1.0\r\n 50\r\n0.0\r\n  71\r\n     0\r\n  3\r\n%s\r\n  4\r\n",
							FontFileName); 
			    fputstring (str,Fid); 
/*				sprintf (str,"1001\r\nACAD\r\n1000\r\n%s\r\n1071\r\n       34",
							FontNames[i]); 
			    fputstring (str,Fid);*/ 
			}
	    }
	    fputstring ("  0\r\nENDTAB",Fid);
	}
				
	sprintf (str,"  0\r\nTABLE\r\n  2\r\nDIMSTYLE\r\n  5\r\n%lX\r\n100\r\nAcDbSymbolTable\r\n 70\r\n     0\r\n  0\r\nENDTAB",
					(*pDXFHandle)++);			
    fputstring (str,Fid); 
	sprintf (str,"  0\r\nTABLE\r\n  2\r\nVIEW\r\n  5\r\n%lX\r\n100\r\nAcDbSymbolTable\r\n 70\r\n     0\r\n  0\r\nENDTAB",
					(*pDXFHandle)++);			
    fputstring (str,Fid); 
	sprintf (str,"  0\r\nTABLE\r\n  2\r\nUCS\r\n  5\r\n%lX\r\n100\r\nAcDbSymbolTable\r\n 70\r\n     0\r\n  0\r\nENDTAB",
					(*pDXFHandle)++);			
    fputstring (str,Fid); 

	sprintf (str,"  0\r\nTABLE\r\n  2\r\nBLOCK_RECORD\r\n  5\r\n%lX\r\n100\r\nAcDbSymbolTable\r\n 70\r\n     0\r\n  0\r\nBLOCK_RECORD",
					(*pDXFHandle)++);			
    fputstring (str,Fid); 
	sprintf (str,"  5\r\n%lX\r\n100\r\nAcDbSymbolTableRecord\r\n100\r\nAcDbBlockTableRecord\r\n  2\r\n*MODEL_SPACE",
					(*pDXFHandle)++);			
    fputstring (str,Fid); 
	sprintf (str,"  0\r\nBLOCK_RECORD\r\n  5\r\n%lX\r\n100\r\nAcDbSymbolTableRecord\r\n100\r\nAcDbBlockTableRecord\r\n  2\r\n*PAPER_SPACE",
					(*pDXFHandle)++);			
    fputstring (str,Fid); 
    
    for (i=0;i<NumPointSymbols;i++)
    {   
    	char	SymbolName[64];
    	
    	GetDictSymName (PointSymbols[i],SymbolName);
    	DXFConvertSymName (SymbolName);
    	if (!_fstrchr (SymbolName,'%'))
    	{
			sprintf (str,"  0\r\nBLOCK_RECORD\r\n  5\r\n%lX\r\n100\r\nAcDbSymbolTableRecord\r\n100\r\nAcDbBlockTableRecord\r\n  2\r\n%s",
							(*pDXFHandle)++,SymbolName);			
		    fputstring (str,Fid);
		} 
    }

	_fstrcpy (str,"  0\r\nENDTAB");
    fputstring (str,Fid); 

	sprintf (str,"  0\r\nTABLE\r\n  2\r\nLTYPE\r\n  5\r\n%lX\r\n100\r\nAcDbSymbolTable\r\n 70\r\n     1",
					(*pDXFHandle)++);			
    fputstring (str,Fid); 

	sprintf (str,"  0\r\nLTYPE\r\n  5\r\n%lX\r\n100\r\nAcDbSymbolTableRecord\r\n100\r\nAcDbLinetypeTableRecord\r\n  2\r\nBYBLOCK\r\n 70\r\n     0\r\n  3\r\n\r\n 72\r\n    65\r\n 73\r\n     0\r\n 40\r\n0.0",
					(*pDXFHandle)++);			
    fputstring (str,Fid); 
	sprintf (str,"  0\r\nLTYPE\r\n  5\r\n%lX\r\n100\r\nAcDbSymbolTableRecord\r\n100\r\nAcDbLinetypeTableRecord\r\n  2\r\nBYLAYER\r\n 70\r\n     0\r\n  3\r\n\r\n 72\r\n    65\r\n 73\r\n     0\r\n 40\r\n0.0",
					(*pDXFHandle)++);			
    fputstring (str,Fid); 
	sprintf (str,"  0\r\nLTYPE\r\n  5\r\n%lX\r\n100\r\nAcDbSymbolTableRecord\r\n100\r\nAcDbLinetypeTableRecord\r\n  2\r\nCONTINUOUS\r\n 70\r\n     0\r\n  3\r\nSolid line\r\n 72\r\n    65\r\n 73\r\n     0\r\n 40\r\n0.0\r\n  0\r\nENDTAB",
					(*pDXFHandle)++);			
    fputstring (str,Fid); 
	

	sprintf (str,"  0\r\nTABLE\r\n  2\r\nLAYER\r\n  5\r\n%lX\r\n100\r\nAcDbSymbolTable\r\n 70\r\n     1",
					(*pDXFHandle)++);			
    fputstring (str,Fid); 
	sprintf (str,"  0\r\nLAYER\r\n  5\r\n%lX\r\n100\r\nAcDbSymbolTableRecord\r\n100\r\nAcDbLayerTableRecord\r\n  2\r\n0\r\n 70\r\n     0\r\n 62\r\n     7\r\n  6\r\nCONTINUOUS\r\n  0\r\nENDTAB",
					(*pDXFHandle)++);			
    fputstring (str,Fid); 

	sprintf (str,"  0\r\nTABLE\r\n  2\r\nVPORT\r\n  5\r\n%lX\r\n100\r\nAcDbSymbolTable\r\n 70\r\n     0\r\n  0\r\nENDTAB",
					(*pDXFHandle)++);			
    fputstring (str,Fid); 
	

    fputstring ("  0\r\nENDSEC",Fid);
	return;				
				
} 

void DXFOutObjects (HFILE Fid,LPLONG pDXFHandle)
{   
	char	str[300]; 
	long	Dict1Handle=(*pDXFHandle)++;
	long	ACAD_GROUPhandle, ACAD_MLINESTYLEhandle;
	
	sprintf (str,"  0\r\nSECTION\r\n  2\r\nOBJECTS\r\n  0\r\nDICTIONARY\r\n  5\r\n%lX",
					Dict1Handle);			
    fputstring (str,Fid);  
    ACAD_GROUPhandle = (*pDXFHandle)++; 
    ACAD_MLINESTYLEhandle = (*pDXFHandle)++; 
	sprintf (str,"100\r\nAcDbDictionary\r\n  3\r\nACAD_GROUP\r\n350\r\n%lX\r\n  3\r\nACAD_MLINESTYLE\r\n350\r\n%lX",
					ACAD_GROUPhandle,ACAD_MLINESTYLEhandle);			
    fputstring (str,Fid); 
	sprintf (str,"  0\r\nDICTIONARY\r\n  5\r\n%lX\r\n102\r\n{ACAD_REACTORS\r\n330\r\n%lX\r\n102\r\n}\r\n100\r\nAcDbDictionary",
					ACAD_GROUPhandle,Dict1Handle);			
    fputstring (str,Fid); 
	sprintf (str,"  0\r\nDICTIONARY\r\n  5\r\n%lX\r\n102\r\n{ACAD_REACTORS\r\n330\r\n%1X\r\n102\r\n}",
					ACAD_MLINESTYLEhandle,Dict1Handle);			
    fputstring (str,Fid); 
	sprintf (str,"100\r\nAcDbDictionary\r\n  3\r\nSTANDARD\r\n350\r\n101D\r\n  0\r\nENDSEC",
					(*pDXFHandle)++);			
    fputstring (str,Fid); 
	return;
}


double AZToDXFAngle (double AZ)
{
	double	Angle;
	
	Angle = LTWOPI (AZ) * DEGRAD;
	return Angle;
}

void DXFConvertSymName (LPSTR SymName)
{
	ReplaceChar (SymName,'/','_');
	ReplaceChar (SymName,'"','_');
	ReplaceChar (SymName,'.','_');
	ReplaceChar (SymName,'&','_');
	ReplaceChar (SymName,' ','_'); 
	Truncate (SymName);
	if (!*SymName)
		_fstrcpy (SymName,"BLANK");
	return;
}  
BOOL FAR PASCAL LOADDXFMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{   char string[130], Name[128], LeafName[128];
    char		Ext[6]=".TL2";
   	HFILE	FidDSC, FidFileList;  
   	BOOL	MoreFiles;
	OFSTRUCTGM	OFStruct;
    long	TotLenFL, CurLocFL;
    RECT    rect;
                    DPOINT PT1,PT2,PT3,PT4,PT11,PT12,PT13,PT14;
                double  lat=45,lon=-93;
    static	BOOL	FileIsOpen;
    FARPROC lpfnDxfLinesMsgProc;
    int    BRtn,got, nRc = 0,i;
     HDC hDC;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG: 
    { 
        int     got;
	    int     TabStops[4]={100,160,222,1300};
        char	str[128];
        
        FileIsOpen = FALSE;
        DXFhWndDlg = hWndDlg;   
        hWnd = hWndDlg;
   //     hWndMain = hWnd; 
    	SetDlgItemTextGlobal (hWndDlg,IDC_STARTNO,"[%STARTREFNO]","1000000");
		SymConvTableChanged = FALSE;  
        SendDlgItemMessage (hWndDlg,IDC_LAYER_LIST ,LB_SETTABSTOPS,4,(LPARAM)&TabStops); 
        SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_SETCHECK,TRUE,0L); 
        GSSiGlobFree (&hLayerSym);
//        SendDlgItemMessage (hWndDlg,IDC_AUTOSYM,BM_SETCHECK,FALSE,0L);
        SetCurvPltCtol (0.05 * 2.0);
        Importing = FALSE;
        Canceled = FALSE;
        GotTables = FALSE;
        GotHeader = FALSE;
        GotBlocks = FALSE;
        NumLines = 0;
        NumPoints = 0;
        NumAttDefs = 0;
        DXFTextScale = 1e0; 

         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Feet");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Meters");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Degrees");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Degrees * 1000000");
         _fstrcpy (str,"*.CVT");
         DlgDirListComboBox (hWndDlg,str,IDC_PROJECTION,0,DDL_READWRITE);  
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_SELECTSTRING,-1,(LPARAM)curunits);
         SendDlgItemMessage (hWndDlg,IDC_PROJECTION,CB_SELECTSTRING,-1,(LPARAM)curproject);
        ContentsOnly = FALSE;
        CurveFitting = FALSE;
        GotLines = FALSE;
        GetWindowRect(hWndDlg, &rect); //GetDesktopWindow(), &rect);
//        SetDlgItemText (hWndDlg,IDC_DXF_X_OFFSET,"0");
//        SetDlgItemText (hWndDlg,IDC_DXF_Y_OFFSET,"0");
//        SetDlgItemText (hWndDlg,IDC_TEXT_SCALE,"1");
        SetDlgItemText (hWndDlg,IDC_DXF_INFILE,"");
//        SendDlgItemMessage(hWndDlg, IDC_DXF_CURVE, BM_SETCHECK, FALSE, 0);
//        SendDlgItemMessage(hWndDlg, IDC_DXF_CHECK, BM_SETCHECK, FALSE, 0);
//        SetDlgItemText (hWndDlg,IDC_DXF_X_OFFSET,"0");
//        SetDlgItemText (hWndDlg,IDC_DXF_Y_OFFSET,"0");
         if (*AutoExportName)
		 	PostMessage(hWndDlg, WM_COMMAND, IDC_RECALL, 0L);
    case GSSI_REINITDIALOG: 
         if (FileIsOpen && *AutoExportName)
	         PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);     
    }
         break; /* End of WM_INITDIALOG                                 */


    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         GSSiGlobFree (&LineMem);  
         GSSiGlobFree (&PointMem);
         PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
           	case IDC_RECALL: 
           	{
           		 short Version;     
            	 HFILE	FidSave;
				 OFSTRUCTGM	OFStruct;
           		 
           		 
             	 if (!*AutoExportName)
             	 { 
					 if (!GetFileName2 (hWndDlg,Name,Ext,IDS_FILETL2))
					 	break;
	             }
	             else
	             	_fstrcpy (Name,AutoExportName);
	             DestroyAdvancedOpts ();
                 FidSave = GSSiOpenFile (Name,&OFStruct,OF_READ);
                 BigRead (FidSave,Ext,6);
                 BigRead (FidSave,(HPSTR)&Version,2);   
/*                 BigRead (FidSave,LoadName,128);
                 SetDlgItemText (hWndDlg,IDC_FILE,LoadName);
                 BigRead (FidSave,PltName,128);
                 SetDlgItemText (hWndDlg,IDC_DESTMAP,PltName);
                 BigRead (FidSave,&NewOpt,2);
                 SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_SETCHECK,NewOpt,0);
                 BigRead (FidSave,Prefix,sizeof(Prefix));
                 SetDlgItemText(hWndDlg,IDC_TAPREFIX,Prefix);
                 BigRead (FidSave,UDI,128);
                 SetDlgItemText(hWndDlg,IDC_UDI,UDI);               
                 BigRead (FidSave,SymStuff,lnSymStuff);
                 SetDlgItemText(hWndDlg,IDC_SYMNAME,SymStuff);               
			     ii = GSSillseek (FidSave,0,1);
                 ii=BigRead (FidSave,curproject,34);
		         SendDlgItemMessage (hWndDlg,IDC_PROJECTION,CB_SELECTSTRING,-1,(LPARAM)curproject);
		     	 ii = GSSillseek (FidSave,0,1);
                 BigRead (FidSave,&UnitsOpt,2);
			     SendDlgItemMessage(hWndDlg,IDC_UNITS,CB_SETCURSEL,UnitsOpt,0); 
			     if (Version > 1)
			     {   
			     	 ii = GSSillseek (FidSave,0,1);
	                 ii=BigRead (FidSave,&NewOpt,2);
	                 SendDlgItemMessage (hWndDlg,IDC_CREATE_SYMS,BM_SETCHECK,NewOpt,0);
	                 ii=BigRead (FidSave,&HiPrecis,2);
	                 SendDlgItemMessage (hWndDlg,IDC_HIPRECIS,BM_SETCHECK,HiPrecis,0);
			     	 ii = GSSillseek (FidSave,0,1);
	                 ii=BigRead (FidSave,str,254);
			     }
			     while (ReadObject (FidSave, FALSE,0,0));*/
                 GSSiClose (FidSave);
				 PostMessage(hWndDlg, WM_COMMAND, IDC_SET_SOURCE, 0L);
                 FileIsOpen = TRUE;
               	 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
            }
           		 break;

            case IDCANCEL:
                 if(Importing)
                 {
                   Canceled = TRUE;
                   break;
                 }  
                 break;
                 
           case	 IDC_EXIT: 
				 if (SymConvTableChanged)
				 {
				 	short opt=MessageBox (hWndDlg,"You have made changes to the symbol conversion table.\r\nDo you wish to save them?",0,MB_YESNOCANCEL);
				 	switch (opt)
				 	{   
				 		case IDCANCEL:
				 			return TRUE;
				 		case IDYES:
							PostMessage(hWndDlg, WM_COMMAND, IDC_DXFSYMCONVSAVE, 0L);
				 			return TRUE;
				 	}
				 }
                 GSSiGlobFree (&LineMem);
                 GSSiGlobFree (&PointMem);
                 HASHC(Hid); 
                 Hid = 0;
			     GSSiGlobFree (&hDXFLayers);
                 EndDialog(hWndDlg, FALSE);
                 break;  
                 
            case IDC_UNIQUEREFNO:
            {
            	 BOOL	On=SendDlgItemMessage (hWndDlg,IDC_UNIQUEREFNO,BM_GETCHECK,0,0);
            	 	 
				 EnableWindow (GetDlgItem(hWndDlg,IDC_USEIDASREF),!On); 
				 if (On)
			    	 SetDlgItemTextGlobal (hWndDlg,IDC_STARTNO,"[%STARTREFNO]","1000000");
				 else
				 	SetDlgItemText (hWndDlg,IDC_STARTNO,"1");
			}
				 break;
				 
            case IDC_DXF_CHECK:
                 if(Importing)break;
                   ContentsOnly = !ContentsOnly ;
                   if(ContentsOnly)
                   {
                     SendDlgItemMessage(hWndDlg, IDC_DXF_CURVE, BM_SETCHECK, FALSE, 0);
                     SendDlgItemMessage(hWndDlg, IDC_DXF_CHECK, BM_SETCHECK, TRUE, 0);
                     CurveFitting = FALSE;
                   }
                   else
                   {
                     SendDlgItemMessage(hWndDlg, IDC_DXF_CHECK, BM_SETCHECK, FALSE, 0);
                   }   
                   PrimeRefno = atol(string);
                 break;        
            case IDC_DXF_CURVE:
                 if(Importing)break;
                 CurveFitting = !CurveFitting;
                 if(CurveFitting)
                 {
                   SendDlgItemMessage(hWndDlg, IDC_DXF_CURVE, BM_SETCHECK, TRUE, 0);
                   SendDlgItemMessage(hWndDlg, IDC_DXF_CHECK, BM_SETCHECK, FALSE, 0);
                   ContentsOnly = FALSE;
                 }
                 else
                   SendDlgItemMessage(hWndDlg, IDC_DXF_CURVE, BM_SETCHECK, FALSE, 0);
                 break;
                      
            case IDC_LOCATE_DESTMAP: 
                 *string=0;
                 if (SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0)) 
                 {
                    if (!GetSaveName2 (hWndDlg,string,IDS_FILTERPLT,".PLT",IDS_FILEPLT)) break;   
                 }
                 else  
                 {
                    if (!GetFileName3 (hWndDlg,string,IDS_FILTERPLT,IDS_FILEPLT)) break;   
                 }
                 SetDlgItemText (hWndDlg,IDC_DESTMAP,string);
                 break; 
                 
            case IDC_SELECT_SYM:  
            {    
            	 char	SymName[34]="",CSize[64]="",CColor[64]="",CRot[64]="",str[256], Layer[66]; 
            	 int	isym, Choice,i, nItems; 
            	 LPSTR	lpTab, lpType;
            	 HANDLE	hLayers;
            	 LPINT lpItems;
            	 
                 nItems=SendDlgItemMessage(hWndDlg,IDC_LAYER_LIST,LB_GETSELCOUNT,0,0);
                 if (!nItems)
                 {
               	 	MessageBox( GetFocus(),"No layers selected",0, MB_ICONEXCLAMATION);
				 	break;
				 }  
                 hLayers = GSSiGlobAlloc ( 991,GMEM_MOVEABLE,nItems*4+4);
                 lpItems = (LPINT)GlobalLock(hLayers);
		         *lpItems = nItems;
                 SendDlgItemMessage(hWndDlg,IDC_LAYER_LIST,LB_GETSELITEMS,nItems,(LPARAM)lpItems); 
               	 SendDlgItemMessage(hWndDlg,IDC_LAYER_LIST,LB_GETTEXT,*lpItems,(DWORD)str);
               	 lpTab = _fstrchr (str,'\t');
               	 lpTab++;
               	 lpTab = _fstrchr (lpTab,'\t');
               	 lpTab++;
               	 lpType = lpTab;
               	 lpTab = _fstrchr (lpType,'\t');
               	 *lpTab = 0;
				 GlobalUnlock(hLayers);
				 if (!_fstrcmp (lpType,"POINT") ||
				 	 !_fstrcmp (lpType,"TEXT"))
		            isym=SelectPointSymbol (hWndDlg,1,SymName,"All",CSize,CRot,CColor,TRUE);  
		         else if (!_fstrcmp (lpType,"AREA"))
			     	isym=SelectAreaSymbol (hWndDlg,1,SymName, CColor,TRUE);
				 else
	             	isym=SelectLineSymbol (hWndDlg,1,SymName,CSize,CColor,TRUE);
                 lpItems = (LPINT)GlobalLock(hLayers);
	             while (nItems--)
	             {
                	  SendDlgItemMessage(hWndDlg,IDC_LAYER_LIST,LB_GETTEXT,*lpItems,(DWORD)str);
                	  lpTab=_fstrrchr (str,'\t');
                	  *lpTab=0;  
                	  lpTab=_fstrrchr (str,'\t');
                	  *lpTab=0;  
                	  sprintf (_fstrchr(str,0),"\t%s\t%i",SymName,(int)isym);
			          SendDlgItemMessage (hWndDlg,IDC_LAYER_LIST,LB_DELETESTRING,*lpItems,0);
			          SendDlgItemMessage (hWndDlg,IDC_LAYER_LIST,LB_INSERTSTRING,*lpItems++,(LPARAM)str);
	             } 
				 GSSiGlobUlFree (&hLayers);   
				 GSSiGlobFree (&hLayerSym);
				 SymConvTableChanged = TRUE;  
	        }
            	 break;
            
            case IDC_AUTOSYM:  
            {    
            	 char	SymName[34]="",str[256]; 
            	 int	isym, Choice,i, nItems; 
            	 LPSTR	lpTab, lpType;
            	 HANDLE	hLayers;
            	 LPINT lpItems;
            	 
                 nItems=SendDlgItemMessage(hWndDlg,IDC_LAYER_LIST,LB_GETSELCOUNT,0,0);
                 if (!nItems)
                 {
               	 	MessageBox( GetFocus(),"No layers selected",0, MB_ICONEXCLAMATION);
				 	break;
				 }  
                 hLayers = GSSiGlobAlloc ( 992,GMEM_MOVEABLE,nItems*4+4);
                 lpItems = (LPINT)GlobalLock(hLayers);
                 SendDlgItemMessage(hWndDlg,IDC_LAYER_LIST,LB_GETSELITEMS,nItems,(LPARAM)lpItems); 
	             while (nItems--)
	             {
                	  SendDlgItemMessage(hWndDlg,IDC_LAYER_LIST,LB_GETTEXT,*lpItems,(DWORD)str);
                	  lpTab=_fstrrchr (str,'\t');
                	  *lpTab=0;  
                	  lpTab=_fstrrchr (str,'\t');
                	  *lpTab=0;  
	               	  lpTab = _fstrchr (str,'\t');  
	               	  *lpTab = 0;
					  _fstrcpy (SymName,str); 
					  *lpTab = '\t';
	               	  lpTab++;
	               	  lpTab = _fstrchr (lpTab,'\t');
	               	  lpTab++;
	               	  lpType = lpTab;
	               	  lpTab = _fstrchr (lpType,'\t');
					  if (!_fstrcmp (lpType,"POINT") ||
					 	 !_fstrcmp (lpType,"TEXT"))
				      	SymType = 1;
				      else if (!_fstrcmp (lpType,"AREA"))
					   	SymType = 3;
					  else
			           	SymType = 2; 
					  isym = GetOrCreateSym (hWndDlg,SymName,0,0,TRUE,SymType);
                	  sprintf (_fstrchr(str,0),"\t%s\t%i",SymName,(int)isym);
			          SendDlgItemMessage (hWndDlg,IDC_LAYER_LIST,LB_DELETESTRING,*lpItems,0);
			          SendDlgItemMessage (hWndDlg,IDC_LAYER_LIST,LB_INSERTSTRING,*lpItems++,(LPARAM)str);
	             } 
				 GSSiGlobUlFree (&hLayers);
				 GSSiGlobFree (&hLayerSym);
				 SymConvTableChanged = TRUE;  
         		 EnableWindow (GetDlgItem (hWndDlg,IDC_SELECT_SYM),
         		 				(BOOL)SendDlgItemMessage(hWndDlg,IDC_LAYER_LIST,LB_GETSELCOUNT,0,0));
         		 EnableWindow (GetDlgItem (hWndDlg,IDC_AUTOSYM),
         		 				(BOOL)SendDlgItemMessage(hWndDlg,IDC_LAYER_LIST,LB_GETSELCOUNT,0,0));
	        }
            	 break;


            case IDC_LAYER_LIST:
            {
                switch(HIWORD(wParam))
                    {
                     case LBN_DBLCLK:
  				         PostMessage(hWndDlg, WM_COMMAND, IDC_SELECT_SYM, 0L); 
  				         break;
                     case LBN_SELCHANGE:
                 		 EnableWindow (GetDlgItem (hWndDlg,IDC_SELECT_SYM),
                 		 				(BOOL)SendDlgItemMessage(hWndDlg,IDC_LAYER_LIST,LB_GETSELCOUNT,0,0));
                 		 EnableWindow (GetDlgItem (hWndDlg,IDC_AUTOSYM),
                 		 				(BOOL)SendDlgItemMessage(hWndDlg,IDC_LAYER_LIST,LB_GETSELCOUNT,0,0));
					}
			}
				break; 
            case IDC_DXFSYMCONVSAVE:
            {
            	short	item=0;  
            	
	             if (!GetSaveName2 (hWndDlg,string,IDS_FILTERDSC,".DSC",IDS_FILEDSC)) break; 
	             FidDSC = GSSiOpenFile (string,&OFStruct,OF_CREATE);
	             item=0;  
              	 while (SendDlgItemMessage(hWndDlg,IDC_LAYER_LIST,LB_GETTEXT,item++,(DWORD)string) != LB_ERR)
              	 	fputstring (string,FidDSC); 
              	 GSSiClose (FidDSC); 
				 SymConvTableChanged = FALSE;  
            }
              	 break;
            
            case IDC_CLEAR:
		         SendDlgItemMessage (hWndDlg,IDC_LAYER_LIST,LB_RESETCONTENT,0,0); 
            	 break;
            	   	 
            case IDC_DXFSYMCONVRECALL: 
            {    
            	 short	i=1, iSym, index; 
            	 LPSTR	pTab, pSym;
            	 
                 if (!GetFileName3 (hWndDlg,string,IDS_FILTERDSC,IDS_FILEDSC)) break;   
	             FidDSC = GSSiOpenFile (string,&OFStruct,OF_READ);  
	             while (fgetstring (string,128,FidDSC))
	             {  
	             	pTab = _fstrrchr (string,'\t');
	             	*pTab = 0;
	             	pSym = _fstrrchr (string,'\t'); 
	             	pSym++;
	             	if (!*pSym || !(iSym = GetDictSymbolNumber (pSym)))  
	             		iSym = -i;
	             	sprintf (_fstrchr (string,0),"\t%i",(int)iSym);
	             	pSym--;
	             	*pSym = 0;		
			 		index = SendDlgItemMessage (hWndDlg,IDC_LAYER_LIST,LB_FINDSTRING,(WPARAM)-1,(LPARAM) string); 
		 			*pSym = '\t';
			 		if (index != LB_ERR)
			 		{
			 			SendDlgItemMessage (hWndDlg,IDC_LAYER_LIST,LB_DELETESTRING,(WPARAM)index,(LPARAM)0); 
			 			SendDlgItemMessage (hWndDlg,IDC_LAYER_LIST,LB_INSERTSTRING,(WPARAM)index,(LPARAM) string); 
			 		}
			 		else
						SendDlgItemMessage (hWndDlg,IDC_LAYER_LIST,LB_ADDSTRING,0,(LPARAM)((LPSTR)string));  
					i++; 
				 }
	             GSSiClose (FidDSC);
				 GSSiGlobFree (&hLayerSym);
	        }
            	 break;
            	 
            case IDOK:    
            	 DXFTextSizeAdjust=GetGlobalDVal2 ("[%DXFTSIZEFACTOR]",1.5);
                 UniqueRefno=SendDlgItemMessage (hWndDlg,IDC_UNIQUEREFNO,BM_GETCHECK,0,0);
			 	 if (GetDlgItemText (hWndDlg,IDC_STARTNO,string,128))  
			 	 {
				 	ExpandText (string);
				 	StartRefno = atol (string);
				 }  
				 else
					StartRefno = 1;
                 if (!GetDlgItemText (hWndDlg,IDC_DESTMAP,PltName,128))
                 {  
                    
                    MessageBox(GetFocus(),"No destination map", 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }
                 *curproject = 0;
                 if (!GetDlgItemText (hWndDlg,IDC_PROJECTION,curproject,lncurproject))
                 {
                    MessageBox(GetFocus(),"No input projection set", 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }
                 ScanForExtents = TRUE;
                 goto DoScan;   
            case IDC_SCAN:   
            	 ScanForExtents = FALSE;
            DoScan:
            {
            	char	str[256]; 
                    ContentsOnly = TRUE; 
                    HaveExtents = FALSE;
                    DBoundsInit (&DXFMinMax);
                  	 EnableWindow(GetDlgItem(hWndDlg, IDCANCEL),TRUE);  
                  	 SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Scanning");  
                  	 IgnoreExtents = SendDlgItemMessage (hWndDlg,IDC_IGNOREEXTENTS,BM_GETCHECK,0,0);
                     DxfIn();  
                  	 EnableWindow(GetDlgItem(hWndDlg, IDCANCEL),FALSE); 
					 GSSiGlobFree (&hLayerSym);
                  	 if (!ScanForExtents)
                  	 {
				         SendDlgItemMessage (hWndDlg,IDC_LAYER_LIST,LB_RESETCONTENT,0,0); 
				         if (!hDXFLayers)
				         	break;
				         pLayer = (LPLAYERDESC)GlobalLock (hDXFLayers);
				         for (i=0;i<NumDXFLayers;i++,pLayer+=sizeof(LAYERDESC))
				         {    
							sprintf (str,"%s\t%s\t%s\t%s\t%i",pLayer->Name,pLayer->SubName,pLayer->Command,pLayer->SymName,(int)pLayer->SymNum);
							SendDlgItemMessage (hWndDlg,IDC_LAYER_LIST,LB_ADDSTRING,0,(LPARAM)((LPSTR)str));
				         }
						 GlobalUnlock (hDXFLayers); 
						 SymConvTableChanged = TRUE;
					}  
			}        
					 if (wParam == IDC_SCAN)
					 	break;  
	Load:   
            {     
            	 short	i;
                 char	str[180];   
                 LPSTR	lpDot;   
                 double	coordcvt;
 
            
                 if(Importing)break;
                 CloseTRANS2 (&hTranFile);
                 ContentsOnly = FALSE;
                 DisableHalt = TRUE;  
                 ContinueProcessing = TRUE;
                // got = GetDlgItemText (hWndDlg,IDC_TEXT_SCALE,string,96);
                 DXFTextScale = 1;//atol(string);
                // GetDlgItemText (hWndDlg,IDC_DXF_X_OFFSET,LAYER,22);
                 XOffset = 0;//atof(LAYER);
//                 GetDlgItemText (hWndDlg,IDC_DXF_Y_OFFSET,LAYER,22);
                 YOffset = 0;//atof(LAYER);
                 got = GetDlgItemText (hWndDlg,IDC_DXF_INFILE,NAME,96);
                 if(got == 0 || NAME[0] == ' ')
                 {
                   MessageBox(0,"You Must Provide a DXF input file name.",
                   "DXF Importer",MB_ICONINFORMATION);
                   break;
                 } 
                 i=SendDlgItemMessage(hWndDlg,IDC_LAYER_LIST,LB_GETCOUNT,0,0);
                 if (!i) 
                 {
                   MessageBox(0,"No symbol conversion list",
                   "DXF Importer",MB_ICONINFORMATION);
                   break;
                 }  
                 if ((lpDot=_fstrrchr(curproject,'.')))
                        *lpDot = 0;
                 SetGlobalValue("%ALT_PROJECTION",curproject);
				 ConvertCoordClose ();
				 ConvertCoordInit();
                 GetDlgItemText (hWndDlg,IDC_UNITS,curunits,lncurunits);
                 if (*curunits)
                 { 
                        if (!_fstrcmp(curunits,"Degrees * 1000000"))
                            coordcvt = 0.000001; 
                        else if (!_fstrcmp(curunits,"Feet"))
                            PRJ_UNITS[3] = 1;
                        else if (!_fstrcmp(curunits,"Meters"))
                            PRJ_UNITS[3] = 2;
                 }
                 else
                 {
                    MessageBox(GetFocus(),"Units field not set", 0,MB_ICONQUESTION|MB_OK);
                    break;
                 }
                 
                 if (GetDlgItemText (hWndDlg,IDC_TRANFILE,str,128))
                 {
                 	hTranFile = LoadTranFileWithDandT (str);
                 	if (!hTranFile)
                	{
                    	MessageBox(GetFocus(),"Invalid transformation file", str,MB_ICONEXCLAMATION|MB_OK);
                    	break;
                    }  
                 }
                  BlocksOnly = ContentsOnly;
//                 AutoDXFSymbols = SendDlgItemMessage (hWndDlg,IDC_AUTOSYM,BM_GETCHECK,0,0);
                 i = 0;
/*				 while (SendDlgItemMessage(DXFhWndDlg,IDC_LAYER_LIST,LB_GETTEXT,i++,(DWORD)str)!=LB_ERR)
				 {
					LPSTR	lpTab, lpType;
					short	isym;
					
					lpTab = _fstrrchr (str,'\t');
					lpTab++;
					isym=atoi(lpTab);
                 	if (isym>0)
                 		AddToSymList (isym,&NumSyms,&hSymDesc);
                 } */
                 
			      _fstrupr (NAME);
			      if (_fstrstr(NAME,".TXT"))
			      {
			      	if ((FidFileList = GSSiOpenFile (NAME,lpOBF,OF_READ)) == HFILE_ERROR)
					{ 
					              
					      sprintf (string,"Error opening filelist file %s",NAME);
					      MessageBox(0,string,"DXF Reader",MB_ICONEXCLAMATION);
					      break ;
					} 
			      	MoreFiles = TRUE; 
			      	TotLenFL = GSSillseek (FidFileList,0,2);
			      	GSSillseek (FidFileList,0,0);
			      	fgetstring (NAME,128,FidFileList);
			      	ShowWindow (GetDlgItem(hWndDlg,IDC_STATUS2),SW_SHOW);
				    PctBox (GetDlgItem(hWndDlg,IDC_STATUS2), TotLenFL, 0,1);
			      }
			      else
			      	MoreFiles = FALSE; 
	NextFile:
				 _splitpath (NAME,0,0,LeafName,0); 
				 SetGlobalValue("%SOURCENAME",LeafName);
                 ContentsOnly = TRUE; 
                 HaveExtents = FALSE;
                 DBoundsInit (&DXFMinMax);
              	 SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Scanning");  
                 DxfIn();  
                 ContentsOnly = FALSE;
				 GetDlgItemText (hWndDlg,IDC_DESTMAP,PltName,128); 
				 ExpandText (PltName);
                 if (SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0)) 
                 {
                     short NumPens=10;
                     PENDESC PenDesc[10];
                     LPSYMBOL   pSym;
                     MNMXCORD MinMaxCoord;
                     DPOINT Points[4];
                     short    i;  
                    
	                 MinMaxCoord.xmn = DBL_MAX;
	                 MinMaxCoord.xmx = -DBL_MAX; 
	                 MinMaxCoord.ymn = DBL_MAX;
	                 MinMaxCoord.ymx = -DBL_MAX; 
                     Points[0].x = DXFMinMax.xmn;   
                     Points[0].y = DXFMinMax.ymn;   
                     Points[1].x = DXFMinMax.xmn;   
                     Points[1].y = DXFMinMax.ymx;   
                     Points[2].x = DXFMinMax.xmx;   
                     Points[2].y = DXFMinMax.ymx;   
                     Points[3].x = DXFMinMax.xmx;   
                     Points[3].y = DXFMinMax.ymx; 
                     for (i=0;i<4;i++)
                     {
                        if (ConvertAndTranCoord (&Points[i],hTranFile))
                        {   
                            MessageBox(GetFocus(),"Unable to convert coordinates as specified", 0,MB_ICONQUESTION|MB_OK);
                            goto ErrorEnd;
                        } 
                        MinMaxCoord.xmn = min(MinMaxCoord.xmn,Points[i].x);
                        MinMaxCoord.xmx = max(MinMaxCoord.xmx,Points[i].x);
                        MinMaxCoord.ymn = min(MinMaxCoord.ymn,Points[i].y);
                        MinMaxCoord.ymx = max(MinMaxCoord.ymx,Points[i].y);
                     }   
                     for (i=0;i<NumPens;i++)
                     {
                        PenDesc[i].PenNum = i+1;
                        PenDesc[i].Width = (float)1; 
                        PenDesc[i].Style = 1;
                        PenDesc[i].Color = RGB(0,0,0);
                     }  
                     CreateNewMap (PltName,&MinMaxCoord,0,hSymDesc,
                                                        NumPens,(LPPENDESC)&PenDesc,0,0,TRUE);
                 }
                 

				 OpenMap (CurView->hWnd,CurView->hDC);
				 EditBounds = CurView->FileMNMX;
				 CloseMap (FALSE);
                 if(!BlocksOnly)
                 {
                    _fmemset (Ignore,0,5*4);
                    ContentsOnly = FALSE;
                  	 EnableWindow(GetDlgItem(hWndDlg, IDCANCEL),TRUE);  
                  	 SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Loading");
                  	 IgnoreExtents = SendDlgItemMessage (hWndDlg,IDC_IGNOREEXTENTS,BM_GETCHECK,0,0);
                     DxfIn(); 
                     GSSiGlobFree (&hLayerSym);
					 AddSymToMap (NumSyms,hSymDesc,0,0); 
		             DestroySymList (&NumSyms,&hSymDesc);  
				     if (MoreFiles)
				     { 
				      	CurLocFL = GSSillseek (FidFileList,0,1);
					    PctBox (GetDlgItem(hWnd,IDC_STATUS2), TotLenFL, min (TotLenFL,CurLocFL),1);
				      	if (!Canceled && fgetstring (NAME,128,FidFileList))
				      	{
				      		goto NextFile;
				      	}
				      	else
				      		GSSiClose (FidFileList);
				     }
                  	 EnableWindow(GetDlgItem(hWndDlg, IDCANCEL),FALSE);
                    GSSiGlobFree (&LineMem);
                    GSSiGlobFree (&PointMem);
                    HASHC(Hid); 
                    Hid = 0;
        			GSSiGlobFree (&hDXFLayers); 
        			HaveIgnore = FALSE;
        			_fstrcpy (str,"The following entities are not supported in GeoMaster and were ignored:\r\n"); 
        			for (i=0;i<5;i++)
        				if (Ignore[i])                                                   
        				{
        					HaveIgnore = TRUE;
        					sprintf (_fstrchr (str,0),"\r\n%s (%ld)",IgnoreType[i],Ignore[i]);
        				}
        			if (HaveIgnore)
        				MessageBox (hWndDlg,str,"Not all entities processed",MB_ICONEXCLAMATION);
                    EndDialog(hWndDlg, FALSE); 
                 }
        ErrorEnd:; 
                 HASHC(Hid); 
                 Hid = 0;
                 CloseTRANS2 (&hTranFile);
				 GSSiGlobFree (&hLayerSym);
        		 GSSiGlobFree (&hDXFLayers);
              }  
                 DisableHalt = FALSE;  
                 ContinueProcessing = TRUE;
             	 if (*AutoExportName)
             	 {
					DestroyAdvancedOpts ();
                 	EndDialog(hWndDlg, TRUE);
                 } 
              
                 break;
      /*      case IDC_CHECKBACK:
                 DxfIn();
                 break;      */
            case IDC_DXF_INFILE:
                 if(Importing)break;
                GSSiGlobFree (&LineMem);
                GSSiGlobFree (&PointMem);
              //   got = GetDlgItemText (hWndDlg,IDC_DXF_INFILE,NAME,96);
                 break;
            case IDC_LOCATE_SOURCE:
                if(Importing)break;
                GSSiGlobFree (&LineMem);
                GSSiGlobFree (&PointMem);
                 NumPoints = 0;
                 NumLines = 0;
                 *NAME=0;
                 if (!GetFileName3 (hWndDlg,NAME,IDS_FILTERDXF,IDS_FILEDXF))
                 	break;   
                 SetDlgItemText (hWndDlg,IDC_DXF_INFILE,NAME);
                 break;
            
           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} 






