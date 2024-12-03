#include <getopt.h> /* getopt_long */
#include <string.h> /* NULL */
#include <stdlib.h> /* atoi */
#include <sys/param.h> /* MAX */

#include "options.h"
#include "callbacks.h"
#include "network_io.h"
#include "log.h"

/* --------------------------------- GLOBALS -------------------------------- */

Arguments g_arguments = {
    .m_options = {
        .m_queries = 3U,
        .m_simultaneous = 16U,
        .m_timeout = 5.0f,
        .m_src_port = 0U,
        .m_dest_port = 0x8000 + 666U,
        .m_tos = 0x0,
        .m_max_hop = 30U,
        .m_start_hop = 1U,
        .m_numeric = false,
        .m_help = false,
    },
    .m_packet_size = UDP_PAYLOAD_SIZE,
    .m_destination = NULL
};

static int _current_option; /* Current option being processed */

/* -------------------------------------------------------------------------- */

/**
 * @brief Total options
 */
enum e_OptionIndex {
    OPTION_HELP = 0U,
    OPTION_Q,
    OPTION_Q_LONG,
    OPTION_N_CAP,
    OPTION_N_CAP_LONG,
    OPTION_W,
    OPTION_W_LONG,
    OPTION_SPORT,
    OPTION_P,
    OPTION_P_LONG,
    OPTION_T,
    OPTION_T_LONG,
    OPTION_M,
    OPTION_M_LONG,
    OPTION_F,
    OPTION_F_LONG,
    OPTION_N,
    OPTION_N_LONG,

    OPTION_COUNT
};

/**
 * @brief Short options (single character) value
 */
enum e_ShortOptionFlag {
    FLAG_Q = 'q',
    FLAG_N_CAP = 'N',
    FLAG_W = 'w',
    FLAG_P = 'p',
    FLAG_T = 't',
    FLAG_M = 'm',
    FLAG_F = 'f',
    FLAG_N = 'n',

    FLAG_LONG = 0,
    FLAG_END = -1
};

/**
 * @brief Long options (multi-character)
 */
enum e_LongOptionIndex {
    LONG_OPTION_HELP = 0,
    LONG_OPTION_QUERIES,
    LONG_OPTION_SIMULTANEOUS,
    LONG_OPTION_WAIT,
    LONG_OPTION_SPORT,
    LONG_OPTION_PORT,
    LONG_OPTION_TOS,
    LONG_OPTION_MAX_HOPS,
    LONG_OPTION_FIRST_HOP,
    LONG_OPTION_NUMERIC,

    LONG_OPTION_COUNT
};

/* -------------------------------------------------------------------------- */

static
void _enable_flag(bool* flag) {
    if (flag == NULL) {
        log_debug("_enable_flag", "flag is NULL");
        return;
    }

    *flag = true;
}

/**
 * @brief Set a flag with a value.
 * If the process value fails, the function returns FT_FAILURE.
 * An extra function can be passed to process the value further.
 */
static
FT_RESULT _set_flag(
    void* flag,
    FT_RESULT (*process_value)(const char*, void*, void*, const char*),
    FT_RESULT (*next)(void*, const char*),
    const char* err
) {
    if (flag == NULL) {
        log_debug("_set_flag", "improper address for flag");
        return FT_FAILURE;

    } else if (optarg == NULL) { /* optarg detailed above */
        log_error("bad value for `%c'", _current_option);
        return FT_FAILURE;

    } else if (process_value(optarg, flag, next, err) == FT_FAILURE) {
        return FT_FAILURE;
    }

    return FT_SUCCESS;
}

/* -------------------------------------------------------------------------- */

