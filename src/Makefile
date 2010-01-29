#/***************************************************************************
#
# YAM - Yet Another Mailer
# Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
# Copyright (C) 2000-2010 by YAM Open Source Team
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

###########################################################################
# This makefile is a very generic one. It tries to identify both, the host
# and the target operating system for which YAM should be compiled.
# However, this auto-detection can be easily overridden by directly
# specifying an option on the commandline while calling 'make'.
#
# Example:
#
# # to explicitly compile for AmigaOS3
# > make OS=os3
#
# # to compile for AmigaOS4 but without debugging
# > make OS=os4 DEBUG=
#

#############################################
# find out the HOST operating system
# on which this makefile is run
HOST ?= $(shell uname)
ifeq ($(HOST), AmigaOS)
  ifeq ($(shell uname -m), powerpc)
    HOST = AmigaOS4
  endif
  ifeq ($(shell uname -m), ppc)
    HOST = AmigaOS4
  endif
endif

# if no host is identifed (no uname tool)
# we assume a AmigaOS build
ifeq ($(HOST),)
  HOST = AmigaOS
endif

#############################################
# now we find out the target OS for
# which we are going to compile YAM in case
# the caller didn't yet define OS himself
ifndef (OS)
  ifeq ($(HOST), AmigaOS4)
    OS = os4
  else
  ifeq ($(HOST), AmigaOS)
    OS = os3
  else
  ifeq ($(HOST), MorphOS)
    OS = mos
  else
  ifeq ($(HOST), AROS)
    # now we find out which CPU system aros will be used
    ifeq ($(shell uname -m), powerpc)
      OS = aros-ppc
    endif
    ifeq ($(shell uname -m), ppc)
      OS = aros-ppc
    endif
    ifeq ($(shell uname -m), i386)
      OS = aros-i386
    endif
    ifeq ($(shell uname -m), i686)
      OS = aros-i686
    endif
    ifeq ($(shell uname -m), x86_64)
      OS = aros-x86_64
    endif
  else
    OS = os4
  endif
  endif
  endif
  endif
endif

#############################################
# define common commands we use in this
# makefile. Please note that each of them
# might be overridden on the commandline.

# common commands
FLEX    = flex
FC      = flexcat
EXPR    = expr
DATE    = date
RM      = rm -f #delete force
RMDIR   = rm -rf #delete force all
MKDIR   = mkdir -p #makedir force
CHMOD   = protect FLAGS=rwed
CP      = copy
CC      = gcc
STRIP   = strip
OBJDUMP = objdump

# path definitions
CDUP  = /
CDTHIS=

# Special macros for version ID and developer builds via Arexx
# and C:Date calls
DEVFLAGS = -DDEVWARNING `rx "say '-DEXPDATE=' || date(I)+31"`
YAMVERID = -D__YAM_BUILDID=$(BUILDID) \
           `rx "say '-D__YAM_VERDATE=' || '22'x || right(date(S),2)'.'substr(date(S),5,2)'.'left(date(S),4) || '22'x"` \
           `rx "say '-D__YAM_BUILDDATE=' || '22'x || date(S) || '22'x"` \
           `rx "say '-D__YAM_VERDAYS=' || date(I)"` \

# override some variables for non-native builds (cross-compiler)
ifneq ($(HOST), AmigaOS)
ifneq ($(HOST), AmigaOS4)

  # this is either a cross-compiler build on a UNIX-like system or
  # on MorphOS, so lets use expr/gdate instead of the above ARexx stuff
  DEVFLAGS = -DDEVWARNING -DEXPDATE="`$(EXPR) \`$(DATE) +%s\` / 86400 - 2922 + 31`"
  YAMVERID = -D__YAM_VERDATE=\"`$(DATE) +%d.%m.%Y`\" \
             -D__YAM_VERDAYS="`$(EXPR) \`$(DATE) +%s\` / 86400 - 2922`" \
             -D__YAM_BUILDDATE=\"`$(DATE) +%Y%m%d`\" \
             -D__YAM_BUILDID="$(BUILDID)"

  ifneq ($(HOST), MorphOS)

    # when we end up here this is either a unix or Aros host
    # so lets use unix kind of commands
    RM      = rm -f
    RMDIR   = rm -rf
    MKDIR   = mkdir -p
    CHMOD   = chmod 755
    CP      = cp -f

    CDUP  = ../
    CDTHIS= ./

  endif
endif
endif

###########################################################################
# CPU and DEBUG can be defined outside, defaults to above
# using e.g. "make DEBUG= CPU=-mcpu=603e" produces optimized non-debug
# PPC-603e version
#
# OPTFLAGS are disabled by DEBUG normally!
#
# ignored warnings are:
# none - because we want to compile with -Wall all the time

# target definition
TARGET  = YAM.$(OS)
BUILDID = 0

# Common Directories
PREFIX    = $(CDTHIS)
MUIDIR    = $(PREFIX)mui
REXXDIR   = $(PREFIX)rexx
MIMEDIR   = $(PREFIX)mime
LOCALE    = $(PREFIX)$(CDUP)locale
EXTRASRC  = $(PREFIX)extrasrc
TOOLS     = $(PREFIX)tools
LIB       = $(PREFIX)lib/$(OS)
OBJDIR    = .obj_$(OS)
VPATH     = $(OBJDIR)

# Common compiler/linker flags
WARN     = -W -Wall -Wwrite-strings -Wpointer-arith -Wsign-compare #-Wunreachable-code
OPTFLAGS = -O3 -fomit-frame-pointer -funroll-loops
DEBUG    = -DDEBUG -fno-omit-frame-pointer #-O0
DEBUGSYM = -g -gstabs
CFLAGS   = -I. -I./include $(CPU) $(WARN) $(OPTFLAGS) $(DEBUG) $(DEBUGSYM)
LDFLAGS  = $(CPU) $(DEBUGSYM)
LDLIBS   = -L$(LIB) -lm

# different options per target OS
ifeq ($(OS), os4)

  ##############################
  # AmigaOS4

  # Compiler/link/strip commands
  ifneq ($(HOST), AmigaOS4)
    CC      = ppc-amigaos-gcc
    STRIP   = ppc-amigaos-strip
    OBJDUMP = ppc-amigaos-objdump
  endif

  # Compiler/Linker flags
  CRT      = newlib
  CPU      = -mcpu=powerpc -mstrict-align
  WARN     += -Wdeclaration-after-statement -Wdisabled-optimization
  REDEFINE = -DCoerceMethod=ICoerceMethod -DDoMethod=IDoMethod \
             -DDoSuperMethod=IDoSuperMethod -DDoSuperMethodA=IDoSuperMethodA
  CFLAGS   += -mcrt=$(CRT) -D__USE_INLINE__ -D__NEW_TIMEVAL_DEFINITION_USED__ \
              $(REDEFINE) -Wa,-mregnames
  LDFLAGS  += -mcrt=$(CRT)
  CDUP     = ../
  CDTHIS   = ./

else
ifeq ($(OS), os3)

  ##############################
  # AmigaOS3

  # Compiler/link/strip commands
  ifneq ($(HOST), AmigaOS)
    CC      = m68k-amigaos-gcc
    STRIP   = m68k-amigaos-strip
    OBJDUMP = m68k-amigaos-objdump
  endif

  # Compiler/Linker flags
  CPU	    = -m68020-60 -msoft-float
  CFLAGS  += -I./include/netinclude -DNO_INLINE_STDARG
  LDFLAGS += -noixemul -mstackextend
  LDLIBS  += -ldebug -lmui

else
ifeq ($(OS), mos)

  ##############################
  # MorphOS

  # Compiler/link/strip commands
  ifneq ($(HOST), MorphOS)
    CC      = ppc-morphos-gcc
    STRIP   = ppc-morphos-strip
    OBJDUMP = ppc-morphos-objdump
  endif

  # Compiler/Linker flags
  CPU     = -mcpu=powerpc
  CFLAGS  += -noixemul -I./include/netinclude
  LDFLAGS += -noixemul
  LDLIBS  += -lxpk -lamissl -lcodesets -lopenurl

