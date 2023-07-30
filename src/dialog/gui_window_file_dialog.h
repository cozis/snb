/*******************************************************************************************
*
*   Window File Dialog v1.2 - Modal file dialog to open/save files
*
*   MODULE USAGE:
*       #define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
*       #include "gui_window_file_dialog.h"
*
*       INIT: GuiWindowFileDialogState state = GuiInitWindowFileDialog();
*       DRAW: GuiWindowFileDialog(&state);
*
*   NOTE: This module depends on some raylib file system functions:
*       - LoadDirectoryFiles()
*       - UnloadDirectoryFiles()
*       - GetWorkingDirectory()
*       - DirectoryExists()
*       - FileExists()
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2019-2023 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#ifndef GUI_WINDOW_FILE_DIALOG_H
#define GUI_WINDOW_FILE_DIALOG_H

#include <raylib.h>

// Gui file dialog context data
typedef struct {

    // Window management variables
    bool windowActive;
    Rectangle windowBounds;
    Vector2 panOffset;
    bool dragMode;
    bool supportDrag;

    // UI variables
    bool dirPathEditMode;
    char dirPathText[1024];

    int filesListScrollIndex;
    bool filesListEditMode;
    int filesListActive;

    bool fileNameEditMode;
    char fileNameText[1024];
    bool SelectFilePressed;
    bool CancelFilePressed;
    int fileTypeActive;
    int itemFocused;

    // Custom state variables
    FilePathList dirFiles;
    char filterExt[256];
    char dirPathTextCopy[1024];
    char fileNameTextCopy[1024];

    int prevFilesListActive;

    bool saveFileMode;

} GuiWindowFileDialogState;

GuiWindowFileDialogState InitGuiWindowFileDialog(const char *initPath);
void GuiWindowFileDialog(GuiWindowFileDialogState *state);

#endif // GUI_WINDOW_FILE_DIALOG_H
