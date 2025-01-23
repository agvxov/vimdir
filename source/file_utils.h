#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <stdbool.h>
#include <sys/stat.h>

extern int init_file_utils(bool is_dry_run);
extern int deinit_file_utis();

extern char mode_type_to_char(mode_t m);
extern mode_t char_to_mode_type(const char c);
extern char * mode_to_str(mode_t mode, char * buffer);
extern mode_t str_to_mode(const char *permissions);

extern int (*mytouch)(const char *filename);
extern int (*mydelete)(const char *filename);
extern int (*mychmod)(const char *filename, mode_t mode);
extern int (*mychown)(const char *filename, const char *owner, const char *group);
extern int (*mymove)(const char *filename, const char *newname);

#endif
