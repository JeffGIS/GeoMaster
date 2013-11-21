/* function declarations */
BOOL GSinitAVI(void);
void GStermAVI(void);
void fileCloseMovie(HWND hWnd);
BOOL fileOpenMovie(HWND hWnd, LPSTR Name);
void positionMovie(HWND hWnd);
void playMovie(HWND hWnd, WORD wDirection);
void seekMovie(HWND hWnd, WORD wAction);
void stepMovie(HWND hWnd, int wDirection);
long GetAVIFrame (void);
void SetAVIFrame (long Frame);
void FullScreenFrame(HWND hWnd, long Frame);
void GSFullScreenPlay(HWND hWnd, LPSTR Name);
void ReOpenMovie (HWND hWnd);
void SetAVIStatus (BOOL On);
BOOL SetAVIFrameNotify (long Frame, HWND hWndNotify);
BOOL GoToNextSubFrame(int wDirection);
BOOL GoToNextRawFrame(int wDirection);
BOOL GoToNextHouseFrame(int wDirection);
void DisplayCurrentFrame (void);
BOOL OpenFrameList(LPSTR Name, LPSTR OrigName); 
void CloseFrameList(void);
void PlayVideo(HWND hWnd, LPSTR Name);
void PlayVideoFrame0(HWND hWnd, LPSTR Name);
HANDLE FrameToDIB (HWND hWnd);
HANDLE FrameToDIB2 (LPSTR File, long Frame);
void EnableVideoControls (BOOL On);
DWORD WINAPI mciSendString2 (LPSTR lpstrCommand,
    LPSTR lpstrReturnString, UINT uReturnLength, HWND hwndCallback);
