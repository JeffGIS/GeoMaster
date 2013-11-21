

class CUGCell{


public:

	//do not modify directly, use the functions below
	UG_CellInfo m_ci;

	CUGCell();
	~CUGCell();

	//clear info
	void ClearInfo();

	//cell information copying functions
	int		CopyInfoTo(CUGCell *dest);
	int		CopyInfoFrom(CUGCell *source);
	int		CopyCellInfo(UG_CellInfo *src,UG_CellInfo *dest);

	//cell information appending funcitons - only updates items that are set
	int		AddInfoTo(CUGCell *dest);
	int		AddInfoFrom(CUGCell *source);
	int		AddCellInfo(UG_CellInfo *src,UG_CellInfo *dest);


	//***** text data type functions *****
	int		SetText(LPCSTR text);
	int		GetText(CString *text);
	int		AppendText(LPCSTR text);

	int		SetMask(LPCSTR text);
	int		GetMask(CString *text);

	int		SetLabelText(LPCSTR text);
	int		GetLabelText(CString *text);

	int		SetMinMaxTextLength(int min,int max);
	int		GetMinTextLength();
	int		GetMaxTextLength();
	
	//***** BOOL data type functions *****
	int		SetBool(BOOL state);
	BOOL	GetBool();

	//***** Number data type functions *****
	int		SetNumber(double number);
	int		GetNumber(int *number);
	int		GetNumber(long *number);
	int		GetNumber(float *number);
	int		GetNumber(double *number);
	double	GetNumber();

	int		SetMinMaxNumber(double min,double max);
	double	GetMinNumber();
	double	GetMaxNumber();
	int		SetNumDecimals(int number);
	int		SetCommas(BOOL state);
	int		SetCurrency(BOOL state);

	//***** Date data type functions *****
	int		SetDate(short day,short month,short year);
	int		GetDate(short* day,short* month,short* year);
	int		SetMinDate(short day,short month,short year);
	int		SetMaxDate(short day,short month,short year);
	int		GetMinDate(short *day,short *month,short *year);
	int		GetMaxDate(short *day,short *month,short *year);

	
	//***** Time data type functions *****
	int		SetTime(short second,short minute,short hour);
	int		GetTime(short* second,short* minute,short* hour);
	int		SetMinTime(short second,short minute,short hour);
	int		SetMaxTime(short second,short minute,short hour);
	int		GetMinTime(short *second,short *minute,short *hour);
	int		GetMaxTime(short *second,short *minute,short *hour);

	
	//***** display formats (data type to string conversion)
	int		SetDisplayFormat(short format);
	int		GetDisplayFormat();
	
	//***** set and get data types
	int		SetDataType(short type);
	int		GetDataType();
	int		SetDataColNumber(short col);
	int		GetDataColNumber();
	int		SetDataSrc(short datasource);
	int		GetDataSrc();
	
	//***** General functions *****
	int		SetMinMaxState(BOOL state);
	BOOL	GetMinMaxState();
	
	int			SetTextColor(COLORREF color);
	COLORREF	GetTextColor();
	int			SetBackColor(COLORREF color);
	COLORREF	GetBackColor();

	int			SetHTextColor(COLORREF color);
	COLORREF	GetHTextColor();
	int			SetHBackColor(COLORREF color);
	COLORREF	GetHBackColor();

	int		SetAlignment(short align);
	short	GetAlignment();

	int		SetBorder(short BorderStyle);
	short	GetBorder();
	int		SetBorderColor(HPEN pen);
	int		SetBorderColor(CPen *pen);
	HPEN	GetBordercolor();


	int		SetFont(HFONT font);
	int		SetFont(CFont *font);
	HFONT	GetFont();

	int		SetBitmap(HBITMAP bitmap);
	int		SetBitmap(CBitmap *bitmap);
	HBITMAP	GetBitmap();

	int		SetReadOnly(BOOL state);
	BOOL	GetReadOnly();


	int		SetCellType(long type);
	int		SetCellTypeEx(long type);
	long	GetCellType();
	long	GetCellTypeEx();

	//extra cell information
	int		ClearExtraMem();
	void *	AllocExtraMem(long len);
	void *	GetExtraMemPtr();
	long	GetExtraMemSize();

	//join information
	int SetJoinInfo(BOOL origin,int col,long row);
	int GetJoinInfo(BOOL *origin,int *col,long *row);
	int ClearJoinInfo();

	//cell saving functions
	long GetCellDataSize();
	int	GetRawCellData(void *data,int len);
	int	SetRawCellData(void *data);

	
	//masking functions
	int MaskString(CString * string);
	
	int StringToNumber(CString *string,double *number);
	int StringToBool(CString *string,BOOL * state);
	int StringToCurrency(CString *string,double *number);
	int StringToDate(CString *string,int *day,int *month,int *year);
	int StringToTime(CString *string,int *second,int *minute,int *hour);

	int NumberToString(double number,CString *string);
	int BoolToString(BOOL state,CString *string);
	int CurrencyToString(double number,CString *string);
	int DateToString(int day,int month,int year,CString *string);
	int TimeToString(int second,int hour,int minute,CString *string);
};