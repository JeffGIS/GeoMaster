include $(IPW)/lib/make/Makestart

LIB=gctp

SRCS=\
	adjlz0.c asinz0.c blkdta.c dmslz0.c e0fnz0.c e1fnz0.c e2fnz0.c \
	e3fnz0.c e4fnz0.c gtrnz0.c mlfnz0.c msfnz0.c paksz0.c phi1z0.c \
	phi2z0.c phi3z0.c phi4z0.c pj01z0.c pj02z0.c pj03z0.c pj04z0.c \
	pj05z0.c pj06z0.c pj07z0.c pj08z0.c pj09z0.c pj10z0.c pj11z0.c \
	pj12z0.c pj13z0.c pj14z0.c pj15z0.c pj16z0.c pj17z0.c pj18z0.c \
	pj19z0.c pj20z0.c qsfnz0.c spakz0.c tsfnz0.c unitz0.c untfz0.c xtra.c

default: lib
install: install-lib

include $(IPW)/lib/make/Makerules

$(OBJS):	./gctp.h
pj02z0.o:	./gctp.h ./nad27.h ./nad83.h
./gctp.h:	./proto.h
