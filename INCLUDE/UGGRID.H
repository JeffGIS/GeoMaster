// UGGrid.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUGGrid window

class CUGGrid : public CWnd
{
// Construction
public:
	CUGGrid();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUGGrid)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CUGGrid();

	// Generated message map functions
protected:
	//{{AFX_MSG(CUGGrid)
	afx_msg void OnPaint();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


public:
	
	//internal data
	UG_GridInfo *	m_GI;			//pointer the the grid info data
	CUGCtrl		*	m_Ctrl;			//pointer to the main class
	int				m_lButtonDown;	//button down flag

	CUGCell			m_Cell;			//general purpose cell object

	CUGCellType*	m_CellTypeTrap;	//used to trap mouse messages

	CUGEdit	*		m_Edit;			//standard edit control

	int				m_counter;		//general purpose debugging counter

	int				m_HasFocus;


	long			m_MenuRow;		//co-ords of where the popup menu 
	int				m_MenuCol;		//was started from

	long			m_EditRow;		//co-ords of the last edit
	int				m_EditCol;


	CUGDrawHint		m_DrawHint;		//cell drawing hints


	int  StartEdit(int key);
	int  ContinueEdit();

	
	void Update();
	void DrawCellsIntern(CDC *dc);
	void DrawCellIntern(CDC *dc,RECT *rect,int col,long row);

	void RedrawCell(int col,long row);
	void RedrawRow(long row);
	void RedrawCol(int col);
	void RedrawRange(int startCol,long startRow,int endCol,long endRow);

};

/////////////////////////////////////////////////////////////////////////////
