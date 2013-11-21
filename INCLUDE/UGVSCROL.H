// UGVScroll.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUGVScroll window

class CUGVScroll : public CScrollBar
{
// Construction
public:
	CUGVScroll();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUGVScroll)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CUGVScroll();

	// Generated message map functions
protected:
	//{{AFX_MSG(CUGVScroll)
	afx_msg void VScroll(UINT nSBCode, UINT nPos);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

public:
	//internal data
	UG_GridInfo *	m_GI;
	CUGCtrl		*	m_Ctrl;			//pointer to the main class

	double			m_MultiRange;	//scroll bar multiplication factor
									//for setting the scroll range
	double			m_MultiPos;		//multiplication factor for setting the 
									//top row during a thumb track
	long			m_LastMaxTopRow;//last max top row
	//internal functions
	void Update();
};

/////////////////////////////////////////////////////////////////////////////
