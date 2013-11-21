include $(IPW)/lib/make/Makestart

PGM= zoom

SRCS=\
	fixhdrs.c headers.c main.c parm.c replicate.c subsamp.c zoom.c

default: pgm
install: install-pgm

include $(IPW)/lib/make/Makerules

$(OBJS): pgm.h
