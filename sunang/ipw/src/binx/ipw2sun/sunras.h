#ifndef	SUNRAS_H
#define	SUNRAS_H

#define	NBITS_ROUND	16

#define START_BIT	bit(CHAR_BIT - 1)
#define NEXT_BIT(i)	( (i) >>= 1 )

#define	is_set(i)	( ((i) & 1) == 0 )

extern void     bitmap();
extern int      sunhdr();
extern void     sunras();

/* $Header: sunras.h,v 1.1 87/10/28 16:38:13 frew Exp $ */
#endif
