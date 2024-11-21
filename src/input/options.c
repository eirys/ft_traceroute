#include <getopt.h> /* getopt_long */
#include <string.h>

#include "options.h"
#include "network_io.h"
#include "log.h"

/* --------------------------------- GLOBALS -------------------------------- */

Arguments g_arguments = {
    .m_options.m_packet_size = UDP_PAYLOAD_SIZE,
    .m_options.m_queries = 3U,
    .m_options.m_simultaneous = 16U,
    .m_options.m_timeout = 5U,
    .m_options.m_src_port = 4242U,
    .m_options.m_dest_port = 2424U,
    .m_options.m_tos = 0x0,
    .m_options.m_max_hop = 30U,
    .m_options.m_start_hop = 1U,
    .m_options.m_numeric = false,
    .m_options.m_help = false,
    .m_destination = NULL
};

static int _current_option; /* Current option being processed */

/* -------------------------------------------------------------------------- */

/**
 * @brief Total options
 */
enum e_OptionIndex {
    OPTION_HELP = 0U,

    OPTION_COUNT
};

/**
 * @brief Short options (single character) value
 */
enum e_ShortOptionFlag {
    FLAG_LONG = 0,
    FLAG_END = -1
};

/**
 * @brief Long options (multi-character)
 */
enum e_LongOptionIndex {
    LONG_OPTION_HELP = 0,

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

static
FT_RESULT _set_flag(void* flag, FT_RESULT (*process_value)(char*, void*)) {
    if (flag == NULL) {
        log_debug("_set_flag", "improper address for flag");
        return FT_FAILURE;

    } else if (optarg == NULL) { /* optarg detailed above */
        log_error("bad value for `%c'", _current_option);
        return FT_FAILURE;

    } else if (process_value(optarg, flag) == FT_FAILURE) {
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
    };

    struct option option_descriptors[OPTION_COUNT + 1] = {
        /* Short options */
        /* Long options */
        { "help",       no_argument,        &long_option[LONG_OPTION_HELP],     1 },
        { 0, 0, 0, 0 }
    };

    const char* short_options = "";

    while (true) {
        _current_option = getopt_long(arg_count, arg_values, short_options, option_descriptors, NULL);

        switch (_current_option) {
            /* Long options */
            case FLAG_LONG:
                if (long_option[LONG_OPTION_HELP]) { _enable_flag(&options->m_help); return FT_SUCCESS; }
                /* Reset long option interception */
                for (u32 i = 0; i < LONG_OPTION_COUNT; ++i) { long_option[i] = 0; }
                break;

            /* Short options */
            case FLAG_END:          return FT_SUCCESS;

            default:                return FT_FAILURE;
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

    if (optind + 1 < arg_count) {
        log_error("too many arguments (use flag `--help` for indications)");
    //     g_arguments.m_options.m_packet_size = (u32)atoi(arg_values[optind + 1]); // TODO
    // } else if (optind + 2 < arg_count) {
        // // log_error("too many arguments (use flag `--help` for indications)");
        return FT_FAILURE;
    } else if (g_arguments.m_destination == NULL) {
        log_error("no destination provided (use flag `--help` for indications)");
        return FT_FAILURE;
    }

    log_debug("retrieve_arguments", "destination: `%s'", g_arguments.m_destination);

    return FT_SUCCESS;
}