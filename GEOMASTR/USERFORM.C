#include "graphint.h"
#include "gmextern.h"

#define MAXUSERCONTROLS	512
#define	MaxUserCheckbox	14
#define MaxUserLists	4
#define	MaxUserTextbox	4    

static	UINT cntls[]={
						IDC_USERCHECKBOX1,
						IDC_USERCHECKBOX2,
						IDC_USERCHECKBOX3,
						IDC_USERCHECKBOX4,
						IDC_USERCHECKBOX5,
						IDC_USERCHECKBOX6,
						IDC_USERCHECKBOX7,
						IDC_USERCHECKBOX8,
						IDC_USERCHECKBOX9,
						IDC_USERCHECKBOX10,
						IDC_USERCHECKBOX11,
						IDC_USERCHECKBOX12,
						IDC_USERCHECKBOX13,
						IDC_USERCHECKBOX14,
						IDC_USERCHECKBOX15,
						IDC_USERCHECKBOX16,
						IDC_USERCHECKBOX17,
						IDC_USERCHECKBOX18,
						IDC_USERCHECKBOX19,
						IDC_USERCHECKBOX20,
						IDC_USERCHECKBOX21,
						IDC_USERCHECKBOX22,
						IDC_USERCHECKBOX23,
						IDC_USERCHECKBOX24,
						IDC_USERCHECKBOX25,
						IDC_USERCHECKBOX26,
						IDC_USERCHECKBOX27,
						IDC_USERCHECKBOX28,
						IDC_USERCHECKBOX29,
						IDC_USERCHECKBOX30,
						IDC_USERCHECKBOX31,
						IDC_USERCHECKBOX32,
						IDC_USERCHECKBOX1};

BOOL FAR PASCAL USERFORMMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{

 int	BRtn; 
 
 if (Message == WM_INITDIALOG)
 {  
 	char	FileName[128];
 	
 	GetGlobalCVal ("[%ARG1]",FileName,NULL);
 	SetDynDlgData (hWndDlg,FileName,cntls,sizeof (cntls));
 }
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
 	return (BRtn);
   switch (Message)
      {
   case WM_INITDIALOG: 
   		hWndUserForm = hWndDlg; 
   		cwCenterInVP (hWndDlg,CenterVP);
		PostMessage(hWndDlg, GSSI_REPOSITION,0, 0L); 
		break;
   case GSSI_REPOSITION:
   		cwCenterInVP (hWndDlg,CenterVP);
   		break;
   case WM_DESTROY:
        hWndUserForm = 0;  
       	EnableWindow (hWndMain,TRUE);
        break; 
   case WM_COMMAND:
      switch (wParam)
         {
          case IDC_EXIT: 
	      case IDCANCEL:
		      DestroyWindow(hWndDlg); 
	          break;
	      default:
	         return FALSE;
	     } 
      break;

   default:
      return FALSE;
      }
   return TRUE;
}

