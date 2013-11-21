#include  <assert.h>
#include  <stdio.h>
#include  <sys/types.h>
#include  <sys/file.h>
#include  <sys/stat.h>
#include  <suntool/sunview.h>

#include  "rastool.h"

#define PIPESIZE	4096
#define	rt_after(a,b)	(a.y>b.y || (a.y==b.y && a.x>b.x))
#define	rt_before(a,b)	(a.y<b.y || (a.y==b.y && a.x<b.x))
#define MIN(a,b)	((a<b) ? (a) : (b))
#define	byt_to_pix(x,d)	(d == 1 ? (x) << 3 : (x))
#define	pix_to_byt(x,d)	(d == 1 ? ((x) + 7) >> 3 : (x))

typedef struct {
    Pixrect        *pr;		/* destination pixrect */
    int             fd;		/* rasterfile file descriptor */
    long            fpos;	/* rasterfile file position */
    struct rasterfile *rfile;	/* ptr to rasterfile hdr */
    int             imofft;	/* offt of img from strt of rstrfile */
    long            fsize;	/* size in bytes of rasterfile */
    int             linebytes;	/* # of bytes in line of rstrfile */
    Pixrect        *dpr;	/* current destination region */
    Pixrect        *slpr;	/* scanline pixrect buffer */
    int             bufbytes;	/* # of bytes in scanline pixrect buffer */
}               rtfinfo, *Rtfinfo;

typedef struct {
    long            offset;
    int             skew;
}               rtfpos, *Rtfpos;

char           *malloc();
long            lseek();

static void     rt_pos_advance();
static          rt_read_rect();
static          rt_read_slines();
static rtfpos   rt_file_pos();
static struct pr_pos rt_image_pos();

/*
 *	These routines duplicate and extend the functions of the pixrect
 *	file I/O routines, pr_dump* and pr_load*.  They only work with
 *	with rasterfiles of type RT_STANDARD and colormap types RMT_NONE
 *	or RMT_EQUAL_RGB.  In addition to providing the standard pixrect
 *	functions, these routines will eventually support scrolling
 *	of images off disk.  They therefore assume that they are reading
 *	uncompressed images.  Byte encoded format is not supported.
 *	The basic routines (rt_load_{hdr,map,img} will read from standard
 *	input as well as an open file.  Thus, as with the pixrect file
 *	I/O routines, they should be called in the proper order.
 */

/*
 *	The analogue of pr_load_header().
 */
rt_load_hdr(rfd, Rf)
int             rfd;
struct rasterfile *Rf;
{
    int             nread;

    nread = read(rfd, (char *) Rf, sizeof(struct rasterfile));
    if (nread < 0)
	syserror("rt_load_hdr", "read");
    else if (nread != sizeof(struct rasterfile))
	return (-1);
    else if (Rf->ras_magic != RAS_MAGIC)
	return (-1);

    return (0);
}

/*
 *	The analogue of pr_load_colormap.
 */
rt_load_map(rfd, Rf, Rcmap)
int             rfd;
struct rasterfile *Rf;
colormap_t     *Rcmap;
{
    int             nread;
    u_char         *rgbmap;

    if (!Rcmap)
	return (0);

    if (Rf->ras_type != RT_STANDARD) {
	return (-1);
    }
    switch (Rf->ras_maptype) {

	case RMT_NONE:
	    return (0);

	case RMT_EQUAL_RGB:
	    rgbmap = (u_char *) malloc((unsigned) Rf->ras_maplength);
	    if (rgbmap == NULL) {
		error("rtio", "Can't get memory for colormap");
	    }
	    nread = read(rfd, (char *) rgbmap, Rf->ras_maplength);
	    if (nread < 0)
		syserror("rt_load_map", "read");
	    else if (nread != Rf->ras_maplength)
		return (-1);
	    else {
		Rcmap->length = Rf->ras_maplength / 3;
		Rcmap->map[0] = rgbmap;
		Rcmap->map[1] = rgbmap + Rcmap->length;
		Rcmap->map[2] = Rcmap->map[1] + Rcmap->length;
		return (0);
	    }

	default:
	    return (-1);
    }
}

/*
 *	The analogue of pr_load_image.
 */
Pixrect        *
rt_load_img(rfd, Rf)
int             rfd;
struct rasterfile *Rf;
{
    long            nread;
    int             toberead;
    Pixrect        *Impix;
    register char  *rptr;
    short          *idata;

