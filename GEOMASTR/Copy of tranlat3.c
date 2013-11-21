#include "graphint.h" 
#include "translat.h"
#include "extrndb.h"
#include "pnet.h"
#include "gmextern.h"

static	short	styles[2][100], stylecounts[2][100],nstyles[2]={0,0};
static long	nChronoFiles=0;
static short	MIFPattern=0;
static COLORREF	MIFForeColor=0;
static COLORREF	MIFBackColor=0;
static COLORREF	MIFPenColor=0;
static short	MIFPenWidth=0;
static short	MIFPenStyle;

double GetTextSizeInBounds (short nchar,double rot,LPMNMXCORD pBounds)
{   
	double	size,rot2=LTWOPI(rot+HALFPI); 
	MNMXCORD	TestBounds;
	DPOINT	MidPoint = MinMaxMidPointD (pBounds), Point;
	
	return (0.065);
	size = 1.01*(pBounds->ymx - pBounds->ymn);    
	do
	{   
		size *= 0.99;
		DBoundsInit (&TestBounds); 
		Point = dnewpt (MidPoint,rot,nchar*0.4*size/2);
		AddDPointToMinMax (&Point,&TestBounds);
		Point = dnewpt (Point,rot2,size/2);
		AddDPointToMinMax (&Point,&TestBounds);
		Point = dnewpt (Point,rot2,-size);
		AddDPointToMinMax (&Point,&TestBounds);
		Point = dnewpt (MidPoint,rot,-nchar*0.4*size/2);
		AddDPointToMinMax (&Point,&TestBounds); 
		Point = dnewpt (Point,rot2,size/2);
		AddDPointToMinMax (&Point,&TestBounds);
		Point = dnewpt (Point,rot2,-size);
		AddDPointToMinMax (&Point,&TestBounds);
	}
	while (!BoundsInBounds (&TestBounds,pBounds,0));
	return size;
}

void ConvertUMText (LPSTR Text,LPSHORT plTxt)
{   
	HANDLE	hNew=GSSiGlobAlloc ( 915,GHND,512);
	LPSTR	NewText=GlobalLock(hNew);
	LPSTR	pBeg, pEnd, pNew=NewText;
	short	ii;
	
	Text[*plTxt] = 0; 
	REPLAC (Text,"#","\r\n",512);  
	if (_fstrstr (Text,"\\\\"))
		ii=1;
	if (_fstrstr (Text,"<"))
		ii=1;
	if (_fstrstr (Text,"!"))
		ii=1;
	pBeg = Text; 
	while (*pBeg) 
	{   
		if (*pBeg == '[')
		{   
			if ((pEnd = _fstrchr (pBeg,']')))
			{   
				pBeg++;
				*pEnd = 0;
				sprintf (pNew,"$LN(%s)",pBeg); 
				pBeg = _fstrchr (pBeg,0)+1;
				pNew = _fstrchr (pNew,0);
				goto Next;
			}
		}			   
		*pNew++ = *pBeg++; 
Next:;
	}
	*plTxt= _fstrlen (NewText);
	_fstrcpy (Text,NewText);     
	GSSiGlobUlFree (&hNew);
	return;
} 

void AddStyle (short i,short style)
{   short	j;
    
    i--;
	for (j=0;j<nstyles[i];j++)
	{
		if (styles[i][j] == style)
		{
			stylecounts[i][j]++;
			return;
		}
	}
	stylecounts[i][nstyles[i]]=0;
	styles[i][nstyles[i]++]=style;
	return;
}

BOOL FAR PASCAL LOADXFERMsgProc(HWND hWndDlg, WORD Message, WPARAM wParam, LPARAM lParam)
{ 
    char		Ext[6]=".TL5";
    LPTAGDEF    lpTAGDef; 
    short     	i;
    char    	SHPExt[8], ExtID[34];
    char    	SymName[34], drive[4],file[34];
    char        Prefix[10];
    HFILE   FidSHP;
    LPSTR   lpDot, lpMIDstr;  
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
    HANDLE      SaveHandle;
    OFSTRUCT   OFStruct;
    BOOL        More;  
    LPSTR		pFile; 
    float		size,rot;
    short         rc; 
    BOOL		NewOpt;
    static	BOOL	FileIsOpen;   
    static	HANDLE	hAttFile=0;
	short		SaveDrive, idrive;
	HANDLE	hMem=0;
    LPSTR    	str, dir;
    LPSTR       UDI, Name;
    LPSTR    	CSize,CRot,CColor; 
    LPSTR		mess, CmdString; 
    LPSHORT		stuff;
    

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 hMem = GSSiGlobAlloc ( 916,GHND,2048+256+256);
 str = GlobalLock (hMem);
 dir = str + 256+256;
 UDI = dir + 128;
 Name = UDI + 128;
 CSize = Name + 256;
 CRot = CSize + 128;
 CColor = CRot + 128;  
 mess = CColor + 128; 
 CmdString = mess + 256;
 stuff = (LPSHORT) (CmdString + 256);
 switch(Message)
   {
    case WM_INITDIALOG:
		 SetDlgItemTextGlobal (hWndDlg,IDC_TFAC,"[%XFERTFAC]","1.0");
		 SetDlgItemTextGlobal (hWndDlg,IDC_SYMFAC,"[%XFERSFAC]","1.0");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Feet");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Meters");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Degrees");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Degrees * 1000000");
         _fstrcpy (str,"*.CVT");
         DlgDirListComboBox (hWndDlg,str,IDC_PROJECTION,0,DDL_READWRITE);   
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_SELECTSTRING,-1,(LPARAM)"Meters");
 		 SendDlgItemMessage (hWndDlg,IDC_PROJECTION,CB_SELECTSTRING,(WPARAM)-1,(LPARAM)"baseproj"); 
         break; /* End of WM_INITDIALOG                                 */

    case WM_COMMAND:
#if WIN32
         switch(LOWORD(wParam))
#else
         switch(wParam)
#endif
         {  
         	case IDC_PLC:
         	case IDC_AREAS: 
         		if (SendDlgItemMessage (hWndDlg,IDC_PLC,BM_GETCHECK,0,0))
		        	SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_SELECTSTRING,-1,(LPARAM)"Feet"); 
		        else
		        	SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_SELECTSTRING,-1,(LPARAM)"Meters"); 
         		EnableWindow (GetDlgItem (hWndDlg,IDC_FILE),TRUE);
         		EnableWindow (GetDlgItem (hWndDlg,IDC_LOCATE_SOURCE),TRUE); 
         		EnableWindow (GetDlgItem (hWndDlg,IDC_SAVE),TRUE); 
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
                 
            case IDC_LOCATE_SOURCE: 
                 
                 hAttFile = 0;   
                 _fstrcpy (SHPExt,".TXT"); 
                 _fstrcpy (ExtID,"UltiMap XFER Files");
                 sprintf (gszFilter,"%s(*%s)|*%s|",ExtID,SHPExt,_fstrlwr(SHPExt));  
                 if (GetFileName3(hWndDlg,LoadName,0,IDS_FILETXT))
                 {   
                   	SetDlgItemText (hWndDlg,IDC_FILE,LoadName);
                 } 
                 break;
                 
            case IDC_EXIT: 
             	GSSiGlobFree (&hAttFile);
                EndDialog(hWndDlg, TRUE); 
                break;
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 if (Processing)
                    ContinueProcessing=FALSE;
                 break;
            
                 
            case IDC_SAVE:
            {    
            	 short	Version=1; 
            	 HFILE	FidSave; 
            	 OFSTRUCT	OFStruct;
            	 
                 if (!GetSaveName2 (hWndDlg,Name,0,Ext,IDS_FILETL5)) break; 
                 FidSave = GSSiOpenFile (Name,&OFStruct,OF_CREATE);
                 BigWrite (FidSave,Ext,6,-1);
                 BigWrite (FidSave,(HPSTR)&Version,2,-1);   
                 NewOpt  = SendDlgItemMessage (hWndDlg,IDC_PLC,BM_GETCHECK,0,0);
                 BigWrite (FidSave,(HPSTR)&NewOpt ,2,-1);
                 GetDlgItemText (hWndDlg,IDC_FILE,LoadName,128);
                 BigWrite (FidSave,(HPSTR)LoadName,128,-1);
                 GetDlgItemText (hWndDlg,IDC_DESTMAP,PltName,128);
                 BigWrite (FidSave,(HPSTR)PltName,128,-1);
                 NewOpt = SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0);
                 BigWrite (FidSave,(HPSTR)&NewOpt,2,-1);
                 GetDlgItemText (hWndDlg,IDC_PROJECTION,curproject,lncurproject);
                 ii=BigWrite (FidSave,curproject,34,-1);
			     UnitsOpt=SendDlgItemMessage(hWndDlg,IDC_UNITS,CB_GETCURSEL,NULL,NULL); 
                 BigWrite (FidSave,(HPSTR)&UnitsOpt,2,-1);
                 GetDlgItemText (hWndDlg,IDC_TRANFILE,Name,128);
                 BigWrite (FidSave,(HPSTR)Name,128,-1);
                 GetDlgItemText (hWndDlg,IDC_TFAC,Name,128);
                 BigWrite (FidSave,(HPSTR)Name,32,-1);
                 GetDlgItemText (hWndDlg,IDC_SYMFAC,Name,128);
                 BigWrite (FidSave,(HPSTR)Name,32,-1);
                 GSSiClose (FidSave);
            } 
                    
            	 break;
            	 
           	case IDC_RECALL: 
           	{
           		 short Version;     
            	 HFILE	FidSave;
            	 OFSTRUCT	OFStruct;
           		 
           		 
				 if (!GetFileName2 (hWndDlg,Name,Ext,IDS_FILETL5))
				 	break;
                 FidSave = GSSiOpenFile (Name,&OFStruct,OF_READ);
                 BigRead (FidSave,Ext,6);
                 BigRead (FidSave,(HPSTR)&Version,2);   
                 BigRead (FidSave,(HPSTR)&NewOpt,2); 
                 SendDlgItemMessage (hWndDlg,IDC_PLC,BM_SETCHECK,NewOpt,0);
                 SendDlgItemMessage (hWndDlg,IDC_AREAS,BM_SETCHECK,!NewOpt,0);
                 BigRead (FidSave,LoadName,128);
                 SetDlgItemText (hWndDlg,IDC_FILE,LoadName);
                 BigRead (FidSave,PltName,128);
                 SetDlgItemText (hWndDlg,IDC_DESTMAP,PltName);
                 BigRead (FidSave,(HPSTR)&NewOpt,2);
                 SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_SETCHECK,NewOpt,0);
                 BigRead (FidSave,curproject,34);
		         SendDlgItemMessage (hWndDlg,IDC_PROJECTION,CB_SELECTSTRING,-1,(LPARAM)curproject);
                 BigRead (FidSave,(HPSTR)&UnitsOpt,2);
			     SendDlgItemMessage(hWndDlg,IDC_UNITS,CB_SETCURSEL,UnitsOpt,NULL); 
                 BigRead (FidSave,Name,128);
                 SetDlgItemText (hWndDlg,IDC_TRANFILE,Name);
                 BigRead (FidSave,Name,32);
                 SetDlgItemText (hWndDlg,IDC_TFAC,Name);
                 BigRead (FidSave,Name,32);
                 SetDlgItemText (hWndDlg,IDC_SYMFAC,Name);
                 GSSiClose (FidSave);
		         PostMessage(hWndDlg, WM_COMMAND, IDC_PLC, 0L);
            }
           		 break;
           		 
				 
            case IDC_UNIQUEREFNO:
            {
            	 BOOL	On=SendDlgItemMessage (hWndDlg,IDC_UNIQUEREFNO,BM_GETCHECK,0,0);
            	 	 
				 EnableWindow (GetDlgItem(hWndDlg,IDC_STARTNOHDR),On);
				 EnableWindow (GetDlgItem(hWndDlg,IDC_STARTNO),On);
				 SetDlgItemText (hWndDlg,IDC_STARTNO,0);
				 EnableWindow (GetDlgItem(hWndDlg,IDC_USEIDASREF),!On);
			}
				 break;
				 
            case IDOK: 
            {
                 long   lineno=0, attline=0, itemno=0, TotLen, CurLoc, MidLine, NewRefno, ID, IDAtt;     
                 DPOINT	CenPt, DPoints[2], BasePoint, RP;
                 short  iUDI=0, LineSym, nc;
                 BOOL   Done, Store; 
                 HANDLE hMIDstr, hBT, hSQLAttImport=0;
                 LPSTR   pPrefix, pChr;
                 double coordcvt=1, SymFac=1;       
                 char	str2[32];
                 long     ii=100, Dummy, nRecBytes, nBytes;
                 short    nTX, NumSyms=0, SymNum, cond, UnknownSym, n,Type=0,NumPoints, tfac; 
                 short		Style1,Style2,Lorc1,Lorc2, nCurvePoints, CurvePoints[1024], PointID; 
                 char	LastType;
                 HANDLE	  hSymDesc=0;      
                 BOOL	Exclusion=FALSE, HiPrecis,ERR; 
                 HANDLE	hPoly = GSSiGlobAlloc ( 917,GMEM_MOVEABLE,(long)sizeof(DPOINT)*(long)4096);
                 LPDPOINT	pDPoints;
				 HANDLE		hGRText=0, hTranFile=0; 
				 LPGRTEXT	lpGRText; 
				 LPUMTEXTTPL	pTextTPL;
				 HANDLE		hTPL=0;
				 double		AZ, TSize, PCAZ, CLEN;  
				 double		SymSize=5;//45.7
				 long		TotRecs[4]={0,0,0,0}, TxtRecs[4]={0,0,0,0}, LeaderLines[4]={0,0,0,0}; 
				 long		nAdded=0, StartRefno; 
				 BOOL		UniqueRefno;  
				 static		long	debugref=-1669837881;
         
                 UniqueRefno=SendDlgItemMessage (hWndDlg,IDC_UNIQUEREFNO,BM_GETCHECK,0,0);
				 if (UniqueRefno)
				 {
				 	if (GetDlgItemText (hWndDlg,IDC_STARTNO,str,128))
				 	{
				 		ExpandText (str);
				 		StartRefno = atol (str);
				 	}
				 	else
				 		StartRefno = GetGlobalLVal2 ("[%STARTREFNO]",1000000);
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
				 HiPrecis = TRUE;
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
                 
                 if (GetDlgItemText (hWndDlg,IDC_TRANFILE,Name,128))
                 {
                 	hTranFile = LoadTranFileWithDandT (Name);
                 	if (!hTranFile)
                	{
                    	MessageBox(GetFocus(),"Invalid transformation file", Name,MB_ICONEXCLAMATION|MB_OK);
                    	goto Exit;
                    }  
                 }
                 GetDlgItemText (hWndDlg,IDC_FILE,Name,128); 
                 FidSHP=GSSiOpenFile (Name,&OFStruct,OF_READ);
                 if (FidSHP == HFILE_ERROR)  
                 {  
                    sprintf (mess,"Unable to open file %s",Name);
                    MessageBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }  
                 GetDlgItemText (hWndDlg,IDC_TFAC,str,32);
                 tfac = atof (str) * 132;
                 GetDlgItemText (hWndDlg,IDC_SYMFAC,str,32);
                 SymFac = atof (str);
                 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),FALSE);
                 SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Loading Data");
                 TotLen = GSSillseek (FidSHP,0,2);  
                 GSSillseek (FidSHP,0,0);
                 ExpandText (PltName);
                 PltType = 2;
                 Done = FALSE;
                 MidLine=0; 
                 str[0]=0;  
                 
                 if (SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0)) 
                 {
                     short NumPens=10;
                     PENDESC PenDesc[10];
                     LPSYMBOL   pSym;
                     MNMXCORD MinMaxCoord;
                     DPOINT Points[4];
                     short    i;  
                     
                     _fstrcpy (PltName,"[%DL]maplib\\xferlim.plt");
					 if (!OpenMap (1,0))
	                 {
	                    MessageBox(GetFocus(),"Import limits not set", 0,MB_ICONQUESTION|MB_OK);
	                    break;
	                 }
					 
					 MinMaxCoord = CurView->FileMNMX; 
					 CloseMap (FALSE);  
	                 GetDlgItemText (hWndDlg,IDC_DESTMAP,PltName,128);
                     for (i=0;i<NumPens;i++)
                     {
                        PenDesc[i].PenNum = i+1;
                        PenDesc[i].Width = (float)1; 
                        PenDesc[i].Style = 1;
                        PenDesc[i].Color = RGB(0,0,0);
                     }
                     if (!CreateNewMap (PltName,&MinMaxCoord,NumSyms,hSymDesc,
                                                        NumPens,(LPPENDESC)&PenDesc,0,0,TRUE)) goto ErrorEnd;
                 }

				 OpenMap (CurView->hWnd,CurView->hDC);
				 EditBounds = CurView->FileMNMX; 
				 CloseMap (FALSE);  
                 SaveFPT = FileProjectionType; 
				 FileProjectionType=0;
                 DisableHalt = TRUE;  
                 ContinueProcessing = TRUE;   
                 Processing = TRUE;
                                        
                 NewRefno=0;  
                 nRecBytes = 0; 
        NextLine: 
                 if (!ContinueProcessing) Done = TRUE;
                 if (Done)
                    goto EndFile;                
                 if (!fgetstring (str,498,FidSHP))
                	goto EndFile;   
                 if (!_fstrcmp (str,"END"))
                 	goto EndFile;
                 lineno++; 
                 if (lineno > 2460)
                 	ii=1;
				 if (SendDlgItemMessage (hWndDlg,IDC_AREAS,BM_GETCHECK,0,0)) 
				 {
					 _fstrncpy (SymName,&str[11],8);
					 SymName[8]=0; 
					 Truncate(SymName); 
	                 SymNum = GetOrCreateSym (SymName,&NumSyms,&hSymDesc,TRUE,1);
	                 NewRefno = IDNINT (dread (&str[0],10) + ZERO$);    
	                 _fstrncpy (Prefix,&str[20],8);
	                 Prefix[8] = 0;  
	                 Truncate (Prefix);
	                 _fstrncpy (UDI,&str[29],32);
	                 UDI[32] = 0; 
	                 PointID = 0;
	                 LastType = ' ';
                	 pDPoints = (HPDPOINT)GlobalLock (hPoly); 
                	 NumPoints = 0;
                	 nCurvePoints = 0;
	                 while (fgetstring (str,498,FidSHP))
	                 {  
	                	lineno++;
	                 	switch (*str)
	                 	{   
	                 		case 'L': 
	                 		case 'C':
				                n = sscanf (&str[1],"%Flf %Flf",&pDPoints->x,&pDPoints->y);
				                if (n != 2)
				                	goto ErrorEnd; 
				                ConvertAndTranCoord (pDPoints++,hTranFile); 
				                if (*str == 'C' && LastType == 'C') 
				                {
				                	CurvePoints[nCurvePoints++] = PointID;
				                	LastType = ' ';
				                }
				                else
				                	LastType = *str;  
				                PointID++;  
				                NumPoints++;
				                break;
				        	case 'E':
	   		                	GlobalUnlock (hPoly); 
	   		                	if (NumPoints)
	   		                	{   
	   		                		if (UniqueRefno)
	   		                			NewRefno = GetNextRefno (&StartRefno,UniqueRefno,TRUE); 
	   		                		else
	   		                			AddRefToUsedRefTable (NewRefno);
		                    		AddPolyToMap (1,&NumPoints,&hPoly,0,NewRefno,NULL,-1,SymNum,0,Prefix,UDI,
		                                  -1,0,0,0,0,nCurvePoints,CurvePoints,HiPrecis); 
		                        }
		                        else
		                        	ii=1;
				        		goto DisplayPCT; 
				        	default:
		                		goto ErrorEnd;
				        }
		             }
                    goto EndFile;                
		         }
                 itemno++; 
                 if (*str != 'E')
				 {
		LineErr:  
				 	sprintf (mess,"Error at line %ld",lineno); 
				    MessageBox(GetFocus(),mess, 0,MB_ICONQUESTION|MB_OK);
				    goto ErrorEnd;
				 }
				 _fstrncpy (SymName,&str[2],8);
				 SymName[8]=0; 
				 Truncate(SymName); 
                 SymNum = GetOrCreateSym (SymName,&NumSyms,&hSymDesc,TRUE,1);
                 NewRefno = ldread (&str[10],11);   
                 if (NewRefno == debugref)
                 	ii=1;
				 nTX = ldread (&str[22],1);
				 *Prefix = 0;
				 *UDI = 0;
				 if ((pChr = _fstrchr (&str[23],':')))
				 {  
				 	*pChr++ = 0;               
				 	_fstrcpy (Prefix,&str[23]);
				 	Truncate (Prefix);
				 	if (!_fstricmp (Prefix,"REFNO"))
				 		*Prefix = 0;
				 	else
				 	{
					 	_fstrcpy (UDI,pChr);
					 	Truncate (UDI);
					}
				 }
                 switch (str[1])
                 {
                 	case 'L': 
	                 	if (!fgetstring (str,498,FidSHP))
	                		goto ErrorEnd;
	                	lineno++;
	                	if (*str != 'C')
	                		goto ErrorEnd;
	                	pDPoints = (HPDPOINT)GlobalLock (hPoly);
		                n = sscanf (&str[1],"%Flf %Flf %Flf %Flf",&pDPoints[0].x,&pDPoints[0].y,
		                										  &pDPoints[1].x,&pDPoints[1].y);
		                if (n != 4)
		                {
			                GlobalUnlock (hPoly);
		                	goto ErrorEnd; 
		                }  
		                BasePoint = pDPoints[0];  
		                ConvertAndTranCoord (&pDPoints[0],hTranFile);
		                ConvertAndTranCoord (&pDPoints[1],hTranFile);
		                Type = 1;
		                NumPoints = 2;   
		                GlobalUnlock (hPoly);
		  GetText:
                 	 	TotRecs[Type]++;
		  				if (nTX)
		  				{   
		  					TxtRecs[Type]++;
		                 	if (!fgetstring (str,498,FidSHP))
		                		goto ErrorEnd;   
		                	lineno++;
		                	if (*str != 'T')
		                		goto ErrorEnd;
		                	
		                	Style1 = ldread (&str[17],2);
		                	Style2 = ldread (&str[19],2);
		                	if (Style1 || Style2) 
		                	{   
		                		DPOINT	points[5], poc, pt;   
		                		double	clen1, clen2, D, RPAZ;
		                		short	st;
		                		
		                		LeaderLines[Type]++; 
				                hTPL = GSSiGlobAlloc ( 918,GHND,sizeof(UMTEXTTPL));
			                	pTextTPL = (LPUMTEXTTPL)GlobalLock (hTPL);
			                	pTextTPL->style1 = Style1;
			                	pTextTPL->style2 = Style2;
			                	AddStyle (1,Style1);
			                	AddStyle (2,Style2);  
			                	if (Style2 == 5)
			                		ii=1;
			                	if (Style2 == 2)
			                		ii=1;
			                	if (Style2 == 9)
			                		ii=1;
			                	pTextTPL->lorc1 = ldread (&str[21],2);
			                	pTextTPL->lorc2 = ldread (&str[23],2);
			                	points[0].x = dread (&str[25],16) * MFT;
			                	points[0].y = dread (&str[41],16) * MFT;
			                	points[1].x = dread (&str[57],16) * MFT;
			                	points[1].y = dread (&str[73],16) * MFT;
			                	points[2].x = dread (&str[89],16) * MFT;
			                	points[2].y = dread (&str[105],16) * MFT;
			                	points[3].x = dread (&str[121],16) * MFT;
			                	points[3].y = dread (&str[137],16) * MFT;
			                	points[4].x = dread (&str[153],16) * MFT;
			                	points[4].y = dread (&str[169],16) * MFT; 
			                	clen1 = dread (&str[185],16) * MFT;
			                	clen2 = dread (&str[201],16) * MFT;
			                	if (pTextTPL->lorc1 == 3)
			                	{
			                		RPAZ = getazd (&points[0],&points[1]);
			                		D = ldistp (points[0],points[1]); 
			                		RPAZ = LTWOPI (RPAZ + PCAZ);
			                		RP = dnewpt (BasePoint,RPAZ,D);  
			                		points[0].x += BasePoint.x;
			                		points[0].y += BasePoint.y; 
			                		st = PCURVE(&points[0].x,&points[0].y,&poc.x,&poc.y,&pt.x,&pt.y,&RP.x,&RP.y,&clen1);
									ConvertAndTranCoord (&points[0],hTranFile); 
									ConvertAndTranCoord (&poc,hTranFile); 
									ConvertAndTranCoord (&pt,hTranFile); 
				                	pTextTPL->points[0].x = points[0].x - pDPoints[0].x;
				                	pTextTPL->points[0].y = points[0].y - pDPoints[0].y;
				                	pTextTPL->points[1].x = poc.x - pDPoints[0].x;
				                	pTextTPL->points[1].y = poc.y - pDPoints[0].y;
				                	pTextTPL->points[2].x = pt.x - pDPoints[0].x;
				                	pTextTPL->points[2].y = pt.y - pDPoints[0].y;
			                	}
			                	else
			                	{   
			                		for (i=0;i<5;i++)
			                		{
				                		points[i].x += BasePoint.x;
				                		points[i].y += BasePoint.y;
										ConvertAndTranCoord (&points[i],hTranFile); 
					                	pTextTPL->points[i].x = points[i].x - pDPoints[0].x;
					                	pTextTPL->points[i].y = points[i].y - pDPoints[0].y;
				                	} 
			                	}	
			                	GlobalUnlock (hTPL); 
			                }
			                hGRText = GSSiGlobAlloc ( 919,GHND,sizeof(GRTEXT));
				            lpGRText = (LPGRTEXT)GlobalLock (hGRText);   
				            lpGRText->UltiMapStyle = 1;  
				            lpGRText->version = 1;    
				            lpGRText->length = sizeof(GRTEXT);   
			                lpGRText->ltext = nc = ldread (&str[1],3);
			                lpGRText->ltext += lpGRText->ltext % 2;
			                TSize = dread (&str[4],8)*tfac;  
			                sprintf (lpGRText->cHeight,"%f",CvtDist(TSize,(short)PRJ_UNITS[3],(short)PRJ_UNITS[1])); 
	                    	lpGRText->FontNum = max (0,(short)ldread (&str[12],1)-1);  
	                    	if (lpGRText->FontNum == 5)
	                    		lpGRText->italic = TRUE;
	                    	lpGRText->hJust = ldread (&str[13],3);
			                lpGRText->vJust = 1; 
	                    	lpGRText->FlipForEasyReading = ldread (&str[16],1);
			                *lpGRText->Text = 0;
		                	while (nTX--)
		                	{
			                 	if (!fgetstring (str,498,FidSHP)) 
			                 	{
			                 		GlobalUnlock (hGRText);
			                		goto ErrorEnd;         
			                	}
					        	_fstrncat(lpGRText->Text,&str[1],min(nc,79));
					        	nc -= 79;
			                	lineno++;
			                } 
			                ConvertUMText (lpGRText->Text,&lpGRText->ltext); 
			                lpGRText->ltext += lpGRText->ltext % 2;
			                GlobalUnlock (hGRText);
		  				}
                 	 	//NewRefno = GetNewRefno (PltName);  
   		                GlobalUnlock (hPoly);
                 	 	if (Type == 2)
                 	 	{    
		                	pDPoints = (HPDPOINT)GlobalLock (hPoly);
							if (DPointInBounds (&pDPoints[0],&EditBounds)) 
							{
   		                		if (UniqueRefno)
	                				NewRefno = GetNextRefno (&StartRefno,UniqueRefno,TRUE); 
	                			else
	                				AddRefToUsedRefTable (NewRefno);
		                    	AddPointToMap (*pDPoints,NewRefno,0,SymNum,SymSize*SymFac,AZ,NULL,hGRText,hTPL,Prefix,UDI,-1,-1,-1,TRUE,HiPrecis,NULL,NULL);
			                }
	   		                GlobalUnlock (hPoly);
                 	 	}
                 	 	else
                 	 	{
	                		if (UniqueRefno)
                				NewRefno = GetNextRefno (&StartRefno,UniqueRefno,TRUE); 
                			else
                				AddRefToUsedRefTable (NewRefno);
	                    	if (AddPolyToMap (1,&NumPoints, 
	                                  &hPoly,Type,NewRefno,NULL,-1,SymNum,0,Prefix,UDI,
	                                  -1,-1,-1,hGRText,hTPL,0,0,HiPrecis))
	                        	nAdded++;
	                    } 
	                    GSSiGlobUlFree (&hGRText); 
	                    GSSiGlobUlFree (&hTPL);
                 	break;
                 	
                 	case 'C':
	                 	if (!fgetstring (str,498,FidSHP))
	                		goto ErrorEnd; 
	                	lineno++;
	                	if (*str != 'C')
	                		goto ErrorEnd;
	                	pDPoints = (HPDPOINT)GlobalLock (hPoly);
		                n = sscanf (&str[1],"%Flf %Flf %Flf %Flf %Flf %Flf",&pDPoints[0].x,&pDPoints[0].y,
		                										  			&pDPoints[1].x,&pDPoints[1].y,
		                										  			&pDPoints[2].x,&pDPoints[2].y);
		                if (n != 6)
		                	goto ErrorEnd; 
		                BasePoint = pDPoints[0];    
		                RCURVE (&pDPoints[0].x,&pDPoints[0].y,
		                		&pDPoints[1].x,&pDPoints[1].y,
		                		&pDPoints[2].x,&pDPoints[2].y, &RP.x,&RP.y,&CLEN); 
		                PCAZ = getazd (&RP,&BasePoint);
		                PCAZ = LTWOPI (PCAZ - DSIGN(HALFPI,CLEN));
		                ConvertAndTranCoord (&pDPoints[0],hTranFile);
		                ConvertAndTranCoord (&pDPoints[1],hTranFile);
                 	    ConvertAndTranCoord (&pDPoints[2],hTranFile);  
                 	    Type = 3;
		                NumPoints = 3;   
		                GlobalUnlock (hPoly);
                 	    goto GetText;
                 	break;
                 	
                 	case 'P':
					{
	                 	if (!fgetstring (str,498,FidSHP))
	                		goto ErrorEnd; 
	                	lineno++;
	                	if (*str != 'C')
	                		goto ErrorEnd;
	                	pDPoints = (HPDPOINT)GlobalLock (hPoly);
		                n = sscanf (&str[1],"%Flf %Flf %Flf",&pDPoints[0].x,&pDPoints[0].y,
		                									 &AZ);
		                if (n != 3)
		                {
			                GlobalUnlock (hPoly);
		                	goto ErrorEnd; 
		                }  
		                Type = 2;  
		                BasePoint = pDPoints[0];  
						ConvertAndTranCoord (&pDPoints[0],hTranFile); 
						GlobalUnlock (hPoly);
						goto GetText;
			        } 
			        GSSiGlobUlFree (&hGRText);
                 	break;
                 	
                 	default:
                 	goto LineErr;
                 }	  
       DisplayPCT:          
                 CurLoc = GSSillseek (FidSHP,0,1);       
                 PctBox (GetDlgItem(hWndDlg,IDC_STATUS), TotLen, CurLoc,0);
                 goto NextLine;
                 
        ErrorEnd: 
                 {  
                    sprintf (mess,"Error in file at line %ld\r\n%s",lineno,str);
                    MessageBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
                 }
                    
        EndFile: 
				 if (ContinueProcessing) 
				 {
	                PctBox (GetDlgItem(hWndDlg,IDC_STATUS), TotLen, TotLen,0);
                 	SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Load complete");
                 }
                 else
                 	SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Load Cancelled");  
        Exit:
				 AddPointToMap (CenPt,0,0,0,0,0,0,0,0,0,0,0,0,0,TRUE,FALSE,NULL,NULL);
        		 CloseTRANS2 (&hTranFile);  
        		 GSSiGlobFree (&hPoly);
                 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),TRUE);
				 CloseMap(TRUE);
                 DisableHalt = FALSE;
         	 	 Processing = FALSE;
             	 ContinueProcessing = TRUE; 
                 FileProjectionType = SaveFPT; 
                 DisableHalt = FALSE;  
   				 GSSiClose (FidSHP);
		 	 	 AddSymToMap (NumSyms,hSymDesc,0,NULL); 
                 DestroySymList (&NumSyms,&hSymDesc);
                 CloseRefIndex(TRUE);           
             	 ForceRefIndex = ForceTAGIndex = FALSE; 
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

