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

#ifndef	VIEW_H
#define	VIEW_H

#include "ipw.h"
#include "fpio.h"
#include "lqh.h"

/*
 * header file for "viewcalc" program
 */

typedef struct {
	int             i_fds;		/* input S/A file desc	 */
	int             i_fdh;		/* input horizon file desc */
	int             o_fd;		/* output image file desc */
	float          *cstbl;		/* cosines of slopes	 */
	float          *hazm;		/* horizon azimuths	 */
	float         **cosdtbl;	/* cos(phi - A)		 */
	float         **hdtbl;		/* H - sinH cos H	 */
	float         **sh2tbl;		/* sin^2 (H)		 */
	fpixel_t       *obuf;		/* output buffer	 */
	fpixel_t       *sstbl;		/* sines of slopes	 */
	fpixel_t      **chtbl;		/* cosines of horizons	 */
	pixel_t        *hbuf;		/* input horizon buff	 */
	pixel_t        *sbuf;		/* input slope/azm buff */
}               PARM_T;

extern PARM_T   parm;

extern void     EXFUN(buffers,(NOARGS));
extern void     EXFUN(headers,(NOARGS));
extern LQH_T  **EXFUN(newlqh,(int fdi, int fdo));
extern void     EXFUN(trigtbl,(NOARGS));
extern void     EXFUN(viewf,(NOARGS));

/* $Header: /local/share/pkg/ipw/src/bin/viewcalc/RCS/view.h,v 1.2 90/11/11 17:10:27 frew Exp $ */
#endif
