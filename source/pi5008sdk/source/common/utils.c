/*
 * utils.c
 *
 *  Created on: May 25, 2017
 *      Author: junghh
 */

#include <stdio.h>
#include <nds32_intrinsic.h>

#include "type.h"
#include "error.h"
#include "osal.h"
#include "api_FAT_FTL.h"
#include "api_calibration.h"
////////////////////////////////////////////////////////////////////////////////////////////
// Performance Counter
#define NDS32_PERFCTL_EN0   (1UL << 0)
#define NDS32_PERFCTL_EN1   (1UL << 1)
#define NDS32_PERFCTL_EN2   (1UL << 2)
#define NDS32_PERFCTL_IE0   (1UL << 3)
#define NDS32_PERFCTL_IE1   (1UL << 4)
#define NDS32_PERFCTL_IE2   (1UL << 5)
#define NDS32_PERFCTL_OVF0  (1UL << 6)
#define NDS32_PERFCTL_OVF1  (1UL << 7)
#define NDS32_PERFCTL_OVF2  (1UL << 8)
#define NDS32_PERFCTL_KS0   (1UL << 9)
#define NDS32_PERFCTL_KS1   (1UL << 10)
#define NDS32_PERFCTL_KS2   (1UL << 11)
#define NDS32_PERFCTL_KU0   (1UL << 12)
#define NDS32_PERFCTL_KU1   (1UL << 13)
#define NDS32_PERFCTL_KU2   (1UL << 14)
#define NDS32_PERFCTL_SEL0_C (0UL << 15)
#define NDS32_PERFCTL_SEL0_I (1UL << 15)
#define NDS32_PERFCTL_SEL1  (0x3f << 16)
#define NDS32_PERFCTL_SEL2  (0x3f << 22)

#define MAX_PF_SAMPLES			100
#define MAX_PF_SPAN				100
#define MAX_PF_SLOT				4
#define PF_CLOCK				5400 // per 100us
static struct
{
	uint32 c0_100us[MAX_PF_SPAN];
	uint32 c1_K[MAX_PF_SPAN];
	uint32 c2_K[MAX_PF_SPAN];
	uint32 c0_sum_100us;
	uint32 c1_sum_K;
	uint32 c2_sum_K;
	uint32 c0_min_100us;
	uint32 c1_min_K;
	uint32 c2_min_K;
	uint32 c0_max_100us;
	uint32 c1_max_K;
	uint32 c2_max_K;
	uint32 cur;
	uint32 count;
} pf_counts[MAX_PF_SLOT];

static unsigned int counter0, counter1, counter2;

void reset_pf_counts (uint32 index)
{
	unsigned int i;
	pf_counts[index].cur = 0;
	pf_counts[index].count = 0;
	pf_counts[index].c0_sum_100us = 0;
	pf_counts[index].c1_sum_K = 0;
	pf_counts[index].c2_sum_K = 0;
	pf_counts[index].c0_min_100us = 0xFFFFFFFF;
	pf_counts[index].c1_min_K = 0xFFFFFFFF;
	pf_counts[index].c2_min_K = 0xFFFFFFFF;
	pf_counts[index].c0_max_100us = 0;
	pf_counts[index].c1_max_K = 0;
	pf_counts[index].c2_max_K = 0;
	for (i = 0; i < MAX_PF_SPAN; i++)
	{
		pf_counts[index].c0_100us[i] = 0;
		pf_counts[index].c1_K[i] = 0;
		pf_counts[index].c2_K[i] = 0;
	}
}

void startPFM (uint32 index)
{
	unsigned int counter0 = 0, counter1 = 0, counter2 = 0;
	unsigned int ctrl = 0;
	//Counter0
	__nds32__mtsr_dsb(counter0, NDS32_SR_PFMC0);

	//Counter1
	__nds32__mtsr_dsb(counter1, NDS32_SR_PFMC1);

	//Counter2
	__nds32__mtsr_dsb(counter2, NDS32_SR_PFMC2);

	/* Enable performance counter0 , counter1, counter2 */
	ctrl |= NDS32_PERFCTL_EN0 | NDS32_PERFCTL_EN1 | NDS32_PERFCTL_EN2;
	/* Enable interrupt for counter0, counter1, counter2 */
	ctrl |= NDS32_PERFCTL_IE0 | NDS32_PERFCTL_IE1 | NDS32_PERFCTL_IE2;
	/* Calculate Cycles for counter0 */
	ctrl |= NDS32_PERFCTL_SEL0_C;
	/* Calculate ICache access for counter1 */
	ctrl |= (0x11 << 16);
	/* Calculate DCache access for counter2 */
	ctrl |= (0x14 << 22);

	__nds32__mtsr(ctrl, NDS32_SR_PFM_CTL);
}

