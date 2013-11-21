/***********************************************
************************************************/
class CUGDataSource{


public:

	CUGDataSource();
	virtual ~CUGDataSource();

	CUGCtrl * m_Ctrl;  //pointer to the main class

	
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
													//8-memo 9-blob 10-ole
	
	//add-update-clear
	virtual int AppendRow();
	virtual int InsertRow(long row);
	virtual int AppendCol();
	virtual int InsertCol(int col);
	virtual int DeleteRow(long row);
	virtual int DeleteCol(long col);
	virtual int DeleteCell(int col,long row);
	virtual int Empty();

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

	//verify
	//virtual int OnEditVerify(int col,long row,CEdit *edit)=0;
	
	//
	virtual int OnHitBottom(long numrows,long rowspast,long *rowsfound);

};