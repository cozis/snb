#include <stdbool.h>
#include "drink.h"
#include "basic.h"

int drinkFromStream(FILE *stream, char *dst, size_t max)
{
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

    if (error)
        return -1;
    return (int) copied;
}

int drinkFromProgram(const char *cmd, char *dst, size_t max)
{
    FILE *stream = popen(cmd, "r");
    if (stream == NULL)
        return -1;

    int res = drinkFromStream(stream, dst, max);

    if (pclose(stream) != 0)
        return -1;
    return res;
}
