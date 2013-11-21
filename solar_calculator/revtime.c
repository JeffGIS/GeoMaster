/* Reverse time order of ephemeris records and combine files
 */

#define MTOPC 0

#include <stdio.h>
#ifdef unix
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

int fi, fo;
#define RECSIZ 584
char dbuf[RECSIZ];
void revbytes();

int
main()
{
char str[40];
long bcnt, recno;

opno:
printf( "Output file ? " );
gets( str );
fo = open( str, O_BINARY | O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE );
if( fo <= 0 )
	{
	printf( "can't open %s\n", str );
	goto opno;
	}

opni:
printf( "input file ? " );
gets( str );
if( str[0] == '\0' )
	{
	close( fo );
	exit(0);
	}
fi = open( str, O_BINARY | O_RDONLY, S_IREAD );
if( fi <= 0 )
	{
	printf( "can't open %s\n", str );
	goto opni;
	}
printf( "Number of records ? " );
gets( str );
sscanf( str, "%ld", &recno );

while( --recno >= 0 )
	{
	bcnt = RECSIZ * recno;
	lseek( fi, bcnt, SEEK_SET );
	if( read( fi, dbuf, RECSIZ ) != RECSIZ )
		{
		printf( "file read error\n" );
		exit(0);
		}
#if MTOPC
	revbytes();
#endif
	write( fo, dbuf, RECSIZ );
	}
goto opni;
}


void
revbytes()
{
register char *p, *q;
register char a, b;
int i;

p = dbuf;
q = dbuf + 7;
for( i=0; i<RECSIZ/8; i++ )
	{
	a = *p;
	b = *q;
	*p++ = b;
	*q-- = a;

	a = *p;
	b = *q;
	*p++ = b;
	*q-- = a;

	a = *p;
	b = *q;
	*p++ = b;
	*q-- = a;

	a = *p;
	b = *q;
	*p = b;
	*q = a;
	p += 5;
	q += 11;
	}
}
