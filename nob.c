#include <stdio.h>
#include <string.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#ifdef __APPLE__
#define INTERCEPT_BUILD , "intercept-build"
#else
#define INTERCEPT_BUILD
#endif

#define GEN_COMP_DATABASE "bear", "--" INTERCEPT_BUILD

// #define EMSCRIPTEN

int main(int argc, char** argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);
    Cmd cmd = { 0 };
#ifndef EMSCRIPTEN
#ifdef __APPLE__
    if (argc == 2) {
        if (strcmp(argv[1], "Debug") == 0) {
            cmd_append(&cmd,
                GEN_COMP_DATABASE,
                "clang",
                "-Wall",
                "-ggdb",
                "-std=c23",
                "-I/opt/homebrew/include/",
                "-L/opt/homebrew/lib",
                "-lraylib",
                "-Wl,-rpath,/opt/homebrew/lib",
                "-o",
                "cetris",
                "main.c");
            if (!cmd_run_sync_and_reset(&cmd))
                return 1;
        } else if (strcmp(argv[1], "Release") == 0) {
            cmd_append(&cmd,
                GEN_COMP_DATABASE,
                "clang",
                "-Wall",
                "-std=c23",
                "-O3",
                "-I/opt/homebrew/include/",
                "-L/opt/homebrew/lib",
                "-lraylib",
                "-Wl,-rpath,/opt/homebrew/lib",
                "-o",
                "cetris",
                "main.c");
            if (!cmd_run_sync_and_reset(&cmd))
                return 1;
        } else if (strcmp(argv[1], "Static") == 0) {
            cmd_append(&cmd,
                GEN_COMP_DATABASE,
                "clang",
                "-Wall",
                "-std=c23",
                "-O3",
                "-I/opt/homebrew/include/",
                "-isysroot",
                "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk",
                "-framework",
                "Cocoa",
                "-framework",
                "OpenGL",
                "-framework",
                "IOKit",
                "-framework",
                "CoreAudio",
                "-framework",
                "CoreVideo",
                "-lm",
                "-lpthread",
                "-o",
                "cetris",
                "main.c",
                "/opt/homebrew/lib/libraylib.a");
            if (!cmd_run_sync_and_reset(&cmd))
                return 1;
        } else {
            printf("ERROR: Use [Debug|Release|Static]\n");
            return 1;
        }
    } else {
        printf("ERROR: use 1 parameter [Debug|Release]\n");
        return 1;
    }
#else
    if (argc == 2 || argc == 3) {
        if (strcmp(argv[1], "Debug") == 0) {
            cmd_append(&cmd,
                GEN_COMP_DATABASE,
                "clang",
                "-Wall",
                "-ggdb",
                "-std=c23",
                "-I/usr/include",
                "-lraylib",
                "-lm",
                "-o",
                "cetris",
                "main.c");
            if (!cmd_run_sync_and_reset(&cmd))
                return 1;
        } else if (strcmp(argv[1], "Release") == 0) {
            cmd_append(&cmd,
                GEN_COMP_DATABASE,
                "clang",
                "-Wall",
                "-std=c23",
                "-O3",
                "-I/usr/include",
                "-lraylib",
                "-lm",
                "-o",
                "cetris",
                "main.c");
            if (!cmd_run_sync_and_reset(&cmd))
                return 1;
        } else if (strcmp(argv[1], "Static") == 0) {
            cmd_append(&cmd,
                GEN_COMP_DATABASE,
                "clang",
                "-Wall",
                "-std=c23",
                "-O3",
                "-I/usr/include",
                "-lm",
                "-o",
                "cetris",
                "main.c",
                argv[2]);
            if (!cmd_run_sync_and_reset(&cmd))
                return 1;
        } else {
            printf("ERROR: Use [Debug|Release|Static + (path-to-static-lib)]\n");
            return 1;
        }
    } else {
        printf("ERROR: use 1 or 2(for static) parameter [Debug|Release|Static+(path-to-static-lib)]\n");
        return 1;
    }
#endif
#else
    cmd_append(&cmd,
        // GEN_COMP_DATABASE,
        "emcc",
        "-o",
        "cetris.html",
        "main.c",
        "-std=c23",
        "-Os",
        "-Wall",
        "raylib-5.5/build/raylib/libraylib.a",
        "-I./raylib-5.5/build/raylib/include",
        "-I/opt/homebrew/opt/emscripten/libexec/cache/sysroot/include",
        "-L./raylib-5.5/build/raylib",
        "-s",
        "USE_GLFW=3",
        "-s",
        "ASYNCIFY",
        "-s",
        "EXPORTED_RUNTIME_METHODS=ccall, HEAPF32",
        "--shell-file",
        "./raylib-5.5/src/minshell.html",
        "--preload-file",
        "./resources",
        "-s",
        "STACK_SIZE=2097152",
        "-s",
        "USE_WEBGL2=1",
        "-s",
        "ALLOW_MEMORY_GROWTH=1",
        "-sASSERTIONS",
        "-DPLATFORM_WEB");
    if (!cmd_run_sync_and_reset(&cmd))
        return 1;
#endif
    return 0;
}
