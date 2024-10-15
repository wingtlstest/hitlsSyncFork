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

#ifndef CRYPT_RSA_H
#define CRYPT_RSA_H

#include "hitls_build.h"
#ifdef HITLS_CRYPTO_RSA

#include <stdlib.h>
#include <stdint.h>
#include "crypt_local_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cpluscplus */

#define RSA_MIN_MODULUS_BITS 1024
#define RSA_MAX_MODULUS_BITS 16384
#define RSA_SMALL_MODULUS_BYTES (3072 / 8)
#define RSA_MAX_PUBEXP_BYTES (64 / 8)
#define RSA_MIN_MODULUS_LEN (RSA_MIN_MODULUS_BITS / 8)
#define RSA_MAX_MODULUS_LEN (RSA_MAX_MODULUS_BITS / 8)

/* RSA */
typedef struct RSA_Ctx CRYPT_RSA_Ctx;
typedef struct RSA_Para CRYPT_RSA_Para;


/* RSA method */

/**
 * @ingroup rsa
 * @brief Allocate rsa context memory space.
 *
 * @retval (CRYPT_RSA_Ctx *) Pointer to the memory space of the allocated context
 * @retval NULL              Invalid null pointer.
 */
CRYPT_RSA_Ctx *CRYPT_RSA_NewCtx(void); // create key structure

/**
 * @ingroup rsa
 * @brief Copy the RSA context. After the duplication is complete, call the CRYPT_RSA_FreeCtx to release the memory.
 *
 * @param ctx [IN] RSA context
 *
 * @return CRYPT_RSA_Ctx    Rsa context pointer
 *         If the operation fails, a null value is returned.
 */
CRYPT_RSA_Ctx *CRYPT_RSA_DupCtx(CRYPT_RSA_Ctx *keyCtx);

/**
 * @ingroup rsa
 * @brief Create rsa key parameter structure
 *
 * @param para [IN] RSA External parameter
 *
 * @retval (CRYPT_RSA_Para *) Pointer to the allocated memory space of the structure
 * @retval NULL               Invalid null pointer.
 */
CRYPT_RSA_Para *CRYPT_RSA_NewPara(const CRYPT_RsaPara *para);

/**
 * @ingroup rsa
 * @brief Release rsa key parameter structure
 *
 * @param para [IN] Storage pointer in the parameter structure to be released. The parameter is set NULL by the invoker.
 */
void CRYPT_RSA_FreePara(CRYPT_RSA_Para *para);

/**
 * @ingroup rsa
 * @brief release rsa key context structure
 *
 * @param ctx [IN] Pointer to the context structure to be released. The ctx is set NULL by the invoker.
 */
void CRYPT_RSA_FreeCtx(CRYPT_RSA_Ctx *ctx);

/**
 * @ingroup rsa
 * @brief Set the data of the key parameter structure to the key structure.
 *
 * @param ctx [OUT] RSA context structure for which related parameters need to be set
 * @param para [IN] Key parameter structure
 *
 * @retval CRYPT_NULL_INPUT         Invalid null pointer input.
 * @retval CRYPT_RSA_ERR_KEY_BITS   The expected key length does not meet the requirements.
 * @retval CRYPT_RSA_ERR_E_VALUE    The expected value of e does not meet the requirements.
 * @retval CRYPT_MEM_ALLOC_FAIL     internal memory allocation error
 * @retval CRYPT_SUCCESS            set successfully.
 */
int32_t CRYPT_RSA_SetPara(CRYPT_RSA_Ctx *ctx, const CRYPT_RSA_Para *para);

/**
 * @ingroup rsa
 * @brief Obtain the valid length of the key.
 *
 * @param ctx [IN] Structure from which the key length is expected to be obtained
 *
 * @retval 0: The input is incorrect or the corresponding key structure does not have a valid key length.
 * @retval uint32_t: Valid key length
 */
uint32_t CRYPT_RSA_GetBits(const CRYPT_RSA_Ctx *ctx);

/**
 * @ingroup rsa
 * @brief Obtain the maximum length of RSA signature data.
 *
 * @param ctx [IN] Maximum length of the RSA signature data that is expected to be obtained
 *
 * @retval 0: The input is incorrect or the corresponding key structure does not contain valid key information.
 * @retval uint32_t: Maximum length of the signature data
 */
