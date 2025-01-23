" Vim syntax file
" Language: Vimdir entry file
" Current Maintainer: Anon
" Last Change: 2025

let b:current_syntax = "vimdir"

syn match vdId          "^\d\+"
syn match vdSeparator   ":"
syn match vdPermissions "[-dcbpls][-rwx]\{9}"
syn match vdDirectory     "[^\t]\+/"
" NOTE: tabs are used for field separation,
"        however it is possible that the user has expandtab on.
"       this would confuse the lexer, leading to unexpected results.
"       therefor, it is a good idea to warn the user.
"       that said, spaces are valid inside file names.
syn match vdSpace       " "

hi link vdId            Number
hi link vdSeparator     Statement
hi link vdPermissions   Special
hi link vdSpace         Error
hi link vdDirectory     Directory

" #placeholder<ls_colors> COLLAPSED
