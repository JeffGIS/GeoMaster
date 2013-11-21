/* gkrellsun.c
 * Copyright (C) 2001, 2002, Norman Walsh. Derived from gkrellmoon by
 * Dale P. Smith and wmSun by Mike Henderson
 *
 * $Id: gkrellsun.c,v 1.6 2002/11/13 11:15:01 nwalsh Exp $
 *
 */

#if !defined(WIN32)
#include <gkrellm2/gkrellm.h>
#else
#include <src/gkrellm.h>
#include <src/win32-plugin.h>
time_t* CurrentTime;
#endif

#include <math.h>

#undef DEBUG

#define HEIGHT 54
#define WIDTH 54
#define SUN_WIDTH 54
#define SUN_HEIGHT 54
#define SUN_INNER_WIDTH 52
#define SUN_INNER_HEIGHT 52
#define SUN_INNER_XOFS 1
#define SUN_INNER_YOFS 1

#include "../images/uvsun.xpm"
#include "../images/osun.xpm"

#define UVSUN_XPM uvsun_xpm
#define UVSUN_COUNT 2

#define OSUN_XPM osun_xpm
#define OSUN_COUNT 6

#include "../images/digitsy.xpm"
#include "../images/digitsw.xpm"

#define DIGITSY_XPM digitsy_xpm
#define DIGITSY_WIDTH 7
#define DIGITSY_HEIGHT 11

#define DIGITSW_XPM digitsw_xpm
#define DIGITSW_WIDTH 4
#define DIGITSW_HEIGHT 5

#define DIGIT_COLON 10
#define DIGIT_DASH 11
#define DIGIT_AM 12
#define DIGIT_PM 13
#define DIGIT_SPACE 14
#define DIGIT_COUNT 15

#include "../images/star.xpm"

#define STAR_XPM star_xpm
#define STAR_XOFS 3
#define STAR_YOFS 3
#define STAR_COUNT 2

#include "../images/dot.xpm"

#define DOT_XPM dot_xpm
#define DOT_COUNT 2

#include "../images/moon.xpm"

#define MOON_XPM moon_xpm
#define MOON_WIDTH 8
#define MOON_HEIGHT 8
#define MOON_COUNT 61
#define MOON_INVISIBLE 60

#define STYLE_NAME "sun"
#define PLUGIN_CONFIG_KEYWORD	"sun"
#define SUNCLOCK_MAJOR_VERSION	0
#define SUNCLOCK_MINOR_VERSION	9
#define SUNCLOCK_PATCH_VERSION	1

typedef struct {
  int longitude;
  int latitude;
  int clock24;
  int showStar;
  int showPath;
  int show90Path;
  int showETA;
  int orangeSun;
  int showMoon;
  int debug;
} Options;

static GkrellmMonitor *sun_monitor;
static GkrellmPanel   *panel;
static Options options;
static gint    style_id;
static gint    redraw = 1;

static int baseX = 0;
static int baseY = 0;

static GtkWidget *longitude_spin_button = NULL;
static GtkWidget *latitude_spin_button = NULL;
static GtkWidget *clock24_button = NULL;
static GtkWidget *showStar_button = NULL;
static GtkWidget *showMoon_button = NULL;
static GtkWidget *showPath_button = NULL;
static GtkWidget *show90Path_button = NULL;
static GtkWidget *showETA_button = NULL;
static GtkWidget *orangeSun_button = NULL;
static GtkWidget *debug_button = NULL;

static GdkPixmap *uvsun_image = NULL;
static GdkBitmap *uvsun_mask = NULL;
static GkrellmDecal *uvsun = NULL;

static GdkPixmap *osun_image = NULL;
static GdkBitmap *osun_mask = NULL;
static GkrellmDecal *osun = NULL;

static GdkPixmap *star_image = NULL;
static GdkBitmap *star_mask = NULL;
static GkrellmDecal *star = NULL;

static GdkPixmap *dot_image = NULL;
static GdkBitmap *dot_mask = NULL;

static GdkPixmap *moon_image = NULL;
static GdkBitmap *moon_mask = NULL;
static GkrellmDecal *moon = NULL;

static GdkPixmap *digitsy_image = NULL;
static GdkBitmap *digitsy_mask = NULL;
static GdkPixmap *digitsw_image = NULL;
static GdkBitmap *digitsw_mask = NULL;

#define RISEHOURY_X 16
#define RISEHOURY_Y 10

static GkrellmDecal *riseTimeY[6];

#define RISEHOURW_X 4
#define RISEHOURW_Y 5

static GkrellmDecal *riseTimeW[6];

#define SETHOURY_X 16
#define SETHOURY_Y 36

static GkrellmDecal *setTimeY[6];

#define SETHOURW_X (SUN_WIDTH - (4*5 + 3))
#define SETHOURW_Y 5

static GkrellmDecal *setTimeW[6];

#define RISEETAW_X 4
#define RISEETAW_Y 12

static GkrellmDecal *riseETA[6];

#define SETETAW_X (SUN_WIDTH - (4*5 + 3))
#define SETETAW_Y 12

static GkrellmDecal *setETA[6];

#define PATH_COUNT 14
#define HALF_PATH_COUNT 7
#define PATH_SPACING 4
static GkrellmDecal *path[PATH_COUNT];
static GkrellmDecal *path90[PATH_COUNT];

static GtkTooltips *tooltip;
static GkrellmTicks *pGK;

#include "CalcEphem.h"

typedef struct CTrans SunData;

typedef struct _Sun Sun;
struct _Sun {
    SunData data;
};

extern long CurrentGMTTime;
static Sun sununit;

static void update_tooltip(Sun *sun);

