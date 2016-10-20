/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2016 YAM Open Source Team

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

#include <string.h>
#include <ctype.h>

#include <clib/alib_protos.h>

#include <proto/amissl.h>
#include <proto/amisslmaster.h>

// we include bsdsocket.h but make sure to not let
// it define a global SocketBase or ISocket so that
// our socket stuff is assured to not use a global socket
// library base
#define __NOLIBBASE__
#define __NOGLOBALIFACE__
#include <proto/bsdsocket.h>
#undef __NOGLOBALIFACE__
#undef __NOLIBBASE__

#if defined(__amigaos3__)
#include <inline/extended_macros.h>
#elif defined(__MORPHOS__)
#include <ppcinline/extended_macros.h>
#endif

#include <proto/exec.h>

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_error.h"
#include "YAM_utilities.h"

#include "tcp/Connection.h"
#include "tcp/ssl.h"

#include "Config.h"
#include "DynamicString.h"
#include "FileInfo.h"
#include "Locale.h"
#include "MailServers.h"
#include "Requesters.h"

#include "Debug.h"

// SocketBase must not be shared between tasks, hence we must not use the global
// SocketBase, but the one tied to the current connection.
#if defined(__amigaos4__)
#define GET_SOCKETBASE(conn)  struct SocketIFace *ISocket = (conn)->socketIFace
#else
#define GET_SOCKETBASE(conn)  struct Library *SocketBase = (conn)->socketBase
#endif

#define DEFAULT_CAPATH "PROGDIR:Resources/certificates"
#define DEFAULT_CAFILE "PROGDIR:Resources/certificates/ca-bundle.crt"

