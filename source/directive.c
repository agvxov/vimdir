#include "directive.h"

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <linux/limits.h>

#include "kvec.h"
#include "global.h"
#include "error.h"
#include "file_utils.h"

typedef struct {
    char * name;
    struct stat st;
    bool is_mentioned;
} entry_t;

static
int entry_cmp(const void * a, const void * b) { // For qsort()
    const entry_t * const A = a;
    const entry_t * const B = b;
    return strcmp(A->name, B->name);
}

static kvec_t(entry_t) entries;
static kvec_t(const char*) directory_queue;



static
int add_directory(const char * const folder) {
    DIR * dir = opendir(folder);
    CHECK_OPEN(dir, folder, return 1);

    char full_path[PATH_MAX];
    struct stat file_stat;
    struct dirent * mydirent;
    entry_t entry;
    while ((mydirent = readdir(dir)) != NULL) {
        if (strcmp(mydirent->d_name, ".") == 0
        ||  strcmp(mydirent->d_name, "..") == 0) {
            continue;
        }

        sprintf(full_path, "%s/%s",
            folder,
            mydirent->d_name
        );

        int e = stat(full_path, &file_stat);
        CHECK_OPEN(!(e == -1), full_path, return 1);

        entry = (entry_t) {
            .name         = strdup(full_path),
            .st           = file_stat,
            .is_mentioned = false,
        };
        kv_push(entry_t, entries, entry);

        if (is_recursive
        && (kv_A(entries, entries.n-1).st.st_mode & S_IFDIR)) {
            kv_push(const char*, directory_queue, kv_A(entries, entries.n-1).name);
        }
    }

    closedir(dir);
    return 0;
}

int init_directive_c(const char * const folder) {
    init_file_utils(is_dry_run);
    kv_init(entries);
    kv_init(directory_queue);

    kv_push(const char*, directory_queue, folder);
    while (directory_queue.n) {
        if (add_directory(kv_pop(directory_queue))) {
            return 1;
        }
    }

    qsort(
        entries.a,
        entries.n,
        sizeof(entry_t),
        entry_cmp
    );

    return 0;
}

int deinit_directive_c(void) {
    for (int i = 0; i < entries.n; i++) {
        free(kv_A(entries, i).name);
    }

    kv_destroy(directory_queue);
    kv_destroy(entries);
    deinit_file_utis();

    return 0;
}

int make_directive_file(FILE * f) {
    for (int i = 0; i < entries.n; i++) {
        entry_t * entry = &kv_A(entries, i);
        // ID
        fprintf(f, "%03d",
            i
        );
        // Permissions
        if (do_permissions) {
            char permissions[11];

            fprintf(f, "\t%s",
                mode_to_str(entry->st.st_mode, permissions)
            );
        }
        // Owner
        if (do_owner) {
            struct passwd * usr = getpwuid(entry->st.st_uid);
            struct group  * grp = getgrgid(entry->st.st_gid);

            fprintf(f, "\t%s:%s",
                usr->pw_name,
                grp->gr_name
            );
        }
        // Name
        fprintf(f, "\t%s",
            entry->name
        );
        // if Directory
        if (entry->st.st_mode & S_IFDIR) {
            putc('/', f);
        }

        putc('\n', f);
    }

    return 0;
}

static inline
char * next_field(const char * s) {
    while (*s != '\t'
       &&  *s != '\n'
       &&  *s != '\0') {
       ++s;
    }
    return (char*)(++s);
}

int execute_directive_file(FILE * f) {
    #define NEXT_FIELD do { \
            if (*(sp = next_field(sp)) == '\0') { \
                errorn(E_FORMAT); \
                return 1; \
            } \
        } while (0)
    const int LINE_SIZE = 1024;
    #define CHECK_FORMAT(n, x) do { \
        if (n != x) { \
            errorn(E_FORMAT); \
            return 1; \
        } \
    } while (0)
    char line[LINE_SIZE];
    char buffer[1024];
    char * sp;
    int id;

    while (fgets(line, LINE_SIZE, f) != NULL) {
        sp = line;

        // ID
        int e = sscanf(line, "%d\t", &id);
          // creation
        if (e != 1) {
            sscanf(sp, "%s\n", buffer);
            mytouch(buffer);
            continue;
        }
        NEXT_FIELD;

        entry_t * entry = &kv_A(entries, id);
        entry->is_mentioned = true;

        // Permission
        if (do_permissions) {
            mode_t mode;
            CHECK_FORMAT(1, sscanf(sp, "%s\t", buffer));

            mode = str_to_mode(buffer);

            if (entry->st.st_mode != mode) {
                mychmod(entry->name, mode);
            }
            NEXT_FIELD;
        }

        // Owner
        if (do_owner) {
            char buffer2[113];
            CHECK_FORMAT(2, sscanf(sp, "%[^:]:%s\t", buffer, buffer2));

            struct passwd * usr = getpwuid(entry->st.st_uid);
            struct group  * grp = getgrgid(entry->st.st_gid);

            if (strcmp(usr->pw_name, buffer)
            ||  strcmp(grp->gr_name, buffer2)) {
                mychown(entry->name, buffer, buffer2);
            }
            NEXT_FIELD;
        }

        // Name
        CHECK_FORMAT(1, sscanf(sp, "%s\n", buffer));
        size_t len = strlen(buffer);
        if (buffer[len-1] == '/') {
            buffer[len-1] = '\0';
        }

        if (strcmp(entry->name, buffer)) {
            mymove(entry->name, buffer);
        }
    }

    // Deletion
    for (int i = 0; i < entries.n; i++) {
        entry_t * entry = &kv_A(entries, i);
        if (!entry->is_mentioned) {
            mydelete(entry->name);
        }
    }

    #undef NEXT_FIELD
    return 0;
}
