/*===========================================================================

     This program's source code, object code, executable file(s),
     documentation and other related materials, are subject to copyright
     protection under the laws of the United States and other countries.

     This software is licensed, not sold, and is subject to the provisions
     of the Source Code License Agreement.

               Copyright(C) 1987-1994 by Knowledge Dynamics Corp
              P.O. Box 780068, San Antonio, Texas 78278
                210-979-9424 (Voice)   210-979-9004 (24-hr FAX)
                     210-979-8837 (24-hr 14,400 Baud BBS)
           CompuServe:[71333,3444]  INTERNET:71333.3444@compuserve.com

                        All rights reserved worldwide.

=============================================================================

FILENAME:
    $Workfile:   kdclib.h  $ $Logfile:   W:/master/install/kdclib.h_v  $

MASTER COPY:
	\\q\kdc

REVISION:
    $Revision:   1.6  $

LAST CHECK-IN:
    $Date:   02 Mar 1995 14:10:40  $

LAST MODIFICATION:
    $Modtime:   02 Mar 1995 14:08:58  $

AUTHOR(S):
    eric jon heflin

PUBLIC FUNCTIONS:
    n/a

LOCAL FUNCTIONS:
    n/a

AFFECTORS:
    WINDOWS			- Generate Windows 3.x compatible code
    InstantC		- Code specific to Rational Systems' Instant-C interpreter

DESCRIPTION:
	This header file is the standard header for all KDC-wide standards.

CAUTIONS:

REVISION HISTORY:
	See end of file for revision history log.

==============================================================================*/

#ifndef KDC_H
#define KDC_H

#ifdef __MICROSOFTC__
#pragma warning ( disable : 4001 )
#pragma warning ( disable : 4100 )
#pragma warning ( disable : 4115 )
#pragma warning ( disable : 4214 )
#pragma warning ( disable : 4018 )
#pragma warning ( disable : 4135 )
#define asm _asm
#endif

#ifdef WINDOWS
#define W_FAR far
#define MIXED_DATA_MODEL
#define WINVER 0x0300
#include <windows.h>
#else
#define W_FAR
#define LOWORD(l)  ((unsigned int)(unsigned long)(l))
#define LOBYTE(w)  ((char)(w))
#define HIBYTE(w)  ((char)(((unsigned int)(w) >> 8) & 0xFF))
#define HIWORD(l)  ((unsigned int)((((unsigned long)(l)) >> 16) & 0xFFFF))
#endif

// Defines for press() data compression interface.
#define PRESS_COMPRESS		(1)
#define PRESS_DECOMPRESS	(2)
#define PRESS_INIT			(4)
#define PRESS_FREE			(8)
#define PRESS_RESET			(16)
#define PRESS_SIZE			(32)
#define PRESS_SPEED			(64)

// Defines for the press() header types
#define PRESS_NONE		(1)		// file is not comressed
#define PRESS_DELETE	(9)		// file is marked for deletion
#define PRESS_AR		(11)	// file is compressed using AR compression

#	include <assert.h>

#define forever for(;;)



///////////////////////////////////////////////////////////////////////////////
//
//	Standard include files
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <setjmp.h>

///////////////////////////////////////////////////////////////////////////////
//
//	Standard typedefs
//
///////////////////////////////////////////////////////////////////////////////

// typedef for 32-bit unsigned integer
#if defined(__BORLANDC__) && defined(__OS2__)
#define far __far16
#endif

#ifdef __MICROSOFTC__
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned int word;
typedef unsigned char byte;
typedef unsigned char bool;
#else
#if (sizeof(unsigned long) == 4)
	typedef unsigned long ulong;
#elif (sizeof(unsigned int) == 4)
	typedef unsigned int ulong;
#else
	#error See KDC.H
#endif

// typedef for 16-bit unsigned integer
#if (sizeof(short) == 2)
	typedef unsigned short ushort;
#else
#	error See KDC.H
#endif

// typedef for 16-bit unsigned integer
#if (sizeof(int) == 2)
	typedef unsigned int word;
#elif (sizeof(int) == 4)
	typedef unsigned short word;
#else
#	error See KDC.H
#endif

// typedef for 8-bit unsigned integer
#if (sizeof(char) == 1)
	typedef unsigned char byte;
#else
#	error See KDC.H
#endif