/// verify_callback
// callback function that is called by AmiSSL/OpenSSL for every certification
// verification step
CROSSCALL2(verify_callback, int, int, preverify_ok, X509_STORE_CTX *, x509_ctx)
{
  ENTER();

  D(DBF_NET, "verify_callback() called");
  SHOWVALUE(DBF_NET, preverify_ok);

  // if there is no error we do nothing but return
  if(preverify_ok == 0)
  {
    int sslIndex = SSL_get_ex_data_X509_STORE_CTX_idx();
    SSL *ssl = NULL;

    SHOWVALUE(DBF_NET, x509_ctx);

    // get SSL connection pointer
    if(sslIndex >= 0 && (ssl = X509_STORE_CTX_get_ex_data(x509_ctx, sslIndex)) != NULL)
    {
      struct Connection *conn;

      // get our (struct Connection) structure out of the app data
      if((conn = SSL_get_ex_data(ssl, G->sslDataIndex)) != NULL)
      {
        // get some information on the potential error
        int err = X509_STORE_CTX_get_error(x509_ctx);
        int depth = X509_STORE_CTX_get_error_depth(x509_ctx);
        int failures = 0;

        W(DBF_NET, "ssl: verify callback @ %ld => %ld:'%s'", depth, err, X509_verify_cert_error_string(err));

        // map the specific X509 error codes to general failures
        // we cna query later on when checking the certificate chain later again
        switch(err)
        {
          // (0) the operation was successful.
          case X509_V_OK:
            // nothing
          break;

          // (2) unable to get issuer certificate
          // (18) self signed certificate
          // (19) self signed certificate in certificate chain
          // (20) unable to get local issuer certificate
          // (21) unable to verify the first certificate
          // (23) the certificate has been revoked.
          // (27) certificate not trusted
          // (28) the root CA is marked to reject the specified purpose.
          case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
          case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
          case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
          case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
          case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
          case X509_V_ERR_CERT_REVOKED:
          case X509_V_ERR_CERT_UNTRUSTED:
          case X509_V_ERR_CERT_REJECTED:
          {
            W(DBF_NET, "ssl: verify failure SSL_CERT_ERR_UNTRUSTED found");
            setFlag(failures, SSL_CERT_ERR_UNTRUSTED);
          }
          break;

          // (3) the CRL of a certificate could not be found.
          // (4) the certificate signature could not be decrypted.
          // (5) the CRL signature could not be decrypted.
          // (6) the public key in the certificate SubjectPublicKeyInfo could not be read.
          // (11) the CRL is not yet valid.
          // (12) the CRL has expired.
          // (15) the CRL lastUpdate field contains an invalid time.
          // (16) the CRL nextUpdate field contains an invalid time.
          // (22) the certificate chain length is greater than the supplied maximum depth. Unused.
          // (24) a CA certificate is invalid. Either it is not a CA or its extensions are not consistent with the supplied purpose.
          // (25) the basicConstraints pathlength parameter has been exceeded.
          // (26) the supplied certificate cannot be used for the specified purpose.
          // (29) subject issuer mismatch: the current candidate issuer certificate was rejected.
          // (30) authority and subject key identifier mismatch: the current candidate issuer certificate was rejected.
          // (31) authority and issuer serial number mismatch: the current candidate issuer certificate was rejected.
          // (32) key usage does not include certificate signing:  the current candidate issuer certificate was rejected.
          case X509_V_ERR_UNABLE_TO_GET_CRL:
          case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
          case X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
          case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
          case X509_V_ERR_CRL_NOT_YET_VALID:
          case X509_V_ERR_CRL_HAS_EXPIRED:
          case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
          case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
          case X509_V_ERR_CERT_CHAIN_TOO_LONG:
          case X509_V_ERR_INVALID_CA:
          case X509_V_ERR_PATH_LENGTH_EXCEEDED:
          case X509_V_ERR_INVALID_PURPOSE:
          case X509_V_ERR_SUBJECT_ISSUER_MISMATCH:
          case X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH:
          case X509_V_ERR_KEYUSAGE_NO_CERTSIGN:
          {
            W(DBF_NET, "ssl: other uncritical certificate problem. Signaling SSL_CERT_ERR_OTHER.");
            setFlag(failures, SSL_CERT_ERR_OTHER);
          }
          break;

          // (7) the signature of the certificate is invalid.
          // (8) the signature of the certificate is invalid.
          case X509_V_ERR_CERT_SIGNATURE_FAILURE:
          case X509_V_ERR_CRL_SIGNATURE_FAILURE:
          {
            W(DBF_NET, "ssl: cert signature failed, signaling SSL_CERT_ERR_SIGINVALID");
            setFlag(failures, SSL_CERT_ERR_SIGINVALID);
          }
          break;

          // (9) the certificate is not yet valid: the notBefore date is after the current time.
          // (13) the certificate notBefore field contains an invalid time.
          case X509_V_ERR_CERT_NOT_YET_VALID:
          case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
          {
            W(DBF_NET, "ssl: verify failure %s found", depth > 0 ? "SSL_CERT_ERR_BADCHAIN" : "SSL_CERT_ERR_NOTYETVALID");
            setFlag(failures, depth > 0 ? SSL_CERT_ERR_BADCHAIN : SSL_CERT_ERR_NOTYETVALID);
          }
          break;

          // (10) the certificate has expired: that is the notAfter date is before the current time.
          // (14) the certificate notAfter field contains an invalid time.
          case X509_V_ERR_CERT_HAS_EXPIRED:
          case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
          {
            W(DBF_NET, "ssl: verify failure %s found", depth > 0 ? "SSL_CERT_ERR_BADCHAIN" : "SSL_CERT_ERR_EXPIRED");
            setFlag(failures, depth > 0 ? SSL_CERT_ERR_BADCHAIN : SSL_CERT_ERR_EXPIRED);
          }
          break;

          // (17) X509_V_ERR_OUT_OF_MEM: an error occurred trying to allocate memory. This should never happen.
          // (50) X509_V_ERR_APPLICATION_VERIFICATION: an application specific error. Unused.
          default:
          {
            // clear the failures bitmask so check_certificates() knows this
            // is a bailout
            setFlag(conn->sslCertFailures, SSL_CERT_ERR_UNHANDLED);
            W(DBF_NET, "ssl: Unhandled certification verification error: SSL_CERT_ERR_UNHANDLED");

            // make sure we return an error and stop the
            // cert verification right away
            RETURN(0);
            return 0;
          }
        }

        // set the connection-wise sslCertFailure bitmask
        setFlag(conn->sslCertFailures, failures);
        D(DBF_NET, "ssl: verify failures |= %ld => %ld", failures, conn->sslCertFailures);

        // make sure we return 1 (success) so that
        // the verification process continues and collects more failures
        preverify_ok = 1;
      }
      else
        E(DBF_NET, "SSL_get_ex_data() returned invalid 'conn'");
    }
    else
      E(DBF_NET, "X509_STORE_CTX_get_ex_data() returned invalid SSL structure");
  }

  RETURN(preverify_ok);
  return preverify_ok;
}

///
/// MatchHostname
// This doesn't actually implement complete RFC 2818 logic; omits
// "f*.example.com" support for simplicity.
static int MatchHostname(const char *cn, size_t cnlen, const char *hostname)
{
  const char *dot;
  int result;

  ENTER();

  D(DBF_NET, "ssl: Match common name '%s' against '%s'", cn, hostname);

  // check for the commonName starting with a wildcard (*.) so that we
  // start matching against it starting after the wildcard
  if(strncmp(cn, "*.", 2) == 0 && cnlen > 2 &&
     (dot = strchr(hostname, '.')) != NULL)
  {
    int i;
    BOOL ipFound = TRUE;

    // Prevent wildcard CN matches against anything which can be
    // parsed as an IP address (i.e. a CN of "*.1.1.1" should not
    // be match 8.1.1.1).  draft-saintandre-tls-server-id-check
    // will require some more significant changes to cert ID
    // verification which will probably obviate this check, but
    // this is a desirable policy tightening in the mean time.

    // check that hostname is not only a pure IP address by scanning it
    // and searching for ?." and numbers only
    for(i = 0; hostname[i] != '\0'; i++)
    {
      // check for IPv4 and IPv6 addresses
      if(isdigit(hostname[i]) == 0 && hostname[i] != '.' && hostname[i] != ':')
      {
        ipFound = FALSE;
        break;
      }
    }

    if(ipFound == TRUE)
    {
      RETURN(0);
      return 0;
    }

    hostname = dot + 1;
    cn += 2;
    cnlen -= 2;
  }

  // now make the case insensitive string compare
  result = cnlen == strlen(hostname) && strcasecmp(cn, hostname) == 0;

  RETURN(result);
  return result;
}

