#include <winsock.h>

int PASCAL FAR __export shutdown (SOCKET s, int how){return 0;}
unsigned long PASCAL FAR __export inet_addr (const char FAR * cp){return 0;}
int PASCAL FAR __export WSAGetLastError(void){return 0;}
int PASCAL FAR __export recv (SOCKET s, char FAR * buf, int len, int flags){return 0;}
int PASCAL FAR __export closesocket (SOCKET s){return 0;}
int PASCAL FAR __export gethostname (char FAR * name, int namelen){return 0;}
int PASCAL FAR __export WSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData){return 1;}
u_short PASCAL FAR __export htons (u_short hostshort){return 0;}
int PASCAL FAR __export WSACancelBlockingCall(void){return 1;}
BOOL PASCAL FAR __export WSAIsBlocking(void){ return(FALSE);} 
int PASCAL FAR __export connect (SOCKET s, const struct sockaddr FAR *name, int namelen){return 0;}
int PASCAL FAR __export sendto (SOCKET s, const char FAR * buf, int len, int flags,
                       const struct sockaddr FAR *to, int tolen){return 0;}
SOCKET PASCAL FAR __export socket (int af, int type, int protocol){return 0;}
struct hostent FAR * PASCAL FAR __export gethostbyname(const char FAR * name)
{
	struct hostent x;
	
	return &x;
}
int PASCAL FAR __export WSACleanup(void){return 0;}
