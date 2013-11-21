GTK_INCLUDE ?= `pkg-config gtk+-2.0 --cflags`
GTK_LIB ?= `pkg-config gtk+-2.0 --libs`

INSTALLDIR = /usr/local/lib/gkrellm2/plugins

FLAGS = -O2 -Wall -fPIC $(GTK_INCLUDE)
LIBS = $(GTK_LIB)
LFLAGS = -shared

CC = gcc $(CFLAGS) $(FLAGS)

OBJS = gkrellsun.o CalcEphem.o Moon.o

IMAGES = ../images/osun.xpm ../images/uvsun.xpm \
         ../images/digitsy.xpm ../images/digitsw.xpm \
         ../images/star.xpm ../images/dot.xpm

all: gkrellsun.so

gkrellsun.so: $(OBJS)
	$(CC) $(OBJS) -o gkrellsun.so $(LFLAGS) $(LIBS)

suninfo: suninfo.o CalcEphem.o Moon.o
	$(CC) suninfo.o CalcEphem.o Moon.o -o suninfo -lm

clean:
	rm -f *.o core *.so* *.bak *~

gkrellsun.o: gkrellsun.c $(IMAGES)

# %.o: %.c

install: gkrellsun.so
	install -c -s -m 644 gkrellsun.so $(INSTALLDIR)
