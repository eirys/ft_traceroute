#include <ctype.h> /* isdigit */
#include <stdlib.h> /* strtoul */
#include <stdio.h> /* sscanf */

#include "typedefs.h"
#include "network_io.h"
#include "log.h"

/* -------------------------------------------------------------------------- */

static
int _isfloat(int c) {
    return c == '.' || isdigit(c);
}


static
FT_RESULT _check_str_kind(const char* value, int (*kind)(int)) {
    const char* copy = value;

    while (*copy != '\0') {
        if (kind(*copy) == 0) {
            log_error("invalid argument: `%s`", value);
            return FT_FAILURE;
        }
        ++copy;
    }

    return FT_SUCCESS;
}

/* -------------------------------------------------------------------------- */

FT_RESULT process_uint(const char* raw, void* flag, void* next, const char* err) {
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

    *((u64*)flag) = (u64)result;

    return FT_SUCCESS;
}

FT_RESULT process_ushort(const char* raw, void* flag, void* next, const char* err) {
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

    *((u16*)flag) = (u16)result;

    return FT_SUCCESS;
}

FT_RESULT process_uchar(const char* raw, void* flag, void* next, const char* err) {
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

    *((u8*)flag) = (u8)result;

    return FT_SUCCESS;
}

FT_RESULT process_float(const char* raw, void* flag, void* next, const char* err) {
    if (_check_str_kind(raw, &_isfloat) == FT_FAILURE) {
        return FT_FAILURE;
    }

    f32 result;
    if (sscanf(raw, "%f", &result) != 1) {
        log_error("bad value: `%s'", raw);
        return FT_FAILURE;
    }

    *((f32*)flag) = result;

    return FT_SUCCESS;
}


FT_RESULT extra_over_zero(void* flag, const char* err) {
    if (*(u64*)flag == 0) {
        log_error("bad value: %s", err);
        return FT_FAILURE;
    }

    return FT_SUCCESS;
}

FT_RESULT extra_default_sport(void* flag, const char* err) {
    (void)err;

    if (*(u16*)flag == 0) {
        *(u16*)flag = (u64)g_pid;
    }

    return FT_SUCCESS;
}

FT_RESULT extra_default_len(void* flag, const char* err) {
    (void)err;

    if (*(u64*)flag > IP_HEADER_SIZE + UDP_HEADER_SIZE) {
        *(u64*)flag = *(u64*)flag - (IP_HEADER_SIZE + UDP_HEADER_SIZE);
    } else {
        *(u64*)flag = 0;
    }

    return FT_SUCCESS;
}