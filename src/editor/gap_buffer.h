#include <stddef.h>
#include <stdbool.h>

typedef struct GapBuffer GapBuffer;

typedef struct {
    GapBuffer *buff;
    bool crossed_gap;
    size_t cur;
    void *mem;
    char maybe[512];
} GapBufferIter;

typedef struct {
    const char *str;
    size_t len;
} GapBufferLine;

GapBuffer *GapBuffer_createUsingMemory(void *mem, size_t len, void (*free)(void*));
GapBuffer *GapBuffer_cloneUsingMemory(void *mem, size_t len, void (*free)(void*), const GapBuffer *src);
void       GapBuffer_whipeClean(GapBuffer *gap);
void       GapBuffer_destroy(GapBuffer *buff);
bool       GapBuffer_insertString(GapBuffer *buff, const char *str, size_t len);
bool       GapBuffer_insertRune(GapBuffer *buff, unsigned int code);
size_t     GapBuffer_moveRelative(GapBuffer *buff, int off);
size_t     GapBuffer_moveAbsolute(GapBuffer *buff, size_t num);
void       GapBuffer_moveAbsoluteRaw(GapBuffer *gap, size_t num);
size_t     GapBuffer_removeForwards(GapBuffer *buff, size_t num);
size_t     GapBuffer_removeBackwards(GapBuffer *buff, size_t num);
size_t     GapBuffer_getByteCount(GapBuffer *buff);
size_t     GapBuffer_rawCursorPosition(GapBuffer *buff);
void       GapBufferIter_init(GapBufferIter *iter, GapBuffer *buff);
void       GapBufferIter_free(GapBufferIter *iter);
bool       GapBufferIter_next(GapBufferIter *iter, GapBufferLine *line);

#ifndef GAPBUFFER_NOMALLOC
GapBuffer *GapBuffer_create(size_t capacity);
bool       GapBuffer_insertStringMaybeRelocate(GapBuffer **buff, const char *str, size_t len);
#endif

#ifndef GAPBUFFER_NOIO
bool GapBuffer_insertFile(GapBuffer *gap, const char *file);
#endif