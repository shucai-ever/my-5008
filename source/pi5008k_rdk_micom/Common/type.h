#ifndef		__TYPE_H__	
#define		__TYPE_H__

#include "stm32f10x.h"
// -----------------------------------------------------------------------------
// Include files
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Struct/Union Types and define
// -----------------------------------------------------------------------------

typedef void				VOID, *PVOID;		// void
typedef int8_t			CHAR, *PCHAR;		// signed 8bit
typedef int16_t			SHORT, *PSHORT;		// signed 16bit
typedef int32_t			LONG, *PLONG;		// signed 32bit

typedef uint8_t			byte, *pbyte ;
typedef uint8_t			BYTE, *PBYTE;		// unsigned 8bit
typedef uint8_t			uchar, *puchar ;
typedef uint8_t			UCHAR, *PUCHAR ;

typedef uint16_t			word, *pword ;
typedef uint16_t			WORD, *PWORD;		// unsigned 16bit
typedef uint16_t			ushort, *pushort ;
typedef uint16_t			USHORT, *PUSHORT ;

typedef uint32_t			ulong, *pulong ;
typedef uint32_t			ULONG, *PULONG ;
typedef uint32_t			dword, *pdword ;
typedef uint32_t			DWORD, *PDWORD;		// unsigned 32bit
typedef	float				FLOAT, *PFLOAT;		// floating number

typedef volatile void		VVOID, *VPVOID;		// volatile void
typedef volatile int8_t		VCHAR, *VPCHAR;		// volatile signed 8bit
typedef volatile int16_t		VSHORT, *VPSHORT;	// volatile signed 16bit
typedef volatile int32_t		VLONG, *VPLONG;		// volatile signed 32bit

typedef volatile uint8_t		VBYTE, *VPBYTE;		// volatile unsigned 8bit
typedef volatile uint16_t	VWORD, *VPWORD;		// volatile unsigned 16bit
typedef volatile uint32_t	VDWORD, *VPDWORD;	// volatile unsigned 32bit

typedef DWORD			MDIN_COLORRGB, *PMDIN_COLORRGB;	// RGB color
typedef DWORD			MDIN_COLORYBR, *PMDIN_COLORYBR;	// YCbCr color

#define	BOOL			BYTE				// boolean flag
#define	ROMDATA		const				// program(read-only) memory

//#define	TRUE				1
//#define	FALSE				0

#define	ON					1
#define	OFF					0

#define	HIGH				1
#define	LOW				0

#define	POSI				1
#define	NEGA				0

#define	NACK				1
#define	ACK					0

#define DMA_RD				1
#define DMA_WR				0

#ifndef NULL
#define	NULL				0
#endif


#define		DIFF(a,b)			((a)>(b)? (a-b):(b-a))
#define		MBIT(a,b)			(((a)&(b))? ON : OFF)
#define		RBIT(a,b)			(((a)&(b))? OFF : ON)

#define		MIN(a,b)				((a)>(b)? (b):(a))
#define		MAX(a,b)			((a)>(b)? (a):(b))

#define		HI4BIT(a)			((BYTE)((a)>>4))
#define		LO4BIT(a)			((BYTE)((a)&0x0f))

#define		HIBYTE(a)			((BYTE)((a)>>8))
#define		LOBYTE(a)			((BYTE)((a)&0xff))

#define		HIWORD(a)			((WORD)((a)>>16))
#define		LOWORD(a)			((WORD)((a)&0xffff))

#define		CLIP12(a)			((WORD)((((a)>2047)? 2047:((a)<-2048)? -2048:(a))))
#define		CLIP09(a)			((WORD)((((a)>255)? 255:((a)<-256)? -256:(a))))

#define		MAKEBYTE(a,b)		((BYTE)(((BYTE)(a)<<4)|(b)))
#define		MAKEWORD(a,b)		((WORD)(((WORD)(a)<<8)|(b)))
#define		MAKEDWORD(a,b)	((DWORD)(((DWORD)(a)<<16)|(b)))

#define		BYTESWAP(a)		(MAKEWORD(LOBYTE(a), HIBYTE(a)))

#define		ADDR2ROW(a,b)		((WORD)((((a)/(b))*2)|(((a)%(b))/((b)/2))))
#define		ADDR2COL(a,b)		((WORD)((a)%((b)/2)))
#define		ROW2ADDR(a,b)		((DWORD)((a)/2*(b))+(((a)&1)*((b)/2)))

#define		ROUNDUP(a,b)		((WORD)(((a)/(b))+(((a)%(b))? 1:0)))
#define		ROUNDDN(a,b)		((WORD)(((a)/(b))))

#endif //		__TYPE_H__	
