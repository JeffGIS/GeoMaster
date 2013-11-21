/*
 *	rastool [-b line,samp] [-c line,samp] [-e line,samp]
 *	        [-n #lines,#samps]
 *	        [imagefile]
 *
 *	Display an image in Sun rasterfile format.  Image can be
 *	monochrome (ras_maptype = RMT_NONE) or possess an RGB
 *	color table (ras_maptype = RMT_EQUAL_RGB).  Image type
 *	must be RT_STANDARD; no encoded formats are currently
 *	supported to allow disk scrolling of images.  If image
 *	is read from a pipe, it must be small enough to scroll
 *	in memory.
 *
 *	Rastool will display a portion of a larger image defined by the
 *	option arguments.  The option format and semantics are taken from
 *	the IPW window command.
 *
 *	No more than two of the following are allowed:
 *
 *	-b	begin line, sample (default: beginning of input image)
 *	-c	center line, sample
 *	-e	end line, sample (default: end of input image)
 *	-n	#lines, #samples in output image (default: remainder of
 *		input image)
 *
 *	Nb. current version does not support disk scrolling.
 *
 *	Written by
 *
 *	Jud Harward
 *	Center for Remote Sensing
 *	Boston University
 *
 *	5 Aug 1988
 */

#include  <stdio.h>
#include  <string.h>
#include  <sys/file.h>
#include  <errno.h>
#include  <assert.h>
#include  <suntool/sunview.h>
#include  <suntool/canvas.h>
#include  <suntool/scrollbar.h>

#include  "rastool.h"

#define MIN(a,b)	((a<b) ? (a) : (b))

#define RT_BORDERWIDTH 5	/* suntool param, defined in tool_struct.h */
#define RT_MAXCANVAS   1152*900

#ifdef lint
/* VARARGS1 ARGSUSED */
Window
window_create(owner, type, attributes)
Window          owner;
int             type, attributes;
{
    return ((Window) 0);
}

/* VARARGS1 ARGSUSED */
int
window_set(window, attributes)
Window          window;
int             attributes;
{
    return (0);
}

#endif

enum access_type {
    SIMPLE, COMPLEX
};
typedef enum access_type Access_type;

static char     Stdin[] = "stdin";

static char    *rt_fname;	/* image file pathname */
static char    *rt_basename;	/* image file base name */
static int      rt_fd;		/* image file descriptor */
static struct rasterfile rt_rasfile;	/* image rasterfile struct */
static struct rasterfile *Rt_rf = &rt_rasfile;
static colormap_t rt_cmap;	/* image colormap */
static colormap_t *Rt_cmap = &rt_cmap;
static Frame    rt_frame;	/* tool frame */
static Rect     rt_frect;	/* last frame rect position */
static Canvas   rt_canvas;	/* image canvas */
static Pixrect *Rt_impix;	/* temp memory pixrect for image */
static Scrollbar rt_vsb;	/* canvas vertical scrollbar */
static long     rt_vsvs = -1;	/* pending vertical SCROLL_VIEW_START */
static Scrollbar rt_hsb;	/* canvas horizontal scrollbar */
static long     rt_hsvs = -1;	/* pending horizontal SCROLL_VIEW_START */
static int      rt_stripeht;	/* height of the frame title stripe */
static int      rt_scrollthick;	/* scrollbar thickness */
static char     rt_cmsname[CMS_NAMESIZE];	/* name of image colormap
						 * segment */
static Access_type rt_access;	/* scroll type: SIMPLE for sunview scrolling,
				 * COMPLEX for large image scrolling off disk */
static Rect     rt_rect;	/* image rectangle relative to the underlying
				 * rasterfile */
static Rect    *Rt_rect = &rt_rect;

extern char    *optarg;
extern int      optind;
extern int      opterr;

static Frame    make_frame();
static Access_type image_args();
static void     get_img_hdr();
static struct pr_pos getpos();
static void     check_dims();
static void     label_frame();
static void     load_image();
static Canvas   make_canvas();
static void     set_colors();
static void     init_image();
static void     rt_event_proc();
static void     rt_resize_proc();
static Notify_value rt_interpose_func();
static void     usage();

