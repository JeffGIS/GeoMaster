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

#ifndef	PGM_H
#define	PGM_H

#include "ipw.h"

#include "geoh.h"
#include "orh.h"
#include "winh.h"

/*
 * header file for "flip" program
 */

typedef struct {
	int             i_fd;		/* input image file descriptor	 */
	int             o_fd;		/* output image file descriptor	 */
	bool_t          lines;		/* ? flip lines			 */
	bool_t          samps;		/* ? flip samples		 */
}               PARM_T;

extern PARM_T   parm;

extern GEOH_T **fixgeoh();
extern ORH_T  **fixorh();
extern WINH_T **fixwinh();
extern void     flip();
extern void     flip1d();
extern void     flip2d();
extern void     headers();
extern void     reverse();
extern void     revgen();
extern void     revchar();
extern void     revlong();
extern void     revshort();

/* $Header: /local/share/pkg/ipw/src/bin/flip/RCS/pgm.h,v 1.2 90/11/11 17:02:46 frew Exp $ */
#endif
