#define NOB_IMPLEMENTATION
#include "nob.h"


int main(int argc, char **argv)
{
    Nob_Cmd cmd = {0};
    NOB_GO_REBUILD_URSELF(argc, argv);
    if (!nob_mkdir_if_not_exists("build/")) return 1;
    nob_cmd_append(&cmd, "clang", "-I./libs/");
    nob_cmd_append(&cmd, "-g", "-gcodeview", "-Wno-deprecated-declarations");
    nob_cmd_append(&cmd, "-o", "build/main.exe", "main.c");
    nob_cmd_append(&cmd, "-L./libs/", "-lraylib", "-lopengl32", "-luser32", "-lmsvcrt", "-lgdi32", "-lshell32", "-lwinmm");
    if (!nob_cmd_run_sync(cmd)) return 1;
    return 0;
}