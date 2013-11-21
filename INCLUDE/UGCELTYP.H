class CUGCellType{

	
	//cell type ID - always set this value to the same as the value
	//returned in the GETID function
protected:
	long	m_ID;
	BOOL	m_TextEditMode;
	BOOL	m_DrawExtraText;

	BOOL	m_HasFocus;
	int		m_FocusCol;
	long	m_FocusRow;

public:

	UG_GridInfo *	m_GI;			//pointer the the grid info data
	CUGCtrl		*	m_Ctrl;			//pointer to the main class

	friend CUGCtrl;
	
	CUGCellType();
	~CUGCellType();

	//general purpose drawing routines
	int DrawBitmap(CDC *dc,HBITMAP hBitmap,RECT *rect,COLORREF backcolor);

	//text editing functions
	int CanTextEdit();
	int SetTextEditMode(int mode);


	//virtual functions
	virtual long GetID();
	
	virtual int GetEditArea(RECT *rect);

	virtual int SetOption(long option,long param);
	virtual int SetCellOption(CUGCell *cell,long option,long param);

	virtual int OnSystemChange();

	virtual int OnLClicked(int col,long row,int updn,RECT *rect,POINT *point);
	virtual int OnRClicked(int col,long row,int updn,RECT *rect,POINT *point);
	virtual int OnDClicked(int col,long row,RECT *rect,POINT *point);

	virtual int OnKeyDown(UINT *vcKey);
	virtual int OnCharDown(UINT *vcKey);

	virtual int OnMouseMove(POINT *point);
	
	virtual int OnDraw(CDC *dc,RECT *rect,CUGCell *cell,int selected,int current);
	virtual int OnPrint(CDC *dc,RECT *rect,CUGCell *cell,int origWidth,int origHeight);

	virtual void OnSetFocus(int col,long row,CUGCell *cell);
	virtual void OnKillFocus(int col,long row,CUGCell *cell);

	virtual void GetBestSize(CDC *dc,CSize *size,CUGCell *cell);
};