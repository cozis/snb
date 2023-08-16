#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <assert.h>
#include "style.h"
#include "dispatch.h"
#include "main_choose_file_dialog.h"

#ifdef _WIN32
#define PATHSEP "\\"
#else
#define PATHSEP "/"
#endif

typedef struct {
    bool    dir;
    long    mod;
    size_t size;
} ItemInfo;

typedef struct {
    Widget    *root;
    GroupView *group;

    TableView table;
    
    TextInput *name;
    Button    *cancel;
    Button    *submit;
    
    TextInput *path;
    Button    *parent;

    int iter_index;

    FilePathList list;
    ItemInfo    *info;

    bool submited;
    bool canceled;
} FileChooser;

static void iter_start(void *context);
static void iter_end(void *context);
static bool iter_next(void *context);
static void iter_field(void *context, int index, char *dst, size_t max);

static bool setDirectory(FileChooser *win, const char *path);

static void eventCallback(void *context, Button *button)
{
    FileChooser *win = context;
    if (button == win->cancel) win->canceled = true;
    if (button == win->submit)   win->submited = true;
    if (button == win->parent) {
        
        char current[1024];
        getTextInputContents(win->path, current, sizeof(current));
        
        const char *parent = GetPrevDirectoryPath(current);
        assert(parent);

        if (setDirectory(win, parent))
            setTextInputContents(win->path, parent);
    }
}

static TableFunctions iter_funcs = {
    .start = iter_start,
    .field = iter_field,
    .next  = iter_next,
    .end   = iter_end,
};

static void table_callback(void *context, int index)
{
    FileChooser *win = context;
    assert(index >= 0 && index < (int) win->list.count);
    
    char     *path = win->list.paths[index];
    ItemInfo *info = &win->info[index];

    if (info->dir)
        setDirectory(win, path);
    else
        setTextInputContents(win->name, GetFileName(path));
}

static bool initFileChooser(FileChooser *win, bool save, float spacing, float button_w, float input_h)
{
    GroupView *group = createStylizedGroupView();

    TextInput *path = createStylizedTextInput();
    setMarginX((Widget*) path, spacing);
    setMarginY((Widget*) path, spacing);
    setDesiredWidth((Widget*) path, GetScreenWidth() - 3 * spacing - button_w);
    setDesiredHeight((Widget*) path, input_h);
    insertChildIntoGroup(group, (Widget*) path);
    
    Button *parent = createStylizedButton("Parent", win, eventCallback);
    setMarginX((Widget*) parent, spacing);
    setMarginY((Widget*) parent, spacing);
    setDesiredWidth((Widget*) parent, button_w);
    setDesiredHeight((Widget*) parent, input_h);
    insertChildIntoGroup(group, (Widget*) parent);
    
    initStylizedTableView(&win->table, win, iter_funcs, table_callback);
    setMarginX((Widget*) &win->table, spacing);
    setMarginY((Widget*) &win->table, spacing);
    setDesiredWidth((Widget*) &win->table, GetScreenWidth() - 2 * spacing);
    setDesiredHeight((Widget*) &win->table, GetScreenHeight() - 4 * spacing - 2 * input_h);
    insertChildIntoGroup(group, (Widget*) &win->table);
    setColumnLabel(&win->table, 0, "Name");
    setColumnLabel(&win->table, 1, "Size");
    setColumnLabel(&win->table, 2, "Time");

    TextInput *name = createStylizedTextInput();
    setMarginX((Widget*) name, spacing);
    setMarginY((Widget*) name, spacing);
    setDesiredWidth((Widget*) name, GetScreenWidth() - 4 * spacing - 2 * button_w);
    setDesiredHeight((Widget*) name, input_h);
    insertChildIntoGroup(group, (Widget*) name);

    Button *cancel = createStylizedButton("Cancel", win, eventCallback);
    setMarginX((Widget*) cancel, spacing);
    setMarginY((Widget*) cancel, spacing);
    setDesiredWidth((Widget*) cancel, button_w);
    setDesiredHeight((Widget*) cancel, input_h);
    insertChildIntoGroup(group, (Widget*) cancel);

    const char *label = save ? "Save" : "Open";
    Button *submit = createStylizedButton(label, win, eventCallback);
    setMarginX((Widget*) submit, spacing);
    setMarginY((Widget*) submit, spacing);
    setDesiredWidth((Widget*) submit, button_w);
    setDesiredHeight((Widget*) submit, input_h);
    insertChildIntoGroup(group, (Widget*) submit);

    Widget *root = (Widget*) group;
    root->parent = &root;

    win->root   = root;
    win->group  = group;
    win->name   = name;
    win->cancel = cancel;
    win->submit = submit;
    win->parent = parent;
    win->path   = path;
    win->info   = NULL;
    win->list.paths    = NULL;
    win->list.count    = 0;
    win->list.capacity = 0;
    win->canceled = false;
    win->submited = false;

    return true;
}

static void freeFileChooser(FileChooser *win)
{
    if (win->list.paths)
        UnloadDirectoryFiles(win->list);
    freeWidget(win->root);
}

static void logRoutine(int level, const char *text, va_list args)
{
    (void) level;
    (void) text;
    (void) args;
/*
    const char *label;
    switch (level) {
        case LOG_TRACE: label = "TRACE"; break;
        case LOG_DEBUG: label = "DEBUG"; break;
        case LOG_INFO:  label = "INFO"; break;
        case LOG_WARNING: label = "WARNING"; break;
        case LOG_ERROR: label = "ERROR"; break;
        case LOG_FATAL: label = "FATAL"; break;
        default: label = "???"; break;
    }
    fprintf(stderr, "%s :: ", label);
    vfprintf(stderr, text, args);
    fprintf(stderr, "\n");
*/
}

