#include <raylib.h>
#include "style.h"
#include "main_editor.h"
#include "dispatch.h"

int editor(int argc, char **argv)
{
    initStyle();

    const char *file;
    if (argc > 1)
        file = argv[1];
    else
        file = NULL;

    //SetTraceLogCallback(logRoutine);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    InitWindow(720, 500, "SnB");

    Widget *root = (Widget*) createStylizedBufferView();
    root->parent = &root;

    if (file)
        openFileIntoWidget(root, file);

    while (!WindowShouldClose()) {
        dispatchEvents(root);
        BeginDrawing();
        ClearBackground(WHITE);
        Vector2 offset = {0, 0};
        Vector2 area = {GetScreenWidth(), GetScreenHeight()};
        drawWidget(root, offset, area);
        EndDrawing();
    }

    freeWidget(root);
    CloseWindow();
    freeStyle();
    return 0;
}