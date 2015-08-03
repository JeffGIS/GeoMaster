#include "graphint.h"  
 
static BOOL	CheckQuad=FALSE;

#include "gmextern.h"

long GetFileConnectOffset(int Type, int Desc,mnmxCor MinMax,int AddLength, long BaseRec,LPSTR Rec)
#if ENABLETRACE
{GSSiEnterProg (1218);
#endif
{
    static  long    LastBaseRec, CurRecLenLoc, debugoffset=1056; 
    static  short     CurRecLen, LastType=-1; 
    mnmxCor SaveMinMax;
    LPLONG pQuadOff;
    LPQUAD    pQuad, InQuad;
    long  QuadOff, InQuadOff, seekloc;
    short nBytes, nRead, FileHeader[9], ipos;
    HANDLE      hPltBuf;
    LPSTR       LPpltBuf;
    LPSHORT       ipnt;
    LPLONG      pOffset;
    long    Type13Offset;
    long    Offset, CurOffset, EOFOffset;
    long    ii, RtnLoc,nn=0;    
    short	iblock;
    BOOL    AddEnd=FALSE;  
    BOOL	InDesc;
    LPDESCBLOCK pDescBlock;    
    HPSTR	pQuadTree;
    LPSTR	pQuadOffset;

    pQuadTree = (HPSTR)GlobalLock (hQuadTree);
    pQuadOffset  = GlobalLock (hQuadOffset); 
    
    if (((BaseRec >= 0 && BaseRec == LastBaseRec) || hQuadTree2)
         && Type == LastType && BoundsTotallyInBounds(MinMax,CurMinMax)) 
    {
        Type13Offset = GSSillseek(FidMap,-(6+18),2);    
        nRead = CurRecLen;  
        AddEnd=TRUE;
        goto AddToEnd;
    } 
    if (hQuadTree2)
    {
        SaveMinMax = MinMax;
        MinMax.xmn = ((long)MinMax.xmn+(long)MinMax.xmx)/2;
        MinMax.xmx = MinMax.xmn;
        MinMax.ymn = ((long)MinMax.ymn+(long)MinMax.ymx)/2;
        MinMax.ymx = MinMax.ymn;
    }
    
    for (QuadLevelAt=1;QuadLevelAt<MaxQuadLevel+1;QuadLevelAt++)
    {
        pQuadOff  = (LPLONG)(pQuadOffset + (QuadLevelAt-1)*4); 
        *pQuadOff = labs(*pQuadOff);
    }
        
    QuadLevelAt = 1;
    QuadTypeNext = -1;

    pQuadOff  = (LPLONG)(pQuadOffset + (QuadLevelAt-1)*4);
    QuadOff   = *pQuadOff;
    if (QuadOff < 0) QuadOff = -QuadOff;
    pQuad     = (LPQUAD)(pQuadTree + ((long)(QuadOff-1))*LenQuadSeg);
S20:
    InQuad = pQuad;
    InQuadOff = QuadOff;
    QuadOff = pQuad->Next[0];  
    ipos=0;
    QuadLevelAt++;
    if (QuadOff == NULLOFF) goto S40;
S30:pQuadOff  = (LPLONG)(pQuadOffset + (QuadLevelAt-1)*4);
    *pQuadOff = QuadOff;
    pQuad = (LPQUAD)(pQuadTree + ((long)(QuadOff-1))*LenQuadSeg); 
    nn++;
    if (BoundsTotallyInBounds(MinMax,pQuad->MinMax)) goto S20;
S40:QuadLevelAt--; 
//	InQuadOff = labs(*(LPLONG)(pQuadOffset + max (0,(QuadLevelAt-1))*4));
//    InQuad    = (LPQUAD)(pQuadTree + ((long)(InQuadOff-1))*LenQuadSeg);    
//apparently if hQuadtree exists we want all items located at the bottom of the quad structure and so the minmax gets expanded
// -not a true quad tree.
    if (!QuadLevelAt || (hQuadTree2 && ipos))
    {   
        pQuad = (LPQUAD)InQuad; 
        CurMinMax = pQuad->MinMax;  
        pDescBlock = 0;
        if (hDescBlock && Desc >0)
        {
			pDescBlock = (LPDESCBLOCK)GlobalLock (hDescBlock);
			pDescBlock += ((long)InQuadOff * (long)MaxQuadType * (long)NumDescBlocks +(long) Type* (long)NumDescBlocks); 
			for (iblock=0;iblock<NumDescBlocks;iblock++,pDescBlock++)
			{    
				if (pDescBlock->Desc < 0)
					break;
				if (!pDescBlock->Desc || pDescBlock->Desc == Desc) 
				{
					InDesc = TRUE;
					goto InDBlock;
				} 
			}
			InDesc = FALSE;
InDBlock:
			GlobalUnlock (hDescBlock);
        }
        else
        	InDesc = TRUE;
        if (pQuad->TypeOffset[Type]<0 || !InDesc)
        {   
        	iblock = max (0,min (iblock,NumDescBlocks-1));
            LastQuadOff = -2;
            QuadOff = InQuadOff;
            pQuad =(LPQUAD)( pQuadTree + ((long)(QuadOff-1))*LenQuadSeg);
            CurRecLenLoc = GSSillseek(FidMap,-18,2);
            pQuad->TypeOffset[Type]=CurRecLenLoc;
            if (hDescBlock)
            {
				pDescBlock = (LPDESCBLOCK)GlobalLock (hDescBlock);
				pDescBlock += ((long)QuadOff * (long)MaxQuadType * (long)NumDescBlocks +(long) Type* (long)NumDescBlocks + iblock); 
				pDescBlock->Offset = CurRecLenLoc;
				pDescBlock->Desc = 0;
				GlobalUnlock (hDescBlock);
            }
            ii=BigRead(FidMap,(HPSTR)&FileHeader,18); 
            seekloc=GSSillseek(FidMap,GraphicsOffset+18+(QuadOff-1)*LenQuadSeg+12+Type*4,0);
            ii=BigWrite(FidMap,(char *)&CurRecLenLoc,4,seekloc);
            seekloc=GSSillseek(FidMap,CurRecLenLoc,0);
            ii=BigWrite(FidMap,(char *)&AddLength,2,seekloc); 
            RtnLoc = GSSillseek (FidMap,0,1);
            BigWrite (FidMap,Rec,AddLength,RtnLoc);
            BigWrite (FidMap,(char *)&FileHeader,18,-1);
            CurRecLen = AddLength;  
        }
        else
        {   
            if (InQuadOff == LastQuadOff && Type == LastType && !hDescBlock)
            {   
                nRead = CurRecLen;
                Type13Offset = GSSillseek(FidMap,-(6+18),2);    
                goto AddToEnd;
            }
            LastQuadOff = InQuadOff;
            Offset = pQuad->TypeOffset[Type];    
	        if (hDescBlock && Desc >0)
	        {
				pDescBlock = (LPDESCBLOCK)GlobalLock (hDescBlock);
				pDescBlock += ((long)InQuadOff * (long)MaxQuadType * (long)NumDescBlocks +(long) Type* (long)NumDescBlocks + iblock); 
				Offset = pDescBlock->Offset;
				GlobalUnlock (hDescBlock);
            }
            CurRecLenLoc = Offset; 
            while (Offset >= 0)
            {
                CurRecLenLoc = Offset;   
                if (Offset == debugoffset)
                	ii=1;
                ii=GSSillseek (FidMap,Offset,0); 
                nRead = BigRead (FidMap,(HPSTR)&nBytes,2);
                CurRecLen = nBytes;
                hPltBuf = GSSiGlobAlloc ( 591,GMEM_MOVEABLE,(DWORD)abs(nBytes));
                LPpltBuf = GlobalLock (hPltBuf);
                nRead = BigRead (FidMap,LPpltBuf,abs(nBytes));
                ipnt = (LPSHORT)LPpltBuf;
                pOffset = (LPLONG)FindNextGraphicsLink (ipnt);
                if (pOffset)
                {
	                Type13Offset = Offset + ((long)pOffset - (long)ipnt) + 2;
	                Offset = *pOffset; 
	            }
	            else
	            	Offset = -1;
                GSSiGlobUlFree (&hPltBuf);
             }
    AddToEnd:
             EOFOffset = GSSillseek(FidMap,-18,2); 
             if (EOFOffset == debugoffset)
             	ii=1;
             ii=BigRead(FidMap,(HPSTR)&FileHeader,18); 
             if (EOFOffset - Type13Offset == 6 &&
                 ((long)nRead + (long)AddLength < SHRT_MAX))
             {
                GSSillseek(FidMap,CurRecLenLoc,0); 
                nRead+=(AddLength-8);
                CurRecLen = nRead;
                BigWrite (FidMap,(char *)&nRead,2,CurRecLenLoc); 
                RtnLoc=GSSillseek(FidMap,Type13Offset-2,0);
                BigWrite (FidMap,Rec,AddLength,RtnLoc);
                BigWrite (FidMap,(char *)&FileHeader,18,-1);
             }
             else
             {
                //LastQuadOff = -2;
               seekloc = GSSillseek(FidMap,Type13Offset,0);
               ii= BigWrite(FidMap,(char *)&EOFOffset,4,seekloc);
               seekloc = GSSillseek(FidMap,EOFOffset,0); 
                CurRecLenLoc = EOFOffset;
               ii= BigWrite(FidMap,(char *)&AddLength,2,seekloc);
                CurRecLen = AddLength;
                CurOffset = GSSillseek(FidMap,0,1);  
                RtnLoc = CurOffset;
               ii= BigWrite (FidMap,Rec,AddLength,RtnLoc);
               ii= BigWrite (FidMap,(char *)&FileHeader,18,-1);

             }
        }   
        CurrentSeg = CurRecLenLoc;
        GlobalUnlock (hQuadTree);
        LastBaseRec = BaseRec; 
        if (hQuadTree2 && !AddEnd)
        {   
            pQuadTree = (HPSTR)GlobalLock (hQuadTree2);
            pQuadOff  = (LPLONG)pQuadOffset;
            pQuadOff++;
            while (QuadLevelAt--)
            {
                pQuad = (LPQUAD)(pQuadTree + ((long)(labs(*pQuadOff)-1))*LenQuadSeg);
                if (SaveMinMax.xmn < pQuad->MinMax.xmn || 
                	SaveMinMax.ymn < pQuad->MinMax.ymn ||
                	SaveMinMax.xmx > pQuad->MinMax.xmx ||
                	SaveMinMax.ymx > pQuad->MinMax.ymx)
                	ii=1;
                pQuad->MinMax.xmn = min (pQuad->MinMax.xmn,SaveMinMax.xmn);
                pQuad->MinMax.ymn = min (pQuad->MinMax.ymn,SaveMinMax.ymn);
                pQuad->MinMax.xmx = max (pQuad->MinMax.xmx,SaveMinMax.xmx);
                pQuad->MinMax.ymx = max (pQuad->MinMax.ymx,SaveMinMax.ymx); 
                pQuadOff++;
            }
            GlobalUnlock (hQuadTree2);
        }
        GlobalUnlock (hQuadOffset);
        pQuadTree = 0;
        pQuadOffset  = 0;
	    LastType = Type;
{
#if ENABLETRACE
GSSiExitProg (1218);
#endif
        return RtnLoc;
}
    }
    pQuadOff  = (LPLONG)(pQuadOffset + (QuadLevelAt-1)*4);
    QuadOff   = *pQuadOff;
    if (QuadOff < 0) goto S40;
    *pQuadOff = -QuadOff;
    pQuad = (LPQUAD)(pQuadTree + ((long)(QuadOff-1))*LenQuadSeg);  
    ipos = 1;
    QuadOff = pQuad->Next[1];
    if (QuadOff == NULLOFF) goto S40;
    QuadLevelAt++;
    goto S30;

#if ENABLETRACE
}
#endif
}

