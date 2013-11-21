From $MISC
/*			{
				int	i;
				POINT	P[2];

				SetViewport(*pCommandViewport);
				SelectClipRgn (CurView->hDC,0);
				SetWindowOrgEx  (CurView->hDC, 0, 0,0 );
				SetViewportOrgEx(CurView->hDC, 0, 0,0 );    
				SetMapMode    (CurView->hDC, MM_TEXT );
				P[0].x = 0;
				P[0].y = 0;
				P[1].x = 25;
				P[1].y = 0;
				for (i=0;i<5;i++)
				{
					P[0].x += 50;
					P[1].x += 50;
					SelectObject (CurView->hDC,GetStockObject (BLACK_PEN));
					Polyline (CurView->hDC,P,2);
				}
			}*/
			//SearchGMC(1);
 {
//     int nRc = DialogBox(hInst, (LPCSTR)"DECONSTRUCT", CurView->hWnd, DeconstructMsgProc);
/*		int	nPoly,np,Type,i,starttime=GetTickCount ();

		for (i=0;i<100;i++)
		{
			int	AreaNum=1;
			HANDLE	hArea;
			int	n=0;

			while (hArea = GetNextHighlightArea (AreaNum++,0,&Type,&np,&nPoly,0,&Offset,n))
			{
				//n=2;
				GSSiGlobFree (&hArea);
			}
			//GetNextHighlightArea (AreaNum++,0,&Type,&np,&nPoly,0,&Offset,3);
		}
		itoa (GetTickCount()-starttime,OutLoc,10);
		goto Rtnl;*/
 }
/*			LPSTR	pmem=(LPSTR)2100000000;

			GSSiGlobAlloc (0,GMEM_MOVEABLE,1000000000);
			if (*pmem > 0)
				(*pmem)--;*/
/*			BROWSEINFO	bi;
			DWORD index;
			char	str[256], Title[32]="Select folder";
			SHFlushClipboard();
			memset (&bi,0,sizeof(BROWSEINFO));
			bi.hwndOwner = CurView->hWnd;
			bi.pidlRoot = NULL;
			bi.pszDisplayName=str;        // Return display name of item selected.
			bi.lpszTitle=Title; 
			SHBrowseForFolder(&bi);*/
 			//CreatePanZoomRotTool (CurView->hWnd,atopt16 (Args,0));
			//CreateToolbarWnd (CurView->hWnd); 
			/*HFILE Fid=GSSiOpenFile ("G:\\mpls\\DataXfer\\SEWER_PILOT\\SANITARY_CONNECTION.CTL",NULL,OF_READ);
			char	str[4096];
			long	mslink,udi;
			LPSTR	loc;

			*str = 0;
			while (stricmp (str,"BEGINDATA"))
				fgetstring (str,4090,Fid);
			while (fgetstring (str,4090,Fid))
			{
				fgetstring (str,4090,Fid);
				fgetstring (str,4090,Fid);
				loc=strstr (str,"|^^");
				*loc = 0;
				loc += 3;
				mslink = atol(&str[1]);
				udi = atol(loc);
				sprintf (str,"%i\t%i",mslink,udi);
				AppendFile ("c:\\connectors.txt",str);
			};*/
/*			COMPVARS	cv;
			nArgs = GetFunArgs (Args,Arg,1,&hMem);  
			memset (&cv,0,sizeof(cv));
			cv.cbSize = sizeof(cv);
			ii=ICCompressorChoose(hWndMain,ICMF_CHOOSE_ALLCOMPRESSORS,0,0,&cv,0); 
			ii=1;*/
			//CreateVehHistMap ("c:\\map.txt","c:\\map.plt","123","CIRCLE",0);
/*			{
				DWORD i=4000000001;
				char	str[64];
				sprintf (str,"%lu",i);
				i--;
			}*/
			//AddTransCanadaDesignatonToStreetName (atoi(Arg[1])); 
			/*
				Fix trav database for vanzandt
				{
				HANDLE hBT = BT_OPEN ("G:\\gmvanzan\\traverse\\VCAD118_del\\TRAVID.IN2",0,BT_READ,0);
				struct {char Prefix[8],UDI[32];}Key;
				long	Loc;
				char	str[128];  
				short	pos=BT_FIRST;   
				long	NextID=797;
				long	LastLoc=62496;
				BTVARDESC	BTVar[2];  
				HANDLE hBT2, hDB=0;  
				char	SymName[32];
			    LPGWDHEADER lpGWDHead; 
			    LPGWFLDINFO lpGWFldInfo;
			    LPTRAVIDDATA pTravIDData; 
				 
				BTVar[0].BT_VARTYP=BT_INTEGER;
				BTVar[0].BT_VARLEN=2;
				BTVar[0].BT_VAROFF=0;
				BT_CREATE ("c:\\tempfix.btr", sizeof(Key), FALSE, 1, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
				hBT2 = BT_OPEN ("c:\\tempfix.btr", 0, BT_WRITE, 0); 
  
				while (!BT_FIND (hBT,(LPSTR)&Key,pos,BT_ANY,(LPSTR)&Loc))
				{
					pos = BT_NEXT; 
					if (Loc > LastLoc)
						BT_PUT (hBT2,(LPSTR)&Loc,(LPSTR)&Key);
					//sprintf (str,"%s\t%s\t%ld",Key.Prefix,Key.UDI,Loc);
					//AppendFile ("c:\\vanfix.txt",str);  
					//SUBDIV "SUBDIVISION_AREA" PINA "PARCEL"
				} 
				BT_CLOSE (hBT); 
				hDB = OpenGWDatabase ("[%TRAVIDDB]",BT_WRITE);
				lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
			    lpGWFldInfo=lpGWDHead->pFldInfo; 
				pTravIDData = (LPTRAVIDDATA)&lpGWDHead->GWDData;
				pos = BT_FIRST;
				while (!BT_FIND (hBT2,(LPSTR)&Loc,pos,BT_ANY,(LPSTR)&Key))
				{
					pos = BT_NEXT;
					pTravIDData->ID = NextID++;
				    lpGWFldInfo=lpGWDHead->pFldInfo; 
				    lpGWFldInfo++;     
	    			SetFieldValFromChar(lpGWDHead,lpGWFldInfo,Key.Prefix,FALSE,FALSE); 
				    lpGWFldInfo++;     
	    			SetFieldValFromChar(lpGWDHead,lpGWFldInfo,Key.UDI,FALSE,FALSE); 
				    lpGWFldInfo++;
				    if (!_fstrnicmp (Key.Prefix,"PINA",4))
				    	_fstrcpy (SymName,"PARCEL");
				    else
				    	_fstrcpy (SymName,"SUBDIVISION_AREA");     
	    			SetFieldValFromChar(lpGWDHead,lpGWFldInfo,SymName,FALSE,FALSE); 
					GWDReplaceRecord (lpGWDHead,0,0,-1);
				}
				BT_CLOSEANDDELETE (&hBT2); 
				GlobalUnlock (hDB);  
				CloseGWDatabase (hDB);   
			} */



BOOL FAR PASCAL SPORTMAPORDERMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1140);
#endif
{
 	int		st, choice, n,i, AreaType;
	char	str[256], str2[128], AreaName[40], AreaTypeName[128], DataTypeName[64], PartID[8]; 
	int		TabStops[4]={100, 180, 214,2010};
	int		TabStops2[2]={1000,1100};
	LPSTR	lpTAB, pLoc, eLoc,pPar; 
	static	HANDLE	hSaveBM;
	char	Drive[32], VolLabel[32];
	UINT	UserCntl[7]={IDC_USERNAME,IDC_ADDRESS1,IDC_ADDRESS2,IDC_CITY,IDC_STATE,IDC_ZIP,IDC_PHONE};
	char	UserVar[7][10]={"USERNAME","ADDRESS1","ADDRESS2","CITY","STATE","ZIP","PHONE"};
    double	MinSize = 1.5, PrimFactor=0.0042, WetFactor=0.0044, DataFactor, Filled, CDs;
    static	long	OrderFreeSpace, CDFreeSpace=CDSize, MB;
    static	double  Area;   
    long	MegB, size;
    static	short	NumCDsInOrder;  
    DPOINT	Points[4];   
    short	ntab=4, PartNum,ii;
 	int		BRtn, nUserVars=7; 
 	MNMXCORD	Bounds;
 	
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1140);
#endif
 	return (BRtn);
}
 switch(Message)
   {
	short	nchar, iday, Choice,DataChoice, sub;
	
    case WM_INITDIALOG:  
    	 hSaveBM = EnterBlockingWindow (hWndDlg);
      	 SendDlgItemMessage (hWndDlg,IDC_DATATYPE,LB_SETTABSTOPS,2,(LPARAM)&TabStops2); 
      	 SendDlgItemMessage (hWndDlg,IDC_AREATYPE,LB_SETTABSTOPS,2,(LPARAM)&TabStops2); 
//    	 ntab = loadtabs (TabStops); 
       	 SendDlgItemMessage (hWndDlg,IDC_CDCONTENTS,LB_SETTABSTOPS,ntab,(LPARAM)&TabStops);
         SendDlgItemMessage (hWndDlg,IDC_CDCONTENTS,LB_RESETCONTENT,0,0);
//   		 SendDlgItemMessage (hWndDlg,IDC_NUMCDS,CB_ADDSTRING,(WPARAM)NULL,(LPARAM) "Calculate");
   		 NumCDsInOrder = 1; 
   		 OrderFreeSpace = 0;
   		 for (i=0;i<5;i++) 
   		 {
   		 	itoa (i+1,str,10);
   		 	SendDlgItemMessage (hWndDlg,IDC_NUMCDS,CB_ADDSTRING,(WPARAM)NULL,(LPARAM) str); 
	   	 }
    	 for (i=0;i<nUserVars;i++)
    	 {
		 	GetPrivateProfileString ("User",UserVar[i],"",str,255,"geomastr.ini");
		 	SetDlgItemText (hWndDlg,UserCntl[i],str);
		 }
		 GetPrivateProfileString ("SportMap","SerialNum","",str,255,"geomastr.ini");
		 SetDlgItemText (hWndDlg,IDC_SERNO,str);  
		 FillList (hWndDlg,IDC_DATATYPE,"[%DL]orders\\datatype.txt",str,0); 
	case GSSI_REINITDIALOG:
		 GetPrivateProfileString ("SportMap","OpenOrderNumCDs","1",str,255,"geomastr.ini"); 
		 NumCDsInOrder = atoi (str);
         SendDlgItemMessage (hWndDlg,IDC_CDNUM,LB_RESETCONTENT,0,0);
		 for (i=1;i<NumCDsInOrder+1;i++)
		 {
			itoa (i,str,10);
			SendDlgItemMessage (hWndDlg,IDC_CDNUM,LB_ADDSTRING,(WPARAM)NULL,(LPARAM) str);  
		 }
		 SendDlgItemMessage(hWndDlg,IDC_NUMCDS, CB_SETCURSEL,NumCDsInOrder-1,0);  
		 GetPrivateProfileString ("SportMap","OpenOrderCurrentCD","0",str,255,"geomastr.ini"); 
		 Choice = atoi (str);
		 sprintf (str,"Contents of CD %i",Choice+1);
		 SetDlgItemText (hWndDlg,IDC_CDID,str);
		 SendDlgItemMessage(hWndDlg,IDC_CDNUM, LB_SETCURSEL,Choice,0);
		 CDFreeSpace = GetCurrentCD (hWndDlg,NumCDsInOrder,&OrderFreeSpace);                 
         PostMessage(hWndDlg, WM_COMMAND, IDC_SETNUMCDS, 0L);
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           {
           	case IDC_CLEARORDER:   
           		 GSSiRemove ("[%DL]orders\\current.txt");
				 WritePrivateProfileString ("SportMap","OpenOrderCurrentCD","0","geomastr.ini");
				 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
           		 break;
           		 
           	case IDC_CLOSEORDER:   
           	{
           		HFILE	FidOut, FidOrder;
           		OFSTRUCT	OFStruct;
           		HANDLE	hSTR;
           		LPSTR	pSTR;
           		long	l;
           			 
           		FidOrder = GSSiOpenFile ("[%DL]orders\\current.txt",&OFStruct,OF_READ); 
				if (FidOrder == HFILE_ERROR)
					break;
				GSSiClose (FidOrder);  
				
				GetPrivateProfileString ("SportMap","PlacedInitialOrder","N",str,8,"geomastr.ini"); 
				if (*str == 'N')
				{
					_fstrcpy (OrderNumber,"SN");
					GetPrivateProfileString ("SportMap","SerialNum","",_fstrchr(OrderNumber,0),16,"geomastr.ini"); 
				}	
				else
         			*OrderNumber = 0;
                {
                  FARPROC lpfnORDERNUMBERMsgProc;
                  short	nRc;
                  
                  lpfnORDERNUMBERMsgProc = MakeProcInstance((FARPROC)ORDERNUMBERMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"ORDERNUMBER", hWndDlg, lpfnORDERNUMBERMsgProc);
                  FreeProcInstance(lpfnORDERNUMBERMsgProc);
                  if (!nRc)
                  	break;

                }
		        _fstrcpy (str,"A:\\order.txt");
		        if (!GetSaveName (hWndDlg,str,IDS_FILTERTEXT,".txt"))
		         	break;   
		    	ExpandText (str);
           		FidOrder = GSSiOpenFile ("[%DL]orders\\current.txt",&OFStruct,OF_READ); 
           		FidOut = GSSiOpenFile (str,&OFStruct,OF_CREATE); 
           		fputstring (GMVersion,FidOut);
           		fputstring (OrderNumber,FidOut);
		 		GetDlgItemText (hWndDlg,IDC_SERNO,str,sizeof(str));  
           		fputstring (str,FidOut);
           		if (App == 2)
           			GetSerialCode (str);
           		else
           			_fstrcpy (str,"UNREGISTERED");
           		fputstring (str,FidOut);
            	for (i=0;i<nUserVars;i++)
            	{
					 GetDlgItemText (hWndDlg,UserCntl[i],str,255);
					 fputstring (str,FidOut);
				}
				while (fgetstring (str,255,FidOrder))
					fputstring (str,FidOut);
           		GSSiClose (FidOrder);
           		GSSiClose (FidOut);
           		FidOut = GSSiOpenFile ("[%DL]orders\\instruct",&OFStruct,OF_READ);   
           		l = _llseek (FidOut,0,2);  
           		_llseek (FidOut,0,0);
           		hSTR = GSSiGlobAlloc (1266,GMEM_MOVEABLE,l+2);
           		pSTR = GlobalLock (hSTR);
           		_lread (FidOut,pSTR,(size_t)l);
           		pSTR[l] = 0;
           		GSSiClose (FidOut); 
           		MessageBox (hWndDlg,pSTR,"",MB_OK);
           		GSSiGlobUlFree (&hSTR);
				WritePrivateProfileString ("SportMap","PlacedInitialOrder","Y","geomastr.ini");
           		
           	} 
           		 break;
           		 
            case IDC_CDNUM: 
                 switch(HIWORD(lParam))
                 {    
                     case LBN_SELCHANGE:
		                  Choice=SendDlgItemMessage(hWndDlg,IDC_CDNUM,LB_GETCURSEL,NULL,NULL); 
						  SaveCurrentCD (hWndDlg);
						  sprintf (str,"Contents of CD %i",Choice+1);
						  SetDlgItemText (hWndDlg,IDC_CDID,str);
						  CDFreeSpace = GetCurrentCD (hWndDlg,NumCDsInOrder,&OrderFreeSpace);
				 }
				 break;
				 
            case IDC_DATATYPE: 
            case IDC_AREATYPE: 
                 switch(HIWORD(lParam))
                 {    
                     case LBN_SELCHANGE:  
            SetSize:
		                  if ((Choice=SendDlgItemMessage(hWndDlg,IDC_AREATYPE,LB_GETCURSEL,NULL,NULL)) < 0)
		                  	break;
		         		  SendDlgItemMessage(hWndDlg,IDC_AREATYPE,LB_GETTEXT,Choice,(DWORD)AreaTypeName); 
		         		  lpTAB = _fstrchr (AreaTypeName,'\t');
		         		  lpTAB++;
		         		  AreaType = atoi (lpTAB);  
		                  if ((DataChoice=SendDlgItemMessage(hWndDlg,IDC_DATATYPE,LB_GETCURSEL,NULL,NULL)) < 0)
		                  	break;
		         		  SendDlgItemMessage(hWndDlg,IDC_DATATYPE,LB_GETTEXT,DataChoice,(DWORD)str2);   
		         		  lpTAB = _fstrrchr (str2,'\t');
		         		  DataFactor = atof (++lpTAB);
						  switch (AreaType)
						  {
						  	case 0: 
						  		MB = CDFreeSpace;
						  		Area = (CDFreeSpace - MinSize) / (DataFactor + PrimFactor + WetFactor);
						  		sprintf (str,"This will cover %.0f square miles",Area);
						  		SetDlgItemText (hWndDlg,IDC_AREAMESS,str);
					  			EnableWindow (GetDlgItem(hWndDlg,IDC_ADDAREA),MB <= CDFreeSpace);
						  	break;
						  	
						  	case 1:
						  		MB = OrderFreeSpace;
						  		Area = (MB - MinSize) / (DataFactor + PrimFactor + WetFactor);
						  		sprintf (str,"This will cover %.0f square miles",Area);
						  		SetDlgItemText (hWndDlg,IDC_AREAMESS,str);
					  			EnableWindow (GetDlgItem(hWndDlg,IDC_ADDAREA),(short)MB);
						  	break;
						  	
						  	case 2:
				         		Area = GetVPArea (*pCommandViewport,NULL,NULL);
				         		Area = ConvertArea (Area,4); 
						  		MB = MinSize + Area * (DataFactor + PrimFactor + WetFactor);   
						  		CDs = (double)MB/CDSize;
						  		sprintf (str,"This will require %ld MB (%.1f CDs)",MB,CDs);
						  		SetDlgItemText (hWndDlg,IDC_AREAMESS,str);
					  			EnableWindow (GetDlgItem(hWndDlg,IDC_ADDAREA),MB <= CDFreeSpace);
						  	break;
						  	
						  	case 3:
			         		    lpTAB = _fstrrchr (AreaTypeName,'|');
			         		    lpTAB++;
			         		    Area = atof (lpTAB);  
						  		MB = MinSize + Area * (DataFactor + PrimFactor + WetFactor);   
						  		CDs = (double)MB/CDSize;
						  		sprintf (str,"This will require %ld MB (%.1f CDs)",MB,CDs);
						  		SetDlgItemText (hWndDlg,IDC_AREAMESS,str);
					  			EnableWindow (GetDlgItem(hWndDlg,IDC_ADDAREA),MB < OrderFreeSpace);
						  	break;
						  }
				 }
				 break;
			
			case IDC_ADDAREA: 
                 Choice=SendDlgItemMessage(hWndDlg,IDC_AREATYPE,LB_GETCURSEL,NULL,NULL); 
         		 SendDlgItemMessage(hWndDlg,IDC_AREATYPE,LB_GETTEXT,Choice,(DWORD)&str);   
         		 lpTAB = _fstrchr (str,'\t');
         		 *lpTAB++ = 0;
         		 AreaType = atoi (lpTAB);
				 if (AreaType == 3)
					_fstrcpy (AreaName,&str[4]);
				 else if (!GetDlgItemText (hWndDlg,IDC_AREANAME,AreaName,32))
				 {  
				 	MessageBox (hWndDlg,"You must enter an area name",NULL,MB_ICONEXCLAMATION);
				 	break;
				 } 
                 DataChoice=SendDlgItemMessage(hWndDlg,IDC_DATATYPE,LB_GETCURSEL,NULL,NULL); 
         		 SendDlgItemMessage(hWndDlg,IDC_DATATYPE,LB_GETTEXT,DataChoice,(DWORD)DataTypeName);   
         		 lpTAB = _fstrrchr (DataTypeName,'\t');
         		 *lpTAB = 0;
         		 MegB = MB;  
         		 GetVPArea (*pCommandViewport,NULL,&Bounds);  
         		 PartNum = 0;
         		 while (MegB > 0)
         		 {
         		 	size = min (MegB,CDFreeSpace);  
         		 	if (size) 
         		 	{
	         		 	if (PartNum || size < MegB)
	         		 		sprintf (PartID,"(%i)",++PartNum);
	         		 	else
	         		 		*PartID = 0;
				 		sprintf (str2,"%s%s\t%s\t%ld\t%ld\t%.0f %.0f %.0f %.0f",AreaName,PartID,DataTypeName,(long)Area,size,
				 				 Bounds.xmn,Bounds.ymn,Bounds.xmx,Bounds.ymx);
			     		ii=SendDlgItemMessage (hWndDlg,IDC_CDCONTENTS,LB_ADDSTRING,0,(LPARAM)str2);
				 		CDFreeSpace -= size; 
				 	}
				 	MegB -= size; 
				 	if (!CDFreeSpace)
				 	{
				 		SaveCurrentCD (hWndDlg);
		                Choice=SendDlgItemMessage(hWndDlg,IDC_CDNUM,LB_GETCURSEL,NULL,NULL);
		                Choice++; 
   			 	 		if (SendDlgItemMessage (hWndDlg,IDC_CDNUM,LB_SETCURSEL,Choice,NULL) != LB_ERR)
   			 	 		{
						    sprintf (str,"Contents of CD %i",Choice+1);
						    SetDlgItemText (hWndDlg,IDC_CDID,str);
					    	CDFreeSpace = GetCurrentCD (hWndDlg,NumCDsInOrder,&OrderFreeSpace);
					    }
				 	}
				 }
				 SaveCurrentCD (hWndDlg);
		    	 CDFreeSpace = GetCurrentCD (hWndDlg,NumCDsInOrder,&OrderFreeSpace);
		         SetDlgItemText (hWndDlg,IDC_AREANAME,"");
				 PostMessage(hWndDlg, WM_COMMAND, IDC_SETNUMCDS, 0L);
				 
				 break;
				 	 
            case IDC_NUMCDS: 
                 switch(HIWORD(lParam))
                 {    
                     case CBN_SELCHANGE:
		                  NumCDsInOrder = SendDlgItemMessage(hWndDlg,IDC_NUMCDS,CB_GETCURSEL,NULL,NULL)+1; 
				          SendDlgItemMessage (hWndDlg,IDC_CDNUM,LB_RESETCONTENT,0,0);
				          for (i=1;i<NumCDsInOrder+1;i++)
				          {
				          	itoa (i,str,10);
				   		  	SendDlgItemMessage (hWndDlg,IDC_CDNUM,LB_ADDSTRING,(WPARAM)NULL,(LPARAM) str);  
				   		  }
		   			 	  SendDlgItemMessage (hWndDlg,IDC_CDNUM,LB_SETCURSEL,0,NULL);				 	 
						  sprintf (str,"Contents of CD %i",1);
						  SetDlgItemText (hWndDlg,IDC_CDID,str);
					      CDFreeSpace = GetCurrentCD (hWndDlg,NumCDsInOrder,&OrderFreeSpace);
				          PostMessage(hWndDlg, WM_COMMAND, IDC_SETNUMCDS, 0L);
		         	 break;
		         	 
                 }
                 break;
            
            case IDC_SETNUMCDS:  
		         SendDlgItemMessage (hWndDlg,IDC_AREATYPE,LB_RESETCONTENT,0,0); 
		         if (CDFreeSpace) 
		   		 	SendDlgItemMessage (hWndDlg,IDC_AREATYPE,LB_ADDSTRING,(WPARAM)NULL,(LPARAM) "Fill CD Centered on Current View\t0"); 
		   		 if (NumCDsInOrder && OrderFreeSpace)
		   		 	SendDlgItemMessage (hWndDlg,IDC_AREATYPE,LB_ADDSTRING,(WPARAM)NULL,(LPARAM) "Fill Order Centered on Current View\t1"); 
	   		 	 SendDlgItemMessage (hWndDlg,IDC_AREATYPE,LB_ADDSTRING,(WPARAM)NULL,(LPARAM) "Create Area From Current View\t2"); 
		   		 if (GetGlobalCVal ("[DEERAREA]",str,0))
		   		 {  
		   		 	double Area = GetGlobalDVal ("[DEERAREASIZE]"); 
		   		 	Area = ConvertArea (Area,4);
		   		 	sprintf (str2,"Use Deer Permit Area %s\t3|%.2f",str,Area);
		   		 	SendDlgItemMessage (hWndDlg,IDC_AREATYPE,LB_ADDSTRING,(WPARAM)NULL,(LPARAM)str2);  
		   		 }
   			 	 SendDlgItemMessage (hWndDlg,IDC_AREATYPE,LB_SETCURSEL,0,NULL);				 	 

            	 goto SetSize;
            	 
            case IDOK: 
            	 for (i=0;i<nUserVars;i++)
            	 {
					 GetDlgItemText (hWndDlg,UserCntl[i],str,255);
					 WritePrivateProfileString ("User",UserVar[i],str,"geomastr.ini");
				 }
		         Choice = SendDlgItemMessage(hWndDlg,IDC_NUMCDS,CB_GETCURSEL,NULL,NULL); 
		         itoa (Choice+1,str,10);
				 WritePrivateProfileString ("SportMap","OpenOrderNumCDs",str,"geomastr.ini");
				 Choice = SaveCurrentCD (hWndDlg);
		         itoa (Choice-1,str,10);
				 WritePrivateProfileString ("SportMap","OpenOrderCurrentCD",str,"geomastr.ini");
                 GSSiEndDialog(hWndDlg,TRUE,hSaveBM); 
                 break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1140);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1140);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}  

BOOL FAR PASCAL ORDERNUMBERMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1141);
#endif
{ 
	char	str[128], cUID[32];	
	HBITMAP	hBmp; 
	HDIB	hDIB;
	HWND	hWnd;
	HDC		hDC; 
	RECT	Rect;

 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1141);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:  
         
       	 SetDlgItemText (hWndDlg,IDC_ORDERNUMBER,OrderNumber); 
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
         {  
         	case IDOK: 
         		if (GetDlgItemText (hWndDlg,IDC_ORDERNUMBER,OrderNumber,16))
                	EndDialog(hWndDlg, TRUE);
         		break;
         		
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);

            break;
         }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1141);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1141);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}                                  

void ERROr( long istat, char *message)
#if ENABLETRACE
{GSSiEnterProg (271);
#endif
{  
   if(istat != 0)
   {
    GSSiMsgBox(
        GetFocus(),
        message,
        0,
        MB_ICONHAND |MB_OK); 
    }    
{
#if ENABLETRACE
GSSiExitProg (271);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}  

HPSTR malloc32 (DWORD Bytes)
{
	MYPROC	Proc;  
	DWORD	rc;
	HPSTR 	pMem=NULL;
	HPSTR	*ppMem=&pMem;
	
	if (!ghLib)
		return FALSE;
	Proc = (MYPROC) lpfnGetProcAddress32W( ghLib, "GM32malloc" );
   	if(Proc == NULL )
   		return FALSE;
   	rc = lpfnCallProcEx32W(2,BitMask("01"),(DWORD)Proc,Bytes,(DWORD)ppMem);
   	return pMem;
}  

DWORD malloc32free (HPSTR pMem)
{
	MYPROC	Proc;  
	DWORD	rc;  
	DWORD	Bytes=0;
	
	if (!ghLib)
		return FALSE;
	Proc = (MYPROC) lpfnGetProcAddress32W( ghLib, "GM32malloc" );
   	if(Proc == NULL )
   		return FALSE;
   	rc = lpfnCallProcEx32W(2,BitMask("01"),(DWORD)Proc,Bytes,(DWORD)pMem);
   	return rc;
}  

double GetNumericFieldDataAtOff (HANDLE hDB,LPFIELDINFO lpField, long Offset, LPINT irc)
#if ENABLETRACE
{GSSiEnterProg (639);
#endif
{ 
    static double   ValD=100;
/*    struct  {char   UseCode[3];
            long    MarketValue, SaleAmt, SaleDate;
            }   data;     */
    LPGWDHEADER lpGWDHead;
    LPGWFLDINFO lpGWFldInfo;
    double      rtn;
    char        str[64];
    short       i, len;
    LPSTR       ep; 
    LPVOID      lpVal;


    *irc = 1;
    
    if (!hDB)
{
#if ENABLETRACE
GSSiExitProg (639);
#endif
    	return (FALSE);
}
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
            
    FillGWDData (lpGWDHead,Offset);
    for (i=0,lpGWFldInfo=lpGWDHead->pFldInfo;i<lpGWDHead->NumFields;i++,lpGWFldInfo++)
    {   if (!_fstrcmp (lpGWFldInfo->Name,lpField->name))
        {
            *irc = 0;
            lpVal = &lpGWDHead->GWDData[lpGWFldInfo->Beg];
            switch (lpGWFldInfo->Type)
            {   
                default:
                case BT_RIGHT_CHAR:
                case BT_CHAR:
                    _fstrncpy (str,lpVal,lpGWFldInfo->Len);
                    str[lpGWFldInfo->Len]='\0';
                    ep = &str[lpGWFldInfo->Len];
                    *ep ='\0';
                    errno = 0;
                    rtn = strtod (str,&ep);
                    while (*ep)
                        if (*ep++ != ' ')
                            *irc = -1;
                break;
                                            
                case BT_INTEGER:
                    if (lpGWFldInfo->Len == 2)
                        rtn = *(LPINT)lpVal;
                    else
                        rtn = *(LPLONG)lpVal;
                break;
                                            
                case BT_REAL:
                    if (lpGWFldInfo->Len == 4)
                        rtn = *(LPFLOAT)lpVal;
                    else
                        rtn = *(LPDOUBLE)lpVal;
                break;
             } 
             GlobalUnlock (hDB);
{
#if ENABLETRACE
GSSiExitProg (639);
#endif
             return (rtn);
}
        }
    
    } 
{
#if ENABLETRACE
GSSiExitProg (639);
#endif
    return FALSE;
}
#if ENABLETRACE
}
#endif
}

DPOINT  OrthoToGround(DPOINT Ortho,LPTRN lptr)
{
    DPOINT  pt;
    
    pt.x = lptr->a * (Ortho.y - lptr->xc) + lptr->b * (Ortho.x - lptr->yc) + lptr->e;
    pt.y = lptr->c * (Ortho.y - lptr->xc) + lptr->d * (Ortho.x - lptr->yc) + lptr->f;
    return pt;
}    

BOOL WriteSavedItems (HANDLE hSaveList)
#if ENABLETRACE
{GSSiEnterProg (110);
#endif
{	LPINT		ipnt, EndItem;
    HANDLE 		hpltBuf;
	LPSTR		LPpltBuf;
	ITEM		*ItemHeader;
	HDC			hDC;
	OFSTRUCT	OFStruct;
	POINT		CenterPoint, WinPoint;
	BOOL		OpenedFid;
	int			SavedItem;
	LPSAVELIST	SaveList, NewList;
	HFILE		Fid;
	LPSTR		ItemPnt;
	HPEN		hOldPen;
	long		SaveLoc;
	int			i;

	if (!PickName[0])
{
#if ENABLETRACE
GSSiExitProg (110);
#endif
		return FALSE;
}
	_fstrcpy (PltName,PickName);

	CloseMap (FALSE);
    SaveList =(LPSAVELIST) GlobalLock(hSaveList);

    for (SavedItem=0;SavedItem<SaveList->NumItems;SavedItem++)
    {
    	if (!SaveList->SLD[SavedItem].ItemHandle)
    		break;
    	PickList[SavedItem] = SaveList->SLD[SavedItem].PD;
		GetPickName (SavedItem);
		if (!*PickName)
{
#if ENABLETRACE
GSSiExitProg (110);
#endif
			return FALSE;
}
		_fstrcpy(PltName,PickName);
		Fid = GSSiOpenFile (PltName,(LPOFSTRUCT)&OFStruct,OF_READWRITE);
		if (Fid == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (110);
#endif
			return FALSE;
}
	    ipnt = (LPINT) GlobalLock((HGLOBAL)SaveList->SLD[SavedItem].ItemHandle);
	    _fmemmove (&SaveLoc,ipnt,4);
	    i=_llseek (Fid,SaveLoc,0);
	    ipnt+=2;
	    ItemHeader = (LPITEM) ipnt;
	    if (!InvalidItem (ItemHeader,FALSE))
		    i=BigWrite (Fid,(HPSTR)ItemHeader,12+2*abs(ItemHeader->Len),-1);
	    GlobalUnlock((HGLOBAL)SaveList->SLD[SavedItem].ItemHandle);
		GSSiClose (Fid); 
	}

	GlobalUnlock (hSaveList);
{
#if ENABLETRACE
GSSiExitProg (110);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL MovePickListToSaveList (HANDLE hSaveList)
#if ENABLETRACE
{GSSiEnterProg (111);
#endif
{	LPINT		ipnt, EndItem;
    HANDLE 		hpltBuf;
	LPSTR		LPpltBuf;
	LPITEM		ItemHeader;
	HDC			hDC;
	OFSTRUCT	OFStruct;
	POINT		CenterPoint, WinPoint;
	BOOL		OpenedFid;
	int			Item;
	LPSTR		ItemPnt;
	LPSAVELIST	SaveList;
	HFILE		Fid;
	WORD		nRead, nBytes;
	long		SaveLoc;
	BOOL		rtn=FALSE;

	if (!PickName[0])
{
#if ENABLETRACE
GSSiExitProg (111);
#endif
		return FALSE;
}
	_fstrcpy (PltName,PickName);


    SaveList = (LPSAVELIST)GlobalLock (hSaveList);

	for (Item=0;Item<NumPicked;Item++)
	{
		GetPickName (Item);
		if (!*PickName)
			break;
		_fstrcpy(PltName,PickName);
		Fid = GSSiOpenFile (PltName,(LPOFSTRUCT)&OFStruct,OF_READ); 
		if (Fid == HFILE_ERROR)
			break;
	    _llseek (Fid,PickList[Item].Segment,0);
	    nRead = _lread (Fid,&nBytes,2);
	    hpltBuf = GSSiGlobAlloc (  60,GMEM_MOVEABLE,(DWORD)nBytes);
	    LPpltBuf = GlobalLock (hpltBuf);
	    SaveLoc = _llseek (Fid,0,1) + PickList[Item].Offset;
	    nRead = _lread (Fid,LPpltBuf,nBytes);
	    if (nRead != nBytes || PickList[Item].Offset > nRead) 
	    {
	    	InvalidItem (NULL,TRUE);
	    	goto Next;
	    }
	    ipnt = (LPINT) (LPpltBuf + PickList[Item].Offset);
	    ItemHeader =  (LPITEM) ((LPINT)ipnt);
	    if (InvalidItem (ItemHeader,TRUE))
	    	goto Next; 
	    SaveList->SLD[Item].PD = PickList[Item];
	    SaveList->SLD[Item].ItemHandle = GSSiGlobAlloc (  61,GHND,2*abs(ItemHeader->Len)+20);
	    ItemPnt = (LPSTR) GlobalLock ((HGLOBAL)SaveList->SLD[Item].ItemHandle);
	    _fmemmove (ItemPnt,&SaveLoc,4);
	    ItemPnt +=4;
	    EndItem = ipnt + abs(ItemHeader->Len);
	    EndItem+=6;
	    *EndItem = 0;
	    _fmemmove (ItemPnt,ipnt,2*abs(ItemHeader->Len)+16);
   	    GlobalUnlock ((HGLOBAL)SaveList->SLD[Item].ItemHandle);
   	    rtn = TRUE;  
Next:
	    GSSiGlobUlFree (&hpltBuf);
		GSSiClose (Fid);
    }
    GlobalUnlock (hSaveList);
{
#if ENABLETRACE
GSSiExitProg (111);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

HANDLE CreateSaveList (int NumItems)
#if ENABLETRACE
{GSSiEnterProg (107);
#endif
{   HANDLE	handle;
	LPSAVELIST	SaveList;
	int	item;

	if (NumItems)
	{
		handle = GSSiGlobAlloc (  57,GHND,2+sizeof(SAVELISTDATA)*NumItems);
		SaveList = (LPSAVELIST) GlobalLock (handle);
		SaveList->NumItems=NumItems;
		GlobalUnlock (handle);
{
#if ENABLETRACE
GSSiExitProg (107);
#endif
		return (handle);
}
	}
	else
{
#if ENABLETRACE
GSSiExitProg (107);
#endif
		return (NULL);
}
#if ENABLETRACE
}
#endif
}

void DeleteSavedItems (HANDLE hSaveList)
#if ENABLETRACE
{GSSiEnterProg (108);
#endif
{   LPSAVELIST	SaveList;
	int	item;

	if (!hSaveList)
{
#if ENABLETRACE
GSSiExitProg (108);
#endif
		return;
}

	SaveList = (LPSAVELIST) GlobalLock (hSaveList);
	for (item=0;item<SaveList->NumItems;item++)
		GSSiGlobFree ( &(HGLOBAL) SaveList->SLD[item].ItemHandle);

	GSSiGlobUlFree (&hSaveList);

{
#if ENABLETRACE
GSSiExitProg (108);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL DrawSavedItems (HANDLE hSaveList, HANDLE hNewList)
#if ENABLETRACE
{GSSiEnterProg (109);
#endif
{	LPINT		ipnt, EndItem;
    HANDLE 		hpltBuf;
	LPSTR		LPpltBuf;
	LPITEM		ItemHeader;
	HDC			hDC;
	OFSTRUCT	OFStruct;
	POINT		CenterPoint, WinPoint;
	BOOL		OpenedFid;
	int			SavedItem;
	LPSAVELIST	SaveList, NewList;
	LPSTR		ItemPnt, hpnt;
	HPEN		hOldPen;
	HANDLE		hVisList;
	LPVISLIST	SaveVis;    

	_fstrcpy (PltName,PickName);

	CloseMap (FALSE);
    SaveList = (LPSAVELIST)GlobalLock(hSaveList);
    if (hNewList)
    	NewList = (LPSAVELIST)GlobalLock(hNewList);

    for (SavedItem=0;SavedItem<SaveList->NumItems;SavedItem++)
    {
    	if (!SaveList->SLD[SavedItem].ItemHandle)
    		continue;
    	PickList[SavedItem] = SaveList->SLD[SavedItem].PD;
		GetPickName (SavedItem);
		if (!*PickName)
			continue;
		_fstrcpy(PltName,PickName);
		if (!OpenMap (CurView->hWnd,CurView->hDC))
			continue;
	    ipnt = (LPINT)GlobalLock((HGLOBAL)SaveList->SLD[SavedItem].ItemHandle);
	    hpnt = (LPSTR)ipnt + 4;
	    ItemHeader = (LPITEM)hpnt;
	    if (InvalidItem (ItemHeader,FALSE))
	    	goto Next;
		GSSiDeleteObject(&CurView->hRgn);
		CurView->hRgn = CreateVPRgn(FALSE,FALSE);
	  	SelectClipRgn (CurView->hDC,CurView->hRgn);
	  	GSSiDeleteObject(&CurView->hRgn);
	    SetDisplayMode (CurView->hDC, GF_MAPMODE);
	    if (hNewList)
	    {   
	    	NewList->SLD[SavedItem].PD = SaveList->SLD[SavedItem].PD;
		    NewList->SLD[SavedItem].ItemHandle = GSSiGlobAlloc (  58,GHND,2*abs(ItemHeader->Len)+20);
		    ItemPnt = (LPSTR) GlobalLock ((HGLOBAL)NewList->SLD[SavedItem].ItemHandle);
		    _fmemmove (ItemPnt,ipnt,4);
		    _fmemmove (ItemPnt,hpnt,2*abs(ItemHeader->Len)+16);
	   	    GlobalUnlock ((HGLOBAL)NewList->SLD[SavedItem].ItemHandle);

	    }
		hVisList=GSSiGlobAlloc (  59,GHND,sizeof(VISLIST));
		SaveVis = CurVis;
		CurVis = (LPVISLIST)GlobalLock (hVisList); 
		CurVis->hVisList=hVisList;
		InitVis ();
		CurVis->WantType[8] = 1;
		CurView->PassID = 4;
	    NewBounds.xmn = INT_MAX;
	    NewBounds.ymn = INT_MAX;
	    NewBounds.xmx = INT_MIN;
	    NewBounds.ymx = INT_MIN;
	    ProcessGraphicsRec (CurView->hDC,(LPINT) hpnt,(LPSTR) ipnt,0);
	    if (NewBounds.xmn != INT_MAX)
	    	ItemHeader->MinMax = NewBounds;
		GlobalUnlock (hVisList);
		GlobalFree	(hVisList);
		CurVis = SaveVis; 
Next:
		CloseMap (FALSE);
	    GlobalUnlock((HGLOBAL)SaveList->SLD[SavedItem].ItemHandle);
	}
	GlobalUnlock (hSaveList);
	if (hNewList)
		GlobalUnlock (hNewList);
{
#if ENABLETRACE
GSSiExitProg (109);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL GolfShot (HWND hWnd, WORD Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (938);
#endif
{
 HDC hDC;
 static HANDLE  SavedScreen=0; 
 static	short	Club=1, ClubType=0;
 DPOINT TowardPoint;
 static DPOINT StartPoint;
 POINT  MousePoint;
 static BOOL    HaveStartPoint;  
 char	Key;
 
 switch (Message)
   {
    case GF_INIT:
        HaveStartPoint = FALSE;   
        break;

    case WM_LBUTTONUP:
        MousePoint = MAKEPOINT(lParam);
        StartPoint = WinPtToBasePt(MousePoint);
        HaveStartPoint = TRUE;   
        break;
    
    case WM_MOUSEMOVE:
        if (HaveStartPoint)
        {   
            MousePoint = MAKEPOINT(lParam);
        	TowardPoint = WinPtToBasePt(MousePoint);
            hDC = GetDC (hWnd);
            ShowGolfShot (hDC,StartPoint,TowardPoint,&SavedScreen,Club);
            ReleaseDC (hWnd,hDC);
        }
        break;
    
    case GF_REDRAW:
    	DestroySavedScreen (&SavedScreen,0);
    	break;
    case GF_CLOSE:
    	DestroySavedScreen (&SavedScreen,0); 
    	return FALSE;
    case GF_CANCEL:
        PostMessage(hWnd, GF_CLOSE,0, 0L);
        break;

    case WM_RBUTTONUP:
        break;
    case WM_KEYDOWN:
		Key = wParam;
		switch (Key)
		{ 
			case 'W':
			case 'w':
				ClubType=0;
				break;
			case 'I':
			case 'i':
				ClubType=1;
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':   
				Club = ClubType*6 + Key-'0';
				break;
			case 'P': 
				Club = 21;
				break;
			case 'S': 
				Club = 22;
				break;
			default:
{
#if ENABLETRACE
GSSiExitProg (938);
#endif
				return (FALSE);
}
		}
		break;
		
    default:
{
#if ENABLETRACE
GSSiExitProg (938);
#endif
        return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (938);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

void ShowGolfShot (HDC hDC,DPOINT StartPoint,DPOINT TowardPoint,LPHANDLE phSavedScreen,short Club)
#if ENABLETRACE
{GSSiEnterProg (939);
#endif
{
	HPEN	hWidePen, hOldPen;
	int		MaxWidth=3;
	double  BoxLineWidth=2, ArrowLineWidth=0, TiplenView, TiplenVeh;
	RECT	Rect;
	int		i;
	POINT	FromPoint, ToPoint;
	double	AZ, DIST, TipFac; 
	COLORREF	VanColor=0;
	char	txt[64]; 
	double	az,  Height=40-Club*2, Dist=250-Club*10-Height*0.75, Width=100-Club*4;   
	HANDLE	hPoints, hPoints2;
	long	nPoints;  
	LPPOINT	pPoints;
    LOGBRUSH    NDB;
    HBRUSH	hBrush, hOldBrush;
    
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
    GSSiDeleteObject(&CurView->hRgn);
	CurView->hRgn = CreateVPRgn(FALSE,FALSE);
  	SelectClipRgn (CurView->hDC,CurView->hRgn);
  	GSSiDeleteObject(&CurView->hRgn);
	FromPoint = BasePtToWinPt (&StartPoint);
	az = getazd (&StartPoint, &TowardPoint);
    TowardPoint = dnewpt (StartPoint,az,Dist);
	ToPoint = BasePtToWinPt (&TowardPoint);
    VanColor = GetGlobalLVal ("[%GOLF_COLOR]");
	hWidePen = CreatePen (PS_SOLID,MaxWidth,VanColor);
	hOldPen = SelectObject (hDC,hWidePen);
    NDB.lbStyle = BS_HATCHED;
    NDB.lbColor = VanColor;
    NDB.lbHatch = HS_DIAGCROSS;
    hBrush =  CreateBrushIndirect(&NDB);
    hOldBrush = SelectObject (hDC,hBrush);

	RestoreScreen2 (hDC, *phSavedScreen,0,FALSE);
    DestroySavedScreen (phSavedScreen,0);
    
    RectInit (&Rect);
    AddPointToRect (FromPoint,&Rect);
    AddPointToRect (ToPoint,&Rect);  
    hPoints = CreateElipse (TowardPoint,az,Height,Width,&nPoints);
    hPoints2 = CreateWinPoints (hPoints,&nPoints,&Rect);
	GSSiGlobUlFree (&hPoints);  
    InflateRect (&Rect,10,10);
	*phSavedScreen = SaveScreen2 (hDC,Rect,CurView,NULL);
//    RegisterSavedScreen (SavedScreen,*SavedRect);
	DrawPointerLine (CurView->hDC,FromPoint,ToPoint,hWidePen,hWidePen,0,0);
	pPoints = (LPPOINT)GlobalLock (hPoints2); 
	Polygon (hDC,pPoints,(short)nPoints);
	GSSiGlobUlFree (&hPoints2);  
Exit:
	SelectObject (hDC,hOldPen);
	SelectObject (hDC,hOldBrush);
	DeleteObject (hWidePen);
	DeleteObject (hBrush);

{
#if ENABLETRACE
GSSiExitProg (939);
#endif
	return;
}

#if ENABLETRACE
}
#endif
}

/*************************************************************************
 *
 * AllocRoomForDIB()
 *
 * Parameters:
 *
 * BITMAPINFOHEADER - bitmap info header stucture
 *
 * HBITMAP          - handle to the bitmap
 *
 * Return Value:
 *
 * HDIB             - handle to memory block
 *
 * Description:
 *
 *  This routine takes a BITMAPINOHEADER, and returns a handle to global
 *  memory which can contain a DIB with that header.  It also initializes
 *  the header portion of the global memory.  GetDIBits() is used to determine
 *  the amount of room for the DIB's bits.  The total amount of memory
 *  needed = sizeof(BITMAPINFOHEADER) + size of color table + size of bits.
 *
 * History:   Date      Author               Reason
 *            6/01/91   Garrett McAuliffe    Created
 *            12/11/91  Patrick Schreiber    Added header and some comments
 *
 ************************************************************************/

/*HANDLE AllocRoomForDIB(BITMAPINFOHEADER bi, HBITMAP hBitmap)
{
   DWORD              dwLen;
   HANDLE             hDIB;
   HDC                hDC;
   LPBITMAPINFOHEADER lpbi;
   HANDLE             hTemp;

   /* Figure out the size needed to hold the BITMAPINFO structure
    * (which includes the BITMAPINFOHEADER and the color table).
    */

   dwLen = bi.biSize + PaletteSize((LPSTR) &bi);
   hDIB  = GlobalAlloc(GHND,dwLen);

   /* Check that DIB handle is valid */
   if (!hDIB)
      return NULL;

   /* Set up the BITMAPINFOHEADER in the newly allocated global memory,
    * then call GetDIBits() with lpBits = NULL to have it fill in the
    * biSizeImage field for us.
    */
   lpbi  = (VOID FAR *)GlobalLock(hDIB);
   *lpbi = bi;

   hDC   = GetDC(NULL);
   GetDIBits(hDC, hBitmap, 0, (WORD) bi.biHeight,
          NULL, (LPBITMAPINFO) lpbi, DIB_RGB_COLORS);
   ReleaseDC(NULL, hDC);

   /* If the driver did not fill in the biSizeImage field,
    * fill it in -- NOTE: this is a bug in the driver!
    */
   if (lpbi->biSizeImage == 0)
      lpbi->biSizeImage = WIDTHBYTES((DWORD)lpbi->biWidth * lpbi->biBitCount) *
              lpbi->biHeight;

   /* Get the size of the memory block we need */
   dwLen = lpbi->biSize + PaletteSize((LPSTR) &bi) + lpbi->biSizeImage;

   /* Unlock the memory block */
   GlobalUnlock(hDIB);

   /* ReAlloc the buffer big enough to hold all the bits */
   if (hTemp = GlobalReAlloc(hDIB,dwLen,GMEM_MOVEABLE))
      return hTemp;
   else
      {
      /* Else free memory block and return failure */
      GlobalFree(hDIB);
      return NULL;
      }
}*/

BOOL FAR PASCAL ATTRIBUTE_TRANSFERMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (448);
#endif
{ 	int		isDir, nchar, Choice, iorig,  i,j;
	LPINT	lpItems;
	static	HANDLE	hItems=0;  
	static	int	nItems;
	char	cmd[512],TagField[33], TagName[9], HPName[33], PCName[13];  
	char	TxtFile[128];
	LPSTR	lpDot, lpTab;
	int			TabStops[2]={300,400}; 
	BTVARDESC BTVar[2], *pVars;
	static	int		NumFields, Reclen, len, KeyType;
	long	Refno, Offset;
	static	long	TotFileLen, KeySequence=0;
	char	CRef[16];
	static	GWDHEADER GWDHead; 
	static	LPGWDHEADER	lpGWDHead;
	static	GWFLDINFO GWFldInfo;
	static	LPGWFLDINFO	lpGWFldInfo;
	static	HANDLE hBT, hVars, hDB, hBlock, hFldInLen;
	static	LPINT	pFldInLen;
	static	LPSTR 	BlockPnt;  
	LPINT	pLen;
	LPSTR	pBlock;
	time_t ltime;
	HDC	hDC;
	static	long	TotRecs, nRecs, nLoaded;
	char	Fname[64], RefStr[32];
	int		FidData;
	int		ibeg,NumIndex,NumIndexFields,ifield;
	OFSTRUCT	OFStruct;
	LPSTR	lpVal;
	static	LPSTR	pName; 
	static	BOOL	TestMode, HaveTimer=FALSE, TextSource=FALSE;   
	MSG		msg;
	static	HFILE	Fid;
	static	LPSTR	lpType, lpEnd;
	LPSTR	lpRec;
	char	str[128];
	static	HANDLE	hRec;
	int		l, inc,ii;   
	long	BlockRec;  
	static	long	debugloaded=24508856;
 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (448);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:  
         cwCenter(hWndDlg, 0);
         TotRecs = 0; 
         TextSource = FALSE;   
       	 SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_SETTABSTOPS,2,(LPARAM)&TabStops); 
         break; /* End of WM_INITDIALOG                                 */

/*    case WM_SETCURSOR:
         if (!hCursor)
{
#if ENABLETRACE
GSSiExitProg (448);
#endif
         	return DefWindowProc(hWndDlg, Message, wParam, lParam); 
}
         GSSiSetCursor (hCursor);
    	 break;  */
    	 
    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
		 if (WSAIsBlocking ())
{
#if ENABLETRACE
GSSiExitProg (448);
#endif
		 	return TRUE;
}
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

   	case WM_TIMER: 
   		 BlockRec = 100;
NextRec:
		 if (WSAIsBlocking ())
{
#if ENABLETRACE
GSSiExitProg (448);
#endif
		 	return TRUE;
}
/*GetMess: if (PeekMessage(&msg, NULL, 0,0, PM_NOREMOVE)) retrn TRUE;*/
	 	 if (TotRecs>0)
		 {  
/*		 	if (!TextSource)
		 	{ 
				SendUMMessage("%NEXT");
				pBlock = BlockPnt; 
				GetUMMessage(cmd);
			    if (!_fstricmp(cmd,"%END"))
			    	goto Done;
			    else 
			    {
			    	if (KeyType == 1)
			    	{
			    		_fstrcpy(RefStr,cmd);
						GetUMMessage(cmd);
					}
					else if (KeyType == 2)
					{
						KeySequence++; 
						ltoa(KeySequence,RefStr,10);
					}
				}
		        while (_fstricmp(cmd,"%END")!=0)
		        {               
		         	_fstrncpy(pBlock,cmd,250);
					GetUMMessage(cmd);
					pBlock+=250;
				}
	            pBlock = BlockPnt;
	            pLen = pFldInLen;
	        }
	        else
	        {*/   
	        	BlockRec = LONG_MAX; 
	        	if (HaveTimer)
	        	{ 
        			 KillTimer(hWndDlg, 1);
                     HaveTimer = FALSE;
                }
	        	lpRec = GlobalLock(hRec);
			    if (!fgetstring (lpRec,32000,Fid)) 
			    	goto Done;
//			}  
			Offset = _llseek (lpGWDHead->Fid,0,2);
			iorig=-1;
        	for (i=0,lpGWFldInfo=lpGWDHead->pFldInfo;i<lpGWDHead->NumFields;
        		 i++,lpGWFldInfo++)
        	{
		 		lpVal = &lpGWDHead->GWDData[lpGWFldInfo->Beg];
				if (!i && KeyType<=2)
				{
					Refno=atol(RefStr);
					_fstrcpy(cmd,RefStr);
				}
				else if (!TextSource)
				{ 
					_fstrncpy(cmd,pBlock,*pLen);
					pBlock+=*pLen;
					cmd[*pLen++]='\0';  
                } 
                else
                {   
            SkipItem: 
            		if (*lpRec == '"')
            		{
            			inc=2;   
            			lpRec++;
                		lpEnd = MatchLev(lpRec,'"');
                		if (!lpEnd)
                			lpEnd = _fstrchr (lpRec,'"');
                	}
                	else 
                	{
                		inc=1;
                		lpEnd = _fstrchr(lpRec,',');
                	}
                	if (!lpEnd)
                		lpEnd = _fstrchr (lpRec,0);
                	*lpEnd = 0;
                	_fstrncpy(cmd,lpRec,512);
                	lpRec = lpEnd;
                	lpRec += inc;
                	iorig++;
  					lpItems=  (LPINT) GlobalLock(hItems); 
  					while (*lpItems<iorig)
  						lpItems++; 
  					GlobalUnlock(hItems);
  					if (*lpItems > iorig)
  						goto SkipItem;

                }
				switch (lpGWFldInfo->Type)
				{
		 			case BT_CHAR:
						_fstrncpy (lpVal,cmd,lpGWFldInfo->Len);
		 			break;
				 			
		 			case BT_RIGHT_CHAR:
						_fmemset (lpVal,' ', lpGWFldInfo->Len);
						l = _fstrlen (cmd);
						lpVal+= max(lpGWFldInfo->Len-l,0);
						_fmemmove (lpVal,cmd,max(l,lpGWFldInfo->Len));
		 			break;
		 			
		 			case BT_INTEGER:
		 				if (lpGWFldInfo->Len == 2)
		 					*(LPINT)lpVal=IDNINT(atof(cmd));
		 				else
		 					*(LPLONG)lpVal=IDNINT(atof(cmd));
		 			break;
				 			
		 			case BT_REAL:
		 			 	if (lpGWFldInfo->Len == 4)
		 					*(LPFLOAT)lpVal=atof(cmd);
		 				else
		 					*(LPDOUBLE)lpVal=atof(cmd);
		 			break;
		 		}
				GSSiTrace(_fstrcat(cmd,lpGWFldInfo->Name),0);
		 	}
	 		lpVal = &lpGWDHead->GWDData[lpGWDHead->pFldInfo->Beg];
			BT_PUT (hBT,(LPSTR)lpVal,(LPSTR)&Offset);
			BigWrite (lpGWDHead->Fid,(HPSTR)&lpGWDHead->Reclen,2,-1);
			BigWrite (lpGWDHead->Fid,(HPSTR)lpGWDHead->GWDData,(UINT)lpGWDHead->Reclen,-1);
			if (TextSource)
			{
				nLoaded = _llseek (Fid,0,1); 
				if (nLoaded >= debugloaded)
					ii=1;
			    GlobalUnlock(hRec);  
			}
			else
				nLoaded++;
			if (!PctBox (GetDlgItem(hWndDlg,IDC_STATUS_BAR), nRecs, nLoaded,10))
				TotRecs=0;
			else
			{
				TotRecs--; 
				if (!(nLoaded%1000) && !TextSource)
				{
					 GlobalUnlock (hDB);
				     CloseGWDatabase (hDB);
					 hDB = OpenGWDatabase (pName,BT_WRITE);
				     if (!hDB)
{
#if ENABLETRACE
GSSiExitProg (448);
#endif
				     	return (FALSE);
}
					 lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
					 hBT = lpGWDHead->BTHandle[0]; 
				}
			} 
			BlockRec--;
			if (BlockRec)
				goto NextRec;
			else
{
#if ENABLETRACE
GSSiExitProg (448);
#endif
				return TRUE;
}
		 }
				  
	Done:
		 EnableWindow(GetDlgItem(hWndDlg,IDC_TEST),TRUE);
		 EnableWindow(GetDlgItem(hWndDlg,IDOK),TRUE);
		 KillTimer(hWndDlg, 1);
		 HaveTimer=FALSE;  
		 GlobalFree (hItems);
		 GlobalUnlock (hDB);
	     CloseGWDatabase (hDB);
		 GSSiGlobUlFree (&hBlock);
		 GSSiGlobUlFree (&hFldInLen);
		 GlobalUnlock(hName);
		 if (TextSource)
		 	GSSiClose (Fid);
/*		 if (!TotRecs && !TextSource)
		 {  
			SendUMMessage("%END"); 
		 	GetUMMessage(cmd);
			GSSiTrace(cmd);
         } */
	 	 TotRecs=0; 
		 {
		    FARPROC lpfnDISPLAY_GWD_DATAMsgProc;
		    int	nRc;
				
		    lpfnDISPLAY_GWD_DATAMsgProc = MakeProcInstance((FARPROC)DISPLAY_GWD_DATAMsgProc, hInst);
		    nRc = DialogBox(hInst, (LPSTR)"DISPLAY_GWD_DATA", hWndMain, lpfnDISPLAY_GWD_DATAMsgProc);
		    FreeProcInstance(lpfnDISPLAY_GWD_DATAMsgProc);
		 }
		 GlobalFree(hName);
			
         break; 
                 
    case WM_COMMAND:
         switch(wParam)
           {   
           	case IDC_SOURCE_HP:  
           		TextSource = FALSE;
          		ShowWindow (GetDlgItem(hWndDlg,IDC_KEY_REFNO),SW_SHOW);
          		ShowWindow (GetDlgItem(hWndDlg,IDC_KEY_FIELD),SW_SHOW);
          		ShowWindow (GetDlgItem(hWndDlg,IDC_KEY_TAG),SW_SHOW);
          		ShowWindow (GetDlgItem(hWndDlg,IDC_KEY_NONE),SW_SHOW);
          		ShowWindow (GetDlgItem(hWndDlg,IDC_LOCATE),SW_HIDE);
          		ShowWindow (GetDlgItem(hWndDlg,IDC_TEXT_FILE),SW_HIDE);
          		ShowWindow (GetDlgItem(hWndDlg,IDC_LOCATE),SW_HIDE);
           		break;
           	
           	case IDC_SOURCE_TEXT:  
          		ShowWindow (GetDlgItem(hWndDlg,IDC_KEY_REFNO),SW_HIDE);
          		ShowWindow (GetDlgItem(hWndDlg,IDC_KEY_FIELD),SW_SHOW);
          		ShowWindow (GetDlgItem(hWndDlg,IDC_KEY_TAG),SW_HIDE);
          		ShowWindow (GetDlgItem(hWndDlg,IDC_KEY_NONE),SW_HIDE);
          		ShowWindow (GetDlgItem(hWndDlg,IDC_LOCATE),SW_SHOW);
           		TextSource = TRUE;
           		break; 
           		
           	case IDC_LOCATE: 
           		*TxtFile = 0;
           		if (GetFileName3 (hWndDlg,TxtFile,IDS_FILTERTEXT,IDS_FILETXT))
           		{   
           			
           			ExpandText (TxtFile);
					Fid = GSSiOpenFile (TxtFile,NULL,OF_READ);
				    if (Fid == HFILE_ERROR)
				   		 break;
           			SetDlgItemText(hWndDlg,IDC_TEXT_FILE,TxtFile);
           			hRec = GSSiGlobAlloc (  30,GMEM_MOVEABLE,SHRT_MAX);
           			lpRec = GlobalLock(hRec);
				    fgetstring (lpRec,32000,Fid);  
				    while (*lpRec)
				    {        
				    	lpRec++;
				    	if (*lpRec == '"')
				    		lpRec++;
				    	if (!(lpType=_fstrchr(lpRec,'('))) goto EndHeader;
				    	*lpType++=0;
				    	lpEnd = _fstrchr(lpType,')');
				    	*lpEnd++=0;
				    	sprintf (str,"%s\t%s",lpRec,lpType);
			 			SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_ADDSTRING,NULL,(LPARAM)str); 
			 			lpRec = lpEnd;
			 			lpRec++;
				    }
			EndHeader:
				    GSSiClose (Fid);
				    GSSiGlobUlFree (&hRec);
           			
           		}
           		break;
           		
           	case IDC_KEY_FIELD: 
				if (WSAIsBlocking ())
{
#if ENABLETRACE
GSSiExitProg (448);
#endif
					return TRUE;
}
           		KeyType=3;
          		ShowWindow (GetDlgItem(hWndDlg,IDC_NUM_KEY_FIELDS),SW_SHOW);
          		ShowWindow (GetDlgItem(hWndDlg,IDC_NUM_KEY_FIELD_TITLE),SW_SHOW);
          		ShowWindow (GetDlgItem(hWndDlg,IDC_TAG_NAME),SW_HIDE);
          		ShowWindow (GetDlgItem(hWndDlg,IDC_TAG_NAME_TITLE),SW_HIDE);
          		ShowWindow (GetDlgItem(hWndDlg,IDC_TAG_FIELD),SW_HIDE);
          		ShowWindow (GetDlgItem(hWndDlg,IDC_TAG_FIELD_TITLE),SW_HIDE);
          		ShowWindow (GetDlgItem(hWndDlg,IDC_ZFILL),SW_HIDE);
          		ShowWindow (GetDlgItem(hWndDlg,IDC_FIELDS_TITLE),SW_SHOW);
          		ShowWindow (GetDlgItem(hWndDlg,IDC_FIELDS),SW_SHOW);
          		ShowWindow (GetDlgItem(hWndDlg,IDC_GWIZ_NAME),SW_SHOW);
          		ShowWindow (GetDlgItem(hWndDlg,IDC_GWIZ_NAME_TITLE),SW_SHOW);
          		if (TextSource)
          		{
	          		ShowWindow (GetDlgItem(hWndDlg,IDC_TEXT_FILE),SW_SHOW);
	          		ShowWindow (GetDlgItem(hWndDlg,IDC_LOCATE),SW_SHOW);
		      		ShowWindow (GetDlgItem(hWndDlg,IDC_HP_NAME),SW_HIDE);
		      		ShowWindow (GetDlgItem(hWndDlg,IDC_HP_NAME_TITLE),SW_SHOW);
		      		SetDlgItemText(hWndDlg,IDC_HP_NAME_TITLE,"Input File"); 
		      	}
/*		      	else 
		      	{
	          		ShowWindow (GetDlgItem(hWndDlg,IDC_TEXT_FILE),SW_HIDE);
	          		ShowWindow (GetDlgItem(hWndDlg,IDC_LOCATE),SW_HIDE);
		      		ShowWindow (GetDlgItem(hWndDlg,IDC_HP_NAME),SW_SHOW);
		      		ShowWindow (GetDlgItem(hWndDlg,IDC_HP_NAME_TITLE),SW_SHOW);	
			        SendUMMessage ("%ATTFILES");
					GetListCB (hWndDlg,IDC_HP_NAME,"HPName");
		      	}*/	      	
           		break;
           		
             
            case IDC_HALT:
            	 if (HaveTimer)
            	 {
					KillTimer(hWndDlg, 1);
					HaveTimer = FALSE;
            	 	SetDlgItemText(hWndDlg,IDC_HALT,"Continue");
            	 }
            	 else
            	 {
					SetTimer(hWndDlg, 1, 1, (FARPROC) NULL);
            	 	HaveTimer=TRUE;               
            	 	SetDlgItemText(hWndDlg,IDC_HALT,"Halt");
            	 }
            	 break; 

            case IDC_TEST:
            	 if (WSAIsBlocking ())
{
#if ENABLETRACE
GSSiExitProg (448);
#endif
            	 	return(TRUE);
}
            	 TestMode=TRUE;
            	 goto StartTransfer;
            	 
            case IDOK:
            	 if (WSAIsBlocking ())
{
#if ENABLETRACE
GSSiExitProg (448);
#endif
            	 	return(TRUE);
}
            	 TestMode=FALSE;
            	 
    StartTransfer:GetDlgItemText(hWndDlg,IDC_GWIZ_NAME,PCName,32);
               	 if (!PCName[0] && KeyType<4)
               	 {
               	 	GSSiMsgBox( GetFocus(), "No GeoMaster File Selected","Error", MB_OK);
				 	break;
				 } 
				 if (TextSource)
				 {
	               	 GetDlgItemText(hWndDlg,IDC_TEXT_FILE,TxtFile,sizeof(TxtFile));
	               	 if (!TxtFile[0])
	               	 {
	               	 	GSSiMsgBox( GetFocus(), "No Text File Selected", "Error",MB_OK);
					 	break;
					 }
				 }
				 else
				 {
	               	 GetDlgItemText(hWndDlg,IDC_HP_NAME,HPName,32);
	               	 if (!HPName[0])
	               	 {
	               	 	GSSiMsgBox( GetFocus(), "No HP/Apollo File Selected", "Error",MB_OK);
					 	break;
					 }
				 }
               	 GetDlgItemText(hWndDlg,IDC_TAG_NAME,TagName,8);
               	 if (!TagName[0] && KeyType == 1)
               	 {
               	 	GSSiMsgBox( GetFocus(), "No TAG Selected", "Error",MB_OK);
				 	break;
				 }
               	 GetDlgItemText(hWndDlg,IDC_TAG_FIELD,TagField,32);
               	 if (!TagField[0] && KeyType==1)
               	 {
               	 	GSSiMsgBox( GetFocus(), "No TAG Field Selected","Error", MB_OK);
				 	break;
				 }
				 if (KeyType==3)
				 {
	               	 GetDlgItemText(hWndDlg,IDC_NUM_KEY_FIELDS,str,32);
	               	 NumIndexFields = atoi(str);
	               	 if (NumIndexFields<1||NumIndexFields>8)
	               	 {
	               	 	GSSiMsgBox( GetFocus(), "The number of key fields must be between 1 and 8","Error", MB_OK);
					 	break;
					 }
				 }
				 else
				 	NumIndexFields=1;
					 
                 nItems=SendDlgItemMessage(hWndDlg,IDC_FIELDS,
										   LB_GETSELCOUNT,
										   NULL,
										   NULL);
                 if (!nItems)
                 {
               	 	GSSiMsgBox( GetFocus(), "No Fields Selected","Error", MB_OK);
				 	break;
				 }
				 
		         
				 _fstrcpy (Fname,"attribut\\");  
				 _fstrcat (Fname,PCName);
				 _fstrcpy (str,Fname);
				 _fstrcat (str,".gmd");
				 FidData = GSSiOpenFile (str,&OFStruct,OF_CREATE); 
				 if (FidData == HFILE_ERROR)
				 {
	               	GSSiMsgBox( GetFocus(), str,"Unable to create data file", MB_OK);
				 	break;
				 }
				 KeySequence=0;
				 lpGWDHead = &GWDHead; 
				 _fmemset (lpGWDHead,'\0',sizeof(GWDHead));  
/*				 if (!TextSource)
				 {
					 if (KeyType < 4)
					 { 
				         _fstrcpy(cmd,"%TRANDATA "); 
				         _fstrcat(cmd,HPName); 
			             SendUMMessage (cmd);
			             SendUMMessage("%GETSIZE");
				     }
				     else
					 { 
				         _fstrcpy(cmd,"%TRANTAG "); 
				         _fstrcat(cmd,HPName); 
				         SendUMMessage (cmd);
				         SendUMMessage("-2000000000");
				     }
				     
					 GetUMMessage(cmd);
				 	 TotFileLen = atol(cmd);
				 }*/
				 if (TestMode)
				 {
					 TotRecs = 25;
					 nRecs = TotRecs;
				 }
				 else
				 {
				 	 nRecs = TotFileLen; 
					 TotRecs = 2000000000;
				 }
				 if (KeyType == 2 || KeyType == 3) _fstrcpy(TagName," "); 
/*				 if (KeyType < 4 && !TextSource)
				 {
			         SendUMMessage(TagName);
			         _fstrcpy(str,"[");
			         _fstrcat(str,Truncate(TagField));
			         _fstrcat(str,"]");
                   	 if (SendDlgItemMessage (hWndDlg,IDC_ZFILL,BM_GETCHECK,0,0L))
				         _fstrcat(str,"Z");

			         SendUMMessage(str); 
			     } */
				 GWDHead.NumFields=0;
				 GWDHead.NumIndex=1;
				 if (GetDlgItemText (hWndDlg,IDC_VERSION,str,16))
				 	GWDHead.Version=atol(str);
				 else	
				 	GWDHead.Version=1;
				 GWDHead.NumIndexFields[0]=NumIndexFields;
				 for (i=0;i<NumIndexFields;i++)
				 	GWDHead.IndexFields[0][i]=i;
				 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER),-1);
				 ibeg = 0;
				 
				 hVars = LocalAlloc (LHND,NumIndexFields * sizeof(BTVARDESC));
			     pVars = (BTVARDESC *) LocalLock(hVars);
			     
				 GWFldInfo.Len = 4;
				 GWFldInfo.Beg = ibeg;
				 GWFldInfo.Type = BT_INTEGER;
				 pVars->BT_VARLEN=4;
				 pVars->BT_VARTYP=BT_INTEGER;
				 pVars->BT_VAROFF=0;
				 ifield=0;
			     if (KeyType == 1)
			     {
					 _fstrcpy (GWFldInfo.Name,"%INT_REFNO");
					 BigWrite (FidData,(HPSTR)&GWFldInfo,sizeof(GWFldInfo),-1);
					 GWDHead.NumFields++; 
					 GWDHead.lKeys[0]=4;
					 pVars++;
					 ibeg += GWFldInfo.Len;
					 ifield=1;
				 }
			     else if (KeyType == 2)
			     {
					 _fstrcpy (GWFldInfo.Name,"%SEQUENCE");
					 BigWrite (FidData,(HPSTR)&GWFldInfo,sizeof(GWFldInfo),-1);
					 GWDHead.NumFields++;
					 GWDHead.lKeys[0]=4;
					 pVars++; 
					 ibeg += GWFldInfo.Len;
					 ifield=1;
				 }
		         
				 hItems=GSSiGlobAlloc (  31,GHND,nItems*2);
				 lpItems=  (LPINT) GlobalLock(hItems);
                 SendDlgItemMessage(hWndDlg,IDC_FIELDS,LB_GETSELITEMS,nItems,(LPARAM)lpItems);
				 for (i=0;i<nItems;i++,lpItems++,ifield++)
				 {  
				 	SendDlgItemMessage(hWndDlg,IDC_FIELDS,LB_GETTEXT,
				                       *lpItems,(LPARAM)str);
                    lpTab = _fstrrchr(str,'\t');
					*lpTab = '\0';
/*				 	if (!TextSource)
				 		SendUMMessage(str); */
				 	lpTab++;  
				 	if (*lpTab == 'C')
						GWFldInfo.Type = BT_CHAR;
					else if (*lpTab == 'B') 
						GWFldInfo.Type = BT_INTEGER;
					else if (*lpTab == 'R') 
						GWFldInfo.Type = BT_REAL;
					else if (*lpTab == 'J') 
						GWFldInfo.Type = BT_RIGHT_CHAR;
					else 
						goto SkipField;
					lpTab++;
					GWFldInfo.Len = atoi(lpTab);
					GWFldInfo.Beg = ibeg;
					if (ifield<NumIndexFields)
					{
						 pVars->BT_VARLEN=GWFldInfo.Len;
						 pVars->BT_VARTYP=GWFldInfo.Type;
						 pVars->BT_VAROFF=ibeg; 
						 pVars++;
						 GWDHead.lKeys[0]=ibeg+GWFldInfo.Len;
					}
					
					ibeg += GWFldInfo.Len;
					_fstrcpy (GWFldInfo.Name,str);
					BigWrite (FidData,(HPSTR)&GWFldInfo,sizeof(GWFldInfo),-1);
					GWDHead.NumFields++;
					SkipField:;
				 }
				 GlobalUnlock(hItems);
				 
/*				 if (!TextSource)
				 {
				 	SendUMMessage("%END");
					 if (KeyType < 4)
					 {
				         SendUMMessage("%GETLENGTHS");
						 GetUMMessage(cmd);
						 hBlock = GSSiGlobAlloc (  32,GHND,32000);
						 BlockPnt = GlobalLock(hBlock);
						 pBlock = BlockPnt; 
						 hFldInLen = GSSiGlobAlloc (  33,GHND,nItems*sizeof(int));
						 pFldInLen = (LPINT)GlobalLock(hFldInLen);
				         while (_fstricmp(cmd,"%END")!=0)
				         {               
				         	_fstrncpy(pBlock,cmd,250);
							GetUMMessage(cmd);
							pBlock+=250;
						 }
		                 pBlock = BlockPnt;
		                 pLen = pFldInLen;
						 for (i=0;i<nItems;i++)
						 {            
						 	_fstrncpy(str,pBlock,3);
						 	str[3]='\0';
						 	*pLen++=atoi(str);
						 	pBlock+=3;
						 }
		             }
		             else
		             { */
						 hBlock = GSSiGlobAlloc (  34,GHND,2500);
						 BlockPnt = GlobalLock(hBlock);
						 pBlock = BlockPnt; 
						 hFldInLen = GSSiGlobAlloc (  35,GHND,nItems*sizeof(int));
						 pFldInLen = (LPINT) GlobalLock(hFldInLen);
		                 pLen = pFldInLen;
						 for (i=0;i<nItems;i++)
						 {            
						 	*pLen++=250;
						 }
//					 }	             
//	             }    
				 GWDHead.Reclen=ibeg; 
				 GWDHead.TimeStamp = time(NULL);
				 _llseek (FidData,0,0);
				 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER),-1);
			 	 _llseek (FidData,0,2);
			     
				 NumIndex = 1;
			
			
				 _fstrcpy (str,Fname);
				 _fstrcat (str,".in1");
			     LocalUnlock(hVars);
			     pVars =(BTVARDESC *)  LocalLock(hVars);
				 BT_CREATE (str, 4, FALSE, NumIndexFields, 1,pVars,FALSE, 0, GWDHead.TimeStamp, FALSE);
			     LocalUnlock(hVars);
			     LocalFree(hVars);
			 	 GSSiClose (FidData);
			 	 
				 hName = GSSiGlobAlloc (  36,GHND,256);
				 pName = GlobalLock(hName);
				 _fstrcpy (pName,Fname);
				 _fstrcat (pName,".gmd");
				 hDB = OpenGWDatabase (pName,BT_WRITE);
			     if (!hDB)
{
#if ENABLETRACE
GSSiExitProg (448);
#endif
			     	return (FALSE);
}
				 lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
				 hBT = lpGWDHead->BTHandle[0]; 
				 nLoaded=0;
		TimerOn: SetTimer(hWndDlg, 1, 1, (FARPROC) NULL); 
				 HaveTimer=TRUE;  
				 if (TextSource) 
				 {
					Fid = GSSiOpenFile (TxtFile,NULL,OF_READ); 
					nRecs = _llseek (Fid,0,2);
					_llseek (Fid,0,0);
	       			hRec = GSSiGlobAlloc (  37,GMEM_MOVEABLE,SHRT_MAX);
	       			lpRec = GlobalLock(hRec);
				    fgetstring (lpRec,32000,Fid);
				    GlobalUnlock(hRec);  
				 }

				 EnableWindow(GetDlgItem(hWndDlg,IDC_TEST),FALSE);
				 EnableWindow(GetDlgItem(hWndDlg,IDOK),FALSE);
				 break;

            case ID_CLOSE: 
				 if (WSAIsBlocking ())
{
#if ENABLETRACE
GSSiExitProg (448);
#endif
				 	return TRUE;
}
            	 if (HaveTimer)
            	 {
					 TotRecs = 0;
            	 }
            	 else if (TotRecs)
            	 {
            	 	 TotRecs = 0;
					 goto TimerOn;
            	 }
            	 else
            	 {
//	               	 CloseUM(FALSE);
	                 EndDialog(hWndDlg, TRUE);
                 }
                 break;

            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */ 
				 if (WSAIsBlocking ())
{
#if ENABLETRACE
GSSiExitProg (448);
#endif
				 	return TRUE;
}
                 if (HaveTimer)
                 {
					 TotRecs = 0;
                 }
            	 else if (TotRecs)
            	 {
            	 	 TotRecs = 0;
					 goto TimerOn;
            	 }
                 else
                 {
//	               	 CloseUM(FALSE);
	                 EndDialog(hWndDlg, FALSE); 
                 }
                 break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (448);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (448);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
} /* End of ATTRIBUTE_TRANSFERMsgProc                                      */ 


long SearchDirectoriesInDir (LPSTR CurDirIN, HFILE OutFile,LPLONG TotFiles,LPSTR WildCard,short Lev)
#if ENABLETRACE
{GSSiEnterProg (282);
#endif
{   
	DWORD	hDir, Type;
	long	NumFilesIn=*TotFiles;
    char    setstr[128],str[128],FullName[144], TestExt[8], CurDir[64], DirName[34];
    short       i, rtn;
    int st;
    BOOL	FirstPass=TRUE, SubDirOnly; 
	LPSTR	lc;
	
	_fstrcpy (CurDir,CurDirIN);   
	lc  = LastChr (CurDir);
    if (*lc == '\\')
    	*lc = 0;
	sprintf (setstr,"%s\\*.*",CurDir);
	st = _dos_findfirst (setstr,_A_SUBDIR,&FileInfo);
   	FirstPass=FALSE;
    while (!st)
    {   
        if (FileInfo.name[0] != '.')
        {
            sprintf (str,"%s\\%s",CurDir,FileInfo.name);
            if (FileInfo.attrib & _A_SUBDIR) 
            {
                SearchDirectoriesInDir (str,OutFile,TotFiles,WildCard,Lev+1);
                _fullpath (FullName,str,sizeof(FullName));
                _splitpath (FullName,NULL,NULL,DirName,TestExt); 
                if (!_fstricmp (DirName,WildCard)) 
                {
		            (*TotFiles)++;
		            if (OutFile != HFILE_ERROR)
		               	fputstring (FullName,OutFile);
		        }
            }
        }
#if WIN32
        st = _findnext (st,&FileInfo);
#else
        st = _dos_findnext (&FileInfo);
#endif
    }
{
#if ENABLETRACE
GSSiExitProg (282);
#endif
    return (*TotFiles - NumFilesIn);
}
#if ENABLETRACE
}
#endif
} 

short DeleteDirAndContents (LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (279);
#endif
{
#if WIN32
    struct  _finddata_t FileInfo; 
#else
    struct  _find_t FileInfo; 
#endif
    char    str[128];
    short       i, rtn, rtn2;
    int st;
    _fstrcpy (str,Name);
    _fstrcat (str,"\\*.*");
#if WIN32
    st = _findfirst (str,&FileInfo);
#else
    st = _dos_findfirst (str,_A_NORMAL|_A_SUBDIR,&FileInfo);
#endif
    while (!st)
    {   
        if (FileInfo.name[0] != '.')
        {
            sprintf (str,"%s\\%s",Name,FileInfo.name);
            if (FileInfo.attrib == _A_SUBDIR) 
            {   
            	rtn2 = DeleteDirAndContents (str);
                rtn = max (rtn,rtn2);
            }
            else 
            { 
                i=GSSiRemove (str);
                if (i == EACCES)
                {   char    mes[256];
                    sprintf (mes,"Unable to delete %s",str);
                    GSSiMsgBox(hWndMain, mes,"Warning", MB_OK|MB_ICONEXCLAMATION); 
                    rtn = 1;
                }
            }
        }
#if WIN32
        st = _findnext (st,&FileInfo);
#else
        st = _dos_findnext (&FileInfo);
#endif
    }
    if (GSSiRemoveDir (Name))
        {   char    mes[256];
            sprintf (mes,"Unable to delete %s",Name);
            GSSiMsgBox(hWndMain, mes,"Warning", MB_OK|MB_ICONEXCLAMATION); 
{
#if ENABLETRACE
GSSiExitProg (279);
#endif
        return 1;
}
    }
    else
{
#if ENABLETRACE
GSSiExitProg (279);
#endif
        return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL CreatePolyProfile (long nPoints,HANDLE hPolyPoints,
						LPLONG pnProfilePoints,LPHANDLE phProfilePoints,short SurfSym)
#if ENABLETRACE
{GSSiEnterProg (1065);
#endif
{
    HPDPOINT	pPoints;
	MNMXCORD	Bounds;
	long		n=nPoints;
	
	DBoundsInit (&Bounds);
	pPoints = (HPDPOINT)GlobalLock (hPolyPoints);
	while (n--)
		AddDPointToMinMax (pPoints++,&Bounds);
	
	ClearHighlightList (FALSE);
	HighlightInArea (CurView->hWnd,&Bounds,TRUE,FALSE);
    
{
#if ENABLETRACE
GSSiExitProg (1065);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
}

HDC CreatePrintCompatibleDC (HWND hWnd)
{	HDC hDC;
    RECT	Rect;
    HANDLE	hPDChunk;
    long	wSize;

	ghWnd = hWnd;
	   
	hWnd = NULL;   
    wSize = sizeof(PRINTDLG);
	lpPDChunk = (LPPRINTDLG)AllocAndLockMem(&hPDChunk, (WORD)wSize);
    InitializeStruct(IDC_PRINTDLG, (LPSTR)lpPDChunk);
    DoPaint = FALSE;   
    HaltPaint = TRUE;
//	EnableWindow (hWndMain,FALSE);
	lpPDChunk->hwndOwner = ghWnd;	
	lpPDChunk->hDC=0;
	PrintDlg(lpPDChunk); 
    hDC = CreateCompatibleDC(lpPDChunk->hDC);
    DoPaint = TRUE;
    HaltPaint = FALSE;
	DeleteDC(lpPDChunk->hDC);
	GSSiGlobUlFree (&hPDChunk);
    return hDC;
} 



BOOL CreateOFT_MATCHTable (LPSTR File,LPSTR BadNames,short OrigKeyLen)
{
 	int		i;
	BTVARDESC BTVar[2], *pVars;
	static	int		NumFields, Reclen, len;
	long	Refno, Offset;
	long	TotFileLen;
	GWDHEADER GWDHead; 
	LPGWDHEADER	lpGWDHead, lpGWDHeadMA, lpGWDHeadCT;
	GWFLDINFO GWFldInfo;
	LPGWFLDINFO	lpGWFldInfo, lpCTField, lpMCDField, lpMAField;
	HANDLE hBT, hVars, hDB, hBlock, hFldInLen,hBTOld, hDBma, hDBct;
	int	FidOld;
	time_t ltime;
	int		FidData;
	int		ibeg,NumVars,NumIndex;
	OFSTRUCT	OFStruct;
	LPVOID	lpVal;
	LPSTR	pName;
	GWFLDINFO FldInfo;
	long	SaveFrame, TLID, Offsetct, Offsetsa;
	int		st, lenct, lenma; 
	char	FileIn1[128], FileIn2[128]; 
	LPSTR	lpDot;  

	 _fstrcpy (FileIn1,File);
	 lpDot = _fstrrchr (FileIn1,'.');
	 if (lpDot)
	 	*lpDot = 0;
	 _fstrcpy (FileIn2,FileIn1);
	 _fstrcpy (BadNames,FileIn1);
	 _fstrcat (FileIn1,".in1");	
	 _fstrcat (FileIn2,".in2");	  
	 _fstrcat (BadNames,".bnf");	  
	 GSSiRemove (FileIn2);
	 lpGWDHead = &GWDHead; 
    _fmemset (lpGWDHead,0,sizeof(GWDHEADER));

	 FidData = GSSiOpenFile (File,&OFStruct,OF_CREATE);
	 GWDHead.NumFields=0;
	 GWDHead.NumIndex=2;
	 GWDHead.Version=1;
	 GWDHead.NumIndexFields[0]=1;
	 GWDHead.IndexFields[0][0]=0;
	 GWDHead.NumIndexFields[1]=-2; // was 2
	 GWDHead.IndexFields[1][0]=1;
	 GWDHead.IndexFields[1][1]=0;
	 GWDHead.lKeys[1]=6;//SHRT_MIN;  //creates as non-unique
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER),-1);
	ibeg = 0;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"RecordNumber");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"MatchCode");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"LocationCode");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"StreetNum");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"HouseNum");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"IntersectionID");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"StreetNum1");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"StreetNum2");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"MunicNum");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"ZIP");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_REAL;
	_fstrcpy (FldInfo.Name,"X");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_REAL;
	_fstrcpy (FldInfo.Name,"Y");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"MunicChanged");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"ZIPChanged");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_REAL;
	_fstrcpy (FldInfo.Name,"Dist");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_REAL;
	_fstrcpy (FldInfo.Name,"Direction");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_REAL;
	_fstrcpy (FldInfo.Name,"Offset");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 32;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"StreetA");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 32;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"StreetB");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = OrigKeyLen;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"OriginalFileKey");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 40;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"Symbol");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 32;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"FromDate");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 32;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"ToDate");  
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	 GWDHead.Reclen=ibeg; 
	 GWDHead.TimeStamp = time(NULL);
	 _llseek (FidData,0,0);
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER),-1);
 	 _llseek (FidData,0,2);
			     
	 NumVars = 1;
	 NumIndex = 2;
			
	 hVars = LocalAlloc (LHND,NumVars * sizeof(BTVARDESC));
     pVars = (LPBTVARDESC)LocalLock(hVars);
			
	 pVars->BT_VARLEN=4;
	 pVars->BT_VARTYP=BT_INTEGER;
	 pVars->BT_VAROFF=0;
	 BT_CREATE (FileIn1, 4, FALSE, 1, 1,pVars,FALSE, 0, GWDHead.TimeStamp, FALSE);
     LocalUnlock(hVars);
     LocalFree(hVars);
 	 GSSiClose (FidData);

     hDB = OpenGWDatabase (File,BT_WRITE);
     if (!hDB) return (FALSE);
     CloseGWDatabase (hDB); 
	 BTVar[0].BT_VARLEN=40;
	 BTVar[0].BT_VARTYP=BT_CHAR;
	 BTVar[0].BT_VAROFF=0;
	 BTVar[1].BT_VARLEN=4;
	 BTVar[1].BT_VARTYP=BT_INTEGER;
	 BTVar[1].BT_VAROFF=40;
	 BT_CREATE (BadNames, 2, FALSE, 2, 1,BTVar,FALSE, 0, GWDHead.TimeStamp, FALSE);
     return TRUE;


} 



int CopyQuadTree (int FidOut)
#if ENABLETRACE
{GSSiEnterProg (666);
#endif
{	
	LPINT pint; 
	int	  id; 
	HANDLE	hQuadTree;
	LPSTR	pQuadTree;
	long	LenQuadSeg, MaxQuadLevel, MaxQuadType, LenQuad, BytesLeft; 
	WORD	nRead, nBytes;

    nRead = _lread (FidMap,&id,2);  
    BigWrite(FidOut,(HPSTR)&id,2,-1);
    nRead = _lread (FidMap,&LenQuadSeg,4); 
    BigWrite(FidOut,(HPSTR)&LenQuadSeg,4,-1);
    nRead = _lread (FidMap,&MaxQuadLevel,4);
    BigWrite(FidOut,(HPSTR)&MaxQuadLevel,4,-1);
    nRead = _lread (FidMap,&MaxQuadType,4);
    BigWrite(FidOut,(HPSTR)&MaxQuadType,4,-1);
    nRead = _lread (FidMap,&LenQuad,4);
    BigWrite(FidOut,(HPSTR)&LenQuad,4,-1);
    BytesLeft = LenQuad;  
    hQuadTree = GSSiGlobAlloc ( 176,GMEM_MOVEABLE,32767);
    pQuadTree = GlobalLock(hQuadTree);
    while (BytesLeft)
    {
	    if (BytesLeft <= 32767)
	    	nBytes = BytesLeft;
	    else
	    	nBytes = 32767;

	    nRead = _lread (FidMap,pQuadTree,nBytes); 
	    BigWrite(FidOut,(HPSTR)pQuadTree,nBytes,-1);
	    BytesLeft -= nBytes;
	}
	GlobalUnlock (hQuadTree);
	GlobalFree (hQuadTree);
{
#if ENABLETRACE
GSSiExitProg (666);
#endif
   	return (MaxQuadType);
}
#if ENABLETRACE
}
#endif
}

#if ENABLETRACE
}
#endif
}


BOOL CopyGraphicsBlock (int  FidOut,
						long NewUsedDescOffset,long NewColorPaletteOffset, long NewTranPointOffset,
						long NewGraphicsOffset )
#if ENABLETRACE
{GSSiEnterProg (667);
#endif
{	HANDLE 		hpltBuf;
	LPSTR		LPpltBuf;
	LPINT		ipnt;
	LPLONG		pOffset; 
	int			Pcode; 
	WORD		nRead, nBytes;
	

    nRead = _lread (FidMap,&nBytes,2);
    hpltBuf = GSSiGlobAlloc ( 177,GMEM_MOVEABLE,(DWORD)nBytes);
    LPpltBuf = GlobalLock (hpltBuf);
    nRead = _lread (FidMap,LPpltBuf,(WORD)nBytes); 
    if (NewUsedDescOffset >= 0)
    {
	    ipnt = (LPINT)LPpltBuf;
        while (*ipnt > 0)
        {   Pcode = *ipnt;
        	ipnt++;
		    switch (Pcode)
	        {
			    case 1: /* color palette offset */
		 		{   ipnt++;
		 			ipnt++;
		 			ipnt++;
		 			ipnt++;
		 		}
	            break;

                case 101:	/*	used description offset	*/
                {   pOffset = (LPLONG)ipnt;
                	*pOffset=NewUsedDescOffset;
                	ipnt += 2;
                }
                break;

			    case 102: /* color palette offset */
                {   pOffset = (LPLONG)ipnt;
                	*pOffset=NewColorPaletteOffset;
                	ipnt += 2;
                }
                break;

                case 103: /* transformation point offset */
                {   pOffset = (LPLONG)ipnt;
                	*pOffset=NewTranPointOffset;
                	ipnt += 2;
                }
                break;

                case 200: /* quad tree offset */
                {   pOffset = (LPLONG)ipnt;
                	*pOffset=NewGraphicsOffset;
                	ipnt += 2;
                }
                break;

                case 11: /* get used description list*/
                {   int	ndesc, i;
                
                	moredesc:   ndesc = *ipnt;
                	ipnt++;
                	for (i=0;i<ndesc;i++)
                	{	ipnt++;
                		ipnt++;
                    	ipnt+=4;
                    }
                	if (ndesc) goto moredesc;
                }
                break;

				case 13: /* continuation offset */
				{	ContinuationOffset = *(LPLONG)ipnt;
					ipnt += 2;
					*ipnt = 0;
                }
                break;

                default:
                	*ipnt =0;
                break;

			}
        }
    }
    BigWrite (FidOut,(HPSTR)&nBytes,2,-1);
    BigWrite (FidOut,(HPSTR)LPpltBuf,(WORD)nBytes,-1); 

	GlobalUnlock (hpltBuf);
    GlobalFree	(hpltBuf);
{
#if ENABLETRACE
GSSiExitProg (667);
#endif
    return (TRUE);
}

short CopyDirectory (LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (279);
#endif
{
#if WIN32
    struct  _finddata_t FileInfo; 
#else
    struct  _find_t FileInfo; 
#endif
    char    str[128];
    short       i, rtn, rtn2;
    int st;
    _fstrcpy (str,Name);
    _fstrcat (str,"\\*.*");
#if WIN32
    st = _findfirst (str,&FileInfo);
#else
    st = _dos_findfirst (str,_A_NORMAL|_A_SUBDIR,&FileInfo);
#endif
    while (!st)
    {   
        if (FileInfo.name[0] != '.')
        {
            sprintf (str,"%s\\%s",Name,FileInfo.name);
            if (FileInfo.attrib == _A_SUBDIR) 
            {   
            	rtn2 = DeleteDirAndContents (str);
                rtn = max (rtn,rtn2);
            }
            else 
            { 
                i=GSSiRemove (str);
                if (i == EACCES)
                {   char    mes[256];
                    sprintf (mes,"Unable to delete %s",str);
                    GSSiMsgBox(hWndMain, mes,"Warning", MB_OK|MB_ICONEXCLAMATION); 
                    rtn = 1;
                }
            }
        }
#if WIN32
        st = _findnext (st,&FileInfo);
#else
        st = _dos_findnext (&FileInfo);
#endif
    }
    if (GSSiRemoveDir (Name))
        {   char    mes[256];
            sprintf (mes,"Unable to delete %s",Name);
            GSSiMsgBox(hWndMain, mes,"Warning", MB_OK|MB_ICONEXCLAMATION); 
{
#if ENABLETRACE
GSSiExitProg (279);
#endif
        return 1;
}
    }
    else
{
#if ENABLETRACE
GSSiExitProg (279);
#endif
        return rtn;
}
#if ENABLETRACE
}
#endif
}

 
BOOL CopyDib2 (HANDLE hdib,LPHANDLE phDibInfo, LPHANDLE phImage)
{
    BYTE __huge *ps;
    BYTE __huge *pd; 
    BYTE	*startimage;
    HANDLE h;
    DWORD cnt;  
    LPBITMAPINFOHEADER	lpbi; 
    clock_t	starttime, endtime;

//    starttime=clock();
    ps = GlobalLock(hdib);
    lpbi = (LPBITMAPINFOHEADER) ps; 
    cnt = lpbi->biSize+lpbi->biClrUsed*sizeof(COLORREF); 
    startimage = ps + cnt;
    h = GSSiGlobAlloc (1406,GMEM_MOVEABLE,cnt);
    pd = GlobalLock(h);

    while (cnt--)
      *pd++ = *ps++;
    *phDibInfo = h;
    GlobalUnlock (h);
    cnt = lpbi->biSizeImage;
    h = GSSiGlobAlloc (1407,GMEM_MOVEABLE,cnt);
    pd = GlobalLock(h);
    ps = startimage;
    while (cnt--)
      *pd++ = *ps++;
    *phImage = h;
    GlobalUnlock(h); 
    GlobalUnlock(hdib);
//    endtime=clock();  
//    TotCopyTime+=(endtime-starttime);
    return TRUE;
}




void ClearQuadTree(int Fid, int Type, long GraphicsOffset)
#if ENABLETRACE
{GSSiEnterProg (668);
#endif
{
	LPLONG pQuadOff;
	LPQUAD pQuad, InQuad;
	long   QuadOff, InQuadOff;
	short  len, nBytes, FileHeader[9], CurRecLen;
	HANDLE		hPltBuf;
	LPSTR		LPpltBuf;
	LPINT		ipnt;
	LPLONG		pOffset;
	long	Type13Offset, CurRecLenLoc;
	long	Offset, CurOffset, EOFOffset, EndQuad;
	long	ii, loc, NegOne=-1;  

	QuadOff=1; 
	loc = GraphicsOffset+18+(QuadOff-1)*LenQuadSeg+12+Type*4;
	EndQuad = loc + LenQuad -1;
	while (loc<EndQuad)
	{
		ii=_llseek(Fid,loc,0);
        BigWrite(Fid,(HPSTR)&NegOne,4,-1);
        QuadOff++; 
		loc = GraphicsOffset+18+(QuadOff-1)*LenQuadSeg+12+Type*4;
    }
{
#if ENABLETRACE
GSSiExitProg (668);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

BOOL ChangeFileSymName (LPSTR FromName,LPSTR ToName)
#if ENABLETRACE
{GSSiEnterProg (976);
#endif
{   
	HFILE	Fid;
	OFSTRUCT	OFStruct; 
	char	NewName[34];
	
	if (!CurView->UpdateFile)
	{
 		GSSiMsgBox(GetFocus(), "No update file in this viewport", NULL,MB_ICONEXCLAMATION|MB_OK);
{
#if ENABLETRACE
GSSiExitProg (976);
#endif
        return FALSE;
}
	} 
	else
		_fstrcpy (PltName,CurView->lpFiles[CurView->UpdateFile-1]);
    
	SymbolNameLoc=0;
	_fstrcpy (NewDescName,FromName);
   	if (!OpenMap (CurView->hWnd,CurView->hDC))
{
#if ENABLETRACE
GSSiExitProg (976);
#endif
   		return FALSE; 
}
   	CloseMap (FALSE);
   	if (!SymbolNameLoc)
{
#if ENABLETRACE
GSSiExitProg (976);
#endif
   		return FALSE;
}
	Fid = GSSiOpenFile (PltName,(LPOFSTRUCT)&OFStruct,OF_READWRITE);
	if (Fid == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (976);
#endif
		return FALSE;
}
	_llseek (Fid,SymbolNameLoc,0);    
	_lread (Fid,NewName,SymbolNameLen);
	NewName[SymbolNameLen]=0;
	Truncate (NewName); 
	if (_fstricmp (NewName,FromName))
	{
		GSSiClose (Fid);
{
#if ENABLETRACE
GSSiExitProg (976);
#endif
		return FALSE;
}
	}
	_llseek (Fid,SymbolNameLoc,0);    
	_fstrncpy (NewName,ToName,SymbolNameLen); 
	BigWrite (Fid,NewName,SymbolNameLen,-1);
	GSSiClose (Fid);  
{
#if ENABLETRACE
GSSiExitProg (976);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
} 


short ChangeFileCoords (short i)
#if ENABLETRACE
{GSSiEnterProg (864);
#endif
{   
	HFILE	Fid; 
	OFSTRUCT	OFStruct;  
	char	Coords[130];
	MNMXCORD MinMaxCoord; 
    MNMXCORD    MinMaxD;
	
    Fid = GSSiOpenFile (PltName,(LPOFSTRUCT)&OFStruct,OF_READWRITE);
	
	_llseek(Fid,TranPointOffset+2,0);
    sprintf (Coords,"%16f%16f%16f%16f%16f%16f%16f%16f",
                (double)-32,(double)32,(double)-23058.178061,(double)23058.178061,  
                (double)652653.961142,(double)696965.463514,
                (double)47187.860422,(double)88271.352328);
/*                MinMaxD.xmx,  
                MinMaxD.ymn,  
                MinMaxD.ymx, 
                MinMaxCoord.xmn, 
                MinMaxCoord.xmx, 
                MinMaxCoord.ymn, 
                MinMaxCoord.ymx);*/ 
    BigWrite (Fid,(HPSTR)Coords,128,-1);
    GSSiClose (Fid);
{
#if ENABLETRACE
GSSiExitProg (864);
#endif
	return 0;
}
#if ENABLETRACE
}
#endif
}


short AutoIncType (LPSTR Prefix)
{   
	short	rtn=0;
	
	LoadTAGDef ();   
	if (NumTAGDef)
	{ 
		LPTAGDEF    lpTAGDef;
		short	i, len, l; 
		LPSTR	pBeg, pEnd;
		double	CurVal; 
		char	SaveEnd;
						            
		lpTAGDef = (LPTAGDEF)GlobalLock (hTAGDef);
		for (i=0;i<NumTAGDef;i++,lpTAGDef++) 
		{
		    if (!_fstricmp (Prefix,lpTAGDef->Prefix))
		    {
		    	rtn = lpTAGDef->IncLen;
		    	break;
		    }
		}
		GlobalUnlock (hTAGDef);
	}
	return rtn; 
}   

HDIB FAR ChangeDIBFormat(HDIB hDIB, WORD wBitCount, DWORD dwCompression)
{
   HDC                hDC;             // Handle to DC
   HBITMAP            hBitmap;         // Handle to bitmap
   BITMAP             Bitmap;          // BITMAP data structure
   BITMAPINFOHEADER   bi;              // Bitmap info header
   LPBITMAPINFOHEADER lpbi;            // Pointer to bitmap info
   HDIB               hNewDIB = NULL;  // Handle to new DIB
   HPALETTE           hPal, hOldPal;   // Handle to palette, prev pal
   WORD               DIBBPP, NewBPP;  // DIB bits per pixel, new bpp
   DWORD              DIBComp, NewComp;// DIB compression, new compression

   /* Check for a valid DIB handle */
   if (!hDIB)
      return NULL;

   /* Get the old DIB's bits per pixel and compression format */
   lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);
   DIBBPP = ((LPBITMAPINFOHEADER)lpbi)->biBitCount;
   DIBComp = ((LPBITMAPINFOHEADER)lpbi)->biCompression;
   GlobalUnlock(hDIB);

   /* Validate wBitCount and dwCompression
    * They must match correctly (i.e., BI_RLE4 and 4 BPP or
    * BI_RLE8 and 8BPP, etc.) or we return failure */
   if (wBitCount == 0)
      {
      NewBPP = DIBBPP;
      if ((dwCompression == BI_RLE4 && NewBPP == 4) ||
      (dwCompression == BI_RLE8 && NewBPP == 8) ||
      (dwCompression == BI_RGB))
     NewComp = dwCompression;
      else
     return NULL;
      }
   else if (wBitCount == 1 && dwCompression == BI_RGB)
      {
      NewBPP = wBitCount;
      NewComp = BI_RGB;
      }
   else if (wBitCount == 4)
      {
      NewBPP = wBitCount;
      if (dwCompression == BI_RGB || dwCompression == BI_RLE4)
     NewComp = dwCompression;
      else
     return NULL;
      }
   else if (wBitCount == 8)
      {
      NewBPP = wBitCount;
      if (dwCompression == BI_RGB || dwCompression == BI_RLE8)
     NewComp = dwCompression;
      else
     return NULL;
      }
   else if (wBitCount == 24 && dwCompression == BI_RGB)
      {
      NewBPP = wBitCount;
      NewComp = BI_RGB;
      }
   else
      return NULL;

   /* Save the old DIB's palette */
   hPal = CreateDIBPalette(hDIB);
   if (!hPal)
      return NULL;

   /* Convert old DIB to a bitmap */
   hBitmap = DIBToBitmap(hDIB, hPal);
   if (!hBitmap)
      {
      DeleteObject(hPal);
      return NULL;
      }

   /* Get info about the bitmap */
   GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);

   /* Fill in the BITMAPINFOHEADER appropriately */
   bi.biSize               = sizeof(BITMAPINFOHEADER);
   bi.biWidth              = Bitmap.bmWidth;
   bi.biHeight             = Bitmap.bmHeight;
   bi.biPlanes             = 1;
   bi.biBitCount           = NewBPP;
   bi.biCompression        = NewComp;
   bi.biSizeImage          = 0;
   bi.biXPelsPerMeter      = 0;
   bi.biYPelsPerMeter      = 0;
   bi.biClrUsed            = 0;
   bi.biClrImportant       = 0;

   /* Go allocate room for the new DIB */
   hNewDIB = AllocRoomForDIB(bi, hBitmap);
   if (!hNewDIB)
      return NULL;

   /* Get a pointer to the new DIB */
   lpbi = (VOID FAR *)GlobalLock(hNewDIB);

   /* Get a DC and select/realize our palette in it */
   hDC  = GetDC(NULL);
   hOldPal = SelectPalette(hDC, hPal, FALSE);
   RealizePalette(hDC);

   /* Call GetDIBits and get the new DIB bits */
   if (!GetDIBits(hDC, hBitmap, 0, (WORD) lpbi->biHeight,
       (LPSTR)lpbi + (WORD)lpbi->biSize + PaletteSize((LPSTR)lpbi),
       (LPBITMAPINFO)lpbi, DIB_RGB_COLORS))
      {
      GlobalUnlock(hNewDIB);
      GlobalFree(hNewDIB);
      hNewDIB = NULL;
      }

   /* Clean up and return */
   SelectPalette(hDC, hOldPal, TRUE);
   RealizePalette(hDC);
   ReleaseDC(NULL, hDC);

   if (hNewDIB)
      /* Unlock the new DIB's memory block */
      GlobalUnlock(hNewDIB);

   DeleteObject(hBitmap);
   DeleteObject(hPal);

   return hNewDIB;
}


HDIB FAR ChangeBitmapFormat(HBITMAP  hBitmap,
            WORD     wBitCount,
            DWORD    dwCompression,
            HPALETTE hPal)
{
   HDC                hDC;          // Screen DC
   HDIB               hNewDIB=NULL; // Handle to new DIB
   BITMAP             Bitmap;       // BITMAP data structure
   BITMAPINFOHEADER   bi;           // Bitmap info. header
   LPBITMAPINFOHEADER lpbi;         // Pointer to bitmap header
   HPALETTE           hOldPal=NULL; // Handle to palette
   WORD               NewBPP;       // New bits per pixel
   DWORD              NewComp;      // New compression format

   /* Check for a valid bitmap handle */
   if (!hBitmap)
      return NULL;

   /* Validate wBitCount and dwCompression
    * They must match correctly (i.e., BI_RLE4 and 4 BPP or
    * BI_RLE8 and 8BPP, etc.) or we return failure
    */
   if (wBitCount == 0)
      {
      NewComp = dwCompression;
      if (NewComp == BI_RLE4)
     NewBPP = 4;
      else if (NewComp == BI_RLE8)
     NewBPP = 8;
      else /* Not enough info */
     return NULL;
      }
   else if (wBitCount == 1 && dwCompression == BI_RGB)
      {
      NewBPP = wBitCount;
      NewComp = BI_RGB;
      }
   else if (wBitCount == 4)
      {
      NewBPP = wBitCount;
      if (dwCompression == BI_RGB || dwCompression == BI_RLE4)
         NewComp = dwCompression;
      else
         return NULL;
      }
   else if (wBitCount == 8)
      {
      NewBPP = wBitCount;
      if (dwCompression == BI_RGB || dwCompression == BI_RLE8)
         NewComp = dwCompression;
      else
         return NULL;
      }
   else if (wBitCount == 24 && dwCompression == BI_RGB)
      {
      NewBPP = wBitCount;
      NewComp = BI_RGB;
      }
   else
      return NULL;

   /* Get info about the bitmap */
   GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);

   /* Fill in the BITMAPINFOHEADER appropriately */
   bi.biSize               = sizeof(BITMAPINFOHEADER);
   bi.biWidth              = Bitmap.bmWidth;
   bi.biHeight             = Bitmap.bmHeight;
   bi.biPlanes             = 1;
   bi.biBitCount           = NewBPP;
   bi.biCompression        = NewComp;
   bi.biSizeImage          = 0;
   bi.biXPelsPerMeter      = 0;
   bi.biYPelsPerMeter      = 0;
   bi.biClrUsed            = 0;
   bi.biClrImportant       = 0;

   /* Go allocate room for the new DIB */
   hNewDIB = AllocRoomForDIB(bi, hBitmap);
   if (!hNewDIB)
      return NULL;

   /* Get a pointer to the new DIB */
   lpbi = (VOID FAR *)GlobalLock(hNewDIB);

   /* If we have a palette, get a DC and select/realize it */
   if (hPal)
   {
      hDC  = GetDC(NULL);
      hOldPal = SelectPalette(hDC, hPal, FALSE);
      RealizePalette(hDC);
   }

   /* Call GetDIBits and get the new DIB bits */
   if (!GetDIBits(hDC, hBitmap, 0, (WORD) lpbi->biHeight,
       (LPSTR)lpbi + (WORD)lpbi->biSize + PaletteSize((LPSTR)lpbi),
       (LPBITMAPINFO)lpbi, DIB_RGB_COLORS))
      {
      GlobalUnlock(hNewDIB);
      GlobalFree(hNewDIB);
      hNewDIB = NULL;
      }

   /* Clean up and return */
   if (hOldPal)
   {
      SelectPalette(hDC, hOldPal, TRUE);
      RealizePalette(hDC);
      ReleaseDC(NULL, hDC);
   }

   if (hNewDIB)
      {
      /* Unlock the new DIB's memory block */
      GlobalUnlock(hNewDIB);
      }

   return hNewDIB;
}

void ApplyVPHalfTone (void)
#if ENABLETRACE
{GSSiEnterProg (1043);
#endif
{   
	short	iview;
	LPVIEWPORT	SaveVP = CurView;
	
    for (iview = 0;iview<*pNumViewports; iview++)
    {   
    	SetCurView ( pViewportsD[iview]); 
//        if (CurViewActive())
//        	HalfToneViewport ();
    }  
    SetCurView ( SaveVP);
{
#if ENABLETRACE
GSSiExitProg (1043);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void ApplyVPBounds (void)
#if ENABLETRACE
{GSSiEnterProg (1044);
#endif
{   
	short	iview;
	MNMXCORD	WBounds; 
	LPBOUNDSDISPLAY lpBoundsDisplay;
	
    for (iview = 0;iview<*pNumViewports; iview++)
    {   
    	SetCurView ( pViewportsD[iview]); 
        if (CurViewActive())
        {
        	if (CurView->Type == 7 && CurView->pTheme)
	        {
				CurTheme = CurView->pTheme;
				if (CurTheme->ID != GF_BOUNDS_DISPLAY_THEME ||
					!CurTheme->IsActive|| 
					!CurTheme->VPDisplayed)
					goto Next;  
			    SetDisplayMode (CurView->hDC,GF_TEXTMODE);
			    GSSiDeleteObject(&CurView->hRgn);
		        CurView->hRgn = CreateVPRgn (TRUE,FALSE);
		        SelectClipRgn (CurView->hDC,CurView->hRgn);
		        GSSiDeleteObject(&CurView->hRgn);
				lpBoundsDisplay = (LPBOUNDSDISPLAY)&CurTheme->ClassBM;  
				SetViewport (CurTheme->TargetViewport);
				WBounds = CurView->WBounds;
				SetViewport (CurTheme->DisplayViewport);
				CurView->BoundsDisplayVP = CurTheme->TargetViewport;   
				ShowZoomArea (CurView->hDC,&WBounds,
							               &lpBoundsDisplay->SavedScreen,
							               &lpBoundsDisplay->SavedRect,
										   lpBoundsDisplay->BoxPoints);
	        }
	        else if (CurView->lpBoundsDisplay)  
		     	BoundsDisplayShow (CurView->lpBoundsDisplay,&CurView->BoundsDisplayed);
		}
        
Next:;
    }
{
#if ENABLETRACE
GSSiExitProg (1044);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}
 

BOOL ClipMapx (HWND hWnd, LPSTR Name,short SizeOpt)
{   
	HDIB	hDib;
	short	SaveMMH=MemMapHeight,SaveMMW=MemMapWidth;
    HDC SaveDC = CurView->hDC;  
    HBITMAP	hbmpOld;
    BOOL	SaveMemMap = MemMap; 
    HDC		hPr; 
    RECT	Rect;
	LPVIEWPORT	lpSaveView, LastVP; 
	short	startW,	endW = 4096;
	short	startH;
	double	WtoHFactor;  
	char	str[256];    
	RECT	ShapeRect; 
	long	rtn;
    
    if (SizeOpt == 1)
    	endW = 1024;
    if (!GetFormatRect (&ShapeRect))
    {
    	if (IsRectEmpty (&ConfigDisplayRect))
    		GetClientRect (hWnd,&ShapeRect);
    	else
    		ShapeRect = ConfigDisplayRect;
    } 
    WtoHFactor =(double)(ShapeRect.bottom - ShapeRect.top) / (ShapeRect.right - ShapeRect.left);
    OldCursor = GSSiSetCursor (LoadCursor (NULL,IDC_WAIT)); 
	MemMap = TRUE;  
	startW = min (endW,pow (16000000 / (WtoHFactor * 3),0.5)-1); 
ReTry:
	MemMapWidth = startW;  
//		MemMapHeight = MemMapWidth * WtoHFactor;
//		hdcMemMap = LargeMemDC (CurView->hDC,MemMapWidth,MemMapHeight);    
//		goto temp;
	hdcMemMap = CreateCompatibleDC(CurView->hDC); 
	hMemBitmap = 0;
	do
	{   
//		sprintf (str,"%i",MemMapWidth);
//		SetWindowText (hWnd,str);
		GSSiDeleteObject (&hMemBitmap);
		MemMapWidth++;
		MemMapHeight = MemMapWidth * WtoHFactor;
		hMemBitmap = CreateCompatibleBitmap (CurView->hDC,MemMapWidth,MemMapHeight);
	}
	while (hMemBitmap && MemMapWidth < endW + 1 && ((double)MemMapWidth * (double)MemMapWidth * WtoHFactor * 3) < 16000000);
	
    SaveViewports ();
	MemMapWidth--;    
	MemMapHeight = MemMapWidth * WtoHFactor;
	GSSiDeleteObject (&hMemBitmap);
	hMemBitmap = CreateCompatibleBitmap (CurView->hDC,MemMapWidth,MemMapHeight);
    if (!hMemBitmap)
    {   
    	if (startW > 1024)
    	{   
    		startW -= 128;
    		goto ReTry;
    	}
    	GSSiMessageBox ("Unable to create memory bitmap",NULL,MB_ICONEXCLAMATION);
    	goto Exit;
    }
	hbmpOld = SelectObject(hdcMemMap, hMemBitmap); 
temp:    
    hPr = hdcMemMap;
    Printing = TRUE;  
    FileMode = TRUE;
	Rect.left = Rect.top = 0;
	Rect.right = MemMapWidth -1;
	Rect.bottom = MemMapHeight -1;  
	SetMainRect (0,hPr,&Rect);
			   if (!SetupViewports (NULL,hPr,0,Rect,0))
			   {
					goto Exit;
			   }
	   	       SetDisplayMode (CurView->hDC, GF_TEXTMODE);
			   FillRectPoly (hPr,&Rect,WindowColor);
			   NumViewportsToDisplay = *pNumViewports;
			   for (DisplayViewID=0;DisplayViewID<NumViewportsToDisplay;DisplayViewID++)
			   {
					SetCurView (pViewports[DisplayViewID]);
					if (CurViewActive ())
						CurView->Display = TRUE;
					ResetViewport (TRUE,TRUE);
/*					if (SaveHaveBounds[DisplayViewID])
					{
						CurView->HaveBounds = TRUE;
						CurView->WBounds = SaveWBounds[DisplayViewID];
						CurView->NewBounds = CurView->WBounds;
					}*/
			   }
			   DisplayViewID = 0; 
			   LastVP = 0;
			   while (DisplayViewID<NumViewportsToDisplay)
			   {
			   		SetCurView (pViewportsD[DisplayViewID]);
			   		if (CurView != LastVP && CurView->NumFiles) 
			   		{
			   			if (CurView->ID == *pCommandViewport &&
					     	CurView->OrthoRes && CurView->WindowZoomedToOrtho)
					    {
					        CurView->WBounds = CurView->NewBounds;
					        SetNewBoundsToOrtho();
					    }
				   			
			   			SetBoundsRect2 (CurView->DrawRect,CurView->hDC);
			   		}  
			   		LastVP = CurView;
					if (DisplayViewport (hWnd,hPr,TRUE)) 
					{  
				       if (OpenMap (hWnd, hPr))
				       {   
	   	       	   	       SetDisplayMode (CurView->hDC, GF_MAPMODE);
			
					       do
					       {
					   		//	sprintf (Line3,"Band %i - Viewport %i - Segment %ld",Band,CurView->ID,iseg++);
							//	PrintMessage2 (NULL,NULL,Line3);
					       }
					       while (DisplaySeg (hPr,FALSE) && CurView); 
					   }
				     }
				     
			   }
	       	   EndDisplayProcessing (TRUE);
			   ApplyVPShadows ();	

/*	CurView->WindowZoomedToOrtho = TRUE;  
	CurView->WindowIsZoomed = TRUE;   
	SaveCurView (0);    
	_fstrcpy (MemMapName,"c:\\mem.bmp");
	PaintMap (CurView->hWnd,hdcMemMap,TRUE,NULL);  
	SaveCurView (1);
    CurView->hDC = SaveDC;
    hDib = BitmapToDIB (hMemBitmap,0);
    sprintf (Name,"c:\\dib%i.bmp",id++);
    SaveDIB (hDib,Name); 
	DestroyDIB (hDib);*/

	if (Name)
	{
//	SaveDCBitMap (hdcMemMap,Name,0,0); 
	    hDib = BitmapToDIB (hMemBitmap,0);
		SelectObject(hdcMemMap, hbmpOld);   
		GSSiDeleteObject (&hMemBitmap);
	    rtn = SaveDIB (hDib,Name); 
		DestroyDIB (hDib); 
		if (rtn)
		{
			sprintf (str,"Error saving bitmap. Code %ld",rtn);
			MessageBox (0,str,NULL,MB_ICONEXCLAMATION);
		}
	
	}
	else
    {
		if (!OpenClipboard (hWnd))
		{  
			SelectObject(hdcMemMap, hbmpOld);   
			GSSiDeleteObject (&hMemBitmap);
			MessageBeep (MB_ICONEXCLAMATION);
			MessageBox( GetFocus(), "ERROR: Cannot access the clipboard",NULL, MB_OK|MB_ICONEXCLAMATION);
		}                   
		else
		{
			EmptyClipboard();
			SetClipboardData(CF_BITMAP, hMemBitmap);
			CloseClipboard();  
			SelectObject(hdcMemMap, hbmpOld);   
		}
	}
Exit:  
	RestoreViewports ();
	MemMap = SaveMemMap;
	MemMapWidth = SaveMMW;
	MemMapHeight = SaveMMH; 
	DeleteDC (hdcMemMap);
	hdcMemMap = 0; 
    GSSiSetCursor (OldCursor);
	Printing = FALSE;   
	FileMode = FALSE;
	return TRUE;
} */

 


BOOL UpdateSysMsgFile (LPSTR Msg)
#if ENABLETRACE
{GSSiEnterProg (386);
#endif
{
	HFILE Fid;
	HANDLE	hSTR=GSSiGlobAlloc (  98,GMEM_MOVEABLE,1024); 
	LPSTR	Name=GlobalLock (hSTR);
	LPOFSTRUCT pOFStruct=(LPOFSTRUCT)(Name+256); 
	long	loc=0, LastLoc=0;
 		
	_fstrcpy (Name,"[%DL]sysmsg.txt");
	ExpandText (Name);
	Fid=OpenFile (Name,pOFStruct,OF_READWRITE);
	if (Fid != HFILE_ERROR)
	{  
		while (fgetstring (Name,300,Fid))
		{
			loc = LastLoc;
			LastLoc = _llseek (Fid,0,1);
		}
		_llseek (Fid,loc,0);
		fputstring (Msg,Fid);
		_lclose (Fid);  
	} 
	GSSiGlobUlFree (&hSTR);	
{
#if ENABLETRACE
GSSiExitProg (386);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}	


short HaveCompleteRaytheonPacket (LPBYTE InPacket,LPSHORT InLength,LPSTR Packet,LPBYTE pChkSum)
{   
	LPSTR	pBeg=InPacket, pEnd, pCk;
	short	l;
	
	InPacket[*InLength] = 0; 
Top:
	pBeg = _fstrchr (pBeg,'$');
	if (!pBeg)
		return INVALIDPACKET;
	pEnd = _fstrchr (pBeg,'\n');  
	if (!pEnd)
		return INVALIDPACKET;
	if ((pCk = _fstrrchr (pBeg,'*'))) 
	{
		*pCk++ = 0;   
		sscanf (pCk,"%X",pChkSum);     
	}
	_fstrcpy (Packet,pBeg);   
	pEnd++;
	l = pEnd - InPacket;
	if (l>0)
	{
		(*InLength) -= l;
		_fmemmove (InPacket,pEnd,*InLength);
	}
	return  VALIDPACKET;
}
	
short GetRaytheonPacket (LPBYTE Packet)
{
	int        nError, nLength=1,ii, ierr,LenInput ;
	COMSTAT    ComStat ;
	MSG        msg ; 
	BOOL	HaveMsg;  
	BYTE	OutPacket[64], ChkSum, ChkSumMG; 
	short	PacketID=0;
	COMSTAT	cStat; 
	HCURSOR	hcurSave;
   
//	hCursor = LoadCursor(NULL, IDC_WAIT);
//	hcurSave = SetCursor(hCursor); 
	nLength = ReadComm( Stream, &InPacket[InLength], 512 );  
	if (nLength > 0)
		InLength += nLength;
	nLength = 1; 
Top:
//	SetTimer (hWndMain,GPSTIMER,GPSTimeOut,NULL);
    HaveMsg = TRUE;
    while (ContinueProcessing)
    {   
    	if (nLength>0)
    		msg.message = MOREDATA;
    	else
			ContinueProcessing = GetMessage( &msg, NULL, 0,0);
		switch (msg.message)
		{   
			case WM_TIMER:
				if (msg.wParam != GPSTIMER)
					break;
				KillTimer (hWndMain,GPSTIMER);  
				goto Exit;
				
			default:  
//				if (msg.message != WM_PAINT)
//					break;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			    break;
			    
			case WM_COMMNOTIFY:
      			if (CN_EVENT & LOWORD( msg.lParam ) != CN_EVENT)
      				break;
      		
            case MOREDATA:
			     GetCommEventMask( Stream, EV_RXFLAG ) ;
				 if ((nLength = ReadComm( Stream, &InPacket[InLength], 1024-InLength))>0)
				 	InLength+=nLength; 
			 	 if ((PacketID = HaveCompleteRaytheonPacket (InPacket,&InLength,Packet,&ChkSumMG)))
				 {
//						KillTimer (hWndMain,GPSTIMER); 
						ChkSum = ComputeNMEACheckSum (Packet,_fstrlen(Packet));
//						sprintf (OutPacket,"$PMGNCSM,%2.2X",ChkSum);  
//						SendMagellanPacket (OutPacket);  
						if (ChkSum == ChkSumMG)
				 			goto Exit;
				 		else
				 			goto Top;
				 } 
			}
      }
//	KillTimer (hWndMain,GPSTIMER);
Exit:
//	SetCursor(hcurSave); 
	return PacketID;
}

BOOL GPSImportRaytheon (HWND hWndDlg,UINT Control,UINT StatusControl)
{ 
	LSI100HEADER	Header; 
	LSI100WAYPOINT	WayPoint;
	WORD			WPNum=1;
	HCURSOR	hcurSave; 
	char	str[256],Packet[512];  
	short	l,ii;
	long	nLoaded=0, nRecs;  
Top:  
	WPNum = 0;  
//	hCursor = LoadCursor(NULL, IDC_WAIT);
//	hcurSave = SetCursor(hCursor); 
	
	_fstrcpy (str,"Waiting for waypoints. Initiate transfer on Raytheon unit.");   
	SetDlgItemText (hWndDlg,StatusControl,str);   
	CreateStatusWindow (hWndMain,1,NULL);
	StatusWindowUpdate ("Waypoint transfer",str, 0,0);
	GetRaytheonPacket (Packet);
	WayPoint.Status = 1;
	while (ContinueProcessing)
	{
		if (!_fstrnicmp (Packet,"$ECWPL,",6))
		{
			LPSTR	pLat,pLon,pName,pDesc,pSym;
			double	lat,lon, latdeg, latmin, londeg, lonmin; 
			short	ii;  
			char	LatC[32],LonC[32], Name[12],savec;
			short	Deg, Min,ndp=1, index;
			double	Sec;    
						 					
			pLat=&Packet[7];
			pLon=_fstrchr (pLat,',');
			pLon++;
			pLon=_fstrchr (pLon,',');
			pLon++;      
			latdeg = ldread (pLat,2); 
			pLat+=2;
			latmin = atof (pLat);
			lat = latdeg + latmin/60;
			londeg = ldread (pLon,3); 
			pLon+=3;
			lonmin = atof (pLon);
			lon = londeg + lonmin/60;
			pDesc = _fstrchr (pLon,',');
			pDesc++;
			pDesc = _fstrchr (pDesc,',');
			pDesc++;
			pDesc = _fstrchr (pDesc,',');
			pDesc++;
			pDesc = _fstrchr (pDesc,',');
			pDesc++; 
			pName = pDesc;
			pDesc = _fstrchr (pDesc,',');
			*pDesc++ = 0;    
			pSym = pDesc;
			pSym = _fstrchr (pDesc,',');
			*pSym++ = 0;
				
			GetDMS (lat,&Deg,&Min,&Sec); 
			sprintf (LatC,"N%2.2iD %2.2iM %04.1fS",abs(Deg),Min,Sec);
			GetDMS (lon,&Deg,&Min,&Sec); 
			sprintf (LonC,"W%2.2iD %2.2iM %04.1fS",abs(Deg),Min,Sec);
			sprintf (str,"%.6s\t%.8s\t%.20s\t\t%s\t%s",pName,"Fish",pDesc,
														 LatC,LonC);  
			savec = str[6];   
			str[6]=0;
	 		index = SendDlgItemMessage (hWndDlg,Control,LB_FINDSTRING,(WPARAM)-1,(LPARAM) str); 
	 		str[6] = savec;
	 		if (index != LB_ERR)
	 		{
	 			SendDlgItemMessage (hWndDlg,Control,LB_DELETESTRING,(WPARAM)index,(LPARAM)0); 
	 			SendDlgItemMessage (hWndDlg,Control,LB_INSERTSTRING,(WPARAM)index,(LPARAM) str); 
	 		}
	 		else
	 			index = SendDlgItemMessage (hWndDlg,Control,LB_ADDSTRING,(WPARAM)0,(LPARAM) str); 
	 		SendDlgItemMessage (hWndDlg,Control,LB_SETTOPINDEX,(WPARAM)index,(LPARAM)0); 
	 		sprintf (str,"%ld points loaded",++nLoaded);
	 		SetDlgItemText (hWndDlg,StatusControl,str);
	        WPNum++;
			StatusWindowUpdate ("Waypoint transfer",str, 0,0);
		}
		GetRaytheonPacket (Packet);
	} 
	DestroyStatusWindow ();
	SetDlgItemText (hWndDlg,StatusControl,"Transfer complete");
	ContinueProcessing = TRUE;
//	SetCursor (hcurSave);
	return TRUE;
} 

short GPSExportRaytheon (HWND hWndDlg,UINT Control,UINT StatusControl)
{
	BYTE	Cmd;
	Merc	T;
	char	str[256],Packet[256],dirlat,dirlon,Sym='a';	 
	HCURSOR	hcurSave;    
	HANDLE	hItems;
	LPINT	lpItems; 
	LPSTR	pLat,pLon, pTAB, pOpts, pSym, pDesc, pRef;
	double	lat,lon, minlat,minlon,decpart;
	short	Err, n, nItems, deglat,deglon;    
	long	nRecs, nLoaded=0; 
	long	Ref;
	
	nItems=(short)SendDlgItemMessage(hWndDlg ,Control,LB_GETSELCOUNT,0,0); 
	hItems=GSSiGlobAlloc (1043,GMEM_MOVEABLE,nItems*2);
	lpItems=  (LPINT) GlobalLock(hItems);
	SendDlgItemMessage(hWndDlg ,Control,LB_GETSELITEMS,nItems,(LPARAM)lpItems);
	GlobalUnlock (hItems);
Top:    
	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
	lpItems=  (LPINT) GlobalLock(hItems);
	
	n = nRecs = nItems;
	nLoaded = 0;
	while (n--)
	{   
		SendDlgItemMessage (hWndDlg,Control,LB_GETTEXT,*lpItems++,(LPARAM)str);
		if ((pRef = _fstrrchr (str,'\t'))) 
		{
			*pRef++ = 0;
			Ref = atol (pRef);
		} 
		else
			Ref=1;
		pTAB = _fstrchr (str,'\t');
		*pTAB++ = 0;
//		_fstrncpy (Wpt_ident,str,6);
//		PadString (Wpt_ident,' ',6);
		pSym = pTAB;
		pTAB = _fstrchr (pSym,'\t');
		*pTAB++ = 0;
		pDesc = pTAB;
		pTAB = _fstrchr (pDesc,'\t');
		*pTAB++ = 0; 
//		if (Ref != -1) //indicate track log
		{ 
			pOpts = pTAB;
			pTAB = _fstrchr (pOpts,'\t');
			*pTAB++ = 0; 
		} 
		pLat = pTAB;
		pTAB = _fstrchr (pLat,'\t');
		*pTAB++ = 0;
		pLon = pTAB;
		lat = DecDegFromDMS (pLat,&Err);
		deglat = fabs (lat);    
		if (lat < 0)
			dirlat = 'S';
		else
			dirlat = 'N';
		decpart = fmod (fabs(lat),1.0);
		minlat = 60.0 * decpart;
		lon = DecDegFromDMS (pLon,&Err); 
		deglon = fabs (lon);     
		if (lon < 0)
			dirlon = 'W';
		else
			dirlon = 'E';
		decpart = fmod (fabs(lon),1.0);
		minlon = 60.0 * decpart;
		sprintf (Packet,"$PMGNWPL,%2.2i%06.3f,%c,%3.3i%06.3f,%c,,,%s,%s,%c",
						deglat,minlat,dirlat,
						deglon,minlon,dirlon,
						str,pDesc,Sym);
		if (!SendMagellanPacket (Packet))  
		{ 
	ErMsg:  
			GSSiSetCursor (hcurSave);
			if (MessageBox (hWndDlg,"The GPS unit is not responding. Be sure it is turned on, the cable is connected and the baud rate and output format are properly set.",
				NULL,MB_ICONEXCLAMATION|MB_RETRYCANCEL)==IDRETRY)
				goto Top;
		   	return FALSE; 
		}  
        PctBox (GetDlgItem(hWndDlg,StatusControl),nRecs,++nLoaded,-1);
	} 
	SetDlgItemText (hWndDlg,StatusControl,"Transfer complete");
	GSSiSetCursor (hcurSave); 
	GSSiGlobUlFree (&hItems);
	return nItems;
}

BOOL LoadBMP (HWND hWnd,LPSTR InFile,LPSTR OutFile,BOOL ShowMessage,LPSTR Ext)
{
	BOOL	rtn;
	FARPROC lpfnLOADBMPMsgProc;
	
	_fstrcpy (LoadBMPInFile,InFile);
	_fstrcpy (LoadBMPOutFile,OutFile);
	_fstrcpy (LoadBMPExt,Ext);   
	LoadBMPShowMess = ShowMessage;
	lpfnLOADBMPMsgProc = MakeProcInstance((FARPROC)LOADBMPMsgProc, hInst);
	rtn = DialogBox(hInst, (LPSTR)"LOADBMP", hWnd, lpfnLOADBMPMsgProc);
	FreeProcInstance(lpfnLOADBMPMsgProc);

	return rtn;
}

BOOL FAR PASCAL LOADBMPMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 
	char	str[512];
	HANDLE hI; 
 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
 	return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
		 cwCenter(hWndDlg, 0); 
		 SetDlgItemText (hWndDlg,IDC_MESSAGE,LoadBMPInFile);
    	 PostMessage(hWndDlg, WM_COMMAND, IDC_LOADFREEIMAGEBMP, 0L);
         break; 
    
    case WM_PAINT:
         if (!LoadBMPShowMess) 
         {
         	ShowWindow (hWndDlg,SW_HIDE);  
         	return 0;
         } 
         return 0;
    	 break;
    	 
    case WM_CLOSE:
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; 

    case WM_COMMAND:
         switch(wParam)
         {  
            case IDC_LOADFREEIMAGEBMP:
            	sprintf (str,"%s %s;%s;%s;%ld","[%DL]loadbmp.exe",LoadBMPInFile,LoadBMPOutFile,LoadBMPExt,(long)hWndDlg);
               	ExpandText (str);
				hI = WinExec (str,SW_SHOWNORMAL);
				if (hI <32)
				{
					DisplayShellExError (hI,str);
					PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
				} 
            	break;
            case IDOK:
                EndDialog(hWndDlg, TRUE);
            	break;
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);

            break;
         }
         break; 

    default:
        return FALSE;
   }
 return TRUE;
}

BOOL WritePickedItem (int FidOut, int Item)
#if ENABLETRACE
{GSSiEnterProg (962);
#endif
{	LPSHORT		ipnt, EndItem;
    HANDLE 		hpltBuf;
	LPSTR		LPpltBuf;
	LPITEM		ItemHeader;
	OFSTRUCT	OFStruct;
	UINT		Length, nRead;

	if (Item > NumPicked)
{
#if ENABLETRACE
GSSiExitProg (962);
#endif
		return FALSE;
}
    GetPickName (Item);

	if (!PickName[0])
{
#if ENABLETRACE
GSSiExitProg (962);
#endif
		return FALSE;
}
	_fstrcpy (PltName,PickName);

	CloseMap (TRUE);
	if (!OpenMap (CurView->hWnd,CurView->hDC))
{
#if ENABLETRACE
GSSiExitProg (962);
#endif
		return FALSE;
}
    _llseek (FidMap,PickList[Item].Segment,0);
    nRead = _lread (FidMap,&nBytes,2);
    hpltBuf = GSSiGlobAlloc ( 759,GMEM_MOVEABLE,(DWORD)nBytes);
    LPpltBuf = GlobalLock (hpltBuf);
    nRead = _lread (FidMap,LPpltBuf,nBytes);
    if (nRead != nBytes || PickList[Item].Offset > nRead) 
    {
    	InvalidItem (NULL,TRUE);
{
#if ENABLETRACE
GSSiExitProg (962);
#endif
    	return FALSE;
}
    }
    ipnt = (LPSHORT)(LPpltBuf + PickList[Item].Offset);
    ItemHeader = (LPITEM)ipnt;
    if (InvalidItem (ItemHeader,TRUE))
{
#if ENABLETRACE
GSSiExitProg (962);
#endif
    	return FALSE;
}
    EndItem = ipnt + abs(ItemHeader->Len);
    EndItem+=6;  
    Length = (LPSTR) EndItem - (LPSTR) ipnt;
    BigWrite (FidOut,(HPSTR)&PickList[Item].Refno,4,-1);
    BigWrite (FidOut,(HPSTR)&Length,2,-1);
    BigWrite (FidOut,(HPSTR)ipnt,Length,-1);  
    GSSiGlobUlFree (&hpltBuf);
{
#if ENABLETRACE
GSSiExitProg (962);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
}  
BOOL MoveIntersection_OLD (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (104);
#endif
{
 char key;
 POINT	MousePoint, CursorPoint; 
 DPOINT	MousePointW;
 static	POINT	StartPoint;
 DPOINT	BasePoint;
 static	int	NumItems;
 int	xmove, ymove, ii;
 static HANDLE	hOriginalItems, hCurrentItems;
 HANDLE	hSave;
 LPTHEME	pTheme;
 BOOL	ShowMoves=FALSE;//GetGlobalBVal2 ("[%SHOWMOVES]",FALSE);    
 static	HANDLE	SaveH1, SaveH2; 
 static	DPOINT	SavePPB;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		NumPicked = NumItems = 0;
   		hCurrentItems = NULL;
       	SaveHighlightList (&SaveH1,&SaveH2); 
   		ClearHighlightList(FALSE); 
   		SetPrompt (PRMT_MOVEINT1,TRUE);  

   		break;
    
    	
    case WM_LBUTTONDOWN:
    	break;
    case GF_CLOSE:  
		DeleteSavedItems(hCurrentItems); 
		if (hCurrentItems != hOriginalItems)
			DeleteSavedItems(hOriginalItems);
		hCurrentItems = hOriginalItems = NULL;
    	RestoreHighlightList (SaveH1, SaveH2);
    	return FALSE;
        break;
        
    case WM_LBUTTONUP:
    {
    	short	SaveMaxPick, i;
    	
		i = NumPicked;
	    while (i > 0)
	    {   
	    	i--;
		    RemoveFromHighlightList (PickList[i].Refno,2);
	    	ShowPickedItem (hWndMain,i); 
		    RemoveFromHighlightList (PickList[i].Refno,0);
	    	ShowPickedItem (hWndMain,i); 
	    }
   		ClearHighlightList(FALSE); 
   		if (Message == GF_CLOSE)
{
#if ENABLETRACE
GSSiExitProg (104);
#endif
   			return FALSE;
}
    	MousePoint = MAKEPOINT(lParam);
	    BasePoint=WinPtToBasePt(MousePoint); 
	    DoPaint=FALSE; 
	    WantOnlyShapePoints = TRUE;
	    PickItems (hWnd,BasePoint); 
	    WantOnlyShapePoints = FALSE;
		DoPaint=TRUE; 
		if (!NumPicked)
			break;
		if (MoveShapePoints)
			PickPointBase = PickList[0].PickedPoint;
		else
		{
			if (ldistp (PickPointBase,PickList[0].BeginPoint) <
				ldistp (PickPointBase,PickList[0].EndPoint))
				PickPointBase = PickList[0].BeginPoint;
			else
				PickPointBase = PickList[0].EndPoint;
		}
	    StartPoint = BasePtToWinPt (&PickPointBase); 
	    MousePoint = StartPoint;
        ClientToScreen (CurView->hWnd,(LPPOINT)&MousePoint);
	    SetCursorPosGM (MousePoint.x,MousePoint.y,0);
		SavePPB = PickPointBase;
		i = NumItems = NumPicked;
	    while (i--)
	    {
		    AddToHighlightList (PickList[i].Refno,&PickList[i],TRUE);
	    	ShowPickedItem (hWndMain,i); 
		    RemoveFromHighlightList (PickList[i].Refno,2);
	    }
		hOriginalItems = CreateSaveList(NumItems);
		hCurrentItems = hOriginalItems;
		MovePickListToSaveList (hOriginalItems); 
   		SetPrompt (PRMT_MOVEINT2,TRUE);  
	}
		break;

    case WM_MOUSEMOVE:
    	if (!ShowMoves) break;
    case WM_RBUTTONUP:
    	if (!NumItems) break;
    	MousePoint = MAKEPOINT(lParam);
	    if (CursorIsLocked)
	    { 
	    	MousePointW = CurrentPoint;
	    	UnlockCursor ();
			MousePoint = BasePtToWinPt (&MousePointW);
			CursorPoint = MousePoint;
        	ClientToScreen (CurView->hWnd,(LPPOINT)&CursorPoint);
			SetCursorPosGM (CursorPoint.x,CursorPoint.y,0);
	    }
	    else
			MousePointW = WinPtToBasePt(MousePoint);   
		EnlargeScreen (0,0);
		PickPointBase = SavePPB;
	    xmove = MousePoint.x - StartPoint.x ;
	    ymove = MousePoint.y - StartPoint.y;
    	if (Message == WM_RBUTTONUP || wParam == MK_RBUTTON)
    	{   
    		MoveOnlyPickedPoint = GetGlobalLVal2 ("[%MOVEONLYPICKEDPOINT]",1);
    		pTheme = AddTheme (GF_UNDRAW_THEME);
    		DrawSavedItems(hCurrentItems,NULL);	/* Undraws current position */
    		DeleteTheme (pTheme);  
    		ClearHighlightList(FALSE);
    		if (hCurrentItems != hOriginalItems) DeleteSavedItems(hCurrentItems);
    		hCurrentItems = CreateSaveList(NumItems);
    		pTheme = AddTheme (GF_MOVE_POLY_THEME);
    		pTheme->Xmove = xmove;
    		pTheme->Ymove = -ymove;
    		pTheme->ClassPnt[0] = MousePointW;
    		DrawSavedItems(hOriginalItems,hCurrentItems);	/* Draws new position and saves as current*/
    		hSave = hOriginalItems;
    		hOriginalItems = hCurrentItems;
    		hCurrentItems = hSave;
    		DeleteTheme (pTheme);

    	}
    	if (Message == WM_RBUTTONUP)
    	{   WriteSavedItems (hCurrentItems);
    		NumItems = 0;
			DeleteSavedItems(hCurrentItems);
			DeleteSavedItems(hOriginalItems);
   			hCurrentItems = NULL;
   			hOriginalItems = NULL; 
   			return (GF_INCREASE_SUCCESS_COUNT); 
   		}

    	break;


    default:
{
#if ENABLETRACE
GSSiExitProg (104);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (104);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

void FillFontNameList (HWND hWndDlg,UINT ListBox)
#if ENABLETRACE
{GSSiEnterProg (410);
#endif
{   
	HDC hDC=GetDC (hWndDlg);
	FONTENUMPROC lpEnumFamCallBack; 
	int aFontCount[] = { 0, 0, 0 };
	
	FontListhWndDlg = hWndDlg;
	FontListCntl = ListBox;
	lpEnumFamCallBack = (FONTENUMPROC) MakeProcInstance(
	    (FARPROC) EnumFontFamProc, hInst);
	EnumFontFamilies(hDC, (LPCSTR)NULL, lpEnumFamCallBack, (LPSTR) aFontCount);
	FreeProcInstance((FARPROC) lpEnumFamCallBack);
	ReleaseDC (hWndDlg,hDC);
{
#if ENABLETRACE
GSSiExitProg (410);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 
int CALLBACK EnumFontFamProc(lpnlf, lpntm, FontType, lParam)
	LOGFONT FAR* lpnlf;	/* address of structure with logical-font data	*/
	TEXTMETRIC FAR* lpntm;	/* address of structure with physical-font data	*/
	int FontType;	/* type of font	*/
	LPARAM lParam;	/* address of application-defined data	*/
#if ENABLETRACE
{GSSiEnterProg (407);
#endif
{
	if (FontType == TRUETYPE_FONTTYPE)              
		SendDlgItemMessage (FontListhWndDlg,FontListCntl,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)lpnlf->lfFaceName));
{
#if ENABLETRACE
GSSiExitProg (407);
#endif
	return 1;
}
#if ENABLETRACE
}
#endif
} 

RECT SizeReport (HDC hDC,HANDLE hReport, RECT CurRect, BOOL FitToVP)
{
	LPREPORT	pReport;
	RECT		Rect;
	int			irow, itab, MaxRowLen=0, ReportHeight=0, RowHeight, ReportWidth, ifont;  
	LPSTR		pRow, Tabloc, StartTab, EndTab, pFirstRow, Tabstr;
	LPLONG		startrow, pRows;
	char		str[1024], tabstr[16], FontStr[32];  
	DWORD		TextExt; 
	HFONT		OldFont;
	
	pReport = (LPREPORT)GlobalLock (hReport); 
	if (pReport->Type == 2 && !FitToVP)
	{
		GlobalUnlock (hReport);
		return (CurRect);
	}
	GlobalUnlock (hReport);
	OpenReportFiles (hReport);   
	pReport = (LPREPORT)GlobalLock (hReport);
	CreateReportFonts(pReport,1.0);

	_fstrcpy (FontStr,"[%FONT]=0;");
	ExpandText (FontStr);
    OldFont = SelectObject(hDC, GetStockObject(SYSTEM_FONT));
    SelectObject (hDC,OldFont);
    if (pReport->hRows)
    {
		pRows = (LPLONG)GlobalLock (pReport->hRows); 
		pFirstRow = (LPSTR) (pRows + pReport->NumRows);
		for (itab = 0;itab<pReport->NumTabs+1;itab++)
			pReport->TabLen[itab]=0;
			
		for (irow = 0;irow<pReport->NumRows;irow++)
		{   
			startrow = pRows;
			startrow += irow;  
			RowHeight = 0;
			pRow = pFirstRow + *startrow; 
			StartTab = pRow; 
			Tabloc = _fstrstr (StartTab,"$TAB(");
			if (Tabloc)
				*Tabloc = '\0';
			itab = 0;   
			tabstr[0]='\0';
			_fstrcpy (str,StartTab);
			while (Tabloc)
			{   
				ExpandText (str); 
				Truncate (str);
				if (!str[0])
					_fstrcpy (str," "); 
				_fstrcpy (FontStr,"[%FONT]");
				ExpandText (FontStr);
				ifont = atoi (FontStr); 
				if (ifont)
					SelectObject (hDC,pReport->hFonts[ifont-1]);
				else
				    SelectObject (hDC,OldFont);
				Tabstr = (LPSTR)(Tabloc+5);
	            EndTab = _fstrchr (Tabstr,')');
	            if (EndTab)
	            {
	            	*EndTab = '\0';
	            	_fstrcpy (tabstr,Tabstr);
	            	*EndTab = ')';
	            }
				_fstrcat (str,tabstr);
				TextExt = GetTextExtent (hDC,str,_fstrlen(str));
	            pReport->TabLen[itab] = max (pReport->TabLen[itab],LOWORD(TextExt));
	            RowHeight = max (RowHeight,HIWORD(TextExt)); 
	            *Tabloc = '$';
	            if (EndTab)
	            {
	            	StartTab = EndTab+1;            
					Tabloc = _fstrstr (StartTab,"$TAB(");
					if (Tabloc)
						*Tabloc = '\0'; 
					_fstrcpy (str,StartTab);
				}
	            else 
	            {
	            	str[0]='\0';
	            	Tabloc = 0; 
	            }
	            itab++;
			}
			ExpandText (str); 
			Truncate (str);
			if (!str[0])
				_fstrcpy (str," "); 
			_fstrcpy (FontStr,"[%FONT]");
			ExpandText (FontStr);
			ifont = atoi (FontStr); 
			if (ifont)
				SelectObject (hDC,pReport->hFonts[ifont-1]);
			else
			    SelectObject (hDC,OldFont);
			TextExt = GetTextExtent (hDC,str,_fstrlen(str));
			if (itab) 
			{
	            pReport->TabLen[itab] = max (pReport->TabLen[itab],LOWORD(TextExt));
	            RowHeight = max (RowHeight,HIWORD(TextExt)); 
			}
			else
			{
	            MaxRowLen = max (MaxRowLen,LOWORD(TextExt));
	            RowHeight = HIWORD(TextExt);
			}
	        ReportHeight += RowHeight;
		} 
		GlobalUnlock (pReport->hRows); 
	}
	ReportWidth = 0;
	for (itab = 0;itab<pReport->NumTabs+1;itab++)
		ReportWidth += pReport->TabLen[itab]*DeviceToScreenFactor; 
	ReportWidth = max (ReportWidth,MaxRowLen);  
	ReportWidth *= (1.0 + 2 * pReport->Margin);
	ReportHeight *= (1.0 + 2 * pReport->Margin);
	Rect.left=0;
	Rect.top=0;
	Rect.bottom=ReportHeight;
	Rect.right=ReportWidth; 
	pReport->Height = ReportHeight;
	pReport->Width = ReportWidth;
    SelectObject (hDC,OldFont);
	DestroyReportFonts(pReport);
	CloseReportFiles (hReport);
	GlobalUnlock (hReport);
	return Rect;
}

BOOL CopyGWDatabase (LPSTR ToName, LPSTR FromName, BOOL CopyData)
#if ENABLETRACE
{GSSiEnterProg (628);
#endif
{   HANDLE DBHandle;
    LPGWDHEADER lpGWDHead;
    GWDHEADER   GWDHead;
    LPGWFLDINFO lpFieldInfo;
    OFSTRUCT    OFStruct, OFStruct2;
    char        IndexName[256];
    short       Fid, i, pos=BT_FIRST, length;
    long		Offset, ii;
    LPSTR       lpEnd;
    unsigned    frequency=1000, duration=100; 
    BTHEAD      BTHead;
    LPSTR		lpDot;     
    HFILE		FidFrom, FidTo;
char	HighlightData[1024];   
LPLONG	pRefno;
    
    FidFrom =  GSSiOpenFile (FromName,&OFStruct,OF_READ);
    if (FidFrom == HFILE_ERROR)
    {   
{
#if ENABLETRACE
GSSiExitProg (628);
#endif
         return (0);
}
    }
    FidTo =  GSSiOpenFile (ToName,&OFStruct2,OF_CREATE);
    if (FidTo == HFILE_ERROR)
    {   
{
#if ENABLETRACE
GSSiExitProg (628);
#endif
         return (0);
}
    }
    _lread (FidFrom,&GWDHead,sizeof(GWDHEADER));
    BigWrite (FidTo,(HPSTR)&GWDHead,sizeof(GWDHEADER),-1);
    DBHandle = GSSiGlobAlloc ( 267,GHND,sizeof (GWDHEADER)+GWDHead.Reclen+4);
    lpGWDHead =(LPGWDHEADER) GlobalLock (DBHandle);
    *lpGWDHead = GWDHead;
    lpGWDHead->Fid =  FidFrom;
    lpGWDHead->hFldInfo = GSSiGlobAlloc ( 268,GHND,lpGWDHead->NumFields*sizeof(FIELDINFO));
    lpGWDHead->pFldInfo = (LPGWFLDINFO)GlobalLock(lpGWDHead->hFldInfo); 
    for (i=0,lpFieldInfo=lpGWDHead->pFldInfo;i<GWDHead.NumFields;i++,lpFieldInfo++)
    {
         _lread (FidFrom,lpFieldInfo,sizeof(GWFLDINFO));
         BigWrite (FidTo,(HPSTR)lpFieldInfo,sizeof(GWFLDINFO),-1);
    }
    for (i=0;i<MAX_GMD_INDEXES;i++) lpGWDHead->BTHandle[i]=0;
    for (i=0;i<1;i++)
    {
        _fstrcpy (IndexName,OFStruct.szPathName);
        _fstrlwr (IndexName); 
        lpDot = _fstrstr (IndexName,".gmd");
        if (!lpDot) lpDot = _fstrstr (IndexName,".gwd");
        if (!lpDot)
{
#if ENABLETRACE
GSSiExitProg (628);
#endif
        	return 0;
}
        _fstrcpy (lpDot,".in");
        itoa (i+1,_fstrchr(IndexName,'\0'),10);
        if (!(lpGWDHead->BTHandle[i] = BT_OPEN (IndexName, lpGWDHead->TimeStamp,OF_READ, 0))) 
        {   
			MessageBox (GetFocus(),"Unable to open source file",NULL,MB_ICONEXCLAMATION);
			GSSiClose (FidFrom);
			GSSiClose (FidTo);
{
#if ENABLETRACE
GSSiExitProg (628);
#endif
			return FALSE;
}
        }
        
        GetBTHeader (lpGWDHead->BTHandle[i],&BTHead); 
        if (!lpGWDHead->lKeys[i] || abs (lpGWDHead->lKeys[i]>256))
            lpGWDHead->lKeys[i] = ComputeGWDKeyLen(lpGWDHead,i);
        lpGWDHead->hKeys[i]=GSSiGlobAlloc ( 269,GHND,abs(lpGWDHead->lKeys[i]));
        lpGWDHead->pKeys[i]=GlobalLock(lpGWDHead->hKeys[i]);
        if (!lpGWDHead->pKeys[i])
            ii=0;
    }
    while (!BT_FIND (lpGWDHead->BTHandle[0],lpGWDHead->pKeys[0],pos,BT_ANY, (LPSTR)&Offset))
    { 
    	pos = BT_NEXT;
       	length = FillGWDData (lpGWDHead,Offset); 
     pRefno =  (LPLONG)&lpGWDHead->GWDData;
	 if (!BT_FIND (hHighlight,(LPSTR)pRefno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData)) 
	 {
       	ii = _llseek (FidTo,0,1);
	    BigWrite (FidTo,(HPSTR)&length,2,-1);
	    BigWrite (FidTo,(HPSTR)&lpGWDHead->GWDData,length,-1); 
	 }
    }
    BT_CLOSE (lpGWDHead->BTHandle[0]);
    GSSiGlobUlFree (&DBHandle);
    GSSiClose (FidFrom);
    GSSiClose (FidTo);
{
#if ENABLETRACE
GSSiExitProg (628);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
}

short CheckLMCode (LPSTR pSNO,BOOL Set)
{   
	OFSTRUCT	OFStruct; 
	BOOL	rtn=0;  
	char	str[128];
	HFILE	Fid;  
	LPSTR	pC;
	long	loc=0;
	UINT	Mode=OF_READ; 
	long	Line=0,ii;
	
	if (Set)
		Mode = OF_READWRITE;
		
	if ((Fid = OpenFile ("c:\\waypoint\\lmserno.txt",&OFStruct,Mode)) == HFILE_ERROR)
		return FALSE;  
	while (fgetstring (str,64,Fid)) 
	{   
		Line++;
		if (Line > 8590)
			ii=1;
		if ((pC = _fstrchr (str,':')))
		{
			*pC++ = 0;
			if (!_fstricmp (str,pSNO))
			{   
				if (Set)
				{   
					pC--;
					_fstrcpy (pC,":Y"); 
					_llseek (Fid,loc,0);
					fputstring (str,Fid);
				}
				if (*pC == ' ')
					rtn = 1;
				else
					rtn = 2;
				break;
			}         
		}
		loc = _llseek (Fid,0,1);
	}
	_lclose (Fid);
	return rtn;
}
short DupDLLs (void)
{ 	
	struct	_find_t	FileInfo; 
	char	str[144], Name[144], ToDir[144], ToName[144], WDir[144], DeleteFile[144], mess[256];
	short	rtn=0, NumErr=0,st;
	BTVARDESC	BTVar[3];
	int	i, ifield;  
	HFILE		hMessFile;
	OFSTRUCT	OFStruct;
	HANDLE	hBT;  
	struct {unsigned short date, time;} dant;
						
	GSSiGetTempFileName (NULL,"gm",NULL,(LPSTR)Name);

	BTVar[0].BT_VARTYP=BT_CHAR;
	BTVar[0].BT_VARLEN=16;
	BTVar[0].BT_VAROFF=0;
	BT_CREATE (Name, 4, FALSE, 1, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
	hBT = BT_OPEN (Name, 0, BT_WRITE, 0);
	
	_fstrcpy (ToDir,"[%INDIR]olddlls");
	ExpandText (ToDir);
	GSSiMakeDir (ToDir);	
	
	sprintf (Name,"%s\\readme.txt",ToDir);
	hMessFile = GSSiOpenFile (Name,&OFStruct,OF_CREATE);
	GetWindowsDirectory (str,144);
	_fstrcpy (WDir,str);
	_fstrcat (str,"\\*.dll");
	st = _dos_findfirst (str,_A_RDONLY|_A_SYSTEM|_A_HIDDEN,&FileInfo);
	while (!st)
	{   
		_fstrncpy (Name,FileInfo.name,16); 
		dant.date = FileInfo.wr_date;
		dant.time = FileInfo.wr_time;
		BT_PUT (hBT,(LPSTR)Name,(LPSTR)&dant);
		rtn++;
		st = _dos_findnext (&FileInfo);
	}
	rtn = 0;
	GetWindowsDirectory (str,144);
	_fstrcat (str,"\\system\\*.dll");
	st = _dos_findfirst (str,_A_RDONLY|_A_SYSTEM|_A_HIDDEN,&FileInfo);
	while (!st)
	{   
		_fstrncpy (Name,FileInfo.name,16); 
		if (!BT_FIND (hBT,(LPSTR)Name,BT_FIRST,BT_EQ,(LPSTR)&dant))
		{
			if (dant.date < FileInfo.wr_date)
				sprintf (DeleteFile,"%s\\%s",WDir,Name);
			else
				sprintf (DeleteFile,"%s\\system\\%s",WDir,Name); 
			sprintf (ToName,"%s\\%s",ToDir,Name);
			copyfile (ToName,DeleteFile,FALSE,0,0,0,0,0,0); 
			if (GSSiRemove (DeleteFile))
			{
				NumErr++;
				sprintf (mess,"Unable to remove %s",DeleteFile);
			}                                                   
			else
				sprintf (mess,"Successfully removed %s",DeleteFile);
			rtn++;
			fputstring (mess,hMessFile);  
		}
		st = _dos_findnext (&FileInfo);
	} 
	BT_CLOSEANDDELETE (&hBT);     
	GSSiClose (hMessFile);
	sprintf (mess,"%i duplicates found - unable to remove %i",rtn,NumErr);
	MessageBox (GetFocus(),mess,"",MB_OK);
	return rtn;
}
HANDLE	GetConnectedItems_old (long StartSequence, LPLONG pEndSequence, LPWORD pNumPoints)
#if ENABLETRACE
{GSSiEnterProg (718);
#endif
{
	HPDPOINT	pPoints;
	short	pos;
	HANDLE	hPoints;   
	HIGHLIGHTDATA	HighlightData1,HighlightData2; 
	DPOINT	OpenEnd;  
	long	Refno1, Refno2, Sequence; 
	BOOL	First=TRUE, Reverse1=FALSE, Reverse2=FALSE;  
	double	D1, D2, MinD;

    if (!TotHLTPoints || !hHighlight)
{
#if ENABLETRACE
GSSiExitProg (718);
#endif
    	return 0;
}
    hPoints = GSSiGlobAlloc ( 329,GMEM_MOVEABLE,(long)TotHLTPoints*sizeof(DPOINT)); 
    pPoints = (HPDPOINT)GlobalLock (hPoints);
    *pNumPoints = 0;    
	BT_FIND (hHighlight2,(LPSTR)&StartSequence,BT_FIRST,BT_ANY,(LPSTR)&Refno1);
	BT_FIND (hHighlight,(LPSTR)&Refno1,BT_FIRST,BT_EQ,(LPSTR)&HighlightData1); 
	while (!BT_FIND (hHighlight2,(LPSTR)&Sequence,BT_NEXT,BT_ANY,(LPSTR)&Refno2))
	{ 
		BT_FIND (hHighlight,(LPSTR)&Refno2,BT_FIRST,BT_EQ,(LPSTR)&HighlightData2); 
		if (First)
		{   
			First = FALSE;
			MinD = ldistp (HighlightData1.PD.EndPoint,HighlightData2.PD.BeginPoint); 
			D1 = ldistp (HighlightData1.PD.BeginPoint,HighlightData2.PD.BeginPoint);   
			if (D1 < MinD)
			{
				MinD = D1;
				Reverse1 = TRUE;
			}
			D1 = ldistp (HighlightData1.PD.BeginPoint,HighlightData2.PD.EndPoint);   
			if (D1 < MinD)
			{
				MinD = D1;
				Reverse1 = TRUE;
				Reverse2 = TRUE;
			}
			D1 = ldistp (HighlightData1.PD.EndPoint,HighlightData2.PD.EndPoint);   
			if (D1 < MinD)
			{
				MinD = D1;
				Reverse1 = FALSE;
				Reverse2 = TRUE;
			} 
		}
		else
		{
			D1 = ldistp (OpenEnd,HighlightData2.PD.BeginPoint); 
			D2 = ldistp (OpenEnd,HighlightData2.PD.EndPoint);    
			MinD = min (D1,D2);
			if (D1 < D2)
				Reverse2 = FALSE;
			else
				Reverse2 = TRUE;
		}
//		if (MinD > P_TOL)
//			break;
		PickList[0] = HighlightData1.PD;
		(*pNumPoints) += (GetPickItemPoints (0,Reverse1,&pPoints) - 1);  
		pPoints--;
		HighlightData1 = HighlightData2;
		Reverse1 = Reverse2;
		Refno1 = Refno2;
		if (Reverse1)
			OpenEnd = HighlightData1.PD.BeginPoint;
		else
			OpenEnd = HighlightData1.PD.EndPoint; 
		StartSequence = Sequence;
	}
	PickList[0] = HighlightData1.PD;
	(*pNumPoints) += GetPickItemPoints (0,Reverse1,&pPoints);
	GlobalUnlock (hPoints); 
	*pEndSequence = StartSequence + 1;
{
#if ENABLETRACE
GSSiExitProg (718);
#endif
	return hPoints;
}
#if ENABLETRACE
}
#endif
}
void CreateStateLev (short State,HWND hWndDlg,int StatusCntl,int MessCntl)
{
	BTVARDESC	BTVar[3];
	long		nRecs, nLoaded, TLID;
	HDC			hDC;
	int			stSeg, st2;
	BOOL		RtnVal=FALSE;
	long			SegMaxData=0, Offset, TotLen, Processed=0;
	time_t ltime;
	HANDLE		hStname1, hStname2, hStname, hSkipTLID;
	char		name[34], File[128], mess[128], str[256]; 
	HFILE		StateLevFID; 
	OFSTRUCT	OFStruct;
    HANDLE  hPointList[2], hTotPoints,hIntersect; 
	struct {
	     long   TLID,
	            SNums[4];
	     DPOINT BeginPoint,
	            EndPoint;
	     char   CFCC[3];
	     } StateLev;
	struct	{
				long Long, Lat, TLID;
			}	IntersectKey; 
	struct	{
				long	Snum;
				long	OPLong, OPLat;
			}	IntersectData;
    LPLPOINT PointList;
    LPDPOINT TotPoints;
    long    snum, IntLong, IntLat, IntTLID, ToLong, ToLat, AtTLID, WantStreet;
    short NumPoints[2], NumShapePoints, pos,PointListID, idum, st, i;
    long    Loc=0, Nrecs=0;  
    short	NumSyms=0, n, SymNum;  
    char	SymName[16];
    HANDLE	hSymDesc=0; 
    MNMXCORD MinMaxCoord; 
    long	NewRefno=1; 
    short	Stuff[64];
    long	StreetNums[4]; 
    HFILE	FidStateLev;
			
	HCURSOR	hcurSave;
    
    CurState = State;
	StateLevFID = GSSiOpenFile("[STATE]\\statelev",&OFStruct,OF_READ);
	if (StateLevFID == HFILE_ERROR)
		return;
    FidStateLev = GSSiOpenFile ("[STATE]\\global.ini",&OFStruct,OF_CREATE); 
    sprintf (str,"[STATE]=%i",State);
    fputstring (str,FidStateLev);
    GSSiClose (FidStateLev);
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
	_fstrcpy (File,"intersec.btr");
	BT_CREATE (File, sizeof(IntersectData), FALSE, 3, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	ltime = 0;
	hIntersect = BT_OPEN (File, ltime, BT_WRITE, 0);  
    BTVar[0].BT_VARTYP=BT_INTEGER;
    BTVar[0].BT_VARLEN=4;
    BTVar[0].BT_VAROFF=0;
    BT_CREATE ("skip.btr", 2, FALSE, 1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
    hSkipTLID= BT_OPEN ("skip.btr", 0, BT_WRITE, 0);
		
    DBoundsInit (&MinMaxCoord);
    sprintf (PltName,"%2.2i\\statelev.plt",CurState); 
    TotLen = _llseek (StateLevFID,0,2);
    _llseek (StateLevFID,0,0);   
    sprintf (mess,"Scanning state %i",CurState);
	SetDlgItemText (hWndDlg,MessCntl,mess);	
    while (_lread (StateLevFID,&StateLev,sizeof(StateLev))==sizeof(StateLev))
    {   
    	if (StateLev.CFCC[1]=='9' && (StateLev.CFCC[2] > '3' && StateLev.CFCC[2] != '6'))
    		goto SkipTLID; 
    	_fmemmove (SymName,StateLev.CFCC,3); 
    	SymName[3]=0;
		SymNum = GetDictSymbolNumber (SymName);
		AddToSymList (SymNum,&NumSyms,&hSymDesc);
		AddDPointToMinMax (&StateLev.BeginPoint,&MinMaxCoord);
		AddDPointToMinMax (&StateLev.EndPoint,&MinMaxCoord);
   		IntersectKey.Long = IDNINT(StateLev.BeginPoint.x*1000000); 
   		IntersectKey.Lat = IDNINT(StateLev.BeginPoint.y*1000000); 
   		IntersectKey.TLID = StateLev.TLID;  
   		IntersectData.Snum = labs (StateLev.SNums[0]);
   		IntersectData.OPLong = IDNINT(StateLev.EndPoint.x*1000000);
   		IntersectData.OPLat = IDNINT(StateLev.EndPoint.y*1000000);  

    	BT_PUT (hIntersect,(LPSTR)&IntersectKey,(LPSTR)&IntersectData);
   		IntersectKey.Long = IDNINT(StateLev.EndPoint.x*1000000); 
   		IntersectKey.Lat = IDNINT(StateLev.EndPoint.y*1000000); 
   		IntersectKey.TLID = StateLev.TLID;
   		IntersectData.Snum = labs (StateLev.SNums[0]);
   		IntersectData.OPLong = IDNINT(StateLev.BeginPoint.x*1000000);
   		IntersectData.OPLat = IDNINT(StateLev.BeginPoint.y*1000000);
    	BT_PUT (hIntersect,(LPSTR)&IntersectKey,(LPSTR)&IntersectData);  
SkipTLID:
		Loc += sizeof(StateLev);
        PctBox (GetDlgItem(hWndDlg,StatusCntl), TotLen, Loc,0);
	}
		
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
	} 
    OpenMap (CurView->hWnd,CurView->hDC);
	EditBounds = CurView->FileMNMX;
    CloseMap (FALSE);
    hPointList[0] = GSSiGlobAlloc (GMEM_MOVEABLE,500*sizeof(LPOINT));
    hPointList[1] = GSSiGlobAlloc (GMEM_MOVEABLE,500*sizeof(LPOINT));
    hTotPoints = GSSiGlobAlloc (GMEM_MOVEABLE,1000*sizeof(DPOINT));   
    
    sprintf (mess,"Loading state %i",CurState);
	SetDlgItemText (hWndDlg,MessCntl,mess);	
    _llseek (StateLevFID,0,0); 
    Loc = 0;
    while (_lread (StateLevFID,&StateLev,sizeof(StateLev))==sizeof(StateLev))
    {   
    	if (StateLev.CFCC[1]=='9' && (StateLev.CFCC[2] > '3' && StateLev.CFCC[2] != '6'))
    		goto NextTLID;
        if (!BT_FIND (hSkipTLID,(LPSTR)&StateLev.TLID,BT_FIRST,BT_EQ,(LPSTR)&idum))
            goto NextTLID; 
        BT_PUT (hSkipTLID,(LPSTR)&StateLev.TLID,(LPSTR)&idum);
        TLID = StateLev.TLID;
    	_fmemmove (SymName,StateLev.CFCC,3); 
    	SymName[3]=0;
		SymNum = GetDictSymbolNumber (SymName);
        
        WantStreet = labs (StateLev.SNums[0]);
        ToLong = IDNINT(StateLev.EndPoint.x*1000000); 
        ToLat = IDNINT(StateLev.EndPoint.y*1000000); 
        IntLong = IDNINT(StateLev.BeginPoint.x*1000000);
        IntLat = IDNINT(StateLev.BeginPoint.y*1000000);  
        PointListID = 0;
NextLink:
        AtTLID = TLID;   
        NumPoints[PointListID] = 1;  
        PointList = (LPLPOINT) GlobalLock (hPointList[PointListID]);
NextPoint:
        BT_PUT (hSkipTLID,(LPSTR)&AtTLID,(LPSTR)&idum);
        PointList->x = IntLong;
        PointList->y = IntLat;
        if (NumPoints[PointListID] >=50)
            goto EndLink;
        IntersectKey.Long = IntLong; 
        IntersectKey.Lat = IntLat; 
        IntersectKey.TLID = 0;  
        st = BT_FIND (hIntersect,(LPSTR)&IntersectKey,BT_FIRST,BT_GT,(LPSTR)&IntersectData);
        while (!st)
        {    
        	snum = IntersectData.Snum;
            if (IntersectKey.Long != IntLong || IntersectKey.Lat != IntLat)
                goto EndLink;
	        if (BT_FIND (hSkipTLID,(LPSTR)&IntersectKey.TLID,BT_FIRST,BT_EQ,(LPSTR)&idum))
	        {
	            if (IntersectKey.TLID != AtTLID && snum == WantStreet)
	            {
	                IntLong = IntersectData.OPLong;
	                IntLat = IntersectData.OPLat;
	                AtTLID = IntersectKey.TLID;
	                NumPoints[PointListID]++;    
	                PointList++;
	                goto NextPoint;
	            }
	        } 
            st = BT_FIND (hIntersect,(LPSTR)&IntersectKey,BT_NEXT,BT_ANY,(LPSTR)&IntersectData);
        } 
EndLink:
        if (!PointListID)
        {   
            GlobalUnlock (hPointList[PointListID]);
            PointListID++;
            IntLong = ToLong;
            IntLat = ToLat;  
            goto NextLink;
        }
        
        PointList = (LPLPOINT)GlobalLock (hPointList[0]);
        PointList+=(NumPoints[0]-1);
        NumShapePoints = 0; 
        TotPoints =(LPDPOINT) GlobalLock (hTotPoints);
        while (NumPoints[0]--)
        {
            TotPoints->x = (double)PointList->x/1000000; 
            TotPoints++->y = (double)PointList--->y/1000000; 
            NumShapePoints++;
        }
        GlobalUnlock (hPointList[1]);

        PointList = (LPLPOINT)GlobalLock (hPointList[1]);
        while (NumPoints[1]--)
        {
            TotPoints->x = (double)PointList->x/1000000; 
            TotPoints++->y = (double)PointList++->y/1000000; 
            
            NumShapePoints++;
        }
        GlobalUnlock (hTotPoints); 
        StreetNums[0]=WantStreet;
        StreetNums[1]=0;
        StreetNums[2]=0;
        StreetNums[3]=0;
        Stuff[0]=18;
        Stuff[1]=10;
        _fmemmove (&Stuff[2],StreetNums,16);
                    
        AddPolyToMap (1,(LPINT)&NumShapePoints, &hTotPoints,1,NewRefno++,NULL,1,SymNum,Stuff,0,0,-1,-1,0,0,0,0,0,FALSE); 
        
NextTLID:;          
		Loc += sizeof(StateLev);
        PctBox (GetDlgItem(hWndDlg,StatusCntl), TotLen, Loc,0);
    } 
Exit: 
    GlobalFree (hPointList[0]);
    GlobalFree (hPointList[1]);
    GlobalFree (hTotPoints);
    BT_CLOSE (hSkipTLID);

	BT_CLOSE (hIntersect);
	GSSiClose (StateLevFID);
	GSSiSetCursor (hcurSave);  
	CloseMap (TRUE);
	return;
}

BOOL HalfToneViewport (void)
{   
    POINT   Points[4];
    HBRUSH  CurBrush, brush;
    HPEN    CurPen, pen;
    short   i; 
    long	Color[5] = {285212672,218103808,150994944,218103808,285212672};  
    LPRECT	Rect=&CurView->Rect;
    
    if (!CurView->HalfTone)
    	return FALSE;
    i = CurView->HalfTone; 
	if (DeviceToScreenFactor > 6)
		i++;
	i = max (1,min (i,5)) - 1;
    SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
    CurView->hRgn = CreateVPRgn(FALSE,FALSE);
    SelectClipRgn (CurView->hDC,CurView->hRgn);
    DeleteObject(CurView->hRgn);  
    brush = CreateGMBrush (Color[i],CurView->hDC);
    CurBrush = SelectObject (CurView->hDC,brush); 
	CurPen = SelectObject (CurView->hDC,GetStockObject(NULL_PEN)); 
    Points[0].x = Rect->left;
    Points[0].y = Rect->bottom;
    Points[1].x = Rect->left;
    Points[1].y = Rect->top;    
    Points[2].x = Rect->right;
    Points[2].y = Rect->top;
    Points[3].x = Rect->right;
    Points[3].y = Rect->bottom; 
    if (CurView->HalfTone > 2)
    	SetROP2(CurView->hDC,R2_MERGEPEN);  
    else
    	SetROP2(CurView->hDC,R2_MERGENOTPEN);  
    Polygon (CurView->hDC,Points,4); 
    if (CurBrush)
	    SelectObject (CurView->hDC,CurBrush);
	if (CurPen)                         
    	SelectObject (CurView->hDC,CurPen);  
    DeleteObject (brush);
    RestoreDC (CurView->hDC,-1);
	return TRUE;
}

short CreateDummyDelete (long NewRefno,LPSTR Prefix, short idesc,LPSTR UDI,HANDLE hUpdateBuf)
{   
	short	ltag=0; 
	short	item_len;
	char	tag[80];
	
	if (Prefix) 
	{   
		if (*Prefix && _fstricmp (Prefix,"REFNO"))
		{
			_fstrcpy (tag,Prefix);
			_fstrcat (tag,":");
			_fstrcat (tag,UDI);
			ltag = _fstrlen (tag);
			if (ltag%2) ltag++;
		}
	}
	item_len = 2+4+ltag + 2+2; 
	if (hTimeStamp)
		item_len += 10; 
	id = 12 + 256 * ipen;
	BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
	MinMaxLoc = pBuf;
	BufWrite(&pBuf,&lbuf,(HPSTR)&MinMax,8);
	item_len /= 2;
	if (item_len > 16000)
		itemlen=0;
	else
		itemlen = -item_len;
	BufWrite(&pBuf,&lbuf,(HPSTR)&itemlen,2); 
	id = 9 + 256 * ltag;
	BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
	BufWrite(&pBuf,&lbuf,(HPSTR)&NewRefno,4); 
	if (ltag)
		BufWrite(&pBuf,&lbuf,tag,ltag);
	id = 8;
	BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
	BufWrite(&pBuf,&lbuf,(HPSTR)&idesc,2);  
	if (hTimeStamp)
	{   
		char	str[256]; 
		long	StartTime, EndTime;
		LPTIMESTAMP	lpTimeStamp=(LPTIMESTAMP)GlobalLock (hTimeStamp); 
		
		id = 37;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		_fstrcpy (str,lpTimeStamp->StartTime);
		ExpandText (str);
		StartTime = atol (str); 
		if (StartTime == 1)     
		{
			time_t	systime;     
			time (&systime);
			StartTime = systime;
		}
		BufWrite(&pBuf,&lbuf,(HPSTR)&StartTime,4);
		_fstrcpy (str,lpTimeStamp->EndTime);
		ExpandText (str);
		EndTime = atol (str); 
		if (EndTime <= StartTime)
			EndTime = StartTime;
		BufWrite(&pBuf,&lbuf,(HPSTR)&EndTime,4);  
		GlobalUnlock (hTimeStamp);
		MinFileTime = min (MinFileTime,StartTime);
	    MaxFileTime = max (MaxFileTime,EndTime);		
	} 
	return TRUE;
}

BOOL IdentifyPolygonsTemp (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{HDC hDC;
 int	st,SaveMaxPick;
 LPGWDHEADER lpGWDHead;
 HIGHLIGHTDATA	HighlightData;         
 short	pos, len;  
 long	Refno, Offset;  
 HCURSOR	hcurSave;
 POINT	Point; 
 char	mess[128], PointID[128]; 
 LPPOLYID	pPI;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		break;
    
    case WM_LBUTTONUP:
		hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
		OpenPolyIDFile ("[%POLYIDFILE].gmd");
	    lpGWDHead = (LPGWDHEADER)GlobalLock (hDBPolyID);   
	    pPI = (LPPOLYID)&lpGWDHead->GWDData; 
		UseUserPickAp =FALSE;
		SystemPickAp = 1;	
		SaveMaxPick=MaxPick;
		MaxPick=1;
		SetPickAp();
	    pos = BT_FIRST;
		while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData)) 
		{   
			pos = BT_NEXT; 
			PickList[0] = HighlightData.PD;
			ProcessPickedItem (0,FALSE);
			_fstrcpy (PointID,"[SOILTYPE]");
			ExpandText (PointID);  
			PickItems2 (CurView->hWnd,HighlightData.PD.BeginPoint,FALSE,TRUE,TRUE);
			if (NumPicked)
			{ 
	        	if (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&PickList[0].Refno,BT_FIRST,BT_EQ,(LPSTR)&Offset))
		        {    
	        		len = FillGWDData (lpGWDHead,Offset);
	        		if (_fstrcmp (pPI->ID,PointID))
	        		{   
	        			pPI->MultipleID=1;
						GWDReplaceRecord (lpGWDHead,len,NULL,Offset); 
					}
				}
				else
				{   
					pPI->AreaRefno = PickList[0].Refno;
					pPI->MultipleID=0;
					_fstrncpy (pPI->ID,PointID,sizeof(pPI->ID));
			    	GWDAddRecord (lpGWDHead,0,NULL);   
			    }
			} 
NextRec:;
		} 

		UseUserPickAp =TRUE;
		MaxPick=SaveMaxPick;
		GlobalUnlock (hDBRefConnect);  
		
		ClearHighlightList (FALSE);
		ClosePolyIDFile ();
	    GSSiSetCursor(hcurSave); 
        PostMessage(hWnd, GF_CLOSE,0, 0L);
	
		break;
		
    default:
    	return (FALSE);
    }
    return (TRUE);
}   

void GetFunName (int id, LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (755);
#endif
{ 
    LPSTR	lpSpace;
	char CmdFName[64]="[%INDIR]cmdid", str[132];  
	HFILE	Fid; 
	long	ID;  
	OFSTRUCT	OFStruct;
		
	ExpandText (CmdFName);
	Fid=GSSiOpenFile (CmdFName,&OFStruct,OF_READ);  
	*Name=0;
	if (Fid)
	{
		while (fgetstring(str,128,Fid))
		{
			if (*str == 'G')
			{
				lpSpace = _fstrpbrk (str," \t");
				if (lpSpace)
				{
					*lpSpace++ = 0;
					ID = atol (lpSpace);
					if (ID == id)
					{
						_fstrcpy (Name,&str[3]); 
						break;
					}
				}
			}
		}
		GSSiClose (Fid);
	} 
#if ENABLETRACE
}
#endif
}    

/*BOOL ConvertGeospanData (HWND hWnd)
{

    SEGDATAGM	Segdata; 
	SEGDATAGS 	SegdataGS;
    LPGWDHEADER lpGWDHeadGM, lpGWDHeadGS;   
    HANDLE		hSegDataGM, hSegDataGS; 
    short		pos=BT_FIRST, len;
    long		TLID;    
    char		StreetName[40]; 
    long		Offset;   
    BOOL		OpenedSeg=FALSE;
    
	OpenAddressFiles (hWnd);
	if (!hDBSegdata)
		return FALSE;
        
	OpenStreetSegmentTable (TRUE,&OpenedSeg); 
						
    lpGWDHeadGM = (LPGWDHEADER)GlobalLock (hDBStreetSegments); 
    hSegDataGM = lpGWDHeadGM->BTHandle[0]; 
    
	lpGWDHeadGS = (LPGWDHEADER)GlobalLock (hDBSegdata); 
	hSegDataGS = lpGWDHeadGS->BTHandle[0];
    
    while (!BT_FIND (hSegDataGS,(LPSTR)&TLID,pos,BT_ANY,(LPSTR)&Offset))
    {   
    	pos = BT_NEXT; 
    	_llseek (lpGWDHeadGS->Fid,Offset,0);
    	_lread (lpGWDHeadGS->Fid,&len,2);
    	_lread (lpGWDHeadGS->Fid,&SegdataGS,len);
        Offset = _llseek (lpGWDHeadGM->Fid,0,2);
        BT_PUT (hSegDataGM,(LPSTR)&TLID,(LPSTR)&Offset);  
        _fmemset (&Segdata,0,sizeof(SEGDATAGM));
		Segdata.TLID = TLID;
		BT_FIND (hNames1,(LPSTR)&SegdataGS.street_num,BT_FIRST,BT_EQ,StreetName); 
		Segdata.StreetNum[0] = AddStreetName (StreetName,0,"","","",""); 
		Segdata.faddl = SegdataGS.faddl;
		Segdata.faddr = SegdataGS.faddr;
		Segdata.taddl = SegdataGS.taddl;
		Segdata.taddr = SegdataGS.taddr;
		Segdata.ZIPL = SegdataGS.ZIPLeft;
		Segdata.ZIPR = SegdataGS.ZIPRight;
		_fmemmove (Segdata.CTBNAL,SegdataGS.CTBNAL,12);
		Segdata.FMCDL = SegdataGS.FMCDL;
		Segdata.FMCDR = SegdataGS.FMCDR;
	  	_fmemmove (Segdata.CFCC,SegdataGS.CFCC,3);
	  	len = sizeof(SEGDATAGM);  
	  	BigWrite (lpGWDHeadGM->Fid,&len,2);
	  	BigWrite (lpGWDHeadGM->Fid,&Segdata,len); 
	}
	GlobalUnlock (hDBSegdata);
	GlobalUnlock (hDBStreetSegments);
    CloseAddressFiles(FALSE);
	CloseStreetNameTable();
	CloseStreetSegmentTable (OpenedSeg);   
	copyfile ("[%DATA_LOC]maplib\\roadnet.plt","[%GEOSPAN_LOC]maplib\\geospan.plt",FALSE,0,0,0,0,0,0);
	return TRUE;
}*/ 

/*BOOL ConvertSSTV1toV2 (LPSTR File)
{   
	char	OldName[128], IndexName[128];  
	short	pos=BT_FIRST, len;
	HANDLE	hDBold;
    LPGWDHEADER lpGWDHead, lpGWDHeadOld; 
    LPSEGDATAGM	pSegdata; 
    long	TLID, Offset;
    BOOL	rtn=FALSE;   
    LPSTR	lpDot, lpDotIndex;
	
	_fstrcpy (OldName,File);
	_fstrlwr (OldName);
	_fstrcpy (IndexName,OldName);
	lpDot = _fstrstr (OldName,".gmd");
	*lpDot = 0;
	_fstrcat (OldName,"1.in1");
	lpDotIndex = _fstrstr (IndexName,".gmd");
	*lpDotIndex = 0;
	_fstrcat (IndexName,".in1");
	if (GSSiRename (IndexName,OldName))
	{    
Mess1:  
		DoPaint = FALSE;
    	MessageBox (GetFocus(),"Cannot convert street seg file - file exists",NULL,MB_ICONEXCLAMATION);
        BlowOut();
	}
	*lpDot = 0;
	_fstrcat (OldName,"1.gmd");
	if (GSSiRename (File,OldName))
		goto Mess1;
	
    hDBold= OpenGWDatabase (OldName,BT_READ);
	lpGWDHeadOld = (LPGWDHEADER)GlobalLock (hDBold); 
	pSegdata = &lpGWDHeadOld->GWDData;
	OpenStreetSegmentTable (TRUE);
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetSegments); 
    while (!BT_FIND (lpGWDHeadOld->BTHandle[0],&TLID,pos,BT_ANY,(LPSTR)&Offset))
    {   
    	pos = BT_NEXT;
		FillGWDData (lpGWDHeadOld,Offset);
        Offset = _llseek (lpGWDHead->Fid,0,2);
        BT_PUT (lpGWDHead->BTHandle[0],(LPSTR)&TLID,(LPSTR)&Offset);  
	  	len = sizeof(SEGDATAGM);
	   	if (_lwrite (lpGWDHead->Fid,&len,2)!=2)
	   		goto ErrOut; 
	   	pSegdata->TrafVol=0; 
	  	pSegdata->FromTLID=0;
	  	pSegdata->Lanes=0;
	  	pSegdata->Filler[0]=0;
	  	pSegdata->Filler[1]=0;
	  	pSegdata->Filler[2]=0;
	   	if (_lwrite (lpGWDHead->Fid,pSegdata,len) != len)
	   		goto ErrOut; 
    }
    rtn=TRUE;  
ErrOut:
    CloseGWDatabase (hDBold);   
    if (!rtn)
    {
		DoPaint = FALSE;
    	MessageBox (GetFocus(),"Error converting street seg file - disk full",NULL,MB_ICONEXCLAMATION);
        BlowOut();
    }
	CloseStreetSegmentTable ();
    remove (OldName);
	*lpDot = 0;
	_fstrcat (OldName,"1.in1");
    remove (OldName);  
    return TRUE;
}*/
BOOL CheckBoardBMInRect (HDC hDC,LPSTR ImageFile1,LPSTR ImageFile2, RECT Rect, BOOL MaintainAspect)
#if ENABLETRACE
{GSSiEnterProg (395);
#endif
{  
    OFSTRUCT    fStruct;
    LPOFSTRUCT  pStruct = &fStruct;
    LPBITMAPINFOHEADER    pDibInfo;
    LPSTR pImage;
    short        i,  x, y, nrow, ncol, w=1, DeleteBM; 
    long    j;
    HANDLE  hDib;  
    HDC     hDCMem;
    HBITMAP hBMMem, hbmpOld;
    COLORREF    White, Color, Yellow=RGB(255,255,0);

    if (!ImageFile1[0])
{
#if ENABLETRACE
GSSiExitProg (395);
#endif
    	return FALSE; 
}
    if (ExistFile(ImageFile1))
    {
        j=0;
        LoadBitMap (ImageFile1, -1,&hDib, &DeleteBM,24,0,0); 
    }
    else
    {
        j=157001;
        LoadBitMap (ImageFile2,-1, &hDib, &DeleteBM,24,0,0); 
    }
    pDibInfo = (LPBITMAPINFOHEADER) GlobalLock (hDib);
    pImage = (LPSTR)pDibInfo + (pDibInfo->biSize+pDibInfo->biClrUsed*sizeof(COLORREF)); 

    hDCMem = CreateCompatibleDC (hDC);

    hBMMem = CreateCompatibleBitmap(hDC, Rect.right - Rect.left,Rect.bottom - Rect.top);

    hbmpOld = SelectObject(hDCMem, hBMMem);
    if (MaintainAspect)
        ComputeBMLoc (Rect, (LPBITMAPINFO)pDibInfo,MaintainAspect);
    else
    { 
        destX = (short)Rect.left;
        destY = (short)Rect.top;
        destW = (short)(Rect.right - Rect.left + 1);
        destH = (short)(Rect.bottom - Rect.top + 1);
    }
    SetStretchBltMode(hDCMem, StretchMode);
    i=StretchDIBits (hDCMem,0,0,
                       destW, destH,
                       0,0,
                       (short) pDibInfo->biWidth,
                       (short) pDibInfo->biHeight,
                       pImage,
                      (LPBITMAPINFO)pDibInfo,
                      (UINT)DIB_RGB_COLORS,
                      (DWORD) SRCCOPY);
    nrow = destH/w;
    ncol = destW/w;  
    White = GetPixel (hDCMem,1,1);
    for (;j<160000;j++)
    {   
        x = (short)(IDNINT(((double)rand()/RAND_MAX) * destW));
        y = (short)(IDNINT(((double)rand()/RAND_MAX) * destH)); 
        if (j<127000)
        {
            if ((Color=GetPixel (hDCMem,x,y)) != White)
                SetPixel (hDC,x,y,White);
        }
        else if (j<157000)
        {
            Color=GetPixel (hDCMem,x,y);
            SetPixel (hDC,x,y,Color);
        }
        else if (j==157000)
        {   
            
            GlobalUnlock (hDib);
            if (DeleteBM)
            	GlobalFree (hDib); 
            SelectObject(hDCMem, hbmpOld); 
            DeleteObject (hBMMem);
            LoadBitMap (ImageFile2, -1,&hDib, &DeleteBM,24,0,0);
		    pDibInfo = (LPBITMAPINFOHEADER) GlobalLock (hDib);
		    pImage = (LPSTR)pDibInfo + (pDibInfo->biSize+pDibInfo->biClrUsed*sizeof(COLORREF)); 
        
            hBMMem = CreateCompatibleBitmap(hDC, Rect.right - Rect.left,Rect.bottom - Rect.top);
        
            hbmpOld = SelectObject(hDCMem, hBMMem);
            if (MaintainAspect)
                ComputeBMLoc (Rect, (LPBITMAPINFO)pDibInfo,MaintainAspect);
            else
            { 
                destX = (short)Rect.left;
                destY = (short)Rect.top;
                destW = (short)(Rect.right - Rect.left + 1);
                destH = (short)(Rect.bottom - Rect.top + 1);
            }
            SetStretchBltMode(hDCMem, StretchMode);
            i=StretchDIBits (hDCMem,0,0,
                               destW, destH,
                               0,0,
                               (short) pDibInfo->biWidth,
                               (short) pDibInfo->biHeight,
                               pImage,
                              (LPBITMAPINFO)pDibInfo,
                              (UINT)DIB_RGB_COLORS,
                              (DWORD) SRCCOPY);
        }
        else
        {
            w=2;
            if (j>159000) w=4;
            x -= x%w;
            y -= y%w;
            BitBlt (hDC,x,y,w,w,
                    hDCMem,x,y,SRCCOPY); 
        }
    } 
    i=BitBlt (hDC,destX,destY,destW,destH,
            hDCMem,0,0,SRCCOPY);
/*  GlobalUnlock (hImage);
    GlobalFree (hImage);*/
    GlobalUnlock (hDib);
            if (DeleteBM)
    GlobalFree (hDib);
    SelectObject(hDCMem, hbmpOld); 
    DeleteObject (hBMMem);
    DeleteDC(hDCMem);

{
#if ENABLETRACE
GSSiExitProg (395);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}     

BOOL PrintMergex (HWND hWnd)
{	HDC hPr;
    RECT	Rect;
   /*******************************************************************
   *                                                                  *
   *                             PRINTDLG VARIABLES                   *
   *                                                                  *
   *******************************************************************/
   short xPage, yPage;
   int		SaveShadow = ShadowInc;
   WORD wSize;
   BOOL bError;
   FARPROC lpfnAbortProc, lpfnPrintDlgProc;
   HBRUSH	BkBrush;
   BOOL		rtn;
   LPVIEWPORT	lpSaveView, LastVP; 
   HANDLE	hSaveView[32], hDLT;
   BOOL		SaveHaveBounds[32];
   MNMXCORD	SaveWBounds[32];
   RECT		BandRect;
   double	SaveWidthFactor; 
   long		Page=0, TotPage, iseg;
   int	SaveShadowInc;
   int		iview, record, bRc, Band; 
   HFILE	FidPM; 
   BOOL		PrintPrompt=TRUE, DoPrint, First;
   short	ForceOrient=DMORIENT_PORTRAIT;  
   double	SaveSFLFF=SmallFontLargeFontFactor;
   LPDEVMODE pDevMode;  
   OFSTRUCT	OFStruct;
   char	str[1030], str2[32], ReportName[144], SavePrintName[34];
   long		Refno;
   char		Prefix[10], UDI[66];
   extern	HWND	ghWnd;  
   COLORREF	SaveColor = WindowColor;
   BOOL		UseBands = GetGlobalBVal2 ("[%USEBANDS]",TRUE);
    
   GetGlobalCVal ("[%PRINTNAME]",SavePrintName,NULL); 
   ghWnd = hWnd;
	if (!OpenConfig(NULL,NULL)) return(FALSE);
    SaveShadowInc = ShadowInc;   
 	SaveWidthFactor = WidthFactor;

    if (!ExistFile (PMMacroFile))
    {
		MessageBox( GetFocus(), PMMacroFile,"Unable to open macro file", MB_OK);
     	return (FALSE);
    } 
	FidPM = GSSiOpenFile (PMDataFile,&OFStruct,OF_READ);
    if (FidPM == HFILE_ERROR)
    {
		MessageBox( GetFocus(), PMDataFile,"Unable to open data file", MB_OK);
     	return (FALSE);
    } 
	fgetstring (str,1024,FidPM);
	    		  
	while (str[0]=='#' || *LastChr (str) == ';') 
	{   
		ExpandText (str);             
	  	if (!fgetstring (str,1024,FidPM))
	  	{
	  		GSSiClose (FidPM);
	  		return FALSE;
	  	}
    }

	TotPage = 0;
	GetPrintMergeRec (0,NULL);
	while (GetPrintMergeRec (FidPM,str))
		TotPage++;
	_llseek (FidPM,0,0);
    wSize = sizeof(PRINTDLG);
    if (!hPDChunk)
    {
    	if (!(lpPDChunk = (LPPRINTDLG)AllocAndLockMem(&hPDChunk, wSize)))
       	return(MemError());
		InitializeStruct(IDC_PRINTDLG, (LPSTR)lpPDChunk);
    }
    else
    	lpPDChunk = (LPPRINTDLG) GlobalLock (hPDChunk);
     	
    EnableWindow (hWndMain,FALSE);  
   	WindowColor = RGB(255,255,255);   
    DoPaint = FALSE;  
    HaltPaint = TRUE;
	lpPDChunk->hwndOwner = ghWnd;	
	if (GetGlobalLVal2("[%PRINTDIALOGOPT]",0))
		lpPDChunk->Flags = lpPDChunk->Flags|PD_PRINTSETUP|PD_RETURNDC;
	SetCurView (pViewportsD[0]); 
	if (CurView->WidthType == 2 || CurView->DesiredWidth > CurView->DesiredHeight)
		ForceOrient = DMORIENT_LANDSCAPE;
	if (ForceOrient)
	{ 
		DWORD	SaveFlags = lpPDChunk->Flags;
			    
		lpPDChunk->Flags = PD_RETURNDEFAULT;
		PrintDlg(lpPDChunk);
		lpPDChunk->Flags = SaveFlags; 
		if (lpPDChunk->hDevMode)
		{ 	
	    	IgnoreLock = TRUE;
			pDevMode = (LPDEVMODE)GlobalLock (lpPDChunk->hDevMode);
			pDevMode->dmOrientation = ForceOrient; 
			pDevMode->dmFields = pDevMode->dmFields | DM_ORIENTATION;
			GlobalUnlock (lpPDChunk->hDevMode); 
			IgnoreLock = FALSE;
		}
	}
	DoPrint = PrintDlg(lpPDChunk); 
	
    if (DoPrint)
    {	DOCINFO	DI;
     
    	hPr = lpPDChunk->hDC;
        gbUserAbort = FALSE;
        bError = FALSE;
        Printing = TRUE;
        PrintMerging = TRUE;
        lpfnPrintDlgProc = MakeProcInstance(PrintDlgProc, ghInst);
        ghPrintingDlg = CreateDialog(ghInst, "PRINTING", ghWnd,
                                         lpfnPrintDlgProc);
	    lpfnAbortProc = MakeProcInstance(AbortProc, ghInst);
        SetAbortProc(hPr,lpfnAbortProc);
	    
		for (iview=0;iview<*pNumViewports;iview++)
		{
			SetCurView (pViewports[iview]);
		    CloseTRANS2 (&CurView->hTranWinToBase);
		    CloseTRANS2 (&CurView->hTranBaseToWin);
			if (CurView->Type == 7)
		        DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
			if (!CurView->WindowIsZoomed)
				CurView->HaveBounds = FALSE;
			SaveHaveBounds[iview]=CurView->HaveBounds;
			SaveWBounds[iview]=CurView->WBounds;
		    hSaveView[iview] = GSSiGlobAlloc (GHND,sizeof(VIEWPORT));
		    lpSaveView = (LPVIEWPORT)GlobalLock (hSaveView[iview]);
			*lpSaveView = *CurView;  
			GlobalUnlock (hSaveView[iview]);
		}  
		  
		fgetstring (str,1024,FidPM);
	    		  
		while (str[0]=='#' || *LastChr (str) == ';') 
		{                
		  	ExpandText (str);
		  	if (!fgetstring (str,1024,FidPM))
		  	{
		  		GSSiClose (FidPM);
		  		return FALSE;
		  	}
        }

		_splitpath (CfgName,NULL,NULL,LeafName,NULL);
	    DI.cbSize = sizeof(DOCINFO);
	    DI.lpszDocName = LeafName;
	    DI.lpszOutput = NULL;  
	    
		ProcessDelimTextHeader(str,&hDLT); 
		GetPrintMergeRec (0,NULL);
		record = 0;
		while (GetPrintMergeRec (FidPM,str))
  	    {  
  	       
           GetDelimTextData(str,hDLT);
           record++; 
		   AddReportToPrintList (NULL,NULL,NULL,NULL);
  	       GetGlobalCVal ("[%PRINTNAME]",DocName,NULL); 
  	       if (!*DocName)
		   {
		   		_splitpath (CfgName,NULL,NULL,LeafName,NULL);
           		sprintf (DocName,"%s.%5.5i",LeafName,record); 
           }
           Page++;
	       DI.lpszDocName = DocName;
	
		    if (StartDoc(hPr,&DI) > 0)
		    {   
		       int dpi = GetDeviceCaps(hPr, LOGPIXELSX);
		       
		       Escape(hPr, SETCOPYCOUNT, sizeof(int),(LPCSTR) &lpPDChunk->nCopies, &lpPDChunk->nCopies);
		       xPage = GetDeviceCaps(hPr, HORZRES);
		       yPage = GetDeviceCaps(hPr, VERTRES);
		       Rect.left = 0;
		       Rect.top = 0;
		       Rect.bottom = yPage-1;
		       Rect.right = xPage-1;
		       IgnoreLock = TRUE;
		       SetMainRect (0,hPr,&Rect);
		       {
		       		double fwidth=(double)xPage/(double)dpi, fheight=(double)yPage/(double)dpi;
		       		LPDEVNAMES pdn=(LPDEVNAMES)GlobalLock (lpPDChunk->hDevNames);
		       		LPSTR	PrinterName=(LPSTR)pdn+pdn->wDeviceOffset; 
		       		
					DoShrinkOrtho = GetGlobalBVal2 ("[%SHRINKORTHOS]",FALSE);
					if (!_fstricmp (PrinterName,"Acrobat PDFWriter"))
						DoShrinkOrtho = FALSE;
			        sprintf (str,"Printer:%s\r\ndpi:%i  width:%.2f  height:%.2f",PrinterName,dpi,fwidth,fheight);
			        GlobalUnlock (lpPDChunk->hDevNames); 
			        SetDlgItemText (ghPrintingDlg,IDC_PRINTERINFO,str);
		       }
	   	       IgnoreLock = FALSE;
		   	   StartPage (hPr); 
		   	   DisplayCycle++;
			   if (UseBands)
			   		bRc = Escape(hPr, NEXTBAND, 0, (LPSTR)NULL, &BandRect);
			   else
	           		bRc = 1;
	           Band=0;
	           while (bRc > 0 && (!UseBands || !IsRectEmpty(&BandRect)))
	           {
		
				   if (!SetupViewports (NULL,hPr,0,Rect,Band+record-1))
				   {
				   		gbUserAbort=TRUE;
						DestroyWindow(ghPrintingDlg);
						ghPrintingDlg = NULL; 
						goto Exit;
				   }
	               Band++;
		   	       SetDisplayMode (CurView->hDC, GF_TEXTMODE);
				   FillRectPoly (hPr,&Rect,WindowColor);
				   NumViewportsToDisplay = *pNumViewports;
				   
				   for (DisplayViewID=0;DisplayViewID<NumViewportsToDisplay;DisplayViewID++)
				   {
						SetCurView (pViewports[DisplayViewID]);
						if (CurViewActive ())
							CurView->Display = TRUE;
						ResetViewport (TRUE,TRUE);
						if (SaveHaveBounds[DisplayViewID])
						{
							CurView->HaveBounds = TRUE;
							CurView->WBounds = SaveWBounds[DisplayViewID];
							CurView->NewBounds = CurView->WBounds;
						}
        				SetBoundsRect2 (CurView->DrawRect,CurView->hDC);
				   }
				   
				   ProcessMacroFile (PMMacroFile,str2,0);

				   for (DisplayViewID=0;DisplayViewID<NumViewportsToDisplay;DisplayViewID++)
				   {
						SetCurView (pViewports[DisplayViewID]);
						if (CurViewActive ())
							CurView->Display = TRUE;
						ResetViewport (TRUE,TRUE); 
				   }

				   LastVP = 0;
				   DisplayViewID = 0;
			       while (DisplayViewID<NumViewportsToDisplay)
				   {
				   		SetCurView (pViewportsD[DisplayViewID]);
				   		if (CurView != LastVP && CurView->NumFiles) 
				   		{
				   			if (CurView->ID == *pCommandViewport &&
						     	CurView->OrthoRes && CurView->WindowZoomedToOrtho)
						    {
						        CurView->WBounds = CurView->NewBounds;
						        SetNewBoundsToOrtho();
						    }
					   			
				   			SetBoundsRect2 (CurView->DrawRect,CurView->hDC);
				   		}  
				   		LastVP = CurView;
						if (DisplayViewport (hWnd,hPr,TRUE)) 
						{  
						   char	Line1[128], Line3[128], str[128]; 
							   
						   sprintf (Line1,"Page %ld of %ld", Page,TotPage);
						   sprintf (Line3,"Band %i - Viewport %i",Band,CurView->ID); 
						   _fstrcpy (str,PltName);
						   ExpandText (str);      
						   iseg = 0;
						   PrintMessage2 (Line1,str,Line3);
					       if (OpenMap (hWnd, hPr))
					       {   
		   	       	   	       SetDisplayMode (CurView->hDC, GF_MAPMODE);
				
						       do
						       {
						   			sprintf (Line3,"Band %i - Viewport %i - Segment %ld",Band,CurView->ID,iseg++);
									PrintMessage2 (NULL,NULL,Line3);
						       }
						       while (DisplaySeg (hPr,FALSE) && CheckPrintAbort (hPr) && CurView); 
						       if (gbUserAbort)
						       		HaltMapDisplay (FALSE);
						   }
					     }
					     
				   }
				   if (!gbUserAbort)
				   {
						//ApplyVPBounds ();
		       			EndDisplayProcessing (TRUE);
				   		ApplyVPShadows ();	
				   }
				   if (GetGlobalCVal ("[%PRINTPROMPT]",str,NULL))
					   DisplayPromptText (hPr,str);
		
		 NextBand: 
		 		   if (UseBands)  
		 		   		bRc = Escape(hPr, NEXTBAND, 0, (LPSTR)NULL, &BandRect); 
		 		   else
				   		bRc = 0;
			   }
	   		   EndPage (hPr);
			   if (GetGlobalLVal2 ("[%PRINTWPOPT]",FALSE) == 2)
			   {    
			   		if (ExistFile ("[%DL]macros\\summary.txt"))
			   		{
				   		_fstrcpy (ReportName,"[%DL]macros\\summary.txt");  
				   		PrintReport2 (hPr,ReportName,NULL,NULL,0); 
				   	}
	           }
	           First = TRUE;
			   while (GetNextPrintReport (First,ReportName,&Refno,Prefix,UDI))
			   {    
			   		LPINT	ID;
			   		int		i, NumIDs;
			   		LPLONG	CNum; 
			   		char	txt[64];
					LPVIEWPORT	SaveView;        
					HANDLE	hView;  
			   		
			   		First = FALSE;
				    if (GetGlobalLVal2 ("[%PRINTWPOPT]",FALSE) == 1)
				    {    
				   		EndPage (hPr);
				   		PrintReport2 (hPr,ReportName,Refno,Prefix,UDI); 
				   	}
			   }   

           	   EndDoc (hPr);
		    }
		    else 
		    {
		       bError = TRUE;		   
		       goto Exit;
		    }
	      NextRec:;
		}
		if (GetGlobalLVal2 ("[%PRINTWPOPT]",FALSE) == 2)
		{    
			if (ExistFile ("[%DL]macros\\jobsum.txt"))
			{
		        _fstrcpy (DocName,"Summary");
			    if (StartDoc(hPr,&DI) > 0) 
			    {
					_fstrcpy (ReportName,"[%DL]macros\\jobsum.txt");  
			   		PrintReport2 (hPr,ReportName,NULL,NULL,0); 
	           	    EndDoc (hPr);
	           	}
		   	}
		}
Exit:
	   GSSiClose (FidPM);
	   GSSiGlobFree (&hDLT);
	   DeleteDC(lpPDChunk->hDC);
	   for (DisplayViewID=0;DisplayViewID<NumViewportsToDisplay;DisplayViewID++)
	   {
			SetCurView (pViewports[DisplayViewID]);
		    CloseTRANS2 (&CurView->hTranWinToBase);
		    CloseTRANS2 (&CurView->hTranBaseToWin);
     		if (CurView->Type == 7)
 		        DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
		    lpSaveView = (LPVIEWPORT)GlobalLock (hSaveView[DisplayViewID]);
		    if (lpSaveView)
		    {  
			    if (CurView->hMaskArea != lpSaveView->hMaskArea) 
			    {
	   			    ClearMaskArea ();  
	   			    lpSaveView->hMaskArea = 0;
				}	   			    
	   			lpSaveView->pVisList1 = CurView->pVisList1;
	   			lpSaveView->pVisListManual = CurView->pVisListManual;
		   		*CurView=*lpSaveView;
				GSSiGlobUlFree (&hSaveView[DisplayViewID]);  
			}
				
    		SetBounds(CurView->hWnd,NULL); /*Null prevents redisplay of view bounds*/

	   }
//	   EnableWindow (hWndMain,TRUE);
	   if (!gbUserAbort)
	   {
	      DestroyWindow(ghPrintingDlg);
	      ghPrintingDlg = NULL;
	   }
	   if (bError)
	      MessageBox(ghWnd, "Error while printing", gszAppName, MB_OK);
	   else
	   {
	      if (gbUserAbort)
	      {
	         	MessageBox(ghWnd, "Printing Aborted", "", MB_OK);
		        if (PrintMsgWnd)
		        {
			         DestroyWindow(PrintMsgWnd); 
			         ghPrintingDlg = NULL; 
			    }
			}
	   }
	   FreeProcInstance(lpfnAbortProc);
	   FreeProcInstance(lpfnPrintDlgProc);
	}
	else
	 {
	 //Process error.  Be sure to free any memory that may be associated with
	 //hDevMode or hDevNames.
	   /* if (lpPDChunk->hDevMode)
	       GlobalFree(lpPDChunk->hDevMode);
	    if (lpPDChunk->hDevNames)
	       GlobalFree(lpPDChunk->hDevNames);*/
	    ProcessCDError(CommDlgExtendedError());
	 }

    GlobalUnlock(hPDChunk);
    Printing = FALSE;  
    PrintMerging = FALSE;
	HaltPaint = FALSE;
   	DoPaint = TRUE;   
    ShadowInc = SaveShadow;
 	WidthFactor = SaveWidthFactor;
	EnableWindow (hWndMain,TRUE);
	SetFocus (hWndMain);
	GSSiTrace ("End PrintMerge");
	{
		HDC	hDC = GetDC (hWndMain);
	    SetMainRect (hWndMain,hDC,NULL);
	    ReleaseDC (hWndMain,hDC); 
	} 
    SmallFontLargeFontFactor = SaveSFLFF;
    WindowColor = SaveColor;
    GetGlobalCVal ("[%PRINTNAME]",SavePrintName,NULL); 
    return (rtn);

}

BOOL FAR PASCAL LOADGENMsgProc(HWND hWndDlg, WORD Message, WPARAM wParam, LPARAM lParam)
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
 hMem = GSSiGlobAlloc (GHND,2048+256);
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
                    		hAttFile = GSSiGlobAlloc (GMEM_MOVEABLE,256);
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
                 _lwrite (FidSave,Ext,6);
                 _lwrite (FidSave,&Version,2);   
                 GetDlgItemText (hWndDlg,IDC_FILE,LoadName,128);
                 _lwrite (FidSave,LoadName,128);
                 GetDlgItemText (hWndDlg,IDC_DESTMAP,PltName,128);
                 _lwrite (FidSave,PltName,128);
                 NewOpt = SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0);
                 _lwrite (FidSave,&NewOpt,2);
                 GetDlgItemText(hWndDlg,IDC_TAPREFIX,Prefix,sizeof(Prefix));
                 _lwrite (FidSave,Prefix,sizeof(Prefix));
                 GetDlgItemText(hWndDlg,IDC_UDI,UDI,sizeof(UDI));               
                 _lwrite (FidSave,UDI,sizeof(UDI));
                 GetDlgItemText(hWndDlg,IDC_SYMNAME,SymStuff,lnSymStuff);               
                 _lwrite (FidSave,SymStuff,lnSymStuff);
                 GetDlgItemText (hWndDlg,IDC_PROJECTION,curproject,lncurproject);
			     ii = _llseek (FidSave,0,1);
                 ii=_lwrite (FidSave,curproject,34);
			     UnitsOpt=SendDlgItemMessage(hWndDlg,IDC_UNITS,CB_GETCURSEL,NULL,NULL); 
			     ii = _llseek (FidSave,0,1);
                 _lwrite (FidSave,&UnitsOpt,2);
                 NewOpt  = SendDlgItemMessage (hWndDlg,IDC_CREATE_SYMS,BM_GETCHECK,0,0);
			     ii = _llseek (FidSave,0,1);
                 ii=_lwrite (FidSave,&NewOpt ,2);
                 _fmemset (str,0,sizeof(str));
                 ii=_lwrite (FidSave,str,sizeof(str)); //spacer for future options
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
                 _lread (FidSave,Ext,6);
                 _lread (FidSave,&Version,2);   
                 _lread (FidSave,LoadName,128);
                 SetDlgItemText (hWndDlg,IDC_FILE,LoadName);
                 _lread (FidSave,PltName,128);
                 SetDlgItemText (hWndDlg,IDC_DESTMAP,PltName);
                 _lread (FidSave,&NewOpt,2);
                 SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_SETCHECK,NewOpt,0);
                 _lread (FidSave,Prefix,sizeof(Prefix));
                 SetDlgItemText(hWndDlg,IDC_TAPREFIX,Prefix);
                 _lread (FidSave,UDI,sizeof(UDI));
                 SetDlgItemText(hWndDlg,IDC_UDI,UDI);               
                 _lread (FidSave,SymStuff,lnSymStuff);
                 SetDlgItemText(hWndDlg,IDC_SYMNAME,SymStuff);               
			     ii = _llseek (FidSave,0,1);
                 ii=_lread (FidSave,curproject,34);
		         SendDlgItemMessage (hWndDlg,IDC_PROJECTION,CB_SELECTSTRING,-1,(LPARAM)curproject);
		     	 ii = _llseek (FidSave,0,1);
                 _lread (FidSave,&UnitsOpt,2);
			     SendDlgItemMessage(hWndDlg,IDC_UNITS,CB_SETCURSEL,UnitsOpt,NULL); 
			     if (Version > 1)
			     {   
			     	 ii = _llseek (FidSave,0,1);
	                 ii=_lread (FidSave,&NewOpt,2);
	                 SendDlgItemMessage (hWndDlg,IDC_CREATE_SYMS,BM_SETCHECK,NewOpt,0);
			     	 ii = _llseek (FidSave,0,1);
	                 ii=_lread (FidSave,str,sizeof(str));
			     }
			     while (ReadObject (FidSave, FALSE,NULL,NULL));
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
                 TotLen = _llseek (FidSHP,0,2);  
                 _llseek (FidSHP,0,0);
                 ExpandText (PltName);
                 PltType = 2;
                 Done = FALSE;
                 MidLine=0; 
                 str[0]=0;  
                 
/*                 if (SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0)) 
                 {
                     short NumPens=10;
                     PENDESC PenDesc[10];
                     LPSYMDESC  pSymDesc;  
                     LPSYMBOL   pSym;
                     MNMXCORD MinMaxCoord;
                     DPOINT Points[4];
                     short    i;  
                    
                     if (hImportLimits) 
                     {   
                    	LPSHORT	pint;
                    	LPMNMXCORD	pMinMax;
                    	
                    	pint = GlobalLock (hImportLimits);
                    	pint+=2;
                    	pMinMax = pint;
                    	MinMaxCoord = *pMinMax;
                    	GlobalUnlock (hImportLimits);  
                     }
                     else
                     {
	                     MinMaxCoord.xmn = DBL_MAX;
	                     MinMaxCoord.xmx = -DBL_MAX; 
	                     MinMaxCoord.ymn = DBL_MAX;
	                     MinMaxCoord.ymx = -DBL_MAX; 
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
	                            MessageBox(GetFocus(),"Unable to convert coordinates as specified", 0,MB_ICONQUESTION|MB_OK);
	                            goto ErrorEnd;
	                        } 
	                        MinMaxCoord.xmn = min(MinMaxCoord.xmn,Points[i].x);
	                        MinMaxCoord.xmx = max(MinMaxCoord.xmx,Points[i].x);
	                        MinMaxCoord.ymn = min(MinMaxCoord.ymn,Points[i].y);
	                        MinMaxCoord.ymx = max(MinMaxCoord.ymx,Points[i].y);
	                     }
	                 }   
                     for (i=0;i<NumPens;i++)
                     {
                        PenDesc[i].PenNum = i+1;
                        PenDesc[i].Width = (float)1; 
                        PenDesc[i].Style = 1;
                        PenDesc[i].Color = RGB(0,0,0);
                     }
                     if (!CreateNewMap (PltName,&MinMaxCoord,NumSyms,hSymDesc,
                                                        NumPens,(LPPENDESC)&PenDesc)) goto ErrorEnd;
                 } */  
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
                    UINT       	*pNumPoints;
                    long		LastIndex; 
                    short		NumPoints, SymType; 
                    MNMXCORD	Bounds;

                    nPoly = 1;  
                    NumPoints = 0;
					hhPoly = GSSiGlobAlloc (GMEM_MOVEABLE,sizeof(HANDLE)*256); 
					phPoly = (LPHANDLE)GlobalLock (hhPoly);
					hNumPoints = GSSiGlobAlloc (GHND,sizeof(short)*256); 
					pNumPoints = (UINT*)GlobalLock (hNumPoints);  
					DBoundsInit (&Bounds);
		NextGENLink:                    
                 	if (!fgetstring (str,128,FidSHP))
                		goto ErrorEnd;
	                 lineno++;
	                *phPoly = GSSiGlobAlloc (GMEM_MOVEABLE,UINT_MAX); 
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
		            	Loc = _llseek (FidSHP,0,1);
	                 	if (!fgetstring (str,128,FidSHP))
	                		goto ErrorEnd;
	                	if (!_fstricmp (str,"END"))
	                	{
   		                	_llseek (FidSHP,Loc,0); 
		                 	goto EndGENItem; 
		                }
                 		Code = atol (str);
               			Exclusion = FALSE;
                 		if (Code == -99999)
                 			Exclusion = TRUE;
                 		else if (Code != 99999) 
                 		{
		                	_llseek (FidSHP,Loc,0); 
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
						pNumPoints = (UINT*)GlobalLock (hNumPoints);
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
						pNumPoints = (UINT*)GlobalLock (hNumPoints);
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
                 
                 CurLoc = _llseek (FidSHP,0,1);       
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
                 CloseRefIndex();           
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

BOOL ClipMapx (HWND hWnd)
{
	 HANDLE hMFP;
	 HDC	hMF;
	 HANDLE	hMFHead;
	 WMFHEADER	*pMFHead;
	 METAFILEPICT	MPF;
	 LPMETAFILEPICT lpMFP;
	   LPVIEWPORT	lpSaveView, LastView; 
	   HANDLE	hSaveView[32];
	   BOOL		SaveHaveBounds[32];
	   MNMXCORD	SaveWBounds[32];
	   int		iview; 
	 HBRUSH BkBrush;   
	 RECT	Rect;

/*     FileMode = TRUE;*/ 
	 Printing = TRUE;  
	 FileMode = TRUE;
     hCursor = LoadCursor (NULL,IDC_WAIT);
     OldCursor = GSSiSetCursor (hCursor); 
     KeepMemLength = TRUE;
     hMFP = GSSiGlobAlloc (GMEM_MOVEABLE,sizeof(METAFILEPICT));   
     KeepMemLength = FALSE;
     lpMFP = (LPMETAFILEPICT)GlobalLock (hMFP);
     hMF = CreateMetaFile (NULL);

	 lpMFP->mm = MM_ISOTROPIC;
	 lpMFP->xExt = GetDeviceCaps(CurView->hDC, HORZSIZE);
	 lpMFP->yExt = GetDeviceCaps(CurView->hDC, VERTSIZE);
	 GetClientRect (CurView->hWnd,&Rect);
	 Rect.right--;
	 Rect.bottom--;
	 Rect.right *= 10;
	 Rect.bottom *=10;  
	 lpMFP->xExt = -Rect.right;
	 lpMFP->yExt = -Rect.bottom;
	 SetWindowOrg  ( hMF, 0,0);
	 SetWindowExt  ( hMF, Rect.right, Rect.bottom );
	 SetMainRect (0,hMF,&Rect);
	 FillRectPoly (hMF,&MainRect,WindowColor);
	 for (iview=0;iview<*pNumViewports;iview++)
	   {
	   		SetCurView (pViewports[iview]); 
	 		if (CurView->Type == 7)
		        DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
	   		SaveHaveBounds[iview]=CurView->HaveBounds;
	   		SaveWBounds[iview]=CurView->WBounds;
		    hSaveView[iview] = GSSiGlobAlloc (GHND,sizeof(VIEWPORT));
		    lpSaveView = (LPVIEWPORT)GlobalLock (hSaveView[iview]);
			*lpSaveView = *CurView;  
			GlobalUnlock (hSaveView[iview]);
	   } 
		   SetupViewports (NULL,hMF,0,MainRect,0);  
		   NumViewportsToDisplay = *pNumViewports;
		   for (DisplayViewID=0;DisplayViewID<NumViewportsToDisplay;DisplayViewID++)
		   {
				SetCurView (pViewports[DisplayViewID]);
				ResetViewport (TRUE,TRUE);
				if (SaveHaveBounds[DisplayViewID])
				{
					CurView->HaveBounds = TRUE;
					CurView->WBounds = SaveWBounds[DisplayViewID];
					CurView->NewBounds = CurView->WBounds;
				}
		   }
		   DisplayViewID = 0;   
		   LastView = NULL;
		   while (DisplayViewID<NumViewportsToDisplay)
		   {    SetCurView (pViewports[DisplayViewID]); 
		   	    if (CurView != LastView)
		   	    { 
					ResetViewport (TRUE,TRUE);
					if (SaveHaveBounds[DisplayViewID])
					{
						CurView->HaveBounds = TRUE;
						CurView->WBounds = SaveWBounds[DisplayViewID];
						CurView->NewBounds = CurView->WBounds;
					} 
					LastView = CurView;
		        }
		   		if (CurView->NumFiles) SetBoundsRect2 (CurView->DrawRect,CurView->hDC);
				if (DisplayViewport (hWnd,hMF,FALSE)) 
				{  

			       if (OpenMap (hWnd, hMF))
			       {   
   	       	   	       SetDisplayMode (CurView->hDC, GF_MAPMODE);
	
				       while (DisplaySeg (hMF,FALSE)); 
				   }
			     }
		   }
		   for (DisplayViewID=0;DisplayViewID<NumViewportsToDisplay;DisplayViewID++)
		   {
			   lpSaveView = (LPVIEWPORT)GlobalLock (hSaveView[DisplayViewID]);
		   		*pViewports[DisplayViewID]=*lpSaveView;
				GlobalUnlock (hSaveView[DisplayViewID]);
				GlobalFree (hSaveView[DisplayViewID]); 
				SetCurView (pViewports[DisplayViewID]);
		 		if (CurView->Type == 7)
			        DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
				SetBounds(CurView->hWnd,NULL); /*Null prevents redisplay of view bounds*/

		   }

	 Printing = FALSE;
	 FileMode = FALSE;



/*	 DisplayPlotInit(hWnd);
	 if (!OpenMap (hWnd, hMF)) return(FALSE);
	 if (WindowZoomed()) SetNewBoundsToBounds();
	 SetBounds (hWnd,hMF);
	 lpMFP->mm = MM_ISOTROPIC;
	 lpMFP->xExt = (CurView->Bounds.xmx-CurView->Bounds.xmn);
	 lpMFP->yExt = (CurView->Bounds.ymx-CurView->Bounds.ymn);
	 hRgn = CreateRectRgn (0,0,lpMFP->xExt,lpMFP->yExt);

	 while (DisplaySeg (hMF));
*/
     hMF = CloseMetaFile (hMF);
     lpMFP->hMF = hMF;

     if (!OpenClipboard (hWnd))
     {  
     	GlobalUnlock (hMFP);
     	GlobalFree (hMFP);
     	DeleteMetaFile (hMF); 
     	MessageBeep (MB_ICONEXCLAMATION);
		MessageBox( GetFocus(), "ERROR: Cannot access the clipboard",NULL, MB_OK|MB_ICONEXCLAMATION);
     }                   
     else
     {
	     EmptyClipboard();
		 #if CHECKMEM
			GSSiRemoveMem (hMFP);
		 #endif
		 SetClipboardData(CF_METAFILEPICT, hMFP);
		 CloseClipboard();  
	 }
/*     DeleteMetaFile (hMF);*/
     hCursor = NULL; 
     GSSiSetCursor (OldCursor);

	 FileMode=FALSE;
	 return (TRUE);
}

BOOL ClipMapy (HWND hWnd)
{	HDC hPr;
    RECT	Rect;
   /*******************************************************************
   *                                                                  *
   *                             PRINTDLG VARIABLES                   *
   *                                                                  *
   *******************************************************************/
   HDC	hDC;
   long xPage, yPage;
   WORD wSize;
   BOOL bError;
   HBRUSH	BkBrush;
   BOOL		rtn=TRUE;
   LPVIEWPORT	lpSaveView, LastVP; 
   HANDLE	hSaveView[MAX_VIEWPORTS],SaveMaskArea[MAX_VIEWPORTS];
   BOOL		SaveHaveBounds[MAX_VIEWPORTS];
   MNMXCORD	SaveWBounds[MAX_VIEWPORTS]; 
   UINT		SaveMaskNumPoints[MAX_VIEWPORTS];
   int		iview;  
   MSG		msg; 
   HWND		DTW;
   int		SaveShadow = ShadowInc;
   RECT		BandRect,LastBandRect, SaveRect; 
   extern	HWND	ghWnd;   
   int		Band, bRc; 
   char		str[256], ReportName[144];
   LPSTR	lpStr=str;
   BOOL		PrintPrompt=TRUE, DoPrint, LastPage=FALSE, First;
   short	ForceOrient=DMORIENT_PORTRAIT;  
   long		iseg=0;
   LPDEVMODE pDevMode;  
   double	SaveSFLFF=SmallFontLargeFontFactor,SaveWidthFactor;
   HANDLE	hSaveBM; 
   long		Refno;
   char		Prefix[10], UDI[66];   
   COLORREF	SaveColor=WindowColor;  
   BOOL		UseBands = GetGlobalBVal2 ("[%USEBANDS]",TRUE);
	 HANDLE hMFP; 
	 HDC	hMF;
	 HANDLE	hMFHead;
	 WMFHEADER	*pMFHead;
	 METAFILEPICT	MPF;
	 LPMETAFILEPICT lpMFP;

/*     FileMode = TRUE;*/ 
	 Printing = TRUE;  
	 FileMode = TRUE;
     hCursor = LoadCursor (NULL,IDC_WAIT);
     OldCursor = GSSiSetCursor (hCursor); 
     KeepMemLength = TRUE;
     hMFP = GSSiGlobAlloc (GMEM_MOVEABLE,sizeof(METAFILEPICT));   
     KeepMemLength = FALSE;
     lpMFP = (LPMETAFILEPICT)GlobalLock (hMFP);
     hMF = CreateMetaFile (NULL);

	 lpMFP->mm = MM_ISOTROPIC;
	 lpMFP->xExt = GetDeviceCaps(CurView->hDC, HORZSIZE);
	 lpMFP->yExt = GetDeviceCaps(CurView->hDC, VERTSIZE);  
	 SetConfig (1);
	 SetCurView (pViewportsD[0]); 
	 if (!CurView->Type)
	 	Rect = CurView->Rect;
	 else
	 	GetClientRect (CurView->hWnd,&Rect);    
	 Rect.right = Rect.right - Rect.left;
	 Rect.bottom = Rect.bottom - Rect.top;
	 Rect.left = 0;
	 Rect.top = 0;
//	 Rect.right *= 10;
//	 Rect.bottom *=10;  
	 lpMFP->xExt = -Rect.right;
	 lpMFP->yExt = -Rect.bottom;
	 SetWindowOrg  ( hMF, 0,0);
	 SetWindowExt  ( hMF, Rect.right, Rect.bottom );

 	SaveWidthFactor = WidthFactor;
	if (!OpenConfig(NULL,NULL)) return(FALSE);

    DoPaint = FALSE;   
    HaltPaint = TRUE;
    hSaveBM = EnterBlockingWindow (hWnd);
	EnableWindow (hWndMain,FALSE);
	SetCurView (pViewportsD[0]); 
	SetMainRect (0,hMF,&Rect);
	for (iview=0;iview<*pNumViewports;iview++)
	{
		SetCurView (pViewports[iview]);
	    CloseTRANS2 (&CurView->hTranWinToBase);
	    CloseTRANS2 (&CurView->hTranBaseToWin); 
		GSSiGlobFree (&CurView->ToolbarHandle);
	    GSSiGlobFree (&CurView->hTAGList);
		if (CurView->Type == 7)
	        DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
		if (!CurView->WindowIsZoomed)
			CurView->HaveBounds = FALSE;
		SaveHaveBounds[iview]=CurView->HaveBounds;
		SaveWBounds[iview]=CurView->WBounds;
		SaveMaskNumPoints[iview] = CurView->NumMaskPoints; 
		if (CurView->hMaskArea)
		{
			HPSTR	pMask=GlobalLock (CurView->hMaskArea), pNewMask;
			long	size = sizeof(MNMXCORD) + (long)CurView->NumMaskPoints * (long)sizeof(DPOINT);
			   			
			SaveMaskArea[iview] = GSSiGlobAlloc (GMEM_MOVEABLE,size);
			pNewMask = GlobalLock (SaveMaskArea[iview]);
			hmemmove (pNewMask,pMask,size);
			GlobalUnlock (SaveMaskArea[iview]);
			GlobalUnlock (CurView->hMaskArea);
		}else
			SaveMaskArea[iview]=0;
	    hSaveView[iview] = GSSiGlobAlloc (GHND,sizeof(VIEWPORT));
	    lpSaveView = (LPVIEWPORT)GlobalLock (hSaveView[iview]);
		*lpSaveView = *CurView;
		GlobalUnlock (hSaveView[iview]);
	}
	DisplayCycle++; 
	SetupViewports (NULL,hMF,0,Rect,Band);
    SetDisplayMode (CurView->hDC, GF_TEXTMODE);
   FillRectPoly (hMF,&Rect,WindowColor);
   NumViewportsToDisplay = *pNumViewports;
   for (DisplayViewID=0;DisplayViewID<NumViewportsToDisplay;DisplayViewID++)
   {
		SetCurView (pViewports[DisplayViewID]);
		if (CurViewActive ())
			CurView->Display = TRUE;
		ResetViewport (TRUE,TRUE);
		if (SaveHaveBounds[DisplayViewID])
		{
			CurView->HaveBounds = TRUE;
			CurView->WBounds = SaveWBounds[DisplayViewID];
			CurView->NewBounds = CurView->WBounds;
		}
   }
   DisplayViewID = 0; 
   LastVP = 0;
   while (DisplayViewID<NumViewportsToDisplay)
   {
   		SetCurView (pViewportsD[DisplayViewID]);
   		if (CurView != LastVP && CurView->NumFiles) 
   		{
   			if (CurView->ID == *pCommandViewport &&
		     	CurView->OrthoRes && CurView->WindowZoomedToOrtho)
		    {
		        CurView->WBounds = CurView->NewBounds;
		        SetNewBoundsToOrtho();
		    }
				   			
   			SetBoundsRect2 (CurView->DrawRect,CurView->hDC);
   		}  
   		LastVP = CurView;
		if (DisplayViewport (hWnd,hMF,TRUE)) 
		{  
	       if (OpenMap (hWnd, hMF))
	       {   
   	   	       SetDisplayMode (CurView->hDC, GF_MAPMODE);
		       while (DisplaySeg (hMF,FALSE)&& CurView); 
		   }
	     }
				     
    }
	ApplyVPBounds ();
   	EndDisplayProcessing (TRUE);
		   
	for (DisplayViewID=0;DisplayViewID<NumViewportsToDisplay;DisplayViewID++)
	{
		SetCurView (pViewports[DisplayViewID]);
	    CloseTRANS2 (&CurView->hTranWinToBase);
	    CloseTRANS2 (&CurView->hTranBaseToWin);
	    GSSiGlobFree (&CurView->hTAGList);
 		if (CurView->Type == 7)
	        DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
	    lpSaveView = (LPVIEWPORT)GlobalLock (hSaveView[DisplayViewID]);
	    if (lpSaveView)
	    {  
   			ClearMaskArea ();
   			lpSaveView->pVisList1 = CurView->pVisList1;
   			lpSaveView->pVisListManual = CurView->pVisListManual;
	   		*CurView = *lpSaveView;
			CurView->NumMaskPoints = SaveMaskNumPoints[DisplayViewID];
			CurView->hMaskArea = SaveMaskArea[DisplayViewID];
			GSSiGlobUlFree (&hSaveView[DisplayViewID]);  
		}
				
		SetBounds(CurView->hWnd,NULL); /*Null prevents redisplay of view bounds*/

	}

	hMF = CloseMetaFile (hMF);
	lpMFP->hMF = hMF;
		
	if (!OpenClipboard (hWnd))
	{  
		GlobalUnlock (hMFP);
		GlobalFree (hMFP);
		DeleteMetaFile (hMF); 
		MessageBeep (MB_ICONEXCLAMATION);
		MessageBox( GetFocus(), "ERROR: Cannot access the clipboard",NULL, MB_OK|MB_ICONEXCLAMATION);
	}                   
	else
	{
		EmptyClipboard();
		#if CHECKMEM
		GSSiRemoveMem (hMFP);
		#endif
		SetClipboardData(CF_METAFILEPICT, hMFP);
		CloseClipboard();  
	}
	Printing = FALSE; 
	FileMode = FALSE; 
	HaltPaint = FALSE;
   	DoPaint = TRUE;   
    ShadowInc = SaveShadow;
 	WidthFactor = SaveWidthFactor;
	EnableWindow (hWndMain,TRUE);
	SetFocus (hWndMain);
	GSSiTrace ("End Clipmap");    
	hDC = GetDC (hWndMain);
    SetMainRect (hWndMain,hDC,NULL);
	LeaveBlockingWindow(hSaveBM);
    ReleaseDC (hWndMain,hDC);  
    SmallFontLargeFontFactor = SaveSFLFF;    
    hCursor = NULL; 
    GSSiSetCursor (OldCursor);

    return (rtn);

}   

BOOL LoadSYMTable (void)
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
    short         rc,TemplateSymbol;   
    LPSYMBOL    CurSymbol; 
	char		NewPointTemplate[64]="[%NEW_POINT_SYMBOL_TEMPLATE]"; 
    char		Parent[40];
    
    hSQL = 0;
    if (!OpenDataFile ("C:\\GMPOLICE\\CODES.TXT","",BT_READ,&hSQL))
        return FALSE;      
    
    OpenSymDict (OF_READWRITE);     
		
    
	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
    _fstrcpy (CurSymbol->Name,"ALL");
    _fstrcpy (CurSymbol->Desc,"Parent of All");  
    GlobalUnlock (hSym);
    SaveSymbol(hSym,0);
	DestroySymbol (hSym);

	hSym = AllocateNewSymbol ();
	CurSymbol = (LPSYMBOL)GlobalLock (hSym);
    _fstrcpy (CurSymbol->Name,"Crimes");
    _fstrcpy (CurSymbol->Desc,"Crime Incident Locations");
    CurSymbol->Type = 0;
    CurSymbol->NumElements=0;
    CurSymbol->Parent=GetDictSymbolNumber("ALL");        
    GlobalUnlock (hSym);
    SaveSymbol(hSym,0);
	DestroySymbol (hSym);
								
    while (FetchDBRec (hSQL))
    {
		hSym = AllocateNewSymbol ();
		CurSymbol = (LPSYMBOL)GlobalLock (hSym);
        GetValFromOpenFiles ("PARENT",CurSymbol->Name);
        GetValFromOpenFiles ("DESC",CurSymbol->Desc);  
        if (!GetDictSymbolNumber(CurSymbol->Name))
        {
	        CurSymbol->Parent=GetDictSymbolNumber("Crimes");
	        CurSymbol->Type = 0;         
		    GlobalUnlock (hSym);
		    SaveSymbol(hSym,0); 
		}
		else
		    GlobalUnlock (hSym);
		DestroySymbol (hSym);
    }
    CloseDataFile (TRUE, &hSQL);   
    if (!OpenDataFile ("C:\\GMPOLICE\\CODES.TXT","",BT_READ,&hSQL))
        return FALSE;      
	ExpandText (NewPointTemplate);
 	TemplateSymbol = GetDictSymbolNumber (NewPointTemplate); 
	hSym = GetDictSymDesc (TemplateSymbol,0); 
    while (FetchDBRec (hSQL))
    {
		CurSymbol = (LPSYMBOL)GlobalLock (hSym);
        GetValFromOpenFiles ("SYMNAME",CurSymbol->Name);
        GetValFromOpenFiles ("DESC",CurSymbol->Desc);
        GetValFromOpenFiles ("PARENT",Parent);
        CurSymbol->Parent=GetDictSymbolNumber(Parent);
        CurSymbol->Type = 1;         
	    GlobalUnlock (hSym);
	    SaveSymbol(hSym,0);
    }
	DestroySymbol (hSym);
    CloseDataFile (TRUE, &hSQL);   
    CloseSymDict();
    return TRUE;
} 

void DumpDCB (int Stream)
#if ENABLETRACE
{GSSiEnterProg (414);
#endif
{   
	DCB		dcb;
	LPDCB	pDCB=&dcb;
	char	str[128];
    if (TraceOn)
    {  
		GetCommState (Stream,&dcb);
		sprintf (str,"%s:%u","Id",(UINT)pDCB->Id);
		GSSiTrace (str);
		sprintf (str,"%s:%u","BaudRate",(UINT)pDCB->BaudRate);
		GSSiTrace (str);
		sprintf (str,"%s:%u","ByteSize",(UINT)pDCB->ByteSize);
		GSSiTrace (str);
		sprintf (str,"%s:%u","Parity",(UINT)pDCB->Parity);
		GSSiTrace (str);
		sprintf (str,"%s:%u","StopBits",(UINT)pDCB->StopBits);
		GSSiTrace (str);
		sprintf (str,"%s:%u","RlsTimeout",(UINT)pDCB->RlsTimeout);
		GSSiTrace (str);
		sprintf (str,"%s:%u","CtsTimeout",(UINT)pDCB->CtsTimeout);
		GSSiTrace (str);
		sprintf (str,"%s:%u","DsrTimeout",(UINT)pDCB->DsrTimeout);
		GSSiTrace (str);
		
		sprintf (str,"%s:%u","fBinary",(UINT)pDCB->fBinary);
		GSSiTrace (str);
		sprintf (str,"%s:%u","fRtsDisable",(UINT)pDCB->fRtsDisable);
		GSSiTrace (str);
		sprintf (str,"%s:%u","fParity",(UINT)pDCB->fParity);
		GSSiTrace (str);
		sprintf (str,"%s:%u","fOutxCtsFlow",(UINT)pDCB->fOutxCtsFlow);
		GSSiTrace (str);
		sprintf (str,"%s:%u","fOutxDsrFlow",(UINT)pDCB->fOutxDsrFlow);
		GSSiTrace (str);
		sprintf (str,"%s:%u","fDummy",(UINT)pDCB->fDummy);
		GSSiTrace (str);
		sprintf (str,"%s:%u","fDtrDisable",(UINT)pDCB->fDtrDisable);
		GSSiTrace (str);
	
	
		sprintf (str,"%s:%u","fOutX",(UINT)pDCB->fOutX);
		GSSiTrace (str);
		sprintf (str,"%s:%u","fInX",(UINT)pDCB->fInX);
		GSSiTrace (str);
		sprintf (str,"%s:%u","fPeChar",(UINT)pDCB->fPeChar);
		GSSiTrace (str);
		sprintf (str,"%s:%u","fNull",(UINT)pDCB->fNull);
		GSSiTrace (str);
		sprintf (str,"%s:%u","fChEvt",(UINT)pDCB->fChEvt);
		GSSiTrace (str);
		sprintf (str,"%s:%u","fDtrflow",(UINT)pDCB->fDtrflow);
		GSSiTrace (str);
		sprintf (str,"%s:%u","fRtsflow",(UINT)pDCB->fRtsflow);
		GSSiTrace (str);
		sprintf (str,"%s:%u","fDummy2",(UINT)pDCB->fDummy2);
		GSSiTrace (str);
	
		sprintf (str,"%s:%u","XonChar",(UINT)pDCB->XonChar);
		GSSiTrace (str);
		sprintf (str,"%s:%u","XoffChar",(UINT)pDCB->XoffChar);
		GSSiTrace (str);
		sprintf (str,"%s:%u","XonLim",(UINT)pDCB->XonLim);
		GSSiTrace (str);
		sprintf (str,"%s:%u","XoffLim",(UINT)pDCB->XoffLim);
		GSSiTrace (str);
		sprintf (str,"%s:%u","PeChar",(UINT)pDCB->PeChar);
		GSSiTrace (str);
		sprintf (str,"%s:%u","EofChar",(UINT)pDCB->EofChar);
		GSSiTrace (str);
		sprintf (str,"%s:%u","EvtChar",(UINT)pDCB->EvtChar);
		GSSiTrace (str);
		sprintf (str,"%s:%u","TxDelay",(UINT)pDCB->TxDelay);
		GSSiTrace (str);
	}
{
#if ENABLETRACE
GSSiExitProg (414);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}
BOOL PickPolygonD (HPDPOINT lpDpoints,long nPnts, double Offdist,HANDLE hElev)
#if ENABLETRACE
{GSSiEnterProg (993);
#endif
{   POINT	PickPoint;
	HPDPOINT	lpNewPoints;
	int	 Type, NearPoint; 
	DWORD	i;
	MNMXCORD	Rect;    
	float	Elev=NULL_ELEV;
	DPOINT	BeginPoint, EndPoint,PickedPoint, LastPoint, PickPointW, NodePoint;  
	POINT	BeginPointFile,EndPointFile,PickedPointFile;
	double	Area=0,Perim=0, MinDist=DBL_MAX, Dist, AZ[3]={0,0,0};    
	BOOL	Selected=FALSE, pina;
    
    if (PickNET)
{
#if ENABLETRACE
GSSiExitProg (993);
#endif
    	return FALSE;
}
    if (hHighlightArea)
{
#if ENABLETRACE
GSSiExitProg (993);
#endif
    	return (PickPolyInAreaD(0,lpDpoints,nPnts,0,0));
}
	PickPointW.x = (CurView->WBounds.xmn + CurView->WBounds.xmx) / 2;
	PickPointW.y = (CurView->WBounds.ymn + CurView->WBounds.ymx) / 2;
    if (PickPerim) 
    {
    	HANDLE hNew = GSSiGlobAlloc (GMEM_MOVEABLE,(nPnts+1)*(long)sizeof(DPOINT));
    	HPDPOINT	pNewPoints=(HPDPOINT)GlobalLock (hNew);
    	BOOL	rtn; 
    	long	np=nPnts;
    	
    	pina = POINT_IN_AREAD (PickPointW, nPnts, lpDpoints,NULL);
    	hmemmove ((HPSTR)pNewPoints,(HPSTR)lpDpoints,np*(long)sizeof(DPOINT));
    	pNewPoints[np++] = *lpDpoints;
    	rtn = PickPolylineD (pNewPoints,np,3,0,0,pina);
    	GSSiGlobUlFree (&hNew); 
    	if (rtn || PickPerim == 1)
{
#if ENABLETRACE
GSSiExitProg (993);
#endif
    		return rtn;
}
    }	
	
 	lpNewPoints = lpDpoints;
	for (i=0;i<nPnts;i++,lpDpoints++)
	{   
		Dist = ldistp (PickPointW,*lpDpoints);  
		if (Dist < MinDist)
		{
			MinDist = Dist;
			NodePoint = *lpDpoints;
			NearPoint = i;
		}
		if (!i)
			BeginPoint = *lpDpoints;
		else  
		{
			Perim += ldistp(LastPoint,*lpDpoints);
			Area += ( LastPoint.y - lpDpoints->y) * (lpDpoints->x + LastPoint.x) / 2;
		}
		LastPoint = *lpDpoints;
	} 
	Perim += ldistp(LastPoint,BeginPoint);
	Area += (LastPoint.y - BeginPoint.y) * (BeginPoint.x + LastPoint.x) / 2;  
	
	
/*		{
			HANDLE hCP = GSSiGlobAlloc (GHND,4); 
			LPSHORT pCP = GlobalLock (hCP); 
			double	Area2;
			
			*pCP = -1;   
			GlobalUnlock (hCP);
			Area2 = AreaFromPolyWithCurves (hUnSplinedPoly,nUnSplinedPoints,hCP);  
			GSSiGlobFree (&hCP);
		}*/
	if (hUnSplinedPoly)
		Area = AreaFromPolyWithCurves (hUnSplinedPoly,nUnSplinedPoints,hCurvePoints);
	EndPoint = LastPoint;
     
    if (PickingByRefno)
    	Selected = TRUE;
    else if (POINT_IN_AREAD (PickPointW, nPnts, lpNewPoints,NULL))
    	Selected = TRUE;
    else
    	Selected = FALSE;
    if (Selected)
	{   
		if (CurrentType == GF_TEXT) 
		{
			BeginPoint = LastElementBeginPoint;
			EndPoint = LastElementEndPoint;
			Type = 4;          
		}
		else
			Type = 3;
		
		GetItemMinMax (&CurrentItemMinMax,&Rect);
		PickedPoint = PickPointW;
		PickedPointFile = BasePtToFilePt (PickPointW);  
		if (hElev && nPnts == 3 && !PickingByRefno)
		{   
			LPFLOAT		pElev = (LPFLOAT)GlobalLock (hElev); 
			double		A1 = getazd (&lpNewPoints[0],&PickPointW);
			double		A2 = getazd (&lpNewPoints[1],&lpNewPoints[2]);  
			double		d1, d2, elev1;
			DPOINT		IntPt;
        	short		IRC = LIN_SEC (lpNewPoints[0].x,lpNewPoints[0].y,A1,
         				   			   lpNewPoints[1].x,lpNewPoints[1].y,A2,
          				   			   &IntPt.x, &IntPt.y);
                        if (IRC)
                        	IRC = 0;
                        else        
                        {
                        	d1 = ldistp (lpNewPoints[1],lpNewPoints[2]);
                        	d2 = ldistp (lpNewPoints[1],IntPt);
                        	elev1 = pElev[1] + (pElev[2] - pElev[1]) * (d2 / d1);
                        	d1 = ldistp (IntPt,lpNewPoints[0]);
                        	if (!d1)
                        		Elev = pElev[0];
                        	else
                        	{
	                        	d2 = ldistp (IntPt,PickPointW);
	                        	Elev = elev1 + (pElev[0] - elev1) * (d2 / d1); 
	                        }
                        }
			GlobalUnlock (hElev);
		}
		PickListAdd (FileNum,SubFile,FileInIndex,ItemSeg,CurrentRefno,CurrentDesc,CurrentiPen,
					   Type,0,Offdist,AZ,Perim,CurrentItem,CurElement,BeginPoint,EndPoint,PickedPoint,
					   NodePoint,NearPoint,
					   Area,&Rect,
					   BeginPointFile,EndPointFile,PickedPointFile,nPnts,Elev);
	}
{
#if ENABLETRACE
GSSiExitProg (993);
#endif
	return (Selected);
}
#if ENABLETRACE
}
#endif
}                  

void LoadDTM (void)
																							#if ENABLETRACE
																							{GSSiEnterProg (1356);
																							#endif
{   
	OFSTRUCT	OFStruct;
	HFILE		Fid1, Fid2;
	char		FileName[132];     
	HANDLE		hRec=GSSiGlobAlloc (GMEM_MOVEABLE,8192);
	LPSTR		pRec=GlobalLock (hRec);
	short		ii, length;  
	UINT		i, n, pos;
	long		GeoSeg, Bias, nFiles=0, MaxFiles=50, nOutOfRange=0, nSubCells=0, nLess12=0;
	short		NumElv, SubCell, Indeterminate, MinElv, MaxElv;    
	BOOL		OutOfRange; 
	short		SubcellData[1024]; 
	long		DTMData[1024], DTMData2[1024];
	BTVARDESC BTVar[2], *pVars;
	int		NumFields, Reclen, len;
	long	Offset;
	long	TotFileLen;
	GWDHEADER GWDHead; 
	LPGWDHEADER	lpGWDHead;
	GWFLDINFO GWFldInfo;
	LPGWFLDINFO	lpGWFldInfo;
	HANDLE hBT, hDB=0;
	HFILE	FidData;
	int		ibeg;
	GWFLDINFO FldInfo;
	char	File[128]="[%DL]attribut\\ot1.dtm"; 
	LPSTR	lpDot; 
	HANDLE	hCell;
	LPSHORT	SubcellDat; 
	LPLONG	pBias;  
	static	long	debugsubcell=845;  
	long	TotLen, CurLoc, SubCellID;
	

	DTMKEY		DTMKey;
	SUBCELLINFO	SUBCELLInfo;
	LPSUBCELLINFO	pSUBCELLInfo;
	LPSTR		CompressedDTMData;
	
	 lpGWDHead = &GWDHead; 
    _fmemset (lpGWDHead,0,sizeof(GWDHEADER));

	 FidData = GSSiOpenFile (File,&OFStruct,OF_CREATE);
	 GWDHead.NumFields=0;
	 GWDHead.NumIndex=1;
	 GWDHead.Version=1;
	 GWDHead.NumIndexFields[0]=1;
	 GWDHead.IndexFields[0][0]=0;
	 _lwrite (FidData,&GWDHead,sizeof(GWDHEADER));
	ibeg = 0;

	FldInfo.Len = sizeof(DTMKEY);
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"DTMKEY");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"BIAS");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = sizeof(SUBCELLINFO);
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"SUBCELLINFO");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	
	FldInfo.Len = 4096;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"COMPRESSEDNODES");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;
	
	 GWDHead.Reclen=ibeg; 
	 GWDHead.TimeStamp = 0;
	 _llseek (FidData,0,0);
	 _lwrite (FidData,&GWDHead,sizeof(GWDHEADER));
 	 _llseek (FidData,0,2);
			     
	 BTVar[0].BT_VARLEN=4;
	 BTVar[0].BT_VARTYP=BT_INTEGER;
	 BTVar[0].BT_VAROFF=0;
	 lpDot = _fstrrchr (File,'.');
	 _fstrcpy (lpDot,".in1");	
	 BT_CREATE (File, 4, FALSE, 1, 1,BTVar,FALSE, 0, 0, FALSE);
 	 GSSiClose (FidData);
	 _fstrcpy (lpDot,".dtm");	

     hDB = OpenGWDatabase (File,BT_WRITE);
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
	pSUBCELLInfo = (LPSUBCELLINFO)((LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY)));
	pBias = (LPLONG)((LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY) + sizeof(SUBCELLINFO)));
	CompressedDTMData = (LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY) + sizeof(SUBCELLINFO) + 4);
	
	Fid1 = GSSiOpenFile ("e:\\mgvngi\\filelist.txt",&OFStruct,OF_READ); 
	TotLen = _llseek (Fid1,0,2);
	_llseek (Fid1,0,0);
	CreateStatusWindow (hWndMain,1,NULL);
	while (ContinueProcessing && fgetstring (FileName,128,Fid1))
	{   
		nFiles++;
		Fid2 = GSSiOpenFile (FileName,&OFStruct,OF_READ);  
		while (fgetstring (pRec,7000,Fid2))
		{   
			nSubCells++;  
			if (nSubCells == debugsubcell)
				ii=1;
			OutOfRange=FALSE;
			GeoSeg = ldread (pRec,12) - 1; 
			SubCell = ldread (&pRec[12],6) - 1; 
			if (GeoSeg == 852606 && SubCell == 180)
				ii=1;
			DTMKey.GEOSEG_ROW = GeoSeg / 4096; 
			DTMKey.GEOSEG_COL = GeoSeg % 4096;    
			DTMKey.SUBCEL_ROW = SubCell / 16;
			DTMKey.SUBCEL_COL = SubCell % 16;    
			_fmemmove (&SubCellID,&DTMKey,4);
			Bias = ldread (&pRec[18],6);
			NumElv = ldread (&pRec[24],6); 
			Indeterminate = ldread (&pRec[30],6);
			if (Bias <=0)
				ii=1;
			n = min (1023,NumElv);
			pos = 36;
			for (i=0;i<n;i++,pos+=6)
				SubcellData[i] = ldread (&pRec[pos],6);   
			if (NumElv == 1024)
			{
				fgetstring (pRec,7000,Fid2);
				SubcellData[1023] = ldread (pRec,12);
			}
			n = min (1024,NumElv); 
			hCell = NEXPND (n,SubcellData); 
			if (hCell)
			{  
				SubcellDat = (LPSHORT)GlobalLock (hCell); 
				n=1024;
				MinElv = SHRT_MAX;
				MaxElv = SHRT_MIN;   
				_fmemset (DTMData,0,4096);
				for (i=0;i<n;i++)
				{   
					if (SubcellDat[i] < 31100)
						DTMData[i] = SubcellDat[i] + Bias;
					else
						DTMData[i] = LONG_MAX;
				}
				GSSiGlobUlFree (&hCell);    
				SUBCELLInfo = CompressSubcell (DTMData,CompressedDTMData,pBias);  
				_fmemset (DTMData2,0,4096); 
				ExpandSubcell (DTMData2,SUBCELLInfo,CompressedDTMData,pBias);
				for (i=0;i<1024;i++)
					if (DTMData[i] != DTMData2[i]) 
					{
						GSSiMessageBox (FileName,NULL,MB_ICONEXCLAMATION);
						goto Exit;        
					}
				*pSUBCELLInfo = SUBCELLInfo;
				Offset = _llseek (lpGWDHead->Fid,0,1);  
				length = sizeof(DTMKEY) + sizeof(SUBCELLINFO) + 4 + SUBCELLInfo.LENGTH;
			    _lwrite (lpGWDHead->Fid,(char *)&length,2);
			    _lwrite (lpGWDHead->Fid,(char *)&lpGWDHead->GWDData,length);
	        	BT_PUT (lpGWDHead->BTHandle[0],(LPSTR)&DTMKey,(LPSTR)&Offset);
	        }
	        else
	        {
	        	char	str[256];
	        	
	        	sprintf (str,"%ld %i %s",GeoSeg,SubCell,FileName);  
	        	AppendFile ("c:\\badcells.txt",str);
	        }
		}
Exit: 
		GSSiClose (Fid2);  
		CurLoc = _llseek (Fid1,0,1);
		StatusWindowUpdate (FileName,"", TotLen, CurLoc);
	}    
	ContinueProcessing = TRUE;
	GSSiClose (Fid1);   
	DestroyStatusWindow ();
	GSSiGlobUlFree (&hRec);  
	GlobalUnlock (hDB);
    CloseGWDatabase (hDB); 
{
																							#if ENABLETRACE
																							GSSiExitProg (1356);
																							#endif
	return;
}
																							#if ENABLETRACE
																							}
																							#endif
} 

void CreateCityExtract (void)
{ 
/*    HANDLE hDB, hDB2;
    LPGWFLDINFO lpFieldInfo;
    LPGWDHEADER lpGWDHead, lpGWDHead2;
    short         st, i, len,  index;
    LPVOID      lpVal; 
    short         FidNew;  
    OFSTRUCT    OFStruct;  
    long    nRecs, nLoaded, Offset;   
    LPSTR   lpMunic;
    
    hDB = OpenGWDatabase ("e:\\gwizdata\\attribut\\pid02.gmd",BT_READ);
    if (!hDB) return;
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);  
    FidNew = GSSiOpenFile("e:\\gmbob\\attribut\\pid02mgv.gmd",&OFStruct,OF_CREATE); 
    _lwrite(FidNew,(char *)lpGWDHead,sizeof(GWDHEADER));
    for (i=0,lpFieldInfo=lpGWDHead->pFldInfo;i<lpGWDHead->NumFields;i++,lpFieldInfo++)
    {
         _lwrite (FidNew,(char *)lpFieldInfo,sizeof(GWFLDINFO));
    }
    GSSiClose(FidNew); 
    hDB2 = OpenGWDatabase ("e:\\gmbob\\attribut\\pid02mgv.gmd",BT_WRITE);
    lpGWDHead2 = (LPGWDHEADER)GlobalLock (hDB2);     
    
    nRecs = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[0]);
    nLoaded = 0;
                                 
    lpFieldInfo=lpGWDHead->pFldInfo; 
    lpFieldInfo+=5;
    lpVal = &lpGWDHead->GWDData[lpFieldInfo->Beg];
    st = BT_FIND (lpGWDHead->BTHandle[0],lpGWDHead->pKeys[0],BT_FIRST,BT_ANY, (LPSTR)&Offset);
    while (!st)
    {
        FillGWDData (lpGWDHead,Offset);
        lpMunic = (LPSTR)lpVal;   
        if (!_fstrnicmp(lpMunic,"76",2)) 
        {
            Offset = _llseek(lpGWDHead2->Fid,0,2);
            _lwrite (lpGWDHead2->Fid,(char *)&len,2);
            _lwrite (lpGWDHead2->Fid,(char *)&lpGWDHead->GWDData,len); 
            _fmemcpy(lpGWDHead2->GWDData,lpGWDHead->GWDData,len);
            for (index=0;index<lpGWDHead2->NumIndex;index++)
            {
                GWDFormKey(lpGWDHead2,index,FALSE,0);
                BT_PUT (lpGWDHead2->BTHandle[index],lpGWDHead2->pKeys[index],(LPSTR)&Offset);  
            } 
        }
        ++nLoaded;
        st = BT_FIND (lpGWDHead->BTHandle[0],lpGWDHead->pKeys[0],BT_NEXT,BT_ANY, (LPSTR)&Offset);
    }
    GlobalUnlock (hDB);
    CloseGWDatabase (hDB);
    GlobalUnlock (hDB2);
    CloseGWDatabase (hDB2); */
    return; 
}
/*void TestGrayScale (HWND hWnd)
{   
	RECT	Rect, WindRect;    
	short	nRect=32;
	short	w,h, gs,i;   
	HDC		hDC;   
	double	val;
	COLORREF	Color;
	
	hDC = GetDC (hWnd);
	GetClientRect (hWnd,&WindRect);
	h = (WindRect.bottom - WindRect.top)/3; 
	w = (WindRect.right - WindRect.left)/(nRect+1); 
	
	for (i=0;i<=nRect;i++)
	{   
		gs = IDNINT (i * (double)255/(double)nRect); 
		Color = RGB(gs,gs,gs);
		Rect.left = WindRect.left + i*w;
		Rect.right = WindRect.left +(i+1)*w;
		Rect.top = WindRect.top;
		Rect.bottom = WindRect.top + h;
		FillRectPoly (hDC,&Rect,Color);
	}  
	for (i=0;i<=nRect;i++)
	{   
		val = i * (double)255/(double)nRect;  
		val = val * val;
		val = val / 255;
		gs = IDNINT (val);
		Color = RGB(gs,gs,gs);
		Rect.left = WindRect.left + i*w;
		Rect.right = WindRect.left +(i+1)*w;
		Rect.top = WindRect.top+h;
		Rect.bottom = Rect.top + h;
		FillRectPoly (hDC,&Rect,Color);
	}  
	for (i=0;i<=nRect;i++)
	{   
		val = i * (double)255/(double)nRect;  
		val = log (val);
		val = 255e0 * val / log (255e0);
		gs = IDNINT (val);
		Color = RGB(gs,gs,gs);
		Rect.left = WindRect.left + i*w;
		Rect.right = WindRect.left +(i+1)*w;
		Rect.top = WindRect.top+h+h;
		Rect.bottom = Rect.top + h;
		FillRectPoly (hDC,&Rect,Color);
	}  
	ReleaseDC (hWnd,hDC);
	return;
}*/

BOOL FAR PASCAL WELL_INFOMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{	char	str[128], TAG[64], SymName[9], Addresses[128];
	LONG	Segment, Refno;
	WORD	Offset;
	static int		item;
	HWND	hWnd; 
	int		TabStops[4]={30,80,122,300};
	LPGWFLDINFO	lpGWFldInfo;


 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
         cwCenter(hWndDlg, 0);
         /* initialize working variables                                */ 
		lpGWFldInfo = lpGWDHeadWell->pFldInfo + 1;
		str[0]='\0';
		_fstrncat (str,&lpGWDHeadWell->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
       	 SetDlgItemText (hWndDlg,IDC_cwi_cty,str); 
       	 
		lpGWFldInfo = lpGWDHeadWell->pFldInfo + 0;
		str[0]='\0';
		_fstrncat (str,&lpGWDHeadWell->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
       	 SetDlgItemText (hWndDlg,IDC_cwi_unique,str);
       	 
		lpGWFldInfo = lpGWDHeadWell->pFldInfo + 2;
		str[0]='\0';
		_fstrncat (str,&lpGWDHeadWell->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
       	 SetDlgItemText (hWndDlg,IDC_cwi_twp,str);
       	 
		lpGWFldInfo = lpGWDHeadWell->pFldInfo + 3;
		str[0]='\0';
		_fstrncat (str,&lpGWDHeadWell->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
       	 SetDlgItemText (hWndDlg,IDC_cwi_rng,str);
       	 
		lpGWFldInfo = lpGWDHeadWell->pFldInfo + 4;
		str[0]='\0';
		_fstrncat (str,&lpGWDHeadWell->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
       	 SetDlgItemText (hWndDlg,IDC_cwi_sec,str);
       	 
		lpGWFldInfo = lpGWDHeadWell->pFldInfo + 5;
		str[0]='\0';
		_fstrncat (str,&lpGWDHeadWell->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
       	 SetDlgItemText (hWndDlg,IDC_cwi_qtr,str);
       	 
		lpGWFldInfo = lpGWDHeadWell->pFldInfo + 9;
		str[0]='\0';
		_fstrncat (str,&lpGWDHeadWell->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
       	 SetDlgItemText (hWndDlg,IDC_cwi_elev,str);
       	 
		lpGWFldInfo = lpGWDHeadWell->pFldInfo + 12;
		str[0]='\0';
		_fstrncat (str,&lpGWDHeadWell->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
       	 SetDlgItemText (hWndDlg,IDC_cwi_depth_compl,str);
       	 
		lpGWFldInfo = lpGWDHeadWell->pFldInfo + 14;
		str[0]='\0';
		_fstrncat (str,&lpGWDHeadWell->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
       	 SetDlgItemText (hWndDlg,IDC_yr_drld,str);
       	 
		lpGWFldInfo = lpGWDHeadWell->pFldInfo + 15;
		str[0]='\0';
		_fstrncat (str,&lpGWDHeadWell->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
       	 SetDlgItemText (hWndDlg,IDC_mo_drld,str);
       	 
		lpGWFldInfo = lpGWDHeadWell->pFldInfo + 17;
		str[0]='\0';
		_fstrncat (str,&lpGWDHeadWell->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
       	 SetDlgItemText (hWndDlg,IDC_cwi_use_code,str);
       	 
		lpGWFldInfo = lpGWDHeadWell->pFldInfo + 18;
		str[0]='\0';
		_fstrncat (str,&lpGWDHeadWell->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
       	 SetDlgItemText (hWndDlg,IDC_cwi_well_status_code,str);
       	 
		lpGWFldInfo = lpGWDHeadWell->pFldInfo + 37;
		str[0]='\0';
		_fstrncat (str,&lpGWDHeadWell->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
       	 SetDlgItemText (hWndDlg,IDC_cwi_case_dia,str);
       	 
		lpGWFldInfo = lpGWDHeadWell->pFldInfo + 38;
		str[0]='\0';
		_fstrncat (str,&lpGWDHeadWell->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
       	 SetDlgItemText (hWndDlg,IDC_cwi_depth_cased,str);
         
		lpGWFldInfo = lpGWDHeadWell->pFldInfo + 48;
		str[0]='\0';
		_fstrncat (str,&lpGWDHeadWell->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
       	 SetDlgItemText (hWndDlg,IDC_cwi_aquifer_code,str);
         
		lpGWFldInfo = lpGWDHeadWell->pFldInfo + 57;
		str[0]='\0';
		_fstrncat (str,&lpGWDHeadWell->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
       	 SetDlgItemText (hWndDlg,IDC_cwi_house_no,str);
         
		lpGWFldInfo = lpGWDHeadWell->pFldInfo + 58;
		str[0]='\0';
		_fstrncat (str,&lpGWDHeadWell->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
		_fstrcat (str," ");
		lpGWFldInfo = lpGWDHeadWell->pFldInfo + 59;
		_fstrncat (str,&lpGWDHeadWell->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
       	 SetDlgItemText (hWndDlg,IDC_cwi_street_name,str);
         
		lpGWFldInfo = lpGWDHeadWell->pFldInfo + 61;
		str[0]='\0';
		_fstrncat (str,&lpGWDHeadWell->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
       	 SetDlgItemText (hWndDlg,IDC_cwi_city,str);
         
		lpGWFldInfo = lpGWDHeadWell->pFldInfo + 62;
		str[0]='\0';
		_fstrncat (str,&lpGWDHeadWell->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
       	 SetDlgItemText (hWndDlg,IDC_cwi_zip,str);
         
		lpGWFldInfo = lpGWDHeadWell->pFldInfo + 33;
		str[0]='\0';
		_fstrncat (str,&lpGWDHeadWell->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
       	 SetDlgItemText (hWndDlg,IDC_cwi_well_name,str);
         
         
       	 SendDlgItemMessage (hWndDlg,IDC_GEOLOGY,LB_SETTABSTOPS,4,(LPARAM)&TabStops);
         DisplayGeology ("[%DATA_LOC]attribut\\cwld.gwd",hWndDlg,IDC_GEOLOGY,WellID);

         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           {
            case IDOK:
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 EndDialog(hWndDlg, FALSE);
                 break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} /* End of WELL_INFOMsgProc*/ 

BOOL DisplayGeology (LPSTR DBName,HWND hWndDlg,int dlgitem, LPSTR WellID)
{   HANDLE hDB;
	LPGWFLDINFO	lpGWFldInfo;
	LPGWDHEADER	lpGWDHead;
	HANDLE		hBT;
	long		Offset; 
	double		rtn;
	char		str[256];
	int			st, i, len;
	char		Well[10];
    
    rtn = FALSE;
    SendDlgItemMessage (hWndDlg,dlgitem,LB_RESETCONTENT,NULL,NULL);
	hDB = OpenGWDatabase (DBName,BT_READ);
    if (!hDB) return (FALSE);
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
	hBT = lpGWDHead->BTHandle[0];
	_fstrcpy (Well,WellID);
	_fstrcat (Well,"  ");
	st = BT_FIND (hBT,Well,BT_FIRST,BT_GE, (LPSTR)&Offset);
Next:	if (!st)
	    {
	    	_llseek (lpGWDHead->Fid,Offset,0);
	       	_lread (lpGWDHead->Fid,&len,2);
	       	_lread (lpGWDHead->Fid,&lpGWDHead->GWDData,len);
	       	for (i=len;i<lpGWDHead->Reclen;i++) lpGWDHead->GWDData[i]='\0';
	    }
	    else
	    	goto NotFound; 
	    	
		lpGWFldInfo = lpGWDHead->pFldInfo;
		if (_fstrncmp(WellID,&lpGWDHead->GWDData[lpGWFldInfo->Beg],6)!=0) goto NotFound; 
		str[0] = '\0';
		lpGWFldInfo = lpGWDHead->pFldInfo + 7;
		_fstrncat (str,&lpGWDHead->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
		_fstrcat (str,"\t");
		lpGWFldInfo = lpGWDHead->pFldInfo + 5;
		_fstrncat (str,&lpGWDHead->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
		_fstrcat (str,"\t");
		lpGWFldInfo = lpGWDHead->pFldInfo + 6;
		_fstrncat (str,&lpGWDHead->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
		_fstrcat (str,"\t");
		lpGWFldInfo = lpGWDHead->pFldInfo + 4;
		_fstrncat (str,&lpGWDHead->GWDData[lpGWFldInfo->Beg],lpGWFldInfo->Len);
		SendDlgItemMessage (hWndDlg,dlgitem,LB_ADDSTRING,NULL,(LPARAM)str);
		st = BT_FIND (hBT,Well,BT_NEXT,BT_ANY, (LPSTR)&Offset); 
		goto Next;
	
	rtn = TRUE;
    NotFound: GlobalUnlock (hDB);
    CloseGWDatabase (hDB);
    return (rtn);
}

BOOL DeleteType3SubCell (HANDLE hSurf)
{
	long		CellID=270999782,ii;
	DTMKEY		DTMKey;
	SUBCELLINFO	SUBCELLInfo;
	LPSTR		CompressedDTMData;
	UINT		i, MinUseID;  
	long		MinUse=LONG_MAX, Offset, nbad=0, nRecs,nLoaded=0;
	LPLONG		pBias;  
	LPSUBCELLINFO	pSUBCELLInfo;
	LPGWDHEADER	lpGWDHead;     
	HANDLE		handle=0;
	short		pos=BT_FIRST, cond=BT_GE;  
	LPDTMINFO pDTMInfo=GlobalLock (hSurf);
	
	lpGWDHead = (LPGWDHEADER)GlobalLock (pDTMInfo->hDB); 
	pSUBCELLInfo = (LPSUBCELLINFO)((LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY)));
	pBias = (LPLONG)((LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY) + sizeof(SUBCELLINFO)));
	CompressedDTMData = (LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY) + sizeof(SUBCELLINFO) + 4); 
    nRecs = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[0]);
	CreateStatusWindow (hWndMain,1,NULL); 
//					BT_DELETE(lpGWDHead->BTHandle[0],(LPSTR)&CellID,(LPSTR)&Offset,FALSE); 

   	while (ContinueProcessing && !BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&CellID,pos,cond,(LPSTR)&Offset)) 
   	{
		unsigned short	len;    
		LPLONG	pCell;

	    _llseek (lpGWDHead->Fid,Offset,0);
	    _lread (lpGWDHead->Fid,&len,2);
	    _lread (lpGWDHead->Fid,&lpGWDHead->GWDData,len); 
	    if (pSUBCELLInfo->TYPE == 3)
	    {
	    	nbad++;
	    }
	    pos = BT_FIRST;
	    cond = BT_GT;
		StatusWindowUpdate (NULL,"Locate DTM voids", nRecs, ++nLoaded);
	} 
	DestroyStatusWindow();  
	ContinueProcessing = TRUE;
	GlobalUnlock (pDTMInfo->hDB);
	return handle;
}

BOOL CreatePickData (void)
{
    BTVARDESC  *pVars;
    short       NumFields, Reclen, len;
    GWDHEADER GWDHead; 
    LPGWDHEADER lpGWDHead;
    HANDLE  hVars, hDB;
    short       FidData,ibeg,NumVars;
    OFSTRUCT    OFStruct;
    GWFLDINFO FldInfo; 
    char	Name[128];   
    char	PrimeIndex[128];
    LPSTR	lpDot, lpEnd;
   
    _fstrcpy (Name,"pickdata.gmd");
    _fstrcpy (PrimeIndex,"pickdata.in1");
    lpGWDHead = &GWDHead; 
    
    FidData = GSSiOpenFile (Name,&OFStruct,OF_CREATE);   
    _fmemset (lpGWDHead,0,sizeof(GWDHEADER));
    GWDHead.NumIndex=1;
    GWDHead.Version=1002;
    GWDHead.NumIndexFields[0]=1;
    GWDHead.IndexFields[0][0]=0;
    _lwrite (FidData,(char *)&GWDHead,sizeof(GWDHEADER));
    ibeg = 0;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"INT_REFNO");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"BeginPointX");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"BeginPointY");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"EndPointX");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"EndPointY");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"PickedPointX");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"PickedPointY");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"PCT");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"Length");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"OffDist");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

	GWDHead.Reclen=ibeg; 
	GWDHead.TimeStamp = time(0);
	_llseek (FidData,0,0);
	_lwrite (FidData,(char *)&GWDHead,sizeof(GWDHEADER));
	_llseek (FidData,0,2);
	                 
	NumVars = 1;
	            
	hVars = GSSiGlobAlloc (GHND,NumVars * sizeof(BTVARDESC));
	pVars =(LPBTVARDESC) GlobalLock(hVars);
	            
	pVars->BT_VARLEN=4;
	pVars->BT_VARTYP=BT_INTEGER;
	pVars->BT_VAROFF=0;
	BT_CREATE (PrimeIndex, 4, FALSE, 1, 1,pVars,FALSE, 0, GWDHead.TimeStamp, FALSE);
	GSSiGlobUlFree (&hVars);
	GSSiClose (FidData);
	hDB = OpenGWDatabase (Name,BT_WRITE);
	if (!hDB) return (FALSE);
	CloseGWDatabase (hDB); 
	return TRUE;
}
 
short TestERDASDem (short dummy)
{
	typedef struct erdhead {
	  char hdword[6];
	  short pack,bands;
	  char res1[6];
	  long cols,rows,col1,row1;
	  char res2[56];
	  short maptyp,nclass;
	  char res3[14];
	  short utyp;
	  float area,xl,yt,xcell,ycell;
	} erdhead;
	erdhead eh; 
	HFILE	Fid;
	OFSTRUCT	OFStruct;
	long		Loc, StartLoc, row,col, n=5000, TotLen; 
    char	DTMFile[128]="[%DL]attribut\\ot1.dtm";
    HANDLE	hSurf = DTMOpen (DTMFile,-1.0);
    short	elv; 
    DPOINT	Point; 
    double	Elev;
    long	stopat=4945,ii;
	HFILE    FidErr = GSSiOpenFile ("c:\\demerr.txt",&OFStruct,OF_CREATE);

	Fid = GSSiOpenFile ("f:\\mndtm\\statedem.gis",&OFStruct,OF_READ); 
	TotLen = _llseek (Fid,0,2);
	_llseek (Fid,0,0);
//	CreateStatusWindow (hWndMain,1);
	_lread (Fid,&eh,sizeof(erdhead)); 
	StartLoc = _llseek (Fid,0,1);
    srand(1);    
     
    while (n--)
    {   
	    row = ((long)rand()*(long)eh.rows)/RAND_MAX;
	    col = ((long)rand()*(long)eh.cols)/RAND_MAX;
	    Loc = StartLoc + row * (long)eh.cols * 2 + col * 2;  
	    _llseek (Fid,Loc,0);
	    _lread (Fid,&elv,2);
	    Point.x = eh.xl + col * eh.xcell ;
	    Point.y = eh.yt - row * eh.ycell;  
	    if (n == stopat)
	    	ii=1;
	    Elev = NGIELV (Point,hSurf,0); 
	    if (fabs ((double)elv-Elev) > 1)
	    {   
	    	char	txt[128];
	    	sprintf (txt,"%f %f %i %f %ld %ld %ld",Point.x,Point.y,elv,Elev,n,row,col);
	    	fputstring (txt,FidErr); 
	    }
	} 
	GSSiClose (FidErr);
	GSSiClose (Fid);
	DTMClose (&hSurf);	
	return 0;
}

void GetFile (HWND hWndDlg,int dialogItem)
{char Text[512];
 int i;

 LPSTR lpText = Text;

    WaitCursor (+1);
	GetUMMessage (lpText);
		while (_fstrcmp (lpText,"%END") !=0)
		 GetUMMessage (lpText);
	WaitCursor (-1);
	return;
}


int GetList (HWND hWndDlg,int dialogItem, LPSTR listid)
{
	return (GetListSub (hWndDlg, dialogItem, LB_RESETCONTENT, LB_ADDSTRING,listid));
}

int GetListCB (HWND hWndDlg,int dialogItem, LPSTR listid)
{
	return (GetListSub (hWndDlg, dialogItem, CB_RESETCONTENT, CB_ADDSTRING,listid));
}

int GetListSub (HWND hWndDlg,int dialogItem, int RESET, int ADD, LPSTR listid)
{char Text[256], str[256];
 int i=0, nchar;
 FILE *Fid=NULL;
 char ListFile[144]; 
 HWND	SaveWnd; 

 LPSTR lpText = Text;
 LPSTR lpEndText, lpBar, lpSpace;
                 
	_fstrcpy (ListFile,UI.HomeDir);
    _fstrcat (ListFile,"\\lists\\");
    _fstrcat (ListFile,listid);
    for (lpSpace=ListFile;*lpSpace;lpSpace++)
    {
    	if (*lpSpace == ' ') *lpSpace = '_';
    }
    if (StandAlone)
    {
 	    Fid=fopen (ListFile,"r");
 	}
	else
	{
		WaitCursor (1);
		if (!SaveStuff)
			GetTempFileName(0,"lst",0,ListFile);
		Fid=fopen (ListFile,"w+");
		GetUMMessage (lpText);
		while (_fstrcmp (lpText,"%END") !=0)
		{    
			i++;
			putw (_fstrlen(lpText),Fid);
			fputs (lpText,Fid);
			GetUMMessage (lpText);
		}
	}
	SendDlgItemMessage (hWndDlg,dialogItem,RESET,NULL,NULL);
	if (!Fid) goto End;
	rewind (Fid);
	nchar = getw (Fid);
	while (nchar != EOF)
	{	fgets (lpText,nchar+1,Fid);
		lpEndText = lpText + nchar;
		*lpEndText = '\0';
		lpBar = _fstrchr(lpText,'|');
		if (lpBar)
			if (RESET == CB_RESETCONTENT)
			{
				*lpBar = '\0';
				_fstrcpy (str,lpText);
				_fstrcat (str,"                                                                                \t");
				_fstrcat (str,++lpBar);
				SendDlgItemMessage (hWndDlg,dialogItem,ADD,NULL,(LPARAM)str);
			}
			else
			{
				*lpBar = '\t';
				SendDlgItemMessage (hWndDlg,dialogItem,ADD,NULL,(LPARAM)lpText);
			}
		else
			SendDlgItemMessage (hWndDlg,dialogItem,ADD,NULL,(LPARAM)lpText);
		nchar = getw (Fid);
	}
	fclose (Fid); 
	if (!SaveStuff)
		GSSiRemove (ListFile);
End:WaitCursor (-1);
	return(i);
}

void SendUMMessage (LPSTR message)
{  if (StandAlone) return;
   WaitCursor (1);

   if (OpenUM ( ))
   {  if (message[0])
   		WriteUM (message,_fstrlen(message));
   	  else
   	  	WriteUM (" ",1);
   }
   WaitCursor (-1);

	return;
}

int GetUMMessage (LPSTR message)
{char Chr;
 int mesLen;
 LPSTR lps;
   WaitCursor (1);
   mesLen = 0;
   if (OpenUM ( ))
   {	mesLen = ReadUM(message);
   		lps = message + mesLen;
	    *lps = NULL;
   }
   WaitCursor (-1);
   return (mesLen);
}

BOOL	OpenUM (void)
{	short	ier, nbytes, ibytes;
	char	CharPriority[4];

	if (StandAlone) return(TRUE);
	if (ConnectBySIO)
	{
	}
	else
	{	if (UMSocket==INVALID_SOCKET)
		{	
			OpenWinSock ();
			UMSocket = connectToServer (UI.svrName,UMPort);
			if (UMSocket==INVALID_SOCKET)
			{   CloseUM(TRUE);
			 	BlowOut("Exiting due to invalid socket",NULL);
			}
			ier = sendto(UMSocket, UI.Userid,4, 0,
		                (LPSOCKADDR)&serverSockAddr, sizeof(serverSockAddr));
		    if(ier == SOCKET_ERROR)
		    {  	MessageBox(NULL,"Unable to Transmit User ID",
	    			 	    "Unable to Connect to Server", MB_OK|MB_TASKMODAL);
	    	    CloseUM(TRUE);
                BlowOut(NULL,NULL);
		    }
			TimeOutTimer =  SetTimer(hWndMain, 3, 60000, (FARPROC) NULL);
	        nbytes = recv(UMSocket,(LPSTR)&ibytes,2, 0);
	        if (nbytes == 0 || nbytes == SOCKET_ERROR)
	        {	err = WSAGetLastError();
        		if (err)
			    {
					wsprintf(ErrMsg, "Error Code: %d ",err);
			    	MessageBox( NULL, ErrMsg,
			    				"Unable to Connect to Server", MB_OK|MB_TASKMODAL);
				}
	        	CloseUM(TRUE);
                BlowOut(NULL,NULL);
            }
	        else
    	 	{	KillTimer(hWndMain, 3);
    	 		WriteUM ("%CONNECT",8);
    	 		GetUMMessage (CharPriority);
    	 		UI.MaxPriority = atoi (CharPriority);
            }
        }
		return (TRUE);
	}
}

void	CloseUM (BOOL Error)
{	int		ier;

	if (StandAlone) return;
	if (ConnectBySIO)
	{
	}
	else
	{	if (UMSocket!=INVALID_SOCKET)
		{	if (!Error) WriteUM ("%DISCONNECT",11);
			WSACancelBlockingCall ();
			ier = shutdown (UMSocket,2);
			ier = closesocket (UMSocket);
		}
		UMSocket = INVALID_SOCKET;
		ier = WSACleanup();
	}
	if (StayLiveTimer) KillTimer(hWndMain, 2);
	StayLiveTimer = 0;

}

int		ReadUM (LPSTR Message)
{	int		nbytes, ier;
	if (ConnectBySIO)
	{
	}
	else
	{	TimeOutTimer =  SetTimer(hWndMain, 3, 60000, (FARPROC) NULL);
		nbytes = recv(UMSocket,Message,1024, 0);
        if (nbytes <= 0 || nbytes == SOCKET_ERROR)
        {	err = WSAGetLastError();
    		if (err)
		    {
				wsprintf(ErrMsg, "Error Code: %d ",err);
		    	MessageBox( NULL, ErrMsg,
		    				"Unable to read from Server", MB_OK|MB_TASKMODAL);
			}
        	CloseUM(TRUE);
            BlowOut(NULL,NULL);
        }
        else
   	 	   KillTimer(hWndMain, 3);
        ier = sendto(UMSocket,(LPSTR)&nbytes, 2, 0,
		             (LPSOCKADDR)&serverSockAddr, sizeof(serverSockAddr));
        if (UMIODebug)
        {   
        	char mess[256];
        	LPSTR lpEnd;
        	
        	_fstrcpy (mess,"Get message:"); 
        	lpEnd = _fstrchr(mess,'\0');
        	_fmemmove (lpEnd,Message,nbytes); 
        	lpEnd+=nbytes;
        	*lpEnd = '\0';
        	SetWindowText(hWndMain,mess);
        }
		return (nbytes);
	}
}

BOOL	WriteUM (LPSTR Message, int len)
{	int		ier, nbytes, ibytes;
	if (ConnectBySIO)
	{
	}
	else
	{	ier = sendto(UMSocket,Message, len, 0,
		             (LPSOCKADDR)&serverSockAddr, sizeof(serverSockAddr));
	    if(ier == SOCKET_ERROR)
        {	err = WSAGetLastError();
    		if (err)
		    {
				wsprintf(ErrMsg, "Error Code: %d ",err);
		    	MessageBox( NULL, ErrMsg,
		    				"Unable to read from Server", MB_OK|MB_TASKMODAL);
		        CloseUM (TRUE);
		        BlowOut(NULL,NULL);
			} 
		}
		TimeOutTimer =  SetTimer(hWndMain, 3, 60000, (FARPROC) NULL);
        nbytes = recv(UMSocket,(LPSTR)&ibytes,2, 0);
        if (nbytes <= 0 || nbytes == SOCKET_ERROR)
        {	err = WSAGetLastError();
    		if (err)
		    {
				wsprintf(ErrMsg, "Error Code: %d ",err);
		    	MessageBox( NULL, ErrMsg,
		    				"Unable to Connect to Server", MB_OK|MB_TASKMODAL);
			}
        	CloseUM(TRUE);
            BlowOut(NULL,NULL);
        }
        else
	        KillTimer(hWndMain, 3);
		if (StayLiveTimer) ier = KillTimer(hWndMain, 2);
        StayLiveTimer =  SetTimer(hWndMain, 2, 60000, (FARPROC) NULL);
        if (UMIODebug)
        {   
        	char mess[256];
        	LPSTR lpEnd;
        	
        	_fstrcpy (mess,"Send message:"); 
        	lpEnd = _fstrchr(mess,'\0');
        	_fmemmove (lpEnd,Message,len); 
        	lpEnd+=len;
        	*lpEnd = '\0';
        	SetWindowText(hWndMain,mess);
        }
	}
	return (TRUE);
} 


short	LoadGS (short i)
{
	LoadGSExtract("j:\\msp424\\",NULL,NULL,NULL);
	return 1;
}

void LoadGSExtract(LPSTR FromDir, HWND hWndDlg, int MessageDlgItem, int StatusDlgItem)
{   HANDLE	hDBSegdata2, hSegData2, hFtoB;
	int		SegDataFid2, st, st2, st3, len, FidTIGER, FidActAddr, FidMap;
	long	NewNum, NumTLID, Done; 
	int		NewNum2;
	LPGWDHEADER	lpGWDHead2;
	long	TLID, Offset2, Offset, Frame, FileLen;
	SEGDATAOLD	Segdata2;
	int	ii, i2, version;           
	char	StreetName[34], OldName[34], Name[128];         
	BOOL	SaveUpdate;   
	OFSTRUCT	OFStruct;
	HCURSOR	hcurSave;
	BOOL	OK; 
	UINT	readlen;

	if (!FromDir)
		if (MessageBox( GetFocus(),"Continue with load?",
			"Verify", MB_OKCANCEL) == IDCANCEL) return;
	SaveUpdate = UpdateAddress;
	CloseAddressFiles(TRUE);
	if (!UpdateAddress) UpdateAddress=TRUE; 
	if (FromDir)
		_fstrcpy (Name,FromDir);
	else
		Name[0]='\0';
	_fstrcat (Name,"extract\\version");
	FidTIGER = GSSiOpenFile (Name,&OFStruct,OF_READ);
    if (FidTIGER == HFILE_ERROR)
    	version = 0;
    else
    {
    	version = 1;
    	GSSiClose (FidTIGER);
    }
	if (FromDir)
		_fstrcpy (Name,FromDir);
	else
		Name[0]='\0';
	_fstrcat (Name,"extract\\TIGER");
	FidTIGER = GSSiOpenFile (Name,&OFStruct,OF_READ);
	if (FidTIGER==HFILE_ERROR)
	{
		MessageBox( GetFocus(), "TIGER extract file not found","Error", MB_OK);
		return;
	}
	
    OpenAddressFiles (hWndMain); 
    BT_CLOSE(hNames1);
    BT_CLOSE(hNames2);
	hNames1 = BT_OPEN ("[%GEOSPAN_LOC]stname1.btr", 0, BT_WRITE, 0);
	hNames2 = BT_OPEN ("[%GEOSPAN_LOC]stname2.btr", 0, BT_WRITE, 0); 
    
    
    if (version)
    	readlen = sizeof(SEGDATA); 
    else
    	readlen = sizeof(OLDSEGDATA);
 	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT));
	if (hWndDlg)
		SetDlgItemText (hWndDlg,MessageDlgItem,"Loading street segment data"); 
	FileLen = GSSifilelength (FidTIGER);
	NumTLID = FileLen / (sizeof(TLID) + readlen + 34); 
	Done = 0;
    while (_lread(FidTIGER,&TLID,4)==4)
    {
		_lread (FidTIGER,&Segdata,readlen);
		_lread (FidTIGER,&StreetName,34);  
		if (LongStreetNum)
		{
		    st2 = BT_FIND(hNames2,(LPSTR)&StreetName,BT_FIRST,BT_EQ,(LPSTR)&NewNum);
		    if (st2)
		    {
			    if (BT_FIND (hNames1,(LPSTR)&NewNum,BT_LAST,BT_ANY,OldName)) NewNum = 0;
			    NewNum++;
		    	BT_PUT (hNames1,(LPSTR)&NewNum,(LPSTR)&StreetName);
		    	BT_PUT (hNames2,(LPSTR)&StreetName,(LPSTR)&NewNum); 
		    }
			Segdata.street_num = NewNum;  
		}
		else
		{
		    st2 = BT_FIND(hNames2,(LPSTR)&StreetName,BT_FIRST,BT_EQ,(LPSTR)&NewNum2);
		    if (st2)
		    {
			    if (BT_FIND (hNames1,(LPSTR)&NewNum2,BT_LAST,BT_ANY,OldName)) NewNum2 = 0;
			    NewNum2++;
		    	BT_PUT (hNames1,(LPSTR)&NewNum2,(LPSTR)&StreetName);
		    	BT_PUT (hNames2,(LPSTR)&StreetName,(LPSTR)&NewNum2); 
		    }
			Segdata.street_num = NewNum2;  
		}
		st = BT_FIND (hSegData,(LPSTR)&TLID,BT_FIRST,BT_EQ,(LPSTR)&Offset);
		if (!st)
		{   
			WriteSegData (Offset,&Segdata);
		}
		else
		{
			Offset = _llseek (SegDataFid,0,2);
			WriteSegData (Offset,&Segdata);
			BT_PUT (hSegData,(LPSTR)&TLID,(LPSTR)&Offset);
        }
		ActAddKey.TLID =  TLID;
		ActAddKey.House = LONG_MIN;
		st3=BT_FIND(hActAdd,(LPSTR)&ActAddKey,BT_FIRST,BT_GE,(LPSTR)&ActAddData);
		
		while (!st3)
		{
			Frame = ActAddData.Frame;
			if (ActAddKey.TLID==TLID)
			{
				ActAddData.Frame=LONG_MIN;
				BT_PUT (hActAdd,(LPSTR)&ActAddKey,(LPSTR)&ActAddData);
				st3=BT_FIND(hActAdd,(LPSTR)&ActAddKey,BT_NEXT,BT_ANY,(LPSTR)&ActAddData);
			}
			else
				st3 = 31;
		}
		if (hWndDlg)
			PctBox (GetDlgItem(hWndDlg,StatusDlgItem), NumTLID, Done++, 0);
    }
	GSSiClose(FidTIGER);
	if (hWndDlg)
		SetDlgItemText (hWndDlg,MessageDlgItem,"Loading address data");
	if (FromDir)
		_fstrcpy (Name,FromDir);
	else
		Name[0]='\0';
	_fstrcat (Name,"extract\\actaddr");
	FidActAddr = GSSiOpenFile (Name,&OFStruct,OF_READ);
	if (FidActAddr==HFILE_ERROR)
	{
		MessageBox( GetFocus(), "Address extract file not found","Error", MB_OK);
		return;
	}
	FileLen = GSSifilelength (FidActAddr);
	NumTLID = FileLen / (sizeof(ActAddKey) + 4); 
	Done = 0;
    while (_lread(FidActAddr,&ActAddKey,sizeof(ActAddKey))==
    		sizeof(ActAddKey))
    {
/*    	_lread(FidActAddr,&Frame,4);  
    	ActAddData.Frame = Frame;  
    	ActAddData.AddFrame=0;
    	ActAddData.Flags=0; */ 
    	if (version)
			_lread(FidActAddr,&ActAddData,sizeof(ActAddData)); 
		else
		{
			_lread(FidActAddr,&ActAddData.Frame,sizeof(ActAddData.Frame)); 
			ActAddData.Flags = 0;
			ActAddData.AddFrame = 0;
		}
		BT_PUT (hActAdd,(LPSTR)&ActAddKey,(LPSTR)&ActAddData);
		if (hWndDlg)
			PctBox (GetDlgItem(hWndDlg,StatusDlgItem), NumTLID, Done++, 0);
    } 
    
	GSSiClose(FidActAddr);
	CloseAddressFiles(TRUE); 
	_fstrcpy(PltName,"maplib\\geospan.plt");
	if (!OpenMap (CurView->hWnd,NULL)) return; 
	
	if (hWndDlg)
		SetDlgItemText (hWndDlg,MessageDlgItem,"Loading map data");
	if (FromDir)
		_fstrcpy (Name,FromDir);
	else
		Name[0]='\0';
	_fstrcat (Name,"extract\\map");
	UpdateAddress=SaveUpdate;

	OK =LoadMapExtract (Name,GetDlgItem(hWndDlg,StatusDlgItem));
	CloseMap(TRUE);                                                 
	if (!OK)
	{
		MessageBox( GetFocus(), "Map extract file not found","Error", MB_OK);
		return;
	} 
	
		
	if (hWndDlg)
	{    
		HANDLE	hVidIndexFrom;
		VIDINDEXKEY		VidIndexKey;
		VIDINDEXDATA	VidIndexData; 
		BTHEAD	BTHead;
		
		SetDlgItemText (hWndDlg,MessageDlgItem,"Loading video index");
		_fstrcpy (Name,FromDir);
		_fstrcat (Name,"vidindex.btr");
		hVidIndexFrom = BT_OPEN (Name,0, BT_READ, 0); 
		if (hVidIndexFrom)
		{
			hVidIndex = BT_OPEN ("vidindex.btr",0, BT_WRITE, 0);
			if (!hVidIndex)
			{
				BTVARDESC	BTVar[2];
								
				BTVar[0].BT_VARTYP=BT_INTEGER;
				BTVar[0].BT_VARLEN=4;
				BTVar[0].BT_VAROFF=0;
				BTVar[1].BT_VARTYP=BT_INTEGER;
				BTVar[1].BT_VARLEN=2;
				BTVar[1].BT_VAROFF=4;
				BT_CREATE ("vidindex.btr", sizeof(VidIndexData), FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
				hVidIndex = BT_OPEN ("vidindex.btr",0, BT_WRITE, 0);
			} 
			
	
			GetBTHeader (hVidIndexFrom,&BTHead);
			NumTLID = BTHead.BT_NUMRECS;
			Done = 0;
			st = BT_FIND (hVidIndexFrom,(LPSTR)&VidIndexKey,BT_FIRST,BT_ANY,(LPSTR)&VidIndexData);
			while (!st)
			{
		  	 	BT_PUT(hVidIndex,(LPSTR)&VidIndexKey,(LPSTR)&VidIndexData);   
				st = BT_FIND (hVidIndexFrom,(LPSTR)&VidIndexKey,BT_NEXT,BT_ANY,(LPSTR)&VidIndexData);
				PctBox (GetDlgItem(hWndDlg,StatusDlgItem), NumTLID, Done++, 0);
			}
			BT_CLOSE (hVidIndex); 
			hVidIndex = 0;
			BT_CLOSE (hVidIndexFrom);
		} 
	}
    GSSiSetCursor (hcurSave); 
    if (!FromDir)
		MessageBox( GetFocus(), "Load Complete","", MB_OK);
	return;
} 

LoadMapExtract (LPSTR Name,HWND hWndUpdate)
{
	HFILE	FidMap;
	long	FileLen, NumTLID, Done, TLID, Offset, BaseRec;
	UINT	Length; 
	ITEM	*ItemHeader;
	HANDLE	hMap, hFtoB=NULL;
	LPSHORT	pMap, EndItem;
	short	Type, Version, i2,id; 
	OFSTRUCT	OFStruct;
	long	NewFileMarker=LONG_MIN, lbuf;  
	LPSTR	LPpltBuf;   
	HANDLE	hBuf;
	HPSTR	pBuf;
	
	FidMap = GSSiOpenFile (Name,&OFStruct,OF_READ);
	if (FidMap==HFILE_ERROR) return FALSE;
	FileLen = GSSifilelength (FidMap); 
	
	NumTLID = FileLen; 
	Done = 0;  
	hBuf = GSSiGlobAlloc (GMEM_MOVEABLE,UINT_MAX);
	
	_lread (FidMap,&Version,2);
	if (Version != 1)
	{
		_llseek (FidMap,0,0);
		hFtoB = ReadTranData (FidMap);
	}

	while (_lread(FidMap,&TLID,4)==4)
	{    
		if (TLID == NewFileMarker)
		{
			if (hFtoB)
				GlobalFree (hFtoB);
			hFtoB = ReadTranData (FidMap);
			goto Next;
        }
		Done += 4;       
		if (Version == 1)
		{
			Done += _lread(FidMap,&BaseRec,4);
			Done += _lread(FidMap,&Type,2);   
		}
		else
			Type = 1;
		Done += _lread(FidMap,&Length,2);  
		hMap = GSSiGlobAlloc (GMEM_MOVEABLE,Length+2);
		pMap = (LPINT)GlobalLock (hMap); 
		LPpltBuf = (LPSTR)pMap;
		Done += _lread(FidMap,pMap,Length); 
		ItemHeader = (ITEM *)pMap; 

	    EndItem = pMap + abs(ItemHeader->Len);
	    EndItem+=6;  
	    *EndItem = 0;
		ConvertPoly (pMap,hFtoB,hTranBaseToFile,LPpltBuf);
		
		lbuf = 0;
		pBuf = GlobalLock (hBuf);  
		BufWrite (&pBuf,&lbuf,(HPSTR)pMap,Length);
		id=13;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		Offset = -1;
		BufWrite(&pBuf,&lbuf,(HPSTR)&Offset,4);
		id = 0;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		GlobalUnlock (hBuf);
		pBuf = GlobalLock (hBuf);
		GetFileConnectOffset(Type,0,ItemHeader->MinMax,(short)lbuf,BaseRec,pBuf);  
		GlobalUnlock (hBuf);
		GlobalUnlock(hMap);
		GlobalFree(hMap);
		if (hWndUpdate)
			PctBox (hWndUpdate, NumTLID, Done, 0); 
Next:
	;
	} 
	GlobalFree (hBuf);
	GlobalFree (hFtoB);
	GSSiClose (FidMap);
	return TRUE;
}
BOOL FileMap (HWND hWnd)
{
    HANDLE hMF;
    HANDLE  hMFHead;
    WMFHEADER   *pMFHead;
    short     MFid;
    DWORD       MFsize;
    LPSTR       pMF;
    OFSTRUCT    OFStruct;


     if (Wincap)
     {
        WinExec ("wincap.exe",SW_SHOW); 
        DoPaint = TRUE;
     }  
     else
     {hMFHead = (HANDLE) GSSiGlobAlloc (GMEM_MOVEABLE,sizeof(WMFHEADER));
     pMFHead = (LPWMFHEADER)GlobalLock (hMFHead);
     MFid = GSSiOpenFile ("\\temp.wmf",(LPOFSTRUCT) &OFStruct,OF_READ);
     _lread (MFid,pMFHead,sizeof(WMFHEADER));
     pMFHead->idChecksum=ComputeWMFChecksum( pMFHead );
     GSSiClose (MFid);


     hMF = CreateMetaFile (0);
     pMFHead->idKey = WMFKEY;
     pMFHead->hMF = 0;
     pMFHead->bbox.left = CurView->Bounds.xmn;/* tight bounding box */
     pMFHead->bbox.right = CurView->Bounds.xmx;/* tight bounding box */
     pMFHead->bbox.top = CurView->Bounds.ymn;/* tight bounding box */
     pMFHead->bbox.bottom = CurView->Bounds.ymx;/* tight bounding box */
     pMFHead->cInch = 500;/* metafile units per inch */
     pMFHead->idChecksum=ComputeWMFChecksum( pMFHead );


       DisplayPlotInit(hWnd,TRUE);
       if (! (hWnd, hMF)) return(FALSE);
       if (WindowZoomed()) SetNewBoundsToBounds();

       SetBounds (hWnd,hMF);
       while (DisplaySeg (hMF,FALSE));

     hMF = CloseMetaFile (hMF);
     MFid = GSSiOpenFile ("\\temp.wmf",(LPOFSTRUCT) &OFStruct,OF_CREATE);
     _lwrite (MFid,(char *)pMFHead,sizeof(WMFHEADER));
     hMF = GetMetaFileBits (hMF);
     MFsize = GlobalSize(hMF);
     pMF = GlobalLock (hMF);
     if (!BigWrite (MFid,pMF,MFsize))
        GSSiMessageBox("Error writing metafile - disk may be full",
                   "Fatal Write Error",MB_OK|MB_ICONQUESTION|MB_TASKMODAL);

     GlobalUnlock (hMF);
     GSSiClose (MFid);
     }
     return (TRUE);
}

BOOL PrintBanderMap (HWND hWnd)
{	HDC hPr;
    DOCINFO DocInfo;
    RECT	Rect;
   LPPRINTDLG lpPDChunk;
   static	HANDLE hPDChunk=NULL;
   short xPage, yPage;
   WORD wSize;
   BOOL bError;
   FARPROC lpfnAbortProc, lpfnPrintDlgProc;
   HBRUSH	BkBrush;
   BOOL		rtn;
   LPVIEWPORT	lpSaveView; 
   HANDLE	hSaveView[32];
   BOOL		SaveHaveBounds[32];
   MNMXCORD	SaveWBounds[32];
   int		iview;

	if (!OpenConfig(NULL)) return(FALSE);

     wSize = sizeof(PRINTDLG);
     if (!hPDChunk)
     {
     	if (!(lpPDChunk = (LPPRINTDLG)AllocAndLockMem(&hPDChunk, wSize)))
        	return(MemError());
	    InitializeStruct(IDC_PRINTDLG, (LPSTR)lpPDChunk);
     }
     else
     	lpPDChunk = (LPPRINTDLG) GlobalLock (hPDChunk);
     	
     DoPaint = FALSE;
     if (PrintDlg(lpPDChunk) != 0)
     {	hPr = lpPDChunk->hDC;
        gbUserAbort = FALSE;
        bError = FALSE;
        Printing = TRUE;
        lpfnPrintDlgProc = MakeProcInstance(PrintDlgProc, ghInst);
        ghPrintingDlg = CreateDialog(ghInst, "PRINTING", ghWnd,
                                         lpfnPrintDlgProc);
	    lpfnAbortProc = MakeProcInstance(AbortProc, ghInst);
	    Escape(hPr, SETABORTPROC, 0, (LPSTR)(long)lpfnAbortProc, NULL); 
         DocInfo.cbSize = sizeof(DOCINFO);
         _fstrcpy((char *) DocInfo.lpszDocName, CfgName);
         DocInfo.lpszOutput = (LPSTR) NULL;
         StartDoc(hPr,&DocInfo);
         //  StartPage (hPr);
	    xPage = GetDeviceCaps(hPr, HORZRES);
	    yPage = GetDeviceCaps(hPr, VERTRES);
	    Escape(hPr, SETCOPYCOUNT, sizeof(int),(LPCSTR) &lpPDChunk->nCopies, &lpPDChunk->nCopies);
        for(;;)
        {
          Escape(hPr, NEXTBAND, 0, (LPSTR)NULL, &Rect);
          if(IsRectEmpty(&Rect)) break;
          DPtoLP(hPr, (POINT FAR*) &Rect, 2);
	       // Rect.left = 0;
	       //Rect.top = 0;
	       //Rect.bottom = yPage;
	       //Rect.right = xPage;
	       MainRect = Rect; 
		   MaxDimension = max (MainRect.right-MainRect.left,MainRect.bottom - MainRect.top);
		   ShadowInc = (float) IDNINT (((float)MaxDimension * ShadowPct)/100.0F);
		   BkBrush = CreateSolidBrush (WindowColor);
		   FillRect (hPr,&Rect,BkBrush);
		   DeleteObject (BkBrush);
		   for (iview=0;iview<NumViewports;iview++)
		   {
		   		CurView=pViewports[iview];
		   		SaveHaveBounds[iview]=CurView->HaveBounds;
		   		SaveWBounds[iview]=CurView->WBounds;
			    hSaveView[iview] = GlobalAlloc (GPTR,sizeof(VIEWPORT));
			    lpSaveView = (LPVIEWPORT)GlobalLock (hSaveView[iview]);
				*lpSaveView = *CurView;  
				GlobalUnlock (hSaveView[iview]);
		   } 
		   SetupViewports (NULL,hPr,0,Rect);  
		   NumViewportsToDisplay = NumViewports;
		   for (DisplayViewID=0;DisplayViewID<NumViewportsToDisplay;DisplayViewID++)
		   {
				CurView=pViewports[DisplayViewID];
				ResetViewport (TRUE);
				if (SaveHaveBounds[DisplayViewID])
				{
					CurView->HaveBounds = TRUE;
					CurView->WBounds = SaveWBounds[DisplayViewID];
					CurView->NewBounds = CurView->WBounds;
				}
		   }
		   DisplayViewID = 0;
		   while (DisplayViewID<NumViewportsToDisplay)
		   {    CurView=pViewports[DisplayViewID];
		   		if (CurView->NumFiles) SetBoundsRect2 (CurView->DrawRect);
				if (DisplayViewport (hWnd,hPr)) 
				{  
				   PrintMessage (CurView->ID,PltName);

			       if (OpenMap (hWnd, hPr))
			       {   
   	       	   	       SetDisplayMode (CurView->hDC, GF_MAPMODE);
	
				       while (DisplaySeg (hPr) && !gbUserAbort); 
				   }
			     }
		   }
		   for (DisplayViewID=0;DisplayViewID<NumViewportsToDisplay;DisplayViewID++)
		   {
			   lpSaveView = (LPVIEWPORT)GlobalLock (hSaveView[DisplayViewID]);
		   		*pViewports[DisplayViewID]=*lpSaveView;
				GlobalUnlock (hSaveView[DisplayViewID]);
				GlobalFree (hSaveView[DisplayViewID]); 
				CurView=pViewports[DisplayViewID];
	    		SetBounds(CurView->hWnd,CurView->hDC);

		   }

           i=EndPage (hPr);
           EndDoc(hPr);
	       //i=Escape(hPr, ENDDOC, 0, NULL, NULL);
           DeleteDC(lpPDChunk->hDC);
           /*if (lpPDChunk->hDevMode)
              GlobalFree(lpPDChunk->hDevMode);
           if (lpPDChunk->hDevNames)
              GlobalFree(lpPDChunk->hDevNames);*/
	    } // end of the for loop
	    if (!gbUserAbort)
	    {
	       DestroyWindow(ghPrintingDlg);
	       ghPrintingDlg = NULL;
	    }
	    if (bError)
	       MessageBox(ghWnd, "Error while printing", gszAppName, MB_OK);
	    else
	    {
	       if (gbUserAbort)
	          MessageBox(ghWnd, "Printing Aborted", gszAppName, MB_OK);
	    }
	    FreeProcInstance(lpfnAbortProc);
	    FreeProcInstance(lpfnPrintDlgProc);
	}
    else
     {
     //Process error.  Be sure to free any memory that may be associated with
     //hDevMode or hDevNames.
       /* if (lpPDChunk->hDevMode)
           GlobalFree(lpPDChunk->hDevMode);
        if (lpPDChunk->hDevNames)
           GlobalFree(lpPDChunk->hDevNames);*/
        ProcessCDError(CommDlgExtendedError());
     }
    GlobalUnlock(hPDChunk);
    /*GlobalFree(hPDChunk); */
    Printing = FALSE; 
    DoPaint = TRUE;
    return (rtn);

}
int DescScan(int desc,WORD SegSize)
{
    static	HANDLE	hTable=0;
    typedef struct {int	desc, tot;} TABLE;
    TABLE	*DescTable;
	int		i;
	static	long	totdesc, totseg, totchain, totlen,
					NumDescInTable, chainid, maxchain,
					TotSegLen, TotChainLen, maxdesc,totitem;
	
	if (!DoDescScan) return (0);
	switch (desc)
	{   case -1:  /*start segment */
			TotSegLen+= SegSize;
			totlen += NumDescInTable;
			maxdesc = max (maxdesc,NumDescInTable);
		/*	NumDescInTable=0;*/
			totseg++;  
			chainid=0;
			break;                  
			
		case -2:	/* start chain segment */  
			TotChainLen+=SegSize;                         
			chainid++;
			maxchain = max (chainid,maxchain); 
			totchain++;
			break;
			
		case -3:		/* start file */ 
			if (CurView->PassID>2) break;
			totlen=totchain=totseg=maxchain=maxdesc=NumDescInTable=totitem=0;  
			if (hTable)
				GlobalFree (hTable);
			hTable = GlobalAlloc (GMEM_MOVEABLE,4096);
			break;
			
		case -4:	/* end file */ 
		{
			double		AveChainLen, AveSegLen, pct;
			
			AveChainLen=AveSegLen=0;
			
			totlen += NumDescInTable;
			maxdesc = max (maxdesc,NumDescInTable);
			if (totseg)
				AveSegLen = TotSegLen / totseg;
			if (totchain)
				AveChainLen = TotChainLen / totchain;
			for (i=0,DescTable=GlobalLock(hTable);
				 i<NumDescInTable;i++,DescTable++)
			{   
				pct = 100 * (double)DescTable->tot/totitem;
			}
			GlobalUnlock (hTable);
			
			if (hTable)
				GlobalFree (hTable); 
		}

			break;
			
		default:
			totitem++;
			for (i=0,DescTable=GlobalLock(hTable);
				 i<NumDescInTable;i++,DescTable++)
			{
				if (desc == DescTable->desc)
				{
					DescTable->tot++;
					goto Exit;
				}
			}
			NumDescInTable++;
			DescTable->desc = desc;
			DescTable->tot=1;
	Exit:	GlobalUnlock (hTable);
			break;
	}
	return 0;
					
}

HBITMAP PointAt (HDC hDC,POINT Coord,HBITMAP hSavedBM, RECT *SavedRect)
{	POINT ArrowPnt[8] = {0,0,-20,0,-8,4,-23,19,-19,23,-4,8,0,20,0,0};
 	HBRUSH hOldBrush, hRedBrush, hOldPen, hRedPen;
 	int OldMode, i;
 	static POINT	Points[8];
 	POINT ScreenCoord;

	if (hSavedBM)
	{
		RestoreScreen (hDC,hSavedBM,*SavedRect);
		DeleteObject (hSavedBM);
	}
 	SavedRect->top = INT_MAX;
 	SavedRect->bottom = INT_MIN;
 	SavedRect->left = INT_MAX;
 	SavedRect->right = INT_MIN;
 	for (i=0; i<8; i++)
 	{	Points[i].x = ArrowPnt[i].x + Coord.x;
 		Points[i].y = ArrowPnt[i].y + Coord.y;
 		SavedRect->left = min (SavedRect->left,Points[i].x);
 		SavedRect->right = max (SavedRect->right,Points[i].x);
 		SavedRect->top = min (SavedRect->top,Points[i].y);
 		SavedRect->bottom = max (SavedRect->bottom,Points[i].y);
 	}
 	hSavedBM = SaveScreen (hDC, *SavedRect);

	hRedBrush =   CreateSolidBrush(RGB(255,   0,   0));
	hRedPen =   CreatePen(PS_SOLID,1,RGB(255,   0,   0));
	hOldBrush = SelectObject (hDC,hRedBrush);
	hOldPen = SelectObject (hDC,hRedPen);
 	Polygon (hDC,Points,8);
 	SelectObject (hDC,hOldBrush);
 	DeleteObject(hRedBrush);

 	SelectObject (hDC,hOldPen);
 	DeleteObject(hRedPen);
 	ReleaseDC (hWndMain,hDC);
 	return hSavedBM;

} 
   

BOOL DrawSymbol (HDC hDC, int isym,POINT Point,int symsiz,COLORREF Color)
{   
	HBRUSH	brush, CurBrush;
	HPEN	pen, CurPen;  
	int		i;           
	DPOINT	NewPoint, SymPoint; 
	LPPOINT	pPoints, pPnt;
	LPSYMBOLVECTOR	pSymVector; 
	HANDLE	hPoints;
	
	
	if (!LoadSymbol (isym)) return FALSE;
	
	hPoints = GlobalAlloc(GMEM_MOVEABLE,NumSymVectors*sizeof(POINT));
	pPoints = GlobalLock (hPoints);   
	pSymVector = GlobalLock (hSymVectors);
	
	SymPoint.x = Point.x;
	SymPoint.y = Point.y;
	for (i=0,pPnt=pPoints;i<NumSymVectors;i++,pPnt++,pSymVector++)
	{
		NewPoint = dnewpt (SymPoint,pSymVector->AZM+PY,pSymVector->Dist*symsiz); 
		pPnt->x = IDNINT(NewPoint.x);
		pPnt->y = IDNINT(NewPoint.y);
	}
	
	brush = CreateSolidBrush (Color);
	CurBrush = SelectObject (hDC,brush); 
	pen = CreatePen (PS_SOLID,0,AutoYellow(Color));
	CurPen = SelectObject (hDC,pen);
	i = Polygon (hDC,pPoints,NumSymVectors);         
	SelectObject (hDC,CurBrush);                         
	SelectObject (hDC,CurPen);  
	DeleteObject (pen);
	DeleteObject (brush);  
	GlobalUnlock (hPoints);
	GlobalFree (hPoints);  
	UnloadSymbol();

	return TRUE;
}

BOOL LoadSymbol (int isym)
{                        
	LPSYMBOLVECTOR	pSymVector;
	
	switch (isym)
	{   
		default:
		case 3300:
			NumSymVectors = 4;
			hSymVectors = GlobalAlloc (GMEM_MOVEABLE,NumSymVectors*sizeof(SYMBOLVECTOR));
			pSymVector = GlobalLock (hSymVectors);
			
			pSymVector->AZM = 0.75 * PY;
			pSymVector++->Dist =  1.0;  
			
			pSymVector->AZM = 0.25 * PY;
			pSymVector++->Dist =  1.0;  
			
			pSymVector->AZM = 1.75 * PY;
			pSymVector++->Dist =  1.0;  
			
			pSymVector->AZM = 1.25 * PY;
			pSymVector++->Dist =  1.0;  
			
			GlobalUnlock (hSymVectors);
			break; 
			
		case 3301:
			NumSymVectors = 3;
			hSymVectors = GlobalAlloc (GMEM_MOVEABLE,NumSymVectors*sizeof(SYMBOLVECTOR));
			pSymVector = GlobalLock (hSymVectors);
			
			pSymVector->AZM = 0.5 * PY;
			pSymVector++->Dist =  1;  
			
			pSymVector->AZM = 1.25 * PY;
			pSymVector++->Dist =  1;  
			
			pSymVector->AZM = 1.75 * PY;
			pSymVector++->Dist =  1;  
			
			GlobalUnlock (hSymVectors);
			break; 
			
		case 3302:
			NumSymVectors = 4;
			hSymVectors = GlobalAlloc (GMEM_MOVEABLE,NumSymVectors*sizeof(SYMBOLVECTOR));
			pSymVector = GlobalLock (hSymVectors);
			
			pSymVector->AZM = 0.5 * PY;
			pSymVector++->Dist =  1.0;  
			
			pSymVector->AZM = 1.0 * PY;
			pSymVector++->Dist =  1.0;  
			
			pSymVector->AZM = 1.5 * PY;
			pSymVector++->Dist =  1.0;  
			
			pSymVector->AZM = 2.0 * PY;
			pSymVector++->Dist =  1.0;  
			
			GlobalUnlock (hSymVectors);
			break; 
			
		case 3303:
			NumSymVectors = 8;
			hSymVectors = GlobalAlloc (GMEM_MOVEABLE,NumSymVectors*sizeof(SYMBOLVECTOR));
			pSymVector = GlobalLock (hSymVectors);
			
			pSymVector->AZM = 0.5 * PY;
			pSymVector++->Dist =  1.0;  
			
			pSymVector->AZM = 0.75 * PY;
			pSymVector++->Dist =  0.25;  
			
			pSymVector->AZM = 1.0 * PY;
			pSymVector++->Dist =  1.0;  
			
			pSymVector->AZM = 1.25 * PY;
			pSymVector++->Dist =  0.25;  
			
			pSymVector->AZM = 1.5 * PY;
			pSymVector++->Dist =  1.0;  
			
			pSymVector->AZM = 1.75 * PY;
			pSymVector++->Dist =  0.25;  
			
			pSymVector->AZM = 2.0 * PY;
			pSymVector++->Dist =  1.0;  

			pSymVector->AZM = 2.25 * PY;
			pSymVector++->Dist =  0.25;  
			
			GlobalUnlock (hSymVectors);
			break; 
			
	} 
	return TRUE;
}

void UnloadSymbol (void)
{
	GlobalFree (hSymVectors);
}

short GetUMSymDef (LPSTR Snam,short UMSnum,short View,LPSHORT pRecIn,HFILE Fid)
{          
	char		snam[10]; 
	HANDLE		hSTR = GSSiGlobAlloc (GMEM_MOVEABLE,5000);
	LPSTR		str=GlobalLock (hSTR);
	short		i, l,snum, n,iatt,lview, iview;
	LPSTR		pStr, eStr;  
	LPSHORT		pRec;
    
    _llseek (Fid,0,0); 
    _fstrcpy (snam,Snam);
    PadString(snam,' ',8);
    while (fgetstring (str,4096,Fid))
    { 
    	if (!UMSnum)
    		i = _fstrnicmp (&str[12],snam,8);
    	else
    	{
    		snum = ldread (&str[6],6);
    		if (snum == UMSnum)
    			i = 0;
    		else
    			i = 1;
    	}
    	if (!i)
    		goto FoundSym;
    }
    GSSiGlobUlFree (&hSTR); 
    return 0;

FoundSym:
	eStr = _fstrchr (str,0);             
	pStr = &str[20];
    l = ldread (pStr,6); 
    pStr += 6;  
    iview = -1;
NextView: 
	pRec = pRecIn; 
	iview++;
    if (pStr > eStr) 
    {
	    GSSiGlobUlFree (&hSTR); 
    	return 0;
    }
    iatt = ldread(pStr,6);
    pStr+=12;
    lview = n = ldread(pStr,6); 
	*pRec++ = iatt;
	*pRec++ = 0;
	*pRec++ = lview;
	pStr+=6;
	while (lview--)
	{
		*pRec++ = ldread(pStr,6);
		pStr+=6;
	} 
    if (iview == View)    
    {
	    GSSiGlobUlFree (&hSTR); 
    	return n;
    }
    else
    	goto NextView;
}   
  
short GetUMSymPolyx (LPSTR Snam,short Snum,short View,double OFF,double Scale,double Rot,
					LPSHORT pnPoly,LPSHORT plPoly,LPHANDLE phPoly,HFILE Fid,HPDPOINT points,LPSHORT pnp)
{   
	short	nPoly=0, l, ISCALE,NUMNOD,JPEN,J=5,K,I,ii, SymNum;
	short	INSCD[201];   
	BYTE	flip[3];  
	DPOINT	OFFPoint={0,0};
	LPDPOINT	pPoints;
	double 	FACTO = 1, XSTRT,YSTRT, XNODE,YNODE,SCALE,ROT, OFF2; 
	struct	{
			 unsigned int type :3,
			 			  esym :1,
			 			  penwt:2,
			 			  scale:2,
			 			  invert:1,
			 			  view	:3,
			 			  shade	:3,
			 			  uselvoff:1;
			 } attint;
	
	if(!(l = GetUMSymDef (Snam,Snum,1,&INSCD[1],Fid))) 
	{
		return 0;                 
	}
	attint.type = JATTR (INSCD[1],1,3);  
//	if (attint.type)
//		return 0;
	attint.esym = JATTR (INSCD[1],4,1);
	attint.penwt = JATTR (INSCD[1],5,2);
	attint.scale = JATTR (INSCD[1],7,2);
	attint.invert = JATTR (INSCD[1],9,1);
	attint.view= JATTR (INSCD[1],10,3);
	attint.shade = JATTR (INSCD[1],13,3);
	attint.uselvoff = JATTR (INSCD[1],16,1);
//C***** SET SCALE FACTOR  
	
      switch (attint.scale)
      {
      	case 1:
      		FACTO=IDNINT((FACTO+.125)*4)/4;
      	break;
      	case 2:
      		FACTO = 1;
        break;
	  }
//C******* GET SYMBOL SCALE FACTOR IF POINT (TYPE 10) RECORD
//      CALL GETPSF(PSCALE)
//      FACTO=FACTO*PSCALE
//C***** GET BEGINNING NODE NUMBER COORDS.
//	   if (INSCD[4] < 0)
//	   	*pnp = 0;
       NODE(INSCD[4],&XSTRT,&YSTRT);  
       if (!Snum)
       {
	       points[*pnp].x = 0;
	       points[(*pnp)++].y = 0;
	   }
	   else if (OFF)
       {
	       points[*pnp].x = 0;
	       points[(*pnp)++].y = OFF;
	   }
	   else if (*pnp)
	   	OFFPoint = points[*pnp-1];
//      CALL ROTATE(ROT1)
      NUMNOD=INSCD[3]+3;   
      if (attint.type == 2) //multiple line
      {   
      	  *pnp = 0;
	      OFF2 = (fabs((double)INSCD[6])-31000)/1000;
	      if (INSCD[6] < 0)
	      	OFF2 = -OFF2;
	      SymNum = INSCD[7] - 23000;  
	      if (SymNum)
		  	GetUMSymPoly (Snam,SymNum,View,OFF2,Scale*SCALE,Rot,pnPoly,plPoly,phPoly,Fid,points,pnp);
	      OFF2 = (fabs((double)INSCD[8])-31000)/1000;
	      if (INSCD[8] < 0)
	      	OFF2 = -OFF2;
	      SymNum = INSCD[9] - 23000;  
	      if (SymNum)
		  	GetUMSymPoly (Snam,SymNum,View,OFF2,Scale*SCALE,Rot,pnPoly,plPoly,phPoly,Fid,points,pnp);
	      return 0;
      }
//C***** PLOT SYMBOL
S40:  for (I=J;I<NUMNOD+1;I++)
	  {
//         JPEN=2;
//         if(INSCD[I] < 0) JPEN=3;
         if(INSCD[I] < -31000)
         	ii=1;
         if(INSCD[I] < 0)
         {
         	if (*pnp>1)
         	{ 
         		plPoly[*pnPoly] = *pnp;
         		phPoly[*pnPoly] = GSSiGlobAlloc (GMEM_MOVEABLE,*pnp*sizeof(DPOINT));
         		pPoints = GlobalLock (phPoly[*pnPoly]);
         		_fmemmove (pPoints,points,*pnp*sizeof(DPOINT));
         		GlobalUnlock (phPoly[*pnPoly]);     
         		(*pnPoly)++;
         	}
         	*pnp = 0; 
         }
         else if (INSCD[I] >= 23000)
	        goto S60;
         NODE(INSCD[I],&XNODE,&YNODE);
         points[*pnp].x = XNODE - XSTRT + OFFPoint.x;
         points[(*pnp)++].y = YNODE-YSTRT+OFF + OFFPoint.y;
//         YNODE=YNODE-YSTRT;
//         CALL PLOT(XNODE*FACTO,YNODE*FACTO,JPEN)
	  }
		if (*pnp>1)
		{ 
			plPoly[*pnPoly] = *pnp;
			phPoly[*pnPoly] = GSSiGlobAlloc (GMEM_MOVEABLE,*pnp*sizeof(DPOINT));
			pPoints = GlobalLock (phPoly[*pnPoly]);
			_fmemmove (pPoints,points,*pnp*sizeof(DPOINT));
			GlobalUnlock (phPoly[*pnPoly]);     
			(*pnPoly)++;
         	*pnp = 0; 
		}
      return 0;
//C***** PLOT EMBEDDED SYMBOL
S60:    K=I;
      if(INSCD[I]>=30000) goto S70;    
      SCALE = 1.0;
      ROT = 0;
      while (INSCD[I] > 26200)
      { 
      	if (INSCD[I] < 28001 && INSCD[I] > 27000)
      		ROT = ((double)INSCD[I] - 27000) / 100.0;
      	if (INSCD[I] < 29001 && INSCD[I] > 28000)
      		SCALE = ((double)INSCD[I] - 28000) / 100.0;
      	I++;
      }
      SymNum = INSCD[I] - 23000;  
      if (SymNum)
	  	GetUMSymPoly (Snam,SymNum,View,0,Scale*SCALE,Rot,pnPoly,plPoly,phPoly,Fid,points,pnp);
/*      CALL EMBSYM(INSCD,K,NSYM,NPEN,AFACT,ROT2)
      afact=afact*pscale     {carry pscale into embedded symbol by s.w.}
      npen=-1       {make PSUB1 use pen weight from NSYM}
      CALL PSUB1(NSYM,ROT2,AFACT,NPEN,IRC)
      IF(IRC.GT.0) RETURN
      ROT2=ROT1
      NPEN=-1
      AFACT=1.
      CALL GOBACK*/
      goto S71;
//C***** PLOT EMBEDDED ANNOTATION
S70:    XNODE=XSTRT;
      YNODE=YSTRT;
//      CALL EMBANN(INSCD,K,XNODE,YNODE,ROT1,IRC)
//      IF(IRC.GT.0)RETURN
S71:    J=I+1;
      *pnp = 0;
      goto S40;

}

void LoadInfo(void)
{
	struct	_find_t	FileInfo; 
	char	FName[128], UDI[34], str[1026], Prefix[10],InFile[128];  
	LPSTR	lpUDI, lpEnd;  
	int		l;
	FILE	*Fid;
					      	 
	  Fid = fopen ("c:\\input.txt","r");
	  fgetss (str,1024,Fid);
		  
	  while (str[0]!='"') 
	  {                
	  	ExpandText (str);
	  	fgetss (str,1024,Fid);
	  }
		  
	  ProcessDelimTextHeader(str);
	     
	  while (fgetss (str,1024,Fid))
	  {
      	GetDelimTextData(str);
		GetGlobalCVal ("[%DOCDIR]",CurDocDir,"[%DATA_LOC]document");
		ExpandText(CurDocDir);
		_mkdir (CurDocDir); 
	 	_fstrcat (CurDocDir,"\\");
	 	_fstrcpy(Prefix,"[PREFIX]");
	 	_fstrcpy(UDI,"[UDI]");
	 	ExpandText(Prefix);
	 	ExpandText(UDI);
		_fstrcat (CurDocDir,Prefix); 
		_mkdir (CurDocDir); 
	 	_fstrcat (CurDocDir,"\\"); 
	 	_fstrcpy (UDI,UDI);
	 	l=_fstrlen(UDI);
	 	lpUDI = UDI;
	 	while (l>0)
	 	{   
	 		char	SaveChar;
				 		
	 		lpEnd = lpUDI + 8;
	 		SaveChar = *lpEnd;
	 		*lpEnd = 0;
			_fstrcat (CurDocDir,lpUDI); 
			_mkdir (CurDocDir); 
		 	_fstrcat (CurDocDir,"\\"); 
		 	*lpEnd = SaveChar;
		 	lpUDI = lpEnd;
		 	l-=8;
	 	} 
	 	_fstrcpy(InFile,"[FILE]");
	 	ExpandText(InFile);  
	 	if (_fstrstr(InFile,"FORFEIT"))
	 		sprintf (str,"%s%s",CurDocDir,"forfeit.doc");
	 	else
	 		sprintf (str,"%s%s",CurDocDir,"pca.doc"); 
	 	copyfile(str,InFile);
 }
 fclose (Fid);
 return;
} 

void LoadMCDS (void)
{   
	char	Name[128]="address\\munics.btr";
	char	Name2[128]="address\\munnames.btr";
	char	Name3[128]="address\\munzips.btr";
	char	str[256];
	BTVARDESC BTVar[2];  
	HANDLE	hBT, hBT2, hBT3;
	HFILE	Fid;
	OFSTRUCT	OFStruct;  
	long	FIPSCode;      
	short	Dummy=0;

    HANDLE hDB;
    LPGWFLDINFO lpGWFldInfo;
    LPGWDHEADER lpGWDHead;
    HANDLE      hSQL;
    long        Offset; 
    double      rtn;
    LPVOID      lpVal; 
    short         st, i, len,ifield;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr;
    LPFIELDINFO lpFieldInfo; 
    HANDLE      SaveHandle;
    LPFILEPATH  FilePathPtr; 
    BOOL        More;
    short         rc;  
	MUNICZIPS	MunZIPs;
	MUNICNAME	MunName;
	ALTMUNICNAME	ALTMunName;
    
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BT_CREATE (Name, sizeof(MUNICNAME), FALSE, 1, 1,(LPBTVARDESC)&BTVar,FALSE, 0, 0, FALSE);
    hBT = BT_OPEN (Name,0,BT_WRITE,0);  
	BTVar[0].BT_VARTYP=BT_CHAR;
	BTVar[0].BT_VARLEN=64;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=64;
	BT_CREATE (Name2, 2, FALSE, 2, 1,(LPBTVARDESC)&BTVar,FALSE, 0, 0, FALSE);
    hBT2 = BT_OPEN (Name2,0,BT_WRITE,0);  
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=4;
	BT_CREATE (Name3, 2, FALSE, 2, 1,(LPBTVARDESC)&BTVar,FALSE, 0, 0, FALSE);
    hBT3 = BT_OPEN (Name3,0,BT_WRITE,0);  
    hSQL = 0;
    if (!OpenDataFile ("D:\\TLG\\JUN1999\\TLGRDS_L.DBF","",BT_READ,&hSQL))
        return;      
    
    while (FetchDBRec (hSQL))
    {
        GetValFromOpenFiles ("CENMUN_L",str);
        FIPSCode = atol (str);
    	ALTMunName.Munic = FIPSCode;
        GetValFromOpenFiles ("CITYLEFT",str); 
        Truncate (str);  
        _fstrupr (str);
        _fstrncpy (MunName.FullName,str,sizeof(MunName.FullName)); 
        _fstrncpy (ALTMunName.ALTName,str,sizeof(ALTMunName.ALTName)); 
    	BT_PUT (hBT2,&ALTMunName,&Dummy); 
        GetValFromOpenFiles ("CTYLABRV",str); 
        Truncate (str);
        _fstrupr (str);
        _fstrncpy (MunName.Abv,str,sizeof(MunName.Abv)); 
        _fstrncpy (ALTMunName.ALTName,str,sizeof(ALTMunName.ALTName)); 
    	BT_PUT (hBT2,&ALTMunName,&Dummy); 
    	BT_PUT (hBT,&FIPSCode,&MunName); 
    	MunZIPs.Munic = FIPSCode;
        GetValFromOpenFiles ("ZIP5_L",str);
        MunZIPs.ZIP = atol (str);
    	BT_PUT (hBT3,&MunZIPs,&Dummy);
        GetValFromOpenFiles ("CENMUN_R",str);
    	MunZIPs.Munic = atol(str);
        GetValFromOpenFiles ("ZIP5_R",str);
        MunZIPs.ZIP = atol (str);
    	BT_PUT (hBT3,&MunZIPs,&Dummy);
    }
    CloseDataFile (TRUE, &hSQL);   
    BT_CLOSE (hBT);
    BT_CLOSE (hBT2);
    BT_CLOSE (hBT3);
	return;
}

void DisplayTwoVThemeLegend()
{   int		xmargin, ymargin;
	long	h, w;
	int		width, x, y, fHeight, MaxTextWidth, twidth;
	HBRUSH	BkBrush;
	int		iclass;
	RECT	ClassColorBox;
	HFONT	hfont, hfontOld, hfont2;
	DWORD	TextExtent;
	char	Text[256], Title[256];
	char	Val1[32], Val2[32];
	long	TotCount;
	float	Pct;
	int		MinFontHeight=2;
	int		inc;
	LPSTR	lpText;
	char	lpLine[256];
    
    _fstrcpy (Title,CurTheme->Title);
    ExpandText (Title);
    SetDisplayMode (CurView->hDC, GF_TEXTMODE);
	CurTheme->Rect=PctRect (CurView->DrawRect,-CurTheme->Margin);
	xmargin = (((long)CurTheme->Rect.right - CurTheme->Rect.left) * CurTheme->InnerMargin) / 100;
	ymargin = (((long)CurTheme->Rect.bottom - CurTheme->Rect.top) * CurTheme->InnerMargin) / 100;
	h = (((long)CurTheme->Rect.bottom - CurTheme->Rect.top) * CurTheme->TitleHeight) / 100;
	CurTheme->TitleBox.top = CurTheme->Rect.top + ymargin;
	CurTheme->TitleBox.bottom = CurTheme->TitleBox.top + h;
	CurTheme->TitleBox.left = CurTheme->Rect.left + xmargin;
	CurTheme->TitleBox.right = CurTheme->Rect.right - xmargin;
	CurTheme->InfoBox.top = CurTheme->TitleBox.bottom + ymargin;
	CurTheme->InfoBox.bottom = CurTheme->Rect.bottom - ymargin;
	CurTheme->InfoBox.left = CurTheme->Rect.left + xmargin;
	CurTheme->InfoBox.right = CurTheme->Rect.right - ymargin;
	CurTheme->ScatterBox = CurTheme->InfoBox;
	CurTheme->ScatterBox.right = CurTheme->InfoBox.right;

	FillRectPoly (CurView->hDC,&CurTheme->Rect,CurTheme->BGColor);
	FillRectPoly (CurView->hDC,&CurTheme->InfoBox,RGB(255,255,255));
	FillRectPoly (CurView->hDC,&CurTheme->TitleBox,CurTheme->TitleBoxBG);

	width = CurTheme->TitleBox.right - CurTheme->TitleBox.left;
	fHeight = CurTheme->TitleHeight+1;
	twidth = INT_MAX;
	while (twidth>width && fHeight>2)
	{   
		fHeight--;
		CurTheme->TitleFont.lfHeight = -MulDiv(fHeight,
											   GetDeviceCaps(CurView->hDC, LOGPIXELSY), 72);
		hfont = CreateFontIndirect((PLOGFONT)&CurTheme->TitleFont);
		hfontOld = SelectObject(CurView->hDC, hfont);
		TextExtent = GetTextExtent (CurView->hDC,Title,_fstrlen(Title));
		lpText = Title;
		twidth = 0;	
	    while (NextLine (&lpText,lpLine))
	    {
			TextExtent = GetTextExtent (CurView->hDC,lpLine,_fstrlen(lpLine));
			twidth = max (twidth,LOWORD (TextExtent));
		}
	}
	x = CurTheme->TitleBox.left + ((CurTheme->TitleBox.right - CurTheme->TitleBox.left) - twidth)/2;
	CurTheme->TitleFont.lfHeight = -MulDiv(CurTheme->TitleHeight-2,
										   GetDeviceCaps(CurView->hDC, LOGPIXELSY), 72);
	hfont = CreateFontIndirect((PLOGFONT)&CurTheme->TitleFont);
	hfontOld = SelectObject(CurView->hDC, hfont); 

	lpText = Title;	
	y = CurTheme->TitleBox.top + 1;
    while (NextLine (&lpText,lpLine))
    {
		TextExtent = GetTextExtent (CurView->hDC,lpLine,_fstrlen(lpLine));
		width = LOWORD (TextExtent);
		x = CurTheme->TitleBox.left + ((CurTheme->TitleBox.right - CurTheme->TitleBox.left) - width)/2;
		TextOut(CurView->hDC, x, y, lpLine,_fstrlen(lpLine));  
		y+= HIWORD (TextExtent);
	}

	CurTheme->DisplayScatterDiagram=TRUE;
	ThemeDisplayScatterDiagram();

	/* Display the color boxes */
/*	if (CurTheme->DisplayScatterDiagram)
		inc = 0; 
	else
		inc = ymargin / 2;
	ClassColorBox.left = CurTheme->ScatterBox.right+2;
	w = (((long)CurTheme->InfoBox.right - CurTheme->InfoBox.left) * CurTheme->ColorsWidth) / 100;
	ClassColorBox.right = ClassColorBox.left + w;
	ClassColorBox.bottom = CurTheme->ScatterBox.bottom-inc;
	h = CurTheme->ScatterBox.bottom - CurTheme->ScatterBox.top + 1;
	for (iclass=0;iclass<CurTheme->NumClass;iclass++)
	{
		ClassColorBox.top = inc + CurTheme->ScatterBox.bottom - ((iclass+1) * h) / CurTheme->NumClass;
		BkBrush=CreateSolidBrush(CurTheme->ClassColor[iclass]);
		FillRect (CurView->hDC,&ClassColorBox,BkBrush);
		DeleteObject (BkBrush);
		CurTheme->ClassClrBox[iclass] = ClassColorBox;
        ClassColorBox.bottom=ClassColorBox.top - inc;

	}  */

	fHeight = 14;
	x =  CurTheme->ClassClrBox[0].right + xmargin;
	width = CurTheme->InfoBox.right - CurTheme->ClassClrBox[0].right - xmargin*2;
	MaxTextWidth = INT_MAX;
	hfont = NULL;
	while (MaxTextWidth > width)
	{   if (fHeight <= MinFontHeight) goto TooSmall;
		fHeight--;
		SelectObject(CurView->hDC, hfontOld);
		if (hfont) DeleteObject(hfont);

		MaxTextWidth = 0;
		CurTheme->ClassFont1.lfHeight = -MulDiv(fHeight,GetDeviceCaps(CurView->hDC, LOGPIXELSY), 72);
		hfont = CreateFontIndirect((PLOGFONT)&CurTheme->ClassFont1);
		hfontOld = SelectObject(CurView->hDC, hfont);

	    TotCount = 0;
		for (iclass=0;iclass<CurTheme->NumClass;iclass++)
		{   TotCount += CurTheme->ClassCount[iclass];
	       	_fstrcpy (Val1,ValueConv ((double)CurTheme->ClassMin[iclass],CurTheme->ValConv,CurTheme->RoundTo,CurTheme->AddCommas));
	       	_fstrcpy (Val2,ValueConv ((double)CurTheme->ClassMax[iclass],CurTheme->ValConv,CurTheme->RoundTo,CurTheme->AddCommas));

	       	wsprintf (Text,"%s to %s",Val1, Val2);
           	TextExtent = GetTextExtent (CurView->hDC,Text,_fstrlen(Text));
           	MaxTextWidth = max (MaxTextWidth,LOWORD (TextExtent));
		}
    }

TooSmall:	fHeight -=1;
	CurTheme->ClassFont1.lfHeight = -MulDiv(fHeight,GetDeviceCaps(CurView->hDC, LOGPIXELSY), 72);
	hfont2 = CreateFontIndirect((PLOGFONT)&CurTheme->ClassFont1);
	for (iclass=0;iclass<CurTheme->NumClass;iclass++)
	{
       	y = CurTheme->ClassClrBox[iclass].top+ymargin;
       	_fstrcpy (Val1,ValueConv ((double)CurTheme->ClassMin[iclass],CurTheme->ValConv,CurTheme->RoundTo,CurTheme->AddCommas));
       	_fstrcpy (Val2,ValueConv ((double)CurTheme->ClassMax[iclass],CurTheme->ValConv,CurTheme->RoundTo,CurTheme->AddCommas));

        SelectObject(CurView->hDC, hfont);
       	wsprintf (Text,"%s to %s",Val1, Val2);
		TextOut(CurView->hDC, x, y, Text, _fstrlen(Text)); 
		if (TotCount)
		{
			Pct = (100.0 * CurTheme->ClassCount[iclass]) / TotCount; 
			if (CurTheme->PCTByArea)
				sprintf (Text,"(%5.1f%%)",Pct); 
			else
				sprintf (Text,"%ld (%5.1f%%)",CurTheme->ClassCount[iclass],Pct); 
		}
        SelectObject(CurView->hDC, hfont2);
       	TextExtent = GetTextExtent (CurView->hDC,Text,_fstrlen(Text));
		TextOut(CurView->hDC, x+MaxTextWidth/2-LOWORD(TextExtent)/2,
							  y+abs(CurTheme->ClassFont1.lfHeight)*2, Text, _fstrlen(Text));

	}

	SelectObject(CurView->hDC, hfontOld);
	DeleteObject(hfont);
	DeleteObject(hfont2);


	return;
}
BOOL FAR PASCAL TV_THEME1MsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{	HWND	hCheckBox;
	RECT	rect;
	HDC		hDC;
	char	str[256], names[84], *ptr;
HANDLE NEW_HANDLE;
LPSTR lpSTRING;
static  FIELDINFO FIELD;
static	LPFIELDINFO lpmess = &FIELD;
//lda addition
static	LPFIELDINFO lpFieldInfo = &FIELD;
HENV henv;
HDBC hdbc;
SWORD iptr, outlen, deslen;
UCHAR namel[256];
SDWORD namelen;
RETCODE rc;
int IRC, dlgitem;
LPINT irc = &IRC;  
char szDescription[65];
static int num_tables, i, NumFieldNames, LastTableChoice=-2,
                          LastDBChoice=-2, LastFieldChoice=-2;
static BOOL ValidTable = FALSE;     
static LPVIEWPORT SaveView;
// end of lda addition
//FIELDINFO FAR *LPFIELDINFO ;
	long	icount;
	char index[] = "refno", keydata[] = "    -97492987" ;
	LPVOID LPIndex = &index, LPKeydata = &keydata;
	char ANSWER[64];
	char *answer = ANSWER;
	int	Choice;

 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
    	SaveView = CurView;
	    NumFieldNames = 0;									       
   		SetDlgItemText(hWndDlg,SV_CONTENTS_LIST,CurTheme->Contents);
       	SendDlgItemMessage (hWndDlg,SV_CB_ZEROASMISS,BM_SETCHECK,CurTheme->ZeroIsMissing,0L);
        sprintf (str,"%lf",CurTheme->XLimit);
        SetDlgItemText (hWndDlg,IDC_XMAX,str);
        sprintf (str,"%lf",CurTheme->YLimit);
        SetDlgItemText (hWndDlg,IDC_YMAX,str);
    	 SetDlgItemText(hWndDlg,SV_TITLE,(LPSTR)&CurTheme->Title);

LoadFields:
	     NumFieldNames = 0;									       
		 if (CurTheme->DataFileType == MSACCESS_DATAFILE ||
		     CurTheme->DataFileType == ODBC_DATAFILE)
		 {
		   	ShowWindow (GetDlgItem(hWndDlg,SV_TABLE_HEADING),SW_SHOW);
		   	ShowWindow (GetDlgItem(hWndDlg,SV_TABLE_NAMES),SW_SHOW);
         }
         else
		 {
		   	ShowWindow (GetDlgItem(hWndDlg,SV_TABLE_HEADING),SW_HIDE);
		   	ShowWindow (GetDlgItem(hWndDlg,SV_TABLE_NAMES),SW_HIDE);
         }
		 if (CurTheme->DataFile[0])
       	 {
       	  	SetDlgItemText(hWndDlg,SV_DATABASE_LIST,CurTheme->DataFile);
       	  	_fstrcpy (ThemeDB,CurTheme->DataFile);
	        i=SendDlgItemMessage (hWndDlg,SV_FIELD_NAME1,CB_RESETCONTENT,NULL,NULL);
	        i=SendDlgItemMessage (hWndDlg,SV_FIELD_NAME2,CB_RESETCONTENT,NULL,NULL);
	        if (!OpenThemeDataFile ()) break;
       	 	// CurTheme is not static, rather than me change your code I opted to store it in ct. lda
            // I use it for -> case SV_CONTENTS_ICON: and again for ->  case IDCANCEL:
            if(NumFieldNames == 0)
            {
              lpFieldInfo = GetFieldInfo (CurTheme->hThemeDB, TRUE,CurTheme->DataFileType);
              icount = 0;
              while (lpFieldInfo)
              {  //lda addition or should I say change
				i=SendDlgItemMessage (hWndDlg,SV_FIELD_NAME1,CB_ADDSTRING,NULL,(LPARAM)((LPSTR) lpFieldInfo->name));
				i=SendDlgItemMessage (hWndDlg,SV_FIELD_NAME2,CB_ADDSTRING,NULL,(LPARAM)((LPSTR) lpFieldInfo->name));
	            lpFieldInfo = GetFieldInfo (CurTheme->hThemeDB, FALSE,CurTheme->DataFileType);
	            NumFieldNames++;
              }  // end lda change
            }
            SetDlgItemText(hWndDlg,SV_FIELD_NAME1,(LPCSTR)&CurTheme->Field[0].name); 
            SetDlgItemText(hWndDlg,SV_FIELD_NAME2,(LPCSTR)&CurTheme->Field[1].name); 

         }
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           {
            case IDC_THEME_HELP:
              //  WinHelp(hWndDlg,"gwizhelp\\gwizhelp.hlp",HELP_CONTEXT,IDD_SV_THEME);
              //  WinHelp(hWndDlg,"gwizhelp\\gwizhelp.hlp",HELP_CONTEXTPOPUP,IDD_SV_THEME);
              WinHelp(hWndDlg,"gwizhelp\\gwizhelp.hlp",HELP_PARTIALKEY,(DWORD)"Theme Editing");
              //  WinHelp(hWndDlg,"gwizhelp\\gwizhelp.hlp",HELP_KEY,(DWORD)"Theme Editing");
              //  WinHelp(hWndDlg,"gwizhelp\\gwizhelp.hlp",HELP_MULTIKEY,(DWORD)"Theme Editing");
               break;
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 CloseThemeDataFile(TRUE);
                 EndDialog(hWndDlg, FALSE);
                 break;
            case SV_CONTENTS_LIST:
               { 
              	switch(HIWORD(lParam))
                {
	                 case CBN_DROPDOWN:
	                 {
	                 	int idesc;
	                 	 
			            SendDlgItemMessage (hWndDlg,SV_CONTENTS_LIST,CB_RESETCONTENT,NULL,NULL);  
			            SetViewport (CurTheme->TargetViewport);
				    	for (idesc=1;idesc<3201;idesc++) 
				    	{
							if (CurView->CurVisType[idesc])
							{   char	SymbolName[34];
							
								GetSymbolName (idesc,SymbolName,NULL,FALSE,NULL);
			 	                SendDlgItemMessage (hWndDlg,SV_CONTENTS_LIST,CB_ADDSTRING,NULL,(LPARAM)((LPSTR) SymbolName));
		 	                }
						} 
					}
				    break; 
				}
     	      /*  OpenThemeDataFile ();
                 _fstrcpy(lpmess->name,"refno");
                GetExternalReport (CurTheme->hThemeDB,  LPIndex, LPKeydata, lpmess, "form1");
                 CloseThemeDatafile(TRUE); */
                //_fstrcpy (lpmess->name,"yrblt"); //yrblt is one of the fields in m.dbf
               /* _fstrcpy (lpmess->name,"e_mrkt_val"); //yrblt is one of the fields in m.dbf
                FIELD.type = WM_CHARS; //character
                _fstrcpy((char *)answer, GetExternalFieldData (CurTheme->hThemeDB ,
                                       LPIndex, LPKeydata, lpmess, TRUE,irc));
                icount = atol(answer);
                ltoa(icount,answer,10);// to get rid of the leading zeros
           	    SetDlgItemText(hWndDlg,SV_answer, answer); */
                break;
               }
            
            case SV_SET_FILE:
                 Choice=SendDlgItemMessage(hWndDlg,SV_DATABASE_LIST, CB_GETCURSEL,NULL,NULL);
		         if(Choice == LastDBChoice && Choice != 0)break;
                 LastDBChoice = Choice;
			     NumFieldNames = 0;									       
	             i=SendDlgItemMessage (hWndDlg,SV_TABLE_NAMES,CB_RESETCONTENT,NULL,NULL);
	             i=SendDlgItemMessage (hWndDlg,SV_FIELD_NAME1,CB_RESETCONTENT,NULL,NULL);
	             i=SendDlgItemMessage (hWndDlg,SV_FIELD_NAME2,CB_RESETCONTENT,NULL,NULL);
	             LastTableChoice = -1;
	             LastFieldChoice = -1;
                 CloseThemeDataFile(TRUE);
                 if(Choice == 0)
             	 {  
             	 	char	AttDir[256];
             	 	
				   	ShowWindow (GetDlgItem(hWndDlg,SV_TABLE_HEADING),SW_HIDE);
				   	ShowWindow (GetDlgItem(hWndDlg,SV_TABLE_NAMES),SW_HIDE);
             	    SetFilterString (IDS_FILTERDB);
	           	    _getcwd (CurDir,128);
           	  	    SaveDrive = _getdrive();
           	  	    _fstrcpy (AttDir,"[%DATA_LOC]attribut");
           	  	    ExpandText (AttDir); 
           	  	    
                    if (GetOpenFileCD (hWndMain,CurTheme->DataFile,AttDir))
                    {
	             	 i=SendDlgItemMessage (hWndDlg,SV_DATABASE_LIST,CB_RESETCONTENT,NULL,NULL);
           	    	 SetDlgItemText(hWndDlg,SV_DATABASE_LIST,CurTheme->DataFile);
           	    	 if (_fstrstr (CurTheme->DataFile,".GMD"))
           	    	 	CurTheme->DataFileType = UMIFS_DATAFILE;
			   	     _chdir (CurDir);
           	  	     _chdrive (SaveDrive);
           	    	 goto LoadFields;
                    }
			   	    _chdir (CurDir);
           	  	    _chdrive (SaveDrive);
                 }
                 else
                 { // user picked a ODBC Driver
				   	ShowWindow (GetDlgItem(hWndDlg,SV_TABLE_HEADING),SW_SHOW);
				   	ShowWindow (GetDlgItem(hWndDlg,SV_TABLE_NAMES),SW_SHOW);
		           _fstrcpy(CurTheme->DataFile, "ODBC|");
		           SendDlgItemMessage(hWndDlg,SV_DATABASE_LIST,CB_GETLBTEXT, 
		         		  	     	    Choice,(LPARAM)((LPSTR)&CurTheme->DataFile[5]));
           	       SetDlgItemText(hWndDlg,SV_DATABASE_LIST,CurTheme->DataFile);
                   CurTheme->DataFileType = ODBC_DATAFILE;
	               OpenThemeDataFile (); 
          	        //goto s44;//LoadFields;  
          	     }
              break;
                 
            case SV_DATABASE_LIST:
              switch(HIWORD(lParam))
              {
              case CBN_SELCHANGE:
              case CBN_DBLCLK: 
		         PostMessage(hWndDlg, WM_COMMAND, SV_SET_FILE, 0L);
              break;
              case CBN_DROPDOWN:
               i=SendDlgItemMessage (hWndDlg,SV_DATABASE_LIST,CB_RESETCONTENT,NULL,NULL);
    		   i=SendDlgItemMessage (hWndDlg,SV_DATABASE_LIST,CB_ADDSTRING,NULL,
    		                            (LPARAM)((LPSTR) "*.GMD"));
           /*    rc = GetDataSource(CurTheme->hThemeDB, TRUE,
                    names, 64, &outlen, szDescription, (SWORD) 64, &deslen); 
                 while (rc != SQL_NO_DATA_FOUND)
                 { 
                  strupr(names);
   		          i=SendDlgItemMessage (hWndDlg,SV_DATABASE_LIST,CB_ADDSTRING,NULL,
    		                            (LPARAM)((LPSTR) names));
                  rc = GetDataSource( CurTheme->hThemeDB, FALSE,  
                       names, 64, &outlen, szDescription, (SWORD) 64, &deslen); 
                 } */
              } //end of the switch   
              break; 

            case SV_TABLE_NAMES: // added by lda 
              i = HIWORD(lParam);
              switch(HIWORD(lParam))
              {
               case CBN_DBLCLK:
               case CBN_SELCHANGE:
                 if(!CurTheme->hThemeDB || CurTheme->DataFileType != ODBC_DATAFILE) break; 
                 Choice=SendDlgItemMessage(hWndDlg,SV_TABLE_NAMES, CB_GETCURSEL,NULL,NULL);
                 if(Choice == LastTableChoice) break;
			     LastTableChoice = Choice;
                  LastFieldChoice = -1;
	              i=SendDlgItemMessage (hWndDlg,SV_FIELD_NAME1,CB_RESETCONTENT,NULL,NULL);
	              i=SendDlgItemMessage (hWndDlg,SV_FIELD_NAME2,CB_RESETCONTENT,NULL,NULL);
				   ValidTable = TRUE;					        
		           SendDlgItemMessage(hWndDlg,SV_TABLE_NAMES,CB_GETLBTEXT,
		         		  		    Choice,(DWORD)&CurTheme->Field[0].name); 
		           _fstrcat(CurTheme->DataFile,"|");
		           _fstrcat(CurTheme->DataFile,(LPCSTR)&CurTheme->Field[0].name);		  		    
		  		    
	               NEW_HANDLE = OpenDatabaseTable (CurTheme->hThemeDB ,&CurTheme->Field[0].name);// needed only by msaccess        
                   CurTheme->Field[0].name[0]='\0';
		           NumFieldNames = 0;									       
                   goto LoadFields; 
                case CBN_DROPDOWN:
	               if(!CurTheme->hThemeDB) break; 
	               lpSTRING =  GetTableName (CurTheme->hThemeDB, TRUE); // the first table name 
   	               if (lpSTRING == NULL) break;
	               i=SendDlgItemMessage (hWndDlg,SV_TABLE_NAMES,CB_RESETCONTENT,NULL,NULL);
 	               i = 0;
 	               while (lpSTRING && lpSTRING[0] != 0 )
 	               {
 	                i=SendDlgItemMessage (hWndDlg,SV_TABLE_NAMES,CB_ADDSTRING,NULL,(LPARAM)((LPSTR) lpSTRING));
 	                lpSTRING =  GetTableName (CurTheme->hThemeDB, FALSE); // subsequent table names 
	               } 
                     
               } //end of the switch
                break; 
  
            case SV_FIELD_NAME1:
              switch(HIWORD(lParam))
              {
               case CBN_DBLCLK:
               case CBN_SELCHANGE:
            
                if(CurTheme->DataFileType == ODBC_DATAFILE && !ValidTable) break; 
                if(NumFieldNames > 0)
                {
                  Choice=SendDlgItemMessage(hWndDlg,SV_FIELD_NAME1,
									       CB_GETCURSEL,NULL,NULL); 
                  if( Choice == LastFieldChoice)break;
		          LastFieldChoice = Choice;
				  if(Choice >= 0)
			      {					        
		               SendDlgItemMessage(hWndDlg,SV_FIELD_NAME1,CB_GETLBTEXT,
		         		  		    Choice,(DWORD)&CurTheme->Field[0].name); 
		           }
		           break;  
		        }
		      }// end of the switch       		  		    
              break; 
                         
            case SV_FIELD_NAME2:
              switch(HIWORD(lParam))
              {
               case CBN_DBLCLK:
               case CBN_SELCHANGE:
            
                if(CurTheme->DataFileType == ODBC_DATAFILE && !ValidTable) break; 
                if(NumFieldNames > 0)
                {
                  Choice=SendDlgItemMessage(hWndDlg,SV_FIELD_NAME2,
									       CB_GETCURSEL,NULL,NULL); 
                  if( Choice == LastFieldChoice)break;
		          LastFieldChoice = Choice;
				  if(Choice >= 0)
			      {					        
		               SendDlgItemMessage(hWndDlg,SV_FIELD_NAME2,CB_GETLBTEXT,
		         		  		    Choice,(DWORD)&CurTheme->Field[0].name); 
		           }
		           break;  
		        }
		      }// end of the switch       		  		    
              break; 
                         
            case IDC_SAVE_THEME:  
            	SaveCurTheme(hWndDlg);
            	break; 
            	
            case IDOK:
  				 CurTheme->Field[0].type = SQL_VARCHAR;
                 Choice=SendDlgItemMessage(hWndDlg,SV_FIELD_NAME1,
									       CB_GETCURSEL,NULL,NULL);
		         SendDlgItemMessage(hWndDlg,SV_FIELD_NAME1,CB_GETLBTEXT,
		         		  		    Choice,(DWORD)&CurTheme->Field[0].name);
                 Choice=SendDlgItemMessage(hWndDlg,SV_FIELD_NAME2,
									       CB_GETCURSEL,NULL,NULL);
		         SendDlgItemMessage(hWndDlg,SV_FIELD_NAME2,CB_GETLBTEXT,
		         		  		    Choice,(DWORD)&CurTheme->Field[1].name);

       	 		 GetDlgItemText (hWndDlg,SV_TITLE,CurTheme->Title,256);
            	 CurTheme->ZeroIsMissing = SendDlgItemMessage (hWndDlg,SV_CB_ZEROASMISS,BM_GETCHECK,0,0L); 
				 GetDlgItemText (hWndDlg,SV_CONTENTS_LIST,CurTheme->Contents,10);
				 SetViewport (CurTheme->TargetViewport);
				 CurTheme->SymNum=GetSymbolNum (CurTheme->Contents);

		       	 if (SendDlgItemMessage (hWndDlg,SV_CB_EVENRANGES,BM_GETCHECK,0,0L)) CurTheme->ClassType =1;
		       	 if (SendDlgItemMessage (hWndDlg,SV_CB_PERCENTILES,BM_GETCHECK,0,0L)) CurTheme->ClassType =2;
		       	 if (SendDlgItemMessage (hWndDlg,SV_CB_MANUAL,BM_GETCHECK,0,0L)) CurTheme->ClassType =3;
		       	 if (CurTheme->ClassType!=1) CurTheme->DisplayScatterDiagram = FALSE;
		       	 GetDlgItemText (hWndDlg,SV_NUM_CLASSES,str,3);
		       	 CurTheme->NumDesiredClass = atoi (str);
		       	 GetDlgItemText (hWndDlg,IDC_XMAX,str,16);
		       	 if (str[0]) CurTheme->XLimit = atof (str); 
		       	 GetDlgItemText (hWndDlg,IDC_YMAX,str,16);
		       	 if (str[0]) CurTheme->YLimit = atof (str); 
		       	 CurView = SaveView;
                 CloseThemeDataFile(TRUE);
                 EndDialog(hWndDlg, TRUE);
                 break;

          /*  case SV_DATA_FILE_ICON:
            	 CloseThemeDataFile(TRUE);
            	 SetFilterString (IDS_FILTERDB);

    no longer   if (GetOpenFileCD (hWndDlg,CurTheme->DataFile,"C:"))
    use this    {
           	    	 SetDlgItemText(hWndDlg,SV_DATA_FILE,CurTheme->DataFile);
           	    	 if (_fstrstr (CurTheme->DataFile,".GWD")) CurTheme->DataFileType = UMIFS_DATAFILE;
           	    	 if (_fstrstr (CurTheme->DataFile,".DBF")) CurTheme->DataFileType = FOXPRO_DATAFILE;
           	    	 if (_fstrstr (CurTheme->DataFile,".MDB")) CurTheme->DataFileType = MSACCESS_DATAFILE;
           	    	 goto LoadFields;
                 }  */


           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}
BOOL FAR PASCAL PROMPTSMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 	 
 RECT	MRect, Rect;
 short	iwidth, iheight;  
 POINT	point;
 
 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
   	case WM_ERASEBKGND:
   		return TRUE;
   	case WM_NCPAINT:
   		return TRUE;
    case WM_INITDIALOG:
    	 if (!GetGlobalBVal ("[%PROMPTS]"))
    	 {
	        EndDialog(hWndDlg, FALSE);
    	 	return FALSE;
    	 }
         hWndPrompt = hWndDlg;   
    case GSSI_REINITDIALOG:
		 GetClientRect(hWndMain,&MRect);
		 GetWindowRect(hWndDlg,&Rect);
		 iwidth = MRect.right - MRect.left+1; 
		 iheight = Rect.bottom - Rect.top + 1;
		 point.x = MRect.left;
		 point.y = MRect.bottom-iheight;
		 MoveWindow(hWndDlg, point.x,point.y, iwidth, iheight, FALSE);
         break; /* End of WM_INITDIALOG                                 */
    
    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         EndDialog(hWndDlg, FALSE);
         break; /* End of WM_CLOSE                                      */
    
    case WM_DESTROY:
    	 hWndPrompt = 0;
    	 return 0L;
    	 
    default:
        return FALSE;
   }
 return TRUE;
} 


from dxfin
 int SymbolNum(char *Name)
 {
  LPSYMBOL   pSymDesc;    
  HANDLE hPoly, hShapes, hSymDesc=0, hCFCC, hSym;
  int idesc, i,NumSyms;
  char SymName[16];
                    _fmemmove (SymName,SymName,3);
                    SymName[3]=0;
                    idesc = GetDictSymbolNumber (SymName);
                    if (!idesc)
                        idesc = GetDictSymbolNumber ("X00"); 
                    if (hSymDesc)
                    {
                        pSymDesc =(LPSYMBOL) GlobalLock(hSymDesc);
                        for (i=0;i<NumSyms;i++,pSymDesc++)
                            if (idesc == pSymDesc->Number) goto HaveSym;
                        GlobalUnlock (hSymDesc);
                        NumSyms++;
                        hSymDesc = GlobalReAlloc (hSymDesc,NumSyms*sizeof(SYMBOL),NULL); 
                    }
                    else                                        
                    {
                        NumSyms=1;
                        hSymDesc = GSSiGlobAlloc (GMEM_MOVEABLE,sizeof(SYMBOL));
                    }
                    pSymDesc = (LPSYMBOL)GlobalLock(hSymDesc);
                    pSymDesc += NumSyms-1;
                    hSym = GetDictSymDesc (idesc);
                    GlobalUnlock (hSymDesc); 
HaveSym:;
return idesc;
}


BOOL FAR PASCAL DXFLinesMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{   char string[96];
    RECT    rect;
    HDC     hDC;
        char far * ptr;
        char  GeoChoice[96];
static    BOOL WorkingOnLines, GotIt;
static int NumDxfPicked, NumGeoPicked, oked;
 int    BRtn, got,   nRc=0, i, choice,j;              /* return code                         */
lpDxfToPC    lpDxf2GM; 
LPLAYERDESC	pLayer;
 
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG: 
    { 
        DWORD   WVer;
        int     WinVer, got;
        char	str[256];
        
         GetWindowRect(hWndDlg, &rect); //GetDesktopWindow(), &rect);
          WVer = GetVersion ();
          Canceled = FALSE;
         WinVer = HIBYTE(LOWORD(WVer));
         SymType = 2;
         DefaultPrefix[0] = '\0';
         SendDlgItemMessage (hWndDlg,IDC_LAYER_LIST,LB_RESETCONTENT,NULL,NULL);
         pLayer = GlobalLock (hDXFLayers);
         for (i=0;i<NumDXFLayers;i++,pLayer+=sizeof(LAYERDESC))
         {    
         	  sprintf (str,"%s\t%s",pLayer->Name,pLayer->Command);
              SendDlgItemMessage (hWndDlg,IDC_LAYER_LIST,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)str));
         }
         NumDXFLayers++;  
		 GlobalUnlock (hDXFLayers);      
         WorkingOnLines = TRUE;
         oked = 0;
         i = SendDlgItemMessage (hWndDlg,IDC_GEOMASTER_SYMBOLS,LB_RESETCONTENT,0,0L);
         i = SendDlgItemMessage (hWndDlg,IDC_DXF_SELECTED, LB_RESETCONTENT,0,0L);
         i = SendDlgItemMessage (hWndDlg,IDC_GEO_SYMBOL, LB_RESETCONTENT,0,0L);
                  NumDxfPicked = 0;
                  NumGeoPicked = 0;
    }
         break; /* End of WM_INITDIALOG                                 */

 /*  case WM_PAINT:
         memset(&ps, 0x00, sizeof(PAINTSTRUCT));
         hDC = BeginPaint(hWndDlg, &ps);
         DisplayBMInRect (hDC,FullBM,ps.rcPaint,TRUE);
         EndPaint(hWndDlg, &ps);
         break;   */    /*  End of WM_PAINT                               */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */
    case WM_MEASUREITEM:// message when the list box is created and a 
         break;
    case WM_DRAWITEM:// message when the view changed
         break;
    case WM_COMMAND:
         switch(wParam)
           {
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 Canceled = TRUE;
                 EndDialog(hWndDlg, FALSE);
                 break;
            case IDC_DXF_ASSIGN_BUTTON:
                  EnableWindow(GetDlgItem(hWndDlg, IDC_DXF_ASSIGN_BUTTON),FALSE);

                  NumDxfPicked = SendDlgItemMessage (hWndDlg,IDC_DXF_SELECTED,
                     LB_GETCOUNT,0,0L);
               j=   SendDlgItemMessage(hWndDlg,IDC_GEO_SYMBOL,LB_GETTEXT,
                                    0,(LPARAM)GeoChoice);
                ptr = _fstrchr(GeoChoice,':');
                ptr--;  *ptr = '\0';                                    
            switch (SymType)
            {     
              case 2:
                  lpLT = (lplType) GlobalLock(LineMem);
                  lpStart = lpLT;
                  for(i = 0;i < NumDxfPicked;i++)
                  {
                     SendDlgItemMessage(hWndDlg,IDC_DXF_SELECTED,LB_GETTEXT,
                                    i,(LPARAM)string);
                     ptr = _fstrchr(string,':');
                     ptr--;
                     *ptr = '\0';
                     for(j = 0;j < NumLines;j++,lpLT++)
                     {
                       if(_fstricmp(lpLT->Name,string) == 0)
                       { 
                         _fstrset(GeoHashString,'\0');
                         _fstrcpy(GeoHashString,string);
                         _fstrcpy(&(GeoHashString[22]),GeoChoice); 
                         //next instruction should really load the
                         //GeoSymNum when it becomes available
                         _fmemcpy(&(GeoHashString[42]), &j,2);
                         _fmemcpy(&(GeoHashString[44]), &SymType,2);
                         
                         HASHP(Hid,GeoHashString,&HashLoc); 
                         _fstrcpy(lpLT->GeoSymbol,GeoChoice);
                         
                         
                         break;
                       }
                     }//end of inner for loop
                     lpLT = lpStart;
                  }//end of the first for loop
                  GlobalUnlock(LineMem);
                  NumDxfPicked = SendDlgItemMessage (hWndDlg,IDC_DXF_SYMBOLS,
                                 LB_GETCOUNT,0,0L);
                  if(NumDxfPicked == 0)
                  {//we are done here
                     oked++;
                     SymType = 1;//points
                     WorkingOnLines = FALSE;
                     //HERE I LOAD THE POINT DESCRIPTIONS
              //      j=SendDlgItemMessage (hWndDlg,IDC_GEOMASTER_SYMBOLS,
              //         LB_RESETCONTENT,0,0L);
                    j=SendDlgItemMessage (hWndDlg,IDC_GEOMASTER_SYMBOLS,
                       LB_ADDSTRING,0,(LPARAM)"Dot : .");
                    j=SendDlgItemMessage (hWndDlg,IDC_GEOMASTER_SYMBOLS,
                       LB_ADDSTRING,0,(LPARAM)"Solid Square : []");
                    j=SendDlgItemMessage (hWndDlg,IDC_GEOMASTER_SYMBOLS,
                       LB_ADDSTRING,0,(LPARAM)" Circle : O");
                    j=SendDlgItemMessage (hWndDlg,IDC_GEOMASTER_SYMBOLS,          
                       LB_ADDSTRING,0,(LPARAM)"Dash : -");
                    j=SendDlgItemMessage (hWndDlg,IDC_GEOMASTER_SYMBOLS,
                       LB_ADDSTRING,0,(LPARAM)"Cross : X");
                    j=SendDlgItemMessage (hWndDlg,IDC_GEOMASTER_SYMBOLS,
                       LB_ADDSTRING,0,(LPARAM)"Money : $");
                    j=SendDlgItemMessage (hWndDlg,IDC_GEOMASTER_SYMBOLS,
                       LB_ADDSTRING,0,(LPARAM)"Right Arrow : ->");
                    j=SendDlgItemMessage (hWndDlg,IDC_GEOMASTER_SYMBOLS,
                       LB_ADDSTRING,0,(LPARAM)"Left Arrow : <-");
                   EnableWindow(GetDlgItem(hWndDlg, IDC_GEOMASTER_SYMBOLS), FALSE);

                    DXFOUT = GSSiOpenFile (DXFOutFile,lpOUTF,OF_READ|OF_SEARCH);
                    if(DXFOUT == HFILE_ERROR)
                    {
                      _fstrcpy(string,"Unable to open ");
                      _fstrcat(string, DXFOutFile);
                      MessageBox(NULL,string,"DXF Importer",MB_ICONSTOP);     
                      EndDialog(hWndDlg, FALSE);
                      break;
                    }
                    if(NumPoints == 0)
                    {// we're done
                       EndDialog(hWndDlg, FALSE);
                      break;
                    }
                    PointMem = GlobalAlloc(GHND,sizeof(PtDesc)*(NumPoints +5));
                    lpPointDesc = (lpPtDesc)GlobalLock(PointMem);
                     fgetstring(string,64,DXFOUT);
                    lpPointDesc->NumPts = NumPoints;
                    for(i=0;i<NumPoints;i++,lpPointDesc++)
                    {
                      j= SendDlgItemMessage (hWndDlg,IDC_DXF_SYMBOLS,
                           LB_ADDSTRING,0,(LPARAM)string);
                       _fstrcpy(lpPointDesc->Name,string);
                       fgetstring(string,64,DXFOUT);
                    }
                    GSSiClose(DXFOUT);  
                    GlobalUnlock(PointMem);
                    WorkingOnLines = FALSE;
                    for(i=0;i<NumAttDefs;i++)
                    {
                      j= SendDlgItemMessage (hWndDlg,IDC_DXF_SYMBOLS,
                           LB_ADDSTRING,0,(LPARAM)ATTDEFS[i]);
                    }
 
                  }
                  SendDlgItemMessage (hWndDlg,IDC_DXF_MESSAGE_BOX, WM_SETTEXT,NULL,
                     (LPARAM)"Please continue selecting DXF symbols and a corresponding\n\
                     GeoMaster symbol.");
                 break;    //if WorkingOnLines
             case 1:
              //working with points
                 SymType = 1;//points
                 lpPointDesc = (lpPtDesc)GlobalLock(PointMem);
                 lpPtStart = lpPointDesc;
                 GotIt = FALSE;
                 for(i = 0;i < NumDxfPicked;i++)
                 {
                     SendDlgItemMessage(hWndDlg,IDC_DXF_SELECTED,LB_GETTEXT,
                                    i,(LPARAM)string);
                     for(j = 0;j < NumPoints;j++,lpPointDesc++)
                     {
                       if(_fstricmp(lpPointDesc->Name,string) == 0)
                       {
                         _fstrset(GeoHashString,'\0');
                         _fstrcpy(GeoHashString,string);
                         _fstrcpy(&(GeoHashString[22]),GeoChoice); 
                         //next instruction should really load the
                         //GeoSymNum when it becomes available
                         _fmemcpy(&(GeoHashString[42]), &j,2);
                         _fmemcpy(&(GeoHashString[44]), &SymType,2);
                         HASHP(Hid,GeoHashString,&HashLoc); 
                         _fstrcpy(lpPointDesc->GeoSymbol,GeoChoice);
                         GotIt = TRUE;
                         break;
                       }
                     }//end of inner for loop
                     lpPointDesc = lpPtStart;
                     if(!GotIt)
                     {
                       for(j = 0; j < NumAttDefs;j++)
                       {
                         if(_fstricmp(string,ATTDEFS[j]) == 0)
                         {
                           _fstrset(GeoHashString,'\0');
                           _fstrcpy(GeoHashString,string);
                           _fstrcpy(&(GeoHashString[22]),GeoChoice); 
                         //next instruction should really load the
                         //GeoSymNum when it becomes available
                           _fmemcpy(&(GeoHashString[42]), &j,2);
                           _fmemcpy(&(GeoHashString[44]), &SymType,2);
                           HASHP(Hid,GeoHashString,&HashLoc); 
                         }
                       }
                       
                     
                     }
                  }//end of the first for loop
                  GlobalUnlock(PointMem);
                  NumDxfPicked = SendDlgItemMessage (hWndDlg,IDC_DXF_SYMBOLS,
                                 LB_GETCOUNT,0,0L);
Victor7:          if(NumDxfPicked == 0 && NumAttDefs == 0)
                  {
                       EndDialog(hWndDlg, FALSE);
                       break;
                  }
                  else if (NumDxfPicked == 0 && NumAttDefs > 0)
                  {//want the user to supply the default db fieldname
                    oked++;
                    SymType = 4;
                    SendDlgItemMessage (hWndDlg,IDC_GEOMASTER_SYMBOLS, LB_RESETCONTENT,NULL,NULL);
                    SendDlgItemMessage (hWndDlg,IDC_DXF_SYMBOLS,LB_RESETCONTENT,NULL,NULL);
                    SendDlgItemMessage (hWndDlg,IDC_DXF_MESSAGE_BOX, WM_SETTEXT,NULL,
                     (LPARAM)"Please select the database datetime stamp field name.");
                     
                    for(i = 0;i < NumAttDefs;i++)
                    {
                    j=SendDlgItemMessage (hWndDlg,IDC_DXF_SYMBOLS,
                       LB_ADDSTRING,0,(LPARAM)&(ATTDEFS[i][0]));
                    }                  
                    SendDlgItemMessage (hWndDlg,IDC_DXF_MESSAGE_BOX, WM_SETTEXT,NULL,
                       (LPARAM)"Lastly, select the name of the datatime stamp field.");
                  }
                  else
                    SendDlgItemMessage (hWndDlg,IDC_DXF_MESSAGE_BOX, WM_SETTEXT,NULL,
                     (LPARAM)"Please continue selecting DXF symbols and a corresponding\n\
                     GeoMaster symbol.");
            }//end of the switch        
                  SendDlgItemMessage (hWndDlg,IDC_GEO_SYMBOL, LB_RESETCONTENT,NULL,NULL);
                  SendDlgItemMessage (hWndDlg,IDC_DXF_SELECTED,LB_RESETCONTENT,NULL,NULL);
                  NumDxfPicked = 0;
                  NumGeoPicked = 0;
                 break;        
            case IDC_DXF_SELECTED:

                switch(HIWORD(lParam))
                {            
                  case LBN_DBLCLK:
                     i=SendDlgItemMessage(hWndDlg,IDC_DXF_SELECTED, 
                               LB_GETCURSEL,0,0L);
                     SendDlgItemMessage(hWndDlg,IDC_DXF_SELECTED,LB_GETTEXT,
                                    i,(LPARAM)string);
                     SendDlgItemMessage (hWndDlg,IDC_DXF_SYMBOLS,
                                  LB_ADDSTRING,NULL,(LPARAM)string);
                     SendDlgItemMessage (hWndDlg,IDC_DXF_SELECTED,
                           LB_DELETESTRING,i,0);
                     NumDxfPicked = GetDlgItem(hWndDlg, IDC_DXF_SYMBOLS);
                    if(NumDxfPicked == 0)
                    {
                       EnableWindow(GetDlgItem(hWndDlg, IDC_GEOMASTER_SYMBOLS),FALSE);
                       SendDlgItemMessage (hWndDlg,IDC_DXF_MESSAGE_BOX,WM_SETTEXT,NULL,
                     (LPARAM)"1. Select one or more DXF symbols.\n\
                     2. Select the GeoMaster symbol to display.\n\
                     3. Pick the Assign Symbol button.");

                    }    
                 }
                 break;                                                           
            case IDC_GEO_SYMBOL:
                switch(HIWORD(lParam))
                {            
                  case LBN_DBLCLK:
                     SendDlgItemMessage (hWndDlg,IDC_GEO_SYMBOL, LB_RESETCONTENT,NULL,NULL);
                     EnableWindow(GetDlgItem(hWndDlg, IDC_DXF_ASSIGN_BUTTON),FALSE);
                     NumGeoPicked = 0;
                }
                break;
                  
             case IDC_DXF_SYMBOLS:
               if(SymType == 4)
               {
                     SendDlgItemMessage(hWndDlg,IDC_DXF_SELECTED,LB_GETTEXT,
                                    0,(LPARAM)DefaultPrefix);
                     EndDialog(hWndDlg, FALSE);
               }  
                switch(HIWORD(lParam))
                {
                 case LBN_DBLCLK:   
                 case LBN_SELCHANGE:
                    SendDlgItemMessage (hWndDlg,IDC_DXF_MESSAGE_BOX, WM_SETTEXT,NULL,
                     (LPARAM)"When you finish selecting DXF symbols, Select the\n\
                     GeoMaster symbol.");
                     NumDxfPicked = SendDlgItemMessage (hWndDlg,IDC_DXF_SYMBOLS,
                     LB_GETCOUNT,0,0L);

                     i=SendDlgItemMessage(hWndDlg,IDC_DXF_SYMBOLS, 
                                      LB_GETCURSEL,0,0L);
                     SendDlgItemMessage(hWndDlg,IDC_DXF_SYMBOLS,LB_GETTEXT,
                                    i,(LPARAM)string);
                     SendDlgItemMessage (hWndDlg,IDC_DXF_SELECTED,
                                  LB_ADDSTRING,NULL,(LPARAM)string);
                     SendDlgItemMessage (hWndDlg,IDC_DXF_SYMBOLS,LB_FINDSTRING,NULL,(LPARAM)string);
                     SendDlgItemMessage (hWndDlg,IDC_DXF_SYMBOLS,LB_DELETESTRING,i,0);
                     EnableWindow(GetDlgItem(hWndDlg, IDC_GEOMASTER_SYMBOLS),
                     (NumDxfPicked == 0 ? FALSE : TRUE));
                     if(NumDxfPicked == 0)
                       SendDlgItemMessage (hWndDlg,IDC_DXF_MESSAGE_BOX,WM_SETTEXT,NULL,
                     (LPARAM)"1. Select one or more DXF symbols.\n\
                     2. Select the GeoMaster symbol to display.\n\
                     3. Pick the Assign Symbol button.");
                   break;
                   case LBN_SELCANCEL: 
                     NumDxfPicked = SendDlgItemMessage (hWndDlg,IDC_DXF_SYMBOLS,
                     LB_GETCOUNT,0,0L);
                }    
                 break; 

            case IDC_GEOMASTER_SYMBOLS:
                switch(HIWORD(lParam))
                {
                 case LBN_DBLCLK:   
                 case LBN_SELCHANGE:
                     SendDlgItemMessage (hWndDlg,IDC_DXF_MESSAGE_BOX, WM_SETTEXT,NULL,
                     (LPARAM)"You may now pick the Assign Symbol button.");
                     NumGeoPicked = GetDlgItem(hWndDlg, IDC_GEOMASTER_SYMBOLS);
                     if(NumDxfPicked != 0 && NumGeoPicked != 0)
                       EnableWindow(GetDlgItem(hWndDlg, IDC_DXF_ASSIGN_BUTTON),TRUE);
                     else
                       EnableWindow(GetDlgItem(hWndDlg, IDC_DXF_ASSIGN_BUTTON),FALSE);
                     i=SendDlgItemMessage(hWndDlg,IDC_GEOMASTER_SYMBOLS, 
                                        LB_GETCURSEL,0,0L);
                     SendDlgItemMessage(hWndDlg,IDC_GEOMASTER_SYMBOLS,LB_GETTEXT,
                                    i,(LPARAM)string);
                     SendDlgItemMessage (hWndDlg,IDC_GEO_SYMBOL,
                                        LB_RESETCONTENT,NULL,NULL);
                     SendDlgItemMessage (hWndDlg,IDC_GEO_SYMBOL,
                                        LB_ADDSTRING,NULL,(LPARAM)((LPSTR)string));
                    break; 
                  case LBN_SELCANCEL: 
                        NumGeoPicked == 0;
                        SendDlgItemMessage (hWndDlg,IDC_DXF_MESSAGE_BOX,
                        WM_SETTEXT,NULL,
                        (LPARAM)"When you finish selecting DXF symbols, Select the\n\
                        GeoMaster symbol.");
                        SendDlgItemMessage (hWndDlg,IDC_GEO_SYMBOL,
                                           LB_RESETCONTENT,NULL,NULL);
                        EnableWindow(GetDlgItem(hWndDlg, IDC_DXF_ASSIGN_BUTTON),FALSE);
                  }
                  break;
            case IDOK:
                  if(oked >= 2)
                  {
                    if(DefaultPrefix[0] != '\0')
                    {
                      EndDialog(hWndDlg, TRUE);
                      break;
                    }
                    else
                      goto Victor7;
                  }  
                    oked++;
                    
                    //HERE I LOAD THE POINT DESCRIPTIONS
                //    j=SendDlgItemMessage (hWndDlg,IDC_GEOMASTER_SYMBOLS,
                //                    LB_RESETCONTENT,0,0L);
                    j=SendDlgItemMessage (hWndDlg,IDC_DXF_SYMBOLS,
                                    LB_RESETCONTENT,0,0L);
                    j=SendDlgItemMessage (hWndDlg,IDC_DXF_SELECTED,
                                 LB_RESETCONTENT,0,0L);
                    j=SendDlgItemMessage (hWndDlg,IDC_GEO_SYMBOL,
                                 LB_RESETCONTENT,0,0L);
                     j=SendDlgItemMessage (hWndDlg,IDC_GEOMASTER_SYMBOLS,
                       LB_ADDSTRING,0,(LPARAM)"Dot : .");
                    j=SendDlgItemMessage (hWndDlg,IDC_GEOMASTER_SYMBOLS,
                       LB_ADDSTRING,0,(LPARAM)"Solid Square : []");
                    j=SendDlgItemMessage (hWndDlg,IDC_GEOMASTER_SYMBOLS,
                       LB_ADDSTRING,0,(LPARAM)"Circle : O");
                    j=SendDlgItemMessage (hWndDlg,IDC_GEOMASTER_SYMBOLS,          
                       LB_ADDSTRING,0,(LPARAM)"Dash : -");
                    j=SendDlgItemMessage (hWndDlg,IDC_GEOMASTER_SYMBOLS,
                       LB_ADDSTRING,0,(LPARAM)"Cross : X");
                    j=SendDlgItemMessage (hWndDlg,IDC_GEOMASTER_SYMBOLS,
                       LB_ADDSTRING,0,(LPARAM)"Money : $");
                    j=SendDlgItemMessage (hWndDlg,IDC_GEOMASTER_SYMBOLS,
                       LB_ADDSTRING,0,(LPARAM)"Right Arrow : ->");
                    j=SendDlgItemMessage (hWndDlg,IDC_GEOMASTER_SYMBOLS,
                       LB_ADDSTRING,0,(LPARAM)"Left Arrow : <-");
                   EnableWindow(GetDlgItem(hWndDlg, IDC_GEOMASTER_SYMBOLS), FALSE);

                    DXFOUT = GSSiOpenFile (DXFOutFile,lpOUTF,OF_READ|OF_SEARCH);
                    if(DXFOUT == HFILE_ERROR)
                    {
                      _fstrcpy(string,"Unable to open ");
                      _fstrcat(string, DXFOutFile);
                      MessageBox(NULL,string,"DXF Importer",MB_ICONSTOP);     
                      EndDialog(hWndDlg, FALSE);
                      break;
                    }
                    if(NumPoints == 0)
                    {// we're done
                      EndDialog(hWndDlg, FALSE);
                      break;
                    }
                    if(PointMem == 0)
                    PointMem = GlobalAlloc(GHND,sizeof(PtDesc)*(NumPoints +5));
                    lpPointDesc = (lpPtDesc)GlobalLock(PointMem);
                     fgetstring(string,64,DXFOUT);
                    lpPointDesc->NumPts = NumPoints;
                    for(i=0;i<NumPoints;i++,lpPointDesc++)
                    {
                      j= SendDlgItemMessage (hWndDlg,IDC_DXF_SYMBOLS,
                           LB_ADDSTRING,0,(LPARAM)string);
                       _fstrcpy(lpPointDesc->Name,string);
                       fgetstring(string,64,DXFOUT);
                    }
                    GSSiClose(DXFOUT);  
                    GlobalUnlock(PointMem);
                    SymType = 1; 
                  NumDxfPicked = 0;
                  NumGeoPicked = 0;
                    break;
      /*      case IDC_CHECKBACK:
                 DxfIn();
                 break;      */
           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} 



{
	CITIESDATA4	CitiesData4; 
	HFILE	Cities4FID,Cities42FID;
	OFSTRUCT	OFStruct;
	
	Cities4FID = GSSiOpenFile ("cities4.dat",&OFStruct,OF_READ); 
	Cities42FID = GSSiOpenFile ("cities42.dat",&OFStruct,OF_CREATE); 
	while (_lread (Cities4FID,&CitiesData4,sizeof(CITIESDATA4)) == sizeof(CITIESDATA4))
	{
		FixCaps (CitiesData4.Name); 
		_lwrite (Cities42FID,&CitiesData4,sizeof(CITIESDATA4));
	}
	_lclose (Cities4FID);
	_lclose (Cities42FID);

} 
 
	_fstrcpy (CfgName,"police.gmc");
	GSSiOpenFile (CfgName,(LPOFSTRUCT)&OFStruct,OF_DELETE);
	FidConfig = GSSiOpenFile (CfgName,(LPOFSTRUCT)&OFStruct,OF_CREATE);
    _lwrite (FidConfig,StartupCommand,sizeof(StartupCommand));
    _lwrite (FidConfig,StartupMenu,sizeof(StartupMenu));
    _lwrite (FidConfig,&NumIB,sizeof(NumIB));  
	NumViewports = 3; 
	CommandViewport = 1;
	WindowColor = RGB(255,255,255);
    _lwrite (FidConfig,&NumViewports,2);
    _lwrite (FidConfig,&CommandViewport,2);
    
    NumMenuMask=0;
    _lwrite (FidConfig,&NumMenuMask,sizeof(NumMenuMask));
    
    _lwrite (FidConfig,&WindowColor,4);
   	_lwrite (FidConfig,&TAGBox,sizeof(TAGBOX));
	hViewports[0]=GSSiGlobAlloc (GHND,sizeof(VIEWPORT));
	if (!hViewports[0]) return(MemError());
	CurView = (LPVIEWPORT)GlobalLock (hViewports[0]);
	
	CurView->ID = 1;
	CurView->Version = Version;
	CurView->Active = TRUE;
	CurView->Parent = 0;
	CurView->Type = 1; 
	CurView->DesiredHeight = 0;
	CurView->DesiredWidth = 0;
	CurView->BackGroundColor = RGB(255,255,255);
	CurView->Shadow = TRUE;
	CurView->TagPointID = 2;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 2;
	CurView->TagPoint.y = 2;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width = 67;
	CurView->Height = 96;
	CurView->Margin = 2;
	CurView->MarginPan = TRUE;
	CurView->Margin = 2;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE; 
    CurView->BoundsDisplayID = 1;
    CurView->lpBoundsDisplay = BoundsDisplayInit (1,2,1);
    CurView->pTheme = 0;
	CurView->NumThemes = 0;
	CurView->NumFiles = 1;
	CurView->FileType[0]=2;
	CurView->lpFiles[0]=0; 
	_fstrcpy (CurView->FileID[0],"City Basemap");
	CurView->NumVisList = 2;
	_fstrcpy (CurView->VisName,"vislists\\[%CONFIG].vis");
	CurView->PickName[0] = '\0';	
	_fstrcpy (CurView->PickMacroFile,"pikmacro.txt");
	
	CurView->StartupFunction=GF_PICK_CRIME;
	_fstrcpy(CurView->FunctionFile,"fundir\\basic2.txt");
	_fstrcpy(CurView->FunctionDir,"index.txt");
	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\polbase.plt");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	if (hVisList)
	{
		GlobalUnlock(hVisList);
		GlobalFree(hVisList);
	}
	hVisList=GSSiGlobAlloc (GHND,sizeof(VISLIST));
	if (!hVisList) return(MemError());
	CurVis = (LPVISLIST)GlobalLock (hVisList);
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->LastVisList=0;
	CurVis->NextVisList=0; 
	CurVis->MinPointSize = 6; 
	CurVis->MaxScale=10.0;
	CurVis->FileIsVisible[0]=FALSE;
	CurVis->FileIsVisible[4]=FALSE;
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST));
	
	InitVis ();
	CurVis->MinScale=10.0;
	CurVis->FileIsVisible[2]=FALSE;
	CurVis->FileIsVisible[3]=FALSE;
	CurVis->FileIsVisible[4]=FALSE;
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST));

	if (CurView->BoundsDisplayID) BoundsDisplayWrite (CurView->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView->lpBoundsDisplay);

	CurView->ID = 2;
	CurView->Active = TRUE; 
	_fstrcpy(CurView->Name,"Index Map");
	CurView->Parent = 0;
	CurView->Type = 7;
	CurView->ZoomTarget = 1;
	CurView->Bitmap = NULL;
	CurView->DesiredHeight = 0;
	CurView->DesiredWidth = 0;
	CurView->BackGroundColor = RGB(255,255,255);
	CurView->Shadow = TRUE; 
	CurView->Margin = 0;
	CurView->TagPointID = 3;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 2;
	CurView->TagPoint.y = 2;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width = 27;
	CurView->Height = 37;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE; 
    CurView->BoundsDisplayID = 0;
    CurView->lpBoundsDisplay = NULL;
    CurView->pTheme = 0;
	CurView->NumThemes = 0;
	CurView->NumFiles = 1;
	CurView->FileType[0]=2;
	CurView->lpFiles[0]=0;
	CurView->NumVisList = 1;
	_fstrcpy (CurView->VisName,"vislists\\index.vis");
	_fstrcpy (CurView->PickMacroFile,"pikmacro.txt");
	CurView->PickMacroFile[0]=0;
	
	if (hVisList)
	{
		GlobalUnlock(hVisList);
		GlobalFree(hVisList);
	}
	hVisList=GSSiGlobAlloc (GHND,sizeof(VISLIST));
	if (!hVisList) return(MemError());
	CurVis = (LPVISLIST)GlobalLock (hVisList); 
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->LastVisList=0;
	CurVis->NextVisList=0;
	CurVis->WantType[1]=0;
	CurVis->WantType[2]=0;

	
	CurView->StartupFunction=GF_PAN_ZOOM_TARGET;
	_fstrcpy(CurView->FunctionFile,"fundir\\test1.txt");
	_fstrcpy(CurView->FunctionDir,"index.txt");
	
	
	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));
	_fstrcpy (File,"\\newbase.plt");
	_fstrcpy (File,"[%DATA_LOC]maplib\\polbase.plt");

	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST)); 
	
	if (CurView->BoundsDisplayID) BoundsDisplayWrite (CurView->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView->lpBoundsDisplay);
	
	
    if (handle)
    {
    	GlobalUnlock(handle);
    	GlobalFree(handle);
    	handle=NULL;
    }
	handle=GSSiGlobAlloc (GHND,sizeof(THEME));
	CurTheme = (LPTHEME)GlobalLock(handle);
	CurView->pTheme = CurTheme;
	CurTheme->ID=GF_CRIME_THEME;
	CurTheme->TargetViewport=1;
	CurTheme->DisplayViewport=CurView->ID+1;
	_fstrcpy (CurTheme->DataFile,"[%DATA_LOC]attribut\\pid02.gmd");
	_fstrcpy (CurTheme->SQL,"REFNO=[%INT_REFNO]");
	_fstrcpy (CurTheme->Field[0].name,"MKT_VAL_200");
	_fstrcpy (CurTheme->Field[1].name,"BLDG_MKT");
	_fstrcpy (CurTheme->Contents,"PARCEL");
	CurTheme->SymNum=99;
	CurTheme->DataFileType = UMIFS_DATAFILE;
   	CurTheme->IsActive = TRUE;
	CurTheme->WantDataPass=FALSE;
	CurTheme->ComputeClassBoundaries=FALSE;
	CurTheme->DisplayScatterDiagram=FALSE;
	CurTheme->NumDesiredClass=5;

	CurTheme->ClassType=1;
	CurTheme->XLimit = 500000;
	CurTheme->YLimit = 500000;
	CurTheme->ClassColor[0]=RGB(255,0,0);
	CurTheme->ClassColor[1]=RGB(0,255,0);
	CurTheme->ClassColor[2]=RGB(0,0,255);
	CurTheme->ClassColor[3]=RGB(0,255,255);
	CurTheme->ClassColor[4]=RGB(255,128,0);
	CurTheme->ClassColor[5]=RGB(255,0,0);
	CurTheme->ClassColor[6]=RGB(0,255,0);
	CurTheme->ClassColor[7]=RGB(0,0,255);
	CurTheme->ClassColor[8]=RGB(255,128,0);
	CurTheme->ClassColor[9]=RGB(255,0,0);
	CurTheme->ClassColor[9]=RGB(0,255,0);
	CurTheme->ClassColor[10]=RGB(0,0,255);
	CurTheme->ClassColor[11]=RGB(255,128,0);
	CurTheme->ClassColor[12]=RGB(255,0,0);
	CurTheme->ClassColor[13]=RGB(0,255,0);
	CurTheme->ClassColor[14]=RGB(0,0,255);
	CurTheme->ClassColor[15]=RGB(255,128,0);
	CurTheme->YLimit = LONG_MAX;
	CurTheme->Margin = 1;
	CurTheme->ScatterWidth=5;
	CurTheme->ColorsWidth=10;
	CurTheme->InnerMargin=2;
	CurTheme->TitleHeight=12;
	CurTheme->BGColor=RGB(255,255,255);
	CurTheme->ScatterColor=RGB(0,0,0);
	CurTheme->ScatterBoxBG=RGB(255,255,255);
	CurTheme->TitleBoxBG=RGB(255,255,255);
	_fstrcpy (CurTheme->Title,"Market Value");
	_fstrcpy (CurTheme->TitleFont.lfFaceName,"Arial Rounded MT Bold");
	_fstrcpy (CurTheme->ClassFont1.lfFaceName,"Arial");
	_fstrcpy (CurTheme->ClassFont2.lfFaceName,"Arial");


	CurView->ID = 3;
	CurView->Active = TRUE;
	_fstrcpy(CurView->Name,"Legend");
	CurView->Parent = 0;
	CurView->Type = 1; 
	CurView->ZoomTarget = 0;
	CurView->DesiredHeight = 0;
	CurView->DesiredWidth = 0;
	CurView->BackGroundColor = RGB(225,225,225);
	CurView->Shadow = TRUE;
	CurView->TagPointID = 4;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 2;
	CurView->TagPoint.y = 2;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width = 27;
	CurView->Height = 57;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
    CurView->BoundsDisplayID = 0;
    CurView->lpBoundsDisplay = NULL;
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE;
	CurView->NumThemes = 0;
	CurView->NumFiles = 0;
	CurView->FileType[0]=2;
	CurView->lpFiles[0]=0;
	CurView->NumVisList = 1;
	CurView->VisName[0]='\0';
	_fstrcpy (CurView->PickMacroFile,"pikmacro.txt");
	CurView->PickMacroFile[0]=0;
	
	if (hVisList)
	{
		GlobalUnlock(hVisList);
		GlobalFree(hVisList);
	}
	hVisList=GSSiGlobAlloc (GHND,sizeof(VISLIST));
	if (!hVisList) return(MemError());
	CurVis = (LPVISLIST)GlobalLock (hVisList);
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->LastVisList=0;
	CurVis->NextVisList=0;
	
	CurView->StartupFunction=0;
	_fstrcpy(CurView->FunctionFile,"fundir\\test2.txt");
	_fstrcpy(CurView->FunctionDir,"index.txt");
	
	
	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST));
	
	_lwrite (FidConfig,CurTheme,sizeof(THEME));
	
	if (CurView->BoundsDisplayID) BoundsDisplayWrite (CurView->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView->lpBoundsDisplay);

	Signature = 28052;
    _lwrite (FidConfig,&Signature,2);
    _lwrite (FidConfig,&Version,2);
    
	GlobalUnlock (hViewports[0]);
	GlobalFree (hViewports[0]);	
    _lclose (FidConfig);  
    FidConfig = 0; 
    
	_fstrcpy (CfgName,"mporth.gmc");
	GSSiOpenFile (CfgName,(LPOFSTRUCT)&OFStruct,OF_DELETE);
	FidConfig = GSSiOpenFile (CfgName,(LPOFSTRUCT)&OFStruct,OF_CREATE);
    _lwrite (FidConfig,StartupCommand,sizeof(StartupCommand));
    _lwrite (FidConfig,StartupMenu,sizeof(StartupMenu));
    _lwrite (FidConfig,&NumIB,sizeof(NumIB));  
	NumViewports = 4; 
	CommandViewport = 1;
	WindowColor = RGB(255,255,255);
    _lwrite (FidConfig,&NumViewports,2);
    _lwrite (FidConfig,&CommandViewport,2);
    
    NumMenuMask=0;
    _lwrite (FidConfig,&NumMenuMask,sizeof(NumMenuMask)); 
    _lwrite (FidConfig,&WindowColor,4);
   	_lwrite (FidConfig,&TAGBox,sizeof(TAGBOX));
	hViewports[0]=GSSiGlobAlloc (GHND,sizeof(VIEWPORT));
	if (!hViewports[0]) return(MemError());
	CurView = (LPVIEWPORT)GlobalLock (hViewports[0]);
	
	CurView->ID = 1;
	CurView->Version = Version;
	CurView->Active = TRUE;
	CurView->Parent = 0;
	CurView->Type = 1; 
	CurView->DesiredHeight = 0;
	CurView->DesiredWidth = 0;
	CurView->BackGroundColor = RGB(255,255,255);
	CurView->Shadow = FALSE;  
	CurView->MarginPan=TRUE;
	CurView->Margin = 2;
	CurView->TagPointID = 1;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 0;
	CurView->TagPoint.y = 0;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width =100;
	CurView->Height = 100;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE; 
    CurView->BoundsDisplayID = 1;
    CurView->lpBoundsDisplay = BoundsDisplayInit (1,2,1);
    CurView->pTheme = 0;
	CurView->NumThemes = 0;
	CurView->NumFiles = 3;
	CurView->FileType[0]=2;
	CurView->lpFiles[0]=0; 
	_fstrcpy (CurView->FileID[0],"City Basemap");
	CurView->FileType[1]=4;
	CurView->lpFiles[1]=0;
	_fstrcpy (CurView->FileID[1],"TOPO");
	CurView->FileType[2]=5;
	CurView->lpFiles[2]=0;
	_fstrcpy (CurView->FileID[2],"orthlowc");
	CurView->NumVisList = 2;
	_fstrcpy (CurView->VisName,"vislists\\[%CONFIG].vis");
	_fstrcpy (CurView->PickName,"piklists\\[%CONFIG].pik");
	_fstrcpy (CurView->DisplayRedefFile,"[%CONFIG].rdf");
	_fstrcpy (CurView->PickMacroFile,"pikmacro.txt");
	
	
	CurView->StartupFunction=GF_PAN_TO_POINT;
	_fstrcpy(CurView->FunctionFile,"fundir\\basic2.txt");
	_fstrcpy(CurView->FunctionDir,"index.txt");
	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\basemap.plt");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\topolayr\\index");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	_fstrcpy (File,"orthlow\\index");
	
	_fstrcpy (File,"[%DATA_LOC]orthlowc\\filelist.txt");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len); 
	
	
	if (hVisList)
	{
		GlobalUnlock(hVisList);
		GlobalFree(hVisList);
	}
	hVisList=GSSiGlobAlloc (GHND,sizeof(VISLIST));
	if (!hVisList) return(MemError());
	CurVis = (LPVISLIST)GlobalLock (hVisList);
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->LastVisList=0;
	CurVis->NextVisList=0; 
	CurVis->MinPointSize = 6; 
	CurVis->MaxScale=10.0;
	CurVis->FileIsVisible[0]=FALSE;
	CurVis->FileIsVisible[4]=FALSE;
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST));
	
	InitVis ();
	CurVis->MinScale=10.0;
	CurVis->FileIsVisible[2]=FALSE;
	CurVis->FileIsVisible[3]=FALSE;
	CurVis->FileIsVisible[4]=FALSE;
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST));

	if (CurView->BoundsDisplayID) BoundsDisplayWrite (CurView->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView->lpBoundsDisplay);

	CurView->ID = 2;
	CurView->Active = TRUE; 
	_fstrcpy(CurView->Name,"Index Map");
	CurView->Parent = 0;
	CurView->Type = 7;
	CurView->ZoomTarget = 1;
	CurView->Bitmap = NULL;
	CurView->DesiredHeight = 0;
	CurView->DesiredWidth = 0;
	CurView->BackGroundColor = RGB(255,255,255);
	CurView->Shadow = TRUE; 
	CurView->Margin = 0;
	CurView->TagPointID = 3;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 2;
	CurView->TagPoint.y = 2;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width = 22;
	CurView->Height = 47;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE; 
    CurView->BoundsDisplayID = 0;
    CurView->lpBoundsDisplay = NULL;
    CurView->pTheme = 0;
	CurView->NumThemes = 0;
	CurView->NumFiles = 1;
	CurView->FileType[0]=2;
	CurView->lpFiles[0]=0;
	CurView->NumVisList = 1;
	_fstrcpy (CurView->VisName,"vislists\\index.vis");
	CurView->DisplayRedefFile[0]='\0';	
	_fstrcpy (CurView->PickMacroFile,"pikmacro.txt");
	CurView->PickMacroFile[0]=0;
	
	if (hVisList)
	{
		GlobalUnlock(hVisList);
		GlobalFree(hVisList);
	}
	hVisList=GSSiGlobAlloc (GHND,sizeof(VISLIST));
	if (!hVisList) return(MemError());
	CurVis = (LPVISLIST)GlobalLock (hVisList); 
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->LastVisList=0;
	CurVis->NextVisList=0;
	CurVis->WantType[1]=0;
	CurVis->WantType[2]=0;

	
	CurView->StartupFunction=GF_PAN_ZOOM_TARGET;
	_fstrcpy(CurView->FunctionFile,"fundir\\test1.txt");
	_fstrcpy(CurView->FunctionDir,"index.txt");
	
	
	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));
	_fstrcpy (File,"\\newbase.plt");
	_fstrcpy (File,"[%DATA_LOC]maplib\\index.plt");

	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST)); 
	
	if (CurView->BoundsDisplayID) BoundsDisplayWrite (CurView->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView->lpBoundsDisplay);
	
	
    if (handle)
    {
    	GlobalUnlock(handle);
    	GlobalFree(handle);
    	handle=NULL;
    }
	handle=GSSiGlobAlloc (GHND,sizeof(THEME));
	CurTheme = (LPTHEME)GlobalLock(handle);
	CurView->pTheme = CurTheme;
	CurTheme->ID=GF_SINGLE_VALUE_THEME;
	CurTheme->TargetViewport=1;
	CurTheme->DisplayViewport=CurView->ID+1;
	_fstrcpy (CurTheme->DataFile,"[%DATA_LOC]attribut\\pid02.gmd");
	_fstrcpy (CurTheme->SQL,"REFNO=[%INT_REFNO]");
	_fstrcpy (CurTheme->Field[0].name,"MKT_VAL_200");
	_fstrcpy (CurTheme->Field[1].name,"BLDG_MKT");
	_fstrcpy (CurTheme->Contents,"PARCEL");
	CurTheme->SymNum=99;
	CurTheme->DataFileType = UMIFS_DATAFILE;
   	CurTheme->IsActive = FALSE;
	CurTheme->WantDataPass=FALSE;
	CurTheme->ComputeClassBoundaries=TRUE;
	CurTheme->DisplayScatterDiagram=TRUE;
	CurTheme->NumDesiredClass=5;

	CurTheme->ClassType=1;
	CurTheme->XLimit = 500000;
	CurTheme->YLimit = 500000;
	CurTheme->ClassColor[0]=RGB(50,150,250);
	CurTheme->ClassColor[1]=RGB(150,250,250);
	CurTheme->ClassColor[2]=RGB(250,150,250);
	CurTheme->ClassColor[3]=RGB(250,50,150);
	CurTheme->ClassColor[4]=RGB(50,250,150);
	CurTheme->YLimit = LONG_MAX;
	CurTheme->Margin = 2;
	CurTheme->ScatterWidth=15;
	CurTheme->ColorsWidth=15;
	CurTheme->InnerMargin=2;
	CurTheme->TitleHeight=15;
	CurTheme->BGColor=RGB(255,255,255);
	CurTheme->ScatterColor=RGB(0,0,0);
	CurTheme->ScatterBoxBG=RGB(255,255,255);
	CurTheme->TitleBoxBG=RGB(255,255,255);
	_fstrcpy (CurTheme->Title,"Market Value");
	_fstrcpy (CurTheme->TitleFont.lfFaceName,"Arial Rounded MT Bold");
	_fstrcpy (CurTheme->ClassFont1.lfFaceName,"Arial");
	_fstrcpy (CurTheme->ClassFont2.lfFaceName,"Arial");


	CurView->ID = 3;
	CurView->Active = TRUE;
	_fstrcpy(CurView->Name,"Legend");
	CurView->Parent = 0;
	CurView->Type = 1; 
	CurView->ZoomTarget = 0;
	CurView->DesiredHeight = 0;
	CurView->DesiredWidth = 0;
	CurView->BackGroundColor = RGB(225,225,225);
	CurView->Shadow = TRUE;
	CurView->TagPointID = 4;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 2;
	CurView->TagPoint.y = 2;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width = 22;
	CurView->Height = 47;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
    CurView->BoundsDisplayID = 0;
    CurView->lpBoundsDisplay = NULL;
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE;
	CurView->NumThemes = 0;
	CurView->NumFiles = 0;
	CurView->FileType[0]=2;
	CurView->lpFiles[0]=0;
	CurView->NumVisList = 1;
	CurView->VisName[0]='\0';
	_fstrcpy (CurView->PickMacroFile,"pikmacro.txt");
	CurView->PickMacroFile[0]=0;
	
	if (hVisList)
	{
		GlobalUnlock(hVisList);
		GlobalFree(hVisList);
	}
	hVisList=GSSiGlobAlloc (GHND,sizeof(VISLIST));
	if (!hVisList) return(MemError());
	CurVis = (LPVISLIST)GlobalLock (hVisList);
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->LastVisList=0;
	CurVis->NextVisList=0;
	
	CurView->CurrentFunction=0;
	_fstrcpy(CurView->FunctionFile,"fundir\\test2.txt");
	_fstrcpy(CurView->FunctionDir,"index.txt");
	
	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST));
	
	_lwrite (FidConfig,CurTheme,sizeof(THEME));
	
	if (CurView->BoundsDisplayID) BoundsDisplayWrite (CurView->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView->lpBoundsDisplay);


	CurView->ID = 4;
	CurView->Parent = 0;
	CurView->Type = 1; 
	CurView->Active = FALSE; 
	_fstrcpy(CurView->Name,"Coordinate Display");
	CurView->DesiredHeight = 0;
	CurView->DesiredWidth = 0;
	CurView->BackGroundColor = RGB(255,255,255);
	CurView->Shadow = TRUE;
	CurView->TagPointID = 1;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 3;
	CurView->TagPoint.y = 3;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width = 30;
	CurView->Height = 5;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
    CurView->BoundsDisplayID = 0;
    CurView->lpBoundsDisplay = NULL;
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE;
	CurView->NumThemes = 0;
	CurView->NumFiles = 0;
	CurView->NumVisList = 0;
	_fstrcpy (CurView->PickMacroFile,"pikmacro.txt");
	CurView->PickMacroFile[0]=0;
	
	CurView->StartupFunction=0;
	
    if (handle)
    {
    	GlobalUnlock(handle);
    	GlobalFree(handle);
    	handle=NULL;
    }
	handle=GSSiGlobAlloc (GHND,sizeof(COORDINATEDISPLAY));
	CD = (LPCOORDINATEDISPLAY)GlobalLock(handle);
	CurView->pTheme = CD;
	CD->TargetViewport=1;
	CD->DisplayViewport=4;
	CD->ID = PF_COORD_DISPLAY;

	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));
	
	_lwrite (FidConfig,CD,sizeof(COORDINATEDISPLAY));
	
    if (handle)
    {
    	GlobalUnlock(handle);
    	GlobalFree(handle);
    	handle=NULL;
    }
	
	
	Signature = 28052;
    _lwrite (FidConfig,&Signature,2);
    _lwrite (FidConfig,&Version,2);
    
	GlobalUnlock (hViewports[0]);
	GlobalFree (hViewports[0]);	
    _lclose (FidConfig);  
    FidConfig = 0;

	_fstrcpy (CfgName,"GWATER.gmc");
	GSSiOpenFile (CfgName,(LPOFSTRUCT)&OFStruct,OF_DELETE);
	FidConfig = GSSiOpenFile (CfgName,(LPOFSTRUCT)&OFStruct,OF_CREATE);
    _lwrite (FidConfig,StartupCommand,sizeof(StartupCommand));
    _lwrite (FidConfig,StartupMenu,sizeof(StartupMenu));
    _lwrite (FidConfig,&NumIB,sizeof(NumIB));  
	NumViewports = 4; 
	CommandViewport = 2;
	WindowColor = RGB(255,255,255);
    _lwrite (FidConfig,&NumViewports,2);
    _lwrite (FidConfig,&CommandViewport,2);
    NumMenuMask=0;
    _lwrite (FidConfig,&NumMenuMask,sizeof(NumMenuMask));
    _lwrite (FidConfig,&WindowColor,4);
   	_lwrite (FidConfig,&TAGBox,sizeof(TAGBOX));
	hViewports[0]=GSSiGlobAlloc (GHND,sizeof(VIEWPORT));
	if (!hViewports[0]) return(MemError());
	CurView = (LPVIEWPORT)GlobalLock (hViewports[0]);
	
	CurView->ID = 1;
	CurView->Version = Version;
	_fstrcpy(CurView->Name,"Index Map");
	CurView->Active = TRUE;
	CurView->Parent = 0;
	CurView->Type = 7; 
	CurView->ZoomTarget = 2;
	CurView->DesiredHeight = 0;
	CurView->DesiredWidth = 0;
	CurView->BackGroundColor = RGB(255,255,255);
	CurView->Shadow = TRUE; 
	CurView->Margin = 2;
	CurView->TagPointID = 3;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 2;
	CurView->TagPoint.y = 2;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width = 22;
	CurView->Height = 47;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE; 
    CurView->BoundsDisplayID = 0;
    CurView->lpBoundsDisplay = NULL;
    CurView->pTheme = 0;
	CurView->NumThemes = 0;
	CurView->NumFiles = 1;
	CurView->FileType[0]=2;
	CurView->lpFiles[0]=0;
	CurView->NumVisList = 1;
	_fstrcpy (CurView->VisName,"vislists\\index.vis");
	CurView->DisplayRedefFile[0]='\0';	
	CurView->PickMacroFile[0]=0; 

	
	if (hVisList)
	{
		GlobalUnlock(hVisList);
		GlobalFree(hVisList);
	}
	hVisList=GSSiGlobAlloc (GHND,sizeof(VISLIST));
	if (!hVisList) return(MemError());
	CurVis = (LPVISLIST)GlobalLock (hVisList); 
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->LastVisList=0;
	CurVis->NextVisList=0;
	CurVis->WantType[1]=0;
	CurVis->WantType[2]=0;

	
	CurView->CurrentFunction=GF_PAN_ZOOM_TARGET;
	_fstrcpy(CurView->FunctionFile,"fundir\\viewport.txt");
	_fstrcpy(CurView->FunctionDir,"index.txt");
	
	
	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));
	_fstrcpy (File,"\\newbase.plt");
	_fstrcpy (File,"[%DATA_LOC]maplib\\index.plt");

	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST)); 
	
	if (CurView->BoundsDisplayID) BoundsDisplayWrite (CurView->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView->lpBoundsDisplay);
	
	CurView->ID = 2;
	CurView->Active = TRUE;
	CurView->Name[0]='\0';
	_fstrcpy(CurView->Name,"Primary Viewport");
	CurView->Parent = 0;
	CurView->Type = 1; 
	CurView->DesiredHeight = 0;
	CurView->DesiredWidth = 0;
	CurView->BackGroundColor = RGB(255,255,255);
	CurView->Shadow = TRUE;
	CurView->TagPointID = 2;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 2;
	CurView->TagPoint.y = 2;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width = 72;
	CurView->Height = 96;
	CurView->Margin = 2;
	CurView->MarginPan = TRUE;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
    CurView->BoundsDisplayID = 1;
    CurView->lpBoundsDisplay = BoundsDisplayInit (1,1,2);
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE;
    CurView->pTheme = 0;
	CurView->NumThemes = 0;
	CurView->NumFiles = 14;
	CurView->FileType[0]=2;
	CurView->lpFiles[0]=0; 
	_fstrcpy (CurView->FileID[0],"County Base");
	CurView->FileType[1]=5;
	CurView->lpFiles[1]=0;
	_fstrcpy (CurView->FileID[1],"Orthos");
	CurView->FileType[2]=4;
	CurView->lpFiles[2]=0;
	_fstrcpy (CurView->FileID[2],"Parcel Base");
	CurView->FileType[3]=4;
	CurView->lpFiles[3]=0;
	_fstrcpy (CurView->FileID[3],"Parcel Text");
	CurView->FileType[4]=4;
	CurView->lpFiles[4]=0;
	_fstrcpy (CurView->FileID[4],"Land Cover");
	CurView->FileType[5]=2;
	CurView->lpFiles[5]=0;
	_fstrcpy (CurView->FileID[5],"Wells");
	CurView->FileType[6]=2;
	CurView->lpFiles[6]=0;
	_fstrcpy (CurView->FileID[6],"Old Farmsites");
	CurView->FileType[7]=4;
	CurView->lpFiles[7]=0;
	_fstrcpy (CurView->FileID[7],"Well Head Prot");
	CurView->FileType[8]=2;
	CurView->lpFiles[8]=0;
	_fstrcpy (CurView->FileID[8],"Dumps");
	CurView->FileType[9]=2;
	CurView->lpFiles[9]=0;
	_fstrcpy (CurView->FileID[9],"Storage Tanks");
	CurView->FileType[10]=2;
	CurView->lpFiles[10]=0;
	_fstrcpy (CurView->FileID[10],"Haz Waste Gen");
	CurView->FileType[11]=2;
	CurView->lpFiles[11]=0;
	_fstrcpy (CurView->FileID[11],"TIGER");
	CurView->FileType[12]=2;
	CurView->lpFiles[12]=0;
	_fstrcpy (CurView->FileID[12],"CAD Areas");
	CurView->FileType[13]=2;
	CurView->lpFiles[13]=0;
	_fstrcpy (CurView->FileID[13],"Wetlands");
	CurView->NumVisList = 2;
	CurView->VisName[0]='\0';
	_fstrcpy (CurView->VisName,"vislists\\[%CONFIG].vis");
	_fstrcpy (CurView->PickName,"piklists\\[%CONFIG].pik");
	_fstrcpy (CurView->DisplayRedefFile,"[%CONFIG].rdf");
	_fstrcpy (CurView->PickMacroFile,"pikmacro.txt");
	
	
	CurView->StartupFunction=GF_PAN_TO_POINT;
	_fstrcpy(CurView->FunctionFile,"fundir\\appl1.txt");
	_fstrcpy(CurView->FunctionDir,"index.txt");
	
	
	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));
	_fstrcpy (File,"[%DATA_LOC]maplib\\newbase.plt");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]orthos\\filelist.txt");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len); 
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\baselayr\\index");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len); 
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\textlayr\\index");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\landcovr\\index");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\wells.plt");
    len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\atlas.plt");
    len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\wellprot\\index");
    len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\dumps.plt");
    len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\tanks.plt");
    len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\hazwaste.plt");
    len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\hentiger.plt");
    len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\cadareas.plt");
    len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\wetlands.plt");
    len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	if (hVisList)
	{
		GlobalUnlock(hVisList);
		GlobalFree(hVisList);
	}
	hVisList=GSSiGlobAlloc (GHND,sizeof(VISLIST));
	if (!hVisList) return(MemError());
	CurVis = (LPVISLIST)GlobalLock (hVisList);
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->LastVisList=0;
	CurVis->NextVisList=0; 
	CurVis->MinPointSize = 6; 
	CurVis->MaxScale=10.0;
	CurVis->FileIsVisible[0]=FALSE;
	CurVis->FileIsVisible[4]=FALSE;
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST));
	
	InitVis ();
	CurVis->MinScale=10.0;
	CurVis->FileIsVisible[2]=FALSE;
	CurVis->FileIsVisible[3]=FALSE;
	CurVis->FileIsVisible[4]=FALSE;
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST));

	
	if (CurView->BoundsDisplayID) BoundsDisplayWrite (CurView->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView->lpBoundsDisplay);

	
    if (handle)
    {
    	GlobalUnlock(handle);
    	GlobalFree(handle);
    	handle=NULL;
    }
	handle=GSSiGlobAlloc (GHND,sizeof(THEME));
	CurTheme = (LPTHEME)GlobalLock(handle);
	CurView->pTheme = CurTheme;
	CurTheme->ID=GF_SINGLE_VALUE_THEME;
	CurTheme->ID=GF_TWO_VALUE_THEME;
	CurTheme->TargetViewport=2;
	CurTheme->DisplayViewport=CurView->ID+1;
	_fstrcpy (CurTheme->DataFile,"[%DATA_LOC]attribut\\pid02.gmd");
	_fstrcpy (CurTheme->SQL,"REFNO=[%INT_REFNO]");
	_fstrcpy (CurTheme->Field[0].name,"MKT_VAL_200");
	_fstrcpy (CurTheme->Field[1].name,"BLDG_MKT"); 
	_fstrcpy (CurTheme->Contents,"PARCEL");
	CurTheme->SymNum=99;
	CurTheme->DataFileType = UMIFS_DATAFILE;
   	CurTheme->IsActive = FALSE;
	CurTheme->WantDataPass=FALSE;
	CurTheme->ComputeClassBoundaries=TRUE;
	CurTheme->DisplayScatterDiagram=TRUE;
	CurTheme->NumDesiredClass=5;

	CurTheme->ClassType=1;
	CurTheme->XLimit = 500000;
	CurTheme->YLimit = 500000;
	CurTheme->ClassColor[0]=RGB(50,150,250);
	CurTheme->ClassColor[1]=RGB(150,250,250);
	CurTheme->ClassColor[2]=RGB(250,150,250);
	CurTheme->ClassColor[3]=RGB(250,50,150);
	CurTheme->ClassColor[4]=RGB(50,250,150);
	CurTheme->YLimit = LONG_MAX;
	CurTheme->Margin = 2;
	CurView->MarginPan = FALSE;
	CurTheme->ScatterWidth=15;
	CurTheme->ColorsWidth=15;
	CurTheme->InnerMargin=2;
	CurTheme->TitleHeight=15;
	CurTheme->BGColor=RGB(255,255,255);
	CurTheme->ScatterColor=RGB(0,0,0);
	CurTheme->ScatterBoxBG=RGB(255,255,255);
	CurTheme->TitleBoxBG=RGB(255,255,255);
	_fstrcpy (CurTheme->Title,"Market Value");
	_fstrcpy (CurTheme->Title,"Market vs Building Value");
	_fstrcpy (CurTheme->TitleFont.lfFaceName,"Arial Rounded MT Bold");
	_fstrcpy (CurTheme->ClassFont1.lfFaceName,"Arial");
	_fstrcpy (CurTheme->ClassFont2.lfFaceName,"Arial");


	CurView->ID = 3;
	_fstrcpy(CurView->Name,"Legend");
	CurView->Active = TRUE;
	CurView->Parent = 0;
	CurView->Type = 1; 
	CurView->DesiredHeight = 0;
	CurView->DesiredWidth = 0;
	CurView->BackGroundColor = RGB(225,225,225);
	CurView->Shadow = TRUE;
	CurView->TagPointID = 4;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 2;
	CurView->TagPoint.y = 2;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width = 22;
	CurView->Height = 47;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
    CurView->BoundsDisplayID = 0;
    CurView->lpBoundsDisplay = NULL;
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE;
	CurView->NumThemes = 0;
	CurView->NumFiles = 0;
	CurView->FileType[0]=2;
	CurView->lpFiles[0]=0;
	CurView->NumVisList = 1;
	CurView->VisName[0]='\0';
	CurView->DisplayRedefFile[0]='\0';	
	
	if (hVisList)
	{
		GlobalUnlock(hVisList);
		GlobalFree(hVisList);
	}
	hVisList=GSSiGlobAlloc (GHND,sizeof(VISLIST));
	if (!hVisList) return(MemError());
	CurVis = (LPVISLIST)GlobalLock (hVisList);
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->LastVisList=0;
	CurVis->NextVisList=0;
	
	CurView->StartupFunction=0;
	_fstrcpy(CurView->FunctionFile,"fundir\\theme.txt");
	_fstrcpy(CurView->FunctionDir,"index.txt");
	
	
	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST));
	
	_lwrite (FidConfig,CurTheme,sizeof(THEME));
	
	if (CurView->BoundsDisplayID) BoundsDisplayWrite (CurView->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView->lpBoundsDisplay);


	CurView->ID = 4;
	CurView->Active = FALSE;
	CurView->Parent = 0;
	_fstrcpy(CurView->Name,"Coordinate Display");
	CurView->Type = 1; 
	CurView->DesiredHeight = 0;
	CurView->DesiredWidth = 0;
	CurView->BackGroundColor = RGB(225,225,225);
	CurView->Shadow = TRUE;
	CurView->TagPointID = 1;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 5;
	CurView->TagPoint.y = 5;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width = 30;
	CurView->Height = 6;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
    CurView->BoundsDisplayID = 0;
    CurView->lpBoundsDisplay = NULL;
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE;
	CurView->NumThemes = 0;
	CurView->NumFiles = 0;
	CurView->NumVisList = 0;
	_fstrcpy (CurView->PickMacroFile,"pikmacro.txt");
	CurView->PickMacroFile[0]=0;
	CurView->StartupFunction=0;
	
    if (handle)
    {
    	GlobalUnlock(handle);
    	GlobalFree(handle);
    	handle=NULL;
    }
	handle=GSSiGlobAlloc (GHND,sizeof(COORDINATEDISPLAY));
	CD = (LPCOORDINATEDISPLAY)GlobalLock(handle);
	CurView->pTheme = CD;
	CD->TargetViewport=2;
	CD->DisplayViewport=4;
	CD->ID = PF_COORD_DISPLAY;

	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));
	
	_lwrite (FidConfig,CD,sizeof(COORDINATEDISPLAY));
	
    if (handle)
    {
    	GlobalUnlock(handle);
    	GlobalFree(handle);
    	handle=NULL;
    }
	
	Signature = 28052;
    _lwrite (FidConfig,&Signature,2);
    _lwrite (FidConfig,&Version,2);
    
	GlobalUnlock (hViewports[0]);
	GlobalFree (hViewports[0]);	
    _lclose (FidConfig);
    FidConfig = 0;


	_fstrcpy (CfgName,"GWATER2.gmc");
	GSSiOpenFile (CfgName,(LPOFSTRUCT)&OFStruct,OF_DELETE);
	FidConfig = GSSiOpenFile (CfgName,(LPOFSTRUCT)&OFStruct,OF_CREATE);
    _lwrite (FidConfig,StartupCommand,sizeof(StartupCommand));
    _lwrite (FidConfig,StartupMenu,sizeof(StartupMenu));
    _lwrite (FidConfig,&NumIB,sizeof(NumIB));  
	NumViewports = 4; 
	CommandViewport = 1;
	WindowColor = RGB(255,255,255);
    _lwrite (FidConfig,&NumViewports,2);
    _lwrite (FidConfig,&CommandViewport,2);
    NumMenuMask=0;
    _lwrite (FidConfig,&NumMenuMask,sizeof(NumMenuMask));
    _lwrite (FidConfig,&WindowColor,4);
   	_lwrite (FidConfig,&TAGBox,sizeof(TAGBOX));
	hViewports[0]=GSSiGlobAlloc (GHND,sizeof(VIEWPORT));
	if (!hViewports[0]) return(MemError());
	CurView = (LPVIEWPORT)GlobalLock (hViewports[0]);
	
	CurView->ID = 1;
	CurView->Version = Version;
	CurView->Active = TRUE;
	CurView->Parent = 0;
	CurView->Type = 1; 
	CurView->DesiredHeight = 0;
	CurView->DesiredWidth = 0;
	CurView->BackGroundColor = RGB(255,255,255);
	CurView->Shadow = TRUE;
	CurView->TagPointID = 2;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 2;
	CurView->TagPoint.y = 2;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width = 96;
	CurView->Height = 96;
	CurView->Margin = 2;
	CurView->MarginPan = TRUE;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
    CurView->BoundsDisplayID = 1;
    CurView->lpBoundsDisplay = BoundsDisplayInit (1,2,1);
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE;
    CurView->pTheme = 0;
	CurView->NumThemes = 0;
	CurView->NumFiles = 15;
	CurView->FileType[0]=2;
	CurView->lpFiles[0]=0; 
	_fstrcpy (CurView->FileID[0],"County Base");
	CurView->FileType[1]=5;
	CurView->lpFiles[1]=0;
	_fstrcpy (CurView->FileID[1],"Orthos");
	CurView->FileType[2]=4;
	CurView->lpFiles[2]=0;
	_fstrcpy (CurView->FileID[2],"Parcel Base");
	CurView->FileType[3]=4;
	CurView->lpFiles[3]=0;
	_fstrcpy (CurView->FileID[3],"Parcel Text");
	CurView->FileType[4]=4;
	CurView->lpFiles[4]=0;
	_fstrcpy (CurView->FileID[4],"Land Cover");
	CurView->FileType[5]=2;
	CurView->lpFiles[5]=0;
	_fstrcpy (CurView->FileID[5],"Wells");
	CurView->FileType[6]=2;
	CurView->lpFiles[6]=0;
	_fstrcpy (CurView->FileID[6],"Old Farmsites");
	CurView->FileType[7]=4;
	CurView->lpFiles[7]=0;
	_fstrcpy (CurView->FileID[7],"Well Head Prot");
	CurView->FileType[8]=2;
	CurView->lpFiles[8]=0;
	_fstrcpy (CurView->FileID[8],"Dumps");
	CurView->FileType[9]=2;
	CurView->lpFiles[9]=0;
	_fstrcpy (CurView->FileID[9],"Storage Tanks");
	CurView->FileType[10]=2;
	CurView->lpFiles[10]=0;
	_fstrcpy (CurView->FileID[10],"Haz Waste Gen");
	CurView->FileType[11]=2;
	CurView->lpFiles[11]=0;
	_fstrcpy (CurView->FileID[11],"TIGER");
	CurView->FileType[12]=2;
	CurView->lpFiles[12]=0;
	_fstrcpy (CurView->FileID[12],"CAD Areas");
	CurView->FileType[13]=2;
	CurView->lpFiles[13]=0;
	_fstrcpy (CurView->FileID[13],"Wetlands");
	CurView->FileType[14]=2;
	CurView->lpFiles[14]=0;
	_fstrcpy (CurView->FileID[14],"Showcase");
	CurView->NumVisList = 2;
	CurView->VisName[0]='\0';
	_fstrcpy (CurView->VisName,"vislists\\[%CONFIG].vis");
	_fstrcpy (CurView->PickName,"piklists\\[%CONFIG].pik");
	_fstrcpy (CurView->DisplayRedefFile,"[%CONFIG].rdf");
	_fstrcpy (CurView->PickMacroFile,"pikmacro.txt");
	
	
	CurView->StartupFunction=GF_PAN_TO_POINT;
	_fstrcpy(CurView->FunctionFile,"fundir\\appl1.txt");
	_fstrcpy(CurView->FunctionDir,"index.txt");
	
	
	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));
	_fstrcpy (File,"[%DATA_LOC]maplib\\newbase.plt");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]orthos\\index");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len); 
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\baselayr\\index");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len); 
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\textlayr\\index");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\landcovr\\index");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\wells.plt");
    len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\atlas.plt");
    len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\wellprot\\index");
    len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\dumps.plt");
    len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\tanks.plt");
    len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\hazwaste.plt");
    len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\hentiger.plt");
    len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\cadareas.plt");
    len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\wetlands.plt");
    len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\showcase.plt");
    len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	if (hVisList)
	{
		GlobalUnlock(hVisList);
		GlobalFree(hVisList);
	}
	hVisList=GSSiGlobAlloc (GHND,sizeof(VISLIST));
	if (!hVisList) return(MemError());
	CurVis = (LPVISLIST)GlobalLock (hVisList);
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->LastVisList=0;
	CurVis->NextVisList=0; 
	CurVis->MinPointSize = 6; 
	CurVis->MaxScale=10.0;
	CurVis->FileIsVisible[0]=FALSE;
	CurVis->FileIsVisible[4]=FALSE;
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST));
	
	InitVis ();
	CurVis->MinScale=10.0;
	CurVis->FileIsVisible[2]=FALSE;
	CurVis->FileIsVisible[3]=FALSE;
	CurVis->FileIsVisible[4]=FALSE;
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST));

	
	if (CurView->BoundsDisplayID) BoundsDisplayWrite (CurView->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView->lpBoundsDisplay);

	
	CurView->ID = 2;
	_fstrcpy(CurView->Name,"Index Map");
	CurView->Active = TRUE;
	CurView->Parent = 0;
	CurView->Type = 7; 
	CurView->ZoomTarget = 1;
	CurView->DesiredHeight = 0;
	CurView->DesiredWidth = 0;
	CurView->BackGroundColor = RGB(255,255,255);
	CurView->Shadow = TRUE; 
	CurView->Margin = 2;   
	CurView->MarginPan = FALSE;
	CurView->TagPointID = 3;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 2;
	CurView->TagPoint.y = 2;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width = 22;
	CurView->Height = 47;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE; 
    CurView->BoundsDisplayID = 0;
    CurView->lpBoundsDisplay = NULL;
    CurView->pTheme = 0;
	CurView->NumThemes = 0;
	CurView->NumFiles = 1;
	CurView->FileType[0]=2;
	CurView->lpFiles[0]=0;
	CurView->NumVisList = 1;
	_fstrcpy (CurView->VisName,"vislists\\index.vis");
	CurView->DisplayRedefFile[0]='\0';	
	_fstrcpy (CurView->PickMacroFile,"pikmacro.txt");
	CurView->PickMacroFile[0]=0;
	
	if (hVisList)
	{
		GlobalUnlock(hVisList);
		GlobalFree(hVisList);
	}
	hVisList=GSSiGlobAlloc (GHND,sizeof(VISLIST));
	if (!hVisList) return(MemError());
	CurVis = (LPVISLIST)GlobalLock (hVisList); 
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->LastVisList=0;
	CurVis->NextVisList=0;
	CurVis->WantType[1]=0;
	CurVis->WantType[2]=0;

	
	CurView->StartupFunction=GF_PAN_ZOOM_TARGET;
	_fstrcpy(CurView->FunctionFile,"fundir\\viewport.txt");
	_fstrcpy(CurView->FunctionDir,"index.txt");
	
	
	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));
	_fstrcpy (File,"\\newbase.plt");
	_fstrcpy (File,"[%DATA_LOC]maplib\\index.plt");

	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST)); 
	
	if (CurView->BoundsDisplayID) BoundsDisplayWrite (CurView->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView->lpBoundsDisplay);
	
    if (handle)
    {
    	GlobalUnlock(handle);
    	GlobalFree(handle);
    	handle=NULL;
    }
	handle=GSSiGlobAlloc (GHND,sizeof(THEME));
	CurTheme = (LPTHEME)GlobalLock(handle);
	CurView->pTheme = CurTheme;
	CurTheme->ID=GF_TWO_VALUE_THEME;
	CurTheme->ID=GF_SINGLE_VALUE_THEME;
	CurTheme->TargetViewport=1;
	CurTheme->DisplayViewport=CurView->ID+1;
	_fstrcpy (CurTheme->DataFile,"[%DATA_LOC]attribut\\pid02.gmd");
	_fstrcpy (CurTheme->SQL,"REFNO=[%INT_REFNO]");
	_fstrcpy (CurTheme->Field[0].name,"MKT_VAL_200");
	_fstrcpy (CurTheme->Field[1].name,"BLDG_MKT"); 
	_fstrcpy (CurTheme->Contents,"PARCEL");
	CurTheme->SymNum=99;
	CurTheme->DataFileType = UMIFS_DATAFILE;
   	CurTheme->IsActive = FALSE;
	CurTheme->WantDataPass=FALSE;
	CurTheme->ComputeClassBoundaries=TRUE;
	CurTheme->DisplayScatterDiagram=TRUE;
	CurTheme->NumDesiredClass=5;

	CurTheme->ClassType=1;
	CurTheme->XLimit = 500000;
	CurTheme->YLimit = 500000;
	CurTheme->ClassColor[0]=RGB(50,150,250);
	CurTheme->ClassColor[1]=RGB(150,250,250);
	CurTheme->ClassColor[2]=RGB(250,150,250);
	CurTheme->ClassColor[3]=RGB(250,50,150);
	CurTheme->ClassColor[4]=RGB(50,250,150);
	CurTheme->YLimit = LONG_MAX;
	CurTheme->Margin = 2;
	CurView->MarginPan = FALSE;
	CurTheme->ScatterWidth=15;
	CurTheme->ColorsWidth=15;
	CurTheme->InnerMargin=2;
	CurTheme->TitleHeight=15;
	CurTheme->BGColor=RGB(255,255,255);
	CurTheme->ScatterColor=RGB(0,0,0);
	CurTheme->ScatterBoxBG=RGB(255,255,255);
	CurTheme->TitleBoxBG=RGB(255,255,255);
	_fstrcpy (CurTheme->Title,"Market vs Building Value");
	_fstrcpy (CurTheme->Title,"Market Value");
	_fstrcpy (CurTheme->TitleFont.lfFaceName,"Arial Rounded MT Bold");
	_fstrcpy (CurTheme->ClassFont1.lfFaceName,"Arial");
	_fstrcpy (CurTheme->ClassFont2.lfFaceName,"Arial");


	CurView->ID = 3;
	_fstrcpy(CurView->Name,"Legend");
	CurView->Active = TRUE;
	CurView->Parent = 0;
	CurView->Type = 1; 
	CurView->ZoomTarget = 0;
	CurView->DesiredHeight = 0;
	CurView->DesiredWidth = 0;
	CurView->BackGroundColor = RGB(225,225,225);
	CurView->Shadow = TRUE;
	CurView->TagPointID = 4;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 2;
	CurView->TagPoint.y = 2;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width = 22;
	CurView->Height = 47;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
    CurView->BoundsDisplayID = 0;
    CurView->lpBoundsDisplay = NULL;
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE;
	CurView->NumThemes = 0;
	CurView->NumFiles = 0;
	CurView->FileType[0]=2;
	CurView->lpFiles[0]=0;
	CurView->NumVisList = 1;
	CurView->VisName[0]='\0';
	CurView->DisplayRedefFile[0]='\0';	
	_fstrcpy (CurView->PickMacroFile,"pikmacro.txt");
	CurView->PickMacroFile[0]=0;
	
	if (hVisList)
	{
		GlobalUnlock(hVisList);
		GlobalFree(hVisList);
	}
	hVisList=GSSiGlobAlloc (GHND,sizeof(VISLIST));
	if (!hVisList) return(MemError());
	CurVis = (LPVISLIST)GlobalLock (hVisList);
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->LastVisList=0;
	CurVis->NextVisList=0;
	
	CurView->StartupFunction=0;
	_fstrcpy(CurView->FunctionFile,"fundir\\theme.txt");
	_fstrcpy(CurView->FunctionDir,"index.txt");
	
	
	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST));
	
	_lwrite (FidConfig,CurTheme,sizeof(THEME));
	
	if (CurView->BoundsDisplayID) BoundsDisplayWrite (CurView->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView->lpBoundsDisplay);


	CurView->ID = 4;
	CurView->Active = FALSE;
	CurView->Parent = 0;
	_fstrcpy(CurView->Name,"Coordinate Display");
	CurView->Type = 1; 
	CurView->DesiredHeight = 0;
	CurView->DesiredWidth = 0;
	CurView->BackGroundColor = RGB(225,225,225);
	CurView->Shadow = TRUE;
	CurView->TagPointID = 1;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 5;
	CurView->TagPoint.y = 5;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width = 30;
	CurView->Height = 6;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
    CurView->BoundsDisplayID = 0;
    CurView->lpBoundsDisplay = NULL;
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE;
	CurView->NumThemes = 0;
	CurView->NumFiles = 0;
	CurView->NumVisList = 0;
	_fstrcpy (CurView->PickMacroFile,"pikmacro.txt");
	CurView->PickMacroFile[0]=0;
	
	CurView->StartupFunction=0;
	
    if (handle)
    {
    	GlobalUnlock(handle);
    	GlobalFree(handle);
    	handle=NULL;
    }
	handle=GSSiGlobAlloc (GHND,sizeof(COORDINATEDISPLAY));
	CD = (LPCOORDINATEDISPLAY)GlobalLock(handle);
	CurView->pTheme = CD;
	CD->TargetViewport=1;
	CD->DisplayViewport=4;
	CD->ID = PF_COORD_DISPLAY;

	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));
	
	_lwrite (FidConfig,CD,sizeof(COORDINATEDISPLAY));
	
    if (handle)
    {
    	GlobalUnlock(handle);
    	GlobalFree(handle);
    	handle=NULL;
    }
	
	Signature = 28052;
    _lwrite (FidConfig,&Signature,2);
    _lwrite (FidConfig,&Version,2);
    
	GlobalUnlock (hViewports[0]);
	GlobalFree (hViewports[0]);	
    _lclose (FidConfig);
    FidConfig = 0;

	_fstrcpy (CfgName,"maplegv.gmc");
	GSSiOpenFile (CfgName,(LPOFSTRUCT)&OFStruct,OF_DELETE);
	FidConfig = GSSiOpenFile (CfgName,(LPOFSTRUCT)&OFStruct,OF_CREATE);
    _lwrite (FidConfig,StartupCommand,sizeof(StartupCommand));
    _lwrite (FidConfig,StartupMenu,sizeof(StartupMenu));
    _lwrite (FidConfig,&NumIB,sizeof(NumIB));  
	NumViewports = 4; 
	CommandViewport = 1;
	WindowColor = RGB(255,255,255);
    _lwrite (FidConfig,&NumViewports,2);
    _lwrite (FidConfig,&CommandViewport,2);
    
    NumMenuMask=0;
    _lwrite (FidConfig,&NumMenuMask,sizeof(NumMenuMask)); 
    _lwrite (FidConfig,&WindowColor,4);
   	_lwrite (FidConfig,&TAGBox,sizeof(TAGBOX));
	hViewports[0]=GSSiGlobAlloc (GHND,sizeof(VIEWPORT));
	if (!hViewports[0]) return(MemError());
	CurView = (LPVIEWPORT)GlobalLock (hViewports[0]);
	
	CurView->ID = 1;
	CurView->Version = Version;
	CurView->Active = TRUE;
	CurView->Parent = 0;
	CurView->Type = 1; 
	CurView->DesiredHeight = 0;
	CurView->DesiredWidth = 0;
	CurView->BackGroundColor = RGB(255,255,255);
	CurView->Shadow = FALSE; 
	CurView->Margin = 2;
	CurView->MarginPan=TRUE;
	CurView->TagPointID = 1;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 0;
	CurView->TagPoint.y = 0;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width =100;
	CurView->Height = 100;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE; 
    CurView->BoundsDisplayID = 1;
    CurView->lpBoundsDisplay = BoundsDisplayInit (1,2,1);
    CurView->pTheme = 0;
	CurView->NumThemes = 0;
	CurView->NumFiles = 9;
	CurView->FileType[0]=2;
	CurView->lpFiles[0]=0; 
	_fstrcpy (CurView->FileID[0],"Base Map");
	CurView->FileType[1]=5;
	CurView->lpFiles[1]=0;
	_fstrcpy (CurView->FileID[1],"USGS Orthos");
	CurView->FileType[2]=4;
	CurView->lpFiles[2]=0;
	_fstrcpy (CurView->FileID[2],"Parcel Base");
	CurView->FileType[3]=4;
	CurView->lpFiles[3]=0;
	_fstrcpy (CurView->FileID[3],"Parcel Text");
	CurView->FileType[4]=4;
	CurView->lpFiles[4]=0;
	_fstrcpy (CurView->FileID[4],"Topo Data");
	CurView->FileType[5]=4;
	CurView->lpFiles[5]=0;
	_fstrcpy (CurView->FileID[5],"Contours");
	CurView->FileType[6]=2;
	CurView->lpFiles[6]=0;
	_fstrcpy (CurView->FileID[6],"Water");
	CurView->FileType[7]=2;
	CurView->lpFiles[7]=0;
	_fstrcpy (CurView->FileID[7],"Planning");
	CurView->FileType[8]=2;
	CurView->lpFiles[8]=0;
	_fstrcpy (CurView->FileID[8],"TIGER");
	CurView->NumVisList = 2;
	_fstrcpy (CurView->VisName,"vislists\\[%CONFIG].vis");
	_fstrcpy (CurView->PickName,"piklists\\[%CONFIG].pik");
	_fstrcpy (CurView->DisplayRedefFile,"[%CONFIG].rdf");
	_fstrcpy (CurView->PickMacroFile,"pikmacro.txt");
	
	CurView->StartupFunction=GF_PAN_TO_POINT;
	_fstrcpy(CurView->FunctionFile,"fundir\\appl1.txt");
	_fstrcpy(CurView->FunctionDir,"index.txt");
	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\basemap.plt");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]USGSORTH\\index");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len); 
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\baselayr\\index");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len); 
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\textlayr\\index");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\topolayr\\index");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\contlayr\\index");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\water.plt");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\planning.plt");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\hentiger.plt");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	
	
	if (hVisList)
	{
		GlobalUnlock(hVisList);
		GlobalFree(hVisList);
	}
	hVisList=GSSiGlobAlloc (GHND,sizeof(VISLIST));
	if (!hVisList) return(MemError());
	CurVis = (LPVISLIST)GlobalLock (hVisList);
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->LastVisList=0;
	CurVis->NextVisList=0; 
	CurVis->MinPointSize = 6; 
	CurVis->MaxScale=10.0;
	CurVis->FileIsVisible[0]=FALSE;
	CurVis->FileIsVisible[4]=FALSE;
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST));
	
	InitVis ();
	CurVis->MinScale=10.0;
	CurVis->FileIsVisible[2]=FALSE;
	CurVis->FileIsVisible[3]=FALSE;
	CurVis->FileIsVisible[4]=FALSE;
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST));

	
	if (CurView->BoundsDisplayID) BoundsDisplayWrite (CurView->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView->lpBoundsDisplay);
	CurView->ID = 2;
	CurView->Active = TRUE; 
	_fstrcpy(CurView->Name,"Index Map");
	CurView->Parent = 0;
	CurView->Type = 7;
	CurView->ZoomTarget = 1;
	CurView->Bitmap = NULL;
	CurView->DesiredHeight = 0;
	CurView->DesiredWidth = 0;
	CurView->BackGroundColor = RGB(255,255,255);
	CurView->Shadow = TRUE; 
	CurView->Margin = 2;
	CurView->TagPointID = 3;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 2;
	CurView->TagPoint.y = 2;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width = 22;
	CurView->Height = 47;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE; 
    CurView->BoundsDisplayID = 0;
    CurView->lpBoundsDisplay = NULL;
    CurView->pTheme = 0;
	CurView->NumThemes = 0;
	CurView->NumFiles = 1;
	CurView->FileType[0]=2;
	CurView->lpFiles[0]=0;
	CurView->NumVisList = 1;
	CurView->DisplayRedefFile[0]='\0';	
	_fstrcpy (CurView->PickMacroFile,"pikmacro.txt");
	CurView->PickMacroFile[0]=0;
	
	if (hVisList)
	{
		GlobalUnlock(hVisList);
		GlobalFree(hVisList);
	}
	hVisList=GSSiGlobAlloc (GHND,sizeof(VISLIST));
	if (!hVisList) return(MemError());
	CurVis = (LPVISLIST)GlobalLock (hVisList); 
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->LastVisList=0;
	CurVis->NextVisList=0;
	CurVis->WantType[1]=0;
	CurVis->WantType[2]=0;

	
	CurView->StartupFunction=GF_PAN_ZOOM_TARGET;
	_fstrcpy(CurView->FunctionFile,"fundir\\viewport.txt");
	_fstrcpy(CurView->FunctionDir,"index.txt");
	
	
	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));
	_fstrcpy (File,"\\newbase.plt");
	_fstrcpy (File,"[%DATA_LOC]maplib\\index.plt");

	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST)); 
	
	if (CurView->BoundsDisplayID) BoundsDisplayWrite (CurView->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView->lpBoundsDisplay);
	
    if (handle)
    {
    	GlobalUnlock(handle);
    	GlobalFree(handle);
    	handle=NULL;
    }
	
	handle=GSSiGlobAlloc (GHND,sizeof(THEME));
	CurTheme = (LPTHEME)GlobalLock(handle);
	CurView->pTheme = CurTheme;
	CurTheme->ID=GF_SINGLE_VALUE_THEME;
	CurTheme->TargetViewport=1;
	CurTheme->DisplayViewport=CurView->ID+1;
	_fstrcpy (CurTheme->DataFile,"[%DATA_LOC]attribut\\pid02.gmd");
	_fstrcpy (CurTheme->SQL,"REFNO=[%INT_REFNO]");
	_fstrcpy (CurTheme->Field[0].name,"MKT_VAL_200");
	_fstrcpy (CurTheme->Field[1].name,"BLDG_MKT");
	_fstrcpy (CurTheme->Contents,"PARCEL");
	CurTheme->SymNum=99;
	CurTheme->DataFileType = UMIFS_DATAFILE;
   	CurTheme->IsActive = FALSE;
	CurTheme->WantDataPass=FALSE;
	CurTheme->ComputeClassBoundaries=TRUE;
	CurTheme->DisplayScatterDiagram=TRUE;
	CurTheme->NumDesiredClass=5;

	CurTheme->ClassType=1;
	CurTheme->XLimit = 500000;
	CurTheme->YLimit = 500000;
	CurTheme->ClassColor[0]=RGB(50,150,250);
	CurTheme->ClassColor[1]=RGB(150,250,250);
	CurTheme->ClassColor[2]=RGB(250,150,250);
	CurTheme->ClassColor[3]=RGB(250,50,150);
	CurTheme->ClassColor[4]=RGB(50,250,150);
	CurTheme->YLimit = LONG_MAX;
	CurTheme->Margin = 2;
	CurTheme->ScatterWidth=15;
	CurTheme->ColorsWidth=15;
	CurTheme->InnerMargin=2;
	CurTheme->TitleHeight=15;
	CurTheme->BGColor=RGB(255,255,255);
	CurTheme->ScatterColor=RGB(0,0,0);
	CurTheme->ScatterBoxBG=RGB(255,255,255);
	CurTheme->TitleBoxBG=RGB(255,255,255);
	_fstrcpy (CurTheme->Title,"Market Value");
	_fstrcpy (CurTheme->TitleFont.lfFaceName,"Arial Rounded MT Bold");
	_fstrcpy (CurTheme->ClassFont1.lfFaceName,"Arial");
	_fstrcpy (CurTheme->ClassFont2.lfFaceName,"Arial");


	CurView->ID = 3;
	CurView->Active = TRUE;
	_fstrcpy(CurView->Name,"Legend");
	CurView->Parent = 0;
	CurView->Type = 1; 
	CurView->ZoomTarget = 0;
	CurView->DesiredHeight = 0;
	CurView->DesiredWidth = 0;
	CurView->BackGroundColor = RGB(225,225,225);
	CurView->Shadow = TRUE;
	CurView->TagPointID = 4;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 2;
	CurView->TagPoint.y = 2;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width = 22;
	CurView->Height = 47;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
    CurView->BoundsDisplayID = 0;
    CurView->lpBoundsDisplay = NULL;
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE;
	CurView->NumThemes = 0;
	CurView->NumFiles = 0;
	CurView->FileType[0]=2;
	CurView->lpFiles[0]=0;
	CurView->NumVisList = 1;
	CurView->VisName[0]='\0';
	
	if (hVisList)
	{
		GlobalUnlock(hVisList);
		GlobalFree(hVisList);
	}
	hVisList=GSSiGlobAlloc (GHND,sizeof(VISLIST));
	if (!hVisList) return(MemError());
	CurVis = (LPVISLIST)GlobalLock (hVisList);
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->LastVisList=0;
	CurVis->NextVisList=0;
	
	CurView->CurrentFunction=0;
	_fstrcpy(CurView->FunctionFile,"fundir\\theme.txt");
	_fstrcpy(CurView->FunctionDir,"index.txt");
	
	
	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST));
	
	_lwrite (FidConfig,CurTheme,sizeof(THEME));
	
	if (CurView->BoundsDisplayID) BoundsDisplayWrite (CurView->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView->lpBoundsDisplay);


	CurView->ID = 4;
	CurView->Parent = 0;
	CurView->Type = 1; 
	CurView->Active = FALSE; 
	_fstrcpy(CurView->Name,"Coordinate Display");
	CurView->DesiredHeight = 0;
	CurView->DesiredWidth = 0;
	CurView->BackGroundColor = RGB(255,255,255);
	CurView->Shadow = TRUE;
	CurView->TagPointID = 1;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 3;
	CurView->TagPoint.y = 3;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width = 30;
	CurView->Height = 5;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
    CurView->BoundsDisplayID = 0;
    CurView->lpBoundsDisplay = NULL;
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE;
	CurView->NumThemes = 0;
	CurView->NumFiles = 0;
	CurView->NumVisList = 0;
	_fstrcpy (CurView->PickMacroFile,"pikmacro.txt");
	CurView->PickMacroFile[0]=0;
	
	CurView->StartupFunction=0;
	
    if (handle)
    {
    	GlobalUnlock(handle);
    	GlobalFree(handle);
    	handle=NULL;
    }
	handle=GSSiGlobAlloc (GHND,sizeof(COORDINATEDISPLAY));
	CD = (LPCOORDINATEDISPLAY)GlobalLock(handle);
	CurView->pTheme = CD;
	CD->TargetViewport=1;
	CD->DisplayViewport=4;
	CD->ID = PF_COORD_DISPLAY;

	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));
	
	_lwrite (FidConfig,CD,sizeof(COORDINATEDISPLAY));
	
    if (handle)
    {
    	GlobalUnlock(handle);
    	GlobalFree(handle);
    	handle=NULL;
    }
	
	
	Signature = 28052;
    _lwrite (FidConfig,&Signature,2);
    _lwrite (FidConfig,&Version,2);
    
	GlobalUnlock (hViewports[0]);
	GlobalFree (hViewports[0]);	
    _lclose (FidConfig);
    FidConfig = 0; 
    

	_fstrcpy (CfgName,"maplegv2.gmc");
	GSSiOpenFile (CfgName,(LPOFSTRUCT)&OFStruct,OF_DELETE);
	FidConfig = GSSiOpenFile (CfgName,(LPOFSTRUCT)&OFStruct,OF_CREATE);
    _lwrite (FidConfig,StartupCommand,sizeof(StartupCommand));
    _lwrite (FidConfig,StartupMenu,sizeof(StartupMenu));
    _lwrite (FidConfig,&NumIB,sizeof(NumIB));  
	NumViewports = 4; 
	CommandViewport = 1;
	WindowColor = RGB(255,255,255);
    _lwrite (FidConfig,&NumViewports,2);
    _lwrite (FidConfig,&CommandViewport,2);
    
    NumMenuMask=0;
    _lwrite (FidConfig,&NumMenuMask,sizeof(NumMenuMask)); 
    _lwrite (FidConfig,&WindowColor,4);
   	_lwrite (FidConfig,&TAGBox,sizeof(TAGBOX));
	hViewports[0]=GSSiGlobAlloc (GHND,sizeof(VIEWPORT));
	if (!hViewports[0]) return(MemError());
	CurView = (LPVIEWPORT)GlobalLock (hViewports[0]);
	
	CurView->ID = 1;
	CurView->Version = Version;
	CurView->Active = TRUE;
	CurView->Parent = 0;
	CurView->Type = 1; 
	CurView->DesiredHeight = 0;
	CurView->DesiredWidth = 0;
	CurView->BackGroundColor = RGB(255,255,255);
	CurView->Shadow = FALSE; 
	CurView->Margin = 2;
	CurView->MarginPan=TRUE;
	CurView->TagPointID = 1;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 2;
	CurView->TagPoint.y = 2;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width = 72;
	CurView->Height = 96;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE; 
    CurView->BoundsDisplayID = 1;
    CurView->lpBoundsDisplay = BoundsDisplayInit (1,2,1);
    CurView->pTheme = 0;
	CurView->NumThemes = 0;
	CurView->NumFiles = 9;
	CurView->FileType[0]=2;
	CurView->lpFiles[0]=0; 
	_fstrcpy (CurView->FileID[0],"Base Map");
	CurView->FileType[1]=4;
	CurView->lpFiles[1]=0;
	_fstrcpy (CurView->FileID[1],"Parcel Base");
	CurView->FileType[2]=4;
	CurView->lpFiles[2]=0;
	_fstrcpy (CurView->FileID[2],"Parcel Text");
	CurView->FileType[3]=4;
	CurView->lpFiles[3]=0;
	_fstrcpy (CurView->FileID[3],"Topo Data");
	CurView->FileType[4]=4;
	CurView->lpFiles[4]=0;
	_fstrcpy (CurView->FileID[4],"Contours");
	CurView->FileType[5]=2;
	CurView->lpFiles[5]=0;
	_fstrcpy (CurView->FileID[5],"Water");
	CurView->FileType[6]=2;
	CurView->lpFiles[6]=0;
	_fstrcpy (CurView->FileID[6],"Planning");
	CurView->FileType[7]=2;
	CurView->lpFiles[7]=0;
	_fstrcpy (CurView->FileID[7],"TIGER");
	CurView->FileType[8]=5;
	CurView->lpFiles[8]=0;
	_fstrcpy (CurView->FileID[8],"Orthos");
	CurView->NumVisList = 2;
	_fstrcpy (CurView->VisName,"vislists\\[%CONFIG].vis");
	_fstrcpy (CurView->PickName,"piklists\\[%CONFIG].pik");
	_fstrcpy (CurView->DisplayRedefFile,"[%CONFIG].rdf");
	_fstrcpy (CurView->PickMacroFile,"pikmacro.txt");
	
	
	CurView->StartupFunction=GF_PAN_TO_POINT;
	_fstrcpy(CurView->FunctionFile,"fundir\\appl1.txt");
	_fstrcpy(CurView->FunctionDir,"index.txt");
	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));
	
	_fstrcpy (File,"maplib\\basemap.plt");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"maplib\\baselayr\\index");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len); 
	
	_fstrcpy (File,"maplib\\textlayr\\index");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"maplib\\topolayr\\index");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"maplib\\contlayr\\index");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"maplib\\water.plt");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"maplib\\planning.plt");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"maplib\\hentiger.plt");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"orthos\\index");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	
	
	if (hVisList)
	{
		GlobalUnlock(hVisList);
		GlobalFree(hVisList);
	}
	hVisList=GSSiGlobAlloc (GHND,sizeof(VISLIST));
	if (!hVisList) return(MemError());
	CurVis = (LPVISLIST)GlobalLock (hVisList);
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->LastVisList=0;
	CurVis->NextVisList=0; 
	CurVis->MinPointSize = 6; 
	CurVis->MaxScale=10.0;
	CurVis->FileIsVisible[0]=FALSE;
	CurVis->FileIsVisible[4]=FALSE;
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST));
	
	InitVis ();
	CurVis->MinScale=10.0;
	CurVis->FileIsVisible[2]=FALSE;
	CurVis->FileIsVisible[3]=FALSE;
	CurVis->FileIsVisible[4]=FALSE;
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST));

	
	if (CurView->BoundsDisplayID) BoundsDisplayWrite (CurView->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView->lpBoundsDisplay);
	CurView->ID = 2;
	CurView->Active = TRUE; 
	_fstrcpy(CurView->Name,"Index Map");
	CurView->Parent = 0;
	CurView->Type = 7;
	CurView->ZoomTarget = 1;
	CurView->Bitmap = NULL;
	CurView->DesiredHeight = 0;
	CurView->DesiredWidth = 0;
	CurView->BackGroundColor = RGB(255,255,255);
	CurView->Shadow = TRUE; 
	CurView->Margin = 2;
	CurView->TagPointID = 3;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 2;
	CurView->TagPoint.y = 2;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width = 22;
	CurView->Height = 47;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE; 
    CurView->BoundsDisplayID = 0;
    CurView->lpBoundsDisplay = NULL;
    CurView->pTheme = 0;
	CurView->NumThemes = 0;
	CurView->NumFiles = 1;
	CurView->FileType[0]=2;
	CurView->lpFiles[0]=0;
	CurView->NumVisList = 1;
	CurView->DisplayRedefFile[0]='\0';	
	_fstrcpy (CurView->PickMacroFile,"pikmacro.txt");
	CurView->PickMacroFile[0]=0;
	
	if (hVisList)
	{
		GlobalUnlock(hVisList);
		GlobalFree(hVisList);
	}
	hVisList=GSSiGlobAlloc (GHND,sizeof(VISLIST));
	if (!hVisList) return(MemError());
	CurVis = (LPVISLIST)GlobalLock (hVisList); 
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->LastVisList=0;
	CurVis->NextVisList=0;
	CurVis->WantType[1]=0;
	CurVis->WantType[2]=0;

	
	CurView->StartupFunction=GF_PAN_TO_POINT;
	_fstrcpy(CurView->FunctionFile,"fundir\\viewport.txt");
	_fstrcpy(CurView->FunctionDir,"index.txt");
	
	
	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));
	_fstrcpy (File,"\\newbase.plt");
	_fstrcpy (File,"maplib\\index.plt");

	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST)); 
	
	if (CurView->BoundsDisplayID) BoundsDisplayWrite (CurView->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView->lpBoundsDisplay);
	
    if (handle)
    {
    	GlobalUnlock(handle);
    	GlobalFree(handle);
    	handle=NULL;
    }
	
	handle=GSSiGlobAlloc (GHND,sizeof(THEME));
	CurTheme = (LPTHEME)GlobalLock(handle);
	CurView->pTheme = CurTheme;
	CurTheme->ID=GF_SINGLE_VALUE_THEME;
	CurTheme->TargetViewport=1;
	CurTheme->DisplayViewport=CurView->ID+1;
	_fstrcpy (CurTheme->DataFile,"attribut\\pid02.gmd");
	_fstrcpy (CurTheme->SQL,"REFNO=[%INT_REFNO]");
	_fstrcpy (CurTheme->Field[0].name,"MKT_VAL_200");
	_fstrcpy (CurTheme->Field[1].name,"BLDG_MKT");
	_fstrcpy (CurTheme->Contents,"PARCEL");
	CurTheme->SymNum=99;
	CurTheme->DataFileType = UMIFS_DATAFILE;
   	CurTheme->IsActive = FALSE;
	CurTheme->WantDataPass=FALSE;
	CurTheme->ComputeClassBoundaries=TRUE;
	CurTheme->DisplayScatterDiagram=TRUE;
	CurTheme->NumDesiredClass=5;

	CurTheme->ClassType=1;
	CurTheme->XLimit = 500000;
	CurTheme->YLimit = 500000;
	CurTheme->ClassColor[0]=RGB(50,150,250);
	CurTheme->ClassColor[1]=RGB(150,250,250);
	CurTheme->ClassColor[2]=RGB(250,150,250);
	CurTheme->ClassColor[3]=RGB(250,50,150);
	CurTheme->ClassColor[4]=RGB(50,250,150);
	CurTheme->YLimit = LONG_MAX;
	CurTheme->Margin = 2;
	CurTheme->ScatterWidth=15;
	CurTheme->ColorsWidth=15;
	CurTheme->InnerMargin=2;
	CurTheme->TitleHeight=15;
	CurTheme->BGColor=RGB(255,255,255);
	CurTheme->ScatterColor=RGB(0,0,0);
	CurTheme->ScatterBoxBG=RGB(255,255,255);
	CurTheme->TitleBoxBG=RGB(255,255,255);
	_fstrcpy (CurTheme->Title,"Market Value");
	_fstrcpy (CurTheme->TitleFont.lfFaceName,"Arial Rounded MT Bold");
	_fstrcpy (CurTheme->ClassFont1.lfFaceName,"Arial");
	_fstrcpy (CurTheme->ClassFont2.lfFaceName,"Arial");


	CurView->ID = 3;
	CurView->Active = TRUE;
	_fstrcpy(CurView->Name,"Legend");
	CurView->Parent = 0;
	CurView->Type = 1; 
	CurView->ZoomTarget = 0;
	CurView->DesiredHeight = 0;
	CurView->DesiredWidth = 0;
	CurView->BackGroundColor = RGB(225,225,225);
	CurView->Shadow = TRUE;
	CurView->TagPointID = 4;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 2;
	CurView->TagPoint.y = 2;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width = 22;
	CurView->Height = 47;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
    CurView->BoundsDisplayID = 0;
    CurView->lpBoundsDisplay = NULL;
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE;
	CurView->NumThemes = 0;
	CurView->NumFiles = 0;
	CurView->FileType[0]=2;
	CurView->lpFiles[0]=0;
	CurView->NumVisList = 1;
	CurView->VisName[0]='\0';
	_fstrcpy (CurView->PickMacroFile,"pikmacro.txt");
	CurView->PickMacroFile[0]=0;
	
	if (hVisList)
	{
		GlobalUnlock(hVisList);
		GlobalFree(hVisList);
	}
	hVisList=GSSiGlobAlloc (GHND,sizeof(VISLIST));
	if (!hVisList) return(MemError());
	CurVis = (LPVISLIST)GlobalLock (hVisList);
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->LastVisList=0;
	CurVis->NextVisList=0;
	
	CurView->CurrentFunction=0;
	_fstrcpy(CurView->FunctionFile,"fundir\\theme.txt");
	_fstrcpy(CurView->FunctionDir,"index.txt");
	
	
	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST));
	
	_lwrite (FidConfig,CurTheme,sizeof(THEME));
	
	if (CurView->BoundsDisplayID) BoundsDisplayWrite (CurView->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView->lpBoundsDisplay);


	CurView->ID = 4;
	CurView->Parent = 0;
	CurView->Type = 1; 
	CurView->Active = FALSE; 
	_fstrcpy(CurView->Name,"Coordinate Display");
	CurView->DesiredHeight = 0;
	CurView->DesiredWidth = 0;
	CurView->BackGroundColor = RGB(255,255,255);
	CurView->Shadow = TRUE;
	CurView->TagPointID = 1;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 3;
	CurView->TagPoint.y = 3;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width = 30;
	CurView->Height = 5;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
    CurView->BoundsDisplayID = 0;
    CurView->lpBoundsDisplay = NULL;
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE;
	CurView->NumThemes = 0;
	CurView->NumFiles = 0;
	CurView->NumVisList = 0;
	_fstrcpy (CurView->PickMacroFile,"pikmacro.txt");
	CurView->PickMacroFile[0]=0;
	
	CurView->StartupFunction=0;
	
    if (handle)
    {
    	GlobalUnlock(handle);
    	GlobalFree(handle);
    	handle=NULL;
    }
	handle=GSSiGlobAlloc (GHND,sizeof(COORDINATEDISPLAY));
	CD = (LPCOORDINATEDISPLAY)GlobalLock(handle);
	CurView->pTheme = CD;
	CD->TargetViewport=1;
	CD->DisplayViewport=4;
	CD->ID = PF_COORD_DISPLAY;

	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));
	
	_lwrite (FidConfig,CD,sizeof(COORDINATEDISPLAY));
	
    if (handle)
    {
    	GlobalUnlock(handle);
    	GlobalFree(handle);
    	handle=NULL;
    }
	
	
	Signature = 28052;
    _lwrite (FidConfig,&Signature,2);
    _lwrite (FidConfig,&Version,2);
    
	GlobalUnlock (hViewports[0]);
	GlobalFree (hViewports[0]);	
    _lclose (FidConfig);
    FidConfig = 0; 
	if (hVisList)
	{
		GlobalUnlock(hVisList);
		GlobalFree(hVisList);
	}
	CurView=NULL;   
	_fstrcpy (CfgName,"deltax.gmc");
	GSSiOpenFile (CfgName,(LPOFSTRUCT)&OFStruct,OF_DELETE);
	FidConfig = GSSiOpenFile (CfgName,(LPOFSTRUCT)&OFStruct,OF_CREATE);
    _lwrite (FidConfig,StartupCommand,sizeof(StartupCommand));
    _lwrite (FidConfig,StartupMenu,sizeof(StartupMenu));
    _lwrite (FidConfig,&NumIB,sizeof(NumIB));  
	NumViewports = 2; 
	CommandViewport = 2;
	WindowColor = RGB(255,255,255);
    _lwrite (FidConfig,&NumViewports,2);
    _lwrite (FidConfig,&CommandViewport,2);
    
    NumMenuMask=0;
    _lwrite (FidConfig,&NumMenuMask,sizeof(NumMenuMask));   
    _lwrite (FidConfig,&WindowColor,4);
   	_lwrite (FidConfig,&TAGBox,sizeof(TAGBOX));
	hViewports[0]=GSSiGlobAlloc (GHND,sizeof(VIEWPORT));
	if (!hViewports[0]) return(MemError());
	CurView = (LPVIEWPORT)GlobalLock (hViewports[0]);
	
	CurView->ID = 1;
	CurView->Version = Version;
	CurView->Active = TRUE;
	CurView->Parent = 0;
	CurView->Type = 1; 
	CurView->DesiredHeight = 9.75;
	CurView->DesiredWidth = 7.5;  
	CurView->BorderPct = 0.75;
	CurView->BackGroundColor = RGB(255,255,255);
	CurView->Shadow = FALSE;  
	CurView->MarginPan=FALSE;
	CurView->Margin = 0;
	CurView->TagPointID = 1;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 2;
	CurView->TagPoint.y = 2;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width =96;
	CurView->Height = 96;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE; 
    CurView->BoundsDisplayID = 0;
    CurView->pTheme = 0;
	CurView->NumThemes = 0;
	CurView->NumFiles = 0;
	CurView->NumVisList = 0;
	
	
	CurView->StartupFunction=GF_PAN_TO_POINT;
	_fstrcpy(CurView->FunctionFile,"fundir\\infobox.txt");
	_fstrcpy(CurView->FunctionDir,"index.txt");
	_fstrcpy (CurView->PickMacroFile,"pikmacro.txt");
	CurView->PickMacroFile[0]=0;
	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));

	if (CurView->BoundsDisplayID) BoundsDisplayWrite (CurView->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView->lpBoundsDisplay);
	
	
	CurView->ID = 2;
	CurView->Active = TRUE;
	CurView->Parent = 1;
	CurView->Type = 1; 
	CurView->DesiredHeight = 0;
	CurView->DesiredWidth = 0;
	CurView->BorderPct = 0.2;
	CurView->BackGroundColor = RGB(255,255,255);
	CurView->Shadow = TRUE;  
	CurView->MarginPan=FALSE;
	CurView->Margin = 2;
	CurView->TagPointID = 2;
	CurView->TagPointType = 2;
	CurView->TagPoint.x = 3;
	CurView->TagPoint.y = 3;
	CurView->WidthType = 2;
	CurView->HeightType = 2;
	CurView->Width =0;
	CurView->Height = 75;
	CurView->NewBounds.xmn = -10000;
	CurView->NewBounds.xmx =  10000;
	CurView->NewBounds.ymn = -10000;
	CurView->NewBounds.ymx =  10000;
	CurView->HaveBounds = FALSE;
    CurView->WindowIsZoomed = FALSE; 
    CurView->BoundsDisplayID = 0;
    CurView->pTheme = 0;
	CurView->NumThemes = 0;
	CurView->NumFiles = 3;
	CurView->FileType[0]=2;
	CurView->lpFiles[0]=0; 
	_fstrcpy (CurView->FileID[0],"County Base");
	CurView->FileType[1]=4;
	CurView->lpFiles[1]=0;
	_fstrcpy (CurView->FileID[1],"Parcel Base");
	CurView->FileType[2]=4;
	CurView->lpFiles[2]=0;
	_fstrcpy (CurView->FileID[2],"Parcel Text");
	CurView->NumVisList = 2;
	_fstrcpy (CurView->VisName,"vislists\\[%CONFIG].vis");
	_fstrcpy (CurView->PickName,"piklists\\[%CONFIG].pik");
	_fstrcpy (CurView->PickMacroFile,"pikmacro.txt");
//	CurView->PickMacroFile[0]=0;
	
	
	CurView->StartupFunction=GF_PAN_TO_POINT;
	_fstrcpy(CurView->FunctionFile,"fundir\\infobox.txt");
	_fstrcpy(CurView->FunctionDir,"index.txt");
	i=_lwrite (FidConfig,CurView,sizeof(VIEWPORT));
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\basemap.plt");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\baselayr\\index");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len); 
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\textlayr\\index");
	len = _fstrlen (File)+1;
	_lwrite (FidConfig,&len,2);
	_lwrite (FidConfig,&File,len);
	
	
	if (hVisList)
	{
		GlobalUnlock(hVisList);
		GlobalFree(hVisList);
	}
	hVisList=GSSiGlobAlloc (GHND,sizeof(VISLIST));
	if (!hVisList) return(MemError());
	CurVis = (LPVISLIST)GlobalLock (hVisList);
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->LastVisList=0;
	CurVis->NextVisList=0; 
	CurVis->MinPointSize = 6; 
	CurVis->MaxScale=10.0;
	CurVis->FileIsVisible[0]=FALSE;
	CurVis->FileIsVisible[4]=FALSE;
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST));
	
	InitVis ();
	CurVis->MinScale=10.0;
	CurVis->FileIsVisible[2]=FALSE;
	CurVis->FileIsVisible[3]=FALSE;
	CurVis->FileIsVisible[4]=FALSE;
	
	_lwrite (FidConfig,CurVis,sizeof(VISLIST));

	if (CurView->BoundsDisplayID) BoundsDisplayWrite (CurView->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView->lpBoundsDisplay);
    if (handle)
    {
    	GlobalUnlock(handle);
    	GlobalFree(handle);
    	handle=NULL;
    }
	
	
	Signature = 28052;
    _lwrite (FidConfig,&Signature,2);
    _lwrite (FidConfig,&Version,2);
    
	GlobalUnlock (hViewports[0]);
	GlobalFree (hViewports[0]);	
    _lclose (FidConfig);
    FidConfig = 0;



int temprep (int i)
{   
	HIGHLIGHTDATA	HighlightData;
	short pos=BT_FIRST;
	HFILE	Fid, Fid1; 
	OFSTRUCT	OFStruct;    
	char	str[256];
	
/*	Fid = GSSiOpenFile ("MP.TXT",&OFStruct,OF_CREATE);
	
    while (!BT_FIND (hHighlight,(LPSTR)&PickList[NumPicked].Refno,pos,BT_ANY,(LPSTR)&HighlightData))
    {
    	pos=BT_NEXT;
    	if (HighlightData.Show)
    	{
			PickList[0]=HighlightData.PD;
			ProcessPickedItem (0,FALSE);
//			if (CurrentChangeDate)
			{
				_fstrcpy (str,"[%INT_REFNO],[ROUTE],[MILEP],[XCOR],[YCOR]");  
				ExpandText (str);
				fputstring (str,Fid);  
			}
		}
	}      		                 
	_lclose (Fid); 
*/    BTVARDESC   BTVar[3];
 	HANDLE	hBT; 
 	short	Dummy; 
 	long	Ref;   
 	LPSTR	lpCom;
 	
    BTVar[0].BT_VARTYP=BT_INTEGER;
    BTVar[0].BT_VARLEN=2;
    BTVar[0].BT_VAROFF=0;
    BT_CREATE ("temp.btr", 2, FALSE, 1, 1,(LPBTVARDESC) &BTVar,FALSE, 0, 0, FALSE);
    hBT = BT_OPEN ("temp.btr", 0, BT_WRITE, 0);
	Fid = GSSiOpenFile ("FILE20.TXT",&OFStruct,OF_CREATE);
	Fid1 = GSSiOpenFile ("MP4.TXT",&OFStruct,OF_READ);
	while (fgetstring (str,sizeof(str)-2,Fid1))
	{
		lpCom = _fstrchr (str,',');
		*lpCom = 0;
		Ref = atol (str);
		lpCom++;
		BT_PUT (hBT,(LPSTR)&Ref,(LPSTR)&Dummy);
		fputstring (lpCom,Fid);
	}
	_lclose (Fid1);	
	Fid1 = GSSiOpenFile ("MP3.TXT",&OFStruct,OF_READ);
	while (fgetstring (str,sizeof(str)-2,Fid1))
	{
		lpCom = _fstrchr (str,',');
		*lpCom = 0;
		Ref = atol (str);
		lpCom++;
		if (BT_FIND (hBT,(LPSTR)&Ref,BT_FIRST,BT_EQ,(LPSTR)&Dummy))
			fputstring (lpCom,Fid);
	}
	_lclose (Fid1);
	_lclose (Fid);
	BT_CLOSE (hBT);
	remove ("temp.btr");   
	return 1;	
}
 

int TAGFontHtToPixels (float TAGFontHeight,int CoordStyle)
{   DPOINT	DPoint1, DPoint2;
	POINT	Point1, Point2; 
	float	Xfac;
	long	iLogPixsY,iLogPixsX, HRes;

	switch (CoordStyle)
	{
		case 0:
			DPoint1.x = 0;
			DPoint1.y = 0;
	    	Point1 = BasePtToWinPt (DPoint1);
			DPoint2.x = TAGFontHeight*100;
			DPoint2.y = 0;
	    	Point2 = BasePtToWinPt (DPoint2);
	    	return ((int)(idist(Point1,Point2)/100));
	    	break;
	    case 1:
	    	return (IDNINT((TAGFontHeight * (CurView->Rect.top - CurView->Rect.bottom))));
	    	break;
	    case 2:
	    	iLogPixsX = GetDeviceCaps(CurView->hDC, LOGPIXELSX);  
	    	HRes = GetDeviceCaps(CurView->hDC, HORZRES);  
	    	Xfac = (float)iLogPixsX * (float)(MainRect.right - MainRect.left)/HRes;
	    	return (IDNINT(TAGFontHeight * Xfac/2));
	    	break;
	}

}

float PixelsToTAGFontHt (LPLOGFONT lpFont)
{   DPOINT	DPoint1, DPoint2;
	POINT	Point1, Point2;
	int		height;
	int 	Pixels;     
	float	Xfac,WidthToHeightRatio;
	long	iLogPixsY,iLogPixsX, HRes;
    
    Pixels = lpFont->lfHeight;
	switch (TAGBox.CoordStyle)
	{
		case 0:
			Point1.x = 0;
			Point1.y = 0;
	    	DPoint1 = WinPtToBasePt (Point1);
			DPoint2.x = 0;
			DPoint2.y = Pixels;
	    	DPoint2 = WinPtToBasePt (Point2);
	    	return (ldistp(DPoint1,DPoint2));
	    	break;
	    case 1: 
	    	iLogPixsY = GetDeviceCaps(CurView->hDC, LOGPIXELSY); 
	   // 	return ((float)Pixels/(float)iLogPixsY);
   // TAGBox.LogFont.lfHeight = -1 * (iLogPixsY * TAGBox.TXheight / 72); 
	    	height = CurView->Rect.top - CurView->Rect.bottom;  
	    	if (!height) return 1.0;
	    	return ((float)Pixels/height);  
	    	break;
	    case 2:
	    	iLogPixsX = GetDeviceCaps(CurView->hDC, LOGPIXELSX);  
	    	HRes = GetDeviceCaps(CurView->hDC, HORZRES);  
//	    	WidthToHeightRatio = GetFontWtoH (CurView->hDC,lpFont);
	    	Xfac = (float)iLogPixsX * (float)(MainRect.right - MainRect.left)/HRes;
	    	return ((float) Pixels / (Xfac/2));
	    	break;
	}

}    

void ConvertClass (LPSTR str)
{ 
	char	FromClass[30][4]={"A01","A02","A03","A04","A05","A06","A07","A08","A09","A10","A11","A12","A13","A14","A15","A16","A17","A18","A19","A20","A21","B01","B02","B03","1","2","3","4","5","6"};
	char	ToClass[30][4]={"A40","A10","A31","A35","Z01","Z01","A20","A32","A36","A42","A42","A63","A50","A71","F20","A70","C10","C20","E10","C10","C20","B10","B20","B30","A11","A21","A31","A31","A31","A41"}; 
	short	i;  
	
	for (i=0;i<30;i++)
		if (!_fstricmp (str,FromClass[i]))
		{
			_fstrcpy (str,ToClass[i]);
			return;
		}
	return;
}
     

void MIFToTIGER (short nPoly,LPINT lpnPnts,LPHANDLE phDPoints,LPLONG pTLID)
{
    long    Num, TLID, Frame, Offset;
    char    StreetName[44], str[128], OutRec[256];
    short      st, st2, st3, stn,  Version, hltpos;
    OFSTRUCT    OFStruct;
    HCURSOR hcurSave; 
    BOOL    Good, FirstMap=TRUE;
    long    NewFileMarker=LONG_MIN;  
    HANDLE  hBT=0;
    DPOINT  FromPoint, ToPoint;
    TIGER1_PEOPLENET    Segdata;
    HIGHLIGHTDATA       HighlightData;
    HANDLE hSymbol;   
    LPSYMBOL    pSymbol;
    short     HaveState=-1;
    
    TLID = *pTLID;
    (*pTLID)++;
    if (nPoly !=1)
    	return;
            _fstrcpy(StreetName,"[STREET]");//used for city level files
            _fstrcpy(StreetName,"[HIGHWAY]");// used for Canada highways
            _fstrcpy(StreetName,"[AltGeoCodeName]");// used for Quebec
            ExpandText (StreetName);  
            _fmemset (OutRec,' ',230);
            OutRec[0]='1';
            OutRec[4]='0';
            sprintf (str,"%10ld",TLID);
            _fmemmove (&OutRec[5],str,_fstrlen(str));
            _fmemmove (&OutRec[19],StreetName,_fstrlen(StreetName)); 
            _fstrcpy (str,"[CLASS]");
            ExpandText (str);
            ConvertClass (str);
            _fmemmove (&OutRec[55],str,_fstrlen(str));
            {
                short SavePFN; 
                                              
                {
                    LPDPOINT    lpDpoints, lpDpoints2; 
                    LPMNMXCORD  lpRect;
                    LPINT   npt; 
                    short     nPnts,i; 
                    HANDLE  hNewPoints;
                    char    OutRec2[256];
                    short     ninrec=0, irec;
        
                    nPnts = *lpnPnts;
                    lpDpoints = GlobalLock (*phDPoints); 
                    FromPoint = *lpDpoints++; 
                    _fmemset (OutRec2,' ',230); 
                    OutRec2[0]='2';
                    OutRec2[4]='0';
                    irec=1;
                    sprintf (str,"%10ld%3i",TLID,irec); 
                    _fmemmove (&OutRec2[5],str,_fstrlen(str));
                    
                    for (i=1;i<nPnts-1;i++,lpDpoints++)
                    {   
                        BOOL    HaveRec=FALSE;
                        
                        sprintf (str,"%10ld%9ld",
                                IDNINT(lpDpoints->x*1000000), 
                                IDNINT(lpDpoints->y*1000000));
                        _fmemmove (&OutRec2[ninrec*19+18],str,_fstrlen(str)); 
                        ninrec++;
                        if (ninrec==10)
                        {
                            OutRec2[208]=0;
                            fputstring(OutRec2,FidTIGER2);
                            _fmemset (OutRec2,' ',230);
                            OutRec2[0]='2';
                            OutRec2[4]='0';
                            irec++;
                            sprintf (str,"%10ld%3i",TLID,irec); 
                            _fmemmove (&OutRec2[5],str,_fstrlen(str));
                            ninrec = 0;
                        }
                        
                    }
                    if (ninrec)
                    {
                        OutRec2[208]=0;
                        fputstring(OutRec2,FidTIGER2);
                    }
                    ToPoint = *lpDpoints;
                    GlobalUnlock(*phDPoints);
                }
                sprintf (str,"%10ld%9ld%10ld%9ld",
                        IDNINT(FromPoint.x*1000000), 
                        IDNINT(FromPoint.y*1000000), 
                        IDNINT(ToPoint.x*1000000), 
                        IDNINT(ToPoint.y*1000000));
                _fmemmove (&OutRec[190],str,_fstrlen(str));
                OutRec[228]=0;
                fputstring(OutRec,FidTIGER1);
            }
    return;
} 

BOOL CreateNullMap(void)
{	int		Signature, Version;
	OFSTRUCT	OFStruct;
	MNMXCORD	Bounds;
	LPVIEWPORT	SaveView;
	char		FullName[128]; 
	int			FidNull,l, i, MaxType, Type, zero=0;
	char		NullName[128];
	long		NewUsedDescOffset, NewColorPaletteOffset, NewTranPointOffset, NewGraphicsOffset;
	long		NewPrimeOffset, PrimeOffset;
	
	_fstrcpy (PltName,"[%PLOT]");
	if (!OpenMap (NULL,NULL)) return(FALSE);
	_fstrcpy (NullName,"maplib");
    if (!GetSaveName (hWndMain,NullName,IDS_FILTERPLT,".PLT")) return FALSE;
		
	Fid = GSSiOpenFile (PltName,(LPOFSTRUCT)&OFStruct,OF_READ);
	FidNull = GSSiOpenFile (NullName,(LPOFSTRUCT)&OFStruct,OF_CREATE);
	if (FidNull == HFILE_ERROR) return (FALSE);
    _llseek(Fid,(LONG)-(6),2);

    _lread (Fid,&Signature,2);
    _lread (Fid,&Version,2);
    _llseek(Fid,(LONG)-(6+12),2);
    _lread (Fid,&PrimeOffset,4);
    _lread (Fid,&MinMax,8);
    
    _llseek(Fid,PrimeOffset,0);
    ProcessPrimarySeg (NULL, NULL, NULL,TRUE);
    
    NewUsedDescOffset = _llseek(FidNull,0,2);
	_llseek(Fid,UsedDescOffset,0);
	CopyGraphicsBlock (FidNull,-1,0,0,0);
    
    NewColorPaletteOffset = _llseek(FidNull,0,2);
	_llseek(Fid,ColorPaletteOffset,0);
	CopyGraphicsBlock (FidNull,-1,0,0,0);
    
    NewTranPointOffset = _llseek(FidNull,0,2);
	_llseek(Fid,TranPointOffset,0);
	CopyGraphicsBlock (FidNull,-1,0,0,0);
    
    _llseek(Fid,GraphicsOffset,0);
    NewGraphicsOffset = _llseek(FidNull,0,2);
    MaxType = CopyQuadTree(FidNull);                                   
    for (Type=0;Type<MaxType;Type++)
		ClearQuadTree(FidNull,Type,NewGraphicsOffset);
		
    NewPrimeOffset = _llseek(FidNull,0,2);
    _llseek(Fid,PrimeOffset,0);
    CopyGraphicsBlock (FidNull, NewUsedDescOffset, NewColorPaletteOffset, NewTranPointOffset, NewGraphicsOffset);
    _lwrite (FidNull,&NewPrimeOffset,4);
    _lwrite (FidNull,&MinMax,8);
    _lwrite (FidNull,&Signature,2);
    _lwrite (FidNull,&Version,2); 
    _lwrite (FidNull,&zero,2);
    
    _lclose (FidNull);
    CloseMap();  
    
    _fullpath (FullName,NullName,sizeof(FullName));
	MessageBox(hWndMain, FullName,"Null Map Complete", MB_OK);
        
	return (TRUE);

} 

            case IDM_REMOVE_NULL_LINES:	 
            {
            	HFILE	Fid, Fid2;
            	int		ii, len;
            	LPSTR	pstr, lpdot;
            	OFSTRUCT	OFStruct; 
            	HANDLE	hstr; 
            	char	Name2[128];
				HCURSOR	hcurSave;
            	
				if (!GetFileName (hWnd,Name,IDS_FILTERTEXT)) break;
				_fstrcpy (Name2,Name);
				lpdot = _fstrrchr (Name2,'.');
				*lpdot = 0;
				_fstrcat (Name2,".tmp");
	
				hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
            	Fid = GSSiOpenFile (Name,&OFStruct,OF_READ);
            	Fid2 = GSSiOpenFile (Name2,&OFStruct,OF_CREATE);
            	
            	hstr = GSSiGlobAlloc (GHND,4096);
            	pstr = GlobalLock (hstr);
            	while (fgetstring (pstr,4090,Fid))
            	{
            		len = _fstrlen (pstr);  
            		if (len) 
            		{
            			if (!fputstring (pstr,Fid2))
            			{   
            				_lclose (Fid);
            				_lclose (Fid2);
            				remove (Name2);  
						   GSSiSetCursor(hcurSave); 
					   		MessageBox( GetFocus(),"Disk full","Error", MB_OK|MB_ICONEXCLAMATION);
            				break;
            			}
            		}
            	}
			    GSSiSetCursor(hcurSave); 
            	GlobalUnlock (hstr);
            	GlobalFree (hstr);
            	_lclose (Fid);
            	_lclose (Fid2); 
            	remove (Name);
            	rename (Name2,Name);
		   		MessageBox( GetFocus(),"Null lines have been removed","", MB_OK);
            } 
            break;

            case IDM_MISC2:  
            {   
            	int	pos, state;
            	HANDLE	hBTCities1;
            	HFILE	Fid;
            	OFSTRUCT	OFStruct;
            	CITIESKEY1	CitiesKey1;
            	CITIESDATA1	CitiesData1; 
            	char	Name[32];
            	 
            	 GetCityCoord ("XXXXX",NULL,NULL);
            	for (state=1;state<61;state++)
            	{   
	           		sprintf (Name,"state%2.2i.txt",state); 
	           		SetWindowText (hWnd,Name);
	            	Fid = GSSiOpenFile (Name,&OFStruct,OF_CREATE);
	            	fputstring ("Cities in ",Fid);
	            	pos = BT_FIRST;
					hBTCities1 = BT_OPEN ("cities1.btr",0,BT_READ,0); 
					while(!BT_FIND (hBTCities1,(LPSTR)&CitiesKey1,pos,BT_ANY,(LPSTR)&CitiesData1)) 
					{   
						if (CitiesData1.MinMax.xmn < 1000000 && CitiesKey1.State==state)
						{
							sprintf (str,"%s|(%f,%f,%f,%f)",CitiesData1.Name,
									CitiesData1.MinMax.xmn,
									CitiesData1.MinMax.ymn,
									CitiesData1.MinMax.xmx,
									CitiesData1.MinMax.ymx);     
							fputstring (str,Fid); 
						}
						pos = BT_NEXT;
					} 
					_lclose (Fid);
					BT_CLOSE (hBTCities1);
				}
            
             
             }
            	 break;  
            	 



BOOL CreateExtract ()
{   
	FILE *Fid;
	char	str[258];
	long	Refno; 
	OFSTRUCT	OFStruct;  
	HFILE	FidExtract;
	
	Fid = fopen ("c:\\pidref.txt","r"); 
	FidExtract = GSSiOpenFile ("c:\\extract",&OFStruct,OF_CREATE);
	while (fgetss (str,256,Fid))
	{
		Refno = atol(str);
		if (PickByRefno (Refno,NULL,NULL,-1))
		{
			OpenMap (CurView->hWnd,CurView->hDC);
			WriteTranData (FidExtract,hTranFileToBase);
			CloseMap();
			WritePickedItem (FidExtract,0);
		} 
	}
	fclose (Fid);
	_lclose (FidExtract);
}

 
void ShowPossibleRoutes(void)
{   int	i;
	POINT	FromPoint;
	DPOINT	MidPoint, ToPoint;
	long	Offset;

	MidPoint.x = (CLPoint1.x + CLPoint2.x)/2;
	MidPoint.y = (CLPoint1.y + CLPoint2.y)/2;

	FromPoint  = BasePtToWinPt(MidPoint);

	GetTurnTLID (AddRefno,CurFrame,NULL,TRUE);
	if (NumPossibleRoutes)
	{
	    SetDlgItemText(TurnWnd,IDC_TURN_MESSAGE,"Pick the next street (marked by green arrows)");
		PosRouteRect.left = INT_MAX;
		PosRouteRect.right = INT_MIN;
		PosRouteRect.top = INT_MAX;
		PosRouteRect.bottom = INT_MIN;
		for (i=0;i<NumPossibleRoutes;i++)
		{
	        ToPoint = dnewpt (MidPoint,PossibleAz[i],20*BaseToWinFactor);
			PosToPoint[i] = BasePtToWinPt(ToPoint);
			PosRouteRect.left = min(PosRouteRect.left,PosToPoint[i].x);
			PosRouteRect.right = max(PosRouteRect.right,PosToPoint[i].x);
			PosRouteRect.top = min(PosRouteRect.top,PosToPoint[i].y);
			PosRouteRect.bottom = max(PosRouteRect.bottom,PosToPoint[i].y);
		}
		PosRouteRect.left -= 7;
		PosRouteRect.right += 7;
		PosRouteRect.top -= 7;
		PosRouteRect.bottom += 7;
	    PosRouteSavedScreen = SaveScreen (CurView->hDC,PosRouteRect);
	    RegisterSavedScreen (&PosRouteSavedScreen,PosRouteRect);
	    if (NumPossibleRoutes==1)
	    {
			OpenAddressFiles (hWndMain);
			if (BT_FIND (hSegData,(LPSTR)&AddRefno,BT_FIRST,BT_EQ,(LPSTR)&Offset))
				goto Show;
		 	ReadSegData (Offset,&Segdata);
	     	if (Segdata.street_num!=PossibleStreet) goto Show;
			SetRouteControls (-2,0,0,0);
	    }
	    else 
	    {
Show:	
			for (i=0;i<NumPossibleRoutes;i++)
			{
				DrawTurnArrow (CurView->hDC,FromPoint,PosToPoint[i]);
			}
			SetRouteControls (-2,0,0,0);
			HaveTurnArrows=TRUE;
		}
	}
    return;
}
long GetTurnTLID (long TLID, long CurFrame, BOOL Left, BOOL All)
{   int		stSeg, stInt; 
	long	Offset, IntLong, IntLat, fframe, tframe, TestFrame, jpegframe, Street; 
	DPOINT	Point;
	double	AZ, Deflection, az, tol, PCT, Length;
	BOOL	DeflectLeft;
    
    if (UpdateAddress) return(NULL);
	OpenAddressFiles (hWndMain);
    stSeg = BT_FIND (hSegData,(LPSTR)&TLID,BT_FIRST,BT_EQ,(LPSTR)&Offset);
	ReadSegData (Offset,&Segdata);
	fframe = Segdata.fframe;
	tframe = Segdata.tframe;
	if (labs (CurFrame - fframe) <
		labs (CurFrame - tframe)) 
	{
   		IntLong = Segdata.frlong; 
   		IntLat  = Segdata.frlat; 
   	}
   	else
   	{
   		IntLong = Segdata.tolong; 
   		IntLat = Segdata.tolat; 
   	}
	IntersectKey.Long = IntLong; 
	IntersectKey.Lat  = IntLat; 
	IntersectKey.TLID = 0;
	NumPossibleRoutes=0;
    stInt = BT_FIND (hIntersect,(LPSTR)&IntersectKey,BT_FIRST,BT_GE,(LPSTR)&Street);
    while (!stInt)
    {   
    	if (IntLong == IntersectKey.Long && IntLat == IntersectKey.Lat)
    	{   
    		if (IntersectKey.TLID != TLID)
    		{   
			    stSeg = BT_FIND (hSegData,(LPSTR)&IntersectKey.TLID,BT_FIRST,BT_EQ,(LPSTR)&Offset);
				ReadSegData (Offset,&Segdata);
				TestFrame = (Segdata.fframe + Segdata.tframe)/2; 
				if (!TestFrame) goto Next;
			 	if (!CheckThisVideo (IntersectKey.TLID,TestFrame,3,&jpegframe,NULL))goto Next;
				if (Segdata.frlong != IntLong || Segdata.frlat != IntLat)
					PCT = 0.9;
				else
					PCT = 0.1;
    			CloseMap();
    			if (GetSegCoorByPct (IntersectKey.TLID,PCT,&Point,&AZ,&Length,FALSE))
    			{   
					CloseMap ();
					az = AZ;

					if (Segdata.frlong != IntLong || Segdata.frlat != IntLat)
						az = az + PY;
	                if (All)
	                {   
	                	PossibleStreet=Segdata.street_num;
	                	PossibleAz[NumPossibleRoutes]=az;
	                	PossibleTLID[NumPossibleRoutes]=IntersectKey.TLID;
	                	NumPossibleRoutes++;
	                }
    				Deflection = DeflectionAngle (CurAZ,az);
    				tol = HALFPI/4;
    				if (fabs(Deflection) > tol && !All)
    					if ((Left && Deflection>0) || (!Left && Deflection<0))
    								return (IntersectKey.TLID);
    			Next:;
				}
    		}
		    stInt = BT_FIND (hIntersect,(LPSTR)&IntersectKey,BT_NEXT,BT_ANY,(LPSTR)&Street);
        }
        else
        	stInt = 31;
    }
	CloseMap ();

    return (NULL);
}

	else if (!_fstricmp (lpAction,"CITYTOUR")) 
	{    
		lpDB = lpNext;
		if (!lpDB)
		{
	   		 MessageBox( GetFocus(), str,"Error in Database parameter", MB_OK|MB_ICONEXCLAMATION);
			 return FALSE;
		}
		*lpDB++=0;
		if (!(lpSQL = _fstrchr(lpDB,',')))
		{
	   		 MessageBox( GetFocus(), str,"Error in SQL parameter", MB_OK|MB_ICONEXCLAMATION);
			 return FALSE;
		}
		*lpSQL++=0;
		if (!(lpHOUSE = _fstrchr(lpSQL,',')))
		{
	   		 MessageBox( GetFocus(), str,"Error in House Number parameter", MB_OK|MB_ICONEXCLAMATION);
			 return FALSE;
		}
		*lpHOUSE++=0;
		if (!(lpSTREET = _fstrchr(lpHOUSE,',')))
		{
	   		 MessageBox( GetFocus(), str,"Error in Street Name parameter", MB_OK|MB_ICONEXCLAMATION);
			 return FALSE;
		}
		*lpSTREET++=0;  
		if (!(lpCITY = _fstrchr(lpSTREET,',')))
		{
	   		 MessageBox( GetFocus(), str,"Error in City Name parameter", MB_OK|MB_ICONEXCLAMATION);
			 return FALSE;
		}
		*lpCITY++=0;
		if (!(lpVIEWPORT = _fstrchr(lpCITY,',')))
		{
	   		 MessageBox( GetFocus(), str,"Error in Viewport parameter", MB_OK|MB_ICONEXCLAMATION);
			 return FALSE;
		}
		*lpVIEWPORT++=0;  
		StartGEOSPANvp = CurView->ID;
		GEOSPANAtAddress (lpDB,lpSQL,lpHOUSE,lpSTREET,lpCITY,lpVIEWPORT);
    }
BOOL ConvertStreetNumber (LPSTR buf,WORD *len)
{   
	LPINT	ipnt;
	LPLONG	pStreets;
	BYTE	*Pcode; 
	LPSTR	EndLoc;
	
    EndLoc = buf+*len;
    ipnt = EndLoc;
    *ipnt = 0;
    ipnt = buf; 
    while (*ipnt != 0)
    {   Pcode = ipnt;
    	ipnt++;

	    switch (*Pcode)
        {   
            case 10:  
            {   short i;
            
            	pStreets = ipnt;
            	for (i=0;i<4;i++,pStreets++) 
            		*pStreets = ConvertTIGERStreetToGMStreet (*pStreets);

            	return TRUE;
            }
            break;

            default: 
            	SkipSubRec (Pcode,&ipnt);
            break;

        } 
    }
	return FALSE;
}

long ConvertTIGERStreetToGMStreet (long TIGERStreet)
{ 
	long	NewNum=0, OldNum;
	char	Name[66], str[66];    
	
	if (!TIGERStreet)
		return 0;
	OldNum = labs (TIGERStreet);
	if (BT_FIND (hNames1,(LPSTR)&OldNum,BT_FIRST,BT_EQ,(LPSTR)&Name))
		return 0;
	NewNum = GetStreetNumFromName (Name,1,BT_FIRST,str);
	if (NewNum)
		return (NewNum);
	NewNum = AddStreetName (Name,0,"","","","");	
/*
	if (!OpenStreetNameTable (FALSE))  
                 
    if (OpenGSStreetNames (BT_READ,1)) 
                 if ((StreetNum = GetStreetNumFromName (Name,1,BT_FIRST,CurrentStreetName))) 
                 {
					 CloseStreetNameTable(); 
                 }
                 else 
                 {
                 	char	str[128];
					STNDSN_INIT();
					STNDST(Name, _fstrlen(Name),STDNAMv,NRONAMv,NMONLYv,
							     SANSCHv,NANDCHv,NCMPNMv,ORIGNMv);   
					if ((StreetNum = GetStreetNumFromName (STDNAMv,2,BT_FIRST,str)))
					{
						SendDlgItemMessage (hWndDlg,IDC_STREET_NAME_LIST,LB_RESETCONTENT,NULL,NULL);
						while (StreetNum)
						{
							SendDlgItemMessage (hWndDlg,IDC_STREET_NAME_LIST,LB_ADDSTRING,NULL,(LPARAM)str);
						    StreetNum = GetStreetNumFromName (STDNAMv,2,BT_NEXT,str);
						}
						sprintf (mess,"There are one or more names similar to '%s' already in the table (dislayed in the list). Select one or pick Add Name?",Name);
						MessageBox (hWndDlg,mess,NULL,MB_OK);
					}
                 	else
                 	{
	                 	sprintf (mess,"'%s' is not in the street name table. Pick 'Add Name' if you wish to add this name",Name);
					 	MessageBox (hWndDlg,mess,NULL,0); 
					} 
                 }
                 break;
				 
            }   
            case IDC_ADD_NAME: 
            {
            	 char Name[34],TrueName[34], str[128];
            	  
                 GetDlgItemText (hWndDlg,IDC_STREET_NAME,Name,32); 
                 
				 STNDSN_INIT();
				 STNDST(Name, _fstrlen(Name),STDNAMv,NRONAMv,NMONLYv,
				 						     SANSCHv,NANDCHv,NCMPNMv,ORIGNMv);   
                 if ((StreetNum = GetStreetNumFromName (STDNAMv,2,BT_FIRST,str)))
                 {
                 	char	mess[256];
                 	
					SendDlgItemMessage (hWndDlg,IDC_STREET_NAME_LIST,LB_RESETCONTENT,NULL,NULL);
					while (StreetNum)
					{
			 			SendDlgItemMessage (hWndDlg,IDC_STREET_NAME_LIST,LB_ADDSTRING,NULL,(LPARAM)str);
                 	    StreetNum = GetStreetNumFromName (STDNAMv,2,BT_NEXT,str);
                 	}
                 	sprintf (mess,"There is one or more names similar to '%s' already in the table (dislayed in the list). Do you still wish to enter this new name?",Name);
				 	if (MessageBox (hWndDlg,mess,NULL,MB_YESNO) == IDNO)
				 		break;
                 }
                 StreetNum = AddStreetName (Name); 
                 CurPath = StreetNum;
                 _fstrcpy(CurrentStreetName,Name);
				 CloseStreetNameTable();
				 EndDialog(hWndDlg, TRUE); 
	             break;   
*/
	return NewNum; 
}
BOOL LoadExtract()
{   
	HANDLE	hFtoB;  
	long	Refno, Offset;  
	long	Done=0;
	ITEM	*ItemHeader;
	HANDLE	hMap;
	LPINT	pMap, EndItem;  
	HFILE	FidMap;
	OFSTRUCT	OFStruct;  
	UINT	Length; 
	short	i2;

	_fstrcpy(PltName,"maplib\\showcase.plt");  
	CloseMap();
	if (!OpenMap (CurView->hWnd,NULL)) return(FALSE); 
	FidMap = GSSiOpenFile ("c:\\extract",&OFStruct,OF_READ);
	
	hFtoB = ReadTranData (FidMap);

	while (_lread(FidMap,&Refno,4)==4)
	{   
		Done += 4;
		Done += _lread(FidMap,&Length,2);  
		hMap = GSSiGlobAlloc (LMEM_MOVEABLE,Length+4);
		pMap = GlobalLock (hMap);
		Done += _lread(FidMap,pMap,Length); 
		ItemHeader = pMap; 

	    EndItem = pMap + abs(ItemHeader->Len);
	    EndItem+=6;
	    *EndItem = 0;
		ConvertPoly (pMap,hFtoB,hTranBaseToFile);
		
		GetFileConnectOffset(0,ItemHeader->MinMax,Length+2+4+2,-1); 
		_lwrite (Fid,pMap,Length);
		i2=13;
		_lwrite(Fid,&i2,2);
		Offset = -1;
		_lwrite(Fid,&Offset,4);
		i2 = 0;
		_lwrite(Fid,&i2,2);  
		GlobalUnlock(hMap);
		GlobalFree(hMap);  
		GlobalFree (hFtoB);
		hFtoB = ReadTranData (FidMap);
	}
	GlobalFree (hFtoB);
	_lclose (FidMap);
	CloseMap(); 
	return TRUE;
}



void MarkStreetFile (HWND hWnd)
{   short stName, stMax;
    long    MinHouseNum, LastMax;
    long    NumTot=0, NumOutOfOrder=0;
    char    StrName[33];
    char    str[64];
    short   StrNum;
    HDC     hDC;
    hDC = GetDC (hWnd);
    stName = BT_FIND (hNames,StrName,BT_FIRST,BT_ANY,(LPSTR)&StrNum);
    while (!stName)
    {   SegMaxKey.StreetNum = StrNum;
        SegMaxKey.MaxHouseNum = 0;
        SegMaxKey.Segid = 0;
        LastMax = 0;
        stMax = BT_FIND (hSegMax,(LPSTR)&SegMaxKey,BT_FIRST,BT_GT,(LPSTR)&MinHouseNum);
        CheckSeg: if (!stMax && SegMaxKey.StreetNum == StrNum)
        {   if (LastMax >= MinHouseNum)
            {   StrNum = -StrNum;
                NumOutOfOrder++;
                UMDB_UPDATE_BT_DATA (hNames,(LPSTR)&StrNum);
            }
            else
            {   LastMax = SegMaxKey.MaxHouseNum;
                stMax = BT_FIND (hSegMax,(LPSTR)&SegMaxKey,BT_NEXT,BT_ANY,(LPSTR)&MinHouseNum);
                goto CheckSeg;
            }
        }
        wsprintf(str, "%ld    %ld",NumOutOfOrder,NumTot);
        TextOut (hDC,60,40,str,_fstrlen(str));
        NumTot++;
        stName = BT_FIND (hNames,StrName,BT_NEXT,BT_ANY,(LPSTR)&StrNum);
    }
    ReleaseDC(hWnd,hDC);
    return;
}

BOOL LoadCitiesTable (void)
{   HANDLE hDB;
    LPGWFLDINFO lpGWFldInfo;
    LPGWDHEADER lpGWDHead;
    HANDLE      hBT, hSQL, hCurSymbol;
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
    CITIESKEY1  CitiesKey1;
    CITIESKEY2  CitiesKey2;
    CITIESDATA1 CitiesData1;
    DPOINT      LatLong;
    BTVARDESC   BTVar[3];
    HANDLE      hBTCities1, hBTCities2;    
    char        str[256]; 
    LPSTR       lpSpace;       
    double      Minx=-94.209675,Miny=44.550366,Maxx=-92.377734,Maxy=45.389885;
    
/*CreateIntersectionFile (TRUE);
    return TRUE;
*/
    hSQL = 0;
    if (!OpenDataFile ("ODBC|TIGER CITIES|plcusa.txt","",BT_READ,&hSQL))
        return FALSE;      
    
        
    SaveHandle = FilePathHandle;
    FilePathHandle = GlobalAlloc (GHND,sizeof(HANDLE)+2+MAXFILESINPATH*sizeof(HANDLE));
    FilePathPtr = (LPFILEPATH) GlobalLock (FilePathHandle);
    FilePathPtr->LastPathHandle = SaveHandle;
    FilePathPtr->FileHandle=hSQL;
    FilePathPtr->NumFiles=1; 
    GlobalUnlock(FilePathHandle);
        
    SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
    FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 

    
        BTVar[0].BT_VARTYP=BT_INTEGER;
        BTVar[0].BT_VARLEN=4;
        BTVar[0].BT_VAROFF=0;
        BTVar[1].BT_VARTYP=BT_INTEGER;
        BTVar[1].BT_VARLEN=2;
        BTVar[1].BT_VAROFF=4;
        BTVar[2].BT_VARTYP=BT_INTEGER;
        BTVar[2].BT_VARLEN=4;
        BTVar[2].BT_VAROFF=6;
        BT_CREATE ("cities2.btr", sizeof(DPOINT), FALSE, 3, 1,(LPBTVARDESC)&BTVar,FALSE, 0, 0, FALSE);
        hBTCities2= BT_OPEN ("cities2.btr", 0, BT_WRITE, 0); 
//      BT_CREATE ("tiger1.btr", 4, FALSE, 1, 1,(LPBTVARDESC)&BTVar,FALSE, 0, 0, FALSE);
    
    More=TRUE;
    while (More)
    {
        GetValFromOpenFiles ("STATECODE",str);
        CitiesKey1.State = atoi(str);
        GetValFromOpenFiles ("FIPS",str);
        CitiesKey1.FIPS = atol(str);
        GetValFromOpenFiles ("LATITUDE",str);
        CitiesData1.LatLong.y = atof(str)/1000000;
        GetValFromOpenFiles ("LONGITUDE",str);  
        CitiesData1.LatLong.x = atof(str)/1000000;
        GetValFromOpenFiles ("POPULATION",str);
        GetValFromOpenFiles ("NAME",CitiesData1.Name);   
        if ((lpSpace = _fstrrchr (CitiesData1.Name,' ')))
            *lpSpace = 0;
        CitiesData1.Pop = IDNINT(atof(str));
        CitiesKey2.Pop = -CitiesData1.Pop;
        CitiesKey2.State = CitiesKey1.State;
        CitiesKey2.FIPS = CitiesKey1.FIPS; 
        CitiesData1.MinMax.xmn = DBL_MAX;
        CitiesData1.MinMax.xmx = -DBL_MAX; 
        CitiesData1.MinMax.ymn = DBL_MAX;
        CitiesData1.MinMax.ymx = -DBL_MAX;
        BT_PUT (hBTCities1,(LPSTR)&CitiesKey1,(LPSTR)&CitiesData1);
        BT_PUT (hBTCities2,(LPSTR)&CitiesKey2,(LPSTR)&CitiesData1.LatLong); 
        rc = FetchODBCRecord (SQLPtr);  
        if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) More=FALSE;
    }
    GlobalUnlock(FilePtr->FileHandle); 
    GlobalUnlock (SQLPtr->OFHandle);
    GlobalFree (FilePathHandle);
    FilePathHandle = SaveHandle;
    CloseDataFile (TRUE, &hSQL);  
    BT_CLOSE (hBTCities1);
    BT_CLOSE (hBTCities2); 
    return TRUE;
} 



short FixF61 (short dum)
{   
    HFILE   FidIn, FidOut; 
    char    str[260]; 
    OFSTRUCT    OFStruct;
    short     ii;
    
    FidIn = GSSiOpenFile ("tiger1.f61",&OFStruct,OF_READ);
    FidOut = GSSiOpenFile ("tiger1.f71",&OFStruct,OF_CREATE);
    
    while (fgetstring (str,256,FidIn))
    {   
        if (!_fstrncmp (&str[19],"I- ",3))
            ii=0;
        else if (!_fstrncmp (&str[19],"State ",6))   
        {
            _fstrncpy (&str[55],"A21",3);
        }
        else if (!_fstrncmp (&str[19],"US ",3)) 
        {
            _fstrncpy (&str[55],"A21",3);
        }
        else
        {
            _fstrncpy (&str[55],"A31",3);
        }
        fputstring (str,FidOut);
    }
    return 0;
} 


void LoadAddressTable(void)
{   char    str[1024],Fname[128]="C:\\adddata",
            Name[128],cmd[256];
    FILE    *Fid;  
    OFSTRUCT    OFStruct;
    GWDHEADER GWDHead; 
    LPGWDHEADER lpGWDHead;
    GWFLDINFO GWFldInfo;
    LPGWFLDINFO lpGWFldInfo;
    HANDLE hBT, hVars, hDB;   
    long    Offset;
    LPSTR   lpVal; 
    short   l;
    
    HFILE   FidData;
    BTVARDESC  *pVars;
    short   len, NumIndexFields,i,ibeg,ifield,NumIndex;
    
    Fid = fopen ("c:\\gis.dat","r");
    if (!Fid)
    {
        MessageBox( GetFocus(), "Load Address","Unable to open data file", MB_OK);
        return;
    } 
              
    fgetss (str,1024,Fid);
              
              
    ProcessDelimTextHeader(str); 
    
     FidData = GSSiOpenFile ("c:\\adddata.gMd",&OFStruct,OF_CREATE); 
     if (FidData == HFILE_ERROR)
     {
        MessageBox( GetFocus(), " ","Unable to create data file", MB_OK);
        return;
     }
    GWDHead.NumFields=0;
    GWDHead.NumIndex=1;
    GWDHead.Version=1;   
    NumIndexFields = 1;
    GWDHead.NumIndexFields[0]=NumIndexFields;
    for (i=0;i<NumIndexFields;i++)
        GWDHead.IndexFields[0][i]=i;
    _lwrite (FidData,(char *)&GWDHead,sizeof(GWDHEADER));
    ibeg = 0;
                     
    hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumIndexFields * sizeof(BTVARDESC));
    pVars = (BTVARDESC *) LocalLock(hVars);
                     
    GWFldInfo.Len = 4;
    GWFldInfo.Beg = ibeg;
    GWFldInfo.Type = BT_INTEGER;
    pVars->BT_VARLEN=4;
    pVars->BT_VARTYP=BT_INTEGER;
    pVars->BT_VAROFF=0;
    ifield=0;
                     
    GWFldInfo.Type = BT_CHAR;
    GWFldInfo.Len = 8;
    GWFldInfo.Beg = ibeg;
    if (ifield<NumIndexFields)
    {
         pVars->BT_VARLEN=GWFldInfo.Len;
         pVars->BT_VARTYP=GWFldInfo.Type;
         pVars->BT_VAROFF=ibeg; 
         pVars++;
         GWDHead.lKeys[0]=ibeg+GWFldInfo.Len;
    }
                        
    ibeg += GWFldInfo.Len;
    _fstrcpy (GWFldInfo.Name,"ACCOUNT");
    _lwrite (FidData,(char *)&GWFldInfo,sizeof(GWFldInfo));
    GWDHead.NumFields++;     
    
    GWDHead.Reclen=ibeg;
    GWFldInfo.Type = BT_CHAR;
    GWFldInfo.Len = 7;
    GWFldInfo.Beg = ibeg;
                        
    ibeg += GWFldInfo.Len;
    _fstrcpy (GWFldInfo.Name,"PID");
    _lwrite (FidData,(char *)&GWFldInfo,sizeof(GWFldInfo));
    GWDHead.NumFields++;     
    
    GWDHead.Reclen=ibeg; 
    GWFldInfo.Type = BT_INTEGER;
    GWFldInfo.Len = 4;          
    GWFldInfo.Beg = ibeg;
                        
    ibeg += GWFldInfo.Len;
    _fstrcpy (GWFldInfo.Name,"%HOUSE_NUM");
    _lwrite (FidData,(char *)&GWFldInfo,sizeof(GWFldInfo));
    GWDHead.NumFields++;     
    
    GWDHead.Reclen=ibeg; 
    GWFldInfo.Type = BT_CHAR;
    GWFldInfo.Len = 32;
    GWFldInfo.Beg = ibeg;
                        
    ibeg += GWFldInfo.Len;
    _fstrcpy (GWFldInfo.Name,"%STREET_NAME");
    _lwrite (FidData,(char *)&GWFldInfo,sizeof(GWFldInfo));
    GWDHead.NumFields++;     
    
    GWDHead.Reclen=ibeg; 

    
     
    GWDHead.TimeStamp = time(0);
    _llseek (FidData,0,0);
    _lwrite (FidData,(char *)&GWDHead,sizeof(GWDHEADER));
    _llseek (FidData,0,2);
                     
    NumIndex = 1;
                
                
    _fstrcpy (str,Fname);
    _fstrcat (str,".in1");
    LocalUnlock(hVars);
    pVars =(BTVARDESC *)  LocalLock(hVars);
    BT_CREATE (str, 4, FALSE, NumIndexFields, 1,pVars,FALSE, 0, GWDHead.TimeStamp, FALSE);
    pVars =(BTVARDESC *) LocalUnlock(hVars);
    LocalFree(hVars);
    _lclose (FidData);
                     
    _fstrcpy (Name,Fname);
    _fstrcat (Name,".gmd");
    hDB = OpenGWDatabase (Name,BT_WRITE);
    if (!hDB) return;
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
    hBT = lpGWDHead->BTHandle[0]; 
             
    while (fgetss (str,1024,Fid)) 
    {
        GetDelimTextData(str);
        Offset = _llseek (lpGWDHead->Fid,0,2);
        for (i=0,lpGWFldInfo=lpGWDHead->pFldInfo;i<lpGWDHead->NumFields;
             i++,lpGWFldInfo++)
        {   
            switch (i)
            {
                case 0:
                    GetVal("ACCOUNT",cmd);
                    break;
                case 1:
                    GetVal("PID",cmd);
                    break;
                    
                case 2:   
                    GetVal("HOUSE_NUM",cmd);
                    break;
                    
                case 3:
                    GetVal("STREET_NAME",cmd);
                    break;
                }   
                    
            len=_fstrlen(cmd);
            lpVal = &lpGWDHead->GWDData[lpGWFldInfo->Beg];
/*          _fstrncpy(cmd,val,len);*/
            switch (lpGWFldInfo->Type)
            {
                case BT_CHAR:
                    _fstrncpy (lpVal,cmd,lpGWFldInfo->Len);
                break;
                                
                case BT_RIGHT_CHAR:
                    _fmemset (lpVal,' ', lpGWFldInfo->Len);
                    l = _fstrlen (cmd);
                    lpVal+= max(lpGWFldInfo->Len-l,0);
                    _fmemmove (lpVal,cmd,max(l,lpGWFldInfo->Len));
                break;
                                
                case BT_INTEGER:
                    if (lpGWFldInfo->Len == 2)
                        *(LPINT)lpVal=atoi(cmd);
                    else
                        *(LPLONG)lpVal=atol(cmd);
                break;
                                
                case BT_REAL:
                    if (lpGWFldInfo->Len == 4)
                        *(LPFLOAT)lpVal= (float)atof(cmd);
                    else
                        *(LPDOUBLE)lpVal=atof(cmd);
                break;
            }
        }
        lpVal = &lpGWDHead->GWDData[lpGWDHead->pFldInfo->Beg];
        BT_PUT (hBT,(LPSTR)lpVal,(LPSTR)&Offset);
        _lwrite (lpGWDHead->Fid,(char *)&lpGWDHead->Reclen,2);
        _lwrite (lpGWDHead->Fid,(char *)&lpGWDHead->GWDData,(UINT)lpGWDHead->Reclen);
     }
                      
     GlobalUnlock (hDB);
     CloseGWDatabase (hDB);
    fclose (Fid);
    return;

}  
     
long GetNearCityx (DPOINT DPoint,long MinPop,LPSTR InCity,LPDOUBLE MinDist, LPDOUBLE MinAZ)
{
	double	Dist; 
	HANDLE	hBTCities1, hBTCities2;  
	HFILE	Cities4FID;
	int		pos=BT_FIRST;  
	DPOINT	DPoint2;
	CITIESKEY1	CitiesKey1; 
	CITIESDATA1	CitiesData1;
	CITIESDATA4	CitiesData4;
	CITIESKEY2	CitiesKey2, MinKey;
	OFSTRUCT	OFStruct;
	char		Name[64], StateID[4];  
	HANDLE		hBuf;
	LPCITIESDATA4	pBuf, pCitiesData4;
	int			nbuf; 
	long		lbuf = 500*sizeof(CitiesData4), lenread;
	
	hBuf = GSSiGlobAlloc (GMEM_MOVEABLE,lbuf);
	pBuf = GlobalLock (hBuf);
	
	*MinDist=DBL_MAX;	
	Cities4FID = GSSiOpenFile ("cities4.dat",&OFStruct,OF_READ); 
	while((lenread=_lread (Cities4FID,pBuf,lbuf)))
	{   
		nbuf = lenread/sizeof(CitiesData4);  
		pCitiesData4 = pBuf;
		while (nbuf--)
		{
			if (pCitiesData4->Pop < MinPop)
				goto Exit;   
			Dist = GetBaseDist (&DPoint,&pCitiesData4->LatLong)*MFT;
//			Dist = ldistp (DPoint,pCitiesData4->LatLong)*MFT;
			if (Dist < *MinDist)
			{
				*MinDist = Dist;
				MinKey.Pop = pCitiesData4->Pop;
				*MinAZ = getazd (pCitiesData4->LatLong,DPoint);  
				_fstrcpy (Name,pCitiesData4->Name);
				_fstrcpy (StateID,pCitiesData4->State);
			} 
			pCitiesData4++;
		}
	} 
Exit:   
	GlobalUnlock (hBuf);
	GlobalFree (hBuf);
	_lclose (Cities4FID);
	sprintf (InCity,"%s, %s",Name,StateID);
/*
	MinPop = -MinPop;
	hBTCities2 = BT_OPEN ("cities2.btr",0,BT_READ,0); 
	while(!BT_FIND (hBTCities2,(LPSTR)&CitiesKey2,pos,BT_ANY,(LPSTR)&DPoint2))
	{   
		if (CitiesKey2.Pop > MinPop)
			goto Exit;
		Dist = ArcDistance (DPoint,DPoint2)*MFT;
		if (Dist < *MinDist)
		{
			*MinDist = Dist;
			MinKey = CitiesKey2;
			*MinAZ = getazd (DPoint2,DPoint); 
		}
		pos = BT_NEXT; 
	} 
Exit:
	BT_CLOSE (hBTCities2); 
	
	CitiesKey1.State = MinKey.State;
	CitiesKey1.FIPS = MinKey.FIPS;
	hBTCities1 = BT_OPEN ("cities1.btr",0,BT_READ,0); 
	if (!BT_FIND (hBTCities1,(LPSTR)&CitiesKey1,BT_FIRST,BT_EQ,(LPSTR)&CitiesData1))
	{
		char	StateID[8];
		
		GetStateID (CitiesKey1.State,StateID);
		sprintf (InCity,"%s, %s",CitiesData1.Name,StateID);
	}
	else
		*InCity = 0;
	BT_CLOSE (hBTCities1);*/ 
	 
	
	return (MinKey.Pop);
}

from ZOOMLISTMsgProc in graphic3.c
    case WM_MEASUREITEM: 
 
        lpmis = (LPMEASUREITEMSTRUCT) lParam; 
 
        /* Set the height of the list box items. */ 
 
//        GetTextMetrics(lpdis->hDC, &tm); 
//        lpmis->itemHeight = tm.tmHeight; 
        return FALSE; 
 
    case WM_DRAWITEM: 
 
        lpdis = (LPDRAWITEMSTRUCT) lParam; 
 
        /* If there are no list box items, skip this message. */ 
 
        if (lpdis->itemID == -1) { 
            break; 
        } 
 
        /* 
         * Draw the bitmap and text for the list box item. Draw a 
         * rectangle around the bitmap if it is selected. 
         */ 
 
        switch (lpdis->itemAction) { 
 
            case ODA_SELECT: 
            case ODA_DRAWENTIRE: 
 
                /* Display the bitmap associated with the item. */ 
 
 
                
                Point.x=lpdis->rcItem.left + 20;
                Point.y=(lpdis->rcItem.top + lpdis->rcItem.bottom)/2;
						   
 
                /* Display the text associated with the item. */ 
 
                ii=SendDlgItemMessage(hWndDlg,IDC_ZOOM_AREAS, CB_GETLBTEXT, 
                    lpdis->itemID, (LPARAM) str); 
 
                GetTextMetrics(lpdis->hDC, &tm); 
 
                y = (lpdis->rcItem.bottom + lpdis->rcItem.top - 
                    tm.tmHeight) / 2; 
                
                if (lpdis->itemState & ODS_SELECTED) { 
                   hBr = CreateSolidBrush (GetSysColor(COLOR_HIGHLIGHT));
	 			   SetTextColor (lpdis->hDC,GetSysColor (COLOR_HIGHLIGHTTEXT));                 
                }     
                else
                   hBr = CreateSolidBrush (GetSysColor(COLOR_MENU));
                    /* 
                     * Draw a rectangle around bitmap to indicate 
                     * the selection. 
                     */ 
                    
//                   	DrawFocusRect(lpdis->hDC, &LastRect);
//                    DrawFocusRect(lpdis->hDC, &lpdis->rcItem);
				FillRect(lpdis->hDC, &lpdis->rcItem,hBr);  
				DeleteObject (hBr); 
                if ((lpBar = _fstrchr (str,'\t')))
                	*lpBar = 0;
                TextOut(lpdis->hDC, 
                    lpdis->rcItem.left, 
                    y, 
                    str, 
                    strlen(str)); 
 
//                SelectObject(hdcMem, hbmpOld); 
//                DeleteDC(hdcMem); 
 
                /* Is the item selected? */ 
 
                break; 
 
            case ODA_FOCUS: 
                InvertRect(lpdis->hDC, &lpdis->rcItem); 
 
                break; 
        } 
        return TRUE; 
 



void TIGEROut(void)
{
    long    Num, TLID, Frame, Offset;
    char    StreetName[34], str[128], OutRec[256], OutRec2[256];
    short      st, st2, st3, stn,  Version, hltpos, PointListID, i;
    OFSTRUCT    OFStruct;
    HCURSOR hcurSave; 
    BOOL    Good, FirstMap=TRUE;
    long    NewFileMarker=LONG_MIN;  
    HFILE   FidTIGER1, FidTIGER2, hData=0; 
    HANDLE  hBT=0;
    DPOINT  FromPoint, ToPoint;
    TIGER1_PEOPLENET    Segdata, Segdata2;
    HIGHLIGHTDATA       HighlightData;
    HANDLE hSymbol, hSkipTLID, hIntersect=0;   
    LPSYMBOL    pSymbol;
    short     HaveState=-1, idum, irec, ninrec;
    HFILE   Fid2;
    BTVARDESC   BTVar[3];
    long    snum, IntLong, IntLat, IntTLID, ToLong, ToLat, AtTLID, WantStreet;
    struct {
            long ref; 
            short Layer, State; 
            DPOINT Point; 
            double AZ, Length;
            } MidPoint; 
    struct  {
                long Long, Lat, TLID;
            }   IntersectKey;   
    struct  {   short     State;
                long    StreetNum;
            }   StreetKey;
    typedef struct {long x,y;} LONGPOINT;
    typedef LONGPOINT FAR   *LPLONGPOINT;
    LPLONGPOINT PointList, TotPoints;
    short NumPoints[2], NumShapePoints, pos;
    HANDLE  hPointList[2], hTotPoints, hStreets, hTLID; 
    long    TotLen, Loc, Nrecs=0;  
    BOOL    FoundState[66];
    char mess[144];
    BTHEAD BTHead;    
    
    _fmemset (FoundState,0,66*sizeof(BOOL));
    Fid2 = GSSiOpenFile ("extract.lst",&OFStruct,OF_READ);
    hcurSave = SetCursor(LoadCursor(0, IDC_WAIT));


    BTVar[0].BT_VARTYP=BT_INTEGER;
    BTVar[0].BT_VARLEN=2;
    BTVar[0].BT_VAROFF=0;
    BTVar[1].BT_VARTYP=BT_INTEGER;
    BTVar[1].BT_VARLEN=4;
    BTVar[1].BT_VAROFF=2;
    BT_CREATE ("streets.btr", 2, FALSE, 2, 1,(LPBTVARDESC)&BTVar,FALSE, 0, 0, FALSE);
    hStreets= BT_OPEN ("streets.btr", 0, BT_WRITE, 0);
    BTVar[0].BT_VARTYP=BT_INTEGER;
    BTVar[0].BT_VARLEN=4;
    BTVar[0].BT_VAROFF=0;
    BT_CREATE ("tlids.btr", 2, FALSE, 1, 1,(LPBTVARDESC)&BTVar,FALSE, 0, 0, FALSE);
    hTLID= BT_OPEN ("tlids.btr", 0, BT_WRITE, 0);
    BTVar[0].BT_VARTYP=BT_INTEGER;
    BTVar[0].BT_VARLEN=4;
    BTVar[0].BT_VAROFF=0;
    BT_CREATE ("skip.btr", 2, FALSE, 1, 1,(LPBTVARDESC)&BTVar,FALSE, 0, 0, FALSE);
    hSkipTLID= BT_OPEN ("skip.btr", 0, BT_WRITE, 0);
    FidTIGER1 = GSSiOpenFile ("tiger1.txt",&OFStruct,OF_CREATE);
    FidTIGER2 = GSSiOpenFile ("tiger2.txt",&OFStruct,OF_CREATE);
    Good = FALSE; 
    TotLen = _llseek (Fid2,0,2);
    _llseek (Fid2,0,0);
    while (_lread (Fid2,&MidPoint,sizeof(MidPoint)) == sizeof(MidPoint))
    {   
        Nrecs++;
        if (Nrecs%1000)
        {
            sprintf (mess,"%5.1f  State = %i",100*((double)_llseek(Fid2,0,1))/TotLen,CurState);
            SetWindowText (hWndMain,mess);
        }
        if (!MidPoint.Layer)
            goto NextTLID; 
//      if (Nrecs > 300)
//          goto EndTlids;
        BT_PUT (hTLID,(LPSTR)&MidPoint.ref,(LPSTR)&MidPoint.State);  
        CurState = MidPoint.State;
        FoundState[CurState]=TRUE;
        TLID = MidPoint.ref;
        if (HaveState != CurState)
        {   
            char    str[8], Name[64];
                
            if (hData)
                _lclose (hData); 
            hData = 0;
            BT_CLOSE (hBT);     
            if (!(hBT = BT_OPEN ("[STATE]\\tiger1.btr", 0, BT_READ, 0)))
                goto NextTLID;
            hData = GSSiOpenFile ("[STATE]\\tiger1.dat",&OFStruct,OF_READ);
            CloseGSStreetNames();
            OpenGSStreetNames (BT_READ,3); 
            BT_FIND (hNames2,(LPSTR)Name,BT_FIRST,BT_ANY,(LPSTR)&BlankStreet);
            HaveState = CurState;
        }
        BT_FIND(hBT,(LPSTR)&TLID,BT_FIRST,BT_EQ,(LPSTR)&Offset);
        _llseek (hData,Offset,0);
        _lread (hData,&Segdata,sizeof(Segdata));
        StreetKey.StreetNum = Segdata.StreetNum;
        StreetKey.State = CurState;  
        if (Segdata.StreetNum != BlankStreet)
            BT_PUT (hStreets,(LPSTR)&StreetKey,(LPSTR)&idum);  
    }
EndTlids: 
    _lclose (Fid2); 
    for (CurState = 1;CurState<66;CurState++) 
    {
        if (FoundState[CurState])
        {    
            sprintf (mess,"Scanning state %i",CurState);
            SetWindowText (hWndMain,mess);
            if (hData)
                _lclose (hData); 
            hData = 0;
            hData = GSSiOpenFile ("[STATE]\\tiger1.dat",&OFStruct,OF_READ); 
            while (_lread (hData,&Segdata,sizeof(Segdata)) == sizeof(Segdata))
            {
                StreetKey.StreetNum = Segdata.StreetNum;
                StreetKey.State = CurState;
                if (!BT_FIND (hStreets,(LPSTR)&StreetKey,BT_FIRST,BT_EQ,(LPSTR)&idum))
                    BT_PUT (hTLID,(LPSTR)&Segdata.TLID,(LPSTR)&StreetKey.State);
            } 
        }
    }   
    hPointList[0] = GSSiGlobAlloc (GMEM_MOVEABLE,500*sizeof(LONGPOINT));
    hPointList[1] = GSSiGlobAlloc (GMEM_MOVEABLE,500*sizeof(LONGPOINT));
    hTotPoints = GSSiGlobAlloc (GMEM_MOVEABLE,1000*sizeof(LONGPOINT));  
    HaveState=-1;
    Nrecs = 0;
    GetBTHeader (hTLID,&BTHead);  
    TotLen = BTHead.BT_NUMRECS;   
    pos = BT_FIRST;
    while (!BT_FIND (hTLID,(LPSTR)&MidPoint.ref,pos,BT_ANY,(LPSTR)&MidPoint.State))
    {   
        pos = BT_NEXT;
        Nrecs++;
        if (Nrecs%1000)
        {
            sprintf (mess,"%5.1f  State = %i",100*((double)Nrecs)/TotLen,CurState);
            SetWindowText (hWndMain,mess);
        }
        if (!BT_FIND (hSkipTLID,(LPSTR)&MidPoint.ref,BT_FIRST,BT_EQ,(LPSTR)&idum))
            goto NextTLID; 
        BT_PUT (hSkipTLID,(LPSTR)&MidPoint.ref,(LPSTR)&idum);
        CurState = MidPoint.State;
        TLID = MidPoint.ref;
        if (HaveState != CurState)
        {   
            char    str[8];
                
            if (hData)
                _lclose (hData); 
            hData = 0;
            BT_CLOSE (hBT);     
            BT_CLOSE (hIntersect);
            if (!(hBT = BT_OPEN ("[STATE]\\tiger1.btr", 0, BT_READ, 0)))
                goto NextTLID;
            hData = GSSiOpenFile ("[STATE]\\tiger1.dat",&OFStruct,OF_READ);
            hIntersect = BT_OPEN ("[STATE]\\intersec.btr",0, BT_READ, 0);
            CloseGSStreetNames();
            OpenGSStreetNames (BT_READ,3);
            HaveState = CurState;
        }
        BT_FIND(hBT,(LPSTR)&TLID,BT_FIRST,BT_EQ,(LPSTR)&Offset);
        _llseek (hData,Offset,0);
        _lread (hData,&Segdata,sizeof(Segdata));
        
        WantStreet = Segdata.StreetNum;
        ToLong = Segdata.TOLONG; 
        ToLat = Segdata.TOLAT; 
        IntLong = Segdata.FRLONG;
        IntLat = Segdata.FRLAT;  
        PointListID = 0;
NextLink:
        AtTLID = TLID;   
        NumPoints[PointListID] = 1;  
        PointList = (LPLONGPOINT) GlobalLock (hPointList[PointListID]);
NextPoint:
        BT_PUT (hSkipTLID,(LPSTR)&AtTLID,(LPSTR)&idum);
        PointList->x = IntLong;
        PointList->y = IntLat;
        if (NumPoints[PointListID] >=50)
            goto EndLink;
        IntersectKey.Long = IntLong; 
        IntersectKey.Lat = IntLat; 
        IntersectKey.TLID = 0;  
        st = BT_FIND (hIntersect,(LPSTR)&IntersectKey,BT_FIRST,BT_GT,(LPSTR)&snum);
        while (!st)
        {
            if (IntersectKey.Long != IntLong || IntersectKey.Lat != IntLat)
                goto EndLink;
            if (IntersectKey.TLID != AtTLID && snum == WantStreet)
            {
                BT_FIND (hBT,(LPSTR)&IntersectKey.TLID,BT_FIRST,BT_EQ,(LPSTR)&Offset);
                _llseek (hData,Offset,0);
                _lread (hData,&Segdata2,sizeof(TIGER1_PEOPLENET)); 
                if (Segdata2.FRLONG == IntLong && Segdata2.FRLAT == IntLat)
                {
                    IntLong = Segdata2.TOLONG;
                    IntLat = Segdata2.TOLAT;
                } 
                else
                {
                    IntLong = Segdata2.FRLONG;
                    IntLat = Segdata2.FRLAT;
                } 
                AtTLID = IntersectKey.TLID;
                NumPoints[PointListID]++;    
                PointList++;
                goto NextPoint;
            } 
            st = BT_FIND (hIntersect,(LPSTR)&IntersectKey,BT_NEXT,BT_ANY,(LPSTR)&snum);
        } 
EndLink:
        if (!PointListID)
        {   
            GlobalUnlock (hPointList[PointListID]);
            PointListID++;
            IntLong = ToLong;
            IntLat = ToLat;  
            goto NextLink;
        }
        if (BT_FIND (hNames1,(LPSTR)&Segdata.StreetNum,BT_FIRST,BT_EQ,(LPSTR)&StreetName))
            _fstrncpy(StreetName,"",34);  
        _fmemset (OutRec,' ',230);
        OutRec[0]='1';
        OutRec[4]='0';
        sprintf (str,"%10ld",TLID);
        _fmemmove (&OutRec[5],str,_fstrlen(str));
        _fmemmove (&OutRec[19],StreetName,_fstrlen(StreetName)); 
            
        _fmemmove (&OutRec[55],"A11",3); 
        sprintf (str,"%2.2i%2.2i",CurState,CurState);
        _fmemmove (&OutRec[130],str,_fstrlen(str)); 
        
        PointList = (LPLONGPOINT)GlobalLock (hPointList[0]);
        PointList+=(NumPoints[0]-1);
        Segdata.FRLONG = PointList->x;
        Segdata.FRLAT = PointList->y;
        NumShapePoints = 0; 
        TotPoints =(LPLONGPOINT) GlobalLock (hTotPoints);
        NumPoints[0]--;
        while (NumPoints[0]--)
        {
            PointList--;
            *TotPoints++ = *PointList;
            NumShapePoints++;
        }
        GlobalUnlock (hPointList[1]);

        PointList = (LPLONGPOINT)GlobalLock (hPointList[1]);
        NumPoints[1]--;
        while (NumPoints[1]--)
        {
            *TotPoints++ = *PointList;
            NumShapePoints++;
            PointList++;
        }
        Segdata.TOLONG = PointList->x;
        Segdata.TOLAT = PointList->y;
        GlobalUnlock (hPointList[1]); 
        GlobalUnlock (hTotPoints);
        sprintf (str,"%10ld%9ld%10ld%9ld", 
                    Segdata.FRLONG,
                    Segdata.FRLAT,
                    Segdata.TOLONG,
                    Segdata.TOLAT);
        _fmemmove (&OutRec[190],str,_fstrlen(str));
        OutRec[228]=0;
        fputstring(OutRec,FidTIGER1);   
        
        _fmemset (OutRec2,' ',230); 
        OutRec2[0]='2';
        OutRec2[4]='0';
        irec=1;
        sprintf (str,"%10ld%3i",TLID,irec); 
        _fmemmove (&OutRec2[5],str,_fstrlen(str));
                    
        TotPoints = (LPLONGPOINT)GlobalLock (hTotPoints);  
        ninrec = 0;
        for (i=0;i<NumShapePoints;i++,TotPoints++)
        {   
            BOOL    HaveRec=FALSE;
                        
            sprintf (str,"%10ld%9ld",TotPoints->x,TotPoints->y);
            _fmemmove (&OutRec2[ninrec*19+18],str,_fstrlen(str)); 
            ninrec++;
            if (ninrec==10)
            {
                OutRec2[208]=0;
                fputstring(OutRec2,FidTIGER2);
                _fmemset (OutRec2,' ',230);
                OutRec2[0]='2';
                OutRec2[4]='0';
                irec++;
                sprintf (str,"%10ld%3i",TLID,irec); 
                _fmemmove (&OutRec2[5],str,_fstrlen(str));
                ninrec = 0;
            }
                        
        }
        if (ninrec)
        {
            OutRec2[208]=0;
            fputstring(OutRec2,FidTIGER2);
        } 
        GlobalUnlock (hTotPoints);
        
        
NextTLID:;          
    } 
    Good = TRUE;
Exit: 
    GlobalFree (hPointList[0]);
    GlobalFree (hPointList[1]);
    GlobalFree (hTotPoints);
    BT_CLOSE (hSkipTLID);
    _lclose(FidTIGER1);
    _lclose(FidTIGER2); 
    BT_CLOSE (hTLID);
    BT_CLOSE (hStreets);
    remove ("tlids.btr");
    remove ("streets.btr");
    if (hData)
        _lclose (hData); 
    hData = 0;
    BT_CLOSE (hBT);
    CloseGSStreetNames();
    BT_CLOSE (hIntersect);
    SetCursor (hcurSave); 
    if (Good)
        MessageBox( GetFocus(), "TIGER Extract Complete","", MB_OK);
    else
        MessageBox( GetFocus(), "Extract Aborted","", MB_OK); 
    DoPaint = TRUE;
    return;
}

