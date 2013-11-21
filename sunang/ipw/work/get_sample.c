
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   get_sample.c   4.3   7/9/90";
#endif

/*
** NAME
** 	get_sample -- return a sample vector of data for the trbxfr model.
** 
** SYNOPSIS
**	get_sample (inbuf, mbuf, samp, buf_index, vector, nbands);
**	fpixel_t *inbuf;
**	pixel_t *mbuf;
**	int samp;
**	int *buf_index;
**	double *vector;
**	int nbands;
** 
** DESCRIPTION
**	get_sample returns a vector of data from inbuf[buf_index] for nbands
**	bands.	If any of the bands are classified (CRH present), the
**	representative value for classified pixel is returned.  The buffer
**	index buf_index is updated and returned.
** 
** RESTRICTIONS
** 
** RETURN VALUE
**	Number of bands returned with data
** 
** GLOBALS ACCESSED
** 
** ERRORS
** 
** WARNINGS
** 
** APPLICATION USAGE
** 
** FUTURE DIRECTIONS
** 
** BUGS
**
*/

#include "ipw.h"
#include "crh.h"

#define NO_DATA		0

int
get_sample (fdi, inbuf, mbuf, samp, buf_index, vector, nbands)

	int		 fdi;		/* input image file desc	*/
	fpixel_t	*inbuf;		/* IPW line of data		*/
	pixel_t		*mbuf;		/* mask line of data		*/
	int		 samp;		/* sample index for mask	*/
	int		*buf_index;	/* index in input buffer	*/
	double		*vector;	/* output vector of data	*/
	int		 nbands;	/* # bands in vector		*/
{
	CRH_T          **crhpp;		/* -> CRH of input image	*/
	int		 index;		/* index of sample		*/
	int		 band;		/* band loop counter		*/
	int		 data_count;	/* # bands with data		*/


	index = *buf_index;
	*buf_index += nbands;
	data_count = 0;

   /* check mask for this pixel */

	if (mbuf != NULL) {
		if (mbuf[samp] == 0)
			return (0);
	}

   /* check for presence of CRH */

	crhpp = crh (fdi);

	for (band = 0; band < nbands; band++) {

		if (crhpp == NULL) {
			vector[band] = inbuf[index++];
			data_count++;

		} else if (crhpp[band] == NULL) {
			vector[band] = inbuf[index++];
			data_count++;

		} else { /* this band is classified */

			assert (inbuf[index] <= crh_nclass(crhpp[band]) &&
				inbuf[index] >= 0);
			if (inbuf[index] != NO_DATA) {
				vector[band] = crh_rep (crhpp[band],
							(int) inbuf[index++]-1);
				data_count++;
			}
		}
	}

	return (data_count);
}
