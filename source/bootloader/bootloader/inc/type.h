#ifndef TYPE_H
#define TYPE_H

typedef char                       	sint8;
typedef unsigned char  				uint8;

typedef short                       sint16;
typedef unsigned short              uint16;

typedef int                         sint32;
typedef unsigned int                uint32;

typedef float                       float32;

typedef volatile unsigned char      vuint8;
typedef volatile unsigned short     vuint16;
typedef volatile unsigned long      vuint32;

typedef volatile char               vsint8;
typedef volatile short              vsint16;
typedef volatile int                vsint32;

typedef enum {FALSE, TRUE}			bool;

typedef unsigned char 				uchar;
typedef unsigned short 				ushort;
typedef unsigned long 				ulong;
typedef unsigned 					uint;

typedef unsigned char				UINT8;
typedef signed char					INT8;
typedef unsigned short				UINT16;
typedef signed short				INT16;
typedef unsigned int				UINT32;
typedef signed int					INT32;
typedef unsigned long long			UINT64;
typedef signed long long			INT64;

typedef unsigned long long 			offset_t;

typedef unsigned int				euint32;
typedef unsigned short				euint16;
typedef unsigned char				euint8;

typedef signed int					esint32;
typedef signed short				esint16;
typedef signed char					esint8;

typedef int							eint32;
typedef short						eint16;
typedef char						eint8;



#ifndef NULL
#define NULL						(0)
#endif

#ifndef NULL_PTR
#define NULL_PTR                    ((void*)0)
#endif




#endif // __TYPE_H__

