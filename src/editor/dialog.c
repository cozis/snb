#include <stdio.h>
#include <stdbool.h>
#include "dialog.h"

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

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

int chooseFile(char *dst, size_t max)
{
    const char *dialog_exe;

#ifdef _WIN32
    dialog_exe = "snb-dialog";
#else
    dialog_exe = "./snb-dialog";
#endif

    FILE *stream = popen(dialog_exe, "r");
    if (stream == NULL)
        return -1;

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

#endif