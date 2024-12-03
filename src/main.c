#include <stdlib.h> /* exit */
#include <unistd.h> /* getpid */
#include <stdio.h> /* printf */
#include <sys/param.h> /* MAX */
#include <arpa/inet.h> /* inet_ntoa */

#include "network_io.h"
#include "log.h"
#include "raw_socket.h"
#include "options.h"
#include "signal_handlers.h"

/* --------------------------------- GLOBALS -------------------------------- */

pid_t g_pid;

/* -------------------------------------------------------------------------- */

static void deb() {
    printf(" packet len = %d\n", g_arguments.m_packet_size);
    printf(" queries = %d\n", g_arguments.m_options.m_queries);
    // printf(" simultaneous = %d\n", g_arguments.m_options.m_simultaneous); // TODO
    printf(" wait = %f\n", g_arguments.m_options.m_timeout);
    printf(" sport = %d\n", g_arguments.m_options.m_src_port);
    printf(" port = %d\n", g_arguments.m_options.m_dest_port);
    printf(" tos = %d\n", g_arguments.m_options.m_tos);
    printf(" max hops = %d\n", g_arguments.m_options.m_max_hop);
    printf(" first hop = %d\n", g_arguments.m_options.m_start_hop);
    printf(" numeric = %d\n", g_arguments.m_options.m_numeric);
    printf(" help = %d\n", g_arguments.m_options.m_help);
    printf(" destination = %s\n", g_arguments.m_destination);
}

static
void _show_help(const char* program_name) {
    log_info("Usage: %s [OPTION] <destination> [packet_len]", program_name);
    log_info("Trace the route to a destination by sending UDP packets with increasing TTLs");
    log_info("Options:");
    log_info("  packet_len              Size of the packet (default: %u)\n", g_arguments.m_packet_size);
    log_info("  --help                  Display this help and exit");
    log_info("  -q, --queries <n>       Number of queries/probes sent (default: %u)", g_arguments.m_options.m_queries);
    // log_info("  -N, --sim-queries <n>   Number of simultaneous probes (default: 16)"); // TODO
    log_info("  -w, --wait <n>          Timeout for a probe (default: %u)", g_arguments.m_options.m_timeout);
    log_info("  --sport <n>             Source port (default: %u)", (u32)g_arguments.m_options.m_src_port);
    log_info("  -p, --port <n>          Destination port (default: %u)", (u32)g_arguments.m_options.m_dest_port);
    log_info("  -t, --tos <n>           Type of Service (default: %u)", (u32)g_arguments.m_options.m_tos);
    log_info("  -m, --max-hops <n>      Max TTL sent (default: %u)", (u32)g_arguments.m_options.m_max_hop);
    log_info("  -f, --first-hop <n>     First TTL sent (default: %u)", (u32)g_arguments.m_options.m_start_hop);
    log_info("  -n, --numeric           Numeric output only");
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
    deallocate_buffer();
    reset_signals();
    destroy_sockets();
}

/* -------------------------------------------------------------------------- */

//TODO: multithreading for simultaneous probes
static
FT_RESULT _traceroute() {
    /* Setup */
    if (create_sockets(g_arguments.m_destination) == FT_FAILURE) {
        return FT_FAILURE;
    }
    if (set_signals() == FT_FAILURE) {
        return FT_FAILURE;
    }
    if (allocate_buffer() == FT_FAILURE) {
        return FT_FAILURE;
    }

    printf("ft_traceroute to %s (%s), %u hops max, %d byte packets\n",
        g_arguments.m_destination,
        inet_ntoa(g_udp_socket.m_ipv4),
        g_arguments.m_options.m_max_hop,
        g_arguments.m_packet_size + IP_HEADER_SIZE + UDP_HEADER_SIZE);

    const Options*  options = &g_arguments.m_options;
    bool            done = false;
    u32             successes = 0U;

    for (u8 ttl = options->m_start_hop; ttl <= options->m_max_hop && !done; ++ttl) {
        printf("%2d  ", (u32)ttl);

        for (u32 i = 0; i < options->m_queries; ++i) {
            if (send_request(ttl) == FT_FAILURE) {
                return FT_FAILURE;
            }

            enum e_Response response = wait_responses();

            if (response == RESPONSE_SUCCESS) {
                ++successes;
            } else if (response == RESPONSE_ERROR) {
                return FT_FAILURE;
            }

            fflush(stdout);

            ++g_sequence;

            if (successes == options->m_queries) {
                done = true;
                break;
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

    g_pid = getpid() & 0xFFFF | 0x8000; /* Retrieve the last 16 bits of the PID and set the most significant bit to 1 */

    if (retrieve_arguments(arg_count, arg_value) == FT_FAILURE) {
        return EXIT_FAILURE;
    }
    //TODO: remove
    deb();

    if (g_arguments.m_options.m_help) {
        _show_help(arg_value[0]);
    } else if (_traceroute() == FT_FAILURE) {
        _cleanup();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}