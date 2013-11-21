typedef void *LPVOID;
typedef struct {
        unsigned char    rgbBlue;
        unsigned char    rgbGreen;
        unsigned char    rgbRed;
        unsigned char    rgbReserved;
} FTC_RGBQUAD;

typedef struct {int x,y;}FTC_POINT;
typedef FTC_POINT *LPFTC_POINT;

LPVOID LKMiPilotInit (double pickX, double pickY, double pickOffset,
							  int maxTrackPoints,double trackPointSpacing,
                       char *pickText, int *PickDepthInFt,
                       int azimuthFromStart[2],int deeperFlag[2],int userPicked[2]);
int LKMiPilotEnableSelectionImage (LPVOID ipAddress,FTC_RGBQUAD directionColors[2],int lineWidth,int  doDash);
void LKMiPilotEnableSelectedContourHighlight (LPVOID ipAddress, FTC_RGBQUAD color,int lineWidth);
LPVOID  LKMiPilotRequestTrack (LPVOID ipAddress,int trackOption,double x, double y,double x2,double y2,
									   double offsetInMeters,
									   int knotRemovalRange, int smoothOpt,int wantDepths);
void LKMiPilotClear (LPVOID ipAddress);

LPFTC_POINT GetFTCTrack (LPVOID ipAddress,int *pNumPoints);
int	*GetFTCTrackDepths (LPVOID ipAddress,int *pNumPoints);
int IsFTCTrackContinuous (LPVOID ipAddress);

int IsFTCCompatible (void);
LPFTC_POINT GetFTCSelectionTrack (LPVOID ipAddress,int iWhichTrack, int *pNumPoints);
LPFTC_POINT GetFTCSelectedContourTrack (LPVOID ipAddress, int *pNumPoints);
void SetFTCSelectSpacing (double trackPointSpacing);
void SetFTCMaxSelectionTrack (int maxSelTrack);
void SetFTCMaxSelectedContour (int maxSelTrack);
void ClearFTCSelectionContours (LPVOID ipAddress);
