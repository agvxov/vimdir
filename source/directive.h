#ifndef COMMAND_FILE_H
#define COMMAND_FILE_H

#include <stdio.h>

extern int init_directive_c(const char * folder_);
extern int deinit_directive_c(void);
extern int make_directive_file(FILE * f);
extern int execute_directive_file(FILE * f);

#endif
