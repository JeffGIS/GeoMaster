typedef unsigned char       BYTE;
typedef	int					*LPINT;
typedef BYTE				*LPBYTE;

typedef	struct	{int	nrow,ncol;
			int	left,bottom;
			int	cellw,cellh;
			int	ifacnotused;//always use 1000000
			int	GridOffset;
			double	fac;
			}STATEGRIDHEADER;
typedef STATEGRIDHEADER	*LPSTATEGRIDHEADER;

static	LPBYTE	pData=0;

#define	SearchRadius	0.0004 //controls distance from border at which negative state values are returned

static	int	SearchOffsetsLat[8]={0,1,1,1,0,-1,-1,-1};
static	int	SearchOffsetsLon[8]={-1,-1,0,1,1,1,0,-1};
static	BYTE	CellArrayRow[256];

void GetStateFromLatLonInit (LPBYTE pDataInit);
int GetStateFromLatLon (int lat,int lon,LPINT StateOrProvinceArray,LPINT TZArray);

int GetStateGridOffset (int irow,int icol,int nrow,int ncol,LPINT pOffsetArray);
int GetAreaID (int ilat,int ilon);
void ExpandRow (int Type,LPBYTE CellArrayRow,LPBYTE *CompressedCell,LPBYTE States);
int	GetPNCellArrayValue (LPBYTE CompressedCell,int subrow,int subcol);

void GetStateFromLatLonInit (LPBYTE pDataInit)
//	Load the data file into memory and pass its address to this routine.
//	Must be called once before making calls to GetStateFromLatLon
{
	LPSTATEGRIDHEADER	pHead;
	int	iSearchRadius, i;
	
	pData = pDataInit;
	pHead = (LPSTATEGRIDHEADER)pData;

	iSearchRadius = SearchRadius * 10000000;//pHead->ifac;
	if (SearchOffsetsLat[1] == 1)
	for (i=0;i<8;i++)
	{
		SearchOffsetsLat[i] *= iSearchRadius;
		SearchOffsetsLon[i] *= iSearchRadius;
	}
	return;
}

int StateFromStateID (int StateID)
{
	if (StateID == 174)
		return 73;
	if (StateID > 100)
		return StateID - 100;
	return StateID;
}

int TZFromStateID (int StateID)
{
	switch (StateID)
	{
	case 1:			//	ALABAMA
		return -6;
	case 2:			//	ALASKA
		return -9;
	case 4:			//	ARIZONA
		return -7;
	case 5:			//	ARKASAS
		return -6;
	case 6:			//	CALIFORNIA
		return -8;
	case 8:			//	COLORADO
		return -7;
	case 9:			//	CONNECTICUT
		return -5;
	case 10:		//	DELAWARE
		return -5;
	case 11:		//	DISTRICT OF COLUMBIA
		return -5;
	case 12:		//	FLORIDA
		return -6;
	case 112:		//	FLORIDA (Eastern)
		return -5;
	case 13:		//	GEORGIA
		return -5;
	case 15:		//	HAWAII
		return -10;
	case 16:		//	IDAHO
		return -8;
	case 116:		//	IDAHO (mountain)
		return -7;
	case 17:		//	ILLINOIS
		return -6;
	case 18:		//	INDIANA
		return -5;
	case 118:		//	INDIANA (Central)
		return -6;
	case 19:		//	IOWA
		return -6;
	case 20:		//	KANSAS
		return -7;
	case 120:		//	KANSAS (Central)
		return -6;
	case 21:		//	KENTUCKY
		return -6;
	case 121:		//	KENTUCKY (Eastern)
		return -5;
	case 22:		//	LOUISIANA
		return -6;
	case 23:		//	MAINE
		return -5;
	case 24:		//	MARYLAND
		return -5;
	case 25:		//	MASSACHUSETTS
		return -5;
	case 26:		//	MICHIGAN
		return -6;
	case 126:		//	MICHIGAN (Eastern)
		return -5;
	case 27:		//	MINNESOTA
		return -6;
	case 28:		//	MISSISSIPPI
		return -6;
	case 29:		//	MISSOURI
		return -6;
	case 30:		//	MONTANA
		return -7;
	case 31:		//	NEBRASKA
		return -7;
	case 131:		//	NEBRASKA (Central)
		return -6;
	case 32:		//	NEVADA
		return -8;
	case 33:		//	NEW HAMPSHIRE
		return -5;
	case 34:		//	NEW JERSEY
		return -5;
	case 35:		//	NEW MEXICO
		return -7;
	case 36:		//	NEW YORK
		return -5;
	case 37:		//	NORTH CAROLINA
		return -5;
	case 38:		//	NORTH DAKOTA
		return -7;
	case 138:		//	NORTH DAKOTA (Central)
		return -6;
	case 39:		//	OHIO
		return -5;
	case 40:		//	OKLAHOMA
		return -6;
	case 41:		//	OREGON
		return -8;
	case 141:		//	OREGON (Mountain)
		return -7;
	case 42:		//	PENNSYLVANIA
		return -5;
	case 44:		//	RHODE ISLAND
		return -5;
	case 45:		//	SOUTH CAROLINA
		return -5;
	case 46:		//	SOUTH DAKOTA
		return -7;
	case 146:		//	SOUTH DAKOTA (Central)
		return -6;
	case 47:		//	TENNESSEE
		return -6;
	case 147:		//	TENNESSEE (Eastern)
		return -5;
	case 48:		//	TEXAS
		return -6;
	case 148:		//	TEXAS (Mountain)
		return -7;
	case 49:		//	UTAH
		return -7;
	case 50:		//	VERMONT
		return -5;
	case 51:		//	VIRGINIA
		return -5;
	case 53:		//	WASHINGTON
		return -8;
	case 54:		//	WEST VIRGINIA
		return -5;
	case 55:		//	WISCONSIN
		return -6;
	case 56:		//	WYOMING
		return -7;
	case 61:		//	Alberta
		return -7;
	case 62:		//	British Columbia
		return -8;
	case 162:		//	British Columbia (Mountain)
		return -7;
	case 63:		//	Manitoba
		return -6;
	case 64:		//	New Brunswick
		return -4;
	case 65:		//	Newfoundland
		return -3;
	case 66:		//	Nova Scotia
		return -4;
	case 67:		//	Ontario
		return -6;
	case 167:		//	Ontario (Eastern)
		return -5;
	case 68:		//	Quebec
		return -5;
	case 168:		//	Quebec (Atlantic)
		return -4;
	case 69:		//	Saskatchewan
		return -6;
	case 70:		//	Prince Edward Island
		return -4;
	case 71:		//	Yukon
		return -8;
	case 72:		//	NorthWest Territories
		return -7;
	case 73:		//	Inuet
		return -7;
	case 173:		//	Inuet (Central)
		return -6;
	case 174:		//	Inuet (Eastern)
		return -5;
	case 75:		//	Mexico
		return -6;
	default:
		return 0;
	}
}