    if (Rf->ras_type != RT_STANDARD) {
	return (NULL);
    }
/*
 *	Instead of using mem_create(), which zeroes out the memory pixrect,
 *	allocate the image area ourselves, read the image data in, and use
 *	mem_point to incorporate it in a memory pixrect.
 */
    toberead = Rf->ras_length;
    if ((idata = (short *) malloc((unsigned) toberead)) == NULL)
	return (NULL);
    rptr = (char *) idata;

    do {
	nread = read(rfd, rptr, toberead);
	if (nread < 0)
	    syserror("rt_load_img", "read");
	rptr += nread;
	if (nread == 0)
	    break;
    } while ((toberead -= nread) > 0);

    if (toberead == 0) {
	Impix = mem_point(Rf->ras_width,
			  Rf->ras_height,
			  Rf->ras_depth,
			  idata);
	if (!Impix) {
	    free((char *) idata);
	    return (NULL);
	}
/*
 *	Set flag so that the image will be freed by a later pr_destroy()
 */
	mpr_d(Impix)->md_primary = 1;
	return (Impix);
    } else {
/*
 *	Wasn't able to read in whole image.  Problem somewhere
 */
	free((char *) idata);
	return (NULL);
    }
}

/*
 *	rt_seekable(fd)
 *	int  fd;
 *
 *	Return TRUE if the open file descriptor fd refers to a seekable
 *	(regular) file and FALSE if it refers to a pipe or a FIFO.
 *	Anything else causes an error return.
 */
rt_seekable(fd)
int             fd;
{
    struct stat     statb;

    if (fstat(fd, &statb)) {
	syserror("rt_seekable", "fstat");
    } else {
	if ((statb.st_mode & S_IFMT) == S_IFREG)
	    return (TRUE);
	else if ((statb.st_mode & S_IFMT) == S_IFIFO)
	    return (FALSE);
	else
	    error("rtio", "Unknown file type: %o",
		  statb.st_mode & S_IFMT);
    }
/* NOTREACHED */
}

/*
 *	Pixrect		*
 *	rt_load_ppimg( rfd, Rf, Sir )
 *	int	 fd;
 *	struct rasterfile  *Rf;
 *	Rect	*Sir;
 *
 *	Load a portion of the rasterfile image described by the header pointed
 *	to by Rf and open on the pipe with file descriptor fd.  No seeks
 *	are used while reading in the image.  The Rect struct pointed to
 *	by Sir specifies what portion of the rasterfile to load.  The routine
 *	assumes the file pointer is set to the first byte of image data.
 *	This routines creates the necessary memory pixrect and returns a
 *	pointer to it or NULL on error.
 */

Pixrect        *
rt_load_ppimg(rfd, Rf, Sir)
int             rfd;
struct rasterfile *Rf;
Rect           *Sir;
{
    unsigned        simsize;
    int             rf_linebytes;
    int             rf_linepixels;
    int             toberead;
    int             nread;
    int             npixels;
    int             spixel;
    int             dx, dy;
    short          *idata;
    struct pr_pos   slbstart;
    struct pr_pos   slbend;
    struct pr_pos   simnext;
    struct pr_pos   imnext;
    rtfpos          rfpos;
    rtfpos          endpos;
    char            slbuffer[PIPESIZE];
    Pixrect        *Ipix;
    Pixrect        *Slpix;

    rf_linebytes = mpr_linebytes(Rf->ras_width, Rf->ras_depth);
    rf_linepixels = byt_to_pix(rf_linebytes, Rf->ras_depth);
    simsize = mpr_linebytes(Sir->r_width, Rf->ras_depth) * Sir->r_height;

    if ((idata = (short *) malloc(simsize)) == NULL)
	return (NULL);
    Ipix = mem_point(Sir->r_width, Sir->r_height, Rf->ras_depth, idata);
    if (Ipix == NULL) {
	free((char *) idata);
	return (NULL);
    }
    Slpix = mem_point(byt_to_pix(PIPESIZE, Rf->ras_depth),
		      1, Rf->ras_depth, (short *) slbuffer);
    if (Slpix == NULL) {
	(void) pr_close(Ipix);
	free((char *) idata);
	return (NULL);
    }
    toberead = rf_linebytes * (Sir->r_top + Sir->r_height - 1) +
	pix_to_byt(Sir->r_left + Sir->r_width, Rf->ras_depth);
    rfpos.offset = 0;
    rfpos.skew = 0;
    simnext.x = 0;
    simnext.y = 0;
    imnext.x = Sir->r_left;
    imnext.y = Sir->r_top;

    do {
	if ((nread = read(rfd, slbuffer, PIPESIZE)) < 0) {
	    syserror("rt_load_ppimg", "read");
	}
	toberead -= nread;

	slbstart = rt_image_pos(Rf, rf_linebytes, rfpos);
	rfpos.offset += nread;
	endpos.offset = rfpos.offset - 1;
	endpos.skew = (Rf->ras_depth == 1 ? 7 : 0);
	slbend = rt_image_pos(Rf, rf_linebytes, endpos);

	while (!rt_after(imnext, slbend)) {
	    dx = Sir->r_width - simnext.x;
	    npixels = (imnext.y == slbend.y) ?
		MIN(slbend.x - imnext.x + 1, dx) : dx;
	    dy = imnext.y - slbstart.y;
	    spixel = dy * rf_linepixels + imnext.x - slbstart.x;
	    (void) pr_rop(Ipix,
			  simnext.x,
			  simnext.y,
			  npixels,
			  1,
			  PIX_SRC | PIX_DONTCLIP,
			  Slpix,
			  spixel,
			  0);
	    rt_pos_advance(&simnext, Sir, npixels);
	    imnext.x = simnext.x + Sir->r_left;
	    imnext.y = simnext.y + Sir->r_top;
	}
    } while (toberead > 0);

    (void) pr_close(Slpix);
    return (Ipix);
}