long ConvertAndTranCoord (LPDPOINT pDPoint,HANDLE hTranFile)
{
	long	rtn;
	
	rtn = ConvertCoord (pDPoint,3,1);
	if (hTranFile)
		TRANS2 (pDPoint->x,pDPoint->y,&pDPoint->x,&pDPoint->y,hTranFile);
	return rtn;
}    

BOOL FAR PASCAL CREATE_ORTHOCDS2MsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 
    short     TabStops[2]={50,1300}, i;
    char    Ext[8], ExtID[34], Name[128],str[260], VolLabel[16], CDDrive[6];
    LPSTR    lpstr=str;  
    HCURSOR OldCursor=0;    
    static   HANDLE hSQL=0;
    long        TotLen, CurLoc, County, NumCounties;
    LPLONG      pCounty, pCnty;  
	LPSTR	pCDDrive, pEnd;
    static      short   FileType = 0;  
	static		char	CDName[8]="";
    
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG: 
        SendDlgItemMessage (hWndDlg,IDC_CDLIST,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
        SendDlgItemMessage (hWndDlg,IDC_DOQLIST,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
        _fstrcpy (str,"*.*"); 
        DlgDirList (hWndDlg,str,IDC_DESTDRIVE,IDC_CURDIR,DDL_DRIVES);
    case GSSI_REINITDIALOG:
	    GetCurVal (str,sizeof(str),IDS_FILEOQL); 
        SendDlgItemMessage (hWndDlg,IDC_CDLIST,LB_RESETCONTENT,0,0);
        SetDlgItemText (hWndDlg,IDC_QUAD_FILE,str);
        { 
				GWDHEADER GWDHead; 
				LPGWDHEADER lpGWDHead;
				HANDLE  hDB;   
				long	Offset;
				short 	pos=BT_FIRST;
				LPCDDATA	pCDData; 
				char	LastCD[8]=""; 
				
				hDB = OpenGWDatabase (str,BT_READ);
				if (!hDB)
					break;
			    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
			    pCDData = (LPCDDATA)&lpGWDHead->GWDData;
			    while (!BT_FIND (lpGWDHead->BTHandle[1],lpGWDHead->pKeys[1],pos,BT_ANY, (LPSTR)&Offset))
			    {   
			    	pos = BT_NEXT;
			        FillGWDData (lpGWDHead,Offset); 
			        if (_fstrnicmp (LastCD,pCDData->GSSiCD,6))
			        {
			        	_fstrncpy (LastCD,pCDData->GSSiCD,6); 
			        	LastCD[6]=0;
	                	SendDlgItemMessage (hWndDlg,IDC_CDLIST,LB_ADDSTRING,0,(LPARAM)LastCD); 
	                }
				}
				GlobalUnlock (hDB);
			    CloseGWDatabase (hDB);
		} 
        
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
         	case IDC_ADDCD:
         	{
         		 short	iCD, iDrive;  
         		 LPSTR	pTab;
         		 char	str2[32];
         		
	          	 iCD=SendDlgItemMessage(hWndDlg,IDC_CDLIST,LB_GETCURSEL,NULL,NULL);
	          	 iDrive=SendDlgItemMessage(hWndDlg,IDC_DESTDRIVE,LB_GETCURSEL,NULL,NULL); 
	          	 if (iCD < 0 || iDrive < 0)
	          	 	break;
				 SendDlgItemMessage(hWndDlg,IDC_CDLIST,LB_GETTEXT,iCD,(DWORD)str); 
				 SendDlgItemMessage(hWndDlg,IDC_DESTDRIVE,LB_GETTEXT,iDrive,(DWORD)str2);
				 if ((pTab = _fstrstr (str,"\t")))
				 	*pTab = 0;
				 _fstrcat (str,"\t");
				 _fstrcat (str,str2);
                 SendDlgItemMessage (hWndDlg,IDC_CDLIST,LB_DELETESTRING,iCD,0); 
                 SendDlgItemMessage (hWndDlg,IDC_CDLIST,LB_INSERTSTRING,iCD,(LPARAM)str); 
                 EnableWindow (GetDlgItem(hWndDlg,IDC_TEST_DOQS),TRUE);
	        }
         		 break;
         		 
            case IDC_LOCATE_QUADFILE: 
            {
                if (!GetFileName3 (hWndDlg,str,IDS_FILTERGWD,IDS_FILEGMD)) break;   
                SetDlgItemText (hWndDlg,IDC_QUAD_FILE,str);
                SetCurVal (str,IDS_FILEOQL);
		        PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
			}
                 break;
            
            case IDC_CREATE_DEST:     
            {
				GWDHEADER GWDHead; 
				LPGWDHEADER lpGWDHead;
			    LPGWFLDINFO lpGWFldInfo;      
				HANDLE  hDB;   
				long	Offset;
				OFSTRUCT	OFStruct;
				HFILE	OutFid;
				short 	pos, cond, iCD, iDOQ, LastFile;
				LPCDDATA	pCDData; 
				char	WantCD[8], Drive[8], DestDrive[8], SourceDrive[8], CDID[16], QuadID[16];
				char	VolLabel[16];
				LPSTR	lpTab; 
				BOOL	FirstFile=TRUE;  
				HCURSOR	hcurSave;
				
			    hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
                OutFid = GSSiOpenFile ("c:\\orthos.txt",&OFStruct,OF_CREATE);
                GetDlgItemText (hWndDlg,IDC_QUAD_FILE,str,128);
				hDB = OpenGWDatabase (str,BT_READ);
			    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
			    pCDData = (LPCDDATA)&lpGWDHead->GWDData;  
			    iCD = 0;
			    while (SendDlgItemMessage(hWndDlg,IDC_CDLIST,LB_GETTEXT,iCD++,(DWORD)str)>=0) 
			    {   
			    	if ((lpTab = _fstrchr (str,'\t')))
			    	{   
			    		*lpTab++ = 0; 
			    		_fstrcpy (WantCD,str); 
			    		_fstrcpy (DestDrive,lpTab);
			    		pos = BT_FIRST;
			    		cond = BT_GE; 
			    		LastFile = 0;
					    lpGWFldInfo=lpGWDHead->pFldInfo;  
					    lpGWFldInfo++;
					    SetFieldValFromChar(lpGWDHead,lpGWFldInfo,WantCD,FALSE,FALSE); 
					    GWDFormKey(lpGWDHead,1,TRUE,0);
					    while (!BT_FIND (lpGWDHead->BTHandle[1],lpGWDHead->pKeys[1],pos,cond, (LPSTR)&Offset))
					    {   
					    	pos = BT_NEXT;  
					    	cond = BT_ANY; 
					        FillGWDData (lpGWDHead,Offset); 
					    	if (_fstrnicmp (WantCD,pCDData->GSSiCD,6)) 
					    		break;
					        {   
					        	if (LastFile != pCDData->GSSiFile)
					        	{   
					        		if (!FirstFile)
					        			fputstring ("***",OutFid);  
					        		FirstFile = FALSE;  
					        		_fstrncpy (CDID,pCDData->GSSiCD,6);
					        		CDID[6]=0; 
					        		Truncate (CDID);
									sprintf (str,"%c:\\%s\\file%i",DestDrive[2],CDID,pCDData->GSSiFile);	
					            	fputstring (str,OutFid);
					        		
					        	}
					        	LastFile = pCDData->GSSiFile;
					        	iDOQ=0;  
					        	GetQuadVolLabel (pCDData->QuadID,VolLabel);
						    	Truncate (VolLabel);
							    while (SendDlgItemMessage(hWndDlg,IDC_DOQLIST,LB_GETTEXT,iDOQ++,(DWORD)str)>=0) 
							    {   
							    	lpTab = _fstrchr (str,'\t');
							    	*lpTab++ = 0; 
							    	if (!_fstricmp (str,VolLabel))
							    	{	
							    		_fstrcpy (SourceDrive,lpTab); 
							    		_fstrncpy (QuadID,pCDData->QuadID,12);
							    		QuadID[12]=0;
										sprintf (str,"%s\\data\\%s",SourceDrive,QuadID);
										if (!ExistFile(str))	
											sprintf (str,"%s\\%s\\data\\%s",SourceDrive,VolLabel,QuadID);
						            	fputstring (str,OutFid);
						            	OutputQuadData (OutFid,pCDData->QuadID); 
							    		goto NextQuad;
							    	}  
							    }
			                }
				NextQuad:;  
						}
					}
				}  
				GSSiClose (OutFid);
				GlobalUnlock (hDB);
			    CloseGWDatabase (hDB); 
			    GSSiSetCursor (hcurSave);
			    _fstrcpy (str,"[%INDIR]LOADDOQ.EXE"); 
			    ExpandText (str);
			    {
			    	UINT	ierr;
			    	char	mess[128];
			    
					if ((ierr = WinExec (str,SW_SHOW)) < 32)
					{
						sprintf (mess,"Error loading DOQ Loader: %ld",(long) ierr);
						MessageBox (hWndMain,mess,NULL,0);  
					}
				} 
								    
			}
                 break;
                 
            case IDC_TEST_DOQS: 
            {
				GWDHEADER GWDHead; 
				LPGWDHEADER lpGWDHead;
			    LPGWFLDINFO lpGWFldInfo;      
				HANDLE  hDB;   
				long	Offset;
				short 	pos, cond, iCD, iDOQ;
				LPCDDATA	pCDData; 
				char	WantCD[8], Drive[8];
				LPSTR	lpTab; 
				BOOL	AllFound = TRUE;
				long	Ref;   
				HIGHLIGHTDATA	HighlightData;
				
			    while (!BT_FIND (hHighlight,(LPSTR)&Ref,pos,BT_ANY,(LPSTR)&HighlightData))
			    {   
			    	OFSTRUCT	OFStruct;
			    	HFILE	Fid = GSSiOpenFile ("[%DL]zonebmps.txt",&OFStruct,OF_READ);
			    	HANDLE	hDLT;
			    	
			    	fgetstring (str,256,Fid);
					ProcessDelimTextHeader(str,NULL,Fid,&hDLT);
		  			while (fgetstring (str,256,Fid))
		  			{
						GetDelimTextData(str,hDLT);  
					}
					GSSiClose (Fid); 
				}  
                SendDlgItemMessage (hWndDlg,IDC_DOQLIST,LB_RESETCONTENT,0,0);
                GetDlgItemText (hWndDlg,IDC_QUAD_FILE,str,128);
				hDB = OpenGWDatabase (str,BT_READ);
			    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
			    pCDData = (LPCDDATA)&lpGWDHead->GWDData;  
			    iCD = 0;
			    while (SendDlgItemMessage(hWndDlg,IDC_CDLIST,LB_GETTEXT,iCD++,(DWORD)str)>=0) 
			    {   
			    	if ((lpTab = _fstrchr (str,'\t')))
			    	{   
			    		*lpTab = 0; 
			    		_fstrcpy (WantCD,str);
			    		pos = BT_FIRST;
			    		cond = BT_GE;
					    lpGWFldInfo=lpGWDHead->pFldInfo;  
					    lpGWFldInfo ++;
					    SetFieldValFromChar(lpGWDHead,lpGWFldInfo,WantCD,FALSE,FALSE); 
					    GWDFormKey(lpGWDHead,1,TRUE,0);
					    while (!BT_FIND (lpGWDHead->BTHandle[1],lpGWDHead->pKeys[1],pos,cond, (LPSTR)&Offset))
					    {   
					    	pos = BT_NEXT;  
					    	cond = BT_ANY; 
					        FillGWDData (lpGWDHead,Offset); 
					    	if (_fstrnicmp (WantCD,pCDData->GSSiCD,6)) 
					    		break;
					        {   
					        	GetQuadVolLabel (pCDData->QuadID,VolLabel);
						    	Truncate (VolLabel);
					        	iDOQ=0;
							    while (SendDlgItemMessage(hWndDlg,IDC_DOQLIST,LB_GETTEXT,iDOQ++,(DWORD)str)>=0) 
							    {   
							    	lpTab = _fstrchr (str,'\t');
							    	*lpTab = 0;  
							    	if (!_fstricmp (str,VolLabel))
							    		goto NextVol;  
							    }
							    if (!GetCDDriveForVol (VolLabel,Drive)) 
							    {
							    	AllFound = FALSE;
							    	_fstrcpy (Drive,"?");
							    }
								sprintf (str,"%s\t%s",VolLabel,Drive);	
								SendDlgItemMessage (hWndDlg,IDC_DOQLIST,LB_ADDSTRING,0,(LPARAM)str);        	
			                }
				NextVol:;  
						}
					}
				}
				GlobalUnlock (hDB);
			    CloseGWDatabase (hDB); 
			    if (AllFound)
                	EnableWindow (GetDlgItem(hWndDlg,IDC_CREATE_DEST),TRUE);
			    
			}
                 break;
                 
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 EndDialog(hWndDlg, FALSE);
                 break;
            
          }
          break;

    default:
        return FALSE;
   }
 return TRUE;    
}   

