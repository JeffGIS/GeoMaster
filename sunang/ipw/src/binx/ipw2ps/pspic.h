#ifndef	PSPIC_H
#define	PSPIC_H

#define	PAGE_HEIGHT	11		/* output page height		 */
#define	PAGE_WIDTH	8.5		/* output page width		 */

#define	IMG_HEIGHT	9.5		/* max. height of output image	 */
#define	IMG_WIDTH	7.0		/* max. width of output image	 */

#define	XTRANS_FUDGE	(1.0 / 16.0)	/* LaserWriter X bias		 */

#define	HI_NYBBLE(i)	( ((i) >> 4) & 0xF )
#define	LO_NYBBLE(i)	( (i) & 0xF )

#define	ITOP(in)	( (int)((in) * 72.0) )

extern void     pspic();
extern void     pspicx();

/* $Header: pspic.h,v 1.1 87/10/18 17:38:17 frew Exp $ */
#endif
