include $(IPW)/lib/make/Makestart

C_NORMAL= -DNO_SC

PGM= config

SRCS=\
	config.c

OBJS := $(SRCS:%.c=%.o)

PGM_DIR=$(ETC_DIR)

default: pgm
install: install-pgm

include $(IPW)/lib/make/Makerules
