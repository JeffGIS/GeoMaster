include $(IPW)/lib/make/Makestart

ACCUM=fast

PGM= mstats

SRCS=\
	accum.c headers.c init.c main.c mcov.c mstats.c parm.c

default: pgm
install: install-pgm

include $(IPW)/lib/make/Makerules

$(OBJS): pgm.h
