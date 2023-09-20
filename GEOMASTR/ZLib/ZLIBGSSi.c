#include "zlib.h"
typedef unsigned char       BYTE;
typedef unsigned long       DWORD;


int ZLibCompress(BYTE* target, DWORD target_size, BYTE* source, DWORD source_size) {
	uLongf dest_len = (uLongf)target_size;

	int zerr = compress(target, &dest_len, source, source_size);
	switch (zerr) {
	case Z_MEM_ERROR:	// not enough memory
	case Z_BUF_ERROR:	// not enough room in the output buffer
//		FreeImage_OutputMessageProc(FIF_UNKNOWN, "Zlib error : %s", zError(zerr));
		return zerr;
	case Z_OK:
		return dest_len;
	}

	return 0;
}
int ZLibUncompress(BYTE* target, DWORD target_size, BYTE* source, DWORD source_size) {
	uLongf dest_len = (uLongf)target_size;

	int zerr = uncompress(target, &dest_len, source, source_size);
	switch (zerr) {
	case Z_MEM_ERROR:	// not enough memory
	case Z_BUF_ERROR:	// not enough room in the output buffer
	case Z_DATA_ERROR:	// input data was corrupted
		//FreeImage_OutputMessageProc(FIF_UNKNOWN, "Zlib error : %s", zError(zerr));
		return zerr;
	case Z_OK:
		return dest_len;
	}

	return 0;
}
