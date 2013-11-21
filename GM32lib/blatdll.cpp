#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

int APIENTRY Send (LPCSTR sCmd);

DWORD GMSendEMail (LPSTR BlatCommandLine)
{
	DWORD rc = Send (BlatCommandLine);

	return rc;
}
