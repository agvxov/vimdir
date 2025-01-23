#include "opts.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "global.h"

void usage() {
    puts("vimdir <path>");
    puts("  -h : print help");
    puts("  -n : dry run; do not modify files, just print operations");
    puts("  -p : allow for editing permissions");
    puts("  -o : allow for editing owner/group");
    puts("  -r : recursive");
}

void get_env(void) {
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
            default: {
                fprintf(stderr, "Unknown option: -%c\n", optopt);
                usage();
                exit(1);
            } break;
        }
    }

    if (optind < argc) {
        folder = strdup(argv[optind]);
    } else {
        folder = strdup(".");
    }
}
