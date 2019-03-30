
#include "pi5008.h"
#include "proc.h"
#include "du_register.h"
#include "du_drv.h"
#include "interrupt.h"
#include "osal.h"
#include "utils.h"
#include "api_vin.h"
#include "sys_api.h"

typedef union ppPP_DU_BTO_CONFIG_U
{
	PP_VU32	var;

	struct
	{
		PP_VU32		bt1120_limit_digital	: 1,	// [0]
					outfmt_yc_inv			: 1,	// [1]
					outfmt_bt656			: 1,	// [2]
					outfmt_16bit			: 1,	// [3]
					sd_fld_pol 				: 1,	// [4]
					sd_outfmt_md			: 1,	// [5]
					sd_960h_md				: 1,	// [6]
					sd_pal_ntb				: 1,	// [7]
					bto_dout_md				: 2,	// [9:8]
					bayer_out_md			: 2,	// [11:10]
					bto_de_md				: 1,	// [12]
					outfmt_hav_vblk			: 1,	// [13]
					dis_async_en			: 1,	// [14]
					dis_async_chg_en		: 1,	// [15]
					clpf					: 1,	// [16]
					sd_p2i_en				: 1,	// [17]
					outfmt_cbcr_inv			: 1,	// [18]
					sd_prg_md				: 1,	// [19]  To Be Reserved.
					bto_hav_del				: 4,	// [23:20]
					dis_hd_csc_md			: 2,	// [25:24]
					bt1120_vmask_md_dig		: 1,	// [26]
					bt1120_vmask_md_ana		: 1,	// [27]
					reserved				: 4;	// [31:28]
	} fld;
} PP_DU_BTO_CONFIG_U;

typedef union ppPP_DU_BTO_CONFIG1_U
{
	PP_VU32	var;

	struct
	{
		PP_VU32		bt1120_lim_ana		: 1,	// [0]
					byte_order			: 3,	// [3:1]
					bto_vsync_delay		: 4,	// [7:4]
					pat_sel				: 4,	// [11:8]
					pat_md 				: 4,	// [15:12]
					dis_clko_phase		: 6,	// [21:16]
					pat_en				: 1,	// [22]
					bayer_bit_width		: 1,	// [23]
					reserved			: 8;	// [31:24]
	} fld;
} PP_DU_BTO_CONFIG1_U;

typedef union ppPP_DU_BTO_CONFIG2_U
{
	PP_VU32	var;

	struct
	{
		PP_VU32		dis_dig_cont		: 8,	// [7:0]
					dis_dig_brt			: 8,	// [15:8]
					dis_dig_sat			: 8,	// [23:16]
					dis_dig_cont_en		: 1,	// [24]
					dis_dig_brt_en 		: 1,	// [25]
					dis_dig_sat_en		: 1,	// [26]
					reserved			: 5;	// [31:27]
	} fld;
} PP_DU_BTO_CONFIG2_U;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
STATIC PP_DU_REG_S *gpDU = (PP_DU_REG_S *)DU_BASE_ADDR;
STATIC PP_DU_CALLBACK gDUCallback = NULL;
STATIC PP_BOOL gDU_isInit = PP_FALSE;
STATIC PP_U32 gu32DUIntcCnt = 0;

//PROC irq
PP_RESULT_E PPDRV_DU_PROC_IRQ(PP_S32 argc, CONST PP_CHAR **argv);
struct proc_irq_struct stDU_PROC_irqs[] = {
	    { .fn = PPDRV_DU_PROC_IRQ,    .irqNum = IRQ_DU_VECTOR, .next = (PP_VOID*)0, },
};

//PROC device
PP_RESULT_E PPDRV_DU_PROC_DEVICE(PP_S32 argc, CONST PP_CHAR **argv);
struct proc_device_struct stDU_PROC_devices[] = {
	    { .pName = "DU",  .fn = PPDRV_DU_PROC_DEVICE,    .next = (PP_VOID*)0, },
};

PP_RESULT_E PPDRV_DU_PROC_IRQ(PP_S32 argc, CONST PP_CHAR **argv)
{
//	PRINT_PROC_IRQ(NAME, NUM, CNT)

	PRINT_PROC_IRQ("DU", IRQ_DU_VECTOR, gu32DUIntcCnt);

	return(eSUCCESS);
}

