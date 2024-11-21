#include <stdlib.h> /* exit */
#include <unistd.h> /* getpid */
#include <stdio.h> /* printf */

#include "network_io.h"
#include "log.h"
#include "raw_socket.h"
#include "stats.h"
#include "options.h"
#include "signal_handlers.h"

/* --------------------------------- GLOBALS -------------------------------- */

pid_t       g_pid;

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
void _cleanup(void) {
    destroy_sockets();
    reset_signals();
}

static
FT_RESULT _traceroute() {
    /* Setup */
    if (create_sockets(g_arguments.m_destination) == FT_FAILURE) {
        return FT_FAILURE;
    }

    const Options* options = &g_arguments.m_options;

    for (u8 ttl = options->m_start_hop; ttl < options->m_max_hop; ++ttl) {
        printf("%2d  ", (u32)ttl);

        u32 query_count = 0;

        for (u8 i = 0; i < options->m_simultaneous; ++i) {
            if (query_count < options->m_queries) {
                if (send_request(ttl) == FT_FAILURE) {
                    return FT_FAILURE;
                }
                // ++g_stats.m_packet_sent; // unused
                ++query_count;
            }
        }

        for (u32 i = 0; i < options->m_queries; ++i) {
            enum e_Response response = wait_responses();

            if (response == RESPONSE_SUCCESS) {
                break;
            } else if (response == RESPONSE_TIMEOUT) {
                printf("* ");
            } else if (response == RESPONSE_ERROR) {
                return FT_FAILURE;
            }
        }
        printf("\n");
    }
    _cleanup();

    return FT_SUCCESS;
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
    } else if (_traceroute() == FT_FAILURE) {
        _cleanup();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}