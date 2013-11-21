
/* C language version of JPL ephemeris installation test program.
   Edit file names, etc., below as appropriate.

   Steve Moshier
   moshier@na-net.ornl.gov */

#include <stdio.h>
#ifdef __STDC__
#include <stdlib.h>
#endif
#include <string.h>

FILE *f;

extern double emrat;		/*  = 81.300585; */
/* Conversion factors between degrees and radians */
double DTR = 1.7453292519943295769e-2;
double RTD = 5.7295779513082320877e1;
double RTS = 2.0626480624709635516e5;	/* arc seconds per radian */
double STR = 4.8481368110953599359e-6;	/* radians per arc second */
double PI = 3.14159265358979323846;
/* Standard epochs.  Note Julian epochs (J) are measured in
 * years of 365.25 days.
 */
double J2000 = 2451545.0;	/* 2000 January 1.5 */
double B1950 = 2433282.42345905;/* 1950 January 0.923 Besselian epoch */
double J1900 = 2415020.0;	/* 1900 January 0, 12h UT */

#include "kep.h"
#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG 0

/* Items referenced by kepjpl.c.  */
struct orbit earth;
struct orbit orb;
/* Ephemeris file name.  */
char defile[128] =
{0};
double jvearth;
double pearthb[3];
double vearth[3];
int objnum;
/* Referenced by lonlat.c.  */
int prtflg = 0;
/* Referenced by dms.c.  */
double TDT, UT;
int ephprint = 0;
FILE *ephfile = NULL;
#if DE406 | DE406CD
extern int defd;
#endif

