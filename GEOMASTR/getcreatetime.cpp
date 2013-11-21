//using namespace System;
//using namespace System::IO;
#include <windows.h>
DWORD	GetCTime (int x)
{
	char	Dir[MAX_PATH];
      
	  strcpy (Dir,"C:\\Program Files\\LakeMaster\\Contour Pro Minnesota\\address");
      // Get the creation time of a well-known directory.
      DateTime dt = Directory::GetCreationTime(Dir );
      return (DWORD)dt;

}