static
FT_RESULT _retrieve_options(const int arg_count, char* const* arg_values) {
    Options* options = &g_arguments.m_options;

    int long_option[LONG_OPTION_COUNT] = {
        [LONG_OPTION_HELP] = 0,
        [LONG_OPTION_QUERIES] = 0,
        [LONG_OPTION_SIMULTANEOUS] = 0,
        [LONG_OPTION_WAIT] = 0,
        [LONG_OPTION_SPORT] = 0,
        [LONG_OPTION_PORT] = 0,
        [LONG_OPTION_TOS] = 0,
        [LONG_OPTION_MAX_HOPS] = 0,
        [LONG_OPTION_FIRST_HOP] = 0,
        [LONG_OPTION_NUMERIC] = 0
    };

    struct option option_descriptors[OPTION_COUNT + 1] = {
        /* Short options */
        { "n",              no_argument,        NULL,                                   FLAG_N },
        { "q",              required_argument,  NULL,                                   FLAG_Q },
        { "N",              required_argument,  NULL,                                   FLAG_N_CAP },
        { "w",              required_argument,  NULL,                                   FLAG_W },
        { "p",              required_argument,  NULL,                                   FLAG_P },
        { "t",              required_argument,  NULL,                                   FLAG_T },
        { "m",              required_argument,  NULL,                                   FLAG_M },
        { "f",              required_argument,  NULL,                                   FLAG_F },
        /* Long options */
        { "help",           no_argument,        &long_option[LONG_OPTION_HELP],         1 },
        { "numeric",        no_argument,        &long_option[LONG_OPTION_NUMERIC],      1 },
        { "queries",        required_argument,  &long_option[LONG_OPTION_QUERIES],      1 },
        { "sim-queries",    required_argument,  &long_option[LONG_OPTION_SIMULTANEOUS], 1 },
        { "wait",           required_argument,  &long_option[LONG_OPTION_WAIT],         1 },
        { "sport",          required_argument,  &long_option[LONG_OPTION_SPORT],        1 },
        { "port",           required_argument,  &long_option[LONG_OPTION_PORT],         1 },
        { "tos",            required_argument,  &long_option[LONG_OPTION_TOS],          1 },
        { "max-hops",       required_argument,  &long_option[LONG_OPTION_MAX_HOPS],     1 },
        { "first-hop",      required_argument,  &long_option[LONG_OPTION_FIRST_HOP],    1 },
        { 0, 0, 0, 0 }
    };

    const char* short_options = "n" "q:N:w:p:t:m:f:";

    while (true) {
        _current_option = getopt_long(arg_count, arg_values, short_options, option_descriptors, NULL);

        switch (_current_option) {
            /* Long options */
            case FLAG_LONG:
                     if (long_option[LONG_OPTION_HELP]) { _enable_flag(&options->m_help); return FT_SUCCESS; }
                else if (long_option[LONG_OPTION_NUMERIC]) { _enable_flag(&options->m_numeric); }
                else if (long_option[LONG_OPTION_QUERIES]) { if (_set_flag((void*)&options->m_queries, &process_uint, &extra_over_zero, "can't send 0 probes") == FT_FAILURE) { return FT_FAILURE; } }
                else if (long_option[LONG_OPTION_SIMULTANEOUS]) { if (_set_flag((void*)&options->m_simultaneous, &process_uint, &extra_over_zero, "can't send 0 probes") == FT_FAILURE) { return FT_FAILURE; } }
                else if (long_option[LONG_OPTION_WAIT]) { if (_set_flag((void*)&options->m_timeout, &process_float, NULL, NULL) == FT_FAILURE) { return FT_FAILURE; } }
                else if (long_option[LONG_OPTION_SPORT]) { if (_set_flag((void*)&options->m_src_port, &process_ushort, &extra_default_sport, NULL) == FT_FAILURE) { return FT_FAILURE; } }
                else if (long_option[LONG_OPTION_PORT]) { if (_set_flag((void*)&options->m_dest_port, &process_ushort, &extra_over_zero, "can't set destination port to 0") == FT_FAILURE) { return FT_FAILURE; } }
                else if (long_option[LONG_OPTION_TOS]) { if (_set_flag((void*)&options->m_tos, &process_uchar, NULL, NULL) == FT_FAILURE) { return FT_FAILURE; } }
                else if (long_option[LONG_OPTION_MAX_HOPS]) { if (_set_flag((void*)&options->m_max_hop, &process_uchar, NULL, NULL) == FT_FAILURE) { return FT_FAILURE; } }
                else if (long_option[LONG_OPTION_FIRST_HOP]) { if (_set_flag((void*)&options->m_start_hop, &process_uchar, NULL, NULL) == FT_FAILURE) { return FT_FAILURE; } }

                /* Reset long option interception */
                for (u32 i = 0; i < LONG_OPTION_COUNT; ++i) { long_option[i] = 0; }
                break;

            /* Short options */
            case FLAG_N:        _enable_flag(&options->m_numeric); break;
            case FLAG_Q:        if (_set_flag((void*)&options->m_numeric, &process_uint, &extra_over_zero, "can't send 0 probes") == FT_FAILURE) { return FT_FAILURE; }; break;
            case FLAG_N_CAP:    if (_set_flag((void*)&options->m_simultaneous, &process_uint, &extra_over_zero, "can't send 0 probes") == FT_FAILURE) { return FT_FAILURE; }; break;
            case FLAG_W:        if (_set_flag((void*)&options->m_timeout, &process_float, NULL, NULL) == FT_FAILURE) { return FT_FAILURE; }; break;
            case FLAG_P:        if (_set_flag((void*)&options->m_dest_port, &process_ushort, &extra_over_zero, "can't set destination port to 0") == FT_FAILURE) { return FT_FAILURE; }; break;
            case FLAG_T:        if (_set_flag((void*)&options->m_tos, &process_uchar, NULL, NULL) == FT_FAILURE) { return FT_FAILURE; }; break;
            case FLAG_M:        if (_set_flag((void*)&options->m_max_hop, &process_uchar, NULL, NULL) == FT_FAILURE) { return FT_FAILURE; }; break;
            case FLAG_F:        if (_set_flag((void*)&options->m_start_hop, &process_uchar, NULL, NULL) == FT_FAILURE) { return FT_FAILURE; }; break;
            case FLAG_END:      return FT_SUCCESS;

            default:            return FT_FAILURE;
        }
    }
    return FT_SUCCESS;
}

/* -------------------------------------------------------------------------- */

FT_RESULT retrieve_arguments(const int arg_count, char* const* arg_values) {
    if (_retrieve_options(arg_count, arg_values) == FT_FAILURE) {
        return FT_FAILURE;
    }

    if (g_arguments.m_options.m_help == true) {
        return FT_SUCCESS;
    }

    g_arguments.m_destination = arg_values[optind];

    if (optind + 2 < arg_count) {
        log_error("too many arguments (use flag `--help` for indications)");
        return FT_FAILURE;
    } else if (optind + 1 < arg_count) {
        if (process_uint(arg_values[optind + 1], &g_arguments.m_packet_size, extra_default_len, NULL) == FT_FAILURE) {
            return FT_FAILURE;
        }
    } else if (g_arguments.m_destination == NULL) {
        log_error("no destination provided (use flag `--help` for indications)");
        return FT_FAILURE;
    }

    /* Extra checks */
    if (g_arguments.m_options.m_src_port == 0U) {
        g_arguments.m_options.m_src_port = g_pid;
    }

    if (g_arguments.m_options.m_start_hop > g_arguments.m_options.m_max_hop) {
        log_error("first hop greater than max hops");
        return FT_FAILURE;
    }

    return FT_SUCCESS;
}