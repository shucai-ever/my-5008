#ifndef __VIN_USER_CONFIG_H__
#define __VIN_USER_CONFIG_H__

//////////////////////////////////////////////////////////////////////////////

typedef union tagVIN_PAR_IN_FMT_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	par0_data_type	:	3,
				par0_bit_lsb	:	1,
				par0_bit_swap	:	1,
				par0_bt1120_md	:	1,
				par0_sync_md	:	1,
				par0_byte_swap	:	1,
		                par1_data_type	:	3,
		                par1_bit_lsb	:	1,
		                par1_bit_swap	:	1,
		                par1_bt1120_md	:	1,
		                par1_sync_md	:	1,
		                par1_byte_swap	:	1,
		                par2_data_type	:	3,
		                par2_bit_lsb	:	1,
		                par2_bit_swap	:	1,
		                par2_bt1120_md	:	1,
		                par2_sync_md	:	1,
		                par2_byte_swap	:	1,
		                par3_data_type	:	3,
		                par3_bit_lsb	:	1,
		                par3_bit_swap	:	1,
		                par3_bt1120_md	:	1,
		                par3_sync_md	:	1,
		                par3_byte_swap	:	1;
	}param;
} VIN_PAR_IN_FMT_CONFIG_U;

typedef union tagVIN_PAR_IN_MUX_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	par0_in_mux	:	2,
				par0_id_md	:	3,
				reserved0	:	3,
				par1_in_mux	:	2,
				par1_id_md	:	3,
				reserved1	:	19;
	}param;
} VIN_PAR_IN_MUX_CONFIG_U;

typedef union tagVIN_MIPI_IN_FMT_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	reserved0	:	2,
				mipi_ch_hsize	:	9,
				reserved1	:	1,
				mipi_vc_size	:	2,
				reserved2	:	2,
				mipi_vc_md	:	2,
				reserved3	:	2,
				mipi_data_type	:	3,
				reserved4	:	1,
				mipi_async_md	:	2,
				reserved5	:	2,
				mipi_clk_sel	:	2,
				reserved6	:	2;
	}param;
} VIN_MIPI_IN_FMT_CONFIG_U;

typedef union tagVIN_VID_IN_SEL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	vid0_in_sel_portsel	:	2,
				vid0_in_sel_pathsel	:	2,
		                vid1_in_sel_portsel	:	2,
				vid1_in_sel_pathsel	:	2,
		                vid2_in_sel_portsel	:	2,
				vid2_in_sel_pathsel	:	2,
		                vid3_in_sel_portsel	:	2,
				vid3_in_sel_pathsel	:	2,
		                vid4_in_sel_portsel	:	2,
				vid4_in_sel_pathsel	:	2,
				par_as_en	:	4,
				mipi_as_en	:	4,
				pvi_as_en	:	4;
	}param;
} VIN_VID_IN_SEL_CONFIG_U;

typedef union tagVIN_ISP_IN_SEL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	isp_tst_pt_sel	:	4,
				isp_tst_mode	:	4,
		                isp_tp_en	:	4,
				isp_in_async_en	:	1,
		                isp_in_async_chg_en	:	1,
				isp_out_async_en	:	1,
		                isp_out_async_chg_en	:	1,
		                reserved0	:	4,
				rec_in_sel	:	4,
		                reserved1	:	4,
		                isp_tp_tst_sim1	:	1,
		                reserved2	:	3;
	}param;
} VIN_ISP_IN_SEL_CONFIG_U;

typedef union tagVIN_SVM_IN_SEL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	svm0_in_sel_portsel	:	2,
		                svm0_in_sel_pathsel	:	2,
		                svm1_in_sel_portsel	:	2,
		                svm1_in_sel_pathsel	:	2,
		                svm2_in_sel_portsel	:	2,
		                svm2_in_sel_pathsel	:	2,
		                svm3_in_sel_portsel	:	2,
		                svm3_in_sel_pathsel	:	2,
				svm_fld_inv	:	4,
		                reserved0	:	4,
		                sel_vsync_md	:	2,
		                reserved1	:	2,
				svm_tp_tst_sim1	:	1,
		                reserved2	:	3;
	}param;
} VIN_SVM_IN_SEL_CONFIG_U;

typedef union tagVIN_QUAD_CH_SEL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	quad0_ch_sel_portsel	:	2,
		                quad0_ch_sel_pathsel	:	2,
		                quad1_ch_sel_portsel	:	2,
		                quad1_ch_sel_pathsel	:	2,
		                quad2_ch_sel_portsel	:	2,
		                quad2_ch_sel_pathsel	:	2,
		                quad3_ch_sel_portsel	:	2,
		                quad3_ch_sel_pathsel	:	2,
				quad_fld_inv	:	4,
		                reserved0	:	12;
	}param;
} VIN_QUAD_CH_SEL_CONFIG_U;

