#/***************************************************************************
#
# YAM - Yet Another Mailer
# Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
# Copyright (C) 2000-2001 by YAM Open Source Team
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# YAM Official Support Site :  http://www.yam.ch
# YAM OpenSource project    :  http://sourceforge.net/projects/yamos/
#
# $Id$
#
#***************************************************************************/

TARGET	= YAM

#

CC	= gcc
RM	= rm -f
CDEFS	= -D__YAM_VERDATE=\"`date +%d.%m.%Y`\" -D__YAM_VERDAYS="`expr \`date +%s\` / 86400 - 2922`" -c
CFLAGS	= -noixemul -O2 -mmultiple -mcpu=604e -c
LDFLAGS	= -noixemul
LDLIBS	= gg:lib/NewReadArgs.o -lmui -lopenurl -lxpkmaster -lamiga

CC68K	  = m68k-amigaos-gcc
CFLAGS68K = -nostdlib -DNAME=\"$(TARGET)\" -O -fomit-frame-pointer

#

OBJS	= YAM.o YAM_AB.o YAM_CL.o YAM_CO.o YAM_COg.o YAM_COs.o YAM_DI.o YAM_EA.o \
	  YAM_ER.o YAM_FI.o YAM_FO.o YAM_global.o YAM_locale.o YAM_MA.o YAM_MAf.o \
	  YAM_md5.o YAM_MI.o YAM_RE.o YAM_rexx.o YAM_rexx_rxcl.o YAM_rexx_rxif.o \
	  YAM_TR.o YAM_US.o YAM_UT.o YAM_WR.o $(EXTRA)

EXTRA	= all_morphos.o

#

all: $(TARGET) starter

#

.c.o:
	$(CC) $(CFLAGS) $<

#

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

starter: starter.c
	$(CC68K) $(CFLAGS68K) -o $@ $<

clean:
	-$(RM) $(TARGET) $(OBJS) YAM_locale.?

#

YAM.o:	YAM.c YAM.h YAM_addressbook.h YAM_addressbookEntry.h YAM_classes.h \
	YAM_config.h YAM_configFile.h YAM_debug.h YAM_find.h YAM_folderconfig.h \
	YAM_global.h YAM_hook.h YAM_locale.h YAM_main.h YAM_mainFolder.h \
	YAM_read.h YAM_rexx.h YAM_rexx_rxif.h YAM_stringsizes.h YAM_transfer.h \
	YAM_userlist.h YAM_utilities.h YAM_write.h

YAM_AB.o: YAM_AB.c YAM.h YAM_addressbook.h YAM_addressbookEntry.h \
	YAM_classes.h YAM_config.h YAM_debug.h YAM_error.h YAM_find.h YAM_global.h \
	YAM_hook.h YAM_locale.h YAM_main.h YAM_mainFolder.h YAM_rexx_rxif.h \
	YAM_stringsizes.h YAM_transfer.h YAM_userlist.h YAM_utilities.h YAM_write.h

YAM_CL.o: YAM_CL.c YAM.h YAM_addressbook.h YAM_addressbookEntry.h \
	YAM_classes.h YAM_config.h YAM_debug.h YAM_error.h YAM_find.h \
	YAM_folderconfig.h YAM_hook.h YAM_locale.h YAM_main.h \
	YAM_mainFolder.h YAM_rexx_rxif.h YAM_stringsizes.h YAM_transfer.h \
	YAM_userlist.h YAM_utilities.h YAM_write.h

YAM_CO.o: YAM_CO.c YAM.h YAM_addressbook.h YAM_addressbookEntry.h \
	YAM_classes.h YAM_config.h YAM_configFile.h YAM_configGUI.h YAM_error.h \
	YAM_find.h YAM_folderconfig.h YAM_global.h YAM_hook.h YAM_locale.h \
	YAM_main.h YAM_mainFolder.h YAM_rexx_rxif.h YAM_stringsizes.h \
	YAM_transfer.h YAM_userlist.h YAM_utilities.h

