/* Implementation of W4118 Shell */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>

#include "defines.h"
#include "path.h"
#include "input.h"

inline void print_errno(void)
{
	printf("error: %s\n", strerror(errno));
}

int exec_cd(struct cmd_seg_s *cmd_seg)
{
	int rval = 0;

	if (cmd_seg->act_token_num > 2)
		printf("error: Too many arguments\n");
	else
		rval = chdir(cmd_seg->token_arr[1]);

	if (rval < 0)
		print_errno();

	return rval;
}

int exec_path(struct cmd_seg_s *cmd_seg)
{
	int rval = 0;
	char *tmp_path = NULL;

	if (cmd_seg->act_token_num > 3) {
		printf("Too many arguments\n");
		return -1;
	}

	if (cmd_seg->act_token_num == 1) {
		printf("PATH=");
		tmp_path = path_get_first_path();
		while (tmp_path != NULL) {
			printf("%s", tmp_path);
			tmp_path = path_get_next_path();
			if (tmp_path != NULL)
				printf(":");
		}
		printf("\n");
	} else {
		char *mode = cmd_seg->token_arr[1];
		char *path = cmd_seg->token_arr[2];

		if (strcmp(mode, "+") == 0) {
			if (path == NULL) {
				printf("error: Illegal path\n");
				rval = -1;
			} else {
				rval = path_push_path(path);
			}
		} else if (strcmp(mode, "-") == 0) {
			if (path == NULL) {
				printf("error: Illegal path\n");
				rval = -1;
			} else {
				rval = path_delete_path(path);
			}
		} else {
			printf("error: Unknown operation\n");
			rval = -1;
		}
	}

	return rval;
}

int exec_program(struct cmd_seg_s *cmd_seg)
{
	int rval = 0;
	int pid = 0;
	char *file = cmd_seg->token_arr[0];
	char *fexec = NULL;

	fexec = path_check_file_exist(file);

	if (fexec == NULL) {
		printf("error: Command not found - %s\n", file);
		return -1;
	}

	pid = fork();
	if (pid == 0) {
		rval = execv(fexec, cmd_seg->token_arr);
		if (rval < 0)
			print_errno();
		/* If exec succeeds, should not get to this point */
		exit(EXIT_FAILURE);
	} else if (pid > 0) {
		int status = 0;

		DBGMSG("Parent wait for child %d\n", pid);
		rval = waitpid(pid, &status, 0);
		DBGMSG("Parent back from child %d: %d\n", rval, status);
	} else {
		print_errno();
	}

	return rval;
}

int main(int argc, char **argv)
{
	int rval = 0;
	char *input_buf = NULL;
	unsigned int input_size = 0;
	int pipe_num = 0;
	struct cmd_seg_s *cmd_seg = NULL;

	while (1) {
		printf("$ ");
		rval = getline(&input_buf, &input_size, stdin);

		if (rval < 0) {
			printf("error: Input buffer allocation failed\n");
			continue;
		} else {
			/* From stdin, The last char will be '\n'.
			   Replace it with '\0'.*/
			input_buf[rval - 1] = '\0';
		}

		if (input_buf[0] == '\0')
			continue;

		rval = input_extract_cmd_seg(input_buf);

		pipe_num = input_get_cmd_seg_num();
		DBGMSG("Input pipe_num: %d\n", pipe_num);

		if (pipe_num == 1) {
			cmd_seg = input_get_first_cmd_seg();
			rval = input_extract_cmd_token(cmd_seg);
			if (rval < 0)
				continue;

			if (strcmp(cmd_seg->token_arr[0], "exit") == 0)
				break;
			else if (strcmp(cmd_seg->token_arr[0], "cd") == 0)
				rval = exec_cd(cmd_seg);
			else if (strcmp(cmd_seg->token_arr[0], "path") == 0)
				rval = exec_path(cmd_seg);
			else
				rval = exec_program(cmd_seg);
		} else {
			/* Handle piped commands */
			printf("Do be done...\n");
		}
	}

	if (input_buf)
		free(input_buf);

	path_terminate();
	input_terminate();

	return 0;
}
