Date: Wed, 11 Jan 1995 18:24:06 PST
From: gtalge@orion.lasierra.edu
To: moshier@world.std.com
Message-Id: <0098A508.FC64B700.18649@orion.lasierra.edu>
Subject: Turbo C make file for aa200

Steve,
I put in your changes that you sent me. Still had some problems, but
I finally got aa200 to compile and run with Borland c++ ver 4.0.
All I can say is that it works. It's probably not set for the best
configurations though.  It will not link in the small model, so
I have set it for the large, and turned off some of the compiler
warnings.  The .EXE file is much bigger than the MSC .EXE even
with the debug information striped out.

                    Gordon


---------------cut here-----------------------

#
# Borland C++ IDE generated makefile
#
.AUTODEPEND


#
# Borland C++ tools
#
IMPLIB  = Implib
BCCDOS  = Bcc +BccDos.cfg 
TLINK   = TLink
TLIB    = TLib
TASM    = Tasm
#
# IDE macros
#


#
# Options
#
IDE_LFLAGSDOS =  -LC:\BC4\LIB
IDE_BFLAGS = 
LLATDOS_aa200dexe =  -c -Tde
RLATDOS_aa200dexe = 
BLATDOS_aa200dexe = 
LEAT_aa200dexe = $(LLATDOS_aa200dexe)
REAT_aa200dexe = $(RLATDOS_aa200dexe)
BEAT_aa200dexe = $(BLATDOS_aa200dexe)

#
# Dependency List
#
Dep_aa200 = \
   aa200.exe

aa200 : BccDos.cfg $(Dep_aa200)
  echo MakeNode aa200

Dep_aa200dexe = \
   kepjpl.obj\
   set3.obj\
   altaz.obj\
   angles.obj\
   annuab.obj\
   constel.obj\
   deflec.obj\
   deltat.obj\
   diurab.obj\
   diurpx.obj\
   dms.obj\
   epsiln.obj\
   fk4fk5.obj\
   kepi.obj\
   kepj.obj\
   kfiles.obj\
   lightt.obj\
   lonlat.obj\
   moonjpl.obj\
   nutate.obj\
   oparams.obj\
   precess.obj\
   refrac.obj\
   rplanet.obj\
   rstar.obj\
   sidrlt.obj\
   sun.obj\
   tdb.obj\
   trnsit.obj\
   vearth.obj\
   zatan2.obj\
   zscsi.obj\
   aa200.obj

aa200.exe : $(Dep_aa200dexe)
  $(TLINK)   @&&|
 /v $(IDE_LFLAGSDOS) $(LEAT_aa200dexe) +
C:\BC4\LIB\c0l.obj+
kepjpl.obj+
set3.obj+
altaz.obj+
angles.obj+
annuab.obj+
constel.obj+
deflec.obj+
deltat.obj+
diurab.obj+
diurpx.obj+
dms.obj+
epsiln.obj+
fk4fk5.obj+
kepi.obj+
kepj.obj+
kfiles.obj+
lightt.obj+
lonlat.obj+
moonjpl.obj+
nutate.obj+
oparams.obj+
precess.obj+
refrac.obj+
rplanet.obj+
rstar.obj+
sidrlt.obj+
sun.obj+
tdb.obj+
trnsit.obj+
vearth.obj+
zatan2.obj+
zscsi.obj+
aa200.obj
$<,$*
C:\BC4\LIB\bidsl.lib+
C:\BC4\LIB\emu.lib+
C:\BC4\LIB\mathl.lib+
C:\BC4\LIB\cl.lib

|

kepjpl.obj :  kepjpl.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ kepjpl.c

set3.obj :  set3.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ set3.c

altaz.obj :  altaz.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ altaz.c

angles.obj :  angles.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ angles.c

annuab.obj :  annuab.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ annuab.c

constel.obj :  constel.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ constel.c

deflec.obj :  deflec.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ deflec.c

deltat.obj :  deltat.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ deltat.c

diurab.obj :  diurab.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ diurab.c

diurpx.obj :  diurpx.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ diurpx.c

dms.obj :  dms.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ dms.c

epsiln.obj :  epsiln.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ epsiln.c

fk4fk5.obj :  fk4fk5.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ fk4fk5.c

kepi.obj :  kepi.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ kepi.c

kepj.obj :  kepj.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ kepj.c

kfiles.obj :  kfiles.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ kfiles.c

lightt.obj :  lightt.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ lightt.c

lonlat.obj :  lonlat.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ lonlat.c

moonjpl.obj :  moonjpl.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ moonjpl.c

nutate.obj :  nutate.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ nutate.c

oparams.obj :  oparams.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ oparams.c

precess.obj :  precess.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ precess.c

refrac.obj :  refrac.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ refrac.c

rplanet.obj :  rplanet.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ rplanet.c

rstar.obj :  rstar.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ rstar.c

sidrlt.obj :  sidrlt.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ sidrlt.c

sun.obj :  sun.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ sun.c

tdb.obj :  tdb.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ tdb.c

trnsit.obj :  trnsit.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ trnsit.c

vearth.obj :  vearth.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ vearth.c

zatan2.obj :  zatan2.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ zatan2.c

zscsi.obj :  zscsi.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ zscsi.c

aa200.obj :  aa200.c
  $(BCCDOS) -P- -c $(CEAT_aa200dexe) -o$@ aa200.c

# Compiler configuration file
BccDos.cfg : 
   Copy &&|
-W-
-R
-v
-vi
-X-
-H
-IC:\BC4\INCLUDE
-H=aa200.csm
-A-
-ml
-f
-DSSYSTEM;
-A
-H-
-w-nak
-w-pro
-N
-3
-a
-v-
-R-
-Ob
-O-p
-Oi
-O-v
| $@