void printPFM (uint32 index)
{
	//uart_puts("Reading Performance Monitor Results for Cache.\n");
	pf_counts[index].c0_sum_100us -= pf_counts[index].c0_100us[pf_counts[index].cur];
	pf_counts[index].c1_sum_K -= pf_counts[index].c1_K[pf_counts[index].cur];
	pf_counts[index].c2_sum_K -= pf_counts[index].c2_K[pf_counts[index].cur];
	pf_counts[index].c0_100us[pf_counts[index].cur] = counter0;
	pf_counts[index].c1_K[pf_counts[index].cur] = counter1;
	pf_counts[index].c2_K[pf_counts[index].cur] = counter2;
	pf_counts[index].c0_sum_100us += counter0;
	pf_counts[index].c1_sum_K += counter1;
	pf_counts[index].c2_sum_K += counter2;
	if (pf_counts[index].c0_min_100us > counter0) pf_counts[index].c0_min_100us = counter0;
	if (pf_counts[index].c0_max_100us < counter0) pf_counts[index].c0_max_100us = counter0;
	if (pf_counts[index].c1_min_K > counter1) pf_counts[index].c1_min_K = counter1;
	if (pf_counts[index].c1_max_K < counter1) pf_counts[index].c1_max_K = counter1;
	if (pf_counts[index].c2_min_K > counter2) pf_counts[index].c2_min_K = counter2;
	if (pf_counts[index].c2_max_K < counter2) pf_counts[index].c2_max_K = counter2;
	pf_counts[index].cur = (pf_counts[index].cur + 1) % MAX_PF_SPAN;

	if (++pf_counts[index].count > MAX_PF_SAMPLES)
	{
		unsigned int c0_avg, c1_avg, c2_avg;
		c0_avg = pf_counts[index].c0_sum_100us / MAX_PF_SPAN;
		c1_avg = pf_counts[index].c1_sum_K / MAX_PF_SPAN;
		c2_avg = pf_counts[index].c2_sum_K / MAX_PF_SPAN;
		printf ("\n___PF(%d)___ (%u/%u/%u)(100us) _I:(%u/%u/%u)K _D:(%u/%u/%u)K\n"
				, index
				, c0_avg, pf_counts[index].c0_min_100us, pf_counts[index].c0_max_100us
				, c1_avg, pf_counts[index].c1_min_K, pf_counts[index].c1_max_K
				, c2_avg, pf_counts[index].c2_min_K, pf_counts[index].c2_max_K);
		reset_pf_counts (index);
	}
}

void stopPFM (uint32 index)
{
	unsigned int pfm_ctl;

	counter0 = __nds32__mfsr(NDS32_SR_PFMC0)/PF_CLOCK;
	counter1 = __nds32__mfsr(NDS32_SR_PFMC1)/1000;
	counter2 = __nds32__mfsr(NDS32_SR_PFMC2)/1000;

	pfm_ctl = __nds32__mfsr(NDS32_SR_PFM_CTL);
	//Disable all counters
	pfm_ctl = pfm_ctl & 0xFFFFFFF8;
	__nds32__mtsr(pfm_ctl, NDS32_SR_PFM_CTL);
}

typedef struct ppBITMAPINFOHEADER{
	PP_U32 biSize;
	PP_U32 biWidth;
	PP_U32 biHeight;
	PP_U16 biPlanes;
	PP_U16 biBitCount;
	PP_U32 biCompression;
	PP_U32 biSizeImage;
	PP_U32 biXPelsPerMeter;
	PP_U32 biYPelsPerMeter;
	PP_U32 biClrUsed;
	PP_U32 biClrImportant;
}PP_BITMAPINFOHEADER;

