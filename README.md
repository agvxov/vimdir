# Vimdir
> VIDIR iMproved

## FEATUES
[X] touching
[X] renaming
[X] deletion
[X] change file permissions
[X] change owner
[ ] swapping
[ ] specify the deletion method (so trash can be supported)
[X] display directories with a trailing `/`
[X] recursion
[X] dryrun
[X] use ${VIMDIREDITOR}


## OPTIONS
**-h**
: Print help and exit.

**-n**
: Dry run.
Do not modify any files, only print what would have happened.

**-p**
: Edit permissions.
Permissions will be prepended in human readable form (similar to ls) to each entry.
Modifying this field results in a (non-recursive) chmod.

**-o**
: Edit owner.
The owner and group will be prepended in human readable form to each entry.
Modifying this field results in a (non-recursive) chown.

**-r**
: Run recursively.
Each child folder's contents will be listed with their full path.
Might slow things down as the whole document has to be constructed
before opening the editor is possible.
Deleting a directory will delete all of its children.
Deleting a directory and modifying a child afterwards will throw an error.
Modifying a child and deleting the parent in some line underneath is valid.

## ENTRIES
The abstract name of the format Vimdir operates on is "entry".
Each entry takes up exactly one line.
Vimdir will output an entry corresponding to every file
and will process every entry as a command (nop included).

The following is the format of an entry. The concrete syntax is command line flag dependent.

    <id> [permissions] [<owner>:group] <name>


## ENVIRONMENT

**VIMDIREDITOR**
Preferred editor to use.

**EDITOR**
Editor to use.

**VIMDIRRM**
`rm` alternative to use.
Set this to an appropriate executable to have move-to-trash behaviour on delete.

## NOTES
The default editor is *Vi*.

Vimdir will terminate at the first sign of an error,
but it has no way to roll-back your filesystem,
so be careful.
