#include <stdio.h>
#include "system.h"
#include "../../inc/ddr_calibration.h"
#include "debug.h"

#define DDR_CALIB_TESET 	0

#define NUM_PATTERN		8

#ifdef DDR_BUS_16BIT
static int ddr_bus_bit = 16;
#else
static int ddr_bus_bit = 32;
#endif

static void finish(void)
{
    //(*(volatile unsigned int *) 0x80000080 = 0x21522152);
}

static void fail(void)
{
    //(*(volatile unsigned int *) 0x80000080 = 0xdeaddead);
}


static int find_center(unsigned int table_array[], int number)
{
    int final_center = -1;
    int cont_pass = 1;
    int i;

    int wmax_start = 0;
    int wmax_num = 0;
    int w_start = 0;
    int w_num = 0;

    cont_pass = 0;
    for(i=0;i<number;i++){
    	if(table_array[i] == NUM_PATTERN){
    		if(cont_pass == 0){
    			w_start = i;
    			w_num = 0;
    		}

    		w_num++;
    		cont_pass = 1;
    	}else{
    		if(w_num > wmax_num){
    			wmax_start = w_start;
    			wmax_num = w_num;
    		}
    		w_num = 0;
    		cont_pass = 0;
    	}

    }
    if(cont_pass){
		if(w_num > wmax_num){
			wmax_start = w_start;
			wmax_num = w_num;
		}
    }

    if(wmax_num > 0){
    	final_center = wmax_start + (wmax_num-1)/2;
    }else{
    	final_center = -1;
    }

    if(final_center == -1)
    {
        fail();
    }

    return final_center;
}

static int Write_Read_compare(int byte_mode, int bit_mode)
{
	unsigned int pattern[NUM_PATTERN] = {0x55555555, 0xAAAAAAAA, 0x44444444, 0xDDDDDDDD, 0x11111111, 0xEEEEEEEE, 0x0F0F0F0F, 0xF0F0F0F0};
	int pass = 0;
	int i;

	if(byte_mode < 4){							// byte 0, 1, 2, 3
		// Write
		for(i=0;i<NUM_PATTERN;i++){
			*(volatile unsigned char *) (DDR_MEM_BASE+byte_mode+i*(bit_mode>>3)) = (volatile unsigned char)(pattern[i]&0xff);
		}

		// Read & Compare
		for(i=0;i<NUM_PATTERN;i++){
			if(*(volatile unsigned char *) (DDR_MEM_BASE+byte_mode+i*(bit_mode>>3)) == (volatile unsigned char)(pattern[i]&0xff) )pass++;
		}
	}else if(byte_mode == 4 && byte_mode < 6){	// short 0, 1
		// Write
		for(i=0;i<NUM_PATTERN;i++){
			*(volatile unsigned short *) (DDR_MEM_BASE+(byte_mode-4)*2+i*(bit_mode>>3)) = (volatile unsigned short)(pattern[i]&0xffff);
		}

		// Read & Compare
		for(i=0;i<NUM_PATTERN;i++){
			if(*(volatile unsigned short *) (DDR_MEM_BASE+(byte_mode-4)*2+i*(bit_mode>>3)) == (volatile unsigned short)(pattern[i]&0xffff) )pass++;
		}

	}else{										// word 0
		// Write
		for(i=0;i<NUM_PATTERN;i++){
			*(volatile unsigned int *) (DDR_MEM_BASE+(byte_mode-6)+i*4) = (volatile unsigned int)(pattern[i]&0xffff);
		}

		// Read & Compare
		for(i=0;i<NUM_PATTERN;i++){
			if(*(volatile unsigned int *) (DDR_MEM_BASE+(byte_mode-6)+i*4) == (volatile unsigned int)(pattern[i]&0xffff) )pass++;
		}

	}

	return pass;
}

#if DDR_CALIB_TESET


