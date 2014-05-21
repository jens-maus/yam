#ifndef SSL_H
#define SSL_H 1

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2014 YAM Open Source Team

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

// SSL certificate verification failures
#define SSL_CERT_ERR_NONE         (0<<0) // no error
#define SSL_CERT_ERR_NOTYETVALID  (1<<0) // the certificate is not yet valid
#define SSL_CERT_ERR_EXPIRED      (1<<1) // the certificate has expired
#define SSL_CERT_ERR_IDMISMATCH   (1<<2) // the hostname does not match hostname of server
#define SSL_CERT_ERR_UNTRUSTED    (1<<3) // the certificate authority which signed the cert is not trusted
#define SSL_CERT_ERR_BADCHAIN     (1<<4) // the certificate chain contained a cert which failed trust
#define SSL_CERT_ERR_SIGINVALID   (1<<5) // the signature of the certificate is invalid
#define SSL_CERT_ERR_OTHER        (1<<6) // other certificate error not specified here
#define SSL_CERT_ERR_UNHANDLED    (1<<7) // unhandled error occurred during cert verification

#define SSL_DIGESTLEN 60

// certificate structure
struct Certificate
{
  struct Certificate *issuer; // links to the certificate of the issuer or NULL if top level

  X509_NAME *subject_dn;
  X509_NAME *issuer_dn;
  X509      *subject;
  char      *identity;
  char      fingerprint[SSL_DIGESTLEN];
  char      *issuerStr;
  char      notBefore[SIZE_DEFAULT];
  char      notAfter[SIZE_DEFAULT];
};

// public functions
BOOL MakeSecureConnection(struct Connection *conn);

#endif /* SSL_H */
