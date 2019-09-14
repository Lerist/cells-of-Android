#ifndef CELL_CONSOLE_H
#define CELL_CONSOLE_H

#define PATH_MAX 4096

struct pty_info {
	int  ptm;			/* Master pts file descriptor		 */
	int  pty;			/* Slave file descriptor			  */
	char name[PATH_MAX];		/* Path to slave in host /dev/pts	 */
	char cont_path[PATH_MAX];	/* Host path to file inside container */
};

int open_cell_console(int ptm, const char *cmd, const char *args);
int create_cell_console(const char *container_root, struct pty_info *pi);
void cleanup_cell_console(struct pty_info *pi);

#endif
