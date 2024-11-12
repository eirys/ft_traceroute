#include "wrapper.h"

#include <string.h> /* strdup */
#include <stdlib.h> /* malloc, free */
#include <unistd.h> /* close */
#include <sys/socket.h> /* socket */
#include <stdio.h> /* fprintf */
#include <sys/time.h> /* gettimeofday */
#include <errno.h> /* errno */
#include "log.h"

static
void _error(const char* function_name, const char* message) {
    fprintf(stderr, "`%s' %s\n", function_name, message);
#ifdef __DEBUG
    fprintf(stderr, "(code %d: %s)\n", errno, strerror(errno));
#endif
}

/* -------------------------------------------------------------------------- */
/*                                 ALLOCATION                                 */
/* -------------------------------------------------------------------------- */

void* Malloc(const u32 size) {
    void* data = malloc(size);
    if (data == NULL) {
        _error("malloc", "failed to allocate memory");
        return NULL;
    }
    return data;
}

FT_RESULT Free(void* data) {
    if (data == NULL) {
        _error("free", "data is NULL");
        return FT_FAILURE;
    }
    free(data);
    return FT_SUCCESS;
}

/* -------------------------------------------------------------------------- */
/*                                   STRING                                   */
/* -------------------------------------------------------------------------- */

char* Strdup(const char* data) {
    if (data == NULL) {
        _error("strdup", "data is NULL");
        return NULL;
    }
    char* str = strdup(data);
    if (str == NULL) {
        _error("strdup", "failed to duplicate string");
        return NULL;
    }
    return str;
}

/* -------------------------------------------------------------------------- */
/*                                 RAW MEMORY                                 */
/* -------------------------------------------------------------------------- */

void Memset64(void* dest, u64 value, u32 count) {
    u64* ptr = (u64*)dest;
    for (u32 i = 0; i < count; ++i)
        ptr[i] = value;
}

/* -------------------------------------------------------------------------- */
/*                                   SOCKET                                   */
/* -------------------------------------------------------------------------- */

int Select(
    int nfds,
    fd_set* readfds,
    fd_set* writefds,
    fd_set* exceptfds,
    struct timeval* timeout
) {
    int fds = select(nfds, readfds, writefds, exceptfds, timeout);
    if (fds == -1 && errno != EINTR) {
        _error("select", "failed to listen on file descriptor");
    } else if (fds == -1 && errno == EINTR) {
        return INT32_MAX; /* Fictive value */
    }
    return fds;
}

int Socket(int domain, int type, int protocol) {
    int fd = socket(domain, type, protocol);
    if (fd == -1)
        _error("socket", "failed to create socket");
    return fd;
}

FT_RESULT Close(int fd) {
    if (close(fd) == -1) {
        _error("close", "failed to close socket");
        return FT_FAILURE;
    }
    return FT_SUCCESS;
}

FT_RESULT Sendto(int sockfd, const void* buf, u32 len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen) {
    const ssize_t bytes = sendto(sockfd, buf, len, flags, dest_addr, addrlen);
    if (bytes == -1) {
        _error("send", "failed to send data");
        return FT_FAILURE;
    } else if (bytes != len) {
        _error("send", "failed to send all data");
        return FT_FAILURE;
    }
    return FT_SUCCESS;
}

ssize_t Recvfrom(int sockfd, void* buf, u32 len, int flags, struct sockaddr* src_addr, socklen_t* addrlen) {
    ssize_t bytes;
    while (true) {
        bytes = recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
        if (bytes == -1) {
            if (errno == EINTR) {
                continue;
            }
            _error("recv", "failed to receive data");
            break;
        } else {
            break;
        }
    }
    return bytes;
}

int Getaddrinfo(const char* node, const char* service, const struct addrinfo* hints, struct addrinfo** res) {
    int result = getaddrinfo(node, service, hints, res);

    switch (result) {
        case 0:
            return FT_SUCCESS;

        case EAI_NONAME:
        case EAI_AGAIN:
        case EAI_FAIL:
            log_error("unknown host");
            break;

        default: /* System error */
            _error("getaddrinfo", gai_strerror(result));
            break;
    }
    return FT_FAILURE;
}

FT_RESULT   Getnameinfo(const struct sockaddr* sa, socklen_t salen, char* host, u32 hostlen, char* serv, u32 servlen, int flags) {
    if (getnameinfo(sa, salen, host, hostlen, serv, servlen, flags) != 0) {
        return FT_FAILURE;
    }
    return FT_SUCCESS;
}

/* -------------------------------------------------------------------------- */
/*                                    OTHER                                   */
/* -------------------------------------------------------------------------- */

FT_RESULT Gettimeofday(struct timeval* tv, void* tz) {
    if (gettimeofday(tv, tz) == -1) {
        _error("gettimeofday", "failed to send data");
        return FT_FAILURE;
    }
    return FT_SUCCESS;
}

FT_RESULT Sigaction(int signum, const struct sigaction* new_act, struct sigaction* old_act) {
    if (sigaction(signum, new_act, old_act) == -1) {
        _error("sigaction", "failed to set sigaction");
        return FT_FAILURE;
    }
    return FT_SUCCESS;
}