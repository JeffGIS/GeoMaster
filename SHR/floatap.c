#include "shr.h"
#include "bt.h"

#define	NVALIDCODES	12

#include "gmextern.h"

static short	ValidCodes[NVALIDCODES]={'==','<','>','!=','!<','!>','<>','>=','<=','&&','||','!'};
static char		LogVals[2][2]={"0","1"}; 
static char		SQLCodes[11][6]={" AND "," OR "," = "," NOT "," >= "," <= "," <> "," < "," !< "," > "," !> "}; 
static char		GSSCodes[11][4]={"&&"   ,"||"  ,"==" ,"!"    ,">="  ,"<="  ,"!="  ,"<"  ,"!<"  ,">"  ,"!>"  };
    

void ConvertSQLToLogicP (LPSTR pLogicP,LPSTR pSQL)
{   
	short	i;
	LPSTR	pLoc;
	
	_fstrcpy (pLogicP,pSQL); 
	if (!*pLogicP)
		return;
	for (i=0;i<11;i++)
		REPLAC (pLogicP,SQLCodes[i],GSSCodes[i],4096); 
	REPLAC (pLogicP," =","=",4096); 
	REPLAC (pLogicP,"= ","=",4096); 
	pLoc = pLogicP+1;
	while (*pLoc)
	{
		if (*pLoc == '=')
		{
			if (*(pLoc-1) == '!' ||
				*(pLoc-1) == '>' ||
				*(pLoc-1) == '<' ||
				*(pLoc-1) == '>')
				pLoc++;
			else if (*(pLoc+1) == '=')
				pLoc += 2;
			else
			{
				i=_fstrlen (pLoc);
				_fmemmove (pLoc+1,pLoc,i+1);
				*pLoc = '=';
				pLoc += 2;
			}
		}
		else
			pLoc++;
	}
	return;
}

BOOL LogicP(LPSTR INEXPR, LPBOOL IRC)
{
	BOOL rtn = LogicPBP(INEXPR, IRC,0,0);
	return rtn;
}

