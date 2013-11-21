/***********************************************************************
*  File:   xim.c
*  Author: Philip Thompson
*  $Date: 89/10/26 11:22:12 $
*  $Revision: 1.1 $
*  Purpose: To view a variety of images on X displays and make them
*       suitable for printing as bitmaps.  "Xim" displays images up to
*       24-bits deep with an 'ImageHeader' on 8-bit color and bitmap
*       displays.  Various dithering and halftoning techniques are used
*       to achieve this image portability.
*
*  Copyright (c) 1988  Philip R. Thompson
*                Computer Resource Laboratory (CRL)
*                Dept. of Architecture and Planning
*                M.I.T., Rm 9-526
*                Cambridge, MA  02139
*   This  software and its documentation may be used, copied, modified,
*   and distributed for any purpose without fee, provided:
*       --  The above copyright notice appears in all copies.
*       --  This disclaimer appears in all source code copies.
*       --  The names of M.I.T. and the CRL are not used in advertising
*           or publicity pertaining to distribution of the software
*           without prior specific written permission from me or CRL.
*   I provide this software freely as a public service.  It is NOT a
*   commercial product, and therefore is not subject to an an implied
*   warranty of merchantability or fitness for a particular purpose.  I
*   provide it as is, without warranty. This software was not sponsored,
*   developed or connected with any grants, funds, salaries, etc.
*
*   This software is furnished  only on the basis that any party who
*   receives it indemnifies and holds harmless the parties who furnish
*   it against any claims, demands, or liabilities connected with using
*   it, furnishing it to others, or providing it to a third party.
*
*   Philip R. Thompson (phils@athena.mit.edu)
***********************************************************************/
#ifndef lint
static char xim_rcs_id[] =
    "$Header: /usr.MC68020/home/ipw/adm/snoopy/src/etc/xim/RCS/xim.c,v 1.1 89/10/26 11:22:12 frew Exp $";
#endif lint

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <stdio.h>
#include "ImageHeader.h"

#define rnd(x)  ((int)((x)+0.5))    /* round off a float to an int */

typedef unsigned char  byte;

int dm4[4][4] = {
     0,  8,  2, 10,
    12,  4, 14,  6,
     3, 11,  1,  9,
    15,  7, 13,  5
};
int dm8[8][8] = {
     0, 32,  8, 40,  2, 34, 10, 42,
    48, 16, 56, 24, 50, 18, 58, 26,
    12, 44,  4, 36, 14, 46,  6, 38,
    60, 28, 52, 20, 62, 30, 54, 22,
     3, 35, 11, 43,  1, 33,  9, 41,
    51, 19, 59, 27, 49, 17, 57, 25,
    15, 47,  7, 39, 13, 45,  5, 37,
    63, 31, 55, 23, 61, 29, 53, 21
};
int dm16[16][16];

extern char  *malloc(), *calloc();
char *progName;
int  dither_bw(), fs_bw(), mfs_bw();
int  (*bw)() = fs_bw;
int  ditherFactor = 8;
int  threshold = 32767;
int  *dm = &(dm8[0][0]);
int  enhance = 0;
Bool  debug_flag = False;
Display  *dpy;
int  screen;
Window  root_win;
Visual  *visual = NULL;
u_long  blackpixel, whitepixel;

