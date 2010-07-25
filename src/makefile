CC      = gcc
CPP	= g++
PROF    = -pg
NOCRYPT =
C_FLAGS = -c -g3 -Wall -O $(PROF) $(NOCRYPT)
CS_FLAGS= $(C_FLAGS) -DSQL_SYSTEM
L_FLAGS = -lcrypt -lm -pg -static
LS_FLAGS= $(L_FLAGS) -L/usr/local/mysql/lib/ -lmysqlclient -lz
SYSTEM  =
DEBUG   = -g
O_FILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o comm.o const.o \
          db.o fight.o handler.o interp.o magic.o save.o special.o update.o \
          bit.o memory.o olc.o olc_act.o olc_save.o string.o mob_commands.o \
          mob_prog.o act_clan.o gr_magic.o id.o scan.o quest.o act_info2.o\
	  ore_prog.o hunt.o language.o drunk.o economy.o act_obj2.o act_wiz2.o\
	  social-edit.o games.o healer.o act_wiz3.o skill_table.o fight2.o\
	  magic2.o magic3.o ssm.o weapons.o act_race.o magic4.o cls_ant.o\
	  act_multi.o marriage.o religion.o cls_shm.o cls_asn.o cls_bar.o\
	  cls_vam.o rel_quest.o cls_nec.o cls_thf.o cls_rng.o cls_none.o\
	  cls_pal.o cls_mag.o cls_psi.o skills.o crafting.o track.o
S_FILES = sqldb.o
OS_FILES= $(O_FILES) $(S_FILES)
ifdef SQL
CFLAGS = $(CS_FLAGS)
LFLAGS = $(LS_FLAGS)
OFILES = $(OS_FILES)
else
CFLAGS = $(C_FLAGS)
LFLAGS = $(L_FLAGS)
OFILES = $(O_FILES)
endif


stormgate: $(OFILES)
	rm -f stormgate
	$(CPP) -o stormgate $(OFILES) $(LFLAGS)


.c.h: merc.h
	$(CC)  $(FLAGS) $(SYSTEM) $<

clean:
	rm -f *.o stormgate *~
