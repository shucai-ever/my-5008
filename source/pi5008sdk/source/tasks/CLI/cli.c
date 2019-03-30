#ifdef SUPPORT_DEBUG_CLI
#include <type.h>
#include <stdio.h>
#include <ctype.h>

#include "debug.h"
#include "osal.h"

#include "task_manager.h"

#include "CLI/cli.h"
#include "CLI/cli_func.h"

//#include "app_task_config.h"
#include "api_FAT_FTL.h"

// Test mode : auto viewmode change after booting
#undef TEST_AUTO_VIEWMODE_CHG

struct cmd_struct *cmds;

#define POSINC(__x) (((__x) < (CLI_CMD_BUFFER_SIZE - 1)) ? (__x + 1) : (__x))
#define POSDEC(__x) ((__x) ? ((__x) - 1) : 0)

/* ================================================================================ */

static void _cli_prompt(t_cli_ctx *a_ctx __attribute__((unused)), unsigned char nl);
static unsigned char _cli_count_arguments(t_cli_ctx *a_ctx);
static unsigned char _cli_interpret_cmd(t_cli_ctx *a_ctx);
static void _cli_reinterpret_cmd(t_cli_ctx *a_ctx, unsigned char a_response);
static void _cli_history_add(t_cli_ctx *a_ctx);
static void _cli_history_up(void* a_ctx);
static void _cli_history_down(void* a_ctx);
static void _cli_delete(void *a_data);
static void _cli_none(void *a_data __attribute__((unused)));

static int do_help(int argc, const char **argv);

/* ================================================================================ */

/**
 * @brief global definition of multi-code commands
 */
static t_multicode_cmd _g_mc_cmds[] = {

    { { 0x1b, 0x5b, 0x33, 0x7E }, 4, _cli_delete },
    { { 0x1b, 0x5b, 0x43 }, 3, _cli_none },

    { { 0x1b, 0x5b, 0x41 }, 3, _cli_history_up },
    { { 0x1b, 0x5b, 0x42 }, 3, _cli_history_down },

    // null
    { {0x00}, 0, 0x00}
};

static struct cmd_struct pi5008_cmds[] =
{       
    { .cmd = "help", 		.fh = do_help,		.next = (void*)0, .usage = ": usage ; help string" },

