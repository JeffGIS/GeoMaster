# MSDOS make file for JPL ephemeris reader.
# This is for Microsoft make.exe make program.
# Microsoft Visual C has nmake.exe; use mscn.mak for that.

# Define one of SSYSTEM, DE400, DE200, DE245, DE102.
# SSYSTEM is for the tabular ephemeris format from de118i.arc.
# The others are to read JPL Chebyshev ephemerides.
CFLAGS = /c /AL /DSSYSTEM=1
#CFLAGS = /c /AL /DDE403=1
#CFLAGS = /c /AL /DLIB403=1
#CFLAGS = /c /AL /DDE404=1
#CFLAGS = /c /AL /DDE400=1
#CFLAGS = /c /AL /DDE245=1
#CFLAGS = /c /AL /DDE200CD=1
#CFLAGS = /c /AL /DDE200=1
#CFLAGS = /c /AL /DDE102=1
#CFLAGS = /c /AL /DDE405=1
#CFLAGS = /c /AL /DDE406=1
#CFLAGS = /c /AL /DDE406CD=1

aa200.obj: aa200.c kep.h
	cl $(CFLAGS) aa200.c

altaz.obj: altaz.c kep.h
	cl $(CFLAGS) altaz.c

angles.obj: angles.c kep.h
	cl $(CFLAGS) angles.c

annuab.obj: annuab.c kep.h
	cl $(CFLAGS) annuab.c

constel.obj: constel.c kep.h
	cl $(CFLAGS) constel.c

deflec.obj: deflec.c kep.h
	cl $(CFLAGS) deflec.c

deltat.obj: deltat.c kep.h
	cl $(CFLAGS) deltat.c

diurab.obj: diurab.c kep.h
	cl $(CFLAGS) diurab.c

diurpx.obj: diurpx.c kep.h
	cl $(CFLAGS) diurpx.c

dms.obj: dms.c kep.h
	cl $(CFLAGS) dms.c

epsiln.obj: epsiln.c kep.h
	cl $(CFLAGS) epsiln.c

fk4fk5.obj: fk4fk5.c kep.h
	cl $(CFLAGS) fk4fk5.c

kepjpl.obj: kepjpl.c kep.h
	cl $(CFLAGS) kepjpl.c

kepi.obj: kepi.c kep.h
	cl $(CFLAGS) kepi.c

kepj.obj: kepj.c kep.h
	cl $(CFLAGS) kepj.c

kfiles.obj: kfiles.c kep.h
	cl $(CFLAGS) kfiles.c

lightt.obj: lightt.c kep.h
	cl $(CFLAGS) lightt.c

lonlat.obj: lonlat.c kep.h
	cl $(CFLAGS) lonlat.c

nutate.obj: nutate.c kep.h
	cl $(CFLAGS) nutate.c

precess.obj: precess.c kep.h
	cl $(CFLAGS) precess.c

refrac.obj: refrac.c kep.h
	cl $(CFLAGS) refrac.c

rplanet.obj: rplanet.c kep.h
	cl $(CFLAGS) rplanet.c

rstar.obj: rstar.c kep.h
	cl $(CFLAGS) rstar.c

sidrlt.obj: sidrlt.c kep.h
	cl $(CFLAGS) sidrlt.c

sun.obj: sun.c kep.h
	cl $(CFLAGS) sun.c

trnsit.obj: trnsit.c kep.h
	cl $(CFLAGS) trnsit.c

vearth.obj: vearth.c kep.h
	cl $(CFLAGS) vearth.c

zatan2.obj: zatan2.c kep.h
	cl $(CFLAGS) zatan2.c

moonjpl.obj: moonjpl.c kep.h
	cl $(CFLAGS) moonjpl.c

set3.obj: set3.c kep.h
	cl $(CFLAGS) set3.c

tdb.obj: tdb.c kep.h
	cl $(CFLAGS) tdb.c

oparams.obj: oparams.c kep.h
	cl $(CFLAGS) oparams.c

rotate.obj: rotate.c kep.h
	cl $(CFLAGS) rotate.c

testpo.obj: testpo.c
	cl $(CFLAGS) testpo.c

aa200.exe: aa200.obj altaz.obj angles.obj annuab.obj constel.obj deflec.obj \
deltat.obj diurab.obj diurpx.obj dms.obj epsiln.obj fk4fk5.obj kepjpl.obj \
kepi.obj kepj.obj kfiles.obj lightt.obj lonlat.obj nutate.obj precess.obj \
refrac.obj rplanet.obj rstar.obj sidrlt.obj sun.obj tdb.obj trnsit.obj \
vearth.obj zatan2.obj moonjpl.obj set3.obj oparams.obj rotate.obj kep.h
	link /STACK:16384 @aa200.rsp

testpo.exe: testpo.obj kepjpl.obj oparams.obj lonlat.obj \
epsiln.obj  nutate.obj precess.obj dms.obj zatan2.obj
	link testpo kepjpl oparams lonlat epsiln nutate precess dms zatan2;
