#ifndef __PCV_TYPE_DEF_H__
#define __PCV_TYPE_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct 
{
	int top;
	int bottom;
	int left;
	int right;
} PcvRect32d;

typedef struct 
{
	int x;
	int y;
} PcvPoint32d;

typedef struct 
{
	float x;
	float y;
} PcvPoint32f;

typedef struct 
{
	float x;
	float y;
	float z;
} PcvPoint3d32f;

#ifdef __cplusplus
}
#endif

#endif /* __PCV_TYPE_DEF_H__ */