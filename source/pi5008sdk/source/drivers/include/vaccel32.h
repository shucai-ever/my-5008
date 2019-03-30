//------------------------------------------//----------------------------------
//  VACCEL3
//------------------------------------------//----------------------------------
#ifndef __VACCEL3_H__
    #define     __VACCEL3_H__

typedef unsigned int   word;
typedef unsigned char  byte;

//------------------------------------------//----------------------------------
#define V3DMA_COPY_NORMAL           0
#define V3DMA_COPY_NOT              1
#define V3DMA_COPY_BYTE_SWAP        2
#define V3DMA_COPY_WORD_SWAP        3

#define V3DMA_MODE_COPY             0
#define V3DMA_MODE_RLE              2
#define V3DMA_MODE_HAMMING          3
#define V3DMA_MODE_DMA2OTF          7

typedef volatile struct {
    word    ctrl;                           // [0] start, busy (auto clear)
                                            // [1] irq_done, manual clear
                                            // [3:2] function_mode_minor
                                            // [6:4] function_mode
                                            // 0: 2d-dma, 
                                            // 1: xyz2xy, 
                                            // 2: rle_encode, 
                                            // 3: hamming_distance, 
                                            // 7: dma2otf, 
                                            // [7] use_wdma1d_otf, undefined length
                                            // [15:8] reserved
                                            // [23:16] mem2mem_bw
                                            // [31:24] otf_bw
    word    rdma2d_base;                    // [31:0] rdma2d_base
    word    rdma2d_stride;                  // [15:0] rdma2d_stride
    word    rdma2d_size;                    // [15:0] rdma2d_width
                                            // [31:16] rdma2d_height
    word    wdma2d_base;                    // [31:0] wdma2d_base
    word    wdma2d_stride;                  // [15:0] wdma2d_stride
    word    wdma2d_size;                    // [15:0] wdma2d_width
                                            // [31:16] wdma2d_height
    word    wdma1d_base;                    // [31:0] wdma1d_base
    word    ro_wdma1d_wordcount;            // [15:0] wdma1d_word_count
    word    wdma_rlewaitdone_count;         // [31:0] wdma_rlewaitdone_count
    word    _reserved[5];                   // 
    word    version;                        // [31:0] YYYYMMDD
} REG_V3DMA_CTRL;

//------------------------------------------//----------------------------------
typedef struct {
} REG_V3DMA_PROJ;

//------------------------------------------//----------------------------------
typedef struct {
} REG_V3DMA_RLE;

//------------------------------------------//----------------------------------
typedef volatile struct {
    word    desc[4];
    word    threshold;
    word    _reserved[3];                   // 
    word    ro_count;
} REG_V3DMA_HAMMING;

