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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "uio_base.h"
#include "hitls_cert_type.h"
#include "hitls_config.h"
#include "hitls_error.h"
#include "hitls_psk.h"
#include "hitls_session.h"
#include "hitls_session.h"
#include "hitls_debug.h"
#include "hitls_sni.h"
#include "hitls_alpn.h"
#include "hitls_security.h"
#include "hitls_crypt_init.h"
#include "hlt_type.h"
#include "logger.h"
#include "tls_res.h"
#include "cert_callback.h"
#include "sctp_channel.h"
#include "tcp_channel.h"
#include "common_func.h"
#include "crypt_eal_rand.h"
#include "crypt_algid.h"
#include "channel_res.h"

#define SUCCESS 0
#define ERROR (-1)
#define FUNC_TIME_OUT_SEC 120

#define ASSERT_RETURN(condition, log) \
    do {                              \
        if (!(condition)) {           \
            LOG_ERROR(log);           \
            return ERROR;             \
        }                             \
    } while (0)

typedef struct {
    char *name;
    uint16_t configValue;
} HitlsConfig;

typedef enum {
    CIPHER,
    GROUPS,
    SIGNATURE,
    POINTFORMAT,
} HitlsConfigType;

static const HitlsConfig g_cipherSuiteList[] = {
    {"HITLS_RSA_WITH_AES_128_CBC_SHA", HITLS_RSA_WITH_AES_128_CBC_SHA},
    {"HITLS_DHE_DSS_WITH_AES_128_CBC_SHA", HITLS_DHE_DSS_WITH_AES_128_CBC_SHA},
    {"HITLS_DHE_RSA_WITH_AES_128_CBC_SHA", HITLS_DHE_RSA_WITH_AES_128_CBC_SHA},
    {"HITLS_RSA_WITH_AES_256_CBC_SHA", HITLS_RSA_WITH_AES_256_CBC_SHA},
    {"HITLS_DHE_DSS_WITH_AES_256_CBC_SHA", HITLS_DHE_DSS_WITH_AES_256_CBC_SHA},
    {"HITLS_DHE_RSA_WITH_AES_256_CBC_SHA", HITLS_DHE_RSA_WITH_AES_256_CBC_SHA},
    {"HITLS_RSA_WITH_AES_128_CBC_SHA256", HITLS_RSA_WITH_AES_128_CBC_SHA256},
    {"HITLS_RSA_WITH_AES_256_CBC_SHA256", HITLS_RSA_WITH_AES_256_CBC_SHA256},
    {"HITLS_DHE_DSS_WITH_AES_128_CBC_SHA256", HITLS_DHE_DSS_WITH_AES_128_CBC_SHA256},
    {"HITLS_DHE_RSA_WITH_AES_128_CBC_SHA256", HITLS_DHE_RSA_WITH_AES_128_CBC_SHA256},
    {"HITLS_DHE_DSS_WITH_AES_256_CBC_SHA256", HITLS_DHE_DSS_WITH_AES_256_CBC_SHA256},
    {"HITLS_DHE_RSA_WITH_AES_256_CBC_SHA256", HITLS_DHE_RSA_WITH_AES_256_CBC_SHA256},
    {"HITLS_RSA_WITH_AES_128_GCM_SHA256", HITLS_RSA_WITH_AES_128_GCM_SHA256},
    {"HITLS_RSA_WITH_AES_256_GCM_SHA384", HITLS_RSA_WITH_AES_256_GCM_SHA384},
    {"HITLS_DHE_RSA_WITH_AES_128_GCM_SHA256", HITLS_DHE_RSA_WITH_AES_128_GCM_SHA256},
    {"HITLS_DHE_RSA_WITH_AES_256_GCM_SHA384", HITLS_DHE_RSA_WITH_AES_256_GCM_SHA384},
    {"HITLS_DHE_DSS_WITH_AES_128_GCM_SHA256", HITLS_DHE_DSS_WITH_AES_128_GCM_SHA256},
    {"HITLS_DHE_DSS_WITH_AES_256_GCM_SHA384", HITLS_DHE_DSS_WITH_AES_256_GCM_SHA384},
    {"HITLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA", HITLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA},
    {"HITLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA", HITLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA},
    {"HITLS_ECDHE_RSA_WITH_AES_128_CBC_SHA", HITLS_ECDHE_RSA_WITH_AES_128_CBC_SHA},
    {"HITLS_ECDHE_RSA_WITH_AES_256_CBC_SHA", HITLS_ECDHE_RSA_WITH_AES_256_CBC_SHA},
    {"HITLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256", HITLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256},
    {"HITLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384", HITLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384},
    {"HITLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256", HITLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256},
    {"HITLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384", HITLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384},
    {"HITLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256", HITLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256},
    {"HITLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384", HITLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384},
    {"HITLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256", HITLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256},
    {"HITLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384", HITLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384},
    {"HITLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256", HITLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256},
    {"HITLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256", HITLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256},
    {"HITLS_DHE_RSA_WITH_CHACHA20_POLY1305_SHA256", HITLS_DHE_RSA_WITH_CHACHA20_POLY1305_SHA256},
    {"HITLS_AES_128_GCM_SHA256", HITLS_AES_128_GCM_SHA256},
    {"HITLS_AES_256_GCM_SHA384", HITLS_AES_256_GCM_SHA384},
    {"HITLS_CHACHA20_POLY1305_SHA256", HITLS_CHACHA20_POLY1305_SHA256},
    {"HITLS_AES_128_CCM_SHA256", HITLS_AES_128_CCM_SHA256},
    {"HITLS_AES_128_CCM_8_SHA256", HITLS_AES_128_CCM_8_SHA256},
    {"HITLS_ECDHE_ECDSA_WITH_AES_128_CCM", HITLS_ECDHE_ECDSA_WITH_AES_128_CCM},
    {"HITLS_ECDHE_ECDSA_WITH_AES_256_CCM", HITLS_ECDHE_ECDSA_WITH_AES_256_CCM},
    {"HITLS_DHE_RSA_WITH_AES_128_CCM", HITLS_DHE_RSA_WITH_AES_128_CCM},
    {"HITLS_DHE_RSA_WITH_AES_256_CCM", HITLS_DHE_RSA_WITH_AES_256_CCM},
    {"HITLS_RSA_WITH_AES_256_CCM", HITLS_RSA_WITH_AES_256_CCM},
    {"HITLS_RSA_WITH_AES_256_CCM_8", HITLS_RSA_WITH_AES_256_CCM_8},
    {"HITLS_RSA_WITH_AES_128_CCM", HITLS_RSA_WITH_AES_128_CCM},
    {"HITLS_RSA_WITH_AES_128_CCM_8", HITLS_RSA_WITH_AES_128_CCM_8},

    /* psk cipher suite */
    {"HITLS_PSK_WITH_AES_128_CBC_SHA", HITLS_PSK_WITH_AES_128_CBC_SHA},
    {"HITLS_PSK_WITH_AES_256_CBC_SHA", HITLS_PSK_WITH_AES_256_CBC_SHA},
    {"HITLS_DHE_PSK_WITH_AES_128_CBC_SHA", HITLS_DHE_PSK_WITH_AES_128_CBC_SHA},
    {"HITLS_DHE_PSK_WITH_AES_256_CBC_SHA", HITLS_DHE_PSK_WITH_AES_256_CBC_SHA},
    {"HITLS_RSA_PSK_WITH_AES_128_CBC_SHA", HITLS_RSA_PSK_WITH_AES_128_CBC_SHA},
    {"HITLS_RSA_PSK_WITH_AES_256_CBC_SHA", HITLS_RSA_PSK_WITH_AES_256_CBC_SHA},
    {"HITLS_PSK_WITH_AES_128_GCM_SHA256", HITLS_PSK_WITH_AES_128_GCM_SHA256},
    {"HITLS_PSK_WITH_AES_256_GCM_SHA384", HITLS_PSK_WITH_AES_256_GCM_SHA384},
    {"HITLS_DHE_PSK_WITH_AES_128_GCM_SHA256", HITLS_DHE_PSK_WITH_AES_128_GCM_SHA256},
    {"HITLS_DHE_PSK_WITH_AES_256_GCM_SHA384", HITLS_DHE_PSK_WITH_AES_256_GCM_SHA384},
    {"HITLS_RSA_PSK_WITH_AES_128_GCM_SHA256", HITLS_RSA_PSK_WITH_AES_128_GCM_SHA256},
    {"HITLS_RSA_PSK_WITH_AES_256_GCM_SHA384", HITLS_RSA_PSK_WITH_AES_256_GCM_SHA384},
    {"HITLS_PSK_WITH_AES_128_CBC_SHA256", HITLS_PSK_WITH_AES_128_CBC_SHA256},
    {"HITLS_PSK_WITH_AES_256_CBC_SHA384", HITLS_PSK_WITH_AES_256_CBC_SHA384},
    {"HITLS_DHE_PSK_WITH_AES_128_CBC_SHA256", HITLS_DHE_PSK_WITH_AES_128_CBC_SHA256},
    {"HITLS_DHE_PSK_WITH_AES_256_CBC_SHA384", HITLS_DHE_PSK_WITH_AES_256_CBC_SHA384},
    {"HITLS_RSA_PSK_WITH_AES_128_CBC_SHA256", HITLS_RSA_PSK_WITH_AES_128_CBC_SHA256},
    {"HITLS_RSA_PSK_WITH_AES_256_CBC_SHA384", HITLS_RSA_PSK_WITH_AES_256_CBC_SHA384},
    {"HITLS_ECDHE_PSK_WITH_AES_128_CBC_SHA", HITLS_ECDHE_PSK_WITH_AES_128_CBC_SHA},
    {"HITLS_ECDHE_PSK_WITH_AES_256_CBC_SHA", HITLS_ECDHE_PSK_WITH_AES_256_CBC_SHA},
    {"HITLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256", HITLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256},
    {"HITLS_ECDHE_PSK_WITH_AES_256_CBC_SHA384", HITLS_ECDHE_PSK_WITH_AES_256_CBC_SHA384},
    {"HITLS_PSK_WITH_CHACHA20_POLY1305_SHA256", HITLS_PSK_WITH_CHACHA20_POLY1305_SHA256},
    {"HITLS_ECDHE_PSK_WITH_CHACHA20_POLY1305_SHA256", HITLS_ECDHE_PSK_WITH_CHACHA20_POLY1305_SHA256},
    {"HITLS_DHE_PSK_WITH_CHACHA20_POLY1305_SHA256", HITLS_DHE_PSK_WITH_CHACHA20_POLY1305_SHA256},
    {"HITLS_RSA_PSK_WITH_CHACHA20_POLY1305_SHA256", HITLS_RSA_PSK_WITH_CHACHA20_POLY1305_SHA256},
    {"HITLS_ECDHE_PSK_WITH_AES_128_GCM_SHA256", HITLS_ECDHE_PSK_WITH_AES_128_GCM_SHA256},
    {"HITLS_ECDHE_PSK_WITH_AES_256_GCM_SHA384", HITLS_ECDHE_PSK_WITH_AES_256_GCM_SHA384},
    {"HITLS_DHE_PSK_WITH_AES_128_CCM", HITLS_DHE_PSK_WITH_AES_128_CCM},
    {"HITLS_DHE_PSK_WITH_AES_256_CCM", HITLS_DHE_PSK_WITH_AES_256_CCM},
    {"HITLS_PSK_WITH_AES_256_CCM", HITLS_PSK_WITH_AES_256_CCM},
    {"HITLS_ECDHE_PSK_WITH_AES_128_CCM_SHA256", HITLS_ECDHE_PSK_WITH_AES_128_CCM_SHA256},

    /* Anonymous ciphersuite */
    {"HITLS_DH_ANON_WITH_AES_256_CBC_SHA", HITLS_DH_ANON_WITH_AES_256_CBC_SHA},
    {"HITLS_DH_ANON_WITH_AES_128_CBC_SHA", HITLS_DH_ANON_WITH_AES_128_CBC_SHA},
    {"HITLS_DH_ANON_WITH_AES_128_CBC_SHA256", HITLS_DH_ANON_WITH_AES_128_CBC_SHA256},
    {"HITLS_DH_ANON_WITH_AES_256_CBC_SHA256", HITLS_DH_ANON_WITH_AES_256_CBC_SHA256},
    {"HITLS_DH_ANON_WITH_AES_128_GCM_SHA256", HITLS_DH_ANON_WITH_AES_128_GCM_SHA256},
    {"HITLS_DH_ANON_WITH_AES_256_GCM_SHA384", HITLS_DH_ANON_WITH_AES_256_GCM_SHA384},
    {"HITLS_ECDH_ANON_WITH_AES_128_CBC_SHA", HITLS_ECDH_ANON_WITH_AES_128_CBC_SHA},
    {"HITLS_ECDH_ANON_WITH_AES_256_CBC_SHA", HITLS_ECDH_ANON_WITH_AES_256_CBC_SHA},
    {"HITLS_ECDHE_SM4_CBC_SM3", HITLS_ECDHE_SM4_CBC_SM3},
    {"HITLS_ECC_SM4_CBC_SM3", HITLS_ECC_SM4_CBC_SM3},

    /* error ciphersuite */
    {"HITLS_INVALID_CIPHER_TC01", 0xFFFF},
    {"HITLS_INVALID_CIPHER_TC02", 0xFFFE},
};

