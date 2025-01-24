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
        "  $B$g-h$0 : print help\n"
        "  $B$g-n$0 : dry run; do not modify files, just print operations\n"
        "  $B$g-p$0 : allow for editing permissions\n"
        "  $B$g-o$0 : allow for editing owner/group\n"
        "  $B$g-r$0 : recursive"
    );
}

void get_env(void) {
  no_color:
    char * no_color = getenv("NO_COLOR");
    if (no_color
    &&  no_color[0] != '\0') {
        dictate_color_enabled(false);
    }
    
  set_editor:
    editor = getenv("VIMDIREDITOR");
    if (editor) { goto set_custom_rm; }

    editor = getenv("EDITOR");
    if (editor) { goto set_custom_rm; }

    editor = "vi";

  set_custom_rm:
    custom_rm = getenv("VIMDIRRM");

  end:
    return;
}

void parse_args(int argc, char * * argv) {
    int opt;

    opterr = 0; // suppress default getopt error messages

    while ((opt = getopt(argc, argv, "hnpor")) != -1) {
        switch (opt) {
            case 'h': {
                usage();
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
