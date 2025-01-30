#include "directive.h"

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
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

static kvec_t(move_data_t) move_data;



static
int add_directory(const char * const folder) {
    DIR * dir = opendir(folder);
    CHECK_OPEN(dir, folder, return 1);

    char full_path[PATH_MAX];
    struct dirent * mydirent;
    struct stat file_stat;
    entry_t entry;
    while ((mydirent = readdir(dir)) != NULL) {
        if (strcmp(mydirent->d_name, ".")  == 0
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
    init_file_utils(is_dry_run, custom_rm);
    kv_init(entries);
    kv_init(directory_queue);
    kv_init(move_data);

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

    for (int i = 0; i < move_data.n; i++) {
        move_data_t move = kv_A(move_data, i);
        free(move.orig_name);
        free(move.curt_name);
        free(move.dest_name);
    }

    kv_destroy(directory_queue);
    kv_destroy(entries);
    kv_destroy(move_data);
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
    if (*s != '\0') {
        ++s;
    }
    return (char*)s;
}

int execute_directive_file(FILE * f) {
    #define NEXT_FIELD do { \
            if (*(sp = next_field(sp)) == '\0') { \
                errorn(E_FORMAT); \
                goto recovery; \
            } \
        } while (0)
    #define CHECK(x) do { \
        if (x) { goto recovery; } \
    } while (0)
    #define CHECK_FORMAT(n, x) do { \
        if (n != x) { \
            errorn(E_FORMAT); \
            goto recovery; \
        } \
    } while (0)
    /* io buffering
     */
    const int LINE_SIZE = 4096;
    char line[LINE_SIZE];
    char buffer[LINE_SIZE/2];
    /* String Pointer, indexing `line`
     */
    char * sp;
    /* alias reference to the current entry being operated on
     */
    entry_t * entry;
    /* since new files fille be missing from `entries`,
     *  but we only the the latest one,
     *  we buffer it on the stack
     */
    char touch_buffer[LINE_SIZE/2];
    entry_t touch_entry;

    while (fgets(line, LINE_SIZE, f) != NULL) {
        sp = line;

        // ID
        do {
            int id;
            int e = sscanf(line, "%d\t", &id);
            if (e == 1) { // normal entry
                if (id < 0
                ||  id > entries.n) {
                    errorn(E_INDEX, id);
                    goto recovery;
                }

                entry = &kv_A(entries, id);
              
                NEXT_FIELD;
            } else { // creation
                char * const saved_sp = sp;
                // skip to the name
                if (do_permissions) { NEXT_FIELD; }
                if (do_owner)       { NEXT_FIELD; }

                CHECK_FORMAT(1, sscanf(sp, "%s\n", touch_buffer));

                mytouch(touch_buffer);

                struct stat file_stat;
                int es = stat(touch_buffer, &file_stat);
                CHECK_OPEN(!(es == -1), touch_buffer, goto recovery);

                touch_entry = (entry_t) {
                    .name         = touch_buffer,
                    .st           = file_stat,
                    .is_mentioned = false,
                };
                entry = &touch_entry;

                sp = saved_sp;
            }
        } while (0);

        // Copy
        if (entry->is_mentioned) {
            char * const saved_sp = sp;
            // skip to the name
            if (do_permissions) { NEXT_FIELD; }
            if (do_owner)       { NEXT_FIELD; }

            CHECK_FORMAT(1, sscanf(sp, "%s\n", buffer));

            mycopy(entry->name, buffer);

            sp = saved_sp;
        }

        // Permission
        if (do_permissions) {
            mode_t mode;
            CHECK_FORMAT(1, sscanf(sp, "%s\t", buffer));

            mode = str_to_mode(buffer);

            if (entry->st.st_mode != mode) {
                CHECK(mychmod(entry->name, mode));
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
                CHECK(mychown(entry->name, buffer, buffer2));
            }
            NEXT_FIELD;
        }

        // Name (move)
        if (!entry->is_mentioned) {
            CHECK_FORMAT(1, sscanf(sp, "%[^\n]", buffer));
            size_t len = strlen(buffer);
            if (buffer[len-1] == '/') {
                buffer[len-1] = '\0';
            }

            if (strcmp(entry->name, buffer)) {
                if (access(buffer, F_OK)) {
                    mymove(entry->name, buffer);
                } else {
                    move_data_t move = mytempmove(entry->name, buffer);
                    if (!move.orig_name) {
                        errorn(E_FILE_SWAP, entry->name, buffer);
                        goto recovery;
                    }

                    kv_push(move_data_t, move_data, move);
                }
            }
        }
        
        // -- Poke
        entry->is_mentioned = true;
    }

    // Deletion
    for (int i = 0; i < entries.n; i++) {
        entry_t * entry = &kv_A(entries, i);
        if (!entry->is_mentioned) {
            CHECK(mydelete(entry->name));
        }
    }

    // Swap (move)
    for (int i = 0; i < move_data.n; i++) {
        move_data_t move = kv_A(move_data, i);
        // NOTE: we could be overwritting here;
        //        thats the behaviour the user would expect
        int result = mymove(move.curt_name, move.dest_name);
        // on the otherhand, upon error,
        //  you dont want your files replaced
        if (result
        &&  !access(move.orig_name, F_OK)) {
            // the result of this is intentionally unchecked
            mymove(move.curt_name, move.orig_name);
        }
    }

    #undef NEXT_FIELD
    #undef CHECK
    #undef CHECK_FORMAT
    return 0;

  recovery:
    /* If an error is encountered, we wish to leave the filesystem in a "valid" state.
     * Therefor, files waiting to be swapped (possessing a temporary name) are restored back
     *  (if possible, if we run into another error, theres not much to do).
     */
    for (int i = 0; i < move_data.n; i++) {
        move_data_t move = kv_A(move_data, i);
        if (!access(move.orig_name, F_OK)) {
            mymove(move.curt_name, move.orig_name);
        }
    }
    return 1;
}
