/*
 * loop thru sequence
 */

#include <stdio.h>
#include "ipw.h"

extern int	errno;

void
DEFUN( seqloop, (start, fin, inc, inv, fmt),
	REG_1 double	start		/* start of range	*/
   AND  REG_2 double	fin		/* end of range		*/
   AND  REG_3 double	inc		/* increment		*/
   AND  double		(*inv)()	/* -> inverse function	*/
   AND  CONST char     *fmt)		/* output format	*/
{
	if (fin > start) {

		if (inc <= 0) {
			error("increment negative or zero");
		}

		/*
		 * print out values
		 */

		if (inv != NULL) {
			errno = 0;
			while (start <= fin) {
				(void)printf(fmt, (*inv)(start));
				if (errno) {
					error("math function error");
				}
				start += inc;
				putchar('\n');
			}
		}

		else {
			while (start <= fin) {
				(void)printf(fmt, start);
				start += inc;
				putchar('\n');
			}
		}
	}

	else {
		if (inc >= 0) {
			error("increment positive or zero");
		}

		/*
		 * print out values
		 */

		if (inv != NULL) {
			errno = 0;
			while (start >= fin) {
				(void)printf(fmt, (*inv)(start));
				if (errno) {
					error("math function error");
				}
				start += inc;
				putchar('\n');
			}
		}
		else {
			while (start >= fin) {
				(void)printf(fmt, start);
				start += inc;
				putchar('\n');
			}
		}
	}
}
