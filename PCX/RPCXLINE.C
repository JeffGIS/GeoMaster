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
			while(i--) p[n++]=~c;
		}
		/* else just store it */
		else p[n++]=~c;
	} while(n < bytes);
	return(n);
}
