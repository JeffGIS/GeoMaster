#-----------------------------------------------------------------------
# Copyright (c) 1990 The Regents of the University of California.
# All rights reserved.
#
# Redistribution and use in source and binary forms are permitted
# provided that: (1) source distributions retain this entire copyright
# notice and comment, and (2) distributions including binaries display
# the following acknowledgement:  ``This product includes software
# developed by the Computer Systems Laboratory, University of
# California, Santa Barbara and its contributors'' in the documentation
# or other materials provided with the distribution and in all
# advertising materials mentioning features or use of this software.
#
# Neither the name of the University nor the names of its contributors
# may be used to endorse or promote products derived from this software
# without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#-----------------------------------------------------------------------
# machine - calculates machine dependent constants

#------------------------------------------------------------------------------
PGM	= machine

OBJS  = \
	Cprint.o Gprint.o Iprint.o LGprint.o Lprint.o Main.o Prt_float.o \
	Prt_ieee.o Prt_limits.o ULprint.o Uprint.o char_bit.o char_lim.o \
	describe.o dig_count.o dmachar.o fmachar.o ilog10.o int_limits.o \
	is_ieee.o ldig_count.o ldlog10.o ldmachar.o long_lim.o lpowi.o \
	powi.o shrt_lim.o usage.o

#------------------------------------------------------------------------------
BIN	= /usr/home/ipw/etc
CLEAN	= $(PGM) $(OBJS) core
COPT	=
DEFINE	=
FOPT	= -u
INCLUDE	=
LIBS	=
LINK	= cc
LINKOPT	=
LINTOPT	= -chapbx
MODE	= 771
SRCS	= *.c
#------------------------------------------------------------------------------
CFLAGS	= $(DEFINE) $(INCLUDE) $(COPT)
FFLAGS	= $(FOPT)

#------------------------------------------------------------------------------
$(PGM): $(OBJS)	;$(LINK) $(LINKOPT) -o $(PGM) $(OBJS) $(LIBS)
	rm -f schar.h
	rm -f ldbl.h

#------------------------------------------------------------------------------
install: $(PGM)	;-install -s -m $(MODE) $(PGM) $(BIN) ;rm -f $(PGM)

clean:		;rm -f $(CLEAN)

schar.h:
	schar.sh > schar.h

ldbl.h:
	ldbl.sh > ldbl.h
#------------------------------------------------------------------------------
pgm: $(PGM)

lint:		;lint $(SRCS)
#------------------------------------------------------------------------------
# specific dependencies
Cprint.o:	tabsize.h
Gprint.o:	Gprint.cpp
Iprint.o:	tabsize.h
LGprint.o:	Gprint.cpp ldbl.h
Lprint.o:	tabsize.h
Prt_float.o:	ldbl.h
Prt_limits.o:	schar.h
ULprint.o:	tabsize.h
Uprint.o:	tabsize.h
char_limits.o:	limits.cpp schar.h
dig_count.o:	dcount.cpp
dmachar.o:	machar.cpp
fmachar.o:	machar.cpp
ilog10.o:	ilog10.cpp
int_limits.o:	limits.cpp
is_ieee.o:	ieee.h
ldig_count.o:	dcount.cpp ldbl.h
ldlog10.o:	ilog10.cpp ldbl.h
ldmachar.o:	ldbl.h machar.cpp
long_limits.o:	limits.cpp
lpowi.o:	ldbl.h powi.cpp
powi.o:		powi.cpp
shrt_limits.o:	limits.cpp

# $Header: /local/share/pkg/ipw/src/etc/machine/RCS/Makefile,v 1.3 90/11/19 14:57:49 frew Exp $