static void update_sun_data(Sun * sun) {
    struct tm *time_struc;	/* The tm struct is defined in <time.h> */
    gdouble local_std_time, univ_time, eot;
    glong date;
    gint day_of_month, month, year;

    CurrentGMTTime = time(NULL);

    time_struc = gmtime(&CurrentGMTTime);
    univ_time =
	time_struc->tm_hour + time_struc->tm_min / 60.0 +
	time_struc->tm_sec / 3600.0;

    /* The date needs to be the date in UTC, i.e. in greenwich, so
     * be sure not to call the localtime function until after date
     * has been set (there's only one tm structure).  */

    year = time_struc->tm_year + 1900;
    month = time_struc->tm_mon + 1;
    day_of_month = time_struc->tm_mday;

    date = year * 10000 + month * 100 + day_of_month;

    if (options.debug) {
      printf("gkrellsun debug: GMT date = %04d-%02d-%02d (%ld)\n",
	     year, month, day_of_month, date);
    }

    time_struc = localtime(&CurrentGMTTime);
    local_std_time =
	time_struc->tm_hour + time_struc->tm_min / 60.0 +
	time_struc->tm_sec / 3600.0;

    if (options.debug) {
      printf("gkrellsun debug: local date = %04d-%02d-%02d\n",
	     time_struc->tm_year + 1900,
	     time_struc->tm_mon + 1,
	     time_struc->tm_mday);
    }

    sun->data.Glat = options.latitude;
    sun->data.Glon = options.longitude;

    if (options.debug) {
      printf("gkrellsun debug: latitude  = %d\n", options.latitude);
      printf("gkrellsun debug: longitude = %d\n", options.longitude);
    }

    sunclock_CalcEphem(date, univ_time, &sun->data, options.debug);

    sun->data.LST = local_std_time;
    sun->data.LMT = univ_time - sun->data.Glon / 15.0;
    if (sun->data.LMT < 0.0)
	sun->data.LMT += 24.0;
    if (sun->data.LMT > 24.0)
	sun->data.LMT -= 24.0;

    if (options.debug) {
      printf("gkrellsun debug: sun LST  = %6.2f\n", sun->data.LST);
      printf("gkrellsun debug: sun LMT  = %6.2f\n", sun->data.LMT);
      printf("gkrellsun debug: sun Rise = %d\n", sun->data.Rise);
      printf("gkrellsun debug: sun Set  = %d\n", sun->data.Set);
      printf("gkrellsun debug: sun LTRise = %6.2f\n", sun->data.LTRise);
      printf("gkrellsun debug: sun LTSet  = %6.2f\n", sun->data.LTSet);
      printf("gkrellsun debug: A_moon = %6.2f\n", sun->data.A_moon);
      printf("gkrellsun debug: h_moon = %6.2f\n", sun->data.h_moon);
      printf("gkrellsun debug: moon age = %6.2f\n", sun->data.MoonAge);
      printf("gkrellsun debug: SinGlat = %6.2f\n", sun->data.SinGlat);
      printf("gkrellsun debug: CosGlat = %6.2f\n", sun->data.CosGlat);
    }

    /* eot is the equation of time. gmst is Greenwich Sidereal
     * Time.  This equation below is correct, but confusing at
     * first.  It's easy to see when you draw the following
     * picture: A sphere with 0 and 180 degree longitude, North on
     * top, a meridian for the real sun, a meridian for a fictive
     * average sun, a meridian denoting the vernal equinox.  Note
     * that universal time is the hour angle between 180 degrees
     * and the fictive sun's meridian measured clockwise.  gmst is
     * the hour angle between 0 degrees and the meridian of the
     * vernal equinox measured clockwise.  RA_sun/15.0 is the hour
     * angle of the real sun measured counterclockwise from the
     * vernal equinox. eot is the difference between the real and
     * the fictive sun.  Looking at the picture, it's easy to see
     * that 12=RA_sun/15-gmst+eot+utc (12 hours = 180 deg.) */

    eot =
	12.0 - univ_time + sun->data.gmst - sun->data.RA_sun / 15.0;

    if (eot < 0.0)
	eot += 24.0;
    if (eot > 24.0)
	eot -= 24.0;

    sun->data.LAT = sun->data.LMT + eot;
    if (sun->data.LAT < 0.0)
	sun->data.LAT += 24.0;
    if (sun->data.LAT > 24.0)
	sun->data.LAT -= 24.0;

    update_tooltip(sun);
}

/* Adjust the hour to reflect 12 or 24 hour clock
 */
static int
clock_adjust_hour( const int H ) {

    int hour;

    hour = H;

    /* For a 24-hour clock, range is 0..23 */
    if( options.clock24 ) {
        hour %= 24;
    }
    /* For a 12-hour clock, range is 1..12 */
    else {
        hour = ( ( hour - 1 ) % 12 ) + 1;
    }

    return hour;
} /* End of clock_adjust_hour() */

/* Adjust AM/PM indicator to reflect 12 or 24 hour clock
 */
static int
clock_ampm( const int H ) {

    int ampm_digit;

    /* Default is 24-hour (no AM/PM indicator) */
    ampm_digit = DIGIT_SPACE;

    /* For a 12-hour clock, set the AM/PM indicator */
    if( ! options.clock24 ) {
        if( H < 12 ) {
            ampm_digit = DIGIT_AM;
        } else {
            ampm_digit = DIGIT_PM;
        }
    }

    return ampm_digit;
} /* End of clock_ampm() */

/* Determine the letter to be used for a given AM/PM digit value
 */
static char
ampm_letter( const int ampm_digit ) {

    char letter;

    switch( ampm_digit ) {
    case DIGIT_AM:
        letter = 'a';
        break;
    case DIGIT_PM:
        letter = 'p';
        break;
    case DIGIT_SPACE:
        letter = ' ';
        break;
    default:
        letter = '?';
        break;
    }

    return letter;

} /* End of ampm_letter() */

