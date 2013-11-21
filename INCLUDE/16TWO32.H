#if WIN32
#define _fstrncpy strncpy
#define _fstrlen strlen
#define _fstrcmp strcmp
#define _fstrnicmp strnicmp
#define _fstrncmp strncmp
#define _fstrcpy strcpy
#define _fstrstr strstr
#define _fstrlwr strlwr
#define _fstrcat strcat
#define _fstrupr strupr
#define _fstrchr strchr
#define _fstrrchr strrchr
#define _fstricmp stricmp
#define _fmemmove memmove
#define _fstrset strset
#define _fmemicmp memicmp
#define _fmemcpy memcpy
#define _fmemcmp memcmp
#define _fmemset memset
#define _fstrcspn strcspn
#define	HUGE
typedef signed long 		SLONG;
typedef signed short		SSHORT;
typedef unsigned long		ULONG;    
typedef ULONG				FAR *LPULONG;
typedef ULONG				HUGE *HPULONG;
typedef unsigned short		USHORT; 
typedef USHORT				FAR	*LPUSHORT;
typedef USHORT				HUGE *HPUSHORT;
typedef char				HUGE *HPSTR;
typedef short				HUGE *HPSHORT;
typedef long				HUGE *HPLONG;   
typedef unsigned char		HUGE *HPBYTE; 
//typedef HBRUSH				FAR	*LPHBRUSH;
//typedef HPEN				FAR	*LPHPEN;
typedef short				FAR *LPSHORT;
typedef float			 	FAR *LPFLOAT;
typedef float   			HUGE *HPFLOAT;
typedef double  			FAR *LPDOUBLE;
typedef double  			HUGE *HPDOUBLE;
typedef struct{double x,y;} DPOINT;
typedef DPOINT  			FAR *LPDPOINT;
typedef DPOINT  			HUGE *HPDPOINT;
typedef struct{double x,y,z;} DPOINT3D;
typedef DPOINT3D  			FAR *LPDPOINT3D;
typedef DPOINT3D  			HUGE *HPDPOINT3D;
typedef struct{float x,y;} FPOINT;
typedef FPOINT  			FAR *LPFPOINT;
typedef FPOINT  			HUGE *HPFPOINT;
typedef POINT				HUGE *HPPOINT;
typedef void				HUGE* LPHUGE;
typedef struct{long x,y;} LPOINT;
typedef LPOINT  			FAR *LPLPOINT;         
typedef DWORD				HDIB32; 
typedef HDIB32				FAR *LPHDIB32;

#endif

