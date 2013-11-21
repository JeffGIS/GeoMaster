# test directory.

# This makes sure we are testing the correct programs.
PATH := $(IPW)/bin:$(IPW)/etc:$(IPW)/tests/Bin:/bin:/usr/bin:/usr/ucb
#
# I would like to do an 'export PATH' here, but some people may have
# GNU Make version 3.62, which doesn't understand such things.
.EXPORT_ALL_VARIABLES:

ORIG_PROG = \
	bitcom cmpimg cnhist convolve demux dither edimg flip    \
	gradient grhist hist histeq hor1d horizon interp ipw2hds \
	ipw2ps ipw2sun ipw2xim lincom lqhx lutx mkbih mkgeoh     \
	mklqh mklut mksath mksunh mkwinh mstats mult mux poly    \
	prhdr primg random rmhdr shade skew sunang transpose     \
	viewcalc viewf wedge window zoom  \
	edhdr ipw

ETC_PROG = \
	atob btoa mc

ERLC_PROG = \
	addtime binexp cell2point classeq classify clmask cnhist.all  \
	cvtime declass despike ftransform gelevrad ggradient ghorizon \
	gshade gsunlight gtoporad gtoporad.24 gtopquad gviewf hcnx    \
	iadd idewpt imgstat imgsum import indvi ipott ipw2pgm ipw2ppm \
	ipw2pgm ipw2ppm ipwinterp isatvp isling isnobal istatgraph    \
	istatrel itrbxfr izpr lqmm mkalbers mkcrh mkgeo mklambert     \
	mklamcc mkproj mkutm pix2fp reproj reproj_area requant resamp \
	rh2vp rotate salbedo scale selevrad stoporad sunweights       \
	surface svop toporad.24 topotherm transcoord wbal             \
	ipw2las las2ipw

UCSB_PROG = \
	elevrad ipw2pbm randomize refdx seq solar sunlight toporad    \
	topquad twostream dempt tmpt

V10_PROG = \
	ipwfile pbm2ipw

PROGRAMS = \
	$(ORIG_PROG) $(ETC_PROG) $(UCSB_PROG) $(V10_PROG) $(ERLC_PROG)


all: base ucsb v10 erlc-fake
	@echo
	@echo "testing done"

clean:
	rm -f [a-z]*

.DEFAULT:
	@echo "    No test for program: $(@)"



base: $(ORIG_PROG) $(ETC_PROG)
	@echo
	@echo "testing of base IPW programs done"
	@echo

ucsb: $(UCSB_PROG)
	@echo
	@echo "testing of UCSB IPW programs done"
	@echo

v10: $(V10_PROG)
	@echo
	@echo "testing of V1.0 IPW programs done"
	@echo

erlc: $(ERLC_PROG)
	@echo
	@echo "testing of ERL-C IPW programs done"
	@echo





#
#-----------------------------------
#

#
# -- base
#

mux:
	@-mux Data/elev.78 Data/mask.alb.78 > elevmask.ipw
	@cmpverb -n mux elevmask.ipw
	@-mux Data/ch1.ipw Data/ch2.ipw | mux - Data/ch5.ipw > channels.ipw
	@cmpverb mux channels.ipw

bitcom:
	@-bitcom -a -m  < Outputs/elevmask.ipw > maskedelev.ipw
	@cmpverb bitcom maskedelev.ipw

cmpimg:
	@-cmpimg Data/ch1.ipw Data/ch1.ipw  > cmpr
	@-cmpimg Data/ch1.ipw Data/ch2.ipw  >> cmpr
	@cmpverb cmpimg cmpr

transpose:
	@-transpose Data/ch1.ipw > ch1t.ipw
	@cmpverb -n transpose ch1t.ipw
	@-transpose Data/ch1.ipw | transpose | cmpimg - Data/ch1.ipw > cmptr
	@cmpverb transpose cmptr

hist:
	@-hist -m Data/mask.alb.78 Data/elev.78 > emhist
	@cmpverb -n hist emhist
	@-hist Data/ndvi.ipw > ndvihist
	@cmpverb hist ndvihist

