/*
 */

#include <stdio.h>
#include <nds32_intrinsic.h>

#include "osal.h"
#include "sub_intr.h"
#include "type.h"
#include "utils.h"
#include "debug.h"
#include "pi5008.h"
#include "vin.h"
#include "adc.h"
#include "diag.h"
#include "proc.h"

static ADC_REG_T *gpADC = ADC_CTRL_REG;

//////////////////////////////////////////////////////////////////////////////
//PROC irq
PP_RESULT_E PPAPI_DIAG_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv);
struct proc_irq_struct stDIAG_PROC_irqs[] = {
	    { .fn = PPAPI_DIAG_PROC_IRQ,	.irqNum = 200/*IRQ_1_VECTOR*/+IRQ_CRC_QUAD,    .next = (void*)0, },
};
//PROC device
PP_RESULT_E PPAPI_DIAG_PROC_DEVICE(PP_S32 argc, const PP_CHAR **argv);
struct proc_device_struct stDIAG_PROC_devices[] = {
	    { .pName = "DIAG",  .fn = PPAPI_DIAG_PROC_DEVICE,    .next = (void*)0, },
};

// Diagnosis0,1 table value
typedef struct
{
        PP_U32 diag0;
        PP_U32 diag1;
}_DIAG_VALUE;

const static _DIAG_VALUE tblDiag[max_vid_resol] = {
	/*vres_720x480i60*/ {0x05, 0x14}, 
	/*vres_720x576i50*/ {0x05, 0x14}, 
	/*vres_960x480i60*/ {0x05, 0x14}, 
	/*vres_960x576i50*/ {0x05, 0x14},
	/*vres_720x480p60*/ {0x05, 0x14}, 
	/*vres_720x576p50*/ {0x05, 0x14}, 
	/*vres_960x480p60*/ {0x05, 0x14}, 
	/*vres_960x576p50*/ {0x05, 0x14},
	/*vres_1280x720p60*/ {0x05, 0x14}, 
	/*vres_1280x720p50*/ {0x05, 0x14}, 
	/*vres_1280x720p30*/ {0x05, 0x14}, 
	/*vres_1280x720p25*/ {0x05, 0x14}, 
	/*vres_1280x960p30*/ {0x05, 0x14}, 
	/*vres_1280x960p25*/ {0x05, 0x14}, 
	/*vres_1920x1080p30*/ {0x05, 0x14}, 
	/*vres_1920x1080p25*/ {0x05, 0x14},
	/*vres_800x480p60*/  {0x05, 0x14}, //don't suppot
	/*vres_800x480p50*/  {0x05, 0x14}, //don't suppot
	/*vres_1024x600p60*/ {0x05, 0x14}, //don't suppot
	/*vres_1024x600p50*/ {0x05, 0x14}, //don't suppot
};
//////////////////////////////////////////////////////////////////////////////
/* INTR1 */
const PP_CHAR *strDiagName[IRQ_GADC1_DIAG+1] = {
	"CRC_QUAD",
	"CRC_CH0",
	"CRC_CH1",
	"CRC_CH2",
	"CRC_CH3",
	"CRC_CH4",
	"GEN_VREFLOSS",
	"GEN_SYNC",
	"LOSS_FMT_CH0",
	"LOSS_FMT_CH1",
	"LOSS_FMT_CH2",
	"LOSS_FMT_CH3",
	"LOSS_FMT_CH4",
	"LOSS_SYNC_CH0",
	"LOSS_SYNC_CH1",
	"LOSS_SYNC_CH2",
	"LOSS_SYNC_CH3",
	"LOSS_SYNC_CH4",
	"FREEZE_DU_CH0",
	"FREEZE_DU_CH1",
	"FREEZE_DU_CH2",
	"FREEZE_DU_CH3",
	"GADC0_DIAG",
	"GADC1_DIAG",
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern PP_U32 gu32DiagIntcCnt[IRQ_GADC1_DIAG+1];

PP_RESULT_E PPAPI_DIAG_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv)
{
	PP_S32 i, irqNum;
	PP_CHAR diagName[20];

	//	PRINT_PROC_IRQ(NAME, NUM, CNT)
	for(i = IRQ_CRC_QUAD; i <= IRQ_GADC1_DIAG; i++)
	{
		irqNum = 200/*IRQ_1_VECTOR*/+i;

		sprintf(diagName, "DIAG_%s", strDiagName[i]);
		PRINT_PROC_IRQ(diagName, irqNum, gu32DiagIntcCnt[i]);
	}

	return(eSUCCESS);
}

