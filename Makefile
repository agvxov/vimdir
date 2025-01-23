SOURCE := main.c opts.c directive.c file_utils.c error.c dictate.c
SOURCE := $(addprefix source/, ${SOURCE})

CFLAGS := -O0

OUT := vimdir

${OUT}: ${SOURCE}
	${CC} ${CFLAGS} -o ${OUT} ${SOURCE} -ggdb

test:
	cmdtest --fast

clean:
	-rm ${OUT}

.PHONY: test
