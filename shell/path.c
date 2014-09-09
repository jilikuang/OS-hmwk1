/* Implementation of path */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

struct path_s {
	char *path;
	struct path_s *next;
};

struct path_s *path_head = NULL;
struct path_s *path_cur = NULL;

char *path_filename = NULL;
int path_filename_size = 0;

char *path_get_first_path(void)
{
	path_cur = path_head;

	if (path_cur == NULL)
		return NULL;

	return path_cur->path;
}

char *path_get_next_path(void)
{
	struct path_s *tmp = path_cur;

	if (tmp == NULL)
		return NULL;
	else if (tmp->next == NULL)
		return NULL;

	path_cur = tmp->next;

	return path_cur->path;
}

int path_search_path(const char *path)
{
	int found = 0;
	struct path_s *tmp = path_head;

	while (tmp != NULL) {
		if (strcmp(tmp->path, path) == 0) {
			found = 1;
			path_cur = tmp;
			break;
		}

		tmp = tmp->next;
	}

	return found;
}

int path_push_path(const char *path)
{
	int rval = 0;
	struct path_s *tmp = NULL;
	int len = strlen(path);

	if (path_search_path(path)) {
		printf("error: Path has already existed\n");
		return -1;
	}

	tmp = (struct path_s *)malloc(sizeof(struct path_s));

	if (tmp == NULL) {
		printf("error: Path node allocation failed\n");
		rval = -1;
	} else {
		tmp->path = (char *)malloc((len+1)*sizeof(char));
		if (tmp->path == NULL) {
			printf("error: Path storage allocation failed\n");
			free(tmp);
			rval = -1;
		} else {
			strcpy(tmp->path, path);
			if (path_head == NULL)
				tmp->next = NULL;
			else
				tmp->next = path_head;
			path_head = tmp;
		}
	}

	return rval;
}

int path_delete_path(const char *path)
{
	int rval = -1;
	struct path_s *tmp = path_head;
	struct path_s *prev = NULL;

	while (tmp != NULL) {
		if (strcmp(tmp->path, path) == 0) {
			if (prev == NULL)
				path_head = tmp->next;
			else
				prev->next = tmp->next;
			path_cur = tmp->next;
			free(tmp->path);
			free(tmp);
			rval = 0;
			break;
		}

		prev = tmp;
		tmp = tmp->next;
	}

	if (rval < 0)
		printf("error: Cannot find the path to delete\n");

	return rval;
}

static char *get_full_filepath(const char *path, const char *filename)
{
	int file_len = strlen(filename);
	int path_len = strlen(path);
	int target_size = 0;

	target_size = path_len + file_len + 2;
	if (target_size > path_filename_size) {
		char *tmp = (char *)realloc(path_filename, target_size);

		if (tmp == NULL) {
			printf("error: filename path reallocation failed\n");
			return NULL;
		}

		path_filename = tmp;
		path_filename_size = target_size;
	}

	strcpy(path_filename, path);
	strcat(path_filename, "/");
	strcat(path_filename, filename);

	return path_filename;
}

char *path_check_file_exist(const char *filename)
{
	int rval = 0;
	char *tmp_path = NULL;
	char *chk_name = NULL;
	struct stat fstat;

	chk_name = get_full_filepath("", filename);
	rval = stat(chk_name, &fstat);
	if (rval == 0)
		return chk_name;

	tmp_path = path_get_first_path();

	while (tmp_path) {
		chk_name = get_full_filepath(tmp_path, filename);
		rval = stat(chk_name, &fstat);
		if (rval == 0)
			break;
		tmp_path = path_get_next_path();
	}

	if (rval < 0)
		return NULL;

	return chk_name;
}

void path_terminate(void)
{
	struct path_s *tmp = path_head;
	struct path_s *next = NULL;

	while (tmp != NULL) {
		next = tmp->next;
		free(tmp->path);
		free(tmp);
		tmp = next;
	}

	if (path_filename)
		free(path_filename);
}
