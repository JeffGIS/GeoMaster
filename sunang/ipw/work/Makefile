include $(IPW)/lib/make/Makestart

C_SPECIAL= -I$(IPW)/src/exp/erlc/h
LIBS_SPECIAL= $(IPW)/lib/liberlc.a

PGM= itrbxfr

SRCS=\
	bihvalid.c get_sample.c headers.c hle1.c itrbxfr.c main.c newlqh.c \
	output.c

default: pgm
install: install-pgm

include $(IPW)/lib/make/Makerules

$(OBJS): ./$(PGM).h
