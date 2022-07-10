#/***************************************************************************
#
# YAM - Yet Another Mailer
# Copyright (C) 1995-2000 Marcel Beck
# Copyright (C) 2000-2022 YAM Open Source Team
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
    ifeq ($(shell uname -m), arm)
      OS = aros-arm
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
SED     = sed
CP      = copy
CC      = gcc
STRIP   = strip
OBJDUMP = objdump
TX      = tx

# path definitions
CDUP  = /
CDTHIS=

# Special macros for version ID and developer builds via Arexx
# and C:Date calls
DEVFLAGS = -DDEVWARNING #`rx "say '-DEXPDATE=' || date(I)+35"`
YAMVERID = -D__YAM_BUILDID=$(BUILDID) \
           `rx "say '-D__YAM_VERDATE=' || '22'x || right(date(S),2)'.'substr(date(S),5,2)'.'left(date(S),4) || '22'x"` \
           `rx "say '-D__YAM_BUILDDATE=' || '22'x || date(S) || '22'x"` \
           `rx "say '-D__YAM_VERDAYS=' || date(I)"` \

# override some variables for non-native builds (cross-compiler)
ifneq ($(HOST), AmigaOS)
ifneq ($(HOST), AmigaOS4)

  # this is either a cross-compiler build on a UNIX-like system or
  # on MorphOS, so lets use expr/gdate instead of the above ARexx stuff
  DEVFLAGS = -DDEVWARNING #-DEXPDATE="$(shell $(EXPR) `$(DATE) +%s` / 86400 - 2922 + 35)"
  YAMVERID = -D__YAM_VERDATE=\"$(shell $(DATE) +%d.%m.%Y)\" \
             -D__YAM_VERDAYS="$(shell $(EXPR) `$(DATE) +%s` / 86400 - 2922)" \
             -D__YAM_BUILDDATE=\"$(shell $(DATE) +%Y%m%d)\" \
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

# Common Directories
PREFIX    = $(CDTHIS)
MUIDIR    = $(PREFIX)mui
REXXDIR   = $(PREFIX)rexx
MIMEDIR   = $(PREFIX)mime
TCPDIR    = $(PREFIX)tcp
LOCALE    = $(PREFIX)$(CDUP)locale
EXTRADIR  = $(PREFIX)extrasrc
TOOLS     = $(PREFIX)tools
LIB       = $(PREFIX)lib/$(OS)
OBJDIR    = .obj_$(OS)
DEPDIR    = .dep_$(OS)
VPATH     = $(OBJDIR)
GCCVER    = 4

