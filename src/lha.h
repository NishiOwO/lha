/* ------------------------------------------------------------------------ */
/* LHa for UNIX    Archiver Driver                                          */
/*                                                                          */
/*      Modified                Nobutaka Watazaki                           */
/*                                                                          */
/*  Ver. 1.14   Soruce All chagned              1995.01.14  N.Watazaki      */
/*  Ver. 1.14i  Modified and bug fixed          2000.10.06  t.okamoto       */
/* ------------------------------------------------------------------------ */
/*
    Included...
        lharc.h     interface.h     slidehuf.h
*/

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
//#include <sys/file.h>
#include <sys/stat.h>
#include <signal.h>

#include <inttypes.h>
#include <stdint.h>

#include <string.h>

#include <stdlib.h>
#include <stddef.h>

#define STDC_HEADERS 1
#define DEFAULT_LZHUFF_METHOD LZHUFF5_METHOD_NUM
#define ADDITIONAL_SUFFIXES
#define PACKAGE_NAME "LHa cross-platform"
#define PACKAGE_VERSION "1.14i-ac20220213"
#define PLATFORM "cross"
#define LHA_CONFIGURE_OPTIONS ""

#ifndef NULL
#define NULL ((char *)0)
#endif

#ifndef _WIN32
#include <unistd.h>
#endif

#if defined(__WATCOMC__) || defined(__MINGW32__) || defined(__unix__)
#include <fcntl.h>
#endif

#include <stdarg.h>
#define va_init(a,b) va_start(a,b)

#ifdef __unix__
# include <pwd.h>
# include <grp.h>
#else
#ifndef __WATCOMC__
typedef int uid_t;
typedef int gid_t;
#endif
#endif

#if !HAVE_UINT64_T
# define HAVE_UINT64_T 1
# if SIZEOF_LONG == 8
    typedef unsigned long uint64_t;
# elif HAVE_LONG_LONG
    typedef unsigned long long uint64_t;
# else
#  undef HAVE_UINT64_T
# endif
#endif

#include <time.h>

#if HAVE_UTIME_H
#include <utime.h>
#else
struct utimbuf {
    time_t actime;
    time_t modtime;
};
int utime(const char *, struct utimbuf *);
#endif

#if !defined(_WIN32) || defined(__MINGW32__)
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
# ifdef NONSYSTEM_DIR_LIBRARY           /* no use ?? */
#  include "lhdir.h"
# endif
#endif

#if HAVE_FNMATCH_H
# include <fnmatch.h>
#else
int fnmatch(const char *pattern, const char *string, int flags);
# define FNM_PATHNAME 1
# define FNM_NOESCAPE 2
# define FNM_PERIOD   4
#endif

#if HAVE_LIBAPPLEFILE
#include <applefile.h>
#endif

#ifdef __DJGPP__
#include <io.h>
#endif /* __DJGPP__ */

#ifndef SEEK_SET
#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2
#endif  /* SEEK_SET */

#include <limits.h>

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

#ifndef UCHAR_MAX
#define UCHAR_MAX ((1<<(sizeof(unsigned char)*8))-1)
#endif

#ifndef USHRT_MAX
#define USHRT_MAX ((1<<(sizeof(unsigned short)*8))-1)
#endif

#ifndef SHRT_MAX
#define SHRT_MAX ((1<<(sizeof(short)*8-1))-1)
#endif

#ifndef SHRT_MIN
#define SHRT_MIN (SHRT_MAX-USHRT_MAX)
#endif

#ifndef ULONG_MAX
#define ULONG_MAX ((1<<(sizeof(unsigned long)*8))-1)
#endif

#ifndef LONG_MAX
#define LONG_MAX ((1<<(sizeof(long)*8-1))-1)
#endif

#ifndef LONG_MIN
#define LONG_MIN (LONG_MAX-ULONG_MAX)
#endif

#if !HAVE_FSEEKO
# define fseeko  fseek
#endif
#if !HAVE_FTELLO
# define ftello  ftell
#endif

#include "lha_macro.h"

#define exit(n) lha_exit(n)

struct encode_option {
#if defined(__STDC__) || defined(AIX)
    void            (*output) ();
    void            (*encode_start) ();
    void            (*encode_end) ();
#else
    int             (*output) ();
    int             (*encode_start) ();
    int             (*encode_end) ();
#endif
};

struct decode_option {
    unsigned short  (*decode_c) ();
    unsigned short  (*decode_p) ();
#if defined(__STDC__) || defined(AIX)
    void            (*decode_start) ();
#else
    int             (*decode_start) ();
#endif
};

/* ------------------------------------------------------------------------ */
/*  LHa File Type Definition                                                */
/* ------------------------------------------------------------------------ */
typedef int boolean;            /* TRUE or FALSE */

struct string_pool {
    int             used;
    int             size;
    int             n;
    char           *buffer;
};

