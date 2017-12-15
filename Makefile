# Makefile for Copper III Diku

# ODIR is where we keep object files 
ODIR	= $(ARCH)
CC	= gcc
CFLAGS	= -g -Wall -p -DCONFIG=\"$(ODIR)/config.h\"
LIBS	=

.c.o :
	$(CC) -c $(CFLAGS) -o $(ODIR)/$@ $<

#rumor.o only temporarily removed..

OBJS	= $(ODIR)/comm.o \
          $(ODIR)/db.o \
          $(ODIR)/act.adm.o \
          $(ODIR)/act.comm.o \
          $(ODIR)/act.org.o \
          $(ODIR)/act.nego.o \
          $(ODIR)/act.vio.o \
          $(ODIR)/act.oth.o \
          $(ODIR)/act.soc.o \
          $(ODIR)/interp.o \
          $(ODIR)/nanny.o \
          $(ODIR)/utility.o \
          $(ODIR)/specproc.o \
          $(ODIR)/modify.o \
          $(ODIR)/weather.o \
          $(ODIR)/spell.o \
          $(ODIR)/const.o \
          $(ODIR)/spec.gen.o \
          $(ODIR)/spec.oth.o \
          $(ODIR)/signals.o \
          $(ODIR)/board.o \
          $(ODIR)/org.o \
          $(ODIR)/cron.o \
          $(ODIR)/cronutil.o \
          $(ODIR)/debug.o \
          $(ODIR)/item.o \
          $(ODIR)/plot.o \
          $(ODIR)/event.o \
          $(ODIR)/player.o \
          $(ODIR)/force.o \
          $(ODIR)/act.info.o \
          $(ODIR)/act.move.o \
          $(ODIR)/act.obj1.o \
          $(ODIR)/act.obj2.o \
          $(ODIR)/handler.o \
          $(ODIR)/pulse.o \
          $(ODIR)/combat.o \
          $(ODIR)/save.o \
          $(ODIR)/mobthink.o \
          $(ODIR)/phys.o \
          $(ODIR)/end.o

SHOBJS	= comm.o \
          save.o \
          db.o \
          handler.o \
          act.adm.o \
          act.comm.o \
          act.info.o \
          act.move.o \
          act.org.o \
          act.nego.o \
          act.obj1.o \
          act.obj2.o \
          act.vio.o \
          act.oth.o \
          act.soc.o \
          interp.o \
          nanny.o \
          utility.o \
          specproc.o \
          pulse.o \
          mobthink.o \
          combat.o \
          modify.o \
          weather.o \
          spell.o \
          const.o \
          spec.gen.o \
          spec.oth.o \
          signals.o \
          board.o \
          org.o \
          cron.o \
          cronutil.o \
          debug.o \
          item.o \
          plot.o \
          event.o \
          player.o \
          force.o \
          phys.o \
          end.o

SRCS  	= comm.c \
          save.c \
          db.c \
          handler.c \
          act.adm.c \
          act.comm.c \
          act.info.c \
          act.move.c \
          act.org.c \
          act.nego.c \
          act.obj1.c \
          act.obj2.c \
          act.vio.c \
          act.oth.c \
          act.soc.c \
          interp.c \
          nanny.c \
          utility.c \
          specproc.c \
          pulse.c \
          mobthink.c \
          combat.c \
          modify.c \
          weather.c \
          spell.c \
          const.c \
          spec.gen.c \
          spec.oth.c \
          signals.c \
          board.c \
          org.c \
          cron.c \
          cronutil.c \
          debug.c \
          item.c \
          plot.c \
          event.c \
          player.c \
          force.c \
          phys.c \
          end.c

#$(ODIR)/idqd
all : $(ODIR) $(ODIR)/dms

$(ODIR) :
	mkdir $(ODIR)

$(ODIR)/idqd : idqd.c
	$(CC) -o $(ODIR)/idqd $(CFLAGS) idqd.c

$(ODIR)/upvers : upvers.c
	$(CC) -o $(ODIR)/upvers $(CFLAGS) upvers.c

