#ifndef STATS_H
# define STATS_H

#include "typedefs.h"

/* -------------------------------------------------------------------------- */
/*                                   STRUCTS                                  */
/* -------------------------------------------------------------------------- */

typedef struct s_Stats {
    u32 m_ttl;
    u32 m_packet_sent;
} Stats;

/* -------------------------------------------------------------------------- */
/*                                   GLOBALS                                  */
/* -------------------------------------------------------------------------- */

extern Stats g_stats;

#endif /* STATS_H */