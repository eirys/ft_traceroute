#include "network_io.h"

#include <arpa/inet.h> /* inet_ntop */
#include <sys/param.h> /* MIN MAX */
#include <stdio.h> /* printf */

#include "options.h"
#include "stats.h"
#include "raw_socket.h"
#include "wrapper.h"
#include "stats.h"
#include "log.h"

/* --------------------------------- STATIC --------------------------------- */

static fd_set           _listen_fds;
static struct timeval   _timeout;

/* -------------------------------------------------------------------------- */

#define _LOG 1

#include <stdarg.h> /* va_list */
static
void display(const char* format, ...) {
#if _LOG
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
#endif
}

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
FT_RESULT _filter_icmpv4(u8* raw, Packet* packet, PacketInfo* packet_info) {
    packet->m_ip_header = (struct iphdr*)raw;

    if (packet_info->m_addr.sin_family != AF_INET) {
        log_debug("_filter_icmpv4", "not ipv4");
        return FT_FAILURE;
    }

    /* Not ICMPv4 */
    if (packet->m_ip_header->protocol != IPPROTO_ICMP) {
        log_debug("_filter_icmpv4", "not an ICMP message");
        return FT_FAILURE;
    }

    u32 ip_size = packet->m_ip_header->ihl * sizeof(u32);
    packet_info->m_ip_size = ip_size;
    packet_info->m_icmp_size = packet_info->m_size - ip_size;

    /* Incomplete packet_info */
    if (packet_info->m_icmp_size - ip_size < ICMP_HEADER_SIZE) {
        log_debug("_filter_icmpv4", "packet malformed");
        return FT_FAILURE;
    }

    packet->m_icmp.m_header = (struct icmphdr*)(raw + ip_size);
    packet->m_icmp.m_udp_ip = (struct iphdr*)(packet->m_icmp.m_header + ICMP_HEADER_SIZE);

    /* Content is not UDP */
    if (packet->m_icmp.m_udp_ip->protocol != IPPROTO_UDP) {
        log_debug("_filter_icmpv4", "not an UDP message");
        return FT_FAILURE;
    }

    ip_size = packet->m_icmp.m_udp_ip->ihl * sizeof(u32);
    packet->m_icmp.m_udp_data = (u8*)(packet->m_icmp.m_udp_ip + ip_size);

    /* Check correspondance */
    const struct udphdr* udp_copy = (struct udphdr*)(packet->m_icmp.m_udp_data);

    if (ntohs(udp_copy->uh_sport) != g_arguments.m_options.m_src_port ||
        ntohs(udp_copy->uh_dport) != g_arguments.m_options.m_dest_port) {
        return FT_FAILURE;
    }

    log_info("ici");

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

static
void _translate_source(const Packet* packet, const PacketInfo* packet_info) {
    bool is_numeric = g_arguments.m_options.m_numeric;

    char src_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &packet_info->m_addr.sin_addr, src_ip, INET_ADDRSTRLEN);

    if (is_numeric) {
        display("%s  ", src_ip);
    } else {
        char src_host[NI_MAXHOST];

        if (Getnameinfo((struct sockaddr*)&packet_info->m_addr, sizeof(struct sockaddr_in), src_host, NI_MAXHOST, NULL, 0, NI_NAMEREQD) == FT_SUCCESS) {
            display("%s (%s)  ", src_host, src_ip);
        } else {
            display("%s (%s)  ", src_ip, src_ip);
        }
    }

    /* Update source */
    g_raw_socket.m_ipv4.s_addr = packet_info->m_addr.sin_addr.s_addr;
}

static
enum e_Response _process_message(const Packet* packet, const PacketInfo* packet_info) {
    const double rtt = _compute_rtt(&packet_info->m_timestamp, &g_outpacket_info.m_timestamp);
    display("%.3f ms  ", rtt);

    if (packet->m_icmp.m_header->type == ICMP_DEST_UNREACH) {
        // log_debug("_process_message", "Destination reached");
        return RESPONSE_SUCCESS;
    } else if (packet->m_icmp.m_header->type == ICMP_TIME_EXCEEDED && packet->m_icmp.m_header->code == ICMP_TIMXCEED_INTRANS) {
        /* New address */
        if (g_raw_socket.m_ipv4.s_addr != packet_info->m_addr.sin_addr.s_addr) {
            _translate_source(packet, packet_info);
        }

        return RESPONSE_ONGOING;
    }
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
    // union u_InPacket buffer;
    u8 buffer[RECV_BUF_SIZE];

    FD_ZERO(&_listen_fds);
    FD_SET(g_raw_socket.m_fd, &_listen_fds);
    _reset_timeout();

    while (true) {
        int fds = Select(g_raw_socket.m_fd + 1, &_listen_fds, NULL, NULL, &_timeout);
        if (fds == -1) {
            return RESPONSE_ERROR;
        } else if (fds == 0) { /* Timeout */
            display("* ");
            return RESPONSE_TIMEOUT;
        }

        PacketInfo  packet_info;
        Packet      packet;

        if (_receive_packet(buffer, &packet_info) == FT_FAILURE)
            return RESPONSE_ERROR;

        if (_filter_icmpv4(buffer, &packet, &packet_info) == FT_FAILURE)
            continue;

        enum e_Response response = _process_message(&packet, &packet_info);

        if (response != RESPONSE_IGNORE)
            return response;
    }

    return RESPONSE_ERROR;
}