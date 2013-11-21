include $(IPW)/lib/make/Makestart

PGM= solar

SRCS=\
	akcoef.c main.c modprod.c solar.c solint.c solval.c splint.c \
	spval1.c updbl.c vdcps.c vtdbl.c

default: pgm
install: install-pgm

include $(IPW)/lib/make/Makerules

solval.o: solar.h
solint.o: solar.h
