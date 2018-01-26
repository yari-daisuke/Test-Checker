#ifndef DELIM_INCUDED
#define DELIM_INCLUDED

#include <stddef.h>

enum
{
    DS_INSIDE   = 0x001,
    DS_MISMATCH = 0x002,
    DS_NESTING  = 0x004,
    DS_ESCAPE   = 0x008,
    DS_IGNORE   = 0x010,
    DS_ERROR    = 0x020,
    DS_SHADOW   = 0x040,
    DS_OPENED   = 0x080,
    DS_CLOSED   = 0x100
};

typedef struct delim_stats_s * delim_stats_t;


delim_stats_t delim_stats_new(char const * ld, char const * rd, int nests);

void          delim_stats_free(delim_stats_t ds);

int           delim_stats_update(delim_stats_t ds, char c);

size_t        delim_stats_line(delim_stats_t ds);

size_t        delim_stats_column(delim_stats_t ds);

void          delim_stats_shadow(delim_stats_t ds);

void          delim_stats_unshadow(delim_stats_t ds);

#endif /* DELIM_INCLUDED */
