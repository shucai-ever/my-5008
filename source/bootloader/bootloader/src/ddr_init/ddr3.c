#include <stdio.h>
#include "system.h"

void fail()
{
    //dbg("fail!! ddr3 verify\n");
    //while(1);

}

void ddr3_set(void)
{
    // do PLL change (DDR frequence) 1026MHz or 810MHz


    // SCU configuration
    // reset control register (0xF000_0048)
    // [13] : quad 2x sw reset
    // [12] : quad sw reset
    // [11] : vdck1 sw reset
    // [10] : vdck0 sw reset
    // [9] : dmclk sw reset
    // [8] : tck sw rest
    // [7] : ddr postclk 2x sw reset
    // [6] : ddr phy pll reset
    // [5] : ddr phy reset
    // [4] : ddr postclk sw reset
    // [3] : ddr ctlclk sw reset
    // [2] : mipi cci sw reset
    // [1] : mipi pixel sw reset
    // [0] : mipi lane sw reset


    // MISC configuration
    // set ddr register (0xF010_0088)
    // [25:24] leaktest (data phy power down)
    // [23:21] reserved
    // [20] manual refresh
    // [19] DSRONB,	// New Added FXDDR412
    // [18] IO15V,	// New Added FXDDR412
    // [17:15] VREF_SELECT
    // [14] phy_pll_pdn
    // [13] reserved
    // [12] phy_dll_pdn
    // [11] reserved
    // [10:8] DLLFRANGE set
    //      0 : reserved
    //      1 : 400 ~ 600 DDR3/DDR3L/LPDDR2/DDR2/MDDR
    //      2 : 600 ~ 700 DDR3/DDR3L/LPDDR2/DDR2
    //      3 : 700 ~ 800 DDR3/DDR3L/LPDDR2/DDR2
    //      4 : 800 ~ 900 DDR3/DDR3L/LPDDR2/DDR2
    //      5 : 900 ~ 1000 DDR3/DDR3L/LPDDR2/DDR2
    //      6 : 1000 ~ 1200 DDR3/DDR3L/LPDDR2/DDR2
    //      7 : 1200 ~ 1600 DDR3/DDR3L
    // [7] : byone
    // [6] : ddr3 mode
    // [5] : lpddr1 mode
    // [4] : lpddr2 mode
    // [3] : reserved
    // [2] : sio  
    // [1] : phy_con_update
    // [0] : self bias


    int DLLFRANGE;
    int DDR_MODE;

#if defined(DDR_SPEED_500M)
    DLLFRANGE = 0x500;
#elif defined(DDR_SPEED_400M)
    DLLFRANGE = 0x300;
#else
#error "unkonwn ddr3 speed dllframe"
#endif

    DDR_MODE = DDR3_MODE; // DDR3


    // ddr postclk, postclk 2x reset release
    *(volatile unsigned int *) 0xF0000048 = 0x00003f9f; // sw rst (ddr_postclk, postclk_2x)

    // ddr mode set (ddr3)
    // DSRONB, DLL FRANGE setting (1000~1200)
    // phy pll pdn release
    *(volatile unsigned int *) 0xf0100088 = 0x00000000 | DDR_MODE; // DDR mode set
    *(volatile unsigned int *) 0xf0100088 = 0x00080000 | DDR_MODE | DLLFRANGE; // DSRONB, DLLFRANGE SET
    *(volatile unsigned int *) 0xf0100088 = 0x00084000 | DDR_MODE | DLLFRANGE; // phy_pll_pdn enable

    // ddr reset low. las, cas, wen low, clock enable ######################################
    LOOPDELAY_10USEC(10); // wait 100us

    // ddr phy pll reset release
    *(volatile unsigned int *) 0xF0000048 = 0x00003fDf; // sw rst (ddr_postclk, phy_pll, postclk_2x)
    // wait 10 * MCLK
    LOOPDELAY_10USEC(1); // wait 10us

    // ddr phy dll pdn release
    *(volatile unsigned int *) 0xf0100088 = 0x00087000 | DDR_MODE | DLLFRANGE; // phy_dll_pdn enable

    LOOPDELAY_10USEC(10); // wait 100us

    // ddr phy reset release
    *(volatile unsigned int *) 0xF0000048 = 0x00003fff; // sw rst (ddr_postclk, phy_pll, phy, postclk_2x)
    // las, cas, wen high ######################################

    // wait 1024 * MCLK

    volatile unsigned int reg00_value;
    volatile unsigned int reg04_value;
    volatile unsigned int reg08_value;
    volatile unsigned int reg0C_value;
    volatile unsigned int reg10_value;
    volatile unsigned int reg14_value;
    volatile unsigned int reg18_value;
    volatile unsigned int reg1C_value;
    volatile unsigned int reg20_value;
    volatile unsigned int reg24_value;
//    volatile unsigned int reg28_value;
//    volatile unsigned int reg2C_value;
    volatile unsigned int reg30_value;
    volatile unsigned int reg34_value;
    volatile unsigned int reg38_value;
    volatile unsigned int reg3C_value;
    volatile unsigned int reg40_value;
    volatile unsigned int reg48_value;
    volatile unsigned int reg4C_value;
    volatile unsigned int reg60_value;
    volatile unsigned int reg64_value;
    volatile unsigned int reg68_value;
    volatile unsigned int reg6C_value;
    volatile unsigned int reg70_value;
    volatile unsigned int reg74_value;
    volatile unsigned int reg78_value;
    volatile unsigned int reg7C_value;
    volatile unsigned int regA0_value;
    volatile unsigned int regA4_value;
    volatile unsigned int regA8_value;
    volatile unsigned int regAC_value;
    volatile unsigned int regB0_value;
    volatile unsigned int regB4_value;
    volatile unsigned int regB8_value;

    int i;
    unsigned int data;

    //--------------------------------------------------------------------------
    //                  Register Configuration  
    //--------------------------------------------------------------------------

    //--------------------
    // --- Offset 0x00 ---
    //-------------------- 
    //bit[28]    MANUAL_REFRESH_EN
    //bit[26:21] WAIT_CYCLE
    //bit[20] DIS_REORDER
    //bit[19] LPDDR_mode
    //bit[18] DisAPBErrResp
    //bit[17] BYONE
    //bit[16] DDR2_mode 
    //bit[15:13] Post refresh command threshold
    //bit[12:10] Auto-Refresh counts in initial sequence
    //bit[9:8]   Memory width
    //bit[5:4]   AMTSEL
    //bit[2:0]   GDS 

    const unsigned int MANUAL_REFRESH_EN = 0x0;
    const unsigned int WAIT_CYCLE        = 0x3; 
    const unsigned int DIS_REORDER       = 0x0;
    const unsigned int DisAPBerrResp     = 0x1;     

    const unsigned int LP_mode           = 0x0;
    const unsigned int BYONE             = 0x0;    //1:2
    const unsigned int DDR_mode          = 0x0;    //DDR3  

    const unsigned int POST_REF_CNT      = 0x4;
    const unsigned int AREFRESH_CNT      = 0x7;
    // const unsigned int MEM_WIDTH       = 0x0; //8bit
#ifdef DDR_BUS_32BIT
    const unsigned int MEM_WIDTH       = 0x2; //32bit  
#else
    const unsigned int MEM_WIDTH       = 0x1; //16bit  
#endif
    // const unsigned int MEM_WIDTH       = 0x3; //64bit    
    const unsigned int AMTSEL            = 0x1;  
#if defined(DDR_SPEED_500M)
    const unsigned int GDS               = 0x3;		// 0 ~ 7
#elif defined(DDR_SPEED_400M)
    const unsigned int GDS               = 0x2;		// 0 ~ 7
#else
#error "unkonwn ddr3 speed dllframe"
#endif


    reg00_value = 
        (MANUAL_REFRESH_EN << 28)
        | (DisAPBerrResp     << 27)
        | (WAIT_CYCLE        << 21)
        | (DIS_REORDER       << 20)
        | (LP_mode           << 19)
        | (BYONE             << 18)
        | (DDR_mode          << 16)
        | (POST_REF_CNT      << 13)
        | (AREFRESH_CNT      << 10)
        | (MEM_WIDTH         << 8)
        | (AMTSEL            << 4)
        | (GDS);

    //--------------------
    // --- Offset 0x04 ---
    //-------------------- 
    //bit[31] = Software reset
    //bit[12] = ZQCL state
    //bit[11] = ZQCS state
    //bit[10] = Self Refresh State
    //bit[9] = Initial State
    //bit[8] = Initial OK
    //bit[7] = ZQCL
    //bit[6] = ZQCS    
    //bit[5:4] = MRS_mode 
    //bit[3] = Exit Self Refresh
    //bit[2] = Self Refresh
    //bit[1] = Mode Register Set 
    //bit[0] = Initial SDRAM  

    const unsigned int SW_RST	        = 0x0;
    const unsigned int ZQCL		    = 0x0;
    const unsigned int ZQCS		    = 0x0;
    const unsigned int MRS_mode        = 0x0;
    const unsigned int MemCmd_exit_srf = 0x0; 
    const unsigned int MemCmd_srf      = 0x0; 
    const unsigned int MemCmd_mrs      = 0x0;
    volatile unsigned int MemCmd_initial  = 0x0;


    reg04_value =
        (SW_RST	         << 31)
        | (ZQCL	           << 7)
        | (ZQCS	           << 6)
        | (MRS_mode        << 4)
        | (MemCmd_exit_srf << 3)
        | (MemCmd_srf      << 2)
        | (MemCmd_mrs      << 1)
        | (MemCmd_initial); 


    //--------------------
    // --- Offset 0x08 ---
    //-------------------- 
    //MR1[12] Output Buffer 1'0 = Enabled, 1'b1 = Disabled                 
    //MR1[11] 1'b0 = Disabled, 1'b1 = Enabled          
    //MR1[7]  1'b0 = write leveling disabled, 1'b1 = write leveling enabled
    //MR1[9]
    //MR1[6] 
    //MR1[2]  RTT[9,6,2] = 3'b010 = 120 Ohm
    //MR1[5]
    //MR1[1]  DIC[5:1] = 00 = RZQ/6
    //MR1[4:3] AL      
    //MR1[0]  DLL 

    //MR0[12] DLL control for precharge power down, 1'b0 = Slow Exit (DLL off) 1'b1 = Fast Exit (DLL on) 
    //MR0[11:9] Write Recovery (Write with auto precharge)
    //3'b001 = 5, 3'b010 = 6, 3'b011 = 7, 3'b100 = 8, 3'b101 = 10, 3'b110 = 12, 3'b111 = 14, 3'b000 = 16   
    //MR0[8] DLL Reset  1'b0 = No, 1'b1 = Yes
    //MR0[7] MODE 
    //MR0[6:4] CAS latency 
    //MR0[2] CAS_Latency, 4'b0010 = 5, 4'b0010 = 6, 4'b0110 = 7, 4'b1000 = 8, 4'b1010 =9, 
    //                    4'b1100 = 10, 4'b1110 = 11, 4'b0001 = 12, 4'b0011 = 13, 4'b0101 = 14  
    //MR0[3] Burst_Type, 1'b0 = Sequential, 1'b1 = Interleaved  
    //MR0[1:0] Burst_Length, 2'b00 = BL8, 2'b01= BC4 or 8 (on the fly), 2'b10 = BC4

    const unsigned int QOff       = 0x0;    //MR1[12]   bit[28]
    const unsigned int TDQS       = 0x0;    //MR1[11]   bit[27]
    // ihkong - 0914
#ifdef DDR_AUTO_LEVELING
    const unsigned int WL         = 0x1;    //MR1[7]    bit[23]  Write Levelization
#else
    const unsigned int WL         = 0x0;    //MR1[7]    bit[23]  Write Levelization
#endif
    const unsigned int RTT_9      = 0x0;     //MR1[9]}  Rtt bit[25]  
    const unsigned int RTT_6      = 0x0;     //MR1[6]}  Rtt bit[22]  
    const unsigned int RTT_2      = 0x1;     //MR1[2]}  Rtt bit[18]
    // 3'b001 = 60 Ohm, 3'b010 = 120 Ohm, 2'b011 = 40 Ohm  
    const unsigned int DIC_5      = 0x1;     //MR1[5]
    const unsigned int DIC_1      = 0x0;     //MR1[1]  

    const unsigned int AL_DDR3    = 0x0;     //MR1[4:3]  bit[20:19] 2'b01=CL-1, 2'b10 = CL-2
    const unsigned int DLL        = 0x0;     //MR1[0]  

    const unsigned int DLL_PPDN   = 0x0;     //MR0[12] DLL control for precharge power down, 1'b0 = Slow Exit (DLL off) 1'b1 = Fast Exit (DLL on) 
    const unsigned int WR         = 0x5;     //MR0[11:9] Write Recovery (Write with auto precharge)
    //3'b001 = 5, 3'b010 = 6, 3'b011 = 7, 3'b100 = 8, 3'b101 = 10, 3'b110 = 12, 3'b111 = 14, 3'b000 = 16   
    const unsigned int DLL_RESET  = 0x0;     //MR0[8] DLL Reset  1'b0 = No, 1'b1 = Yes
    const unsigned int MODE       = 0x0;     //MR0[7] MODE 

    // 400M: 7, 513M: 8 ~ 9 ?
    // 400M(2.5ns) ~ 533M(1.875ns): CL=7, CWL6
    const unsigned int CAS_Latency_654 = 0x3;   //MR0[6:4] 
    const unsigned int CAS_Latency_2   = 0x0;   //MR0[2] CAS_Latency, 4'b0010 = 5, 4'b0010 = 6, 4'b0110 = 7, 4'b1000 = 8, 4'b1010 =9, 
    //                    4'b1100 = 10, 4'b1110 = 11, 4'b0001 = 12, 4'b0011 = 13, 4'b0101 = 14

    const unsigned int Burst_Type = 0x0;   //MR0[3] Burst_Type, 1'b0 = Sequential, 1'b1 = Interleaved  
    const unsigned int BL_DDR3    = 0x0;    //MR0[1:0] Burst_Length, 2'b00 = BL8, 2'b01= BC4 or 8 (on the fly), 2'b10 = BC4

    reg08_value =
    (QOff         << 28)
    | (TDQS         << 27)
    | (RTT_9        << 25)
    | (WL           << 23)
    | (RTT_6        << 22)
    | (DIC_5        << 21)
    | (AL_DDR3      << 19)
    | (RTT_2        << 18)              
    | (DIC_1        << 17)
    | (DLL          << 16)
    | (DLL_PPDN     << 12)
    | (WR           <<  9)
    | (DLL_RESET    <<  8) 
    | (MODE         <<  7) 
    | (CAS_Latency_654  <<  4)
    | (Burst_Type       <<  3)
| (CAS_Latency_2    <<  2)
    | (BL_DDR3);


    //--------------------
    // --- Offset 0x0C ---
    //-------------------- 
    //MR3[2], MPR operation 1'b0 = normal operation, 1'b1 = Dataflow from MPR
    //MR3[1:0] 2'b00 = Predefined pattern

    //MR2[10:9] RTT_WR 2'b00 = Dynamic ODT off, 2'b01 = RZQ/4, 2'b10 = RZQ/2  
    //MR2[6] ASR Auto self refresh, 1'b0 = Manual 1'b1 = ASR enable   
    //MR2[5:3] CWL 3'b000 = 5, 3'b001 = 6, 3'b010 =7, 3'b011 = 8, 3'b100 =9, 
    // 3'b101 = 10, 3'b110 = 11, 3'b111 =12


    const unsigned int MPR    = 0x0;    //MR3[2], bit[18]
    const unsigned int MPR_RF = 0x0;    //MR3[1:0], bit[17:16]
    const unsigned int RTT_WR = 0x0;    //MR2[10:9], 2'b00 = Dynamic ODT off, 2'b01=RZQ/4, 2'b10=RZQ/2
    const unsigned int SRT    = 0x0;    //MR2[7], Self-Refresh Temperature, 1'b0=normal, 1'b1=extended
    const unsigned int ASR    = 0x1;    //MR2[6], 1'b0=manual, 1'b1=automatic
    const unsigned int CWL    = 0x1;    //MR2[5:3], 3'b000 = 5, 3'b001 = 6, 3'b010 =7, 3'b011 = 8, 3'b100 =9, 
    // 3'b101 = 10, 3'b110 = 11, 3'b111 =12
    const unsigned int PASR   = 0x0;   //MR2[2:0] PASR Partial Array Self Refresh  

    reg0C_value =
    (MPR        << 18)
    | (MPR_RF     << 16)
    | (RTT_WR     << 9)
    | (SRT        << 7)
    | (ASR        << 6)
| (CWL         << 3)
    | (PASR);

    //--------------------
    // --- Offset 0x10 ---
    //-------------------- 
    // Rank0_base;  //bit[31:24]
    // Rank1_en;    //bit[16]
    // Rank1_type;  //bit[14:12]
    // Rank1_size;  //bit[10:8] 
    // Rank0_type;  //bit[6:4]
    // Rank0_size;  //bit[2:0]

    const unsigned int Rank0_base = 0x20;
    const unsigned int Rank1_en =   0x0;     // 2 Ranks

    const unsigned int Rank1_type = 0x0;     //15x10x3 // LOGIC_ADDR_NO
    const unsigned int Rank1_size = 0x0;    //32bit 1G bytes // LOGIC_ADDR_NO

#ifdef DDR_BUS_32BIT
#if defined(DDR_SIZE_128M)
    const unsigned int Rank0_type = 0x3;
    const unsigned int Rank0_size = 0x3;
#elif defined(DDR_SIZE_256M)
    const unsigned int Rank0_type = 0x4;
    const unsigned int Rank0_size = 0x4;
#elif defined(DDR_SIZE_512M)
    const unsigned int Rank0_type = 0x5;
    const unsigned int Rank0_size = 0x5;
#elif defined(DDR_SIZE_1024M)
    const unsigned int Rank0_type = 0x6;
    const unsigned int Rank0_size = 0x6;
#else
    "unknown ddr size"
#endif

#else	// DDR_BUS_16BIT
#if defined(DDR_SIZE_64M)
    const unsigned int Rank0_type = 0x3;
    const unsigned int Rank0_size = 0x2;
#elif defined(DDR_SIZE_128M)
    const unsigned int Rank0_type = 0x4;
    const unsigned int Rank0_size = 0x3;
#elif defined(DDR_SIZE_256M)
    const unsigned int Rank0_type = 0x5;
    const unsigned int Rank0_size = 0x4;
#elif defined(DDR_SIZE_512M)
    const unsigned int Rank0_type = 0x6;
    const unsigned int Rank0_size = 0x5;
#else
    "unknown ddr size"
#endif

#endif




    reg10_value =
    (Rank0_base << 24)
    | (Rank1_en   << 16)
    | (Rank1_type << 12)
    | (Rank1_size << 8)             
| (Rank0_type << 4)
    | (Rank0_size);

    //DDR3 1066
    //--------------------
    // --- Offset 0x14 ---
    //-------------------- 
    const unsigned int TRFC = 0x24;  
    const unsigned int TFAW = 0xe;  
    const unsigned int TRC  = 0x10;  
    const unsigned int TRAS = 0xb;    

    reg14_value = 
    (TRFC << 24)
    | (TFAW << 16)
| (TRC  <<  8)
    | (TRAS);

    //--------------------
    // --- Offset 0x18 ---
    //-------------------- 
    const unsigned int TWTR = 0x4;  
    const unsigned int TRTP = 0x2;   
    const unsigned int TWR  = 0x9;  
    const unsigned int TMOD = 0x5;  
    const unsigned int TMRD = 0x1;      
    const unsigned int TRP  = 0x4;   
    const unsigned int TRRD = 0x3;          
    const unsigned int TRCD = 0x4;    

    reg18_value = 
    (TWTR << 28)
    | (TRTP << 24)
    | (TWR  << 20)
    | (TMOD << 16)
    | (TMRD << 12) 
    | (TRP  <<  8)
| (TRRD <<  4)
    | (TRCD);

    //--------------------
    // --- Offset 0x1C ---
    //-------------------- 
    const unsigned int TWtoR_ctrl = 0x0;  
    const unsigned int TWtoW_ctrl = 0x0;
    const unsigned int TRtoR_ctrl = 0x0;
    const unsigned int TRtoW_ctrl = 0x0;  
    const unsigned int TXSR       = 0x20; 

    // Refresh interval = TREFI(32 x 0x0f) x MCLK cycle(500MHz->2ns) x Post Refresh Cnt(4) = 3840ns
    // Requirement. Commercial: 7800ns, Industrial: 3900ns
    const unsigned int TREFI      = 0x0f;
    //const unsigned int TREFI      = 0x03;

    reg1C_value =
    (TWtoR_ctrl << 30)
    | (TWtoW_ctrl << 28)
    | (TRtoR_ctrl << 26)      
    | (TRtoW_ctrl << 24)
| (TXSR       << 8)
    | (TREFI);

    //--------------------
    // --- Offset 0x20 ---
    //-------------------- 

    const unsigned int phy_odd          = 0x0;
    const unsigned int AUTO_IO_DEEP_PDN = 0x0;
    const unsigned int SELF_BIAS        = 0x0;
    const unsigned int PHY_CON_UPDATE   = 0x0;  
    const unsigned int CLKPHASESHIFT    = 0x1;
    const unsigned int DQIE             = 0x1;
    const unsigned int CLKOEN           = 0x1;
    const unsigned int CMDADDROEN       = 0x1;
    const unsigned int ODTOEN           = 0x1;
    const unsigned int AUTO_IO_CTRL_PDN = 0x0;
    const unsigned int ODTMD_DQS            = 0x1;
    const unsigned int SIO              = 0x0;       
    const unsigned int ODTMD_DQ            = 0x1;

    reg20_value =   
    (phy_odd          << 17)
    | (AUTO_IO_DEEP_PDN << 15)
    | (SELF_BIAS        << 14)
    | (PHY_CON_UPDATE   << 13)
    | (CLKPHASESHIFT    << 12)
    | (DQIE             << 11)
    | (CLKOEN           << 10)
    | (CMDADDROEN       << 9)
    | (ODTOEN           << 8)
    | (AUTO_IO_CTRL_PDN << 7)
    | (ODTMD_DQS << 4) 
    | (SIO              << 3)
    | (ODTMD_DQ); 


    //--------------------
    // --- Offset 0x24 ---
    //-------------------- 
#if 0
    const unsigned int DLLSEL_B7 = 0x0;
    const unsigned int DLLSEL_B6 = 0x0;
    const unsigned int DLLSEL_B5 = 0x0;
    const unsigned int DLLSEL_B4 = 0x0;
    const unsigned int DLLSEL_B3 = 0x0;
    const unsigned int DLLSEL_B2 = 0x0;
    const unsigned int DLLSEL_B1 = 0x0;
    const unsigned int DLLSEL_B0 = 0x0;
#else
    const unsigned int DLLSEL_B7 = 0x7;
    const unsigned int DLLSEL_B6 = 0x7;
    const unsigned int DLLSEL_B5 = 0x7;
    const unsigned int DLLSEL_B4 = 0x7;
    const unsigned int DLLSEL_B3 = 0x7;
    const unsigned int DLLSEL_B2 = 0x7;
    const unsigned int DLLSEL_B1 = 0x7;
    const unsigned int DLLSEL_B0 = 0x7;
#endif
    reg24_value =
    (DLLSEL_B7 << 28)
    | (DLLSEL_B6 << 24)
    | (DLLSEL_B5 << 20)
    | (DLLSEL_B4 << 16)
    | (DLLSEL_B3 << 12)
    | (DLLSEL_B2 << 8)
| (DLLSEL_B1 << 4)
    | (DLLSEL_B0);

#if 0
    //--------------------
    // --- Offset 0x28 ---
    //-------------------- 
    const unsigned int DIN = 0x0;
    const unsigned int DIP = 0x0;
    const unsigned int COMP_SEL = 0x0;
    //const unsigned int COMP_SEL = 0x1;

    reg28_value =
    (DIN << 7)
| (DIP << 1)
    | (COMP_SEL);

    //--------------------
    // --- Offset 0x2C ---
    //-------------------- 

    const unsigned int AUTO_SRF_EN    = 0x0;    //bit[28]  
    const unsigned int AUTO_SRF_TIMER = 0x0;    //bit[27:16]
    const unsigned int AUTO_PDN_EN    = 0x0;    //bit[12]  
    const unsigned int AUTO_PDN_TIMER = 0x0;    //bit[11:0]

    reg2C_value =
    (AUTO_SRF_EN    << 28)
    | (AUTO_SRF_TIMER << 16)
| (AUTO_PDN_EN    << 12)
    | (AUTO_PDN_TIMER);
#endif

    //--------------------
    // --- Offset 0x30 ---
    //-------------------- 
    const unsigned int RW_Grp_EN = 0x0;  
    const unsigned int group_grant_cnt = 0x16; // 0001 0110
    //const unsigned int Burst_orient_ch = 0xd5;  
    //const unsigned int Burst_orient_ch_true = 0xd5;  // 1101 0101
    const unsigned int Burst_orient_ch_true = 0xff;  // 1101 0101
    const unsigned int Ch_hi_prior = 0x00;  //High priority: 0, 2, 5, 6, 7
    const unsigned int Reorder_burst_en = 0x00;

    reg30_value =
    (RW_Grp_EN << 31)
    | (group_grant_cnt << 24)
    | (Reorder_burst_en << 16)
| (Burst_orient_ch_true << 8)
    | (Ch_hi_prior);


    //--------------------
    // --- Offset 0x34 ---
    //--------------------
    const unsigned int GrantCnt0 = 0x6;
    const unsigned int GrantCnt1 = 0x5;
    const unsigned int GrantCnt2 = 0x5;
    const unsigned int GrantCnt3 = 0xf;

    reg34_value =
    (GrantCnt3 << 24)
    | (GrantCnt2 << 16)
| (GrantCnt1 << 8)
    | (GrantCnt0);

    //--------------------
    // --- Offset 0x38 ---
    //--------------------
    const unsigned int GrantCnt4 = 0x5;
    const unsigned int GrantCnt5 = 0x3;
    const unsigned int GrantCnt6 = 0x8;
    const unsigned int GrantCnt7 = 0x5;

    reg38_value =
    (GrantCnt7 << 24)
    | (GrantCnt6 << 16)
| (GrantCnt5 << 8)
    | (GrantCnt4);

    //--------------------
    // --- Offset 0x3C ---
    //--------------------

    const unsigned int cf_tphy_wrlat  = 0x2;
    const unsigned int cf_tphy_wrdata = 0x1;
    const unsigned int cf_trddata_en  = 0x2; 
    const unsigned int cf_tphy_rdlat  = 0x0;

    reg3C_value =   
    (cf_tphy_rdlat   << 20)
    | (cf_trddata_en   << 16)
| (cf_tphy_wrdata  << 4)
    | (cf_tphy_wrlat);


    //--------------------
    // --- Offset 0x40 ---
    //--------------------   
    const unsigned int debug_int_en = 0x0;
    const unsigned int flush_int_en = 0xaa;
    const unsigned int flush_en = 0x0;

    reg40_value =
    (debug_int_en << 16)
| (flush_int_en << 8)
    | (flush_en);  



    //--------------------
    // --- Offset 0x48 ---
    //--------------------
    const unsigned int CH0_split_disable = 0x0;
    const unsigned int CH1_split_disable = 0x0;
    const unsigned int CH2_split_disable = 0x0;
    const unsigned int CH3_split_disable = 0x0;
    const unsigned int CH4_split_disable = 0x0;
    const unsigned int CH5_split_disable = 0x0;
    const unsigned int CH6_split_disable = 0x0;
    const unsigned int CH7_split_disable = 0x0;

    const unsigned int CH0_hprot_sel     = 0x01;
    const unsigned int CH1_hprot_sel     = 0x01;  
    const unsigned int CH2_hprot_sel     = 0x01;  
    const unsigned int CH3_hprot_sel     = 0x01;  
    const unsigned int CH4_hprot_sel     = 0x01;  
    const unsigned int CH5_hprot_sel     = 0x01;  
    const unsigned int CH6_hprot_sel     = 0x01;  
    const unsigned int CH7_hprot_sel     = 0x01;

    reg48_value =
    (CH7_hprot_sel     << 22)
    | (CH6_hprot_sel     << 20)
    | (CH5_hprot_sel     << 18)
    | (CH4_hprot_sel     << 16)
    | (CH3_hprot_sel     << 14)
    | (CH2_hprot_sel     << 12)
    | (CH1_hprot_sel     << 10)
    | (CH0_hprot_sel     << 8)
    | (CH7_split_disable << 7)
    | (CH6_split_disable << 6)     
    | (CH5_split_disable << 5)    
    | (CH4_split_disable << 4)   
    | (CH3_split_disable << 3)  
    | (CH2_split_disable << 2) 
| (CH1_split_disable << 1)
    | (CH0_split_disable) ; 




    //--------------------
    // --- Offset 0x4C ---
    //--------------------   
    //const unsigned int ZQ_update = 0x0; 
    //const unsigned int tWEVEL_UP = 0x10; 
    //const unsigned int tDLL_UP   = 0x05;

    const unsigned int ZQ_update = 0x0; 
    const unsigned int tWEVEL_UP = 0x0; 
    const unsigned int tDLL_UP   = 0x0;

    reg4C_value =
    (ZQ_update << 16)
| (tWEVEL_UP << 8)
    | (tDLL_UP);  



    //--------------------
    // --- Offset 0x60 ---
    //--------------------   
    // ihkong - 0914
#ifdef DDR_AUTO_LEVELING
	const unsigned int wlevel_byte7_hw_disable = 0x0;
    const unsigned int wlevel_byte6_hw_disable = 0x0;
    const unsigned int wlevel_byte5_hw_disable = 0x0;
    const unsigned int wlevel_byte4_hw_disable = 0x0;
    const unsigned int wlevel_byte3_hw_disable = 0x0;
    const unsigned int wlevel_byte2_hw_disable = 0x0;
    const unsigned int wlevel_byte1_hw_disable = 0x0;
    const unsigned int wlevel_byte0_hw_disable = 0x0;
#else
	const unsigned int wlevel_byte7_hw_disable = 0x1;
    const unsigned int wlevel_byte6_hw_disable = 0x1;
    const unsigned int wlevel_byte5_hw_disable = 0x1;
    const unsigned int wlevel_byte4_hw_disable = 0x1;
    const unsigned int wlevel_byte3_hw_disable = 0x1;
    const unsigned int wlevel_byte2_hw_disable = 0x1;
    const unsigned int wlevel_byte1_hw_disable = 0x1;
    const unsigned int wlevel_byte0_hw_disable = 0x1;
#endif
    const unsigned int tWLO = 0x6;   

    reg60_value =
    (wlevel_byte7_hw_disable << 23)
    | (wlevel_byte6_hw_disable << 22)
    | (wlevel_byte5_hw_disable << 21)
    | (wlevel_byte4_hw_disable << 20)
    | (wlevel_byte3_hw_disable << 19)
    | (wlevel_byte2_hw_disable << 18)
    | (wlevel_byte1_hw_disable << 17)
| (wlevel_byte0_hw_disable << 16)
    | (tWLO);  

    //--------------------
    // --- Offset 0x64 ---
    //--------------------   
    const unsigned int PDL_set_B7 = 0x0;  //bit[30:24]
    const unsigned int PDL_set_B6 = 0x0;  //bit[22:16]
    const unsigned int PDL_set_B5 = 0x0;  //bit[14:8]
    const unsigned int PDL_set_B4 = 0x0;  //bit[6:0]

    reg64_value =
    (PDL_set_B7 << 24)
    | (PDL_set_B6 << 16)
| (PDL_set_B5 << 8)
    | (PDL_set_B4);  


    //--------------------
    // --- Offset 0x68 ---
    //--------------------   
    const unsigned int PDL_set_B3 = 0x0;  //bit[30:24]
    const unsigned int PDL_set_B2 = 0x0;  //bit[22:16]
    const unsigned int PDL_set_B1 = 0x0;  //bit[14:8]
    const unsigned int PDL_set_B0 = 0x0;  //bit[6:0]

    reg68_value =
    (PDL_set_B3 << 24)
    | (PDL_set_B2 << 16)
| (PDL_set_B1 << 8)
    | (PDL_set_B0);  


    //--------------------
    // --- Offset 0x6C ---
    //--------------------   
    const unsigned int dqs_sel_wr_byte7 = 0x0; //bit[29]
    const unsigned int dqs_sel_byte7    = 0x0; //bit[28]
    const unsigned int dqs_sel_wr_byte6 = 0x0; //bit[25]
    const unsigned int dqs_sel_byte6    = 0x0; //bit[24]
    const unsigned int dqs_sel_wr_byte5 = 0x0; //bit[21]
    const unsigned int dqs_sel_byte5    = 0x0; //bit[20]
    const unsigned int dqs_sel_wr_byte4 = 0x0; //bit[17]
    const unsigned int dqs_sel_byte4    = 0x0; //bit[16]
    const unsigned int dqs_sel_wr_byte3 = 0x0; //bit[13]
    const unsigned int dqs_sel_byte3    = 0x0; //bit[12]
    const unsigned int dqs_sel_wr_byte2 = 0x0; //bit[9]
    const unsigned int dqs_sel_byte2    = 0x0; //bit[8]
    const unsigned int dqs_sel_wr_byte1 = 0x0; //bit[5]
    const unsigned int dqs_sel_byte1    = 0x0; //bit[4]
    const unsigned int dqs_sel_wr_byte0 = 0x0; //bit[1]
    const unsigned int dqs_sel_byte0    = 0x0; //bit[0]

    reg6C_value =
    (dqs_sel_wr_byte7 << 29)
    | (dqs_sel_byte7    << 28)
    | (dqs_sel_wr_byte6 << 25)
    | (dqs_sel_byte6    << 24)
    | (dqs_sel_wr_byte5 << 21)
    | (dqs_sel_byte5    << 20)
    | (dqs_sel_wr_byte4 << 17)
    | (dqs_sel_byte4    << 16)
    | (dqs_sel_wr_byte3 << 13)
    | (dqs_sel_byte3    << 12)
    | (dqs_sel_wr_byte2 << 9)
    | (dqs_sel_byte2    << 8)
    | (dqs_sel_wr_byte1 << 5)
    | (dqs_sel_byte1    << 4)
| (dqs_sel_wr_byte0 << 1)
    | (dqs_sel_byte0);  


    //--------------------
    // --- Offset 0x70 ---
    //--------------------   
	// ihkong - 0914
#ifdef DDR_AUTO_LEVELING
    const unsigned int rlevel_byte7_hw_disable = 0x0;
    const unsigned int rlevel_byte6_hw_disable = 0x0;
    const unsigned int rlevel_byte5_hw_disable = 0x0;
    const unsigned int rlevel_byte4_hw_disable = 0x0;
    const unsigned int rlevel_byte3_hw_disable = 0x0;
    const unsigned int rlevel_byte2_hw_disable = 0x0;
    const unsigned int rlevel_byte1_hw_disable = 0x0;
    const unsigned int rlevel_byte0_hw_disable = 0x0;

#else
    const unsigned int rlevel_byte7_hw_disable = 0x1;
    const unsigned int rlevel_byte6_hw_disable = 0x1;
    const unsigned int rlevel_byte5_hw_disable = 0x1;
    const unsigned int rlevel_byte4_hw_disable = 0x1;
    const unsigned int rlevel_byte3_hw_disable = 0x1;
    const unsigned int rlevel_byte2_hw_disable = 0x1;
    const unsigned int rlevel_byte1_hw_disable = 0x1;
    const unsigned int rlevel_byte0_hw_disable = 0x1;
#endif
	
    reg70_value =
    (rlevel_byte7_hw_disable << 7)
    | (rlevel_byte6_hw_disable << 6)
    | (rlevel_byte5_hw_disable << 5)
    | (rlevel_byte4_hw_disable << 4)
    | (rlevel_byte3_hw_disable << 3)
    | (rlevel_byte2_hw_disable << 2)
| (rlevel_byte1_hw_disable << 1)
    | (rlevel_byte0_hw_disable);  


    //--------------------
    // --- Offset 0x74 ---
    //--------------------   

    const unsigned int msdly_byte7 = 0x1; 
    const unsigned int msdly_byte6 = 0x1; 
    const unsigned int msdly_byte5 = 0x1; 
    const unsigned int msdly_byte4 = 0x1; 
    const unsigned int msdly_byte3 = 0x1; 
    const unsigned int msdly_byte2 = 0x1; 
    const unsigned int msdly_byte1 = 0x1; 
    const unsigned int msdly_byte0 = 0x1; 

    reg74_value =
    (msdly_byte7 << 28)
    | (msdly_byte6 << 24)
    | (msdly_byte5 << 20)
    | (msdly_byte4 << 16)
    | (msdly_byte3 << 12)
    | (msdly_byte2 << 8)
| (msdly_byte1 << 4)
    | (msdly_byte0);  



    //--------------------
    // --- Offset 0x78 ---
    //--------------------   

    const unsigned int wrdll_sel_byte7 = 0x0; 
    const unsigned int wrdll_sel_byte6 = 0x0; 
    const unsigned int wrdll_sel_byte5 = 0x0; 
    const unsigned int wrdll_sel_byte4 = 0x0; 
    const unsigned int wrdll_sel_byte3 = 0x0; 
    const unsigned int wrdll_sel_byte2 = 0x0; 
    const unsigned int wrdll_sel_byte1 = 0x0; 
    const unsigned int wrdll_sel_byte0 = 0x0; 

    reg78_value =
    (wrdll_sel_byte7 << 28)
    | (wrdll_sel_byte6 << 24)
    | (wrdll_sel_byte5 << 20)
    | (wrdll_sel_byte4 << 16)
    | (wrdll_sel_byte3 << 12)
    | (wrdll_sel_byte2 << 8)
| (wrdll_sel_byte1 << 4)
    | (wrdll_sel_byte0);  

    //--------------------
    // --- Offset 0x7C ---
    //--------------------   

    const unsigned int TM_cycle_reg = 0x0; 

    reg7C_value = TM_cycle_reg;


    //--------------------
    // --- Offset 0xA0 ---
    //--------------------

    const unsigned int CH0_pref_value   = 0x1;
    const unsigned int CH0_limited_pref = 0x1;  
    const unsigned int CH1_pref_value   = 0x1;
    const unsigned int CH1_limited_pref = 0x1;  
    const unsigned int CH2_pref_value   = 0x1;
    const unsigned int CH2_limited_pref = 0x1;  
    const unsigned int CH3_pref_value   = 0x1;
    const unsigned int CH3_limited_pref = 0x1;  

    regA0_value =
    (CH3_limited_pref  << 31)
    | (CH3_pref_value    << 24)     
    | (CH2_limited_pref  << 23)     
    | (CH2_pref_value    << 16)    
    | (CH1_limited_pref  << 15)   
    | (CH1_pref_value    << 8) 
| (CH0_limited_pref  << 7)
    | (CH0_pref_value); 

    //--------------------
    // --- Offset 0xA4 ---
    //--------------------

    const unsigned int CH4_pref_value   = 0x1;
    const unsigned int CH4_limited_pref = 0x1;  
    const unsigned int CH5_pref_value   = 0x1;
    const unsigned int CH5_limited_pref = 0x1;  
    const unsigned int CH6_pref_value   = 0x1;
    const unsigned int CH6_limited_pref = 0x1;  
    const unsigned int CH7_pref_value   = 0x1;
    const unsigned int CH7_limited_pref = 0x1;  

    regA4_value =
    (CH7_limited_pref  << 31)
    | (CH7_pref_value    << 24)     
    | (CH6_limited_pref  << 23)
    | (CH6_pref_value    << 16)      
    | (CH5_limited_pref  << 15)      
    | (CH5_pref_value    << 8)     
| (CH4_limited_pref  << 7)
    | (CH4_pref_value); 

    //--------------------
    // --- Offset 0xA8 ---
    //--------------------

    //wait_cycle_200us = 20'h1a0ab;
    const unsigned int wait_cycle_200us = 0x00200;

    regA8_value = wait_cycle_200us;

    //--------------------
    // --- Offset 0xAC ---
    //--------------------

    //wait_cycle_500us = 20'h61a80;
    const unsigned int wait_cycle_500us = 0x1f000;

    regAC_value = wait_cycle_500us;


    //--------------------
    // --- Offset 0xB0 ---
    //--------------------

    //const unsigned int QoS_En = 0x1;
    const unsigned int QoS_En = 0x0;
    const unsigned int QoS_Period = 0x1; //0: 512 cycles
    //1: 1024 cycles
    //2: 2048 cycles
    //3: 4096 cycles  

    regB0_value=
(QoS_Period <<1)
    | (QoS_En);

    //--------------------
    // --- Offset 0xB4 ---
    //--------------------

    const unsigned int ch3_QoS_CmdCnt = 0x3;
    const unsigned int ch2_QoS_CmdCnt = 0x3;
    const unsigned int ch1_QoS_CmdCnt = 0x4;
    const unsigned int ch0_QoS_CmdCnt = 0x4;


    regB4_value=
    (ch3_QoS_CmdCnt << 24)
    | (ch2_QoS_CmdCnt << 16) 
| (ch1_QoS_CmdCnt << 8) 
    | (ch0_QoS_CmdCnt);

    //--------------------
    // --- Offset 0xB8 ---
    //--------------------


    const unsigned int ch7_QoS_CmdCnt = 0x1;
    const unsigned int ch6_QoS_CmdCnt = 0x1;
    const unsigned int ch5_QoS_CmdCnt = 0x2;
    const unsigned int ch4_QoS_CmdCnt = 0x2;


    regB8_value=
    (ch7_QoS_CmdCnt << 24)
    | (ch6_QoS_CmdCnt << 16)
| (ch5_QoS_CmdCnt << 8)
    | (ch4_QoS_CmdCnt);




    *(volatile unsigned int *) (DDR_REG_BASE + 0x00) = reg00_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x04) = reg04_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x08) = reg08_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x0C) = reg0C_value;  
    *(volatile unsigned int *) (DDR_REG_BASE + 0x10) = reg10_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x14) = reg14_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x18) = reg18_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x1c) = reg1C_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x20) = reg20_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x24) = reg24_value;
    //*(volatile unsigned int *) (DDR_REG_BASE + 0x28) = reg28_value;
    //*(volatile unsigned int *) (DDR_REG_BASE + 0x2C) = reg2C_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x30) = reg30_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x34) = reg34_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x38) = reg38_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x3C) = reg3C_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x40) = reg40_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x48) = reg48_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x4C) = reg4C_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x60) = reg60_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x64) = reg64_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x68) = reg68_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x6C) = reg6C_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x70) = reg70_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x74) = reg74_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x78) = reg78_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x7C) = reg7C_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0xa0) = regA0_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0xa4) = regA4_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0xa8) = regA8_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0xaC) = regAC_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0xb0) = regB0_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0xb4) = regB4_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0xb8) = regB8_value;

    *(volatile unsigned int *) (DDR_REG_BASE + 0x138) = 0x5;

    *(volatile unsigned int *) (DDR_REG_BASE + 0x134) = 0x00304444;

    //wait 500us
    for(i=0;i<50;i++)
    *(volatile unsigned int *) (DDR_REG_BASE + 0x00) = reg00_value;

    MemCmd_initial = 0x1;

    reg04_value =
    (MRS_mode        << 4)
    | (MemCmd_exit_srf << 3)
    | (MemCmd_srf      << 2)
| (MemCmd_mrs      << 1)
    | (MemCmd_initial); 

    *(volatile unsigned int *) (DDR_REG_BASE + 0x04) = reg04_value;

    data = *(volatile unsigned int *) (DDR_REG_BASE + 0x04);

while ( (data & 0x100) != 0x100) 
    data = *(volatile unsigned int *) (DDR_REG_BASE + 0x04);

    //DDR test write & verify
    *(volatile unsigned int *) (DDR_MEM_BASE) = 0x01020304; data = *(volatile unsigned int *) (DDR_MEM_BASE);
if (data == 0x01020304)
{
    *(volatile unsigned int *) (DDR_MEM_BASE) = 0x89abcdef;
}
else
{
    fail();
}

}