else
ifeq ($(OS), aros-i386)

  ##############################
  # AROS (i386)

  ifneq ($(HOST), AROS)
    CC      = i386-aros-gcc
    STRIP   = i386-aros-strip
    OBJDUMP = i386-aros-objdump
  endif

  # Compiler/Linker flags
  CFLAGS += -Wno-pointer-sign -DNO_INLINE_STDARG
  LDLIBS += -lamiga -larossupport -larosc -lmui

else
ifeq ($(OS), aros-ppc)

  ##############################
  # AROS (PPC)

  ifneq ($(HOST), AROS)
    CC      = ppc-aros-gcc
    STRIP   = ppc-aros-strip
    OBJDUMP = ppc-aros-objdump
  endif

  # Compiler/Linker flags
  CFLAGS += -Wno-pointer-sign -DNO_INLINE_STDARG
  LDLIBS += -lamiga -larossupport -larosc -lmui

else
ifeq ($(OS), aros-x86_64)

  ##############################
  # AROS (x86_64)

  ifneq ($(HOST), AROS)
    CC      = x86_64-aros-gcc
    STRIP   = x86_64-aros-strip
    OBJDUMP = x86_64-aros-objdump
  endif

  # Compiler/Linker flags
  CFLAGS += -Wno-pointer-sign -DNO_INLINE_STDARG
  LDLIBS += -lamiga -larossupport -larosc -lmui

endif
endif
endif
endif
endif
endif

###########################################################################
# Here starts all stuff that is common for all target platforms and
# hosts.

# General vars
GENCLASSES= $(TOOLS)/genclasses

# our few new rewritten classes
MUI = \
	Classes.o \
	Aboutwindow.o \
	AddrBookEntryList.o \
	AddrBookListtree.o \
	AddrBookToolbar.o \
	Addrmatchlist.o \
	AttachmentGroup.o \
	AttachmentImage.o \
	AttachmentObject.o \
	AttachmentRequestWindow.o \
	CheckboxRequestWindow.o \
	ConfigPageList.o \
	FolderRequestWindow.o \
	GenericRequestWindow.o \
	ImageArea.o \
	InfoBar.o \
	InfoWindow.o \
	MailTextEdit.o \
	MainFolderListtree.o \
	MainMailList.o \
	MainMailListGroup.o \
	MainWindow.o \
	MainWindowToolbar.o \
	PassphraseRequestWindow.o \
	QuickSearchBar.o \
	ReadMailGroup.o \
	ReadWindow.o \
	ReadWindowToolbar.o \
	ReadWindowStatusBar.o \
	Recipientstring.o \
	Searchwindow.o \
	SearchControlGroup.o \
	ShutdownWindow.o \
	Splashwindow.o \
	StringRequestWindow.o \
	ThemeListGroup.o \
	TransferMailList.o \
	UpdateNotifyWindow.o \
	WriteAttachmentList.o \
	WriteWindow.o \
	WriteWindowToolbar.o \
	YAM.o

REXX = \
	addrdelete.o \
	addredit.o \
	addrfind.o \
	addrgoto.o \
	addrinfo.o \
	addrload.o \
	addrnew.o \
	addrresolve.o \
	addrsave.o \
	appbusy.o \
	appnobusy.o \
	flushindexes.o \
	folderinfo.o \
	getconfiginfo.o \
	getfolderinfo.o \
	getmailinfo.o \
	getselected.o \
	geturl.o \
	help.o \
	hide.o \
	info.o \
	isonline.o \
	listfreeze.o \
	listselect.o \
	listunfreeze.o \
	mailarchive.o \
	mailbounce.o \
	mailchangesubject.o \
	mailcheck.o \
	mailcopy.o \
	maildelete.o \
	mailedit.o \
	mailexport.o \
	mailfilter.o \
	mailforward.o \
	mailimport.o \
	mailinfo.o \
	mailmove.o \
	mailread.o \
	mailreply.o \
	mailsendall.o \
	mailsend.o \
	mailstatus.o \
	mailupdate.o \
	mailwrite.o \
	newmailfile.o \
	quit.o \
	readclose.o \
	readinfo.o \
	readprint.o \
	readsave.o \
	request.o \
	requestfile.o \
	requestfolder.o \
	requeststring.o \
	screentoback.o \
	screentofront.o \
	setflag.o \
	setfolder.o \
	setmail.o \
	setmailfile.o \
	show.o \
	userinfo.o \
	writeattach.o \
	writebcc.o \
	writecc.o \
	writeeditor.o \
	writefrom.o \
	writeletter.o \
	writemailto.o \
	writeoptions.o \
	writequeue.o \
	writereplyto.o \
	writesend.o \
	writesubject.o \
	writeto.o

MIME = \
	base64.o \
	qprintable.o \
	uucode.o \
	rfc1738.o \
	rfc2047.o \
	rfc2231.o \
	md5.o

# and the other object files where
# YAM_global.o and YAM.o have to stay at the top
OBJS = \
	YAM_global.o \
	YAM.o \
	$(addprefix $(MUIDIR)/,$(MUI)) \
	$(addprefix $(REXXDIR)/,$(REXX)) \
	$(addprefix $(MIMEDIR)/,$(MIME)) \
	YAM_AB.o \
	YAM_CO.o \
	YAM_COg.o \
	YAM_COs.o \
	YAM_DI.o \
	YAM_EA.o \
	YAM_ER.o \
	YAM_FI.o \
	YAM_FO.o \
	YAM_MA.o \
	YAM_MAf.o \
	YAM_RE.o \
	YAM_TR.o \
	YAM_US.o \
	YAM_UT.o \
	YAM_WR.o \
	AppIcon.o \
	BayesFilter.o \
	DockyIcon.o \
	FileInfo.o \
	FolderList.o \
	HashTable.o \
	HTML2Mail.o \
	ImageCache.o \
	Locale.o \
	MailList.o \
	MimeTypes.o \
	MUIObjects.o \
	ParseEmail.o \
	Requesters.o \
	Rexx.o \
	Themes.o \
	Threads.o \
	Timer.o \
	UpdateCheck.o \
	extrasrc.o \
	Debug.o

# available catalog translations
CATALOGS = \
	$(LOCALE)/czech.catalog \
	$(LOCALE)/english-british.catalog \
	$(LOCALE)/french.catalog \
	$(LOCALE)/german.catalog \
	$(LOCALE)/greek.catalog \
	$(LOCALE)/italian.catalog \
	$(LOCALE)/swedish.catalog \
	# $(LOCALE)/bosnian.catalog \
	# $(LOCALE)/danish.catalog \
	# $(LOCALE)/finnish.catalog \
	# $(LOCALE)/hungarian.catalog \
	# $(LOCALE)/norwegian.catalog \
	# $(LOCALE)/polish.catalog \
	# $(LOCALE)/portuguese-brazil.catalog \
	# $(LOCALE)/portuguese.catalog \
	# $(LOCALE)/russian.catalog \
	# $(LOCALE)/serbian.catalog \
	# $(LOCALE)/slovenian.catalog \
	# $(LOCALE)/spanish.catalog \
	# $(LOCALE)/turkish.catalog

# main target
.PHONY: all
all: $(OBJDIR) $(OBJDIR)/$(MUIDIR) $(OBJDIR)/$(REXXDIR) \
 $(OBJDIR)/$(MIMEDIR) classinfo Locale.c ParseEmail.c HTML2Mail.c $(TARGET)

# make the object directories
$(OBJDIR):
	@echo "  MK $@"
	@$(MKDIR) $(OBJDIR)

$(OBJDIR)/$(MUIDIR): $(OBJDIR)
	@$(MKDIR) $(OBJDIR)/$(MUIDIR)

$(OBJDIR)/$(REXXDIR): $(OBJDIR)
	@$(MKDIR) $(OBJDIR)/$(REXXDIR)

$(OBJDIR)/$(MIMEDIR): $(OBJDIR)
	@$(MKDIR) $(OBJDIR)/$(MIMEDIR)

# for compiling single .c files
$(OBJDIR)/%.o: %.c
	@echo "  CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# for making .c file out of .l lexer files
%.c: %.l
	@echo "  LX $<"
	@$(FLEX) -Cfe -i -o$@ $<

