

#ifdef __AFXOLE_H__  //OLE must be included

class CUGDropTarget: public COleDropTarget{


public:
	CUGCtrl		*	m_Ctrl;			//pointer to the main class

	virtual DROPEFFECT OnDragEnter( CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point );
	virtual DROPEFFECT OnDragOver( CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point );
	virtual DROPEFFECT OnDropEx( CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point );

};

#endif