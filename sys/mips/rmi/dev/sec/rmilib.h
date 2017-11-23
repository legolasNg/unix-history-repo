/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2003-2009 RMI Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of RMI Corporation, nor the names of its contributors,
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * $FreeBSD$
 * RMI_BSD
 */

#ifndef _RMILIB_H_
#define _RMILIB_H_

#include <sys/cdefs.h>
#include <mips/rmi/dev/sec/desc.h>
#include <mips/rmi/iomap.h>

/* #define XLR_SEC_CMD_DEBUG */

#ifdef XLR_SEC_CMD_DEBUG
#define DPRINT  printf
#define XLR_SEC_CMD_DIAG(fmt, args...) { \
                DPRINT(fmt, ##args); \
        }
#define XLR_SEC_CMD_DIAG_SYM_DESC(desc, vec) { \
                decode_symkey_desc ((desc), (vec)); \
        }
#else
#define DPRINT(fmt, args...)
#define XLR_SEC_CMD_DIAG(fmt, args...)
#define XLR_SEC_CMD_DIAG_SYM_DESC(desc, vec)
#endif






/*
#include <mips/include/pmap.h>

#define OS_ALLOC_KERNEL(size) kmalloc((size), GFP_KERNEL)
#define virt_to_phys(x)  vtophys((vm_offset_t)(x))
*/
/*
 * Cryptographic parameter definitions
 */
#define XLR_SEC_DES_KEY_LENGTH        8	/* Bytes */
#define XLR_SEC_3DES_KEY_LENGTH       24	/* Bytes */
#define XLR_SEC_AES128_KEY_LENGTH     16	/* Bytes */
#define XLR_SEC_AES192_KEY_LENGTH     24	/* Bytes */
#define XLR_SEC_AES256_KEY_LENGTH     32	/* Bytes */
#define XLR_SEC_AES128F8_KEY_LENGTH   32	/* Bytes */
#define XLR_SEC_AES192F8_KEY_LENGTH   48	/* Bytes */
#define XLR_SEC_AES256F8_KEY_LENGTH   64	/* Bytes */
#define XLR_SEC_KASUMI_F8_KEY_LENGTH  16	/* Bytes */
#define XLR_SEC_MAX_CRYPT_KEY_LENGTH  XLR_SEC_AES256F8_KEY_LENGTH


#define XLR_SEC_DES_IV_LENGTH         8	/* Bytes */
#define XLR_SEC_AES_IV_LENGTH         16	/* Bytes */
#define XLR_SEC_ARC4_IV_LENGTH        0	/* Bytes */
#define XLR_SEC_KASUMI_F8_IV_LENGTH   16	/* Bytes */
#define XLR_SEC_MAX_IV_LENGTH         16	/* Bytes */
#define XLR_SEC_IV_LENGTH_BYTES       8	/* Bytes */

#define XLR_SEC_AES_BLOCK_SIZE        16	/* Bytes */
#define XLR_SEC_DES_BLOCK_SIZE        8	/* Bytes */
#define XLR_SEC_3DES_BLOCK_SIZE       8	/* Bytes */

#define XLR_SEC_MD5_BLOCK_SIZE        64	/* Bytes */
#define XLR_SEC_SHA1_BLOCK_SIZE       64	/* Bytes */
#define XLR_SEC_SHA256_BLOCK_SIZE     64	/* Bytes */
#define XLR_SEC_SHA384_BLOCK_SIZE     128	/* Bytes */
#define XLR_SEC_SHA512_BLOCK_SIZE     128	/* Bytes */
#define XLR_SEC_GCM_BLOCK_SIZE        16	/* XXX: Bytes */
#define XLR_SEC_KASUMI_F9_BLOCK_SIZE  16	/* XXX: Bytes */
#define XLR_SEC_MAX_BLOCK_SIZE        64	/* Max of MD5/SHA */
#define XLR_SEC_MD5_LENGTH            16	/* Bytes */
#define XLR_SEC_SHA1_LENGTH           20	/* Bytes */
#define XLR_SEC_SHA256_LENGTH         32	/* Bytes */
#define XLR_SEC_SHA384_LENGTH         64	/* Bytes */
#define XLR_SEC_SHA512_LENGTH         64	/* Bytes */
#define XLR_SEC_GCM_LENGTH            16	/* Bytes */
#define XLR_SEC_KASUMI_F9_LENGTH      16	/* Bytes */
#define XLR_SEC_KASUMI_F9_RESULT_LENGTH 4	/* Bytes */
#define XLR_SEC_HMAC_LENGTH           64	/* Max of MD5/SHA/SHA256 */
#define XLR_SEC_MAX_AUTH_KEY_LENGTH   XLR_SEC_SHA512_BLOCK_SIZE
#define XLR_SEC_MAX_RC4_STATE_SIZE    264	/* char s[256], int i, int j */

/* Status code is used by the SRL to indicate status */
typedef unsigned int xlr_sec_status_t;

/*
 * Status codes
 */
#define XLR_SEC_STATUS_SUCCESS              0
#define XLR_SEC_STATUS_NO_DEVICE           -1
#define XLR_SEC_STATUS_TIMEOUT             -2
#define XLR_SEC_STATUS_INVALID_PARAMETER   -3
#define XLR_SEC_STATUS_DEVICE_FAILED       -4
#define XLR_SEC_STATUS_DEVICE_BUSY         -5
#define XLR_SEC_STATUS_NO_RESOURCE         -6
#define XLR_SEC_STATUS_CANCELLED           -7

/*
 * Flags
 */
#define XLR_SEC_FLAGS_HIGH_PRIORITY         1

/* Error code is used to indicate any errors */
typedef int xlr_sec_error_t;

/*
 */
#define XLR_SEC_ERR_NONE                    0
#define XLR_SEC_ERR_CIPHER_OP              -1
#define XLR_SEC_ERR_CIPHER_TYPE            -2
#define XLR_SEC_ERR_CIPHER_MODE            -3
#define XLR_SEC_ERR_CIPHER_INIT            -4
#define XLR_SEC_ERR_DIGEST_TYPE            -5
#define XLR_SEC_ERR_DIGEST_INIT            -6
#define XLR_SEC_ERR_DIGEST_SRC             -7
#define XLR_SEC_ERR_CKSUM_TYPE             -8
#define XLR_SEC_ERR_CKSUM_SRC              -9
#define XLR_SEC_ERR_ALLOC                  -10
#define XLR_SEC_ERR_CONTROL_VECTOR         -11
#define XLR_SEC_ERR_LOADHMACKEY_MODE       -12
#define XLR_SEC_ERR_PADHASH_MODE           -13
#define XLR_SEC_ERR_HASHBYTES_MODE         -14
#define XLR_SEC_ERR_NEXT_MODE              -15
#define XLR_SEC_ERR_PKT_IV_MODE            -16
#define XLR_SEC_ERR_LASTWORD_MODE          -17
#define XLR_SEC_ERR_PUBKEY_OP              -18
#define XLR_SEC_ERR_SYMKEY_MSGSND          -19
#define XLR_SEC_ERR_PUBKEY_MSGSND          -20
#define XLR_SEC_ERR_SYMKEY_GETSEM          -21
#define XLR_SEC_ERR_PUBKEY_GETSEM          -22

