#include "graphint.h"

#include "gmextern.h"

BOOL LoadTravTAGs (HWND hWndDlg,UINT IDCTAPREFIX)
{
	HFILE Fid=GSSiOpenFile ("[%DL]travtags.txt",0,OF_READ);
	char	str[260]; 
	LPSTR	pComma;
	
	if (Fid == HFILE_ERROR)
	{
		MessageBox (0,"travtags.txt file not found",0,MB_ICONEXCLAMATION);
		return FALSE;
	}
	while (fgetstring (str,256,Fid))
	{   
		if (*str == '#')
			continue;
		if ((pComma = _fstrchr (str,',')))
			*pComma = 0;
    	SendDlgItemMessage (hWndDlg,IDCTAPREFIX,CB_ADDSTRING,0,(LPARAM)str);
     }
     GSSiClose2 (&Fid);
     return TRUE;
}

BOOL TravGetDefaultSymbol (LPSTR CurTAG,LPSTR SymName,int Which)
{   
	HFILE	Fid; 
	LPSTR	pComma;
	char	line[260];    
	BOOL	rtn=FALSE;
			
	_fstrcpy (SymName,"");
	Fid = GSSiOpenFile("[%DL]travtags.txt",0,OF_READ);
	if (Fid != HFILE_ERROR)
	{   
		while (fgetstring (line,256,Fid))
		{   
			if (*line == '#')
				continue;
			if ((pComma=_fstrrchr (line,',')))
			{
				*pComma++ = 0;   
				rtn = atob (pComma);
			}
			if ((pComma=_fstrchr (line,','))) 
			{
				*pComma++=0; 
				_fstrcpy (SymName,pComma);
				if (Which > 1)
				{
					if ((pComma=_fstrchr (pComma,','))) 
					{
						*pComma++=0; 
						_fstrcpy (SymName,pComma);
						if (Which > 2) 
						{
							if ((pComma=_fstrchr (pComma,','))) 
							{
								*pComma++=0; 
								_fstrcpy (SymName,pComma); 
							}
						}
					}
				}
			} 
			if ((pComma=_fstrchr (SymName,','))) 
				*pComma++=0; 
			
			if (!_fstricmp (CurTAG,line))
				break;
		}
		GSSiClose2 (&Fid);
	}  
	return rtn;
} 

BOOL TravAllowUpdateLayer (LPSTR CurTAG)
{   
	HFILE	Fid; 
	LPSTR	pComma;
	char	line[260];  
	BOOL	rtn=FALSE;
			
	Fid = GSSiOpenFile("[%DL]travtags.txt",0,OF_READ);
	if (Fid != HFILE_ERROR)
	{   
		while (fgetstring (line,256,Fid))
		{   
			if (*line != '#')
			{
				if ((pComma=_fstrrchr (line,',')))
					*pComma++ = 0; 
				rtn = atob (pComma);
				if ((pComma=_fstrchr (line,','))) 
					*pComma++=0; 
				if (!_fstricmp (CurTAG,line))
					break; 
			}
		}
		GSSiClose2 (&Fid);
	}  
	return rtn;
} 

double GetAzAdjustment (DPOINT WPoint)
{   
	DPOINT	Point1, Point2;
	
	double Adjust; 
	
	ConvertCoord(&WPoint,1,3); 
	
	Point1 = Point2 = WPoint;
	Point1.y -= 0.25;
	Point2.y += 0.25;
	ConvertCoord(&Point1,3,1); 
	ConvertCoord(&Point2,3,1); 
	Adjust = getazd (&Point1,&Point2)-HALFPI;
	return Adjust;
}







double atobasedist (LPSTR Dist,LPBOOL perr)
{
    double	DistVal,OutDist;
    short	Units;
    
    *perr = 0;
	GetDistAndUnits (Dist,&DistVal,&Units,TRUE);
	OutDist = ConvertInDist (DistVal,Units+1);
	return OutDist;
}


LPSTR GetDistAndUnits (LPSTR pParm,LPDOUBLE pDist,LPSHORT pCurDistUnits,BOOL UseDefault)
{   
	short i, lun; 
	char	savec;
	LPSTR	punits, pSave; 
	HANDLE	hTemp=0;
	long	l;
	LPSTR	pTemp; 
	LPSTR	pComma=0;
	
	l = _fstrlen (pParm);
	if (l)
	{
		hTemp = GSSiGlobAlloc(GAIDNO 1079,GMEM_MOVEABLE,l+8);
		pTemp = GlobalLock (hTemp);
		_fstrcpy (pTemp,pParm);
		_fstrupr (pTemp);
		pParm = pTemp; 
		punits = FirstAlpha (pParm);
		if (punits)
		{ 
			if ((pComma = _fstrchr (punits,',')))
				*pComma = 0;
			lun = _fstrlen(punits); 
			if (lun)
			for (i=0;i<6;i++)
			{
				if (!_fstrnicmp (punits,DistUnitOpts[i],lun) || (!i && !_fstricmp (punits,"FOOT")))
				{   
					if (i == 5)
						i = PRJ_UNITS[1]-1;
					*pCurDistUnits=i;  
					savec = *punits; 
					pSave = punits;
					*punits = 0;
					punits += lun;
					*pDist = atof (pParm); 
					*pSave = savec;  
					if (pComma)
						*pComma = ',';
					GSSiGlobUlFree (&hTemp);
					return punits;
				}
			}
		}
	} 
	if (UseDefault)
	{
		if (PRJ_UNITS[1] == 4)
			*pCurDistUnits = 1;
		else
			*pCurDistUnits = PRJ_UNITS[1]-1;
	}
	else
		*pCurDistUnits = -1;
	*pDist = atof (pParm);
	if (pComma)
		*pComma = ',';
	GSSiGlobUlFree (&hTemp);
	return 0;
}    