PP_RESULT_E PPDRV_DU_PROC_DEVICE(PP_S32 IN argc, CONST PP_CHAR** IN argv)
{
	PP_DU_BTO_CONFIG_U cfg;
	PP_DU_BTO_CONFIG1_U cfg1;

	cfg.var = gpDU->bto.ctrl0;
	cfg1.var = gpDU->bto.ctrl1;

	if( (argc) && (strcmp(argv[0], stDU_PROC_devices[0].pName) && strcmp(argv[0], "ALL")) )
	{
		return(eERROR_INVALID_ARGUMENT);
	}
	printf("\n%s Device Info -------------\n", stDU_PROC_devices[0].pName);

	printf("### DU info(Driver Version : 0x%08X) ### \n", gpDU->ctrl.version);

	printf("\n------------------ [DU CTRL] setting info -------------------------\n");
	printf("------------------ [ctrl] setting info\n");
	printf("ctrl : ");
	if((gpDU->ctrl.ctrl&0xF) == 0)		// bt_to_port_sel
		printf("BT I/O port");
	
	if(((gpDU->ctrl.ctrl>>4)&0xF) == 0)	// bt_to_mvi_sel
	{
		if((gpDU->ctrl.ctrl&0xF) == 0)
			printf("&");
		printf("MVI");
	}
	switch((gpDU->ctrl.ctrl>>24)&0xF)	// yuv_to_bto_sel
	{
//		case 0:	printf("ISP");		break;
		case 1:	printf("<-SVM");	break;
//		case 2:	printf("ISP");		break;
		case 4:	printf("<-OSD");	break;
	}
	switch((gpDU->ctrl.ctrl>>20)&0xF)	// yuv_to_osd_sel
	{
		case 1:	printf("<-SVM");	break;
//		case 2:	printf("ISP");		break;
	}
	switch((gpDU->ctrl.ctrl>>8)&0xF)	// yuv_from_isp_sel
	{
		case 0:	printf("<-CAM0/ISP0");	break;
		case 1:	printf("<-CAM1/ISP1");	break;
		case 2:	printf("<-CAM2/ISP2");	break;
		case 3:	printf("<-CAM3/ISP3");	break;
	}
	printf("\n");
	
	printf("------------------ [ctrl1] setting info \n");
	printf("bto_chroma_swap : %d\n", gpDU->ctrl.ctrl1&0x1);
	printf("osd2pvi_async_enable : %s\n", ((gpDU->ctrl.ctrl1>>4)&0x3)?"enable":"disable");
	printf("quad2pvi_async_enable : %s\n", ((gpDU->ctrl.ctrl1>>6)&0x3)?"enable":"disable");
	printf("pvi_tx_sel : %s\n", ((gpDU->ctrl.ctrl1>>8)&0x1)?"quad":"osd");

	printf("\n------------------ [BTO] setting info -------------------------\n");
	printf("------------------ [bto_ctrl0] setting info\n");
	printf("bt1120_limit_digital : %s\n", (cfg.fld.bt1120_limit_digital)?"16-240":"1-254");
	printf("outfmt_yc_inv : %s\n", (cfg.fld.outfmt_yc_inv)?"swap Y/C":"normal Y/C");
	printf("outfmt_bt656 : %s\n", (cfg.fld.outfmt_bt656)?"bt656":"bt1120");
	printf("outfmt_16bit : %s\n", (cfg.fld.outfmt_16bit)?"16bit":"8bit");
	printf("sd_fld_pol : %s\n", (cfg.fld.sd_fld_pol)?"sync inversion":" ");
	printf("sd_outfmt_md : %s\n", (cfg.fld.sd_outfmt_md)?"sd":"hd");
	printf("sd_960h_md : %s\n", (cfg.fld.sd_960h_md)?"960h":"720h");
	printf("sd_pal_ntb : %s\n", (cfg.fld.sd_pal_ntb)?"pal":"ntsc");
	printf("bto_dout_md : ");
	if(cfg.fld.bto_dout_md == 0)		printf("YC\n");
	else if(cfg.fld.bto_dout_md == 1)	printf("RGB\n");
	else if(cfg.fld.bto_dout_md == 2)	printf("Bayer (LSB Mapping)\n");
	else if(cfg.fld.bto_dout_md == 3)	printf("Bayer (MSB Mapping)\n");
	printf("bayer_out_md : ");
	if(cfg.fld.bayer_out_md == 0)		printf("R\n");
	else if(cfg.fld.bayer_out_md == 1)	printf("Gr\n");
	else if(cfg.fld.bayer_out_md == 2)	printf("Gb\n");
	else if(cfg.fld.bayer_out_md == 1)	printf("B\n");
	printf("bto_de_md : %s\n", (cfg.fld.bto_de_md)?"data enb mode":"field mode");
	printf("outfmt_hav_vblk : %s\n", (cfg.fld.outfmt_hav_vblk)?"No HAV in VBLANK duration":"normal mode");
	printf("dis_async_en : %s\n", (cfg.fld.dis_async_en)?"disable":"enable");
	printf("dis_async_chg_en : %s\n", (cfg.fld.dis_async_chg_en)?"disable":"enable");
	printf("clpf : %s\n", (cfg.fld.clpf)?"Chroma LPF enable":"No Filtering");
	printf("sd_p2i_en : %s\n", (cfg.fld.sd_p2i_en)?"on":"off");
	printf("outfmt_cbcr_inv : %s\n", (cfg.fld.outfmt_cbcr_inv)?"swap":"normal");
	printf("sd_prg_md : %s\n", (cfg.fld.sd_prg_md)?"progressive":"interlace");
	printf("bto_hav_del : %d\n", cfg.fld.bto_hav_del);
	printf("dis_hd_csc_md : ");
	if(cfg.fld.dis_hd_csc_md == 0)		printf("bypass\n");
	else if(cfg.fld.dis_hd_csc_md == 1)	printf("bypass\n");
	else if(cfg.fld.dis_hd_csc_md == 2)	printf("bt601 to bt709\n");
	else if(cfg.fld.dis_hd_csc_md == 3)	printf("bt709 to bt601\n");
	printf("bt1120_vmask_md_dig : %s\n", (cfg.fld.bt1120_vmask_md_dig)?"only H":"H&V");
	printf("bt1120_vmask_md_ana : %s\n", (cfg.fld.bt1120_vmask_md_ana)?"only H":"H&V");

	printf("------------------ [ctrl1] setting info\n");
	printf("bt1120_lim_ana : %s\n", (cfg1.fld.bt1120_lim_ana)?"16-240":"1-254");
	printf("byte_order : ");
	if(cfg1.fld.byte_order == 0b000)		printf("{R,G,B}\n");
	else if(cfg1.fld.byte_order == 0b001)	printf("{G,B,R}\n");
	else if(cfg1.fld.byte_order == 0b010)	printf("{B,R,G}\n");
	else if(cfg1.fld.byte_order == 0b100)	printf("{R,B,G}\n");
	else if(cfg1.fld.byte_order == 0b101)	printf("{G,R,B}\n");
	else if(cfg1.fld.byte_order == 0b110)	printf("{B,G,R}\n");
	printf("bto_vsync_delay : %d\n", cfg1.fld.bto_vsync_delay);
	printf("pat_sel : ");
	if(cfg1.fld.pat_sel == 0)		printf("720p25\n");
	else if(cfg1.fld.pat_sel == 1)	printf("720p30\n");
	else if(cfg1.fld.pat_sel == 2)	printf("720p50\n");
	else if(cfg1.fld.pat_sel == 3)	printf("720p60\n");
	else if(cfg1.fld.pat_sel == 4)	printf("1080p25\n");
	else if(cfg1.fld.pat_sel == 5)	printf("1080p30\n");
	else if(cfg1.fld.pat_sel == 6)	printf("1080i50\n");
	else if(cfg1.fld.pat_sel == 7)	printf("1080i60\n");
	else if(cfg1.fld.pat_sel == 8)	printf("960Hi50\n");
	else if(cfg1.fld.pat_sel == 9)	printf("960Hi60\n");
	else if(cfg1.fld.pat_sel == 10)	printf("720Hi50\n");
	else if(cfg1.fld.pat_sel == 11)	printf("720Hi60\n");
	else if(cfg1.fld.pat_sel == 12)	printf("960Hp50\n");
	else if(cfg1.fld.pat_sel == 13)	printf("960Hp60\n");
	else if(cfg1.fld.pat_sel == 14)	printf("720Hp50\n");
	else if(cfg1.fld.pat_sel == 15)	printf("720Hp60\n");
	printf("pat_md : ");
	if(cfg1.fld.pat_md == 0)		printf("White\n");
	else if(cfg1.fld.pat_md == 1)	printf("Yellow\n");
	else if(cfg1.fld.pat_md == 2)	printf("Cyan\n");
	else if(cfg1.fld.pat_md == 3)	printf("Green\n");
	else if(cfg1.fld.pat_md == 4)	printf("Magenta\n");
	else if(cfg1.fld.pat_md == 5)	printf("Red\n");
	else if(cfg1.fld.pat_md == 6)	printf("Blue\n");
	else if(cfg1.fld.pat_md == 7)	printf("Black\n");
	else if(cfg1.fld.pat_md == 8)	printf("color bar\n");
	else if(cfg1.fld.pat_md == 9)	printf("ramp\n");
	else if(cfg1.fld.pat_md == 10)	printf("gray bar\n");
	else if(cfg1.fld.pat_md == 11)	printf("combination\n");
	
	printf("dis_clko_phase\n");
	if((cfg1.fld.dis_clko_phase>>5)&0x1)
		printf("  - No Multi-Phase Clock Mode");
	else
		printf("  - Multi-phase Clock Output Mode");
	if((cfg1.fld.dis_clko_phase>>1)&0x1)
		printf("(Manual Control Mode)\n");
	else
		printf("(Auto Control Mode)\n");
		
	if((cfg1.fld.dis_clko_phase>>4)&0x1)
		printf("  - 74.25/37.125MHz");
	else
		printf("  - 148.5MHz");
	if((cfg1.fld.dis_clko_phase>>2)&0x1)
		printf("(Manual Control Mode)\n");
	else
		printf("(Auto Control Mode)\n");

	if((cfg1.fld.dis_clko_phase>>4)&0x1)
	{
		if((cfg1.fld.dis_clko_phase>>3)&0x1)
			printf("  - Inverted 148.5MHz Latch Mode\n");
		else
			printf("  - 148.5MHz Latch Mode\n");
	}
	else
	{
		if((cfg1.fld.dis_clko_phase>>3)&0x1)
			printf("  - Inverted Clock\n");
		else
			printf("  - Normal Clock\n");
	}
	
	printf("pat_en : %s\n", (cfg1.fld.pat_en)?"enable":"disable");
	printf("bayer_bit_width : %s\n", (cfg1.fld.bayer_bit_width)?"8bit":"10bit");

	printf("\n------------------ [OSD] setting info -------------------------\n");

	printf("------------------ [reg_blocked] setting info\n");
	printf("reg_blocked : %d\n", gpDU->osd.ctrl.reg_blocked&0x1);

	printf("------------------ [osd_src_sel] setting info\n");
	printf("osd_src_sel : ");
	{
		PP_U8 i;
		PP_U8 sel = 0;

		printf("OSD OUT<-");
		for(i=0 ; i<6 ; i++)
		{
			sel = (gpDU->osd.ctrl.src_sel>>(4*sel))&0xF;
			switch(sel)
			{
				case 0:	printf("YUV");	break;
				case 1:	printf("RLE0");	break;
				case 2:	printf("RLE1");	break;
				case 3:	printf("RLE2");	break;
				case 4:	printf("RLE3");	break;
				case 5:	printf("RLE4");	break;
			}

			if(sel == 0)		break;
			else			printf("<-");
		}
		printf("\n");
	}
	
	printf("------------------ [osd_bus_bw] setting info\n");
	printf("rle0_bw : 0x%01X\n", gpDU->osd.ctrl.bus_bw&0xF);
	printf("rle1_bw : 0x%01X\n", (gpDU->osd.ctrl.bus_bw>>8)&0xF);
	printf("rle2_bw : 0x%01X\n", (gpDU->osd.ctrl.bus_bw>>16)&0xF);
	printf("rle3_bw : 0x%01X\n", (gpDU->osd.ctrl.bus_bw>>24)&0xF);
	printf("rle4_bw : 0x%01X\n", gpDU->osd.ctrl.bus_bw1&0xF);

	printf("------------------ [osd_tg_ctrl] setting info\n");
	printf("tg_enable : %s\n", (gpDU->osd.tg.ctrl&0x1)?"enable":"disable");

	{
		PP_U8 layer, area;
		PP_U32 value;
		
		for(layer=eLayer0 ; layer<eLayer_MAX ; layer++)
		{
			printf("\n------------------ [OSD RLE%d CANVAS] setting info -------------------------\n", layer);
			printf("canvas_width : %d\n", gpDU->osd.rle[layer].canvas.canvas_size&0xFFF);
			printf("canvas_height : %d\n", (gpDU->osd.rle[layer].canvas.canvas_size>>16)&0xFFF);
			printf("canvas_color : 0x%08X\n", gpDU->osd.rle[layer].canvas.canvas_color);
			printf("use_global_alpha : %s\n", ((gpDU->osd.rle[layer].canvas.global_alpha>>8)&0x1)?"enable":"disable");
			printf("global_alpha : 0x%01X\n", gpDU->osd.rle[layer].canvas.global_alpha&0xF);
			printf("2d_dma_mode : %s\n", ((gpDU->osd.rle[layer].canvas.layer_mode>>5)&0x1)?"enable":"disable");
			printf("field_en : %s\n", ((gpDU->osd.rle[layer].canvas.layer_mode>>7)&0x1)?"enable":"disable");
			printf("format : ");

			if(gpDU->osd.rle[layer].canvas.layer_mode&0x1)
			{
				switch(gpDU->osd.rle[layer].canvas.layer_mode&0xF)
				{
					case 0x1:	printf("INDEX\n");		break;
					case 0x3:	printf("RGB565\n");		break;
					case 0x5:	printf("RGB888\n");		break;
					case 0x7:	printf("ARGB8888\n");	break;
					case 0x9:	printf("1BIT\n");		break;
					case 0xB:	printf("RGBA4444\n");	break;
				}
			}
			else
			{
				printf("RLE\n");
			}

			for(area=eArea0 ; area<eArea_MAX ; area++)
			{
				printf("------------------ [OSD RLE%d AREA%d] setting info\n", layer, area);
				printf("area base : 0x%08X\n", gpDU->osd.rle[layer].area[area].base_addr);
				if((gpDU->osd.rle[layer].canvas.layer_mode>>7)&0x1)	// field mode
				{
					printf("area base2 : 0x%08X\n", gpDU->osd.rle[layer].area[area].base_addr2);
				}
				
				if((gpDU->osd.rle[layer].canvas.layer_mode>>5)&0x1)	// 2d-dma mode
				{
					printf("area_stride : 0x%08X\n", gpDU->osd.rle[layer].area[area].stride);
				}
				else												// 1d-dma mode
				{
					printf("area_byte : 0x%08X\n", gpDU->osd.rle[layer].area[area].byte);
				}

				if(area == 0)		value = gpDU->osd.rle[layer].canvas.area0_start;
				else if(area == 1)	value = gpDU->osd.rle[layer].canvas.area1_start;
				else if(area == 2)	value = gpDU->osd.rle[layer].canvas.area2_start;
				else if(area == 3)	value = gpDU->osd.rle[layer].canvas.area3_start;
				
				printf("area size (x, y, w, h) : %d, %d, %d, %d\n",
						value&0xFFF, (value>>16)&0xFFF, gpDU->osd.rle[layer].area[area].image_size&0xFFF, (gpDU->osd.rle[layer].area[area].image_size>>16)&0xFFF);
				printf("area%d en : %s\n", area, (gpDU->osd.rle[layer].area[area].config&0x1)?"enable":"disable");
			}
		}
	}

	printf("\n------------------ [STATE for DEBUG] setting info -------------------------\n");
	printf("------------------ [du_state0] setting info\n");
	printf("  - (%d)bt_from_bto_hsync\n", (gpDU->ctrl.state0>>13)&0x1);
	printf("  - (%d)bt_from_bto_vsync\n", (gpDU->ctrl.state0>>12)&0x1);
	printf("  - (%d)pvi_irq_flag\n", (gpDU->ctrl.state0>>11)&0x1);
	printf("  - (%d)osd_tg_irq\n", (gpDU->ctrl.state0>>9)&0x1);
	printf("  - (%d)osd_bus_irq\n", (gpDU->ctrl.state0>>8)&0x1);
	printf("  - (%d)svm_hsync monitor\n", (gpDU->ctrl.state0>>7)&0x1);
	printf("  - (%d)svm_vsync monitor\n", (gpDU->ctrl.state0>>6)&0x1);
	printf("  - (%d)isp_hsync monitor\n", (gpDU->ctrl.state0>>5)&0x1);
	printf("  - (%d)isp_vsync monitor\n", (gpDU->ctrl.state0>>4)&0x1);
	printf("  - (%d)osd_hsync monitor\n", (gpDU->ctrl.state0>>3)&0x1);
	printf("  - (%d)osd_vsync monitor\n", (gpDU->ctrl.state0>>2)&0x1);
	printf("  - (%d)digital_bto_hsync monitor\n", (gpDU->ctrl.state0>>1)&0x1);
	printf("  - (%d)digital_bto_vsync monitor\n", gpDU->ctrl.state0&0x1);
	
	printf("------------------ [bto_state] setting info\n");
	printf("  - (%d)yuvi_hsync, HSYNC of YUV input to BTO block\n", (gpDU->bto.state>>5)&0x1);
	printf("  - (%d)yuvi_vsync, VSYNC of YUV input to BTO block\n", (gpDU->bto.state>>4)&0x1);
	printf("  - (%d)bto_fsync, FSYNC/DE from BTO block\n", (gpDU->bto.state>>2)&0x1);
	printf("  - (%d)bto_hsync, HSYNC from BTO block\n", (gpDU->bto.state>>1)&0x1);
	printf("  - (%d)bto_vsync, VSYNC from BTO block\n", gpDU->bto.state&0x1);
	
	printf("------------------ [osd_state0] setting info\n");
	printf("  - (%d) input hsync to OSDTG\n", (gpDU->osd.ctrl.state0>>3)&0x1);
	printf("  - (%d) input vsync to OSDTG\n", (gpDU->osd.ctrl.state0>>2)&0x1);
	printf("  - (%d) output hsync from OSDTG\n", (gpDU->osd.ctrl.state0>>1)&0x1);
	printf("  - (%d) output vsync from OSDTG\n", gpDU->osd.ctrl.state0&0x1);
	
	printf("------------------ [osd_state1] setting info\n");
	printf("  - (%d%d%d%d%d)exception from layer\n",
			(gpDU->osd.ctrl.state1>>8)&0x1, (gpDU->osd.ctrl.state1>>7)&0x1, (gpDU->osd.ctrl.state1>>6)&0x1, (gpDU->osd.ctrl.state1>>5)&0x1, (gpDU->osd.ctrl.state1>>4)&0x1);
	printf("  - (%d) input to OSDTG blocked\n", (gpDU->osd.ctrl.state1>>2)&0x1);
	printf("  - (%d) OSDTG queue full\n", gpDU->osd.ctrl.state1&0x1);
	
	printf("------------------ [frame_counter] setting info\n");
	printf("  - frame_counter : %d\n", gpDU->osd.ctrl.frame_cnt);
	{
		PP_U8 layer;
		for(layer=eLayer0 ; layer<eLayer_MAX ; layer++)
		{
			printf("------------------ [rle%d_state] setting info\n", layer);
			printf("  - (%d) area3 buffer empty\n", (gpDU->osd.rle[layer].canvas.layer_state>>23)&0x1);
			printf("  - (%d) area2 buffer empty\n", (gpDU->osd.rle[layer].canvas.layer_state>>22)&0x1);
			printf("  - (%d) area1 buffer empty\n", (gpDU->osd.rle[layer].canvas.layer_state>>21)&0x1);
			printf("  - (%d) area0 buffer empty\n", (gpDU->osd.rle[layer].canvas.layer_state>>20)&0x1);
		}
	}

	printf("\n------------------ [BUS MONITOR] setting info -------------------------\n");
	printf("  - enable_ch0 : %s\n", (gpDU->osd.monitor0.ctrl&0x1)?"enable":"disable");
	printf("  - enable_ch1 : %s\n", ((gpDU->osd.monitor0.ctrl>>1)&0x1)?"enable":"disable");
	printf("  - enable_ch2 : %s\n", ((gpDU->osd.monitor0.ctrl>>2)&0x1)?"enable":"disable");
	printf("  - enable_ch3 : %s\n", ((gpDU->osd.monitor0.ctrl>>3)&0x1)?"enable":"disable");
	printf("  - enable_ch4 : %s\n", (gpDU->osd.monitor1.ctrl&0x1)?"enable":"disable");
	printf("  - enable_ch5 : %s\n", ((gpDU->osd.monitor1.ctrl>>1)&0x1)?"enable":"disable");
	printf("  - enable_ch6 : %s\n", ((gpDU->osd.monitor1.ctrl>>2)&0x1)?"enable":"disable");
	printf("  - enable_ch7 : %s\n", ((gpDU->osd.monitor1.ctrl>>3)&0x1)?"enable":"disable");

	printf("  - ch0_total : %d\n", gpDU->osd.monitor0.ch0_total);
	printf("  - ch0_word : %d\n", gpDU->osd.monitor0.ch0_word);
	printf("  - ch1_total : %d\n", gpDU->osd.monitor0.ch1_total);
	printf("  - ch1_word : %d\n", gpDU->osd.monitor0.ch1_word);
	printf("  - ch2_total : %d\n", gpDU->osd.monitor0.ch2_total);
	printf("  - ch2_word : %d\n", gpDU->osd.monitor0.ch2_word);
	printf("  - ch3_total : %d\n", gpDU->osd.monitor0.ch3_total);
	printf("  - ch3_word : %d\n", gpDU->osd.monitor0.ch3_word);
	printf("  - ch4_total : %d\n", gpDU->osd.monitor1.ch4_total);
	printf("  - ch4_word : %d\n", gpDU->osd.monitor1.ch4_word);
	printf("  - ch5_total : %d\n", gpDU->osd.monitor1.ch5_total);
	printf("  - ch5_word : %d\n", gpDU->osd.monitor1.ch5_word);
	printf("  - ch6_total : %d\n", gpDU->osd.monitor1.ch6_total);
	printf("  - ch6_word : %d\n", gpDU->osd.monitor1.ch6_word);
	printf("  - ch7_total : %d\n", gpDU->osd.monitor1.ch7_total);
	printf("  - ch7_word : %d\n", gpDU->osd.monitor1.ch7_word);

	printf("\n------------------ [BUS TEST MODULE] setting info -------------------------\n");
	printf("bist_en : %s\n", (gpDU->bus_test.ctrl0&0x1)?"enable":"disable");
	printf("crc128_data[127:0] : 0x%08X%08X%08X%08X\n", gpDU->bus_test.crc3, gpDU->bus_test.crc2, gpDU->bus_test.crc1, gpDU->bus_test.crc0);

	printf("\n------------------ [BTO_CRC] setting info -------------------------\n");
	{
		PP_U32 area;
		PP_BTO_CRC_REG_S *p_bto_crc_area;

		for(area=eArea0 ; area<eArea_MAX ; area++)
		{
			printf("------------------ [BTO_CRC/AREA%d] setting info\n", area);
			if(area == eArea0)		p_bto_crc_area = (PP_BTO_CRC_REG_S *)&gpDU->bto_crc.area0;
			else if(area == eArea1)	p_bto_crc_area = (PP_BTO_CRC_REG_S *)&gpDU->bto_crc.area1;
			else if(area == eArea2)	p_bto_crc_area = (PP_BTO_CRC_REG_S *)&gpDU->bto_crc.area2;
			else if(area == eArea3)	p_bto_crc_area = (PP_BTO_CRC_REG_S *)&gpDU->bto_crc.area3;
			
			printf("BIST_EN : %s\n", (p_bto_crc_area->ctrl0&0x1)?"enable":"disable");
			printf("FR_PERIOD : %d\n", (p_bto_crc_area->ctrl0>>1)&0x7);
			printf("ROI_EN : %s\n", ((p_bto_crc_area->ctrl0>>4)&0x1)?"enable":"disable");
			printf("ROI_MD : %d\n", (p_bto_crc_area->ctrl0>>5)&0x1);
			printf("FLD_MD : %d\n", (p_bto_crc_area->ctrl0>>6)&0x1);
			printf("FLD_SEL : %s\n", ((p_bto_crc_area->ctrl0>>7)&0x1)?"O":"X");
			printf("FR_OS : %d\n", (p_bto_crc_area->ctrl0>>8)&0x2);

			printf("REF_DATA : %d\n", p_bto_crc_area->ctrl1&0xFFFFFF);
			printf("HSTRT : %d | VSTRT : %d\n", p_bto_crc_area->ctrl2&0x7FF, (p_bto_crc_area->ctrl2>>16)&0x7FF);
			printf("HEND : %d | VEND : %d\n", p_bto_crc_area->ctrl3&0x7FF, (p_bto_crc_area->ctrl3>>16)&0x7FF);
			printf("CRC_SUM_DATA_ACC0 : %d\n", p_bto_crc_area->acc0&0xFFFFFF);
			printf("CRC_SUM_DATA_ACC1 : %d\n", p_bto_crc_area->acc1&0xFFFFFF);
			printf("CRC_SUM_DATA_ACC2 : %d\n", p_bto_crc_area->acc2&0xFFFFFF);
			printf("CRC_SUM_DATA_ACC3 : %d\n", p_bto_crc_area->acc3&0xFFFFFF);
			printf("CRC_BIST_DONE : %s\n", (p_bto_crc_area->state&0x1)?"O":"X");
			printf("CRC_BIST_DONE_ACC : %s\n", ((p_bto_crc_area->state>>4)&0x7)?"O":"X");
		}
	}
	
	printf("--------------------------------------------------------------\n");
	return(eSUCCESS);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ISR(du_isr, num)
{
    PP_U32 msk = (1 << num);

    INTC_irq_clean(num);
    //INTC_irq_enable(num);

    __nds32__mtsr_dsb(__nds32__mfsr(NDS32_SR_INT_MASK2) & ~msk, NDS32_SR_INT_MASK2);
    __nds32__mtsr(msk, NDS32_SR_INT_PEND2);

	gu32DUIntcCnt++;

	if(gDUCallback)
		gDUCallback();
	
    __nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK2) | msk, NDS32_SR_INT_MASK2);
}

