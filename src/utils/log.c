#include <stdio.h>
#include <stdarg.h>
#include <string.h>

void log_info(const char* message, ...) {
    va_list args;
    va_start(args, message);

    vfprintf(stdout, message, args);
    fprintf(stdout, "\n");

    va_end(args);
}

void log_error(const char* message, ...) {
    va_list args;
    va_start(args, message);

    fprintf(stderr, "error: ");
    vfprintf(stderr, message, args);
    fprintf(stdout, "\n");

    va_end(args);
}

#ifdef __DEBUG
void log_debug(const char* function_name, const char* message, ...) {
    va_list args;
    va_start(args, message);

    fprintf(stdout, "[Debug from `%s'] ", function_name);
    vfprintf(stdout, message, args);
    fprintf(stdout, "\n");

    va_end(args);
}
#else
void log_debug(const char* function_name, const char* message, ...) { (void)function_name; (void)message; }
#endif
