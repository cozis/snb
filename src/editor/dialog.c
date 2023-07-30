#include <stdio.h>
#include <stdbool.h>
#include "dialog.h"

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

int choose_file(char *dst, size_t max)
{
    FILE *stream = popen("snb-dialog", "r");
    if (stream == NULL)
        return false;

    bool error = false;
    size_t copied = 0;
    for (bool done = false; !done; ) {
        size_t cap = max - copied;
        size_t num = fread(dst + copied, 1, cap, stream);
        if (num < cap) {
            if (ferror(stream))
                error = true;
            done = true;
        }
        copied += num;
        if (copied == max)
            done = true;
    }

    copied = MIN(max-1, copied);
    dst[copied] = '\0';

    fclose(stream);
    
    if (error)
        return -1;
    return (int) copied; // Overflow?
}