static const HitlsConfig g_groupList[] = {
    {"HITLS_EC_GROUP_SECP256R1", HITLS_EC_GROUP_SECP256R1},
    {"HITLS_EC_GROUP_SECP384R1", HITLS_EC_GROUP_SECP384R1},
    {"HITLS_EC_GROUP_SECP521R1", HITLS_EC_GROUP_SECP521R1},
    {"HITLS_EC_GROUP_CURVE25519", HITLS_EC_GROUP_CURVE25519},
    {"HITLS_EC_GROUP_SM2", HITLS_EC_GROUP_SM2},
    {"HITLS_INVALID_GROUP_TC01", 0xFF},
    {"HITLS_INVALID_GROUP_TC02", 0xFE},
    {"HITLS_FF_DHE_2048", HITLS_FF_DHE_2048},
    {"HITLS_FF_DHE_3072", HITLS_FF_DHE_3072},
    {"HITLS_FF_DHE_4096", HITLS_FF_DHE_4096},
    {"HITLS_FF_DHE_6144", HITLS_FF_DHE_6144},
    {"HITLS_FF_DHE_8192", HITLS_FF_DHE_8192},
};

static const HitlsConfig g_signatureList[] = {
    {"CERT_SIG_SCHEME_RSA_PKCS1_SHA1", CERT_SIG_SCHEME_RSA_PKCS1_SHA1},
    {"CERT_SIG_SCHEME_ECDSA_SHA1", CERT_SIG_SCHEME_ECDSA_SHA1},
    {"CERT_SIG_SCHEME_ECDSA_SHA224", CERT_SIG_SCHEME_ECDSA_SHA224},
    {"CERT_SIG_SCHEME_RSA_PKCS1_SHA224", CERT_SIG_SCHEME_RSA_PKCS1_SHA224},
    {"CERT_SIG_SCHEME_RSA_PKCS1_SHA256", CERT_SIG_SCHEME_RSA_PKCS1_SHA256},
    {"CERT_SIG_SCHEME_RSA_PKCS1_SHA384", CERT_SIG_SCHEME_RSA_PKCS1_SHA384},
    {"CERT_SIG_SCHEME_RSA_PKCS1_SHA512", CERT_SIG_SCHEME_RSA_PKCS1_SHA512},
    {"CERT_SIG_SCHEME_ECDSA_SECP256R1_SHA256", CERT_SIG_SCHEME_ECDSA_SECP256R1_SHA256},
    {"CERT_SIG_SCHEME_ECDSA_SECP384R1_SHA384", CERT_SIG_SCHEME_ECDSA_SECP384R1_SHA384},
    {"CERT_SIG_SCHEME_ECDSA_SECP521R1_SHA512", CERT_SIG_SCHEME_ECDSA_SECP521R1_SHA512},
    {"CERT_SIG_SCHEME_RSA_PSS_RSAE_SHA256", CERT_SIG_SCHEME_RSA_PSS_RSAE_SHA256},
    {"CERT_SIG_SCHEME_RSA_PSS_RSAE_SHA384", CERT_SIG_SCHEME_RSA_PSS_RSAE_SHA384},
    {"CERT_SIG_SCHEME_RSA_PSS_RSAE_SHA512", CERT_SIG_SCHEME_RSA_PSS_RSAE_SHA512},
    {"CERT_SIG_SCHEME_ED25519", CERT_SIG_SCHEME_ED25519},
    {"CERT_SIG_SCHEME_ED448", CERT_SIG_SCHEME_ED448},
    {"CERT_SIG_SCHEME_DSA_SHA1", CERT_SIG_SCHEME_DSA_SHA1},
    {"CERT_SIG_SCHEME_DSA_SHA224", CERT_SIG_SCHEME_DSA_SHA224},
    {"CERT_SIG_SCHEME_DSA_SHA256", CERT_SIG_SCHEME_DSA_SHA256},
    {"CERT_SIG_SCHEME_DSA_SHA384", CERT_SIG_SCHEME_DSA_SHA384},
    {"CERT_SIG_SCHEME_DSA_SHA512", CERT_SIG_SCHEME_DSA_SHA512},
    {"CERT_SIG_SCHEME_SM2_SM3", CERT_SIG_SCHEME_SM2_SM3},
    {"CERT_SIG_SCHEME_RSA_PSS_PSS_SHA256", CERT_SIG_SCHEME_RSA_PSS_PSS_SHA256},
    {"HITLS_INVALID_SIG_TC01", 0xFFFF},
    {"HITLS_INVALID_SIG_TC02", 0xFFFE},
};

