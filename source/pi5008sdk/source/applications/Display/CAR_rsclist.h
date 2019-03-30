/* CAR_rsclist.h */
#ifndef __CAR_RSCLIST_H__
#define __CAR_RSCLIST_H__

#ifdef __cplusplus
EXTERN "C" {
#endif

typedef enum ppCAR_RSCLIST_E {


#if (!defined (USE_16BIT_CAR) || defined(USE_SEPERATE_SHADOW))
	ecarImage_LUT,		//0
#endif

	eTop2d_carImage_tire01,		//1
#if defined (CACHE_VIEW_USE)
	eView360_carImage_tire01_start,		//2
	eView360_carImage_tire01_end = 	eView360_carImage_tire01_start + 179,		//181
#endif

#if defined (USE_SEPERATE_SHADOW)
	eTop2d_carImage_tire01_shadow,		  //182
#if defined (CACHE_VIEW_USE)
	eView360_carImage_tire01_shadow_start,		 //183
	eView360_carImage_tire01_shadow_end = 	eView360_carImage_tire01_shadow_start + 179,		 //362
#endif 
#endif

#if defined (USE_CAR_WHEEL)
#if defined (CACHE_VIEW_USE)
	eView360_carImage_wheel_start,		//363
	eView360_carImage_wheel_end = eView360_carImage_wheel_start + 23,			//386
#endif
#endif


#if defined (USE_CAR_DOOR)
	eTop2d_carImage_tire01_door_start,
	eTop2d_carImage_tire01_lf = eTop2d_carImage_tire01_door_start,		//387
	eTop2d_carImage_tire01_lf_rf,		//388
	eTop2d_carImage_tire01_lf_rf_lr,	//389
eTop2d_carImage_tire01_lf_rf_rr,		//390
	eTop2d_carImage_tire01_lf_rf_lr_rr,	//391
	eTop2d_carImage_tire01_lf_lr,		//392
	eTop2d_carImage_tire01_lf_lr_rr,	//393
	eTop2d_carImage_tire01_lf_rr,		//394
	eTop2d_carImage_tire01_rf,			//395
	eTop2d_carImage_tire01_rf_lr,		//396
	eTop2d_carImage_tire01_rf_lr_rr,	//397
	eTop2d_carImage_tire01_rf_rr,		//398
	eTop2d_carImage_tire01_lr,			//399
	eTop2d_carImage_tire01_lr_rr,		//400
	eTop2d_carImage_tire01_rr,			//401
	eTop2d_carImage_tire01_door_end = eTop2d_carImage_tire01_rr,
#endif


	eCAR_RSCLIST_MAX
} PP_CAR_RSCLIST_E;

#ifdef __cplusplus
}
#endif

#endif  // __CAR_RSCLIST_H__
