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

#define ICMP_COPY_SIZE      64 /* Copy 64 bytes of the original packet */
#define UDP_PAYLOAD_SIZE    (40 - UDP_HEADER_SIZE)

#define RECV_BUF_SIZE       32768 /* Large to handle replies overflows */

/* -------------------------------------------------------------------------- */
/*                                   STRUCTS                                  */
/* -------------------------------------------------------------------------- */

/**
 * @brief Full packet structure.
 */
typedef struct s_Packet {
    struct iphdr*           m_ip_header;
    union {
        struct {
            struct icmphdr* m_header;
            #define m_icmp_unused  m_header.un.gateway
            struct iphdr*   m_udp_ip;
            void*           m_udp_data;//[ICMP_COPY_SIZE];
        } __icmp;
        struct {
            struct udphdr*  m_header;
            u8*             m_payload;
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
        u32 __icmp_size;    /* Size of the ICMP content */
        u32 __udp_size;     /* Size of the UDP content */
    } __size;
    #define m_icmp_size         __size.__icmp_size
    #define m_udp_size          __size.__udp_size

    struct timeval          m_timestamp;    /* Timestamp of the packet */
    struct sockaddr_in      m_addr;         /* Associated address */
} PacketInfo;

/* -------------------------------------------------------------------------- */

enum e_Response {
    RESPONSE_ONGOING,   /* Received intermediate router response */
    RESPONSE_TIMEOUT,   /* Timeout */
    RESPONSE_IGNORE,    /* Received irrelevant message */
    RESPONSE_SUCCESS,   /* Reached destination */
    RESPONSE_ERROR      /* System error */
};

/* -------------------------------------------------------------------------- */
/*                                   GLOBALS                                  */
/* -------------------------------------------------------------------------- */

extern pid_t                g_pid;
extern u8*                  g_outpacket;
extern PacketInfo           g_outpacket_info;

/* -------------------------------------------------------------------------- */
/*                                 PROTOTYPES                                 */
/* -------------------------------------------------------------------------- */

/**
 * @brief Send an UDP packet with a specific TTL.
 */
FT_RESULT       send_request(const u8 ttl);

/**
 * @brief Wait for an ICMP response.
 */
enum e_Response wait_responses(void);

/**
 * @brief Allocate a buffer for the sending packet.
 */
FT_RESULT       allocate_buffer(void);

/**
 * @brief Deallocate the sending buffer.
 */
void            deallocate_buffer(void);

#endif /* NETWORK_IO_H */