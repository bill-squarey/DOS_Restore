// DOS_Restore.c : Defines the entry point for the console application.
//

#include "util_lib.h"

#define TIME_DATE_FMT ( TD_LEAD_ZERO | TD_HR_LEAD_ZERO | TD_24HOUR | TD_SECONDS | TD_YR_LEAD_ZERO | TD_YEAR_4DIG )

REST_HEADER head;

DIR_FILE dr_fl;

DIR_FILE32 dr_fl32;

// Command line options
char Opt_az[ 26 + 1 ], OptAZ_[ 26 + 1 ],  Opt_09[ 10 + 1 ];

/*------------------------------------------------------------------------------------------*/
void show_help( char *err, char *msg ) {
static char *help[] = {
"Oops! %s\n",
"Usage:\n%s -options CONTROL.nnn Out_Restore_Path\n",
//       0       1       2             3
"If Out_Restore_Path given, files restored, else only analysis to stdout\n",
"Out_Restore_Path must exist.\n\n",
"If restored file exits, new restored file given next version number *_(n).*\n",
"Existing files never overwritten.\n\n",
"If restoring from a backup with multiple parts e.g. BACKUP.001, BACKUP.002, BACKUP.003 etc.\n",
"Restore them all in sequence. At the command prompt:\n",
"MD rest_path\n",
"DOS_Restore -S CONTROL.001 rest_path >restore.log\n",
"DOS_Restore -S CONTROL.002 rest_path >>restore.log\n",
"DOS_Restore -S CONTROL.003 rest_path >>restore.log\n",
"etc.\n\n",
"Options:\n",
"  -B = Show all Bytes for Long File Name OS/2 (No Interpretation of offset size date/time)\n",
"  -H = Help. Print this message\n",
"  -L = Long Files Names (OS/2)\n",
"  -S = Short File Names (DOS 8.3)\n",
"  -V = Verbose output to stdout\n",
NULL };

    short ii = 0;
    char **cp = help;
    while( *cp ) {
        if( ii == 0 ) printf( *cp, err );
        else if( ii == 1 ) printf( *cp, msg );
        else printf( *cp );
        ii++; cp++;
        }

    printf( "\n%s %s [MSC_VER:%d WIN32:%d M_IX86:%d]\n\n      W. R. S. WEBBER Ph.D.\n",
        __DATE__, __TIME__, _MSC_VER, _WIN32, _M_IX86 );

    exit( 1 );
    } // show_help()

/*------------------------------------------------------------------------------------------*/

int analyze_OS2_bytes( char *fn ) {
    static UI8 buf[ 2048 ];

    FILE *fp;
    UI8 *pt;
    int ch, ii, os, st_ct;

    fp = fopen( fn, "rb" ); // CONTROL.001

    if( fp == NULL ) { printf( "Oops(%d): fopen( %s ) need file\n", __LINE__, fn ); exit( 2 ); }

    while( 1 ) {
        ch = fgetc( fp ); if( ch == EOF ) { break; }
        fread( buf, ( ch - 1 ), 1, fp );
        os = 0; st_ct = -1;
        printf( "\nBlock length %d 0x%X\n", ch, ch );
        pt = buf;
        for( ii = 0; ii < ( ch - 1 ); ii++ ) {
            if( *pt < 0x20 || *pt > '~' ) { // Non ASCII so print Hex 0xNN
                if( st_ct > 0 ) { if( st_ct > 1 ) printf( " (%d chk=%d)\n", st_ct, ( ch - st_ct ) ); } // Close last text string
                printf( "  os-%d %X", ii, *pt ); os = 0; // Print Hex and prep for next text string
                st_ct = 0; // prep for next text string
                }
            else{
                if( os == 0 ) { printf( "\nos=%d " , ii ); os++; } // Start text string with byte offset (index)
                printf( "%c", *pt ); st_ct++; // print one text char and count
                }
            pt++;
            }
        printf( "\n" );
        if( buf[ 7 ] > 0 ) { printf( "Dir_name: 0x%X %s\n", buf[ 7 ],  buf + 18 ); } // Dir falg 
        else {               printf( "File_name: %s\n", buf + 32 ); }
        } // while( 1 )

    printf( "Done reading %s -B (%d)\n", fn, __LINE__ );

    return( 0 );
    } // analyze_OS2_bytes()


