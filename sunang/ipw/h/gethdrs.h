#ifndef GETHDRS_H
#define GETHDRS_H

#include "hdrio.h"

typedef addr_t  EXFUN( (*ingest_t), (int efd) );

typedef struct {
	char           *name;		/* name of header to get/skip	 */
	ingest_t        ingest;		/* -> ingest function		 */
	addr_t          hdr;		/* -> ingested header		 */
} GETHDR_T;

#define	got_hdr(g)	( (g).hdr != NULL )
#define	hdr_addr(g)	( (g).hdr )

/*
 * if NO_COPY is supplied as the nbands argument to gethdrs(), then gethdrs()
 * will not copy ANY header data to the output file.
 */
#define	NO_COPY		NO_BAND

extern void     EXFUN(copyhdrs, (int fdi, int nbands, int fdo));
extern void     EXFUN(gethdrs, (int fdi, GETHDR_T **request, int nbands,
			 int fdo));
extern void     EXFUN(skiphdrs, (int fd));

/* $Header: gethdrs.h,v 1.6 87/11/03 09:37:11 frew Exp $ */
#endif
