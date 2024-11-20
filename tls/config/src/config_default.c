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

#include "hitls_build.h"
#include "bsl_sal.h"
#include "tls_binlog_id.h"
#include "hitls_type.h"
#include "hitls_crypt_type.h"
#include "hitls_config.h"
#include "hitls_error.h"
#include "tls_config.h"
#include "config.h"
#include "cipher_suite.h"
#include "cert_mgr.h"
#ifdef HITLS_TLS_FEATURE_SESSION
#include "session_mgr.h"
#endif
#ifdef HITLS_TLS_FEATURE_SECURITY
#include "security.h"
#endif

#ifdef HITLS_TLS_PROTO_TLCP11
uint16_t g_tlcpCipherSuites[] = {
    HITLS_ECDHE_SM4_CBC_SM3,
    HITLS_ECC_SM4_CBC_SM3,
};
#endif

uint16_t g_tls12CipherSuites[] = {
    HITLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,
    HITLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384,
    HITLS_DHE_DSS_WITH_AES_256_GCM_SHA384,
    HITLS_DHE_RSA_WITH_AES_256_GCM_SHA384,
    HITLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256,
    HITLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256,
    HITLS_DHE_RSA_WITH_CHACHA20_POLY1305_SHA256,
    HITLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
    HITLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,
    HITLS_DHE_DSS_WITH_AES_128_GCM_SHA256,
    HITLS_DHE_RSA_WITH_AES_128_GCM_SHA256,
    HITLS_ECDHE_ECDSA_WITH_AES_128_CCM,
    HITLS_ECDHE_ECDSA_WITH_AES_256_CCM,
    HITLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384,
    HITLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384,
    HITLS_DHE_RSA_WITH_AES_128_CCM,
    HITLS_DHE_RSA_WITH_AES_256_CCM,
    HITLS_DHE_RSA_WITH_AES_256_CBC_SHA256,
    HITLS_DHE_DSS_WITH_AES_256_CBC_SHA256,
    HITLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256,
    HITLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256,
    HITLS_DHE_RSA_WITH_AES_128_CBC_SHA256,
    HITLS_DHE_DSS_WITH_AES_128_CBC_SHA256,
    HITLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA,
    HITLS_ECDHE_RSA_WITH_AES_256_CBC_SHA,
    HITLS_DHE_RSA_WITH_AES_256_CBC_SHA,
    HITLS_DHE_DSS_WITH_AES_256_CBC_SHA,
    HITLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA,
    HITLS_ECDHE_RSA_WITH_AES_128_CBC_SHA,
    HITLS_DHE_RSA_WITH_AES_128_CBC_SHA,
    HITLS_DHE_DSS_WITH_AES_128_CBC_SHA,
    HITLS_RSA_WITH_AES_256_GCM_SHA384,
    HITLS_RSA_WITH_AES_128_GCM_SHA256,
    HITLS_RSA_WITH_AES_256_CBC_SHA256,
    HITLS_RSA_WITH_AES_128_CBC_SHA256,
    HITLS_RSA_WITH_AES_256_CBC_SHA,
    HITLS_RSA_WITH_AES_128_CBC_SHA,
};

int32_t SetDefaultCipherSuite(HITLS_Config *config, const uint16_t *cipherSuites, uint32_t cipherSuiteSize)
{
    BSL_SAL_FREE(config->cipherSuites);
    config->cipherSuites = BSL_SAL_Dump(cipherSuites, cipherSuiteSize);
    if (config->cipherSuites == NULL) {
        BSL_LOG_BINLOG_FIXLEN(BINLOG_ID16563, BSL_LOG_LEVEL_ERR, BSL_LOG_BINLOG_TYPE_RUN, "Dump fail", 0, 0, 0, 0);
        return HITLS_MEMALLOC_FAIL;
    }

    config->cipherSuitesSize = cipherSuiteSize / sizeof(uint16_t);
    return HITLS_SUCCESS;
}

