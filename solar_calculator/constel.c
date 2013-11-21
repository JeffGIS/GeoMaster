/* Constellation names
 */

#include <stdio.h>
#if __STDC__
static int islow (char *);
static int isup (char *);
static int isnumber (char *);
static int skipwh(char *);
double sqrt (double);
double sin (double);
double cos (double);
double atan2 (double, double);
double asin (double);
int precess (double *, double, int);
#else
static int islow(), isup(), isnumber(), skipwh();
double sqrt(), sin(), cos(), atan2(), asin();
int precess();
#endif

#define  RTD 57.295779513082320877

#define NCON 89
char *constel[NCON] = {
"And Andromedae",
"Ant Antliae",
"Aps Apodis",
"Aql Aquilae",
"Aqr Aquarii",
"Ari Arietis",
"Ara Arae",
"Aur Aurigae",
"Boo Bootis",
"Cae Caeli",
"Cam Camelopardalis",
"Can Cancri",		/* also abbreviated Cnc */
"Cap Capricorni",
"Car Carinae",
"Cas Cassiopeiae",
"Cen Centauri",
"Cep Cephei",
"Cet Ceti",
"Cha Chamaeleontis",
"Cir Circini",
"CMa Canis Majoris",
"CMi Canis Minoris",
"Cnc Cancri",
"Col Columbae",
"Com Comae Berenices",
"CrA Coronae Austrinae",
"CrB Coronae Borealis",
"Crt Crateris",
"Cru Crucis",
"Crv Corvi",
"CVn Canum Venaticorum",
"Cyg Cygni",
"Del Delphini",
"Dor Doradus",
"Dra Draconis",
"Equ Equulei",
"Eri Eridani",
"For Fornacis",
"Gem Geminorum",
"Gru Gruis",
"Her Herculis",
"Hor Horologii",
"Hya Hydrae",
"Hyi Hydri",
"Ind Indi",
"Lac Lacertae",
"Leo Leonis",
"Lep Leporis",
"Lib Librae",
"LMi Leonis Minoris",
"Lup Lupi",
"Lyn Lyncis",
"Lyr Lyrae",
"Men Mensae",
"Mic Microscopii",
"Mon Monocerotis",
"Mus Muscae",
"Nor Normae",
"Oct Octantis",
"Oph Ophiuchi",
"Ori Orionis",
"Pav Pavonis",
"Peg Pegasi",
"Per Persei",
"Phe Phoenicis",
"Pic Pictoris",
"PsA Piscis Austrini",
"Psc Piscium",
"Pup Puppis",
"Pyx Pyxidis",
"Ret Reticuli",
"Scl Sculptoris",
"Sco Scorpii",
"Sct Scuti",
"Ser Serpentis",
"Sex Sextantis",
"Sge Sagittae",
"Sgr Sagittarii",
"Tau Tauri",
"Tel Telescopii",
"TrA Trianguli Australis",
"Tri Trianguli",
"Tuc Tucanae",
"UMa Ursae Majoris",
"UMi Ursae Minoris",
"Vel Velorum",
"Vir Virginis",
"Vol Volantis",
"Vul Vulpeculae",
};



/* Greek letters
 */

#define NGREEK 24
char *greek[NGREEK] = {
"alpha",
"beta",
"gamma",
"delta",
"epsilon",
"zeta",
"eta",
"theta",
"iota",
"kappa",
"lambda",
"mu",
"nu",
"xi",
"omicron",
"pi",
"rho",
"sigma",
"tau",
"upsilon",
"phi",
"chi",
"psi",
"omega",
};