    { .cmd = "test", 		.fh = _test, 		.next = (void*)0, .usage = "test quad register set" },
    { .cmd = "pireg", 		.fh = _pireg,		.next = (void*)0, .usage = "[base|r|w] [hex address|offset] [hex count|data] ; r/w address = base + offset. '<'=>\"pireg r 10 <2 1\" "},
    { .cmd = "pvirx_test", 		.fh = _pvirx_test,	.next = (void*)0, .usage = "[chanAddr] [test item] ; pvi test function\r\n\
              chanAddr: 0~3, 4(all)\r\n\
              test: hdelay(hd), hactive(ha), vdelay(vd), vactive(va),contrast(co), brightness(br), saturation(sa), hue(hu), \r\n\
                    sharpness(sh), HPFcorning(hp), demodulationLPF(de), all" },
    { .cmd = "pvirx_mode", 		.fh = _pvirx_mode, 	.next = (void*)0, .usage = "[ch:0~3] [standard] [camresol] [vidresol]; set pvi rx mode. \r\n\
              standard: 0(cvbs), 1(PVI), 2(CVI), 3(HDA), 4(HDT_OLD), 5(HDT_NEW)\r\n\
              camresol: 0(ntsc), 1(pal), 2(720p60), 3(720p50), 4(720p30), 5(720p25),\r\n\
                        6(1080p30), 7(1080p25), 8(960p30), 9(960p25) \r\n\
              vidresol: 0(720x480i), 1(720x576i), 2(960x480i), 3(960x576i),\r\n\
                        4(720p60), 5(720p50), 6(720p30), 7(720p25), 8(1080p30), 9(1080p25), 10(960p30), 11(960p25)" },
    { .cmd = "pvirx_utcsend", 	.fh = _pvirx_utcsend, 	.next = (void*)0, .usage = "[chanAddr] [utc command] ; command:S(SEL), U(up), R(right), D(down), L(left)" },
    { .cmd = "pvitx_test", 		.fh = _pvitx_test,	.next = (void*)0, .usage = "[test item] ; pvi test function\r\n\
              test item: agcgain_minmax(am), agcgain_onoff(ao), agcgain_sync(as)\r\n\
                         chromagain_tracking(ct), hpll_hollingrange(hh), hda_vsync_change(hv), all" },
    { .cmd = "pvitx_mode", 		.fh = _pvitx_mode, 	.next = (void*)0, .usage = "[standard] [resol] (pattern) ; set pvi mode. pattern(0~15) \r\n\
              standard: 0(sd720), 1(sd960), 2(PVI), 3(HDA), 4(CVI), 5(HDT)\r\n\
              resol: 0(720x480i), 1(720x576i), 2(960x480i), 3(960x576i),\r\n\
                     4(720p60), 5(720p50), 6(720p30), 7(720p25), 8(960p30), 9(960p25), 10(1080p30), 11(1080p25)" },
    { .cmd = "vidinsel", 		.fh = _vidinsel, 	.next = (void*)0, .usage = "[vid_ch] [path_ch] [in_ch] ; vid:0~3, path:0(para),1(rec),2(mipi),3(pvi), in:0~3" },
    { .cmd = "svminsel", 		.fh = _svminsel, 	.next = (void*)0, .usage = "[svm_ch] [path_ch] [in_ch] ; svm:0~3, path:0(video),1(isp),2(quad),3(pattern), in:0~3" },
    { .cmd = "recinsel", 		.fh = _recinsel, 	.next = (void*)0, .usage = "[rec_ch] [path_ch] [in_ch] ; rec:0~3, path:0(video),1(quad/pb/tp), in: 0~3 / 0(pb),1(quad),2(tp)" },
    { .cmd = "recpath", 		.fh = _recpath, 	.next = (void*)0, .usage = "[8|16] [1|2|4]; 8/16bit, mux1,2,4channel" },
	{ .cmd = "quadview", 		.fh = _setQuadView, .next = (void*)0, .usage = "[ch]; ch0~3 full,4(pb full),5(Quad)" },
    { .cmd = "caminsel", 		.fh = _caminsel, 	.next = (void*)0, .usage = "[cam_ch0] [cam_ch1] [cam_ch2] [cam_ch3]; cam:1~4, 0:NG" },
    { .cmd = "capture", 		.fh = _capture, 	.next = (void*)0, .usage = "[ch] [bYonly] ([sd file]); ch0~3,4(pb),5(TP) yonly(0:uyvy,1:yonly), sd file:sd write filename" },
    { .cmd = "capscl", 		.fh = _capscl, 	.next = (void*)0, .usage = "; scale down capture test" },
    { .cmd = "monpvi", 		.fh = _monitor_pvi, 	.next = (void*)0, .usage = "; monitoring pvi status." },
    { .cmd = "adc", 		.fh = _adc,	 	.next = (void*)0, .usage = "[en|dis|read]; read adc ch0~ch3." },
    { .cmd = "genlock", 		.fh = _genlock, 	.next = (void*)0, .usage = "[en|dis|refch]; gen lock sync" },
    { .cmd = "vpuctl", 		.fh = _vpuctl, 		.next = (void*)0, .usage = "[start|stop|draw|mon|dump|dma2d2d|dma1d2d|dma2d1d|rle|evendma|odddma|hamd]; control vpu. \"[start] [svm|vin]\"" },
    { .cmd = "bgtest", 		.fh = _bgtest, 		.next = (void*)0, .usage = "[1(start)|0(stop)] [tasknum]; background test. no argu: print list. if tasknum, do it only." },
    { .cmd = "alivetask", 	.fh = _alivetask, 	.next = (void*)0, .usage = ";check alive task." },
    { .cmd = "proc", 		.fh = _proc, 		.next = (void*)0, .usage = "[irq|device]; print information." },
    { .cmd = "event", 		.fh = _event, 		.next = (void*)0, .usage = "[event]; test send event." },
    { .cmd = "fwdn", 		.fh = _fwdn, 		.next = (void*)0, .usage = "[all|main] [file_name]; fwdn flash update." },
    { .cmd = "remokey", 	.fh = _remoconkey, 	.next = (void*)0, .usage = "; test remocon key emulating." },
	{ .cmd = "sdfs", 		.fh = _sdfs, 		.next = (void*)0, .usage = "[mount|umount]; sdcard mount or unmount.\r\n\t"
																			"[ls] [/path]; show file list.\r\n\t"
																			"[read|write] [filename] [addr] [size]; read/write file.\r\n\t"
																			"[fattest]do test - warning!!! all data will be removed." },
	{ .cmd = "flash", 		.fh = _flashop, 	.next = (void*)0, .usage = 	"[init] [nor|nand]; initialize\r\n\t"
																			"[erase] [flash addr] [size]; flash erase.\r\n\t"
																			"[readdma|read|write] [flash addr] [dram addr] [size], flash readdma/read/write\r\n\t"
																			"[readspare|writespare] [block offset] [page offset] [dram addr] [size]; read nand flash spare area\r\n\t"
																			"[readcache] [column addr] [dram addr] [size]; read nand flash cache.\r\n\t"
																			"[getfeature] [addr]; show nand flash feature.\r\n\t"
																			"[setfeature] [addr] [value]; set nand flash feature.\r\n\t"
																			"" },
	{ .cmd = "ipc", 		.fh = _ipctest, 	.next = (void*)0, .usage = 	"[setprn|setdata|camst]; set print/data memory\r\n\t"
																			"[bc] [on/off]; set brightness control on/off\r\n\t"
																			"[camcon] [cnt]; \r\n\t"
	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 },
	{ .cmd = "gpio", 		.fh = _gpio, 	    .next = (void*)0, .usage = "; test " },
	{ .cmd = "top", 		.fh = _top, 	    .next = (void*)0, .usage = "; print status of running task, system" },
	{ .cmd = "dumpreg", 	.fh = _dumpreg, 	.next = (void*)0, .usage = "; dump & print register value." },
#ifdef DB_LIB_USE
	{ .cmd = "od", 	        .fh = _testOD, 	    .next = (void*)0, .usage = "[init|osd|fast|test|exit|task [on|off]]; test ObjectDetect" },
#endif // DB_LIB_USE
	{ .cmd = "viewgen", 	.fh = _viewgen, 	.next = (void*)0, .usage = "; calibration task viewgen" },
	{ .cmd = "reset", 		.fh = _reset, 		.next = (void*)0, .usage = "; reset whole system" },
	{ .cmd = "nand", 		.fh = _nandtest, 	.next = (void*)0, .usage = "[init] [device type]; nand flash init. 0: gd1g2g, 1: mt29fxg01\r\n\t"
																			"[close]; nand flash close.\r\n\t"
																			"[erase] [block offset] [#blocks]; nand flash erase block.\r\n\t"
																			"[write_pattern] [block offset] [page offset]; nand flash write pattern.\r\n\t"
																			"[read|write] [block offset] [page offset] [#pages] [dram addr]; nand flash read/write pages"
																			"[readpart] [block offset] [page offset] [option] [dram addr]; nand read part. option 0: 1st half, 1: 2nd half, 2: data, 3: spare\r\n\t"
																			"[checkbad] [block offset] [#blocks]; Check factory marked bad block" },
//	{ .cmd = "ftl", 		.fh = _ftl,		 	.next = (void*)0, .usage = "[init] init ftl.\r\n\t"
//																			"[write_sector] [sector] [#sectors] [dram addr]; nand ftl write sector(s).\r\n\t"
//																			"[read_sector] [sector] [#sectors] [dram addr]; nand ftl read sector(s)"
//																			"[info]; nand ftl show info" },
	{ .cmd = "ftl", 		.fh = _ftl,		 	.next = (void*)0, .usage = "[init]; init ftl.\r\n\t"
																			"[write] [flash addr] [size] [dram addr]; nand ftl write.\r\n\t"
																			"[read] [flash addr] [size] [dram addr]; nand ftl read." },
	{ .cmd = "dramtest", 	.fh = _dramest, 	.next = (void*)0, .usage = "; dram speed test" },
	{ .cmd = "cmp", 		.fh = _compare, 	.next = (void*)0, .usage = "; [addr1] [addr2] [size] compare memory" },
	{ .cmd = "spi", 		.fh = _spitest, 	.next = (void*)0, .usage = "[m(master)|s(slave)] [tx|rx|xfer] [size] [loop cnt]; spi transfer test" },
	{ .cmd = "calib", 		.fh = _calib, 	.next = (void*)0, .usage = "; start calibration test code" },
	{ .cmd = "i2c_test",	.fh = _i2c_test,	.next = (void*)0, .usage = "; i2c master/slave loop test" },
	{ .cmd = "cache_test",	.fh = _cache_test,	.next = (void*)0, .usage = "; cache test" },
	{ .cmd = "isp_test",	.fh = _isp_test,	.next = (void*)0, .usage = "; isp test" },
	{ .cmd = "wboot",		.fh = _wboot_test,	.next = (void*)0, .usage = "; warm boot test" },
	{ .cmd = "pd",			.fh = _pd_test,		.next = (void*)0, .usage = "; power down test" },
#ifdef BD_SLT
	{ .cmd = "slt_test",	.fh = _slt_test,	.next = (void*)0, .usage = "[r|w|mipi]; slt test" },
#endif // BD_SLT
	{ .cmd = "viewmode",	.fh = _viewmode,	.next = (void*)0, .usage = "; viewmode [view number]" },	
	//{ .cmd = "core1_test",	.fh = _core1_test,	.next = (void*)0, .usage = "; core1 test" },
	{ .cmd = "i2creg",	    .fh = _i2c_reg,	    .next = (void*)0, .usage = "; i2creg [setup|read|write] ; setup [7bitSlvAd] [ch(0|1)], read/write [addr] [cnt|data]" },	
    { .cmd = NULL, 			.fh = NULL, 		.next = (void*)0, .usage = NULL},
};

