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
 * Copyright (c) 1982, 1986, 1993
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
 *	@(#)ip_var.h	8.2 (Berkeley) 1/9/95
 */

#ifndef _NETINET_IP_VAR_H
#define _NETINET_IP_VAR_H

/****************************************************************************/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif /* EXEC_TYPES_H */

#ifndef _NETINET_IN_H
#include <netinet/in.h>
#endif /* _NETINET_IN_H */

#ifndef _NETINET_IN_VAR_H
#include <netinet/in_var.h>
#endif /* _NETINET_IN_VAR_H */

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
 * Overlay for ip header used by other protocols (tcp, udp).
 */
struct ipovly {
	APTR	ih_next, ih_prev;	/* for protocol sequence q's */
	UBYTE	ih_x1;			/* (unused) */
	UBYTE	ih_pr;			/* protocol */
	WORD	ih_len;			/* protocol length */
	struct	in_addr ih_src;		/* source internet address */
	struct	in_addr ih_dst;		/* destination internet address */
};

/*
 * Structure stored in mbuf in inpcb.ip_options
 * and passed to ip_output when ip options are in use.
 * The actual length of the options (including ipopt_dst)
 * is in m_len.
 */
#define MAX_IPOPTLEN	40

struct ipoption {
	struct	in_addr ipopt_dst;	/* first-hop dst if source routed */
	char	ipopt_list[MAX_IPOPTLEN];	/* options proper */
};

struct	ipstat {
	ULONG	ips_total;		/* total packets received */
	ULONG	ips_badsum;		/* checksum bad */
	ULONG	ips_tooshort;		/* packet too short */
	ULONG	ips_toosmall;		/* not enough data */
	ULONG	ips_badhlen;		/* ip header length < data size */
	ULONG	ips_badlen;		/* ip length < ip header length */
	ULONG	ips_fragments;		/* fragments received */
	ULONG	ips_fragdropped;	/* frags dropped (dups, out of space) */
	ULONG	ips_fragtimeout;	/* fragments timed out */
	ULONG	ips_forward;		/* packets forwarded */
	ULONG	ips_cantforward;	/* packets rcvd for unreachable dest */
	ULONG	ips_redirectsent;	/* packets forwarded on same net */
	ULONG	ips_noproto;		/* unknown or unsupported protocol */
	ULONG	ips_delivered;		/* datagrams delivered to upper level*/
	ULONG	ips_localout;		/* total ip packets generated here */
	ULONG	ips_odropped;		/* lost packets due to nobufs, etc. */
	ULONG	ips_reassembled;	/* total packets reassembled ok */
	ULONG	ips_fragmented;		/* datagrams sucessfully fragmented */
	ULONG	ips_ofragments;		/* output fragments created */
	ULONG	ips_cantfrag;		/* don't fragment flag was set, etc. */
	ULONG	ips_badoptions;		/* error in option processing */
	ULONG	ips_noroute;		/* packets discarded due to no route */
	ULONG	ips_badvers;		/* ip version != 4 */
	ULONG	ips_rawout;		/* total raw ip packets generated */
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

#endif /* _NETINET_IP_VAR_H */
