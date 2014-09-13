/* Implementation of input command handling */

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "defines.h"
#include "input.h"

struct cmd_seg_s *cmd_seg_head = NULL;
struct cmd_seg_s *cmd_seg_prev = NULL;
struct cmd_seg_s *cmd_seg_cur = NULL;
unsigned int cmd_seg_act_num = 0;

static int insert_cmd_seg_node(char *cmd, struct cmd_seg_s *node)
{
	int rval = 0;
	int size = strlen(cmd) + 1;
	struct cmd_seg_s *tmp = NULL;

	tmp = (struct cmd_seg_s *)malloc(sizeof(struct cmd_seg_s));
	if (tmp == NULL) {
		printf("error: New node creationg failed\n");
		rval = -1;
	} else {
		tmp->cmd = (char *)malloc(size * sizeof(char));
		if (tmp->cmd == NULL) {
			printf("error: New node cmd allocation failed\n");
			free(tmp);
			rval = -1;
		}
		strcpy(tmp->cmd, cmd);
		tmp->size = size * sizeof(char);
		tmp->next = NULL;
	}

	if (rval == 0) {
		if (node == NULL) {
			if (cmd_seg_head == NULL) {
				cmd_seg_head = tmp;
				cmd_seg_prev = NULL;
				cmd_seg_cur = cmd_seg_head;
			} else {
				printf("error: Cannot insert node into head\n");
				free(tmp->cmd);
				free(tmp);
				rval = -1;
			}
		} else {
			tmp->next = node->next;
			node->next = tmp;
			cmd_seg_prev = node;
			cmd_seg_cur = tmp;
		}
	}

	return rval;
}

static int update_cmd_seg_node(char *cmd, struct cmd_seg_s *node)
{
	int rval = 0;
	int size = strlen(cmd) + 1;

	if (node == NULL) {
		printf("error: node must exist for update\n");
		return -1;
	}

	if (size > node->size) {
		char *tmp_cmd = NULL;

		tmp_cmd = realloc(node->cmd, size);
		if (tmp_cmd == NULL) {
			printf("error: node cmd update failed\n");
			rval = -1;
		} else {
			node->cmd = tmp_cmd;
			node->size = size;
		}
	}

	if (rval == 0)
		strcpy(node->cmd, cmd);

	return rval;
}

struct cmd_seg_s *input_get_first_cmd_seg(void)
{
	cmd_seg_cur = cmd_seg_head;
	cmd_seg_prev = NULL;
	return cmd_seg_cur;
}

struct cmd_seg_s *input_get_cur_cmd_seg(void)
{
	return cmd_seg_cur;
}

struct cmd_seg_s *input_get_next_cmd_seg(void)
{
	/* Don't move if the last */
	if (cmd_seg_cur->next == NULL)
		return NULL;

	cmd_seg_prev = cmd_seg_cur;
	cmd_seg_cur = cmd_seg_cur->next;
	return cmd_seg_cur;
}

unsigned int input_get_cmd_seg_num(void)
{
	return cmd_seg_act_num;
}

int input_extract_cmd_seg(char *input)
{
	int rval = 0;
	char *tmp_str = NULL;
	struct cmd_seg_s *tmp_node = NULL;

	tmp_node = input_get_first_cmd_seg();
	if (tmp_node == NULL) {
		/* create an empty head node first */
		rval = insert_cmd_seg_node("", NULL);
		if (rval < 0)
			return rval;
	}

	tmp_node = cmd_seg_head;
	cmd_seg_act_num = 0;

	tmp_str = strtok(input, "|");
	if (tmp_str == NULL)
		printf("error: Syntax error in pipe usage\n");
	else
		while (tmp_str != NULL) {
			DBGMSG("'|' found %s\n", tmp_str);
			if (tmp_node == NULL) {
				tmp_node = input_get_cur_cmd_seg();
				rval = insert_cmd_seg_node(tmp_str, tmp_node);
			} else {
				rval = update_cmd_seg_node(tmp_str, tmp_node);
			}
			if (rval < 0) {
				cmd_seg_act_num = 0;
				break;
			}
			cmd_seg_act_num++;
			tmp_node = input_get_next_cmd_seg();
			tmp_str = strtok(NULL, "|");
		}

	return rval;
}

int input_extract_cmd_token(struct cmd_seg_s *cmd_seg)
{
	int rval = 0;
	int i = 0;
	char *tmp_str = NULL;

	tmp_str = strtok(cmd_seg->cmd, " ");

	for (i = 0; i < (INPUT_TOKEN_NUM_MAX - 1); i++) {
		if (tmp_str == NULL)
			break;
		DBGMSG("' ' found %s\n", tmp_str);
		cmd_seg->token_arr[i] = tmp_str;
		tmp_str = strtok(NULL, " ");
	}

	cmd_seg->act_token_num = i;
	cmd_seg->token_arr[i] = NULL;
	if (tmp_str != NULL)
		printf("error: Too many arguments lead to truncation\n");

	return rval;
}


void input_terminate(void)
{
	struct cmd_seg_s *tmp = NULL;
	struct cmd_seg_s *next = NULL;

	tmp = cmd_seg_head;
	while (tmp != NULL) {
		next = tmp->next;
		free(tmp->cmd);
		free(tmp);
		tmp = next;
	}
}