PP_VOID PPDRV_DU_SetInterrupt (PP_DU_CALLBACK IN callback)
{
	OSAL_register_isr(IRQ_DU_VECTOR, du_isr, NULL);
	INTC_irq_config(IRQ_DU_VECTOR, IRQ_LEVEL_TRIGGER);
	INTC_irq_clean(IRQ_DU_VECTOR);
	INTC_irq_enable(IRQ_DU_VECTOR);

	gDUCallback = callback;

	//register proc infomation.
	SYS_PROC_addIrq(stDU_PROC_irqs);
	SYS_PROC_addDevice(stDU_PROC_devices);
}

PP_VOID PPDRV_DU_SWreset (PP_VOID)
{
	while( (gpDU->bto.state & 0x1) != 0 );	// wait blank
	
	(*((PP_VU32 *)(0xF0000044))) &= ~(1<<15);
	(*((PP_VU32 *)(0xF0000044))) |= (1<<15);
}

/* 1. Set video path */
PP_VOID PPDRV_DU_SetVideoPath (PP_DU_VIDEO_PATH_E IN path)
{
	switch(path)
	{
		case ePATH_SVMOUT_BYPASS_OUT:	gpDU->ctrl.ctrl = 0x01800000;	break;	//	SVM_OUTPUT -> bypass -> OSD_OUTPUT
		case ePATH_SVMIN0_BYPASS_OUT:	gpDU->ctrl.ctrl = 0x02800000;	break;	//	SVM_INPUT0 -> bypass -> OSD_OUTPUT
		case ePATH_SVMIN1_BYPASS_OUT:	gpDU->ctrl.ctrl = 0x02800100;	break;	//	SVM_INPUT1 -> bypass -> OSD_OUTPUT
		case ePATH_SVMIN2_BYPASS_OUT:	gpDU->ctrl.ctrl = 0x02800200;	break;	//	SVM_INPUT2 -> bypass -> OSD_OUTPUT
		case ePATH_SVMIN3_BYPASS_OUT:	gpDU->ctrl.ctrl = 0x02800300;	break;	//	SVM_INPUT3 -> bypass -> OSD_OUTPUT
		case ePATH_SVMOUT_MIXER_OUT:	gpDU->ctrl.ctrl = 0x04100000;	break;	//	SVM_OUTPUT -> OSD -> OSD_OUTPUT
		case ePATH_SVMIN0_MIXER_OUT:	gpDU->ctrl.ctrl = 0x04200000;	break;	//	SVM_INPUT0 -> OSD -> OSD_OUTPUT
		case ePATH_SVMIN1_MIXER_OUT:	gpDU->ctrl.ctrl = 0x04200100;	break;	//	SVM_INPUT1 -> OSD -> OSD_OUTPUT
		case ePATH_SVMIN2_MIXER_OUT:	gpDU->ctrl.ctrl = 0x04200200;	break;	//	SVM_INPUT2 -> OSD -> OSD_OUTPUT
		case ePATH_SVMIN3_MIXER_OUT:	gpDU->ctrl.ctrl = 0x04200300;	break;	//	SVM_INPUT3 -> OSD -> OSD_OUTPUT
        default: break;
	}
}

