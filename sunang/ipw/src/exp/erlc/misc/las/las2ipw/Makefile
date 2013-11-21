
##      To set the LAS environment variables LASINCLUDE, LASLIB, etc.:
##              setenv response LAS50BD
##              source $LASROOT/lasinit

include $(IPW)/lib/make/Makestart

C_SPECIAL= -I$(LASINCLUDE) -I$(WORINCLUDE) -I$(TINC)
LIBS_SPECIAL= $(LASLIB)/las.a $(WORLIB)/geolib.a $(WORLIB)/world.a \
		$(TOLB)/clib.a $(TOLB)/lib.a -ltermlib

PGM= las2ipw

SRCS=\
	main.c headers.c copyimg.c copy_byte.c copy_short.c copy_long.c \
	copy_float.c c_errmsg.c

default: pgm
install: install-pgm

include $(IPW)/lib/make/Makerules

$(OBJS): check_for_LAS ./copy_buf.cpp

check_for_LAS:
	@if [ -n "$(LASINCLUDE)" ]; \
	then exit 0; \
	else echo 'Land Analysis System (LAS) source code not available . . .'; \
		echo "  cannot compile ipw2las"; \
		exit 1; \
	fi
