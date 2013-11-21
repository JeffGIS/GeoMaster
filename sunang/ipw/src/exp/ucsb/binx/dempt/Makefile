include $(IPW)/lib/make/Makestart

PGM= dempt

SRCS=\
	atoid.c bigatof.c demhdr.c fillvec.c headers.c litatof.c main.c \
        output.c parm.c prdemhdr.c profhdr.c profiles.c proj.c prprofhdr.c \
        units.c

default: pgm
install: install-pgm

include $(IPW)/lib/make/Makerules

$(OBJS): pgm.h