#ifdef DEBUG
static void printTOD(double tod) {
  double val = tod;
  int H, M;

  H = (int)val; val = (val-H)*60.0;
  M = (int)val;
  if (H >= 12) {
    if (H > 12) {
      H -= 12;
    }
    printf("%d:%02dp\n", H, M);
  } else {
    printf("%d:%02da\n", H, M);
  }
}
#endif

static double percentOfDay(Sun *sun, double tod) {
  if (sun->data.Rise && sun->data.Set) {
    return ((tod - sun->data.LTRise) / (sun->data.LTSet - sun->data.LTRise));
  } else {
    return 0;
  }
}

static double percentOfAltitude(Sun *sun, double tod) {
  double dayperc = percentOfDay(sun, tod);
  if (dayperc > 0.5) {
    return 1.0 - dayperc;
  } else {
    return dayperc;
  }
}

static double altitudeAtNoon(Sun *sun) {
  double altnoon = 90 - sun->data.Glat + sun->data.DEC_sun;
  if (altnoon > 90) {
    altnoon = 90 - (altnoon - 90);
  }
  return altnoon;
}

static void drawSun(Sun *sun) {
  int image_number;
  double perc = percentOfAltitude(sun, sun->data.LST) * 2;

  if (options.orangeSun) {
    gkrellm_draw_decal_pixmap(panel, uvsun, 0);
    if (sun->data.Rise && sun->data.Set) {
      /* If the sun rises and sets today... */
      if (sun->data.LST < sun->data.LTRise || sun->data.LST > sun->data.LTSet) {
	image_number = 1;
      } else {
	image_number = 2;
	if (perc > 0.25) image_number = 3;
	if (perc > 0.50) image_number = 4;
	if (perc > 0.75) image_number = 5;
      }
    } else if (!sun->data.Rise && sun->data.Set) {
      /* If the sun doesn't rise but it does set... */
      if (sun->data.LST > sun->data.LTRise) {
	image_number = 1;
      } else {
	image_number = 4;
      }
    } else if (sun->data.Rise && !sun->data.Set) {
      /* If the sun rises but it doesn't set... */
      if (sun->data.LST < sun->data.LTRise) {
	image_number = 1;
      } else {
	image_number = 4;
      }
    } else {
      /* The sun neither rises nor sets */
      image_number = 1;
    }
    gkrellm_draw_decal_pixmap(panel, osun, image_number);
  } else {
    gkrellm_draw_decal_pixmap(panel, osun, 0);
    gkrellm_draw_decal_pixmap(panel, uvsun, 1);
  }
}

static int
moon_image_number(double MoonPhase) {
    /* MoonPhase expresses phase of moon as fraction of 1; 0.5=full. */
    gdouble image_float;
    gint image_int;
    gint image_number;

    if (!options.showMoon) {
      return MOON_INVISIBLE;
    }

    image_float = MoonPhase * (gdouble) 60;
    image_int = (gint) image_float;

    if ((image_float - image_int) >= 0.5)
	image_number = (image_int + 1) % 60;
    else
	image_number = image_int % 60;

    return image_number;
}

static void drawMoon(Sun *sun) {
  int image_number = moon_image_number(sun->data.MoonPhase);
  double alt = sun->data.h_moon;
  int x = baseX + WIDTH - MOON_WIDTH - SUN_INNER_XOFS;
  int y = 0;

  /* turn off the moon */
  gkrellm_draw_decal_pixmap(panel, moon, MOON_INVISIBLE);

  if (alt < 0) {
    return;
  }

  y = (int) (alt / 90.0 * HEIGHT / 2.0);
  y = baseY + y;
  y = SUN_INNER_HEIGHT - y - SUN_INNER_YOFS;

#ifdef DEBUG
  printf("Moon at %d, %d (%6.2f): %d\n", x, y, alt, image_number);
#endif

  gkrellm_move_decal(panel, moon, x, y);
  gkrellm_draw_decal_pixmap(panel, moon, image_number);
}

/* for a circle that crosses (0,0), (26, y2), (52, 0) compute the center (x0,y0) */

static double centerX(double y2) {
  return 26;
}

static double centerY(double y2) {
  double x1 = 0;
  double y1 = 0;

  double x2 = 26;

  return ( ((y2*y2) - (x1*x1) + (2*x1*x2) - (x2*x2))
	   / (2 * (y2 - y1)) );
}

static int computeY(int x, double maxAlt) {
  double x0, y0, x2, y2, radius;
  double tx, ty;

  /* Compute y2, the point at apogee */
  x2 = 26;
  y2 = 26 * maxAlt / 90.0;

  /* Compute x0,y0 the center of the circle */
  x0 = centerX(y2);
  y0 = centerY(y2);

  radius = y2 - y0;

  /* calculate y from the circle... */
  tx = x - 26;
  ty = sqrt((radius*radius) - (tx*tx));

  return (int) (ty + y0);
}

static void computePath(Sun *sun, GkrellmDecal **path, double maxAlt) {
  double dayLength = sun->data.LTSet - sun->data.LTRise;
  double unitLength = dayLength / (PATH_COUNT-1);
  double tod;
  int count, x, y;

  /* Divide the length of the solar day into PATH_COUNT segments */
  if (!sun->data.Rise || !sun->data.Set) {
    return;
  }

#ifdef DEBUG
  printf ("Rise: ");
  printTOD(sun->data.LTRise);
  printf ("Set: ");
  printTOD(sun->data.LTSet);
  printf ("At Noon: %6.2f\n", altitudeAtNoon(sun));
  printf ("Max: %6.2f\n", maxAlt);
#endif

  for (count = 0; count < PATH_COUNT; count++) {
    tod = sun->data.LTRise + (count * unitLength);

    x = (int) (SUN_INNER_WIDTH * percentOfDay(sun, tod));
    y = computeY(x, maxAlt);

#ifdef DEBUG
    printf ("[%d] ", count);
    printf ("%6.2f, %6.2f (%d, %d) ", percentOfDay(sun, tod), percentOfAltitude(sun, tod), x, y);
    printTOD(tod);
#endif

    x = baseX + x + SUN_INNER_XOFS;
    y = baseY + y;

    y = SUN_INNER_HEIGHT - y - SUN_INNER_YOFS;

    gkrellm_move_decal(panel, path[count], x, y);

  }

#ifdef DEBUG
  printf("-------\n");
#endif
}

