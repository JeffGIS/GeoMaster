// UGCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUGCtrl window


#ifndef CUGCTRLINC
#define CUGCTRLINC

//standard grid classes
class CUGGrid;
class CUGTopHdg;
class CUGSideHdg;
class CUGCnrBtn;
class CUGVScroll;
class CUGHScroll;
class CUGCtrl;
class CUGCell;
class CUGCellType;
class CUGLstBox;
class CUGEdit;
class CUGHint;


//standard grid includes
#include "UGDefine.h"
#include "UGstruct.h"
#include "UGDrwHnt.h"
#include "UGCell.h"
#include "UGCnrBtn.h"
#include "UGGrid.h"
#include "UGHScrol.h"
#include "UGSideHd.h"
#include "UGTopHdg.h"
#include "UGVScrol.h"
#include "UGDtaSrc.h"
#include "UGMemMan.h"
#include "UGCelTyp.h"
#include "UGCBType.h"
#include "UGLstBox.h"
#include "UGDLType.h"
#include "UGCTafnt.h"
#include "UGCTarrw.h"
#include "UGCTprog.h"
#include "UGCTspin.h"
#include "UGEdit.h"
#include "UGMultiS.h"
#include "UGHint.h"
#include "UGPrint.h"


#ifdef __AFXOLE_H__  //OLE must be included
	#include "UGDrgDrp.h"
#endif


BOOL InitCUGCtrl();


class CUGCtrl : public CWnd
{
// Construction
public:
	CUGCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUGCtrl)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CUGCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CUGCtrl)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSysColorChange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()



protected:
	
	//fried classses
	friend CUGGrid;
	friend CUGTopHdg;
	friend CUGSideHdg;
	friend CUGHScroll;
	friend CUGVScroll;
	friend CUGCnrBtn;
	friend CUGHint;
	friend CUGCellType;
	friend CUGEdit;

	//***** internal information *****
	UG_GridInfo m_GI;		//grid info structure
		

	//***** internal classes *****
	//data sources
	CUGDataSource * m_DataSrc[UG_MAXDATASOURCES];
	int				m_DefDataSrc;
	CUGMem		  * m_CUGMem;	


	//3D pens
	CPen ThreeDLightPen;
	CPen ThreeDDarkPen;
	//multi-select
	CUGMultiSelect	* m_MS;			//multiselect class
	
	//standard cell types
	CUGCellType		m_NormalCellType;
	CUGCheckBoxType	m_CheckBoxType;
	CUGDropListType	m_DropListType;
	CUGArrowType	m_ArrowType;
	CUGAutoFontType m_AutoFontType;
	CUGProgressType m_ProgressType;
	CUGSpinButtonType m_SpinButtonType;

	//cell type list
	CUGCellType **	m_CellTypeList;
	int				m_CellTypeListLength;

	//popup menu
	CMenu			m_Menu;			

	CUGCell			m_Cell;			//general purpose cell object


	//***** internal functions *****
	void CalcTopRow();		//Calcs the max top row then adjusts the top row if needed
	void CalcLeftCol();		//Calcs the max left col then adjusts the left col if needed
	void AdjustTopRow();		//moves top row so that the current row is in view
	void AdjustLeftCol();		//move the left col so that the current col is in view
	void Update();				//updates all windows - also performs recalculations
	void Moved();				//this function is called whenever a grid movement is made
								//this function then notifies the child windows
	void AdjustChildSizes(); //adjusts and positions the child windows

	BOOL CreateChildWindows();//creates the child windows

	int VerifyTopRow(long newrow);
	int VerifyCurrentRow(long newrow);
	int VerifyLeftCol(int newcol);
	int VerifyCurrentCol(int newcol);

	
