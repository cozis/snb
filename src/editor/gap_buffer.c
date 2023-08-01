#include <stdint.h>
#include <assert.h>
#include <string.h>
#include "gap_buffer.h"

#ifdef GAPBUFFER_DEBUG
#define PRIVATE
#else
#define PRIVATE static
#endif

#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

typedef struct {
    const char *data;
    size_t      size;
} String;

struct GapBuffer {
    void (*free)(void*);
    size_t gap_offset;
    size_t gap_length;
    size_t total;
    size_t column_target;
    size_t column_current;
    char   data[];
};

size_t GapBuffer_getColumn(GapBuffer *gap)
{
    return gap->column_current;
}

size_t GapBuffer_getTargetColumn(GapBuffer *gap)
{
    return gap->column_target;
}

size_t GapBuffer_rawCursorPosition(GapBuffer *buff)
{
    return buff->gap_offset;
}

size_t GapBuffer_getByteCount(GapBuffer *buff)
{
    return buff->total - buff->gap_length;
}

GapBuffer *GapBuffer_createUsingMemory(void *mem, size_t len, void (*free)(void*))
{
    if (mem == NULL || len < sizeof(GapBuffer)) {
        if (free) free(mem);
        return NULL;
    }
    
    size_t capacity = len - sizeof(GapBuffer);

    GapBuffer *buff = mem;
    buff->gap_offset = 0;
    buff->gap_length = capacity;
    buff->column_target = 0;
    buff->column_current = 0;
    buff->total = capacity;
    buff->free = free;
    return buff;
}

void GapBuffer_whipeClean(GapBuffer *gap)
{
    gap->gap_offset = 0;
    gap->gap_length = gap->total;
}

/* Symbol: GapBuffer_destroy
**   Delete an instanciated gap buffer. 
*/
void GapBuffer_destroy(GapBuffer *buff)
{
    if (buff->free)
        buff->free(buff);
}

/* Symbol: getStringBeforeGap
**   Returns a slice to the memory region before the gap
**   in the form of a (pointer, length) pair.
*/
PRIVATE String getStringBeforeGap(const GapBuffer *buff)
{
    return (String) {

        .data=buff->data, // The start of the buffer is also the
                          // start of the region before the gap.

        .size=buff->gap_offset, // The offset of the gap is the the length
                                // of the region that comes before it.
    };
}

/* Symbol: getStringAfterGap
**   Returns a slice to the memory region after the gap
**   in the form of a (pointer, length) pair.
*/
PRIVATE String getStringAfterGap(const GapBuffer *buff)
{
    // The first byte after the gap is the offset
    // of the text that comes after the gap and
    // the length of the region before the gap plus
    // the length of the gap.
    size_t first_byte_after_gap = buff->gap_offset + buff->gap_length;

    return (String) {
        .data = buff->data  + first_byte_after_gap,
        .size = buff->total - first_byte_after_gap, // The length of the region following the
                                                    // gap is the total number of bytes minus
                                                    // the offset of the first byte after the gap.
    };
}

// Returns true if and only if the [byte] is in the form 10xxxxxx
PRIVATE bool isSymbolAuxiliaryByte(uint8_t byte)
{
    //   Hex    Binary
    // +-----+----------+
    // | C0  | 11000000 |
    // +-----+----------+
    // | 80  | 10000000 |
    // +-----+----------+
    
    return (byte & 0xC0) == 0x80;
}

