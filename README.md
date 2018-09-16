# DOS_Restore
Restore old DOS Backups (BACKUP.001 CONTROL.001 from before 1995) on Win32 systems

Program to restore files from DOS and OS/2 BACKUP.001 CONTROL.001 backups that were in use prior to Win95

DOS_Restore is itself an Console Win32 program. 

The idea is that these ancient files can be recovered on a modern Win32 system.
You can then use either of these to run DOS programs:
DOSBox https://www.dosbox.com

Free DOS http://www.freedos.org
in Virtual Box https://www.virtualbox.org

DOS_Restore restores all files in BACKUP.001, CONTROL.001 and
recreates the original directory path in a directory that you specify on the commnad line.
It does not overwriet any files. If a target file exists its wriet the new file with a new version number.
I.e. is my_file.txt exists it is restored as my_file_(1).txt and if  my_file_(1).txt then the restored file
is my_file_(2).txt. This process continue until a new name is found that does not exist.

In essence CONTROL.001 is a list of records that are either directories or file in those directories.
Directory records just contain the name of the diectory.

File records contain the name of the file its offset in BACKUP.001 and size in bytes.
The orginal timestamp of the file is also stored in each file record.

The files themselves are stored in BACKUP.001 concatinated one after the other.
There is no compression performed on these files.


Details for the format of CONTROL.001 for DOS 8.3 names was found in:

https://www.ibiblio.org/pub/micro/pc-stuff/freedos/files/dos/backup/brtecdoc.htm
http://ldp.rediris.es/sites/freedos.org/files/dos/backup/brtecdoc.htm

These pages were written by: Ralf Quint
eMail:Ralf@aztechvision.net
 
Written for the FreeDOS project: http://www.freedos.org/

W.R.S. Webber Ph.D.
49bobw@GMail.com
15 September 2018