double BearToAZ (char PreDir,LPSTR pDeg,LPSTR pMin,LPSTR pSec,char PostDir)
{   
	double AZ;
	double	DEG,MIN,SEC;
	BOOL	NORTH,EAST;
	
    switch (PreDir)
    {   
    	default:
    	case 'n': 
    	case 'N':
    		NORTH = TRUE;
    		break;
    	case 's':
    	case 'S':
    		NORTH = FALSE;
    		break;
    }
    switch (PostDir)
    {   
    	default:
    	case 'e':
    	case 'E':
    		EAST = TRUE;
    		break;
    	case 'w':
    	case 'W':
    		EAST = FALSE;
    		break;
    }
    DEG = atof (pDeg);
    MIN = atof (pMin); 
    SEC = atof (pSec);
	AZ = DEG + MIN / 60E0 + SEC / 3600E0;
	if (NORTH && EAST)
		AZ+=0;
	else if (NORTH && !EAST)
	  AZ = 360E0 - AZ; 
	else if (!NORTH && EAST)
	  AZ = 180E0 - AZ; 
	else if (!NORTH && !EAST) 
	  AZ = 180E0 + AZ;
	AZ *= RADDEG; 
	AZ = LTWOPI(HALFPI-AZ);
	return AZ;
}

BOOL DecodeBANDDParm (LPSTR Parm,LPDOUBLE pDist,LPDOUBLE pAZ,LPSHORT pDir,short Type)
{ 
	char	str[256], PreDir,PostDir,DegC[32],MinC[32],SecC[32]; 
	LPSTR	pParm, pEnd; 
	double	Dist,AZ;    
	short	inc;
	
	_fstrcpy (str,Parm);
	pParm = _fstrstr (str,"BEAR=");
	if (pParm)
	{
		pParm+=5;
		PreDir = *pParm++;
		pEnd = _fstrchr(pParm,'D');
		*pEnd++ = 0;
		_fstrcpy (DegC,pParm); 
		pParm = pEnd;
		pEnd = _fstrchr(pParm,'M');
		*pEnd++ = 0;
		_fstrcpy (MinC,pParm);
		pParm = pEnd;
		pEnd = _fstrchr(pParm,'S');
		*pEnd++ = 0; 
		pEnd++;
		_fstrcpy (SecC,pParm);  
		pParm = pEnd;
		pEnd++;
		*pEnd=0;
		PostDir = *pParm;
		*pAZ = BearToAZ (PreDir,DegC,MinC,SecC,PostDir);
	}
	else
		return FALSE;
	_fstrcpy (str,Parm);
	switch (Type)
	{
		default:
			pParm = _fstrstr (str,"DIST="); 
			inc = 5; 
			break;
		case 22:
			pParm = _fstrstr (str,"RADIUS=");
			inc = 7;  
			break;
	}
	if (pParm)
	{   
		short	CurDistUnits;
		
		pParm+=inc;
		pParm = GetDistAndUnits (pParm,&Dist,&CurDistUnits,TRUE);  
		*pDist = ConvertInDist (Dist,CurDistUnits+1); 
	}
	else
		return FALSE;
	_fstrcpy (str,Parm);
	pParm = _fstrstr (str,"DIR=");  
	if (pParm)
	{   
		short	CurDistUnits;
		
		pParm+=4; 
		if (!_fstrnicmp (pParm,"Right",5))
			*pDir = 1; 
		else
			*pDir = 2;
	}
	return TRUE;
}  

BOOL DecodeTYPE1CRVParm (LPSTR Parm,LPDOUBLE pRadius,LPDOUBLE pDist,LPDOUBLE pAZ,LPSHORT pDir,LPSHORT pDistOpt,LPSHORT pBearOpt)
{ 
	char	str[256], PreDir,PostDir,DegC[32],MinC[32],SecC[32]; 
	LPSTR	pParm, pEnd; 
	double	Dist,AZ;    
	short	inc;
	
	_fstrcpy (str,Parm);
	pParm = _fstrstr (str,"BEAR=");
	if (pParm)
	{
	 	if (!_fstrnicmp ((pParm-3),"TAN",3))
			*pBearOpt = 1;
	 	else if (!_fstrnicmp ((pParm-3),"RAD",3))
			*pBearOpt = 2;
		else
			*pBearOpt = 3;
		pParm+=5;
		PreDir = *pParm++;
		pEnd = _fstrchr(pParm,'D');
		*pEnd++ = 0;
		_fstrcpy (DegC,pParm); 
		pParm = pEnd;
		pEnd = _fstrchr(pParm,'M');
		*pEnd++ = 0;
		_fstrcpy (MinC,pParm);
		pParm = pEnd;
		pEnd = _fstrchr(pParm,'S');
		*pEnd++ = 0; 
		pEnd++;
		_fstrcpy (SecC,pParm);  
		pParm = pEnd;
		pEnd++;
		*pEnd=0;
		PostDir = *pParm;
		*pAZ = BearToAZ (PreDir,DegC,MinC,SecC,PostDir);
	}
	else
		return FALSE;
	_fstrcpy (str,Parm);
	pParm = _fstrstr (str,"DIST="); 
	inc = 5; 
	if (pParm)
	{   
		short	CurDistUnits;
		
	 	if (!_fstrnicmp ((pParm-3),"ARC",3))
			*pDistOpt = 1;
    	else if (!_fstrnicmp ((pParm-3),"TAN",3))
			*pDistOpt = 2;
		else 
			*pDistOpt = 3;
		pParm+=inc;
		pParm = GetDistAndUnits (pParm,&Dist,&CurDistUnits,TRUE);  
		*pDist = ConvertInDist (Dist,CurDistUnits+1); 
	}
	else
		return FALSE;
	pParm = _fstrstr (str,"RADIUS=");
	inc = 7;  
	if (pParm)
	{   
		short	CurDistUnits;
		
		pParm+=inc;
		pParm = GetDistAndUnits (pParm,&Dist,&CurDistUnits,TRUE);  
		*pRadius = ConvertInDist (Dist,CurDistUnits+1); 
	}
	else
		return FALSE;
	_fstrcpy (str,Parm);
	pParm = _fstrstr (str,"DIR=");  
	if (pParm)
	{   
		short	CurDistUnits;
		
		pParm+=4; 
		if (!_fstrnicmp (pParm,"Right",5))
			*pDir = 1; 
		else
			*pDir = 2;
	}
	return TRUE;
} 