/*
 * Descriptor Vector quantities
 *  (helps to identify descriptor type per operation)
 */
#define XLR_SEC_VECTOR_CIPHER_DES             0x0001
#define XLR_SEC_VECTOR_CIPHER_3DES            0x0002
#define XLR_SEC_VECTOR_CIPHER_AES128          0x0004
#define XLR_SEC_VECTOR_CIPHER_AES192          0x0008
#define XLR_SEC_VECTOR_CIPHER_AES256          0x0010
#define XLR_SEC_VECTOR_CIPHER_ARC4            0x0020
#define XLR_SEC_VECTOR_CIPHER_AES             (XLR_SEC_VECTOR_CIPHER_AES128 | \
                                           XLR_SEC_VECTOR_CIPHER_AES192 | \
                                           XLR_SEC_VECTOR_CIPHER_AES256)
#define XLR_SEC_VECTOR_CIPHER                 (XLR_SEC_VECTOR_CIPHER_DES | \
                                           XLR_SEC_VECTOR_CIPHER_3DES | \
                                           XLR_SEC_VECTOR_CIPHER_AES128 | \
                                           XLR_SEC_VECTOR_CIPHER_AES192 | \
                                           XLR_SEC_VECTOR_CIPHER_AES256 | \
                                           XLR_SEC_VECTOR_CIPHER_ARC4)

#define XLR_SEC_VECTOR_HMAC                   0x0040
#define XLR_SEC_VECTOR_MAC                    0x0080
#define XLR_SEC_VECTOR_MODE_CTR_CFB           0x0100
#define XLR_SEC_VECTOR_MODE_ECB_CBC_OFB       0x0200
#define XLR_SEC_VECTOR_MODE_ECB_CBC           0x0400
#define XLR_SEC_VECTOR_STATE                  0x0800
#define XLR_SEC_VECTOR_CIPHER_KASUMI_F8       0x01000
#define XLR_SEC_VECTOR_HMAC2                  0x02000
#define XLR_SEC_VECTOR_GCM                    0x04000
#define XLR_SEC_VECTOR_F9                     0x08000
#define XLR_SEC_VECTOR_MODE_F8                0x10000

#define XLR_SEC_VECTOR_CIPHER_ARC4__HMAC  \
(XLR_SEC_VECTOR_CIPHER_ARC4 | XLR_SEC_VECTOR_HMAC)
#define XLR_SEC_VECTOR_CIPHER_ARC4__STATE  \
(XLR_SEC_VECTOR_CIPHER_ARC4 | XLR_SEC_VECTOR_STATE)
#define XLR_SEC_VECTOR_CIPHER_ARC4__HMAC__STATE  \
(XLR_SEC_VECTOR_CIPHER_ARC4 | XLR_SEC_VECTOR_HMAC | XLR_SEC_VECTOR_STATE)

#define XLR_SEC_VECTOR__CIPHER_DES__HMAC__MODE_ECB_CBC \
(XLR_SEC_VECTOR_CIPHER_DES | XLR_SEC_VECTOR_HMAC | XLR_SEC_VECTOR_MODE_ECB_CBC)

#define XLR_SEC_VECTOR__CIPHER_DES__MODE_ECB_CBC \
(XLR_SEC_VECTOR_CIPHER_DES | XLR_SEC_VECTOR_MODE_ECB_CBC)

#define XLR_SEC_VECTOR__CIPHER_3DES__HMAC__MODE_ECB_CBC \
(XLR_SEC_VECTOR_CIPHER_3DES | XLR_SEC_VECTOR_HMAC | XLR_SEC_VECTOR_MODE_ECB_CBC)

#define XLR_SEC_VECTOR__CIPHER_3DES__MODE_ECB_CBC \
(XLR_SEC_VECTOR_CIPHER_3DES | XLR_SEC_VECTOR_MODE_ECB_CBC)

#define XLR_SEC_VECTOR__CIPHER_AES128__HMAC__MODE_CTR_CFB \
(XLR_SEC_VECTOR_CIPHER_AES128 | XLR_SEC_VECTOR_HMAC | XLR_SEC_VECTOR_MODE_CTR_CFB)

#define XLR_SEC_VECTOR__CIPHER_AES128__MODE_CTR_CFB \
(XLR_SEC_VECTOR_CIPHER_AES128 | XLR_SEC_VECTOR_MODE_CTR_CFB)

#define XLR_SEC_VECTOR__CIPHER_AES128__HMAC__MODE_ECB_CBC_OFB \
(XLR_SEC_VECTOR_CIPHER_AES128 | XLR_SEC_VECTOR_HMAC | XLR_SEC_VECTOR_MODE_ECB_CBC_OFB)

#define XLR_SEC_VECTOR__CIPHER_AES128__MODE_ECB_CBC_OFB \
(XLR_SEC_VECTOR_CIPHER_AES128 | XLR_SEC_VECTOR_MODE_ECB_CBC_OFB)

#define XLR_SEC_VECTOR__CIPHER_AES192__HMAC__MODE_CTR_CFB \
(XLR_SEC_VECTOR_CIPHER_AES192 | XLR_SEC_VECTOR_HMAC | XLR_SEC_VECTOR_MODE_CTR_CFB)

#define XLR_SEC_VECTOR__CIPHER_AES192__MODE_CTR_CFB \
(XLR_SEC_VECTOR_CIPHER_AES192 | XLR_SEC_VECTOR_MODE_CTR_CFB)

#define XLR_SEC_VECTOR__CIPHER_AES192__HMAC__MODE_ECB_CBC_OFB \
(XLR_SEC_VECTOR_CIPHER_AES192 | XLR_SEC_VECTOR_HMAC | XLR_SEC_VECTOR_MODE_ECB_CBC_OFB)

#define XLR_SEC_VECTOR__CIPHER_AES192__MODE_ECB_CBC_OFB \
(XLR_SEC_VECTOR_CIPHER_AES192 | XLR_SEC_VECTOR_MODE_ECB_CBC_OFB)

