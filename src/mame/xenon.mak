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

$(DRIVERS)/neogeo.o:	$(MAMESRC)/drivers/neodrvr.c

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

$(DRIVERS)/30test.o:	$(LAYOUT)/30test.lh

$(DRIVERS)/8080bw.o:	$(LAYOUT)/invrvnge.lh \
			$(LAYOUT)/shuttlei.lh \
			$(LAYOUT)/cosmicm.lh

$(DRIVERS)/acefruit.o:	$(LAYOUT)/sidewndr.lh

$(DRIVERS)/amaticmg.o:	$(LAYOUT)/suprstar.lh

$(DRIVERS)/ampoker2.o:	$(LAYOUT)/ampoker2.lh \
			$(LAYOUT)/sigmapkr.lh \

$(DRIVERS)/aristmk4.o:	$(LAYOUT)/aristmk4.lh \
			$(LAYOUT)/arimk4nz.lh \
			$(LAYOUT)/3bagflnz.lh \
			$(LAYOUT)/3bagflvt.lh \
			$(LAYOUT)/arcwins.lh \
			$(LAYOUT)/cgold2.lh \
			$(LAYOUT)/eforest.lh \
			$(LAYOUT)/fhunter.lh \
			$(LAYOUT)/goldenc.lh \
			$(LAYOUT)/kgbird.lh \
			$(LAYOUT)/topgear.lh \
			$(LAYOUT)/wildone.lh \
			$(LAYOUT)/gldnpkr.lh \

$(DRIVERS)/astrocde.o:	$(LAYOUT)/gorf.lh \
			$(LAYOUT)/tenpindx.lh

$(DRIVERS)/atarifb.o:	$(LAYOUT)/atarifb.lh \
			$(LAYOUT)/atarifb4.lh \
			$(LAYOUT)/abaseb.lh

$(DRIVERS)/avalnche.o:	$(LAYOUT)/avalnche.lh

$(DRIVERS)/balsente.o:	$(LAYOUT)/stocker.lh

$(DRIVERS)/beaminv.o:	$(LAYOUT)/beaminv.lh

$(DRIVERS)/bfm_sc1.o:	$(LAYOUT)/bfm_sc1.lh

$(DRIVERS)/bfm_sc2.o:	$(LAYOUT)/bfm_sc2.lh \
			$(LAYOUT)/awpdmd.lh \
			$(LAYOUT)/awpvid14.lh \
			$(LAYOUT)/awpvid16.lh \
			$(LAYOUT)/drwho.lh \
			$(LAYOUT)/gldncrwn.lh \
			$(LAYOUT)/quintoon.lh \
			$(LAYOUT)/paradice.lh \
			$(LAYOUT)/pyramid.lh \
			$(LAYOUT)/pokio.lh \
			$(LAYOUT)/slots.lh \
			$(LAYOUT)/sltblgpo.lh \
			$(LAYOUT)/sltblgtk.lh

$(DRIVERS)/bfm_sc4.o:	$(LAYOUT)/bfm_sc4.lh

$(DRIVERS)/bfm_sc4h.o:	$(LAYOUT)/bfm_sc4.lh

$(DRIVERS)/blockade.o:	$(LAYOUT)/blockade.lh

$(DRIVERS)/buggychl.o:	$(LAYOUT)/buggychl.lh

$(DRIVERS)/bzone.o:	$(LAYOUT)/bzone.lh

$(DRIVERS)/cardline.o:	$(LAYOUT)/cardline.lh

$(DRIVERS)/cdi.o:	$(LAYOUT)/cdi.lh

$(DRIVERS)/chance32.o:	$(LAYOUT)/chance32.lh

$(DRIVERS)/changela.o:	$(LAYOUT)/changela.lh

$(DRIVERS)/chqflag.o:	$(LAYOUT)/chqflag.lh

$(DRIVERS)/cinemat.o:	$(LAYOUT)/armora.lh \
			$(LAYOUT)/solarq.lh \
			$(LAYOUT)/starcas.lh

$(DRIVERS)/cischeat.o:	$(LAYOUT)/cischeat.lh \
			$(LAYOUT)/f1gpstar.lh

$(DRIVERS)/circus.o:	$(LAYOUT)/circus.lh \
			$(LAYOUT)/crash.lh

$(DRIVERS)/copsnrob.o:	$(LAYOUT)/copsnrob.lh

$(DRIVERS)/corona.o:	$(LAYOUT)/re800.lh \
			$(LAYOUT)/luckyrlt.lh

$(DRIVERS)/darius.o:	$(LAYOUT)/darius.lh

$(DRIVERS)/destroyr.o:	$(LAYOUT)/destroyr.lh

$(DRIVERS)/dlair.o:	$(LAYOUT)/dlair.lh