uint32_t CRYPT_RSA_GetSignLen(const CRYPT_RSA_Ctx *ctx);
/**
 * @ingroup rsa
 * @brief Generate the RSA key pair.
 *
 * @param ctx [IN/OUT] rsa context structure
 *
 * @retval CRYPT_NULL_INPUT         Error null pointer input
 * @retval CRYPT_RSA_ERR_KEY_BITS   The value of e in the context structure does not meet the requirements.
 * @retval CRYPT_MEM_ALLOC_FAIL     Memory allocation failure
 * @retval BN error                 An error occurs in the internal BigNum operation.
 * @retval CRYPT_SUCCESS            The key pair is successfully generated.
 */
int32_t CRYPT_RSA_Gen(CRYPT_RSA_Ctx *ctx);

/**
 * @ingroup rsa
 * @brief RSA public key encryption
 *
 * @param ctx [IN] RSA context structure
 * @param input [IN] Information to be encrypted
 * @param inputLen [IN] Length of the information to be encrypted
 * @param out [OUT] Pointer to the encrypted information output.
 * @param outLen [IN/OUT] Pointer to the length of the encrypted information.
 *                        Before being transferred, the value must be set to the maximum length of the array.
 *
 * @retval CRYPT_NULL_INPUT             Invalid null pointer input
 * @retval CRYPT_RSA_NO_KEY_INFO        does not contain the key information.
 * @retval CRYPT_RSA_ERR_INPUT_VALUE    The entered value does not meet the calculation conditions.
 * @retval CRYPT_MEM_ALLOC_FAIL         Memory allocation failure
 * @retval CRYPT_SECUREC_FAIL           A security function error occurs.
 * @retval BN error                     An error occurs in the internal BigNum operation.
 * @retval CRYPT_SUCCESS                encryption succeeded.
 */
int32_t  CRYPT_RSA_PubEnc(const CRYPT_RSA_Ctx *ctx, const uint8_t *input, uint32_t inputLen,
    uint8_t *out, uint32_t *outLen);

/**
 * @ingroup rsa
 * @brief RSA private key decryption
 *
 * @param ctx [IN] RSA context structure
 * @param input [IN] Information to be decrypted
 * @param inputLen [IN] Length of the information to be decrypted
 * @param out [OUT] Pointer to the decrypted information output.
 * @param outLen [IN/OUT] Pointer to the length of the decrypted information.
 *                        Before being transferred, the value must be set to the maximum length of the array.
 *
 * @retval CRYPT_NULL_INPUT             Invalid null pointer input
 * @retval CRYPT_RSA_ERR_DEC_BITS       Incorrect length of the encrypted private key.
 * @retval CRYPT_RSA_NO_KEY_INFO        does not contain the key information.
 * @retval CRYPT_RSA_ERR_INPUT_VALUE    The entered value does not meet the calculation conditions.
 * @retval CRYPT_MEM_ALLOC_FAIL         Memory allocation failure
 * @retval CRYPT_SECUREC_FAIL           A security function error occurs.
 * @retval BN error.                    An error occurs in the internal BigNum operation.
 * @retval CRYPT_SUCCESS                Decrypted Successfully
 */
int32_t CRYPT_RSA_PrvDec(const CRYPT_RSA_Ctx *ctx, const uint8_t *input, uint32_t inputLen,
    uint8_t *out, uint32_t *outLen);

/**
 * @ingroup rsa
 * @brief RSA Set the private key information.
 *
 * @param ctx [OUT] rsa context structure
 * @param prv [IN] Private key data
 *
 * @retval CRYPT_NULL_INPUT             Error null pointer input
 * @retval CRYPT_RSA_ERR_KEY_BITS       The key length does not meet the requirements.
 * @retval CRYPT_RSA_NO_KEY_INFO        does not contain the key information.
 * @retval CRYPT_RSA_ERR_INPUT_VALUE    The entered value does not meet the calculation conditions.
 * @retval CRYPT_MEM_ALLOC_FAIL         Memory allocation failure
 * @retval BN error                     An error occurs in the internal BigNum operation.
 * @retval CRYPT_SUCCESS                The private key is successfully set.
 */
int32_t CRYPT_RSA_SetPrvKey(CRYPT_RSA_Ctx *ctx, const CRYPT_RsaPrv *prv);

/**
 * @ingroup rsa
 * @brief RSA Set the public key information.
 *
 * @param ctx [OUT] RSA context structure
 * @param pub [IN] Public key data
 *
 * @retval CRYPT_NULL_INPUT          Error null pointer input
 * @retval CRYPT_RSA_ERR_KEY_BITS    The key length does not meet the requirements.
 * @retval CRYPT_RSA_ERR_INPUT_VALUE The entered value does not meet the calculation conditions.
 * @retval CRYPT_MEM_ALLOC_FAIL      Memory allocation failure
 * @retval BN error                  An error occurs in the internal BigNum operation.
 * @retval CRYPT_SUCCESS             The public key is successfully set.
 */
