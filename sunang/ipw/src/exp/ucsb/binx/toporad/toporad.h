#ifndef TOPO_H
#define TOPO_H

 /*
 * band #'s for quantities in input file
 */

#define BEAM_BAND	0
#define DIFFUSE_BAND	1
#define MU_BAND		2
#define VF_BAND		3
#define CT_BAND		4
#define ALB_BAND	5
#define NBANDS		6

extern int head_init();
extern int head_final();
extern int radcalc();

/* $Header: /usr/home/dozier/ipw/src/bin/toporad/RCS/toporad.h,v 1.1 89/07/05 13:25:28 dozier Exp $ */
#endif