PP_RESULT_E PPAPI_DIAG_PROC_DEVICE(PP_S32 argc, const PP_CHAR **argv)
{
	PP_S32 i, regOffset;

	INTR_REG_T	*pSUB1_INTC_REG = SUB_INTR1_CTRL_REG;
	PP_U32 polarity;

	if( (argc) && (strcmp(argv[0], stDIAG_PROC_devices[0].pName) && strcmp(argv[0], "ALL")) )
	{
		return(eERROR_FAILURE);
	}
	printf("\n%s Device Info -------------\n", stDIAG_PROC_devices[0].pName);

	printf("### DIAG info(Driver Version : v%s) ### \n", _VER_DIAG);

	//printf("0x%08x\n", pSUB1_INTC_REG->polarity);    // 1:falling/low, 0:rising/high
	polarity  = pSUB1_INTC_REG->polarity;    // 1:falling/low, 0:rising/high

	printf("crc_freeze: quad     ch0      ch1      ch2      ch3      ch4\n");
	//printf("    crc_freeze(quad):");
	printf("            ");
	if(polarity & (1<<IRQ_CRC_QUAD)) { printf("%8s ", "freeze"); }
	else { printf("%8s ", "unfreeze"); }

	//printf("    crc_freeze(ch0):");
	if(polarity & (1<<IRQ_CRC_CH0)) { printf("%8s ", "freeze"); }
	else { printf("%8s ", "unfreeze"); }

	//printf("    crc_freeze(ch1):");
	if(polarity & (1<<IRQ_CRC_CH1)) { printf("%8s ", "freeze"); }
	else { printf("%8s ", "unfreeze"); }

	//printf("    crc_freeze(ch2):");
	if(polarity & (1<<IRQ_CRC_CH2)) { printf("%8s ", "freeze"); }
	else { printf("%8s ", "unfreeze"); }

	//printf("    crc_freeze(ch3):");
	if(polarity & (1<<IRQ_CRC_CH3)) { printf("%8s ", "freeze"); }
	else { printf("%8s ", "unfreeze"); }

	//printf("    crc_freeze(ch4):");
	if(polarity & (1<<IRQ_CRC_CH4)) { printf("%8s", "freeze"); }
	else { printf("%8s", "unfreeze"); }
	printf("\n");

	printf("gen vrefloss:");
	if(polarity & (1<<IRQ_GEN_VREFLOSS))
	{
		printf("loss reference\n");
	}
	else
	{
		printf("sync reference\n");
	}

	printf("gen sync:");
	if(polarity & (1<<IRQ_GEN_SYNC))
	{
		printf("lock\n");
	}
	else
	{
		printf("unlock\n");
	}

	printf("loss fmt: status fmtlossdet1 vlossdet1 sysclock(va/ha/fs/hs) sync_hperiod fmtlossdet2 vlossdet2 syncsize(fs/va/ha)\n");
	for(i = 0, regOffset = 0; i < 5; i++, regOffset+=0xC)
	{
		printf("     ch%d  ", i);
		if(polarity & (1<<(IRQ_LOSS_FMT_CH0+i))) { printf("%6s ", "loss"); }
		else { printf("%6s ", "get"); }

		{
			PP_U32 reg;
			reg = GetRegValue(0xF0F00400+(regOffset));		// sync_bt_sel -> DU e-sync
			//printf("        0xF0F00400:0x%08x\n", reg);
			printf("%11d ", (reg>>29)&1); //fmtloassdet1
			printf("%9d ", (reg>>28)&1); //vlossdet1
			printf("         %2d/%2d/%2d/%2d  ", (reg>>27)&1, (reg>>26)&1, (reg>>25)&1, (reg>>24)&1); //sysclock(v/h/f/h)
			printf("%12d ", reg&0x3FFF); //sync_hperiod
			reg = GetRegValue(0xF0F00404+(regOffset));		// sync_bt_sel -> DU e-sync
			//printf("        0xF0F00404:0x%08x\n", reg);
			printf("%11d ", (reg>>29)&1); //fmtloassdet2
			printf("%9d ", (reg>>28)&1); //vlossdet2
			printf("%07x/", reg&0xFFFFFF); //synsize(f)
			reg = GetRegValue(0xF0F00408+(regOffset));		// sync_bt_sel -> DU e-sync
			//printf("        0xF0F00408:0x%08x\n", reg);
			printf("%04x/", (reg>>16)&0x7FF); //synsize(v)
			printf("%04x", (reg)&0xFFF); //synsize(h)
		}
		printf("\n");
	}

	printf("loss sync:  ch0  ch1  ch2  ch3  ch4\n");
	printf("            ");
	for(i = 0; i < 5; i++)
	{
		if(polarity & (1<<(IRQ_LOSS_SYNC_CH0+i))) { printf("%4s ", "loss"); }
		else { printf("%4s ", "sync"); }
	}
	printf("\n");

	printf("freeze du: ch0      ch1      ch2      ch3\n");
	printf("           ");
	for(i = 0; i < 4; i++)
	{
		if(polarity & (1<<(IRQ_FREEZE_DU_CH0+i))) { printf("%8s ", "freeze"); }
		else { printf("%8s ", "unfreeze"); }
	}
	printf("\n");

	printf("gadc0:");
	if(polarity & (1<<IRQ_GADC0_DIAG))
	{
		printf("below threshold\n");
	}
	else
	{
		printf("normal\n");
	}

	printf("gadc1:");
	if(polarity & (1<<IRQ_GADC1_DIAG))
	{
		printf("above threshold\n");
	}
	else
	{
		printf("normal\n");
	}

	printf("Detect Set: vlossEn input syncThr SelFmt(va/ha/fs/hs) hsizeDet fsizeMode fsizeDet vaDet haDet\n");
	for(i = 0, regOffset = 0; i < 5; i++, regOffset+=0xC)
	{
		printf("       ch%d  ", i);
		{
			PP_U32 reg;
			reg = GetRegValue(0xF0F00440+(regOffset));
			//printf("        0xF0F00440:0x%08x\n", reg);
			printf("%7d ", (reg>>31)&1); //vlossEn
			printf("%5s ", (reg>>25)&1 ? "intr":"prog"); //input
			printf("%7s ", (reg>>24)&1 ? "12.5%":"6.25%"); //syncThr
			printf("       %2d/%2d/%2d/%2d  ", (reg>>23)&1, (reg>>22)&1, (reg>>21)&1, (reg>>20)&1); //selFmt(va/ha/fs/hs)
			printf("%8d ", reg&0x3FFF); //hsizeDet
			reg = GetRegValue(0xF0F00444+(regOffset));
			//printf("        0xF0F00444:0x%08x\n", reg);
			printf("%9s ", (reg>>24)&1 ? "HxVactiv":"HxVtotal"); //fsizeMode
			printf("%08x ", reg&0xFFFFFF); //fsizeDet
			reg = GetRegValue(0xF0F00448+(regOffset));
			//printf("        0xF0F00448:0x%08x\n", reg);
			printf("%5d ", (reg>>16)&0x7FF); //vaDet
			printf("%5d", (reg)&0xFFF); //haDet
		}
		printf("\n");
	}

	printf("------------------------------\n");

	return(eSUCCESS);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PPDRV_DIAG_GetDisplayFreezeStatus(unsigned char pRetStatus[])
{
	INTR_REG_T	*pSUB1_INTC_REG = SUB_INTR1_CTRL_REG;
	PP_U32 polarity;
	int i;

	//printf("0x%08x\n", pSUB1_INTC_REG->polarity);    // 1:falling/low, 0:rising/high
	polarity  = pSUB1_INTC_REG->polarity;    // 1:falling/low, 0:rising/high


	for(i = 0; i < 4; i++)
	{
		if(polarity & (1<<(IRQ_FREEZE_DU_CH0+i))) { pRetStatus[i] = 1; /*freeze*/ }
		else { pRetStatus[i] = 0; /*unfreeze*/ }
    }
}

void PPDRV_DIAG_GetCamInvalidStatus(unsigned char pRetStatus[])
{
	INTR_REG_T	*pSUB1_INTC_REG = SUB_INTR1_CTRL_REG;
	PP_U32 polarity;
	int i;

	//printf("0x%08x\n", pSUB1_INTC_REG->polarity);    // 1:falling/low, 0:rising/high
	polarity  = pSUB1_INTC_REG->polarity;    // 1:falling/low, 0:rising/high

	for(i = 0; i < 4; i++)
	{
		if(polarity & (1<<(IRQ_LOSS_FMT_CH0+i))) { pRetStatus[i] = 1; /*invalid*/ }
		else { pRetStatus[i] = 0; /*normal*/ }
    }
}


void PPDRV_DIAG_Set(void)
{
	PP_U32 diag0;
	PP_U32 diag1;

	diag0 = tblDiag[BD_IN_RESOLUTION].diag0;
	diag1 = tblDiag[BD_IN_RESOLUTION].diag1;

	//gpADC->ADCC_DIAG0 = 0x100 | (diag0&0xff); //en | value
	//gpADC->ADCC_DIAG1 = 0x100 | (diag1&0xff); //en | value
	gpADC->ADCC_DIAG0 = (diag0&0xff);
	gpADC->ADCC_DIAG1 = (diag1&0xff);
}

void PPDRV_DIAG_Initialize(void)
{
    PPDRV_DIAG_Set();

	//register proc infomation.
	SYS_PROC_addIrq(stDIAG_PROC_irqs);
	SYS_PROC_addDevice(stDIAG_PROC_devices);

}

