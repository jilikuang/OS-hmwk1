/* Implementation of W4118 Shell */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>

#include "path.h"
#include "input.h"

#ifdef SHELL_DEBUG
#define DBGMSG printf
#else
#define DBGMSG(...)
#endif

char *input_buf = NULL;
unsigned int input_size = 0;

inline void print_errno(void)
{
	printf("error: %s\n", strerror(errno));
}

int exec_cd(const char *path)
{
	int rval = 0;

	DBGMSG("Current dir (before): %s\n", (char *)get_current_dir_name());
	rval = chdir(path);
	if (rval < 0)
		print_errno();
	DBGMSG("Current dir (after): %s\n", (char *)get_current_dir_name());

	return rval;
}

int exec_program(const char *file, char **argv)
{
	int rval = 0;
	int pid = 0;
	char *fexec = NULL;

	fexec = path_check_file_exist(file);

	if (fexec == NULL) {
		printf("w4118_sh: Command not found - %s\n", file);
		return -1;
	}

	pid = fork();
	if (pid == 0) {
		rval = execv(fexec, argv);
		if (rval < 0)
			print_errno();
		DBGMSG("Child process exit\n");
		exit(EXIT_SUCCESS);
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

int exec_path(const char *mode, const char *path)
{
	int rval = 0;
	char *tmp_path = NULL;

	if (mode == NULL) {
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
		if ((strcmp(mode, "+") == 0) && (path != NULL)) {
			rval = path_push_path(path);
		} else if ((strcmp(mode, "-") == 0) && (path != NULL)) {
			rval = path_delete_path(path);
		} else {
			printf("w4118_sh: Unknown operation\n");
			rval = -1;
		}
	}

	return rval;
}

int main(int argc, char **argv)
{
	int rval = 0;
	int pipe_num = 0;
	int i = 0;
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

		cmd_seg = input_get_first_cmd_seg();

		for (i = 0; i < pipe_num; i++) {
			rval = input_extract_cmd_token(cmd_seg);
			cmd_seg = input_get_next_cmd_seg();
		}

		cmd_seg = input_get_first_cmd_seg();

		if (strcmp(cmd_seg->token_arr[0], "exit") == 0) {
			if (cmd_seg->act_token_num > 1) {
				printf("w4118_sh: ");
				printf("exit don't need any arguments\n");
			} else {
				break;
			}
		} else if (strcmp(cmd_seg->token_arr[0], "cd") == 0) {
			if (cmd_seg->act_token_num > 2) {
				printf("w4118_sh: Too many arguments for cd\n");
			} else {
				rval = exec_cd(cmd_seg->token_arr[1]);
				if (rval < 0) {
					printf("w4118_sh: ");
					printf("Invalid directory path\n");
				}
			}
		} else if (strcmp(cmd_seg->token_arr[0], "path") == 0) {
			if (cmd_seg->act_token_num > 3) {
				printf("w4118_sh: ");
				printf("Too many arguments for path\n");
			} else {
				rval = exec_path(cmd_seg->token_arr[1], cmd_seg->token_arr[2]);
			}
		} else {
			rval = exec_program(cmd_seg->token_arr[0], cmd_seg->token_arr);
		}
	}

	if (input_buf)
		free(input_buf);

	path_terminate();
	input_terminate();

	return 0;
}