# for linking the target
$(TARGET): $(addprefix $(OBJDIR)/,$(OBJS))
	@echo "  LD $@.debug"
	@$(CC) $(LDFLAGS) -o $@.debug $(addprefix $(OBJDIR)/,$(OBJS)) $(LDLIBS) -Wl,--cref,-M,-Map=$@.map
	@echo "  LD $@"
	@$(STRIP) --preserve-dates -R.comment -R.sdata2 -S -o $@ $@.debug
	@$(CHMOD) $@

# for creating a .dump file
.PHONY: dump
dump:
	-$(OBJDUMP) --section-headers --all-headers --reloc --disassemble-all $(TARGET).debug > $(TARGET).dump

# cleanup target
.PHONY: clean
clean:
	-$(RM) $(TARGET) $(TARGET).debug $(TARGET).map $(addprefix $(OBJDIR)/,$(OBJS))

# clean all including .obj directory
.PHONY: cleanall
cleanall: clean
	-$(RM) Locale.? ParseEmail.c HTML2Mail.c $(MUIDIR)/Classes.? $(MUIDIR)/*_cl.h
	-$(RM) $(LOCALE)/*.catalog
	-$(RMDIR) $(OBJDIR)

# clean all stuff, inclduing our autotools
.PHONY: distclean
distclean: cleanall
	@$(MAKE) -C$(GENCLASSES) clean

## SPECIAL COMPILE RULES ##############

$(OBJDIR)/YAM_global.o: YAM_global.c
	@echo "  CC $<"
	@$(CC) $(YAMVERID) $(CFLAGS) -c $< -o $@

$(OBJDIR)/YAM.o: YAM.c
	@echo "  CC $<"
	@$(CC) $(DEVFLAGS) $(CFLAGS) -c $< -o $@

$(OBJDIR)/ParseEmail.o: ParseEmail.c
	@echo "  CC $<"
	@$(CC) $(CFLAGS) -Wno-sign-compare -c $< -o $@

$(OBJDIR)/HTML2Mail.o: HTML2Mail.c
	@echo "  CC $<"
	@$(CC) $(CFLAGS) -Wno-sign-compare -c $< -o $@

## DEPENDENCY GENERATION ##############

Makefile.dep: ;
	@echo "WARNING: Makefile.dep missing. Please run 'make depend'"

.PHONY: depend
depend: classinfo Locale.c ParseEmail.c HTML2Mail.c
	@echo "  MK Makefile.dep"
	@echo "# AUTOGENERATED! DO NOT EDIT!!!" >Makefile.dep
	@$(CC) -MM $(CFLAGS) $(patsubst %.o,%.c, $(OBJS)) >>Makefile.dep
	@echo "# AUTOGENERATED! DO NOT EDIT!!!" >>Makefile.dep

## GENCLASSES TOOL ####################

.PHONY: classinfo
classinfo: $(MUIDIR)/Classes.h

$(MUIDIR)/Classes.h: $(GENCLASSES)/GenClasses $(MUIDIR)/*.c
	@echo "  GN GenClasses"
	@$(GENCLASSES)/GenClasses mui -bYAM -gpl -storm -iClassesExtra.h -q

$(MUIDIR)/Classes.c: $(MUIDIR)/ClassesExtra.h

$(GENCLASSES)/GenClasses: $(GENCLASSES)/gc.c $(GENCLASSES)/gc.h \
	$(GENCLASSES)/lists.c $(GENCLASSES)/lists.h
	@$(MAKE) -C$(GENCLASSES)

## CATALOG GENERATION #################

$(LOCALE)/%.catalog: $(LOCALE)/%.ct
	@echo "  MK $@"
	@$(FC) $(LOCALE)/YAM.cd $< CATALOG $@

.IGNORE: $(CATALOGS)

catalogs: $(CATALOGS)

## FLEXCAT ############################

Locale.h: Locale.c
Locale.c: ../locale/YAM.cd Locale_h.sd Locale_c.sd
	@echo "  FC $@"
	@$(FC) $(LOCALE)/YAM.cd Locale.h=Locale_h.sd Locale.c=Locale_c.sd

## TO CREATE RELEASE ARCHIVES #########

.PHONY: release
release:
	@$(RMDIR) $@
	@$(MKDIR) $@
## Create catalogs first ##############
	@echo "  MK catalogs"
	@$(RM) $(CATALOGS)
	@$(MAKE) catalogs
## AmigaOS3 distribution ##############
	@echo "  MK AmigaOS3"
	@$(MAKE) OS=os3 DEBUG= DEVFLAGS= clean
	@$(MAKE) OS=os3 DEBUG= DEVFLAGS= all
	@$(MKDIR) "$@/AmigaOS3"
	@$(MKDIR) "$@/AmigaOS3/YAM 2.6"
	@$(CP) $(CDUP)icons/os3/YAM_directory.info "$@/AmigaOS3/YAM 2.6.info"
	@echo "  MK AmigaOS3/Catalogs"
	@$(MKDIR) "$@/AmigaOS3/YAM 2.6/Catalogs"
	@$(CP) $(CDUP)icons/os3/Catalogs_directory.info "$@/AmigaOS3/YAM 2.6/Catalogs.info"
	@$(CP) $(LOCALE)/YAM.cd "$@/AmigaOS3/YAM 2.6/Catalogs/YAM.cd"
	@$(MKDIR) "$@/AmigaOS3/YAM 2.6/Catalogs/czech"
	@$(CP) $(LOCALE)/czech.catalog "$@/AmigaOS3/YAM 2.6/Catalogs/czech/YAM.catalog"
	@$(MKDIR) "$@/AmigaOS3/YAM 2.6/Catalogs/french"
	@$(CP) $(LOCALE)/french.catalog "$@/AmigaOS3/YAM 2.6/Catalogs/french/YAM.catalog"
	@$(MKDIR) "$@/AmigaOS3/YAM 2.6/Catalogs/german"
	@$(CP) $(LOCALE)/german.catalog "$@/AmigaOS3/YAM 2.6/Catalogs/german/YAM.catalog"
	@$(MKDIR) "$@/AmigaOS3/YAM 2.6/Catalogs/greek"
	@$(CP) $(LOCALE)/greek.catalog "$@/AmigaOS3/YAM 2.6/Catalogs/greek/YAM.catalog"
	@$(MKDIR) "$@/AmigaOS3/YAM 2.6/Catalogs/italian"
	@$(CP) $(LOCALE)/italian.catalog "$@/AmigaOS3/YAM 2.6/Catalogs/italian/YAM.catalog"
	@$(MKDIR) "$@/AmigaOS3/YAM 2.6/Catalogs/swedish"
	@$(CP) $(LOCALE)/swedish.catalog "$@/AmigaOS3/YAM 2.6/Catalogs/swedish/YAM.catalog"
	@echo "  MK AmigaOS3/Docs"
	@$(MKDIR) "$@/AmigaOS3/YAM 2.6/Docs"
	@$(CP) $(CDUP)icons/os3/Docs_directory.info "$@/AmigaOS3/YAM 2.6/Docs.info"
	@$(CP) $(CDUP)doc/YAM_english.guide "$@/AmigaOS3/YAM 2.6/Docs/YAM_english.guide"
	@$(CP) $(CDUP)icons/os3/guide.info "$@/AmigaOS3/YAM 2.6/Docs/YAM_english.guide.info"
	@$(CP) $(CDUP)doc/YAM_french.guide "$@/AmigaOS3/YAM 2.6/Docs/YAM_french.guide"
	@$(CP) $(CDUP)icons/os3/guide.info "$@/AmigaOS3/YAM 2.6/Docs/YAM_french.guide.info"
	@$(CP) $(CDUP)AUTHORS "$@/AmigaOS3/YAM 2.6/Docs/AUTHORS"
	@$(CP) $(CDUP)icons/os3/Docs_AUTHORS.info "$@/AmigaOS3/YAM 2.6/Docs/AUTHORS.info"
	@$(CP) $(CDUP)ChangeLog "$@/AmigaOS3/YAM 2.6/Docs/ChangeLog"
	@$(CP) $(CDUP)icons/os3/Docs_ChangeLog.info "$@/AmigaOS3/YAM 2.6/Docs/ChangeLog.info"
	@$(CP) $(CDUP)COPYING "$@/AmigaOS3/YAM 2.6/Docs/COPYING"
	@$(CP) $(CDUP)icons/os3/Docs_COPYING.info "$@/AmigaOS3/YAM 2.6/Docs/COPYING.info"
	@echo "  MK AmigaOS3/Gallery"
	@$(MKDIR) "$@/AmigaOS3/YAM 2.6/Gallery"
	@$(CP) $(CDUP)icons/os3/Gallery_directory.info "$@/AmigaOS3/YAM 2.6/Gallery.info"
	@$(CP) $(CDUP)doc/ReadMe_gallery "$@/AmigaOS3/YAM 2.6/Gallery/ReadMe"
	@$(CP) $(CDUP)icons/os3/ReadMe.info "$@/AmigaOS3/YAM 2.6/Gallery/ReadMe.info"
	@echo "  MK AmigaOS3/Install"
	@svn export -q "https://yamos.svn.sourceforge.net/svnroot/yamos/distribution/AmigaOS3/YAM 2.6/Install" "$@/AmigaOS3/YAM 2.6/Install"
	@$(CP) $(CDUP)icons/os3/Install_directory.info "$@/AmigaOS3/YAM 2.6/Install.info"
	@$(CP) $(CDUP)doc/Install-YAM "$@/AmigaOS3/YAM 2.6/Install/Install-YAM"
	@$(CP) $(CDUP)icons/os3/Install-YAM.info "$@/AmigaOS3/YAM 2.6/Install/Install-YAM.info"
	@echo "  MK AmigaOS3/Rexx"
	@svn export -q $(CDUP)rexx "$@/AmigaOS3/YAM 2.6/Rexx"
	@$(CP) $(CDUP)icons/os3/Rexx_directory.info "$@/AmigaOS3/YAM 2.6/Rexx.info"
	@echo "  MK AmigaOS3/Themes"
	@svn export -q $(CDUP)themes "$@/AmigaOS3/YAM 2.6/Themes"
	@$(CP) $(CDUP)icons/os3/Themes_directory.info "$@/AmigaOS3/YAM 2.6/Themes.info"
	@$(CP) $(CDUP)themes/ReadMe "$@/AmigaOS3/YAM 2.6/Themes/ReadMe"
	@$(CP) $(CDUP)icons/os3/Themes_ReadMe.info "$@/AmigaOS3/YAM 2.6/Themes/ReadMe.info"
	@$(CP) YAM.os3 "$@/AmigaOS3/YAM 2.6/YAM"
	@$(CP) $(CDUP)icons/os3/YAM.info "$@/AmigaOS3/YAM 2.6/YAM.info"
	@$(CP) $(CDUP)doc/ReadMe "$@/AmigaOS3/YAM 2.6/ReadMe"
	@$(CP) $(CDUP)icons/os3/ReadMe.info "$@/AmigaOS3/YAM 2.6/ReadMe.info"
	@echo "  MK YAM26-AmigaOS3.lha"
	@cd $@/AmigaOS3 && lha -aq $(CDUP)YAM26-AmigaOS3.lha *
## AmigaOS3-debug distribution ########
	@echo "  MK AmigaOS3-debug"
	@$(MAKE) OS=os3 DEVFLAGS= clean
	@$(MAKE) OS=os3 DEVFLAGS= all
	@$(CP) $(CDUP)doc/README.debug "$@/"
	@$(CP) YAM.os3.debug "$@/YAM.debug"
	@$(CP) YAM.os3.map "$@/YAM.debug.map"
	@echo "  MK YAM26-AmigaOS3-debug.lha"
	@cd $@ && lha -aq YAM26-AmigaOS3-debug.lha README.debug YAM.debug YAM.debug.map
## AmigaOS4 distribution ##############
	@echo "  MK AmigaOS4"
	@$(MAKE) OS=os4 DEBUG= DEVFLAGS= clean
	@$(MAKE) OS=os4 DEBUG= DEVFLAGS= all
	@$(MKDIR) "$@/AmigaOS4"
	@$(MKDIR) "$@/AmigaOS4/YAM 2.6"
	@$(CP) $(CDUP)icons/os4/YAM_directory.info "$@/AmigaOS4/YAM 2.6.info"
	@echo "  MK AmigaOS4/Catalogs"
	@$(MKDIR) "$@/AmigaOS4/YAM 2.6/Catalogs"
	@$(CP) $(CDUP)icons/os4/Catalogs_directory.info "$@/AmigaOS4/YAM 2.6/Catalogs.info"
	@$(CP) $(LOCALE)/YAM.cd "$@/AmigaOS4/YAM 2.6/Catalogs/YAM.cd"
	@$(MKDIR) "$@/AmigaOS4/YAM 2.6/Catalogs/czech"
	@$(CP) $(LOCALE)/czech.catalog "$@/AmigaOS4/YAM 2.6/Catalogs/czech/YAM.catalog"
	@$(MKDIR) "$@/AmigaOS4/YAM 2.6/Catalogs/french"
	@$(CP) $(LOCALE)/french.catalog "$@/AmigaOS4/YAM 2.6/Catalogs/french/YAM.catalog"
	@$(MKDIR) "$@/AmigaOS4/YAM 2.6/Catalogs/german"
	@$(CP) $(LOCALE)/german.catalog "$@/AmigaOS4/YAM 2.6/Catalogs/german/YAM.catalog"
	@$(MKDIR) "$@/AmigaOS4/YAM 2.6/Catalogs/greek"
	@$(CP) $(LOCALE)/greek.catalog "$@/AmigaOS4/YAM 2.6/Catalogs/greek/YAM.catalog"
	@$(MKDIR) "$@/AmigaOS4/YAM 2.6/Catalogs/italian"
	@$(CP) $(LOCALE)/italian.catalog "$@/AmigaOS4/YAM 2.6/Catalogs/italian/YAM.catalog"
	@$(MKDIR) "$@/AmigaOS4/YAM 2.6/Catalogs/swedish"
	@$(CP) $(LOCALE)/swedish.catalog "$@/AmigaOS4/YAM 2.6/Catalogs/swedish/YAM.catalog"
	@echo "  MK AmigaOS4/Docs"
	@$(MKDIR) "$@/AmigaOS4/YAM 2.6/Docs"
	@$(CP) $(CDUP)icons/os4/Docs_directory.info "$@/AmigaOS4/YAM 2.6/Docs.info"
	@$(CP) $(CDUP)doc/YAM_english.guide "$@/AmigaOS4/YAM 2.6/Docs/YAM_english.guide"
	@$(CP) $(CDUP)icons/os4/guide.info "$@/AmigaOS4/YAM 2.6/Docs/YAM_english.guide.info"
	@$(CP) $(CDUP)doc/YAM_french.guide "$@/AmigaOS4/YAM 2.6/Docs/YAM_french.guide"
	@$(CP) $(CDUP)icons/os4/guide.info "$@/AmigaOS4/YAM 2.6/Docs/YAM_french.guide.info"
	@$(CP) $(CDUP)AUTHORS "$@/AmigaOS4/YAM 2.6/Docs/AUTHORS"
	@$(CP) $(CDUP)icons/os4/Docs_AUTHORS.info "$@/AmigaOS4/YAM 2.6/Docs/AUTHORS.info"
	@$(CP) $(CDUP)ChangeLog "$@/AmigaOS4/YAM 2.6/Docs/ChangeLog"
	@$(CP) $(CDUP)icons/os4/Docs_ChangeLog.info "$@/AmigaOS4/YAM 2.6/Docs/ChangeLog.info"
	@$(CP) $(CDUP)COPYING "$@/AmigaOS4/YAM 2.6/Docs/COPYING"
	@$(CP) $(CDUP)icons/os4/Docs_COPYING.info "$@/AmigaOS4/YAM 2.6/Docs/COPYING.info"
	@echo "  MK AmigaOS4/Gallery"
	@$(MKDIR) "$@/AmigaOS4/YAM 2.6/Gallery"
	@$(CP) $(CDUP)icons/os4/Gallery_directory.info "$@/AmigaOS4/YAM 2.6/Gallery.info"
	@$(CP) $(CDUP)doc/ReadMe_gallery "$@/AmigaOS4/YAM 2.6/Gallery/ReadMe"
	@$(CP) $(CDUP)icons/os4/ReadMe.info "$@/AmigaOS4/YAM 2.6/Gallery/ReadMe.info"
	@echo "  MK AmigaOS4/Install"
	@svn export -q "https://yamos.svn.sourceforge.net/svnroot/yamos/distribution/AmigaOS4/YAM 2.6/Install" "$@/AmigaOS4/YAM 2.6/Install"
	@$(CP) $(CDUP)icons/os4/Install_directory.info "$@/AmigaOS4/YAM 2.6/Install.info"
	@$(CP) $(CDUP)doc/Install-YAM "$@/AmigaOS4/YAM 2.6/Install/Install-YAM"
	@$(CP) $(CDUP)icons/os4/Install-YAM.info "$@/AmigaOS4/YAM 2.6/Install/Install-YAM.info"
	@echo "  MK AmigaOS4/Rexx"
	@svn export -q $(CDUP)rexx "$@/AmigaOS4/YAM 2.6/Rexx"
	@$(CP) $(CDUP)icons/os4/Rexx_directory.info "$@/AmigaOS4/YAM 2.6/Rexx.info"
	@echo "  MK AmigaOS4/Themes"
	@svn export -q $(CDUP)themes "$@/AmigaOS4/YAM 2.6/Themes"
	@$(CP) $(CDUP)icons/os4/Themes_directory.info "$@/AmigaOS4/YAM 2.6/Themes.info"
	@$(CP) $(CDUP)themes/ReadMe "$@/AmigaOS4/YAM 2.6/Themes/ReadMe"
	@$(CP) $(CDUP)icons/os4/Themes_ReadMe.info "$@/AmigaOS4/YAM 2.6/Themes/ReadMe.info"
	@$(CP) YAM.os4 "$@/AmigaOS4/YAM 2.6/YAM"
	@$(CP) $(CDUP)icons/os4/YAM.info "$@/AmigaOS4/YAM 2.6/YAM.info"
	@$(CP) $(CDUP)doc/ReadMe "$@/AmigaOS4/YAM 2.6/ReadMe"
	@$(CP) $(CDUP)icons/os4/ReadMe.info "$@/AmigaOS4/YAM 2.6/ReadMe.info"
	@echo "  MK YAM26-AmigaOS4.lha"
	@cd $@/AmigaOS4 && lha -aq $(CDUP)YAM26-AmigaOS4.lha *
## AmigaOS4-debug distribution ########
	@echo "  MK AmigaOS4-debug"
	@$(MAKE) OS=os4 DEVFLAGS= clean
	@$(MAKE) OS=os4 DEVFLAGS= all
	@$(CP) $(CDUP)doc/README.debug "$@/"
	@$(CP) YAM.os4.debug "$@/YAM.debug"
	@$(CP) YAM.os4.map "$@/YAM.debug.map"
	@echo "  MK YAM26-AmigaOS4-debug.lha"
	@cd $@ && lha -aq YAM26-AmigaOS4-debug.lha README.debug YAM.debug YAM.debug.map
## MorphOS distribution ###############
	@echo "  MK MorphOS"
	@$(MAKE) OS=mos DEBUG= DEVFLAGS= clean
	@$(MAKE) OS=mos DEBUG= DEVFLAGS= all
	@$(MKDIR) "$@/MorphOS"
	@$(MKDIR) "$@/MorphOS/YAM 2.6"
	@$(CP) $(CDUP)icons/mos/YAM_directory.info "$@/MorphOS/YAM 2.6.info"
	@echo "  MK MorphOS/Catalogs"
	@$(MKDIR) "$@/MorphOS/YAM 2.6/Catalogs"
	@$(CP) $(CDUP)icons/mos/Catalogs_directory.info "$@/MorphOS/YAM 2.6/Catalogs.info"
	@$(CP) $(LOCALE)/YAM.cd "$@/MorphOS/YAM 2.6/Catalogs/YAM.cd"
	@$(MKDIR) "$@/MorphOS/YAM 2.6/Catalogs/czech"
	@$(CP) $(LOCALE)/czech.catalog "$@/MorphOS/YAM 2.6/Catalogs/czech/YAM.catalog"
	@$(MKDIR) "$@/MorphOS/YAM 2.6/Catalogs/french"
	@$(CP) $(LOCALE)/french.catalog "$@/MorphOS/YAM 2.6/Catalogs/french/YAM.catalog"
	@$(MKDIR) "$@/MorphOS/YAM 2.6/Catalogs/german"
	@$(CP) $(LOCALE)/german.catalog "$@/MorphOS/YAM 2.6/Catalogs/german/YAM.catalog"
	@$(MKDIR) "$@/MorphOS/YAM 2.6/Catalogs/greek"
	@$(CP) $(LOCALE)/greek.catalog "$@/MorphOS/YAM 2.6/Catalogs/greek/YAM.catalog"
	@$(MKDIR) "$@/MorphOS/YAM 2.6/Catalogs/italian"
	@$(CP) $(LOCALE)/italian.catalog "$@/MorphOS/YAM 2.6/Catalogs/italian/YAM.catalog"
	@$(MKDIR) "$@/MorphOS/YAM 2.6/Catalogs/swedish"
	@$(CP) $(LOCALE)/swedish.catalog "$@/MorphOS/YAM 2.6/Catalogs/swedish/YAM.catalog"
	@echo "  MK MorphOS/Docs"
	@$(MKDIR) "$@/MorphOS/YAM 2.6/Docs"
	@$(CP) $(CDUP)icons/mos/Docs_directory.info "$@/MorphOS/YAM 2.6/Docs.info"
	@$(CP) $(CDUP)doc/YAM_english.guide "$@/MorphOS/YAM 2.6/Docs/YAM_english.guide"
	@$(CP) $(CDUP)icons/mos/guide.info "$@/MorphOS/YAM 2.6/Docs/YAM_english.guide.info"
	@$(CP) $(CDUP)doc/YAM_french.guide "$@/MorphOS/YAM 2.6/Docs/YAM_french.guide"
	@$(CP) $(CDUP)icons/mos/guide.info "$@/MorphOS/YAM 2.6/Docs/YAM_french.guide.info"
	@$(CP) $(CDUP)AUTHORS "$@/MorphOS/YAM 2.6/Docs/AUTHORS"
	@$(CP) $(CDUP)icons/mos/Docs_AUTHORS.info "$@/MorphOS/YAM 2.6/Docs/AUTHORS.info"
	@$(CP) $(CDUP)ChangeLog "$@/MorphOS/YAM 2.6/Docs/ChangeLog"
	@$(CP) $(CDUP)icons/mos/Docs_ChangeLog.info "$@/MorphOS/YAM 2.6/Docs/ChangeLog.info"
	@$(CP) $(CDUP)COPYING "$@/MorphOS/YAM 2.6/Docs/COPYING"
	@$(CP) $(CDUP)icons/mos/Docs_COPYING.info "$@/MorphOS/YAM 2.6/Docs/COPYING.info"
	@echo "  MK MorphOS/Gallery"
	@$(MKDIR) "$@/MorphOS/YAM 2.6/Gallery"
	@$(CP) $(CDUP)icons/mos/Gallery_directory.info "$@/MorphOS/YAM 2.6/Gallery.info"
	@$(CP) $(CDUP)doc/ReadMe_gallery "$@/MorphOS/YAM 2.6/Gallery/ReadMe"
	@$(CP) $(CDUP)icons/mos/ReadMe.info "$@/MorphOS/YAM 2.6/Gallery/ReadMe.info"
	@echo "  MK MorphOS/Install"
	@@svn export -q "https://yamos.svn.sourceforge.net/svnroot/yamos/distribution/MorphOS/YAM 2.6/Install" "$@/MorphOS/YAM 2.6/Install"
	@$(CP) $(CDUP)icons/mos/Install_directory.info "$@/MorphOS/YAM 2.6/Install.info"
	@$(CP) $(CDUP)doc/Install-YAM "$@/MorphOS/YAM 2.6/Install/Install-YAM"
	@$(CP) $(CDUP)icons/mos/Install-YAM.info "$@/MorphOS/YAM 2.6/Install/Install-YAM.info"
	@echo "  MK MorphOS/Rexx"
	@svn export -q $(CDUP)rexx "$@/MorphOS/YAM 2.6/Rexx"
	@$(CP) $(CDUP)icons/mos/Rexx_directory.info "$@/MorphOS/YAM 2.6/Rexx.info"
	@echo "  MK MorphOS/Themes"
	@svn export -q $(CDUP)themes "$@/MorphOS/YAM 2.6/Themes"
	@$(CP) $(CDUP)icons/mos/Themes_directory.info "$@/MorphOS/YAM 2.6/Themes.info"
	@$(CP) $(CDUP)themes/ReadMe "$@/MorphOS/YAM 2.6/Themes/ReadMe"
	@$(CP) $(CDUP)icons/mos/Themes_ReadMe.info "$@/MorphOS/YAM 2.6/Themes/ReadMe.info"
	@$(CP) YAM.mos "$@/MorphOS/YAM 2.6/YAM"
	@$(CP) $(CDUP)icons/mos/YAM.info "$@/MorphOS/YAM 2.6/YAM.info"
	@$(CP) $(CDUP)doc/ReadMe "$@/MorphOS/YAM 2.6/ReadMe"
	@$(CP) $(CDUP)icons/mos/ReadMe.info "$@/MorphOS/YAM 2.6/ReadMe.info"
	@echo "  MK YAM26-MorphOS.lha"
	@cd $@/MorphOS && lha -aq $(CDUP)YAM26-MorphOS.lha *
## MorphOS-debug distribution ########
	@echo "  MK MorphOS-debug"
	@$(MAKE) OS=os4 DEVFLAGS= clean
	@$(MAKE) OS=os4 DEVFLAGS= all
	@$(CP) $(CDUP)doc/README.debug "$@/"
	@$(CP) YAM.mos.debug "$@/YAM.debug"
	@$(CP) YAM.mos.map "$@/YAM.debug.map"
	@echo "  MK YAM26-MorphOS-debug.lha"
	@cd $@ && lha -aq YAM26-MorphOS-debug.lha README.debug YAM.debug YAM.debug.map
## AROS-i386 distribution #############
	@echo "  MK AROS-i386"
	@$(MAKE) OS=aros-i386 DEBUG= DEVFLAGS= clean
	@$(MAKE) OS=aros-i386 DEBUG= DEVFLAGS= all
	@$(MKDIR) "$@/AROS-i386"
	@$(MKDIR) "$@/AROS-i386/YAM 2.6"
	@$(CP) $(CDUP)icons/aros/YAM_directory.info "$@/AROS-i386/YAM 2.6.info"
	@echo "  MK AROS-i386/Catalogs"
	@$(MKDIR) "$@/AROS-i386/YAM 2.6/Catalogs"
	@$(CP) $(CDUP)icons/aros/Catalogs_directory.info "$@/AROS-i386/YAM 2.6/Catalogs.info"
	@$(CP) $(LOCALE)/YAM.cd "$@/AROS-i386/YAM 2.6/Catalogs/YAM.cd"
	@$(MKDIR) "$@/AROS-i386/YAM 2.6/Catalogs/czech"
	@$(CP) $(LOCALE)/czech.catalog "$@/AROS-i386/YAM 2.6/Catalogs/czech/YAM.catalog"
	@$(MKDIR) "$@/AROS-i386/YAM 2.6/Catalogs/french"
	@$(CP) $(LOCALE)/french.catalog "$@/AROS-i386/YAM 2.6/Catalogs/french/YAM.catalog"
	@$(MKDIR) "$@/AROS-i386/YAM 2.6/Catalogs/german"
	@$(CP) $(LOCALE)/german.catalog "$@/AROS-i386/YAM 2.6/Catalogs/german/YAM.catalog"
	@$(MKDIR) "$@/AROS-i386/YAM 2.6/Catalogs/greek"
	@$(CP) $(LOCALE)/greek.catalog "$@/AROS-i386/YAM 2.6/Catalogs/greek/YAM.catalog"
	@$(MKDIR) "$@/AROS-i386/YAM 2.6/Catalogs/italian"
	@$(CP) $(LOCALE)/italian.catalog "$@/AROS-i386/YAM 2.6/Catalogs/italian/YAM.catalog"
	@$(MKDIR) "$@/AROS-i386/YAM 2.6/Catalogs/swedish"
	@$(CP) $(LOCALE)/swedish.catalog "$@/AROS-i386/YAM 2.6/Catalogs/swedish/YAM.catalog"
	@echo "  MK AROS-i386/Docs"
	@$(MKDIR) "$@/AROS-i386/YAM 2.6/Docs"
	@$(CP) $(CDUP)icons/aros/Docs_directory.info "$@/AROS-i386/YAM 2.6/Docs.info"
	@$(CP) $(CDUP)doc/YAM_english.guide "$@/AROS-i386/YAM 2.6/Docs/YAM_english.guide"
	@$(CP) $(CDUP)icons/aros/guide.info "$@/AROS-i386/YAM 2.6/Docs/YAM_english.guide.info"
	@$(CP) $(CDUP)doc/YAM_french.guide "$@/AROS-i386/YAM 2.6/Docs/YAM_french.guide"
	@$(CP) $(CDUP)icons/aros/guide.info "$@/AROS-i386/YAM 2.6/Docs/YAM_french.guide.info"
	@$(CP) $(CDUP)AUTHORS "$@/AROS-i386/YAM 2.6/Docs/AUTHORS"
	@$(CP) $(CDUP)icons/aros/Docs_AUTHORS.info "$@/AROS-i386/YAM 2.6/Docs/AUTHORS.info"
	@$(CP) $(CDUP)ChangeLog "$@/AROS-i386/YAM 2.6/Docs/ChangeLog"
	@$(CP) $(CDUP)icons/aros/Docs_ChangeLog.info "$@/AROS-i386/YAM 2.6/Docs/ChangeLog.info"
	@$(CP) $(CDUP)COPYING "$@/AROS-i386/YAM 2.6/Docs/COPYING"
	@$(CP) $(CDUP)icons/aros/Docs_COPYING.info "$@/AROS-i386/YAM 2.6/Docs/COPYING.info"
	@echo "  MK AROS-i386/Gallery"
	@$(MKDIR) "$@/AROS-i386/YAM 2.6/Gallery"
	@$(CP) $(CDUP)icons/aros/Gallery_directory.info "$@/AROS-i386/YAM 2.6/Gallery.info"
	@$(CP) $(CDUP)doc/ReadMe_gallery "$@/AROS-i386/YAM 2.6/Gallery/ReadMe"
	@$(CP) $(CDUP)icons/aros/ReadMe.info "$@/AROS-i386/YAM 2.6/Gallery/ReadMe.info"
	@echo "  MK AROS-i386/Install"
	@@svn export -q "https://yamos.svn.sourceforge.net/svnroot/yamos/distribution/AROS-i386/YAM 2.6/Install" "$@/AROS-i386/YAM 2.6/Install"
	@$(CP) $(CDUP)icons/aros/Install_directory.info "$@/AROS-i386/YAM 2.6/Install.info"
	@$(CP) $(CDUP)doc/Install-YAM "$@/AROS-i386/YAM 2.6/Install/Install-YAM"
	@$(CP) $(CDUP)icons/aros/Install-YAM.info "$@/AROS-i386/YAM 2.6/Install/Install-YAM.info"
	@echo "  MK AROS-i386/Rexx"
	@svn export -q $(CDUP)rexx "$@/AROS-i386/YAM 2.6/Rexx"
	@$(CP) $(CDUP)icons/aros/Rexx_directory.info "$@/AROS-i386/YAM 2.6/Rexx.info"
	@echo "  MK AROS-i386/Themes"
	@svn export -q $(CDUP)themes "$@/AROS-i386/YAM 2.6/Themes"
	@$(CP) $(CDUP)icons/aros/Themes_directory.info "$@/AROS-i386/YAM 2.6/Themes.info"
	@$(CP) $(CDUP)themes/ReadMe "$@/AROS-i386/YAM 2.6/Themes/ReadMe"
	@$(CP) $(CDUP)icons/aros/Themes_ReadMe.info "$@/AROS-i386/YAM 2.6/Themes/ReadMe.info"
	@$(CP) YAM.aros-i386 "$@/AROS-i386/YAM 2.6/YAM"
	@$(CP) $(CDUP)icons/aros/YAM.info "$@/AROS-i386/YAM 2.6/YAM.info"
	@$(CP) $(CDUP)doc/ReadMe "$@/AROS-i386/YAM 2.6/ReadMe"
	@$(CP) $(CDUP)icons/aros/ReadMe.info "$@/AROS-i386/YAM 2.6/ReadMe.info"
	@echo "  MK YAM26-AROS-i386.lha"
	@cd $@/AROS-i386 && lha -aq $(CDUP)YAM26-AROS-i386.lha *
## AROS-i386-debug distribution ########
	@echo "  MK AROS-i386-debug"
	@$(MAKE) OS=os4 DEVFLAGS= clean
	@$(MAKE) OS=os4 DEVFLAGS= all
	@$(CP) $(CDUP)doc/README.debug "$@/"
	@$(CP) YAM.aros-i386.debug "$@/YAM.debug"
	@$(CP) YAM.aros-i386.map "$@/YAM.debug.map"
	@echo "  MK YAM26-AROS-i386-debug.lha"
	@cd $@ && lha -aq YAM26-AROS-i386-debug.lha README.debug YAM.debug YAM.debug.map
## AROS-ppc distribution ##############
	@echo "  MK AROS-ppc"
	@$(MAKE) OS=aros-ppc DEBUG= DEVFLAGS= clean
	@$(MAKE) OS=aros-ppc DEBUG= DEVFLAGS= all
	@$(MKDIR) "$@/AROS-ppc"
	@$(MKDIR) "$@/AROS-ppc/YAM 2.6"
	@$(CP) $(CDUP)icons/aros/YAM_directory.info "$@/AROS-ppc/YAM 2.6.info"
	@echo "  MK AROS-ppc/Catalogs"
	@$(MKDIR) "$@/AROS-ppc/YAM 2.6/Catalogs"
	@$(CP) $(CDUP)icons/aros/Catalogs_directory.info "$@/AROS-ppc/YAM 2.6/Catalogs.info"
	@$(CP) $(LOCALE)/YAM.cd "$@/AROS-ppc/YAM 2.6/Catalogs/YAM.cd"
	@$(MKDIR) "$@/AROS-ppc/YAM 2.6/Catalogs/czech"
	@$(CP) $(LOCALE)/czech.catalog "$@/AROS-ppc/YAM 2.6/Catalogs/czech/YAM.catalog"
	@$(MKDIR) "$@/AROS-ppc/YAM 2.6/Catalogs/french"
	@$(CP) $(LOCALE)/french.catalog "$@/AROS-ppc/YAM 2.6/Catalogs/french/YAM.catalog"
	@$(MKDIR) "$@/AROS-ppc/YAM 2.6/Catalogs/german"
	@$(CP) $(LOCALE)/german.catalog "$@/AROS-ppc/YAM 2.6/Catalogs/german/YAM.catalog"
	@$(MKDIR) "$@/AROS-ppc/YAM 2.6/Catalogs/greek"
	@$(CP) $(LOCALE)/greek.catalog "$@/AROS-ppc/YAM 2.6/Catalogs/greek/YAM.catalog"
	@$(MKDIR) "$@/AROS-ppc/YAM 2.6/Catalogs/italian"
	@$(CP) $(LOCALE)/italian.catalog "$@/AROS-ppc/YAM 2.6/Catalogs/italian/YAM.catalog"
	@$(MKDIR) "$@/AROS-ppc/YAM 2.6/Catalogs/swedish"
	@$(CP) $(LOCALE)/swedish.catalog "$@/AROS-ppc/YAM 2.6/Catalogs/swedish/YAM.catalog"
	@echo "  MK AROS-ppc/Docs"
	@$(MKDIR) "$@/AROS-ppc/YAM 2.6/Docs"
	@$(CP) $(CDUP)icons/aros/Docs_directory.info "$@/AROS-ppc/YAM 2.6/Docs.info"
	@$(CP) $(CDUP)doc/YAM_english.guide "$@/AROS-ppc/YAM 2.6/Docs/YAM_english.guide"
	@$(CP) $(CDUP)icons/aros/guide.info "$@/AROS-ppc/YAM 2.6/Docs/YAM_english.guide.info"
	@$(CP) $(CDUP)doc/YAM_french.guide "$@/AROS-ppc/YAM 2.6/Docs/YAM_french.guide"
	@$(CP) $(CDUP)icons/aros/guide.info "$@/AROS-ppc/YAM 2.6/Docs/YAM_french.guide.info"
	@$(CP) $(CDUP)AUTHORS "$@/AROS-ppc/YAM 2.6/Docs/AUTHORS"
	@$(CP) $(CDUP)icons/aros/Docs_AUTHORS.info "$@/AROS-ppc/YAM 2.6/Docs/AUTHORS.info"
	@$(CP) $(CDUP)ChangeLog "$@/AROS-ppc/YAM 2.6/Docs/ChangeLog"
	@$(CP) $(CDUP)icons/aros/Docs_ChangeLog.info "$@/AROS-ppc/YAM 2.6/Docs/ChangeLog.info"
	@$(CP) $(CDUP)COPYING "$@/AROS-ppc/YAM 2.6/Docs/COPYING"
	@$(CP) $(CDUP)icons/aros/Docs_COPYING.info "$@/AROS-ppc/YAM 2.6/Docs/COPYING.info"
	@echo "  MK AROS-ppc/Gallery"
	@$(MKDIR) "$@/AROS-ppc/YAM 2.6/Gallery"
	@$(CP) $(CDUP)icons/aros/Gallery_directory.info "$@/AROS-ppc/YAM 2.6/Gallery.info"
	@$(CP) $(CDUP)doc/ReadMe_gallery "$@/AROS-ppc/YAM 2.6/Gallery/ReadMe"
	@$(CP) $(CDUP)icons/aros/ReadMe.info "$@/AROS-ppc/YAM 2.6/Gallery/ReadMe.info"
	@echo "  MK AROS-ppc/Install"
	@@svn export -q "https://yamos.svn.sourceforge.net/svnroot/yamos/distribution/AROS-ppc/YAM 2.6/Install" "$@/AROS-ppc/YAM 2.6/Install"
	@$(CP) $(CDUP)icons/aros/Install_directory.info "$@/AROS-ppc/YAM 2.6/Install.info"
	@$(CP) $(CDUP)doc/Install-YAM "$@/AROS-ppc/YAM 2.6/Install/Install-YAM"
	@$(CP) $(CDUP)icons/aros/Install-YAM.info "$@/AROS-ppc/YAM 2.6/Install/Install-YAM.info"
	@echo "  MK AROS-ppc/Rexx"
	@svn export -q $(CDUP)rexx "$@/AROS-ppc/YAM 2.6/Rexx"
	@$(CP) $(CDUP)icons/aros/Rexx_directory.info "$@/AROS-ppc/YAM 2.6/Rexx.info"
	@echo "  MK AROS-ppc/Themes"
	@svn export -q $(CDUP)themes "$@/AROS-ppc/YAM 2.6/Themes"
	@$(CP) $(CDUP)icons/aros/Themes_directory.info "$@/AROS-ppc/YAM 2.6/Themes.info"
	@$(CP) $(CDUP)themes/ReadMe "$@/AROS-ppc/YAM 2.6/Themes/ReadMe"
	@$(CP) $(CDUP)icons/aros/Themes_ReadMe.info "$@/AROS-ppc/YAM 2.6/Themes/ReadMe.info"
	@$(CP) YAM.aros-ppc "$@/AROS-ppc/YAM 2.6/YAM"
	@$(CP) $(CDUP)icons/aros/YAM.info "$@/AROS-ppc/YAM 2.6/YAM.info"
	@$(CP) $(CDUP)doc/ReadMe "$@/AROS-ppc/YAM 2.6/ReadMe"
	@$(CP) $(CDUP)icons/aros/ReadMe.info "$@/AROS-ppc/YAM 2.6/ReadMe.info"
	@echo "  MK YAM26-AROS-ppc.lha"
	@cd $@/AROS-ppc && lha -aq $(CDUP)YAM26-AROS-ppc.lha *
## AROS-ppc-debug distribution ########
	@echo "  MK AROS-ppc-debug"
	@$(MAKE) OS=os4 DEVFLAGS= clean
	@$(MAKE) OS=os4 DEVFLAGS= all
	@$(CP) $(CDUP)doc/README.debug "$@/"
	@$(CP) YAM.aros-ppc.debug "$@/YAM.debug"
	@$(CP) YAM.aros-ppc.map "$@/YAM.debug.map"
	@echo "  MK YAM26-AROS-ppc-debug.lha"
	@cd $@ && lha -aq YAM26-AROS-ppc-debug.lha README.debug YAM.debug YAM.debug.map
## AROS-x86_64 distribution ###########
	@echo "  MK AROS-x86_64"
	@$(MAKE) OS=aros-x86_64 DEBUG= DEVFLAGS= clean
	@$(MAKE) OS=aros-x86_64 DEBUG= DEVFLAGS= all
	@$(MKDIR) "$@/AROS-x86_64"
	@$(MKDIR) "$@/AROS-x86_64/YAM 2.6"
	@$(CP) $(CDUP)icons/aros/YAM_directory.info "$@/AROS-x86_64/YAM 2.6.info"
	@echo "  MK AROS-x86_64/Catalogs"
	@$(MKDIR) "$@/AROS-x86_64/YAM 2.6/Catalogs"
	@$(CP) $(CDUP)icons/aros/Catalogs_directory.info "$@/AROS-x86_64/YAM 2.6/Catalogs.info"
	@$(CP) $(LOCALE)/YAM.cd "$@/AROS-x86_64/YAM 2.6/Catalogs/YAM.cd"
	@$(MKDIR) "$@/AROS-x86_64/YAM 2.6/Catalogs/czech"
	@$(CP) $(LOCALE)/czech.catalog "$@/AROS-x86_64/YAM 2.6/Catalogs/czech/YAM.catalog"
	@$(MKDIR) "$@/AROS-x86_64/YAM 2.6/Catalogs/french"
	@$(CP) $(LOCALE)/french.catalog "$@/AROS-x86_64/YAM 2.6/Catalogs/french/YAM.catalog"
	@$(MKDIR) "$@/AROS-x86_64/YAM 2.6/Catalogs/german"
	@$(CP) $(LOCALE)/german.catalog "$@/AROS-x86_64/YAM 2.6/Catalogs/german/YAM.catalog"
	@$(MKDIR) "$@/AROS-x86_64/YAM 2.6/Catalogs/greek"
	@$(CP) $(LOCALE)/greek.catalog "$@/AROS-x86_64/YAM 2.6/Catalogs/greek/YAM.catalog"
	@$(MKDIR) "$@/AROS-x86_64/YAM 2.6/Catalogs/italian"
	@$(CP) $(LOCALE)/italian.catalog "$@/AROS-x86_64/YAM 2.6/Catalogs/italian/YAM.catalog"
	@$(MKDIR) "$@/AROS-x86_64/YAM 2.6/Catalogs/swedish"
	@$(CP) $(LOCALE)/swedish.catalog "$@/AROS-x86_64/YAM 2.6/Catalogs/swedish/YAM.catalog"
	@echo "  MK AROS-x86_64/Docs"
	@$(MKDIR) "$@/AROS-x86_64/YAM 2.6/Docs"
	@$(CP) $(CDUP)icons/aros/Docs_directory.info "$@/AROS-x86_64/YAM 2.6/Docs.info"
	@$(CP) $(CDUP)doc/YAM_english.guide "$@/AROS-x86_64/YAM 2.6/Docs/YAM_english.guide"
	@$(CP) $(CDUP)icons/aros/guide.info "$@/AROS-x86_64/YAM 2.6/Docs/YAM_english.guide.info"
	@$(CP) $(CDUP)doc/YAM_french.guide "$@/AROS-x86_64/YAM 2.6/Docs/YAM_french.guide"
	@$(CP) $(CDUP)icons/aros/guide.info "$@/AROS-x86_64/YAM 2.6/Docs/YAM_french.guide.info"
	@$(CP) $(CDUP)AUTHORS "$@/AROS-x86_64/YAM 2.6/Docs/AUTHORS"
	@$(CP) $(CDUP)icons/aros/Docs_AUTHORS.info "$@/AROS-x86_64/YAM 2.6/Docs/AUTHORS.info"
	@$(CP) $(CDUP)ChangeLog "$@/AROS-x86_64/YAM 2.6/Docs/ChangeLog"
	@$(CP) $(CDUP)icons/aros/Docs_ChangeLog.info "$@/AROS-x86_64/YAM 2.6/Docs/ChangeLog.info"
	@$(CP) $(CDUP)COPYING "$@/AROS-x86_64/YAM 2.6/Docs/COPYING"
	@$(CP) $(CDUP)icons/aros/Docs_COPYING.info "$@/AROS-x86_64/YAM 2.6/Docs/COPYING.info"
	@echo "  MK AROS-x86_64/Gallery"
	@$(MKDIR) "$@/AROS-x86_64/YAM 2.6/Gallery"
	@$(CP) $(CDUP)icons/aros/Gallery_directory.info "$@/AROS-x86_64/YAM 2.6/Gallery.info"
	@$(CP) $(CDUP)doc/ReadMe_gallery "$@/AROS-x86_64/YAM 2.6/Gallery/ReadMe"
	@$(CP) $(CDUP)icons/aros/ReadMe.info "$@/AROS-x86_64/YAM 2.6/Gallery/ReadMe.info"
	@echo "  MK AROS-x86_64/Install"
	@@svn export -q "https://yamos.svn.sourceforge.net/svnroot/yamos/distribution/AROS-x86_64/YAM 2.6/Install" "$@/AROS-x86_64/YAM 2.6/Install"
	@$(CP) $(CDUP)icons/aros/Install_directory.info "$@/AROS-x86_64/YAM 2.6/Install.info"
	@$(CP) $(CDUP)doc/Install-YAM "$@/AROS-x86_64/YAM 2.6/Install/Install-YAM"
	@$(CP) $(CDUP)icons/aros/Install-YAM.info "$@/AROS-x86_64/YAM 2.6/Install/Install-YAM.info"
	@echo "  MK AROS-x86_64/Rexx"
	@svn export -q $(CDUP)rexx "$@/AROS-x86_64/YAM 2.6/Rexx"
	@$(CP) $(CDUP)icons/aros/Rexx_directory.info "$@/AROS-x86_64/YAM 2.6/Rexx.info"
	@echo "  MK AROS-x86_64/Themes"
	@svn export -q $(CDUP)themes "$@/AROS-x86_64/YAM 2.6/Themes"
	@$(CP) $(CDUP)icons/aros/Themes_directory.info "$@/AROS-x86_64/YAM 2.6/Themes.info"
	@$(CP) $(CDUP)themes/ReadMe "$@/AROS-x86_64/YAM 2.6/Themes/ReadMe"
	@$(CP) $(CDUP)icons/aros/Themes_ReadMe.info "$@/AROS-x86_64/YAM 2.6/Themes/ReadMe.info"
	@$(CP) YAM.aros-x86_64 "$@/AROS-x86_64/YAM 2.6/YAM"
	@$(CP) $(CDUP)icons/aros/YAM.info "$@/AROS-x86_64/YAM 2.6/YAM.info"
	@$(CP) $(CDUP)doc/ReadMe "$@/AROS-x86_64/YAM 2.6/ReadMe"
	@$(CP) $(CDUP)icons/aros/ReadMe.info "$@/AROS-x86_64/YAM 2.6/ReadMe.info"
	@echo "  MK YAM26-AROS-x86_64.lha"
	@cd $@/AROS-x86_64 && lha -aq $(CDUP)YAM26-AROS-x86_64.lha *
## AROS-x86_64-debug distribution ########
	@echo "  MK AROS-x86_64-debug"
	@$(MAKE) OS=os4 DEVFLAGS= clean
	@$(MAKE) OS=os4 DEVFLAGS= all
	@$(CP) $(CDUP)doc/README.debug "$@/"
	@$(CP) YAM.aros-x86_64.debug "$@/YAM.debug"
	@$(CP) YAM.aros-x86_64.map "$@/YAM.debug.map"
	@echo "  MK YAM26-AROS-x86_64-debug.lha"
	@cd $@ && lha -aq YAM26-AROS-x86_64-debug.lha README.debug YAM.debug YAM.debug.map

## DEPENDENCY INCLUDE #################

-include Makefile.dep
