# Vimdir developer guide

## Files
| Name | Description |
| :--: | :---------: |
| kvec.h | Vector library. |
| dictate.* | IO library. |
| remove\_all.* | Recursive remove library. |
| global.h | Global variables from flags and the environment. There are only a few and it shall be kept like that. |
| main.c | Project core. Implements `global.h`. |
| opts.* | Flag and environment handler. Fills up `global.h` |
| error.* | Simplifies and unifies error handling. All error messages are dispatched from within here. |
| directive.* | Responsible for creating parsing entries. |
| file\_utils.* | File operations. This is where `directive.*` delegates commands to. Responsible for respecting dry-runs. |

## Errors
As mentioned,
`error.*` are responsible for all the actual reporting.

All errors which are reported are fatal.

The only errors which are not fatal are those
which occur during a shutdown caused by another error.

Exception like in the sense that they continuously propagate upward.
This is not an architectural nightmare only because
the maximum stack frame number within the code base is very small.
While the codebase has seemingly no direction to bloat to,
if such thing were to happen,
C++ (maybe Objective-C or some dialect)
should be adapted (for true exceptions).

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
