#ifndef	M70_H
#define	M70_H

/*
** NAME
**	m70.h -- header file for IIS model 70
**
** SYNOPSIS
**	#include "m70.h"
**
** DESCRIPTION
**
** FUTURE DIRECTIONS
**
** BUGS
*/

#ifndef	bit
#define	bit(i)			( 1 << (i) )
#endif

#ifndef	mask
#define	mask(i)			( ~(~0 << (i)) )
#endif

#define	M70_DEV			"/dev/ga0"

typedef short m70word_t;		/* basic integer type		 */
typedef float m70real_t;		/* basic floating-point type	 */
/*
/*
 * I/O header
 */

typedef struct {
	m70word_t       transfer_id;
	m70word_t       thing_count;
	m70word_t       subunit_select;
	m70word_t       checksum;
	m70word_t       x_register;
	m70word_t       y_register;
	m70word_t       z_register;
	m70word_t       t_register;
} M70HDR_T;

/*
 * transfer ID
 */
#define	M70_32BIT_MUX_MODE	bit(7)
#define	M70_VERTICAL_RETRACE	bit(8)
#define	M70_BLOCK_TRANSFER	bit(9)
#define	M70_ACCUM_CHAN_MODE	bit(10)
#define	M70_ADDITIVE_WRITE	bit(11)
#define	M70_BYTE_MODE		bit(12)
#define	M70_BYPASS_IFM		bit(13)
#define	M70_PACKED_MODE		bit(14)
#define	M70_READ		bit(15)

#define	M70_USE_IFM		0	/* ~ M70_BYPASS_IFM		 */

/*
 * thing count
 */
#define	M70_THING_COUNT_MAX	32768

/*
 * subunit select
 */
#define	M70_REFRESH		1	/* refresh memory		 */
#define	M70_LUT			2	/* look-up tables; split screen	 */
#define	M70_OFM			3	/* output function memory	 */
#define	M70_IFM			4	/* input function memory	 */
#define	M70_FEEDBACK		5
#define	M70_SCROLL		6
#define	M70_VIDEOMETER		7
#define	M70_SUM			8	/* sum processor		 */
#define	M70_GRAPHICS		9
#define	M70_CURSOR		10	/* programmable cursor		 */
#define	M70_ALU			11	/* arithmetic/logic unit	 */
#define	M70_ZOOM		12

#define	M70_COMMAND_TRANSFER	bit(15)

/*
/*
 * x register
 */
#define	M70_ADV_X_ON_Y_OVFLO	bit(14)	/* advance X on Y overflow	 */
#define	M70_ADV_X_ON_THING_CNT	bit(15)	/* advance X on thing count adv	 */

#define	M70_N_X_BITS		9

#define	m70_x_part(i)		( (i) & mask(M70_N_X_BITS) )

/*
 * y register
 */
#define	M70_ADV_Y_ON_THING_CNT	bit(14)	/* advance Y on thing count adv	 */
#define	M70_ADV_Y_ON_X_OVFLO	bit(15)	/* advance Y on X overflow	 */

#define	M70_N_Y_BITS		9

#define	m70_y_part(i)		( ((i) >> M70_N_X_BITS) & mask(M70_N_Y_BITS) )

/*
 * miscellany
 */
#define	M70_N_X			(mask(M70_N_X_BITS) + 1)
#define	M70_N_Y			(mask(M70_N_Y_BITS) + 1)

#define	M70_GRAPHICS_CHANNEL	bit(15)
#define	M70_N_GRAPHICS_PLANES	4

#define	M70_N_IMAGE_CHANNELS	4
#define	M70_N_IMAGE_PLANES	8

#define	M70_BLUE		bit(0)
#define	M70_GREEN		bit(1)
#define	M70_RED			bit(2)
/*
/*
 * refresh memory
 */

#define	m70rrefresh(fd, options, n, x, y, channels, planes, data)	\
		m70io(	fd,						\
			M70_READ | (options),				\
			n,						\
			M70_REFRESH,					\
			x,						\
			y,						\
			channels,					\
			planes,						\
			(m70word_t *) data)

#define	m70wrefresh(fd, options, n, x, y, channels, planes, data)	\
		m70io(	fd,						\
			options,					\
			n,						\
			M70_REFRESH,					\
			x,						\
			y,						\
			channels,					\
			planes,						\
			(m70word_t *) data)