#define XLR_SEC_VECTOR__CIPHER_AES256__HMAC__MODE_CTR_CFB \
(XLR_SEC_VECTOR_CIPHER_AES256 | XLR_SEC_VECTOR_HMAC | XLR_SEC_VECTOR_MODE_CTR_CFB)

#define XLR_SEC_VECTOR__CIPHER_AES256__MODE_CTR_CFB \
(XLR_SEC_VECTOR_CIPHER_AES256 | XLR_SEC_VECTOR_MODE_CTR_CFB)

#define XLR_SEC_VECTOR__CIPHER_AES256__HMAC__MODE_ECB_CBC_OFB \
(XLR_SEC_VECTOR_CIPHER_AES256 | XLR_SEC_VECTOR_HMAC | XLR_SEC_VECTOR_MODE_ECB_CBC_OFB)

#define XLR_SEC_VECTOR__CIPHER_AES256__MODE_ECB_CBC_OFB \
(XLR_SEC_VECTOR_CIPHER_AES256 | XLR_SEC_VECTOR_MODE_ECB_CBC_OFB)

#define XLR_SEC_VECTOR__CIPHER_AES128__HMAC__MODE_F8 \
(XLR_SEC_VECTOR_CIPHER_AES128 | XLR_SEC_VECTOR_HMAC | XLR_SEC_VECTOR_MODE_F8)

#define XLR_SEC_VECTOR__CIPHER_AES128__MODE_F8 \
(XLR_SEC_VECTOR_CIPHER_AES128 | XLR_SEC_VECTOR_MODE_F8)

#define XLR_SEC_VECTOR__CIPHER_AES192__HMAC__MODE_F8 \
(XLR_SEC_VECTOR_CIPHER_AES192 | XLR_SEC_VECTOR_HMAC | XLR_SEC_VECTOR_MODE_F8)

#define XLR_SEC_VECTOR__CIPHER_AES192__MODE_F8 \
(XLR_SEC_VECTOR_CIPHER_AES192 | XLR_SEC_VECTOR_MODE_F8)

#define XLR_SEC_VECTOR__CIPHER_AES256__HMAC__MODE_F8 \
(XLR_SEC_VECTOR_CIPHER_AES256 | XLR_SEC_VECTOR_HMAC | XLR_SEC_VECTOR_MODE_F8)

#define XLR_SEC_VECTOR__CIPHER_AES256__MODE_F8 \
(XLR_SEC_VECTOR_CIPHER_AES256 | XLR_SEC_VECTOR_MODE_F8)

#define XLR_SEC_VECTOR_CIPHER_KASUMI_F8__F9  \
(XLR_SEC_VECTOR_CIPHER_KASUMI_F8 | XLR_SEC_VECTOR_F9)

#define XLR_SEC_VECTOR_CIPHER_KASUMI_F8__HMAC  \
(XLR_SEC_VECTOR_CIPHER_KASUMI_F8 | XLR_SEC_VECTOR_HMAC)

#define XLR_SEC_VECTOR_CIPHER_KASUMI_F8__HMAC2  \
(XLR_SEC_VECTOR_CIPHER_KASUMI_F8 | XLR_SEC_VECTOR_HMAC2)

#define XLR_SEC_VECTOR_CIPHER_KASUMI_F8__GCM  \
(XLR_SEC_VECTOR_CIPHER_KASUMI_F8 | XLR_SEC_VECTOR_GCM)

#define XLR_SEC_VECTOR_CIPHER_ARC4__HMAC2  \
(XLR_SEC_VECTOR_CIPHER_ARC4 | XLR_SEC_VECTOR_HMAC2)

#define XLR_SEC_VECTOR_CIPHER_ARC4__HMAC2__STATE  \
(XLR_SEC_VECTOR_CIPHER_ARC4 | XLR_SEC_VECTOR_HMAC2 | XLR_SEC_VECTOR_STATE)

#define XLR_SEC_VECTOR__CIPHER_DES__HMAC2__MODE_ECB_CBC \
(XLR_SEC_VECTOR_CIPHER_DES | XLR_SEC_VECTOR_HMAC2 | XLR_SEC_VECTOR_MODE_ECB_CBC)

#define XLR_SEC_VECTOR__CIPHER_3DES__HMAC2__MODE_ECB_CBC \
(XLR_SEC_VECTOR_CIPHER_3DES | XLR_SEC_VECTOR_HMAC2 | XLR_SEC_VECTOR_MODE_ECB_CBC)

#define XLR_SEC_VECTOR__CIPHER_AES128__HMAC2__MODE_CTR_CFB \
(XLR_SEC_VECTOR_CIPHER_AES128 | XLR_SEC_VECTOR_HMAC2 | XLR_SEC_VECTOR_MODE_CTR_CFB)

#define XLR_SEC_VECTOR__CIPHER_AES128__HMAC2__MODE_ECB_CBC_OFB \
(XLR_SEC_VECTOR_CIPHER_AES128 | XLR_SEC_VECTOR_HMAC2 | XLR_SEC_VECTOR_MODE_ECB_CBC_OFB)

#define XLR_SEC_VECTOR__CIPHER_AES192__HMAC2__MODE_CTR_CFB \
(XLR_SEC_VECTOR_CIPHER_AES192 | XLR_SEC_VECTOR_HMAC2 | XLR_SEC_VECTOR_MODE_CTR_CFB)

#define XLR_SEC_VECTOR__CIPHER_AES192__HMAC2__MODE_ECB_CBC_OFB \
(XLR_SEC_VECTOR_CIPHER_AES192 | XLR_SEC_VECTOR_HMAC2 | XLR_SEC_VECTOR_MODE_ECB_CBC_OFB)

#define XLR_SEC_VECTOR__CIPHER_AES256__HMAC2__MODE_CTR_CFB \
(XLR_SEC_VECTOR_CIPHER_AES256 | XLR_SEC_VECTOR_HMAC2 | XLR_SEC_VECTOR_MODE_CTR_CFB)

#define XLR_SEC_VECTOR__CIPHER_AES256__HMAC2__MODE_ECB_CBC_OFB \
(XLR_SEC_VECTOR_CIPHER_AES256 | XLR_SEC_VECTOR_HMAC2 | XLR_SEC_VECTOR_MODE_ECB_CBC_OFB)

#define XLR_SEC_VECTOR__CIPHER_AES128__HMAC2__MODE_F8 \
(XLR_SEC_VECTOR_CIPHER_AES128 | XLR_SEC_VECTOR_HMAC2 | XLR_SEC_VECTOR_MODE_F8)

