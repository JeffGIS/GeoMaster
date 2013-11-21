

typedef struct UG_MSListTag{
	int		startCol,endCol;
	long	startRow,endRow;
	int		loCol,hiCol;
	long	loRow,hiRow;
	int		selected;
	UG_MSListTag * next;
}UG_MSList;


class CUGMultiSelect{

private:
	UG_MSList *	m_list;		//list of selected regions

	int			m_startCol;	//start region co-ords
	long		m_startRow;
	int			m_endCol;	//end region co-ords
	long		m_endRow;
	int			m_blockInProgress;
	UG_MSList *	m_currentItem;

	int			m_mode;	//selection mode  0:off  1:cell  2:row


	//enum info
	int			m_EnumStartCol;
	long		m_EnumStartRow;
	int			m_EnumEndCol;
	long		m_EnumEndRow;
	int			m_EnumCol;
	int			m_EnumRow;
	int			m_EnumInProgress;

public:

	UG_GridInfo *	m_GI;

	CUGMultiSelect();
	~CUGMultiSelect();

	int ClearAll();

	int StartBlock(int col,long row);
	int UpdateStartBlock(int col,long row);
	int EndBlock(int col,long row);
	int ToggleCell(int col,long row);

	int GetStartBlock(int *col,long *row);
	int GetCurrentBlock(int *startcol,long *startrow,int *endcol,long *endrow);
	int IsSelected(int col,long row);

	int AddToDrawHints(CUGDrawHint *dh);
	
	int SelectMode(int mode);	//0-off 1-cell selection 2- row selection

	int EnumFirstSelected(int *col,long *row);
	int EnumNextSelected(int *col,long *row);
};