//------------------------------------------//----------------------------------
typedef volatile struct {
    word    config;                         // 00: 
                                            // [0] enable, only valid on negedge of vsync
                                            // [7:4] fast_n
                                            // 
                                            // [18:16] ch_sel, 0~3 : isp, 4: svm, 5: bto
                                            // [19] 1: 5x3 nms, 0: 3x3 nms
                                            // [31:20] config
                                            //  [20] : brief_enable
                                            //  [21] : roi_enable
                                            //  [22] : scl_enable
                                            //  [23] : dma2otf_enable
                                            //  [24] : use_posedge_vsync
                                            //  [26:25] : otf2dma_enable
                                            //      0: disable, 1:scl2dma, 2:filterfast2dma, 3:filterbrief2dma
                                            //  [27] : fast_n_be
                                            //  [29:28] : wdma_flush (write 1 and write 0, manually)
                                            //      00: auto flush, 1x: manual flush [28]: 1:manual flush, 0: off, manual setting 01 -> 00
    word    size;                           // 04: 
                                            // [31:16] width
                                            // [15:0] height
    word    bandwidth;                      // 08: 
                                            // [7:0] bw_fast
                                            // [15:8] bw_brief
                                            // [31:8] counter_limit for a zone, interrupt asserted when the featur point is over the counter_limit
    word    roi_start;                      // 0C: 
                                            // [15:0] roi_start_y, dst_width
                                            // [31:16] roi_start_x, dst_height
    word    roi_end;                        // 10: 
                                            // [15:0] roi_end_y, hor_ratio
                                            // [31:16] roi_end_x, ver_ratio
    word    xy_base;                        // 14: [31:0] xy_base (x,y) 32-bit unit, padded, for each zones
    word    desc_base;                      // 18: [31:0] desc_base, 128-bit unit, for each zones
    word    fc_base;                        // 1c: [31:0] feature_count_base, 16-bit counter for each line of each zones are stored
    word    scl_size;                       // 20: [31:16] scl_width, [15:0] scl_height
    word    scl_ratio;                      // 24: [31:16] scl_hor_ratio, [15:0] scl_ver_ratio
    word    zone2lut[2];                    // 28: brief_lut index for each zones, zone0~7
                                            // [31:28] : brief_lut index for zone7
                                            // [27:24] : brief_lut index for zone6
                                            // [23:20] : brief_lut index for zone5
                                            // [19:16] : brief_lut index for zone4
                                            // [15:12] : brief_lut index for zone3
                                            // [11:8] : brief_lut index for zone2
                                            // [7:4] : brief_lut index for zone1
                                            // [3:0] : brief_lut index for zone0
                                            // 2c: brief_lut index for each zones, zone8~15
                                            // [31:28] : brief_lut index for zone15
                                            // [27:24] : brief_lut index for zone14
                                            // [23:20] : brief_lut index for zone13
                                            // [19:16] : brief_lut index for zone12
                                            // [15:12] : brief_lut index for zone11
                                            // [11:8] : brief_lut index for zone10
                                            // [7:4] : brief_lut index for zone9
                                            // [3:0] : brief_lut index for zone8
    word    zone2th[4];                     // 30: fast_th for each zones, zone0~3
                                            // [31:24] : fast_th for zone3
                                            // [23:16] : fast_th for zone2
                                            // [15:8] : fast_th for zone1
                                            // [7:0] : fast_th for zone0
                                            // 34: fast_th for each zones, zone4~7
                                            // [31:24] : fast_th for zone7
                                            // [23:16] : fast_th for zone6
                                            // [15:8] : fast_th for zone5
                                            // [7:0] : fast_th for zone4
                                            // 38: fast_th for each zones, zone8~11
                                            // [31:24] : fast_th for zone11
                                            // [23:16] : fast_th for zone10
                                            // [15:8] : fast_th for zone9
                                            // [7:0] : fast_th for zone8
                                            // 3c: fast_th for each zones, zone12~15
                                            // [31:24] : fast_th for zone15
                                            // [23:16] : fast_th for zone14
                                            // [15:8] : fast_th for zone13
                                            // [7:0] : fast_th for zone12
    word    zone[16];                       // 40~7c: 16-zones, in 8-pixel unit
                                            // [31:24] eoy, end of y position
                                            // [23:16] soy, start of y position
                                            // [15:8]  eox, end of x position
                                            // [7:0]   sox, start of x position
//------------------------------------------//----------------------------------
//  read-only part
//------------------------------------------//----------------------------------
    word    _reserved_80;                   // 80:
    word    ro_state;                       // 84: 
                                            // [7:0] pix_data
                                            // [8] pix_hsync
                                            // [9] pix_vsync
                                            // [12] frame_busy
                                            // [16] safe_enable
    word    _reserved_88;                   // 88:
    word    frame_counter;                  // 8c: 
                                            // [31:0] frame_counter
    word    ro_error;                       // 90: 
                                            // [0] done
                                            // [1] counter limit over
                                            // [2] fast_error, must reset
                                            // [3] pixel_blocked, must reset, fifo full
                                            // [4] scl_config_error, must reset
                                            // [5] irq_bus_request_not_stored, must reset
                                            // [6] reserved
                                            // [7] reserved
                                            // [8] safe_enabled_sync, sync only durin
    word    ro_irq_from_fast;               // 94:
                                            // [31:0] pos_exception, neg_exception
    word    ro_irq_count_limit;             // 98:
                                            // [15:0] irq_count_limit from 16-zones
    word    ro_version;                     // 9c:
                                            // [31:0] YYYYMMDD
    word    ro_in_active;                   // a0: free-running input
                                            //  [31:16] vsync active count, number of hsync positive edge during vactive
                                            //  [15:0] hsync active count, number of clock during hactive
    word    ro_in_blank;                    // a4:
                                            //  [31:16] vsync blank count, number of hsync positive edge during vblank
                                            //  [15:0] hsync blank count, number of clock during hblank
    word    ro_in_htotal;                   // a8:
                                            //  [15:0] number of clock from hsync positive to the next hsync positive
    word    ro_in_vtotal;                   // ac:
                                            //  [31:0] number of clock from vsync positive to the next vsync positive
    word    ro_safe_active;                 // b0: only for internal h/v sync 
                                            //  [31:16] vsync active count, number of hsync positive edge during vactive
                                            //  [15:0] hsync active count, number of clock during hactive
    word    ro_safe_blank;                  // b4:
                                            //  [31:16] vsync blank count, number of hsync positive edge during vblank
                                            //  [15:0] hsync blank count, number of clock during hblank
    word    ro_safe_htotal;                 // b8:
                                            //  [15:0] number of clock from hsync positive to the next hsync positive
    word    ro_safe_vtotal;                 // bc:
                                            //  [31:0] number of clock from vsync positive to the next vsync positive
    word    ro_num_feature[8];              // c0:
                                            //  [31:16] number of feature for zone1
                                            //  [15:0] number of feature for zone0
                                            // c4:
                                            //  [31:16] number of feature for zone3
                                            //  [15:0] number of feature for zone2
                                            // c8:
                                            //  [31:16] number of feature for zone5
                                            //  [15:0] number of feature for zone4
                                            // cc:
                                            //  [31:16] number of feature for zone7
                                            //  [15:0] number of feature for zone6
                                            // d0:
                                            //  [31:16] number of feature for zone9
                                            //  [15:0] number of feature for zone8
                                            // d4:
                                            //  [31:16] number of feature for zone11
                                            //  [15:0] number of feature for zone10
                                            // d8:
                                            //  [31:16] number of feature for zone13
                                            //  [15:0] number of feature for zone12
                                            // dc:
                                            //  [31:16] number of feature for zone15
                                            //  [15:0] number of feature for zone14
    word    ro_prev_num_feature[8];         // e0:
                                            //  [31:16] number of feature for zone1, previous frame
                                            //  [15:0] number of feature for zone0, previous frame
                                            // e4:
                                            //  [31:16] number of feature for zone3, previous frame
                                            //  [15:0] number of feature for zone2, previous frame
                                            // e8:
                                            //  [31:16] number of feature for zone5, previous frame
                                            //  [15:0] number of feature for zone4, previous frame
                                            // ec:
                                            //  [31:16] number of feature for zone7, previous frame
                                            //  [15:0] number of feature for zone6, previous frame
                                            // f0:
                                            //  [31:16] number of feature for zone9, previous frame
                                            //  [15:0] number of feature for zone8, previous frame
                                            // f4:
                                            //  [31:16] number of feature for zone11, previous frame
                                            //  [15:0] number of feature for zone10, previous frame
                                            // f8:
                                            //  [31:16] number of feature for zone13, previous frame
                                            //  [15:0] number of feature for zone12, previous frame
                                            // fc:
                                            //  [31:16] number of feature for zone15, previous frame
                                            //  [15:0] number of feature for zone14, previous frame

} REG_V3OTF_CTRL;