/*
/*
 * channel select
 */

typedef struct {
	struct {
		m70word_t       quadrant[4];
	}               blue, green, red;
} M70SELECT_T;

#define	m70rselect(fd, n, start, data)					\
		m70io(	fd,						\
			M70_READ,					\
			n,						\
			M70_LUT | M70_COMMAND_TRANSFER,			\
			start,						\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)

#define	m70wselect(fd, n, start, data)					\
		m70io(	fd,						\
			0,						\
			n,						\
			M70_LUT | M70_COMMAND_TRANSFER,			\
			start,						\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)
/*
/*
 * split screen
 */

typedef struct {
	m70word_t       x;
	m70word_t       y;
} M70SPLIT_T;

#define	m70rsplit(fd, n, start, data)					\
		m70io(	fd,						\
			M70_READ,					\
			n,						\
			M70_LUT | M70_COMMAND_TRANSFER,			\
			(start) + 12,					\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)

#define	m70wsplit(fd, n, start, data)					\
		m70io(	fd,						\
			0,						\
			n,						\
			M70_LUT | M70_COMMAND_TRANSFER,			\
			(start) + 12,					\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)
/*
/*
 * lookup table
 */

#define	M70_LUT_DIM		256

typedef m70word_t M70LUT_T[M70_LUT_DIM];

#define	M70_LUT_MIN_VAL		(-256)
#define	M70_LUT_MAX_VAL		255

#define	m70rlut(fd, n, start, colors, channels, data)			\
		m70io(	fd,						\
			M70_READ,					\
			n,						\
			M70_LUT,					\
			start,						\
			0,						\
			colors,						\
			channels,					\
			(m70word_t *) data)

#define	m70wlut(fd, n, start, colors, channels, data)			\
		m70io(	fd,						\
			0,						\
			n,						\
			M70_LUT,					\
			start,						\
			0,						\
			colors,						\
			channels,					\
			(m70word_t *) data)
/*
/*
 * range register
 */

typedef union {
	m70word_t       word;
	struct {
#if CC_BIGENDIAN
		unsigned:       10;
		unsigned        red:2;
		unsigned        green:2;
		unsigned        blue:2;
#else
		unsigned        blue:2;
		unsigned        green:2;
		unsigned        red:2;
#endif
	}               bits;
} M70RANGE_T;

#define	m70rrange(fd, data)						\
		m70io(	fd,						\
			M70_READ,					\
			1,						\
			M70_OFM | M70_COMMAND_TRANSFER,			\
			0,						\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)

#define	m70wrange(fd, data)						\
		m70io(	fd,						\
			0,						\
			1,						\
			M70_OFM | M70_COMMAND_TRANSFER,			\
			0,						\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)
/*
/*
 * output function memory
 */
#define	M70_OFM_DIM		1024

typedef m70word_t M70OFM_T[M70_OFM_DIM];

#define	M70_OFM_MIN_VAL		0
#define	M70_OFM_MAX_VAL		1023

#define	m70rofm(fd, n, start, colors, data)				\
		m70io(	fd,						\
			M70_READ,					\
			n,						\
			M70_OFM,					\
			m70_x_part(start),				\
			m70_y_part(start),				\
			colors,						\
			0,						\
			(m70word_t *) data)

#define	m70wofm(fd, n, start, colors, data)				\
		m70io(	fd,						\
			0,						\
			n,						\
			M70_OFM,					\
			m70_x_part(start),				\
			m70_y_part(start),				\
			colors,						\
			0,						\
			(m70word_t *) data)
/*
/*
 * input function memory
 */

#define	M70_IFM_DIM		4096

typedef uchar_t M70IFM_T[M70_IFM_DIM];

#define	M70_IFM_MIN_VAL		0
#define	M70_IFM_MAX_VAL		255

#define	m70rifm(fd, n, start, data)					\
		m70io(	fd,						\
			M70_READ | M70_PACKED_MODE,			\
			n,						\
			M70_IFM,					\
			m70_x_part(start),				\
			m70_y_part(start),				\
			0,						\
			0,						\
			(m70word_t *) data)

#define	m70wifm(fd, n, start, data)					\
		m70io(	fd,						\
			M70_PACKED_MODE,				\
			n,						\
			M70_IFM,					\
			m70_x_part(start),				\
			m70_y_part(start),				\
			0,						\
			0,						\
			(m70word_t *) data)