int GetStateFromLatLon (int lat,int lon,LPINT StateOrProvinceArray,LPINT TZArray)
// Determines the state, province (or Mexico) the supplied lat/lon is in and places
// its code (from the following table) into the first element of the StateOrProvinceArray.
// Also puts the timezone into the TZArray.
// If the point is within the distance determined by the SearchRadius value (about 150 feet) of
// one or more borders the bordering states will also be placed into the StateOrProvinceArray.
// The supplied lat/lon must be in decimal degrees with the longitude being a negative value.
// The return value is the number of elements in the StateOrProvinceArray.
// The StateOrProvinceArray should be defined with at least 4 elements in the calling routine. 
/*
STATEID	STATENAME			STATEABRV
	1	ALABAMA					AL
	2	ALASKA					AK
	4	ARIZONA					AZ
	5	ARKANSAS				AR
	6	CALIFORNIA				CA
	8	COLORADO				CO
	9	CONNECTICUT				CT
	10	DELAWARE				DE
	11	DISTRICT OF COLUMBIA	DC
	12	FLORIDA					FL
	13	GEORGIA					GA
	15	HAWAII					HI
	16	IDAHO					ID
	17	ILLINOIS				IL
	18	INDIANA					IN
	19	IOWA					IA
	20	KANSAS					KS
	21	KENTUCKY				KY
	22	LOUISIANA				LA
	23	MAINE					ME
	24	MARYLAND				MD
	25	MASSACHUSETTS			MA
	26	MICHIGAN				MI
	27	MINNESOTA				MN
	28	MISSISSIPPI				MS
	29	MISSOURI				MO
	30	MONTANA					MT
	31	NEBRASKA				NE
	32	NEVADA					NV
	33	NEW HAMPSHIRE			NH
	34	NEW JERSEY				NJ
	35	NEW MEXICO				NM
	36	NEW YORK				NY
	37	NORTH CAROLINA			NC
	38	NORTH DAKOTA			ND
	39	OHIO					OH
	40	OKLAHOMA				OK
	41	OREGON					OR
	42	PENNSYLVANIA			PA
	44	RHODE ISLAND			RI
	45	SOUTH CAROLINA			SC
	46	SOUTH DAKOTA			SD
	47	TENNESSEE				TN
	48	TEXAS					TX
	49	UTAH					UT
	50	VERMONT					VT
	51	VIRGINIA				VA
	53	WASHINGTON				WA
	54	WEST VIRGINIA			WV
	55	WISCONSIN				WI
	56	WYOMING					WY
	61	Alberta					ALB
	62	British Columbia		BRC
	63	Manitoba				MAN
	64	New Brunswick			NBR
	65	Newfoundland			NEW
	66	Nova Scotia				NOV
	67	Ontario					ONT
	68	Quebec					QUE
	69	Saskatchewan			SAS
	70	Prince Edward Island	PEI
	71	Yukon					YUK
	72	NorthWest Territories	NWT
	73	Inuet
	75	Mexico					MEX
	99	Undefined

  Time Zones
	-10	Hawaii/Aleutian
	-9	Alaska
	-8	Pacific
	-7	Mountain
	-6	Central
	-5	Eastern
	-4	Atlantic
	-3	NewFoundland
*/
{
	int nHits=1;
	int	AreaID, i, j;
	LPSTATEGRIDHEADER	pHead;

	if (!pData)
	{
		StateOrProvinceArray[0] = 0;
		return 0;
	}
	pHead = (LPSTATEGRIDHEADER)pData;
	
	if (lon <= (-1410000000))
	{
		TZArray[0] = -10;
		if (lat < 500000000)
			StateOrProvinceArray[0] = 15; // return Hawaii
		else
			StateOrProvinceArray[0] = 2; // return Alaska
		return 1;
	}
	AreaID = GetAreaID (lat,lon);
	StateOrProvinceArray[0] = StateFromStateID (AreaID);
	TZArray[0] = TZFromStateID (AreaID);
	for (i=0;i<8;i++)
	{
		AreaID = GetAreaID (lat+SearchOffsetsLat[i],lon+SearchOffsetsLon[i]);
		for (j=0;j<nHits;j++)
			if (StateFromStateID (AreaID) == StateOrProvinceArray[j])
				goto Next;
		StateOrProvinceArray[nHits] = StateFromStateID (AreaID);
		TZArray[nHits++] = TZFromStateID (AreaID);
		if (nHits > 3)
			break;
Next:;
	}

	return nHits;
}