public:
	
	//***** child window classes *****
	CUGGrid		m_CUGGrid;
	CUGTopHdg	m_CUGTopHdg;
	CUGSideHdg	m_CUGSideHdg;
	CUGVScroll	m_CUGVScroll;
	CUGHScroll	m_CUGHScroll;
	CUGCnrBtn	m_CUGCnrBtn;
	CUGHint		m_CUGHint;
	CUGPrint	m_CUGPrint;

	//general routines
	void DrawBorder(CDC *dc,RECT *rect,RECT *rectout,long style,HPEN pen);
	void CalcDrawBorder(RECT *rect,long style); //recalcs the given rect for the area
												//inside the given border style


	//window creation
	BOOL CreateGrid(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

	//dialog resource functions
	AttachGrid(CWnd * wnd,UINT ID);

	//***** editing functions
	int		StartEdit();
	int		StartEdit(int key);
	int		StartEdit(int col,long row,int key);
	int		ContinueEdit(int adjustCol,long adjustRow);

	//***** Get and Set Functions *****
	//row and col
	int		SetNumberRows(long rows);
	int		SetNumberRows(long rows,int Redraw);
	long	GetNumberRows();
	
	int		SetNumberCols(int cols);
	int		SetNumberCols(int cols,int Redraw);
	int		GetNumberCols();

	int		SetDefColWidth(int width);
	int		SetColWidth(int col,int width);
	int		GetColWidth(int col,int *width);

	int		SetDefRowHeight(int height);
	int		SetUniformRowHeight(int flag);
	int		SetRowHeight(long row,int height);
	int		GetRowHeight(long row,int *height);

	int		GetCurrentCol();
	long	GetCurrentRow();
	int		GetLeftCol();
	long	GetTopRow();

	int		InsertCol(int col);
	int		AppendCol();
	int		DeleteCol(int col);
	int		InsertRow(long row);
	int		AppendRow();
	int		DeleteRow(long row);

	int FindFirst(int col,CString *string,long *row);
	int FindNext(int col,CString *string,long *row);
	int FindDialog();
	int ReplaceDialog();
	int SortBy(int col);
	int SortBy(int *cols,int num);


	//child window sizes
	int		SetTH_Height(int height);
	int		GetTH_Height();
	int		SetSH_Width(int width);
	int		GetSH_Width();
	int		SetVS_Width(int width);
	int		GetVS_Width();
	int		SetHS_Height(int height);
	int		GetHS_Height();

	//cell information
	int		GetCell(int col,long row,CUGCell *cell);
	int		GetCellIndirect(int col,long row,CUGCell *cell);
	int		SetCell(int col,long row,CUGCell *cell);
	int		SetCellIndirect(int col,long row,CUGCell *cell);
	int		PerformDataTranslations(BOOL state);
	int		DeleteCell(int col,long row);

	int		SetColDefault(int col,CUGCell *cell);
	int		GetColDefault(int col,CUGCell *cell);
	int		SetGridDefault(CUGCell *cell);
	int		GetGridDefault(CUGCell *cell);
	int		SetHeadingDefault(CUGCell *cell);
	int		GetHeadingDefault(CUGCell *cell);

	int     GetDataColNumber(int col);

	int		JoinCells(int startcol,long startrow,int endcol,long endrow);
	int		UnJoinCells(int col,long row);

	int		QuickSetText(int col,long row,LPCSTR string);
	int		QuickSetTextColor(int col,long row,COLORREF color);
	int		QuickSetBackColor(int col,long row,COLORREF color);
	int		QuickSetAlignment(int col,long row,int align);
	int		QuickSetBorder(int col,long row,int border);
	int		QuickSetFont(int col,long row,CFont *font);
	int		QuickSetBitmap(int col,long row,CBitmap *bitmap);
	int		QuickSetCellType(int col,long row,long type);
	int		QuickSetCellTypeEx(int col,long row,long typeEx);
	

	//general properties
	int		SetCurrentCellMode(int mode); //focus and highlighting
			//bit 1:focus 2:hightlight
	int		SetHighlightRow(int mode);	//on / off
	int		SetMultiSelectMode(int mode);
	int		Set3DHeight(int height);
	int		SetPaintMode(int mode);	//on off
	int     SetDefaultFont(HFONT hFont);
	int		SetVScrollMode(int mode);
	int		SetHScrollMode(int mode);
	

	//***** movement functions *****
	int MoveTopRow(int flag);	//0-lineup 1-linedown 2-pageup 3-pagedown 4-top 5-bottom
	int AdjustTopRow(long adjust);
	int MoveCurrentRow(int flag);
	int AdjustCurrentRow(long adjust);
	int GotoRow(long row);
	int	SetTopRow(long row);

	int MoveLeftCol(int flag);
	int AdjustLeftCol(int adjust);
	int MoveCurrentCol(int flag);
	int AdjustCurrentCol(int adjust);
	int GotoCol(int col);
	int	SetLeftCol(int col);

	int GotoCell(int col,long row);

	int GetCellFromPoint(int x,int y,int *col,long *row);
	int GetCellFromPoint(int x,int y,int *ptcol,long *ptrow,RECT *rect);
	int GetAbsoluteCellFromPoint(int x,int y,int *ptcol,long *ptrow);

	int GetCellRect(int col,long row,RECT *rect);
	int GetCellRect(int *col,long *row,RECT *rect);
	
	int GetJoinStartCell(int *col,long *row);
	int GetJoinStartCell(int *col,long *row,CUGCell *cell);
	int GetJoinRange(int *col,long *row,int *col2,long *row2);

	//cell type functions
	long AddCellType(CUGCellType *);   //returns the cell type ID
	int RemoveCellType(long ID);
	int SetCellTypeOption(long ID,long option,long param);
	int GetCellTypeOption(long ID,long option,long *param);
	int SetCellTypeEditMode(long ID,BOOL mode);

	//retrieves the pointer to a cell type class
	CUGCellType * GetCellType(long type);

	//data source functions
	int AddDataSource(CUGDataSource * ds);
	CUGDataSource * GetDataSource(int index);
	int RemoveDataSource(int index);
	int SetDefDataSource(int index);
	int SetGridUsingDataSource(int index);

	//redrawing functions
	int RedrawAll();
	int RedrawCell(int col,long row);
	int RedrawRow(long row);
	int RedrawCol(int col);
	int RedrawRange(int startCol,long startRow,int endCol,long endRow);

	//multi-select
	int ClearSelections();
	int Select(int col,long row);
	int SelectRange(int startCol,long startRow,int endCol,long endRow);
	int EnumFirstSelected(int *col,long *row);
	int EnumNextSelected(int *col,long *row);

	int CopySelected();
	int CutSelected();
	int CopySelected(int cutflag);  //TRUE,FALSE
	int Paste();
	int Paste(CString &string);
	int CopyToClipBoard(CString* string);
	int CopyFromClipBoard(CString* string);
	void CreateSelectedString(CString& string,int cutFlag);

	//column sizing
	int FitToWindow(int startCol,int endCol);
	int BestFit(int startCol,int endCol,int CalcRange);

	//printing
	int PrintDialog();
	int SelectPrinter(CPrintDialog * printer);
	int SetPrintOption(int option,long param);
	int PrintRange(long style,int startCol,long startRow,int endCol,long endRow);
	int PrintPreviewRange(long style,int startCol,long startRow,int endCol,long endRow);
	int GetPrintPageCount(long style,int startCol,long startRow,int endCol,long endRow);
	
	//hints 
	int UseHints(BOOL state);
	int UseVScrollHints(BOOL state);
	int UseHScrollHints(BOOL state);


	//Popup menu
	CMenu * GetPopupMenu();
	int EmptyMenu();
	int AddMenuItem(int ID,LPCSTR string);
	int RemoveMenuItem(int ID);
	int EnableMenu(BOOL state);

	//drag and drop
	int					m_DragStartFlag; //set to TRUE when the StartDragDrop is called
	#ifdef __AFXOLE_H__  //OLE must be included
		COleDataSource	m_DataSource;
		CUGDropTarget	m_DropTarget;

		int StartDragDrop();
		int DragDropTarget(BOOL state);
	#endif

	
	//***** Over-ridable Notify Functions *****
	virtual void OnSetup();

	//movement and sizing
	virtual int  OnCanMove(int oldcol,long oldrow,int newcol,int newrow);
	virtual int  OnCanViewMove(int oldcol,long oldrow,int newcol,int newrow);
	virtual void OnHitBottom(long numrows,long rowspast,long rowsfound);
	virtual void OnHitTop(long numrows,long rowspast);
	
	virtual int  OnCanSizeCol(int col);
	virtual void OnColSizing(int col,int *width);
	virtual void OnColSized(int col,int *width);
	virtual int  OnCanSizeRow(long row);
	virtual void OnRowSizing(long row,int *height);
	virtual void OnRowSized(long row,int *height);

	virtual int  OnCanSizeTopHdg();
	virtual int  OnCanSizeSideHdg();
	virtual int  OnTopHdgSizing(int *height);
	virtual int  OnSideHdgSizing(int *width);
	
	virtual void OnColChange(int oldcol,int newcol);
	virtual void OnRowChange(long oldrow,int newrow);
	virtual void OnCellChange(int oldcol,int newcol,long oldrow,int newrow);
	virtual void OnLeftColChange(int oldcol,int newcol);
	virtual void OnTopRowChange(long oldrow,int newrow);

	//mouse and key strokes
	virtual void OnLClicked(int col,long row,int updn,RECT *rect,POINT *point,int processed);
	virtual void OnRClicked(int col,long row,int updn,RECT *rect,POINT *point,int processed);
	virtual void OnDClicked(int col,long row,RECT *rect,POINT *point,int processed);
	virtual void OnTH_LClicked(int col,int updn);
	virtual void OnTH_RClicked(int col,int updn);
	virtual void OnTH_DClicked(int col);
	virtual void OnSH_LClicked(long row,int updn);
	virtual void OnSH_RClicked(long row,int updn);
	virtual void OnSH_DClicked(long row);
	virtual void OnCB_LClicked(int updn);
	virtual void OnCB_RClicked(int updn);
	virtual void OnCB_DClicked();
	
	virtual void OnKeyDown(UINT *vcKey,int processed);
	virtual void OnCharDown(UINT *vcKey,int processed);

	
	//GetCellIndirect notification
	virtual void OnGetCell(int col,long row,CUGCell *cell);
	//SetCell notification
	virtual void OnSetCell(int col,long row,CUGCell *cell);
	
	//data source notifications
	virtual void OnDataSourceNotify(int ID,long msg,long param);

	//cell type notifications
	virtual void OnCellTypeNotify(long ID,int col,long row,long msg,long param);

	//editing
	virtual int OnEditStart(int col, long row,CEdit *edit);
	virtual int OnEditVerify(int col,long row,CEdit *edit,UINT *vcKey);
	virtual int OnEditFinish(int col, long row,CEdit *edit);
	virtual int OnEditContinue(int oldcol,long oldrow,int* newcol,long* newrow);

	//menu notifications
	virtual void OnMenuCommand(int col,long row,int section,int item);
	virtual int  OnMenuStart(int col,long row,int section);

	//hints
	virtual int OnHint(int col,long row,CUGHint *hint);
	virtual int OnVScrollHint(long row,int percent,CUGHint *hint);
	virtual int OnHScrollHint(int col,int percent,CUGHint *hint);


	#ifdef __AFXOLE_H__  //OLE must be included

	//drag and drop
	virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject);
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject,int col,long row);
	virtual DROPEFFECT OnDragDrop(COleDataObject* pDataObject,int col,long row);

	#endif
};

/////////////////////////////////////////////////////////////////////////////


#endif