/* ================================================================================ */
void cli_add_cmd(struct cmd_struct *cmd)
{
    cmd->next = cmds;
    cmds = cmd;
}

void cli_add_cmds(struct cmd_struct *cmds, int len)
{
    int i;

    for (i = 0; i < len; i++)
    {
        cli_add_cmd(cmds++);
    }
}


void cli_read(t_cli_ctx *a_ctx) {

    unsigned char data = 0x00;
    unsigned char res = E_CMD_OK;

    // if no character available - then exit
    if (!CLI_IO_INPUT(&data)) return;

    /// multi-code matching
    if (a_ctx->mc.pos && (a_ctx->mc.pos < MULTICODE_INPUT_MAX_LEN)) {
        unsigned char mi = 0x00;

        a_ctx->mc.buf[a_ctx->mc.pos++] = data;
        while (a_ctx->mcmds[mi].fh) {
            if (!memcmp(a_ctx->mcmds[mi].pattern, 
                        a_ctx->mc.buf,
                        a_ctx->mcmds[mi].len)) {
                a_ctx->mcmds[mi].fh((void *)a_ctx);
                a_ctx->mc.pos = 0;
                memset(a_ctx->mc.buf, 0x00, MULTICODE_INPUT_MAX_LEN);
                break;
            }
            mi++;
        }
        return;
    }

    /// char by char matching
    switch(data) {
        case KEY_CODE_BACKSPACE: // backspace
        case KEY_CODE_DELETE: // del
            _cli_delete((void *)a_ctx);
            break;

        case KEY_CODE_ESCAPE: // special characters
            if (a_ctx->mcmds) {
                a_ctx->mc.pos = 1;
                a_ctx->mc.buf[0x00] = data;
            }
            break;

        case KEY_CODE_ENTER: // new line
            a_ctx->cmd[POSINC(a_ctx->cpos)] = '\0';
            CLI_IO_OUTPUT("\r\n", 2);
            res = _cli_interpret_cmd(a_ctx);
            _cli_reinterpret_cmd(a_ctx, res);

            a_ctx->cpos = 0;
            memset(a_ctx->cmd, 0x00, CLI_CMD_BUFFER_SIZE);
            _cli_prompt(a_ctx, 1);
            break;

        default:
            /* echo */
            if (a_ctx->cpos < (CLI_CMD_BUFFER_SIZE - 1) && isprint(data)) {
                a_ctx->cmd[a_ctx->cpos++] = data;
                CLI_IO_OUTPUT(&data, 1);
            }
            break;
    }
}