BOOL ImportSSURGOTables (LPSTR CompFile,LPSTR CompGMD)
{   
	char	AttFile[128], TextFile[128];
	HANDLE	hStr=GSSiGlobAlloc ( 920,GMEM_MOVEABLE,4096);  
	LPSTR	str=GlobalLock (hStr);
	HANDLE	hVal=GSSiGlobAlloc ( 921,GMEM_MOVEABLE,4096);  
	LPSTR	vals=GlobalLock (hVal);
	HANDLE	hLoc=GSSiGlobAlloc ( 922,GMEM_MOVEABLE,4096);  
	LPSTR	*loc=(LPSTR*)GlobalLock (hLoc);
	HFILE	Fid;
	OFSTRUCT	OFStruct;
	LPSTR	lpTAB, EndVar, Val;
	short	nVar=1,nVar2=1, i,j;  
	LPGWDHEADER lpGWDHead;
	HANDLE  hDB=0;
	BOOL	rtn=FALSE;
	char	muid[12]; 
	long	line;  

	
	if (!ExistFile (CompFile)) 
	{
		MessageBox (GetFocus(),"Comp file missing",NULL,MB_ICONEXCLAMATION);
		goto Exit; 
	}
	if (!ExistFile (CompGMD))
	{
		Fid = GSSiOpenFile ("[%INDIR]comp.txt",&OFStruct,OF_READ);
		if (Fid == HFILE_ERROR)
		{
			MessageBox (GetFocus(),"CompGMD template file missing",NULL,MB_ICONEXCLAMATION);
			goto Exit;
		}
		fgetstring (str,4090,Fid); 
		GSSiClose (Fid);
		lpTAB = str;
		while ((lpTAB = _fstrchr (lpTAB,',')))
		{
			nVar++;
			lpTAB++;
		}
		CreateGWDDatabase (CompGMD,1,TRUE,nVar,2,str);
	}
	Fid = GSSiOpenFile (CompFile,&OFStruct,OF_READ);
	fgetstring (str,4090,Fid);   
	*loc = str;
	nVar = 1;
	while ((lpTAB=_fstrchr (*loc,'\t')))
	{   
		nVar++;
		EndVar = _fstrchr (*loc,':');
		*EndVar = 0; 
		lpTAB++;
		loc++;
		*loc = lpTAB;
	}
	EndVar = _fstrchr (*loc,':');
	*EndVar = 0; 
	GlobalUnlock (hLoc);
	hDB = OpenGWDatabase (CompGMD,BT_WRITE);
	if (!hDB)
	{
		MessageBox (GetFocus(),"Unable to open soil attribute database",CompGMD,MB_ICONEXCLAMATION);
		goto Exit;
	}
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
	fgetstring (vals,4090,Fid); 
	line = 2;  
	while (fgetstring (vals,4090,Fid))
	{   
		line++;
		loc = (LPSTR*)GlobalLock (hLoc); 
		Val = vals;
		for (i=0;i<nVar;i++)
		{   
			if (!Val)
			{   
				char	mess[256];
				
				sprintf (mess,"Error in %s at line %ld",OFStruct.szPathName,line);
				MessageBox (GetFocus(),mess,NULL,MB_ICONEXCLAMATION);
				goto NextLine;
			}
			if ((lpTAB=_fstrchr (Val,'\t')))
				*lpTAB++ = 0;  
			if (!_fstricmp(*loc,"stssaid"))
			{ 
				_fstrncpy (muid,Val,2);
				muid[2] = 0;
			}  
			else
			{   
				LPSTR	vloc=Val;
				
				if (!_fstricmp(*loc,"muid"))
				{ 
					_fstrcat (muid,Val);  
					vloc = muid;
				} 
				Truncate (vloc); 
				if (!SetFieldValFromCharAndName(lpGWDHead,*loc,vloc,FALSE))
				{
					MessageBox (GetFocus(),*loc,"Error loading field",MB_ICONEXCLAMATION);
					GlobalUnlock (hLoc);
					goto Exit;
				}
			}
			Val = lpTAB;
			loc++;
		} 
NextLine:
		GlobalUnlock (hLoc);
		GWDAddRecord (lpGWDHead,0,NULL);
	} 
	GSSiClose (Fid);
	rtn = TRUE;
Exit: 
	if (hDB)
		GlobalUnlock (hDB);
    CloseGWDatabase (hDB);
   	GSSiGlobUlFree (&hStr);
	GSSiGlobFree (&hLoc);
	GSSiGlobUlFree (&hVal);
	return rtn;
}
 
BOOL FAR PASCAL LOADUMAREASMsgProc(HWND hWndDlg, WORD Message, WPARAM wParam, LPARAM lParam)
{ 
    char		Ext[6]=".TL3";
    LPTAGDEF    lpTAGDef; 
    short     	i;
    char    	SHPExt[8], ExtID[34];
    char    	SymName[34], drive[4],file[34];
    char        Prefix[10];
    HFILE   FidSHP;
    LPSTR   lpDot, lpMIDstr;  
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
    BOOL        More;  
    LPSTR		pFile; 
    float		size,rot;
    short         rc; 
    BOOL		NewOpt;
    static	BOOL	FileIsOpen;   
    static	HANDLE	hAttFile=0;
	short		SaveDrive, idrive;
	HANDLE	hMem=0;
    LPSTR    	str, dir;
    LPSTR       UDI, Name;
    LPSTR    	CSize,CRot,CColor; 
    LPSTR		mess, CmdString; 
    LPSHORT		stuff;
    

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 if ((BRtn = ImportCommonCode (hWndDlg,Message, wParam, lParam,hSQL))) return (BRtn);  
 hMem = GSSiGlobAlloc ( 923,GHND,2048+256);
 str = GlobalLock (hMem);
 dir = str + 256;
 UDI = dir + 128;
 Name = UDI + 128;
 CSize = Name + 256;
 CRot = CSize + 128;
 CColor = CRot + 128;  
 mess = CColor + 128; 
 CmdString = mess + 256;
 stuff = (LPSHORT) (CmdString + 256);
 switch(Message)
   {
    case WM_INITDIALOG:
         FileIsOpen = FALSE;
         if (NumTAGDef>0)
         { 
	         lpTAGDef = (LPTAGDEF)GlobalLock (hTAGDef);
	         for (i=0;i<NumTAGDef;i++,lpTAGDef++) 
	            SendDlgItemMessage (hWndDlg,IDC_TAPREFIX,CB_ADDSTRING,0,(LPARAM)lpTAGDef->Prefix);
	         GlobalUnlock (hTAGDef); 
	     }
         SendDlgItemMessage (hWndDlg,IDC_TYPEAREA,BM_SETCHECK,TRUE,0);
	     SetDlgItemText (hWndDlg,IDC_GET_SYM,"Area Symbol");  
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Feet");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Meters");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Degrees");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Degrees * 1000000");
         _fstrcpy (str,"*.CVT");
         DlgDirListComboBox (hWndDlg,str,IDC_PROJECTION,0,DDL_READWRITE);   
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_SELECTSTRING,-1,(LPARAM)curunits);
         SendDlgItemMessage (hWndDlg,IDC_PROJECTION,CB_SELECTSTRING,-1,(LPARAM)curproject);
         if (*AutoExportName)
		 	PostMessage(hWndDlg, WM_COMMAND, IDC_RECALL, 0L);
    case GSSI_REINITDIALOG: 
    	 
         if (FileIsOpen && *AutoExportName)
	         PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);     
         break; /* End of WM_INITDIALOG                                 */

    case WM_COMMAND:
#if WIN32
         switch(LOWORD(wParam))
#else
         switch(wParam)
