/* a program to pack a getimage fragment into a PCX file */

#include "stdio.h"
#include "alloc.h"

#define	size	(wide * pixels2bytes(deep))

char bin_file[80];      /* buffers for file names */
char pcx_file[80];

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

int wide,deep,bytes;	/* global size values */

main(argc,argv)
	int argc;
	char *argv[];
{
	FILE *source,*dest;
	char *p,b[4];

	if(argc > 1) {
		/* make file names */
		strmfe(bin_file,argv[1],"BIN");
		strmfe(pcx_file,argv[1],"PCX");

		/* open the source file */
		if((source=fopen(bin_file,"rb")) != NULL) {
			/* create the destination file */
			if((dest=fopen(pcx_file,"wb")) != NULL) {
				/* read in the size */
				if(fread(b,1,4,source)==4) {
					/* and get it from the buffer */
					wide = b[0]+(b[1]<<8)+1;
					deep = b[2]+(b[3]<<8)+1;
					bytes = pixels2bytes(wide);
					/* allocate an image buffer */
					if((p=malloc(size)) != NULL) {
						/* read in the image */
						if(fread(p,1,size,source) == size) {
							printf("Packing image %d by %d pixels\n",
							    wide,deep);
							/* pack the image */
							PackPcxFile(dest,p);
						} else printf("Error reading %s\n",bin_file);
						free(p);
					} else puts("Error allocating memory");
				} else puts("Error reading header");
				fclose(dest);
			} else printf("Error creating %s\n",pcx_file);
			fclose(source);
		} else printf("Error opening %s\n",bin_file);
	} else puts("I need an argument");
}

WritePcxLine(p,fp)
	char *p;
	FILE *fp;
{
	char b[64];
	unsigned int i=0,j=0,t=0;

	do {
		i=0;
		while((p[t+i]==p[t+i+1]) && ((t+i) < bytes) && (i<63))++i;
		if(i>0) {
			fputc(i | 0xc0,fp);
			fputc(~p[t],fp);
			t+=i;
			j+=2;
		}
		else {
			if(((~p[t]) & 0xc0)==0xc0) {
				fputc(0xc1,fp);
				++j;
			}
			fputc(~p[t++],fp);
			++j;
		}
	} while(t<bytes);
	return(j);
}

/* pack an image into a PCX file */
PackPcxFile(fp,p)
	FILE *fp;
	char *p;
{
	int i;

	/* write the header */
	WritePcxHeader(fp);
	/* pack the lines */
	for(i=0;i<deep;++i) WritePcxLine(p+(i*bytes),fp);
}

/* write a PCX header */
WritePcxHeader(fp)
	FILE *fp;
{
	memset((char *)&header,0,sizeof(PCXHEAD));
	header.manufacturer=0x0a;
	header.version=0;
	header.encoding=1;
	header.bits_per_pixel=8;
	header.xmin=header.ymin=0;
	header.xmax=wide-1;
	header.ymax=deep-1;
	header.colour_planes=1;
	header.bytes_per_line=bytes;
	header.palette_type=2;
	return(fwrite((char *)&header,1,sizeof(PCXHEAD),fp));
}

/* make a new file name with a fixed extension */
strmfe(new,old,ext)
	char *new,*old,*ext;
{
	while(*old != 0 && *old != '.') *new++=*old++;
	*new++='.';
	while(*ext) *new++=*ext++;
	*new=0;
}

/* return number of bytes in number of pixels */
pixels2bytes(n)
	int n;
{
	if(n & 0x0007) return((n >> 3) + 1);
	else return(n >> 3);
}
