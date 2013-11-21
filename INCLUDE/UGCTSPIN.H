class CUGSpinButtonType: public CUGCellType{

	CPen	pen;
	CBrush	brush;

	int		btnBottomDown;
	int		btnTopDown;
	int		btnTopBottom;	//1-top button in progress 2-bottom in progress

	int		btnWidth;
	RECT	btnRect;
	int		btnCol;
	long	btnRow;

public:

	CUGSpinButtonType();
	~CUGSpinButtonType();
	virtual int GetEditArea(RECT *rect);
	virtual int OnDClicked(int col,long row,RECT *rect,POINT *point);
	virtual int OnLClicked(int col,long row,int updn,RECT *rect,POINT *point);
	virtual int OnKeyDown(UINT *vcKey);
	virtual int OnMouseMove(POINT *point);
	virtual int OnDraw(CDC *dc,RECT *rect,CUGCell *cell,int selected,int current);
};