static const HitlsConfig g_eccFormatList[] = {
    {"HITLS_POINT_FORMAT_UNCOMPRESSED", HITLS_POINT_FORMAT_UNCOMPRESSED},
    {"HITLS_INVALID_FORMAT_TC01", 0xFF},
    {"HITLS_INVALID_FORMAT_TC02", 0xFE},
};

int HitlsInit(void)
{
    int ret;
    ret = RegMemCallback(MEM_CALLBACK_DEFAULT);
    ret |= RegCertCallback(CERT_CALLBACK_DEFAULT);
    CRYPT_EAL_RandInit(CRYPT_RAND_SHA256, NULL, NULL, NULL, 0);
    HITLS_CryptMethodInit();
    return ret;
}

HITLS_Config *HitlsNewCtx(TLS_VERSION tlsVersion)
{
    HITLS_Config *hitlsConfig = NULL;
    switch (tlsVersion) {
        case DTLS1_2:
            LOG_DEBUG("HiTLS New DTLS1_2 Ctx");
            hitlsConfig = HITLS_CFG_NewDTLS12Config();
            break;
        case TLS1_2:
            LOG_DEBUG("HiTLS New TLS1_2 Ctx");
            hitlsConfig = HITLS_CFG_NewTLS12Config();
            break;
        case TLS1_3:
            LOG_DEBUG("HiTLS New TLS1_3 Ctx");
            hitlsConfig = HITLS_CFG_NewTLS13Config();
            break;
        case TLS_ALL:
            LOG_DEBUG("HiTLS New TLS_ALL Ctx");
            hitlsConfig = HITLS_CFG_NewTLSConfig();
            break;
#ifdef HITLS_TLS_PROTO_TLCP11
        case TLCP1_1:
            LOG_DEBUG("HiTLS New TLCP1_1 Ctx");
            hitlsConfig = HITLS_CFG_NewTLCPConfig();
            break;
#endif
        default:
            /* Unknown protocol type */
            break;
    }
    if (hitlsConfig == NULL) {
        LOG_ERROR("HITLS Not Support This TlsVersion's ID %d", tlsVersion);
    }
#ifdef HITLS_TLS_FEATURE_SECURITY
    // Setting the security level
    HITLS_CFG_SetSecurityLevel(hitlsConfig, HITLS_SECURITY_LEVEL_ZERO);
#endif /* HITLS_TLS_FEATURE_SECURITY */
    return hitlsConfig;
}

