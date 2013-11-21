
typedef struct UGMemCITag{
	CUGCell *		cell;
	UGMemCITag *	next;
	UGMemCITag *	prev;
}UGMemCI;

typedef struct UGMemRITag{
	UGMemCI	*		col;	
	UGMemRITag *	next;
	UGMemRITag *	prev;
}UGMemRI;



class CUGMem: public CUGDataSource{

private:
	
	long		m_CurrentRow;	//current row
	UGMemRI	*	m_RowInfo;		//current row information pointer
	int			m_CurrentCol;	//current column
	UGMemCI	*	m_ColInfo;		//current column information pointer

	long		m_FindRow;	//current row for the findnext member

	int GotoRow(long row);	//moves the internal memory pointers to the row
	int GotoCol(int col);	//moves the internal memory pointers to the col
	int PrevRow();			//moves the internal memory pointers to the prev row
	int NextRow();			//moves the internal memory pointers to the next row
	int PrevCol();			//moves the internal memory pointers to the prev col
	int NextCol();			//moves the internal memory pointers to the next col

public:
	CUGMem();
	~CUGMem();

		//opening and closing
	virtual int Open(LPCSTR name,LPCSTR option);
	virtual int SetPassword(LPCSTR user,LPCSTR pass);
	
	virtual int Close();
	virtual int Save();
	virtual int SaveAs(LPCSTR name,LPCSTR option);

	//row and col info
	virtual long GetNumRows();
	
	virtual int GetNumCols();
	virtual int GetColName(int col,CString * string);
	virtual int GetColType(int col,int *type);	//0-string 1-bool 2-short 3-long 4-float 
													//5-double 6-currency 7-data 8-time
													//8-blob 9-graphic 10-ole
	
	//add-update-clear
	virtual int AppendRow();
	virtual int InsertRow(long row);
	virtual int AppendCol();
	virtual int InsertCol(int col);
	virtual int DeleteRow(long row);
	virtual int DeleteCol(long col);
	virtual int Empty();
	virtual int DeleteCell(int col,long row);

	//cell info
	virtual int	GetCell(int col,long row,CUGCell *cell);
	virtual int	SetCell(int col,long row,CUGCell *cell);

	//finding sorting
	virtual int FindFirst(int col,CString *string,long *row);
	virtual int FindNext(int col,CString *string,long *row);
	virtual int SortBy(int col);
	virtual int SortBy(int *cols,int num);

	//options
	virtual int SetOption(int option,long param1,long param2);
	virtual int GetOption(int option,long param1,long param2);

};