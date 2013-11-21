class CUGArrowType: public CUGCellType{

public:

	CUGArrowType();
	~CUGArrowType();
	virtual int OnDraw(CDC *dc,RECT *rect,CUGCell *cell,int selected,int current);

};