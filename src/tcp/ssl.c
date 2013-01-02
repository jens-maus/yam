/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2013 YAM Open Source Team

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
#include "YAM_config.h"
#include "YAM_error.h"
#include "YAM_utilities.h"

#include "FileInfo.h"
#include "Locale.h"
#include "MailServers.h"
#include "Requesters.h"
#include "StrBuf.h"

#include "tcp/Connection.h"
#include "tcp/ssl.h"

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
  struct Connection *conn;

  ENTER();

  SHOWVALUE(DBF_NET, preverify_ok);
  SHOWVALUE(DBF_NET, x509_ctx);

  // if there is no error we do nothing but return
  if(preverify_ok == 0)
  {
    // get our (struct Connection) structure out of the app data
    if((conn = X509_STORE_CTX_get_app_data(x509_ctx)) != NULL)
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
        case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
        case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
        case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
        case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
        case X509_V_ERR_CERT_UNTRUSTED:
        case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
        {
          D(DBF_NET, "ssl: verify failure SSL_CERT_ERR_UNTRUSTED found");
          setFlag(failures, SSL_CERT_ERR_UNTRUSTED);
        }
        break;

        case X509_V_ERR_CERT_NOT_YET_VALID:
        {
          D(DBF_NET, "ssl: verify failure %s found", depth > 0 ? "SSL_CERT_ERR_BADCHAIN" : "SSL_CERT_ERR_NOTYETVALID");
          setFlag(failures, depth > 0 ? SSL_CERT_ERR_BADCHAIN : SSL_CERT_ERR_NOTYETVALID);
        }
        break;

        case X509_V_ERR_CERT_HAS_EXPIRED:
        {
          D(DBF_NET, "ssl: verify failure %s found", depth > 0 ? "SSL_CERT_ERR_BADCHAIN" : "SSL_CERT_ERR_EXPIRED");
          setFlag(failures, depth > 0 ? SSL_CERT_ERR_BADCHAIN : SSL_CERT_ERR_EXPIRED);
        }
        break;

        case X509_V_OK:
          // nothing
        break;

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
      E(DBF_NET, "X509_STORE_CTX_get_app_data() return invalid 'conn'");
  }

  RETURN(preverify_ok);
  return preverify_ok;
}

