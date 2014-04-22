#include "graphint.h"   
#include "dgnlib.h"
#include "gmextern.h"   
#include "wownt16.h"  
#include <errno.h>       

#define		MAX_POINTS	100
HFILE	Fid[32];
short	nOpenFiles=0;
int DGNWriteGeoMaster (LPSTR Infile);
DGNElemCore *psMembers[100], *psLineMembers[100];
DGNElemCore *symMembers[100][100];
int	NumSym=0, NumSymElem[100];
char	SymNames[100][8];
int		nAreaElements=0;
BOOL	InArea=FALSE;
long	recno=0;
int		Color = 0, Weight=1, Font, Style, Level;
int		LineType;
char	VNames[32][4];

DWORD DGN7GetElementExtents ( DGNHandle hDGN,double * pBounds)
{
	DGNElemCore *pDGNElemCore;
	DGNPoint	MinPt, MaxPt;
	DWORD	rtn;

	if (!(pDGNElemCore = DGNReadElement(hDGN)))
		return FALSE;

	rtn = DGNGetElementExtents( hDGN, pDGNElemCore,(DGNPoint *) &MinPt,&MaxPt);
	if (rtn)
	{
		pBounds[0] = MinPt.x;
		pBounds[1] = MinPt.y;
		pBounds[2] = MaxPt.x;
		pBounds[3] = MaxPt.y;
	}
	return rtn;
}

double PolyLength (DWORD nPoints,DGNPoint * Points)
{
	double l=0;
	DWORD	i;

	for (i=1;i<nPoints;i++)
		l += sqrt (pow(Points[i].x-Points[i-1].x,2)
            +pow(Points[i].y-Points[i-1].y,2));
	return l;
}

DWORD DGN7ReadElement( DGNHandle hDGN,LPSTR pMem,DWORD MaxMemSize,double * pPixelSize,
								 int * pFillColor,int * pNumAttributes,int * Attributes )
{
	int	size;
	DGNElemCore *pDGNElemCore = DGNReadElement(hDGN);

	if (pDGNElemCore == NULL)
		return 0;
	switch (pDGNElemCore->stype)
	{
		case DGNST_MULTIPOINT:
		{
			DGNElemMultiPoint *pRec = (DGNElemMultiPoint *)pDGNElemCore;

			size = sizeof (DGNElemMultiPoint) + (pRec->num_vertices - 2) * sizeof (DGNPoint);
		}
		break;

		case DGNST_ARC:
			size = sizeof(DGNElemArc);
		break;
		
		case DGNST_TEXT:
		{
			DGNElemText *pRec = (DGNElemText *)pDGNElemCore;

			size = sizeof (DGNElemText) + strlen (pRec->string);
		}
		break;

		case DGNST_COMPLEX_HEADER:
			
			size = sizeof(DGNElemComplexHeader);
		break;

		case DGNST_COLORTABLE:
		
			size = sizeof(DGNElemColorTable);
		break;


		case DGNST_TCB:
		
			size = sizeof (DGNElemTCB);
		break;


		case DGNST_CELL_HEADER:
	
			size = sizeof (DGNElemCellHeader);
		break;

		case DGNST_CELL_LIBRARY:
	
			size = sizeof (DGNElemCellLibrary);
		break;
	
		default:
			size = sizeof (DGNElemCore);
		break;
	}
	
	memcpy (pMem,pDGNElemCore,size);
	if (pDGNElemCore->stype == DGNST_CORE)
	{
		memcpy (&pMem[size],pDGNElemCore->raw_data,pDGNElemCore->raw_bytes);
		size += pDGNElemCore->raw_bytes;
	}
	if (pPixelSize)
	{
		int * pnum_vertices = (int *)&pMem[size];

		*pnum_vertices = 0;
		if (*pPixelSize > 0)
		{
			DGNPoint	*pvertices = (DGNPoint *)(pnum_vertices+1);
			DGNPoint	TestPoints[5];
			int			MaxPoints = (MaxMemSize - size - 4) / sizeof (DGNPoint);
			double		dlen;

			switch (pDGNElemCore->type)
			{
				case DGNT_ELLIPSE:
				case DGNT_ARC:
					{
						DGNStrokeArc( hDGN,(DGNElemArc *)pDGNElemCore,5,TestPoints);
						dlen = PolyLength (5,TestPoints);
						*pnum_vertices = min (MaxPoints-1,max (2,(int)(dlen/(*pPixelSize)))) + 1;
						DGNStrokeArc( hDGN,(DGNElemArc *)pDGNElemCore,*pnum_vertices,pvertices);
					}
					break;
				case DGNT_CURVE:
					{
						DGNElemMultiPoint *pRec = (DGNElemMultiPoint *)pDGNElemCore;
						dlen = PolyLength (pRec->num_vertices,pRec->vertices);
						*pnum_vertices = min (MaxPoints-1,max (2,(int)(dlen/(*pPixelSize)))) + 1;
						DGNStrokeCurve( hDGN,(DGNElemMultiPoint *)pDGNElemCore,*pnum_vertices,pvertices);
					}
					break;
			}
		}
	}
	if (pFillColor != NULL)
	{
		int	Color;

		if (DGNGetShapeFillInfo( hDGN, pDGNElemCore,&Color))
			*pFillColor = Color;
		else
			*pFillColor = -1;
	}
	if (pNumAttributes != NULL)
	{
		int	nLinkType, nEntity, nMSLink, nLinkSize;

		*pNumAttributes = 0;
		while (DGNGetLinkage( hDGN, pDGNElemCore, *pNumAttributes, &nLinkType, 
                                  &nEntity, &nMSLink, &nLinkSize ))
		{
			Attributes[(*pNumAttributes)*3] = nLinkType;
			Attributes[(*pNumAttributes)*3 + 1] = nEntity;
			Attributes[(*pNumAttributes)*3 + 2] = nMSLink;
			(*pNumAttributes)++;
		}
	}
	DGNFreeElement (hDGN,pDGNElemCore);
	return 1;
}