/*
/*
 * feedback
 */

typedef union {
	m70word_t       word;
	struct {
#if CC_BIGENDIAN
		unsigned        zero_data:1;
		unsigned:       6;
		unsigned        pixel_shift:1;
		unsigned:       5;
		unsigned        ofm_select:3;
#else
		unsigned        ofm_select:3;
		unsigned:       5;
		unsigned        pixel_shift:1;
		unsigned:       6;
		unsigned        zero_data:1;
#endif
	}               bits;
} M70FEEDBACK_T;

#define	m70wfeedback(fd, ifm, channels, planes, data)			\
		m70io(	fd,						\
			M70_BLOCK_TRANSFER | M70_BYTE_MODE | (ifm),	\
			1,						\
			M70_FEEDBACK,					\
			0,						\
			0,						\
			channels,					\
			planes,						\
			(m70word_t *) data)
/*
/*
 * scroll
 */

typedef struct {
	m70word_t       x;
	m70word_t       y;
} M70SCROLL_T;

#define	m70rscroll(fd, n, start, channels, data)			\
		m70io(	fd,						\
			M70_READ,					\
			n,						\
			M70_SCROLL,					\
			start,						\
			0,						\
			0,						\
			channels,					\
			(m70word_t *) data)

#define	m70wscroll(fd, n, start, channels, data)			\
		m70io(	fd,						\
			0,						\
			n,						\
			M70_SCROLL,					\
			start,						\
			0,						\
			0,						\
			channels,					\
			(m70word_t *) data)
/*
/*
 * videometer control register
 */

#define	M70_VIDEOM_BLOTCH	0
#define	M70_VIDEOM_NOT_BLOTCH	1
#define	M70_VIDEOM_ALL		2
#define	M70_VIDEOM_EXT_BLOTCH	3

#define	m70rvctrl(fd, data)						\
		m70io(	fd,						\
			M70_READ,					\
			1,						\
			M70_VIDEOMETER | M70_COMMAND_TRANSFER,		\
			0,						\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)

#define	m70wvctrl(fd, data)						\
		m70io(	fd,						\
			0,						\
			1,						\
			M70_VIDEOMETER | M70_COMMAND_TRANSFER,		\
			0,						\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)
/*
/*
 * videometer
 */

#define	M70_HIST_DIM		1024

typedef long M70HIST_T[M70_HIST_DIM];

#define	m70rhist(fd, n, start, colors, data)				\
		m70io(	fd,						\
			M70_READ,					\
			(n) * 2,					\
			M70_VIDEOMETER,					\
			m70_x_part(start),				\
			m70_y_part(start),				\
			colors,						\
			0,						\
			(m70word_t *) data)
/*
/*
 * min/max registers
 */

typedef struct {
	struct {
		m70word_t       min;
		m70word_t       max;
	}               blue, green, red;
} M70MINMAX_T;

#define	m70rminmax(fd, n, start, data)					\
		m70io(	fd,						\
			M70_READ,					\
			n,						\
			M70_SUM | M70_COMMAND_TRANSFER,			\
			start,						\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)
/*
/*
 * constant registers
 */

#define	M70_CONST_MIN		(-4096)
#define	M70_CONST_MAX		4095

typedef struct {
	m70word_t       blue;
	m70word_t       green;
	m70word_t       red;
} M70CONST_T;

#define	m70rconst(fd, n, start, data)					\
		m70io(	fd,						\
			M70_READ,					\
			n,						\
			M70_SUM | M70_COMMAND_TRANSFER,			\
			(start) + 8,					\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)

#define	m70wconst(fd, n, start, data)					\
		m70io(	fd,						\
			0,						\
			n,						\
			M70_SUM | M70_COMMAND_TRANSFER,			\
			(start) + 8,					\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)
/*
/*
 * graphics status register
 */

typedef struct {
	m70word_t       word;
	struct {
#if CC_BIGENDIAN
		unsigned:       5;
		unsigned        disable_cursor:1;
		unsigned        disable_video:1;
		unsigned        disable_graphics:1;
		unsigned        blotch_plane_select:3;
		unsigned:       1;
		unsigned        status_plane_select:3;
		unsigned        status_video_on:1;
#else
		unsigned        status_video_on:1;
		unsigned        status_plane_select:3;
		unsigned:       1;
		unsigned        blotch_plane_select:3;
		unsigned        disable_graphics:1;
		unsigned        disable_video:1;
		unsigned        disable_cursor:1;
#endif
	}               bits;
} M70GRSTAT_T;

