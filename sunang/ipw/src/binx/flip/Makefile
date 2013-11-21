include $(IPW)/lib/make/Makestart

PGM= flip

SRCS=\
	fixgeoh.c fixorh.c fixwinh.c flip.c flip1d.c flip2d.c headers.c \
        main.c parm.c revchar.c reverse.c revgen.c revlong.c revshort.c

default: pgm
install: install-pgm

include $(IPW)/lib/make/Makerules

$(OBJS): pgm.h
revchar.o: reverse.cpp
revlong.o: reverse.cpp
revshort.o: reverse.cpp
