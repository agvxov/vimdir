#include "error.h"

#include <stdarg.h>
#include <stdio.h>
#include "dictate.h"

static
void verror(const char * fmt, va_list argv) {
    fdictatef(stderr, "vimdir: error: ");
    vafdictatef(stderr, fmt, argv);
    fdictate(stderr, "");
}

void warning(const char * fmt, ...) {
    va_list argv;
    va_start(argv, fmt);

    fdictatef(stderr, "vimdir: warning: ");
    vafdictatef(stderr, fmt, argv);
    fdictate(stderr, "");

    va_end(argv);
}

void errorn(int i, ...) {
    va_list argv;
    va_start(argv, i);

    switch (i) {
        case E_OPEN_EDITOR: verror("failed to open editor '%s'", argv); break;
        case E_FILE_ACCESS: verror("failed to interact with file '%s'", argv); break;
        case E_FILE_DELETE: verror("failed to delete file '%s'", argv); break;
        case E_FILE_MOVE:   verror("failed to move '%s' to '%s'", argv); break;
        case E_FORMAT:      verror("directive-file format violation", argv); break;
    }

    va_end(argv);
}
