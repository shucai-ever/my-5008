1. Setup driver for user board
	1) Make "pr1000_user_config.c"
		: The "pr1000_user_config.c" file is board specific configuration file.
		: Refering to "pr1000_user_config_sample_xxxx" files.
		
	2) Make pr1000_drv.ko and loading device driver.
	
2. Control the phase delay of VDCKx pin. (reference datasheet 0xE8,0xE9 VDCKx_DEL)
	1) Find stable phase delay [0x8 ~ 0xF]
		: If phase delay unstable, live video is broken or stop.
		: If fined stable phase delay is 0x8/0x9/0xA, get center value-0x9.
	2) Apply device driver.
		: Update got value to PR1000_VIDOUTF_CLKPHASE of "pr1000_user_config.c"