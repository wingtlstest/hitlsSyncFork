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

#include <limits.h>
#include "crypt_eal_md.h"
#include "bsl_sal.h"
#include "eal_md_local.h"
#include "crypt_algid.h"
#include "crypt_errno.h"
#include "crypt_md5.h"
/* END_HEADER */

/**
 * @test   SDV_CRYPTO_MD5_API_TC001
 * @title  update and final test.
 * @precon nan
 * @brief
 *    1.Call CRYPT_EAL_MdNewCtx create the CTX, expected result 1.
 *    2.Call CRYPT_EAL_MdUpdate and CRYPT_EAL_MdFinal before initialization, expected result 2.
 *    3.Call CRYPT_EAL_MdUpdate and CRYPT_EAL_MdFinal use null pointer, expected result 3.
 *    4.Call CRYPT_EAL_MdUpdate and CRYPT_EAL_MdFinal normally, expected result 4.
 * @expect
 *    1.Create successful.
 *    2.Return CRYPT_EAL_ERR_STATE.
 *    3.Return CRYPT_NULL_INPUT.
 *    4.Successful.
 */
/* BEGIN_CASE */
void SDV_CRYPTO_MD5_API_TC001(void)
{
    TestMemInit();
    uint8_t data[100];
    uint32_t dataLen = sizeof(data);
    uint8_t output[CRYPT_MD5_DIGESTSIZE + 1];
    uint32_t outputLen = CRYPT_MD5_DIGESTSIZE;
    CRYPT_EAL_MdCTX *md5Ctx = CRYPT_EAL_MdNewCtx(CRYPT_MD_MD5);
    ASSERT_TRUE(md5Ctx != NULL);

    ASSERT_EQ(CRYPT_EAL_MdGetDigestSize(CRYPT_MD_MD5), CRYPT_MD5_DIGESTSIZE);

    ASSERT_EQ(CRYPT_EAL_MdUpdate(md5Ctx, data, dataLen), CRYPT_EAL_ERR_STATE);
    ASSERT_EQ(CRYPT_EAL_MdFinal(md5Ctx, output, &outputLen), CRYPT_EAL_ERR_STATE);

    ASSERT_EQ(CRYPT_EAL_MdInit(md5Ctx), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_MdUpdate(md5Ctx, NULL, dataLen), CRYPT_NULL_INPUT);
    ASSERT_EQ(CRYPT_EAL_MdUpdate(md5Ctx, data, dataLen), CRYPT_SUCCESS);

    ASSERT_EQ(CRYPT_EAL_MdFinal(NULL, output, &outputLen), CRYPT_NULL_INPUT);
    ASSERT_EQ(CRYPT_EAL_MdFinal(md5Ctx, NULL, &outputLen), CRYPT_NULL_INPUT);
    ASSERT_EQ(CRYPT_EAL_MdFinal(md5Ctx, output, NULL), CRYPT_NULL_INPUT);

    outputLen = CRYPT_MD5_DIGESTSIZE - 1;
    ASSERT_EQ(CRYPT_EAL_MdFinal(md5Ctx, output, &outputLen), CRYPT_MD5_OUT_BUFF_LEN_NOT_ENOUGH);
    outputLen = CRYPT_MD5_DIGESTSIZE;
    ASSERT_EQ(CRYPT_EAL_MdFinal(md5Ctx, output, &outputLen), CRYPT_SUCCESS);

    ASSERT_EQ(CRYPT_EAL_MdInit(md5Ctx), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_MdUpdate(md5Ctx, data, dataLen), CRYPT_SUCCESS);
    outputLen = CRYPT_MD5_DIGESTSIZE + 1;
    ASSERT_EQ(CRYPT_EAL_MdFinal(md5Ctx, output, &outputLen), CRYPT_SUCCESS);
exit:
    CRYPT_EAL_MdFreeCtx(md5Ctx);
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_MD5_FUNC_TC001
 * @title  CRYPT_EAL_MdFinal test without update.
 * @precon nan
 * @brief
 *    1.Call CRYPT_EAL_MdNewCtx to create ctx, expected result 1.
 *    2.Call CRYPT_EAL_MdFinal get results. expected result 2.
 *    3.Compare with expected results. expected result 3.
 * @expect
 *    1.The ctx is created successfully.
 *    2.Successful.
 *    2.Consistent with expected results.
 */
/* BEGIN_CASE */
void SDV_CRYPTO_MD5_FUNC_TC001(Hex *hash)
{
    TestMemInit();
    uint8_t output[CRYPT_MD5_DIGESTSIZE];
    uint32_t outLen = CRYPT_MD5_DIGESTSIZE;

    CRYPT_EAL_MdCTX *md5Ctx = CRYPT_EAL_MdNewCtx(CRYPT_MD_MD5);
    ASSERT_TRUE(md5Ctx != NULL);
    ASSERT_EQ(CRYPT_EAL_MdInit(md5Ctx), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_MdFinal(md5Ctx, output, &outLen), CRYPT_SUCCESS);
    ASSERT_EQ(outLen, CRYPT_MD5_DIGESTSIZE);
    ASSERT_COMPARE("md5", output, outLen, hash->x, hash->len);

exit:
    CRYPT_EAL_MdFreeCtx(md5Ctx);
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_MD5_FUNC_TC002
 * @title  Perform the vector test to check whether the calculation result is consistent with the standard output.
 * @precon nan
 * @brief
 *    1.Calculate the hash of each group of data, expected result 1.
*     2.Compare the result to the expected value, expected result 2.
 * @expect
 *    1.Hash calculation succeeded.
 *    2.The results are as expected.
 */
/* BEGIN_CASE */
void SDV_CRYPTO_MD5_FUNC_TC002(Hex *msg, Hex *hash)
{
    TestMemInit();
    uint8_t output[CRYPT_MD5_DIGESTSIZE];
    uint32_t outLen = CRYPT_MD5_DIGESTSIZE;

    CRYPT_EAL_MdCTX *md5Ctx = CRYPT_EAL_MdNewCtx(CRYPT_MD_MD5);
    ASSERT_TRUE(md5Ctx != NULL);

    ASSERT_EQ(CRYPT_EAL_MdInit(md5Ctx), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_MdUpdate(md5Ctx, msg->x, msg->len), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_MdFinal(md5Ctx, output, &outLen), CRYPT_SUCCESS);
    ASSERT_EQ(outLen, CRYPT_MD5_DIGESTSIZE);
    ASSERT_COMPARE("md5", output, outLen, hash->x, hash->len);
exit:
    CRYPT_EAL_MdFreeCtx(md5Ctx);
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_MD5_FUNC_TC003
 * @title  Split the data and update test.
 * @precon nan
 * @brief
 *    1.Create two ctx and initialize them, expected result 1.
 *    2.Use ctx1 to update data 100 times, expected result 2.
 *    3.Use ctx2 to update all data at once, expected result 3.
 *    4.Compare two outputs, expected result 4.
 * @expect
 *    1.Successful.
 *    2.Successful.
 *    3.Successful.
 *    4.The results are the same.
 */
/* BEGIN_CASE */
void SDV_CRYPTO_MD5_FUNC_TC003(void)
{
    TestMemInit();
    CRYPT_EAL_MdCTX *ctx1 = NULL;
    CRYPT_EAL_MdCTX *ctx2 = NULL;

    ctx1 = CRYPT_EAL_MdNewCtx(CRYPT_MD_MD5);
    ASSERT_TRUE(ctx1 != NULL);

    ctx2 = CRYPT_EAL_MdNewCtx(CRYPT_MD_MD5);
    ASSERT_TRUE(ctx2 != NULL);

    uint8_t input[5050];
    uint32_t inLenTotal = 0;
    uint32_t inLenBase;
    uint8_t out1[CRYPT_MD5_DIGESTSIZE];
    uint8_t out2[CRYPT_MD5_DIGESTSIZE];
    uint32_t outLen = CRYPT_MD5_DIGESTSIZE;

    ASSERT_EQ(CRYPT_EAL_MdInit(ctx1), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_MdInit(ctx2), CRYPT_SUCCESS);

    for (inLenBase = 1; inLenBase <= 100; inLenBase++) {
        ASSERT_EQ(CRYPT_EAL_MdUpdate(ctx1, input + inLenTotal, inLenBase), CRYPT_SUCCESS);
        inLenTotal += inLenBase;
    }
    ASSERT_EQ(CRYPT_EAL_MdFinal(ctx1, out1, &outLen), CRYPT_SUCCESS);
    ASSERT_EQ(outLen, CRYPT_MD5_DIGESTSIZE);

    ASSERT_EQ(CRYPT_EAL_MdUpdate(ctx2, input, inLenTotal), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_MdFinal(ctx2, out2, &outLen), CRYPT_SUCCESS);
    ASSERT_EQ(outLen, CRYPT_MD5_DIGESTSIZE);

    ASSERT_COMPARE("md5", out1, outLen, out2, outLen);
exit:
    CRYPT_EAL_MdFreeCtx(ctx1);
    CRYPT_EAL_MdFreeCtx(ctx2);
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_MD5_FUNC_TC004
 * @title  Hash calculation for multiple updates,comparison with standard results.
 * @precon nan
 * @brief
 *    1.Call CRYPT_EAL_MdNewCtx to create a ctx and initialize, expected result 1.
 *    2.Call CRYPT_EAL_MdUpdate to calculate the hash of a data segmentxpected result 2.
 *    3.Call CRYPT_EAL_MdUpdate to calculate the next data segmentxpected result 3.
 *    4.Call CRYPT_EAL_MdUpdate to calculate the next data segmentxpected result 4.
 *    5.Call CRYPT_EAL_MdFinal get the result, expected result 5.
 * @expect
 *    1.Successful
 *    2.Successful
 *    3.Successful
 *    4.Successful
 *    5.The results are as expected.
 */
/* BEGIN_CASE */
void SDV_CRYPTO_MD5_FUNC_TC004(Hex *data1, Hex *data2, Hex *data3, Hex *hash)
{
    TestMemInit();
    uint8_t output[CRYPT_MD5_DIGESTSIZE];
    uint32_t outLen = CRYPT_MD5_DIGESTSIZE;
    CRYPT_EAL_MdCTX *md5Ctx = CRYPT_EAL_MdNewCtx(CRYPT_MD_MD5);
    ASSERT_TRUE(md5Ctx != NULL);

    ASSERT_EQ(CRYPT_EAL_MdInit(md5Ctx), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_MdUpdate(md5Ctx, data1->x, data1->len), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_MdUpdate(md5Ctx, data2->x, data2->len), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_MdUpdate(md5Ctx, data3->x, data3->len), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_MdFinal(md5Ctx, output, &outLen), CRYPT_SUCCESS);
    ASSERT_EQ(outLen, CRYPT_MD5_DIGESTSIZE);
    ASSERT_COMPARE("md5", output, outLen, hash->x, hash->len);
exit:
    CRYPT_EAL_MdFreeCtx(md5Ctx);
}
/* END_CASE */

/**
 * @test   SDV_CRYPTO_MD5_COPY_CTX_FUNC_TC001
 * @title  MD5 copy ctx function test.
 * @precon nan
 * @brief
 *    1. Create the context ctx of md algorithm, expected result 1
 *    2. Call to CRYPT_EAL_MdCopyCtx method to copy ctx, expected result 2
 *    3. Calculate the hash of msg, and compare the calculated result with hash vector, expected result 3
 * @expect
 *    1. Success, the context is not null.
 *    2. CRYPT_SUCCESS
 *    3. Success, the hashs are the same.
 */
/* BEGIN_CASE */
void SDV_CRYPTO_MD5_COPY_CTX_FUNC_TC001(int id, Hex *msg, Hex *hash)
{
    TestMemInit();
    CRYPT_EAL_MdCTX *cpyCtx = NULL;
    CRYPT_EAL_MdCTX *ctx = CRYPT_EAL_MdNewCtx(id);
    ASSERT_TRUE(ctx != NULL);
    uint8_t output[CRYPT_MD5_DIGESTSIZE];
    uint32_t outLen = CRYPT_MD5_DIGESTSIZE;

    cpyCtx = BSL_SAL_Calloc(1u, sizeof(CRYPT_EAL_MdCTX));
    ASSERT_TRUE(cpyCtx != NULL);
    ASSERT_EQ(CRYPT_EAL_MdCopyCtx(cpyCtx, ctx), CRYPT_SUCCESS);

    ASSERT_EQ(CRYPT_EAL_MdInit(cpyCtx), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_MdUpdate(cpyCtx, msg->x, msg->len), CRYPT_SUCCESS);
    ASSERT_EQ(CRYPT_EAL_MdFinal(cpyCtx, output, &outLen), CRYPT_SUCCESS);

    ASSERT_EQ(id, cpyCtx->id);
    ASSERT_EQ(memcmp(output, hash->x, hash->len), 0);

exit:
    CRYPT_EAL_MdFreeCtx(ctx);
    CRYPT_EAL_MdFreeCtx(cpyCtx);
}
/* END_CASE */
