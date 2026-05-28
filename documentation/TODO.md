# TODO
- [ ] fix failing test
- [ ] sigsegv when trying to create a new folder (new file number, trailing slash)
- [ ] for vim in particular, the working dir should be the tree root

## Ideas
Accept multiple folders and individual files.

Accept input from stdin.
Could possibly be useful for big data.
Entries should not be (re)ordered.

Maybe do away with the default `rm` and require VIMDIRRM to be set
(while having some special value to signal `remove_all()`)?
