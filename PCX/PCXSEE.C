

/* A program to look at PCX pictures */
#define	EGACARD	1

#include "stdio.h"
#include "alloc.h"
#include "dos.h"
#if !EGACARD
#include "graphics.h"
#endif

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

					if((p=malloc(4+bytes*depth)) != NULL) {
						/* unpack the file */
						if(UnpackPcxFile(p+4,fp)==bytes)
						    /* show the picture */
						    ShowPcxPicture(p+4);
						free(p);
					}
				} else printf("Not a PCX file.\n");
			} else printf("Error reading %s.\n",argv[1]);
			fclose(fp);
		} else printf("Error opening %s.\n",argv[1]);
	}
}

ShowPcxPicture(p)	/* display the top of the picture */
	char *p;
{
	unsigned int i;

	/* graphics on */
	init();

	#if EGACARD
	/* copy the graphics to the screen */
        for(i=0;i<350;++i)
		memcpy(MK_FP(0xa000,i*80),p+4+(i*bytes),bytes);

	#else
	p[0]=width-1;
	p[1]=((width-1) >> 8);
	p[2]=depth-1;
	p[3]=((depth-1) >> 8);
	putimage(0,0,p,COPY_PUT);	/* show the picture 	*/
	#endif

	/* wait for a key press */
	getch();
	/* graphics off */
	deinit();
}

UnpackPcxFile(p,fp)	  /* open and print GEM/IMG image n */
	char *p;
	FILE *fp;
{
	int i,n;

	for(i=0;i<depth;++i) {
		n=ReadPcxLine(p,fp);
		p+=bytes;
	}
	return(n);
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

#if EGACARD
init()		/* turn on graphics mode */
{
	union REGS r;

	r.x.ax=0x0010;
	int86(0x10,&r,&r);
}

deinit()	/* turn off graphics card */
{
	union REGS r;

	r.x.ax=0x0003;
	int86(0x10,&r,&r);
}
#else
init()		/* turn on graphics mode */
{
	int d,m,e=0;

	detectgraph(&d,&m);
	if(d<0) {
		puts("No graphics card");
		exit(1);
	}
	if(d==EGA) {
		d=CGA;
		m=CGAHI;
	}
        initgraph(&d,&m,"");
	e=graphresult();
        if(e<0) {
		printf("Graphics error %d: %s",e,grapherrormsg(e));
		exit(1);
	}
	setcolor(getmaxcolor());
}

deinit()	/* turn off graphics card */
{
	closegraph();
}
#endif

