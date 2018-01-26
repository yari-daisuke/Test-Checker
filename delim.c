#include "delim.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dfa.h"
#include "stack.h"

struct delim_stats_s
{
    size_t ** ld_dfa;
    size_t    ld_state;
    size_t    ld_length;
    size_t ** rd_dfa;
    size_t    rd_state;
    size_t    rd_length;
    size_t    line;
    size_t    col;
    stack_t   ppos;
    int       balance;
    int       flags;
};

struct pos_s
{
    size_t line;
    size_t column;
};

struct pos_s * pos_new(size_t line, size_t column)
{
    struct pos_s * pos = malloc(sizeof(*pos));

    assert(pos);

    pos->line   = line;
    pos->column = column;

    return pos;
}

static void dfa_free(size_t ** dfa, size_t cols)
{
    if (dfa)
      {
        while (cols--)
            free(dfa[cols]);

        free(dfa);
      }
}

delim_stats_t delim_stats_new(char const * ld, char const * rd, int flags)
{
    delim_stats_t ds;
    
    assert(ld);
    assert(rd);
    
    ds = calloc(1, sizeof(*ds));

    if (!ds)
        goto C0;

    ds->ld_length = strlen(ld);
    ds->ld_dfa    = dfa_of_string(ld, ds->ld_length);

    if (!ds->ld_dfa)
        goto C1;

    ds->rd_length = strlen(rd);
    ds->rd_dfa    = dfa_of_string(rd, ds->rd_length);

    if (!ds->rd_dfa)
        goto C2;

    ds->ppos = stack_new();

    if (!ds->ppos)
        goto C3;

    ds->flags |= flags;
    ds->line   = 1;

    return ds;

C3: dfa_free(ds->rd_dfa, ds->rd_length);
C2: dfa_free(ds->ld_dfa, ds->ld_length);
C1: free(ds);
C0: return NULL;
}

void delim_stats_free(delim_stats_t ds)
{
    if (ds)
      {
        dfa_free(ds->ld_dfa, ds->ld_length);
        dfa_free(ds->rd_dfa, ds->rd_length);
        stack_foreach(ds->ppos, free);
        stack_free(ds->ppos);
        free(ds);
      }
}

int delim_stats_update(delim_stats_t ds, char _c)
{
    unsigned char c = _c;

    assert(ds);

    if (c == '\n')
      {
        ++ds->line;
        ds->col = 0;
      }
    else
        ++ds->col;

    if (ds->flags & DS_SHADOW)
        goto R0;

    if (ds->flags & DS_IGNORE)
      {
        ds->flags &= ~DS_IGNORE;
        goto R0;
      }

    if (ds->flags & DS_ESCAPE && c == '\\')
      {
        ds->flags   |= DS_IGNORE;
        ds->flags   &= ~(DS_OPENED | DS_CLOSED);
        ds->ld_state = 0;
        ds->rd_state = 0;
        goto R0;
      }

    if (ds->flags & DS_NESTING || !(ds->flags & DS_INSIDE))
      {
        ds->ld_state = ds->ld_dfa[ds->ld_state][c];

        if (ds->ld_state == ds->ld_length)
          {
            ds->flags |= DS_INSIDE | DS_OPENED;
            ++ds->balance;

            ds->ld_state = 0;
            ds->rd_state = 0;

            if (!stack_push(ds->ppos, pos_new(ds->line, ds->col)))
                ds->flags |= DS_ERROR;

            goto R0;
          }
      }

    ds->flags &= ~DS_OPENED;

    ds->rd_state = ds->rd_dfa[ds->rd_state][c];

    if (ds->rd_state == ds->rd_length)
      {
        --ds->balance;

        ds->flags |= DS_CLOSED;

        if (ds->balance < 0)
          {
            if (stack_push(ds->ppos, pos_new(ds->line, ds->col)))
                ds->flags |= DS_ERROR;

            ds->flags |= DS_MISMATCH;
          }
        else if (ds->balance >= 0)
          {
            if (ds->balance == 0)
                ds->flags &= ~DS_INSIDE;

            assert(!stack_empty(ds->ppos));

            free(stack_top(ds->ppos));
            stack_pop(ds->ppos);
          }

        ds->ld_state = 0;
        ds->rd_state = 0;

        goto R0;
      }

    ds->flags &= ~DS_CLOSED;

R0: return ds->flags;
}

size_t delim_stats_line(delim_stats_t ds)
{
    assert(ds);
    assert(!stack_empty(ds->ppos));

    return ((struct pos_s *) stack_top(ds->ppos))->line;
}

size_t delim_stats_column(delim_stats_t ds)
{
    assert(ds);
    assert(!stack_empty(ds->ppos));

    return ((struct pos_s *) stack_top(ds->ppos))->column;
}

void delim_stats_shadow(delim_stats_t ds)
{
    assert(ds);

    ds->flags   |= DS_SHADOW;
    ds->ld_state = 0;
    ds->rd_state = 0;
}

void delim_stats_unshadow(delim_stats_t ds)
{
    assert(ds);

    ds->flags &= ~DS_SHADOW;
}
