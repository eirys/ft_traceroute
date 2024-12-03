#include <string.h> /* memset */

#include "network_io.h"
#include "typedefs.h"
#include "options.h"
#include "wrapper.h"
#include "raw_socket.h"

/* --------------------------------- GLOBALS -------------------------------- */

u32         g_sequence = 0U;
u8*         g_outpacket = NULL;
PacketInfo  g_outpacket_info;

/* -------------------------------------------------------------------------- */

static
void _set_ttl(u8 ttl) {
    setsockopt(g_udp_socket.m_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(u8));
}

static
void _set_payload(u8* payload) {
    for (u32 i = 0; i < g_arguments.m_packet_size; ++i)
        payload[i] = i; /* Random pattern */
}

/* -------------------------------------------------------------------------- */

FT_RESULT allocate_buffer(void) {
    g_outpacket = Malloc(g_arguments.m_packet_size);
    if (g_outpacket == NULL)
        return FT_FAILURE;
    return FT_SUCCESS;
}

void deallocate_buffer(void) {
    Free(g_outpacket);
    g_outpacket = NULL;
}

FT_RESULT send_request(const u8 ttl) {
    memset(&g_outpacket_info.m_addr.sin_zero, 0, sizeof(g_outpacket_info.m_addr.sin_zero));
    g_outpacket_info.m_addr.sin_addr = g_udp_socket.m_ipv4;
    g_outpacket_info.m_addr.sin_family = AF_INET;
    g_outpacket_info.m_addr.sin_port = htons(g_arguments.m_options.m_dest_port + g_sequence);

    if (Gettimeofday(&g_outpacket_info.m_timestamp, NULL) == FT_FAILURE) {
        return FT_FAILURE;
    }

    u8* payload = (u8*)g_outpacket;

    _set_ttl(ttl);
    _set_payload(payload);

    return Sendto(
        g_udp_socket.m_fd,
        g_outpacket,
        g_arguments.m_packet_size,
        0x0, /* No flags */
        (struct sockaddr*)&g_outpacket_info.m_addr,
        sizeof(struct sockaddr_in));
}