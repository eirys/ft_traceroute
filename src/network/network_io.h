#ifndef NETWORK_IO_H
# define NETWORK_IO_H

#include <sys/types.h> /* pid_t */
#include <netinet/ip_icmp.h> /* struct icmphdr */
#include <netinet/ip.h> /* struct iphdr */
#include <netinet/in.h> /* struct sockaddr_in */
#include <sys/time.h> /* struct timeval */

#include "typedefs.h"

#define IP_HEADER_SIZE      20  /* No options */
#define ICMP_MSG_SIZE       64
#define ICMP_HEADER_SIZE    8

/* -------------------------------------------------------------------------- */
/*                                 PROTOTYPES                                 */
/* -------------------------------------------------------------------------- */

/**
 * @brief Send an ICMP echo request to the given destination.
 */
FT_RESULT   send_request();

/**
 * @brief Wait for an ICMP response.
 */
FT_RESULT   wait_responses();

#endif /* NETWORK_IO_H */