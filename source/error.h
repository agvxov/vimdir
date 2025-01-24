#ifndef ERROR_H
#define ERROR_H

enum {
    E_OPEN_EDITOR = 1,
    E_FILE_ACCESS,
    E_FILE_DELETE,
    E_FILE_MOVE,
    E_FILE_SWAP,
    E_FILE_COPY,
    E_FILE_CHOWN,
    E_NO_USER,
    E_NO_GROUP,
    E_FORMAT,
    E_INDEX,
    E_FLAG,
};

extern void errorn(int n, ...);
extern void warning(const char * fmt, ...);

#define CHECK_OPEN(f, n, E) \
    if (!f) { \
        errorn(E_FILE_ACCESS, n); \
        E; \
    }

#endif
