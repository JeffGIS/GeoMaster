include $(IPW)/lib/make/Makestart

PGM= lincom

SRCS=\
	coeffs.c headers.c lincom.c main.c output.c parm.c re_map.c

default: pgm
install: install-pgm

include $(IPW)/lib/make/Makerules

$(OBJS): pgm.h
