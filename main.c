#include <stdlib.h>
#include <stdio.h>

#include "delim.h"

int main(int argc, char ** argv)
{
    delim_stats_t paren_stats;
    delim_stats_t scope_stats;
    delim_stats_t cmmnt_stats;
    delim_stats_t sltrl_stats;
    delim_stats_t cltrl_stats;
    delim_stats_t brckt_stats;
    int paren_ret;
    int scope_ret;
    int cmmnt_ret;
    int sltrl_ret;
    int cltrl_ret;
    int brckt_ret;
    int c;
    int exit_status = EXIT_SUCCESS;
    size_t line   = 1;
    size_t column = 0;

    if (!(paren_stats = delim_stats_new("(", ")", DS_NESTING)))
      {
        exit_status = EXIT_FAILURE;
        goto C0;
      }

    if (!(scope_stats = delim_stats_new("{", "}", DS_NESTING)))
      {
        exit_status = EXIT_FAILURE;
        goto C1;
      }

    if (!(cmmnt_stats = delim_stats_new("/*", "*/", 0)))
      {
        exit_status = EXIT_FAILURE;
        goto C2;
      }

    if (!(sltrl_stats = delim_stats_new("\"", "\"", DS_ESCAPE)))
      {
        exit_status = EXIT_FAILURE;
        goto C3;
      }

    if (!(cltrl_stats = delim_stats_new("\'", "\'", DS_ESCAPE)))
      {
        exit_status = EXIT_FAILURE;
        goto C4;
      }

    if (!(brckt_stats = delim_stats_new("[", "]", DS_NESTING)))
      {
        exit_status = EXIT_FAILURE;
        goto C5;
      }

    while ((c = getchar()) != EOF)
      {
        if (c == '\n')
          {
            ++line;
            column = 0;
          }
        else
            ++column;

        paren_ret = delim_stats_update(paren_stats, c);
        scope_ret = delim_stats_update(scope_stats, c);
        cmmnt_ret = delim_stats_update(cmmnt_stats, c);
        sltrl_ret = delim_stats_update(sltrl_stats, c);
        cltrl_ret = delim_stats_update(cltrl_stats, c);
        brckt_ret = delim_stats_update(brckt_stats, c);

        if (cmmnt_ret & DS_OPENED)
          {
            delim_stats_shadow(paren_stats);
            delim_stats_shadow(scope_stats);
            delim_stats_shadow(sltrl_stats);
            delim_stats_shadow(cltrl_stats);
            delim_stats_shadow(brckt_stats);
          }
        if (cmmnt_ret & DS_CLOSED)
          {
            delim_stats_unshadow(paren_stats);
            delim_stats_unshadow(scope_stats);
            delim_stats_unshadow(sltrl_stats);
            delim_stats_unshadow(cltrl_stats);
            delim_stats_unshadow(brckt_stats);
          }
        if (sltrl_ret & DS_OPENED)
          {
            delim_stats_shadow(paren_stats);
            delim_stats_shadow(scope_stats);
            delim_stats_shadow(cmmnt_stats);
            delim_stats_shadow(cltrl_stats);
            delim_stats_shadow(brckt_stats);
          }
        if (sltrl_ret & DS_CLOSED)
          {
            delim_stats_unshadow(paren_stats);
            delim_stats_unshadow(scope_stats);
            delim_stats_unshadow(cmmnt_stats);
            delim_stats_unshadow(cltrl_stats);
            delim_stats_unshadow(brckt_stats);
          }
        if (cltrl_ret & DS_OPENED)
          {
            delim_stats_shadow(paren_stats);
            delim_stats_shadow(scope_stats);
            delim_stats_shadow(cmmnt_stats);
            delim_stats_shadow(sltrl_stats);
            delim_stats_shadow(brckt_stats);
          }
        if (cltrl_ret & DS_CLOSED)
          {
            delim_stats_unshadow(paren_stats);
            delim_stats_unshadow(scope_stats);
            delim_stats_unshadow(cmmnt_stats);
            delim_stats_unshadow(sltrl_stats);
            delim_stats_unshadow(brckt_stats);
          }
        if (scope_ret & DS_CLOSED)
          {
            if (paren_ret & DS_INSIDE)
              {
                fprintf(stderr, "Expected matching ')' before end of scope: " \
                    "line %lu, column %lu\n", delim_stats_line(paren_stats),
                    delim_stats_column(paren_stats));
                exit_status = EXIT_FAILURE;
                goto C6;
              }
            if (brckt_ret & DS_INSIDE)
              {
                fprintf(stderr, "Expected matching ']' before end of scope: " \
                    "line %lu, column %lu\n", delim_stats_line(brckt_stats),
                    delim_stats_column(brckt_stats));
                exit_status = EXIT_FAILURE;
                goto C6;
              }
          }
        if (c == ';' && !(DS_INSIDE & (cmmnt_ret | sltrl_ret | cltrl_ret)))
          {
            if (paren_ret & DS_INSIDE)
              {
                fprintf(stderr, "Expected matching ')' before end of statement: " \
                    "line %lu, column %lu\n", delim_stats_line(paren_stats),
                    delim_stats_column(paren_stats));
                exit_status = EXIT_FAILURE;
                goto C6;
              }
            if (brckt_ret & DS_INSIDE)
              {
                fprintf(stderr, "Expected matching ']' before end of statement: " \
                    "line %lu, column %lu\n", delim_stats_line(brckt_stats),
                    delim_stats_column(brckt_stats));
                exit_status = EXIT_FAILURE;
                goto C6;
              }
          }
        if (c == '\\' && !(DS_INSIDE & (cmmnt_ret | sltrl_ret | cltrl_ret)))
          {
            fprintf(stderr, "Unrecognized operator '\\': line %lu, column %lu\n",
                line, column);
            exit_status = EXIT_FAILURE;
            goto C6;
          }
        if (paren_ret & DS_MISMATCH)
          {
            fprintf(stderr, "Unmatched parenthesis: line %lu, column %lu\n",
                delim_stats_line(paren_stats), delim_stats_column(paren_stats));
            exit_status = EXIT_FAILURE;
            goto C6;
          }
        if (scope_ret & DS_MISMATCH)
          {
            fprintf(stderr, "Unmatched brace: line %lu, column %lu\n",
                delim_stats_line(scope_stats), delim_stats_column(scope_stats));
            exit_status = EXIT_FAILURE;
            goto C6;
          }
        if (brckt_ret & DS_MISMATCH)
          {
            fprintf(stderr, "Unmatched brace: line %lu, column %lu\n",
                delim_stats_line(brckt_stats), delim_stats_column(brckt_stats));
            exit_status = EXIT_FAILURE;
            goto C6;
          }
      }

    if (paren_ret & DS_INSIDE)
      {
        fprintf(stderr, "Unterminated parenthesis: line %lu, column %lu\n",
            delim_stats_line(paren_stats), delim_stats_column(paren_stats));
        exit_status = EXIT_FAILURE;
        goto C6;
      }
    if (scope_ret & DS_INSIDE)
      {
        fprintf(stderr, "Unterminated brace: line %lu, column %lu\n",
            delim_stats_line(scope_stats), delim_stats_column(scope_stats));
        exit_status = EXIT_FAILURE;
        goto C6;
      }
    if (brckt_ret & DS_INSIDE)
      {
        fprintf(stderr, "Unterminated bracket: line %lu, column %lu\n",
            delim_stats_line(brckt_stats), delim_stats_column(brckt_stats));
        exit_status = EXIT_FAILURE;
        goto C6;
      }
    if (cmmnt_ret & DS_INSIDE)
      {
        fprintf(stderr, "Unterminated comment: line %lu, column %lu\n",
            delim_stats_line(cmmnt_stats), delim_stats_column(cmmnt_stats));
        exit_status = EXIT_FAILURE;
        goto C6;
      }
    if (sltrl_ret & DS_INSIDE)
      {
        fprintf(stderr, "Unterminated string literal: line %lu, column %lu\n",
            delim_stats_line(sltrl_stats), delim_stats_column(sltrl_stats));
        exit_status = EXIT_FAILURE;
        goto C6;
      }
    if (cltrl_ret & DS_INSIDE)
      {
        fprintf(stderr, "Unterminated character literal: line %lu, column %lu\n",
            delim_stats_line(cltrl_stats), delim_stats_column(cltrl_stats));
        exit_status = EXIT_FAILURE;
        goto C6;
      }

C6: delim_stats_free(brckt_stats);
C5: delim_stats_free(cltrl_stats);
C4: delim_stats_free(sltrl_stats);
C3: delim_stats_free(cmmnt_stats);
C2: delim_stats_free(scope_stats);
C1: delim_stats_free(paren_stats);
C0: return exit_status;
}
