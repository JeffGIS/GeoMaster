/*--------------------------------------------------------------------------*\
|| fiio_mem.cpp by Ryan Rubley <ryan@lostreality.org>                       ||
||                                                                          ||
|| (v1.02) 4-28-2004                                                        ||
|| FreeImageIO to memory                                                    ||
||                                                                          ||
\*--------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include "fiio_mem.h"

#ifdef __cplusplus
extern "C" {
#endif


FIBITMAP *
LoadImageFromMem (FREE_IMAGE_FORMAT fif, void *pMem, int MemLen,int flags)
{
	fiio_mem_handle handle;
	FIBITMAP * rtn;

	memset (&handle,0,sizeof(handle));
	handle.datalen = MemLen;
	handle.data = pMem;
	rtn = FreeImage_LoadFromMem(fif,&handle, flags);
	return rtn;
}

BOOL
SaveImageToMem (FREE_IMAGE_FORMAT fif, FIBITMAP *dib,void *pMem, int *MemLen,int flags)
{
	fiio_mem_handle handle;
	BOOL rtn;

	memset (&handle,0,sizeof(handle));
	handle.data = pMem;
	rtn = FreeImage_SaveToMem(fif,dib,&handle, flags);
	*MemLen = handle.datalen;
	return rtn;
}

FIBITMAP *
FreeImage_LoadFromMem(FREE_IMAGE_FORMAT fif, fiio_mem_handle *handle, int flags) {
	FreeImageIO io;
	SetMemIO(&io);
	
	if (handle && handle->data) {
		handle->curpos = 0;
		return FreeImage_LoadFromHandle(fif, &io, (fi_handle)handle, flags);
	}

	return NULL;
}

BOOL
FreeImage_SaveToMem(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, fiio_mem_handle *handle, int flags) {
	FreeImageIO io;
	SetMemIO(&io);

	if (handle) {
		handle->filelen = 0;
		handle->curpos = 0;
		return FreeImage_SaveToHandle(fif, dib, &io, (fi_handle)handle, flags);
	}

	return FALSE;
}

// ----------------------------------------------------------

// ----------------------------------------------------------

#define FIIOMEM(member) (((fiio_mem_handle *)handle)->member)

unsigned
fiio_mem_ReadProc(void *buffer, unsigned size, unsigned count, fi_handle handle) {
	unsigned x;
	for( x=0; x<count; x++ ) {
		//if there isnt size bytes left to read, set pos to eof and return a short count
		if( FIIOMEM(filelen)-FIIOMEM(curpos) < (long)size ) {
			FIIOMEM(curpos) = FIIOMEM(filelen);
			break;
		}
		//copy size bytes count times
		memcpy( buffer, (char *)FIIOMEM(data) + FIIOMEM(curpos), size );
		FIIOMEM(curpos) += size;
		buffer = (char *)buffer + size;
	}
	return x;
}

unsigned
fiio_mem_WriteProc(void *buffer, unsigned size, unsigned count, fi_handle handle) {
	void *newdata;
	long newdatalen;
	//double the data block size if we need to
	while( FIIOMEM(curpos)+(long)(size*count) >= FIIOMEM(datalen) ) {
		//if we are at or above 1G, we cant double without going negative
		if( FIIOMEM(datalen) & 0x40000000 ) {
			//max 2G
			if( FIIOMEM(datalen) == 0x7FFFFFFF ) {
				return 0;
			}
			newdatalen = 0x7FFFFFFF;
		} else if( FIIOMEM(datalen) == 0 ) {
			//default to 4K if nothing yet
			newdatalen = 4096;
		} else {
			//double size
			newdatalen = FIIOMEM(datalen) << 1;
		}
		newdata = realloc( FIIOMEM(data), newdatalen );
		if( !newdata ) {
			return 0;
		}
		FIIOMEM(data) = newdata;
		FIIOMEM(datalen) = newdatalen;
	}
	memcpy( (char *)FIIOMEM(data) + FIIOMEM(curpos), buffer, size*count );
	FIIOMEM(curpos) += size*count;
	if( FIIOMEM(curpos) > FIIOMEM(filelen) ) {
		FIIOMEM(filelen) = FIIOMEM(curpos);
	}
	return count;
}

int
fiio_mem_SeekProc(fi_handle handle, long offset, int origin) {
	switch(origin) { //0 to filelen-1 are 'inside' the file
	default:
	case SEEK_SET: //can fseek() to 0-7FFFFFFF always
		if( offset >= 0 ) {
			FIIOMEM(curpos) = offset;
			return 0;
		}
		break;

	case SEEK_CUR:
		if( FIIOMEM(curpos)+offset >= 0 ) {
			FIIOMEM(curpos) += offset;
			return 0;
		}
		break;

	case SEEK_END:
		if( FIIOMEM(filelen)+offset >= 0 ) {
			FIIOMEM(curpos) = FIIOMEM(filelen)+offset;
			return 0;
		}
		break;
	}

	return -1;
}

long
fiio_mem_TellProc(fi_handle handle) {
	return FIIOMEM(curpos);
}

void
SetMemIO(FreeImageIO *io) {
	io->read_proc  = (FI_ReadProc)fiio_mem_ReadProc;
	io->seek_proc  = (FI_SeekProc)fiio_mem_SeekProc;
	io->tell_proc  = (FI_TellProc)fiio_mem_TellProc;
	io->write_proc = (FI_WriteProc)fiio_mem_WriteProc;
}


#ifdef __cplusplus
}
#endif
