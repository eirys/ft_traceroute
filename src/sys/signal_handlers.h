#ifndef SIGNAL_HANDLERS_H
# define SIGNAL_HANDLERS_H

#include "typedefs.h"

void        stop(__attribute__((unused)) int signal);
FT_RESULT   set_signals(void);
void        reset_signals(void);

#endif /* SIGNAL_HANDLERS_H */