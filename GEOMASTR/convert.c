#include "graphint.h"

void ConvertThemeV1toV2 (LPTHEME pTheme,LPTHEME_V1 pTheme16);

BOOL ConvertThemeV0toV1 (LPTHEME pTheme32,LPTHEME_v0 pTheme_v0)
{   
	HANDLE	hSave=0;
	THEME_V1	Theme;
	LPTHEME_V1 pTheme=&Theme;
	
	memset (pTheme,0,sizeof(THEME_V1));
	switch (pTheme_v0->ID)
	{
		case GF_STREET_TEXT_THEME:
		{
		    LPSTREETTEXTDATA	pStreetData=(LPSTREETTEXTDATA)pTheme_v0->ClassBM,pSDSave;   
		    
		    hSave = GSSiGlobAlloc (1139,GMEM_MOVEABLE,sizeof(STREETTEXTDATA));
		    pSDSave = (LPSTREETTEXTDATA)GlobalLock(hSave);
		    *pSDSave = *pStreetData;
		    GlobalUnlock (hSave);
		    break;
		}  
		 
		case GF_BOUNDS_DISPLAY_THEME:
		{
			LPBOUNDSDISPLAY lpBoundsDisplay=(LPBOUNDSDISPLAY)&pTheme_v0->ClassBM, pBDSave; 

		    hSave = GSSiGlobAlloc (1140,GMEM_MOVEABLE,sizeof(BOUNDSDISPLAY));
		    pBDSave = (LPBOUNDSDISPLAY)GlobalLock(hSave);
		    *pBDSave = *lpBoundsDisplay;
		    GlobalUnlock (hSave);
			break;
		}
	    default:
	    break;
	}
 	_fmemmove (pTheme,pTheme_v0,(size_t)((long)&pTheme->ClassSymbol[MAX_THEME_CLASSES_v0]-(long)pTheme));
 	_fmemmove (&pTheme->SymSizeC,&pTheme_v0->SymSizeC,(size_t)((long)&pTheme->ClassIsSelected[MAX_THEME_CLASSES_v0]-(long)&pTheme->SymSizeC));
 	_fmemmove (&pTheme->Statement,&pTheme_v0->Statement,(size_t)((long)&pTheme->ClassMin[MAX_THEME_CLASSES_v0]-(long)&pTheme->Statement));
 	_fmemmove (&pTheme->ClassMax,&pTheme_v0->ClassMax,sizeof(pTheme_v0->ClassMax));
 	_fmemmove (&pTheme->ClassColor,&pTheme_v0->ClassColor,sizeof(pTheme_v0->ClassColor));
 	_fmemmove (&pTheme->ClassPen,&pTheme_v0->ClassPen,sizeof(pTheme_v0->ClassPen));
 	_fmemmove (&pTheme->ClassBrush,&pTheme_v0->ClassBrush,sizeof(pTheme_v0->ClassBrush));
	pTheme->NoDataBrush = pTheme_v0->NoDataBrush;
	pTheme->InvalidDataBrush = pTheme_v0->InvalidDataBrush;
 	_fmemmove (&pTheme->ClassCount,&pTheme_v0->ClassCount,sizeof(pTheme_v0->ClassCount));
 	_fmemmove (&pTheme->ClassPnt,&pTheme_v0->ClassPnt,sizeof(pTheme_v0->ClassPnt));
 	_fmemmove (&pTheme->BGColor,&pTheme_v0->BGColor,(size_t)((long)&pTheme->ClassClrBox[MAX_THEME_CLASSES_v0]-(long)&pTheme->BGColor));
 	_fmemmove (&pTheme->Margin,&pTheme_v0->Margin,(size_t)((long)&pTheme->ClassBM[MAX_THEME_CLASSES_v0]-(long)&pTheme->Margin));
 	_fmemmove (&pTheme->Title,&pTheme_v0->Title,sizeof(pTheme_v0->Title));
 	_fmemmove (&pTheme->Contents,&pTheme_v0->Contents,sizeof(pTheme_v0->Contents));
 	_fmemmove (&pTheme->SymNum,&pTheme_v0->SymNum,(size_t)((long)&pTheme->ClassDefDB-(long)&pTheme->SymNum));
	switch (pTheme_v0->ID)
	{
		case GF_STREET_TEXT_THEME:
		{
		    LPSTREETTEXTDATA	pStreetData=(LPSTREETTEXTDATA)pTheme->ClassBM,pSDSave;   
		    
		    pSDSave = (LPSTREETTEXTDATA)GlobalLock(hSave);
		    *pStreetData = *pSDSave;
		    break;
		}
		case GF_BOUNDS_DISPLAY_THEME:
		{
			LPBOUNDSDISPLAY lpBoundsDisplay=(LPBOUNDSDISPLAY)pTheme->ClassBM, pBDSave; 
		    
		    pBDSave = (LPBOUNDSDISPLAY)GlobalLock(hSave);
		    *lpBoundsDisplay = *pBDSave;
		    break;
		}
	    default:
	    break;
	} 
	GSSiGlobUlFree (&hSave);
	ConvertThemeV1toV2 (pTheme32,pTheme);
	return TRUE;
}