#ifdef HITLS_TLS_PROTO_TLS13
static int32_t SetTLS13DefaultCipherSuites(HITLS_Config *config)
{
    const uint16_t ciphersuites13[] = {
        HITLS_AES_256_GCM_SHA384,
        HITLS_CHACHA20_POLY1305_SHA256,
        HITLS_AES_128_GCM_SHA256,
    };

    BSL_SAL_FREE(config->tls13CipherSuites);
    config->tls13CipherSuites = BSL_SAL_Dump(ciphersuites13, sizeof(ciphersuites13));
    if (config->tls13CipherSuites == NULL) {
        BSL_LOG_BINLOG_FIXLEN(BINLOG_ID16564, BSL_LOG_LEVEL_ERR, BSL_LOG_BINLOG_TYPE_RUN, "Dump fail", 0, 0, 0, 0);
        return HITLS_MEMALLOC_FAIL;
    }

    config->tls13cipherSuitesSize = sizeof(ciphersuites13) / sizeof(uint16_t);
    return HITLS_SUCCESS;
}
#endif
static int32_t SetDefaultPointFormats(HITLS_Config *config)
{
    const uint8_t pointFormats[] = {HITLS_POINT_FORMAT_UNCOMPRESSED};
    uint32_t size = sizeof(pointFormats);

    BSL_SAL_FREE(config->pointFormats);
    config->pointFormats = BSL_SAL_Dump(pointFormats, size);
    if (config->pointFormats == NULL) {
        BSL_LOG_BINLOG_FIXLEN(BINLOG_ID16565, BSL_LOG_LEVEL_ERR, BSL_LOG_BINLOG_TYPE_RUN, "Dump fail", 0, 0, 0, 0);
        return HITLS_MEMALLOC_FAIL;
    }
    config->pointFormatsSize = size / sizeof(uint8_t);

    return HITLS_SUCCESS;
}