BOOL LogicPBP (LPSTR INEXPR,LPBOOL IRC,LPSHORT pBrkPt,int lenBP)
#if ENABLETRACE
{GSSiEnterProg (1190);
#endif
{ 
// IRC (0=success, 1 = invalid flt value, 2 = error in paren matching, 3 = invalid statement, 4 = devide by 0, 5=invalid op code)  
    LPSTR	pLchr, pVal1Begin, pVal1End, pVal2Begin, pVal2End, pEnd; 
    HANDLE	hInput=GSSiGlobAlloc ( 744,GMEM_MOVEABLE,4096);
    LPSTR	INPUT = GlobalLock (hInput); 
    UINT	OpCode, OpCodeNext=0;  
    BOOL	rtn=0, Value1, Value2, NeedVal1, Error;  
    short	InBrackets=0;
    short	Inc; 
    BOOL	Literal=FALSE;  
    char	NullVal[2]="";

	*IRC = 0;  
	_fstrcpy (INPUT,INEXPR);
	Truncate (INPUT);  
// remove enclosing parens 
/*	if (*INPUT == '!')
	{
		INPUT++;
		rtn = LogicP (INPUT,IRC);
		rtn = !rtn;
		goto Exit;
	}*/
	if (*INPUT == '(')
	{
		pLchr = LastChr (INPUT);
    	pEnd = MatchLev ((LPSTR)(INPUT+1),')');
    	if (pEnd == pLchr)
    	{
	    	INPUT++;
	    	*pLchr-- = 0;
	    }
    }

// find first value 
	pVal1Begin = pVal1End = 0; 
NextVal1: 
	if (!Literal) 
    switch (*INPUT)
    {   
    	case 0:
    		if (!pVal1Begin)
    		{
    			rtn = 0;
    			goto Exit;
    		} 
    		if (pVal1End)
    			*pVal1End = 0; 
    		rtn = ComputeLogValue (pVal1Begin,NULL,0,IRC);
    		goto Exit; 
    	case '@':
    		Literal = TRUE;
    		goto Next1;
    	case '!':
			if (!strcspn (INPUT+1,"=<>"))
				break;
			if (!pVal1Begin)
				pVal1Begin = INPUT;
    		goto Next1;
    	case '[':
			if (!pVal1Begin)
				pVal1Begin = INPUT;
    		InBrackets++;
    		goto Next1;
    	case ']':
    		InBrackets--; 
    		goto Next1;  
    	default:
    		break;
    }
	if (!InBrackets)  
	{
		if (Literal)
		{
			if (!pVal1Begin)
				pVal1Begin = INPUT;
		}
		else
		    switch (*INPUT)
		    {   
		    	case ' ':  
		    		if (pVal1Begin)
		    			pVal1End = INPUT;
		    		break; 
		    	case '(':
		    		if (!pVal1Begin)
		    			pVal1Begin = INPUT;
		    		INPUT++;
		    		if (!(pEnd = MatchLev (INPUT,')')))
		    		{
		    			*IRC = 2;
		    			goto Exit;
		    		}
		    		INPUT = pEnd;
		    		break;
				default: 
					OpCode = GetBoolOpCode (INPUT,&Inc,&NeedVal1,&Error);
					if (Error)
					{
						*IRC = 5;
						goto Exit;
					}
					if (OpCode)
					{   
			    		if (pVal1Begin)
			    			pVal1End = INPUT; 
			    		else if (NeedVal1)
			    		{
			    			*IRC = 3;
			    			goto Exit;
			    		}
					    INPUT += Inc;
			    		goto GetSecondValue;
					}
					if (!pVal1Begin) 
					{
						pVal1Begin = INPUT;
						if (*pVal1Begin == '\'')
						{
				    		if ((pEnd = _fstrchr (INPUT+1,'\'')))
				    		{
				    			INPUT = pEnd;
				    		}
						}
					}
					break;
		    } 
	}
    Literal = FALSE;
Next1:
    INPUT++;
    goto NextVal1;
    		
GetSecondValue:   
	InBrackets = 0;
	pVal2Begin = 0;
NextOp:
	pVal2End = 0; 
NextVal2:
	if (!Literal)  
    switch (*INPUT)
    {   
    	case 0:
    		if (!pVal2Begin) 
    			pVal2Begin = NullVal;
/*    		{
    			*IRC = 3;
    			goto Exit;
    		}*/
    		if (pVal1End) 
				*pVal1End = 0;
    		rtn = ComputeLogValue (pVal1Begin,pVal2Begin,OpCode,IRC);
    		goto Exit;
    	case '@':
    		Literal = TRUE;
    		goto Next2;
    	case '!':
			if (!strcspn (INPUT+1,"=<>"))
				break;
			if (!pVal2Begin)
				pVal2Begin = INPUT;
    		goto Next2;
    	case '[':
			if (!pVal2Begin)
				pVal2Begin = INPUT;
    		InBrackets++;
    		goto Next2;
    	case ']':
    		InBrackets--; 
    		goto Next2;  
    	default:
    		break;
    }
    if (!InBrackets)
    {
    	if (Literal)
    	{
			if (!pVal2Begin)
				pVal2Begin = INPUT; 
		}
		else
		    switch (*INPUT)
		    {   
		    	case ' ':  
		    		if (pVal2Begin)
		    			pVal2End = INPUT;
		    		break; 
		    	case '(':
		    		if (!pVal2Begin)
		    			pVal2Begin = INPUT;
		    		INPUT++;
		    		if (!(pEnd = MatchLev (INPUT,')')))
		    		{
		    			*IRC = 2;
		    			goto Exit;
		    		}
		    		INPUT = pEnd;
		    		break;
				default: 
					OpCodeNext = GetBoolOpCode (INPUT,&Inc,&NeedVal1,&Error);
					if (Error)
					{
						*IRC = 5;
						goto Exit;
					}
					if (OpCodeNext)
					{   
			    		if (pVal2Begin)
			    			pVal2End = INPUT; 
			    		else if (NeedVal1)
			    		{
			    			*IRC = 3;
			    			goto Exit;
			    		}
					    INPUT += Inc;
			    		goto CheckEndSecondValue;
					}
					if (!pVal2Begin) 
					{
						pVal2Begin = INPUT;  
						if (*pVal2Begin == '\'')
						{
				    		if ((pEnd = _fstrchr (INPUT+1,'\'')))
				    		{
				    			INPUT = pEnd;
				    		}
						}
					}
					break;
		    }
	} 
    Literal = FALSE;
Next2:
    INPUT++;
    goto NextVal2;
    		
CheckEndSecondValue:
    
	if (OpRankBool(OpCodeNext) == OpRankBool(OpCode))
    {   
    	if (!pVal1Begin || !pVal2Begin)  
    	{
    		*IRC = 3;
    		goto Exit; 
    	}
		*pVal1End = 0;
		*pVal2End = 0;
	    Value2 = ComputeLogValue (pVal1Begin,pVal2Begin,OpCode,IRC); 
	    if (*IRC)
	    	goto Exit;
		pVal1Begin = (LPSTR)&LogVals[Value2]; 
		OpCode = OpCodeNext;
		goto GetSecondValue;
	} 
	if (OpRankBool(OpCodeNext) > OpRankBool(OpCode))
	{
		pVal1End = pVal2End;
		OpCode = OpCodeNext;
		goto GetSecondValue;
	}
	goto NextVal2;

Exit:		
	GSSiGlobUlFree (&hInput);
{
#if ENABLETRACE
GSSiExitProg (1190);
#endif
	return rtn; 		
}
#if ENABLETRACE
}
#endif
} 