#define XLR_SEC_VECTOR__CIPHER_AES192__HMAC2__MODE_F8 \
(XLR_SEC_VECTOR_CIPHER_AES192 | XLR_SEC_VECTOR_HMAC2 | XLR_SEC_VECTOR_MODE_F8)

#define XLR_SEC_VECTOR__CIPHER_AES256__HMAC2__MODE_F8 \
(XLR_SEC_VECTOR_CIPHER_AES256 | XLR_SEC_VECTOR_HMAC2 | XLR_SEC_VECTOR_MODE_F8)

#define XLR_SEC_VECTOR_CIPHER_ARC4__GCM  \
(XLR_SEC_VECTOR_CIPHER_ARC4 | XLR_SEC_VECTOR_GCM)

#define XLR_SEC_VECTOR_CIPHER_ARC4__GCM__STATE  \
(XLR_SEC_VECTOR_CIPHER_ARC4 | XLR_SEC_VECTOR_GCM | XLR_SEC_VECTOR_STATE)

#define XLR_SEC_VECTOR__CIPHER_DES__GCM__MODE_ECB_CBC \
(XLR_SEC_VECTOR_CIPHER_DES | XLR_SEC_VECTOR_GCM | XLR_SEC_VECTOR_MODE_ECB_CBC)

#define XLR_SEC_VECTOR__CIPHER_3DES__GCM__MODE_ECB_CBC \
(XLR_SEC_VECTOR_CIPHER_3DES | XLR_SEC_VECTOR_GCM | XLR_SEC_VECTOR_MODE_ECB_CBC)

#define XLR_SEC_VECTOR__CIPHER_AES128__GCM__MODE_CTR_CFB \
(XLR_SEC_VECTOR_CIPHER_AES128 | XLR_SEC_VECTOR_GCM | XLR_SEC_VECTOR_MODE_CTR_CFB)

#define XLR_SEC_VECTOR__CIPHER_AES128__GCM__MODE_ECB_CBC_OFB \
(XLR_SEC_VECTOR_CIPHER_AES128 | XLR_SEC_VECTOR_GCM | XLR_SEC_VECTOR_MODE_ECB_CBC_OFB)

#define XLR_SEC_VECTOR__CIPHER_AES192__GCM__MODE_CTR_CFB \
(XLR_SEC_VECTOR_CIPHER_AES192 | XLR_SEC_VECTOR_GCM | XLR_SEC_VECTOR_MODE_CTR_CFB)

#define XLR_SEC_VECTOR__CIPHER_AES192__GCM__MODE_ECB_CBC_OFB \
(XLR_SEC_VECTOR_CIPHER_AES192 | XLR_SEC_VECTOR_GCM | XLR_SEC_VECTOR_MODE_ECB_CBC_OFB)

#define XLR_SEC_VECTOR__CIPHER_AES256__GCM__MODE_CTR_CFB \
(XLR_SEC_VECTOR_CIPHER_AES256 | XLR_SEC_VECTOR_GCM | XLR_SEC_VECTOR_MODE_CTR_CFB)

#define XLR_SEC_VECTOR__CIPHER_AES256__GCM__MODE_ECB_CBC_OFB \
(XLR_SEC_VECTOR_CIPHER_AES256 | XLR_SEC_VECTOR_GCM | XLR_SEC_VECTOR_MODE_ECB_CBC_OFB)

#define XLR_SEC_VECTOR__CIPHER_AES128__GCM__MODE_F8 \
(XLR_SEC_VECTOR_CIPHER_AES128 | XLR_SEC_VECTOR_GCM | XLR_SEC_VECTOR_MODE_F8)

#define XLR_SEC_VECTOR__CIPHER_AES192__GCM__MODE_F8 \
(XLR_SEC_VECTOR_CIPHER_AES192 | XLR_SEC_VECTOR_GCM | XLR_SEC_VECTOR_MODE_F8)

#define XLR_SEC_VECTOR__CIPHER_AES256__GCM__MODE_F8 \
(XLR_SEC_VECTOR_CIPHER_AES256 | XLR_SEC_VECTOR_GCM | XLR_SEC_VECTOR_MODE_F8)

#define XLR_SEC_VECTOR_CIPHER_ARC4__F9  \
(XLR_SEC_VECTOR_CIPHER_ARC4 | XLR_SEC_VECTOR_F9)

#define XLR_SEC_VECTOR_CIPHER_ARC4__F9__STATE  \
(XLR_SEC_VECTOR_CIPHER_ARC4 | XLR_SEC_VECTOR_F9 | XLR_SEC_VECTOR_STATE)

#define XLR_SEC_VECTOR__CIPHER_DES__F9__MODE_ECB_CBC \
(XLR_SEC_VECTOR_CIPHER_DES | XLR_SEC_VECTOR_F9 | XLR_SEC_VECTOR_MODE_ECB_CBC)

#define XLR_SEC_VECTOR__CIPHER_3DES__F9__MODE_ECB_CBC \
(XLR_SEC_VECTOR_CIPHER_3DES | XLR_SEC_VECTOR_F9 | XLR_SEC_VECTOR_MODE_ECB_CBC)

#define XLR_SEC_VECTOR__CIPHER_AES128__F9__MODE_CTR_CFB \
(XLR_SEC_VECTOR_CIPHER_AES128 | XLR_SEC_VECTOR_F9 | XLR_SEC_VECTOR_MODE_CTR_CFB)

#define XLR_SEC_VECTOR__CIPHER_AES128__F9__MODE_ECB_CBC_OFB \
(XLR_SEC_VECTOR_CIPHER_AES128 | XLR_SEC_VECTOR_F9 | XLR_SEC_VECTOR_MODE_ECB_CBC_OFB)

#define XLR_SEC_VECTOR__CIPHER_AES192__F9__MODE_CTR_CFB \
(XLR_SEC_VECTOR_CIPHER_AES192 | XLR_SEC_VECTOR_F9 | XLR_SEC_VECTOR_MODE_CTR_CFB)

#define XLR_SEC_VECTOR__CIPHER_AES192__F9__MODE_ECB_CBC_OFB \
(XLR_SEC_VECTOR_CIPHER_AES192 | XLR_SEC_VECTOR_F9 | XLR_SEC_VECTOR_MODE_ECB_CBC_OFB)

#define XLR_SEC_VECTOR__CIPHER_AES256__F9__MODE_CTR_CFB \
(XLR_SEC_VECTOR_CIPHER_AES256 | XLR_SEC_VECTOR_F9 | XLR_SEC_VECTOR_MODE_CTR_CFB)

