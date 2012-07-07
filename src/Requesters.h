#ifndef REQUESTERS_H
#define REQUESTERS_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2012 YAM Open Source Team

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :  http://www.yam.ch
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

***************************************************************************/

// forward declarations
struct Connection;
struct Certificate;

// attachment requester flags & macros
#define ATTREQ_DISP       (1<<0)
#define ATTREQ_SAVE       (1<<1)
#define ATTREQ_PRINT      (1<<2)
#define ATTREQ_MULTI      (1<<3)
#define isDisplayReq(v)   (isFlagSet((v), ATTREQ_DISP))
#define isSaveReq(v)      (isFlagSet((v), ATTREQ_SAVE))
#define isPrintReq(v)     (isFlagSet((v), ATTREQ_PRINT))
#define isMultiReq(v)     (isFlagSet((v), ATTREQ_MULTI))

// MUI_Request() flags
#define MUIF_REQ_FLOATTEXT (1<<0)

LONG YAMMUIRequest(Object *app, Object *win, LONG flags, const char *title, const char *gadgets, const char *format, ...);
LONG YAMMUIRequestA(Object *app, Object *parent, LONG flags, const char *tit, const char *gad, const char *reqtxt);
int StringRequest(char *string, int size, const char *title, const char *body, const char *yestext, const char *alttext, const char *notext, BOOL secret, Object *parent);
int PassphraseRequest(char *string, int size, Object *parent);
struct Folder *FolderRequest(const char *title, const char *body, const char *yestext, const char *notext, struct Folder *exclude, Object *parent);
struct Part *AttachRequest(const char *title, const char *body, const char *yestext, const char *notext, int mode, struct ReadMailData *rmData);
LONG CheckboxRequest(Object *parent, const char *tit, ULONG numBoxes, const char *text, ...);
BOOL CertWarningRequest(struct Connection *conn, struct Certificate *cert);

#endif /* REQUESTERS_H */
