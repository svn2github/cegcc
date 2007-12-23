#ifndef _SYS_DIRENT_H_
#define _SYS_DIRENT_H_

#define DIRBLKSIZ       512             /* size of directory block */
#define MAXNAMLEN       (512-1)

/* NOTE:  MAXNAMLEN must be one less than a multiple of 4 */

struct dirent               /* data from readdir() */
{
  long  d_ino;              /* inode number of entry */
  unsigned short d_reclen;  /* length of this record */
  unsigned short d_namlen;  /* length of string in d_name */
  char d_name[MAXNAMLEN+1]; /* name of file */
};

typedef struct
{
  int   dd_fd;                  /* file descriptor */
  int   dd_loc;                 /* offset in block */
  int   dd_size;                /* amount of valid data */
  char  dd_buf[DIRBLKSIZ];      /* directory block */
  void *dd_handle;              /* Find handle */
  int   dd_isfat;               /* Is it FAT? */
  char  dd_path[256];           /* dir path */
} DIR;                          /* stream data from opendir() */

#ifdef __cplusplus
extern "C" {
#endif

extern DIR  * opendir(const char *name);
extern struct dirent * readdir(DIR *dir);
extern long telldir(DIR *dir);
extern void seekdir(DIR *dir, long loc);
extern void rewinddir(DIR *dir);
extern int  closedir(DIR *dir);

extern int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);

#ifdef __cplusplus
};
#endif

#endif  /* _SYS_DIRENT_H_ */
