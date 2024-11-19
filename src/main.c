#include <stdlib.h> /* exit */
#include <unistd.h> /* getpid */

#include "log.h"
#include "options.h"

pid_t g_pid;

/* -------------------------------------------------------------------------- */

static
void _show_help(const char* program_name) {
    log_info("Usage: %s [OPTION] <destination>", program_name);
    log_info("\n"); // TODO description
    log_info("Options:");
    log_info("  -?, --help                  Display this help and exit");
}

static
FT_RESULT _check_privileges() {
    if (getuid() != 0) {
        log_error("root privileges required");
        return FT_FAILURE;
    }

    return FT_SUCCESS;
}

static
FT_RESULT _trigger() {

}

/* -------------------------------------------------------------------------- */

int main(const int arg_count, char* const* arg_value) {
    if (_check_privileges() == FT_FAILURE) {
        return EXIT_FAILURE;
    }
    if (retrieve_arguments(arg_count, arg_value) == FT_FAILURE) {
        return EXIT_FAILURE;
    }

    g_pid = getpid() & 0xFFFF | 0x8000; /* Retrieve the last 16 bits of the PID and set the most significant bit to 1 */

    if (g_arguments.m_options.m_help) {
        _show_help(arg_value[0]);
    }

    return EXIT_SUCCESS;
}