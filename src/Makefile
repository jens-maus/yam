#/***************************************************************************
#
# YAM - Yet Another Mailer
# Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
# Copyright (C) 2000-2007 by YAM Open Source Team
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
ifndef (HOST)
  HOST = $(shell uname)
  ifeq ($(HOST), AmigaOS)
    ifeq ($(shell uname -m), powerpc)
      HOST = AmigaOS4
    endif
    ifeq ($(shell uname -m), ppc)
      HOST = AmigaOS4
    endif
  endif
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
  ifeq ($(HOST), Aros)
    OS = aros
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
FLEX  = flex
FC    = flexcat
EXPR  = expr
CHMOD = chmod
SED   = sed
DATE  = date
RM    = rm -f
RMDIR = rm -rf
MKDIR = mkdir

# Special macros for version ID and developer builds
DEVFLAGS = -DDEVWARNING -DEXPDATE="`$(EXPR) \`$(DATE) +%s\` / 86400 - 2922 + 31`"
YAMVERID = -D__YAM_VERDATE=\"`$(DATE) +%d.%m.%Y`\" \
           -D__YAM_VERDAYS="`$(EXPR) \`$(DATE) +%s\` / 86400 - 2922`" \
           -D__YAM_BUILDID="$(BUILDID)"

# override commands for native builds
ifeq ($(HOST), AmigaOS4)
  # AmigaOS4 host
  RM    = delete force
  RMDIR = delete force all
  MKDIR = makedir
  DATE  = gdate
else
ifeq ($(HOST), AmigaOS)
  # AmigaOS3 host
  RM    = delete force
  RMDIR = delete force all
  MKDIR = makedir

  # override the versionID and developer build macros with calls
  # to ARexx instead of unix expr/date
  DEVFLAGS = -DDEVWARNING `rx "say '-DEXPDATE=' || date('I')+31"`
  YAMVERID = `rx "say '-D__YAM_VERDATE=' || '22'x ||\
              right(date(S),2)*1'.'substr(date(S),5,2)*1'.'left(date(S),4) ||\
              '22'x || ' -D__YAM_VERDAYS=' || date('I')"` -D__YAM_BUILDID="$(BUILDID)"

else
ifeq ($(HOST), MorphOS)
  # MorphOS host
  RM    = delete force
  RMDIR = delete force all
  MKDIR = makedir
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
PREFIX    = .
CLASSDIR  = $(PREFIX)/classes
LOCALE    = $(PREFIX)/../locale
EXTRASRC  = $(PREFIX)/extrasrc
TOOLS     = $(PREFIX)/tools
LIB       = $(PREFIX)/lib/$(OS)
OBJDIR    = .obj_$(OS)

# Common compiler/linker flags
WARN     = -W -Wall -Wwrite-strings
OPTFLAGS = -O3 -fomit-frame-pointer -funroll-loops
DEBUG    = -DDEBUG #-O0
DEBUGSYM = -gstabs
CFLAGS   = -I. -I./include $(CPU) $(WARN) $(OPTFLAGS) \
           $(DEBUG) $(DEBUGSYM) -c
LDFLAGS  = $(CPU) $(DEBUGSYM)
LDLIBS   = -L$(LIB) -lm

# different options per target OS
ifeq ($(OS), os4)

  ##############################
  # AmigaOS4

  # Compiler/link/strip commands
  CC      = ppc-amigaos-gcc
  STRIP   = ppc-amigaos-strip
  OBJDUMP = ppc-amigaos-objdump

  # Compiler/Linker flags
  CRT      = newlib
  CPU      = -mcpu=powerpc
  WARN     += -Wdeclaration-after-statement -Wdisabled-optimization
  REDEFINE = -DCoerceMethod=ICoerceMethod -DDoMethod=IDoMethod \
             -DDoSuperMethod=IDoSuperMethod -DDoSuperMethodA=IDoSuperMethodA
  CFLAGS   += -mcrt=$(CRT) -D__USE_INLINE__ -D__NEW_TIMEVAL_DEFINITION_USED__ \
              $(REDEFINE) -Wa,-mregnames
  LDFLAGS  += -mcrt=$(CRT)

