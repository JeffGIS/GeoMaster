#include "graphint.h"   
#include "translat.h"
#include "extrndb.h"   
#include "pnet.h"

#include "gmextern.h"

static	TAGKEY TAGKey;
static struct {long   TLID;
     short    Type;
     long   StreetNum;
     } Names1Key;
static struct {
     short    Type;
     long   StreetNum;
     } Names2Data; 
static HANDLE		hIntersect = 0;
static HFILE	FidTIGER1;
static HFILE	FidTIGER2;
static long	FeatID;
static short	PrimeName;
static HANDLE	hTiger1BT;
static HFILE	hTiger1Data;
static	struct	{
			long Long, Lat, TLID;
		}	IntersectKey; 
static	struct	{
			long	Snum;
			long	OPLong, OPLat;
		}	IntersectData;


BOOL FAR PASCAL LOADBNAMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 
    short   i;
    char    Ext[8], ExtID[34], str[260];
    LPSTR    lpstr=str;  
    HCURSOR OldCursor=0;    
    static   HANDLE hSQL=0;
    long        TotLen, CurLoc, County, NumCounties;
    FILE        *FidBNA;  
    LPLONG      pCounty, pCnty;  
    HANDLE      hCounties; 
    static      short     FileType = 0;
/* Sample data  
"27001","AITKIN","527001","county",395
-93.811766, 46.500000 -93.811693, 46.496375 -93.811507, 46.487099
-93.811437, 46.483625 -93.811414, 46.482483 -93.811402, 46.481905

"270019901.00","9901.00","3270019901.00","tract",862
-93.273181, 46.758028 -93.272593, 46.758502 -93.268033, 46.762597
-93.264812, 46.754008 -93.254338, 46.751282 -93.250267, 46.753054
-93.236524, 46.766906 -93.233334, 46.769330 -93.230801, 46.771326

"271090001.001","1","2271090001.001","blkgrp",27
-92.468000, 44.027800 -92.468100, 44.027400 -92.468000, 44.027000
-92.468100, 44.026200 -92.468100, 44.025300 -92.468100, 44.024400
-92.468100, 44.023400 -92.467300, 44.023500 -92.465800, 44.023500
-92.464400, 44.023500 -92.462900, 44.023500 -92.462900, 44.022500
-92.462900, 44.021500 -92.461300, 44.021500 -92.459900, 44.021600
-92.459600, 44.021200 -92.456200, 44.020500 -92.457200, 44.023500
-92.458000, 44.024300 -92.458600, 44.025200 -92.459100, 44.026000
-92.459800, 44.026200 -92.461400, 44.026500 -92.462900, 44.026800
-92.464400, 44.027100 -92.465800, 44.027400 -92.468000, 44.027800
"271090001.002","2","2271090001.002","blkgrp",24
-92.468100, 44.023400 -92.468100, 44.022500 -92.468100, 44.021500
-92.468200, 44.020300 -92.468200, 44.019300 -92.468000, 44.018000
-92.468100, 44.016600 -92.467400, 44.016700 -92.465900, 44.016700
-92.464500, 44.016700 -92.463000, 44.016800 -92.462600, 44.016800
-92.461800, 44.019300 -92.461500, 44.020100 -92.459600, 44.021200
-92.459900, 44.021600 -92.461300, 44.021500 -92.462900, 44.021500
-92.462900, 44.022500 -92.462900, 44.023500 -92.464400, 44.023500
-92.465800, 44.023500 -92.467300, 44.023500 -92.468100, 44.023400
  
"271370001.00101A","101A","271370001.00101A","blk",9
-92.047900, 46.865600 -92.047800, 46.864000 -92.047000, 46.863700
-92.046000, 46.864600 -92.043900, 46.863000 -92.041800, 46.863400
-92.041700, 46.864500 -92.040700, 46.865700 -92.047900, 46.865600
"271370001.00101B","101B","271370001.00101B","blk",13
-92.038600, 46.865700 -92.037700, 46.865100 -92.035900, 46.865100
-92.036500, 46.863700 -92.034700, 46.860100 -92.033200, 46.860200
-92.031200, 46.858400 -92.028700, 46.855600 -92.015700, 46.861200
-92.016000, 46.865700 -92.017400, 46.865700 -92.031800, 46.865700
-92.038600, 46.865700
"271370001.00101C","101C","271370001.00101C","blk",12
-92.047800, 46.869400 -92.047900, 46.865600 -92.040700, 46.865700
-92.039700, 46.866100 -92.038600, 46.865700 -92.031800, 46.865700
-92.017400, 46.865700 -92.017300, 46.869100 -92.031800, 46.868500
-92.037056, 46.868448 -92.037500, 46.868500 -92.047800, 46.869400

"0900313435","Central Manchester","60900313435","fplace",129
-72.562200, 41.780900 -72.559500, 41.780400 -72.557800, 41.779700
-72.555300, 41.773400 -72.557100, 41.772900 -72.558700, 41.772000
-72.560700, 41.768500 -72.560400, 41.768200 -72.559700, 41.767800

*/
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:
         
         SendDlgItemMessage (hWndDlg,IDC_ALL_COUNTIES,BM_SETCHECK,TRUE,0L);
         
         cwCenter(hWndDlg, 0);
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
#if WIN32
         switch(LOWORD(wParam))
#else
         switch(wParam)
#endif
         {  
            case IDC_LOCATE_SOURCE: 
            {    
                BOOL    HaveMID=FALSE;
                LPTAGDEF    lpTAGDef; 
                short     nFields; 
                LPSTR   lpSpace, lpDelim; 
                    
                 _fstrcpy (Ext,".BNA"); 
                 _fstrcpy (ExtID,"BNA Files");
                 sprintf (gszFilter,"%s(*%s)|*%s|",ExtID,Ext,_fstrlwr(Ext));
                 if (GetFileName3(hWndDlg,LoadName,0,IDS_FILEBNA))   
                 {   
                 	 ExpandText (LoadName);
                     FidBNA=fopen(LoadName,"rt");
                     if (!FidBNA)
                     {
                        GSSiMsgBox(GetFocus(),"Unable to open source file", 0,MB_ICONQUESTION|MB_OK);
                        break;
                     }
                     fgetss(str,256,FidBNA);  
                     fclose (FidBNA);    
                     lpDelim = _fstrrchr (str,'"');
                     *lpDelim = 0;
                     lpDelim = _fstrrchr (str,'"');
                     lpDelim++;
                     if (!_fstricmp (lpDelim,"county"))
                        FileType = 1;
                     else if (!_fstricmp (lpDelim,"tract"))
                        FileType = 2;
                     else if (!_fstricmp (lpDelim,"blkgrp"))
                        FileType = 3;
                     else if (!_fstricmp (lpDelim,"blk"))
                        FileType = 4;  
                     else if (!_fstricmp (lpDelim,"fplace"))
                        FileType = 5;  
                     else
                     {
                        FileType = 0; 
                        sprintf (str,"Unrecognized file type: %s",lpDelim);
                        GSSiMsgBox(GetFocus(),str, 0,MB_ICONQUESTION|MB_OK);
                        break;
                     }   
                     SetDlgItemText (hWndDlg,IDC_FILE,LoadName); 
                     SendDlgItemMessage (hWndDlg,IDC_ALL_COUNTIES,BM_SETCHECK,TRUE,0L);
                     SendDlgItemMessage (hWndDlg,IDC_SELECT_COUNTIES,BM_SETCHECK,FALSE,0L);
                     SendDlgItemMessage (hWndDlg,IDC_COUNTY_LIST,LB_RESETCONTENT,0,0);
                     
                 }
            }
                 break;
                 
            case IDC_LOCATE_DESTMAP: 
                 *str=0;  
                 if (SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0))                   
                 {
                    if (!GetSaveName2 (hWndDlg,str,IDS_FILTERPLT,".PLT",IDS_FILEPLT)) break;   
                 }
                 else  
                 {
                    if (!GetFileName3 (hWndDlg,str,IDS_FILTERPLT,IDS_FILEPLT)) break;   
                 }
                 SetDlgItemText (hWndDlg,IDC_DESTMAP,str);
                 break;
                 
            case IDC_ALL_COUNTIES:
                 SendDlgItemMessage (hWndDlg,IDC_COUNTY_LIST,LB_RESETCONTENT,0,0);
                 break;
                 
            case IDC_SELECT_COUNTIES:
            {    
            	 char	Name[128];
            	 
                 SendDlgItemMessage (hWndDlg,IDC_COUNTY_LIST,LB_RESETCONTENT,0,0);  
                 SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Scanning for Counties");
                 GetDlgItemText (hWndDlg,IDC_FILE,Name,sizeof(Name));  
                 ExpandText (Name);
                 FidBNA=fopen(Name,"rt");
                 if (!FidBNA)
                 {
                    GSSiMsgBox(GetFocus(),"Unable to open source file", 0,MB_ICONQUESTION|MB_OK);
                    break;
                 }
                 fseek (FidBNA,0,SEEK_END);
                 TotLen = ftell (FidBNA);  
                 fseek (FidBNA,0,SEEK_SET);   
                 hCounties = GSSiGlobAlloc ( 411,GHND,4096);
                 NumCounties = 0;
                 pCounty = (LPLONG)GlobalLock (hCounties);
                 while (fgetss(str,256,FidBNA))
                 {  
                    if (*str == '"')
                    {   
                        County = ldread (&str[1],5);  
                        for (i=0,pCnty=pCounty;i<NumCounties;i++,pCnty++)
                        {
                            if (County == *pCnty)
                                goto GotCounty;
                        } 
                        NumCounties++;
                        *pCnty = County;
                GotCounty:
                        CurLoc = ftell (FidBNA);       
                        PctBox (GetDlgItem(hWndDlg,IDC_STATUS), TotLen, CurLoc,0);
                    }
                 } 
                 PctBox (GetDlgItem(hWndDlg,IDC_STATUS), TotLen, TotLen,0);
                 for (i=0,pCnty=pCounty;i<NumCounties;i++,pCnty++)
                 {  
                    sprintf (str,"%5.5ld   %s",*pCnty,"Hennepin County, MN");
                    SendDlgItemMessage (hWndDlg,IDC_COUNTY_LIST,LB_ADDSTRING,0,(LPARAM)str);
                 }
                 GlobalUnlock (hCounties);
                 GlobalFree (hCounties);
                 fclose (FidBNA); 
                 SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"");
                 PctBox (GetDlgItem(hWndDlg,IDC_STATUS), TotLen, -1,0);
                 
            }
                 break;
                 
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 if (Processing)
                    ContinueProcessing=FALSE;
                 else
                    EndDialog(hWndDlg, FALSE);
                 break;
            
            case IDOK: 
            {
                 MNMXCORD MinMaxCoord;  
                 long   lineno=0, TotLen, CurLoc, MidLine;  
                 long   NewRefno; 
                 short    st, SymNum, iUDI=0, AreaSym, LineSym, Pass;
                 BOOL   Done; 
                 HANDLE hMIDstr;
                 char   Prefix[10], UDI[34], AreaSymName[10], LineSymName[10]; 
                 LPSTR  lpComma;  
                 OFSTRUCT   OFStruct;
                 long       filecode,filelength,ii=100, NumOverLimit;  
                 HFILE  FidBNA;
                 char   mess[256];  
                 short        nPoly,lastnpoints, Symbol, MaxSides,j;
                 long   nVertex;
                 DPOINT	LinkPoint, FirstIslandPoint;
                 HANDLE hPoly[256];  
#if WIN32
                 DPOINT  *pPoints;
#else                  
                 DPOINT huge *pPoints;
#endif                 
                 HANDLE     hPoints, hnPoints;
                 WORD     	nPoints[256];
                 short      NumPoints, Nump;  
                 short    NumSyms=0, n, stuff[128];  
                 HANDLE hSymDesc=0; 
                 BOOL   Store,WantFirstPoint;   
                 char   LSym[20], Name[128];
                 LPSTR  lpBeg, lpEnd, lpSpace;
                                          
                                        
                 if (!GetDlgItemText (hWndDlg,IDC_DESTMAP,PltName,128))
                 {  
                    
                    GSSiMsgBox(GetFocus(),"No destination map", 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }
                 GetDlgItemText (hWndDlg,IDC_FILE,Name,sizeof(Name)); 
                 FidBNA=GSSiOpenFile (Name,&OFStruct,OF_READ);
                 if (FidBNA == HFILE_ERROR)  
                 {  
                    sprintf (mess,"Unable to open file %s",Name);
                    GSSiMsgBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }
                 PltType = 2; 
                 switch (FileType)
                 {
                    case 1:
                        _fstrcpy (Prefix,"COUNTY");
                        _fstrcpy (LSym,"COUNTYBL");
                        break;
                    case 2:
                        _fstrcpy (Prefix,"CENTRACT");
                        _fstrcpy (LSym,"CENTRABL");
                        break;
                    case 3:
                        _fstrcpy (Prefix,"BLOCKGRP");
                        _fstrcpy (LSym,"BLOCKGBL");
                        break;
                    case 4:
                        _fstrcpy (Prefix,"CENBLOCK");
                        _fstrcpy (LSym,"CENBLOBL");
                        break;
                    case 5:
                        _fstrcpy (Prefix,"CENPLACE");
                        _fstrcpy (LSym,"CENPLACE");
                        break;
                 } 
                 AreaSym = GetDictSymbolNumber (Prefix);
                 LineSym = GetDictSymbolNumber (LSym);
                 if (AreaSym)
                    AddToSymList (AreaSym,&NumSyms,&hSymDesc);
                 else
                 {
                    sprintf (mess,"Unable to load area symbol: %s",Prefix);
                    GSSiMsgBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }
                 if (LineSym)
                    AddToSymList (LineSym,&NumSyms,&hSymDesc); 
                 else
                 {
                    sprintf (mess,"Unable to load line symbol: %s",LSym);
                    GSSiMsgBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }
                                 
                 if (SendDlgItemMessage (hWndDlg,IDC_SELECT_COUNTIES,BM_GETCHECK,0,0))
                 {   
                    long nItems;
                    HANDLE hItems;  
                    LPSHORT   lpItems;
                    
                     nItems=SendDlgItemMessage(hWndDlg,IDC_COUNTY_LIST,LB_GETSELCOUNT,0,0);
                     if (!nItems)
                     {
                        GSSiMsgBox( GetFocus(), "No Counties Selected",0, MB_OK);
                        break;
                     } 
                     hItems = GSSiGlobAlloc ( 412,GMEM_MOVEABLE,nItems*2);
                     lpItems = (LPSHORT) GlobalLock (hItems); 
                     SendDlgItemMessage(hWndDlg,IDC_COUNTY_LIST,LB_GETSELITEMS,(WPARAM)nItems,(LPARAM)lpItems); 
                     hCounties = GSSiGlobAlloc ( 413,GHND,nItems*4);
                     pCounty = (LPLONG)GlobalLock(hCounties);      
                     NumCounties = nItems;
                     for (i=0;i<nItems;i++,pCounty++)
                     {
                        SendDlgItemMessage(hWndDlg,IDC_COUNTY_LIST,LB_GETTEXT,*lpItems,(LPARAM)str); 
                        *pCounty = atol (str);
                     }
                     GlobalUnlock (hCounties); 
                     GlobalUnlock (hItems);
                     GlobalFree (hItems);
                 }
                 else
                    hCounties = 0; 
                 TotLen = GSSillseek (FidBNA,0,2);  
                 ExpandText (PltName);
                 PltType = 2;
                 Done = FALSE;
                 MidLine=0; 
                 str[0]=0;  
                 if (SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0))
                    Pass = 0;
                 else
                    Pass = 1; 
                 
                 SaveFPT = FileProjectionType; 
				 FileProjectionType=0;
                 DisableHalt = TRUE;
                 Processing = TRUE; 
NextPass:        
				 NumOverLimit = MaxSides = 0;
                 GSSillseek (FidBNA,0,0);
                 if (Pass)
                 {
                    SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Loading Data");
					EditBounds = CurView->FileMNMX;
				 }                    
                 else
                 {       
                    SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Scanning for min/max coordinates");
                    MinMaxCoord.xmn = DBL_MAX;
                    MinMaxCoord.xmx = -DBL_MAX; 
                    MinMaxCoord.ymn = DBL_MAX;
                    MinMaxCoord.ymx = -DBL_MAX; 
                 }       
                                    
                 NewRefno=0;
NextLine:       
                if (!ContinueProcessing)
                    goto EndFile;
                if (!fgetstring (str,128,FidBNA))
                    goto EndFile; 
                lineno++; 
                if (!(lpComma = _fstrrchr (lpstr,',')))
                    goto ErrorEnd;
                NumPoints = (short)atol (++lpComma);
                if (hCounties)
                {   
                    Store = FALSE;
                    County = ldread (&str[1],5);  
                    pCounty = (LPLONG)GlobalLock (hCounties);
                    for (i=0;i<NumCounties;i++,pCounty++)
                    {
                        if (County == *pCounty)
                            Store = TRUE;  
                    }
                    GlobalUnlock (hCounties); 
                }
                else
                    Store = TRUE; 
               	MaxSides = max (MaxSides,NumPoints);
                if (NumPoints > 16000)  
                {
                	NumOverLimit++;
                    Store = FALSE; 
                } 
                if (NumPoints < 0)
                	Store = FALSE;
                lpBeg = lpstr+1;
                lpEnd = _fstrchr (lpBeg,'"');
                *(lpEnd++) = 0;
                _fstrcpy (UDI,lpBeg);
                stuff[0]=0;
                if (FileType == 1)
                {
                 	short lcmdstring;
                 	char	CmdString[256]="[COUNTY_NAME]=";
                 	
                	lpBeg = lpEnd+2;
	                lpEnd = _fstrchr (lpBeg,'"');
	                *lpEnd++ = 0;
		                     	 
                 	_fstrcat (CmdString,lpBeg);
                 	lcmdstring = _fstrlen (CmdString);
                 	lcmdstring += lcmdstring%2; 
                    stuff[1]=40;
                    stuff[2]=lcmdstring;
                    _fstrncpy ((LPSTR)&stuff[3],CmdString,lcmdstring);
                    stuff[0]+=2+2+lcmdstring; 
                    GlobalUnlock (hGRCommand);
                }
                if (SendDlgItemMessage (hWndDlg,IDC_REMOVE_DOT,BM_GETCHECK,0,0)) 
                	Strip (UDI,'.');
           		hPoly[0] = GSSiGlobAlloc ( 414,GMEM_MOVEABLE,sizeof(DPOINT)*(long)NumPoints);
                pPoints = (LPDPOINT)GlobalLock (hPoly[0]); 
				nPoly = 0;      
                n = NumPoints; 
                WantFirstPoint=FALSE;
                Nump = 0;
                while (n)
                {   
                    
                    if (!fgetstring (str,128,FidBNA))
                        goto ErrorEnd;   
                    lineno++;
                    lpEnd = _fstrchr (str,0);
                    lpBeg = str; 
                    nPoints[0] = 0;
                    while (lpBeg < lpEnd)
                    {
                        if ((lpComma = _fstrchr (lpBeg,',')))
                        {
                            *lpComma++ = 0;
                            lpComma = _fstrpbrk (lpComma,"+-0123456789."); 
                            lpSpace = _fstrchr (lpComma,' ');
                            if (!lpSpace)
                                lpSpace = lpEnd;
                            pPoints->x = atof (lpBeg);
                            pPoints->y = atof (lpComma);
                            nPoints[nPoly]++;  
                            if (n == NumPoints)
                            {
                            	LinkPoint = *pPoints;
                            	FirstIslandPoint = LinkPoint;
                            }
                            else if (WantFirstPoint)
                            {
                            	FirstIslandPoint = *pPoints;
                            	WantFirstPoint=FALSE;
							}                            
                            else
                            {
                            	if (fabs (FirstIslandPoint.x - pPoints->x) < 0.00000001 &&   
                            		fabs (FirstIslandPoint.y - pPoints->y) < 0.00000001) 
                            	{
                            		if (nPoly)
                            		{  
                            			pPoints++;
                            			*pPoints = LinkPoint;  
                            			nPoints[nPoly]++; 
                            		}
                            		GlobalUnlock (hPoly[nPoly]); 
                            		nPoly++; 
                            		nPoints[nPoly] = 0;
			                   		hPoly[nPoly] = GSSiGlobAlloc ( 415,GMEM_MOVEABLE,sizeof(DPOINT)*(long)NumPoints);
				                    pPoints = (LPDPOINT)GlobalLock (hPoly[nPoly]); 
                            		
                            		WantFirstPoint=TRUE;
                            	}
                            } 
                            pPoints++;
                            n--; 
                            lpBeg = lpSpace+1; 
                        }
                        else
                            lpBeg = lpEnd;
                    }
                }
                GlobalUnlock (hPoly[nPoly]); 
                if (nPoly < 1)
                	Store = FALSE;
                
                if (Store)
                {
                    for (j=0;j<nPoly;j++)
                    {
	                    pPoints = (LPDPOINT)GlobalLock (hPoly[j]); 
	                    for (i=0;i<nPoints[j];i++,pPoints++)
	                    {   
	                        ConvertCoord (pPoints,2,1);
	                        Store = PointInFileBounds (pPoints,&MinMaxCoord,Pass);
	                    }
	                    GlobalUnlock (hPoly[j]);  
	                }
                } 
                if (Pass)
                {
					 NewRefno = GetNewRefno(PltName,NULL,NULL,NULL,NULL); 
//                     if (nPoly>1)
//                     	nPoly = -nPoly;
                     if (Store)     
                        AddPolyToMap (nPoly,
                                      nPoints, 
                                      hPoly,0,NewRefno,NULL,-1,AreaSym,stuff,Prefix,UDI,
                                        -1,0,0,0,0,0,0,FALSE); 
                 }
                 while (nPoly--)
                 	GlobalFree (hPoly[nPoly]); 
                 CurLoc = GSSillseek (FidBNA,0,1);       
                 PctBox (GetDlgItem(hWndDlg,IDC_STATUS), TotLen, CurLoc,0);
                 goto NextLine;
                 
        ErrorEnd: 
                 {  
                    char    mess[256];
                    
                    sprintf (mess,"Error in file at line %ld\r\n%s",lineno,str);
                    GSSiMsgBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
                    Pass = 2;
                 }
                    
        EndFile: 
                 CloseMap (TRUE);
                 CloseRefIndex(FALSE);           
                 if (!Pass && ContinueProcessing)
                 {
                     short NumPens=10;
                     PENDESC PenDesc[10];
                     
                     for (i=0;i<NumPens;i++)
                     {
                        PenDesc[i].PenNum = i+1;
                        PenDesc[i].Width = (float)1.0; 
                        PenDesc[i].Style = 1;
                        PenDesc[i].Color = RGB(0,0,0);
                     }
                     CreateNewMap (PltName,&MinMaxCoord,NumSyms,hSymDesc,
                                                        NumPens,(LPPENDESC)&PenDesc,0,0,TRUE); 
                    Pass = 1;
                    lineno = 0;
                    goto NextPass;
                 }
                 GSSiClose (FidBNA);  
				 CloseMap(TRUE);
                 if (hCounties)
                    GlobalFree (hCounties);
                 DestroySymList (&NumSyms,&hSymDesc);
                 
                 sprintf (mess,"Load complete. %ld areas skipped due to too many points. Largest area had %i points",
                 				NumOverLimit,MaxSides);
                 GSSiMsgBox( GetFocus(),mess,"", MB_OK);
                 DisableHalt = FALSE;   
                 Processing = FALSE;
                 FileProjectionType = SaveFPT; 
                 EndDialog(hWndDlg, TRUE); 
                 break;
                 
            }   
          }
          break;

    default:
        return FALSE;
   }
 return TRUE;    
} 

BOOL GuessProjection (HWND hWndDlg,UINT ProjCntl,UINT ProjUnits,LPMNMXCORD pFileBounds,BOOL UseFirst)
{
	MNMXCORD	BoundsInt, FileBoundsCVT, ProjBounds;
	short	PCTIn[32][2], UsedUnits[32][2], CVTUnits[32], CvtID=0, UnitsID;  
	short	nFound=0, MaxFound=0, MaxID, MaxUnits, Err;   
	char	SaveAltProj[34], str[256];  
	DPOINT	Points[4]; 
	short	i;

	GetGlobalCVal ("[%ALT_PROJECTION]",SaveAltProj,NULL);
	GetGlobalCVal ("[%PROJECTBOUNDS]",str,NULL);
	ProjBounds = atobounds (str,&Err);
	 
	while (SendDlgItemMessage (hWndDlg,ProjCntl,CB_GETLBTEXT,(WPARAM)CvtID,(LPARAM)str) != CB_ERR)
	{   
		SetGlobalValue("%ALT_PROJECTION",str);
		PCTIn[CvtID][0] = 0;
		PCTIn[CvtID][1] = 0;  
		UnitsID = 0;
		ConvertCoordClose ();
		ConvertCoordInit();
		if (PRJ_UNITS[3] == 1 || PRJ_UNITS[3] == 2)  
			CVTUnits[CvtID] = 1;
		else
			CVTUnits[CvtID] = PRJ_UNITS[3];
BeginUnits:
		PRJ_UNITS[3] = CVTUnits[CvtID];  
		UsedUnits[CvtID][UnitsID] = PRJ_UNITS[3]-1;
		DBoundsInit (&FileBoundsCVT);
		Points[0].x = pFileBounds->xmn;   
		Points[0].y = pFileBounds->ymn;   
		Points[1].x = pFileBounds->xmn;   
		Points[1].y = pFileBounds->ymx;   
		Points[2].x = pFileBounds->xmx;   
		Points[2].y = pFileBounds->ymx;   
		Points[3].x = pFileBounds->xmx;   
		Points[3].y = pFileBounds->ymn;   
		for (i=0;i<4;i++)
		{
			if (ConvertCoord(&Points[i],3,1))
			{   
			    goto NextUnits;
			}
			AddDPointToMinMax (&Points[i],&FileBoundsCVT); 
		} 
		if (IntersectBounds (&ProjBounds,&FileBoundsCVT,&BoundsInt)) 
		{
			PCTIn[CvtID][UnitsID] = IDNINT(10000 * BoundsArea(&BoundsInt) / BoundsArea(&FileBoundsCVT));  
			if (PCTIn[CvtID][UnitsID])
			{  
				if (PCTIn[CvtID][UnitsID] > MaxFound)
				{
					MaxFound = PCTIn[CvtID][UnitsID]; 
					MaxID = CvtID;
					MaxUnits = UnitsID;
				}
				nFound++;
			}
		}
NextUnits:
		if (CVTUnits[CvtID] == 1)
		{
			CVTUnits[CvtID]++;  
			UnitsID++;
			goto BeginUnits;
		}
		CvtID++;
		} 
		if (UseFirst)
			nFound = min (nFound,1);
		switch (nFound)
		{	         
		case 1:
			SendDlgItemMessage (hWndDlg,ProjCntl,CB_SETCURSEL,(WPARAM)MaxID,(LPARAM)NULL); 
			SendDlgItemMessage (hWndDlg,ProjUnits,CB_SETCURSEL,(WPARAM)min(2,UsedUnits[MaxID][MaxUnits]),(LPARAM)NULL); 
		break;
		case 0:
			GSSiMsgBox (hWndDlg,"None of the supplied projections fit this data","",MB_ICONEXCLAMATION);
		break;
		default:
		{   
			char	PrjName[32], UnitsName[64];
			short	i,j;
			char	UnitsNames[4][8]={"Feet","Meters","Degrees","Degrees"};			 		 		
			*str = 0;
			for (i=0;i<CvtID;i++)
				for (j=0;j<2;j++)
					if (PCTIn[i][j])
					{   
						SendDlgItemMessage (hWndDlg,ProjCntl,CB_GETLBTEXT,(WPARAM)i,(LPARAM)PrjName);
						sprintf (_fstrchr(str,0),"%s - %s (%.0f percent)\r\n",PrjName,UnitsNames[UsedUnits[i][j]],(double)PCTIn[i][j]/100);
					}
			GSSiMsgBox (hWndDlg,str,"Multiple projections fit this data",MB_ICONEXCLAMATION); 
		}
		} 
	SetGlobalValue("%ALT_PROJECTION",SaveAltProj);
	return TRUE;
}  