char           *sprintf();
long            strtol();

main(argc, argv)
int             argc;
char          **argv;
{

    rt_frame = make_frame(&argc, argv);
    rt_access = image_args(Rt_rf, Rt_rect, &rt_fd, argc, argv);
    load_image(&Rt_impix, Rt_cmap, Rt_rf, Rt_rect, rt_fd);
    rt_canvas = make_canvas(rt_frame, Rt_rect, rt_access);
    set_colors(rt_cmsname, rt_canvas, Rt_cmap);
    init_image(rt_frame, rt_canvas, Rt_impix, rt_access);

    window_main_loop(rt_frame);
}

static void
usage()
{
	(void) fprintf(stderr, "Usage:\trastool\t[-b line,samp] [-c line,samp] [-e line,samp]\n" );
	(void) fprintf(stderr, "\t\t[-n #lines,#samps]\n" );
	(void) fprintf(stderr, "\t\t[imagefile]\n" );
	exit(1);
}

/*
 *	make_frame( ptr_argc, argv )
 *
 *	Make the image tool frame and perform some initialization.
 *	Set
 *
 *		rt_stripeht,
 *		rt_hsb,
 *		rt_vsb,
 *		rt_scrollthick
 */
 
static          Frame
make_frame(acp, av)
int            *acp;
char          **av;
{
    Frame           frame;

    frame = window_create((Window) NULL, FRAME,
			  FRAME_ARGC_PTR_ARGV, acp, av,
			  0);
    if (frame == NULL)
	error("rastool", "Can't create frame");

/*
 *	The following is necessary to resize properly while leaving the
 *	image stationary.  See comments on rt_interpose_func().
 */
    if (notify_interpose_event_func(frame,
				    rt_interpose_func,
				    NOTIFY_SAFE) != NOTIFY_OK) {
	notify_perror("rastool(interpose event func)");
    }

    (void) window_set(frame,
		      WIN_CONSUME_PICK_EVENT,
		      WIN_MOUSE_BUTTONS,
		      0);

    rt_stripeht = (int) window_get(frame, WIN_TOP_MARGIN);

/*
 * Create horizontal and vertical scrollbars once, and then attach them
 * to the image canvas as needed.  A scrollbar doesn't have a thickness until
 * it is attached to a window.  SCROLLBAR is a workaround.  A scrollbar_get
 * on a pseudo-scrollbar SCROLLBAR gets you the default thickness.
 */
    rt_hsb = scrollbar_create(LINT_CAST(SCROLL_PLACEMENT), SCROLL_SOUTH,
			      SCROLL_LINE_HEIGHT, 10,
			      0);
    rt_vsb = scrollbar_create(LINT_CAST(SCROLL_PLACEMENT), SCROLL_EAST,
			      SCROLL_LINE_HEIGHT, 10,
			      0);
    if (rt_hsb == NULL || rt_vsb == NULL)
	error("rastool", "Can't create scrollbars");
    rt_scrollthick = (int) scrollbar_get((Scrollbar) SCROLLBAR,
					 SCROLL_THICKNESS);
    return (frame);
}

/*
 *	Access_type
 *	image_args( Rf, Sir, pfd, ac, av )
 *	struct rasterfile *rf;
 *	Rect	*Sir;
 *	int	*pfd;
 *	int	 ac;
 *	char   **av;
 *
 *	Process the non-suntool command line arguments and get the image
 *	file name.  Open the image file and place the file descriptor
 *	in the int pointed to by pfd.  Retrieve the rasterfile header
 *	and store it in the struct pointed to by Rf.  If a subimage is
 *	specified, enter its geometry in the rect struct pointed to by Sir.
 *	Return scrolling type (Access_type) SIMPLE for in memory sunview
 *	automatic scrolling, or COMPLEX for off disk scrolling for larger
 *	images.
 */
 
#define ARGSTRING	"b:c:e:n:"
#define hasval(aval)	(aval.x > 0 || aval.y > 0)
#define haspos(apos)	(apos.x >= 0 || apos.y >= 0)

