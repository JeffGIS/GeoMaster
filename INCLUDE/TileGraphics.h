#define TGPOLYGON	0
#define TGPOLYLINE	1

#ifdef HBIRD
typedef struct{short x,y;} POINTS;
typedef POINTS *LPPOINTS;
typedef struct{int x,y;} POINT;
typedef POINT *LPPOINT;
#endif
typedef struct{unsigned char x,y;}	BPOINT;
typedef struct{char x:4,y:4;}CPOINT;
typedef BPOINT				*LPBPOINT;
typedef CPOINT				*LPCPOINT;

//BPOINT POINTtoBPOINT (POINT p);
//POINT BPOINTtoPOINT (BPOINT p);


typedef	struct {
				unsigned short
				symbol:10,
				type:2,
				nsegtype:1,
				coordtype:2,
				completeTile:1;
				long refno;} TGRECHEADER;
typedef TGRECHEADER	*LPTGRECHEADER;

unsigned int MapSize(int levelOfDetail);
double GroundResolution(double latitude, int levelOfDetail);
void LatLongToPixelXY(double latitudein, double longitudein, int levelOfDetail, int *pixelX, int *pixelY);
void LatLongToPixelXYd(double latitudein, double longitudein, int levelOfDetail, double *pixelX, double *pixelY);
void PixelXYToLatLong(int pixelX, int pixelY, int levelOfDetail, double *latitude, double *longitude);
void PixelXYToTileXY(int pixelX, int pixelY, int *tileX, int *tileY);
void PixelXYToTileXYd(double pixelX, double pixelY, int *tileX, int *tileY);
void TileXYToPixelXY(int tileX, int tileY, int *pixelX, int *pixelY);
void TileXYToQuadKey(int tileX, int tileY, int levelOfDetail,char *quadKey,int maxLen);
int QuadKeyToTileXY(char *quadKey, int *tileX, int *tileY, int *levelOfDetail);
