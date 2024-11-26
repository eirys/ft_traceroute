#include <stdlib.h> /* exit */

#include "wrapper.h"
#include "raw_socket.h"
#include "log.h"
#include "network_io.h"

/* -------------------------------------------------------------------------- */

static struct sigaction _old_sigint;

/* -------------------------------------------------------------------------- */

void reset_signals(void) {
    if (Sigaction(SIGINT, &_old_sigint, NULL) == FT_FAILURE) {
        log_error("failed to reset sigint");
    }
}

/**
 * @brief Handler for SIGINT signal: stop the program gracefully.
 */
void stop(__attribute__((unused)) int signal) {
    reset_signals();
    destroy_sockets();
    deallocate_buffer();
    exit(EXIT_SUCCESS);
}

/**
 * @brief Enable signals to handle alarm (ping) and interrupt.
 */
FT_RESULT set_signals(void) {
    if (Sigaction(SIGINT, NULL, &_old_sigint) == FT_FAILURE) {
        log_error("failed to save old sigint");
        return FT_FAILURE;
    }

    struct sigaction    new_sigint;
    sigemptyset(&new_sigint.sa_mask);
    new_sigint.sa_flags = SA_RESTART;
    new_sigint.sa_handler = stop;
    if (Sigaction(SIGINT, &new_sigint, NULL) == FT_FAILURE) {
        log_error("failed to set new sigint");
        return FT_FAILURE;
    }

    return FT_SUCCESS;
}