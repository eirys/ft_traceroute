#ifndef CALLBACKS_H
# define CALLBACKS_H

#include "typedefs.h"

FT_RESULT process_uint(const char* raw, void* value, void* next, const char* err);
FT_RESULT process_ushort(const char* raw, void* value, void* next, const char* err);
FT_RESULT process_uchar(const char* raw, void* value, void* next, const char* err);
FT_RESULT process_float(const char* raw, void* value, void* next, const char* err);

FT_RESULT extra_over_zero(void* value, const char* err);
FT_RESULT extra_default_sport(void* value, const char* err);
FT_RESULT extra_default_len(void* flag, const char* err);

#endif /* CALLBACKS_H */