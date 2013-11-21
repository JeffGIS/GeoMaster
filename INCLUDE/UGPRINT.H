
typedef struct SUGPrintListTag{
	int page;
	int startCol;
	int endCol;
	long startRow;
	long endRow;

	SUGPrintListTag * next;

}SUGPrintList;


class CUGPrint{

private:

	int				m_PagesWide;
	int				m_PagesHigh;
	SUGPrintList*	m_PrintList;
	CPrintDialog*	m_PrintDlg;
	CPrintDialog*	m_ExternPrintDlg;
	float			m_PrintVScale;
	float			m_PrintHScale;
	int				m_PrintLeftMarginPX;
	int				m_PrintTopMarginPX;
	int				m_PrintRightMarginPX;
	int				m_PrintBottomMarginPX;
	int				m_PaperWidthPX;
	int				m_PaperHeightPX;


	//margins
	int	m_PrintLeftMargin;	//margin in mm
	int	m_PrintTopMargin;
	int	m_PrintRightMargin;
	int	m_PrintBottomMargin;


	int ClearPrintList();
	int	AddPageToList(int page,int startCol,int endCol,long startRow,long endRow);
	int	GetPageFromList(int page,int *startCol,int *endCol,long *startRow,long *endRow);
	int DCPrintPage(HDC hdc,int page,long style);
	void PrintCellIntern(CDC *dc,RECT * rect,int col,long row);

public:
	
	CUGCtrl *		m_Ctrl;
	UG_GridInfo *	m_GI;

	CUGPrint();
	~CUGPrint();

	int PrintDialog();
	int SelectPrinter(CPrintDialog * printer);
	int SetPrintOption(int option,long param);
	int PrintRange(long style,int startCol,long startRow,int endCol,long endRow);
	int PrintPreviewRange(long style,int startCol,long startRow,int endCol,long endRow);
	int GetPrintPageCount(long style,int startCol,long startRow,int endCol,long endRow);

};