/*
 *	static struct pr_pos
 *	rt_image_pos( Rf, linebytes, ipos )
 *	struct rasterfile	*Rf;
 *	int			 linebytes;
 *	rtfpos			 ipos;
 *
 *	Convert a displacement in a rasterfile image to coordinates
 *	relative to the rasterfile image.  linebytes specifies the number
 *	of bytes in a scanline of the rasterfile.  Example:
 *
 *		Rf->ras_width = 16;
 *		Rf->ras_height = 20;
 *		Rf->ras_depth = 1;
 *		linebytes = 2;
 *		ipos.offset = 8;
 *		ipos.skew = 2;
 *	will return
 *		ret.x = 2;
 *		ret.y = 4;
 *
 *	Note that the displacement is calculated from the start of the
 *	image, not from the start of the rasterfile as in rt_file_pos().
 */

static struct pr_pos
rt_image_pos(Rf, linebytes, ipos)
struct rasterfile *Rf;
int             linebytes;
rtfpos          ipos;
{
    struct pr_pos   rfp;

    rfp.y = ipos.offset / linebytes;
    rfp.x = byt_to_pix(ipos.offset - rfp.y * linebytes, Rf->ras_depth)
	+ ipos.skew;
    return (rfp);
}

/*
 *	Rtfile
 *	rt_read_setup(Ipix, rfd, Rf)
 *	Pixrect           *Ipix;
 *	int                rfd;
 *	struct rasterfile *Rf;
 *
 *	rt_read_setup() associates a rasterfile open on file descriptor
 *	rfd and a memory pixrect pointed to by Ipix to prepare for future
 *	random access reads of rectangular regions.  Rf points to the
 *	rasterfile header structure already in core.  Routine returns an
 *	opaque structure that contains bookkeeping information.  Storage is
 *	released by calling rt_shutdown().  Routine assumes that the file
 *	pointer is positioned at the start of the image data.
 */

Rtfile
rt_read_setup(Ipix, rfd, Rf)
Pixrect        *Ipix;
int             rfd;
struct rasterfile *Rf;
{
    register Rtfinfo Rti;
    int             bufpixels;

    if ((Rti = (Rtfinfo) malloc(sizeof(rtfinfo))) == NULL) {
	error("rtio", "Can't get storage for an Rtfile");
    }
    Rti->pr = Ipix;
    Rti->fd = rfd;
    Rti->rfile = Rf;
    Rti->imofft = sizeof(struct rasterfile) + Rf->ras_maplength;
    Rti->fpos = Rti->imofft;
    Rti->fsize = Rti->imofft + pix_to_byt(Rf->ras_width * Rf->ras_height,
					  Rf->ras_depth);
    Rti->linebytes = mpr_linebytes(Rf->ras_width, Rf->ras_depth);
    Rti->bufbytes = Rti->linebytes + BUFSIZ;