/* 2. Set Output */
PP_VOID PPDRV_DU_BTO_SetYC8(PP_DU_RESOLUTION_E IN in_resolution, PP_DU_RESOLUTION_E IN out_resolution, PP_DU_SYNC_E IN sync)
{
	PP_DU_BTO_CONFIG_U cfg;
	PP_DU_BTO_CONFIG1_U cfg1;

	if (in_resolution >= eRESOLUTION_MAX)
	{
		printf("in resolution is invalid!\n");
		return;
	}
	if (out_resolution >= eRESOLUTION_MAX)
	{
		printf("out resolution is invalid!\n");
		return;
	}

	cfg.var = 0x0;
	cfg1.var = 0x0;

	if( (out_resolution == eRESOLUTION_720Hi_NTSC) || (out_resolution == eRESOLUTION_720Hp_NTSC) )
	{
		cfg.fld.bt1120_vmask_md_ana		= 0;	// [27] bt1120_vmask_md_ana = 0 : ANA YC H&V Mask Mode, 1 : ANA YC H Only Mask Mode
		cfg.fld.bt1120_vmask_md_dig		= 0;	// [26] bt1120_vmask_md_dig = 0 : DIG YC H&V Mask Mode, 1 : DIG YC H Only Mask Mode
		cfg.fld.dis_hd_csc_md			= 0;	// [25:24] dis_hd_csc_md = 0,1 : Bypass, 2 : BT601 to BT709, 3 : BT709 to BT601
		cfg.fld.bto_hav_del				= 3;	// [23:20] bto_hav_del = 0 ~ 15:External Hsync Delay Control (Recommend : 4'd4)
		cfg.fld.outfmt_cbcr_inv			= 0;	// [18] outfmt_cbcr_inv = 0:Normal CbCr, 1:Swap CbCr
		if( (in_resolution == eRESOLUTION_720Hp_NTSC) && (out_resolution == eRESOLUTION_720Hi_NTSC) )
			cfg.fld.sd_p2i_en			= 1;	// [17] sd_p2i_en = 0:SD P2I Off, 1:SD P2I On
		else
			cfg.fld.sd_p2i_en			= 0;	// [17] sd_p2i_en = 0:SD P2I Off, 1:SD P2I On
		cfg.fld.clpf					= 1;	// [16] clpf = 1:Chroma LPF enable, 0:No Filtering
		cfg.fld.dis_async_chg_en		= 0;	// [15] dis_async_chg_en
		cfg.fld.dis_async_en			= 1;	// [14] dis_async_en
		cfg.fld.outfmt_hav_vblk			= 0;	// [13] outfmt_hav_vblk = 0 : Normal Mode, 1 : No HAV in VBLANK duration
		cfg.fld.bto_de_md				= 1;	// [12] bto_de_md = 0:data_enable mode, 1:field mode
		cfg.fld.bayer_out_md			= 0;	// [11:10] bayer_out_md = 0:R, 1:Gr, 2:Gb, 3:B
		cfg.fld.bto_dout_md 			= 0;	// [9:8] bto_dout_md = 0:YC, 1:RGB, 2:Bayer (LSB Mapping), 3:Bayer (MSB Mapping)
		cfg.fld.sd_pal_ntb				= 0;	// [7] SD_PAL_NTB = 0:NTSC, 1:PAL
		cfg.fld.sd_960h_md				= 0;	// [6] SD_960H_MD = 1:SD 960H mode, 0:SD 720H mode
		cfg.fld.sd_outfmt_md			= 1;	// [5] SD_OUTFMT_MD = 1:SD mode, 0:HD mode
		cfg.fld.sd_fld_pol				= 1;	// [4] SD_FLD_POL = 0:Even High, 1:Odd High
		cfg.fld.outfmt_16bit			= 0;	// [3] OUTFMT_16BIT = 0:8 Bit, 1:16 bit
  		cfg.fld.outfmt_bt656			= 1;	// [2] OUTFMT_BT656 = 0:1120 Mode, 1:656 Mode
  		cfg.fld.outfmt_yc_inv			= 0;	// [1] OUTFMT_YC_INV = 1:swap Y/C, 0:Normal Y/C
		cfg.fld.bt1120_limit_digital	= 0;	// [0] BT1120_LIM_DIG = 1:data limited to 16-240, 0:data limited to 1-254

		cfg1.fld.bayer_bit_width		= 0;	// [23] bayer_bit_width = 0: 10bit, 1 : 8bit
		cfg1.fld.pat_en 				= 0;	// [22] pat_en
		cfg1.fld.dis_clko_phase 		= 0;	// [21:16] dis_clko_phase
		cfg1.fld.pat_md 				= 0;	// [15:12] pat_md
		cfg1.fld.pat_sel				= 0;	// [11:8] pat_sel
		cfg1.fld.bto_vsync_delay		= 0;	// [7:4] BTO_VAV_DEL = VAV Delay (Capture Option..)
		cfg1.fld.byte_order				= 0;	// [3:1] byte_order = 000{RGB}, 001{GBR}, 010{BRG}, 100{RBG}, 101{GRB}, 110{BGR}
		cfg1.fld.bt1120_lim_ana			= 0;	// [0] bt1120_lim_ana = 0:1-254, 1:16-240
		
		gpDU->bto.ctrl0 = cfg.var;
		gpDU->bto.ctrl1 = cfg1.var;
	}
	else if( (out_resolution == eRESOLUTION_720Hi_PAL) || (out_resolution == eRESOLUTION_720Hp_PAL) )
	{
		cfg.fld.bt1120_vmask_md_ana		= 0;	// [27] bt1120_vmask_md_ana = 0 : ANA YC H&V Mask Mode, 1 : ANA YC H Only Mask Mode
		cfg.fld.bt1120_vmask_md_dig		= 0;	// [26] bt1120_vmask_md_dig = 0 : DIG YC H&V Mask Mode, 1 : DIG YC H Only Mask Mode
		cfg.fld.dis_hd_csc_md			= 0;	// [25:24] dis_hd_csc_md = 0,1 : Bypass, 2 : BT601 to BT709, 3 : BT709 to BT601
		cfg.fld.bto_hav_del				= 3;	// [23:20] bto_hav_del = 0 ~ 15:External Hsync Delay Control (Recommend : 4'd4)
		cfg.fld.outfmt_cbcr_inv			= 0;	// [18] outfmt_cbcr_inv = 0:Normal CbCr, 1:Swap CbCr
		if( (in_resolution == eRESOLUTION_720Hp_PAL) && (out_resolution == eRESOLUTION_720Hi_PAL) )
			cfg.fld.sd_p2i_en			= 1;	// [17] sd_p2i_en = 0:SD P2I Off, 1:SD P2I On
		else
			cfg.fld.sd_p2i_en			= 0;	// [17] sd_p2i_en = 0:SD P2I Off, 1:SD P2I On
		cfg.fld.clpf					= 1;	// [16] clpf = 1:Chroma LPF enable, 0:No Filtering
		cfg.fld.dis_async_chg_en		= 0;	// [15] dis_async_chg_en
		cfg.fld.dis_async_en			= 1;	// [14] dis_async_en
		cfg.fld.outfmt_hav_vblk			= 0;	// [13] outfmt_hav_vblk = 0 : Normal Mode, 1 : No HAV in VBLANK duration
		cfg.fld.bto_de_md				= 1;	// [12] bto_de_md = 0:data_enable mode, 1:field mode
		cfg.fld.bayer_out_md			= 0;	// [11:10] bayer_out_md = 0:R, 1:Gr, 2:Gb, 3:B
		cfg.fld.bto_dout_md 			= 0;	// [9:8] bto_dout_md = 0:YC, 1:RGB, 2:Bayer (LSB Mapping), 3:Bayer (MSB Mapping)
		cfg.fld.sd_pal_ntb				= 1;	// [7] SD_PAL_NTB = 0:NTSC, 1:PAL
		cfg.fld.sd_960h_md				= 0;	// [6] SD_960H_MD = 1:SD 960H mode, 0:SD 720H mode
		cfg.fld.sd_outfmt_md			= 1;	// [5] SD_OUTFMT_MD = 1:SD mode, 0:HD mode
		cfg.fld.sd_fld_pol				= 1;	// [4] SD_FLD_POL = 0:Even High, 1:Odd High
		cfg.fld.outfmt_16bit			= 0;	// [3] OUTFMT_16BIT = 0:8 Bit, 1:16 bit
  		cfg.fld.outfmt_bt656			= 1;	// [2] OUTFMT_BT656 = 0:1120 Mode, 1:656 Mode
  		cfg.fld.outfmt_yc_inv			= 0;	// [1] OUTFMT_YC_INV = 1:swap Y/C, 0:Normal Y/C
		cfg.fld.bt1120_limit_digital	= 0;	// [0] BT1120_LIM_DIG = 1:data limited to 16-240, 0:data limited to 1-254
		
		cfg1.fld.bayer_bit_width		= 0;	// [23] bayer_bit_width = 0: 10bit, 1 : 8bit
		cfg1.fld.pat_en 				= 0;	// [22] pat_en
		cfg1.fld.dis_clko_phase 		= 0;	// [21:16] dis_clko_phase
		cfg1.fld.pat_md 				= 0;	// [15:12] pat_md
		cfg1.fld.pat_sel				= 0;	// [11:8] pat_sel
		cfg1.fld.bto_vsync_delay		= 0;	// [7:4] BTO_VAV_DEL = VAV Delay (Capture Option..)
		cfg1.fld.byte_order				= 0;	// [3:1] byte_order = 000{RGB}, 001{GBR}, 010{BRG}, 100{RBG}, 101{GRB}, 110{BGR}
		cfg1.fld.bt1120_lim_ana			= 0;	// [0] bt1120_lim_ana = 0:1-254, 1:16-240
		
		gpDU->bto.ctrl0 = cfg.var;
		gpDU->bto.ctrl1 = cfg1.var;
	}
	else if( (out_resolution == eRESOLUTION_960Hi_NTSC) || (out_resolution == eRESOLUTION_960Hp_NTSC) )
	{
		cfg.fld.bt1120_vmask_md_ana		= 0;	// [27] bt1120_vmask_md_ana = 0 : ANA YC H&V Mask Mode, 1 : ANA YC H Only Mask Mode
		cfg.fld.bt1120_vmask_md_dig		= 0;	// [26] bt1120_vmask_md_dig = 0 : DIG YC H&V Mask Mode, 1 : DIG YC H Only Mask Mode
		cfg.fld.dis_hd_csc_md			= 0;	// [25:24] dis_hd_csc_md = 0,1 : Bypass, 2 : BT601 to BT709, 3 : BT709 to BT601
		cfg.fld.bto_hav_del				= 3;	// [23:20] bto_hav_del = 0 ~ 15:External Hsync Delay Control (Recommend : 4'd4)
		cfg.fld.outfmt_cbcr_inv			= 0;	// [18] outfmt_cbcr_inv = 0:Normal CbCr, 1:Swap CbCr
		if( (in_resolution == eRESOLUTION_960Hp_NTSC) && (out_resolution == eRESOLUTION_960Hi_NTSC) )
			cfg.fld.sd_p2i_en			= 1;	// [17] sd_p2i_en = 0:SD P2I Off, 1:SD P2I On
		else
			cfg.fld.sd_p2i_en			= 0;	// [17] sd_p2i_en = 0:SD P2I Off, 1:SD P2I On
		cfg.fld.clpf					= 1;	// [16] clpf = 1:Chroma LPF enable, 0:No Filtering
		cfg.fld.dis_async_chg_en		= 0;	// [15] dis_async_chg_en
		cfg.fld.dis_async_en			= 1;	// [14] dis_async_en
		cfg.fld.outfmt_hav_vblk			= 0;	// [13] outfmt_hav_vblk = 0 : Normal Mode, 1 : No HAV in VBLANK duration
		cfg.fld.bto_de_md				= 1;	// [12] bto_de_md = 0:data_enable mode, 1:field mode
		cfg.fld.bayer_out_md			= 0;	// [11:10] bayer_out_md = 0:R, 1:Gr, 2:Gb, 3:B
		cfg.fld.bto_dout_md 			= 0;	// [9:8] bto_dout_md = 0:YC, 1:RGB, 2:Bayer (LSB Mapping), 3:Bayer (MSB Mapping)
		cfg.fld.sd_pal_ntb				= 0;	// [7] SD_PAL_NTB = 0:NTSC, 1:PAL
		cfg.fld.sd_960h_md				= 1;	// [6] SD_960H_MD = 1:SD 960H mode, 0:SD 720H mode
		cfg.fld.sd_outfmt_md			= 1;	// [5] SD_OUTFMT_MD = 1:SD mode, 0:HD mode
		cfg.fld.sd_fld_pol				= 1;	// [4] SD_FLD_POL = 0:Even High, 1:Odd High
		cfg.fld.outfmt_16bit			= 0;	// [3] OUTFMT_16BIT = 0:8 Bit, 1:16 bit
  		cfg.fld.outfmt_bt656			= 1;	// [2] OUTFMT_BT656 = 0:1120 Mode, 1:656 Mode
		cfg.fld.outfmt_yc_inv			= 0;	// [1] OUTFMT_YC_INV = 1:swap Y/C, 0:Normal Y/C
		cfg.fld.bt1120_limit_digital	= 0;	// [0] BT1120_LIM_DIG = 1:data limited to 16-240, 0:data limited to 1-254
		
		cfg1.fld.bayer_bit_width		= 0;	// [23] bayer_bit_width = 0: 10bit, 1 : 8bit
		cfg1.fld.pat_en 				= 0;	// [22] pat_en
		cfg1.fld.dis_clko_phase 		= 0;	// [21:16] dis_clko_phase
		cfg1.fld.pat_md 				= 0;	// [15:12] pat_md
		cfg1.fld.pat_sel				= 0;	// [11:8] pat_sel
		cfg1.fld.bto_vsync_delay		= 0;	// [7:4] BTO_VAV_DEL = VAV Delay (Capture Option..)
		cfg1.fld.byte_order				= 0;	// [3:1] byte_order = 000{RGB}, 001{GBR}, 010{BRG}, 100{RBG}, 101{GRB}, 110{BGR}
		cfg1.fld.bt1120_lim_ana			= 0;	// [0] bt1120_lim_ana = 0:1-254, 1:16-240
		
		gpDU->bto.ctrl0 = cfg.var;
		gpDU->bto.ctrl1 = cfg1.var;
	}
	else if( (out_resolution== eRESOLUTION_960Hi_PAL) || (out_resolution == eRESOLUTION_960Hp_PAL) )
	{
		cfg.fld.bt1120_vmask_md_ana		= 0;	// [27] bt1120_vmask_md_ana = 0 : ANA YC H&V Mask Mode, 1 : ANA YC H Only Mask Mode
		cfg.fld.bt1120_vmask_md_dig		= 0;	// [26] bt1120_vmask_md_dig = 0 : DIG YC H&V Mask Mode, 1 : DIG YC H Only Mask Mode
		cfg.fld.dis_hd_csc_md			= 0;	// [25:24] dis_hd_csc_md = 0,1 : Bypass, 2 : BT601 to BT709, 3 : BT709 to BT601
		cfg.fld.bto_hav_del				= 3;	// [23:20] bto_hav_del = 0 ~ 15:External Hsync Delay Control (Recommend : 4'd4)
		cfg.fld.outfmt_cbcr_inv			= 0;	// [18] outfmt_cbcr_inv = 0:Normal CbCr, 1:Swap CbCr
		if( (in_resolution == eRESOLUTION_960Hp_PAL) && (out_resolution == eRESOLUTION_960Hi_PAL) )
			cfg.fld.sd_p2i_en			= 1;	// [17] sd_p2i_en = 0:SD P2I Off, 1:SD P2I On
		else
			cfg.fld.sd_p2i_en			= 0;	// [17] sd_p2i_en = 0:SD P2I Off, 1:SD P2I On
		cfg.fld.clpf					= 1;	// [16] clpf = 1:Chroma LPF enable, 0:No Filtering
		cfg.fld.dis_async_chg_en		= 0;	// [15] dis_async_chg_en
		cfg.fld.dis_async_en			= 1;	// [14] dis_async_en
		cfg.fld.outfmt_hav_vblk			= 0;	// [13] outfmt_hav_vblk = 0 : Normal Mode, 1 : No HAV in VBLANK duration
		cfg.fld.bto_de_md				= 1;	// [12] bto_de_md = 0:data_enable mode, 1:field mode
		cfg.fld.bayer_out_md			= 0;	// [11:10] bayer_out_md = 0:R, 1:Gr, 2:Gb, 3:B
		cfg.fld.bto_dout_md 			= 0;	// [9:8] bto_dout_md = 0:YC, 1:RGB, 2:Bayer (LSB Mapping), 3:Bayer (MSB Mapping)
		cfg.fld.sd_pal_ntb				= 1;	// [7] SD_PAL_NTB = 0:NTSC, 1:PAL
		cfg.fld.sd_960h_md				= 1;	// [6] SD_960H_MD = 1:SD 960H mode, 0:SD 720H mode
		cfg.fld.sd_outfmt_md			= 1;	// [5] SD_OUTFMT_MD = 1:SD mode, 0:HD mode
		cfg.fld.sd_fld_pol				= 1;	// [4] SD_FLD_POL = 0:Even High, 1:Odd High
		cfg.fld.outfmt_16bit			= 0;	// [3] OUTFMT_16BIT = 0:8 Bit, 1:16 bit
  		cfg.fld.outfmt_bt656			= 1;	// [2] OUTFMT_BT656 = 0:1120 Mode, 1:656 Mode
		cfg.fld.outfmt_yc_inv			= 0;	// [1] OUTFMT_YC_INV = 1:swap Y/C, 0:Normal Y/C
		cfg.fld.bt1120_limit_digital	= 0;	// [0] BT1120_LIM_DIG = 1:data limited to 16-240, 0:data limited to 1-254
		
		cfg1.fld.bayer_bit_width		= 0;	// [23] bayer_bit_width = 0: 10bit, 1 : 8bit
		cfg1.fld.pat_en 				= 0;	// [22] pat_en
		cfg1.fld.dis_clko_phase 		= 0;	// [21:16] dis_clko_phase
		cfg1.fld.pat_md 				= 0;	// [15:12] pat_md
		cfg1.fld.pat_sel				= 0;	// [11:8] pat_sel
		cfg1.fld.bto_vsync_delay		= 0;	// [7:4] BTO_VAV_DEL = VAV Delay (Capture Option..)
		cfg1.fld.byte_order				= 0;	// [3:1] byte_order = 000{RGB}, 001{GBR}, 010{BRG}, 100{RBG}, 101{GRB}, 110{BGR}
		cfg1.fld.bt1120_lim_ana			= 0;	// [0] bt1120_lim_ana = 0:1-254, 1:16-240
		
		gpDU->bto.ctrl0 = cfg.var;
		gpDU->bto.ctrl1 = cfg1.var;
	}
	else if	((out_resolution == eRESOLUTION_WVGA) || (out_resolution== eRESOLUTION_WSVGA) || (out_resolution == eRESOLUTION_720P) || (out_resolution== eRESOLUTION_960P) || (out_resolution == eRESOLUTION_1080P))
	{
		cfg.fld.bt1120_vmask_md_ana		= 0;	// [27] bt1120_vmask_md_ana = 0 : ANA YC H&V Mask Mode, 1 : ANA YC H Only Mask Mode
		cfg.fld.bt1120_vmask_md_dig		= 0;	// [26] bt1120_vmask_md_dig = 0 : DIG YC H&V Mask Mode, 1 : DIG YC H Only Mask Mode
		cfg.fld.dis_hd_csc_md			= 0;	// [25:24] dis_hd_csc_md = 0,1 : Bypass, 2 : BT601 to BT709, 3 : BT709 to BT601
		cfg.fld.bto_hav_del				= 3;	// [23:20] bto_hav_del = 0 ~ 15:External Hsync Delay Control (Recommend : 4'd4)
		cfg.fld.outfmt_cbcr_inv			= 0;	// [18] outfmt_cbcr_inv = 0:Normal CbCr, 1:Swap CbCr
		cfg.fld.sd_p2i_en				= 0;	// [17] sd_p2i_en = 0:SD P2I Off, 1:SD P2I On
		cfg.fld.clpf					= 1;	// [16] clpf = 1:Chroma LPF enable, 0:No Filtering
		cfg.fld.dis_async_chg_en		= 0;	// [15] dis_async_chg_en
		cfg.fld.dis_async_en			= 1;	// [14] dis_async_en
		cfg.fld.outfmt_hav_vblk			= 0;	// [13] outfmt_hav_vblk = 0 : Normal Mode, 1 : No HAV in VBLANK duration
		cfg.fld.bto_de_md				= 1;	// [12] bto_de_md = 0:data_enable mode, 1:field mode
		cfg.fld.bayer_out_md			= 0;	// [11:10] bayer_out_md = 0:R, 1:Gr, 2:Gb, 3:B
		cfg.fld.bto_dout_md 			= 0;	// [9:8] bto_dout_md = 0:YC, 1:RGB, 2:Bayer (LSB Mapping), 3:Bayer (MSB Mapping)
		cfg.fld.sd_pal_ntb				= 0;	// [7] SD_PAL_NTB = 0:NTSC, 1:PAL
		cfg.fld.sd_960h_md				= 0;	// [6] SD_960H_MD = 1:SD 960H mode, 0:SD 720H mode
		cfg.fld.sd_outfmt_md			= 0;	// [5] SD_OUTFMT_MD = 1:SD mode, 0:HD mode
		cfg.fld.sd_fld_pol				= 0;	// [4] SD_FLD_POL = 0:Even High, 1:Odd High
		cfg.fld.outfmt_16bit			= 0;	// [3] OUTFMT_16BIT = 0:8 Bit, 1:16 bit
  		cfg.fld.outfmt_bt656			= 1;	// [2] OUTFMT_BT656 = 0:1120 Mode, 1:656 Mode
		cfg.fld.outfmt_yc_inv			= 0;	// [1] OUTFMT_YC_INV = 1:swap Y/C, 0:Normal Y/C
		cfg.fld.bt1120_limit_digital	= 0;	// [0] BT1120_LIM_DIG = 1:data limited to 16-240, 0:data limited to 1-254
		
		cfg1.fld.bayer_bit_width		= 0;	// [23] bayer_bit_width = 0: 10bit, 1 : 8bit
		cfg1.fld.pat_en 				= 0;	// [22] pat_en
		if(out_resolution == eRESOLUTION_1080P)
			cfg1.fld.dis_clko_phase 	= 8;	// [21:16] dis_clko_phase
		else
			cfg1.fld.dis_clko_phase 	= 0;	// [21:16] dis_clko_phase
		cfg1.fld.pat_md 				= 0;	// [15:12] pat_md
		cfg1.fld.pat_sel				= 0;	// [11:8] pat_sel
		cfg1.fld.bto_vsync_delay		= 0;	// [7:4] BTO_VAV_DEL = VAV Delay (Capture Option..)
		cfg1.fld.byte_order				= 0;	// [3:1] byte_order = 000{RGB}, 001{GBR}, 010{BRG}, 100{RBG}, 101{GRB}, 110{BGR}
		cfg1.fld.bt1120_lim_ana			= 0;	// [0] bt1120_lim_ana = 0:1-254, 1:16-240
		
		gpDU->bto.ctrl0 = cfg.var;
		gpDU->bto.ctrl1 = cfg1.var;
	}
	else
	{
		printf("Not Support!\n");
	}
}

