/* Header of path */

extern char* path_get_first_path(void);
extern char* path_get_next_path(void);
extern int path_search_path(const char *path);
extern int path_push_path(const char *path);
extern int path_delete_path(const char *path);
extern char* path_check_file_exist(const char *filename);
extern void path_terminate(void);
