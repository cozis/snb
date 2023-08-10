#include <raylib.h>
#include "utils/drink.h"
#include "utils/exename.h"
#include "spawn_dialog.h"

#ifdef _WIN32
#define PATHSEP "\\"
#else
#define PATHSEP "/"
#endif

int chooseFileToOpen(char *dst, size_t max)
{
    const char *path = GetApplicationDirectory();
    const char *name = GetFileName(getExecutableName());
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s" PATHSEP "%s open-file-dialog", path, name);
    
    return drinkFromProgram(cmd, dst, max);
}

int chooseFileToSave(char *dst, size_t max)
{
    const char *path = GetApplicationDirectory();
    const char *name = GetFileName(getExecutableName());
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s" PATHSEP "%s save-file-dialog", path, name);
    
    return drinkFromProgram(cmd, dst, max);
}