else
ifeq ($(OS), os3)

  ##############################
  # AmigaOS3

  # Compiler/link/strip commands
  CC      = m68k-amigaos-gcc
  STRIP   = m68k-amigaos-strip
  OBJDUMP = m68k-amigaos-objdump

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
  CC      = ppc-morphos-gcc
  STRIP   = ppc-morphos-strip
  OBJDUMP = ppc-morphos-objdump

  # Compiler/Linker flags
  CPU     = -mcpu=powerpc
  CFLAGS  += -noixemul -pipe -I./include/netinclude
  LDFLAGS += -noixemul
  LDLIBS  += -ldebug -lxpk -lamissl -lcodesets

else
ifeq ($(OS), aros)

  ##############################
  # AROS

  # Compiler/link/strip commands
  CC      = i686-aros-gcc
  STRIP   = i686-aros-strip
  OBJDUMP = i686-aros-objdump

  # Compiler/Linker flags
  CPU     =

endif
endif
endif
endif

###########################################################################
# Here starts all stuff that is common for all target platforms and
# hosts.

# General vars
GENCLASSES= $(TOOLS)/genclasses

.PHONY: all clean cleanall distclean classinfo dump depend

# our few new rewritten classes
CLASSES	= \
	$(OBJDIR)/$(CLASSDIR)/Classes.o \
	$(OBJDIR)/$(CLASSDIR)/Recipientstring.o \
	$(OBJDIR)/$(CLASSDIR)/Searchwindow.o \
	$(OBJDIR)/$(CLASSDIR)/SearchControlGroup.o \
	$(OBJDIR)/$(CLASSDIR)/AddrBookEntryList.o \
	$(OBJDIR)/$(CLASSDIR)/AddrBookListtree.o \
	$(OBJDIR)/$(CLASSDIR)/AddrBookToolbar.o \
	$(OBJDIR)/$(CLASSDIR)/Addrmatchlist.o \
	$(OBJDIR)/$(CLASSDIR)/Aboutwindow.o \
	$(OBJDIR)/$(CLASSDIR)/AttachmentGroup.o \
	$(OBJDIR)/$(CLASSDIR)/AttachmentImage.o \
	$(OBJDIR)/$(CLASSDIR)/ConfigPageList.o \
	$(OBJDIR)/$(CLASSDIR)/ReadMailGroup.o \
	$(OBJDIR)/$(CLASSDIR)/ReadWindow.o \
	$(OBJDIR)/$(CLASSDIR)/ReadWindowToolbar.o \
	$(OBJDIR)/$(CLASSDIR)/ReadWindowStatusBar.o \
	$(OBJDIR)/$(CLASSDIR)/Splashwindow.o \
	$(OBJDIR)/$(CLASSDIR)/ShutdownWindow.o \
	$(OBJDIR)/$(CLASSDIR)/UserImage.o \
	$(OBJDIR)/$(CLASSDIR)/MailTextEdit.o \
	$(OBJDIR)/$(CLASSDIR)/MainFolderListtree.o \
	$(OBJDIR)/$(CLASSDIR)/MainMailList.o \
	$(OBJDIR)/$(CLASSDIR)/MainMailListGroup.o \
	$(OBJDIR)/$(CLASSDIR)/MainWindow.o \
	$(OBJDIR)/$(CLASSDIR)/MainWindowToolbar.o \
	$(OBJDIR)/$(CLASSDIR)/InfoBar.o \
	$(OBJDIR)/$(CLASSDIR)/WriteAttachmentList.o \
	$(OBJDIR)/$(CLASSDIR)/WriteWindowToolbar.o \
	$(OBJDIR)/$(CLASSDIR)/QuickSearchBar.o \
	$(OBJDIR)/$(CLASSDIR)/ImageArea.o \
	$(OBJDIR)/$(CLASSDIR)/TransferMailList.o \
	$(OBJDIR)/$(CLASSDIR)/UpdateNotifyWindow.o \
	$(OBJDIR)/$(CLASSDIR)/ThemeListGroup.o \
	$(OBJDIR)/$(CLASSDIR)/YAM.o