cnhist:
	@-cnhist < Outputs/emhist > cnh.txt
	@cmpverb cnhist cnh.txt

convolve:
	@-convolve -i Data/ndvi.ipw -c Data/conv.laplacian > lndvi.ipw
	@cmpverb -e convolve lndvi.ipw

demux:
	@-demux -b 1,0 Outputs/elevmask.ipw > maskelev.ipw
	@cmpverb demux maskelev.ipw

dither:
	@-dither -r 8 Data/ch1.ipw > ch1-d8.ipw
	@cmpverb dither ch1-d8.ipw

edimg:
	@-edimg -c Data/edimg-coords -i Data/ndvi.ipw > ndvi-ed.ipw
	@cmpverb edimg ndvi-ed.ipw

flip:
	@-flip -l -s Data/ch1.ipw > ch1-180.ipw
	@cmpverb flip ch1-180.ipw

gradient:
	@-gradient Data/elev.78 > grad.78
	@cmpverb -e gradient grad.78

grhist:
	@-grhist < Outputs/emhist > graph
	@cmpverb grhist graph

histeq:
	@-histeq < Outputs/ndvihist > histeq1
	@cmpverb -n histeq histeq1
	@-histeq -n 4096 < Outputs/emhist > histeq2
	@cmpverb -n histeq histeq2
	@-histeq -n 4096 -o 0,255 -i 1,4094 < Outputs/emhist > histeq3
	@cmpverb histeq histeq3

hor1d:
	@-hor1d -a -135       Data/elev.78 > hor.nw
	@cmpverb -n hor1d hor.nw
	@-hor1d -a -135 -b    Data/elev.78 > hor.nwb
	@cmpverb -n hor1d hor.nwb
	@-hor1d -a -135 -z 88 Data/elev.78 > hor.nw88
	@cmpverb hor1d hor.nw88

horizon:
	@-horizon -a 180 Data/elev.78 > hor.n
	@cmpverb horizon hor.n

interp:
	@-interp < Data/interp1 > int1
	@cmpverb -n interp int1
	@-interp < Data/interp2 > int2
	@cmpverb interp int2

ipw2hds:
	@-ipw2hds < Outputs/ch1-d8.ipw > ch1-d8.hds
	@cmpverb ipw2hds ch1-d8.hds

ipw2ps:
	@-ipw2ps Data/ch2.ipw > ch2.ps
	@cmpverb -n ipw2ps ch2.ps
	@-ipw2ps -r Data/ch2.ipw > ch2r.ps
	@cmpverb ipw2ps ch2r.ps

ipw2sun:
	@-ipw2sun Outputs/ch1-d8.ipw > ch1-d8.ras
	@cmpverb -n ipw2sun ch1-d8.ras
	@-ipw2sun Data/ch2.ipw > ch2.ras
	@cmpverb ipw2sun ch2.ras

ipw2xim:
	@-ipw2xim Data/ch2.ipw > ch2.xim
	@cmpverb ipw2xim ch2.xim

lincom:
	@-lincom < Outputs/grad.78 > gradave
	@cmpverb -n -e lincom gradave
	@-lincom -c 1,-1 < Outputs/grad.78 > gradsub
	@cmpverb -n -e lincom gradsub
	@-lincom -c .5,.5,1,-1 < Outputs/grad.78 > gradsumdiff
	@cmpverb -e lincom gradsumdiff

lqhx:
	@-lqhx -h Data/rad.10.ipw -i Data/rad.04.ipw > rad.04.res.ipw
	@cmpverb -n lqhx rad.04.res.ipw
	@-lqhx -i Data/ch1.ipw < Outputs/head1 > ch1-lq.ipw
	@cmpverb lqhx ch1-lq.ipw

mkbih:
	@-mkbih -s 1 -l 1 -i 10 -f > head0
	@cmpverb -n mkbih head0
	@-mkbih -l 64 -s 64 -f > head64
	@cmpverb -n mkbih head64
	@-mkbih -l 512 -s 512 -f > head512
	@cmpverb mkbih head512