#endif
         {  
            case IDC_GET_SYM:
				DecodeAreaSym (SymStuff,SymName,CColor);
            	if (!SelectAreaSymbol (hWndDlg,1,SymName,CColor,FALSE))
            		goto NoSym;    
            	sprintf (SymStuff,"%s;%s",SymName,CColor);
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
                 
                 hAttFile = 0;   
                 _fstrcpy (SHPExt,".GEN"); 
                 _fstrcpy (ExtID,"ARC GEN Files");
                 sprintf (gszFilter,"%s(*%s)|*%s|",ExtID,SHPExt,_fstrlwr(SHPExt));  
                 if (GetFileName3(hWndDlg,LoadName,0,IDS_FILETXT))
                 {   
                   	SetDlgItemText (hWndDlg,IDC_FILE,LoadName);
                   	if ((lpDot = _fstrrchr (LoadName,'.')))
                   	{
                   		_fstrcpy (lpDot,".txt");
                   		if (ExistFile (LoadName))  
                   		{
                    		SetDlgItemText (hWndDlg,IDC_ATTRIBUTE_FILE,LoadName);
                    		SetDlgItemText (hWndDlg,IDC_UDI,"[%GENDATA]"); 
                    		hAttFile = GSSiGlobAlloc ( 924,GMEM_MOVEABLE,256);
                    		pFile = GlobalLock (hAttFile);
                    		_fstrcpy (pFile,LoadName);
                    		GlobalUnlock (hAttFile);
                    	}
                    }
                 } 
                 break;
                 
            case IDC_TYPEAREA:
            case IDC_TYPELINE:
            {      
            		
                 if (SendDlgItemMessage (hWndDlg,IDC_TYPEAREA,BM_GETCHECK,0,0))
	               	SetDlgItemText (hWndDlg,IDC_GET_SYM,"Area Symbol"); 
	             else 
	               	SetDlgItemText (hWndDlg,IDC_GET_SYM,"Line Symbol");  
            }  
            break;
                 
            case IDC_SAVE:
            {    
            	 short	Version=2; 
            	 HFILE	FidSave; 
            	 OFSTRUCT	OFStruct;
            	 
                 if (!GetSaveName2 (hWndDlg,Name,0,Ext,IDS_FILETL3)) break; 
                 FidSave = GSSiOpenFile (Name,&OFStruct,OF_CREATE);
                 BigWrite (FidSave,Ext,6,-1);
                 BigWrite (FidSave,(HPSTR)&Version,2,-1);   
                 GetDlgItemText (hWndDlg,IDC_FILE,LoadName,128);
                 BigWrite (FidSave,(HPSTR)LoadName,128,-1);
                 GetDlgItemText (hWndDlg,IDC_DESTMAP,PltName,128);
                 BigWrite (FidSave,(HPSTR)PltName,128,-1);
                 NewOpt = SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0);
                 BigWrite (FidSave,(HPSTR)&NewOpt,2,-1);
                 GetDlgItemText(hWndDlg,IDC_TAPREFIX,Prefix,sizeof(Prefix));
                 BigWrite (FidSave,(HPSTR)Prefix,sizeof(Prefix),-1);
                 GetDlgItemText(hWndDlg,IDC_UDI,UDI,sizeof(UDI));               
                 BigWrite (FidSave,(HPSTR)UDI,sizeof(UDI),-1);
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
                 _fmemset (str,0,sizeof(str));
                 ii=BigWrite (FidSave,(HPSTR)str,sizeof(str),-1); //spacer for future options
                 WriteAdvancedOpts (FidSave);
                 GSSiClose (FidSave);
            } 
                    
            	 break;
            	 
            case IDC_EXIT: 
             	GSSiGlobFree (&hAttFile);
                EndDialog(hWndDlg, TRUE); 
                break;
           	case IDC_RECALL: 
           	{
           		 short Version;     
            	 HFILE	FidSave;
            	 OFSTRUCT	OFStruct;
           		 
           		 
             	 if (!*AutoExportName)
             	 { 
					 if (!GetFileName2 (hWndDlg,Name,Ext,IDS_FILETL3))
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
                 BigRead (FidSave,UDI,sizeof(UDI));
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
			     	 ii = GSSillseek (FidSave,0,1);
	                 ii=BigRead (FidSave,str,sizeof(str));
			     }
			     while (ReadObject (&FidSave, FALSE,NULL,0));
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
                 long   lineno=0, attline=0, itemno=0, TotLen, CurLoc, MidLine, NewRefno, ID, IDAtt;     
                 DPOINT	CenPt;
                 short  iUDI=0, LineSym;
                 BOOL   Done, Create=FALSE, FileIsDir; 
                 HANDLE hMIDstr, hBT, hSQLAttImport=0;
                 LPSTR   pPrefix, lpChr;
                 double coordcvt=1;
                 long     ii=100, Dummy, nRecBytes, nBytes;
                 short    NumSyms=0, SymNum, cond, UnknownSym, n,Type=0; 
                 HANDLE	  hSymDesc=0, hTranFile=0;      
                 BOOL	Exclusion=FALSE, HiPrecis; 
                 HFILE	FidAtt;
                 long	NoFile=0;                                           
                 char	AttData[256];
         
                 if (!GetDlgItemText (hWndDlg,IDC_DESTMAP,PltName,128)) 
                 {
                    MessageBox(GetFocus(),"No destination map", 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }
                 if (!GetDlgItemText (hWndDlg,IDC_SYMNAME,SymStuff,lnSymStuff))
                 {
                    MessageBox(GetFocus(),"No symbol selected", 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 } 
                 *curproject = 0;
                 if (!GetDlgItemText (hWndDlg,IDC_PROJECTION,curproject,lncurproject))
                 {
                    MessageBox(GetFocus(),"No input projection set", 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }   
				 HiPrecis = SendDlgItemMessage (hWndDlg,IDC_HIPRECIS,BM_GETCHECK,0,0);
                 if (GetDlgItemText (hWndDlg,IDC_TRANFILE,str,128))
                 {
                 	hTranFile = LoadTranFileWithDandT (str);
                 	if (!hTranFile)
                	{
                    	MessageBox(GetFocus(),"Invalid transformation file", str,MB_ICONEXCLAMATION|MB_OK);
                    	break;
                    }  
                 }
                 if (SendDlgItemMessage (hWndDlg,IDC_TYPELINE,BM_GETCHECK,0,0)) 
                 	Type = 1;
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
                 
                 GetDlgItemText (hWndDlg,IDC_FILE,Name,128); 
                 GetDlgItemText (hWndDlg,IDC_ATTRIBUTE_FILE,str,128); 
                 FidSHP=GSSiOpenFile (Name,&OFStruct,OF_READ);
                 if (FidSHP == HFILE_ERROR)  
                 {  
                    sprintf (mess,"Unable to open file %s",Name);
                    MessageBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }  
				 DecodePointSym (SymStuff,SymName,CSize,CRot,CColor);
                 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),FALSE);
                 SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Loading Data");
                 TotLen = GSSillseek (FidSHP,0,2);  
                 GSSillseek (FidSHP,0,0);
                 ExpandText (PltName);
                 PltType = 2;
                 Done = FALSE;
                 MidLine=0; 
                 str[0]=0;  
                 
                 if ((FileIsDir=SendDlgItemMessage (hWndDlg,IDC_DIRECTORY,BM_GETCHECK,0,0)))
                 	DBoundsInit (&EditBounds);
                 else
                 { 
					 OpenMap (CurView->hWnd,CurView->hDC);
					 EditBounds = CurView->FileMNMX; 
					 CloseMap (FALSE);  
				 }
                 pPrefix = Prefix;    
                 GetDlgItemText(hWndDlg,IDC_TAPREFIX,Prefix,sizeof(Prefix));
                 if (!*pPrefix) 
                    pPrefix = 0;   
                    
                 
                 if (hAttFile)
                 {
                 	pFile = GlobalLock (hAttFile);
                 	FidAtt = GSSiOpenFile (pFile,&OFStruct,OF_READ);
                 	GlobalUnlock (hAttFile);
                 }   
                 SaveFPT = FileProjectionType; 
				 FileProjectionType=0;
                 DisableHalt = TRUE;  
                 ContinueProcessing = TRUE;   
                 Processing = TRUE;
                                        
                 NewRefno=0;  
                 nRecBytes = 0; 
        NextLine: 
                 if (!ContinueProcessing) Done = TRUE;
                 if (Done)
                    goto EndFile;                
                 if (!fgetstring (str,128,FidSHP))
                	goto EndFile;
                 lineno++;
                 if (!_fstricmp (str,"END"))
                 	goto EndFile;
                 itemno++;   
                 ReplaceChar (str,'D','E'); 
                 n = sscanf (str,"%ld %Flf %Flf",&ID,&CenPt.x,&CenPt.y);
                 
                 if (hAttFile)
                 {
                 	fgetstring (str,200,FidAtt);
                 	attline++;
                 	lpChr = FirstNonBlank(str);
                 	IDAtt = atol (lpChr);
	                if (ID != IDAtt)
	                {
	                    MessageBox(GetFocus(),"Coordinate file/data file mismatch", 0,MB_ICONQUESTION|MB_OK);
	                    goto ErrorEnd;
	                }
                 	if (!(lpChr = NextBlank (str)))
                 		lpChr = _fstrchr (str,0);
                 	else
                 		lpChr++;
           		    SetGlobalValue ("%GENDATA",lpChr);
                 }
				 GetDlgItemText (hWndDlg,IDC_SYMNAME,SymStuff,lnSymStuff);
				 DecodePointSym (SymStuff,SymName,CSize,CRot,CColor);
				 ExpandText (SymName);
				 Truncate (SymName); 

                 {  
                    short     nPoly, nPoints,lastnpoints,j;
                    long    nVertex, Loc, Code;
                    HANDLE  hhPoly,hNumPoints; 
                    LPHANDLE	phPoly;
                    LPDPOINT    lpDPoint, lpDPoints;
                    DPOINT  LinkPoint;
                    LPSTR   lpSpace;
                    BOOL    Store, FirstPoly;
                    SHPPOLYHEADER   SHPPolyHeader;  
                    HPDPOINT    pPoints;
                    HANDLE      hPoints, hnPoints;
                    LPSHORT     pNumPoints;
                    long		LastIndex; 
                    short		NumPoints, SymType; 
                    MNMXCORD	Bounds;

                    nPoly = 1;  
                    NumPoints = 0;
					hhPoly = GSSiGlobAlloc ( 925,GMEM_MOVEABLE,sizeof(HANDLE)*256); 
					phPoly = (LPHANDLE)GlobalLock (hhPoly);
					hNumPoints = GSSiGlobAlloc ( 926,GHND,sizeof(short)*256); 
					pNumPoints = (LPSHORT)GlobalLock (hNumPoints);  
					DBoundsInit (&Bounds);
		NextGENLink:                    
                 	if (!fgetstring (str,128,FidSHP))
                		goto ErrorEnd;
	                 lineno++;
	                *phPoly = GSSiGlobAlloc ( 927,GMEM_MOVEABLE,UINT_MAX); 
	                pPoints = (HPDPOINT)GlobalLock (*phPoly);
	    NextGENPoint:
	                if (!_fstricmp (str,"END"))
	                {   
	                	DPOINT LastPoint;
	                	
	                	pPoints--;
	                	LastPoint = *pPoints;
                 		GlobalUnlock (*phPoly);
	                	pPoints = (HPDPOINT)GlobalLock (*phPoly);
	                	if (!Type && (pPoints->x != LastPoint.x || pPoints->y != LastPoint.y))
	                	{   
	                		LastPoint = *pPoints;
	                		pPoints += *pNumPoints;
	                		*pPoints = LastPoint;
	                		(*pNumPoints)++;       
	                	}
                 		GlobalUnlock (*phPoly);
                 		if (Exclusion)
                 			*phPoly = ReversePoints (*pNumPoints,*phPoly);
		            	*phPoly = GlobalReAlloc (*phPoly,*pNumPoints*sizeof(DPOINT),GMEM_MOVEABLE);
		            	Loc = GSSillseek (FidSHP,0,1);
	                 	if (!fgetstring (str,128,FidSHP))
	                		goto ErrorEnd;
	                	if (!_fstricmp (str,"END"))
	                	{
   		                	GSSillseek (FidSHP,Loc,0); 
		                 	goto EndGENItem; 
		                }
                 		Code = atol (str);
               			Exclusion = FALSE;
                 		if (Code == -99999)
                 			Exclusion = TRUE;
                 		else if (Code != 99999) 
                 		{
		                	GSSillseek (FidSHP,Loc,0); 
                 			goto EndGENItem;
                 		}
		                lineno++;
                 		nPoly++; 
                 		phPoly++;
                 		pNumPoints++;
                 		goto NextGENLink;
	                }
	                ReplaceChar (str,'D','E'); 
                 	n = sscanf (str,"%Flf %Flf",&pPoints->x,&pPoints->y);
                 	if (n != 2)
	               		goto ErrorEnd;
                 	(*pNumPoints)++;
                 	if (!fgetstring (str,128,FidSHP))
                		goto ErrorEnd; 
		            lineno++;
                	pPoints++;
                	goto NextGENPoint;
        EndGENItem:
					GlobalUnlock (hNumPoints);
                    GlobalUnlock (hhPoly);
                    if (pPrefix)
                    {
						GetDlgItemText(hWndDlg,IDC_UDI,UDI,256); 
						ExpandText (UDI); 
					}             
                    else
                    	*UDI=0;
                    SymNum = GetOrCreateSym (SymName,&NumSyms,&hSymDesc,Create,SymType);
                    if (SymNum)
                    {   
		                if (SendDlgItemMessage (hWndDlg,IDC_USERECIDASREFNO,BM_GETCHECK,0,0)) 
							NewRefno = ID;
						else
							NewRefno = GetNewRefno(PltName,NULL,NULL,NULL,NULL);
						SetIntRefno (NewRefno); //in case TLID set to [%INT_REFNO] 
	
						phPoly = (LPHANDLE)GlobalLock (hhPoly);
						pNumPoints = (LPSHORT)GlobalLock (hNumPoints);
	                    for (j=0;j<nPoly;j++,phPoly++,pNumPoints++)
	                    {
		                    pPoints = (LPDPOINT)GlobalLock (*phPoly); 
		                    for (i=0;i<*pNumPoints;i++,pPoints++)
		                    {
								if (ConvertAndTranCoord (pPoints,hTranFile))
		                        {   
		                            MessageBox(GetFocus(),"Unable to convert coordinates as specified", 0,MB_ICONQUESTION|MB_OK);
		                            goto ErrorEnd;
		                        }
								AddDPointToMinMax (pPoints,&Bounds);
		                    }
		                    GlobalUnlock (*phPoly);  
		                }
		                GlobalUnlock (hhPoly);
		                GlobalUnlock (hNumPoints);
	                    Store=TRUE; 
	                    LastIndex = 0;   
	                    FirstPoly=TRUE;
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
						pNumPoints = (LPSHORT)GlobalLock (hNumPoints);
	                    if (Store) 
	                    {
			                SetIntRefno (NewRefno);   
	                        AddPolyToMap (nPoly,pNumPoints, phPoly,Type,NewRefno,NULL,-1,SymNum,stuff,Prefix,UDI,
	                                        -1,-1,0,0,0,0,0,HiPrecis);
	                    }
                    	GlobalUnlock (hNumPoints);
                    } 
                    GlobalFree (hNumPoints);
                    GlobalUnlock (hhPoly); 
                    phPoly = (LPHANDLE)GlobalLock (hhPoly);
                    while (nPoly--)
                    	GlobalFree (*phPoly++);  
                    GlobalUnlock (hhPoly);
                    GlobalFree (hhPoly);
                    
                 } 
                 
                 CurLoc = GSSillseek (FidSHP,0,1);       
                 PctBox (GetDlgItem(hWndDlg,IDC_STATUS), TotLen, CurLoc,0);
                 if (TotLen-CurLoc <= 8)
                    Done=TRUE;
                 goto NextLine;
                 
        ErrorEnd: 
                 {  
                    sprintf (mess,"Error in file at line %ld\r\n%s",lineno,str);
                    MessageBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
                 }
                    
        EndFile: 
				 if (ContinueProcessing) 
				 {
	                PctBox (GetDlgItem(hWndDlg,IDC_STATUS), TotLen, TotLen,0);
	                sprintf (mess,"Load complete - %ld records skipped",NoFile);
                 	SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,mess);
                 }
                 else
                 	SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Load Cancelled");
                 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),TRUE);
				 CloseMap(TRUE);
                 DisableHalt = FALSE;
         	 	 Processing = FALSE;
             	 ContinueProcessing = TRUE; 
                 FileProjectionType = SaveFPT; 
                 DisableHalt = FALSE;  
                 CloseTRANS2 (&hTranFile);
   				 GSSiClose (FidSHP);
   				 if (FileIsDir) 
			 	 	AddSymToDir (PltName,NumSyms,hSymDesc,0,NULL);  
			 	 else
			 	 	AddSymToMap (NumSyms,hSymDesc,0,NULL); 
                 DestroySymList (&NumSyms,&hSymDesc);
                 CloseRefIndex(TRUE);           
             	 ForceRefIndex = ForceTAGIndex = FALSE; 
             	 if (hAttFile)
             	 	GSSiClose (FidAtt);
             	 if (*AutoExportName) 
             	 {
             	 	GSSiGlobFree (&hAttFile);
                 	EndDialog(hWndDlg, TRUE); 
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

BOOL FAR PASCAL LOADSSURGOMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 
    static	HANDLE	hAttFile=0;
    short     TabStops[2]={65,1300}, i;
    LPSTR  lpTab;
    char    Ext[8], ExtID[34], SHPExt[8], str[256];
    LPSTR    lpDot, pFile, pSlash;  
	HCURSOR	hcurSave;
    static   HANDLE hSQL=0;
    long        TotLen, CurLoc, County, NumCounties;
    LPLONG      pCounty, pCnty;  
    static      short     FileType = 0, nJust;
	char	drive[8], dir[128], name[10], ext[6], VolLabel[16],CurDir[128], FileName[132], DirName[128];
   	OFSTRUCT	OFStruct;     
   	HFILE	FidList, FidAtt;  
   	char	MUSYM[10], MUID[16];
   	static	HANDLE	hTempFile=0;   
   	static	long	TotFiles;  
   	static	char	STSSAID[8]; 
   	
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:
         
         SetDlgItemText (hWndDlg,IDC_ATTDIR,"[%DL]soilmap\\soilcomp.gmd");
		 SendDlgItemMessage (hWndDlg,IDC_SKIPBLANK,BM_SETCHECK,TRUE,0); 
         
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
                 
            case IDCANCEL:
                 if (Processing)
                    ContinueProcessing=FALSE;
                 break;
            
            case IDC_EXIT:
                 ForceRefIndex = ForceTAGIndex = FALSE;
             	 if (hTempFile)
             	 {
             		pFile = GlobalLock (hTempFile);
             		GSSiRemove (pFile);
             		GSSiGlobUlFree (&hTempFile);
             	 }
                 EndDialog(hWndDlg, TRUE);
                 break;
            
            case IDC_LOCATE_SOURCE: 
                 
                 hAttFile = 0;
                 _fstrcpy (SHPExt,".?AF"); 
                 _fstrcpy (ExtID,"SSURGO Soil Files");
                 sprintf (gszFilter,"%s(*%s)|*%s|",ExtID,SHPExt,_fstrlwr(SHPExt));  
                 if (GetFileName3(hWndDlg,DirName,0,IDS_FILEOAF))
                 {   
	                 _splitpath (DirName,drive,dir,NULL,NULL);
	                 sprintf (DirName,"%s%s",drive,dir);
	                 if (!hTempFile)  
	                 	hTempFile = GSSiGlobAlloc ( 928,GMEM_MOVEABLE,256);
	                 pFile = GlobalLock (hTempFile);
					 GSSiGetTempFileName (0,"gm",0,pFile); 
	            	 FidList = 	GSSiOpenFile (pFile,(LPOFSTRUCT)&OFStruct,OF_CREATE);
	            	 lpTab = LastChr (DirName);
	            	 if (*lpTab == '\\')
	            	 	*lpTab = 0;
                   	 SetDlgItemText (hWndDlg,IDC_SOURCEDIR,DirName);
                   	 if ((pSlash = _fstrrchr (DirName,'\\'))) 
                   	 {
                   	 	char	destdir[64]="[%DL]soilmap"; 
                   	 	
                   	 	_fstrcat (destdir,pSlash); 
                   	 	SetDlgItemText (hWndDlg,IDC_DESTDIR,destdir);
                   	 }
                   	 TotFiles = 0;
					 SearchFilesInDir (DirName,"", FidList,&TotFiles,"*.?af",1);
	                 GSSillseek (FidList,0,0);
	                 while (fgetstring (FileName,128,FidList))
	                 {
	                   	lpDot = LastChr (FileName);
	                   	*lpDot = 'a';
	                   	if (ExistFile (FileName))
	                   		goto Next; 
	                   	sprintf (str,"Attribute file missing: %s",FileName);
	                   	MessageBox (GetFocus(),str,NULL,MB_ICONEXCLAMATION);
	                   	break; 
            Next:;
                     }
	                 GSSiClose (FidList); 
	                 sprintf (FileName,"%s\\comp",DirName);
	                 FidAtt = GSSiOpenFile (FileName,&OFStruct,OF_READ);
	                 if (FidAtt == HFILE_ERROR)
	                 {  
	                 	MessageBox (GetFocus(),"comp file not found in directory",NULL,MB_ICONEXCLAMATION);
	                 	break;
	                 } 
	                 {
	                 	HANDLE	hSTR = GSSiGlobAlloc ( 929,GMEM_MOVEABLE,4096);
	                 	LPSTR	str = GlobalLock (hSTR); 
	                 	      
		                 fgetstring (str,4090,FidAtt);
		                 fgetstring (str,4090,FidAtt);
		                 fgetstring (str,4090,FidAtt);
		                 sscanf (str,"%s\t%s\t%s\t",STSSAID,MUID,MUSYM);
		                 nJust = _fstrlen(MUID) - _fstrlen(MUSYM) - 2;
 
		                 GSSiGlobUlFree (&hSTR);
		             }
	                 STSSAID[5]=0;
	                 GSSiClose (FidAtt);
					 sprintf (str,"%ld soil files to be loaded",TotFiles);
                	 SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,str);
                 	 EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE);
                 } 
                 break; 
                 
            case IDOK: 
            {
                 MNMXCORD MinMaxCoord;  
                 long   lineno=0, TotLen, CurLoc, MidLine;  
                 long   NewRefno; 
                 short    st, SymNum, iUDI=0, AreaSym, LineSym, Pass;
                 BOOL   Done; 
                 HANDLE hMIDstr;
                 char   Prefix[10], UDI[34], AreaSymName[10], LineSymName[10], FloodInfoDB[128]; 
                 LPSTR  lpComma;  
                 OFSTRUCT   OFStruct;
                 long       filecode,filelength,ii=100, NumOverLimit;  
                 HFILE  FidBNA, FidCvt;
                 char   mess[256];  
                 short        nPoly, nPoints,lastnpoints, Symbol;
                 long	MaxSides;
                 long   nVertex, AreaRef=0;
                 DPOINT	LinkPoint, FirstIslandPoint, ControlPt;
                 HANDLE hPoly;
#if WIN32
                 DPOINT  *pPoints;
#else                  
                 HPPOINT	pPoints;
#endif                 
                 HANDLE     hPoints, hnPoints, hLines;
                 LPSHORT      pnPoints;
                 short        NumPoints, Nump;  
                 short    NumSyms=0, n;  
                 HANDLE hSymDesc=0; 
                 BOOL   Store,WantFirstPoint;   
                 char   LSym[20], Name[132], SaveAlt[64], Zone[64], Parent[64], Code[16];
				 long nFiles, iFile=0, AreaNum=0, NumAreas;
				 HANDLE hNumPoints, hBT;  
				 LPSHORT   lpItems; 
				 short	UTMZone, NumAttributes,NumLinesInArea,nRec, NumIslands,ipoly;
				 long	NumLines,Loc,LineID,NodeLoc,NextAreaLoc,Dummy;
				 HANDLE	hLineIndex;   
				 LPHANDLE	phPoints;
				 LPLONG	pLineIndex;
				 LPSHORT pLineID;
				 LPSHORT pNumPoints; 
				 short	cond,AttID, Major, Minor, LineIDLoc;
				 long	Minor99;
			     LPOPENFILEDATA  FilePtr;
			     LPOPENSQLDATA   SQLPtr;  
			     char	SymName[34]; 
			     HANDLE	hSymbol; 
			     short	nCodes=0; 
			     long	ifile;
				 LPSYMBOL	CurSymbol;  
				 char	DestDir[128], AttDir[128], format[32], drive[8], dir[128], CompName[128];
				 HANDLE	hMinorCodes, hMinorVals;
				 LPSHORT	pMinorCodes;
				 LPSTR	MinorVals; 
				 short	StateNum;
                                          
                 sprintf (format,"%%s%%.%ild%%s",nJust);
                 _fstrcpy (str,STSSAID);
                 str[2] = 0;
                 StateNum = GetStateNum (str); 
                 sprintf (str,"%.2i%s0000",StateNum,&STSSAID[2]); 
                 SetGlobalValue ("%INITIALREFNO",str);                     
                 if (!GetDlgItemText (hWndDlg,IDC_DESTDIR,DestDir,sizeof(DestDir)))
                 {  
                    
                    MessageBox(GetFocus(),"No destination directory", 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }
                 makedirectories (DestDir,TRUE,FALSE);
                 GetDlgItemText (hWndDlg,IDC_ATTDIR,AttDir,sizeof(AttDir));
                 AreaSym = GetOrCreateSym ("SOILAREA",&NumSyms,&hSymDesc,TRUE,3);
                 AddToSymList (AreaSym,&NumSyms,&hSymDesc);
                 PltType = 2;  
                 
                 pFile = GlobalLock (hTempFile);
                 FidList = GSSiOpenFile (pFile,&OFStruct,OF_READ);

				 fgetstring (Name,128,FidList); 
				 GSSillseek (FidList,0,0);
			 	 _splitpath (Name,drive,dir,NULL,NULL);
				 sprintf (CompName,"%s%scomp",drive,dir);
				 if (*AttDir)
				 	if (!ImportSSURGOTables (CompName,AttDir))
				 		break;                 
                 
                 EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),FALSE);
                 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE); 
                 
                 
				 NumOverLimit = MaxSides = 0;  
                 for (ifile = 0;ifile < TotFiles;ifile++)
				 {   
				 	 fgetstring (Name,128,FidList); 
				 	 sprintf (str,"Loading file %s",Name);             
                	 SetDlgItemText (hWndDlg,IDC_PROCESS_MESS2,str);
	                 FidBNA = GSSiOpenFile (Name,&OFStruct,OF_READ); 
	                 TotLen = GSSillseek (FidBNA,0,2);   
	                 AreaNum=0;
	                 GSSillseek (FidBNA,0,0);
	                 lpTab = LastChr (Name);  
	                 *lpTab = 'a';
	                 FidAtt = GSSiOpenFile (Name,&OFStruct,OF_READ);
	                 hMinorCodes = GSSiGlobAlloc ( 930,GHND,UINT_MAX); 
	                 hMinorVals = GSSiGlobAlloc ( 931,GHND,UINT_MAX);
	                 while (fgetstring (str,128,FidAtt))
	                 {  
	                 	long	seq,major,minor,n=0;
	                 	char	sym[16];
	                 	sscanf (str,"%ld\t%ld\t%ld\t%s",&seq,&major,&minor,sym);
	                 	if (major == 999)
	                 	{
		                	pMinorCodes = (LPSHORT)GlobalLock (hMinorCodes);
		                	MinorVals = GlobalLock (hMinorVals);
		                	while (*pMinorCodes && (minor != *pMinorCodes))
		                	{
		                		pMinorCodes++;
		                		MinorVals+=8;
		                	}
		                	if (!*pMinorCodes)
		                	{
		                		*pMinorCodes = minor;
		                		_fstrcpy (MinorVals,sym);
		                	}
		                	GlobalUnlock (hMinorCodes);
		                	GlobalUnlock (hMinorVals);
	                 	}
	                 } 
	                 GSSiClose (FidAtt); 
	                 _splitpath (Name,drive,dir,name,ext);
	                 sprintf (PltName,"%s\\%7s%c.plt",DestDir,&name[1],ext[1]);
	                 ExpandText (PltName);
	                 PltType = 2;
	                 Done = FALSE;
	                 MidLine=0; 
	                 str[0]=0;  
	                 SaveFPT = FileProjectionType; 
					 FileProjectionType=0;   
					 Processing = TRUE;
	                 DisableHalt = TRUE;  
					 
	                 i=4;
	                 while (i--)
	                 	fgetstring (str,80,FidBNA);
					 UTMZone = atoi (&str[14]);                 
	                 FidCvt = GSSiOpenFile ("fema.cvt",&OFStruct,OF_CREATE); 
	                 fputstring ("1",FidCvt);
	                 ltoa (UTMZone,str,10);
	                 fputstring (str,FidCvt);
	                 fputstring ("0",FidCvt);
	                 fputstring ("1.0",FidCvt);
	                 for (i=0;i<14;i++)
	                	fputstring ("0.0",FidCvt);
	                 GSSiClose (FidCvt);   
			         ConvertCoordClose (); 
	                 _fstrcpy (SaveAlt,"[%ALT_PROJECTION]");
	                 ExpandText (SaveAlt);
	                 _fstrcpy (str,"[%ALT_PROJECTION]=FEMA");
		             ExpandText (str);
		             i = 6;
	                 while (i--)
	                 	fgetstring (str,80,FidBNA);
	                DBoundsInit (&MinMaxCoord);
	                i=4;
	                while (i--)
	                {
		              	fgetstring (str,80,FidBNA);
		              	ControlPt.x = atof (&str[36]);
		              	ControlPt.y = atof (&str[49]);
		            	ConvertCoord(&ControlPt,3,1);
			            AddDPointToMinMax(&ControlPt,&MinMaxCoord);
			        }
	                {
	                    short NumPens=10;
	                    PENDESC PenDesc[10];
	                    
	                    SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Loading Data");
	                    for (i=0;i<NumPens;i++)
	                    {
	                        PenDesc[i].PenNum = i+1;
	                        PenDesc[i].Width = (float)1.0; 
	                        PenDesc[i].Style = 1;
	                        PenDesc[i].Color = RGB(0,0,0);
	                    }
	                    CreateNewMap (PltName,&MinMaxCoord,NumSyms,hSymDesc,
	                                                       NumPens,(LPPENDESC)&PenDesc,0,0,TRUE); 
						EditBounds = CurView->FileMNMX;
	                } 
	              	fgetstring (str,80,FidBNA);
	              	NumLines = atol (&str[56]);
	              	NumAreas = atol (&str[46]);
	// build the line index
					hLineIndex = GSSiGlobAlloc ( 932,GMEM_MOVEABLE,4*(NumLines+1));
	              	fgetstring (str,80,FidBNA);
					NodeLoc = GSSillseek (FidBNA,0,1);
	                SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Building line index");
					
				    hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
	NextLine:       
					while (*str != 'L') 
					{
						Loc = GSSillseek (FidBNA,0,1);
		              	if (!fgetstring (str,80,FidBNA))
		              		goto GotLines;
		            }
					pLineIndex = (LPLONG)GlobalLock (hLineIndex); 
					LineID = atol (&str[1]);
					pLineIndex+=LineID;
					*pLineIndex = Loc;
					GlobalUnlock (hLineIndex);
					Loc = GSSillseek (FidBNA,0,1);
	              	if (fgetstring (str,80,FidBNA))
	              		goto NextLine;
			GotLines:
					GlobalUnlock (hLineIndex);					         
	                GSSillseek (FidBNA,NodeLoc,0);
	                lineno = 0;
	                *str = 0;
					while (*str != 'A') 
					{
						NextAreaLoc = GSSillseek (FidBNA,0,1);
		              	if (!fgetstring (str,80,FidBNA))
		              		goto EndFile;
		            } 
					GSSiSetCursor (hcurSave);
	                SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Loading areas");
		  	NextArea: 
		  			GSSillseek (FidBNA,NextAreaLoc,0);
	              	fgetstring (str,80,FidBNA); 
	              	if (*str == 'L')
	              		goto EndFile;
	                if (!ContinueProcessing)
	                    goto EndFile;
	                NumLinesInArea = atoi (&str[36]);  
	                NumAttributes = atoi (&str[48]);   
	                NumIslands = atoi (&str[60]);  
	                nPoly = NumIslands+1;
	                hNumPoints = GSSiGlobAlloc ( 933,GMEM_MOVEABLE,2*nPoly);
	                hLines = GSSiGlobAlloc ( 934,GHND,2*(long)(1+NumLinesInArea)); 
	                pLineID = (LPSHORT)GlobalLock (hLines);
	                nRec = (NumLinesInArea-1)/12 +1; 
	                n = NumLinesInArea;
	                while (nRec--) 
	                {
		              	fgetstring (str,80,FidBNA); 
	                	LineIDLoc = 0;
	                	while (n && LineIDLoc < 72)
	                	{
	                		*pLineID++ = atoi (&str[LineIDLoc]);
	                		LineIDLoc += 6; 
	                		n--;
	                	}
		            }
		            GlobalUnlock (hLines);
	                nRec = (NumAttributes-1)/6 +1;
	                n = NumAttributes;
	                AttID = Minor99 = 0;
	                while (nRec--) 
	                {
		              	fgetstring (str,80,FidBNA);
	                	LineIDLoc = 0;
	                	while (n && LineIDLoc < 72)
	                	{   
	                		n--;
	                		AttID++;
	                		Major = atoi (&str[LineIDLoc]);
	                		LineIDLoc += 6;
	                		Minor = atoi (&str[LineIDLoc]);
	                		LineIDLoc += 6; 
	                		if (Major == 999)
	                			Minor99 = Minor;
	                	}
		            }
		            Store = FALSE;  
	                if (Minor99) 
	                {                
	                	pMinorCodes = (LPSHORT)GlobalLock (hMinorCodes);
	                	MinorVals = GlobalLock (hMinorVals);
	                	while (*pMinorCodes && (Minor99 != *pMinorCodes))
	                	{
	                		pMinorCodes++;
	                		MinorVals+=8;
	                	}
	                	if (!*pMinorCodes)
	                		MessageBox (GetFocus(),"Unable to locate map unit symbol",NULL,MB_ICONEXCLAMATION);
	                	_fstrcpy (MUSYM,MinorVals);
	                	GlobalUnlock (hMinorCodes);
	                	GlobalUnlock (hMinorVals);
	                	Store = TRUE;
	                }
		            NextAreaLoc = GSSillseek (FidBNA,0,1); 
	                AreaNum++; 
	                if (!_fstricmp (MUSYM,"BLANK") &&
	                	SendDlgItemMessage (hWndDlg,IDC_SKIPBLANK,BM_GETCHECK,0,0))
	                	Store = FALSE;    
	                if (Store)
	                {   
						short lcmdstring;
						char	CmdString[64];
						short	stuff[40];
						long	TotPoints=0, NumSym;  
						LPSTR	NonNumSym;
						
	                	pNumPoints = (LPSHORT)GlobalLock (hNumPoints); 
	                	pLineID = (LPSHORT)GlobalLock (hLines);  
	                	hPoints = GSSiGlobAlloc ( 935,GMEM_MOVEABLE,nPoly*4);
	                	phPoints = (LPHANDLE)GlobalLock (hPoints);
	                	for (ipoly=0;ipoly<nPoly;ipoly++)
	                	{ 
							*phPoints++ = LoadDLGPoints (pNumPoints,&pLineID,hLineIndex,FidBNA);
							TotPoints += *pNumPoints;
							pNumPoints++;
						} 
						GlobalUnlock (hNumPoints);
						GlobalUnlock (hPoints);
	                	pNumPoints = (LPSHORT)GlobalLock (hNumPoints); 
						AreaRef = GetNewRefno(PltName,NULL,NULL,NULL,NULL); 
						phPoints = (LPHANDLE)GlobalLock (hPoints); 
						
						NonNumSym = FirstNonInt (MUSYM);
						if (NonNumSym > MUSYM)
						{
							NumSym = atol (MUSYM);
							sprintf (MUID,format,&STSSAID[2],NumSym,NonNumSym);
						}
						else		                     	 
							sprintf (MUID,"%s%s",&STSSAID[2],NonNumSym);
						sprintf (CmdString,"[%%MUSYM]=%s;[%%MUID]=%c%c%s",MUSYM,STSSAID[0],STSSAID[1],MUID);
						lcmdstring = _fstrlen (CmdString);
						lcmdstring += lcmdstring%2; 
						stuff[1]=40;
						stuff[2]=lcmdstring;
						_fstrncpy ((LPSTR)&stuff[3],CmdString,lcmdstring);
						stuff[0]=2+2+lcmdstring; 
						MaxSides = max (MaxSides,TotPoints);   
		                if (TotPoints > 65000)  
		                	NumOverLimit++;
		                else
							AddPolyToMap (nPoly,pNumPoints,phPoints,0,AreaRef,NULL,-1,AreaSym,stuff,NULL,NULL,-1,-1,-1,0,0,0,0,FALSE);
						GlobalUnlock (hNumPoints);
						GlobalUnlock (hPoints);
						phPoints = (LPHANDLE)GlobalLock (hPoints);
						while (nPoly--)
							GlobalFree (*phPoints++); 
						GSSiGlobUlFree (&hPoints);
	                }  
	                GlobalFree (hLines);
	                GlobalFree (hNumPoints);
	                CurLoc = GSSillseek (FidBNA,0,1);       
	                PctBox (GetDlgItem(hWndDlg,IDC_STATUS), NumAreas, AreaNum,0);
	                goto NextArea;
	                 
	        EndFile: 
	                 CloseMap (TRUE);
	                 CloseRefIndex(TRUE);           
	                 GSSiClose (FidBNA);  
					 GlobalFree (hLineIndex); 
					 GlobalFree (hMinorCodes);
					 GlobalFree (hMinorVals);
	                 
			         ConvertCoordClose ();  
			         SetGlobalValue ("%ALT_PROJECTION",SaveAlt);
	                 PctBox (GetDlgItem(hWndDlg,IDC_STATUS2), TotFiles, ifile+1,0);  
	                 if (!ContinueProcessing)
	                 	ifile = TotFiles;
			     }
                 DestroySymList (&NumSyms,&hSymDesc);
             	 if (hTempFile)
             	 {
             		pFile = GlobalLock (hTempFile);
             		GSSiRemove (pFile);
             		GSSiGlobUlFree (&hTempFile);
             	 }
                 sprintf (mess,"Load complete. %ld areas skipped due to too many points. Largest area had %ld points",
                 				NumOverLimit,MaxSides);
                 MessageBox( GetFocus(),mess,"", MB_OK);
                 DisableHalt = FALSE;
                 Processing = FALSE; 
                 ForceRefIndex = ForceTAGIndex = FALSE;
                 ContinueProcessing = TRUE;
                 FileProjectionType = SaveFPT;  
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),TRUE);
                 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE);
                 break;
                 
            }   
          }
          break;
    default:
        return FALSE;
   }
 return TRUE;    
}  

DWORD MIFColorToWinColor (COLORREF MIFColor)
{ 
	BYTE	R,G,B; 
	
	R = GetRValue (MIFColor);
	G = GetGValue (MIFColor);
	B = GetBValue (MIFColor);
	return RGB (B,G,R);
}

BOOL GetMIFCharacteristics (HFILE FidMIF, LPSTR str,LPLONG lineno)
{
    LPSTR   lpBeg, lpEnd; 
    long    ii;
    
NextLine:
    if (!fgetstring(str,256,FidMIF))
        return TRUE;
    (*lineno)++;  
    lpBeg = FirstNonBlank(str);
    if (!_fstrnicmp (lpBeg,"PEN ",4))
    {
        lpBeg = _fstrchr (str,'(');
        lpEnd = _fstrchr (lpBeg++,',');
        *lpEnd = 0;
        MIFPenWidth = atoi (lpBeg);
        lpBeg = ++lpEnd;
        lpEnd = _fstrchr (lpBeg,',');
        *lpEnd = 0;
        MIFPenStyle = atoi (lpBeg);
        lpBeg = ++lpEnd;
        lpEnd = _fstrchr (lpBeg,')');
        *lpEnd = 0;
        MIFPenColor = MIFColorToWinColor(atol (lpBeg)); 
    }
    else if (!_fstrnicmp (lpBeg,"BRUSH ",6))
    { 
        lpBeg = _fstrchr (str,'(');
        lpEnd = _fstrchr (lpBeg++,',');
        *lpEnd = 0;
        MIFPattern = atoi (lpBeg);
        lpBeg = ++lpEnd;
        lpEnd = _fstrchr (lpBeg,',');
        *lpEnd = 0;
        MIFForeColor = MIFColorToWinColor(atol (lpBeg));
        lpBeg = ++lpEnd;
        lpEnd = _fstrchr (lpBeg,')');
        *lpEnd = 0;
        MIFBackColor = MIFColorToWinColor(atol (lpBeg)); 
    }
    else if (!_fstrnicmp (lpBeg,"SYMBOL ",7))
    {
    }
    else if (!_fstrnicmp (lpBeg,"CENTER ",7))
    {
    } 
    else if (!_fstrnicmp (lpBeg,"SMOOTH",6))
    {
    } 
    else
        return FALSE;
    goto NextLine;
}

