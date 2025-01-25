#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include "error.h"
#include "opts.h"
#include "directive.h"
#include "file_utils.h"

char * editor = NULL;
char * folder = NULL;
char * custom_rm = NULL;
bool is_dry_run     = false;
bool is_recursive   = false;
bool do_permissions = false;
bool do_owner       = false;

int get_tmpfile_name(char * name_buffer) {
  #if DEBUG == 1
    strcpy(name_buffer, "vimdir_test_file.vimdir");

    return 0;
  #else
    strcpy(name_buffer, "/tmp/vidirXXXXXX.vimdir");
    int fd = mkstemps(name_buffer, sizeof(".vimdir")-1);
    if (fd == -1) { return 1; }
    close(fd);

    return 0;
  #endif
}

int edit(const char * filename) {
    size_t cmd_len = strlen(editor) + sizeof(' ') + strlen(filename) + 1;
    char cmd[cmd_len];

    snprintf(cmd, cmd_len, "%s %s", editor, filename);

    int result = system(cmd);
    if (result == 127   // shell could not be executed
    ||  result == -1) { // child process could not be created
        errorn(E_OPEN_EDITOR, editor);
        return 1;
    } else
    if (WIFEXITED(result)
    &&  WEXITSTATUS(result) != 0) {
        errorn(E_IN_EDITOR, WEXITSTATUS(result));
        return 1;
    }

    return 0;
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

    folder = trim_trailing_slashes(folder);

  create:
    FILE * tmpfile;
    char tmpfile_name[32];

    CHECK(get_tmpfile_name(tmpfile_name));

    tmpfile = fopen(tmpfile_name, "w+");
    CHECK_OPEN(tmpfile, tmpfile_name, goto end);

    CHECK(init_directive_c(folder));

    CHECK(make_directive_file(tmpfile));

    fflush(tmpfile);

  edit:
    CHECK(edit(tmpfile_name));

  process:
    fclose(tmpfile);
    tmpfile = fopen(tmpfile_name, "r");
    CHECK_OPEN(tmpfile, tmpfile_name, goto end);

    CHECK(execute_directive_file(tmpfile));

  end:
    if (tmpfile) {
        fclose(tmpfile);
    }
    if (!r) {
        unlink(tmpfile_name);
    } else {
        notice("the location of the preserved directive file is '%s'", tmpfile_name);
    }
    free(folder);
    deinit_directive_c();
    #undef CHECK

    return r;
}