BOOL GuessShapeProjection (LPSTR Name,HWND hWnd,UINT ProjCntl,UINT ProjUnits,BOOL UseFirst)
{
	SHPHEADER	SHPHeader;
	HFILE		FidSHP;
	MNMXCORD	SHPBounds;
	
	switch (MapFileType (Name))
	{
		case MT_SHP:
			FidSHP=GSSiOpenFile (Name,NULL,OF_READ);
			if (FidSHP == HFILE_ERROR)  
				return FALSE;  
			BigRead (FidSHP,(HPSTR)&SHPHeader,(UINT)sizeof(SHPHeader));
			GSSiClose (FidSHP);      
			SHPBounds = *(LPMNMXCORD)&SHPHeader.Xmin;
		break;
		
		case MT_PERSONAL_GEO_DB: 
			ReadPGDBHeader (Name,&SHPBounds);
		break;
	}  
	return GuessProjection (hWnd,ProjCntl,ProjUnits,&SHPBounds,UseFirst);

}  

BOOL FAR PASCAL LOADSHPMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LPARAM lParam)
{ 
    char		Ext[6]=".TL2";
    LPTAGDEF    lpTAGDef; 
    short     	i;
    char    	Prefix[10], SHPExt[8], ExtID[34], SymName[256], drive[4], file[34];
    LPSTR   lpDot;  
    HCURSOR OldCursor=0;    
    static   HANDLE hSQL=0;
    LPGWFLDINFO lpGWFldInfo;
    LPGWDHEADER lpGWDHead;
    HANDLE      hBT;
    long        Offset,ii; 
    double      rtn;
    LPVOID      lpVal; 
    short       st, len,ifield,UnitsOpt;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr;
    LPFIELDINFO lpFieldInfo; 
    static		SHPHEADER SHPHeader;
    HANDLE      SaveHandle;
    OFSTRUCT   OFStruct;
    BOOL        More,HiPrecis;   
    float		size,rot;
    short         rc; 
    BOOL		NewOpt,Err;
    static	BOOL	FileIsOpen;
	short		SaveDrive, idrive;
	HANDLE	hMem=0;
    LPSTR    	str, dir;
    LPSTR       UDI, Name;
    LPSTR    	CSize,CRot,CColor; 
    LPSTR		mess, CmdString; 
    LPSHORT		stuff;
    MNMXCORD	ProjBounds;
    MNMXCORD 	MinMaxCoord;
    DPOINT 		Points[4];
	char	SaveAltProj[34];
	static	HANDLE	hSaveBM=0;  
	BOOL	SaveAllowCache=AllowCache;
    

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 if ((BRtn = ImportCommonCode (hWndDlg,Message, wParam, lParam,hSQL))) return (BRtn);  
 hMem = GSSiGlobAlloc ( 416,GMEM_MOVEABLE,2048+1024);
 str = GlobalLock (hMem);
 dir = str + 1024;
 UDI = dir + 128;
 Name = UDI + 128;
 CSize = Name + 256;
 CRot = CSize + 128;
 CColor = CRot + 128;  
 mess = CColor + 128; 
 CmdString = mess + 256;
 stuff = (LPSHORT) (CmdString + 256);   
 
 AllowCache = FALSE;
//    char    	str[256], dir[128];
//    char        UDI[128], Name[128];
//    char    CSize[128],CRot[128],CColor[128];
//   	 char	CmdString[256];
 switch(Message)
   {
    case WM_INITDIALOG:
    	 hSaveBM = EnterBlockingWindow (hWndDlg);
         FileIsOpen = FALSE;
         if (NumTAGDef>0)
         { 
	         lpTAGDef = (LPTAGDEF)GlobalLock (hTAGDef);
	         for (i=0;i<NumTAGDef;i++,lpTAGDef++) 
	            SendDlgItemMessage (hWndDlg,IDC_TAPREFIX,CB_ADDSTRING,0,(LPARAM)lpTAGDef->Prefix);
	         GlobalUnlock (hTAGDef); 
	     }
         SendDlgItemMessage (hWndDlg,IDC_HIPRECIS,BM_SETCHECK,TRUE,0);
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Feet");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Meters");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Degrees");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Degrees * 1000000");
         _fstrcpy (str,"*.CVT");
         DlgDirListComboBox (hWndDlg,str,IDC_PROJECTION,0,DDL_READWRITE);   
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_SELECTSTRING,(WPARAM)-1,(LPARAM)curunits);
		 if (PRJ_UNITS[1] == 1)
 		 	SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_SETCURSEL,(WPARAM)0,(LPARAM)NULL); 
		 else if (PRJ_UNITS[1] == 2)
 		 	SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_SETCURSEL,(WPARAM)1,(LPARAM)NULL); 
 		 SendDlgItemMessage (hWndDlg,IDC_PROJECTION,CB_SELECTSTRING,(WPARAM)-1,(LPARAM)"baseproj"); 
         if (GetGlobalCVal ("[%PROJECTBOUNDS]",str,NULL))
         {
 			ProjBounds = atobounds (str,&Err); 
 			if (!Err)
         		EnableWindow (GetDlgItem(hWndDlg,IDC_ASSIGNCVT),TRUE);
         } 
         GetGlobalCVal ("[%STARTREFNO]",str,"1000000"); 
         SetDlgItemText (hWndDlg,IDC_STARTNO,str);
         EnableWindow (GetDlgItem(hWndDlg,IDC_GET_SYM),FALSE);
         if (*AutoExportName)
		 	PostMessage(hWndDlg, WM_COMMAND, IDC_RECALL, 0L);
    case GSSI_REINITDIALOG: 
    {
    	 char	CheckMark;
    	 
         if (hStreetSegFields)
         	CheckMark = 'X';
         else
         	CheckMark = ' ';
         sprintf (str,"%c\tStreet Segment Data",CheckMark);
         SendDlgItemMessage (hWndDlg,IDC_ADVANCED_OPTS,LB_ADDSTRING,0,(LPARAM)str);
         if (hAttImport)
         	CheckMark = 'X';
         else
         	CheckMark = ' ';
         sprintf (str,"%c\tImport Attributes",CheckMark);
         SendDlgItemMessage (hWndDlg,IDC_ADVANCED_OPTS,LB_ADDSTRING,0,(LPARAM)str);
         if (FileIsOpen && *AutoExportName)
	         PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);     
	}
         break; /* End of WM_INITDIALOG                                 */

    case WM_COMMAND:
#if WIN32
         switch(LOWORD(wParam))
#else
         switch(wParam)
#endif
         {  
            case IDC_GET_SYM:
				switch (SHPHeader.ShapeType)
				{   
					case 8:
					case 1: //points   
						DecodePointSym (SymStuff,SymName,CSize,CRot,CColor);
                 		if (!SelectPointSymbol (hWndDlg,1,SymName,"All",CSize,CRot,CColor,FALSE))
                    		goto NoSym;
                    	sprintf (SymStuff,"%s;%s;%s;%s",SymName,CSize,CRot,CColor);
                    	
                    break;  
					case 3: //lines 
						DecodeLineSym (SymStuff,SymName,CSize,CColor);
	                	if (!SelectLineSymbol (hWndDlg,1,SymName,CSize,CColor,FALSE))
                    		goto NoSym;  
                    	sprintf (SymStuff,"%s;%s;%s",SymName,CSize,CColor);
					break;
					case 5: //areas 
						DecodeAreaSym (SymStuff,SymName,CColor);
	                	if (!SelectAreaSymbol (hWndDlg,1,SymName,CColor,FALSE))
                    		goto NoSym;    
                    	sprintf (SymStuff,"%s;%s",SymName,CColor);
                    	
					break;
				}         
                SetDlgItemText (hWndDlg,IDC_SYMNAME,SymStuff);  
            NoSym:
                break;
                
            case IDC_LOCATE_DESTMAP: 
                 *str=0;
				 if (SendDlgItemMessage (hWndDlg,IDC_DIRECTORY,BM_GETCHECK,0,0))
				 {
                     if (!GetFileName3 (hWndDlg,str,IDS_FILTERINDEX,IDS_FILEINDEX)) break;
                 }   
				 else
				 {
	                 if (SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0)) 
	                 {
	                    if (!GetSaveName2 (hWndDlg,str,IDS_FILTERPLT,".PLT",IDS_FILEPLT)) break;   
	                 }
	                 else  
	                 {
	                    if (!GetFileName3 (hWndDlg,str,IDS_FILTERPLT,IDS_FILEPLT)) break;   
	                 } 
	             }
                 SetDlgItemText (hWndDlg,IDC_DESTMAP,str);
                 break;
                 
            case IDC_LOCATE_SOURCE: 
                    
                 _fstrcpy (SHPExt,".SHP"); 
                 _fstrcpy (ExtID,"Shape Files");
                 sprintf (gszFilter,"%s(*%s)|*%s|",ExtID,SHPExt,_fstrlwr(SHPExt));  
