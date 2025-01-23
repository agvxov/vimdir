#include "file_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include "error.h"

int (*mytouch)(const char *filename) = NULL;
int (*mydelete)(const char *filename) = NULL;
int (*mychmod)(const char *filename, mode_t mode) = NULL;
int (*mychown)(const char *filename, const char *owner, const char *group) = NULL;
int (*mymove)(const char *filename, const char *newname) = NULL;

static int dry_touch(const char * filename);
static int dry_delete(const char * filename);
static int dry_chmod(const char * filename, mode_t mode);
static int dry_chown(const char * filename, const char * owner, const char * group);
static int dry_move(const char * filename, const char * newname);

static int moist_touch(const char * filename);
static int moist_delete(const char * filename);
static int moist_chmod(const char * filename, mode_t mode);
static int moist_chown(const char * filename, const char * owner, const char * group);
static int moist_move(const char * filename, const char * newname);

int init_file_utils(bool is_dry_run) {
    if (is_dry_run) {
        mytouch  = dry_touch;
        mydelete = dry_delete;
        mychmod  = dry_chmod;
        mychown  = dry_chown;
        mymove   = dry_move;
    } else {
        //mytouch  = moist_touch;
        //mydelete = moist_delete;
        //mychmod  = moist_chmod;
        //mychown  = moist_chown;
        //mymove   = moist_move;
    }
}

int deinit_file_utis() {
    mytouch  = NULL;
    mydelete = NULL;
    mychmod  = NULL;
    mychown  = NULL;
    mymove   = NULL;
}

char mode_type_to_char(mode_t m) {
    switch (m & S_IFMT) {
        case S_IFREG:  return '-';  // regular file
        case S_IFDIR:  return 'd';  // directory
        case S_IFCHR:  return 'c';  // character device
        case S_IFBLK:  return 'b';  // block device
        case S_IFIFO:  return 'p';  // fifo (pipe)
        case S_IFLNK:  return 'l';  // symbolic link
        case S_IFSOCK: return 's'; // socket
        default: return '?';       // unknown
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
    warning("touch '%s'", filename);
    return 0;
}

static
int dry_delete(const char * filename) {
    warning("delete '%s'", filename);
    return 0;
}

static
int dry_chmod(const char * filename, mode_t mode) {
    char buf[11];
    warning("chmod '%s' (%s)", filename, mode_to_str(mode, buf));
    return 0;
}

static
int dry_chown(const char * filename, const char * owner, const char * group) {
    warning("chown '%s' (%s:%s)", filename, owner, group);
    return 0;
}

static
int dry_move(const char * filename, const char * newname) {
    warning("rename '%s' (-> %s)", filename, newname);
    return 0;
}

// --- Moist implementations
static
int moist_touch(const char * filename) {
    FILE * f = fopen(filename, "w");
    CHECK_OPEN(f, filename, return 1);
    fclose(f);
    return 0;
}

static
int moist_delete(const char * filename) {
    if (unlink(filename) != 0) {
        errorn(E_FILE_DELETE, filename);
        return 1;
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

    struct passwd * pwd = getpwnam(owner);
    if (!pwd) {
        fprintf(stderr, "Error: User '%s' not found\n", owner);
        return 1;
    }
    uid = pwd->pw_uid;

    struct group * grp = getgrnam(group);
    if (!grp) {
        fprintf(stderr, "Error: Group '%s' not found\n", group);
        return 1;
    }
    gid = grp->gr_gid;

    if (chown(filename, uid, gid) != 0) {
        perror("Error changing file ownership");
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