#define XLR_SEC_VECTOR__CIPHER_AES256__F9__MODE_ECB_CBC_OFB \
(XLR_SEC_VECTOR_CIPHER_AES256 | XLR_SEC_VECTOR_F9 | XLR_SEC_VECTOR_MODE_ECB_CBC_OFB)

#define XLR_SEC_VECTOR__CIPHER_AES128__F9__MODE_F8 \
(XLR_SEC_VECTOR_CIPHER_AES128 | XLR_SEC_VECTOR_F9 | XLR_SEC_VECTOR_MODE_F8)

#define XLR_SEC_VECTOR__CIPHER_AES192__F9__MODE_F8 \
(XLR_SEC_VECTOR_CIPHER_AES192 | XLR_SEC_VECTOR_F9 | XLR_SEC_VECTOR_MODE_F8)

#define XLR_SEC_VECTOR__CIPHER_AES256__F9__MODE_F8 \
(XLR_SEC_VECTOR_CIPHER_AES256 | XLR_SEC_VECTOR_F9 | XLR_SEC_VECTOR_MODE_F8)

/*
 * Cipher Modes
 */
typedef enum {
	XLR_SEC_CIPHER_MODE_NONE = 0,
	XLR_SEC_CIPHER_MODE_PASS = 1,
	XLR_SEC_CIPHER_MODE_ECB,
	XLR_SEC_CIPHER_MODE_CBC,
	XLR_SEC_CIPHER_MODE_OFB,
	XLR_SEC_CIPHER_MODE_CTR,
	XLR_SEC_CIPHER_MODE_CFB,
	XLR_SEC_CIPHER_MODE_F8
}    XLR_SEC_CIPHER_MODE;

typedef enum {
	XLR_SEC_CIPHER_OP_NONE = 0,
	XLR_SEC_CIPHER_OP_ENCRYPT = 1,
	XLR_SEC_CIPHER_OP_DECRYPT
}    XLR_SEC_CIPHER_OP;

typedef enum {
	XLR_SEC_CIPHER_TYPE_UNSUPPORTED = -1,
	XLR_SEC_CIPHER_TYPE_NONE = 0,
	XLR_SEC_CIPHER_TYPE_DES,
	XLR_SEC_CIPHER_TYPE_3DES,
	XLR_SEC_CIPHER_TYPE_AES128,
	XLR_SEC_CIPHER_TYPE_AES192,
	XLR_SEC_CIPHER_TYPE_AES256,
	XLR_SEC_CIPHER_TYPE_ARC4,
	XLR_SEC_CIPHER_TYPE_KASUMI_F8
}    XLR_SEC_CIPHER_TYPE;

typedef enum {
	XLR_SEC_CIPHER_INIT_OK = 1,	/* Preserve old Keys */
	XLR_SEC_CIPHER_INIT_NK	/* Load new Keys */
}    XLR_SEC_CIPHER_INIT;


/*
 *  Hash Modes
 */
typedef enum {
	XLR_SEC_DIGEST_TYPE_UNSUPPORTED = -1,
	XLR_SEC_DIGEST_TYPE_NONE = 0,
	XLR_SEC_DIGEST_TYPE_MD5,
	XLR_SEC_DIGEST_TYPE_SHA1,
	XLR_SEC_DIGEST_TYPE_SHA256,
	XLR_SEC_DIGEST_TYPE_SHA384,
	XLR_SEC_DIGEST_TYPE_SHA512,
	XLR_SEC_DIGEST_TYPE_GCM,
	XLR_SEC_DIGEST_TYPE_KASUMI_F9,
	XLR_SEC_DIGEST_TYPE_HMAC_MD5,
	XLR_SEC_DIGEST_TYPE_HMAC_SHA1,
	XLR_SEC_DIGEST_TYPE_HMAC_SHA256,
	XLR_SEC_DIGEST_TYPE_HMAC_SHA384,
	XLR_SEC_DIGEST_TYPE_HMAC_SHA512,
	XLR_SEC_DIGEST_TYPE_HMAC_AES_CBC,
	XLR_SEC_DIGEST_TYPE_HMAC_AES_XCBC
}    XLR_SEC_DIGEST_TYPE;

typedef enum {
	XLR_SEC_DIGEST_INIT_OLDKEY = 1,	/* Preserve old key HMAC key stored in
					 * ID registers (moot if HASH.HMAC ==
					 * 0) */
	XLR_SEC_DIGEST_INIT_NEWKEY	/* Load new HMAC key from memory ctrl
					 * section to ID registers */
}    XLR_SEC_DIGEST_INIT;

typedef enum {
	XLR_SEC_DIGEST_SRC_DMA = 1,	/* DMA channel */
	XLR_SEC_DIGEST_SRC_CPHR	/* Cipher if word count exceeded
				 * Cipher_Offset; else DMA */
}    XLR_SEC_DIGEST_SRC;

/*
 *  Checksum Modes
 */
typedef enum {
	XLR_SEC_CKSUM_TYPE_NOP = 1,
	XLR_SEC_CKSUM_TYPE_IP
}    XLR_SEC_CKSUM_TYPE;

typedef enum {
	XLR_SEC_CKSUM_SRC_DMA = 1,
	XLR_SEC_CKSUM_SRC_CIPHER
}    XLR_SEC_CKSUM_SRC;

/*
 *  Packet Modes
 */
typedef enum {
	XLR_SEC_LOADHMACKEY_MODE_OLD = 1,
	XLR_SEC_LOADHMACKEY_MODE_LOAD
}    XLR_SEC_LOADHMACKEY_MODE;

typedef enum {
	XLR_SEC_PADHASH_PADDED = 1,
	XLR_SEC_PADHASH_PAD
}    XLR_SEC_PADHASH_MODE;

typedef enum {
	XLR_SEC_HASHBYTES_ALL8 = 1,
	XLR_SEC_HASHBYTES_MSB,
	XLR_SEC_HASHBYTES_MSW
}    XLR_SEC_HASHBYTES_MODE;

typedef enum {
	XLR_SEC_NEXT_FINISH = 1,
	XLR_SEC_NEXT_DO
}    XLR_SEC_NEXT_MODE;

typedef enum {
	XLR_SEC_PKT_IV_OLD = 1,
	XLR_SEC_PKT_IV_NEW
}    XLR_SEC_PKT_IV_MODE;

typedef enum {
	XLR_SEC_LASTWORD_128 = 1,
	XLR_SEC_LASTWORD_96MASK,
	XLR_SEC_LASTWORD_64MASK,
	XLR_SEC_LASTWORD_32MASK
}    XLR_SEC_LASTWORD_MODE;