typedef union tagVIN_REC_CH_SEL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	rec0_ch_sel_portsel	:	2,
		                rec0_ch_sel_pathsel	:	2,
		                rec1_ch_sel_portsel	:	2,
		                rec1_ch_sel_pathsel	:	2,
		                rec2_ch_sel_portsel	:	2,
		                rec2_ch_sel_pathsel	:	2,
		                rec3_ch_sel_portsel	:	2,
		                rec3_ch_sel_pathsel	:	2,
		                canny_ch_sel_portsel	:	2,
		                canny_ch_sel_pathsel	:	2,
		                canny_fld_pol	:	1,
		                canny_fld_sel	:	1,
		                reserved0	:	2,
		                vpu_ch_sel_portsel	:	2,
		                vpu_ch_sel_pathsel	:	2,
		                vpu_fld_pol	:	1,
		                vpu_fld_sel	:	1,
		                reserved1	:	2;
	}param;
} VIN_REC_CH_SEL_CONFIG_U;

typedef union tagVIN_SVM_TP_SEL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	svm0_tp_sel	:	4,
		                svm1_tp_sel	:	4,
		                svm2_tp_sel	:	4,
		                svm3_tp_sel	:	4,
		                svm0_tp_mode	:	4,
		                svm1_tp_mode	:	4,
		                svm2_tp_mode	:	4,
		                svm3_tp_mode	:	4;
	}param;
} VIN_SVM_TP_SEL_CONFIG_U;

typedef union tagVIN_VID_IN_CTRL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	vid_rd_vsize	:	11,
		                reserved0	:	1,
		                vid_rd_fld_pol	:	1,
		                vid_rd_raw_md	:	1,
		                vid_sync_os	:	2,
		                vid_rd_hsize	:	11,
		                reserved1	:	1,
		                vid_rd_vblk_md	:	2,
		                vid_rd_int_md	:	1,
		                vid_rd_en	:	1;
	}param;
} VIN_VID_IN_CTRL_CONFIG_U;

typedef union tagVIN_VID_IN_HTOTAL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	vid_rd_htotal	:	14,
		                reserved0	:	2,
		                vid_rd_swap	:	6,
		                reserved1	:	10;
	}param;
} VIN_VID_IN_HTOTAL_CONFIG_U;

typedef union tagVIN_VID_IN_VTOTAL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	vid_rd_vtotal_odd	:	14,
		                reserved0	:	2,
		                vid_rd_vtotal_even	:	14,
		                reserved1	:	2;
	}param;
} VIN_VID_IN_VTOTAL_CONFIG_U;

typedef union tagVIN_VID_IN_FLD_OS_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	vid_rd_fld_os_odd	:	14,
		                reserved0	:	2,
		                vid_rd_fld_os_even	:	14,
		                reserved1	:	2;
	}param;
} VIN_VID_IN_FLD_OS_CONFIG_U;

typedef union tagVIN_REC_IN_CTRL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	resync_rd_vsize		:	11,
				reserved0	:	1,
		                resync_rd_fld_pol	:	1,
		                reserved1		:	3,
		                resync_rd_hsize		:	11,
		                reserved2		:	1,
		                resync_rd_vblk_md	:	2,
		                resync_rd_int_md	:	1,
		                resync_rd_en		:	1;
	}param;
} VIN_REC_IN_CTRL_CONFIG_U;

typedef union tagVIN_REC_IN_HTOTAL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	resync_rd_htotal	:	14,
		                reserved0	:	2,
		                resync_brt_ctrl	:	8,
		                reserved1	:	8;
	}param;
} VIN_REC_IN_HTOTAL_CONFIG_U;

typedef union tagVIN_REC_IN_VTOTAL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	resync_rd_vtotal_odd	:	14,
		                reserved0	:	2,
		                resync_rd_vtotal_even	:	14,
		                reserved1	:	2;
	}param;
} VIN_REC_IN_VTOTAL_CONFIG_U;

typedef union tagVIN_REC_IN_FLD_OS_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	resync_rd_fld_os_odd	:	14,
		                reserved0	:	2,
		                resync_rd_fld_os_even	:	14,
		                reserved1	:	2;
	}param;
} VIN_REC_IN_FLD_OS_CONFIG_U;

typedef union tagVIN_PAR_VSYNC_CTRL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	par_vav_size	:	14,
		                par_vav_end_md	:	1,
		                par_fld_pol	:	1,
		                par_vsv_os	:	14,
		                par_vsv_pol	:	1,
		                par_vsync_md	:	1;
	}param;
} VIN_PAR_VSYNC_CTRL_CONFIG_U;