static Access_type
image_args(Rf, Sir, pfd, ac, av)
struct rasterfile *Rf;
Rect           *Sir;
int            *pfd;
int             ac;
char          **av;
{
    int             c;
    int             siopts = 0;	/* number of subimage options */
    struct pr_pos   bpos, cpos, epos, npos;
    struct pr_size  nsize;
    Access_type     atype;

    opterr = 0;

    bpos.x = cpos.x = epos.x = -1;
    bpos.y = cpos.y = epos.y = -1;
    nsize.x = nsize.y = 0;

    while ((c = getopt(ac, av, ARGSTRING)) != EOF) {
	switch (c) {

	    case 'b':
		bpos = getpos(optarg);
		siopts++;
		break;

	    case 'c':
		cpos = getpos(optarg);
		siopts++;
		break;

	    case 'e':
		epos = getpos(optarg);
		siopts++;
		break;

	    case 'n':
		npos = getpos(optarg);
		nsize.x = npos.x;
		nsize.y = npos.y;
		siopts++;
		break;

	    case '?':
		usage();
		break;

	}
    }

/*   Check for image filename */
    if (optind > 0 && av[optind] != NULL) {
	rt_fname = av[optind];
	if ((rt_basename = strrchr(rt_fname, '/')) == NULL) {
	    rt_basename = rt_fname;
	} else {
	    rt_basename++;
	}
	if ((*pfd = open(rt_fname, O_RDONLY)) < 0) {
	    error("rastool", "can't open file %s", rt_fname);
	}
    } else {
	rt_basename = Stdin;
	*pfd = 0;
    }

    get_img_hdr(Rf, *pfd);

/*  Check for a subimage specification and compute its Rect struct */
    if (siopts) {
	if (siopts > 2)
	    usage();
	if (hasval(nsize)) {
	    Sir->r_width = nsize.x ? nsize.x : Rf->ras_width;
	    Sir->r_height = nsize.y ? nsize.y : Rf->ras_height;
	    if (haspos(bpos)) {
		Sir->r_left = bpos.x >= 0 ? bpos.x : 0;
		Sir->r_top = bpos.y >= 0 ? bpos.y : 0;
	    } else if (haspos(epos)) {
		Sir->r_left = (epos.x >= 0 ? epos.x + 1 : Rf->ras_width)
		    - Sir->r_width;
		Sir->r_top = (epos.y >= 0 ? epos.y + 1 : Rf->ras_height)
		    - Sir->r_height;
	    } else if (haspos(cpos)) {
		Sir->r_left = (cpos.x >= 0 ? cpos.x + 1 : (Rf->ras_width + 1) / 2)
		    - (Sir->r_width + 1) / 2;
		Sir->r_top = (cpos.y >= 0 ? cpos.y + 1 : (Rf->ras_height + 1) / 2)
		    - (Sir->r_height + 1) / 2;
	    }
	} else {
	    if (haspos(cpos)) {
		(void) fprintf(stderr,
			       "Usage: the -c option should be accompanied by the -n option\n");
	    } else if (haspos(bpos)) {
		Sir->r_left = bpos.x >= 0 ? bpos.x : 0;
		Sir->r_top = bpos.y >= 0 ? bpos.y : 0;
		Sir->r_width = (epos.x >= 0 ? epos.x + 1 : Rf->ras_width)
		    - Sir->r_left;
		Sir->r_height = (epos.y >= 0 ? epos.y + 1 : Rf->ras_height)
		    - Sir->r_top;
	    } else if (haspos(epos)) {
		Sir->r_left = 0;
		Sir->r_top = 0;
		Sir->r_width = epos.x + 1;
		Sir->r_height = epos.y + 1;
	    }
	}
    }
    if (rect_isnull(Sir)) {
	Sir->r_left = Sir->r_top = 0;
	Sir->r_width = Rf->ras_width;
	Sir->r_height = Rf->ras_height;
    }
    check_dims(Sir, Rf);
/*
    if (Sir->r_width * Sir->r_height > RT_MAXCANVAS)
	error("rastool", "Can't display this large an image");
*/
    atype = SIMPLE;

    label_frame(rt_basename, Sir, Rf);

    return (atype);
}


