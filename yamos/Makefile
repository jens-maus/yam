
OBJS= YAM.o YAM_AB.o YAM_CL.o YAM_CO.o YAM_COg.o YAM_COs.o YAM_DI.o YAM_EA.o \
      YAM_ER.o YAM_FI.o YAM_FO.o YAM_MA.o YAM_MAf.o YAM_MI.o YAM_RE.o YAM_TR.o YAM_US.o \
      YAM_UT.o YAM_WR.o YAM_locale.o YAM_rexx.o YAM_rexx_rxcl.o YAM_rexx_rxif.o YAM_md5.o

EXTRA_OBJS= YAM_extra.o
YAM: $(OBJS) $(EXTRA_OBJS)
	gcc -noixemul -o YAM $(OBJS) $(EXTRA_OBJS) gg:lib/NewReadArgs.o -lxpkmaster -lmui -lopenurl -lamiga

starter: starter.c
	m68k-amigaos-gcc -o starter -O2 -nostlib -noixemul -DNAME="\"YAM\"" starter.c

%.o: %.c
	gcc -noixemul -O2 -mmultiple -mcpu=604e -c -DYAMVER="\"2.2\"" -D__YAM_VERDATE="\"7.4.2001\"" -D__YAM_VERDAYS=8497 $<


YAM.o:     YAM.c     YAM.h YAM_locale.h
YAM_AB.o:  YAM_AB.c  YAM.h YAM_locale.h
YAM_CL.o:  YAM_CL.c  YAM.h YAM_locale.h
YAM_CO.o:  YAM_CO.c  YAM.h YAM_locale.h
YAM_COg.o: YAM_COg.c YAM.h YAM_locale.h
YAM_COs.o: YAM_COs.c YAM.h YAM_locale.h
YAM_DI.o:  YAM_DI.c  YAM.h YAM_locale.h
YAM_EA.o:  YAM_EA.c  YAM.h YAM_locale.h
YAM_ER.o:  YAM_ER.c  YAM.h YAM_locale.h
YAM_FI.o:  YAM_FI.c  YAM.h YAM_locale.h
YAM_FO.o:  YAM_FO.c  YAM.h YAM_locale.h
YAM_MA.o:  YAM_MA.c  YAM.h YAM_locale.h
YAM_MAf.o: YAM_MAf.c YAM.h YAM_locale.h
YAM_MI.o:  YAM_MI.c  YAM.h YAM_locale.h
YAM_RE.o:  YAM_RE.c  YAM.h YAM_locale.h
YAM_TR.o:  YAM_TR.c  YAM.h YAM_locale.h
YAM_US.o:  YAM_US.c  YAM.h YAM_locale.h
YAM_UT.o:  YAM_UT.c  YAM.h YAM_locale.h
YAM_WR.o:  YAM_WR.c  YAM.h YAM_locale.h
YAM_locale.o: YAM_locale.c
YAM_rexx.o     : YAM_rexx.c YAM_rexx.h
YAM_rexx_rxcl.o: YAM_rexx_rxcl.c YAM_rexx.h
YAM_rexx_rxif.o: YAM_rexx_rxif.c YAM_rexx.h YAM.h YAM_locale.h
YAM_extra.o: YAM_extra.c YAM.h YAM_locale.h
YAM_md5.o: YAM_md5.c

YAM_locale.h: locale/YAM.cd
	flexcat locale/YAM.cd YAM_locale.h=C_h.sd YAM_locale.c=C_c.sd

