#ifndef SOCKET_H
# define SOCKET_H

#include <netinet/in.h> /* sockaddr_in */
#include "typedefs.h"

/* -------------------------------------------------------------------------- */
/*                                   STRUCT                                   */
/* -------------------------------------------------------------------------- */

typedef struct s_SocketInfo {
    // void*                   m_data;         /* IPv4 if raw socket */
    struct in_addr          m_ipv4;         /* IPv4 address */
    int                     m_fd;           /* File descriptor */
} SocketInfo;

/* -------------------------------------------------------------------------- */
/*                                   GLOBALS                                  */
/* -------------------------------------------------------------------------- */

/**
 * @brief Sending socket.
 */
extern SocketInfo g_udp_socket;

/**
 * @brief Receiving socket.
 */
extern SocketInfo g_raw_socket;

/* -------------------------------------------------------------------------- */
/*                                 PROTOTYPES                                 */
/* -------------------------------------------------------------------------- */

/**
 * @brief Create and open a raw socket and udp socket, with extra information.
 */
FT_RESULT   create_sockets(const char* destination);

/**
 * @brief Close the raw socket and free allocated data.
 */
void        destroy_sockets(void);

#endif /* SOCKET_H */