/*------------------------------------------------------------------------------------------*/

int DOS_rest( char *fn, char *out, int verbose ) {

    FILE *fp;    // CONTROL.001
    FILE *fp_bk; // BACKUP.001
    static UI8 buf[ 2048 ];
    int hd_len, dr_fl_len, delta, rec_size;

    UI8 *read_pt, *ptd, *ptf;
    UI32 f_pos, f_os, f_size, f_chk = 0L;
    time_t tmt;

    static char tx[ 1024 + 4 ];
    static char dir_name[ PATH_LEN + 4 ];
    static char file_name[ FILE_LEN + 4 ];
    
    static char new_dir[   _MAX_FNAME + 8 ];
    static char new_file[  _MAX_FNAME + 8 ];
    static char fn_backup[ _MAX_FNAME + 8 ];

    static char bk_drv[  _MAX_DRIVE + 4 ];
    static char bk_path[ _MAX_FNAME + 4 ];
    static char bk_name[ _MAX_FNAME + 4 ];
    static char bk_ext[  _MAX_FNAME + 4 ];

    fp = fopen( fn, "rb" ); // CONTROL.001

    if( fp == NULL ) { printf( "Oops(%d): fopen( %s ) need file\n", __LINE__, fn ); exit( 2 ); }

    if( out == NULL ) { fp_bk = NULL; }

    else { // Setup output Dir and file
        _splitpath( _fullpath( fn_backup, fn, _MAX_FNAME ), bk_drv, bk_path, bk_name, bk_ext );
        // printf( "Drv: %s\nPath: %s\nName: %s\nExt: %s\n", bk_drv, bk_path, bk_name, bk_ext );

        sprintf( fn_backup, "%s%s%s%s", bk_drv, bk_path, "BACKUP", bk_ext ); // BACKUP.001

        fp_bk = fopen( fn_backup, "rb" );
        if( fp_bk == NULL ) { printf( "Oops(%d): fopen( %s ) need file\n", __LINE__, fn_backup ); exit( 3 ); }
        else { if( verbose) printf( "Backup: %s\n", fn_backup ); }

        /* Need to create root dir for restore.
        CreateDirectoryA() does not do more than one level i.e. CreateDirectoryA( "aa\bb" ) fails */
        strcpy( new_dir, out ); make_dir_path( new_dir );
        }

    hd_len = sizeof( head );    dr_fl_len = sizeof( dr_fl );

    delta = sizeof(dr_fl.dr_rec) - sizeof(dr_fl.fl_rec);
    if( verbose ) printf( "sizeof(head) %d sizeof(dir_file) %d sizeof(fl_rec) %d delta %d\n",
        hd_len, dr_fl_len, sizeof(dr_fl.fl_rec), delta );

    if( fread( &head, hd_len, 1, fp ) != 1 ) { printf( "Read head failed\n" ); exit( 3 ); }
    else {
        strncpy( tx, head.sig, SIG_LEN ); tx[ 8 ] = 0;
        if( verbose ) printf( "Read head Ok First 0x%X Sig %s Seq %d Last %X\n",
            (unsigned int)head.hd, tx, head.seq, (unsigned int)head.last );
        }

    while( 1 ) {
        f_pos = ftell( fp );
        if( verbose ) printf( "\n%s f_pos 0x%lX %ld\n", fn, f_pos, f_pos );
        if( fread( &dr_fl, sizeof(dr_fl.fl_rec), 1, fp ) != 1 ) { printf( "Done reading %s (%d)\n", fn, __LINE__ ); break; }
        else {
            rec_size = (int)dr_fl.fl_rec.blk_size;
            if( verbose ) printf( "rec_size %d f_pos 0x%lX\n", rec_size, ftell( fp ) );

            if( rec_size != 34 ) { // is Directory. Need to read rest
                read_pt = (char *)&dr_fl + 34; 
                if( fread( read_pt, delta, 1, fp ) != 1 ) { printf( "Done reading %s (%d)\n", fn, __LINE__ ); break; }

                else { // Process directory
                    strncpy( dir_name, dr_fl.dr_rec.path, PATH_LEN );
                    dir_name[ PATH_LEN ] = 0; ptd = dir_name;

                    if( verbose ) printf( "Dir: %s Nr entries %hd 0x%lX\n", ptd, dr_fl.dr_rec.nr, dr_fl.dr_rec.end );
                    else printf( "Dir: %s Nr entries %hd\n", ptd, dr_fl.dr_rec.nr );
                    if( fp_bk && *ptd && *ptd != 0x20 && strlen( ptd ) > 0 ) {
                        join_path2( strcpy( new_dir, out), ptd );
                        make_dir_path( new_dir );
                        }
                    }
                } // if( rec_size != 34 )

            else { // process File
                f_os   = dr_fl.fl_rec.f_os;         f_size = dr_fl.fl_rec.f_size;

                tmt = DOS_dttm_to_time( dr_fl.fl_rec.f_date, dr_fl.fl_rec.f_time );
                strncpy( file_name, dr_fl.fl_rec.name, FILE_LEN );
                file_name[ FILE_LEN ] = 0; ptf = file_name;

                if( verbose ) printf( "File: Size %8ld Os %8ld %-12s %s\n", f_size, f_os, ptf, time_date1( tmt, TIME_DATE_FMT ) );
                else printf( "File: %-12s Size %8ld %s\n", ptf, f_size, time_date1( tmt, TIME_DATE_FMT ) );

                if( fp_bk ) {
                    join_path2( strcpy( new_file, new_dir ), ptf );

                    // int write_file( FILE *fp_bk, char *fn, UI32 f_os, UI32 f_size, time_t tmt )
                    if( *ptf && f_size > 0 && strlen( ptf ) ) write_file( fp_bk, new_file, f_os, f_size, tmt );
                    else printf( "Skip Zero length file: %s\n", new_file );

                    }
                } //  // process File
            }

        } // while( 1 )

    return 0;
    
    } // DOS_rest()


