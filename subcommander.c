/*
 * Copyright 2023 Lassi Kortela
 * SPDX-License-Identifier: ISC
 */

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef PROGNAME
#define PROGNAME "subcommander"
#endif

static char *simple_basename(char *path)
{
    char *slash;

    if ((slash = strrchr(path, '/'))) {
        return &slash[1];
    }
    return path;
}

static char path[2048];
static const char *command;

static void fatal(const char *message)
{
    fprintf(stderr, "%s: %s\n", command, message);
    exit(2);
}

static void fatal_errno_string(const char *message, const char *string)
{
    fprintf(
        stderr, "%s: %s %s: %s\n", command, message, string, strerror(errno));
    exit(2);
}

static void fatal_memory(void) { fatal("out of memory"); }

static void fatal_path(void) { fatal("path not valid"); }

static void fatal_home(void) { fatal("HOME not valid"); }

static void fatal_errno_dirlist(void)
{
    fatal_errno_string("cannot list directory", path);
}

static void fatal_errno_exec(void) { fatal_errno_string("cannot run", path); }

static void assert_valid_name(const char *name)
{
    if (!name[0] || strchr(name, '/')) {
        fatal_path();
    }
}

static void assert_valid_command(const char *name)
{
    assert_valid_name(name);
    if (name[0] == '.') {
        fatal_path();
    }
}

static void path_set(const char *newpath)
{
    snprintf(path, sizeof(path), "%s", newpath);
}

static void path_add(const char *name)
{
    size_t n;

    assert_valid_name(name);
    n = strlen(path);
    snprintf(&path[n], sizeof(path) - n, "/%s", name);
}

static int compare_names(const void *a_void, const void *b_void)
{
    const char *a = *(const char **)a_void;
    const char *b = *(const char **)b_void;

    return strcmp(a, b);
}

static void list_subcommands(void)
{
    static char *names[100];
    const size_t width = sizeof(names[0]);
    const size_t max = sizeof(names) / width;
    char *name;
    DIR *dir;
    struct dirent *d;
    size_t i, n;

    n = 0;
    if (!(dir = opendir(path))) {
        fatal_errno_dirlist();
    }
    for (;;) {
        errno = 0;
        if (n >= max) {
            break;
        }
        if (!(d = readdir(dir))) {
            break;
        }
        name = d->d_name;
        if (name[0] == '.') {
            continue;
        }
        if (!(name = strdup(name))) {
            fatal_memory();
        }
        names[n++] = name;
    }
    if (errno) {
        fatal_errno_dirlist();
    }
    if (closedir(dir) == -1) {
        fatal_errno_dirlist();
    }
    qsort(names, n, width, compare_names);
    for (i = 0; i < n; i++) {
        printf("%s\n", names[i]);
    }
}

static void run_subcommand(char **argv)
{
    assert_valid_command(argv[0]);
    path_add(argv[0]);
    execv(path, argv);
    if (errno == ENOENT) {
        fatal("no such subcommand");
    }
    fatal_errno_exec();
}

int main(int argc, char **argv)
{
    const char *home;
    char **sub_argv;

    command = simple_basename(argv[0]);
    assert_valid_command(command);
    sub_argv = 0;
    if (argc > 1) {
        sub_argv = &argv[1];
    }
    home = getenv("HOME");
    if (!home) {
        fatal_home();
    }
    if (home[0] != '/') {
        fatal_home();
    }
    path_set(home);
    path_add(".config");
    path_add(PROGNAME);
    path_add(command);
    if (sub_argv) {
        run_subcommand(sub_argv);
    } else {
        list_subcommands();
    }
    return 0;
}