//                 SetOpenFlags(OFN_ALLOWMULTISELECT);
                 if (GetFileName3(hWndDlg,LoadName,0,IDS_FILESHP))
                 {   
                    SetDlgItemText (hWndDlg,IDC_FILE,LoadName);
					PostMessage(hWndDlg, WM_COMMAND, IDC_SET_SOURCE, 0L);
                 } 
                 break;
            
            case IDC_ASSIGNCVT: 
            {
            	 
                 GetDlgItemText (hWndDlg,IDC_FILE,Name,256);
                 if (!GuessShapeProjection (Name,hWndDlg,IDC_PROJECTION,IDC_UNITS,FALSE))
                 {                    
                    sprintf (mess,"Unable to open file %s",Name);
                    GSSiMsgBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK); 
                    break;
                 }  

            }
            	 break;
            	     
            case IDC_SET_SOURCE:
            {    
                BOOL    HaveMID=FALSE;
                short   nFields; 
                LPSTR   lpSpace;
                HFILE	FidSHP; 
                LPSTR	pDot;
				 
				GetDlgItemText (hWndDlg,IDC_FILE,LoadName,128);
				FidSHP=GSSiOpenFile (LoadName,&OFStruct,OF_READ);
				if (FidSHP == HFILE_ERROR)  
				{  
						                    
				    sprintf (mess,"Unable to open file %s",LoadName);
				    GSSiMsgBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
				    break;
				}
				BigRead (FidSHP,(HPSTR)&SHPHeader,(UINT)sizeof(SHPHeader));
				flip ((LPSTR)&SHPHeader.FileCode,4);
				flip ((LPSTR)&SHPHeader.FileLength,4); 
				GSSiClose (FidSHP); 
                OldCursor = GSSiSetCursor (LoadCursor (0,IDC_WAIT));
                
                _fstrcpy (str,LoadName);
                pDot = _fstrrchr (str,'.');
                if (pDot)
                	_fstrcpy (pDot,".SHP");    
                //_splitpath (OFStruct.szPathName,drive,dir,file,NULL);   
                //sprintf (str,"%s%s%s.SHP",drive,dir,file);
                    
                hSQL = 0;        
 
                st = OpenDataFile (str,"",BT_READ,&hSQL);
			 	if (!st)
                {   
                    sprintf (mess,"Unable to open DBF via ODBC: %s",str);
                    GSSiMsgBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
                    break;   
                }
    
                SetDlgItemText (hWndDlg,IDC_ATTRIBUTE_FILE,str);
                EnableWindow (GetDlgItem(hWndDlg,IDC_GET_SYM),TRUE);
             	EnableWindow (GetDlgItem(hWndDlg,IDC_SHOW_FIELDS),TRUE);
				switch (SHPHeader.ShapeType)
				{   
					case 8: //multipoints
					case 1: //points
	                	SetDlgItemText (hWndDlg,IDC_GET_SYM,"Point Symbol"); 
	               	break; 
					case 3: //lines
	                	SetDlgItemText (hWndDlg,IDC_GET_SYM,"Line Symbol");  
					break;
					case 5: //areas
	                	SetDlgItemText (hWndDlg,IDC_GET_SYM,"Area Symbol");  
					break;
					default:
	                    sprintf (mess,"Unrecognized shape file type: %ld",SHPHeader.ShapeType);
	                    GSSiMsgBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
                    break;   
				}         
            }  
            break;
                 
            case IDC_SAVE:
            {    
            	 short	Version=2; 
            	 HFILE	FidSave; 
            	 OFSTRUCT	OFStruct;
            	 
                 if (!GetSaveName2 (hWndDlg,Name,0,Ext,IDS_FILETL2)) break; 
                 FidSave = GSSiOpenFile (Name,&OFStruct,OF_CREATE);
                 BigWrite (FidSave,Ext,6,-1);
                 BigWrite (FidSave,(HPSTR)&Version,2,-1);   
                 GetDlgItemText (hWndDlg,IDC_FILE,LoadName,128);
                 BigWrite (FidSave,LoadName,128,-1);
                 GetDlgItemText (hWndDlg,IDC_DESTMAP,PltName,128);
                 BigWrite (FidSave,PltName,128,-1);
                 NewOpt = SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0);
                 BigWrite (FidSave,(HPSTR)&NewOpt,2,-1);
                 GetDlgItemText(hWndDlg,IDC_TAPREFIX,Prefix,sizeof(Prefix));
                 BigWrite (FidSave,(HPSTR)Prefix,sizeof(Prefix),-1);
                 GetDlgItemText(hWndDlg,IDC_UDI,UDI,128);               
                 BigWrite (FidSave,(HPSTR)UDI,128,-1);
                 GetDlgItemText(hWndDlg,IDC_SYMNAME,SymStuff,lnSymStuff);               
                 BigWrite (FidSave,(HPSTR)SymStuff,lnSymStuff,-1);
                 GetDlgItemText (hWndDlg,IDC_PROJECTION,curproject,lncurproject);
			     ii = GSSillseek (FidSave,0,1);
                 ii=BigWrite (FidSave,(HPSTR)curproject,34,-1);
			     UnitsOpt=SendDlgItemMessage(hWndDlg,IDC_UNITS,CB_GETCURSEL,NULL,NULL); 
			     ii = GSSillseek (FidSave,0,1);
                 BigWrite (FidSave,(HPSTR)&UnitsOpt,2,-1);
                 NewOpt  = SendDlgItemMessage (hWndDlg,IDC_CREATE_SYMS,BM_GETCHECK,0,0);
			     ii = GSSillseek (FidSave,0,1);
                 ii=BigWrite (FidSave,(HPSTR)&NewOpt ,2,-1);
				 HiPrecis = SendDlgItemMessage (hWndDlg,IDC_HIPRECIS,BM_GETCHECK,0,0);
				 BigWrite (FidSave,(HPSTR)&HiPrecis ,2,-1);
				 HiPrecis = SendDlgItemMessage (hWndDlg,IDC_UNIQUEREFNO,BM_GETCHECK,0,0);
				 BigWrite (FidSave,(HPSTR)&HiPrecis ,2,-1);
                 GetDlgItemText (hWndDlg,IDC_STARTNO,str,16);
                 BigWrite (FidSave,(HPSTR)str,16,-1);
                 _fmemset (str,0,256);
                 ii=BigWrite (FidSave,(HPSTR)str,236,-1); //spacer for future options
                 WriteAdvancedOpts (FidSave);
                 GSSiClose (FidSave);
            } 
                    
            	 break;
            
            case IDC_UNIQUEREFNO:
            {
            	 BOOL	On=SendDlgItemMessage (hWndDlg,IDC_UNIQUEREFNO,BM_GETCHECK,0,0);
            	 
            	 if (On)
            	 	GetGlobalCVal ("[%STARTREFNO]",str,"1000000");
            	 else
            	 	_fstrcpy (str,"1"); 
         		 SetDlgItemText (hWndDlg,IDC_STARTNO,str);
			}
				 break;
				 
            case IDC_EXIT: 
                if (hSQL)
                {
                    CloseDataFile (TRUE, &hSQL);  
					ODBCTerminate (FALSE);
                } 
				DestroyAdvancedOpts ();
                GSSiEndDialog(hWndDlg, TRUE,hSaveBM); 
                break;
           	case IDC_RECALL: 
           	{
           		 short Version;     
            	 HFILE	FidSave;
            	 OFSTRUCT	OFStruct;
           		 
           		 
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
                 BigRead (FidSave,LoadName,128);
                 SetDlgItemText (hWndDlg,IDC_FILE,LoadName);
                 BigRead (FidSave,PltName,128);
                 SetDlgItemText (hWndDlg,IDC_DESTMAP,PltName);
                 BigRead (FidSave,(HPSTR)&NewOpt,2);
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
                 BigRead (FidSave,(HPSTR)&UnitsOpt,2);
			     SendDlgItemMessage(hWndDlg,IDC_UNITS,CB_SETCURSEL,UnitsOpt,NULL); 
			     if (Version > 1)
			     {   
			     	 ii = GSSillseek (FidSave,0,1);
	                 ii=BigRead (FidSave,(HPSTR)&NewOpt,2);
	                 SendDlgItemMessage (hWndDlg,IDC_CREATE_SYMS,BM_SETCHECK,NewOpt,0);
	                 ii=BigRead (FidSave,(HPSTR)&HiPrecis,2);
	                 SendDlgItemMessage (hWndDlg,IDC_HIPRECIS,BM_SETCHECK,HiPrecis,0);
	                 ii=BigRead (FidSave,(HPSTR)&HiPrecis,2);
	                 SendDlgItemMessage (hWndDlg,IDC_UNIQUEREFNO,BM_SETCHECK,HiPrecis,0);
	                 BigRead (FidSave,str,16);
	                 SetDlgItemText(hWndDlg,IDC_STARTNO,str);               
			     	 ii = GSSillseek (FidSave,0,1);
	                 ii=BigRead (FidSave,str,236);
			     }
			     while (ReadObject (&FidSave, FALSE,NULL,NULL));
                 GSSiClose (FidSave);
				 PostMessage(hWndDlg, WM_COMMAND, IDC_SET_SOURCE, 0L);
                 FileIsOpen = TRUE;
               	 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
            }
           		 break;
           		 
				 
            case IDC_SHOW_FIELDS:
            	 DisplayFieldList (hWndDlg,hSQL,NULL,0);
                 break;
                      
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 if (Processing)
                    ContinueProcessing=FALSE;
                 break;
            
            case IDOK: 
            {
                 long   lineno=0, TotLen,NoFile=0, MidLine, NewRefno, BadRecs=0, TotBadRecs, LastLoc; 
                 short  iUDI=0, LineSym;
                 BOOL   Done, Create, BadCoord, FileIsDir; 
                 BOOL	SaveSTB = StoreTAGBounds; 
                 BOOL	UniqueRefno=SendDlgItemMessage (hWndDlg,IDC_UNIQUEREFNO,BM_GETCHECK,0,0);
                 long	StartRefno;
                 HANDLE hMIDstr, hBT, hSQLAttImport=0;     
                 LPSTR   pPrefix;
                 SHPRECHEADER SHPRecHeader;
                 double coordcvt=1;
                 long     ii=100, Dummy, nRecBytes, nBytes, Type,MaxPointsPerLine=0;
                 short    NumSyms=0, SymNum, cond, UnknownSym; 
                 HANDLE	  hSymDesc=0;   
                 static	long	debugline=263;
                 HFILE	FidFL;  
			     HFILE  FidSHP;
                 char	LeafName[32];  
                 long	CurLoc; 
                 BOOL	OpenedSeg=FALSE;
                 
                 SetViewport (*pCommandViewport);
             	 if (hImportPreSet)
             	 {
             	 	LPSETREFNO pSetRef = (LPSETREFNO)GlobalLock (hImportPreSet);
		                 	 	
             	 	ProcessText (pSetRef->SetRefno);
             	 	GlobalUnlock (hImportPreSet);
             	 	if (!ContinueProcessing)
             	 	{   	
             	 	    ContinueProcessing = TRUE;
	                    GSSiMsgBox(GetFocus(),"Pre-processing Failed", 0,MB_ICONEXCLAMATION|MB_OK);
             	 		break;
             	 	}
             	 }
                 
                 MaxPointsPerLine = GetGlobalLVal2 ("[%MAXPOINTSPERPOLY]",0);
                 if (!GetDlgItemText (hWndDlg,IDC_DESTMAP,PltName,128)) 
                 {
                    GSSiMsgBox(GetFocus(),"No destination map", 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }
                 if (!GetDlgItemText (hWndDlg,IDC_SYMNAME,SymStuff,lnSymStuff))
                 {
                    GSSiMsgBox(GetFocus(),"No symbol selected", 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 } 
				 HiPrecis = SendDlgItemMessage (hWndDlg,IDC_HIPRECIS,BM_GETCHECK,0,0);
				 Create = SendDlgItemMessage (hWndDlg,IDC_CREATE_SYMS,BM_GETCHECK,0,0);
                 *curproject = 0;
                 if (!GetDlgItemText (hWndDlg,IDC_PROJECTION,curproject,lncurproject))
                 {
                    GSSiMsgBox(GetFocus(),"No input projection set", 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }   
                 if ((lpDot=_fstrrchr(curproject,'.')))
                        *lpDot = 0;  
                 
                 if (hStreetSegFields)
                 	OpenStreetSegmentTable (TRUE,&OpenedSeg); 
/* 
                 {      
                 	SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Creating intersection file");
                    CreateIntersectionFile (TRUE,GetDlgItem(hWndDlg,IDC_STATUS));
                 }*/    
                 GetGlobalCVal ("[%ALT_PROJECTION]",SaveAltProj,NULL);
                 SetGlobalValue("%ALT_PROJECTION",curproject);
				 ConvertCoordClose ();
				 ConvertCoordInit(); 
			 	 if (GetDlgItemText (hWndDlg,IDC_STARTNO,str,128))
			 	 {
			 		ExpandText (str);
			 		StartRefno = atol (str);
			 	 }
			 	 else
			 		StartRefno = 1;
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
                    GSSiMsgBox(GetFocus(),"Units field not set", 0,MB_ICONQUESTION|MB_OK);
                    break;
                 }
                 
                 GetDlgItemText (hWndDlg,IDC_FILE,Name,256); 
                 GetDlgItemText (hWndDlg,IDC_ATTRIBUTE_FILE,str,256); 
                 _fstrupr (Name);
				 if (_fstrstr (Name,"FILELIST.TXT"))
				 {
				 	 FidFL = GSSiOpenFile (Name,NULL,OF_READ);
				 	 if (FidFL == HFILE_ERROR)
	                 {  
	                    
	                    sprintf (mess,"Unable to open file %s",Name);
	                    GSSiMsgBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK); 
	                    break;
	                 }  
				 }
				 else
				 	FidFL = HFILE_ERROR;
NextFile:        
				 if (FidFL != HFILE_ERROR)
				 {  
				 	LPSTR	pDot;
				 	
				 	if (!fgetstring (Name,128,FidFL)) 
				 	{
				 		break; 
				 	}
				 	_fstrcpy (str,Name);
				 	pDot = _fstrrchr (str,'.');
				 	_fstrcpy (pDot,".DBF");
				 }
				 if (!OpenSHPFile (Name))
//                 FidSHP=GSSiOpenFile (Name,&OFStruct,OF_READ);
//                 if (FidSHP == HFILE_ERROR)  
                 {  
                    sprintf (mess,"Unable to open file %s",Name);
                    GSSiMsgBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK); 
                    break;
                 }
                FidSHP = SHPFid; 
                GSSillseek (FidSHP,0,0);
                CloseDataFile (TRUE, &hSQL);  
                
                _splitpath (OFStruct.szPathName,NULL,NULL,LeafName,NULL); 
				SetGlobalValue("%SOURCENAME",LeafName);
                _fstrcpy (Name,"SHP=");
                _fstrcat (Name,str);    
                st = OpenDataFile (Name,"",BT_READ,&hSQL);
                if (!st)
                {  
                    GSSiMsgBox(GetFocus(),"Cannot open data file", 0,MB_ICONQUESTION|MB_OK);
                    break;
                }
                SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
                FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
                st = OpenImportAttributesFile (&hSQLAttImport);
                if (!st)
                {  
                    GSSiMsgBox(GetFocus(),"Cannot open attribute import file", 0,MB_ICONQUESTION|MB_OK);
                    break;
                }

				DecodePointSym (SymStuff,SymName,CSize,CRot,CColor);
                EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE);
                EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),FALSE);
                 SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Loading Data");
                 TotLen = NumSHPRecs;  
                 ExpandText (PltName);
                 PltType = 2;
                 Done = FALSE;
                 MidLine=0; 
                 str[0]=0;  
                 BigRead (FidSHP,(HPSTR)&SHPHeader,(UINT)sizeof(SHPHeader));
                 flip ((LPSTR)&SHPHeader.FileCode,4);
                 flip ((LPSTR)&SHPHeader.FileLength,4);  
                 
                 if (GetGlobalBVal2 ("[%MERGETAGS]",FALSE))
                 	StoreTAGBounds = TRUE;
                 if (SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0)) 
                 {
                     short NumPens=10;
                     PENDESC PenDesc[10];
                     LPSYMBOL   pSym;
                     short    i;  
                    
                     if (hImportLimits) 
                     {   
                    	LPSHORT	pint;
                    	LPMNMXCORD	pMinMax;
                    	
                    	pint = (LPSHORT)GlobalLock (hImportLimits);
                    	pint+=2;
                    	pMinMax = (LPMNMXCORD)pint;
                    	MinMaxCoord = *pMinMax;
                    	GlobalUnlock (hImportLimits);  
                     }
                     else
                     {
	                     DBoundsInit (&MinMaxCoord);
	                     Points[0].x = SHPHeader.Xmin;   
	                     Points[0].y = SHPHeader.Ymin;   
	                     Points[1].x = SHPHeader.Xmin;   
	                     Points[1].y = SHPHeader.Ymax;   
	                     Points[2].x = SHPHeader.Xmax;   
	                     Points[2].y = SHPHeader.Ymax;   
	                     Points[3].x = SHPHeader.Xmax;   
	                     Points[3].y = SHPHeader.Ymin; 
	                     for (i=0;i<4;i++)
	                     {
	                        if (ConvertCoord(&Points[i],3,1))
	                        {   
	                            GSSiMsgBox(GetFocus(),"Unable to convert coordinates as specified", 0,MB_ICONQUESTION|MB_OK);
	                            goto ErrorEnd;
	                        }
                            AddDPointToMinMax (&Points[i],&MinMaxCoord); 
	                     }
	                 }   
                     for (i=0;i<NumPens;i++)
                     {
                        PenDesc[i].PenNum = i+1;
                        PenDesc[i].Width = (float)1; 
                        PenDesc[i].Style = 1;
                        PenDesc[i].Color = RGB(0,0,0);
                     }
					 GetDlgItemText (hWndDlg,IDC_DESTMAP,PltName,128);
					 ExpandText (PltName);
                     if (!CreateNewMap (PltName,&MinMaxCoord,NumSyms,hSymDesc,
                                                        NumPens,(LPPENDESC)&PenDesc,0,0,TRUE)) goto ErrorEnd;
                 }
                 if ((FileIsDir=SendDlgItemMessage (hWndDlg,IDC_DIRECTORY,BM_GETCHECK,0,0)))
                 {
                 	 DBoundsInit (&EditBounds);
                 }
                 else
                 {
					 OpenMap (CurView->hWnd,CurView->hDC);
					 EditBounds = CurView->FileMNMX; 
					 CloseMap (FALSE);
				 }
                    
                 SaveFPT = FileProjectionType; 
				 FileProjectionType=0;
                 DisableHalt = TRUE;  
                 ContinueProcessing = TRUE;   
                 Processing = TRUE;    
                                        
                 NewRefno=0;  
                 nRecBytes = 0; 
                 BadRecs = 0;
        NextLine: 
                 if (!ContinueProcessing) Done = TRUE;
                 if (Done)
                    goto EndFile; 
               
            	 if (nRecBytes)
            		GSSillseek (FidSHP,nRecBytes,1);
            	 CurrentSHPRec = lineno;
                 lineno++;  
                 CurLoc = lineno;
                 ODBCRecNum = lineno;
                 if (lineno == debugline)
                 	ii=1;  
                 CurView->PassID = 0;
                 SHPRecOffset = GetSHPRecordOffset (CurrentSHPRec,FALSE);
    			 if (SHPRecOffset < 0)
		   		 	goto NextLine;
            	 GSSillseek (FidSHP,SHPRecOffset,0);
                 if (BigRead (FidSHP,(HPSTR)&SHPRecHeader,sizeof(SHPRecHeader))!=sizeof(SHPRecHeader))
                	goto EndFile;
                 CurrentSHPRec--;    
                 if (!FetchDBRec (hSQL))
                 {
                    GSSiMsgBox(GetFocus(),"Attribute database ends before shape file", 0,MB_ICONQUESTION|MB_OK);
                    goto ErrorEnd;
                 }         
                 flip ((LPSTR)&SHPRecHeader.RecNum,4);
                 flip ((LPSTR)&SHPRecHeader.Length,4);
                 stuff[0]=0;
                 if (hGRCommand)
                 {   
                 	 short lcmdstring;
                 	 LPGRCOMMAND	lpGRCommand=(LPGRCOMMAND)GlobalLock (hGRCommand);
		                     	 
                 	 lcmdstring = lpGRCommand->lcmdstring;
                 	 lcmdstring += lcmdstring%2; 
                 	 _fstrcpy (CmdString,lpGRCommand->CmdString);
                     ExpandText (CmdString);
                     stuff[1]=40;
                     stuff[2]=lcmdstring;
                     _fstrncpy ((LPSTR)&stuff[3],CmdString,lcmdstring);
                     stuff[0]+=2+2+lcmdstring; 
                     GlobalUnlock (hGRCommand);
                 } 
				 GetDlgItemText (hWndDlg,IDC_SYMNAME,SymStuff,lnSymStuff);
				 DecodePointSym (SymStuff,SymName,CSize,CRot,CColor);
				 ExpandText (SymName);
				 Truncate (SymName); 
                 
                 if (BigRead (FidSHP,(HPSTR)&Type,4) != 4) //some files have null records (type 0)
                    	goto EndFile;
                 if (!Type)
                 { 
                 	nRecBytes = SHPRecHeader.Length*2 - 4;
                 	goto NextLine;
                 }
                 else
                 	GSSillseek (FidSHP,-4,1);
                 pPrefix = Prefix;    
                 GetDlgItemText(hWndDlg,IDC_TAPREFIX,Prefix,sizeof(Prefix));
                 if (!*pPrefix) 
                	pPrefix = 0;
                 else
                 {
                 	_fstrcpy (str,pPrefix);
                 	ExpandText (str);
                 	_fstrncpy (pPrefix,str,8);
                 	pPrefix[8] = 0;
                 }
                 if (SHPHeader.ShapeType == 3 || SHPHeader.ShapeType == 5)
                 {  
                    short     nPoly, nPoints,lastnpoints,Type,j;
                    long    nVertex;
                    HANDLE  hhPoly,hNumPoints; 
                    LPHANDLE	phPoly;
                    HPDPOINT    lpDPoint, lpDPoints;
                    DPOINT  LinkPoint, FirstPoint,LastPoint;
                    LPSTR   lpSpace;
                    BOOL    Store=TRUE, FirstPoly, FirstP;
                    SHPPOLYHEADER   SHPPolyHeader;  
                    HPDPOINT    pPoints;
                    HPLONG      pPartIndex;
                    HANDLE      hPoints, hPartIndex, hnPoints;
                    HPWORD      pNumPoints;
                    long		LastIndex; 
                    short		SymType;    
                    long		NumPoints;                                        
                    MNMXCORD	Bounds;
					REFINDEXDATA	RefIdxData;

					DBoundsInit (&Bounds);
                    
                 	nRecBytes = SHPRecHeader.Length*2;
/*	                 {
	                 	short	ii;
	                 	char	ln[32]="[LAKE_NAME]";
	                 	
	                 	ExpandText (ln);
	                 	if (_fstrnicmp (ln,"VERMIL",6))
	                 		goto NextLine;
	                 }  */
                    if (BigRead (FidSHP,(HPSTR)&SHPPolyHeader,sizeof(SHPPolyHeader)) != sizeof(SHPPolyHeader))
                    	goto EndFile; 
                    nRecBytes -=  sizeof(SHPPolyHeader);
                    if (SHPPolyHeader.NumPoints > SHRT_MAX)
                    	ii=1;
                    if (SHPPolyHeader.NumPoints > UINT_MAX)
                    	ii=1;
 /*                   {   
						GSSillseek (FidSHP,nRecBytes,1);
						nRecBytes = 0;
						BadRecs++;
                    	goto NextLine;
                    }  */
                    hPartIndex = GSSiGlobAlloc ( 417,GMEM_MOVEABLE,sizeof(long)*(SHPPolyHeader.NumParts+1));
                    nPoly = SHPPolyHeader.NumParts;  
                    NumPoints = SHPPolyHeader.NumPoints;  
                    if (!NumPoints)
                    	goto NextLine;
					if (SHPHeader.ShapeType == 5) 
					{
						SymType = 3;
						Type = 0;
					} 
					else 
					{
						SymType = 2;
						Type = 1;   
					}
					hhPoly = GSSiGlobAlloc ( 418,GMEM_MOVEABLE,sizeof(HANDLE)*SHPPolyHeader.NumParts); 
					phPoly = (LPHANDLE)GlobalLock (hhPoly);
					hNumPoints = GSSiGlobAlloc ( 419,GMEM_MOVEABLE,sizeof(short)*SHPPolyHeader.NumParts); 
					pNumPoints = (HPWORD)GlobalLock (hNumPoints);
                    pPartIndex = (HPLONG)GlobalLock (hPartIndex); 
                    nBytes = sizeof(long)*SHPPolyHeader.NumParts;
                    if (BigRead (FidSHP,(HPSTR)pPartIndex,(UINT)nBytes) != nBytes)
                    	goto EndFile;
                    nRecBytes -= nBytes; 
                    pPartIndex+=SHPPolyHeader.NumParts;
                    *pPartIndex = SHPPolyHeader.NumPoints;
                    GlobalUnlock (hPartIndex);  
                    pPartIndex = (HPLONG)GlobalLock (hPartIndex);
                    
                    for (i=0;i<SHPPolyHeader.NumParts;i++)
                    {   
                        long    numpoints, startpoint,ii;
                        
                        startpoint = *pPartIndex++;
                        numpoints = *pPartIndex-startpoint; 
                        *pNumPoints++ = numpoints;  
                        if (numpoints > USHRT_MAX)
                        	Store = FALSE;
                   		*phPoly = GSSiGlobAlloc ( 420,GMEM_MOVEABLE,sizeof(DPOINT)*(long)numpoints);
	                    pPoints = (HPDPOINT)GlobalLock (*phPoly); 
                        nBytes = sizeof(DPOINT)*numpoints;
                        if ((ii=BigRead (FidSHP,(HPSTR)pPoints,nBytes)) != nBytes)
                        	goto EndFile;   
                        GlobalUnlock (*phPoly++);
                        nRecBytes -= nBytes;
                    } 
					GlobalUnlock (hNumPoints);
                    GlobalUnlock (hhPoly);
                    GSSiGlobUlFree (&hPartIndex);  
                    if (pPrefix)
                    {
						GetDlgItemText(hWndDlg,IDC_UDI,UDI,128); 
						ExpandText (UDI); 
					}             
                    else
                    	*UDI=0; 
                    Store = ProcessImportFilter ();
                    if (Store)
                    	SymNum = GetOrCreateSym (SymName,&NumSyms,&hSymDesc,Create,SymType);
                    else
                    	SymNum = 0;
                    if (SymNum)
                    {
	            //      if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) More=FALSE; 
						phPoly = (LPHANDLE)GlobalLock (hhPoly);
						pNumPoints = (LPWORD)GlobalLock (hNumPoints);
						BadCoord = FALSE;
						if (!_fstricmp (UDI,"118-24-25"))
							ii=1; 
						FirstP = TRUE;
	                    for (j=0;j<nPoly;j++,phPoly++,pNumPoints++)
	                    {
		                    pPoints = (HPDPOINT)GlobalLock (*phPoly); 
		                    if (FirstP)
		                    {
		                    	FirstP = FALSE;
		                    	FirstPoint = *pPoints;
		                    }
		                    for (i=0;i<*pNumPoints;i++,pPoints++)
		                    {   
		                    	LastPoint = *pPoints;
		                        if (ConvertCoord(pPoints,3,1))
		                        {   
		                            //GSSiMsgBox(GetFocus(),"Unable to convert coordinates as specified", 0,MB_ICONQUESTION|MB_OK);
		                            //goto ErrorEnd;
		                            BadCoord = TRUE;
		                            break;
		                        }
								AddDPointToMinMax (pPoints,&Bounds);
		                    }
		                    GlobalUnlock (*phPoly);  
		                }
		                GlobalUnlock (hhPoly);
		                GlobalUnlock (hNumPoints);
		                if (BadCoord)
	                    {
	                    	Store=FALSE;
	                    	BadRecs++;  
	                    }
	                    if (GetGlobalBVal2 ("[%MERGETAGS]",FALSE) && Store && pPrefix)
	                    {
	                    	if (*pPrefix && *UDI)
	                    	{
							    ForceTAGIndex = TRUE; 
								if (OpenMap (CurView->hWnd,NULL))
								{   
									MNMXCORD	MinMaxNew,MinMaxOld={SHPPolyHeader.Xmin,SHPPolyHeader.Ymin,SHPPolyHeader.Xmax,SHPPolyHeader.Ymax};
									short	pos=BT_FIRST,cond=BT_GE, getnext=1;  
									
									ConvertRectCoord (&MinMaxNew, &MinMaxOld,3,1)									;
									ExpandBounds (&MinMaxNew,GetGlobalDVal2 ("[%MERGETAGDIST]",1000));
								    ForceTAGIndex = FALSE;
						    		_fstrncpy(TAGKey.PREFIX,pPrefix,8);
						    		_fstrncpy (TAGKey.UDI,UDI,sizeof(TAGKey.UDI));
						    		TAGKey.Refno = LONG_MIN;
									while (getnext && !BT_FIND (hTAGIdx,(LPSTR)&TAGKey,pos,cond,(LPSTR)&RefIdxData))
									{    
										LPTHEME pTheme;
										MNMXCORD	MinMaxOld;
										
										pos=BT_NEXT;
										cond=BT_ANY;
										MinMaxToBase ((LPMINMAX)&RefIdxData.MinMax,&MinMaxOld);

										if (!_fstrnicmp (TAGKey.PREFIX,pPrefix,8) &&  
											!_fstrnicmp (TAGKey.UDI,UDI,sizeof(TAGKey.UDI)) && 
											BoundsInBounds (&MinMaxOld,&MinMaxNew,1))
										{   
											getnext = 0;
										    PickList[0].ViewID = 0;
											PickList[0].FileNum = -1;  
											PickList[0].SubFile = 0;
											PickList[0].FileInIndex = RefIdxData.FileInIndex;
											PickList[0].Segment = RefIdxData.Segment;
											PickList[0].Offset = RefIdxData.Offset; 
											PickList[0].Element = 0;
						                    pTheme = AddTheme (GF_SAVEPOLYPARTS_THEME);
						                    CurView->PassID = 4; 
						                    ProcessSelectedTheme = CurView->NumThemes;
	   										BT_DELETE (hTAGIdx,(LPSTR)&TAGKey,(LPSTR)&RefIdxData,FALSE);
						                    ProcessPickedItem (0,-2); 
											ProcessSelectedTheme = 0;
						                    WantElement = LONG_MAX;               
						                    DeleteTheme (pTheme);
						                    OpenMap (CurView->hWnd,NULL); 
						                    DeletePickedItem (0,12,92);  
						                    CloseMap(FALSE);
		                    				while (GetSavedPolys ())
		                    				{
							                    if (hSavePoly)
							                    {   LPMNMXCORD lpRect;
							                        short   nPnts,i,nareas; 
							                        long	Totp;   
							                        BOOL	First, FirstLoop;
							                        HPDPOINT    lpDpoint;
													HPWORD	pPolyParts;
													WORD	nPParts; 
													DPOINT	FirstPoint;
							
							                        if (hSavePolyParts)
							                        {
							                        	pPolyParts = (HPWORD)GlobalLock (hSavePolyParts);
							                        	nareas = *pPolyParts++; 
							                        }
							                        else
							                        { 
							                            nareas=1; 
							                            nPParts = nSavePoly; 
							                            pPolyParts = &nPParts;
							                        }
							                            
							                        lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
							                        lpRect++;
							                        lpDpoint = (HPDPOINT) lpRect;  
							                        FirstLoop = TRUE;
													hhPoly = GlobalReAlloc (hhPoly,sizeof(HANDLE)*(nPoly+nareas),GMEM_MOVEABLE); 
													phPoly = (LPHANDLE)GlobalLock (hhPoly);   
													phPoly += nPoly;
													hNumPoints = GlobalReAlloc (hNumPoints,sizeof(short)*(nPoly+nareas),GMEM_MOVEABLE); 
													pNumPoints = (HPWORD)GlobalLock (hNumPoints); 
													pNumPoints += nPoly;
							                        First = TRUE;
							                        while (nareas--)
							                        {   
							                        	DPOINT	FirstPoint, LastPoint;   
							                        	HPDPOINT	pNewPoint;
							                        	
							                        	*pNumPoints = *pPolyParts;
							                        	*phPoly = GSSiGlobAlloc ( 421,GMEM_MOVEABLE,(long)*pPolyParts*sizeof(DPOINT));
							                        	pNewPoint = (HPDPOINT)GlobalLock (*phPoly);
								                        while ((*pPolyParts)--)
								                            *pNewPoint++ = *lpDpoint++;
								                        GlobalUnlock (*phPoly++);
								                        nPoly++;     
								                        pPolyParts++; 
								                        pNumPoints++; 
								                        if (!First)
								                        	lpDpoint++;
								                        First = FALSE;
								                    }
								                    GlobalUnlock (hhPoly);
								                    GlobalUnlock (hNumPoints); 
							                       	GSSiGlobUlFree (&hSavePolyParts);
									                GSSiGlobUlFree (&hSavePoly);
							                    }
							                }
										}
									}
								} 
	                    	}
	                    }
	                    LastIndex = 0;   
	                    FirstPoly=TRUE;
	                    if (Store) 
	                    	Store = BoundsInBounds (&Bounds,&EditBounds,0);
	                    if (!Store && FileIsDir)
	                    {
	                    	if ((Store = GetIndexedEditFile (PltName,&Bounds)))
	                    	{
								CloseMap (TRUE);  
								OpenMap (CurView->hWnd,CurView->hDC);
								EditBounds = CurView->FileMNMX; 
								CloseMap (FALSE);  
	                    	}
	                    	else
	                    	{
	                    		NoFile++;
			                 	DBoundsInit (&EditBounds);
				                sprintf (mess,"%ld records skipped",NoFile);
			                 	SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,mess);
			                }
	                    }
	                	phPoly = (LPHANDLE)GlobalLock (hhPoly);
						pNumPoints = (LPWORD)GlobalLock (hNumPoints);
	                    if (Store) 
	                    {   
	                    	if (MaxPointsPerLine && Type)
	                    	{   
	                    		USHORT	np,rempoints;
	                    		UINT	ipoly,ip;  
	                    		HANDLE	hPoly=GSSiGlobAlloc ( 422,GMEM_MOVEABLE,sizeof(DPOINT)*MaxPointsPerLine);
	                    		HPDPOINT	pPoly, pPoly2; 
	                    		
	                    		for (ipoly=0;ipoly<nPoly;ipoly++,pNumPoints++,phPoly++)
	                    		{   
	                    			pPoly2 = (HPDPOINT)GlobalLock (*phPoly);
	                    			rempoints = *pNumPoints-1; 
	                    			while (rempoints > 0)
	                    			{
	                    				np = min (rempoints+1,MaxPointsPerLine);
	                    				rempoints -= (MaxPointsPerLine-1);
	                    				pPoly = (HPDPOINT)GlobalLock (hPoly);
	                    				for (ip=0;ip<np;ip++)
	                    					*pPoly++ = *pPoly2++; 
	                    				pPoly2--;
	                    				GlobalUnlock (hPoly); 
										NewRefno = GetNextRefno (&StartRefno,UniqueRefno,TRUE); 
				                        if (!AddPolyToMap (1,&np, &hPoly,Type,NewRefno,NULL,-1,SymNum,stuff,Prefix,UDI,
				                                        -1,-1,0,0,0,0,0,HiPrecis))
				                            RemoveRefno (NewRefno,&StartRefno,UniqueRefno); 
				                    }
				                    GlobalUnlock (*phPoly);
			                    } 
			                    GlobalFree (hPoly);
	                    	}
	                    	else
	                    	{
			                    if (hStreetSegFields)
			                    	NewRefno = LoadStreetSegData (stuff,FirstPoint,LastPoint,&StartRefno,UniqueRefno); 
			                 	else if (hImportRefno)
			                 	{
			                 	 	LPSETREFNO pSetRef = (LPSETREFNO)GlobalLock (hImportRefno);
			                 	 	HANDLE	hStr = GSSiGlobAlloc ( 423,GMEM_MOVEABLE,512);
			                 	 	LPSTR	pStr = GlobalLock (hStr);
			                 	 	
			                 	 	_fstrcpy (pStr,pSetRef->SetRefno);
			                 	 	ExpandText (pStr);
			                 	 	NewRefno = atol (pStr);
			                 	 	GSSiGlobUlFree (&hStr);
			                 	 	GlobalUnlock (hImportRefno);
			                 	}
			                    else
									NewRefno = GetNextRefno (&StartRefno,UniqueRefno,TRUE); 
				                ImportAttributes (hSQLAttImport);    
		                        if (!AddPolyToMap (nPoly,pNumPoints, phPoly,Type,NewRefno,NULL,-1,SymNum,stuff,Prefix,UDI,
		                                        -1,-1,0,0,0,0,0,HiPrecis))
		                            RemoveRefno (NewRefno,&StartRefno,UniqueRefno); 
		                                        
		                    }
	                    }
	                    else
	                    	ii=1;
                    	GlobalUnlock (hNumPoints);  
	                    GlobalUnlock (hhPoly); 
                    }
                    else
                    	ii=1; 
                    GlobalFree (hNumPoints);
                    phPoly = (LPHANDLE)GlobalLock (hhPoly);
                    while (nPoly--)
                    	GlobalFree (*phPoly++);  
                    GSSiGlobUlFree (&hhPoly);
                    
                 } 
                 else if (SHPHeader.ShapeType == 1)
                 {  
                    DPOINT  Point;
                    BOOL    Store=TRUE;
                    COLORREF    Color;    
                    SHPPOINTREC	SHPPointRec;
                    double	size=0,rot=0;                                          
                    
                    nRecBytes = 0;
                    if (BigRead (FidSHP,(HPSTR)&SHPPointRec,sizeof(SHPPointRec)) != sizeof(SHPPointRec))
                    	goto EndFile; 
                    if (pPrefix)
                    {
						GetDlgItemText(hWndDlg,IDC_UDI,UDI,128); 
						ExpandText (UDI); 
					}             
                    else
                    	*UDI=0; 
                    Store = ProcessImportFilter ();
                    if (Store)
                    	SymNum = GetOrCreateSym (SymName,&NumSyms,&hSymDesc,Create, 1);
                    else
                    	SymNum = 0;
                    if (SymNum)
                    {
	                    Store=TRUE; 
	                    if (ConvertCoord(&SHPPointRec.Point,3,1))
	                    {   
	                        GSSiMsgBox(GetFocus(),"Unable to convert coordinates as specified", 0,MB_ICONQUESTION|MB_OK);
	                        goto ErrorEnd;
	                    }
	                    if (SHPPointRec.Point.x < CurView->FileMNMX.xmn ||   
	                        SHPPointRec.Point.x > CurView->FileMNMX.xmx ||
	                        SHPPointRec.Point.y < CurView->FileMNMX.ymn ||
	                        SHPPointRec.Point.y > CurView->FileMNMX.ymx)
	                        Store=FALSE;    
	                    if (Store)
	                    {     
			                 if (!SetPointSize (&size,CSize))
			                 	goto EndFile;
			                 if (!SetPointRot (&rot,CRot))
			                 	goto EndFile;
			                 if (!SetPointColor (&Color,CColor))
			                 	goto EndFile;
		                 	 if (hImportRefno)
		                 	 {
		                 	 	LPSETREFNO pSetRef = (LPSETREFNO)GlobalLock (hImportRefno);
		                 	 	HANDLE	hStr = GSSiGlobAlloc ( 424,GMEM_MOVEABLE,512);
		                 	 	LPSTR	pStr = GlobalLock (hStr);
			                 	 	
		                 	 	_fstrcpy (pStr,pSetRef->SetRefno);
		                 	 	ExpandText (pStr);
		                 	 	NewRefno = atol (pStr);
		                 	 	GSSiGlobUlFree (&hStr);
		                 	 	GlobalUnlock (hImportRefno);
		                 	 }
							 else
							 	NewRefno = GetNextRefno (&StartRefno,UniqueRefno,TRUE); 
							 SetIntRefno (NewRefno);  
	                    	 ImportAttributes (hSQLAttImport);    
		                     if (!AddPointToMap (SHPPointRec.Point,NewRefno,hTimeStamp,SymNum,size,rot,stuff,hGRText,0,Prefix,UDI,Color,Color,-1,TRUE,HiPrecis,NULL,NULL))
		                     {
	                            RemoveRefno (NewRefno,&StartRefno,UniqueRefno); 
		                     	goto EndFile;
		                     } 
		                }
		            } 
                 }
                 else if (SHPHeader.ShapeType == 8)
                 {  
                    DPOINT  Point;
                    BOOL    Store=TRUE;
                    COLORREF    Color;    
                    SHPMULTIPOINTHEADER	SHPMultiPointHeader;
                    double	size=0,rot=0;
                    HANDLE	hPoints;
                    HPDPOINT	pPoints;                                          
                    
                    nRecBytes = 0;
                    if (BigRead (FidSHP,(HPSTR)&SHPMultiPointHeader,sizeof(SHPMultiPointHeader)) != sizeof(SHPMultiPointHeader))
                    	goto EndFile;   
                    hPoints = GSSiGlobAlloc ( 425,GMEM_MOVEABLE,(long)sizeof(DPOINT)*SHPMultiPointHeader.NumPoints);
                    pPoints = (HPDPOINT)GlobalLock (hPoints);
                    if (_hread (FidSHP,pPoints,sizeof(DPOINT)*SHPMultiPointHeader.NumPoints) != sizeof(DPOINT)*SHPMultiPointHeader.NumPoints)
                    	goto EndFile;
                    if (pPrefix)
                    {
						GetDlgItemText(hWndDlg,IDC_UDI,UDI,128); 
						ExpandText (UDI); 
					}             
                    else
                    	*UDI=0;
                    Store = ProcessImportFilter ();
                    if (Store)
                    	SymNum = GetOrCreateSym (SymName,&NumSyms,&hSymDesc,Create, 1);
                    else
                    	SymNum = 0;
                    if (SymNum)
                    {   
                    	while (SHPMultiPointHeader.NumPoints--)
                    	{
		                    Store=TRUE; 
		                    if (ConvertCoord(pPoints,3,1))
		                    {   
		                        GSSiMsgBox(GetFocus(),"Unable to convert coordinates as specified", 0,MB_ICONQUESTION|MB_OK);
		                        goto ErrorEnd;
		                    }
		                    if (pPoints->x < CurView->FileMNMX.xmn ||   
		                        pPoints->x > CurView->FileMNMX.xmx ||
		                        pPoints->y < CurView->FileMNMX.ymn ||
		                        pPoints->y > CurView->FileMNMX.ymx)
		                        Store=FALSE;    
		                    if (Store)
		                    {     
				                 if (!SetPointSize (&size,CSize))
				                 	goto EndFile;
				                 if (!SetPointRot (&rot,CRot))
				                 	goto EndFile;
				                 if (!SetPointColor (&Color,CColor))
				                 	goto EndFile; 
			                 	 if (hImportRefno)
			                 	 {
			                 	 	LPSETREFNO pSetRef = (LPSETREFNO)GlobalLock (hImportRefno);
			                 	 	HANDLE	hStr = GSSiGlobAlloc ( 426,GMEM_MOVEABLE,512);
			                 	 	LPSTR	pStr = GlobalLock (hStr);
			                 	 	
			                 	 	_fstrcpy (pStr,pSetRef->SetRefno);
			                 	 	ExpandText (pStr);
			                 	 	NewRefno = atol (pStr);
			                 	 	GSSiGlobUlFree (&hStr);
			                 	 	GlobalUnlock (hImportRefno);
			                 	 }
								 else
								 	NewRefno = GetNextRefno (&StartRefno,UniqueRefno,TRUE); 
								 SetIntRefno (NewRefno); 
		                    	 ImportAttributes (hSQLAttImport);    
			                     if (!AddPointToMap (*pPoints,NewRefno,hTimeStamp,SymNum,size,rot,stuff,hGRText,0,Prefix,UDI,Color,Color,-1,TRUE,HiPrecis,NULL,NULL))
			                     {
		                            RemoveRefno (NewRefno,&StartRefno,UniqueRefno); 
			                     	goto EndFile;
			                     } 
			                }                     
			                pPoints++;
			            }
		            } 
                 }
                 else   
                    goto ErrorEnd;
                 
                 LastLoc = CurLoc;     
                 PctBox (GetDlgItem(hWndDlg,IDC_STATUS), TotLen, CurLoc,0);
                 if (CurLoc >= TotLen)
                    Done=TRUE;
                 goto NextLine;
                 
        ErrorEnd: 
                 {  
                    
                    sprintf (mess,"Error in file at line %ld\r\n%s",lineno,str);
                    GSSiMsgBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
                 }
                    
        EndFile: 
		        {
		         	DPOINT Dpoint;
					AddPointToMap (Dpoint,0,0,0,0,0,0,0,0,0,0,0,0,0,TRUE,FALSE,NULL,NULL);
				}
				 if (ContinueProcessing) 
				 {
	                PctBox (GetDlgItem(hWndDlg,IDC_STATUS), TotLen, TotLen,0);
                 	_fstrcpy (mess,"Load Complete");
                 }
                 else
                 	_fstrcpy (mess,"Load Cancelled");
                 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),TRUE);
				 CloseMap(TRUE); 
				 CloseGSStreetNames();
                 DisableHalt = FALSE;
         	 	 Processing = FALSE;
             	 ContinueProcessing = TRUE; 
				 CloseSHPFile ();
   				 //GSSiClose (FidSHP); 
   				 if (FileIsDir)
   				 { 
	                SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Inserting symbols");
			 	 	AddSymToDir (PltName,NumSyms,hSymDesc,0,NULL);
			 	 }  
			 	 else
			 	 	AddSymToMap (NumSyms,hSymDesc,0,NULL); 
                 DestroySymList (&NumSyms,&hSymDesc);
                 CloseRefIndex(TRUE);
                 StoreTAGBounds = SaveSTB;           
                 if (hSQL)
                 {
					GlobalUnlock (SQLPtr->OFHandle);                  
					GlobalUnlock (hSQL);
                    CloseDataFile (TRUE, &hSQL);  
                 }
                 CloseImportAttributesFile (&hSQLAttImport);
               	 CloseStreetSegmentTable (OpenedSeg);  
               	 CloseStreetNameTable();
             	 ForceRefIndex = ForceTAGIndex = FALSE; 
             	 TotBadRecs = GetGlobalLVal ("[%BADRECS]");
             	 TotBadRecs += BadRecs;
             	 SetGlobalValueLong ("%BADRECS",TotBadRecs);
                 SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,mess);   
                 if (FidFL != HFILE_ERROR)
                 	goto NextFile;
                 FileProjectionType = SaveFPT;  
                 SetGlobalValue("%ALT_PROJECTION",SaveAltProj);
           		 ConvertCoordClose ();
                 DisableHalt = FALSE;  
             	 if (*AutoExportName)
             	 {
					DestroyAdvancedOpts ();
                 	GSSiEndDialog(hWndDlg, TRUE,hSaveBM);
                 } 
                 break;
                 
            }   
          }
          break;

    default:
    	GSSiGlobUlFree (&hMem);    
    	AllowCache = SaveAllowCache;
        return FALSE;
   }
 GSSiGlobUlFree (&hMem);
 AllowCache = SaveAllowCache;
 return TRUE;    
} 

