include $(IPW)/lib/make/Makestart

C_SPECIAL= -I$(IPW)/src/exp/erlc/h

LIB=erlc

SRCS=\
	dms.c projname.c projunits.c readproj.c unpack_dms.c writeproj.c \
	nameproj.c

default: lib
install: install-lib

include $(IPW)/lib/make/Makerules

$(OBJS):	$(IPW)/src/exp/erlc/h/mproj.h
