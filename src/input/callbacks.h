#ifndef CALLBACKS_H
# define CALLBACKS_H

#include "typedefs.h"

FT_RESULT process_uint(const char* raw, void* value, void* next);
FT_RESULT process_ushort(const char* raw, void* value, void* next);
FT_RESULT process_uchar(const char* raw, void* value, void* next);
FT_RESULT process_float(const char* raw, void* value, void* next);

FT_RESULT extra_port(void* value);
FT_RESULT extra_probes(void* flag);
FT_RESULT extra_default_sport(void* value);
FT_RESULT extra_default_len(void* flag);

#endif /* CALLBACKS_H */