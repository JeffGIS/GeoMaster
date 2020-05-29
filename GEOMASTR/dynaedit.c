#include "graphint.h"  
#include "extrndb.h"
#include <sqlext.h>

#define DEFINE_GLOBALS
#include "dyndlg.h" 
#undef DEFINE_GLOBALS


#define NUMEDIT 12
#define NUMCHECKBOX 4
#define NUMSTATIC 4
#define NUMENTERBUTTON 4
//#define NUMNUMERIC 4
#define NUMLISTBOX 4
#define NUMCOMBOBOX 4
#define NUMRADIO 4
#define TOTALEXTRA (NUMEDIT + NUMCHECKBOX + NUMSTATIC + NUMENTERBUTTON + \
 NUMLISTBOX + NUMCOMBOBOX + NUMRADIO)
#define EndedEditing  21999
#define RButtonDown  22000
#define NewEditString 22001
#define START_EDITOR  (WM_USER + 14)
#define GETMETRIC(x) GetSystemMetrics(x)
#define GETINSTANCE(w)  0//GetWindowInstance(w)
#define FRAMEINFLATE(r,i)  InflateRect(r,i*GETMETRIC(SM_CXFRAME),GETMETRIC(SM_CXFRAME))
#define W(r) (r.right - r.left+1)
#define H(r) (r.bottom - r.top+1)

static const UINT WDM_REDRAW = (WM_USER+111);
static const UINT WDM_GROUPMOVE = (WM_USER+112);
static const UINT WDM_RESIZE = (WM_USER+113);
static const UINT WDM_SELECT = (WM_USER+114);
static const char * WC_DLGEDIT = "WuiDialogEdit";
static const char * WC_DLGEDITMASK = "WuiDialogEditMask";
static const char * WC_DLGEDITCHILD = "WuiDialogEditChild";
static const char * WC_DLGTOOLKIT = "DYNAMIC_TOOLS";

 typedef struct
    {
      RECT Rect;
      HWND hWnd;
    } Rangle;
 typedef Rangle * lpRect;
typedef struct TEditInfo
   {
     HWND Child, DragSel, Dialog, OldParent, Mask, ToolKit,hModifyProc;
     HWND Selections[255];
     int NSelected, Recurse, DragSelect;
   } TEditInfo;
TEditInfo *  GlobEdit;

#include "gmextern.h"

static	LPOPENFILEDATA  FilePtr;
static	LPOPENSQLDATA   SQLPtr;
static	HGLOBAL hGlobal;
static	LPSTR lpS;
static char  MinVal[32],MaxVal[32],DefVal[32];
static HBITMAP hBitmaps[24][2];
static    char buffer[MAX_STRING+1];
static    char cx1[] = " 6    ";
static    char cx2[] = "] 76  ";
static    char cx3[] = " 150  ";
static    char cx4[] = "] 220 ";
static char EditString[128];
static char OutName[128]="", aValue[7];
static short	WhatEditType;
static HWND	GlobDialog;
static HWND	GlobhSQL;
static HANDLE	GlobalhThemeDB;
static GLOBALHANDLE	hDlgInfo=0;
static LPDLGINFO	lpDlgInfo;
static short	GlobNumItems;
static short	FldType;
static short	FldFmt;
static FIELDINFO	FIELD;
static FIELDINFO	FIELD2;
static LPFIELDINFO	lpFieldInfo=&FIELD;
static LPFIELDINFO	lpFirstInfo=&FIELD2;
static lpFldInfo	lpFI;
static lpFldInfo	lpItemInfo;
static lpFldInfo	lpF;
static short	DataFileType;
static short	BRtn;
static BOOL	GottaDB;
static BOOL	GottaName;
static BOOL	Loaded;
static BOOL	GlobalChange;
static RECT	WinRect;
static HANDLE	hCurrentValues;
static DWORD	cListItems;
static USHORT	FldLength;
static HANDLE	hThemeDB=0;
static POINT	pt;
static SIZE		txSize;
static LPDRAWITEMSTRUCT	lpdis;
static RECT	rectHi;
static RECT	rectHiDown;
static short	nSpaceWidth;
static short	nSpaceHeight;
static short	nOldBkMode;
static HDC	hDC;
static BITMAP	bm;
static DWORD	dwSize;
static short	iCurrent;
static short	i;
static short	j;
static short	x;
static short	y;
static HFILE	OutFid;
static DlgInfo	far*lpInfo=NULL;
static USHORT	NumDynamicDialogFields;
static USHORT	NumItems;
static HGLOBAL	hGlob;
static HGLOBAL	hDView;
static BOOL	GotSomething=FALSE;
static HWND	hDynamicParent;
static LPVIEWPORT	DynamicView;
static HWND	GlobWnd;
static HINSTANCE	GlobhInst;
static	BOOL GottaString;
static	UINT EditDialog;
static	lpRect lpR; 
static	short	SaveDataChanges;  
static	BOOL	CloseOnSave=TRUE;
static	short	SaveOnClose=2;
static	short	BorderOption=1;	
static	char	DDDataFile[256]="",DDSQL[1024], DDFile[128];
static	short	DDDataFileType=0;
static	HANDLE	hDynControls=0;   
static	HANDLE	hDDSQL=0; 
static	short	DynRecordExists; 
static	USHORT	lDynStrings,lInitialStrings;
static	HANDLE	hDynStrings=0,hDynInitialStrings=0;    
static	char	CurrentDynWhere[256], DynAutoVals[1024], DynOpenCommand[512], DynCloseCommand[512]; 
static	BOOL	AllowCreateNewRecord=FALSE;
static	HWND	DynWnd=0;
static	BOOL	DDFileUpdated=FALSE;     
static	char	DynName[80];