PRIVATE int getSymbolRune(const char *sym, size_t symlen, uint32_t *rune)
{
    if(symlen == 0)
        return 0;
    
    if(sym[0] & 0x80) {

        // May be UTF-8.
            
        if((unsigned char) sym[0] >= 0xF0) {

            // 4 bytes.
            // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

            if(symlen < 4)
                return -1;

            if (!isSymbolAuxiliaryByte(sym[1]) ||
                !isSymbolAuxiliaryByte(sym[2]) ||
                !isSymbolAuxiliaryByte(sym[3]))
                return -1;
                    
            uint32_t temp 
                = (((uint32_t) sym[0] & 0x07) << 18) 
                | (((uint32_t) sym[1] & 0x3f) << 12)
                | (((uint32_t) sym[2] & 0x3f) <<  6)
                | (((uint32_t) sym[3] & 0x3f));

            if(temp < 0x010000 || temp > 0x10ffff)
                return -1;

            *rune = temp;
            return 4;
        }
            
        if((unsigned char) sym[0] >= 0xE0) {

            // 3 bytes.
            // 1110xxxx 10xxxxxx 10xxxxxx

            if(symlen < 3)
                return -1;

            if (!isSymbolAuxiliaryByte(sym[1]) ||
                !isSymbolAuxiliaryByte(sym[2]))
                return -1;

            uint32_t temp
                = (((uint32_t) sym[0] & 0x0f) << 12)
                | (((uint32_t) sym[1] & 0x3f) <<  6)
                | (((uint32_t) sym[2] & 0x3f));
            
            if (temp < 0x0800 || temp > 0xffff)
                return -1;

            *rune = temp;
            return 3;
        }
            
        if((unsigned char) sym[0] >= 0xC0) {

            // 2 bytes.
            // 110xxxxx 10xxxxxx

            if(symlen < 2)
                return -1;

            if (!isSymbolAuxiliaryByte(sym[1]))
                return -1;

            *rune 
                = (((uint32_t) sym[0] & 0x1f) << 6)
                | (((uint32_t) sym[1] & 0x3f));

            if (*rune < 0x80 || *rune > 0x07ff)
                return -1;

            assert(*rune <= 0x10ffff);
            return 2;
        }
            
        return -1;
    }

    // It's ASCII
    // 0xxxxxxx

    *rune = (uint32_t) sym[0];
    return 1;
}

static size_t countSymbolsAfterLastNewline(String str, bool *have_newline)
{
    size_t cur = str.size;
    
    while (cur > 0 && str.data[cur-1] != '\n')
        cur--;

    *have_newline = (cur > 0);

    size_t col = 0;
    while (cur < str.size && str.data[cur] != '\n') {
        uint32_t unused;
        cur += getSymbolRune(str.data + cur, str.size - cur, &unused);
        col++;
    }

    return col;
}

PRIVATE bool insertBytesBeforeCursor(GapBuffer *buff, String str)
{
    if (buff->gap_length < str.size)
        return false;
    
    memcpy(buff->data + buff->gap_offset, str.data, str.size);
    buff->gap_offset += str.size;
    buff->gap_length -= str.size;

    // Update column index
    {
        bool have_newline;
        size_t count = countSymbolsAfterLastNewline(str, &have_newline);
        if (have_newline)
            buff->column_current = count;
        else
            buff->column_current += count;
    }
    return true;
}

PRIVATE bool insertBytesAfterCursor(GapBuffer *buff, String str)
{
    if (buff->gap_length < str.size)
        return false;

    memcpy(buff->data + buff->gap_offset + buff->gap_length - str.size, str.data, str.size);
    buff->gap_length -= str.size;
    return true;
}

GapBuffer *GapBuffer_cloneUsingMemory(void *mem, size_t len, 
                                      void (*free)(void*),
                                      const GapBuffer *src)
{
    GapBuffer *clone = GapBuffer_createUsingMemory(mem, len, free);
    if (!clone)
        return NULL;

    String before = getStringBeforeGap(src);
    if (!insertBytesBeforeCursor(clone, before))
        goto oopsie;

    String after = getStringAfterGap(src);
    if (!insertBytesAfterCursor(clone, after))
        goto oopsie;

    return clone;

oopsie:
    GapBuffer_destroy(clone);
    return NULL;
}

PRIVATE bool isValidUTF8(const char *str, size_t len)
{
    size_t i = 0;
    while (i < len) {
        uint32_t rune; // Unused
        int n = getSymbolRune(str + i, len - i, &rune);
        if (n < 0)
            return false;
        i += n;
    }
    return true;
}

bool GapBuffer_insertString(GapBuffer *buff, const char *str, size_t len)
{
    if (!isValidUTF8(str, len))
        return false;
    bool ok = insertBytesBeforeCursor(buff, (String) {.data=str, .size=len});
    if (ok)
        buff->column_target = buff->column_current;
    return ok;
}

// https://stackoverflow.com/questions/42012563/convert-unicode-code-points-to-utf-8-and-utf-32
static size_t runeToUTF8(unsigned char *const buffer, const unsigned int code)
{
    if (code <= 0x7F) {
        buffer[0] = code;
        return 1;
    }
    if (code <= 0x7FF) {
        buffer[0] = 0xC0 | (code >> 6);            /* 110xxxxx */
        buffer[1] = 0x80 | (code & 0x3F);          /* 10xxxxxx */
        return 2;
    }
    if (code <= 0xFFFF) {
        buffer[0] = 0xE0 | (code >> 12);           /* 1110xxxx */
        buffer[1] = 0x80 | ((code >> 6) & 0x3F);   /* 10xxxxxx */
        buffer[2] = 0x80 | (code & 0x3F);          /* 10xxxxxx */
        return 3;
    }
    if (code <= 0x10FFFF) {
        buffer[0] = 0xF0 | (code >> 18);           /* 11110xxx */
        buffer[1] = 0x80 | ((code >> 12) & 0x3F);  /* 10xxxxxx */
        buffer[2] = 0x80 | ((code >> 6) & 0x3F);   /* 10xxxxxx */
        buffer[3] = 0x80 | (code & 0x3F);          /* 10xxxxxx */
        return 4;
    }
    return 0;
}

