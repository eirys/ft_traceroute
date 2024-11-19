#include "network_io.h"

#include <arpa/inet.h> /* inet_ntop */
#include <sys/param.h> /* MIN MAX */
#include <stdio.h> /* printf */

#include "options.h"
#include "stats.h"
#include "raw_socket.h"
#include "wrapper.h"
#include "log.h"

/* --------------------------------- STATIC --------------------------------- */

static fd_set           _listen_fds;
static struct timeval   _timeout;

/* -------------------------------------------------------------------------- */

/**
 * @brief Receive an IPv4 packet.
 */
static
FT_RESULT _receive_packet(u8* buffer, PacketInfo* packet_info) {
    struct sockaddr raw_src;
    socklen_t src_len = sizeof(raw_src);

    ssize_t bytes_received = Recvfrom(g_raw_socket.m_fd, buffer, RECV_BUF_SIZE, 0x0, &raw_src, &src_len);
    if (bytes_received < 0)
        return FT_FAILURE;

    packet_info->m_size = (u32)bytes_received;
    packet_info->m_addr = *(struct sockaddr_in*)&raw_src;

    if (Gettimeofday(&packet_info->m_timestamp, NULL) == FT_FAILURE)
        return FT_FAILURE;

    return FT_SUCCESS;
}

/**
 * @brief Filter properly formed ICMPv4 packets that correspond to the sent packet.
 */
static
FT_RESULT _filter_icmpv4(const Packet* packet, PacketInfo* packet_info) {
    if (packet_info->m_addr.sin_family != AF_INET) {
        log_debug("_filter_icmpv4", "not ipv4");
        return FT_FAILURE;
    }

    /* Not ICMPv4 */
    if (packet->m_ip_header.protocol != IPPROTO_ICMP) {
        log_debug("_filter_icmpv4", "not an ICMP message");
        return FT_FAILURE;
    }

    /* Incomplete packet_info */
    const u32 ip_size = packet->m_ip_header.ihl * sizeof(u32);
    if (packet_info->m_icmp_size - ip_size < ICMP_HEADER_SIZE) {
        log_debug("_filter_icmpv4", "packet malformed");
        return FT_FAILURE;
    }

    /* Not corresponding */
    if (packet->m_icmp.m_header.un.echo.id != g_pid
        || packet->m_icmp.m_header.un.echo.sequence != g_stats.m_packet_sent) { //TODO
        log_debug("_filter_icmpv4", "packet received is not for us");
        return FT_FAILURE;
    }

    packet_info->m_ip_size = ip_size;
    packet_info->m_icmp_size = packet_info->m_size - ip_size;

    return FT_SUCCESS;
}

/**
 * @brief Subtract two timeval structures and compute the result in milliseconds.
 */
static
double _compute_rtt(const struct timeval* t1, const struct timeval* t2) {
    long int seconds = t1->tv_sec - t2->tv_sec;
    long int microseconds = t1->tv_usec - t2->tv_usec;
    if (microseconds < 0) {
        seconds -= 1; /* Result is 1 second */
        microseconds += 1e7;
    }
    return (seconds * (double)1e3 + microseconds / (double)1e3);
}

// static
// void _process_reply(const Packet* packet) {
//     printf("icmp_seq=%u ttl=%d", packet->m_icmp_header->un.echo.sequence, packet->m_ip_header->ttl);

//     if (packet->m_icmp_size >= sizeof(struct timeval)) {
//         const struct timeval*   sent_tv = (struct timeval*)packet->m_icmp_payload;
//         const double            rtt = _compute_rtt(&packet->m_timestamp, sent_tv);

//         printf(" time=%.3f ms", rtt);

//         g_stats.m_packet_received += 1;
//         g_stats.m_min_rtt = MIN(rtt, g_stats.m_min_rtt);
//         g_stats.m_max_rtt = MAX(rtt, g_stats.m_max_rtt);
//         g_stats.m_total_rtt += rtt;
//         g_stats.m_total_rtt_square += rtt * rtt;
//     }
//     printf("\n");
// }

static
void _translate_source(const Packet* packet, const PacketInfo* packet_info) {
    bool is_numeric = g_arguments.m_options.m_numeric;

    char src_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &packet_info->m_addr.sin_addr, src_ip, INET_ADDRSTRLEN);

    char src_host[NI_MAXHOST];
    if (!is_numeric && Getnameinfo((struct sockaddr*)&packet_info->m_addr, sizeof(struct sockaddr_in), src_host, NI_MAXHOST, NULL, 0, NI_NAMEREQD) == FT_SUCCESS) {
        printf("%s (%s)  ", src_host, src_ip);
    } else {
        printf("%s  ", src_ip);
    }
}

static
enum e_Response _keep_going(const Packet* packet, const PacketInfo* packet_info) {
    return RESPONSE_ONGOING;
}

static
enum e_Response _reach_destination(const Packet* packet, const PacketInfo* packet_info) {
    return RESPONSE_SUCCESS;
}

static
enum e_Response _process_message(const Packet* packet, const PacketInfo* packet_info) {
    if (packet->m_icmp.m_header.type == ICMP_DEST_UNREACH)
        return _reach_destination(packet, packet_info);
    else if (packet->m_icmp.m_header.type == ICMP_TIME_EXCEEDED)
        return _keep_going(packet, packet_info);
    return RESPONSE_IGNORE;
}

/* -------------------------------------------------------------------------- */

/**
 * @brief Set a timeout for response reception.
 */
static
void _reset_timeout() {
    _timeout.tv_sec = g_arguments.m_options.m_timeout;
    _timeout.tv_usec = 0;
}

enum e_Response wait_responses() {
    union u_InPacket buffer;

    FD_ZERO(&_listen_fds);
    FD_SET(g_raw_socket.m_fd, &_listen_fds);
    _reset_timeout();

    while (true) {
        int fds = Select(g_raw_socket.m_fd + 1, &_listen_fds, NULL, NULL, &_timeout);
        if (fds == -1) {
            return RESPONSE_ERROR;
        } else if (fds == 0) { /* Timeout */
            return RESPONSE_TIMEOUT;
        }

        const Packet*   packet = &buffer.m_packet;
        PacketInfo      packet_info;

        if (_receive_packet(buffer.m_raw, &packet_info) == FT_FAILURE)
            return RESPONSE_ERROR;

        if (_filter_icmpv4(packet, &packet_info) == FT_FAILURE)
            continue;

        enum e_Response response = _process_message(packet, &packet_info);

        if (response != RESPONSE_IGNORE)
            return response;
    }

    return RESPONSE_ERROR;
}