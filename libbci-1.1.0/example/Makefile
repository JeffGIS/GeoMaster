TARGET = bciexample
SRCS = bciexample.c
LDADD += -lm -lbci
CFLAGS += -I/usr/local/include -L/usr/local/lib

all: ${TARGET}

clean:
	rm -f ${TARGET}

${TARGET}: bciexample.c
	${CC} ${CFLAGS} -o ${TARGET} bciexample.c ${LDADD}