    bufpixels = byt_to_pix(Rti->bufbytes, Rf->ras_depth);
    if ((Rti->slpr = mem_create(bufpixels, 1, Rf->ras_depth)) == NULL) {
	error("rtio", "Can't create scanline pixrect");
    }
/*
 *	Create a dummy destination region initialized to the whole
 *	destination pixrect.  The destination region will be adjusted
 *	on each call to rt_read_rect().
 */
    if ((Rti->dpr = pr_region(Ipix, 0, 0, Ipix->pr_size.x, Ipix->pr_size.y))
	== NULL) {
	error("rtio", "Can't create subregion");
    }
    return ((Rtfile) Rti);
}


void
rt_shutdown(Rtf)
Rtfile          Rtf;
{
    register Rtfinfo Rti;

    Rti = (Rtfinfo) Rtf;
    (void) pr_close(Rti->dpr);
    (void) pr_destroy(Rti->slpr);
    free(Rtf);
}

/*
 *	rt_read_pskimg( dx, dy, Rtf, Srect )
 *	int       dx;
 *	int       dy;
 *	Rtfile    Rtf;
 *	Rect     *Srect;
 *
 *	Read from a seekable file the subimage defined by the Rect
 *	structure pointed to by Srect and place it in the memory pixrect
 *	pointed to by Ipix with its upper left corner at coordinates,
 *	dx, dy.  The file must have been initialized by a previous call
 *	to rt_read_setup().  Rtf is the handle returned by
 *	rt_read_sk_setup() that identifies the file.  Return -1 on error,
 *	0 otherwise.
 */

rt_read_pskimg(Rtf, dx, dy, Srect)
Rtfile          Rtf;
int             dx;
int             dy;
Rect           *Srect;
{
    register Rtfinfo Rti;

    Rti = (Rtfinfo) Rtf;

/*
 *  If subimage is the full width of the rasterfile and the same width as
 *  the image pixrect, simply read the appropriate number of scanlines
 *  into the image pixrect.
 */
    if (Srect->r_width == Rti->rfile->ras_width &&
	Srect->r_width == Rti->pr->pr_size.x) {
	assert(dx == 0);
	return (rt_read_slines(Rtf,
			       dy,
			       Srect->r_height,
			       Srect->r_top));

    } else {
/*
 *  The general case, made messy by the following factors:
 *	1) we must deal with the monochrome as well as the 8 bit pixel case,
 *	2) the subimage may not be as wide as the rasterfile, and
 *	3) the subimage may not be as wide as the destination pixrect.
 */
	return (rt_read_rect(Rtf, dx, dy, Srect));
    }
}

/*
 *	rt_read_slines(Rtf, dy, nlines, sy)
 *	Rtfile	Rtf;
 *	int	dy;
 *	int	nlines;
 *	int	sy;
 *
 *	Read nlines complete scanlines from a rasterfile into a memory
 *	pixrect of the same width.  The rasterfile and pixrect have
 *	already been designated by a call to rt_read_setup().  dy and sy
 *	specify the top destination and source lines respectively.  Returns
 *	-1 on error and 0 otherwise.
 */

static
rt_read_slines(Rtf, dy, nlines, sy)
Rtfile          Rtf;
int             dy;
int             nlines;
int             sy;
{
    long            spos;
    int             nread, toberead;
    register Rtfinfo Rti;

    Rti = (Rtfinfo) Rtf;
    assert(Rti->linebytes == mpr_d(Rti->pr)->md_linebytes);

    spos = Rti->imofft + Rti->linebytes * sy;
    if (Rti->fpos != spos) {
	if (lseek(Rti->fd, spos, L_SET) < 0) {
	    syserror("rt_read_slines", "full scanline lseek");
	}
    }
    toberead = Rti->linebytes * nlines;
    nread = read(Rti->fd,
		 (char *) mpr_d(Rti->pr)->md_image + dy * Rti->linebytes,
		 toberead);
    Rti->fpos += nread;

    if (nread < 0) {
	syserror("rt_read_slines", "full scanline read");
    } else if (nread != toberead) {
	return (-1);
    } else
	return (0);
/* NOTREACHED */
}

