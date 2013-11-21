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
