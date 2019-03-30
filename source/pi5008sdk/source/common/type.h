/**
 * \file
 *
 * \brief	type
 *
 * Copyright (c) 2016 Pixelplus Co.,Ltd. All rights reserved
 *
 */

#ifndef TYPE_H
#define TYPE_H

//#include "inttypes.h"


typedef char                             sint8;
typedef unsigned char                   uint8;

typedef short                           sint16;
typedef unsigned short                 uint16;

typedef int                             sint32;
typedef unsigned int                   uint32;

typedef float                          float32;

typedef volatile unsigned char        vuint8;
typedef volatile unsigned short      vuint16;
typedef volatile unsigned long       vuint32;

typedef volatile char                  vsint8;
typedef volatile short                vsint16;
typedef volatile int                  vsint32;

typedef enum {FALSE, TRUE}			       bool;

typedef unsigned char					uint8_t;
typedef unsigned short					uint16_t;

#ifndef NULL
#define NULL						(0)
#endif

#ifndef NULL_PTR
#define NULL_PTR                    ((void*)0)
#endif

#define IN
#define OUT
#define INOUT

#define STATIC                      static
#define CONST                       const
#define EXTERN						extern

#define FLAG_SET                    (1U)
#define FLAG_CLEAR                  (0U)


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

typedef void*						HANDLE;

typedef void (*handler)(void);

////////////////////////PI5008K////////////////////////////
typedef unsigned char				PP_U8;
typedef unsigned short				PP_U16;
typedef unsigned int				PP_U32;
typedef unsigned long long			PP_U64;
typedef signed char					PP_S8;
typedef signed short				PP_S16;
typedef signed int					PP_S32;
typedef signed long long			PP_S64;
typedef char						PP_CHAR;
typedef float						PP_F32;
typedef double						PP_D64;
typedef void						PP_VOID;

typedef volatile unsigned char		PP_VU8;
typedef volatile unsigned short		PP_VU16;
typedef volatile unsigned int		PP_VU32;

typedef void*						PP_HANDLE;

#define PP_SUCCESS					(0L)
#define PP_FAILURE					(-1)
#define PP_NULL					    (NULL)
#define PP_INVALID				    (-1)
#define PP_WAIT_TIMEOUT			    (1)
typedef enum { PP_FALSE = 0, PP_TRUE = 1, }PP_BOOL;
typedef PP_BOOL              Boolean;


///////////////////////////////////////////////////////////

#endif // __TYPE_H__