static void drawPath(GkrellmDecal **path, int imgNo) {
  int count;
  for (count = 0; count < PATH_COUNT; count++) {
    gkrellm_draw_decal_pixmap(panel, path[count], imgNo);
  }
}

static void drawTime(GkrellmDecal **time, double tod, int showTime, int showBlank, int eta) {
  double val = tod;
  int H, M, ampm_digit;

  H = (int)val;
  val = (val-H)*60.0;
  M = (int)val;
  ampm_digit = clock_ampm( H );
  H = clock_adjust_hour( H );

  if (eta) {
    ampm_digit = DIGIT_SPACE;
  }

  if (showTime) {
    gkrellm_draw_decal_pixmap(panel, time[0], (int)(H / 10));
    gkrellm_draw_decal_pixmap(panel, time[1], H % 10);
    gkrellm_draw_decal_pixmap(panel, time[2], DIGIT_COLON);
    gkrellm_draw_decal_pixmap(panel, time[3], (int)(M / 10));
    gkrellm_draw_decal_pixmap(panel, time[4], M % 10);
    gkrellm_draw_decal_pixmap(panel, time[5], ampm_digit);
  } else if (showBlank) {
    gkrellm_draw_decal_pixmap(panel, time[0], DIGIT_SPACE);
    gkrellm_draw_decal_pixmap(panel, time[1], DIGIT_SPACE);
    gkrellm_draw_decal_pixmap(panel, time[2], DIGIT_SPACE);
    gkrellm_draw_decal_pixmap(panel, time[3], DIGIT_SPACE);
    gkrellm_draw_decal_pixmap(panel, time[4], DIGIT_SPACE);
    gkrellm_draw_decal_pixmap(panel, time[5], DIGIT_SPACE);
  } else {
    gkrellm_draw_decal_pixmap(panel, time[0], DIGIT_DASH);
    gkrellm_draw_decal_pixmap(panel, time[1], DIGIT_DASH);
    gkrellm_draw_decal_pixmap(panel, time[2], DIGIT_COLON);
    gkrellm_draw_decal_pixmap(panel, time[3], DIGIT_DASH);
    gkrellm_draw_decal_pixmap(panel, time[4], DIGIT_DASH);
    gkrellm_draw_decal_pixmap(panel, time[5], DIGIT_SPACE);
  }
}

static void
sun_update_plugin() {
  int image_x_offset, image_y_offset;
  int pos_x, pos_y;
  int starImage = 0;
  double eta = 0;
  int inDaylight = 0;
  GkrellmDecal **riseTime;
  GkrellmDecal **setTime;

  /* Draw plugin specific data on the chart */
  /* Use xlib or gdk functions, or gkrellm_draw_chart() if applicable */

  /* After the first drawing, there's no reason to do this more than once a minute! */
  if (redraw || pGK->minute_tick) {
    update_sun_data(&sununit);
  }
  redraw = 0;

  inDaylight = (sununit.data.LST >= sununit.data.LTRise
		&& sununit.data.LST <= sununit.data.LTSet
		&& sununit.data.Rise
		&& sununit.data.Set);

  drawSun(&sununit);
  drawMoon(&sununit);

  drawPath(path, options.showPath && inDaylight);
  drawPath(path90, options.show90Path && inDaylight);

  if (options.orangeSun) {
    /* Make sure the yellow numbers are all "blanks" */
    drawTime(riseTimeY, 0, 0, 1, 0);
    drawTime(setTimeY, 0, 0, 1, 0);

    if (options.showETA) {
      if (sununit.data.LST < sununit.data.LTRise) {
	eta = sununit.data.LTRise - sununit.data.LST;
	drawTime(riseETA, eta, 1, 0, 1);
      } else {
	drawTime(riseETA, 0, 0, 1, 1);
      }

      if (sununit.data.LST >= sununit.data.LTRise
	  && sununit.data.LST < sununit.data.LTSet) {
	eta = sununit.data.LTSet - sununit.data.LST;
	drawTime(setETA, eta, 1, 0, 1);
      } else {
	drawTime(setETA, 0, 0, 1, 1);
      }
    } else {
      drawTime(riseETA, eta, 0, 1, 0);
      drawTime(setETA, eta, 0, 1, 0);
    }

    riseTime = riseTimeW;
    setTime = setTimeW;
  } else {
    /* Make sure the white numbers are all "blanks" */
    drawTime(riseTimeW, 0, 0, 1, 0);
    drawTime(setTimeW, 0, 0, 1, 0);
    drawTime(riseETA, 0, 0, 1, 0);
    drawTime(setETA, 0, 0, 1, 0);

    riseTime = riseTimeY;
    setTime = setTimeY;
  }

  if (sununit.data.Rise) {
    drawTime(riseTime, sununit.data.LTRise, 1, 0, 0);
  } else {
    drawTime(riseTime, sununit.data.LTRise, 0, 0, 0);
  }

  if (sununit.data.Set) {
    drawTime(setTime, sununit.data.LTSet, 1, 0, 0);
  } else {
    drawTime(setTime, sununit.data.LTSet, 0, 0, 0);
  }

  starImage = 0; /* by default off */

  /* If the user wants the star, and the sun rises and sets today, draw it... */
  if (options.showStar && inDaylight) {

#ifdef DEBUG
  printf ("Now: ");
  printTOD(sununit.data.LST);
#endif

    image_x_offset = baseX;
    image_y_offset = baseY;

    /* Calculate plot pos */
    pos_x = (int) (SUN_INNER_WIDTH * percentOfDay(&sununit, sununit.data.LST));
    pos_y = computeY(pos_x, altitudeAtNoon(&sununit));

    image_x_offset += pos_x;
    image_y_offset += pos_y;

    image_x_offset += SUN_INNER_XOFS;
    image_y_offset = SUN_INNER_HEIGHT - image_y_offset - SUN_INNER_XOFS;

    image_x_offset -= STAR_XOFS;
    image_y_offset -= STAR_YOFS;

#ifdef DEBUG
    printf ("Star at: (%d,%d)\n", image_x_offset, image_y_offset);
#endif

    gkrellm_move_decal(panel, star, image_x_offset, image_y_offset);
    starImage = 1;
  }

  gkrellm_draw_decal_pixmap(panel, star, starImage);
  gkrellm_draw_panel_layers(panel);
}

