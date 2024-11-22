#include "hitls_build.h"
#ifdef HITLS_BSL_PARAMS
#include "bsl_errno.h"
#include "securec.h"
#include "bsl_err_internal.h"
#include "bsl_params.h"

#define BSL_PARAM_MAX_NUMBER 1000

int32_t BSL_PARAM_InitValue(BSL_Param *param, int32_t key, uint32_t type, void *val, uint32_t valueLen)
{
    if (key == 0) {
        BSL_ERR_PUSH_ERROR(BSL_PARAMS_INVALID_KEY);
        return BSL_PARAMS_INVALID_KEY;
    }
    /* Parameter validation: param cannot be NULL, if val is NULL, valueLen must be 0 */
    if ((val == NULL && valueLen != 0) || param == NULL) {
        BSL_ERR_PUSH_ERROR(BSL_INVALID_ARG);
        return BSL_INVALID_ARG;
    }

    switch (type) {
        case BSL_PARAM_TYPE_UINT32:
        case BSL_PARAM_TYPE_OCTETS:
        case BSL_PARAM_TYPE_UINT32_PTR:
            param->value = val;
            param->valueLen = valueLen;
            param->valueType = type;
            param->key = key;
            param->useLen = 0;
            return BSL_SUCCESS;
        default:
            BSL_ERR_PUSH_ERROR(BSL_PARAMS_INVALID_TYPE);
            return BSL_PARAMS_INVALID_TYPE;
    }
}

int32_t BSL_PARAM_SetValue(BSL_Param *param, int32_t key, uint32_t type, void *val, uint32_t len)
{
    if (key == 0) {
        BSL_ERR_PUSH_ERROR(BSL_PARAMS_INVALID_KEY);
        return BSL_PARAMS_INVALID_KEY;
    }
    if (param == NULL) {
        BSL_ERR_PUSH_ERROR(BSL_INVALID_ARG);
        return BSL_INVALID_ARG;
    }
    if (param->key != key || param->valueType != type) {
        BSL_ERR_PUSH_ERROR(BSL_PARAMS_MISMATCH);
        return BSL_PARAMS_MISMATCH;
    }
    switch (type) {
        case BSL_PARAM_TYPE_OCTETS_PTR:
            param->value = val;
            param->useLen = len;
            return BSL_SUCCESS;
        case BSL_PARAM_TYPE_UINT32:
            if (param->valueLen != len || val == NULL || param->value == NULL) {
                BSL_ERR_PUSH_ERROR(BSL_INVALID_ARG);
                return BSL_INVALID_ARG;
            }
            *(uint32_t *)param->value = *(uint32_t *)val;
            param->useLen = len;
            return BSL_SUCCESS;
        case BSL_PARAM_TYPE_OCTETS:
            if (param->valueLen < len || val == NULL || param->value == NULL) {
                BSL_ERR_PUSH_ERROR(BSL_INVALID_ARG);
                return BSL_INVALID_ARG;
            }
            (void)memcpy_s(param->value, len, val, len);
            param->useLen = len;
            return BSL_SUCCESS;
        default:
            BSL_ERR_PUSH_ERROR(BSL_PARAMS_INVALID_TYPE);
            return BSL_PARAMS_INVALID_TYPE;
    }
}

int32_t BSL_PARAM_GetPtrValue(const BSL_Param *param, int32_t key, uint32_t type, void **val, uint32_t *valueLen)
{
    if (key == 0) {
        BSL_ERR_PUSH_ERROR(BSL_PARAMS_INVALID_KEY);
        return BSL_PARAMS_INVALID_KEY;
    }
    if (param == NULL || val == NULL || valueLen == NULL) {
        BSL_ERR_PUSH_ERROR(BSL_INVALID_ARG);
        return BSL_INVALID_ARG;
    }
    if (param->key != key || param->valueType != type) {
        BSL_ERR_PUSH_ERROR(BSL_PARAMS_MISMATCH);
        return BSL_PARAMS_MISMATCH;
    }
    switch (type) {
        case BSL_PARAM_TYPE_UINT32_PTR:
        case BSL_PARAM_TYPE_OCTETS_PTR:
            *val = param->value;
            *valueLen = param->valueLen;
            return BSL_SUCCESS;
        default:
            BSL_ERR_PUSH_ERROR(BSL_PARAMS_INVALID_TYPE);
            return BSL_PARAMS_INVALID_TYPE;
    }
}

int32_t BSL_PARAM_GetValue(const BSL_Param *param, int32_t key, uint32_t type, void *val, uint32_t *valueLen)
{
    if (key == 0) {
        BSL_ERR_PUSH_ERROR(BSL_PARAMS_INVALID_KEY);
        return BSL_PARAMS_INVALID_KEY;
    }
    if (param == NULL || val == NULL || valueLen == NULL) {
        BSL_ERR_PUSH_ERROR(BSL_INVALID_ARG);
        return BSL_INVALID_ARG;
    }
    if (param->key != key || param->valueType != type) {
        BSL_ERR_PUSH_ERROR(BSL_PARAMS_MISMATCH);
        return BSL_PARAMS_MISMATCH;
    }
    switch (type) {
        case BSL_PARAM_TYPE_UINT32:
        case BSL_PARAM_TYPE_OCTETS:
            if (*valueLen < param->valueLen) {
                BSL_ERR_PUSH_ERROR(BSL_INVALID_ARG);
                return BSL_INVALID_ARG;
            }
            (void)memcpy_s(val, param->valueLen, param->value, param->valueLen);
            *valueLen = param->valueLen;
            return BSL_SUCCESS;
        default:
            BSL_ERR_PUSH_ERROR(BSL_PARAMS_INVALID_TYPE);
            return BSL_PARAMS_INVALID_TYPE;
    }
}

const BSL_Param *BSL_PARAM_FindParam(const BSL_Param *param, int32_t key)
{
    if (key == 0) {
        BSL_ERR_PUSH_ERROR(BSL_PARAMS_INVALID_KEY);
        return NULL;
    }
    if (param == NULL) {
        BSL_ERR_PUSH_ERROR(BSL_INVALID_ARG);
        return NULL;
    }
    int32_t index = 0;
    while (param[index].key != 0 && index < BSL_PARAM_MAX_NUMBER) {
        if (param[index].key == key) {
            return &param[index];
        }
        index++;
    }
    BSL_ERR_PUSH_ERROR(BSL_PARAMS_MISMATCH);
    return NULL;
}

#endif