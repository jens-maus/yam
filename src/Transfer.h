#ifndef TRANSFER_H
#define TRANSFER_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2010 by YAM Open Source Team

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

#include <stdio.h>

#include <intuition/classusr.h> // Object
#include <openssl/ssl.h>        // SSL_CTX, SSL

#include "YAM_utilities.h"      // struct TimeVal

#include "MailServers.h"

// forward declarations
struct Folder;
struct MailList;
struct MailServerNode;

// some general flags for the Read(Recv)/Write(Send) TCP functions
#define TCPF_NONE             (0)
#define TCPF_FLUSH            (1<<0)
#define TCPF_FLUSHONLY        (1<<1)
#define TCPF_FREEBUFFER       (1<<2)
#define hasTCP_FLUSH(v)       (isFlagSet((v), TCPF_FLUSH))
#define hasTCP_ONLYFLUSH(v)   (isFlagSet((v), TCPF_FLUSHONLY))
#define hasTCP_FREEBUFFER(v)  (isFlagSet((v), TCPF_FREEBUFFER))

enum TransferType
{
  TR_IMPORT = 0,   // import mails
  TR_EXPORT,       // export mails
  TR_GET_USER,     // get mails, user triggered
  TR_GET_AUTO,     // get mails, timer/ARexx triggered
  TR_SEND_USER,    // send mails, user triggered
  TR_SEND_AUTO     // send mails, timer/ARexx triggered
};

enum TransWinMode   { TWM_HIDE=0, TWM_AUTO, TWM_SHOW };
enum PreSelMode     { PSM_NEVER=0, PSM_LARGE, PSM_ALWAYS, PSM_ALWAYSLARGE };

#define TCP_NO_SOCKET -1

// structure for the transfer statistics
struct TransStat
{
  int   Msgs_Tot;
  int   Msgs_Done;
  int   Msgs_Curr;
  int   Msgs_ListPos;
  ULONG Size_Tot;
  ULONG Size_Done;
  ULONG Size_Curr;
  ULONG Size_Curr_Max;
  ULONG Clock_Start;
  struct TimeVal Clock_Last;
  char str_size_tot[SIZE_SMALL];
  char str_size_done[SIZE_SMALL];
  char str_size_curr[SIZE_SMALL];
  char str_size_curr_max[SIZE_SMALL];
  char str_speed[SIZE_SMALL];
};

#warning "TODO: replace usage of DownloadResult with TransStat"
struct DownloadResult
{
  LONG Downloaded;
  LONG OnServer;
  LONG DupSkipped;
  LONG Deleted;
  BOOL Error;
};

#define TS_SETMAX (-1)

// structure for managing TCP/IP transfers
struct TransferNode
{
  struct MinNode node;              // for putting it into the transferQueue in struct Global

  struct MinList mailTransferList;  // list for managing the downloads/uploads via MailTransferNode
  int socket;                       // socket ID/pointer - is -1 (TCP_NO_SOCKET) if not in use
  struct MailServerNode *msn;       // pointer to the mail server this transfer refers to
  ULONG smtpFlags;                  // during connection/transfer we deal with SMTP server flags
  BOOL useSSL;                      // TRUE if a secure connection/transfer should be performed
  int mode;                         // receive/send mode for USER/AUTO identification
  BOOL processFilters;              // do we have to process filters after transfer (e.g. remote/sent filters)?
  BOOL abort;                       // abort flag that signals that the transfer should end
  BOOL pause;                       // pause flag that signals that the transfer should be paused
  BOOL start;                       // start flag that signals that the transfer should start
  struct DownloadResult stats;      // for storing some statistics about the transfer
  BOOL duplicatesChecking;          // true if we check for duplicate mail downloads

  // for the SSL communication context
  SSL_CTX *ssl_ctx;
  SSL     *ssl;
};

// mail transfer structure
struct MailTransferNode
{
  struct MinNode node;            // required for placing it into "struct TR_ClassData"

  struct Mail         *mail;      // pointer to the corresponding mail
  struct TransferNode *tfn;       // pointer to the transfer node with info about server/download
  char                *UIDL;      // an unique identifier (UIDL) in case AvoidDuplicates is used
  unsigned char       tflags;     // transfer flags
  int                 position;   // current position of the mail in the GUI NList
  int                 index;      // the index value of the mail as told by a POP3 server
  long                importAddr; // the position (addr) within an export file to find the mail
};