main(argc, argv)
int argc;
char **argv;
{
    register  i, j, k;
    register byte  *buffer, *grn_buf, *blu_buf, *icon_buf;
    unsigned  buf_size;
    int  icon_width, icon_height, iconfact;
    int  buf_width, buf_height, ncolors, nchannels, npics; 
    int  modN[256], divN[256];
    char  *win_name = NULL, *display_name = NULL;
    Bool  inverse_flag = False, mono_flag = False, cdither_flag = False;
    Bool  runlen_flag=False, newmap_flag = False;
	FILE  *in_fp = stdin, *popen(), *fopen();
	extern int pclose(), fclose();
	int (*closefunc)() = fclose;
    ImageHeader  p_head;
    XColor  colors[256], fg_color, bg_color;
    Window  image_win, icon_win;
    Colormap  colormap, GetColormap();
    XEvent  event;
    XExposeEvent  *expose;
    XCrossingEvent  *xcrossing;
    GC  image_gc, icon_gc;
    XGCValues  gc_val;
    XSetWindowAttributes  xswa;
    XImage  *image = NULL, *icon_image = NULL;
    XSizeHints  sizehints;
    XWMHints  wmhints;

    progName = argv[0];
    for (i=1; i < argc; i++) {
        char *ptr = NULL;
        if (strncmp(argv[i], "-dis", 4) == 0) {
            display_name = argv[++i];
            continue;
        }
        if (strcmp(argv[i], "-in") == 0) {   /* compressed file ? */
            ptr = rindex(argv[++i], '.');
            if (ptr && strcmp(ptr, ".Z") == 0) {
                char popen_cmd[80];
                (void)strcpy(popen_cmd, "zcat ");
                (void)strcat(popen_cmd, argv[i]);
                in_fp = popen(popen_cmd, "r");
				closefunc = pclose;
            } else
                in_fp = fopen(argv[i], "r");
            if (in_fp == NULL)
                error("Can't open input file: %s ", argv[i]);
            win_name = argv[i];
            continue;
        }
        if (strncmp(argv[i], "-mf", 3) == 0) {
            if ((ptr=rindex(argv[i],'=')) != NULL)
                (void)sscanf(ptr+1,"%d", &threshold);
            bw = mfs_bw;
            mono_flag = True;
            continue;
        }
        if (strncmp(argv[i], "-fs", 3) == 0) {      /* default */
            if ((ptr=rindex(argv[i],'=')) != NULL)
                (void)sscanf(ptr+1,"%d", &threshold);
            bw = fs_bw;
            mono_flag = True;
            continue;
        }
        if (strncmp(argv[i], "-dit", 4) == 0) {
            if ((ptr=rindex(argv[i],'=')) != NULL) {
                (void)sscanf(ptr+1,"%d", &ditherFactor);
                dm = &(dm4[0][0]);
            }
            bw = dither_bw;
            cdither_flag = True;
            continue;
        }
        if (strncmp(argv[i], "-en", 3) == 0) {
            if ((ptr=rindex(argv[i],'=')) != NULL)
                (void)sscanf(ptr+1,"%d", &enhance);
            else
                enhance = 9;
            continue;
        }
        if (strncmp(argv[i], "-ru", 3) == 0) {
            runlen_flag = True;
            continue;
        }
        if (strncmp(argv[i], "-re", 3) == 0) {
            inverse_flag = True;
            continue;
        }
        if (strncmp(argv[i], "-mo", 3) == 0) {
            mono_flag = True;
            continue;
        }
        if (strncmp(argv[i], "-cm", 3) == 0) {
            newmap_flag = True;
            continue;
        }
        if (strncmp(argv[i], "-de", 3) == 0) {
            debug_flag = True;
            continue;
        }
        error("Usage: %s [-in <file>][-display <host>][-reverse][-cmap]\
    \n[-mono][-dither[=4] | -mfs][-enhance[=1-9]][-runlen][-debug]\n",
          progName);
    }

    /*  Open the display & set defualts */
    if ((dpy = XOpenDisplay(display_name)) == NULL)
        error("Can't open display '%s'", XDisplayName(display_name));
    screen = XDefaultScreen(dpy);
    root_win = XDefaultRootWindow(dpy, screen);
    visual = XDefaultVisual(dpy, screen);
    blackpixel = XBlackPixel(dpy, screen);
    whitepixel = XWhitePixel(dpy, screen);
    if (XDisplayPlanes(dpy, screen) == 1)
        mono_flag = True;

    if (cdither_flag) {
        if (mono_flag) {
            cdither_flag = False;              /* mono flag wins */
            if ((ditherFactor != 4) && (ditherFactor != 8))
                error("Dither size must be 4 or 8 in monochrome.");
        } else
            if (ditherFactor < 2 || ditherFactor > 8)
                error("Dither level must be from 2 to 8 in color");
    }

    /* Read header and verify image file formats */
    if (fread((char *)&p_head, sizeof(ImageHeader), 1, in_fp) != 1)
        error("Unable to read file header");
    if (atoi(p_head.header_size) != sizeof(ImageHeader))
        error("Header size mismatch");
    if (atoi(p_head.file_version) != IMAGE_VERSION)
        error("Incorrect Image_file Version.");
    buf_width = atoi(p_head.image_width);
    buf_height = atoi(p_head.image_height);
    if (atoi(p_head.runlength))
        runlen_flag = True;
    (void)fprintf(stderr,"Author: %s\nDate: %s\n", p_head.author,
            p_head.date);
    (void)fprintf(stderr,"Size: %d x %d   ", buf_width, buf_height);
    ncolors = atoi(p_head.num_colors);
    nchannels = atoi(p_head.num_channels);
    npics = atoi(p_head.num_pictures);
    buf_size = (unsigned)(buf_width * buf_height);

    /* Get or make the color table.
    */
    if (nchannels >= 3 || cdither_flag) {
        (void)fprintf(stderr,"nchannels %d\n", nchannels);
        if (mono_flag)          /* make map of grey values 0-65535 */
            for (i=0; i < ncolors; i++) {
                colors[i].pixel = (u_long)i;
                colors[i].red = colors[i].green =
                colors[i].blue = (u_short)(i * 257);
            }
        else {            /* make color dither map with correct depth */
            for (i=ditherFactor; i > 1; i--)
                if ((i*i*i) <= XDisplayCells(dpy,screen))
                    break;
            ncolors = i*i*i;
            ditherFactor = i;
            make_dithermap(ditherFactor, colors, divN, modN, dm16);
        }
    } else {                    /* use provided colormap */
        (void)fprintf(stderr,"ncolors %d\n", ncolors);
        for (i=0; i < ncolors; i++) {
            colors[i].pixel = (u_long)i;
            colors[i].red = (u_short)(p_head.c_map[i][0] * 257);
            colors[i].green = (u_short)(p_head.c_map[i][1] * 257);
            colors[i].blue = (u_short)(p_head.c_map[i][2] * 257);
            colors[i].flags = DoRed | DoGreen | DoBlue;
        }
    }
    if (inverse_flag)
        for (i=0; i < ncolors; i++) {
            colors[i].red = ~colors[i].red;
            colors[i].green = ~colors[i].green;
            colors[i].blue = ~colors[i].blue;
        }

    /* Allocate and read the data buffer(s) */
    if ((buffer = (byte *)malloc(buf_size)) == NULL)
        error("Can't malloc() image buffer.");
    Read_image_buf(in_fp, buffer, &buf_size, runlen_flag);
    if (nchannels >= 3) {
        grn_buf = (byte *)malloc(buf_size);
        blu_buf = (byte *)malloc(buf_size);
        if (grn_buf == NULL || blu_buf == NULL)
            error("Can't malloc() channel buffers.");
        Read_image_buf(in_fp, grn_buf, &buf_size, runlen_flag);
        Read_image_buf(in_fp, blu_buf, &buf_size, runlen_flag);
    }

    /*  Allocate the icon with max. dimension of 100 or 50 */
    iconfact = rnd(buf_height/100.0) > rnd(buf_width/100.0) ?
        rnd(buf_height/100.0) : rnd(buf_width/100.0);
    if (iconfact == 0)
        iconfact = 1;
    if ((icon_width = buf_width/iconfact) % 2)  /* even for XPutImage */
        icon_width -= 1;
    icon_height = buf_height/iconfact;
    if (debug_flag)
        fprintf(stderr,"\nicon width %d  height %d  scale factor %d\n",
            icon_width, icon_height, iconfact);
    if ((icon_buf = (byte *)malloc((unsigned)icon_width*icon_height))
            == NULL)
        error("Can't malloc() icon buffer");

    /* process and store the image and icon.
    */
    if (mono_flag) {               /* make a bitmap image, but first */
        if (nchannels >= 3) {      /* convert to 8-bit grey values */
            for (i=0; i < buf_size; i++) 
                buffer[i] = (byte)GreyValue((u_short)buffer[i],
                    (u_short)grn_buf[i], (u_short)blu_buf[i]);
            if (npics <= 1) {
                free((char *)grn_buf);
                free((char *)blu_buf);
            }
        } else {
            ColorToBW(buffer, buf_size, colors, &ncolors);
        }
        if (bw == dither_bw)
            NormalizeDM(ditherFactor);
        for (i=0, k=0; i < icon_height; i++) /* sample image for icon */
            for (j=0; j < icon_width; j++, k++)
                icon_buf[k]= buffer[(i*buf_width+j)*iconfact];
        if (enhance) {
            edge_enhance(buffer, buf_width, buf_height, buf_size);
            edge_enhance(icon_buf, icon_width, icon_height,
                (unsigned)icon_width*icon_height);
        }
        /* Translate pixmap into a bitmap for monochrome display. */
        BWToBitmap(buffer, buf_width, buf_height, colors);
        BWToBitmap(icon_buf, icon_width, icon_height, colors);
        colormap = XDefaultColormap(dpy,screen);
        icon_image = XCreateImage(dpy, visual, 1, XYBitmap, 0,
                (char *)icon_buf, icon_width, icon_height, 8, 0);
    } else {                      /* use color image */
        if (nchannels >= 3) {
            if (enhance) {
                edge_enhance(buffer, buf_width, buf_height, buf_size);
                edge_enhance(grn_buf, buf_width, buf_height, buf_size);
                edge_enhance(blu_buf, buf_width, buf_height, buf_size);
            }
            rgb_dither(buffer, grn_buf, blu_buf, buf_width,
                    buf_height, ditherFactor, divN, modN);
            if (npics <= 1) {
                free((char *)grn_buf);
                free((char *)blu_buf);
            }
        } else if (cdither_flag)
            pixmap_dither(buffer, p_head.c_map, buf_width, buf_height,
                    ditherFactor, divN, modN);
        if (ncolors > 250)          /* Don't bother trying to fit */
            newmap_flag = True;     /* into default map, faster too */
        colormap = GetColormap(colors, ncolors, &newmap_flag, buffer,
            buf_size);
        icon_image = XCreateImage(dpy, visual, 8, ZPixmap, 0,
                (char *)icon_buf, icon_width, icon_height, 8, 0);
        for (i=0; i < icon_height; i++)
            for (j=0; j < icon_width; j++)
                XPutPixel(icon_image, j, i,
                    buffer[(i*buf_width+j)*iconfact]);
    }
    image = XCreateImage(dpy, visual, (mono_flag ? 1 : 8),
            (mono_flag ? XYBitmap : ZPixmap), 0, (char *)buffer,
            buf_width, buf_height, 8, 0);
    if (debug_flag)
        fprintf(stderr,"processed.\n");

    /* Get window attributes */
    xswa.event_mask = ExposureMask |ButtonPressMask |ColormapChangeMask|
        LeaveWindowMask | EnterWindowMask;
    xswa.background_pixel = blackpixel;
    xswa.border_pixel = whitepixel;
    xswa.colormap = colormap;
    xswa.cursor = XCreateFontCursor(dpy, XC_gumby);
    image_win = XCreateWindow(dpy, root_win, 0, 0,
        buf_width, buf_height, 5, XDefaultDepth(dpy,screen),
        InputOutput, visual, CWBackPixel |CWEventMask |CWCursor |
        CWBorderPixel |CWColormap, &xswa);
    xswa.event_mask = ExposureMask;
    icon_win = XCreateWindow(dpy, root_win, 0, 0,
        icon_width, icon_height, 1, XDefaultDepth(dpy,screen),
        InputOutput, visual, CWBackPixel | CWBorderPixel, &xswa);

    /* set window manager hints */
    sizehints.flags = PPosition | PSize | PMaxSize;
    sizehints.width = sizehints.max_width = buf_width;
    sizehints.height = sizehints.max_height = buf_height;
    sizehints.x = 0;
    sizehints.y = 0;
    XSetStandardProperties(dpy, image_win, "X Imager", win_name,
            None, argv, argc, &sizehints);
    wmhints.flags = IconWindowHint /* | IconPositionHint */;
    wmhints.icon_window = icon_win;
    wmhints.icon_x = XDisplayWidth(dpy,screen) - 200;
    wmhints.icon_y = 2;
    XSetWMHints(dpy, image_win, &wmhints);

    gc_val.function = GXcopy;
    gc_val.plane_mask = AllPlanes;
    gc_val.foreground = blackpixel;
    gc_val.background = whitepixel;
    image_gc = XCreateGC(dpy,image_win, GCFunction | GCPlaneMask |
        GCForeground | GCBackground, &gc_val);
    icon_gc = XCreateGC(dpy, icon_win, GCFunction | GCPlaneMask |
        GCForeground | GCBackground, &gc_val);

    XMapWindow(dpy, image_win);             /* Map the image window. */
    if ((newmap_flag) && (!mono_flag)) {
        XInstallColormap(dpy, colormap);
        if (ncolors >= XDisplayCells(dpy,screen)) {
            i = XDisplayCells(dpy,screen);
            bg_color.red = colors[i].red;
            bg_color.green = colors[i].green;
            bg_color.blue = colors[i].blue;
            fg_color.red = colors[++i].red;     /* force the last */
            fg_color.green = colors[i].green;   /* two colors and */
            fg_color.blue = colors[i].blue;     /* sacrifice cursor */
            XRecolorCursor(dpy, xswa.cursor, &fg_color, &bg_color);
        }
    }

    /* Select events to listen for  */
    XSelectInput(dpy, image_win, (ButtonPressMask | ColormapChangeMask |
        ExposureMask | LeaveWindowMask | EnterWindowMask));
    XSelectInput(dpy, icon_win, ExposureMask);

    if (debug_flag)
       fprintf(stderr,"While loop.\n");
    expose = (XExposeEvent *)&event;
    xcrossing = (XCrossingEvent *)&event;
    while (True) {          /* Set up a loop to maintain the image. */
        XNextEvent(dpy, &event);           /* Wait on input event. */
        switch((int)event.type) {
        int modulo;     /* Temporary var. for expose->x % 4 */
        case Expose:
            if (expose->window == icon_win) {
                XPutImage(dpy, icon_win, icon_gc, icon_image, 0, 0,
                    0, 0, icon_width, icon_height);
                break;
            }
            if (debug_flag)
                fprintf(stderr,
                "expose event x= %d y= %d width= %d height= %d\n",
                expose->x, expose->y, expose->width, expose->height);
            modulo = expose->x % 4;
            if ((modulo != 0) && (!mono_flag)) {
                expose->x -= modulo;
                expose->width += modulo;
            }
            if ((expose->width % 4 != 0) && (!mono_flag))
                expose->width += 4 - (expose->width % 4);
            XPutImage(dpy, image_win, image_gc, image,
                expose->x, expose->y, expose->x, expose->y,
                expose->width, expose->height);
            if (debug_flag)
                fprintf(stderr, "Actual expose: %d  %d  %d  %d\n",
                expose->x, expose->y, expose->width, expose->height);
            break;
        case ButtonPress:
            switch((int)event.xbutton.button) {
            case Button1:
                if (--npics > 0) {
                    Read_image_buf(in_fp,buffer, &buf_size,runlen_flag);
                    if (mono_flag)
                        BWToBitmap(buffer,buf_width,buf_height,colors);
                else if (cdither_flag)
                    pixmap_dither(buffer, p_head.c_map, buf_width,
                        buf_height, ditherFactor, divN, modN);
                    XPutImage(dpy, image_win, image_gc, image, 0, 0, 0,
                        0, image->width, image->height);
                } else
                    fprintf(stderr,"No more pictures in file.\n");
                break;
            case Button2:
                fprintf(stderr,"(x,y): %d %d\n",
					event.xbutton.x, event.xbutton.y);
                break;
            case Button3:
                (void)(*closefunc)(in_fp);
                if (newmap_flag)
                    XInstallColormap(dpy, XDefaultColormap(dpy,screen));
                XDestroyWindow(dpy, image_win);
                XDestroyWindow(dpy, icon_win);
                XCloseDisplay(dpy);
                exit(0);
            }
        case LeaveNotify:
            if (newmap_flag && (xcrossing->mode != NotifyGrab))
                XInstallColormap(dpy, XDefaultColormap(dpy,screen));
            break;
        case EnterNotify:
            if (newmap_flag && (xcrossing->mode != NotifyUngrab))
                XInstallColormap(dpy, colormap);
            break;
        case ColormapNotify:
                /* Don't do anything for now */
            break;
        default:
             fprintf(stderr,"Bad X event.\n");
        }
    }
}  /* end main */


