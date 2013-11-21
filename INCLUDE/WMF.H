#include <windows.h>
/*
 * WMF Import Filter - MAIN HEADER FILE
 *
 * LANGUAGE      : Microsoft C 6.0
 * MODEL         : medium
 * ENVIRONMENT   : Microsoft Windows 3.0 SDK
 * STATUS        : operational
 *
 * This header contains all the definitions used by the Aldus WMF import
 * filter.  The Aldus filter definitions are extracted from the Aldus header
 * files and included in a section in this header file.
 *
 *    Eikon Systems, Inc.
 *    989 East Hillsdale Blvd, Suite 260
 *    Foster City, California 94404
 *    415-349-4664
 *
 * 08/01/91 1.00 - David E. West - initial creation.
 * 09/01/91 1.01 - Kevin P. Welch - minor revisions.
 *
 */

/* ALDUS IMPORT FILTER DEFINITIONS */

#define 	SUCCESS 			0
#define	FAILURE 			-1

#define	PMMETAFILEKEY	0x9AC6CDD7L		/* Windows metafile keyword */

#define	DC 				short				/* screen device coordinates */
typedef	DWORD				FILETYPE;		/* file type flags */

typedef struct _PICTINFO {
	HANDLE	hmf;								/* Global memory handle to metafile */
	RECT		bbox;								/* bounding rect in metafile units */
	DC			inch;								/* metafile units per inch */
} PICTINFO;

typedef struct _PMMETAFILEHEADER {
	DWORD		key;								/* Windows metafile keyword */
	PICTINFO	mfPict;							/* PM Metafile structure */
	DWORD		reserved;						/* Reserved words */
	WORD		checksum;						/* Check sum of previous entries */
} PMMETAFILEHEADER;

#define	IBMFNSIZE 		124

typedef struct _FILESPEC {
	unsigned		slippery : 1;				/* TRUE if file may disappear. */
	unsigned		write : 1;					/* TRUE if open for write. */
	unsigned		unnamed : 1;				/* TRUE if unnamed. */
	unsigned		linked : 1;					/* Linked to an FS FCB. */
	unsigned		mark : 1;					/* Generic mark bit. */
	FILETYPE		fType;						/* The file type. */
	short			handle;						/* MS-DOS open file handle. */
	char			fullName[IBMFNSIZE];		/* Device, path, file names. */
	DWORD			filePos;						/* Our current file posn. */
} FILESPEC;

typedef PICTINFO FAR *				LPPICTINFO;
typedef PMMETAFILEHEADER FAR *	LPPMMETAFILEHEADER;
typedef FILESPEC FAR *				LPFILESPEC;

/* Import Error Codes */

#define IE_BASE					5300				/* base value for IE_ error codes */
#define IE_NOT_MY_FILE			(IE_BASE+1)		/* generic "not my file" error */
#define IE_TOO_BIG				(IE_BASE+2)		/* bitmap or pict too big error */
#define IE_DUMB_BITMAP			(IE_BASE+3)		/* bitmap all white */
#define IE_BAD_VCHAR				(IE_BASE+4)		/* bad vchar in ImportString */
#define IE_BAD_TOKEN				(IE_BASE+5)		/* illegal wp token */
#define IE_NO_VERIFY				(IE_BASE+6)		/* failed to verify imported story */
#define IE_UNKNOWN_TYPE			(IE_BASE+7)		/* unknown file type */
#define IE_NOT_WP_FILE			(IE_BASE+8)		/* Not a %s file */
#define IE_BAD_FILE_DATA 		(IE_BASE+9)  	/* current file data is bad */
#define IE_IMPORT_ABORT 		(IE_BASE+10)	/* import abort alert */
#define IE_MEM_FULL				(IE_BASE+11)	/* ran out of memory during import */
#define IE_MSNG_FONTS			(IE_BASE+12)	/* missing system fonts */
#define IE_METAFILE_TOO_BIG	(IE_BASE+13)	/* metafile too big */
#define IE_INVALID_LOTUS_PIC	(IE_BASE+14)	/* bad lotus .pic */
#define IE_MEM_FAIL				(IE_BASE+15)	/* couldn't lock memory during import */

#define IE_PAINT_BASE			(IE_BASE+40)
#define IE_UNSUPP_COMPR			(IE_PAINT_BASE+1)	/* unsupp'd compress. style */
#define IE_UNSUPP_VERSION		(IE_PAINT_BASE+2)	/* unsupp'd file version */
#define IE_UNSUPP_COLOR			(IE_PAINT_BASE+3)	/* can't handle this color style */

/* Windows metafile header pointers */
typedef METAHEADER *			PMETAHEADER;
typedef METAHEADER FAR *	LPMETAHEADER;

/* Aldus placeable metafile header key value */
#define WMFKEY		0x9AC6CDD7

/* Aldus placeable metafile format header */
typedef struct {
	DWORD			idKey;						/* binary key--0x9AC6CDD7 */
	HANDLE		hMF;							/* handle to metafile--zero */
	RECT			bbox;							/* tight bounding box */
	WORD			cInch;						/* metafile units per inch */
	DWORD			dwReserved;					/* reserved--must be zero */
	WORD			idChecksum;					/* checksum of first 10 words */
} WMFHEADER;

typedef WMFHEADER FAR *		LPWMFHEADER;

/* LOCAL DEFINITIONS */
#if WIN32
typedef char  *	HPSTR;
typedef BYTE  *	HPBYTE;
#else
typedef char huge *	HPSTR;
typedef BYTE huge *	HPBYTE;
#endif
#define INT				int
#define SHORT			short

/* other defines */
#define ABS(x)			((x) < 0 ? (-(x)) : (x))

/* WMF Filter Functions - wmf1.c */
WORD FAR PASCAL		WEP( WORD );
BOOL FAR PASCAL		LibMain( HANDLE, WORD, WORD, LPSTR );
VOID FAR PASCAL		GetFilterPref( HANDLE, HWND, HANDLE, WORD );
WORD FAR PASCAL		ImportGr( HDC, LPFILESPEC, LPPICTINFO, HANDLE );
WORD FAR PASCAL		GetFilterInfo( WORD, LPSTR, HANDLE FAR *, HANDLE FAR * );

/* WMF Filter Functions - wmf2.c */
BOOL FAR PASCAL		ValidateWMF( LPWMFHEADER );
WORD FAR PASCAL		ComputeWMFChecksum( LPWMFHEADER );
WORD FAR PASCAL		GetWMFBits( LPHANDLE, LONG, INT );

/* WMF Filter Functions - wmf3.c */
DWORD FAR PASCAL		hread( INT, HPSTR, DWORD );
