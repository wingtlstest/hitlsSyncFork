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
/* INCLUDE_BASE test_suite_sdv_eal_sm2 */

/* BEGIN_HEADER */

#include "crypt_local_types.h"
#include "crypt_sm2.h"
#include "crypt_encode.h"

#define MAX_PLAIN_TEXT_LEN 2048
#define CIPHER_TEXT_EXTRA_LEN 97

/* END_HEADER */

/**
 * @test   SDV_CRYPTO_SM2_ENC_API_TC001
 * @title  SM2 CRYPT_EAL_PkeyEncrypt: Test the validity of input parameters.
 * @precon Vector: public key.
 * @brief
 *    1. Init the DRBG and create the context of the SM2 algorithm, expected result 1
 *    2. Call the CRYPT_EAL_PkeyEncrypt method, where all parameters are valid, expected result 2
 *    3. Set public key, expected result 3
 *    4. Call the CRYPT_EAL_PkeyEncrypt method:
 *       (1) data = NULL, dataLen != 0, expected result 4
 *       (2) data = NULL, dataLen = 0, expected result 5
 *       (3) output = NULL, outLen != 0, expected result 6
 *       (4) outLen = NULL, expected result 7
 *       (5) outLen is not enough(less than 32+97), expected result 8
 *       (6) all parameters are valid, expected result 9
 * @expect
 *    1. Success, and context is not NULL.
 *    2. CRYPT_SM2_NO_PUBKEY
 *    3. CRYPT_SUCCESS
 *    4-7. CRYPT_NULL_INPUT
 *    8. CRYPT_SM2_BUFF_LEN_NOT_ENOUGH
 *    9. CRYPT_SUCCESS
 */