typedef union tagVIN_PAR_HSYNC_CTRL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	par_hav_size	:	14,
				reserved0	:	2,
		                par_hav_os	:	14,
		                par_hav_pol	:	1,
		                par_hsync_md	:	1;
	}param;
} VIN_PAR_HSYNC_CTRL_CONFIG_U;

typedef union tagVIN_QUAD_CTRL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	quad_vav_con_en		:	1,
				quad_hav_con_en		:	1,
		                quad_tst_mem_sim	:	1,
		                quad_tst_vscl		:	1,
		                quad_tst_lat		:	1,
		                quad_tst_byp		:	1,
		                quad_v_ofst		:	1,
		                quad_intl_md		:	1,
		                quad_vscl_md		:	1,
		                quad_hlpf_md_c		:	1,
		                quad_hlpf_md_y		:	2,
		                reserved0		:	4,
				svm_tp0_chg		: 	4,
				svm_tp1_chg		: 	4,
				svm_tp2_chg		: 	4,
				svm_tp3_chg		: 	4;
	}param;
} VIN_QUAD_CTRL_CONFIG_U;

typedef union tagVIN_QUAD_HVSCL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	quad_hscl	:	16,
				quad_vscl	:	16;
	}param;
} VIN_QUAD_HVSCL_CONFIG_U;

typedef union tagVIN_QUAD_HVDEL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	quad_hdelay	:	16,
				quad_vdelay	:	16;
	}param;
} VIN_QUAD_HVDEL_CONFIG_U;

typedef union tagVIN_QUAD_HVACT_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	quad_hactive	:	16,
				quad_vactive	:	16;
	}param;
} VIN_QUAD_HVACT_CONFIG_U;

typedef union tagVIN_QUAD_WIN_SIZE_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	quad_win_vsize	:	10,
				reserved0	:	9,
				quad_win_hsize	:	7,
				reserved1	:	2,
				quad_wr_page	:	2,
				reserved2	:	2;
	}param;
} VIN_QUAD_WIN_SIZE_CONFIG_U;

typedef union tagVIN_QUAD_WIN_WR_EN_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	quad_ddr_wr_delay	:	8,
				quad_ddr_rd_delay	:	8,
				quad_wr_i2p_en		:	4,
				quad_wr_period		:	4,
				quad_wr_int_md		:	4,
				quad_wr_en		:	4;
	}param;
} VIN_QUAD_WIN_WR_EN_CONFIG_U;

typedef union tagVIN_QUAD_WIN_RD_EN_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	quad_bnd_col	:	2,
				reserved0	:	2,
				quad_bgd_col	:	2,
				reserved1	:	2,
				quad_blk_col		:	2,
				reserved2	:	2,
				quad_win_bnd_size		:	2,
				reserved3	:	2,
				quad_win_bnd_en		:	4,
				quad_win_blk_en		:	4,
				quad_win_en		:	4,
				quad_rd_p2i_fld		:	1,
				quad_rd_p2i_en		:	1,
				quad_rd_int_md		:	1,
				quad_rd_en		:	1;
	}param;
} VIN_QUAD_WIN_RD_EN_CONFIG_U;

typedef union tagVIN_QUAD_WIN_HVOS_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	quad_win_vos	:	11,
				reserved0	:	5,
				quad_win_hos	:	11,
				reserved1	:	5;
	}param;
} VIN_QUAD_WIN_HVOS_CONFIG_U;

typedef union tagVIN_QUAD_RD_HVSIZE_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	quad_rd_vsize	:	11,
				reserved0	:	5,
				quad_rd_hsize	:	11,
				reserved1	:	5;
	}param;
} VIN_QUAD_RD_HVSIZE_CONFIG_U;

typedef union tagVIN_QUAD_RD_HTOTAL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	quad_rd_htotal	:	14,
				reserved0	:	18;
	}param;
} VIN_QUAD_RD_HTOTAL_CONFIG_U;

typedef union tagVIN_QUAD_RD_VTOTAL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	quad_rd_vtotal_odd	:	14,
				reserved0	:	2,
				quad_rd_vtotal_even	:	14,
				reserved1	:	2;
	}param;
} VIN_QUAD_RD_VTOTAL_CONFIG_U;

typedef union tagVIN_QUAD_RD_FLD_OS_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	quad_rd_fld_os_odd	:	14,
				reserved0	:	2,
				quad_rd_fld_os_even	:	14,
				reserved1	:	2;
	}param;
} VIN_QUAD_RD_FLD_OS_CONFIG_U;