static void
update_tooltip(Sun *sun)
{
    GString	*mboxes = NULL;
    gchar	buf[128];
    gchar       time_format[128];
    gchar       format_string[128];
    double      val, altnoon;
    int         H,M;
    int         ampm_digit;

    if (tooltip == NULL)
	return;

    mboxes = g_string_sized_new(512);

    snprintf(buf, sizeof(buf), "Location: %d lat %d lon\n",
	     options.latitude, options.longitude);
    g_string_append(mboxes, buf);

    /* Determine clock format */
    if( options.clock24 ) {
        strncpy( time_format, "%02d:%02d", sizeof( time_format ) );
    } else {
        strncpy( time_format, "%d:%02d%c", sizeof( time_format ) );
    }

    /* Sun rise ... */

    H = 0;
    M = 0;
    ampm_digit = DIGIT_SPACE;
    if (sun->data.Rise) {
      val = sun->data.LTRise;
      H = (int)val; val = (val-H)*60.0;
      M = (int)val;

      ampm_digit = clock_ampm( H );
      H = clock_adjust_hour( H );
      snprintf( format_string, sizeof( format_string ), "%s: %s\n",
                "Sunrise", time_format );
    } else {
      strncpy( format_string, "Sunrise: never\n", sizeof( format_string ) );
    }
    snprintf( buf, sizeof( buf ), format_string,
                H, M, ampm_letter( ampm_digit ) );
    g_string_append(mboxes, buf);

    /* Sun set ... */

    H = 0;
    M = 0;
    ampm_digit = DIGIT_SPACE;
    if (sun->data.Set) {
      val = sun->data.LTSet;
      H = (int)val; val = (val-H)*60.0;
      M = (int)val;

      ampm_digit = clock_ampm( H );
      H = clock_adjust_hour( H );
      snprintf( format_string, sizeof( format_string ), "%s: %s\n",
                "Sunset", time_format );
    } else {
      strncpy( format_string, "Sunset: never\n", sizeof( format_string ) );
    }
    snprintf( buf, sizeof( buf ), format_string,
                H, M, ampm_letter( ampm_digit ) );
    g_string_append(mboxes, buf);

    /* Solar noon ... */

    H = 0;
    M = 0;
    ampm_digit = DIGIT_SPACE;
    if (sun->data.Rise && sun->data.Set) {
      val = sun->data.LTRise + ((sun->data.LTSet - sun->data.LTRise)/2.0);
      H = (int)val; val = (val-H)*60.0;
      M = (int)val;

      ampm_digit = clock_ampm( H );
      H = clock_adjust_hour( H );
      snprintf( format_string, sizeof( format_string ), "%s: %s\n",
                "Solar noon", time_format );

      snprintf( buf, sizeof( buf ), format_string,
                H, M, ampm_letter( ampm_digit ) );
      g_string_append(mboxes, buf);

      /* Solar altitude at noon */
      altnoon = 90 - sun->data.Glat + sun->data.DEC_sun;
      if (altnoon > 90) {
	altnoon = 90 - (altnoon - 90);
      }

      snprintf (buf, sizeof (buf), "%s: %4.1f\n", "Altitude at noon", altnoon);
      g_string_append(mboxes, buf);

      /* Altitude now */

      val = ((sun->data.LST - sun->data.LTRise)
	     / (sun->data.LTSet - sun->data.LTRise));

      if (val > 0.5) {
	val = 1.0 - val;
      }

      snprintf (buf, sizeof (buf), "%s: %4.1f\n",
		"Altitude now", altnoon * (val * 2));
      g_string_append(mboxes, buf);
    }

    gtk_tooltips_set_tip(tooltip, panel->drawing_area, mboxes->str, NULL);
    gtk_tooltips_set_delay(tooltip, 750);
    gtk_tooltips_enable(tooltip);
    if (mboxes)
	g_string_free(mboxes, TRUE);
}


static gint
panel_expose_event(GtkWidget *widget, GdkEventExpose *ev)
{
    gdk_draw_pixmap(widget->window,
		    widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		    panel->pixmap, ev->area.x, ev->area.y, ev->area.x, ev->area.y,
		    ev->area.width, ev->area.height);
    return FALSE;
}

static void panel_button_event(GtkWidget *widget, GdkEventButton *ev, gpointer data) {
  if (ev->button == 3)
    gkrellm_open_config_window(sun_monitor);
}