DWORD DGN7SetSpatialFilter( DGNHandle hDGN,double * Extents )
{
	DGNSetSpatialFilter (hDGN,Extents[0],Extents[1],Extents[2],Extents[3]);
	return 1;
}

DWORD DGN7GetNumElements( DGNHandle hDGN)
{
//    DGNInfo     *psDGN = (DGNInfo *) hDGN;
	int			NumElements;

	DGNGetElementIndex (hDGN,&NumElements);
	return NumElements;
}

void AddMSLink (DGNHandle hNewDGN,DGNElemCore *psMember,LPSTR MSLinkVals)
{
	LPSTR	NextVal = MSLinkVals;
	int		LinkageType, EntityNum, MSLink;
	LPSTR	pType, pEntity, pMSLink;

	if (!*MSLinkVals)
		return;
	while (NextVal)
	{
		NextVal = strchr (MSLinkVals,'|');
		if (NextVal)
			*NextVal++ = 0;
		pType = MSLinkVals;
		pEntity = strchr (pType,':');
		*pEntity++ = 0;
		pMSLink = strchr (pEntity,':');
		*pMSLink++ = 0;
		LinkageType = 0;
		if (!strcmp (pType,"ORACLE"))
			LinkageType = DGNLT_ORACLE;
		else if (!strcmp (pType,"DMRS"))
			LinkageType = DGNLT_DMRS;
		else if (!strcmp (pType,"ODBC"))
			LinkageType = DGNLT_ODBC;
		else if (!strcmp (pType,"RIS"))
			LinkageType = DGNLT_RIS;
		else if (!strcmp (pType,"SYBASE"))
			LinkageType = DGNLT_SYBASE;
		else if (!strcmp (pType,"XBASE"))
			LinkageType = DGNLT_XBASE;
		EntityNum = atol (pEntity);
		MSLink = atol (pMSLink);
		if (LinkageType)
			DGNAddMSLink(hNewDGN, psMember, LinkageType, EntityNum, MSLink);
		MSLinkVals = NextVal;
	}
	return;
}

