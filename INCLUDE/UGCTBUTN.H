
//type ID
#define UGCT_BUTTON			10000
//notifications
#define UGCT_BUTTONCLICK	1	
#define UGCT_BUTTONDOWN		2	
#define UGCT_BUTTONUP		3	


//styles
#define UG_NORMAL			1
#define UG_TOGGLE			2


class CUGButtonType: public CUGCellType{

	int		m_btnDown;
	int		m_btnDownCol;
	long	m_btnDownRow;
	int		m_mode;
	RECT	m_btnDownRect;

public:

	CUGButtonType();
	~CUGButtonType();
	virtual int GetEditArea(RECT *rect);
	virtual int OnLClicked(int col,long row,int updn,RECT *rect,POINT *point);
	virtual int OnKeyDown(UINT *vcKey);
	virtual int OnMouseMove(POINT *point);
	virtual int OnDraw(CDC *dc,RECT *rect,CUGCell *cell,int selected,int current);
	virtual void OnKillFocus(int col,long row,CUGCell *cell);

};