#define	m70rgrstat(fd, data)						\
		m70io(	fd,						\
			M70_READ,					\
			1,						\
			M70_GRAPHICS | M70_COMMAND_TRANSFER,		\
			0,						\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)

#define	m70wgrstat(fd, data)						\
		m70io(	fd,						\
			0,						\
			1,						\
			M70_GRAPHICS | M70_COMMAND_TRANSFER,		\
			0,						\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)
/*
/*
 * graphics color assignment RAM
 */

#define	M70_GCA_RAM_DIM	256

typedef m70word_t M70GCA_T[M70_GCA_RAM_DIM];

#define	m70_mk_gca(r, g, b, insert)	(				\
		 ((b) & mask(5))	|				\
		(((g) & mask(5)) << 5)	|				\
		(((r) & mask(5)) << 10)	|				\
		(((insert) & 1) << 15)	)

#define	m70rgcaram(fd, n, start, data)					\
		m70io(	fd,						\
			M70_READ,					\
			n,						\
			M70_GRAPHICS,					\
			start,						\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)

#define	m70wgcaram(fd, n, start, data)					\
		m70io(	fd,						\
			0,						\
			n,						\
			M70_GRAPHICS,					\
			start,						\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)
/*
/*
 * cursor registers
 */

typedef union {
	struct {
		m70word_t       status;
		m70word_t       x;
		m70word_t       y;
	}               words;
	struct {
#if CC_BIGENDIAN
		unsigned        cursor_moved:1;
		unsigned        button_pushed:1;
		unsigned        z_axis:1;
		unsigned        proximity:1;
		unsigned        button:4;
		unsigned        aux_function:2;
		unsigned        beeper_disable:1;
		unsigned        blink_rate:2;
		unsigned        link_tball_to_y:1;
		unsigned        link_tball_to_x:1;
		unsigned        cursor_on:1;
#else
		unsigned        cursor_on:1;
		unsigned        link_tball_to_x:1;
		unsigned        link_tball_to_y:1;
		unsigned        blink_rate:2;
		unsigned        beeper_disable:1;
		unsigned        aux_function:2;
		unsigned        button:4;
		unsigned        proximity:1;
		unsigned        z_axis:1;
		unsigned        button_pushed:1;
		unsigned        cursor_moved:1;
#endif
	}               bits;
} M70CURSOR_T;

#define	M70_BUTTON_A		bit(0);
#define	M70_BUTTON_B		bit(1);
#define	M70_BUTTON_C		bit(2);
#define	M70_BUTTON_D		bit(3);

#define	M70_CURSOR_BLINK_OFF	0
#define	M70_CURSOR_BLINK_FAST	1
#define	M70_CURSOR_BLINK_MEDIUM	2
#define	M70_CURSOR_BLINK_SLOW	3
/*
 */
#define	m70rcursor(fd, n, start, data)					\
		m70io(	fd,						\
			M70_READ,					\
			n,						\
			M70_CURSOR | M70_COMMAND_TRANSFER,		\
			start,						\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)

#define	m70wcursor(fd, n, start, data)					\
		m70io(	fd,						\
			0,						\
			n,						\
			M70_CURSOR | M70_COMMAND_TRANSFER,		\
			start,						\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)
/*
/*
 * cursor array
 */

#define	M70_CURSOR_X_DIM	64
#define	M70_CURSOR_Y_DIM	64

#define	M70_CURSOR_SIZE		( M70_CURSOR_X_DIM * M70_CURSOR_Y_DIM )

typedef m70word_t M70CARRAY_T[M70_CURSOR_Y_DIM][M70_CURSOR_X_DIM];

#define	m70_carray(x, y)	( (((y) & mask(6)) << 6) | ((x) & mask(6)) )

#define	m70rcarray(fd, n, x, y, data)					\
		m70io(	fd,						\
			M70_READ,					\
			n,						\
			M70_CURSOR,					\
			m70_x_part(m70_carray(x,y)),			\
			m70_y_part(m70_carray(x,y)),			\
			0,						\
			0,						\
			(m70word_t *) data)