extern Colormap GetColormap(colors, ncolors, newmap_flag, buf, bufsize)
XColor  colors[];
int  ncolors;
Bool  *newmap_flag;
register byte  *buf;
unsigned  bufsize;
{
    register i;
    Colormap cmap, cmap2;
    XColor qcolor;
    extern u_long FindColorValue();
    
    if (ncolors >= XDisplayCells(dpy,screen)) {
        ncolors = XDisplayCells(dpy,screen);
		*newmap_flag = True;
	}
    if (debug_flag)
        fprintf(stderr,"Colormap size %d\n", ncolors);

    if (*newmap_flag) {
        cmap = XCreateColormap(dpy, root_win, visual, AllocAll);
        XStoreColors(dpy, cmap, colors, ncolors);
    } else {
        cmap = XDefaultColormap(dpy, screen);
        for (i=0; i < ncolors; i++) {
            if (XAllocColor(dpy, cmap, &colors[i]) == 0) {
                fprintf(stderr,"Too many colors %d - new map made\n",i);
                cmap2 = XCopyColormapAndFree(dpy, cmap);
                *newmap_flag = True;
                for ( ; i < ncolors; i++)
                    if (XAllocColor(dpy, cmap2, &colors[i]) == 0)
                        fprintf(stderr,"Can't xalloc colormap %d.\n",i);
                cmap = cmap2;
                break;
            }
        }
        for (i=0; i < bufsize; i++)
            buf[i] = (byte)colors[buf[i]].pixel;
    }
    if (*newmap_flag) {
        whitepixel = FindColorValue(cmap, ncolors, 255, 255, 255);
        blackpixel = FindColorValue(cmap, ncolors, 0, 0, 0);
    }
    if (debug_flag) {
        fprintf(stderr,"white %lu   black %lu\n",whitepixel,blackpixel);
        for (i=0; i < ncolors; i++) {
            qcolor.pixel = (u_long)i;
            XQueryColor(dpy, cmap, &qcolor);
            fprintf(stderr,"color[%3d]: pix %3u r= %5u g= %5u b= %5u\n",
                i, qcolor.pixel, qcolor.red, qcolor.green, qcolor.blue);
        }
    }
    return(cmap);
}