bool GapBuffer_insertRune(GapBuffer *gap, unsigned int code)
{
    char temp[4];
    
    size_t num = runeToUTF8((unsigned char*) temp, code);
    return GapBuffer_insertString(gap, temp, num);
}

/* Symbol: getPrecedingSymbol
**
**   Calculate the absolute byte offset of the 
**   [num]-th unicode symbol preceding the cursor.
**
**   If less than [num] symbols precede the
**   cursor, 0 is returned.
**
** Arguments:
**   - buff: Reference to the gap buffer
**
**   - num: Position of the unicode symbol preceding
**          the cursor of which the offset should be
**          returned, relative to the cursor.
**
** Notes:
**   - It's analogous to getFollowingSymbol.
*/
PRIVATE size_t getPrecedingSymbol(GapBuffer *buff, size_t num)
{
    size_t i = buff->gap_offset;

    while (num > 0 && i > 0) {

        // Consume the auxiliary bytes of the
        // UTF-8 sequence (those in the form
        // 10xxxxxx) preceding the cursor
        do {
            assert(i > 0); // FIXME: This triggers sometimes
            i--;
            // The index can never be negative because
            // this loop only iterates over the auxiliary
            // bytes of a UTF-8 byte sequence. If the
            // buffer only contains valid UTF-8, it will
            // not underflow.
        } while (isSymbolAuxiliaryByte(buff->data[i]));

        // A character was consumed.
        num--;
    }

    return i;
}

PRIVATE size_t getSymbolLengthFromFirstByte(uint8_t first)
{
    // NOTE: It's assumed a valid first byte
    if (first >= 0xf0)
        return 4;
    if (first >= 0xe0)
        return 3;
    if (first >= 0xc0)
        return 2;
    return 1;
}

/* Symbol: getFollowingSymbol
**
**   Calculate the absolute byte offset of the 
**   [num]-th unicode symbol following the cursor.
**
**   If less than [num] symbols follow the cursor, 
**   0 is returned.
**
** Arguments:
**   - buff: Reference to the gap buffer
**
**   - num: Position of the unicode symbol following
**          the cursor of which the offset should be
**          returned, relative to the cursor.
**
** Notes:
**   - It's analogous to getPrecedingSymbol.
*/
PRIVATE size_t getFollowingSymbol(GapBuffer *buff, size_t num)
{
    size_t i = buff->gap_offset + buff->gap_length;
    while (num > 0 && i < buff->total) {
        i += getSymbolLengthFromFirstByte(buff->data[i]);
        num--;
    }
    return i;
}

size_t GapBuffer_removeForwards(GapBuffer *buff, size_t num)
{
    size_t gap_length = buff->gap_length;
    size_t i = getFollowingSymbol(buff, num);
    buff->gap_length = i - buff->gap_offset;
    size_t removed = buff->gap_length - gap_length;
    return removed;
}

static void recalculateColumn(GapBuffer *buff)
{
    bool unused;
    buff->column_current = countSymbolsAfterLastNewline(getStringBeforeGap(buff), &unused);
}

size_t GapBuffer_removeBackwards(GapBuffer *buff, size_t num)
{
    size_t gap_length = buff->gap_length;
    size_t i = getPrecedingSymbol(buff, num);
    buff->gap_length += buff->gap_offset - i;
    buff->gap_offset = i;
    
    size_t removed_bytes = buff->gap_length - gap_length;

    if (num > buff->column_current)
        recalculateColumn(buff);
    else
        buff->column_current -= num;

    buff->column_target = buff->column_current;
    return removed_bytes;
}

PRIVATE void moveBytesAfterGap(GapBuffer *buff, size_t num)
{
    assert(buff->gap_offset >= num);

    assert(buff->gap_offset <= buff->total);
    assert(buff->gap_offset <= buff->total);
    assert(buff->gap_offset + buff->gap_length <= buff->total); 

    char *src = buff->data + buff->gap_offset - num;
    char *dst = src + buff->gap_length;

    memmove(dst, src, num);
    buff->gap_offset -= num;

    // Update the cursor index
    {
        String dst2 = {.data=dst, .size=num};
        bool have_newline;
        size_t syms_after_newline = countSymbolsAfterLastNewline(dst2, &have_newline);
        if (have_newline)
            recalculateColumn(buff);
        else
            buff->column_current -= syms_after_newline;
    }
}