typedef enum {
	XLR_SEC_CFB_MASK_REGULAR_CTR = 0,
	XLR_SEC_CFB_MASK_CCMP,
	XLR_SEC_CFB_MASK_GCM_WITH_SCI,
	XLR_SEC_CFB_MASK_GCM_WITHOUT_SCI
}    XLR_SEC_CFB_MASK_MODE;

/*
 *  Public Key
 */
typedef enum {
	RMIPK_BLKWIDTH_512 = 1,
	RMIPK_BLKWIDTH_1024
}    RMIPK_BLKWIDTH_MODE;

typedef enum {
	RMIPK_LDCONST_OLD = 1,
	RMIPK_LDCONST_NEW
}    RMIPK_LDCONST_MODE;


typedef struct xlr_sec_io_s {
	unsigned int command;
	unsigned int result_status;
	unsigned int flags;
	unsigned int session_num;
	unsigned int use_callback;
	unsigned int time_us;
	unsigned int user_context[2];	/* usable for anything by caller */
	unsigned int command_context;	/* Context (ID) of this command). */
	unsigned char initial_vector[XLR_SEC_MAX_IV_LENGTH];
	unsigned char crypt_key[XLR_SEC_MAX_CRYPT_KEY_LENGTH];
	unsigned char mac_key[XLR_SEC_MAX_AUTH_KEY_LENGTH];

	XLR_SEC_CIPHER_OP cipher_op;
	XLR_SEC_CIPHER_MODE cipher_mode;
	XLR_SEC_CIPHER_TYPE cipher_type;
	XLR_SEC_CIPHER_INIT cipher_init;
	unsigned int cipher_offset;

	XLR_SEC_DIGEST_TYPE digest_type;
	XLR_SEC_DIGEST_INIT digest_init;
	XLR_SEC_DIGEST_SRC digest_src;
	unsigned int digest_offset;

	XLR_SEC_CKSUM_TYPE cksum_type;
	XLR_SEC_CKSUM_SRC cksum_src;
	unsigned int cksum_offset;

	XLR_SEC_LOADHMACKEY_MODE pkt_hmac;
	XLR_SEC_PADHASH_MODE pkt_hash;
	XLR_SEC_HASHBYTES_MODE pkt_hashbytes;
	XLR_SEC_NEXT_MODE pkt_next;
	XLR_SEC_PKT_IV_MODE pkt_iv;
	XLR_SEC_LASTWORD_MODE pkt_lastword;

	unsigned int nonce;
	unsigned int cfb_mask;

	unsigned int iv_offset;
	unsigned short pad_type;
	unsigned short rc4_key_len;

	unsigned int num_packets;
	unsigned int num_fragments;

	uint64_t source_buf;
	unsigned int source_buf_size;
	uint64_t dest_buf;
	unsigned int dest_buf_size;

	uint64_t auth_dest;
	uint64_t cksum_dest;

	unsigned short rc4_loadstate;
	unsigned short rc4_savestate;
	uint64_t rc4_state;

}            xlr_sec_io_t, *xlr_sec_io_pt;


#define XLR_SEC_SESSION(sid)   ((sid) & 0x000007ff)
#define XLR_SEC_SID(crd,ses)   (((crd) << 28) | ((ses) & 0x7ff))

/*
 *  Length values for cryptography
 */
/*
#define XLR_SEC_DES_KEY_LENGTH     8
#define XLR_SEC_3DES_KEY_LENGTH        24
#define XLR_SEC_MAX_CRYPT_KEY_LENGTH   XLR_SEC_3DES_KEY_LENGTH
#define XLR_SEC_IV_LENGTH          8
#define XLR_SEC_AES_IV_LENGTH      16
#define XLR_SEC_MAX_IV_LENGTH      XLR_SEC_AES_IV_LENGTH
*/

#define SEC_MAX_FRAG_LEN 16000

struct xlr_sec_command {
	uint16_t session_num;
	struct cryptop *crp;
	struct cryptodesc *enccrd, *maccrd;

	xlr_sec_io_t op;
};
struct xlr_sec_session {
	uint32_t sessionid;
	int hs_used;
	int hs_mlen;
	struct xlr_sec_command cmd;
	void *desc_ptr;
	uint8_t multi_frag_flag;
};

/*
 * Holds data specific to rmi security accelerators
 */
struct xlr_sec_softc {
	device_t sc_dev;	/* device backpointer */
	struct mtx sc_mtx;	/* per-instance lock */

	int32_t sc_cid;
	struct xlr_sec_session *sc_sessions;
	int sc_nsessions;
	xlr_reg_t *mmio;
};


/*

union xlr_sec_operand_t {
        struct mbuf *m;
        struct uio *io;
        void *buf;
}xlr_sec_operand;
*/





/* this is passed to packet setup to optimize */
#define XLR_SEC_SETUP_OP_CIPHER              0x00000001
#define XLR_SEC_SETUP_OP_HMAC                0x00000002
#define XLR_SEC_SETUP_OP_CIPHER_HMAC         (XLR_SEC_SETUP_OP_CIPHER | XLR_SEC_SETUP_OP_HMAC)
/* this is passed to control_setup to update w/preserving existing keys */
#define XLR_SEC_SETUP_OP_PRESERVE_HMAC_KEY    0x80000000
#define XLR_SEC_SETUP_OP_PRESERVE_CIPHER_KEY  0x40000000
#define XLR_SEC_SETUP_OP_UPDATE_KEYS          0x00000010
#define XLR_SEC_SETUP_OP_FLIP_3DES_KEY        0x00000020





/*
 *   Message Ring Specifics
 */

#define SEC_MSGRING_WORDSIZE      2


/*
 *
 *
 * rwR      31  30 29     27 26    24 23      21 20     18
 *         |  NA  | RSA0Out | Rsa0In | Pipe3Out | Pipe3In | ...
 *
 *          17       15 14     12 11      9 8       6 5        3 2       0
 *         |  Pipe2Out | Pipe2In | Pipe1In | Pipe1In | Pipe0Out | Pipe0In |
 *
 * DMA CREDIT REG -
 *   NUMBER OF CREDITS PER PIPE
 */

#define SEC_DMA_CREDIT_RSA0_OUT_FOUR   0x20000000
#define SEC_DMA_CREDIT_RSA0_OUT_TWO    0x10000000
#define SEC_DMA_CREDIT_RSA0_OUT_ONE    0x08000000

#define SEC_DMA_CREDIT_RSA0_IN_FOUR    0x04000000
#define SEC_DMA_CREDIT_RSA0_IN_TWO     0x02000000
#define SEC_DMA_CREDIT_RSA0_IN_ONE     0x01000000

