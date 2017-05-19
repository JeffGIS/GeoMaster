//  MathTypes.h
//  CCodeLibrary

#ifndef __MATH_TYPES
#define __MATH_TYPES

#define PI 3.14159265358979323846e0
#define PI_DIV_2 PI * 0.5
#define PY 3.14159265358979323846e0
#define TWOPI PY*2
#define	HALFPI PY/2
#define RADtoDEG 57.295779513082322
#define DEGtoRAD 0.017453292519943296
#define  RADDEG    1.7453292519943396e-2
#define  DEGRAD    5.7295779513082322e1
#define MFT 3.280833333333333e0
#define FTM 3.04800609601219e-1
#define ACRES_PER_SQMETER   0.000247105
#define SQMETERS_PER_SQMILE 2589988.11e1
#define SQMILES_PER_SQMETER 3.86102e-7

#ifndef max
#define max(a,b)(((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)(((a) < (b)) ? (a) : (b))
#endif

#define IDNINT(r)(int)(r < 0 ? r - 0.5f : r + 0.5f)
#define IDNINT64(r)(int)(r < 0 ? r - 0.5f : r + 0.5f)

typedef struct
{
    int NSETPT;
    double  A1, A2, B1, B2, C1, C2, BASX, BASY;
    double	FitPointAZFrom, FitPointAZTo;
} TRANDATA;
typedef TRANDATA    *LPTRANDATA;
#define MAXTRANPOINTS	32

typedef struct
{	int	xmn;
    int	ymn;
    int	xmx;
    int	ymx;
} MNMXCORL;

typedef MNMXCORL *LPMNMXCORL;
typedef MNMXCORL IBOUNDS;
typedef IBOUNDS LPIBOUNDS;

typedef struct
{	double	xmn;
    double	ymn;
    double	xmx;
    double	ymx;
} MNMXCORD;

typedef MNMXCORD *LPMNMXCORD;


typedef float f32;
typedef int	s32;
typedef unsigned int u32;
typedef char s8;
typedef unsigned char u8;
typedef float mat3[3][3];
typedef float mat4[16];

#pragma pack(2)			
typedef struct
{
	float r;
	float g;
	float b;
	float a;
}  bvec4;

typedef struct
{
	unsigned short r;
	unsigned short g;
	unsigned short b;
}  bvec4x;
#pragma pack()			

typedef struct
{
	short x;
	short y;
}  svec2;

typedef struct
{
	short x;
	short y;
	short z;
}  svec3;

typedef struct
{
	int x;
	int y;
}  ivec2;

typedef struct
{
	int x;
	int y;
	int z;
}  ivec3;

typedef struct
{
	float x;
	float y;
}  vec2;

typedef DPOINT  dvec2;

typedef struct
{
	float x;
	float y;
	float z;
}  vec3;

typedef struct
{
	double x;
	double y;
	double z;
}  dvec3;

typedef struct
{
	float x;
	float y;
	float z;
	float w;
}  vec4;

typedef struct
{
	float xmn;
	float ymn;
	float xmx;
	float ymx;
}  bounds;

typedef MNMXCORD dbounds;

vec4 MakeVec4(float singleValue);

struct PlaneVec2
{
	vec2 point;
	vec2 normal;
};

struct PlaneVec3
{
	vec3 point;
	vec3 normal;
};
typedef double CLocationDegrees;
typedef struct
{
    CLocationDegrees latitude;
    CLocationDegrees longitude;
} CLocationCoordinate2D;

typedef struct
{
    CLocationDegrees latitudeDelta;
    CLocationDegrees longitudeDelta;
} MCoordinateSpan;

typedef struct
{
    CLocationCoordinate2D center;
    MCoordinateSpan span;
} MCoordinateRegion;

#endif
