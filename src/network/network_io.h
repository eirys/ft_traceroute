#ifndef NETWORK_IO_H
# define NETWORK_IO_H

#include <sys/types.h> /* pid_t */
#include <netinet/ip.h> /* struct iphdr */
#include <netinet/ip_icmp.h> /* struct icmphdr */
#include <netinet/udp.h> /* struct udphdr */
#include <netinet/in.h> /* struct sockaddr_in */
#include <sys/time.h> /* struct timeval */

#include "typedefs.h"

#define IP_HEADER_SIZE      20  /* No options */
#define ICMP_HEADER_SIZE    8
#define UDP_HEADER_SIZE     8

#define ICMP_PAYLOAD_SIZE   (64 - ICMP_HEADER_SIZE)
#define UDP_PAYLOAD_SIZE    (40 - UDP_HEADER_SIZE)

#define SEND_BUFFER_SIZE    1024
#define RECV_BUF_SIZE       32768 /* Large to handle replies overflows */

/* -------------------------------------------------------------------------- */
/*                                   STRUCTS                                  */
/* -------------------------------------------------------------------------- */

/**
 * @brief Full packet structure.
 */
typedef struct s_Packet {
    struct iphdr            m_ip_header;
    union {
        struct {
            struct icmphdr  m_header;
            u8              m_payload[ICMP_PAYLOAD_SIZE];
        } __icmp;
        struct {
            struct udphdr   m_header;
            u8              m_payload[UDP_PAYLOAD_SIZE];
        } __udp;
    } __un;
    #define m_icmp  __un.__icmp
    #define m_udp   __un.__udp
} Packet;

/**
 * @brief Packet information.
 */
typedef struct s_PacketInfo {
    u32                     m_size;         /* Size of the packet */
    u32                     m_ip_size;      /* Size of the IP header */
    union {
        u32 __icmp_size;    /* Size of the ICMP header */
        u32 __udp_size;     /* Size of the UDP header */
    } __size;
    #define m_icmp_size         __size.__icmp_size
    #define m_udp_size          __size.__udp_size

    struct timeval          m_timestamp;    /* Timestamp of the packet */
    struct sockaddr_in      m_src_addr;     /* Source address */
} PacketInfo;

/* -------------------------------------------------------------------------- */

/**
 * @brief Buffer structure: either raw buffer or packet structure.
 */
union u_OutPacket {
    u8      m_raw[SEND_BUFFER_SIZE];
    Packet  m_packet;
};

union u_InPacket {
    u8      m_raw[RECV_BUF_SIZE];
    Packet  m_packet;
};

/* -------------------------------------------------------------------------- */
/*                                   GLOBALS                                  */
/* -------------------------------------------------------------------------- */

extern pid_t                g_pid;
extern union u_OutPacket    g_outpacket;

/* -------------------------------------------------------------------------- */
/*                                 PROTOTYPES                                 */
/* -------------------------------------------------------------------------- */

/**
 * @brief Send an ICMP echo request to the given destination.
 */
FT_RESULT   send_request(void);

/**
 * @brief Wait for an ICMP response.
 */
FT_RESULT   wait_responses(void);

#endif /* NETWORK_IO_H */