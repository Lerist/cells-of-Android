#ifndef UTIL_H
#define UTIL_H

#include <dirent.h>

#define __STR(X) #X
#define _STR(X) __STR(X)
#define STR(X) _STR(X)

int vdir_exists(const char *fmt, ...);
int dir_exists(const char *path);
int file_exists(const char *path);
void daemonize(void);
void close_fds(void);
int rmtree(const char *path);
int walkdir(void *ctx, const char *base, const char *sdir, int depth,
		void (*callback)(void *ctx, const char *path, const char *subpath, struct dirent *e));
int copy_file(const char *src, const char *dst);

int is_mounted(const char *path);
int __unmount_dir(const char *root_path, char *dir);

int insert_module(const char *path, const char *params);

#endif
