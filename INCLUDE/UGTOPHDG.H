// UGTopHdg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUGTopHdg window

class CUGTopHdg : public CWnd
{
// Construction
public:
	CUGTopHdg();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUGTopHdg)
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CUGTopHdg();

	// Generated message map functions
protected:
	//{{AFX_MSG(CUGTopHdg)
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	//internal data
	UG_GridInfo *	m_GI;
	CUGCtrl		*	m_Ctrl;		//pointer to the main class
	CUGCell			m_Cell;		//general purpose cell class


	int				m_IsSizing;	//sizing flag
	int				m_CanSize;	//sizing flag
	int				m_SizingColumn;
	int				m_SizingStartWidth;
	int				m_SizingStartPos;

	long			m_MenuRow;		//co-ords of where the popup menu 
	int				m_MenuCol;		//was started from

	//internal functions
	void Update();
	int ColFromPoint(int x);
};

/////////////////////////////////////////////////////////////////////////////