PP_VOID PPDRV_DU_BTO_SetYC16(PP_DU_RESOLUTION_E IN resolution, PP_DU_SYNC_E IN sync)
{
	PP_DU_BTO_CONFIG_U cfg;
	PP_DU_BTO_CONFIG1_U cfg1;

	if (resolution >= eRESOLUTION_MAX)
	{
		printf("resolution is invalid!\n");
		return;
	}

	cfg.var = 0x0;
	cfg1.var = 0x0;
	
	if( (resolution == eRESOLUTION_WVGA) || (resolution == eRESOLUTION_WSVGA) || (resolution == eRESOLUTION_720P) || (resolution == eRESOLUTION_960P) || (resolution == eRESOLUTION_1080P) )
	{
		cfg.fld.bt1120_vmask_md_ana		= 0;	// [27] bt1120_vmask_md_ana = 0 : ANA YC H&V Mask Mode, 1 : ANA YC H Only Mask Mode
		cfg.fld.bt1120_vmask_md_dig		= 0;	// [26] bt1120_vmask_md_dig = 0 : DIG YC H&V Mask Mode, 1 : DIG YC H Only Mask Mode
		cfg.fld.dis_hd_csc_md			= 0;	// [25:24] dis_hd_csc_md = 0,1 : Bypass, 2 : BT601 to BT709, 3 : BT709 to BT601
		cfg.fld.bto_hav_del				= 6;	// [23:20] bto_hav_del = 0 ~ 15:External Hsync Delay Control (Recommend : 4'd4)
   		cfg.fld.outfmt_cbcr_inv			= 0;	// [18] outfmt_cbcr_inv = 0:Normal CbCr, 1:Swap CbCr
		cfg.fld.sd_p2i_en				= 0;	// [17] sd_p2i_en = 0:SD P2I Off, 1:SD P2I On
		cfg.fld.clpf					= 1;	// [16] clpf = 1:Chroma LPF enable, 0:No Filtering
		cfg.fld.dis_async_chg_en		= 0;	// [15] dis_async_chg_en
		cfg.fld.dis_async_en			= 1;	// [14] dis_async_en
		cfg.fld.outfmt_hav_vblk			= 0;	// [13] outfmt_hav_vblk = 0 : Normal Mode, 1 : No HAV in VBLANK duration
		cfg.fld.bto_de_md				= 1;	// [12] bto_de_md = 0:data_enable mode, 1:field mode
		cfg.fld.bayer_out_md			= 0;	// [11:10] bayer_out_md = 0:R, 1:Gr, 2:Gb, 3:B
		cfg.fld.bto_dout_md 			= 0;	// [9:8] bto_dout_md = 0:YC, 1:RGB, 2:Bayer (LSB Mapping), 3:Bayer (MSB Mapping)
		cfg.fld.sd_pal_ntb				= 0;	// [7] SD_PAL_NTB = 0:NTSC, 1:PAL
		cfg.fld.sd_960h_md				= 0;	// [6] SD_960H_MD = 1:SD 960H mode, 0:SD 720H mode
		cfg.fld.sd_outfmt_md			= 0;	// [5] SD_OUTFMT_MD = 1:SD mode, 0:HD mode
		cfg.fld.sd_fld_pol				= 0;	// [4] SD_FLD_POL = 0:Even High, 1:Odd High
		cfg.fld.outfmt_16bit			= 1;	// [3] OUTFMT_16BIT = 0:8 Bit, 1:16 bit
  		cfg.fld.outfmt_bt656			= 0;	// [2] OUTFMT_BT656 = 0:1120 Mode, 1:656 Mode
  		cfg.fld.outfmt_yc_inv		 	= 0;	// [1] OUTFMT_YC_INV = 1:swap Y/C, 0:Normal Y/C
		cfg.fld.bt1120_limit_digital	= 0;	// [0] BT1120_LIM_DIG = 1:data limited to 16-240, 0:data limited to 1-254
		
		cfg1.fld.bayer_bit_width		= 0;	// [23] bayer_bit_width = 0: 10bit, 1 : 8bit
		cfg1.fld.pat_en 				= 0;	// [22] pat_en
		cfg1.fld.dis_clko_phase 		= 0;	// [21:16] dis_clko_phase
		cfg1.fld.pat_md 				= 0;	// [15:12] pat_md
		cfg1.fld.pat_sel				= 0;	// [11:8] pat_sel
		cfg1.fld.bto_vsync_delay		= 0;	// [7:4] BTO_VAV_DEL = VAV Delay (Capture Option..)
		cfg1.fld.byte_order				= 0;	// [3:1] byte_order = 000{RGB}, 001{GBR}, 010{BRG}, 100{RBG}, 101{GRB}, 110{BGR}
		cfg1.fld.bt1120_lim_ana			= 0;	// [0] bt1120_lim_ana = 0:1-254, 1:16-240
		
		gpDU->bto.ctrl0 = cfg.var;
		gpDU->bto.ctrl1 = cfg1.var;
	}
	else
	{
		printf("Not Support!\n");
	}
}

PP_VOID PPDRV_DU_BTO_SetRGB24 (PP_DU_RESOLUTION_E IN resolution)
{
	PP_DU_BTO_CONFIG_U cfg;
	PP_DU_BTO_CONFIG1_U cfg1;

	if (resolution >= eRESOLUTION_MAX)
	{
		printf("resolution is invalid!\n");
		return;
	}

	cfg.var = 0x0;
	cfg1.var = 0x0;
	
	if( (resolution == eRESOLUTION_WVGA) || (resolution == eRESOLUTION_WSVGA) || (resolution == eRESOLUTION_720P) || (resolution == eRESOLUTION_960P) || (resolution == eRESOLUTION_1080P) )
	{
		cfg.fld.bt1120_vmask_md_ana		= 0;	// [27] bt1120_vmask_md_ana = 0 : ANA YC H&V Mask Mode, 1 : ANA YC H Only Mask Mode
		cfg.fld.bt1120_vmask_md_dig		= 0;	// [26] bt1120_vmask_md_dig = 0 : DIG YC H&V Mask Mode, 1 : DIG YC H Only Mask Mode
		cfg.fld.dis_hd_csc_md			= 0;	// [25:24] dis_hd_csc_md = 0,1 : Bypass, 2 : BT601 to BT709, 3 : BT709 to BT601
		cfg.fld.bto_hav_del				= 0;	// [23:20] bto_hav_del = 0 ~ 15:External Hsync Delay Control (Recommend : 4'd4)
		cfg.fld.outfmt_cbcr_inv			= 0;	// [18] outfmt_cbcr_inv = 0:Normal CbCr, 1:Swap CbCr
		cfg.fld.sd_p2i_en				= 0;	// [17] sd_p2i_en = 0:SD P2I Off, 1:SD P2I On
		cfg.fld.clpf					= 1;	// [16] clpf = 1:Chroma LPF enable, 0:No Filtering
		cfg.fld.dis_async_chg_en		= 0;	// [15] dis_async_chg_en
		cfg.fld.dis_async_en			= 1;	// [14] dis_async_en
		cfg.fld.outfmt_hav_vblk			= 0;	// [13] outfmt_hav_vblk = 0 : Normal Mode, 1 : No HAV in VBLANK duration
		cfg.fld.bto_de_md				= 0;	// [12] bto_de_md = 0:data_enable mode, 1:field mode
		cfg.fld.bayer_out_md			= 0;	// [11:10] bayer_out_md = 0:R, 1:Gr, 2:Gb, 3:B
		cfg.fld.bto_dout_md 			= 1;	// [9:8] bto_dout_md = 0:YC, 1:RGB, 2:Bayer (LSB Mapping), 3:Bayer (MSB Mapping)
		cfg.fld.sd_pal_ntb				= 0;	// [7] SD_PAL_NTB = 0:NTSC, 1:PAL
		cfg.fld.sd_960h_md				= 0;	// [6] SD_960H_MD = 1:SD 960H mode, 0:SD 720H mode
		cfg.fld.sd_outfmt_md			= 0;	// [5] SD_OUTFMT_MD = 1:SD mode, 0:HD mode
		cfg.fld.sd_fld_pol				= 0;	// [4] SD_FLD_POL = 0:Even High, 1:Odd High
		cfg.fld.outfmt_16bit			= 1;	// [3] OUTFMT_16BIT = 0:8 Bit, 1:16 bit
  		cfg.fld.outfmt_bt656			= 0;	// [2] OUTFMT_BT656 = 0:1120 Mode, 1:656 Mode
  		cfg.fld.outfmt_yc_inv		 	= 0;	// [1] OUTFMT_YC_INV = 1:swap Y/C, 0:Normal Y/C
		cfg.fld.bt1120_limit_digital	= 0;	// [0] BT1120_LIM_DIG = 1:data limited to 16-240, 0:data limited to 1-254

		cfg1.fld.bayer_bit_width		= 0;	// [23] bayer_bit_width = 0: 10bit, 1 : 8bit
		cfg1.fld.pat_en 				= 0;	// [22] pat_en
		cfg1.fld.dis_clko_phase 		= 0;	// [21:16] dis_clko_phase
		cfg1.fld.pat_md 				= 0;	// [15:12] pat_md
		cfg1.fld.pat_sel				= 0;	// [11:8] pat_sel
		cfg1.fld.bto_vsync_delay		= 0;	// [7:4] BTO_VAV_DEL = VAV Delay (Capture Option..)
		#if 0	// Default
		cfg1.fld.byte_order				= 0;	// [3:1] byte_order = 000{RGB}, 001{GBR}, 010{BRG}, 100{RBG}, 101{GRB}, 110{BGR}
		#else	// HDMI convert board
		cfg1.fld.byte_order				= 6;	// [3:1] byte_order = 000{RGB}, 001{GBR}, 010{BRG}, 100{RBG}, 101{GRB}, 110{BGR} 
		#endif
		cfg1.fld.bt1120_lim_ana			= 0;	// [0] bt1120_lim_ana = 0:1-254, 1:16-240
		
		gpDU->bto.ctrl0 = cfg.var;
		gpDU->bto.ctrl1 = cfg1.var;
	}
	else
	{
		printf("Not Support!\n");
	}
}

PP_VOID PPDRV_DU_BTO_SetBayer (PP_DU_RESOLUTION_E IN resolution, PP_DU_BAYER_BIT_E IN bit)
{
	PP_DU_BTO_CONFIG_U cfg;
	PP_DU_BTO_CONFIG1_U cfg1;

	if (resolution >= eRESOLUTION_MAX)
	{
		printf("resolution is invalid!\n");
		return;
	}

	cfg.var = 0x0;
	cfg1.var = 0x0;
	
	if( (resolution == eRESOLUTION_720P) || (resolution == eRESOLUTION_960P) || (resolution == eRESOLUTION_1080P) )
	{
		cfg.fld.bt1120_vmask_md_ana		= 0;	// [27] bt1120_vmask_md_ana = 0 : ANA YC H&V Mask Mode, 1 : ANA YC H Only Mask Mode
		cfg.fld.bt1120_vmask_md_dig		= 0;	// [26] bt1120_vmask_md_dig = 0 : DIG YC H&V Mask Mode, 1 : DIG YC H Only Mask Mode
		cfg.fld.dis_hd_csc_md			= 0;	// [25:24] dis_hd_csc_md = 0,1 : Bypass, 2 : BT601 to BT709, 3 : BT709 to BT601
		cfg.fld.bto_hav_del				= 0;	// [23:20] bto_hav_del = 0 ~ 15:External Hsync Delay Control (Recommend : 4'd4)
		cfg.fld.outfmt_cbcr_inv			= 0;	// [18] outfmt_cbcr_inv = 0:Normal CbCr, 1:Swap CbCr
		cfg.fld.sd_p2i_en				= 0;	// [17] sd_p2i_en = 0:SD P2I Off, 1:SD P2I On
		cfg.fld.clpf					= 1;	// [16] clpf = 1:Chroma LPF enable, 0:No Filtering
		cfg.fld.dis_async_chg_en		= 0;	// [15] dis_async_chg_en
		cfg.fld.dis_async_en			= 1;	// [14] dis_async_en
		cfg.fld.outfmt_hav_vblk			= 0;	// [13] outfmt_hav_vblk = 0 : Normal Mode, 1 : No HAV in VBLANK duration
		cfg.fld.bto_de_md				= 0;	// [12] bto_de_md = 0:data_enable mode, 1:field mode
		cfg.fld.bayer_out_md			= 0;	// [11:10] bayer_out_md = 0:R, 1:Gr, 2:Gb, 3:B
		if (bit == eBAYER_8BIT)
			cfg.fld.bto_dout_md 		= 2;	// [9:8] bto_dout_md = 0:YC, 1:RGB, 2:Bayer (LSB Mapping), 3:Bayer (MSB Mapping)
		else if (bit == eBAYER_10BIT)
			cfg.fld.bto_dout_md 		= 3;	// [9:8] bto_dout_md = 0:YC, 1:RGB, 2:Bayer (LSB Mapping), 3:Bayer (MSB Mapping)
		else
			cfg.fld.bto_dout_md 		= 0;	// [9:8] bto_dout_md = 0:YC, 1:RGB, 2:Bayer (LSB Mapping), 3:Bayer (MSB Mapping)
		cfg.fld.sd_pal_ntb				= 0;	// [7] SD_PAL_NTB = 0:NTSC, 1:PAL
		cfg.fld.sd_960h_md				= 0;	// [6] SD_960H_MD = 1:SD 960H mode, 0:SD 720H mode
		cfg.fld.sd_outfmt_md			= 0;	// [5] SD_OUTFMT_MD = 1:SD mode, 0:HD mode
		cfg.fld.sd_fld_pol				= 0;	// [4] SD_FLD_POL = 0:Even High, 1:Odd High
		cfg.fld.outfmt_16bit			= 1;	// [3] OUTFMT_16BIT = 0:8 Bit, 1:16 bit
  		cfg.fld.outfmt_bt656			= 0;	// [2] OUTFMT_BT656 = 0:1120 Mode, 1:656 Mode
  		cfg.fld.outfmt_yc_inv		 	= 0;	// [1] OUTFMT_YC_INV = 1:swap Y/C, 0:Normal Y/C
		cfg.fld.bt1120_limit_digital	= 0;	// [0] BT1120_LIM_DIG = 1:data limited to 16-240, 0:data limited to 1-254
		
		if (bit == eBAYER_8BIT)
			cfg1.fld.bayer_bit_width	= 1;	// [23] bayer_bit_width = 0: 10bit, 1 : 8bit
		else if (bit == eBAYER_10BIT)
			cfg1.fld.bayer_bit_width	= 0;	// [23] bayer_bit_width = 0: 10bit, 1 : 8bit
		else
			cfg1.fld.bayer_bit_width	= 0;	// [23] bayer_bit_width = 0: 10bit, 1 : 8bit
		cfg1.fld.pat_en					= 0;	// [22] pat_en
		cfg1.fld.dis_clko_phase 		= 0;	// [21:16] dis_clko_phase
		cfg1.fld.pat_md 				= 0;	// [15:12] pat_md
		cfg1.fld.pat_sel				= 0;	// [11:8] pat_sel
		cfg1.fld.bto_vsync_delay		= 0;	// [7:4] BTO_VAV_DEL = VAV Delay (Capture Option..)
		if (bit == eBAYER_8BIT)
			cfg1.fld.byte_order 		= 4;	// [3:1] byte_order = 000{RGB}, 001{GBR}, 010{BRG}, 100{RBG}, 101{GRB}, 110{BGR}
		else if (bit == eBAYER_10BIT)
			cfg1.fld.byte_order 		= 5;	// [3:1] byte_order = 000{RGB}, 001{GBR}, 010{BRG}, 100{RBG}, 101{GRB}, 110{BGR}
		else
			cfg1.fld.byte_order 		= 0;	// [3:1] byte_order = 000{RGB}, 001{GBR}, 010{BRG}, 100{RBG}, 101{GRB}, 110{BGR}
		cfg1.fld.bt1120_lim_ana			= 0;	// [0] bt1120_lim_ana = 0:1-254, 1:16-240

		gpDU->bto.ctrl0 = cfg.var;
		gpDU->bto.ctrl1 = cfg1.var;
	}
	else
	{
		printf("Not Support!\n");
	}
}

