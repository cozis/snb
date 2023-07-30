#include <stdio.h>
#include <raylib.h>
#include "raygui.h"
#include "gui_window_file_dialog.h"

static void log_callback(int logLevel, const char *text, va_list args)
{
    //vfprintf(stderr, text, args);
}

int main(int argc, char **argv)
{
    SetTraceLogCallback(log_callback);

    int win_w = 800;
    int win_h = 560;

    InitWindow(win_w, win_h, "Choose file");
    SetTargetFPS(60);

    GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
    fileDialogState.windowActive = true;
    fileDialogState.windowBounds.x = 0;
    fileDialogState.windowBounds.y = 0;
    fileDialogState.windowBounds.width  = GetScreenWidth();
    fileDialogState.windowBounds.height = GetScreenHeight();
    fileDialogState.supportDrag = false;

    while (!WindowShouldClose() && fileDialogState.windowActive) {
        BeginDrawing();
        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
        GuiWindowFileDialog(&fileDialogState);
        EndDrawing();
    }

    if (fileDialogState.fileNameText[0])
        fprintf(stdout, "%s\\%s", fileDialogState.dirPathText, fileDialogState.fileNameText);

    CloseWindow();
    return 0;
}