// typedef for 8-bit unsigned integer boolean flag
#if (sizeof(char) == 1)
	typedef unsigned char bool;
#else
#	error See KDC.H
#endif
#endif	// _MSC_VER


typedef struct
	{
	unsigned read_only : 1;
	unsigned hidden : 1;
	unsigned system : 1;
	unsigned reserved : 1;
	unsigned directory : 1;
	unsigned archive : 1;
	unsigned reserved2 : 2;
	} file_attribute_t;

// MS-DOS or Windows
typedef struct
	{
	byte reserved[21];   // Microsoft reserved - do not change
	file_attribute_t attribute;
//    unsigned read_only : 1;
//    unsigned hidden : 1;
//    unsigned system : 1;
//    unsigned reserved : 1;
//    unsigned directory : 1;
//    unsigned archive : 1;
//    unsigned reserved2 : 10;
	word wr_time;        // time of last write to file
	word wr_date;        // date of last write to file
	ulong size;          // size of file
	byte name[13];       // asciiz name of matched file
	} file_find_t;


	/*
	 *	Defines to handle compiler-specific filename find operations.
	 */
#	define DTA_ATTR(f) ((f)->attribute)
#	define DTA_NAME(f) ((f)->name)
#	define DTA_DATE(f) ((f)->wr_date)
#	define DTA_TIME(f) ((f)->wr_time)
#	define DTA_SIZE(f) ((f)->size)

#	if defined(__MICROSOFTC__) || defined(InstantC)
#			include <direct.h>
#			include <dos.h>
		/* this is for MSC 6.00 */
#		define USE_PROTOTYPES
#	elif defined(__TURBOC__)
#		include <dir.h>
#			include <dos.h>
#		undef DRIVE
#	endif



///////////////////////////////////////////////////////////////////////////////
//
//	Predefined constants
//
///////////////////////////////////////////////////////////////////////////////

#ifndef WINDOWS
#define TRUE			(1)
#define FALSE			(0)
#endif	// !WINDOWS

//	Define an equivalent of NULL for function pointers.  We can't use
//	NULL since the sizeof(byte *) is not always equal to sizeof(function *)
#define NULL_FUNCTION	(0)


#define _1K (1024U)
#define _2K (2*1024U)
#define _4K (4*1024U)
#define _8K (8*1024U)
#define _16K (16*1024U)
#define _32K (32*1024U)


#define _0000B 	(0U)
#define _0001B 	(1U)
#define _0010B 	(2U)
#define _0011B	(3U)
#define _0100B	(4U)
#define _0101B	(5U)
#define _0110B	(6U)
#define _0111B	(7U)
#define _1000B	(8U)
#define _1001B	(9U)
#define _1010B	(10U)
#define _1011B	(11U)
#define _1100B	(12U)
#define _1101B	(13U)
#define _1110B	(14U)
#define _1111B	(15U)


// ALTERNATES
#define _0B 	(_0000B)
#define _1B 	(_0001B)
#define _10B 	(_0010B)
#define _11B	(_0011B)
#define _100B	(_0100B)
#define _101B	(_0101B)
#define _110B	(_0110B)
#define _111B	(_0111B)


#define _00B 	(_0000B)
#define _01B 	(_0001B)
#define _010B 	(_0010B)
#define _011B	(_0011B)

#define _000B 	(_0000B)
#define _001B 	(_0001B)

/*
 *	Functions to help ensure code coverage by setting various debugger
 *	breakpoints.  Based on work of David Theilan.
 */
#if !defined(NDEBUG) && !defined(OS2_TARGET)
#define IntTrap asm int 3
#define IntTrapIf(x) if(x) asm int 3
#define IntTrapIfNot(x) if(!(x)) asm int 3
#define IntTrapElse else asm int 3
#else
#define IntTrap 
#define IntTrapIf(x)
#define IntTrapIfNot(x)
#define IntTrapElse else
#endif

/*
 *	The KDC Library (c) 1990-93 Knowledge Dynamics Corporation
 *
 *	Header file for the Knowledge Dynamics Corporation Function Library
 */



///////////////////////////////////////////////////////////////////////////////
//
//	Function prototypes and macros
//
///////////////////////////////////////////////////////////////////////////////

#ifdef WINDOWS
	#include <string.h>
	#include <stdlib.h>
	#ifndef __MICROSOFTC__
	#include <mem.h>
	#endif