void HitlsFreeCtx(void *ctx)
{
    HITLS_CFG_FreeConfig(ctx);
}

static int32_t GetConfigVauleFromStr(const HitlsConfig *hitlsConfigList, uint32_t configSize, const char *cipherName)
{
    for (uint32_t i = 0; i < configSize; i++) {
        if (strcmp(cipherName, hitlsConfigList[i].name) != 0) {
            continue;
        }
        return hitlsConfigList[i].configValue;
    }
    return ERROR; // The cipher suite does not exist.
}

static int8_t HitlsSetConfig(const HitlsConfig *hitlsConfigList, int configListSize, void *ctx,
                             char *name, HitlsConfigType type)
{
    int ret = 0;
    char configArray[MAX_CIPHERSUITES_LEN] = {0}; // A maximum of 512 characters are supported.
    char *token, *rest;
    int32_t configValue;
    uint16_t configValueArray[20] = {0}; // Currently, a maximum of 20 cipher suites are supported.
    uint32_t configSize = 0;
    ret = memcpy_s(configArray, sizeof(configArray), name, strlen(name));
    ASSERT_RETURN(ret == EOK, "Memcpy Error");

    configSize = 0;
    token = strtok_s(configArray, ":", &rest);
    do {
        // Currently, a maximum of 20 cipher suites are supported.
        ASSERT_RETURN(configSize < 20, "Max Support Set 20 Config");
        configValue = GetConfigVauleFromStr(hitlsConfigList, configListSize, token);
        ASSERT_RETURN(configValue != ERROR, "GetConfigVauleFromStr Error");
        configValueArray[configSize] = (uint16_t)configValue;
        token = strtok_s(NULL, ":", &rest);
        configSize++;
    } while (token != NULL);

    switch (type) {
        case CIPHER:
            ret = HITLS_CFG_SetCipherSuites(ctx, configValueArray, configSize);
            break;
        case GROUPS:
            ret = HITLS_CFG_SetGroups(ctx, configValueArray, configSize);
            break;
        case SIGNATURE:
            ret = HITLS_CFG_SetSignature(ctx, configValueArray, configSize);
            break;
        case POINTFORMAT:
        {
            uint8_t pointformatArray[20] = {0};
            for (uint32_t i = 0; i < configSize; i++) {
                pointformatArray[i] = configValueArray[i];
            }
            ret = HITLS_CFG_SetEcPointFormats(ctx, pointformatArray, configSize);
            break;
        }
        default:
            ret = ERROR;
    }

    ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetXXX Error");
    return SUCCESS;
}

