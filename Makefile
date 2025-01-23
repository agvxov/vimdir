SOURCE := main.c opts.c directive.c file_utils.c error.c dictate.c
SOURCE := $(addprefix source/, ${SOURCE})

CFLAGS := -O0

OUT := vimdir

${OUT}: ${SOURCE}
	${CC} ${CFLAGS} -o ${OUT} ${SOURCE} -ggdb

test:
	cmdtest --fast

ls_colors:
	tclsh script/gen_syntax_from_ls_colors.tcl > object/ls_colors.vimdir
	plug -g -e ls_colors object/ls_colors.vimdir .vim/syntax/vimdir.vim

bundle:
	tar -c .vim/ -f vimdir.tar

install: bundle
	tar -x -f vimdir.tar --dereference -C ~/

clean:
	-rm ${OUT}

.PHONY: test
