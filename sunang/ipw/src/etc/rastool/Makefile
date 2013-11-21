include $(IPW)/lib/make/Makestart

LIBS_SPECIAL=-lsuntool -lsunwindow -lpixrect

PGM= rastool

SRCS=\
	error.c rastool.c rtio.c

OBJS := $(SRCS:%.c=%.o)

PGM_DIR=$(ETC_DIR)

default: pgm
install: install-pgm

include $(IPW)/lib/make/Makerules