BOOL FindNextSegment (void)
#if ENABLETRACE
{GSSiEnterProg (1219);
#endif
{           
    BOOL    rtn;
    short     desc,ii;
    LPDESCBLOCK pDescBlock;
    char	str[MAX_PATH]; 
    switch (MapType)
    {
    	case MT_SID:
	    	if (FidMap == HFILE_ERROR)
	    		goto RtnFalse;
    	    goto RtnTrue;
    	case MT_SHP:
    	   
	    	if (FidMap == HFILE_ERROR)
	    		goto RtnFalse;
			do
			{
	    		CurrentSHPRec = NextSHPRec; 
	    		SHPRecOffset = GetSHPRecordOffset (CurrentSHPRec,TRUE);
	    		NextSHPRec = CurrentSHPRec + 1;
			}
			while (SHPRecOffset == -2);
	    	if (SHPRecOffset == -1)
	    		goto RtnFalse;
	    	goto RtnTrue;
    	case MT_PERSONAL_GEO_DB:
    	    if (!FetchDBRec (PGDBHandle))
	    		goto RtnFalse; 
	    	_fstrcpy (str,"[PGDB.OBJECTID]");
	    	ExpandText (str);
	    	CurrentSHPRec = atol (str); 
	    	goto RtnTrue;
    	case MT_FILE_GEO_DB:
    	    if (!FetchDBRec (FGDBHandle))
	    		goto RtnFalse; 
	    	_fstrcpy (str,"[FGDB.OBJECTID]");
	    	ExpandText (str);
	    	CurrentSHPRec = atol (str); 
	    	goto RtnTrue;
		case MT_DTM: 
			if (GetNextDTMSegment (FALSE))
				goto RtnTrue;  
			DisplayContourLabels (FALSE);
			goto RtnFalse;
		case MT_ORA:
	    	if (FidMap == HFILE_ERROR)
	    		goto RtnFalse;
	    	CurrentORARec = NextORARec; 
	    	ORARecOffset = GetORARecordOffset (CurrentORARec,TRUE);  
	    	NextORARec = CurrentORARec + 1;
	    	if (ORARecOffset < 0)
	    		goto RtnFalse;
	    	goto RtnTrue; 
	    
		case MT_GMD:
			if (FidMap == HFILE_ERROR)
				goto RtnFalse;
			CurrentGMDRec = NextGMDRec;
			GMDRecOffset = GetGMDRecordOffset(CurrentGMDRec, TRUE);
			NextGMDRec = CurrentGMDRec + 1;
			if (GMDRecOffset < 0)
				goto RtnFalse;
			goto RtnTrue;

		case MT_SQLITE:
			if (GetNextSQLITERecord(&CurView->WBounds))
				goto RtnTrue;
			goto RtnFalse;

		case MT_DGN7:
    		if (ReadNextDGNRecord (&CurView->WBounds))
	    		goto RtnTrue; 
    		goto RtnFalse;
		case MT_GPX: 
			if (GetNextGPXSegment (FALSE))
				goto RtnTrue;  
			goto RtnFalse;
    	default:
    	break;
    }   
    if (FidMap == HFILE_ERROR || !hQuadTree)
		goto RtnFalse;
//    if (FoundInvalidRec) 2/13/00
//    	retrn FALSE; 
    if (MinFileTime > TimeRangeEnd || MaxFileTime < TimeRangeBeg)
    	goto RtnFalse;
    if (ContinuationOffset>=0)
    {
        CurrentSeg=ContinuationOffset;
		goto RtnTrue;
    }
    if (!hDescBlock)
    {
        rtn = FindNextSegment2();  
{
#if ENABLETRACE
GSSiExitProg (1219);
#endif
        return rtn;
}
    }
Next:  
    if (idescblock >= NumDescBlocks)
    {
        rtn = FindNextSegment2();
        if (!rtn)
            goto RtnFalse;  
        idescblock = 0;
    } 
    pDescBlock = (LPDESCBLOCK)GlobalLock (hDescBlock); 
    
/*    {
    	short i,ii; 
    	LPDESCBLOCK	pDescBlock2=pDescBlock;
    	for (i=0;i<DescBlockLen/sizeof(DESCBLOCK);i++,pDescBlock2++)
    		if (pDescBlock2->Offset)
    			ii=1;
    }*/
    pDescBlock += ((long)QuadOff * (long)MaxQuadType * (long)NumDescBlocks + (long)QuadTypeNext* (long)NumDescBlocks + idescblock++); 
    desc = pDescBlock->Desc;
    CurrentSeg = pDescBlock->Offset;
    GlobalUnlock (hDescBlock); 
    if (CurrentSeg < 0)
    	goto Next;
    if (desc <= 0)
    {
        idescblock=100;  
        if (desc < 0) goto Next;  
        goto RtnTrue; 
    }
    if (!GetVisibility(desc))
        goto Next;
RtnTrue:
{
#if ENABLETRACE
GSSiExitProg (1219);
#endif
    return (TRUE);
}
RtnFalse:
{
#if ENABLETRACE
GSSiExitProg (1219);
#endif
    return (FALSE);
}
    
#if ENABLETRACE
}
#endif
}