/*------------------------------------------------------------------------------------------*/

int OS2_rest( char *fn, char *out, int verbose ){
    FILE *fp, *fp_bk;
    UI8 *pt, *ptd, *ptf;
    int ch, ii;
    UI32 f_os, f_size, f_chk = 0L;
    time_t tmt;
    static char new_dir[   _MAX_FNAME + 8 ];
    static char new_file[  _MAX_FNAME + 8 ];
    static char fn_backup[ _MAX_FNAME + 8 ];

    static char bk_drv[  _MAX_DRIVE + 4 ];
    static char bk_path[ _MAX_FNAME + 4 ];
    static char bk_name[ _MAX_FNAME + 4 ];
    static char bk_ext[  _MAX_FNAME + 4 ];

    fp = fopen( fn, "rb" ); // CONTROL.001

    if( fp == NULL ) { printf( "Oops(%d): fopen( %s ) need file\n", __LINE__, fn ); exit( 2 ); }

    if( out == NULL ) { fp_bk = NULL; }
    else {
        _splitpath( _fullpath( fn_backup, fn, _MAX_FNAME ), bk_drv, bk_path, bk_name, bk_ext );
        // printf( "Drv: %s\nPath: %s\nName: %s\nExt: %s\n", bk_drv, bk_path, bk_name, bk_ext );

        sprintf( fn_backup, "%s%s%s%s", bk_drv, bk_path, "BACKUP", bk_ext ); // BACKUP.001

        fp_bk = fopen( fn_backup, "rb" );
        if( fp_bk == NULL ) { printf( "Oops(%d): fopen( %s ) need file\n", __LINE__, fn_backup ); exit( 3 ); }
        else { printf( "Backup: %s\n", fn_backup ); }

        /* Need to create root dir for restore.
        CreateDirectoryA() does not do more than one level i.e. CreateDirectoryA( "aa\bb" ) fails */
        strcpy( new_dir, out ); make_dir_path( new_dir );
        }

    while( 1 ) {
        ch = fgetc( fp ); if( ch == EOF ) { break; }
        fread( &dr_fl32, ( ch - 1 ), 1, fp );

        if( verbose ) printf( "\nBlock %d 0x%X\nBytes 0-6 ", ch, ch );
        pt = dr_fl32.dr_rec.pad1;
        if( verbose ) { for( ii = 0; ii < 7; ii++ ) { printf( " %2X", *pt++ ); } printf( "\n" ); }
        
        if( dr_fl32.dr_rec.dir_flag > 0 ) { // Directory
            pt = dr_fl32.dr_rec.pad2;
            if( verbose ) {
                printf( "Dir_flag 0x%2X Bytes 8-17 ", dr_fl32.dr_rec.dir_flag );
                for( ii = 8; ii < 18; ii++ ) { printf( " %2X", *pt++ ); }
                printf( "\n" );
                }
            ptd = dr_fl32.dr_rec.path;
            if( verbose ) printf( "Dir: %2X %s\n", dr_fl32.dr_rec.dir_flag, ptd );
            else printf( "Dir: %s\n", ptd );

            if( fp_bk && *ptd && *ptd != 0x20 && strlen( ptd ) > 0 ) {
                join_path2( strcpy( new_dir, out), ptd );
                make_dir_path( new_dir );
                }

            }
        else { // File name
            if( verbose ) { 
                printf( "Bytes 16-32 " ); 
                for( ii = 16; ii < 32; ii++ ) { printf( " %2X", *pt++ ); } printf( "\n" );
                }

            f_os   = dr_fl32.fl_rec.f_os;         f_size = dr_fl32.fl_rec.f_size;

            tmt = DOS_dttm_to_time( dr_fl32.fl_rec.f_date, dr_fl32.fl_rec.f_time );

            pt = dr_fl32.fl_rec.pad2;

            ptf = dr_fl32.fl_rec.name;
            if( verbose ) printf( "File: Os %8ld Size %8ld Ck %6ld %-12s %s\n", f_os, f_size, (f_os - f_chk), ptf, time_date1( tmt, TIME_DATE_FMT ) );
            else printf( "File: %-12s Size %8ld %s\n", ptf, f_size, time_date1( tmt, TIME_DATE_FMT ) );
            f_chk += f_size;

            if( fp_bk ) {
                join_path2( strcpy( new_file, new_dir ), ptf );
                if( *ptf && f_size > 0 && strlen( ptf ) > 0 ) write_file( fp_bk, new_file, f_os, f_size, tmt );
                else printf( "Skip Zero length file: %s\n", new_file );
                }
            }

        } // while( 1 )

    printf( "Done reading %s -L (%d)\n", fn, __LINE__ );

    return( 0 );
    } // OS2_rest()