int HitlsSetCtx(HITLS_Config *outCfg, HLT_Ctx_Config *inCtxCfg)
{
    int ret = 0;
    if (inCtxCfg->setSessionCache >= 0) {
        LOG_DEBUG("HiTLS Set SessionCache is %d", inCtxCfg->setSessionCache);
        HITLS_CFG_SetSessionCacheMode(outCfg, inCtxCfg->setSessionCache);
    }

    // Set the protocol version.
    if ((inCtxCfg->minVersion != 0) && (inCtxCfg->maxVersion != 0)) {
        LOG_DEBUG("HiTLS Set minVersion is %u maxVersion is %u", inCtxCfg->minVersion, inCtxCfg->maxVersion);
        ret = HITLS_CFG_SetVersion(outCfg, inCtxCfg->minVersion, inCtxCfg->maxVersion);
        ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetVersion Error ERROR");
    }
    if (inCtxCfg->SupportType == SERVER_CFG_SET_TRUE) {
        HITLS_CFG_SetCipherServerPreference(outCfg, true);
    }
    if (inCtxCfg->SupportType == SERVER_CFG_SET_FALSE) {
        HITLS_CFG_SetCipherServerPreference(outCfg, false);
    }
    // Setting Renegotiation
    LOG_DEBUG("HiTLS Set Support Renegotiation is %d", inCtxCfg->isSupportRenegotiation);
    ret = HITLS_CFG_SetRenegotiationSupport(outCfg, inCtxCfg->isSupportRenegotiation);
    ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetRenegotiationSupport ERROR");
    // Whether allow a renegotiation initiated by the client
    LOG_DEBUG("HiTLS Set allow Client Renegotiate is %d", inCtxCfg->allowClientRenegotiate);
    ret = HITLS_CFG_SetClientRenegotiateSupport(outCfg, inCtxCfg->allowClientRenegotiate);
    ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetClientRenegotiateSupport ERROR");

    // Whether to enable dual-ended verification
    LOG_DEBUG("HiTLS Set Support Client Verify is %d", inCtxCfg->isSupportClientVerify);
    ret = HITLS_CFG_SetClientVerifySupport(outCfg, inCtxCfg->isSupportClientVerify);
    ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetClientVerifySupport ERROR");

    LOG_DEBUG("HiTLS Set readAhead is %d", inCtxCfg->readAhead);
    ret = HITLS_CFG_SetReadAhead(outCfg, inCtxCfg->readAhead);
    ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetReadAhead ERROR");

    // Indicates whether to allow empty certificate list on the client.
    LOG_DEBUG("HiTLS Set Support Not Client Cert is %d", inCtxCfg->isSupportNoClientCert);
    ret = HITLS_CFG_SetNoClientCertSupport(outCfg, inCtxCfg->isSupportNoClientCert);
    ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetNoClientCertSupport ERROR");

    // Whether to enable pha
    LOG_DEBUG("HiTLS Set Support pha is %d", inCtxCfg->isSupportPostHandshakeAuth);
    ret = HITLS_CFG_SetPostHandshakeAuthSupport(outCfg, inCtxCfg->isSupportPostHandshakeAuth);
    ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetPostHandshakeAuth ERROR");

    // Indicates whether extended master keys are supported.
    LOG_DEBUG("HiTLS Set Support Extend Master Secret is %d", inCtxCfg->isSupportExtendMasterSecret);
    ret = HITLS_CFG_SetExtenedMasterSecretSupport(outCfg, inCtxCfg->isSupportExtendMasterSecret);
    ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetExtenedMasterSecretSupport ERROR");

    // Support CloseCheckKeyUsage
    LOG_DEBUG("HiTLS Set CloseCheckKeyUsage is false");
    ret = HITLS_CFG_SetCheckKeyUsage(outCfg, inCtxCfg->needCheckKeyUsage);
    ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetCheckKeyUsage ERROR");

	// Indicates whether to support sessionTicket.
    LOG_DEBUG("HiTLS Set Support SessionTicket is %d", inCtxCfg->isSupportSessionTicket);
    ret = HITLS_CFG_SetSessionTicketSupport(outCfg, inCtxCfg->isSupportSessionTicket);
    ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetSessionTicketSupport ERROR");

    // Whether encrypt-then-mac is supported
    LOG_DEBUG("HiTLS Set Support EncryptThenMac is %d", inCtxCfg->isEncryptThenMac);
    ret = HITLS_CFG_SetEncryptThenMac(outCfg, (uint32_t)inCtxCfg->isEncryptThenMac);
    ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetEncryptThenMac ERROR");

    // ECC Point Format Configuration for Asymmetric Algorithms
    if (strncmp("NULL", inCtxCfg->pointFormats, strlen(inCtxCfg->pointFormats)) != 0) {
        LOG_DEBUG("HiTLS Set PoinFormats is %s", inCtxCfg->pointFormats);
        int configListSize = sizeof(g_eccFormatList) / sizeof(g_eccFormatList[0]);
        ret = HitlsSetConfig(g_eccFormatList, configListSize, outCfg, inCtxCfg->pointFormats, POINTFORMAT);
        ASSERT_RETURN(ret == SUCCESS, "ECC Format ERROR");
    }

    // Loading cipher suites
    if (strncmp("NULL", inCtxCfg->cipherSuites, strlen(inCtxCfg->cipherSuites)) != 0) {
        LOG_DEBUG("HiTLS Set CipherSuites is %s", inCtxCfg->cipherSuites);
        int configListSize = sizeof(g_cipherSuiteList) / sizeof(g_cipherSuiteList[0]);
        ret = HitlsSetConfig(g_cipherSuiteList, configListSize, outCfg, inCtxCfg->cipherSuites, CIPHER);
        ASSERT_RETURN(ret == SUCCESS, "Hitls Set Cipher ERROR");
    }

    // set groups
    if (strncmp("NULL", inCtxCfg->groups, strlen(inCtxCfg->groups)) != 0) {
        LOG_DEBUG("HiTLS Set Groups is %s", inCtxCfg->groups);
        int configListSize = sizeof(g_groupList) / sizeof(g_groupList[0]);
        ret = HitlsSetConfig(g_groupList, configListSize, outCfg, inCtxCfg->groups, GROUPS);
        ASSERT_RETURN(ret == SUCCESS, "Hitls Set Group ERROR");
    }

    // signature algorithm
    if (strncmp("NULL", inCtxCfg->signAlgorithms, strlen(inCtxCfg->signAlgorithms)) != 0) {
        LOG_DEBUG("HiTLS Set SignAlgorithms is %s", inCtxCfg->signAlgorithms);
        int configListSize = sizeof(g_signatureList) / sizeof(g_signatureList[0]);
        ret = HitlsSetConfig(g_signatureList, configListSize, outCfg, inCtxCfg->signAlgorithms, SIGNATURE);
        ASSERT_RETURN(ret == SUCCESS, "Hitls Set Signature ERROR");
    }

    // sni
    if (strncmp("NULL", inCtxCfg->serverName, strlen(inCtxCfg->serverName)) != 0) {
        LOG_DEBUG("HiTLS Set ServerName is %s", inCtxCfg->serverName);
        ret = HITLS_CFG_SetServerName(outCfg, (uint8_t *)inCtxCfg->serverName, strlen(inCtxCfg->serverName));
        ASSERT_RETURN(ret == SUCCESS, "Hitls Set ServerName ERROR");
    }

    // Register the server_name function callback.
    if (strncmp("NULL", inCtxCfg->sniDealCb, strlen(inCtxCfg->sniDealCb)) != 0) {
        LOG_DEBUG("HiTLS Set server_name callback is %s", inCtxCfg->sniDealCb);
        ret = HITLS_CFG_SetServerNameCb(outCfg, GetExtensionCb(inCtxCfg->sniDealCb));
        ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetServerNameCb Fail");
    }

    // Register values related to server_name.
    if (strncmp("NULL", inCtxCfg->sniArg, strlen(inCtxCfg->sniArg)) != 0) {
        LOG_DEBUG("HiTLS Set sniArg");
        ret = HITLS_CFG_SetServerNameArg(outCfg, GetExampleData(inCtxCfg->sniArg));
        ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetServerNameArg Fail");
    }

    // alpn
    if (strncmp("NULL", inCtxCfg->alpnList, strlen(inCtxCfg->alpnList)) != 0) {
        LOG_DEBUG("HiTLS Set alpnList is %s", inCtxCfg->alpnList);
        ret = HITLS_CFG_SetAlpnProtos(outCfg, (const uint8_t *)inCtxCfg->alpnList, strlen(inCtxCfg->alpnList));
        ASSERT_RETURN(ret == SUCCESS, "Hitls Set alpnList ERROR");
    }

    // Sets the ALPN selection callback on the server.
    if (strncmp("NULL", inCtxCfg->alpnSelectCb, strlen(inCtxCfg->alpnSelectCb)) != 0) {
        LOG_DEBUG("HiTLS Set ALPN callback is %s", inCtxCfg->alpnSelectCb);
        ret = HITLS_CFG_SetAlpnProtosSelectCb(
            outCfg, GetExtensionCb(inCtxCfg->alpnSelectCb), GetExampleData(inCtxCfg->alpnUserData));
        ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetAlpnProtosSelectCb Fail");
    }

    // Loading Certificates
    ret = HiTLS_X509_LoadCertAndKey(outCfg, inCtxCfg->caCert, inCtxCfg->chainCert,
        inCtxCfg->eeCert, inCtxCfg->signCert, inCtxCfg->privKey, inCtxCfg->signPrivKey);
    ASSERT_RETURN(ret == SUCCESS, "Load cert Fail");

    if (strncmp("NULL", inCtxCfg->psk, strlen(inCtxCfg->psk)) != 0) {
        ret = ExampleSetPsk(inCtxCfg->psk);
        ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetPskClientCallback Fail");

        ret = HITLS_CFG_SetPskClientCallback(outCfg, ExampleClientCb);
        ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetPskClientCallback Fail");

        ret = HITLS_CFG_SetPskServerCallback(outCfg, ExampleServerCb);
        ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetPskServerCallback Fail");
    }

    if (strncmp("NULL", inCtxCfg->ticketKeyCb, strlen(inCtxCfg->ticketKeyCb)) != 0) {
        LOG_DEBUG("HiTLS Set Ticker key callback is %s", inCtxCfg->ticketKeyCb);
        ret = HITLS_CFG_SetTicketKeyCallback(outCfg, GetTicketKeyCb(inCtxCfg->ticketKeyCb));
        ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetTicketKeyCallback Fail");
    }

    // Load link setup callback
    if (inCtxCfg->infoCb != NULL) {
        LOG_DEBUG("HiTLS Set info callback");
        ret = HITLS_CFG_SetInfoCb(outCfg, inCtxCfg->infoCb);
        ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetInfoCb Fail");
    }

    if (inCtxCfg->msgCb != NULL) {
        LOG_DEBUG("HiTLS Set msg callback");
        ret = HITLS_CFG_SetMsgCb(outCfg, inCtxCfg->msgCb);
        ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetMsgCb Fail");
    }

    if (inCtxCfg->msgArg != NULL) {
        LOG_DEBUG("HiTLS Set msgArg");
        ret = HITLS_CFG_SetMsgCbArg(outCfg, inCtxCfg->msgArg);
        ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetMsgCbArg Fail");
    }

    // Sets whether to enable the function of sending handshake messages by flight.
    LOG_DEBUG("HiTLS Set Support isFlightTransmitEnable is %d", inCtxCfg->isFlightTransmitEnable);
    ret = HITLS_CFG_SetFlightTransmitSwitch(outCfg, inCtxCfg->isFlightTransmitEnable);
    ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetFlightTransmitSwitch ERROR");

    // Setting the security level
    LOG_DEBUG("HiTLS Set SecurityLevel is %d", inCtxCfg->securitylevel);
    ret = HITLS_CFG_SetSecurityLevel(outCfg, inCtxCfg->securitylevel);
    ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetSecurityLevel ERROR");

    // Indicates whether the DH key length can be followed by the certificate.
    LOG_DEBUG("HiTLS Set Support DHAuto is %d", inCtxCfg->isSupportDhAuto);
    ret = HITLS_CFG_SetDhAutoSupport(outCfg, inCtxCfg->isSupportDhAuto);
    ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetDhAutoSupport ERROR");

    // TLS1.3 key exchange mode
    if (outCfg->maxVersion == HITLS_VERSION_TLS13) {
        LOG_DEBUG("HiTLS Set keyExchMode is %u", inCtxCfg->keyExchMode);
    ret = HITLS_CFG_SetKeyExchMode(outCfg, inCtxCfg->keyExchMode);
    ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetKeyExchMode ERROR");
    }

    // Set whether to enable isSupportVerifyNone;
    LOG_DEBUG("HiTLS Set Support pha is %d", inCtxCfg->isSupportVerifyNone);
    ret = HITLS_CFG_SetVerifyNoneSupport(outCfg, inCtxCfg->isSupportVerifyNone);
    ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetVerifyNoneSupport ERROR");

    LOG_DEBUG("HiTLS Set Empty Record Number is %u", inCtxCfg->emptyRecordsNum);
    ret = HITLS_CFG_SetEmptyRecordsNum(outCfg, inCtxCfg->emptyRecordsNum);
    ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetEmptyRecordsNum");

#if defined(HITLS_TLS_PROTO_TLS_BASIC) || defined(HITLS_TLS_PROTO_DTLS12)
    LOG_DEBUG("HiTLS Set allow Legacy Renegotiate is %d", inCtxCfg->allowLegacyRenegotiate);
    ret = HITLS_CFG_SetLegacyRenegotiateSupport(outCfg, inCtxCfg->allowLegacyRenegotiate);
    ASSERT_RETURN(ret == SUCCESS, "HITLS_CFG_SetLegacyRenegotiateSupport ERROR");
#endif /* defined(HITLS_TLS_PROTO_TLS_BASIC) || defined(HITLS_TLS_PROTO_DTLS12) */

    return SUCCESS;
}

