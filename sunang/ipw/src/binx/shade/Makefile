include $(IPW)/lib/make/Makestart

#C_SPECIAL=$(C_SINGLE)

PGM= shade

SRCS=\
	azmf.c buffers.c cosines.c data.c headers.c init_shade.c invzenf.c \
        main.c newlqh.c newsunh.c shadetbl.c trigtbl.c zenf.c

default: pgm
install: install-pgm

include $(IPW)/lib/make/Makerules

$(OBJS): $(PGM).h
