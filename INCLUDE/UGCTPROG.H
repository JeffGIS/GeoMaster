class CUGProgressType: public CUGCellType{

public:

	CUGProgressType();
	~CUGProgressType();
	virtual int OnDraw(CDC *dc,RECT *rect,CUGCell *cell,int selected,int current);

};