static struct pr_pos
getpos(cp)
char           *cp;
{
    char           *cp0;
    struct pr_pos   opos;

    cp0 = cp;
    opos.y = (int) strtol(cp0, &cp, 0);
    if (*cp != ',')
	usage();
    cp0 = ++cp;
    opos.x = (int) strtol(cp0, &cp, 0);
    if (*cp != '\0')
	usage();

    return (opos);
}


static void
check_dims(Sir, Irf)
Rect           *Sir;
struct rasterfile *Irf;
{
    if (Sir->r_width <= 0 ||
	Sir->r_height <= 0 ||
	Sir->r_left + Sir->r_width > Irf->ras_width ||
	Sir->r_top + Sir->r_height > Irf->ras_height) {
	error("rastool", "Bad subimage definition");
    }
}

static void
get_img_hdr( Irf, ifd )
struct rasterfile *Irf;
int             ifd;
{
    if (rt_load_hdr(ifd, Irf) != 0) {
	error("rastool", "Can't read header of rasterfile %s", rt_fname);
    }
    if (Irf->ras_magic != RAS_MAGIC)
	error("rastool", "Bad rasterfile %s", rt_fname);
}


static void
label_frame(basename, Sir, Irf)
char           *basename;
Rect           *Sir;
struct rasterfile *Irf;
{
    char            flabel[80];

    if (Sir->r_width != Irf->ras_width || Sir->r_height != Irf->ras_height) {
	(void) sprintf(flabel, "%s (%d x %d of %d x %d)",
		       basename, Sir->r_width, Sir->r_height,
		       Irf->ras_width, Irf->ras_height);
    } else {
	(void) sprintf(flabel, "%s (%d x %d)", basename, Irf->ras_width,
		       Irf->ras_height);
    }
    (void) window_set(rt_frame, FRAME_LABEL, flabel, 0);
}

/*
 *	Access_type
 *	load_image( Pipix, Icmap, Irf, Sir, ifd )
 *	Pixrect       **Pipix;
 *	colormap_t     *Icmap;
 *	struct rasterfile *Irf;
 *	Rect           *Sir;
 *	int             ifd;
 *
 *
 *	Read in the colormap, if present, into appropriate arrays and
 *	set the structure pointed to by Icmap accordingly.  Then read
 *	in the portion of the image specified by the rect struct pointed
 *	to by Sir into an appropriately sized memory pixrect and set the
 *	target of Pipix to point to it.  Determine scrolling mechanism
 *	based on image size.
 */
 
static void
load_image(Pipix, Icmap, Irf, Sir, ifd)
Pixrect       **Pipix;
colormap_t     *Icmap;
struct rasterfile *Irf;
Rect           *Sir;
int             ifd;
{
    Rtfile          Rtf;

    if (rt_load_map(ifd, Irf, Icmap) != 0) {
	error("rastool", "Can't read color map of rasterfile %s", rt_fname);
    }
/*
 *  Load whole image?
 */
    if (Sir->r_width == Irf->ras_width && Sir->r_height == Irf->ras_height) {
	if ((*Pipix = rt_load_img(ifd, Irf)) == NULL) {
	    error("rastool", "Can't read image from rasterfile %s", rt_fname);
	}
    } else {
/*
 *  No, partial image.  Is the file seekable (a regular file)?
 */
	if (rt_seekable(ifd)) {
	    if ((*Pipix = mem_create(Sir->r_width,
				     Sir->r_height,
				     Irf->ras_depth)) == NULL) {
		error("rastool", "Can't allocate pixrect for subimage");
	    }
	    Rtf = rt_read_setup(*Pipix, ifd, Irf);
	    if (rt_read_pskimg(Rtf, 0, 0, Sir) < 0)
		error("rastool",
		      "Can't read partial image from rasterfile %s",
		      rt_fname);

	} else {
/*
 *  No, must be a pipe.
 */
	    if ((*Pipix = rt_load_ppimg(ifd, Irf, Sir)) == NULL)
		error("rastool",
		      "Can't read partial image from rasterfile %s",
		      rt_fname);
	}

    }
    if (close(ifd) < 0) {
	syserror("load_image", "close");
    }
}