BOOL GetMIFTextCharacteristics (HFILE FidMIF, LPSTR str,LPLONG lineno,LPDOUBLE pAngle)
{
    LPSTR   lpBeg, lpEnd; 
    long    ii;
    
    *pAngle = 0;
NextLine:
    if (!fgetstring(str,256,FidMIF))
        return TRUE;
    (*lineno)++;  
    lpBeg = FirstNonBlank(str);
    if (!_fstrnicmp (lpBeg,"FONT ",4))
    {
        lpBeg = _fstrchr (str,'(');
        lpEnd = _fstrchr (lpBeg++,',');
        *lpEnd = 0;
        MIFPenWidth = atoi (lpBeg);
        lpBeg = ++lpEnd;
        lpEnd = _fstrchr (lpBeg,',');
        *lpEnd = 0;
        MIFPenStyle = atoi (lpBeg);
        lpBeg = ++lpEnd;
        lpEnd = _fstrchr (lpBeg,')');
        *lpEnd = 0;
        MIFPenColor = MIFColorToWinColor(atol (lpBeg)); 
    }
    else if (!_fstrnicmp (lpBeg,"SPACING ",8))
    { 
    }
    else if (!_fstrnicmp (lpBeg,"JUSTIFY ",8))
    {
    }
    else if (!_fstrnicmp (lpBeg,"ANGLE ",6))
    { 
    	lpBeg += 6;
    	*pAngle = atof (lpBeg) * RADDEG;
    } 
    else if (!_fstrnicmp (lpBeg,"LABEL ",6))
    {
    } 
    else
        return FALSE;
    goto NextLine;
}

#if WIN32
    BOOL WINAPI LOADMIFMsgProc(HWND hWndDlg, WORD Message, WPARAM wParam, LPARAM lParam)
#else
    BOOL FAR PASCAL LOADMIFMsgProc(HWND hWndDlg, WORD Message, WPARAM wParam, LPARAM lParam)
#endif
{ 
    char   Prefix[10], UDI[256], txt[256]; 
    short   i;
    char    ExtID[34];
    char	Ext[6]=".TL4";
    char    MidName[128];     
    LPSTR	str;
    HANDLE	hStr;
    HFILE   FidMIF, FidMID;
    LPSTR   pPrefix=NULL, pUDI=NULL, lpDot, lpMIDstr;  
    HCURSOR OldCursor=0;  
    char	SymName[64]; 
    double	size,rot; 
    long	TLID=1000000000;
    BOOL	NewOpt, CoordOpt; 
    static	BOOL	FileIsOpen;   
    static	HANDLE	hDLT=0;  
   	char	CSize[128],CRot[128],CColor[128];
    LPTAGDEF    lpTAGDef; 
    OFSTRUCT	OFStruct;


 short    BRtn;
 if ((BRtn = ImportCommonCode (hWndDlg,Message, wParam, lParam,0))) return (BRtn);  
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:
    	 FileIsOpen = FALSE;
		 SetDlgItemText(hWndDlg,IDC_SYMNAMEA,"AUTOMATIC");         
		 SetDlgItemText(hWndDlg,IDC_SYMNAMEL,"AUTOMATIC");         
		 SetDlgItemText(hWndDlg,IDC_SYMNAMEP,"AUTOMATIC");         
         SendDlgItemMessage (hWndDlg,IDC_HIPRECIS,BM_SETCHECK,TRUE,0L);   
         SendDlgItemMessage (hWndDlg,IDC_DOCONVERT,BM_SETCHECK,TRUE,0L);   
         SendDlgItemMessage (hWndDlg,IDC_PRESERVE_MAPINFO,BM_SETCHECK,TRUE,0L);   
         LoadTAGDef ();   
         if (NumTAGDef)
         { 
             lpTAGDef = (LPTAGDEF)GlobalLock (hTAGDef);
             for (i=0;i<NumTAGDef;i++,lpTAGDef++) 
                SendDlgItemMessage (hWndDlg,IDC_PREFIX,CB_ADDSTRING,0,(LPARAM)lpTAGDef->Prefix);
             GlobalUnlock (hTAGDef); 
         }
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
                short     nFields; 
                LPSTR   lpSpace;
                HANDLE	hFldstr;
    
                 _fstrcpy (Ext,".MIF"); 
                 _fstrcpy (ExtID,"MapInfo Data Interchange Format");
                 sprintf (gszFilter,"%s(*%s)|*%s|",ExtID,Ext,_fstrlwr(Ext));
                 if (GetFileName3(hWndDlg,LoadName,0,IDS_FILEMIF))   
                 {
                    OldCursor = GSSiSetCursor (LoadCursor (0,IDC_WAIT));
                    
                     SetDlgItemText (hWndDlg,IDC_FILE,LoadName);  
                     _fstrcpy (MidName,LoadName);
                     lpDot = _fstrrchr (MidName,'.');
                     if (lpDot)
                     {
                        *lpDot = 0;
                        _fstrcat (MidName,".mid");   
                        HaveMID = ExistFile (MidName);
                     }
                     if (!HaveMID) 
	                 {  
	                    
	                    MessageBox(GetFocus(),"No data file (.mid extension)", 0,MB_ICONEXCLAMATION|MB_OK);
	                    break;
	                 }
                     SetDlgItemText (hWndDlg,IDC_ATTRIBUTE_FILE,MidName);
                     ExpandText (LoadName);
                     FidMIF=GSSiOpenFile(LoadName,&OFStruct,OF_READ);
                     if (FidMIF==HFILE_ERROR) break; 
                     hStr = GSSiGlobAlloc ( 936,GMEM_MOVEABLE,1024);
                     str = GlobalLock (hStr); 
                     while (fgetstring(str,256,FidMIF))
                     {
                        if (!_fstrnicmp (str,"Columns ",8))
                        {   
                        	LPSTR	fldstr,lpEnd;
                        	
                        	hFldstr = GSSiGlobAlloc ( 937,GMEM_MOVEABLE,4096);
                        	fldstr = GlobalLock (hFldstr);
                            nFields = atoi (&str[8]); 
                            _fstrcpy (fldstr,"\"");
                            while (nFields--)
                            {
                                fgetstring(str,256,FidMIF);
                                if ((lpSpace=_fstrchr(&str[2],' ')))
                                    *lpSpace = 0; 
                                _fstrcat (fldstr,&str[2]);
                                _fstrcat (fldstr,"\",\"");
                            } 
                            lpEnd = _fstrchr (fldstr,0);
                            lpEnd-=2;
                            *lpEnd = 0;
							ProcessDelimTextHeader(fldstr,NULL,FidMIF,&hDLT); 
							GlobalUnlock (hFldstr);
							GlobalFree (hFldstr);  
							EnableWindow (GetDlgItem(hWndDlg,IDC_SHOW_FIELDS),TRUE);
                        }
                     }
                     GSSiClose (FidMIF); 
                 }
            }  
            GSSiSetCursor (OldCursor);     
            GSSiGlobUlFree (&hStr);
                 break;
                 
            case IDC_SHOW_FIELDS: 
            	 
            	 DisplayFieldList (hWndDlg,hDLT,NULL,2);
                 break;
                      
            case IDC_SAVE:
            {    
            	 short	Version=1; 
            	 HFILE	FidSave; 
            	 OFSTRUCT	OFStruct;
            	 
                 if (!GetSaveName2 (hWndDlg,MidName,0,Ext,IDS_FILETL4)) break; 
                 FidSave = GSSiOpenFile (MidName,&OFStruct,OF_CREATE);
                 BigWrite (FidSave,Ext,6,-1);
                 BigWrite (FidSave,(HPSTR)&Version,2,-1);   
                 GetDlgItemText (hWndDlg,IDC_FILE,LoadName,128);
                 BigWrite (FidSave,(HPSTR)LoadName,128,-1);
                 GetDlgItemText (hWndDlg,IDC_DESTMAP,PltName,128);
                 BigWrite (FidSave,PltName,128,-1);
                 NewOpt = SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0);
                 BigWrite (FidSave,(HPSTR)&NewOpt,2,-1);
                 GetDlgItemText(hWndDlg,IDC_TAPREFIX,Prefix,sizeof(Prefix));
                 BigWrite (FidSave,(HPSTR)Prefix,sizeof(Prefix),-1);
                 GetDlgItemText(hWndDlg,IDC_UDI,UDI,128);               
                 BigWrite (FidSave,(HPSTR)UDI,128,-1);
                 GetDlgItemText(hWndDlg,IDC_SYMNAMEA,SymStuff,lnSymStuff);               
                 BigWrite (FidSave,(HPSTR)SymStuff,lnSymStuff,-1);
                 GetDlgItemText(hWndDlg,IDC_SYMNAMEL,SymStuff,lnSymStuff);               
                 BigWrite (FidSave,(HPSTR)SymStuff,lnSymStuff,-1);
                 GetDlgItemText(hWndDlg,IDC_SYMNAMEP,SymStuff,lnSymStuff);               
                 BigWrite (FidSave,(HPSTR)SymStuff,lnSymStuff,-1);
                 CoordOpt = SendDlgItemMessage (hWndDlg,IDC_HIPRECIS,BM_GETCHECK,0,0);
                 BigWrite (FidSave,(HPSTR)&CoordOpt,2,-1);
                 NewOpt  = SendDlgItemMessage (hWndDlg,IDC_CREATE_SYMS,BM_GETCHECK,0,0);
                 BigWrite (FidSave,(HPSTR)&NewOpt ,2,-1);
                 NewOpt  = SendDlgItemMessage (hWndDlg,IDC_DIRECTORY,BM_GETCHECK,0,0);
                 BigWrite (FidSave,(HPSTR)&NewOpt ,2,-1);
                 WriteAdvancedOpts (FidSave);
                 _fmemset (txt,0,254);
                 BigWrite (FidSave,(HPSTR)txt,256,-1); //spacer for future options
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
					 if (!GetFileName2 (hWndDlg,MidName,Ext,IDS_FILETL4))
					 	break;
	             }
	             else
	             	_fstrcpy (MidName,AutoExportName);
                 FidSave = GSSiOpenFile (MidName,&OFStruct,OF_READ);
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
                 SetDlgItemText(hWndDlg,IDC_SYMNAMEA,SymStuff);               
                 BigRead (FidSave,SymStuff,lnSymStuff);
                 SetDlgItemText(hWndDlg,IDC_SYMNAMEL,SymStuff);               
                 BigRead (FidSave,SymStuff,lnSymStuff);
                 SetDlgItemText(hWndDlg,IDC_SYMNAMEP,SymStuff);               
	             BigRead (FidSave,(HPSTR)&NewOpt,2);
	             SendDlgItemMessage (hWndDlg,IDC_HIPRECIS,BM_SETCHECK,NewOpt,0);
	             BigRead (FidSave,(HPSTR)&NewOpt,2);
	             SendDlgItemMessage (hWndDlg,IDC_CREATE_SYMS,BM_SETCHECK,NewOpt,0);
	             BigRead (FidSave,(HPSTR)&NewOpt,2);
	             SendDlgItemMessage (hWndDlg,IDC_DIRECTORY,BM_SETCHECK,NewOpt,0);
			     while (ReadObject (&FidSave, FALSE,NULL,0));
                 GSSiClose (FidSave);
				 PostMessage(hWndDlg, WM_COMMAND, IDC_SET_SOURCE, 0L);
                 FileIsOpen = TRUE;
               	 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
            }    
            	break;
            	
            case IDC_LOCATE_DESTMAP: 
                 *txt=0;
				 if (SendDlgItemMessage (hWndDlg,IDC_DIRECTORY,BM_GETCHECK,0,0))
				 {
                     if (!GetFileName3 (hWndDlg,txt,IDS_FILTERINDEX,IDS_FILEINDEX)) break;
                 }   
				 else
				 {
	                 if (SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0)) 
	                 {
	                    if (!GetSaveName2 (hWndDlg,txt,IDS_FILTERPLT,".PLT",IDS_FILEPLT)) break;   
	                 }
	                 else  
	                 {
	                    if (!GetFileName3 (hWndDlg,txt,IDS_FILTERPLT,IDS_FILEPLT)) break;   
	                 } 
                 }
                 SetDlgItemText (hWndDlg,IDC_DESTMAP,txt);
                 break;
                 
            case IDC_GET_SYMA:
                _fstrcpy (SymName,"[%NEW_AREA_SYM]");
                ExpandText (SymName);
                 if (!SelectAreaSymbol (hWndDlg,1,SymName,NULL,FALSE))
                    break;  
                SetDlgItemText (hWndDlg,IDC_SYMNAMEA,SymName); 
                break;
                
            case IDC_GET_SYML:
                GetDlgItemText (hWndDlg,IDC_SYMNAMEL,SymStuff,lnSymStuff); 
				DecodeLineSym (SymStuff,SymName,CSize,CColor);
                if (!SelectLineSymbol (hWndDlg,1,SymName,NULL,NULL,FALSE))
                    break;  
               	sprintf (SymStuff,"%s;%s;%s",SymName,CSize,CColor);
                SetDlgItemText (hWndDlg,IDC_SYMNAMEL,SymStuff); 
                break;
                
            case IDC_GET_SYMP:
            {
            	
                GetDlgItemText (hWndDlg,IDC_SYMNAMEP,SymStuff,lnSymStuff); 
				DecodePointSym (SymStuff,SymName,CSize,CRot,CColor);
                if (!SelectPointSymbol (hWndDlg,1,SymName,"All",CSize,CRot,CColor,FALSE))
                    break;
             	sprintf (SymStuff,"%s;%s;%s;%s",SymName,CSize,CRot,CColor);
                SetDlgItemText (hWndDlg,IDC_SYMNAMEP,SymStuff); 
            }
                break;   
                
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
             	 DestroyAdvancedOpts();
				 DestroyFieldList ();   
				 GSSiGlobFree (&hDLT);
                 EndDialog(hWndDlg, FALSE);
                 break;
            
            case IDC_CANCELLOAD:
            	 ContinueProcessing = FALSE;
            	 break;
            	 
            case IDOK: 
            {
                 MNMXCORD MinMaxCoord;  
                 long   lineno=0, TotLen, CurLoc, MidLine;  
                 long   NewRefno, NoFile=0; 
                 short    st, iPREFIX=0, AreaSym, LineSym, PointSym, Pass, RegionType=0,ii;
                 BOOL   Done; 
                 HANDLE hMIDstr,hDLT;
                 LPSTR  lpTAB, lpBS, pBeg;
                 short  NumSyms=0, SymNum, TPointSym, TLineSym;  
                 HANDLE hSymDesc=0; 
                 OFSTRUCT	OFStruct;  
                 BOOL	Create, HiRes=TRUE, FileIsDir=FALSE, DoConvert = TRUE; 
                 HFILE	FidFiles=-2;
                 char	Name[132], FullName[144],drive[8],dir[128],name[16];   
                 long	ForeColor, PenColor;
                 short	PenWidth;

                 
				 if (GetGlobalBVal ("[%LOADREGIONSASPOLYLINES]"))
				 	RegionType = 1;
				 Create = SendDlgItemMessage (hWndDlg,IDC_CREATE_SYMS,BM_GETCHECK,0,0);
				 HiRes = SendDlgItemMessage (hWndDlg,IDC_HIPRECIS,BM_GETCHECK,0,0);
				 DoConvert = SendDlgItemMessage (hWndDlg,IDC_DOCONVERT,BM_GETCHECK,0,0);
                 if (!GetDlgItemText (hWndDlg,IDC_DESTMAP,PltName,128))
                 {  
                    
                    MessageBox(GetFocus(),"No destination map", 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }
                 GetDlgItemText (hWndDlg,IDC_FILE,Name,sizeof(Name));   
                 _fstrlwr (Name);
                 if (_fstrstr (Name,".mif")) goto OneFile;
		FidFiles = GSSiOpenFile (Name,&OFStruct,OF_READ);
NextFile: 
		if (FidFiles == -2)
			goto EndFiles;
		if (!ContinueProcessing || !fgetstring (Name,128,FidFiles))
			goto EndFiles;
OneFile:         
				 ExpandText (Name);
				 _fullpath (FullName,Name,144);  
				 SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,FullName);
				 _splitpath (FullName,drive,dir,name,NULL); 
				 sprintf (MidName,"%s%s%s.mid",drive,dir,name);
				 SetGlobalValue("%SOURCENAME",name);
                 FidMID=GSSiOpenFile(MidName,&OFStruct,OF_READ);
                 hMIDstr = GSSiGlobAlloc ( 938,GMEM_MOVEABLE,2048);
                 lpMIDstr = GlobalLock (hMIDstr);
                 FidMIF=GSSiOpenFile(Name,&OFStruct,OF_READ);
                 if (FidMIF==HFILE_ERROR)  
                 {  
                    char    mess[256];
                    
                    sprintf (mess,"Unable to open file %s",Name);
                    MessageBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }
                 i=GetDlgItemText(hWndDlg,IDC_SYMNAMEA,SymName,sizeof(SymName));
                 if (i<0)  
                 {  
                    MessageBox(GetFocus(),"No area symbol selected", 0,MB_ICONQUESTION|MB_OK);
                    break;
                 } 
                 if (_fstricmp (SymName,"AUTOMATIC"))
                 {
					 AreaSym = GetOrCreateSym (SymName,&NumSyms,&hSymDesc,Create,3);
	             }
                 i=GetDlgItemText(hWndDlg,IDC_SYMNAMEL,SymStuff,lnSymStuff);
                 if (i<0)  
                 {  
                    MessageBox(GetFocus(),"No line symbol selected", 0,MB_ICONQUESTION|MB_OK);
                    break;
                 }
				 DecodeLineSym (SymStuff,SymName,CSize,CColor);
                 if (_fstricmp (SymName,"AUTOMATIC"))
                 {
					 LineSym = GetOrCreateSym (SymName,&NumSyms,&hSymDesc,Create,2);
	             }
                 i=GetDlgItemText(hWndDlg,IDC_SYMNAMEP,SymName,sizeof(SymName));
                 if (i<0)  
                 {  
                    MessageBox(GetFocus(),"No point symbol selected", 0,MB_ICONQUESTION|MB_OK);
                    break;
                 }
                 if (!_fstricmp (SymStuff,"AUTOMATIC"))
                 {
	             }
                                 
				 TPointSym = GetOrCreateSym ("TUTTTEXT",&NumSyms,&hSymDesc,Create,3);
//				 TLineSym = GetOrCreateSym ("A01",&NumSyms,&hSymDesc,Create,3);
                 TotLen = GSSillseek (FidMIF,0,2);
                 DBoundsInit (&MinMaxCoord);
                 if (SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0))
                    Pass = 0;
                 else
                    Pass = 1; 
                 
                 SaveFPT = FileProjectionType; 
				 FileProjectionType=0;
                 DisableHalt = TRUE;  
                 ExpandText (PltName);
                 PltType = 2;  
                 EnableWindow (GetDlgItem(hWndDlg,IDC_CANCELLOAD),TRUE);
/*                 
  FidTIGER1 = GSSiOpenFile (Tiger1Name,&OFStruct,OF_CREATE);
  FidTIGER2 = GSSiOpenFile (Tiger2Name,&OFStruct,OF_CREATE);
  Pass=1;
  IgnoreBounds = TRUE;
*/                 
                 
NextPass:       
				 lineno = 0;
                 GSSillseek (FidMIF,0,0); 
                 GSSillseek (FidMID,0,0); 
                 ExpandText (PltName);
                 PltType = 2;
                 Done = FALSE;
                 MidLine=0; 
                 hStr = GSSiGlobAlloc ( 939,GHND,4096);
                 str = GlobalLock (hStr);  