//------------------------------------------//----------------------------------
typedef volatile struct {
    word    coef0y0;                        // 00:
                                            //  [19:16] c0_x4y0
                                            //  [15:12] c0_x3y0
                                            //  [11: 8] c0_x2y0
                                            //  [ 7: 4] c0_x1y0
                                            //  [ 3: 0] c0_x0y0
    word    coef0y1;                        // 04:
                                            //  [19:16] c0_x4y1
                                            //  [15:12] c0_x3y1
                                            //  [11: 8] c0_x2y1
                                            //  [ 7: 4] c0_x1y1
                                            //  [ 3: 0] c0_x0y1
    word    coef0y2;                        // 08:
                                            //  [19:16] c0_x4y2
                                            //  [15:12] c0_x3y2
                                            //  [11: 8] c0_x2y2
                                            //  [ 7: 4] c0_x1y2
                                            //  [ 3: 0] c0_x0y2
    word    shift10;                        // 0c:
                                            //  [7:4] c0_shift
                                            //  [3:0] c1_shift
    word    coef1y0;                        // 10:
                                            //  [19:16] c1_x4y0
                                            //  [15:12] c1_x3y0
                                            //  [11: 8] c1_x2y0
                                            //  [ 7: 4] c1_x1y0
                                            //  [ 3: 0] c1_x0y0
    word    coef1y1;                        // 14:
                                            //  [19:16] c1_x4y1
                                            //  [15:12] c1_x3y1
                                            //  [11: 8] c1_x2y1
                                            //  [ 7: 4] c1_x1y1
                                            //  [ 3: 0] c1_x0y1
    word    coef1y2;                        // 18:
                                            //  [19:16] c1_x4y2
                                            //  [15:12] c1_x3y2
                                            //  [11: 8] c1_x2y2
                                            //  [ 7: 4] c1_x1y2
                                            //  [ 3: 0] c1_x0y2
    word    _reserved_1c;                   // 1c:
    word    zone2coef_fast[2];              // 20:
                                            //  [31:28] zone7 select coef
                                            //  [27:24] zone6 select coef
                                            //  [23:20] zone5 select coef
                                            //  [19:16] zone4 select coef
                                            //  [15:12] zone3 select coef
                                            //  [11: 8] zone2 select coef
                                            //  [ 7: 4] zone1 select coef
                                            //  [ 3: 0] zone0 select coef
                                            // 24:
                                            //  [31:28] zone15 select coef
                                            //  [27:24] zone14 select coef
                                            //  [23:20] zone13 select coef
                                            //  [19:16] zone12 select coef
                                            //  [15:12] zone11 select coef
                                            //  [11: 8] zone10 select coef
                                            //  [ 7: 4] zone9 select coef
                                            //  [ 3: 0] zone8 select coef
    word    zone2coef_brief[2];             // 28:
                                            //  [31:28] zone7 select coef
                                            //  [27:24] zone6 select coef
                                            //  [23:20] zone5 select coef
                                            //  [19:16] zone4 select coef
                                            //  [15:12] zone3 select coef
                                            //  [11: 8] zone2 select coef
                                            //  [ 7: 4] zone1 select coef
                                            //  [ 3: 0] zone0 select coef
                                            // 2C:
                                            //  [31:28] zone15 select coef
                                            //  [27:24] zone14 select coef
                                            //  [23:20] zone13 select coef
                                            //  [19:16] zone12 select coef
                                            //  [15:12] zone11 select coef
                                            //  [11: 8] zone10 select coef
                                            //  [ 7: 4] zone9 select coef
                                            //  [ 3: 0] zone8 select coef
} REG_V3OTF_FILTER;

