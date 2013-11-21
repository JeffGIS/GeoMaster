/*
	Features Program

	Ultimate Grid Example Program Number 1
	From Dundas Software Ltd.

	NOTE:
		Parts of this code may be used to create new programs.
		This source may not be redistributed.


	Purpose of program:
		- shows how data is retrieved from a propriatary data file
		- shows the different ways to adjust column widths
		- shows the different text color and alignment possiblities
		- shows how to adjust the table size when its parent window resizes

*/

#include "graphint.h"
#include "ugtable.h"   
#include "ugrid.h" 

extern	HANDLE	hHighlight, hHighlight2;
extern	HINSTANCE hInst; 

static	HIGHLIGHTDATA	HighlightData;

long far pascal UGRID_HLTDlgProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

/*******************************************
********************************************/
/*int ShowGrid(HWND hWnd){

	DLGPROC dlgproc;
	int		rc;

	//create a modal dialog box
	dlgproc = (DLGPROC) MakeProcInstance((FARPROC)UGRID1DlgProc, hInst);
	rc=DialogBox(hInst,"UGRID1",hWnd, dlgproc);
	FreeProcInstance((FARPROC)dlgproc);

	return 0;
}*/

/*******************************************
********************************************/
long far pascal UGRID_HLTDlgProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam){

	//general purpose variables
	int  		t,x;
	long 		l,rowloc;
	double 	d;
	char 		string[50];
	RECT 		rect;
	HMENU 	hmenu;
	LPWINDOWPOS wp;
	DLGPROC dlgproc;  
	OFSTRUCT	OFStruct; 
	LPSTR	lpStr; 
	int		nCol; 
	char	val[256];

	char *hdg[]={"Number","Type","TAG","Length","Area","Perimeter"};

 	static TABLEINFO far *ti;


	static DATA data;             //data file structure
	static HFILE fptr;				//file pointer to the data file
	static long lastrow=-1;			//saves the last row that the table asked for
	static int 	savecol[5];			//stores the column widths
	static int 	align=TA_LEFT;		//text alignment flag
	static int 	color=0;				//text color flag
	static int 	fittowindow=0;		//Fit-To-Window flag
	static HBRUSH hbrush;			//brush handle
	static int vlines=1;				//separation lines ON/OFF flag   
	static HANDLE	hData;
	static LPSTR	pData;
	static long		maxrowlen, startindex;

	//process messages
	switch(message){

		case WM_INITDIALOG:{
             
            BTHEAD	BTHead; 
			//create a gray brush (to be used for the dialog background)
			hbrush=CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

			//open up the data file
			//get the number of records in the datafile    
			GetBTHeader (hHighlight,&BTHead);
			l =  BTHead.BT_NUMRECS;
			nCol=6;
			//get the tableinfo structure
			ti = (TABLEINFO far *)SendMessage(GetDlgItem(hwnd,IDC_TABLE),
         	TB_GETADDRESS,0,0);

			//set up the table
			TB_SetupTable(
				GetDlgItem(hwnd,IDC_TABLE),		//table window handle
				GetDlgItem(hwnd,IDC_TABLEHDG),   //table heading window handle
				l,                               //number of rows
				nCol,                            //number of columns
				NULL,                            //column widths
				3,                               //interspace value
				TRUE);                           //user resize option



			//send a WM_SIZE (to adjust the windows to fit within the parent);
			PostMessage(hwnd,WM_SIZE,0,0);

			//set the focus to the table window
			SetFocus(GetDlgItem(hwnd,IDC_TABLE));

			return 0;
		}

		case WM_CTLCOLORDLG:{
			//if the dialog box is to be painted then return the gray brush
			if(HIWORD(lParam)==CTLCOLOR_DLG ||HIWORD(lParam)==CTLCOLOR_STATIC){
				SetBkColor(wParam,GetSysColor(COLOR_BTNFACE));
				return hbrush;
			}
			return NULL;
		}

		case WM_DESTROY:{
			//close the data file  
			_lclose(fptr);
			//delete the brush
			DeleteObject(hbrush);
			return 0;
		}

		case WM_CLOSE:{
			//close the dialog
			EndDialog(hwnd,0);
			return 0;
		}

		case WM_WINDOWPOSCHANGING:{
      	// this message is called just before the WM_SIZE message

			// make sure that the window parent window is greater than the minimum
			// specified size limit when it is resized
			// if it isn't then adjust the window to the minimum size
			wp=(LPWINDOWPOS)lParam;
			//min height is 210 pixels
			if(wp->cy<210) wp->cy=210;
			//min width is 400 pixels
			if(wp->cx<400) wp->cx=400;

			return 0;
		}
		case WM_SIZE :{
			//resizes the child windows to fit inside the parent window

			//get the size of the client window
			GetClientRect(hwnd,&rect);
			//adjust the table child window to fit in the parent window
			SetWindowPos(GetDlgItem(hwnd,IDC_TABLE),HWND_TOP,25,88,rect.right-50,
				rect.bottom-100,SWP_NOZORDER);
			//adjust the heading child window to fit
			SetWindowPos(GetDlgItem(hwnd,IDC_TABLEHDG),HWND_TOP,25,60,
				rect.right-48-GetSystemMetrics(SM_CYVSCROLL),18,SWP_NOZORDER);

			//if the fittowindow flag is set then readjust the coulmn widths to
			//fit inside the new table child window width
			if(fittowindow){
				TB_FitToWindow(GetDlgItem(hwnd,IDC_TABLE),0);
			}
			return 1;
		}

		case WM_COMMAND:{
			switch(wParam){

				case IDC_TABLE:{

					//find the message sent (ti->msg)
					switch((HIWORD(lParam))){
						case TBN_WANTTEXT :{
							// check to see if the row is the same as the last
							// if not then get a new record from the database
							// (this way a record doesnt need to be read in each time
							// a cell within the table needs to be drawn)   
							
							long	Refno;
							
							if(ti->row!=lastrow){
								//find the record that coresponds to the row given
								lastrow=ti->row; 
								BT_FIND (hHighlight2,(LPSTR)&ti->row,BT_FIRST,BT_EQ,(LPSTR)&Refno);
								BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData);
							}

							// get the field that co-responds to the column and put it
							// in the ti->buf parameter     
							Varptr = GlobalLock(DLTVar[ti->col]); 
							_fstrncpy(val,Varptr->Value,Varptr->Len);   
							val[Varptr->Len]=0; 
							if (val[0])
								_fstrcpy(ti->buf,val);
							else
								_fstrcpy(ti->buf,"   ");
							GlobalUnlock(DLTVar[ti->col]);
							// set the alignment for the first three fields according
							// to the alignment selected from the menu
							if(ti->col < 3){
									ti->alignment=align;
							}

							// menu selected color options
							if(color>0){
								//color every fifth line
								if(color==1){
									if((ti->row%5)==0){
										ti->textcolor=GetSysColor(COLOR_WINDOWTEXT);
										ti->backcolor=RGB(0,255,0);
									}
								}
								//color all outstanding balances
								else if(color==2){
									d=data.Balance;
									if(d>0){
										ti->textcolor=GetSysColor(COLOR_WINDOWTEXT);
										ti->backcolor=RGB(0,255,0);
									}
								}
								//color all selected
								else if(color==3){
									if(data.flag==1){
										ti->textcolor=GetSysColor(COLOR_HIGHLIGHTTEXT);
										ti->backcolor=GetSysColor(COLOR_HIGHLIGHT);
									}
								}
							}
							return 1;
						}

						case TBN_ROWCHANGE :{
							//display the new row number in the status window
							wsprintf(string,"Row Changed To: %ld",ti->row);
							SetDlgItemText(hwnd,IDC_STATUS,string);

							return 1;
						}

						case TBN_COLCHANGE :{
							//display the new column number in the status window
							wsprintf(string,"Col Changed To: %d",ti->col);
							SetDlgItemText(hwnd,IDC_STATUS,string);

							return 1;
						}

						case TBN_ROWSELECTED:{
							//display the row/column that was selected
							wsprintf(string," Row:%ld  Col:%d  Selected",ti->row,ti->col);
							SetDlgItemText(hwnd,IDC_STATUS,string);

							//if multiple selection is on then tag/untag the field
							if(color==3){
								//retrive the current record
								fseek(fptr,ti->row * sizeof(DATA),SEEK_SET);
								fread(&data,sizeof(DATA),1,fptr);

								//if it is not already selected then select it
								if(data.flag==0){
									data.flag=1;
								}
								//otherwise un-select it
								else{
									data.flag=0;
								}

							  	//save the record
								fseek(fptr,ti->row * sizeof(DATA),SEEK_SET);
								fwrite(&data,sizeof(DATA),1,fptr);

								//clear the lastrow flag
								lastrow=-1;

								//redraw the table so the changes will be shown
								TB_RedrawTable(GetDlgItem(hwnd,IDC_TABLE));
							}
							//create a modal dialog box
							dlgproc = (DLGPROC) MakeProcInstance((FARPROC)InformationDlgProc, hInst);
							DialogBoxParam(hInst,"INFORMATION",NULL, dlgproc,
								GetDlgItem(hwnd,IDC_TABLE));
							FreeProcInstance((FARPROC)dlgproc);

							return 1;
						}

						case TBN_KEYBOARD:{
							//display the key that was hit
							wsprintf(string,"Key: %c",ti->wParam);
							SetDlgItemText(hwnd,IDC_STATUS,string);
							//search the database for the closest match
							t=0;					//set the counter to zero
							rewind(fptr);     //start from the beginning of the file
							while(1){
								//retrive a record
								x=fread(&data,sizeof(DATA),1,fptr);
								if(x==0){
                        	t--;
									break;
								}

								if(data.Company[0] >= ti->wParam){
									break;
								}
								t++;
							}
							//clear the lastrow flag
							lastrow=-1;

							//update the table position
							SendDlgItemMessage(hwnd,IDC_TABLE,TB_GOTOROW,0,t);

							return 1;
						}
					}
					return 1;
				}
				case IDC_TABLEHDG:{

					switch(ti->msg){
						case TBN_WANTHDG :{
							//set the text buffer to the column name    
							Varptr = GlobalLock(DLTVar[ti->col]); 
							_fstrcpy(ti->buf,Varptr->Name);
							GlobalUnlock(DLTVar[ti->col]);
							
							return 1;
						}
					}
					return 1;
				}

				case IDC_SEARCH:{
					//if the search edit box has changed do a new search
					if(HIWORD(lParam)==EN_CHANGE){

						//get the text from the control
						GetDlgItemText(hwnd,IDC_SEARCH,string,50);

						//search the database for the closest match
						t=0;					//set the counter to zero
						rewind(fptr);     //start from the beginning of the file
						while(1){
							//retrive a record
							x=fread(&data,sizeof(DATA),1,fptr);
							if(x==0){
								t--;
								break;
							}

							if(stricmp(data.Company,string)>=0){
								break;
							}
							t++;
						}
						//clear the lastrow flag
						lastrow=-1;

						//update the table position
						SendDlgItemMessage(hwnd,IDC_TABLE,TB_GOTOROW,0,t);
					}
					return 0;
				}

				case ID_GOTO_TOP:{
					//update the table position
					SendDlgItemMessage(hwnd,IDC_TABLE,TB_GOTOROW,0,0);

					return 0;
				}

				case ID_GOTO_BOTTOM:{
					//get the number of records in the database
               fseek(fptr,0,SEEK_END);
					l=(ftell(fptr) / sizeof(DATA)) -1;

					//update the table position
					SendDlgItemMessage(hwnd,IDC_TABLE,TB_GOTOROW,0,l);

					return 0;
				}

				case ID_SAVE_WIDTHS:{
					//retrieve the widths of the columns in the table and save them
					for(t=0;t<3;t++){
						savecol[t]=(int)SendDlgItemMessage(hwnd,IDC_TABLE,TB_GETCOLWIDTH,t,0);
					}

					return 0;
				}

				case ID_RESTORE_WIDTHS:{
					//use the previously saved column widths to set the coulmn withs
					for(t=0;t<3;t++){
						SendDlgItemMessage(hwnd,IDC_TABLE,TB_SETCOLWIDTH,
							t,savecol[t]);
					}
					//redraw the table
					TB_RedrawTable(GetDlgItem(hwnd,IDC_TABLE));

					return 0;
				}

				case ID_TEXT_LEFT:{
					//set the alignment flag to 1 (1=left)
					align=TA_LEFT;
					//redraw the table
					TB_RedrawTable(GetDlgItem(hwnd,IDC_TABLE));
					return 0;
				}

				case ID_TEXT_RIGHT:{
					//set the alignment flag to 2 (2=right)
					align=TA_RIGHT;
					//redraw the table
					TB_RedrawTable(GetDlgItem(hwnd,IDC_TABLE));
					return 0;
				}

				case ID_TEXT_CENTER:{
					//set the alignment flag to 3 (3=center)
					align=TA_CENTER;
					//redraw the table
					TB_RedrawTable(GetDlgItem(hwnd,IDC_TABLE));
					return 0;
				}

				case ID_COLOR_NONE:{
					//set the color flag to 0 (0=no color)
					color=0;
					//redraw the table
					TB_RedrawTable(GetDlgItem(hwnd,IDC_TABLE));
					return 0;
				}

				case ID_COLOR_5:{
					//set the color flag to 0 (1=color every fifth line)
					color=1;
					//redraw the table
					TB_RedrawTable(GetDlgItem(hwnd,IDC_TABLE));
					return 0;
				}

				case ID_COLOR_OUTSTANDING:{
					//set the color flag to 2 (0=color outstanding balances)
					color=2;
					//redraw the table
					TB_RedrawTable(GetDlgItem(hwnd,IDC_TABLE));
					return 0;
				}

				case ID_MULTIPLE:{

					//set the color flag to 3 (3=color selected records)
					color=3;

					//clear the all of the select fields in the database
					rewind(fptr);     //start from the beginning of the file
					while(1){
						//store the current file pos
						l = ftell(fptr);
						//retrive a record
						if( fread(&data,sizeof(DATA),1,fptr) ==0){
							//finished
							break;
						}
						//clear the flag
						data.flag=0;
                  //move back to the beginning of this record
						fseek(fptr,l,SEEK_SET);
						//save the record
						fwrite(&data,sizeof(DATA),1,fptr);
					}
					rewind(fptr);

					//clear the last database record position flag
					lastrow=-1;

					//redraw the table
					TB_RedrawTable(GetDlgItem(hwnd,IDC_TABLE));

					return 0;
				}

				case ID_VLINES:{
					if(vlines==0){
						vlines=1;
					}
					else{
						vlines=0;
					}
					SendDlgItemMessage(hwnd,IDC_TABLE,TB_SETVLINES,vlines,0);
					return 0;
				}

				case ID_BESTFIT:{
					//clear the fit to window settings (just in case it was set)
					hmenu=GetMenu(hwnd);
					hmenu=GetSubMenu(hmenu,1);
					CheckMenuItem(hmenu,ID_FITTOWINDOW,MF_BYCOMMAND | MF_UNCHECKED);
					fittowindow=0;

					//search the whole table for the best coulmn widths
					TB_BestFit((HWND) GetDlgItem(hwnd,IDC_TABLE),3,0,NULL);
					
					return 0;
				}

				case ID_FITTOWINDOW:{
					//check to  see if the menu item is checked or not
					hmenu=GetMenu(hwnd);
					hmenu=GetSubMenu(hmenu,1);
					t=GetMenuState(hmenu,ID_FITTOWINDOW,MF_BYCOMMAND);

					if(t & MF_CHECKED){
						//uncheck the menu item
						CheckMenuItem(hmenu,ID_FITTOWINDOW,MF_BYCOMMAND | MF_UNCHECKED);
						//clear the fit to window flag
						fittowindow=0;
					}
					else{
						//check the menu item
						CheckMenuItem(hmenu,ID_FITTOWINDOW,MF_BYCOMMAND | MF_CHECKED);
						//fit to window
						TB_FitToWindow(GetDlgItem(hwnd,IDC_TABLE),0);
						//set the fit to window flag for future window sizings
						fittowindow=1;
					}

					return 0;
				}
				case ID_SETCOLWIDTHS:{
					//create a modal dialog box
					dlgproc = (DLGPROC) MakeProcInstance((FARPROC)WidthDlgProc, hInst);
					DialogBoxParam(hInst,"SETCOLUMNWIDTH",NULL, dlgproc,
						GetDlgItem(hwnd,IDC_TABLE));
					FreeProcInstance((FARPROC)dlgproc);

					return 0;
				}
				case ID_EXIT:{
					//close the dialog
					EndDialog(hwnd,0);
					return 0;

				}
			}
		}
	}
	return 0;
}