mklqh:
	@-mklqh -m 0,0,1023,1 -f < Outputs/head0 > head1
	@cmpverb -n mklqh head1
	@-mklqh -m 0,462,1023,895 -u "W m^-2" -f < Outputs/head0 > head3
	@cmpverb mklqh head3

lutx:
	@-lutx -i Data/ndvi.ipw < Outputs/histeq1 > ndvi-heq.ipw
	@cmpverb -n lutx ndvi-heq.ipw
	@-lutx -i Data/elev.78 < Outputs/histeq2 > elev-heq.ipw
	@cmpverb -n lutx elev-heq.ipw
	@-lutx -i Data/elev.78 < Outputs/lut12 > elev.78-8
	@cmpverb lutx elev.78-8

mklut:
	@-mklut -i 12 < Outputs/int2 > lut12
	@cmpverb mklut lut12

mkgeoh:
	@-mkgeoh -c utm -u meters -o 4051800,349350 -d 5,5 -f \
	 < Outputs/head1 > head2
	@cmpverb mkgeoh head2

mksath:
	@-mksath -p EOS1 -s TM -l "North America" -d 19921201 -t 221510.213 -f \
	 < Outputs/head3 > head4
	@cmpverb mksath head4

mksunh:
	@-mksunh -z 0.838671 -a 1.064651 -f < Outputs/head4 > head5
	@cmpverb mksunh head5

mkwinh:
	@-mkwinh -b 50,70 Data/ch1.ipw > ch1-5070.ipw
	@cmpverb mkwinh ch1-5070.ipw

mstats:
	@-mstats -c Data/avhrr-mask.ipw Outputs/channels.ipw > chanstats
	@cmpverb mstats chanstats

mult:
	@-mult Outputs/grad.78  > mult-grad
	@cmpverb -n -e mult mult-grad
	@-mult -r 2 < Outputs/channels.ipw > mult-chans
	@cmpverb -e mult mult-chans

poly:
	@-poly < Data/poly1 > opoly1
	@cmpverb -n poly opoly1
	@-poly -s 15,15 < Data/poly2 > opoly2
	@cmpverb poly opoly2

prhdr:
	@-prhdr Data/ch1.ipw > ch1.hdr
	@cmpverb prhdr ch1.hdr

rmhdr:
	@-rmhdr Data/elev.78 > elev.78.data
	@cmpverb rmhdr elev.78.data

primg:
	@-primg -r -a -i Data/elev.78 > elev.78.asc
	@cmpverb primg elev.78.asc

skew:
	@-skew -a 30 Data/ch1.ipw > ch1-skew30.ipw
	@cmpverb skew ch1-skew30.ipw

window:
	@-window -c 48,140 -n 48,140 Data/ch1.ipw  > ch1-win.ipw
	@cmpverb -n window ch1-win.ipw
	@-window -g 0 -b 2875000,-1655000 -e 2825000,-1605000 Data/elev.78 \
          > elev-gwin.ipw
	@cmpverb window elev-gwin.ipw

zoom:
	@-zoom -s -20 -l -4 Data/ch1.ipw  > ch1-zoom1.ipw
	@cmpverb -n zoom ch1-zoom1.ipw
	@-zoom -s 2 -l 3 Outputs/ch1-zoom1.ipw  > ch1-zoom2.ipw
	@cmpverb -n zoom ch1-zoom2.ipw
	@-zoom -s 20 -l -40 Outputs/channels.ipw  > channels-zoom.ipw
	@cmpverb zoom channels-zoom.ipw

wedge:
	@-wedge -c 1,1 -n 8 < Outputs/head64 > wedge64.ipw
	@cmpverb -n -e wedge wedge64.ipw
	@-wedge -c 0,1,0,-1 -n 12 Data/elev.78 > wedge-elev.ipw
	@cmpverb -e wedge wedge-elev.ipw

random:
	@-random -s 5 -n 100 -r 0,100,0,100 > random500
	@cmpverb random random500

sunang:
	@-sunang -b 34,25 -l -119,54 -t 1990,2,15,12,30 -z 480 -d -f > ucsb-sun
	@cmpverb sunang ucsb-sun

