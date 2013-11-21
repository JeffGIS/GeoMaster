include $(IPW)/lib/make/Makestart

C_SPECIAL = -I$(IPW)/src/exp/ucsb/h

PGM= refdx

SRCS=\
	akcoef.c main.c modprod.c refh2o.c refice.c spval1.c vdcps.c vtdbl.c \
	wavechoice.c

default: pgm
install: install-pgm

include $(IPW)/lib/make/Makerules

refh2o.o:	genref.cpp genreft.cpp waterdata.h
refice.o:	genref.cpp genreft.cpp icedata.h

$(OBJS): pgm.h
