//in LRESULT CALLBACK __export ModifyProc(HWND Window, UINT Message,
//          WPARAM Param1, LPARAM Param2)      
// find and replace the VK_TAB section with


    else if(Param1 == VK_TAB)
    {
      HWND Next = NULL;
      int Flag = GW_HWNDNEXT, Num = 0;
      BOOL IsVis = FALSE;
      if(GetAsyncKeyState(VK_SHIFT) & 0X8000)
         Flag = GW_HWNDPREV;
      if(Edit->NSelected > 0)
         Next = GetWindow(Edit->Selections[Edit->NSelected-1],Flag);
      if(!IsWindowVisible(Next))
           Next = NULL;
         
      if(Next == NULL)
      {
         Next = GetWindow(Dialog, GW_CHILD);
         if(Next != NULL && Flag == GW_HWNDPREV)
            Next = GetWindow(Next,GW_HWNDLAST);
            while(!IsWindowVisible(Next) && Num < 200)
            {
              Next = GetWindow(Next,Flag);
              Num++;
            }
      }
      Edit->NSelected = 0;
      if(Next != NULL)
      {
        Edit->Selections[Edit->NSelected++] = Next;
        BoundSelections(Edit,Window);
        PostMessage(Window,WDM_REDRAW,0,0);
      }  
     }         
         GlobEdit = Edit;
    }
    
// in  BOOL FAR PASCAL DialogProc(HWND hDlg,WORD Msg,WPARAM wParam,LPARAM lParam)   
// find and replace the VK_TAB section with this

        case VK_TAB:
        {HWND Rover = NULL;
         BOOL IsVis = FALSE;
         int Type = 0, Num = 0;
         int Flag = GW_HWNDNEXT;
         if(GetAsyncKeyState(VK_SHIFT) & 0X8000) Flag = GW_HWNDPREV;
         Rover = GetFocus();
here:    Rover = GetWindow(Rover,Flag);
         if(!IsWindowVisible(Rover))
           Rover = NULL;
         if(Rover == NULL)
         {
           Rover = GetWindow(hDlg, GW_CHILD);
           if(Rover != NULL && Flag == GW_HWNDPREV)
           {//I want to find the last visible window.
             Rover = GetWindow(Rover,GW_HWNDLAST);
             while(!IsWindowVisible(Rover) && Num < 200)
             {
               Rover = GetWindow(Rover,Flag);
               Num++;
             }
           }  
         }
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
         } 
        break;
--=====================_864856103==_--



----------------------- Headers --------------------------------
From klovely@pressenter.com  Wed May 28 11:17:32 1997
Return-Path: <klovely@pressenter.com>
Received: from mrin06.mail.aol.com (mrin06.mail.aol.com [152.163.125.86])
	  by emin16.mail.aol.com (8.8.5/8.8.5/AOL-4.0.0)
