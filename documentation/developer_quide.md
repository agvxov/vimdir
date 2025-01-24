# Vimdir developer guide

## Files
| Name | Description |
| :--: | :---------: |
| kvec.h | Vector library. |
| dictate.* | IO library. |
| global.h | Global variables from flags and the environment. There are only a few and it shall be kept like that. |
| main.c | Project core. Implements `global.h`. |
| opts.* | Flag and environment handler. Fills up `global.h` |
| error.* | Simplifies and unifies error handling. All error messages are dispatched from within here. |
| directive.* | Responsible for creating parsing entries. |
| file\_utils.* | File operations. This is where `directive.*` delegates commands to. Responsible for respecting dry-runs. |

## LS\_COLORS
Using the `ls_colors` Make target,
we can statically render the vim highlighting script.
Tcl is used to generate the vim commands.
Plug is used to insert it into `.vim/syntax/vimdir.vim`.
`.vim/syntax/vimdir.vim` is version controlled for user convenience.

## Debugging
If the `$DEBUG` environment variable is set to 1,
a clean build will produce a debug version executable of Vimdir.
Debug builds contain DRARF symbols, may output extra info
and use `./vimdir_test_file.vimdir` as a tmpfile.

Tests require `./vimdir_test_file.vimdir`,
therefor they will only succeed on debug builds.