static void load_images() {
    GkrellmPiximage *image = NULL;

    gkrellm_load_piximage(NULL, OSUN_XPM, &image, NULL);
    gkrellm_scale_piximage_to_pixmap(image, &osun_image, &osun_mask, 0, 0);

    gkrellm_load_piximage(NULL, UVSUN_XPM, &image, NULL);
    gkrellm_scale_piximage_to_pixmap(image, &uvsun_image, &uvsun_mask, 0, 0);

    gkrellm_load_piximage(NULL, STAR_XPM, &image, NULL);
    gkrellm_scale_piximage_to_pixmap(image, &star_image, &star_mask, 0, 0);

    gkrellm_load_piximage(NULL, MOON_XPM, &image, NULL);
    gkrellm_scale_piximage_to_pixmap(image, &moon_image, &moon_mask, 0, 0);

    gkrellm_load_piximage(NULL, DOT_XPM, &image, NULL);
    gkrellm_scale_piximage_to_pixmap(image, &dot_image, &dot_mask, 0, 0);

    gkrellm_load_piximage(NULL, digitsy_xpm, &image, NULL);
    gkrellm_scale_piximage_to_pixmap(image, &digitsy_image, &digitsy_mask, 0, 0);

    gkrellm_load_piximage(NULL, digitsw_xpm, &image, NULL);
    gkrellm_scale_piximage_to_pixmap(image, &digitsw_image, &digitsw_mask, 0, 0);
}

static void sun_create_plugin(GtkWidget *vbox, gint first_create) {
  GkrellmStyle *style = NULL;
  int   image_x_offset, image_y_offset;
  int   count;

  update_sun_data(&sununit);

  load_images();

  if (first_create) {
    panel = gkrellm_panel_new0();
  } else {
    gkrellm_destroy_decal_list(panel);
  }

  style = gkrellm_meter_style(style_id);

  /* Load the solar images */
  baseX = (gkrellm_chart_width() - SUN_WIDTH) / 2;
  baseY = (HEIGHT - SUN_HEIGHT) / 2;

  image_x_offset = baseX;
  image_y_offset = baseY; 

  uvsun = gkrellm_create_decal_pixmap(panel, uvsun_image, uvsun_mask,
				      UVSUN_COUNT,
				      style, image_x_offset, image_y_offset);

  osun = gkrellm_create_decal_pixmap(panel, osun_image, osun_mask,
				     OSUN_COUNT,
				     style, image_x_offset, image_y_offset);

  /* Load the "yellow" numbers */
  image_x_offset = RISEHOURY_X;
  image_y_offset = RISEHOURY_Y;

  for (count = 0; count < 6; count++) {
    /* tweak the position of the colon */
    if (count == 2) image_x_offset -= 2;
    if (count == 3) image_x_offset -= 3;

    riseTimeY[count] = gkrellm_create_decal_pixmap(panel, digitsy_image, digitsy_mask,
						   DIGIT_COUNT, style,
						   image_x_offset, image_y_offset);
    image_x_offset += DIGITSY_WIDTH;
  }

  image_x_offset = SETHOURY_X;
  image_y_offset = SETHOURY_Y;

  for (count = 0; count < 6; count++) {
    /* tweak the position of the colon */
    if (count == 2) image_x_offset -= 2;
    if (count == 3) image_x_offset -= 3;

    setTimeY[count] = gkrellm_create_decal_pixmap(panel, digitsy_image, digitsy_mask,
						  DIGIT_COUNT, style,
						  image_x_offset, image_y_offset);
    image_x_offset += DIGITSY_WIDTH;
  }

  /* Load the "white" numbers */
  image_x_offset = RISEHOURW_X;
  image_y_offset = RISEHOURW_Y;

  for (count = 0; count < 6; count++) {
    riseTimeW[count] = gkrellm_create_decal_pixmap(panel, digitsw_image, digitsw_mask,
						   DIGIT_COUNT, style,
						   image_x_offset, image_y_offset);
    image_x_offset += DIGITSW_WIDTH;
  }

  image_x_offset = SETHOURW_X;
  image_y_offset = SETHOURW_Y;

  for (count = 0; count < 6; count++) {
    setTimeW[count] = gkrellm_create_decal_pixmap(panel, digitsw_image, digitsw_mask,
						  DIGIT_COUNT, style,
						  image_x_offset, image_y_offset);
    image_x_offset += DIGITSW_WIDTH;
  }

  /* Load the "white" ETA numbers */
  image_x_offset = RISEETAW_X;
  image_y_offset = RISEETAW_Y;

  for (count = 0; count < 6; count++) {
    riseETA[count] = gkrellm_create_decal_pixmap(panel, digitsw_image, digitsw_mask,
						 DIGIT_COUNT, style,
						 image_x_offset, image_y_offset);
    image_x_offset += DIGITSW_WIDTH;
  }

  image_x_offset = SETETAW_X;
  image_y_offset = SETETAW_Y;

  for (count = 0; count < 6; count++) {
    setETA[count] = gkrellm_create_decal_pixmap(panel, digitsw_image, digitsw_mask,
						DIGIT_COUNT, style,
						image_x_offset, image_y_offset);
    image_x_offset += DIGITSW_WIDTH;
  }

  /* Load the star */
  image_x_offset = baseX;
  image_y_offset = baseY;

  image_x_offset += 3;
  image_y_offset += 3;

  star = gkrellm_create_decal_pixmap(panel, star_image, star_mask,
				     STAR_COUNT,
				     style, image_x_offset, image_y_offset);

  /* Load the dots */
  for (count = 0; count < PATH_COUNT; count++) {
    path[count] = gkrellm_create_decal_pixmap(panel, dot_image, dot_mask,
					      DIGIT_COUNT, style, 0, count*PATH_SPACING);
    path90[count] = gkrellm_create_decal_pixmap(panel, dot_image, dot_mask,
						DIGIT_COUNT, style, 0, count*PATH_SPACING);
  }

  computePath(&sununit, path, altitudeAtNoon(&sununit));
  computePath(&sununit, path90, 90);

  /* Load the moon */
  image_x_offset = baseX + WIDTH - MOON_WIDTH - SUN_INNER_XOFS;
  image_y_offset = 27;

  moon = gkrellm_create_decal_pixmap(panel, moon_image, moon_mask,
				     MOON_COUNT,
				     style, image_x_offset, image_y_offset);


  panel->textstyle = gkrellm_meter_textstyle(style_id);
  //gkrellm_panel_configure_add_height(panel, HEIGHT);
  gkrellm_panel_configure(panel, NULL, style);
  gkrellm_panel_create(vbox, sun_monitor, panel);

  if (first_create) {
    gtk_signal_connect(GTK_OBJECT(panel->drawing_area),
		       "expose_event", (GtkSignalFunc) panel_expose_event,
		       NULL);

    gtk_signal_connect(GTK_OBJECT(panel->drawing_area),
		       "button_press_event", (GtkSignalFunc) panel_button_event,
		       NULL);

    tooltip=gtk_tooltips_new();
  }

  drawSun(&sununit);
  //gkrellm_draw_panel_layers(panel);
}

