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

#ifndef	BITCOM_H
#define	BITCOM_H

#define	BITOP_AND	'a'
#define	BITOP_OR	'o'
#define	BITOP_XOR	'x'

extern void	EXFUN(bitcom, (int i_fd, bool_t mflag, void (*op)(), int o_fd));
extern void	EXFUN(bitio, (int i_fd, int nlines, int nsamps, int nbands,
			bool_t mflag, void (*op)(), int o_fd));
extern void	EXFUN(bx_and, (pixel_t *buf, int nsamps, int nbands));
extern void	EXFUN(bx_or, (pixel_t *buf, int nsamps, int nbands));
extern void	EXFUN(bx_xor, (pixel_t *buf, int nsamps, int nbands));
extern void	EXFUN(m_expand, (pixel_t *buf, int nsamps, int nbands));

/* $Header: /local/share/pkg/ipw/src/bin/bitcom/RCS/bitcom.h,v 1.3 90/11/11 17:00:29 frew Exp $ */
#endif