BOOL WriteAdvancedOpts (HFILE FidSave)
{ 
     if (hAttImport)
     {  
     	UINT	Length;
     	short	id=TR_ATTIMPORT;
     	LPSHORT	lpVersion=(LPSHORT)GlobalLock (hAttImport);

     	BigWrite (FidSave,(HPSTR)&id,2,-1);  
     	*lpVersion = 1;
     	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
     	lpVersion++;
     	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
     	Length = *lpVersion++;
     	BigWrite (FidSave,(HPSTR)lpVersion,Length-4,-1);
     	GlobalUnlock (hAttImport);
     }
     if (hStreetSegFields)
     {  
     	short	Length, id=TR_STREET_SEG_FIELDS;
     	LPSHORT	lpVersion=(LPSHORT)GlobalLock (hStreetSegFields);

     	BigWrite (FidSave,(HPSTR)&id,2,-1);  
     	*lpVersion = 1;
     	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
     	lpVersion++;
     	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
     	Length = *lpVersion++;
     	BigWrite (FidSave,(HPSTR)lpVersion,Length-4,-1);
     	GlobalUnlock (hStreetSegFields);
     }
	 if (hImportLimits)
	 {  
	 	short	Length, id=TR_IMPORT_LIMITS;
	 	LPSHORT	lpVersion=(LPSHORT)GlobalLock (hImportLimits);
	
	 	BigWrite (FidSave,(HPSTR)&id,2,-1);  
	 	*lpVersion = 1;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	Length = *lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,Length-4,-1);
	 	GlobalUnlock (hImportLimits);
	 }
	 if (hImportFilter)
	 {  
	 	short	Length, id=TR_IMPORT_FILTER;
	 	LPSHORT	lpVersion=(LPSHORT)GlobalLock (hImportFilter);
	
	 	BigWrite (FidSave,(HPSTR)&id,2,-1);  
	 	*lpVersion = 1;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	Length = *lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,Length-4,-1);
	 	GlobalUnlock (hImportFilter);
	 }
	 if (hGRCommand)
	 {  
	 	short	Length, id=TR_GRCOMMAND;
	 	LPSHORT	lpVersion=(LPSHORT)GlobalLock (hGRCommand);
	
	 	BigWrite (FidSave,(HPSTR)&id,2,-1);  
	 	*lpVersion = 1;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	Length = *lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,Length-4,-1);
	 	GlobalUnlock (hGRCommand);
	 }
	 if (hGRText)
	 {  
	 	short	Length, id=TR_GRTEXT;
	 	LPSHORT	lpVersion=(LPSHORT)GlobalLock (hGRText);
	
	 	BigWrite (FidSave,(HPSTR)&id,2,-1);  
	 	*lpVersion = 1;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	Length = *lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,Length-4,-1);
	 	GlobalUnlock (hGRText);
	 }
	 if (hTimeStamp)
	 {  
	 	short	Length, id=TR_TIMESTAMP;
	 	LPSHORT	lpVersion=(LPSHORT)GlobalLock (hTimeStamp);
	
	 	BigWrite (FidSave,(HPSTR)&id,2,-1);  
	 	*lpVersion = 1;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	Length = *lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,Length-4,-1);
	 	GlobalUnlock (hTimeStamp);
	 }
	 if (hImportRefno)
	 {  
	 	short	Length, id=TR_IMPORT_REFNO;
	 	LPSHORT	lpVersion=(LPSHORT)GlobalLock (hImportRefno);
	
	 	BigWrite (FidSave,(HPSTR)&id,2,-1);  
	 	*lpVersion = 1;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	Length = *lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,Length-4,-1);
	 	GlobalUnlock (hImportRefno);
	 }
	 if (hImportPreSet)
	 {  
	 	short	Length, id=TR_IMPORT_SETUP;
	 	LPSHORT	lpVersion=(LPSHORT)GlobalLock (hImportPreSet);
	
	 	BigWrite (FidSave,(HPSTR)&id,2,-1);  
	 	*lpVersion = 1;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,2,-1);
	 	Length = *lpVersion++;
	 	BigWrite (FidSave,(HPSTR)lpVersion,Length-4,-1);
	 	GlobalUnlock (hImportPreSet);
	 }
	 return TRUE;
}

void DestroyAdvancedOpts (void)
{
     GSSiGlobFree (&hAttImport);
     GSSiGlobFree (&hStreetSegFields);
	 GSSiGlobFree (&hImportLimits);
	 GSSiGlobFree (&hImportFilter);
	 GSSiGlobFree (&hGRCommand);
	 GSSiGlobFree (&hGRText);
	 GSSiGlobFree (&hTimeStamp);
	 GSSiGlobFree (&hImportRefno);
	 GSSiGlobFree (&hImportPreSet);
	 return;
} 

BOOL ImportCommonCode (HWND hWndDlg, WORD Message, WPARAM wParam, LPARAM lParam,HANDLE hDB) 
{
  HANDLE	hStr=GSSiGlobAlloc ( 427,GMEM_MOVEABLE,256);
  LPSTR	str=GlobalLock (hStr);
  short TabStops[2]={10,1300},i; 
  BOOL	RemoveOpt, rtn=FALSE; 
  
  switch(Message)
   {
    case WM_INITDIALOG:
         if (GetDlgItem(hWndDlg,IDC_ADVANCED_OPTS))
         	SendDlgItemMessage (hWndDlg,IDC_ADVANCED_OPTS,LB_SETTABSTOPS,1,(LPARAM)&TabStops); 
         LoadTAGDef (); 
    case GSSI_REINITDIALOG:
    {    
    	 char	CheckMark, str[256];
    	 
         ContinueProcessing=TRUE;  
         
         if (!GetDlgItem(hWndDlg,IDC_ADVANCED_OPTS))
        	 break; 
	     SetWindowText (GetDlgItem(hWndDlg,IDC_AVOADD),"Add");
		 EnableWindow (GetDlgItem(hWndDlg,IDC_AVOADD),FALSE); 
		 EnableWindow (GetDlgItem(hWndDlg,IDC_AVOREMOVE),FALSE);
         SendDlgItemMessage (hWndDlg,IDC_ADVANCED_OPTS,LB_RESETCONTENT,0,0);  
         if (hImportRefno)
         	CheckMark = 'X';
         else
         	CheckMark = ' ';
         sprintf (str,"%c\tRefno",CheckMark);
         SendDlgItemMessage (hWndDlg,IDC_ADVANCED_OPTS,LB_ADDSTRING,0,(LPARAM)str);
          if (hImportPreSet)
         	CheckMark = 'X';
         else
         	CheckMark = ' ';
         sprintf (str,"%c\tPre-Processing Setup",CheckMark);
         SendDlgItemMessage (hWndDlg,IDC_ADVANCED_OPTS,LB_ADDSTRING,0,(LPARAM)str);
          if (hImportFilter)
         	CheckMark = 'X';
         else
         	CheckMark = ' ';
         sprintf (str,"%c\tFilter",CheckMark);
         SendDlgItemMessage (hWndDlg,IDC_ADVANCED_OPTS,LB_ADDSTRING,0,(LPARAM)str);
         if (hImportLimits)
         	CheckMark = 'X';
         else
         	CheckMark = ' ';
         sprintf (str,"%c\tSet Geographic Limits",CheckMark);
         SendDlgItemMessage (hWndDlg,IDC_ADVANCED_OPTS,LB_ADDSTRING,0,(LPARAM)str);
         if (hGRText)
         	CheckMark = 'X';
         else
         	CheckMark = ' ';
         sprintf (str,"%c\tText",CheckMark);
         SendDlgItemMessage (hWndDlg,IDC_ADVANCED_OPTS,LB_ADDSTRING,0,(LPARAM)str);
         if (hGRCommand)
         	CheckMark = 'X';
         else
         	CheckMark = ' ';
         sprintf (str,"%c\tImbedded Command",CheckMark);
         SendDlgItemMessage (hWndDlg,IDC_ADVANCED_OPTS,LB_ADDSTRING,0,(LPARAM)str);
         if (hTimeStamp)
         	CheckMark = 'X';
         else
         	CheckMark = ' ';
         sprintf (str,"%c\tTime Stamp",CheckMark);
         SendDlgItemMessage (hWndDlg,IDC_ADVANCED_OPTS,LB_ADDSTRING,0,(LPARAM)str);
         
	}
         goto Exit;

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
         rtn = TRUE;     
         goto Exit;

    case WM_COMMAND:
#if WIN32
         switch(LOWORD(wParam))
#else
         switch(wParam)
#endif
         {  
         	case IDC_EXIT:  
         		Processing = FALSE;
             	ContinueProcessing = TRUE; 
             	DestroyAdvancedOpts();
				DestroyFieldList ();
             	ForceRefIndex = ForceTAGIndex = FALSE;
         		goto Exit;  
         		
         	case IDC_AVOADD: 
         		RemoveOpt = FALSE;
         		goto AddAvancedOpt; 
         		
         	case IDC_AVOREMOVE:
         		RemoveOpt = TRUE;
         		goto AddAvancedOpt; 
         		
            case IDC_ADVANCED_OPTS:
			{   
				short	Choice;
				
				switch(HIWORD(lParam))
			    { 
			     case LBN_SELCHANGE:
			     {
			    	BOOL	Enable=TRUE; 
			    	
			     	if ((Choice = SendDlgItemMessage(hWndDlg,IDC_ADVANCED_OPTS,LB_GETCURSEL,NULL,NULL)) == LB_ERR)
			     		Enable = FALSE;
			     	else
			     	{
	                	  SendDlgItemMessage(hWndDlg,IDC_ADVANCED_OPTS,LB_GETTEXT,Choice,(DWORD)str); 
	                	  if (*str == 'X')
	                	  	SetWindowText (GetDlgItem(hWndDlg,IDC_AVOADD),"Edit");
			     	}
	                EnableWindow (GetDlgItem(hWndDlg,IDC_AVOADD),Enable);
	                EnableWindow (GetDlgItem(hWndDlg,IDC_AVOREMOVE),Enable);
	                break;
	             }
			     case LBN_DBLCLK: 
			     	RemoveOpt=FALSE;
	AddAvancedOpt:
					{
	        	  	  short	nRc;
	                  FARPROC lpfnMsgProc=0;
	                  char	DLGName[32]; 
	                  
			          Choice=SendDlgItemMessage(hWndDlg,IDC_ADVANCED_OPTS,LB_GETCURSEL,NULL,NULL);
			          if (Choice != LB_ERR)
			          {
	                	  SendDlgItemMessage(hWndDlg,IDC_ADVANCED_OPTS,LB_GETTEXT,Choice,(DWORD)str); 
	                	  if (_fstrstr (str,"Set Geog"))
	                	  {    
	                	  	  if (RemoveOpt) 
	                	  	  	  GSSiGlobFree (&hImportLimits);
	                	  	  else
	                	  	  {
				                  lpfnMsgProc = MakeProcInstance((FARPROC)IMPORT_LIMITSMsgProc, hInst); 
				                  _fstrcpy (DLGName,"IMPORT_LIMITS");  
				              }
			              }
	                	  else if (_fstrstr (str,"Filter"))
	                	  {   
	                	  	  if (RemoveOpt) 
	                	  	  	  GSSiGlobFree (&hImportFilter);
	                	  	  else
	                	  	  {
				                  lpfnMsgProc = MakeProcInstance((FARPROC)IMPORT_FILTERMsgProc, hInst); 
				                  _fstrcpy (DLGName,"IMPORT_FILTER");
				              }
			              }
	                	  else if (_fstrstr (str,"Text"))
	                	  {   
	                	  	  if (RemoveOpt) 
	                	  	  	  GSSiGlobFree (&hGRText);
	                	  	  else
	                	  	  {
				                  lpfnMsgProc = MakeProcInstance((FARPROC)GRTEXTMsgProc, hInst); 
				                  _fstrcpy (DLGName,"GRTEXT");
				              }
			              }
	                	  else if (_fstrstr (str,"Imbedded"))
	                	  {   
	                	  	  if (RemoveOpt) 
	                	  	  	  GSSiGlobFree (&hGRCommand);
	                	  	  else
	                	  	  {
				                  lpfnMsgProc = MakeProcInstance((FARPROC)GRCOMMANDMsgProc, hInst); 
				                  _fstrcpy (DLGName,"GRCOMMAND");
				              }
			              }
	                	  else if (_fstrstr (str,"Import Att"))
	                	  {   
	                	  	  if (RemoveOpt) 
	                	  	  	  GSSiGlobFree (&hAttImport);
	                	  	  else
	                	  	  {
				                  lpfnMsgProc = MakeProcInstance((FARPROC)IMPORT_ATTRIBUTESMsgProc, hInst); 
				                  _fstrcpy (DLGName,"IMPORT_ATTRIBUTES");
				              }
			              }
	                	  else if (_fstrstr (str,"Time Stamp"))
	                	  {   
	                	  	  if (RemoveOpt) 
	                	  	  	  GSSiGlobFree (&hTimeStamp);
	                	  	  else
	                	  	  {
				                  lpfnMsgProc = MakeProcInstance((FARPROC)TIMESTAMPMsgProc, hInst); 
				                  _fstrcpy (DLGName,"TIMESTAMP");
				              }
			              }
	                	  else if (_fstrstr (str,"Street S"))
	                	  {   
	                	  	  if (RemoveOpt) 
	                	  	  	  GSSiGlobFree (&hStreetSegFields);
	                	  	  else
	                	  	  {
				                  lpfnMsgProc = MakeProcInstance((FARPROC)STREETSEG_FIELDSMsgProc, hInst); 
				                  _fstrcpy (DLGName,"STREETSEG_FIELDS");
				              }
			            
			              }
	                	  else if (_fstrstr (str,"Pre-"))
	                	  {   
	                	  	  if (RemoveOpt) 
	                	  	  	  GSSiGlobFree (&hImportPreSet);
	                	  	  else
	                	  	  {   
	                	  	  	  LPSTR	pStr;
	                	  	  	  LPSETREFNO	lpSetRef; 

	                	  	  	  if (!hImportPreSet)
	                	  	  	  	hImportPreSet = GSSiGlobAlloc ( 428,GHND,sizeof(SETREFNO));
	                	  	  	  lpSetRef = (LPSETREFNO)GlobalLock (hImportPreSet);
	                	  	  	  
       	  	  	  			   	  GetTextString (hWndDlg,lpSetRef->SetRefno,256,"Enter Pre-load settings",NULL,NULL,0,TRUE,TRUE);
       	  	  	  			   	  if (*lpSetRef->SetRefno)
       	  	  	  			   	  {
       	  	  	  			   	  	lpSetRef->length = 4 + sizeof(SETREFNO);
       	  	  	  			   	  	GlobalUnlock (hImportPreSet); 
								  }       	  	  	  			   	  	
       	  	  	  			   	  else
       	  	  	  			   	  	GSSiGlobUlFree (&hImportPreSet);
				              }
			              }
	                	  else if (_fstrstr (str,"Refno"))
	                	  {   
	                	  	  if (RemoveOpt) 
	                	  	  	  GSSiGlobFree (&hImportRefno);
	                	  	  else
	                	  	  {   
	                	  	  	  LPSTR	pStr;
	                	  	  	  LPSETREFNO	lpSetRef; 

	                	  	  	  
	                	  	  	  if (!hImportRefno)
	                	  	  	  	hImportRefno = GSSiGlobAlloc ( 429,GHND,sizeof(SETREFNO));
	                	  	  	  lpSetRef = (LPSETREFNO)GlobalLock (hImportRefno);
	                	  	  	  
       	  	  	  			   	  GetTextString (hWndDlg,lpSetRef->SetRefno,256,"Set Refno From",NULL,NULL,0,TRUE,TRUE);
       	  	  	  			   	  if (*lpSetRef->SetRefno)
       	  	  	  			   	  {
       	  	  	  			   	  	lpSetRef->length = 4 + sizeof(SETREFNO);
       	  	  	  			   	  	GlobalUnlock (hImportRefno); 
								  }       	  	  	  			   	  	
       	  	  	  			   	  else
       	  	  	  			   	  	GSSiGlobUlFree (&hImportRefno);
				              }
			              }
			              if (lpfnMsgProc)
			              {   
			              	  ImportAttSourcehDB = hDB;
			                  nRc = DialogBox(hInst, (LPSTR)DLGName, hWndDlg, lpfnMsgProc);
			                  FreeProcInstance(lpfnMsgProc);
	                	  }
	                  } 
	                  PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
                	}
				 	  break; 
			    }
			} 
			rtn = TRUE;
			break;
    default:
        break;
   }
   default:
        break;
   }
Exit:
   GSSiGlobUlFree (&hStr);
   return rtn;    
}    

short GetImportFileType (HWND hWndDlg)
{ 
	short	rtn=0;

    if (SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0))
    	rtn = 1;
    if (SendDlgItemMessage (hWndDlg,IDC_DIRECTORY,BM_GETCHECK,0,0))
    	rtn = 2;
    if (SendDlgItemMessage (hWndDlg,IDC_CHRONDIR,BM_GETCHECK,0,0))
    	rtn = 3;
    return rtn;
} 

void SetImportFileType (HWND hWndDlg, short Type)
{
    SendDlgItemMessage (hWndDlg,IDC_EXISTFILE,BM_SETCHECK,FALSE,0L);
    SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_SETCHECK,FALSE,0L);
    SendDlgItemMessage (hWndDlg,IDC_DIRECTORY,BM_SETCHECK,FALSE,0L);
    SendDlgItemMessage (hWndDlg,IDC_CHRONDIR,BM_SETCHECK,FALSE,0L);  
    switch (Type)
    {
    	case 0:
		    SendDlgItemMessage (hWndDlg,IDC_EXISTFILE,BM_SETCHECK,TRUE,0L);
		    break;
    	case 1:
		    SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_SETCHECK,TRUE,0L);
		    break;
    	case 2:
		    SendDlgItemMessage (hWndDlg,IDC_DIRECTORY,BM_SETCHECK,TRUE,0L);
		    break;
    	case 3:
		    SendDlgItemMessage (hWndDlg,IDC_CHRONDIR,BM_SETCHECK,TRUE,0L);
		    break;
	}
	return;
}
			
BOOL FAR PASCAL POINTMAPMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LPARAM lParam)
{ 
    short   i;
    LPSTR   pPrefix, lpDot;  
    HCURSOR OldCursor=0;    
    LPGWFLDINFO lpGWFldInfo;
    LPGWDHEADER lpGWDHead;
    HANDLE      hBT;    
    double      size=(float)12.0,rot=(float)0.0;
    long        Offset; 
    double      rtn;
    LPVOID      lpVal; 
    short         st, len,ifield, ishort, FileType;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr;
    LPFIELDINFO lpFieldInfo; 
    HANDLE      SaveHandle;
    BOOL        More, Err;    
    WPARAM		OtherParam;
    char		Ext[6]=".TL1";
    short       rc, NewOpt, UnitsOpt,PrecisionOpt; 
    static      short PMDataFileType,LocOpt=0;
    static      HANDLE  PMhDB=0;
    static      char    PMDataFile[128]; 
    HANDLE		hMem=GSSiGlobAlloc ( 430,GMEM_MOVEABLE,4096*2);
    LPSTR		SymName=GlobalLock (hMem);
    LPSTR		Prefix=SymName+64, RefPrefix=Prefix+10,UDI=RefPrefix+10,UDIval= UDI+256;
    LPSTR		XCoordField=UDIval+256, YCoordField=XCoordField+256;
    LPSTR		CSymSize=YCoordField+256,CSymRot=CSymSize+64,CSymColor=CSymRot+64;
    LPSTR		RefUDI=CSymColor+64,SymStuff=CSymColor+128,Name=SymStuff+256;
    LPSTR        str1=Name+256, str2=str1+64;
    LPSTR        str=str2+64; 
    LPSTR		SavePltName=str+256, mess=SavePltName+128;   
    LPSHORT		stuff = (LPSHORT)(mess + 256);
    short		Choice,ii;
    static		BOOL		FileIsOpen=FALSE;
	static	HANDLE	hSaveBM=0;
    

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
 {
 	GSSiGlobUlFree (&hMem);
 	return (BRtn);
 }
 if ((BRtn = ImportCommonCode (hWndDlg,Message, wParam, lParam,PMhDB))) 
 {
 	GSSiGlobUlFree (&hMem);
 	return (BRtn);
 }
 if (DATAFILEMsgProc(hWndDlg,Message,wParam,lParam,0,
                     SV_SET_FILE, SV_DATABASE_LIST, SV_TABLE_NAMES, SV_TABLE_HEADING,NULL,0,
                     PMDataFile, &PMDataFileType, &PMhDB,NULL,TRUE))  
 {
 	GSSiGlobUlFree (&hMem);
 	return (TRUE);
 }
                     
 switch(Message)
   {
    case WM_INITDIALOG:
    	 hSaveBM = EnterBlockingWindow (hWndDlg);
         SendDlgItemMessage (hWndDlg,IDC_UNIQUEREFNO,BM_SETCHECK,TRUE,0L);
         SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_SETCHECK,TRUE,0L);
         if (NumTAGDef)
         { 
            LPTAGDEF    lpTAGDef; 
            
            lpTAGDef = (LPTAGDEF)GlobalLock (hTAGDef);
            for (i=0;i<NumTAGDef;i++,lpTAGDef++) 
            {
                SendDlgItemMessage (hWndDlg,IDC_TAPREFIX,CB_ADDSTRING,0,(LPARAM)lpTAGDef->Prefix);
                SendDlgItemMessage (hWndDlg,IDC_REF_PREFIX,CB_ADDSTRING,0,(LPARAM)lpTAGDef->Prefix);
            }
            GlobalUnlock (hTAGDef);
         } 
         SendDlgItemMessage (hWndDlg,IDC_METHOD,CB_ADDSTRING,0,(LPARAM)"By Coordinates");
         SendDlgItemMessage (hWndDlg,IDC_METHOD,CB_ADDSTRING,0,(LPARAM)"From TAGed Item");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Feet");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Meters");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Degrees");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Degrees * 1000000");
         _fstrcpy (str,"*.CVT");
         DlgDirListComboBox (hWndDlg,str,IDC_PROJECTION,0,DDL_READWRITE); 
		 if (PRJ_UNITS[1] == 1)
 		 	SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_SETCURSEL,(WPARAM)0,(LPARAM)NULL); 
		 else if (PRJ_UNITS[1] == 2)
 		 	SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_SETCURSEL,(WPARAM)1,(LPARAM)NULL); 
 		 SendDlgItemMessage (hWndDlg,IDC_PROJECTION,CB_SELECTSTRING,(WPARAM)-1,(LPARAM)"baseproj"); 
         if (*curunits)
         	SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_SELECTSTRING,-1,(LPARAM)curunits);
         if (*curproject)
         	SendDlgItemMessage (hWndDlg,IDC_PROJECTION,CB_SELECTSTRING,-1,(LPARAM)curproject);
         if (*AutoExportName)
		 	PostMessage(hWndDlg, WM_COMMAND, IDC_RECALL, 0L);
    case GSSI_REINITDIALOG:
    {    
    	 short	showopt2[2]={SW_HIDE,SW_SHOW};
    	 short	showopt1[2]={SW_SHOW,SW_HIDE}; 
         
 		 SendDlgItemMessage (hWndDlg,IDC_METHOD,CB_SETCURSEL,(WPARAM)(LocOpt),(LPARAM)NULL);  
		 ShowWindow (GetDlgItem(hWndDlg,IDC_XFIELD),showopt1[LocOpt]);
		 ShowWindow (GetDlgItem(hWndDlg,IDC_YFIELD),showopt1[LocOpt]);
		 ShowWindow (GetDlgItem(hWndDlg,IDC_XFIELD_TITLE),showopt1[LocOpt]);
		 ShowWindow (GetDlgItem(hWndDlg,IDC_YFIELD_TITLE),showopt1[LocOpt]);
		 ShowWindow (GetDlgItem(hWndDlg,IDC_PROJECTION),showopt1[LocOpt]);
		 ShowWindow (GetDlgItem(hWndDlg,IDC_PROJECTION_TITLE),showopt1[LocOpt]);
		 ShowWindow (GetDlgItem(hWndDlg,IDC_UNITS),showopt1[LocOpt]);
		 ShowWindow (GetDlgItem(hWndDlg,IDC_UNITS_TITLE),showopt1[LocOpt]); 
		 ShowWindow (GetDlgItem(hWndDlg,IDC_REF_PREFIX),showopt2[LocOpt]);
		 ShowWindow (GetDlgItem(hWndDlg,IDC_REF_PREFIX_TITLE),showopt2[LocOpt]);
		 ShowWindow (GetDlgItem(hWndDlg,IDC_REF_UDI),showopt2[LocOpt]);
		 ShowWindow (GetDlgItem(hWndDlg,IDC_REF_UDI_TITLE),showopt2[LocOpt]); 
         if (FileIsOpen && *AutoExportName)
	         PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
	}
         break; /* End of WM_INITDIALOG                                 */

    case WM_COMMAND:
#if WIN32
         switch(LOWORD(wParam))
#else
         switch(wParam)
