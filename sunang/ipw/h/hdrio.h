#ifndef	HDRIO_H
#define	HDRIO_H

/*
 * header file for IPW image header I/O subsystem
 */

#define	BOIMAGE		"image"		/* name for image data preamble	 */
#define	HGOT_DATA	1		/* got a header data record	 */
#define	HGOT_PRMB	0		/* got a header preamble record	 */
/*
 * NB: setting HREC_MAX to MAX_CHAR is kind of a shot in the dark; at least it
 *     guarantees that no header record will be generated that couldn't be
 *     entered by a human at the keyboard.  The real limit is the host's
 *     maximum string length.
 */
#define	HREC_MAX	MAX_CHAR	/* max hdr rec size (incl '\n')	 */
#define	HREC_SIZ	(HREC_MAX + 1)	/* (backward compatibility)	 */
#define	NO_BAND		(-1)
#define	hwid		hwprmb		/* (backward compatibility)	 */

/*
 * '\f' at end of version string will stop "more" before image data
 */
#define	boimage(fd)	hwprmb(fd, BOIMAGE, NO_BAND, "$Revision: 1.5 $\f")

extern int       EXFUN(hcopy, (int fdi, int fdo));
extern int       EXFUN(hgetrec, (int fd, char *comment, char *key, 
                                 char *value));
extern int       EXFUN(hpass, (int fdi, int fdo));
extern int       EXFUN(hputrec, (int fd, CONST char *comment, CONST char *key,
                                 CONST char *value));
extern int       EXFUN(hrband, (int fd));
extern char    * EXFUN(hrname, (int fd));
extern int       EXFUN(hrskip, (int fdi));
extern char    * EXFUN(hrvers, (int fd));
extern int       EXFUN(hwprmb, (int fd, CONST char *name, int band, 
                                CONST char *version));

/* $Header: hdrio.h,v 1.4 87/10/12 10:55:31 frew Exp $ */
#endif
