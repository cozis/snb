#include <stdio.h>
#include <stdbool.h>
#include "dialog.h"

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

#ifdef _WIN32
#define SNB_DIALOG "snb-dialog"
#else
#define SNB_DIALOG "./snb-dialog"
#endif

#ifdef _WIN32

#include <windows.h>
#include <string.h>

int chooseFile(char *dst, size_t max)
{
    HWND hWnd = GetActiveWindow();

    // common dialog box structure, setting all fields to 0 is important
    OPENFILENAME ofn = {0}; 
    TCHAR szFile[260]={0};
    // Initialize remaining fields of OPENFILENAME structure
    ofn.lStructSize = sizeof(ofn); 
    ofn.hwndOwner = hWnd; 
    ofn.lpstrFile = szFile; 
    ofn.nMaxFile = sizeof(szFile); 
    ofn.lpstrFilter = TEXT("All\0*.*\0Text\0*.TXT\0"); 
    ofn.nFilterIndex = 1; 
    ofn.lpstrFileTitle = NULL; 
    ofn.nMaxFileTitle = 0; 
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if(GetOpenFileName(&ofn) == TRUE) {
        
        size_t len = strlen(ofn.lpstrFile);
        if (len >= max) len = max-1;

        strncpy(dst, ofn.lpstrFile, max);
        dst[len] = '\0';
        return (int) len;
    }

    return 0;
}

#else

static int drinkFromStream(FILE *stream, char *dst, size_t max)
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

static int drinkFromProgram(const char *cmd, char *dst, size_t max)
{
    FILE *stream = popen(cmd, "r");
    if (stream == NULL)
        return -1;

    int res = drinkFromStream(stream, dst, max);

    if (pclose(stream) != 0)
        return -1;
    return res;
}

static int chooseFileRaylib(char *dst, size_t max)
{
    return drinkFromProgram(SNB_DIALOG, dst, max);
}

static void removeTrailingNewline(char *str, int *len)
{
    if (*len > 0 && str[*len-1] == '\n')
        str[--(*len)] = '\0';
}

static int chooseFileZenity(char *dst, size_t max)
{
    int res = drinkFromProgram("zenity --file-selection", dst, max);
    removeTrailingNewline(dst, &res); // Zenity returns a newline at the end of the output
    return res;
}

static bool zenityMissing(void)
{
    FILE *stream = popen("zenity --help", "r");
    return stream == NULL || pclose(stream) != 0;
}

int chooseFile(char *dst, size_t max)
{
    int res;
    if (zenityMissing())
        res = chooseFileRaylib(dst, max);
    else
        res = chooseFileZenity(dst, max);
    return res;
}

#endif