typedef union tagVIN_QUAD_OUT_FMT_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	quad_out_clk0_md	:	2,
				quad_out_clk1_md	:	2,
				quad_out_bt656_md	:	1,
				quad_out_fld_inv	:	1,
				quad_out_yc_inv		:	1,
				quad_out_lim_en		:	1,
				quad_out_16bit_md	:	1,
				quad_out_validb		:	1,
				quad_vpu_fld_sel	:	1,
				quad_vpu_fld_pol	:	1,
				quad_out_async_chg_en	:	1,
				quad_out_async_en	:	1,
				quad_rec_async_chg_en	:	1,
				quad_rec_async_en	:	1,
				quad_pvi_out_bt656_md	:	1,
				quad_pvi_out_fld_inv	:	1,
				quad_pvi_out_yc_inv	:	1,
				quad_pvi_out_lim_en	:	1,
				quad_pvi_async_chg_en	:	1,
				quad_pvi_async_en	:	1,
				reserved0		:	10;
	}param;
} VIN_QUAD_OUT_FMT_CONFIG_U;

typedef union tagVIN_QUAD_FULL_MD_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	quad0_wr_page		:	4,
				quad1_wr_page		:	4,
				quad2_wr_page		:	4,
				quad3_wr_page		:	4,
				quad_wr_strobe		:	4,
				quad_wr_busy_det	:	4,
				quad_single_md		:	4,
				reserved0		:	3,
				quad_full_md		:	1;
	}param;
} VIN_QUAD_FULL_MD_CONFIG_U;

typedef union tagVIN_REC_FMT_CTRL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	rec_chid_en	:	1,
				rec_async_chg_en	:	1,
				rec_async_en	:	1,
				bt1120_lim_rec	:	1,
				ch_mux_md	:	2,
				reserved0		:	2,
				outfmt_rate	:	2,
				reserved1		:	2,
				outfmt_yc_inv	:	4,
				outfmt_bt656	:	1,
				outfmt_16bit	:	1,
				reserved2		:	2,
				outfmt_fld_pol	:	4,
				reserved3		:	2,
				sd_pal_ntb	:	1,
				sd_960h_md	:	1,
				sd_p2i_en	:	4;
	}param;
} VIN_REC_FMT_CTRL_CONFIG_U;

typedef union tagVIN_REC0_CH_SEL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	ch_sel_a_0	:	3,
				reserved0		:	1,
				ch_sel_b_0	:	3,
				reserved1		:	1,
				ch_sel_c_0	:	3,
				reserved2		:	1,
				ch_sel_d_0	:	3,
				reserved3		:	1,
				chid_num_0	:	2,
				chid_num_1	:	2,
				chid_num_2	:	2,
				chid_num_3	:	2,
				rec_clk_phase	:	6,
				reserved4		:	2;
	}param;
} VIN_REC0_CH_SEL_CONFIG_U;

typedef union tagVIN_REC1_CH_SEL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	ch_sel_a_0	:	3,
				reserved0		:	1,
				ch_sel_b_0	:	3,
				reserved1		:	1,
				ch_sel_c_0	:	3,
				reserved2		:	1,
				ch_sel_d_0	:	3,
				reserved3		:	1,
				chid_num_0	:	2,
				chid_num_1	:	2,
				chid_num_2	:	2,
				chid_num_3	:	2,
				rec_clk_phase	:	6,
				reserved4		:	2;
	}param;
} VIN_REC1_CH_SEL_CONFIG_U;

typedef union tagVIN_PAR_PB_FMT_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	par_pb_fmt	:	11,
				reserved0		:	20,
				par_pb_input_mode	:	1;
	}param;
} VIN_PAR_PB_FMT_CONFIG_U;

typedef union tagVIN_PAR_PB_YCOEF_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	par_pb_ycoef	:	20,
				reserved0		:	12;
	}param;
} VIN_PAR_PB_YCOEF_CONFIG_U;

typedef union tagVIN_PAR_PB_UCOEF_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	par_pb_ucoef	:	20,
				reserved0		:	12;
	}param;
} VIN_PAR_PB_UCOEF_CONFIG_U;

typedef union tagVIN_PAR_PB_VCOEF_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	par_pb_vcoef	:	20,
				reserved0		:	12;
	}param;
} VIN_PAR_PB_VCOEF_CONFIG_U;

typedef union tagVIN_GENLOCK_LOW_WIDTH_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	genlock_low_width	:	28,
				genlock_save_sel	:	1,
				genlock_pulse_pol	:	1,
				genlock_pulse_en	:	1,
				genlock_en	:	1;
	}param;
} VIN_GENLOCK_LOW_WIDTH_CONFIG_U;