# Common compiler/linker flags
WARN     = -W -Wall -Wwrite-strings -Wpointer-arith -Wsign-compare #-Wunreachable-code
OPTFLAGS = -O3 -fomit-frame-pointer -fno-strict-aliasing
DEBUG    = -DDEBUG -fno-omit-frame-pointer #-O0
DEBUGSYM = -g -gstabs
CFLAGS   = -I. -I./include $(CPU) $(WARN) $(OPTFLAGS) $(DEVFLAGS) $(DEBUG) $(DEBUGSYM)
LDFLAGS  = $(CPU) $(DEBUGSYM)
LDLIBS   = -L$(LIB) -lm -ltz

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
  WARN     += -Wdeclaration-after-statement -Wdisabled-optimization -Wshadow
  REDEFINE = -DCoerceMethod=ICoerceMethod -DDoMethod=IDoMethod \
             -DDoSuperMethod=IDoSuperMethod -DDoSuperMethodA=IDoSuperMethodA
  CFLAGS   += -mcrt=$(CRT) -D__USE_INLINE__ -D__NEW_TIMEVAL_DEFINITION_USED__ \
              $(REDEFINE) -Wa,-mregnames
  LDFLAGS  += -mcrt=$(CRT)
  CDUP     = ../
  CDTHIS   = ./

  EXTRAOBJS = DoSuperNew.o \
              NewReadArgs.o \
              getdelim.o \
              memdup.o \
              stcgfe.o \
              strcasestr.o

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
  CFLAGS  += -mcrt=clib2 -I./include/netinclude -DNO_INLINE_STDARG -D__amigaos3__
  LDFLAGS += -mcrt=clib2
  LDLIBS  = ./lib/os3/libtz.a ./lib/os3/libmui.a ./lib/os3/libm.a ./lib/os3/libdebug.a ./lib/os3/libamiga.a ./lib/os3/libc.a -ldebug -lmui
  GCCVER  = 2

  EXTRAOBJS = AllocSysObject.o \
              AllocVecPooled.o \
              DoSuperNew.o \
              ExamineDir.o \
              FreeVecPooled.o \
              GetHead.o \
              GetPred.o \
              GetSucc.o \
              GetTail.o \
              ItemPoolAlloc.o \
              ItemPoolFree.o \
              MoveList.o \
              NewMinList.o \
              NewReadArgs.o \
              SetProcWindow.o \
              getdelim.o \
              memdup.o \
              stcgfe.o \
              strcasestr.o \
              vastubs.o

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
  CFLAGS  += -noixemul -I./include/netinclude -Wno-pointer-sign
  LDFLAGS += -noixemul
  LDLIBS  += -lxpk -lamissl -lcodesets -lopenurl

  EXTRAOBJS = AllocSysObject.o \
              ExamineDir.o \
              GetHead.o \
              GetPred.o \
              GetSucc.o \
              GetTail.o \
              ItemPoolAlloc.o \
              ItemPoolFree.o \
              MoveList.o \
              NewReadArgs.o \
              SetProcWindow.o \
              asprintf.o \
              getdelim.o \
              memdup.o \
              vasprintf.o

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
  OPTFLAGS = -O2 -fomit-frame-pointer
  CFLAGS += -Wno-pointer-sign -DNO_INLINE_STDARG -D__BSD_VISIBLE=1
  LDLIBS += -lamiga -larossupport -larosc -lmui

  EXTRAOBJS = AllocSysObject.o \
              DoSuperNew.o \
              ExamineDir.o \
              ItemPoolAlloc.o \
              ItemPoolFree.o \
              MoveList.o \
              NewMinList.o \
              NewReadArgs.o \
              SetProcWindow.o \
              asprintf.o \
              getdelim.o \
              memdup.o \
              stcgfe.o \
              strtok_r.o \
              vasprintf.o \
              vastubs.o

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
  OPTFLAGS = -O2 -fomit-frame-pointer
  CFLAGS += -Wno-pointer-sign -DNO_INLINE_STDARG -D__BSD_VISIBLE=1
  LDLIBS += -lamiga -larossupport -larosc -lmui

  EXTRAOBJS = AllocSysObject.o \
              DoSuperNew.o \
              ExamineDir.o \
              ItemPoolAlloc.o \
              ItemPoolFree.o \
              MoveList.o \
              NewMinList.o \
              NewReadArgs.o \
              SetProcWindow.o \
              asprintf.o \
              getdelim.o \
              memdup.o \
              stcgfe.o \
              strtok_r.o \
              vasprintf.o \
              vastubs.o

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
  OPTFLAGS = -O2 -fomit-frame-pointer
  CFLAGS += -Wno-pointer-sign -DNO_INLINE_STDARG -D__BSD_VISIBLE=1
  LDLIBS += -lamiga -larossupport -larosc -lmui

  EXTRAOBJS = AllocSysObject.o \
              DoSuperNew.o \
              ExamineDir.o \
              ItemPoolAlloc.o \
              ItemPoolFree.o \
              MoveList.o \
              NewMinList.o \
              NewReadArgs.o \
              SetProcWindow.o \
              asprintf.o \
              getdelim.o \
              memdup.o \
              stcgfe.o \
              strtok_r.o \
              vasprintf.o \
              vastubs.o