BOOL FindNextSegment2 (void)
#if ENABLETRACE
{GSSiEnterProg (1220);
#endif
{   LPLONG pQuadOff;
    LPQUAD pQuad;    
	HPSTR       pQuadTree;
	LPSTR		pQuadOffset;  
    short	ii;

    ReorgOut2 (0,0);
    pQuadTree = (HPSTR)GlobalLock (hQuadTree);
    pQuadOffset  = GlobalLock (hQuadOffset);
    
    if (CheckQuad)
    {            
        HFILE   TraceFID;
        OFSTRUCTGM    OFStruct;  
        char    str[128];
        
        TraceFID = GSSiOpenFile ("quadchek.txt",&OFStruct,OF_READWRITE);
        if (TraceFID == HFILE_ERROR)  
            TraceFID = GSSiOpenFile ("quadchek.txt",&OFStruct,OF_CREATE); 
        sprintf (str,"%ld %ld %ld %ld",nBlocksRead,lBlocksRead,nBlocksIn,nBlocksOut); 
        GSSillseek (TraceFID,0,2);
        nBlocksRead=lBlocksRead=nBlocksIn=nBlocksOut=0;    
        fputstring (str,TraceFID);
        GSSiClose (TraceFID);
    }
        
    pQuadOff  = (LPLONG) (pQuadOffset + (QuadLevelAt-1)*4);
    QuadOff   = *pQuadOff;
    if (QuadOff < 0)
    	QuadOff = -QuadOff; 
    if (QuadOff > NumQuadSegs)
    	ii=1;
    pQuad     = (LPQUAD) (pQuadTree + ((long)(QuadOff-1))*LenQuadSeg);
S20:QuadTypeNext++;
    if (QuadTypeNext<MaxQuadType)
    {   if (!CurVis)
			goto S20;
        if (!CurVis->WantType[QuadTypeNext] && !(QuadTypeNext == 1 && CurVis->WantType[2]))
			goto S20;
        if (CurView->PassID == 2 && (QuadTypeNext != 0))
			goto S20;
        if (CurView->PassID == 3 && (QuadTypeNext == 0))
			goto S20;
        if (pQuad->TypeOffset[QuadTypeNext]<0) 
			goto S20; 
//if (QuadLevelAt < MaxQuadLevel) goto S20; // time test only!!
        QuadOffLoc = (long)(HPSTR)&pQuad->TypeOffset[QuadTypeNext] - (long)(HPSTR)pQuadTree + 18;
        CurrentSeg=pQuad->TypeOffset[QuadTypeNext];
        GlobalUnlock (hQuadTree);
        GlobalUnlock (hQuadOffset);
{
#if ENABLETRACE
GSSiExitProg (1220);
#endif
        return (TRUE);
}
    }

    NumQuadSegsProcessed++; 
    if (NumQuadSegsProcessed > 2049)
    	ii=1;
    QuadTypeNext = -1;
    QuadOff = pQuad->Next[0];
    QuadLevelAt++;
    if (QuadOff == NULLOFF)
    	goto S40;
    if (QuadOff > NumQuadSegs)
    	ii=1;

S30:pQuadOff  = (LPLONG) (pQuadOffset + (QuadLevelAt-1)*4);
    *pQuadOff = QuadOff;
    pQuad = (LPQUAD) (pQuadTree + ((long)(QuadOff-1))*LenQuadSeg);
    if (BlockInWindow(&pQuad->MinMax,0))
		goto S20;
S40:QuadLevelAt--;
    if (!QuadLevelAt)
    {   
        GlobalUnlock (hQuadTree);
        GlobalUnlock (hQuadOffset);
{
#if ENABLETRACE
GSSiExitProg (1220);
#endif
        return (FALSE);
}
    }
    pQuadOff  = (LPLONG) ( pQuadOffset + (QuadLevelAt-1)*4);
    QuadOff   = *pQuadOff;
    if (QuadOff < 0)
    	goto S40;
    *pQuadOff = -QuadOff;
    pQuad = (LPQUAD) (pQuadTree + ((long)(QuadOff-1))*LenQuadSeg);
    QuadOff = pQuad->Next[1];
    if (QuadOff == NULLOFF)
    	goto S40;
    if (QuadOff > NumQuadSegs)
    	ii=1;
    QuadLevelAt++;
    goto S30;

#if ENABLETRACE
}
#endif
}  