int32_t CRYPT_RSA_SetPubKey(CRYPT_RSA_Ctx *ctx, const CRYPT_RsaPub *pub);

/**
 * @ingroup rsa
 * @brief RSA Obtain the private key information.
 *
 * @param ctx [IN] RSA context structure
 * @param prv [OUT] Private key data
 *
 * @retval CRYPT_NULL_INPUT Invalid null pointer input
 * @retval BN error         An error occurs in the internal BigNum operation.
 * @retval CRYPT_SUCCESS    The private key is obtained successfully.
 */
int32_t CRYPT_RSA_GetPrvKey(const CRYPT_RSA_Ctx *ctx, CRYPT_RsaPrv *prv);

/**
 * @ingroup rsa
 * @brief RSA Obtain the public key information.
 *
 * @param ctx [IN] RSA context structure
 * @param pub [OUT] Public key data
 *
 * @retval CRYPT_NULL_INPUT Invalid null pointer input
 * @retval BN error         An error occurs in the internal BigNum operation.
 * @retval CRYPT_SUCCESS    The public key is obtained successfully.
 */
int32_t CRYPT_RSA_GetPubKey(const CRYPT_RSA_Ctx *ctx, CRYPT_RsaPub *pub);

/**
 * @ingroup rsa
 * @brief Set the PSS for the original data.
 *
 * @param hashMethod [IN] pss Required Hash Method
 * @param mgfMethod [IN] pss Internal hash method required by the mgf.
 * @param keyBits [IN] pss Key length
 * @param salt [IN] Input salt value
 * @param saltLen [IN] Length of the input salt.
 * @param data [IN] Original data
 * @param dataLen [IN] Length of the original data
 * @param pad [OUT] pss Output buffer
 * @param padLen [OUT] Maximum length of the array output by the PSS.
 *
 * @retval CRYPT_NULL_INPUT                 Error null pointer input
 * @retval CRYPT_RSA_ERR_PSS_SALT_DATA      The salt value does not meet the requirements.
 * @retval CRYPT_RSA_ERR_KEY_BITS           The key length does not meet the requirements.
 * @retval CRYPT_RSA_ERR_PSS_SALT_LEN       The salt length does not meet the requirements.
 * @retval CRYPT_RSA_BUFF_LEN_NOT_ENOUGH    The length of the reserved buffer is insufficient.
 * @retval CRYPT_MEM_ALLOC_FAIL             Memory allocation failure
 * @retval CRYPT_SUCCESS                    Succeeded in setting the PSS.
 */
int32_t CRYPT_RSA_SetPss(const EAL_MdMethod *hashMethod, const EAL_MdMethod *mgfMethod, uint32_t keyBits,
    const uint8_t *salt, uint32_t saltLen, const uint8_t *data, uint32_t dataLen, uint8_t *pad, uint32_t padLen);

/**
 * @ingroup rsa
 * @brief Compare the original data from the PSS.
 *
 * @param hashMethod [IN] pss Required the hash method
 * @param mgfMethod [IN] pss Internal hash method required by the mgf.
 * @param keyBits [IN] pss Key length
 * @param saltLen [IN] Salt value length
 * @param data [IN] Original data
 * @param dataLen [IN] Length of the original data
 * @param pad [IN] Data after PSS is set.
 * @param padLen [IN] Data length after PSS is set.
 *
 * @retval CRYPT_NULL_INPUT                 Invalid null pointer input
 * @retval CRYPT_RSA_ERR_PSS_SALT_DATA      The salt value does not meet the requirements.
 * @retval CRYPT_RSA_ERR_PSS_SALT_LEN       The salt length does not meet the requirements.
 * @retval CRYPT_RSA_BUFF_LEN_NOT_ENOUGH    The length required for padding does not match the input parameter.
 * @retval CRYPT_MEM_ALLOC_FAIL             Memory allocation failure
 * @retval CRYPT_SUCCESS                    pss comparison succeeded.
 */
int32_t CRYPT_RSA_VerifyPss(const EAL_MdMethod *hashMethod, const EAL_MdMethod *mgfMethod, uint32_t keyBits,
    uint32_t saltLen, const uint8_t *data, uint32_t dataLen, const uint8_t *pad, uint32_t padLen);

