/* Implementation of W4118 Shell */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef SHELL_DEBUG
#define DBGMSG printf
#else
#define DBGMSG(...)
#endif

#define INPUT_BUF_LEN_UNIT	(1024)

char *input_buf[2] = {NULL, NULL};
unsigned int input_buf_size = INPUT_BUF_LEN_UNIT;
unsigned int input_buf_idx = 0;
char *input_buf_cur = NULL;

inline void print_errno(int errno)
{
	printf("error: %s\n", strerror(errno));
	return;
}

int switch_input_buf(void)
{
	int rval = 0;
	int next_idx = (input_buf_idx+1) & 0x1;
	int next_size = input_buf_size + INPUT_BUF_LEN_UNIT;

	DBGMSG("Try to switch to input buffer %d, size = %d\n", next_idx, next_size);

	input_buf[next_idx] = malloc(next_size);
	if (input_buf[next_idx] == NULL) {
		printf("error: Input buffer switching failed\n");
		rval = -1;
	} else {
		DBGMSG("Input buffer switching succeeded\n");
		strncpy(input_buf[next_idx], input_buf[input_buf_idx], input_buf_size-1);
		free(input_buf[input_buf_idx]);
		input_buf_idx = next_idx;
		input_buf_size = next_size;
		input_buf_cur = input_buf[input_buf_idx];
		DBGMSG("Input swtiched to buffer 0x%X\n", (unsigned int)input_buf_cur);
	}

	return rval;
}

int get_input(char **input_buf_p)
{
	int rval = 0;
	unsigned int c_count;
	char tmp_c;

	printf("$ ");
	c_count = 0;
	do {
		tmp_c = getchar();
		if ((c_count == input_buf_size-1) && tmp_c != '\n') {
			rval = switch_input_buf();
			if (rval < 0) {
				printf("error: Input will be trancated\n");
				c_count++;
				break;
			}
		}
		input_buf_cur[c_count] = tmp_c;
	} while (input_buf_cur[c_count++] != '\n');

	input_buf_cur[c_count-1] = '\0';

	DBGMSG("get_input get input %s\n", input_buf_cur);

	*input_buf_p = input_buf_cur;

	return -1;
}

int main(int argc, char **argv)
{
	char *input = NULL;
#ifdef SHELL_DEBUG
	int i = 0;
	printf("argc: %d\n", argc);
	for (i=0; i<argc; i++)
		printf("argv[%d]: %s\n", i, argv[i]);
#endif

	input_buf[input_buf_idx] = (char*) malloc(input_buf_size);

	if (input_buf[input_buf_idx] == NULL) {
		printf("error: Input buffer allocation failed! Shell terminates\n");
		goto __main_err_exit;
	} else {
		DBGMSG("Input buffer: 0x%X\n", (unsigned int)input_buf[input_buf_idx]);
		input_buf_cur = input_buf[input_buf_idx];
	}

	do {
		get_input(&input);

		if (strcmp(input, "exit") == 0) {
			break;
		} else {
			printf("Cannot find command: %s\n", input);
		}
	} while (1);

	if (input_buf_cur != NULL) {
		DBGMSG("Free input buffer: 0x%X\n", (unsigned int)input_buf_cur);
		free(input_buf_cur);
	}

	return 0;

__main_err_exit:
	return -1;
}