shade:
	@-shade -z 45 -a -135 < Outputs/grad.78  > shade1.78
	@cmpverb -n shade shade1.78
	@-shade -u 0.680436 -a -5.413 < Outputs/grad.78  > shade2.78
	@cmpverb shade shade2.78

viewcalc:
	@-viewcalc -s Outputs/grad.78 -h Data/horizons.78  > skyview-terr.ipw
	@cmpverb -h viewcalc skyview-terr.ipw

viewf:
	@-viewf Data/elev.78  > skyview-terr.ipw
	@cmpverb -h viewf skyview-terr.ipw

edhdr:
	@echo "    Not testing program: $(@)"

ipw:
	@echo "    Not testing program: $(@)"


#
# -- etc
#

btoa:
	@-btoa -2 Outputs/elev.78.data > elev.78.asc
	@cmpverb btoa elev.78.asc

atob:
	@-atob -2 Outputs/elev.78.asc > elev.78.data
	@cmpverb atob elev.78.data

mc:
	@-mc -w 56 - < Outputs/random500 > random.mc
	@cmpverb mc random.mc

#
# -- ucsb
#

ipw2pbm:
	@-ipw2pbm Data/ch2.ipw > ch2.pbm
	@cmpverb ipw2pbm ch2.pbm

twostream:
	@-twostream -u .5 -t .5 -w .5 -g .5 -r .5 -s .5     > twomw
	@cmpverb -n twostream twomw
	@-twostream -u .5 -t .5 -w .5 -g .5 -r .5 -s .5 -d  > twoed
	@cmpverb twostream twoed

randomize:
	@-randomize -s 5 Outputs/ch1.hdr  > ch1.hdr-rand
	@cmpverb -n randomize ch1.hdr-rand
	@-cat Outputs/ch1.hdr | randomize -s 5  > ch1.hdr-rand
	@cmpverb randomize ch1.hdr-rand

seq:
	@-seq -r 200,600 -n 4       > seq1
	@cmpverb -n seq seq1
	@-seq -r 200,600 -d 100     > seq2
	@cmpverb -n seq seq2
	@-seq -r 0,10 -n 30 -s exp  > seq3
	@cmpverb seq seq3

elevrad:
	@-elevrad -z 100 -t 0.2 -w .85 -g .55 -r .155 -s 150.214 \
	          -u 0.816876 Data/elev.col  > col-bdrad
	@cmpverb elevrad col-bdrad

sunlight:
	@-sunlight -d 1990,6,22 -b 37 -l -112,30 -z 480  > col-sunlight
	@cmpverb -n sunlight col-sunlight
	@-sunlight -d 1990,6,22 -b 37 -l -112,30 -z 480 -q 21 > col-sunlightq
	@cmpverb sunlight col-sunlightq

solar:
	@-solar -d 1990,6,22 -w .58,.68  > col-solar
	@cmpverb solar col-solar

refdx:
	@-refdx -s ice,water < Data/refwl > refri1
	@cmpverb -n refdx refri1
	@-refdx -s ice,water -u cm < Data/refwl2 > refri1cm
	@cmpverb -n refdx refri1cm
	@-refdx -s water -t 100 < Data/refwl > refri2
	@cmpverb -n refdx refri2
	@-refdx -s ice   -t -55 < Data/refwl > refri3
	@cmpverb -n refdx refri3
	@-refdx -s ice,water -t -7.42 -d < Data/refwl > refdf
	@cmpverb refdx refdf

toporad:
	@-toporad Data/top-parms > top-incoming
	@cmpverb -n -e toporad top-incoming
	@-toporad -n Data/top-parms > top-net
	@cmpverb -e toporad top-net

topquad:
	@echo "    Testing topquad -- please wait."
	@-topquad -z 200 -t .2 -w .85 -g .55 -r .1244 -x .58,.68 -d 1990,6,22 \
	          -b 48 -l -119,30  Data/topq-parms   > topq-incoming
	@cmpverb topquad topq-incoming

dempt:
	@echo "    Not testing program: $(@)"

tmpt:
	@echo "    Not testing program: $(@)"

#
# -- v10
#

