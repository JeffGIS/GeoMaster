
BOOL	OpenUM (void);
void	CloseUM (BOOL Error);
int		GetList (HWND hWndDlg,int dialogItem,LPSTR listid);
int		GetListCB (HWND hWndDlg,int dialogItem,LPSTR listid);
int 	GetListSub (HWND hWndDlg,int dialogItem, int RESET, int ADD, LPSTR listid);
void	SendUMMessage (LPSTR message);
int		GetUMMessage (LPSTR message);
void CBInitIO (UserInfo UI2);
int OpenTCPIPSocket (HWND hWnd,LPSTR TCPAddress,USHORT Port,SOCKET *pSock,LPSTR InputTerminator,LPSTR InputProcessString,LPSTR CloseProcessString,LPSTR RestartProcessString,BOOL Notify);
short SendTCPIPString (SOCKET socket,LPSTR string,BOOL SendNullTerm,UINT WaitSeconds); 
BOOL SetupSocketTimer (SOCKET socket,long Seconds,LPSTR TimerString);
BOOL SetupSocketTerminator (SOCKET socket,LPSTR Term);
void ProcessSocketTimer (int TimerID);
void ProcessSocketError (SOCKET sock,int err,LPSTR Info);
BOOL OpenWinSock (void);
void CloseWinSock (BOOL ProcessCloseString);
BOOL ProcessTCPData (SOCKET sock);
BOOL ProcessTCPCmd (SOCKET sock,LPSTR Cmd);
BOOL AcceptTCPConnection (HWND hWnd,SOCKET sock);
BOOL CloseTCPIPSocket (SOCKET socket,BOOL ProcessCloseString);
BOOL RestartSocket (SOCKET sock); 
BOOL OpenTCPIPServer (LPSTR IPAddress,USHORT Port); 
BOOL OpenTCPIPServer2 (HWND hWnd);
short CheckForRegistrationServer (HWND hWnd,LPSTR SerialNum);
BOOL FAR PASCAL REGCONNECTMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam);
BOOL BlockSocketProcessing (BOOL Block);
void SetLogSocketIO (BOOL set);
BOOL OpenTCPReplay (LPSTR ReplayFile);
void ProcessReplayTCPTimer (void);
void DisplayLenSocketBuffer (int i);
int GetSocketID (SOCKET sock);
BOOL GetMyIPNetAddress (LPSTR inetAddr);
BOOL PaintServerInfo (HDC hDC,LPRECT pRect); 
SOCKET connectToServer ( char *serverName, u_short port,BOOL UpdateServerOpen, LPINT perr );
BOOL ReadNTBlock (SOCKET sock,LPSTR pBuf,long lBuf);
BOOL SendNTBlock (SOCKET sock,LPSTR pBuf,long lBuf);
void ResetSocketLog(void);
void LogServerActivity(LPSTR Mess);