BOOL LoadQuadTree ()
#if ENABLETRACE
{GSSiEnterProg (1221);
#endif
{   
	long	nRead; 
    LPLONG	pint;  
    HPSTR   pQuadTree,pQuadTree2; 
	short	LQB;
    struct {long LQS, MQL, MQT, LQ;} ReadStruct; 

    GSSiGlobFree (&hQuadTree);
    GSSiGlobFree (&hQuadOffset);
	CurMinMax.xmn = 1;
	CurMinMax.xmx = -1;
    NumQuadSegsProcessed=0;
    idescblock = 100;
	LastQuadOff = -2;
    if (!PltName[0] || FidMap == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (1221);
#endif
    	return (FALSE);
}
    
    nRead = BigRead (FidMap,(HPSTR)&LQB,2);
    nRead = BigRead (FidMap,(HPSTR)&ReadStruct,sizeof(ReadStruct));
    
    LenQuadSeg = ReadStruct.LQS;
    MaxQuadLevel = ReadStruct.MQL;
    MaxQuadType = ReadStruct.MQT;
    LenQuad = ReadStruct.LQ; 
    NumQuadSegs = LenQuad/LenQuadSeg;
    if (!LenQuad)
{
#if ENABLETRACE
GSSiExitProg (1221);
#endif
    	return FALSE;
}
    hQuadTree = GSSiGlobAlloc ( 592,GMEM_MOVEABLE,(DWORD)LenQuad);
    pQuadTree = (HPSTR)GlobalLock (hQuadTree);   
    if (WantQuadTree2)
    {
        hQuadTree2 = GSSiGlobAlloc ( 593,GMEM_MOVEABLE,(DWORD)LenQuad);
        pQuadTree2 = GlobalLock (hQuadTree2);
        QuadTreeOffset = GSSillseek (FidMap,0,1); 
        CurMinMax.xmn=SHRT_MAX;
        CurMinMax.ymn=SHRT_MAX;
        CurMinMax.xmx=SHRT_MIN;
        CurMinMax.ymx=SHRT_MIN;
    }
    BigRead (FidMap,pQuadTree,LenQuad);
    if (hQuadTree2) 
    {
        hmemmove (pQuadTree2,pQuadTree,LenQuad);
        GlobalUnlock(hQuadTree2);
    }
    
    QuadLevelAt = 1;
    QuadTypeNext = -1;
    GlobalUnlock (hQuadTree);
    hQuadOffset=GSSiGlobAlloc ( 594,GHND,(DWORD)((MaxQuadLevel+1)*sizeof(long)));
    pint = (LPLONG)GlobalLock(hQuadOffset);
    *pint = 1;
    GlobalUnlock(hQuadOffset);
{
#if ENABLETRACE
GSSiExitProg (1221);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL SaveQuadTree (void)
#if ENABLETRACE
{GSSiEnterProg (1222);
#endif
{    
    LPSHORT pint;  
    HPSTR   pQuadTree;

#pragma pack(1)
    struct {USHORT LQB; long LQS, MQL, MQT, LQ;} ReadStruct;
#pragma pack()

    if (!PltName[0] || FidMap == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (1222);
#endif
    	return (FALSE);
}
    
    ReadStruct.LQB=0;
    ReadStruct.LQS=LenQuadSeg;
    ReadStruct.MQL=MaxQuadLevel;
    ReadStruct.MQT=MaxQuadType;
    ReadStruct.LQ=LenQuad;
    GSSillseek(FidMap,GraphicsOffset,0);
    BigWrite (FidMap,(HPSTR)&ReadStruct,sizeof(ReadStruct),GraphicsOffset);
    pQuadTree = GlobalLock (hQuadTree); 
    BigWrite (FidMap,pQuadTree,LenQuad,-1); 
    GlobalUnlock (hQuadTree);
{
#if ENABLETRACE
GSSiExitProg (1222);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL AnyDataInMap (void)
#if ENABLETRACE
{GSSiEnterProg (1223);
#endif
{   
	HPSTR	pQuadTree;
	long	i,j; 
	BOOL	rtn=FALSE;
    LPQUAD  pQuad;
	
    pQuadTree = (HPSTR)GlobalLock (hQuadTree);   
	for (i=0;i<NumQuadSegs;i++)
	{
    	pQuad     = (LPQUAD)(pQuadTree + ((long)(i))*LenQuadSeg);  
		for (j=0;j<MaxQuadType;j++)
		{
        	if (pQuad->TypeOffset[j]>=0)
        	{
        		rtn=TRUE; 
        		goto Exit;
	       	}  
	    }
    }
Exit:
    GlobalUnlock (hQuadTree);
{
#if ENABLETRACE
GSSiExitProg (1223);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}    


long AddSimpleQuadTree (HFILE Fid)
#if ENABLETRACE
{GSSiEnterProg (1224);
#endif
{   
    QUAD    Quad; 
	int		numQuadSeg = 1;//31;
    long    LenQuadSeg, MaxQuadLevel,MaxQuadType,LenQuad;
    long    NewLoc;    
    short     lMem, i, i2;
    
	MaxQuadType = max (2,MaxNewType)+1;
    LenQuadSeg = 8 + 4 + 4 * MaxQuadType;
    MaxQuadLevel = 4;
    LenQuad = LenQuadSeg*numQuadSeg;
     
    NewLoc = GSSillseek (Fid,0,1); 
    lMem = LenQuad + 4*4;
    BigWrite (Fid,(char *)&lMem,2,-1);
    BigWrite (Fid,(char *)&LenQuadSeg,4,-1);
    BigWrite (Fid,(char *)&MaxQuadLevel,4,-1);
    BigWrite (Fid,(char *)&MaxQuadType,4,-1);
    BigWrite (Fid,(char *)&LenQuad,4,-1);
    
//1 
    Quad.MinMax.xmn = SHRT_MIN;
    Quad.MinMax.xmx = SHRT_MAX;
    Quad.MinMax.ymn = SHRT_MIN;
    Quad.MinMax.ymx = SHRT_MAX;
    Quad.Next[0] = NULLOFF;//2;
    Quad.Next[1] = NULLOFF;//3;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1); 
	goto Exit;
//2 
    Quad.MinMax.xmn = SHRT_MIN;
    Quad.MinMax.xmx = 0;
    Quad.MinMax.ymn = SHRT_MIN;
    Quad.MinMax.ymx = SHRT_MAX;
    Quad.Next[0] = 4;
    Quad.Next[1] = 5;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1);
//3 
    Quad.MinMax.xmn = 0;
    Quad.MinMax.xmx = SHRT_MAX;
    Quad.MinMax.ymn = SHRT_MIN;
    Quad.MinMax.ymx = SHRT_MAX;
    Quad.Next[0] = 6;
    Quad.Next[1] = 7;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1); 
//4 
    Quad.MinMax.xmn = SHRT_MIN;
    Quad.MinMax.xmx = 0;
    Quad.MinMax.ymn = SHRT_MIN;
    Quad.MinMax.ymx = 0;
    Quad.Next[0] = 8;
    Quad.Next[1] = 9;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1); 
//5 
    Quad.MinMax.xmn = SHRT_MIN;
    Quad.MinMax.xmx = 0;
    Quad.MinMax.ymn = 0;
    Quad.MinMax.ymx = SHRT_MAX;
    Quad.Next[0] = 10;
    Quad.Next[1] = 11;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1);
//6 
    Quad.MinMax.xmn = 0;
    Quad.MinMax.xmx = SHRT_MAX;
    Quad.MinMax.ymn = SHRT_MIN;
    Quad.MinMax.ymx = 0;
    Quad.Next[0] = 12;
    Quad.Next[1] = 13;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1);  
//7 
    Quad.MinMax.xmn = 0;
    Quad.MinMax.xmx = SHRT_MAX;
    Quad.MinMax.ymn = 0;
    Quad.MinMax.ymx = SHRT_MAX;
    Quad.Next[0] = 14;
    Quad.Next[1] = 15;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1);   
//8 
    Quad.MinMax.xmn = SHRT_MIN;
    Quad.MinMax.xmx = SHRT_MIN/2;
    Quad.MinMax.ymn = SHRT_MIN;
    Quad.MinMax.ymx = 0;
    Quad.Next[0] = 16;
    Quad.Next[1] = 17;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1); 
//9 
    Quad.MinMax.xmn = SHRT_MIN/2;
    Quad.MinMax.xmx = 0;
    Quad.MinMax.ymn = SHRT_MIN;
    Quad.MinMax.ymx = 0;
    Quad.Next[0] = 18;
    Quad.Next[1] = 19;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1); 
//10    
    Quad.MinMax.xmn = SHRT_MIN;
    Quad.MinMax.xmx = SHRT_MIN/2;
    Quad.MinMax.ymn = 0;
    Quad.MinMax.ymx = SHRT_MAX;
    Quad.Next[0] = 20;
    Quad.Next[1] = 21;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1);
//11    
    Quad.MinMax.xmn = SHRT_MIN/2;
    Quad.MinMax.xmx = 0;
    Quad.MinMax.ymn = 0;
    Quad.MinMax.ymx = SHRT_MAX;
    Quad.Next[0] = 22;
    Quad.Next[1] = 23;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1);