int
main ()
{
  char s[128];
  int denum;
  char jcal[32];
  int t, c, x, i, j, k, m, didt, didc;
  double JD, value, err, y, ratio;
  double pobj[3], vobj[3];
  double tvec[6], cvec[6];

  f = NULL;
#if DE406CD
#ifdef __BORLANDC__
  f = fopen ("F:\TESTPO.406", "r");
#else
#ifdef _MSC_VER
#if _MSC_VER > 1000
  f = fopen ("F:\\TESTPO.406", "r");
#else
  f = fopen ("E:\\TESTPO.406", "r");
#endif
#else
  f = fopen ("/cdrom/testpo.406", "r");
#endif /* not _MSC_VER  */
#endif /* not __BORLANDC__ */
#endif /* not DE406CD */

#if DE406
  f = fopen ("/cdrom/testpo.406", "r");
#endif
#if DE404
  f = fopen ("testpo.404", "r");
#endif
#if DE200 | DE200CD
  f = fopen ("d:/tmp/testpo.200", "r");
#endif
  if (f == NULL)
    {
      printf ("can't find testpo.NNN");
      exit (1);
    }
#if DE406CD
#ifdef __BORLANDC__
  strcpy (defile, "F:\UNIX.406");
#else
#ifdef _MSC_VER
#if _MSC_VER > 1000
  strcpy (defile, "F:\\UNIX.406");
#else
  strcpy (defile, "E:\\UNIX.406");
#endif
#else
  strcpy (defile, "/cdrom/unix.406");
  /*  strcpy (defile, "/a/de/unix.406"); */
#endif /* not _MSC_VER */
#endif /* not __BORLANDC__ */
#endif  /* DE406CD */

#if DE406
  strcpy (defile, "/cdrom/unxm3000.406");
#endif
#if DE404
  strcpy (defile, "/a/de/de404.unx");
#endif
#if DE200 | DE200CD
  strcpy (defile, "d:/tmp/de200.unx");
#endif
  for (i = 0; i < 6; i++)
    fgets (s, 128, f);
  s[0] = '\n';
  m = 0;
  while (fgets (s, 128, f) != NULL)
    {
      didt = 0;
      didc = 0;
      if (s[0] == '\n')
	break;
      denum = 0;
      sscanf (s, "%d %s %lf %d %d %d %lf",
	      &denum, &jcal[0], &JD, &t, &c, &x, &value);
#if DE406 | DE406CD
      if (denum != 406)
	{
	  printf ("Skipping denum = %d\n", denum);
	  break;
	}
      if (JD < 625360.5)
	{
	  printf ("Skipping JD = %.15e\n", JD);
	  continue;
	}
#endif
#if DE404
      if (denum != 404)
	break;
      if (JD < 625296.5)
	continue;
#endif
#if DE200 | DE200CD
      if (denum != 200)
	break;
#endif
      if ((t == 10 && c == 3) || (t == 3 && c == 10))
	{
	  objnum = 10;
	  k = jpl (JD, objnum, pobj, vobj);
	  if (k)
	    printf ("jpl(%d) returned %d\n", objnum, k);
	  for (j = 0; j < 3; j++)
	    {
	      tvec[j] = pobj[j];
	      tvec[3 + j] = vobj[j];
	      cvec[j] = 0.0;
	      cvec[3 + j] = 0.0;
	    }
	  if (c == 10)
	    {
	      for (j = 0; j < 6; j++)
		{
		  tvec[j] = -tvec[j];
		}
	    }
	  didt = 1;
	  didc = 1;
	  goto czero;
	}
      if (t == 3 || c == 3)
	{
	  objnum = 3;
	  k = jpl (JD, objnum, pobj, vobj);
	  if (k)
	    printf ("jpl(%d) returned %d\n", objnum, k);
	  for (j = 0; j < 3; j++)
	    {
	      cvec[j] = pobj[j];
	      cvec[3 + j] = vobj[j];
	    }
	  objnum = 10;
	  k = jpl (JD, objnum, pobj, vobj);
	  if (k)
	    printf ("jpl(%d) returned %d\n", objnum, k);
	  ratio = 1.0 / (emrat + 1.0);
	  for (j = 0; j < 3; j++)
	    {
	      tvec[j] = pobj[j];
	      tvec[3 + j] = vobj[j];
	    }
	  for (i = 0; i < 6; i++)
	    {
	      cvec[i] = cvec[i] - ratio * tvec[i];
	    }
	  if (t == 3)
	    {
	      for (i = 0; i < 6; i++)
		{
		  tvec[i] = cvec[i];
		}
	      didt = 1;
	    }
	  else
	    didc = 1;
	}
      if (t == 10 || c == 10)
	{
	  objnum = 3;
	  k = jpl (JD, objnum, pobj, vobj);
	  if (k)
	    printf ("jpl(%d) returned %d\n", objnum, k);
	  for (j = 0; j < 3; j++)
	    {
	      cvec[j] = pobj[j];
	      cvec[3 + j] = vobj[j];
	    }
	  objnum = 10;
	  k = jpl (JD, objnum, pobj, vobj);
	  if (k)
	    printf ("jpl(%d) returned %d\n", objnum, k);
	  ratio = emrat / (emrat + 1.0);
	  for (j = 0; j < 3; j++)
	    {
	      tvec[j] = pobj[j];
	      tvec[3 + j] = vobj[j];
	    }
	  for (i = 0; i < 6; i++)
	    {
	      cvec[i] = cvec[i] + ratio * tvec[i];
	    }
	  if (t == 10)
	    {
	      for (i = 0; i < 6; i++)
		{
		  tvec[i] = cvec[i];
		}
	      didt = 1;
	    }
	  else
	    didc = 1;
	}

      if (didt)
	goto tzero;
      objnum = t;
      if (t == 13)
	objnum = 3;
      if (t == 12)
	{
	  for (j = 0; j < 6; j++)
	    tvec[j] = 0.0;
	  didt = 1;
	  goto tzero;
	}
#if DEBUG
      printf ("%d %.16e\n", objnum, JD);
#endif
      k = jpl (JD, objnum, pobj, vobj);
      if (k)
	printf ("jpl(%d) returned %d\n", objnum, k);
      for (j = 0; j < 3; j++)
	{
	  tvec[j] = pobj[j];
	  tvec[3 + j] = vobj[j];
	}
    tzero:
      if (didc)
	goto czero;
      objnum = c;
      if (c == 13)
	objnum = 3;
      if (c == 12)
	{
	  for (j = 0; j < 6; j++)
	    cvec[j] = 0.0;
	  didc = 1;
	  goto czero;
	}
      k = jpl (JD, objnum, pobj, vobj);
      if (k)
	printf ("jpl(%d) returned %d\n", objnum, k);
      for (j = 0; j < 3; j++)
	{
	  cvec[j] = pobj[j];
	  cvec[3 + j] = vobj[j];
	}
    czero:
      y = tvec[x - 1] - cvec[x - 1];
      err = value - y;
      if (err < 0)
	err = -err;
      if (err > 1.0e-13)
	{
	  printf ("%s", s);
	  printf ("%.16e\n", y);
	  printf ("%.16e\n\n", value);
	}
      s[0] = '\n';
      m += 1;
#if DEBUG
      if (m == 10)
	break;
#endif
    }
  printf ("%d tests done.\n", m);
  exit(0);
  return 0;
}