/**
 * @ingroup rsa
 * @brief Set pkcsv1.5 padding.
 *
 * @param hashId [IN] the hash method required by pkcsv1.5 setting.
 * @param data [IN] Original data
 * @param dataLen [IN] Length of the original data
 * @param pad [OUT] Pointer to the array for receiving the padding.
 * @param padLen [IN] Array length for receiving padding.
 *
 * @retval CRYPT_NULL_INPUT                 Invalid null pointer input
 * @retval CRYPT_RSA_NO_KEY_INFO            The key information is insufficient.
 * @retval CRYPT_SECUREC_FAIL               The security function fails.
 * @retval CRYPT_RSA_BUFF_LEN_NOT_ENOUGH    The length required by the padding does not match the input parameter.
 * @retval CRYPT_RSA_ERR_INPUT_VALUE        The hash algorithm ID is not supported.
 * @retval CRYPT_SUCCESS                    The pkcsv1.5 padding is successfully set.
 */
int32_t CRYPT_RSA_SetPkcsV15Type1(CRYPT_MD_AlgId hashId, const uint8_t *data, uint32_t dataLen,
    uint8_t *pad, uint32_t padLen);

/**
 * @ingroup rsa
 * @brief Verify pkcsv1.5 padding.
 *
 * @param hashId [IN] the hash method corresponding to pkcsv1.5 verification.
 * @param pad [IN] Data after padding
 * @param padLen [IN] Data length after padding
 * @param data [IN] Original data
 * @param dataLen [IN] Length of the original data
 *
 * @retval CRYPT_NULL_INPUT                 Invalid null pointer input
 * @retval CRYPT_RSA_ERR_INPUT_VALUE        Incorrect padding value.
 * @retval CRYPT_SECUREC_FAIL               Security Function Failure
 * @retval CRYPT_RSA_BUFF_LEN_NOT_ENOUGH    The length required for padding does not match the input parameter.
 * @retval CRYPT_RSA_ERR_MD_ALGID           The hash algorithm ID is not supported.
 * @retval CRYPT_SUCCESS                    Verify pkcsv1.5 is padded successfully.
 */
int32_t CRYPT_RSA_VerifyPkcsV15Type1(CRYPT_MD_AlgId hashId, const uint8_t *pad, uint32_t padLen,
    const uint8_t *data, uint32_t dataLen);

int32_t CRYPT_RSA_Ctrl(CRYPT_RSA_Ctx *ctx, CRYPT_PkeyCtrl opt, void *val, uint32_t len);

int32_t CRYPT_RSA_Verify(CRYPT_RSA_Ctx *ctx, const uint8_t *data, uint32_t dataLen,
    const uint8_t *sign, uint32_t signLen);

int32_t CRYPT_RSA_Sign(CRYPT_RSA_Ctx *ctx, const uint8_t *data, uint32_t dataLen,
    uint8_t *sign, uint32_t *signLen);

/**
 * @ingroup rsa
 * @brief RSA public key encryption
 *
 * @param ctx [IN] RSA context structure
 * @param data [IN] Information to be encrypted
 * @param dataLen [IN] Length of the information to be encrypted
 * @param out [OUT] Pointer to the encrypted information output.
 * @param outLen [OUT] Pointer to the length of the encrypted information
 *
 * @retval CRYPT_NULL_INPUT                 Invalid null pointer input
 * @retval CRYPT_RSA_NO_KEY_INFO            does not contain the key information.
 * @retval CRYPT_RSA_ERR_INPUT_VALUE        The entered value does not meet the calculation conditions.
 * @retval CRYPT_RSA_BUFF_LEN_NOT_ENOUGH    Outbuf Insufficient
 * @retval CRYPT_MEM_ALLOC_FAIL             Memory allocation failure
 * @retval CRYPT_SECUREC_FAIL               A safe function error occurs.
 * @retval BN error.                        An error occurs in the internal BigNum operation.
 * @retval CRYPT_EAL_ALG_NOT_SUPPORT        does not register the encryption method.
 * @retval CRYPT_SUCCESS                    encryption succeeded.
*/
int32_t CRYPT_RSA_Encrypt(CRYPT_RSA_Ctx *ctx, const uint8_t *data, uint32_t dataLen,
    uint8_t *out, uint32_t *outLen);

/**
 * @ingroup rsa
 * @brief RSA private key decryption
 *
 * @param ctx [IN] RSA context structure
 * @param data [IN] Information to be decrypted
 * @param dataLen [IN] Length of the information to be decrypted
 * @param out [OUT] Pointer to the output information after decryption.
 * @param outLen [OUT] Pointer to the length of the decrypted information
 *
 * @retval CRYPT_NULL_INPUT                 Error null pointer input
 * @retval CRYPT_RSA_NO_KEY_INFO            does not contain the key information.
 * @retval CRYPT_RSA_ERR_INPUT_VALUE        The entered value does not meet the calculation conditions.
 * @retval CRYPT_RSA_BUFF_LEN_NOT_ENOUGH    Outbuf Insufficient
 * @retval CRYPT_MEM_ALLOC_FAIL             Memory allocation failure
 * @retval CRYPT_SECUREC_FAIL               A security function error occurs.
 * @retval CRYPT_EAL_ALG_NOT_SUPPORT        does not register the decryption method.
 * @retval BN error.                        An error occurs in the internal BigNum operation.
 * @retval CRYPT_SUCCESS                    Decryption succeeded.
 */
