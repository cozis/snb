#include <string.h>
#include "main_editor.h"
#include "main_choose_file_dialog.h"

static const char *program_name;
const char *getExecutableName(void)
{
    return program_name;
}

int main(int argc, char **argv)
{
    program_name = argv[0];
    
    if (argc > 1 && (!strcmp(argv[1], "open-file-dialog") || !strcmp(argv[1], "save-file-dialog")))
        return chooseFileDialog(argc, argv);
    else
        return editor(argc, argv);
}