typedef struct LzHeader {
    size_t          header_size;
    int             size_field_length;
    char            method[METHOD_TYPE_STORAGE];
    off_t           packed_size;
    off_t           original_size;
    unsigned char   attribute;
    unsigned char   header_level;
    char            name[FILENAME_LENGTH];
    char            realname[FILENAME_LENGTH];/* real name for symbolic link */
    unsigned int    crc;      /* file CRC */
    boolean         has_crc;  /* file CRC */
    unsigned int    header_crc; /* header CRC */
    unsigned char   extend_type;
    unsigned char   minor_version;

    /* extend_type == EXTEND_UNIX  and convert from other type. */
    time_t          unix_last_modified_stamp;
    unsigned short  unix_mode;
    unsigned short  unix_uid;
    unsigned short  unix_gid;
    char            user[256];
    char            group[256];
}  LzHeader;

struct interfacing {
    FILE            *infile;
    FILE            *outfile;
    off_t           original;
    off_t           packed;
    off_t           read_size;
    int             dicbit;
    int             method;
};


/* ------------------------------------------------------------------------ */
/*  Option switch variable                                                  */
/* ------------------------------------------------------------------------ */
#ifdef LHA_MAIN_SRC
#define EXTERN
#else
#define EXTERN extern
#endif

/* command line options (common options) */
EXTERN boolean  quiet;
EXTERN boolean  text_mode;
EXTERN boolean  verbose;
EXTERN boolean  noexec;     /* debugging option */
EXTERN boolean  force;
EXTERN boolean  timestamp_archive;
EXTERN boolean  delete_after_append;
EXTERN int      compress_method;
EXTERN int      header_level;
/* EXTERN int       quiet_mode; */   /* 1996.8.13 t.okamoto */
#ifdef EUC
EXTERN boolean  euc_mode;
#endif

/* list command flags */
EXTERN boolean  verbose_listing;

/* extract/print command flags */
EXTERN boolean  output_to_stdout;

/* add/update/delete command flags */
EXTERN boolean  new_archive;
EXTERN boolean  update_if_newer;
EXTERN boolean  generic_format;

EXTERN boolean  remove_temporary_at_error;
EXTERN boolean  recover_archive_when_interrupt;
EXTERN boolean  remove_extracting_file_when_interrupt;
EXTERN boolean  get_filename_from_stdin;
EXTERN boolean  ignore_directory;
EXTERN char   **exclude_files;
EXTERN boolean  verify_mode;

/* Indicator flag */
EXTERN int      quiet_mode;

EXTERN boolean backup_old_archive;
EXTERN boolean extract_broken_archive;
EXTERN boolean decode_macbinary_contents;

/* for debugging */
EXTERN boolean sort_contents;
EXTERN boolean recursive_archiving;
EXTERN boolean dump_lzss;


/* ------------------------------------------------------------------------ */
/*  Globale Variable                                                        */
/* ------------------------------------------------------------------------ */
EXTERN char     **cmd_filev;
EXTERN int      cmd_filec;

EXTERN char     *archive_name;
EXTERN char     temporary_name[FILENAME_LENGTH];
EXTERN char     backup_archive_name[FILENAME_LENGTH];

EXTERN char     *extract_directory;
EXTERN char     *reading_filename, *writing_filename;

EXTERN int      archive_file_mode;
EXTERN int      archive_file_gid;

EXTERN int      convertcase;    /* 2000.10.6 */

EXTERN char     iconv_code_system[FILENAME_LENGTH];
EXTERN char     iconv_code_archive[FILENAME_LENGTH];

/* slide.c */
EXTERN int      unpackable;
EXTERN off_t origsize, compsize;
EXTERN unsigned short dicbit;
EXTERN unsigned short maxmatch;
EXTERN off_t decode_count;
EXTERN unsigned long loc;           /* short -> long .. Changed N.Watazaki */
EXTERN unsigned char *text;         /* encoding buffer */
EXTERN unsigned char *dtext;        /* decoding buffer */

/* huf.c */
#ifndef LHA_MAIN_SRC  /* t.okamoto 96/2/20 */
EXTERN unsigned short left[], right[];
EXTERN unsigned char c_len[], pt_len[];
EXTERN unsigned short c_freq[], c_table[], c_code[];
EXTERN unsigned short p_freq[], pt_table[], pt_code[], t_freq[];
#endif

/* bitio.c */
EXTERN FILE     *infile, *outfile;
EXTERN unsigned short bitbuf;

/* crcio.c */
EXTERN unsigned int crctable[UCHAR_MAX + 1];
EXTERN int      dispflg;

/* from dhuf.c */
EXTERN unsigned int n_max;

/* lhadd.c */
EXTERN int temporary_fd;

/* ------------------------------------------------------------------------ */
/*  Functions                                                               */
/* ------------------------------------------------------------------------ */
#include "prototypes.h"
