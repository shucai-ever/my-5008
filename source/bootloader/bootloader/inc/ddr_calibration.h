#ifndef _DDR_CALIBRATION_H_
#define _DDR_CALIBRATION_H_

#define DDR_CAL_WLEVEL		(1<<0)
#define DDR_CAL_RLEVEL		(1<<1)
#define DDR_CAL_GDS			(1<<2)
#define DDR_CAL_RDQDQS		(1<<3)
#define DDR_CAL_WDQDQS		(1<<3)


int ddr_calibration(int opt);
int memory_size_check(void);

#endif /* _DDR_CALIBRATION_H_ */