typedef struct
{
	int max;
	int pass;
}CalbCnt;
static void do_calib_test(void)
{
	int i0, i1, i2, i3, i4, i5, i6, i7,i8;
	volatile unsigned int reg;
	unsigned int val;
	int match_count;
	int not_match_count = 0;
	int total_loop_count = 0;

	CalbCnt stCnt[9] = {
			{1, 0},		// byte
			{128, 0}, 	// Write leveling
			{16, 0}, 	// read leveling
			{8, 0}, 	// DQ rd
			{8, 1}, 	// DQS rd
			{8, 0}, 	// DQ rd falling
			{8, 1}, 	// DQS rd falling
			{8, 0}, 	// DQ wr
			{8, 1}, 	// DQS wr

	};
	*(volatile unsigned int *) (DDR_REG_BASE + 0x68) = 0;
	*(volatile unsigned int *) (DDR_REG_BASE + 0x74) = 0;
	*(volatile unsigned int *) (DDR_REG_BASE + 0x24) = 0;
	*(volatile unsigned int *) (DDR_REG_BASE + 0x130) = 0;
	*(volatile unsigned int *) (DDR_REG_BASE + 0x78) = 0;

	for(i0=0;i0<stCnt[0].max;i0++){									// byte 0~3
		for(i1=0;i1<stCnt[1].max;i1++){								// Write leveling
			if(stCnt[1].pass)stCnt[1].max=1;
			else{
				*(volatile unsigned int *) (DDR_REG_BASE + 0x68) &= ~(0xff<<(i0*8));
				*(volatile unsigned int *) (DDR_REG_BASE + 0x68) |= (i1<<(i0*8));
			}
			printf("i0: %d, i1: %d\n", i0, i1);
			for(i2=0;i2<stCnt[2].max;i2++){							// Read leveling
				if(stCnt[2].pass)stCnt[2].max=1;
				else{
					*(volatile unsigned int *) (DDR_REG_BASE + 0x74) &= ~(0xf<<(i0*4));
					*(volatile unsigned int *) (DDR_REG_BASE + 0x74) |= (i2<<(i0*4));
				}

				for(i3=0;i3<stCnt[3].max;i3++){						// DQ rd
					if(stCnt[3].pass)stCnt[3].max=1;
					else{
						*(volatile unsigned int *) (DDR_REG_BASE + 0x6C) &= ~0x1111;	// Read DQS is fixed, only delays DQ

						*(volatile unsigned int *) (DDR_REG_BASE + 0x24) &= ~(0xff<<(i0*8));
						*(volatile unsigned int *) (DDR_REG_BASE + 0x24) |= (i3<<(i0*8));
					}
					for(i4=0;i4<stCnt[4].max;i4++){					// DQS rd
						if(stCnt[4].pass)stCnt[4].max=1;
						else{
							*(volatile unsigned int *) (DDR_REG_BASE + 0x6C) |= 0x1111;	// Read DQ is fixed, only delays DQS
							*(volatile unsigned int *) (DDR_REG_BASE + 0x24) &= ~(0xff<<(i0*8));
							*(volatile unsigned int *) (DDR_REG_BASE + 0x24) |= (i4<<(i0*8));
						}
						for(i5=0;i5<stCnt[5].max;i5++){				// DQ rd falling
							if(stCnt[5].pass)stCnt[5].max=1;
							else{
								*(volatile unsigned int *) (DDR_REG_BASE + 0x6C) &= ~0x1111;	// Read DQS is fixed, only delays DQ

								*(volatile unsigned int *) (DDR_REG_BASE + 0x130) &= ~(0xff<<(i0*8));
								*(volatile unsigned int *) (DDR_REG_BASE + 0x130) |= (i5<<(i0*8));
							}
							for(i6=0;i6<stCnt[6].max;i6++){			// DQS rd falling
								if(stCnt[6].pass)stCnt[6].max=1;
								else{
									*(volatile unsigned int *) (DDR_REG_BASE + 0x6C) |= 0x1111;	// Read DQ is fixed, only delays DQS
									*(volatile unsigned int *) (DDR_REG_BASE + 0x130) &= ~(0xff<<(i0*8));
									*(volatile unsigned int *) (DDR_REG_BASE + 0x130) |= (i6<<(i0*8));
								}

								for(i7=0;i7<stCnt[7].max;i7++){		// DQ wr
									if(stCnt[7].pass)stCnt[7].max=1;
									else{
										*(volatile unsigned int *) (DDR_REG_BASE + 0x6C) &= ~0x2222;	// Write DQS is fixed, only delays DQ
										*(volatile unsigned int *) (DDR_REG_BASE + 0x78) &= ~(0xff<<(i0*8));
										*(volatile unsigned int *) (DDR_REG_BASE + 0x78) |= (i7<<(i0*8));
									}
									for(i8=0;i8<stCnt[8].max;i8++){	// DQS wr
										if(stCnt[8].pass)stCnt[8].max=1;
										else{
											*(volatile unsigned int *) (DDR_REG_BASE + 0x6C) |= 0x2222;	// Write DQ is fixed, only delays DQS
											*(volatile unsigned int *) (DDR_REG_BASE + 0x78) &= ~(0xff<<(i0*8));
											*(volatile unsigned int *) (DDR_REG_BASE + 0x78) |= (i8<<(i0*8));
										}
										LOOPDELAY_10USEC(1); // wait 10us
										match_count = Write_Read_compare(i0, ddr_bus_bit);
										if(match_count > 1){
											if(not_match_count){
												printf("###### No matched data for %d loops\n", not_match_count);
												not_match_count = 0;
											}
											printf("Match: %10d - Byte: %d, WL: %03d, RL: %02d, DQ RD: %d, DQS RD: %d, DQ RDF: %d, DQS RDF: %d, DQ WR: %d, DQS WR: %d\n",
													match_count, i0, i1, i2,i3, i4, i5, i6, i7, i8);

										}else{
											not_match_count++;
										}
										total_loop_count++;
									}
								}
							}
						}
					}
				}
			}
		}
	}


	printf("calibration test end. total loop: %d\n", total_loop_count);

}



