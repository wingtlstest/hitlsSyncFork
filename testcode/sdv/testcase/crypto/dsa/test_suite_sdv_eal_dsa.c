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

/* BEGIN_HEADER */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "securec.h"
#include "bsl_err.h"
#include "bsl_sal.h"
#include "crypt_errno.h"
#include "crypt_dsa.h"
#include "crypt_eal_pkey.h"
#include "crypt_eal_rand.h"
#include "crypt_bn.h"
#include "eal_pkey_local.h"
#include "stub_replace.h"
#include "crypt_util_rand.h"

#include "crypt_encode.h"
#include "crypt_eal_md.h"
/* END_HEADER */

#define SUCCESS 0
#define ERROR (-1)
#define BITS_OF_BYTE 8
#define CRYPT_EAL_PKEY_KEYMGMT_OPERATE 0
static uint8_t g_kRandBuf[64];
static uint32_t g_kRandBufLen = 0;

int32_t STUB_RandRangeK(BN_BigNum *r, const BN_BigNum *p)
{
    (void)p;
    BN_Bin2Bn(r, g_kRandBuf, g_kRandBufLen);
    return CRYPT_SUCCESS;
}

int Compute_Md(CRYPT_MD_AlgId mdId, Hex *msgIn, Hex *mdOut)
{
    uint32_t outLen;
    CRYPT_EAL_MdCTX *mdCtx = NULL;
    uint32_t mdOutLen = CRYPT_EAL_MdGetDigestSize(mdId);
    ASSERT_TRUE(mdOutLen != 0);
    mdOut->x = (uint8_t *)malloc(mdOutLen);
    ASSERT_TRUE(mdOut->x != NULL);
    mdOut->len = mdOutLen;
    outLen = mdOutLen;
    mdCtx = CRYPT_EAL_MdNewCtx(mdId);
    ASSERT_TRUE_AND_LOG("CRYPT_EAL_MdNewCtx", mdCtx != NULL);
    ASSERT_TRUE_AND_LOG("CRYPT_EAL_MdInit", CRYPT_EAL_MdInit(mdCtx) == 0);
    ASSERT_TRUE_AND_LOG("CRYPT_EAL_MdUpdate", CRYPT_EAL_MdUpdate(mdCtx, msgIn->x, msgIn->len) == 0);
    ASSERT_TRUE_AND_LOG("CRYPT_EAL_MdFinal", CRYPT_EAL_MdFinal(mdCtx, mdOut->x, &outLen) == 0);
    mdOut->len = outLen;
    CRYPT_EAL_MdFreeCtx(mdCtx);
    return SUCCESS;

exit:
    CRYPT_EAL_MdFreeCtx(mdCtx);
    free(mdOut->x);
    mdOut->x = NULL;
    return ERROR;
}

void Set_DSA_Para(
    CRYPT_EAL_PkeyPara *para, CRYPT_EAL_PkeyPrv *prv, CRYPT_EAL_PkeyPub *pub, Hex *P, Hex *Q, Hex *G, Hex *X, Hex *Y)
{
    para->id = CRYPT_PKEY_DSA;
    para->para.dsaPara.p = P->x;
    para->para.dsaPara.pLen = P->len;
    para->para.dsaPara.q = Q->x;
    para->para.dsaPara.qLen = Q->len;
    para->para.dsaPara.g = G->x;
    para->para.dsaPara.gLen = G->len;

    if (prv && X) {
        prv->id = CRYPT_PKEY_DSA;
        prv->key.dsaPrv.data = X->x;
        prv->key.dsaPrv.len = X->len;
    }
    if (pub && Y) {
        pub->id = CRYPT_PKEY_DSA;
        pub->key.dsaPub.data = Y->x;
        pub->key.dsaPub.len = Y->len;
    }
}

static void Set_DSA_Pub(CRYPT_EAL_PkeyPub *pub, uint8_t *key, uint32_t keyLen)
{
    pub->id = CRYPT_PKEY_DSA;
    pub->key.dsaPub.data = key;
    pub->key.dsaPub.len = keyLen;
}

static void Set_DSA_Prv(CRYPT_EAL_PkeyPrv *prv, uint8_t *key, uint32_t keyLen)
{
    prv->id = CRYPT_PKEY_DSA;
    prv->key.dsaPrv.data = key;
    prv->key.dsaPrv.len = keyLen;
}

int SignEncode(
    DSA_Sign *dsaSign, uint8_t *vectorSign, uint32_t *vectorSignLen, Hex *R, Hex *S, BN_BigNum **bn_r, BN_BigNum **bn_s)
{
    *bn_r = BN_Create(R->len * BITS_OF_BYTE);
    *bn_s = BN_Create(S->len * BITS_OF_BYTE);
    ASSERT_EQ(BN_Bin2Bn(*bn_r, R->x, R->len), CRYPT_SUCCESS);
    ASSERT_EQ(BN_Bin2Bn(*bn_s, S->x, S->len), CRYPT_SUCCESS);
    dsaSign->r = *bn_r;
    dsaSign->s = *bn_s;
    ASSERT_EQ(ASN1_SignDataEncode(dsaSign, vectorSign, vectorSignLen), CRYPT_SUCCESS);
    return CRYPT_SUCCESS;

exit:
    return ERROR;
}

