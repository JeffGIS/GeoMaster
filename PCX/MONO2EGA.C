/* return an EGA version of a monochrome bitmap */
char *mono2ega(source)
	char *source;
{
	char *p;
	int x,y,i,j,ls,sz;

	x=1+source[0]+(source[1] << 8);
	y=1+source[2]+(source[3] << 8);
	if((sz=imagesize(0,0,x,y)) != -1) {
		if((p=malloc(sz)) != NULL) {
			memset(p,0,sz);
			memcpy(p,source,4);
			ls=pixels2bytes(x);
			for(j=0;j<y;++j) {
				memcpy(p+4+((j*4)*ls),
				    source+4+(j*ls),ls);
				memcpy(p+4+ls+((j*4)*ls),
				    source+4+(j*ls),ls);
				memcpy(p+4+(ls*2)+((j*4)*ls),
				    source+4+(j*ls),ls);
				memcpy(p+4+(ls*3)+((j*4)*ls),
				    source+4+(j*ls),ls);
			}
			return(p);
		}
	} else return(NULL);
}
