/* Implementation of W4118 Shell */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef SHELL_DEBUG
#define DBGMSG printf
#else
#define DBGMSG(...)
#endif

#define INPUT_BUF_SIZE_UNIT	(1024)

char *input_buf[2] = {NULL, NULL};
unsigned int input_buf_size = INPUT_BUF_SIZE_UNIT;
unsigned int input_buf_idx = 0;
char *input_buf_cur = NULL;

#define INPUT_TOKEN_NUM_UNIT	(8)

char **input_token_arr = NULL;
unsigned int input_token_num = INPUT_TOKEN_NUM_UNIT;

inline void print_errno(int errno)
{
	printf("error: %s\n", strerror(errno));
	return;
}

int switch_input_buf(void)
{
	int rval = 0;
	unsigned int next_idx = (input_buf_idx+1) & 0x1;
	unsigned int next_size = input_buf_size + INPUT_BUF_SIZE_UNIT;

	DBGMSG("Try to switch to input buffer %d, size = %d\n", \
			next_idx, next_size);

	input_buf[next_idx] = malloc(next_size);
	if (input_buf[next_idx] == NULL) {
		printf("error: Input buffer switching failed\n");
		rval = -1;
	} else {
		DBGMSG("Input buffer switching succeeded\n");
		strncpy(input_buf[next_idx], \
				input_buf[input_buf_idx], \
				input_buf_size-1);
		free(input_buf[input_buf_idx]);
		input_buf_idx = next_idx;
		input_buf_size = next_size;
		input_buf_cur = input_buf[input_buf_idx];
		DBGMSG("Input swtiched to buffer 0x%X\n", \
				(unsigned int)input_buf_cur);
	}

	return rval;
}

int get_input(char **input_buf_p)
{
	int rval = 0;
	unsigned int c_count;
	char tmp_c;

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

int search_input_token(char *input)
{
	int i = 0;

	input_token_arr[0] = strtok(input, " ");
	DBGMSG("Input token 0: %s\n", input_token_arr[0]);

	i++;

	do {
		input_token_arr[i] = strtok(NULL, " ");
		if (input_token_arr[i] == NULL) {
			DBGMSG("No more token\n");
			break;
		}
		DBGMSG("Input token %d: %s\n", i, input_token_arr[i]);

		i++;
	} while (i < input_token_num);

	return i;
}

int main(int argc, char **argv)
{
	char *input = NULL;
	int token_num = 0;
#ifdef SHELL_DEBUG
	int i = 0;
	printf("argc: %d\n", argc);
	for (i=0; i<argc; i++)
		printf("argv[%d]: %s\n", i, argv[i]);
#endif

	DBGMSG("Allocate buffer char: %d\n", input_buf_size);
	DBGMSG("Allocate buffer size: %d\n", input_buf_size * sizeof(char));
	input_buf[input_buf_idx] = (char*)malloc(input_buf_size * sizeof(char));

	if (input_buf[input_buf_idx] == NULL) {
		printf("error: Input buffer allocation failed! \
				Shell terminates\n");
		goto __main_exit;
	} else {
		DBGMSG("Input buffer: 0x%X\n", \
				(unsigned int)input_buf[input_buf_idx]);
		input_buf_cur = input_buf[input_buf_idx];
	}

	DBGMSG("Allocate token number: %d\n", input_token_num);
	DBGMSG("Allocate token size: %d\n", input_token_num * sizeof(char*));
	input_token_arr = (char**)malloc(input_token_num * sizeof(char*));

	if (input_token_arr == NULL) {
		printf("error: Input token array allocation failed! \
				Shell terminates\n");
		goto __main_exit;
	}
	DBGMSG("Input token array: 0x%X\n", \
			(unsigned int)input_token_arr);

	do {
		printf("$ ");
		get_input(&input);

		if (input[0] == '\0')
			continue;

		token_num = search_input_token(input);
		DBGMSG("Input token number: %d\n", token_num);
#ifdef SHELL_DEBUG
		{
			int i = 0;
			for (i=0; i<token_num; i++) {
				printf("token %d: %s\n", i, input_token_arr[i]);
			}
		}
#endif

		if (strcmp(input_token_arr[0], "exit") == 0) {
			if (token_num > 1)
				printf("exit don't need any arguments\n");
			else
				break;
		} else {
			printf("Cannot find command: %s\n", input_token_arr[0]);		}
	} while (1);

__main_exit:
	if (input_buf_cur != NULL) {
		DBGMSG("Free input buffer: 0x%X\n", \
				(unsigned int)input_buf_cur);
		free(input_buf_cur);
	}

	if (input_token_arr != NULL) {
		DBGMSG("Free input token array: 0x%X\n", \
				(unsigned int)input_token_arr);
		free(input_token_arr);
	}

	return 0;
}