long far pascal WidthDlgProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam){

	int i,i2;
	char string[99];
	static HWND tblhwnd;

	switch(message){
		case WM_INITDIALOG:{
			//store the table window handle
			tblhwnd = (HWND)lParam;
			//set up the starting column
			SetDlgItemText(hwnd,ID_COLUMN,"0");
			//set up the default radio button
			CheckRadioButton(hwnd,ID_RADPIXELS,ID_RADCHARWIDTH,ID_RADPIXELS);
			return 1;
		}
		case WM_COMMAND:{
			switch(wParam){
				case ID_OK:{
					//get the column to set
					GetDlgItemText(hwnd,ID_COLUMN,string,99);
					i = atoi(string);
					if(i<4){
						//get the width
						GetDlgItemText(hwnd,ID_WIDTH,string,99);
						i2 = atoi(string);
						if(IsDlgButtonChecked(hwnd,ID_RADPIXELS)){
							//set the column width in pixels
							SendMessage(tblhwnd,TB_SETCOLWIDTH,i,i2);
						}
						else{
							//set the column width in ave char widths
							SendMessage(tblhwnd,TB_FSETCOLWIDTH,i,i2);
						}
                  //redraw the table
                  TB_RedrawTable(tblhwnd);
               }
					//end the dialog
					EndDialog(hwnd,0);
					return 0;
				}
				case ID_COLUMN:{
					if(HIWORD(lParam)==EN_CHANGE){
						//get the value
						GetDlgItemText(hwnd,ID_COLUMN,string,99);
						i = atoi(string);
						//find the current width of the column specified
						i = (int)SendMessage(tblhwnd,TB_GETCOLWIDTH,i,0);
						//change the value into a string
						wsprintf(string,"%d",i);
						//put the string into the edit control
						SetDlgItemText(hwnd,ID_WIDTH,string);

						return 0;
					}
				}
				case ID_RADPIXELS:{
					CheckRadioButton(hwnd,ID_RADPIXELS,ID_RADCHARWIDTH,ID_RADPIXELS);
					return 0;
				}
				case ID_RADCHARWIDTH:{
					CheckRadioButton(hwnd,ID_RADPIXELS,ID_RADCHARWIDTH,ID_RADCHARWIDTH);
					return 0;
				}
			}
		}
	}
	return 0;
}