/* Find the the closest color in the colormap.
*/
u_long FindColorValue(cmap, ncolors, red, green, blue)
Colormap cmap;
int ncolors, red, green, blue;
{
    register i, red2, blue2, green2;
    XColor qcolor;
    u_long value;
    long dist, least = 1e5;
    
    for (i=0; i < ncolors; i++) {
        qcolor.pixel = (u_long)i;
        XQueryColor(dpy, cmap, &qcolor);
        red2 = (int)qcolor.red / 257;   
        green2 = (int)qcolor.green / 257;   
        blue2 = (int)qcolor.blue / 257; 
        dist = ((red2 - red) * (red2 - red)) +
               ((green2 - green) * (green2 - green)) +
               ((blue2 - blue) * (blue2 - blue));
        if (dist == 0)
            return(qcolor.pixel);
        else if (dist < least) {
            least = dist;
            value = qcolor.pixel;
        }
    }
    return(value);
}

/* This edge enhancing is taken from the ACM Transaction on Graphics
*  Vol. 6, No. 4, October 1987.  Dot diffusion is not implemented.
*/
edge_enhance(buf, width, height, bufsize)
register byte *buf;
int width, height;
unsigned bufsize;
{
    register i, x, y, lbyt, hbyt, tmp;
    register byte *tbuf;
    register float phi;

    if (debug_flag)
        fprintf(stderr,"enhancing... ");
    if ((tbuf = (byte *)malloc(bufsize)) == NULL)
        error("Can't malloc() core for edge enhancementi.\n");
    bcopy((char *)buf, (char *)tbuf, (int)bufsize);

    lbyt = width-1;
    hbyt = width+1;
    i = lbyt;
    if (enhance == 9) {             /* is much faster, default */
        for (y=2; y < height; y++) {
            i += 2;
            for (x=2; x < width; x++,i++) {
                tmp = (9 * tbuf[i]) - (tbuf[i-hbyt] + tbuf[i-width]+
                 tbuf[i-lbyt] + tbuf[i-1] + tbuf[i+1] + tbuf[i+lbyt] +
                 tbuf[i+width] + tbuf[i+hbyt]);
                if (tmp > 255)
                    buf[i] = 255;
                else if (tmp < 0)
                    buf[i] = 0;
                else
                    buf[i] = (byte)tmp;
            }
        }
    } else {                        /* allows greater control */
        phi = enhance / 10.0;
        for (y=2; y < height; y++) {
            i += 2;
            for (x=2; x < width; x++,i++) {
                tmp = (tbuf[i-hbyt] + tbuf[i-width] + tbuf[i-lbyt] +
                    tbuf[i-1] + tbuf[i] + tbuf[i+1] +
                    tbuf[i+lbyt] + tbuf[i+width] + tbuf[i+hbyt]) / 9.0;
                tmp = rnd((tbuf[i]-(phi*tmp)) / (1.0-phi));
                if (tmp > 255)
                    buf[i] = 255;
                else if (tmp < 0)
                    buf[i] = 0;
                else
                    buf[i] = (byte)tmp;
            }
        }
    }
    free((char *)tbuf);
}