//                 CSize = str + 512;
//                 CRot  = CSize + 512;
//                 CColor = CRot + 512; 
                 while (_fstricmp(str,"Data")) 
                 {
                    if (!fgetstring(str,256,FidMIF)) goto ErrorEnd;lineno++;
                    if (!_fstrnicmp(str,"Columns ",8))
                    {   
                    	short NumDataFlds;
                    	HANDLE	hHeader=GSSiGlobAlloc ( 940,GHND,4096);
                    	LPSTR	pHeader=GlobalLock (hHeader), pFldName, pFldNameEnd;
                    	
                    	NumDataFlds = atoi (&str[7]);
                    	while (NumDataFlds--)
                    	{
		                    if (!fgetstring(str,256,FidMIF)) goto ErrorEnd;lineno++; 
		                    pFldName = &str[2];
		                    pFldNameEnd = _fstrchr (pFldName,' ');
		                    *pFldNameEnd = 0;
		                    _fstrcat (pHeader,"\"");
		                    _fstrcat (pHeader,pFldName);
		                    _fstrcat (pHeader,"\"");  
		                    if (NumDataFlds)
		                    	_fstrcat (pHeader,",");  
                    	} 
						ProcessDelimTextHeader(pHeader,NULL,FidMIF,&hDLT); 
						GlobalUnlock (hHeader);
						GlobalFree (hHeader);
                    }
                    else if (!_fstrnicmp(str,"CoordSys ",9))
                    {   
                    	_fstrupr (str);
                    	if (_fstrstr (str,"NONEARTH UNITS"))
                    		DoConvert = FALSE;
                    }
                 }
                 if (!fgetstring(str,256,FidMIF)) goto ErrorEnd;lineno++; 
                 
                 if (Pass)
                 {
	                if ((FileIsDir=SendDlgItemMessage (hWndDlg,IDC_DIRECTORY,BM_GETCHECK,0,0)))
	                 	DBoundsInit (&EditBounds);
	                else
	                { 
						OpenMap (CurView->hWnd,CurView->hDC);
						EditBounds = CurView->FileMNMX; 
						CloseMap (FALSE);  
					}
					MinMaxCoord = EditBounds;
                    pPrefix = Prefix;    
                    iPREFIX = SendDlgItemMessage(hWndDlg,IDC_PREFIX,LB_GETCURSEL,0,0); 
                 }
                 else
                    iPREFIX = -1;
        NextLine:
                 if (Done || !ContinueProcessing) goto EndFile;
                 if (!*str)  //skip blank line before primitive type if it exists
                 	if (!fgetstring(str,256,FidMIF)) goto EndFile;lineno++;
                                
                 lineno++;
                 if (Pass)
                 {   
                 	 NewRefno = GetNewRefno (PltName,NULL,NULL,NULL,NULL);
                 }
				 if (!GetMIDData(FidMID,lpMIDstr,hDLT))
				 	goto ErrorEnd;  
				 if (iPREFIX >= 0)
				 {
				    GetDlgItemText(hWndDlg,IDC_PREFIX,Prefix,sizeof(Prefix));  
				    GetDlgItemText(hWndDlg,IDC_UDI,UDI,sizeof(UDI));  
				    ExpandText (UDI);
				 }
				 else
					pPrefix = 0;   
				 _fstrupr (str);  
                 if (!_fstrncmp(str,"REGION ",7))
                 {  
                    short   nPoly, nPoints,lastnpoints, nPolygon;
                    long    nVertex;
                    HANDLE  hhPoly, hnPnts; 
                    LPSHORT   pnPnts;
                    HPDPOINT    lpDPoint, lpDPoints;
                    DPOINT  LinkPoint;
                    LPSTR   lpSpace;
                    BOOL    Store, FirstPoly;
                    LPHANDLE	phPoly; 
                    
                    Store=TRUE;  
                    nPoints = 0;
                    nPoly = atoi(&str[7]);  
                    FirstPoly=TRUE;
					hhPoly = GSSiGlobAlloc ( 941,GMEM_MOVEABLE,sizeof(HANDLE)*nPoly); 
					phPoly = (LPHANDLE)GlobalLock (hhPoly);
                    hnPnts = GSSiGlobAlloc ( 942,GHND,nPoly*sizeof(short));
                    pnPnts = (LPSHORT)GlobalLock (hnPnts); 
                    nPolygon = nPoly;
                    while (nPoly--)
                    {    
	                     ForceRefIndex = TRUE;  
	                     OpenRefIndex (FALSE);
	                     ForceRefIndex = FALSE;
                         if (!fgetstring(str,256,FidMIF))
                         	goto ErrorEnd;
                         lineno++; 
                         nVertex = atol (str);
                         *(pnPnts++)=nVertex;
                         lastnpoints=nPoints;
                         nPoints += nVertex;
                         *phPoly = GSSiGlobAlloc ( 943,GMEM_MOVEABLE,(DWORD)nPoints*sizeof(DPOINT));
                         lpDPoint = (LPDPOINT)GlobalLock (*phPoly); 
                         lpDPoints = lpDPoint;
                         while (nVertex--)
                         { 
                            if (!fgetstring(str,256,FidMIF))
                            	goto ErrorEnd;
                            lineno++; 
                            lpDPoint->x = atof (str);
                            if (!(lpSpace=_fstrchr(str,' ')))
                            	goto ErrorEnd;
                            lpDPoint->y = atof (lpSpace); 
                            if (DoConvert)
                            if (ConvertCoord(lpDPoint,2,1)) 
                            {
                                Store=FALSE;
                                Done=TRUE;
                                FirstPoly=FALSE; 
                                break;
                            } 
                            if (!PointInFileBounds (lpDPoint,&MinMaxCoord,Pass))
                                Store=FALSE;    
                            lpDPoint++;
                         }   
                         GlobalUnlock (*phPoly++);
                         FirstPoly=FALSE;
                    } 
                    GlobalUnlock (hhPoly);
                	Done = GetMIFCharacteristics (FidMIF,str,&lineno);
                    GlobalUnlock (hnPnts);  
                    pnPnts = (LPSHORT)GlobalLock (hnPnts);
                	phPoly = (LPHANDLE)GlobalLock (hhPoly);
	                if (SendDlgItemMessage (hWndDlg,IDC_PRESERVE_MAPINFO,BM_GETCHECK,0,0))
	                {
	                	ForeColor = MIFForeColor;
	                	PenColor = MIFPenColor;
	                	PenWidth = MIFPenWidth;
	                }
	                else
	                {
	                	ForeColor = -1;
	                	PenColor = -1;
	                	PenWidth = 0;
	                }

                    if (Store)
                        AddPolyToMap (nPolygon,pnPnts, phPoly,RegionType,NewRefno,NULL,-1,AreaSym,0,pPrefix,UDI,
                                      ForeColor,PenColor,PenWidth,0,0,0,0,HiRes); 
                    GSSiGlobUlFree (&hnPnts); 
                    GlobalUnlock (hhPoly); 
                    phPoly = (LPHANDLE)GlobalLock (hhPoly);
                    while (nPolygon--)
                    	GlobalFree (*phPoly++);  
                    GSSiGlobUlFree (&hhPoly);
                    
                 }
                 else if (!_fstrncmp(str,"POINT ",6))
                 {  
                    DPOINT  DPoint; 
                    BOOL    Store=TRUE;  
                    LPSTR   lpSpace;  
                    short     i, nPnts, nPoly;
                    COLORREF	color;
                    BOOL	rtn,HiPrecis=TRUE; 
                    
                    
                    lpSpace = &str[5];
                    DPoint.x = atof(lpSpace++);
                    lpSpace = _fstrchr(lpSpace,' ');
                    DPoint.y = atof(lpSpace++);
                    if (DoConvert)
	                    ConvertCoord(&DPoint,2,1);
                    if (!Pass || !FileIsDir)
                    {
    	                if (!PointInFileBounds (&DPoint,&MinMaxCoord,Pass))
	                        Store=FALSE; 
	                }
                    else 
                    {
	                    MNMXCORD	Bounds;

						DBoundsInit (&Bounds);
						AddDPointToMinMax (&DPoint,&Bounds);
	                    Store = PointInBounds (DPoint,&EditBounds);
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
				                sprintf (str,"%ld records skipped",NoFile);
			                 	SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,str);
			                }
	                    }
                    }        
                    Done = GetMIFCharacteristics (FidMIF,str,&lineno);
                 	GetDlgItemText(hWndDlg,IDC_SYMNAMEP,SymStuff,lnSymStuff);
					DecodePointSym (SymStuff,SymName,CSize,CRot,CColor);
					if (!SetPointSize (&size,CSize))
						goto ErrorEnd;
					if (!SetPointRot (&rot,CRot))
						goto ErrorEnd;
					if (!SetPointColor (&color,CColor))
						goto ErrorEnd;
					PointSym = GetOrCreateSym (SymName,&NumSyms,&hSymDesc,Create,2);
                    if (Store)
					    rtn=AddPointToMap (DPoint,NewRefno,NULL,PointSym,size,rot,NULL,hGRText,NULL,
					    					pPrefix,pUDI,color,color,-1,FALSE,HiPrecis,NULL,NULL);  
//                        AddPolyToMap (nPoly,pnPnts, phDpoints,0,NewRefno,3,251,0,pPrefix,pUDI,
//                                        MIFForeColor,MIFPenColor,MIFPenWidth,0,0,HiRes); 
                        
                 }
                 else if (!_fstrncmp(str,"LINE ",5))
                 {  
                    HANDLE  hDPoints;
                    HPDPOINT    lpDPoints;  
                    BOOL    Store=TRUE;  
                    LPSTR   lpSpace;  
                    short     i, nPnts;
                    
                    hDPoints = GSSiGlobAlloc ( 944,GMEM_MOVEABLE,2*sizeof(DPOINT));  
                    lpDPoints = (LPDPOINT)GlobalLock (hDPoints);
                    lpSpace = &str[5];
                    lpDPoints->x = atof(lpSpace++);
                    lpSpace = _fstrchr(lpSpace,' ');
                    lpDPoints++->y = atof(lpSpace++);
                    lpSpace = _fstrchr(lpSpace,' ');
                    lpDPoints->x = atof(lpSpace++);
                    lpSpace = _fstrchr(lpSpace,' ');
                    lpDPoints->y = atof(lpSpace); 
                    GlobalUnlock (hDPoints);
                    lpDPoints = (LPDPOINT)GlobalLock (hDPoints);
                    for (i=0;i<2;i++,lpDPoints++)
                    {
                        if (DoConvert)
	                        ConvertCoord(lpDPoints,2,1);
                        if (!PointInFileBounds (lpDPoints,&MinMaxCoord,Pass))
                            Store=FALSE; 
//Store=TRUE;                               
                    } 
                    GlobalUnlock (hDPoints); 
                    Done = GetMIFCharacteristics (FidMIF,str,&lineno);
                    nPnts = 2;
	                if (SendDlgItemMessage (hWndDlg,IDC_PRESERVE_MAPINFO,BM_GETCHECK,0,0))
	                {
	                	ForeColor = MIFForeColor;
	                	PenColor = MIFPenColor;
	                	PenWidth = MIFPenWidth;
	                }
	                else
	                {
	                	ForeColor = -1;
	                	PenColor = -1;
	                	PenWidth = 0;
	                }
                    if (Store)
                        AddPolyToMap (1,(int *)&nPnts, &hDPoints,1,NewRefno,NULL,-1,LineSym,0,pPrefix,pUDI,
                                      ForeColor,PenColor,PenWidth,0,0,0,0,HiRes); 
//                        MIFToTIGER (1,&nPnts, &hDPoints,&TLID); 
                        
                    GlobalFree (hDPoints); 
                 }
                 else if (!_fstrncmp(str,"PLINE",5))
                 { 
                    short     nPoly, nPoints,lastnpoints, i,ii;
                    long    nVertex=0; 
                    LPSHORT   pnPnts;    
                    LPHANDLE    phPoly;
                    HANDLE  hPoly, hPolys, hnPnts;
                    HPDPOINT    lpDPoint, lpDPoints;
                    DPOINT  LinkPoint;
                    LPSTR   lpSpace, pMult;
                    BOOL    Store, FirstPoly;
                    
                    if (!GetMIDData(FidMID,lpMIDstr,hDLT))
                    	goto ErrorEnd;
                    Store=TRUE;  
                    hPoly = 0; 
                    if (!(pMult = _fstrstr (str,"MULTIPLE")))
                    {
                        nPoly = 1;
                        nVertex = atoi(&str[6]);
                    }
                    else
                    {   
                    	pMult += 8;
                        nPoly = atoi(pMult); 
                    }  
                    hnPnts = GSSiGlobAlloc ( 945,GHND,nPoly*sizeof(short));
                    pnPnts = (LPSHORT)GlobalLock (hnPnts);
                    hPolys = GSSiGlobAlloc ( 946,GHND,nPoly*sizeof(HANDLE));
                    phPoly = (LPHANDLE)GlobalLock (hPolys);
                    for (i=0;i<nPoly;i++,pnPnts++,phPoly++)
                    {   
                    	if (i || !nVertex)
                    	{
		                    if (!fgetstring(str,256,FidMIF))
		                    	goto ErrorEnd;
		                    lineno++;
		                    nVertex = atoi(str);
                    	}                    
                        *pnPnts = nVertex;
                        *phPoly = GSSiGlobAlloc ( 947,GMEM_MOVEABLE,(DWORD)nVertex*sizeof(DPOINT));
                        lpDPoint = (LPDPOINT) GlobalLock (*phPoly); 
                        lpDPoints = lpDPoint;
                        while (nVertex--)
                        { 
                            if (!fgetstring(str,256,FidMIF))
                            	goto ErrorEnd;
                            lineno++; 
                            lpDPoint->x = atof (str);
                            if (!(lpSpace=_fstrchr(str,' ')))
                            	goto ErrorEnd;
                            lpDPoint->y = atof (lpSpace); 
                            if (DoConvert)
	                            ConvertCoord(lpDPoint,2,1);
                            if (!PointInFileBounds (lpDPoint,&MinMaxCoord,Pass))
                                Store=FALSE;  
Store=TRUE;                                  
                            lpDPoint++;
                        }  
                        GlobalUnlock (*phPoly);
                        if (i+1<nPoly) 
                        {
                            if (!fgetstring(str,256,FidMIF))
                            	goto ErrorEnd;
                            lineno++;
                            nVertex = atoi(str);
                        }
                    }  
                    GlobalUnlock (hPolys);   
                    Done = GetMIFCharacteristics (FidMIF,str,&lineno);
                    phPoly = (LPHANDLE)GlobalLock (hPolys);
                    GlobalUnlock (hnPnts);  
                    pnPnts = (LPHANDLE)GlobalLock (hnPnts);
                    if (Store)     
                        AddPolyToMap (nPoly,pnPnts, phPoly,1,NewRefno,NULL,3,LineSym,0,pPrefix,pUDI,
                                        MIFForeColor,MIFPenColor,MIFPenWidth,0,0,0,0,HiRes); 
//                    MIFToTIGER (nPoly,pnPnts, phPoly,&TLID); 
       
                    GlobalUnlock (hnPnts);  
                    GlobalUnlock (hPolys); 
                    phPoly = (LPHANDLE)GlobalLock (hPolys);  
                    for (i=0;i<nPoly;i++,phPoly++)
                        GlobalFree (*phPoly);   
                    GlobalFree (hPolys);  
                    GlobalFree (hnPnts);
                 }  
                 else if (!_fstrnicmp(str,"NONE",4))
                 {
                    if (!GetMIDData(FidMID,lpMIDstr,hDLT))
                    	goto ErrorEnd;
                    Done = GetMIFCharacteristics (FidMIF,str,&lineno);
                 }                  
                 else if (!_fstrnicmp(str,"TEXT",4))
                 {
					HANDLE		hGRText=0, hTranFile=0; 
					LPGRTEXT	lpGRText; 
					LPUMTEXTTPL	pTextTPL;
					HANDLE		hTPL=0;
					DPOINT		DPoints[2], DPoint;
                    HPDPOINT    pDPoints; 
                    HANDLE		hDPoints; 
                    BOOL    	Store=TRUE; 
                    double		TSize; 
                    short		i, nPnts, nchar; 
                    LPSTR		pBeg, pEnd; 
                    MNMXCORD	Bounds;

	                hGRText = GSSiGlobAlloc ( 948,GHND,sizeof(GRTEXT));
		            lpGRText = (LPGRTEXT)GlobalLock (hGRText);   
		            lpGRText->UltiMapStyle = 0;  
		            lpGRText->version = 1;    
		            lpGRText->length = sizeof(GRTEXT);   
                	lpGRText->FontNum = 0;  
                	lpGRText->hJust = 2;
	                lpGRText->vJust = 2; 
                    if (!fgetstring(str,300,FidMIF))
                    	goto ErrorEnd; 
                    pBeg = FirstNonBlank(str);
                    if (*pBeg == '"')
                    	pBeg++;
                    pEnd = LastChr (pBeg); 
                    if (*pEnd == '"')
                    	*pEnd = 0; 
                    REPLAC (pBeg,"\"\"","\"",_fstrlen (pBeg));
                    _fstrcpy (lpGRText->Text,pBeg);  
                    if (_fstrstr (pBeg,"Gregory"))
                    	ii=1;
                    nchar = lpGRText->ltext = _fstrlen (pBeg);
	                lpGRText->ltext += lpGRText->ltext % 2;
                    if (!fgetstring(str,256,FidMIF))
                    	goto ErrorEnd;
	                i = sscanf (str,"%Flf %Flf %Flf %Flf",&DPoints[0].x,&DPoints[0].y,&DPoints[1].x,&DPoints[1].y);
	                if (i != 4)   
	                	goto ErrorEnd; 
	                DPoint = MidPointD (DPoints[0],DPoints[1]); 
	                DPoint.x = DPoints[0].x;
	                Bounds.xmn = DPoints[0].x;
	                Bounds.ymn = DPoints[0].y; 
	                Bounds.xmx = DPoints[1].x;
	                Bounds.ymx = DPoints[1].y; 
                    Done = GetMIFCharacteristics (FidMIF,str,&lineno);
                    Done = GetMIFTextCharacteristics (FidMIF,str,&lineno,&rot);
	                TSize = GetTextSizeInBounds (nchar,rot,&Bounds);
                    if (!GetMIDData(FidMID,lpMIDstr,hDLT))
                    	goto ErrorEnd;
                    hDPoints = GSSiGlobAlloc ( 949,GMEM_MOVEABLE,2*sizeof(DPOINT));
                    pDPoints = (HPDPOINT)GlobalLock (hDPoints);
                    for (i=0;i<2;i++)
                    {   
                    	pDPoints[i] = DPoints[i];
                        if (DoConvert)
	                        ConvertCoord(&pDPoints[i],2,1);
                        if (!PointInFileBounds (&pDPoints[i],&MinMaxCoord,Pass))
                            Store=FALSE; 
                    }
                    GlobalUnlock (hDPoints); 
	                sprintf (lpGRText->cHeight,"%f",TSize); 
	                GlobalUnlock (hGRText);
                    nPnts = 2;
	                if (SendDlgItemMessage (hWndDlg,IDC_PRESERVE_MAPINFO,BM_GETCHECK,0,0))
	                {
	                	ForeColor = MIFForeColor;
	                	PenColor = MIFPenColor;
	                	PenWidth = MIFPenWidth;
	                }
	                else
	                {
	                	ForeColor = -1;
	                	PenColor = -1;
	                	PenWidth = 0;
	                }
                    if (Store)  
                    {
					    AddPointToMap (DPoint,NewRefno,NULL,TPointSym,TSize,rot,NULL,hGRText,NULL,
					    					pPrefix,pUDI,ForeColor,PenColor,-1,FALSE,HiRes,NULL,NULL);
//		                 if (Pass)
//		                 	 NewRefno = GetNewRefno (PltName,NULL,NULL,NULL,NULL);
//                        AddPolyToMap (1,(int *)&nPnts, &hDPoints,1,NewRefno,-1,TLineSym,0,pPrefix,pUDI,
//              							ForeColor,PenColor,PenWidth,0,0,0,0,HiRes); 
                    }
                    GlobalFree (hDPoints);
                    GSSiGlobUlFree (&hGRText); 
                 }                  
                 else if (!_fstrnicmp(str,"ELLIPSE",7))
                 {
                    if (!GetMIDData(FidMID,lpMIDstr,hDLT))
                    	goto ErrorEnd;
                    Done = GetMIFCharacteristics (FidMIF,str,&lineno); 
					if (GSSiMessageBox("Ellipse not supported",NULL,MB_OKCANCEL|MB_ICONEXCLAMATION) ==  IDCANCEL)
						goto ErrorEnd;
                 }                  
                 else   
                    goto ErrorEnd;
                 
                 CurLoc = GSSillseek (FidMIF,0,1);       
                 PctBox (GetDlgItem(hWndDlg,IDC_STATUS), TotLen, CurLoc,0);
                 
                 goto NextLine;
                 
        ErrorEnd: 
                 Pass = 2;
                 {  
                    char    mess[256];
                    
                    sprintf (mess,"Error in file %s \r\n at line %ld\r\n%s",Name,lineno,str);
                    MessageBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
                 }
                    
        EndFile: 
                 if (!Pass && ContinueProcessing)
                 {   
                     short NumPens=10;
                     PENDESC PenDesc[10];  
                     SYMBOL Symbol;
                         
                     for (i=0;i<NumPens;i++)
                     {
                        PenDesc[i].PenNum = i+1;
                        PenDesc[i].Width = (float)1.0; 
                        PenDesc[i].Style = 1;
                        PenDesc[i].Color = RGB(0,0,0);
                     }
                     CreateNewMap (PltName,&MinMaxCoord,NumSyms,hSymDesc,
                                                        NumPens,(LPPENDESC)&PenDesc,0,0,TRUE);
                     Pass=1;
                     goto NextPass;
                 }
//GSSiClose(FidTIGER1);
//GSSiClose(FidTIGER2); 
                 GSSiClose (FidMIF); 
                 if (FidMID!=HFILE_ERROR)
                    GSSiClose (FidMID);
                 GSSiGlobUlFree (&hStr); 
                 GlobalUnlock (hMIDstr);
                 GlobalFree (hMIDstr);
		         {
		         	DPOINT Dpoint;
					AddPointToMap (Dpoint,0,0,0,0,0,0,0,0,0,0,0,0,0,TRUE,TRUE,NULL,NULL);
				 }
				 CloseMap(TRUE);
   				 if (FileIsDir)
   				 { 
	                SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Inserting symbols");
			 	 	AddSymToDir (PltName,NumSyms,hSymDesc,0,NULL);
			 	 }  
			 	 else
			 	 	AddSymToMap (NumSyms,hSymDesc,0,NULL); 
                 DestroySymList (&NumSyms,&hSymDesc);
                 CloseRefIndex(TRUE);           
                 goto NextFile;
EndFiles:        
				 if (ContinueProcessing)
 				 	SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Load complete");
 				 else
 				 	SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Load cancelled");
                 FileProjectionType = SaveFPT; 
                 DisableHalt = FALSE;
                 ForceRefIndex = ForceTAGIndex = FALSE; 
                 Processing = FALSE;
                 ContinueProcessing = TRUE;  
				 DestroyFieldList (); 
				 DestroyAdvancedOpts();
				 GSSiGlobFree (&hDLT);
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
BOOL GetMIDData(HFILE FidMID,LPSTR lpMIDstr,HANDLE hDLT) 
{   
    LPSTR   lpStart, lpEnd, lpChar;
    char    EndChar; 
    short     i;
    
    if (FidMID==HFILE_ERROR) return TRUE;
    if (!fgetstring(lpMIDstr,2040,FidMID))
        return TRUE; 
    GetDelimTextData(lpMIDstr,hDLT);
    return TRUE;
} 

COLORREF AutoYellow (COLORREF color)
{   
	COLORREF AutoOrthoColor;

	if (CurView && CurView->HaveOrthos && !color)
	{
		AutoOrthoColor = GetGlobalLVal2 ("[%AUTO_ORTH_COLOR]",RGB(255,255,0));
		AutoOpaque = GetGlobalBVal2 ("[%AUTO_ORTH_OPAQUE]",FALSE);
		return AutoOrthoColor;   
	}
	AutoOpaque = FALSE;
	return color;
}


COLORREF ConvertColor (COLORREF Color,short UseHalfTone)
{   
	//UseHalfTone <0 always half tone, = 0 never halftone, > 0 halftone if Symnum=UseHalfTone in viewport halftone vis 
	
	if (Color < 0)
		goto Exit; 
	Color = AutoYellow (Color); 
	if (!ForceHalfTone)
	{
		if (!UseHalfTone)
			goto Exit;
		if (UseHalfTone > 0)
		{
			if (!GetHalfToneVisibility (UseHalfTone))
				goto Exit;
		} 
	}
	if (CurView && CurView->HalfTone)
	{
		short	R=GetRValue (Color);
		short	G=GetGValue (Color);
		short	B=GetBValue (Color);  
        double	HT = ((double)CurView->HalfTone*10)/100;
        
		Color = RGB(R+(255-R)*HT,G+(255-G)*HT,B+(255-B)*HT);
	}
	if (CurView && CurView->ConvertToGray) 
		Color = ConvertToGray (Color);
Exit:
	return Color;
}

BOOL LoadTIN (LPSTR FromFile, LPSTR ToPlt)
{   
	HANDLE	hSymDesc=0;
	short	NumSyms=0, TINSym, n;  
	char	str[260];   
	HFILE	FidTIN;
	OFSTRUCT	OFStruct;  
	long	TotLen, CurLoc,PointID[4];  
	double	Elev;
	DPOINT	Point;
	HPDPOINT3D	pPoints;
	WORD	NumPoints=0; 
	HANDLE	hPoints;  
	MNMXCORD	Bounds;
  	long	Ref=1;  
  	BOOL	rtn;
    
    if ((FidTIN = GSSiOpenFile(FromFile,&OFStruct,OF_READ)) == HFILE_ERROR)
    	return FALSE;
//    GetGlobalCVal ("[%LINKMAP]",Name,"testlink.plt");
	CreateStatusWindow (hWndMain,1,NULL);
	DBoundsInit (&Bounds);
    TotLen = GSSillseek (FidTIN,0,2); 
    GSSillseek (FidTIN,0,0);   
	StatusWindowUpdate (OFStruct.szPathName,"Scanning for Min/Max",TotLen,0);
    while (fgetstring (str,256,FidTIN) && ContinueProcessing) 
    { 
    	n = sscanf (str,"%ld %Flf %Flf %Flf",PointID,&Point.x,&Point.y,&Elev);
    	Point.x *= FTM;
    	Point.y *= FTM;
    	AddDPointToMinMax (&Point,&Bounds);
		CurLoc = GSSillseek (FidTIN,0,1);
			StatusWindowUpdate (NULL,NULL,TotLen,CurLoc);
    }
    CreateNewMap (ToPlt,&Bounds,0,NULL,0,NULL,0,0,TRUE);
    GSSillseek (FidTIN,0,0);   
    hPoints = GSSiGlobAlloc ( 950,GMEM_MOVEABLE,4*sizeof(DPOINT3D));    
    TINSym =  GetDictSymbolNumber ("TINTRIANGLE");
	AddToSymList (TINSym,&NumSyms,&hSymDesc); 
	StatusWindowUpdate (NULL,"Loading Areas",TotLen,0);
    while (fgetstring (str,256,FidTIN) && ContinueProcessing)
    {
    	short   rtn;
		        
		pPoints = (HPDPOINT3D)GlobalLock (hPoints); 
		pPoints += NumPoints; 
    	n = sscanf (str,"%ld %Flf %Flf %Flf",&PointID[NumPoints],&pPoints->x,&pPoints->y,&pPoints->z);     
    	if (PointID[NumPoints] != 13 && PointID[NumPoints] != 14)
    		continue;
    	pPoints->x *= FTM;
    	pPoints->y *= FTM;
    	pPoints->z *= FTM;
    	GlobalUnlock (hPoints); 
    	NumPoints++;
    	if (NumPoints == 4)
    	{   
    		short	i=NumPoints--;
    		long	id = PointID[0];
    		
    		while (i--)
    			if (PointID[i] != id)
    				id = 0;
    		
			rtn=AddPolyToMap (1,&NumPoints, &hPoints,0,Ref++,0,-1,TINSym,NULL,NULL,NULL,-1,-1,-1,0,0,0,NULL,2); 
			NumPoints = 0;   
			CurLoc = GSSillseek (FidTIN,0,1);
			StatusWindowUpdate (NULL,NULL,TotLen,CurLoc);
		}
    }
    rtn = ContinueProcessing;
    ContinueProcessing = TRUE;
    GSSiClose (FidTIN);
	GSSiGlobFree (&hPoints);
    CloseMap(TRUE);  
	AddSymToMap (NumSyms,hSymDesc,0,NULL); 
    DestroySymList (&NumSyms,&hSymDesc);
	DestroyStatusWindow(0);  
	return rtn;
}

