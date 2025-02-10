# Vimdir
> VIDIR iMproved; edit directories and filenames

## SYNOPSIS
**vidir** **[***options***]** **[**directory**]**

## DESCRIPTION
Vimdir allows editing of directories and filenames in a text editor.
If no directory is specified, the filenames of the current directory are edited.

Vimdir is an extended reimplementation of *vidir(1)*.

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

## OPERATIONS
Changing a field will manupulate the file (e.g. renaming, chmod, chown).

Swapping the names of files is allowed and should be handed correctly.

Deleting a line will delete the file.

Duplicating a line and changing the name will copy the file.

## COLORS
Vimdir uses the extension `.vimdir` for entry files,
this can be used to recognize them in your editor.

A Vim syntax file should be packaged with Vimdir.
It is recommended to use it.

If you are interested in generating your highlighting dynamically and based on **$LS_COLORS**,
you might be interested in this:
[https://github.com/trapd00r/vim-syntax-vidir-ls](https://github.com/trapd00r/vim-syntax-vidir-ls).

## ENVIRONMENT

**VIMDIREDITOR**
Preferred editor to use.

**EDITOR**
Editor to use.

**VIMDIRRM**
`rm` alternative to use.
Set this to an appropriate executable to have move-to-trash behaviour on delete.

**NO_COLOR**
According to the no-color convention,
if this variable is set and is not empty,
no colors will be printed.

## NOTES
The default editor is *Vi*.

Vimdir will terminate at the first sign of an error,
but it has no way to roll-back your filesystem,
so be careful.

If Vimdir fails, the directive file is preserved and its path is returned to the user.

You will not be able to manage files which have tab or newline characters in their names.
If you have aforementioned files, please seek the help of a specialist.

The first character of the permission string is simply informative,
its (changed) value is ignored.
