Oops! Need Input file CONTROL.nnn (1)
Usage:
DOS_Restore.exe -options CONTROL.nnn Out_Restore_Path
If Out_Restore_Path given, files restored, else only analysis to stdout
Out_Restore_Path must exist.

If restored file exits, new restored file given next version number *_(n).*
Existing files never overwritten.

If restoring from a backup with multiple parts e.g. BACKUP.001, BACKUP.002, BACKUP.003 etc.
Restore them all in sequence. At the command prompt:
MD rest_path
DOS_Restore -S CONTROL.001 rest_path >restore.log
DOS_Restore -S CONTROL.002 rest_path >>restore.log
DOS_Restore -S CONTROL.003 rest_path >>restore.log
etc.

Options:
  -B = Show all Bytes for Long File Name OS/2 (No Interpretation of offset size date/time)
  -H = Help. Print this message
  -L = Long Files Names (OS/2)
  -S = Short File Names (DOS 8.3)
  -V = Verbose output to stdout

Mar 26 2020 16:03:25 [MSC_VER:1912 WIN32:1 M_IX86:600]

      W. R. S. WEBBER Ph.D.
