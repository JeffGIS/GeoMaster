/* C code produced by gperf version 2.5 (GNU C++ version) */
/* Command-line: gperf -t -p -j1 -o -k 1,6,8,9,$ nameproj.gperf  */

/*
** NAME
**      nameproj -- return ID of projection for given name
** 
** SYNOPSIS
**      int nameproj (proj_name);
**      char *proj_name;
** 
** DESCRIPTION
** 
** RESTRICTIONS
** 
** RETURN VALUE
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

/*
 * Use: gperf -t -p -j1 -o -k 1,6,8,9,$ nameproj.gperf  > nameproj.c
 */

#include "ipw.h"
#include "mproj.h"

struct projection { char *name; int projid; };

#define TOTAL_KEYWORDS 81
#define MIN_WORD_LENGTH 3
#define MAX_WORD_LENGTH 38
#define MIN_HASH_VALUE 6
#define MAX_HASH_VALUE 153
/* maximum key range = 148, duplicates = 0 */

static unsigned int
hash (str, len)
     register char *str;
     register int unsigned len;
{
  static unsigned char asso_values[] =
    {
     154, 154, 154, 154, 154, 154, 154, 154, 154, 154,
     154, 154, 154, 154, 154, 154, 154, 154, 154, 154,
     154, 154, 154, 154, 154, 154, 154, 154, 154, 154,
     154, 154,   0, 154, 154, 154, 154, 154, 154, 154,
       7,   5, 154, 154, 154, 154, 154, 154, 154,   0,
     154, 154, 154, 154, 154, 154, 154, 154, 154, 154,
     154, 154, 154, 154, 154,   3, 154,  27, 154,  14,
     154,  41, 154,  11, 154, 154,  15,  56, 154,  47,
       8, 154, 154,  22,  28,  45,  53, 154, 154, 154,
     154, 154, 154, 154, 154,   1, 154,   0,  25,   1,
      13,   0, 154,  43,  42,   0, 154, 154,   0,  33,
       0,  37,  50,   1,   0,   7,   0,   0,  43, 154,
     154,   1, 154, 154, 154, 154, 154, 154,
    };
  register int hval = len;

  switch (hval)
    {
      default:
      case 9:
        hval += asso_values[str[8]];
      case 8:
        hval += asso_values[str[7]];
      case 7:
      case 6:
        hval += asso_values[str[5]];
      case 5:
      case 4:
      case 3:
      case 2:
      case 1:
        hval += asso_values[str[0]];
    }
  return hval + asso_values[str[len - 1]];
}

struct projection *
in_word_set (str, len)
     register char *str;
     register unsigned int len;
{
  static struct projection wordlist[] =
    {
      {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, 
      {"lamcc",  LAMCC},
      {"lambert",  LAMAZ},
      {"",}, 
      {"azimuthal",  AZMEQD},
      {"",}, 
      {"equidistant",  EQUIDC},
      {"Azimuthal",  AZMEQD},
      {"Polar",  PS},
      {"",}, 
      {"equirectangular",  EQRECT},
      {"",}, 
      {"lambert azimuthal",  LAMAZ},
      {"lambert conformal",  LAMCC},
      {"Perspective",  GVNSP},
      {"albers",  ALBERS},
      {"",}, 
      {"Lambert",  LAMAZ},
      {"Albers",  ALBERS},
      {"Azimuthal Equidistant",  AZMEQD},
      {"Equidistant",  EQUIDC},
      {"",}, {"",}, 
      {"Polar Stereographic",  PS},
      {"Equirectangular",  EQRECT},
      {"sinusiodal",  SNSOID},
      {"",}, 
      {"Equidistant Conic",  EQUIDC},
      {"State Plane",  SPCS},
      {"State_Plane",  SPCS},
      {"Lambert Azimuthal",  LAMAZ},
      {"utm",  UTM},
      {"(Hotine) Oblique Mercator",  HOM},
      {"",}, 
      {"miller",  MILLER},
      {"StatePlane",  SPCS},
      {"mercator",  MERCAT},
      {"Albers Equal Area",  ALBERS},
      {"som",  SOM},
      {"lat/lon",  GEO},
      {"Sinusiodal",  SNSOID},
      {"Lambert Azimuthal Equal Area",  LAMAZ},
      {"LamCC",  LAMCC},
      {"Grinten",  VGRINT},
      {"Equidistant Cylindrical Rectangular",  EQRECT},
      {"grinten",  VGRINT},
      {"Gnomonic",  GNOMON},
      {"State Plane Coordinates",  SPCS},
      {"gnomonic",  GNOMON},
      {"space oblique mercator",  SOM},
      {"polar",  PS},
      {"Polyconic",  POLYC},
      {"lambert azimuth",  LAMAZ},
      {"stereographic",  STEREO},
      {"Lambert Conformal",  LAMCC},
      {"transverse",  TM},
      {"perspective",  GVNSP},
      {"Miller",  MILLER},
      {"",}, 
      {"Mercator",  MERCAT},
      {"",}, 
      {"Lambert Conformal Conic",  LAMCC},
      {"",}, {"",}, 
      {"Space Oblique Mercator",  SOM},
      {"Space_Oblique_Mercator",  SOM},
      {"",}, 
      {"Grinten I",  VGRINT},
      {"Stereographic",  STEREO},
      {"Universal Transverse Mercator",  UTM},
      {"Lambert Azimuth",  LAMAZ},
      {"Lambert_Azimuth",  LAMAZ},
      {"",}, {"",}, 
      {"ortho",  ORTHO},
      {"Space Oblique Mercator (SOM)",  SOM},
      {"SOM",  SOM},
      {"",}, 
      {"geo",  GEO},
      {"Vertical Near Side Perspective",  GVNSP},
      {"Universal Transverse Mercator (UTM)",  UTM},
      {"oblique mercator",  HOM},
      {"",}, 
      {"Transverse",  TM},
      {"Ortho",  ORTHO},
      {"",}, {"",}, {"",}, {"",}, 
      {"Geographic",  GEO},
      {"",}, 
      {"geographic",  GEO},
      {"Transverse Mercator",  TM},
      {"polyconic",  POLYC},
      {"Albers Conical Equal Area",  ALBERS},
      {"",}, {"",}, 
      {"Miller Cylindrical",  MILLER},
      {"",}, 
      {"UTM",  UTM},
      {"",}, {"",}, {"",}, {"",}, 
      {"Van der Grinten",  VGRINT},
      {"",}, 
      {"Van der Grinten 1",  VGRINT},
      {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, 
      {"Oblique Mercator",  HOM},
      {"Oblique_Mercator",  HOM},
      {"",}, 
      {"Van der Grinten I",  VGRINT},
      {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, 
      {"General Vertical Near-Side Perspective",  GVNSP},
      {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, 
      {"",}, 
      {"orthographic",  ORTHO},
      {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, 
      {"Orthographic",  ORTHO},
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register char *s = wordlist[key].name;

          if (*s == *str && !strcmp (str + 1, s + 1))
            return &wordlist[key];
        }
    }
  return 0;
}

int nameproj(proj_name)
char *proj_name;
{
  struct projection *proj;
 
  proj = in_word_set (proj_name, strlen(proj_name) );
  if (proj == NULL)
    return (UNKNOWN);
 
  return (proj->projid);
}
