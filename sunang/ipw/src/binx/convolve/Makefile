include $(IPW)/lib/make/Makestart

PGM= convolve

SRCS=\
	bufcycle.c conv1d.c convolve.c do_kmap.c getkernel.c main.c \
        putkernel.c

default: pgm
install: install-pgm

include $(IPW)/lib/make/Makerules

$(OBJS): $(PGM).h