//12    
    Quad.MinMax.xmn = 0;
    Quad.MinMax.xmx = SHRT_MAX/2;
    Quad.MinMax.ymn = SHRT_MIN;
    Quad.MinMax.ymx = 0;
    Quad.Next[0] = 24;
    Quad.Next[1] = 25;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1);  
//13    
    Quad.MinMax.xmn = SHRT_MAX/2;
    Quad.MinMax.xmx = SHRT_MAX;
    Quad.MinMax.ymn = SHRT_MIN;
    Quad.MinMax.ymx = 0;
    Quad.Next[0] = NULLOFF;
    Quad.Next[1] = NULLOFF;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1);  
//14    
    Quad.MinMax.xmn = 0;
    Quad.MinMax.xmx = SHRT_MAX/2;
    Quad.MinMax.ymn = 0;
    Quad.MinMax.ymx = SHRT_MAX;
    Quad.Next[0] = 28;
    Quad.Next[1] = 29;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1);   
//15    
    Quad.MinMax.xmn = SHRT_MAX/2;
    Quad.MinMax.xmx = SHRT_MAX;
    Quad.MinMax.ymn = 0;
    Quad.MinMax.ymx = SHRT_MAX;
    Quad.Next[0] = NULLOFF;
    Quad.Next[1] = NULLOFF;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1);   
