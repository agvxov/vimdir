SOURCE := main.c opts.c directive.c file_utils.c error.c dictate.c remove_all.c
SOURCE := $(addprefix source/, ${SOURCE})

CFLAGS += -Wno-unused-label

ifeq (${DEBUG}, 1)
  CFLAGS += -O0 -ggdb -Wall -Wpedantic
  CPPFLAGS += -DDEBUG
else
  CFLAGS += -O3 -flto=auto -fomit-frame-pointer
endif

OUT := vimdir

${OUT}: ${SOURCE}
	${CC} ${CPPFLAGS} ${CFLAGS} -o ${OUT} ${SOURCE}

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
	-rm vimdir.tar

.PHONY: test
