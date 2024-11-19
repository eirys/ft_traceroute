#include "network_io.h"
#include "typedefs.h"
#include "options.h"
#include "wrapper.h"
#include "raw_socket.h"

#define UDP_SIZE    (UDP_HEADER_SIZE + UDP_PAYLOAD_SIZE)

// u8 g_outpacket[SEND_BUFFER_SIZE];
union u_OutPacket g_outpacket;

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
void _set_ip_header(struct iphdr* ip_header, struct sockaddr_in* destination, u32 packet_size, u8 ttl) {
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
void _set_udp_header(struct udphdr* udp_header, struct sockaddr_in* destination) {
    udp_header->source  = htons(g_pid);                 /* Source Port: our PxID */
    udp_header->dest    = htons(destination->sin_port); /* Destination Port */
    udp_header->len     = htons(UDP_SIZE);              /* UDP header + payload size */
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
    pseudo_header.udp_length = htons(UDP_SIZE);

    udp_header->check = _compute_checksum((u16*)&pseudo_header, sizeof(struct s_UdpPseudoHeader));
}

static
void _set_payload(u8* payload) {
    for (u32 i = 0; i < UDP_PAYLOAD_SIZE; ++i)
        payload[i] = 0x21; /* Random pattern */
}

/* -------------------------------------------------------------------------- */

FT_RESULT send_request(void) {
    static u8   ttl = 1U;
    static u32  seq = 0U;

    struct sockaddr_in destination = {
        .sin_family = AF_INET,
        .sin_port = g_arguments.m_options.m_dest_port + ++seq,
        .sin_addr = g_udp_socket.m_ipv4
    };

    struct iphdr*   ip = &g_outpacket.m_packet.m_ip_header;
    struct udphdr*  udp = &g_outpacket.m_packet.m_udp.m_header;
    u8*             payload = g_outpacket.m_packet.m_udp.m_payload;

    _set_ip_header(ip, &destination, IP_HEADER_SIZE + UDP_HEADER_SIZE + UDP_PAYLOAD_SIZE, ttl++);
    _set_udp_header(udp, &destination);
    _set_payload(payload);

    return Sendto(
        g_udp_socket.m_fd,
        g_outpacket.m_raw,
        IP_HEADER_SIZE + UDP_SIZE,
        0x0, /* No flags */
        (struct sockaddr*)&destination,
        sizeof(struct sockaddr_in));
}