#endif	// WINDOWS



//////////////////////////////////////////////////////////////////////////////
//
//	Function prototypes section
//
//////////////////////////////////////////////////////////////////////////////


// command.c
//extern void init_command(word c, byte **v);
//extern int get_command(void);
//extern void get_string(const byte *prompt, byte *buf, word buf_len);

// crc.c
word ECRCMem(unsigned short W_FAR *icrc, byte W_FAR *icp, word icnt);
word ECRCHandle(ushort W_FAR *ret_crc, int fh, long file_size, word buf_size, byte W_FAR *buf);
word ECRCFname(ushort W_FAR *ret_crc, byte W_FAR *filename, word buf_size, byte W_FAR *buf);

// debug.c
void i_debug_output(const byte W_FAR *file, word line, const byte W_FAR *msg, ...);
#ifdef NDEBUG
#define do1(a)
#define do2(a,b)
#define do3(a,b,c)
#define do4(a,b,c,d)
#else
#define do1(a) i_debug_output(__FILE__,__LINE__,(a))
#define do2(a,b) i_debug_output(__FILE__,__LINE__,(a),(b))
#define do3(a,b,c) i_debug_output(__FILE__,__LINE__,(a),(b),(c))
#define do4(a,b,c,d) i_debug_output(__FILE__,__LINE__,(a),(b),(c),(d))
#endif


// dos.c
extern word int80x86r(const word call, const word ax, const word bx, const word cx, const word dx);
extern word int80x86(const word call, const union REGS W_FAR *input_regs, union REGS W_FAR *output_regs, struct SREGS W_FAR *seg_regs);
extern word fp_off(const void W_FAR *fp);
extern word fp_seg(const void W_FAR *fp);
extern void W_FAR *mk_fp(const word segment, const word offset);
extern unsigned bios_disk_fn(word function, word drive, word head, word track, word sector, word num_secs, byte W_FAR *buf);
extern unsigned smart_bios_disk(word function, word drive, word head, word track, word sector, word num_secs, byte W_FAR *buf, word max, byte W_FAR *text);

// dosprob.c
extern byte W_FAR *dosprob(word W_FAR *type);

// drive.c
extern void init_drive_table(void);
#ifndef __TURBOC__
extern int getdisk(void);
extern int setdisk(byte drive);
#endif
extern long disk_size(byte drive);
extern long disk_free(byte drive);
extern int ckremote(int drive);
extern int ckremovable(int drive);

// EGetOSEr.c
byte W_FAR *EGetOSErrorMsg(word W_FAR *type);


