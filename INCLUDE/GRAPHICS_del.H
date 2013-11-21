#define GF_INIT			10000
#define GF_COMPLETE		10001
#define GF_ZOOM_RECT	1
#define GF_MOVE_TAG		2
#define	GF_WINDOW_ZOOM	3
#define	GF_BLOWUP		4
#define	GF_CREATE_HLT_AREA 5
#define GF_POINT		101
#define	GF_LINE			102
#define	GF_POLYLINE		103
#define	GF_CURVE		104
#define	GF_AREA			105
#define	GF_MAPMODE		201
#define	GF_TEXTMODE		202
#define GF_REDRAW		203


int DisplayPlotInit (HWND hWnd, BOOL Plot);
BOOL OpenMap (HWND hWnd, HDC hDC, BOOL Print);
void CloseMap (void);
void ClearMap (HWND hWnd);
int	PanWindow (HWND hWnd, float PanVert, float PanHorz);
int CenterWindow (HWND hWnd, POINT CenterPoint);
int	ZoomWindow (HWND hWnd, float ZoomFactor);
int SetBounds (HWND hWnd,HDC hDC);
int	SetBoundsRect2 (RECT Rect);
int	MouseInput (HWND hWnd,WORD Message,POINT MousePoint, int Opt);
BOOL WindowZoomed (void);
void SetNewBoundsToBounds(void);
BOOL PrintMap (HWND hWnd, HWND DataWnd, int DataControl);
BOOL ClipMap (HWND hWnd);
BOOL FileMap (HWND hWnd);
void PaintMap (HWND hWnd, HDC hDC);
void ClearBounds(void);
void OpenBasePens(void);
void CloseBasePens(void);
BOOL DisplaySeg (HDC hDC);
int HaveMapTimer (void);
BOOL GetVisibility (int idesc);
BOOL ToggleVisibility (int idesc);
BOOL ToggleFileVisibility (int ifile);
void SetTypeVisibility (int type, BOOL on);
void GetVisList (HWND hWndDlg,int DlgItemSym, int DlgItemPar);
int GetTypeVisibility (int type);
void NewMap(void);
int GetLBFunction(void);
void SetLBFunction(int Fun);
void HaltMapDisplay(void);
void ShowPickedItem (HWND hWnd, int Item);
void DisplayPickedItems (int NumPicked);
void GetItemTAG (int item,LPSTR TAG, LPSTR Symbol, LPLONG Refno);
void GetItemLoc (int item,LPLONG Segment,LPWORD Offset);
void SetParentVisibility (int Parent, BOOL Vis);
void AddGraphicsFunction (HWND hWnd,int FunType);
void RemoveGraphicsFunction (HWND hWnd);
BOOL ProcessGraphicsFunction (HWND hWnd, WORD Message, WORD wParam, LONG lParam);
void SetDisplayChar (HDC hDC, int Type,long iref,int desc);
BOOL PickByRefno (long Refno,LPSTR Prefix, LPSTR UDI,int PickFile);
void QuitGraphics(void);
void MapDblClick (void);
POINT GetCursorReturnPoint(void);
void SetCursorReturnPoint(POINT point);
void OpenNewObjects();
void CloseNewObjects();
void SetClassColor (int desc,int type, COLORREF color);
void ClearHighlightList ();
BOOL SetAutoPan (BOOL AP);
BOOL OpenPlotFile (void);
BOOL ClosePlotFile (void);