BOOL ComputeTYPE1CRV (double Radius,double Dist,double AZ,short Dir,short DistOpt,short BearOpt,DPOINT PC,
					  LPDPOINT pPOC, LPDPOINT pPT,LPDPOINT pRP,
					  LPDOUBLE pArcDist,LPDOUBLE pTanDist,LPDOUBLE pChordDist,
					  LPDOUBLE pTanAZ, LPDOUBLE pRPAZ, LPDOUBLE pCrdAZ,LPDOUBLE pAngle)
{   
	short	rc;      
	double	clen;
	
	if (!Radius)
	{
    	MessageBox(GetFocus(),"Invalid radius",0,MB_ICONEXCLAMATION); 
		return FALSE;
	}
	switch (DistOpt)
	{
		case 1:
			*pArcDist = Dist;
			*pAngle = *pArcDist/Radius;
			break; 
		case 2:  
			*pTanDist = Dist; 
			*pAngle = atan2 (*pTanDist,Radius) * 2;
			break;
		case 3: 
			*pChordDist = Dist;
			*pAngle = asin ((Dist/2)/Radius) * 2; 
			break;
	}
	switch (BearOpt)
	{   
		case 1:
			if (Dir == 1)
				*pRPAZ = LTWOPI(AZ-HALFPI);	
			else
				*pRPAZ = LTWOPI(AZ+HALFPI);
			break;	 
		
		case 2:
			*pRPAZ = AZ; 
			break;
				 
		case 3:
			if (Dir == 1) 
			{ 
				AZ = LTWOPI(AZ-HALFPI);
				*pRPAZ = LTWOPI(AZ+*pAngle/2);
			}	
			else
			{ 
				AZ = LTWOPI(AZ+HALFPI);
				*pRPAZ = LTWOPI(AZ-*pAngle/2);
			}	
			break;	 
	} 
	*pArcDist = *pAngle * Radius; 
	*pTanDist = tan (*pAngle/2) * Radius;
	*pChordDist = sin (*pAngle/2) * Radius * 2;
	*pRP = dnewpt (PC,*pRPAZ,Radius);  
	if (Dir == 1)
		*pTanAZ = LTWOPI(*pRPAZ+HALFPI);	
	else
		*pTanAZ = LTWOPI(*pRPAZ-HALFPI);
	if (Dir == 1)
		*pCrdAZ = LTWOPI(*pRPAZ - *pAngle/2 + PY - HALFPI);	
	else
		*pCrdAZ = LTWOPI(*pRPAZ + *pAngle/2 + PY + HALFPI);	
	if (Dir == 2)
		clen = -(*pArcDist);  
	else
		clen = *pArcDist;
	rc = PCURVE(&PC.x,&PC.y,&pPOC->x,&pPOC->y,&pPT->x,&pPT->y,&pRP->x,&pRP->y,&clen); 
	return TRUE;
} 


 

BOOL EncodeTravLegData (LPTRAVLEGDATA pLegData,LPSTR str)
{
	switch (pLegData->Type)
	{
		case 0: //start point
			sprintf (str,"Starting at %s\t%i",pLegData->Parameters,pLegData->Type);
		break;               
		
		case 1: //POB
			sprintf (str,"*** Point of Beginning ***\t%i",pLegData->Type);
		break;               
		
		case 3: //Close
			sprintf (str,"*** Close Traverse ***\t%i",pLegData->Type);
		break;               
		
		case 11: //distance and bearing  	
		{ 
			sprintf (str,"Bearing and Distance (%s)\t%i",pLegData->Parameters,pLegData->Type);
		}
		break;
		
		case 21: //type 1 curve
			sprintf (str,"Type 1 curve (%s)\t%i",pLegData->Parameters,pLegData->Type);
		break;
		case 22: //type 2 curve
			sprintf (str,"Type 2 curve (%s)\t%i",pLegData->Parameters,pLegData->Type);
		break;
		case 23: //type 3 curve
			sprintf (str,"Type 3 curve (%s)\t%i",pLegData->Parameters,pLegData->Type);
		break;
		case 31: //sub-traverse
			sprintf (str,"Sub-Traverse (%s)\t%i",pLegData->Parameters,pLegData->Type);
		break;
	}
	return TRUE;
}

BOOL DecodeTravLegData (long TravID,int LegNum,LPTRAVLEGDATA pLegData,LPSTR str,LPDPOINT pPoints,LPDOUBLE pDist)
{   
	LPSTR	pParm,pEnd,pTAB; 
	double	Dist=0, AZ,Radius, ArcDist, TanDist, ChordDist, Angle, TanAZ, RPAZ, CrdAZ;   
	DPOINT	RP; 
	short	Dir,DistOpt,BearOpt;  
	LPSTR	pEQ;
	 
 	pTAB = _fstrchr(str,'\t'); 
 	if (pTAB)
 		*pTAB++=0;
 	else	
 		pTAB = _fstrchr (str,0);
    pLegData->Type=atoi(pTAB);
	pLegData->ID = TravID;
	pLegData->LegNum = LegNum;
	switch (pLegData->Type)
	{   
		case 0: 
			pParm = _fstrstr (str," at ");
			pParm+=4;
			_fstrcpy (pLegData->Parameters,pParm);   
			if (!pPoints)
				break;
			sscanf (pParm,"%lf %lf",&pPoints->x,&pPoints->y); 
			pPoints[2]=pPoints[0];
			break; 
		case 2://not used
		case 3://CLOSURE
		case 1://POB
			*pLegData->Parameters = 0;
			break;
		case 11: //bear and dist
			pParm = _fstrchr (str,'(');
			pParm++;
			pEnd = _fstrchr (pParm,')');
			*pEnd = 0;
			_fstrcpy (pLegData->Parameters,pParm); 
			if (!pPoints)
				break;
			DecodeBANDDParm (pParm,&Dist,&AZ,&Dir,pLegData->Type);
			CurrentAZ = AZ; 
			pPoints[0]=pPoints[2];
			pPoints[2]=dnewpt (pPoints[0],AZ+AZAdjust,Dist); 
			break;
		case 21: //type 1 curve
			pParm = _fstrchr (str,'(');
			pParm++;
			pEnd = _fstrchr (pParm,')');
			*pEnd = 0;
			_fstrcpy (pLegData->Parameters,pParm); 
			if (!pPoints)
				break;
			pPoints[0]=pPoints[2];
			DecodeTYPE1CRVParm (pParm,&Radius,&Dist,&AZ,&Dir,&DistOpt,&BearOpt);
			ComputeTYPE1CRV (Radius,Dist,AZ+AZAdjust,Dir,DistOpt,BearOpt,pPoints[0],
							 &pPoints[1],&pPoints[2],&RP,&ArcDist,&TanDist,&ChordDist,&TanAZ,&RPAZ,&CrdAZ,&Angle);
			break;
		case 31: //sub-traverse
			pParm = _fstrchr (str,'(');
			pParm++;
			pEnd = _fstrchr (pParm,')');
			*pEnd = 0;
			_fstrcpy (pLegData->Parameters,pParm);  
			if ((pEQ = _fstrchr (pParm,'=')))
				SubTravStartLeg = atol (++pEQ);
			else
				SubTravStartLeg = -1;
			break;
		default:
			pParm = _fstrchr (str,'(');
			pParm++;
			pEnd = _fstrchr (pParm,')');
			*pEnd = 0;
			_fstrcpy (pLegData->Parameters,pParm); 
			break;
	}
	if (pDist)
		*pDist = Dist;
	return TRUE;
}  