//16    
    Quad.MinMax.xmn = SHRT_MIN;
    Quad.MinMax.xmx = SHRT_MIN/2;
    Quad.MinMax.ymn = SHRT_MIN;
    Quad.MinMax.ymx = SHRT_MIN/2;
    Quad.Next[0] = NULLOFF;
    Quad.Next[1] = NULLOFF;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1); 
//17    
    Quad.MinMax.xmn = SHRT_MIN;
    Quad.MinMax.xmx = SHRT_MIN/2;
    Quad.MinMax.ymn = SHRT_MIN/2;
    Quad.MinMax.ymx = 0;
    Quad.Next[0] = NULLOFF;
    Quad.Next[1] = NULLOFF;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1); 
//18    
    Quad.MinMax.xmn = SHRT_MIN/2;
    Quad.MinMax.xmx = 0;
    Quad.MinMax.ymn = SHRT_MIN;
    Quad.MinMax.ymx = SHRT_MIN/2;
    Quad.Next[0] = NULLOFF;
    Quad.Next[1] = NULLOFF;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1); 
//19    
    Quad.MinMax.xmn = SHRT_MIN/2;
    Quad.MinMax.xmx = 0;
    Quad.MinMax.ymn = SHRT_MIN/2;
    Quad.MinMax.ymx = 0;
    Quad.Next[0] = NULLOFF;
    Quad.Next[1] = NULLOFF;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1); 
