# TODO
- [ ] fix failing test
- [ ] sigsegv when trying to create a new folder (new file number, trailing slash)
- [ ] for vim in particular, the working dir should be the tree root
- [ ] allow for files with tab and newline by displaying them as already having record separator characters in their names
- [ ] bump dictate, move it into library/
- [ ] bump kvec, move it into library/
- [ ] complain to the C committee that nodiscard cannot be applied to function pointers
- [ ] path_cmp should account for duplicate slashes and inline ./

## Ideas
Accept multiple folders and individual files.

Accept input from stdin.
Could possibly be useful for big data.
Entries should not be (re)ordered.

Maybe do away with the default `rm` and require VIMDIRRM to be set
(while having some special value to signal `remove_all()`)?
