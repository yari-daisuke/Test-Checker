#include "dfa.h"

#include <assert.h>
#include <stdlib.h>

#define ALPHABET_SIZE 256

size_t ** dfa_of_string(char const * _str, size_t len)
{
    unsigned char const * str = (unsigned char const *) _str;
    size_t i;
    size_t j;
    size_t sim = 0;
    size_t ** dfa;

    assert(str);
    assert(len > 0);

    dfa = malloc(len * sizeof(*dfa));

    if (!dfa)
        return NULL;
    
    for (i = 0; i < len; i++)
      {
        dfa[i] = malloc(ALPHABET_SIZE * sizeof(*dfa[i]));

        if (!dfa[i])
          {
            while (i--)
                free(dfa[i]);

            free(dfa);

            return NULL;
          }

        dfa[i][str[i]] = i + 1;
      }

    for (i = 0; i < ALPHABET_SIZE; i++)
        if (i != str[0])
            dfa[0][i] = 0;

    for (i = 1; i < len; i++)
      {
        for (j = 0; j < ALPHABET_SIZE; j++)
            if (j != str[i])
                dfa[i][j] = dfa[sim][j];

        sim = dfa[sim][str[i]];
      }

    return dfa;
}
