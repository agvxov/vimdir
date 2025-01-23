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