// flags for the Transfer preselection stuff
#define TRF_NONE              (0)
#define TRF_LOAD              (1<<0)
#define TRF_DELETE            (1<<1)
#define TRF_PRESELECT         (1<<2)
#define hasTR_LOAD(v)         (isFlagSet((v)->tflags, TRF_LOAD))
#define hasTR_DELETE(v)       (isFlagSet((v)->tflags, TRF_DELETE))
#define hasTR_PRESELECT(v)    (isFlagSet((v)->tflags, TRF_PRESELECT))

// Socket Options a user can set in .config
// if a value was not specified by the user it is either -1 or
// FALSE for a boolean.
struct TRSocketOpt
{
  LONG SendBuffer;   // SO_SNDBUF
  LONG RecvBuffer;   // SO_RCVBUF
  LONG SendLowAt;    // SO_SNDLOWAT
  LONG RecvLowAt;    // SO_RCVLOWAT
  LONG SendTimeOut;  // SO_SNDTIMEO
  LONG RecvTimeOut;  // SO_RCVTIMEO
  BOOL KeepAlive;    // SO_KEEPALIVE
  BOOL NoDelay;      // TCP_NODELAY
  BOOL LowDelay;     // IPTOS_LOWDELAY
};

enum ImportFormat
{
  IMF_UNKNOWN = 0,
  IMF_MBOX,
  IMF_DBX,
  IMF_PLAIN
};

// general connection/transfer error enumeration values
enum ConnectError
{
  CONNECTERR_SUCCESS       = 0,
  CONNECTERR_NO_ERROR      = -1,
  CONNECTERR_UNKNOWN_ERROR = -2,
  CONNECTERR_SOCKET_IN_USE = -3,
  CONNECTERR_UNKNOWN_HOST  = -4,
  CONNECTERR_NO_SOCKET     = -5,
  CONNECTERR_NO_NONBLOCKIO = -6,
  CONNECTERR_TIMEDOUT      = -7,
  CONNECTERR_ABORTED       = -8,
  CONNECTERR_SSLFAILED     = -9,
  CONNECTERR_INVALID8BIT   = -10
};

extern struct Hook TR_ProcessIMPORTHook;
extern struct Hook TR_AbortIMPORTHook;

// public prototypes

struct TransferNode *CreateNewTransfer(struct MailServerNode *msn);
void ProcessTransferQueue(enum MailServerType mst);

BOOL TR_OpenTCPIP(void);
void TR_CloseTCPIP(void);
void TR_Cleanup(struct TransferNode *tfn);
BOOL TR_IsOnline(void);

BOOL TR_DownloadURL(const char *server, const char *request, const char *filename);

void TR_SetWinTitle(BOOL from, const char *text);
struct TR_ClassData *TR_New(enum TransferType TRmode);

void TR_TransStat_Init(struct TransferNode *tfn, struct TransStat *ts);
void TR_TransStat_Start(struct TransStat *ts);
void TR_TransStat_NextMsg(struct TransStat *ts, int index, int listpos, LONG size, const char *status);
void TR_TransStat_Update(struct TransStat *ts, int size_incr, const char *status);

int TR_Recv(struct TransferNode *tfn, char *recvdata, const int maxlen);
int TR_Send(struct TransferNode *tfn, const char *ptr, const int len, const int flags);
int TR_ReadLine(struct TransferNode *tfn, char *vptr, const int maxlen);
#define TR_WriteLine(tfn, buf)   (TR_Send((tfn), (buf), strlen(buf), TCPF_FLUSH))
#define TR_WriteFlush(tfn)       (TR_Send((tfn), NULL,  0, TCPF_FLUSHONLY))
#define TR_FreeTransBuffers(tfn) { TR_ReadBuffered(tfn, NULL, 0, TCPF_FREEBUFFER);  \
                                  TR_WriteBuffered(tfn, NULL, 0, TCPF_FREEBUFFER); }

enum ConnectError TR_Connect(struct TransferNode *tfn);
void TR_Disconnect(struct TransferNode *tfn);
BOOL TR_InitTLS(struct TransferNode *tfn);
BOOL TR_StartTLS(struct TransferNode *tfn);

BOOL SetupTransferWindow(enum TransferType type);
void CleanupTransferWindow(void);

#endif /* TRANSFER_H */
