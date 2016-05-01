#include "jwsmtp.h"
#include <time.h>
extern "C" BOOL GetGlobalCVal (LPSTR Global,LPSTR Val,LPSTR Default);

void GetTimeString (LPSTR timeStr)
{
    struct tm   *newTime;
    time_t      szClock;

    // Get time in seconds
    time( &szClock );

    // Convert time to struct tm form 
    newTime = localtime( &szClock );

    strcpy (timeStr,asctime( newTime ) );  
    // Note: asctime is deprecated; consider using asctime_s instead
	return;
}

extern "C" void SendEmailToJeff (LPSTR Info)
{
		char	subjectLine[256], timeLine[64]="";
		char	server[128], user[128], pw[64];
		std::string	respons;
		int	ii;

		GetGlobalCVal ("[%MAILSERVER]",server,"smtp.frontiernet.net");
		GetGlobalCVal ("[%MAILUSER]",user,"jeffgis@frontiernet.net");
		GetGlobalCVal ("[%MAILPW]",pw,"SRINGOS");
		GetTimeString (timeLine);
		sprintf (subjectLine,"%s at %s",Info,timeLine);
		jwsmtp::mailer m("jeffgis@aol.com", "jeffgis@aol.com", subjectLine,
                    "Nothing to see here folks","mg-mail.ci.maple-grove.mn.us",//"smtp.frontiernet.net",
                    jwsmtp::mailer::SMTP_PORT, false);

//		m.setmessageHTML(html);

		if (*user)
		{
			m.username(user);
			m.password (pw);
		}
//		m.attach ("c:\\temp\\updates_2011_04_21.zip");
		m.send(); // send the mail
		respons = m.response();
		m.reset();
		ii=1;
 	}

extern "C" BOOL SendEmail (LPSTR From,LPSTR To,LPSTR Subject,LPSTR Message,LPSTR Attach,LPSTR Response)
{
	char	server[128], user[128], pw[64], Bcc[128] = { 0 };
		char * pSC;
		std::string	respons;

		if (*To == '(')
		{
			LPSTR pEnd = strrchr (To,')');

			if (pEnd)
			{
				To++;
				*pEnd = 0;
			}
		}
		pSC = strchr(To, ';');
		if (pSC)
		{
			*pSC++ = 0;
			strcpy(Bcc, pSC);
		}
		GetGlobalCVal ("[%MAILSERVER]",server,"smtp.frontiernet.net");
		GetGlobalCVal ("[%MAILUSER]",user,"jeffgis@frontiernet.net");
		GetGlobalCVal ("[%MAILPW]",pw,"SRINGOS");
		jwsmtp::mailer m(To,From,Subject,Message,
						 server,
						 jwsmtp::mailer::SMTP_PORT, false);

//		m.setmessageHTML(html);

		if (*user)
		{
			m.username(user);
			m.password (pw);
		}
		if (*Attach)
			m.attach (Attach);
		if (*Bcc)
			m.addrecipient(Bcc, jwsmtp::mailer::Bcc);
		m.send(); // send the mail
		respons = m.response();
		strcpy (Response,respons.c_str());
		m.reset();
	return TRUE;
}