//20    
    Quad.MinMax.xmn = SHRT_MIN;
    Quad.MinMax.xmx = SHRT_MIN/2;
    Quad.MinMax.ymn = 0;
    Quad.MinMax.ymx = SHRT_MAX/2;
    Quad.Next[0] = NULLOFF;
    Quad.Next[1] = NULLOFF;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1);
//21    
    Quad.MinMax.xmn = SHRT_MIN;
    Quad.MinMax.xmx = SHRT_MIN/2;
    Quad.MinMax.ymn = SHRT_MAX/2;
    Quad.MinMax.ymx = SHRT_MAX;
    Quad.Next[0] = NULLOFF;
    Quad.Next[1] = NULLOFF;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1);
//22    
    Quad.MinMax.xmn = SHRT_MIN/2;
    Quad.MinMax.xmx = 0;
    Quad.MinMax.ymn = 0;
    Quad.MinMax.ymx = SHRT_MAX/2;
    Quad.Next[0] = NULLOFF;
    Quad.Next[1] = NULLOFF;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1);
//23    
    Quad.MinMax.xmn = SHRT_MIN/2;
    Quad.MinMax.xmx = 0;
    Quad.MinMax.ymn = SHRT_MAX/2;
    Quad.MinMax.ymx = SHRT_MAX;
    Quad.Next[0] = NULLOFF;
    Quad.Next[1] = NULLOFF;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1);