/*
 *	rt_read_rect( Rtf, dx, dy, Srect)
 *	Rtfile  Rtf;
 *	int     dx;
 *	int     dy;
 *	Rect   *Srect;
 *
 *	Read the rectangular pixrect specified by Srect from a rasterfile
 *	into a memory pixrect at coordinates dx, dy.  The rasterfile and
 *	pixrect have already been designated by a call to rt_read_setup().
 *	Returns -1 on error or 0 otherwise.
 */

static
rt_read_rect(Rtf, dx, dy, Srect)
Rtfile          Rtf;
int             dx;
int             dy;
Rect           *Srect;
{
    register Rtfinfo Rti;
    struct pr_pos   simnext;	/* next pixel in the subimage to be read */
    struct pr_pos   sim1last;	/* last pixel of first row in the subimage to
				 * be read */
    struct pr_pos   simlast;	/* last pixel in the subimage to be read */
    rtfpos          scslfpos;	/* file position of start of current subimage
				 * scanline */
    rtfpos          ecslfpos;	/* file position of end of current subimage
				 * scanline */
    rtfpos          snslfpos;	/* file position of start of next subimage
				 * scanline */
    int             nbytes;	/* number of bytes in the current scanline
				 * pixrect buffer				 				 */
    int             rbytes;	/* number of unprocessed bytes remaining
    				 * in the current scanline pixrect buffer				 	 */
    int             startpix;	/* address of the pixel at the start of the
				 * next subimage scanline relative to the
				 * scanline buffer pixrect							 */
    short           overlap;	/* number of bytes in the current buffer from
				 * the start of the next scanline to the end
				 * of the buffer */
    int             lastline;	/* TRUE if we are processing the last line of
				 * the subimage */
    long            bufhead;	/* offset in rasterfile to the byte at the
				 * head of the current scanline buffer */
    int             npixels;
    int             toberead;
    int             nread;
    short           dw;
    long            l;
    long m;

    Rti = (Rtfinfo) Rtf;

    if (Srect->r_height == 0 || Srect->r_width == 0)
	return (0);

/* Initialize loop variables */
    rbytes = 0;
    simnext.x = 0;
    simnext.y = sim1last.y = 0;
    sim1last.x = simlast.x = Srect->r_width - 1;
    simlast.y = Srect->r_height - 1;
    scslfpos = rt_file_pos(&simnext, Rti, Srect);
    ecslfpos = rt_file_pos(&sim1last, Rti, Srect);
    snslfpos.offset = scslfpos.offset + Rti->linebytes;
    snslfpos.skew = scslfpos.skew;
    if (Srect->r_height > 1)
	lastline = FALSE;
    else
	lastline = TRUE;

/*  Set secondary pixrect to point to the destination region */
    mpr_d(Rti->dpr)->md_offset.x = dx;
    mpr_d(Rti->dpr)->md_offset.y = dy;
    Rti->dpr->pr_size.x = Srect->r_width;
    Rti->dpr->pr_size.y = Srect->r_height;

    do {
/*
 *  If the start of the next scanline of the subimage lies in the remainder
 *  of the current scanline buffer, move it to the destination.
 */
	if (rbytes > 0) {
	    startpix = byt_to_pix(scslfpos.offset - bufhead,
				  Rti->slpr->pr_depth)
		+ scslfpos.skew;
	    npixels = byt_to_pix(nbytes, Rti->slpr->pr_depth) - startpix;
	    npixels = MIN(npixels, Srect->r_width);
	    (void) pr_rop(Rti->dpr,
			  simnext.x,
			  simnext.y,
			  npixels,
			  1,
			  PIX_SRC | PIX_DONTCLIP,
			  Rti->slpr,
			  startpix,
			  0);
	    rt_pos_advance(&simnext, Srect, npixels);
	    rbytes -= Rti->linebytes;

	} else {
/*
 *	Scanline buffer is exhausted.  Are we at the start of the next
 *	scanline of the subimage?
 *
 *	Yes.  Check file position, and clear overlap for the case where
 *	an entire subimage scanline lay at the end of the previous
 *	buffer.
 */
	    if (simnext.x == 0) {
		if (Rti->fpos != scslfpos.offset) {
		    if (lseek(Rti->fd, scslfpos.offset, L_SET) < 0) {
			syserror("rt_read_rect", "lseek");
		    }
		    Rti->fpos = scslfpos.offset;
		}
	    }
/*
 *	If the end of the current subimage scanline and the start of the
 *	next subimage scanline falls in the same disk block, read in the
 *	whole block so that it is not read off disk twice.  Unless, of course,
 *	the following scanline terminates before the end of that block.
 */
	    if (ecslfpos.offset / BUFSIZ == snslfpos.offset / BUFSIZ
		&& !lastline) {
		l = (snslfpos.offset / BUFSIZ + 1) * BUFSIZ;
		m = MIN(l, Rti->fsize);
		toberead = m - Rti->fpos;
		overlap = m - snslfpos.offset;
/*
 *	Otherwise just read to the end of the current scanline.
 */
	    } else {
		toberead = ecslfpos.offset - Rti->fpos + 1;
		overlap = 0;
	    }

/*
 *	If we are not reading byte pixels or the current read overlaps
 *	the next scanline, then we must read into the scanline pixrect
 *	and transfer from there.
 */
	    if (overlap || Rti->slpr->pr_depth == 1) {
		bufhead = Rti->fpos;
		nbytes = read(Rti->fd,
			      (char *) mpr_d(Rti->slpr)->md_image,
			      toberead);
		if (nbytes < 0)
		    syserror("rt_read_rect", "read to scanline buffer");
		else if (nbytes != toberead)
		    return (-1);
		Rti->fpos += nbytes;
		dw = Srect->r_width - simnext.x;
		(void) pr_rop(Rti->dpr,
			      simnext.x,
			      simnext.y,
			      dw,
			      1,
			      PIX_SRC | PIX_DONTCLIP,
			      Rti->slpr,
			      scslfpos.skew,
			      0);
		rt_pos_advance(&simnext, Srect, dw);
		rbytes = nbytes - Rti->linebytes;

/*
 *	Otherwise, we are reading byte pixels to the end of the current
 *	scanline and can read directly into the destination pixrect.
 */
	    } else {
		rbytes = 0;
		bufhead = Rti->fpos;
		nread = read(Rti->fd,
			     (char *) mprd8_addr(mpr_d(Rti->dpr),
						 simnext.x,
						 simnext.y,
						 8),
			     toberead);
		if (nread < 0)
		    syserror("rt_read_rect", "read to destination");
		else if (nread != toberead)
		    return (-1);
		Rti->fpos += nread;
		rt_pos_advance(&simnext, Srect, toberead);
	    }

	}

/*
 *	If we have completed a subimage scanline, advance pointers.
 */
	if (simnext.x == 0) {
	    scslfpos = snslfpos;
	    ecslfpos.offset += Rti->linebytes;
	    snslfpos.offset += Rti->linebytes;
	    if (simnext.y >= simlast.y)
		lastline = TRUE;
	}
    } while (!rt_after(simnext, simlast));

    return (0);
}

