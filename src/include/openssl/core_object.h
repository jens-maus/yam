/*
 * Copyright (c) 1999-2006 Andrija Antonijevic, Stefan Burstroem.
 * Copyright (c) 2014-2025 AmiSSL Open Source Team.
 * All Rights Reserved.
 *
 * This file has been modified for use with AmiSSL for AmigaOS-based systems.
 *
 * Copyright 2020-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#if !defined(PROTO_AMISSL_H) && !defined(AMISSL_COMPILE)
# include <proto/amissl.h>
#endif

#ifndef OPENSSL_CORE_OBJECT_H
# define OPENSSL_CORE_OBJECT_H
# if defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ > 3))
#  pragma once
# endif

# ifdef __cplusplus
extern "C" {
# endif

/*-
 * Known object types
 *
 * These numbers are used as values for the OSSL_PARAM parameter
 * OSSL_OBJECT_PARAM_TYPE.
 *
 * For most of these types, there's a corresponding libcrypto object type.
 * The corresponding type is indicated with a comment after the number.
 */
# define OSSL_OBJECT_UNKNOWN            0
# define OSSL_OBJECT_NAME               1 /* char * */
# define OSSL_OBJECT_PKEY               2 /* EVP_PKEY * */
# define OSSL_OBJECT_CERT               3 /* X509 * */
# define OSSL_OBJECT_CRL                4 /* X509_CRL * */

/*
 * The rest of the associated OSSL_PARAM elements is described in core_names.h
 */

# ifdef __cplusplus
}
# endif

#endif
