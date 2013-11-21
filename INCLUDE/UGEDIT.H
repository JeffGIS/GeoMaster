// UGEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUGEdit window

class CUGEdit : public CEdit
{
// Construction
public:
	CUGEdit();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUGEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CUGEdit();


	// Generated message map functions
protected:
	//{{AFX_MSG(CUGEdit)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnUpdate();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()


protected:

	int			m_MaskKeyInProgress;

	int			m_useMask;
	char		m_blankChar;
	BOOL		m_storeLiterals;
	char*		m_maskChars;
	
	CString		m_origMask;
	CString		m_mask;
	CString		m_literals;

	void SaveEdits();

	int  MaskKeyStrokes(UINT *vKey);
	void ShowInvalidEntryMessage();

public:

	//pointer to the main class
	CUGCtrl *	m_Ctrl;
	int			m_hasFocus;
	int			m_saveEdits;

	int SetMask(LPCSTR string);
	int MaskString(CString *string);	
};

/////////////////////////////////////////////////////////////////////////////
