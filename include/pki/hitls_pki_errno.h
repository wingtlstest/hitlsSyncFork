/*
 * This file is part of the openHiTLS project.
 *
 * openHiTLS is licensed under the Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *     http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 */

#ifndef HITLS_PKI_ERRNO_H
#define HITLS_PKI_ERRNO_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HITLS_X509_SUCCESS,
    HITLS_X509_ERR_TIME_EXPIRED = 0x04000001,
    HITLS_X509_ERR_TIME_FUTURE,
    HITLS_X509_ERR_VFY_KU_NO_CERTSIGN,
    HITLS_X509_ERR_VFY_KU_NO_CRLSIGN,
    HITLS_X509_ERR_VFY_SIGNALG_NOT_MATCH,
    HITLS_X509_ERR_INVALID_PARAM,
    HITLS_X509_ERR_VFY_CHECK_SECBITS,
    HITLS_X509_ERR_VFY_CERT_REVOKED,
    HITLS_X509_ERR_VFY_GET_HASHID,
    HITLS_X509_ERR_VFY_GET_SIGNID,
    HITLS_X509_ERR_VFY_DUP_PUBKEY,
    HITLS_X509_ERR_CERT_CHAIN_COUNT_IS0,
    HITLS_X509_ERR_ISSUE_CERT_NOT_FOUND,
    HITLS_X509_ERR_ROOT_CERT_NOT_FOUND,
    HITLS_X509_ERR_CHAIN_DEPTH_UP_LIMIT,
    HITLS_X509_ERR_VFY_AKI_SKI_NOT_MATCH,

    HITLS_X509_ERR_CERT_NOT_CA = 0x04010001,
    HITLS_X509_ERR_CERT_EXIST,
    HITLS_X509_ERR_CERT_START_TIME_LATER,
    HITLS_X509_ERR_PROCESS_CRITICALEXT,
    HITLS_X509_ERR_CERT_INVALID_DN,
    HITLS_X509_ERR_CERT_INVALID_SERIAL_NUM,
    HITLS_X509_ERR_CERT_INVALID_TIME,
    HITLS_X509_ERR_CERT_INVALID_PUBKEY,
    HITLS_X509_ERR_CERT_INACCURACY_VERSION,
    HITLS_X509_ERR_CERT_NOT_SIGNED,
    HITLS_X509_ERR_CERT_SIGN_ALG,
    HITLS_X509_ERR_CERT_DUP_FAIL,

    HITLS_X509_ERR_CRL_EXIST = 0x04020001,
    HITLS_X509_ERR_CRL_NOT_FOUND,
    HITLS_X509_ERR_CRL_INACCURACY_VERSION,
    HITLS_X509_ERR_CRL_ENTRY,
    HITLS_X509_ERR_CRL_THISUPDATE_UNEXIST,
	HITLS_X509_ERR_CRL_NEXTUPDATE_UNEXIST,
    HITLS_X509_ERR_CRL_REVOKELIST_UNEXIST,
    HITLS_X509_ERR_CRL_ISSUER_EMPTY,
    HITLS_X509_ERR_CRL_TIME_INVALID,
    HITLS_X509_ERR_CRL_NOT_SIGNED,

    HITLS_X509_ERR_NOT_SUPPORT_FORMAT = 0x04030001,
    HITLS_X509_ERR_ALG_OID,
    HITLS_X509_ERR_NAME_OID,
    HITLS_X509_ERR_PARSE_STR,
    HITLS_X509_ERR_CHECK_TAG,
    HITLS_X509_ERR_GET_ANY_TAG,
    HITLS_X509_ERR_PARSE_NO_ELEMENT,
    HITLS_X509_ERR_PARSE_NO_ENOUGH,
    HITLS_X509_ERR_HASHID,
    HITLS_X509_ERR_SET_DNNAME_UNKKOWN,
    HITLS_X509_ERR_SET_DNNAME_TOOMUCH,
    HITLS_X509_ERR_SET_DNNAME_INVALID_LEN,
    HITLS_X509_ERR_SET_KEY,
    HITLS_X509_ERR_SIGN_PARAM,
    HITLS_X509_ERR_MD_NOT_MATCH,
    HITLS_X509_ERR_MGF_NOT_MATCH,
    HITLS_X509_ERR_PSS_SALTLEN,
    HITLS_X509_ERR_ENCODE_SIGNID,
    HITLS_X509_ERR_PARSE_OBJ_ID,
    HITLS_X509_ERR_PARSE_ATTR_BUF,
    HITLS_X509_ERR_SET_ATTR_REPEAT,
    HITLS_X509_ERR_SET_AFTER_PARSE,
    HITLS_X509_ERR_SET_NAME_LIST,
    HITLS_X509_ERR_SORT_NAME_NODE,
    HITLS_X509_ERR_ATTR_NOT_FOUND,
    HITLS_X509_ERR_SIGN_AFTER_PARSE,

    /* extensions */
    HITLS_X509_ERR_EXT_NOT_FOUND = 0x04040001,
    HITLS_X509_ERR_EXT_NOT_SUPPORT,
    HITLS_X509_ERR_EXT_PARSE_AFTER_SET,
    HITLS_X509_ERR_EXT_SET_AFTER_PARSE,
    HITLS_X509_ERR_EXT_SET,
    HITLS_X509_ERR_EXT_KU,
    HITLS_X509_ERR_EXT_OID,
    HITLS_X509_ERR_EXT_KID,
    HITLS_X509_ERR_EXT_SAN,
    HITLS_X509_ERR_EXT_SAN_ELE,
    HITLS_X509_ERR_EXT_EXTENDED_KU,
    HITLS_X509_ERR_EXT_EXTENDED_KU_ELE,
    HITLS_X509_ERR_EXT_GN_UNSUPPORT,
    HITLS_X509_ERR_EXT_CRLNUMBER,
    HITLS_X509_ERR_PARSE_EXT_KU,
    HITLS_X509_ERR_PARSE_EXT_BUF,
    HITLS_X509_ERR_PARSE_EXT_REPEAT,
    HITLS_X509_ERR_PARSE_AKI,
    HITLS_X509_ERR_PARSE_SAN,
    HITLS_X509_ERR_PARSE_SAN_ITEM_UNKNOW,
    HITLS_X509_ERR_PARSE_EXKU,
    HITLS_X509_ERR_PARSE_EXKU_ITEM,

    HITLS_X509_ERR_CSR_INVALID_PUBKEY = 0x04050001,
    HITLS_X509_ERR_CSR_INVALID_SUBJECT_DN,
    HITLS_X509_ERR_CSR_NOT_SIGNED,

    HITLS_CMS_ERR_NULL_POINTER = 0x04060001,
    HITLS_CMS_ERR_INVALID_DATA,
    HITLS_CMS_ERR_INVALID_ALGO,
    HITLS_CMS_ERR_PARSE_TYPE,

    HITLS_PKCS12_ERR_NULL_POINTER = 0x04070001,
    HITLS_PKCS12_ERR_INVALID_PARAM,
    HITLS_PKCS12_ERR_INVALID_PFX,

    HITLS_PKCS12_ERR_INVALID_ALGO,
    HITLS_PKCS12_ERR_PARSE_TYPE,
    HITLS_PKCS12_ERR_VERIFY_FAIL,
    HITLS_PKCS12_ERR_INVALID_CONTENTINFO,
    HITLS_PKCS12_ERR_INVALID_SAFEBAG_TYPE,
    HITLS_PKCS12_ERR_INVALID_SAFEBAG_ATTRIBUTES,
    HITLS_PKCS12_ERR_INVALID_CERTYPES,
    HITLS_PKCS12_ERR_INVALID_PASSWORD,
    HITLS_PKCS12_ERR_INVALID_SALTLEN,
    HITLS_PKCS12_ERR_INVALID_ITERATION,
    HITLS_PKCS12_ERR_NO_ENTITYKEY,
    HITLS_PKCS12_ERR_NO_ENTITYCERT,
    HITLS_PKCS12_ERR_NOT_SUPPORT_FORMAT,
    HITLS_PKCS12_ERR_NONE_DATA,
    HITLS_PKCS12_ERR_NO_PAIRED_CERT_AND_KEY,
    HITLS_PKCS12_ERR_KDF_TOO_LONG_INPUT,
    HITLS_PKCS12_ERR_REPEATED_SET_ENTITYCERT,
    HITLS_PKCS12_ERR_REPEATED_SET_KEY,
} HITLS_X509_ERRNO;

#ifdef __cplusplus
}
#endif

#endif // HITLS_PKI_ERRNO_H