#define	m70wcarray(fd, n, x, y, data)					\
		m70io(	fd,						\
			M70_READ,					\
			n,						\
			M70_CURSOR,					\
			m70_x_part(m70_carray(x,y)),			\
			m70_y_part(m70_carray(x,y)),			\
			0,						\
			0,						\
			(m70word_t *) data)
/*
/*
 * ALU control word
 */

typedef union {
	m70word_t       word;
	struct {
#if CC_BIGENDIAN
		unsigned        equal_status_flag:1;
		unsigned        carry_status_flag:1;
		unsigned        sign_extend_right_shift_output:1;
		unsigned        right_shift_output:1;
		unsigned        sign_extend_selected_ofm:1;
		unsigned        enable_blotch:1;
		unsigned        alu_carry_in:1;
		unsigned        alu_mode:1;
		unsigned        blotch_function:4;
		unsigned        normal_function:4;
#else
		unsigned        normal_function:4;
		unsigned        blotch_function:4;
		unsigned        alu_mode:1;
		unsigned        alu_carry_in:1;
		unsigned        enable_blotch:1;
		unsigned        sign_extend_selected_ofm:1;
		unsigned        right_shift_output:1;
		unsigned        sign_extend_right_shift_output:1;
		unsigned        carry_status_flag:1;
		unsigned        equal_status_flag:1;
#endif
	}               bits;
} M70ALUCTRL_T;

#define	m70raluctrl(fd, data)						\
		m70io(	fd,						\
			M70_READ,					\
			1,						\
			M70_ALU | M70_COMMAND_TRANSFER,			\
			0,						\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)

#define	m70waluctrl(fd, data)						\
		m70io(	fd,						\
			0,						\
			1,						\
			M70_ALU | M70_COMMAND_TRANSFER,			\
			0,						\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)
/*
/*
 * ALU constant array
 */

#define	M70_ALU_CONST_DIM	8

typedef m70word_t M70ALUCONST_T[M70_ALU_CONST_DIM];

#define	M70_ALU_MIN_CONST	0
#define	M70_ALU_MAX_CONST	32767

#define	m70_alu_const(k)	( ~(k) )

#define	m70raluconst(fd, n, start, data)				\
		m70io(	fd,						\
			M70_READ,					\
			n,						\
			M70_ALU,					\
			start,						\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)

#define	m70waluconst(fd, n, start, data)				\
		m70io(	fd,						\
			0,						\
			n,						\
			M70_ALU,					\
			start,						\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)
/*
/*
 * ALU output select table
 */

#define	M70_ALU_OST_DIM		8

typedef m70word_t M70ALUOST_T[M70_ALU_OST_DIM];

#define	M70_ALU_SELECT_ACCUM	8
#define	M70_ALU_SELECT_OFM	9
#define	M70_ALU_SELECT_ALU	10
#define	M70_ALU_SELECT_EXTERN	11

#define	m70_alu_select_const(k)	( (k) & mask(3) )

#define	m70raluost(fd, n, start, data)					\
		m70io(	fd,						\
			M70_READ,					\
			n,						\
			M70_ALU,					\
			(start) + 8,					\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)

#define	m70waluost(fd, n, start, data)					\
		m70io(	fd,						\
			0,						\
			n,						\
			M70_ALU,					\
			(start) + 8,					\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)
/*
/*
 * zoom
 */

typedef struct {
	m70word_t	magnification;
	m70word_t	x;
	m70word_t	y;
} M70ZOOM_T;

#define	M70_ZOOM_MIN	0
#define	M70_ZOOM_MAX	3

#define	m70rzoom(fd, n, start, data)					\
		m70io(	fd,						\
			M70_READ,					\
			n,						\
			M70_ZOOM,					\
			start,						\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)

#define	m70wzoom(fd, n, start, data)					\
		m70io(	fd,						\
			0,						\
			n,						\
			M70_ZOOM,					\
			start,						\
			0,						\
			0,						\
			0,						\
			(m70word_t *) data)
/*
/*
 * extern declarations
 */

extern int      m70io();
extern int      m70mclr();
extern int      m70open();
extern void     m70vr();

/* $Header: /usr/home/ipw/h/RCS/m70.h,v 1.5 89/08/22 19:06:48 frew Exp $ */
#endif
