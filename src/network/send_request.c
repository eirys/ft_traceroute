#include "typedefs.h"

# define BUFFER_SIZE    1024

u8      g_send_buffer[BUFFER_SIZE];

/* -------------------------------------------------------------------------- */

FT_RESULT send_request() {
    struct iphdr*   ip = (struct iphdr*)g_send_buffer;
    struct icmphdr* icmp = (struct icmphdr*)(g_send_buffer + IP_HEADER_SIZE);
    u8*             payload = (u8*)(g_send_buffer + IP_HEADER_SIZE + ICMP_HEADER_SIZE);

    _set_ip_header(ip, g_send_buffer_size);
    if (_set_payload(payload, &g_arguments.m_options.m_pattern, g_arguments.m_options.m_size) == FT_FAILURE)
        return FT_FAILURE;
    _set_icmp_header(icmp, ICMP_HEADER_SIZE + g_arguments.m_options.m_size);

    return Sendto(
        g_socket.m_fd,
        g_send_buffer,
        g_send_buffer_size,
        0x0, /* No flags */
        (struct sockaddr*)g_socket.m_ipv4,
        sizeof(struct sockaddr_in));
}