static int32_t SetDefaultGroups(HITLS_Config *config)
{
    const uint16_t groupsTls[] = {
        HITLS_EC_GROUP_CURVE25519,
        HITLS_EC_GROUP_SECP521R1,
        HITLS_EC_GROUP_SECP384R1,
        HITLS_EC_GROUP_SECP256R1,
        HITLS_EC_GROUP_BRAINPOOLP512R1,
        HITLS_EC_GROUP_BRAINPOOLP384R1,
        HITLS_EC_GROUP_BRAINPOOLP256R1,
    };
    const uint16_t groupsTlcp[] = {
        HITLS_EC_GROUP_SM2,
    };

    const uint16_t *groups = groupsTls;
    uint32_t size = sizeof(groupsTls);
    if (config->maxVersion == HITLS_VERSION_TLCP11) {
        groups = groupsTlcp;
        size = sizeof(groupsTlcp);
    }

    BSL_SAL_FREE(config->groups);
    config->groups = BSL_SAL_Dump(groups, size);
    if (config->groups == NULL) {
        BSL_LOG_BINLOG_FIXLEN(BINLOG_ID16566, BSL_LOG_LEVEL_ERR, BSL_LOG_BINLOG_TYPE_RUN, "Dump fail", 0, 0, 0, 0);
        return HITLS_MEMALLOC_FAIL;
    }
    config->groupsSize = size / sizeof(uint16_t);

    return HITLS_SUCCESS;
}
#ifdef HITLS_TLS_PROTO_TLS13
static int32_t SetDefaultTLS13Groups(HITLS_Config *config)
{
    /* rfc8446 4.2.7 Supported Groups */
    const uint16_t groupsTls[] = {
        HITLS_EC_GROUP_CURVE25519,
        HITLS_EC_GROUP_SECP521R1,
        HITLS_EC_GROUP_SECP384R1,
        HITLS_EC_GROUP_SECP256R1,
        HITLS_FF_DHE_2048,
        HITLS_FF_DHE_3072,
        HITLS_FF_DHE_4096,
        HITLS_FF_DHE_6144,
        HITLS_FF_DHE_8192,
    };

    BSL_SAL_FREE(config->groups);
    config->groups = BSL_SAL_Dump(groupsTls, sizeof(groupsTls));
    if (config->groups == NULL) {
        BSL_LOG_BINLOG_FIXLEN(BINLOG_ID16567, BSL_LOG_LEVEL_ERR, BSL_LOG_BINLOG_TYPE_RUN, "Dump fail", 0, 0, 0, 0);
        return HITLS_MEMALLOC_FAIL;
    }
    config->groupsSize = sizeof(groupsTls) / sizeof(uint16_t);

    return HITLS_SUCCESS;
}
#endif
static int32_t SetDefaultSignHashAlg(HITLS_Config *config)
{
    uint32_t listLen = 0;
#ifdef HITLS_TLS_PROTO_TLCP11
    const SignSchemeInfo *signHashAlgList = (config->maxVersion != HITLS_VERSION_TLCP11) ?
        CFG_GetSignSchemeList(&listLen) :
        CFG_GetSignSchemeListTlcp(&listLen);
#else
    const SignSchemeInfo *signHashAlgList = CFG_GetSignSchemeList(&listLen);
#endif
    if (listLen == 0) {
        config->signAlgorithmsSize = 0;
        return HITLS_SUCCESS;
    }
    config->signAlgorithms = BSL_SAL_Calloc(1u, listLen * sizeof(uint16_t));
    if (config->signAlgorithms == NULL) {
        BSL_LOG_BINLOG_FIXLEN(BINLOG_ID16568, BSL_LOG_LEVEL_ERR, BSL_LOG_BINLOG_TYPE_RUN, "Dump fail", 0, 0, 0, 0);
        return HITLS_MEMALLOC_FAIL;
    }
    for (uint32_t i = 0; i < listLen; i++) {
        config->signAlgorithms[i] = signHashAlgList[i].scheme;
    }
    config->signAlgorithmsSize = listLen;

    return HITLS_SUCCESS;
}
#ifdef HITLS_TLS_PROTO_TLS13
static int32_t SetTLS13DefaultSignScheme(HITLS_Config *config)
{
    uint32_t listSize = 0;
    uint32_t validNum = 0;
    const SignSchemeInfo *signHashAlgList = CFG_GetSignSchemeList(&listSize);

    config->signAlgorithms = BSL_SAL_Calloc(listSize, sizeof(uint16_t));
    if (config->signAlgorithms == NULL) {
        BSL_LOG_BINLOG_FIXLEN(BINLOG_ID16569, BSL_LOG_LEVEL_ERR, BSL_LOG_BINLOG_TYPE_RUN, "Calloc fail", 0, 0, 0, 0);
        return HITLS_MEMALLOC_FAIL;
    }
    for (uint32_t i = 0; i < listSize; i++) {
        /* rfc8446 4.2.3 These algorithms are deprecated as of
        TLS 1.3.  They MUST NOT be offered or negotiated by any
        implementation.  In particular, MD5 [SLOTH], SHA-224, and DSA
        MUST NOT be used. */
        if ((signHashAlgList[i].signAlg == HITLS_SIGN_DSA) || (signHashAlgList[i].hashAlg == HITLS_HASH_SHA1) ||
            (signHashAlgList[i].hashAlg == HITLS_HASH_SHA_224)) {
            continue;
        }
        config->signAlgorithms[validNum] = signHashAlgList[i].scheme;
        validNum++;
    }
    config->signAlgorithmsSize = validNum;

    return HITLS_SUCCESS;
}
#endif

