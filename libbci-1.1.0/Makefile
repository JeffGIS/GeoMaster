LIB = bci
LIBNAME = lib${LIB}.a
SRCS = tools.c tdspl.c bci.c
CFLAGS += -I/usr/local/include -L/usr/local/lib
INCDIR = /usr/local/include
LIBDIR = /usr/local/lib

OBJS+= ${SRCS:S/.c/.o/g}

all: ${LIBNAME}

install: ${LIBNAME}
	${INSTALL} -g wheel -m 0644 -o root libbci.a ${LIBDIR}
	${INSTALL} -g wheel -m 0644 -o root bci.h ${INCDIR}

uninstall:
	rm -f ${LIBDIR}/libbci.a ${INCDIR}/bci.h

clean:
	rm -f ${OBJS} ${LIBNAME}

${LIBNAME}: ${OBJS}
	@rm -f ${LIBNAME}
	@${AR} rcs ${LIBNAME} ${OBJS}
	${RANLIB} ${LIBNAME}

${OBJS}: ${SRCS}
	${CC} -c ${CFLAGS} $<