/* ================================================================================ */

static void _cli_prompt(t_cli_ctx *a_ctx __attribute__((unused)), unsigned char nl) {
    if (nl) CLI_IO_OUTPUT("\r\n", 2);
    CLI_IO_OUTPUT("#> ", 3);
}


static unsigned char _cli_count_arguments(t_cli_ctx *a_ctx) {
    char *cur = a_ctx->cmd;
    unsigned char cnt = 0;
    for(;;) {
        while (*cur == ' ') cur++;
        if (*cur == '\0') break;
        cnt++;
        while (*cur != '\0' && *cur != ' ') {
            cur++;
        }
    }
    return cnt;
}

const char *gArgv[CLI_CMD_MAX_ARGC];

static unsigned char _cli_interpret_cmd(t_cli_ctx *a_ctx) {
    unsigned char i = 0;
    unsigned char ret = E_CMD_OK;
    int argcNum = 0;
    char *pArgv;
    char *cur;
    unsigned char cnt = 0;

    if (!strlen(a_ctx->cmd)) {
        return E_CMD_EMPTY;
    }

    while (a_ctx->cmds[i].fh) {
        if (!strncmp(a_ctx->cmds[i].cmd, a_ctx->cmd, strlen(a_ctx->cmds[i].cmd))) {

            a_ctx->argc = _cli_count_arguments(a_ctx);

            if(a_ctx->argc < CLI_CMD_MAX_ARGC)
            {
                cur = (char *)a_ctx->cmd;
                cnt = 0;
                argcNum = 0;

                for(;;) {
                    while (*cur == ' ') cur++;
                    pArgv = (char *)&gArgv[argcNum][0];
                    if (*cur == '\0') break;

                    cnt++;

                    while (*cur != '\0' && *cur != ' ') {
                        *pArgv = *cur;
                        cur++;
                        pArgv++;
                    }

                    *pArgv = '\0';
                    argcNum++;
                }
            }

            // call the handler
            a_ctx->cmds[i].fh(argcNum, (const char **)gArgv);
            break;
        }
        i++;
    }

    if (!a_ctx->cmds[i].fh) {
        ret = E_CMD_NOT_FOUND;
    }

    return ret;
}