else
ifeq ($(OS), aros-arm)

  ##############################
  # AROS (x86_64)

  ifneq ($(HOST), AROS)
    CC      = arm-aros-gcc
    STRIP   = arm-aros-strip
    OBJDUMP = arm-aros-objdump
  endif

  # Compiler/Linker flags
  OPTFLAGS = -O2 -fomit-frame-pointer
  CFLAGS += -Wno-pointer-sign -DNO_INLINE_STDARG -D__BSD_VISIBLE=1
  LDLIBS += -lamiga -larossupport -larosc -lmui

  EXTRAOBJS = AllocSysObject.o \
              DoSuperNew.o \
              ExamineDir.o \
              ItemPoolAlloc.o \
              ItemPoolFree.o \
              MoveList.o \
              NewMinList.o \
              NewReadArgs.o \
              SetProcWindow.o \
              asprintf.o \
              getdelim.o \
              memdup.o \
              stcgfe.o \
              strtok_r.o \
              vasprintf.o \
              vastubs.o

endif
endif
endif
endif
endif
endif
endif

###########################################################################
# Here starts all stuff that is common for all target platforms and
# hosts.

# Third-party tools/libraries
GENCLASSES = $(TOOLS)/genclasses
TZLIB = $(TOOLS)/tz

# our few new rewritten classes
MUIOBJS = \
	Classes.o \
	AboutWindow.o \
	AccountList.o \
	AddressBookConfigPage.o \
	AddressBookEditGroupPanel.o \
	AddressBookEditListPanel.o \
	AddressBookEditUserPanel.o \
	AddressBookEditWindow.o \
	AddressBookEntryList.o \
	AddressBookListtree.o \
	AddressBookToolbar.o \
	AddressBookWindow.o \
	AddressField.o \
	AddressMatchList.o \
	AddressMatchPopupWindow.o \
	Attachment.o \
	AttachmentGroup.o \
	AttachmentImage.o \
	AttachmentList.o \
	AttachmentKeywordList.o \
	AttachmentRequestWindow.o \
	Base64Dataspace.o \
	BirthdayRequestWindow.o \
	CheckboxRequestWindow.o \
	CodesetPopup.o \
	CodesetPopupList.o \
	ConfigPage.o \
	ConfigPageList.o \
	ConfigWindow.o \
	FilterChooser.o \
	FilterList.o \
	FilterPopupList.o \
	FilterRuleList.o \
	FiltersConfigPage.o \
	FirstStepsConfigPage.o \
	FolderEditWindow.o \
	FolderListtree.o \
	FolderRequestListtree.o \
	FolderRequestPopup.o \
	FolderRequestWindow.o \
	GenericRequestWindow.o \
	HeaderList.o \
	IdentitiesConfigPage.o \
	IdentityChooser.o \
	IdentityList.o \
	ImageArea.o \
	InfoBar.o \
	InfoWindow.o \
	LookFeelConfigPage.o \
	MailServerChooser.o \
	MailTextEdit.o \
	MainFolderListtree.o \
	MainMailList.o \
	MainMailListGroup.o \
	MainWindow.o \
	MainWindowToolbar.o \
	MimeConfigPage.o \
	MimeTypeList.o \
	MimeTypePopup.o \
	MixedConfigPage.o \
	ObjectList.o \
	ObjectListitem.o \
	PassphraseRequestWindow.o \
	PGPKeyPopup.o \
	PlaceholderPopup.o \
	PlaceholderPopupList.o \
	PreselectionWindow.o \
	QuickSearchBar.o \
	ReadConfigPage.o \
	ReadMailGroup.o \
	ReadWindow.o \
	ReadWindowStatusBar.o \
	ReadWindowToolbar.o \
	RecipientString.o \
	ReplyForwardConfigPage.o \
	ScriptList.o \
	ScriptsConfigPage.o \
	SearchControlGroup.o \
	SearchMailWindow.o \
	SearchTextWindow.o \
	SecurityConfigPage.o \
	ShutdownWindow.o \
	SignatureChooser.o \
	SignatureConfigPage.o \
	SignatureList.o \
	SignatureTextEdit.o \
	SpamConfigPage.o \
	SplashWindow.o \
	StartupQuitConfigPage.o \
	StringRequestWindow.o \
	TCPIPConfigPage.o \
	ThemeList.o \
	ThemeListGroup.o \
	TransferControlGroup.o \
	TransferControlList.o \
	TransferMailList.o \
	TransferWindow.o \
	TZoneChooser.o \
	TZoneContinentChooser.o \
	TZoneInfoBar.o \
	TZoneLocationChooser.o \
	UpdateComponentList.o \
	UpdateConfigPage.o \
	UpdateNotifyWindow.o \
	UserList.o \
	UserPortraitGroup.o \
	WriteAttachmentList.o \
	WriteConfigPage.o \
	WriteWindow.o \
	WriteWindowToolbar.o \
	YAMApplication.o

