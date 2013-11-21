class CUGAutoFontType: public CUGCellType{


	HFONT CreateAutoSizeFont(HDC hdc, RECT *rect,LPCSTR string);

public:

	CUGAutoFontType();
	~CUGAutoFontType();
	virtual int OnDraw(CDC *dc,RECT *rect,CUGCell *cell,int selected,int current);

};