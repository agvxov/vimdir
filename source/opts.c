#include "opts.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "global.h"
#include "error.h"
#include "dictate.h"

static
void usage(void) {
    dictate(
        "$B$gvimdir$0 $y[$boptions$y] <$bpath$y>$0\n"
        "  $B$g-h --help$0       : print help\n"
        "  $B$g-n --dry-run$0    : dry run; do not modify files, just print operations\n"
        "  $B$g-p --permission$0 : allow for editing permissions\n"
        "  $B$g-o --owner$0      : allow for editing owner/group\n"
        "  $B$g-r --recursive$0  : recursive"
    #if DEBUG == 1
        "\n$B$rTHIS IS A DEBUG BUILD OF VIMDIR!$0"
    #endif
    );
}

void get_env(void) {
    // NO_COLOR
    char * no_color = getenv("NO_COLOR");
    if (no_color
    &&  no_color[0] != '\0') {
        dictate_color_enabled(false);
    }
    
    // Editor
    editor = getenv("VIMDIREDITOR");
    if (editor
    &&  editor[0] != '\0') {
        goto set_custom_rm;
    }

    editor = getenv("EDITOR");
    if (editor
    &&  editor[0] != '\0') {
        goto set_custom_rm;
    }

    editor = "vi";

    // RM
  set_custom_rm:
    custom_rm = getenv("VIMDIRRM");
    if (custom_rm
    &&  custom_rm[0] == '\0') {
        custom_rm = NULL;
    }

    return;
}

void parse_args(int argc, char * * argv) {
    int opt;

    opterr = 0; // suppress default getopt error messages

    const struct option long_options[] = {
        {"help",       no_argument, 0, 'h'},
        {"dry-run",    no_argument, 0, 'd'},
        {"permission", no_argument, 0, 'p'},
        {"owner",      no_argument, 0, 'o'},
        {"recursive",  no_argument, 0, 'r'},
        {"recurse",    no_argument, 0, 'r'}, // intentionally undocumented
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "hvnpor", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h': {
                usage();
                exit(0);
            } break;
            case 'v': {
                dictate(
                    #include "version.inc"
                );
                exit(0);
            } break;
            case 'n': {
                is_dry_run = true;
            } break;
            case 'p': {
                do_permissions = true;
            } break;
            case 'o': {
                do_owner = true;
            } break;
            case 'r': {
                is_recursive = true;
            } break;
            case '?': {
                errorn(E_FLAG, optopt);
                usage();
                exit(1);
            }
        }
    }

    if (optind < argc) {
        folder = strdup(argv[optind]);
    } else {
        folder = strdup(".");
    }
}
