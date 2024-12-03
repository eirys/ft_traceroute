#include <stdlib.h> /* strtoul */
#include <stdio.h> /* sscanf */

#include "libft.h"
#include "typedefs.h"
#include "network_io.h"
#include "log.h"

/* -------------------------------------------------------------------------- */

static
FT_RESULT _isfloat(const char* s) {
    while (ft_isdigit(*s) != 0) {
        ++s;
    }

    if (*s == '.') {
        ++s;
    }

    while (ft_isdigit(*s) != 0) {
        ++s;
    }

    return *s == '\0' ? FT_SUCCESS : FT_FAILURE;
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

FT_RESULT process_uint(const char* raw, void* flag, void* next) {
    if (_check_str_kind(raw, &ft_isdigit) == FT_FAILURE) {
        return FT_FAILURE;
    }

    const u64 result = ft_atou(raw);

    if (next != NULL) {
        FT_RESULT (*extra)(void*) = (FT_RESULT (*)(void*))next;
        if (extra((void*)&result) == FT_FAILURE) {
            return FT_FAILURE;
        }
    }

    *((u32*)flag) = (u32)result;

    return FT_SUCCESS;
}

FT_RESULT process_ushort(const char* raw, void* flag, void* next) {
    if (_check_str_kind(raw, &ft_isdigit) == FT_FAILURE) {
        return FT_FAILURE;
    }

    const int result = ft_atoi(raw);
    if (result < 0 || result > UINT16_MAX) {
        log_error("bad value: `%s'", raw);
        return FT_FAILURE;
    }

    if (next != NULL) {
        FT_RESULT (*extra)(void*) = (FT_RESULT (*)(void*))next;
        if (extra((void*)&result) == FT_FAILURE) {
            return FT_FAILURE;
        }
    }

    *((u16*)flag) = (u16)result;

    return FT_SUCCESS;
}

FT_RESULT process_uchar(const char* raw, void* flag, void* next) {
    if (_check_str_kind(raw, &ft_isdigit) == FT_FAILURE) {
        return FT_FAILURE;
    }

    const int result = ft_atoi(raw);
    if (result < 0 || result > UINT8_MAX) {
        log_error("bad value: `%s'", raw);
        return FT_FAILURE;
    }

    if (next != NULL) {
        FT_RESULT (*extra)(void*) = (FT_RESULT (*)(void*))next;
        if (extra((void*)&result) == FT_FAILURE) {
            return FT_FAILURE;
        }
    }

    *((u8*)flag) = (u8)result;

    return FT_SUCCESS;
}

FT_RESULT process_float(const char* raw, void* flag, void* next) {
    if (_isfloat(raw) == FT_FAILURE) {
        return FT_FAILURE;
    }

    f32 result;
    if (sscanf(raw, "%f", &result) != 1) {
        log_error("bad value: `%s'", raw);
        return FT_FAILURE;
    }

    if (next != NULL) {
        FT_RESULT (*extra)(void*) = (FT_RESULT (*)(void*))next;
        if (extra((void*)&result) == FT_FAILURE) {
            return FT_FAILURE;
        }
    }

    *((f32*)flag) = result;

    return FT_SUCCESS;
}


FT_RESULT extra_port(void* flag) {
    if (*(u16*)flag == 0) {
        log_error("can't set destination port to 0");
        return FT_FAILURE;
    }

    return FT_SUCCESS;
}

FT_RESULT extra_default_sport(void* flag) {
    if (*(u16*)flag == 0) {
        *(u16*)flag = (u64)g_pid;
    }

    return FT_SUCCESS;
}

FT_RESULT extra_default_len(void* flag) {
    if (*(u32*)flag > IP_HEADER_SIZE + UDP_HEADER_SIZE) {
        *(u32*)flag = *(u32*)flag - (IP_HEADER_SIZE + UDP_HEADER_SIZE);
    } else {
        *(u32*)flag = 0;
    }

    return FT_SUCCESS;
}

FT_RESULT extra_probes(void* flag) {
    if (*(u32*)flag == 0) {
        log_error("can't send 0 probes");
        return FT_FAILURE;
    } else if (*(u32*)flag > 10) {
        log_error("no more than 10 probes per hop");
        return FT_FAILURE;
    }

    return FT_SUCCESS;
}