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

#define NB 8192
char buff[NB];

int
main(argc,argv)
int argc;
char **argv;
{
int fdi, fdo, i, n, m;
char *p, *q;
char c;


if( argc > 0 )
	printf( "%s:  Reverse bytes in 8-byte values \n", argv[0] );
if( argc > 1 )
	strcpy( buff, argv[1] );
else
	{
	printf( "Input file name ? " );
	gets( buff );
	}
fdi = open( buff, O_BINARY | O_RDONLY, S_IREAD );

if( fdi <= 0 )
	{
	printf( "Can't open <%s>\n", buff );
	exit(2);
	}

if( argc > 2 )
	strcpy( buff, argv[2] );
else
	{
	printf( "Output file name ? " );
	gets( buff );
	}
fdo = open( buff, O_BINARY | O_RDWR | O_CREAT | O_TRUNC,
      S_IREAD | S_IWRITE );
if( fdo <= 0 )
	{
	printf( "Can't open <%s>\n", buff );
	exit(2);
	}

while( (n = read( fdi, buff, NB )) > 0 )
	{
	m = n / 8;
	p = buff;
	q = buff+7;
	for( i=0; i<m; i++ )
		{
		c = *p;
		*p++ = *q;
		*q-- = c;
		c = *p;
		*p++ = *q;
		*q-- = c;
		c = *p;
		*p++ = *q;
		*q-- = c;
		c = *p;
		*p++ = *q;
		*q-- = c;
		p += 4;
		q += 12;
		}
	write( fdo, buff, n );
	}
close( fdo );
close( fdi );
exit(0);
}
