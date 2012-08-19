###########################################################################
#
#   tiny.mak
#
#   Small driver-specific example makefile
#	Use make SUBTARGET=tiny to build
#
#   Copyright Nicola Salmoria and the MAME Team.
#   Visit  http://mamedev.org for licensing and usage restrictions.
#
###########################################################################

MAMESRC = $(SRC)/mame
MAMEOBJ = $(OBJ)/mame

AUDIO = $(MAMEOBJ)/audio
DRIVERS = $(MAMEOBJ)/drivers
LAYOUT = $(MAMEOBJ)/layout
MACHINE = $(MAMEOBJ)/machine
VIDEO = $(MAMEOBJ)/video

OBJDIRS += \
	$(AUDIO) \
	$(DRIVERS) \
	$(LAYOUT) \
	$(MACHINE) \
	$(VIDEO) \



#-------------------------------------------------
# Specify all the CPU cores necessary for the
# drivers referenced in tiny.c.
#-------------------------------------------------
CPUS += I86
CPUS += I386
CPUS += NEC
CPUS += V30MZ
CPUS += V60
CPUS += Z80
CPUS += M6502
CPUS += MCS48
CPUS += MCS51
CPUS += M6800
CPUS += M6809
CPUS += M680X0
CPUS += TMS9900
CPUS += COP400
CPUS += PIC16C5X
CPUS += I8085
CPUS += TMS340X0

#-------------------------------------------------
# Specify all the sound cores necessary for the
# drivers referenced in tiny.c.
#-------------------------------------------------

SOUNDS += SAMPLES
SOUNDS += DAC
SOUNDS += DISCRETE
SOUNDS += AY8910
SOUNDS += ASTROCADE
SOUNDS += TMS5220
SOUNDS += OKIM6295
SOUNDS += HC55516
SOUNDS += CEM3394
SOUNDS += YM2151
SOUNDS += YM2203
SOUNDS += YM2413
SOUNDS += YM2608
SOUNDS += YM2610
SOUNDS += YM2610B
SOUNDS += YM2612
SOUNDS += YM3438
SOUNDS += YM3812
SOUNDS += YM3526
SOUNDS += Y8950
SOUNDS += YMF262
SOUNDS += YMF271
SOUNDS += YMF278B
SOUNDS += YMZ280B
SOUNDS += SN76477
SOUNDS += SN76496
SOUNDS += UPD7759
SOUNDS += QSOUND
SOUNDS += MSM5205
SOUNDS += SEGAPCM
SOUNDS += MULTIPCM
SOUNDS += RF5C68
SOUNDS += RF5C400
SOUNDS += SP0250
SOUNDS += TMS36XX
SOUNDS += TMS3615
SOUNDS += TMS5110
SOUNDS += TMS5220

#-------------------------------------------------
# the following files are general components and
# shared across a number of drivers
#-------------------------------------------------

$(MAMEOBJ)/shared.a: \
	$(MACHINE)/nmk112.o \
	$(MACHINE)/pckeybrd.o \
	$(MACHINE)/pcshare.o \
	$(MACHINE)/segacrpt.o \
	$(MACHINE)/segacrp2.o \
	$(MACHINE)/ticket.o \
	$(VIDEO)/avgdvg.o \

# CPS

$(MAMEOBJ)/cps.a: \
	$(DRIVERS)/cps1.o $(VIDEO)/cps1.o \
	$(DRIVERS)/cps2.o \
	$(MACHINE)/cps2crpt.o \
	$(MACHINE)/kabuki.o \

$(MAMEOBJ)/neogeo.a: \
	$(DRIVERS)/neogeo.o $(VIDEO)/neogeo.o \
	$(MACHINE)/neoboot.o \
	$(MACHINE)/neocrypt.o \
	$(MACHINE)/neoprot.o \
	