long far pascal InformationDlgProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam){

	int i;
	char string[99];
	TABLEGETRECT gr;

	static int col;
	static long row;
	static HWND tblhwnd;

	switch(message){
		case WM_INITDIALOG:{
			//store the tables window handle
			tblhwnd =(HWND)lParam;
			//***** set up the edit boxes *****
			//get the column
			col=(int)SendMessage(tblhwnd,TB_GETSELCOL,0,0);
			wsprintf(string,"%d",col);
			SetDlgItemText(hwnd,ID_COLUMN,string);
			//get the row
			row=SendMessage(tblhwnd,TB_GETSELCOL,0,0);
			wsprintf(string,"%ld",row);
			SetDlgItemText(hwnd,ID_ROW,string);

			//fill the rest of the info in
			SendMessage(hwnd,WM_USER,0,0);
			return 0;
		}
		case WM_USER:{

			//get the font handle
			i=(int)SendMessage(tblhwnd,TB_GETFONT,0,0);
			wsprintf(string,"%d",i);
			SetDlgItemText(hwnd,ID_FONT,string);
			//get the Ave char width
			i=(int)SendMessage(tblhwnd,TB_GETCHARWIDTH,0,0);
			wsprintf(string,"%d",i);
			SetDlgItemText(hwnd,ID_CHARWIDTH,string);
			//get the column width
			i=(int)SendMessage(tblhwnd,TB_GETCOLWIDTH,col,0);
			wsprintf(string,"%d",i);
			SetDlgItemText(hwnd,ID_COLWIDTH,string);
			//get the rectangle
			gr.col=col;
			gr.row=row;
			i=(int)SendMessage(tblhwnd,TB_GETRECT,0,(LPARAM)(TABLEGETRECT far *)&gr);
			wsprintf(string,"%dx%dx%dx%d",gr.rect.left,gr.rect.top,gr.rect.right,gr.rect.bottom);
			SetDlgItemText(hwnd,ID_RECTANGLE,string);

			return 1;
		}
		case WM_COMMAND:{
			switch(wParam){
				case ID_OK:{
					EndDialog(hwnd,0);
					return 1;
				}
				case ID_COLUMN:{
					if(HIWORD(lParam) == EN_CHANGE){
						//get the value
						GetDlgItemText(hwnd,ID_COLUMN,string,99);
						col = atoi(string);
					}
					return 1;
				}
				case ID_ROW:{
					if(HIWORD(lParam) == EN_CHANGE){
						//get the value
						GetDlgItemText(hwnd,ID_COLUMN,string,99);
						row = atoi(string);
					}
				}
			}
		}
	}
	return 0;
}

