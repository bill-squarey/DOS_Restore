/* UTIL_lib.c
Utility functions for DOS_Restore

W.R.S. Webber Ph.D.
15 September 2018

*/

#include "util_lib.h"

#define RD_BLK 8192

// Command line options
char Opt_az[ 26 + 1 ], OptAZ_[ 26 + 1 ],  Opt_09[ 10 + 1 ];

/*------------------------------------------------------------------------------------------*/
void set_cl_options( char *AZU, char *azl, char *d09, char *opt ) {
    if( opt == NULL ) return;     if( *opt == 0 )   return;

    while( *opt ) {
        if( *opt >= '0' && *opt <= '9' ) d09[ *opt - '0' ] = *opt;
      if( *opt >= 'A' && *opt <= 'Z' ) AZU[ *opt - 'A' ] = *opt;
      if( *opt >= 'a' && *opt <= 'z' ) azl[ *opt - 'a' ] = *opt;
      opt++;
        }

    } // set_cl_options()

/*------------------------------------------------------------------------------------------*/
void clear_cl_options( char *AZU, char *azl, char *d09 ) {
    int ii;
    for( ii = 0; ii < 26; ii++ ) AZU[ ii ] = azl[ ii ] = 0;
    for( ii = 0; ii < 9;  ii++ ) d09[ ii ] = 0;

    } // clear_cl_options()


/*------------------------------------------------------------------------------------------*/
char *lead_zero_number( int number, int nr_digits ) {
   static char out[ TIME_DATE_TEXT_SIZE  ],  nr_str[ TIME_DATE_TEXT_SIZE  ];
   char *zeros = "00000000"; // 8 zeros
   int nlen;

   strcpy( out, zeros );

   if( nr_digits > 8 ) nr_digits = 8;

   sprintf( nr_str, "%d", number );
   nlen = strlen( nr_str );
   if( nlen >= nr_digits ) { return( nr_str ); }
   else {
      strcpy( ( out + nr_digits - nlen ), nr_str );
      return( out );
      }

   } // lead_zero_number()

/*------------------------------------------------------------------------------------------*/

/*** localtime() returns a pointer to the structure result.
If the value in timer represents a date before midnight,
January 1, 1970, localtime returns NULL. The fields of the
structure type tm store the following values, each of which
is an int:

tm_sec  Seconds after minute (0 - 59)
tm_min  Minutes after hour (0 - 59)
tm_hour Hours after midnight (0 - 23)

tm_mday Day of month (1 - 31)
tm_mon  Month (0 - 11; January = 0)
tm_year Year (current year minus 1900)

tm_wday Day of week (0 - 6; Sunday = 0)

tm_yday Day of year (0 - 365; January 1 = 0)

tm_isdst
Positive value if daylight saving time is in effect; 0 if
daylight saving time is not in effect; negative value if
status of daylight saving time is unknown. The C run-time
library assumes the United States’s rules for implementing
the calculation of Daylight Saving Time (DST).
TD_LEAD_ZERO    = 0x01 = hh:0m:0s else hh:m:s
TD_HR_LEAD_ZERO = 0x02 = hh:mm:ss else 0h:mm
TD_YR_LEAD_ZERO = 0x04 = mm/dd/0y else mm/dd/y
TD_SECONDS      = 0x08 = hh:mm:ss else hh:mm
TD_24HOUR       = 0x10 = time 17:23 else 5:23P
TD_YEAR_4DIG    = 0x20 = 2001 shown as 2001 else 01
TD_DATE_HYPHEN  = 0x40 = date mm-dd-yy else mm/dd/yy
TD_MONTH_UK     = 0x80 = dd/mm/yy else USA mm/dd/yy

*****************************************************************/

