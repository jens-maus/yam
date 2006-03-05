/*
 * $Id$
 *
 * :ts=8
 *
 * 'Roadshow' -- Amiga TCP/IP stack
 * Copyright © 2001-2004 by Olaf Barthel.
 * All Rights Reserved.
 *
 * Amiga specific TCP/IP 'C' header files;
 * Freely Distributable
 */

/*
 * Copyright (c) 1982, 1986, 1993, 1994, 1995
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)tcp_var.h	8.4 (Berkeley) 5/24/95
 */

#ifndef _NETINET_TCP_VAR_H
#define _NETINET_TCP_VAR_H

/****************************************************************************/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif /* EXEC_TYPES_H */

#ifndef _NETINET_TCP_TIMER_H
#include <netinet/tcp_timer.h>
#endif /* _NETINET_TCP_TIMER_H */

#ifndef _NETINET_TCP_H
#include <netinet/tcp.h>
#endif /* _NETINET_TCP_H */

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

#ifdef __GNUC__
 #ifdef __PPC__
  #pragma pack(2)
 #endif
#elif defined(__VBCC__)
 #pragma amiga-align
#endif

/****************************************************************************/

/*
 * TCP statistics.
 * Many of these should be kept per connection,
 * but that's inconvenient at the moment.
 */
struct	tcpstat {
	ULONG	tcps_connattempt;	/* connections initiated */
	ULONG	tcps_accepts;		/* connections accepted */
	ULONG	tcps_connects;		/* connections established */
	ULONG	tcps_drops;		/* connections dropped */
	ULONG	tcps_conndrops;		/* embryonic connections dropped */
	ULONG	tcps_closed;		/* conn. closed (includes drops) */
	ULONG	tcps_segstimed;		/* segs where we tried to get rtt */
	ULONG	tcps_rttupdated;	/* times we succeeded */
	ULONG	tcps_delack;		/* delayed acks sent */
	ULONG	tcps_timeoutdrop;	/* conn. dropped in rxmt timeout */
	ULONG	tcps_rexmttimeo;	/* retransmit timeouts */
	ULONG	tcps_persisttimeo;	/* persist timeouts */
	ULONG	tcps_keeptimeo;		/* keepalive timeouts */
	ULONG	tcps_keepprobe;		/* keepalive probes sent */
	ULONG	tcps_keepdrops;		/* connections dropped in keepalive */

	ULONG	tcps_sndtotal;		/* total packets sent */
	ULONG	tcps_sndpack;		/* data packets sent */
	ULONG	tcps_sndbyte;		/* data bytes sent */
	ULONG	tcps_sndrexmitpack;	/* data packets retransmitted */
	ULONG	tcps_sndrexmitbyte;	/* data bytes retransmitted */
	ULONG	tcps_sndacks;		/* ack-only packets sent */
	ULONG	tcps_sndprobe;		/* window probes sent */
	ULONG	tcps_sndurg;		/* packets sent with URG only */
	ULONG	tcps_sndwinup;		/* window update-only packets sent */
	ULONG	tcps_sndctrl;		/* control (SYN|FIN|RST) packets sent */

	ULONG	tcps_rcvtotal;		/* total packets received */
	ULONG	tcps_rcvpack;		/* packets received in sequence */
	ULONG	tcps_rcvbyte;		/* bytes received in sequence */
	ULONG	tcps_rcvbadsum;		/* packets received with ccksum errs */
	ULONG	tcps_rcvbadoff;		/* packets received with bad offset */
	ULONG	tcps_rcvshort;		/* packets received too short */
	ULONG	tcps_rcvduppack;	/* duplicate-only packets received */
	ULONG	tcps_rcvdupbyte;	/* duplicate-only bytes received */
	ULONG	tcps_rcvpartduppack;	/* packets with some duplicate data */
	ULONG	tcps_rcvpartdupbyte;	/* dup. bytes in part-dup. packets */
	ULONG	tcps_rcvoopack;		/* out-of-order packets received */
	ULONG	tcps_rcvoobyte;		/* out-of-order bytes received */
	ULONG	tcps_rcvpackafterwin;	/* packets with data after window */
	ULONG	tcps_rcvbyteafterwin;	/* bytes rcvd after window */
	ULONG	tcps_rcvafterclose;	/* packets rcvd after "close" */
	ULONG	tcps_rcvwinprobe;	/* rcvd window probe packets */
	ULONG	tcps_rcvdupack;		/* rcvd duplicate acks */
	ULONG	tcps_rcvacktoomuch;	/* rcvd acks for unsent data */
	ULONG	tcps_rcvackpack;	/* rcvd ack packets */
	ULONG	tcps_rcvackbyte;	/* bytes acked by rcvd acks */
	ULONG	tcps_rcvwinupd;		/* rcvd window update packets */
	ULONG	tcps_pawsdrop;		/* segments dropped due to PAWS */
	ULONG	tcps_predack;		/* times hdr predict ok for acks */
	ULONG	tcps_preddat;		/* times hdr predict ok for data pkts */
	ULONG	tcps_pcbcachemiss;
	ULONG	tcps_persistdrop;	/* timeout in persist state */
	ULONG	tcps_badsyn;		/* bogus SYN, e.g. premature ACK */
};

/****************************************************************************/

#ifdef __GNUC__
 #ifdef __PPC__
  #pragma pack()
 #endif
#elif defined(__VBCC__)
 #pragma default-align
#endif

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _NETINET_TCP_VAR_H */