#endif
         {  
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 if (Processing)
                    ContinueProcessing=FALSE;
                 break;
            
            case IDC_METHOD:
			{
				switch(HIWORD(lParam))
			    {
			     case LBN_DBLCLK:
			     case LBN_SELCHANGE:
			          LocOpt=SendDlgItemMessage(hWndDlg,wParam,CB_GETCURSEL,NULL,NULL);
	                  PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
				 	  break; 
			    }
			}
				 break;
            case IDC_SHOW_FIELDS:
            	 DisplayFieldList (hWndDlg,PMhDB,NULL,0);
                 break;
                      
            case IDC_LOCATE_DESTMAP: 
                 *str=0;
				 if (SendDlgItemMessage (hWndDlg,IDC_DIRECTORY,BM_GETCHECK,0,0))
				 {
                     if (!GetFileName3 (hWndDlg,str,IDS_FILTERINDEX,IDS_FILEINDEX)) break;
                 }   
				 else
				 {
	                 if (SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0)) 
	                 {
	                    if (!GetSaveName2 (hWndDlg,str,IDS_FILTERPLT,".PLT",IDS_FILEPLT)) break;   
	                 }
	                 else  
	                 {
	                    if (!GetFileName3 (hWndDlg,str,IDS_FILTERPLT,IDS_FILEPLT)) break;   
	                 } 
	             }
                 SetDlgItemText (hWndDlg,IDC_DESTMAP,str);
                 break;
                 
            case IDC_OPEN_DB:
                 EnableWindow (GetDlgItem(hWndDlg,IDC_SAADD),TRUE);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_SHOW_FIELDS),TRUE);
                 break;
                      
            case IDC_SAADD:      
            {    
                 HANDLE hMem;
                 LPSTR  lpStr, lpWhere;
                 
                 _fstrcpy (CSymSize,"[%NEW_POINT_SIZE]");
                 ExpandText (CSymSize);
                 _fstrcpy (CSymRot,"[%NEW_POINT_ROT]");
                 ExpandText (CSymRot);
                 _fstrcpy (CSymColor,"[%NEW_POINT_COLOR]");
                 ExpandText (CSymColor);  
                 *SymName = 0;
                 if (!SelectPointSymbol (hWndDlg,1,SymName,"All",CSymSize,CSymRot,CSymColor,FALSE))
                    break;

                 hMem = GSSiGlobAlloc ( 431,GHND,4096);
                 lpStr = GlobalLock (hMem);
                 if (GetSQLWhereClause (hWndDlg, PMhDB, lpStr))
                 {  
                 	sprintf (str,"%s;%s;%s;%s",SymName,CSymSize,CSymRot,CSymColor);
	                SendDlgItemMessage (hWndDlg,IDC_SYMBOL_LIST,LB_ADDSTRING,0,(LPARAM)str);
	                SendDlgItemMessage (hWndDlg,IDC_SQL_LIST,LB_ADDSTRING,0,(LPARAM)lpStr);
                 }
                 GlobalUnlock (hMem);
                 GlobalFree (hMem);
                 break;
            }    
            case IDC_SQL_LIST:  
            case IDC_SYMBOL_LIST:
			{
				switch(HIWORD(lParam))
			    {
			     case LBN_DBLCLK:
			          PostMessage(hWndDlg, WM_COMMAND, IDC_SAMODIFY, 0L); 
			          break;
			     case LBN_SELCHANGE:
			          Choice=SendDlgItemMessage(hWndDlg,wParam,LB_GETCURSEL,NULL,NULL);
			          if (wParam == IDC_SQL_LIST)
			          	OtherParam = IDC_SYMBOL_LIST;
			          else
			          	OtherParam = IDC_SQL_LIST; 
			 		  SendDlgItemMessage(hWndDlg,OtherParam,LB_SETCURSEL,-1,0);
				 	  EnableWindow (GetDlgItem(hWndDlg,IDC_SADELETE),TRUE);
				 	  EnableWindow (GetDlgItem(hWndDlg,IDC_SAMODIFY),TRUE);
				 	  break; 
			    }
			}
				 break;
            case IDC_SADELETE:
		         Choice=max(SendDlgItemMessage(hWndDlg,IDC_SYMBOL_LIST,LB_GETCURSEL,NULL,NULL),
							SendDlgItemMessage(hWndDlg,IDC_SQL_LIST,LB_GETCURSEL,NULL,NULL));
	             SendDlgItemMessage (hWndDlg,IDC_SYMBOL_LIST,LB_DELETESTRING,Choice,0);
               	 SendDlgItemMessage (hWndDlg,IDC_SQL_LIST,LB_DELETESTRING,Choice,0);
                 break;
            
            case IDC_SAMODIFY:
		         if ((Choice=SendDlgItemMessage(hWndDlg,IDC_SYMBOL_LIST,LB_GETCURSEL,NULL,NULL)) >= 0)
		         {
	                 SendDlgItemMessage(hWndDlg,IDC_SYMBOL_LIST,LB_GETTEXT,Choice,(DWORD)SymStuff);
	                 SetSymParms (SymStuff,SymName,CSymSize,CSymRot,CSymColor);            
                 	 if (!SelectPointSymbol (hWndDlg,1,SymName,"All",CSymSize,CSymRot,CSymColor,FALSE))
	                    break;
	                 SendDlgItemMessage (hWndDlg,IDC_SYMBOL_LIST,LB_DELETESTRING,Choice,0);
                 	 sprintf (SymStuff,"%s;%s;%s;%s",SymName,CSymSize,CSymRot,CSymColor);
	                 SendDlgItemMessage (hWndDlg,IDC_SYMBOL_LIST,LB_INSERTSTRING,Choice,(LPARAM)SymStuff);
		         }
		         else
		         if ((Choice=SendDlgItemMessage(hWndDlg,IDC_SQL_LIST,LB_GETCURSEL,NULL,NULL)) >= 0)
		         {   
		         	 HANDLE	hMem;
		         	 LPSTR	lpStr;
		         	 
	                 hMem = GSSiGlobAlloc ( 432,GHND,4096);
	                 lpStr = GlobalLock (hMem);
                	 SendDlgItemMessage(hWndDlg,IDC_SQL_LIST,LB_GETTEXT,Choice,(DWORD)lpStr); 
	                 if (GetSQLWhereClause (hWndDlg, PMhDB, lpStr))
	                 {
	                 	SendDlgItemMessage (hWndDlg,IDC_SQL_LIST,LB_DELETESTRING,Choice,0);
		                SendDlgItemMessage (hWndDlg,IDC_SQL_LIST,LB_INSERTSTRING,Choice,(LPARAM)lpStr);
	                 }
	                 GlobalUnlock (hMem);
	                 GlobalFree (hMem);
		         }
		         
                 break;
            
            case IDC_EXIT: 
                 CloseDataFile (TRUE, &PMhDB);  
				 DestroyAdvancedOpts ();
	             GSSiEndDialog(hWndDlg, ContinueProcessing,hSaveBM);  
			 	 ContinueProcessing=TRUE;
                 break;
                 
            case IDC_UNIQUEREFNO:
            {
            	 BOOL	On=SendDlgItemMessage (hWndDlg,IDC_UNIQUEREFNO,BM_GETCHECK,0,0);
            	 
            	 if (On)
            	 	GetGlobalCVal ("[%STARTREFNO]",str,"1000000");
            	 else
            	 	_fstrcpy (str,"1"); 
         		 SetDlgItemText (hWndDlg,IDC_STARTNO,str);
			}
				 break;
				 
            case IDC_SAVE:
            {    
            	 short	Version=2; 
            	 HFILE	FidSave; 
            	 OFSTRUCT	OFStruct;
            	 
                 if (!GetSaveName2 (hWndDlg,Name,0,Ext,IDS_FILETL1)) break; 
        Update:
                 FidSave = GSSiOpenFile (Name,&OFStruct,OF_CREATE);
                 BigWrite (FidSave,(HPSTR)Ext,6,-1);
                 BigWrite (FidSave,(HPSTR)&Version,2,-1);   
                 GetDlgItemText (hWndDlg,SV_DATABASE_LIST,PMDataFile,128);
                 BigWrite (FidSave,(HPSTR)PMDataFile,128,-1);
                 GetDlgItemText (hWndDlg,IDC_DESTMAP,PltName,128);
                 BigWrite (FidSave,(HPSTR)PltName,128,-1);
                 FileType = GetImportFileType (hWndDlg);
                 BigWrite (FidSave,(HPSTR)&FileType,2,-1);
			     LocOpt=SendDlgItemMessage(hWndDlg,IDC_METHOD,CB_GETCURSEL,NULL,NULL); 
                 BigWrite (FidSave,(HPSTR)&LocOpt,2,-1);
                 GetDlgItemText (hWndDlg,IDC_XFIELD,XCoordField,256);
                 BigWrite (FidSave,(HPSTR)XCoordField,256,-1);
                 GetDlgItemText (hWndDlg,IDC_YFIELD,YCoordField,256); 
                 BigWrite (FidSave,(HPSTR)YCoordField,256,-1);
                 GetDlgItemText (hWndDlg,IDC_PROJECTION,curproject,lncurproject);
                 ii=BigWrite (FidSave,(HPSTR)curproject,34,-1);
			     UnitsOpt=SendDlgItemMessage(hWndDlg,IDC_UNITS,CB_GETCURSEL,NULL,NULL); 
                 ii=BigWrite (FidSave,(HPSTR)&UnitsOpt,2,-1);
                 PrecisionOpt = SendDlgItemMessage (hWndDlg,IDC_HIGH_PRECISION,BM_GETCHECK,0,0);
                 BigWrite (FidSave,(HPSTR)&PrecisionOpt,2,-1);
                 GetDlgItemText(hWndDlg,IDC_REF_PREFIX,RefPrefix,10);
                 BigWrite (FidSave,(HPSTR)RefPrefix,10,-1);
                 GetDlgItemText(hWndDlg,IDC_REF_UDI,RefUDI,128);               
                 BigWrite (FidSave,(HPSTR)RefUDI,128,-1);
                 GetDlgItemText(hWndDlg,IDC_TAPREFIX,Prefix,10);
                 BigWrite (FidSave,(HPSTR)Prefix,10,-1);
                 GetDlgItemText(hWndDlg,IDC_UDI,UDI,256);               
                 BigWrite (FidSave,(HPSTR)UDI,256,-1);
				 HiPrecis = SendDlgItemMessage (hWndDlg,IDC_UNIQUEREFNO,BM_GETCHECK,0,0);
				 BigWrite (FidSave,(HPSTR)&HiPrecis ,2,-1);
                 GetDlgItemText (hWndDlg,IDC_STARTNO,str,16);
                 BigWrite (FidSave,str,16,-1);
                 ishort = SendDlgItemMessage(hWndDlg,IDC_SYMBOL_LIST,LB_GETCOUNT,0,0); 
                 if (SendDlgItemMessage (hWndDlg,IDC_AUTOCREATENEWSYMS,BM_GETCHECK,0,0))
                 	ishort = -ishort;
                 BigWrite (FidSave,(HPSTR)&ishort,2,-1); 
                 ishort = abs(ishort);
                 i=0;
                 while (ishort--)
                 {
                	SendDlgItemMessage(hWndDlg,IDC_SYMBOL_LIST,LB_GETTEXT,i,(DWORD)SymStuff);
                	BigWrite (FidSave,(HPSTR)SymStuff,256,-1);
                	SendDlgItemMessage(hWndDlg,IDC_SQL_LIST,LB_GETTEXT,i++,(DWORD)str); 
                	BigWrite (FidSave,(HPSTR)str,256,-1);
                 }
                 WriteAdvancedOpts (FidSave);                
                 GSSiClose (FidSave);
            } 
                    
            	 break;
            	 
           	case IDC_RECALL: 
           	{
           		 short Version;     
            	 HFILE	FidSave;
            	 OFSTRUCT	OFStruct;
           		 
           		 
             	 if (!*AutoExportName)
             	 { 
					 if (!GetFileName2 (hWndDlg,Name,Ext,IDS_FILETL1))
					 	break;
	             }
	             else
	             	_fstrcpy (Name,AutoExportName);  
                 FidSave = GSSiOpenFile (Name,&OFStruct,OF_READ);
                 BigRead (FidSave,Ext,6);
                 BigRead (FidSave,(HPSTR)&Version,2);   
                 BigRead (FidSave,PMDataFile,128);
                 BigRead (FidSave,PltName,128);
                 SetDlgItemText (hWndDlg,IDC_DESTMAP,PltName);
                 BigRead (FidSave,(HPSTR)&FileType,2);  
                 SetImportFileType (hWndDlg,FileType);
                 BigRead (FidSave,(HPSTR)&LocOpt,2);
                 BigRead (FidSave,XCoordField,256);
                 SetDlgItemText (hWndDlg,IDC_XFIELD,XCoordField);
                 BigRead (FidSave,YCoordField,256);
                 SetDlgItemText (hWndDlg,IDC_YFIELD,YCoordField);
                 ii=BigRead (FidSave,curproject,34);
		         SendDlgItemMessage (hWndDlg,IDC_PROJECTION,CB_SELECTSTRING,-1,(LPARAM)curproject);
                 ii=BigRead (FidSave,(HPSTR)&UnitsOpt,2);
			     SendDlgItemMessage(hWndDlg,IDC_UNITS,CB_SETCURSEL,UnitsOpt,NULL); 
                 BigRead (FidSave,(HPSTR)&PrecisionOpt,2);
                 SendDlgItemMessage (hWndDlg,IDC_HIGH_PRECISION,BM_SETCHECK,PrecisionOpt,0);
                 BigRead (FidSave,RefPrefix,10);
                 SetDlgItemText(hWndDlg,IDC_REF_PREFIX,RefPrefix);
                 BigRead (FidSave,RefUDI,128);
                 SetDlgItemText(hWndDlg,IDC_REF_UDI,RefUDI);               
                 BigRead (FidSave,Prefix,10);
                 SetDlgItemText(hWndDlg,IDC_TAPREFIX,Prefix);
                 BigRead (FidSave,UDI,256);
                 SetDlgItemText(hWndDlg,IDC_UDI,UDI);               
			     if (Version > 1)
			     {   
	                 ii=BigRead (FidSave,(HPSTR)&HiPrecis,2);
	                 SendDlgItemMessage (hWndDlg,IDC_UNIQUEREFNO,BM_SETCHECK,HiPrecis,0);
	                 BigRead (FidSave,str,16);
	                 SetDlgItemText(hWndDlg,IDC_STARTNO,str);               
			     }
                 BigRead (FidSave,(HPSTR)&ishort,2);  
                 if (ishort < 0)
                 {
                 	ishort = -ishort;
                 	SendDlgItemMessage (hWndDlg,IDC_AUTOCREATENEWSYMS,BM_SETCHECK,TRUE,0);
				 } 
				 else
                 	SendDlgItemMessage (hWndDlg,IDC_AUTOCREATENEWSYMS,BM_SETCHECK,FALSE,0);
                 SendDlgItemMessage (hWndDlg,IDC_SYMBOL_LIST,LB_RESETCONTENT,0,0);
                 SendDlgItemMessage (hWndDlg,IDC_SQL_LIST,LB_RESETCONTENT,0,0);
                 while (ishort--)
                 {
                	if (BigRead (FidSave,SymStuff,256) != 256)
                		goto ExitPMRecall;
                	SendDlgItemMessage(hWndDlg,IDC_SYMBOL_LIST,LB_ADDSTRING,0,(DWORD)SymStuff);
                	if (BigRead (FidSave,str,256) != 256)
                		goto ExitPMRecall;
                	SendDlgItemMessage(hWndDlg,IDC_SQL_LIST,LB_ADDSTRING,0,(DWORD)str); 
                 }                
			     while (ReadObject (&FidSave, FALSE,NULL,NULL)); 
	ExitPMRecall:
                 GSSiClose (FidSave);
                 FileIsOpen = TRUE;
                 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
            }
           		 break;
           		 
            case IDOK: 
            {
                 long   lineno=0, TotLen, CurLoc, MidLine, ii, TotPoints, NumBad,NumLoaded;  
                 long   NewRefno, StartRefno; 
                 short    st, SymNum, iUDI=0, AreaSym, LineSym, Pass=1, pos;
                 BOOL   Done, First=TRUE, BatchMode=FALSE, UniqueRefno;     //batchmode not setting refindex parameters correctly 7/17/2006
                 HANDLE hMIDstr;
                 double X,Y;
                 LPSTR  lpTAB, pSQL, pUDI=UDI; 
                 DPOINT Point;
                 BOOL   Store, Err, HiPrecis, Create, FileIsDir=FALSE, UsingHltList,Status;
                 short    Symbol=1,nr;   
                 char	SaveAltProj[34];
                 COLORREF   color;      
                 LPGRCOMMAND	lpGRCommand;
                 LPSTR  lpDot, lpFld, lpSC; 
                 MNMXCORD MinMaxCoord;  
                 double coordcvt=1;
                 short    NumSyms=0, NumRules, Rule;  
                 HANDLE hSymDesc=0;
                 BOOL	UseSingleRulesPass;
                  
				 ContinueProcessing=TRUE;
	             UniqueRefno=SendDlgItemMessage (hWndDlg,IDC_UNIQUEREFNO,BM_GETCHECK,0,0);
			 	 if (GetDlgItemText (hWndDlg,IDC_STARTNO,str,128))
			 	 {
			 		ExpandText (str);
			 		StartRefno = atol (str);
			 	 }
			 	 else
			 		StartRefno = 1;
                 if (!GetDlgItemText (hWndDlg,IDC_DESTMAP,PltName,128))
                 {
                    GSSiMsgBox(GetFocus(),"No destination map", 0,MB_ICONQUESTION|MB_OK);
                    break;
                 }
                 GetGlobalCVal ("[%ALT_PROJECTION]",SaveAltProj,NULL);
			     LocOpt=SendDlgItemMessage(hWndDlg,IDC_METHOD,CB_GETCURSEL,NULL,NULL); 
                 if (LocOpt == 1)
                 {
                    GetDlgItemText (hWndDlg,IDC_REF_PREFIX,RefPrefix,10);
                    GetDlgItemText (hWndDlg,IDC_REF_UDI,RefUDI,128); 
                    if (!*RefPrefix ||!*RefUDI)
                    {
                        GSSiMsgBox(GetFocus(),"Reference Prefix and/or UDI missing", 0,MB_ICONQUESTION|MB_OK);
                        break;
                    }
                 }
                 else 
                 {
                    GetDlgItemText (hWndDlg,IDC_XFIELD,XCoordField,256);
                    GetDlgItemText (hWndDlg,IDC_YFIELD,YCoordField,256); 
                    if (!*XCoordField ||!*YCoordField)
                    {
                        GSSiMsgBox(GetFocus(),"X and/or Y coordinate field missing", 0,MB_ICONQUESTION|MB_OK);
                        break;
                    }
                    if (!GetDlgItemText (hWndDlg,IDC_PROJECTION,curproject,lncurproject))
                    {
                        GSSiMsgBox(GetFocus(),"No input projection set", 0,MB_ICONEXCLAMATION|MB_OK);
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
                        GSSiMsgBox(GetFocus(),"Units field not set", 0,MB_ICONQUESTION|MB_OK);
                        break;
                    }
                 }
                 NumRules = SendDlgItemMessage(hWndDlg,IDC_SYMBOL_LIST,LB_GETCOUNT,0,0);
                 if (NumRules<=0)  
                 {  
                    GSSiMsgBox(GetFocus(),"No symbol assignment rules", 0,MB_ICONQUESTION|MB_OK);
                    break;
                 }  

             	 if (hImportPreSet)
             	 {
             	 	LPSETREFNO pSetRef = (LPSETREFNO)GlobalLock (hImportPreSet);
		                 	 	
             	 	ProcessText (pSetRef->SetRefno);
             	 	GlobalUnlock (hImportPreSet);
             	 }
                 
                 SaveFPT = FileProjectionType; 
				 FileProjectionType=0; 
				 Processing = TRUE;
                 DisableHalt = TRUE;   
                 DoPaint = FALSE;
                 ExpandText (PltName);
                 PltType = 2;
                 Done = FALSE;
                 MidLine=0; 
                 str[0]=0;  
/*                 for (Rule=0;Rule<NumRules;Rule++)
                 {
	                SendDlgItemMessage(hWndDlg,IDC_SYMBOL_LIST,LB_GETTEXT,Rule,(DWORD)SymStuff); 
	                if ((lpSC = _fstrchr (SymStuff,';')))
						*lpSC = 0;
					DecodePointSym (SymStuff,SymName,CSymSize,CSymRot,CSymColor);
					PointSym = GetOrCreateSym (SymName,&NumSyms,&hSymDesc,Create,2);
	             }*/
                 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),FALSE);
                 HiPrecis = SendDlgItemMessage (hWndDlg,IDC_HIGH_PRECISION,BM_GETCHECK,0,0);
                 FileType = 0;
                 if (SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0))
                 { 
                    Pass = 0;       
                    FileType = 1;
                    if (hImportLimits) 
                    {   
                    	LPSHORT	pint;
                    	LPMNMXCORD	pMinMax;
                    	
                    	pint = (LPSHORT)GlobalLock (hImportLimits);
                    	pint+=2;
                    	pMinMax = (LPMNMXCORD)pint;
                    	EditBounds = *pMinMax;
                    	GlobalUnlock (hImportLimits);  
                    	goto SkipFirstPass;
                    }
                 }
                 else 
                 {   
                    Pass = 1; 
                 }
                 if (SendDlgItemMessage (hWndDlg,IDC_DIRECTORY,BM_GETCHECK,0,0))
                 {
                 	 FileType = 2;   
                 	 FileIsDir = TRUE; 
                 	 BatchMode = FALSE;
                 	 DBoundsInit (&EditBounds);
                 }
                 else if (SendDlgItemMessage (hWndDlg,IDC_CHRONDIR,BM_GETCHECK,0,0))
                 {
                 	 FileType = 3;     
                 	 FileIsDir = TRUE;   
                 	 BatchMode = FALSE;
                 	 _fstrcpy (str,PltName);
                 	 if (!OpenChronoIndex (str,&EditBounds))
                 	 {
                     	GSSiMsgBox(GetFocus(),"Unable to open index", PltName,MB_ICONQUESTION|MB_OK);
                     	break;
                     }
                 }
                 else if (Pass)
                 {
					 OpenMap (CurView->hWnd,CurView->hDC);
					 EditBounds = CurView->FileMNMX; 
					 CloseMap (FALSE);
				 }
                 
NextPass:       
				 NewRefno=0; 
				 TotPoints = 0;
				 NumBad = NumLoaded = 0;
                 if (!Pass)
                 	DBoundsInit (&EditBounds);
                 pPrefix = Prefix;    
                 GetDlgItemText(hWndDlg,IDC_TAPREFIX,Prefix,10);
                 UDI[0]=0; 
                 if (*pPrefix) 
                     GetDlgItemText(hWndDlg,IDC_UDI,UDI,256);               
                 else
                    pPrefix = 0;   
                    
				Create = SendDlgItemMessage (hWndDlg,IDC_AUTOCREATENEWSYMS,BM_GETCHECK,0,0);
                CloseDataFile (TRUE, &PMhDB);
                if ((UseSingleRulesPass = FALSE))//SendDlgItemMessage (hWndDlg,IDC_SINGLEPASS,BM_GETCHECK,0,0)))  
                	nr = 1;
                else
                	nr = NumRules;
                for (Rule=0;Rule<nr;Rule++)
                {   
                	if (!ContinueProcessing)
                		break;
                	lineno = 0;
                	First = TRUE;
	                SendDlgItemMessage(hWndDlg,IDC_SQL_LIST,LB_GETTEXT,Rule,(DWORD)str);                
	                pSQL = str;
	                if (!_fstrcmp (pSQL,"ALL ROWS") || UseSingleRulesPass)
	                    *pSQL = 0;
	                    
	                PMhDB = 0;          
	                if (_fstrstr (pSQL,"[%PICKED"))
	                {
	                	UsingHltList = TRUE; 
	                	pos = BT_FIRST;
	                }
	                else
	                	UsingHltList = FALSE;
	                
	                if (!OpenDataFile (PMDataFile,pSQL,BT_READ,&PMhDB))
	                {  
	                    GSSiMsgBox(GetFocus(),"Cannot open data file", PMDataFile,MB_ICONQUESTION|MB_OK);
	                    break;
	                }
	                    
	                SendDlgItemMessage(hWndDlg,IDC_SYMBOL_LIST,LB_GETTEXT,Rule,(DWORD)SymStuff);
//	                SetSymParms (SymStuff,SymName,CSymSize,CSymRot,CSymColor);            
					DecodePointSym (SymStuff,SymName,CSymSize,CSymRot,CSymColor);
	                if (First)
	                {   
	                	HCURSOR	OldCursor;
	                	
		                sprintf (mess,"Retrieving records for SQL statement %i",Rule+1);
		                SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,mess);
	                    OldCursor = GSSiSetCursor (LoadCursor (0,IDC_WAIT)); 
	                    if (UsingHltList)
	                    	TotLen = BT_NUM_IN_INDEX (hHighlight);
	                    else
	                    	TotLen = NumSQLRows (PMhDB);  
			            GSSiSetCursor (OldCursor);
	                }
	                First = FALSE;
					if (Pass)
					{
						sprintf (mess,"Loading Data for SQL statement %i",Rule+1);
						SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,mess);
					}
					else
					{
						sprintf (mess,"Scanning for min/max coordinates for SQL statement %i",Rule+1);
						SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,mess);
					}
			NextHlt:
	                 if (UsingHltList)
	                 {  
	                 	HIGHLIGHTDATA	HighlightData; 
	                 	long	iref;
	                 	
	                 	if (BT_FIND (hHighlight,(LPSTR)&iref,pos,BT_ANY,(LPSTR)&HighlightData))
	                 		goto EndFile;
	                 	pos = BT_NEXT;
						PickList[0]=HighlightData.PD; 
						SetPickGlobals (0);
	                 }                   
	                 while (FetchDBRec (PMhDB) && ContinueProcessing)
	                 {   
		                 lineno++; 
	                 	 TotPoints++;
		                 if (LocOpt == 1) 
		                 {
		                    _fstrcpy (str,RefUDI);
		                    ExpandText(str); 
		                    _fstrcpy (SavePltName,PltName); 
							st=PickByRefno (NULL,RefPrefix,str,-1);  
							_fstrcpy (PltName,SavePltName);
							if (st)
							{
		                 		Point = PickList[0].PickedPoint;
		                 	}
		                 	else
		                 	{
		                 		NumBad++;
		                 		goto NextPoint;
		                 	}
		                 }
		                 else 
		                 {  
		                    _fstrcpy (str,XCoordField);
		                    ExpandText(str);  
		                    Point.x = atof (str)*coordcvt;
		                    _fstrcpy (str,YCoordField);
		                    ExpandText(str);  
		                    Point.y = atof (str)*coordcvt;
		                    if (*curproject)
		                        ConvertCoord(&Point,3,1);
		                 }
		                 if (*UDI)
		                 {
		                    _fstrcpy (UDIval,UDI);
		                    ExpandText (UDIval); 
		                    if (_fstrlen (UDIval) > 64)
		                    	ii=1;
		                 } 
		                 if (Point.y < 0)
		                 	ii=1;
	                 	 Store = PointInFileBounds (&Point,&EditBounds,Pass); 
		                 if (!Store && Pass > 0)
		                 {  
		                 	NumBad++;
		                 	if (GetGlobalPVal ("[%BADCOORDPOINT]",NULL,&Point))
		                 		Store = PointInFileBounds (&Point,&EditBounds,Pass); 
		                 }
		                 else
		                 	ii=1;
		                 if (Store && FileType == 3 && hTimeStamp)
		                 {  
		                 	HANDLE	hStr=GSSiGlobAlloc ( 433,GMEM_MOVEABLE,256);
		                 	LPSTR	pStr=GlobalLock (hStr);
							LPTIMESTAMP	lpTimeStamp=(LPTIMESTAMP)GlobalLock (hTimeStamp); 
							long	StartTime,EndTime;
							
							_fstrcpy (pStr,lpTimeStamp->StartTime);
							ExpandText (pStr);
							StartTime = atol (pStr);
							_fstrcpy (pStr,lpTimeStamp->EndTime);
							ExpandText (pStr);
							EndTime = atol (pStr); 
							EndTime = max (StartTime,EndTime);
							GlobalUnlock (hTimeStamp);
							GSSiGlobUlFree (&hStr);
							Store = GetChronoIndexedEditFile (PltName,StartTime, EndTime);
						 }
		                 if (Pass && Store)
		                 {   
		                 	 if (hImportRefno)
		                 	 {
		                 	 	LPSETREFNO pSetRef = (LPSETREFNO)GlobalLock (hImportRefno);
		                 	 	HANDLE	hStr = GSSiGlobAlloc ( 434,GMEM_MOVEABLE,512);
		                 	 	LPSTR	pStr = GlobalLock (hStr);
		                 	 	
		                 	 	_fstrcpy (pStr,pSetRef->SetRefno);
		                 	 	ExpandText (pStr);
		                 	 	NewRefno = atol (pStr);
		                 	 	GSSiGlobUlFree (&hStr);
		                 	 	GlobalUnlock (hImportRefno);
		                 	 }
		                 	 else
								NewRefno = GetNextRefno (&StartRefno,UniqueRefno,TRUE); 
							 DecodePointSym (SymStuff,SymName,CSymSize,CSymRot,CSymColor);
							 _fstrcpy (str,SymName);
							 ExpandText (str);
							 Symbol = GetOrCreateSym (SymName,&NumSyms,&hSymDesc,Create,1);
    	                     stuff[0]=0;
		                     if (hGRCommand)
		                     {   
		                     	 short lcmdstring;
		                     	 LPGRCOMMAND	lpGRCommand=(LPGRCOMMAND)GlobalLock (hGRCommand);
		                     	 char	CmdString[256];
		                     	 
		                     	 lcmdstring = lpGRCommand->lcmdstring;
		                     	 lcmdstring += lcmdstring%2; 
		                     	 _fstrcpy (CmdString,lpGRCommand->CmdString);
			                     ExpandText (CmdString);
			                     stuff[1]=40;
			                     stuff[2]=lcmdstring;
			                     _fstrncpy ((LPSTR)&stuff[3],CmdString,lcmdstring);
			                     stuff[0]+=2+2+lcmdstring; 
			                     GlobalUnlock (hGRCommand);
			                 }
			                 if (!SetPointSize (&size,CSymSize))
			                 	goto EndFile;
			                 if (!SetPointRot (&rot,CSymRot))
			                 	goto EndFile;
			                 if (!SetPointColor (&color,CSymColor))
			                 	goto EndFile;
		                     if (AddPointToMap (Point,NewRefno,hTimeStamp,Symbol,size,rot,stuff,hGRText,0,pPrefix,UDIval,color,color,-1,BatchMode,HiPrecis,NULL,NULL))
		                     	NumLoaded++;
                             else
	                            RemoveRefno (NewRefno,&StartRefno,UniqueRefno); 
		                     if (LocOpt == 1)
		                     	CloseMap(TRUE);
		                 } 
		                 else
		                 	ii=1;
		    NextPoint:
		                 PctBox (GetDlgItem(hWndDlg,IDC_STATUS), TotLen, lineno,0);
		             } 
		             if (UsingHltList && ContinueProcessing)
		             	goto NextHlt;
                 
        EndFile: 
	                 CloseRefIndex(FALSE);           
	                 if (PMhDB)
	                 {
	                    CloseDataFile (TRUE, &PMhDB);  
	                 } 
	                 CloseMap(TRUE);
		         }
                 if (NumBad == TotPoints && !*AutoExportName) 
                 {  
                    GSSiMsgBox(GetFocus(),"Unable to locate any points in this file", 0,MB_ICONEXCLAMATION|MB_OK);
                    goto Exit;
                 } 
                 if (Pass && !NumLoaded && !*AutoExportName) 
                 {  
                    GSSiMsgBox(GetFocus(),"No points loaded", 0,MB_ICONEXCLAMATION|MB_OK);
                    goto Exit;
                 }
                 if (TotPoints)
                 { 
	                 sprintf (mess,"Load complete: %ld of %ld points located (%.2f%%), %ld points loaded (%.2f%%)",
	                 		TotPoints-NumBad,TotPoints,((double)(TotPoints-NumBad)*100)/TotPoints,NumLoaded,
	                 								   ((double)(NumLoaded)*100)/TotPoints);
	                 SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,mess);
                 }