///
/// GENERAL_NAME_free_wrapper
// Define a wrapper function for GENERAL_NAME_free.
// This is a function in AmiSSL.library and hence cannot
// be used as a function pointer directly.
CROSSCALL1NR(GENERAL_NAME_free_wrapper, void *, names)
{
  ENTER();

  GENERAL_NAME_free(names);

  LEAVE();
}

///
/// CheckCertificateIdentity
// Check certificate identity.  Returns zero if identity matches; 1 if
// identity does not match, or <0 if the certificate had no identity.
// If 'identity' is non-NULL, store the malloc-allocated identity in
// *identity.  Logic specified by RFC 2818 and RFC 3280.
static int CheckCertificateIdentity(const char *hostname, X509 *cert, char **identity)
{
  STACK_OF(GENERAL_NAME) *names;
  int match = 0;
  int found = 0;

  ENTER();

  if(hostname == NULL)
    hostname = "";

  // check for subjectAltName entries in the certificate and
  // prefer them for checking the identity.
  names = X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL);
  if(names != NULL)
  {
    int n;

    // subjectAltName contains a sequence of GeneralNames
    for(n = 0; n < sk_GENERAL_NAME_num(names) && match == 0; n++)
    {
      GENERAL_NAME *nm = sk_GENERAL_NAME_value(names, n);

      switch(nm->type)
      {
        // DNS:
        case GEN_DNS:
        {
          char *name = (char *)ASN1_STRING_data(nm->d.ia5);
          D(DBF_NET, "GEN_DNS: '%s'", name);

          if(identity != NULL && found == 0)
            *identity = strdup(name);

          match = MatchHostname(name, strlen(name), hostname);
          found = 1;
        }
        break;

        // "IP Address:"
        case GEN_IPADD:
        {
          char ipaddr[60];

          if(nm->d.iPAddress->length == 4) // IPv4 address
          {
            snprintf(ipaddr, sizeof(ipaddr), "%d.%d.%d.%d", nm->d.iPAddress->data[0],
                     nm->d.iPAddress->data[1],
                     nm->d.iPAddress->data[2],
                     nm->d.iPAddress->data[3]);
          }
          else if((nm->d.iPAddress->length == 16) || // IPv6 address
                  (nm->d.iPAddress->length == 20))
          {
            char *pos = ipaddr;
            int j;
            #define VAL2HEX(s)  ((s) + (((s) >= 10) ? ('a'-10) : '0'))

            for(j = 0; j < nm->d.iPAddress->length; ++j)
            {
              *pos++ = VAL2HEX(nm->d.iPAddress->data[j]);
              *pos++ = ':';
            }
            *pos = '\0';
          }
          else
            E(DBF_NET, "unexpected ip addr length %ld", nm->d.iPAddress->length);

          D(DBF_NET, "GEN_IPADD: '%s'", ipaddr);

          // compare IP address with server IP address
          match = MatchHostname(ipaddr, strlen(ipaddr), hostname);
          found = 1;
        }
        break;

        default:
          D(DBF_NET, "unknown/unsupported GEN type %ld", nm->type);
        break;
      }
    }

    // free all allocated names
    sk_GENERAL_NAME_pop_free(names, ENTRY(GENERAL_NAME_free_wrapper));
  }

  D(DBF_NET, "found: %ld", found);

  // Check against the commonName if no DNS alt. names were found,
  // as per RFC3280.
  if(found == 0)
  {
    X509_NAME *subj = X509_get_subject_name(cert);
    int idx = -1;
    int lastidx;
    char *cname = NULL;

    // find the most specific commonName attribute.
    do
    {
      lastidx = idx;
      idx = X509_NAME_get_index_by_NID(subj, NID_commonName, lastidx);
    }
    while(idx >= 0);

    if(lastidx < 0)
    {
      // no commonName at all
      RETURN(-1);
      return -1;
    }

    // extract the string from the entry
    cname = strdup((char *)ASN1_STRING_data(X509_NAME_ENTRY_get_data(X509_NAME_get_entry(subj, lastidx))));
    if(identity != NULL)
      *identity = strdup(cname);

    match = MatchHostname(cname, strlen(cname), hostname);

    free(cname);
  }

  D(DBF_NET, "Identity match for '%s': %s", hostname, match ? "good" : "bad");

  RETURN(match ? 0 : 1);
  return match ? 0 : 1;
}

