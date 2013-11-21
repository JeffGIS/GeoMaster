/*
**  NAME
**	zerobr - finds zero of a function by Brent's algorithm
**
**  SYNOPSIS
**	double	zerobr (a, b, t, f)
**	double	a, b, t, (*f)();
**
**  DESCRIPTION
**	Returns zero of a function by Brent's algorithm (R. Brent,
**	Algorithms for Minimization without Derivatives).
**
**	a,b	spanning guesses for root
**	t	tolerable relative error
**	f	pointer to function
**
**  DIAGNOSTICS
**	sets errno and writes message via usrerr().
**	(the function f is also assumed to do this, so zerobr checks errno)
*/

#include <math.h>
#include <errno.h>
#include "ipw.h"
#include "sunlight.h"

double
DEFUN( zerobr, (a, b, t, f),
	double         a
   AND  double         b
   AND  double         t
   AND  double         (*f) ())
{
	double          meps;
	int             maxfun;
	double         c;
	double         d;
	double         e;
	double         fa;
	double         fb;
	double         fc;
	double         m;
	double         p;
	double         q;
	double         r;
	double         s;
	double         tol;

	meps = DBL_EPSILON;
	errno = 0;

 /*
  * compute max number of function evaluations
  */
	if (a == b) {
		usrerr("a = b = %g", a);
		errno = ERROR;
		return (a);
	}

	fb = (fabs(b) >= fabs(a)) ? fabs(b) : fabs(a);
	tol = 5.e-1 * t + 2 * meps * fb;
	s = log(fabs(b - a) / tol) / log(2.);
	maxfun = s * s + 1;

	fa = (*f) (a);
	fc = fb = (*f) (b);
	if (errno) {
		usrerr("bad function return");
		return (0.);
	}

	if (fabs(fb) <= tol)
		return (b);
	if (fabs(fa) <= tol)
		return (a);

	if (fb * fa > 0) {
		usrerr("root not spanned:\n\ta %g, b %g, f(a) %g, f(b) %g",
		       a, b, fa, fb);
		errno = ERROR;
		return (0.);
	}

	while (maxfun--) {

		if ((fb > 0 && fc > 0) || (fb <= 0 && fc <= 0)) {
			c = a;
			fc = fa;
			d = e = b - a;
		}

		if (fabs(fc) < fabs(fb)) {
			a = b;
			b = c;
			c = a;
			fa = fb;
			fb = fc;
			fc = fa;
		}

		tol = meps * fabs(b) + t;
		m = (c - b) / 2;

		if (fabs(m) < tol || fb == 0)
			return (b);
 /*
  * see if bisection is forced
  */
		if (fabs(e) < tol || fabs(fa) <= fabs(fb))
			d = e = m;

		else {
			s = fb / fa;

			if (a == c) {	/* linear interpolation */
				p = 2 * m * s;
				q = 1 - s;
			}

 /*
  * inverse quadratic interpolation
  */
			else {
				q = fa / fc;
				r = fb / fc;
				p = s * (2 * m * q * (q - r) -
					 (b - a) * (r - 1));
				q -= 1;
				q *= (r - 1) * (s - 1);
			}

			if (p > 0) {
				q = -q;
			}
			else {
				p = -p;
			}
			s = e;
			e = d;

			if (2 * p < 3 * m * q - fabs(tol * q) &&
			    p < fabs(s * q / 2)) {
				d = p / q;
			}
			else {
				d = e = m;
			}
		}

		a = b;
		fa = fb;

		if (fabs(d) > tol) {
			b += d;
		}
		else if (m > 0) {
			b += tol;
		}
		else {
			b -= tol;
		}

		fb = (*f) (b);
		if (errno) {
			return (0.);
		}
	}
	usrerr("did not converge");

	errno = ERROR;
	return (0.);
}