/*
 *	rt_pos_advance(Pos, Srect, n)
 *	struct pr_pos  *Pos;
 *	Rect           *Srect;
 *	int             n;
 *
 *	Advance the position structure pointed to by Pos n pixels
 *	given the region geometry specified in the rect structure
 *	pointed to by Srect.
 */

static void
rt_pos_advance(Pos, Srect, n)
register struct pr_pos *Pos;
Rect           *Srect;
int             n;
{
    Pos->x += n;
    if (Pos->x >= Srect->r_width) {
	Pos->x -= Srect->r_width;
	Pos->y++;
    }
}


/*
 *	rtfpos
 *	rt_file_pos( Pos, Rti, Srect )
 *	struct pr_pos *Pos;
 *	Rtfinfo Rti;
 *	Rect *Srect;
 *
 *	Return the offset and skew (offset within byte) of the pixel
 *	located at the coordinates pointed to by Pos in the subimage
 *	pointed to by Srect.  Offset and skew are calculated relative
 *	rasterfile associated with Rti.
 */
static          rtfpos
rt_file_pos(Pos, Rti, Srect)
struct pr_pos  *Pos;
Rtfinfo         Rti;
Rect           *Srect;
{
    rtfpos          rfpos;
    int             x;

    x = pr_product(Srect->r_left + Pos->x, Rti->rfile->ras_depth);
    rfpos.skew = x & 07;
    rfpos.offset = pr_product(Rti->linebytes, Srect->r_top + Pos->y)
	+ (x >> 3) + Rti->imofft;
    return (rfpos);
}

#ifndef	lint
static char     rcsid[] = "$Header: /home/ipw/src/etc/rastool/RCS/rtio.c,v 1.1 90/01/31 13:31:41 frew Exp $";

#endif
