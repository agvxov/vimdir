#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <stdbool.h>
#include <sys/stat.h>

extern int init_file_utils(bool is_dry_run, const char * custom_rm_);
extern int deinit_file_utis();

extern char * trim_trailing_slashes(char * path);

extern char mode_type_to_char(mode_t m);
extern mode_t char_to_mode_type(const char c);
extern char * mode_to_str(mode_t mode, char * buffer);
extern mode_t str_to_mode(const char *permissions);

/*`mytouch` actually performs mkdir duty too,
 *  if the last character of its argument is '/'
 */
extern int (*mytouch)(const char * filename);
extern int (*mydelete)(const char * filename);
extern int (*mychmod)(const char * filename, mode_t mode);
extern int (*mychown)(const char * filename, const char * owner, const char * group);
extern int (*mymove)(const char * filename, const char * newname);
extern int (*mycopy)(const char * filename, const char * newname);

/* Swapping file names is only possible with an intermediate rename,
 *  and it also means that we can't headlessly replace files.
 * For this reason we buffer failing renames until
 *  the rest of the file operations are completed.
 * However, we have the following concerns:
 *  1) we don't want to move the file out of its directory,
 *      so that in case of an error,
 *      its not lost *somewhere* on the filesystem
 *  2) a simple and predictable name is preferable,
 *      because if an error is encountered,
 *      we want the user to easily recognize his files
 *  3) the temp name might already be taken
 * Points 1 and 2 cause 3 to exists.
 * To deal with 3, we use the tactic employed by the original vidir:
 *  + we try incrementing suffixes until something works or we get bored
 */
typedef struct {
    char * orig_name;
    char * curt_name;
    char * dest_name;
} move_data_t;

extern move_data_t (*mytempmove)(const char * filename, const char * newname);

#endif