REXXOBJS = \
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
	findmail.o \
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
	restart.o \
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
	writeidentity.o \
	writeletter.o \
	writemailto.o \
	writeoptions.o \
	writequeue.o \
	writereplyto.o \
	writesend.o \
	writesubject.o \
	writeto.o

MIMEOBJS = \
	base64.o \
	qprintable.o \
	uucode.o \
	rfc1738.o \
	rfc2047.o \
	rfc2231.o \
	md5.o

TCPOBJS = \
	Connection.o \
  ssl.o \
	pop3.o \
	smtp.o \
	http.o

YAMOBJS = \
	YAM_global.o \
	YAM.o \
	YAM_DI.o \
	YAM_ER.o \
	YAM_FI.o \
	YAM_FO.o \
	YAM_MA.o \
	YAM_MAf.o \
	YAM_RE.o \
	YAM_US.o \
	YAM_UT.o \
	YAM_WR.o \
	AddressBook.o \
	AppIcon.o \
	BayesFilter.o \
	BoyerMooreSearch.o \
	Busy.o \
	Config.o \
	DockyIcon.o \
	DynamicString.o \
	FileInfo.o \
	FolderList.o \
	HashTable.o \
	HTML2Mail.o \
	ImageCache.o \
	Locale.o \
	Logfile.o \
	MailExport.o \
	MailImport.o \
	MailList.o \
	MailServers.o \
	MailTransferList.o \
	MethodStack.o \
	MimeTypes.o \
	MUIObjects.o \
	ParseEmail.o \
	Requesters.o \
	Rexx.o \
	Signature.o \
	Themes.o \
	Threads.o \
	Timer.o \
	TZone.o \
	UIDL.o \
	UpdateCheck.o \
	UserIdentity.o \
	Debug.o

# and the other object files where
# YAM_global.o and YAM.o have to stay at the top
OBJS = \
	$(YAMOBJS) \
	$(addprefix $(MUIDIR)/,$(MUIOBJS)) \
	$(addprefix $(REXXDIR)/,$(REXXOBJS)) \
	$(addprefix $(MIMEDIR)/,$(MIMEOBJS)) \
	$(addprefix $(TCPDIR)/,$(TCPOBJS)) \
	$(addprefix $(EXTRADIR)/,$(EXTRAOBJS))

