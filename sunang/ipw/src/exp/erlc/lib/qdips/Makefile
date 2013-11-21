include $(IPW)/lib/make/Makestart

C_SPECIAL= -I$(IPW)/src/exp/erlc/h

LIB=erlc

SRCS=\
	bdfunc.c bevap.c bowen.c brutsaert.c budit2.c budyer.c \
	dew_point.c efcon.c evap.c g_snow.c g_soil.c getf.c getok.c \
	heat_stor.c hle1.c net_therm.c new_tsno.c psychrom.c \
	ri_no.c sati.c satw.c ssxfr.c zerobr.c

default: lib
install: install-lib

include $(IPW)/lib/make/Makerules

$(OBJS): $(IPW)/src/exp/erlc/h/physdefs.h
