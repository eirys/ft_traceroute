#ifndef LOG_H
# define LOG_H

void        log_info(const char* message, ...);
void        log_debug(const char* function_name, const char* message, ...);
void        log_error(const char* message, ...);

#endif /* LOG_H */