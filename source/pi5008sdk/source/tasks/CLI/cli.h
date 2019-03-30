#ifdef SUPPORT_DEBUG_CLI
#ifndef __CLI_H__
#define __CLI_H__

#include <CLI/cli_serial_io.h>

/**
 * @brief IO input routine - change it accordingly to your implementation
 */
#define CLI_IO_INPUT(__data) serial_getc((unsigned char *)__data)
	

/**
 * @brief IO output routine - change it accordingly to your implementation
 */
#define CLI_IO_OUTPUT(__data, __len) serial_putc((unsigned char *)__data, __len)


/**
 * @brief key code for backspace
 */
//#define KEY_CODE_BACKSPACE 0x7f
#define KEY_CODE_BACKSPACE 0x08

/**
 * @brief key code for delete key
 */
#define KEY_CODE_DELETE 0x7e

/**
 * @brief key code for enter key
 */
//#define KEY_CODE_ENTER 0x0a
#define KEY_CODE_ENTER 0x0d

/**
 * @brief key code for escape key
 */
#define KEY_CODE_ESCAPE 0x1b

/**
* @brief command line argc num (defines the maximum command length)
*/
#define CLI_CMD_MAX_ARGC 10


/**
* @brief command line buffer size (defines the maximum command length)
*/
//#define CLI_CMD_BUFFER_SIZE 40
#define CLI_CMD_BUFFER_SIZE 64


/**
* @brief how many history items to maintain
*/
#define CLI_CMD_HISTORY_LEN 8


/**
* @brief how long a multi-code sequence can be
*/
#define MULTICODE_INPUT_MAX_LEN 5


/**
 * @brief command interpretation result
 */
typedef enum _t_cmd_status {
	E_CMD_OK = 0,
	E_CMD_NOT_FOUND,
	E_CMD_EMPTY
} t_cmd_status;


/**
 * @brief CLI command
 */
typedef struct _t_cmd {
	const char *cmd;
	void (*fh)(void*);
} t_cmd;


/**
 * @brief multicode input
 */
typedef struct _t_multicode_input {
	unsigned char buf[MULTICODE_INPUT_MAX_LEN];
	unsigned char pos;
} t_multicode_input;


/**
 * @brief multicode command
 */
typedef struct _t_multicode_cmd {
	unsigned char pattern[MULTICODE_INPUT_MAX_LEN];
	unsigned char len;
	void (*fh)(void*);
} t_multicode_cmd; 


struct cmd_struct {

	const char *cmd;
	int (*fh)(int argc, const char **argv);
	struct cmd_struct *next;
	const char *usage;
};

/**
* @brief CLI system context structure
*/
typedef struct _t_cli_ctx {
	// command set
	struct cmd_struct *cmds;
	t_multicode_cmd *mcmds;

	// cmd
	char cmd[CLI_CMD_BUFFER_SIZE];
	unsigned char cpos;
	unsigned char argc;

	// history
	char history[CLI_CMD_HISTORY_LEN][CLI_CMD_BUFFER_SIZE];
	unsigned char hpos;
	unsigned char hhead, htail;

	// multicode input
	t_multicode_input mc;
} t_cli_ctx;
	

void cli_read(t_cli_ctx *a_ctx);
void cli_add_cmd(struct cmd_struct *cmd);
void cli_add_cmds(struct cmd_struct *cmds, int len);

//////////////////////////////////////////////////////////////////////////////////////////

PP_VOID vTaskCLI(PP_VOID *pvData);

#endif /* __CLI_H__ */
#endif /* SUPPORT_DEBUG_CLI */