int *error1, *error2;

/* Grey to monochrome conversion. Performed in place - overwrites 
* the source pixmap and transforms it into a bitmap in the process.
*/
BWToBitmap(buf,width,height,map)
register byte  *buf;
int  width, height;
XColor  map[];
{
    register byte  *mbuffer, mvalue;   /* monochrome buffer */
    register byte  *mpbuffer;          /* monochrome pixel buffer */
    register  row, col, bit;

    if (debug_flag)
        fprintf(stderr,"BWToBitmap... ");
    error1 = (int *)malloc((unsigned)(width+1) * sizeof(int));
    error2 = (int *)malloc((unsigned)(width+1) * sizeof(int));
    mbuffer= (byte *)calloc((unsigned)width*height/8, sizeof(byte));
    if ((error1 == NULL) || (error2 == NULL) || (mbuffer == NULL))
        error("calloc() in BWToBitmap conversion");
    mpbuffer = mbuffer = buf;

    if (XBitmapBitOrder(dpy) == LSBFirst) {
        for (row=0; row < height; row++)
            for (col=0; col < width; ) {
                mvalue = 0x00;
                for (bit=0; (bit < 8) && (col < width); bit++,col++)
                    if ((*bw)(*mpbuffer++, map, col, row))
                        mvalue |= (0x01 << bit);    /*  for Vax */
                *mbuffer++ = mvalue;
            }
    } else {
        for (row=0; row < height; row++)
            for (col=0; col < width; ) {
                mvalue = 0x00;
                for (bit=0; (bit < 8) && (col < width); bit++,col++)
                    if ((*bw)(*mpbuffer++, map, col, row))
                        mvalue |= (0x80 >> bit);    /*  for RT, Sun  */
                *mbuffer++ = mvalue;
            }
    }
    free((char *)buf);
    buf = mbuffer;
    free((char *)error1);
    free((char *)error2);
}


