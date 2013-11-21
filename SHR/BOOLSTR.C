/*int BoolString (LPSTR str,LPINT irc)
{ 
	LPSTR	lpArg1, lpArg2;
	char	endchar=0;
	
    *irc = 0;
	lpArg1=str;
	
	arg=0;
	while (!arg)
	{   
		if (*lpStr == '\'')
		{
			EndChar = '\'';
		else if (!_fstricmp(lpStr,"<="))  
		{
			arg=1;
			*lpStr=0;
			lpStr+=2;
		}
		else if (!_fstricmp(lpStr,">=")
		{
			arg=2;
			*lpStr=0;
			lpStr+=2;
		}
		else if (!_fstricmp(lpStr,"==")
		{
			arg=3;
			*lpStr=0;
			lpStr+=2;
		}
		else if (!_fstricmp(lpStr,"!=") 
		{
			arg=4;
			*lpStr=0;
			lpStr+=2;
		}
		else if (!_fstricmp(lpStr,">")
		{
			arg=5;
			*lpStr=0;
			lpStr++;
		}
		else if (!_fstricmp(lpStr,"<")  
		{
			arg=6;	
			*lpStr=0;
			lpStr++;
		}
		else if (!_fstricmp(lpStr,"&&") 
		{
			arg=7;	
			*lpStr=0;
			lpStr+=2;
		}
		else if (!_fstricmp(lpStr,"||") 
		{
			arg=8; 
			*lpStr=0;
			lpStr+=2;
		} 
		else if (!*lpStr)
			arg=-1;
		else
			lpStr++;
	}
	lpArg2 = lpStr;	
}
*/


	