BOOL SaveTrav (long TravID,HWND hWndDlg,UINT ControlID)
{	
	HANDLE	hDB;
    LPGWDHEADER lpGWDHead; 
	short	item=0,st;
	char	str[256];  
	long	Offset;
	LPTRAVLEGDATA	pLegData;     
	LPSTR	pTAB;   
	
	_fstrcpy (str,"[%TRAVDATADB]");
	hDB = OpenGWDatabase (str,BT_WRITE);
	if (!hDB)
	{   
    	MessageBox(GetFocus(),"Unable to open traverse database",str,MB_ICONEXCLAMATION|MB_OK);
		return FALSE; 
	}
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
	pLegData = (LPTRAVLEGDATA)&lpGWDHead->GWDData;

NextDelete:	 
	pLegData->ID = TravID;
	pLegData->LegNum = -1;     
	GWDFormKey(lpGWDHead,0,TRUE,0,0);
	st = BT_FIND (lpGWDHead->BTHandle[0],lpGWDHead->pKeys[0],BT_FIRST,BT_GE, (LPSTR)&Offset);
	if (!st)
	{   
		FillGWDData (lpGWDHead,Offset);  
	    if (pLegData->ID == TravID)
	    {
	    	BT_DELETE (lpGWDHead->BTHandle[0],(LPSTR)lpGWDHead->pKeys[0],(LPSTR)&Offset,FALSE);
	    	goto NextDelete;
	    }
	}
    while (SendDlgItemMessage(hWndDlg,ControlID,LB_GETTEXT,item,(DWORD)str) != LB_ERR)
    {
		DecodeTravLegData (TravID,item,pLegData,str,0,0);  
		if (pLegData->Type == 1)
			POBItem = item;
		GWDReplaceRecord (lpGWDHead,0,0,-1);  
		item++;
	}
	GlobalUnlock (hDB);  
	CloseGWDatabase (hDB);   
	return TRUE;
}