YAM_COg.o: YAM_COg.c YAM.h YAM_classes.h YAM_config.h YAM_find.h \
	 YAM_global.h YAM_hook.h YAM_locale.h YAM_main.h YAM_mainFolder.h \
	 YAM_mime.h YAM_rexx_rxif.h YAM_stringsizes.h YAM_transfer.h YAM_userlist.h \
	 YAM_utilities.h

YAM_COs.o: YAM_COs.c YAM.h YAM_config.h YAM_error.h YAM_find.h \
	 YAM_folderconfig.h YAM_global.h YAM_locale.h YAM_mainFolder.h \
	 YAM_rexx_rxif.h YAM_stringsizes.h YAM_transfer.h YAM_userlist.h \
	 YAM_utilities.h

YAM_DI.o: YAM_DI.c YAM.h YAM_classes.h YAM_error.h YAM_glossarydisplay.h \
	YAM_hook.h YAM_locale.h YAM_rexx_rxif.h YAM_stringsizes.h YAM_transfer.h \
	YAM_userlist.h YAM_utilities.h YAM_write.h

YAM_EA.o: YAM_EA.c YAM.h YAM_addressbook.h YAM_addressbookEntry.h \
	YAM_classes.h YAM_config.h YAM_debug.h YAM_error.h YAM_find.h YAM_hook.h \
	YAM_locale.h YAM_mainFolder.h YAM_rexx_rxif.h YAM_stringsizes.h \
	YAM_transfer.h YAM_userlist.h YAM_utilities.h

YAM_ER.o: YAM_ER.c YAM.h YAM_config.h YAM_error.h YAM_find.h YAM_hook.h \
	YAM_locale.h YAM_main.h YAM_mainFolder.h YAM_rexx_rxif.h YAM_stringsizes.h \
	YAM_transfer.h YAM_userlist.h YAM_utilities.h

YAM_FI.o: YAM_FI.c YAM.h YAM_config.h YAM_configFile.h YAM_error.h \
	YAM_find.h YAM_folderconfig.h YAM_global.h YAM_hook.h YAM_locale.h \
	YAM_main.h YAM_mainFolder.h YAM_read.h YAM_rexx_rxif.h YAM_stringsizes.h \
	YAM_transfer.h YAM_userlist.h YAM_utilities.h YAM_write.h

YAM_FO.o: YAM_FO.c YAM.h YAM_classes.h YAM_debug.h YAM_error.h \
	YAM_folderconfig.h YAM_global.h YAM_hook.h YAM_locale.h YAM_main.h \
	YAM_mainFolder.h YAM_rexx_rxif.h YAM_stringsizes.h YAM_transfer.h \
	YAM_userlist.h YAM_utilities.h

YAM_global.o: YAM_global.c YAM_global.h YAM_locale.h YAM_stringsizes.h
	$(CC) $(CDEFS) $(CFLAGS) $<

YAM_MA.o: YAM_MA.c YAM.h YAM_addressbook.h YAM_addressbookEntry.h \
	YAM_classes.h YAM_config.h YAM_debug.h YAM_error.h YAM_find.h \
	YAM_folderconfig.h YAM_global.h YAM_hook.h YAM_locale.h YAM_main.h \
	YAM_mainFolder.h YAM_read.h YAM_rexx.h YAM_rexx_rxif.h YAM_stringsizes.h \
	YAM_transfer.h YAM_userlist.h YAM_utilities.h YAM_write.h

YAM_MAf.o: YAM_MAf.c YAM.h YAM_config.h YAM_debug.h YAM_find.h \
	 YAM_folderconfig.h YAM_global.h YAM_hook.h YAM_locale.h YAM_main.h \
	 YAM_mainFolder.h YAM_read.h YAM_rexx.h YAM_rexx_rxif.h YAM_stringsizes.h \
	 YAM_transfer.h YAM_userlist.h YAM_utilities.h YAM_write.h

YAM_md5.o: YAM_md5.c YAM_md5.h

