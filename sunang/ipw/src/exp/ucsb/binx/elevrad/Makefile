include $(IPW)/lib/make/Makestart

C_SPECIAL = -I$(IPW)/src/exp/ucsb/h
LIBS_SPECIAL= $(IPW)/lib/libDoz.a

PGM= elevrad

SRCS=\
	data.c elevrad.c init_tau.c main.c radtbls.c

default: pgm
install: install-pgm

include $(IPW)/lib/make/Makerules

radtbls.o: elevrad.h
elevrad.o: elevrad.h