int showcname( in )
char *in;
{
char *g, *p, *q;
char ans[80];
int i;


p = in;
q = ans;


skipwh(p);
if( isnumber(p) )
	{
	while( isnumber(p) )
		*q++ = *p++;
	}
skipwh(p);
*q++ = ' ';

if( islow(p) )
	{
	for( i=0; i<NGREEK; i++ )
		{
		g = greek[i];
		if( (*p == *g) && (*(p+1) == *(g+1)) )
			break;
		}
	if( i < NGREEK )
		{
		while( *g != '\0' )
			*q++ = *g++;
		}
	while( islow(p) )
		++p;
	}
skipwh(p);
/* copy things like "-a" until uppercase letter found */
while( (*p != '\0') && !isup(p) )
	*q++ = *p++;

*q++ = ' ';

if( isup(p) )
	{
/* Check the list of constellation names */
	for( i=0; i<NCON; i++ )
		{
		g = constel[i];
		if( (*p == *g) && ( *(p+1) == *(g+1) )
			&& ( *(p+2) == *(g+2) ) )
			break;
		}
/* Get the name found */
	if( i < NCON )
		{
		g += 4;
		while( *g != '\0' )
			*q++ = *g++;
		p += 3;
		}
	}
skipwh(p);
*q++ = ' ';
while( *p )
	*q++ = *p++;
*q++ = '\0';
/* convert all '_' characters to spaces */
q = ans;
while( *q != '\0' )
	{
	if( *q == '_' )
		*q = ' ';
	++q;
	}
printf( "\n              %s\n", ans );
return(0);
}




static int islow(p)
char *p;
{
if( (*p >= 'a') && (*p <= 'z') )
	return(1);
else
	return(0);
}



static int isup(p)
char *p;
{
if( (*p >= 'A') && (*p <= 'Z') )
	return(1);
else
	return(0);
}




static int isnumber(p)
char *p;
{
if( (*p >= '0') && (*p <= '9') )
	return(1);
else
	return(0);
}



static int skipwh(p)
char *p;
{
while( ((*p == ' ') || (*p == '\t') || (*p == '_'))
	&& (*p != '\0') && (*p != '\n') && (*p != '\r') )
		++p;
return(0);
}


/* Table of constellation boundaries.

    Roman, Nancy Grace, "Identification of a Constellation from a Position"
    Pub. Astron. Soc. Pac. 99, 695, (1987)

   Array items are
   Lower Right Ascension, Upper Right Ascension,
     both in units of hours times 3600;
   Lower Declination, in units of degrees times 3600;
   and array index of constellation name.  */