ipwfile:
	@-ipwfile Data/ch1.ipw Data/ch5.ipw Data/avhrr-elev.ipw \
	          Data/horizons.78 Data/ea.78 >ipwf-all
	@cmpverb ipwfile ipwf-all

pbm2ipw:
	@-pbm2ipw Outputs/ndvi.ppm >ndvi-ppm.ipw
	@cmpverb -n pbm2ipw ndvi-ppm.ipw
	@-pbm2ipw Outputs/ndvig.pgm >ndvi-pgm.ipw
	@cmpverb pbm2ipw ndvi-pgm.ipw

#
# -- erlc
#

erlc-fake:
	@echo ""
	@echo "Most of the ERL-C programs have not had test procedures added."
	@echo "Use 'make erlc' to test the ones that have."
	@echo ""

addtime:
	@-addtime -d 1990,6,22,9,0,0 -h 4.6 > addtime-jun22
	@cmpverb addtime addtime-jun22

binexp:
	@echo "    Not testing program: $(@)"

classify:
	@-classify -i Data/classvals Data/rad.10.ipw > clrad10.ipw
	@cmpverb classify clrad10.ipw

clmask:
	@-clmask -c 2 -i Outputs/clrad10.ipw > clradmask.ipw
	@cmpverb clmask clradmask.ipw

ghorizon:
	@-ghorizon -a 90.0 -m Data/maskbuf.geo Data/dem.geo >hzs.check
	@cmpverb ghorizon hzs.check

gviewf:
	@echo "    Testing gviewf -- please wait."
	@-gviewf -m Data/maskbuf.geo Data/dem.geo >views.out
	@cmpverb -h gviewf views.out

iadd:
	@-iadd Outputs/grad.78 > iadd-grad
	@cmpverb iadd iadd-grad

idewpt:
	@-idewpt -m Data/mask.alb.78 Data/ea.78 > dpt.78
	@cmpverb idewpt dpt.78

imgstat:
	@-imgstat Data/ch1.ipw >ch1.stats
	@cmpverb -n imgstat ch1.stats
	@-imgstat -c Data/ndc.ipw >ndvi.clsstats
	@cmpverb imgstat ndvi.clsstats

imgsum:	
	@-imgsum -m Data/mask.alb.78 Data/elev.78 > elev78-sum
	@cmpverb imgsum elev78-sum

ipw2pgm:
	@-ipw2pgm Data/ndvi.ipw > ndviG.pgm
	@cmpverb ipw2pgm ndviG.pgm

ipw2ppm:
	@-ipw2ppm -c Data/ndvi.cm Data/ndvi.ipw > ndvi.ppm
	@cmpverb -n ipw2ppm ndvi.ppm
	@-ipw2ppm -b -g Data/ndvi.ipw > ndvig.pgm
	@cmpverb ipw2ppm ndvig.pgm

ipwinterp:
	@-ipwinterp Data/ch1.ipw < Outputs/ch1-2 > ch2i.ipw
	@cmpverb -e ipwinterp ch2i.ipw

istatgraph:
	@echo "    Not testing program: $(@)"

istatrel:
	@-istatrel Data/ch1.ipw Data/ch2.ipw > ch1-2
	@cmpverb istatrel ch1-2

mkcrh:
	@-mkcrh -f -c Data/classvals -i Data/rad.10.ipw > col.crh
	@cmpverb mkcrh col.crh

requant:
	@-requant -b 8 Data/avhrr-elev.ipw > avelevb8.ipw
	@cmpverb -n requant avelevb8.ipw
	@-requant -m -500,10000 Data/avhrr-elev.ipw > avelevmc.ipw
	@cmpverb -e requant avelevmc.ipw

resamp:
	@-resamp -s 20 -l 4 Data/ch1.ipw  > ch1-resamp1.ipw
	@cmpverb -n resamp ch1-resamp1.ipw
	@-resamp -s 5 -l 5 -f max Data/ch2.ipw >ch2-resamp2.ipw
	@cmpverb resamp ch2-resamp2.ipw

rotate:
	@-rotate -a 14 Data/ndvi.ipw >ndvi14.ipw
	@cmpverb rotate ndvi14.ipw