/*
 *	Create the image canvas.
 */
 
static          Canvas
make_canvas(frame, Sir, iaccess)
Frame           frame;
Rect           *Sir;
Access_type     iaccess;
{
    Canvas          canvas;

    if (iaccess == SIMPLE) {
	canvas = window_create(frame, CANVAS,
    /* Fix the size of the canvas so it doesn't track the default window size */
			       CANVAS_AUTO_EXPAND, FALSE,
			       CANVAS_AUTO_SHRINK, FALSE,
			       CANVAS_FIXED_IMAGE, TRUE,
			       CANVAS_WIDTH, Sir->r_width,
			       CANVAS_HEIGHT, Sir->r_height,

    /*
     * Don't make it a retained canvas until we have defined the color depth
     * of the canvas.  Otherwise suntools doesn't know the pixel depth for
     * the backing pixrect.
     */
			       CANVAS_RETAINED, FALSE,
			       0);
	if (canvas == NULL)
	    error("rastool", "Can't create image canvas");
    }
    return (canvas);
}

static void
set_colors(cmsname, canvas, cmap)
char           *cmsname;
Canvas          canvas;
colormap_t     *cmap;
{
    int             pid;
    Pixwin         *pw;

    pw = canvas_pixwin(canvas);
    if (cmap->length > 0) {
/* Generate a unique colormap name */
	pid = getpid();
	(void) sprintf(cmsname, "rt%d", pid);
	pw_setcmsname(pw, cmsname);
	pw_putcolormap(pw, 0,
		       cmap->length,
		       cmap->map[0],	/* red */
		       cmap->map[1],	/* green */
		       cmap->map[2]);	/* blue */
    }
/*
 *	Now that we have set up the colormap segment, we create the backing
 *	store by setting the canvas to have a retained Pixwin.  We then write
 *	the memory pixrect containing the image into the canvas and
 *	retained pixwin using init_image().  It would be more efficient
 *	to assign the memory pixrect to be the backing store, but there doesn't
 *	seem to be a way to do this.
 */

    (void) window_set(canvas,
		      CANVAS_RETAINED, TRUE,
		      0);
    if (pw->pw_prretained == NULL)
	error("rastool", "Can't create backing pixwin");
}


static void
init_image(frame, canvas, Impix, access)
Frame           frame;
Canvas          canvas;
Pixrect        *Impix;
Access_type     access;
{
    Pixwin         *pw;

    assert(access == SIMPLE);
    pw = canvas_pixwin(canvas);
    pw_write(pw, 0, 0,
	     Impix->pr_size.x,
	     Impix->pr_size.y,
	     PIX_SRC,
	     Impix, 0, 0);
    (void) pr_destroy(Impix);	/* free the original pixrect */
    
/*
 *	Unlike canvases, frames don't call their resize_procs on initial
 *	display so we have to do it explicitly.
 */
    rt_resize_proc(frame);
}

/*
 *	This function is necessary to resize properly from all four
 *	corners.  The Sunview default on a canvas resize is to take the upper
 *	corner of the current image and map it to the upper left corner of
 *	the resize frame.  But that is not what one wants if one moves in
 *	the upper left corner, say, to isolate the lower right corner.  What
 *	we want here is a resize where the image stays motionless, and only
 *	the portion of the underlying canvas that is visible changes.
 *
 *	This requires some finagling.  When one receives a resize event,
 *	one can only determine the new size of the frame.  I can find no way
 *	to determine which corner or side the mouse is against.  So, to
 *	determine from which corner or side we are resizing, we must log
 *	all changes in size AND position of the frame.  The only way I
 *	can find to track changes in position is to interpose on all
 *	mouse events.  There should be a more elegant way to do this.
 *
 *	The second problem is that there seems to be a race condition
 *	implicit in a scollbar_scroll_to() request processed during
 *	a resize event.  Sometimes it works properly, sometimes it doesn't.
 *	Since scroll requests are posted using the notifier, I see how
 *	this can happen, but feel it shouldn't.  My current solution is
 *	to trap and execute the requests during the WIN_REPAINT event
 *	that follows every resize event.  For some reason this appears to
 *	work reliably.  Voodoo_scroll().
 */