///
/// cert_verify_callback
//
CROSSCALL2(cert_verify_callback, int, X509_STORE_CTX *, x509_ctx, void *, parm)
{
  int ok;

  ENTER();

  SHOWVALUE(DBF_NET, x509_ctx);
  SHOWVALUE(DBF_NET, parm);

  // Store param (struct Connection) in context for verify_callback()
  ok = X509_STORE_CTX_set_app_data(x509_ctx, parm);

  // and verify the certificate chain if no error
  if(ok == 1)
    ok = X509_verify_cert(x509_ctx);

  RETURN(ok);
  return ok;
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
  char *result = AllocStrBuf(SIZE_DEFAULT);
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
        StrBufCat(&result, ", ");

      StrBufCat(&result, (char *)ASN1_STRING_data(X509_NAME_ENTRY_get_data(ent)));
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

  if(atm->length >= 10)
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

    FreeStrBuf(current->issuerStr);

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

    E(DBF_NET, "Certificate verification error: %s", X509_verify_cert_error_string(result));
    #endif

    RETURN(1);
    return 1;
  }

  ret = CheckCertificateIdentity(conn->server->hostname, x509_cert, NULL);
  if(ret < 0)
  {
    E(DBF_NET, "Server certificate was missing commonName attribute in subject name");

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
      W(DBF_NET, "User accepted cert permanently %08lx vs %08lx", conn->sslCertFailures, conn->server->certFailures);

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
      #if defined(__amigaos4__)
      rc = InitAmiSSL(AmiSSL_ISocket, conn->socketIFace, TAG_DONE);
      #else
      rc = InitAmiSSL(AmiSSL_SocketBase, conn->socketBase, TAG_DONE);
      #endif

      if(rc != 0) // rc=0 signals NO error
      {
        E(DBF_NET, "InitAmiSSL() failed");
        ER_NewError(tr(MSG_ER_INITAMISSL));
      }
      else
      {
        char tmp[24+1];

        // 2) lets initialize the library and load the error strings
        // these function don't return any serious error
        SSL_load_error_strings();
        SSL_library_init();

        // 3) We have to feed the random number generator first
        D(DBF_NET, "seeding random number generator");
        snprintf(tmp, sizeof(tmp), "%08lx%08lx%08lx", (unsigned long)time((time_t *)NULL), (unsigned long)FindTask(NULL), (unsigned long)rand());
        RAND_seed(tmp, strlen(tmp));

        // 4) check if we have enough entropy
        if((rc = RAND_status()) == 0) // rc=0 is error
          E(DBF_NET, "not enough entropy in the SSL pool");
        // 5) create new SSL context
        else if((conn->sslCtx = SSL_CTX_new(SSLv23_client_method())) == NULL)
          E(DBF_NET, "can't create SSL_CTX object!");
        // 6) disable SSLv2 as it is insecure and obsolete
        else if(isFlagClear(SSL_CTX_set_options(conn->sslCtx, SSL_OP_ALL | SSL_OP_NO_SSLv2), SSL_OP_NO_SSLv2))
          E(DBF_NET, "SSLv2 couldn't be disabled. SSL: %s", ERR_error_string(ERR_get_error(), NULL));
        else
        {
          rc = 0; // make sure set_default_verify_paths() is called

          if(FileExists(DEFAULT_CAPATH) == TRUE)
          {
            D(DBF_NET, "CAfile = '%s', CApath = '%s'", DEFAULT_CAFILE, DEFAULT_CAPATH);

            if(FileExists(DEFAULT_CAFILE) == FALSE)
              ER_NewError(tr(MSG_ER_WARN_CAFILE), DEFAULT_CAFILE);

            // 7) load the certificates (e.g. CA) from either a file or a directory path
            if((rc = SSL_CTX_load_verify_locations(conn->sslCtx, DEFAULT_CAFILE, DEFAULT_CAPATH)) == 0)
            {
              W(DBF_NET, "warning: setting default verify locations failed!");

              ER_NewError(tr(MSG_ER_WARN_LOADCAPATH), DEFAULT_CAFILE, DEFAULT_CAPATH);
            }
          }
          else
            ER_NewError(tr(MSG_ER_WARN_CAPATH), DEFAULT_CAPATH);

          // 7) if no CA file or path is given we set the default pathes
          if(rc == 0 && (rc = SSL_CTX_set_default_verify_paths(conn->sslCtx)) == 0)
            E(DBF_NET, "error: setting default verify locations failed");

          // 8) set SSL_VERIFY_PEER so that we later can decide on our own in the verify_callback
          //    function wheter the connection should continue or if it should be terminated right away.
          SSL_CTX_set_verify(conn->sslCtx, SSL_VERIFY_PEER, ENTRY(verify_callback));

          // 9) set the cert_verify_callback as well so that we can supply userdata (struct Connection)
          //    to the verify_callback() func
          SSL_CTX_set_cert_verify_callback(conn->sslCtx, ENTRY(cert_verify_callback), conn);

          // 10) set the ciphers we want to use and exclude unwanted ones
          if(rc != 0 && (rc = SSL_CTX_set_cipher_list(conn->sslCtx, C->DefaultSSLCiphers)) == 0)
            E(DBF_NET, "SSL_CTX_set_cipher_list() error!");
          else
          {
            D(DBF_NET, "initializing TLS/SSL session");

            // 11) check if we are ready for creating the ssl connection
            if((conn->ssl = SSL_new(conn->sslCtx)) == NULL)
              E(DBF_NET, "can't create a new SSL structure for a connection");
            else
            {
              // output some debug information on the
              // available ciphers
              #if defined(DEBUG)
              {
                int i = 0;
                const char *next = NULL;

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

              // 12) set the socket descriptor to the ssl context
			  D(DBF_NET, "set socket descriptor %ld for context %08lx", conn->socket, conn->ssl);
              if(SSL_set_fd(conn->ssl, (int)conn->socket) != 1)
                E(DBF_NET, "SSL_set_fd() error, socket %ld", conn->socket);
              else
              {
                BOOL errorState = FALSE;
                int res;

                // 13) establish the ssl connection and take care of non-blocking IO
				D(DBF_NET, "connect SSL context %08lx", conn->ssl);
                while(errorState == FALSE && (res = SSL_connect(conn->ssl)) <= 0)
                {
                  int err = SSL_get_error(conn->ssl, res);

                  switch(err)
                  {
                    case SSL_ERROR_WANT_READ:
                    case SSL_ERROR_WANT_WRITE:
                    {
                      // we are using non-blocking socket IO so an SSL_ERROR_WANT_READ
                      // signals that the SSL socket wants us to wait until data
                      // is available and reissue the SSL_connect() command.
                      LONG retVal;
                      GET_SOCKETBASE(conn);

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
                      if(err == SSL_ERROR_WANT_READ)
                        retVal = WaitSelect(conn->socket+1, &conn->fdset, NULL, NULL, (APTR)&conn->timeout, NULL);
                      else
                        retVal = WaitSelect(conn->socket+1, NULL, &conn->fdset, NULL, (APTR)&conn->timeout, NULL);

                      // if WaitSelect() returns 1 we successfully waited for
                      // being able to write to the socket. So we go and do another
                      // iteration in the while() loop as the next connect() call should
                      // return EISCONN if the connection really succeeded.
                      if(retVal >= 1 && FD_ISSET(conn->socket, &conn->fdset))
                      {
                        // everything fine
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
                        E(DBF_NET, "WaitSelect() returned error %ld", err);
                        errorState = TRUE;
                      }
                    }
                    break;

                    default:
                    {
                      E(DBF_NET, "SSL_connect() returned error %ld", err);
                      errorState = TRUE;
                    }
                    break;
                  }
                }

                if(errorState == FALSE)
                {
                  STACK_OF(X509) *chain;

                  // 14) now we get the peer certificate chain
				  D(DBF_NET, "get peer certificate chain");
                  chain = SSL_get_peer_cert_chain(conn->ssl);
                  if(chain == NULL || sk_X509_num(chain) == 0)
                    E(DBF_NET, "SSL server did not present certificate, chain=%08lx", chain);
                  else
                  {
                    struct Certificate *cert;

                    // 15) make a local copy of the certificate chain so that
                    //     we can bug the user with information on accepting/rejecting the certificate
                    cert = MakeCertificateChain(chain);

                    // 16) now check the certificate chain for any errors and ask the user
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
      }
    }
    else
      W(DBF_NET, "AmiSSLBase == NULL");

    // if we weren't ale to initialize the TLS/SSL stuff we have to clear it
    // before leaving
    if(secure == FALSE)
    {
      if(conn->sslCtx != NULL)
      {
        SSL_CTX_free(conn->sslCtx);
        conn->sslCtx = NULL;
      }

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
