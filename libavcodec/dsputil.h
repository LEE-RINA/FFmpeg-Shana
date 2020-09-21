/*
 * DSP utils
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * DSP utils.
 * Note, many functions in here may use MMX which trashes the FPU state, it is
 * absolutely necessary to call emms_c() between DSP & float/double code.
 */

#ifndef AVCODEC_DSPUTIL_H
#define AVCODEC_DSPUTIL_H

#include "avcodec.h"

extern uint32_t ff_square_tab[512];

void ff_gmc_c(uint8_t *dst, uint8_t *src, int stride, int h, int ox, int oy,
              int dxx, int dxy, int dyx, int dyy, int shift, int r,
              int width, int height);

/* minimum alignment rules ;)
 * If you notice errors in the align stuff, need more alignment for some ASM code
 * for some CPU or need to use a function with less aligned data then send a mail
 * to the ffmpeg-devel mailing list, ...
 *
 * !warning These alignments might not match reality, (missing attribute((align))
 * stuff somewhere possible).
 * I (Michael) did not check them, these are just the alignments which I think
 * could be reached easily ...
 *
 * !future video codecs might need functions with less strict alignment
 */

struct MpegEncContext;
/* Motion estimation:
 * h is limited to { width / 2, width, 2 * width },
 * but never larger than 16 and never smaller than 2.
 * Although currently h < 4 is not used as functions with
 * width < 8 are neither used nor implemented. */
typedef int (*me_cmp_func)(struct MpegEncContext *c,
                           uint8_t *blk1 /* align width (8 or 16) */,
                           uint8_t *blk2 /* align 1 */, int line_size, int h);

/**
 * Scantable.
 */
typedef struct ScanTable {
    const uint8_t *scantable;
    uint8_t permutated[64];
    uint8_t raster_end[64];
} ScanTable;

void ff_init_scantable(uint8_t *permutation, ScanTable *st,
                       const uint8_t *src_scantable);
void ff_init_scantable_permutation(uint8_t *idct_permutation,
                                   int idct_permutation_type);

/**
 * DSPContext.
 */
