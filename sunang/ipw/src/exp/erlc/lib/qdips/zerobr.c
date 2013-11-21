 
#include <math.h>
#include "ipw.h"
#include "erlc.h"
#include "qmacs.h"
 
/********************************************************************
** NAME
**	zerobr - finds zero of a function by Brent's algorithm
**
** SYNOPSIS
**	double	zerobr (a, b, t, f)
**	double	a;
**	double	b;
**	double	t;
**	double	(*f)();
**
** DESCRIPTION
**	Returns zero of a function by Brent's algorithm (R. Brent,
**	Algorithms for Minimization without Derivatives).
**
**	a,b	spanning guesses for root
**	t	tolerable relative error
**	f	pointer to function
**
** RESTRICTIONS
**
** RETURN VALUE
**      vapor pressure for success, ERROR for failure
**
** GLOBALS ACCESSED
**
** ERRORS
**	Terminages on error with message
**	(the function f is also assumed to do this)
**
** WARNINGS
**
** APPLICATION USAGE
**
** HISTORY
**	July 1982:	Written by J. Dozier, Department of Geography, UCSB
**      10/21/92:	Converted to IPW by D. Marks, USGS, EPA, ERL/C;
**
** FUTURE DIRECTIONS
**
** BUGS
*/
 
#define TYPE	double

TYPE
DEFUN( zerobr, (a, b, t, f),
	TYPE	a
   AND  TYPE	b
   AND  TYPE	t
   AND  TYPE	(*f)())
{
	TYPE	c;
	TYPE	d;
	TYPE	e;
	TYPE	fa;
	TYPE	fb;
	TYPE	fc;
	TYPE	m;
	TYPE	p;
	TYPE	q;
	TYPE	r;
	TYPE	s;
	TYPE	tol;
	char	str[80];
	double	meps;
	int	maxfun;

	meps = DBL_EPSILON;
	errno = 0;

	/* compute max number of function evaluations */
	if (a == b) {
		usrerr("a = b = %g", a);
		errno = ERROR;
		return(a);
	}

	fb = (ABS(b) >= ABS(a)) ? ABS(b) : ABS(a);
	tol = 5.e-1 * t + 2 * meps * fb;
	s = log(ABS(b - a) / tol) / log(2.);
	maxfun = s * s + 1;

	fa = (*f)(a);
	fc = fb = (*f)(b);
	if (errno) {
		return(0.);
	}

	if (ABS(fb) <= tol)
		return(b);
	if (ABS(fa) <= tol)
		return(a);

	if (fb*fa > 0) {
		sprintf (str,"root not spanned:\n\ta %g, b %g, f(a) %g, f(b) %g",
		a,b,fa,fb);
		usrerr("%s", str);
		errno = ERROR;
		return(0.);
	}

	while (maxfun--) {

		if ((fb > 0 && fc > 0)  ||  (fb <= 0  && fc <= 0))  {
			c = a; 
			fc = fa; 
			d = e = b - a;
		}

		if (ABS(fc) < ABS(fb)) {
			a = b; 
			b = c; 
			c = a;
			fa = fb; 
			fb = fc; 
			fc = fa;
		}

		tol = meps * ABS(b) + t;
		m = (c - b) / 2;

		if (ABS(m) < tol  ||  fb == 0)
			return(b);

		/* see if bisection is forced */
		if (ABS(e) < tol  ||  ABS(fa) <= ABS(fb))
			d = e = m;

		else {
			s = fb/fa;

			if (a == c) {	/* linear interpolation */
				p = 2 * m * s;
				q = 1 - s;
			}

			else {		/* inverse quadratic interpolation */
				q = fa/fc;
				r = fb/fc;
				p = s * (2 * m * q * (q-r) - (b-a) * (r-1));
				q -= 1;
				q *= (r-1) * (s-1);
			}

			if (p > 0)
				q = -q;
			else
				p = -p;

			s = e;
			e = d;

			if (2*p < 3*m*q - ABS(tol*q) && p < ABS(s*q/2))
				d = p/q;
			else
				d = e = m;
		}

		a = b; 
		fa = fb;

		if (ABS(d) > tol)
			b += d;
		else if (m > 0)
			b += tol;
		else
			b -= tol;

		fb = (*f)(b);
		if (errno) {
			return(0.);
		}
	}
	usrerr("%s","did not converge");

	errno = ERROR;
	return(0.);
}