static void
sun_create_tab(GtkWidget *tab_vbox) {
  GtkWidget		*tabs;
  GtkWidget		*vbox;

  tabs = gtk_notebook_new();
  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(tabs), GTK_POS_TOP);
  gtk_box_pack_start(GTK_BOX(tab_vbox), tabs, TRUE, TRUE, 0);

  /* --Setup Tab */
  vbox = gkrellm_gtk_notebook_page(tabs, _("Setup"));

  gkrellm_gtk_spin_button(vbox, &longitude_spin_button,
			  (gfloat) options.longitude,
			  -180.0, 180.0, 1.0, 1.0, 0, 60, NULL, NULL,
			  FALSE, _("Longitude (decimal degrees + = W, - = E)"));

  gkrellm_gtk_spin_button(vbox, &latitude_spin_button,
			  (gfloat) options.latitude,
			  -90.0, 90.0, 1.0, 1.0, 0, 60, NULL, NULL,
			  FALSE, _("Latitude (decimal degrees + = N, - = S)"));

  gkrellm_gtk_check_button(vbox, &clock24_button,
			   options.clock24,
			   TRUE, 0, _("Use 24 hour clock"));

  gkrellm_gtk_check_button(vbox, &showMoon_button,
			   options.showMoon,
			   TRUE, 0, _("Show moon"));

  gkrellm_gtk_check_button(vbox, &showStar_button,
			   options.showStar,
			   TRUE, 0, _("Show star"));

  gkrellm_gtk_check_button(vbox, &showPath_button,
			   options.showPath,
			   TRUE, 0, _("Show path"));

  gkrellm_gtk_check_button(vbox, &show90Path_button,
			   options.show90Path,
			   TRUE, 0, _("Show apogee path"));

  gkrellm_gtk_check_button(vbox, &orangeSun_button,
			   options.orangeSun,
			   TRUE, 0, _("Use orange sun"));

  gkrellm_gtk_check_button(vbox, &showETA_button,
			   options.showETA,
			   TRUE, 0, _("Show sun rise/set ETA (requires orange sun)"));

  gkrellm_gtk_check_button(vbox, &debug_button,
			   options.debug,
			   TRUE, 0, _("Enable debugging output"));

/* ----------------- info text --------------------*/
    {
	GtkWidget *text;
	gchar *info_text[] = {
	    "<b>GKrellM2 SunClock Plugin\n\n",
	    "<b>Options:\n\n",
	    "<b>Use 24 hour clock:\n",
	    "\tdisplay sunrise/sunset using 24 hour clock\n",
	    "<b>Show moon:\n",
	    "\tdisplay a small image of the moon when it's visible\n",
	    "<b>Show star:\n",
	    "\tdisplay a small star showing the relative position of the sun\n",
	    "\tas it appears on the horizon.\n",
	    "<b>Show path:\n",
	    "\tuse dots to show the path of the sun across the sky\n",
	    "<b>Show apogee path:\n",
	    "\tuse dots to show the path the sun would take if it went\n",
	    "\t through the zenith at solar noon (it's highest path).\n",
	    "<b>Use orange sun:\n",
	    "\tuse the orange sun image instead of the blue image\n",
	    "<b>Show sun rise/set ETA:\n",
	    "\tshow ETA until sunrise/sunset below sunrise/sunset times\n",
	    "\t(this option is only available if you're using the orange sun)\n"
	};

	vbox = gkrellm_gtk_notebook_page(tabs, _("Info"));
	text = gkrellm_gtk_scrolled_text_view(vbox, NULL,
					      GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gkrellm_gtk_text_view_append_strings(text, info_text,
					     sizeof(info_text)/sizeof(gchar *));
    }

    /* ----------------- about text --------------------*/

    {
      gchar *plugin_about_text;
      GtkWidget *label, *text;

      plugin_about_text = g_strdup_printf(
		"SunClock %d.%d.%d\n"
		"GKrellM2 SunClock Plugin\n\n"
		"Copyright (C) 2001, 2002 Norman Walsh\n"
		"ndw@nwalsh.com\n\n"
		"Derived from MoonClock 0.3 Copyright (C) 2001 Dale P. Smith\n"
		"and wmSun 1.03 Copyright (C) 1999 Mike Hnderson\n\n"
		"Released under the GNU Public Licence",
		SUNCLOCK_MAJOR_VERSION,
		SUNCLOCK_MINOR_VERSION,
		SUNCLOCK_PATCH_VERSION);

      text = gtk_label_new(plugin_about_text);
      label = gtk_label_new("About");
      gtk_notebook_append_page(GTK_NOTEBOOK(tabs),text,label);
      g_free(plugin_about_text);
    }
}