int GetStateGridOffset (int irow,int icol,int nrow,int ncol,LPINT pOffsetArray)
// Determines the primary grid cell in which the point is located and returns the
// state or province code (as a negative number) if the primary grid cell is totally
// within a single state or province or the offset to the secondary grid data.
{
	int	RowBeg;
	LPINT	pRun, pState;


	if (irow < 0)
		return -75;
	if (icol < 0)
		return -2;
	if (irow >= nrow || icol >= ncol)
		return -99;
	RowBeg = pOffsetArray[irow];
	pRun   = &pOffsetArray[RowBeg];
	pState = &pOffsetArray[RowBeg+1];
	while (icol+1 > *pRun)
	{
		icol   -= *pRun;
		pRun   += 2;
		pState += 2;
	}
	return *pState;
}

void ExpandRow (int Type,LPBYTE CellArrayRow,LPBYTE *CompressedCell,LPBYTE States)
// Decompresses a row in the secondary grid.
{
	int	RowLen = *(*CompressedCell)++ + 1;
	int	icol = 0, RunLen;
	int	StateIndex, State;

	switch (Type)
	{
	case 1:
		StateIndex = 0;
		while (RowLen--)
		{
			RunLen = *(*CompressedCell)++ + 1;
			while (RunLen--)
				CellArrayRow[icol++] = States[StateIndex];
			StateIndex = !StateIndex;
		}
		break;
	case 2:
		while (RowLen--)
		{
			RunLen = *(*CompressedCell)++ + 1;
			State  = *(*CompressedCell)++;
			RowLen--;
			while (RunLen--)
				CellArrayRow[icol++] = State;
		}
		break;
	}
	return;
}

int	GetPNCellArrayValue (LPBYTE CompressedCell,int subrow,int subcol)
// Gets the value of the secondary grid cell specified by the subrow and subcol parameters
{
	int	Type,irow=0, nRowsInType;
	int	CurRowInType, nRepeats, FromRow;
	BYTE	States[2];

	subrow++;
	while (irow < subrow)
	{
		Type = *CompressedCell++;
		nRowsInType = *CompressedCell++ + 1;
		CurRowInType = 0;
		if (Type == 1)
		{
			States[0] = *CompressedCell++;
			States[1] = *CompressedCell++;
		}
		while (CurRowInType < nRowsInType && irow < subrow)
		{
			nRepeats = *CompressedCell++ + 1;
			ExpandRow (Type,CellArrayRow,&CompressedCell,States);
			FromRow = irow++;
			CurRowInType++;
			nRepeats--;
			while (nRepeats--)
			{
				irow++;
				CurRowInType++;
			}
		}
	}
	return CellArrayRow[subcol];
}

int GetAreaID (int ilat,int ilon)
// returns the state/prov code for a specified lat/lon
{
	int	AreaID;
	LPBYTE	pLoc;
	LPSTATEGRIDHEADER	pHead = (LPSTATEGRIDHEADER)pData;
	int	irow = (ilat - pHead->bottom) / pHead->cellh;
	int	icol = (ilon - pHead->left) / pHead->cellw;
	int	Offset = GetStateGridOffset (irow,icol,pHead->nrow,pHead->ncol,(LPINT)(pData+pHead->GridOffset));
	int	cell_left = pHead->left + icol * pHead->cellw;
	int	cell_bottom = pHead->bottom + irow * pHead->cellh;
	int	subrow, subcol;
	
	if (ilat < pHead->bottom)
		return 75;
	if (ilon < pHead->left)
		return 2;
	if (Offset < 0)
		return -Offset;

	subrow = ((ilat - cell_bottom)*256) / pHead->cellh;
	subcol = ((ilon - cell_left)*256) / pHead->cellh;
	pLoc = (LPBYTE)(pData + Offset); 
	AreaID = GetPNCellArrayValue (pLoc,subrow,subcol);
	return AreaID ;
}
