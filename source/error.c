#include "error.h"

#include <stdarg.h>
#include <stdio.h>
#include "dictate.h"

static
void verror(const char * fmt, va_list argv) {
    fdictatef(stderr, "$rvimdir: error: ");
    vafdictatef(stderr, fmt, argv);
    fdictate(stderr, "$0");
}

void warning(const char * fmt, ...) {
    va_list argv;
    va_start(argv, fmt);

    fdictatef(stderr, "$yvimdir: warning: ");
    vafdictatef(stderr, fmt, argv);
    fdictate(stderr, "$0");

    va_end(argv);
}

void errorn(int i, ...) {
    va_list argv;
    va_start(argv, i);

    switch (i) {
        case E_OPEN_EDITOR: verror("failed to open editor '%s'", argv);        break;
        case E_FILE_ACCESS: verror("failed to interact with file '%s'", argv); break;
        case E_FILE_DELETE: verror("failed to delete file '%s'", argv);        break;
        case E_FILE_MOVE:   verror("failed to move '%s' to '%s'", argv);       break;
        case E_FORMAT:      verror("directive-file format violation", argv);   break;
        default: verror("unknown error encountered; this is an illegal inner state", 0); break;
    }

    va_end(argv);
}