PP_VOID PPDRV_DU_BTO_SetCSC(PP_DU_CSC_E IN csc)
{
	PP_DU_BTO_CONFIG_U cfg;

	cfg.var = gpDU->bto.ctrl0;

	if(csc == eCSC_BYPASS)
		cfg.fld.dis_hd_csc_md			= 0;	// [25:24] dis_hd_csc_md = 0,1 : Bypass, 2 : BT601 to BT709, 3 : BT709 to BT601
	else if(csc == eCSC_BT601_to_BT709)
		cfg.fld.dis_hd_csc_md			= 2;	// [25:24] dis_hd_csc_md = 0,1 : Bypass, 2 : BT601 to BT709, 3 : BT709 to BT601
	else if(csc == eCSC_BT709_to_BT601)
		cfg.fld.dis_hd_csc_md			= 3;	// [25:24] dis_hd_csc_md = 0,1 : Bypass, 2 : BT601 to BT709, 3 : BT709 to BT601
		
	gpDU->bto.ctrl0 = cfg.var;
}

PP_VOID PPDRV_DU_BTO_EnableBrightness (PP_BOOL IN enable)
{
	PP_DU_BTO_CONFIG2_U cfg2;

	cfg2.var = gpDU->bto.ctrl2;
	cfg2.fld.dis_dig_brt_en = enable;
	gpDU->bto.ctrl2 = cfg2.var;
}

PP_U8 PPDRV_DU_BTO_GetBrightness (PP_VOID)
{
	PP_DU_BTO_CONFIG2_U cfg2;
	cfg2.var = gpDU->bto.ctrl2;
	return cfg2.fld.dis_dig_brt;
}

PP_RESULT_E PPDRV_DU_BTO_SetBrightness (PP_U8 IN brt)
{
	PP_DU_BTO_CONFIG2_U cfg2;

	// Range : (0(-50%) ~ 128(0%) ~ 256(+50%))
	if(brt > 0xFF)
	{
		printf("Invalid Parameter!\n");
		return eERROR_INVALID_ARGUMENT;
	}

	cfg2.var = gpDU->bto.ctrl2;
	cfg2.fld.dis_dig_brt = brt;
	gpDU->bto.ctrl2 = cfg2.var;

	return eSUCCESS;
}

PP_VOID PPDRV_DU_BTO_EnableContrast (PP_BOOL IN enable)
{
	PP_DU_BTO_CONFIG2_U cfg2;

	cfg2.var = gpDU->bto.ctrl2;
	cfg2.fld.dis_dig_cont_en = enable;
	gpDU->bto.ctrl2 = cfg2.var;
}

PP_U8 PPDRV_DU_BTO_GetContrast (PP_VOID)
{
	PP_DU_BTO_CONFIG2_U cfg2;
	cfg2.var = gpDU->bto.ctrl2;
	return cfg2.fld.dis_dig_cont;
}

PP_RESULT_E PPDRV_DU_BTO_SetContrast (PP_U8 IN cont)
{
	PP_DU_BTO_CONFIG2_U cfg2;

	// Range : (0(x0) ~ 128(x1) ~ 256(x2))
	if(cont > 0xFF)
	{
		printf("Invalid Parameter!\n");
		return eERROR_INVALID_ARGUMENT;
	}

	cfg2.var = gpDU->bto.ctrl2;
	cfg2.fld.dis_dig_cont = cont;
	gpDU->bto.ctrl2 = cfg2.var;

	return eSUCCESS;
}

PP_VOID PPDRV_DU_BTO_EnableSaturation (PP_BOOL IN enable)
{
	PP_DU_BTO_CONFIG2_U cfg2;

	cfg2.var = gpDU->bto.ctrl2;
	cfg2.fld.dis_dig_sat_en = enable;
	gpDU->bto.ctrl2 = cfg2.var;
}

PP_U8 PPDRV_DU_BTO_GetSaturation (PP_VOID)
{
	PP_DU_BTO_CONFIG2_U cfg2;
	cfg2.var = gpDU->bto.ctrl2;
	return cfg2.fld.dis_dig_sat;
}

PP_RESULT_E PPDRV_DU_BTO_SetSaturation (PP_U8 IN sat)
{
	PP_DU_BTO_CONFIG2_U cfg2;

	// Range : (0(x0) ~ 128(x1) ~ 256(x2))
	if(sat > 0xFF)
	{
		printf("Invalid Parameter!\n");
		return eERROR_INVALID_ARGUMENT;
	}
	
	cfg2.var = gpDU->bto.ctrl2;
	cfg2.fld.dis_dig_sat = sat;
	gpDU->bto.ctrl2 = cfg2.var;

	return eSUCCESS;
}

PP_VOID PPDRV_DU_PVITX_SelSrc (PP_PVITX_SRC IN src)
{
	PP_U32 reg;
	PP_DU_BTO_CONFIG_U cfg;

	if(src == ePVITX_SRC_DU)
	{
		gpDU->ctrl.ctrl1 &= ~(0x000001F0);
		gpDU->ctrl.ctrl1 |= (3<<4);

		/* Select PVI Tx BT656/BT1120 Mode */
		reg = GetRegValue(0xF0901840); //TVI Tx reg:0x10
		reg &= 0xF8;
		cfg.var = gpDU->bto.ctrl0;
		if(cfg.fld.sd_outfmt_md == 1)	// [5] SD_OUTFMT_MD = 1:SD mode, 0:HD mode
		{
			reg |= 0x0;	//Set BT656
		}
		else
		{
			reg |= 0x1;	//Set BT1120
		}
		SetRegValue(0xF0901840, reg);
	}
	else if(src == ePVITX_SRC_QUAD)
	{
		gpDU->ctrl.ctrl1 &= ~(0x000001F0);
		gpDU->ctrl.ctrl1 |= (1<<8) | (3<<6);

		/* Select PVI Tx BT656 Mode */
		reg = GetRegValue(0xF0901840); //TVI Tx reg:0x10
		reg &= 0xF8;
		cfg.var = gpDU->bto.ctrl0;
		if(cfg.fld.sd_outfmt_md == 1)	// [5] SD_OUTFMT_MD = 1:SD mode, 0:HD mode
		{
		reg |= 0x0;	//Set BT656
		}
		else
		{
			reg |= 0x1;	//Set BT1120
		}
		SetRegValue(0xF0901840, reg);
	}
	else
	{
		printf("Invalid PVI TX SRC!\n");
	}
}

/* 3. Set Mixer */
PP_VOID PPDRV_DU_OSD_SetMixerPath (PP_DU_MIXER_PATH_E path)
{
	gpDU->osd.ctrl.bus_bw1 = 0x7f7f7f7f;	// default
	gpDU->osd.ctrl.bus_bw = 0x7f3f3f3f;		// default

	switch(path)
	{
		case ePATH_IN_bypass_OUT:	gpDU->osd.ctrl.src_sel = 0x76543210;	break;	// INPUT -> bypass/no_mixing -> OUTPUT
		
		case ePATH_IN_01234_OUT:	gpDU->osd.ctrl.src_sel = 0x76432105;	break;	// INPUT -> RLE0 -> RLE1 -> RLE2 -> RLE3 -> RLE4 -> OUTPUT (RGB format use upper layer)
		case ePATH_IN_12340_OUT:	gpDU->osd.ctrl.src_sel = 0x76432051;	break;	// INPUT -> RLE1 -> RLE2 -> RLE3 -> RLE4 -> RLE0 -> OUTPUT (RGB format use lower layer)
		case ePATH_IN_23401_OUT:	gpDU->osd.ctrl.src_sel = 0x76430152;	break;	// INPUT -> RLE2 -> RLE3 -> RLE4 -> RLE0 -> RLE1 -> OUTPUT
		case ePATH_IN_34012_OUT:	gpDU->osd.ctrl.src_sel = 0x76402153;	break;	// INPUT -> RLE3 -> RLE4 -> RLE0 -> RLE1 -> RLE2 -> OUTPUT
		case ePATH_IN_40123_OUT:	gpDU->osd.ctrl.src_sel = 0x76032154;	break;	// INPUT -> RLE4 -> RLE0 -> RLE1 -> RLE2 -> RLE3 -> OUTPUT
		
		case ePATH_IN_43210_OUT:	gpDU->osd.ctrl.src_sel = 0x76054321;	break;	// INPUT -> RLE4 -> RLE3 -> RLE2 -> RLE1 -> RLE0 -> OUTPUT
		case ePATH_IN_32104_OUT:	gpDU->osd.ctrl.src_sel = 0x76104325;	break;	// INPUT -> RLE3 -> RLE2 -> RLE1 -> RLE0 -> RLE4 -> OUTPUT
		case ePATH_IN_21043_OUT:	gpDU->osd.ctrl.src_sel = 0x76150324;	break;	// INPUT -> RLE2 -> RLE1 -> RLE0 -> RLE4 -> RLE3 -> OUTPUT
		case ePATH_IN_10432_OUT:	gpDU->osd.ctrl.src_sel = 0x76154023;	break;	// INPUT -> RLE1 -> RLE0 -> RLE4 -> RLE3 -> RLE2 -> OUTPUT
		case ePATH_IN_04321_OUT:	gpDU->osd.ctrl.src_sel = 0x76154302;	break;	// INPUT -> RLE0 -> RLE4 -> RLE3 -> RLE2 -> RLE1 -> OUTPUT
		
		case ePATH_IN_32140_OUT:	gpDU->osd.ctrl.src_sel = 0x76204351;	break;	// INPUT -> RLE3 -> RLE2 -> RLE1 -> RLE4 -> RLE0 -> OUTPUT
        default: break;
	}
	
#if 0
	// for test
	gpDU->osd.ctrl.src_sel = 0x76543201;	// INPUT -> RLE0 -> OUTPUT	
#endif
}

PP_VOID PPDRV_DU_OSD_SetLayerSize (PP_DU_LAYER_E IN layer, PP_RECT_S IN rect)
{
	// TBD. canvas size exception (RECOMMENT to set it to same as Input Image Size)
	gpDU->osd.rle[layer].canvas.canvas_size = (rect.u16Height << 16) | rect.u16Width;
}

PP_RECT_S PPDRV_DU_OSD_GetLayerSize (PP_DU_LAYER_E IN layer)
{
	PP_RECT_S rect;

	rect.u16X = 0;
	rect.u16Y = 0;
	rect.u16Height = (gpDU->osd.rle[layer].canvas.canvas_size >> 16) & 0xFFFF;
	rect.u16Width =  gpDU->osd.rle[layer].canvas.canvas_size & 0xFFFF;
	
	return rect;
}

PP_VOID PPDRV_DU_OSD_SetLayerColor (PP_DU_LAYER_E IN layer, PP_U32 IN color)
{
	gpDU->osd.rle[layer].canvas.canvas_color = color;	// ARGB
}

PP_U32 PPDRV_DU_OSD_GetLayercolor (PP_DU_LAYER_E IN layer)
{
	return gpDU->osd.rle[layer].canvas.canvas_color;	// ARGB
}

PP_RESULT_E PPDRV_DU_OSD_SetLayerFormat (PP_DU_LAYER_E IN layer, PP_DU_FORMAT_E IN format)
{
	PP_U32 layer_mode;
	
	if(format >= eFORMAT_MAX)
	{
		printf("Invalid format!\n");
		return eERROR_INVALID_ARGUMENT;
	}
	
	if( (format == eFORMAT_RGB565) || (format == eFORMAT_RGB888) || (format == eFORMAT_RGBA4444) || (format == eFORMAT_ARGB8888) )
	{
		if(layer < eLayer3)
		{
			printf("Not support format!\n");
			return eERROR_NOT_SUPPORT;
		}
	}

	layer_mode = gpDU->osd.rle[layer].canvas.layer_mode;

	// register block
	gpDU->osd.ctrl.reg_blocked = 1;

	layer_mode &= ~(0xF);
	if(format == eFORMAT_RLE)
	{
		layer_mode &= ~(1<<0);
	}
	else if(format == eFORMAT_INDEX)
	{
		layer_mode |= (0<<1) | (1<<0);
	}
	else if(format == eFORMAT_RGB565)
	{
		layer_mode |= (1<<1) | (1<<0);
	}
	else if(format == eFORMAT_RGB888)
	{
		layer_mode |= (2<<1) | (1<<0);
	}
	else if(format == eFORMAT_ARGB8888)
	{
		layer_mode |= (3<<1) | (1<<0);
	}
	else if(format == eFORMAT_RGBA4444)
	{
		layer_mode |= (5<<1) | (1<<0);
	}
	else if(format == eFORMAT_1BIT)
	{
		layer_mode |= (4<<1) | (1<<0);
	}
	
	gpDU->osd.rle[layer].canvas.layer_mode = layer_mode;
	
	// register block freeze
	gpDU->osd.ctrl.reg_blocked = 0;

	return eSUCCESS;
}

