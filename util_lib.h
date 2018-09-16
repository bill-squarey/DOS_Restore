/* UTIL_lib.h

W.R.S. Webber Ph.D.
15 September 2018

*/

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include <time.h>
#include <io.h>
#include <errno.h>

#ifndef FILE_EXIST
/* for use with _access() */
#define FILE_EXIST  0
#define FILE_WRITE  2
#define FILE_READ   4
#define FILE_RD_WRT 6
#endif

// Used for mode in:
// time_date( char **time_str, char **date_str, int mode );
// time_date2( time_t jsec_1970, char **time_str, char **date_str, int mode );
#define TD_LEAD_ZERO    0x01 // hh:0m:0s else hh:m:s
#define TD_HR_LEAD_ZERO 0x02 // hh:mm:ss else 0h:mm
#define TD_YR_LEAD_ZERO 0x04 // dd/mm/0y else dd/mm/y
#define TD_SECONDS      0x08 // hh:mm:ss else hh:mm
#define TD_24HOUR       0x10 // time 17:23 else 5:23P
#define TD_YEAR_4DIG    0x20 // 2001 shown as 2001 else 01
#define TD_DATE_HYPHEN  0x40 // date mm-dd-yy else mm/dd/yy
#define TD_MONTH_UK     0x80 // dd/mm/yy else USA mm/dd/yy

#define _USE_32BIT_TIME_T 1 // VC++ 2010 Express

#define TIME_DATE_TEXT_SIZE 14

#define PATH_LEN 63
#define FILE_LEN 12
#define FILE_LEN_LONG 1024
#define SIG_LEN   8

typedef unsigned long   UI32;
typedef unsigned short  UI16;
typedef unsigned char   UI8;

typedef long            SI32;
typedef short           SI16;
typedef char            SI8;

typedef float           FLT32;
typedef double          FLT64;

// CONTROL.001 Record for DOS 8.3 files 
typedef struct Rest_Header {
    UI8 hd;
    char sig[ SIG_LEN ];
    UI8 seq;
    char unknown[ 128 ];
    UI8 last;

    } REST_HEADER;

// CONTROL.001 Record for DOS 8.3 names
/* Based on:
BACKUP & RESTORE for the FreeDOS project

Ralf Quint. The Pacific Coast Byte Factory

Los Angeles, California, U.S.A
eMail:Ralf@aztechvision.net
http://members.xoom.com/PCByteFactory/

Written for the FreeDOS project http://www.freedos.org/

From:
https://www.ibiblio.org/pub/micro/pc-stuff/freedos/files/dos/backup/brtecdoc.htm
http://ldp.rediris.es/sites/freedos.org/files/dos/backup/brtecdoc.htm
*/

typedef union Dir_File {
    struct {
        UI8 blk_size;          // 0x46, length of the entry record, including this byte
        char path[ PATH_LEN ]; // Directory path name (zero padded ?), root is 0x00
        short nr;              // 1 word, number entries in this directory
        long end;              // 0xFFFFFFFF
        } dr_rec;

    struct {
        UI8 blk_size;          // 0x22, length of the entry record, including this byte
        char name[ FILE_LEN ]; // File name
        UI8 split_flg;         // Unknown, Flag for complete (03h) or split (02h) file
        long f_size;           // Original file size
        UI16 seq;    // Sequence/part of the backup file (1= first/complete, 2,3..=part of the file
        UI32 f_os;   // Offset into BACKUP.001 File
        UI32 len;    // Saved length in the BACKUP.001 File
        UI8  att;    // File attributes
        UI8  flg;    // Unknown
        UI16 f_time; // Packed DOS Time
        UI16 f_date; // Packed DOS Date
        } fl_rec;

    } DIR_FILE;


// CONTROL.001 Record for OS/2 Long file names 
/* Reverse Engineered by W.R.S. Webber Ph.D. August 2018 */
typedef union Dir_File32 {
    struct {
        // UI8 blk_size; Not used because dr_rec and fl_rec are variable size in file. Different read methods used.
        UI8 pad1[ 7 ]; // Padding Unknown meaning

        UI8 dir_flag;
        UI8 pad2[ 10 ];  // Padding Unknown meaning

        char path[ FILE_LEN_LONG ];
        } dr_rec;

    struct {
        // UI8 blk_size;
        UI8 pad1[ 7 ];  // Padding Unknown meaning

        UI8 dir_flag; // If non Zero is Directory

        UI32 f_os;      // File Offset in BACKUP.001
        UI32 f_size;    // File size in Bytes
        UI8  pad2[ 2 ]; // Padding Unknown meaning

        UI16 f_time;    // Packed DOS Time
        UI16 f_date;    // Packed DOS Date

        UI8 pad3[ 10 ];  // Padding Unknown meaning

        char name[ FILE_LEN_LONG ]; // OS/2 Long Name i.e. can be greater than 12 Bytes (8.3)
        } fl_rec;

    } DIR_FILE32;

enum {
	CL_OPT_A = 0,
	CL_OPT_B,
	CL_OPT_C,
	CL_OPT_D,
	CL_OPT_E,
	CL_OPT_F,
	CL_OPT_G,
	CL_OPT_H,
	CL_OPT_I,
	CL_OPT_J,
	CL_OPT_K,
	CL_OPT_L,
	CL_OPT_M,
	CL_OPT_N,
	CL_OPT_O,
	CL_OPT_P,
	CL_OPT_Q,
	CL_OPT_R,
	CL_OPT_S,
	CL_OPT_T,
	CL_OPT_U,
	CL_OPT_V,
	CL_OPT_W,
	CL_OPT_X,
	CL_OPT_Y,
	CL_OPT_Z
	};

enum {
	CL_OPT_a = 0,
	CL_OPT_b,
	CL_OPT_c,
	CL_OPT_d,
	CL_OPT_e,
	CL_OPT_f,
	CL_OPT_g,
	CL_OPT_h,
	CL_OPT_i,
	CL_OPT_j,
	CL_OPT_k,
	CL_OPT_l,
	CL_OPT_m,
	CL_OPT_n,
	CL_OPT_o,
	CL_OPT_p,
	CL_OPT_q,
	CL_OPT_r,
	CL_OPT_s,
	CL_OPT_t,
	CL_OPT_u,
	CL_OPT_v,
	CL_OPT_w,
	CL_OPT_x,
	CL_OPT_y,
	CL_OPT_z
	};

enum {
	CL_OPT_0 = 0,
	CL_OPT_1,
	CL_OPT_2,
	CL_OPT_3,
	CL_OPT_4,
	CL_OPT_5,
	CL_OPT_6,
	CL_OPT_7,
	CL_OPT_8,
	CL_OPT_9
	};
extern void set_cl_options( char *AZU, char *azl, char *d09, char *opt );

extern void clear_cl_options( char *AZU, char *azl, char *d09 );

extern void time_date2( time_t jsec_1970, char **time_str, char **date_str, int mode );

extern char *time_date1( time_t jsec_1970, int mode );

extern char *end_slash( char *in );

extern char *join_path2( char *base, char *in );

extern char *new_version_file_name( char *old_fn, int *tries );

extern time_t DOS_dttm_to_time( UI16 dt, UI16 tm );

extern int make_dir_path( char *out );

extern write_file( FILE *fp_bk, char *fn, UI32 f_os, UI32 f_size, time_t tmt );

// End of File
