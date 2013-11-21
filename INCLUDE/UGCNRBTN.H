// UGCnrBtn.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUGCnrBtn window

class CUGCnrBtn : public CWnd
{
// Construction
public:
	CUGCnrBtn();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUGCnrBtn)
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CUGCnrBtn();

	// Generated message map functions
protected:
	//{{AFX_MSG(CUGCnrBtn)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	// internal information
	UG_GridInfo *	m_GI;		//grid info structure
	CUGCtrl		*	m_Ctrl;		//pointer to the main class

	int				m_IsSizing;		//sizing flag
	int				m_CanSize;		//sizing flag
	int				m_SizingHeight;	//sizing flag
	int				m_SizingWidth;	//sizing flag

	//internal functions
	void Update();
};

/////////////////////////////////////////////////////////////////////////////
