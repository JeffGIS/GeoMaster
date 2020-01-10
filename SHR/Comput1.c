#include "graphint.h"
#include "dibapi.h"

    static	long	counts[3]={0,0,0};

typedef USHORT	COLORREF16;
typedef COLORREF16	*LPCOLORREF16;

static	BYTE	nBitsInByte[256];

#include "gmextern.h"   

static  DWORD    Mask[32] = {1,2,4,8,16,32,64,128,
							 256,512,1024,2048,0x1000,0x2000,0x4000,0x8000,
							 0x10000,0x20000,0x40000,0x80000,0x100000,0x200000,0x400000,0x800000,
							 0x100000,0x200000,0x400000,0x800000,0x1000000,0x2000000,0x4000000,0x8000000}; 

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
float DotVec2(vec2* vec0, vec2* vec1)
{
	return vec0->x*vec1->x + vec0->y*vec1->y;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
float DotVec3(const vec3* vec0, const vec3* vec1)
{
	return vec0->x*vec1->x + vec0->y*vec1->y + vec0->z*vec1->z;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
float Lerp(float f0, float f1, float t)
{
	return f0 + t*(f1-f0);
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  LerpVec2(vec2* out_lerpedPos, vec2* p0, vec2* p1, float t)
{
	const float outX = p0->x + t*(p1->x-p0->x);
	const float outY = p0->y + t*(p1->y-p0->y);
	
	if(out_lerpedPos)
	{
		out_lerpedPos->x = outX;
		out_lerpedPos->y = outY;
	}
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  LerpVec3(vec3* out_lerpedPos, const vec3* p0, const vec3* p1, float t)
{
	const float outX = p0->x + t*(p1->x-p0->x);
	const float outY = p0->y + t*(p1->y-p0->y);
	const float outZ = p0->z + t*(p1->z-p0->z);
	
	if(out_lerpedPos)
	{
		out_lerpedPos->x = outX;
		out_lerpedPos->y = outY;
		out_lerpedPos->z = outZ;
	}
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
float MagnitudeVec2(vec2* vec)
{
	return sqrtf(DotVec2(vec,vec));
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
float DistVec3_2D(const vec3* vec0, const vec3* vec1)
{
	vec3 vec2D;
	SubVec3(&vec2D, vec0, vec1);
	vec2D.y = 0.0f;
	return MagnitudeSqVec3(&vec2D);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
float DistVec3(const vec3* vec0, const vec3* vec1)
{
	return sqrtf(DistSqVec3(vec0,vec1));
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
float DistSqVec3(const vec3* vec0, const vec3* vec1)
{
	vec3 distVec;
	SubVec3(&distVec, vec0, vec1);
	return MagnitudeSqVec3(&distVec);
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
BOOL PosIsBehind(const vec3* frontPos, const vec3* frontAt, const vec3* behindPos)
{
	vec3 dirVec;
	SubVec3(&dirVec, behindPos, frontPos);
	
	return DotVec3(frontAt, &dirVec) <= 0.0f;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
float MagnitudeSqVec3(const vec3* vec)
{
	return DotVec3(vec,vec);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
float MagnitudeVec3(const vec3* vec)
{
	return sqrtf(MagnitudeSqVec3(vec));
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  NormalizeVec2(vec2* vec)
{
	const float length = MagnitudeVec2(vec);
	vec->x/=length;
	vec->y/=length;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  NormalizeVec3(vec3* vec)
{
	const float length = MagnitudeVec3(vec);
	vec->x/=length;
	vec->y/=length;
	vec->z/=length;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  SubVec2(vec2* out_resultVec, vec2* p0, vec2* p1)
{
	out_resultVec->x = p0->x-p1->x;
	out_resultVec->y = p0->y-p1->y;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  SubVec2_Self(vec2* out_resultVec, vec2* p)
{
	out_resultVec->x -= p->x;
	out_resultVec->y -= p->y;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  SubVec3(vec3* out_resultVec, const vec3* p0, const vec3* p1)
{
	out_resultVec->x = p0->x-p1->x;
	out_resultVec->y = p0->y-p1->y;
	out_resultVec->z = p0->z-p1->z;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  ScaleVec2_Self(vec2* vec, float scale)
{
	vec->x *= scale;
	vec->y *= scale;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  ScaleVec2(vec2* out_resultVec, vec2* vec, float scale)
{
	out_resultVec->x = vec->x*scale;
	out_resultVec->y = vec->y*scale;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  ScaleVec3_Self(vec3* vec, float scale)
{
	vec->x *= scale;
	vec->y *= scale;
	vec->z *= scale;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  ScaleVec3(vec3* out_resultVec, const vec3* vec, float scale)
{
	out_resultVec->x = vec->x*scale;
	out_resultVec->y = vec->y*scale;
	out_resultVec->z = vec->z*scale;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  AddScaledVec2_Self(vec2* out_resultVec, vec2* vec, float scale)
{
	out_resultVec->x += vec->x*scale;
	out_resultVec->y += vec->y*scale;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  AddScaledVec2(vec2* out_resultVec, vec2* vec0, vec2* vec1, float scale)
{
	out_resultVec->x = vec0->x + vec1->x*scale;
	out_resultVec->y = vec0->y + vec1->y*scale;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  SubScaledVec2_Self(vec2* out_resultVec, vec2* vec, float scale)
{
	out_resultVec->x -= vec->x*scale;
	out_resultVec->y -= vec->y*scale;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  SubScaledVec2(vec2* out_resultVec, vec2* vec0, vec2* vec1, float scale)
{
	out_resultVec->x = vec0->x - vec1->x*scale;
	out_resultVec->y = vec0->y - vec1->y*scale;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  AddVec2_Self(vec2* out_resultVec, vec2* vec)
{
	out_resultVec->x += vec->x;
	out_resultVec->y += vec->y;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  AddVec2(vec2* out_resultVec, vec2* vec0, vec2* vec1)
{
	out_resultVec->x = vec0->x + vec1->x;
	out_resultVec->y = vec0->y + vec1->y;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  CopyVec2(vec2* out_result, vec2* point)
{
	out_result->x = point->x;
	out_result->y = point->y;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  CopyVec3(vec3* out_result, const vec3* point)
{
	out_result->x = point->x;
	out_result->y = point->y;
	out_result->z = point->z;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  CreatePlaneFromPointsVec2(struct PlaneVec2* out_resultPlane, vec2* p0, vec2* p1)
{
	float tempX;
	//Copy point
	CopyVec2(&out_resultPlane->point, p0);
	
	//Create normal
	SubVec2(&out_resultPlane->normal,p1,p0);
	NormalizeVec2(&out_resultPlane->normal);
	
	tempX = out_resultPlane->normal.x;
	
	//Rotate 90 degrees (normal faces to the right of the direction)
	out_resultPlane->normal.x = out_resultPlane->normal.y;
	out_resultPlane->normal.y = -tempX;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
BOOL  PointInsidePlaneVec2(vec2* point,struct PlaneVec2* plane)
{
	vec2 dirVec;
	SubVec2(&dirVec,point,&plane->point);
	return DotVec2(&dirVec,&plane->normal) < 0.0f;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
float AngleBetweenVec2(vec2* vec0, vec2* vec1)
{
	return (float)(atan2(vec1->y,vec1->x) - atan2(vec0->y,vec0->x));
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
float AngleBetweenVec3(vec3* vec0, vec3* vec1)
{
	//TODO: get the angle
	return 0;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  CrossVec3(vec3* out_resultVec, const vec3* vec0, const vec3* vec1)
{
	const float x = vec0->y*vec1->z - vec1->y*vec0->z;
	const float y = vec0->z*vec1->x - vec1->z*vec0->x;
	const float z = vec0->x*vec1->y - vec1->x*vec0->y;
	
	out_resultVec->x = x;
	out_resultVec->y = y;
	out_resultVec->z = z;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  SetVec3(vec3* out_resultVec,float x, float y, float z)
{
	out_resultVec->x = x;
	out_resultVec->y = y;
	out_resultVec->z = z;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  AddVec3_Self(vec3* out_resultVec, vec3* vec)
{
	out_resultVec->x += vec->x;
	out_resultVec->y += vec->y;
	out_resultVec->z += vec->z;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  AddVec3(vec3* out_resultVec, vec3* vec0, vec3* vec1)
{
	out_resultVec->x = vec0->x+vec1->x;
	out_resultVec->y = vec0->y+vec1->y;
	out_resultVec->z = vec0->z+vec1->z;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  AddScaledVec3_Self(vec3* out_resultVec, const vec3* vec, float scale)
{
	out_resultVec->x += vec->x*scale;
	out_resultVec->y += vec->y*scale;
	out_resultVec->z += vec->z*scale;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  AddScaledVec3(vec3* out_resultVec, const vec3* vec0, const vec3* vec1, float scale)
{
	out_resultVec->x = vec0->x+vec1->x*scale;
	out_resultVec->y = vec0->y+vec1->y*scale;
	out_resultVec->z = vec0->z+vec1->z*scale;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  SubScaledVec3_Self(vec3* out_resultVec, vec3* vec, float scale)
{
	out_resultVec->x -= vec->x*scale;
	out_resultVec->y -= vec->y*scale;
	out_resultVec->z -= vec->z*scale;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void  SubScaledVec3(vec3* out_resultVec, vec3* vec0, vec3* vec1, float scale)
{
	out_resultVec->x = vec0->x-vec1->x*scale;
	out_resultVec->y = vec0->y-vec1->y*scale;
	out_resultVec->z = vec0->z-vec1->z*scale;
}

vec3 VectorFromPoints (vec3 * p1, vec3 * p2)
{
	vec3 outVector;

	outVector.x = p2->x - p1->x;
	outVector.y = p2->y - p1->y;
	outVector.z = p2->z - p1->z;
	return outVector;
}
vec3 sVectorFromPoints (svec3 * p1, svec3 * p2)
{
	vec3 outVector;

	outVector.x = p2->x - p1->x;
	outVector.y = p2->y - p1->y;
	outVector.z = p2->z - p1->z;
	return outVector;
}

vec3 GetTriangleNormal (vec3 * vertex1,vec3 * vertex2,vec3 * vertex3)
{
	vec3 vNormal;
	vec3 v1;
	vec3 v2;

	v1 = VectorFromPoints (vertex2,vertex1);
	v2 = VectorFromPoints (vertex3,vertex1);
	
	CrossVec3(&vNormal, &v1, &v2);
	NormalizeVec3(&vNormal);

    return vNormal;
}

double ComputeMaxSlope (double frontSlope,double sideSlope)
{
    double rtn=0;
    vec3 vertex1, vertex2, vertex3;
    vec3 normal;
    double xyDist;
    
    vertex1.x = vertex1.y = vertex1.z = 0.0f;
    vertex2.x = 1.0f;
    vertex2.y = 0.0f;
    vertex2.z = sideSlope/100.0;
    vertex3.x = 0.0f;
    vertex3.y = 1.0f;
    vertex3.z = frontSlope/100.0;
    normal = GetTriangleNormal(&vertex1,&vertex2,&vertex3);
    xyDist = LDIST (0.0,0.0,normal.x,normal.y);
    if (fabs (normal.z) > 0.0000001)
        rtn = 100.0 * fabs (xyDist / normal.z);
    else
        rtn = -9999.0;
       
    return rtn;
}

vec3 sGetTriangleNormal (svec3 * vertex1,svec3 * vertex2,svec3 * vertex3,double metersperpixel)
{
	vec3 vNormal;
	vec3 v1;
	vec3 v2;
	vec3 vertex1f, vertex2f, vertex3f;
	float f = metersperpixel;
	
	vertex1f.x = vertex1->x * f;
	vertex1f.y = vertex1->y * f;
	vertex1f.z = FTM * vertex1->z / 8.0f;

	vertex2f.x = vertex2->x * f;
	vertex2f.y = vertex2->y * f;
	vertex2f.z = FTM * vertex2->z / 8.0f;

	vertex3f.x = vertex3->x * f;
	vertex3f.y = vertex3->y * f;
	vertex3f.z = FTM * vertex3->z / 8.0f;

	/*vertex1f.x = 0;
	vertex1f.y = 5;
	vertex1f.z = 0;
	vertex2f.x = -1;
	vertex2f.y = 0;
	vertex2f.z = 1;
	vertex3f.x = 1;
	vertex3f.y = 0;
	vertex3f.z = 1;*/

	v1 = VectorFromPoints (&vertex2f,&vertex1f);
	v2 = VectorFromPoints (&vertex3f,&vertex1f);
	
	CrossVec3(&vNormal, &v1, &v2);
	NormalizeVec3(&vNormal);

    return vNormal;
}

double PercentOfItemInHighlightAreas (int item)
{
    HANDLE	hPoly=0; 
	HPDPOINT	pDPoint;
	int		npnts;
	double	pct;

	if (!GetPolyPoints ((LPPICKDATAHEADER)&PickList[item],FALSE,&npnts,&hPoly))
		return -1; 
	pDPoint = GlobalLock (hPoly);
	pct = PercentOfPolyInHighlightAreas (PickList[item].Type,&PickList[item].Rect,npnts,pDPoint);
	GSSiGlobUlFree (&hPoly);
	return pct;
}

double PercentOfPolyInHighlightAreas (int PolyType,LPMNMXCORD pBounds,int npnts,HPDPOINT pDPoint)
#if ENABLETRACE
{GSSiEnterProg (1379);
#endif
{
    DWORD   i,n, ibit,bytesperrow,wordsperrow; 
    HDC		hDC,hDCMain;
    HBITMAP	hBMItem=0, hBMArea=0, hBMOld=0; 
    double	Factor, pct=0, basetopixel; 
    UINT	Height,Width,irow,icol;
    DWORD	loc;  
    HANDLE	hPoint = 0;
	HPPOINT	pPoint;
    long	memsize,ii;
	HPBYTE	array_item, array_area, pByteItem, pByteArea;  
	LPDWORD	pDwordItem, pDwordArea;
	BOOL	savebm=FALSE;  
    BITMAP	bm;   
	HANDLE	hBitmapInfo = GSSiGlobAlloc (0,GHND,sizeof(BITMAPINFO)+sizeof(RGBQUAD));
	LPBITMAPINFO	BitmapInfo = GlobalLock (hBitmapInfo);
	int		st;
	HBRUSH	hOldBrush;
	HPEN	hOldPen, hPen;
	int		AreaNum,nHighlightAreaPoints;
	HANDLE	hHighlightArea;
	int		Type, nPoly;
	double	Offset;
	int		narea, nitem, nintersect, OpenClose;
	MNMXCORD	Bounds=*pBounds;

	InflateBounds (&Bounds,P_TOL);
	Factor = (Bounds.ymx - Bounds.ymn) / (Bounds.xmx - Bounds.xmn); 
	if (Factor > 1)
		Height = 2000;
	else
		Height = IDNINT (2000 * Factor);
	Width = IDNINT (Height / Factor);
	if (Width % 32)
		Width += 32 - (Width % 32);
	Height = Width * Factor + 1;
	hDCMain = GetDC (hWndMain);
    hDC = CreateCompatibleDC(hDCMain); 
	ReleaseDC (hWndMain,hDCMain);
    memset (BitmapInfo,0,sizeof(BITMAPINFO));
	BitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFO);
	BitmapInfo->bmiHeader.biBitCount = 1;
	BitmapInfo->bmiHeader.biHeight = Height;
	BitmapInfo->bmiHeader.biWidth = Width;
	BitmapInfo->bmiHeader.biPlanes = 1;
	BitmapInfo->bmiHeader.biSizeImage = (Width * Height) / 8;
	BitmapInfo->bmiHeader.biClrImportant = 0;
	BitmapInfo->bmiHeader.biClrUsed = 2;
	BitmapInfo->bmiColors[0].rgbReserved = 0;
	BitmapInfo->bmiColors[1].rgbReserved = 0;
	BitmapInfo->bmiColors[0].rgbBlue = BitmapInfo->bmiColors[0].rgbRed = BitmapInfo->bmiColors[0].rgbGreen = 0;
	BitmapInfo->bmiColors[1].rgbBlue = BitmapInfo->bmiColors[1].rgbRed = BitmapInfo->bmiColors[1].rgbGreen = 255;
	hBMItem = CreateDIBSection(hDC,BitmapInfo,0,&array_item,0,0);
	hBMArea = CreateDIBSection(hDC,BitmapInfo,0,&array_area,0,0);
    GetObject(hBMItem, sizeof(bm), (LPSTR)&bm);
	BitmapInfo->bmiHeader.biSizeImage = bm.bmWidthBytes * bm.bmHeight;
	memset (array_item,0,BitmapInfo->bmiHeader.biSizeImage);
	memset (array_area,0,BitmapInfo->bmiHeader.biSizeImage);
    hBMOld = SelectObject(hDC,hBMItem);
	SetMapMode    ( hDC, MM_ISOTROPIC );
    SetWindowOrgEx  ( hDC, 0, 0,0 );
    SetViewportOrgEx( hDC, 0, 0,0 );    
  	SetWindowExtEx  ( hDC, Width, Height,0 ); 
	SetViewportExtEx( hDC, Width, Height,0 );  
	hPoint = GSSiGlobAlloc ( 294,GMEM_MOVEABLE,npnts * sizeof(POINT));
	pPoint = (HPPOINT)GlobalLock(hPoint);
	basetopixel = Width / (Bounds.xmx - Bounds.xmn);
	for (i=0;i<npnts;i++)
	{
		pPoint[i].x = IDNINT ((pDPoint[i].x - Bounds.xmn) * basetopixel);
		pPoint[i].y = IDNINT ((pDPoint[i].y - Bounds.ymn) * basetopixel);
	}
	hOldBrush = SelectObject (hDC,GetStockObject(BLACK_BRUSH));
    st = Polygon (hDC,pPoint,npnts);
 	GdiFlush();
	GSSiGlobUlFree (&hPoint);  
	SelectObject(hDC,hBMArea);
	SetMapMode    ( hDC, MM_ISOTROPIC );
    SetWindowOrgEx  ( hDC, 0, 0,0 );
    SetViewportOrgEx( hDC, 0, 0,0 );    
  	SetWindowExtEx  ( hDC, Width, Height,0 ); 
	SetViewportExtEx( hDC, Width, Height,0 );  
	AreaNum = 1;
	OpenClose = 1;
	while (hHighlightArea = GetNextHighlightArea (AreaNum++,0,&Type,&nHighlightAreaPoints,&nPoly,0,&Offset,OpenClose))
	{
		LPMNMXCORD	pRect = GlobalLock (hHighlightArea);
		LPDPOINT WPoint = (LPDPOINT)(pRect+1);
		int	width = IDNINT (Offset * 2 * basetopixel);
		LOGBRUSH	lb;
		
		lb.lbStyle = BS_SOLID;
		lb.lbColor = 0;
		lb.lbHatch = 0;
		
		OpenClose = 2;
		switch (Type)
		{
			case 2:
			case 5:
				hPen = ExtCreatePen (PS_GEOMETRIC|PS_SOLID|PS_ENDCAP_FLAT|PS_JOIN_ROUND,width,&lb,0,0);
			break;
			default:
				hPen = CreatePen (PS_SOLID,width,0);
			break;
		}
		hOldPen = SelectObject (hDC,hPen);
		hPoint = GSSiGlobAlloc ( 294,GMEM_MOVEABLE,nHighlightAreaPoints * sizeof(POINT));
		pPoint = (HPPOINT)GlobalLock(hPoint);

		for (i=0;i<nHighlightAreaPoints;i++)
		{
			pPoint[i].x = IDNINT ((WPoint[i].x - Bounds.xmn) * basetopixel);
			pPoint[i].y = IDNINT ((WPoint[i].y - Bounds.ymn) * basetopixel);
		}
		switch (Type)
		{
			case 2:
			case 5:
			    st = Polyline (hDC,pPoint,nHighlightAreaPoints);
			break;
			default:
			    st = Polygon (hDC,pPoint,nHighlightAreaPoints);
			break;
		}
		SelectObject (hDC,hOldPen);
		DeleteObject (hPen);
	    GSSiGlobUlFree (&hPoint); 
		GSSiGlobUlFree (&hHighlightArea);
	}
	GetNextHighlightArea (AreaNum++,0,&Type,&nHighlightAreaPoints,&nPoly,0,&Offset,3);
	SelectObject (hDC,hOldBrush);
	GdiFlush();
	if (hBMOld)    
    	SelectObject(hDC, hBMOld);  
	pByteItem = array_item;
	pByteArea = array_area;
	pDwordItem = (LPDWORD)array_item;
	pDwordArea = (LPDWORD)array_area;
	nitem = 0;
	narea = 0;
	nintersect = 0;
	bytesperrow = Width/8;
	wordsperrow = Width/32;
	/*
	for (irow = 0;irow<Height;irow++) 
	{
    	for (icol = 0;icol<bytesperrow;icol++,pByteItem++,pByteArea++)
		{
			if (*pByteItem)
			{
				for (ibit=0;ibit<8;ibit++)
					if (Mask[ibit] & *pByteItem)
					{
						nitem++;
						if (Mask[ibit] & *pByteArea)
							nintersect++;
					}
			}
			if (*pByteArea)
			{
				for (ibit=0;ibit<8;ibit++)
					if (Mask[ibit] & *pByteArea)
						narea++;

			}
		}
	}
	*/
	for (irow = 0;irow<Height;irow++) 
	{
    	for (icol = 0;icol<wordsperrow;icol++,pDwordItem++,pDwordArea++)
		{
			if (*pDwordItem)
			{
				for (ibit=0;ibit<32;ibit++)
					if (Mask[ibit] & *pDwordItem)
					{
						nitem++;
						if (Mask[ibit] & *pDwordArea)
							nintersect++;
					}
			}
			if (*pDwordArea)
			{
				for (ibit=0;ibit<32;ibit++)
					if (Mask[ibit] & *pDwordArea)
						narea++;

			}
		}
	}
	if (nitem)
		pct = (double)nintersect / (double)nitem;
Exit:
    st = DeleteDC(hDC);

    if (savebm)
	{
//		HPALETTE	hPal = CreateDIBPalette (hBM);
//		hBM = DIBToBitmap (hBM,hPal); 
		HDIB hDib;
		hDib=BitmapToDIB (hBMItem,(HPALETTE) 0,0);
		SaveDIB (hDib,"c:\\testitem.bmp");
		GSSiGlobFree (&hDib);
		hDib=BitmapToDIB (hBMArea,(HPALETTE) 0,0);
		SaveDIB (hDib,"c:\\testarea.bmp");
		GSSiGlobFree (&hDib);
 //   	SaveBitmap (hBM,"c:\\test.bmp",0,0);
	}
    GSSiDeleteObject(&hBMItem);  
    GSSiDeleteObject(&hBMArea);  
	GSSiGlobUlFree (&hBitmapInfo);
{
#if ENABLETRACE
GSSiExitProg (1379);
#endif
    return pct;
}
#if ENABLETRACE
}
#endif
}  

int ShowCounts (int i)
{
	char	str[256];
	long	Tot = counts[0]+counts[1]+counts[2]; 
	
	sprintf (str,"tot %ld:%ld:%ld:%ld",Tot,100*counts[0]/Tot,100*counts[1]/Tot,100*counts[2]/Tot);
	MessageBox (0,str,0,MB_OK);
	return i;
}

float MULREG (double Y[], double X1[], double X2[], int N,LPDOUBLE A,LPDOUBLE B, LPDOUBLE C)

/******* SPECIFICATIONS ***********************************************
C*                                                                    *
C*       PROGRAM SUMMARY                                              *
C*       ------- -------                                              *
C*    MULREG PERFORMS A MULTIPLE LINEAR REGRESSION OF Y ON X1, X2 AND*
C*    THE PARAMETERS OF THE REGRESSION IN A,B,C AND RSQ.              *
C*    THE EQUATION IS OF THE FORM                                     *
C*         Y = A*X1 + B*X2 + C                                        *
C*                                                                    *
C*       ARGUMENT DESCRIPTION                                         *
C*       -------- -----------                                         *
C*    Y       R*8(N) THE Y VALUES                                     *
C*    X1      R*8(N) THE X1 VALUES                                    *
C*    X2      R*8(N) THE X2 VALUES                                    *
C*    N       I*4    THE NUMBER OF Y, X1 AND X2 VALUES                *
C*    A       R*8    THE A VALUE                                      *
C*    B       R*8    THE B VALUE                                      *
C*    C       R*8    THE C VALUE                                      *
C*    RSQ     R*8    THE R-SQUARED VALUE INDICATING THE SIGNIFICANCE  *
C*                   OF THE REGRESSION                                *
C*                                                                    *
C*       AUTHOR                                                       *
C*       ------                                                       *
C*     JEFF SMITH                                                     *
C**********************************************************************
C*/
#if ENABLETRACE
{GSSiEnterProg (355);
#endif
{
      double    D, DY, SMD1SQ, SMD2SQ,
                SMYSQ, SMD1D2, SMD1Y, SMD2Y, YHAT, X1HAT, X2HAT, D1, D2;
      short         I;
      float     RSQ;

      RSQ    = (float)0.0;
      SMD1SQ = 0e0;
      SMD2SQ = 0e0;
      SMYSQ  = 0e0;
      SMD1D2 = 0e0;
      SMD1Y  = 0e0;
      SMD2Y  = 0e0;
      YHAT   = AMEAN (Y,N);
      X1HAT  = AMEAN (X1,N);
      X2HAT  = AMEAN (X2,N);
      for (I=0;I<N;I++)
      {
          D1     = X1[I] - X1HAT;
          D2     = X2[I] - X2HAT;
          DY     = Y[I]  - YHAT;
          SMD1SQ = SMD1SQ + pow (D1,2);
          SMD2SQ = SMD2SQ + pow (D2,2);
          SMD1D2 = SMD1D2 + D1 * D2;
          SMYSQ  = SMYSQ  + pow (DY,2);
          SMD1Y  = SMD1Y  + D1 * DY;
          SMD2Y  = SMD2Y  + D2 * DY;
      }
      D      = SMD1SQ * SMD2SQ - pow (SMD1D2,2);
      if (D == 0E0)
{
#if ENABLETRACE
GSSiExitProg (355);
#endif
      	return (float)(0.0);
}
      *B      = (SMD2SQ * SMD1Y - SMD1D2 * SMD2Y) / D;
      *C      = (SMD1SQ * SMD2Y - SMD1D2 * SMD1Y) / D;
      *A      = YHAT - *B * X1HAT - *C * X2HAT; 
      if (!SMYSQ)
      	RSQ = 1.0;
      else
      	RSQ    = (float)((*B * SMD1Y + *C * SMD2Y) / SMYSQ);
{
#if ENABLETRACE
GSSiExitProg (355);
#endif
      return(RSQ);
}
#if ENABLETRACE
}
#endif
}

double AMEAN (double X[],int N)
#if ENABLETRACE
{GSSiEnterProg (356);
#endif
{
      double SUM;
      short  I;

      SUM    = 0;
      for (I=0;I<N;I++) SUM+=X[I];
{
#if ENABLETRACE
GSSiExitProg (356);
#endif
      return (SUM / N);
}
#if ENABLETRACE
}
#endif
}

static double M12X,M12Y,M23X,M23Y,CL,AZP12,AZ12, AZ23, AZP23,RAD,AZC,AZM,AZT,
              AZDCM,AZDCT,RCX,RCY;

 
BOOL LINFIT (HPDPOINT pPoint,LONG N,LPDOUBLE A,LPDOUBLE B,LPDOUBLE MAXDIF,LPLONG LOFMDF)
#if ENABLETRACE
{GSSiEnterProg (1200);
#endif
{
/*C******* SPECIFICATIONS ************************************************
C*                                                                     *
C*       PROGRAM SUMMARY                                               *
C*       ------- -------                                               *
C*    LINFIT (LINE FIT) FINDS, THROUGH A LEAST SQUARES TECHNIQUE, THE  *
C*    PARAMETERS A AND B OF THE EQUATION                               *
C*                                                                     *
C*                 RAWELV = A + B * RAWDIS                             *
C*                                                                     *
C*    WHICH DESCRIBES THE LINE OF BEST FIT FOR THE N VALUES OF         *
C*    RAWDIS, RAWELV. THE MAXIMUM VERTICAL DISTANCE FROM ANY RAWELV    *
C*    TO THE LINE IS ALSO COMPUTED AND RETURNED ALONG THE LOCATION OF  *
C*    THAT RAWELV IN THE RAWELV ARRAY.                                 *
C*                                                                     *
C*       ARGUMENT DESCRIPTION                                          *
C*       -------- -----------                                          *
C*    X       R*8 (N)  DISTANCES,OR RAWDIS VALUES, OF DATA TO BE FIT.  *
C*    Y       R*8 (N)  ELEVATIONS, OR RAWELV VALUES, OF DATA TO BE FIT.*
C*    N       I*4      NUMBER OF POINTS TO BE FIT                      *
C*    A       R*8      0 DEGREE COEFFICIENT OF THE LINER EQUATION      *
C*    B       R*8      1ST DEGREE COEFFICIENT OF THE LINEAR EQUATION   *
C*    MAXDIF  R*8      THE MAXIMUM DISTANCE DISCUSSED ABOVE            *
C*    LOFMDF  I*4      THE LOCATION OF THE MAXIMUM DIFFERENCE          *
C*                                                                     *
C*       AUTHOR                                                        *
C*       ------                                                        *
C*      JEFF SMITH                                                     *
C*                                                                     *
C***********************************************************************  */
double SUMX=0, SUMY=0, XAVE, YAVE, SMXDYD, SMXDSQ, SUMXY=0, SUMXSQ=0, DIFF;  
double	SUMxy=0, SUMx2=0;
long	I;
HPDPOINT	pPointIn=pPoint;

	for (I=0;I<N;I++,pPoint++)
	{
        SUMX   += pPoint->x;
        SUMY   += pPoint->y;
        SUMXY  += pPoint->x * pPoint->y;
    	SUMXSQ += pPoint->x * pPoint->x;
    }
    XAVE   = SUMX / N;
    YAVE   = SUMY / N;
    SMXDSQ = SUMXSQ - (SUMX * SUMX) / N;
    SMXDYD = SUMXY  - (SUMX * SUMY) / N;
    if (!SMXDSQ)
{
#if ENABLETRACE
GSSiExitProg (1200);
#endif
    	return FALSE;
}
    *B      = SMXDYD / SMXDSQ;
    *A      = YAVE - *B * XAVE;
	if (MAXDIF)
	{
		*LOFMDF = 0;
		*MAXDIF = 0;
	}
    pPoint = pPointIn;
	for (I=0;I<N;I++,pPoint++)
	{
		SUMxy += (pPoint->x - XAVE) * (pPoint->y - YAVE);  
		SUMx2 += pow (pPoint->x - XAVE,2);
	} 
	if (!SUMx2)
{
#if ENABLETRACE
GSSiExitProg (1200);
#endif
		return FALSE;
}
	*B = SUMxy / SUMx2;
    pPoint = pPointIn;
	for (I=0;I<N;I++,pPoint++)
	{
    	DIFF = fabs (pPoint->y - (*A + *B * pPoint->x));
        if (MAXDIF && DIFF > *MAXDIF)
        {
         	*MAXDIF = DIFF;
         	*LOFMDF = I;
        }
    }
{
#if ENABLETRACE
GSSiExitProg (1200);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
}

void XLC(const double *PCX,  const double *PCY,
         const double *RCX,  const double *RCY, const double *CL,
         const double *BPX,  const double *BPY, 
         const double *EPX,  const double *EPY,
         double far *X1, double far *Y1, double far *X2,double far *Y2,
         double far *X3, double far *Y3, short  *NC,short  *NL,
         double *DMN,short  *ICD)
#if ENABLETRACE
{GSSiEnterProg (1201);
#endif
{
//C*
//C******* SPECIFICATIONS ***********************************************
//C*                                                                    *
//C*       PROGRAM SUMMARY                                              *
//C*       ------- -------                                              *
//C*                                                                    *
//C*       ROUTINE XLC COMPUTES INTERSECTION POINTS OR MINIMUM          *
//C*       DISTANT POINTS BETWEEN A CURVE AND A LINE(REAL*8 VERSION)    *
//C*                                                                    *
//C*       ARGUMENT DESCRIPTION                                         *
//C*       -------- -----------                                         *
//C*       PCX,PCY  PC OF CURVE                                  R*8    *
//C*       RCX,RCY  RC OF CURVE                                  R*8    *
//C*       CL1      LENGTH OF CURVE                              R*8    *
//C*       BPX,BPY  BP OF LINE                                   R*8    *
//C*       EPX,EPY  EP OF LINE                                   R*8    *
//C*       X1,Y1    FIRST INTERSECTION POINT ON CURVE            R*8    *
//C*                OR SELECTED END PT OF CURVE                         *
//C*       X2,Y2    SECOND INTERSECTION POINT                    R*8    *
//C*                OR SELECTED END PT OF LINE                          *
//C*       X3,Y3    INTERSECTION POINT SELECTED                  R*8    *
//C*       NC,NL    END POINTS SELECTION CODES                   I*2    *
//C*       DMN      DISTANCE WITHIN THE JOINT                    R*8    *
//C*       ICD      JOINT INTERSECTION CODES                     I*2    *
//C*                = 0   NO INTERSECTION                               *
//C*                = 1   1 TANGENT PT,ON LINE & CURVE                  *
//C*                =-1   1 TANGENT PT,ON CURVE ONLY                    *
//C*                = 2   2 INTERSECTION PTS,ON LINE & CURVE            *
//C*                =-2   2 INTERSECTION PTS,ON CURVE ONLY              *
//C*                = 3   1ST PT ON BOTH,2ND PT ON CURVE ONLY           *
//C*                =-3   2ND PT ON BOTH,1ST PT ON CURVE ONLY           *
//C*                = 4   1ST PT ON BOTH,2ND PT ON EXT. OF BOTH         *
//C*                =-4   1ST PT ON CURVE ONLY                          *
//C*                = 5   IN CONTINUITY                                 *
//C*                = 6   BOTH PTS ON EXT. OF BOTH                      *
//C*                = 7   BOTH PTS ON EXT. OF CURVE,ONE OF THEM ON LN   *
//C*                = 8   BOTH PTS ON LINE,BOTH PTS ON EXT OF CURVE     *
//C*                = -99 on input causes the routine to attempt        *
//C*                      congruency checking, else it won't            *
//C*                                                                    *
//C*                                                                    *
//C*       VARIABLE DESCRIPTION                                         *
//C*       -------- -----------                                         *
//C*       PTX,PTY  INTERSETING POINT                            R*8    *
//C*       PDMN     PERPENDICULAR DISTANCE                       R*8    *
//C*       AZPT     AZIMUTH OF RC TO POINT                       R*8    *
//C*       AZP1     SAME AS AZPT                                        *
//C*       AZP2     SAME AS AZP1                                        *
//C*       AZDFCT   AZIMUTH DIFFERENCE BETWEEN PC AND PT         R*8    *
//C*       AZDFC1   AZIMUTH DIFFERENCE BETWEEN PC AND PT ONE     R*8    *
//C*       AZDFC2   AZIMUTH DIFFERENCE BETWEEN PC AND PT TWO     R*8    *
//C*       DTMN1    MINIMUM DISTANCE                             R*8    *
//C*                                                                    *
//C*       AUTHOR                                                       *
//C*       ------                                                       *
//C*       STEVE WU,DEPT OF PUBLIC WORK, CITY OF MINNEAPOLIS            *
//C*                NOVEMBER 10, 1983                                   *
//C*                                                                    *
//C*       MODIFIED                                                     *
//C*       ------                                                       *
//C*       10/31/83 REDESIGN ICD CODE FOR 6,7,8                         *
//C*       08/16/84 LEVEL 17,18                                         *
//C*                WHEN GAPS EXIST PUT XC,YC AT X1,Y1 & XL,YL AT X2,Y2 *
//C*                AFFECTING ICD= -4,-2,-1 WHERE NC=NL=3               *
//C*       22 Aug 88 Found this routine to only retrn continuity       *
//C*                 when continuity and another intersection existed.  *
//C*                 Fixed to retrn any other types of intersections   *
//C*                 after finding continuity.  LDA                     *
//C*                                                                    *
//C**********************************************************************
//C
      
      DPOINT dp,odp;
      double  DMN2, X, Y, AZLN, AZPC, AZLNP, RAD, PTX, PTY, cgtx, cgty,
      XL, XC, YL, YC, X0, Y0, PDMN, AZTOL,AZPT, SECT=0,AZP1,AZP2,AZDFCT,
      AZDFC1, AZDFC2,AZDFT, XT, YT,DTMN1;
      short I2CODE, IRC, K, IRC1, IRC2, NW;
      BOOL cgt;
      short M1 = -1, N3 = 3;
//cd     print*,' '
      AZLN = LGETAZ(*BPX,*BPY,*EPX,*EPY); //azm of line
//cd     print*,'line azimuth = ',azln
      AZPC = LGETAZ(*RCX,*RCY,*PCX,*PCY); //azm from rad to pc
//cd     print*,'azimuth to PC = ',azpc
      AZLNP = LTWOPI(AZLN+HALFPI); //right angle azm to line
//cd     print*,'right angle to line azimuth = ',azlnp
      RAD = LDIST(*PCX,*PCY,*RCX,*RCY); //radius length
      AZTOL = P_TOL/RAD; //azm tolerance
//cd     print*,'curve radius = ',rad,' with an aztol of ',aztol
      *NC  = 0;
      *NL  = 0;
      *X3  = 0e0;
      *Y3  = 0e0;
      *DMN = 0e0;
      cgt = FALSE;
      
//C******* GET THE P.T. OF CURVE 
       AZPT = fabs(*CL);
       LOL8(PCX,PCY,RCX,RCY,CL,&AZPT,&PTX,&PTY,N3);
//cd     print*,'*** XLC P.T. IS',PTX,PTY
       AZPT = LGETAZ(*RCX,*RCY,PTX,PTY);
//cd     print*,'azimuth to PT = ',azpt
      if(*ICD ==-99)
      {   //check for continuity
          CNGRNT(PCX,PCY,&PTX,&PTY,BPX,BPY,EPX,EPY,NC,NL,&IRC);
        switch (IRC)
        {
          case 0:
             if(*NC == 1)
             { 
                cgtx = *PCX;
                cgty = *PCY;
             } 
             else 
             {
                cgtx = PTX;
                cgty = PTY;
             }
             cgt = TRUE;
             goto S99;
             break;
          case 2:
             *ICD = 2;
             *X1 = *BPX;
             *Y1 = *BPY;
             *X2 = *EPX;
             *Y2 = *EPY;
             *X3 = *X1;
             *Y3 = *Y1;
             *NC = 2;
             *NL = 2;
             *DMN = 0;
{
#if ENABLETRACE
GSSiExitProg (1201);
#endif
             return;
}
        }  //end of the switch
      }  //end of if(*ICD ==-99)
//cd     print*,'returncode of continuity check = ',irc
//C  


//C******* COMPUTE POSSIBLE INTERSECTIONS
//C
      *ICD = 0;
      IRC = 0;
      *NC = 0;
      *NL = 0;
//c      X3 = 0D0
//c      Y3 = 0D0
      I2CODE = -1;
      *DMN = DMNMX_TOL(PCX,PCY,&PTX,&PTY,BPX,BPY,EPX,EPY,NC,NL,
                      P_TOL,&I2CODE);
      if(*NC == 1)
      {
         XC = *PCX     ; //xc,yc is min distance point on curve
         YC = *PCY;
      }
      else
      {
         XC = PTX;
         YC = PTY;
      }
      if(*NL == 1)
      {
         XL = *BPX     ; //xl,yl is min distance point on line
         YL = *BPY;
      }
      else
      {
         XL = *EPX;
         YL = *EPY;
      }
      //c*    Find a point on the line at right angles to the radius
      SECLIN8 (RCX,RCY,&AZLNP,BPX,BPY,&AZLN,&X0,&Y0,&K);
//c*    find distance between rad and line at right angle
      PDMN = LDIST(*RCX,*RCY,X0,Y0);
//cd     print*,'Minimum perpendicular dist from RAD to LINE = ',pdmn
      if(PDMN > RAD + P_TOL)
      { //rad doesn't come near the line
//C******* NO INTERSECTION POSSIBLE, PROXIMITY RULE APPLIES
//C        (X1,Y1) POINT TO CHOSEN END POINT OF CURVE
//C        (X2,Y2) POINT TO CHOSEN END POINT OF LINE
//C        (X3,Y3) IS 0, 0
         *ICD = 0;
         *X1 = XC  ; //closest endpoint of curve
         *Y1 = YC;
         *X2 = XL  ; //closest endpoint of line
         *Y2 = YL;
{
#if ENABLETRACE
GSSiExitProg (1201);
#endif
         return;
}
      }  //
//c**   next I check for tangential intersections
       if(PDMN <= RAD + P_TOL || PDMN >= RAD - P_TOL)
       { //
//c*        at least one and possible two intersection points
//c*        sect = the distance on the line from the right angle
//c*               intersection point to the line curve
//c*               intersection points
          if(RAD <= PDMN)
             SECT = 0;
          else 
             SECT = sqrt(fabs(RAD * RAD) - (PDMN * PDMN));
          if(SECT <= P_TOL)
          { //consider this a single hit
            if(cgt)
{
#if ENABLETRACE
GSSiExitProg (1201);
#endif
            	return;
}
            *X1 = X0;
            *Y1 = Y0;
            *X2 = X0;
            *Y2 = Y0;
            *X3 = X0;
            *Y3 = Y0;
            goto S300 ;
          }   
       }   
       
       
       
//C******* INTERSECTION POSSIBLE
//C
//c*    Here it generates the coordinate of two points,
//c*    one on each side of the perpendicular point,
//c*    a distance of sect from the perpendicular point.
       odp.x = X0;
       odp.y = Y0;
       dp = dnewpt(odp,AZLN,SECT);
       *X1 = dp.x;
       *Y1 = dp.y;
       dp = dnewpt(odp,AZLN,-SECT);
       *X2 = dp.x;
       *Y2 = dp.y;
//cd     print*,'distance from perpendicular intx to line curve intx = ',
//cd    +        sect

//C******* ORDER THE XPT'S IN THE DIRECTION OF CURVE
S300:  AZP1 = LGETAZ(*RCX,*RCY,*X1,*Y1) ; //azm from rad to ip1
//cd     print*,'Azimuth to first possible intx point = ',azp1
       AZP2 = LGETAZ(*RCX,*RCY,*X2,*Y2) ; //azm from rad to ip2
//cd     print*,'Azimuth to second possible intx point = ',azp2
       AZDFCT = AZDF(AZPC,AZPT,*CL)  ; //length of curve in radians
      if(*CL > RAD && AZDFCT <= AZTOL)AZDFCT = TWOPI ; //a full circle
//cd     print*,'Curve length in radians = ',azdfct
      AZDFC1 = AZDF(AZPC,AZP1,*CL)  ; //dist from pc to ip1 in radians
//cd     print*,'Azm diff from PC to first intx point = ',azdfc1

      if(fabs(AZP1-AZPC) < AZTOL)AZDFC1 = 0e0;
      AZDFC2 = AZDF(AZPC,AZP2,*CL); //dist from pc to ip2 in radians
      if(fabs(AZP2-AZPC) < AZTOL)AZDFC2 = 0e0;
      if(AZDFC1 > AZDFC2)
      { //swap point so ip1 is closer to pc
         AZDFT = AZDFC2;
         AZDFC2 = AZDFC1;
         AZDFC1 = AZDFT;
         XT  = *X2;
         YT  = *Y2;
         *X2 = *X1;
         *Y2 = *Y1;
         *X1 = XT;
         *Y1 = YT;
      }

      if(fabs(AZDFC1-AZDFC2) <= AZTOL)
      { 
//c*       ip1 and ip2 so close together to be considered one point
//C******* ONE TANGENT POINT found on the curve or its' extension
         if(AZDFC1 - AZTOL > AZDFCT) goto S800 ; //beyond PT of curve
//C********* FALLS ON CURVE, now check to if on line
           *ICD = -1  ; //the one point is actually on the curve
           IRC = INLNCK(BPX,BPY,EPX,EPY,X1,Y1);
           if(IRC == 0) *ICD = 1; //on line also
           goto S900;
      }

//C******* two intersections possible, must determine if actual or
//c*       on the extensions of one or both lines

//c*    if ip1 is beyond the pt and beyond it by more than aztol
//c*    both points considered to be off the actual curve.
      if(AZDFC1 - AZTOL > AZDFCT) goto S800;
//c*      IF(AZDFC1.GT.AZDFCT && dabs (azdfc1-azdfct) > aztol)GOTO 800

//c*    if ip2 less aztol is on the curve then we have two
//c*    actual intersection points on the curve
      if(AZDFC2 - AZTOL <= AZDFCT) goto S600;
//c*      IF(AZDFC2.LT.AZDFCT ||  dabs (azdfc2-azdfct) <aztol)GOTO 600

//c*    Get here if ip1 on the actual curve and ip2 is off the actual curve
//C********* (X1,Y1) IN CURVE ONLY,ICD=-4
//C********* (X1,Y1) IN CURVE & LINE ONLY,ICD=4
           *ICD = -4 ; //ip1 is actually on the curve only
           IRC = INLNCK(BPX,BPY,EPX,EPY,X1,Y1);
           if(IRC ==0)*ICD = 4 ; //on actual line also
           goto S900;

//C******* BOTH INTERSECTION POINTS ON CURVE
S600:  IRC1 = INLNCK(BPX,BPY,EPX,EPY,X1,Y1);
       IRC2 = INLNCK(BPX,BPY,EPX,EPY,X2,Y2);
      if(IRC1 == 0)
      { //first point on line
         *ICD = 3 ; //ip1 on line too
         if(IRC2 ==0) *ICD = 2 ; //second point on line too
      } 
      else 
      {
         *ICD = -2; //no points on actual line
         if(IRC2 == 0)*ICD = -3; //only second point on the line
      }
      goto S900 ;
      
      
//C
//C******* BOTH INTERSECTION POINTS ON THE EXTENSION OF CURVE
//C
S800: IRC1 = INLNCK(BPX,BPY,EPX,EPY,X1,Y1);
      IRC2 = INLNCK(BPX,BPY,EPX,EPY,X2,Y2);
      if(IRC1 !=0 && IRC2 !=0)
      { //no actual line hits
//C******* (X1,Y1) & (X2,Y2) ARE ON EXTENSION OF LIN & CURVE
//C        PROXIMITY RULE APPLIED
S99:     if(cgt)
         { 
             *DMN = 0;
             *X1 = cgtx;
             *Y1 = cgty;
             *X2 = *X1;
             *Y2 = *Y1;
             *X3 = *X1;
             *Y3 = *Y1;
             *ICD = 5;
{
#if ENABLETRACE
GSSiExitProg (1201);
#endif
             return;
}
         }  
         *ICD = 6;
         *X1 = XC  ; //closest curve endpoint
         *Y1 = YC;
         *X2 = XL  ; //closest line endpoint
         *Y2 = YL;
         *X3 = 0e0;
         *Y3 = 0e0;
{
#if ENABLETRACE
GSSiExitProg (1201);
#endif
         return;
}
      } 
      if(IRC1 == 0 && IRC2 == 0)
      { //both on actual line
//C******* (X1,Y1) & (X2,Y2) ARE ON LINE ONLY
//C        ((X1,Y1),(X2,Y2),(XL,YL)) WHICH ONE IS CLOSEST TO (XC,YC)
         if(cgt) goto S99;
         *ICD = 8;
         *DMN = DSTMIN(X1,Y1,X2,Y2,&XL,&YL,&XC,&YC,&NW);
         if(NW == 1)
         { //ip1 closest to curve
            *NC = 2;
            *NL = 2;
            *X3 = *X1;
            *Y3 = *Y1;
            *DMN = 0e0;
         } 
         else 
         { 
           if(NW == 2)
           { //ip2 closest to curve
              *NC = 2;
              *NL = 2;
              *X3 = *X2;
              *Y3 = *Y2;
              *DMN = 0e0;
           } 
           else 
           {   //neither intersection point
//c*             closer than actual line endpoint
            *X3 = 0e0;
            *Y3 = 0e0;
           }
           *X1=XC;
           *Y1=YC;
           *X2=XL;
           *Y2=YL;
{
#if ENABLETRACE
GSSiExitProg (1201);
#endif
           return;
}
      }
      //only one point actually on the line
         if(cgt)goto S99;
         *ICD=7;
         *DMN = 0e0;
//C                if the distance between either endpoint of the curve and the
//C                intersection point is less than the current DMN value reset
//C                the DMN and XC, YC values (JS 6/28/89)
         if (IRC1 ==0)
         { 
             X = *X1;
             Y = *Y1;
         } 
         else 
         {
             X = *X2;
             Y = *Y2;
         }  
         DMN2 = DSPTLN(PCX,PCY,&PTX,&PTY,&X,&Y,&NW,M1);
         if (DMN2 < *DMN)
         { 
             *DMN = DMN2;
             if (NW == 1)
             { 
                 XC = *PCX;
                 YC = *PCY;
             } 
             else 
             {
                 XC = PTX;
                 YC = PTY;
             }  
             XL = X;
             YL = Y;
         }  
         if(IRC1 == 0)
         { 
            *DMN = DSPTLN(X1,Y1,&XL,&YL,&XC,&YC,&NW,M1);
            if(NW == 1)
            { //ip1 closer to curve
               *X3 = *X1;
               *Y3 = *Y1;
               *NC = 2;
               *NL = 2;
            } 
            else 
            { //actual line endpoint closer
               *X3 = 0e0;
               *Y3 = 0e0;
            }
            *X1=XC;
            *Y1=YC;
            *X2=XL;
            *Y2=YL;
{
#if ENABLETRACE
GSSiExitProg (1201);
#endif
            return;
}
         } 
          if(IRC2 ==0)
          { 
            *DMN = DSPTLN(X2,Y2,&XL,&YL,&XC,&YC,&NW,M1);
            if(NW == 1)
            {  //ip2 closer to curve
               *X3 = *X2;
               *Y3 = *Y2;
               *NC = 2;
               *NL = 2;
            } 
            else 
            {  //actual line endpoint closer
               *X3 = 0e0;
               *Y3 = 0e0;
            }
            *X1=XC;
            *Y1=YC;
            *X2=XL;
            *Y2=YL;
         }
      }
{
#if ENABLETRACE
GSSiExitProg (1201);
#endif
      return;
}
 
 
 
S900:    switch (*ICD)
      {
       case -1:
       case -4:
         if(cgt)goto S99;
//c*       have an actual hit on the curve only
         *X3 = *X1;
         *Y3 = *Y1;
         DTMN1 = DSPTLN(BPX,BPY,EPX,EPY,X3,Y3,NL,M1);
S910:    XL = *BPX;
         YL = *BPY;
         if(*NL != 1)
         { 
            XL = *EPX;
            YL = *EPY;
         }   
//C        *** ALWAYS PLACE SEP OF LINE ON X2,Y2 FOR GAP MARKING PROCESS
         *X2 = XL;
         *Y2 = YL;
         *DMN = DSTMIN(PCX,PCY,&PTX,&PTY,X3,Y3,&XL,&YL,NC) ; //closest to xl,yl
         if(*NC == 3)
         { 
            *NL = 2;
//*APOLLO TREAT THIS CASE AS GAP*            DMN=0.0D0;
//C        *** ALWAYS PLACE SEP OF CURVE ON X1,Y1 FOR GAP MARKING PROCESS
            *X1 = *X3;
            *Y1 = *Y3;
         } 
         else 
         {
            if(*NC == 2)
            { 
               *X1 = PTX;
               *Y1 = PTY;
            } 
            else 
            {
               *X1 = *PCX;
               *Y1 = *PCY;
            }   
         }   
         break;
      case -2:
//c*       two point on actual curve, no points on actual line
         if(cgt)goto S99;
         I2CODE = M1;
         DTMN1 = DMNMX(BPX,BPY,EPX,EPY,X1,Y1,X2,Y2,NL,NC,&I2CODE);
         *X3 = *X1;
         *Y3 = *Y1;
         if(*NC !=1 )
         { 
            *X3 = *X2;
            *Y3 = *Y2;
         }  
         goto S910;
      case 1:
      case 3:
      case 4:
         *NC = 2;
         *NL = 2;
         *DMN = 0e0;
//c*       when icd = 1 ip1 on line and tangent point on curve
//c*       when icd = 3 ip1 actual hit on line and curve,
//c*                    ip2 on ext of line
//c*       when icd = 4 ip1 actual hit on line and curve
//c*                    ip2 on ext of curve  
            *X3 = *X1;
            *Y3 = *Y1;
            if(*ICD == 1)
            { 
               *X1 = XC ; //closest curve coords
               *Y1 = YC;
               *X2 = XL ; //closest line coords
               *Y2 = YL; 
               break;
            } 
            if(*ICD == 3 && cgt)  *ICD = 5;
            break;  
      case -3:
//c*          ip2 on both curve and line
            *X3 = *X2;
            *Y3 = *Y2;
            if(cgt)*ICD = 5;
            break;
      case 2:
//c*       ip1 and ip2 both on line and curve
            *X3 = *X1;
            *Y3 = *Y1;
        break;
      }  //end of the switch 
{
#if ENABLETRACE
GSSiExitProg (1201);
#endif
      return ;
}
#if ENABLETRACE
}
#endif
}       
     
void  XCC(const double *c1x,const double *c1y,const double *r1x,const double *r1y,const double *cl1,
		  const double *c2x,const double *c2y,const double *r2x,const double *r2y,const double *cl2,
		  double *x1, double *y1,double *x2, double *y2, double *x3, double *y3,
          short *n1, short *n2, double *dmn, short *icd)
#if ENABLETRACE
{GSSiEnterProg (1202);
#endif
 {
//c*
//c******* specifications ***********************************************
//c*                                                                    *
//c*       program summary                                              *
//c*       ------- -------                                              *
//c*       ldaxcc computes intersecting condition between two curves    *
//c*       This routine is a modification of xcc.ftn, hopefully fixing  *
//c*       the problem of xcc returning continuous lines when two       *
//c*       lines intersect once at their endpoints and again elsewhere. *
//c*                                                                    *
//c*                                                                    *
//c*                                                                    *
//c*       argument description                                         *
//c*       -------- -----------                                         *
//c*       c1x,c1y  point of curve of first curve                r*8    *
//c*       r1x,r1y  radius of first curve                        r*8    *
//c*       cl1      length of first curve with sign              r*8    *
//c*       c2x,c2y  point of curve of second curve               r*8    *
//c*       r2x,r2y  radius of second curve                       r*8    *
//c*       cl2      length of second curve                       r*8    *
//c*       x1,y1    first intersection point                     r*8    *
//c*                or selected end point of curve 1 if using proximity *
//c*       x2,y2    second intersection point                    r*8    *
//c*                or selected end point of curve 2 if using proximity *
//c*       x3,y3    selected intersection point                  r*8    *
//c*       n1,n2    end points selection codes                   i*2    *
//c*       icd      joint intersection returncode               i*2    *
//c*                =0  two curves in continuity                        *
//c*                =1  (x1,y1) or (x2,y2) is on both curves            *
//c*                =2  (x1,y1) and (x2,y2) are on both curves          *
//c*                =3  neither (x1,y1) nor (x2,y2) is on both curves   *
//c*       dmn      distance within the joint, if gap present    r*8    *
//c*                                                                    *
//c*       author                                                       *
//c*       ------                                                       *
//c*       steve wu       dept of public work, city of minneapolis      *
//c*                      november 10,1983                              *
//c*                                                                    *
//c*       modified                                                     *
//c*       --------                                                     *
//c*       10/24/83   complete the logic for all cases                  *
//c*                                                                    *
//c*       Sep 88 Modified to look beyond one continuity intersection   *
//c*              point.  Modified to tighten tolerances when testing   *
//c*              for tangential intersections. LDA                     *
//c*                                                                    *
//c*       Jun 89 Modified to retrn coordinates of intersection        *
//c*              points when the two lines are parallel (the endpoints *
//c*              of the lines are returned rather than the midpoint.   *
//c*                                                                    *
//c*                                                                    *
//c**********************************************************************
//c
      
      double r1, r2,rr,rbig, rsmall,azbs, t1x, t2x, t1y, t2y, cgtx , cgty,
      alph, dazm, azr1r2, az1, az2;
      BOOL cgt;            
      short irc, n1cgt, n2cgt, irc11, irc12, n3, irc21, irc22,
            m1;
      DPOINT dp,odp;           
             
      *x1 = 0;
      *y1 = 0;
      *x2 = 0;
      *y2 = 0;
      *x3 = 10e50 ; //code indicating x3,y3 not set
      *y3 = 0;
      *n1 = 0;
      *n2 = 0;
      n3 = 3;
      cgt = FALSE;
//c*    Here the coords of the first curve's PT are found 
       rr = fabs(*cl1);
       LOL8(c1x,c1y,r1x,r1y,cl1,&rr,&t1x,&t1y,n3);
//c*    Here the coords of the second curve's PT are found 
       rr = fabs(*cl2);
       LOL8(c2x,c2y,r2x,r2y,cl2,&rr,&t2x,&t2y,n3);
//c*   the continuity test is then made
       CNGRNT(c1x,c1y,&t1x,&t1y,c2x,c2y,&t2x,&t2y,n1,n2,&irc);
    switch (irc)
    {
      case 0: // in continuity, store, temporarily the congruent endpoints
        *icd = 0;
        *dmn = 0;
        cgt = TRUE;
        if(*n1 == 1)
        { //use the bp of line 1
            cgtx = *c1x;
            cgty = *c1y;
         } 
         else 
         {  //use the ep of line 1
            cgtx = t1x;
            cgty = t1y;
         } 
         n1cgt = *n1 ; //which end of line 1 ][ 1 = bp, 2 = ep
         n2cgt = *n2 ; //which end of line 2 ][ 1 = bp, 2 = ep 
         break;
     case 2: //both ends in continuity, lines overlap
             // and are basicly identical.
         *icd = 2;
         *x1 = *c1x;
         *y1 = *c1y;
         *x2 = t1x;
         *y2 = t1y;
         *x3 = *x1;
         *y3 = *y1;
         *dmn = 0;
{
#if ENABLETRACE
GSSiExitProg (1202);
#endif
         return;
}
    }  
      
      //now try to find any other intersections
//c*
      rr=LDIST(*r1x,*r1y,*r2x,*r2y) ; //dist between radi
      r1=LDIST(*r1x,*r1y,*c1x,*c1y) ; //first radius length
      r2=LDIST(*r2x,*r2y,*c2x,*c2y) ; //second radius length
      if(r1 > r2)
      {  rbig   = r1; //set rbig to point to the first radius
         rsmall = r2; } 
      else 
      {  rbig   = r2; //set rbig to point to the second radius
         rsmall = r1; }
          
      if(rr-P_TOL <= r1+r2 && rr+P_TOL >= rbig-rsmall)
      // the centers are within reach 
      // if smaller arc within the big arc they are
      // far enough apart that their arcs could intersect
      { //possible intersection exist based on proximity 
         if(rr >= r1+r2 || rbig >= rr+rsmall)
         {          
//c*           the curves are with p_tol of one another
//c*           but do not actually touch.  We want to find
//c*           the azimuth from the big rad to the small rad
//c*           and place a test point big rad distance from
//c*           the big rad radius point.

S10:          if (r1 > r2)
              {  azbs = LGETAZ(*r1x,*r1y,*r2x,*r2y);
                 odp.x =  *r1x;
                 odp.y = *r1y;
                 dp = dnewpt(odp,azbs,r1); 
                 *x1 = dp.x;
                 *y1 = dp.y; } 
             else 
             {   azbs = LGETAZ(*r2x,*r2y,*r1x,*r1y); 
                 odp.x = *r2x;
                 odp.y = *r2y;
                 dp = dnewpt(odp,azbs,r2);
                 *x1 = dp.x;
                 *y1 = dp.y;}
                 
             irc11 = INCRVE(c1x,c1y,r1x,r1y,cl1,x1,y1);
             if (irc11 != 0)
             { //not within the first arc
                 *x1 = *c1x;
                 *y1 = *c1y;
                 irc11 = INCRVE(c2x,c2y,r2x,r2y,cl2,x1,y1);
                 if (irc11 == 0) goto S50; //within this arc
                 *x1 = t1x;
                 *y1 = t1y;
                 irc11 = INCRVE(c2x,c2y,r2x,r2y,cl2,x1,y1);
                 if (irc11 != 0) goto S100;//not in arc two either
             }
             
// gets here if the intx point on first arc             
S50:          irc12 = INCRVE(c2x,c2y,r2x,r2y,cl2,x1,y1);
              if (irc12 !=0)
              { //not in the second arc
                 *x1 = *c2x;
                 *y1 = *c2y;
                 irc12 = INCRVE(c1x,c1y,r1x,r1y,cl1,x1,y1);
                 if (irc12 == 0) goto S55; //this is within the second arc
                 *x1 = t2x;
                 *y1 = t2y;
                 irc12 = INCRVE(c1x,c1y,r1x,r1y,cl1,x1,y1);
                 if (irc12 !=0) goto S100;
              }
// gets here if the intx point on second irc             
S55:          if(irc11 == 0 && irc12 == 0)
              { //on both curves
                if(cgt)
                { //the endpoint
                  *x1 = cgtx;
                  *y1 = cgty;
                  *n1 = n1cgt;
                  *x2 = *x1;
                  *y2 = *y1;
                  *x3 = *x1;
                  *y3 = *y1;
                  *n2 = 3;
                  *dmn = 0;
                  *icd = 1;
                } 
                else 
                {
                  *x2 = *x1;
                  *y2 = *y1;
                  *x3 = *x1;
                  *y3 = *y1;
                  *n1 = 3;
                  *n2 = 3;
                  *dmn = 0;
                  *icd = 1;
                }
{
#if ENABLETRACE
GSSiExitProg (1202);
#endif
                return;
}
             }
             else
                goto S100;   
            
         } 
         else 
         {  //two intersection points possible, though
//c*             they may be close enough together to be
//c*             considered as one intersection point.
//c*           using the law of cosines, the cosine of the
//c*           line from r2 to one of the intersection points
//c*           is calculated
             alph = (r1*r1+rr*rr-r2*r2) / (2e0*r1*rr);
             if(fabs(alph) > 1e0) alph = DSIGN(1e0,alph);
             alph = LTWOPI(acos(alph)) ; //the delta_azm
//c*                  from the line between the radii to
//c*                  the intersection point

//c*           now I check to see if I should treat this
//c*           as one intersection point or two.
             dazm = __min(atan2(P_TOL,r1), atan2(P_TOL,r2));
             if (alph <= dazm)goto S10 ; //treat as single hit

             if(rr <= P_TOL)
             { // ; //lines have the same radius point
//c*              I use the endpoints of the lines to test for
//c*              coincidence.  If coincidence is found (x1,y1) and
//c*              (x2,y2) are set to the appropriate line endpoint.

//c*              I check if the pc of curve two is in curve one
                 irc11 = INCRVE(c1x,c1y,r1x,r1y,cl1,c2x,c2y);

//c*              I check if the pc of curve one is in curve two
                 irc12 = INCRVE(c2x,c2y,r2x,r2y,cl2,c1x,c1y);

//c*              I check if the pt of curve two is in curve one
                 irc21 = INCRVE(c1x,c1y,r1x,r1y,cl1,&t2x,&t2y);

//c*              I check if the pt of curve one is in curve two
                 irc22 = INCRVE(c2x,c2y,r2x,r2y,cl2,&t1x,&t1y);

                  if(irc11 == 0 && irc12 == 0 && irc21 == 0 && irc22 == 0)
                  { // the lines are identical (within p_tol)
                     *x1 = *c1x;
                     *y1 = *c1y;
                     *x2 = t1x;
                     *y2 = t1y;
                     *n1 = 1;
                     *n2 = 2;
                     *icd = 2;
                     *dmn = 0;
{
#if ENABLETRACE
GSSiExitProg (1202);
#endif
                     return;
}
                   } 
                   if(irc11 == 0 || irc12 == 0 ||irc21 == 0 || irc22 == 0)
                   {  //overlap occurs
//c*                 the lines overlap
                     *icd = 2  ; //two hits occurred
                     if(irc11 ==0 && irc21 ==0)
                     { //  line two is in line one
                        *x1 = *c2x;
                        *y1 = *c2y;
                        *x2 = t2x;
                        *y2 = t2y;
                        *n1 = 1;
                        *n2 = 2;
{
#if ENABLETRACE
GSSiExitProg (1202);
#endif
                        return;
}
                     }
                     if(irc12 ==0 && irc22 ==0)
                     { // line one is in line two
                        *x1 = *c1x;
                        *y1 = *c1y;
                        *x2 = t1x;
                        *y2 = t1y;
                        *n1 = 1;
                        *n2 = 2;
{
#if ENABLETRACE
GSSiExitProg (1202);
#endif
                        return;
}
                     } 
                      //gotta find the right endpoint of the overlap
//c*                    the lines overlap for a portion of both
//c*                    their lengths.  Here I determine which
//c*                    endpoints fall within the other line.
                      if(irc11 == 0)
                      { // ; //two's pc is in one
                         *x1 = *c2x;
                         *y1 = *c2y;
                         *n1 = 2;
                      } 
                      else 
                      {//two's PT is in one
                         *x1 = t2x;
                         *y1 = t2y;
                         *n1 = 2;
                      }
                      if(irc12 == 0)
                      { // ; //one's PC is in two
                         *x2 = *c1x;
                         *y2 = *c1y;
                         *n2 = 1;
                      } 
                      else 
                      {   //one's PT is in two
                         *x2 = t1x;
                         *y2 = t1y;
                         *n2 = 1;
                      }
{
#if ENABLETRACE
GSSiExitProg (1202);
#endif
                      return;
}
                   }
             } 
             else // radii not within P_TOL of one another
             {
               azr1r2 = LGETAZ(*r1x,*r1y,*r2x,*r2y);
               az1 = LTWOPI(azr1r2-alph);
               az2 = LTWOPI(azr1r2+alph);
               odp.x = *r1x;
               odp.y = *r1y;
               dp = dnewpt(odp,az1,r1);
               *x1 = dp.x;
               *y1 = dp.y;
//d               write(6,*)'*** xcc x1,y1=',x1,y1
               irc11 = INCRVE(c1x,c1y,r1x,r1y,cl1,x1,y1);
               irc12 = INCRVE(c2x,c2y,r2x,r2y,cl2,x1,y1);
               odp.x = *r1x;
               odp.y = *r1y;
               dp = dnewpt(odp,az2,r1);
               *x2 = dp.x;
               *y2 = dp.y;
//d               write(6,*)'*** xcc x2,y2=',x2,y2
               irc21 = INCRVE(c1x,c1y,r1x,r1y,cl1,x2,y2);
               irc22 = INCRVE(c2x,c2y,r2x,r2y,cl2,x2,y2);
//d               print 11,irc11,irc12,irc21,irc22
//d11             format(' *** xcc irc11,irc12,irc21,irc22=',4i4)
               if(irc11 ==0 && irc12 ==0)
               { 
                   *n1 = 3;
                   *n2 = 3;
                   *x3 = *x1;
                   *y3 = *y1; 
                   *n1 = 3;
                   dmn = 0;
                   if(irc21 ==0 && irc22 ==0)
                     *icd = 2;//  (x1,y1) and (x2,y2) are both on both curves
                   else 
                     *icd = 1;//  only (x1,y1) on both curves
{
#if ENABLETRACE
GSSiExitProg (1202);
#endif
                   return;
}
               } 
               if(irc21 ==0 && irc22 ==0)
               {// only (x2,y2) on both curves
                  *x3 = *x2;
                  *y3 = *y2;
                  *n1 = 3;
                  *n2 = 3;
                  *dmn = 0;
                  *icd = 1;
{
#if ENABLETRACE
GSSiExitProg (1202);
#endif
                  return;
}
               } 
               if(cgt)goto S99;
//c*             NO HITS, neither (x1,y1) nor (x2,y2) on both curves
//c*             treated as non-contacting
                goto S100;
         } 
{
#if ENABLETRACE
GSSiExitProg (1202);
#endif
         return;
}
      }  
     
     
S99:   if(cgt)
       {  //already found continuity, use it.
           *x1 = cgtx;
           *y1 = cgty;
           *x2 = cgtx;
           *y2 = cgty;
           *n1 = n1cgt;
           *n2 = n2cgt;
           *dmn = 0;
           *icd = 1;
{
#if ENABLETRACE
GSSiExitProg (1202);
#endif
           return;
}
       }
     }
   
//    get here if rad locations and lengths of radii make it impossible
//    for the two arcs to intersect.     
 //c*         NOT HITS, no actual hits
S100:      *icd = 3;
           m1 = -1;
           *dmn = DMNMX_TOL(c1x,c1y,&t1x,&t1y,c2x,c2y,&t2x,&t2y,n1,n2,
                           P_TOL, &m1);
           *x3 = 10e50;
           if(*n1 == 1)
           {   *x1 = *c1x;
               *y1 = *c1y;} 
           else 
           {   *x1 = t1x;
               *y1 = t1y;}
           if(*n2 == 1)
           {   *x2 = *c2x;
               *y2 = *c2y;} 
           else 
           {   *n1 = 2;
               *x2 = t2x;
               *y2 = t2y;}
      if (*icd == 2)
      { 
          if (LDIST (*x1,*y1,*x2,*y2) <=P_TOL)
          { 
              if (cgt)
              {  
                  *icd = 0;
                  *x1 = (*x1 + *x2) /2e0;
                  *y1 = (*y1 + *y2) /2e0;
              }
          }
      }
      if(*icd == 0)
      {    *x3 = *x1;
           *y3 = *y1; } 
      else 
      { if (*x3 ==10e50)
        {  *x3 = (*x1 + *x2) /2;
           *y3 = (*y1 + *y2) /2; }
      }
{
#if ENABLETRACE
GSSiExitProg (1202);
#endif
 return;
}
#if ENABLETRACE
}
#endif
}             
    
short  PRCURV(double *PCX,double *PCY,double *POCX,double *POCY,
             double *PTX,double *PTY,double *RPX,double *RPY,double *CLEN,
             const short HOW)
#if ENABLETRACE
{GSSiEnterProg (1203);
#endif
{
//C*    (void)RCURVE(PCX,PCY,POCX,POCY,PTX,PTY,RPX,RPY,CLEN)
//C*    (void)PCURVE(PCX,PCY,POCX,POCY,PTX,PTY,RPX,RPY,CLEN)
//C******* SPECIFICATIONS ***********************************************
//C*                                                                    *
//C*       PROGRAM SUMMARY                                              *
//C*       ------- -------                                              *
//C*       P3CURV CHANGES A 3-POINTS CURVE INTO A STANDARD CURVE        *
//C*       OR VICE VERSA.                                               *
//C*       (void)POINT RCURVE CONVERTS 3_POINT CURVE INTO REGULAR ONE   *
//C*       (void)POINT PCURVE CONVERTS REGULAR CURVE INTO 3_POINT ONE   *
//C*                                                                    *
//C*       ARGUMENT DESCRIPTION                                         *
//C*       -------- -----------                                         *
//C*       PCX,PCY  PC CORDINATES                             R*8       *
//C*       POCX,POCY POC CORDINATES                           R*8       *
//C*       PTX,PTY  PT CORDINATES                             R*8       *
//C*       RPX,RPY  RADIUS POINT OF ARC (OLD PT)              R*8       *
//C*       CLEN     INPUT AND OUTPUT ARC LENGTH WITH SIGN     R*8       *
//C*       HOW      HOW TO CONVERT                            I*2       *
//C*                >=0 3P_CURVE INTO REGULAR CURVE                     *
//C*                < 0 REGULAR CURVE INTO 3P_CURVE                     *
//C*                                                                    *
//C*       AUTHOR                                                       *
//C*       ------                                                       *
//C*       STEVE WU       DEPT OF PUBLIC WORK, CITY OF MINNEAPOLIS      *
//C*                      OCTOBER 4, 1985                               *
//C*                                                                    *
//C**********************************************************************
//C
//C
   if(HOW < 0)
{
#if ENABLETRACE
GSSiExitProg (1203);
#endif
           return (PCURVE(PCX,PCY,POCX,POCY,PTX,PTY,RPX,RPY,CLEN));
}
       else
{
#if ENABLETRACE
GSSiExitProg (1203);
#endif
           return (RCURVE(PCX,PCY,POCX,POCY,PTX,PTY,RPX,RPY,CLEN));
}
#if ENABLETRACE
}
#endif
}     
//C**** CONVERTS TO REGULAR CURVE
//C
 short RCURVE(double *PCX,double *PCY,double *POCX,double *POCY,
             double *PTX,double *PTY,double *RPX,double *RPY,double *CLEN)
#if ENABLETRACE
{GSSiEnterProg (1204);
#endif
      { short IRC;
//C     ***  FIND THE RADIUS POINT
      if (LDIST(*PCX,*PCY,*POCX,*POCY) < P_TOL ||
      	  LDIST(*POCX,*POCY,*PTX,*PTY) < P_TOL)
{
#if ENABLETRACE
GSSiExitProg (1204);
#endif
      	  return 1;
}
      if (LDIST(*PCX,*PCY,*PTX,*PTY) < P_TOL) 
      {
      	*RPX = (*PCX + *POCX) / 2;
      	*RPY = (*PCY + *POCY) / 2;
      	RAD=LDIST(*RPX,*RPY,*PCX,*PCY);
      	*CLEN = TWOPI*RAD;
{
#if ENABLETRACE
GSSiExitProg (1204);
#endif
	    return 0; 
}
      }
      AZ12=LGETAZ(*PCX,*PCY,*POCX,*POCY);
      AZ23=LGETAZ(*POCX,*POCY,*PTX,*PTY);
      AZP12=LTWOPI(AZ12+HALFPI);
      AZP23=LTWOPI(AZ23+HALFPI);
      M12X=(*PCX+*POCX)/2e0;
      M12Y=(*PCY+*POCY)/2e0;
      M23X=(*PTX+*POCX)/2e0;
      M23Y=(*PTY+*POCY)/2e0;
      IRC = LIN_SEC(M12X,M12Y,AZP12,M23X,M23Y,AZP23,RPX, RPY);
      if(IRC != 0)
      { 
         //PRINT *,'**RCURVE** 3 POINTS COLINEAR';
{
#if ENABLETRACE
GSSiExitProg (1204);
#endif
         return IRC;
}
      }
//C     *** FIND CURVE LENGTH AND DIRECTION
      RAD=LDIST(*RPX,*RPY,*PCX,*PCY);
      AZC=LGETAZ(*RPX,*RPY,*PCX,*PCY);
      AZM=LGETAZ(*RPX,*RPY,*POCX,*POCY);
      AZT=LGETAZ(*RPX,*RPY,*PTX,*PTY);
      AZDCM=LTWOPI(AZC-AZM);
      AZDCT=LTWOPI(AZC-AZT);
      if(AZDCT>AZDCM)
         {*CLEN=AZDCT*RAD;} 
      else 
         {if(AZDCT<AZDCM) *CLEN=(AZDCT-TWOPI)*RAD;}
//C     PRINT *,'**RCURVE** PCX ,PCY =',PCX ,PCY
//C     PRINT *,'           POCX,POCY=',POCX,POCY
//C     PRINT *,'           PTX ,PTY =',PTX ,PTY
//C     PRINT *,'           RPX ,RPY =',RPX ,RPY
//C     PRINT *,'           CLEN     =',CLEN
{
#if ENABLETRACE
GSSiExitProg (1204);
#endif
      return 0; 
}
#if ENABLETRACE
}
#endif
 }
//C
//C***** CONVERTS TO 3-POINTS CURVE
//C
 short PCURVE(double *PCX,double *PCY,double *POCX,double *POCY,
             double *PTX,double *PTY,double *RPX,double *RPY,double *CLEN)
#if ENABLETRACE
{GSSiEnterProg (1205);
#endif
 {
       CL=*CLEN/2.0;
       M12X = fabs(CL);
       LOL8(PCX,PCY,RPX,RPY,&CL,&M12X,POCX,POCY,3);
       M12X = fabs(*CLEN);
       LOL8(PCX,PCY,RPX,RPY,CLEN,&M12X,PTX,PTY,3);
//C     PRINT *,'**PCURVE** PCX ,PCY =',PCX ,PCY
//C     PRINT *,'           POCX,POCY=',POCX,POCY
//C     PRINT *,'           PTX ,PTY =',PTX ,PTY
//C     PRINT *,'           RPX ,RPY =',RPX ,RPY
//C     PRINT *,'           CLEN     =',CLEN
{
#if ENABLETRACE
GSSiExitProg (1205);
#endif
      return 0;
}
#if ENABLETRACE
}
#endif
 }
BOOL CurveMNMX (DPOINT PC,DPOINT POC,DPOINT PT,LPMNMXCORD pMinMax)
#if ENABLETRACE
{GSSiEnterProg (1206);
#endif
{   
	DPOINT	RP;
	double	CLEN;
	
	if (RCURVE(&PC.x,&PC.y,&POC.x,&POC.y,&PT.x,&PT.y,&RP.x,&RP.y,&CLEN))
{   
	DBoundsInit (pMinMax);
	AddDPointToMinMax (&PC,pMinMax);
	AddDPointToMinMax (&POC,pMinMax);
	AddDPointToMinMax (&PT,pMinMax);
#if ENABLETRACE
GSSiExitProg (1206);
#endif
		return TRUE;
}
	CRVMNMX (PC.x,PC.y,RP.x,RP.y,CLEN,P_TOL,&pMinMax->xmn,&pMinMax->xmx,&pMinMax->ymn,&pMinMax->ymx);
{
#if ENABLETRACE
GSSiExitProg (1206);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

void CRVMNMX (double X1,double Y1,double XR,double YR,double LNG,double TOL,
              LPDOUBLE XMIN,LPDOUBLE XMAX,LPDOUBLE YMIN,LPDOUBLE YMAX)
#if ENABLETRACE
{GSSiEnterProg (1207);
#endif
{
      double X2,Y2,RAD,AZ,AZLNG, AZTOT, AZINC, X,Y, AZ2, AZMOD;

      RAD  = LDIST (X1,Y1,XR,YR);
      if (RAD < TOL)
      {
          *XMIN = X1;
          *XMAX = X1;
          *YMIN = Y1;
          *YMAX = Y1;
{
#if ENABLETRACE
GSSiExitProg (1207);
#endif
          return;
}
      }
      AZ   = LGETAZ (XR,YR,X1,Y1);
      AZINC = LNG / RAD;
      AZLNG = fabs (AZINC);
      AZMOD = fmod (AZ,HALFPI);
      AZ2   = AZ - AZINC;
      LNEWPT (XR,YR,&X2,&Y2,AZ2,RAD);
      *XMIN = min (X1,X2);
      *XMAX = max (X1,X2);
      *YMIN = min (Y1,Y2);
      *YMAX = max (Y1,Y2);
      if (LNG > 0)
      {
          AZTOT =  AZMOD;
          AZ = AZ - AZTOT;
          AZINC = -HALFPI;
      }
      else
      {
          AZTOT = HALFPI - AZMOD;
          AZ = AZ + AZTOT;
          AZINC = HALFPI;
      }

      while (AZTOT < AZLNG)
      {
          LNEWPT (XR,YR, &X, &Y, AZ, RAD);
          *XMIN = min (*XMIN,X);
          *XMAX = max (*XMAX,X);
          *YMIN = min (*YMIN,Y);
          *YMAX = max (*YMAX,Y);
          AZTOT = AZTOT + HALFPI;
          AZ = AZ + AZINC;
      }
{
#if ENABLETRACE
GSSiExitProg (1207);
#endif
      return;
}
#if ENABLETRACE
}
#endif
}
                               
void XLL(const double *BX1, const double *BY1, const double *EX1, const double *EY1,
         const double *BX2, const double *BY2, const double *EX2, const double *EY2,
         double *X1, double *Y1, double *X2, double *Y2, double *X3, double *Y3, 
         short *N1, short *N2, double *DMN, short *ICD)
#if ENABLETRACE
{GSSiEnterProg (1208);
#endif
{         
/*C**/
/*C******* SPECifICATIONS ************************************************/
/*C*                                                                    **/
/*C*       PROGRAM SUMMARY                                              **/
/*C*       ------- -------                                              **/
/*C*                                                                    **/
/*C*       ROUTINE XLL COMPUTES INTERSECTION POINTS OR MINIMUM          **/
/*C*       DISTANT POINTS BETWEEN TWO CONSECCUTIVE LINES                **/
/*C*       (REAL*8 VERSION)                                             **/
/*C*                                                                    **/
/*C*       XLL COMPUTES INTERSECTION PT BY TWO LINES, WHERE;            **/
/*C*         ICD=-1     No Intersection possible                        **/
/*C*         ICD=0     2 LINES PARALLEL                                 **/
/*C*         ICD=1     XPT INSIDE OF LINE 1                             **/
/*C*         ICD=2     XPT INSIDE OF LINE 2 ONLY                        **/
/*C*         ICD=3     XPT INSIDE OF BOTH LINES                         **/
/*C*                   N1=N2=3                                          **/
/*C*         ICD=4     XPT OUTSIDE OF BOTH LINE,USE TWO CLOSEST EP      **/
/*C*         ICD=7     XPT OUTSIDE OF BOTH LINE,USE XPT POINT           **/
/*C*                   N1=N2=3                                          **/
/*C*         ICD=5     CONGRUENT AT PT(BX1,BY1)                         **/
/*C*         ICD=6     CONGRUENT AT PT(EX1,EX2)                         **/
/*C*         ICD=8     LINES ARE IDENTICAL (OVERLAP EACH OTHER FOR      **/
/*C*                     AT LEAST A PORTION OF EACH LINE.)              **/
/*C*         N1=1 IS (BX1,BY1); =2 IS (EX1,EY1); =3 IS (X3,Y3)          **/
/*C*         N2=1 IS (BX2,BY2); =2 IS (EX2,EY2); =3 IS (X3,Y3)          **/
/*C*                                                                    **/
/*C*                                                                    **/
/*C*       ARGUMENT DESCRIPTION                                         **/
/*C*       -------- -----------                                         **/
/*C* input      BX1,BY1  BP OF FIRST LINE                        R*8    **/
/*C* input      EX1,EY1  EP OF FIRST LINE                        R*8    **/
/*C* input      BX2,BY2  BP OF SECOND LINE                       R*8    **/
/*C* input      EX2,EY2  EP OF SECOND LINE                       R*8    **/
/*C* output     X1,Y1    FIRST LINE'S SELECTED END POINT         R*8    **/
/*C* output     X2,Y2    2ND LINE'S SELECTED END POINT           R*8    **/
/*C* output     X3,Y3    INTERSECTION POINT                      R*8    **/
/*C* output     N1       1ST LINE'S END POINT CODE               I*2    **/
/*C* output     N2       2ND LINE'S END POINT CODE               I*2    **/
/*C* output     ICD      JOINT INTERSECTION CODES                I*2    **/
/*C* output     DMN      DISTANCE WITHIN THE JOINT               R*8    **/
/*C*                                                                    **/
/*C*       VARIABLE DESCRIPTION                                         **/
/*C*       -------- -----------                                         **/
/*C*       AZLN1    FIRST LINE'S AZIMUTH                         R*8    **/
/*C*       AZLN2    SECOND LINE'S AZIMUTH                        R*8    **/
/*C*       DTMN1    MINIMUM DISTANCE                             R*8    **/
/*C*                                                                    **/
/*C***********************************************************************/
/*C*/
#define  M1 -1
      BOOL CONGRT;
      double  DTMN1, MNX1, MXX1, MNX2, MXX2, MNY1, MNY2,
             MXY1, MXY2, OVMNX, OVMXX, OVMNY, OVMXY, AZLN1, AZLN2,
             AZNORT, DISN1P, DISN2P, DISN12, DISS, TEMP;
      long    IGOT,  K;
      short I2CODE, IRC1, IRC2,IRC, N;
      *ICD = -1;
      *N1=0;
      *N2=0;
	  MNX1 = min(*BX1,*EX1);
	  MXX1 = max(*BX1,*EX1);
	  MNX2 = min(*BX2,*EX2);
	  MXX2 = max(*BX2,*EX2);
	  MNY1 = min(*BY1,*EY1);
	  MXY1 = max(*BY1,*EY1);
	  MNY2 = min(*BY2,*EY2);
	  MXY2 = max(*BY2,*EY2);
     if(MNX1-P_TOL > MXX2 || 
        MXX1+P_TOL < MNX2 ||
        MNY1-P_TOL > MXY2 || 
        MXY1+P_TOL < MNY2)
{
#if ENABLETRACE
GSSiExitProg (1208);
#endif
        return;
}
       *ICD=0;
      AZLN1=LGETAZ(*BX1,*BY1,*EX1,*EY1);
      AZLN2=LGETAZ(*BX2,*BY2,*EX2,*EY2);
      CONGRT = FALSE;
      CNGRNT(BX1,BY1,EX1,EY1,BX2,BY2,EX2,EY2,N1,N2,&IRC);
/*C*       IRC = 0 if 1 END CONTINUOUS*/
/*C*             1 if NEITHER AND 2 if BOTH ENDS CONTINUOUS*/ 
       switch(IRC)
       {
       case 2:
          if (IRC == 2) 
          { // COINCIDENT AND IDENTICAL LINES
            if(*N1 == 1)
            { 
              *X1 = *BX1;
              *Y1 = *BY1;
              *X2 = *EX1;
              *Y2 = *EY1;
            } 
            else 
            {
              *X1 = *EX1;
              *Y1 = *EY1;
              *X2 = *BX1;
              *Y2 = *EY1;
            }  
            *ICD = 8; //IDENTICAL LINES (WITHIN P_TOL TOLERANCE;
            *DMN = 0;
            goto s990;
          } 
          case 0:
          {
              switch (*N1)
              {
                case 1:
                 *X3 = *BX1;
                  *Y3 = *BY1;
                  *ICD = 3;
                  *N1 = 3;
                  *N2 = 3;
{
#if ENABLETRACE
GSSiExitProg (1208);
#endif
                  return;
}
                break;
                case 2:
                  *X3 = *EX1;
                  *Y3 = *EY1;
                  *ICD = 3;
                  *N1 = 3;
                  *N2 = 3;
{
#if ENABLETRACE
GSSiExitProg (1208);
#endif
                  return;
}
              } //end of the switch  
                
       
          }
       
       }//end of the switch
       
       // here we see if an endpoint point of line two lies
       // within p_tol of line 1 .or. an endpoint of line 1
       // lies within p_tol of line 2
       K =  LINSEC (*BX1,*BY1,AZLN1,*BX2,*BY2,
                               LTWOPI(AZLN2+HALFPI),X3,Y3);
       TEMP = LDIST(*BX2,*BY2,*X3,*Y3); 
       if(TEMP <= P_TOL)
       {
         *ICD = 3;
         *N1 = 3;
         *N2 = 3;
{
#if ENABLETRACE
GSSiExitProg (1208);
#endif
         return;
}
       } 
       K = LINSEC (*BX1,*BY1,AZLN1,*EX2,*EY2,
                           LTWOPI(AZLN2+HALFPI),X3,Y3);
       TEMP = LDIST(*EX2,*EY2,*X3,*Y3); 
       if(TEMP <= P_TOL)
       {
         *ICD = 3;
         *N1 = 3;
         *N2 = 3;
{
#if ENABLETRACE
GSSiExitProg (1208);
#endif
         return;
}
       }  
       K = LINSEC (*BX2,*BY2,AZLN2,*BX1,*BY1,
                         LTWOPI(AZLN1+HALFPI),X3,Y3);
       TEMP = LDIST(*BX1,*BY1,*X3,*Y3); 
       if(TEMP <= P_TOL)
       {
         *ICD = 3;
         *N1 = 3;
         *N2 = 3;
{
#if ENABLETRACE
GSSiExitProg (1208);
#endif
         return;
}
       } 
       K = LINSEC (*BX2,*BY2,AZLN2,*EX1,*EY1,
                         LTWOPI(AZLN1-HALFPI),X3,Y3);
       TEMP = LDIST(*EX1,*EY1,*X3,*Y3); 
       if(TEMP <= P_TOL)
       {
         *ICD = 3;
         *N1 = 3;
         *N2 = 3;
{
#if ENABLETRACE
GSSiExitProg (1208);
#endif
         return;
}
       }  
  
/*C*/
/*C******* NOT IN CONTINUITY*/
/*C*/
      I2CODE = M1;
      *DMN=DMNMX_TOL(BX1,BY1,EX1,EY1,BX2,BY2,EX2,EY2,N1,N2,
                    P_TOL,&I2CODE);
      if(*N1==1)
      { 
      
         *X1=*BX1;
         *Y1=*BY1;
      } 
      else 
      {
         *X1=*EX1;
         *Y1=*EY1;
      }
      if(*N2==1)
      { 
      
         *X2=*BX2;
         *Y2=*BY2;
      }
      else 
      {
         *X2=*EX2;
         *Y2=*EY2;
      }
      *X3=0;
      *Y3=0;
/*C*    HERE IT CALCULATES THE HYPOTHETICAL INTERSECTION POINT*/
       SECLIN8(BX1,BY1,&AZLN1,BX2,BY2,&AZLN2,X3,Y3,&IRC);
/*C*/
/*C******* TWO LINES PARALLEL*/
/*C*/
      if(IRC==1)goto s990;// LINES ARE PARALLEL BUT NOT COINCIDENT;
      if(IRC==2)
      { //  LINES ARE COINCIDENT... MAYBE
/*C*/
/*C******* TWO LINES IDENTICAL. if THEY OVELAP USE THE MID OVERLAP*/
/*C        POINT AS THE INTERSECTION.*/
/*C*    MODifIED THIS PORTION JUN 89 TO RETRN THE TWO INTERSECTION*/
/*C*    POINTS IN ADDITION TO CALCULATING A MIDPOINT.  LDA*/
        *DMN  = 0;
        AZNORT = fabs(cos(AZLN1));
        if(AZNORT <= 1e-7) 
            { // THIS IS A STRAIGHT NORTH/SOUTH LINE
/*C*            I FIND THE FURTHEST SOUTH POINT AND NEXT AND NEXT*/
              MNY1 = min(MNY1, MNY2) + P_TOL;//LOWEST Y;
              MXY2 = max(MXY1, MXY2) - P_TOL; //HIGHEST Y;
              IGOT = 0;
              if(*BY1 != MNY1 && *BY1 != MXY2)
              { 
                 *X1 = *BX1;
                 *Y1 = *BY1;
                 *N1 = 1;
                 IGOT = 1;
              }  
              if(*BY2 != MNY1 && *BY2 != MXY2)
              { 
                 if(IGOT == 0)
                 { 
                    *X1 = *BX2;
                    *Y1 = *BY2;
                    *N1 = 2;
                 } 
                 else 
                 {
                    *X2 = *BX2;
                    *Y2 = *BY2;
                    *N2 = 2;
                 }                 
                 IGOT++;
              }  
              if(*EY2 != MNY1 && *EY2 != MXY2)
              { 
                 if(IGOT == 0)
                 {
                    *X1 = *EX2;
                    *Y1 = *EY2;
                    *N1 = 2;
                 } 
                 else 
                 {
                    *X2 = *EX2;
                    *Y2 = *EY2;
                    *N2 = 2;
                 }  
                 IGOT++;
              }  
              if(*EY1 != MNY1 && *EY1 != MXY2)
              { 
                 *X2 = *EX1;
                 *Y2 = *EY1;
                 *N2 = 1;
              }  
            } 
            else 
            {
/*C*            THE OVMXX WILL CONTAIN THE DEFINITION OF THAT PORTION*/
/*C*            OF THE LINE THAT OVERLAPS.*/

              OVMNX = max (MNX1,MNX2); // LARGER OF SMALLEST OF ALL X'S
              OVMNY = max (MNY1,MNY2); // LARGER OF SMALLEST OF ALL Y'S;
              OVMXX = min (MXX1,MXX2); // LEAST OF LARGEST OF ALL X'S;
              OVMXY = min (MXY1,MXY2); // LEAST OF LARGEST OF ALL Y'S;
              *X3 = (OVMNX + OVMXX) /2e0;
              *Y3 = (OVMNY + OVMXY) /2e0;
              if(OVMNX == MNX1)
              { 
                OVMNX = OVMNX + P_TOL;
                if(OVMNX == *BX1)
                { 
                   *X1 = *BX1;
                   *Y1 = *BY1;
                   *N1 = 1;
                } 
                else 
                {
                   *X1 = *EX1;
                   *Y1 = *EY1;
                   *N1 = 2;
                }  
              } 
              else 
              {
                OVMNX = OVMNX + P_TOL;
                if(OVMNX == *BX2)
                { 
                   *X1 = *BX2;
                   *Y1 = *BY2;
                   *N1 = 1;
                } 
                else 
                {
                   *X1 = *EX2;
                   *Y1 = *EY2;
                   *N1 = 2;
                } 
              } 
              if(OVMXX == MXX1)
              { 
                OVMXX = OVMXX - P_TOL;
                if(OVMXX == *BX1)
                { 
                   *X2 = *BX1;
                   *Y2 = *BY1;
                   *N2 = 1;
                } 
                else 
                {
                   *X2 = *EX1;
                   *Y2 = *EY1;
                   *N2 = 2;
                } 
              } 
              else 
              {
                OVMXX = OVMXX - P_TOL;
                if(OVMXX == *BX2)
                {
                   *X2 = *BX2;
                   *Y2 = *BY2;
                   *N2 = 1;
                }
                 else 
                 {
                   *X2 = *EX2;
                   *Y2 = *EY2;
                   *N2 = 2;
                }                
              }  
            }  
            *ICD = 8;
            goto s990;
           
      }  
/*C*/
/*C******* TWO LINES NOT PARALLEL*/
/*C*/
      IRC1 = INLNCK(BX1,BY1,EX1,EY1,X3,Y3);
      IRC2 = INLNCK(BX2,BY2,EX2,EY2,X3,Y3);
/*C     WRITE(7,*)'*** XLL FROM INLNCK IRC1=',IRC1,',IRC2=',IRC2*/
      if(IRC1!=0) goto s200;// X3 Y3 NOT ON LINE 1;
      if(IRC2==0)goto s400;//  X3 Y3 ON LINE 2;
/*C*/
/*C******* INTERSECTION ON LINE 1 ONLY*/
/*C*/
         *ICD=1;
         DTMN1 = DSPTLN(BX2, BY2, EX2, EY2, X3, Y3, &N, M1);
         *N2=N;
         *X2=*BX2;
         *Y2=*BY2;
         if(*N2==2)
         { 
            *X2=*EX2;
            *Y2=*EY2;
         }
         *DMN=DSTMIN(BX1,BY1,EX1,EY1,X3,Y3,X2,Y2,&N);
         *N1=N;
         if(*N1==3)
         { 
            *X1=*X3;      //APOLLO TREATS IT AS GAP;
            *Y1=*Y3;
         } 
         else 
         { 
           if(*N1==2)
           {  
            *X1=*EX1;
            *Y1=*EY1;
           } 
           else 
           {
            *X1=*BX1;
            *Y1=*BY1;
           }
         }  
         goto s990;
s200:   if(IRC2 !=0 ) goto s300;// X3 Y3 NOT ON LINE 2;
/*C*/
/*C******* INTERSECTION ON LINE 2 ONLY*/
/*C*/
         *ICD=2;
         DTMN1=DSPTLN(BX1,BY1,EX1,EY1,X3,Y3,&N,M1);
         *N1=N;
         *X1=*BX1;
         *Y1=*BY1;
         if(*N1==2)
         { 
            *X1=*EX1;
            *Y1=*EY1;
         }
         *DMN=DSTMIN(BX2,BY2,EX2,EY2,X3,Y3,X1,Y1,&N);
         *N2=N;
         if(*N2==3)
         { 
            *X2=*X3;
            *Y2=*Y3;
         } 
         else 
         { if(*N2==2)
           {  
             *X2=*EX2;
             *Y2=*EY2;
           } 
           else 
           {
             *X2=*BX2;
             *Y2=*BY2;
           }
         }
         goto s990;
/*C*/
/*C******* INTERSECTION ON EXTENSION OF BOTH LINES*/
/*C*/
s300: *ICD=4;
      DISN1P=LDIST(*X1,*Y1,*X3,*Y3);
      DISN2P=LDIST(*X2,*Y2,*X3,*Y3);
      DISN12=LDIST(*X1,*Y1,*X2,*Y2);
      DISS=min(DISN1P,DISN2P);
      DISS=min(DISS,DISN12);
/*C      if(DABS(DISS-DISN12).LE.P_TOL)RETURN*/
      if(DISS <= P_TOL)
{
#if ENABLETRACE
GSSiExitProg (1208);
#endif
      	return;
}
/*C******* USE INTERSECTION INSTEAD*/
      *ICD=7;
{
#if ENABLETRACE
GSSiExitProg (1208);
#endif
     return;
}
/*C*/
/*C******* INTERSECTION ON BOTH LINE*/
/*C*/
s400:   *ICD=3;
        *N1=3;
        *N2=3;
        *DMN=0;
s990:
{
#if ENABLETRACE
GSSiExitProg (1208);
#endif
	 return;
}
#if ENABLETRACE
}
#endif
}
short INLNCK(const double *BX,const double *BY,const double *EX,const double *EY,
           const double *PX8,const double *PY8)
#if ENABLETRACE
{GSSiEnterProg (1209);
#endif
{                   
/*C**/
/*C******* SPECifICATIONS ************************************************/
/*C*                                                                    **/
/*C*       PROGRAM SUMMARY                                              **/
/*C*       ------- -------                                              **/
/*C*       THIS ROUTINE CHECKS TO SEE if PT(PX,PY) IS ON AND INSIDE OF  **/
/*C*       LINE FROM B.P.(BX,BY) TO E.P.(EX,EY). (double VERSION)       **/
/*C*                                                                    **/
/*C*       ARGUMENT DESCRIPTION                                         **/
/*C*       -------- -----------                                         **/
/*C*       PX8,PY8  POINTS TO BE CHECKED                      R*8       **/
/*C*       BX,BY    B.P. OF LINE                              R*8       **/
/*C*       EX,EY    E.P. OF LINE                              R*8       **/
/*C*       IRC      retrn CODE                               I*2       **/
/*C*                =0  ON THE LINE                                     **/
/*C*                =1  NOT ON THE LINE                                 **/
/*C*                                                                    **/
/*C*       VARIABLE DESCRIPTION                                         **/
/*C*       -------- -----------                                         **/
/*C*       XMIN,YMIN MINIMUM X,Y COORDINATES                  R*8       **/
/*C*       XMAN,YMAN MAXIMUM X,Y COORDINATES                  R*8       **/
/*C*       AZPT     AZIMUTH OF POINT                          R*8       **/
/*C*       AZLN     AZIMUTH OF LINE                           R*8       **/
/*C*                                                                    **/
/*C*       AUTHOR                                                       **/
/*C*       ------                                                       **/
/*C*       STEVE WU       DEPT OF PUBLIC WORK, CITY OF MINNEAPOLIS      **/
/*C*                      OCTOBER 17, 1979                              **/
/*C*                                                                    **/
/*C***********************************************************************/
/*C*/
      short    K;
      double  TEMP, DISTX, DISTE,AZLN, AZLNP, XMIN, YMIN, XMAX, YMAX,
               X0,Y0 ;
      short IRC = 1;
      XMIN=__min(*BX,*EX);
      YMIN=__min(*BY,*EY);
      XMAX=__max(*BX,*EX);
      YMAX=__max(*BY,*EY);
      if(*PX8<(XMIN-P_TOL)||*PX8>(XMAX+P_TOL))goto s100;
      if(*PY8<(YMIN-P_TOL)||*PY8>(YMAX+P_TOL))goto s100;
      DISTX = LDIST(*BX,*BY,*PX8,*PY8) ;
      if (DISTX <= P_TOL) 
      { // THIS WITHIN P_TOL OF THE POINT
{
#if ENABLETRACE
GSSiExitProg (1209);
#endif
         return 0;
}
      } 
      else 
      {
         DISTE = LDIST(*EX,*EY,*PX8,*PY8) ;
         if (DISTE <= P_TOL) 
         { // THIS WITHIN P_TOL OF THE POINT
{
#if ENABLETRACE
GSSiExitProg (1209);
#endif
            return 0;
}
         } 
         else 
         {
            AZLN = LGETAZ(*BX,*BY,*EX,*EY) ;
            AZLNP = LTWOPI(AZLN+HALFPI);// TURN LINE TO POINT 90 DEGREES;
            SECLIN8 (PX8,PY8,&AZLNP,BX,BY,&AZLN,&X0,&Y0,&K);
            TEMP = LDIST(*PX8,*PY8,X0,Y0);
            if (TEMP <= P_TOL) IRC = 0;
         } 
      }
s100:
{
#if ENABLETRACE
GSSiExitProg (1209);
#endif
	   return IRC;
}
#if ENABLETRACE
}
#endif
      } 

 short     INCRVE(const double *CX,
                const double *CY,
                const double *RX,
                const double *RY,
                const double *CL,
                const double *PtX,
                const double *PtY)
#if ENABLETRACE
{GSSiEnterProg (1210);
#endif
{      
//C*
//C******* SPECIFICATIONS ***********************************************
//C*                                                                    *
//C*       PROGRAM SUMMARY                                              *
//C*       ------- -------                                              *
//C*                                                                    *
//C*       ROUTINE INCRVE CHESKS TO SEE IF POINT(PX,PY) IS INSIDE OF    *
//C*       CURVE(DEFINED BY CX,XY,RX,RY,CL)                             *
//C*                                                                    *
//C*       ARGUMENT DESCRIPTION                                         *
//C*       -------- -----------                                         *
//C*       CX,CY    P.C. OF CURVE                                R*8    *
//C*       RX,RY    R.C. OF CURVE                                R*8    *
//C*       CL       LENGTH OF CURVE                              R*8    *
//C*                + IS CLOCKWISE, - IS CNTR CLOCKWISE                 *
//C*       PX,PY    POINT COORDINATE TO BE CHECKED               R*8    *
//C*       IRC      returnCODE                                  I*2    *
//C*                =0, POINT IS ON CURVE                               *
//C*                >0, POINT IS ON EXTENSION OF CURVE                  *
//C*                <0, POINT IS NOT ON CURVE AT ALL                    *
//C*                                                                    *
//C*       VARIABLE DESCRIPTION                                         *
//C*       -------- -----------                                         *
//C*       TX,TY    P.T. OF CURVE                                R*8    *
//C*       AZP      AZIMUTH OF RC TO POINT                       R*8    *
//C*       AZC      AZIMUTH OF R.C. TO P.C.                      R*8    *
//C*       AZT      AZIMUTH OF R.C. TO P.T.                      R*8    *
//C*       AZDFCT   AZIMUTH DIFFERENCE FROM P.C. TO P.T.         R*8    *
//C*       AZDFCP   AZIMUTH DIFFERENCE FROM P.C. TO POINT        R*8    *
//C*                                                                    *
//C*       AUTHOR                                                       *
//C*       ------                                                       *
//C*       STEVE WU   DEPT OF PUBLIC WORK, CITY OF MINNEAPOLIS          *
//C*                  OCTOBER 24,1983                                   *
//C*                                                                    *
//C*       MODIFIED                                                     *
//C*       --------                                                     *
//C*                                                                    *
//C*                                                                    *
//C**********************************************************************
//C
#define M1 -1
      double  RAD, AZTOL,DIS,ABSLEN, AZP,AZC,AZT,AZDFCP,AZDFCT,
             TX,TY;
      
      RAD=LDIST(*RX,*RY,*CX,*CY);
//c******* if rad is zero treat as point (JS 2/26/90/)
      if (RAD == 0)
      {
          if (LDIST (*PtX,*PtY, *CX, *CY) > P_TOL)
{
#if ENABLETRACE
GSSiExitProg (1210);
#endif
              return -1;
}
          else
{
#if ENABLETRACE
GSSiExitProg (1210);
#endif
              return  0;
}
      }
      AZTOL = P_TOL/RAD;
      DIS = LDIST(*RX,*RY,*PtX,*PtY);
//C*** change hard code tol (1.0D-3) to a relative tolerance
//C*** however, I don't know why 1.0D-3 is used for this condition. I just
//C*** divide 1.0D-3 by p_tol (which is 8.5D-5) and come up with 12 as a factor
//C*** (King 1/17/92)

        if(fabs(RAD-DIS) > 12.0*P_TOL)
{
#if ENABLETRACE
GSSiExitProg (1210);
#endif
        	return -1;
}
//C******* POINT NOT ON CURVE AT ALL
//C******* POINT ON CURVE OR EXTENSION OF CURVE
         ABSLEN = *CL;
         LOL8(CX,CY,RX,RY,CL,&ABSLEN,&TX,&TY,3);
         AZP=LGETAZ(*RX,*RY,*PtX,*PtY);
         AZC=LGETAZ(*RX,*RY,*CX,*CY);
         AZT=LGETAZ(*RX,*RY,TX,TY);
//c*** lda 11 feb 87 ***   IF(DABS(AZP-AZC)<=P_TOL.OR.DABS(AZP-AZT)<=P_TOL)THEN
         if(fabs(AZP-AZC) <= AZTOL || fabs(AZP-AZT) <=AZTOL)
{
#if ENABLETRACE
GSSiExitProg (1210);
#endif
         	return 0;
}
//C********** (PX,PY) HITS P.C. OR P.T.
//c*              AZDFCT=AZDF(AZC,AZT,CL)
         AZDFCT=fabs(*CL/RAD) ; //lda apr 90 to catch full circles
         AZDFCP = AZDF(AZC,AZP,*CL);
         if(AZDFCP <= AZDFCT)
{
#if ENABLETRACE
GSSiExitProg (1210);
#endif
         	return 0;
}
//C************* (PX,PY) IS ON THE CURVE
//C************* (PX,PY) IS ON THE EXTENSION OF CURVE
{
#if ENABLETRACE
GSSiExitProg (1210);
#endif
       return 1;
}
#if ENABLETRACE
}
#endif
 }

 double DSTMIN(const double *X1,const double *Y1,const double *X2, const double *Y2,
               const double *X3,const double *Y3,const double *XP, const double *YP,
               short *N)
#if ENABLETRACE
{GSSiEnterProg (1211);
#endif
{
/*C*
C******* SPECIFICATIONS ************************************************
C*                                                                     *
C*       PROGRAM SUMMARY                                               *
C*       ------- -------                                               *
C*       THIS ROUTINE RETURNS MINIMUM DIST PT FROM (XP,YP) TO OTHER    *
C*       THREE POINTS ON A LINE.(REAL*8 VERSION)                       *
C*                                                                     *
C*       ARGUMENT DESCRIPTION                                          *
C*       -------- -----------                                          *
C*       X1,Y1    PT ONE ON LINE                           R*8         *
C*       X2,Y2    PT TWO ON LINE                           R*8         *
C*       X3,Y3    PT THREE ON LINE                         R*8         *
C*       XP,YP    POINT                                    R*8         *
C*       N        1 FOR PT ONE,2 FOR PT TWO,3 FOR PT THREE I*2         *
C*       IRC      RETRN CODE                              I*2         *
C*                                                                     *
C*       VARIABLE DESCRIPTION                                          *
C*       -------- -----------                                          *
C*       DIST1    DISTANCE TO FIRST POINT                  R*8         *
C*       DIST2    DISTANCE TO SECOND POINT                 R*8         *
C*       DIST3    DISTANCE TO THIRD POINT                  R*8         *
C*                                                                     *
C*       AUTHOR                                                        *
C*       ------                                                        *
C*       STEVE WU       DEPT OF PUBLIC WORK, CITY OF MINNEAPOLIS       *
C*                   October 07, 1986                                  *
C*                                                                     *
C*       Rewritten by L Anderson Aug 88 to use P_TOL instead of 1d-4   *
C*                                                                     *
C*                                                                     *
C*                                                                     *
C* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
C*/
#include <xll.h>
#include <math.h>

     double DIST1, DIST2, DIST3, DISTT;
      DIST1 = LDIST(*X1,*Y1,*XP,*YP);
      DIST2 = LDIST(*X2,*Y2,*XP,*YP);
      DIST3 = LDIST(*X3,*Y3,*XP,*YP);
      DISTT = __min(DIST1,DIST2);
      DISTT = __min(DISTT,DIST3);
      if(fabs(DISTT-DIST1) <= P_TOL)*N = 1;
      if(fabs(DISTT-DIST2) <= P_TOL)*N = 2;
      if(fabs(DISTT-DIST3) <= P_TOL)*N = 3;
{
#if ENABLETRACE
GSSiExitProg (1211);
#endif
return DISTT;
}
#if ENABLETRACE
}
#endif
}
double DSPTLN(const double *X1,const double *Y1,const double *X2,const double *Y2,
              const double *XP,const double *YP, short *N, short IRC)
#if ENABLETRACE
{GSSiEnterProg (1212);
#endif
{
 /*     FUNCTION DSPTLN(X1,Y1,X2,Y2,XP,YP,N,IRC)
C*
C******* SPECIFICATIONS ************************************************
C*                                                                     *
C*       PROGRAM SUMMARY                                               *
C*       ------- -------                                               *
C*       THIS ROUTINE IDENTIFIES TWO ENDS OF A LINE WHICH              *
C*       MAKE UP THE MINIMUM (IRC<0) OR MAXIMUM (IRC>0) DISTANCE       *
C*       WITH A POINT.(REAL*8 VERSION)                                 *
C*                                                                     *
C*       ARGUMENT DESCRIPTION                                          *
C*        -------- -----------                                          *
C*       X1,Y1    B.P. OF THE LINE                         R*8         *
C*       X2,Y2    E.P. OF THE LINE                         R*8         *
C*       XP,YP    POINT                                    R*8         *
C*       N        INDICATE 1 FOR B.P. 2 FOR E.P.           I*2         *
C*       IRC      RETURN CODE                              I*2         *
C*                INCOMING >0 GET MAXIMUM DISTANCE PT'S                *
C*                                                                     *
C*       VARIABLE DESCRIPTION                                          *
C*       -------- -----------                                          *
C*       DIST1    DISTANT BETWEEN B.P. OF LINE AND 3RD PT  R*8         *
C*       DIST2    DISTANT BETWEEN E.P. OF LINE AND 3RD PT  R*8         *
C*                                                                     *
C*       AUTHOR                                                        *
C*       ------                                                        *
C*       STEVE WU       DEPT OF PUBLIC WORK, CITY OF MINNEAPOLIS       *
C*                   October 07, 1986                                  *
C*                                                                     *
C*       Rewritten by L Anderson Aug 88 to enhance its legibility      *
C*                                                                     *
C* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
C*
      IMPLICIT INTEGER*2 (I-N),REAL*8 (A-H,O-Z)
  */
      double DIST1, DIST2;  

      DIST1 = LDIST(*XP,*YP,*X1,*Y1);
      DIST2 = LDIST(*XP,*YP,*X2,*Y2);
      if(IRC > 0)
      {//then user wants maximum distance
        DIST1 = __max(DIST1,DIST2);
      }
      else  
      {//user wants the minimum distance
        DIST1 = __min(DIST1,DIST2); 
      }
      if(fabs(DIST1 - DIST2) < P_TOL)
      {
         *N = 2;// distance was from E.P. point
      }   
      else
      {
         *N = 1;// distance was from B.P. point
      }
{
#if ENABLETRACE
GSSiExitProg (1212);
#endif
     return DIST1;
}
#if ENABLETRACE
}
#endif
}
double DMNMX(const double *X1,const double *Y1,const double *X2,const double *Y2,
              const double *X3,const double *Y3,const double *X4,const double *Y4,
                        short *N1,short *N2,short *IRC)
#if ENABLETRACE
{GSSiEnterProg (1213);
#endif
{                        
/*C**/
/*C******* SPECifICATIONS *************************************************/
/*C*                                                                     **/
/*C*       PROGRAM SUMMARY                                               **/
/*C*       ------- -------                                               **/
/*C*       THIS ROUTINE IDENTifIES TWO  OF SEPERATE LINES WHICH      **/
/*C*       MAKE UP THE MINIMUM (IRC<0) OR MAXIMUM (IRC>0).               **/
/*C*       ARGUMENT DESCRIPTION                                          **/
/*C*                                                                     **/
/*C*       -------- -----------                                          **/
/*C*       X1,Y1    B.P. OF FIRST LINE                       R*8         **/
/*C*       X2,Y2    E.P. OF FIRST LINE                       R*8         **/
/*C*       X3,Y3    B.P. OF SECOND LINE                      R*8         **/
/*C*       X4,Y4    E.P. OF SECOND LINE                      R*8         **/
/*C*       N1       INDICATE 1 FOR B.P. 2 FOR E.P. OF LINE 1 I*2         **/
/*C*       N2       INDICATE 1 FOR B.P. 2 FOR E.P. OF LINE 2 I*2         **/
/*C*       IRC      retrn CODE                              I*2         **/
/*C*                <= 0 MINIMUM DISTANCE POINT                          **/
/*C*                >  0 MAXIMUM DISTANCE POINT                          **/
/*C*                                                                     **/
/*C*       VARIABLE DESCRIPTION                                          **/
/*C*       -------- -----------                                          **/
/*C*       DIS123   DISTANT BETWEEN  PTS OF LINE 1 & PT 3 R*8         **/
/*C*       DIS124   DISTANT BETWEEN  PTS OF LINE 1 & PT 4 R*8         **/
     short  N3, N4,IRC1;
      double DIS123, DIS124, DMAX;
      *N1=1;
      *N2=1;
      IRC1=*IRC;
      DIS123=DSPTLN(X1,Y1,X2,Y2,X3,Y3,&N3,IRC1);
      DIS124=DSPTLN(X1,Y1,X2,Y2,X4,Y4,&N4,IRC1);
      if(IRC > 0) goto s100;
           DMAX=min(DIS123,DIS124);
           goto s200;
s100:      DMAX=max(DIS123,DIS124);
s200:   if(fabs(DMAX-DIS123)>1e-4)goto s300;
           *N1= N3;
           *N2=1;
           goto s400;
s300:   *N1=N4;
        *N2=2;
s400:   *IRC=0;
{
#if ENABLETRACE
GSSiExitProg (1213);
#endif
      return DMAX;
}
#if ENABLETRACE
}
#endif
 }      

double DMNMX_TOL(const double *X1,const double *Y1,const double *X2,const double *Y2,
                 const double* X3,const double *Y3,const double *X4,const double *Y4,
                 short *N1,short *N2, const double TOL,short *IRC)
#if ENABLETRACE
{GSSiEnterProg (1214);
#endif
{
/*C**/
/*C******* SPECifICATIONS *************************************************/
/*C*                                                                     **/
/*C*       PROGRAM SUMMARY                                               **/
/*C*       ------- -------                                               **/
/*C*       THIS ROUTINE IDENTifIES TWO S OF SEPERATE LINES WHICH      **/
/*C*       MAKE UP THE MINIMUM (IRC<0) OR MAXIMUM (IRC>0).               **/
/*C*       ARGUMENT DESCRIPTION                                          **/
/*C*                                                                     **/
/*C*       -------- -----------                                          **/
/*C*       X1,Y1    B.P. OF FIRST LINE                       R*8         **/
/*C*       X2,Y2    E.P. OF FIRST LINE                       R*8         **/
/*C*       X3,Y3    B.P. OF SECOND LINE                      R*8         **/
/*C*       X4,Y4    E.P. OF SECOND LINE                      R*8         **/
/*C*       N1       INDICATE 1 FOR B.P. 2 FOR E.P. OF LINE 1 I*2         **/
/*C*       N2       INDICATE 1 FOR B.P. 2 FOR E.P. OF LINE 2 I*2         **/
/*C*       IRC      retrn CODE                              I*2         **/
/*C*                <= 0 MINIMUM DISTANCE POINT                          **/
/*C*                >  0 MAXIMUM DISTANCE POINT                          **/
/*C*                                                                     **/
/*C*       VARIABLE DESCRIPTION                                          **/
/*C*       -------- -----------                                          **/
/*C*       DIS123   DISTANT BETWEEN  PTS OF LINE 1 & PT 3 R*8         **/
/*C*       DIS124   DISTANT BETWEEN  PTS OF LINE 1 & PT 4 R*8         **/
/*C*       AUTHOR                                                        **/
/*C*       ------                                                        **/
/*C*       STEVE WU       DEPT OF PUBLIC WORK, CITY OF MINNEAPOLIS       **/
/*C*                                                                     **/
/*C*       CALLED ROUTINE DESCRIPTION                                    **/
/*C*       ------ ------- -----------                                    **/
/*C*       DMNMX                                              R*4        **/
/*C*       DSPTLN                                             R*8        **/
/*C*                                                                     **/
/*C*       AUTHOR                                                        **/
/*C*       ------                                                        **/
/*C*       STEVE WU    OCTOBER 07, 1986                                  **/
/*C*                                                                     **/
/*C************************************************************************/
/*C**/
       double DIS123, DIS124,DMNMX;
       short N3, N4, IRC1;
      *N1=1;
      *N2=1;
      IRC1=*IRC;
      DIS123=DSPTLN(X1,Y1,X2,Y2,X3,Y3,&N3,IRC1);
      DIS124=DSPTLN(X1,Y1,X2,Y2,X4,Y4,&N4,IRC1);
      if(IRC>0) goto s100;
      
       DMNMX=min(DIS123,DIS124);
       goto s200;
       
s100:   DMNMX=max(DIS123,DIS124);
s200:   if(fabs(DMNMX-DIS123)>TOL)goto s300;
        *N1=(short) N3;
        *N2=1;
        goto s400;
s300:   *N1=(short)N4;
      *N2=2;
s400: *IRC=0;
{
#if ENABLETRACE
GSSiExitProg (1214);
#endif
      return DMNMX;
}
#if ENABLETRACE
}
#endif
      }
//C*    (void)LOL8(X1,Y1,X2,Y2,CLEN,DIST,X3,Y3,ITYPE)
//C*
//C******** SPECIFICATIONS **********************************************
//C*                                                                    *
//C*        PROGRAM SUMMARY                                             *
//C*        ------- -------                                             *
//C*        LOL DETERMINES THE COORDINATES OF A POINT ON A   *
//C*        LINE OR CURVE WHEN GIVEN A PLUS OR MINUS DISTANCE ALONG     *
//C*        A LINE OR CURVE.(doubleVERSION)                            *
//C*                                                                    *
//C*        ARGUMENT DESCRIPTION                                        *
//C*        -------- -----------                                        *
//C*        X1,Y1 R*8 COORDINATES OF TEXT BASE LINE BEGINNING POINT     *
//C*        X2,Y2 R*8 COMPUTED OVER AND UP DISTANCES FROM BEG. POINT    *
//C*        CLEN  R*8 LENGTH OF CURVE                                   *
//C*        DIST  R*8 DISTANCE FROM BEG. COOR.                          *
//C*        X3,Y3 R*8 ACTUAL COORDINATES FOR BEGINNING OF TEXT          *
//C*        TYPE  I*2 TYPE OF RECORD                                    *
//C*                                                                    *
//C*        VARIABLE DESCRIPTION                                        *
//C*        -------- -----------                                        *
//C*        AZ    AZIMUTH                                 R*8           *
//C*        RAD   RADIUS                                  R*8           *
//C*                                                                    *
//C*        AUTHOR                                                      *
//C*        ------                                                      *
//C*        LARRY PRATT  MARCH 14, 1977                                 *
//C*        STEVE WU, REVISED,JUNE 1980                                 *
//C*                                                                    *
//C**********************************************************************
//C
void  LOL8(const double *X1,   const double *Y1,
           const double *X2,   const double *Y2,
           const double *CLEN, const double *DIST,
           double *X3, double *Y3, int ITYPE)
#if ENABLETRACE
{GSSiEnterProg (1215);
#endif
{ 
  double AZ=0, RAD, dist=0;
      if(ITYPE == 3) goto S10;
//C
//C******* DETERMINE X & Y CORR. OF A NEW POINT
//C
      AZ=0e0;
      if(fabs(*Y2-*Y1) < 1e-4 || fabs(*X2-*X1) < 1e-4)goto S5;
      AZ=LTWOPI(atan2(*Y2-*Y1,*X2-*X1));
S5:   *X3=*X1+*DIST*cos(AZ);
      *Y3=*Y1+*DIST*sin(AZ);
{
#if ENABLETRACE
GSSiExitProg (1215);
#endif
      return;
}
//C
//C******* DETERMINE RADIUS OF CURVE
//C
S10:   RAD=sqrt((*X1-*X2)*(*X1-*X2) + (*Y1-*Y2)*(*Y1-*Y2));
      if(RAD < 1e-4)goto S20;
//C
//C******* DETERMINE AZIMUTH FROM R.P. TO B.P. OF TEXT
//C   
      dist = *DIST;
      AZ = LTWOPI( atan2(*Y1-*Y2,*X1-*X2) - DSIGN( *DIST/RAD,*CLEN * *DIST));
//C
//C******* DETERMINE X & Y COOR. OF A NEW POINT
//C
S20:   *X3=*X2+RAD*cos(AZ);
       *Y3=*Y2+RAD*sin(AZ);
{
#if ENABLETRACE
GSSiExitProg (1215);
#endif
       return;
}
#if ENABLETRACE
}
#endif
}
      
 
 void CNGRNT(const double *BX1,const double *BY1,const double *EX1,const double *EY1,
             const double *BX2,const double *BY2,const double *EX2,const double *EY2,
                   short *N1,short *N2, short *IRC)
#if ENABLETRACE
{GSSiEnterProg (1216);
#endif
{

/*C**/
/*C******* SPECifICATIONS ************************************************/
/*C*                                                                    **/
/*C*       PROGRAM SUMMARY                                              **/
/*C*       ------- -------                                              **/
/*C*       THIS ROUTINE CHECKS TO SEE if TWO LINES ARE CONTINUING       **/
/*C*                                                                    **/
/*C*       ARGUMENT DESCRIPTION                                         **/
/*C*       -------- -----------                                         **/
/*C*       BX1,BY1  B.P. OF FIRST LINE                       R*8        **/
/*C*       EX1,EY1  E.P. OF FIRST LINE                       R*8        **/
/*C*       BX2,BY2  B.P. OF SECOND LINE                      R*8        **/
/*C*       EX2,EY2  E.P. OF SECOND LINE                      R*8        **/
/*C*       N1       INDICATE 1 FOR B.P. 2 FOR E.P. OF LINE 1 I*2        **/
/*C*       N2       INDICATE 1 FOR B.P. 2 FOR E.P. OF LINE 2 I*2        **/
/*C*       IRC      retrn CODE (0 if 1  CONTINUOUS       I*2        **/
/*C*                             1 if NEITHER AND 2 if BOTH             **/
/*C*                                                                    **/
/*C*                                                                    **/
/*C***********************************************************************/
/*C*/ 
     
      *IRC=1;
      *N1=0;
      *N2=0;
      if (LDIST (*BX1,*BY1,*BX2,*BY2) > P_TOL) goto s10;
           *N1=1;
           *N2=1;
           *IRC = 0;
           goto s30;
s10:  if (LDIST (*BX1,*BY1,*EX2,*EY2) > P_TOL) goto s20;
           *N1=1;
           *N2=2;
           *IRC = 0;
s20:  if (LDIST (*EX1,*EY1,*BX2,*BY2) > P_TOL) goto s30;
           *N1=2;
           *N2=1;
           if (*IRC == 0) 
           {
               *IRC = 2;
           } 
           else 
           {
               *IRC = 0;
           } 
{
#if ENABLETRACE
GSSiExitProg (1216);
#endif
           return;
}
s30:  if (LDIST (*EX1,*EY1,*EX2,*EY2) > P_TOL) goto s40;
           *N1=2;
           *N2=2;             
           if (*IRC == 0) 
           {  
               *IRC = 2;
           } 
           else 
           {
               *IRC = 0;
           }   
s40: 
{
#if ENABLETRACE
GSSiExitProg (1216);
#endif
	   return ;
}
#if ENABLETRACE
}
#endif
      }
      
void ThinPoly (LPLONG pnPnts, HPDPOINT pPoints, double maxdist)
#if ENABLETRACE
{GSSiEnterProg (1217);
#endif
{   
	long	ipoint=2, jpoint, nNewPnts=0, lpoint=0;
	HANDLE	hNewPoints = GSSiGlobAlloc ( 293,GMEM_MOVEABLE,*pnPnts*sizeof(DPOINT));
	HPDPOINT	NewPoints=(HPDPOINT)GlobalLock (hNewPoints); 
	double	A1, A2;
	DPOINT	IntPoint;
	short	IRC;
//	char	str[64];
	
	if (maxdist <= 0 || *pnPnts < 3)
		goto Exit; 
	NewPoints[nNewPnts++] = *pPoints;
	while (ipoint < *pnPnts)
	{
//          A1 = getazd (&pPoints[lpoint],&pPoints[ipoint]);
          for (jpoint=lpoint+1; jpoint<ipoint; jpoint++)
          {
/*	          A2 = LTWOPI (A1 + HALFPI);
	          if (LIN_SEC (pPoints[lpoint].x,pPoints[lpoint].y,A1, pPoints[jpoint].x,pPoints[jpoint].y, A2, &IntPoint.x, &IntPoint.y))
			  {
				  IntPoint.x = (pPoints[lpoint].x + pPoints[jpoint].x) / 2;
				  IntPoint.y = (pPoints[lpoint].y + pPoints[jpoint].y) / 2;
			  }
			  */ //not sure what that was all about
			  IntPoint = pPoints[lpoint];
	          if (ldistp (pPoints[jpoint],IntPoint) > maxdist)
	          {
	          	NewPoints[nNewPnts++] = pPoints[jpoint];
	          	lpoint = jpoint;
	          	ipoint = lpoint + 2;
	          	goto Next;
	          }
	      }
		  ipoint++;
Next:;
	}
   	NewPoints[nNewPnts++] = pPoints[(*pnPnts)-1];
	hmemmove ((HPSTR)pPoints,(HPSTR)NewPoints,nNewPnts*sizeof(DPOINT));  
//	sprintf (str,"Thinned from %ld to %ld",*pnPnts,nNewPnts);
//	SetWindowText (hWndMain,str);
	*pnPnts = nNewPnts; 
Exit:
	GSSiGlobUlFree (&hNewPoints);
{
#if ENABLETRACE
GSSiExitProg (1217);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

int	ReducePolyPoints (int nPnts,LPHANDLE phPoly,int MaxPoints)
{
	double polylength, maxdist;
	LPDPOINT	pPoints=GlobalLock (*phPoly);

	polylength = GetPolyLengthD (pPoints,nPnts);
	maxdist = ((polylength * MaxPoints) / nPnts)/nPnts;
	ThinPoly (&nPnts, pPoints, maxdist);
	while (nPnts > MaxPoints)
	{
		maxdist *= 2;
		ThinPoly (&nPnts, pPoints, maxdist);
	}
	GlobalUnlock (*phPoly);
	return nPnts;
}

double ComputeAreaAreaDH (HANDLE hPoints,long nPnts,LPDOUBLE pPerim)
{
	HPDPOINT pPoints = (HPDPOINT)GlobalLock (hPoints); 
	double	size = ComputeAreaAreaD (pPoints,nPnts,pPerim);
	
	GlobalUnlock (hPoints);
	return size;
}

BOOL GetItemMidpoint (LPSTR TagOrRef,LPDPOINT pMidPoint)
{
	BOOL	rtn=FALSE;
	long	npnts;
	HANDLE	hPoly=0;

	if (PickByTagOrRefno(TagOrRef,-1))
	{
		if (PickList[0].Type == 1)
		{
			rtn = TRUE;
			*pMidPoint = PickList[0].BeginPoint;
		}
		else if (GetPolyPnts ((LPPICKDATAHEADER)&PickList[0],FALSE,&npnts,&hPoly,TRUE))
		{
			if (PickList[0].Type == 3)
				*pMidPoint = ComputeAreaMidpoint (hPoly,npnts);
			else
				*pMidPoint = ComputePolylineMidpoint (hPoly,npnts);
			rtn = TRUE;
		}
	}
	GSSiGlobFree (&hPoly);
	return rtn;
}

DPOINT ComputePolylineMidpoint2 (HPDPOINT pPoints,long nPnts)
{
	DPOINT		MidP;
	double		Length = GetPolyLengthD (pPoints,nPnts);

	MidP = PointAtDistOnPoly (pPoints,nPnts,Length/2,0,0);
	return MidP;
}
DPOINT ComputePolylineMidpoint (HANDLE hPoints,long nPnts)
{
	DPOINT		MidP;
	HPDPOINT	pPoints = GlobalLock (hPoints);

	MidP = ComputePolylineMidpoint2 (pPoints,nPnts);
	GlobalUnlock (hPoints);
	return MidP;
}


DPOINT ComputeAreaMidpoint (HANDLE hPoints,long nPnts)
{
	HPDPOINT pPoints = (HPDPOINT)GlobalLock (hPoints);   
	DPOINT	mp = ComputeAreaMidpoint2 (pPoints,nPnts);
	
	GlobalUnlock (hPoints);      
	return mp;
}

DPOINT ComputeAreaMidpoint2 (HPDPOINT pPoints,long nPnts)
{
	DPOINT	mp;
	double	x=0, y=0, d, totd=0; 
	ULONG	i;
	
	if (nPnts <= 1)
		return *pPoints;
	for (i=1;i<nPnts;i++)
	{
		d = ldistp (pPoints[i-1],pPoints[i]);
		totd += d;
		mp = MidPointD (pPoints[i-1],pPoints[i]);
		x += mp.x * d;
		y += mp.y * d;
	}
	if (!SameDPoint (&pPoints[0],&pPoints[nPnts-1]))
	{
		d = ldistp (pPoints[0],pPoints[nPnts-1]);
		totd += d;
		mp = MidPointD (pPoints[0],pPoints[nPnts-1]);
		x += mp.x * d;
		y += mp.y * d;
	}
	if (totd)
	{
		mp.x = x / totd;
		mp.y = y / totd; 
	}
	else
		mp = pPoints[0];
	return mp;
}

double RectArea (LPRECT pRect)
{
	double w=(double)pRect->right - (double)pRect->left;
	double h=(double)pRect->top - (double)pRect->bottom;
	double Area = w * h;
	
	return fabs(Area);
}
	
double RectArea16 (LPRECT16 pRect)
{
	double w=(double)pRect->right - (double)pRect->left;
	double h=(double)pRect->top - (double)pRect->bottom;
	double Area = w * h;
	
	return fabs(Area);
}
	
double BoundsArea (LPMNMXCORD pRect)
{
	double w=(double)pRect->xmx - (double)pRect->xmn;
	double h=(double)pRect->ymx - (double)pRect->ymn;
	double Area = w * h;
	
	return fabs(Area);
}
	
double ComputeAreaAreaD (HPDPOINT lpDpoints,long nPnts,LPDOUBLE pPerim)
#if ENABLETRACE
{GSSiEnterProg (404);
#endif
{    
	DWORD	i; 
	DPOINT	BeginPoint, EndPoint, LastPoint;
	double	Area=0,Perim=0;    

	if (nPnts > 2)
	{
		for (i = 0; i < nPnts; i++, lpDpoints++)
		{
			if (!i)
				BeginPoint = *lpDpoints;
			else
			{
				Perim += ldistp(LastPoint, *lpDpoints);
				Area += (LastPoint.y - lpDpoints->y) * (lpDpoints->x + LastPoint.x) / 2;
			}
			LastPoint = *lpDpoints;
		}
		Perim += ldistp(LastPoint, BeginPoint);
		Area += (LastPoint.y - BeginPoint.y) * (BeginPoint.x + LastPoint.x) / 2;
		if (pPerim)
			*pPerim = Perim;
	}
{
#if ENABLETRACE
GSSiExitProg (404);
#endif
	return (Area);
}
#if ENABLETRACE
}
#endif
}  

double ArcDistance(DPOINT p1, DPOINT p2)  
// retrns distance in meters between two lat-longs
#if ENABLETRACE
{GSSiEnterProg (192);
#endif
{
  double GreatCircle, cos_c, ArcCos, sina,sinb, cosa, cosb, cosl;

    sina =  sin(p1.y/DEGRAD);
    sinb =  sin(p2.y/DEGRAD);
    cosa =  cos(p1.y/DEGRAD);
    cosb =  cos(p2.y/DEGRAD);
    cosl =  cos((p2.x-p1.x)/DEGRAD);

   cos_c = sina*sinb+cosa*cosb*cosl;
   ArcCos = acos(cos_c);
   GreatCircle = fabs(6371100 * ArcCos);


{
#if ENABLETRACE
GSSiExitProg (192);
#endif
return GreatCircle;
}
#if ENABLETRACE
}
#endif
}  

DPOINT NewLatLong(double Lat,double Long,double Dist,double Direction)
#if ENABLETRACE
{GSSiEnterProg (193);
#endif
{
 DPOINT LatLong;
//given:          Fixed 7 Oct 96 lda
//  Point N is at the North Pole
//  Point A is at the Given Lat,Long
//  Length of two sides of a triangle and the angle it forms
    double  cosc,cosb,cosa,sinb,sinc,sina,cosC,cosA;



    cosc = cos(Dist/6371100);
    sinc = sin(Dist/6371100);
    cosb = cos((90e0-Lat)/DEGRAD);
    sinb = sin((90e0-Lat)/DEGRAD);
    cosa = cos(Direction)*sinb*sinc+cosb*cosc;
    cosA = acos(cosa);
    cosA *= DEGRAD;
    LatLong.y = 90e0 - cosA;   
    sina = acos(cosa);
    sina = sin(sina);
    cosC = (cosc-(cosb*cosa)) / (sinb*sina);
	cosC = max (-1.0,min (cosC,1.0));
    cosC = acos(cosC);
    cosC *= DEGRAD;
    if(Direction > PY)
      LatLong.x = Long - cosC;
    else
      LatLong.x = Long + cosC;  

{
#if ENABLETRACE
GSSiExitProg (193);
#endif
  return LatLong;
}
#if ENABLETRACE
}
#endif
} 

POINT	DPointToPIAAPointWindow (HPDPOINT pDPoint,LPMNMXCORD pBounds,LPDOUBLE pFactor,short Offset)
{
	POINT	Point;
	
	Point.x = Offset + IDNINT((pDPoint->x - pBounds->xmn) * (*pFactor));
	Point.y = Offset + IDNINT((pDPoint->y - pBounds->ymn) * (*pFactor));	
	return Point;
}

POINT	DPointToPIAAPoint (HPDPOINT pDPoint,LPMNMXCORD pBounds,LPDOUBLE pFactor,short Offset,short Type)
{
	POINT	Point;
	
	if (Type)
		return DPointToPIAAPointWindow (pDPoint,pBounds,pFactor,Offset);
	Point.x = Offset + IDNINT((pDPoint->x - pBounds->xmn) * (*pFactor));
	Point.y = Offset + IDNINT((pBounds->ymx - pDPoint->y) * (*pFactor));	
	return Point;
}

DPOINT	PIAAPointToDPointWindow (POINT Point,LPMNMXCORD pBounds,LPDOUBLE pFactor,short Offset)
{
	DPOINT	DPoint;
	
	DPoint.x = pBounds->xmn + (Point.x - Offset)/(*pFactor);
	DPoint.y = pBounds->ymn + (Point.y - Offset)/(*pFactor);	
	return DPoint;
}

DPOINT	PIAAPointToDPoint (POINT Point,LPMNMXCORD pBounds,LPDOUBLE pFactor,short Offset,short Type)
{
	DPOINT	DPoint; 
	
	if (Type)
		return PIAAPointToDPointWindow (Point,pBounds,pFactor,Offset);	
	DPoint.x = pBounds->xmn + (Point.x - Offset)/(*pFactor);
	DPoint.y = pBounds->ymx - (Point.y - Offset)/(*pFactor);	
	return DPoint;
}

/*POINT	DPointToPIAAPoint2 (HPDPOINT pDPoint,LPMNMXCORD pBounds,LPDOUBLE pFactor)
{
	POINT	Point;
	
	Point.x = 3 + IDNINT((pDPoint->x - pBounds->xmn) * (*pFactor));
	Point.y = 3 + IDNINT((pDPoint->y - pBounds->ymn ) * (*pFactor));	
	return Point;
}*/

int	PointInAreaAccelerator (LPDPOINT pDPoint,HANDLE hAccelerator)
{
	int	iacc = 0;
	if (hAccelerator)
	{
		LPPIAAStruct pPIAA=(LPPIAAStruct)GlobalLock (hAccelerator);
		long loc;
		POINT	ACPoint = DPointToPIAAPoint (pDPoint,&pPIAA->Bounds,&pPIAA->Factor,pPIAA->Offset,pPIAA->Type);
		HPBYTE	barray;
		BOOL PlotArray=FALSE;
		int row, col;
		
		barray = (HPBYTE)&pPIAA->barray[0];
		if (PlotArray)
		{
			POINT	p, wp;
			DPOINT	DPoint = PIAAPointToDPoint (ACPoint,&pPIAA->Bounds,&pPIAA->Factor,pPIAA->Offset,pPIAA->Type); 
			COLORREF	color[3];

			color[0] = 0;
			color[1] = RGB(255,255,0);
			color[2] = RGB(0,255,0);
			SaveDC (CurView->hDC);
			SetDisplayMode (CurView->hDC, GF_TEXTMODE);
			for (row=0;row<pPIAA->Height;row++)
			{
				p.y = row;
				for (col=0;col<pPIAA->Width;col++)
				{
					p.x = col;
					DPoint = PIAAPointToDPoint (p,&pPIAA->Bounds,&pPIAA->Factor,pPIAA->Offset,pPIAA->Type); 
					wp = BasePtToScreenPt (&DPoint);
					loc = (long)p.x + (long)p.y * pPIAA->Width;
					SetPixel (CurView->hDC,wp.x,wp.y,color[barray[loc]]);
				}
			}
			RestoreDC (CurView->hDC,-1);
		}
		if (ACPoint.x < 0 || ACPoint.x > pPIAA->Width-1 ||
			ACPoint.y < 0 || ACPoint.y > pPIAA->Height-1)
			iacc = 0;
		else if (!pPIAA->Compression)
		{   
			loc = (long)ACPoint.x + (long)ACPoint.y * pPIAA->Width;
			iacc = barray[loc];
		}
		else if (pPIAA->Compression == 1)
		{ 
			LPUSHORT	pArray=(LPUSHORT)&pPIAA->barray[0];
			USHORT		CurVal;
			int			bit;

			loc = 0;
			for (row = 0;row < ACPoint.y;row++)
				loc += pArray[loc];
			loc++;
			CurVal = pArray[loc];
			col = 0;
			while (col + pArray[loc+1]*16 + 16 < ACPoint.x)
			{
				col += pArray[loc+1]*16 + 16;
				loc += 2;
				CurVal = pArray[loc];
			}
			bit = ACPoint.x - col;
			bit = bit % 16;
			iacc = GetBit ((short)bit,(LPSTR)&CurVal);
		}

		GlobalUnlock (hAccelerator);
	}
	return iacc;
}

HANDLE  PointInAreaAcceleratorSetup (DWORD nPoints, HPDPOINT pAreaPoints,int nPoly,HANDLE hPolyPartLen, HANDLE hMaskAccelerator)
#if ENABLETRACE
{GSSiEnterProg (1379);
#endif
{
    DWORD   i; 
    MNMXCORD	Bounds;        
    HDC		hDC,hDCMain;
    HBITMAP	hBM=0, hBMOld=0; 
    double	Factor; 
    UINT	Height,Width,irow,icol;
    DWORD	loc;  
    HANDLE	hPIAA=0; 
    HANDLE	hPoint = GSSiGlobAlloc ( 294,GMEM_MOVEABLE,max(5,(long)nPoints+1) * sizeof(POINT));
    HPPOINT	pPoint = (HPPOINT)GlobalLock(hPoint);
	HBRUSH	hRedBrush =   CreateSolidBrush(RGB(255,   0,   0));
    HBRUSH	hGreenBrush = CreateSolidBrush(RGB(  0, 255,   0));
    HBRUSH	hBlueBrush = CreateSolidBrush(RGB(  0, 0, 255));
	HBRUSH	OldBrush; 
	HPEN	OldPen;
	HPEN	hBluePen = CreatePen(PS_SOLID,PIAWidthFactor,RGB(0,0,255));
	HPEN	hRedPen = CreatePen(PS_SOLID,5,RGB(255,0,0));
    COLORREF	Red, Green, Blue; 
    long	memsize,ii;
	LPPIAAStruct pPIAA;
	HPBYTE	array;   
	double	Totx=0, Toty=0, dWidth, dHeight;    
	BOOL	savebm=FALSE;  
	DPOINT	DPoint;   
	POINT	PIAAPoint;  
	mnmxCor InBounds;   
	double	FAC=PIASizeFactor;   
	short	Offset=3;
    BITMAP	bm;   
	BOOL	saveuseGDIPlus = useGDIPlus;

	useGDIPlus = FALSE;
	
//	if (FAC > 5)
//		ii=1;
	if (nPoints < 16)
		FAC /= 10;
		
    DBoundsInit (&Bounds);  
     
	for (i=0;i<nPoints;i++)
	{
		Totx += pAreaPoints[i].x;
		Toty += pAreaPoints[i].y;
		AddDPointToMinMax (&pAreaPoints[i],&Bounds);
	}  
	if (Bounds.ymx - Bounds.ymn <= 0)
		goto Exit2;
	Bounds = FactorBounds (&Bounds,1.01);
	Factor = (Bounds.xmx - Bounds.xmn)/(Bounds.ymx - Bounds.ymn);
	if (Factor <= 0)
		goto Exit2;
	dWidth = min (10000,100 * Factor);
	dHeight = dWidth / Factor; 
	Factor = sqrt ((((double)USHRT_MAX*FAC)-2*sizeof(PIAAStruct)) / (dWidth * dHeight)); 
	dHeight *= Factor;
	dWidth *= Factor; 
	Height = dHeight - 1;
	Width  = dWidth - 1;
	Factor = Width / (Bounds.xmx - Bounds.xmn); 
	Height = Factor * (Bounds.ymx - Bounds.ymn);
	hDCMain = GetDC (hWndMain);
    hDC = CreateCompatibleDC(hDCMain); 
    Height *= PIASizeFactor;
    Width *= PIASizeFactor;
    hBM = CreateCompatibleBitmap(hDCMain,Width,Height); 
    GetObject(hBM, sizeof(bm), (LPSTR)&bm);
	ReleaseDC (hWndMain,hDCMain);
    hBMOld = SelectObject(hDC,hBM);
	SetMapMode    ( hDC, MM_ISOTROPIC );
    SetWindowOrgEx  ( hDC, 0, 0,0 );
    SetViewportOrgEx( hDC, 0, 0,0 );    
  	SetWindowExtEx  ( hDC, 1024, 1024,0 ); 
	SetViewportExtEx( hDC, 1024, 1024,0 );         
	OldBrush = SelectObject (hDC,hRedBrush);  
    OldPen = SelectObject (hDC,hRedPen);
	pPoint[0].x = pPoint[0].y = 0;
	pPoint[1].x = 0;
	pPoint[1].y = Height;
	pPoint[2].x = Width;
	pPoint[2].y = Height;
	pPoint[3].x = Width;
	pPoint[3].y = 0;
	pPoint[4] = pPoint[0];
	Polygon (hDC,pPoint,5); 
	Red = GetPixel (hDC,Width/2,Height/2); 
	SelectObject (hDC,hGreenBrush);  
	Polygon (hDC,pPoint,5); 
	Green = GetPixel (hDC,Width/2,Height/2); 
	SelectObject (hDC,hBlueBrush);  
	Polygon (hDC,pPoint,5); 
	Blue = GetPixel (hDC,Width/2,Height/2);    
	SelectObject (hDC,hRedBrush);
	Polygon (hDC,pPoint,5); 
	Factor = (Width-9) / (Bounds.xmx - Bounds.xmn);    
	if (Factor <= 0)
		goto Exit;
    for (i=0;i<nPoints;i++)
	{
    	pPoint[i] = DPointToPIAAPoint (&pAreaPoints[i],&Bounds,&Factor,Offset,0); 
		if (pPoint[i].x < Width - 5 && pPoint[i].y < 5)
			ii=1;
	}
    pPoint[nPoints] = pPoint[0];  
	SelectObject (hDC,hGreenBrush);    
    SelectObject (hDC,hBluePen);

	if (nPoly > 1 && hPolyPartLen)
	{
		LPINT	pPartLen = (LPINT)GlobalLock (hPolyPartLen);
		LPPOINT pPnt = pPoint;

		for (i=0;i<nPoly;i++)
		{ 
			int	np = *pPartLen;
			int	dec = 0;

			Polyline (hDC,pPnt,np); 
			pPnt+=*pPartLen++;
			if (i)
				pPnt++;
		}
		GlobalUnlock (hPolyPartLen);
	}
	else
		Polyline (hDC,pPoint,(int)nPoints+1);
    SelectObject (hDC,GetStockObject (NULL_PEN));
    Polygon (hDC,pPoint,(int)nPoints);
    SelectObject (hDC,OldBrush);
    SelectObject (hDC,OldPen);  
    memsize = (long)sizeof(PIAAStruct)+(long)Width*(long)Height;
/*    if (memsize > UINT_MAX) 
    {   
    	char	str[256];
    	sprintf (str,"memsize too big:%ld",memsize);
    	MessageBox (0,str,0,MB_OK);  
    } */
    hPIAA = GSSiGlobAlloc (1778,GMEM_MOVEABLE,memsize);
    pPIAA = (LPPIAAStruct)GlobalLock (hPIAA); 
    pPIAA->Offset = Offset;   
    pPIAA->Type = 0;
    pPIAA->Compression = 0;
    pPIAA->Factor = Factor;
    pPIAA->Bounds = Bounds;
    pPIAA->Width = Width;
    pPIAA->Height = Height;  
    pPIAA->AveX = Totx / nPoints;
    pPIAA->AveY = Toty / nPoints;
    loc = 0; 
    array = (HPBYTE)&pPIAA->barray[0];  
    MinMaxInit (&InBounds);
	pPIAA->HaveInPoints = FALSE; 
	if (bm.bmBitsPixel == 32)
	{
		int		l = (int)bm.bmWidthBytes * (int)bm.bmHeight; 
		HANDLE	hBits = GSSiGlobAlloc (0,GMEM_MOVEABLE,l);
//		LPCOLORREF	pBits = GlobalLock (hBits);
		LPRGBQUAD	pBits = GlobalLock (hBits);

		if (hBMOld)    
    		SelectObject(hDC, hBMOld);  
		GetBitmapBits(hBM,l,pBits);
		for (irow = 0;irow<Height;irow++) 
		{
    		PIAAPoint.y = irow;
    		for (icol = 0;icol<Width;icol++)
			{
				COLORREF c = COLORREFFromRGBQUAD (*pBits++);  
    			
//				if (irow == 493 && icol == 808)
//					ii=1;
	    		PIAAPoint.x = icol;
				if (c == Green)
					array[loc]=1;
				else if (c == Blue)
					array[loc] = 2;
				else
					array[loc] = 0; 
				if (array[loc] && hMaskAccelerator)
				{ 
					DPoint = PIAAPointToDPoint (PIAAPoint,&pPIAA->Bounds,&pPIAA->Factor,pPIAA->Offset,pPIAA->Type); 
					DPoint = WinPtToBasePtD (&DPoint);
		      		if (PointInAreaAccelerator (&DPoint,hMaskAccelerator) != 1)
						array[loc]=0; 
				} 
				if (array[loc] == 1)  
				{
            		pPIAA->HaveInPoints = TRUE;
            		AddPointToMinMax (PIAAPoint,&InBounds);  
				}
				loc++;
			}
		}
		GSSiGlobUlFree (&hBits);
	}
	else
	{
		for (irow = 0;irow<Height;irow++) 
		{
    		PIAAPoint.y = irow;
    		for (icol = 0;icol<Width;icol++)
			{
				COLORREF c = GetPixel (hDC,icol,irow);  
    				
	    		PIAAPoint.x = icol;
				if (c == Green)
					array[loc]=1;
				else if (c == Blue)
					array[loc] = 2;
				else
					array[loc] = 0; 
				if (array[loc] && hMaskAccelerator)
				{ 
					DPoint = PIAAPointToDPoint (PIAAPoint,&pPIAA->Bounds,&pPIAA->Factor,pPIAA->Offset,pPIAA->Type); 
					DPoint = WinPtToBasePtD (&DPoint);
		      		if (PointInAreaAccelerator (&DPoint,hMaskAccelerator) != 1)
						array[loc]=0; 
				} 
				if (array[loc] == 1)  
				{
            		pPIAA->HaveInPoints = TRUE;
            		AddPointToMinMax (PIAAPoint,&InBounds);  
				}
				loc++;
			}
		}
		if (hBMOld)    
    		SelectObject(hDC, hBMOld);  
	} 
	DBoundsInit (&pPIAA->InBounds);
	DPoint = PIAAPointToDPoint (*(LPPOINT)&InBounds.xmn,&pPIAA->Bounds,&pPIAA->Factor,pPIAA->Offset,pPIAA->Type); 
	AddDPointToMinMax (&DPoint,&pPIAA->InBounds);
	DPoint = PIAAPointToDPoint (*(LPPOINT)&InBounds.xmx,&pPIAA->Bounds,&pPIAA->Factor,pPIAA->Offset,pPIAA->Type); 
	AddDPointToMinMax (&DPoint,&pPIAA->InBounds);
    GlobalUnlock (hPIAA);			
Exit:
    GSSiGlobUlFree (&hPoint);  
    DeleteDC(hDC);
    GSSiDeleteObject (&hRedBrush);
    GSSiDeleteObject (&hGreenBrush); 
    GSSiDeleteObject (&hBlueBrush); 
    GSSiDeleteObject (&hBluePen); 
    GSSiDeleteObject (&hRedPen);
    if (savebm) 
    	SaveBitmap (hBM,"c:\\test.bmp",0,0);
    GSSiDeleteObject(&hBM); 
Exit2:
	useGDIPlus = saveuseGDIPlus;
{
#if ENABLETRACE
GSSiExitProg (1379);
#endif
    return hPIAA;
}
#if ENABLETRACE
}
#endif
}  
    
HANDLE  PointInAreaAcceleratorSetupMono (DWORD nPoints, HPDPOINT pAreaPoints, int nPoly,HANDLE hPolyPartLen,double Offset,LPMNMXCORD pBounds)
#if ENABLETRACE
{GSSiEnterProg (1379);
#endif
{
    DWORD   i; 
    HDC		hDC=0,hDCMain;
    HBITMAP	hBM=0, hBMOld=0; 
    double	Factor; 
    UINT	Height,Width,irow,icol;
    DWORD	loc;  
    HANDLE	hPIAA=0; 
    HANDLE	hPoint = GSSiGlobAlloc ( 294,GMEM_MOVEABLE,max(5,(long)nPoints+1) * sizeof(POINT));
    HPPOINT	pPoint = (HPPOINT)GlobalLock(hPoint);
	HBRUSH	OldBrush; 
	HPEN	OldPen, hPen;
    long	memsize,ii;
	LPPIAAStruct pPIAA;
	HPUSHORT	barray;   
	double	Totx=0, Toty=0, dWidth, dHeight;    
	BOOL	savebm=FALSE;  
	DPOINT	DPoint;   
	POINT	PIAAPoint;  
	mnmxCor InBounds;   
	double	FAC=PIASizeFactor;   
	short	Offsetx=3;
    BITMAP	bm;   
	double	PenWidth;
	BOOL	saveuseGDIPlus = useGDIPlus;

	useGDIPlus = FALSE;
//	if (FAC > 5)
//		ii=1;
	if (nPoints < 16)
		FAC /= 10;
		
	if (pBounds->ymx - pBounds->ymn <= 0)
		goto Exit;
	Factor = (pBounds->xmx - pBounds->xmn)/(pBounds->ymx - pBounds->ymn);
	if (Factor <= 0)
		goto Exit;
	dWidth = min (10000,100 * Factor);
	dHeight = dWidth / Factor; 
	Factor = sqrt ((((double)USHRT_MAX*FAC)-2*sizeof(PIAAStruct)) / (dWidth * dHeight)); 
	dHeight *= Factor;
	dWidth *= Factor; 
	Height = dHeight - 1;
	Width  = dWidth - 1;
	Factor = Width / (pBounds->xmx - pBounds->xmn); 
	Height = Factor * (pBounds->ymx - pBounds->ymn);
	hDCMain = GetDC (hWndMain);
    hDC = CreateCompatibleDC(hDCMain); 
    Height *= PIASizeFactor * 16;
    Width *= PIASizeFactor * 16;
//    hBM = CreateCompatibleBitmap(hDCMain,Width,Height); 
	hBM = CreateBitmap (Width,Height,1,1,0);
    GetObject(hBM, sizeof(bm), (LPSTR)&bm);
	ReleaseDC (hWndMain,hDCMain);
    hBMOld = SelectObject(hDC,hBM);
	SetMapMode    ( hDC, MM_ISOTROPIC );
    SetWindowOrgEx  ( hDC, 0, 0,0 );
    SetViewportOrgEx( hDC, 0, 0,0 );    
  	SetWindowExtEx  ( hDC, 1024, 1024,0 ); 
	SetViewportExtEx( hDC, 1024, 1024,0 );         
	OldBrush = SelectObject (hDC,GetStockObject(BLACK_BRUSH));  
    OldPen = SelectObject (hDC,GetStockObject(BLACK_PEN)); 
	pPoint[0].x = pPoint[0].y = 0;
	pPoint[1].x = 0;
	pPoint[1].y = Height;
	pPoint[2].x = Width;
	pPoint[2].y = Height;
	pPoint[3].x = Width;
	pPoint[3].y = 0;
	pPoint[4] = pPoint[0];
	Polygon (hDC,pPoint,5); 
	Factor = (Width-9) / (pBounds->xmx - pBounds->xmn);    
	PenWidth = (Offset * 2 * Width)/(pBounds->xmx - pBounds->xmn);
	if (Factor <= 0)
		goto Exit;
    for (i=0;i<nPoints;i++)
	{
    	pPoint[i] = DPointToPIAAPoint (&pAreaPoints[i],pBounds,&Factor,0,0); 
		if (pPoint[i].x < Width - 5 && pPoint[i].y < 5)
			ii=1;
	}
    pPoint[nPoints] = pPoint[0];  
	SelectObject (hDC,GetStockObject(WHITE_BRUSH));
    hPen = CreatePen (PS_SOLID,(int)IDNINT(PenWidth),RGB(255,255,255));
    SelectObject (hDC,hPen);
    Polyline (hDC,pPoint,(int)nPoints+1);
    SelectObject (hDC,GetStockObject (NULL_PEN));
	GSSiDeleteObject (&hPen);
    Polygon (hDC,pPoint,(int)nPoints);
    SelectObject (hDC,OldBrush);
    SelectObject (hDC,OldPen);  
    memsize = 1024 * 1024 * 16;//(long)sizeof(PIAAStruct)+(long)Width*(long)Height;
/*    if (memsize > UINT_MAX) 
    {   
    	char	str[256];
    	sprintf (str,"memsize too big:%ld",memsize);
    	MessageBox (0,str,0,MB_OK);  
    } */
    hPIAA = GSSiGlobAlloc (0,GMEM_MOVEABLE,memsize);
    pPIAA = (LPPIAAStruct)GlobalLock (hPIAA); 
    pPIAA->Offset = Offset;   
    pPIAA->Type = 0;
	pPIAA->Compression = 1;
    pPIAA->Factor = Factor;
    pPIAA->Bounds = *pBounds;
    pPIAA->Width = Width;
    pPIAA->Height = Height;  
    pPIAA->AveX = Totx / nPoints;
    pPIAA->AveY = Toty / nPoints;
    loc = 0; 
    barray = (HPUSHORT)&pPIAA->barray[0];  
    MinMaxInit (&InBounds);
	pPIAA->HaveInPoints = FALSE; 
	if (pPIAA->Compression == 1)
	{
		int		l = ((int)bm.bmWidthBytes * (int)bm.bmHeight); 
		int		rowleninwords = bm.bmWidthBytes / 2;
		HANDLE	hBits = GSSiGlobAlloc (0,GMEM_MOVEABLE,l);
		LPUSHORT	pBitsStart,pBits = (LPUSHORT)GlobalLock (hBits), pNextRow;//(long)pBits - (long)pBitsStart
		ULONG	RowLenLoc, CurValLoc, CurRepeatLoc, nRepeat, RowLen;

		if (hBMOld)    
    		SelectObject(hDC, hBMOld);  
		GetBitmapBits(hBM,l,pBits);
		pBitsStart = pBits;
		pNextRow = pBits + rowleninwords;
		for (irow = 0;irow<Height;irow++,pNextRow+=rowleninwords) 
		{
			RowLenLoc = loc++;
			do
			{
				CurValLoc = loc++;
				CurRepeatLoc = loc++;
				barray[CurValLoc] = *pBits++;
				barray[CurRepeatLoc] = 0;
				while (pBits < pNextRow && barray[CurValLoc] == *pBits)
				{
					barray[CurRepeatLoc]++;
					pBits++;
				}
			}
			while (pBits < pNextRow);
			barray[RowLenLoc] = loc - RowLenLoc;
		}
		GSSiGlobUlFree (&hBits);
	}
	DBoundsInit (&pPIAA->InBounds);
	DPoint = PIAAPointToDPoint (*(LPPOINT)&InBounds.xmn,&pPIAA->Bounds,&pPIAA->Factor,pPIAA->Offset,pPIAA->Type); 
	AddDPointToMinMax (&DPoint,&pPIAA->InBounds);
	DPoint = PIAAPointToDPoint (*(LPPOINT)&InBounds.xmx,&pPIAA->Bounds,&pPIAA->Factor,pPIAA->Offset,pPIAA->Type); 
	AddDPointToMinMax (&DPoint,&pPIAA->InBounds);
	pPIAA->arraylen = loc;
    GlobalUnlock (hPIAA);	
	hPIAA = GSSiGlobalReAlloc (0,hPIAA,sizeof(PIAAStruct)+loc*2,GMEM_MOVEABLE);
Exit:
    GSSiGlobUlFree (&hPoint);  
    if (hDC)
		DeleteDC(hDC);
    if (savebm) 
    	SaveBitmap (hBM,"c:\\test.bmp",0,0);
    GSSiDeleteObject(&hBM);   
	useGDIPlus = saveuseGDIPlus;
{
#if ENABLETRACE
GSSiExitProg (1379);
#endif
    return hPIAA;
}
#if ENABLETRACE
}
#endif
}  

void PCTInAreasDestroy (HANDLE hPIA)
{
	if (hPIA)
	{
		LPPCTIAStruct pPIA = GlobalLock (hPIA);

		GSSiDeleteObject (&pPIA->hBitMap[0]);
		GSSiDeleteObject (&pPIA->hBitMap[1]);
		GSSiGlobUlFree (&hPIA);
	}
    return;
}

HANDLE  PCTInAreasInit (LPMNMXCORD pBounds,int Precision)
#if ENABLETRACE
{GSSiEnterProg (1437);
#endif
{
	double	rtn=0;
    DWORD   i; 
    HDC		hDC,hDCMain;
    HBITMAP	hBM=0, hBMOld=0; 
    double	Factor; 
    DWORD	loc;  
    HANDLE	hPIA=0; 
    long	memsize,ii;
	LPPCTIAStruct pPIA;
	double	Totx=0, Toty=0, dWidth, dHeight;    
	BOOL	savebm=FALSE;  
	DPOINT	DPoint;   
	POINT	PIAAPoint;  
	mnmxCor InBounds;   
	double	FAC=PIASizeFactor;   
	short	Offsetx=3;
    BITMAP	bm;   
	DPOINT	Point[5];
	int		MaxDim[3]={4000,8000,16000};
	
//	if (FAC > 5)
//		ii=1;
		
	if (BoundsWidth (pBounds)<= 0 || BoundsHeight(pBounds)<=0)
		goto Exit;
	Factor = MaxDim[min (2,max (0,Precision))] / max (BoundsWidth (pBounds),BoundsHeight(pBounds));
    hPIA = GSSiGlobAlloc (1723,GHND,sizeof(PCTIAStruct));
    pPIA = (LPPCTIAStruct)GlobalLock (hPIA); 
    pPIA->Type = 0;
    pPIA->Bounds = *pBounds;
	pPIA->Offset = 4;
 	pPIA->Width  = BoundsWidth (pBounds) * Factor;
	pPIA->Width += 8 - (pPIA->Width % 8);
    pPIA->Factor = (double)pPIA->Width / BoundsWidth (pBounds);
	pPIA->Width += 2 * pPIA->Offset;
	pPIA->Height = BoundsHeight(pBounds) * pPIA->Factor  + 2*pPIA->Offset;
 	pPIA->hBitMap[0] = CreateBitmap (pPIA->Width,pPIA->Height,1,1,0);
 	pPIA->hBitMap[1] = CreateBitmap (pPIA->Width,pPIA->Height,1,1,0);
	GlobalUnlock (hPIA);
Exit:
{
#if ENABLETRACE
GSSiExitProg (1437);
#endif
    return hPIA;
}
#if ENABLETRACE
}
#endif
}  
    
double  PCTInAreasLoad (HANDLE hPIA,int opt,int Type,DWORD nPoints, HPDPOINT pAreaPoints, int nPoly,HANDLE hPolyPartLen,double Offset,LPMNMXCORD pBounds)
#if ENABLETRACE
{GSSiEnterProg (1437);
#endif
{
	double	rtn=0;
    DWORD   i; 
    HDC		hDC,hDCMain;
    HBITMAP	hBM=0, hBMOld=0; 
    double	Factor; 
    UINT	Height,Width,irow,icol;
    DWORD	loc;  
    HANDLE	hPoint = GSSiGlobAlloc ( 294,GMEM_MOVEABLE,max(5,(long)nPoints+1) * sizeof(POINT));
    HPPOINT	pPoint = (HPPOINT)GlobalLock(hPoint);
	HBRUSH	OldBrush; 
	HPEN	OldPen, hPen;
    long	memsize,ii;
	LPPCTIAStruct pPIA;
	HPUSHORT	array;   
	double	Totx=0, Toty=0, dWidth, dHeight;    
	BOOL	savebm=FALSE;  
	DPOINT	DPoint;   
	POINT	PIAAPoint;  
	mnmxCor InBounds;   
	double	FAC=PIASizeFactor;   
	short	Offsetx=3;
    BITMAP	bm;   
	double	PenWidth;
	
    pPIA = (LPPCTIAStruct)GlobalLock (hPIA); 
	hDCMain = GetDC (hWndMain);
    hDC = CreateCompatibleDC(hDCMain); 
	ReleaseDC (hWndMain,hDCMain);
    hBMOld = SelectObject(hDC,pPIA->hBitMap[opt]);
	SetMapMode    ( hDC, MM_ISOTROPIC );
    SetWindowOrgEx  ( hDC, 0, 0,0 );
    SetViewportOrgEx( hDC, 0, 0,0 );    
  	SetWindowExtEx  ( hDC, 1024, 1024,0 ); 
	SetViewportExtEx( hDC, 1024, 1024,0 );         
	OldBrush = SelectObject (hDC,GetStockObject(BLACK_BRUSH));  
    OldPen = SelectObject (hDC,GetStockObject(BLACK_PEN)); 
	PenWidth = 1;
    for (i=0;i<nPoints;i++)
    	pPoint[i] = DPointToPIAAPoint (&pAreaPoints[i],&pPIA->Bounds,&pPIA->Factor,pPIA->Offset,0); 
	switch (Type)
	{
	case 2:
		hPen = CreatePen (PS_SOLID,5,RGB(255,255,255));
		OldPen = SelectObject (hDC,hPen);
		Polyline (hDC,pPoint,(int)nPoints);
	    SelectObject (hDC,OldPen);  
		GSSiDeleteObject (&hPen);
		break;
	case 3:
	    pPoint[nPoints] = pPoint[0];  
		hPen = CreatePen (PS_SOLID,(int)IDNINT(PenWidth),RGB(255,255,255));
		OldPen = SelectObject (hDC,hPen);
		OldBrush = SelectObject (hDC,GetStockObject(WHITE_BRUSH));
		Polygon (hDC,pPoint,(int)nPoints+1);
		SelectObject (hDC,OldBrush);
	    SelectObject (hDC,OldPen);  
		GSSiDeleteObject (&hPen);
		break;
	}
    GSSiGlobUlFree (&hPoint);  
	SelectObject(hDC,hBMOld);
    DeleteDC(hDC);
    if (savebm) 
    	SaveBitmap (pPIA->hBitMap[opt],"c:\\test.bmp",0,0);
    GlobalUnlock (hPIA);  
{
#if ENABLETRACE
GSSiExitProg (1437);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}  

double PCTInAreas (HANDLE hPIA)
{
	LPPCTIAStruct pPIA;
	double rtn=0;
    BITMAP	bm;  
	int	lMem;
	HANDLE	hMem1,hMem2;
	WORD	irow, icol;
	LPBYTE	pBits1, pBits2;
	BYTE	MaskedByte;
	int		TotBits=0, TotMaskedBits=0;
	static	BOOL	First=TRUE;

	if (!hPIA)
		return -1;
	if (First)
	{
		BYTE	b=0;
		int		i,j;

		for (i=0;i<256;i++,b++)
		{
			nBitsInByte[i]=0;
			for (j=0;j<8;j++)
				nBitsInByte[i] += GetBit (j,&b);
		}
		First = FALSE;
	}
	pPIA = GlobalLock (hPIA);
    GetObject(pPIA->hBitMap[0], sizeof(bm), (LPSTR)&bm);
	lMem = bm.bmWidthBytes * bm.bmHeight;
	hMem1 = GSSiGlobAlloc (1724,GMEM_MOVEABLE,lMem);
	hMem2 = GSSiGlobAlloc (1724,GMEM_MOVEABLE,lMem);
	pBits1 = GlobalLock (hMem1);
	pBits2 = GlobalLock (hMem2);
	GetBitmapBits (pPIA->hBitMap[0],lMem,pBits1);
	GetBitmapBits (pPIA->hBitMap[1],lMem,pBits2);

	for (irow = 0;irow<bm.bmHeight;irow++) 
	{
		for (icol=0;icol<bm.bmWidthBytes;icol++,pBits1++,pBits2++)
		{
			if (!*pBits1)
				;
			else
			{
				TotBits += nBitsInByte[*pBits1];
				MaskedByte = *pBits1 & *pBits2;
				TotMaskedBits += nBitsInByte[MaskedByte];
			}
		}
	}
	GSSiGlobUlFree (&hMem1);
	GSSiGlobUlFree (&hMem2);
    GlobalUnlock (hPIA);
	
	if (TotBits && TotMaskedBits)
		rtn = (double)TotMaskedBits / (double)TotBits;
	return rtn;
}
    

/*HANDLE  PointInAreaAcceleratorSetupWindow_Color (DWORD nPoints, HPDPOINT pAreaPoints, HANDLE hMaskAccelerator)
#if ENABLETRACE
{GSSiEnterProg (1378);
#endif
{
    DWORD   i; 
    MNMXCORD	Bounds,AreaBounds;        
    HDC		hDC,hDCMain;
    HBITMAP	hBM=0, hBMOld=0; 
    double	Factor, PenWidthFactor; 
    int		Height,Width,irow,icol;
    DWORD	loc;  
    HANDLE	hPIAA=0; 
    HANDLE	hPoint = GSSiGlobAlloc ( 294,GMEM_MOVEABLE,max(5,(long)nPoints+1) * sizeof(POINT));
    HPPOINT	pPoint = (HPPOINT)GlobalLock(hPoint);
	HBRUSH	hRedBrush =   CreateSolidBrush(RGB(255,   0,   0));
    HBRUSH	hGreenBrush = CreateSolidBrush(RGB(  0, 255,   0));
    HBRUSH	hBlueBrush = CreateSolidBrush(RGB(  0, 0, 255));
	HBRUSH	OldBrush; 
	HPEN	OldPen;
	HPEN	hBluePen=0;
	HPEN	hRedPen=0;
    COLORREF	Red, Green, Blue; 
    long	memsize;//,ii;
	LPPIAAStruct pPIAA, pPIAAMask;
	HPBYTE	array;   
	double	Totx=0, Toty=0, dWidth, dHeight;    
	BOOL	savebm=FALSE;  
	DPOINT	DPoint;   
	POINT	PIAAPoint;  
	mnmxCor InBounds;   
	double	FAC=PIASizeFactor;   
	short	Offset=0;
	
//	if (FAC > 5)
//		ii=1;
	if (nPoints < 16)
		FAC /= 10;
	DBoundsInit (&AreaBounds);  
	for (i=0;i<nPoints;i++)
	{
		Totx += pAreaPoints[i].x;
		Toty += pAreaPoints[i].y;
		AddDPointToMinMax (&pAreaPoints[i],&AreaBounds);
	}
	if (hMaskAccelerator)
	{
		pPIAAMask=(LPPIAAStruct)GlobalLock (hMaskAccelerator);
		IntersectBounds (&pPIAAMask->Bounds,&AreaBounds,&Bounds);
		GlobalUnlock (hMaskAccelerator);
		if (AreaBounds.xmn == Bounds.xmn && 
			AreaBounds.xmx == Bounds.xmx &&
			AreaBounds.ymn == Bounds.ymn &&
			AreaBounds.ymx == Bounds.ymx)
			hMaskAccelerator = 0;
	}
	else
		Bounds = AreaBounds;
	if (Bounds.ymx - Bounds.ymn <= 0)
		goto Exit;
	Factor = (Bounds.xmx - Bounds.xmn)/(Bounds.ymx - Bounds.ymn);
	if (Factor <= 0)
		goto Exit;
	dWidth = min (10000,100 * Factor);
	dHeight = dWidth / Factor; 
	Factor = sqrt ((((double)USHRT_MAX*FAC)-2*sizeof(PIAAStruct)) / (dWidth * dHeight)); 
	dHeight *= Factor;
	dWidth *= Factor; 
	Height = dHeight - 1;
	Width  = dWidth - 1;
	Factor = Width / (Bounds.xmx - Bounds.xmn); 
	Height = Factor * (Bounds.ymx - Bounds.ymn);
	hDCMain = GetDC (hWndMain);
    hDC = CreateCompatibleDC(hDCMain); 
    Height *= PIASizeFactor;
    Width *= PIASizeFactor;
    hBM = CreateCompatibleBitmap(hDCMain,Width,Height); 
	ReleaseDC (hWndMain,hDCMain);
    hBMOld = SelectObject(hDC,hBM);
	SetMapMode    ( hDC, MM_ISOTROPIC );
    SetWindowOrgEx  ( hDC, (int)Bounds.xmn, (int)Bounds.ymn,0 );
    SetViewportOrgEx( hDC, 0,0,0 );    
  	SetWindowExtEx  ( hDC, (int)(Bounds.xmx - Bounds.xmn), (int)(Bounds.ymx-Bounds.ymn),0 ); 
	SetViewportExtEx( hDC, Width,Height,0 );     
	PenWidthFactor = (Bounds.xmx - Bounds.xmn) / Width;
	hBluePen = CreatePen(PS_SOLID,(int)IDNINT(PIAWidthFactor*PenWidthFactor),RGB(0,0,255));
	hRedPen = CreatePen(PS_SOLID,(int)IDNINT(5*PenWidthFactor),RGB(255,0,0));
	OldBrush = SelectObject (hDC,hRedBrush);  
    OldPen = SelectObject (hDC,hRedPen);
	pPoint[0].x = Bounds.xmn;
	pPoint[0].y = Bounds.ymn;
	pPoint[1].x = Bounds.xmn;
	pPoint[1].y = Bounds.ymx;
	pPoint[2].x = Bounds.xmx;
	pPoint[2].y = Bounds.ymx;
	pPoint[3].x = Bounds.xmx;
	pPoint[3].y = Bounds.ymn;
	pPoint[4] = pPoint[0];
	Polygon (hDC,pPoint,5); 
	Red = GetPixel (hDC,(int)(Bounds.xmx + Bounds.xmn)/2,(int)(Bounds.ymx+Bounds.ymn)/2); 
	SelectObject (hDC,hGreenBrush);  
	Polygon (hDC,pPoint,5); 
	Green = GetPixel (hDC,(int)(Bounds.xmx + Bounds.xmn)/2,(int)(Bounds.ymx+Bounds.ymn)/2); 
	SelectObject (hDC,hBlueBrush);  
	Polygon (hDC,pPoint,5); 
	Blue = GetPixel (hDC,(int)(Bounds.xmx + Bounds.xmn)/2,(int)(Bounds.ymx+Bounds.ymn)/2);    
	SelectObject (hDC,hRedBrush);
	Polygon (hDC,pPoint,5); 
	Factor = (Width) / (Bounds.xmx - Bounds.xmn);    
	if (Factor <= 0)
		goto Exit;
    for (i=0;i<nPoints;i++)
//    	pPoint[i] = DPointToPIAAPoint (&pAreaPoints[i],&Bounds,&Factor); 
    	pPoint[i] = DPointToPoint (pAreaPoints[i]); 
    pPoint[nPoints] = pPoint[0];  
	SelectObject (hDC,hGreenBrush);    
    SelectObject (hDC,hBluePen);
    Polyline (hDC,pPoint,(int)nPoints+1);
    SelectObject (hDC,GetStockObject (NULL_PEN));
    Polygon (hDC,pPoint,(int)nPoints);
    SelectObject (hDC,OldBrush);
    SelectObject (hDC,OldPen);  
    memsize = (long)sizeof(PIAAStruct)+(long)Width*(long)Height;
    hPIAA = GSSiGlobAlloc (0,GMEM_MOVEABLE,memsize);
    pPIAA = (LPPIAAStruct)GlobalLock (hPIAA);  
    pPIAA->Offset = Offset; 
    pPIAA->Type = 1;
    pPIAA->Factor = Factor;
    pPIAA->Bounds = Bounds;
    pPIAA->Width = Width;
    pPIAA->Height = Height;  
    pPIAA->AveX = Totx / nPoints;
    pPIAA->AveY = Toty / nPoints;
    loc = 0; 
    array = (HPBYTE)&pPIAA->array[0];  
    MinMaxInit (&InBounds);
	pPIAA->HaveInPoints = FALSE; 
    SetWindowOrgEx  ( hDC, 0,0,0 );
    SetViewportOrgEx( hDC, 0,0,0 );    
  	SetWindowExtEx  ( hDC, Width,Height,0 ); 
	SetViewportExtEx( hDC, Width,Height,0 );     
    for (irow = 0;irow<Height;irow++) 
    {
    	PIAAPoint.y = irow;
    	for (icol = 0;icol<Width;icol++)
		{
			COLORREF c;
			WORD	ii=100;
			while (ii--)
			c = GetPixel (hDC,icol,irow);  
    			
	    	PIAAPoint.x = icol;
			if (c == Green)
				array[loc]=1;
			else if (c == Blue)
				array[loc] = 2;
			else
				array[loc] = 0; 
			if (array[loc] && hMaskAccelerator)
			{ 
				DPoint = PIAAPointToDPoint (PIAAPoint,&pPIAA->Bounds,&pPIAA->Factor,pPIAA->Offset,pPIAA->Type); 
				//DPoint = WinPtToBasePtD (&DPoint);
		      	if (PointInAreaAccelerator (&DPoint,hMaskAccelerator) != 1)
					array[loc]=0; 
            } 
            if (array[loc] == 1)  
            {
            	pPIAA->HaveInPoints = TRUE;
            	AddPointToMinMax (PIAAPoint,&InBounds);  
            }
			loc++;
		}
	} 
	DBoundsInit (&pPIAA->InBounds);
	DPoint = PIAAPointToDPoint (POINTStoPOINT(*(LPPOINTS)&InBounds.xmn),&pPIAA->Bounds,&pPIAA->Factor,pPIAA->Offset,pPIAA->Type); 
	AddDPointToMinMax (&DPoint,&pPIAA->InBounds);
	DPoint = PIAAPointToDPoint (POINTStoPOINT(*(LPPOINTS)&InBounds.xmx),&pPIAA->Bounds,&pPIAA->Factor,pPIAA->Offset,pPIAA->Type); 
	AddDPointToMinMax (&DPoint,&pPIAA->InBounds);
	if (hBMOld)    
    	SelectObject(hDC, hBMOld);  
    GlobalUnlock (hPIAA);			
Exit: 
    GSSiGlobUlFree (&hPoint);  
    DeleteDC(hDC);
    GSSiDeleteObject (&hRedBrush);
    GSSiDeleteObject (&hGreenBrush); 
    GSSiDeleteObject (&hBlueBrush); 
    GSSiDeleteObject (&hBluePen); 
    GSSiDeleteObject (&hRedPen);
    if (savebm) 
    	SaveBitmap (hBM,"c:\\test.bmp",0,0);
    GSSiDeleteObject(&hBM);   
{
#if ENABLETRACE
GSSiExitProg (1378);
#endif
    return hPIAA;
}
#if ENABLETRACE
}
#endif
}  
*/    
HANDLE  PointInAreaAcceleratorSetupWindow (DWORD nPoints, HPDPOINT pAreaPoints, HANDLE hMaskAccelerator)
#if ENABLETRACE
{GSSiEnterProg (1378);
#endif
{
    DWORD   i,ii; 
    MNMXCORD	Bounds,AreaBounds;        
    HDC		hDC=0,hDCMain;
    HBITMAP	hBM=0, hBMOld=0; 
    double	Factor, PenWidthFactor; 
    DWORD	Height,Width,irow,icol;
    DWORD	loc;  
    HANDLE	hPIAA=0; 
    HANDLE	hPoint = GSSiGlobAlloc ( 294,GMEM_MOVEABLE,max(5,(long)nPoints+1) * sizeof(POINT));
    HPPOINT	pPoint = (HPPOINT)GlobalLock(hPoint);
	HBRUSH	hRedBrush =   CreateSolidBrush(RGB(255,   0,   0));
    HBRUSH	hGreenBrush = CreateSolidBrush(RGB(  0, 255,   0));
    HBRUSH	hBlueBrush = CreateSolidBrush(RGB(  0, 0, 255));
	HBRUSH	OldBrush; 
	HPEN	OldPen;
	HPEN	hBluePen=0;
	HPEN	hRedPen=0;
    COLORREF	Red, Green, Blue; 
	//RGBTRIPLE	Red3, Green3, Blue3;
    long	memsize;//,ii;
	LPPIAAStruct pPIAA, pPIAAMask;
	HPBYTE	barray;   
	double	Totx=0, Toty=0, dWidth, dHeight;    
	BOOL	savebm=FALSE;  
	DPOINT	DPoint;   
	POINT	PIAAPoint;  
	mnmxCor InBounds;   
	double	FAC=PIASizeFactor;   
	short	Offset=0;
    BITMAP	bm;   
    WORD	biBits; 
	LPBYTE	pBits;
	HANDLE	hBits=0;
	LPCOLORREF16	pColor16;
	DWORD	MidPixel;
	BOOL	saveuseGDIPlus = useGDIPlus;
	HWND	hWnd = hWndMain;
	
	useGDIPlus = FALSE;
//	if (FAC > 5)
//		ii=1;
	if (nPoints < 16)
		FAC /= 10;
	DBoundsInit (&AreaBounds);  
	for (i=0;i<nPoints;i++)
	{
		Totx += pAreaPoints[i].x;
		Toty += pAreaPoints[i].y;
		AddDPointToMinMax (&pAreaPoints[i],&AreaBounds);
	}
	if (hMaskAccelerator)
	{
		pPIAAMask=(LPPIAAStruct)GlobalLock (hMaskAccelerator);
		IntersectBounds (&pPIAAMask->Bounds,&AreaBounds,&Bounds);
		GlobalUnlock (hMaskAccelerator);
		if (AreaBounds.xmn == Bounds.xmn && 
			AreaBounds.xmx == Bounds.xmx &&
			AreaBounds.ymn == Bounds.ymn &&
			AreaBounds.ymx == Bounds.ymx)
			hMaskAccelerator = 0;
	}
	else
		Bounds = AreaBounds;
	if (Bounds.ymx - Bounds.ymn <= 0)
		goto Exit;
	Factor = (Bounds.xmx - Bounds.xmn)/(Bounds.ymx - Bounds.ymn);
	if (Factor <= 0)
		goto Exit;
	dWidth = min (10000,100 * Factor);
	dHeight = dWidth / Factor; 
	Factor = sqrt ((((double)USHRT_MAX*FAC)-2*sizeof(PIAAStruct)) / (dWidth * dHeight)); 
	dHeight *= Factor;
	dWidth *= Factor; 
	Height = dHeight - 1;
	Width  = dWidth - 1;
	Factor = Width / (Bounds.xmx - Bounds.xmn); 
	Height = Factor * (Bounds.ymx - Bounds.ymn);
	hDCMain = GetDC (hWnd);
    hDC = CreateCompatibleDC(hDCMain); 

	if (!hDC)
	{
		ReleaseDC(hWnd, hDCMain);
		goto Exit;
	}

    Height *= PIASizeFactor;
    Width *= PIASizeFactor;
	if (Height < 5 || Width < 5)
		goto Exit;
    hBM = CreateCompatibleBitmap(hDCMain,Width,Height); 
    GetObject(hBM, sizeof(bm), (LPSTR)&bm);
    biBits = bm.bmPlanes * bm.bmBitsPixel;
	hBits = GSSiGlobAlloc (0,GMEM_MOVEABLE,bm.bmWidthBytes * bm.bmHeight);
	pBits = GlobalLock (hBits);
//    hBM = CreateCompatibleBitmap(hDC,Width,Height); 
	ReleaseDC (hWndMain,hDCMain);
    hBMOld = SelectObject(hDC,hBM);
	SetMapMode    ( hDC, MM_ISOTROPIC );
    SetWindowOrgEx  ( hDC, (int)Bounds.xmn, (int)Bounds.ymn,0 );
    SetViewportOrgEx( hDC, 0,0,0 );    
  	SetWindowExtEx  ( hDC, (int)(Bounds.xmx - Bounds.xmn), (int)(Bounds.ymx-Bounds.ymn),0 ); 
	SetViewportExtEx( hDC, Width,Height,0 );     
	PenWidthFactor = (Bounds.xmx - Bounds.xmn) / Width;
	hBluePen = CreatePen(PS_SOLID,(int)IDNINT(PIAWidthFactor*PenWidthFactor),RGB(0,0,255));
	hRedPen = CreatePen(PS_SOLID,(int)IDNINT(5*PenWidthFactor),RGB(255,0,0));
	OldBrush = SelectObject (hDC,hRedBrush);  
    OldPen = SelectObject (hDC,hRedPen);
	pPoint[0].x = Bounds.xmn;
	pPoint[0].y = Bounds.ymn;
	pPoint[1].x = Bounds.xmn;
	pPoint[1].y = Bounds.ymx;
	pPoint[2].x = Bounds.xmx;
	pPoint[2].y = Bounds.ymx;
	pPoint[3].x = Bounds.xmx;
	pPoint[3].y = Bounds.ymn;
	pPoint[4] = pPoint[0];
	Polygon (hDC,pPoint,5); 
	MidPixel = bm.bmHeight/2 * bm.bmWidth + bm.bmWidth/2;
	if (biBits == 16)
	{
		GetBitmapBits(hBM,bm.bmWidthBytes * bm.bmHeight,pBits);
		pColor16 = (LPCOLORREF16) pBits;
		Red = *(pColor16 + MidPixel);
	}
	else
		Red = GetPixel (hDC,(int)(Bounds.xmx + Bounds.xmn)/2,(int)(Bounds.ymx+Bounds.ymn)/2); 
//	Red3 = COLORREFtoRGBTRIPLE (Red);
	SelectObject (hDC,hGreenBrush);  
	Polygon (hDC,pPoint,5); 
	if (biBits == 16)
	{
		GetBitmapBits(hBM,bm.bmWidthBytes * bm.bmHeight,pBits);
		pColor16 = (LPCOLORREF16) pBits;
		Green = *(pColor16 + MidPixel);
	}
	else
		Green = GetPixel (hDC,(int)(Bounds.xmx + Bounds.xmn)/2,(int)(Bounds.ymx+Bounds.ymn)/2); 
	//Green3 = COLORREFtoRGBTRIPLE (Green);
	SelectObject (hDC,hBlueBrush);  
	Polygon (hDC,pPoint,5); 
	if (biBits == 16)
	{
		GetBitmapBits(hBM,bm.bmWidthBytes * bm.bmHeight,pBits);
		pColor16 = (LPCOLORREF16) pBits;
		Blue = *(pColor16 + MidPixel);
	}
	else
		Blue = GetPixel (hDC,(int)(Bounds.xmx + Bounds.xmn)/2,(int)(Bounds.ymx+Bounds.ymn)/2);  
	//Blue3 = COLORREFtoRGBTRIPLE (Blue);
	SelectObject (hDC,hRedBrush);
	Polygon (hDC,pPoint,5); 
	Factor = (Width) / (Bounds.xmx - Bounds.xmn);    
	if (Factor <= 0)
		goto Exit;
    for (i=0;i<nPoints;i++)
//    	pPoint[i] = DPointToPIAAPoint (&pAreaPoints[i],&Bounds,&Factor); 
    	pPoint[i] = DPointToPoint (pAreaPoints[i]); 
    pPoint[nPoints] = pPoint[0];  
	SelectObject (hDC,hGreenBrush);    
    SelectObject (hDC,hBluePen);
    Polyline (hDC,pPoint,(int)nPoints+1);
    SelectObject (hDC,GetStockObject (NULL_PEN));
    Polygon (hDC,pPoint,(int)nPoints);
    SelectObject (hDC,OldBrush);
    SelectObject (hDC,OldPen);  
    memsize = (long)sizeof(PIAAStruct)+(long)Width*(long)Height;
/*    if (memsize > UINT_MAX) 
    {   
    	char	str[256];
    	sprintf (str,"memsize too big:%ld",memsize);
    	MessageBox (0,str,0,MB_OK);  
    } */
    hPIAA = GSSiGlobAlloc (0,GMEM_MOVEABLE,memsize);
    pPIAA = (LPPIAAStruct)GlobalLock (hPIAA);  
    pPIAA->Offset = Offset; 
    pPIAA->Type = 1;
    pPIAA->Compression = 0;
    pPIAA->Factor = Factor;
    pPIAA->Bounds = Bounds;
    pPIAA->Width = Width;
    pPIAA->Height = Height;  
    pPIAA->AveX = Totx / nPoints;
    pPIAA->AveY = Toty / nPoints;
	pPIAA->arraylen = (long)Width*(long)Height;
    loc = 0; 
    barray = (HPBYTE)&pPIAA->barray[0];  
    MinMaxInit (&InBounds);
	pPIAA->HaveInPoints = FALSE; 
    SetWindowOrgEx  ( hDC, 0,0,0 );
    SetViewportOrgEx( hDC, 0,0,0 );    
  	SetWindowExtEx  ( hDC, Width,Height,0 ); 
	SetViewportExtEx( hDC, Width,Height,0 );     
	hBM = SelectObject(hDC, hBMOld);  
   
	ii=GetBitmapBits(hBM,bm.bmWidthBytes * bm.bmHeight,pBits);
	if (biBits == 16)
    for (irow = 0;irow<Height;irow++) 
    {
		pColor16 = (LPCOLORREF16) (pBits+irow*bm.bmWidthBytes);

    	PIAAPoint.y = irow;
    	for (icol = 0;icol<Width;icol++,pColor16++)
		{

			COLORREF16 c = *pColor16;
			
   			
	    	PIAAPoint.x = icol;
			if (c == Green)
				barray[loc]=1;
			else if (c == Blue)
				barray[loc] = 2;
			else
				barray[loc] = 0; 
			if (barray[loc] && hMaskAccelerator)
			{ 
				DPoint = PIAAPointToDPoint (PIAAPoint,&pPIAA->Bounds,&pPIAA->Factor,pPIAA->Offset,pPIAA->Type); 
		      	if (PointInAreaAccelerator (&DPoint,hMaskAccelerator) != 1)
					barray[loc]=0; 
            } 
            if (barray[loc] == 1)  
            {
            	pPIAA->HaveInPoints = TRUE;
            	AddPointToMinMax (PIAAPoint,&InBounds);  
            }
			loc++;
		}
	} 
	else
    for (irow = 0;irow<Height;irow++) 
    {
		LPCOLORREF	pColor = (LPCOLORREF) (pBits+irow*bm.bmWidthBytes);

    	PIAAPoint.y = irow;
    	for (icol = 0;icol<Width;icol++,pColor++)
		{

			COLORREF c = *pColor;
			
   			
	    	PIAAPoint.x = icol;
			if (c == Green)
				barray[loc]=1;
			else if (c == Blue)
				barray[loc] = 2;
			else
				barray[loc] = 0; 
			if (barray[loc] && hMaskAccelerator)
			{ 
				DPoint = PIAAPointToDPoint (PIAAPoint,&pPIAA->Bounds,&pPIAA->Factor,pPIAA->Offset,pPIAA->Type); 
		      	if (PointInAreaAccelerator (&DPoint,hMaskAccelerator) != 1)
					barray[loc]=0; 
            } 
            if (barray[loc] == 1)  
            {
            	pPIAA->HaveInPoints = TRUE;
            	AddPointToMinMax (PIAAPoint,&InBounds);  
            }
			loc++;
		}
	} 
	DBoundsInit (&pPIAA->InBounds);
	DPoint = PIAAPointToDPoint (POINTStoPOINT(*(LPPOINTS)&InBounds.xmn),&pPIAA->Bounds,&pPIAA->Factor,pPIAA->Offset,pPIAA->Type); 
	AddDPointToMinMax (&DPoint,&pPIAA->InBounds);
	DPoint = PIAAPointToDPoint (POINTStoPOINT(*(LPPOINTS)&InBounds.xmx),&pPIAA->Bounds,&pPIAA->Factor,pPIAA->Offset,pPIAA->Type); 
	AddDPointToMinMax (&DPoint,&pPIAA->InBounds);
    GlobalUnlock (hPIAA);			
Exit: 
    GSSiGlobUlFree (&hPoint);  
    if (hDC)
		DeleteDC(hDC);
    GSSiDeleteObject (&hRedBrush);
    GSSiDeleteObject (&hGreenBrush); 
    GSSiDeleteObject (&hBlueBrush); 
    GSSiDeleteObject (&hBluePen); 
    GSSiDeleteObject (&hRedPen);
	GSSiGlobUlFree (&hBits);
    if (savebm) 
    	SaveBitmap (hBM,"c:\\test.bmp",0,0);
    GSSiDeleteObject(&hBM);   
	useGDIPlus = saveuseGDIPlus;
{
#if ENABLETRACE
GSSiExitProg (1378);
#endif
    return hPIAA;
}
#if ENABLETRACE
}
#endif
}  
    
BOOL  POINT_IN_AREA (POINT PickPoint, DWORD nPoints, HPPOINT lpAreaPoints)
#if ENABLETRACE
{GSSiEnterProg (358);
#endif
{
    BOOL    rtn;
    HANDLE  hPoint;
    HPDPOINT  pDPoint; 
    DPOINT  PickPointD;
    DWORD   i;
     
    PickPointD.x = PickPoint.x;
    PickPointD.y = PickPoint.y; 
    hPoint = GSSiGlobAlloc ( 294,GMEM_MOVEABLE,(long)nPoints * sizeof(DPOINT));
    pDPoint = (HPDPOINT)GlobalLock(hPoint);
    i=nPoints;
    while (i--)
    {
        pDPoint->x = lpAreaPoints->x;
        pDPoint++->y = lpAreaPoints++->y;
    }
    GlobalUnlock (hPoint);
    pDPoint = (HPDPOINT)GlobalLock(hPoint);
    rtn = POINT_IN_AREAD (PickPointD,nPoints,pDPoint,1,0,0,0);
    GSSiGlobUlFree (&hPoint);
{
#if ENABLETRACE
GSSiExitProg (358);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL  POINT_IN_AREAS (POINT PickPoint, DWORD nPoints, HPPOINTS lpAreaPoints)
#if ENABLETRACE
{GSSiEnterProg (358);
#endif
{
    BOOL    rtn;
    HANDLE  hPoint;
    HPDPOINT  pDPoint; 
    DPOINT  PickPointD;
    DWORD   i;
     
    PickPointD.x = PickPoint.x;
    PickPointD.y = PickPoint.y; 
    hPoint = GSSiGlobAlloc ( 294,GMEM_MOVEABLE,(long)nPoints * sizeof(DPOINT));
    pDPoint = (HPDPOINT)GlobalLock(hPoint);
    i=nPoints;
    while (i--)
    {
        pDPoint->x = lpAreaPoints->x;
        pDPoint++->y = lpAreaPoints++->y;
    }
    GlobalUnlock (hPoint);
    pDPoint = (HPDPOINT)GlobalLock(hPoint);
    rtn = POINT_IN_AREAD (PickPointD,nPoints,pDPoint,1,0,0,0);
    GSSiGlobUlFree (&hPoint);
{
#if ENABLETRACE
GSSiExitProg (358);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}
BOOL  POINT_IN_AREAD (DPOINT PickPoint, DWORD nPoints, HPDPOINT lpAreaPoints,int nPoly,HANDLE hPolyPartLen,LPDOUBLE pNextXIntersect,LPHANDLE pAccelerator)
#if ENABLETRACE
{GSSiEnterProg (359);
#endif
{
/*C******* SPECIFICATIONS ***********************************************
C*                                                                    *
C*       PROGRAM SUMMARY                                              *
C*       ------- -------                                              *
C*       THIS ROUTINE DETERMINES if THE POINT SEGPOS IS IN THE        *
C*       AREA DEFINED IN THE PICKED AREA COMMON.                      *
C*                                                                    *
C*       ARGUMENT DESCRIPTION                                         *
C*       -------- -----------                                         *
C*       SEGPOS   R*4[1] THE SEGMENT COORDINATES OF THE POINT         *
C*                                                                    *
C*       AUTHOR: JEFF SMITH                                           *
C*                                                                    *
C**********************************************************************
C*/
#if WIN32
    DPOINT  *lpPoint;
#else
    HPDPOINT lpPoint;
#endif

    double   A1, A2, INTX, INTY, XP, YP, XMID1, XMID2, YMID1, YMID2, AMID,
             PAX1, PAY1, PAX2, PAY2, MXX, MNY, MXY,
             XMIDMN, XMIDMX, YMIDMN, YMIDMX, XMID2_NEED, YMID2_NEED;

    BOOL HAVE_MID1, NEED_MID1, p_in_a;

    long  IRC ;
    DWORD	ip;
    short NIN,ii; 
	LPPIAAStruct pPIAA;
      
      if (pAccelerator)
      { 
      	int	iaccel;
      	
      	if (!*pAccelerator)
      		*pAccelerator = PointInAreaAcceleratorSetup (nPoints,lpAreaPoints,nPoly,hPolyPartLen,0);
      	if (*pAccelerator) 
      	{
	      	iaccel = PointInAreaAccelerator (&PickPoint,*pAccelerator);
	      	if (iaccel == 0)         
	      	{
	      		counts[0]++;
	      		goto RtnFalse;  
	      	}
	      	else if (iaccel == 1)   
	      	{
	      		counts[1]++;
	      		goto RtnTrue;
	      	}
	     }
	     else
	     	ii=1;
      } 
      counts[2]++;
      if (pNextXIntersect)
      	*pNextXIntersect = DBL_MAX;
      A2 =  (double) 0;
      p_in_a = FALSE;

      XP = PickPoint.x;
      YP = PickPoint.y;

      HAVE_MID1 = FALSE;
      NEED_MID1 = FALSE;
      NIN = 0;
      
      if (pAccelerator && *pAccelerator)
      {
		pPIAA = (LPPIAAStruct)GlobalLock (*pAccelerator); 
		GlobalUnlock (*pAccelerator);
      }
      lpPoint = lpAreaPoints + (nPoints-1);
      PAX1 = lpPoint->x;
      PAY1 = lpPoint->y;
      lpPoint = lpAreaPoints;
      for (ip = 0; ip <nPoints; ip++,lpPoint++)
      {
/*C******* LOOK ONLY FOR INTERSECTIONS WITH X GE XP  */
          PAX2 = lpPoint->x;
          PAY2 = lpPoint->y;
          MXX = max (PAX1,PAX2);
          if (XP > MXX+P_TOL) goto S100;
          MNY = min (PAY1,PAY2);
          if (YP < MNY-P_TOL) goto S100;
          MXY = max (PAY1,PAY2);
          if (YP > MXY+P_TOL) goto S100;
          A1 = LGETAZ (PAX1,PAY1,PAX2,PAY2);
          IRC = LIN_SEC (PAX1,PAY1,A1, XP, YP, A2, &INTX, &INTY);
/*C******* IGNORE PARALLEL LINES */
          if (IRC != 0)  goto S100;
/*C******* if POINT ON AREA SIDE IT IS IN THE AREA */
          if (LDIST(INTX,INTY,XP,YP) <= P_TOL)  
          {
          if (pNextXIntersect)
          	*pNextXIntersect = INTX;
			goto RtnTrue;
		  }
          if (INTX+P_TOL < XP)  goto S100;
          if (pNextXIntersect)
          	*pNextXIntersect = min (*pNextXIntersect,INTX);
          NIN++;
/*             if intersection within p_tol of the begin point of this line
C              find the midpoint of the last non-parallel line and intersect
C              the horizontal line with the line between the found midpoint
C              and the midpoint of this line. if the intersection
C              is not between the midpoints keep this intersection. */
          if (LDIST(INTX,INTY,PAX1,PAY1) <= P_TOL)
          {
               if (HAVE_MID1)
	           {  XMID2  = (PAX1 + PAX2) / (double) 2;
	              YMID2  = (PAY1 + PAY2) / (double) 2;
	              XMIDMN = min (XMID1, XMID2);
	              XMIDMX = max (XMID1, XMID2);
	              YMIDMN = min (YMID1, YMID2);
	              YMIDMX = max (YMID1, YMID2);
	              AMID   = LGETAZ (XMID1,YMID1,XMID2,YMID2);
	              IRC = LIN_SEC (XMID1,YMID1,AMID,XP,YP,A2,&INTX,&INTY);
	              if (IRC == 0 && (INTX >= XMIDMN &&
	                  INTX <= XMIDMX)&& (INTY >= YMIDMN &&
	                  INTY <= YMIDMX))
	              { NIN--; }  
	           }
	           else
               {   
               	  NEED_MID1 = TRUE;
                  XMID2_NEED = (PAX1 + PAX2) / (double) 2 ;
                  YMID2_NEED = (PAY1 + PAY2) / (double) 2 ;
               }
          }
          if (fabs (PAY1 - PAY2) > P_TOL)
          {
	          HAVE_MID1 = TRUE;
	          XMID1 = (PAX1 + PAX2) / (double) 2 ;
	          YMID1 = (PAY1 + PAY2) / (double) 2 ; 
	      }
S100:     PAX1 = PAX2;
          PAY1 = PAY2;
      } /* end of the for loop */



      if (NEED_MID1 && HAVE_MID1)
      {   XMID2  = XMID2_NEED;
          YMID2  = YMID2_NEED;
          XMIDMN = min (XMID1, XMID2);
          XMIDMX = max (XMID1, XMID2);
          YMIDMN = min (YMID1, YMID2);
          YMIDMX = max (YMID1, YMID2) ;
          AMID  = LGETAZ (XMID1,YMID1,XMID2,YMID2);
          IRC = LIN_SEC (XMID1,YMID1,AMID,XP,YP,A2,&INTX,&INTY);

          if (IRC == 0 &&
             (INTX >= XMIDMN && INTX <= XMIDMX)&&
             (INTY >= YMIDMN && INTY <= YMIDMX))
             { NIN--;}
      }
      if (NIN % 2  == 1){ p_in_a = TRUE;}
{
#if ENABLETRACE
GSSiExitProg (359);
#endif
      return(p_in_a);
}
RtnTrue:
{
#if ENABLETRACE
GSSiExitProg (359);
#endif
      return(TRUE);
}
RtnFalse:
{
#if ENABLETRACE
GSSiExitProg (359);
#endif
      return(FALSE);
}
#if ENABLETRACE
}
#endif
}

BOOL LININT (double L1X1, double L1Y1, double L1X2, double L1Y2,
			 double L2X1, double L2Y1, double L2X2, double L2Y2)
#if ENABLETRACE
{GSSiEnterProg (360);
#endif
{
      double SLOPE1, SLOPE2, YINT1, YINT2, X, Y, XDIFF, YDIFF;
      BOOL INF1, INF2;
      
      if ( (L1X1 ==L2X1 && L1Y1 ==L2Y1) ||
           (L1X1 ==L2X2 && L1Y1 ==L2Y2) ||
           (L1X2 ==L2X1 && L1Y2 ==L2Y1) ||
           (L1X2 ==L2X2 && L1Y2 ==L2Y2) ) goto S550;
      XDIFF  = L1X1 - L1X2;
      YDIFF  = L1Y1 - L1Y2;
      if (fabs (XDIFF) <1e-20) goto S10;
      SLOPE1 = YDIFF / XDIFF;
      YINT1  = L1Y1 - SLOPE1 * L1X1;
      INF1   = FALSE;
      goto S20;
 S10: INF1   = TRUE;
      YINT1  = L1Y1;
 S20: XDIFF  = L2X1 - L2X2;
      YDIFF  = L2Y1 - L2Y2;
      if (fabs (XDIFF) <1e-20) goto S30;
      SLOPE2 = YDIFF / XDIFF;
      YINT2  = L2Y1 - SLOPE2 * L2X1;
      INF2   = FALSE;
      goto S50;
 S30: INF2   = TRUE;
      YINT2  = L2Y1;
 S50: if (!INF1) goto S60;
      if (!INF2) goto S100;
      goto S250;
 S60: if (!INF2) goto S300;
      goto S200;
//******* SLOPE1 IS INFINITE - SLOPE2 IS NOT
S100: X      = L1X1;
      Y      = SLOPE2 * X + YINT2;
      goto S500;
//******* SLOPE2 IS INFINITE - SLOPE1 IS NOT
S200: X      = L2X1;
      Y      = SLOPE1 * X + YINT1;
      goto S500;
//******* BOTH SLOPES ARE INFINITE
S250: if (L1X1 ==L2X1) goto S510;
      goto S550;
//****** NIETHER SLOPE IS INFINITE
S300: if (SLOPE1 !=SLOPE2) goto S310;
      if (YINT1  !=YINT2)  goto S550;
      goto S510;
S310: X      = (YINT2 - YINT1) / (SLOPE1 - SLOPE2);
      Y      = SLOPE1 * X + YINT1;
//****** DETERMINE IF THE POINT X,Y IS ON BOTH LINES
S500: if (X <min (L1X1,L1X2) || X > max (L1X1,L1X2) ||
          X <min (L2X1,L2X2) || X > max (L2X1,L2X2) ||
          Y <min (L1Y1,L1Y2) || Y > max (L1Y1,L1Y2) ||
          Y <min (L2Y1,L2Y2) || Y > max (L2Y1,L2Y2))
          goto S550;
S510:
{
#if ENABLETRACE
GSSiExitProg (360);
#endif
	return TRUE;
}
S550:
{
#if ENABLETRACE
GSSiExitProg (360);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}

short LIN_SEC (double X1IN,double Y1IN,double A1,double X2IN,double Y2IN,
             double A2,double *X3,double *Y3)
#if ENABLETRACE
{GSSiEnterProg (361);
#endif
 {  short   irc;
    // retrns 0 if successful, 1 if lines parallel or 2 if cooincident
    irc = LINSEC_TOL (X1IN, Y1IN, A1, X2IN, Y2IN, A2, X3, Y3, P_TOL);
    if (irc == 1)
{
#if ENABLETRACE
GSSiExitProg (361);
#endif
    	return(0);
}
    if (!irc)
{
#if ENABLETRACE
GSSiExitProg (361);
#endif
    	return(2);
}
{
#if ENABLETRACE
GSSiExitProg (361);
#endif
    return(1);
}
#if ENABLETRACE
}
#endif
 }

short LINSEC (double X1IN,double Y1IN,double A1,double X2IN,double Y2IN,
            double A2,double *X3,double *Y3)
#if ENABLETRACE
{GSSiEnterProg (362);
#endif
 {
/*C******* SPECIFICATIONS ***********************************************
C*                                                                    *
C*       PROGRAM SUMMARY                                              *
C*       ------- -------                                              *
C*       SUBROUTINE LINSEC (LINE INTERSECT) DETERMINES THE            *
C*       COORDINATES OF THE INTERSECTION BETWEEN 2 LINES WHEN         *
C*       GIVEN A STARTING POINT AND AZIMUTH. IT COMPUTES ANY          *
C*       INTERSECTION, BE IT ON EITHER THE FORWARD OR BACK            *
C*       AZIMUTH.                                                     *
C*                                                                    *
C*       POSSIBLE WARNINGS OR ERRORS:                                 *
C*                                                                    *
C*       RC = 0 - GIVEN COORDINATES ARE EQUAL                         *
C*       RC = 1 - COMPUTATION WAS SUCCESSFUL                          *
C*       RC = 2 - COULD NOT COMPUTE INTERSECTION BECAUSE LINES        *
C*                ARE PARALLEL                                        *
C*                                                                    *
C*       AUTHOR: LARRY PRATT                                          *
C*                                                                    *
C*       ARGUMENT DESCRIPTION                                         *
C*       -------- -----------                                         *
C*       A1      AZIMUTH OF THE FIRST LINE                            *
C*       A2      AZIMUTH OF THE SECOND LINE                           *
C*       RC      RETRN CODE                                          *
C*       X1      X-COORDINATE ALONG THE FIRST LINE                    *
C*       X2      X-COORDINATE ALONG THE SECOND LINE                   *
C*       X3      X-COORDINATE THE INTERSECTION POINT                  *
C*       Y1      Y-COORDINATE ALONG THE FIRST LINE                    *
C*       Y2      Y-COORDINATE ALONG THE SECOND LINE                   *
C*       Y3      Y-COORDINATE THE INTERSECTION POINT                  *
C*                                                                    *
C*   89/12/30 - JS - offset x and y coords by x1,y1 to reduce error   *
C*                                                                    *
C**********************************************************************
*/

      double  TOL ;
      TOL = P_TOL;
{
#if ENABLETRACE
GSSiExitProg (362);
#endif
      return (LINSEC_TOL (X1IN, Y1IN, A1, X2IN, Y2IN, A2, X3, Y3, TOL));
}

#if ENABLETRACE
}
#endif
}

short LINSEC_TOL (double X1IN,double Y1IN,double A1,double X2IN,double Y2IN,
      double A2,double *X3,double *Y3,double INTOL)
#if ENABLETRACE
{GSSiEnterProg (363);
#endif
{

    double X2, Y2, Z1, Z2, DM1, DM2, TOL, ADIFF,  TAZ;
    short       RC;

      if (INTOL == 0.0) { TOL = 1e-4;}
      else { TOL = INTOL;}

      X2 = X2IN - X1IN; /* delta_x*/
      Y2 = Y2IN - Y1IN; /*  delta_y */
      RC = 1;
      Z1 = fabs(cos(A1)); /*   cosine of line in radians  */
      Z2 = fabs(cos(A2));
      ADIFF = fabs (A1-A2);
      if (ADIFF < (TOL * 1e-3)) {goto S10;}/*  parallel  check for coincident */
      if (fabs (PY-ADIFF) < (TOL * 1e-3)) {goto S10;} /*parallel  check for coincident*/
      if (min(Z1, Z2) < (TOL * 1e-3)) {goto S30;} /*going straight up */
      DM1 = tan(A1);  /*tangent of line in radians */
      DM2 = tan(A2);
      if (fabs(DM1-DM2) < (TOL * 1e-3))
       {goto S10;} /*parallel  check for coincident */

    /*gets here if lines aren't parallel or going straight up */
      *X3 = -(-Y2+DM2*X2)/(DM1-DM2);
      *Y3 = DM1* *X3;
      goto S1000;

/*c*    gets here if the lines are virtually parallel*/
S10:   *X3 = 0;
       *Y3 = 0;
       RC = 2; /*lines parallel and NOT coincident */
       if (fabs(Z1+Z2) < (TOL * 1e-3)) 
       	{goto S50;} /*going straight up */
       RC = 0; /*lines parallel and coincident*/
       TAZ = LDIST(X1IN, Y1IN, X2IN, Y2IN);
       if (TAZ <= TOL)
       	{goto S1000;} /*lines start the same place*/
       TAZ= LGETAZ(X1IN, Y1IN, X2IN, Y2IN);
       if(fabs(TAZ-A1) <= (TOL * 1e-3)){
             *X3 = X2IN;
             *Y3 = Y2IN;}
       else
         {if(fabs(TAZ-LTWOPI(A1+PY)) <= (TOL * 1e-3))
            { *X3 = X1IN;
              *Y3 = Y1IN;}
          else 
          	{ RC = 2;}
         }
      goto Exit;

/*c*    here it handles lines going straight up*/
S30:  if (Z2 < (TOL * 1e-3)) {goto S40;}
      *X3 = 0;
      *Y3 = tan(A2)*(*X3-X2)+Y2;
      goto S1000;
S40:  if (Z1 < (TOL * 1e-3)){ goto S50;}
      *X3 = X2;
      *Y3 = tan(A1)* *X3;
      goto S1000;
S50:  if (fabs(X2) <= (TOL * 1e-3))
		 {RC = 0;}
S1000:*X3 = *X3 + X1IN ;
      *Y3 = *Y3 + Y1IN ;
Exit:
{
#if ENABLETRACE
GSSiExitProg (363);
#endif
      return(RC);
}
#if ENABLETRACE
}
#endif
 } 
 
void AreaFromLineAndOffset (LPPOINT Line,double Offset,LPPOINT AreaPoints)  
{   
	double	AZ=getaz (Line[0],Line[1]);
	
	AreaPoints[0] = newpt (Line[0],AZ+HALFPI,Offset);
	AreaPoints[1] = newpt (Line[1],AZ+HALFPI,Offset);
	AreaPoints[2] = newpt (Line[1],AZ+HALFPI,-Offset);
	AreaPoints[3] = newpt (Line[0],AZ+HALFPI,-Offset);
	return;
}
 
BOOL GetPerpendicularOffsetToPoly (LPDPOINT Point,long nPolyPoints,HPDPOINT PolyPoints,LPDPOINT IntPoint,LPDOUBLE OffDist,LPDOUBLE PolyDist,double LastPolyDist)
{
	ULONG	i;    
	BOOL	HavePoint=FALSE;
	DPOINT	NewIntPoint;
	double	dist, distalongpoly=0, polyDist;
	
	for (i=0;i<nPolyPoints-1;i++)
	{
		if (GetPerpendicularIntersect (Point,&PolyPoints[i],&NewIntPoint,TRUE))
		{  
			polyDist = distalongpoly + ldistpp (&PolyPoints[i],&NewIntPoint);
			dist = ldistpp (&NewIntPoint,Point);
			if (polyDist >= LastPolyDist)
			{
				if (HavePoint)
				{
					if (dist < *OffDist)
					{
						*IntPoint = NewIntPoint;
						*OffDist = dist; 
						*PolyDist = polyDist;
					}
				}
				else
				{
					HavePoint = TRUE;
					*IntPoint = NewIntPoint;   
					*OffDist = dist;
					*PolyDist = polyDist;
				}
			}
		}
		distalongpoly += ldistpp (&PolyPoints[i],&PolyPoints[i+1]);
	}
	return HavePoint;
}

BOOL GetPerpendicularIntersect (LPDPOINT Point,LPDPOINT Line,LPDPOINT IntPoint,BOOL IncludeDistToEPs)
#if ENABLETRACE
{GSSiEnterProg (364);
#endif
{
	double	AZ1, AZ2, LineLen, dtobp, dtoep;
	BOOL	rtn=FALSE;

	AZ1 = getazd (&Line[0],&Line[1]); 
	AZ2 = LTWOPI (AZ1 + HALFPI);
    LineLen = ldistp (Line[0],Line[1]);
	if (LINSEC (Line[0].x,Line[0].y,AZ1,Point->x,Point->y,AZ2,&IntPoint->x,&IntPoint->y) != 1)
		goto Exit;
	
	dtobp = ldistp (*IntPoint,Line[0]);
	dtoep = ldistp (*IntPoint,Line[1]);
	if (dtobp > LineLen || dtoep > LineLen)
	{
		if (IncludeDistToEPs)
		{
			if (dtobp < dtoep)
				*IntPoint = Line[0];
			else
				*IntPoint = Line[1];
			rtn = TRUE;
		}
	}
	else
		rtn = TRUE;
Exit:
{
#if ENABLETRACE
GSSiExitProg (364);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}
BOOL GetPerpendicularIntersect2 (LPDPOINT Point,LPDPOINT LinePt,LPDOUBLE pLineAZ,LPDPOINT IntPoint)
#if ENABLETRACE
{GSSiEnterProg (364);
#endif
{
	double	AZ2, LineLen;

	AZ2 = LTWOPI (*pLineAZ + HALFPI);
	if (LINSEC (LinePt->x,LinePt->y,*pLineAZ,Point->x,Point->y,AZ2,&IntPoint->x,&IntPoint->y) != 1)
{
#if ENABLETRACE
GSSiExitProg (364);
#endif
		return FALSE;
}
{
#if ENABLETRACE
GSSiExitProg (364);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

double RGBDist (RGBTRIPLE C1, RGBTRIPLE C2)
{
	double dist; 
	
	dist = pow (pow ((double)C1.rgbtRed-(double)C2.rgbtRed,2) + 
				pow ((double)C1.rgbtGreen-(double)C2.rgbtGreen,2) +
				pow ((double)C1.rgbtBlue-(double)C2.rgbtBlue,2)
				,0.5e0);
	return dist;
}

double RGBQUADDist (RGBQUAD C1, RGBQUAD C2)
{
	double dist; 
	
	dist = pow (pow ((double)C1.rgbRed-(double)C2.rgbRed,2) + 
				pow ((double)C1.rgbGreen-(double)C2.rgbGreen,2) +
				pow ((double)C1.rgbBlue-(double)C2.rgbBlue,2)
				,0.5e0);
	return dist;
}

double ColorDist (COLORREF C1, COLORREF C2)
{
	double dist; 
	
/*	double	r1 =  GetRValue(C1);
	double	g1 =  GetGValue(C1);
	double	b1 =  GetBValue(C1);
	double	r2 =  GetRValue(C2);
	double	g2 =  GetGValue(C2);
	double	b2 =  GetBValue(C2);  */
	
	dist = pow (pow ((double)GetRValue(C1)-(double)GetRValue(C2),2) + 
				pow ((double)GetGValue(C1)-(double)GetGValue(C2),2) +
				pow ((double)GetBValue(C1)-(double)GetBValue(C2),2)
				,0.5e0);
	return dist;
}

COLORREF NearestColor (COLORREF Color, COLORREF C1, COLORREF C2)
{
	if (ColorDist (Color,C1) < ColorDist (Color,C2))
		return C2;
	else
		return C1;
}
BOOL RectanglesAreEqual (LPRECT pRect1,LPRECT pRect2)
{
	if (pRect1->left   != pRect2->left ||
		pRect1->right  != pRect2->right ||
		pRect1->top    != pRect2->top ||
		pRect1->bottom != pRect2->bottom)
		return FALSE;
	return TRUE;
}
                         
void AZToAZIM (double AZ,LPSTR DegC,LPSTR MinC,LPSTR SecC)
#if ENABLETRACE
{GSSiEnterProg (412);
#endif
{
      short IDEG, IMIN, QUAD;
      double AN, SEC;
//C******* CHANGE AZIMUTH TO USER BEARING
      AN = LTWOPI(HALFPI - AZ);
//C******* CHANGE FROM RADIANS TO DEGREES
      AN = AN * DEGRAD + HAFSEC;
//C
//C******* DETERMINE MINUTES
      IDEG = AN;
      AN = (AN - IDEG) * 6E1;
//C
//C******* DETERMINE SECONDS
      IMIN = AN;
      SEC = fabs((AN - IMIN) * 6E1  - 5E-2);
//C
//C******* PUT BEARING IN CHARACTER FORMAT 
	  itoa (IDEG,DegC,10);
	  itoa (IMIN,MinC,10);
	  sprintf (SecC,"%.4f",SEC);
{
#if ENABLETRACE
GSSiExitProg (412);
#endif
   return;
}
#if ENABLETRACE
}
#endif
}

void AZToBear (double AZ,LPSTR PreDir,LPSTR DegC,LPSTR MinC,LPSTR SecC,LPSTR PostDir)
#if ENABLETRACE
{GSSiEnterProg (411);
#endif
{
      int	IDEG, IMIN, QUAD;
      double AN, SEC;
      char DIR1[5]="NSSN", DIR2[5]="EEWW";
//C******* CHANGE AZIMUTH TO USER BEARING
      AN = LTWOPI(HALFPI - AZ);
//C
//C******* FIND QUADRANT THAT AZIMUTH IS IN
      if (TWOPI - AN > PIHALF) goto S50;
      if (TWOPI - AN > PY) goto S20;
      if (TWOPI - AN > HALFPI) goto S30;
      goto S40;
//C
//C******* AZIMUTH IS IN THE SE QUADRANT
 S20: AN = PY - AN;
      QUAD = 2;
      goto S60;
//C
//C******* AZIMUTH IS IN THE SW QUADRANT
 S30: AN = AN - PY;
      QUAD = 3;
      goto S60;
//C
//C******* AZIMUTH IS IN THE NW QUADRANT
 S40: AN = TWOPI - AN;
      QUAD = 4;
      goto S60;
//C
//C******* AZIMUTH IS IN THE NE QUADRANT
 S50: QUAD = 1;
    
//C******* INITIALIZE ARRAY TO BLANKS
 S60: 
//C
//C******* CHANGE FROM RADIANS TO DEGREES
      AN = AN * DEGRAD + HAFSEC;
//C
//C******* DETERMINE MINUTES
      IDEG = AN;
      AN = (AN - IDEG) * 6E1;
//C
//C******* DETERMINE SECONDS
      IMIN = AN;
      SEC = fabs((AN - IMIN) * 6E1  - 5E-2);
//C
//C******* PUT BEARING IN CHARACTER FORMAT 
	  *PreDir = DIR1[QUAD-1]; 
	  PreDir[1]=0;
	  *PostDir = DIR2[QUAD-1];
	  PostDir[1]=0;  
	  sprintf (DegC,"%2.2i",IDEG);  
	  sprintf (MinC,"%2.2i",IMIN);  
	  sprintf (SecC,"%05.2f",SEC);
{
#if ENABLETRACE
GSSiExitProg (411);
#endif
   return;
}
#if ENABLETRACE
}
#endif
}

double CourseOverGround (double lat1,double lon1,double lat2,double lon2)
{
	double cog;

	lat1 *= RADDEG;
	lon1 *= RADDEG;
	lat2 *= RADDEG;
	lon2 *= RADDEG;

	cog = atan2 (sin(lon2-lon1)*sin(lat2),cos(lat1)*sin(lat2)-sin(lat1)*cos(lat2)*cos(lon2-lon1));
	cog *= DEGRAD;
	cog = fmod (cog + 360.0,360.0);
	return cog;
}

double CourseOverGroundBase (LPDPOINT pPt1, LPDPOINT pPt2)
{
	double cog;
	DPOINT pt1 = *pPt1;
	DPOINT pt2 = *pPt2;

	ConvertCoord (&pt1,1,2);
	ConvertCoord (&pt2,1,2);

	cog =  CourseOverGround (pt1.y,pt1.x,pt2.y,pt2.x);
	return cog;
}

double ConvertDist (double Dist,int outUnits)
{   
//char	DistUnitOpts[5][12]={"FEET","METERS","YARDS","MILES","KILOMETERS"};
	extern  long  PRJ_UNITS[MAX_PROJ];
	switch (PRJ_UNITS[1])
	{
	case PRJ_UNITS_FEET:
		switch (outUnits)
			{
			case IU_FEET:
					return (Dist);
			case IU_METERS:
					return Dist * FTM;
			case IU_MILES:
					return (Dist) / 5280.0;
			case IU_YARDS:
					return (Dist) / 3.0;
			case IU_KILOMETERS:
					return Dist/1000 * FTM;
			} 
			break;
		case PRJ_UNITS_METERS:
		case PRJ_UNITS_LATLON:
			switch (outUnits)
			{
				case IU_FEET:
					return (Dist * MFT);
				case IU_METERS:
					return Dist;
				case IU_MILES:
					return (Dist * MFT) / 5280.0;
				case IU_YARDS:
					return (Dist * MFT) / 3.0;
				case IU_KILOMETERS:
					return Dist / 1000;
			} 
			break;
	}
	return Dist;
} 

double ConvertInDist (double Dist,int opt)
{   
//char	DistUnitOpts[5][12]={"FEET","METERS","YARDS","MILES","KILOMETERS"};
	extern  long  PRJ_UNITS[MAX_PROJ];
	if (PRJ_UNITS[1] == PRJ_UNITS_METERS) //meters
	switch (opt)
	{
		case 1:
			return (Dist * FTM);
		case 2:
			return Dist;
		case 4:
			return (Dist * FTM) * 5280.0;
		case 3:
			return (Dist * FTM) * 3.0;
		case 5:
			return Dist*1000;
	}
	if (PRJ_UNITS[1] == PRJ_UNITS_FEET)//feet
	switch (opt)
	{
		case 1:
			return Dist;
		case 2:
			return Dist * MFT;
		case 4:
			return Dist * 5280.0;
		case 3:
			return Dist * 3.0;
		case 5:
			return Dist * MFT * 1000;
	}
	if (PRJ_UNITS[1] == PRJ_UNITS_LATLON)//latlon
	{
		DPOINT newPt;
		DPOINT zpt={0,0};

		switch (opt)
			{
			case 1:
				Dist = Dist * FTM;
				break;
			case 2:
				break;
			case 4:
				Dist = (Dist * FTM) * 5280.0;
				break;
			case 3:
				Dist = (Dist * FTM) * 3.0;
				break;
			case 5:
				Dist*=1000;
				break;
			}
		newPt = NewLatLong(0,0,Dist,0);
		Dist = ldistp (zpt,newPt);
		Dist = ArcDistance(zpt,newPt);
	}
	return Dist;
} 

double ConvertAltDist(double Dist, int opt)
{
	//char	DistUnitOpts[5][12]={"FEET","METERS","YARDS","MILES","KILOMETERS"};
	extern  long  PRJ_UNITS[MAX_PROJ];
	if (PRJ_UNITS[3] == PRJ_UNITS_METERS) //meters
		switch (opt)
		{
		case 1:
			return (Dist * FTM);
		case 2:
			return Dist;
		case 4:
			return (Dist * FTM) * 5280.0;
		case 3:
			return (Dist * FTM) * 3.0;
		case 5:
			return Dist * 1000;
		}
	if (PRJ_UNITS[3] == PRJ_UNITS_FEET)//feet
		switch (opt)
		{
		case 1:
			return Dist;
		case 2:
			return Dist * MFT;
		case 4:
			return Dist * 5280.0;
		case 3:
			return Dist * 3.0;
		case 5:
			return Dist * MFT * 1000;
		}
	if (PRJ_UNITS[3] == PRJ_UNITS_LATLON)//latlon
	{
		DPOINT newPt;
		DPOINT zpt = { 0,0 };

		switch (opt)
		{
		case 1:
			Dist = Dist * FTM;
			break;
		case 2:
			break;
		case 4:
			Dist = (Dist * FTM) * 5280.0;
			break;
		case 3:
			Dist = (Dist * FTM) * 3.0;
			break;
		case 5:
			Dist *= 1000;
			break;
		}
		newPt = NewLatLong(0, 0, Dist, 0);
		Dist = ldistp(zpt, newPt);
		Dist = ArcDistance(zpt, newPt);
	}
	return Dist;
}

double ConvertDist2 (double Dist,int from, int to)
{   
//char	DistUnitOpts[5][12]={"FEET","METERS","YARDS","MILES","KILOMETERS"};
	if (from == 4)
	{
		from = 1;
		Dist *= 5280;
	}
	switch (from)
	{   
		case 3:
			Dist *= 3;
		case 1:
			switch (to)
			{
				case 1:
					return (Dist);
				case 2:
					return Dist * FTM;
				case 4:
					return (Dist) / 5280.0;
				case 3:
					return (Dist) / 3.0;
				case 5:
					return Dist/1000 * FTM;
			}
		break;
		
		case 5:
			Dist *= 1000;
		case 2:
			switch (to)
			{
				case 1:
					return (Dist * MFT);
				case 2:
					return Dist;
				case 4:
					return (Dist * MFT) / 5280.0;
				case 3:
					return (Dist * MFT) / 3.0;
				case 5:
					return Dist / 1000;
			}
		break;
	}
	return Dist;
}                     


double ConvertArea (double area,int opt)   
{
	extern  long  PRJ_UNITS[MAX_PROJ];
//char	AreaUnitOpts[6][10]={"SQRFEET","SQRMETERS","SQRYARDS","SQRMILES","SQRKILOS","ACRES"};
	if (PRJ_UNITS[1] == 1)
	switch (opt)
	{
		case 1:
			return area;
		case 2:
			return (area * SFTSM);
		case 4:
			return (area / ((double)5280 * (double)5280));  
		case 3:
			return (area/9);
		case 5:
			return ((area * SFTSM) /(double) 1000000);
		case 6:
			return ((area * SFTSM) / ACRSM);
	}
	if (PRJ_UNITS[1] == 2)
	switch (opt)
	{
		case 1:
			return (area * SMSFT);
		case 2:
			return area;
		case 4:
			return (area * (SMSFT / ((double)5280 * (double)5280)));  
		case 3:
			return (area/9 * SMSFT);
		case 5:
			return (area /(double) 1000000);
		case 6:
			return (area / ACRSM);
	}
	return area;
} 

double LDIST(double X1,double Y1,double X2,double Y2)
{ 
	double xdf = (X1-X2);
	double ydf = (Y1-Y2);
  	
  	return sqrt(xdf * xdf + ydf * ydf);
}

void AddToMinMaxD (LPMNMXCORD mm1, LPDPOINT pPoint)
#if ENABLETRACE
{GSSiEnterProg (189);
#endif
{
    mm1->xmn = min (mm1->xmn,pPoint->x);
    mm1->xmx = max (mm1->xmx,pPoint->x);
    mm1->ymn = min (mm1->ymn,pPoint->y);
    mm1->ymx = max (mm1->ymx,pPoint->y); 
{
#if ENABLETRACE
GSSiExitProg (189);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void AddMinMax (LPMINMAX mm1, LPMINMAX mm2)
#if ENABLETRACE
{GSSiEnterProg (190);
#endif
{
    mm1->xmn = min (mm1->xmn,mm2->xmn);
    mm1->xmx = max (mm1->xmx,mm2->xmx);
    mm1->ymn = min (mm1->ymn,mm2->ymn);
    mm1->ymx = max (mm1->ymx,mm2->ymx);     
{
#if ENABLETRACE
GSSiExitProg (190);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void AddMinMaxD(LPMNMXCORD mm1, LPMNMXCORD mm2)
#if ENABLETRACE
{GSSiEnterProg (191);
#endif
{
	mm1->xmn = min(mm1->xmn, mm2->xmn);
	mm1->xmx = max(mm1->xmx, mm2->xmx);
	mm1->ymn = min(mm1->ymn, mm2->ymn);
	mm1->ymx = max(mm1->ymx, mm2->ymx);
	{
#if ENABLETRACE
		GSSiExitProg (191);
#endif
		return;
	}
#if ENABLETRACE
}
#endif
}
void AddMinMax3D(LPMNMXCORD3D mm1, LPMNMXCORD3D mm2)
#if ENABLETRACE
{GSSiEnterProg (191);
#endif
{
	mm1->xmn = min(mm1->xmn, mm2->xmn);
	mm1->xmx = max(mm1->xmx, mm2->xmx);
	mm1->ymn = min(mm1->ymn, mm2->ymn);
	mm1->ymx = max(mm1->ymx, mm2->ymx);
	mm1->zmn = min(mm1->zmn, mm2->zmn);
	mm1->zmx = max(mm1->zmx, mm2->zmx);
	{
#if ENABLETRACE
		GSSiExitProg(191);
#endif
		return;
	}
#if ENABLETRACE
}
#endif
}

void AddMinMaxL (LPMNMXCORL mm1, LPMNMXCORL mm2)
#if ENABLETRACE
{GSSiEnterProg (191);
#endif
{
    mm1->xmn = min (mm1->xmn,mm2->xmn);
    mm1->xmx = max (mm1->xmx,mm2->xmx);
    mm1->ymn = min (mm1->ymn,mm2->ymn);
    mm1->ymx = max (mm1->ymx,mm2->ymx);
{
#if ENABLETRACE
GSSiExitProg (191);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

long Factorial (long value)
#if ENABLETRACE
{GSSiEnterProg (194);
#endif
{
    long    n=0;  
    
    while (value--)
        n+=value;
{
#if ENABLETRACE
GSSiExitProg (194);
#endif
    return  n;
}
#if ENABLETRACE
}
#endif
} 

DPOINT AverageDPoint (DPOINT Point1, DPOINT Point2)
#if ENABLETRACE
{GSSiEnterProg (207);
#endif
{   
    DPOINT  OutPoint;
    
    OutPoint.x = (Point1.x + Point2.x)/2;
    OutPoint.y = (Point1.y + Point2.y)/2;
{
#if ENABLETRACE
GSSiExitProg (207);
#endif
    return (OutPoint);
}
#if ENABLETRACE
}
#endif
}    

POINT AveragePoint (POINT Point1, POINT Point2)
#if ENABLETRACE
{GSSiEnterProg (208);
#endif
{   
    POINT  OutPoint;
    
    OutPoint.x = ((long)Point1.x + (long)Point2.x)/2;
    OutPoint.y = ((long)Point1.y + (long)Point2.y)/2;
{
#if ENABLETRACE
GSSiExitProg (208);
#endif
    return (OutPoint);
}
#if ENABLETRACE
}
#endif
}    

LPOINT AveragePointL (LPOINT Point1, LPOINT Point2)
#if ENABLETRACE
{GSSiEnterProg (208);
#endif
{   
    LPOINT  OutPoint;
    
    OutPoint.x = ((long)Point1.x + (long)Point2.x)/2;
    OutPoint.y = ((long)Point1.y + (long)Point2.y)/2;
{
#if ENABLETRACE
GSSiExitProg (208);
#endif
    return (OutPoint);
}
#if ENABLETRACE
}
#endif
}    

DPOINT AverageDPoints (HPDPOINT Point,long nPnts)
#if ENABLETRACE
{GSSiEnterProg (209);
#endif
{   
    DPOINT  OutPoint={0.0,0.0};
    long	n=nPnts;
    
    while (n--)
    {
    	OutPoint.x += Point->x;
    	OutPoint.y += Point++->y;
    }
    OutPoint.x /= nPnts;
    OutPoint.y /= nPnts;
{
#if ENABLETRACE
GSSiExitProg (209);
#endif
    return (OutPoint);
}
#if ENABLETRACE
}
#endif
}    

double Round (double Value,double RoundTo)
#if ENABLETRACE
{GSSiEnterProg (221);
#endif
{   
	double Rem;
	BOOL	negval = Value < 0;
	
	Value = fabs (Value);
	Rem = fmod(Value,RoundTo);
	if (Rem >= RoundTo / 2)
		Value = Value - Rem + RoundTo;
	else
		Value = Value - Rem;
	if (Value > 0 && negval)
		Value = -Value;
{
#if ENABLETRACE
GSSiExitProg (221);
#endif
	return Value;
}
#if ENABLETRACE
}
#endif
} 

double DecDegFromDMS (LPSTR DMS,LPBOOL pErr)
#if ENABLETRACE
{GSSiEnterProg (222);
#endif
{   
	double dir=1, deg=0,min,sec;   
	LPSTR	pEnd;
	
	*pErr = 0;
	switch (*DMS)
	{
		case 'N':
		case 'E':
			DMS++;
			break;
		case 'S':
		case 'W':
			dir=-1;
			DMS++;
			break;
	} 
	if ((pEnd = FirstAlpha (DMS)) && *pEnd)
	{
		if (*pEnd == 'd' || *pEnd == 'D')
		{   
			*pEnd = 0;
			deg = strtod (DMS,&pEnd); 
			*pEnd = 'D';
			DMS = pEnd + 1;
		}
		else
		{
			*pErr = 1;
			goto Exit;
		}
	}
	else 
	{
		deg = strtod (DMS,&pEnd);
		goto Exit;
	}
	pEnd++;
	min = strtod (pEnd,&pEnd);
	pEnd++;
	sec = strtod (pEnd,&pEnd);  
	deg += min / 60 + sec / 3600; 
Exit:
	deg *= dir;
{
#if ENABLETRACE
GSSiExitProg (222);
#endif
	return deg;
}
#if ENABLETRACE
}
#endif
}

BOOL GetDMS (double DecDegrees,LPSHORT Deg,LPSHORT Min,LPDOUBLE Sec)
#if ENABLETRACE
{GSSiEnterProg (223);
#endif
{   
	double	Neg=1, Rem, SEC=(double)1/(double)3600;
	
	if (DecDegrees > 360)
{
#if ENABLETRACE
GSSiExitProg (223);
#endif
		return FALSE;
}
	if (DecDegrees < 0)                
	{
		Neg = -1;
		DecDegrees = fabs (DecDegrees);
	}
	DecDegrees += SEC/5000;
	*Deg = floor (DecDegrees);
	Rem = DecDegrees - (double)*Deg;
	Rem *= 60;
	*Min = floor (Rem);
	Rem = Rem - (double)*Min;
	*Sec = Rem * 60;
	if (*Sec < 0.0000001)
		*Sec = 0; 
	*Deg *= Neg;
{
#if ENABLETRACE
GSSiExitProg (223);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}


void BiasPoly (HANDLE hDPoints,long np,double xbias,double ybias)
{   
	HPDPOINT	pPoint=(HPDPOINT)GlobalLock (hDPoints);
	 
	while (np--)
	{
		pPoint->x += xbias;
		pPoint++->y += ybias;
	} 
	GlobalUnlock (hDPoints);
	
	return;
}

HANDLE	GetOffsetPoly (LPDPOINT pPoints,int nPoints,LPINT pnOffPoints,double Offset)
{
	HANDLE hOffPoly=0;
	int	i;
	LPDPOINT	pOffPoints;
	double	AZ1, AZ2, AZ, AZDiff;

	if (nPoints < 1)
		return 0;
	hOffPoly = GSSiGlobAlloc (1594,GMEM_MOVEABLE,nPoints*sizeof(DPOINT));
	pOffPoints = GlobalLock (hOffPoly);
	AZ1 = getazd (&pPoints[0], &pPoints[1]);
	pOffPoints[0] = dnewpt (pPoints[0], AZ1+HALFPI,Offset);
	for (i=1;i<nPoints-1;i++)
	{
		AZ2 = getazd (&pPoints[i], &pPoints[i+1]);
		AZDiff = DeltaAZ (AZ1,AZ2);
		AZ = AZ1 + AZDiff/2;
		pOffPoints[i] = dnewpt (pPoints[i], AZ+HALFPI,Offset);
		AZ1 = AZ2;
	}
	pOffPoints[nPoints-1] = dnewpt (pPoints[nPoints-1],AZ1+HALFPI,Offset);
	
	GlobalUnlock (hOffPoly);
	*pnOffPoints = nPoints;

	return hOffPoly;
}

void GetPolyBoundsD2 (LPDPOINT pPoint,long np,LPMNMXCORD pBounds,int Type)
{
	
	DBoundsInit (pBounds); 
	if (Type == TYPE_AREA && np == 2)
	{
		double	Radius = ldistp (pPoint[0],pPoint[1]);
		DPOINT	NewPoint;

		NewPoint = dnewpt (pPoint[0],0,Radius);
		AddDPointToMinMax (&NewPoint,pBounds); 
		NewPoint = dnewpt (pPoint[0],HALFPI,Radius);
		AddDPointToMinMax (&NewPoint,pBounds); 
		NewPoint = dnewpt (pPoint[0],PY,Radius);
		AddDPointToMinMax (&NewPoint,pBounds); 
		NewPoint = dnewpt (pPoint[0],PIHALF,Radius);
		AddDPointToMinMax (&NewPoint,pBounds); 
	}
	else 
		while (np--)
			AddDPointToMinMax (pPoint++,pBounds); 
	return;
}

void GetPolyBoundsD (HANDLE hPoly,long np,LPMNMXCORD pBounds,int Type)
{
	HPDPOINT	pPoint=(HPDPOINT)GlobalLock (hPoly);
	
	GetPolyBoundsD2 (pPoint,np,pBounds,Type);
	GlobalUnlock (hPoly);
	return;
}
RECT GetPolyBounds2 (LPPOINT Poly,long np)
{
	RECT rect;
	int	i;

	RectInit (&rect);
	for (i=0;i<np;i++)
		AddPointToRect (Poly[i],&rect);

	return rect;
}

RECT GetPolyBounds (HANDLE hPoly,long np)
{
	HPPOINT	pPoint=(HPPOINT)GlobalLock (hPoly);
	RECT	rect;
	
	rect = GetPolyBounds2 (pPoint,np);
	GlobalUnlock (hPoly);
	return rect;
}

DPOINT PointAtDistOnPoly16 (HPPOINT lpPoints,long nPnts,double AtDist,LPDOUBLE pAZ,LPSHORT pLasti)   
#if ENABLETRACE
{GSSiEnterProg (1124);
#endif
{
	double Dist=0, LastDist, AZ, dst;
	DWORD	i; 
	HPPOINT	lpPoints2=lpPoints+1; 
	DPOINT	Point;
	
	for (i=1;i<nPnts;i++,lpPoints++,lpPoints2++)
	{   
		LastDist = Dist;
		Dist += idist(*lpPoints,*lpPoints2);
		if (Dist >= AtDist)
		{
			AZ = getaz (*lpPoints,*lpPoints2);
			dst = AtDist - LastDist; 
    		Point = dnewpt (PointToDPoint (*lpPoints),AZ,dst);   
    		if (pAZ)
    			*pAZ = AZ;
    		if (pLasti)
    			*pLasti = i-1;
{
#if ENABLETRACE
GSSiExitProg (1124);
#endif
    		return Point;
}
		}
		Point = PointToDPoint (*lpPoints2);
	} 
	LastDist = Dist; 
	lpPoints2--;
	lpPoints--;
	AZ = getaz (*lpPoints,*lpPoints2);
	dst = AtDist - LastDist; 
	Point = dnewpt (PointToDPoint (*lpPoints2),AZ,dst);   
	if (pLasti)
		*pLasti = nPnts;
	if (pAZ)
		*pAZ = AZ;
{
#if ENABLETRACE
GSSiExitProg (1124);
#endif
	return Point;
}
#if ENABLETRACE
}
#endif
}
DPOINT PointAtDistOnPolyF(HPFPOINT lpPoints, long nPnts, double AtDist, LPDOUBLE pAZ, LPSHORT pLasti)
#if ENABLETRACE
{GSSiEnterProg (1124);
#endif
{
	double Dist = 0, LastDist, AZ, dst;
	DWORD	i;
	HPFPOINT	lpPoints2 = lpPoints + 1;
	FPOINT	Point;

	for (i = 1; i<nPnts; i++, lpPoints++, lpPoints2++)
	{
		LastDist = Dist;
		Dist += ldistp(*lpPoints, *lpPoints2);
		if (Dist >= AtDist)
		{
			AZ = getazF(*lpPoints, *lpPoints2);
			dst = AtDist - LastDist;
			Point = newptF(*lpPoints, AZ, dst);
			if (pAZ)
				*pAZ = AZ;
			if (pLasti)
				*pLasti = i - 1;
			{
#if ENABLETRACE
				GSSiExitProg(1124);
#endif
				return Point;
			}
		}
		Point = *lpPoints2;
	}
	LastDist = Dist;
	lpPoints2--;
	lpPoints--;
	AZ = getazF(*lpPoints, *lpPoints2);
	dst = AtDist - LastDist;
	Point = newptF(*lpPoints2, AZ, dst);
	if (pLasti)
		*pLasti = nPnts;
	if (pAZ)
		*pAZ = AZ;
	{
#if ENABLETRACE
		GSSiExitProg(1124);
#endif
		return Point;
	}
#if ENABLETRACE
}
#endif
}
DPOINT3D PointAtDistOnPoly3D(HPDPOINT3D lpPoints, long nPnts, double AtDist, LPDOUBLE pAZ, LPLONG pEndPointNum)
#if ENABLETRACE
{GSSiEnterProg (1124);
#endif
{
	double Dist=0, LastDist, AZ=0, dst;
	DWORD	i; 
	HPDPOINT3D	lpPoints2=lpPoints+1; 
	DPOINT3D	Point = *lpPoints;
	DPOINT		Point2D;
	
	for (i=1;i<nPnts;i++,lpPoints++,lpPoints2++)
	{   
		LastDist = Dist;
		Dist += ldistpp((LPDPOINT)lpPoints,(LPDPOINT)lpPoints2);
		if (Dist >= AtDist)
		{
			AZ = getazd ((LPDPOINT)lpPoints,(LPDPOINT)lpPoints2);
			dst = AtDist - LastDist; 
    		Point = DPointToDPoint3D (dnewpt (DPoint3DToDPoint(*lpPoints),AZ,dst));   
    		if (AtDist - LastDist)
    			Point.z = lpPoints->z + (lpPoints2->z - lpPoints->z) * (dst/(Dist - LastDist));
    		else
    			Point.z = lpPoints->z;	
    		if (pAZ)
    			*pAZ = AZ;
    		if (pEndPointNum)
    			*pEndPointNum = i-1;
{
#if ENABLETRACE
GSSiExitProg (1124);
#endif
    		return Point;
}
		}
		Point = *lpPoints2;
	}
	if (pAZ)
		*pAZ = AZ;
	if (pEndPointNum)
		*pEndPointNum = nPnts-1;
{
#if ENABLETRACE
GSSiExitProg (1124);
#endif
	return Point;
}
#if ENABLETRACE
}
#endif
}

DPOINT PointAtDistOnPoly (HPDPOINT lpPoints,long nPnts,double AtDist,LPDOUBLE pAZ,LPLONG pEndPointNum)   
#if ENABLETRACE
{GSSiEnterProg (1124);
#endif
{
	double Dist=0, LastDist, AZ=0, dst;
	DWORD	i; 
	HPDPOINT	lpPoints2=lpPoints+1; 
	DPOINT	Point=*lpPoints;
	
	for (i=1;i<nPnts;i++,lpPoints++,lpPoints2++)
	{   
		LastDist = Dist;
		Dist += ldistp(*lpPoints,*lpPoints2);
		if (Dist >= AtDist)
		{
			AZ = getazd (lpPoints,lpPoints2);
			dst = AtDist - LastDist; 
    		Point = dnewpt (*lpPoints,AZ,dst);   
    		if (pAZ)
    			*pAZ = AZ;
    		if (pEndPointNum)
    			*pEndPointNum = i-1;
{
#if ENABLETRACE
GSSiExitProg (1124);
#endif
    		return Point;
}
		}
		Point = *lpPoints2;
	}
	if (pAZ)
		*pAZ = AZ;
	if (pEndPointNum)
		*pEndPointNum = nPnts-1;
{
#if ENABLETRACE
GSSiExitProg (1124);
#endif
	return Point;
}
#if ENABLETRACE
}
#endif
}

DPOINT PointAtScreenXOnPoly (HPDPOINT lpPoints,long nPnts,double AtX,LPDOUBLE pAZ)   
#if ENABLETRACE
{GSSiEnterProg (1124);
#endif
{
	double Dist=0, LastDist, AZ=0, dst, DToBP, DToEP;
	DWORD	i; 
	DPOINT	Point, BasePt1=*lpPoints++, BasePt2, Point1=BasePtToScreenPtD (&BasePt1), Point2, BPoint=BasePt1;
	
	DToBP = fabs(Point1.x - AtX);
	for (i=1;i<nPnts;i++,lpPoints++)
	{   
		BasePt2 = *lpPoints;
		Point2=BasePtToScreenPtD (&BasePt2);
		if (AtX >= min(Point1.x,Point2.x) && AtX <= max (Point1.x,Point2.x))
		{
			double	pct=0;

			if (Point2.x - Point1.x != 0)
				pct = (AtX - Point1.x)/(Point2.x - Point1.x);
			Point.x = BasePt1.x + pct * (BasePt2.x - BasePt1.x); 
			Point.y = BasePt1.y + pct * (BasePt2.y - BasePt1.y);
			AZ = getazd(&BasePt1, &BasePt2);
			if (pAZ)
				*pAZ = AZ;
{
#if ENABLETRACE
GSSiExitProg (1124);
#endif
    		return Point;
}
		}
		BasePt1 = BasePt2;
		Point1 = Point2;
	}
	DToEP = fabs(Point1.x - AtX);
	if (DToBP < DToEP)
		Point = BPoint;
	else
		Point = BasePt1;

{
#if ENABLETRACE
GSSiExitProg (1124);
#endif
	return Point;
}
#if ENABLETRACE
}
#endif
}

HANDLE GetPolyBetweenDist (HPDPOINT lpPoints,long nPnts,double StartDist,double EndDist,LPINT pnPnts,BOOL ShapePointsOnly,BOOL WantOp)
#if ENABLETRACE
{GSSiEnterProg (1125);
#endif
{
	double Dist=0, LastDist, AZ, dst;
	DWORD	i,j; 
	HPDPOINT	lpPoints2=lpPoints+1, lpPointsIn = lpPoints; 
	HANDLE	hPoly = GSSiGlobAlloc (1069,GMEM_MOVEABLE,nPnts*sizeof(DPOINT));
	HPDPOINT	pPoly=(HPDPOINT)GlobalLock (hPoly); 
	HPDPOINT	pPnt;
	
	*pnPnts = 0;
	for (i=1;i<nPnts;i++,lpPoints++,lpPoints2++)
	{   
		LastDist = Dist;
		if (Dist == StartDist && !ShapePointsOnly)
		{
			if (WantOp)
			{   
				pPnt = lpPoints;
				for (j=i;j;j--)
					pPoly[(*pnPnts)++] = *pPnt--;
			}
			else
				pPoly[(*pnPnts)++] = *lpPoints;
		}
		Dist += ldistp(*lpPoints,*lpPoints2);
		if (!*pnPnts && Dist > StartDist && !ShapePointsOnly)
		{
			AZ = getazd (lpPoints,lpPoints2);
			dst = StartDist - LastDist; 
    		pPoly[(*pnPnts)++] = dnewpt (*lpPoints,AZ,dst);
		}
		if (Dist == EndDist)
		{   
			if (!ShapePointsOnly)
				pPoly[(*pnPnts)++] = *lpPoints2; 
			if (WantOp)
			{
				pPnt = lpPointsIn + (nPnts-1);
				for (j=i;j;j--)
					pPoly[(*pnPnts)++] = *pPnt--; 
			}
			break;
		}
		if (!WantOp && Dist > StartDist && Dist < EndDist)
			pPoly[(*pnPnts)++] = *lpPoints2;
		else if (Dist > EndDist && !ShapePointsOnly)
		{
			AZ = getazd (lpPoints,lpPoints2);
			dst = EndDist - LastDist; 
    		pPoly[(*pnPnts)++] = dnewpt (*lpPoints,AZ,dst);
			if (WantOp)
			{
				pPnt = lpPointsIn + (nPnts-1);
				for (j=i;j;j--)
					pPoly[(*pnPnts)++] = *pPnt--; 
			}
			break;
		}
	}
	GlobalUnlock (hPoly); 
	if (!*pnPnts)
		GSSiGlobFree (&hPoly);
{
#if ENABLETRACE
GSSiExitProg (1125);
#endif
	return hPoly;
}
#if ENABLETRACE
}
#endif
} 

BOOL GetAreaCenters (LPSTR OutFile,int SpeedFactor,BOOL UseMask)
{    
	short	pos = BT_FIRST, x, y;
	long	Refno;
	HIGHLIGHTDATA	HighlightData;   
	long	nPnts,loc;
	HANDLE	hPnts;
	HPDPOINT	Points;
	LPPIAAStruct pPIAA;
	HFILE	Fid=GSSiOpenFile (OutFile,0,OF_CREATE);
	DPOINT	MidPoint;
	long	nRecs,NumRecs=1;  
	POINT	CenterPoint, TestPoint;
	char	str[256], TempName[256]; 
	short	SavePIASizeFactor = PIASizeFactor;
	BTVARDESC	BTVar[2];   
	HANDLE	hBTTemp; 
	double	Area;
	HANDLE	hMaskArea= CurView->hMaskArea;     
	USHORT	NumMaskPoints=CurView->NumMaskPoints;
	LPHANDLE	pMaskAccelerator=&CurView->hMaskAccelerator[0]; 
	HPDPOINT	lpMaskPoints;
	LPMNMXCORD	lpRect;  
	POINT	PIAAPoint;
	DPOINT	DPoint;  
	long	iCPDist, maxn;
	
	if (Fid == HFILE_ERROR)
		return FALSE;
	
	GSSiGetTempFileName (0,"gma",0,TempName); 
	
    if (CurView->DisplayInParent && CurView->Parent > 0)  
    {
    	hMaskArea = pViewports[CurView->Parent-1]->hMaskArea; 
    	pMaskAccelerator=&pViewports[CurView->Parent-1]->hMaskAccelerator[0];  
    	NumMaskPoints=pViewports[CurView->Parent-1]->NumMaskPoints;   
    }
	if (UseMask && hMaskArea)
	{
		lpRect = (LPMNMXCORD) GlobalLock (hMaskArea);
	    lpRect++;
	    lpMaskPoints = (LPDPOINT) lpRect;  
	}
	else
		UseMask = FALSE;
	BTVar[0].BT_VARTYP=BT_REAL;
	BTVar[0].BT_VARLEN=8;
	BTVar[0].BT_VAROFF=0;
	BT_CREATE (TempName, 4, FALSE, 1, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
	hBTTemp = BT_OPEN (TempName,0, BT_WRITE, 0);
	while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData))
	{   
		pos = BT_NEXT;
		BT_PUT (hBTTemp,(LPSTR)&HighlightData.PD.Area,(LPSTR)&Refno);
	}
	pos = BT_FIRST;
	if (SpeedFactor)
		PIASizeFactor=(double)SpeedFactor;  
	fputstring ("MaskRefno(B4),REFNO(B4),PREFIX(C8),UDI(C64),X(R8),Y(R8),D(R4)",Fid);
	CreateStatusWind (hWndMain,2,"Get Area Centers");
	nRecs = BT_NUM_IN_INDEX (hHighlight);
	while (ContinueProcessing && !BT_FIND (hBTTemp,(LPSTR)&Area,pos,BT_ANY,(LPSTR)&Refno))
	{   
		pos = BT_NEXT;     
		BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData);
		if (HighlightData.PD.Type == 3)
		{   
			StatusWindowUpdate (0,HighlightData.PD.UDI, nRecs, NumRecs);
			if (GetPolyPoints ((LPPICKDATAHEADER)&HighlightData.PD,FALSE,&nPnts,&hPnts))
			{  
				HPDPOINT	Points = (HPDPOINT)GlobalLock (hPnts);
	      		HANDLE		hAccelerator = PointInAreaAcceleratorSetup (nPnts,Points,1,0,0); 
				HPBYTE		barray;
				BOOL		HaveCP;
	      		
	      		pPIAA = (LPPIAAStruct)GlobalLock (hAccelerator);
	
    			barray = (HPBYTE)&pPIAA->barray[0];
				if (UseMask)
				{
			    	StatusWindowUpdate2 ("Apply Mask",pPIAA->Height,0); 
					for (y=0;y<pPIAA->Height;y++) 
					{   
						for (x=0;x<pPIAA->Width;x++)
						{  
							loc = (long)x + (long)y * pPIAA->Width;
							if (barray[loc] == 1) 
							{
								PIAAPoint.x = x;
								PIAAPoint.y = y;
								DPoint = PIAAPointToDPoint (PIAAPoint,&pPIAA->Bounds,&pPIAA->Factor,pPIAA->Offset,pPIAA->Type); 
						      	if (!*pMaskAccelerator)
						      		*pMaskAccelerator = PointInAreaAcceleratorSetup (NumMaskPoints,lpMaskPoints,1,0,0);
						      	if (PointInAreaAccelerator (&DPoint,*pMaskAccelerator) != 1)
									barray[loc]=0; 
							}
						} 
				    	StatusWindowUpdate2 (0,pPIAA->Height,y); 
					}
				}
		    	StatusWindowUpdate2 ("Locate Center",pPIAA->Height,0);
		    	CenterPoint = PIAACenter (pPIAA,&iCPDist,&maxn,TRUE,&HaveCP);
				if (ContinueProcessing && maxn && HaveCP)
				{   
					double CPDist;
					DPOINT	DP1,DP2;
					POINT	P1,P2;
					
					MidPoint = PIAAPointToDPoint (CenterPoint,&pPIAA->Bounds,&pPIAA->Factor,pPIAA->Offset,pPIAA->Type); 
					P2 = CenterPoint;
					P2.x += iCPDist * 2 + 1;
					DP2 = PIAAPointToDPoint (P2,&pPIAA->Bounds,&pPIAA->Factor,pPIAA->Offset,pPIAA->Type);
					CPDist = ldistp (MidPoint,DP2); 
					sprintf (str,"%ld,%ld,%s,%s,%f,%f,%f",CurView->MaskAreaRefno,Refno,HighlightData.PD.Prefix,HighlightData.PD.UDI,MidPoint.x,MidPoint.y,CPDist);
					fputstring (str,Fid);
				}
	      		GSSiGlobUlFree (&hAccelerator);
				GSSiGlobUlFree (&hPnts);
			}
		}
		StatusWindowUpdate (0,0, nRecs, NumRecs++);
	}
	SetContinueProcessing ( TRUE);
	DestroyStatusWindow(0);  
	GSSiClose2 (&Fid);   
	BT_CLOSEANDDELETE (&hBTTemp);  
	PIASizeFactor = SavePIASizeFactor;
	if (UseMask && hMaskArea)
		GlobalUnlock (hMaskArea);
	return TRUE;
}

void Rotate256(int irot, LPDPOINT pt)
{
#include "rotate256.h"

	float XOUT = B1[irot] * pt->x + C1[irot] * pt->y;
	float YOUT = B2[irot] * pt->y + C2[irot] * pt->x;

	pt->x = XOUT;
	pt->y = YOUT;
	return;
}

