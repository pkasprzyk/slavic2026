#include "chamber.h"
#include "reactor.h"

void chamber_init(void) { reactor_init(); }

void chamber_update(void) { reactor_update(); }

void chamber_deinit(void) { reactor_deinit(); }