PRIVATE void moveBytesBeforeGap(GapBuffer *buff, size_t num)
{
    assert(buff->total - buff->gap_offset - buff->gap_length >= num); // FIXME: This triggers sometimes

    assert(buff->gap_offset <= buff->total);
    assert(buff->gap_offset <= buff->total);
    assert(buff->gap_offset + buff->gap_length <= buff->total);

    char *dst = buff->data + buff->gap_offset;
    char *src = dst + buff->gap_length;

    memmove(dst, src, num);
    buff->gap_offset += num;
    
    // Update the column index    
    {
        String dst2 = {.data=dst, .size=num};
        bool have_newline;
        size_t syms_after_newline = countSymbolsAfterLastNewline(dst2, &have_newline);
        if (have_newline)
            buff->column_current = syms_after_newline;
        else
            buff->column_current += syms_after_newline;
    }
}

size_t GapBuffer_moveRelative(GapBuffer *buff, int off)
{
    if (off < 0) {
        size_t i = getPrecedingSymbol(buff, -off);
        moveBytesAfterGap(buff, buff->gap_offset - i);
    } else {
        size_t i = getFollowingSymbol(buff, off);
        moveBytesBeforeGap(buff, i - buff->gap_offset - buff->gap_length);
    }
    buff->column_target = buff->column_current;
    return buff->gap_offset;
}

size_t GapBuffer_moveAbsolute(GapBuffer *buff, size_t num)
{
    size_t i;
    if (buff->gap_offset > 0)
        i = 0;
    else
        i = buff->gap_length;

    while (num > 0 && i < buff->total) {

        i += getSymbolLengthFromFirstByte(buff->data[i]);

        // If the cursor reached the gap, jump over it.
        if (i == buff->gap_offset)
            i += buff->gap_length;

        num--;
    }
    
    if (i <= buff->gap_offset)
        moveBytesAfterGap(buff, buff->gap_offset - i);
    else
        moveBytesBeforeGap(buff, i - buff->gap_offset - buff->gap_length);
    buff->column_target = buff->column_current;
    
    return buff->gap_offset;
}

void GapBuffer_moveAbsoluteRaw(GapBuffer *gap, size_t num)
{
    if (gap->gap_offset < num)
        moveBytesBeforeGap(gap, num - gap->gap_offset);
    else
        moveBytesAfterGap(gap, gap->gap_offset - num);
    gap->column_target = gap->column_current;
}

void GapBuffer_moveRelativeVertically(GapBuffer *buff, bool up)
{
    size_t cur;
    
    if (up) {

        // Get to the start of the current line
        cur = buff->gap_offset;

        while (cur > 0 && buff->data[cur-1] != '\n')
            cur--;

        if (cur == 0)
            // There's no previous line, so we can't move up
            return;

        cur--;
        assert(buff->data[cur] == '\n');

        // Now find the start of the previous line
        while (cur > 0 && buff->data[cur-1] != '\n')
            cur--;

    } else {

        // Get to the end of the current line
        cur = (int) (buff->gap_offset + buff->gap_length);

        while (cur < buff->total && buff->data[cur] != '\n')
            cur++;

        if (cur == buff->total)
            // It's the last line. Can't move down
            return;

        assert(buff->data[cur] == '\n');
        cur++;
    }

    // Find the byte offset of the character at the given column
    size_t col = 0;
    while (col < buff->column_target && cur < buff->total && buff->data[cur] != '\n') {
        uint32_t unused;
        cur += getSymbolRune(buff->data + cur, buff->gap_offset - cur, &unused);
        col++;
    }

    if (up)
        moveBytesAfterGap(buff, buff->gap_offset - cur);
    else
        moveBytesBeforeGap(buff, cur - (buff->gap_offset + buff->gap_length));
}

void GapBufferIter_init(GapBufferIter *iter, GapBuffer *buff)
{
    iter->crossed_gap = false;
    iter->buff = buff;
    iter->cur = 0;
    iter->mem = NULL;
}

void GapBufferIter_free(GapBufferIter *iter)
{
    iter->mem = NULL;
}

