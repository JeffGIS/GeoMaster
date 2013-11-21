// UGSideHdg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUGSideHdg window

class CUGSideHdg : public CWnd
{
// Construction
public:
	CUGSideHdg();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUGSideHdg)
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CUGSideHdg();

	// Generated message map functions
protected:
	//{{AFX_MSG(CUGSideHdg)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
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
	long			m_SizingRow;
	int				m_SizingStartHeight;
	int				m_SizingStartPos;

	long			m_MenuRow;		//co-ords of where the popup menu 
	int				m_MenuCol;		//was started from

	//internal functions
	void Update();
	long RowFromPoint(int y);

};

/////////////////////////////////////////////////////////////////////////////
