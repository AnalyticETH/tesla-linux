/*
* Support for Intel Camera Imaging ISP subsystem.
 * Copyright (c) 2010 - 2020, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
*/

#ifndef __IA_CSS_PROGRAM_GROUP_DATA_DEFS_H
#define __IA_CSS_PROGRAM_GROUP_DATA_DEFS_H


/*
 * Pre-defined frame format
 *
 * Those formats have inbuild support of traffic
 * and access functions
 *
 * Note that the formats are for terminals, so there
 * is no distinction between input and output formats
 *	- Custom formats with ot without descriptor
 *	- 4CC formats such as YUV variants
 *	- MIPI (line) formats as produced by CSI receivers
 *	- MIPI (sensor) formats such as Bayer or RGBC
 *	- CSS internal formats (private types)
 *  - CSS parameters (type 1 - 6)
 */
#define IA_CSS_FRAME_FORMAT_TYPE_BITS 32
typedef enum ia_css_frame_format_type {
	IA_CSS_DATA_CUSTOM_NO_DESCRIPTOR = 0,
	IA_CSS_DATA_CUSTOM,

	/* 12 bit YUV 411, Y, UV 2-plane  (8 bit per element) */
	IA_CSS_DATA_FORMAT_NV11,
	/* bpp bit YUV 420, Y, U, V 3-plane (bpp/1.5 bpe) */
	IA_CSS_DATA_FORMAT_YUV420,
	/* 12 bit YUV 420, Y, V, U 3-plane (8 bit per element) */
	IA_CSS_DATA_FORMAT_YV12,
	/* 12 bit YUV 420, Y, UV 2-plane (8 bit per element) */
	IA_CSS_DATA_FORMAT_NV12,
	/* 16 bit YUV 420, Y, UV 2-plane (8 bit per element) */
	IA_CSS_DATA_FORMAT_NV12_16,
	/* 12 bit YUV 420, Intel proprietary tiled format, TileY */
	IA_CSS_DATA_FORMAT_NV12_TILEY,
	/* 12 bit YUV 420, Y, VU 2-plane  (8 bit per element) */
	IA_CSS_DATA_FORMAT_NV21,
	/* bpp bit YUV 422, Y, U, V 3-plane (bpp/2 bpe) */
	IA_CSS_DATA_FORMAT_YUV422,
	/* 16 bit YUV 422, Y, V, U 3-plane  (8 bit per element) */
	IA_CSS_DATA_FORMAT_YV16,
	/* 16 bit YUV 422, Y, UV 2-plane  (8 bit per element) */
	IA_CSS_DATA_FORMAT_NV16,
	/* 16 bit YUV 422, Y, VU 2-plane  (8 bit per element) */
	IA_CSS_DATA_FORMAT_NV61,
	/* 16 bit YUV 422, UYVY 1-plane interleaved  (8 bit per element) */
	IA_CSS_DATA_FORMAT_UYVY,
	/* 16 bit YUV 422, YUYV 1-plane interleaved  (8 bit per element) */
	IA_CSS_DATA_FORMAT_YUYV,
	/* bpp bit YUV 444, Y, U, V 3-plane (bpp/3 bpe) */
	IA_CSS_DATA_FORMAT_YUV444,
	/* 8 bit monochrome plane */
	IA_CSS_DATA_FORMAT_Y800,

	/* 5-6-5 bit packed (1-plane) RGB (16bpp, ~5 bpe) */
	IA_CSS_DATA_FORMAT_RGB565,
	/* 24 bit RGB, 3 planes  (8 bit per element) */
	IA_CSS_DATA_FORMAT_RGB888,
	/* 32 bit RGB-Alpha, 1 plane  (8 bit per element) */
	IA_CSS_DATA_FORMAT_RGBA888,

	/* bpp bit raw, [[Gr, R];[B, Gb]] 1-plane (bpp == bpe) */
	IA_CSS_DATA_FORMAT_BAYER_GRBG,
	/* bpp bit raw, [[R, Gr];[Gb, B]] 1-plane (bpp == bpe) */
	IA_CSS_DATA_FORMAT_BAYER_RGGB,
	/* bpp bit raw, [[B, Gb];[Gr, R]] 1-plane (bpp == bpe) */
	IA_CSS_DATA_FORMAT_BAYER_BGGR,
	/* bpp bit raw, [[Gb, B];[R, Gr]] 1-plane (bpp == bpe) */
	IA_CSS_DATA_FORMAT_BAYER_GBRG,

	/* bpp bit (NV12) YUV 420, Y, UV 2-plane derived 3-line,
	 * 2-Y, 1-UV (bpp/1.5 bpe): M420 format
	 */
	IA_CSS_DATA_FORMAT_YUV420_LINE,
	/* Deprecated RAW, 1 plane */
	IA_CSS_DATA_FORMAT_RAW,
	/* Deprecated RAW, 1 plane, packed */
	IA_CSS_DATA_FORMAT_RAW_PACKED,
	/* Internal, for advanced ISP */
	IA_CSS_DATA_FORMAT_QPLANE6,
	/* 1D byte stream, used for jpeg 1-plane */
	IA_CSS_DATA_FORMAT_BINARY_8,
	/* Deprecated MIPI frame, 1D byte stream 1 plane */
	IA_CSS_DATA_FORMAT_MIPI,
	/* 12 bit [[YY];[UYVY]] 1-plane interleaved 2-line
	 * (8 bit per element)
	 */
	IA_CSS_DATA_FORMAT_MIPI_YUV420_8,
	/* 15 bit [[YY];[UYVY]] 1-plane interleaved 2-line
	 * (10 bit per element)
	 */
	IA_CSS_DATA_FORMAT_MIPI_YUV420_10,
	/* 12 bit [[UY];[VY]] 1-plane interleaved 2-line (8 bit per element) */
	IA_CSS_DATA_FORMAT_MIPI_LEGACY_YUV420_8,

	/* Type 1-5 parameter, not fragmentable */
	IA_CSS_DATA_GENERIC_PARAMETER,
	/* Video stabilisation Type 6 parameter, fragmentable */
	IA_CSS_DATA_DVS_PARAMETER,
	/* Video stabilisation Type 6 parameter, coordinates */
	IA_CSS_DATA_DVS_COORDINATES,
	/* Dead Pixel correction Type 6 parameter, fragmentable */
	IA_CSS_DATA_DPC_PARAMETER,
	/* Lens Shading Correction Type 6 parameter, fragmentable */
	IA_CSS_DATA_LSC_PARAMETER,
	/* 3A statistics output HI. */
	IA_CSS_DATA_S3A_STATISTICS_HI,
	/* 3A statistics output LO. */
	IA_CSS_DATA_S3A_STATISTICS_LO,
	/* histogram output */
	IA_CSS_DATA_S3A_HISTOGRAM,
	/* GammaStar grid */
	IA_CSS_DATA_GAMMASTAR_GRID,

	/* Gr R B Gb Gr R B Gb  in PIXELS  (also called isys interleaved) */
	IA_CSS_DATA_FORMAT_BAYER_LINE_INTERLEAVED,
	/* Gr R B Gb Gr R B Gb  in VECTORS (VCC IMAGE, ISP NWAY depentdent) */
	IA_CSS_DATA_FORMAT_BAYER_VECTORIZED,
	/* Gr R Gr R ... | B Gb B Gb ..  in VECTORS (ISP NWAY depentdent) */
	IA_CSS_DATA_FORMAT_BAYER_GRBG_VECTORIZED,

	/* 16 bit YUV 420, Y even plane, Y uneven plane,
	 * UV plane vector interleaved
	 */
	IA_CSS_DATA_FORMAT_YUV420_VECTORIZED,
	/* 16 bit YUV 420, YYUVYY vector interleaved */
	IA_CSS_DATA_FORMAT_YYUVYY_VECTORIZED,

	/* 12 bit YUV 420, Intel proprietary tiled format, TileYf */
	IA_CSS_DATA_FORMAT_NV12_TILEYF,

	/*Y samples appear first in the memory. All Y samples are array of WORDs;
	* even number of lines ;
	* Surface stride can be larger than the width of Y plane.
	* This array is followed immediately by chroma array.
	* Chroma array is an array of WORDs, with interleaved U/V samples.
	* If the interleaved U/V plane is addresses as an * array of DWORDs,
	* the least significant word contains U sample. The stride of the
	* interleaved U/V plane is equal to Y plane. 10 bit data.
	*/
	IA_CSS_DATA_FORMAT_P010,

	/* MSB aligned version of P010*/
	IA_CSS_DATA_FORMAT_P010_MSB,

	/* P016/P012 Y samples appear first in the memory.
	* All Y samples are array of WORDs;
	* even number of lines ;
	* Surface stride can be larger than the width of Y plane.
	* This array is followed immediately by chroma array.
	* Chroma array is an array of WORDs, with interleaved U/V samples.
	* If the interleaved U/V plane is addresses as an * array of DWORDs,
	* the least significant word contains U sample. The stride of the
	* interleaved U/V plane is equal to Y plane. 12 bit data.
	*/
	IA_CSS_DATA_FORMAT_P016,

	/* MSB aligned version of P016*/
	IA_CSS_DATA_FORMAT_P016_MSB,

	/* TILEYYf representation of P010*/
	IA_CSS_DATA_FORMAT_P010_TILEYF,

	/* TILEYYf representation of P010 MSB aligned*/
	IA_CSS_DATA_FORMAT_P010_MSB_TILEYF,

	/* TILEYYf representation of P016*/
	IA_CSS_DATA_FORMAT_P016_TILEYF,

	/* TILEYYf representation of P016 MSB aligned*/
	IA_CSS_DATA_FORMAT_P016_MSB_TILEYF,

        /* consists of L and R PDAF pixel pairs.
         * L and R can be interleaved or not. 1-plane (bpp == bpe) */
	IA_CSS_DATA_FORMAT_PAF,

	IA_CSS_N_FRAME_FORMAT_TYPES
} ia_css_frame_format_type_t;


#endif /* __IA_CSS_PROGRAM_GROUP_DATA_DEFS_H */
