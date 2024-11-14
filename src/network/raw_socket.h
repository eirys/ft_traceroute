#ifndef SOCKET_H
# define SOCKET_H

#include <netinet/in.h> /* sockaddr_in */
#include "typedefs.h"

/* -------------------------------------------------------------------------- */
/*                                   STRUCT                                   */
/* -------------------------------------------------------------------------- */

enum e_SocketType {
    Raw,
    Datagram
};

typedef struct s_SocketInfo {
    struct sockaddr_in* m_ipv4;
    // char*               m_ipv4_str;
    int                 m_fd;           /* File descriptor */
    enum e_SocketType   m_type;
} SocketInfo;

/* -------------------------------------------------------------------------- */
/*                                   GLOBALS                                  */
/* -------------------------------------------------------------------------- */

extern SocketInfo g_send_socket;
extern SocketInfo g_recv_socket;

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