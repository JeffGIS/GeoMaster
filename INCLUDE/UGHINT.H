// UGHint.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUGHint window

class CUGHint : public CWnd
{
// Construction
public:
	CUGHint();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUGHint)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CUGHint();

	// Generated message map functions
protected:
	//{{AFX_MSG(CUGHint)
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


public:

	UG_GridInfo *	m_GI;
	CUGCtrl *		m_Ctrl;		//pointer to the main class
	CString			m_Text;
	COLORREF		m_TextColor;
	COLORREF		m_BackColor;
	int				m_WindowAlign;
	int				m_TextAlign;
	HFONT			m_hFont;
	int				m_FontHeight;

	int	SetWindowAlign(int align);
	int SetTextAlign(int align);
	int SetTextColor(COLORREF color);
	int SetBackColor(COLORREF color);
	int SetFont(HFONT font);

	int SetText(LPCSTR string,int update);
	int MoveHintWindow(int x,int y,int width);

	int Hide();
	int Show();

};

/////////////////////////////////////////////////////////////////////////////