//24    
    Quad.MinMax.xmn = 0;
    Quad.MinMax.xmx = SHRT_MAX/2;
    Quad.MinMax.ymn = SHRT_MIN;
    Quad.MinMax.ymx = SHRT_MIN/2;
    Quad.Next[0] = NULLOFF;
    Quad.Next[1] = NULLOFF;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1);  
//25    
    Quad.MinMax.xmn = 0;
    Quad.MinMax.xmx = SHRT_MAX/2;
    Quad.MinMax.ymn = SHRT_MIN/2;
    Quad.MinMax.ymx = 0;
    Quad.Next[0] = NULLOFF;
    Quad.Next[1] = NULLOFF;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1);  
//26    
    Quad.MinMax.xmn = SHRT_MAX/2;
    Quad.MinMax.xmx = SHRT_MAX;
    Quad.MinMax.ymn = SHRT_MIN;
    Quad.MinMax.ymx = SHRT_MIN/2;
    Quad.Next[0] = NULLOFF;
    Quad.Next[1] = NULLOFF;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1);  
//27    
    Quad.MinMax.xmn = SHRT_MAX/2;
    Quad.MinMax.xmx = SHRT_MAX;
    Quad.MinMax.ymn = SHRT_MIN/2;
    Quad.MinMax.ymx = 0;
    Quad.Next[0] = NULLOFF;
    Quad.Next[1] = NULLOFF;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1);  
//28    
    Quad.MinMax.xmn = 0;
    Quad.MinMax.xmx = SHRT_MAX/2;
    Quad.MinMax.ymn = 0;
    Quad.MinMax.ymx = SHRT_MAX/2;
    Quad.Next[0] = NULLOFF;
    Quad.Next[1] = NULLOFF;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1);   
//29    
    Quad.MinMax.xmn = 0;
    Quad.MinMax.xmx = SHRT_MAX/2;
    Quad.MinMax.ymn = SHRT_MAX/2;
    Quad.MinMax.ymx = SHRT_MAX;
    Quad.Next[0] = NULLOFF;
    Quad.Next[1] = NULLOFF;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1);   
//30    
    Quad.MinMax.xmn = SHRT_MAX/2;
    Quad.MinMax.xmx = SHRT_MAX;
    Quad.MinMax.ymn = 0;
    Quad.MinMax.ymx = SHRT_MAX/2;
    Quad.Next[0] = NULLOFF;
    Quad.Next[1] = NULLOFF;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1);   
//31    
    Quad.MinMax.xmn = SHRT_MAX/2;
    Quad.MinMax.xmx = SHRT_MAX;
    Quad.MinMax.ymn = SHRT_MAX/2;
    Quad.MinMax.ymx = SHRT_MAX;
    Quad.Next[0] = NULLOFF;
    Quad.Next[1] = NULLOFF;
    for (i=0;i<MaxQuadType;i++)
        Quad.TypeOffset[i]=-1;
    BigWrite (Fid,(char *)&Quad,(UINT)LenQuadSeg,-1);   
    
Exit:  
    i2 = 0;  
    BigWrite (Fid,(char *)&i2,2,-1); 
{
#if ENABLETRACE
GSSiExitProg (1224);
#endif
    return NewLoc;
}
#if ENABLETRACE
}
#endif
}                   