//------------------------------------------//----------------------------------
typedef volatile struct {
    word    lut[32];                        // 00:
                                            //  [31:24] lut3
                                            //  [23:16] lut2
                                            //  [15: 8] lut1
                                            //  [ 7: 0] lut0
                                            //      [7:6] : p1x
                                            //      [5:4] : p1y
                                            //      [3:2] : p0x
                                            //      [1:0] : p0y
                                            // 04:
                                            //  [31:24] lut7
                                            //  [23:16] lut6
                                            //  [15: 8] lut5
                                            //  [ 7: 0] lut4
                                            // ...
} REG_V3OTF_BRIEF_LUT;

//------------------------------------------//----------------------------------
typedef volatile struct {
    word    ctrl;                           // [0] : ch0 enable, RDMA2D
                                            // [1] : ch0 clear , RDMA2D
                                            // [2] : ch1 enable, WDMA1D
                                            // [3] : ch1 clear , WDMA1D
                                            // [4] : ch2 enable, reserved
                                            // [5] : ch2 clear , reserved
                                            // [6] : ch3 enable, WDMA1D
                                            // [7] : ch3 clear , WDMA1D
    word    _rserved_0x[7];
    word    ch0_total;                     // total number of cycle after clear and enabled
    word    ch0_word;                      // total number of word after clear and enabled
    word    ch1_total;
    word    ch1_word;
    word    ch2_total;
    word    ch2_word;
    word    ch3_total;
    word    ch3_word;
} REG_V3DMA_BUS_MON;