void *HitlsNewSsl(void *ctx)
{
    return HITLS_New(ctx);
}

void HitlsFreeSsl(void *ssl)
{
    HITLS_Free(ssl);
}

const BSL_UIO_Method *GetDefaultMethod(HILT_TransportType type)
{
    switch (type) {
        case SCTP:
            return SctpGetDefaultMethod();
        case TCP:
            return TcpGetDefaultMethod();
        default:
            break;
    }
    return NULL;
}

int HitlsSetSsl(void *ssl, HLT_Ssl_Config *sslConfig)
{
    int ret;
    SetNeedCbSctpCtrlCmd(sslConfig->sctpCtrlCmd);
    if (sslConfig->SupportType == SERVER_CTX_SET_TRUE) {
        HITLS_SetCipherServerPreference((HITLS_Ctx *)ssl, true);
    }
    if (sslConfig->SupportType == SERVER_CTX_SET_FALSE) {
        HITLS_SetCipherServerPreference((HITLS_Ctx *)ssl, false);
    }
    HILT_TransportType type = (sslConfig->connType == NONE_TYPE) ? SCTP : sslConfig->connType;

    BSL_UIO *uio = BSL_UIO_New(GetDefaultMethod(type));
    ASSERT_RETURN(uio != NULL, "HITLS_SetUio Fail");

    ret = BSL_UIO_Ctrl(uio, BSL_UIO_SET_FD, (int32_t)sizeof(sslConfig->sockFd), &sslConfig->sockFd);
    if (ret != SUCCESS) {
        LOG_ERROR("BSL_UIO_SET_FD Fail");
        BSL_UIO_Free(uio);
        return ERROR;
    }
    BSL_UIO_SetInit(uio, 1);

    ret = HITLS_SetUio(ssl, uio);
    if (ret != SUCCESS) {
        LOG_ERROR("HITLS_SetUio Fail");
        BSL_UIO_Free(uio);
        return ERROR;
    }

    // Release the UIO to prevent memory leakage.
    BSL_UIO_Free(uio);
    return SUCCESS;
}