static          Notify_value
rt_interpose_func(frame, event, arg, type)
Frame           frame;
Event          *event;
Notify_arg      arg;
Notify_event_type type;
{
    Notify_value    value;
    Rect            frect;

    switch (event_id(event)) {

	case WIN_RESIZE:
	    value = notify_next_event_func(frame, event, arg, type);
	    rt_resize_proc(frame);
	    return (value);

	case WIN_REPAINT:
	    if (rt_hsvs >= 0) {
		scrollbar_scroll_to(rt_hsb, rt_hsvs);
		rt_hsvs = -1;
	    }
	    if (rt_vsvs >= 0) {
		scrollbar_scroll_to(rt_vsb, rt_vsvs);
		rt_vsvs = -1;
	    }
	    return (notify_next_event_func(frame, event, arg, type));

	case MS_LEFT:
	case MS_MIDDLE:
	case MS_RIGHT:

	/*
	 * Note we retrieve the frame rect after processing the mouse event.
	 * notify_next_event_func() will only return after a MOVE is complete
	 * if that is what caused the mouse hit.  We will also trap other
	 * events including a RESIZE.  On a RESIZE, the reported frame rect
	 * will be the post resize rect.  The RESIZE event is reported
	 * separately and after the mouse event.  Since rt_frect is meant to
	 * capture the pre resize last position of the frame rect, we do not
	 * record the frame position if it results from a RESIZE.
	 */
	    value = notify_next_event_func(frame, event, arg, type);
	    frect = *(Rect *) window_get(frame, WIN_RECT);
	    if (frect.r_width == rt_frect.r_width &&
		frect.r_height == rt_frect.r_height) {
		rt_frect.r_left = frect.r_left;
		rt_frect.r_top = frect.r_top;
	    }
	    return (value);

	default:
	    return (notify_next_event_func(frame, event, arg, type));

    }
}