typedef union tagVIN_GENLOCK_HIGH_WIDTH_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	genlock_high_width	:	28,
				genlock_save_sel	:	3,
				genlock_save_pol	:	1;
	}param;
} VIN_GENLOCK_HIGH_WIDTH_CONFIG_U;

typedef union tagVIN_GENLOCK_START_OS_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	genlock_start_os	:	24,
				genlock_ref_sel	:	3,
				reserved0		:	1,
				genlock_chk_en	:	4;
	}param;
} VIN_GENLOCK_START_OS_CONFIG_U;

typedef union tagVIN_GENLOCK_END_OS_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	genlock_end_os	:	24,
				reserved0	:	4,
				genlock_fld_sel	:	2,
				genlock_ref_pol	:	1,
				genlock_chk_pol	:	1;
	}param;
} VIN_GENLOCK_END_OS_CONFIG_U;

typedef union tagVIN_VID_CRC_CTRL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	vid_crc_ref_data	:	16,
				reserved0	:	8,
				vid_crc_fr_period	:	3,
				vid_crc_fld_sel	:	1,
				vid_crc_int_md	:	1,
				vid_crc_roi_md	:	1,
				vid_crc_roi_en	:	1,
				vid_crc_bist_en	:	1;
	}param;
} VIN_VID_CRC_CTRL_CONFIG_U;

typedef union tagVIN_VID_CRC_STRT_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	vid_crc_hstrt	:	11,
				reserved0	:	5,
				vid_crc_vstrt	:	11,
				reserved1	:	5;
	}param;
} VIN_VID_CRC_STRT_CONFIG_U;

typedef union tagVIN_VID_CRC_END_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	vid_crc_hend	:	11,
				reserved0	:	5,
				vid_crc_vend	:	11,
				reserved1	:	5;
	}param;
} VIN_VID_CRC_END_CONFIG_U;

typedef union tagVIN_QUAD_DDR_CRC_CTRL_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	quad_crc_ddrro_mode	:	4,
				quad_cc_ddrro_en	:	1,
				reserved0	:	3,
				quad_cc_ddrwo_en	:	1,
				reserved1	:	23;
	}param;
} VIN_QUAD_DDR_CRC_CTRL_CONFIG_U;

typedef union tagVIN_SYNC_VIN_HSIZE_INFO_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	hsize	:	14,
				reserved1	:	10,
				sync_lock_hsize	:	1,
				sync_lock_fsize	:	1,
				sync_lock_hactive	:	1,
				sync_lock_vactive	:	1,
				video_loss_det	:	1,
				format_det	:	1,
				reserved0	:	2;
	}param;
} VIN_SYNC_VIN_HSIZE_INFO_CONFIG_U;

typedef union tagVIN_SYNC_VIN_FSIZE_INFO_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	fsize	:	24,
				sync_lock_hsize	:	1,
				sync_lock_fsize	:	1,
				sync_lock_hactive	:	1,
				sync_lock_vactive	:	1,
				video_loss_det	:	1,
				format_det	:	1,
				reserved0	:	2;
	}param;
} VIN_SYNC_VIN_FSIZE_INFO_CONFIG_U;

typedef union tagVIN_SYNC_VIN_HVACT_INFO_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	hact	:	12,
				reserved1	:	4,
				vact	:	11,
				reserved0	:	5;
	}param;
} VIN_SYNC_VIN_HVACT_INFO_CONFIG_U;

typedef union tagVIN_SYNC_VIN_HSIZE_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	sync_hsize	:	14,
				reserved0	:	6,
				fomat_det_en	:	4,
				sync_det_th	:	1,
				sync_int_md	:	1,
				reserved1	:	5,
				vloss_det_en	:	1;
	}param;
} VIN_SYNC_VIN_HSIZE_CONFIG_U;

typedef union tagVIN_SYNC_VIN_FSIZE_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	sync_fsize	:	24,
				sync_hvact_md	:	1,
				reserved0	:	7;
	}param;
} VIN_SYNC_VIN_FSIZE_CONFIG_U;

typedef union tagVIN_SYNC_VIN_HVACT_CONFIG_U
{
	unsigned int		var;
	struct
	{
		unsigned int	sync_hact	:	12,
				reserved0	:	4,
				sync_vact	:	11,
				reserved1	:	5;
	}param;
} VIN_SYNC_VIN_HVACT_CONFIG_U;

