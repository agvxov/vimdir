#ifndef ERROR_H
#define ERROR_H

enum {
    E_OPEN_EDITOR = 1,
    E_FILE_ACCESS,
    E_FILE_DELETE,
    E_FILE_MOVE,
    E_FORMAT,
    E_FLAG,
};

extern void errorn(int i, ...);
extern void warning(const char * fmt, ...);

#define CHECK_OPEN(f, n, E) \
    if (!f) { \
        errorn(E_FILE_ACCESS, n); \
        E; \
    }

#endif