#define NBNDRIES 357
long bndries[4*NBNDRIES] = {
    0L,  86400L, 316800L,  84L,
 28800L,  52200L, 311400L,  84L,
 75600L,  82800L, 310200L,  84L,
 64800L,  75600L, 309600L,  84L,
     0L,  28800L, 306000L,  16L,
 33000L,  38400L, 295200L,  10L,
     0L,  18000L, 288000L,  16L,
 38400L,  52200L, 288000L,  10L,
 63000L,  64800L, 288000L,  84L,
 72600L,  75600L, 288000L,  34L,
     0L,  12630L, 277200L,  16L,
 41400L,  48900L, 277200L,  10L,
 59520L,  63000L, 270000L,  84L,
 72600L,  74400L, 270000L,  16L,
 28680L,  33000L, 264600L,  10L,
 33000L,  40800L, 264600L,  34L,
 46800L,  59520L, 252000L,  84L,
 11160L,  12300L, 244800L,  14L,
 73500L,  74400L, 241200L,  34L,
 40800L,  43200L, 239400L,  34L,
     0L,   1200L, 237600L,  16L,
 50400L,  56400L, 237600L,  84L,
 84900L,  86400L, 237600L,  16L,
 43200L,  48600L, 230400L,  34L,
 48600L,  51900L, 226800L,  34L,
 83400L,  84900L, 226800L,  16L,
 21960L,  25200L, 223200L,  10L,
 72000L,  73500L, 221400L,  34L,
 73932L,  74160L, 219300L,  16L,
 25200L,  28680L, 216000L,  10L,
 28680L,  30300L, 216000L,  83L,
 71160L,  72000L, 214200L,  34L,
 72000L,  73932L, 214200L,  16L,
 82320L,  83400L, 212700L,  16L,
     0L,   8760L, 210600L,  14L,
 69900L,  71160L, 208800L,  34L,
  6120L,   6870L, 207000L,  14L,
  8760L,  11160L, 205200L,  14L,
 11160L,  11400L, 205200L,  10L,
 80340L,  82320L, 202500L,  16L,
 18000L,  21960L, 201600L,  10L,
 50520L,  51900L, 199800L,  83L,
 51900L,  69900L, 199800L,  34L,
 11400L,  12000L, 198000L,  10L,
 79680L,  80340L, 198000L,  16L,
 74160L,  79080L, 197400L,  16L,
     0L,   6120L, 194400L,  14L,
 21960L,  23400L, 194400L,  51L,
 43500L,  48600L, 190800L,  83L,
 54900L,  56700L, 190800L,  34L,
 79080L,  79680L, 189900L,  16L,
 12000L,  18000L, 189000L,  10L,
 82320L,  84000L, 189000L,  14L,
 56700L,  61200L, 185400L,  34L,
  7350L,   9060L, 181800L,  63L,
 61200L,  65640L, 181800L,  34L,
     0L,   4920L, 180000L,  14L,
  4920L,   6000L, 180000L,  63L,
 23400L,  24480L, 180000L,  51L,
 84000L,  86400L, 180000L,  14L,
 48600L,  50520L, 174600L,  83L,
     0L,   4020L, 172800L,  14L,
 84900L,  86400L, 172800L,  14L,
 65430L,  65640L, 171000L,  40L,
 65640L,  68700L, 171000L,  34L,
 68700L,  69000L, 171000L,  31L,
  6000L,   7350L, 169200L,  63L,
 30300L,  33000L, 169200L,  83L,
   600L,   3120L, 165600L,  14L,
 43200L,  43500L, 162000L,  83L,
 24480L,  26520L, 160200L,  51L,
 78870L,  79080L, 158400L,  31L,
 78750L,  78870L, 157500L,  31L,
 69000L,  69840L, 156600L,  31L,
 33000L,  36600L, 151200L,  83L,
 36600L,  38820L, 144000L,  83L,
 55560L,  56700L, 144000L,   8L,
 56700L,  58800L, 144000L,  40L,
 33300L,  34500L, 143100L,  51L,
     0L,   9060L, 132300L,   0L,
  9060L,   9240L, 132300L,  63L,
 69690L,  69840L, 131400L,  52L,
 16200L,  16890L, 129600L,  63L,
 78240L,  78750L, 129600L,  31L,
 78750L,  79200L, 129600L,  45L,
 23520L,  26520L, 127800L,   7L,
 26520L,  27900L, 127800L,  51L,
     0L,   7200L, 126000L,   0L,
 79200L,  82140L, 126000L,  45L,
 82140L,  82320L, 124200L,  45L,
 82320L,  84600L, 124200L,   0L,
  9240L,   9780L, 122400L,  63L,
 38820L,  39600L, 122400L,  83L,
 43200L,  44400L, 122400L,  30L,
 27900L,  33300L, 120600L,  51L,
 33300L,  35580L, 120600L,  49L,
  2580L,   5070L, 118800L,   0L,
 54660L,  55560L, 118800L,   8L,
 84600L,  85500L, 115500L,   0L,
 44400L,  47700L, 115200L,  30L,
 85500L,  86400L, 112800L,   0L,
 50250L,  50520L, 110700L,  30L,
  8700L,   9780L, 110400L,  81L,
  9780L,  16200L, 110400L,  63L,
 16200L,  17100L, 108000L,   7L,
 65430L,  69690L, 108000L,  52L,
 39600L,  43200L, 104400L,  83L,
 70800L,  75300L, 104400L,  31L,
 17100L,  21180L, 102600L,   7L,
 35580L,  37800L, 102600L,  49L,
 47700L,  50250L, 102600L,  30L,
     0L,    240L, 100800L,   0L,
  5070L,   6000L, 100800L,  81L,
 21180L,  23520L, 100800L,   7L,
 28380L,  28800L, 100800L,  38L,
 75300L,  78240L, 100800L,  31L,
 69330L,  70800L,  99000L,  31L,
  6900L,   8700L,  98100L,  81L,
 58200L,  58800L,  97200L,  26L,
 54300L,  54660L,  93600L,   8L,
 54660L,  58200L,  93600L,  26L,
 66120L,  67920L,  93600L,  52L,
 38700L,  39600L,  91800L,  49L,
 67920L,  69330L,  91800L,  52L,
  6000L,   6900L,  90000L,  81L,
  2580L,   3060L,  85500L,  67L,
 37800L,  38700L,  84600L,  49L,
 76500L,  77100L,  84600L,  88L,
 20520L,  21180L,  82200L,  78L,
   240L,    510L,  79200L,   0L,
 57300L,  57720L,  79200L,  74L,
 21180L,  22380L,  77400L,  38L,
 71400L,  72900L,  76500L,  88L,
 67920L,  69300L,  75900L,  88L,
   510L,   3060L,  75600L,   0L,
 72900L,  74040L,  73800L,  88L,
 28110L,  28380L,  72000L,  38L,
 74040L,  76500L,  70200L,  88L,
 69300L,  71400L,  69000L,  88L,
 11820L,  12120L,  68400L,   5L,
 67920L,  68400L,  66600L,  76L,
 20520L,  20760L,  64800L,  60L,
 22380L,  22710L,  63000L,  38L,
 68400L,  71400L,  58200L,  76L,
 17880L,  19200L,  57600L,  78L,
 57300L,  57900L,  57600L,  40L,
 71400L,  72900L,  56700L,  76L,
 16620L,  17880L,  55800L,  78L,
 19200L,  20160L,  55800L,  78L,
 46200L,  48600L,  54000L,  24L,
 62100L,  65700L,  51600L,  40L,
 42720L,  46200L,  50400L,  24L,
 27000L,  28110L,  48600L,  38L,
 60300L,  62100L,  46200L,  40L,
     0L,    510L,  45000L,  62L,
 20160L,  20760L,  45000L,  78L,
 25200L,  27000L,  45000L,  38L,
 76020L,  76800L,  45000L,  62L,
 22710L,  24960L,  43200L,  38L,
 65700L,  67920L,  43200L,  40L,
 75150L,  75780L,  42600L,  32L,
 75780L,  76020L,  42600L,  62L,
 41460L,  42720L,  39600L,  46L,
 22470L,  22710L,  36000L,  60L,
 24960L,  25200L,  36000L,  38L,
 28110L,  28530L,  36000L,  22L,
 85800L,  86400L,  36000L,  62L,
  6000L,  11820L,  35700L,   5L,
 72510L,  73080L,  30600L,  32L,
 48600L,  54300L,  28800L,   8L,
 81900L,  85800L,  27000L,  62L,
 28530L,  33300L,  25200L,  22L,
 33300L,  38700L,  25200L,  46L,
 65700L,  67184L,  22500L,  59L,
 67184L,  67920L,  22500L,   3L,
 75000L,  75150L,  21600L,  32L,
 25200L,  25260L,  19800L,  21L,
 65700L,  66330L,  16200L,  74L,
 57900L,  60300L,  14400L,  40L,
 65700L,  66330L,  10800L,  59L,
 77280L,  78000L,   9900L,  62L,
     0L,   7200L,   7200L,  67L,
 66900L,  67920L,   7200L,  74L,
 73080L,  75000L,   7200L,  32L,
 75000L,  76800L,   7200L,  35L,
 76800L,  77280L,   7200L,  62L,
 79200L,  81900L,   7200L,  62L,
 78000L,  79200L,   6300L,  62L,
 25260L,  25920L,   5400L,  21L,
 12900L,  16620L,      0L,  78L,
 16620L,  16800L,      0L,  60L,
 25920L,  29100L,      0L,  21L,
 52800L,  54300L,      0L,  86L,
 64200L,  65700L,      0L,  59L,
  9540L,  11820L,  -6300L,  17L,
 11820L,  12900L,  -6300L,  78L,
 54300L,  58560L, -11700L,  74L,
 16800L,  18300L, -14400L,  60L,
 21000L,  22470L, -14400L,  60L,
 64200L,  64680L, -14400L,  74L,
 65700L,  66900L, -14400L,  74L,
 66900L,  67920L, -14400L,   3L,
 81900L,  85800L, -14400L,  67L,
 38700L,  41460L, -21600L,  46L,
 41460L,  42600L, -21600L,  86L,
     0L,   1200L, -25200L,  67L,
 85800L,  86400L, -25200L,  67L,
 51300L,  52800L, -28800L,  86L,
 57300L,  58560L, -28800L,  59L,
 72000L,  73920L, -32400L,   3L,
 76800L,  78720L, -32400L,   4L,
 61800L,  64680L, -36000L,  59L,
 21000L,  29100L, -39600L,  55L,
 17700L,  18300L, -39600L,  36L,
 18300L,  21000L, -39600L,  60L,
 29100L,  30120L, -39600L,  42L,
 34500L,  38700L, -39600L,  75L,
 42600L,  46200L, -39600L,  86L,
 63300L,  63600L, -42000L,  59L,
 67920L,  72000L, -43320L,   3L,
 17400L,  17700L, -52200L,  36L,
 73920L,  76800L, -54000L,   4L,
 61800L,  65700L, -57600L,  74L,
 65700L,  67920L, -57600L,  73L,
 30120L,  30900L, -61200L,  42L,
 58560L,  58950L, -65700L,  59L,
 30900L,  32700L, -68400L,  42L,
 38700L,  39000L, -68400L,  27L,
 58560L,  58950L, -69300L,  59L,
 56400L,  57300L, -72000L,  48L,
 45300L,  46200L, -79200L,  29L,
 46200L,  51300L, -79200L,  86L,
 32700L,  35100L, -86400L,  42L,
  6000L,   9540L, -87780L,  17L,
  9540L,  13500L, -87780L,  36L,
 39000L,  42600L, -88200L,  27L,
 42600L,  45300L, -88200L,  29L,
 51300L,  53700L, -88200L,  48L,
 58560L,  60300L, -88500L,  59L,
     0L,   6000L, -91800L,  17L,
 76800L,  78720L, -91800L,  12L,
 78720L,  85800L, -91800L,   4L,
 85800L,  86400L, -91800L,  17L,
 35100L,  36900L, -95400L,  42L,
 16920L,  17400L, -98100L,  36L,
 17400L,  22020L, -98100L,  47L,
 72000L,  76800L, -100800L,  12L,
 36900L,  38100L, -105000L,  42L,
 45300L,  53700L, -106200L,  42L,
 53700L,  56400L, -106200L,  48L,
 56400L,  57600L, -106200L,  72L,
 16500L,  16920L, -108000L,  36L,
 60300L,  63360L, -108000L,  59L,
 63360L,  64200L, -108000L,  77L,
 38100L,  39000L, -112200L,  42L,
 22020L,  26520L, -118800L,  20L,
 44100L,  45300L, -118800L,  42L,
 39000L,  44100L, -126000L,  42L,
 12600L,  13500L, -129600L,  37L,
 30120L,  33720L, -132300L,  69L,
 15360L,  16500L, -133200L,  36L,
 64200L,  69000L, -133200L,  77L,
 76800L,  82800L, -133200L,  66L,
 82800L,  84000L, -133200L,  71L,
 10800L,  12600L, -142500L,  37L,
 33720L,  39600L, -143100L,   1L,
     0L,   6000L, -144000L,  71L,
  6000L,  10800L, -144000L,  37L,
 13920L,  15360L, -144000L,  36L,
 84000L,  86400L, -144000L,  71L,
 51000L,  53700L, -151200L,  15L,
 56400L,  57600L, -151200L,  50L,
 57600L,  59115L, -151200L,  72L,
 17400L,  18000L, -154800L,   9L,
 18000L,  23700L, -154800L,  23L,
 28800L,  30120L, -154800L,  68L,
 12300L,  13920L, -158400L,  36L,
 59115L,  64200L, -163800L,  72L,
 64200L,  69000L, -163800L,  25L,
 69000L,  73200L, -163800L,  77L,
 73200L,  76800L, -163800L,  54L,
 10800L,  12300L, -165600L,  36L,
 16200L,  17400L, -167400L,   9L,
 55200L,  56400L, -172800L,  50L,
     0L,   8400L, -173400L,  64L,
  9600L,  10800L, -176400L,  36L,
 14700L,  15360L, -176400L,  41L,
 15360L,  16200L, -176400L,   9L,
 76800L,  79200L, -180000L,  39L,
 21600L,  28800L, -182700L,  68L,
 28800L,  29400L, -182700L,  85L,
  8700L,   9600L, -183600L,  36L,
 13800L,  14700L, -183600L,  41L,
     0L,   6600L, -185400L,  64L,
 21600L,  22200L, -189000L,  13L,
 29400L,  30420L, -190800L,  85L,
 12600L,  13800L, -191400L,  41L,
 13800L,  14400L, -191400L,  33L,
     0L,   5700L, -192600L,  64L,
  7800L,   8700L, -194400L,  36L,
 16200L,  18000L, -194400L,  65L,
 54180L,  55200L, -194400L,  50L,
 30420L,  31800L, -196200L,  85L,
 22200L,  23400L, -198000L,  13L,
 42600L,  46200L, -198000L,  15L,
 51000L,  54180L, -198000L,  50L,
 54180L,  55200L, -198000L,  57L,
 14400L,  15600L, -203400L,  33L,
 31800L,  39600L, -203400L,  85L,
 39600L,  40500L, -203400L,  15L,
 63000L,  64800L, -205200L,   6L,
 64800L,  73200L, -205200L,  79L,
 79200L,  84000L, -205200L,  39L,
 11520L,  12600L, -207000L,  41L,
 18000L,  19800L, -207000L,  65L,
 23400L,  24600L, -208800L,  13L,
     0L,   4800L, -210600L,  64L,
  4800L,   7800L, -210600L,  36L,
 84000L,  86400L, -210600L,  64L,
 15600L,  16500L, -212400L,  33L,
 55200L,  59115L, -216000L,  57L,
 73200L,  76800L, -216000L,  44L,
 19800L,  21600L, -219600L,  65L,
 54600L,  55200L, -219600L,  19L,
 59115L,  59700L, -219600L,   6L,
 53700L,  54600L, -228900L,  19L,
 59700L,  60300L, -228900L,   6L,
 21600L,  24600L, -230400L,  65L,
 24600L,  32520L, -230400L,  13L,
 40500L,  42600L, -230400L,  15L,
 42600L,  46200L, -230400L,  28L,
 46200L,  52320L, -230400L,  15L,
 48600L,  49200L, -234000L,  19L,
 60300L,  60600L, -234000L,   6L,
  7800L,  11520L, -243000L,  41L,
 11520L,  16500L, -243000L,  70L,
 53100L,  53700L, -243000L,  19L,
 60600L,  63000L, -243000L,   6L,
 63000L,  64800L, -243000L,  61L,
 79200L,  84000L, -243000L,  82L,
 16500L,  23700L, -252000L,  33L,
 49200L,  53100L, -252000L,  19L,
 53100L,  61200L, -252000L,  80L,
     0L,   4800L, -270000L,  82L,
 12600L,  16500L, -270000L,  43L,
 23700L,  32520L, -270000L,  87L,
 32520L,  40500L, -270000L,  13L,
 40500L,  49200L, -270000L,  56L,
 64800L,  76800L, -270000L,  61L,
 76800L,  84000L, -270000L,  44L,
 84000L,  86400L, -270000L,  82L,
  2700L,   4800L, -273600L,  82L,
     0L,  12600L, -297000L,  43L,
 27600L,  49200L, -297000L,  18L,
 49200L,  64800L, -297000L,   2L,
 12600L,  27600L, -306000L,  53L,
     0L,  86400L, -324000L,  58L,
};