UINT GetBoolOpCode (LPSTR Input,LPSHORT pInc,LPBOOL pNeedArg1,LPBOOL pError)
#if ENABLETRACE
{GSSiEnterProg (1191);
#endif
{    
	UINT	OpCode,i;
	
    *pInc = 0;
    OpCode = *Input;
    *pError = FALSE;
	switch (OpCode) 
	{
    	case '!':
    	case '>':
    	case '<':
    	case '=':
    	case '&':
    	case '|':
    		(*pInc)++;
    		Input++;
    		switch (*Input)
    		{
		    	case '>':
		    	case '<':
		    	case '=':
		    	case '&':
		    	case '|':
		    		OpCode = (UINT)*Input + (OpCode << 8);   
		    		*pNeedArg1 = TRUE;
		    		(*pInc)++;
		    		break;
		    	default:
		    		*pNeedArg1 = FALSE;
		    }
		    *pError = TRUE;
		    for (i=0;i<NVALIDCODES;i++)
		    	if (OpCode == ValidCodes[i])
		    		*pError = FALSE;
{
#if ENABLETRACE
GSSiExitProg (1191);
#endif
		    return OpCode;
}
    	default:
{
#if ENABLETRACE
GSSiExitProg (1191);
#endif
    		return 0;
}
    }
#if ENABLETRACE
}
#endif
}