static void
sun_apply_config()
{
    options.longitude = gtk_spin_button_get_value_as_int(
	GTK_SPIN_BUTTON(longitude_spin_button));

    options.latitude = gtk_spin_button_get_value_as_int(
	GTK_SPIN_BUTTON(latitude_spin_button));

    options.clock24 = GTK_TOGGLE_BUTTON(clock24_button)->active;
    options.showMoon = GTK_TOGGLE_BUTTON(showMoon_button)->active;
    options.showStar = GTK_TOGGLE_BUTTON(showStar_button)->active;
    options.showPath = GTK_TOGGLE_BUTTON(showPath_button)->active;
    options.show90Path = GTK_TOGGLE_BUTTON(show90Path_button)->active;
    options.showETA = GTK_TOGGLE_BUTTON(showETA_button)->active;
    options.orangeSun = GTK_TOGGLE_BUTTON(orangeSun_button)->active;
    options.debug = GTK_TOGGLE_BUTTON(debug_button)->active;

    /* Update the display and tooltip to reflect changes */
    update_sun_data( &sununit );
    redraw = 1;
    update_tooltip ( &sununit );
}

static void
sun_save_config (FILE *f)
{
    fprintf(f, "%s longitude %d\n", PLUGIN_CONFIG_KEYWORD, options.longitude);
    fprintf(f, "%s latitude %d\n", PLUGIN_CONFIG_KEYWORD, options.latitude);
    fprintf(f, "%s clock24 %d\n", PLUGIN_CONFIG_KEYWORD, options.clock24);
    fprintf(f, "%s showMoon %d\n", PLUGIN_CONFIG_KEYWORD, options.showMoon);
    fprintf(f, "%s showStar %d\n", PLUGIN_CONFIG_KEYWORD, options.showStar);
    fprintf(f, "%s showPath %d\n", PLUGIN_CONFIG_KEYWORD, options.showPath);
    fprintf(f, "%s show90Path %d\n", PLUGIN_CONFIG_KEYWORD, options.show90Path);
    fprintf(f, "%s showETA %d\n", PLUGIN_CONFIG_KEYWORD, options.showETA);
    fprintf(f, "%s orangeSun %d\n", PLUGIN_CONFIG_KEYWORD, options.orangeSun);
    fprintf(f, "%s debug %d\n", PLUGIN_CONFIG_KEYWORD, options.debug);
}

static void
sun_load_config (gchar *arg)
{
    gchar config[64], item[256];
    gint n;

    n = sscanf(arg, "%s %[^\n]", config, item);
    if (n != 2)
	return;

    if (strcmp(config, "longitude") == 0)
	sscanf(item, "%d\n", &(options.longitude));
    if (strcmp(config, "latitude") == 0)
	sscanf(item, "%d\n", &(options.latitude));
    if (strcmp(config, "clock24") == 0)
        sscanf(item, "%d\n", &(options.clock24));
    if (strcmp(config, "showMoon") == 0)
        sscanf(item, "%d\n", &(options.showMoon));
    if (strcmp(config, "showStar") == 0)
        sscanf(item, "%d\n", &(options.showStar));
    if (strcmp(config, "showPath") == 0)
        sscanf(item, "%d\n", &(options.showPath));
    if (strcmp(config, "show90Path") == 0)
        sscanf(item, "%d\n", &(options.show90Path));
    if (strcmp(config, "showETA") == 0)
        sscanf(item, "%d\n", &(options.showETA));
    if (strcmp(config, "orangeSun") == 0)
        sscanf(item, "%d\n", &(options.orangeSun));
    if (strcmp(config, "debug") == 0)
        sscanf(item, "%d\n", &(options.debug));
}

static GkrellmMonitor plugin_mon  = {
    "Sun Clock",		/* Name, for config tab.        */
    0,				/* Id,  0 if a plugin           */
    sun_create_plugin,		/* The create_plugin() function */
    sun_update_plugin,		/* The update_plugin() function */
    sun_create_tab,		/* The create_plugin_tab() config function */
    sun_apply_config,		/* The apply_plugin_config() function      */

    sun_save_config,		/* The save_plugin_config() function  */
    sun_load_config,		/* The load_plugin_config() function  */
    PLUGIN_CONFIG_KEYWORD,	/* config keyword                     */

    NULL,			/* Undefined 2  */
    NULL,			/* Undefined 1  */
    NULL,			/* private		*/

    MON_INSERT_AFTER|MON_CLOCK,	/* Insert plugin before this monitor.       */
    NULL,			/* Handle if a plugin, filled in by GKrellM */
    NULL			/* path if a plugin, filled in by GKrellM   */
};

#if defined(WIN32)
__declspec(dllexport) GkrellmMonitor *
gkrellm_init_plugin(win32_plugin_callbacks* calls)
#else
GkrellmMonitor *
gkrellm_init_plugin()
#endif
{
#if defined(WIN32)
  callbacks = calls;
  pwin32GK = callbacks->GK;
#endif

    options.longitude = 73;	/* Where I live! */
    options.latitude = 42;
    options.clock24 = 1;        /* Use a 24 hour clock */
    options.showMoon = 1;       /* Plot the moon */
    options.showStar = 1;       /* Plot the star */
    options.showPath = 0;       /* But not the path */
    options.show90Path = 0;     /* Or the apogee path */
    options.showETA = 0;        /* Or the rise/set ETA */
    options.orangeSun = 0;      /* And use the blue sun */
    options.debug = 0;          /* No debugging */

    style_id = gkrellm_add_meter_style(&plugin_mon, STYLE_NAME);
    pGK = gkrellm_ticks();

    sun_monitor = &plugin_mon;
    return &plugin_mon;
}
