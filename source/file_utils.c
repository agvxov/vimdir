#include "file_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include "error.h"

extern char * trim_trailing_slashes(char * path) {
    int len = strlen(path);
    while (len > 1
       &&  path[len-1] == '/') {
        path[len-1] = '\0';
        --len;
    }
    return path;
}

static const char * custom_rm = NULL;

int (*mytouch)(const char *filename) = NULL;
int (*mydelete)(const char *filename) = NULL;
int (*mychmod)(const char *filename, mode_t mode) = NULL;
int (*mychown)(const char *filename, const char *owner, const char *group) = NULL;
int (*mymove)(const char *filename, const char *newname) = NULL;
int (*mycopy)(const char *filename, const char *newname) = NULL;
move_data_t (*mytempmove)(const char *filename, const char *newname) = NULL;

static int dry_touch(const char * filename);
static int dry_delete(const char * filename);
static int dry_chmod(const char * filename, mode_t mode);
static int dry_chown(const char * filename, const char * owner, const char * group);
static int dry_move(const char * filename, const char * newname);
static int dry_copy(const char * filename, const char * newname);
static move_data_t dry_mytempmove(const char *filename, const char *newname);

static int moist_touch(const char * filename);
static int moist_delete(const char * filename);
static int moist_chmod(const char * filename, mode_t mode);
static int moist_chown(const char * filename, const char * owner, const char * group);
static int moist_move(const char * filename, const char * newname);
static int moist_copy(const char * filename, const char * newname);
static move_data_t moist_mytempmove(const char *filename, const char *newname);

int init_file_utils(bool is_dry_run, const char * custom_rm_) {
    custom_rm = custom_rm_;

    if (is_dry_run) {
        mytouch  = dry_touch;
        mydelete = dry_delete;
        mychmod  = dry_chmod;
        mychown  = dry_chown;
        mymove   = dry_move;
        mycopy   = dry_copy;
        mytempmove = dry_mytempmove;
    } else {
        mytouch  = moist_touch;
        mydelete = moist_delete;
        mychmod  = moist_chmod;
        mychown  = moist_chown;
        mymove   = moist_move;
        mycopy   = moist_copy;
        mytempmove = moist_mytempmove;
    }

    return 0;
}

int deinit_file_utis() {
    mytouch  = NULL;
    mydelete = NULL;
    mychmod  = NULL;
    mychown  = NULL;
    mymove   = NULL;

    custom_rm = NULL;

    return 0;
}

char mode_type_to_char(mode_t m) {
    switch (m & S_IFMT) {
        case S_IFREG:  return '-';  // regular file
        case S_IFDIR:  return 'd';  // directory
        case S_IFCHR:  return 'c';  // character device
        case S_IFBLK:  return 'b';  // block device
        case S_IFIFO:  return 'p';  // fifo (pipe)
        case S_IFLNK:  return 'l';  // symbolic link
        case S_IFSOCK: return 's';  // socket
        default: return '?';        // unknown
    }
}

mode_t char_to_mode_type(const char c) {
    switch (c) {
        case '-': return S_IFREG;  // regular file
        case 'd': return S_IFDIR;  // directory
        case 'c': return S_IFCHR;  // character device
        case 'b': return S_IFBLK;  // block device
        case 'p': return S_IFIFO;  // fifo (pipe)
        case 'l': return S_IFLNK;  // symbolic link
        case 's': return S_IFSOCK; // socket
        default: return 0;         // unknown
    }
}

char * mode_to_str(mode_t mode, char * buffer) {
    buffer[0] = mode_type_to_char(mode);

    buffer[1] = (mode & S_IRUSR) ? 'r' : '-';
    buffer[2] = (mode & S_IWUSR) ? 'w' : '-';
    buffer[3] = (mode & S_IXUSR) ? 'x' : '-';

    buffer[4] = (mode & S_IRGRP) ? 'r' : '-';
    buffer[5] = (mode & S_IWGRP) ? 'w' : '-';
    buffer[6] = (mode & S_IXGRP) ? 'x' : '-';

    buffer[7] = (mode & S_IROTH) ? 'r' : '-';
    buffer[8] = (mode & S_IWOTH) ? 'w' : '-';
    buffer[9] = (mode & S_IXOTH) ? 'x' : '-';

    buffer[10] = '\0';
    return buffer;
}

mode_t str_to_mode(const char *permissions) {
    mode_t mode = 0;

    mode |= char_to_mode_type(permissions[0]);

    mode |= (permissions[1] == 'r') ? S_IRUSR : 0;
    mode |= (permissions[2] == 'w') ? S_IWUSR : 0;
    mode |= (permissions[3] == 'x') ? S_IXUSR : 0;

    mode |= (permissions[4] == 'r') ? S_IRGRP : 0;
    mode |= (permissions[5] == 'w') ? S_IWGRP : 0;
    mode |= (permissions[6] == 'x') ? S_IXGRP : 0;

    mode |= (permissions[7] == 'r') ? S_IROTH : 0;
    mode |= (permissions[8] == 'w') ? S_IWOTH : 0;
    mode |= (permissions[9] == 'x') ? S_IXOTH : 0;

    return mode;
}

