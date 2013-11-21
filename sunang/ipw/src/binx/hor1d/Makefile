include $(IPW)/lib/make/Makestart

#C_SPECIAL=$(C_SINGLE)

PGM= hor1d

SRCS=\
	azmf.c bihvalid.c headers.c hor1b.c hor1f.c horizon.c hormask.c \
        horval.c main.c newlqh.c parm.c zenf.c

default: pgm
install: install-pgm

include $(IPW)/lib/make/Makerules

horizon.o: horizon.h