int HitlsAccept(void *ssl)
{
    int ret, tryNum;
    tryNum = 0;
    LOG_DEBUG("HiTLS Tls Accept Ing...");
    do {
        ret = HITLS_Accept(ssl);
        usleep(1000); // stay 1000us
        tryNum++;
    } while ((ret == HITLS_REC_NORMAL_RECV_BUF_EMPTY ||
            ret == HITLS_REC_NORMAL_IO_BUSY) &&
            (tryNum < FUNC_TIME_OUT_SEC * 1000)); // usleep(1000) after each attemp.
    if (ret != SUCCESS) {
        LOG_ERROR("HITLS_Accept Error is %d", ret);
    } else {
        LOG_DEBUG("HiTLS Tls Accept Success");
    }
    return ret;
}

int HitlsConnect(void *ssl)
{
    int ret, tryNum;
    tryNum = 0;
    LOG_DEBUG("HiTLS Tls Connect Ing...");
    do {
        ret = HITLS_Connect(ssl);
        usleep(1000); // stay 1000us
        tryNum++;
    } while ((ret == HITLS_REC_NORMAL_RECV_BUF_EMPTY ||
            ret == HITLS_REC_NORMAL_IO_BUSY) &&
            (tryNum < FUNC_TIME_OUT_SEC * 1000)); // usleep(1000) after each attemp.
    if (ret != SUCCESS) {
        LOG_ERROR("HITLS_Connect Error is %d", ret);
    } else {
        LOG_DEBUG("HiTLS Tls Connect Success");
    }
    return ret;
}