#else
static void do_calibration_Leveling(int mode)  // write / read leveling
{

    unsigned int level_table[128];
    unsigned int i, j;

    unsigned char u8Reg[4];
    unsigned char u8RegDefault[4];
    unsigned int u32LevelCnt;
	unsigned int u32LevelRange = 128;
	int s32Center;


    volatile unsigned int reg68_value; // write leveling byte3~0 value
    volatile unsigned int reg74_value; // read leveling value msdly

#ifdef DDR_BUS_16BIT
    u32LevelCnt = 2;
#else
    u32LevelCnt = 4;
#endif

    if(mode == 0){
		// -------------- Write leveling --------------//
		u32LevelRange = 128;
		reg68_value = *(volatile unsigned int *) (DDR_REG_BASE + 0x68);
		for(i=0;i<u32LevelCnt;i++){
			u8RegDefault[i] = ((reg68_value >> (i*8))&0xff);
			u8Reg[i] = 0;
		}

		//dbg("Write Leveling\n");
		for(i=0;i<u32LevelCnt;i++){
			u8Reg[i] = 0;
			for(j=0;j<u32LevelRange;j++){
				reg68_value = ( (u8Reg[3]<<24) | (u8Reg[2]<<16) | (u8Reg[1]<<8) | (u8Reg[0]<<0) );
				*(volatile unsigned int *) (DDR_REG_BASE + 0x68) = reg68_value;
				level_table[j] = Write_Read_compare(i, ddr_bus_bit);
				//dbg("Byte[%d] PDLSET value: %d, Match Cnt: %d\n", i, ( (*(volatile unsigned int *) (DDR_REG_BASE + 0x68))>>(i*8)) & 0xff, level_table[j] );
				u8Reg[i]++;
			}

			if( (s32Center = find_center(level_table, u32LevelRange)) == -1){
				u8Reg[i] = u8RegDefault[i];	// set to default
				//dbg("can't find window\n");
			}
			else u8Reg[i] = (unsigned char )s32Center;
			//dbg("center: %d\n", s32Center);
		}
		reg68_value = ( (u8Reg[3]<<24) | (u8Reg[2]<<16) | (u8Reg[1]<<8) | (u8Reg[0]<<0) );
		*(volatile unsigned int *) (DDR_REG_BASE + 0x68) = reg68_value;
    }else{
		// -------------- Read leveling --------------//

		u32LevelRange = 16;
		reg74_value = *(volatile unsigned int *) (DDR_REG_BASE + 0x74);
		for(i=0;i<u32LevelCnt;i++){
			u8RegDefault[i] = ((reg74_value >> (i*4))&0xf);
			u8Reg[i] = 0;
		}

		//dbg("Read Leveling\n");
		for(i=0;i<u32LevelCnt;i++){
			u8Reg[i] = 0;
			for(j=0;j<u32LevelRange;j++){
				reg74_value = ( (u8Reg[3]<<12) | (u8Reg[2]<<8) | (u8Reg[1]<<4) | (u8Reg[0]<<0) );
				*(volatile unsigned int *) (DDR_REG_BASE + 0x74) = reg74_value;

			    *(volatile unsigned int *) 0xF0000048 = 0x00003fDf; // sw rst (ddr_postclk, phy_pll, phy, postclk_2x)
			    *(volatile unsigned int *) 0xF0000048 = 0x00003fFf; // sw rst (ddr_postclk, phy_pll, phy, postclk_2x)


				level_table[j] = Write_Read_compare(i, ddr_bus_bit);
				//dbg("Byte[%d] Cmp Cnt[%d]: %d, reg: 0x%x\n", i, j, level_table[j], *(volatile unsigned int *) (DDR_REG_BASE + 0x74));
				//dbg("Byte[%d] MSDLY value: %d, Match Cnt: %d\n", i, ( (*(volatile unsigned int *) (DDR_REG_BASE + 0x74))>>(i*4)) & 0xff, level_table[j]);
				u8Reg[i]++;
			}

			if( (s32Center = find_center(level_table, u32LevelRange)) == -1){
				u8Reg[i] = u8RegDefault[i];	// set to default
				//dbg("can't find window\n");
			}
			else u8Reg[i] = (unsigned char )s32Center;
			//dbg("center: %d\n", s32Center);
		}
		reg74_value = ( (u8Reg[3]<<12) | (u8Reg[2]<<8) | (u8Reg[1]<<4) | (u8Reg[0]<<0) );
		*(volatile unsigned int *) (DDR_REG_BASE + 0x74) = reg74_value;
    }

}

static void do_calibration_GDS()
{

#if 0        
    unsigned int gds_table[8];
    unsigned int i;
    volatile unsigned int reg00_value;
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

    reg00_value = *(volatile unsigned int *) (DDR_REG_BASE + 0x00);

    for (i=0 ; i < 8; i++) {
        reg00_value = (reg00_value & 0xfffffff8) + i;
        *(volatile unsigned int *) (DDR_REG_BASE + 0x00) = reg00_value;

        // PHY Reset (DATA PHY)
        *(volatile unsigned int *) 0xF0000048 = 0x00003fDf; // sw rst (ddr_postclk, phy_pll, phy, postclk_2x)
        *(volatile unsigned int *) 0xF0000048 = 0x00003fFf; // sw rst (ddr_postclk, phy_pll, phy, postclk_2x)

        LOOPDELAY_10USEC(1); // wait 10us

        gds_table[i] = Write_Read_compare(6, ddr_bus_bit);
        //dbg("i:%d, gds_table:0x%08x\n", i, gds_table[i]);
    }

    // select middle value of passed GDS value
    {
        unsigned int final_center = 0;
        unsigned int cur_pass   = 0;
        unsigned int cont_pass = 1;
#if 1
        for (i=0 ; i < 8; i++) {
            if (gds_table[i] == 8) {
                if (cont_pass) final_center = cur_pass;
                cur_pass = i;
                cont_pass = 1;
            }
            else {
                cont_pass = 0;
            }
        }
#else //kiki
        final_center = find_center(gds_table, 8);
        printf("final center:%d,0x%08x\n", final_center, reg00_value);
        final_center = 2; //DDR400 => 1,2,3:ok, 0,4,5,6,7:NG
        //final_center = 4; //DDR500 => 2,3,4:ok, 0,1,5,6,7:NG
        reg00_value = (reg00_value & 0xfffffff8) + final_center;
        *(volatile unsigned int *) (DDR_REG_BASE + 0x00) = reg00_value;
        printf("center:%d,0x%08x\n", final_center, reg00_value);
#endif
    }
#endif

}

