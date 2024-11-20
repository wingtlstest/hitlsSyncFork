#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "securec.h"

#include "bsl_sal.h"
#include "bsl_err.h"
#include "crypt_algid.h"
#include "crypt_eal_rand.h"
#include "hitls_error.h"
#include "hitls_config.h"
#include "hitls.h"
#include "hitls_cert_init.h"
#include "hitls_cert.h"
#include "hitls_crypt_init.h"
#include "hitls_pki.h"

#define CERTS_PATH      "../../../testcode/testdata/tls/certificate/der/ecdsa_sha256/"
#define HTTP_BUF_MAXLEN (18 * 1024) /* 18KB */

int main(int32_t argc, char *argv[])
{
    int32_t exitValue = -1;
    int32_t ret = 0;
    HITLS_Config *config = NULL;
    HITLS_Ctx *ctx = NULL;
    BSL_UIO *uio = NULL;
    int fd = 0;
    HITLS_X509_Cert *rootCA = NULL;
    HITLS_X509_Cert *subCA = NULL;

    /* 注册BSL内存能力、仅供参考 */
    BSL_SAL_MemCallback memMthod = {(void *(*)(uint32_t size))malloc, free};
    BSL_SAL_RegMemCallback(&memMthod);
    BSL_ERR_Init();

    HITLS_CertMethodInit();
    CRYPT_EAL_RandInit(CRYPT_RAND_SHA256, NULL, NULL, NULL, 0);
    HITLS_CryptMethodInit();

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        printf("Create socket failed.\n");
        return -1;
    }
    int option = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0) {
        close(fd);
        printf("setsockopt SO_REUSEADDR failed.\n");
        return -1;
    }

    // Set the protocol and port number
    struct sockaddr_in serverAddr;
    (void)memset_s(&serverAddr, sizeof(serverAddr), 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) != 0) {
        printf("connect failed.\n");
        goto exit;
    }

    config = HITLS_CFG_NewTLS12Config();
    if (config == NULL) {
        printf("HITLS_CFG_NewTLS12Config failed.\n");
        goto exit;
    }
    ret = HITLS_CFG_SetCheckKeyUsage(config, false); // disable cert keyusage check
    if (ret != HITLS_SUCCESS) {
        printf("Disable check KeyUsage failed.\n");
        goto exit;
    }

    /* 加载证书：需要用户实现 */
    ret = HITLS_X509_CertParseFile(BSL_FORMAT_ASN1, CERTS_PATH "ca.der", &rootCA);
    if (ret != HITLS_SUCCESS) {
        printf("Parse ca failed.\n");
        goto exit;
    }
    ret = HITLS_X509_CertParseFile(BSL_FORMAT_ASN1, CERTS_PATH "inter.der", &subCA);
    if (ret != HITLS_SUCCESS) {
        printf("Parse subca failed.\n");
        goto exit;
    }
    HITLS_CFG_AddCertToStore(config, rootCA, TLS_CERT_STORE_TYPE_DEFAULT, true);
    HITLS_CFG_AddCertToStore(config, subCA, TLS_CERT_STORE_TYPE_DEFAULT, true);

    /* 新建openHiTLS上下文 */
    ctx = HITLS_New(config);
    if (ctx == NULL) {
        printf("HITLS_New failed.\n");
        goto exit;
    }

    uio = BSL_UIO_New(BSL_UIO_TcpMethod());
    if (uio == NULL) {
        printf("BSL_UIO_New failed.\n");
        goto exit;
    }

    ret = BSL_UIO_Ctrl(uio, BSL_UIO_SET_FD, (int32_t)sizeof(fd), &fd);
    if (ret != HITLS_SUCCESS) {
        BSL_UIO_Free(uio);
        printf("BSL_UIO_SET_FD failed, fd = %u.\n", fd);
        goto exit;
    }

    ret = HITLS_SetUio(ctx, uio);
    if (ret != HITLS_SUCCESS) {
        BSL_UIO_Free(uio);
        printf("HITLS_SetUio failed. ret = 0x%x.\n", ret);
        goto exit;
    }

    /* 进行TLS连接、用户需按实际场景考虑返回值 */
    ret = HITLS_Connect(ctx);
    if (ret != HITLS_SUCCESS) {
        printf("HITLS_Connect failed, ret = 0x%x.\n", ret);
        goto exit;
    }

    /* 向对端发送报文、用户需按实际场景考虑返回值 */
    const uint8_t sndBuf[] = "Hi, this is client\n";
    ret = HITLS_Write(ctx, sndBuf, sizeof(sndBuf));
    if (ret != HITLS_SUCCESS) {
        printf("HITLS_Write error:error code:%d\n", ret);
        goto exit;
    }

    /* 读取对端报文、用户需按实际场景考虑返回值 */
    uint8_t readBuf[HTTP_BUF_MAXLEN + 1] = {0};
    uint32_t readLen = 0;
    ret = HITLS_Read(ctx, readBuf, HTTP_BUF_MAXLEN, &readLen);
    if (ret != HITLS_SUCCESS) {
        printf("HITLS_Read failed, ret = 0x%x.\n", ret);
        goto exit;
    }

    printf("get from server size:%u :%s\n", readLen, readBuf);

    exitValue = 0;
exit:
    HITLS_Close(ctx);
    HITLS_Free(ctx);
    HITLS_CFG_FreeConfig(config);
    close(fd);
    HITLS_X509_CertFree(rootCA);
    HITLS_X509_CertFree(subCA);
    return exitValue;
}