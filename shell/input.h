/* Header of input command handling */

#define INPUT_TOKEN_NUM_MAX (64)

struct cmd_seg_s {
	char *cmd;
	unsigned int size;
	char *token_arr[INPUT_TOKEN_NUM_MAX];
	unsigned int act_token_num;
	struct cmd_seg_s *next;
};

struct cmd_seg_s *input_get_first_cmd_seg(void);
struct cmd_seg_s *input_get_cur_cmd_seg(void);
struct cmd_seg_s *input_get_next_cmd_seg(void);
unsigned int input_get_cmd_seg_num(void);
int input_extract_cmd_seg(char *input);
int input_extract_cmd_token(struct cmd_seg_s *cmd_seg);
void input_terminate(void);