static void BasicInitConfig(HITLS_Config *config)
{
    config->isSupportExtendMasterSecret = false;
    config->emptyRecordsNum = HITLS_MAX_EMPTY_RECORDS;
#if defined(HITLS_TLS_PROTO_TLS_BASIC) || defined(HITLS_TLS_PROTO_DTLS12)
    config->allowLegacyRenegotiate = false;
#endif
}
static void InitConfig(HITLS_Config *config)
{
    BasicInitConfig(config);
#ifdef HITLS_TLS_FEATURE_RENEGOTIATION
    config->allowClientRenegotiate = false;
    config->isSupportRenegotiation = false;
#endif
#if defined(HITLS_TLS_FEATURE_RENEGOTIATION) && defined(HITLS_TLS_FEATURE_SESSION)
    config->isResumptionOnRenego = false;
#endif
#ifdef HITLS_TLS_SUITE_KX_RSA
    config->needCheckPmsVersion = false;
#endif
    config->readAhead = 0;
#ifdef HITLS_TLS_CONFIG_KEY_USAGE
    config->needCheckKeyUsage = true;
#endif
#ifdef HITLS_TLS_CONFIG_MANUAL_DH
    config->isSupportDhAuto = (config->maxVersion == HITLS_VERSION_TLCP11) ? false : true;
#endif
    if (config->maxVersion == HITLS_VERSION_TLCP11) {
        config->isSupportExtendMasterSecret = false;
    }
#ifdef HITLS_TLS_FEATURE_FLIGHT
    config->isFlightTransmitEnable = false;
#endif
#ifdef HITLS_TLS_FEATURE_CERT_MODE
    /** Set the certificate verification mode */
    config->isSupportClientVerify = false;
    config->isSupportNoClientCert = true;
    config->isSupportVerifyNone = false;
#endif
#ifdef HITLS_TLS_FEATURE_PHA
    config->isSupportPostHandshakeAuth = false;
#endif
#if defined(HITLS_TLS_FEATURE_RENEGOTIATION) && defined(HITLS_TLS_FEATURE_CERT_MODE)
    config->isSupportClientOnceVerify = false;
#endif
    config->isQuietShutdown = false;
    config->maxCertList = HITLS_MAX_CERT_LIST_DEFAULT;
    config->isKeepPeerCert = true;
#ifdef HITLS_TLS_FEATURE_SESSION_TICKET
    config->isSupportSessionTicket = true;
    config->ticketNums = HITLS_TLS13_TICKET_NUM_DEFAULT;
#endif
#ifdef HITLS_TLS_FEATURE_SECURITY
    // Default security settings
    SECURITY_SetDefault(config);
#endif
}

static int32_t DefaultCipherSuitesByVersion(uint16_t version, HITLS_Config *config)
{
    const uint16_t *groups = g_tls12CipherSuites;
    uint32_t size = sizeof(g_tls12CipherSuites);
    switch (version) {
#ifdef HITLS_TLS_PROTO_TLCP11
        case HITLS_VERSION_TLCP11:
            groups = g_tlcpCipherSuites;
            size = sizeof(g_tlcpCipherSuites);
            break;
#endif
        default:
            break;
    }
    return SetDefaultCipherSuite(config, groups, size);
}