static bool setDirectory(FileChooser *win, const char *path)
{
    FilePathList list = LoadDirectoryFiles(path);
    // TODO: Check that the operation was successful

    ItemInfo *info = malloc(list.count * sizeof(ItemInfo));
    if (info == NULL) {
        UnloadDirectoryFiles(list);
        return false;
    }
    for (int i = 0; i < (int) list.count; i++) {
        const char *path = list.paths[i];
        info[i].size = GetFileLength(path);
        info[i].mod  = GetFileModTime(path);
        info[i].dir  = DirectoryExists(path);
    }

    if (win->list.paths != NULL) {
        free(win->info);
        UnloadDirectoryFiles(win->list);
    }
    win->info = info;
    win->list = list;

    setTextInputContents(win->path, path);
    tableViewChanged(&win->table);
    return true;
}

static void iter_start(void *context)
{
    FileChooser *win = context;
    win->iter_index = 0;
}

static void iter_end(void *context)
{
    (void) context;
}

static bool iter_next(void *context)
{
    FileChooser *win = context;
    if (win->iter_index == (int) win->list.count)
        return false;
    win->iter_index++;
    return true;
}

#define GB (1024 * 1024 * 1024)
#define MB (1024 * 1024)
#define KB (1024)

static void byteCountToHumanReadableString(size_t bytes, char *dst, size_t max)
{
    int gb = (bytes %  1) / GB;
    int mb = (bytes % GB) / MB;
    int kb = (bytes % MB) / KB;
    int  b = (bytes % KB) /  1;

    if (gb > 0)
        snprintf(dst, max, "%d.%d GB", gb, mb);
    else if (mb > 0)
        snprintf(dst, max, "%d.%d MB", mb, kb);
    else if (kb > 0)
        snprintf(dst, max, "%d.%d KB", kb, b);
    else
        snprintf(dst, max, "%d B", b);
}

static void 
timeToHumanReadableString(long time, char *dst, size_t max)
{
    time_t t = 1 * time;
    struct tm lt;
#ifdef _WIN32
    localtime_s(&lt, &t);
#else
    localtime_r(&t, &lt); // TODO: Check that this is right
#endif
    strftime(dst, max, "%c", &lt);
}

static void iter_field(void *context, int index, char *dst, size_t max)
{
    FileChooser *win = context;

    assert(win->iter_index <= (int) win->list.count);

    char     *path =  win->list.paths[win->iter_index-1];
    ItemInfo *info = &win->info[win->iter_index-1];

    switch (index) {
        case 0: strncpy(dst, GetFileName(path), max); break;
        case 1: byteCountToHumanReadableString(info->size, dst, max); break;
        case 2: timeToHumanReadableString(info->mod, dst, max); break;
        default: strncpy(dst, "???", max); break;
    }
}

int chooseFileDialog(int argc, char **argv)
{
    initStyle();

    assert(argc >= 2);
    bool save = (argv[1][0] == 's');

    SetTraceLogCallback(logRoutine);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    const char *title = save ? "SnB - Save file" : "SnB - Open file";
    InitWindow(720, 500, title);

    loadStyleFrom("style.cfg");
    
    float spacing  = 10;
    float input_h  = 30;
    float button_w = 100;

    FileChooser file_chooser;
    initFileChooser(&file_chooser, save, spacing, button_w, input_h);
    setDirectory(&file_chooser, GetWorkingDirectory());

    float last_window_w = GetScreenWidth();
    float last_window_h = GetScreenHeight();

    while (!WindowShouldClose() && !file_chooser.canceled && !file_chooser.submited) {

        float window_w = GetScreenWidth();
        float window_h = GetScreenHeight();
        bool resized_w = (window_w != last_window_w);
        bool resized_h = (window_h != last_window_h);
        bool resized   = resized_w || resized_h;
        last_window_w = window_w;
        last_window_h = window_h;

        if (resized) {
            setDesiredWidth((Widget*) file_chooser.path, GetScreenWidth() - 3 * spacing - 1 * button_w);
            setDesiredWidth((Widget*) file_chooser.name, GetScreenWidth() - 4 * spacing - 2 * button_w);
            setDesiredWidth((Widget*) &file_chooser.table, GetScreenWidth() - 2 * spacing);
            setDesiredHeight((Widget*) &file_chooser.table, GetScreenHeight() - 4 * spacing - 2 * input_h);
        }

        dispatchEvents(file_chooser.root);
        BeginDrawing();
        ClearBackground(WHITE);
        Vector2 offset = {0, 0};
        Vector2 area = {window_w, window_h};
        drawWidget(file_chooser.root, offset, area);
        EndDrawing();
    }

    if (file_chooser.submited) {
        size_t total = getTextInputContents(file_chooser.path, NULL, 0)
                     + getTextInputContents(file_chooser.name, NULL, 0)
                     + sizeof(PATHSEP)-1;
        char buffer[1024];
        if (total > sizeof(buffer)-1)
            fprintf(stderr, "Path buffer is too small");
        else {
            size_t copied;
            copied = getTextInputContents(file_chooser.path, buffer, sizeof(buffer));    
            strcat(buffer + copied, PATHSEP);
            copied += sizeof(PATHSEP)-1;
            getTextInputContents(file_chooser.name, buffer + copied, sizeof(buffer) - copied);

            fwrite(buffer, 1, total, stdout);
        }
    }

    freeFileChooser(&file_chooser);
    CloseWindow();
    freeStyle();
    return 0;
}