$(MAMEOBJ)/sega.a: \
	$(DRIVERS)/angelkds.o $(VIDEO)/angelkds.o \
	$(DRIVERS)/bingoc.o \
	$(DRIVERS)/blockade.o $(AUDIO)/blockade.o $(VIDEO)/blockade.o \
	$(DRIVERS)/calorie.o \
	$(DRIVERS)/coolridr.o \
	$(DRIVERS)/deniam.o $(VIDEO)/deniam.o \
	$(DRIVERS)/dotrikun.o \
	$(VIDEO)/genesis.o \
	$(DRIVERS)/gpworld.o \
	$(DRIVERS)/hikaru.o \
	$(DRIVERS)/hshavoc.o \
	$(DRIVERS)/kopunch.o $(VIDEO)/kopunch.o \
	$(MACHINE)/megadriv.o \
	$(MACHINE)/megacd.o \
	$(MACHINE)/mega32x.o \
	$(MACHINE)/megasvp.o \
	$(MACHINE)/megavdp.o \
	$(MACHINE)/md_cart.o \
	$(DRIVERS)/megadrvb.o \
	$(DRIVERS)/megaplay.o \
	$(DRIVERS)/megatech.o \
    $(AUDIO)/dsbz80.o \
	$(DRIVERS)/puckpkmn.o \
	$(DRIVERS)/segac2.o \
	$(DRIVERS)/segae.o $(MACHINE)/segamsys.o \
	$(DRIVERS)/shtzone.o \
	$(DRIVERS)/segag80r.o $(MACHINE)/segag80.o $(AUDIO)/segag80r.o $(VIDEO)/segag80r.o \
	$(DRIVERS)/segag80v.o $(AUDIO)/segag80v.o $(VIDEO)/segag80v.o \
	$(DRIVERS)/segahang.o $(VIDEO)/segahang.o \
	$(DRIVERS)/segajw.o \
	$(DRIVERS)/segald.o \
	$(DRIVERS)/segaorun.o $(VIDEO)/segaorun.o \
	$(DRIVERS)/segas16a.o $(VIDEO)/segas16a.o \
	$(DRIVERS)/segas16b.o $(VIDEO)/segas16b.o \
	$(DRIVERS)/segas18.o $(VIDEO)/segas18.o \
	$(DRIVERS)/segas24.o $(VIDEO)/segas24.o \
	$(DRIVERS)/segas32.o $(MACHINE)/segas32.o $(VIDEO)/segas32.o \
	$(DRIVERS)/segaxbd.o $(VIDEO)/segaxbd.o \
	$(DRIVERS)/segaybd.o $(VIDEO)/segaybd.o \
	$(DRIVERS)/sg1000a.o \
	$(DRIVERS)/stactics.o $(VIDEO)/stactics.o \
	$(DRIVERS)/suprloco.o $(VIDEO)/suprloco.o \
	$(DRIVERS)/system1.o $(VIDEO)/system1.o \
	$(DRIVERS)/system16.o $(VIDEO)/system16.o \
	$(DRIVERS)/timetrv.o \
	$(DRIVERS)/turbo.o $(AUDIO)/turbo.o $(VIDEO)/turbo.o \
	$(DRIVERS)/vicdual.o $(AUDIO)/vicdual.o $(VIDEO)/vicdual.o \
	$(DRIVERS)/zaxxon.o $(AUDIO)/zaxxon.o $(VIDEO)/zaxxon.o \
	$(MACHINE)/fd1089.o \
	$(MACHINE)/fd1094.o \
	$(MACHINE)/fddebug.o \
	$(MACHINE)/mc8123.o \
	$(MACHINE)/s16fd.o \
	$(MACHINE)/s24fd.o \
	$(MACHINE)/scudsp.o \
	$(MACHINE)/segaic16.o \
	$(AUDIO)/carnival.o \
	$(AUDIO)/depthch.o \
	$(AUDIO)/invinco.o \
	$(AUDIO)/pulsar.o \
	$(AUDIO)/segasnd.o \
	$(VIDEO)/segaic16.o \
	$(VIDEO)/sega16sp.o \
	$(VIDEO)/segaic24.o \

#-------------------------------------------------
# This is the list of files that are necessary
# for building all of the drivers referenced
# in tiny.c
#-------------------------------------------------

DRVLIBS = \
	$(EMUDRIVERS)/emudummy.o \
	$(MACHINE)/ticket.o \
	$(DRIVERS)/carpolo.o $(MACHINE)/carpolo.o $(VIDEO)/carpolo.o \
	$(DRIVERS)/circus.o $(AUDIO)/circus.o $(VIDEO)/circus.o \
	$(DRIVERS)/exidy.o $(AUDIO)/exidy.o $(VIDEO)/exidy.o \
	$(AUDIO)/exidy440.o \
	$(DRIVERS)/starfire.o $(VIDEO)/starfire.o \
	$(DRIVERS)/vertigo.o $(MACHINE)/vertigo.o $(VIDEO)/vertigo.o \
	$(DRIVERS)/victory.o $(VIDEO)/victory.o \
	$(AUDIO)/targ.o \
	$(DRIVERS)/astrocde.o $(VIDEO)/astrocde.o \
	$(DRIVERS)/gridlee.o $(AUDIO)/gridlee.o $(VIDEO)/gridlee.o \
	$(DRIVERS)/williams.o $(MACHINE)/williams.o $(AUDIO)/williams.o $(VIDEO)/williams.o \
	$(AUDIO)/gorf.o \
	$(AUDIO)/wow.o \
	$(DRIVERS)/gaelco.o $(VIDEO)/gaelco.o $(MACHINE)/gaelcrpt.o \
	$(DRIVERS)/wrally.o $(MACHINE)/wrally.o $(VIDEO)/wrally.o \
	$(DRIVERS)/looping.o \
	$(DRIVERS)/supertnk.o \
	$(MAMEOBJ)/cps.a \
	$(MAMEOBJ)/neogeo.a \
	$(MAMEOBJ)/sega.a \
	$(MAMEOBJ)/shared.a \



#-------------------------------------------------
# layout dependencies
#-------------------------------------------------

$(DRIVERS)/astrocde.o:	$(LAYOUT)/gorf.lh \
						$(LAYOUT)/tenpindx.lh
$(DRIVERS)/circus.o:	$(LAYOUT)/circus.lh \
						$(LAYOUT)/crash.lh
