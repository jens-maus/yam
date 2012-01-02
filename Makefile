#/***************************************************************************
#
# YAM - Yet Another Mailer
# Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
# Copyright (C) 2000-2012 YAM Open Source Team
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

.PHONY: all
all: src

.PHONY: src
src:
	@$(MAKE) -C src

.PHONY: catalogs
catalogs:
	@$(MAKE) -C src catalogs

.PHONY: clean
clean:
	@$(MAKE) -C src clean

.PHONY: cleanall
cleanall:
	@$(MAKE) -C src cleanall

.PHONY: distclean
distclean:
	@$(MAKE) -C src distclean

.PHONY: release
release:
	@$(MAKE) -C src release
