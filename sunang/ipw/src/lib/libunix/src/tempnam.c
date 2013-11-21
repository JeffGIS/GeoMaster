/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the Computer Systems Laboratory, University of
 * California, Santa Barbara and its contributors'' in the documentation
 * or other materials provided with the distribution and in all
 * advertising materials mentioning features or use of this software.
 *
 * Neither the name of the University nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific written prior permission. This software
 * is provided ``as is'' without express or implied warranty.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)tmpnam.c	4.4 (Berkeley) 6/8/88";
#endif /* LIBC_SCCS and not lint */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <stdio.h>

#define	P_tmpdir	"/usr/tmp"

#if 0
FILE *
tmpfile()
{
	FILE *fp;
	char *f, name[MAXPATHLEN], *tmpnam();

	if (!(fp = fopen(f = tmpnam(name), "w+"))) {
		fprintf(stderr, "tmpfile: cannot open %s.\n", name);
		return(NULL);
	}
	(void)unlink(f);
	return(fp);
}

char *
tmpnam(s)
	char *s;
{
	static char name[MAXPATHLEN];
	char *mktemp();

	if (!s)
		s = name;
	(void)sprintf(s, "%s/XXXXXX", P_tmpdir);
	return(mktemp(s));
}
#endif

char *
tempnam(dir, pfx)
	char *dir, *pfx;
{
	struct stat buf;
	char *f, *name, *getenv(), *malloc(), *mktemp(), *strcat(), *strcpy();

	if (!(name = malloc((u_int)MAXPATHLEN)))
		return(NULL);
	if ((f = getenv("TMPDIR")) && !stat(f, &buf) &&
	    (buf.st_mode&S_IFMT) == S_IFDIR && !access(f, W_OK|X_OK)) {
		(void)strcpy(name, f);
		goto done;
	}
	if (dir && !stat(dir, &buf) &&
	    (buf.st_mode&S_IFMT) == S_IFDIR && !access(dir, W_OK|X_OK)) {
		(void)strcpy(name, dir);
		goto done;
	}
	if (!stat(P_tmpdir, &buf) &&
	    (buf.st_mode&S_IFMT) == S_IFDIR && !access(P_tmpdir, W_OK|X_OK)) {
		(void)strcpy(name, P_tmpdir);
		goto done;
	}
	if (!stat("/tmp", &buf) &&
	    (buf.st_mode&S_IFMT) == S_IFDIR && !access("/tmp", W_OK|X_OK)) {
		(void)strcpy(name, "/tmp");
		goto done;
	}
	return(NULL);
done:	(void)strcat(name, "/");
	if (pfx)
		(void)strcat(name, pfx);
	(void)strcat(name, "XXXXXX");
	return(mktemp(name));
}

#ifndef	lint
static char     rcsid[] = "$Id: tempnam.c,v 1.2 90/11/11 17:20:38 frew Exp $";

#endif
