
##      To set the LAS environment variables LASINCLUDE, LASLIB, etc.:
##              setenv response LAS50BD
##              source $LASROOT/lasinit

include $(IPW)/lib/make/Makestart

C_SPECIAL= -I$(IPW)/src/exp/erlc/h -I$(LASINCLUDE) -I$(WORINCLUDE) -I$(TINC)
LIBS_SPECIAL= $(IPW)/lib/liberlc.a \
		$(LASLIB)/las.a $(WORLIB)/geolib.a $(WORLIB)/world.a \
		$(WORLIB)/notae.a $(TOLB)/clib.a $(TOLB)/lib.a -ltermlib

PGM= ipw2las

SRCS=\
	copyimg.c fix_dms.c headers.c las_dms.c main.c out_byte.c out_float.c \
	out_long.c out_short.c update_ddr.c

default: pgm
install: install-pgm

include $(IPW)/lib/make/Makerules

$(OBJS): check_for_LAS $(LASINCLUDE)/las.h ./out_buf.cpp

check_for_LAS:
	@if [ -n "$(LASINCLUDE)" ]; \
	then exit 0; \
	else echo 'Land Analysis System (LAS) source code not available . . .'; \
		echo "  cannot compile ipw2las"; \
		exit 1; \
	fi