int32_t DefaultConfig(uint16_t version, HITLS_Config *config)
{
    // Static settings
    config->minVersion = version;
    config->maxVersion = version;

    InitConfig(config);

    int32_t ret = DefaultCipherSuitesByVersion(version, config);
    if (ret != HITLS_SUCCESS) {
        return ret;
    }
#ifdef HITLS_TLS_PROTO_TLS13
    /* Configure the TLS1.3 cipher suite for all TLS versions */
    ret = SetTLS13DefaultCipherSuites(config);
    if (ret != HITLS_SUCCESS) {
        BSL_LOG_BINLOG_FIXLEN(BINLOG_ID16570, BSL_LOG_LEVEL_ERR, BSL_LOG_BINLOG_TYPE_RUN,
            "SetCipherSuites fail", 0, 0, 0, 0);
        goto ERR;
    }
#endif
    if (SetDefaultSignHashAlg(config) != HITLS_SUCCESS) {
        BSL_LOG_BINLOG_FIXLEN(BINLOG_ID16571, BSL_LOG_LEVEL_ERR, BSL_LOG_BINLOG_TYPE_RUN,
            "SetSignHashAlg fail", 0, 0, 0, 0);
        goto ERR;
    }

    if ((SetDefaultPointFormats(config) != HITLS_SUCCESS) ||
        (SetDefaultGroups(config) != HITLS_SUCCESS)) {
        BSL_LOG_BINLOG_FIXLEN(BINLOG_ID16572, BSL_LOG_LEVEL_ERR, BSL_LOG_BINLOG_TYPE_RUN,
            "SetPointFormats or SetGroups fail", 0, 0, 0, 0);
        goto ERR;
    }

    if (SAL_CERT_MgrIsEnable()) {
        config->certMgrCtx = SAL_CERT_MgrCtxNew();
        if (config->certMgrCtx == NULL) {
            BSL_LOG_BINLOG_FIXLEN(BINLOG_ID16573, BSL_LOG_LEVEL_ERR, BSL_LOG_BINLOG_TYPE_RUN,
                "sessMgr new fail", 0, 0, 0, 0);
            goto ERR;
        }
    }
#ifdef HITLS_TLS_FEATURE_SESSION
    config->sessMgr = SESSMGR_New();
    if (config->sessMgr == NULL) {
        BSL_LOG_BINLOG_FIXLEN(BINLOG_ID16574, BSL_LOG_LEVEL_ERR, BSL_LOG_BINLOG_TYPE_RUN,
            "sessMgr new fail", 0, 0, 0, 0);
        goto ERR;
    }
#endif
    return HITLS_SUCCESS;
ERR:
    CFG_CleanConfig(config);
    return HITLS_MEMALLOC_FAIL;
}
#ifdef HITLS_TLS_PROTO_TLS13
int32_t DefaultTLS13Config(HITLS_Config *config)
{
    // Static settings
    config->minVersion = HITLS_VERSION_TLS13;
    config->maxVersion = HITLS_VERSION_TLS13;

    InitConfig(config);

    // Dynamic setting. By default, only the cipher suite and point format are set. For details, see the comments in
    // HITLS_CFG_NewDTLS12Config.
    if ((SetTLS13DefaultCipherSuites(config) != HITLS_SUCCESS) ||
        (SetDefaultPointFormats(config) != HITLS_SUCCESS) ||
        (SetDefaultTLS13Groups(config) != HITLS_SUCCESS) ||
        (SetTLS13DefaultSignScheme(config) != HITLS_SUCCESS)) {
        BSL_LOG_BINLOG_FIXLEN(BINLOG_ID16575, BSL_LOG_LEVEL_ERR, BSL_LOG_BINLOG_TYPE_RUN,
            "Failed to set the default configuration of tls13", 0, 0, 0, 0);
        CFG_CleanConfig(config);
        return HITLS_MEMALLOC_FAIL;
    }

    config->keyExchMode = TLS13_KE_MODE_PSK_WITH_DHE;

    if (SAL_CERT_MgrIsEnable()) {
        config->certMgrCtx = SAL_CERT_MgrCtxNew();
        if (config->certMgrCtx == NULL) {
            BSL_LOG_BINLOG_FIXLEN(BINLOG_ID16576, BSL_LOG_LEVEL_ERR, BSL_LOG_BINLOG_TYPE_RUN,
                "certMgrCtx new fail", 0, 0, 0, 0);
            CFG_CleanConfig(config);
            return HITLS_MEMALLOC_FAIL;
        }
    }
#ifdef HITLS_TLS_FEATURE_SESSION
    config->sessMgr = SESSMGR_New();
    if (config->sessMgr == NULL) {
        BSL_LOG_BINLOG_FIXLEN(BINLOG_ID16577, BSL_LOG_LEVEL_ERR, BSL_LOG_BINLOG_TYPE_RUN,
            "sessMgr new fail", 0, 0, 0, 0);
        CFG_CleanConfig(config);
        return HITLS_MEMALLOC_FAIL;
    }
#endif
    return HITLS_SUCCESS;
}
#endif
#ifdef HITLS_TLS_PROTO_ALL
static int32_t SetDefaultTlsAllCipherSuites(HITLS_Config *config)
{
#ifdef HITLS_TLS_PROTO_TLS13
    int32_t ret = SetTLS13DefaultCipherSuites(config);
    if (ret != HITLS_SUCCESS) {
        return ret;
    }
#endif

    return SetDefaultCipherSuite(config, g_tls12CipherSuites, sizeof(g_tls12CipherSuites));
}
#endif
#ifdef HITLS_TLS_PROTO_ALL
int32_t DefaultTlsAllConfig(HITLS_Config *config)
{
    // Support full version
    config->minVersion = HITLS_VERSION_TLS12;
    config->maxVersion = HITLS_VERSION_TLS13;

    InitConfig(config);

    // Dynamic setting
    if ((SetDefaultTlsAllCipherSuites(config) != HITLS_SUCCESS) ||
        (SetDefaultPointFormats(config) != HITLS_SUCCESS) ||
        (SetDefaultGroups(config) != HITLS_SUCCESS) ||
        (SetDefaultSignHashAlg(config) != HITLS_SUCCESS)) {
        BSL_LOG_BINLOG_FIXLEN(BINLOG_ID16578, BSL_LOG_LEVEL_ERR, BSL_LOG_BINLOG_TYPE_RUN,
            "Failed to set the default configuration of tls_all", 0, 0, 0, 0);
        CFG_CleanConfig(config);
        return HITLS_MEMALLOC_FAIL;
    }

    config->keyExchMode = TLS13_KE_MODE_PSK_WITH_DHE;

    if (SAL_CERT_MgrIsEnable()) {
        config->certMgrCtx = SAL_CERT_MgrCtxNew();
        if (config->certMgrCtx == NULL) {
            BSL_LOG_BINLOG_FIXLEN(BINLOG_ID16579, BSL_LOG_LEVEL_ERR, BSL_LOG_BINLOG_TYPE_RUN,
                "MgrCtx new fail", 0, 0, 0, 0);
            CFG_CleanConfig(config);
            return HITLS_MEMALLOC_FAIL;
        }
    }
#ifdef HITLS_TLS_FEATURE_SESSION
    config->sessMgr = SESSMGR_New();
    if (config->sessMgr == NULL) {
        BSL_LOG_BINLOG_FIXLEN(BINLOG_ID16580, BSL_LOG_LEVEL_ERR, BSL_LOG_BINLOG_TYPE_RUN,
            "sessMgr new fail", 0, 0, 0, 0);
        CFG_CleanConfig(config);
        return HITLS_MEMALLOC_FAIL;
    }
#endif
    return HITLS_SUCCESS;
}
#endif
#ifdef HITLS_TLS_PROTO_DTLS
static int32_t SetDefaultDtlsAllCipherSuites(HITLS_Config *config)
{
    const uint16_t cipherSuites[] = {
        /* DTLS1.2 */
        HITLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384, HITLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384,
        HITLS_DHE_DSS_WITH_AES_256_GCM_SHA384, HITLS_DHE_RSA_WITH_AES_256_GCM_SHA384,
        HITLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256, HITLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256,
        HITLS_DHE_RSA_WITH_CHACHA20_POLY1305_SHA256, HITLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
        HITLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256, HITLS_DHE_DSS_WITH_AES_128_GCM_SHA256,
        HITLS_DHE_RSA_WITH_AES_128_GCM_SHA256,

        /* The DTLS1.0 cipher suite is not supported */
    };

    return SetDefaultCipherSuite(config, cipherSuites, sizeof(cipherSuites));
}

