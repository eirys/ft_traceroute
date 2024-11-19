#include <string.h> /* bzero */

#include "network_io.h"
#include "typedefs.h"
#include "options.h"
#include "wrapper.h"
#include "raw_socket.h"

// u8 g_outpacket[SEND_BUFFER_SIZE];

/* --------------------------------- GLOBALS -------------------------------- */

union u_OutPacket   g_outpacket;
PacketInfo          g_outpacket_info;

/* -------------------------------------------------------------------------- */

/**
 * @brief Implementation is from Mike Muuss (`ping` public domain version).
 */
static
u16 _compute_checksum(const u16* addr, u16 data_len) {
    i32         bytes_to_process = (i32)data_len;

    /* Iterate over addr, accumulates 2 bytes at a time */
    const u16*  it = addr;
    u32         accumulator = 0U;

    while (bytes_to_process > 1) {
        accumulator += *it++;
        bytes_to_process -= 2;
    }

    /* Process eventual remaining byte */
    if (bytes_to_process == 1) {
        accumulator += *(u8*)it;
    }

    /* Add carry from the top 16 bits */
    accumulator = (accumulator >> 16) + (accumulator & 0xFFff);

    /* Handle potential extra carry */
    accumulator += (accumulator >> 16);

    /* Return one's complement */
    return ~(accumulator);
}

static
void _set_ip_header(struct iphdr* ip_header, struct sockaddr_in* destination, u8 ttl) {
    const u32 packet_size = IP_HEADER_SIZE + UDP_HEADER_SIZE + g_arguments.m_options.m_packet_size;

    ip_header->version  = 4U;                           /* IPv4 */
    ip_header->ihl      = IP_HEADER_SIZE / sizeof(i32); /* 20 bytes */
    ip_header->tos      = g_arguments.m_options.m_tos;  /* Type of Service */
    ip_header->tot_len  = htons(packet_size);           /* Total Length */
    ip_header->id       = htons(42U);                   /* Identification */
    ip_header->frag_off = htons(0U);                    /* Fragment Offset */
    ip_header->ttl      = ttl;                          /* Time to Live */
    ip_header->protocol = IPPROTO_UDP;                  /* Protocol */
    ip_header->saddr    = INADDR_ANY;                   /* Source Address */
    ip_header->daddr    = destination->sin_addr.s_addr; /* Destination Address */
    ip_header->check    = 0U;

    ip_header->check = _compute_checksum((u16*)ip_header, IP_HEADER_SIZE);
}

static
void _set_udp_header(struct udphdr* udp_header, struct sockaddr_in* destination, u32 udp_size) {
    udp_header->source  = htons(g_pid);                 /* Source Port: our PxID */
    udp_header->dest    = htons(destination->sin_port); /* Destination Port */
    udp_header->len     = htons(udp_size);              /* UDP header + payload size */
    udp_header->check   = 0U;

    /* Compute checksum using pseudo header */
    struct s_UdpPseudoHeader {
        u32     source_address;
        u32     dest_address;
        u8      padding;
        u8      protocol;
        u16     udp_length;
    } __attribute__((packed)) pseudo_header;

    pseudo_header.source_address = INADDR_ANY;
    pseudo_header.dest_address = destination->sin_addr.s_addr;
    pseudo_header.padding = 0U;
    pseudo_header.protocol = IPPROTO_UDP;
    pseudo_header.udp_length = htons(udp_size);

    udp_header->check = _compute_checksum((u16*)&pseudo_header, sizeof(struct s_UdpPseudoHeader));
}

static
void _set_payload(u8* payload) {
    for (u32 i = 0; i < g_arguments.m_options.m_packet_size; ++i)
        payload[i] = 0x21; /* Random pattern */
}

/* -------------------------------------------------------------------------- */

FT_RESULT send_request(const u8 ttl) {
    static u32  seq = 0U;

    struct iphdr*   ip = &g_outpacket.m_packet.m_ip_header;
    struct udphdr*  udp = &g_outpacket.m_packet.m_udp.m_header;
    u8*             payload = g_outpacket.m_packet.m_udp.m_payload;

    memset(&g_outpacket_info.m_addr.sin_zero, 0, sizeof(g_outpacket_info.m_addr.sin_zero));
    g_outpacket_info.m_addr.sin_addr = g_udp_socket.m_ipv4;
    g_outpacket_info.m_addr.sin_family = AF_INET;
    g_outpacket_info.m_addr.sin_port = g_arguments.m_options.m_dest_port + ++seq;
    g_outpacket_info.m_udp_size = UDP_HEADER_SIZE + g_arguments.m_options.m_packet_size;
    g_outpacket_info.m_size = IP_HEADER_SIZE + g_outpacket_info.m_udp_size;
    g_outpacket_info.m_ip_size = IP_HEADER_SIZE;

    _set_ip_header(ip, &g_outpacket_info.m_addr, ttl);
    _set_udp_header(udp, &g_outpacket_info.m_addr, g_outpacket_info.m_udp_size);
    _set_payload(payload);

    return Sendto(
        g_udp_socket.m_fd,
        g_outpacket.m_raw,
        g_outpacket_info.m_size,
        0x0, /* No flags */
        (struct sockaddr*)&g_outpacket_info.m_addr,
        sizeof(struct sockaddr_in));
}