/*************************
* code for dithering     *
*************************/
extern int dither_bw(pixel,map,count,line)
unsigned int pixel;
XColor map[]; 
register count, line;
{   
    if (map[pixel].red > dm[((line%ditherFactor)*ditherFactor) +
            (count%ditherFactor)])
        return(0);
    else
        return(1);
}


/*****************************
* code for floyd steinberg   *
*****************************/
/* ARGSUSED */
extern int fs_bw(pixel, map, count, line)
unsigned int pixel;
XColor map[]; 
register count;
{
    int  onoff, *te; 
    int  intensity, pixerr;

    if (count == 0) {
        te = error1;
        error1 = error2;
        error2 = te;
        error2[0] = 0;
    }  
    intensity = map[pixel].red + error1[count];
    if (intensity > 65535)
        intensity = 65535;
    else
        if (intensity < 0)
            intensity = 0;
    if (intensity < threshold) {
        onoff = 1;
        pixerr = intensity;
    } else {
        onoff = 0;
        pixerr = intensity - 65535;
    }
    error1[count+1] += (int)(3*pixerr)/8;
    error2[count+1] = (int)pixerr/4;
    error2[count] += (int)(3*pixerr)/8;
    return(onoff);
}


/***************************************
* code for modified floyd steinberg    *
****************************************/
/* ARGSUSED */
extern int mfs_bw(pixel,map,count,line)
unsigned int pixel;
XColor map[]; 
register count;
{
    int  onoff, *te;
    int  intensity, pixerr;

    if (count == 0) {
        te = error1;
        error1 = error2;
        error2 = te;
        error2[0] = 0;
    }  
    intensity = map[pixel].red + error1[count];
    if (intensity > 65535)
        intensity = 65535;
    else if (intensity < 0)
        intensity = 0;

    if (intensity < threshold) {
        onoff = 1;
        pixerr = threshold - intensity;
    }
    else {
        onoff = 0;
        pixerr = threshold - intensity;
    }
    error1[count+1] += (int)(3*pixerr)/8;
    error2[count+1] = (int)pixerr/4;
    error2[count] += (int)(3*pixerr)/8;
    return(onoff);
}


