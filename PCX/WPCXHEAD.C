WritePcxHeader(fp)
	FILE *fp;
{
	memset((char *)&header,0,sizeof(PCXHEAD));
	header.manufacturer=0x0a;
	header.version=0;
	header.encoding=1;

/* bits per pixel: 	1 for monochrome
			4 for 16 colors
			8 for 256 colors */

	header.bits_per_pixel=1;	
	header.xmin=header.ymin=0;
	header.xmax=wide-1;
	header.ymax=deep-1;
	header.colour_planes=1;
	header.bytes_per_line=bytes;
	header.palette_type=2;
	return(fwrite((char *)&header,1,sizeof(PCXHEAD),fp));
}