SkipFirstPass:   
                 if (!Pass && ContinueProcessing)
                 {
                     short NumPens=10;
                     PENDESC PenDesc[10];
                     
                     for (i=0;i<NumPens;i++)
                     {
                        PenDesc[i].PenNum = i+1;
                        PenDesc[i].Width = (float)1.0; 
                        PenDesc[i].Style = 1;
                        PenDesc[i].Color = RGB(0,0,0);
                     }
                     if (!CreateNewMap (PltName,&EditBounds,NumSyms,hSymDesc,
                                                        NumPens,(LPPENDESC)&PenDesc,0,0,TRUE))
                                                        goto Exit;
                                                         
//                   GlobalUnlock (hSymDesc);     
                    Pass = 1;
                    Done = FALSE; 
                    lineno = 0;
                    goto NextPass;
                 }
       Exit:
			 	 Status = ContinueProcessing;
			 	 ContinueProcessing=TRUE;
		         {
		         	DPOINT Dpoint;
					AddPointToMap (Dpoint,0,0,0,0,0,0,0,0,0,0,0,0,0,TRUE,FALSE,NULL,NULL);
				 }
				 CloseMap(TRUE); 
				 CloseChronoIndex();
                 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),TRUE);
                 if (Pass)
                 {
	   				 if (FileIsDir)
	   				 { 
		                SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Inserting symbols");
				 	 	AddSymToDir (PltName,NumSyms,hSymDesc,0,NULL);
				 	 }  
				 	 else
				 	 	AddSymToMap (NumSyms,hSymDesc,0,NULL); 
				 }
                 DestroySymList (&NumSyms,&hSymDesc);
                 FileProjectionType = SaveFPT;    
                 SetGlobalValue("%ALT_PROJECTION",SaveAltProj);
				 ConvertCoordClose ();
				 ConvertCoordInit();
                 DisableHalt = FALSE; 
                 DoPaint = TRUE;
				 ForceRefIndex = ForceTAGIndex = FALSE;
				 Processing = FALSE;  
				 FileIsOpen=FALSE;
				 SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Load completed");  
				 SetGlobalValueLong ("%NUMLOADED",NumLoaded); 
				 SetGlobalValueLong ("%NUMBAD",NumBad);
				 if (*AutoExportName)
				 {
	                 CloseDataFile (TRUE, &PMhDB);  
					 DestroyAdvancedOpts ();
		             GSSiEndDialog(hWndDlg, Status,hSaveBM);
		         }  
				 else
				 {
                 	PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
				 }  
                 break;
                 
            }   
          }
          break;

    default: 
    	GSSiGlobUlFree (&hMem);
        return FALSE;
   }
 GSSiGlobUlFree (&hMem);
 return TRUE;    
} 

BOOL SetSymParms (LPSTR SymStuff,LPSTR SymName,LPSTR CSymSize,LPSTR CSymRot,LPSTR CSymColor)
{   
	LPSTR pSize, pRot, pColor;
	
    if ((pSize = _fstrchr (SymStuff,';')))
    {
		*pSize++ = 0;
	    if ((pRot = _fstrchr (pSize,';')))
	    {
			*pRot++ = 0;
		    if ((pColor = _fstrchr (pRot,';')))
		    {
				*pColor++ = 0;
			}
			else
				pColor = _fstrchr (SymStuff,0);
		}
		else
			pRot = pColor = _fstrchr (SymStuff,0);
	}
	else
		pSize = pRot = pColor = _fstrchr (SymStuff,0);
	_fstrcpy (SymName,SymStuff);	                
	_fstrcpy (CSymSize,pSize);	                
	_fstrcpy (CSymRot,pRot);	                
	_fstrcpy (CSymColor,pColor);
	return TRUE;
}	                

short StreetNameType (LPSTR STName)
{
    short l, Type;
    
    l = _fstrlen (STName);
    if (!_fstrnicmp (STName,"I-",2))
        Type = 1;
    else if (_fstrstr (STName,"US Hwy"))
        Type = 2;
    else if (_fstrstr (STName,"United States Highway"))
        Type = 2;
    else if (_fstrstr (STName,"US Rte"))
        Type = 2;
    else if (_fstrstr (STName,"State Hwy"))
        Type = 3;
    else if (_fstrstr (STName,"State Highway"))
        Type = 3;
    else if (_fstrstr (STName,"State Rte"))
        Type = 3;
    else if (_fstrstr (STName,"County Hwy"))
        Type = 4;     
    else if (_fstrstr (STName,"County Highway"))
        Type = 4;     
    else if (_fstrstr (STName,"County Rte"))
        Type = 4;
    else if (_fstrstr (STName,"Twp Hwy"))
        Type = 5;
    else if (_fstrstr (STName,"Township Hwy"))
        Type = 5;
    else if (!_fstricmp (&STName[max(0,l-4)],"Frwy"))
        Type = 6;
    else if (!_fstricmp (&STName[max(0,l-3)],"Hwy"))
        Type = 7;
    else if (!_fstricmp (&STName[max(0,l-4)],"Mtwy"))
        Type = 7;  
    else if (!_fstricmp (&STName[max(0,l-5)],"Thoro"))
        Type = 7;  
    else if (!_fstricmp (&STName[max(0,l-4)],"Thwy"))
        Type = 7;  
    else if (!_fstricmp (&STName[max(0,l-4)],"Tpke"))
        Type = 7;  
    else if (_fstrstr (STName,"State Rd"))
        Type = 8;
    else if (_fstrstr (STName,"County Rd"))
        Type = 8;
    else if (_fstrstr (STName,"Twp Rd"))
        Type = 9;
    else if (_fstrstr (STName,"Township"))
        Type = 9;
    else if (_fstrstr (STName,"State"))
        Type = 9;
    else if (_fstrstr (STName,"County"))
        Type = 9;
    else
        Type = 100;
    return Type; 
}
    

BOOL FAR PASCAL LOAD_TIGERMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LPARAM lParam)
{ 
    char    Ext[8], ExtID[34];

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:
         SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_SETCHECK,TRUE,0L);
         SendDlgItemMessage (hWndDlg,IDC_TF_STANDARD,BM_SETCHECK,TRUE,0L);    
		 SetDlgItemText (hWndDlg,IDC_CFCC_FILE,"All");         
		 Processing=FALSE;
//         cwCenter(hWndDlg, 0);
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
#if WIN32
         switch(LOWORD(wParam))
#else
         switch(wParam)
#endif
         {  
            case IDC_LOCATE_SOURCE: 
            {    
            	 
                 _fstrcpy (Ext,GetTIGERFileExtension('1')); 
                 _fstrcpy (ExtID,"TIGER File");
                 sprintf (gszFilter,"%s(*%s;filelist.txt)|*%s;filelist.txt|",ExtID,Ext,_fstrlwr(Ext));
                 if (GetFileName3(hWndDlg,LoadName,0,IDS_FILETIGER1))   
                 {
                    SetDlgItemText (hWndDlg,IDC_FILE,LoadName);
                 }
            }
                 break;
                 
            case IDC_LOCATE_DESTMAP: 
            {
            	char	str[128];
                 *str=0;
                 if (SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0)) 
                 {
                    if (!GetSaveName2 (hWndDlg,str,IDS_FILTERPLT,".PLT",IDS_FILEPLT)) break;   
                 }
                 else  
                 {
                    if (!GetFileName3 (hWndDlg,str,IDS_FILTERPLT,IDS_FILEPLT)) break;   
                 }
                 SetDlgItemText (hWndDlg,IDC_DESTMAP,str);
            }
                 break;
                 
            case IDC_FIND_CFCC: 
            {    char	File[128];
            
                 _fstrcpy (Ext,".txt"); 
                 _fstrcpy (ExtID,"TIGER CFCC list");
                 sprintf (gszFilter,"%s(*%s)|*%s|",ExtID,Ext,_fstrlwr(Ext));
                 if (GetFileName3(hWndDlg,File,0,IDS_FILECFCC))   
                 {
                    SetDlgItemText (hWndDlg,IDC_CFCC_FILE,File);
                 }
            }
                 break;
                 
            case IDC_ADD_CFCC_TO_DICT:
                 LoadCFCCTable ();
                 break;                 
            
            case IDCANCEL:
                 if (ContinueProcessing && Processing)
                    ContinueProcessing=FALSE;
                 else
                 {
                    EndDialog(hWndDlg, FALSE); 
                    ContinueProcessing = TRUE;
                 }
                 break;
                 
            case IDOK: 
            {
                 char Name[128],TrueName[34], str[256], SymName[16], Prefix[]="TLID",UDI[12];
                 HFILE  FidTIGER1, FidTIGER2, FidTIGER4,FidTIGER5, FidCFCC;
                 long   lineno=0, TotLen, CurLoc;  
                 long   NewRefno, Offset; 
                 short    st, NumShapes, ii, Stuff[64];   
                 BTVARDESC  BTVar[3];
                 TIGER1 Tiger1; 
                 TIGER4 Tiger4; 
                 TIGER5 Tiger5; 
                 TIGER1_MN   Tiger1PN; 
                 TIGER1_GEOSPAN     Tiger1GS; 
                 OFSTRUCT   OFStruct;
                 short        nPoly, lastnpoints, Pass, CoordPass,idesc, NumSyms=0,i, AttFormat=0;
                 long   nVertex;
			     long	StreetNums[4], TLID, TotLenFL, CurLocFL;
                 HANDLE hPoly, hShapes, hSymDesc=0, hCFCC;
                 HPDPOINT   lpDPoint, lpDPoints, lpShapes;
                 DPOINT LinkPoint;
                 LPSTR  lpSpace, lpExt, lpCFCC;
                 LPSHORT  lpNumCFCC; 
                 short	speed;
                 BOOL   Store, FirstPoly, WantAreas[8], OpenedSeg=FALSE;
                 MNMXCORD MinMaxCoord;  
                 HANDLE     hBTTiger1Out,hBTNames1, hBTNames2;
                 HFILE  FidTiger1Out,FidFileList=HFILE_ERROR; 
                 char   STName[48], Buff[64], CFCCName[128], LeafName[64];     
                 BOOL	HiPrecis;
                
				 HiPrecis = SendDlgItemMessage (hWndDlg,IDC_HIPRECISION,BM_GETCHECK,0,0);
                 if (!GetDlgItemText (hWndDlg,IDC_DESTMAP,PltName,128))
                 {  
                    GSSiMsgBox(GetFocus(),"No destination map", 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }
                 if (SendDlgItemMessage (hWndDlg,IDC_TF_NONE,BM_GETCHECK,0,0))
                    AttFormat = 0;
                 if (SendDlgItemMessage (hWndDlg,IDC_TF_NAMES,BM_GETCHECK,0,0))
                    AttFormat = 1;
                 if (SendDlgItemMessage (hWndDlg,IDC_TF_GEOSPAN,BM_GETCHECK,0,0))
                    AttFormat = 3;
                 if (SendDlgItemMessage (hWndDlg,IDC_TF_STANDARD,BM_GETCHECK,0,0))
                    AttFormat = 4;
                 
                switch (AttFormat)
                {
                    case 0:
                    break;  
                    case 1:
                        BTVar[0].BT_VARTYP=BT_INTEGER;
                        BTVar[0].BT_VARLEN=4;
                        BTVar[0].BT_VAROFF=0;
                        if (!(hBTTiger1Out= BT_OPEN ("tiger1.btr", 0, BT_WRITE, 0)))
                        {
                            BT_CREATE ("tiger1.btr", 4, FALSE, 1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
                            hBTTiger1Out= BT_OPEN ("tiger1.btr", 0, BT_WRITE, 0);
                            FidTiger1Out = GSSiOpenFile ("tiger1.dat",&OFStruct,OF_CREATE);
                         } 
                         else 
                            FidTiger1Out = GSSiOpenFile ("tiger1.dat",&OFStruct,OF_READWRITE);
                    
                    case 3:  
/*                    {
                        LPGWDHEADER lpGWDHead;    
                        
                        CreateSegData();  
                        hDBSegdata = OpenGWDatabase ("geospan.gwd",BT_WRITE);
                        lpGWDHead = (LPGWDHEADER)GlobalLock (hDBSegdata); 
                        hSegData = lpGWDHead->BTHandle[0]; 
                        hBTTiger1Out = hSegData;
                        SegDataFid = lpGWDHead->Fid; 
                        FidTiger1Out = SegDataFid;
                        GlobalUnlock (hDBSegdata);
                    }*/
                    break; 
                    case 4: 
                    {
                        LPGWDHEADER lpGWDHead;    
						OpenStreetSegmentTable (TRUE,&OpenedSeg); 
						
                        lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetSegments); 
                        hSegData = lpGWDHead->BTHandle[0]; 
                        hBTTiger1Out = hSegData;
                        SegDataFid = lpGWDHead->Fid; 
                        FidTiger1Out = SegDataFid;
                        GlobalUnlock (hDBStreetSegments);
                    }
                    break;
                    default:

                        BTVar[0].BT_VARTYP=BT_INTEGER;
                        BTVar[0].BT_VARLEN=4;
                        BTVar[0].BT_VAROFF=0;
                        if (!(hBTTiger1Out= BT_OPEN ("tiger1.btr", 0, BT_WRITE, 0)))
                        {
                            BT_CREATE ("tiger1.btr", 4, FALSE, 1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
                            hBTTiger1Out= BT_OPEN ("tiger1.btr", 0, BT_WRITE, 0);
                            FidTiger1Out = GSSiOpenFile ("tiger1.dat",&OFStruct,OF_CREATE);
                         } 
                         else 
                            FidTiger1Out = GSSiOpenFile ("tiger1.dat",&OFStruct,OF_READWRITE);
                 }
                 GetDlgItemText (hWndDlg,IDC_FILE,Name,sizeof(Name)); 
                 _fstrlwr (Name);
                 if (_fstrstr (Name,"filelist.txt"))
                 {
                 	FidFileList = GSSiOpenFile (Name,NULL,OF_READ);
					ShowWindow (GetDlgItem(hWndDlg,IDC_STATUS2),SW_SHOW);
					ShowWindow (GetDlgItem(hWndDlg,IDC_MESS2),SW_SHOW); 
					TotLenFL = GSSifilelength (FidFileList);
                 } 
        NextFile:
        		 if (FidFileList != HFILE_ERROR)     
        		 {
        		 	if (!fgetstring (Name,128,FidFileList))
        		 	{
        		 		GSSiClose (FidFileList);
        		 		goto Done;
        		 	} 
        		 	CurLocFL = GSSillseek (FidFileList,0,1);
                 	PctBox (GetDlgItem(hWndDlg,IDC_STATUS2), TotLenFL, CurLocFL,0);
        		 }
                 _fstrlwr (Name);
                 FidTIGER1=GSSiOpenFile (Name,&OFStruct,OF_READ);
                 if (FidTIGER1==HFILE_ERROR)  
                 {  
                    char    mess[256];
                    
                    sprintf (mess,"Unable to open file %s",Name);
                    GSSiMsgBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }
                 SetDlgItemText (hWndDlg,IDC_MESS2,Name);
                 _splitpath (OFStruct.szPathName,NULL,NULL,LeafName,NULL); 
				 SetGlobalValue("%SOURCENAME",LeafName);
                 TotLen = GSSillseek (FidTIGER1,0,2);
                 GSSillseek (FidTIGER1,0,0);
                 lpExt = _fstrrchr(Name,'.');  
                 *lpExt = 0;
                 _fstrcat (Name,GetTIGERFileExtension('2'));
                 FidTIGER2=GSSiOpenFile (Name,&OFStruct,OF_READ);
                 if (FidTIGER2==HFILE_ERROR)  
                 {  
                    char    mess[256];
                    
                    sprintf (mess,"Unable to open file %s",Name);
                    GSSiMsgBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }
                 GetDlgItemText (hWndDlg,IDC_CFCC_FILE,CFCCName,sizeof(CFCCName));
                 if (!_fstricmp (CFCCName,"All"))
                 	hCFCC = 0;
                 else
                 { 
	                 _fstrlwr (CFCCName);
	                 FidCFCC=GSSiOpenFile(CFCCName,&OFStruct,OF_READ);
	                 if (FidCFCC==HFILE_ERROR)  
	                 {  
	                    char    mess[256];
	                    
	                    sprintf (mess,"Unable to open CFCC file %s",CFCCName);
	                    GSSiMsgBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
	                    break;
	                 }
	                 
	                 hCFCC = GSSiGlobAlloc ( 435,GHND,1024);
	                 lpNumCFCC = (LPSHORT)GlobalLock (hCFCC);
	                 lpCFCC = (LPSTR)lpNumCFCC;
	                 lpCFCC+=2;
	                 while (fgetstring(str,sizeof(str)-4,FidCFCC))  
	                 {
	                    (*lpNumCFCC)++;
	                    _fstrncpy(lpCFCC,str,3);
	                    lpCFCC+=3;
	                 }
	                 GlobalUnlock (hCFCC);
	                 GSSiClose (FidCFCC);     
	             }    
                 
                 hShapes = GSSiGlobAlloc ( 436,GHND,(long)UINT_MAX*4);
                 lpShapes = (LPDPOINT)GlobalLock (hShapes);   
                 SetDlgItemText (hWndDlg,IDC_MESS1,"Creating shape file index");
                 GetTLIDShapePoints (-1,&NumShapes,lpShapes,FidTIGER2);
                 
                 lpExt = _fstrstr(Name,GetTIGERFileExtension('2'));  
                 *lpExt = 0;
                 hBTNames1=hBTNames2=0;
                 _fstrcat (Name,GetTIGERFileExtension('4')); 
                 if (!AttFormat)
                 	goto NoFile4;
                 FidTIGER4=GSSiOpenFile(Name,&OFStruct,OF_READ);
                 if (FidTIGER4==HFILE_ERROR) 
                    goto NoFile4; 
                 lpExt = _fstrstr(Name,GetTIGERFileExtension('4'));  
                 *lpExt = 0;
                 _fstrcat (Name,GetTIGERFileExtension('5'));  
                 FidTIGER5=GSSiOpenFile(Name,&OFStruct,OF_READ);
                 if (FidTIGER5==HFILE_ERROR)  
                 { 
                    GSSiClose (FidTIGER4); 
                    goto NoFile4; 
                 }
                 
                 BTVar[0].BT_VARTYP=BT_INTEGER;
                 BTVar[0].BT_VARLEN=4;
                 BTVar[0].BT_VAROFF=0;
                 BTVar[1].BT_VARTYP=BT_INTEGER;
                 BTVar[1].BT_VARLEN=2;
                 BTVar[1].BT_VAROFF=4;
                 BTVar[2].BT_VARTYP=BT_INTEGER;
                 BTVar[2].BT_VARLEN=4;
                 BTVar[2].BT_VAROFF=6;
                 BT_CREATE ("tempnam1.btr", 2, FALSE, 3, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
                 BTVar[0].BT_VARTYP=BT_INTEGER;
                 BTVar[0].BT_VARLEN=4;
                 BTVar[0].BT_VAROFF=0;
                 BT_CREATE ("tempnam2.btr", sizeof(Names2Data), FALSE, 1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
                 hBTNames1= BT_OPEN ("tempnam1.btr", 0, BT_WRITE, 0);
                 hBTNames2= BT_OPEN ("tempnam2.btr", 0, BT_WRITE, 0);
                 
                 SetDlgItemText (hWndDlg,IDC_MESS1,"Loading Alternate Street Names");
                 while (fgetstring((char *)&Tiger5,sizeof(TIGER5),FidTIGER5))
                 {
                    FeatID = ldread (Tiger5.FEAT,8);  
                        
                    sprintf (STName,"%s %s %s %s", 
                             strncpy0(Buff,Tiger5.FEDIRP,2),
                             strncpy0(&Buff[16],Tiger5.FENAME,30),
                             strncpy0(&Buff[4],Tiger5.FETYPE,4),
                             strncpy0(&Buff[10],Tiger5.FEDIRS,2)); 
                    OneSpace (STName); 
                    if (*STName)
                    {
                        REPLAC (STName,"I -","I-",sizeof(STName));
                        REPLAC (STName,"I- ","I-",sizeof(STName)); 
						Names2Data.StreetNum = AddStreetName (STName,0,Tiger1.FEDIRP,Tiger1.FENAME,
																	   Tiger1.FETYPE,Tiger1.FEDIRS);
                        Names2Data.Type = StreetNameType (STName);
                        BT_PUT (hBTNames2,(LPSTR)&FeatID,(LPSTR)&Names2Data);
                    }  
                 }
                 GSSiClose (FidTIGER5);
                 while (fgetstring((char *)&Tiger4,sizeof(TIGER4),FidTIGER4))
                 {
                    Names1Key.TLID = ldread (Tiger4.TLID,10);
                    
                    for (i=0;i<5;i++)
                    {
                        if ((FeatID = ldread (Tiger4.FEAT[i],8)))
                        { 
                            if (!(st=BT_FIND (hBTNames2,(LPSTR)&FeatID,BT_FIRST,BT_EQ,(LPSTR)&Names2Data)))
                            {
                                Names1Key.Type = Names2Data.Type;
                                Names1Key.StreetNum = Names2Data.StreetNum; 
                                PrimeName = 0;
                                BT_PUT (hBTNames1,(LPSTR)&Names1Key,(LPSTR)&PrimeName);
                            }
                        }
                        else
                            goto NextTIGER4;
                    }
    NextTIGER4:; 
                 }
                 GSSiClose (FidTIGER4); 
                 BT_CLOSE (hBTNames2); 
                 GSSiRemove ("tempnam2.btr");
    NoFile4:
                 
                   
                 GlobalUnlock (hShapes);
                 if (!OpenSymDict (OF_READ))
                 	break; 
                 if (AttFormat == 1)
                 	OpenGSStreetNames (BT_WRITE,3);
                 if (SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0)) 
                 {
                    Pass=CoordPass=0;
	                DBoundsInit (&MinMaxCoord);
                 }
                 else
                 {
				    OpenMap (CurView->hWnd,CurView->hDC);
					MinMaxCoord = CurView->FileMNMX;
					EditBounds = CurView->FileMNMX;
				    CloseMap (FALSE);
                    Pass=1;
                    CoordPass = 1;
                 }
                 SaveFPT = FileProjectionType; 
				 FileProjectionType=0;
                 DisableHalt = TRUE;  
				 GetDlgItemText (hWndDlg,IDC_DESTMAP,PltName,128);
                 ExpandText (PltName);
                 PltType = 2; 
                 Processing = TRUE;
        NextPass:

                 GSSillseek (FidTIGER1,0,0);
                 if (!Pass)
                    SetDlgItemText (hWndDlg,IDC_MESS1,"Scanning for min/max coordinates");
                 else
                    SetDlgItemText (hWndDlg,IDC_MESS1,"Loading data");
        NextLine:                
                 if (!fgetstring((char *)&Tiger1,sizeof(TIGER1),FidTIGER1) || !ContinueProcessing)
                    goto EndFile;
                 lineno++;  
                 NewRefno = ldread(Tiger1.TLID,10);
                 TLID = NewRefno; 
                 if (!_fstrncmp(Tiger1.CFCC,"   ",3))
                 	_fstrncpy (Tiger1.CFCC,"X00",3);
                 if (WantThisTLID (NewRefno,&Tiger1,WantAreas,Pass,hCFCC))
                 {  
	                lpShapes = (LPDPOINT)GlobalLock (hShapes);
	                GetTLIDShapePoints (NewRefno,&NumShapes,lpShapes,FidTIGER2); 
                    if (Pass)
                    {   
	                 	LPSTR	lpEnd;
	                 	
	                    sprintf (STName,"%s %s %s %s", 
	                             strncpy0(Buff,Tiger1.FEDIRP,2),
	                             strncpy0(&Buff[16],Tiger1.FENAME,30),
	                             strncpy0(&Buff[4],Tiger1.FETYPE,4),
	                             strncpy0(&Buff[10],Tiger1.FEDIRS,2)); 
	                    OneSpace (STName); 
	                    lpEnd = _fstrchr (STName,0);
	                    lpEnd--;
/*	 code to load PEOPLENET USA level Freeways
	                    if (*STName != 'I' || (*lpEnd != '0' && *lpEnd != '5'))
	                    	goto NextLine;*/ 
	                    if (AttFormat == 1)     
	                    	Tiger1PN.StreetNum[0] = GetGSStreetNum (STName);
	                    else
							Tiger1PN.StreetNum[0] = AddStreetName (STName,0,Tiger1.FEDIRP,Tiger1.FENAME,
																	Tiger1.FETYPE,Tiger1.FEDIRS);
				        StreetNums[0]=Tiger1PN.StreetNum[0];
				        StreetNums[1]=0;
				        StreetNums[2]=0;
				        StreetNums[3]=0;
		                Names1Key.StreetNum = 0; 
		                Names1Key.Type = 0;
		                Names1Key.TLID = NewRefno; 
		                i=1;  
		                st = BT_FIND (hBTNames1,(LPSTR)&Names1Key,BT_FIRST,BT_GE,(LPSTR)&PrimeName); 
		                while (!st && Names1Key.TLID == NewRefno && i < 4)
		                {   
		                    StreetNums[i++] = Names1Key.StreetNum;  
		                    st = BT_FIND (hBTNames1,(LPSTR)&Names1Key,BT_NEXT,BT_ANY,(LPSTR)&PrimeName);  
		                }
				        Stuff[0]=18;
				        Stuff[1]=10;
				        _fmemmove (&Stuff[2],StreetNums,16);
	                    
                        if (AttFormat)
                        {
                            Offset = GSSillseek (FidTiger1Out,0,2);
                            BT_PUT (hBTTiger1Out,(LPSTR)&NewRefno,(LPSTR)&Offset);  
                            Tiger1PN.TLID = ldread (Tiger1.TLID,10);  
                            _fmemmove (Tiger1PN.StreetNum,StreetNums,16); 
                            for (i=1;i<4;i++)
                            	Tiger1PN.StreetNum[i]=StreetNums[i];
                            Tiger1PN.ZIPL = ldread (Tiger1.ZIPL,5);
                            Tiger1PN.ZIPR = ldread (Tiger1.ZIPR,5);
                            Tiger1PN.STATEL = ldread (Tiger1.STATEL,2);
                            Tiger1PN.STATER = ldread (Tiger1.STATER,2);
                            Tiger1PN.COUNTYL = ldread (Tiger1.COUNTYL,3);
                            Tiger1PN.COUNTYR = ldread (Tiger1.COUNTYR,3);
                            Tiger1PN.FPLL = ldread (Tiger1.FPLL,5);
                            Tiger1PN.FPLR = ldread (Tiger1.FPLR,5);
                            _fmemmove (Tiger1PN.CTBNAL,Tiger1.CTBNAL,6);
                            _fmemmove (Tiger1PN.CTBNAR,Tiger1.CTBNAR,6);
                            _fmemmove (Tiger1PN.BLKL,Tiger1.BLKL,4);
                            _fmemmove (Tiger1PN.BLKR,Tiger1.BLKR,4);
                            Tiger1PN.FMCDL = ldread (Tiger1.FMCDL,5);
                            Tiger1PN.FMCDR = ldread (Tiger1.FMCDR,5);
                            Tiger1PN.FRLONG = ldread (Tiger1.FRLONG,10);
                            Tiger1PN.TOLONG = ldread (Tiger1.TOLONG,10);
                            Tiger1PN.FRLAT = ldread (Tiger1.FRLAT,9);
                            Tiger1PN.TOLAT = ldread (Tiger1.TOLAT,9); 
                            Tiger1PN.faddl = ldread (Tiger1.FRADDL,11);
                            Tiger1PN.taddl = ldread (Tiger1.TOADDL,11);
                            Tiger1PN.faddr = ldread (Tiger1.FRADDR,11);
                            Tiger1PN.taddr = ldread (Tiger1.TOADDR,11);
                            Tiger1PN.Width = 0;
							switch (Tiger1.CFCC[1])
							{
								case '1':
									speed = 60;
									break;
								case '2':
									speed = 50;
									break;
								case '3':
									speed = 45;
									break;
								default:
									speed = 30;
									break;
							}
                            
                            Tiger1PN.Speed = speed;
                            Tiger1PN.OneWay = 0;
                            _fmemcpy (Tiger1PN.CFCC,Tiger1.CFCC,3);
                            Tiger1PN.ChangedFlag = 0; 
                            switch (AttFormat)
                            {  
                                case 2:
                                    BigWrite (FidTiger1Out,(HPSTR)&Tiger1PN,sizeof(TIGER1_PEOPLENET),-1);
                                break; 
                                case 4:  
                                {   UINT	len;

								  	len = sizeof(Tiger1PN);
								   	BigWrite (SegDataFid,(HPSTR)&len,2,-1);
								   	BigWrite (SegDataFid,(HPSTR)&Tiger1PN,len,-1); 
								} 
								break;
					/*			case 3:
                                    _fmemset (&Tiger1GS,0,sizeof(Tiger1GS)); 
                                    Tiger1GS.TLID = Tiger1PN.TLID;
                                    Tiger1GS.street_num = Tiger1PN.StreetNum[0];
                                    Tiger1GS.frlong = Tiger1PN.FRLONG;
                                    Tiger1GS.frlat = Tiger1PN.FRLAT;
                                    Tiger1GS.tolong = Tiger1PN.TOLONG;
                                    Tiger1GS.tolat = Tiger1PN.TOLAT;
                                    _fmemcpy (Tiger1GS.CTBNAL,Tiger1PN.CTBNAL,6);
                                    _fmemcpy (Tiger1GS.CTBNAR,Tiger1PN.CTBNAR,6);
                                    _fmemcpy (Tiger1GS.CFCC,Tiger1.CFCC,3);
                                    Tiger1GS.BGL = ldread (Tiger1PN.BLKL,1);
                                    Tiger1GS.BGR = ldread (Tiger1PN.BLKR,1);
                                    Tiger1GS.FMCDL = ldread (Tiger1.FMCDL,5);
                                    Tiger1GS.FMCDR = ldread (Tiger1.FMCDR,5);
                                    Tiger1GS.ZIPLeft = Tiger1PN.ZIPL;
                                    Tiger1GS.ZIPRight = Tiger1PN.ZIPR;
                                    Tiger1GS.faddl = ldread (Tiger1.FRADDL,11);
                                    Tiger1GS.taddl = ldread (Tiger1.TOADDL,11);
                                    Tiger1GS.faddr = ldread (Tiger1.FRADDR,11);
                                    Tiger1GS.taddr = ldread (Tiger1.TOADDR,11);
                                    WriteSegData(Offset,&Tiger1GS);
                                break; */  
                            }  
                        }
                    }
                    Store=TRUE;  
                    hPoly = 0; 
                    nVertex = NumShapes+2;
                    hPoly = GSSiGlobAlloc ( 437,GMEM_MOVEABLE,(DWORD)nVertex*sizeof(DPOINT));
                    lpDPoint = (LPDPOINT)GlobalLock (hPoly); 
                    lpDPoints = lpDPoint;  
                    lpDPoint->x = dread(Tiger1.FRLONG,10)/1000000;
                    lpDPoint->y = dread(Tiger1.FRLAT,10)/1000000;    
                    ConvertCoord(lpDPoint,2,1);
                    if (!PointInFileBounds (lpDPoint,&MinMaxCoord,CoordPass))
                        Store=FALSE;
                    lpDPoint++;
                    while (NumShapes--)
                    { 
                        *lpDPoint = *(lpShapes++);
                        ConvertCoord(lpDPoint,2,1);
                        if (!PointInFileBounds (lpDPoint,&MinMaxCoord,CoordPass))
                            Store=FALSE;    
                        lpDPoint++;
                    } 
                    lpDPoint->x = dread(Tiger1.TOLONG,10)/1000000;
                    lpDPoint->y = dread(Tiger1.TOLAT,10)/1000000;    
                    ConvertCoord(lpDPoint,2,1);
                    if (!PointInFileBounds (lpDPoint,&MinMaxCoord,CoordPass))
                        Store=FALSE;    
                    if (!Pass)
                    	Store=FALSE;
                    _fmemmove (SymName,Tiger1.CFCC,3);
                    SymName[3]=0;
                    idesc = GetDictSymbolNumber (SymName);
                    if (!idesc)  
                    {
                        _fstrcpy (SymName,"X00");
                        idesc = GetDictSymbolNumber ("X00"); 
                    }
                    GlobalUnlock (hPoly);
                    if (Store)  
                    {    
                        short     ipen; 
                        
	                    AddToSymList (idesc,&NumSyms,&hSymDesc);
                        ipen = SymName[0]-'A' + 1;
                        ipen = min (9,ipen);
                        
                        if (AttFormat) 
                        	_ltoa (TLID,UDI,10); 
                        else
                        	*Prefix = 0;
                       
                        AddPolyToMap (1,(LPSHORT)&nVertex, &hPoly,1,NewRefno,NULL,ipen,idesc,Stuff,Prefix,UDI,-1,-1,0,0,0,0,0,HiPrecis); 
                    }
                    GlobalFree (hPoly); 
                 	GlobalUnlock (hShapes); 
                 }
                 CurLoc = GSSillseek (FidTIGER1,0,1);       
                 PctBox (GetDlgItem(hWndDlg,IDC_STATUS), TotLen, CurLoc,0);
                 
                 goto NextLine; 
                 Pass=1; 
                 {  
                    char    mess[256];
                    
                    sprintf (mess,"Error in file at line %ld",lineno);
                    GSSiMsgBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
                 }
                    
        EndFile: 
                 if (Pass>=0 && ContinueProcessing)
                 {
                     if (!Pass)
                     {   
                         short NumPens=10;
                         PENDESC PenDesc[10];
                         
                         for (i=0;i<NumPens;i++)
                         {
                            PenDesc[i].PenNum = i+1;
                            PenDesc[i].Width = 1; 
                            PenDesc[i].Style = 1;
                            PenDesc[i].Color = RGB(0,0,0);
                         }
                         CreateNewMap (PltName,&MinMaxCoord,NumSyms,hSymDesc,
                                                            NumPens,(LPPENDESC)&PenDesc,0,0,TRUE);
                         Pass=1;
		                 CoordPass = 1;
						 EditBounds = CurView->FileMNMX;
                         goto NextPass;
                     }
                 }
				 CloseMap(TRUE);
                 GSSiClose (FidTIGER1); 
                 GSSiClose (FidTIGER2);
                 BT_CLOSE (hBTNames1); 
                 GSSiRemove ("tempnam1.btr");
                 CloseGSStreetNames();
				 CloseStreetNameTable();
                 GetTLIDShapePoints (-2,&NumShapes,lpShapes,FidTIGER2);
                 GlobalFree (hShapes); 
                 GSSiGlobFree (&hCFCC);   
                 if (FidFileList != HFILE_ERROR)
                 	goto NextFile;             
           Done:
                 switch (AttFormat)
                 {
                    case 0:
                        break;
                    case 3:
                        CloseGWDatabase (hDBSegdata);
                        hDBSegdata = 0;
                        hSegData = 0;  
                        break;
                    case 4:
                    	CloseStreetSegmentTable (OpenedSeg);
                    	break;
                    default:                    
                        GSSiClose (FidTiger1Out);
                        BT_CLOSE (hBTTiger1Out);
                        break;
                 }           
			 	 AddSymToMap (NumSyms,hSymDesc,0,NULL); 
                 DestroySymList (&NumSyms,&hSymDesc);
                 CloseSymDict();
                 DisableHalt = FALSE;
                 FileProjectionType = SaveFPT;
                 Processing = FALSE;
                 EndDialog(hWndDlg, ContinueProcessing); 
                 ContinueProcessing = TRUE;  
                 break;
                 
            }   
          }
          break;

    default:
        return FALSE;
   }
 return TRUE;    
} 