/**
 * @test   SDV_CRYPTO_DSA_SET_PARA_API_TC001
 * @title  DSA: CRYPT_EAL_PkeySetPara test.
 * @precon Registering memory-related functions.
 *         Dsa para vertors.
 * @brief
 *    1. Create the context of the dsa algorithm, expected result 1.
 *    2. CRYPT_EAL_PkeySetPara: para = NULL, expected result 2.
 *    3. CRYPT_EAL_PkeySetPara, expected result 3, the parameters are as follows:
 *       (1) p != NULL, pLen = 0
 *       (2) p = NULL, pLen != 0
 *       (3) q != NULL, qLen = 0
 *       (4) q = NULL, qLen != 0
 *       (5) g != NULL, gLen = 0
 *       (6) g = NULL, gLen != 0
 * @expect
 *    1. Success, and context is not NULL.
 *    2. CRYPT_NULL_INPUT
 *    3. CRYPT_EAL_ERR_NEW_PARA_FAIL
 */
/* BEGIN_CASE */
void SDV_CRYPTO_DSA_SET_PARA_API_TC001(Hex *p, Hex *q, Hex *g, int isProvider)
{
    CRYPT_EAL_PkeyCtx *pkey = NULL;
    uint8_t tmp[1];
    CRYPT_EAL_PkeyPara para;
    para.id = CRYPT_PKEY_DSA;
    para.para.dsaPara.p = p->x;
    para.para.dsaPara.pLen = p->len;
    para.para.dsaPara.q = q->x;
    para.para.dsaPara.qLen = q->len;
    para.para.dsaPara.g = g->x;
    para.para.dsaPara.gLen = g->len;

    TestMemInit();
    if (isProvider == 1) {
        pkey = CRYPT_EAL_ProviderPkeyNewCtx(NULL, CRYPT_PKEY_DSA, CRYPT_EAL_PKEY_KEYMGMT_OPERATE, "provider=default");
    } else {
        pkey = CRYPT_EAL_PkeyNewCtx(CRYPT_PKEY_DSA);
    }
    ASSERT_TRUE(pkey != NULL);
    ASSERT_EQ(CRYPT_EAL_PkeySetPara(pkey, NULL), CRYPT_NULL_INPUT);

    if (p->x == NULL) {
        para.para.dsaPara.p = tmp;
        ASSERT_TRUE_AND_LOG("p != NULL, pLen = 0", CRYPT_EAL_PkeySetPara(pkey, &para) == CRYPT_EAL_ERR_NEW_PARA_FAIL);

        para.para.dsaPara.p = p->x;
        para.para.dsaPara.pLen = 128;
        ASSERT_TRUE_AND_LOG("p = NULL, pLen != 0", CRYPT_EAL_PkeySetPara(pkey, &para) == CRYPT_EAL_ERR_NEW_PARA_FAIL);
        para.para.dsaPara.pLen = p->len;
    }
    if (q->x == NULL) {
        para.para.dsaPara.q = tmp;
        ASSERT_TRUE_AND_LOG("q != NULL, qLen = 0", CRYPT_EAL_PkeySetPara(pkey, &para) == CRYPT_EAL_ERR_NEW_PARA_FAIL);

        para.para.dsaPara.q = q->x;
        para.para.dsaPara.qLen = 20;
        ASSERT_TRUE_AND_LOG("q == NULL, qLen != 0", CRYPT_EAL_PkeySetPara(pkey, &para) == CRYPT_EAL_ERR_NEW_PARA_FAIL);
        para.para.dsaPara.qLen = q->len;
    }
    if (g->x == NULL) {
        para.para.dsaPara.g = tmp;
        ASSERT_TRUE_AND_LOG("g != NULL, gLen = 0", CRYPT_EAL_PkeySetPara(pkey, &para) == CRYPT_EAL_ERR_NEW_PARA_FAIL);

        para.para.dsaPara.g = g->x;
        para.para.dsaPara.gLen = 128;
        ASSERT_TRUE_AND_LOG("g!= NULL, gLen != 0", CRYPT_EAL_PkeySetPara(pkey, &para) == CRYPT_EAL_ERR_NEW_PARA_FAIL);
    }
exit:
    CRYPT_EAL_PkeyFreeCtx(pkey);
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_DSA_CMP_API_TC001
 * @title  DSA: CRYPT_EAL_PkeyCmp test.
 * @precon Registering memory-related functions.
 *         Dsa para vertors.
 * @brief
 *    1. Create the contexts(ctx1, ctx2) of the dsa algorithm, expected result 1
 *    2. Call the CRYPT_EAL_PkeyCmp to compare ctx1 and ctx2, expected result 2
 *    3. Set public key and para for ctx1, expected result 3
 *    4. Call the CRYPT_EAL_PkeyCmp to compare ctx1 and ctx2, expected result 4
 *    5. Set public key and para for ctx2, expected result 5
 *    6. Call the CRYPT_EAL_PkeyCmp to compare ctx1 and ctx2, expected result 6
 * @expect
 *    1. Success, and contexts are not NULL.
 *    2. CRYPT_DSA_ERR_KEY_INFO
 *    3. CRYPT_SUCCESS
 *    4. CRYPT_DSA_ERR_KEY_INFO
 *    5. CRYPT_SUCCESS
 *    6. CRYPT_SUCCESS
 */
/* BEGIN_CASE */
void SDV_CRYPTO_DSA_CMP_API_TC001(Hex *p, Hex *q, Hex *g, Hex *y, int isProvider)
{
    CRYPT_EAL_PkeyPara para = {0};
    CRYPT_EAL_PkeyPub pub = {0};
    Set_DSA_Para(&para, NULL, &pub, p, q, g, NULL, y);

    TestMemInit();
    CRYPT_EAL_PkeyCtx *ctx1 = NULL;
    if (isProvider == 1) {
        ctx1 = CRYPT_EAL_ProviderPkeyNewCtx(NULL, CRYPT_PKEY_DSA, CRYPT_EAL_PKEY_KEYMGMT_OPERATE, "provider=default");
    } else {
        ctx1 = CRYPT_EAL_PkeyNewCtx(CRYPT_PKEY_DSA);
    }
    CRYPT_EAL_PkeyCtx *ctx2 = NULL;
    if (isProvider == 1) {
        ctx2 = CRYPT_EAL_ProviderPkeyNewCtx(NULL, CRYPT_PKEY_DSA, CRYPT_EAL_PKEY_KEYMGMT_OPERATE, "provider=default");
    } else {
        ctx2 = CRYPT_EAL_PkeyNewCtx(CRYPT_PKEY_DSA);
    }
    ASSERT_TRUE(ctx1 != NULL && ctx2 != NULL);

    ASSERT_EQ(CRYPT_EAL_PkeyCmp(ctx1, ctx2), CRYPT_DSA_ERR_KEY_INFO);  // no key and no para

    ASSERT_EQ(CRYPT_EAL_PkeySetPara(ctx1, &para), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_PkeySetPub(ctx1, &pub), CRYPT_SUCCESS);

    ASSERT_EQ(CRYPT_EAL_PkeyCmp(ctx1, ctx2), CRYPT_DSA_ERR_KEY_INFO);  // ctx2 no pubkey

    ASSERT_EQ(CRYPT_EAL_PkeySetPara(ctx2, &para), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_PkeySetPub(ctx2, &pub), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_PkeyCmp(ctx1, ctx2), CRYPT_SUCCESS);

exit:
    CRYPT_EAL_PkeyFreeCtx(ctx1);
    CRYPT_EAL_PkeyFreeCtx(ctx2);
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_DSA_CTRL_API_TC001
 * @title  DSA: CRYPT_EAL_PkeyCtrl test.
 * @precon Registering memory-related functions.
 * @brief
 *    1. Create the context(ctx) of the dsa algorithm, expected result 1
 *    2. Call the CRYPT_EAL_PkeyCtrl method:
 *       (1) val = NULL, expected result 2
 *       (2) len = 0, expected result 3
 *       (3) opt = CRYPT_CTRL_SET_RSA_PADDING, expected result 4
 * @expect
 *    1. Success, and contexts are not NULL.
 *    2. CRYPT_NULL_INPUT
 *    3. CRYPT_DSA_UNSUPPORTED_CTRL_OPTION
 *    4. CRYPT_DSA_UNSUPPORTED_CTRL_OPTION
 */
/* BEGIN_CASE */
void SDV_CRYPTO_DSA_CTRL_API_TC001(int isProvider)
{
    CRYPT_EAL_PkeyCtx *ctx = NULL;
    int32_t ref = 1;

    TestMemInit();
    if (isProvider == 1) {
        ctx = CRYPT_EAL_ProviderPkeyNewCtx(NULL, CRYPT_PKEY_DSA, CRYPT_EAL_PKEY_KEYMGMT_OPERATE, "provider=default");
    } else {
        ctx = CRYPT_EAL_PkeyNewCtx(CRYPT_PKEY_DSA);
    }
    ASSERT_TRUE(ctx != NULL);

    ASSERT_EQ(CRYPT_EAL_PkeyCtrl(ctx, CRYPT_CTRL_UP_REFERENCES, NULL, sizeof(uint32_t)), CRYPT_INVALID_ARG);
    ASSERT_EQ(CRYPT_EAL_PkeyCtrl(ctx, CRYPT_CTRL_UP_REFERENCES, &ref, 0), CRYPT_INVALID_ARG);
    ASSERT_EQ(
        CRYPT_EAL_PkeyCtrl(ctx, CRYPT_CTRL_SET_RSA_PADDING, &ref, sizeof(int32_t)), CRYPT_DSA_UNSUPPORTED_CTRL_OPTION);

exit:
    CRYPT_EAL_PkeyFreeCtx(ctx);
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_DSA_GET_PARA_API_TC001
 * @title  DSA: CRYPT_EAL_PkeyGetPara test.
 * @precon Registering memory-related functions.
 *         Dsa para vertors.
 * @brief
 *    1. Create the contexts of the dsa algorithm, expected result 1.
 *    2. Call the CRYPT_EAL_PkeySetPara method with correct parameters, expected result 2
 *    3. Call the CRYPT_EAL_PkeySetPara method: para.id != pkey.id, expected result 3
 *    4. Call the CRYPT_EAL_PkeySetPara method: pkey=NULL or para=NULL, expected result 4
 *    5. Call the CRYPT_EAL_PkeySetPara method with correct parameters, expected result 5
 *    6. Check whether the configured parameters are the same as the obtained parameters, expected result 6
 * @expect
 *    1. Success, and context is not NULL.
 *    2. CRYPT_SUCCESS
 *    3. CRYPT_EAL_ERR_ALGID
 *    4. CRYPT_NULL_INPUT
 *    5. CRYPT_SUCCESS
 *    6. Parameters are equal.
 */
/* BEGIN_CASE */
void SDV_CRYPTO_DSA_GET_PARA_API_TC001(Hex *p, Hex *q, Hex *g, int isProvider)
{
    uint8_t buf_p[1030] = {0};
    uint32_t bufLen = sizeof(buf_p);
    uint8_t buf_q[1030] = {0};
    uint8_t buf_g[1030] = {0};
    Hex getP = {buf_p, bufLen};
    Hex getQ = {buf_q, bufLen};
    Hex getG = {buf_g, bufLen};

    CRYPT_EAL_PkeyPara para1 = {0};
    CRYPT_EAL_PkeyPara para2 = {0};
    Set_DSA_Para(&para1, NULL, NULL, p, q, g, NULL, NULL);
    Set_DSA_Para(&para2, NULL, NULL, &getP, &getQ, &getG, NULL, NULL);
    TestMemInit();
    CRYPT_EAL_PkeyCtx *pKey = NULL;
    if (isProvider == 1) {
        pKey = CRYPT_EAL_ProviderPkeyNewCtx(NULL, CRYPT_PKEY_DSA, CRYPT_EAL_PKEY_KEYMGMT_OPERATE, "provider=default");
    } else {
        pKey = CRYPT_EAL_PkeyNewCtx(CRYPT_PKEY_DSA);
    }
    ASSERT_TRUE(pKey != NULL);

    para2.id = CRYPT_PKEY_RSA;
    ASSERT_TRUE(CRYPT_EAL_PkeySetPara(pKey, &para1) == CRYPT_SUCCESS);

    ASSERT_TRUE(CRYPT_EAL_PkeyGetPara(pKey, &para2) == CRYPT_EAL_ERR_ALGID);

    ASSERT_TRUE(CRYPT_EAL_PkeyGetPara(NULL, &para2) == CRYPT_NULL_INPUT);
    ASSERT_TRUE(CRYPT_EAL_PkeyGetPara(pKey, NULL) == CRYPT_NULL_INPUT);

    para2.id = CRYPT_PKEY_DSA;
    ASSERT_TRUE(CRYPT_EAL_PkeyGetPara(pKey, &para2) == CRYPT_SUCCESS);
    ASSERT_TRUE(para1.para.dsaPara.pLen == para2.para.dsaPara.pLen);
    ASSERT_TRUE(memcmp(para1.para.dsaPara.p, para2.para.dsaPara.p, para1.para.dsaPara.pLen) == 0);
    ASSERT_TRUE(para1.para.dsaPara.qLen == para2.para.dsaPara.qLen);
    ASSERT_TRUE(memcmp(para1.para.dsaPara.q, para2.para.dsaPara.q, para1.para.dsaPara.qLen) == 0);
    ASSERT_TRUE(para1.para.dsaPara.gLen == para2.para.dsaPara.gLen);
    ASSERT_TRUE(memcmp(para1.para.dsaPara.g, para2.para.dsaPara.g, para1.para.dsaPara.gLen) == 0);

exit:
    CRYPT_EAL_PkeyFreeCtx(pKey);
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_DSA_SIGN_VERIFY_FUNC_TC001
 * @title  DSA: Set(or copy) the key, sign, and verify the signature.
 * @precon Registering memory-related functions.
 *         Dsa vertors.
 * @brief
 *    1. Mock BN_RandRange method to generate vector K.
 *    2. Create the context of the dsa algorithm, expected result 1.
 *    3. Set para, private key and public key, expected result 2.
 *    4. Call the CRYPT_EAL_PkeyGetSignLen method to get sign length, expected result 3.
 *    5. Allocate the memory for the signature, expected result 4.
 *    6. Encoding r and s vectors, expected result 5.
 *    7. Sign and compare the signatures of hitls and vector, expected result 6.
 *    8. Verify, expected result 7.
 *    9. Copy the ctx and repeat steps 7 through 8.
 * @expect
 *    1. Success, and context is not NULL.
 *    2. CRYPT_SUCCESS
 *    3. signLen > 0
 *    4. Success
 *    5. Success
 *    6. CRYPT_SUCCESS, the two signatures are the same.
 *    7. CRYPT_SUCCESS
 */
/* BEGIN_CASE */
void SDV_CRYPTO_DSA_SIGN_VERIFY_FUNC_TC001(
    int hashId, Hex *P, Hex *Q, Hex *G, Hex *Msg, Hex *X, Hex *Y, Hex *K, Hex *R, Hex *S, int isProvider)
{
    if (IsMdAlgDisabled(hashId)) {
        SKIP_TEST();
    }
    uint32_t signLen;
    uint8_t *vectorSign = NULL;
    uint8_t *hitlsSign = NULL;
    uint32_t vectorSignLen, hitlsSignOutLen;
    BN_BigNum *bn_r = NULL;
    BN_BigNum *bn_s = NULL;
    CRYPT_EAL_PkeyCtx *pkey = NULL;
    CRYPT_EAL_PkeyCtx *cpyCtx = NULL;

    CRYPT_EAL_PkeyPara para = {0};
    CRYPT_EAL_PkeyPrv prv = {0};
    CRYPT_EAL_PkeyPub pub = {0};
    Set_DSA_Para(&para, &prv, &pub, P, Q, G, X, Y);

    FuncStubInfo tmpRpInfo;
    ASSERT_EQ(memcpy_s(g_kRandBuf, sizeof(g_kRandBuf), K->x, K->len), 0);
    g_kRandBufLen = K->len;
    STUB_Init();
    STUB_Replace(&tmpRpInfo, BN_RandRange, STUB_RandRangeK);

    TestMemInit();
    if (isProvider == 1) {
        pkey = CRYPT_EAL_ProviderPkeyNewCtx(NULL, CRYPT_PKEY_DSA,
            CRYPT_EAL_PKEY_KEYMGMT_OPERATE + CRYPT_EAL_PKEY_SIGN_OPERATE, "provider=default");
    } else {
        pkey = CRYPT_EAL_PkeyNewCtx(CRYPT_PKEY_DSA);
    }
    ASSERT_TRUE(pkey != NULL);
    ASSERT_EQ(CRYPT_EAL_PkeySetPara(pkey, &para), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_PkeySetPrv(pkey, &prv), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_PkeySetPub(pkey, &pub), CRYPT_SUCCESS);

    signLen = CRYPT_EAL_PkeyGetSignLen(pkey);
    ASSERT_TRUE(signLen > 0);

    /* Encoding r and s vectors */
    DSA_Sign dsaSign = {0};
    vectorSign = (uint8_t *)malloc(signLen);
    vectorSignLen = signLen;
    ASSERT_EQ(SignEncode(&dsaSign, vectorSign, &vectorSignLen, R, S, &bn_r, &bn_s), CRYPT_SUCCESS);

    /* Sign */
    hitlsSign = (uint8_t *)malloc(signLen);
    hitlsSignOutLen = signLen;
    ASSERT_EQ(CRYPT_EAL_PkeySign(pkey, hashId, Msg->x, Msg->len, hitlsSign, &hitlsSignOutLen), CRYPT_SUCCESS);

    /* Compare the signatures of hitls and vector. */
    ASSERT_EQ(hitlsSignOutLen, vectorSignLen);
    ASSERT_EQ(memcmp(vectorSign, hitlsSign, hitlsSignOutLen), 0);

    /* Verify */
    ASSERT_EQ(CRYPT_EAL_PkeyVerify(pkey, hashId, Msg->x, Msg->len, hitlsSign, hitlsSignOutLen), CRYPT_SUCCESS);

    /* Copy the ctx and verify the signature. */
    cpyCtx = BSL_SAL_Calloc(1u, sizeof(CRYPT_EAL_PkeyCtx));
    ASSERT_TRUE(cpyCtx != NULL);
    ASSERT_EQ(CRYPT_EAL_PkeyCopyCtx(cpyCtx, pkey), CRYPT_SUCCESS);
    hitlsSignOutLen = signLen;
    ASSERT_EQ(CRYPT_EAL_PkeySign(cpyCtx, hashId, Msg->x, Msg->len, hitlsSign, &hitlsSignOutLen), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_PkeyVerify(cpyCtx, hashId, Msg->x, Msg->len, hitlsSign, hitlsSignOutLen), CRYPT_SUCCESS);
exit:
    STUB_Reset(&tmpRpInfo);
    free(vectorSign);
    free(hitlsSign);
    BN_Destroy(bn_r);
    BN_Destroy(bn_s);
    BSL_ERR_RemoveErrorStack(true);
    CRYPT_EAL_PkeyFreeCtx(pkey);
    CRYPT_EAL_PkeyFreeCtx(cpyCtx);
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_DSA_SIGN_VERIFY_DATA_FUNC_TC001
 * @title  DSA sets the key and performs signature and signature verification tests on the hash data.
 * @precon Registering memory-related functions.
 *         Dsa vertors.
 * @brief
 *    1. Mock BN_RandRange method to generate vector K.
 *    2. Create the context of the dsa algorithm, expected result 1.
 *    3. Set para, private key and public key, expected result 2.
 *    4. Call the CRYPT_EAL_PkeyGetSignLen method to get sign length, expected result 3.
 *    5. Allocate the memory for the signature, expected result 4.
 *    6. Encoding r and s vectors, expected result 5.
 *    7. Compute the hash of the msg, sign and compare the signatures of hitls and vector, expected result 6.
 *    8. Verify, expected result 7.
 * @expect
 *    1. Success, and context is not NULL.
 *    2. CRYPT_SUCCESS
 *    3. signLen > 0
 *    4. Success
 *    5. Success
 *    6. CRYPT_SUCCESS, the two signatures are the same.
 */
/* BEGIN_CASE */
void SDV_CRYPTO_DSA_SIGN_VERIFY_DATA_FUNC_TC001(
    int hashId, Hex *P, Hex *Q, Hex *G, Hex *Msg, Hex *X, Hex *Y, Hex *K, Hex *R, Hex *S, int isProvider)
{
    if (IsMdAlgDisabled(hashId)) {
        SKIP_TEST();
    }
    uint32_t signLen;
    uint8_t *vectorSign = NULL;
    uint8_t *hitlsSign = NULL;
    uint32_t vectorSignLen, hitlsSignOutLen;
    BN_BigNum *bn_r = NULL;
    BN_BigNum *bn_s = NULL;
    CRYPT_EAL_PkeyCtx *pkey = NULL;
    Hex mdOut = {0};

    FuncStubInfo tmpRpInfo;
    ASSERT_EQ(memcpy_s(g_kRandBuf, sizeof(g_kRandBuf), K->x, K->len), 0);
    g_kRandBufLen = K->len;
    STUB_Init();
    STUB_Replace(&tmpRpInfo, BN_RandRange, STUB_RandRangeK);

    CRYPT_EAL_PkeyPara para;
    CRYPT_EAL_PkeyPrv prv = {0};
    CRYPT_EAL_PkeyPub pub;
    Set_DSA_Para(&para, &prv, &pub, P, Q, G, X, Y);

    TestMemInit();
    if (isProvider == 1) {
        pkey = CRYPT_EAL_ProviderPkeyNewCtx(NULL, CRYPT_PKEY_DSA,
            CRYPT_EAL_PKEY_KEYMGMT_OPERATE + CRYPT_EAL_PKEY_SIGN_OPERATE, "provider=default");
    } else {
        pkey = CRYPT_EAL_PkeyNewCtx(CRYPT_PKEY_DSA);
    }
    ASSERT_TRUE(pkey != NULL);
    ASSERT_EQ(CRYPT_EAL_PkeySetPara(pkey, &para), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_PkeySetPrv(pkey, &prv), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_PkeySetPub(pkey, &pub), CRYPT_SUCCESS);

    signLen = CRYPT_EAL_PkeyGetSignLen(pkey);
    ASSERT_TRUE(signLen > 0);

    /* Encoding r and s vectors */
    DSA_Sign dsaSign;
    vectorSign = (uint8_t *)malloc(signLen);
    vectorSignLen = signLen;
    ASSERT_EQ(SignEncode(&dsaSign, vectorSign, &vectorSignLen, R, S, &bn_r, &bn_s), CRYPT_SUCCESS);

    /* Calculates the hash of the msg. */
    ASSERT_EQ(Compute_Md(hashId, Msg, &mdOut), SUCCESS);

    /* Sign */
    hitlsSign = (uint8_t *)malloc(signLen);
    hitlsSignOutLen = signLen;
    ASSERT_EQ(CRYPT_EAL_PkeySignData(pkey, mdOut.x, mdOut.len, hitlsSign, &hitlsSignOutLen), CRYPT_SUCCESS);

    /* Compare the signatures of hitls and vector. */
    ASSERT_EQ(hitlsSignOutLen, vectorSignLen);
    ASSERT_EQ(memcmp(vectorSign, hitlsSign, hitlsSignOutLen), 0);

    /* Verify the signature of the hash data. */
    ASSERT_EQ(CRYPT_EAL_PkeyVerifyData(pkey, mdOut.x, mdOut.len, hitlsSign, hitlsSignOutLen), CRYPT_SUCCESS);
exit:
    STUB_Reset(&tmpRpInfo);
    if (mdOut.x != NULL) {
        free(mdOut.x);
    }
    free(vectorSign);
    free(hitlsSign);
    BN_Destroy(bn_r);
    BN_Destroy(bn_s);
    BSL_ERR_RemoveErrorStack(true);
    CRYPT_EAL_PkeyFreeCtx(pkey);
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_DSA_GEN_FUNC_TC001
 * @title  DSA function test (gen a key pair).
 * @precon Registering memory-related functions.
 *         Dsa vertors.
 * @brief
 *    1. Init the drbg, expected result 1.
 *    2. Create the context(ctx) of the DSA algorithm, expected result 2.
 *    3. Set para for dsa, expected result 3.
 *    4. Generate a key pair, expected result 4.
 *    5. Call the CRYPT_EAL_PkeyGetSignLen method to get sign length, expected result 5.
 *    6. Allocate the memory for the signature, expected result 6.
 *    7. Sign, expected result 7.
 *    8. Verify, expected result 8.
 * @expect
 *    1. CRYPT_SUCCESS
 *    2. Success, and two contexts are not NULL.
 *    3. CRYPT_SUCCESS
 *    4. CRYPT_SUCCESS
 *    5. signLen > 0
 *    6. Success
 *    7. CRYPT_SUCCESS
 *    8. CRYPT_SUCCESS
 */
/* BEGIN_CASE */
void SDV_CRYPTO_DSA_GEN_FUNC_TC001(Hex *p, Hex *q, Hex *g, Hex *data, int isProvider)
{
    CRYPT_EAL_PkeyCtx *ctx = NULL;
    CRYPT_EAL_PkeyPara para = {0};
    uint8_t *sign = NULL;
    uint32_t signLen;

    Set_DSA_Para(&para, NULL, NULL, p, q, g, NULL, NULL);

    TestMemInit();
    ASSERT_EQ(TestRandInit(), CRYPT_SUCCESS);

    if (isProvider == 1) {
        ctx = CRYPT_EAL_ProviderPkeyNewCtx(NULL, CRYPT_PKEY_DSA,
            CRYPT_EAL_PKEY_KEYMGMT_OPERATE + CRYPT_EAL_PKEY_SIGN_OPERATE, "provider=default");
    } else {
        ctx = CRYPT_EAL_PkeyNewCtx(CRYPT_PKEY_DSA);
    }
    ASSERT_TRUE(ctx != NULL);

    ASSERT_EQ(CRYPT_EAL_PkeySetPara(ctx, &para), CRYPT_SUCCESS);

    ASSERT_EQ(CRYPT_EAL_PkeyGen(ctx), CRYPT_SUCCESS);

    signLen = CRYPT_EAL_PkeyGetSignLen(ctx);
    ASSERT_TRUE(signLen > 0);
    sign = (uint8_t *)malloc(signLen);

    ASSERT_EQ(CRYPT_EAL_PkeySign(ctx, CRYPT_MD_SHA256, data->x, data->len, sign, &signLen), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_PkeyVerify(ctx, CRYPT_MD_SHA256, data->x, data->len, sign, signLen), CRYPT_SUCCESS);
exit:
    CRYPT_EAL_PkeyFreeCtx(ctx);
    free(sign);
    CRYPT_EAL_RandDeinit();
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_DSA_DUP_CTX_FUNC_TC001
 * @title  DSA: CRYPT_EAL_PkeyDupCtx test.
 * @precon Registering memory-related functions.
 *         Dsa vertors.
 * @brief
 *    1. Create the context of the dsa algorithm, expected result 1.
 *    2. Init the drbg, expected result 2.
 *    3. Set para and generate a key pair, expected result 3.
 *    4. Call the CRYPT_EAL_PkeyDupCtx method to dup dsa context, expected result 4.
 *    5. Call the CRYPT_EAL_PkeyCmp method to compare public key, expected result 5.
 *    6. Call the CRYPT_EAL_PkeyGetKeyBits to get keyLen from contexts, expected result 6.
 *    7. Call the CRYPT_EAL_PkeyGetPub method to obtain the public key from the contexts, expected result 7.
 *    8. Compare public keys, expected result 8.
 *    9. Call the CRYPT_EAL_PkeyGetPrv method to obtain the private key from the contexts, expected result 9.
 *    10. Compare privates keys, expected result 10.
 * @expect
 *    1. Success, and context is not NULL.
 *    2-3. CRYPT_SUCCESS
 *    4. Success, and context is not NULL.
 *    5. CRYPT_SUCCESS
 *    6. The key length obtained from both contexts is the same.
 *    7. CRYPT_SUCCESS
 *    8. The two public keys are the same.
 *    9. CRYPT_SUCCESS
 *    10. The two private keys are the same.
 */
/* BEGIN_CASE */
void SDV_CRYPTO_DSA_DUP_CTX_FUNC_TC001(Hex *p, Hex *q, Hex *g, int isProvider)
{
    uint8_t *key1 = NULL;
    uint8_t *key2 = NULL;
    uint32_t keyLen1, keyLen2;
    CRYPT_EAL_PkeyPara para = {0};
    CRYPT_EAL_PkeyPub pub1, pub2;
    CRYPT_EAL_PkeyPrv prv1, prv2;
    CRYPT_EAL_PkeyCtx *ctx = NULL;
    CRYPT_EAL_PkeyCtx *dupCtx = NULL;

    Set_DSA_Para(&para, NULL, NULL, p, q, g, NULL, NULL);

    TestMemInit();
    if (isProvider == 1) {
        ctx = CRYPT_EAL_ProviderPkeyNewCtx(NULL, CRYPT_PKEY_DSA, CRYPT_EAL_PKEY_KEYMGMT_OPERATE, "provider=default");
    } else {
        ctx = CRYPT_EAL_PkeyNewCtx(CRYPT_PKEY_DSA);
    }
    ASSERT_TRUE(ctx != NULL);

    ASSERT_EQ(TestRandInit(), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_PkeySetPara(ctx, &para), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_PkeyGen(ctx), CRYPT_SUCCESS);

    dupCtx = CRYPT_EAL_PkeyDupCtx(ctx);
    ASSERT_TRUE(dupCtx != NULL);

    ASSERT_EQ(CRYPT_EAL_PkeyCmp(ctx, dupCtx), CRYPT_SUCCESS);

    keyLen1 = CRYPT_EAL_PkeyGetKeyBits(ctx);
    keyLen2 = CRYPT_EAL_PkeyGetKeyBits(dupCtx);
    ASSERT_EQ(keyLen1, keyLen2);

    key1 = calloc(1u, keyLen1);
    key2 = calloc(1u, keyLen2);
    ASSERT_TRUE(key1 != NULL && key2 != NULL);

    Set_DSA_Pub(&pub1, key1, keyLen1);
    Set_DSA_Pub(&pub2, key2, keyLen2);
    ASSERT_EQ(CRYPT_EAL_PkeyGetPub(ctx, &pub1), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_PkeyGetPub(dupCtx, &pub2), CRYPT_SUCCESS);
    ASSERT_COMPARE("Compare public key", key1, pub1.key.dsaPub.len, key2, pub2.key.dsaPub.len);

    Set_DSA_Prv(&prv1, key1, keyLen1);
    Set_DSA_Prv(&prv2, key2, keyLen2);
    ASSERT_EQ(CRYPT_EAL_PkeyGetPrv(ctx, &prv1), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_PkeyGetPrv(dupCtx, &prv2), CRYPT_SUCCESS);
    ASSERT_COMPARE("Compare private key", key1, prv1.key.dsaPrv.len, key2, prv2.key.dsaPrv.len);

exit:
    CRYPT_EAL_RandDeinit();
    CRYPT_EAL_PkeyFreeCtx(ctx);
    CRYPT_EAL_PkeyFreeCtx(dupCtx);
    BSL_SAL_Free(key1);
    BSL_SAL_Free(key2);
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_DSA_KEY_PAIR_CHECK_FUNC_TC001
 * @title  DSA: key pair check.
 * @precon Registering memory-related functions.
 * @brief
 *    1. Create two contexts(pubCtx, prvCtx) of the dsa algorithm, expected result 1
 *    2. Set para and public key for pubCtx, expected result 2
 *    3. Set para and private key for prvCtx, expected result 3
 *    4. Init the drbg, expected result 5, expected result 4
 *    5. Check whether the public key matches the private key, expected result 5
 * @expect
 *    1. Success, and contexts are not NULL.
 *    2-4. CRYPT_SUCCESS
 *    5. Return CRYPT_SUCCESS when expect is 1, CRYPT_DSA_VERIFY_FAIL otherwise.
 */
/* BEGIN_CASE */
void SDV_CRYPTO_DSA_KEY_PAIR_CHECK_FUNC_TC001(Hex *P, Hex *Q, Hex *G, Hex *X, Hex *Y, int expect)
{
    CRYPT_EAL_PkeyCtx *pubCtx = NULL;
    CRYPT_EAL_PkeyCtx *prvCtx = NULL;
    CRYPT_EAL_PkeyPara para = {0};
    CRYPT_EAL_PkeyPrv prv = {0};
    CRYPT_EAL_PkeyPub pub = {0};
    int expectRet = expect == 1 ? CRYPT_SUCCESS : CRYPT_DSA_VERIFY_FAIL;

    Set_DSA_Para(&para, &prv, &pub, P, Q, G, X, Y);

    TestMemInit();
    pubCtx = CRYPT_EAL_PkeyNewCtx(CRYPT_PKEY_DSA);
    prvCtx = CRYPT_EAL_PkeyNewCtx(CRYPT_PKEY_DSA);
    ASSERT_TRUE(pubCtx != NULL && prvCtx != NULL);

    ASSERT_EQ(CRYPT_EAL_PkeySetPara(pubCtx, &para), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_PkeySetPub(pubCtx, &pub), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_PkeySetPara(prvCtx, &para), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_PkeySetPrv(prvCtx, &prv), CRYPT_SUCCESS);

    ASSERT_EQ(TestRandInit(), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_PkeyPairCheck(pubCtx, prvCtx), expectRet);

exit:
    CRYPT_EAL_RandDeinit();
    CRYPT_EAL_PkeyFreeCtx(pubCtx);
    CRYPT_EAL_PkeyFreeCtx(prvCtx);
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_DSA_GET_KEY_BITS_FUNC_TC001
 * @title  DSA: get key bits.
 * @brief
 *    1. Create a context of the DSA algorithm, expected result 1
 *    2. Get key bits, expected result 2
 * @expect
 *    1. Success, and context is not NULL.
 *    2. Equal to keyBits.
 */
/* BEGIN_CASE */
void SDV_CRYPTO_DSA_GET_KEY_BITS_FUNC_TC001(int id, int keyBits, Hex *P, Hex *Q, Hex *G, int isProvider)
{
    CRYPT_EAL_PkeyCtx *pkey = NULL;
    if (isProvider == 1) {
        pkey = CRYPT_EAL_ProviderPkeyNewCtx(NULL, id, CRYPT_EAL_PKEY_KEYMGMT_OPERATE, "provider=default");
    } else {
        pkey = CRYPT_EAL_PkeyNewCtx(id);
    }
    ASSERT_TRUE(pkey != NULL);
    CRYPT_EAL_PkeyPara para;
    para.id = CRYPT_PKEY_DSA;
    para.para.dsaPara.p = P->x;
    para.para.dsaPara.pLen = P->len;
    para.para.dsaPara.q = Q->x;
    para.para.dsaPara.qLen = Q->len;
    para.para.dsaPara.g = G->x;
    para.para.dsaPara.gLen = G->len;

    ASSERT_TRUE(CRYPT_EAL_PkeySetPara(pkey, &para) == CRYPT_SUCCESS);
    ASSERT_TRUE(CRYPT_EAL_PkeyGetKeyBits(pkey) == (uint32_t)keyBits);
exit:
    CRYPT_EAL_PkeyFreeCtx(pkey);
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_DSA_GET_SEC_BITS_FUNC_TC001
 * @title  DSA CRYPT_EAL_PkeyGetSecurityBits test.
 * @precon nan
 * @brief
 *    1. Create the context of the dsa algorithm, expected result 1
 *    2. Set dsa para, expected result 2
 *    3. Call the CRYPT_EAL_PkeyGetSecurityBits Obtains secbits, expected result 3
 * @expect
 *    1. Success, and the context is not null.
 *    2. CRYPT_SUCCESS
 *    3. The return value is secBits.
 */
/* BEGIN_CASE */
void SDV_CRYPTO_DSA_GET_SEC_BITS_FUNC_TC001(int id, int secBits, Hex *P, Hex *Q, Hex *G)
{
    CRYPT_EAL_PkeyCtx *pkey = CRYPT_EAL_PkeyNewCtx(id);
    ASSERT_TRUE(pkey != NULL);
    CRYPT_EAL_PkeyPara para;
    para.id = CRYPT_PKEY_DSA;
    para.para.dsaPara.p = P->x;
    para.para.dsaPara.pLen = P->len;
    para.para.dsaPara.q = Q->x;
    para.para.dsaPara.qLen = Q->len;
    para.para.dsaPara.g = G->x;
    para.para.dsaPara.gLen = G->len;

    ASSERT_TRUE(CRYPT_EAL_PkeySetPara(pkey, &para) == CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_PkeyGetSecurityBits(pkey), secBits);
exit:
    CRYPT_EAL_PkeyFreeCtx(pkey);
}
/* END_CASE */
