# TODO
- [X] dryrun
- [X] `NO_COLOR` / color
- [X] recursion
- [X] display directories with a trailing `/`
- [X] specify the deletion method (so trash can be supported)
- [X] touching / mkdir
- [X] renaming
- [X] swapping
- [X] deletion
- [X] change file permissions
- [X] change owner
- [X] copying
- [X] use `${VIMDIREDITOR}`
- [X] dry and moist tests
- [X] remove tempfile
- [X] path ordering that accounts for `-` (recursion; common prefix)

## Ideas
Accept multiple folders and individual files.

Accept input from stdin.
Could possibly be useful for big data.
Entries should not be (re)ordered.

Maybe do away with the default `rm` and require VIMDIRRM to be set
(while having some special value to signal `remove_all()`)?