/* Map rg&b channels to 8 bits through dithering.  This color dithering
* code has been adapted and greatly simplified from the original work by
*   author: Spencer W. Thomas
*           Computer Science Dept. (cs.utah.edu)
*           University of Utah
* Copyright (c) 1986, University of Utah
*/
rgb_dither(red, grn, blu, width, height, levels, divN, modN)
register byte  *red, *grn, *blu;
int width, height, levels, divN[], modN[];
{
    register i, x, y, col=0, row=0, levelsq;

    levelsq = levels * levels;
    for (i=0, y=0; y < height; y++, row=y%16) {
        for (x=0; x < width; x++, col=x%16, i++) {
            red[i] = (byte)(
                (modN[red[i]] > dm16[col][row] ? divN[red[i]]+1 :
                divN[red[i]]) +
                (modN[grn[i]] > dm16[col][row] ? divN[grn[i]]+1 :
                divN[grn[i]]) * levels +
                (modN[blu[i]] > dm16[col][row] ? divN[blu[i]]+1 :
                divN[blu[i]]) * levelsq);
        }
    }
}

pixmap_dither(buf, cmap, width, height, levels, divN, modN)
register byte *buf;
byte cmap[256][3];
int width, height, levels, divN[], modN[];
{
    register x, y, col=0, row=0, levelsq;
    register byte red, grn, blu, *loc;

    levelsq = levels * levels;
    if (debug_flag)
        fprintf(stderr,"Pixmap_dither: levels = %d\n", levels);
    for (loc = buf, y=0; y < height; y++, row=y%16) {
        for (x=0; x < width; x++, col=x%16, loc++) {
            red = cmap[*loc][0];
            grn = cmap[*loc][1];
            blu = cmap[*loc][2];
            *loc = (byte)(
                (modN[red] > dm16[col][row] ? divN[red]+1 : divN[red]) +
                (modN[grn] > dm16[col][row] ? divN[grn]+1 : divN[grn])
                * levels +
                (modN[blu] > dm16[col][row] ? divN[blu]+1 : divN[blu])
                * levelsq);
        }
    }
}


/* Create a color map and dithering matrix for the specified
*  intensity levels.
*/
make_dithermap(levels, colors, divN, modN, magic)
int levels, divN[256], modN[256], magic[16][16];
XColor colors[];
{
    float N, magicfact;
    register i, j, k, l, levelsq, ncolors;
    
    levelsq = levels * levels;      /* squared */
    ncolors = levelsq * levels;     /* and cubed */
    N = 255.0 / (levels-1.0);       /* Get size of each step */

    /* Set up the color map entries.  */
    for (i=0; i < ncolors; i++) {
        colors[i].pixel = (u_long)i;
        colors[i].red = (u_short)(rnd((i%levels)*N)*257);
        colors[i].green = (u_short)(rnd(((i/levels)%levels)*N)*257);
        colors[i].blue = (u_short)(rnd(((i/levelsq)%levels)*N)*257);
        colors[i].flags = DoRed | DoGreen | DoBlue;
    }
    for (i=0; i < 256; i++) {
        divN[i] = (int)(i / N);
        modN[i] = i - (int)(N * divN[i]);
    }
    magicfact = (N - 2.0) / 16.0;
    for (i=0; i < 4; i++)
        for (j=0; j < 4; j++)
            for (k=0; k < 4; k++)
                for (l=0; l < 4; l++)
                    magic[4*k+i][4*l+j] = rnd(dm4[i][j] * magicfact +
                                        (dm4[k][l]/16.0) * magicfact);
    if (debug_flag) {
        fprintf(stderr,"Ncolors = %d   Levels = %d\n", ncolors,levels);
        for (i=0; i < 16; i++) {
            for (j=0; j < 16; j++)
                fprintf(stderr,"%4d", magic[i][j]);
            fprintf(stderr,"\n");
        }
        fprintf(stderr,"\n");
    }
}


