#ifndef MOON_H
#define MOON_H

#include <glib.h>

gdouble Moon(gdouble T, gdouble * LAMBDA, gdouble * BETA, gdouble * R,
	     gdouble * AGE);
gdouble frac(gdouble x);
gdouble NewMoon(gdouble ax, gdouble bx, gdouble cx);
gint MiniMoon(gdouble T, gdouble * RA, gdouble * DEC);

#endif				/* MOON_H */
