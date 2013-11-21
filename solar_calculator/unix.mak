# Makefile for JPL ephemeris reader.
# Ephemeris files can be obtained from JPL's ftp site navigator.jpl.nasa.gov
# or on CD-ROM.

# Name of your C compiler.
CC = gcc

# Define one of SSYSTEM, DE102, DE200, DE245, DE403, ... .
# SSYSTEM is for the tabular ephemeris format from de118i.arc.
# The others are to read JPL binary Chebyshev ephemerides.
# DE102 and DE200 are for the old distributions in 9-track magtape format.
# Use DE200CD for the CD-ROM or internet version.  To use fragments
# from JPL's ftp site, define rec0 = 0 in deNNN.h, indicating that
# there is no header information to skip over at the start of the file.
# See the read.me file for other configuration instructions.
# If your computer has IEEE floating point, you should not need to
# translate the ASCII ephemeris because the program handles endian-ness
# swapping if needed (the JPL files are big-endian).
#CFLAGS = -O -DSSYSTEM=1
#CFLAGS = -O -DDE102=1
#CFLAGS = -O -DDE200=1
#CFLAGS = -O -DDE200CD=1
#CFLAGS = -O -DDE245=1
#CFLAGS = -O -DDE400=1
#CFLAGS = -O -DDE403=1
#CFLAGS = -O -DDE404=1
#CFLAGS = -O -DLIB403=1
#CFLAGS = -O -DDE405=1
#CFLAGS = -O -DDE406=1
CFLAGS = -O -DDE406CD=1

OBJS = altaz.o angles.o annuab.o constel.o deflec.o deltat.o \
diurab.o diurpx.o dms.o epsiln.o fk4fk5.o kepjpl.o kepi.o kepj.o \
kfiles.o lightt.o lonlat.o nutate.o precess.o refrac.o \
rplanet.o rstar.o sidrlt.o sun.o tdb.o trnsit.o vearth.o zatan2.o \
moonjpl.o set3.o oparams.o rotate.o

XOBJS = kepjpl.o oparams.o lonlat.o epsiln.o nutate.o precess.o dms.o zatan2.o

INCS = kep.h

all: aa200 x200 # testpo

aa200: aa200.o $(OBJS) $(INCS)
	$(CC) $(CFLAGS) -o aa200 aa200.o $(OBJS) -lm

x200: x200.o $(XOBJS) $(INCS)
	$(CC) $(CFLAGS) -o x200 x200.o $(XOBJS) -lm

# To run the installation test, you need the file of test vectors
# called testpo.NNN as well as the ephemeris file deNNN.unx.
testpo: testpo.o $(XOBJS)
	$(CC) $(CFLAGS) -o testpo testpo.o $(XOBJS) -lm

clean:
	rm -f *.o
