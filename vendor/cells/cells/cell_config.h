#ifndef CELL_CONFIG_H
#define CELL_CONFIG_H

#define MAX_CONFIG_KEY_LEN 50 /* Max length of config keywords */
#define MAX_CONFIG_LINE_LEN   MAX_CONFIG_KEY_LEN + MAX_MSG_LEN
#define CONFIG_DIR ".cellconf"

struct config_info {
	int newcell;
	int uts_ns;
	int ipc_ns;
	int user_ns;
	int net_ns;
	int pid_ns;
	int mount_ns;
	int mnt_rootfs;
	int mnt_tmpfs;
	int newpts;
	int newcgrp;
	int share_dalvik_cache;
	int sdcard_branch;
	int autostart;
	int autoswitch;
	int wifiproxy;
	int initpid;
	int restart_pid; /* pid of restart process */
	int id;
	int console;
};

int cell_exists(char *name);
int id_exists(int id);
int read_config(char *name, struct config_info *config);
int write_config(char *name, struct config_info *config);
int remove_config(char *name);
void init_config(struct config_info *config);

char **get_cell_names(void);
void free_cell_names(char **name_list);
void start_args_to_config(struct cell_start_args *args,
			  struct config_info *config);
void config_to_start_args(struct config_info *config,
			  struct cell_start_args *args);

void init_cellvm_config(const char* name);

#endif