static void do_calibration_DQ_DQS_READ()
{
    volatile unsigned int reg24_value; // DQ/DQS value for read
    volatile unsigned int reg6C_value; // DQ/DQS sel(wr,rd) 1:DQS, 0:DQ
    volatile unsigned int reg130_value; // DQ/DQS fall value

    unsigned int DQSDLSEL_count;
    unsigned int DLSEL_RD_count;
    unsigned int DLSEL_RD_FALL_count;

    unsigned int DQ_RD_B0_table[8];
    unsigned int DQ_RD_B1_table[8];
    unsigned int DQ_RD_B2_table[8];
    unsigned int DQ_RD_B3_table[8];
    unsigned int DQS_RD_B0_table[8];
    unsigned int DQS_RD_B1_table[8];
    unsigned int DQS_RD_B2_table[8];
    unsigned int DQS_RD_B3_table[8];

    unsigned int DQ_RD_FALL_B0_table[8];
    unsigned int DQ_RD_FALL_B1_table[8];
    unsigned int DQ_RD_FALL_B2_table[8];
    unsigned int DQ_RD_FALL_B3_table[8];
    unsigned int DQS_RD_FALL_B0_table[8];
    unsigned int DQS_RD_FALL_B1_table[8];
    unsigned int DQS_RD_FALL_B2_table[8];
    unsigned int DQS_RD_FALL_B3_table[8];

    unsigned int DQ_RD_B0_result;
    unsigned int DQ_RD_B1_result;
    unsigned int DQ_RD_B2_result;
    unsigned int DQ_RD_B3_result;
    unsigned int DQS_RD_B0_result;
    unsigned int DQS_RD_B1_result;
    unsigned int DQS_RD_B2_result;
    unsigned int DQS_RD_B3_result;

    unsigned int DQ_RD_FALL_B0_result;
    unsigned int DQ_RD_FALL_B1_result;
    unsigned int DQ_RD_FALL_B2_result;
    unsigned int DQ_RD_FALL_B3_result;
    unsigned int DQS_RD_FALL_B0_result;
    unsigned int DQS_RD_FALL_B1_result;
    unsigned int DQS_RD_FALL_B2_result;
    unsigned int DQS_RD_FALL_B3_result;


    //:step 3 read eye traning
    // RD
    for (DQSDLSEL_count = 0; DQSDLSEL_count < 2; DQSDLSEL_count++) {
        // DQ / DQS select
        reg6C_value = (reg6C_value & 0x0000000)+(DQSDLSEL_count*0x1111);
        *(volatile unsigned int *) (DDR_REG_BASE + 0x6C) = reg6C_value;
        if (DQSDLSEL_count == 0) {
            // byte 0
            for (DLSEL_RD_count = 0; DLSEL_RD_count < 8; DLSEL_RD_count++) {
                reg24_value = (reg24_value & 0x0000000)+DLSEL_RD_count;
                *(volatile unsigned int *) (DDR_REG_BASE + 0x24) = reg24_value;
                LOOPDELAY_10USEC(1); // wait 10us

                DQ_RD_B0_table[DLSEL_RD_count] = Write_Read_compare(0, ddr_bus_bit);
            }
            // byte 1
            for (DLSEL_RD_count = 0; DLSEL_RD_count < 8; DLSEL_RD_count++) {
                reg24_value = (reg24_value & 0x0000000)+(DLSEL_RD_count<<8);
                *(volatile unsigned int *) (DDR_REG_BASE + 0x24) = reg24_value;
                LOOPDELAY_10USEC(1); // wait 10us

                DQ_RD_B1_table[DLSEL_RD_count] = Write_Read_compare(1, ddr_bus_bit);
            }

            // byte 2
            for (DLSEL_RD_count = 0; DLSEL_RD_count < 8; DLSEL_RD_count++) {
                reg24_value = (reg24_value & 0x0000000)+(DLSEL_RD_count<<16);
                *(volatile unsigned int *) (DDR_REG_BASE + 0x24) = reg24_value;
                LOOPDELAY_10USEC(1); // wait 10us

                DQ_RD_B2_table[DLSEL_RD_count] = Write_Read_compare(2, ddr_bus_bit);
            }

            // byte 3
            for (DLSEL_RD_count = 0; DLSEL_RD_count < 8; DLSEL_RD_count++) {
                reg24_value = (reg24_value & 0x0000000)+(DLSEL_RD_count<<24);
                *(volatile unsigned int *) (DDR_REG_BASE + 0x24) = reg24_value;
                LOOPDELAY_10USEC(1); // wait 10us

                DQ_RD_B3_table[DLSEL_RD_count] = Write_Read_compare(3, ddr_bus_bit);
            }
        }

        if (DQSDLSEL_count == 1) {
            // byte 0
            for (DLSEL_RD_count = 0; DLSEL_RD_count < 8; DLSEL_RD_count++) {
                reg24_value = (reg24_value & 0x0000000)+DLSEL_RD_count;
                *(volatile unsigned int *) (DDR_REG_BASE + 0x24) = reg24_value;
                LOOPDELAY_10USEC(1); // wait 10us

                DQS_RD_B0_table[DLSEL_RD_count] = Write_Read_compare(0, ddr_bus_bit);
            }
            // byte 1
            for (DLSEL_RD_count = 0; DLSEL_RD_count < 8; DLSEL_RD_count++) {
                reg24_value = (reg24_value & 0x0000000)+(DLSEL_RD_count<<8);
                *(volatile unsigned int *) (DDR_REG_BASE + 0x24) = reg24_value;
                LOOPDELAY_10USEC(1); // wait 10us

                DQS_RD_B1_table[DLSEL_RD_count] = Write_Read_compare(1, ddr_bus_bit);
            }

            // byte 2
            for (DLSEL_RD_count = 0; DLSEL_RD_count < 8; DLSEL_RD_count++) {
                reg24_value = (reg24_value & 0x0000000)+(DLSEL_RD_count<<16);
                *(volatile unsigned int *) (DDR_REG_BASE + 0x24) = reg24_value;
                LOOPDELAY_10USEC(1); // wait 10us

                DQS_RD_B2_table[DLSEL_RD_count] = Write_Read_compare(2, ddr_bus_bit);
            }

            // byte 3
            for (DLSEL_RD_count = 0; DLSEL_RD_count < 8; DLSEL_RD_count++) {
                reg24_value = (reg24_value & 0x0000000)+(DLSEL_RD_count<<24);
                *(volatile unsigned int *) (DDR_REG_BASE + 0x24) = reg24_value;
                LOOPDELAY_10USEC(1); // wait 10us

                DQS_RD_B3_table[DLSEL_RD_count] = Write_Read_compare(3, ddr_bus_bit);
            }
        }
    }

    // RD FALL
    for (DQSDLSEL_count = 0; DQSDLSEL_count < 2; DQSDLSEL_count++) {
        // DQ / DQS select
        reg6C_value = (reg6C_value & 0x0000000)+(DQSDLSEL_count*0x1111);
        *(volatile unsigned int *) (DDR_REG_BASE + 0x6C) = reg6C_value;
        if (DQSDLSEL_count == 0) {
            // byte 0
            for (DLSEL_RD_FALL_count = 0; DLSEL_RD_FALL_count < 8; DLSEL_RD_FALL_count++) {
                reg130_value = (reg130_value & 0x0000000)+DLSEL_RD_FALL_count;
                *(volatile unsigned int *) (DDR_REG_BASE + 0x130) = reg130_value;
                LOOPDELAY_10USEC(1); // wait 10us

                DQ_RD_FALL_B0_table[DLSEL_RD_FALL_count] = Write_Read_compare(0, ddr_bus_bit);
            }
            // byte 1
            for (DLSEL_RD_FALL_count = 0; DLSEL_RD_FALL_count < 8; DLSEL_RD_FALL_count++) {
                reg130_value = (reg130_value & 0x0000000)+(DLSEL_RD_FALL_count<<8);
                *(volatile unsigned int *) (DDR_REG_BASE + 0x130) = reg130_value;
                LOOPDELAY_10USEC(1); // wait 10us

                DQ_RD_FALL_B1_table[DLSEL_RD_FALL_count] = Write_Read_compare(1, ddr_bus_bit);
            }

            // byte 2
            for (DLSEL_RD_FALL_count = 0; DLSEL_RD_FALL_count < 8; DLSEL_RD_FALL_count++) {
                reg130_value = (reg130_value & 0x0000000)+(DLSEL_RD_FALL_count<<16);
                *(volatile unsigned int *) (DDR_REG_BASE + 0x130) = reg130_value;
                LOOPDELAY_10USEC(1); // wait 10us

                DQ_RD_FALL_B2_table[DLSEL_RD_FALL_count] = Write_Read_compare(2, ddr_bus_bit);
            }

            // byte 3
            for (DLSEL_RD_FALL_count = 0; DLSEL_RD_FALL_count < 8; DLSEL_RD_FALL_count++) {
                reg130_value = (reg130_value & 0x0000000)+(DLSEL_RD_FALL_count<<24);
                *(volatile unsigned int *) (DDR_REG_BASE + 0x130) = reg130_value;
                LOOPDELAY_10USEC(1); // wait 10us

                DQ_RD_FALL_B3_table[DLSEL_RD_FALL_count] = Write_Read_compare(3, ddr_bus_bit);
            }

        }
        if (DQSDLSEL_count == 1) {
            // byte 0
            for (DLSEL_RD_FALL_count = 0; DLSEL_RD_FALL_count < 8; DLSEL_RD_FALL_count++) {
                reg130_value = (reg130_value & 0x0000000)+DLSEL_RD_FALL_count;
                *(volatile unsigned int *) (DDR_REG_BASE + 0x130) = reg130_value;
                LOOPDELAY_10USEC(1); // wait 10us

                DQS_RD_FALL_B0_table[DLSEL_RD_FALL_count] = Write_Read_compare(0, ddr_bus_bit);
            }
            // byte 1
            for (DLSEL_RD_FALL_count = 0; DLSEL_RD_FALL_count < 8; DLSEL_RD_FALL_count++) {
                reg130_value = (reg130_value & 0x0000000)+(DLSEL_RD_FALL_count<<8);
                *(volatile unsigned int *) (DDR_REG_BASE + 0x130) = reg130_value;
                LOOPDELAY_10USEC(1); // wait 10us

                DQS_RD_FALL_B1_table[DLSEL_RD_FALL_count] = Write_Read_compare(1, ddr_bus_bit);
            }

            // byte 2
            for (DLSEL_RD_FALL_count = 0; DLSEL_RD_FALL_count < 8; DLSEL_RD_FALL_count++) {
                reg130_value = (reg130_value & 0x0000000)+(DLSEL_RD_FALL_count<<16);
                *(volatile unsigned int *) (DDR_REG_BASE + 0x130) = reg130_value;
                LOOPDELAY_10USEC(1); // wait 10us

                DQS_RD_FALL_B2_table[DLSEL_RD_FALL_count] = Write_Read_compare(2, ddr_bus_bit);
            }

            // byte 3
            for (DLSEL_RD_FALL_count = 0; DLSEL_RD_FALL_count < 8; DLSEL_RD_FALL_count++) {
                reg130_value = (reg130_value & 0x0000000)+(DLSEL_RD_FALL_count<<24);
                *(volatile unsigned int *) (DDR_REG_BASE + 0x130) = reg130_value;
                LOOPDELAY_10USEC(1); // wait 10us

                DQS_RD_FALL_B3_table[DLSEL_RD_FALL_count] = Write_Read_compare(3, ddr_bus_bit);
            }

        }

    }

    // select middle value of passed read DQ/DQS value
    DQ_RD_B0_result = find_center(DQ_RD_B0_table, 8);
    DQ_RD_B1_result = find_center(DQ_RD_B1_table, 8);
    DQ_RD_B2_result = find_center(DQ_RD_B2_table, 8);
    DQ_RD_B3_result = find_center(DQ_RD_B3_table, 8);
    DQ_RD_FALL_B0_result = find_center(DQ_RD_FALL_B0_table, 8);
    DQ_RD_FALL_B1_result = find_center(DQ_RD_FALL_B1_table, 8);
    DQ_RD_FALL_B2_result = find_center(DQ_RD_FALL_B2_table, 8);
    DQ_RD_FALL_B3_result = find_center(DQ_RD_FALL_B3_table, 8);

    DQS_RD_B0_result = find_center(DQS_RD_B0_table, 8);
    DQS_RD_B1_result = find_center(DQS_RD_B1_table, 8);
    DQS_RD_B2_result = find_center(DQS_RD_B2_table, 8);
    DQS_RD_B3_result = find_center(DQS_RD_B3_table, 8);
    DQS_RD_FALL_B0_result = find_center(DQS_RD_FALL_B0_table, 8);
    DQS_RD_FALL_B1_result = find_center(DQS_RD_FALL_B1_table, 8);
    DQS_RD_FALL_B2_result = find_center(DQS_RD_FALL_B2_table, 8);
    DQS_RD_FALL_B3_result = find_center(DQS_RD_FALL_B3_table, 8);


    *(volatile unsigned int *) (DDR_REG_BASE + 0x24) = 
        (DQ_RD_B3_result << 24)
        | (DQ_RD_B2_result << 16)
        | (DQ_RD_B1_result << 8) 
        | (DQ_RD_B0_result);

    *(volatile unsigned int *) (DDR_REG_BASE + 0x6C) = reg6C_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x130) = 
        (DQ_RD_FALL_B3_result << 24)
        | (DQ_RD_FALL_B2_result << 16) 
        | (DQ_RD_FALL_B1_result << 8)  
        | (DQ_RD_FALL_B0_result);


    *(volatile unsigned int *) (DDR_REG_BASE + 0x24) = 
        (DQS_RD_B3_result << 24)
        | (DQS_RD_B2_result << 16) 
        | (DQS_RD_B1_result << 8)  
        | (DQS_RD_B0_result);

    *(volatile unsigned int *) (DDR_REG_BASE + 0x6C) = reg6C_value;
    *(volatile unsigned int *) (DDR_REG_BASE + 0x130) = 
        (DQS_RD_FALL_B3_result << 24)
        | (DQS_RD_FALL_B2_result << 16) 
        | (DQS_RD_FALL_B1_result << 8)  
        | (DQS_RD_FALL_B0_result);


    //unsigned int final_center = 0;
    //unsigned int cur_pass   = 0;
    //unsigned int cont_pass = 1;
    //unsigned int i;

    //// DQ_RD_Bx_result
    //final_center = 0;
    //cur_pass = 0;
    //cont_pass = 1;
    //for (i=0 ; i < 8; i++) {
    //    if (DQ_RD_B0_table[i] == 8) {
    //        if (cont_pass) final_center = cur_pass;
    //        cur_pass = i;
    //        cont_pass = 1;
    //    }
    //    else {
    //        cont_pass = 0;
    //    }
    //}
    ////DQ_RD_B0_result = final_center;
    //DQ_RD_B0_result = find_center(DQ_RD_B0_table, 8);;

    //final_center = 0;
    //cur_pass = 0;
    //cont_pass = 1;
    //for (i=0 ; i < 8; i++) {
    //    if (DQ_RD_B1_table[i] == 8) {
    //        if (cont_pass) final_center = cur_pass;
    //        cur_pass = i;
    //        cont_pass = 1;
    //    }
    //    else {
    //        cont_pass = 0;
    //    }
    //}
    //DQ_RD_B1_result = final_center;

    //final_center = 0;
    //cur_pass = 0;
    //cont_pass = 1;
    //for (i=0 ; i < 8; i++) {
    //    if (DQ_RD_B2_table[i] == 8) {
    //        if (cont_pass) final_center = cur_pass;
    //        cur_pass = i;
    //        cont_pass = 1;
    //    }
    //    else {
    //        cont_pass = 0;
    //    }
    //}
    //DQ_RD_B2_result = final_center;

    //final_center = 0;
    //cur_pass = 0;
    //cont_pass = 1;
    //for (i=0 ; i < 8; i++) {
    //    if (DQ_RD_B3_table[i] == 8) {
    //        if (cont_pass) final_center = cur_pass;
    //        cur_pass = i;
    //        cont_pass = 1;
    //    }
    //    else {
    //        cont_pass = 0;
    //    }
    //}
    //DQ_RD_B3_result = final_center;

    //// DQS_RD_Bx_result
    //final_center = 0;
    //cur_pass = 0;
    //cont_pass = 1;
    //for (i=0 ; i < 8; i++) {
    //    if (DQS_RD_B0_table[i] == 8) {
    //        if (cont_pass) final_center = cur_pass;
    //        cur_pass = i;
    //        cont_pass = 1;
    //    }
    //    else {
    //        cont_pass = 0;
    //    }
    //}
    //DQS_RD_B0_result = final_center;

    //final_center = 0;
    //cur_pass = 0;
    //cont_pass = 1;
    //for (i=0 ; i < 8; i++) {
    //    if (DQS_RD_B1_table[i] == 8) {
    //        if (cont_pass) final_center = cur_pass;
    //        cur_pass = i;
    //        cont_pass = 1;
    //    }
    //    else {
    //        cont_pass = 0;
    //    }
    //}
    //DQS_RD_B1_result = final_center;

    //final_center = 0;
    //cur_pass = 0;
    //cont_pass = 1;
    //for (i=0 ; i < 8; i++) {
    //    if (DQS_RD_B2_table[i] == 8) {
    //        if (cont_pass) final_center = cur_pass;
    //        cur_pass = i;
    //        cont_pass = 1;
    //    }
    //    else {
    //        cont_pass = 0;
    //    }
    //}
    //DQS_RD_B2_result = final_center;

    //final_center = 0;
    //cur_pass = 0;
    //cont_pass = 1;
    //for (i=0 ; i < 8; i++) {
    //    if (DQS_RD_B3_table[i] == 8) {
    //        if (cont_pass) final_center = cur_pass;
    //        cur_pass = i;
    //        cont_pass = 1;
    //    }
    //    else {
    //        cont_pass = 0;
    //    }
    //}
    //DQS_RD_B3_result = final_center;


    //// DQ_RD_FALL_Bx_result
    //final_center = 0;
    //cur_pass = 0;
    //cont_pass = 1;
    //for (i=0 ; i < 8; i++) {
    //    if (DQ_RD_FALL_B0_table[i] == 8) {
    //        if (cont_pass) final_center = cur_pass;
    //        cur_pass = i;
    //        cont_pass = 1;
    //    }
    //    else {
    //        cont_pass = 0;
    //    }
    //}
    //DQ_RD_FALL_B0_result = final_center;

    //final_center = 0;
    //cur_pass = 0;
    //cont_pass = 1;
    //for (i=0 ; i < 8; i++) {
    //    if (DQ_RD_FALL_B1_table[i] == 8) {
    //        if (cont_pass) final_center = cur_pass;
    //        cur_pass = i;
    //        cont_pass = 1;
    //    }
    //    else {
    //        cont_pass = 0;
    //    }
    //}
    //DQ_RD_FALL_B1_result = final_center;

    //final_center = 0;
    //cur_pass = 0;
    //cont_pass = 1;
    //for (i=0 ; i < 8; i++) {
    //    if (DQ_RD_FALL_B2_table[i] == 8) {
    //        if (cont_pass) final_center = cur_pass;
    //        cur_pass = i;
    //        cont_pass = 1;
    //    }
    //    else {
    //        cont_pass = 0;
    //    }
    //}
    //DQ_RD_FALL_B2_result = final_center;

    //final_center = 0;
    //cur_pass = 0;
    //cont_pass = 1;
    //for (i=0 ; i < 8; i++) {
    //    if (DQ_RD_FALL_B3_table[i] == 8) {
    //        if (cont_pass) final_center = cur_pass;
    //        cur_pass = i;
    //        cont_pass = 1;
    //    }
    //    else {
    //        cont_pass = 0;
    //    }
    //}
    //DQ_RD_FALL_B3_result = final_center;

    //// DQS_RD_FALL_Bx_result
    //final_center = 0;
    //cur_pass = 0;
    //cont_pass = 1;
    //for (i=0 ; i < 8; i++) {
    //    if (DQS_RD_FALL_B0_table[i] == 8) {
    //        if (cont_pass) final_center = cur_pass;
    //        cur_pass = i;
    //        cont_pass = 1;
    //    }
    //    else {
    //        cont_pass = 0;
    //    }
    //}
    //DQS_RD_FALL_B0_result = final_center;

    //final_center = 0;
    //cur_pass = 0;
    //cont_pass = 1;
    //for (i=0 ; i < 8; i++) {
    //    if (DQS_RD_FALL_B1_table[i] == 8) {
    //        if (cont_pass) final_center = cur_pass;
    //        cur_pass = i;
    //        cont_pass = 1;
    //    }
    //    else {
    //        cont_pass = 0;
    //    }
    //}
    //DQS_RD_FALL_B1_result = final_center;

    //final_center = 0;
    //cur_pass = 0;
    //cont_pass = 1;
    //for (i=0 ; i < 8; i++) {
    //    if (DQS_RD_FALL_B2_table[i] == 8) {
    //        if (cont_pass) final_center = cur_pass;
    //        cur_pass = i;
    //        cont_pass = 1;
    //    }
    //    else {
    //        cont_pass = 0;
    //    }
    //}
    //DQS_RD_FALL_B2_result = final_center;

    //final_center = 0;
    //cur_pass = 0;
    //cont_pass = 1;
    //for (i=0 ; i < 8; i++) {
    //    if (DQS_RD_FALL_B3_table[i] == 8) {
    //        if (cont_pass) final_center = cur_pass;
    //        cur_pass = i;
    //        cont_pass = 1;
    //    }
    //    else {
    //        cont_pass = 0;
    //    }
    //}
    //DQS_RD_FALL_B3_result = final_center;


}





