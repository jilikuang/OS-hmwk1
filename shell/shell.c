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
		printf("error: Too many arguments\n");
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
				printf("error: Empty path\n");
				rval = -1;
			} else {
				rval = path_push_path(path);
			}
		} else if (strcmp(mode, "-") == 0) {
			if (path == NULL) {
				printf("error: Empty path\n");
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

static int is_selfdefcmd(char *cmd)
{
	int i = 0;
	static char * const selfdefcmd[3] = {
		"exit",
		"cd",
		"path"
	};

	for (i = 0; i < 3; i++) {
		if (strcmp(selfdefcmd[i], cmd) == 0)
			break;
	}

	if (i == 3)
		return 0;
	return 1;
}

int do_piped_commands(int cmd_seg_num)
{
	int rval = 0;
	int i = 0, j = 0;
	struct cmd_seg_s *cmd_seg = NULL;
	char *fexec = NULL;
	int *cmdpipe = NULL;
	int *pid_arr = NULL;

	for (i = 0; i < cmd_seg_num; i++) {
		if (i == 0)
			cmd_seg = input_get_first_cmd_seg();
		else
			cmd_seg = input_get_next_cmd_seg();
		rval = input_extract_cmd_token(cmd_seg);
		fexec = path_check_file_exist(cmd_seg->token_arr[0]);
		if (fexec == NULL) {
			printf("error: Command not found - %s\n",
					cmd_seg->token_arr[0]);
			return -1;
		}
		if (is_selfdefcmd(cmd_seg->token_arr[0])) {
			printf("error: Don't use non-program command - %s\n",
					cmd_seg->token_arr[0]);
			return -1;
		}
		DBGMSG("cmd seg %d: %s\n", i, cmd_seg->token_arr[0]);
	}

	cmdpipe = (int *)realloc(cmdpipe, 2 * (cmd_seg_num - 1) * sizeof(int));
	pid_arr = (int *)realloc(pid_arr, cmd_seg_num * sizeof(int));

	for (i = 0; i < (cmd_seg_num - 1); i++) {
		rval = pipe(cmdpipe + (2 * i));
		DBGMSG("pipe[%d] = (%d, %d)\n", i,
				*(cmdpipe + (2 * i)),
				*(cmdpipe + (2 * i) + 1));
	}

	for (i = 0; i < cmd_seg_num; i++) {
		pid_arr[i] = fork();
		if (pid_arr[i] == 0) {
			cmd_seg = input_get_first_cmd_seg();
			for (j = 0; j < i; j++)
				cmd_seg = input_get_next_cmd_seg();
			DBGMSG("This is child %d %d\n", i, getpid());
			DBGMSG("cmd: %s\n", cmd_seg->token_arr[0]);
			break;
		} else if (pid_arr[i] < 0) {
			print_errno();
			rval = -1;
			goto __exit;
		}
	}

	if (i == cmd_seg_num) {
		/* The parent */
		for (j = 0; j < 2*(cmd_seg_num - 1); j++)
			if (close(*(cmdpipe+j)) < 0)
				print_errno();
		DBGMSG("Parent wait %d\n", pid_arr[i-1]);
		waitpid(pid_arr[i-1], NULL, 0);
		DBGMSG("All child is done\n");
	} else {
		if (i == 0) {
			/* The first child */
			DBGMSG("%d: c %d / w %d\n", i,
					*(cmdpipe),
					*(cmdpipe+1));
			if (dup2(*(cmdpipe+1), 1) < 0)
				print_errno();
			for (j = 0; j < 2*(cmd_seg_num - 1); j++)
				if (close(*(cmdpipe+j)) < 0)
					print_errno();
		} else if (i == (cmd_seg_num - 1)) {
			/* The last child */
			DBGMSG("%d: r %d / c %d\n", i,
					*(cmdpipe+(2*(i-1))),
					*(cmdpipe+(2*(i-1)+1)));
			if (dup2(*(cmdpipe+(2*(i-1))), 0) < 0)
				print_errno();
			for (j = 0; j < 2*(cmd_seg_num - 1); j++)
				if (close(*(cmdpipe+j)) < 0)
					print_errno();
		} else {
			/* The middle child */
			DBGMSG("%d: r %d / w %d\n", i,
					*(cmdpipe+(2*(i-1))),
					*(cmdpipe+(2*i)+1));
			if (dup2(*(cmdpipe+(2*(i-1))), 0) < 0)
				print_errno();
			if (dup2(*(cmdpipe+(2*i)+1), 1) < 0)
				print_errno();
			for (j = 0; j < 2*(cmd_seg_num - 1); j++)
				if (close(*(cmdpipe+j)) < 0)
					print_errno();
		}
		fexec = path_check_file_exist(cmd_seg->token_arr[0]);
		DBGMSG("exec %s\n", fexec);
		if (execv(fexec, cmd_seg->token_arr) < 0)
			print_errno();
	}
__exit:

	free(cmdpipe);
	free(pid_arr);

	return rval;
}

int main(int argc, char **argv)
{
	int rval = 0;
	char *input_buf = NULL;
	unsigned int input_size = 0;
	int cmd_seg_num = 0;
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

		cmd_seg_num = input_get_cmd_seg_num();
		DBGMSG("Input cmd_seg_num: %d\n", cmd_seg_num);

		if (cmd_seg_num == 1) {
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
			rval = do_piped_commands(cmd_seg_num);
		}
	}

	if (input_buf)
		free(input_buf);

	path_terminate();
	input_terminate();

	return 0;
}
