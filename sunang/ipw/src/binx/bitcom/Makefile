include $(IPW)/lib/make/Makestart

PGM= bitcom

SRCS=\
	bitcom.c bitio.c bx_and.c bx_or.c bx_xor.c m_expand.c main.c

default: pgm
install: install-pgm

include $(IPW)/lib/make/Makerules

$(OBJS): $(PGM).h