int DGNWriteGeoMaster (LPSTR Infile)
{
	char	str[520],Outfile[MAX_PATH+2],Seedfile[MAX_PATH+2], MSLinkValues[512], DGNLevC[66];
	OFSTRUCTGM	OFStruct;
	HFILE	Fid=GSSiOpenFile (Infile,&OFStruct,OF_READ);
    DGNHandle hNewDGN;
    DGNPoint   asPoints[500];
    DGNElemCore *psLine, *psPoint;
	DGNPoint	PointLoc;
	double	Rotation;
	char SymName[64];
	LPSTR	pSymName;
	int	isym, line=0;

	if (!*Infile)
		return 0;
	fgetstring (Outfile,MAX_PATH,Fid);line++;
	fgetstring (Seedfile,MAX_PATH,Fid);line++;
/* -------------------------------------------------------------------- */
/*      Create new DGN file.                                            */
/* -------------------------------------------------------------------- */
    hNewDGN = DGNCreate( Outfile,Seedfile,DGNCF_USE_SEED_UNITS
                         | DGNCF_USE_SEED_ORIGIN |DGNCF_COPY_SEED_FILE_COLOR_TABLE, 
                         0.0, 0.0, 0.0, 0, 0, "", "" );
	if (!hNewDGN)
		return 0;
/* -------------------------------------------------------------------- */
/*      Write one line segment to it.                                   */
/* -------------------------------------------------------------------- */
	while (fgetstring (str,518,Fid))
	{
		int	GMType=atoi (str);
		int	Refno, debugref=-1646448536;
		int	nPnts, i, LineType,ii, nParts;
		LPSTR pRef = strchr (str,',');

		if (pRef)
		{
			pRef++;
			Refno = atol (pRef);
			if (Refno == debugref)
				ii=1;
		}
		line++;
		if (line > 420)
			ii=1;
		if (GMType != 0 && GMType != 99)
		{
			pSymName = strrchr (str,',');
			pSymName++;
			strcpy (SymName,pSymName);
			if (GMType != 6)
			{
				fgetstring (DGNLevC,64,Fid);line++;
				sscanf (DGNLevC,"%i,%i,%i,%i,%i",&Level,&Color,&Weight,&Style,&Font);
				fgetstring (MSLinkValues,510,Fid);line++;
			}
		}
		if (GMType < 0)
		{
			InArea = TRUE;
			GMType = -GMType;
		}
		else
			InArea = FALSE;
		switch (GMType)
		{

		case 0://symbol def
		{
			LPSTR	pSpace=strchr (str,' ');
			int		ielem, Type;

			pSpace++;
			NumSymElem[NumSym] = atoi (pSpace);
//			NumSymElem[NumSym] = 0;
			pSpace = strchr (pSpace,' ');
			pSpace++;
			strcpy (SymNames[NumSym],pSpace);
			if (!stricmp (pSpace,"USB"))
				ii=1;
			for (ielem=0;ielem<NumSymElem[NumSym];ielem++)
			{
				fgetstring (str,128,Fid);line++;
				sscanf (str,"%i %i",&Type,&nPnts);
				for (i=0;i<nPnts;i++)
				{
					fgetstring (str,128,Fid);line++;
					sscanf (str,"%Flf %Flf",&asPoints[i].x,&asPoints[i].y);
					asPoints[i].z = 0;
				}
				if (Type == -3)
					LineType = DGNT_SHAPE;
				else if (nPnts > 2)
					LineType = DGNT_LINE_STRING;
				else
					LineType = DGNT_LINE;

				symMembers[NumSym][ielem] = DGNCreateMultiPointElem( hNewDGN, LineType, nPnts, asPoints );
			}
			NumSym++;

		}
			break;
		case 1://point
			for (isym = 0;isym<NumSym;isym++)
				if (!stricmp (SymNames[isym],SymName))
					goto FoundSym;
			fgetstring (str,128,Fid);line++;
			break;
FoundSym:
//			fgetstring (str,128,Fid);line++;
//			break;
			fgetstring (str,128,Fid);line++;
			sscanf (str,"%Flf %Flf %Flf",&PointLoc.x,&PointLoc.y,&Rotation);
			PointLoc.z = 0;
			if (!NumSymElem[isym])
				break;
			psPoint = DGNCreateCellHeaderFromGroup( hNewDGN, SymNames[isym], 1, NULL,
												   NumSymElem[isym], symMembers[isym], &PointLoc, 
												   100.0, 100.0, Rotation );

			AddMSLink (hNewDGN,psPoint,MSLinkValues);
					ii=DGNUpdateElemCore( hNewDGN, psMembers[nAreaElements], Level, 0, Color, Weight, Style );
					if (!InArea)
					{
						DGNWriteElement( hNewDGN, psMembers[nAreaElements]);
						DGNFreeElement( hNewDGN, psMembers[nAreaElements]);
					}
			DGNWriteElement( hNewDGN, psPoint );
			for (i=0;i<NumSymElem[isym];i++)
				DGNWriteElement( hNewDGN, symMembers[isym][i] );
			break;
		case 3:
			nParts=1;
			InArea = TRUE;
			nAreaElements = 0;
			goto Temp;
		case 2://line
			fgetstring (str,128,Fid);line++;
			nParts = atoi (str);
Temp:
			while (nParts--)
			{
				fgetstring (str,128,Fid);line++;
				nPnts = atoi (str);
				if (nPnts < MAX_POINTS)
				{
					for (i=0;i<nPnts;i++)
					{
						fgetstring (str,128,Fid);line++;
						sscanf (str,"%Flf %Flf",&asPoints[i].x,&asPoints[i].y);
						asPoints[i].z = 0;
					}
					if (nPnts > 2)
						LineType = DGNT_LINE_STRING;
					else
						LineType = DGNT_LINE;
					psMembers[nAreaElements] = DGNCreateMultiPointElem( hNewDGN, LineType, nPnts, asPoints );
					if (!InArea)
						AddMSLink (hNewDGN,psMembers[nAreaElements],MSLinkValues);
					ii=DGNUpdateElemCore( hNewDGN, psMembers[nAreaElements], Level, 0, Color, Weight, Style );
					if (!InArea)
					{
						DGNWriteElement( hNewDGN, psMembers[nAreaElements]);
						DGNFreeElement( hNewDGN, psMembers[nAreaElements]);
					}
					else
						nAreaElements++;
				}
				else
				{
					long	j,nLineElements=0,np,nSegs = (nPnts-1)/MAX_POINTS + 1;
					int	ib=0;

					for (j=0;j<nSegs;j++)
					{

						if (j)
						{
							asPoints[0] = asPoints[np+ib-1];
							ib=1;
						}
						if (j < nSegs - 1)
							np = MAX_POINTS;
						else
							np = (nPnts-1)%MAX_POINTS + 1;
						for (i=0;i<np;i++)
						{
							fgetstring (str,128,Fid);line++;
							sscanf (str,"%Flf %Flf",&asPoints[i+ib].x,&asPoints[i+ib].y);
							asPoints[i+ib].z = 0;
						}
						if (np+ib > 2)
							LineType = DGNT_LINE_STRING;
						else
							LineType = DGNT_LINE;
						psMembers[nLineElements] = DGNCreateMultiPointElem( hNewDGN, LineType, np+ib, asPoints );
						AddMSLink (hNewDGN,psMembers[nLineElements],MSLinkValues);
						DGNUpdateElemCore( hNewDGN, psMembers[nLineElements++], Level, 0, Color, Weight, Style );
					}
					if (InArea)
						nAreaElements = nLineElements;
					else
					{
						psLine = DGNCreateComplexHeaderFromGroup( hNewDGN, 
															  DGNT_COMPLEX_CHAIN_HEADER,
															  nLineElements, psMembers );

						//DGNAddShapeFillInfo( hNewDGN, psLine, 7 );

						DGNWriteElement( hNewDGN, psLine );
						for (i=0;i<nLineElements;i++)
							DGNWriteElement( hNewDGN, psMembers[i] );

						DGNFreeElement( hNewDGN, psLine );
						for (i=0;i<nLineElements;i++)
							DGNFreeElement( hNewDGN, psMembers[i] );
					}
					 
				}
			}

			break;
		case 33://shape
			fgetstring (str,128,Fid);line++;
			nPnts = atoi (str);
			for (i=0;i<nPnts;i++)
			{
				fgetstring (str,128,Fid);line++;
				sscanf (str,"%Flf %Flf",&asPoints[i].x,&asPoints[i].y);
				asPoints[i].z = 0;
			}
			LineType = DGNT_SHAPE;
			if (nPnts > MAX_POINTS)
				nPnts = MAX_POINTS;
			psMembers[nAreaElements] = DGNCreateMultiPointElem( hNewDGN, LineType, nPnts, asPoints );
			if (!InArea)
				AddMSLink (hNewDGN,psMembers[nAreaElements],MSLinkValues);
			DGNUpdateElemCore( hNewDGN, psMembers[nAreaElements], Level, 0, Color, Weight, Style );
			DGNAddShapeFillInfo( hNewDGN, psMembers[nAreaElements], 7 );

			if (!InArea)
			{
				DGNWriteElement( hNewDGN, psMembers[nAreaElements]);
				DGNFreeElement( hNewDGN, psMembers[nAreaElements]);
			}
			else
				nAreaElements++;

			break;
		case 5://curve
		{
		/* -------------------------------------------------------------------- */
		/*      Write an Arc.                                                   */
		/* -------------------------------------------------------------------- */
		    double	LengthInDegrees,BackAZ,ForAZ, Radius;
			DPOINT	RP;

			fgetstring (str,256,Fid);line++;
			sscanf (str,"%Flf %Flf %Flf %Flf %Flf %Flf",&RP.x,&RP.y,&Radius,&LengthInDegrees,&BackAZ,&ForAZ);
			psMembers[nAreaElements] = DGNCreateArcElem2D( hNewDGN, DGNT_ARC, 
										 RP.x,RP.y, Radius, Radius, 
										 0,BackAZ,-LengthInDegrees);
			AddMSLink (hNewDGN,psMembers[nAreaElements],MSLinkValues);
			DGNUpdateElemCore( hNewDGN, psMembers[nAreaElements], Level, 0, Color,Weight, Style );
			if (!InArea)
			{
				DGNWriteElement( hNewDGN, psMembers[nAreaElements]);
				DGNFreeElement( hNewDGN, psMembers[nAreaElements]);
			}
			else
				nAreaElements++;
		}
			break;
		case 4://text
		/* -------------------------------------------------------------------- */
		/*      Write some text.                                                */
		/* -------------------------------------------------------------------- */
		{
			double	OrigX, OrigY, Height, Rotation;
			int		Font, nText;
			//DGNInfo	*psDGN = (DGNInfo *) hNewDGN;


			fgetstring (str,128,Fid);line++;
			nText = atoi (str);
			while (nText--)
			{
				fgetstring (str,128,Fid);line++;
				sscanf (str,"%Flf %Flf %i %Flf %Flf",&OrigX,&OrigY,&Font,&Height,&Rotation);
				//DGNInverseTransformPoint( psDGN, &sOrigin );

				fgetstring (str,512,Fid);line++;
				psMembers[nAreaElements] = DGNCreateTextElem( hNewDGN, str, 
											Font, DGNJ_CENTER_CENTER,  Height,Height, Rotation,NULL,
											OrigX,OrigY, 0.0 );
				AddMSLink (hNewDGN,psMembers[nAreaElements],MSLinkValues);
				DGNUpdateElemCore( hNewDGN, psMembers[nAreaElements], Level, 0, Color,Weight, Style );
				if (!InArea)
				{
					DGNWriteElement( hNewDGN, psMembers[nAreaElements]);
					DGNFreeElement( hNewDGN, psMembers[nAreaElements]);
				}
				else
					nAreaElements++;
			}
		}
			break;

		/* -------------------------------------------------------------------- */
		/*      Write a complex shape consisting of two line strings.           */
		/* -------------------------------------------------------------------- */
		case 6://area
			psLine = DGNCreateComplexHeaderFromGroup( hNewDGN, 
													  DGNT_COMPLEX_SHAPE_HEADER,
													  nAreaElements, psMembers );
			AddMSLink (hNewDGN,psLine,MSLinkValues);
			DGNUpdateElemCore( hNewDGN, psLine, Level, 0, Color, Weight, 0 );

			DGNAddShapeFillInfo( hNewDGN, psLine, Color );

			DGNWriteElement( hNewDGN, psLine );
			for (i=0;i<nAreaElements;i++)
				DGNWriteElement( hNewDGN, psMembers[i] );

			DGNFreeElement( hNewDGN, psLine );
			for (i=0;i<nAreaElements;i++)
				DGNFreeElement( hNewDGN, psMembers[i] );
			nAreaElements = 0;
			break;
		case 99: //color table
			{
				GByte ColorTable[256][3];
				int	r,g,b;
				for (i=0;i<256;i++)
				{
					fgetstring (str,64,Fid);line++;
					sscanf (str,"%i,%i,%i",&r,&g,&b);
					ColorTable[i][0] = r;
					ColorTable[i][1] = g;
					ColorTable[i][2] = b;
				}
				//psLine = DGNCreateColorTableElem(hNewDGN,0,ColorTable);
     			//DGNWriteElement( hNewDGN, psLine );
				//DGNFreeElement( hNewDGN, psLine );
			}
			break;
		default:
			ii=1;
			break;
		}
	}
	GSSiClose (Fid);
/* -------------------------------------------------------------------- */
/*      Close it.                                                       */
/* -------------------------------------------------------------------- */
    DGNClose( hNewDGN );

    return 0;
}