# and the other object files
OBJS = \
	$(OBJDIR)/YAM_global.o \
	$(OBJDIR)/YAM.o \
	$(CLASSES) \
	$(OBJDIR)/YAM_AB.o \
	$(OBJDIR)/YAM_CO.o \
	$(OBJDIR)/YAM_COg.o \
	$(OBJDIR)/YAM_COs.o \
	$(OBJDIR)/YAM_DI.o \
	$(OBJDIR)/YAM_EA.o \
	$(OBJDIR)/YAM_ER.o \
	$(OBJDIR)/YAM_FI.o \
	$(OBJDIR)/YAM_FO.o \
	$(OBJDIR)/YAM_locale.o \
	$(OBJDIR)/YAM_MA.o \
	$(OBJDIR)/YAM_MAf.o \
	$(OBJDIR)/YAM_md5.o \
	$(OBJDIR)/YAM_MI.o \
	$(OBJDIR)/YAM_mail_lex.o \
	$(OBJDIR)/YAM_RE.o \
	$(OBJDIR)/YAM_rexx.o \
	$(OBJDIR)/YAM_rexx_rxcl.o \
	$(OBJDIR)/YAM_rexx_rxif.o \
	$(OBJDIR)/YAM_TR.o \
	$(OBJDIR)/YAM_US.o \
	$(OBJDIR)/YAM_UT.o \
	$(OBJDIR)/YAM_WR.o \
	$(OBJDIR)/ImageCache.o \
	$(OBJDIR)/UpdateCheck.o \
	$(OBJDIR)/HTML2Mail.o \
	$(OBJDIR)/HashTable.o \
	$(OBJDIR)/BayesFilter.o \
	$(OBJDIR)/extrasrc.o \
	$(OBJDIR)/Debug.o

# available catalog translations
CATALOGS = \
  $(LOCALE)/deutsch.catalog \
  $(LOCALE)/bosanski.catalog \
  $(LOCALE)/czech.catalog \
  $(LOCALE)/dansk.catalog \
  $(LOCALE)/greek.catalog \
  $(LOCALE)/italiano.catalog \
  $(LOCALE)/magyar.catalog \
  $(LOCALE)/norsk.catalog \
  $(LOCALE)/polski.catalog \
  $(LOCALE)/russian.catalog \
  $(LOCALE)/srpski.catalog \
  $(LOCALE)/suomi.catalog \
  $(LOCALE)/svenska.catalog

#
all: $(OBJDIR) classinfo YAM_locale.c YAM_mail_lex.c HTML2Mail.c $(TARGET)

# make the object directories
$(OBJDIR):
	@printf '\033[33mGenerating $@ directory\033[0m\n'
	@$(MKDIR) $(OBJDIR)
	@$(MKDIR) $(OBJDIR)/$(CLASSDIR)

#

$(OBJDIR)/%.o: %.c
	@printf '\033[32mCompiling $<\033[0m\n'
	@$(CC) $(CFLAGS) $< -o $@

%.c: %.l
	@printf '\033[33mGenerating lexer $<\033[0m\n'
	@$(FLEX) -Cfe -i -o$@ $<

#

$(TARGET): $(OBJS)
	@printf '\033[32mLinking \033[1m$@.debug\033[0m\n'
	@$(CC) $(LDFLAGS) -o $@.debug $(OBJS) $(LDLIBS) -Wl,-Map,$@.map
	@printf '\033[32mGenerating \033[1m$@\033[0m\n'
	@$(STRIP) --preserve-dates --remove-section=".comment" --remove-section=".sdata2" --strip-debug -o $@ $@.debug
	@$(CHMOD) a+x $@