/*------------------------------------------------------------------------------------------*/

int main( int argc, char *argv[] ) {
    int argc0;
    char **argv0;

    int verbose;
    char *fn, *out, *opt;

    argv0 = argv; argc0 = argc; // save originals JIC.

    if( argc < 2 ) { show_help( "Need Input file CONTROL.nnn (1)", *argv ); } // does not return, but exit();

    clear_cl_options( OptAZ_, Opt_az, Opt_09 );    // Clear all option flags.

    while( argc > 1 && ( *(argv[ 1 ]) == '-' || *(argv[ 1 ]) == '/' ) ) {
        opt = *argv + 1;

        if( *opt == '?' || *opt == 'h' || *opt == 'H' ) show_help( "Need Input file CONTROL.nnn (2)", *argv0 ); // does not return, but exit();

        argc--;     argv++; // Set options as processed

        set_cl_options( OptAZ_, Opt_az, Opt_09, ( *argv + 1 ) ); /* Point to first char after '-' */
        } // while( *(argv[ 1 ]) == '-' || *(argv[ 1 ]) == '/')

   if( argc < 2 ) show_help( "Need Input file CONTROL.nnn (3)", *argv0 ); // does not return, but exit();

    fn = argv[ 1 ];   out = NULL; if( argc > 2 ) out = argv[ 2 ];

    if( OptAZ_[ CL_OPT_B ] ) { analyze_OS2_bytes( fn ); exit( 0 ); } 

    verbose = 0;  if( OptAZ_[ CL_OPT_V ] ) verbose = 0xFF;

    if( OptAZ_[ CL_OPT_L ] || OptAZ_[ CL_OPT_O ] ) { OS2_rest( fn, out, verbose ); exit( 0 ); } 
    if( OptAZ_[ CL_OPT_S ] || OptAZ_[ CL_OPT_D ] ) { DOS_rest( fn, out, verbose ); exit( 0 ); } 

    show_help( "No valid options given", *argv );

    }

// End of File 
