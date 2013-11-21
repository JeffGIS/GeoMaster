// UGHScroll.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUGHScroll window

class CUGHScroll : public CScrollBar
{
// Construction
public:
	CUGHScroll();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUGHScroll)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CUGHScroll();

	// Generated message map functions
protected:
	//{{AFX_MSG(CUGHScroll)
	afx_msg void HScroll(UINT nSBCode, UINT nPos);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

public:
	//internal data
	UG_GridInfo *	m_GI;
	CUGCtrl		*	m_Ctrl;		//pointer to the main class

	//internal functions
	void Update();
};

/////////////////////////////////////////////////////////////////////////////