# available catalog translations (po files)
CATALOGS = \
	$(patsubst %.po,%.catalog,$(wildcard $(LOCALE)/*.po))

# main target
.PHONY: all
all: $(OBJDIR) $(OBJDIR)/$(MUIDIR) $(OBJDIR)/$(REXXDIR) $(OBJDIR)/$(MIMEDIR) $(OBJDIR)/$(TCPDIR) $(OBJDIR)/$(EXTRADIR) $(DEPDIR) $(DEPDIR)/$(MUIDIR) $(DEPDIR)/$(REXXDIR) $(DEPDIR)/$(MIMEDIR) $(DEPDIR)/$(TCPDIR) $(DEPDIR)/$(EXTRADIR) $(MUIDIR)/Classes.crc $(TARGET)

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

$(OBJDIR)/$(TCPDIR): $(OBJDIR)
	@$(MKDIR) $(OBJDIR)/$(TCPDIR)

$(OBJDIR)/$(EXTRADIR): $(OBJDIR)
	@$(MKDIR) $(OBJDIR)/$(EXTRADIR)

# make the dependencies directories
$(DEPDIR):
	@echo "  MK $@"
	@$(MKDIR) $(DEPDIR)

$(DEPDIR)/$(MUIDIR): $(DEPDIR)
	@$(MKDIR) $(DEPDIR)/$(MUIDIR)

$(DEPDIR)/$(REXXDIR): $(DEPDIR)
	@$(MKDIR) $(DEPDIR)/$(REXXDIR)

$(DEPDIR)/$(MIMEDIR): $(DEPDIR)
	@$(MKDIR) $(DEPDIR)/$(MIMEDIR)

$(DEPDIR)/$(TCPDIR): $(DEPDIR)
	@$(MKDIR) $(DEPDIR)/$(TCPDIR)

$(DEPDIR)/$(EXTRADIR): $(DEPDIR)
	@$(MKDIR) $(DEPDIR)/$(EXTRADIR)

# for compiling single .c files
$(OBJDIR)/%.o: %.c
	@echo "  CC $<"
ifeq ($(GCCVER), 2)
	@$(CC) -MM -MG -o $(DEPDIR)/$(subst .c,.d,$<) $(CFLAGS) $<
	@$(SED) -i 's,^.*\.o:,$@:,g' $(DEPDIR)/$(subst .c,.d,$<)
else
	@$(CC) -MM -MP -MT '$@' -MF $(DEPDIR)/$(subst .c,.d,$<) $(CFLAGS) $<
endif
	@$(CC) $(CFLAGS) -c $< -o $@

# for making .c file out of .l lexer files
.SECONDARY: ParseEmail.c HTML2Mail.c
%.c: %.l
	@echo "  LX $<"
	@$(FLEX) -Cfe -i -o$@ $<

# for linking the target
$(TARGET): $(addprefix $(OBJDIR)/,$(OBJS)) $(LIB)/libtz.a
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
	@echo "  CLEAN"
	@$(RM) $(TARGET) $(TARGET).debug $(TARGET).map $(addprefix $(OBJDIR)/,$(OBJS)) gitrev.h

# clean all including .obj directory
.PHONY: cleanall
cleanall: clean
	@echo "  CLEANALL"
	@$(RM) Locale.? ParseEmail.c HTML2Mail.c $(MUIDIR)/Classes.* $(MUIDIR)/*_cl.h $(MUIDIR)/*.crc
	@$(RM) $(LOCALE)/*.catalog
	@$(RMDIR) $(OBJDIR)
	@$(RMDIR) $(DEPDIR)

# clean all stuff, including our autotools
.PHONY: distclean
distclean: cleanall
	@echo "  DISTCLEAN"
	@$(MAKE) -C $(GENCLASSES) clean
	@$(MAKE) -C $(TZLIB) -f Makefile.amiga clean

## ADD FLAGS TARGETS ##################

$(OBJDIR)/YAM_global.o: gitrev.h
$(OBJDIR)/YAM_global.o: CFLAGS += $(YAMVERID)

$(OBJDIR)/ParseEmail.o: CFLAGS += -Wno-sign-compare

$(OBJDIR)/HTML2Mail.o: CFLAGS += -Wno-sign-compare

## GENCLASSES TOOL ####################

$(MUIDIR)/Classes.h: $(MUIDIR)/Classes.crc
$(MUIDIR)/Classes.c: $(MUIDIR)/Classes.crc

$(MUIDIR)/Classes.crc: $(GENCLASSES)/GenClasses $(MUIDIR)/ClassesExtra.h Locale.h $(filter-out $(MUIDIR)/Classes.c, $(addprefix $(MUIDIR)/, $(subst .o,.c, $(MUIOBJS))))
	@echo "  GN GenClasses"
	@$(GENCLASSES)/GenClasses mui -bYAM -gpl -storm -iClassesExtra.h,ClassesSetup.h -qdl

$(GENCLASSES)/GenClasses: $(GENCLASSES)/gc.c $(GENCLASSES)/gc.h \
	$(GENCLASSES)/lists.c $(GENCLASSES)/lists.h \
	$(GENCLASSES)/crc32.c $(GENCLASSES)/crc32.h
	@$(MAKE) -C $(GENCLASSES)

## TZLIB TOOL #########################

$(LIB)/libtz.a: $(TZLIB)/asctime.c $(TZLIB)/difftime.c \
	$(TZLIB)/localtime.c $(TZLIB)/private.h \
	$(TZLIB)/strftime.c $(TZLIB)/tzfile.h
	@$(MAKE) -C $(TZLIB) -f Makefile.amiga

## GITREV TOOL ########################

gitrev.h: $(TOOLS)/gitrev.sh
	@echo "  GN $@"
	@$(TOOLS)/gitrev.sh header >$@

## CATALOG GENERATION #################

$(LOCALE)/%.catalog: $(LOCALE)/%.po
	@echo "  FC $@"
	@$(FC) REVISION $(shell git rev-list --all --count $<) POFILE $< CATALOG $@ CODESET UTF-8

.IGNORE: $(CATALOGS)

catalogs: $(CATALOGS)

## FLEXCAT ############################

Locale.h: Locale.c
Locale.c: $(LOCALE)/YAM.pot Locale_h.sd Locale_c.sd
	@echo "  FC $@"
	@$(FC) $(LOCALE)/YAM.pot Locale.h=Locale_h.sd Locale.c=Locale_c.sd CODESET ISO-8859-1

## TRANSIFEX ##########################

.PHONY: txpull
txpull:
	@$(TX) pull -a -f

.PHONY: txpush
txpush:
	@$(TX) push -t

## TO CREATE RELEASE ARCHIVES #########

.PHONY: release
release: distclean
	@cd ..; scripts/mkdist.sh release os3
	@cd ..; scripts/mkdist.sh release os4
	@cd ..; scripts/mkdist.sh release mos
	#@cd ..; scripts/mkdist.sh release aros-i386
	#@cd ..; scripts/mkdist.sh release aros-ppc
	#@cd ..; scripts/mkdist.sh release aros-x86_64
	#@cd ..; scripts/mkdist.sh release aros-arm

## TO CREATE A NIGHTLY-BUILD ARCHIVE #######

.PHONY: nightly
nightly: distclean
	@cd ..; scripts/mkdist.sh nightly os3
	@cd ..; scripts/mkdist.sh nightly os4
	@cd ..; scripts/mkdist.sh nightly mos
	#@cd ..; scripts/mkdist.sh nightly aros-i386
	#@cd ..; scripts/mkdist.sh nightly aros-ppc
	#@cd ..; scripts/mkdist.sh nightly aros-x86_64
	#@cd ..; scripts/mkdist.sh nightly aros-arm

## TO CREATE A SNAPSHOT ARCHIVE #######

.PHONY: snapshot
snapshot: BUILDID = $(shell $(TOOLS)/gitrev.sh)
snapshot: distclean all
	@lha a YAM_$(OS)_$(shell $(TOOLS)/gitrev.sh).lha $(TARGET)*

## DEPENDENCY INCLUDE #################

-include $(YAMOBJS:%.o=$(DEPDIR)/%.d)
-include $(MUIOBJS:%.o=$(DEPDIR)/$(MUIDIR)/%.d)
-include $(REXXOBJS:%.o=$(DEPDIR)/$(REXXDIR)/%.d)
-include $(MIMEOBJS:%.o=$(DEPDIR)/$(MIMEDIR)/%.d)
-include $(TCPOBJS:%.o=$(DEPDIR)/$(TCPDIR)/%.d)
-include $(EXTRAOBJS:%.o=$(DEPDIR)/$(EXTRADIR)/%.d)