YAM_MI.o: YAM_MI.c YAM_error.h YAM_locale.h YAM_mime.h YAM_stringsizes.h \
	YAM_utilities.h

YAM_RE.o: YAM_RE.c YAM.h YAM_addressbook.h YAM_addressbookEntry.h \
	YAM_classes.h YAM_config.h YAM_debug.h YAM_error.h YAM_find.h \
	YAM_folderconfig.h YAM_hook.h YAM_global.h YAM_locale.h YAM_main.h \
	YAM_mainFolder.h YAM_mime.h YAM_read.h YAM_rexx_rxif.h YAM_stringsizes.h \
	YAM_transfer.h YAM_userlist.h YAM_utilities.h YAM_write.h

YAM_rexx.o: YAM_rexx.c YAM_rexx.h YAM_rexx_rxcl.h YAM_rexx_rxif.h

YAM_rexx_rxcl.o: YAM_rexx_rxcl.c YAM_rexx_rxcl.h YAM_rexx_rxif.h

YAM_rexx_rxif.o: YAM_rexx_rxif.c YAM.h YAM_addressbook.h \
	YAM_addressbookEntry.h YAM_config.h YAM_find.h YAM_folderconfig.h \
	YAM_global.h YAM_locale.h YAM_main.h YAM_mainFolder.h YAM_read.h \
	YAM_rexx_rxcl.h YAM_rexx_rxif.h YAM_stringsizes.h YAM_transfer.h \
	YAM_userlist.h YAM_utilities.h YAM_write.h

YAM_TR.o: YAM_TR.c YAM.h YAM_addressbookEntry.h YAM_config.h YAM_error.h \
	YAM_find.h YAM_folderconfig.h YAM_global.h YAM_hook.h YAM_locale.h \
	YAM_main.h YAM_mainFolder.h YAM_md5.h YAM_mime.h YAM_rexx_rxif.h \
	YAM_stringsizes.h YAM_transfer.h YAM_userlist.h YAM_utilities.h

YAM_US.o: YAM_US.c YAM.h YAM_config.h YAM_error.h YAM_find.h YAM_hook.h \
	YAM_locale.h YAM_mainFolder.h YAM_rexx_rxif.h YAM_stringsizes.h \
	YAM_transfer.h YAM_userlist.h YAM_utilities.h

YAM_UT.o: YAM_UT.c YAM.h YAM_classes.h YAM_config.h YAM_debug.h YAM_error.h \
	YAM_find.h YAM_folderconfig.h YAM_global.h YAM_hook.h YAM_locale.h \
	YAM_main.h YAM_mainFolder.h YAM_mime.h YAM_read.h YAM_rexx_rxif.h \
	YAM_stringsizes.h YAM_transfer.h YAM_userlist.h YAM_utilities.h \
	YAM_write.h

YAM_WR.o: YAM_WR.c YAM.h YAM_addressbook.h YAM_addressbookEntry.h \
	YAM_classes.h YAM_config.h YAM_debug.h YAM_error.h YAM_find.h \
	YAM_folderconfig.h YAM_global.h YAM_glossarydisplay.h YAM_hook.h \
	YAM_locale.h YAM_main.h YAM_mainFolder.h YAM_mime.h YAM_read.h \
	YAM_rexx_rxif.h YAM_stringsizes.h YAM_transfer.h YAM_userlist.h \
	YAM_utilities.h YAM_write.h

all_gcc.o: all_gcc.c extrasrc/astcsma.c extrasrc/getft.c extrasrc/stccpy.c \
	extrasrc/stcgfe.c extrasrc/stpblk.c extrasrc/strmfp.c extrasrc/strsfn.c \
	extrasrc/NewReadArgs.c extrasrc/dice.c

YAM_locale.h: YAM_locale.c
YAM_locale.c: locale/YAM.cd C_h.sd C_c.sd
	flexcat $< YAM_locale.h=C_h.sd YAM_locale.c=C_c.sd