// --- Dry implementations
static
int dry_touch(const char * filename) {
    size_t len = strlen(filename);
    if (filename[len-1] != '/' ) {
        notice("touch '%s' (dry; subsequent stats will fail)", filename);
    } else {
        notice("mkdir '%s' (dry; subsequent stats will fail)", filename);
    }
    return 0;
}

static
int dry_delete(const char * filename) {
    notice("delete '%s'", filename);
    return 0;
}

static
int dry_chmod(const char * filename, mode_t mode) {
    char buf[11];
    notice("chmod '%s' (%s)", filename, mode_to_str(mode, buf));
    return 0;
}

static
int dry_chown(const char * filename, const char * owner, const char * group) {
    notice("chown '%s' (%s:%s)", filename, owner, group);
    return 0;
}

static
int dry_move(const char * filename, const char * newname) {
    notice("rename '%s' (-> '%s')", filename, newname);
    return 0;
}

static
int dry_copy(const char * filename, const char * newname) {
    notice("copy '%s' (as '%s')", filename, newname);
    return 0;
}

static
move_data_t dry_mytempmove(const char * filename, const char * newname) {
    notice("swap detected in a dry-run ('%s' <-> '%s'); the following logs will be inaccurate", filename, newname);
    return (move_data_t) {
        .orig_name = strdup(filename),
        .curt_name = strdup(filename),
        .dest_name = strdup(newname),
    };
}

// --- Moist implementations
static
int moist_touch(const char * filename) {
    size_t len = strlen(filename);
    if (filename[len-1] != '/' ) {
        FILE * f = fopen(filename, "w");
        CHECK_OPEN(f, filename, return 1);
        fclose(f);
    } else {
        mkdir(filename, 0777);
    }
    return 0;
}

static
int moist_delete(const char * filename) {
    /* Theres the situation where the user attempts
     *  to delete a recursively listed directory.
     * He would delete all references to the directory
     *  (otherwise it would be a stat error too).
     * Since the entry came somewhere, its reasonably safe to assume the file should exist,
     *  and if it does not, its not an actual error.
     * Therefor, we simply make deletes on a missing file a nop.
     */
    if (access(filename, F_OK)) { return 0; }

    if (custom_rm) {
        size_t cmd_len = strlen(custom_rm) + sizeof(' ') + strlen(filename) + 1;
        char cmd[cmd_len];

        snprintf(cmd, cmd_len, "%s %s", custom_rm, filename);

        int result = system(cmd);
        if (result == 127
        ||  result == -1
        || (WIFEXITED(result) && WEXITSTATUS(result) != 0)) {
            errorn(E_FILE_DELETE, filename);
            return 1;
        }
    } else {
        if (unlink(filename) != 0) {
            errorn(E_FILE_DELETE, filename);
            return 1;
        }
    }
    return 0;
}

static
int moist_chmod(const char * filename, mode_t mode) {
    if (chmod(filename, mode) != 0) {
        errorn(E_FILE_ACCESS, filename);
        return 1;
    }
    return 0;
}

static
int moist_chown(const char * filename, const char * owner, const char * group) {
    uid_t uid = -1;
    gid_t gid = -1;

    struct passwd * usr = getpwnam(owner);
    if (!usr) {
        errorn(E_NO_USER, owner);
        return 1;
    }
    uid = usr->pw_uid;

    struct group * grp = getgrnam(group);
    if (!grp) {
        errorn(E_NO_GROUP, group);
        return 1;
    }
    gid = grp->gr_gid;

    if (chown(filename, uid, gid)) {
        errorn(E_FILE_CHOWN, filename);
        return 1;
    }

    return 0;
}

static
int moist_move(const char * filename, const char * newname) {
    if (rename(filename, newname) != 0) {
        errorn(E_FILE_MOVE, filename, newname);
        return 1;
    }
    return 0;
}

static
int moist_copy(const char * filename, const char * newname) {
    // Is using system for copying terrible? yes.
    // Do I have know a better solution thats not filled with footguns? no.
    size_t cmd_len = strlen("cp -a")
                   + sizeof(' ') + strlen(filename)
                   + sizeof(' ') + strlen(newname)
                   + 1
    ;
    char cmd[cmd_len];

    snprintf(cmd, cmd_len, "cp -a %s %s", filename, newname);

    int result = system(cmd);
    if (result == 127
    ||  result == -1
    || (WIFEXITED(result) && WEXITSTATUS(result) != 0)) {
        errorn(E_FILE_COPY, filename, newname);
        return 1;
    }

    return 0;
}

static
move_data_t moist_mytempmove(const char * filename, const char * newname) {
    move_data_t r = {
        .orig_name = NULL,
        .curt_name = NULL,
        .dest_name = NULL,
    };

    const int COLISION_DIGITS = 3;
    const size_t buf_size = strlen(filename) + COLISION_DIGITS + sizeof("~");
    char buffer[buf_size];

    unsigned n = 0;
    do {
        snprintf(buffer, buf_size, "%s~%d", filename, n++);
        if (n > 10 * COLISION_DIGITS) { goto end; }
    } while (!access(buffer, F_OK));

    if (mymove(filename, buffer)) { goto end; }

    r.orig_name = strdup(filename);
    r.curt_name = strdup(buffer);
    r.dest_name = strdup(newname);

  end:
    return r;
}