BOOL CreateVideoPolys (LPSTR InFile,LPSTR PltFile)
{
	char	str[260], TrackID[66];  
	MNMXCORD	Bounds;
	OFSTRUCT	OFStruct;
	HFILE	Fid, Fid2; 
	HANDLE	hDLT=0, hDLT2=0;   
	short	err;
	HDIB	hDib;
	
//	hDib = FrameToDIB2 ("c:\\mgvvid\\V0500559\\5413.avi", 10000);
     
    if ((Fid = GSSiOpenFile (InFile,&OFStruct,OF_READ)) == HFILE_ERROR)
    	return FALSE;
    GetGlobalCVal ("[%PROJECTBOUNDS]",str,NULL); 
    Bounds = atobounds (str,&err); 
    _fstrcpy (PltName,PltFile);
    CreateNewMap (PltFile,&Bounds,0,NULL,0,NULL,0,0,TRUE); 
    fgetstring (str,256,Fid);
	ProcessDelimTextHeader(str,NULL,Fid,&hDLT); 
 	       
    while (fgetstring (str,256,Fid))
    {
		GetDelimTextData(str,hDLT);
	    if ((Fid2 = GSSiOpenFile ("[FULLNAME]",&OFStruct,OF_READ)) != HFILE_ERROR)
	    {
			HANDLE	hSymDesc=0; 
			LPSTR	pEnd;
			short	NumSyms=0, SymNum=GetDictSymbolNumber("VIDEOTRACK");  
	    	short	rtn;
	    	long	Refno;
	    	USHORT	nPoints=0;
	    	HANDLE	hPoints=GSSiGlobAlloc ( 951,GMEM_MOVEABLE,(long)sizeof(DPOINT)*UINT_MAX); 
			HPDPOINT	pPoints = (HPDPOINT)GlobalLock (hPoints);    

		    fgetstring (str,256,Fid2);
				        
			ProcessDelimTextHeader(str,NULL,Fid,&hDLT2); 
		    while (fgetstring (str,256,Fid2))
		    {
				GetDelimTextData(str,hDLT2);
				pPoints[nPoints].y = GetGlobalDVal ("[X]");
				pPoints[nPoints].x = GetGlobalDVal ("[Y]");   
				ConvertCoord (&pPoints[nPoints],2,1);
				nPoints++;
		    }
		    GSSiClose (Fid2); 
		    GlobalUnlock (hPoints);
			AddToSymList (SymNum,&NumSyms,&hSymDesc); 
			Refno = GetNewRefno(PltName,NULL,NULL,NULL,NULL);  
			_fstrcpy (str,"[DIRECTORY]");
			ExpandText (str);
			pEnd = _fstrrchr (str,'\\');
			*pEnd = 0;
			pEnd = _fstrrchr (str,'\\');
			pEnd++;
			_fstrcpy (TrackID,pEnd);
			_fstrcat (TrackID,"\\");
			_fstrcat (TrackID,"[FILENAME]");
			ExpandText (TrackID);
			rtn=AddPolyToMap (1,&nPoints, &hPoints,1,Refno,NULL,2,SymNum,NULL,"VIDTRACK",TrackID,-1,-1,-1,0,0,0,0,TRUE);
			GSSiGlobFree (&hPoints);
		    CloseMap(TRUE);  
			AddSymToMap (NumSyms,hSymDesc,0,NULL); 
		    DestroySymList (&NumSyms,&hSymDesc);
		}
    } 
    GSSiClose (Fid);
    return TRUE;
}
  
BOOL OpenChronoIndex (LPSTR PName,LPMNMXCORD pMinMaxCoord)
{
	static	char	Drive[8], Dir[144], FullName[144];
    LPFILEINDEX lpIndex;
 	HANDLE	handle;    
 	LPSTR	pChronoDir, pPar, pName;
 	BOOL	rtn=FALSE;
 	LPLONG	pDate;  
 	LPUSHORT	pFileNo;
	
	GSSiGlobFree (&hChronoIndex);
	DBoundsInit (pMinMaxCoord); 
	_fstrcpy (Dir,PName);
	ExpandText (Dir);
	_fullpath (FullName,Dir,sizeof(FullName));
	_splitpath (FullName,Drive,Dir,NULL,NULL); 
	sprintf (FullName,"%s%sindex",Drive,Dir);
   	IgnoreBounds=TRUE;
	handle = OpenMapIndex (FullName,NULL);
	if (!handle)
		goto Exit; 
	rtn = TRUE;
	nChronoFiles = 0;
	hChronoIndex = GSSiGlobAlloc ( 952,GHND,UINT_MAX);
	pChronoDir = GlobalLock (hChronoIndex);
	sprintf (pChronoDir,"%s%s",Drive,Dir);
	
	lpIndex = (LPFILEINDEX)GlobalLock (handle); 
NextFile:
	CloseMap(FALSE);
	pChronoDir = _fstrchr (pChronoDir,0);  
	pChronoDir++;
    if (lpIndex->FileInIndex >= (long)lpIndex->NumFiles)
    {   
        if (!(lpIndex=GetNextIndexHeader(&handle,TRUE)))
        	goto Exit;
        goto NextFile;
    }
    GetNextIndexEntry(lpIndex); 
    if (*lpIndex->CurrentEntry->Name == '\\')
    	pName = lpIndex->CurrentEntry->Name + 1;
    else
    	pName = lpIndex->CurrentEntry->Name;
	sprintf (PltName,"%s%s%s",Drive,Dir,pName);  
	if ((pPar = _fstrrchr (PltName,'(')))
		*pPar = 0;
 	if (!OpenMap (0,CurView->hDC))
 		goto NextFile;
	AddMinMaxD (pMinMaxCoord, &CurView->FileMNMX);
	pDate = (LPLONG)pChronoDir;
	*pDate++ = MinFileTime;
	*pDate++ = MaxFileTime; 
	pFileNo = (LPUSHORT)pDate;
	*pFileNo++ = lpIndex->FileInIndex; 
	pChronoDir = (LPSTR)pFileNo;
	_fstrcpy (pChronoDir,pName);
	if ((pPar = _fstrrchr (pChronoDir,'(')))
		*pPar = 0;
	nChronoFiles++;
	goto NextFile;
		
Exit:      
	if (hChronoIndex)
		GlobalUnlock (hChronoIndex);
	IgnoreBounds = FALSE;
	return rtn;	
}

void CloseChronoIndex (void)
{
	GSSiGlobFree (&hChronoIndex);
	return;
}

BOOL GetChronoIndexedEditFile (LPSTR PName,long BegTime, long EndTime)
{   
	char	PltName[128];
 	BOOL	rtn=FALSE;
 	LPSTR	pIndex, pChronoIndex;  
 	long	n=nChronoFiles, FTime, TTime; 
 	LPLONG	pDate; 
 	long	DateRange, CurDateRange=LONG_MAX;   
 	USHORT	FileInIdx;
 	LPUSHORT	pFileNo;
	
	if (!hChronoIndex)
		return FALSE;
	pIndex = pChronoIndex = GlobalLock (hChronoIndex);
	while (n--) 
	{
		pChronoIndex = _fstrchr (pChronoIndex,0);
		pChronoIndex++; 
		pDate = (LPLONG)pChronoIndex;
		FTime = *pDate++;
		TTime = *pDate++; 
		pFileNo = (LPUSHORT)pDate;
		FileInIdx = *pFileNo++;
		pChronoIndex = (LPSTR)pFileNo;
		if (BegTime >= FTime && BegTime <= TTime &&
			EndTime >= FTime && EndTime <= TTime)
		{
			rtn = TRUE;
			DateRange = TTime - FTime; 
			if (DateRange < CurDateRange)
			{
				CurDateRange = DateRange;
				sprintf (PltName,"%s%s",pIndex,pChronoIndex); 
                FileInIndex = FileInIdx;
			}
		} 
	} 
	if (rtn)
		if (_fstricmp (PName,PltName))
		{
		    DPOINT Dpoint;
		
			AddPointToMap (Dpoint,0,0,0,0,0,0,0,0,0,0,0,0,0,TRUE,FALSE,NULL,NULL);
			CloseMap (TRUE);
			_fstrcpy (PName,PltName);
		}
	GlobalUnlock (hChronoIndex);
	return rtn;
}

BOOL AddSymToDir (LPSTR DirName,short NumSyms,HANDLE hSymDesc,short NumPens,LPPENDESC pPenDesc)
{
	char	Drive[8], Dir[128], FullName[144];
    LPFILEINDEX lpIndex;
 	HANDLE	handle;
 	BOOL	rtn=FALSE; 
   	HCURSOR	OldCursor; 
   	LPSTR	pName, pPar;
	                	
    OldCursor = GSSiSetCursor (LoadCursor (0,IDC_WAIT));
	_fullpath (FullName,DirName,sizeof(FullName));
	_splitpath (FullName,Drive,Dir,NULL,NULL); 
	sprintf (FullName,"%s%sindex",Drive,Dir);
	
   	IgnoreBounds=TRUE;
	handle = OpenMapIndex (FullName,NULL);
	if (!handle)
		goto Exit;
	lpIndex = (LPFILEINDEX)GlobalLock (handle); 
NextFile:
    if (lpIndex->FileInIndex >= (long)lpIndex->NumFiles)
    {   
        if (!(lpIndex=GetNextIndexHeader(&handle,TRUE)))
        	goto Exit;
        goto NextFile;
    }
    GetNextIndexEntry(lpIndex);     
    pName = lpIndex->CurrentEntry->Name;
    if (*pName == '\\')
    	pName++;
	sprintf (PltName,"%s%s%s",Drive,Dir,pName); 
	if ((pPar = _fstrrchr (PltName,'('))) 
		*pPar = 0;
	rtn=AddSymToMap (NumSyms,hSymDesc,NumPens,pPenDesc);
	goto NextFile;
	
Exit:
    GSSiSetCursor (OldCursor);
	IgnoreBounds = FALSE;
	return rtn;	
}

HANDLE ReversePoints (long NumPoints,HANDLE hPoints)
{
	HANDLE		hNewPoints;
	HPDPOINT	pNewPoint;
	HPDPOINT	pPoint;
	
	if (!NumPoints)
		return 0;  
	hNewPoints = GSSiGlobAlloc ( 953,GMEM_MOVEABLE,NumPoints * sizeof(DPOINT));
	pNewPoint = (HPDPOINT)GlobalLock (hNewPoints); 
	pPoint = (HPDPOINT)GlobalLock (hPoints);
	pPoint += (NumPoints-1);
	while (NumPoints--)
		*pNewPoint++ = *pPoint--;
	GlobalUnlock (hNewPoints);
	GSSiGlobUlFree (&hPoints);
	return hNewPoints;
}

HANDLE ReversePoints3D (long NumPoints,HANDLE hPoints)
{
	HANDLE		hNewPoints;
	HPDPOINT3D	pNewPoint;
	HPDPOINT3D	pPoint;
	
	if (!NumPoints)
		return 0;  
	hNewPoints = GSSiGlobAlloc ( 953,GMEM_MOVEABLE,NumPoints * sizeof(DPOINT3D));
	pNewPoint = (HPDPOINT3D)GlobalLock (hNewPoints); 
	pPoint = (HPDPOINT3D)GlobalLock (hPoints);
	pPoint += (NumPoints-1);
	while (NumPoints--)
		*pNewPoint++ = *pPoint--;
	GlobalUnlock (hNewPoints);
	GSSiGlobUlFree (&hPoints);
	return hNewPoints;
}

void ReversePoints2 (long NumPoints,HPDPOINT pPoint)
{
	long	i;
	
	if (!NumPoints)
		return;  
	{
		HANDLE hNewPoints=GSSiGlobAlloc ( 954,GMEM_MOVEABLE,NumPoints * sizeof(DPOINT));
		HPDPOINT	pNewPoint = (HPDPOINT)GlobalLock (hNewPoints); \
		
		for (i=0;i<NumPoints;i++)
			pNewPoint[i] = pPoint[NumPoints - i - 1];
		for (i=0;i<NumPoints;i++)
			pPoint[i] = pNewPoint[i];
		GSSiGlobUlFree (&hNewPoints); 
	}
	return;
}

HANDLE ReversePoints3 (long NumPoints,HPDPOINT pPoint)
{
	HANDLE		hNewPoints;
	HPDPOINT	pNewPoint;
	
	if (!NumPoints)
		return 0;  
	hNewPoints = GSSiGlobAlloc ( 953,GMEM_MOVEABLE,NumPoints * sizeof(DPOINT));
	pNewPoint = (HPDPOINT)GlobalLock (hNewPoints); 
	pPoint += (NumPoints-1);
	while (NumPoints--)
		*pNewPoint++ = *pPoint--;
	GlobalUnlock (hNewPoints);
	return hNewPoints;
}