/* Return the constellation name corresponding to a given mean equatorial
   position P.  EPOCH is the precessional equinox and ecliptic date
   of P.  */

char *
whatconstel (pp, epoch)
double pp[];
double epoch;
{
  int i, k;
  double ra, dec, d;
  double p[3];

  for (i = 0; i < 3; i++)
    p[i] = pp[i];

  /* Precess from given epoch to J2000.  */
  precess (p, epoch, 1);
  /* Precess from J2000 to Besselian epoch 1875.0.  */
  precess (p, 2405889.25855, -1);
  d = p[0] * p[0] + p[1] * p[1] + p[2] * p[2];
  d = sqrt (d);
  ra = atan2 (p[1], p[0]) * (RTD * 3600. / 15.);
  if (ra < 0.0)
    ra += 86400.0;
  dec = asin (p[2] / d) * (RTD * 3600.);

    /* FIND CONSTELLATION SUCH THAT THE DECLINATION ENTERED IS HIGHER THAN
       THE LOWER BOUNDARY OF THE CONSTELLATION WHEN THE UPPER AND LOWER
       RIGHT ASCENSIONS FOR THE CONSTELLATION BOUND THE ENTERED RIGHT
       ASCENSION
    */
  for (i = 0; i < NBNDRIES; i++)
    {
      k = i << 2;
      if (ra >= bndries[k] && ra < bndries[k+1] && dec > bndries[k+2])
	{
	  k = (int) bndries[k+3];
	  return (constel[k]);
	}
    }
  return ("?? constellation not found");
}

