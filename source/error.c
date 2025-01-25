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

void notice(const char * fmt, ...) {
    va_list argv;
    va_start(argv, fmt);

    fdictatef(stderr, "$yvimdir: notice: ");
    vafdictatef(stderr, fmt, argv);
    fdictate(stderr, "$0");

    va_end(argv);
}

void errorn(int n, ...) {
    va_list argv;
    va_start(argv, n);

    switch (n) {
        case E_OPEN_EDITOR: verror("failed to open editor '%s'", argv);            break;
        case E_IN_EDITOR:   verror("editor exited with '%d'", argv);               break;
        case E_FILE_ACCESS: verror("failed to interact with file '%s'", argv);     break;
        case E_FILE_DELETE: verror("failed to delete file '%s'", argv);            break;
        case E_FILE_MOVE:   verror("failed to move '%s' to '%s'", argv);           break;
        case E_FILE_SWAP:   verror("failed to swap '%s' with '%s'", argv);         break;
        case E_FILE_COPY:   verror("failed to copy '%s' to '%s'", argv);           break;
        case E_FILE_CHOWN:  verror("failed to chown '%s'", argv);                  break;
        case E_NO_USER:     verror("failed to find the user '%s'", argv);          break;
        case E_NO_GROUP:    verror("failed to find the group '%s'", argv);         break;
        case E_FORMAT:      verror("directive-file format violation", argv);       break;
        case E_INDEX:       verror("file index violation encountered (%d)", argv); break; 
        case E_FLAG:        verror("unknown flag '%c'", argv);                     break;
        default: verror("unknown error encountered; this is an illegal inner state", 0); break;
    }

    va_end(argv);
}