///
/// GetCertFingerprint
// extracts the fingerprint of a SSL certificate
static int GetCertFingerprint(const struct Certificate *cert, char *digest)
{
  unsigned char sha1[EVP_MAX_MD_SIZE];
  unsigned int len;
  int result;
  char *p;

  ENTER();

  #if SHA_DIGEST_LENGTH != 20
   #error SHA digest length is not 20 bytes
  #endif

  if(X509_digest(cert->subject, EVP_sha1(), sha1, &len) == 0 || len != SHA_DIGEST_LENGTH)
  {
    ERR_clear_error();
    result = -1;
  }
  else
  {
    int j;

    #define HEX2ASC(x) ((char) ((x) > 9 ? ((x) - 10 + 'A') : ((x) + '0')))
    for(j = 0, p = digest; j < SHA_DIGEST_LENGTH; j++)
    {
      *p++ = HEX2ASC((sha1[j] >> 4) & 0x0f);
      *p++ = HEX2ASC(sha1[j] & 0x0f);
      *p++ = ':';
    }

    p[-1] = '\0'; // NUL terminate
    result = 0;
  }

  RETURN(result);
  return result;
}

///
/// ExtractReadableDN
// extract a readable string of all issuers DNAME information
static char *ExtractReadableDN(X509_NAME *dname)
{
  char *result = NULL;
  int n;
  int flag = 0;
  const ASN1_OBJECT * const cname = OBJ_nid2obj(NID_commonName);
  const ASN1_OBJECT * const email = OBJ_nid2obj(NID_pkcs9_emailAddress);

  ENTER();

  for(n = X509_NAME_entry_count(dname); n > 0; n--)
  {
    X509_NAME_ENTRY *ent = X509_NAME_get_entry(dname, n-1);

    // Skip commonName or emailAddress except if there is no other
    // attribute in dname.
    if((OBJ_cmp(ent->object, cname) && OBJ_cmp(ent->object, email)) ||
       (!flag && n == 1))
    {
      if(flag++)
        dstrcat(&result, ", ");

      dstrcat(&result, (char *)ASN1_STRING_data(X509_NAME_ENTRY_get_data(ent)));
    }
  }

  RETURN(result);
  return result;
}

///
/// ASN1Time2TimeVal
// convert an ASN1_TIME to a TimeVal
static BOOL ASN1Time2TimeVal(const ASN1_TIME *atm, struct TimeVal *tv)
{
  BOOL result = FALSE;

  ENTER();

  if(atm->length >= 12)
  {
    char datestring[] = "MM-DD-YY HH:MM:SS";

    // month
    datestring[0]  = atm->data[2];
    datestring[1]  = atm->data[3];
    // day
    datestring[3]  = atm->data[4];
    datestring[4]  = atm->data[5];
    // year
    datestring[6]  = atm->data[0];
    datestring[7]  = atm->data[1];
    // hour
    datestring[9]  = atm->data[6];
    datestring[10] = atm->data[7];
    // minute
    datestring[12] = atm->data[8];
    datestring[13] = atm->data[9];
    // second
    datestring[15] = atm->data[10];
    datestring[16] = atm->data[11];

    // now convert the temporary string to a TimeVal
    result = String2TimeVal(tv, datestring, DSS_USDATETIME, TZC_NONE);
  }

  RETURN(result);
  return result;
}

///
/// MakeCertificateChain
// takes a X509 certificate chain and creates an own one using
// struct Certificate
static struct Certificate *MakeCertificateChain(STACK_OF(X509) *chain)
{
  int n;
  int count = sk_X509_num(chain);
  struct Certificate *top = NULL;
  struct Certificate *current = NULL;

  ENTER();

  D(DBF_NET, "Certificate chain depth: %ld", count);

  for(n = 0; n < count; n++)
  {
    struct Certificate *cert = calloc(1, sizeof(*cert));
    X509 *x5 = X509_dup(sk_X509_value(chain, n));
    struct TimeVal tv;

    // populate cert
    cert->subject_dn = X509_get_subject_name(x5);
    cert->issuer_dn = X509_get_issuer_name(x5);
    cert->issuer = NULL;
    cert->subject = x5;
    // Retrieve the cert identity; pass a dummy hostname to match.
    cert->identity = NULL;
    CheckCertificateIdentity(NULL, x5, &cert->identity);
    GetCertFingerprint(cert, cert->fingerprint);
    cert->issuerStr = ExtractReadableDN(cert->issuer_dn);
    if(ASN1Time2TimeVal(X509_get_notBefore(cert->subject), &tv))
      TimeVal2String(cert->notBefore, sizeof(cert->notBefore), &tv, DSS_DATETIME, TZC_NONE);
    if(ASN1Time2TimeVal(X509_get_notAfter(cert->subject), &tv))
      TimeVal2String(cert->notAfter, sizeof(cert->notAfter), &tv, DSS_DATETIME, TZC_NONE);

    // now link the certificate to the issuer
    if(top == NULL)
      current = top = cert;
    else
    {
      current->issuer = cert;
      current = cert;
    }
  }

  RETURN(top);
  return top;
}

///
/// FreeCertificateChain
// frees a full certificate chain
static void FreeCertificateChain(struct Certificate *top_cert)
{
  struct Certificate *current;
  struct Certificate *next = top_cert;

  ENTER();

  do
  {
    current = next;
    next = current->issuer;

    dstrfree(current->issuerStr);

    if(current->identity != NULL)
      free(current->identity);

    free(current);
  }
  while(next != NULL);

  LEAVE();
}