void time_date2( time_t jsec_1970, char **time_str, char **date_str, int mode ) {
	struct tm *now;
	static struct tm def_1980;
	time_t long_time;
	static char time_buf[ TIME_DATE_TEXT_SIZE  ], date_buf[ TIME_DATE_TEXT_SIZE  ];
	int df1, df2, df3, hour;
	int n_dig;
	static char s1[ 4 ], s2[ 4 ], s3[ 8 ];
	char sep, *am_pm;

    long_time = jsec_1970;

	now = localtime( &long_time ); /* Convert to local time. */
    if( now == NULL ) {
        def_1980.tm_mday = 1;
        
        def_1980.tm_hour = def_1980.tm_min = def_1980.tm_sec = 0;
        
        def_1980.tm_mon = 0;
        def_1980.tm_year = 80;
        
        def_1980.tm_wday = 2; // Tuesday
        def_1980.tm_yday = 0;
        
        def_1980.tm_isdst = 0;
        now = &def_1980;
        }

/* Date ****************************************/
	if( mode & TD_MONTH_UK ) {
		df1 = now->tm_mday;
		df2 = now->tm_mon + 1;
		}
	else {
		df1 = now->tm_mon + 1;
		df2 = now->tm_mday;
		}

	if( mode & TD_YEAR_4DIG ) df3 = now->tm_year + 1900;
	else                      df3 = now->tm_year % 100;

	if( mode & TD_LEAD_ZERO ) n_dig = 2;
	else                      n_dig = 1;
	strcpy( s1, lead_zero_number( df1, n_dig ) );
	strcpy( s2, lead_zero_number( df2, n_dig ) );

	if( mode & TD_YR_LEAD_ZERO ) n_dig = 2;
	else                         n_dig = 1;
	strcpy( s3, lead_zero_number( df3, n_dig ) );

	if( mode & TD_DATE_HYPHEN ) sep = '-';
	else                        sep = '/';

	sprintf( date_buf, "%s%c%s%c%s", s1, sep, s2, sep, s3 );

/* Time ****************************************/
	if( mode & TD_LEAD_ZERO ) n_dig = 2;
	else                      n_dig = 1;
	strcpy( s2, lead_zero_number( now->tm_min, n_dig ) );

	if( mode & TD_SECONDS )
		strcpy( s3, lead_zero_number( now->tm_sec, n_dig ) );

	hour = now->tm_hour;
	if( mode & TD_24HOUR ) {
		am_pm = NULL;
		}
	else {
		am_pm = "A";
		if( now->tm_hour > 12 ) hour -= 12;

		if( now->tm_hour >= 12 ) am_pm = "P"; 
		}		

	if( mode & TD_HR_LEAD_ZERO ) n_dig = 2;
	else                         n_dig = 1;
	strcpy( s1, lead_zero_number( hour, n_dig ) );

	if( mode & TD_SECONDS )
		sprintf( time_buf, "%s:%s:%s", s1, s2, s3 );

	else
		sprintf( time_buf, "%s:%s", s1, s2 );

	if( am_pm ) strcat( time_buf, am_pm );

	if( time_str ) *time_str = time_buf;
	if( date_str ) *date_str = date_buf;

	} // time_date2()


/*------------------------------------------------------------------------------------------*/
char *time_date1( time_t jsec_1970, int mode ) {
   static char ret[ TIME_DATE_TEXT_SIZE * 2 + 2 ], *ptm, *pdt;

    time_date2( jsec_1970, &ptm, &pdt, mode );     sprintf( ret, "%s %s", pdt, ptm );
    return( ret );
    } // time_date1()

/*------------------------------------------------------------------------------------------*/
char *end_slash( char *in ) {
    char *last;
    last = in + strlen( in ) - 1;
    if( *last != '\\' ) { last++; *last++ = '\\'; *last = 0; }
    return( in );

    } //end_slash()

/*------------------------------------------------------------------------------------------*/
char *join_path2( char *base, char *in )
    {

    if( base ) { end_slash( base );   strcat( base, in ); }
    return( base );

    }   /* join_path2() */

/************************************************************************
Returns a new file name for an existing file name such that the new name does
not exist as a file. Used to create a new version name for existing file
so as not to overwrite the exsiting file. 
E.G. my_file.sav => my_file_(1).sav
if my_file_(1).sav exists => my_file_(2).sav
etc.
Keeps going until 99999 tries = Error or new non-existing name found.
Similary to Firefox download behavior
************************************************************************/

