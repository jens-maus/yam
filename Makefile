#/***************************************************************************
#
# YAM - Yet Another Mailer
# Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
# Copyright (C) 2000-2017 YAM Open Source Team
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
# YAM Official Support Site :  http://yam.ch/
# YAM OpenSource project    :  https://github.com/jens-maus/yam/
#
#***************************************************************************/

# List targets defined in this file
TARGETS_SELF :=

# Exclude targets defined in this file
TARGETS_OTHER := $(filter-out $(TARGETS_SELF), $(MAKECMDGOALS))

# Call all targets using `Makefile` in src directory in one `make` command. It
# can depend on targets defined in this file, e.g., depending on a target to
# create the Makefile.
#
# If no targets are specified, use the dummy `all` target
$(or $(lastword $(TARGETS_OTHER)),all):
	@$(MAKE) -C src $(TARGETS_OTHER)
.PHONY: $(TARGETS_OTHER) all

# Do nothing for all targets but last. Also quiet the message "Noting to be done on xxx"
$(filter-out $(lastword $(TARGETS_OTHER)), $(TARGETS_OTHER)):
	@cd .