///
/// CheckCertificate
// Verifies an SSL server certificate
static int CheckCertificate(struct Connection *conn, struct Certificate *cert)
{
  X509 *x509_cert = cert->subject;
  int ret;

  ENTER();

  SHOWVALUE(DBF_NET, conn->sslCertFailures);

  // If the verification callback hit a case which can't be mapped
  // to one of the exported error bits, it's treated as a hard
  // failure rather than invoking the callback, which can't present
  // a useful error to the user.  "Um, something is wrong.  OK?" */
  if(isFlagSet(conn->sslCertFailures, SSL_CERT_ERR_UNHANDLED))
  {
    #if defined(DEBUG)
    long result = SSL_get_verify_result(conn->ssl);

    E(DBF_NET, "certificate verification error: %s", X509_verify_cert_error_string(result));
    #endif

    RETURN(1);
    return 1;
  }

  ret = CheckCertificateIdentity(conn->server->hostname, x509_cert, NULL);
  if(ret < 0)
  {
    E(DBF_NET, "server certificate was missing commonName attribute in subject name");

    RETURN(1);
    return 1;
  }
  else if(ret > 0)
  {
    D(DBF_NET, "ssl: verify failure SSL_CERT_ERR_IDMISMATCH found");
    setFlag(conn->sslCertFailures, SSL_CERT_ERR_IDMISMATCH);
  }

  // check if the certificate chain could be verified or if
  // we need to ask the user how to continue
  if(conn->sslCertFailures == SSL_CERT_ERR_NONE)
    ret = 0;
  else
  {
    #if defined(DEBUG)
    if((conn->sslCertFailures & ~conn->server->certFailures) != 0)
      W(DBF_NET, "different certificate failures %08lx vs %08lx", conn->sslCertFailures, conn->server->certFailures);

    if(stricmp(cert->fingerprint, conn->server->certFingerprint) != 0)
      W(DBF_NET, "mismatching finger prints '%s' vs '%s'", cert->fingerprint, conn->server->certFingerprint);
    #endif

    // now that we have identified cert failures we check
    // if the user has already accepted these failures and
    // the cert or if we have to ask him once again
    if((conn->sslCertFailures & ~conn->server->certFailures) != 0 || // check if any bits were added
       stricmp(cert->fingerprint, conn->server->certFingerprint) != 0)
    {
      // ask user how to proceed and react upon his request
      if(CertWarningRequest(conn, cert) == TRUE)
        ret = 0; // signal NO error and continue the SSL connection
      else
        ret = 1; // signal ERROR that aborts the SSL connection
    }
    else
    {
      W(DBF_NET, "user accepted cert permanently %08lx vs %08lx", conn->sslCertFailures, conn->server->certFailures);

      // signal NO error
      ret = 0;
    }
  }

  RETURN(ret);
  return ret;
}

