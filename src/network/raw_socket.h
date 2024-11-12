#ifndef SOCKET_H
# define SOCKET_H

#include <netinet/in.h> /* sockaddr_in */
#include "typedefs.h"

/* -------------------------------------------------------------------------- */
/*                                   STRUCT                                   */
/* -------------------------------------------------------------------------- */

typedef struct RawSocket {
    struct sockaddr_in* m_ipv4;
    int                 m_fd;           /* File descriptor */
    char*               m_ipv4_str;
} RawSocket;

/* -------------------------------------------------------------------------- */
/*                                   GLOBALS                                  */
/* -------------------------------------------------------------------------- */

extern RawSocket g_socket;

/* -------------------------------------------------------------------------- */
/*                                 PROTOTYPES                                 */
/* -------------------------------------------------------------------------- */

/**
 * @brief Create and open a raw socket, with extra information.
 */
FT_RESULT   create_raw_socket(const char* destination);

/**
 * @brief Close the raw socket and free allocated data.
 */
void        destroy_raw_socket(void);

#endif /* SOCKET_H */