dump:
	-$(OBJDUMP) --section-headers --all-headers --reloc --disassemble-all $(TARGET).debug > $(TARGET).dump

clean:
	-$(RM) $(TARGET) $(TARGET).debug $(TARGET).map $(OBJS)

cleanall: clean
	-$(RM) YAM_locale.? YAM_mail_lex.c HTML2Mail.c $(CLASSDIR)/Classes.? $(CLASSDIR)/*_cl.h
	-$(RMDIR) $(OBJDIR)

distclean: cleanall
	@(cd $(GENCLASSES) && $(MAKE) clean)

## SPECIAL COMPILE RULES ##############

$(OBJDIR)/YAM_global.o: YAM_global.c
	@printf '\033[32mCompiling $<\033[0m\n'
	@$(CC) $(YAMVERID) $(CFLAGS) $< -o $@

$(OBJDIR)/YAM.o: YAM.c
	@printf '\033[32mCompiling $<\033[0m\n'
	@$(CC) $(DEVFLAGS) $(CFLAGS) $< -o $@

$(OBJDIR)/YAM_mail_lex.o: YAM_mail_lex.c
	@printf '\033[32mCompiling $<\033[0m\n'
	@$(CC) $(CFLAGS) -Wno-sign-compare $< -o $@

$(OBJDIR)/HTML2Mail.o: HTML2Mail.c
	@printf '\033[32mCompiling $<\033[0m\n'
	@$(CC) $(CFLAGS) -Wno-sign-compare $< -o $@

## DEPENDENCY GENERATION ##############

Makefile.depend: ;
	@printf "\033[33mWARNING: Makefile.depend missing. Please run 'make depend'\033[0m\n"

depend: classinfo YAM_locale.c YAM_mail_lex.c HTML2Mail.c
	@printf '\033[33mGenerating Makefile.depend\033[0m\n'
	@echo '# AUTOGENERATED! DO NOT EDIT!!!' >Makefile.depend
	@$(CC) $(CFLAGS) $(OBJS:$(OBJDIR)/%.o=./%.c) -MM >>Makefile.depend
	@$(SED) -i 's,^\(.*\)\.o:,$$\(OBJDIR\)/\1.o:,g' Makefile.depend
	@echo '# AUTOGENERATED! DO NOT EDIT!!!' >>Makefile.depend

## GENCLASSES TOOL ####################

classinfo: $(CLASSDIR)/Classes.h

$(CLASSDIR)/Classes.h: $(GENCLASSES)/GenClasses $(CLASSDIR)/*.c
	@printf '\033[33mGenerating class files...\033[0m\n'
	@$(GENCLASSES)/GenClasses classes -bYAM -gpl -storm -iClassesExtra.h

$(CLASSDIR)/Classes.c: $(CLASSDIR)/ClassesExtra.h

$(GENCLASSES)/GenClasses: $(GENCLASSES)/gc.c $(GENCLASSES)/gc.h \
	$(GENCLASSES)/lists.c $(GENCLASSES)/lists.h
	@(cd $(GENCLASSES) && $(MAKE))

## CATALOG GENERATION #################

$(LOCALE)/%.catalog: $(LOCALE)/%.ct
	@printf '\033[33mGenerating $@\033[0m\n'
	@$(FC) $(LOCALE)/YAM.cd $< CATALOG $@

.IGNORE: $(CATALOGS)

catalogs: $(CATALOGS)

## FLEXCAT ############################

YAM_locale.h: YAM_locale.c
YAM_locale.c: $(LOCALE)/YAM.cd C_h.sd C_c.sd
	@printf '\033[33mGenerating locale file $@...\033[0m\n'
	@$(FC) $(LOCALE)/YAM.cd YAM_locale.h=C_h.sd YAM_locale.c=C_c.sd

## DEPENDENCY INCLUDE #################

-include Makefile.depend
