#pragma once
#include <stddef.h>
#include <sys/types.h>
struct fs_file_t { const char *path;size_t offset; };
enum {FS_O_READ=1,FS_O_CREATE=2,FS_O_WRITE=4,FS_O_TRUNC=8};
inline void fs_file_t_init(fs_file_t *f) {*f={};}
int fs_open(fs_file_t*,const char*,int);
int fs_close(fs_file_t*);
ssize_t fs_read(fs_file_t*,void*,size_t);
ssize_t fs_write(fs_file_t*,const void*,size_t);
int fs_mkdir(const char*);
int fs_unlink(const char*);
int fs_sync(fs_file_t*);
int fs_rename(const char*,const char*);
enum { FS_SEEK_SET = 0 };
int fs_seek(fs_file_t*, off_t, int);
