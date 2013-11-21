
/* Sun - object - earth angles and distances.
 * q (object), e (earth), and p (q minus e) are input vectors.
 * The answers are posted in the following global locations:
 */

double SE = 0.0;	/* earth-sun distance */
double SO = 0.0;	/* object-sun distance */
double EO = 0.0;	/* object-earth distance */

double pq = 0.0;	/* cosine of sun-object-earth angle */
double ep = 0.0;	/* -cosine of sun-earth-object angle */
double qe = 0.0;	/* cosine of earth-sun-object angle */

#include "kep.h"


int angles( p, q, e )
double p[], q[], e[];
{
double a, b, s;
int i;

EO = 0.0;
SE = 0.0;
SO = 0.0;
pq = 0.0;
ep = 0.0;
qe = 0.0;
for( i=0; i<3; i++ )
	{
	a = e[i];
	b = q[i];
	s = p[i];
	EO += s * s;
	SE += a * a;
	SO += b * b;
	pq += s * b;
	ep += a * s;
	qe += b * a;
	}
EO = sqrt(EO); /* Distance between Earth and object */
SO = sqrt(SO); /* Sun - object */
SE = sqrt(SE); /* Sun - earth */
/* Avoid fatality: if object equals sun, SO is zero.  */
if( SO > 1.0e-12 )
	{
	pq /= EO*SO;	/* cosine of sun-object-earth */
	qe /= SO*SE;	/* cosine of earth-sun-object */
	}
ep /= SE*EO;	/* -cosine of sun-earth-object */
return(0);
}

