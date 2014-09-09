/* Implementation of W4118 Shell */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>

#include "path.h"

#ifdef SHELL_DEBUG
#define DBGMSG printf
#else
#define DBGMSG(...)
#endif

char *input_buf = NULL;
unsigned int input_size = 0;

#define INPUT_PIPE_NUM_UNIT	(8)

char **input_pipe_arr = NULL;
unsigned int input_pipe_num = INPUT_PIPE_NUM_UNIT;

#define INPUT_TOKEN_NUM_UNIT	(128)

char *input_token_arr[INPUT_TOKEN_NUM_UNIT];

inline void print_errno(void)
{
	printf("error: %s\n", strerror(errno));
}

int search_input_token(char *input, const char *token, char **output_arr, unsigned int output_arr_num)
{
	int i = 0;
	char *tmp = NULL;

	*output_arr = strtok(input, token);
	DBGMSG("Input token 0: %s\n", *output_arr);

	i++;

	do {
		*(output_arr+i) = strtok(NULL, token);
		if (*(output_arr+i) == NULL) {
			DBGMSG("No more token\n");
			break;
		}
		DBGMSG("Input token %d: %s\n", i, *(output_arr+i));

		i++;
	} while (i < output_arr_num);

	tmp = strtok(NULL, token);
	if (tmp != NULL)
		printf("error: Too many arguments. They will be ignored\n");

	return i;
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
#ifdef SHELL_DEBUG
		int i = 0;

		DBGMSG("Child exec %s\n", fexec);
		for (i = 0; i < INPUT_TOKEN_NUM_UNIT; i++)
			printf("arg[%d]: %s\n", i, argv[i]);
#endif
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
	int pipe_num = 0, token_num = 0;
	int i = 0;

#ifdef SHELL_DEBUG
	printf("argc: %d\n", argc);
	for (i = 0; i < argc; i++)
		printf("argv[%d]: %s\n", i, argv[i]);
#endif

	DBGMSG("Allocate pipe number: %d\n", input_pipe_num);
	DBGMSG("Allocate pipe size: %d\n", input_pipe_num * sizeof(char *));
	input_pipe_arr = (char **)malloc(input_pipe_num * sizeof(char *));

	if (input_pipe_arr == NULL) {
		printf("error: Input pipe array allocation failed! Shell terminates\n");
		goto __main_exit;
	}
	DBGMSG("Input pipe array: 0x%X\n", (unsigned int)input_pipe_arr);

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

		pipe_num = search_input_token(input_buf, "|", input_pipe_arr, input_pipe_num);
		DBGMSG("Input pipe_num: %d\n", pipe_num);

		for (i = 0; i < pipe_num; i++) {
			token_num = search_input_token(input_pipe_arr[i], " ", input_token_arr, INPUT_TOKEN_NUM_UNIT-1);
			DBGMSG("Input token number: %d\n", token_num);
#ifdef SHELL_DEBUG
			{
				int j = 0;

				for (j = 0; j < INPUT_TOKEN_NUM_UNIT; j++)
					printf("token %d: %s\n", j, input_token_arr[j]);
			}
#endif
		}

		if (strcmp(input_token_arr[0], "exit") == 0) {
			if (token_num > 1) {
				printf("w4118_sh: ");
				printf("exit don't need any arguments\n");
			} else {
				break;
			}
		} else if (strcmp(input_token_arr[0], "cd") == 0) {
			if (token_num > 2) {
				printf("w4118_sh: Too many arguments for cd\n");
			} else {
				rval = exec_cd(input_token_arr[1]);
				if (rval < 0) {
					printf("w4118_sh: ");
					printf("Invalid directory path\n");
				}
			}
		} else if (strcmp(input_token_arr[0], "path") == 0) {
			if (token_num > 3) {
				printf("w4118_sh: ");
				printf("Too many arguments for path\n");
			} else {
				rval = exec_path(input_token_arr[1], input_token_arr[2]);
			}
		} else {
			rval = exec_program(*input_token_arr, input_token_arr);
		}
	}

__main_exit:
	if (input_buf)
		free(input_buf);

	if (input_pipe_arr != NULL) {
		DBGMSG("Free input pipe array: 0x%X\n", (unsigned int)input_pipe_arr);
		free(input_pipe_arr);
	}

	path_terminate();

	return 0;
}