char *new_version_file_name( // Returns new version name of non existant file. NULL if error
    char *old_fn,            // Old name to save
    int *tries ) {           // Number of names tried. Error = -1 NULL if not used

    static char new_fn[ _MAX_PATH * 2 ];
    static char old_ext[ _MAX_PATH ];
    char *p, *end_name, *p_ext = NULL;
    int ii, tag = 1;

    if( old_fn == NULL || *old_fn == 0 ) { // NULL or empty string
        if( tries ) { *tries = -1; }
        return( NULL );
        }

    if( _access( old_fn, FILE_EXIST ) ) { if( tries ) *tries = 0; return( old_fn ); }

    old_ext[ 0 ] = 0; 
    strcpy( new_fn, old_fn );
    ii = strlen( new_fn ); 
    end_name = new_fn + ii--; // Save end of names and Point to end of name for '.' search

    while( ii > 0 ) {
        p = new_fn + ii;
        if( *p == '.' ) { strcpy( old_ext, p ); *p = 0;  end_name = p; break; } // Cut off .ext and save

        ii--;

        } // while( ii )

    while( tag < 100000 ) {
            sprintf( end_name, "_(%d)%s", tag++, old_ext );
            if( _access( new_fn, FILE_EXIST ) ) break; // Found a new name that does no exist
            }

    if( tries ) { *tries = tag - 1; }

    return( new_fn );

    } // new_version_file_name()

/*------------------------------------------------------------------------------------------
Convert DOS packed time to Unix style time i.e. seconds from 1 Jan 1970 */
time_t DOS_dttm_to_time( UI16 dt, UI16 tm ) {
    static struct tm ymd_hms;
    static time_t rc;

    ymd_hms.tm_mday =    dt & 0x1F;
    ymd_hms.tm_mon  = ( ( dt >> 5 ) & 0x0F ) - 1 ; // 1 to 12 
    ymd_hms.tm_year = ( ( dt >> 9 ) & 0x7F ) + 80; // 9 - 15 Start at 1980

    ymd_hms.tm_hour = ( tm >> 11 ) & 0x1F;
    ymd_hms.tm_min  = ( tm >> 5 ) & 0x3F;
    ymd_hms.tm_sec  = ( tm & 0x0F ) << 1;

    rc = mktime( &ymd_hms );

    return( rc );
    }


/*------------------------------------------------------------------------------------------*/

int make_dir_path( char *out ) {
    static struct _stat sb;
    char *msg;
    int rc = 0;

    if( _stat( out, &sb ) ) {
        printf( "Not exist: %s\n", out );
        if( CreateDirectoryA( out, NULL ) ) { msg = "Ok:"; }
        else { msg = "Fail:"; rc = 0xFF; }

        printf( "%s CreateDirectoryA(%s)\n", msg, out);
        }
     else {
         if( sb.st_mode & _S_IFDIR ) { printf( "Directory exists: %s\n", out ); }
         else { printf( "Exected Directory but File exists: %s\n", out ); rc = 0xFF;}

        }
    return( rc );

    } // make_dir_path()


/*------------------------------------------------------------------------------------------
Write a block of BACKUP.001 to a file
The name, offset, length and time stamp  come from the CONTROL.001 file. */

int write_file(
    FILE *fp_bk,   // Open file pointer into BACKUP.001
    char *fn,      // Name of file to be written
    UI32 f_os,     // Offset into BACKUP.001
    UI32 f_size,   // Size of file in Bytes
    time_t tmt ) { // Time stamp of file. Seconds from 1 Jan 1970

    static char buf[ RD_BLK + 8 ];
    FILE *fp;
    int tries, rc = 0, more = 0xFF;
    long blk;
    static struct _utimbuf utmb;

    _set_errno( 0 );
    if( fseek( fp_bk, f_os, SEEK_SET ) != 0  ) {
        printf( "Oops! %d fseek(%ld) failed %s\n", __LINE__, f_os, strerror( errno ) );
        return( rc );
        }

    fn = new_version_file_name( fn, &tries );
    if( tries > 0 ) { printf( "FYI: previous (%d) exists. New: %s\n\n", tries, fn ); }

    fp = fopen( fn, "wb" ); if( fp == NULL ) {
        printf( "Oops! %d fopen(%s) failed\n", __LINE__, fn );
        return( rc );
        }

    while( more ) {
        if( f_size > RD_BLK ) { blk = RD_BLK; f_size -= RD_BLK; }
        else { more = 0; blk = f_size; }
        if( fread( buf, blk, 1, fp_bk ) != 1 ) {
            printf( "Oops! %d fead(%ld) failed\n", __LINE__, blk );
            return( rc );
            }         

        if( fwrite( buf, blk, 1, fp ) != 1 )   {
            printf( "Oops! %d fwrite(%s %ld) failed\n", __LINE__, fn, blk ); 
            return( rc );
            }         

        rc++;
        }
    fclose( fp );

    utmb.actime = tmt;    utmb.modtime = tmt;

    _utime( fn, &utmb );

    return( rc );
    } // write_file()


// End of File 