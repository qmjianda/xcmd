#ifndef XCMD_DEFAULT_KEYS_H
#define XCMD_DEFAULT_KEYS_H

#include "xcmd.h"

#define STR_UP     "\x1B\x5B\x41"
#define STR_DW     "\x1B\x5B\x42"
#define STR_RIGHT  "\x1B\x5B\x43"
#define STR_LEFT   "\x1B\x5B\x44"
#define STR_END    "\x1B\x5B\x46"
#define STR_HOME   "\x1B\x5B\x48"
#define STR_INSERT "\x1B\x5B\x32\x7E"
#define STR_DELETE "\x1B\x5B\x33\x7E"
#define STR_PGUP   "\x1B\x5B\x35\x7E"
#define STR_PGDW   "\x1B\x5B\x36\x7E"

void default_keys_init(xcmder_t *cmder);

#endif /* XCMD_DEFAULT_KEYS_H */