static void _cli_reinterpret_cmd(t_cli_ctx *a_ctx, unsigned char a_response) {
    switch(a_response) {
        case E_CMD_NOT_FOUND: {
                                  char str[] = "\r\nCommand not found";
                                  CLI_IO_OUTPUT(str, strlen(str));
                                  _cli_history_add(a_ctx);
                              }
                              break;

        default:
                              _cli_history_add(a_ctx);
                              break;
    } // switch
}


static void _cli_history_add(t_cli_ctx *a_ctx) {	

    if (a_ctx->hhead == CLI_CMD_HISTORY_LEN &&
            a_ctx->htail == CLI_CMD_HISTORY_LEN) {
        a_ctx->hhead = 0;
        a_ctx->htail = 0;
    }
    else {
        a_ctx->hhead++;
        a_ctx->hhead %= CLI_CMD_HISTORY_LEN;
        a_ctx->hpos = a_ctx->hhead;

        if (a_ctx->htail >= a_ctx->hhead) {
            a_ctx->htail = (a_ctx->hhead + 1) % CLI_CMD_HISTORY_LEN;
        }
    }

    memset(a_ctx->history[a_ctx->hhead], 0x00, CLI_CMD_BUFFER_SIZE);
    strcpy(a_ctx->history[a_ctx->hhead], a_ctx->cmd);
}


