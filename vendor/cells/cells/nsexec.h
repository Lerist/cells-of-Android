#ifndef NSEXEC_H
#define NSEXEC_H

extern int mount_dev_tmpfs(char *root_path);
extern int mount_freezer_on_dev(char *root_path);

extern void tear_down_cell(struct cell_args *args,
			   struct pty_info *console_pty);

extern int cell_nsexec(int sd, struct cell_args *args,
			   char *name, struct pty_info *pty);

extern char *create_cgroup(char *cellname);

#endif