static void
rt_resize_proc(frame)
Frame           frame;
{
    Scrollbar       hbar;	/* horizontal scrollbar */
    long            hsvs;	/* horizontal temp for SCROLL_VIEW_START */
    Scrollbar       vbar;	/* vertical scrollbar */
    long            vsvs;	/* vertical temp for SCROLL_VIEW_START */
    Rect            frect;	/* frame size and position */
    short           f_width;	/* post resize internal width of frame */
    short           f_height;	/* post resize internal height of frame */
    short           c_width;	/* viewable width of canvas allowing for sb */
    short           c_height;	/* viewable height of canvas allowing for sb */
    short           vscroll;	/* TRUE if we need a vertical scrollbar */
    short           hscroll;	/* TRUE if we need a horizontal scrollbar */
    short           resize_dleft;	/* delta of left edge of frame on
					 * resize */
    short           resize_dtop;/* delta of top edge of frame on resize */

/* If frame is iconic, do nothing */
    if ((int) window_get(frame, FRAME_CLOSED))
	return;

/*
 * If resize event was generated by a previous call of this routine
 * rather than by a user action, do nothing.
 */
    frect = *(Rect *) window_get(frame, WIN_RECT);
    if ((frect.r_width == rt_frect.r_width) &&
	(frect.r_height == rt_frect.r_height)) {
	return;
    }
/*
 * If this is the initial call to sync the frame size, i.e. not routed
 * through the notifier, then rt_frect should be set to the current
 * frame rect so that there are no resize deltas.
 */
    if (rt_frect.r_width == 0) {
	rt_frect = frect;
    }
/*
 * vbar and hbar will each either be NULL or the same as rt_hsb/vsb
 */
    vbar = (Scrollbar) window_get(rt_canvas, WIN_VERTICAL_SCROLLBAR);
    hbar = (Scrollbar) window_get(rt_canvas, WIN_HORIZONTAL_SCROLLBAR);

    f_width = frect.r_width - 2 * RT_BORDERWIDTH;
    f_height = frect.r_height - rt_stripeht - RT_BORDERWIDTH;
    hscroll = (f_width < Rt_rect->r_width) ? 1 : 0;
    vscroll = (f_height < Rt_rect->r_height) ? 1 : 0;
    c_width = f_width - vscroll * rt_scrollthick;
    c_height = f_height - hscroll * rt_scrollthick;
    resize_dleft = frect.r_left - rt_frect.r_left;
    resize_dtop = frect.r_top - rt_frect.r_top;

    if (hscroll) {		/* Do we need a horizontal scrollbar? */

    /*
     * Yes.  Calculate the SCROLL_VIEW_START. Nb. that Rt_rect->r_width is
     * the same as SCROLL_OBJECT_LENGTH for the horizontal scrollbar.
     * Scroll will be processed during the following WIN_REPAINT event to
     * avoid race condition. See rt_interpose_proc().
     */
	hsvs = (long) scrollbar_get(rt_hsb, SCROLL_VIEW_START) + resize_dleft;
	rt_hsvs = MIN(hsvs, Rt_rect->r_width - c_width);
	if (rt_hsvs < 0)
	    rt_hsvs = 0;
	rt_frect.r_width = frect.r_width;

	if (!hbar) {
	/* If it is not already attached, attach it. */
	    (void) window_set(rt_canvas, WIN_HORIZONTAL_SCROLLBAR, rt_hsb, 0);
	}


    } else {			/* No horizontal scrollbar needed */

	if (hbar) {		/* Is one attached anyway? */
	/* Yes.  Home the scroll and detach it. */
	    scrollbar_scroll_to(rt_hsb, (long) 0);
	    (void) window_set(rt_canvas, WIN_HORIZONTAL_SCROLLBAR, 0, 0);
	    rt_hsvs = -1;
	}

    /*
     * Since scrolling is not necessary, frame may be too large for image.
     * Adjust it.
     */
	(void) window_set(rt_canvas,
			  WIN_WIDTH, Rt_rect->r_width,
			  0);
	rt_frect.r_width = Rt_rect->r_width + 2 * RT_BORDERWIDTH;
	(void) window_set(frame,
			  WIN_WIDTH, rt_frect.r_width,
			  0);
    }

    if (vscroll) {		/* Do we need a vertical scrollbar? */

    /*
     * Yes.  Calculate the SCROLL_VIEW_START. Nb. that Rt_rect->r_height is
     * the same as SCROLL_OBJECT_LENGTH for the vertical scrollbar.  Scroll
     * will be processed during the following WIN_REPAINT event to
     * avoid race condition. See rt_interpose_proc().
     */
	vsvs = (long) scrollbar_get(rt_vsb, SCROLL_VIEW_START) + resize_dtop;
	rt_vsvs = MIN(vsvs, Rt_rect->r_height - c_height);
	if (rt_vsvs < 0)
	    rt_vsvs = 0;
	rt_frect.r_height = frect.r_height;

	if (!vbar) {
	/* If it is not already attached, attach it. */
	    (void) window_set(rt_canvas, WIN_VERTICAL_SCROLLBAR, rt_vsb, 0);
	}


    } else {			/* No vertical scrollbar needed */

	if (vbar) {		/* Is one attached anyway? */
	/* Yes.  Home the scroll and detach it. */
	    scrollbar_scroll_to(rt_vsb, (long) 0);
	    (void) window_set(rt_canvas, WIN_VERTICAL_SCROLLBAR, 0, 0);
	    rt_vsvs = -1;
	}

    /*
     * Since scrolling is not necessary, frame may be too large for image.
     * Adjust it.
     */
	(void) window_set(rt_canvas,
			  WIN_HEIGHT, Rt_rect->r_height,
			  0);
	rt_frect.r_height = Rt_rect->r_height + rt_stripeht + RT_BORDERWIDTH;
	(void) window_set(frame,
			  WIN_HEIGHT, rt_frect.r_height,
			  0);
    }

    rt_frect.r_left = frect.r_left;
    rt_frect.r_top = frect.r_top;
}

#ifndef	lint
static char     rcsid[] = "$Header: /home/ipw/src/etc/rastool/RCS/rastool.c,v 1.1 90/01/31 13:31:40 frew Exp $";

#endif