// EMem.c
#ifdef DEBUG_MEM
word __EMemCallocMinMax(void W_FAR * W_FAR *ret_buf, word min, word max, word W_FAR *ret_size, const byte W_FAR *msg, byte W_FAR *file, word line);
word __EMemCalloc(void W_FAR * W_FAR *ret_buf, word num, word size, const byte W_FAR *msg, byte W_FAR *file, word line);
void __EMemPerm(void W_FAR * W_FAR *ret_buf, const byte W_FAR *s, byte W_FAR *file, word line);
void __EMemFree(void W_FAR * W_FAR *data, byte W_FAR *file, word line);
word __EMemSize(const void W_FAR *data, ulong W_FAR *size);
void __EMemCheck(const void W_FAR *data, byte W_FAR *file, word line);
void __EHeapCheck(byte W_FAR *file, word line);
void __EHeapFree(byte W_FAR *file, word line);
void __EHeapWalk(byte W_FAR *file, word line);
#else	// !DEBUG_MEM
word __EMemCallocMinMax(void W_FAR * W_FAR *ret_buf, word min, word max, word W_FAR *ret_size, const byte W_FAR *msg);
word __EMemCalloc(void W_FAR * W_FAR *ret_buf, word num, word size, const byte W_FAR *msg);
void __EMemPerm(void W_FAR * W_FAR *ret_buf, const byte W_FAR *s);
void __EMemFree(void W_FAR * W_FAR *data);
word __EMemSize(const void W_FAR *data, ulong W_FAR *size);
void __EMemCheck(const void W_FAR *data);
void __EHeapCheck(void);
void __EHeapFree(void);
void __EHeapWalk(void);
#endif	// !DEBUG_MEM
#ifdef DEBUG_MEM
#define EMemCallocMinMax(ret_buf,min,max,ret_size,msg) __EMemCallocMinMax((void W_FAR * W_FAR *)ret_buf,min,max,ret_size,msg,(byte W_FAR *)__FILE__,__LINE__)
#define EMemCalloc(ret_buf,num,size,msg) __EMemCalloc((void W_FAR * W_FAR *)ret_buf,num,size,msg,(byte W_FAR *)__FILE__,__LINE__)
#define EMemPerm(ret_buf,s) __EMemPerm((void W_FAR * W_FAR *)ret_buf,s,(byte W_FAR *)__FILE__,__LINE__)
#define EMemFree(data) __EMemFree((void W_FAR * W_FAR *)&data,(byte W_FAR *)__FILE__,__LINE__)
#define EMemSize(data,ret_size) __EMemSize(data,&ret_size)
#define EMemCheck(data) __EMemCheck(data,(byte W_FAR *)__FILE__,__LINE__)
#define EHeapCheck() __EHeapCheck((byte W_FAR *)__FILE__,__LINE__)
#define EHeapFree() __EHeapFree((byte W_FAR *)__FILE__,__LINE__)
#define EHeapWalk() __EHeapWalk((byte W_FAR *)__FILE__,__LINE__)
#else	// !DEBUG_MEM
#define EMemCallocMinMax(ret_buf,min,max,ret_size,msg) __EMemCallocMinMax((void W_FAR * W_FAR *)ret_buf,min,max,ret_size,msg)
#define EMemCalloc(ret_buf,num,size,msg) __EMemCalloc((void W_FAR * W_FAR *)ret_buf,num,size,msg)
#define EMemPerm(ret_buf,s) __EMemPerm((void W_FAR * W_FAR *)ret_buf,s)
#define EMemFree(data) __EMemFree((void W_FAR * W_FAR *)&data)
#define EMemSize(data,ret_size) __EMemSize(data,&ret_size)
#define EMemCheck(data) __EMemCheck(data)
#define EHeapCheck() __EHeapCheck()
#define EHeapFree() __EHeapFree()
#define EHeapWalk() __EHeapWalk()
#endif	// !DEBUG_MEM
#define EMemClear(struct) EMemSet(&(struct), 0, sizeof(struct))

#ifdef WINDOWS
// EMsgBox.c
int FAR PASCAL EMessageBox(HWND hwnd, LPSTR lpcszStr, LPSTR lpcszTitle,	word fuStyle, ulong fuStyleEx, LPSTR iostring, WNDPROC lpfMsgProc);
int WinGetEdit(word min_row, word min_col, word max_row, word max_col, word str_max, int type, byte W_FAR *title, byte W_FAR *text, byte W_FAR *iostring);

// defines for EMessageBox()
#define MB_EX_HELP 		(1)
#define MB_EX_DEFBUTTON4 (2)

// WARNING: All edit-box types must be together and must have a corresponding
// edit mask defined.
#define MB_EX_STRING 	(8)
#define MB_EX_PRINT		(16)	// get any printable chars
#define MB_EX_BASE10	(32)	// get 0-9
#define MB_EX_PATH		(64)	// fully specified file path
#define MB_EX_ALNUM		(128)	// alphanumerics only
#define MB_EX_ALPHA		(256)	// a-zA-Z only
#define MB_EX_BASE16	(512) // 0-9, a-f, A-F
#define MB_EX_PATHND	(1024)	// path w/o a drive letter
// end of edit-box types

// This mask must include all edit-box types
#define MB_EX_EDIT_MASK	(\
	MB_EX_STRING | \
	MB_EX_PRINT | \
	MB_EX_BASE10 | \
	MB_EX_PATH | \
	MB_EX_ALNUM | \
	MB_EX_ALPHA	| \
	MB_EX_BASE16 | \
	MB_EX_PATHND)

// list types
#define MB_EX_LIST		(2048)

// control "ID" for the help button
#define IDEXHELP		(300)
#endif	// WINDOWS

// error.c
extern kdclib_error(byte W_FAR *string, ...);
extern kdclib_warning(byte W_FAR *string, ...);