void TravCompute (HWND hWndDlg)
{    
	double	size,Perim,area;
	DPOINT	Points[3],POB;
	short	choice; 
	BOOL	HavePOB=FALSE;   
	char	str[256]; 
	HPDPOINT	pPoints;  
	double		CloseErr; 
	HANDLE	hExpandedPoints;
	int		nExpandedPoints; 
	LPBYTE	pType;
	LPLEGDATA	pLegData;  
	MNMXCORD	TravBounds;
	
	HaveClosure=FALSE;			
//	SetDlgItemText (hWndDlg,IDC_SNAPDIST,0); 
	choice=0;    
	GSSiGlobFree (&hTravPoints);
	GSSiGlobFree (&hLegData);
	GSSiGlobFree (&hSnappedPoints);
	GSSiGlobFree (&hPointType);
	GSSiGlobFree (&hSnapStatus);  
	DBoundsInit (&TravBounds);
	nLegs = nTravPoints = 0;
	EnableWindow (GetDlgItem(hWndDlg,IDC_POB),TRUE);
	EnableWindow (GetDlgItem(hWndDlg,IDC_CLOSETRAV),FALSE);
	EnableWindow (GetDlgItem(hWndDlg,IDC_GENERATE),FALSE);    
 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_SNAPDIST),FALSE);
 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_SNAPDISTTITLE),FALSE);
 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_SNAPDISTTITLE2),FALSE);
	//EnableWindow (GetDlgItem(hWndDlg,IDC_ADDLEG),TRUE);
	LoadTravAddLegOptions (hWndDlg,IDC_ADDLEG,1);
	*str=0;
	SetDlgItemText (hWndDlg,IDC_MESS,str);

    while (SendDlgItemMessage(hWndDlg,IDC_TRAVLIST,LB_GETTEXT,choice++,(DWORD)str)!=LB_ERR)
    {   
    	double	Dist;
			    	
      	DecodeTravLegData (CurTravID,choice,&CurLegData,str,Points,&Dist);
		if (choice == 1)
		{
	 		char	curproject[32], SaveProj[32]; 
				 		
	 		GetGlobalCVal ("[%ALT_PROJECTION]",SaveProj,0);
			if (TravDest == DEST_TEST) 
	 			_fstrcpy (curproject,"baseproj.cvt"); 
	 		else
            	GetGlobalCVal ("[%TRAVPROJECTION]",curproject,"latlon83.cvt");
            SetGlobalValue("%ALT_PROJECTION",curproject);
			ConvertCoordClose ();
			ConvertCoordInit(); 
			AZAdjust = 0;
			if (TravDest != DEST_TEST)
			{
				if (SendDlgItemMessage (hWndDlg,IDC_ADJUSTAZ,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
					AZAdjust = GetAzAdjustment (Points[0]);
				else
				{
					GetDlgItemText (hWndDlg,IDC_ROTATEDEG,str,32);
					AZAdjust = atof (str) * DEGtoRAD;
				}
			}   
            SetGlobalValue("%ALT_PROJECTION",SaveProj);
			ConvertCoordClose ();
		}
      	switch (CurLegData.Type)
      	{
      		case 0: 
      			SetGlobalValueDPoint ("%STARTPOINT",Points[2]);
      		break;
      		case 1: //POB  
      			HavePOB = TRUE; 
      			POBItem = choice - 1;
      			POB = Points[2]; 
      			AddDPointToMinMax (&POB,&TravBounds);
      			SetGlobalValueDPoint ("%POB",POB);
      			EnableWindow (GetDlgItem(hWndDlg,IDC_POB),FALSE);
				EnableWindow (GetDlgItem(hWndDlg,IDC_CLOSETRAV),TRUE); 
				GSSiGlobFree (&hLegData);
				GSSiGlobFree (&hTravPoints); 
				GSSiGlobFree (&hSnappedPoints);
				GSSiGlobFree (&hPointType);
				GSSiGlobFree (&hSnapStatus);
				hTravPoints = GSSiGlobAlloc(GAIDNO 1080,GMEM_MOVEABLE,(long)USHRT_MAX*sizeof(DPOINT));
				hLegData = GSSiGlobAlloc(GAIDNO 1081,GMEM_MOVEABLE,(long)USHRT_MAX); 
				nLegs = 0;
				pPoints = (HPDPOINT)GlobalLock (hTravPoints);  
				*pPoints++ = POB;
				nTravPoints=1;    
				GlobalUnlock (hTravPoints);
				hPointType = GSSiGlobAlloc(GAIDNO 1082,GHND,(long)USHRT_MAX);
      		break;
      		case 3: //closure   
      			HaveClosure = TRUE;
				if (TravDest == DEST_UPDATE) 
				{
					EnableWindow (GetDlgItem(hWndDlg,IDC_GENERATE),TRUE);
				 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_SNAPDIST),TRUE);
				 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_SNAPDISTTITLE),TRUE);
				 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_SNAPDISTTITLE2),TRUE);
				}    
				EnableWindow (GetDlgItem(hWndDlg,IDC_CLOSETRAV),FALSE);
				//EnableWindow (GetDlgItem(hWndDlg,IDC_ADDLEG),FALSE);
				LoadTravAddLegOptions (hWndDlg,IDC_ADDLEG,2);
				if (HavePOB)
				{   
					long	icoord;
					HPDPOINT	pSnapCoord;
								
					CloseErr = ldistp (POB,Points[2]);
					CloseErr = ConvertDist (CloseErr,1);
/*					if (SendDlgItemMessage (hWndDlg,IDC_FORCECLOSE,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
					{
						pPoints = (HPDPOINT)GlobalLock (hTravPoints);
						pPoints[nTravPoints++]=Points[2]; 
						GlobalUnlock (hTravPoints);
					} */
					hExpandedPoints = ExpandTravPoints (hTravPoints,hPointType,nTravPoints,&nExpandedPoints);
					pPoints = (HPDPOINT)GlobalLock (hExpandedPoints); 
					size = ComputeAreaAreaD (pPoints,nExpandedPoints,&Perim);
					GSSiGlobUlFree (&hExpandedPoints);
					area = ConvertArea (size,6); 
					sprintf (str,"Closure error = %.4f Feet, Acres = %.4f",CloseErr,area);
					SetDlgItemText (hWndDlg,IDC_MESS,str);
					GSSiGlobFree (&hSnapStatus);      
					hSnapStatus = GSSiGlobAlloc(GAIDNO 1083,GMEM_MOVEABLE,(long)sizeof(DPOINT)*nTravPoints);
					pSnapCoord = (HPDPOINT)GlobalLock (hSnapStatus);
					for (icoord=0;icoord<nTravPoints;icoord++,pSnapCoord++)
						pSnapCoord->x=DBL_MAX;
					GlobalUnlock (hSnapStatus);
					pLegData = (LPLEGDATA)GlobalLock (hLegData); 
					pLegData += nLegs;
					pLegData->Type = 13;
					GlobalUnlock (hLegData);
       				nLegs++;

				}  
			break;
			case 11://distance and bearing  
			{
				if (!HavePOB)
					break;
				if (!HaveClosure)
				{
					pPoints = (HPDPOINT)GlobalLock (hTravPoints);
					pPoints+=nTravPoints; 
					*pPoints++ = Points[2];
					nTravPoints++;    
					GlobalUnlock (hTravPoints);
				} 
				pLegData = (LPLEGDATA)GlobalLock (hLegData); 
				pLegData += nLegs;
				pLegData->Type = 2;
				pLegData->Points[0] = Points[0];
				pLegData->Points[1] = Points[2]; 
      			AddDPointToMinMax (&Points[0],&TravBounds);
      			AddDPointToMinMax (&Points[2],&TravBounds);
				pLegData->Dist = Dist;
				GlobalUnlock (hLegData);  
				nLegs++;
			}
			break;
			case 21: //type 1 curve   
			case 22: //type 2 curve 
			case 23: //type 3 curve 
			{
							
				if (!HavePOB)
					break;
				if (!HaveClosure)
				{
					pPoints = (HPDPOINT)GlobalLock (hTravPoints);
					pPoints+=nTravPoints;     
					pType = GlobalLock (hPointType);
					pType += nTravPoints;
					*pPoints++ = Points[1]; 
					*pType = 1;
					GlobalUnlock (hPointType);
					*pPoints++ = Points[2];
					nTravPoints+=2;    
					GlobalUnlock (hTravPoints);
				}  
				pLegData = (LPLEGDATA)GlobalLock (hLegData); 
				pLegData += nLegs;
				pLegData->Type = 3;
				pLegData->Points[0] = Points[0];
				pLegData->Points[1] = Points[1];   
				pLegData->Points[2] = Points[2];   
      			AddDPointToMinMax (&Points[0],&TravBounds);
      			AddDPointToMinMax (&Points[1],&TravBounds);
      			AddDPointToMinMax (&Points[2],&TravBounds);
				pLegData->Dist = Dist;
				GlobalUnlock (hLegData);  
				nLegs++;
			}
			break;
			case 31:  
			{
				pLegData = (LPLEGDATA)GlobalLock (hLegData); 
				LoadTravAddLegOptions (hWndDlg,IDC_ADDLEG,3);
				pLegData[nLegs].Type = 31;
				pLegData[nLegs].Points[0] = pLegData[SubTravStartLeg].Points[0];   
				Points[1] = pLegData[nLegs].Points[1] = pLegData[SubTravStartLeg-1].Points[0];   
				Points[2] = pLegData[nLegs].Points[2] = pLegData[SubTravStartLeg-1].Points[1];   
				GlobalUnlock (hLegData);
				nLegs++; 
			}
			break; 
		} 
    } 
    SetGlobalValueBounds ("%TRAVBOUNDS",&TravBounds);
    return;
}  