///
/// MakeSecureConnection
// Initialize an SSL/TLS session
BOOL MakeSecureConnection(struct Connection *conn)
{
  BOOL secure = FALSE;

  ENTER();

  if(conn != NULL && conn->isConnected == TRUE)
  {
    if(AmiSSLBase != NULL)
    {
      long rc;

      // 1) init AmiSSL
      rc = InitAmiSSL(AmiSSL_ErrNoPtr, &errno,
                      #if defined(__amigaos4__)
                      AmiSSL_ISocket, conn->socketIFace,
                      #else
                      AmiSSL_SocketBase, conn->socketBase,
                      #endif
                      TAG_DONE);

      if(rc != 0) // rc=0 signals NO error
      {
        E(DBF_NET, "InitAmiSSL() failed");
        ER_NewError(tr(MSG_ER_INITAMISSL));
      }
      else
      {
        // 2) check if we have enough entropy
        if((rc = RAND_status()) == 0) // rc=0 is error
          E(DBF_NET, "not enough entropy in the SSL pool");
        // 3) check if we are ready for creating the ssl connection
        else if((conn->ssl = SSL_new(G->sslCtx)) == NULL)
          E(DBF_NET, "can't create a new SSL structure for a connection");
        else if(SSL_set_ex_data(conn->ssl, G->sslDataIndex, conn) == 0)
          E(DBF_NET, "couldn't assign connection pointer");
        else
        {
          // output some debug information
          #if defined(DEBUG)
          {
            int i = 0;
            const char *next = NULL;

            // output all availble ciphers
            D(DBF_NET, "available SSL ciphers:");
            do
            {
              if((next = SSL_get_cipher_list(conn->ssl, i)) != NULL)
                D(DBF_NET, "%s", next);

              i++;
            }
            while(next != NULL);
          }
          #endif

          // 4) set the socket descriptor to the ssl context
          D(DBF_NET, "set socket descriptor %ld for context %08lx", conn->socket, conn->ssl);
          if(SSL_set_fd(conn->ssl, (int)conn->socket) != 1)
            E(DBF_NET, "SSL_set_fd() error, socket %ld", conn->socket);
          else
          {
            BOOL errorState = FALSE;
            int res;

            // 5) establish the ssl connection and take care of non-blocking IO
            D(DBF_NET, "connect SSL context %08lx", conn->ssl);
            STARTCLOCK(DBF_NET);
            while(errorState == FALSE && (res = SSL_connect(conn->ssl)) <= 0)
            {
              #if defined(DEBUG)
              int errnosv = errno; // preserve errno directly after SSL_connect()
              SSL_SESSION *sslSession = NULL;
              #endif
              int err;

              #if defined(DEBUG)
              STOPCLOCK(DBF_NET, "SSL_connect()");
              sslSession = SSL_get_session(conn->ssl);
              D(DBF_NET, "SSL session timeout: %ld s", SSL_get_timeout(sslSession));
              D(DBF_NET, "SSL session times: %ld (%ld)", SSL_get_time(sslSession), time(NULL));
              #endif

              // get the reason why SSL_connect() returned an error
              switch((err = SSL_get_error(conn->ssl, res)))
              {
                case SSL_ERROR_WANT_READ:
                case SSL_ERROR_WANT_WRITE:
                {
                  // we are using non-blocking socket IO so an SSL_ERROR_WANT_READ
                  // signals that the SSL socket wants us to wait until data
                  // is available and reissue the SSL_connect() command.
                  LONG retVal;
                  GET_SOCKETBASE(conn);

                  D(DBF_NET, "SSL_get_error returned %s, running WaitSelect() with timeout %ld s", err == SSL_ERROR_WANT_READ ? "SSL_ERROR_WANT_READ" : "SSL_ERROR_WANT_WRITE", C->SocketTimeout);

                  // set SocketTimeout to our timeout variable
                  // so that we can let WaitSelect() timeout correctly.
                  conn->timeout.tv_sec = C->SocketTimeout;
                  conn->timeout.tv_usec = 0;

                  // now we put our socket handle into a descriptor set
                  // we can pass on to WaitSelect()
                  FD_ZERO(&conn->fdset);
                  FD_SET(conn->socket, &conn->fdset);

                  // depending on the SSL error (WANT_READ/WANT_WRITE)
                  // we either do a WaitSelect() on the read or write mode
                  // as with SSL both things can happen
                  // see http://www.openssl.org/docs/ssl/SSL_connect.html
                  STARTCLOCK(DBF_NET);

                  if(err == SSL_ERROR_WANT_READ)
                    retVal = WaitSelect(conn->socket+1, &conn->fdset, NULL, NULL, (APTR)&conn->timeout, NULL);
                  else
                    retVal = WaitSelect(conn->socket+1, NULL, &conn->fdset, NULL, (APTR)&conn->timeout, NULL);

                  STOPCLOCK(DBF_NET, "WaitSelect()");

                  // if WaitSelect() returns 1 we successfully waited for
                  // being able to write to the socket. So we go and do another
                  // iteration in the while() loop as the next SSL_connect() call should
                  // return EISCONN if the connection really succeeded.
                  if(retVal >= 1 && FD_ISSET(conn->socket, &conn->fdset))
                  {
                    // everything fine
                    STARTCLOCK(DBF_NET); // for the next iteration in calling SSL_connect()
                    continue;
                  }
                  else if(retVal == 0)
                  {
                    W(DBF_NET, "WaitSelect() socket timeout reached");
                    errorState = TRUE;
                  }
                  else
                  {
                    // the rest should signal an error
                    E(DBF_NET, "WaitSelect() returned %ld with SSL_get_error() = %d", retVal, err);
                    errorState = TRUE;
                  }
                }
                break;

                default:
                {
                  E(DBF_NET, "SSL_connect() returned %ld with SSL_get_error() = %ld", res, err);

                  // get more information on the error
                  #if defined(DEBUG)
                  {
                    GET_SOCKETBASE(conn);
                    char buf[255];
                    // save the errno(sv) values to avoid that they are modified by further function calls
                    int _errno = errno;
                    int _errnosv = errnosv;
                    LONG _bsderrno = Errno();
                    unsigned long errcode;

                    // query errno first
                    #if defined(__amigaos4__) || defined(__amigaos3__)
                    if(strerror_r(_errno, buf, sizeof(buf)) != 0)
                    {
                      E(DBF_NET, "strerror_r(errno=%ld) failed", _errno);
                      buf[0] = '\0';
                    }
                    E(DBF_NET, "errno(%ld) = '%s'", _errno, buf);
                    if(strerror_r(_errnosv, buf, sizeof(buf)) != 0)
                    {
                      E(DBF_NET, "strerror_r(errnosv=%ld) failed", _errnosv);
                      buf[0] = '\0';
                    }
                    E(DBF_NET, "errnosv(%ld) = '%s'", _errnosv, buf);
                    #else
                    E(DBF_NET, "errno(%ld) = '%s'", _errno, strerror(_errno));
                    E(DBF_NET, "errnosv(%ld) = '%s'", _errnosv, strerror(_errnosv));
                    #endif

                    E(DBF_NET, "bsderrno(%ld)", _bsderrno);
                    E(DBF_NET, "querying ERR_get_error() stack:");
                    while((errcode = ERR_get_error()) != 0)
                    {
                      ERR_error_string_n(errcode, buf, sizeof(buf));
                      E(DBF_NET, "ERR_get_error()=%ld stack: '%s'", errcode, buf);
                    }
                    E(DBF_NET, "done");
                  }
                  #endif

                  errorState = TRUE;
                }
                break;
              }
            }

            if(errorState == FALSE)
            {
              STACK_OF(X509) *chain;

              // 6) now we get the peer certificate chain
              D(DBF_NET, "get peer certificate chain");
              chain = SSL_get_peer_cert_chain(conn->ssl);
              if(chain == NULL || sk_X509_num(chain) == 0)
                E(DBF_NET, "SSL server did not present certificate, chain=%08lx", chain);
              else
              {
                struct Certificate *cert;

                // 7) make a local copy of the certificate chain so that
                //     we can bug the user with information on accepting/rejecting the certificate
                cert = MakeCertificateChain(chain);

                // 8) now check the certificate chain for any errors and ask the user
                //     how to proceed in case there were an certificate error found
                if(CheckCertificate(conn, cert) != 0)
                  E(DBF_NET, "SSL certificate checks failed");
                else
                {
                  // everything was successfully so lets set the result
                  // value of that function to true
                  secure = TRUE;

                  // Debug information on the certificate
                  #if defined(DEBUG)
                  {
                    char *x509buf;
                    SSL_CIPHER *cipher;
                    X509 *server_cert;
                    char peer_CN[256] = "";

                    cipher = SSL_get_current_cipher(conn->ssl);
                    if(cipher != NULL)
                      D(DBF_NET, "%s connection using %s", SSL_CIPHER_get_version(cipher), SSL_get_cipher(conn->ssl));

                    D(DBF_NET, "Certificate verify result: %ld", SSL_get_verify_result(conn->ssl));

                    if((server_cert = SSL_get_peer_certificate(conn->ssl)) == NULL)
                      E(DBF_NET, "SSL_get_peer_certificate() error!");

                    D(DBF_NET, "Server public key is %ld bits", EVP_PKEY_bits(X509_get_pubkey(server_cert)));

                    X509_NAME_get_text_by_NID(X509_get_subject_name(server_cert), NID_commonName, peer_CN, sizeof(peer_CN));
                    D(DBF_NET, "peer_commonName: '%s'", peer_CN);

                    #define X509BUFSIZE 4096
                    if((x509buf = calloc(1, X509BUFSIZE)) != NULL)
                    {
                      D(DBF_NET, "Server certificate:");

                      if(!(X509_NAME_oneline(X509_get_subject_name(server_cert), x509buf, X509BUFSIZE)))
                        E(DBF_NET, "X509_NAME_oneline...[subject] error!");

                      D(DBF_NET, "subject: %s", x509buf);

                      if(!(X509_NAME_oneline(X509_get_issuer_name(server_cert), x509buf, X509BUFSIZE)))
                        E(DBF_NET, "X509_NAME_oneline...[issuer] error!");

                      D(DBF_NET, "issuer:  %s", x509buf);

                      free(x509buf);
                    }

                    if(server_cert != NULL)
                      X509_free(server_cert);
                  }
                  #endif
                }

                FreeCertificateChain(cert);
              }
            }
          }
        }
      }
    }
    else
      W(DBF_NET, "AmiSSLBase == NULL");

    // if we weren't ale to initialize the TLS/SSL stuff we have to clear it
    // before leaving
    if(secure == FALSE)
    {
      conn->ssl = NULL;
      conn->error = CONNECTERR_SSLFAILED;

      // tell the user if secure connection are impossible due to AmiSSL being unavailable
      if(AmiSSLBase == NULL)
        ER_NewError(tr(MSG_ER_UNUSABLEAMISSL));
    }
  }

  RETURN(secure);
  return secure;
}