//////////////////////////////////////////////////////////////////////////////////////
typedef struct {
	VIN_PAR_IN_FMT_CONFIG_U PI_BD_CONFIG_PAR_IN_FMT_port;   // 0xF0F0_0000
	VIN_PAR_IN_MUX_CONFIG_U PI_BD_CONFIG_PAR_IN_MUX_val;    // 0xF0F0_0004
	VIN_MIPI_IN_FMT_CONFIG_U PI_BD_CONFIG_MIPI_IN_FMT_val;  // 0xF0F0_0008
	VIN_VID_IN_SEL_CONFIG_U PI_BD_CONFIG_VID_IN_SEL_val; // 0xF0F0_000C
	VIN_ISP_IN_SEL_CONFIG_U PI_BD_CONFIG_ISP_IN_SEL_val; // 0xF0F0_0010
	VIN_SVM_IN_SEL_CONFIG_U PI_BD_CONFIG_SVM_IN_SEL_val; // 0xF0F0_0014
	VIN_QUAD_CH_SEL_CONFIG_U PI_BD_CONFIG_QUAD_CH_SEL_val; // 0xF0F0_0018
	VIN_REC_CH_SEL_CONFIG_U PI_BD_CONFIG_REC_CH_SEL_val; // 0xF0F0_001C
	VIN_SVM_TP_SEL_CONFIG_U PI_BD_CONFIG_SVM_TP_SEL_val; // 0xF0F0_0038
	VIN_VID_IN_CTRL_CONFIG_U PI_BD_CONFIG_VID_IN_CTRL_val[4]; // 0xF0F0_0040, 0xF0F0_0050, 0xF0F0_0060, 0xF0F0_0070
	VIN_VID_IN_HTOTAL_CONFIG_U PI_BD_CONFIG_VID_IN_HTOTAL_val[4];// 0xF0F0_0044, 0xF0F0_0054, 0xF0F0_0064, 0xF0F0_0074
	VIN_VID_IN_VTOTAL_CONFIG_U PI_BD_CONFIG_VID_IN_VTOTAL_val[4];// 0xF0F0_0048, 0xF0F0_0058, 0xF0F0_0068, 0xF0F0_0078
	VIN_VID_IN_FLD_OS_CONFIG_U PI_BD_CONFIG_VID_IN_FLD_OS_val[4];// 0xF0F0_004C, 0xF0F0_005C, 0xF0F0_006C, 0xF0F0_007C
	VIN_REC_IN_CTRL_CONFIG_U PI_BD_CONFIG_REC_IN_CTRL_val[4];       // 0xF0F0_0080, 0xF0F0_0090, 0xF0F0_00A0, 0xF0F0_00B0
	VIN_REC_IN_HTOTAL_CONFIG_U PI_BD_CONFIG_REC_IN_HTOTAL_val[4];   // 0xF0F0_0084, 0xF0F0_0094, 0xF0F0_00A4, 0xF0F0_00B4
	VIN_REC_IN_VTOTAL_CONFIG_U PI_BD_CONFIG_REC_IN_VTOTAL_val[4];   // 0xF0F0_0088, 0xF0F0_0098, 0xF0F0_00A8, 0xF0F0_00B8
	VIN_REC_IN_FLD_OS_CONFIG_U PI_BD_CONFIG_REC_IN_FLD_OS_val[4];   // 0xF0F0_008C, 0xF0F0_009C, 0xF0F0_00AC, 0xF0F0_00BC
	VIN_PAR_VSYNC_CTRL_CONFIG_U PI_BD_CONFIG_PAR_VSYNC_CTRL_val[4];// 0xF0F0_00C0, 0xF0F0_00C8, 0xF0F0_00D0, 0xF0F0_00D8
	VIN_PAR_HSYNC_CTRL_CONFIG_U PI_BD_CONFIG_PAR_HSYNC_CTRL_val[4];// 0xF0F0_00C4, 0xF0F0_00CC, 0xF0F0_00D4, 0xF0F0_00DC
	VIN_QUAD_CTRL_CONFIG_U PI_BD_CONFIG_QUAD_CTRL_val[4];   // 0xF0F0_0100, 0xF0F0_0110, 0xF0F0_0120, 0xF0F0_0130
	VIN_QUAD_HVSCL_CONFIG_U PI_BD_CONFIG_QUAD_HVSCL_val[4]; // 0xF0F0_0104, 0xF0F0_0114, 0xF0F0_0124, 0xF0F0_0134
	VIN_QUAD_HVDEL_CONFIG_U PI_BD_CONFIG_QUAD_HVDEL_val[4]; // 0xF0F0_0108, 0xF0F0_0118, 0xF0F0_0128, 0xF0F0_0138
	VIN_QUAD_HVACT_CONFIG_U PI_BD_CONFIG_QUAD_HVACT_val[4]; // 0xF0F0_010C, 0xF0F0_011C, 0xF0F0_012C, 0xF0F0_013C
	VIN_QUAD_WIN_SIZE_CONFIG_U PI_BD_CONFIG_QUAD_WIN_SIZE_val; // 0xF0F0_0150
	VIN_QUAD_WIN_WR_EN_CONFIG_U PI_BD_CONFIG_QUAD_WIN_WR_EN_val; // 0xF0F0_0154
	VIN_QUAD_WIN_RD_EN_CONFIG_U PI_BD_CONFIG_QUAD_WIN_RD_EN_val; // 0xF0F0_0158
	VIN_QUAD_WIN_HVOS_CONFIG_U PI_BD_CONFIG_QUAD_WIN_HVOS_val; // 0xF0F0_015C
	VIN_QUAD_RD_HVSIZE_CONFIG_U PI_BD_CONFIG_QUAD_RD_HVSIZE_val; // 0xF0F0_0160
	VIN_QUAD_RD_HTOTAL_CONFIG_U PI_BD_CONFIG_QUAD_RD_HTOTAL_val; // 0xF0F0_0164
	VIN_QUAD_RD_VTOTAL_CONFIG_U PI_BD_CONFIG_QUAD_RD_VTOTAL_val; // 0xF0F0_0168
	VIN_QUAD_RD_FLD_OS_CONFIG_U PI_BD_CONFIG_QUAD_RD_FLD_OS_val; // 0xF0F0_016C
	VIN_QUAD_OUT_FMT_CONFIG_U PI_BD_CONFIG_QUAD_OUT_FMT_val; // 0xF0F0_0170
	VIN_QUAD_FULL_MD_CONFIG_U PI_BD_CONFIG_QUAD_FULL_MD_val; // 0xF0F0_0174
	VIN_REC_FMT_CTRL_CONFIG_U PI_BD_CONFIG_REC_FMT_CTRL_val; // 0xF0F0_0180
	VIN_REC0_CH_SEL_CONFIG_U PI_BD_CONFIG_REC0_CH_SEL_val; // 0xF0F0_0184
	VIN_REC1_CH_SEL_CONFIG_U PI_BD_CONFIG_REC1_CH_SEL_val; // 0xF0F0_0188
	VIN_PAR_PB_FMT_CONFIG_U PI_BD_CONFIG_PAR_PB_FMT_val; // 0xF0F0_01C0
	VIN_PAR_PB_YCOEF_CONFIG_U PI_BD_CONFIG_PAR_PB_YCOEF_val[4]; // 0xF0F0_01D0, 0xF0F0_01D4, 0xF0F0_01D8, 0xF0F0_01DC
	VIN_PAR_PB_UCOEF_CONFIG_U PI_BD_CONFIG_PAR_PB_UCOEF_val[4]; // 0xF0F0_01E0, 0xF0F0_01E4, 0xF0F0_01E8, 0xF0F0_01EC
	VIN_PAR_PB_VCOEF_CONFIG_U PI_BD_CONFIG_PAR_PB_VCOEF_val[4]; // 0xF0F0_01F0, 0xF0F0_01F4, 0xF0F0_01F8, 0xF0F0_01FC
	VIN_GENLOCK_LOW_WIDTH_CONFIG_U PI_BD_CONFIG_GENLOCK_LOW_WIDTH_val; // 0xF0F0_0200
	VIN_GENLOCK_HIGH_WIDTH_CONFIG_U PI_BD_CONFIG_GENLOCK_HIGH_WIDTH_val; // 0xF0F0_0204
	VIN_GENLOCK_START_OS_CONFIG_U PI_BD_CONFIG_GENLOCK_START_OS_val; // 0xF0F0_0220
	VIN_GENLOCK_END_OS_CONFIG_U PI_BD_CONFIG_GENLOCK_END_OS_val; // 0xF0F0_0224
	VIN_VID_IN_CTRL_CONFIG_U PI_BD_CONFIG_PB_IN_CTRL_val; // 0xF0F0_0240
	VIN_VID_IN_HTOTAL_CONFIG_U PI_BD_CONFIG_PB_IN_HTOTAL_val; // 0xF0F0_0244
	VIN_VID_IN_VTOTAL_CONFIG_U PI_BD_CONFIG_PB_IN_VTOTAL_val; // 0xF0F0_0248
	VIN_VID_IN_FLD_OS_CONFIG_U PI_BD_CONFIG_PB_IN_FLD_OS_val; // 0xF0F0_024C
	VIN_REC_IN_CTRL_CONFIG_U PI_BD_CONFIG_PBREC_IN_CTRL_val; // 0xF0F0_0250
	VIN_REC_IN_HTOTAL_CONFIG_U PI_BD_CONFIG_PBREC_IN_HTOTAL_val; // 0xF0F0_0254
	VIN_REC_IN_VTOTAL_CONFIG_U PI_BD_CONFIG_PBREC_IN_VTOTAL_val; // 0xF0F0_0258
	VIN_REC_IN_FLD_OS_CONFIG_U PI_BD_CONFIG_PBREC_IN_FLD_OS_val; // 0xF0F0_025C
	VIN_VID_CRC_CTRL_CONFIG_U PI_BD_CONFIG_VID_CRC_CTRL_val[6];// 0xF0F0_0300, 0xF0F0_0310, 0xF0F0_0320, 0xF0F0_0330, 0xF0F0_0340, 0xF0F0_0350
	VIN_VID_CRC_STRT_CONFIG_U PI_BD_CONFIG_VID_CRC_STRT_val[6];// 0xF0F0_0304, 0xF0F0_0314, 0xF0F0_0324, 0xF0F0_0334, 0xF0F0_0344, 0xF0F0_0354
	VIN_VID_CRC_END_CONFIG_U PI_BD_CONFIG_VID_CRC_END_val[6];// 0xF0F0_0308, 0xF0F0_0318, 0xF0F0_0328, 0xF0F0_0338, 0xF0F0_0348, 0xF0F0_0358
	VIN_QUAD_DDR_CRC_CTRL_CONFIG_U PI_BD_CONFIG_QUAD_DDR_CRC_CTRL_val; // 0xF0F0_03C0
	VIN_SYNC_VIN_HSIZE_INFO_CONFIG_U PI_BD_CONFIG_SYNC_VIN_INFO_HSIZE_val[5];// 0xF0F0_0400, 0xF0F0_040C, 0xF0F0_0418, 0xF0F0_0424, 0xF0F0_0430
	VIN_SYNC_VIN_FSIZE_INFO_CONFIG_U PI_BD_CONFIG_SYNC_VIN_INFO_FSIZE_val[5];// 0xF0F0_0404, 0xF0F0_0410, 0xF0F0_041C, 0xF0F0_0428, 0xF0F0_0434
	VIN_SYNC_VIN_HVACT_INFO_CONFIG_U PI_BD_CONFIG_SYNC_VIN_INFO_HVACT_val[5];// 0xF0F0_0408, 0xF0F0_0414, 0xF0F0_0420, 0xF0F0_042C, 0xF0F0_0438
	VIN_SYNC_VIN_HSIZE_CONFIG_U PI_BD_CONFIG_SYNC_VIN_HSIZE_val[5];// 0xF0F0_0440, 0xF0F0_044C, 0xF0F0_0458, 0xF0F0_0464, 0xF0F0_0470
	VIN_SYNC_VIN_FSIZE_CONFIG_U PI_BD_CONFIG_SYNC_VIN_FSIZE_val[5];// 0xF0F0_0444, 0xF0F0_0450, 0xF0F0_045C, 0xF0F0_0468, 0xF0F0_0474
	VIN_SYNC_VIN_HVACT_CONFIG_U PI_BD_CONFIG_SYNC_VIN_HVACT_val[5];// 0xF0F0_0448, 0xF0F0_0454, 0xF0F0_0460, 0xF0F0_046C, 0xF0F0_0478

}_VIN_USER_CONFIG_REG;