int32_t CRYPT_RSA_Decrypt(CRYPT_RSA_Ctx *ctx, const uint8_t *data, uint32_t dataLen,
    uint8_t *out, uint32_t *outLen);

/**
 * @ingroup rsa
 * @brief RSA compare the public key
 *
 * @param a [IN] RSA context structure
 * @param b [IN] RSA context structure
 *
 * @retval CRYPT_SUCCESS                is the same
 * @retval CRYPT_NULL_INPUT             Invalid null pointer input
 * @retval CRYPT_RSA_NO_KEY_INFO        No public key
 * @retval CRYPT_RSA_PUBKEY_NOT_EQUAL   Public Keys are not equal
 */
int32_t CRYPT_RSA_Cmp(const CRYPT_RSA_Ctx *a, const CRYPT_RSA_Ctx *b);

/**
 * @ingroup rsa
 * @brief oaep padding
 *
 * @param hashMethod [IN] Hash method. Only sha1, sha244, sha256, sha384, and sha512 are supported.
 * @param mgfMethod [IN] Hash method required by mgf
 * @param in [IN] Original data
 * @param inLen [IN] Original data length
 * @param param [IN] oaep parameter, which can be null
 * @param paramLen [IN] oaep Parameter length
 * @param pad [IN] Data after padding
 * @param padLen [IN] Data length after padding
 *
 * @retval CRYPT_NULL_INPUT                 Error null pointer input
 * @retval CRYPT_RSA_ERR_INPUT_VALUE        The entered value does not meet the calculation conditions.
 * @retval CRYPT_SECUREC_FAIL               A security function error occurs.
 * @retval CRYPT_MEM_ALLOC_FAIL             Memory allocation failure
 * @retval CRYPT_RSA_BUFF_LEN_NOT_ENOUGH    Outbuf Insufficient
 * */
int32_t CRYPT_RSA_SetPkcs1Oaep(const EAL_MdMethod *hashMethod, const EAL_MdMethod *mgfMethod, const uint8_t *in,
    uint32_t inLen, const uint8_t *param, uint32_t paramLen, uint8_t *pad, uint32_t padLen);

/**
 * @ingroup rsa
 * @brief Verify the oaep padding.
 *
 * @param hashMethod [IN] Hash method, which supports sha1, sha244, sha256, sha384, and sha512.
 * @param mgfMethod [IN] Hash method required by mgf
 * @param in [IN] Data after padding
 * @param inLen [IN] Data length after padding
 * @param param [IN] oaep parameter, which can be null
 * @param paramLen [IN] oaep Parameter length
 * @param msg [IN] Data after the de-padding
 * @param msgLen [IN/OUT] The input parameter is the length of the msg buffer,
 *                        and the output parameter is the length of the msg after the de-padding.
 *
 * @retval CRYPT_NULL_INPUT             Error null pointer input
 * @retval CRYPT_RSA_ERR_INPUT_VALUE    The entered value does not meet the calculation conditions.
 * @retval CRYPT_SECUREC_FAIL           A security function error occurs.
 * @retval CRYPT_MEM_ALLOC_FAIL         Memory allocation failure
 * */
int32_t CRYPT_RSA_VerifyPkcs1Oaep(const EAL_MdMethod *hashMethod, const EAL_MdMethod *mgfMethod, const uint8_t *in,
    uint32_t inLen, const uint8_t *param, uint32_t paramLen, uint8_t *msg, uint32_t *msgLen);

int32_t CRYPT_RSA_SetPkcsV15Type2(const uint8_t *in, uint32_t inLen,
    uint8_t *out, uint32_t outLen);

int32_t CRYPT_RSA_VerifyPkcsV15Type2(const uint8_t *in, uint32_t inLen,
    uint8_t *out, uint32_t *outLen);

/**
 * @ingroup RSA
 * @brief RSA get security bits
 *
 * @param ctx [IN] RSA Context structure
 *
 * @retval security bits
 */
int32_t CRYPT_RSA_GetSecBits(const CRYPT_RSA_Ctx *ctx);


#ifdef __cplusplus
}
#endif

#endif // HITLS_CRYPTO_RSA

#endif // CRYPT_RSA_H