static void do_calibration_DQ_DQS_WRITE()
{
    volatile unsigned int reg6C_value; // DQ/DQS sel(wr,rd) 1:DQS, 0:DQ
    volatile unsigned int reg78_value; // DQ/DQS value for write

    unsigned int DQSDLSEL_count;
    unsigned int DLSEL_WR_count;

    unsigned int DQ_WR_B0_table[8];
    unsigned int DQ_WR_B1_table[8];
    unsigned int DQ_WR_B2_table[8];
    unsigned int DQ_WR_B3_table[8];
    unsigned int DQS_WR_B0_table[8];
    unsigned int DQS_WR_B1_table[8];
    unsigned int DQS_WR_B2_table[8];
    unsigned int DQS_WR_B3_table[8];

    unsigned int DQ_WR_B0_result;
    unsigned int DQ_WR_B1_result;
    unsigned int DQ_WR_B2_result;
    unsigned int DQ_WR_B3_result;
    unsigned int DQS_WR_B0_result;
    unsigned int DQS_WR_B1_result;
    unsigned int DQS_WR_B2_result;
    unsigned int DQS_WR_B3_result;


    //:step 3 write eye traning
    for (DQSDLSEL_count = 0; DQSDLSEL_count < 2; DQSDLSEL_count++) {
        // DQ / DQS select
        reg6C_value = (reg6C_value & 0x0000000)+(DQSDLSEL_count*0x2222);
        *(volatile unsigned int *) (DDR_REG_BASE + 0x6C) = reg6C_value;
        if (DQSDLSEL_count == 0) {
            // byte 0
            for (DLSEL_WR_count = 0; DLSEL_WR_count < 8; DLSEL_WR_count++) {
                reg78_value = (reg78_value & 0x0000000)+DLSEL_WR_count;
                *(volatile unsigned int *) (DDR_REG_BASE + 0x78) = reg78_value;
                LOOPDELAY_10USEC(1); // wait 10us

                DQ_WR_B0_table[DLSEL_WR_count] = Write_Read_compare(0, ddr_bus_bit);
            }
            // byte 1
            for (DLSEL_WR_count = 0; DLSEL_WR_count < 8; DLSEL_WR_count++) {
                reg78_value = (reg78_value & 0x0000000)+(DLSEL_WR_count<<8);
                *(volatile unsigned int *) (DDR_REG_BASE + 0x78) = reg78_value;
                LOOPDELAY_10USEC(1); // wait 10us

                DQ_WR_B1_table[DLSEL_WR_count] = Write_Read_compare(1, ddr_bus_bit);
            }

            // byte 2
            for (DLSEL_WR_count = 0; DLSEL_WR_count < 8; DLSEL_WR_count++) {
                reg78_value = (reg78_value & 0x0000000)+(DLSEL_WR_count<<16);
                *(volatile unsigned int *) (DDR_REG_BASE + 0x78) = reg78_value;
                LOOPDELAY_10USEC(1); // wait 10us

                DQ_WR_B2_table[DLSEL_WR_count] = Write_Read_compare(2, ddr_bus_bit);
            }

            // byte 3
            for (DLSEL_WR_count = 0; DLSEL_WR_count < 8; DLSEL_WR_count++) {
                reg78_value = (reg78_value & 0x0000000)+(DLSEL_WR_count<<24);
                *(volatile unsigned int *) (DDR_REG_BASE + 0x78) = reg78_value;
                LOOPDELAY_10USEC(1); // wait 10us

                DQ_WR_B3_table[DLSEL_WR_count] = Write_Read_compare(3, ddr_bus_bit);
            }

        }
        if (DQSDLSEL_count == 1) {
            // byte 0
            for (DLSEL_WR_count = 0; DLSEL_WR_count < 8; DLSEL_WR_count++) {
                reg78_value = (reg78_value & 0x0000000)+DLSEL_WR_count;
                *(volatile unsigned int *) (DDR_REG_BASE + 0x78) = reg78_value;
                LOOPDELAY_10USEC(1); // wait 10us

                DQS_WR_B0_table[DLSEL_WR_count] = Write_Read_compare(0, ddr_bus_bit);
            }
            // byte 1
            for (DLSEL_WR_count = 0; DLSEL_WR_count < 8; DLSEL_WR_count++) {
                reg78_value = (reg78_value & 0x0000000)+(DLSEL_WR_count<<8);
                *(volatile unsigned int *) (DDR_REG_BASE + 0x78) = reg78_value;
                LOOPDELAY_10USEC(1); // wait 10us

                DQS_WR_B1_table[DLSEL_WR_count] = Write_Read_compare(1, ddr_bus_bit);
            }

            // byte 2
            for (DLSEL_WR_count = 0; DLSEL_WR_count < 8; DLSEL_WR_count++) {
                reg78_value = (reg78_value & 0x0000000)+(DLSEL_WR_count<<16);
                *(volatile unsigned int *) (DDR_REG_BASE + 0x78) = reg78_value;
                LOOPDELAY_10USEC(1); // wait 10us

                DQS_WR_B2_table[DLSEL_WR_count] = Write_Read_compare(2, ddr_bus_bit);
            }

            // byte 3
            for (DLSEL_WR_count = 0; DLSEL_WR_count < 8; DLSEL_WR_count++) {
                reg78_value = (reg78_value & 0x0000000)+(DLSEL_WR_count<<24);
                *(volatile unsigned int *) (DDR_REG_BASE + 0x78) = reg78_value;
                LOOPDELAY_10USEC(1); // wait 10us

                DQS_WR_B3_table[DLSEL_WR_count] = Write_Read_compare(3, ddr_bus_bit);
            }

        }

    }

    // select middle value of passed write DQ/DQS value
    DQ_WR_B0_result = find_center(DQ_WR_B0_table, 8);
    DQ_WR_B1_result = find_center(DQ_WR_B1_table, 8);
    DQ_WR_B2_result = find_center(DQ_WR_B2_table, 8);
    DQ_WR_B3_result = find_center(DQ_WR_B3_table, 8);

    DQS_WR_B0_result = find_center(DQS_WR_B0_table, 8);
    DQS_WR_B1_result = find_center(DQS_WR_B1_table, 8);
    DQS_WR_B2_result = find_center(DQS_WR_B2_table, 8);
    DQS_WR_B3_result = find_center(DQS_WR_B3_table, 8);


    *(volatile unsigned int *) (DDR_REG_BASE + 0x78) = 
        (DQ_WR_B3_result << 24)
        | (DQ_WR_B2_result << 16) 
        | (DQ_WR_B1_result << 8)  
        | (DQ_WR_B0_result);

    *(volatile unsigned int *) (DDR_REG_BASE + 0x6C) = reg6C_value;

    *(volatile unsigned int *) (DDR_REG_BASE + 0x78) = 
        (DQS_WR_B3_result << 24)
        | (DQS_WR_B2_result << 16) 
        | (DQS_WR_B1_result << 8)  
        | (DQS_WR_B0_result);


}
#endif	//DDR_CALIB_TESET