static void _cli_history_up(void* a_ctx) {

    t_cli_ctx *ctx = (t_cli_ctx *)a_ctx;
    char prompt[12] = {0x00};

    if ((ctx->hhead != ctx->htail) && 
            strlen(ctx->history[ctx->hpos])) {
        memset(ctx->cmd, 0x00, CLI_CMD_BUFFER_SIZE);

        strcpy(ctx->cmd, ctx->history[ctx->hpos]);
        ctx->cpos = strlen(ctx->cmd);

        if (!ctx->hpos)
            ctx->hpos = CLI_CMD_HISTORY_LEN - 1;
        else
            ctx->hpos--;

        snprintf(prompt, sizeof(prompt), "\r\n(%d/%d)",
                ctx->hpos, CLI_CMD_HISTORY_LEN);

        CLI_IO_OUTPUT(prompt, strlen(prompt));
        CLI_IO_OUTPUT(ctx->cmd, strlen(ctx->cmd));
    }
}


static void _cli_history_down(void* a_ctx) {
    t_cli_ctx *ctx = (t_cli_ctx *)a_ctx;
    char prompt[12] = {0x00};

    if ((ctx->hhead != ctx->htail) &&
            strlen(ctx->history[ctx->hpos])) {
        memset(ctx->cmd, 0x00, CLI_CMD_BUFFER_SIZE);

        ctx->hpos++;
        ctx->hpos %= CLI_CMD_HISTORY_LEN;

        strcpy(ctx->cmd, ctx->history[ctx->hpos]);
        ctx->cpos = strlen(ctx->cmd);

        snprintf(prompt, sizeof(prompt), "\r\n(%d/%d)",
                ctx->hpos, CLI_CMD_HISTORY_LEN);

        CLI_IO_OUTPUT(prompt, strlen(prompt));
        CLI_IO_OUTPUT(ctx->cmd, strlen(ctx->cmd));
    }
}


static void _cli_delete(void *a_data) {
    t_cli_ctx *ctx = (t_cli_ctx *)a_data;

    if (ctx->cpos) {
        ctx->cmd[--ctx->cpos] = '\0';
        CLI_IO_OUTPUT("\b \b", 3);
    }
}


static void _cli_none(void *a_data) {
    return;
}

static int do_help(int argc, const char **argv)
{
    struct cmd_struct *cmd = cmds;

    if(argc == 1)LOG_DEBUG("\r\nSupported commands:\r\n");
    do {
        if( (cmd->cmd) && (cmd->usage) )
        {
        	if(argc > 1){
        		if(strcmp(argv[1], cmd->cmd) == 0){
        			LOG_DEBUG("  %s %s\r\n", cmd->cmd, cmd->usage);
        			break;
        		}
        	}else{
        		LOG_DEBUG("  %s %s\r\n", cmd->cmd, cmd->usage);
        	}
        }
        cmd = cmd->next;

    } while (cmd);

    return 0;
}
/* ================================================================================ */