#define SEC_DMA_CREDIT_PIPE3_OUT_FOUR  0x00800000
#define SEC_DMA_CREDIT_PIPE3_OUT_TWO   0x00400000
#define SEC_DMA_CREDIT_PIPE3_OUT_ONE   0x00200000

#define SEC_DMA_CREDIT_PIPE3_IN_FOUR   0x00100000
#define SEC_DMA_CREDIT_PIPE3_IN_TWO    0x00080000
#define SEC_DMA_CREDIT_PIPE3_IN_ONE    0x00040000

#define SEC_DMA_CREDIT_PIPE2_OUT_FOUR  0x00020000
#define SEC_DMA_CREDIT_PIPE2_OUT_TWO   0x00010000
#define SEC_DMA_CREDIT_PIPE2_OUT_ONE   0x00008000

#define SEC_DMA_CREDIT_PIPE2_IN_FOUR   0x00004000
#define SEC_DMA_CREDIT_PIPE2_IN_TWO    0x00002000
#define SEC_DMA_CREDIT_PIPE2_IN_ONE    0x00001000

#define SEC_DMA_CREDIT_PIPE1_OUT_FOUR  0x00000800
#define SEC_DMA_CREDIT_PIPE1_OUT_TWO   0x00000400
#define SEC_DMA_CREDIT_PIPE1_OUT_ONE   0x00000200

#define SEC_DMA_CREDIT_PIPE1_IN_FOUR   0x00000100
#define SEC_DMA_CREDIT_PIPE1_IN_TWO    0x00000080
#define SEC_DMA_CREDIT_PIPE1_IN_ONE    0x00000040

#define SEC_DMA_CREDIT_PIPE0_OUT_FOUR  0x00000020
#define SEC_DMA_CREDIT_PIPE0_OUT_TWO   0x00000010
#define SEC_DMA_CREDIT_PIPE0_OUT_ONE   0x00000008

#define SEC_DMA_CREDIT_PIPE0_IN_FOUR   0x00000004
#define SEC_DMA_CREDIT_PIPE0_IN_TWO    0x00000002
#define SEC_DMA_CREDIT_PIPE0_IN_ONE    0x00000001


/*
 *  Currently, FOUR credits per PIPE
 *  0x24924924
 */
#define SEC_DMA_CREDIT_CONFIG          SEC_DMA_CREDIT_RSA0_OUT_FOUR | \
                                       SEC_DMA_CREDIT_RSA0_IN_FOUR | \
                                       SEC_DMA_CREDIT_PIPE3_OUT_FOUR | \
                                       SEC_DMA_CREDIT_PIPE3_IN_FOUR | \
                                       SEC_DMA_CREDIT_PIPE2_OUT_FOUR | \
                                       SEC_DMA_CREDIT_PIPE2_IN_FOUR | \
                                       SEC_DMA_CREDIT_PIPE1_OUT_FOUR | \
                                       SEC_DMA_CREDIT_PIPE1_IN_FOUR | \
                                       SEC_DMA_CREDIT_PIPE0_OUT_FOUR | \
                                       SEC_DMA_CREDIT_PIPE0_IN_FOUR




/*
 * CONFIG2
 *    31   5         4                   3
 *   |  NA  | PIPE3_DEF_DBL_ISS | PIPE2_DEF_DBL_ISS | ...
 *
 *                 2                   1                   0
 *   ... | PIPE1_DEF_DBL_ISS | PIPE0_DEF_DBL_ISS | ROUND_ROBIN_MODE |
 *
 *  DBL_ISS - mode for SECENG and DMA controller which slows down transfers
 *             (to be conservativei; 0=Disable,1=Enable).
 *  ROUND_ROBIN - mode where SECENG dispatches operations to PIPE0-PIPE3
 *                and all messages are sent to PIPE0.
 *
 */

#define SEC_CFG2_PIPE3_DBL_ISS_ON      0x00000010
#define SEC_CFG2_PIPE3_DBL_ISS_OFF     0x00000000
#define SEC_CFG2_PIPE2_DBL_ISS_ON      0x00000008
#define SEC_CFG2_PIPE2_DBL_ISS_OFF     0x00000000
#define SEC_CFG2_PIPE1_DBL_ISS_ON      0x00000004
#define SEC_CFG2_PIPE1_DBL_ISS_OFF     0x00000000
#define SEC_CFG2_PIPE0_DBL_ISS_ON      0x00000002
#define SEC_CFG2_PIPE0_DBL_ISS_OFF     0x00000000
#define SEC_CFG2_ROUND_ROBIN_ON        0x00000001
#define SEC_CFG2_ROUND_ROBIN_OFF       0x00000000


enum sec_pipe_config {

	SEC_PIPE_CIPHER_KEY0_L0 = 0x00,
	SEC_PIPE_CIPHER_KEY0_HI,
	SEC_PIPE_CIPHER_KEY1_LO,
	SEC_PIPE_CIPHER_KEY1_HI,
	SEC_PIPE_CIPHER_KEY2_LO,
	SEC_PIPE_CIPHER_KEY2_HI,
	SEC_PIPE_CIPHER_KEY3_LO,
	SEC_PIPE_CIPHER_KEY3_HI,
	SEC_PIPE_HMAC_KEY0_LO,
	SEC_PIPE_HMAC_KEY0_HI,
	SEC_PIPE_HMAC_KEY1_LO,
	SEC_PIPE_HMAC_KEY1_HI,
	SEC_PIPE_HMAC_KEY2_LO,
	SEC_PIPE_HMAC_KEY2_HI,
	SEC_PIPE_HMAC_KEY3_LO,
	SEC_PIPE_HMAC_KEY3_HI,
	SEC_PIPE_HMAC_KEY4_LO,
	SEC_PIPE_HMAC_KEY4_HI,
	SEC_PIPE_HMAC_KEY5_LO,
	SEC_PIPE_HMAC_KEY5_HI,
	SEC_PIPE_HMAC_KEY6_LO,
	SEC_PIPE_HMAC_KEY6_HI,
	SEC_PIPE_HMAC_KEY7_LO,
	SEC_PIPE_HMAC_KEY7_HI,
	SEC_PIPE_NCFBM_LO,
	SEC_PIPE_NCFBM_HI,
	SEC_PIPE_INSTR_LO,
	SEC_PIPE_INSTR_HI,
	SEC_PIPE_RSVD0,
	SEC_PIPE_RSVD1,
	SEC_PIPE_RSVD2,
	SEC_PIPE_RSVD3,

	SEC_PIPE_DF_PTRS0,
	SEC_PIPE_DF_PTRS1,
	SEC_PIPE_DF_PTRS2,
	SEC_PIPE_DF_PTRS3,
	SEC_PIPE_DF_PTRS4,
	SEC_PIPE_DF_PTRS5,
	SEC_PIPE_DF_PTRS6,
	SEC_PIPE_DF_PTRS7,