PP_DU_FORMAT_E PPDRV_DU_OSD_GetLayerFormat (PP_DU_LAYER_E IN layer)
{
	PP_DU_FORMAT_E format;
	
	if((gpDU->osd.rle[layer].canvas.layer_mode&0x1) == 0x0)
	{
		format = eFORMAT_RLE;
	}
	else if((gpDU->osd.rle[layer].canvas.layer_mode&0xF) == 0x1)
	{
		format = eFORMAT_INDEX;
	}
	else if((gpDU->osd.rle[layer].canvas.layer_mode&0xF) == 0x3)
	{
		format = eFORMAT_RGB565;
	}
	else if((gpDU->osd.rle[layer].canvas.layer_mode&0xF) == 0x5)
	{
		format = eFORMAT_RGB888;
	}
	else if((gpDU->osd.rle[layer].canvas.layer_mode&0xF) == 0x7)
	{
		format = eFORMAT_ARGB8888;
	}
	else if((gpDU->osd.rle[layer].canvas.layer_mode&0xF) == 0x9)
	{
		format = eFORMAT_1BIT;
	}
	else if((gpDU->osd.rle[layer].canvas.layer_mode&0xF) == 0xB)
	{
		format = eFORMAT_RGBA4444;
	}

	return format;
}

PP_VOID PPDRV_DU_OSD_EnableLayer2dDMA (PP_DU_LAYER_E IN layer, PP_BOOL IN enable)
{
	if(enable)
	{
		gpDU->osd.rle[layer].canvas.layer_mode |= (1<<5);
	}
	else
	{
		gpDU->osd.rle[layer].canvas.layer_mode &= ~(1<<5);
	}
}

PP_VOID PPDRV_DU_OSD_EnableLayerInterlace (PP_DU_LAYER_E IN layer, PP_BOOL IN enable)
{
	if(enable)
	{
		gpDU->osd.rle[layer].canvas.layer_mode |= (1<<7);
	}
	else
	{
		gpDU->osd.rle[layer].canvas.layer_mode &= ~(1<<7);
	}
}

PP_VOID PPDRV_DU_OSD_RunMixer (PP_VOID)
{
	gpDU->osd.tg.ctrl = 0x1;

	gDU_isInit = TRUE;
}

/* 4. Set Area */
PP_RESULT_E PPDRV_DU_OSD_GetColorLut (PP_DU_LAYER_E IN layer, PP_U32* IN lut)
{
	PP_U32 i;
	
	if ( gDU_isInit == PP_FALSE )
	{
		printf("eERROR_DU_NOT_INITIALIZE\n");
		return eERROR_DU_NOT_INITIALIZE;
	}

	gpDU->osd.tg.ctrl = 0x11;
	for(i=0 ; i<256 ; i++)
		lut[i] = gpDU->osd.rle[layer].lut.argb[i];
	gpDU->osd.tg.ctrl = 0x1;
	
	return eSUCCESS;
}

#include "dma.h"

PP_RESULT_E PPDRV_DU_OSD_SetColorLut (PP_DU_LAYER_E IN layer, PP_U32* IN lut)
{
	PP_U32 i;

	if ( gDU_isInit == PP_FALSE )
	{
		printf("eERROR_DU_NOT_INITIALIZE\n");
		return eERROR_DU_NOT_INITIALIZE;
	}

	if(lut == NULL)
	{
		printf("LUT is NULL!\n");
		return eERROR_INVALID_ARGUMENT;
	}

	gpDU->osd.tg.ctrl = 0x11;
#if 1
	for(i=0 ; i<256 ; i++)
		gpDU->osd.rle[layer].lut.argb[i] = lut[i];
#else
	PPAPI_SYS_CACHE_Writeback(lut, 1024);
	PPDRV_DMA_M2M_Word(0, lut, (PP_U32*)&gpDU->osd.rle[layer].lut, 1024, 3000);
#endif
	gpDU->osd.tg.ctrl = 0x1;

	return eSUCCESS;
}

PP_RESULT_E PPDRV_DU_OSD_EnableLayerGlobalAlpha (PP_DU_LAYER_E IN layer, PP_BOOL IN enable)
{
	// Notice : ignoring alpha value from each pixel for LUT/RGBA8888/RGBA4444 mode
	if(enable)
	{
		gpDU->osd.rle[layer].canvas.global_alpha |= (1<<8);
	}
	else
	{
		gpDU->osd.rle[layer].canvas.global_alpha &= ~(1<<8);
	}

	return eSUCCESS;
}

PP_RESULT_E PPDRV_DU_OSD_SetLayerGlobalAlpha (PP_DU_LAYER_E IN layer, PP_U8 IN value)
{
	gpDU->osd.rle[layer].canvas.global_alpha = (gpDU->osd.rle[layer].canvas.global_alpha & 0xFFFFFF00) | value;

	return eSUCCESS;
}

PP_RESULT_E PPDRV_DU_OSD_EnableArea (PP_DU_LAYER_E IN layer, PP_DU_AREA_E IN area, PP_BOOL IN enable)
{
	if ( gDU_isInit == PP_FALSE )
	{
		printf("eERROR_DU_NOT_INITIALIZE\n");
		return eERROR_DU_NOT_INITIALIZE;
	}
	
	if(enable)
	{
		gpDU->osd.rle[layer].area[area].config |= (1<<0);
	}
	else
	{
		gpDU->osd.rle[layer].area[area].config &= ~(1<<0);
	}

	return eSUCCESS;
}

#if 0 // ����
PP_RESULT_E PPDRV_DU_OSD_SetAreaConfig (PP_DU_LAYER_E IN layer, PP_DU_AREA_E IN area, PP_U32* IN addr, PP_U32 IN byte, PP_RECT_S IN rect, PP_U32 IN stride, PP_DU_FORMAT_E IN format)
{
	// check du init
	if ( gDU_isInit == PP_FALSE )
	{
		printf("eERROR_DU_NOT_INITIALIZE\n");
		return eERROR_DU_NOT_INITIALIZE;
	}
	
	// check spec (layer format must be same area format)
	if((gpDU->osd.rle[layer].canvas.layer_mode&0x1) == 0x0)
	{
		if(format != eFORMAT_RLE)
		{
			printf("Invalid area format!\n");
			return eERROR_INVALID_ARGUMENT;
		}
	}
	else if((gpDU->osd.rle[layer].canvas.layer_mode&0xF) == 0x1)
	{
		if(format != eFORMAT_INDEX)
		{
			printf("Invalid area format!\n");
			return eERROR_INVALID_ARGUMENT;
		}
	}
	else if((gpDU->osd.rle[layer].canvas.layer_mode&0xF) == 0x3)
	{
		if(format != eFORMAT_RGB565)
		{
			printf("Invalid area format!\n");
			return eERROR_INVALID_ARGUMENT;
		}
	}
	else if((gpDU->osd.rle[layer].canvas.layer_mode&0xF) == 0x5)
	{
		if(format != eFORMAT_RGB888)
		{
			printf("Invalid area format!\n");
			return eERROR_INVALID_ARGUMENT;
		}
	}
	else if((gpDU->osd.rle[layer].canvas.layer_mode&0xF) == 0x7)
	{
		if(format != eFORMAT_ARGB8888)
		{
			printf("Invalid area format!\n");
			return eERROR_INVALID_ARGUMENT;
		}
	}
	else if((gpDU->osd.rle[layer].canvas.layer_mode&0xF) == 0xB)
	{
		if(format != eFORMAT_RGBA4444)
		{
			printf("Invalid area format!\n");
			return eERROR_INVALID_ARGUMENT;
		}
	}
	else if((gpDU->osd.rle[layer].canvas.layer_mode&0xF) == 0x9)
	{
		if(format != eFORMAT_1BIT)
		{
			printf("Invalid area format!\n");
			return eERROR_INVALID_ARGUMENT;
		}
	}

	// check spec (RGB888 or ARGB8888 format must be half the width)
	if( ((gpDU->osd.rle[layer].canvas.layer_mode&0xF) == 0x5) || ((gpDU->osd.rle[layer].canvas.layer_mode&0xF) == 0x7) )
	{
		if( (gpDU->osd.rle[layer].canvas.canvas_size&0xFFFF)/2 < rect.u16Width)
		{
			printf("Not support area size!\n");
			return eERROR_NOT_SUPPORT;
		}
	}

	// check boundary //
	if(		(rect.u16X < 0)
		||	((rect.u16X + rect.u16Width) > ((gpDU->osd.rle[layer].canvas.canvas_size)&0xFFFF))
		||	(rect.u16Y < 0)
		||	((rect.u16Y + rect.u16Height) > ((gpDU->osd.rle[layer].canvas.canvas_size>>16)&0xFFFF))	)
	{
		printf("over boundary area point! (rect:%dx%d_%dx%d, canvas:%dx%d)\n", rect.u16X, rect.u16Y, rect.u16Width, rect.u16Height, ((gpDU->osd.rle[layer].canvas.canvas_size)&0xFFFF), ((gpDU->osd.rle[layer].canvas.canvas_size>>16)&0xFFFF) );
		return eERROR_INVALID_ARGUMENT;
	}

	// register block
	gpDU->osd.ctrl.reg_blocked = 1;

	// set area config//
	gpDU->osd.rle[layer].area[area].base_addr = (PP_U32)addr;
	gpDU->osd.rle[layer].area[area].image_size = (rect.u16Height << 16) | rect.u16Width;

	if((gpDU->osd.rle[layer].canvas.layer_mode >> 5) & 0x1)
	{
		// 2d-dma mode
		if(format == eFORMAT_RLE)
			gpDU->osd.rle[layer].area[area].byte = byte;
		else
			gpDU->osd.rle[layer].area[area].stride = stride;
	}
	else
	{
		// 1d-dma mode
		gpDU->osd.rle[layer].area[area].byte = byte;
	}

	if(area == eArea0)
	{
		gpDU->osd.rle[layer].canvas.area0_start = (rect.u16Y << 16) | rect.u16X;
	}
	else if(area == eArea1)
	{
		gpDU->osd.rle[layer].canvas.area1_start = (rect.u16Y << 16) | rect.u16X;
	}
	else if(area == eArea2)
	{
		gpDU->osd.rle[layer].canvas.area2_start = (rect.u16Y << 16) | rect.u16X;
	}
	else if(area == eArea3)
	{
		gpDU->osd.rle[layer].canvas.area3_start = (rect.u16Y << 16) | rect.u16X;
	}

	// register block freeze
	gpDU->osd.ctrl.reg_blocked = 0;

	return eSUCCESS;
}
#else
PP_RESULT_E PPDRV_DU_OSD_SetAreaConfig (PP_DU_LAYER_E IN layer, PP_DU_AREA_E IN area, PP_U32* IN addr, PP_U32* IN addr2, PP_U32 IN byte, PP_RECT_S IN rect, PP_U32 IN stride, PP_DU_FORMAT_E IN format)
{
	// check du init
	if ( gDU_isInit == PP_FALSE )
	{
		printf("eERROR_DU_NOT_INITIALIZE\n");
		return eERROR_DU_NOT_INITIALIZE;
	}
	
	// check spec (layer format must be same area format)
	if((gpDU->osd.rle[layer].canvas.layer_mode&0x1) == 0x0)
	{
		if(format != eFORMAT_RLE)
		{
			printf("Invalid area format!\n");
			return eERROR_INVALID_ARGUMENT;
		}
	}
	else if((gpDU->osd.rle[layer].canvas.layer_mode&0xF) == 0x1)
	{
		if(format != eFORMAT_INDEX)
		{
			printf("Invalid area format!\n");
			return eERROR_INVALID_ARGUMENT;
		}
	}
	else if((gpDU->osd.rle[layer].canvas.layer_mode&0xF) == 0x3)
	{
		if(format != eFORMAT_RGB565)
		{
			printf("Invalid area format!\n");
			return eERROR_INVALID_ARGUMENT;
		}
	}
	else if((gpDU->osd.rle[layer].canvas.layer_mode&0xF) == 0x5)
	{
		if(format != eFORMAT_RGB888)
		{
			printf("Invalid area format!\n");
			return eERROR_INVALID_ARGUMENT;
		}
	}
	else if((gpDU->osd.rle[layer].canvas.layer_mode&0xF) == 0x7)
	{
		if(format != eFORMAT_ARGB8888)
		{
			printf("Invalid area format!\n");
			return eERROR_INVALID_ARGUMENT;
		}
	}
	else if((gpDU->osd.rle[layer].canvas.layer_mode&0xF) == 0xB)
	{
		if(format != eFORMAT_RGBA4444)
		{
			printf("Invalid area format!\n");
			return eERROR_INVALID_ARGUMENT;
		}
	}
	else if((gpDU->osd.rle[layer].canvas.layer_mode&0xF) == 0x9)
	{
		if(format != eFORMAT_1BIT)
		{
			printf("Invalid area format!\n");
			return eERROR_INVALID_ARGUMENT;
		}
	}

	// check spec (RGB888 or ARGB8888 format must be half the width)
	if( ((gpDU->osd.rle[layer].canvas.layer_mode&0xF) == 0x5) || ((gpDU->osd.rle[layer].canvas.layer_mode&0xF) == 0x7) )
	{
		if( (gpDU->osd.rle[layer].canvas.canvas_size&0xFFFF)/2 < rect.u16Width )
		{
			printf("Not support area size!\n");
			return eERROR_NOT_SUPPORT;
		}
	}

	// check boundary //
	if(		(rect.u16X < 0)
		||	((rect.u16X + rect.u16Width) > ((gpDU->osd.rle[layer].canvas.canvas_size)&0xFFFF))
		||	(rect.u16Y < 0)
		||	((rect.u16Y + rect.u16Height) > ((gpDU->osd.rle[layer].canvas.canvas_size>>16)&0xFFFF))	)
	{
		printf("over boundary area point! layer:%d, area:%d (rect:%d,%d_%dx%d, canvas:%dx%d)\n", layer, area, rect.u16X, rect.u16Y, rect.u16Width, rect.u16Height, ((gpDU->osd.rle[layer].canvas.canvas_size)&0xFFFF), ((gpDU->osd.rle[layer].canvas.canvas_size>>16)&0xFFFF) );
		return eERROR_INVALID_ARGUMENT;
	}

	// register block
	gpDU->osd.ctrl.reg_blocked = 1;

	// set area config//
	if( (gpDU->osd.rle[layer].canvas.layer_mode >> 7) & 0x1 )		// filed en.
	{
		if( (gpDU->osd.rle[layer].canvas.layer_mode >> 5) & 0x1 )	// 2d-dma mode
		{
			// 2d-dma mode
			if(format == eFORMAT_RLE)
				gpDU->osd.rle[layer].area[area].byte = byte;
			else
				gpDU->osd.rle[layer].area[area].stride = stride*2;
		}
		else														// 1d-dma mode
		{
			printf("Not set 2d-dma mode!\n");
			return eERROR_FAILURE;
		}

		if(format == eFORMAT_RLE)
		{
			gpDU->osd.rle[layer].area[area].base_addr2 = (PP_U32)addr;	// even
			gpDU->osd.rle[layer].area[area].base_addr = (PP_U32)addr2;	// odd
		}
		else
		{
			gpDU->osd.rle[layer].area[area].base_addr2 = (PP_U32)addr;							// even
			gpDU->osd.rle[layer].area[area].base_addr = (PP_U32)(addr + (stride/sizeof(addr)));	// odd
		}
	}
	else															// filed dis.
	{
		if( (gpDU->osd.rle[layer].canvas.layer_mode >> 5) & 0x1 )	// 2d-dma mode
		{
			// 2d-dma mode
			if(format == eFORMAT_RLE)
				gpDU->osd.rle[layer].area[area].byte = byte;
			else
				gpDU->osd.rle[layer].area[area].stride = stride;
		}
		else														// 1d-dma mode
		{
			gpDU->osd.rle[layer].area[area].byte = byte;
		}
		gpDU->osd.rle[layer].area[area].base_addr = (PP_U32)addr;
	}

	gpDU->osd.rle[layer].area[area].image_size = (rect.u16Height << 16) | rect.u16Width;

	if(area == eArea0)
	{
		gpDU->osd.rle[layer].canvas.area0_start = (rect.u16Y << 16) | rect.u16X;
	}
	else if(area == eArea1)
	{
		gpDU->osd.rle[layer].canvas.area1_start = (rect.u16Y << 16) | rect.u16X;
	}
	else if(area == eArea2)
	{
		gpDU->osd.rle[layer].canvas.area2_start = (rect.u16Y << 16) | rect.u16X;
	}
	else if(area == eArea3)
	{
		gpDU->osd.rle[layer].canvas.area3_start = (rect.u16Y << 16) | rect.u16X;
	}

	// register block freeze
	gpDU->osd.ctrl.reg_blocked = 0;

	return eSUCCESS;
}
#endif

