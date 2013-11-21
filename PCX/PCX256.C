/* A program to look at 256 colour PCX pictures */

#include "stdio.h"
#include "alloc.h"
#include "dos.h"
#include "graphics.h"

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
char palette[768];		/* where the palette lives */

main(argc,argv)
	int argc;
	char *argv[];
{
	FILE *fp;

	if(argc > 1) {
		/* attempt to open the file */
		if((fp=fopen(argv[1],"rb")) != NULL) {
			/* read in the header */
			if(fread((char *)&header,1,sizeof(PCXHEAD),fp)
			    == sizeof(PCXHEAD)) {
				/* check to make sure it's a picture */
				if(header.manufacturer==0x0a &&
				   header.version == 5) {
					/* find the palette */
					if(!fseek(fp,-769L,SEEK_END)) {
					  if(fgetc(fp) == 0x0c &&
					    fread(palette,1,768,fp) == 768) {
					      fseek(fp,128L,SEEK_SET);
					      /* allocate a big buffer */
					        width = (header.xmax-
						  header.xmin)+1;
						depth = (header.ymax-
						  header.ymin)+1;
						bytes=header.bytes_per_line;
						/* unpack the file */
						UnpackPcxFile(fp);
						} else 
						  puts("Error reading palette");
					} else puts("Error seeking to palette");
				} else printf("Not a 256 color PCX file.\n");
			} else printf("Error reading %s.\n",argv[1]);
			fclose(fp);
		} else printf("Error opening %s.\n",argv[1]);
	}
}

UnpackPcxFile(fp)	  /* open and print image */
	FILE *fp;
{
	int i;

	/* graphics on */
	init();

	/* set the palette */
	setVGApalette(palette);

	/* unpack the file directly to the VGA buffer */
	for(i=0;i<depth;++i) ReadPcxLine(MK_FP(0xa000,i*320),fp);

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
	union REGS r;

	r.x.ax=0x0013;
	int86(0x10,&r,&r);
}

deinit()	/* turn off graphics card */
{
	union REGS r;

	r.x.ax=0x0003;
	int86(0x10,&r,&r);
}

setVGApalette(p)	/* set the VGA palette to RGB buffer p */
	char *p;
{
	union REGS r;
	struct SREGS sr;
	int i;

	/* convert eight bits to six bits */
	for(i=0;i<768;++i) p[i]=p[i] >> 2;

	r.x.ax=0x1012;
	r.x.bx=0;
	r.x.cx=256;
	r.x.dx=FP_OFF(p);
	sr.es=FP_SEG(p);
	int86x(0x10,&r,&r,&sr);
}