PP_RESULT_E SaveBMP24FromY(const PP_CHAR *szFileName, PP_U8 *pu8ImgY, PP_U32 u32Width, PP_U32 u32Height)
{
	PP_U8 u8BitmapFileHdr[14];
	PP_BITMAPINFOHEADER stInfoHdr;
	PP_U8 *pu8Buf;
	PP_U8 *pu8Ptr;
	PP_U32 u32Size;
	PP_S32 i,j;
	PP_VOID *hFile = NULL;
	PP_RESULT_E ret = eERROR_FAILURE;

	hFile = PPAPI_FATFS_Open(szFileName, "w");

#ifdef CALIB_LIB_USE
	pu8Buf = PPAPI_Lib_Ext_Malloc(u32Width * u32Height * 3);
#else
	pu8Buf = OSAL_malloc(u32Width * u32Height * 3);
#endif
	if(pu8Buf == NULL){
		LOG_DEBUG("malloc fail\n");
		goto END_FT;
	}

	// Bitmap File Header
	memset(u8BitmapFileHdr, 0, sizeof(u8BitmapFileHdr));
	u8BitmapFileHdr[0] = 0x42;
	u8BitmapFileHdr[1] = 0x4D;
	u32Size = u32Width * u32Height * 3 + sizeof(u8BitmapFileHdr) + sizeof(stInfoHdr);
	memcpy(&u8BitmapFileHdr[2], &u32Size, 4);
	u32Size = sizeof(u8BitmapFileHdr) + sizeof(stInfoHdr);
	memcpy(&u8BitmapFileHdr[10], &u32Size, 4);


	// Bitmap Info Header
	stInfoHdr.biSize = sizeof(stInfoHdr);
	stInfoHdr.biWidth = u32Width;
	stInfoHdr.biHeight = u32Height;
	stInfoHdr.biPlanes = 1;
	stInfoHdr.biBitCount = 24;			// 24bit
	stInfoHdr.biCompression = 0;		// BI_RGB
	stInfoHdr.biSizeImage = u32Width * u32Height * 3;
	stInfoHdr.biXPelsPerMeter = 0;
	stInfoHdr.biYPelsPerMeter = 0;
	stInfoHdr.biClrUsed = 0;
	stInfoHdr.biClrImportant = 0;

	if(PPAPI_FATFS_Write((PP_VOID *)&u8BitmapFileHdr, 1, sizeof(u8BitmapFileHdr), hFile) != sizeof(u8BitmapFileHdr))goto END_FT;
	if(PPAPI_FATFS_Write((PP_VOID *)&stInfoHdr, 1, sizeof(stInfoHdr), hFile) != sizeof(stInfoHdr))goto END_FT;

	for(j=0; j<u32Height; j++){
		pu8Ptr = pu8ImgY + (u32Height-j-1) * u32Width;

		for(i=0;i<u32Width;i++){
			*(pu8Buf + i*3) = *(pu8Buf + i*3 + 1) = *(pu8Buf + i*3 + 2) = *(pu8Ptr + i);
		}

		if(PPAPI_FATFS_Write((PP_VOID *)pu8Buf, 1, u32Width*3, hFile) != u32Width*3)goto END_FT;
	}

	ret = eSUCCESS;
END_FT:
	if(hFile)PPAPI_FATFS_Close(hFile);
	
#ifdef CALIB_LIB_USE
	if(pu8Buf) PPAPI_Lib_Ext_Free(pu8Buf);
#else
	if(pu8Buf) OSAL_free(pu8Buf);	
#endif
	return ret;
}