// EString.c
#ifdef WINDOWS
#define ESPrintf wsprintf
#define EVSPrintf wvsprintf
#define ES
#else
#define ESPrintf sprintf
#define EVSPrintf vsprintf
#endif
extern void W_FAR *EMemChr(const void W_FAR *s, const byte c, const size_t n);
extern long EStrToLong(const byte W_FAR *str);
extern void W_FAR *EMemMove(void W_FAR *dest, const void W_FAR *src, const size_t maxlen);
extern void W_FAR *EMemCpy(void W_FAR *dest, const void W_FAR *src, const size_t maxlen);
extern byte W_FAR *EStrLwr(byte W_FAR *in);
extern byte W_FAR * EStrUpr(byte W_FAR *in);
extern size_t EStrLen(const byte W_FAR *s);
extern byte W_FAR *EStrCpy(byte W_FAR *dest, const byte W_FAR *src);
extern byte W_FAR *EStrCpyLen(byte W_FAR *dest, const byte W_FAR *src, const size_t maxlen);
extern byte W_FAR *EStrCat(byte W_FAR *dest, const byte W_FAR *src);
extern byte W_FAR *EStrCatLen(byte W_FAR *dest, const byte W_FAR *src, const size_t maxlen);
extern void W_FAR *EMemSet(void W_FAR *src, const int c, const size_t n);
extern byte W_FAR *EStrChr(const byte W_FAR *s, const byte c);
extern byte W_FAR *EStrChrRev(const byte W_FAR *s, const int c);
extern int EStrCmp(const byte W_FAR *s1, const byte W_FAR *s2);
extern int EStrCmpNos(const byte W_FAR *s1, const byte W_FAR *s2);
extern int EStrCmpLen(const byte W_FAR *s1, const byte W_FAR *s2, const size_t maxlen);
extern int EStrCmpLenNos(const byte W_FAR *s1, const byte W_FAR *s2, const size_t maxlen);
extern byte W_FAR *EStrRev(byte W_FAR *s);
extern byte W_FAR *EStrTok(byte W_FAR *s1, const byte W_FAR *s2);
extern byte W_FAR *EStrStr(const byte W_FAR *s1, const byte W_FAR *s2);
extern size_t EStrSpn(const byte W_FAR *s1, const byte W_FAR *s2);
extern size_t EStrSpnNot(const byte W_FAR *s1, const byte W_FAR *s2);
extern byte W_FAR *EStrBrk(const byte W_FAR *s1, const byte W_FAR *s2);

// exists.c
extern bool EFileExists(const byte W_FAR *file);
extern bool EDirExists(const byte W_FAR *file);
extern long s_copy(byte W_FAR *from, byte W_FAR *to);
#define s_copyh(from, to)	(s_copy_handles_len(from, to, -1L))
extern long s_copyhlen(int from, int to, long length);
extern unsigned long getfilet(int fh);
extern int chgfilet(int fh, unsigned long ftime);
extern long findsize(const byte W_FAR *path);
extern void parse_file(byte W_FAR *file, byte W_FAR *drive, byte W_FAR *path, byte W_FAR *node, byte W_FAR *ext);
extern byte W_FAR *findfile(byte W_FAR *path);
extern void touch(char W_FAR *filename);
extern byte W_FAR *fixslashes(byte W_FAR *filename);
extern void make_rel_abs(byte W_FAR *filename, int length, byte W_FAR *cwd);
extern void make_abs_rel(byte W_FAR *filename, int length, byte W_FAR *cwd);
//extern void mkpath(byte drive, byte *path);

// format.c
//extern int check_format_disk(byte drive);

// int24.c

// press.c
word press(
	word sub_function,
	word bias,
	int in_handle, int out_handle,
	// was word W_FAR ...
	word (*get_in_handle)(
							int W_FAR *handle,
							ulong W_FAR *pr_max_read,
							ulong W_FAR *origsize,
							word W_FAR *ar_last_handle_this_file),
	// was word W_FAR ...
	word (*get_out_handle)(
							int W_FAR *out_file,
							ulong orig_size,
							ulong comp_size,
							ulong W_FAR *max_write),
	void (*caller_warning_fn)(const byte W_FAR *,...),
	void (*caller_error_fn)(const byte W_FAR *,...),
	void (*caller_percent_fn)(const signed int percent, const ulong numerator, const ulong denominator),
	ulong caller_max_input_bytes, ulong caller_max_output_bytes,
	ulong W_FAR *bytes_read, ulong W_FAR *bytes_written,
	word last_fragment_this_file,
	bool caller_use_crcs,
	bool caller_return_errors
	);