	SEC_PIPE_DU_DATA_IN_LO,
	SEC_PIPE_DU_DATA_IN_HI,
	SEC_PIPE_DU_DATA_IN_CTRL,
	SEC_PIPE_DU_DATA_OUT_LO,
	SEC_PIPE_DU_DATA_OUT_HI,
	SEC_PIPE_DU_DATA_OUT_CTRL,

	SEC_PIPE_STATE0,
	SEC_PIPE_STATE1,
	SEC_PIPE_STATE2,
	SEC_PIPE_STATE3,
	SEC_PIPE_STATE4,
	SEC_PIPE_INCLUDE_MASK0,
	SEC_PIPE_INCLUDE_MASK1,
	SEC_PIPE_INCLUDE_MASK2,
	SEC_PIPE_INCLUDE_MASK3,
	SEC_PIPE_INCLUDE_MASK4,
	SEC_PIPE_EXCLUDE_MASK0,
	SEC_PIPE_EXCLUDE_MASK1,
	SEC_PIPE_EXCLUDE_MASK2,
	SEC_PIPE_EXCLUDE_MASK3,
	SEC_PIPE_EXCLUDE_MASK4,
};


enum sec_pipe_base_config {

	SEC_PIPE0_BASE = 0x00,
	SEC_PIPE1_BASE = 0x40,
	SEC_PIPE2_BASE = 0x80,
	SEC_PIPE3_BASE = 0xc0

};

enum sec_rsa_config {

	SEC_RSA_PIPE0_DU_DATA_IN_LO = 0x100,
	SEC_RSA_PIPE0_DU_DATA_IN_HI,
	SEC_RSA_PIPE0_DU_DATA_IN_CTRL,
	SEC_RSA_PIPE0_DU_DATA_OUT_LO,
	SEC_RSA_PIPE0_DU_DATA_OUT_HI,
	SEC_RSA_PIPE0_DU_DATA_OUT_CTRL,
	SEC_RSA_RSVD0,
	SEC_RSA_RSVD1,

	SEC_RSA_PIPE0_STATE0,
	SEC_RSA_PIPE0_STATE1,
	SEC_RSA_PIPE0_STATE2,
	SEC_RSA_PIPE0_INCLUDE_MASK0,
	SEC_RSA_PIPE0_INCLUDE_MASK1,
	SEC_RSA_PIPE0_INCLUDE_MASK2,
	SEC_RSA_PIPE0_EXCLUDE_MASK0,
	SEC_RSA_PIPE0_EXCLUDE_MASK1,
	SEC_RSA_PIPE0_EXCLUDE_MASK2,
	SEC_RSA_PIPE0_EVENT_CTR

};




enum sec_config {

	SEC_DMA_CREDIT = 0x140,
	SEC_CONFIG1,
	SEC_CONFIG2,
	SEC_CONFIG3,

};



enum sec_debug_config {

	SEC_DW0_DESCRIPTOR0_LO = 0x180,
	SEC_DW0_DESCRIPTOR0_HI,
	SEC_DW0_DESCRIPTOR1_LO,
	SEC_DW0_DESCRIPTOR1_HI,
	SEC_DW1_DESCRIPTOR0_LO,
	SEC_DW1_DESCRIPTOR0_HI,
	SEC_DW1_DESCRIPTOR1_LO,
	SEC_DW1_DESCRIPTOR1_HI,
	SEC_DW2_DESCRIPTOR0_LO,
	SEC_DW2_DESCRIPTOR0_HI,
	SEC_DW2_DESCRIPTOR1_LO,
	SEC_DW2_DESCRIPTOR1_HI,
	SEC_DW3_DESCRIPTOR0_LO,
	SEC_DW3_DESCRIPTOR0_HI,
	SEC_DW3_DESCRIPTOR1_LO,
	SEC_DW3_DESCRIPTOR1_HI,

	SEC_STATE0,
	SEC_STATE1,
	SEC_STATE2,
	SEC_INCLUDE_MASK0,
	SEC_INCLUDE_MASK1,
	SEC_INCLUDE_MASK2,
	SEC_EXCLUDE_MASK0,
	SEC_EXCLUDE_MASK1,
	SEC_EXCLUDE_MASK2,
	SEC_EVENT_CTR

};


enum sec_msgring_bucket_config {

	SEC_BIU_CREDITS = 0x308,

	SEC_MSG_BUCKET0_SIZE = 0x320,
	SEC_MSG_BUCKET1_SIZE,
	SEC_MSG_BUCKET2_SIZE,
	SEC_MSG_BUCKET3_SIZE,
	SEC_MSG_BUCKET4_SIZE,
	SEC_MSG_BUCKET5_SIZE,
	SEC_MSG_BUCKET6_SIZE,
	SEC_MSG_BUCKET7_SIZE,
};

enum sec_msgring_credit_config {

	SEC_CC_CPU0_0 = 0x380,
	SEC_CC_CPU1_0 = 0x388,
	SEC_CC_CPU2_0 = 0x390,
	SEC_CC_CPU3_0 = 0x398,
	SEC_CC_CPU4_0 = 0x3a0,
	SEC_CC_CPU5_0 = 0x3a8,
	SEC_CC_CPU6_0 = 0x3b0,
	SEC_CC_CPU7_0 = 0x3b8

};

enum sec_engine_id {
	SEC_PIPE0,
	SEC_PIPE1,
	SEC_PIPE2,
	SEC_PIPE3,
	SEC_RSA
};

enum sec_cipher {
	SEC_AES256_MODE_HMAC,
	SEC_AES256_MODE,
	SEC_AES256_HMAC,
	SEC_AES256,
	SEC_AES192_MODE_HMAC,
	SEC_AES192_MODE,
	SEC_AES192_HMAC,
	SEC_AES192,
	SEC_AES128_MODE_HMAC,
	SEC_AES128_MODE,
	SEC_AES128_HMAC,
	SEC_AES128,
	SEC_DES_HMAC,
	SEC_DES,
	SEC_3DES,
	SEC_3DES_HMAC,
	SEC_HMAC
};

enum sec_msgrng_msg_ctrl_config {
	SEC_EOP = 5,
	SEC_SOP = 6,
};



void 
xlr_sec_init(struct xlr_sec_softc *sc);

int 
xlr_sec_setup(struct xlr_sec_session *ses,
    struct xlr_sec_command *cmd, symkey_desc_pt desc);

symkey_desc_pt xlr_sec_allocate_desc(void *);

#endif
