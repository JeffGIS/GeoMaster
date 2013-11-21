/* A program to look at 16 colour PCX pictures */

#include "stdio.h"
#include "alloc.h"
#include "dos.h"
#include "graphics.h"

#define	stripsize	50

typedef struct	{
	char	manufacturer;
	char	version;
	char	encoding;
	char	bits_per_pixel;
	int	xmin,ymin;
	int	xmax,ymax;
	int	hres;
	int	vres;
	char	palette[48];
        char	reserved;
	char	colour_planes;
	int	bytes_per_line;
	int	palette_type;
	char	filler[58];
		} PCXHEAD;

PCXHEAD header;			/* where the header lives */
unsigned int width,depth;
unsigned int bytes;

main(argc,argv)
	int argc;
	char *argv[];
{
	FILE *fp;
	char *p;

	if(argc > 1) {
		/* attempt to open the file */
		if((fp=fopen(argv[1],"rb")) != NULL) {
			/* read in the header */
			if(fread((char *)&header,1,sizeof(PCXHEAD),fp)
			    == sizeof(PCXHEAD)) {
				/* check to make sure it's a picture */
				if(header.manufacturer==0x0a) {
					/* allocate a big buffer */
					width = (header.xmax-header.xmin)+1;
					depth = (header.ymax-header.ymin)+1;
					bytes=header.bytes_per_line;
					if((p=malloc(4+(stripsize*bytes*header.colour_planes))) != NULL) {
						/* unpack the file */
						UnpackPcxFile(p,fp);
						free(p);
					}
				} else printf("Not a PCX file.\n");
			} else printf("Error reading %s.\n",argv[1]);
			fclose(fp);
		} else printf("Error opening %s.\n",argv[1]);
	}
}

UnpackPcxFile(p,fp)	  /* open and print image */
	char *p;
	FILE *fp;
{
	int i,n=0,st=0,j;

	/* graphics on */
	init();
	/* set the palette */
	setEGApalette(header.palette,header.colour_planes);

	/* set the image size */
	p[0]=width-1;
	p[1]=((width-1) >> 8);
	p[2]=stripsize-1;
	p[3]=((stripsize-1) >> 8);

	for(i=0;i<depth;++i) {
		/* read lines in inverse order */
		j=header.colour_planes;
		ReadPcxLine(p+4+(bytes*(n + --j)),fp);
		ReadPcxLine(p+4+(bytes*(n + --j)),fp);
		ReadPcxLine(p+4+(bytes*(n + --j)),fp);
		ReadPcxLine(p+4+(bytes*(n + --j)),fp);
		n+=header.colour_planes;

		/* if a whole strip is done, show it */
		if(n == (stripsize * header.colour_planes)) {
			putimage(0,st*stripsize,p,COPY_PUT);
			++st;
			n=0;
		}
	}
	getch();
	/* graphics off */
	deinit();
}

ReadPcxLine(p,fp)   /* read and decode a PCX line into p */
	char *p;
	FILE *fp;
{
	int n=0,c,i;

	/* null the buffer */
	memset(p,0,bytes);
	do {
		/* get a key byte */
        	c=fgetc(fp) & 0xff;
		/* if it's a run of bytes field */
		if((c & 0xc0) == 0xc0) {
			/* and off the high bits */
			i=c & 0x3f;
			/* get the run byte */
			c=fgetc(fp);
			/* run the byte */
			while(i--) p[n++]=c;
		}
		/* else just store it */
		else p[n++]=c;
	} while(n < bytes);
	return(n);
}

init()		/* turn on graphics mode */
{
	int d,m,e=0;

	detectgraph(&d,&m);
	if(d<0) {
		puts("No graphics card");
		exit(1);
	}
	if(d != EGA) {
		puts("EGA card not found");
		exit(1);
	}
        initgraph(&d,&m,"");
	e=graphresult();
        if(e<0) {
		printf("Graphics error %d: %s",e,grapherrormsg(e));
		exit(1);
	}
}

deinit()	/* turn off graphics card */
{
	closegraph();
}

setEGApalette(p,n)	/* set the EGA palette to RGB buffer p */
	char *p;
	int n;
{
	struct palettetype pt;

	/* translate it into colour numbers */
	rgb2ega_pal(pt.colors,p,1 << n);
	pt.size=(1 << n);
	/* and set it */
	setallpalette(&pt);
}

/* translate RGB values into an EGA colour number */
pbits(dest,source,l,h)
	char *dest,*source;
	int l,h;
{
	if(*source > 0x33) {
		*dest |=l;
		if(*source > 0x77) {
			*dest &= ~l;
			*dest |= h;
			if(*source > 0xbb) *dest |= l+h;
		}
	}
}

/* translate a buffer of RGB values into EGA colour numbers */
rgb2ega_pal(dest,source,n)
	char *dest,*source;
	int n;
{
	int i;

	for(i=0;i<n;++i) {
		*dest=0;
		pbits(dest,source++,0x20,0x04);
		pbits(dest,source++,0x10,0x02);
		pbits(dest,source++,0x08,0x01);
		++dest;
	}
}