bool GapBufferIter_next(GapBufferIter *iter, GapBufferLine *line)
{
    iter->mem = NULL;

    size_t i = iter->cur;
    size_t total = iter->buff->total;
    size_t gap_offset = iter->buff->gap_offset;
    char *data = iter->buff->data;

    if (iter->crossed_gap) {
        
        size_t line_offset = iter->cur;
        while (i < total && data[i] != '\n')
            i++;
        size_t line_length = i - line_offset;

        if (i < total)
            i++;
        else {
            if (line_length == 0)
                return false;
        }

        line->str = data + line_offset;
        line->len = line_length;
    
    } else {

        size_t line_offset = i;
        while (i < gap_offset && data[i] != '\n')
            i++;
        size_t line_length = i - line_offset;

        if (i == gap_offset) {
            
            i += iter->buff->gap_length;

            size_t line_offset_2 = i;
            while (i < total && data[i] != '\n')
                i++;
            size_t line_length_2 = i - line_offset_2;

            if (i < total)
                i++; // Consume "\n"
            else {
                if (line_length + line_length_2 == 0)
                    return false;
            }

            iter->crossed_gap = true;

            if (line_length + line_length_2 > sizeof(iter->maybe)) {
                // Line will be truncated
                if (line_length > sizeof(iter->maybe))
                    memcpy(iter->maybe, data + line_offset, sizeof(iter->maybe));
                else {
                    memcpy(iter->maybe,               data + line_offset,   line_length);
                    memcpy(iter->maybe + line_offset, data + line_offset_2, sizeof(iter->maybe) - line_length);
                }
                line->str = iter->maybe;
                line->len = line_length + line_length_2;
            } else {
                memcpy(iter->maybe,               data + line_offset,   line_length);
                memcpy(iter->maybe + line_length, data + line_offset_2, line_length_2);
                line->str = iter->maybe;
                line->len = line_length + line_length_2;
            }

        } else {
            i++; // Consume "\n"

            line->str = data + line_offset;
            line->len = line_length;
        }
    }
    iter->cur = i;
    return true;
}

#ifndef GAPBUFFER_NOIO
#include <stdio.h>
bool GapBuffer_insertFile(GapBuffer *gap, const char *file)
{
    FILE *stream = fopen(file, "rb");
    if (stream == NULL)
        return false;

    char buffer[1024];
    for (bool done = false; !done;) {
        size_t num = fread(buffer, 1, sizeof(buffer), stream);
        if (num < sizeof(buffer)) {
            if (ferror(stream))
                goto ouch; // Failed to read from stream
            done = true;
        }
        bool ok = GapBuffer_insertString(gap, buffer, num);
        if (!ok)
            // NOTE: It's possible to have a failure because
            //       the buffer doesn't contain valid utf-8
            //       when a multi-byte symbol is truncated
            //       while reading into the fixed-size buffer.
            goto ouch; // File too big or invalid utf-8
    }
    GapBuffer_moveAbsolute(gap, 0);

    fclose(stream);
    return true;

ouch:
    fclose(stream);
    return false;
}

bool GapBuffer_saveTo(GapBuffer *gap, const char *file)
{
    FILE *stream = fopen(file, "wb");
    if (stream == NULL)
        return false;

    String before = getStringBeforeGap(gap);
    fwrite(before.data, 1, before.size, stream);
    if (ferror(stream)) {
        fclose(stream);
        return false;
    }

    String after = getStringAfterGap(gap);
    fwrite(after.data, 1, after.size, stream);
    if (ferror(stream)) {
        fclose(stream);
        return false;
    }
    fclose(stream);    
    return true;
}
#endif

#ifndef GAPBUFFER_NOMALLOC
#include <stdlib.h>
GapBuffer *GapBuffer_create(size_t capacity)
{
    size_t len = sizeof(GapBuffer) + capacity;
    void  *mem = malloc(len);
    return GapBuffer_createUsingMemory(mem, len, free);
}
bool GapBuffer_insertStringMaybeRelocate(GapBuffer **buff, const char *str, size_t len)
{
    if (!GapBuffer_insertString(*buff, str, len)) {
        // Need to relocate
        GapBuffer *buff2 = GapBuffer_create(GapBuffer_getByteCount(*buff) + len);
        if (buff2 == NULL)
            return false; // Failed to create new location

        if (!GapBuffer_insertString(buff2, str, len)) {
            // Insertion failed unexpectedly. The gap was created
            // with enough free memory to hold the new text..
            GapBuffer_destroy(buff2);
            return false;
        }

        // Swap the parent buffer with the new one
        GapBuffer_destroy(*buff);
        *buff = buff2;
    }

    return true;
}
#endif