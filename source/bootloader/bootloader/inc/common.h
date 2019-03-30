#ifndef _COMMON_DEF_H
#define _COMMON_DEF_H

#define MAX_DATA_SIZE		0x400	// MAX 1KB

#define ALIGN(X,SIZE)	( ((X)+(SIZE-1))&(~(SIZE-1)) )

#endif
