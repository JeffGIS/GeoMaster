/*****************************************************************************
Name			:	MIC2.H

Title			:	MIC System II for Windows 3.xx DLL header file.

Version			:	2.0

Copyright 		:	VideoLogic Limited 1991-92, All rights reserved.
*****************************************************************************/

/****************************************************************************/
#if		!defined( _MIC2_ )
#define 	_MIC2_

/****************************************************************************/
/*	External Defines.														*/
/****************************************************************************/
#define MIC2_OK						((WORD)0x0000)		/* all OK	*/
#define ERROR_MIC2_ERROR			((WORD)~MIC2_OK)	/* MIC2 error	*/
#define ERROR_MIC2_INVALID_ID		((WORD)0xFFFE)		/* Invalid MIC Id	*/
#define ERROR_MIC2_ALREADY_OPEN		((WORD)0xFFFD)		/* MIC already opened by this application */
#define ERROR_MIC2_NOT_OPEN			((WORD)0xFFFC)		/* MIC has not been opened */
#define ERROR_MIC2_NOT_INITIALIZED	((WORD)0xFFFB)		/* MIC not initialised (micinit.exe is not in win.ini file)	*/
#define ERROR_MIC2_NOT_INITIALISED	((WORD)0xFFFB)		/* MIC not initialised (micinit.exe is not in win.ini file)	*/
#define ERROR_MIC2_FATAL_ERROR		((WORD)0xFFFA)		/* MIC fatal error	*/
#define ERROR_MIC2_TOO_MANY_PROCS	((WORD)0xFFF9)		/* Too many application trying to acess MIC at once	*/

/****************************************************************************/
/*	External Prototypes.													*/
/****************************************************************************/
WORD FAR PASCAL MICOpen(LPDWORD lpdwMICId);
WORD FAR PASCAL MICClose(LPDWORD lpdwMICId);
WORD FAR PASCAL MICWriteStr(DWORD dwMICId, LPSTR lpszCommand);
WORD FAR PASCAL MICReadStr(DWORD dwMICId, LPSTR lpszResponse, WORD wSize);
WORD FAR PASCAL MICWrRdStr(DWORD dwMICId, LPSTR lpszCommand, LPSTR lpszResponse, WORD wSize);
WORD FAR PASCAL MICReadDef(DWORD dwMICId, LPSTR lpszResponse, WORD wSize);
WORD FAR PASCAL MICNotify(DWORD dwMICId, DWORD dwWindow, DWORD dwMessage);

#endif		/*	_MIC2_	*/
/****************************************************************************/
/*	End of File.															*/
/****************************************************************************/

