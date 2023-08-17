#include "3p/cbuild/cbuild.c"

void raylib(Library *L, Mode M, System OS)
{
    includeDir(L, "/include");
    libraryDir(L, "/lib");
    linkFlags(L, "-l:libraylib.a");
    switch (OS) {
        case LINUX:   linkFlags(L, "-lm -lpthread -ldl"); break;
        case WINDOWS: linkFlags(L, "-lopengl32 -lgdi32 -lwinmm -lcomdlg32 -lws2_32"); break;
    }
}

void snb(Target *T, Mode M, System OS)
{
    targetDesc(T, "The editor's executable");

    sourceDir(T, "src");
    sourceDir(T, "src/utils");
    sourceDir(T, "src/widget");

    compileFlags(T, "-Wall -Wextra -Wpedantic");
    if (M == DEBUG) 
        compileFlags(T, "-g");
    
    switch (OS) {
        case LINUX:   plugLibrary(T, raylib, "3p/raylib-4.5.0_linux_amd64");     break;
        case WINDOWS: plugLibrary(T, raylib, "3p/raylib-4.5.0_win64_mingw-w64"); break;
    }
}

void script(Script *S, System OS)
{
    switch (OS) {
        case LINUX:   plugTarget(S, "editor", "snb",     snb); break;
        case WINDOWS: plugTarget(S, "editor", "snb.exe", snb); break;
    }
    defaultTarget(S, "editor");
}