// raw.c
ulong raw_overhead(ulong data_size);
int raw_in(void);
int raw_out(int c);
word update_raw_in(ulong caller_read_this_file_max);
word open_raw_in
	(
	int caller_in_fh,	// input handle
	byte W_FAR *caller_in_buf_ptr,	// in buf pointer
	word caller_in_buf_size,	// size of above
	ulong caller_read_this_file_max,	// max input size for all disks for this file
	ulong caller_read_this_handle_max,	// max read 1st segment of this file
	void (*caller_warning_fn)(const byte W_FAR *, ...),	// function called for non-fatal error
	void (*caller_error_fn)(const byte W_FAR *, ...),	// function called for fatal error
	// was word W_FAR (*caller_get_next_in_handle_fn)(int W_FAR *in_handle, ulong W_FAR *max_read_this_handle, ulong W_FAR *max_read_this_file, word W_FAR *last_file_this_disk),	// called when need an in disk
	word (*caller_get_next_in_handle_fn)(int W_FAR *in_handle, ulong W_FAR *max_read_this_handle, ulong W_FAR *max_read_this_file, word W_FAR *last_file_this_disk),	// called when need an in disk
	bool caller_use_crcs,	// TRUE if caller wants CRC checked in data
	bool caller_return_errors,	// TRUE if caller to raw_in() will handle errors
	jmp_buf caller_jmp_buf	// Location to longjmp() to if error and above is TRUE
	);
word open_raw_out
	(
	int caller_out_fh,		// output handle
	byte W_FAR *caller_out_buf_ptr,	// io buf storage, must be allocated
	word caller_out_buf_size,	// size of above
	ulong caller_write_this_file_max, // max out bytes (needed for incompressable files)
	ulong caller_write_this_handle_max, // max out bytes before 1st new handle
	void (*caller_warning_fn)(const byte W_FAR *, ...),	// function called for non-fatal error
	void (*caller_error_fn)(const byte W_FAR *, ...),	// function called for fatal error
	// was word W_FAR (*caller_get_next_out_handle_fn)(),	// called when need next out disk
	word (*caller_get_next_out_handle_fn)(),	// called when need next out disk
	bool caller_use_crcs,	// TRUE if caller wants CRC checked in data
	bool caller_return_errors	// TRUE if caller to raw_in() will handle errors
	);
word close_raw_in(void);
word close_raw_out(void);
word get_in_pos(long W_FAR *long_ptr);
word get_out_pos(long W_FAR *long_ptr);
word raw_out_flush(void);


// recycle.c
extern FILE W_FAR *recycle_in;
extern FILE W_FAR *recycle_out;
extern void ropen(byte W_FAR *file_name);
extern void rclose(void);


// specific.c
word EFileAttrGet(file_attribute_t W_FAR *attr, const byte W_FAR *path, const byte W_FAR *emsg);
word EFileAttrSet(const file_attribute_t attr, const byte W_FAR *path, const byte W_FAR *emsg);
void parse_file(byte W_FAR *file, byte W_FAR *drive, byte W_FAR *path, byte W_FAR *node, byte W_FAR *ext);
int getdisk(void);
//int setdisk(byte drive);
unsigned long getfilet(int fh);
int chgfilet(int fh, unsigned long ftime);
int smart_chgfilet(int fh, ulong ftime, const byte W_FAR *fname, const byte W_FAR *emsg);
unsigned int80x86r(const word call, const word ax, const word bx, const word cx, const word dx);
word int80x86(const word call, const union REGS W_FAR *in_regs, union REGS W_FAR *out_regs, struct SREGS W_FAR *seg_regs);
word fp_off(const void W_FAR *fp);
word fp_seg(const void W_FAR *fp);
void W_FAR *mk_fp(const unsigned segment, const unsigned offset);
word EFileSizeGet(long W_FAR *fsize, const byte W_FAR *path);
word EFileCopy(long W_FAR *bytes_copied, const byte W_FAR *from, const byte W_FAR *to, const byte W_FAR *emsg);
word EHandleCopy(long W_FAR *bytes_copied, int from, int to);
word EHandleCopyLenCRCs(long W_FAR *bytes_copied, int from, int to, long in_file_size, bool use_crcs);
word EFindFirst(const byte W_FAR *path, file_find_t W_FAR *f, const file_attribute_t attrib);
word EFindNext(file_find_t W_FAR *f);
word EGetCWD(byte W_FAR * W_FAR *ret_addr, byte W_FAR *buf, size_t size, const byte W_FAR *emsg);
word EFileRename(const byte W_FAR *oldname, const byte W_FAR *newname, const byte W_FAR *emsg);
word EFileRemove(const byte W_FAR *fname, const byte W_FAR *emsg);
word EChDir(const byte W_FAR *dir, const byte W_FAR *emsg);
word EGetEnv(byte W_FAR * W_FAR *value, const byte W_FAR *variable);
word EGetEnv(byte W_FAR * W_FAR *value, const byte W_FAR *variable);
word ERmDir(const byte W_FAR *dir, const byte W_FAR *emsg);
word EMkDir(const byte W_FAR *dir, const byte W_FAR *emsg);
int EGetCDROMInfo(int function, byte letter);
word EDirFind(const bool new_search, const byte W_FAR *pattern, file_find_t W_FAR *file_struct, const file_attribute_t fa, byte W_FAR *matching_fname);