$(ODIR)/dms : $(OBJS) $(ODIR)/version.o
	$(CC) -o $(ODIR)/dms $(CFLAGS) $(OBJS) $(ODIR)/version.o $(LIBS)

$(ODIR)/version.o : version.c $(SRCS) $(ODIR)/upvers
	$(ODIR)/upvers
	$(CC) -c -o $(ODIR)/version.o $(CFLAGS) version.c

$(ODIR)/debug.o : debug.c structs.h utils.h comm.h interp.h db.h \
  weather.h time.h player.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/debug.o debug.c

$(ODIR)/item.o : item.c structs.h utils.h db.h weather.h error.h comm.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/item.o item.c

$(ODIR)/event.o : event.c structs.h db.h weather.h utils.h event.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/event.o event.c

$(ODIR)/plot.o : plot.c structs.h utils.h db.h weather.h org.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/plot.o plot.c

$(ODIR)/rumor.o : rumor.c structs.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/rumor.o rumor.c

$(ODIR)/force.o : force.c structs.h comm.h utils.h magic.h \
  error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/force.o force.c

$(ODIR)/save.o : save.c structs.h save.h utils.h comm.h player.h db.h \
  weather.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/save.o save.c

$(ODIR)/comm.o : comm.c structs.h utils.h comm.h interp.h db.h weather.h \
  time.h player.h $(ODIR)/config.h vt100.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/comm.o comm.c

$(ODIR)/cron.o : cron.c cron.h structs.h interp.h utils.h comm.h error.h \
  proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/cron.o cron.c

$(ODIR)/cronutil.o : cronutil.c structs.h db.h weather.h cron.h utils.h \
  interp.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/cronutil.o cronutil.c

$(ODIR)/act.adm.o : act.adm.c structs.h utils.h comm.h time.h event.h \
  player.h interp.h db.h weather.h bio.h error.h org.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/act.adm.o act.adm.c

$(ODIR)/act.comm.o : act.comm.c structs.h utils.h comm.h interp.h db.h \
  weather.h player.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/act.comm.o act.comm.c

$(ODIR)/act.nego.o : act.nego.c structs.h utils.h comm.h event.h interp.h \
  db.h weather.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/act.nego.o act.nego.c

$(ODIR)/act.vio.o : act.vio.c structs.h utils.h comm.h interp.h \
  db.h weather.h skills.h time.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/act.vio.o act.vio.c

$(ODIR)/act.info.o : act.info.c structs.h utils.h comm.h interp.h db.h \
  weather.h org.h time.h player.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/act.info.o act.info.c

$(ODIR)/act.move.o : act.move.c structs.h utils.h comm.h interp.h db.h \
  weather.h skills.h error.h proto.h  bio.h
	$(CC) -c $(CFLAGS) -o $(ODIR)/act.move.o act.move.c

$(ODIR)/act.obj1.o : act.obj1.c structs.h utils.h comm.h interp.h db.h \
  weather.h skills.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/act.obj1.o act.obj1.c

$(ODIR)/act.obj2.o : act.obj2.c structs.h utils.h comm.h interp.h db.h \
  weather.h skills.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/act.obj2.o act.obj2.c

$(ODIR)/act.org.o : act.org.c structs.h utils.h comm.h interp.h db.h \
  weather.h skills.h org.h event.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/act.org.o act.org.c

$(ODIR)/act.oth.o : act.oth.c structs.h utils.h comm.h interp.h db.h \
  weather.h skills.h player.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/act.oth.o act.oth.c

$(ODIR)/act.soc.o : act.soc.c structs.h utils.h comm.h interp.h \
  db.h weather.h event.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/act.soc.o act.soc.c

$(ODIR)/handler.o : handler.c structs.h utils.h comm.h db.h weather.h skills.h \
  error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/handler.o handler.c

$(ODIR)/db.o : db.c structs.h mob.h utils.h db.h weather.h org.h bio.h comm.h \
  player.h skills.h time.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/db.o db.c

$(ODIR)/org.o : org.c structs.h utils.h comm.h interp.h db.h weather.h \
  skills.h org.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/org.o org.c

