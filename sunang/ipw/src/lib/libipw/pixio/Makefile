include $(IPW)/lib/make/Makestart

LIB=ipw

SRCS=\
	_piocb.c _pioinit.c pvread.c pvwrite.c pxclose.c


default: lib
install: install-lib

include $(IPW)/lib/make/Makerules

_piocb.o _pioinit.o pvread.o pxclose.o: _pixio.h