// string.c
extern int substrcpy(byte W_FAR *destination, byte W_FAR *source);
extern void trimend(byte W_FAR *s);
extern void trimfront(byte W_FAR *s);
extern void trimdir(byte W_FAR *directory);
extern byte W_FAR *deformat(byte W_FAR *s);
extern int ispath(int c);


// suio.c
extern int s_ioerror(int newvalue);
extern int EOpen(const byte W_FAR *name, unsigned mode, unsigned access, const byte W_FAR *emsg);
extern word ECloseFH(int W_FAR *fh, const byte W_FAR *filename, const byte W_FAR *emsg);
//extern word EClose(int fh, const byte W_FAR *emsg);
//extern word ERead(int file, void W_FAR *work_buf, ulong len, const byte W_FAR *emsg);
extern word ERead(word W_FAR *bytes_read, int in, void W_FAR *buf, ulong size, const byte W_FAR *emsg);
extern word EWrite(int file, void W_FAR *work_buf, ulong len, const byte W_FAR *emsg);
extern long ELSeek(int file, long offset, int mode, const byte W_FAR *emsg);
extern word EFGets(byte W_FAR *inbuf, ulong bufsize, FILE W_FAR *file, const byte W_FAR *emsg);
extern word EFPuts(const byte W_FAR *outbuf, FILE W_FAR *outfp, const byte W_FAR *emsg);
extern void EFPrintf(const byte W_FAR *emsg, FILE W_FAR *outfp, const byte W_FAR *fmt, ...);
extern word ESeekFH(long W_FAR *new_pos, int fh, long pos, int mode, const byte W_FAR *filename, const byte W_FAR *emsg);


// time.c
extern void time_delay(word ticks);


// wildcard.c
extern int pattern_cmp(byte W_FAR *node_pattern, byte W_FAR *ext_pattern, byte W_FAR *node, byte W_FAR *ext);
extern int iswild(byte W_FAR *s);
extern byte W_FAR *expand_wild(byte W_FAR *wild, byte W_FAR *notwild);


// xsystem.asm
word xspawnl(int, byte W_FAR *, byte W_FAR *,...);
word xspawnle(int, byte W_FAR *, byte W_FAR *,...);
word xspawnlp(int, byte W_FAR *, byte W_FAR *,...);
word xspawnlpe(int, byte W_FAR *, byte W_FAR *,...);
word xspawnv(int, byte W_FAR *, byte W_FAR * W_FAR *);
word xspawnve(int, byte W_FAR *, byte W_FAR * W_FAR *, byte W_FAR * W_FAR *);
word xspawnvp(int, byte W_FAR *, byte W_FAR * W_FAR *);
word xspawnvpe(int, byte W_FAR *, byte W_FAR * W_FAR *, byte W_FAR * W_FAR *);
word xsystem(byte W_FAR *);


// yieldarg.c
extern void init_yieldarg(int argc, char W_FAR * W_FAR *argv);
extern byte W_FAR *yieldarg(void);


//////////////////////////////////////////////////////////////////////////////
//
//	End of function prototypes section
//
//////////////////////////////////////////////////////////////////////////////

#endif /* KDC_H */

/*
 * Place your revision control comments here.
 */

