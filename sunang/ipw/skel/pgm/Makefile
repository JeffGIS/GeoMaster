include $(IPW)/lib/make/Makestart

# C_SPECIAL     "cc" options
# LD_SPECIAL    loader options
# LIBS_SPECIAL  object libraries
 
C_SPECIAL=
LD_SPECIAL=
LIBS_SPECIAL=

# PGM           program name

PGM= 

# SRCS          component source files
# OBJS		is set for you automatically, so there is no need to set
#		it unless you are using non-.c files.

SRCS=\
# vi: type !!srcs here

default: pgm
install: install-pgm

include $(IPW)/lib/make/Makerules

# use one of these.  The old style was to have a header file named pgm.h.
# the new style is to have you header file be the name of the program.h.

# $(OBJS): pgm.h

$(OBJS): ./$(PGM).h
