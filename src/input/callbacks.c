#include <ctype.h> /* isdigit */
#include <stdlib.h> /* strtoul */

#include "typedefs.h"
#include "log.h"

/* -------------------------------------------------------------------------- */

static
FT_RESULT _check_str_kind(const char* value, int (*kind)(int)) {
    const char* copy = value;

    while (*copy != '\0') {
        if (kind(*copy) == 0) {
            log_error("invalid argument: `%s`", (char*)value);
            return FT_FAILURE;
        }
        copy++;
    }

    return FT_SUCCESS;
}

/* -------------------------------------------------------------------------- */

FT_RESULT process_uint(const char* raw, void* value, void* next, const char* err) {
    if (_check_str_kind(raw, &isdigit) == FT_FAILURE) {
        return FT_FAILURE;
    }

    const u64 result = strtoul(raw, NULL, 10);

    if (next != NULL) {
        FT_RESULT (*extra)(void*, const char*) = (FT_RESULT (*)(void*, const char*))next;
        if (extra((void*)&result, err) == FT_FAILURE) {
            return FT_FAILURE;
        }
    }

    *((u64*)value) = (u64)result;

    return FT_SUCCESS;
}

FT_RESULT process_ushort(const char* raw, void* value, void* next, const char* err) {
    if (_check_str_kind(raw, &isdigit) == FT_FAILURE) {
        return FT_FAILURE;
    }

    const int result = atoi(raw);
    if (result < 0 || result > UINT16_MAX) {
        log_error("bad value: `%s'", raw);
        return FT_FAILURE;
    }

    if (next != NULL) {
        FT_RESULT (*extra)(void*, const char*) = (FT_RESULT (*)(void*, const char*))next;
        if (extra((void*)&result, err) == FT_FAILURE) {
            return FT_FAILURE;
        }
    }

    *((u16*)value) = (u16)result;

    return FT_SUCCESS;
}

FT_RESULT process_uchar(const char* raw, void* value, void* next, const char* err) {
    if (_check_str_kind(raw, &isdigit) == FT_FAILURE) {
        return FT_FAILURE;
    }

    const int result = atoi(raw);
    if (result < 0 || result > UINT8_MAX) {
        log_error("bad value: `%s'", raw);
        return FT_FAILURE;
    }

    if (next != NULL) {
        FT_RESULT (*extra)(void*, const char*) = (FT_RESULT (*)(void*, const char*))next;
        if (extra((void*)&result, err) == FT_FAILURE) {
            return FT_FAILURE;
        }
    }

    *((u8*)value) = (u8)result;

    return FT_SUCCESS;
}

FT_RESULT extra_over_zero(void* value, const char* err) {
    u64* number = (u64*)value;

    if (*number == 0) {
        log_error("bad value: %s", err);
        return FT_FAILURE;
    }

    return FT_SUCCESS;
}