/* Implementation of path */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct path_s {
	char *path;
	struct path_s *next;
};

static struct path_s *path_head = NULL;
static struct path_s *path_cur = NULL;

char* path_get_first_path(void)
{
	path_cur = path_head;

	if (path_cur == NULL)
		return NULL;

	return path_cur->path;
}

char* path_get_next_path(void)
{
	struct path_s *tmp = path_cur;

	if (tmp == NULL)
		return NULL;
	else if (tmp->next == NULL)
		return NULL;
	else
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

	tmp = (struct path_s*)malloc(sizeof(struct path_s));

	if (tmp == NULL) {
		printf("error: Path node allocation failed\n");
		rval = -1;
	} else {
		tmp->path = (char*)malloc((len+1)*sizeof(char));
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
}