typedef struct DSPContext {
    /* pixel ops : interface with DCT */
    void (*get_pixels)(int16_t *block /* align 16 */,
                       const uint8_t *pixels /* align 8 */,
                       int line_size);
    void (*diff_pixels)(int16_t *block /* align 16 */,
                        const uint8_t *s1 /* align 8 */,
                        const uint8_t *s2 /* align 8 */,
                        int stride);
    void (*put_pixels_clamped)(const int16_t *block /* align 16 */,
                               uint8_t *pixels /* align 8 */,
                               int line_size);
    void (*put_signed_pixels_clamped)(const int16_t *block /* align 16 */,
                                      uint8_t *pixels /* align 8 */,
                                      int line_size);
    void (*add_pixels_clamped)(const int16_t *block /* align 16 */,
                               uint8_t *pixels /* align 8 */,
                               int line_size);
    int (*sum_abs_dctelem)(int16_t *block /* align 16 */);
    /**
     * translational global motion compensation.
     */
    void (*gmc1)(uint8_t *dst /* align 8 */, uint8_t *src /* align 1 */,
                 int srcStride, int h, int x16, int y16, int rounder);
    /**
     * global motion compensation.
     */
    void (*gmc)(uint8_t *dst /* align 8 */, uint8_t *src /* align 1 */,
                int stride, int h, int ox, int oy,
                int dxx, int dxy, int dyx, int dyy,
                int shift, int r, int width, int height);

    int (*pix_sum)(uint8_t *pix, int line_size);
    int (*pix_norm1)(uint8_t *pix, int line_size);

    me_cmp_func sad[6]; /* identical to pix_absAxA except additional void * */
    me_cmp_func sse[6];
    me_cmp_func hadamard8_diff[6];
    me_cmp_func dct_sad[6];
    me_cmp_func quant_psnr[6];
    me_cmp_func bit[6];
    me_cmp_func rd[6];
    me_cmp_func vsad[6];
    me_cmp_func vsse[6];
    me_cmp_func nsse[6];
    me_cmp_func w53[6];
    me_cmp_func w97[6];
    me_cmp_func dct_max[6];
    me_cmp_func dct264_sad[6];

    me_cmp_func me_pre_cmp[6];
    me_cmp_func me_cmp[6];
    me_cmp_func me_sub_cmp[6];
    me_cmp_func mb_cmp[6];
    me_cmp_func ildct_cmp[6]; // only width 16 used
    me_cmp_func frame_skip_cmp[6]; // only width 8 used

    me_cmp_func pix_abs[2][4];

    void (*bswap_buf)(uint32_t *dst, const uint32_t *src, int w);
    void (*bswap16_buf)(uint16_t *dst, const uint16_t *src, int len);

    /* assume len is a multiple of 8, and arrays are 16-byte aligned */
    void (*vector_clipf)(float *dst /* align 16 */,
                         const float *src /* align 16 */,
                         float min, float max, int len /* align 16 */);

    /* (I)DCT */
    void (*fdct)(int16_t *block /* align 16 */);
    void (*fdct248)(int16_t *block /* align 16 */);

    /* IDCT really */
    void (*idct)(int16_t *block /* align 16 */);

    /**
     * block -> idct -> clip to unsigned 8 bit -> dest.
     * (-1392, 0, 0, ...) -> idct -> (-174, -174, ...) -> put -> (0, 0, ...)
     * @param line_size size in bytes of a horizontal line of dest
     */
    void (*idct_put)(uint8_t *dest /* align 8 */,
                     int line_size, int16_t *block /* align 16 */);

    /**
     * block -> idct -> add dest -> clip to unsigned 8 bit -> dest.
     * @param line_size size in bytes of a horizontal line of dest
     */
    void (*idct_add)(uint8_t *dest /* align 8 */,
                     int line_size, int16_t *block /* align 16 */);

    /**
     * IDCT input permutation.
     * Several optimized IDCTs need a permutated input (relative to the
     * normal order of the reference IDCT).
     * This permutation must be performed before the idct_put/add.
     * Note, normally this can be merged with the zigzag/alternate scan<br>
     * An example to avoid confusion:
     * - (->decode coeffs -> zigzag reorder -> dequant -> reference IDCT -> ...)
     * - (x -> reference DCT -> reference IDCT -> x)
     * - (x -> reference DCT -> simple_mmx_perm = idct_permutation
     *    -> simple_idct_mmx -> x)
     * - (-> decode coeffs -> zigzag reorder -> simple_mmx_perm -> dequant
     *    -> simple_idct_mmx -> ...)
     */
    uint8_t idct_permutation[64];
    int idct_permutation_type;
#define FF_NO_IDCT_PERM 1
#define FF_LIBMPEG2_IDCT_PERM 2
#define FF_SIMPLE_IDCT_PERM 3
#define FF_TRANSPOSE_IDCT_PERM 4
#define FF_PARTTRANS_IDCT_PERM 5
#define FF_SSE2_IDCT_PERM 6

    int (*try_8x8basis)(int16_t rem[64], int16_t weight[64],
                        int16_t basis[64], int scale);
    void (*add_8x8basis)(int16_t rem[64], int16_t basis[64], int scale);
#define BASIS_SHIFT 16
#define RECON_SHIFT 6

    void (*draw_edges)(uint8_t *buf, int wrap, int width, int height,
                       int w, int h, int sides);
#define EDGE_WIDTH 16
#define EDGE_TOP    1
#define EDGE_BOTTOM 2

    void (*shrink[4])(uint8_t *dst, int dst_wrap, const uint8_t *src,
                      int src_wrap, int width, int height);

    /**
     * Calculate scalar product of two vectors.
     * @param len length of vectors, should be multiple of 16
     */
    int32_t (*scalarproduct_int16)(const int16_t *v1,
                                   const int16_t *v2 /* align 16 */, int len);

    /**
     * Clip each element in an array of int32_t to a given minimum and
     * maximum value.
     * @param dst  destination array
     *             constraints: 16-byte aligned
     * @param src  source array
     *             constraints: 16-byte aligned
     * @param min  minimum value
     *             constraints: must be in the range [-(1 << 24), 1 << 24]
     * @param max  maximum value
     *             constraints: must be in the range [-(1 << 24), 1 << 24]
     * @param len  number of elements in the array
     *             constraints: multiple of 32 greater than zero
     */
    void (*vector_clip_int32)(int32_t *dst, const int32_t *src, int32_t min,
                              int32_t max, unsigned int len);
} DSPContext;

void ff_dsputil_static_init(void);
void ff_dsputil_init(DSPContext *p, AVCodecContext *avctx);
void avpriv_dsputil_init(DSPContext* p, AVCodecContext *avctx);
attribute_deprecated void dsputil_init(DSPContext* c, AVCodecContext *avctx);

int ff_check_alignment(void);

void ff_set_cmp(DSPContext *c, me_cmp_func *cmp, int type);

void ff_dsputil_init_alpha(DSPContext* c, AVCodecContext *avctx);
void ff_dsputil_init_arm(DSPContext *c, AVCodecContext *avctx,
                         unsigned high_bit_depth);
void ff_dsputil_init_ppc(DSPContext *c, AVCodecContext *avctx,
                         unsigned high_bit_depth);
void ff_dsputil_init_x86(DSPContext *c, AVCodecContext *avctx,
                         unsigned high_bit_depth);

void ff_dsputil_init_dwt(DSPContext *c);

#endif /* AVCODEC_DSPUTIL_H */
