include $(IPW)/lib/make/Makestart

PGM= sunlight

SRCS=\
	krontime.c main.c sunlight_opts.c sunrise.c vqk15.c vqk21.c vqkagn.c \
	zerobr.c

default: pgm
install: install-pgm

include $(IPW)/lib/make/Makerules

main.o: $(PGM).h
sunrise.o: $(PGM).h
sunlight_opts.o: $(PGM).h