/* BEGIN_CASE */
void SDV_CRYPTO_SM2_ENC_API_TC001(Hex *pubKey)
{
    uint8_t plainText[32];
    uint8_t cipherText[141];  // 32 + 97 + 12
    uint32_t outLen = sizeof(cipherText);
    CRYPT_EAL_PkeyPub pub = {0};
    SetSm2PubKey(&pub, pubKey->x, pubKey->len);

    TestMemInit();
    CRYPT_RandRegist(RandFunc);
    CRYPT_EAL_PkeyCtx *ctx = CRYPT_EAL_PkeyNewCtx(CRYPT_PKEY_SM2);
    ASSERT_TRUE(ctx != NULL);

    ASSERT_TRUE(CRYPT_EAL_PkeyEncrypt(ctx, plainText, sizeof(plainText), cipherText, &outLen) == CRYPT_SM2_NO_PUBKEY);

    ASSERT_TRUE(CRYPT_EAL_PkeySetPub(ctx, &pub) == CRYPT_SUCCESS);

    ASSERT_TRUE(CRYPT_EAL_PkeyEncrypt(ctx, NULL, sizeof(plainText), cipherText, &outLen) == CRYPT_NULL_INPUT);
    ASSERT_TRUE(CRYPT_EAL_PkeyEncrypt(ctx, NULL, 0, cipherText, &outLen) == CRYPT_NULL_INPUT);
    ASSERT_TRUE(CRYPT_EAL_PkeyEncrypt(ctx, plainText, sizeof(plainText), NULL, &outLen) == CRYPT_NULL_INPUT);
    ASSERT_TRUE(CRYPT_EAL_PkeyEncrypt(ctx, plainText, sizeof(plainText), cipherText, NULL) == CRYPT_NULL_INPUT);

    outLen = sizeof(cipherText) - 12;
    ASSERT_TRUE(
        CRYPT_EAL_PkeyEncrypt(ctx, plainText, sizeof(plainText), cipherText, &outLen) == CRYPT_SM2_BUFF_LEN_NOT_ENOUGH);
    outLen = sizeof(cipherText);
    ASSERT_TRUE(CRYPT_EAL_PkeyEncrypt(ctx, plainText, sizeof(plainText), cipherText, &outLen) == CRYPT_SUCCESS);

exit:
    CRYPT_EAL_PkeyFreeCtx(ctx);
    CRYPT_RandRegist(NULL);
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_SM2_ENC_API_TC002
 * @title  SM2: CRYPT_EAL_PkeyEncrypt test: Random number error.
 * @precon Vertor: public key.
 * @brief
 *    1. Create the context of the SM2 algorithm, expected result 1.
 *    2. Set public key, expected result 2.
 *    3. Call the CRYPT_EAL_PkeyEncrypt method, where all parameters are valid, expected result 3.
 *    4. Register wrong rand method: FakeRandFunc(The random number it generated is 0), expected result 4.
 *    5. Call the CRYPT_EAL_PkeyEncrypt method, where all parameters are valid, expected result 5.
 *    6. Register correct rand method and Call the CRYPT_EAL_PkeyEncrypt method to signature, expected result 6.
 * @expect
 *    1. Success, and context is not NULL.
 *    2. CRYPT_SUCCESS
 *    3. CRYPT_NO_REGIST_RAND
 *    4. CRYPT_SUCCESS
 *    5. CRYPT_SM2_ERR_TRY_CNT.
 *    6. CRYPT_SUCCESS
 */
/* BEGIN_CASE */
void SDV_CRYPTO_SM2_ENC_API_TC002(Hex *pubKey)
{
    uint8_t plainText[32];
    uint8_t cipherText[141];  // 32 + 97 + 12
    uint32_t outLen = sizeof(cipherText);
    uint8_t zero[100] = {0};
    CRYPT_EAL_PkeyPub pub = {0};
    SetSm2PubKey(&pub, pubKey->x, pubKey->len);

    TestMemInit();
    CRYPT_EAL_PkeyCtx *ctx = CRYPT_EAL_PkeyNewCtx(CRYPT_PKEY_SM2);
    ASSERT_TRUE(ctx != NULL);

    ASSERT_TRUE(CRYPT_EAL_PkeySetPub(ctx, &pub) == CRYPT_SUCCESS);
    ASSERT_TRUE(CRYPT_EAL_PkeyEncrypt(ctx, plainText, sizeof(plainText), cipherText, &outLen) == CRYPT_NO_REGIST_RAND);

    CRYPT_RandRegist(FakeRandFunc);
    ASSERT_TRUE(SetFakeRandOutput(zero, sizeof(zero)) == CRYPT_SUCCESS);
    ASSERT_TRUE(CRYPT_EAL_PkeyEncrypt(ctx, plainText, sizeof(plainText), cipherText, &outLen) == CRYPT_SM2_ERR_TRY_CNT);

    CRYPT_RandRegist(RandFunc);
    ASSERT_TRUE(CRYPT_EAL_PkeyEncrypt(ctx, plainText, sizeof(plainText), cipherText, &outLen) == CRYPT_SUCCESS);

exit:
    CRYPT_EAL_PkeyFreeCtx(ctx);
    CRYPT_RandRegist(NULL);
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_SM2_DEC_API_TC001
 * @title  SM2 CRYPT_EAL_PkeyDecrypt: Test the validity of input parameters.
 * @precon Vector: private key, ciphertext.
 * @brief
 *    1. Create the context of the SM2 algorithm, expected result 1
 *    2. Call the CRYPT_EAL_PkeyDecrypt method, where all parameters are valid, expected result 2
 *    3. Set private key, expected result 3
 *    4. Call the CRYPT_EAL_PkeyDecrypt method:
 *       (1) data = NULL, dataLen != 0, expected result 4
 *       (2) output = NULL, outLen != 0, expected result 5
 *       (3) outLen = NULL, expected result 6
 *       (4) data = NULL, dataLen = 0, expected result 7
 *       (5) the length of ciphertext is too long, expected result 8
 *       (6) all parameters are valid, expected result 9
 * @expect
 *    1. Success, and context is not NULL.
 *    2. CRYPT_SM2_NO_PRVKEY
 *    3. CRYPT_SUCCESS
 *    4-7. CRYPT_NULL_INPUT
 *    8. CRYPT_SM2_BUFF_LEN_NOT_ENOUGH
 *    9. CRYPT_SUCCESS
 */
/* BEGIN_CASE */
void SDV_CRYPTO_SM2_DEC_API_TC001(Hex *prvKey, Hex *cipherText)
{
    uint8_t plainText[MAX_PLAIN_TEXT_LEN];
    uint32_t outLen = cipherText->len;
    CRYPT_EAL_PkeyPrv prv = {0};
    SetSm2PrvKey(&prv, prvKey->x, prvKey->len);

    TestRandInit();
    TestMemInit();
    CRYPT_EAL_PkeyCtx *ctx = CRYPT_EAL_PkeyNewCtx(CRYPT_PKEY_SM2);
    ASSERT_TRUE(ctx != NULL);

    ASSERT_TRUE(CRYPT_EAL_PkeyDecrypt(ctx, cipherText->x, cipherText->len, plainText, &outLen) == CRYPT_SM2_NO_PRVKEY);

    ASSERT_TRUE(CRYPT_EAL_PkeySetPrv(ctx, &prv) == CRYPT_SUCCESS);

    ASSERT_TRUE(CRYPT_EAL_PkeyDecrypt(ctx, NULL, cipherText->len, plainText, &outLen) == CRYPT_NULL_INPUT);
    ASSERT_TRUE(CRYPT_EAL_PkeyDecrypt(ctx, cipherText->x, cipherText->len, NULL, &outLen) == CRYPT_NULL_INPUT);
    ASSERT_TRUE(CRYPT_EAL_PkeyDecrypt(ctx, cipherText->x, cipherText->len, plainText, NULL) == CRYPT_NULL_INPUT);
    ASSERT_TRUE(CRYPT_EAL_PkeyDecrypt(ctx, NULL, 0, plainText, &outLen) == CRYPT_NULL_INPUT);

    ASSERT_TRUE(CRYPT_EAL_PkeyDecrypt(ctx, cipherText->x, cipherText->len * 2, plainText, &outLen) ==
                CRYPT_SM2_BUFF_LEN_NOT_ENOUGH);

    outLen = cipherText->len;
    ASSERT_EQ(CRYPT_EAL_PkeyDecrypt(ctx, cipherText->x, cipherText->len, plainText, &outLen), CRYPT_SUCCESS);

exit:
    CRYPT_EAL_PkeyFreeCtx(ctx);
    CRYPT_EAL_RandDeinit();
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_SM2_CTRL_API_TC001
 * @title  SM2 CRYPT_EAL_PkeyCtrl test.
 * @precon nan
 * @brief
 *    1. Create the context of the SM2 algorithm, expected result 1
 *    2. Call the CRYPT_EAL_PkeyCtrl to set sm2 hash method, expected result 2
 *    2. Call the CRYPT_EAL_PkeyCtrl, opt is CRYPT_CTRL_UP_REFERENCES, len is 0, expected result 3
 * @expect
 *    1. Success, and context is not NULL.
 *    2. CRYPT_EAL_ALG_NOT_SUPPORT
 *    3. CRYPT_NULL_INPUT
 */
/* BEGIN_CASE */
void SDV_CRYPTO_SM2_CTRL_API_TC001(void)
{
    uint32_t ref = 1;

    TestMemInit();
    CRYPT_EAL_PkeyCtx *ctx = CRYPT_EAL_PkeyNewCtx(CRYPT_PKEY_SM2);
    ASSERT_TRUE(ctx != NULL);

    EAL_MdMethod hashMethod = {0};
    ASSERT_TRUE(CRYPT_EAL_PkeyCtrl(ctx, CRYPT_CTRL_SET_SM2_HASH_METHOD, &hashMethod, sizeof(EAL_MdMethod)) ==
                CRYPT_EAL_ALG_NOT_SUPPORT);
    ASSERT_EQ(CRYPT_EAL_PkeyCtrl(ctx, CRYPT_CTRL_UP_REFERENCES, &ref, 0), CRYPT_NULL_INPUT);

exit:
    CRYPT_EAL_PkeyFreeCtx(ctx);
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_SM2_ENC_FUNC_TC001
 * @title  SM2: public key encryption.
 * @precon Vectors: public key, plaintext, generate random number k, ciphertext.
 * @brief
 *    1. Create the context of the SM2 algorithm, expected result 1
 *    2. Set public key, expected result 2
 *    3. Take over random numbers, mock BN_RandRange to generate k
 *    4. Call the CRYPT_EAL_PkeyEncrypt to encrypt plaintext, expected result 3
 *    5. Compare the encryption result with the ciphertext vector, expected result 4
 * @expect
 *    1. Success, and context is not NULL.
 *    2-3. CRYPT_SUCCESS
 *    4. Both are the same.
 */
/* BEGIN_CASE */
void SDV_CRYPTO_SM2_ENC_FUNC_TC001(Hex *pubKey, Hex *plain, Hex *k, Hex *cipher)
{
    FuncStubInfo tmpRpInfo;
    uint8_t cipherText[MAX_PLAIN_TEXT_LEN + CIPHER_TEXT_EXTRA_LEN] = {0};
    uint8_t decodeText[MAX_PLAIN_TEXT_LEN + CIPHER_TEXT_EXTRA_LEN] = {0};
    uint32_t decodeLen = sizeof(decodeText);
    uint32_t outLen = sizeof(cipherText);
    CRYPT_EAL_PkeyPub pub = {0};

    SetSm2PubKey(&pub, pubKey->x, pubKey->len);
    TestMemInit();

    CRYPT_EAL_PkeyCtx *ctx = CRYPT_EAL_PkeyNewCtx(CRYPT_PKEY_SM2);
    ASSERT_TRUE(ctx != NULL);

    ASSERT_EQ(CRYPT_EAL_PkeySetPub(ctx, &pub), CRYPT_SUCCESS);

    STUB_Init();
    ASSERT_TRUE(SetFakeRandOutput(k->x, k->len) == CRYPT_SUCCESS);
    STUB_Replace(&tmpRpInfo, BN_RandRange, STUB_RandRangeK);

    ASSERT_TRUE(CRYPT_EAL_PkeyEncrypt(ctx, plain->x, plain->len, cipherText, &outLen) == CRYPT_SUCCESS);

    ASSERT_TRUE(ASN1_Sm2EncryptDataDecode(cipherText, outLen, decodeText + 1, &decodeLen) == CRYPT_SUCCESS);
    decodeText[0] = 0x04;
    ASSERT_TRUE(decodeLen + 1 == cipher->len);
    ASSERT_TRUE(memcmp(decodeText, cipher->x, cipher->len) == 0);

exit:
    CRYPT_EAL_PkeyFreeCtx(ctx);
    STUB_Reset(&tmpRpInfo);
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_SM2_DEC_FUNC_TC001
 * @title  SM2: private key decryption.
 * @precon Vectors: private key, plaintext, ciphertext.
 * @brief
 *    1. Create the context of the SM2 algorithm, expected result 1
 *    2. Set private key, expected result 2
 *    3. Call the CRYPT_EAL_PkeyDecrypt to decrypt ciphertext, expected result 3
 *    4. Compare the decryption result with the ciphertext vector, expected result 4
 * @expect
 *    1. Success, and context is not NULL.
 *    2-3. CRYPT_SUCCESS
 *    4. Both are the same.
 */
/* BEGIN_CASE */
void SDV_CRYPTO_SM2_DEC_FUNC_TC001(Hex *prvKey, Hex *plain, Hex *cipher)
{
    CRYPT_RandRegist(RandFunc);
    uint8_t plainText[MAX_PLAIN_TEXT_LEN] = {0};
    uint32_t outLen = sizeof(plainText);
    uint8_t encodeText[MAX_PLAIN_TEXT_LEN + 20] = {0};
    uint32_t encodeLen = MAX_PLAIN_TEXT_LEN + 20;
    CRYPT_EAL_PkeyPrv prv = {0};
    SetSm2PrvKey(&prv, prvKey->x, prvKey->len);

    TestMemInit();

    CRYPT_EAL_PkeyCtx *ctx = CRYPT_EAL_PkeyNewCtx(CRYPT_PKEY_SM2);
    ASSERT_TRUE(ctx != NULL);

    ASSERT_TRUE(CRYPT_EAL_PkeySetPrv(ctx, &prv) == CRYPT_SUCCESS);

    ASSERT_TRUE(ASN1_Sm2EncryptDataEncode(cipher->x + 1, cipher->len - 1, encodeText, &encodeLen) == CRYPT_SUCCESS);

    ASSERT_TRUE(CRYPT_EAL_PkeyDecrypt(ctx, encodeText, encodeLen, plainText, &outLen) == CRYPT_SUCCESS);

    ASSERT_TRUE(outLen == plain->len);
    ASSERT_TRUE(memcmp(plainText, plain->x, plain->len) == 0);

exit:
    CRYPT_RandRegist(NULL);
    CRYPT_EAL_PkeyFreeCtx(ctx);
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_SM2_DEC_FUNC_TC002
 * @title  SM2: Private key decryption failure scenario.
 * @precon Vectors: private key, ciphertext.
 * @brief
 *    1. Create the context of the SM2 algorithm, expected result 1
 *    2. Set private key, expected result 2
 *    3. Call the CRYPT_EAL_PkeyDecrypt to decrypt ciphertext, expected result 3
 * @expect
 *    1. Success, and context is not NULL.
 *    2. CRYPT_SUCCESS
 *    3. Failure.
 */
/* BEGIN_CASE */
void SDV_CRYPTO_SM2_DEC_FUNC_TC002(Hex *prvKey, Hex *cipher)
{
    TestMemInit();
    uint8_t plainText[MAX_PLAIN_TEXT_LEN];
    uint32_t outLen = sizeof(plainText);
    CRYPT_EAL_PkeyPrv prv = {0};
    SetSm2PrvKey(&prv, prvKey->x, prvKey->len);

    CRYPT_EAL_PkeyCtx *ctx = CRYPT_EAL_PkeyNewCtx(CRYPT_PKEY_SM2);
    ASSERT_TRUE(ctx != NULL);

    ASSERT_TRUE(CRYPT_EAL_PkeySetPrv(ctx, &prv) == CRYPT_SUCCESS);
    /* Different error codes are returned for different phases. */
    ASSERT_TRUE(CRYPT_EAL_PkeyDecrypt(ctx, cipher->x, cipher->len, plainText, &outLen) != CRYPT_SUCCESS);

exit:
    CRYPT_EAL_PkeyFreeCtx(ctx);
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_SM2_GEN_CRYPT_FUNC_TC001
 * @title  SM2: Generate key pair, encryption, decryption.
 * @precon Vector: plaintext.
 * @brief
 *    1. Create the context of the SM2 algorithm, expected result 1
 *    2. Initialize the DRBG.
 *    3. Call the CRYPT_EAL_PkeyGen to generate a key pair, expected result 2
 *    4. Call the CRYPT_EAL_PkeyEncrypt to encrypt plaintext, expected result 3
 *    5. Call the CRYPT_EAL_PkeyDecrypt to decrypt ciphertext, expected result 4
 *    6. Compare the decryption result with the plaintext vector, expected result 5
 * @expect
 *    1. Success, and context is not NULL.
 *    2-4. CRYPT_SUCCESS
 *    5. Both are the same.
 */
/* BEGIN_CASE */
void SDV_CRYPTO_SM2_GEN_CRYPT_FUNC_TC001(Hex *msg)
{
    uint8_t cipherText[MAX_PLAIN_TEXT_LEN + CIPHER_TEXT_EXTRA_LEN];
    uint8_t plainText[MAX_PLAIN_TEXT_LEN];
    uint32_t ctLen = sizeof(cipherText);
    uint32_t ptLen = sizeof(plainText);

    TestMemInit();
    CRYPT_EAL_PkeyCtx *ctx = CRYPT_EAL_PkeyNewCtx(CRYPT_PKEY_SM2);
    ASSERT_TRUE(ctx != NULL);

    CRYPT_RandRegist(RandFunc);
    ASSERT_TRUE(CRYPT_EAL_PkeyGen(ctx) == CRYPT_SUCCESS);

    ASSERT_TRUE(CRYPT_EAL_PkeyEncrypt(ctx, msg->x, msg->len, cipherText, &ctLen) == CRYPT_SUCCESS);
    ASSERT_TRUE(CRYPT_EAL_PkeyDecrypt(ctx, cipherText, ctLen, plainText, &ptLen) == CRYPT_SUCCESS);

    ASSERT_TRUE(ptLen == msg->len);
    ASSERT_TRUE(memcmp(plainText, msg->x, msg->len) == 0);

exit:
    CRYPT_RandRegist(NULL);
    CRYPT_EAL_PkeyFreeCtx(ctx);
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_SM2_GEN_CRYPT_FUNC_TC002
 * @title  SM2: The input and output parameters address are the same.
 * @precon Vector: plaintext.
 * @brief
 *    1. Create the context of the SM2 algorithm, expected result 1
 *    2. Initialize the DRBG.
 *    3. Call the CRYPT_EAL_PkeyGen to generate a key pair, expected result 2
 *    4. Call the CRYPT_EAL_PkeyEncrypt, and the input and output parameters address are the same, expected result 3
 *    5. Call the CRYPT_EAL_PkeyDecrypt, and the input and output parameters address are the same, expected result 4
 *    6. Compare the decryption result with the plaintext vector, expected result 5
 * @expect
 *    1. Success, and context is not NULL.
 *    2-4. CRYPT_SUCCESS
 *    5. Both are the same.
 */
/* BEGIN_CASE */
void SDV_CRYPTO_SM2_GEN_CRYPT_FUNC_TC002(Hex *msg)
{
    uint8_t buf[MAX_PLAIN_TEXT_LEN + CIPHER_TEXT_EXTRA_LEN];
    uint32_t ctLen = sizeof(buf);
    uint32_t ptLen = sizeof(buf);
    ASSERT_TRUE(memcpy_s(buf, ptLen, msg->x, msg->len) == CRYPT_SUCCESS);

    TestMemInit();
    CRYPT_EAL_PkeyCtx *ctx = CRYPT_EAL_PkeyNewCtx(CRYPT_PKEY_SM2);
    ASSERT_TRUE(ctx != NULL);

    CRYPT_RandRegist(RandFunc);
    ASSERT_TRUE(CRYPT_EAL_PkeyGen(ctx) == CRYPT_SUCCESS);

    ASSERT_TRUE(CRYPT_EAL_PkeyEncrypt(ctx, buf, msg->len, buf, &ctLen) == CRYPT_SUCCESS);
    ASSERT_TRUE(CRYPT_EAL_PkeyDecrypt(ctx, buf, ctLen, buf, &ptLen) == CRYPT_SUCCESS);

    ASSERT_TRUE(ptLen == msg->len);
    ASSERT_TRUE(memcmp(buf, msg->x, msg->len) == 0);

exit:
    CRYPT_RandRegist(NULL);
    CRYPT_EAL_PkeyFreeCtx(ctx);
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_SM2_CMP_FUNC_TC001
 * @title  SM2: The input and output parameters address are the same.
 * @precon Vector: private key and public key.
 * @brief
 *    1. Create the contexts(ctx1, ctx2) of the SM2 algorithm, expected result 1
 *    2. Call the CRYPT_EAL_PkeyCmp to compare ctx1 and ctx2, expected result 2
 *    3. Set public key for ctx1, expected result 3
 *    4. Call the CRYPT_EAL_PkeyCmp to compare ctx1 and ctx2, expected result 4
 *    5. Set public key for ctx2, expected result 5
 *    6. Call the CRYPT_EAL_PkeyCmp to compare ctx1 and ctx2, expected result 6
 * @expect
 *    1. Success, and contexts are not NULL.
 *    2. CRYPT_ECC_KEY_PUBKEY_NOT_EQUAL
 *    3. CRYPT_SUCCESS
 *    4. CRYPT_ECC_KEY_PUBKEY_NOT_EQUAL
 *    5-6. CRYPT_SUCCESS
 */
/* BEGIN_CASE */
void SDV_CRYPTO_SM2_CMP_FUNC_TC001(Hex *pubKey)
{
    CRYPT_EAL_PkeyPub pub = {0};

    SetSm2PubKey(&pub, pubKey->x, pubKey->len);

    TestMemInit();

    CRYPT_EAL_PkeyCtx *ctx1 = CRYPT_EAL_PkeyNewCtx(CRYPT_PKEY_SM2);
    CRYPT_EAL_PkeyCtx *ctx2 = CRYPT_EAL_PkeyNewCtx(CRYPT_PKEY_SM2);
    ASSERT_TRUE(ctx1 != NULL && ctx2 != NULL);

    ASSERT_EQ(CRYPT_EAL_PkeyCmp(ctx1, ctx2), CRYPT_ECC_KEY_PUBKEY_NOT_EQUAL);

    ASSERT_EQ(CRYPT_EAL_PkeySetPub(ctx1, &pub), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_PkeyCmp(ctx1, ctx2), CRYPT_ECC_KEY_PUBKEY_NOT_EQUAL);

    ASSERT_EQ(CRYPT_EAL_PkeySetPub(ctx2, &pub), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_PkeyCmp(ctx1, ctx2), CRYPT_SUCCESS);

exit:
    CRYPT_EAL_PkeyFreeCtx(ctx1);
    CRYPT_EAL_PkeyFreeCtx(ctx2);
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_SM2_ENC_DECODE_FUNC_TC001
 * @title  SM2: for testing sm2 ciphertext decode.
 * @precon Vector: SM2 ciphertext.
 * @brief
 *    1. Call the ASN1_Sm2EncryptDataDecode to decode the SM2 ciphertext
 * @expect
 *    1. CRYPT_SUCCESS
 */
/* BEGIN_CASE */
void SDV_CRYPTO_SM2_ENC_DECODE_FUNC_TC001(Hex *cipher)
{
    uint8_t decode[MAX_PLAIN_TEXT_LEN] = {0};
    uint32_t decodelen = MAX_PLAIN_TEXT_LEN;
    ASSERT_TRUE(ASN1_Sm2EncryptDataDecode(cipher->x, cipher->len, decode, &decodelen) == CRYPT_SUCCESS);
exit:
    return;
}
/* END_CASE */