int32_t DefaultDtlsAllConfig(HITLS_Config *config)
{
    // Static settings
    config->minVersion =
        HITLS_VERSION_DTLS12;  // does not support DTLS 1.0. Therefore, the minimum version number is set to DTLS 1.2.
    config->maxVersion = HITLS_VERSION_DTLS12;

    InitConfig(config);

    // Dynamic setting
    if ((SetDefaultDtlsAllCipherSuites(config) != HITLS_SUCCESS) ||
        (SetDefaultPointFormats(config) != HITLS_SUCCESS) ||
        (SetDefaultGroups(config) != HITLS_SUCCESS) ||
        (SetDefaultSignHashAlg(config) != HITLS_SUCCESS)) {
        BSL_LOG_BINLOG_FIXLEN(BINLOG_ID16581, BSL_LOG_LEVEL_ERR, BSL_LOG_BINLOG_TYPE_RUN,
            "set default config fail", 0, 0, 0, 0);
        CFG_CleanConfig(config);
        return HITLS_MEMALLOC_FAIL;
    }

    if (SAL_CERT_MgrIsEnable()) {
        config->certMgrCtx = SAL_CERT_MgrCtxNew();
        if (config->certMgrCtx == NULL) {
            BSL_LOG_BINLOG_FIXLEN(BINLOG_ID16582, BSL_LOG_LEVEL_ERR, BSL_LOG_BINLOG_TYPE_RUN,
                "MgrCtxNew fail", 0, 0, 0, 0);
            CFG_CleanConfig(config);
            return HITLS_MEMALLOC_FAIL;
        }
    }
#ifdef HITLS_TLS_FEATURE_SESSION
    config->sessMgr = SESSMGR_New();
    if (config->sessMgr == NULL) {
        BSL_LOG_BINLOG_FIXLEN(BINLOG_ID16583, BSL_LOG_LEVEL_ERR, BSL_LOG_BINLOG_TYPE_RUN,
            "SESSMGR_New fail", 0, 0, 0, 0);
        CFG_CleanConfig(config);
        return HITLS_MEMALLOC_FAIL;
    }
#endif
    return HITLS_SUCCESS;
}
#endif