void DrawBitmap(HDC, short, short, HBITMAP, DWORD,short,short);
void SaveChanges(HWND hDlg);
BOOL FAR PASCAL DynamicMsgProc(HWND hDlg,WORD Msg,WPARAM wParam,LPARAM lParam);
BOOL FAR PASCAL OpenDBMsgProc(HWND hWndDlg,WORD Message,WPARAM wParam,LPARAM lParam);
BOOL FAR PASCAL InvisMsgProc(HWND hDlg,WORD Msg,WPARAM wParam,LPARAM lParam);
BOOL FAR PASCAL ComboBoxMsgProc(HWND hWndDlg,WORD Message,WPARAM wParam,LPARAM lParam);
//LRESULT CALLBACK __export ToolsMsgProc(HWND hWndDlg,WORD Message,WPARAM wParam,LPARAM lParam);
BOOL FAR PASCAL  ToolsMsgProc(HWND hWndDlg,WORD Message,WPARAM wParam,LPARAM lParam);
BOOL AddNewDlgItem(HWND Window,HWND Dialog, WORD Type);
BOOL FAR PASCAL EditStringMsgProc(HWND hWndDlg,UINT Message,WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL DIALOGSTYLEDynDialogMsgProc(HWND hWndDlg,UINT Message,WPARAM wParam, LPARAM lParam); 


//***************************************************// 
static RECT GetOtherRect(HWND Source, HWND Dest)
{
RECT ChildRect, New;
POINT P;
    GetWindowRect(Source, &ChildRect);
    P.x = ChildRect.left;
    P.y = ChildRect.top;
    ScreenToClient(Dest, &P);
    New.left = P.x; 
    New.top  = P.y;
    New.right = New.left + W(ChildRect);
    New.bottom = New.top + H(ChildRect);
    return New;
}
//*****************************************************//
static void BoundSelections(TEditInfo *Edit, HWND Parent)
{
int i;
RECT SelectRect = GetOtherRect(Edit->Selections[0],Parent);
   for(i = 1; i < Edit->NSelected; i++)
   {
     RECT Other = GetOtherRect(Edit->Selections[i],Parent);
     RECT Temp = SelectRect;
     UnionRect(&SelectRect, &Other, &Temp);
   }
   GlobEdit = Edit;
   FRAMEINFLATE(&SelectRect,1);
   SetWindowPos(Edit->Child, HWND_TOP, SelectRect.left,
        SelectRect.top, W(SelectRect), H(SelectRect), 0);
   ShowWindow(Edit->Child, SW_SHOWNA);
 }
//****************************************************************// 
 LRESULT CALLBACK ChildModifyProc(HWND Window, UINT Message, WPARAM Param1,
      LPARAM Param2)
{
    if(Message == WM_SYSCOMMAND && (Param1&0xFFF0) == SC_SIZE)
    {     PostMessage(GetParent(Window),WDM_RESIZE,0,0);}
    else if(Message == WM_LBUTTONDOWN)
    {
      RECT Start, Stop;
      GetWindowRect(Window,&Start);
      DefWindowProc(Window, WM_SYSCOMMAND, SC_MOVE+2,0);
      GetWindowRect(Window,&Stop);
      if(memcmp(&Start,&Stop, sizeof(Start)))
      {
        PostMessage(GetParent(Window),WDM_GROUPMOVE,
           Start.left - Stop.left, Start.top - Stop.top);
        PostMessage(GetParent(Window), WDM_REDRAW,0,0);
      }  
    }
    else if(Message == WM_RBUTTONDOWN || Message == WM_LBUTTONDBLCLK )
    {
      PostMessage(GetParent(Window), WM_RBUTTONDOWN,0,0);
    }
    else if(Message == WM_GETMINMAXINFO)       
    {
        MINMAXINFO FAR *Info = (MINMAXINFO FAR *) Param2;
        Info->ptMinTrackSize.x = 0;
        Info->ptMinTrackSize.y = 0;
        return 0;
    }
    else if(Message == WM_PAINT)        
        PostMessage(GetParent(Window), WDM_REDRAW,0,0);
    return DefWindowProc(Window, Message, Param1, Param2);    
    }
//****************************************************************// 
static HWND CreateChild(HWND Parent, int Move)
{

  return CreateWindow(WC_DLGEDITCHILD,"",
      WS_CHILD|WS_BORDER| (Move?0:WS_THICKFRAME),
      0,0,1,1, Parent, (HMENU) 0, GETINSTANCE(Parent),0);
 }         
static HWND CreateToolKit(HWND Parent)
{
DLGPROC dlgprc = (DLGPROC) MakeProcInstance(ToolsMsgProc, GETINSTANCE(Parent));

return CreateDialog(GETINSTANCE(Parent), "DYNAMIC_TOOLS", Parent, dlgprc);


 /* return CreateWindowEx(WS_EX_TOPMOST,"DYNAMIC_TOOLS","Toolkit",
      WS_OVERLAPPED|WS_CAPTION|WS_BORDER| WS_THICKFRAME|
      WS_SYSMENU|WS_VISIBLE| WS_POPUP ,//WS_CHILD| WS_CLIPSIBLINGS,
      400,200,120,220, Parent, (HWND) 0, GETINSTANCE(Parent),0); */
 }         
           
           
//****************************************************************// 
static void FillChild(HWND Parent, HWND Child)
{
  RECT R;
  GetClientRect(Parent,&R);
  MoveWindow(Child,0,0,W(R),H(R),FALSE);
}

LRESULT CALLBACK MaskProc(HWND Window, UINT Message, 
                WPARAM Param1, LPARAM Param2)
{
  if(Message == WM_MOUSEMOVE && (Param1&MK_LBUTTON))
  {
    DWORD D = (DWORD) GetWindowLong(Window,0);
    POINTS Start = MAKEPOINTS(D);
    POINTS Now = MAKEPOINTS(Param2);
    int Code;
    GlobalChange = TRUE;
    if(Now.x >= Start.x)
      Code = (Now.y >= Start.y)?8:5;
    else
      Code = (Now.y >= Start.y)?7:4;
    if(memcmp(&Start,&Now,sizeof(Start)))
      SendMessage(GetParent(Window),WDM_SELECT,Code,D);
  }
  else if(Message== WM_LBUTTONDOWN || Message == WM_LBUTTONUP)
  {
    if(Message == WM_LBUTTONDOWN)
    {
       SetWindowLong(Window,0, (long)Param2);
       return SendMessage(GetParent(Window),Message,Param1,Param2);
      }
  }    
    return DefWindowProc(Window,Message, Param1, Param2);
}
//****************************************************************// 
LRESULT CALLBACK ModifyProc(HWND Window, UINT Message,
          WPARAM Param1, LPARAM Param2)      
{ RECT R;
   TEditInfo *Edit = 0;
   HWND  Dialog = 0;
   if(Message == WM_CREATE)
   { 
     CREATESTRUCT *Info = (CREATESTRUCT *)Param2;
     Edit = (TEditInfo *) malloc(sizeof(*Edit));
     _fmemset(Edit,0,sizeof(*Edit));
     Edit->Dialog = (HWND)Info->lpCreateParams;
     Edit->hModifyProc = Window;
     SetParent(Edit->Dialog, Window);
     Edit->OldParent = Info->hwndParent;
     FillChild(Window, Edit->Dialog);
     GetWindowRect(Edit->Dialog,&R);
     Edit->Mask = CreateWindowEx(WS_EX_TRANSPARENT,
                  WC_DLGEDITMASK,"",WS_CHILD|WS_VISIBLE,
                  0,0,W(R),H(R),Window,(HMENU)0,
                  GETINSTANCE(Window),0);
     Edit->Child = CreateChild(Window,FALSE);
     Edit->DragSel = CreateChild(Window,TRUE);          
     Edit->ToolKit = CreateToolKit(Window);
     GlobalChange = FALSE;
     GotItUp = TRUE;
     SetWindowLong(Window,0,(long)Edit);
    i =  BringWindowToTop(Edit->Mask); 
    i =  BringWindowToTop(Edit->ToolKit); 
     return DefWindowProc(Window,Message,Param1,Param2);
   }
   Edit = (TEditInfo *) GetWindowLong(Window,0);
   GlobEdit = Edit;
   if(Edit != 0)
      Dialog = Edit->Dialog;
   else
      return DefWindowProc(Window,Message,Param1,Param2);
      
  //our keyboard interface
  if(Message == WM_KEYUP)
  {
    if(Param1 == VK_DELETE)    
    {
      if(Edit->NSelected)
      {
         int i;
         for(i=0;i < Edit->NSelected; ++i)
         { 
           lpF = lpItemInfo;
           j = GetDlgCtrlID(Edit->Selections[i]);
           while(lpF->FI.name[0])
           {
             if(lpF->FldID == j)
             {
              lpF->FldID = 0;//indicates it was deleted
              lpF->FI.length = -1;
              lpF->TempFldLen = -1;
              if(lpF->hCurrValue)
                 GSSiGlobUlFree (&lpF->hCurrValue);
              break;
             }
             lpF++;
           }
           DestroyWindow(Edit->Selections[i]);
         }
         ShowWindow(Edit->Child, SW_HIDE);
         Edit->NSelected = 0;
         GlobalChange = TRUE;
         GlobEdit = Edit;
      }   
    }
    else if(Param1 == VK_TAB)
    {
      HWND Next = 0;
      int Flag = GW_HWNDNEXT;
      if(GetAsyncKeyState(VK_SHIFT) & 0X8000)
         Flag = GW_HWNDPREV;
      if(Edit->NSelected > 0)
         Next = GetWindow(Edit->Selections[Edit->NSelected-1],Flag);
      if(Next == 0)
      {
         //Next - GetWindow(Dialog, GW_CHILD);
         if(Next != 0 && Flag == GW_HWNDPREV)
            Next = GetWindow(Next,GW_HWNDLAST);
      }
      Edit->NSelected = 0;
      if(Next != 0)
      {
        Edit->Selections[Edit->NSelected++] = Next;
        BoundSelections(Edit,Window);
        PostMessage(Window,WDM_REDRAW,0,0);
      }  
     }         
         GlobEdit = Edit;
    }
    else if(Message == WM_RBUTTONDOWN || Message == WM_LBUTTONDBLCLK )
    { //I want the user to have the ability to edit the field value
       DLGPROC lpfnEditStringMsgProc,lpfnComboBoxMsgProc;
       int epyT, Type,CntlNum;
       if(Edit->NSelected != 1) 
           return DefWindowProc(Window,Message,Param1,Param2); 
       epyT = GetDlgCtrlID(Edit->Selections[0]);
       Type = (epyT>>8);
       Type = HIBYTE(epyT);
       CntlNum = LOBYTE (epyT);
     switch (Type)
     {
      case TYPE_RADIO: 
      case TYPE_CHECKBOX:
         WhatEditType = TYPE_CHECKBOX;
         GetDlgItemText(Edit->Dialog, GetDlgCtrlID(Edit->Selections[0]),EditString,128);
         lpfnEditStringMsgProc = MakeProcInstance((DLGPROC)EditStringMsgProc, GlobhInst);
         DialogBox(GlobhInst, (LPSTR)"TEXTSTRING", GetParent(Window), lpfnEditStringMsgProc);
         FreeProcInstance(lpfnEditStringMsgProc);
         if(EditString[0])
         {//the user wants to change the value in this rectangles field
           SetDlgItemText(Edit->Dialog, GetDlgCtrlID(Edit->Selections[0]),
                          EditString);
           Edit->NSelected = 0;
           ShowWindow(Edit->Child,SW_HIDE);
           UpdateWindow(Edit->Dialog);
           GlobalChange = TRUE;
         }  
      break;
      case  TYPE_ENTERBUTTON:
    //  case  TYPE_NUMERIC:
      case  TYPE_EDIT:
      case  TYPE_LTEXT:  
           WhatEditType = Type;
           GetDlgItemText(Edit->Dialog, GetDlgCtrlID(Edit->Selections[0]),
                          EditString,128);
           lpfnEditStringMsgProc = MakeProcInstance((DLGPROC)EditStringMsgProc, GlobhInst);
         if(Type == TYPE_EDIT )
         {
           DialogBox(GlobhInst, (LPSTR)"DYNAMIC_PROPERTIES", GetParent(Window), lpfnEditStringMsgProc);
         }  
         else
         {
           DialogBox(GlobhInst, (LPSTR)"DYNAMICFIELD", GetParent(Window), lpfnEditStringMsgProc);
         }  
         FreeProcInstance(lpfnEditStringMsgProc);
         if(Type == TYPE_EDIT)
         {//CntlNum points to the control number just modified;
          
          
         } 
         if(EditString[0])
         {//the user wants to change the value in this rectangles field
           SetDlgItemText(Edit->Dialog, GetDlgCtrlID(Edit->Selections[0]),
                          EditString);
           Edit->NSelected = 0;
           ShowWindow(Edit->Child,SW_HIDE);
           UpdateWindow(Edit->Dialog);
           GlobalChange = TRUE;
         }
         break;
      case TYPE_LISTBOX:
      case TYPE_COMBOBOX:
         WhatEditType = Type;
         if(Type == TYPE_LISTBOX)
           cListItems = SendDlgItemMessage(Edit->Dialog, epyT, LB_GETCOUNT, 0, 0);
         else
           cListItems = SendDlgItemMessage(Edit->Dialog, epyT, CB_GETCOUNT, 0, 0);
         hCurrentValues = GSSiGlobAlloc ( 520,GHND,18000);
         if(cListItems)
         {//i load them into memory for Combobox to unload and work with
           lpS = (LPSTR) GlobalLock(hCurrentValues);
           for(i = 0;i < cListItems;i++)
           {
            if(Type == TYPE_LISTBOX)
               j = SendDlgItemMessage(Edit->Dialog, epyT, 
                            LB_GETTEXT, i, (LPARAM) lpS);
            else
               j = SendDlgItemMessage(Edit->Dialog, epyT, 
                            CB_GETLBTEXT, i, (LPARAM)lpS);
            
            if(j)lpS = _fstrchr(lpS,'\0') + 1;
           }
           GlobalUnlock(hCurrentValues);
         }
         GlobalChange = TRUE;
         lpfnComboBoxMsgProc = MakeProcInstance((DLGPROC)ComboBoxMsgProc, GlobhInst);
         DialogBox(GlobhInst, (LPSTR)"COMBOBOX", GetParent(Window), lpfnComboBoxMsgProc);
         FreeProcInstance(lpfnComboBoxMsgProc);
         if(cListItems)
         {//the user wants to change the value in this rectangles field
           lpS = GlobalLock(hCurrentValues);
           if(Type == TYPE_LISTBOX)
               SendDlgItemMessage (Edit->Dialog,GetDlgCtrlID(Edit->Selections[0]),
                                  LB_RESETCONTENT,0,0);
           else
               SendDlgItemMessage (Edit->Dialog,GetDlgCtrlID(Edit->Selections[0]),
                                  CB_RESETCONTENT,0,0);
                                    
           for(i=0;i<cListItems;i++)
           {
            if(Type == TYPE_LISTBOX)
               SendDlgItemMessage (Edit->Dialog,GetDlgCtrlID(Edit->Selections[0]),
                                 LB_ADDSTRING,0, (LPARAM)lpS++);
            else
               SendDlgItemMessage (Edit->Dialog,GetDlgCtrlID(Edit->Selections[0]),
                                 CB_ADDSTRING,0, (LPARAM)lpS++);
               lpS = _fstrchr(lpS,'\0');
               lpS++;
           }
           GlobalUnlock(hCurrentValues);
           Edit->NSelected = 0;
           ShowWindow(Edit->Child,SW_HIDE);
           UpdateWindow(Edit->Dialog);
           GlobEdit = Edit;
         }//if(cITemList)
       break;
      default: { }
     }//end of the switch   
    }    
    else if(Message == WM_WINDOWPOSCHANGED)
    {
      const WINDOWPOS *Info = (const WINDOWPOS FAR *) Param2;
      if((Info->flags & (SWP_NOMOVE|SWP_NOSIZE))
        == (SWP_NOMOVE|SWP_NOSIZE)) return TRUE;
      FillChild(Window, Edit->Dialog);
      FillChild(Window, Edit->Mask);
           GlobalChange = TRUE;
      return TRUE;
    }
    else if(Message == WDM_GROUPMOVE && Edit->NSelected)
    {
      int XOff = (int) Param1;
      int YOff = (int) Param2;
      int i;
      for (i = 0; i <Edit->NSelected; ++i)
      {
        RECT R;
        POINT P;
           GlobalChange = TRUE;
        GetWindowRect(Edit->Selections[i],&R);
        P.x = R.left - XOff;P.y = R.top - YOff;
        ScreenToClient(Dialog,&P);
        MoveWindow(Edit->Selections[i],P.x,P.y,
            W(R), H(R),FALSE);
      }
      UpdateWindow(Edit->Dialog);
      ShowWindow(Edit->Child, SW_SHOW);
    }
    else if(Message == WDM_RESIZE && Edit->NSelected == 1)
    {
      RECT R; POINT P;
      GetClientRect(Edit->Child, &R);
      P.x = R.left; P.y = R.top;
      MapWindowPoints(Edit->Child, Dialog, &P,1);
      MoveWindow(Edit->Selections[0],P.x,P.y,
            W(R), H(R),FALSE);
           GlobalChange = TRUE;
    }
    else if(Message == WDM_REDRAW)
    {
      if(IsWindowVisible(Edit->Child))
      {
         if(Edit->Recurse)
            Edit->Recurse = 0;
         else
         {
            ++Edit->Recurse;
            ShowWindow(Edit->Child, SW_HIDE);
            UpdateWindow(Edit->Dialog);
            ShowWindow(Edit->Child,SW_SHOW); 
            GlobEdit = Edit;
         }
      }
    }
    else if(Message == WM_LBUTTONDOWN)
    {
      HWND Found;
      POINT P = POINTStoPOINT(MAKEPOINTS(Param2));
      MapWindowPoints(Window,Dialog,&P,1);
      Found = ChildWindowFromPoint(Dialog,P);
      if(Found == Dialog) Found = 0;
      if(Found == 0)
      {
         if(Edit->NSelected)
         {
           Edit->NSelected = 0;
           ShowWindow(Edit->Child,SW_HIDE);
           UpdateWindow(Edit->Dialog);
         }
         Edit->DragSelect  = TRUE;
         SetWindowPos(Edit->DragSel,HWND_TOP,
            LOWORD(Param2),HIWORD(Param2),1,1,
            SWP_NOACTIVATE|SWP_NOREDRAW);
       }
       else //mouse selected a child control
       {
          if(!(Param1&MK_SHIFT))
                  Edit->NSelected = 0;
          Edit->Selections[Edit->NSelected++] = Found;
          i = GetDlgCtrlID(Found);
          BoundSelections(Edit,Window);
          InvalidateRect(Edit->Child,0,TRUE);
          GlobDialog = Dialog; 
          GlobEdit = Edit;
       }          
    }   
    else if(Message == WDM_SELECT && Edit->DragSelect)
    {     
       int j;
       HWND Rover;
       RECT Bound;
       BringWindowToTop(Edit->DragSel);
       ShowWindow(Edit->DragSel,SW_SHOW);
       DefWindowProc(Edit->DragSel,WM_SYSCOMMAND,
          SC_SIZE+Param1,Param2);
       ShowWindow(Edit->DragSel,SW_HIDE);
       GetWindowRect(Edit->DragSel, &Bound);
       Edit->NSelected = 0;
       Rover = GetWindow(Dialog,GW_CHILD);
       for(j = 0;j < 255 && Rover != 0;++j)
       {
         RECT A,B;
         GetWindowRect(Rover,&A);
         if(IntersectRect(&B,&A,&Bound))
           Edit->Selections[Edit->NSelected++] = Rover;
         Rover = GetWindow(Rover, GW_HWNDNEXT);
         if(Rover == GetWindow(Dialog,GW_CHILD)) break;
       }  
       if(Edit->NSelected)
         BoundSelections(Edit,Window);
         Edit->DragSelect = FALSE;
         GlobEdit = Edit;
    }

    else if(Message == WM_DESTROY)
    {
      RECT R;
      GotItUp = FALSE;
      GetWindowRect(Dialog,&R);
      SetWindowPos(Edit->Dialog, (HWND)0,R.left,R.top,
          0,0,SWP_NOSIZE|SWP_NOREDRAW);
      SetParent(Dialog,Edit->OldParent);
      EnableWindow(Dialog,TRUE);
      SendMessage(Dialog,EndedEditing,0,0L);
    }
    else if(Message == WM_NCDESTROY)
    {
      free(Edit);
      SetFocus(Dialog);
    }
    else if(Message == WM_COMMAND)
    {
          switch(Param1)
      { 
          case IDC_DIALOG_PROPERTIES:
		{
			DLGPROC lpfnEDITDYNDIALOGMsgProc;
					
			lpfnEDITDYNDIALOGMsgProc = MakeProcInstance((DLGPROC)EDITDYNDIALOGMsgProc, hInst);
			GlobalChange = DialogBox(hInst, (LPSTR)"EDITDYNDIALOG", Window, lpfnEDITDYNDIALOGMsgProc);
			FreeProcInstance(lpfnEDITDYNDIALOGMsgProc);  
			break; 
		} 
          case IDB_COMBO:         
          {     int BlockSize, Rover;
                WORD wFieldType = TYPE_COMBOBOX;         
                AddNewDlgItem(Window,Dialog,wFieldType);
                MessageBeep(MB_ICONHAND);
            //    Rover = GetWindow(Window,GW_CHILD);
            //    UpdateWindow(GlobEdit->OldParent);
                GlobalChange = TRUE;
           }     
           break;
          case IDB_LIST:
          {
            WORD wFieldType = TYPE_LISTBOX;        
            AddNewDlgItem(Window,Dialog,wFieldType);
            MessageBeep(MB_ICONHAND);
         //   UpdateWindow(GlobEdit->OldParent);
            GlobalChange = TRUE;
          }  
          break;
          case IDB_HELP:
                WinHelp(Window,"gwizhelp\\Dynohelp.hlp",HELP_PARTIALKEY,(DWORD)"Theme Editing");
          break;          
          case IDB_HSPACE:         
          { BOOL ok;
            int wide[2], high,vspace, width;
            RECT A,B,C;
            POINT P;
            if(Edit->NSelected <= 2)break;
            GetWindowRect(Edit->Selections[Edit->NSelected-1],&A);
            GetWindowRect(Edit->Selections[Edit->NSelected-2],&B);
            wide[0] = B.left - A.right;
            wide[1] = A.left - B.right ;
            if(wide[1] > wide[0])// in left to right order
               vspace = 1;
            else // in right to left order
               vspace = 0;
            if(wide[vspace] < 0)wide[vspace] = 0;   
            _fmemcpy(&C,&B,sizeof(POINT)*2);
            ScreenToClient(Edit->Dialog,(POINT *)&C);
            ScreenToClient(Edit->Dialog,(POINT *)&C.right);
            for(i = Edit->NSelected-3 ; i >= 0; i--)
            {
              GetWindowRect(Edit->Selections[i],&B);
              high = B.bottom - B.top;
              width = B.right - B.left;
              ScreenToClient(Edit->Dialog,(POINT *)&B);
              if(vspace)//user chose in left to right order.. 
                        //I come off the left of C
                B.left = C.left - (wide[vspace] + width);
              else // user chose in right to left order... 
                   //I come off the right of C
                B.left = C.right + wide[vspace];
              ok = MoveWindow(Edit->Selections[i],B.left,B.top,
                         width,high,FALSE);
              B.bottom = B.top + high;
              B.right  = B.left + width;
              _fmemcpy(&C,&B,sizeof(POINT)*2);
            }    
            Edit->NSelected = 0;
            ShowWindow(Edit->Child,SW_HIDE);
            UpdateWindow(Edit->Dialog);
            Edit->DragSelect  = TRUE;
            SetWindowPos(Edit->DragSel,HWND_TOP,
             LOWORD(Param2),HIWORD(Param2),1,1,
             SWP_NOACTIVATE|SWP_NOREDRAW);
             GlobalChange = TRUE;
            break;
          }
          case IDB_VSPACE:         
          { BOOL ok;
            int high[2], wide,hspace, height;
            RECT A,B,C;
            POINT P;
            if(Edit->NSelected <= 2)break;
            GetWindowRect(Edit->Selections[Edit->NSelected-1],&A);
            GetWindowRect(Edit->Selections[Edit->NSelected-2],&B);
            high[0] = B.top - A.bottom;
            high[1] = A.top - B.bottom ;
            if(high[1] > high[0])// in descending order
               hspace = 1;
            else // in ascending order
               hspace = 0;
            if(high[hspace] < 0) high[hspace] = 0;      
            _fmemcpy(&C,&B,sizeof(POINT)*2);
            ScreenToClient(Edit->Dialog,(POINT *)&C);
            ScreenToClient(Edit->Dialog,(POINT *)&C.right);
            for(i = Edit->NSelected-3 ; i >= 0; i--)
            {
              GetWindowRect(Edit->Selections[i],&B);
              height = B.bottom - B.top;
              wide = B.right - B.left;
              ScreenToClient(Edit->Dialog,(POINT *)&B);
              if(hspace)//user chose in descending order.. 
                        //I come off the Top of C
                B.top = C.top - (high[hspace] + height);
              else // user chose in ascending... 
                   //I come off the bottom of C
                B.top = C.bottom + high[hspace];
              ok = MoveWindow(Edit->Selections[i],B.left,B.top,
                         wide,height,FALSE);
              B.bottom = B.top + height;
              B.right  = B.left + wide;
              _fmemcpy(&C,&B,sizeof(POINT)*2);
            }    
            Edit->NSelected = 0;
            ShowWindow(Edit->Child,SW_HIDE);
            UpdateWindow(Edit->Dialog);
            Edit->DragSelect  = TRUE;
            SetWindowPos(Edit->DragSel,HWND_TOP,
             LOWORD(Param2),HIWORD(Param2),1,1,
             SWP_NOACTIVATE|SWP_NOREDRAW);
             GlobalChange = TRUE;
            break;
          } 
          case IDB_RADIO:         
          {  
            WORD wFieldType = TYPE_RADIO;         
            AddNewDlgItem(Window,Dialog,wFieldType);
            MessageBeep(MB_ICONHAND);
        //    UpdateWindow(GlobEdit->OldParent);
            GlobalChange = TRUE;
          }
          break;
          case IDB_EDIT:         
          {
            WORD wFieldType = TYPE_EDIT;        
            AddNewDlgItem(Window,Dialog,wFieldType);
            MessageBeep(MB_ICONHAND);
          //  UpdateWindow(GlobEdit->OldParent);
                  GlobalChange = TRUE;
           }
           break;
          case IDB_CHECKBOX:         
          {
            WORD wFieldType = TYPE_CHECKBOX;       
            AddNewDlgItem(Window,Dialog,wFieldType);
            MessageBeep(MB_ICONHAND);
        //    UpdateWindow(GlobEdit->OldParent);
                  GlobalChange = TRUE;
          }
          break;
          case IDB_GROUP:         
                  MessageBeep(MB_ICONEXCLAMATION  );
                  GlobalChange = TRUE;
           break;
          case IDB_PUSH_BUTTON:         
          {
            WORD wFieldType = TYPE_ENTERBUTTON;         
            AddNewDlgItem(Window,Dialog,wFieldType);
            MessageBeep(MB_ICONHAND);
         //   UpdateWindow(GlobEdit->OldParent);
                  GlobalChange = TRUE;
          }
          break;
          case IDB_POINTER:         
                  MessageBeep(MB_ICONEXCLAMATION  );
           break;
          case IDB_STANDARD_HEIGHT:        
          { BOOL ok;
            int high, wide;
            RECT A,B;
            POINT P;
            if(Edit->NSelected <= 1)break;
            GetWindowRect(Edit->Selections[Edit->NSelected-1],&A);
            high = A.bottom - A.top;
            ScreenToClient(Edit->Dialog,(POINT *)&A);
            for(i=0;i<Edit->NSelected-1;i++)
            {
              GetWindowRect(Edit->Selections[i],&B);
              wide = B.right - B.left;
              ScreenToClient(Edit->Dialog,(POINT *)&B);
              ok = MoveWindow(Edit->Selections[i],B.left,B.top,
                         wide,high,FALSE);
              GetWindowRect(Edit->Selections[i],&B);
            }    
            Edit->NSelected = 0;
            ShowWindow(Edit->Child,SW_HIDE);
            UpdateWindow(Edit->Dialog);
            Edit->DragSelect  = TRUE;
            SetWindowPos(Edit->DragSel,HWND_TOP,
             LOWORD(Param2),HIWORD(Param2),1,1,
             SWP_NOACTIVATE|SWP_NOREDRAW);
             GlobalChange = TRUE;
            break;
          }        
          case IDB_STANDARD_WIDTH:        
          { BOOL ok;
            int high, wide;
            RECT A,B;
            POINT P;
            if(Edit->NSelected <= 1)break;
            GetWindowRect(Edit->Selections[Edit->NSelected-1],&A);
            wide = A.right - A.left;
            ScreenToClient(Edit->Dialog,(POINT *)&A);
            for(i=0;i<Edit->NSelected-1;i++)
            {
              GetWindowRect(Edit->Selections[i],&B);
              high = B.bottom-B.top;
             ScreenToClient(Edit->Dialog,(POINT *)&B);
             ok = MoveWindow(Edit->Selections[i],B.left,B.top,
                         wide,high,FALSE);
             GetWindowRect(Edit->Selections[i],&B);
            }    
            Edit->NSelected = 0;
            ShowWindow(Edit->Child,SW_HIDE);
            UpdateWindow(Edit->Dialog);
            Edit->DragSelect  = TRUE;
            SetWindowPos(Edit->DragSel,HWND_TOP,
            LOWORD(Param2),HIWORD(Param2),1,1,
            SWP_NOACTIVATE|SWP_NOREDRAW);
            GlobalChange = TRUE;
            break;
          }         
          case IDB_STATIC:         
          {
            WORD wFieldType = TYPE_LTEXT;         
            AddNewDlgItem(Window,Dialog,wFieldType);
            MessageBeep(MB_ICONHAND);
        //    UpdateWindow(GlobEdit->OldParent);
                  GlobalChange = TRUE;
          }
          case IDB_ALIGN_LEFT:
          { BOOL ok;
            int high, wide;
            RECT A,B;
            POINT P;
            if(Edit->NSelected <= 1)break;
            GetWindowRect(Edit->Selections[Edit->NSelected-1],&A);
            ScreenToClient(Edit->Dialog,(POINT *)&A);
            for(i=0;i<Edit->NSelected-1;i++)
            {
              GetWindowRect(Edit->Selections[i],&B);
              high = B.bottom-B.top;
              wide = B.right-B.left;
              ScreenToClient(Edit->Dialog,(POINT *)&B);
              ok = MoveWindow(Edit->Selections[i],A.left,B.top,
                         wide,high,FALSE);
              GetWindowRect(Edit->Selections[i],&B);
            }    
            Edit->NSelected = 0;
            ShowWindow(Edit->Child,SW_HIDE);
            UpdateWindow(Edit->Dialog);
            Edit->DragSelect  = TRUE;
            SetWindowPos(Edit->DragSel,HWND_TOP,
            LOWORD(Param2),HIWORD(Param2),1,1,
            SWP_NOACTIVATE|SWP_NOREDRAW);
            GlobalChange = TRUE;
            break;
          }
          case IDB_ALIGN_TOP:
          { BOOL ok;
            int high, wide;
            RECT A,B;
            POINT P;
            if(Edit->NSelected <= 1)break;
            GetWindowRect(Edit->Selections[Edit->NSelected-1],&A);
            ScreenToClient(Edit->Dialog,(POINT *)&A);
            for(i=0;i<Edit->NSelected-1;i++)
            {
              GetWindowRect(Edit->Selections[i],&B);
              high = B.bottom-B.top;
              wide = B.right-B.left;
              ScreenToClient(Edit->Dialog,(POINT *)&B);
              ok = MoveWindow(Edit->Selections[i],B.left,A.top,
                         wide,high,FALSE);
              GetWindowRect(Edit->Selections[i],&B);
            }    
            Edit->NSelected = 0;
            ShowWindow(Edit->Child,SW_HIDE);
            UpdateWindow(Edit->Dialog);
            Edit->DragSelect  = TRUE;
            SetWindowPos(Edit->DragSel,HWND_TOP,
            LOWORD(Param2),HIWORD(Param2),1,1,
            SWP_NOACTIVATE|SWP_NOREDRAW);
                  GlobalChange = TRUE;
            break;
          }         
          case IDB_ALIGN_RIGHT:
          { BOOL ok;
            int high, wide;
            RECT A,B;
            POINT P, TargetP;
            if(Edit->NSelected <= 1)break;
            GetWindowRect(Edit->Selections[Edit->NSelected-1],&A);
            wide = A.right - A.left;
            ScreenToClient(Edit->Dialog,(POINT *)&A);
            TargetP.x = A.left + wide;
            for(i=0;i<Edit->NSelected-1;i++)
            {
              GetWindowRect(Edit->Selections[i],&B);
              high = B.bottom-B.top;
              wide = B.right-B.left;
              ScreenToClient(Edit->Dialog,(POINT *) &B);
              B.left = TargetP.x - wide;
              ok = MoveWindow(Edit->Selections[i],B.left,B.top,
                              wide,high,FALSE);
              GetWindowRect(Edit->Selections[i],&B);
            }    
            Edit->NSelected = 0;
            ShowWindow(Edit->Child,SW_HIDE);
            UpdateWindow(Edit->Dialog);
            Edit->DragSelect  = TRUE;
            SetWindowPos(Edit->DragSel,HWND_TOP,
            LOWORD(Param2),HIWORD(Param2),1,1,
            SWP_NOACTIVATE|SWP_NOREDRAW);
                  GlobalChange = TRUE;
            break;
          }
          case IDB_ALIGN_BOTTOM:
          { BOOL ok;
            int high, wide;
            RECT A,B;
            POINT P, TargetP;
            if(Edit->NSelected <= 1)break;
            GetWindowRect(Edit->Selections[Edit->NSelected-1],&A);
            high = A.bottom - A.top;
            ScreenToClient(Edit->Dialog,(POINT *)&A);
            TargetP.y = A.top + high;
            for(i=0;i<Edit->NSelected-1;i++)
            {
              GetWindowRect(Edit->Selections[i],&B);
              high = B.bottom-B.top;
              wide = B.right-B.left;
             ScreenToClient(Edit->Dialog,(POINT *)&B);
             B.top = TargetP.y - high;
             ok = MoveWindow(Edit->Selections[i],B.left,B.top,
                         wide,high,FALSE);
             GetWindowRect(Edit->Selections[i],&B);
            }    
            Edit->NSelected = 0;
            ShowWindow(Edit->Child,SW_HIDE);
            UpdateWindow(Edit->Dialog);
            Edit->DragSelect  = TRUE;
            SetWindowPos(Edit->DragSel,HWND_TOP,
            LOWORD(Param2),HIWORD(Param2),1,1,
            SWP_NOACTIVATE|SWP_NOREDRAW);
                  GlobalChange = TRUE;
            break;
          }         
          
        }
      }
    return DefWindowProc(Window,Message,Param1,Param2); 
    
}
//****************************************************************// 
BOOL FAR PASCAL ComboBoxMsgProc(HWND hWndDlg,WORD Message,WPARAM wParam,LPARAM lParam)
{   
 static Choice;
 char  lps[128];

 if ((BRtn = DIALOGSTYLEDynDialogMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);

    switch(Message)
    {  
       case WM_CLOSE:
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break;                                       
       case WM_INITDIALOG:
           lpS = GlobalLock(hCurrentValues);
           SendDlgItemMessage (hWndDlg,IDC_COMBOBOX,
                                  CB_RESETCONTENT,0,0);
           for(i=0;i<cListItems;i++)
           {
               SendDlgItemMessage (hWndDlg,IDC_COMBOBOX,
                                 CB_ADDSTRING,0,(LPARAM) lpS);
               lpS = _fstrchr(lpS,'\0');
               lpS++;
           }
           GlobalUnlock(hCurrentValues);
 
           Choice = -1;
           return TRUE;

       case WM_COMMAND:
         switch(wParam)
         {  
            case IDOK:
            case IDQUIT: 
                cListItems = SendDlgItemMessage(hWndDlg,IDC_COMBOBOX, 
                             CB_GETCOUNT, 0, 0);
                if(cListItems)
                {//i load them into memory for Combobox to unload and work with
                  lpS = (LPSTR) GlobalLock(hCurrentValues);
                  for(i = 0;i <cListItems;i++)
                  {
                     j = SendDlgItemMessage(hWndDlg,IDC_COMBOBOX, 
                                  CB_GETLBTEXT, i,(LPARAM) lpS);
                     lpS = _fstrchr(lpS,'\0') + 1;
                  }
                  GlobalUnlock(hCurrentValues);
                }
                EndDialog(hWndDlg, FALSE);
                break;
            case IDCANCEL:
                 EndDialog(hWndDlg, FALSE);
                 break;
            case IDC_ADD:
               GetDlgItemText(hWndDlg,IDC_COMBOBOX,lps,128);                   
               if(SendDlgItemMessage(hWndDlg,IDC_COMBOBOX,
                               CB_FINDSTRING,-1,(LPARAM)lps) == CB_ERR)
                   SendDlgItemMessage (hWndDlg,IDC_COMBOBOX,
                                   CB_ADDSTRING,0,(LPARAM) lps);
               lps[0] = '\0';
               SetDlgItemText(hWndDlg,IDC_COMBOBOX,lps);                   
               Choice = -1;                    
                  GlobalChange = TRUE;
               return TRUE; 
            break;
            case IDC_DELETE_THIS:
               if(Choice == -1)return TRUE;
               SendDlgItemMessage (hWndDlg,IDC_COMBOBOX,
                                   CB_DELETESTRING,Choice,0);
               lps[0] = '\0';
               SetDlgItemText(hWndDlg,IDC_COMBOBOX,lps);
               Choice = -1;                    
                  GlobalChange = TRUE;
               return TRUE;
            case IDC_COMBOBOX:
                switch(HIWORD(wParam))
                {
                  case CBN_DROPDOWN:
                     break;
                  case CBN_DBLCLK:
                  case CBN_SELCHANGE:
            
                  Choice=SendDlgItemMessage(hWndDlg,IDC_COMBOBOX,
                                           CB_GETCURSEL,0,0);
                  return TRUE; 
                }
                break;      
         }//end of switch(wParam)

      default:{ }
   }//end of switch(Message)   

   return DefWindowProc(hWndDlg,Message,wParam,lParam); 

}  

//****************************************************************// 
static int RegisterWClass(HINSTANCE Instance, WNDPROC Proc,
   const char  *ClassName, LPCSTR Icon)
{
   WNDCLASS wc;
   _fmemset (&wc, 0, sizeof(wc));
   wc.style = CS_HREDRAW|CS_VREDRAW;
   wc.lpfnWndProc  = Proc;
   wc.hInstance = Instance;
   wc.cbWndExtra = 4;
   wc.hCursor = LoadCursor((HINSTANCE)0, Icon);
   wc.lpszClassName = ClassName;
   return RegisterClass(&wc);
}
//****************************************************************// 
static void ModifyInit(HINSTANCE Instance)
{
  static int Registered = FALSE;
  if(!Registered)
  { 
    int i;
    i = RegisterWClass(Instance, ModifyProc, WC_DLGEDIT, IDC_CROSS);
    i = RegisterWClass(Instance, MaskProc, WC_DLGEDITMASK, IDC_ARROW);
    i = RegisterWClass(Instance, ChildModifyProc, WC_DLGEDITCHILD, IDC_SIZE);
 //   i = RegisterWClass(Instance, ToolsMsgProc, WC_DLGTOOLKIT, IDC_ARROW);
                                 
    Registered = TRUE;
  }
}    
//****************************************************************// 
void TinyEditor(HWND Dialog)
{
  HINSTANCE Instance = GETINSTANCE(Dialog);
  RECT  DialogRect;
  GlobhInst = Instance;
  GlobWnd = Dialog;
  ModifyInit(Instance);
  EnableWindow(Dialog,FALSE);
  GetWindowRect(Dialog, &DialogRect);
  DialogRect.top -= GETMETRIC(SM_CYCAPTION);
  FRAMEINFLATE(&DialogRect,1);
  CreateWindow(WC_DLGEDIT,"Dynamic Dialog Editor",
      WS_CAPTION|WS_BORDER|WS_POPUP| WS_THICKFRAME|
      WS_SYSMENU|WS_VISIBLE|WS_CLIPSIBLINGS,
      DialogRect.left  , DialogRect.top  , W(DialogRect)  ,H(DialogRect),
      0,0, Instance, (void FAR *)Dialog);
}        
  
//****************************************************************// 
BOOL FAR PASCAL EditStringMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
 static TypeLen; 
 //TEditInfo * Edit;                  
 int    BRtn;
 if ((BRtn = DIALOGSTYLEDynDialogMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:
     { char buffer[128], *ptr;
       if(WhatEditType == TYPE_EDIT )
       {
          //next I load the field names into a combobox
           LPFIELDINFO lpField;
           SendDlgItemMessage (hWndDlg,IDC_FIELD_NAMES,
                                  CB_RESETCONTENT,0,0);
            SQLPtr = (LPOPENSQLDATA) GlobalLock (GlobhSQL);
            FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
            lpField = &FilePtr->FldInfo;                                  
            for(i = 0;i < FilePtr->NumFields;i++,lpField++)
           {
             SendDlgItemMessage (hWndDlg,IDC_FIELD_NAMES,
                    CB_ADDSTRING,0,(LPARAM) lpField->name);
           }
           GlobalUnlock(SQLPtr->OFHandle);
           GlobalUnlock(GlobhSQL);         
         lpF = lpItemInfo;
         for (lpF = lpItemInfo,i=0;i < 256;i++,lpF++)
         {
           if(lpF->FldID == GetDlgCtrlID(GlobEdit->Selections[0]))
           { 
             SetDlgItemText(hWndDlg,IDC_MINIMUM_VALUE,lpF->MinVal);
             SetDlgItemText(hWndDlg,IDC_MAXIMUM_VALUE,lpF->MaxVal);
             SetDlgItemText(hWndDlg,IDC_DEFAULT_VALUE,lpF->DefVal);
             if(lpF->FldFormat)
             SendDlgItemMessage(hWndDlg, IDC_RADIO3+lpF->FldFormat,
                    BM_SETCHECK, 1, 0);
             SetDlgItemText(hWndDlg,IDC_STRING,EditString);
             if(_fstricmp(lpF->FI.name,"SQL") == 0)
             {  
               SendDlgItemMessage(hWndDlg, IDC_RADIO2,
                    BM_SETCHECK, 1, 0);
             }       
             else if(_fstricmp(lpF->FI.name,"CALC") == 0)
             {
               SendDlgItemMessage(hWndDlg, IDC_RADIO1,
                    BM_SETCHECK, 1, 0);
             }
             else
             {
               SendDlgItemMessage(hWndDlg, IDC_RADIO3,
                    BM_SETCHECK, 1, 0);
             }       
             break;
           }
         }
       }
       else
       {
         SetDlgItemText(hWndDlg,IDC_STRING,EditString);
         SetDlgItemText(hWndDlg,IDC_MINIMUM_VALUE,"Minimum Value");
         SetDlgItemText(hWndDlg,IDC_MAXIMUM_VALUE,"Max Val");
         SetDlgItemText(hWndDlg,IDC_DEFAULT_VALUE,"Def Val");
       }  
       break; /* End of WM_INITDIALOG                                 */
      }   
    case WM_COMMAND:
       switch(wParam)
       {  
         case IDOK:
         {
          HWND  Rover;
          int checked, k, epyT, length;
		  short	irc;
          LPSTR lps, lpS;
          BOOL ok, NameChanged;
          char buffer[256], *ptr, id[256];
           GetDlgItemText(hWndDlg,IDC_STRING,id,256);
       if(WhatEditType == TYPE_EDIT)
       {
           GetDlgItemText(hWndDlg,IDC_STRING,lpF->FldContents,128);
           if(!lpF->FldContents[0])
           {
             _fstrcpy(lpF->FldContents,EditString);
             SetDlgItemText(hWndDlg,IDC_STRING,EditString);
             MessageBox(0,"You Must Provide A Contents",
             "Dynamic Dialog",MB_ICONSTOP);
             return TRUE;
           }
            for(i = 0,FldType = 1;i < 3;i++,FldType++)
            {
              checked = (int) SendDlgItemMessage(hWndDlg, IDC_RADIO1+i,
                    BM_GETCHECK, 0, 0);
              if(checked)break;
            }
            lpF->FldType = FldType;
            if(!checked)lpF->FldType = TYPE_FIELD;//default to Database Field Name  
           if(_fstrcmp(EditString,lpF->FldContents) != 0
               && lpF->FldType == TYPE_FIELD)
           {
TryAgain:     _fstrcpy(buffer,lpF->FldContents);
              ptr = _fstrchr(buffer,']');
              if(ptr)
                *ptr = '\0';
              else
              {
                ptr = _fstrchr(buffer,'\0');
                *ptr = ']';
                ptr++;
                *ptr = '\0';
                ptr = _fstrchr(lpF->FldContents,'\0');
                *ptr = ']';
                ptr++;
                *ptr = '\0';
              }  
              ptr = _fstrchr(buffer,'[');
              if(!ptr)
              {
                _fstrcpy(&lpF->FldContents[1],buffer);
                lpF->FldContents[0] = '[';
                SetDlgItemText(hWndDlg,IDC_STRING,lpF->FldContents);
                _fstrcpy(EditString,lpF->FldContents);
                goto TryAgain;
              }
              ptr++;
              
              SQLPtr = (LPOPENSQLDATA) GlobalLock (GlobhSQL);
              FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
              lpFieldInfo = &FilePtr->FldInfo; 
              j = 0;
              for (i=0;i<FilePtr->NumFields;i++,lpFieldInfo++) 
              {
                 if(_fstricmp(lpFieldInfo->name,ptr) == 0)
                 {//found another field name that matches
                  _fmemcpy(&lpF->FI,lpFieldInfo,sizeof(FIELDINFO));
                  lps = (LPSTR) GetExternalFieldData (FilePtr,
                                SQLPtr->SQL,&SQLPtr->hstmt,
                                lpFieldInfo, FALSE,0, &irc,
                                 FilePtr->NumFields, &FilePtr->FldInfo);
                  if(lpFieldInfo->type == SQL_CHAR || 
                     lpFieldInfo->type == SQL_VARCHAR ||
					 lpFieldInfo->type == SQL_UNKCHAR)
                     length = 256;//lpFieldInfo->length;
                  else
                     length = 24;
                  if(lpF->hCurrValue)
                  {
                    GlobalUnlock(lpF->hCurrValue);
                    GSSiGlobUlFree (&lpF->hCurrValue);
                  }
                  lpF->hCurrValue = GSSiGlobAlloc ( 521,GHND ,length+1);
                  _fstrcpy(lpF->FI.name,lpF->FldContents);
                  lpS = (LPSTR)GlobalLock(lpF->hCurrValue);
                  if(lps)_fstrcpy(lpS,lps);
                  GlobalUnlock(lpF->hCurrValue);
                  j = 1;
                  break;
                 }
              }
              GlobalUnlock(SQLPtr->OFHandle);
              GlobalUnlock(GlobhSQL);
              if(!j)
              {
                 MessageBox(0,"Unable to find This Field Name",
                 "Dynamic Dialog",MB_ICONINFORMATION);
                 return TRUE;
              }
           }
            for(i = 0,FldFmt = 1;i < 9;i++,FldFmt++)
            {
              checked = (int) SendDlgItemMessage(hWndDlg, IDC_RADIO4+i,
                    BM_GETCHECK, 0, 0);
              if(checked)break;
            }
            lpF->FldFormat = FldFmt;
            if(!checked)lpF->FldFormat = 0;//default to text 
            if(!lpF->FldFormat || lpF->FldType == TYPE_FIELD)lpF->FldFormat = 0;//default to text  
            GetDlgItemText(hWndDlg,IDC_STRING,lpF->FldContents,128);
            NameChanged = FALSE;
            if(_fstrcmp(lpF->FldContents,EditString) != 0)
            {i = GetDlgCtrlID(GlobEdit->Selections[0]);
             SetDlgItemText(GlobEdit->Dialog, GetDlgCtrlID(GlobEdit->Selections[0]),
                             lpF->FldContents);
             NameChanged = TRUE;
            } 
            GetDlgItemText(hWndDlg,IDC_MINIMUM_VALUE,lpF->MinVal,32);
            GetDlgItemText(hWndDlg,IDC_MAXIMUM_VALUE,lpF->MaxVal,32);
            GetDlgItemText(hWndDlg,IDC_DEFAULT_VALUE,lpF->DefVal,32);
           // if(NameChanged) UpdateWindow(GlobEdit->Dialog);  //GlobEdit->OldParent
              _fstrcpy(EditString,lpF->FldContents);
              PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         }
         else //wasn't an edit or numeric field
         {  
              _fstrcpy(EditString,  id);
              PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         }
              
          }
          break;
         case IDC_FIELD_NAMES:
             switch(HIWORD(wParam))
              {
               case CBN_DBLCLK:
               case CBN_SELCHANGE:
                 buffer[0] = '[';
                 j = SendDlgItemMessage(hWndDlg,IDC_FIELD_NAMES,CB_GETCURSEL,0,0);
                 SendDlgItemMessage(hWndDlg,IDC_FIELD_NAMES,CB_GETLBTEXT,j,(LPARAM)&buffer[1]);
                 _fstrcat(buffer,"]");
                 SetDlgItemText(hWndDlg,IDC_STRING,buffer);
              }
         break;
         case IDCANCEL:
           EndDialog(hWndDlg, FALSE);
          break;  
       }//end of the switch
       break;
    case WM_CLOSE:     
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
      break;
    default:
      return DefWindowProc(hWndDlg,Message,wParam,lParam);  
   }
   return TRUE;
}             
//****************************************************************// 
BOOL CreateDynamicDialog (LPSTR DBName, LPGWDHEADER lpGWDHead,
        int NumSelect, LPINT Selected, HWND hWndDlg, UINT DlgItem,
        HGLOBAL hSQL)
{ /* DBName is the database being used as the basis for this screen
     NumSelect is the number of fields chosen by the user
     Selected is a pointer to the field names in the DlgItem
     list of field names on the hWndDlg Dialog.  What
     I want to do: for each name, find it in lpFieldInfo,
     and copy it into another pointer structure.    */
      int length;
      LPSTR lps, pTAB;
      lpFldInfo lpT;
      char	str[1024];
      HGLOBAL hglob, *lphGlob;
                
		    if (!GetSaveName2 (hWndDlg,OutName,IDS_DYNAMIC_DIALOG,".DDO",IDS_FILEDYNDIA))
		    	return FALSE;
            ExpandText (OutName);
            OutFid = GSSiOpenFile (OutName,0,OF_CREATE);    
            if (OutFid == HFILE_ERROR)
            {
               MessageBox(0,"Unable to Open It","Dynamic Dialog",MB_ICONSTOP);
               return FALSE;
            } 
            SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
            GlobhSQL = hSQL;

            FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
            lpFieldInfo = &FilePtr->FldInfo;
            hGlobal = GSSiGlobAlloc ( 522, GHND, sizeof(FldInfo)* (NUMEDIT+NumSelect));
            if(!hGlobal) return FALSE;
            lpItemInfo = (lpFldInfo) GlobalLock(hGlobal);
            lpF = lpItemInfo;
            SQLPtr->lastreadtime = 0;
            //ok, get the memory to hold the fieldinfo for these items
            for (j = 0; j < NumSelect;j++,lpF++,Selected++)
            { 
              SendDlgItemMessage(hWndDlg,DlgItem ,LB_GETTEXT, *Selected,
                    (LPARAM) ((LPSTR) str)); 
              if ((pTAB = _fstrchr (str,'\t')))
              	*pTAB = 0;
              _fstrcpy (lpF->FI.name,str);  
              ReplaceChar (lpF->FI.name,' ','|');
              lpFieldInfo = &FilePtr->FldInfo;
              for (i=0;i<FilePtr->NumFields;i++,lpFieldInfo++)
              { 
                 if(_fstricmp(lpF->FI.name,lpFieldInfo->name)==0) 
                 {
                  lpT = _fmemcpy(&lpF->FI,lpFieldInfo,sizeof(FIELDINFO)); 
                  switch (lpFieldInfo->type)
                  {
                  	case BT_CHAR:
                  		lpF->FI.type = SQL_CHAR;
                  		break;
					case BT_REAL:
					case BT_REAL4: 
					case BT_REAL8:
                  		lpF->FI.type = SQL_DOUBLE;
                  		break;
					case BT_INTEGER: 
					case BT_INT2:
					case BT_INT4:
				   		lpF->FI.type = SQL_INTEGER;
                  		break;
                  }
	              sprintf (str,"[%s]",lpF->FI.name);
	              ExpandText (str); 
	              length = _fstrlen (str);
	              //now I want to get the value currently held in this field 
	              lpF->hCurrValue = GSSiGlobAlloc ( 523,GHND,length+1);
	              lpS = (LPSTR)GlobalLock(lpF->hCurrValue);
	              _fstrcpy(lpS,str);
	              GlobalUnlock(lpF->hCurrValue); 
	            }
              }
            }
            GetWindowRect(hWndMain,&WinRect);
            _fstrcpy(buffer,"DIALOG  Dynamic_Dialog 1 1 2 16 18 300 206 100 ");   
            NumDynamicDialogFields = (NumSelect * 2) + 1 + TOTALEXTRA;
            itoa(NumDynamicDialogFields,aValue,10);
            _fstrcat(buffer,aValue);
            _fstrcat(buffer,"  '");
            _fstrcat(buffer,ThemeDB);
            _fstrcat(buffer,"'");
            fputstring(buffer,OutFid);
         //   fputstring("ENTERBUTTON Modify_This_Format  16 1 70 10 0 0",OutFid);
            x = 6; y = 30;
            lpF = lpItemInfo;
            
            for (i=0;i<NumSelect;i++,lpF++) 
            { _fstrcpy(buffer,"LTEXT ");
              _fstrcat(buffer,lpF->FI.name);
              _fstrcat(buffer,cx1);
              itoa(y,aValue,10);
              _fstrcat(buffer,aValue);
              _fstrcat(buffer," 60 8 40 12");
              fputstring(buffer,OutFid);
               _fstrcpy(buffer,"EDIT [");
              _fstrcat(buffer,lpF->FI.name);
              _fstrcat(buffer,cx2);
              itoa(y,aValue,10);
              _fstrcat(buffer,aValue);
              _fstrcat(buffer," 60 8 40 12");
              fputstring(buffer,OutFid);
              _fstrcpy(buffer,"[");
              _fstrcat(buffer,lpF->FI.name);
              _fstrcat(buffer,"]");
              fputstring(buffer,OutFid);
              _fstrcpy(buffer,"Minimum Value");
              fputstring(buffer,OutFid);
              _fstrcpy(buffer,"Maximum Value");
              fputstring(buffer,OutFid);
              _fstrcpy(buffer,"Default Value");
              fputstring(buffer,OutFid);
              lpF->FldType = 3;
              lpF->FldFormat = lpF->FI.type;
              lpF->TempFldLen = lpF->FI.length;
              y +=  9;
              if(y == 156)
              {
                y = 30;
                _fstrcpy(cx1, cx3);
                _fstrcpy(cx2, cx4);
              }  
            }
            
            for(i = 0;i<NUMEDIT;i++)
            {fputstring("EDIT [Field_Name]  299 2 2 2 0 0",OutFid);}
            for(i = 0;i<NUMCHECKBOX;i++)
            {fputstring("CHECKBOX [CheckBox]  299 2 2 2 0 0",OutFid);}
            for(i = 0;i<NUMSTATIC;i++)
            {fputstring("LTEXT [Static]  299 4 2 2 0 0",OutFid);}
            for(i = 0;i<NUMENTERBUTTON;i++)
            {fputstring("ENTERBUTTON [PushButton]  299 6 2 2 0 0",OutFid);}
          //  for(i = 0;i<NUMNUMERIC;i++)
          //  {fputstring("NUMERIC [Numeric]  299 8 2 2 0 0",OutFid);}
            for(i = 0;i<NUMLISTBOX;i++)
            {fputstring("LISTBOX [Listbox]  299 10 2 2 0 0",OutFid);}   
            for(i = 0;i<NUMCOMBOBOX;i++)
            {fputstring("COMBOBOX [ComboBox]  299 12 2 2 0 0",OutFid);}
            for(i = 0;i<NUMRADIO;i++)
            {fputstring("RADIOBUTTON [RadioButton]  299 30 2 2 0 0",OutFid);}
            GlobalUnlock (SQLPtr->OFHandle);
            GlobalUnlock (hSQL);
            GSSiClose2 (&OutFid);
            lpF = lpItemInfo;
            OpenModelessDialog(OutName); 
            
 return TRUE;
} 

BOOL UpdateDynamicData (HWND hDlg,BOOL FromSave)
{ 
   int Type, epyT, i,j,ii;
   char string[256]; 
   HANDLE	hUpStr;
   LPSTR	lpUpdate, lpEnd, lpStr;
   BOOL		rtn=FALSE;
           
   hUpStr = GSSiGlobAlloc ( 524,GMEM_MOVEABLE,4096);
   lpUpdate = GlobalLock (hUpStr);
   *lpUpdate = 0;
           
   lpF = lpItemInfo;
   for(i=0,lpF = lpItemInfo;lpF->FI.name[0];i++,lpF++)
   {   
   	   lpEnd = _fstrchr (lpF->FldContents,0);
   	   if (lpEnd == lpF->FldContents) goto Next;
   	   lpEnd--;
   	   if (*lpEnd != ']') goto Next;
   	   *lpEnd = 0; 
       epyT = lpF->FldID;
       Type = (epyT>>8);
       if(Type == TYPE_EDIT && lpF->hCurrValue)
       { 
           lpS = (LPSTR) GlobalLock(lpF->hCurrValue);
           GetDlgItemText(hDlg,epyT,string,sizeof(string)); 
           if (_fstrcmp (lpS,string)) 
           { 
           		lpStr = _fstrchr (lpUpdate,0); 
           		if (_fstrchr (lpF->FldContents,'|'))
           		{
           			char	tmp[66];
           			_fstrcpy (tmp,lpF->FldContents);
           			ReplaceChar (tmp,'|',' ');
           			sprintf (lpStr," \"%s\" = '%s',",&tmp[1],string); 
           		}
           		else
           			sprintf (lpStr," %s = '%s',",&lpF->FldContents[1],string);
           }
           GlobalUnlock(lpF->hCurrValue);
       }
       *lpEnd = ']'; 
 Next: ;
   }
   if (*lpUpdate && (FromSave || (SaveDataChanges && SaveOnClose)))
   {   
   		if (!FromSave && SaveOnClose == 2)
   		{
   			if (MessageBox (hDlg,"Save Changes?","Verify Update",MB_YESNO) == IDNO)
   				goto Exit; 
   		}
   		lpEnd = _fstrchr (lpUpdate,0);
   		lpEnd--;
   		*lpEnd = 0; 
//           		UpdateGMDFile (File,SQLPtr->SQL,lpUpdate,',');
   		rtn = UpdateExternalFieldData (FilePtr,SQLPtr->SQL,lpUpdate);
   }  
Exit:
   GSSiGlobUlFree (&hUpStr);
   return rtn;
}  
                                    
              
  // Opens dynamic modeless dialog box.
//****************************************************************// 
HWND OpenModelessDialog(char * filename)
{
    HWND hDlgBox=0;
    int	rc;
    LPSTR lpDlgTmp;

    if(0!=(hDlgInfo=ReadDialogDef(filename)))
    {
       DLGPROC lpDlgProc = (DLGPROC) MakeProcInstance(DialogProc, hInst);
       lpDlgInfo= (LPDLGINFO)GlobalLock(hDlgInfo);
       lpDlgTmp=GlobalLock(lpDlgInfo->dihDlgTmp);
       hDlgBox=CreateDialogIndirect(hInst,(LPCDLGTEMPLATEA)lpDlgTmp,0,lpDlgProc);
//       rc=DialogBoxIndirect(hInst,lpDlgTmp,hWndMain,lpDlgProc);
//       FreeProcInstance(lpDlgProc);
       GlobalUnlock(lpDlgInfo->dihDlgTmp);
      // GlobalFree(lpDlgInfo->dihDlgTmp);
       if(hDlgBox==0)
       {
        farfree(lpDlgInfo);
        hDlgInfo = 0;
        return 0;
       }
    }
    else
    	return 0;
    GlobalUnlock(hDlgInfo);
    SetProp(hDlgBox,PROP_DLGINFO,hDlgInfo);
    return hDlgBox;
}

// returns the index of the dialog box in hDlgBox[]
int FindDlgBox(HWND hDlg)
{
    int i;
    for(i=0;i<DLG_COUNT;i++)
    {
       if(hDlg==hDlgBox[i])
       break;
    }
    return i;
}
// CallBack for dynamic dialog
BOOL FAR PASCAL DialogProc(HWND hDlg,WORD Msg,WPARAM wParam,LPARAM lParam)
{   HWND hTest;
    int i,j;
    unsigned int Checked;
    char buffer[MAX_STRING+1];
 static WORD  sVPos;       /* Position of Vertical scrollbar slider     */
  WORD wP,four = 4;
HWND hwndCurrent, hwndStart;
 int    BRtn,ii;
 if ((BRtn = DIALOGSTYLEDynDialogMsgProc (hDlg,Msg, wParam, lParam))) return (BRtn);
  switch(Msg)
  { 
    case WM_INITDIALOG:
      {WORD wX,wY,wCX,wCY,wLength,wPosition;
       HFILE Fid;
        char buffer[200], str1[64],str2[64], *ptr;
       CREATESTRUCT *Info = (CREATESTRUCT *)wParam;
       int Type, epyT, i,j;
       char string[64];
       HWND  Rover = GetWindow(hDlg,GW_CHILD);
       hDynamicDialog = hDlg;   
       SaveDataChanges = TRUE;
       Ready = TRUE;     
       SetWindowText (hDlg,DynName);
       SetWindowPos (hDlg,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
       Fid = GSSiOpenFile (OutName,0,OF_READ);
        while(Rover)
        {
          epyT = GetDlgCtrlID(Rover);
          if(epyT != 0)
          {
            Type = (epyT>>8);
            switch (Type)
            {
              case TYPE_EDIT:
               ii=GetDlgItemText(hDlg,epyT,buffer,128);
               if(_fstricmp("[Field_Name]",buffer) ==  0)
               {//got to the stuff I want invisible
                 while(Rover)
                 {
                   ShowWindow(Rover,SW_HIDE);
                   Rover = GetWindow(Rover,GW_HWNDNEXT); 
                 }
                 goto Out;
               }
              break;
              case TYPE_COMBOBOX: 
              case TYPE_LISTBOX:
              {
               if(!fgetstring(buffer,200,Fid))
               	 goto Out;
                while(_fstrnicmp(buffer,"COMBOBOX",8) != 0 &&
                      _fstrnicmp(buffer,"LISTBOX",7) != 0)
                {
                  fgetstring(buffer,196,Fid);
                }//ok should have the next box.
                if(8!=sscanf(buffer,"%64s%64s%hd%hd%hd%hd%hd%hd",str1,
                str2,&wX,&wY,&wCX,&wCY,&wLength,&wPosition))
                {
                  break;
                }  
                for (i = 0;i < wLength;i++)
                { 
                   fgetstring(buffer,196,Fid);
                   if(Type == TYPE_COMBOBOX)
                     SendDlgItemMessage(hDlg,epyT,CB_ADDSTRING,0,(LPARAM)buffer);
                   else
                     SendDlgItemMessage(hDlg,epyT,LB_ADDSTRING,0,(LPARAM)buffer);
                }
                break;
              }
            default:{   }
            }
          }
          Rover = GetWindow(Rover,GW_HWNDNEXT); 
        }//end of the while loop   

Out:    GSSiClose2 (&Fid);   
   		if (CenterVP)
   			cwCenterInVP (hDlg,CenterVP);
   		else
			cwCenter(hDlg, 0); 
      }
     //I want to reset any combobox or listbox text items
     break;
     case EndedEditing:
       {    int Type, epyT, i,j;
           char string[64];
           HWND  Rover = GetWindow(hDlg,GW_CHILD);   
           
           SetWindowText (hDlg,DynName);
           for(i = 0;i < GlobNumItems;i++)
           {  
             while(Rover)
             {
               epyT = GetDlgCtrlID(Rover);
               if(epyT != 0)
               {
                 Type = (epyT>>8);
                 if(Type == TYPE_EDIT)
                 { 
                   lpF = lpItemInfo;
                   while(lpF->FI.name[0])
                   { if(lpF->FldID == epyT)
                     {
                       lpS = (LPSTR) GlobalLock(lpF->hCurrValue);
                       SetDlgItemText(hDlg,epyT,lpS);
                       GlobalUnlock(lpF->hCurrValue);
                       break;
                     }
                     lpF++;  
                   }
                   Rover = GetWindow(Rover,GW_HWNDNEXT); 
                   break;
                 }
               }  
               Rover = GetWindow(Rover,GW_HWNDNEXT); 
             } 
           }//end of for loop 
         }  
         break;
     case WM_COMMAND:
     { 
       switch(LOWORD(lParam))
       {
        case VK_TAB:
        {HWND Rover;
         int Type;
          Rover = GetFocus();
          Type = 0;
here:     Rover = GetWindow(Rover,GW_HWNDNEXT);
          if(Rover == 0)goto there; 
          Type = GetDlgCtrlID(Rover);
          Type = (Type>>8);
          if(Type == TYPE_EDIT ||Type == TYPE_CHECKBOX ||
          Type == TYPE_ENTERBUTTON ||Type == TYPE_LISTBOX ||
          Type ==  TYPE_COMBOBOX ||Type == TYPE_RADIO)
          {
             SetFocus(Rover);
             break;
          }
          else
           goto here;      
there:    Rover = GetWindow(hDlg,GW_CHILD);
          goto here;       
         } 
        break; 

		case IDC_SETVAL:
      	break;
      	  

        case IDM_CONTROL_M:
         if(ControlM && !GotItUp && Ready)
         {               //^already editing it    
           int Type, epyT, WitchOne;
           char string[64];
           HWND  Rover = GetWindow(hDlg,GW_CHILD);
           lpF = lpItemInfo;
           for(i = 0;i < GlobNumItems;i++)
           {  
             while(Rover)
             {
               epyT = GetDlgCtrlID(Rover);
               if(epyT != 0)
               {
                 Type = (epyT>>8);
                 if(Type == TYPE_EDIT)
                 { j = 1;
                   while(lpF->FI.length == -1 && j < 256){lpF++;j++;}
                   SetDlgItemText(hDlg,epyT,lpF->FldContents);
                   lpF->FldID = epyT;
                   lpF++;
                   Rover = GetWindow(Rover,GW_HWNDNEXT); 
                   break;
                 }
               }  
               Rover = GetWindow(Rover,GW_HWNDNEXT); 
             }//end of the while loop 
           }//end of for loop
           lpF = lpItemInfo;
           TinyEditor(hDlg);
           ControlM = FALSE;
           GotItUp = TRUE;
           break;
        }
       }
       switch(HIBYTE(wParam))
       {  
       
         case TYPE_ENTERBUTTON:
         { //user picked a pushbutton  
           int Type, epyT, i,j,WhichOne;
           char string[64];
           HWND  Rover = GetWindow(hDlg,GW_CHILD);
           WhichOne = wParam;
           GetDlgItemText (hDlg,WhichOne,string,64); 
           if (!_fstricmp (string,"Cancel")) 
           {
           		SaveDataChanges=FALSE; 
				PostMessage(hDlg, WM_SYSCOMMAND, SC_CLOSE, 0L);
           }
           if (!_fstricmp (string,"Save")) 
           {
	            UpdateDynamicData (hDlg,TRUE);
				if (CloseOnSave) 
				{
	           		SaveDataChanges=FALSE; 
					PostMessage(hDlg, WM_SYSCOMMAND, SC_CLOSE, 0L); 
				}
           }
		
         //  if(WitchOne != 0) return FALSE;

           break;     
         }    
        default:
            return FALSE;
      } //end of switch(HIBYTE(wParam))
       }//end of case WM_COMMAND:
       case WM_SYSCOMMAND:
       {
        if(wParam==SC_CLOSE)
        { int save;
          if(GlobalChange)
          {
           	SaveChanges(hDlg); 
            GlobalChange = FALSE;
          } 
          DestroyWindow(hDlg);
           return TRUE;
        }
        return FALSE;
       }
       case WM_CLOSE:
          DestroyWindow(hDlg);
          return TRUE;
       
       case WM_DESTROY:
       { 
       	 HWND	hWndParent=GetParent(hDlg);
       	 
       	 if (hWndParent)
         	SendMessage(GetParent(hDlg),IDC_CLOSE_DYNDLG,(LPARAM)hDlg,0L);
//         GlobalFree(RemoveProp(hDlg,PROP_DLGINFO));   
         RemoveProp(hDlg,PROP_DLGINFO);
//         lpDlgInfo = (LPDLGINFO)GlobalLock(hDlgInfo);
//         GlobalUnlock(lpDlgInfo->dihDlgTmp);
//         GlobalFree(lpDlgInfo->dihDlgTmp);

		 UpdateDynamicData (hDlg,FALSE);


         lpF = lpItemInfo;
         for(i=0,lpF = lpItemInfo;lpF->FI.name[0];i++,lpF++)
            GSSiGlobFree (&lpF->hCurrValue);     
         if (hThemeDB)
         	GlobalUnlock (hThemeDB);
     	 CloseDataFile(TRUE,&hThemeDB);
         lpDlgInfo = (LPDLGINFO)GlobalLock(hDlgInfo);
         GSSiGlobFree (&lpDlgInfo->dihDlgTmp);
         GSSiGlobUlFree (&hDlgInfo);
         GSSiGlobUlFree (&hGlobal);
         GSSiGlobUlFree (&hGlob); 
         hDynamicDialog =0;
         return FALSE;
       }
       default:
      return FALSE;
    }
	return FALSE;
}
 
//****************************************************************// 
GLOBALHANDLE ReadDialogDef(char *filename)
{
    int i=0, cntlid;
    WORD wX,wY,wCX,wCY,wItemCount,wFieldType=0,
     wPosition,wLength;
    char str1[MAX_STRING+1],str2[MAX_STRING+1];
    char buffer[200];
    char *cptr;
    GLOBALHANDLE hInfo=0;
    HFILE	Fid;
    long lStyle;
    LPSTR lps;
    HGLOBAL *lphGlob; 
    BOOL	rc=FALSE;
    
    do
    {
		Fid = GSSiOpenFile (filename,0,OF_READ); 
		fgetstring (buffer,199,Fid);
		//fgets(buffer,199,file);
		if(12 != sscanf(buffer,"%12s%80s%hd%d%hd%hd%hd%hd%hd%hd%hd%80s",str1,str2,&BorderOption,&CloseOnSave,&SaveOnClose,&wX,&wY,&wCX,&wCY,&wLength,&wItemCount,DataFile))
			break;
		hInfo=GSSiGlobAlloc ( 525,GHND, sizeof(DlgInfo)+((2*wItemCount-1)*sizeof(DlgItemInfo)));
		lpInfo=(LPDLGINFO)GlobalLock(hInfo);
		if(_fstrcmp(str1,"DIALOG"))
			break;
		if(!_fstrcmp(str2,"\"\""))
			str2[0]=0;
		else
		{   
			cptr = _fstrchr(str2,'|');
			while(cptr)
			{
				*cptr = ' ';
				cptr = _fstrchr(str2,'|');
			}  
		} 
		_fstrcpy (DynName,str2);
		switch (BorderOption)
		{
			case 1:
				lStyle=WS_POPUP | WS_VISIBLE |WS_SYSMENU | WS_DLGFRAME | WS_CAPTION; 
				break;
			case 2:
				lStyle=WS_POPUP | WS_VISIBLE |WS_THICKFRAME;
				break;
			case 3:
				lStyle=WS_POPUP | WS_VISIBLE |WS_SYSMENU;
		}
		lpInfo->diRecSize=wLength;
		GlobNumItems=0;
		
		if(0==(lpInfo->dihDlgTmp = NewDlgTemplate(lStyle,wX,wY,wCX,wCY,"","",str2,"system",0)))
			break;
		               
		for(i=0;i<wItemCount;i++)
		{
			//if(0==fgets(buffer,200,file))  
			if (!fgetstring (buffer,200,Fid))
				break;
			if(wFieldType == TYPE_COMBOBOX || wFieldType == TYPE_LISTBOX)
			{
				for(j = 0;j < wLength;j++)
				{//next I skip over any box items;
					if (!fgetstring (buffer,200,Fid))
						break;
				}
			}  
			if(8!=sscanf(buffer,"%80s%80s%hd%hd%hd%hd%hd%hd",str1,str2,&wX,&wY,&wCX,&wCY,&wLength,&wPosition))
			{
				i--;
				continue;
			}
			if(!_fstrcmp(str2,"\"\""))
				*str2=0;
			else
			{   
				cptr = _fstrchr(str2,'|');
				while(cptr)
				{
					*cptr = ' ';
					cptr = _fstrchr(str2,'|');
				}  
			}
			wFieldType=Lookup(str1,TypeNames);
			if(wFieldType>TYPE_NAME_COUNT-1)
			{
				i--;
				continue;
			}//next, for field type TYPE_EDIT, I replace the field name
		// with its value.
			if(wFieldType == TYPE_EDIT && lpF->hCurrValue)
			{
				_fstrcpy(lpF->FI.name,str2);
				lps = GlobalLock(lpF->hCurrValue);
				_fstrcpy(str2,lps);
				GlobalUnlock(lpF->hCurrValue);
			}
			lStyle=FieldStyles[wFieldType];
			if(!(cntlid = AddDlgItem(&lpInfo->dihDlgTmp,(DWORD)lStyle,(WORD)wX,(WORD)wY,
									(WORD)wCX,(WORD)wCY,(LPSTR)ClassNames[wFieldType],
									(LPSTR)str2,(BYTE)0,0,(BYTE)wFieldType)))
			{
				i--; 
				continue;
			}
			lpF->FldID = cntlid;
			if(_fstrcmp(str1,"EDIT") == 0 && _fstricmp("[Field_Name]",str2) != 0)
			{  char *ptr;
				if (!fgetstring (buffer,200,Fid))
					break;
				_fstrcpy(lpF->FldContents,buffer);
				if (!fgetstring (buffer,200,Fid))
					break;
				_fstrcpy(lpF->MinVal,buffer);
				if (!fgetstring (buffer,200,Fid))
					break;
				_fstrcpy(lpF->MaxVal,buffer);
				if (!fgetstring (buffer,200,Fid))
					break;
				_fstrcpy(lpF->DefVal,buffer);
				lpF++;
			}
			lpInfo->diItems[i].diiLength=wLength;
			lpInfo->diItems[i].diiPosition=wPosition;
			GlobNumItems++;
		}//end of the for loop  
		rc = TRUE;
    }
    while(0);
    if (!rc)
    	MessageBox (0,filename,"Invalid dialog file",MB_ICONEXCLAMATION);
    GSSiClose2 (&Fid);
       lpF = lpItemInfo;
    if(hInfo)
       GlobalUnlock(hInfo);
    return hInfo;
}

//****************************************************************// 
int Lookup(char far *key,char far *table[])
{
    int i;
    for(i=0;table[i]!=0;i++)
    {
       if(!_fstrcmp(key,table[i]))
          return i;
    }
    return -1;
}
/* bldtmp.c */

//#include <string.h>
//#define STRICT
//#include <windows.h>
#include "dyndlg.h"


typedef struct
    {
        long  dtStyle;
        BYTE  dtItemCount;
        int   dtX;
        int   dtY;
        int   dtCX;
        int   dtCY;
     } DlgTemplateHeader;
typedef DlgTemplateHeader FAR *LPDLGTEMPLATEHEADER;
typedef struct
    {
        int   dtilX;
        int   dtilY;
        int   dtilCX;
        int   dtilCY;
        int   dtilID;
        long  dtilStyle;
     }DlgItemTemplateHeader;

 
// Creates a dialog template. Returns
// NULL on failure.
GLOBALHANDLE NewDlgTemplate(DWORD Style,WORD X,WORD Y,
        WORD CX,WORD CY,LPSTR Menu,LPSTR Class,
        LPSTR Text,LPSTR TypeFace,WORD PtSize)
{
    DlgTemplateHeader far * lpDT;
    LPSTR lpsztemp;
    GLOBALHANDLE hDTemplate;
    long needed_size;

    needed_size=sizeof(DlgTemplateHeader)+
    _fstrlen(Menu)+1+lstrlen(Class)+1+lstrlen(Text)+1;

    if(TypeFace[0])
        needed_size+=_fstrlen(TypeFace)+1+sizeof(short);


    hDTemplate=GSSiGlobAlloc ( 526,GMEM_MOVEABLE | GMEM_ZEROINIT,
                        needed_size);
    if(hDTemplate==0)
       return hDTemplate;

    lpDT=(LPDLGTEMPLATEHEADER)GlobalLock(hDTemplate);

    lpDT->dtStyle=WS_VISIBLE | Style;
    lpDT->dtX=X;
    lpDT->dtY=Y;
    lpDT->dtCX=CX;
    lpDT->dtCY=CY;
     // here's where we copy in the variable length
     // elements
    lpsztemp=((LPSTR)lpDT)+sizeof(*lpDT);
    lpsztemp=_fmemccpy(lpsztemp,Menu,0,MAX_STRING+1);
    lpsztemp=_fmemccpy(lpsztemp,Class,0,MAX_STRING+1);
    lpsztemp=_fmemccpy(lpsztemp,Text,0,MAX_STRING+1);

    // if a Font was specified, build FontInfo
    if(TypeFace[0])
    {
       *((short far *)lpsztemp)=PtSize;
       _fmemccpy(lpsztemp+sizeof(short),TypeFace,0,
                          MAX_STRING+1);
       lpDT->dtStyle|=DS_SETFONT;
    }
    GlobalUnlock(hDTemplate);
    return hDTemplate ;
}


//****************************************************************// 
int AddDlgItem(GLOBALHANDLE *DlgTmp,DWORD Style,
           WORD X,WORD Y,WORD CX,WORD CY,
            LPSTR Class,LPSTR Text,
            BYTE DataLen,LPSTR Data,
            BYTE Type)
{   
	int	rtn=0;
	
    DlgTemplateHeader far * lpDT;
    DlgItemTemplateHeader far *lpDIT;
    LPSTR lpsztemp;
    GLOBALHANDLE hDTemplate;
    unsigned new_size,BlockSize;

    BlockSize=TemplateSize(*DlgTmp);

    new_size=BlockSize+sizeof(DlgItemTemplateHeader)+
           _fstrlen(Class)+1+lstrlen(Text)+1+1;

    // if DataLen is non-zero, reserve space for Data
    if(DataLen) new_size+=DataLen;

    hDTemplate=GSSiGlobalReAlloc (0,DlgTmp,new_size,
              GMEM_ZEROINIT| GMEM_MOVEABLE);

    if(hDTemplate==0)
       return FALSE;
    
    *DlgTmp = hDTemplate;
    lpDT=(LPDLGTEMPLATEHEADER)GlobalLock(hDTemplate);
    lpDIT=(DlgItemTemplateHeader far*)(((LPSTR)lpDT)+BlockSize);
    lpDIT->dtilX=X;
    lpDIT->dtilY=Y;
    lpDIT->dtilCX=min(CX,lpDT->dtCX);
    lpDIT->dtilCY=min(CY,lpDT->dtCY);
     // Control ID combines index and
     // Field type so DialogProc
     // will know how to handle control
    lpDIT->dtilID=lpDT->dtItemCount | (Type<<8); 
    rtn = lpDIT->dtilID;
    lpDIT->dtilStyle=WS_VISIBLE | WS_CHILD | Style;
    lpsztemp=(char far*)(lpDIT)+sizeof(*lpDIT);
    lpsztemp=_fmemccpy(lpsztemp,Class,0,MAX_STRING+1);
    lpsztemp=_fmemccpy(lpsztemp,Text,0,MAX_STRING+1);
    if(DataLen)
       _fmemcpy(lpsztemp,Data,DataLen);

    lpDT->dtItemCount++;

    GlobalUnlock(hDTemplate);

    return rtn;
}
// Adjusts the location of a HIDDEN Dialog window by
// relocating it into the visible portion of the client
// dialog.  This gives the uses the sense that they just
// created a NEW control to a dialog template. Returns
// FALSE if allocation fails.
//****************************************************************// 
BOOL AddNewDlgItem(HWND Window,HWND Dialog, WORD Type)
{
// each different type of control has a pool of unused
// controls hidden outside the client area of the dialog
// visible window.
   int iType, val;
       HWND Found;
      POINT pt;
      char cval[12];
   pt.x = 599;
   switch (Type)
   {
     case TYPE_EDIT:
           lpF = lpItemInfo;
           for(i = 0;i < 256;i++,lpF++)//find an unused structure
           {
             if(lpF->FI.length <= 0 && lpF->TempFldLen <= 0)
               break;
           } 
            _fstrcpy(lpF->FI.name,"[Field_Name]");
            _fstrcpy(lpF->FldContents,"[Field_Name]");
            lpF->FldType = 3;//A FIELD NAME
            lpF->FldFormat = 0;
            lpF->FI.length = 256;
            lpF->FI.type = TYPE_EDIT;
            lpF->TempFldLen = 256;
            if(lpF->hCurrValue)
            {
              GlobalUnlock(lpF->hCurrValue);
              GSSiGlobUlFree (&lpF->hCurrValue);
            }              
            lpF->hCurrValue = GSSiGlobAlloc ( 527,GHND,lpF->TempFldLen);
        pt.y = 1;
     break;
     case TYPE_CHECKBOX: 
        pt.y = 5;
     break;
     case TYPE_LTEXT: 
        pt.y = 9;
     break;
     case TYPE_ENTERBUTTON: 
        pt.y = 13;
     break;
     case TYPE_LISTBOX:
        pt.y = 20;
     break;
     case TYPE_COMBOBOX: 
        pt.y = 25;
     break;
     case TYPE_RADIO: 
        pt.y = 61;
     break;
   }  
      Found = ChildWindowFromPoint(Dialog,pt);
      if(Found == Dialog) Found = 0;
      if(Found != 0)
      {
           i=MoveWindow(Found,416,0,160,30, TRUE);
           i =ShowWindow(Found,SW_RESTORE);
           UpdateWindow(Dialog);
           if(Type ==  TYPE_EDIT)
              lpF->FldID = GetDlgCtrlID(Found);
       }
       else
       { 
         lpF->FI.length = 0;
         lpF->TempFldLen = 0;
         return FALSE;
       }  
    return TRUE;
}
// Scans a dialog box template and returns
// its length
//****************************************************************// 
int TemplateSize(GLOBALHANDLE hDlgTmp)

{
#include <memory.h>
#include <string.h>
#include <stdio.h>
   DlgTemplateHeader far * lpDT;
    // DlgItemTemplateHeader far *lpDIT;
    LPSTR lpstr,lpbase;
    int isize;
    int i;
    // first scan the DLGTEMPLATE
    lpDT=(DlgTemplateHeader far *)GlobalLock(hDlgTmp);
    lpbase=(LPSTR)lpDT;
    lpstr=(LPSTR)(lpDT+1);
    isize=sizeof(*lpDT);
    lpstr=lpbase+(isize+=(_fstrlen(lpstr)+1));// Menu[]
    lpstr=lpbase+(isize+=(_fstrlen(lpstr)+1));// Class[]
    lpstr=lpbase+(isize+=(_fstrlen(lpstr)+1));// Caption[]
    // if necessary, the FONTINFO
    if((lpDT->dtStyle)&DS_SETFONT)
    {
       isize+=sizeof(short);
       lpstr+=sizeof(short);
       lpstr=lpbase+(isize+=(_fstrlen(lpstr)+1));
    }
       // finally, each of the DLGITEMTEMPLATEs
    for(i=0;i<lpDT->dtItemCount;i++)
    {
         lpstr=lpbase+(isize+=sizeof(DlgItemTemplateHeader));
        lpstr=lpbase+(isize+=(_fstrlen(lpstr)+1));// Class[]
        lpstr=lpbase+(isize+=(_fstrlen(lpstr)+1));// Text[]
        if(*lpstr) // i.e. if DataLen is non zero
        {  
        	int	j=*(LPBYTE)lpstr;
        	
           isize+=j;
           lpstr+=*(LPBYTE)lpstr;
        }
        isize++;
    }
    GlobalUnlock(hDlgTmp);
    return isize;
}
//****************************************************************// 
void ShowDynWindows (void)
{   int i;

    if (!PopDynDlg) return;
    for(i=0;i<DLG_COUNT;i++) if(hDlgBox[i]) BringWindowToTop (hDlgBox[i]);
}
//****************************************************************// 
void CloseDynWindows (void)
{   int i;

    for(i=0;i<DLG_COUNT;i++) if(hDlgBox[i]) SendMessage(hWndMain,DYNM_CLOSE,(WPARAM)hDlgBox[i],0);
}


//****************************************************************// 
void DynDlgOn (BOOL On)
{
    PopDynDlg = On;
} 

//****************************************************************// 
 BOOL FAR PASCAL DynamicMsgProc(HWND hWndDlg,WORD Message,WPARAM wParam,LPARAM lParam)
{   
    int     IDC_FieldName=SV_FIELD_NAME;     
    BOOL    True=TRUE;
 if ((BRtn = DIALOGSTYLEDynDialogMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
  if (DATAFILEMsgProc(hWndDlg,Message,wParam,lParam, IDC_SQL,
        SV_SET_FILE, SV_DATABASE_LIST, SV_TABLE_NAMES, SV_TABLE_HEADING,&IDC_FieldName, 1,
        DataFile, &DataFileType, &GlobalhThemeDB,&True, TRUE))  return TRUE;  
        
 if (ThemeCommonCode (hWndDlg, Message, wParam, lParam,GlobalhThemeDB)) return TRUE;

    switch(Message)
    {
       case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */
       case WM_INITDIALOG:
       OutName[0] = '\0';
       DataFile[0] = '\0';
       GottaDB = FALSE;
       GottaName = FALSE;
       Loaded = FALSE;
       ShowWindow (GetDlgItem(hWndDlg,SV_TABLE_NAMES),SW_HIDE);
       ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_NAME),SW_HIDE);
       ShowWindow (GetDlgItem(hWndDlg,IDC_VIEW_DIALOG),SW_HIDE);
       ShowWindow (GetDlgItem(hWndDlg,IDC_LOAD_DIALOG),SW_HIDE);
       ShowWindow (GetDlgItem(hWndDlg,IDC_SAVE_AS),SW_HIDE);
      return TRUE;
       case WM_COMMAND:
         switch(wParam)
         {  
            //case IDCANCEL:
            case IDCANCEL:
            case IDOK:
            case IDQUIT:
               DestroyWindow(hWndDlg);
//                 EndDialog(hWndDlg, FALSE);
                 break;
           case IDC_OPEN_DB:
                  GottaDB = TRUE;
                  SetDlgItemText (hWndDlg,IDC_DYNAMIC_MESSAGE,
                  "Select a file to save the dialog in");
                  ShowWindow (GetDlgItem(hWndDlg,IDC_SAVE_AS),SW_SHOW);

           break;
           default:
            return FALSE;
         }   
       default:
      return FALSE;
    }
    return TRUE;
}
 
//****************************************************************// 
//*********************************************************//
void HandleFocusState(LPDRAWITEMSTRUCT lpdis)
{
    // if focus state, draw a dashed rect around text
    if (lpdis->itemState & ODS_FOCUS)
    {
        // if selected, shift focus rect right and down 2 pixels around text
        if (lpdis->itemState & ODS_SELECTED)
//            DrawFocusRect(lpdis->hDC, (LPRECT)&rectHiDown);
            DrawFocusRect(lpdis->hDC, (LPRECT)&lpdis->rcItem);
        // else text centered, so focus rect will be too
        else
//            DrawFocusRect(lpdis->hDC, (LPRECT)&rectHi);
            DrawFocusRect(lpdis->hDC, (LPRECT)&lpdis->rcItem);
    }
    return;
}


/*****************************************************************************
    FUNCTION: HandleSelectedState
    PURPOSE:  Draw button bitmaps
*****************************************************************************/
void HandleSelectedState(LPDRAWITEMSTRUCT lpdis)
{   
    switch (lpdis->CtlID)
    {
      case IDB_COMBO:
        iCurrent = 1;
        break;
      case IDB_LIST:
        iCurrent = 2;  
        break;
      case IDB_HSPACE:
        iCurrent = 3;
        break;
      case IDB_VSPACE:
        iCurrent = 4;  
        break;
      case IDB_ALIGN_TOP:
        iCurrent = 5;
        break;
      case IDB_ALIGN_BOTTOM:
        iCurrent = 6;  
        break;
      case IDB_ALIGN_LEFT:
        iCurrent = 7;
        break;
      case IDB_ALIGN_RIGHT:
        iCurrent = 8;  
        break;
      case IDB_RADIO:
        iCurrent = 9;
        break;
      case IDB_EDIT:
        iCurrent = 10;  
        break;
      case IDB_CHECKBOX:
        iCurrent = 11;
        break;
      case IDB_GROUP:
        iCurrent = 12;  
        break;
      case IDB_PUSH_BUTTON:
        iCurrent = 13;
        break;
      case IDB_POINTER:
        iCurrent = 14;  
        break;
      case IDB_STANDARD_HEIGHT:
        iCurrent = 15;
        break;
      case IDB_STANDARD_WIDTH:
        iCurrent = 16;  
        break;
      case IDB_STATIC:
        iCurrent = 17;  
        break;
      case IDB_HELP:
        iCurrent = 18;  
        break;
    }
    // handle button pressed down select state -- button down bitmap
    if (lpdis->itemState & ODS_SELECTED)
    {
        DrawBitmap(lpdis->hDC,lpdis->rcItem.left,
            lpdis->rcItem.top, hBitmaps[iCurrent][1],SRCCOPY,
            lpdis->rcItem.right,lpdis->rcItem.bottom);

    }
    else // not selected -- button up; text is in normal position
    {
        DrawBitmap(lpdis->hDC,lpdis->rcItem.left,
            lpdis->rcItem.top, hBitmaps[iCurrent][0],SRCCOPY,
            lpdis->rcItem.right,lpdis->rcItem.bottom);
    }
    return;
}

//****************************************************************// 
BOOL FAR PASCAL  ToolsMsgProc(HWND hDlg, WORD message, WPARAM wParam, LPARAM lParam)
{ 
 int BRtn;
 RECT R;
   TEditInfo *Edit = 0;
   HWND  Dialog = 0;
   static int times, WhichOnes[125];
   
   if(message == WM_INITDIALOG)
   {    times = 0;
        hBitmaps[1][0] = LoadBitmap(hInst,"COMBO_UP"); // regular state
        hBitmaps[1][1] = LoadBitmap(hInst,"COMBO_DOWN");   // pushed state
        hBitmaps[2][0] = LoadBitmap(hInst,"LIST_UP"); // regular state
        hBitmaps[2][1] = LoadBitmap(hInst,"LIST_DOWN");   // pushed state
        hBitmaps[3][0] = LoadBitmap(hInst,"HSPACE_UP"); // regular state
        hBitmaps[3][1] = LoadBitmap(hInst,"HSPACE_DOWN");   // pushed state
        hBitmaps[4][0] = LoadBitmap(hInst,"VSPACE_UP"); // regular state
        hBitmaps[4][1] = LoadBitmap(hInst,"VSPACE_DOWN");   // pushed state
        hBitmaps[5][0] = LoadBitmap(hInst,"ALIGN_TOP"); // regular state
        hBitmaps[5][1] = LoadBitmap(hInst,"ALIGN_TOP_DOWN");   // pushed state
        hBitmaps[6][0] = LoadBitmap(hInst,"ALIGN_BOTTOM"); // regular state
        hBitmaps[6][1] = LoadBitmap(hInst,"ALIGN_BOTTOM_DOWN");   // pushed state
        hBitmaps[7][0] = LoadBitmap(hInst,"ALIGN_LEFT"); // regular state
        hBitmaps[7][1] = LoadBitmap(hInst,"ALIGN_LEFT_DOWN");   // pushed state
        hBitmaps[8][0] = LoadBitmap(hInst,"ALIGN_RIGHT"); // regular state
        hBitmaps[8][1] = LoadBitmap(hInst,"ALIGN_RIGHT_DOWN");   // pushed state
        hBitmaps[9][0] = LoadBitmap(hInst,"RADIO_UP"); // regular state
        hBitmaps[9][1] = LoadBitmap(hInst,"RADIO_DOWN");   // pushed state
        hBitmaps[10][0] = LoadBitmap(hInst,"EDIT_UP"); // regular state
        hBitmaps[10][1] = LoadBitmap(hInst,"EDIT_DOWN");   // pushed state
        hBitmaps[11][0] = LoadBitmap(hInst,"CHECKBOX"); // regular state
        hBitmaps[11][1] = LoadBitmap(hInst,"CHECKBOX_DOWN");   // pushed state
        hBitmaps[12][0] = LoadBitmap(hInst,"GROUP_UP"); // regular state
        hBitmaps[12][1] = LoadBitmap(hInst,"GROUP_DOWN");   // pushed state
        hBitmaps[13][0] = LoadBitmap(hInst,"PUSH_BUTTON_UP"); // regular state
        hBitmaps[13][1] = LoadBitmap(hInst,"PUSH_BUTTON_DOWN");   // pushed state
        hBitmaps[14][0] = LoadBitmap(hInst,"POINTER_UP"); // regular state
        hBitmaps[14][1] = LoadBitmap(hInst,"POINTER_DOWN");   // pushed state
        hBitmaps[15][0] = LoadBitmap(hInst,"STANDARD_HEIGHT"); // regular state
        hBitmaps[15][1] = LoadBitmap(hInst,"STANDARD_HEIGHT_DOWN");   // pushed state
        hBitmaps[16][0] = LoadBitmap(hInst,"STANDARD_WIDTH"); // regular state
        hBitmaps[16][1] = LoadBitmap(hInst,"STANDARD_WIDTH_DOWN");   // pushed state
        hBitmaps[17][0] = LoadBitmap(hInst,"STATIC_UP"); // regular state
        hBitmaps[17][1] = LoadBitmap(hInst,"STATIC_DOWN");   // pushed state
        hBitmaps[18][0] = LoadBitmap(hInst,"HELP_UP"); // regular state
        hBitmaps[18][1] = LoadBitmap(hInst,"HELP_DOWN");   // pushed state
        GetObject(hBitmaps[1][1], sizeof(BITMAP), (LPSTR) &bm);
        pt.x = bm.bmWidth;
        pt.y = bm.bmHeight;
        // define size of dashed rectangles around text in both bitmaps
        hDC           = GetDC(hDlg);
        GetTextExtentPoint32(hDC, "Hi!", lstrlen("Hi!"),&txSize);
        ReleaseDC(hDlg, hDC);
        nSpaceWidth   = GetSystemMetrics(SM_CXBORDER) << 1;
        nSpaceHeight  = GetSystemMetrics(SM_CYBORDER) << 1;
        rectHi.top    = -nSpaceHeight + ((pt.y - txSize.cy) >> 1);
        rectHi.bottom = (nSpaceHeight << 1) + rectHi.top + txSize.cy;
        rectHi.left   = -nSpaceWidth + ((pt.x - txSize.cx) >> 1);
        rectHi.right  = (nSpaceWidth << 1) + rectHi.left + txSize.cx;
        rectHiDown.top    = 2 + rectHi.top;
        rectHiDown.bottom = 2 + rectHi.bottom;
        rectHiDown.left   = 2 + rectHi.left;
        rectHiDown.right  = 2 + rectHi.right; 
     return TRUE;
   }
   if ((BRtn = DIALOGSTYLEDynDialogMsgProc (hDlg,message, wParam, lParam))) return (BRtn);

  switch (message)
  {
    /* draw owner-draw button in all its states: normal, selected, focus */
    case WM_DRAWITEM:
        lpdis = (LPDRAWITEMSTRUCT)lParam;    
        switch (lpdis->itemAction)
        {  
            case ODA_SELECT:
            case ODA_DRAWENTIRE:
                  HandleSelectedState(lpdis);
            case ODA_FOCUS:
             //    HandleFocusState(lpdis);
                  WhichOnes[times++] = lpdis->CtlID;
                  return TRUE;
        }  //itemAction
        break;

    case WM_COMMAND:                      /* message: received a command */
        switch(wParam)
        {
          case IDB_COMBO:         // owner-draw button selected
          case IDB_LIST:         // owner-draw button selected
          case IDB_HSPACE:         // owner-draw button selected
          case IDB_VSPACE:         // owner-draw button selected
          case IDB_ALIGN_TOP:         // owner-draw button selected
          case IDB_ALIGN_BOTTOM:         // owner-draw button selected
          case IDB_ALIGN_LEFT:         // owner-draw button selected
          case IDB_ALIGN_RIGHT:         // owner-draw button selected
          case IDB_RADIO:         // owner-draw button selected
          case IDB_EDIT:         // owner-draw button selected
          case IDB_CHECKBOX:         // owner-draw button selected
          case IDB_GROUP:         // owner-draw button selected
          case IDB_PUSH_BUTTON:         // owner-draw button selected
          case IDB_POINTER:         // owner-draw button selected
          case IDB_STANDARD_HEIGHT:         // owner-draw button selected
          case IDB_STANDARD_WIDTH:         // owner-draw button selected
          case IDB_STATIC:         // owner-draw button selected
          case IDC_DIALOG_PROPERTIES:   
          case IDB_HELP:
              if (HIWORD(lParam) == BN_CLICKED)
                  SendMessage(GlobEdit->hModifyProc,WM_COMMAND,wParam,0L);
                return TRUE;
          case IDOK:
          case IDCANCEL:
               DestroyWindow(hDlg);
               /* Exits the dialog box        */
                return TRUE;
        }    
        break;
       case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_DESTROY:
         for(i = 1;i <= 18;i++)
         {
          DeleteObject(hBitmaps[i][0]);
          DeleteObject(hBitmaps[i][1]);
         } 
        break;
    }
    return FALSE;//DefWindowProc(hDlg,message,wParam,lParam); 

    return FALSE;                       /* Didn't process a message    */
}

//****************************************************************// 
void DrawBitmap(hDC, xStart, yStart, hBitmap, rop,right,bottom)

HDC     hDC;
short   xStart, yStart,right,bottom;
HBITMAP hBitmap;
DWORD   rop;
{
    HDC           hMemDC;
    RECT rc;
    BOOL Result;
    hMemDC = CreateCompatibleDC(hDC);
    SelectObject(hMemDC, hBitmap);
   // BitBlt(hDC, xStart, yStart, pt.x, pt.y, hMemDC, 0, 0, rop);


    Result = StretchBlt(hDC, xStart, yStart, right, bottom, hMemDC, 
           0, 0, pt.x, pt.y, rop);

    DeleteDC(hMemDC);
}
   void  RectToString(RECT Rect,char far *cptr)
{
  char far *ptr;
  ptr = cptr;
                   itoa(Rect.left,ptr,10);
                   ptr = _fstrchr(ptr,'\0');
                   *ptr++ = ' ';
                   itoa(Rect.top,ptr,10);
                   ptr = _fstrchr(ptr,'\0');
                   *ptr++ = ' ';
                   itoa(Rect.right,ptr,10);
                   ptr = _fstrchr(ptr,'\0');
                   *ptr++ = ' ';
                   itoa(Rect.bottom,ptr,10);
                   ptr = _fstrchr(ptr,'\0');
                   *ptr++ = ' ';
                   _fstrcpy(ptr," 2   2");
return;
}
//****************************************************************// 
void SaveChanges(HWND hDlg)
{ //user wants to save the changes made during a dialog edit
  //session.
           int Type, epyT, i,j,WitchOne, Items,Used, high, wide;
           char string[128], *ptr;
           RECT Rect;
           POINT Start, End;
           HWND  Rover,FirstRover;  
           
           SetWindowPos (hDlg,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
           if (MessageBox(0,"Save Dialog Changes?","Dialog Editor",MB_YESNO) == IDNO)
           	return;

           ExpandText (OutName); 
           OutFid = GSSiOpenFile (OutName,0,OF_CREATE); 
           if (OutFid == HFILE_ERROR)
           {
               MessageBox(0,"Unable to Open It","Dynamic Dialog",MB_ICONSTOP);
               return;
           }
           //first I write out the record defining the main window
           _fstrcpy(string,"DIALOG ");
           ptr = _fstrchr(string,'\0');
           GetWindowText(hDlg,ptr,80);
           ptr = _fstrchr(ptr,' ');
           while(ptr) 
           {
              *ptr = '|'; 
               ptr = _fstrchr(ptr,' ');
           }
           ptr = _fstrchr(string,'\0');
           *ptr++ = ' '; 
           sprintf (ptr,"%i %i %i ",BorderOption,CloseOnSave,SaveOnClose );
           ptr = _fstrchr(string,'\0');
           Rover = GetWindow(hDlg,GW_CHILD);
           Items = 0;
           while(Rover)
           {
               Items++; //count how many dialog items on the screen
               GetWindowRect(Rover, &Rect);
               if(Rect.right-Rect.left  < 8)Items--;
               Rover = GetWindow(Rover,GW_HWNDNEXT);
           }
           Used = 0;
           *ptr++ = ' '; 
           GetWindowRect(hDlg, &Rect);
           //Rect.top -= GETMETRIC(SM_CYCAPTION);
           if (BorderOption == 1)
           	Rect.bottom -= GETMETRIC(SM_CYCAPTION);
           Rect.right = (Rect.right - Rect.left)/2;
           Rect.bottom = (Rect.bottom - Rect.top)/2;
           Rect.left = Rect.left/2;
           RectToString(Rect,ptr); //want to include #items
           ptr = _fstrrchr(string,'2');
           itoa(Items+TOTALEXTRA,ptr,10);
           ptr = _fstrchr(ptr,'\0');
           _fstrcat(ptr++," '");
           ptr = _fstrchr(ptr,'\0');
           _fstrcpy(ptr,ThemeDB);
           ptr = _fstrchr(ptr,'\0');
           _fstrcat(ptr++,"'");
           fputstring(string,OutFid);

//next I work with each of the child windows
           GlobalUnlock(hGlobal);
           Rover = GetWindow(hDlg,GW_CHILD);
           while(Rover)
           {  
             epyT = GetDlgCtrlID(Rover);
             Type = (epyT>>8);
             switch(Type)
             {   
                 case TYPE_CHECKBOX: 
                 case TYPE_ENTERBUTTON:
                 case TYPE_LTEXT:
                   GetWindowRect(Rover, &Rect);
                   if(Rect.right-Rect.left  < 8)break;
                   wide = (Rect.right - Rect.left)/2;
                   high = (Rect.bottom - Rect.top)/2;
                   ScreenToClient(hDlg,(POINT *)&Rect); //resets left & top
                   Rect.right = wide;
                   Rect.bottom = high;
                   Rect.left = (Rect.left/2);
                   Rect.top  = (Rect.top/2);
                   _fstrcpy(string,TypeNames[Type]);
                   _fstrcat(string," ");
                   ptr = _fstrchr(string,'\0');
                   GetDlgItemText(hDlg,epyT,ptr,128);
                   ptr = _fstrchr(ptr,' ');
                   while(ptr) 
                   {
                     *ptr = '|'; 
                     ptr = _fstrchr(ptr,' ');
                   } 
                   ptr = _fstrchr(string,'\0');
                   *ptr++ = ' ';
                   RectToString(Rect,ptr);
                   fputstring(string,OutFid);
                 break;
                 case TYPE_LISTBOX:
                 {
                   GetWindowRect(Rover, &Rect);
                   if(Rect.right-Rect.left  < 8)break;
                   wide = (Rect.right - Rect.left)/2;
                   high = (Rect.bottom - Rect.top)/2;
                   ScreenToClient(hDlg,(POINT *)&Rect); //resets left & top
                   Rect.right = wide;
                   Rect.bottom = high;
                   Rect.left = (Rect.left/2);
                   Rect.top  = (Rect.top/2);

                    _fstrcpy(string,TypeNames[Type]); 
                   _fstrcat(string," AddedListBox ");
                   ptr = _fstrchr(string,'\0');
                   *ptr++ = ' ';
                   i = SendDlgItemMessage(hDlg,epyT,LB_GETCOUNT,0,0);
                   RectToString(Rect,ptr);
                   //I want to put the number of items in the 
                   //length field e.g., replaces this->2  2\r\n
                   ptr = _fstrchr(ptr,'\r');
                   ptr -= 5; //places me on the second to last 2
                   itoa(i,ptr,10);
                   ptr = _fstrchr(string,'\0');
                   _fstrcpy(ptr,"  2");
                   fputstring(string,OutFid);
                   for(j=0;j<i;j++)
                   {
                      SendDlgItemMessage(hDlg,epyT,LB_GETTEXT,j,(LPARAM)string);
                      fputstring(string,OutFid);
                   }
                 }
                 break;
                 case TYPE_COMBOBOX:
                 {
                   GetWindowRect(Rover, &Rect);
                   if(Rect.right-Rect.left  < 8)break;
                   wide = (Rect.right - Rect.left)/2;
                   high = (Rect.bottom - Rect.top)/2;
                   ScreenToClient(hDlg,(POINT *)&Rect); //resets left & top
                   Rect.right = wide;
                   Rect.bottom = high;
                   Rect.left = (Rect.left/2);
                   Rect.top  = (Rect.top/2);
                   i = SendDlgItemMessage(hDlg,epyT,CB_GETCOUNT,0,0);
                   Rect.bottom = (i+1) * 12;
                   _fstrcpy(string,TypeNames[Type]);
                   ptr = _fstrchr(string,'\0');
                   _fstrcat(ptr," AddedComboBox ");
                   ptr = _fstrchr(string,'\0');
                   RectToString(Rect,ptr);
                   //I want to put the number of items in the 
                   //length field e.g., replaces this->2  2\r\n
                   ptr = _fstrchr(ptr,'\r');
                   ptr -= 5; //places me on the second to last 2
                   itoa(i,ptr,10);
                   ptr = _fstrchr(ptr,'\0');
                   _fstrcpy(ptr,"  2");
                   fputstring(string,OutFid);
                   for(j=0;j<i;j++)
                   {
                      SendDlgItemMessage(hDlg,epyT,CB_GETLBTEXT,j,(LPARAM)string);
                      fputstring(string,OutFid);
                   }
                 }
                 break;
              //   case TYPE_NUMERIC:
                 case TYPE_EDIT:
                 { BOOL GottaSQL = FALSE;
                   char *ptr, OutString[128];
                   GetWindowRect(Rover, &Rect);
                   if(Rect.right-Rect.left  < 8)break;
                   lpF = (lpFldInfo) GlobalLock(hGlobal);
                   wide = (Rect.right - Rect.left)/2;
                   high = (Rect.bottom - Rect.top)/2;
                   ScreenToClient(hDlg,(POINT *)&Rect); //resets left & top
                   Rect.right = wide;
                   Rect.bottom = high;
                   Rect.left = (Rect.left/2);
                   Rect.top  = (Rect.top/2);
                   _fstrcpy(OutString,TypeNames[Type]);
                   ptr = _fstrchr(OutString,'\0');
                   *ptr++ = ' ';
                   *ptr = '\0';
                   lpF = lpItemInfo;
                   while(lpF->FI.name[0])
                   {
                     if(lpF->FldID == epyT)
                     {
                        if(lpF->FldType == 2)
                       {
                         _fstrcpy(ptr,"SQL");
                       }
                       else if(lpF->FldType == 1)
                       {
                         _fstrcpy(ptr,"CALC");
                       }
                       else
                       {  
                         if(lpF->FI.name[0] != '[')
                         {
                           *ptr = '[';
                           ptr++;
                         }
                         _fstrcpy(ptr, lpF->FI.name);  
                         ReplaceChar (ptr,' ','|');
                         ptr = _fstrchr(OutString,']');
                         if(!ptr)
                         {
                           ptr = _fstrchr(OutString,'\0');
                           *ptr = ']';
                           ptr++;
                           *ptr = '\0';
                         }  
                       }
                       ptr = _fstrchr(OutString,'\0');
                       *ptr++ = ' ';
                       *ptr = '\0';

                       break;
                     }
                     lpF++;
                   }
                         

                   RectToString(Rect,ptr);
                   fputstring(OutString,OutFid);
                   fputstring(lpF->FldContents,OutFid);
                   fputstring(lpF->MinVal,OutFid);
                   fputstring(lpF->MaxVal,OutFid);
                   fputstring(lpF->DefVal,OutFid);
                   GlobalUnlock(hGlobal);
                   break;
                 }
             }//end of the switch
             Rover = GetWindow(Rover,GW_HWNDNEXT); 
           }//end of for loop
           lpF = (lpFldInfo) GlobalLock(hGlobal);
  //next comes the optional, extra fields
            for(i = 0;i<NUMEDIT;i++)
            {  fputstring("EDIT [Field_Name] 299 0 2 2 0 0",OutFid); }
            for(i = 0;i<NUMCHECKBOX;i++)
            {fputstring("CHECKBOX [CheckBox]  299 2 2 2 0 0",OutFid);}
            for(i = 0;i<NUMSTATIC;i++)
            {fputstring("LTEXT [Static]  299 4 2 2 0 0",OutFid);}
            for(i = 0;i<NUMENTERBUTTON;i++)
            {fputstring("ENTERBUTTON [PushButton]  299 6 2 2 0 0",OutFid);}
        //    for(i = 0;i<NUMNUMERIC;i++)
        //    {fputstring("NUMERIC [Numeric]  299 8 2 2 0 0",OutFid);}
            for(i = 0;i<NUMLISTBOX;i++)
            {fputstring("LISTBOX [Listbox]  299 10 2 2 0 0",OutFid);}   
            for(i = 0;i<NUMCOMBOBOX;i++)
            {fputstring("COMBOBOX [ComboBox]  299 12 2 2 0 0",OutFid);}
            for(i = 0;i<NUMRADIO;i++)
            {fputstring("RADIOBUTTON [RadioButton]  299 30 2 2 0 0",OutFid);}


           GSSiClose2 (&OutFid);
return; 
}
//****************************************************************// 

BOOL   EditDynamicDialog (HWND hWndDlg,LPSTR Name, LPSTR SQL, LPSTR InsertString)
{    char buffer[260],*ptr, str1[80],str2[80], *ptr2, *ptr3,str[512];
     HANDLE hSQL;
     LPSTR lps;  
     BOOL	First=TRUE;
     int irc,length,k; 
	 HCURSOR	hcurSave; 
	 short	FileType;
            	
	 hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
     
     if (!Name)
     {
		 if (!GetFileName3(hWndDlg,OutName,IDS_DYNAMIC_DIALOG,IDS_FILEDYNDIA))  
		 	return FALSE; 
     }
     else
     	_fstrcpy (OutName,Name);
     ExpandText (OutName);   
     OutFid = GSSiOpenFile (OutName,0,OF_READ);
     if (OutFid == HFILE_ERROR)
	 {  
		char	mess[256];
	        	
		sprintf (mess,"Unable to open %s",OutName);
		MessageBox(0,mess,"Dynamic Dialog",MB_ICONSTOP);
		return FALSE;
	 } 
       fgetstring(buffer,195,OutFid);     
       ptr2 = _fstrrchr(buffer,39);//find single quote
       *ptr2 = '\0';
       ptr = _fstrrchr(buffer,39);
       *ptr = '\0';
       ptr++;
       _fstrcpy(ThemeDB,ptr);//the database used by this file
     //  ptr = _fstrrchr(buffer,39);//find second to last quote
    //   ptr++;
     //  _fstrcpy(ThemeDB,ptr);//the database used by this file
       ptr = _fstrrchr(buffer,' ');//isolate an unused number
       while (*ptr == ' ')
           {ptr--;}
       ptr++;
       *ptr = '\0';
       ptr = _fstrrchr(buffer,' ');//isolate number of items in file
       ptr++;
       GlobNumItems = atoi(ptr);
       hGlobal = GSSiGlobAlloc ( 528, GHND, sizeof(FldInfo)* GlobNumItems + TOTALEXTRA + 45);
       lpItemInfo = (lpFldInfo) GlobalLock(hGlobal);
       lpF = lpItemInfo;
       if (!(FileType = OpenDataFile (ThemeDB ,SQL,BT_READ,&hThemeDB))) return FALSE;
       GlobhSQL = hThemeDB;
       SQLPtr = (LPOPENSQLDATA) GlobalLock (hThemeDB);
       FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
  for(j=0;j<GlobNumItems;j++)
  {
Again:if(0==fgetstring(buffer,200,OutFid)) break;
      if(2 != sscanf(buffer,"%80s%80s",str1, str2))
      {
        MessageBox(0,buffer,"Unable to read this file.",
        MB_ICONSTOP);
        GSSiClose2 (&OutFid);
        return FALSE;
      }
      if(_fstrcmp(str1,"COMBOBOX") == 0 ||
         _fstrcmp(str1,"LISTBOX") == 0)
      {
        ptr = _fstrrchr(buffer,' '); //find last space
        while(*ptr == ' '){ ptr--;}
        ptr++; *ptr = '\0';
        ptr = _fstrrchr(buffer,' ');
        ptr++;
        i = atoi(ptr);
        for(k = 0;k<i;k++) {fgetstring(buffer,256,OutFid);}
        goto Again;
      }   
      if( _fstrcmp(str1,"EDIT") != 0)
         goto Again;  
      if(_fstrcmp(str2,"SQL") == 0)
      {
        lpF->FldType = 2;
        lpF->TempFldLen = 256;
        lpF->hCurrValue = GSSiGlobAlloc ( 529,GHND,lpF->TempFldLen);
        lpS = (LPSTR)GlobalLock(lpF->hCurrValue);
        _fstrcpy(lpS,lpF->FldContents);
        GlobalUnlock(lpF->hCurrValue);
        goto KeepGoing;
      }
      if(_fstrcmp(str2,"CALC") == 0)
      {
        lpF->FldType = 1;
        lpF->TempFldLen = 256;
        lpF->hCurrValue = GSSiGlobAlloc ( 530,GHND,lpF->TempFldLen);
        lpS = (LPSTR)GlobalLock(lpF->hCurrValue);
        _fstrcpy(lpS,lpF->FldContents);
        GlobalUnlock(lpF->hCurrValue);
        goto KeepGoing;
      }
      if(_fstrcmp(str2,"[Field_Name]") == 0)
          goto Again;
      
      ptr = _fstrchr(str2,']');
      *ptr = '\0';
      ptr = _fstrchr(str2,'[');
      ptr++;  
      ReplaceChar (ptr,'|',' ');
TryAgain:
      lpFieldInfo = &FilePtr->FldInfo; 
      for (i=0;i<FilePtr->NumFields;i++,lpFieldInfo++) 
      {
         if(_fstricmp(lpFieldInfo->name,ptr) == 0)
         {//found a match
           _fmemcpy(&lpF->FI,lpFieldInfo,sizeof(FIELDINFO));  
           HaltReport = FALSE;
           sprintf (str,"[%s]",lpFieldInfo->name);
		   ExpandTextDataNotFound=FALSE;
           ExpandText (str); 
            if (HaltReport)
            	goto Exit;
            if (ExpandTextDataNotFound)
            {   
            	if (!First)
            		goto Exit;
            	First = FALSE;
            	if (!InsertString)
            		goto Exit;   
            	if (!*InsertString)
            		goto Exit;
            	if (!InsertExternalFieldData (FilePtr,InsertString))
            		goto Exit;
            	goto TryAgain;
            } 
            else if (First && InsertString && *InsertString)
            {   
            	LPSTR	lpEnd;
            	char	SaveChr=0;
            	
            	First = FALSE;
            	if (*InsertString == '(')
            	{
            		InsertString++;
            		lpEnd = _fstrchr (InsertString,0);
            		lpEnd--;
            		SaveChr = *lpEnd;
            		*lpEnd=0;
            	}
				UpdateExternalFieldData (FilePtr,SQLPtr->SQL,InsertString);
				if (SaveChr)
					*lpEnd = SaveChr;
            	goto TryAgain;
            }  
            length = _fstrlen (str);
			lpF->hCurrValue = GSSiGlobAlloc ( 531,GHND,length+1);
			lpS = (LPSTR)GlobalLock(lpF->hCurrValue); 
			if (length)
				_fstrcpy(lpS,str);
			GlobalUnlock(lpF->hCurrValue);

KeepGoing: fgetstring(buffer,256,OutFid);//contents
           _fstrcpy(lpF->FldContents,buffer);
           fgetstring(buffer,32,OutFid);
           _fstrcpy(lpF->MinVal,buffer);
           fgetstring(buffer,32,OutFid);
           _fstrcpy(lpF->MaxVal,buffer);
           fgetstring(buffer,32,OutFid);
           _fstrcpy(lpF->DefVal,buffer);
           lpF++;
           break;
         }
      }
   }
Exit:
   GlobalUnlock(SQLPtr->OFHandle);
   GSSiClose2 (&OutFid);
   lpF = lpItemInfo; 
   if (!HaltReport)
   		OpenModelessDialog(OutName);
   GSSiSetCursor(hcurSave); 


return TRUE;
}
BOOL FAR PASCAL EDITDYNDIALOGMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{
 int	BRtn; 
 if ((BRtn = DIALOGSTYLEDynDialogMsgProc (hWndDlg,Message, wParam, lParam)))
 	return (BRtn); 
 if (Message != WM_DESTROY)
 if (DATAFILEMsgProc(hWndDlg,Message,wParam,lParam, 0,
                     SV_SET_FILE, SV_DATABASE_LIST, SV_TABLE_NAMES,SV_TABLE_HEADING, 0,0,
                     DDDataFile, &DDDataFileType, &hDDSQL,0,TRUE))  return TRUE;  
   switch (Message)
      {
   case WM_INITDIALOG:  
   		SetDlgItemText (hWndDlg,IDC_SQL,DDSQL);
		SetDlgItemText (hWndDlg,IDC_AUTOVALS,DynAutoVals);
		SetDlgItemText (hWndDlg,IDC_OPENCOMMAND,DynOpenCommand);
		SetDlgItemText (hWndDlg,IDC_CLOSECOMMAND,DynCloseCommand); 
		SetDlgItemText (hWndDlg,IDC_DIALOG_NAME,DynName);
        SendDlgItemMessage(hWndDlg, IDC_CLOSE_ON_SAVE,BM_SETCHECK, CloseOnSave, 0); 
        switch (SaveOnClose)
        {
        	case 0:
	        	SendDlgItemMessage(hWndDlg, IDC_SAVEONCLOSE_NO,BM_SETCHECK, TRUE, 0);  
	        break;
        	case 1:
	        	SendDlgItemMessage(hWndDlg, IDC_SAVEONCLOSE_YES,BM_SETCHECK, TRUE, 0);  
	        break;
        	case 2:
	        	SendDlgItemMessage(hWndDlg, IDC_SAVEONCLOSE_PROMPT,BM_SETCHECK, TRUE, 0);  
	        break;
        }
        switch (BorderOption)
        {
        	case 1:
	        	SendDlgItemMessage(hWndDlg, IDC_BORDER_STANDARD,BM_SETCHECK, TRUE, 0);  
	        break;
        	case 2:
	        	SendDlgItemMessage(hWndDlg, IDC_BORDER_NOTITLE,BM_SETCHECK, TRUE, 0);  
	        break;
        	case 3:
	        	SendDlgItemMessage(hWndDlg, IDC_BORDER_NONE,BM_SETCHECK, TRUE, 0);  
	        break;
        }
   		break;
   case WM_DESTROY:  
        GetDlgItemText (hWndDlg,IDC_SQL,DDSQL,1024); 
       	EnableWindow (hWndMain,TRUE);
        break; 
   case WM_COMMAND:
      switch (wParam)
         { 
			case IDCANCEL: 
			case IDOK: 
				 GetDlgItemText (hWndDlg,IDC_AUTOVALS,DynAutoVals,sizeof(DynAutoVals));
				 GetDlgItemText (hWndDlg,IDC_OPENCOMMAND,DynOpenCommand,sizeof(DynAutoVals));
				 GetDlgItemText (hWndDlg,IDC_CLOSECOMMAND,DynCloseCommand,sizeof(DynAutoVals));
				 GetDlgItemText (hWndDlg,SV_DATABASE_LIST,DDDataFile,sizeof(DDDataFile));
				 GetDlgItemText (hWndDlg,IDC_DIALOG_NAME,DynName,sizeof(DynName));
		         CloseOnSave = SendDlgItemMessage(hWndDlg, IDC_CLOSE_ON_SAVE,BM_GETCHECK,0 , 0); 
			     if (SendDlgItemMessage(hWndDlg, IDC_SAVEONCLOSE_NO,BM_GETCHECK, 0, 0))
			     	SaveOnClose = 0;  
			     if (SendDlgItemMessage(hWndDlg, IDC_SAVEONCLOSE_YES,BM_GETCHECK, 0, 0))  
			     	SaveOnClose = 1;  
			     if (SendDlgItemMessage(hWndDlg, IDC_SAVEONCLOSE_PROMPT,BM_SETCHECK, 0,0)) 
			     	SaveOnClose = 2;  
			     if (SendDlgItemMessage(hWndDlg, IDC_BORDER_STANDARD,BM_GETCHECK, 0, 0))
			        BorderOption = 1;
			     if (SendDlgItemMessage(hWndDlg, IDC_BORDER_NOTITLE,BM_GETCHECK, 0, 0))
			        BorderOption = 2;
			     if (SendDlgItemMessage(hWndDlg, IDC_BORDER_NONE,BM_GETCHECK, 0, 0))
			        BorderOption = 3;
				 EndDialog(hWndDlg, TRUE);
				 break;   
            case IDC_SHOW_FIELDS:
            	 DisplayFieldList (hWndMain,hDDSQL,0,0,0);
                 break;
                      
            case IDC_OPEN_DB:
                 EnableWindow (GetDlgItem(hWndDlg,IDC_SHOW_FIELDS),TRUE);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_SETSQL),TRUE);
                 break;
            case IDC_SETSQL:      
            {    
                 HANDLE hMem;
                 LPSTR  lpStr, lpWhere;
                 
                 hMem = GSSiGlobAlloc ( 335,GHND,4096);
                 lpStr = GlobalLock (hMem); 
                 GetDlgItemText (hWndDlg,IDC_SQL,lpStr,1024);
                 if (GetSQLWhereClause (hWndDlg, hDDSQL, lpStr))
                 	SetDlgItemText (hWndDlg,IDC_SQL,lpStr);    
                 GSSiGlobUlFree (&hMem);
                 break;
            }
            
                      
	       default:
	         return FALSE;
	     } 
      break;

   default:
      return FALSE;
      }
   return TRUE;
} 

short GetControlDefByID (short OldID,LPCONTROLDEF pControlDef,short NumControls)
{
	short	i;
	
	for (i=0;i<NumControls;i++)
		if (pControlDef[i].ControlID == OldID)
			return i;
	return NumControls;
}

short GetControlDefByhWnd (HWND hWnd,LPCONTROLDEF pControlDef,short NumControls)
{
	short	i;
	
	for (i=0;i<NumControls;i++)
		if (pControlDef[i].hWnd == hWnd)
			return i;
	return NumControls;
}

BOOL DynValueValid (HWND hWndDlg,short id)
{    
	LPCONTROLDEF	pControlDef;  
	LPSTR	pDynStrings;
	BOOL	Valid=TRUE; 
	BOOL	Err; 
	char	str[256];
			
	pControlDef = (LPCONTROLDEF)GlobalLock (hDynControls); 
	pDynStrings=GlobalLock (hDynStrings);
	GetDlgItemText (hWndDlg,pControlDef[id].ControlID,str,sizeof(str)-1); 
	if (!*str || !_fstricmp (str,"NULL"))
		goto Exit;
	SetGlobalValue ("%V",str);
	if (pDynStrings[pControlDef[id].ValidLoc])
		Valid = LogicP (&pDynStrings[pControlDef[id].ValidLoc],&Err);
	if (!Valid)
	{ 
		MessageBox (hWndDlg,&pDynStrings[pControlDef[id].ValidMsgLoc],0,MB_ICONEXCLAMATION);
	}
Exit:
	GlobalUnlock (hDynStrings);
	GlobalUnlock (hDynControls);
	return Valid;
}

BOOL DynValueChanged (HWND hWndDlg)
{
	LPCONTROLDEF pControlDef=(LPCONTROLDEF)GlobalLock (hDynControls);
	short	i, choice;
	char	str[256]; 
	LPSTR	pInitialStrings;  
	BOOL	rtn=FALSE;
	LPSTR	pDynStrings=GlobalLock (hDynStrings);;
    
    if (!hDynInitialStrings)
    	goto Exit;
    pInitialStrings=GlobalLock (hDynInitialStrings);
	for (i=0;i<NumDynControls;i++) 
	{   
		if (pDynStrings[pControlDef[i].DefLoc])
        switch (pControlDef[i].Type)
        {
        	default:
			GetDlgItemText (hWndDlg,pControlDef[i].ControlID,str,sizeof(str));
			if (_fstrcmp (&pInitialStrings[pControlDef[i].InitialLoc],str))
			{
				pControlDef[i].ValueChanged = TRUE;
			}
			break;
		} 
		if (pControlDef[i].ValueChanged)  
			rtn = TRUE; 
	}
	GlobalUnlock (hDynInitialStrings); 
Exit:  
	GlobalUnlock (hDynStrings); 
	GlobalUnlock (hDynControls);
	return rtn;
} 

BOOL SetDynDefaultValues (HWND hWndDlg)
{
	LPCONTROLDEF pControlDef=(LPCONTROLDEF)GlobalLock (hDynControls);
	short	i, choice;
	char	str[256]; 
	LPSTR	pDynStrings=GlobalLock (hDynStrings);
    
	for (i=0;i<NumDynControls;i++) 
	{   
   		pControlDef[i].ValueChanged = FALSE;
		_fstrcpy (str,&pDynStrings[pControlDef[i].DefaultLoc]);
        ExpandText (str);
        if (pControlDef[i].Type != 3) 
        {
           	if (*str)
        	{
        		pControlDef[i].ValueChanged = TRUE;
        	}
        }
        switch (pControlDef[i].Type)
        {   
        	case 2: 
        		choice = SendDlgItemMessage(hWndDlg,pControlDef[i].ControlID,CB_FINDSTRING,-1,(LPARAM)str);
        		if (choice != CB_ERR)
        			SendDlgItemMessage(hWndDlg,pControlDef[i].ControlID,CB_SETCURSEL,choice,0);	
        	break;
        	
        	default:
			SetDlgItemText (hWndDlg,pControlDef[i].ControlID,str);
			break;
		}   
	}
	GlobalUnlock (hDynStrings);    
	GlobalUnlock (hDynControls);
	return TRUE;
}

BOOL SetDynCurrentValues (HWND hWndDlg,LPSTR VarName,LPSTR Value)
{
	LPCONTROLDEF pControlDef=(LPCONTROLDEF)GlobalLock (hDynControls);
	short	i, choice;
	char	str[256], fieldname[128]; 
	LPSTR	pDynStrings, pSC;
    
    pDynStrings=GlobalLock (hDynStrings);
	for (i=0;i<NumDynControls;i++) 
	{
   		pControlDef[i].ValueChanged = FALSE;
		_fstrcpy (str,&pDynStrings[pControlDef[i].DefLoc]); 
		if (VarName)
		{   
			if (*str == '[' && *LastChr (str) == ']')
			{
				*LastChr (str) = 0;
				if (!_fstricmp (VarName,&str[1]))
				{ 
					SetDlgItemText (hWndDlg,pControlDef[i].ControlID,Value);
					break;
				}
			}
			continue;
		}
		if ((pSC = _fstrchr (str,';')))
			*pSC = 0;
		_fstrcpy (fieldname,str); 
        ExpandText (str); 
        if (!*str)
        {   if (pControlDef[i].DefaultLoc)
        	{
        		_fstrcpy (str,&pDynStrings[pControlDef[i].DefaultLoc]); 
				ExpandText (str);
        		pControlDef[i].ValueChanged = TRUE;
			}
        } 
        switch (pControlDef[i].Type)
        {
        	case 2: 
        		choice = SendDlgItemMessage(hWndDlg,pControlDef[i].ControlID,CB_FINDSTRING,-1,(LPARAM)str);
        		if (choice == CB_ERR && *str) 
        		{   
        			char	mess[256];
        			
        			sprintf (mess,"The value '%s' for field %s is not in the selection list \r\nDo you wish to add it?",str,fieldname);
        			if (MessageBox (hWndDlg,mess,"",MB_YESNO) == IDYES)
        				choice = SendDlgItemMessage(hWndDlg,pControlDef[i].ControlID,CB_ADDSTRING,0,(LPARAM)str);
        		}	
        		SendDlgItemMessage(hWndDlg,pControlDef[i].ControlID,CB_SETCURSEL,choice,0);
        	break;
        	
        	default:
			SetDlgItemText (hWndDlg,pControlDef[i].ControlID,str);
			break;
		}   
	}
	GlobalUnlock (hDynStrings);    
	GlobalUnlock (hDynControls);
	return TRUE;
}

BOOL ClearDynCurrentValues (HWND hWndDlg)
{
	LPCONTROLDEF pControlDef=(LPCONTROLDEF)GlobalLock (hDynControls);
	short	i, choice;
	char	str[256], fieldname[128]; 
	LPSTR	pDynStrings, pSC;
    
    pDynStrings=GlobalLock (hDynStrings);
	for (i=0;i<NumDynControls;i++) 
	{
   		pControlDef[i].ValueChanged = FALSE;
        switch (pControlDef[i].Type)
        {
        	case 2: 
        		choice = -1;
        		SendDlgItemMessage(hWndDlg,pControlDef[i].ControlID,CB_SETCURSEL,choice,0);
        	break;
        	
        	default:
			SetDlgItemText (hWndDlg,pControlDef[i].ControlID,"");
			break;
		}   
	}
	GlobalUnlock (hDynStrings);    
	GlobalUnlock (hDynControls);
	return TRUE;
}

BOOL GetDynInitialValues (HWND hWndDlg)
{
	LPCONTROLDEF pControlDef=(LPCONTROLDEF)GlobalLock (hDynControls);
	short	i, choice;
	char	str[256]; 
	LPSTR	pInitialStrings;
    
    GSSiGlobFree (&hDynInitialStrings);
    lInitialStrings = 1;
    hDynInitialStrings = GSSiGlobAlloc ( 331,GHND,USHRT_MAX);
    pInitialStrings=GlobalLock (hDynInitialStrings);
	for (i=0;i<NumDynControls;i++) 
	{
        switch (pControlDef[i].Type)
        {
        	default:
			GetDlgItemText (hWndDlg,pControlDef[i].ControlID,str,sizeof(str));
			pControlDef[i].InitialLoc = AddStringToList (str,pInitialStrings,&lInitialStrings);
			break;
		}   
	}
	GlobalUnlock (hDynInitialStrings);    
	GlobalUnlock (hDynControls);
	return TRUE;
} 

BOOL WriteDynDlgData (HWND hWndDlg)
{
	return TRUE;
}

BOOL SetDynDlgData (HWND hWndDlg,LPSTR Name,LPUINT pcntls,short lncntls)
{   
	LPCONTROLDEF pControlDef;     
	HFILE		Fid; 
	char		str[1026];   
	LPSTR		pDynStrings;
	short		i, j, id, OldNumControls;

	
	if (Name)
	{   
		GSSiGlobFree (&hDynControls);
		GSSiGlobFree (&hDynStrings);  
		_fstrcpy (DDFile,Name);
		NumDynControls = lncntls/2;
		hDynControls = GSSiGlobAlloc ( 332,GHND,(NumDynControls+1)*sizeof(CONTROLDEF));
		hDynStrings = GSSiGlobAlloc ( 333,GHND,USHRT_MAX);
		lDynStrings = 1;  
		pDynStrings = GlobalLock (hDynStrings);
		pControlDef = (LPCONTROLDEF)GlobalLock (hDynControls);
		for (i=0;i<NumDynControls;i++)
		{
			pControlDef[i].ControlID = pcntls[i]; 
			pControlDef[i].hWnd = GetDlgItem (hWndDlg,pcntls[i]); 
		    GetClassName (pControlDef[i].hWnd, str, 100);
		    if (!_fstrcmp (str, "Edit"))
				pControlDef[i].Type = 1; 
		    else if (!_fstrcmp (str, "ListBox"))
				pControlDef[i].Type = 2; 
		    else if (!_fstrcmp (str, "ComboBox"))
				pControlDef[i].Type = 2; 
		    else if (!_fstrcmp (str, "Static"))
				pControlDef[i].Type = 3; 
			else            
				pControlDef[i].Type = 0; 
		}
		DynWnd = hWndDlg; 
		Fid = GSSiOpenFile (DDFile,0,OF_READ);
		if (Fid != HFILE_ERROR)
		{   
			fgetstring (str,128,Fid); //version
			fgetstring (str,128,Fid); 
			SetWindowText (hWndDlg,str);
			fgetstring (DDDataFile,sizeof(DDDataFile)-2,Fid);
			fgetstring (DDSQL,sizeof(DDSQL)-2,Fid);  
			fgetstring (DynAutoVals,sizeof(DynAutoVals)-2,Fid);
			fgetstring (DynOpenCommand,sizeof(DynOpenCommand)-2,Fid);
			fgetstring (DynCloseCommand,sizeof(DynCloseCommand)-2,Fid);
			fgetstring (str,128,Fid); //oldnumcontrols 
			OldNumControls = atoi (str); 
			for (i=0;i<OldNumControls;i++)
			{   
				LPSTR pSpace;
				USHORT	OldID;
				short	NumVals;
				
				fgetstring (str,128,Fid);  
				pSpace = _fstrchr (str,' ');
				OldID = atol (str); 
				id = GetControlDefByID (OldID,pControlDef,NumDynControls);
				NumVals = atol (pSpace);
				fgetstring (str,1024,Fid); 
				pControlDef[id].DefLoc = AddStringToList (str,pDynStrings,&lDynStrings);
				fgetstring (str,1024,Fid); 
				pControlDef[id].DefaultLoc = AddStringToList (str,pDynStrings,&lDynStrings);
				fgetstring (str,1024,Fid); 
				pControlDef[id].ValidLoc = AddStringToList (str,pDynStrings,&lDynStrings);
				fgetstring (str,1024,Fid); 
				pControlDef[id].ValidMsgLoc = AddStringToList (str,pDynStrings,&lDynStrings);
				for (j=0;j<NumVals;j++)  
				{
					fgetstring (str,1024,Fid);
					if (pControlDef[id].ControlID && *str)
			        	SendDlgItemMessage (hWndDlg,pControlDef[id].ControlID,CB_ADDSTRING,0,(LPARAM)str);
				} 
			}
			GSSiClose2 (&Fid);
		} 
		DDFileUpdated = FALSE;
		GlobalUnlock (hDynControls); 
		GlobalUnlock (hDynStrings); 
        OpenDataFile (DDDataFile,DDSQL,BT_WRITE,&hDDSQL);
		DynRecordExists	= 0;     
		ProcessText (DynOpenCommand);
	}
	else if (hWndDlg == DynWnd)
	{
		ProcessText (DynCloseCommand);
        CloseDataFile (FALSE,&hDDSQL);      
    	DestroyFieldList ();
    	if (DDFileUpdated)
    	{
	    	Fid = GSSiOpenFile (DDFile,0,OF_CREATE); 
	    	if (Fid != HFILE_ERROR)
	    	{
		    	fputstring ("GMDIALOG:VERSION1",Fid);
		    	GetWindowText (hWndDlg,str,sizeof(str));
		    	fputstring (str,Fid);                     
		    	fputstring (DDDataFile,Fid);
		    	fputstring (DDSQL,Fid); 
		    	fputstring (DynAutoVals,Fid);
		    	fputstring (DynOpenCommand,Fid);
		    	fputstring (DynCloseCommand,Fid);
				pControlDef = (LPCONTROLDEF)GlobalLock (hDynControls);
				pDynStrings = GlobalLock (hDynStrings); 
				ltoa (NumDynControls,str,10);
				fputstring (str,Fid);  
				for (i=0;i<NumDynControls;i++)
				{   
					short	ival=0, NumVals;
					
					NumVals = SendDlgItemMessage(hWndDlg, pControlDef[i].ControlID, CB_GETCOUNT, 0, 0);
					sprintf (str,"%ld %i",(long)pControlDef[i].ControlID,(int)NumVals);
					fputstring (str,Fid);
					fputstring (&pDynStrings[pControlDef[i].DefLoc],Fid);
					fputstring (&pDynStrings[pControlDef[i].DefaultLoc],Fid);
					fputstring (&pDynStrings[pControlDef[i].ValidLoc],Fid);  
					fputstring (&pDynStrings[pControlDef[i].ValidMsgLoc],Fid);  
					for (ival = 0;ival<NumVals;ival++)
					{
						SendDlgItemMessage(hWndDlg, pControlDef[i].ControlID, CB_GETLBTEXT, ival,(LPARAM)str);
						fputstring (str,Fid);
					}
				}
		    	GSSiClose2 (&Fid);
		    	GlobalUnlock (hDynStrings);
		    	GlobalUnlock (hDynControls); 
		    }
	    } 
    	GSSiGlobFree (&hDynStrings);
		GSSiGlobFree (&hDynControls);
	    GSSiGlobFree (&hDynInitialStrings);
		DynWnd = 0;  
	}
	return TRUE;
} 

BOOL GetDynDlgHandle (HWND hWndDlg,LPHANDLE phDynDlgControls)
{   
	if (!hDynControls || hWndDlg != DynWnd)
		return FALSE; 
	*phDynDlgControls = hDynControls;
	return TRUE;
}

BOOL ShowDlgUpdateOptions (HWND hWndEdit,HWND hWndInput)
{
    HMENU hMenu=CreatePopupMenu(); 
    POINT	position; 
    char	str[1024];
	char	FileNames[2][MAX_PATH];
	LPSTR	pMacro, pEnd, pEq;
	int	n=0;
    
    GetClassName (hWndEdit, str, sizeof (str)-1);
    if (!_fstrcmp (str, "Edit"))
    {
		char FileName[MAX_PATH+1];

		GetWindowText (hWndEdit,FileName,1023);
		if (strlen (FileName) > 4 && !stricmp (&FileName[strlen(FileName)-4],".TXT"))
		{
			if ((pEq = strchr (FileName,'=')))
				pEq++;
			else
				pEq = FileName;
			if (ExistFile (pEq))
			{
				strcpy (FileNames[0],pEq);
				sprintf (str,"Edit file %s",pEq);
				AppendMenu (hMenu,MF_ENABLED|MF_STRING,0,str);
			}
		}
		else if ((pMacro = strstr (FileName,"$MACRO(")))
		{
			LPSTR	pEq;
			
			pMacro += 7;
			pEnd = strchr (pMacro,')');
			if (pEnd)
			{
				*pEnd = 0;
				pEnd = strchr (pMacro,',');
				if (pEnd)
					*pEnd = 0;
				if (ExistFile (pMacro))
				{
					strcpy (FileNames[1],pMacro);
					sprintf (str,"Edit macro %s",pMacro);
					AppendMenu (hMenu,MF_ENABLED|MF_STRING,1,str);
				}
			}
		}
	}
    else if (!_fstrcmp (str, "ListBox"))
    {
/*	    AppendMenu (hMenu,MF_ENABLED|MF_STRING,65001,IADDR("Add Entry",1));
	    AppendMenu (hMenu,MF_ENABLED|MF_STRING,65002,IADDR("Remove Entry",1));
	    AppendMenu (hMenu,MF_ENABLED|MF_STRING,65008,IADDR("Clear All Entries",1));
	    AppendMenu (hMenu,MF_ENABLED|MF_STRING,65007,IADDR("Enter All Unique Values from Database",1));
*/
	}
    else if (!_fstrcmp (str, "ComboBox"))
    {
/*	    AppendMenu (hMenu,MF_ENABLED|MF_STRING,65001,IADDR("Add Entry",1));
	    AppendMenu (hMenu,MF_ENABLED|MF_STRING,65002,IADDR("Remove Entry",1));
	    AppendMenu (hMenu,MF_ENABLED|MF_STRING,65008,IADDR("Clear All Entries",1));
	    AppendMenu (hMenu,MF_ENABLED|MF_STRING,65007,IADDR("Enter All Unique Values from Database",1));
*/	}
    else if (!_fstrcmp (str, "Static"))
	    ;//AppendMenu (hMenu,MF_ENABLED|MF_STRING,65006,IADDR("Change Heading",1));
	if (GetMenuItemCount (hMenu))
	{
		int iopt;
		AppendMenu (hMenu,MF_ENABLED|MF_STRING,65009,IADDR("Cancel",1));
		GetCursorPos (&position);
		iopt = TrackPopupMenu (hMenu,TPM_LEFTBUTTON|TPM_RETURNCMD,position.x,position.y,0,hWndInput,0);
		if (iopt < 2)
			GMEdit (hWndEdit,FileNames[iopt]);
	}
    DestroyMenu (hMenu);  
    return TRUE;
}
 
BOOL ShowDynDlgUpdateOptions (HWND hWndEdit,HWND hWndInput)
{
    HMENU hMenu=CreatePopupMenu(); 
    POINT	position; 
    char	str[100];
    
    GetClassName (hWndEdit, str, sizeof (str)-1);
    if (!_fstrcmp (str, "Edit"))
    {
	    AppendMenu (hMenu,MF_ENABLED|MF_STRING,65003,IADDR("Edit Validation Rule",1));
	}
    else if (!_fstrcmp (str, "ListBox"))
    {
	    AppendMenu (hMenu,MF_ENABLED|MF_STRING,65001,IADDR("Add Entry",1));
	    AppendMenu (hMenu,MF_ENABLED|MF_STRING,65002,IADDR("Remove Entry",1));
	    AppendMenu (hMenu,MF_ENABLED|MF_STRING,65008,IADDR("Clear All Entries",1));
	    AppendMenu (hMenu,MF_ENABLED|MF_STRING,65007,IADDR("Enter All Unique Values from Database",1));
	}
    else if (!_fstrcmp (str, "ComboBox"))
    {
	    AppendMenu (hMenu,MF_ENABLED|MF_STRING,65001,IADDR("Add Entry",1));
	    AppendMenu (hMenu,MF_ENABLED|MF_STRING,65002,IADDR("Remove Entry",1));
	    AppendMenu (hMenu,MF_ENABLED|MF_STRING,65008,IADDR("Clear All Entries",1));
	    AppendMenu (hMenu,MF_ENABLED|MF_STRING,65007,IADDR("Enter All Unique Values from Database",1));
	}
    else if (!_fstrcmp (str, "Static"))
	    AppendMenu (hMenu,MF_ENABLED|MF_STRING,65006,IADDR("Change Heading",1));
    AppendMenu (hMenu,MF_ENABLED|MF_STRING,65005,IADDR("Edit Definition",1));
    AppendMenu (hMenu,MF_ENABLED|MF_STRING,65004,IADDR("Change Default",1)); 
    AppendMenu (hMenu,MF_ENABLED|MF_STRING,65009,IADDR("Cancel",1));
    GetCursorPos (&position);
    TrackPopupMenu (hMenu,TPM_LEFTBUTTON,position.x,position.y,0,hWndInput,0);
    DestroyMenu (hMenu);  
    return TRUE;
} 

BOOL ProcessDynEdit (HWND hWndDlg,HWND hWndEdit,WPARAM wParam,LPARAM lParam) 
{   
	char	str[1024], ControlName[66], FieldName[66], FieldVal[256]; 
	short	id,choice,i,j; 
	BOOL	rtn = FALSE;
	LPCONTROLDEF	pControlDef;  
	LPSTR	pDynStrings, pSC;
	LPOPENSQLDATA	SQLPtr;
	LPFIELDINFO	lpFieldInfo;
	LPOPENFILEDATA	FilePtr; 
	
	if (!hWndDlg)
		hWndDlg = GetParent (hWndEdit);
	if (hWndDlg != DynWnd)
		return FALSE;
	pControlDef=(LPCONTROLDEF)GlobalLock (hDynControls); 
	if (HIWORD(lParam) == EN_KILLFOCUS)
		hWndEdit = GetDlgItem (hWndDlg,wParam); 
	id = GetControlDefByhWnd (hWndEdit,pControlDef,NumDynControls); 
	if (id < NumDynControls)
	{
		if (wParam == pControlDef[id].ControlID && HIWORD(lParam) == EN_KILLFOCUS)   
		{   
			if (!InDynClose)
			{
				GlobalUnlock (hDynControls);
				return DynValueValid (hWndDlg,id); 
			} 
		}
	}
	switch (wParam)
	{   
		default:
			GlobalUnlock (hDynControls);
			return FALSE;
		case IDC_DYNEDIT: 
		case IDC_GETRECORD:  
		case IDC_SETMULTIPLE:  
		case IDC_SETVAL:
		case IDC_EXIT:
		case 65001: 
		case 65002: 
		case 65003: 
		case 65004: 
		case 65005: 
		case 65006: 
		case 65007: 
		case 65008:   
		case 65009:
		break;
	}
	pDynStrings=GlobalLock (hDynStrings);
	switch (wParam)
	{   
		case IDC_SETVAL:    
			GetGlobalCVal ("[%ARG1]",ControlName,0);
			GetGlobalCVal ("[%ARG2]",str,0);
			SetDynCurrentValues (hWndDlg,ControlName,str);
			break;
		case IDC_SETMULTIPLE:
		case IDC_GETRECORD:  
			rtn = TRUE;   
		case IDC_EXIT: 
		{
			HANDLE	hCmd = GSSiGlobAlloc ( 334,GMEM_MOVEABLE,USHRT_MAX);
			LPSTR	pCmd = GlobalLock (hCmd);  
			LPSTR	pTableName;
			char	delim=' ';
				
			if (hDDSQL)
			{	
				SQLPtr = (LPOPENSQLDATA)GlobalLock (hDDSQL);
				FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
				lpFieldInfo = &FilePtr->FldInfo;    
				pTableName = _fstrrchr (FilePtr->fullpath,'|');
				if (!pTableName)
					pTableName = FilePtr->fullpath; 
				else
					pTableName++;
		   		pControlDef = (LPCONTROLDEF)GlobalLock (hDynControls); 
				if (DynValueChanged (hWndDlg))
				{ 
	   				for (i=0;i<NumDynControls;i++)
	   				{   
	   					if (pControlDef[i].ValueChanged)
	   					{
	   						if (!DynValueValid (hWndDlg,i)) 
	   						{
	   							rtn = TRUE;
	   							goto Exit; 
	   						} 
	   					}
	   				}
					if (DynRecordExists)
					{
						sprintf (pCmd,"UPDATE %s SET",pTableName); 
		   				for (i=0;i<NumDynControls;i++)
		   				{   
		   					if (pControlDef[i].ValueChanged)
		   					{   
		   						*FieldVal = 0;
		   						pControlDef[i].ValueChanged = 0;
		   						_fstrcpy (str,&pDynStrings[pControlDef[i].DefLoc]); 
		   						if ((pSC = _fstrchr (str,';')))
		   						{
		   							LPSTR pEQ = _fstrchr (pSC,'=');
		   							
		   							pSC++;
		   							if (pEQ)
		   							{
		   								*pEQ++ = 0;
		   								_fstrcpy (FieldVal,pEQ);
		   							}
		   							_fmemmove (str,pSC,_fstrlen(pSC)+1);
		   						}
		   						if (*str == '[' && *LastChr(str) == ']') 
		   						{
		   							_fstrcpy (FieldName,&str[1]);
		   							*LastChr (FieldName) = 0; 
		   						}
		   						else
		   							_fstrcpy (FieldName,str);
		   						if ((j=GetFieldIDFromName(SQLPtr->IDName,FilePtr,FieldName))>=0)
		   						{
	   								sprintf (_fstrchr (pCmd,0),"%c%s=",delim,lpFieldInfo[j].name);
	   								GetWindowText(pControlDef[i].hWnd,str,256); 
	   								if (*str && _fstricmp (str,"NULL"))
	   								{
                                        if (*FieldVal)
                                        {
	   										SetGlobalValue ("%V",str);
	   										_fstrcpy (str,FieldVal);
	   										ExpandText (str);
	   									}
							            switch (lpFieldInfo[j].type)
							            {
							               	case BT_INTEGER:
								 			case BT_REAL:
								 			case SQL_DOUBLE:
											case SQL_INTEGER:
											case SQL_TINYINT: 
											case SQL_GUID:
												_fstrcat (pCmd,str);
				   							break;
				   							case SQL_TIMESTAMP:
						                    	sprintf (_fstrchr (pCmd,0),"'%s'",str);
				   							break;
				   							default: 
				   								REPLAC (str,"'","''",254);
						                    	sprintf (_fstrchr (pCmd,0),"'%s'",str);
				   							break;
				   						} 
				   					}
			   						else
			   							_fstrcat (pCmd,"NULL");		
			                    	delim = ',';
			                    }
		                    }
		   				}
					    for (i=0;i<FilePtr->NumFields;i++) 
    					{
    						if (GetFieldValFromSetList (DynAutoVals,lpFieldInfo[i].name,str))
    						{   
								sprintf (_fstrchr(pCmd,0),"%c%s=",delim,lpFieldInfo[i].name);
					            if (*str)
					            switch (lpFieldInfo[i].type)
					            {
					               	case BT_INTEGER:
						 			case BT_REAL:
						 			case SQL_DOUBLE:
									case SQL_INTEGER: 
									case SQL_TINYINT:
	                                	_fstrcat (_fstrchr(pCmd,0),str);
	                                break;
						 			default:
						 				REPLAC (str,"'","''",254);
						 				sprintf (_fstrchr(pCmd,0),"'%s'",str);
					 				break;
					            }
					            else
					            	_fstrcat (pCmd,"NULL");
					         }
		   				}
						sprintf (_fstrchr (pCmd,0)," WHERE %s",CurrentDynWhere);
					}
					else if (AllowCreateNewRecord)
					{
						sprintf (pCmd,"INSERT INTO %s VALUES(",pTableName); 
					    for (i=0;i<FilePtr->NumFields;i++) 
    					{
    						if (!GetFieldValFromSetList (DynAutoVals,lpFieldInfo[i].name,str))
    						{   
    							*str = 0;
				   				for (j=0;j<NumDynControls;j++)
				   				{   
			   						_fstrcpy (ControlName,&pDynStrings[pControlDef[j].DefLoc+1]);
			   						*LastChr (ControlName) = 0;
			   						if (!_fstricmp (ControlName,lpFieldInfo[i].name))
			   						{
				                    	GetWindowText(pControlDef[j].hWnd,str,256); 
				                    	break;
				                    }
				                }
				            }
				            if (i)
				            	_fstrcat (pCmd,","); 
				            if (*str)
				            switch (lpFieldInfo[i].type)
				            {
				               	case BT_INTEGER:
					 			case BT_REAL:
					 			case SQL_DOUBLE:
								case SQL_INTEGER: 
								case SQL_TINYINT:
                                	_fstrcat (_fstrchr(pCmd,0),str);
                                break;
					 			default:
					 				REPLAC (str,"'","''",254);
					 				sprintf (_fstrchr(pCmd,0),"'%s'",str);
				 				break;
				            }
				            else
				            	_fstrcat (pCmd,"NULL");
		   				}
						_fstrcat (pCmd,")");
					}
					else
						goto Exit;
				    ClearCurVals (FilePtr);
				    if (DynRecordExists == 2)
				    {   
				    	HIGHLIGHTDATA	HighlightData;
				    	long	Refno;
				    	short	pos=BT_FIRST;
				    	LPSTR	pWhere = _fstrchr (pCmd,0);
				    	
	    				while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData))
						{   
							pos = BT_NEXT;
							PickList[0]=HighlightData.PD;
							SetPickGlobals (0);
							_fstrcpy (CurrentDynWhere,SQLPtr->SQL);
							ExpandText (CurrentDynWhere);
							_fstrcpy (pWhere,CurrentDynWhere);		
							ExternalSQLDirect ((int)FilePtr->FileHandle,pCmd);
						}
				    }
				    else if (!ExternalSQLDirect ((int)FilePtr->FileHandle,pCmd))
						rtn = TRUE;
				}
			Exit:
   				GlobalUnlock (hDynControls); 
   				if (wParam == IDC_SETMULTIPLE)
   				{
   					rtn = 0;  
   					ClearDynCurrentValues (hWndDlg);
					GetDynInitialValues (hWndDlg);
					DynRecordExists = 2; 
					SetDlgItemText (hWndDlg,IDC_MESSAGE,""); 
					*CurrentDynWhere = 0;
   				}

   				if (!rtn || wParam == IDC_EXIT)
   				{
					GlobalUnlock (SQLPtr->OFHandle);
					GlobalUnlock (hDDSQL);
					GSSiGlobUlFree (&hCmd); 
					if (HaltReport && wParam == IDC_EXIT)
						rtn = 0;
   					break;
   				}  
				_fstrcpy (CurrentDynWhere,SQLPtr->SQL);
				ExpandText (CurrentDynWhere);		
				ExpandTextDataNotFound=FALSE;  
				sprintf (str,"[%s]",lpFieldInfo->name);  
				GlobalUnlock (SQLPtr->OFHandle);
				GlobalUnlock (hDDSQL);
				ProcessText (str);
				if (!ExpandTextDataNotFound)
				{
					SetDynCurrentValues (hWndDlg,0,0); 
					GetDynInitialValues (hWndDlg);
					DynRecordExists = 1; 
					SetDlgItemText (hWndDlg,IDC_MESSAGE,"");
					GSSiGlobUlFree (&hCmd); 
					break;
				}
			}
			SetDynDefaultValues (hWndDlg); 
			GetDynInitialValues (hWndDlg);
			DynRecordExists = FALSE;
			SetDlgItemText (hWndDlg,IDC_MESSAGE,"Record not found");
			GSSiGlobUlFree (&hCmd); 
		}
			break;
		case IDC_DYNEDIT:  
		{
			DLGPROC lpfnEDITDYNDIALOGMsgProc;
					
			lpfnEDITDYNDIALOGMsgProc = MakeProcInstance((DLGPROC)EDITDYNDIALOGMsgProc, hInst);
			DialogBox(hInst, (LPSTR)"EDITDYNDIALOG", hWndDlg, lpfnEDITDYNDIALOGMsgProc);
			FreeProcInstance(lpfnEDITDYNDIALOGMsgProc);  
			rtn = TRUE;
			DDFileUpdated = TRUE;
		} 
		break; 
		case 65001:
			*str = 0;
			if (GetTextString (hWndDlg,str,1020,"Enter New Entry",0,0,0,TRUE,TRUE)) 
				SendMessage (hWndEdit,CB_ADDSTRING,0,(LPARAM)str); 
			DDFileUpdated = TRUE;
            break;
    	case 65002: 
    		choice = (short)SendMessage(hWndEdit,CB_GETCURSEL,0,0);
    		if (choice != CB_ERR)
    			SendMessage(hWndEdit,CB_DELETESTRING,choice,0); 
			DDFileUpdated = TRUE;
    		break;
    	case 65003:
	    	_fstrcpy (str,&pDynStrings[pControlDef[id].ValidLoc]);
			if (GetTextString (hWndDlg,str,1020,"Enter New Validation Rule",0,0,0,TRUE,TRUE)) 
			{
				pControlDef[id].ValidLoc = AddStringToList (str,pDynStrings,&lDynStrings);
		    }	
	    	_fstrcpy (str,&pDynStrings[pControlDef[id].ValidMsgLoc]);
			if (GetTextString (hWndDlg,str,1020,"Enter New Validation Rule Error Message",0,0,0,TRUE,TRUE)) 
			{
				pControlDef[id].ValidMsgLoc = AddStringToList (str,pDynStrings,&lDynStrings);
		    }	
			DDFileUpdated = TRUE;
    		break;
    	case 65004: 
    		if (pDynStrings[pControlDef[id].DefaultLoc])
	    		_fstrcpy (str,&pDynStrings[pControlDef[id].DefaultLoc]);
    		else
    			GetDlgItemText (hWndDlg,pControlDef[id].ControlID,str,sizeof(str)-1);
			if (GetTextString (hWndDlg,str,1020,"Enter Default Value",0,0,0,TRUE,TRUE)) 
			{
				pControlDef[id].DefaultLoc = AddStringToList (str,pDynStrings,&lDynStrings);
		    }
			DDFileUpdated = TRUE;
    		break;
    	case 65005:
	    	_fstrcpy (str,&pDynStrings[pControlDef[id].DefLoc]);
			if (GetTextString (hWndDlg,str,1020,"Enter Field Definition",0,0,0,TRUE,TRUE)) 
			{
				pControlDef[id].DefLoc = AddStringToList (str,pDynStrings,&lDynStrings);
		    }
			DDFileUpdated = TRUE;
		    break;	
    	case 65006:
	    	_fstrcpy (str,&pDynStrings[pControlDef[id].DefaultLoc]);
			if (GetTextString (hWndDlg,str,1020,"Enter New Default Value",0,0,0,TRUE,TRUE)) 
			{
		    	SetWindowText (hWndEdit,str);
				pControlDef[id].DefaultLoc = AddStringToList (str,pDynStrings,&lDynStrings);
		    }	
			DDFileUpdated = TRUE;
		    break; 
		case 65007:
		{    
			HANDLE	hBT;
			short	fLen;  
			short	cond=BT_FIRST, Dummy;
			
			if (!hDDSQL)
				break;
			SQLPtr = (LPOPENSQLDATA)GlobalLock (hDDSQL);
			FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
			lpFieldInfo = &FilePtr->FldInfo;
			_fstrcpy (str,&pDynStrings[pControlDef[id].DefLoc+1]);
			*LastChr (str) = 0; 
			if ((j=GetFieldIDFromName(SQLPtr->IDName,FilePtr,str))>=0) 
			{
				fLen = min (lpFieldInfo[j].length,255);    
				hBT = CreateUniqueList (fLen,0);   
				GetODBCUniqueFieldValues ((int)FilePtr->FileHandle,"",lpFieldInfo[j].name,fLen,hBT);
				cond = BT_FIRST;
				while (!BT_FIND (hBT,(LPSTR)str,cond,BT_ANY,(LPSTR)&Dummy))
				{  
					cond = BT_NEXT;   
					Truncate (str);
					if (*str)
						SendMessage (hWndEdit,CB_ADDSTRING,0,(LPARAM)str);
	            }
	            BT_CLOSEANDDELETE (&hBT);
	        }
			GlobalUnlock (SQLPtr->OFHandle); 
			GlobalUnlock (hDDSQL);
			DDFileUpdated = TRUE;
		}
		break;

    	case 65008: 
   			SendMessage(hWndEdit,CB_RESETCONTENT,0,0); 
			DDFileUpdated = TRUE;
    		break;
		case 65009:
			break;

     }
     GlobalUnlock (hDynStrings);
     GlobalUnlock (hDynControls); 
     return rtn;
}