#if 0
/* Test program  */
double J2000 = 2451545.0;
double STR = 4.8481368110953599359e-6;

test (r,d)
double r, d;
{
  double c, p[3], jd;

  d /= RTD;
  r *= 15.0/RTD;
  c = cos(d);
  p[2] = sin(d);
  p[0] = c * cos(r);
  p[1] = c * sin(r);
  jd = 2433282.423; /* 1950.0 Besselian epoch */
  printf ("%8.4f %9.4f %s\n", r, d, whatconstel (p, jd));
}


int
main()
{
  /*    The following is an example of the output of the program:
      RA =  9.0000 DEC =  65.0000  IS IN CONSTELLATION UMa
      RA = 23.5000 DEC = -20.0000  IS IN CONSTELLATION Aqr
      RA =  5.1200 DEC =   9.1200  IS IN CONSTELLATION Ori
      RA =  9.4555 DEC = -19.9000  IS IN CONSTELLATION Hya
      RA = 12.8888 DEC =  22.0000  IS IN CONSTELLATION Com
      RA = 15.6687 DEC = -12.1234  IS IN CONSTELLATION Lib
      RA = 19.0000 DEC = -40.0000  IS IN CONSTELLATION CrA
      RA =  6.2222 DEC = -81.1234  IS IN CONSTELLATION Men  */
  test (9.0, 65.0);
  test (23.5, -20.0);
  test (5.12, 9.12);
  test (9.4555, -19.9);
  test (12.8888, 22.0);
  test (15.6687, -12.1234);
  test (19.0, -40.0);
  test (6.2222, -81.1234);
  exit(0);
}
#endif