double fltread (LPSTR Instr, LPBOOL IRC)
#if ENABLETRACE
{GSSiEnterProg (1192);
#endif
{
    LPSTR lpEnd, lpComma;
    double	rtn=0, val, val1, val2;  
    HANDLE	hInput=GSSiGlobAlloc ( 745,GMEM_MOVEABLE,USHRT_MAX);
    LPSTR	str = GlobalLock (hInput); 
    
    _fstrcpy (str,Instr);
//    if (AllowFltExpand)
    	ExpandText (str);
    Truncate (str);

   	lpEnd = LastChr (str); 
   	if (*lpEnd == ')')
   	{
	    if (!_fstrnicmp (str,"-(",2))
	    {
	    	str+=2;
	    	*lpEnd = 0;
	    	val = FltAP (str,IRC);
	    	if (!*IRC)
	    		rtn = -val;
	    }
	    else if (!_fstrnicmp (str,"+(",2))
	    {
	    	str+=2;
	    	*lpEnd = 0;
	    	val = FltAP (str,IRC);
	    	if (!*IRC)
	    		rtn = val;
	    }
	    else if (!_fstrnicmp (str,"sin(",4))
	    {
	    	str+=4;
	    	*lpEnd = 0;
	    	val = FltAP (str,IRC);
	    	if (!*IRC)
	    		rtn = sin (val);
	    }
	    else if (!_fstrnicmp (str,"cos(",4))
	    {
	    	str+=4;
	    	*lpEnd = 0;
	    	val = FltAP (str,IRC);
	    	if (!*IRC)
	    		rtn = cos (val);
	    }
	    else if (!_fstrnicmp (str,"tan(",4))
	    {
	    	str+=4;
	    	*lpEnd = 0;
	    	val = FltAP (str,IRC);
	    	if (!*IRC)
	    		rtn = tan (val);
	    }
	    else if (!_fstrnicmp (str,"cot(",4))
	    {
	    	str+=4;
	    	*lpEnd = 0;
	    	val = FltAP (str,IRC);
	    	if (!*IRC)
	    		rtn = 1.0/tan (val);
	    }
	    else if (!_fstrnicmp (str,"log(",4))
	    {
	    	str+=4;
	    	*lpEnd = 0;
	    	val = FltAP (str,IRC);
	    	if (!*IRC)
	    		rtn = log10 (val);
	    }
	    else if (!_fstrnicmp (str,"sqrt(",5))
	    {
	    	str+=5;
	    	*lpEnd = 0;
	    	val = FltAP (str,IRC);
	    	if (!*IRC)
	    		rtn = sqrt (val);
	    }
	    else if (!_fstrnicmp (str,"abs(",4))
	    {
	    	str+=4;
	    	*lpEnd = 0;
	    	val = FltAP (str,IRC);
	    	if (!*IRC)
	    		rtn = fabs (val);
	    }
	    else if (!_fstrnicmp (str,"nint(",5))
	    {
	    	str+=5;
	    	*lpEnd = 0;
	    	val = FltAP (str,IRC);
	    	if (!*IRC)
	    		rtn = IDNINT (val);
	    }
	    else if (!_fstrnicmp (str,"int(",4))
	    {
	    	str+=4;
	    	*lpEnd = 0;
	    	val = FltAP (str,IRC);
	    	if (!*IRC)
	    		rtn = floor (val);
	    }
	    else if (!_fstrnicmp (str,"deg(",4))
	    {
	    	str+=4;
	    	*lpEnd = 0;
	    	val = FltAP (str,IRC);
	    	if (!*IRC)
	    		rtn = RADDEG * val;
	    }
	    else if (!_fstrnicmp (str,"rad(",4))
	    {
	    	str+=4;
	    	*lpEnd = 0;
	    	val = FltAP (str,IRC);
	    	if (!*IRC)
	    		rtn = val/RADDEG;
	    }
	    else if (!_fstrnicmp (str,"acos(",5))
	    {
	    	str+=5;
	    	*lpEnd = 0;
	    	val = FltAP (str,IRC);
	    	if (!*IRC)
	    		rtn = acos (val);
	    }
	    else if (!_fstrnicmp (str,"asin(",5))
	    {
	    	str+=5;
	    	*lpEnd = 0;
	    	val = FltAP (str,IRC);
	    	if (!*IRC)
	    		rtn = asin (val);
	    }
	    else if (!_fstrnicmp (str,"mod(",4))
	    {
	    	str+=4;
	    	*lpEnd = 0;
    		lpComma = MatchLev (str,',');
    		if (!lpComma)
    		{
    			*IRC = 3;
{
#if ENABLETRACE
GSSiExitProg (1192);
#endif
    			return 0;
}
    		}
    		*lpComma++ = 0;
	    	val1 = FltAP (str,IRC); 
	    	if (!*IRC)
	    		val2 = FltAP (lpComma,IRC);
	    	if (!*IRC)
	    		rtn = fmod (val1,val2);
	    }
	    else if (!_fstrnicmp (str,"min(",4))
	    {
	    	str+=4;
	    	*lpEnd = 0;
    		lpComma = MatchLev (str,',');
    		if (!lpComma)
    		{
    			*IRC = 3;
{
#if ENABLETRACE
GSSiExitProg (1192);
#endif
    			return 0;
}
    		}
    		*lpComma++ = 0;
	    	val1 = FltAP (str,IRC); 
	    	if (!*IRC)
	    		val2 = FltAP (lpComma,IRC);
	    	if (!*IRC)
	    		rtn = min (val1,val2);
	    }
	    else if (!_fstrnicmp (str,"max(",4))
	    {
	    	str+=4;
	    	*lpEnd = 0;
    		lpComma = MatchLev (str,',');
    		if (!lpComma)
    		{
    			*IRC = 3;
{
#if ENABLETRACE
GSSiExitProg (1192);
#endif
    			return 0;
}
    		}
    		*lpComma++ = 0;
	    	val1 = FltAP (str,IRC); 
	    	if (!*IRC)
	    		val2 = FltAP (lpComma,IRC);
	    	if (!*IRC)
	    		rtn = max (val1,val2);
	    }
	    else if (!_fstrnicmp (str,"atan2(",6))
	    {
	    	str+=6;
	    	*lpEnd = 0;
    		lpComma = MatchLev (str,',');
    		if (!lpComma)
    		{
    			*IRC = 3;
{
#if ENABLETRACE
GSSiExitProg (1192);
#endif
    			return 0;
}
    		}
    		*lpComma++ = 0;
	    	val1 = FltAP (str,IRC); 
	    	if (!*IRC)
	    		val2 = FltAP (lpComma,IRC);
	    	if (!*IRC)
	    		rtn = atan2 (val1,val2);
	    }
	    else
	    	*IRC = 1;
	}
	else
	{
	    rtn = strtod (str,&lpEnd);
	    if (*lpEnd)
	    	*IRC = 1;
	    else
	    	*IRC = 0; 
	}
    GSSiGlobUlFree (&hInput);
{
#if ENABLETRACE
GSSiExitProg (1192);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL ValToFileVal (LPSTR InVal) //returns TRUE if character field
{   
	short	i;
	LPFIELDINFO	pFldInfo;
	LPOPENFILEDATA	FilePtr; 
	char	str[256];
	char	Val[128];
	
	if (!LogicPSQLPtr)
		return FALSE;
	strcpy (Val,InVal);
	Strip (Val,'"');
	FilePtr = (LPOPENFILEDATA)GlobalLock (LogicPSQLPtr->OFHandle); 
	for (i=0,pFldInfo=&FilePtr->FldInfo;i<FilePtr->NumFields;i++,pFldInfo++)
	{
		if (!_fstricmp (Val,pFldInfo->name))
		{
			short	l=_fstrlen(Val);
			
			if (*LogicPSQLPtr->IDName)
				sprintf (str,"[%s.%s]",LogicPSQLPtr->IDName,Val);
			else
				sprintf (str,"[%s]",Val);
			_fstrcpy (InVal,str);
			GlobalUnlock (LogicPSQLPtr->OFHandle);
			return (pFldInfo->type == BT_CHAR);
		}
	} 
	GlobalUnlock (LogicPSQLPtr->OFHandle);
	return FALSE;
}

BOOL ComputeLogValue (LPSTR Inval1, LPSTR Inval2, UINT OpCode,LPBOOL IRC)
#if ENABLETRACE
{GSSiEnterProg (1193);
#endif
{   
	BOOL	irc1=0, irc2=0;
	short	rtn=0, ival;
	BOOL	rc;
	double	dval1=0, dval2=0; 
	HANDLE	hVals=GSSiGlobAlloc ( 746,GMEM_MOVEABLE,USHRT_MAX); 
	LPSTR	val1=GlobalLock (hVals), val2=val1+4096;
	
	*val1 = 0;
	*val2 = 0; 
	if (hLogicPStatements && nLogicPStatements < 512 && Inval1 && Inval2)
	{
		LPHANDLE	phStatement = (LPHANDLE)GlobalLock (hLogicPStatements); 
		LPLOGICPSTATEMENT	pStatement;
		
		phStatement[nLogicPStatements] = GSSiGlobAlloc ( 747,GMEM_MOVEABLE,sizeof(LOGICPSTATEMENT));
		pStatement = (LPLOGICPSTATEMENT)GlobalLock (phStatement[nLogicPStatements]);    
		_fstrcpy (pStatement->Arg1,Inval1); 
		if (Inval2)
			_fstrcpy (pStatement->Arg2,Inval2);
		pStatement->OpCode = OpCode;
		GlobalUnlock (phStatement[nLogicPStatements++]); 
		GlobalUnlock (hLogicPStatements);
	}
	if (Inval1)
	{
		if (!_fstricmp (Inval1,"NULL"))
			irc1 = 1;
		else
		{
			short l=_fstrlen (Inval1);
			
			if (l>1 && *Inval1 == '\'' && Inval1[l-1] == '\'')
			{
				Inval1++;
				_fstrcpy (val1,Inval1);
				*LastChr (val1) = 0; 
				irc1 = 1;
			}
			else 
			{
				_fstrcpy (val1,Inval1); 
				irc1 = ValToFileVal (val1); 
			}
			if (OpCode == '&&' || OpCode == '||' || (!OpCode && (*val1 == '!' || *val1 == '(')))
			{
				BOOL	Invert = FALSE;

				if (*val1 == '!')
				{
					Invert = TRUE;
					val1++;
				}
				ival = LogicP (val1,&rc);
				if (Invert)
					ival = !ival;
				if (!rc)
					val1 = (LPSTR)&LogVals[ival];
			} 
			else if (!hLogicPStatements)
				ExpandText (val1);
			dval1 = fltread (val1,&irc1); 
		}
	}
	if (Inval2)
	{   
		if (!_fstricmp (Inval2,"NULL"))
			irc2 = 1;
		else
		{
			short l=_fstrlen (Inval2);
			
			if (l>1 && *Inval2 == '\'' && Inval2[l-1] == '\'')
			{
				Inval2++;
				_fstrcpy (val2,Inval2);
				*LastChr (val2) = 0; 
				irc2 = 1;
			}
			else 
			{
				_fstrcpy (val2,Inval2);
				irc2 = ValToFileVal (val2);
			}
			if (OpCode == '&&' || OpCode == '||' || OpCode == '!')
			{
				BOOL	Invert = FALSE;

				if (*val2 == '!')
				{
					Invert = TRUE;
					val2++;
				}

				ival = LogicP (val2,&rc);
				if (Invert)
					ival = !ival;
				if (!rc)
					val2 = (LPSTR)&LogVals[ival];
			} 
			else if (!hLogicPStatements)
				ExpandText (val2); 
		} 
    }
	switch (OpCode)
	{   
		case '&&':
		if (!_fstrcmp (val1,"1") && !_fstrcmp (val2,"1"))
			rtn = 1;
			goto Exit;
		case '||':
		if (!_fstrcmp (val1,"1") || !_fstrcmp (val2,"1"))
			rtn = 1;
			goto Exit;
	}
	
	if (!irc2 && Inval2 && *Inval2)
	{
		dval2 = fltread (val2,&irc2);
		*IRC = 0; 
	}
	
	if (irc1 || irc2)
	switch (OpCode)
	{   
		case 0:
			rtn = atob(val1);
			break;
		case '!':
			if (!*val2 || !_fstrcmp (val2,"0"))
				rtn = 1;
			break;
		case '<':
			if (_fstrcmp (val1,val2) < 0)
				rtn = 1;
			break;
		case '>':
			if (_fstrcmp (val1,val2) > 0)
				rtn = 1;
			break;
		case '==':
			if (!_fstrcmp (val1,val2))
				rtn = 1;
			break;
		case '<>':
		case '!=':
			if (_fstrcmp (val1,val2))
				rtn = 1;
			break;
		case '!>':
			if (_fstrcmp (val1,val2) <= 0)
				rtn = 1;
			break;
		case '!<':
			if (_fstrcmp (val1,val2) >= 0)
				rtn = 1;
			break;
		case '>=':
			if (_fstrcmp (val1,val2) >= 0)
				rtn = 1;
			break;
		case '<=':
			if (_fstrcmp (val1,val2) <= 0)
				rtn = 1;
			break;
		case 'a':
			if (*val1 == '1' && *val2 == '1')
				rtn = 1;
			break;
	}
	else
	switch (OpCode)
	{   
		case 0:
			if (dval1)
				rtn = 1;
			break;
		case '<':
			if (dval1 < dval2)
				rtn = 1;
			break;
		case '>':
			if (dval1 > dval2)
				rtn = 1;
			break;
		case '==':
			if (dval1 == dval2)
				rtn = 1;
			break;
		case '<>':
		case '!=':
			if (dval1 != dval2)
				rtn = 1;
			break;
		case '!>':
			if (!(dval1 > dval2))
				rtn = 1;
			break;
		case '!<':
			if (!(dval1 < dval2))
				rtn = 1;
			break;
		case '>=':
			if (dval1 >= dval2)
				rtn = 1;
			break;
		case '<=':
			if (dval1 <= dval2)
				rtn = 1;
			break;
		case '!':
			if (!dval2)
				rtn = 1;
			break;
	} 
Exit:
	GSSiGlobUlFree (&hVals);
{
#if ENABLETRACE
GSSiExitProg (1193);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

double ComputeFLTValue (double val1, double val2, char OpCode,LPBOOL IRC)
#if ENABLETRACE
{GSSiEnterProg (1194);
#endif
{   
	*IRC = 0;
	switch (OpCode)
	{
		case '+':
{
#if ENABLETRACE
GSSiExitProg (1194);
#endif
			return (val1 + val2);
}
		case '-':
{
#if ENABLETRACE
GSSiExitProg (1194);
#endif
			return (val1 - val2);
}
		case '*':
{
#if ENABLETRACE
GSSiExitProg (1194);
#endif
			return (val1 * val2);
}
		case '/': 
			if (!val2)
			{
				*IRC = 4;
{
#if ENABLETRACE
GSSiExitProg (1194);
#endif
				return 0;
}
			}
{
#if ENABLETRACE
GSSiExitProg (1194);
#endif
			return (val1 / val2);
}
		case 'p':
{
#if ENABLETRACE
GSSiExitProg (1194);
#endif
			return (pow (val1,val2)); 
}
	}
	*IRC = 3;
{
#if ENABLETRACE
GSSiExitProg (1194);
#endif
			return 0;
}
#if ENABLETRACE
}
#endif
}

short OpRankBool (UINT OpCode)
#if ENABLETRACE
{GSSiEnterProg (1195);
#endif
{
	switch (OpCode)
	{
		case '>':
		case '<':
		case '!':
		case '==':
		case '!=':
		case '!>':
		case '!<':
		case '>=':
		case '<=':
{
#if ENABLETRACE
GSSiExitProg (1195);
#endif
			return 1;
}
		case '&&':
		case '||':
{
#if ENABLETRACE
GSSiExitProg (1195);
#endif
			return 2;
}
	} 
{
#if ENABLETRACE
GSSiExitProg (1195);
#endif
	return 0;
}
#if ENABLETRACE
}
#endif
}

short OpRank (char OpCode)
#if ENABLETRACE
{GSSiEnterProg (1196);
#endif
{
	switch (OpCode)
	{
	default:
		case '+':
		case '-':
{
#if ENABLETRACE
GSSiExitProg (1196);
#endif
			return 1;
}
		case '*':
		case '/':
{
#if ENABLETRACE
GSSiExitProg (1196);
#endif
			return 2;
}
		case 'p':
{
#if ENABLETRACE
GSSiExitProg (1196);
#endif
			return 3; 
}
	}
#if ENABLETRACE
}
#endif
}

double FltAP (LPSTR INEXPR,LPBOOL IRC)
#if ENABLETRACE
{GSSiEnterProg (1197);
#endif
{ 
// IRC (0=success, 1 = invalid flt value, 2 = error in paren matching, 3 = invalid statement, 4 = devide by 0)  
    LPSTR	pLchr, pVal1Begin, pVal1End, pVal2Begin, pVal2End, pEnd; 
    HANDLE	hInput=GSSiGlobAlloc ( 748,GMEM_MOVEABLE,1024);
    LPSTR	INPUT = GlobalLock (hInput); 
    char	OpCode, OpCodeNext=0;  
    double	rtn=0, Value1, Value2;  
    short	InBrackets=0;

	*IRC = 0;  
	_fstrcpy (INPUT,INEXPR);
	Truncate (INPUT);  
// remove enclosing parens
	if (*INPUT == '(')
	{
		pLchr = LastChr (INPUT);
    	pEnd = MatchLev ((LPSTR)(INPUT+1),')');
    	if (pEnd == pLchr)
    	{
	    	INPUT++;
	    	*pLchr-- = 0;
	    }
    }

// find first value 
	pVal1Begin = pVal1End = 0; 
NextVal1:
    switch (*INPUT)
    {   
    	case 0:
    		if (!pVal1Begin)
    		{
    			rtn = 0;
    			goto Exit;
    		} 
    		if (pVal1End)
    			*pVal1End = 0; 
    		rtn = fltread (pVal1Begin,IRC);
    		goto Exit; 
    	case '[':
			if (!pVal1Begin)
				pVal1Begin = INPUT;
    		InBrackets++;
    		goto Next1;
    	case ']':
    		InBrackets--; 
    		goto Next1;  
    	default:
    		break;
    }
	if (!InBrackets)  
    switch (*INPUT)
    {   
    	case ' ':  
    		if (pVal1Begin)
    			pVal1End = INPUT;
    		break; 
    	case '(':
    		if (!pVal1Begin)
    			pVal1Begin = INPUT;
    		INPUT++;
    		if (!(pEnd = MatchLev (INPUT,')')))
    		{
    			*IRC = 2;
    			goto Exit;
    		}
    		INPUT = pEnd;
    		break;
    	case '*':
    		if (pVal1Begin)
    			pVal1End = INPUT; 
    		else
    		{
    			*IRC = 3;
    			goto Exit;
    		}
    		if (*(INPUT+1) == '*')
    		{
    			INPUT+=2;
    			OpCode = 'p';
    			goto GetSecondValue;
    		} 
    	case '+':
    	case '-':
//    		if (isdigit(*(INPUT+1))) 
    		{
				if (!pVal1Begin) 
				{
					pVal1Begin = INPUT;
    				break; 
    			}
    		}
    	case '/':
    		if (pVal1Begin)
    			pVal1End = INPUT; 
    		OpCode = *INPUT++;
    		goto GetSecondValue;
		default:
			if (!pVal1Begin)
				pVal1Begin = INPUT;
			break;
    }
Next1:
    INPUT++;
    goto NextVal1;
    		
GetSecondValue: 
	InBrackets = 0;
	if (pVal1Begin)
	{   
		*pVal1End = 0;
		Value1 = FltAP (pVal1Begin,IRC);
		if (*IRC)
		{
			GSSiGlobUlFree (&hInput);
{
#if ENABLETRACE
GSSiExitProg (1197);
#endif
			return 0; 
}
		}
	}
	else           
		Value1 = 0;

// find second value 
GetSecondVal:
	pVal2Begin = 0;
NextOp:
	pVal2End = 0; 
NextVal2:  
    switch (*INPUT)
    {   
    	case 0:
    		if (!pVal2Begin)
    		{
    			*IRC = 3;
    			goto Exit;
    		} 
    		Value2 = FltAP (pVal2Begin,IRC);
    		if (!*IRC)
    			rtn = ComputeFLTValue (Value1,Value2,OpCode,IRC);
    		goto Exit;
    	case '[':
			if (!pVal2Begin)
				pVal2Begin = INPUT;
    		InBrackets++;
    		goto Next2;
    	case ']':
    		InBrackets--; 
    		goto Next2;  
    	default:
    		break;
    }
    if (!InBrackets)	
    switch (*INPUT)
    {   
    	case ' ':  
    		if (pVal2Begin)
    			pVal2End = INPUT;
    		break; 
    	case '(':
    		if (!pVal2Begin)
    			pVal2Begin = INPUT;
    		INPUT++;
    		if (!(pEnd = MatchLev (INPUT,')')))
    		{
    			*IRC = 2;
    			goto Exit;
    		}
    		INPUT = pEnd;
    		break;
    	case '*':
    		if (pVal2Begin)
    			pVal2End = INPUT; 
    		else
    		{
    			*IRC = 3;
    			goto Exit;
    		}
    		if (*(INPUT+1) == '*')
    		{
    			INPUT+=2;
    			OpCodeNext = 'p';
    			goto CheckEndSecondValue;
    		} 
    	case '-':
    		if (pVal2Begin)
    			pVal2End = INPUT; 
    		else 
    		{
				pVal2Begin = INPUT;
				break;
			}
    	case '+':
    	case '/':
    		if (pVal2Begin)
    			pVal2End = INPUT; 
    		else
    		{
    			*IRC = 3;
    			goto Exit;
    		}
    		OpCodeNext = *INPUT++;
    		goto CheckEndSecondValue;
		default:
			if (!pVal2Begin)
				pVal2Begin = INPUT;
			break;
    }   
Next2:
    INPUT++;
    goto NextVal2;
    		
CheckEndSecondValue:
	if (OpRank(OpCodeNext) > OpRank(OpCode))
		goto NextOp;
	*pVal2End = 0;
	Value2 = FltAP (pVal2Begin,IRC);
	if (!*IRC)
		Value1 = ComputeFLTValue (Value1,Value2,OpCode,IRC); 
	if (*IRC)
		goto Exit;
	OpCode = OpCodeNext;
	goto GetSecondVal; 

Exit:		
	GSSiGlobUlFree (&hInput);
{
#if ENABLETRACE
GSSiExitProg (1197);
#endif
	return rtn; 		
}
#if ENABLETRACE
}
#endif
} 
short MATPAR (LPSTR STRING,short IBEG,short IEND)
#if ENABLETRACE
{GSSiEnterProg (1198);
#endif
{     
	short	NEST=0;
	short	I, rtn;

      I   = IBEG;
      rtn = 0;
 S10: if (I > IEND)
{
#if ENABLETRACE
GSSiExitProg (1198);
#endif
   		return rtn;
}
      if (STRING[I] !=')')
        goto S20;
      if (!NEST)
        goto S50;
      NEST--;
      I++;
      goto S10;
  S20:if (STRING[I] =='(')
  		NEST++;
      I++;
      goto S10;
  S50:rtn = I;
{
#if ENABLETRACE
GSSiExitProg (1198);
#endif
      return rtn;
}
#if ENABLETRACE
}
#endif
}    

short NDELIM (LPSTR INPUT,short IB,short IE,char DELIM,LPSTR LEVDLM)
#if ENABLETRACE
{GSSiEnterProg (1199);
#endif
{
      short FLIP[2]={2,1}, FLOP, ITEST, ILLDLM, IRLDLM, J, rtn, LEVEL;
      BOOL SAME;
      
      if (LEVDLM[0] ==LEVDLM[1]) 
          SAME = TRUE;
      else
          SAME = FALSE;
      FLOP   = 1;
      LEVEL  = 0; 
      for (J = IB;J<IE+1;J++)
      {
          ILLDLM = 1;
          IRLDLM = 1;
          ITEST  = 1;
          if (INPUT[J] ==LEVDLM[0])
          {
              ILLDLM = 0;
              if (SAME) IRLDLM = 0;
          }
          else if (INPUT[J] ==LEVDLM[1]) 
              IRLDLM = 0;
          else if (INPUT[J] ==DELIM && LEVEL ==0) 
              goto S20;
          else
              goto S10;

          if (FLOP) goto S5;
          if (ILLDLM) goto S2;
          LEVEL++;
          FLOP   = FLIP[FLOP];
          goto S10;
    S2:   if (IRLDLM) goto S8;
          LEVEL--;
          FLOP   = FLIP[FLOP];
          goto S10;
    S5:   if (IRLDLM) goto S6;
          LEVEL--;
          FLOP   = FLIP[FLOP];
          goto S10;
    S6:   if (ILLDLM) goto S8;
          LEVEL++;
          FLOP   = FLIP[FLOP];
          goto S10;
    S8:   if (!ITEST && !LEVEL) goto S20;
   S10:;
   		}
      rtn  = 0;
{
#if ENABLETRACE
GSSiExitProg (1199);
#endif
      return rtn;
}
  S20: rtn  = J;
{
#if ENABLETRACE
GSSiExitProg (1199);
#endif
      return rtn;
}
#if ENABLETRACE
}
#endif
}