BOOL ShowTrav (HWND hWndDlg,UINT IDCTRAVLIST,long CurTravID)
{  
	HANDLE	hDB;
    LPGWDHEADER lpGWDHead; 
    LPGWFLDINFO lpGWFldInfo; 
	LPTRAVLEGDATA	pLegData;    
	char	str[256]; 
    long	Offset;   
	short	st, choice;
	
    SendDlgItemMessage (hWndDlg,IDCTRAVLIST,LB_RESETCONTENT,0,0);
	hDB = OpenGWDatabase ("[%TRAVDATADB]",BT_READ);
	if (!hDB)
	{   
		MessageBox(GetFocus(),"Unable to open traverse database", 0,MB_ICONEXCLAMATION|MB_OK);
		return FALSE; 
	}
	EnableWindow (GetDlgItem(hWndDlg,IDC_POB),TRUE);
	EnableWindow (GetDlgItem(hWndDlg,IDC_NEXT),TRUE);
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
	pLegData = (LPTRAVLEGDATA)&lpGWDHead->GWDData;
	pLegData->ID = CurTravID;
	pLegData->LegNum = -1;     
	GWDFormKey(lpGWDHead,0,TRUE,0,0);
	st = BT_FIND (lpGWDHead->BTHandle[0],lpGWDHead->pKeys[0],BT_FIRST,BT_GE, (LPSTR)&Offset);
	while (!st)
	{   
		FillGWDData (lpGWDHead,Offset);  
		if (pLegData->ID != CurTravID) 
			break;     
		EncodeTravLegData (pLegData,str);   
		if (pLegData->Type == 3)
			EnableWindow (GetDlgItem(hWndDlg,IDC_POB),FALSE);
		choice = SendDlgItemMessage (hWndDlg,IDCTRAVLIST,LB_ADDSTRING,0,(LPARAM)str);  
		st = BT_FIND (lpGWDHead->BTHandle[0],lpGWDHead->pKeys[0],BT_NEXT,BT_ANY, (LPSTR)&Offset);
	}   
	GlobalUnlock (hDB);  
	CloseGWDatabase (hDB);   
    return TRUE;
}  

void LoadTravAddLegOptions (HWND hWndDlg,UINT IDCADDLEG,int Opt)
{
   	int	nItems = SendDlgItemMessage (hWndDlg,IDCADDLEG,LB_GETCOUNT,0,0);
   	int	Choice = SendDlgItemMessage (hWndDlg,IDCADDLEG,LB_GETCURSEL,0,0); 
   	
   	SendDlgItemMessage (hWndDlg,IDCADDLEG,LB_RESETCONTENT,0,0);
    if (Opt != 2)
    {
	 	SendDlgItemMessage (hWndDlg,IDCADDLEG,LB_ADDSTRING,0,
	 						(LPARAM)"Bearing and Distance\t11");
	 	SendDlgItemMessage (hWndDlg,IDCADDLEG,LB_ADDSTRING,0,
	 						(LPARAM)"Distance along curve\t21"); 
	}
 	if (Opt > 1)
 		SendDlgItemMessage (hWndDlg,IDCADDLEG,LB_ADDSTRING,0,
 						(LPARAM)"Begin Sub-Traverse\t31");
//	 	SendDlgItemMessage (hWndDlg,IDC_ADDLEG,LB_ADDSTRING,0,
//	 						(LPARAM)"Distance along type 2 curve (Direction, radius point bearing and radius)\t22");
//	 	SendDlgItemMessage (hWndDlg,IDC_ADDLEG,LB_ADDSTRING,0,
//	 						(LPARAM)"Distance along type 3 curve (Direction, radius and tangent length)\t23");
	SendDlgItemMessage (hWndDlg,IDCADDLEG,LB_SETCURSEL,0,0);
	return;
}

void TravShowSelectedLeg (HWND hWndDlg,UINT IDCTRAVLIST,UINT IDCCALLNO,int choice)
{   
	long	Refno;
	char	str[256];
	
	if (choice > POBItem)    
	{   
		if (!CurrentConfig)
			SetConfig (1);
 		SetViewport (*pCommandViewport);
		Refno = choice-POBItem;
	    ltoa (Refno,str,10);
	    SetDlgItemText (hWndDlg,IDCCALLNO,str); 
//		if (SendDlgItemMessage (hWndDlg,IDC_TESTMODE,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)) 
		if (TravDest == DEST_TEST)
		{   
	    	if (PickByRefno(Refno,0,0,GetPickFile (-1)))
	    	{
	    		AddToHighlightList (Refno,&PickList[0],TRUE);
				ProcessPickedItem (0,TRUE);
//				RedisplayViewport(FALSE,FALSE);
			} 
	    }
	}
	return;
} 

void AdjustSubTraverse (HWND hWndDlg,UINT IDCTRAVLIST,int From,int Increment)
{   
	int	choice = 0;
	TRAVLEGDATA	LegData;   
	char	str[256];
	int	ModLeg=SHRT_MAX;
	
	while (SendDlgItemMessage(hWndDlg,IDC_TRAVLIST,LB_GETTEXT,choice,(DWORD)str) != LB_ERR)
	{
     	DecodeTravLegData (CurTravID,choice,&LegData,str,0,0);
    	if (LegData.Type==1)
    		ModLeg = From - choice;
    	else if (LegData.Type==31 && SubTravStartLeg >= ModLeg)
    	{
			SubTravStartLeg += Increment;    	
		    sprintf (LegData.Parameters,"StartLeg=%i",SubTravStartLeg);
		    EncodeTravLegData (&LegData,str); 
			SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_DELETESTRING,choice,0); 
			SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_INSERTSTRING,choice,(LPARAM)str); 
		}
		choice++;
	} 
	return;
}   

 

void RemoveTravTranFile (void)
{   
	char	File[256];
	
	if (!GetGlobalCVal ("[%TRAVTRANFILE]",File,0)) 
		return;
	GSSiRemove (File);
	return;
}