//------------------------------------------//----------------------------------
typedef volatile struct {
    word    ctrl;                           // [0] : ch0 enable,
                                            // [1] : ch0 clear ,
                                            // [2] : ch1 enable,
                                            // [3] : ch1 clear ,
                                            // [4] : ch2 enable,
                                            // [5] : ch2 clear ,
                                            // [6] : ch3 enable,
                                            // [7] : ch3 clear ,
    word    _rserved_0x[7];
    word    ch0_total;                      // total number of cycle after clear and enabled
    word    ch0_word;                       // total number of word after clear and enabled
    word    ch1_total;
    word    ch1_word;
    word    ch2_total;
    word    ch2_word;
    word    ch3_total;
    word    ch3_word;
} REG_V3OTF_BUS_MON;



//------------------------------------------//----------------------------------
#define V_BASE                                  0xf1400000

//  #define VACCEL3         ((REG_VACCEL3 *)        V_BASE)
#define V3DMA_CTRL          ((REG_V3DMA_CTRL *)     (V_BASE + 0x00000))
#define V3DMA_PROJ          ((REG_V3DMA_PROJ *)     (V_BASE + 0x00100))
#define V3DMA_RLE           ((REG_V3DMA_RLE *)      (V_BASE + 0x00200))
#define V3DMA_HAMMING       ((REG_V3DMA_HAMMING *)  (V_BASE + 0x00300))
#define V3DMA_BUS_MON       ((REG_V3DMA_BUS_MON *)  (V_BASE + 0x00600))

#define V3OTF_CTRL          ((REG_V3OTF_CTRL *)     (V_BASE + 0x20000))
#define V3OTF_FILTER        ((REG_V3OTF_FILTER *)   (V_BASE + 0x22000))
#define V3OTF_BUS_MON       ((REG_V3OTF_BUS_MON *)  (V_BASE + 0x24000))
#define V3OTF_BRIEF_LUT     ((REG_V3OTF_BRIEF_LUT *)(V_BASE + 0x26000))

#define V3OTF_MEM_FAST      ((word *)   (V_BASE + 0x28000))      // 0: 32-bit, 4: 24-bit, 8: 32-bit, ..., 27f8: 32-bit, 27fc: 24-bit
#define V3OTF_MEM_SCORE     ((word *)   (V_BASE + 0x30000))      // 0: 24-bit, 4: 24-bit, 13fc: 24-bit
#define V3OTF_MEM_BRIEF     ((word *)   (V_BASE + 0x38000))      // 0: 32-bit, 4: 32-bit, 8: 32-bit, c: none, 4ff0: 32-bit, 4ff4: 32-bit, 4ff8: 32-bit, 4ffc: none

//------------------------------------------//----------------------------------

typedef struct {
    unsigned short  x;
    unsigned short  y;
} POSITION;

typedef struct {
    POSITION    src;
    POSITION    dst;
} VECTOR;

typedef struct {
    unsigned int desc[4];
} DESCRIPTOR;

typedef struct {
    unsigned int    xy_base;
    unsigned int    desc_base;
    unsigned int    num_feature;
} FRAME_INFO;

typedef struct {
    FRAME_INFO  f0;
    FRAME_INFO  f1;
    unsigned char fast_n;
    unsigned char fast_th;
    unsigned char ch_sel;
} CORNERS;

#endif  // __VACCEL3_H__