void cli_main(void)
{
    int i;

    t_cli_ctx cli_ctx;

    LOG_DEBUG("Cli maxargc:%d, maxargv size:%d\n", CLI_CMD_MAX_ARGC, CLI_CMD_BUFFER_SIZE);
    for(i = 0; i < CLI_CMD_MAX_ARGC; i++)
    {
        gArgv[i] = (char *)OSAL_malloc(CLI_CMD_BUFFER_SIZE);
        if(gArgv[i] == NULL) 
        {
        	LOG_DEBUG("Error! can't malloc\n");
            return;
        }
    }

    /* add pi5008 command */
    cli_add_cmds(pi5008_cmds, sizeof(pi5008_cmds) / sizeof(struct cmd_struct));

    memset(&cli_ctx, 0x00, sizeof(t_cli_ctx));
    cli_ctx.cmds = pi5008_cmds;
    cli_ctx.mcmds = _g_mc_cmds;

    PPAPI_FATFS_EnterFS();
    
#ifdef TEST_AUTO_VIEWMODE_CHG
	{
		TickType_t xLastWakeTime = xTaskGetTickCount();
		{
			xLastWakeTime = xTaskGetTickCount();
			vTaskDelayUntil( &xLastWakeTime, ((uint32_t)OSAL_ms2ticks(20000)) );
	
		}
	}
#endif

	//_slt_test_manual();		///< Manual Test Mode

	//_slt_test_auto_mipi_rx();	///< FT SLT Mode - CRC Check Mode

	//_slt_test_auto_pvi_rx();	///< FT SLT Mode - Frame Capture & Diff Mode

    while (1) {
#ifdef TEST_AUTO_VIEWMODE_CHG
        {
            TickType_t xLastWakeTime = xTaskGetTickCount();
            uint32_t u32Cmd = CMD_UI_KEY_UP;		
            printf("test auto view mode change.\n");
            {
                xLastWakeTime = xTaskGetTickCount();
                vTaskDelayUntil( &xLastWakeTime, ((uint32_t)OSAL_ms2ticks(2000)) );

                u32Cmd = CMD_UI_KEY_UP;		
                AppTask_SendCmd(u32Cmd, TASK_CLI, TASK_UI, 0, 0, 0, 100);

                xLastWakeTime = xTaskGetTickCount();
                vTaskDelayUntil( &xLastWakeTime, ((uint32_t)OSAL_ms2ticks(2000)) );

                u32Cmd = CMD_UI_KEY_DOWN;	
                AppTask_SendCmd(u32Cmd, TASK_CLI, TASK_UI, 0, 0, 0, 100);

                xLastWakeTime = xTaskGetTickCount();
                vTaskDelayUntil( &xLastWakeTime, ((uint32_t)OSAL_ms2ticks(2000)) );

                u32Cmd = CMD_UI_KEY_LEFT;	
                AppTask_SendCmd(u32Cmd, TASK_CLI, TASK_UI, 0, 0, 0, 100);

                xLastWakeTime = xTaskGetTickCount();
                vTaskDelayUntil( &xLastWakeTime, ((uint32_t)OSAL_ms2ticks(2000)) );

                u32Cmd = CMD_UI_KEY_RIGHT;	
                AppTask_SendCmd(u32Cmd, TASK_CLI, TASK_UI, 0, 0, 0, 100);
            }
        }
#else            
        cli_read(&cli_ctx);
#endif // TEST_AUTO_VIEWMODE_CHG
    }

    PPAPI_FATFS_ReleaseFS();
}       

#ifdef SUPPORT_DEBUG_CLI
PP_VOID vTaskCLI(PP_VOID *pvData)
{

	LOG_DEBUG("Start Task(%s), Priority:%d\n", __FUNCTION__, (int)uxTaskPriorityGet(NULL));

#if 1
    cli_main();     /* this function never return */
#else
    {
        int myHandleNum = TASK_CLI;
        TickType_t xLastWakeTime = xTaskGetTickCount();
        int i = 0;

        /*  test event&queue receiving */
        for(;;)
        {
            vTaskDelayUntil( &xLastWakeTime, ((uint32_t)OSAL_ms2ticks(500)) );

            if(gHandle[TASK_FWDN].phQueue)
            {/*{{{*/
                stQueueItem queueItem;
                char testStringBuf[80];
                sprintf(testStringBuf, "Send buf string..(%d)\n", i);
                memset(&queueItem, 0, sizeof(queueItem));
                queueItem.u32Cmd = i++;
                queueItem.u16Sender = myHandleNum;
                //queueItem.u16Attr = (1<<QUEUE_CMDATTR_BIT_REQACK); //If you want ack.
                queueItem.u16Attr = 0;
                queueItem.u32Length = strlen(testStringBuf);
                queueItem.pData = (char *)OSAL_malloc(queueItem.u32Length);
                strncpy(queueItem.pData, testStringBuf, queueItem.u32Length);

                if( OSAL_QUEUE_CMD_Send(gHandle[TASK_FWDN].phQueue, &queueItem, 0) != eSUCCESS )
                {
                    printf("Fail Send queue-CMD\n");
                }
                printf("Send Queue[Sender:%d(%s), Receiver:%d(%s)]\n", 
                        queueItem.u16Sender, psTaskName[queueItem.u16Sender], TASK_FWDN, psTaskName[TASK_FWDN]);
            }/*}}}*/

            OSAL_EVENTGROUP_CMD_SetBits(gHandle[TASK_FWDN].phEventGroup, 1<<EVENT_FWDN_MSG);
            printf("SetEventBit[%08x(%s)]\n",1<<EVENT_FWDN_MSG, psEventNameFWDN[EVENT_FWDN_MSG]);
        }
    }
#endif

}       
#endif

#endif /* SUPPORT_DEBUG_CLI */
