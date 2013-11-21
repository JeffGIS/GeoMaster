
/*
** LAS function c_errmsg from $(WORLDROOT)/baseline/source/support/notae
** modified to print the error messages to standard error instead of
** standard output.
*/

/******************************************************************************
FUNCTION:	c_errmsg

PURPOSE:

PROGRAM HISTORY:
Version    Date     Author       	Change Request
-------    -----    -------------	--------------
  5.0	   02/89    B. Davis		fixed bug in for loop of print_mes that
					searches for white space to continue
					message on next line.  Also, to help
					accommodate long file names, if no
					white space is found in a line-widths-
					worth of message, a line wrap is
					allowed to occur at a / for unix host
					file names, at a . for vms host file
					names, and at a ] for tae file names. 
  5.1	   03/89    B. Davis		Refrained from changing the value of
					vrity before passing it to z_exit.  It
					is possible for a fortran routine to
					pass 0, but the address being passed by
					fortran recieved by C is not necessarily
					an address that is allowed to be written
					to.  We will therefore assign a value to
					a local variable for passing to exit.
  5.2	  04/89	   B.Ailts		Made sure that -1 is passed to exit
					when an fatal error code is passed in.

COMPUTER HARDWARE AND/OR SOFTWARE LIMITATIONS:
	None.

PROJECT			LAS

ALGORITHM 

ALGORITHM REFERENCES:
	c_errmsg hacked for the I hate TAE crowd
*******************************************************************************/

#include <stdio.h>
#include <ctype.h>

#include "worgen.h"

FUNCTION c_errmsg( message, key, vrity )
	char	*message;
	char	*key;
	long	*vrity;

{
long	vflag;			/* long for changing the value of vrity to be */
				/* pass to exit			      */
char	os_mes[81];	/* message for OS dependent error */

/* Resolve the type of key. */
if ( key[0] == '\0' )			/* null string check */
	if ( *vrity <= 0 )			   /* fatal error */
		print_mes("Fatal error encountered.","ERROR-FATAL");
	else
		print_mes("Non-fatal error encountered.","ERROR-NONFATAL");

else if ( key[0] == ' ')
	if ( *vrity <= 0 )			   /* fatal error */
		print_mes(message,"general-error");
	else
		print_mes(message,"informational");

else if ( (strcmp(key,"unix")==0) || (strcmp(key,"UNIX")==0) )
	{
	sprintf(os_mes,"%s%s%d."," UNIX "," error code ",*vrity); 
  	print_mes(os_mes,"ERROR-UNIX");
	}

else if ( (strcmp(key,"vms")==0) || (strcmp(key,"VMS")==0) )
	{
	sprintf(os_mes,"%s%s%d."," VMS "," error code ",*vrity); 
  	print_mes(os_mes,"ERROR-VMS");
	}

else
	print_mes( message, key );

/* Decide whether or not to continue processing. 
------------------------------------------------ */
if ( *vrity <= 0 )
	{
	vflag = -1;
	exit(vflag);
	}

return;
}

#define MSG_PART_LEN 80
FUNCTION print_mes( ms, ky )
	char	*ms, *ky;

{
short	len, back;	/* counters to break up string MS */
char	*newms;		/* pointer to remaining part of ms */
			/* if it is too long for a one line */
char	restms[CMLEN];	/* string to pass recursively to print_mes */

/* If the length of MS is not greater than MSG_PART_LEN - the length of 
   the key then print it 
-----------------------------------------------------------------------*/
if ( strlen(ms) <= (MSG_PART_LEN-3)-strlen(ky) )
	{
	fprintf (stderr, "\r[%s] %s\n",ky ,ms);
	}
else
	{
	for ( back = (MSG_PART_LEN-3)-strlen(ky);
	   ((back>=0) && (!isspace(ms[back])));
		back--)
		;		/* find w. space to def substring */
	if ( back <= 0 )
	   {
	   for ( back = (MSG_PART_LEN-3)-strlen(ky);
	      ((back>=0) && (ms[back] != '/') &&
			     (ms[back] != '.') &&
			      (ms[back] != ']'));
	   	   back--)
		   ;		/* no w. space, find host path separator */
	   if ( back <= 0 ) back = (MSG_PART_LEN-3)-strlen(ky);
	   }			/*no path separator, just wrap after 80th char*/
	newms = (ms + back + 1);
	strcpy(restms,newms);
	ms[back + 1] = '\0';	/* define substr with null */
	fprintf (stderr, "\r[%s] %s\n",ky ,ms);
	print_mes( restms, ky );		/* print rest of string */
	}

return;
}
/******************************************************************************
NAME:	SQUEEZE

FUNCTION:
	SQUEEZE returns a pointer to a null terminated character string.  It
	searches for the first non-blank character of the input parameter
	"str" from the right.  If the first non-blank character is not
	the null value (0), then the next character is assigned to the null
	value.  This routine is used to null terminate a fortran character
	string.

PROGRAM HISTORY:
  Version	Date       Author       Request
  -------	----	   ------       -------
    1.0         5/86       K. Gacke     initial development
    1.1        10/86       K. Gacke     if a " " is inputted, the returned
				        string is null
    1.2	       12/87	   B.Ailts      change include directory specifications
					Use raw 'C' types
					
COMPUTER HARDWARE AND/OR SOFTWARE LIMITATIONS:	
		none

PROJECT:	LAS

ALGORITHM:
	Search from the right of a character string for the first non blank
	character.  If this character is not the null value, then it is 
	assigned to the null value.  This routine is used to null terminate
	fortran character strings.

******************************************************************************/

#include <stdio.h>

#include "worgen.h"

FUNCTION char *squeeze(str,len)
    register char *str;
    register int  len;
{
char *malloc();
register char *newstr;
register char *ptr;
long nonfatal = 1;		/* Fatal error message	*/

newstr = malloc(len+1);
if (newstr == NULL)
   c_errmsg("Error allocating dynamic memory","err-alloc",&nonfatal);

strncpy(newstr,str,len);
for(ptr = newstr + len - 1; ((*ptr == ' ') && (ptr >= newstr)); ptr--)
	;
*(ptr + 1) = '\0';
return(newstr);
}
