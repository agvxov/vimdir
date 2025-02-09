#ifndef ERROR_H
#define ERROR_H

enum {
    E_OPEN_EDITOR = 1,
    E_IN_EDITOR,
    E_FILE_ACCESS,
    E_FILE_DELETE,
    E_FILE_MOVE,
    E_FILE_SWAP,
    E_FILE_COPY,
    E_FILE_CHOWN,
    E_DIR_ACCESS,
    E_NO_USER,
    E_NO_GROUP,
    E_FORMAT,
    E_INDEX,
    E_FLAG,
};

extern void errorn(int n, ...);
extern void notice(const char * fmt, ...);

#define CHECK_OPEN(f, n, E) do { \
        if (!f) { \
            errorn(E_FILE_ACCESS, n); \
            E; \
        } \
    } while (0)

#endif
