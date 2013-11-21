include $(IPW)/lib/make/Makestart

PGM= dither

SRCS=\
	dither.c dithimg.c dm_val.c main.c mk_dm.c

default: pgm
install: install-pgm

include $(IPW)/lib/make/Makerules

$(OBJS): $(PGM).h