$(DRIVERS)/firebeat.o:	$(LAYOUT)/firebeat.lh

$(DRIVERS)/fortecar.o:	$(LAYOUT)/fortecrd.lh

$(DRIVERS)/funworld.o:	$(LAYOUT)/jollycrd.lh \
			$(LAYOUT)/bigdeal.lh \
			$(LAYOUT)/novoplay.lh \
			$(LAYOUT)/royalcrd.lh

$(DRIVERS)/galaxi.o:	$(LAYOUT)/galaxi.lh

$(DRIVERS)/gatron.o:	$(LAYOUT)/poker41.lh \
			$(LAYOUT)/pulltabs.lh

$(DRIVERS)/globalfr.o:	$(LAYOUT)/globalfr.lh

$(DRIVERS)/goldnpkr.o:	$(LAYOUT)/goldnpkr.lh \
			$(LAYOUT)/pmpoker.lh \
			$(LAYOUT)/upndown.lh

$(DRIVERS)/goldstar.o:	$(LAYOUT)/lucky8.lh \
			$(LAYOUT)/bingowng.lh

$(DRIVERS)/grchamp.o:	$(LAYOUT)/grchamp.lh

$(DRIVERS)/highvdeo.o:	$(LAYOUT)/fashion.lh

$(DRIVERS)/icecold.o:	$(LAYOUT)/icecold.lh

$(DRIVERS)/igspoker.o:	$(LAYOUT)/igspoker.lh

$(DRIVERS)/jankenmn.o:	$(LAYOUT)/jankenmn.lh

$(DRIVERS)/kas89.o:	$(LAYOUT)/kas89.lh

$(DRIVERS)/kingdrby.o:	$(LAYOUT)/kingdrby.lh

$(DRIVERS)/kungfur.o:	$(LAYOUT)/kungfur.lh

$(DRIVERS)/lazercmd.o:	$(LAYOUT)/lazercmd.lh

$(DRIVERS)/luckgrln.o:	$(LAYOUT)/luckgrln.lh

$(DRIVERS)/lucky74.o:	$(LAYOUT)/lucky74.lh

$(DRIVERS)/magic10.o:	$(LAYOUT)/sgsafari.lh \
			$(LAYOUT)/musicsrt.lh

$(DRIVERS)/majorpkr.o:	$(LAYOUT)/majorpkr.lh

$(DRIVERS)/maxaflex.o:	$(LAYOUT)/maxaflex.lh

$(DRIVERS)/mcr3.o:	$(LAYOUT)/turbotag.lh

$(DRIVERS)/mpoker.o:	$(LAYOUT)/mpoker.lh

$(DRIVERS)/mpu4.o:	$(LAYOUT)/mpu4.lh \
			$(LAYOUT)/connect4.lh \
			$(LAYOUT)/mpu4ext.lh \
			$(LAYOUT)/gamball.lh

$(DRIVERS)/mpu4vid.o:	$(LAYOUT)/crmaze2p.lh \
			$(LAYOUT)/crmaze4p.lh

$(DRIVERS)/mw18w.o:	$(LAYOUT)/18w.lh

$(DRIVERS)/mw8080bw.o:	$(LAYOUT)/280zzzap.lh \
			$(LAYOUT)/clowns.lh \
			$(LAYOUT)/invaders.lh \
			$(LAYOUT)/invad2ct.lh \
			$(LAYOUT)/lagunar.lh \
			$(LAYOUT)/spacwalk.lh

$(DRIVERS)/meadows.o:	$(LAYOUT)/deadeye.lh \
			$(LAYOUT)/gypsyjug.lh

$(DRIVERS)/meyc8080.o:	$(LAYOUT)/wldarrow.lh \
			$(LAYOUT)/mdrawpkr.lh \
			$(LAYOUT)/meybjack.lh

$(DRIVERS)/meyc8088.o:	$(LAYOUT)/gldarrow.lh

$(DRIVERS)/midzeus.o:	$(LAYOUT)/crusnexo.lh

$(DRIVERS)/mil4000.o:	$(LAYOUT)/mil4000.lh

$(DRIVERS)/namcofl.o:	$(LAYOUT)/namcofl.lh

$(DRIVERS)/nbmj8688.o:	$(LAYOUT)/nbmj8688.lh

$(DRIVERS)/namcos2.o:	$(LAYOUT)/finallap.lh

$(DRIVERS)/neogeo.o:	$(LAYOUT)/neogeo.lh

$(DRIVERS)/norautp.o:	$(LAYOUT)/noraut11.lh \
			$(LAYOUT)/noraut12.lh

$(DRIVERS)/overdriv.o:	$(LAYOUT)/overdriv.lh