BOOL FAR PASCAL LOADFLOODMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 
    short     TabStops[2]={65,1300}, i;
    char    Ext[8], ExtID[34], str[260];
    LPSTR    lpstr=str;  
	HCURSOR	hcurSave;
    static   HANDLE hSQL=0;
    long        TotLen, CurLoc, County, NumCounties;
    LPLONG      pCounty, pCnty;  
    static      short     FileType = 0;
	char	VolLabel[16],CurDir[128];
   	OFSTRUCT	OFStruct;
   	
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:
         
         SendDlgItemMessage (hWndDlg,IDC_COUNTY_LIST,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
         
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
            case IDC_DRIVE_LIST:
               { 
              	switch(HIWORD(lParam))
                {
		             case LBN_SELCHANGE:
		             {
		             	char	drive[8]="j",file[34]; 
		             	int		index=0, SaveDrive, Choice;
		             	HFILE	Fid; 
		             	LPSTR	lpDrive;
		             	
					    hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
				        Choice=SendDlgItemMessage(hWndDlg,IDC_DRIVE_LIST,CB_GETCURSEL,NULL,NULL);  
				        SendDlgItemMessage(hWndDlg,IDC_DRIVE_LIST,CB_GETLBTEXT,Choice,(LPARAM)VolLabel); 
				        lpDrive = _fstrstr (VolLabel,":)");
				        *lpDrive--=0;
			           	_getcwd (CurDir,128);
		           	  	SaveDrive = _getdrive();
		             	sprintf (str,"%s:\\dlg\\*.dlg",lpDrive);  
		         		SendDlgItemMessage (hWndDlg,IDC_COUNTY_LIST,LB_RESETCONTENT,0,0);
                     	DlgDirList (hWndDlg,str,IDC_COUNTY_LIST,IDC_DIR,DDL_READWRITE);
				   	    _chdir (CurDir);
	           	  	    _chdrive (SaveDrive);
                     	while (SendDlgItemMessage (hWndDlg,IDC_COUNTY_LIST,LB_GETTEXT,index,(LPARAM)file)!=
                     			LB_ERR)
                     	{   
		           	  	    GetDlgItemText (hWndDlg,IDC_DIR,CurDir,128);  
                     		sprintf (str,"%s\\%s",CurDir,file); 
                     		Fid = GSSiOpenFile (str,&OFStruct,OF_READ); 
                     		fgetstring (str,128,Fid);
                     		GSSiClose (Fid); 
                     		str[59]=0;
                     		sprintf (CurDir,"%s\t%s",file,str);
                     		SendDlgItemMessage (hWndDlg,IDC_COUNTY_LIST,LB_DELETESTRING,index,0);	
                     		SendDlgItemMessage (hWndDlg,IDC_COUNTY_LIST,LB_INSERTSTRING,index++,(LPARAM)CurDir);	
                     	} 
                     	GSSiSetCursor (hcurSave);
                     			
                	 }
		             	break;
		             	
	                 case CBN_DROPDOWN:
	                 {
		                BOOL    HaveMID=FALSE;
		                LPTAGDEF    lpTAGDef; 
		                short     nFields,ii,itype; 
		                LPSTR   lpSpace, lpDelim; 
						UINT	PrevErrMode;
						
						PrevErrMode = SetErrorMode(SEM_NOOPENFILEERRORBOX|SEM_FAILCRITICALERRORS);

		                SendDlgItemMessage (hWndDlg,IDC_DRIVE_LIST,CB_RESETCONTENT,0,0);
						for (i=3;i<26;i++)
						{   
							ii=GetDriveType (i);
							if ((itype=GetDriveType (i)) == DRIVE_REMOTE)
							{                 
								if (GetVolumeLabel(i,VolLabel))
								{
				                    sprintf (str,"%s (%c:)",VolLabel,(char)('A'+i));
				                    SendDlgItemMessage (hWndDlg,IDC_DRIVE_LIST,CB_ADDSTRING,0,(LPARAM)str); 
				                } 
				            }
						}
						SetErrorMode (PrevErrMode);
					}
				    break; 
				}
                break;
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
                 
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 ForceRefIndex = ForceTAGIndex = FALSE;
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
                 short  st, SymNum, iUDI=0, AreaSym, LineSym, Pass;
                 BOOL   Done; 
                 HANDLE hMIDstr;
                 char   Prefix[10], UDI[34], AreaSymName[10], LineSymName[10], FloodInfoDB[128]; 
                 LPSTR  lpComma;  
                 OFSTRUCT   OFStruct;
                 long   filecode,filelength,ii=100, NumOverLimit;  
                 HFILE  FidBNA, FidCvt;
                 char   mess[256];  
                 short  nPoly, nPoints,lastnpoints, Symbol, MaxSides;
                 long   nVertex, AreaRef=0;
                 DPOINT	LinkPoint, FirstIslandPoint, ControlPt;
                 HANDLE hPoly;
                 HPDPOINT pPoints;
                 HANDLE     hPoints, hnPoints, hLines;
                 short        NumPoints, Nump;  
                 short    NumSyms=0, n;  
                 HANDLE hSymDesc=0; 
                 BOOL   Store,WantFirstPoint;   
                 char   LSym[20], Name[128], SaveAlt[64], Zone[64], Parent[64], Code[16];
                 LPSTR  lpTab;
				 long nFiles, iFile=0, AreaNum=0, NumAreas;
				 HANDLE hItems, hNumPoints, hBT;  
				 LPSHORT   lpItems; 
				 short	UTMZone, NumAttributes,NumLinesInArea,nRec, NumIslands,ipoly;
				 long	NumLines,Loc,LineID,NodeLoc,NextAreaLoc,Dummy;
				 HANDLE	hLineIndex;   
				 LPHANDLE	phPoints;
				 LPLONG	pLineIndex;
				 LPSHORT	pLineID;
				 UINT	*pNumPoints; 
				 short	cond,AttID, Major, Minor, LineIDLoc, Major12, Minor12, Major13, Minor13, Major14, Minor14;
			     LPOPENFILEDATA  FilePtr;
			     LPOPENSQLDATA   SQLPtr;  
			     char	SymName[34]; 
			     HANDLE	hSymbol; 
			     short	FEMACode[100], GMSym[100], nCodes=0;
				 LPSYMBOL	CurSymbol; 
                                          
                                        
                 if (!GetDlgItemText (hWndDlg,IDC_DESTMAP,PltName,128))
                 {  
                    
                    MessageBox(GetFocus(),"No destination map", 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }
                 PltType = 2;  
                _fstrcpy (FloodInfoDB,"[%INDIR]FLOOD.TXT");
                ExpandText (FloodInfoDB);
                st = OpenDataFile (FloodInfoDB,"",BT_READ,&hSQL);
                if (!st)
                {  
                    MessageBox(GetFocus(),"Cannot open ODBC|TXT|[%INDIR]flood.txt", 0,MB_ICONQUESTION|MB_OK);
                    break;
                }
                SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
                FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
				
				OpenSymDict (OF_READWRITE);
				_fstrcpy (SymName,"FEMAFLOOD");	                	
             	SymNum = GetDictSymbolNumber (SymName); 
				if (!SymNum)
				{
					hSymbol = AllocateNewSymbol ();
					CurSymbol = (LPSYMBOL)GlobalLock (hSymbol);
			        CurSymbol->Type = 0;         
					_fstrcpy (CurSymbol->Name,SymName);  
					_fstrcpy (CurSymbol->Desc,"FEMA Flood Zones");
					CurSymbol->Parent=GetDictSymbolNumber("ALL");
					GlobalUnlock (hSymbol);		 
					SymNum = SaveSymbol(hSymbol,0);   
					DestroySymbol (hSymbol); 
				}					                 	
				hBT = CreateUniqueList (32,NULL);   
				GetODBCUniqueFieldValues (FilePtr->FileHandle,"SYMBOL IS NOT NULL","PARENT",32,hBT); 
				GlobalUnlock (SQLPtr->OFHandle);
				GlobalUnlock (hSQL);
                CloseDataFile (TRUE, &hSQL);  
				cond = BT_FIRST;
				while (!BT_FIND (hBT,(LPSTR)SymName,cond,BT_ANY,(LPSTR)&Dummy))
				{  
					cond = BT_NEXT;   
					Truncate (SymName); 
					if (*SymName)
					{
	                 	SymNum = GetDictSymbolNumber (SymName); 
						if (!SymNum)
						{   
							hSymbol = AllocateNewSymbol ();
							CurSymbol = (LPSYMBOL)GlobalLock (hSymbol);
					        CurSymbol->Type = 0;         
							_fstrcpy (CurSymbol->Name,SymName);  
							_fstrcpy (CurSymbol->Desc,"");
							CurSymbol->Parent=GetDictSymbolNumber("FEMAFLOOD");
							GlobalUnlock (hSymbol);		 
							SymNum = SaveSymbol(hSymbol,0);   
							DestroySymbol (hSymbol); 
						}
					}					                 	
                }
                BT_CLOSEANDDELETE (&hBT);
                st = OpenDataFile (FloodInfoDB,"",BT_READ,&hSQL);
                SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
                FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
				hBT = CreateUniqueList (32,NULL);   
				GetODBCUniqueFieldValues (FilePtr->FileHandle,"SYMBOL IS NOT NULL","SYMBOL",32,hBT); 
				GlobalUnlock (hSQL);
                CloseDataFile (TRUE, &hSQL);  
                st = OpenDataFile (FloodInfoDB,"SYMBOL = [SYMNAME]",BT_READ,&hSQL);
				cond = BT_FIRST;
				while (!BT_FIND (hBT,(LPSTR)SymName,cond,BT_ANY,(LPSTR)&Dummy))
				{  
					cond = BT_NEXT;   
					Truncate (SymName);
					if (*SymName)
					{
	                 	SymNum = GetDictSymbolNumber (SymName); 
						SetGlobalValue ("SYMNAME",SymName);
						_fstrcpy (Zone,"[ZONE]");
						ExpandText (Zone);
						_fstrcpy (Code,"[Code]");
						ExpandText (Code);
						if (!SymNum)
						{   
							_fstrcpy (Parent,"[PARENT]");
							ExpandText (Parent);
							hSymbol = AllocateNewSymbol ();
							CurSymbol = (LPSYMBOL)GlobalLock (hSymbol);
					        CurSymbol->Type = 3;         
							_fstrcpy (CurSymbol->Name,SymName);  
							_fstrcpy (CurSymbol->Desc,"");
							CurSymbol->Parent=GetDictSymbolNumber(Parent);
							GlobalUnlock (hSymbol);		 
							SymNum = SaveSymbol(hSymbol,0); 
							DestroySymbol (hSymbol); 
						}					                 	
						FEMACode[nCodes]=atoi(Code);
						GMSym[nCodes++] = SymNum;  
	                 	AddToSymList (SymNum,&NumSyms,&hSymDesc); 
	                 } 
                 }
                 BT_CLOSEANDDELETE (&hBT);
				 CloseSymDict();
				 GlobalUnlock (SQLPtr->OFHandle);
				 GlobalUnlock (hSQL);
                 CloseDataFile (TRUE, &hSQL);  
                 
                 
                 nFiles=SendDlgItemMessage(hWndDlg,IDC_COUNTY_LIST,LB_GETSELCOUNT,0,0);
                 if (!nFiles)
                 {
                    MessageBox( GetFocus(), "No Counties Selected",0, MB_OK);
                    break;
                 } 
                 hItems = GSSiGlobAlloc ( 955,GMEM_MOVEABLE,nFiles*2);
                 lpItems = (LPSHORT) GlobalLock (hItems); 
                 SendDlgItemMessage(hWndDlg,IDC_COUNTY_LIST,LB_GETSELITEMS,(WPARAM)nFiles,(LPARAM)lpItems); 
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
                 
       	  	     GetDlgItemText (hWndDlg,IDC_DIR,CurDir,128);  
                 SaveFPT = FileProjectionType; 
				 FileProjectionType=0;   
				 Processing = TRUE;
                 DisableHalt = TRUE;  
NextFile:        
				 NumOverLimit = MaxSides = 0;  
				 
                 SendDlgItemMessage (hWndDlg,IDC_COUNTY_LIST,LB_GETTEXT,*lpItems,(LPARAM)str);
                 lpTab = _fstrchr (str,'\t');
                 *lpTab = 0; 
                 sprintf (Name,"%s\\%s",CurDir,str); 
//        _fstrcpy (Name,"c:\\mintiger\\nwi\\beauli.dlg");
                 FidBNA = GSSiOpenFile (Name,&OFStruct,OF_READ); 
                 i=4;
                 while (i--)
                 	fgetstring (str,80,FidBNA);
				 UTMZone = atoi (&str[14]);                 
                 FidCvt = GSSiOpenFile ("fema.cvt",&OFStruct,OF_CREATE); 
                 fputstring ("1",FidCvt);
                 ltoa (UTMZone,str,10);
                 fputstring (str,FidCvt);
                 fputstring ("0",FidCvt);
                 fputstring ("1.0",FidCvt);
                 for (i=0;i<14;i++)
                	fputstring ("0.0",FidCvt);
                 GSSiClose (FidCvt);   
		         ConvertCoordClose (); 
                 _fstrcpy (SaveAlt,"[%ALT_PROJECTION]");
                 ExpandText (SaveAlt);
                 _fstrcpy (str,"[%ALT_PROJECTION]=FEMA");
	             ExpandText (str);
	             i = 6;
                 while (i--)
                 	fgetstring (str,80,FidBNA);
                MinMaxCoord.xmn = DBL_MAX;
                MinMaxCoord.xmx = -DBL_MAX; 
                MinMaxCoord.ymn = DBL_MAX;
                MinMaxCoord.ymx = -DBL_MAX; 
                i=4;
                while (i--)
                {
	              	fgetstring (str,80,FidBNA);
	              	ControlPt.x = atof (&str[36]);
	              	ControlPt.y = atof (&str[49]);
	            	ConvertCoord(&ControlPt,3,1);
		            MinMaxCoord.xmn = min (MinMaxCoord.xmn,ControlPt.x);
		            MinMaxCoord.xmx = max (MinMaxCoord.xmx,ControlPt.x);
		            MinMaxCoord.ymn = min (MinMaxCoord.ymn,ControlPt.y);
		            MinMaxCoord.ymx = max (MinMaxCoord.ymx,ControlPt.y);
		        }
                {
                    short NumPens=10;
                    PENDESC PenDesc[10];
                    
                    SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Loading Data");
                    for (i=0;i<NumPens;i++)
                    {
                        PenDesc[i].PenNum = i+1;
                        PenDesc[i].Width = (float)1.0; 
                        PenDesc[i].Style = 1;
                        PenDesc[i].Color = RGB(0,0,0);
                    }
                    CreateNewMap (PltName,&MinMaxCoord,NumSyms,hSymDesc,
                                                       NumPens,(LPPENDESC)&PenDesc,0,0,TRUE); 
					EditBounds = CurView->FileMNMX;
                } 
              	fgetstring (str,80,FidBNA);
              	NumLines = atol (&str[56]);
              	NumAreas = atol (&str[46]);
// build the line index
				hLineIndex = GSSiGlobAlloc ( 956,GMEM_MOVEABLE,4*(NumLines+1));
              	fgetstring (str,80,FidBNA);
				NodeLoc = GSSillseek (FidBNA,0,1);
                SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Building line index");
				
			    hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
NextLine:       
				while (*str != 'L') 
				{
					Loc = GSSillseek (FidBNA,0,1);
	              	if (!fgetstring (str,80,FidBNA))
	              		goto GotLines;
	            }
				pLineIndex = (LPLONG)GlobalLock (hLineIndex); 
				LineID = atol (&str[1]);
				pLineIndex+=LineID;
				*pLineIndex = Loc;
				GlobalUnlock (hLineIndex);
				Loc = GSSillseek (FidBNA,0,1);
              	if (fgetstring (str,80,FidBNA))
              		goto NextLine;
		GotLines:
				GlobalUnlock (hLineIndex);					         
                GSSillseek (FidBNA,NodeLoc,0);
                lineno = 0;
                *str = 0;
				while (*str != 'A') 
				{
					NextAreaLoc = GSSillseek (FidBNA,0,1);
	              	if (!fgetstring (str,80,FidBNA))
	              		goto EndFile;
	            } 
				GSSiSetCursor (hcurSave);
                SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Loading areas");
	  	NextArea: 
	  			GSSillseek (FidBNA,NextAreaLoc,0);
              	fgetstring (str,80,FidBNA); 
              	if (*str == 'L')
              		goto EndFile;
                if (!ContinueProcessing)
                    goto EndFile;
                NumLinesInArea = atoi (&str[36]);  
                NumAttributes = atoi (&str[48]);   
                NumIslands = atoi (&str[60]);  
                nPoly = NumIslands+1;
                hNumPoints = GSSiGlobAlloc ( 957,GMEM_MOVEABLE,2*nPoly);
                hLines = GSSiGlobAlloc ( 958,GHND,2*(long)(1+NumLinesInArea)); 
                pLineID = (LPSHORT)GlobalLock (hLines);
                nRec = (NumLinesInArea-1)/12 +1; 
                n = NumLinesInArea;
                while (nRec--) 
                {
	              	fgetstring (str,80,FidBNA); 
                	LineIDLoc = 0;
                	while (n && LineIDLoc < 72)
                	{
                		*pLineID++ = atoi (&str[LineIDLoc]);
                		LineIDLoc += 6; 
                		n--;
                	}
	            }
	            GlobalUnlock (hLines);
                nRec = (NumAttributes-1)/6 +1;
                n = NumAttributes;
                AttID = Major12 = Major13 = Major14 = 0;
                while (nRec--) 
                {
	              	fgetstring (str,80,FidBNA);
                	LineIDLoc = 0;
                	while (n && LineIDLoc < 72)
                	{   
                		n--;
                		AttID++;
                		Major = atoi (&str[LineIDLoc]);
                		LineIDLoc += 6;
                		Minor = atoi (&str[LineIDLoc]);
                		LineIDLoc += 6;  
                		switch (AttID)
                		{
                			case 12:
                				Major12 = Major;
                				Minor12 = Minor;
                				break;
                			case 13:
                				Major13 = Major;
                				Minor13 = Minor;
                				break;
                			case 14:
                				Major14 = Major;
                				Minor14 = Minor;
                				break; 
                			default:
                				break;
                		}
                	}
	            }
	            Store = FALSE;   
//	     Store = TRUE;
                if (Major12 == 440) 
                {
                	Store=FALSE; 
                	for (i=0;i<nCodes;i++)
                	{
                		if (Minor12 == FEMACode[i])
                		{
                			AreaSym = GMSym[i];
		                	Store=TRUE; 
                			break;
                		}
                	}
                	switch (Minor12)
                	{
                		case 161: 
                		case 181:
                		case 191:
                 			if (SendDlgItemMessage (hWndDlg,IDC_EXCLUDE_NOFLOOD,BM_GETCHECK,0,0)) 
                		    	Store=FALSE;
                		break;
                		default:
                		break;
                	}
                }
	            NextAreaLoc = GSSillseek (FidBNA,0,1); 
                AreaNum++;     
                if (Store)
                {   
					short lcmdstring;
					char	CmdString[48];
					short	stuff[32];
					long	TotPoints=0; 
					
                	pNumPoints = (UINT*)GlobalLock (hNumPoints); 
                	pLineID = (LPSHORT)GlobalLock (hLines);  
                	hPoints = GSSiGlobAlloc ( 959,GMEM_MOVEABLE,nPoly*4);
                	phPoints = (LPHANDLE)GlobalLock (hPoints);
                	for (ipoly=0;ipoly<nPoly;ipoly++)
                	{ 
						*phPoints++ = LoadDLGPoints (pNumPoints,&pLineID,hLineIndex,FidBNA);
						TotPoints += *pNumPoints;
						pNumPoints++;
					} 
					GlobalUnlock (hNumPoints);
					GlobalUnlock (hPoints);
                	pNumPoints = (UINT*)GlobalLock (hNumPoints); 
					AreaRef = GetNewRefno(PltName,NULL,NULL,NULL,NULL); 
					phPoints = (LPHANDLE)GlobalLock (hPoints); 
							                     	 
					sprintf (CmdString,"[CODE12]=%i;[CODE13]=%i;[CODE14]=%i",Minor12,Minor13,Minor14);
					lcmdstring = _fstrlen (CmdString);
					lcmdstring += lcmdstring%2; 
					stuff[1]=40;
					stuff[2]=lcmdstring;
					_fstrncpy ((LPSTR)&stuff[3],CmdString,lcmdstring);
					stuff[0]=2+2+lcmdstring; 
					MaxSides = max (MaxSides,TotPoints);
					AddPolyToMap (nPoly,pNumPoints,phPoints,0,AreaRef,NULL,-1,AreaSym,stuff,NULL,NULL,-1,-1,-1,0,0,0,0,FALSE);
					GlobalUnlock (hNumPoints);
					GlobalUnlock (hPoints);
					phPoints = (LPHANDLE)GlobalLock (hPoints);
					while (nPoly--)
						GlobalFree (*phPoints++); 
					GlobalUnlock (hPoints);
					GlobalFree (hPoints);
                }  
                GlobalFree (hLines);
                GlobalFree (hNumPoints);
                CurLoc = GSSillseek (FidBNA,0,1);       
                PctBox (GetDlgItem(hWndDlg,IDC_STATUS), NumAreas, AreaNum,0);
                goto NextArea;
                 
        EndFile: 
                 CloseMap (TRUE);
                 CloseRefIndex(TRUE);           
                 GSSiClose (FidBNA);  
				 GlobalFree (hLineIndex);
                 DestroySymList (&NumSyms,&hSymDesc);
                 
                 sprintf (mess,"Load complete. %ld areas skipped due to too many points. Largest area had %i points",
                 				NumOverLimit,MaxSides);
                 MessageBox( GetFocus(),mess,"", MB_OK);
                 DisableHalt = FALSE;
                 Processing = FALSE; 
                 ForceRefIndex = ForceTAGIndex = FALSE;
                 ContinueProcessing = TRUE;
                 FileProjectionType = SaveFPT;  
                 GlobalFree (hItems);
		         ConvertCoordClose ();  
		         SetGlobalValue ("%ALT_PROJECTION",SaveAlt);
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

HANDLE LoadDLGPoints (LPSHORT pNumSides,LPSHORT *pLineID,HANDLE hLineIndex,HFILE Fid)
{   
	LPLONG	pLineLoc; 
	HPDPOINT	pPoints, pPoints2;
	HANDLE	hPoints, hPoints2; 
	short	loc, i, nPnts, nrec, n, j;   
	char	str[84]; 
	BOOL	First=TRUE;
	long	NumSides = 1;
	
	hPoints = GSSiGlobAlloc ( 960,GMEM_MOVEABLE,(long)UINT_MAX*(long)sizeof(DPOINT));
	pPoints = (HPDPOINT)GlobalLock (hPoints); 
	while (**pLineID)
	{
		if (**pLineID < 0)
		{
			hPoints2 = GSSiGlobAlloc ( 961,GMEM_MOVEABLE,(long)UINT_MAX*(long)sizeof(DPOINT)); 
			pPoints2 = pPoints;
			pPoints = (HPDPOINT)GlobalLock (hPoints2);
		} 
		pLineLoc = (LPLONG)GlobalLock (hLineIndex); 
		pLineLoc += abs(**pLineID);
		GSSillseek (Fid,*pLineLoc,0);
		GlobalUnlock (hLineIndex); 
		fgetstring (str,80,Fid); 
		nPnts = atoi (&str[32]); 
		NumSides+=(nPnts-1); 
		nrec = (nPnts-1)/3 + 1; 
		n=nPnts;
		while (nrec--)
		{
			fgetstring (str,80,Fid);
			loc = 0; 
			j=n;
			for (i=0;i<min(j,3);i++)
			{
				pPoints->x = atof (&str[loc]);
				loc+=12; 
				pPoints->y = atof (&str[loc]);
				loc+=12;
	            ConvertCoord(pPoints,3,1);
	            pPoints++; 
	            n--;
			} 
		} 
		if (**pLineID < 0)
		{   
			while (nPnts--)
			{
				pPoints--;
				*pPoints2++ = *pPoints;
			}
			GlobalUnlock (hPoints2);
			GlobalFree (hPoints2); 
			pPoints = pPoints2;
		} 
		pPoints--; // keeps from duplicating node points
		*(*pLineID)++;     
		First = FALSE;
	}
	*(*pLineID)++;
	GlobalUnlock (hPoints); 
	if (NumSides > SHRT_MAX)
		MessageBox (GetFocus(),"Too many points in area",NULL,MB_ICONEXCLAMATION); 
	*pNumSides = NumSides; 
	hPoints = GlobalReAlloc (hPoints,NumSides*(long)sizeof(DPOINT),GMEM_MOVEABLE);
	return hPoints; 
} 

BOOL FAR PASCAL NULLDIRMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{  
	char		str[256], dirname[128];
	MNMXCORD	Bounds, FileBounds;  
	double		Acc, Ovr, Width, Height, span;
	short		BegYear, EndYear, BegMonth, EndMonth, imonth, iyear, FirstMonth, LastMonth;      
	long		nRows, nCols, n=0, i, StartTime, EndTime;
	time_t		systime, today;
	struct		tm	tmtime;
	DWORD		Err;
	
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
            	
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
         {  
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);
            break;   
            
            case IDOK:
            case IDC_TEST:
            	GetDlgItemText (hWndDlg,IDC_DIRNAME,str,128); 
            	if (!*str || !makedirectories (str,TRUE,FALSE))  
            	{   
            		GSSiMessageBox ("Unable to create directory",NULL,MB_ICONEXCLAMATION);
            		break;
            	}
            	else if (wParam == IDC_TEST)
            		_rmdir (str);
                if (SendDlgItemMessage (hWndDlg,IDC_CHRONDIR,BM_GETCHECK,0,0))
                {
					char	txt[128];
					 
                    _fmemset (&tmtime,0,sizeof(tmtime));  
	            	GetDlgItemText (hWndDlg,IDC_STARTDATE,str,128);
	            	sprintf (txt,"$CLK(%s,1)",str);     
	            	ExpandText (txt);
					systime = atol(txt);
					if (systime > 0)
					{
						tmtime = *localtime (&systime);   
						BegYear = tmtime.tm_year+1900;
		            	BegMonth = tmtime.tm_mon+1;
						
	                    _fmemset (&tmtime,0,sizeof(tmtime));  
		            	if (GetDlgItemText (hWndDlg,IDC_ENDDATE,str,128))
		            	{
			            	sprintf (txt,"$CLK(%s,1)",str);     
			            	ExpandText (txt);
							systime = atol(txt);
						}
						else
							time(&systime);
						tmtime = *localtime (&systime);   
						EndYear = tmtime.tm_year+1900;
						EndMonth = tmtime.tm_mon+1;
	                    _fmemset (&tmtime,0,sizeof(tmtime));  
						nRows = 1;  
						if (EndYear != BegYear)
							nCols = (EndYear - BegYear - 1) * 13 + (13 - BegMonth) + 1 + EndMonth + 1;
						else
							nCols = EndMonth - BegMonth + 2; 
					}
					else
						nCols = 0;
                }
                else
                { 
	            	GetDlgItemText (hWndDlg,IDC_ACCURACY,str,128);
	            	Acc = atof (str);
	            	if (!Acc) 
	            	{   
	            		GSSiMessageBox ("Accuracy not specified",NULL,MB_ICONEXCLAMATION);
	            		break;
	            	}
	            	GetDlgItemText (hWndDlg,IDC_OVERLAP,str,128);
	            	Ovr = atof (str);
	            	if (!Ovr) 
	            	{   
	            		GSSiMessageBox ("Overlap not specified",NULL,MB_ICONEXCLAMATION);
	            		break;
	            	} 
	            	span = Acc * 60000.0;
	            	Width = span - 2.0 * Ovr;
	            	if (Width <= 0)
	            	{   
	            		GSSiMessageBox ("Invalid parameters",NULL,MB_ICONEXCLAMATION);
	            		break;
	            	} 
	            	nCols = (long)((UserBounds[1].x - UserBounds[0].x) / Width) + 1;
	            	nRows = (long)((UserBounds[1].y - UserBounds[0].y) / Width) + 1;  
	            }
            	if (wParam == IDOK)
            	{   
	               	HCURSOR	OldCursor;
	                	
                    OldCursor = GSSiSetCursor (LoadCursor (0,IDC_WAIT));
            		if (nRows*nCols > 9999)
	            	{   
	            		GSSiMessageBox ("Cannot create more than 9999 tiles",NULL,MB_ICONEXCLAMATION);
	            		break;
	            	} 
	            	GetDlgItemText (hWndDlg,IDC_DIRNAME,dirname,128); 
	            	ExpandText (dirname);
	            	GSSiMakeDir (dirname,&Err); 
                	if (SendDlgItemMessage (hWndDlg,IDC_CHRONDIR,BM_GETCHECK,0,0))
	            	{
		            	for (iyear = BegYear;iyear <= EndYear;iyear++)
		            	{   
		            		tmtime.tm_isdst = -1;
	            			tmtime.tm_mday = 1; 
	            			if (iyear == BegYear)
	            				FirstMonth = BegMonth;
	            			else
	            				FirstMonth = 1; 
	            			if (iyear == EndYear)
	            				LastMonth = EndMonth;
	            			else
	            				LastMonth = 12;
                            for (imonth = FirstMonth; imonth<=LastMonth; imonth++)
                            {   
		            			tmtime.tm_year = iyear -1900;
                            	tmtime.tm_mon = imonth-1;
                            	StartTime = mktime (&tmtime);
                            	if (imonth == 12)
   			            		{
   			            			tmtime.tm_year = iyear+1 -1900;
   			            			tmtime.tm_mon = 0;
   			            		}
   			            		else
   			            			tmtime.tm_mon++;
   			            		EndTime = mktime (&tmtime)-1;
		            			n++; 
		            			sprintf (str,"%s\\%i%2.2i.plt",dirname,iyear,imonth);  
		            			FileBounds = *(LPMNMXCORD)&UserBounds;
			        			CreateNewMap (str,&FileBounds,0,NULL,0,NULL,StartTime,EndTime,FALSE);
			        		}
	            			tmtime.tm_year = iyear -1900;
                        	tmtime.tm_mon = 0;
                        	StartTime = mktime (&tmtime);
	            			tmtime.tm_year = iyear+1 -1900;
	            			tmtime.tm_mon = 0;
		            		EndTime = mktime (&tmtime)-1;
	            			n++; 
		            		sprintf (str,"%s\\%i00.plt",dirname,iyear);  
	            			FileBounds = *(LPMNMXCORD)&UserBounds;
		        			CreateNewMap (str,&FileBounds,0,NULL,0,NULL,StartTime,EndTime,FALSE);
			        	}
			        }
			        else
	            	{ 
		            	Bounds.ymn = UserBounds[0].y - Ovr;
		            	while (nRows--)
		            	{   
		            		Bounds.ymx = Bounds.ymn + span;
		            		if (Bounds.ymx > UserBounds[1].y + span)
		            		{
		            			Bounds.ymx -= Bounds.ymx - (UserBounds[1].y + span);
		            			Bounds.ymn = Bounds.ymx - span;
		            		}
		            		i=nCols;
			            	Bounds.xmn = UserBounds[0].x - Ovr;
		            		while (i--)
		            		{   
			            		Bounds.xmx = Bounds.xmn + span;
			            		if (Bounds.xmx > UserBounds[1].x + span)
			            		{
			            			Bounds.xmx -= Bounds.xmx - (UserBounds[1].x + span);
			            			Bounds.xmn = Bounds.xmx - span;
			            		}
		            			n++; 
		            			sprintf (str,"%s\\file%4.4ld.plt",dirname,n);  
		            			FileBounds = Bounds;
			        			CreateNewMap (str,&FileBounds,0,NULL,0,NULL,0,0,FALSE);
				        		Bounds.xmn += (Width + Ovr); 
			        		}
			        		Bounds.ymn += (Width + Ovr); 
			        	}  
			        }
		            GSSiSetCursor (OldCursor);
	                EndDialog(hWndDlg, TRUE);
            	}
            	else
            	{     
	            	sprintf (str,"%ld tiles required",nRows*nCols);
	            	SetDlgItemText (hWndDlg,IDC_MESS,str);
	                EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE);
	            }
            break;
                

            break; 
            
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} 

BOOL GetIndexedEditFile (LPSTR PltName,LPMNMXCORD pBounds)
{ 
	char	Drive[8], Dir[128], FullName[144];
    LPFILEINDEX lpIndex;
 	HANDLE	handle;
 	BOOL	rtn=FALSE; 
	
	_fullpath (FullName,PltName,sizeof(FullName));
	_splitpath (FullName,Drive,Dir,NULL,NULL); 
	sprintf (FullName,"%s%sindex",Drive,Dir);
	
   	IgnoreBounds=TRUE;
	handle = OpenMapIndex (FullName,NULL);
	if (!handle)
		goto Exit;
	lpIndex = (LPFILEINDEX)GlobalLock (handle); 
NextFile:
    if (lpIndex->FileInIndex >= (long)lpIndex->NumFiles)
    {   
        if (!(lpIndex=GetNextIndexHeader(&handle,TRUE)))
        	goto Exit;
        goto NextFile;
    }
    GetNextIndexEntry(lpIndex);
    if (BoundsInBounds (pBounds,&lpIndex->CurrentEntry->Bounds,0))
    {
		sprintf (PltName,"%s%s%s",Drive,Dir,lpIndex->CurrentEntry->Name); 
	    GSSiGlobUlFree (&handle);
		rtn = TRUE;
	}
	else
 		goto NextFile;
	
Exit:
	IgnoreBounds = FALSE;
	return rtn;	
} 

 

