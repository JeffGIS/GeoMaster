#include <assert.h>
#include <stdio.h>

/* for getopt */
extern char    *optarg;
extern int      opterr;
extern int      optind;
extern int      optopt;
#ifdef __STDC__
#include <sys/types.h>
extern void     mc(FILE *fin);
extern char *   getenv(const char *name);
extern void     perror(const char *s);
extern void     free(void *ptr);
extern void *   malloc(size_t size);
extern void     exit(int status);
extern int      getopt(int argc, char* const *argv, const char *optstring);
extern int      atoi(const char *ptr);
extern int      strcmp(const char *s1, const char *s2);
extern size_t   strlen(const char *s);
extern char *   strcpy(char *dest, const char *src);
/* termcap stuff */
extern int      tgetent (char *buffer, const char *termtype);
extern int      tgetnum (const char *name);
#else
extern char *   getenv();
extern void     mc();
#endif


/*
** NAME
**	mc -- multiple column filter
**
** SYNOPSIS
**	mc [-w width] [file ...]
**
** DESCRIPTION
**	mc reads text lines from {file} (default: standard input) and
**	writes them in multiple columns to the standard output, in
**	column-major order (the first input lines appear in the first
**	column, then move to the next and so on).  If multiple {file}s
**	are specified, then they are read in sequence.
**
** OPTIONS
**	-w	Output lines will contain no more that {width} characters
**		(default: 80, or the width of the display, if the output
**		is a terminal device).  The terminating newline is counted
**		as one character.
**
** EXAMPLES
**	To produce a multicolumn list, sorted by column, of all of the login
**	names on the system:
**
**		cut -d: -f1 /etc/passwd | sort | mc
**
**	To generate a multi-column list of the files in the current directory:
**
**		ls | mc
**
** FILES
**
** DIAGNOSTICS
**	{width}: bad width
**
**		The specified {width} is less than 8 columns.
**
**	{inwidth}-col input too wide for {outwidth}-col output
**
**		An input line is too wide for the specified (or default)
**		output width.
**
** RESTRICTIONS
**	mc is not an IPW program, but is provided with IPW for use in
**	shell scripts.
**
** FUTURE DIRECTIONS
**
** HISTORY
**	?/?/?	Written by Dan Ts'o, Dept. of Neurobiology, Rockefeller
**		University (modified for V7 and posted to USENET).
**	7/1/90	Added getopt(), made "-" option work, James Frew, UCSB.
**
** BUGS
**
** SEE ALSO
**	UNIX:  ls, pr
*/

#define	OPTSTRING	"w:"
#define	SYNOPSIS	"[-w width] [file ...]"

#define	DFLT_WIDTH	80
#define	MIN_WIDTH	8

#define	MEMINCR	1024L			/* Memory buffer increments	 */

static int      width;
static char    *ZSTACK1 = NULL;		/* An impossible (char *) (Sorry) */

static char   **
stack1(s)
	char           *s;
{
	static char   **s_beg;
	static char   **s_end;
	static long     nbuf = 0;

	char          **v;
	char          **u;
	long            n;

	if (s == ZSTACK1) {
		if (nbuf > 0) {
 /* NOSTRICT */
			free((char *) s_beg);
		}

		nbuf = 0;
		return (0);
	}

	if (nbuf == 0 || s_end <= s_beg) {
		n = MEMINCR * (nbuf + 1);
 /* NOSTRICT */
		v = (char **) malloc((unsigned) (n * sizeof(char *)));
		assert(v != NULL);

		s_beg = v;
 /* NOSTRICT */
		v += n;

		if (nbuf > 0) {
 /* NOSTRICT */
			u = s_end + (MEMINCR * nbuf);
			while (u > s_end) {
				*--v = *--u;
			}
 /* NOSTRICT */
			free((char *) s_end);
		}

		nbuf++;
		s_end = v;
	}

	*--s_end = s;
	return (s_end);
}

static void
usage(pgm)
	char           *pgm;
{
	(void) fflush(stdout);
	(void) fprintf(stderr, "Usage: %s %s\n", pgm, SYNOPSIS);
	exit(1);
}

void
main(argc, argv)
	int             argc;
	char           *argv[];

{
	int             opt;

	width = 0;

	while ((opt = getopt(argc, argv, OPTSTRING)) != EOF) {
		switch (opt) {

		case 'w':
			width = atoi(optarg);
			if (width < MIN_WIDTH) {
				(void) fprintf(stderr, "%d: bad width\n",
					       width);
				exit(1);
			}

			break;

		default:
			usage(argv[0]);
		}
	}

	if (width == 0) {
		char           *cp;
		char            tbuf[1024];

		cp = getenv("TERM");
		if (cp == NULL
		    || tgetent(tbuf, cp) <= 0
		    || (width = tgetnum("co")) < MIN_WIDTH) {
			width = DFLT_WIDTH;
		}
	}

	if (optind >= argc) {
		mc(stdin);
	}
	else {
		do {
			if (strcmp(argv[optind], "-") == 0) {
				mc(stdin);
			}
			else {
				FILE           *fpi;

				fpi = fopen(argv[optind], "r");
				if (fpi == NULL) {
					(void) fflush(stdout);
					perror(argv[optind]);
				}
				else {
					mc(fpi);
					(void) fclose(fpi);
				}
			}
		} while (++optind < argc);
	}

	exit(0);
}

void
mc(fin)
	FILE           *fin;
{
	char          **bot = NULL;
	int             col_p;
	int             columns;
	char           *cp;

	int             i;
	int             index;
	int             items;
	char            line[1024];
	int             max;
	int             row_p;
	int             rows;

	items = max = 0;

	while (fgets(line, sizeof line, fin) != NULL) {
		i = strlen(line) - 1;

		if (line[i] == '\n') {
			line[i] = 0;
		}
		else {
			++i;
		}

		if (i >= width) {
			(void) fprintf(stderr,
				 "%d-col input too wide for %d-col output\n",
				       i, width);
			exit(1);
		}

		cp = (char *) malloc((unsigned) (i + 1));
		assert(cp != NULL);

		(void) strcpy(cp, line);
		bot = stack1(cp);

		if (i > max) {
			max = i;
		}

		++items;
	}

	columns = width / (max + 1);
	rows = (items + columns - 1) / columns;

	for (row_p = 0; row_p < rows; row_p++) {
		for (col_p = 0; col_p < columns; col_p++) {
			index = (col_p * rows) + row_p;
			if (index >= items) {
				continue;
			}

			if ((col_p + 1) * rows + row_p >= items) {
				(void) printf("%s", bot[items - index - 1]);
			}
			else {
				(void) printf("%-*s ",
					      max, bot[items - index - 1]);
			}
		}

		(void) printf("\n");
	}

	(void) fflush(stdout);
	(void) stack1(ZSTACK1);
}

#ifndef	lint
static char     rcsid[] = "$Header: /home/ipw/src/etc/mc/RCS/mc.c,v 1.4 90/02/25 15:36:50 frew Exp $";

#endif
