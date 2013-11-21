include $(IPW)/lib/make/Makestart

PGM= toporad

SRCS=\
	head_final.c head_init.c main.c radcalc.c toporad.c

default: pgm
install: install-pgm

include $(IPW)/lib/make/Makerules

head_init.o: toporad.h
radcalc.o: toporad.h
toporad.o: toporad.h