BOOL SaveTravTransformation (HANDLE hPoints,HANDLE hSnapStatus,long nPoints)
{
	HPDPOINT	SnapCoord, Point;    
	long	i;
	BOOL	HaveAdjust = FALSE;
	char	str[256], File[256];  
	HFILE	Fid;
	
	if (!GetGlobalCVal ("[%TRAVTRANFILE]",File,0))
	{
		GSSiGetTempFileName (0,"gmt",0,File); 
		SetGlobalValue("%TRAVTRANFILE",File);
	}
	Fid = GSSiOpenFile (File,0,OF_CREATE);
	Point = (HPDPOINT)GlobalLock (hPoints);
	SnapCoord = (HPDPOINT)GlobalLock (hSnapStatus);
	for (i=0;i<nPoints-1;i++)
	{
		if (SnapCoord[i].x < DBL_MAX)
		{
			sprintf (str,"%lf %lf %lf %lf",Point[i].x,Point[i].y,SnapCoord[i].x,SnapCoord[i].y);
			HaveAdjust = TRUE;
		}
		else
			sprintf (str,"%lf %lf %lf %lf",Point[i].x,Point[i].y,Point[i].x,Point[i].y); 
		fputstring (str,Fid);
	} 
	GlobalUnlock (hPoints);
	GlobalUnlock (hSnapStatus);
	GSSiClose2 (&Fid);
	if (!HaveAdjust)
		GSSiRemove (File);    
	return HaveAdjust;
}

HANDLE SnapTravPoints (HANDLE hPoints,HANDLE hSnapStatus,long nPoints,double SnapDist,BOOL ForceClose)
{   
	//Snaps traverse points to existing linework. Uses snapped points to transformed un-snapped
	// points.
	
	HPDPOINT	pPoint1,pPoint2; 
	HPDPOINT	pSnapCoord;    
	long	i, NumNotSnapped=0, MaxTran = min (MAXTRANPOINTS,nPoints);
	HANDLE handle=GSSiGlobAlloc(GAIDNO 1085,GHND,nPoints*(long)sizeof(DPOINT));   
	HANDLE	hTran=0, hTemp;
	short	SaveMaxPick=MaxPick, nTran=0;
	float	RSQMIN;	    
	char	PickFile[128];
	static	char	SavePickFile[128]="";
    LPDOUBLE  XT1, YT1, XT2, YT2; 
    LPVISLIST	pSaveVis=CurVis;
    
    if (!*SavePickFile)
		 GSSiGetTempFileName (0,"gmb",0,SavePickFile);
    SaveVisFile (SavePickFile,TRUE,"");
    GetGlobalCVal ("[%SNAPPICKLIST]",PickFile,0);
    if (*PickFile)
    	LoadPickList (PickFile);
    hTemp = GSSiGlobAlloc(GAIDNO 1086,GMEM_MOVEABLE,MaxTran*16*2);
    XT1 = (LPDOUBLE)GlobalLock (hTemp);
    XT2 = XT1 + MaxTran;
    YT1 = XT2 + MaxTran;
    YT2 = YT1 + MaxTran;
	pPoint1 = (HPDPOINT)GlobalLock (hPoints);
	pPoint2 = (HPDPOINT)GlobalLock (handle);  
	pSnapCoord = (HPDPOINT)GlobalLock (hSnapStatus);    
	UseUserPickAp =FALSE;
	SystemPickAp = -SnapDist;	
	MaxPick=1;
	SetPickAp(0);  

	for (i=0;i<nPoints-1;i++,pPoint1++,pPoint2++,pSnapCoord++)
	{             
		NumPicked = 0; 
		if (pSnapCoord->x < DBL_MAX)
		{
			NumPicked = 1;
			PickList[0].PickedPoint = *pSnapCoord; 
		} 
		else 
		{
			short SavePP = PickPerim;

			PickPerim = 1;  
			PickOnlyNodePoints = TRUE;
			if (SnapDist > 0)
				PickItems2 (CurView->hWnd,*pPoint1,FALSE,TRUE,TRUE); 
			PickOnlyNodePoints = FALSE;
			if (!NumPicked && SnapDist > 0)
				PickItems2 (CurView->hWnd,*pPoint1,FALSE,TRUE,TRUE);  
			PickPerim = SavePP; 
			if (!NumPicked)
			{
				NumPicked = 1;
				PickList[0].PickedPoint = *pPoint1; 
			}
		}
		if (NumPicked)
		{
			*pPoint2 =  *pSnapCoord = PickList[0].PickedPoint;
			XT1[nTran] = pPoint1->x;
			YT1[nTran] = pPoint1->y;
			XT2[nTran] = pPoint2->x;
			YT2[nTran] = pPoint2->y;
			if (i+1 < nPoints)
				nTran++;   
		}
		else  
		{
			NumNotSnapped++;
			pPoint2->x = -DBL_MAX;
		}
	}
	if (nTran > 3)
		hTran = STRAN2 (1668,XT1,YT1,XT2,YT2,nTran,&RSQMIN,3,0);
	GSSiGlobUlFree (&hTemp);   
	GlobalUnlock (hPoints);
	GlobalUnlock (handle);
	GlobalUnlock (hSnapStatus);  
	if (NumNotSnapped)   
	{
		pPoint1 = (HPDPOINT)GlobalLock (hPoints);
		pPoint2 = (HPDPOINT)GlobalLock (handle);  
		for (i=0;i<nPoints;i++,pPoint1++,pPoint2++)
		{   
//					MessageBox (0,"pastpickap",0,MB_OK);
			if (pPoint2->x == -DBL_MAX)
				*pPoint2 = TranPoint (pPoint1,hTran);
	    }
		GlobalUnlock (hPoints);
		GlobalUnlock (handle);
	}
	pPoint1 = (HPDPOINT)GlobalLock (hPoints);
	pPoint2 = (HPDPOINT)GlobalLock (handle);
	if (ForceClose)
		pPoint2[nPoints-1] = pPoint2[0];
	else  
		pPoint2[nPoints-1] = TranPoint (&pPoint1[nPoints-1],hTran);
	GlobalUnlock (hPoints);
	GlobalUnlock (handle);
	CloseTRANS2 (&hTran);
	UseUserPickAp =TRUE;
	MaxPick = SaveMaxPick;  
	CurVis = pSaveVis;
    if (*PickFile)
    	LoadPickList (SavePickFile);
    CurVis = pSaveVis;
	return handle;
}  

