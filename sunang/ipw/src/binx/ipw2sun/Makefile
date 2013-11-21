include $(IPW)/lib/make/Makestart

PGM= ipw2sun

SRCS=\
	bitmap.c main.c sunhdr.c sunras.c

default: pgm
install: install-pgm

include $(IPW)/lib/make/Makerules

$(OBJS): sunras.h