int HitlsWrite(void *ssl, uint8_t *data, uint32_t dataLen)
{
    int ret, tryNum;
    LOG_DEBUG("HiTLS Write Ing...");
    tryNum = 0;
    uint32_t len = 0;
    do {
        ret = HITLS_Write(ssl, data, dataLen, &len);
        tryNum++;
        usleep(1000); // stay 1000us
    } while ((ret == HITLS_REC_NORMAL_RECV_BUF_EMPTY ||
            ret == HITLS_REC_NORMAL_IO_BUSY) &&
            (tryNum < 4000)); // A maximum of 4000 calls
    LOG_DEBUG("HiTLS Write Result is %d", ret);
    return ret;
}

int HitlsRead(void *ssl, uint8_t *data, uint32_t bufSize, uint32_t *readLen)
{
    int ret, tryNum;
    LOG_DEBUG("HiTLS Read Ing...");
    tryNum = 0;
    do {
        ret = HITLS_Read(ssl, data, bufSize, readLen);
        tryNum++;
        usleep(1000); // stay 1000us
    } while ((ret == HITLS_REC_NORMAL_RECV_BUF_EMPTY ||
            ret == HITLS_REC_NORMAL_IO_BUSY) &&
            (tryNum < 8000)); // A maximum of 4000 calls
    LOG_DEBUG("HiTLS Read Result is %d", ret);
    return ret;
}

int HitlsClose(void *ctx)
{
    return HITLS_Close(ctx);
}

int HitlsRenegotiate(void *ssl)
{
    return HITLS_Renegotiate(ssl);
}

int HitlsSetMtu(void *ssl, uint16_t mtu)
{
    return HITLS_SetMtu(ssl, mtu);
}

int HitlsSetSession(void *ssl, void *session)
{
    return HITLS_SetSession(ssl, session);
}

int HitlsSessionReused(void *ssl)
{
    int32_t ret;
    uint8_t isReused = 0;

    ret = HITLS_IsSessionReused(ssl, &isReused);
    if (ret != HITLS_SUCCESS) {
        return 0;
    }

    return (int)isReused;
}

void *HitlsGet1Session(void *ssl)
{
    return HITLS_GetDupSession(ssl);
}

int HitlsSessionHasTicket(void *session)
{
    return (HITLS_SESS_HasTicket(session) ? 1 : 0);
}

int HitlsSessionIsResumable(void *session)
{
    return (HITLS_SESS_IsResumable(session) ? 1 : 0);
}

void HitlsFreeSession(void *session)
{
    HITLS_SESS_Free(session);
}

int HitlsGetErrorCode(void *ssl)
{
    return HITLS_GetErrorCode(ssl);
}