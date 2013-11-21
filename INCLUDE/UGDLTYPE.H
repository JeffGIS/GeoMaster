class CUGDropListType: public CUGCellType{

	CPen	pen;
	CBrush	brush;

	int		btnWidth;
	int		btnDown;
	RECT	btnRect;
	int		btnCol;
	long	btnRow;

	CUGLstBox*	listBox;

public:

	CUGDropListType();
	~CUGDropListType();
	virtual int GetEditArea(RECT *rect);
	virtual int OnLClicked(int col,long row,int updn,RECT *rect,POINT *point);
	virtual int OnDClicked(int col,long row,RECT *rect,POINT *point);
	virtual int OnKeyDown(UINT *vcKey);
	virtual int OnMouseMove(POINT *point);
	virtual void OnKillFocus(int col,long row,CUGCell *cell);
	virtual int OnDraw(CDC *dc,RECT *rect,CUGCell *cell,int selected,int current);
	int StartDropList();
};
