class CUGCheckBoxType: public CUGCellType{

	CPen	Darkpen;
	CPen	Lightpen;
	CPen	Facepen;

	CUGCell m_cell;

public:

	CUGCheckBoxType();
	~CUGCheckBoxType();
	virtual long GetID();
	virtual int OnLClicked(int col,long row,int updn,RECT *rect,POINT *point);
	virtual int OnKeyDown(UINT *vcKey);
	virtual int OnDraw(CDC *dc,RECT *rect,CUGCell *cell,int selected,int current);

};