/* Normalize dither matrix to 65535 maximum value.
*/
NormalizeDM(dimension)
int dimension;
{
    int i, matsize;
    float normalValue;

    matsize = dimension * dimension;
    normalValue = 65536.0 / (float)matsize;
    for (i=0; i < matsize; i++) {
        dm[i] = rnd(dm[i] * normalValue);
        if (debug_flag) {
            fprintf(stderr,"%8d", dm[i]);
            if  (((i+1) % dimension) == 0)
                fprintf(stderr,"\n");
        }
    }
}


/* Transformation from RGB to the Y (or luminence) factor from
*  YIQ encoding.
*/
int GreyValue(red, green, blue)
u_short red, green, blue;
{
    return(rnd((float)red * 0.30 + (float)green * 0.55 +
            (float)blue * 0.15));
}


ColorToBW(buf, bufsize, colors, ncolors)
register byte  *buf;
unsigned  bufsize;
XColor colors[];
int *ncolors;
{
    register unsigned  i;
    byte  tval[256];

    /* determine b/w intensity level and place in pixel */
    if (debug_flag)
        fprintf(stderr,"making grey pixmap ...\n");
    for (i=0; i < *ncolors; i++) {
        tval[i] = GreyValue(colors[i].red, colors[i].green,
                colors[i].blue) / 257;
        if (debug_flag)
            fprintf(stderr,
            "color[%3d] pix: %3u  r: %5u  g: %5u  b: %5u  new: %3u\n",
            i, colors[i].pixel, colors[i].red, colors[i].green,
            colors[i].blue, tval[i]);
    }
    for (i=0; i < bufsize; i++)
        buf[i] = tval[buf[i]];
    *ncolors = 256;
    for (i=0; i < *ncolors; i++) {
        colors[i].pixel = (u_long)i;
        colors[i].red = colors[i].green = colors[i].blue =
                (u_short)(i * 257);
        colors[i].flags = DoRed | DoGreen | DoBlue;
    }
}


Read_image_buf(infile, buf, bufsize, encoded)
FILE *infile;
register byte  *buf;
unsigned  *bufsize;
int  encoded;
{
    register int  i, runlen, nbytes;
    register unsigned  j;
    register byte *line;
    long  marker;
    
    if (debug_flag)
        fprintf(stderr,"Reading pixmap... ");
    if (!encoded) {
        j = fread((char *)buf, 1, (int)*bufsize, infile);
    } else {
        if ((line=(byte *)malloc((unsigned)BUFSIZ)) == NULL)
            error("Can't malloc() fread string.");
        /* Unrunlength encode data */
        marker = ftell(infile);
        j = 0;
        while (((nbytes=fread((char *)line, 1, BUFSIZ, infile)) > 0) &&
            (j < *bufsize)) {
            for (i=0; (i < nbytes) && (j < *bufsize); i++) {
                runlen = (int)line[i]+1;
                i++;
                while (runlen--)
                    buf[j++] = line[i];
            }
            marker += i;
        }
        /* return to the begining of the next image's bufffer */
        if (fseek(infile, marker, 0) == -1)
            error("Can't fseek to location in image buffer.");
        free((char *)line);
    }
    if (j != *bufsize) {
        fprintf(stderr,"%cUnable to complete pixmap: %u / %u (%d%%)\n",
            7, j, *bufsize, (int)(j*100.0 / *bufsize));
        *bufsize = j;
    }
}


/* VARARGS1 */
error(s1, s2)           /*  A most tragic and fatal error.  */
char *s1, *s2;
{
    extern int errno, sys_nerr;
    extern char *sys_errlist[];

    fprintf(stderr,"%c%s: Error =>\n%c", 7, progName, 7);
    fprintf(stderr, s1, s2);
    if ((errno > 0) && (errno < sys_nerr))
        fprintf(stderr, " (%s)", sys_errlist[errno]);
    fprintf(stderr, "\n");
    exit(1);
}


/* End of xim.c */