BOOL FAR PASCAL LOAD_TIGER_PNMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LPARAM lParam)
{ 
    char    File[128], Ext[8], ExtID[34], str[32];
    HANDLE	hTiger1=0, hTiger4=0, hTiger5=0;

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:
         cwCenter(hWndDlg, 0);
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
#if WIN32
         switch(LOWORD(wParam))
#else
         switch(wParam)
#endif
         {  
         	case IDC_STATELEV: 
         	{
				short State, StartState;
				SaveFPT = FileProjectionType; 
				FileProjectionType=0;
				DisableHalt = TRUE;  
                GetDlgItemText (hWndDlg,IDC_DIRECTORY,str,sizeof(str)); 
                StartState = max (1,atoi (str));
//         		for (State = StartState;State<57;State++)
         		for (State = 27;State<28;State++) 
				{
//					 CreateStateLev (State,hWndDlg,IDC_STATUS,IDC_MESS2); 
				}
				DisableHalt = FALSE;
				FileProjectionType = SaveFPT; 
			}
         		break;
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */   
                 if (Processing)
                    ContinueProcessing=FALSE;
                 else
                    EndDialog(hWndDlg, FALSE);
                 break;
                 
            case IDOK: 
            {
                 char Name[132],TrueName[34], str[260], SymName[16], Dir[128], Directories[128];
                 HFILE  FidTIGER4, FidTIGER5;
                 HFILE  FidCFCC, FidTIGER1,FidTIGER2;
                 long   lineno=0, TotLen, CurLoc, TotLen2, CurLoc2;  
                 long   NewRefno, Offset; 
                 short    st, NumShapes, idummy,len;   
                 BTVARDESC  BTVar[3];
                 LPTIGER1 Tiger1; 
                 LPTIGER4 Tiger4; 
                 LPTIGER5 Tiger5; 
                 TIGER1_MN  Tiger1PN;  
                 OFSTRUCT   OFStruct;
                 short        nPoly, lastnpoints, Pass, idesc, NumSyms,i, Stuff[256], FirstType;
                 long   nVertex, StreetNum,  StreetNums[4], Tiger1Snum;
                 HANDLE hPoly, hShapes, hSymDesc=0, *phSymDesc, hCFCC, hBTNames1,hBTNames2;
                 HPDPOINT   lpDPoint, lpDPoints, lpShapes;
                 DPOINT LinkPoint, BeginPoint,EndPoint;
                 LPSTR  lpSpace, lpExt, lpCFCC, lpSlash, lpNextState, lpState;
                 LPSHORT  lpNumCFCC;
                 BOOL   Store, FirstPoly, WantAreas[8], HaveFiles;
                 MNMXCORD MinMaxCoord;  
                 HANDLE     hBTTiger1Out, hIntersect;
                 HFILE  FidTiger1Out;   
                 BOOL	HiPrecis=FALSE;
                 char   STName[48], Buff[64];
                 char   mess[256], NewName[128];
                 short  State, County, Layer, ii; 
                 DWORD	Err;
                 HFILE  FidFileList, FidStateLev, FidStateLev2;      
                 struct {
                         long   TLID,
                                SNums[4];
                         DPOINT BeginPoint,
                                EndPoint;
                         char   CFCC[3];
                         } StateLev;
                         
                  
                 hTiger1 = GSSiGlobAlloc ( 438,GMEM_MOVEABLE,sizeof(TIGER1)+4);
                 Tiger1 = (LPTIGER1)GlobalLock (hTiger1);
                 hTiger4 = GSSiGlobAlloc ( 439,GMEM_MOVEABLE,sizeof(TIGER4)+4);
                 Tiger4 = (LPTIGER4)GlobalLock (hTiger4);
                 hTiger5 = GSSiGlobAlloc ( 440,GMEM_MOVEABLE,sizeof(TIGER5)+4);
                 Tiger5 = (LPTIGER5)GlobalLock (hTiger5);
				 ContinueProcessing=TRUE;
                 SaveFPT = FileProjectionType; 
				 FileProjectionType=0;
                 DisableHalt = TRUE;
                 Processing = TRUE;  
                 GetDlgItemText (hWndDlg,IDC_DIRECTORY,Directories,sizeof(Directories)); 
                 lpNextState = Directories;
     NextState:        
     			 if (!lpNextState)
     			 	goto EndState;
     			 lpState = lpNextState; 
                 lpNextState = _fstrchr (lpState,',');
                 if (lpNextState)
                 	*lpNextState++=0; 
                 State = atoi (lpState);
                 sprintf (Dir,"[%%DL]%2.2i",State);  
                 sprintf (str,"Loading state %s",Dir);
                 SetDlgItemText (hWndDlg,IDC_MESS3,str);
                 _fstrcat (Dir,"\\");
                 _fstrcpy (str,Dir);
                 _fstrcat (str,"layer1");
                 GSSiMakeDir (str,&Err);   
                 _fstrcpy (str,Dir);
                 _fstrcat (str,"layer2");
                 GSSiMakeDir (str,&Err);   
                 SetGlobalValue("STATEDIR",Dir); 
                 SetGlobalValue("%GEOSPAN_LOC",Dir); 
                 SetGlobalValue("%DATA_LOC",Dir); 
                FidStateLev = GSSiOpenFile ("[STATEDIR]layer1\\global.ini",&OFStruct,OF_CREATE); 
                sprintf (str,"[%%STATE]=%i",State);
                fputstring (str,FidStateLev);
                GSSiClose (FidStateLev);
                FidStateLev = GSSiOpenFile ("[STATEDIR]layer2\\global.ini",&OFStruct,OF_CREATE); 
                fputstring (str,FidStateLev);
                GSSiClose (FidStateLev);  
                if (SendDlgItemMessage (hWndDlg,IDC_HIPRECISIONPN,BM_GETCHECK,0,0)) 
                	HiPrecis = TRUE;
//                 CreateStnameFilesUpdate (); 
				if (!ExistFile ("[STATEDIR]statelev"))
				{
	                FidStateLev = GSSiOpenFile ("[STATEDIR]statelev",&OFStruct,OF_CREATE);
	                GSSiClose (FidStateLev);
	                BTVar[0].BT_VARTYP=BT_INTEGER;
	                BTVar[0].BT_VARLEN=4;
	                BTVar[0].BT_VAROFF=0;
	                BT_CREATE ("[STATEDIR]tiger1.btr", 4, FALSE, 1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	                FidTiger1Out = GSSiOpenFile ("[STATEDIR]tiger1.dat",&OFStruct,OF_CREATE);
	                GSSiClose (FidTiger1Out);
	                HaveFiles = FALSE;  
	             }
	             else
	             	HaveFiles = TRUE;
                 FidFileList = GSSiOpenFile ("[STATEDIR]filelist.txt",&OFStruct,OF_READ);
                 if (FidFileList == HFILE_ERROR)  
                 {  
                    char    mess[256];
                    
                    sprintf (mess,"Unable to open filelist file: %s","[STATEDIR]filelist.txt");
                    ExpandText (mess);
                    GSSiMsgBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }  
                TotLen2 = GSSillseek (FidFileList,0,2);  
                GSSillseek (FidFileList,0,0);  
                if (HaveFiles)
                	goto NextFile;
     NextFile1:
                 if (!fgetstring (Name,128,FidFileList)|| !ContinueProcessing) 
                 {
                    GSSillseek (FidFileList,0,0); 
                    CloseGSStreetNames();
                    CreateIntersectionFile (TRUE,GetDlgItem(hWndDlg,IDC_STATUS));
					_fstrcpy (str,"[STATEDIR]tiger1.btr");
					ExpandText (str);
					GSSiRemove (str);
					_fstrcpy (str,"[STATEDIR]tiger1.dat");
					ExpandText (str);
					GSSiRemove (str);
                 	SetDlgItemText (hWndDlg,IDC_MESS1,"Reorganizing Intersection File");
   				    ReorgBTree ("[STATEDIR]intersec.btr",GetDlgItem(hWndDlg,IDC_STATUS));
                    goto NextFile; 
                 }

                 lpSlash = _fstrrchr (Name,'\\');
                 if (lpSlash)
                    _fstrcpy (str,++lpSlash);
                 else
                    _fstrcpy (str,Name);
                 State = ldread (&str[3],2); 
                 CurState = State;
                 OpenGSStreetNames (BT_WRITE,3);
                 SetDlgItemText (hWndDlg,IDC_MESS2,Name); 
                 hBTTiger1Out= BT_OPEN ("[STATEDIR]tiger1.btr", 0, BT_WRITE, 0);
                 FidTiger1Out = GSSiOpenFile ("[STATEDIR]tiger1.dat",&OFStruct,OF_READWRITE);
                 CurLoc2 = GSSillseek (FidFileList,0,1);
                 PctBox (GetDlgItem(hWndDlg,IDC_STATUS2), TotLen2, CurLoc2,0);   
                 FidTIGER1=GSSiOpenFile (Name,&OFStruct,OF_READ);
                 if (FidTIGER1 == HFILE_ERROR)  
                 {  
                    sprintf (mess,"Unable to open file %s",Name);
                    GSSiMsgBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }
                 TotLen = GSSillseek (FidTIGER1,0,2);
                 GSSillseek (FidTIGER1,0,0);  
                 SetDlgItemText (hWndDlg,IDC_MESS1,"Creating Intersection File");  
        NextLine1:
                 if (!fgetstring((LPSTR)Tiger1,sizeof(TIGER1),FidTIGER1) || !ContinueProcessing) 
                 {
                    GSSiClose (FidTIGER1);
                    GSSiClose (FidTiger1Out);
                    BT_CLOSE (hBTTiger1Out);
                    goto NextFile1;      
                 }  
                 NewRefno = ldread(Tiger1->TLID,10);  
                 if (NewRefno == 203169772)
                    ii=1;
                
                 if (Tiger1->CFCC[0] == 'A')
                 {  
                    sprintf (STName,"%s %s %s %s", 
                             strncpy0(Buff,Tiger1->FEDIRP,2),
                             strncpy0(&Buff[16],Tiger1->FENAME,30),
                             strncpy0(&Buff[4],Tiger1->FETYPE,4),
                             strncpy0(&Buff[10],Tiger1->FEDIRS,2)); 
                    OneSpace (STName); 
                    if (*STName)
                    {
                        REPLAC (STName,"I -","I-",sizeof(STName)); 
                        REPLAC (STName,"I- ","I-",sizeof(STName)); 
                        Tiger1Snum = GetGSStreetNum (STName);
                    }
                    else
                        Tiger1Snum = 0;
                    Offset = GSSillseek (FidTiger1Out,0,2);
                    BT_PUT (hBTTiger1Out,(LPSTR)&NewRefno,(LPSTR)&Offset);  
                    Tiger1PN.TLID = NewRefno;  
                    Tiger1PN.StreetNum[0] = Tiger1Snum;
                    if (State > 60)
                    {
	                    Tiger1PN.ZIPL = CanZipCharToNum (Tiger1->ZIPL);
	                    Tiger1PN.ZIPR = CanZipCharToNum (Tiger1->ZIPR);
                    }
                    else
                    {
	                    Tiger1PN.ZIPL = ldread (Tiger1->ZIPL,5);
	                    Tiger1PN.ZIPR = ldread (Tiger1->ZIPR,5);
	                }
                    Tiger1PN.STATEL = ldread (Tiger1->STATEL,2);
                    Tiger1PN.STATER = ldread (Tiger1->STATER,2);
                    Tiger1PN.COUNTYL = ldread (Tiger1->COUNTYL,3);
                    Tiger1PN.COUNTYR = ldread (Tiger1->COUNTYR,3);
                    Tiger1PN.FPLL = ldread (Tiger1->FPLL,5);
                    Tiger1PN.FPLR = ldread (Tiger1->FPLR,5);
                    _fmemmove (Tiger1PN.CTBNAL,Tiger1->CTBNAL,6);
                    _fmemmove (Tiger1PN.CTBNAR,Tiger1->CTBNAR,6);
                    Tiger1PN.FRLONG = ldread (Tiger1->FRLONG,10);
                    Tiger1PN.TOLONG = ldread (Tiger1->TOLONG,10);
                    Tiger1PN.FRLAT = ldread (Tiger1->FRLAT,9);
                    Tiger1PN.TOLAT = ldread (Tiger1->TOLAT,9);
                    _fmemcpy (Tiger1PN.CFCC,Tiger1->CFCC,3);
                    Tiger1PN.FMCDL = ldread (Tiger1->FMCDL,5);
                    Tiger1PN.FMCDR = ldread (Tiger1->FMCDR,5);
                    Tiger1PN.faddl = ldread (Tiger1->FRADDL,11);
                    Tiger1PN.taddl = ldread (Tiger1->TOADDL,11);
                    Tiger1PN.faddr = ldread (Tiger1->FRADDR,11);
                    Tiger1PN.taddr = ldread (Tiger1->TOADDR,11); 
					len = sizeof(TIGER1_MN);
   					BigWrite (FidTiger1Out,(HPSTR)&len,2,-1);
                    BigWrite (FidTiger1Out,(HPSTR)&Tiger1PN,sizeof(Tiger1PN),-1);   
                }
                CurLoc = GSSillseek (FidTIGER1,0,1);       
                PctBox (GetDlgItem(hWndDlg,IDC_STATUS), TotLen, CurLoc,0);
                goto NextLine1;
                  
        NextFile:
                 if (!fgetstring (Name,128,FidFileList) || !ContinueProcessing) 
                    goto Exit;  
                 _fstrlwr (Name);
                 SetDlgItemText (hWndDlg,IDC_MESS2,Name); 
                 CurLoc2 = GSSillseek (FidFileList,0,1);
                 PctBox (GetDlgItem(hWndDlg,IDC_STATUS2), TotLen2, CurLoc2,0);   
                 lpSlash = _fstrrchr (Name,'\\');
                 if (lpSlash)
                    _fstrcpy (str,++lpSlash);
                 else
                    _fstrcpy (str,Name);
                 State = ldread (&str[3],2); 
                 CurState = State;
                 County = ldread (&str[5],3);
                 sprintf (NewName,"%slayer1\\tg%2.2i%3.3i1.plt",Dir,State,County);  
                 if (ExistFile (NewName))
                 	goto NextFile;
//               hBTTiger1Out= BT_OPEN ("[STATE_DIR]tiger1.btr", 0, BT_READ, 0);
//               FidTiger1Out = GSSiOpenFile ("[STATE_DIR]tiger1.dat",&OFStruct,OF_READ);
                 FidStateLev = GSSiOpenFile ("[STATEDIR]statetmp",&OFStruct,OF_CREATE);
                 GSSillseek (FidStateLev,0,2);
                 hIntersect = BT_OPEN ("[STATEDIR]intersec.btr", 0, BT_READ, 0);
                 _fstrlwr (Name);
                 FidTIGER1=GSSiOpenFile (Name,&OFStruct,OF_READ);
                 if (FidTIGER1 == HFILE_ERROR)  
                 {  
                    
                    sprintf (mess,"Unable to open file %s",Name);
                    GSSiMsgBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }
                 TotLen = GSSillseek (FidTIGER1,0,2);
                 lpExt = _fstrstr(Name,GetTIGERFileExtension('1'));  
                 *lpExt = 0;
                 _fstrcat (Name,GetTIGERFileExtension('2'));
                 FidTIGER2=GSSiOpenFile(Name,&OFStruct,OF_READ);
                 if (FidTIGER2 == HFILE_ERROR)  
                 {  
                        
                    sprintf (mess,"Unable to open file %s",Name);
                    GSSiMsgBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }
                 
                 hShapes = GSSiGlobAlloc ( 441,GHND,(long)UINT_MAX*4);
                 lpShapes = (LPDPOINT)GlobalLock (hShapes);   
                 SetDlgItemText (hWndDlg,IDC_MESS1,"Creating shape file index");
                 GetTLIDShapePoints (-1,&NumShapes,lpShapes,FidTIGER2);  
                 GlobalUnlock (hShapes);  
                 BTVar[0].BT_VARTYP=BT_INTEGER;
                 BTVar[0].BT_VARLEN=4;
                 BTVar[0].BT_VAROFF=0;
                 BTVar[1].BT_VARTYP=BT_INTEGER;
                 BTVar[1].BT_VARLEN=2;
                 BTVar[1].BT_VAROFF=4;
                 BTVar[2].BT_VARTYP=BT_INTEGER;
                 BTVar[2].BT_VARLEN=4;
                 BTVar[2].BT_VAROFF=6;
                 BT_CREATE ("tempnam1.btr", 2, FALSE, 3, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
                 BTVar[0].BT_VARTYP=BT_INTEGER;
                 BTVar[0].BT_VARLEN=4;
                 BTVar[0].BT_VAROFF=0;
                 BT_CREATE ("tempnam2.btr", sizeof(Names2Data), FALSE, 1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
                 hBTNames1= BT_OPEN ("tempnam1.btr", 0, BT_WRITE, 0);
                 OpenGSStreetNames (BT_WRITE,3);
                 
                 lpExt = _fstrstr(Name,GetTIGERFileExtension('2'));  
                 *lpExt = 0;
                 _fstrcat (Name,GetTIGERFileExtension('4'));
                 FidTIGER4=GSSiOpenFile (Name,&OFStruct,OF_READ);
                 if (FidTIGER4 == HFILE_ERROR) 
                    goto NoFile4; 
                 lpExt = _fstrstr(Name,GetTIGERFileExtension('4'));  
                 *lpExt = 0;
                 _fstrcat (Name,GetTIGERFileExtension('5'));
                 FidTIGER5=GSSiOpenFile (Name,&OFStruct,OF_READ);
                 if (FidTIGER5 ==  HFILE_ERROR)  
                 { 
                    GSSiClose (FidTIGER4); 
                    goto NoFile4; 
                 }
                 
                 hBTNames2= BT_OPEN ("tempnam2.btr", 0, BT_WRITE, 0);
                 
                 SetDlgItemText (hWndDlg,IDC_MESS1,"Loading Alternate Street Names");
                 while (fgetstring((LPSTR)Tiger5,sizeof(TIGER5),FidTIGER5))
                 {
                    FeatID = ldread (Tiger5->FEAT,8);  
                        
                    sprintf (STName,"%s %s %s %s", 
                             strncpy0(Buff,Tiger5->FEDIRP,2),
                             strncpy0(&Buff[16],Tiger5->FENAME,30),
                             strncpy0(&Buff[4],Tiger5->FETYPE,4),
                             strncpy0(&Buff[10],Tiger5->FEDIRS,2)); 
                    OneSpace (STName); 
                    if (*STName)
                    {
                        REPLAC (STName,"I -","I-",sizeof(STName));
                        REPLAC (STName,"I- ","I-",sizeof(STName)); 
                        Names2Data.StreetNum = GetGSStreetNum (STName); 
                        Names2Data.Type = StreetNameType (STName);
                        BT_PUT (hBTNames2,(LPSTR)&FeatID,(LPSTR)&Names2Data);
                    }  
                 }
                 GSSiClose (FidTIGER5);
                 while (fgetstring((LPSTR)Tiger4,sizeof(TIGER4),FidTIGER4))
                 {
                    Names1Key.TLID = ldread (Tiger4->TLID,10);
                    if (Names1Key.TLID == 37920461)
                    	ii=1;

                    for (i=0;i<5;i++)
                    {
                        if ((FeatID = ldread (Tiger4->FEAT[i],8)))
                        { 
                            if (!(st=BT_FIND (hBTNames2,(LPSTR)&FeatID,BT_FIRST,BT_EQ,(LPSTR)&Names2Data)))
                            {
                                Names1Key.Type = Names2Data.Type;
                                Names1Key.StreetNum = Names2Data.StreetNum; 
                                PrimeName = 0;
                                BT_PUT (hBTNames1,(LPSTR)&Names1Key,(LPSTR)&PrimeName);
                            }
                        }
                        else
                            goto NextTIGER4;
                    }
    NextTIGER4:; 
                 }
                 GSSiClose (FidTIGER4); 
                 BT_CLOSE (hBTNames2); 
    NoFile4:
                 OpenSymDict (OF_READWRITE);     
                 Pass=0;
                 PltType = 2; 
                 DBoundsInit (&MinMaxCoord);
                 Layer = 0;
                 
        NextPass:

                 GSSillseek (FidTIGER1,0,0);
                 if (!Pass)
                    SetDlgItemText (hWndDlg,IDC_MESS1,"Scanning for min/max coordinates");
                 else  
                 {  
                    char    CFCCFile[128]; 
                    char    Mess[128];
                    
	                BT_CLOSE (hBTNames1);
	                CloseGSStreetNames();
	                hBTNames1= BT_OPEN ("tempnam1.btr", 0, BT_READ, 0);
	                OpenGSStreetNames (BT_READ,3);
                    Layer++;  
				    OpenMap (CurView->hWnd,CurView->hDC);
					EditBounds = CurView->FileMNMX;
					CloseMap (FALSE);
                    if (Layer > 2)
                        goto EndLayers;  
                    sprintf (NewName,"%slayer%i\\tg%2.2i%3.3i%i.plt",Dir,Layer,State,County,Layer);
                    sprintf (CFCCFile,"layer%i.txt",Layer);
                    FidCFCC=GSSiOpenFile(CFCCFile,&OFStruct,OF_READ);
                    if (FidCFCC == HFILE_ERROR)  
                    {  
                        char    mess[256];
                                                
                        sprintf (mess,"Unable to open CFCC file %s",CFCCFile);
                        GSSiMsgBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
                        break;
                    }
                                         
                    hCFCC = GSSiGlobAlloc ( 442,GHND,1024);
                    lpNumCFCC = (LPSHORT)GlobalLock (hCFCC);
                    lpCFCC = (LPSTR)lpNumCFCC;
                    lpCFCC+=2;
                    while (fgetstring(str,256,FidCFCC))  
                    {
                        (*lpNumCFCC)++;
                        _fstrncpy(lpCFCC,str,3);
                        lpCFCC+=3;
                    }
                    GlobalUnlock (hCFCC);
                    GSSiClose (FidCFCC);    

                    sprintf (PltName,"%slayer%i\\tg%2.2i%3.3i%i.plt",Dir,Layer,State,County,Layer);
                    
                    sprintf (Mess,"Loading layer %i",Layer);
                    SetDlgItemText (hWndDlg,IDC_MESS1,Mess); 
                }
        NextLine:                
                 if (!fgetstring((LPSTR)Tiger1,sizeof(TIGER1),FidTIGER1)|| !ContinueProcessing)
                    goto EndFile;   
                 NewRefno = ldread(Tiger1->TLID,10);  
                 if (NewRefno == 37920461)
                    ii=1;
                 if (!Pass)
                 {
                    sprintf (STName,"%s %s %s %s", 
                             strncpy0(Buff,Tiger1->FEDIRP,2),
                             strncpy0(&Buff[16],Tiger1->FENAME,30),
                             strncpy0(&Buff[4],Tiger1->FETYPE,4),
                             strncpy0(&Buff[10],Tiger1->FEDIRS,2)); 
                    OneSpace (STName); 
                    if (*STName)
                    {
                        REPLAC (STName,"I -","I-",sizeof(STName)); 
                        REPLAC (STName,"I- ","I-",sizeof(STName)); 
                        Tiger1Snum = GetGSStreetNum (STName);
                        Names1Key.StreetNum = Tiger1Snum; 
                        Names1Key.Type = StreetNameType (STName);
                        Names1Key.TLID = NewRefno;   
                        PrimeName = 1;
                        BT_PUT (hBTNames1,(LPSTR)&Names1Key,(LPSTR)&PrimeName); 
                    }
                    else
                        Tiger1Snum = 0;
                 }
                
                _fmemset (StreetNums,0,16); 
                Names1Key.StreetNum = 0; 
                Names1Key.Type = 0;
                Names1Key.TLID = NewRefno; 
                i=0;  
                FirstType = 100;  
                Tiger1Snum = 0;
                st = BT_FIND (hBTNames1,(LPSTR)&Names1Key,BT_FIRST,BT_GE,(LPSTR)&PrimeName); 
                if (Pass)
                while (!st && Names1Key.TLID == NewRefno && i < 4)
                {   
                    if (!i)
                        FirstType = Names1Key.Type; 
                    if (PrimeName)
                    {
                        Tiger1Snum = Names1Key.StreetNum;  
                        Names1Key.StreetNum*=-1;
                    }
                    StreetNums[i++] = Names1Key.StreetNum;  
                    st = BT_FIND (hBTNames1,(LPSTR)&Names1Key,BT_NEXT,BT_ANY,(LPSTR)&PrimeName);  
                }
                if (Tiger1->CFCC[0] == 'A')
                {  
                    short Acode;
                                                
                    Acode = ldread (&Tiger1->CFCC[1],2);
                    if (Acode >= 40)
                    {
                        if (FirstType < 100)
                        {
                            Tiger1->CFCC[1] = '9'; 
                            Tiger1->CFCC[2] = '0' + FirstType;
                        }
                    }
                } 
                 lineno++;   
                 if (WantThisTLID (NewRefno,Tiger1,WantAreas,Pass,hCFCC))
                 {  
                    Store=TRUE;  
                    lpShapes = (LPDPOINT)GlobalLock (hShapes);
                    GetTLIDShapePoints (NewRefno,&NumShapes,lpShapes,FidTIGER2);
                    hPoly = 0; 
                    nVertex = NumShapes+2;
                    hPoly = GSSiGlobAlloc ( 443,GMEM_MOVEABLE,(DWORD)nVertex*sizeof(DPOINT));
                    lpDPoint = (LPDPOINT)GlobalLock (hPoly); 
                    lpDPoints = lpDPoint;  
                    lpDPoint->x = dread(Tiger1->FRLONG,10)/1000000;
                    lpDPoint->y = dread(Tiger1->FRLAT,10)/1000000; 
                    StateLev.BeginPoint = *lpDPoint;   
                    ConvertCoord(lpDPoint,2,1);
                    if (!PointInFileBounds (lpDPoint,&MinMaxCoord,Pass))
                        Store=FALSE;
                    lpDPoint++;
                    while (NumShapes--)
                    { 
                        *lpDPoint = *(lpShapes++);
                        ConvertCoord(lpDPoint,2,1);
                        if (!PointInFileBounds (lpDPoint,&MinMaxCoord,Pass))
                            Store=FALSE;    
                        lpDPoint++;
                    } 
                    lpDPoint->x = dread(Tiger1->TOLONG,10)/1000000;
                    lpDPoint->y = dread(Tiger1->TOLAT,10)/1000000;   
                    StateLev.EndPoint = *lpDPoint; 
                    ConvertCoord(lpDPoint,2,1);
                    if (!PointInFileBounds (lpDPoint,&MinMaxCoord,Pass))
                        Store=FALSE;    
                    
                    _fmemmove (SymName,Tiger1->CFCC,3);
                    SymName[3]=0;
                    idesc = GetDictSymbolNumber (SymName);
                    if (!idesc)  
                    {
                        _fstrcpy (SymName,"X00");
                        idesc = GetDictSymbolNumber ("X00");
                        AppendFile ("symerr.txt",SymName); 
                    }
                    AddToSymList (idesc,&NumSyms,&hSymDesc);
                    GlobalUnlock (hPoly);
                    if (Pass && Tiger1->CFCC[0] == 'A' &&
                        (Tiger1->CFCC[1] < '4' || Tiger1->CFCC[1] == '9'))  
                    {
                        StateLev.TLID = NewRefno;
                        _fmemmove (StateLev.CFCC,Tiger1->CFCC,3);
                        _fmemmove (StateLev.SNums,StreetNums,16);
                        BigWrite (FidStateLev,(char *)&StateLev,sizeof(StateLev),-1);
                    }
                    if (Store)  
                    {    
                        short     ipen, AddressDataLoc; 
                        LPSHORT   pStuff;
                        
                        ipen = SymName[0]-'A' + 1;
                        ipen = min (9,ipen);
                        
                        if (Tiger1->CFCC[0] == 'A')
                        {   
                            long    Long, Lat, NextSnum; 
                            float   X4, Y4; 
                            DPOINT  DP;
                            
                            pStuff = Stuff; 
                            Stuff[1]=10;
                            _fmemmove (&Stuff[2],StreetNums,16);
                            Stuff[10]=31;
                            Long = ldread (Tiger1->FRLONG,10);
                            Lat  = ldread (Tiger1->FRLAT,9);
                            GetNextStreet (NewRefno,Tiger1Snum,hIntersect,&Long, &Lat, &NextSnum);
                            _fmemmove (&Stuff[11],&NextSnum,4); 
                            if (NextSnum)
                            {  
                                DP.x = (double)Long/1000000;
                                DP.y = (double)Lat/1000000;    
                                ConvertCoord(&DP,2,1);
                                X4 = DP.x;
                                Y4 = DP.y;
                                _fmemmove (&Stuff[13],&X4,4);
                                _fmemmove (&Stuff[15],&Y4,4);
                            }
                            Long = ldread (Tiger1->TOLONG,10);
                            Lat  = ldread (Tiger1->TOLAT,9);
                            GetNextStreet (NewRefno,Tiger1Snum,hIntersect,&Long, &Lat, &NextSnum);
                            _fmemmove (&Stuff[17],&NextSnum,4); 
                            if (NextSnum)
                            {  
                                DP.x = (double)Long/1000000;
                                DP.y = (double)Lat/1000000;    
                                ConvertCoord(&DP,2,1);
                                X4 = DP.x;
                                Y4 = DP.y;
                                _fmemmove (&Stuff[19],&X4,4);
                                _fmemmove (&Stuff[21],&Y4,4);
                            }
                            Stuff[23]=30; 
                            Stuff[24]=GetIntersectStuff(NewRefno,hIntersect,Tiger1,&Stuff[25]); 
                            AddressDataLoc = 25 + Stuff[24];
                            Stuff[AddressDataLoc] = 32;
                            Stuff[AddressDataLoc+1] = AddPNExtraData (Tiger1,&Stuff[AddressDataLoc+2]);
                            Stuff[0] =2+24 + 2 + 16 + 2 + 2 + Stuff[24]*2 + 2 + 2 + Stuff[AddressDataLoc+1]*2;     
                        } 
                        else
                            pStuff = 0;
                        AddPolyToMap (1,(LPSHORT)&nVertex, &hPoly,1,NewRefno,NULL,ipen,idesc,pStuff,0,0,-1,-1,0,0,0,0,0,HiPrecis); 
                    }
                    GlobalFree (hPoly); 
                    GlobalUnlock (hShapes); 
                 }
                 CurLoc = GSSillseek (FidTIGER1,0,1);       
                 PctBox (GetDlgItem(hWndDlg,IDC_STATUS), TotLen, CurLoc,0);
                 
                 goto NextLine;
                 
        ErrorEnd: 
                 
                 Pass=1; 
                 {  
                    char    mess[256];
                    
                    sprintf (mess,"Error in file at line %ld",lineno);
                    GSSiMsgBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
                 }
                    
        EndFile: 
                 if (ContinueProcessing && Pass>=0)
                 {
                     if (!Pass)
                     {   
                         short NumPens=10;
                         PENDESC PenDesc[10];
                         
                         for (i=0;i<NumPens;i++)
                         {
                            PenDesc[i].PenNum = i+1;
                            PenDesc[i].Width = (float)1.0; 
                            PenDesc[i].Style = 1;
                            PenDesc[i].Color = RGB(0,0,0);
                         }
                         for (i=1;i<3;i++)
                         {
                            sprintf (NewName,"%slayer%i\\tg%2.2i%3.3i%i.plt",Dir,i,State,County,i);
                            CreateNewMap (NewName,&MinMaxCoord,NumSyms,hSymDesc,
                                                                NumPens,(LPPENDESC)&PenDesc,0,0,TRUE);
                         }
                         Pass=1;
                         goto NextPass;
                     }
                 }
                 GSSiGlobFree (&hCFCC);  
                 CloseMap(TRUE); 
                 if (ContinueProcessing)
                 	goto NextPass; 
        EndLayers: 
                 DestroySymList (&NumSyms,&hSymDesc);
                 GSSiClose (FidStateLev); 
                 FidStateLev = GSSiOpenFile ("[STATEDIR]statelev",&OFStruct,OF_READWRITE);
                 GSSillseek (FidStateLev,0,2);
                 FidStateLev2 = GSSiOpenFile ("[STATEDIR]statetmp",&OFStruct,OF_READ);
                 while (ContinueProcessing && (BigRead (FidStateLev2,(char *)&StateLev,sizeof(StateLev))==sizeof(StateLev) ))
                        BigWrite (FidStateLev,(char *)&StateLev,sizeof(StateLev),-1);
                 GSSiClose (FidStateLev);
                 GSSiClose (FidStateLev2);
                 GSSiRemove ("[STATEDIR]statetmp");
//               GSSiClose (FidTiger1Out);
//               BT_CLOSE (hBTTiger1Out);
				 CloseMap(TRUE);
                 BT_CLOSE (hBTNames1);
                 CloseGSStreetNames();
                 GetTLIDShapePoints (-2,&NumShapes,lpShapes,FidTIGER2);
                 GSSiClose (FidTIGER1); 
                 GSSiClose (FidTIGER2);
                 CloseSymDict();    
                 GlobalFree (hShapes);              
                 if (hIntersect)
                    BT_CLOSE (hIntersect); 
                 if (ContinueProcessing)
                 	goto NextFile;
            Exit: 
                 GSSiClose (FidFileList);  
				 _fstrcpy (str,"[STATEDIR]intersec.btr"); 
				 ExpandText (str);
				 if (ContinueProcessing)
				 	GSSiRemove (str); 
				 if (ContinueProcessing)
                 	goto NextState;
        EndState:
                 EndDialog(hWndDlg, TRUE); 
                 Processing = FALSE;
                 ContinueProcessing = TRUE;
                 DisableHalt = FALSE;
                 FileProjectionType = SaveFPT; 
                 break;
                 
            }   
          }
          break;

    default:  
    	GSSiGlobUlFree (&hTiger1);
    	GSSiGlobUlFree (&hTiger4);
    	GSSiGlobUlFree (&hTiger5);
        return FALSE;
   }
 GSSiGlobUlFree (&hTiger1);
 GSSiGlobUlFree (&hTiger4);
 GSSiGlobUlFree (&hTiger5);
 return TRUE;    
} 

long GetGSStreetNum (LPSTR Name)
{   
    char    StrName[34], LastName[34];  
    long    StrNum;
    
    if (!hNames2)
   		OpenGSStreetNames (BT_WRITE,3);

    _fstrncpy (StrName,Name,34);
    if (!BT_FIND (hNames2,StrName,BT_FIRST,BT_EQ,(LPSTR)&StrNum))
        return StrNum;
    if (!BT_FIND (hNames1,(LPSTR)&StrNum,BT_LAST,BT_ANY,(LPSTR)&LastName))
        StrNum++;
    else
        StrNum=1;
    BT_PUT (hNames1,(LPSTR)&StrNum,(LPSTR)StrName);
    BT_PUT (hNames2,(LPSTR)StrName,(LPSTR)&StrNum);
    return StrNum;
} 
BOOL PointInFileBounds (LPDPOINT lpDPoint,LPMNMXCORD MinMaxCoord, short Pass)
{
	SetViewport(*pCommandViewport);
    if (!Pass)
    {   
        MinMaxCoord->xmn = min (MinMaxCoord->xmn,lpDPoint->x);
        MinMaxCoord->xmx = max (MinMaxCoord->xmx,lpDPoint->x);
        MinMaxCoord->ymn = min (MinMaxCoord->ymn,lpDPoint->y);
        MinMaxCoord->ymx = max (MinMaxCoord->ymx,lpDPoint->y);
        return FALSE;
    } 
    if (Pass < 0) return FALSE;
    if (lpDPoint->x < CurView->FileMNMX.xmn ||   
        lpDPoint->x > CurView->FileMNMX.xmx ||
        lpDPoint->y < CurView->FileMNMX.ymn ||
        lpDPoint->y > CurView->FileMNMX.ymx)
        return FALSE;    
    else
        return TRUE;
}

void GetTLIDShapePoints (long TLID,LPSHORT NumShapes,HPDPOINT lpShapes,HFILE FidTIGER2)
{   
    static  short     CurNumPoints;
    static	LPTIGER2  Tiger2;  
    static	HANDLE	hTiger2=0;
    short     i, WantEvery=1;
    BTVARDESC   BTVar[2];  
    static  char    TigerShapeIndex[144]=""; 
    static  HANDLE  hIndex;
    long    Offset, CurTLID;
    struct {long    TLID;
            short     seq;} Index;
    
    *NumShapes=0;
    if (TLID == -1)
    {
        hTiger2 = GSSiGlobAlloc ( 444,GMEM_MOVEABLE,sizeof(TIGER2)+4);
        Tiger2 = (LPTIGER2)GlobalLock (hTiger2);
        if (!TigerShapeIndex[0])
            GSSiGetTempFileName (0,"gmt",0,(LPSTR)TigerShapeIndex);
        BTVar[0].BT_VARTYP=BT_INTEGER;
        BTVar[0].BT_VARLEN=4;
        BTVar[0].BT_VAROFF=0;
        BTVar[1].BT_VARTYP=BT_INTEGER;
        BTVar[1].BT_VARLEN=2;
        BTVar[1].BT_VAROFF=4;
        BT_CREATE (TigerShapeIndex, 4, FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
        hIndex= BT_OPEN (TigerShapeIndex, 0, BT_WRITE, 0); 
        Offset = GSSillseek(FidTIGER2,0,1);
        while (fgetstring((LPSTR)Tiger2,sizeof(TIGER2),FidTIGER2))  
        {
            Index.TLID = ldread(Tiger2->TLID,10);
            Index.seq = ldread(Tiger2->RTSQ,3); 
            BT_PUT (hIndex,(LPSTR)&Index,(LPSTR)&Offset);  
            Offset = GSSillseek(FidTIGER2,0,1);
        }
        BT_CLOSE (hIndex);
        hIndex= BT_OPEN (TigerShapeIndex, 0, BT_READ, 0); 
        return;
    }
    if (TLID == -2)
    {   
    	GSSiGlobUlFree (&hTiger2);
        BT_CLOSE (hIndex);
        GSSiRemove (TigerShapeIndex);
        return;
    } 
    Index.TLID = TLID;
    Index.seq = 1;
    
    if (!BT_FIND (hIndex,(LPSTR)&Index,BT_FIRST,BT_EQ,(LPSTR)&Offset)) 
    {
        CurTLID = Index.TLID;      
        GSSillseek (FidTIGER2,Offset,0); 
        fgetstring((LPSTR)Tiger2,sizeof(TIGER2),FidTIGER2);       
    }
    else
        CurTLID = LONG_MAX;
    while (CurTLID == TLID)  
    {   
        for (i=0;i<10;i++)
        {
            lpShapes->x = dread((LPSTR)&Tiger2->LONGLAT[i].Longitude,10)/1000000;
            lpShapes->y = dread((LPSTR)&Tiger2->LONGLAT[i].Latitude,9)/1000000;
            if (lpShapes->x != 0 && ((i+1)%WantEvery) == 0)
            {
                (*NumShapes)++;
                lpShapes++;
            }
        }
        if (fgetstring((LPSTR)Tiger2,sizeof(TIGER2),FidTIGER2))
            CurTLID = ldread(Tiger2->TLID,10); 
        else
            CurTLID = LONG_MAX;
    }
    return;
}

BOOL WantThisTLID (long TLID,LPTIGER1 Tiger1,LPSHORT WantAreas, short Pass,HANDLE hCFCC)
{   
    LPSHORT   pNumCFCC;
    LPSTR   lpCFCC; 
    short     i;
    
    if (!Pass) return TRUE; 
    if (Pass<0)
    {
        if (Tiger1->CFCC[0]=='A')
            return TRUE;
        else
            return FALSE;
    }
    if (!hCFCC)
    	return TRUE;
    pNumCFCC = (LPSHORT)GlobalLock (hCFCC);
    lpCFCC = (LPSTR)pNumCFCC;
    lpCFCC+=2;
    for (i=0;i<*pNumCFCC;i++,lpCFCC+=3)
    {
        if (!_fstrncmp(Tiger1->CFCC,lpCFCC,3))
        {
            GlobalUnlock (hCFCC);
            return TRUE;         
        }
    }
    GlobalUnlock (hCFCC);
    return FALSE;
}

BOOL LoadCFCCTable (void)
{   HANDLE hDB;
    LPGWFLDINFO lpGWFldInfo;
    LPGWDHEADER lpGWDHead;
    HANDLE      hBT, hSQL, hSym;
    long        Offset; 
    double      rtn;
    LPVOID      lpVal; 
    char        str1[16], str2[64];
    short         st, i, len,ifield;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr;
    LPFIELDINFO lpFieldInfo; 
    HANDLE      SaveHandle;
    LPFILEPATH  FilePathPtr; 
    BOOL        More;
    short         rc;   
    LPSYMBOL    CurSymbol;
    
    hSQL = 0;
    if (!OpenDataFile ("[%DL]cfccall.txt","",BT_READ,&hSQL))
        return FALSE;      
    
    OpenSymDict (OF_READWRITE);     
		
    SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
    FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
    
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
    _fstrcpy (CurSymbol->Name,"ALL");
    _fstrcpy (CurSymbol->Desc,"Parent of All");  
    GlobalUnlock (hSym);
    SaveSymbol(hSym,0);
	DestroySymbol (hSym);

	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
    _fstrcpy (CurSymbol->Name,"TIGER");
    _fstrcpy (CurSymbol->Desc,"TIGER file data");
    CurSymbol->Type = 0;
    CurSymbol->NumElements=0;
    CurSymbol->Parent=GetDictSymbolNumber("ALL");        
    GlobalUnlock (hSym);
    SaveSymbol(hSym,0);
	DestroySymbol (hSym);
    More=TRUE;
    while (FetchDBRec (hSQL))
    {
		hSym = AllocateNewSymbol ();
		CurSymbol = (LPSYMBOL)GlobalLock (hSym);
        GetValFromOpenFiles ("CFCC",CurSymbol->Name);
        GetValFromOpenFiles ("DESC",CurSymbol->Desc);
        CurSymbol->Parent=GetDictSymbolNumber("TIGER");
        CurSymbol->Type = 2;         
	    GlobalUnlock (hSym);
	    SaveSymbol(hSym,0);
		DestroySymbol (hSym);
    }
    GlobalUnlock (SQLPtr->OFHandle);  
    GlobalUnlock (hSQL);
    CloseDataFile (TRUE, &hSQL);   
    CloseSymDict();
    return TRUE;
} 

void DisplayTIGERName (long refno)
{
    HANDLE      hBTTiger1Out;
    HFILE   FidTiger1Out;
    long    Offset;  
    OFSTRUCT    OFStruct;
    TIGER1_PEOPLENET    Tiger1PN;
    char    Name[34];  
    HMENU   NameMenu;
    POINT   position;  
    short     i; 
    char    str[128];  
    BOOL    HaveName=FALSE;
    long    SNum;
    
    NameMenu = CreatePopupMenu();  
    for (i=0;i<4;i++)
        if ((SNum=labs(CurStreetNumbers[i])))
            if (GetTrueStreetName (SNum, Name, CurState))
            {
                AppendMenu (NameMenu,MF_ENABLED|MF_STRING,0,Name); 
                HaveName = TRUE;
            }
    if (!HaveName)
        AppendMenu (NameMenu,MF_ENABLED|MF_STRING,0,"Unnamed Street"); 
    if (GetTrueStreetName (FromStreet, Name, CurState))
    {
        sprintf (str,"From: %s",Name);
        AppendMenu (NameMenu,MF_ENABLED|MF_STRING,0,str);  
    }
    if (GetTrueStreetName (ToStreet, Name, CurState))
    {
        sprintf (str,"To:   %s",Name);
        AppendMenu (NameMenu,MF_ENABLED|MF_STRING,0,str);  
    }
        
    CloseGSStreetNames();
	CloseStreetNameTable();
    GetCursorPos (&position);  
    TrackPopupMenu (NameMenu,TPM_LEFTBUTTON,position.x,position.y,0,CurView->hWnd,0);
    DestroyMenu (NameMenu);
    return;
} 

void ReadSegData (long Offset, LPSEGDATA pSegdata)
{   UINT	len;

  	GSSillseek (SegDataFid,Offset,0);
   	BigRead (SegDataFid,(HPSTR)&len,2);
   	BigRead (SegDataFid,(HPSTR)pSegdata,len);
   	return;

}

void CreateIntersectionFile (BOOL GeoMaster,HWND hWndStatus)
{
	BTVARDESC	BTVar[3];
	long		nRecs, nLoaded, TLID;
	HDC			hDC;
	int			stSeg, st2;
	short		len;
	BOOL		RtnVal=FALSE;
	long			SegMaxData=0, Offset, TotLen, Processed=0;
	time_t ltime;
	HANDLE		hStname1, hStname2, hStname;
	char		name[34], File[128], State[32], mess[128]; 
	HFILE		Tiger1FID; 
	TIGER1		Tiger1;
	TIGER1_MN	Tiger1PN;
	SEGDATA		Segdata;

	HCURSOR	hcurSave;

	if (!hWndStatus)
		hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT));
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=4;
	BTVar[2].BT_VARTYP=BT_INTEGER;
	BTVar[2].BT_VARLEN=4;
	BTVar[2].BT_VAROFF=8;   
	if (GeoMaster)	
		_fstrcpy (File,"[%DL][%STATE]\\intersec.btr");
	else
		_fstrcpy (File,"[%GEOSPAN_LOC]intersec.btr");
	BT_CREATE (File, sizeof(IntersectData), FALSE, 3, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	ltime = 0;
	hIntersect = BT_OPEN (File, ltime, BT_WRITE, 0);  
	if (GeoMaster) 
	{   
		OFSTRUCT	OFStruct;
		
		hSegData = BT_OPEN ("[%DL][%STATE]\\tiger1.btr", 0, BT_READ, 0); 
		Tiger1FID = GSSiOpenFile("[%DL][%STATE]\\tiger1.dat",&OFStruct,OF_READ);
		_fstrcpy (State,"[%STATE]");
		ExpandText (State);
		TotLen = GSSillseek (Tiger1FID,0,2);
	}
    stSeg = BT_FIND (hSegData,(LPSTR)&TLID,BT_FIRST,BT_ANY,(LPSTR)&Offset);
    while (!stSeg)
    {    
    	if (GeoMaster)
    	{
/*    		GSSillseek (Tiger1FID,Offset,0);
    		BigRead (Tiger1FID,&Tiger1,sizeof(TIGER1));
    		Segdata.frlong = ldread(Tiger1.FRLONG,10);
    		Segdata.frlat = ldread(Tiger1.FRLAT,10);
    		Segdata.tolong = ldread(Tiger1.TOLONG,10);
    		Segdata.tolat = ldread(Tiger1.TOLAT,10); 
    		Segdata.street_num = 0; 
*/
    		GSSillseek (Tiger1FID,Offset,0);
    		Processed+=BigRead (Tiger1FID,(HPSTR)&len,2); 
    		Processed+=BigRead (Tiger1FID,(HPSTR)&Tiger1PN,sizeof(Tiger1PN));
    		Segdata.frlong = Tiger1PN.FRLONG;
    		Segdata.frlat = Tiger1PN.FRLAT;
    		Segdata.tolong = Tiger1PN.TOLONG;
    		Segdata.tolat = Tiger1PN.TOLAT; 
    		Segdata.street_num = Tiger1PN.StreetNum[0]; 
            PctBox (hWndStatus, TotLen, Processed,0);   
    	}
    	else
   			ReadSegData (Offset,&Segdata);
   		IntersectKey.Long = Segdata.frlong; 
   		IntersectKey.Lat = Segdata.frlat; 
   		IntersectKey.TLID = TLID;  
   		IntersectData.Snum = Segdata.street_num;
   		IntersectData.OPLong = Segdata.tolong;
   		IntersectData.OPLat = Segdata.tolat;
    	BT_PUT (hIntersect,(LPSTR)&IntersectKey,(LPSTR)&IntersectData);
   		IntersectKey.Long = Segdata.tolong; 
   		IntersectKey.Lat = Segdata.tolat; 
   		IntersectKey.TLID = TLID;
   		IntersectData.Snum = Segdata.street_num;
   		IntersectData.OPLong = Segdata.frlong;
   		IntersectData.OPLat = Segdata.frlat;
    	BT_PUT (hIntersect,(LPSTR)&IntersectKey,(LPSTR)&IntersectData);
	    stSeg = BT_FIND (hSegData,(LPSTR)&TLID,BT_NEXT,BT_ANY,(LPSTR)&Offset);
	}
	BT_CLOSE (hIntersect);
	if (GeoMaster)
	{
		BT_CLOSE (hSegData); 
		hIntersect = 0;
		GSSiClose (Tiger1FID);
	}  
	else
		hIntersect = BT_OPEN (File,ltime, BT_READ, 0);
	
	if (!hWndStatus)
		GSSiSetCursor (hcurSave);
	return;
} 