$(ODIR)/interp.o : interp.c structs.h comm.h interp.h db.h \
  weather.h org.h utils.h event.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/interp.o interp.c

$(ODIR)/nanny.o : nanny.c structs.h comm.h interp.h player.h db.h \
  weather.h org.h utils.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/nanny.o nanny.c

$(ODIR)/utility.o : utility.c structs.h db.h weather.h utils.h time.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/utility.o utility.c

$(ODIR)/specproc.o : specproc.c structs.h db.h weather.h $(ODIR)/config.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/specproc.o specproc.c

$(ODIR)/spec.gen.o : spec.gen.c structs.h utils.h comm.h interp.h \
  db.h weather.h event.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/spec.gen.o spec.gen.c

$(ODIR)/spec.oth.o : spec.oth.c structs.h utils.h comm.h interp.h \
  db.h weather.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/spec.oth.o spec.oth.c

$(ODIR)/pulse.o : pulse.c structs.h utils.h skills.h comm.h db.h weather.h \
  error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/pulse.o pulse.c

$(ODIR)/combat.o : combat.c structs.h utils.h comm.h interp.h db.h \
  weather.h player.h skills.h event.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/combat.o combat.c

$(ODIR)/weather.o : weather.c structs.h utils.h comm.h interp.h db.h \
  weather.h time.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/weather.o weather.c

$(ODIR)/spell.o : spell.c structs.h utils.h comm.h db.h \
  weather.h interp.h skills.h magic.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/spell.o spell.c

$(ODIR)/phys.o : phys.c structs.h utils.h comm.h db.h weather.h error.h proto.h \
  bio.h
	$(CC) -c $(CFLAGS) -o $(ODIR)/phys.o phys.c

$(ODIR)/end.o : end.c structs.h utils.h comm.h db.h weather.h error.h proto.h
	$(CC) -c $(CFLAGS) -o $(ODIR)/end.o end.c

$(ODIR)/mobthink.o : mobthink.c structs.h mob.h utils.h db.h weather.h \
  comm.h event.h skills.h interp.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/mobthink.o mobthink.c

$(ODIR)/player.o : player.c structs.h utils.h db.h weather.h comm.h event.h \
  skills.h interp.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/player.o player.c

$(ODIR)/modify.o : modify.c structs.h utils.h interp.h db.h weather.h \
  comm.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/modify.o modify.c

$(ODIR)/const.o : const.c structs.h magic.h time.h vt100.h bio.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/const.o const.c

$(ODIR)/board.o : board.c structs.h utils.h comm.h db.h weather.h error.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/board.o board.c

$(ODIR)/signals.o : signals.c structs.h utils.h interp.h comm.h proto.h 
	$(CC) -c $(CFLAGS) -o $(ODIR)/signals.o signals.c

# following generated by gcc -MM (these as yet unused)
#
#desc.o : desc.c structs.h utils.h comm.h interp.h db.h weather.h \
#  skills.h org.h error.h proto.h 
#end.o : end.c structs.h comm.h 
#magic2.o : magic2.c structs.h utils.h comm.h interp.h db.h weather.h \
#  skills.h magic.h error.h proto.h 
#mobutil.o : mobutil.c 

clean :
	rm -f $(ODIR)/*.o

dist :
	rm c3.tar.gz
	tar -cvf c3.tar *.h act.comm.c act.vio.c act.info.c act.move.c act.nego.c act.obj1.c act.obj2.c act.org.c act.oth.c act.soc.c comm.c const.c cron.c cronutil.c db.c debug.c end.c event.c combat.c handler.c idcomm.c idqd.c interp.c item.c force.c magic2.c modify.c nanny.c phys.c player.c pulse.c save.c signals.c specproc.c spec.gen.c spec.oth.c spell.c upvers.c utility.c version.c weather.c Makefile vers.log filelist c3.readme
	@echo -n "Before gzip: "
	@ls -l c3.tar
	gzip c3.tar
	@echo -n "After gzip:  "
	@ls -l c3.tar.gz
	@echo Done.
