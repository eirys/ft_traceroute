#include <arpa/inet.h> /* inet_ntop */

#include "libft.h"
#include "raw_socket.h"
#include "log.h"
#include "wrapper.h"
#include "options.h"

/* -------------------------------------------------------------------------- */

SocketInfo  g_udp_socket = {
    .m_fd = -1
};

SocketInfo  g_raw_socket = {
    .m_fd = -1
};

/* -------------------------------------------------------------------------- */

static struct addrinfo* _destination_info = NULL;

/* -------------------------------------------------------------------------- */

static
struct addrinfo* _resolve_ip(const char* destination) {
    struct addrinfo* output;

    struct addrinfo hints;
    ft_memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;          /* Allow IPv4 */

    if (Getaddrinfo(destination, NULL, &hints, &output) == FT_FAILURE) {
        return NULL;
    }

    return output;
}

static
FT_RESULT _destroy_malformed_data(void) {
    if (_destination_info != NULL) {
        freeaddrinfo(_destination_info);
    }

    destroy_sockets();

    return FT_FAILURE;
}

/* -------------------------------------------------------------------------- */

/**
 * @brief Open sending socket (UDP) and receiving socket (RAW).
 */
FT_RESULT create_sockets(const char* destination) {
    _destination_info = _resolve_ip(destination);
    if (_destination_info == NULL) {
        return _destroy_malformed_data();
    }

    /* Open raw socket */
    g_raw_socket.m_fd = Socket(_destination_info->ai_family, SOCK_RAW, IPPROTO_ICMP);
    g_raw_socket.m_ipv4.s_addr = INADDR_ANY;

    /* Open udp socket */
    g_udp_socket.m_fd = Socket(_destination_info->ai_family, SOCK_DGRAM, 0);

    if (g_raw_socket.m_fd == -1 || g_udp_socket.m_fd == -1) {
        return _destroy_malformed_data();
    }

    /* Remember destination IPv4 */
    g_udp_socket.m_ipv4 = ((struct sockaddr_in*)_destination_info->ai_addr)->sin_addr;

    /* Set TOS */
    setsockopt(g_udp_socket.m_fd, IPPROTO_IP, IP_TOS, &g_arguments.m_options.m_tos, sizeof(u8));

    /* Bind udp socket source port */
    struct sockaddr_in source = {
        .sin_family = AF_INET,
        .sin_port = htons(g_arguments.m_options.m_src_port),
        .sin_addr = { .s_addr = INADDR_ANY }
    };

    if (Bind(g_udp_socket.m_fd, (struct sockaddr*)&source, sizeof(source)) == FT_FAILURE) {
        return _destroy_malformed_data();
    }

    freeaddrinfo(_destination_info);
    _destination_info = NULL;

    return FT_SUCCESS;
}

void destroy_sockets(void) {
    if (g_raw_socket.m_fd != -1) {
        Close(g_raw_socket.m_fd);
    }
    if (g_udp_socket.m_fd != -1) {
        Close(g_udp_socket.m_fd);
    }
}