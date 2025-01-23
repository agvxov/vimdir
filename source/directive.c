#include "directive.h"

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

#include "kvec.h"
#include "global.h"
#include "error.h"
#include "file_utils.h"

typedef struct {
    char * name;
    struct stat st;
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

    char full_path[1024];
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
        if (e == -1) {
            errorn(E_FILE_ACCESS, full_path);
            return 1;
        }

        entry = (entry_t) {
            .name = strdup(full_path),
            .st   = file_stat,
        };
        kv_push(entry_t, entries, entry);

        if (is_recursive
        && (kv_A(entries, entries.n-1).st.st_mode & S_IFDIR)) {
            kv_push(const char*, directory_queue, kv_A(entries, entries.n-1).name);
        }
    }

    closedir(dir);
}

int init_directive_c(const char * folder) {
    init_file_utils(is_dry_run);
    kv_init(entries);
    kv_init(directory_queue);

    kv_push(const char*, directory_queue, folder);
    while (directory_queue.n) {
        add_directory(kv_pop(directory_queue));
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
            struct passwd * pw = getpwuid(entry->st.st_uid);
            struct group  * gr = getgrgid(entry->st.st_gid);

            fprintf(f, "\t%s:%s",
                pw->pw_name,
                gr->gr_name
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
    char line[LINE_SIZE];
    char buffer[1024];
    char * sp;
    int expected_id = 0;
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
          // deletion
        while (expected_id != id) {
            const char * last_filename = kv_A(entries, expected_id).name;
            mydelete(last_filename);
            ++expected_id;
        }
        ++expected_id;
        NEXT_FIELD;

        entry_t * entry = &kv_A(entries, id);

        // Permission
        if (do_permissions) {
            mode_t mode;
            sscanf(sp, "%s\t", buffer);
            mode = str_to_mode(buffer);

            if (entry->st.st_mode != mode) {
                mychmod(entry->name, mode);
            }
            NEXT_FIELD;
        }

        // Owner
        if (do_owner) {
            char buffer2[113];
            sscanf(sp, "%s:%s\t", buffer, buffer2);
            //mychown(filename, buffer, buffer2);
            NEXT_FIELD;
        }

        // Name
        sscanf(sp, "%s\n", buffer);
        size_t len = strlen(buffer);
        if (buffer[len-1] == '/') {
            buffer[len-1] = '\0';
        }

        if (strcmp(entry->name, buffer)) {
            mymove(entry->name, buffer);
        }
    }
}
