GTK_INCLUDE = `gtk-config --cflags`
GTK_LIB = `gtk-config --libs`

IMLIB_INCLUDE = `imlib-config --cflags-gdk`
IMLIB_LIB = `imlib-config --libs-gdk`
THREAD_LIB = -lpthread
INSTALLDIR = /usr/local/share/gkrellm/plugins

ZIPVER=

FLAGS = -O2 -Wall -fPIC $(GTK_INCLUDE) $(IMLIB_INCLUDE)
LIBS = $(GTK_LIB) $(IMLIB_LIB) $(THREAD_LIB)
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

zip:
ifeq ($(ZIPVER),)
	@echo You must specify ZIPVER for the zip target
else
	rm -rf /tmp/gkrellsun-$(ZIPVER)
	rm -f /tmp/tar.exclude
	rm -f /tmp/gkrellsun-$(ZIPVER).tar.gz
	rm -f /tmp/gkrellsun-$(ZIPVER).zip
	mkdir /tmp/gkrellsun-$(ZIPVER)
	touch /tmp/tar.exclude
	find . -print  | grep /CVS$$ | cut -c3- >> /tmp/tar.exclude
	find . -print  | grep /CVS/ | cut -c3- >> /tmp/tar.exclude
	find . -print  | grep .classes | cut -c3- >> /tmp/tar.exclude
	find . -type f -name "*~"  | cut -c3- >> /tmp/tar.exclude
	find . -type f -name "*.o"  | cut -c3- >> /tmp/tar.exclude
	find . -type f -name "*.so"  | cut -c3- >> /tmp/tar.exclude
	find . -type f -name "#*"  | cut -c3- >> /tmp/tar.exclude
	find . -type f -name "suninfo"  | cut -c3- >> /tmp/tar.exclude
	tar cf - * --exclude-from /tmp/tar.exclude | (cd /tmp/gkrellsun-$(ZIPVER); tar xf -)
	cd /tmp && tar cf - gkrellsun-$(ZIPVER) | gzip > gkrellsun-$(ZIPVER).tar.gz
	cd /tmp && zip -rpD gkrellsun-$(ZIPVER).zip gkrellsun-$(ZIPVER)
	rm -f tar.exclude
endif
