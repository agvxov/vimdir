#include "remove_all.h"

#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <ftw.h>

static
int remove_wrapper(
    const char * fpath,
    const struct stat * sb,
    int typeflag,
    struct FTW * ftwbuf
) {
    if (typeflag == FTW_DNR
    ||  typeflag == FTW_NS) {
        return 1;
    }

    if (remove(fpath)) { return 2; }

    return 0;
}

int remove_all(const char * const p) {
    #define NOPENFD 256
    const int result = nftw(p, remove_wrapper, NOPENFD, FTW_DEPTH | FTW_PHYS);
    if (result) { return -1; }

    return 0;
    #undef NOPENFD
}
