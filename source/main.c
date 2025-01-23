#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "error.h"
#include "opts.h"
#include "directive.h"

char * editor = NULL;
char * folder = NULL;
char * custom_rm = NULL;
bool is_dry_run     = false;
bool is_recursive   = false;
bool do_permissions = false;
bool do_owner       = false;

#define DEBUG

int edit(const char * filename) {
    size_t cmd_len = strlen(editor) + sizeof(' ') + strlen(filename) + 1;
    char cmd[cmd_len];

    snprintf(cmd, cmd_len, "%s %s", editor, filename);

    // XXX
    int result = system(cmd);
    if (result == -1) {
        errorn(E_OPEN_EDITOR, editor);
    } else
    if (WIFEXITED(result)
    &&  WEXITSTATUS(result) != 0) {
        return 1;
    }

    return 0;
}

void clean_up(void) {
    free(folder);
    deinit_directive_c();
}

signed main(int argc, char * * argv) {
    #define CHECK(x) do { \
            if ((r = x)) { \
                goto end; \
            } \
        } while (0)
    int r = 0;

    get_env();
    parse_args(argc, argv);

    // XXX: what if the user passed '/'?
    size_t len = strlen(folder);
    if (folder[len-1] == '/') {
        folder[len-1] = '\0';
    }

    char * tmpfile_name;
    FILE * tmpfile;
  #ifdef DEBUG
    tmpfile_name = "vimdir_test_file.vimdir";
  #else
    tmpfile_name = mktemp("/tmp/vidirXXXXXX.vimdir");
  #endif
    tmpfile = fopen(tmpfile_name, "w+");
    CHECK_OPEN(tmpfile, tmpfile_name, goto end);

    CHECK(init_directive_c(folder));

    CHECK(make_directive_file(tmpfile));

    fflush(tmpfile);

    CHECK(edit(tmpfile_name));

    fclose(tmpfile);
    tmpfile = fopen(tmpfile_name, "r");
    CHECK_OPEN(tmpfile, tmpfile_name, goto end);

    CHECK(execute_directive_file(tmpfile));

  end:
    if (tmpfile) {
        fclose(tmpfile);
    }
    clean_up();
    #undef CHECK

    return r;
}
