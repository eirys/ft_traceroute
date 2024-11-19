#ifndef OPTIONS_H
# define OPTIONS_H

#include "typedefs.h"

/* -------------------------------------------------------------------------- */
/*                                   STRUCTS                                  */
/* -------------------------------------------------------------------------- */

typedef struct s_Options {
    u32     m_packet_size;  /* Packet size, set after destination */
    u32     m_queries;      /* -q --queries : Number of queries/probes sent */
    u32     m_simultaneous; /* -N --sim-queries : Number of simultaneous probes */
    u32     m_timeout;      /* -w --wait : Timeout for a probe */
    u16     m_src_port;     /* --sport : Source port */
    u16     m_dest_port;    /* -p --port : Destination port */
    u8      m_tos;          /* -t --tos : Type of Service */
    u8      m_max_hop;      /* -m --max-hops : Max TTL sent */
    u8      m_start_hop;    /* -f --first-hop : First TTL sent */
    bool    m_numeric;      /* -n --numeric : Numeric output only */
    bool    m_help;
} Options;

typedef struct s_Arguments {
    Options     m_options;
    char*       m_destination;
} Arguments;

/* -------------------------------------------------------------------------- */
/*                                   GLOBALS                                  */
/* -------------------------------------------------------------------------- */

extern Arguments g_arguments;

/* -------------------------------------------------------------------------- */
/*                                 PROTOTYPES                                 */
/* -------------------------------------------------------------------------- */

FT_RESULT retrieve_arguments(const int arg_count, char* const* arg_values);

#endif /* OPTIONS_H */