///
///
BOOL InitSSLConnections(void)
{
  BOOL result = FALSE;
  ENTER();

  // try to open amisslmaster.library first
  if((AmiSSLMasterBase = OpenLibrary("amisslmaster.library", AMISSLMASTER_VERSION)) != NULL &&
     LIB_VERSION_IS_AT_LEAST(AmiSSLMasterBase, AMISSLMASTER_VERSION, AMISSLMASTER_REVISION) && // minimum 3.5
     GETINTERFACE("main", 1, IAmiSSLMaster, AmiSSLMasterBase))
  {
    if(InitAmiSSLMaster(AMISSL_VERSION, TRUE))
    {
      if((AmiSSLBase = OpenAmiSSL()) != NULL &&
         GETINTERFACE("main", 1, IAmiSSL, AmiSSLBase))
      {
        char tmp[24+1];

        D(DBF_STARTUP, "successfully opened AmiSSL library %d.%d (%s)", AmiSSLBase->lib_Version, AmiSSLBase->lib_Revision, AmiSSLBase->lib_IdString);

        // initialize AmiSSL/OpenSSL related stuff that
        // needs to be initialized before each threads spans
        // own initializations
        ERR_load_BIO_strings(); // Load BIO error strings
        SSL_load_error_strings(); // Load SSL error strings
        OpenSSL_add_all_algorithms(); // Load all available encryption algorithms
        SSL_library_init(); // Initialize OpenSSL's SSL libraries

        // seed the random number generator with some valuable entropy
        D(DBF_NET, "AmiSSL: seeding random number generator");
        snprintf(tmp, sizeof(tmp), "%08lx%08lx%08lx", (unsigned long)time((time_t *)NULL), (unsigned long)FindTask(NULL), (unsigned long)rand());
        RAND_seed(tmp, strlen(tmp));

        // 1) now we create a common SSL_CTX object which all our SSL connections will share
        if((G->sslCtx = SSL_CTX_new(TLS_client_method())) == NULL)
          E(DBF_NET, "AmiSSL: can't create SSL_CTX object!");
        // 2) set minimum allowed protocol version to SSL3 (SSLv2 is deprecated/insecure)
        else if(SSL_CTX_set_min_proto_version(G->sslCtx, SSL3_VERSION) == 0)
          E(DBF_NET, "AmiSSL: couldn't set minimum protocol version to SSL3. SSL: %s", ERR_error_string(ERR_get_error(), NULL));
        else
        {
          int rc = 0; // make sure set_default_verify_paths() is called

          D(DBF_NET, "AmiSSL: SSL ctx timeout: %ld s", SSL_CTX_get_timeout(G->sslCtx));

          if(FileExists(DEFAULT_CAPATH) == TRUE)
          {
            D(DBF_NET, "AmiSSL: CAfile = '%s', CApath = '%s'", DEFAULT_CAFILE, DEFAULT_CAPATH);

            if(FileExists(DEFAULT_CAFILE) == FALSE)
              ER_NewError(tr(MSG_ER_WARN_CAFILE), DEFAULT_CAFILE);

            // 3) load the certificates (e.g. CA) from either a file or a directory path
            if(FileExists(DEFAULT_CAFILE) == TRUE)
              rc = SSL_CTX_load_verify_locations(G->sslCtx, DEFAULT_CAFILE, DEFAULT_CAPATH);
            else
              rc = SSL_CTX_load_verify_locations(G->sslCtx, NULL, DEFAULT_CAPATH);

            if(rc == 0)
            {
              W(DBF_NET, "AmiSSL: setting default verify locations failed!");

              ER_NewError(tr(MSG_ER_WARN_LOADCAPATH), DEFAULT_CAFILE, DEFAULT_CAPATH);
            }
          }
          else
            ER_NewError(tr(MSG_ER_WARN_CAPATH), DEFAULT_CAPATH);

          // 4) if no CA file or path is given we set the default pathes
          if(rc == 0 && (rc = SSL_CTX_set_default_verify_paths(G->sslCtx)) == 0)
            E(DBF_NET, "AmiSSL: setting default verify locations failed");

          // 5) get a new ssl Data Index for storing application specific data
          if(rc != 0 && (G->sslDataIndex = SSL_get_ex_new_index(0, NULL, NULL, NULL, NULL)) < 0)
          {
            E(DBF_NET, "AmiSSL: SSL_get_ex_new_index() failed");
            rc = 0; // error
          }

          // 6) set SSL_VERIFY_PEER so that we later can decide on our own in the verify_callback
          //    function wheter the connection should continue or if it should be terminated right away.
          SSL_CTX_set_verify(G->sslCtx, SSL_VERIFY_PEER, ENTRY(verify_callback));

          // 7) set the ciphers we want to use and exclude unwanted ones
          if(rc != 0 && (rc = SSL_CTX_set_cipher_list(G->sslCtx, C->DefaultSSLCiphers)) == 0)
             E(DBF_NET, "AmiSSL: SSL_CTX_set_cipher_list() error!");
          else
          {
            D(DBF_STARTUP, "AmiSSL: successfully initialized");

            result = TRUE;
          }
        }
      }
      else
        E(DBF_NET, "AmiSSL: OpenAmiSSL() returned an error");
    }
    else
      E(DBF_NET, "AmiSSL: InitAmiSSLMaster() returned an error");
  }
  else
    W(DBF_NET, "AmiSSL: Couldn't open required amisslmaster lib version %d.%d", AMISSLMASTER_VERSION, AMISSLMASTER_REVISION);

  // if an error occurred make sure to have everything cleaned up correctly.
  if(result == FALSE)
    CleanupSSLConnections();

  RETURN(result);
  return result;
}

///
///
void CleanupSSLConnections(void)
{
  ENTER();

  // cleanup the SSL connection context
  if(G->sslCtx != NULL)
  {
    SSL_CTX_free(G->sslCtx);
    G->sslCtx = NULL;
  }

  // close amissl
  if(AmiSSLBase != NULL)
  {
    CleanupAmiSSLA(NULL);
    DROPINTERFACE(IAmiSSL);
    CloseAmiSSL();
    AmiSSLBase = NULL;
  }

  // close amisslmaster
  if(AmiSSLMasterBase != NULL)
  {
    DROPINTERFACE(IAmiSSLMaster);
    CloseLibrary(AmiSSLMasterBase);
    AmiSSLMasterBase = NULL;
  }

  LEAVE();
}

///