extern const _VIN_DATATYPE PI_BD_CONFIG_VinRawTypeMIPI;
extern const _VIN_DATATYPE PI_BD_CONFIG_VinRawType[4];
extern const int PI_BD_CONFIG_VinVideoType[4];
extern const int DEF_VIN_MIPI_HSIZE[max_vid_resol][max_vin_datatype];
extern const int DEF_VIN_VACTIVE_SIZE[2][max_vid_resol];
extern const int DEF_VIN_HACTIVE_SIZE[2][max_vid_resol];
extern const int DEF_VIN_HTOTAL_SIZE[2][max_vid_resol];
extern const int DEF_VIN_ODD_EVEN_FIELD_VTOTAL_SIZE[2][max_vid_resol][2];
extern const int DEF_VIN_ODD_EVEN_FIELD_F_TRANSITION_LINE[2][max_vid_resol][2];
extern const int DEF_VIN_MAXLINE_CNT[2][max_vid_resol];
extern const int DEF_VIN_RECOUT_DATARATE[max_vid_resol];
extern const int DEF_VIN_QUAD_HV_WIN_SIZE[max_vid_resol][2];
extern const char *DEF_STR_VIN_RESOL[max_vid_resol];
extern const char *DEF_STR_VIN_DATATYPE[max_vin_datatype];

void PI_VIN_GetUserConfigVar(_VIN_PATH *pVin, _VIN_USER_CONFIG_REG *pVinUserReg);

#endif // __VIN_USER_CONFIG_H__
