#/***************************************************************************
#
# YAM - Yet Another Mailer
# Copyright (C) 1995-2000 Marcel Beck
# Copyright (C) 2000-2013 YAM Open Source Team
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
AR      = ar
STRIP   = strip
OBJDUMP = objdump

# path definitions
CDUP  = /
CDTHIS=

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
TARGET  = libtz.a
TDIR    = ../../lib/$(OS)

# Common Directories
PREFIX    = $(CDTHIS)
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
CFLAGS   = -I. -I./include $(CPU) $(WARN) $(OPTFLAGS) $(DEBUG) $(DEBUGSYM) -DTZDIR=\"PROGDIR:Resources/zoneinfo\" -Dtime_tz="int_fast64_t"
LDFLAGS  = $(CPU) $(DEBUGSYM)
LDLIBS   = -L$(LIB) -lm

# different options per target OS
ifeq ($(OS), os4)

  ##############################
  # AmigaOS4

  # Compiler/link/strip commands
  ifneq ($(HOST), AmigaOS4)
    CC      = ppc-amigaos-gcc
    AR      = ppc-amigaos-ar
    STRIP   = ppc-amigaos-strip
    OBJDUMP = ppc-amigaos-objdump
  endif

  # Compiler/Linker flags
  CRT      = newlib
  CPU      = -mcpu=powerpc -mstrict-align
  WARN     += -Wdeclaration-after-statement -Wdisabled-optimization -Wshadow
  CFLAGS   += -mcrt=$(CRT) -Wa,-mregnames -DHAVE_INTTYPES_H=1
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
    AR      = m68k-amigaos-ar
    STRIP   = m68k-amigaos-strip
    OBJDUMP = m68k-amigaos-objdump
  endif

  # Compiler/Linker flags
  CPU	    = -m68020-60 -msoft-float
  CFLAGS  += -I./include/netinclude -DNO_INLINE_STDARG -D__amigaos3__ -Wno-sign-compare -DHAVE_INTTYPES_H=1 -DHAVE_SYS_WAIT_H=0
  LDFLAGS += -noixemul
  LDLIBS  +=
  GCCVER  = 2

else
ifeq ($(OS), mos)

  ##############################
  # MorphOS

  # Compiler/link/strip commands
  ifneq ($(HOST), MorphOS)
    CC      = ppc-morphos-gcc
    AR      = ppc-morphos-ar
    STRIP   = ppc-morphos-strip
    OBJDUMP = ppc-morphos-objdump
  endif

  # Compiler/Linker flags
  CPU     = -mcpu=powerpc
  CFLAGS  += -noixemul -I./include/netinclude -DHAVE_INTTYPES_H=1
  LDFLAGS += -noixemul
  LDLIBS  +=
  GCCVER  = 2

else
ifeq ($(OS), aros-i386)

  ##############################
  # AROS (i386)

  ifneq ($(HOST), AROS)
    CC      = i386-aros-gcc
    AR      = i386-aros-ar
    STRIP   = i386-aros-strip
    OBJDUMP = i386-aros-objdump
  endif

  # Compiler/Linker flags
  OPTFLAGS = -O2 -fomit-frame-pointer
  CFLAGS += -Wno-pointer-sign -DNO_INLINE_STDARG -D__BSD_VISIBLE=1 -DINT_FAST32_MAX=2147483647L -DINTMAX_MAX=9223372036854775807LL -DUINTMAX_MAX=18446744073709551615ULL -DINT64_MAX=9223372036854775807LL -DINT_FAST32_MIN=-2147483647L-1 -DPRIdMAX=\"ld\" -DPRIuMAX=\"lu\"
  LDLIBS +=

else
ifeq ($(OS), aros-ppc)

  ##############################
  # AROS (PPC)

  ifneq ($(HOST), AROS)
    CC      = ppc-aros-gcc
    AR      = ppc-aros-ar
    STRIP   = ppc-aros-strip
    OBJDUMP = ppc-aros-objdump
  endif

  # Compiler/Linker flags
  OPTFLAGS = -O2 -fomit-frame-pointer
  CFLAGS += -Wno-pointer-sign -DNO_INLINE_STDARG -D__BSD_VISIBLE=1 -DINT_FAST32_MAX=2147483647L -DINTMAX_MAX=9223372036854775807LL -DUINTMAX_MAX=18446744073709551615ULL -DINT64_MAX=9223372036854775807LL -DINT_FAST32_MIN=-2147483647L-1 -DPRIdMAX=\"lld\" -DPRIuMAX=\"llu\"
  LDLIBS +=

else
ifeq ($(OS), aros-x86_64)

  ##############################
  # AROS (x86_64)

  ifneq ($(HOST), AROS)
    CC      = x86_64-aros-gcc
    AR      = x86_64-aros-ar
    STRIP   = x86_64-aros-strip
    OBJDUMP = x86_64-aros-objdump
  endif

  # Compiler/Linker flags
  OPTFLAGS = -O2 -fomit-frame-pointer
  CFLAGS += -Wno-pointer-sign -DNO_INLINE_STDARG -D__BSD_VISIBLE=1 -DINT_FAST32_MAX=2147483647L -DINTMAX_MAX=9223372036854775807LL -DUINTMAX_MAX=18446744073709551615ULL -DINT_FAST64_MAX=9223372036854775807LL -DINT64_MAX=9223372036854775807LL -DINT_FAST32_MIN=-2147483647L-1 -DPRIdMAX=\"ld\" -DPRIuMAX=\"lu\"
  LDLIBS +=

else
ifeq ($(OS), aros-arm)

  ##############################
  # AROS (x86_64)

  ifneq ($(HOST), AROS)
    CC      = arm-aros-gcc
    AR      = arm-aros-ar
    STRIP   = arm-aros-strip
    OBJDUMP = arm-aros-objdump
  endif

  # Compiler/Linker flags
  OPTFLAGS = -O2 -fomit-frame-pointer
  CFLAGS += -Wno-pointer-sign -DNO_INLINE_STDARG -D__BSD_VISIBLE=1 -DINT_FAST32_MAX=2147483647L -DINTMAX_MAX=9223372036854775807LL -DUINTMAX_MAX=18446744073709551615ULL -DINT64_MAX=9223372036854775807LL -DINT_FAST32_MIN=-2147483647L-1 -DPRIdMAX=\"lld\" -DPRIuMAX=\"llu\"
  LDLIBS +=

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

# and the other object files where
# YAM_global.o and YAM.o have to stay at the top
OBJS = \
  asctime.o \
  difftime.o \
  localtime.o \
  strftime.o

# main target
.PHONY: all
all: $(OBJDIR) $(TDIR)/$(TARGET)

# make the object directories
$(OBJDIR):
	@echo "  MK $@"
	@$(MKDIR) $(OBJDIR)

# for compiling single .c files
$(OBJDIR)/%.o: %.c
	@echo "  CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# for linking the target
$(TDIR)/$(TARGET): $(addprefix $(OBJDIR)/,$(OBJS))
	@echo "  AR $@"
	@$(AR) -cq $@ $(addprefix $(OBJDIR)/,$(OBJS))

# cleanup target
.PHONY: clean
clean:
	-$(RM) $(TDIR)/$(TARGET) $(addprefix $(OBJDIR)/,$(OBJS))