PP_RESULT_E PPDRV_DU_OSD_SetArea (PP_DU_LAYER_E IN layer, PP_DU_AREA_E IN area, PP_U32* IN addr, PP_U32* IN addr2, PP_U32 IN byte, PP_U32 IN stride, PP_DU_FORMAT_E IN format)
{
	// check du init
	if ( gDU_isInit == PP_FALSE )
	{
		printf("eERROR_DU_NOT_INITIALIZE\n");
		return eERROR_DU_NOT_INITIALIZE;
	}
	
	// register block
	gpDU->osd.ctrl.reg_blocked = 1;

	// set area config//
	if( (gpDU->osd.rle[layer].canvas.layer_mode >> 7) & 0x1 )		// filed en.
	{
		if( (gpDU->osd.rle[layer].canvas.layer_mode >> 5) & 0x1 )	// 2d-dma mode
		{
			// 2d-dma mode
			if(format == eFORMAT_RLE)
				gpDU->osd.rle[layer].area[area].byte = byte;
			else
				gpDU->osd.rle[layer].area[area].stride = stride*2;
		}
		else														// 1d-dma mode
		{
			printf("Not set 2d-dma mode!\n");
			return eERROR_FAILURE;
		}

		if(format == eFORMAT_RLE)
		{
			gpDU->osd.rle[layer].area[area].base_addr2 = (PP_U32)addr;	// even
			gpDU->osd.rle[layer].area[area].base_addr = (PP_U32)addr2;	// odd
		}
		else
		{
			gpDU->osd.rle[layer].area[area].base_addr2 = (PP_U32)addr;							// even
			gpDU->osd.rle[layer].area[area].base_addr = (PP_U32)(addr + (stride/sizeof(addr)));	// odd
		}
	}
	else															// filed dis.
	{
		if( (gpDU->osd.rle[layer].canvas.layer_mode >> 5) & 0x1 )	// 2d-dma mode
		{
			// 2d-dma mode
			if(format == eFORMAT_RLE)
				gpDU->osd.rle[layer].area[area].byte = byte;
			else
				gpDU->osd.rle[layer].area[area].stride = stride;
		}
		else														// 1d-dma mode
		{
			gpDU->osd.rle[layer].area[area].byte = byte;
		}
		gpDU->osd.rle[layer].area[area].base_addr = (PP_U32)addr;
	}

	// register block freeze
	gpDU->osd.ctrl.reg_blocked = 0;

	return eSUCCESS;
}

PP_RESULT_E PPDRV_DU_OSD_SetAreaPosition (PP_DU_LAYER_E IN layer, PP_DU_AREA_E IN area, PP_U16 IN x, PP_U16 IN y, PP_U16 IN w, PP_U16 IN h)
{
	// check du init
	if ( gDU_isInit == PP_FALSE )
	{
		printf("eERROR_DU_NOT_INITIALIZE\n");
		return eERROR_DU_NOT_INITIALIZE;
	}

	// check boundary //
	if(		(x < 0)
		||	((x + w) > ((gpDU->osd.rle[layer].canvas.canvas_size)&0xFFFF))
		||	(y < 0)
		||	((y + h) > ((gpDU->osd.rle[layer].canvas.canvas_size>>16)&0xFFFF))	)
	{
		printf("over boundary area point! layer:%d, area:%d (rect:%d,%d_%dx%d, canvas:%dx%d)\n", layer, area, x, y, w, h, ((gpDU->osd.rle[layer].canvas.canvas_size)&0xFFFF), ((gpDU->osd.rle[layer].canvas.canvas_size>>16)&0xFFFF) );
		return eERROR_INVALID_ARGUMENT;
	}

	// register block
	gpDU->osd.ctrl.reg_blocked = 1;

	gpDU->osd.rle[layer].area[area].image_size = (h << 16) | w;

	if(area == eArea0)
	{
		gpDU->osd.rle[layer].canvas.area0_start = (y << 16) | x;
	}
	else if(area == eArea1)
	{
		gpDU->osd.rle[layer].canvas.area1_start = (y << 16) | x;
	}
	else if(area == eArea2)
	{
		gpDU->osd.rle[layer].canvas.area2_start = (y << 16) | x;
	}
	else if(area == eArea3)
	{
		gpDU->osd.rle[layer].canvas.area3_start = (y << 16) | x;
	}

	// register block freeze
	gpDU->osd.ctrl.reg_blocked = 0;

	return eSUCCESS;
}


/* 5. etc */
PP_U32 PPDRV_DU_GetVersion (PP_VOID)
{
	return gpDU->ctrl.version;
}

PP_U32 PPDRV_DU_OSD_GetFrameCnt (PP_VOID)
{
	return gpDU->osd.ctrl.frame_cnt;
}

PP_VOID PPDRV_DU_OSD_SetR2Y (PP_DU_LAYER_E IN layer, PP_U32 IN bypass, PP_DU_R2Y_COEF_S IN coef)
{
	if(bypass)
	{
		gpDU->osd.rle[layer].rgb2yuv.ycoef0 |= (1<<12);
	}
	else
	{
		gpDU->osd.rle[layer].rgb2yuv.ycoef0 = coef.y0 & 0xFFF;
		gpDU->osd.rle[layer].rgb2yuv.ycoef1 = coef.y1 & 0xFFF;
		gpDU->osd.rle[layer].rgb2yuv.ycoef2 = coef.y2 & 0xFFF;
		gpDU->osd.rle[layer].rgb2yuv.ycoef3 = coef.y3 & 0xFFFFF;

		gpDU->osd.rle[layer].rgb2yuv.ucoef0 = coef.u0 & 0xFFF;
		gpDU->osd.rle[layer].rgb2yuv.ucoef1 = coef.u1 & 0xFFF;
		gpDU->osd.rle[layer].rgb2yuv.ucoef2 = coef.u2 & 0xFFF;
		gpDU->osd.rle[layer].rgb2yuv.ucoef3 = coef.u3 & 0xFFFFF;
		
		gpDU->osd.rle[layer].rgb2yuv.vcoef0 = coef.v0 & 0xFFF;
		gpDU->osd.rle[layer].rgb2yuv.vcoef1 = coef.v1 & 0xFFF;
		gpDU->osd.rle[layer].rgb2yuv.vcoef2 = coef.v2 & 0xFFF;
		gpDU->osd.rle[layer].rgb2yuv.vcoef3 = coef.v3 & 0xFFFFF;

		gpDU->osd.rle[layer].rgb2yuv.ycoef0 &= ~(1<<12);
	}
}

// only DEBUG //
PP_VOID PPDRV_DU_SetTestPattern(PP_BOOL IN enable, PP_U8 IN mode, PP_U8 IN sel)
{
	PP_DU_BTO_CONFIG1_U cfg1;

	cfg1.var = gpDU->bto.ctrl1;

	cfg1.fld.pat_sel				= sel;		// [11:8] pat_sel
	cfg1.fld.pat_md					= mode;		// [15:12] pat_md
	cfg1.fld.pat_en					= enable;	// [22] pat_en
	
	gpDU->bto.ctrl1 = cfg1.var;
}

PP_VOID PPDRV_DU_WaitBlank (PP_VOID)
{
	while( (gpDU->ctrl.state0 & 0x40) == 0 );	// wait active
	while( (gpDU->ctrl.state0 & 0x40) != 0 );	// wait blank
}

PP_VOID PPDRV_DU_SkipVSync (PP_VOID)
{
	if((gpDU->ctrl.state0 & 0x40) == 0)
	{
		while( (gpDU->ctrl.state0 & 0x40) == 0 );	// wait active
	}
	else
	{
		while( (gpDU->ctrl.state0 & 0x40) != 0 );	// wait blank
		while( (gpDU->ctrl.state0 & 0x40) == 0 );	// wait active
	}
}

PP_VOID PPDRV_DU_WaitBlank_BT (PP_VOID)
{
	while( (gpDU->ctrl.state0 & 0x1) != 0 );
}

PP_VOID PPDRV_DU_BTO_SetFreeze(PP_RECT_S IN dispRect)
{
    PP_U32 marginPixel = 16;

	/* make quad split region */
	gpDU->bto_crc.area0.ctrl0 = 0x0000001F;
//	if( ((BD_SVM_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD720H) || ((BD_SVM_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD960H) )
//	{
//		gpDU->bto_crc.area0.ctrl0 |= 0x00000002;
//	}
	gpDU->bto_crc.area0.ctrl1 = 0x00000000;
	gpDU->bto_crc.area0.ctrl2 = ((dispRect.u16Y+marginPixel)<<16) | (dispRect.u16X+marginPixel);
	gpDU->bto_crc.area0.ctrl3 = ((((dispRect.u16Height)/2)-marginPixel)<<16) | (((dispRect.u16Width)/2)-marginPixel);
	//printf("Area0: %dx%d_%dx%d\n", dispRect.u16X+marginPixel, dispRect.u16Y+marginPixel, (dispRect.u16Width/2)-marginPixel, (dispRect.u16Height/2)-marginPixel); 

	gpDU->bto_crc.area1.ctrl0 = 0x0000001F;
//	if( ((BD_SVM_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD720H) || ((BD_SVM_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD960H) )
//	{
//		gpDU->bto_crc.area1.ctrl0 |= 0x00000002;
//	}
	gpDU->bto_crc.area1.ctrl1 = 0x00000000;
	gpDU->bto_crc.area1.ctrl2 = ((dispRect.u16Y+marginPixel)<<16) | ((dispRect.u16Width)/2+marginPixel);
	gpDU->bto_crc.area1.ctrl3 = ((((dispRect.u16Height)/2)-marginPixel)<<16)| (((dispRect.u16Width))-marginPixel);
	//printf("Area1: %dx%d_%dx%d\n", dispRect.u16Width/2+marginPixel, dispRect.u16Y+marginPixel, (dispRect.u16Width)-marginPixel, (dispRect.u16Height/2)-marginPixel); 

	gpDU->bto_crc.area2.ctrl0 = 0x0000001F;
//	if( ((BD_SVM_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD720H) || ((BD_SVM_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD960H) )
//	{
//		gpDU->bto_crc.area2.ctrl0 |= 0x00000002;
//	}
	gpDU->bto_crc.area2.ctrl1 = 0x00000000;
	gpDU->bto_crc.area2.ctrl2 = (((dispRect.u16Height)/2+marginPixel)<<16) | ((dispRect.u16X+marginPixel));
	gpDU->bto_crc.area2.ctrl3 = ((((dispRect.u16Height))-marginPixel)<<16)| (((dispRect.u16Width)/2)-marginPixel);
	//printf("Area2: %dx%d_%dx%d\n", dispRect.u16X+marginPixel, dispRect.u16Height/2+marginPixel, (dispRect.u16Width/2)-marginPixel, (dispRect.u16Height)-marginPixel); 

	gpDU->bto_crc.area3.ctrl0 = 0x0000001F;
//	if( ((BD_SVM_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD720H) || ((BD_SVM_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD960H) )
//	{
//		gpDU->bto_crc.area3.ctrl0 |= 0x00000002;
//	}
	gpDU->bto_crc.area3.ctrl1 = 0x00000000;
	gpDU->bto_crc.area3.ctrl2 = (((dispRect.u16Height)/2+marginPixel)<<16) | ((dispRect.u16Width)/2+marginPixel);
	gpDU->bto_crc.area3.ctrl3 = ((((dispRect.u16Height))-marginPixel)<<16)| (((dispRect.u16Width))-marginPixel);
	//printf("Area3: %dx%d_%dx%d\n", dispRect.u16Width/2+marginPixel, dispRect.u16Height/2+marginPixel, (dispRect.u16Width)-marginPixel, (dispRect.u16Height)-marginPixel); 

}

/***************************************************************************************************************************************/
// API

STATIC PP_VOID du_isr_handler (PP_VOID)
{
	PP_U32 layerIdx, areaIdx;
	
	printf("\n\n=> DU OSD INTERRUPT !!!<=\n\n");

	// disable all area.
	for(layerIdx=eLayer0 ; layerIdx<eLayer_MAX ; layerIdx++)
	{
		for(areaIdx=eArea0 ; areaIdx<eArea_MAX ; areaIdx++)
		{
			PPDRV_DU_OSD_EnableArea(layerIdx, areaIdx, FALSE);
		}
	}
}