$(DRIVERS)/peplus.o:	$(LAYOUT)/peplus.lh \
			$(LAYOUT)/pe_schip.lh \
			$(LAYOUT)/pe_poker.lh \
			$(LAYOUT)/pe_bjack.lh \
			$(LAYOUT)/pe_keno.lh \
			$(LAYOUT)/pe_slots.lh

$(DRIVERS)/polepos.o:	$(LAYOUT)/polepos.lh \
			$(LAYOUT)/topracer.lh

$(DRIVERS)/qix.o:	$(LAYOUT)/elecyoyo.lh

$(DRIVERS)/quizshow.o:	$(LAYOUT)/quizshow.lh

$(DRIVERS)/re900.o:	$(LAYOUT)/re900.lh

$(DRIVERS)/roul.o:	$(LAYOUT)/roul.lh

$(DRIVERS)/sbrkout.o:	$(LAYOUT)/sbrkout.lh

$(DRIVERS)/sderby.o:	$(LAYOUT)/sderby.lh \
			$(LAYOUT)/spacewin.lh \
			$(LAYOUT)/pmroulet.lh

$(DRIVERS)/segaorun.o:	$(LAYOUT)/outrun.lh

$(DRIVERS)/segas32.o:	$(LAYOUT)/radr.lh

$(DRIVERS)/segasms.o:	$(LAYOUT)/sms1.lh

$(DRIVERS)/segaybd.o:	$(LAYOUT)/pdrift.lh

$(DRIVERS)/snookr10.o:	$(LAYOUT)/snookr10.lh

$(DRIVERS)/splus.o:	$(LAYOUT)/splus.lh

$(DRIVERS)/sspeedr.o:	$(LAYOUT)/sspeedr.lh

$(DRIVERS)/stactics.o:	$(LAYOUT)/stactics.lh

$(DRIVERS)/sstrangr.o:	$(LAYOUT)/sstrangr.lh

$(DRIVERS)/subsino.o:	$(LAYOUT)/victor5.lh \
			$(LAYOUT)/victor21.lh \
			$(LAYOUT)/tisub.lh \
			$(LAYOUT)/stisub.lh \
			$(LAYOUT)/crsbingo.lh \
			$(LAYOUT)/sharkpy.lh \
			$(LAYOUT)/sharkpye.lh \
			$(LAYOUT)/smoto.lh

$(DRIVERS)/superchs.o:	$(LAYOUT)/superchs.lh

$(DRIVERS)/sfbonus.o:	$(LAYOUT)/pirpok2.lh

$(DRIVERS)/taito_z.o:	$(LAYOUT)/contcirc.lh \
			$(LAYOUT)/dblaxle.lh

$(DRIVERS)/tatsumi.o:	$(LAYOUT)/roundup5.lh

$(DRIVERS)/tceptor.o:	$(LAYOUT)/tceptor2.lh

$(DRIVERS)/tehkanwc.o:	$(LAYOUT)/gridiron.lh

$(DRIVERS)/tetrisp2.o:	$(LAYOUT)/rocknms.lh \
			$(LAYOUT)/stepstag.lh

$(DRIVERS)/thayers.o:	$(LAYOUT)/dlair.lh

$(DRIVERS)/topspeed.o:	$(LAYOUT)/topspeed.lh

$(DRIVERS)/turbo.o:	$(LAYOUT)/turbo.lh \
			$(LAYOUT)/subroc3d.lh \
			$(LAYOUT)/buckrog.lh

$(DRIVERS)/tx1.o:	$(LAYOUT)/buggybjr.lh \
			$(LAYOUT)/buggyboy.lh \
			$(LAYOUT)/tx1.lh

$(DRIVERS)/umipoker.o:	$(LAYOUT)/saiyukip.lh

$(DRIVERS)/undrfire.o:	$(LAYOUT)/cbombers.lh

$(DRIVERS)/vicdual.o:	$(LAYOUT)/depthch.lh

$(DRIVERS)/videopin.o:	$(LAYOUT)/videopin.lh

$(DRIVERS)/videopkr.o:	$(LAYOUT)/videopkr.lh \
			$(LAYOUT)/blckjack.lh \
			$(LAYOUT)/videodad.lh \
			$(LAYOUT)/videocba.lh \
			$(LAYOUT)/babypkr.lh \
			$(LAYOUT)/babydad.lh

$(DRIVERS)/warpwarp.o:	$(LAYOUT)/geebee.lh \
			$(LAYOUT)/navarone.lh \
			$(LAYOUT)/sos.lh

$(DRIVERS)/wecleman.o:	$(LAYOUT)/wecleman.lh

$(DRIVERS)/zac2650.o:	$(LAYOUT)/tinv2650.lh

$(DRIVERS)/peyper.o:    $(LAYOUT)/peyper.lh