int ddr_calibration(int opt)
{

    // memory initialize
    *(volatile unsigned int *) (DDR_MEM_BASE+0x00000) = 0x0;
    *(volatile unsigned int *) (DDR_MEM_BASE+0x00004) = 0x0;
    *(volatile unsigned int *) (DDR_MEM_BASE+0x00008) = 0x0;
    *(volatile unsigned int *) (DDR_MEM_BASE+0x0000C) = 0x0;
    *(volatile unsigned int *) (DDR_MEM_BASE+0x00010) = 0x0;
    *(volatile unsigned int *) (DDR_MEM_BASE+0x00014) = 0x0;
    *(volatile unsigned int *) (DDR_MEM_BASE+0x00018) = 0x0;
    *(volatile unsigned int *) (DDR_MEM_BASE+0x0001C) = 0x0;


#if DDR_CALIB_TESET
    do_calib_test();
#else
    if(opt & DDR_CAL_WLEVEL)
    	do_calibration_Leveling(0); // write leveling
    if(opt & DDR_CAL_RLEVEL)
    	do_calibration_Leveling(1); // read leveling
    if(opt & DDR_CAL_GDS)
    	do_calibration_GDS(); // GDS
    if(opt & DDR_CAL_RDQDQS)
    	do_calibration_DQ_DQS_READ(); // DQ/DQS
    if(opt & DDR_CAL_WDQDQS)
    	do_calibration_DQ_DQS_WRITE(); // DQ/DQS
#endif

    // finish condition
    finish();
    return 0;
}