BOOL TravCreateDimText (void)
{
	short	SymNum=0, NumSyms=0,PickFile,i; 
	HANDLE	hSymDesc=0; 
	long	Refno;
	char	SymName[66]; 
	BYTE	LastType;  
	LPBYTE	pPointType;
	HPDPOINT	pSnappedPoint, pOrigPoint;  
	DPOINT	Points[2], OrigPoints[2];
//	LPLEGDATA pLegData;
	    
    if (!nTravPoints || !hTravPoints || !hSnappedPoints || !hPointType)
    {
 		MessageBox(GetFocus(), "No current traverse", 0,MB_ICONEXCLAMATION|MB_OK);
        return FALSE;
	} 
	GetGlobalCVal ("[%LOTDIMSYMBOL]",SymName,"LOTDIMENSION_POINT");
	SymNum = GetDictSymbolNumber (SymName);
	if (!SymNum)
	{
 		MessageBox(GetFocus(), "Lot dimension symbol not found", SymName,MB_ICONEXCLAMATION|MB_OK);
        return FALSE;
	} 
	if (!CurrentConfig)
		SetConfig (1);
	SetViewport (*pCommandViewport);
	if (!CurView->UpdateFile)
	{
 		MessageBox(GetFocus(), "No update file in this viewport", 0,MB_ICONEXCLAMATION|MB_OK);
        return FALSE;
	} 
	else
		_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
   	            
    if (!ExistFile(EditName))
    	copyfile (EditName,"[%NULLMAP]",FALSE,0,0,0,0,0,0);
	AddToSymList (SymNum,&NumSyms,&hSymDesc); 
	_fstrcpy (PltName,EditName);
	PltType = 2;
	OpenMap (CurView->hWnd,CurView->hDC);
	EditBounds = CurView->FileMNMX; 
	CloseMap (FALSE); 
	pPointType = GlobalLock (hPointType); 
	pSnappedPoint = (HPDPOINT)GlobalLock (hSnappedPoints); 
	pOrigPoint = (HPDPOINT)GlobalLock (hTravPoints); 
	Points[0] = *pSnappedPoint++; 
	OrigPoints[0] = *pOrigPoint++; 
	LastType = *pPointType++;
	for (i=1;i<nTravPoints;i++,pSnappedPoint++,pPointType++,pOrigPoint++)
	{   
		
		Points[1] = *pSnappedPoint;
		OrigPoints[1] = *pOrigPoint; 
		if (!LastType && !*pPointType)
		{   
			DPOINT	DPoint; 
			double	AZ,d; 
			double	SymSize=20;
			HANDLE		hGRText; 
			LPGRTEXT	lpGRText; 
			char	txt[32];
			
			d = ldistp (OrigPoints[0],OrigPoints[1]);  
			d = CvtDist(d,(short)PRJ_UNITS[1],1);
			RWRITE (d, 2, txt); 
			_fstrcat (txt,"'");
			DPoint = MidPointD (Points[0],Points[1]);
			AZ = getazd (&Points[0],&Points[1]);

            hGRText = GSSiGlobAlloc(GAIDNO 1087,GHND,sizeof(GRTEXT));
            lpGRText = (LPGRTEXT)GlobalLock (hGRText);   
            lpGRText->UltiMapStyle = 1;  
            lpGRText->version = 1;    
            lpGRText->length = sizeof(GRTEXT);   
            lpGRText->ltext = _fstrlen (txt);
            lpGRText->ltext += lpGRText->ltext % 2;
            TSize = 6.5;  
            sprintf (lpGRText->cHeight,"%f",CvtDist(TSize,(short)PRJ_UNITS[3],(short)PRJ_UNITS[1])); 
        	lpGRText->FontNum = 0;  
        	lpGRText->hJust = 0;
            lpGRText->vJust = 3; 
        	lpGRText->FlipForEasyReading = 1;
            _fstrcpy (lpGRText->Text,txt);
            GlobalUnlock (hGRText);
            
            if (d > 10)
            {
				Refno = GetNewRefno (EditName,0,0,0,0);
	           	AddPointToMap (DPoint,Refno,0,SymNum,SymSize,AZ,0,hGRText,0,0,0,-1,-1,-1,FALSE,HiPrecis,0,0); 
	        }
           	GSSiGlobFree (&hGRText);
//			AddPolyToMap (1,&nPoints, &hPoints,0,Refno,0,-1,SymNum,0,0,0,-1,-1,-1,0,0,0,0,TRUE); 
		}
		Points[0] = Points[1];
		OrigPoints[0] = OrigPoints[1];
		LastType = *pPointType;
	} 
	GlobalUnlock (hSnappedPoints);
	GlobalUnlock (hPointType);  
	GlobalUnlock (hTravPoints);
    CloseMap(TRUE);  
	AddSymToMap (NumSyms,hSymDesc,0,0); 
    DestroySymList (&NumSyms,&hSymDesc);  
	return TRUE;
}

HANDLE ExpandTravPoints (HANDLE hPoints,HANDLE hPointTypes,long nPoints,LPINT pnExpandedPoints)
{
	HANDLE handle=GSSiGlobAlloc(GAIDNO 1088,GMEM_MOVEABLE,(long)USHRT_MAX*(long)sizeof(DPOINT));
	long	nNewPoints=0;
	HPDPOINT	pPoint1= (HPDPOINT)GlobalLock (hPoints);
	HPDPOINT	pPoint2= (HPDPOINT)GlobalLock (handle);  
	LPBYTE	pType=GlobalLock (hPointTypes); 
	BOOL 	HavePOC=FALSE;   
	DPOINT	PC, POC, PT; 
	double	BackAZ;
	
	nNewPoints = 0; 
	while (nPoints--) 
	{
		if (*pType)
		{
			HavePOC = TRUE;  
			POC = *pPoint1++;
		}
		else if (HavePOC)
		{
		    PT = *pPoint1++;
            CurvePointsD(&PC,&POC,&PT, &nNewPoints, &pPoint2,&BackAZ,USHRT_MAX-4-nNewPoints-nPoints,CurveChordDist,1);
            HavePOC = FALSE;  
            PC = PT;
	    }
	    else   
	    {
			PC = *pPoint2++ = *pPoint1++;  
			nNewPoints++;
		} 
		pType++;
	}
	GlobalUnlock (hPoints);
	GlobalUnlock (handle);   
	GlobalUnlock (hPointTypes);          
	*pnExpandedPoints = nNewPoints;
	return handle; 
}
  


 