PP_RESULT_E SaveBMP24FromYUV(const PP_CHAR *szFileName, PP_U8 *pu8ImgYUV, PP_U32 u32Width, PP_U32 u32Height)
{
	PP_U8 u8BitmapFileHdr[14];
	PP_BITMAPINFOHEADER stInfoHdr;
	PP_U8 *pu8Buf;
	PP_U8 *pu8Ptr;
	PP_U32 u32Size;
	PP_S32 i,j;
	PP_VOID *hFile = NULL;
	PP_RESULT_E ret = eERROR_FAILURE;

	PP_D64 R,G,B;
	PP_U8 *Y1,*Y2,*U,*V;

	hFile = PPAPI_FATFS_Open(szFileName, "w");

#ifdef CALIB_LIB_USE
	pu8Buf = PPAPI_Lib_Ext_Malloc(u32Width * u32Height * 3);
#else
	pu8Buf = OSAL_malloc(u32Width * u32Height * 3);
#endif
	if(pu8Buf == NULL){
		LOG_DEBUG("malloc fail\n");
		goto END_FT;
	}

	// Bitmap File Header
	memset(u8BitmapFileHdr, 0, sizeof(u8BitmapFileHdr));
	u8BitmapFileHdr[0] = 0x42;
	u8BitmapFileHdr[1] = 0x4D;
	u32Size = u32Width * u32Height * 3 + sizeof(u8BitmapFileHdr) + sizeof(stInfoHdr);
	memcpy(&u8BitmapFileHdr[2], &u32Size, 4);
	u32Size = sizeof(u8BitmapFileHdr) + sizeof(stInfoHdr);
	memcpy(&u8BitmapFileHdr[10], &u32Size, 4);


	// Bitmap Info Header
	stInfoHdr.biSize = sizeof(stInfoHdr);
	stInfoHdr.biWidth = u32Width;
	stInfoHdr.biHeight = u32Height;
	stInfoHdr.biPlanes = 1;
	stInfoHdr.biBitCount = 24;			// 24bit
	stInfoHdr.biCompression = 0;		// BI_RGB
	stInfoHdr.biSizeImage = u32Width * u32Height * 3;
	stInfoHdr.biXPelsPerMeter = 0;
	stInfoHdr.biYPelsPerMeter = 0;
	stInfoHdr.biClrUsed = 0;
	stInfoHdr.biClrImportant = 0;

	if(PPAPI_FATFS_Write((PP_VOID *)&u8BitmapFileHdr, 1, sizeof(u8BitmapFileHdr), hFile) != sizeof(u8BitmapFileHdr))goto END_FT;
	if(PPAPI_FATFS_Write((PP_VOID *)&stInfoHdr, 1, sizeof(stInfoHdr), hFile) != sizeof(stInfoHdr))goto END_FT;




	for(j=0; j<u32Height; j++){





	
		//pu8Ptr = pu8ImgYUV + (u32Height-j-1) * u32Width;

		for(i=0;i<u32Width/2;i++){

			//move pointer
			U = pu8ImgYUV + (u32Height-j-1) * u32Width * 2 + (i*4);
			Y1 = pu8ImgYUV + (u32Height-j-1) * u32Width * 2 + (i*4+1);
			V = pu8ImgYUV + (u32Height-j-1) * u32Width * 2 + (i*4+2);
			Y2 = pu8ImgYUV + (u32Height-j-1) * u32Width * 2 + (i*4+3);

			/*make first pixel*/
			//convert to RGB from UYVY
			B=1.164*(*Y1-16)+2.018*(*U-128);
			G=1.164*(*Y1-16)-0.813*(*V-128)-0.391*(*U-128);
			R=1.164*(*Y1-16)+1.596*(*V-128);

			// clamping
			if(B<0)B=0;
			if(R<0)R=0;
			if(G<0)G=0;
			if(B>255)B=255;
			if(R>255)R=255;
			if(G>255)G=255;
			
			// insert to buffer
			*(pu8Buf + (i*2)*3) = B;
			*(pu8Buf + (i*2)*3+1) = G;
			*(pu8Buf + (i*2)*3+2) = R;

			
			/*make second pixel*/

			
			/*make first pixel*/
			//convert to RGB from UYVY
			B=1.164*(*Y2-16)+2.018*(*U-128);
			G=1.164*(*Y2-16)-0.813*(*V-128)-0.391*(*U-128);
			R=1.164*(*Y2-16)+1.596*(*V-128);

			// clamping
			if(B<0)B=0;
			if(R<0)R=0;
			if(G<0)G=0;
			if(B>255)B=255;
			if(R>255)R=255;
			if(G>255)G=255;
			
			// insert to buffer
			*(pu8Buf + ((i*2)+1)*3) = B;
			*(pu8Buf + ((i*2)+1)*3+1) = G;
			*(pu8Buf + ((i*2)+1)*3+2) = R;

			//*(pu8Buf + i*3) = *(pu8Buf + i*3 + 1) = *(pu8Buf + i*3 + 2) = *(pu8Ptr + i);
		}

		if(PPAPI_FATFS_Write((PP_VOID *)pu8Buf, 1, u32Width*3, hFile) != u32Width*3)goto END_FT;
	}

	ret = eSUCCESS;
END_FT:
	if(hFile)PPAPI_FATFS_Close(hFile);
#ifdef CALIB_LIB_USE
	if(pu8Buf) PPAPI_Lib_Ext_Free(pu8Buf);
#else
	if(pu8Buf) OSAL_free(pu8Buf);	
#endif

	return ret;
}

