#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <linux/sched.h>

#define LOG_TAG "Cells/nsexec"
#include <cutils/log.h>
#include <cutils/memory.h>
#include <cutils/misc.h>
#include <cutils/container.h>
#include <selinux/selinux.h>
#include "cutils/properties.h"

#include "celld.h"
#include "cell_console.h"
#include "util.h"
#include "network.h"

#ifndef MNT_DETACH
#define MNT_DETACH 2
#endif

/* Linux clone flags not available in bionic's kernel headers */
#ifndef CLONE_NEWUTS
#define CLONE_NEWUTS			0x04000000	  /* New utsname group? */
#endif
#ifndef CLONE_NEWIPC
#define CLONE_NEWIPC			0x08000000	  /* New ipcs */
#endif
#ifndef CLONE_NEWUSER
#define CLONE_NEWUSER		   0x10000000	  /* New user namespace */
#endif
#ifndef CLONE_NEWPID
#define CLONE_NEWPID			0x20000000	  /* New pid namespace */
#endif
#ifndef CLONE_NEWNET
#define CLONE_NEWNET			0x40000000	  /* New network namespace */
#endif
#ifndef CLONE_IO
#define CLONE_IO				0x80000000	  /* Clone io context */
#endif

static void config_host_net_work(int pid)
{
	if (pid <= 0){
		ALOGD("pid <= 0");
		return;
	}

	system("ip link add name vm_wlan type veth peer name vm_wlan0");
	ALOGD("ip link add name vm_wlan type veth peer name vm_wlan0 errno=%s",strerror(errno));

	system("ifconfig vm_wlan 172.17.3.2 netmask 255.255.0.0 up");
	ALOGD("ifconfig vm_wlan 172.17.3.2 netmask 255.255.0.0 up errno=%s",strerror(errno));

	system("echo 1 > /proc/sys/net/ipv4/ip_forward");
	ALOGD("echo 1 > /proc/sys/net/ipv4/ip_forward errno=%s",strerror(errno));

	system("iptables -t nat -A POSTROUTING -s 172.17.0.0/16 -o wlan0 -j MASQUERADE");
	ALOGD("iptables -t nat -A POSTROUTING -s 172.17.0.0/16 -o wlan0 -j MASQUERADE errno=%s",strerror(errno));

	system("iptables -t filter -A FORWARD -i wlan0 -o vm_wlan -j ACCEPT");
	ALOGD("iptables -t filter -A FORWARD -i wlan0 -o vm_wlan -j ACCEPT errno=%s",strerror(errno));

	system("iptables -t filter -A FORWARD -o wlan0 -i vm_wlan -j ACCEPT");
	ALOGD("iptables -t filter -A FORWARD -o wlan0 -i vm_wlan -j ACCEPT errno=%s",strerror(errno));

	char buf[150];
	memset(buf,0,sizeof(buf));
	snprintf(buf, sizeof(buf), "ip link set vm_wlan0 netns %d", pid);
	system(buf);
	ALOGD("%s errno=%s",buf,strerror(errno));
};

int g_cgroup_pipe[2];

extern int clone(int (*fn)(void *), void *child_stack, int flags, void *arg, ...);

int load_cgroup_dir(char *dest, int len)
{
	FILE *f = fopen("/proc/mounts", "r");
	char buf[200];
	char *name, *path, *fsname, *options, *p1, *p2, *s;
	if (!f)
		return 0;
	while (fgets(buf, sizeof(buf), f)) {
		name = strtok_r(buf, " ", &p1);
		path = strtok_r(NULL, " ", &p1);
		fsname = strtok_r(NULL, " ", &p1);
		options = strtok_r(NULL, " ", &p1);
		if (strcmp(fsname, "cgroup") != 0)
			continue;
		strncpy(dest, path, len);
		fclose(f);
		return 1;
	}
	fclose(f);
	return 0;
}

char *get_cgroup_folder(char *cellname)
{
	char cgroupbase[100];
	char *folder;
	int ret;

	if (!load_cgroup_dir(cgroupbase, sizeof(cgroupbase)))
		return NULL;

	folder = malloc(MAX_PATH_LEN);
	if (!folder)
		return NULL;

	ret = snprintf(folder, MAX_PATH_LEN, "%s/%s", cgroupbase, cellname);
	if (ret >= MAX_PATH_LEN) {
		free(folder);
		return NULL;
	}
	return folder;
}

char *create_cgroup(char *cellname)
{
	int ret;
	char *cgroupname;

	cgroupname = get_cgroup_folder(cellname);
	if (!cgroupname)
		return NULL;

	ret = mkdir(cgroupname, 0755);
	if (ret && errno != EEXIST)
		return NULL;
	return cgroupname;
}

int move_to_new_cgroup(struct cell_args *args)
{
	char tasksfname[200];
	FILE *fout;
	char *cgroupname;
	int ret;

	ret = -1;

	cgroupname = create_cgroup(args->cellname);
	if (!cgroupname)
		goto out;

	snprintf(tasksfname, sizeof(tasksfname), "%s/tasks", cgroupname);
	fout = fopen(tasksfname, "w");
	if (!fout)
		goto out;
	fprintf(fout, "%d\n", args->init_pid);
	fclose(fout);

	ALOGI("Moved %s (%d) into new cgroup (%s)",
		 args->cellname, args->init_pid, cgroupname);
	ret = 0;
out:
	free(cgroupname);
	return ret;
}

int do_newcgroup(struct cell_args *args)
{
	if (!args->start_args.newcgrp)
		return 0;

	return move_to_new_cgroup(args);
}

static int do_child(void *vargv)
{
	struct cell_args *cell_args = (struct cell_args *)vargv;
	struct cell_start_args *start_args = &cell_args->start_args;
	char **argv;
	char *rootdir;
	char *cellname;
	char *syserr;
	int ret;
	char buf[20];
	sigset_t sigset;
	/*char * selinuxcon;
	setcon("u:object_r:celld:s0");
	getcon(&selinuxcon);
	ALOGD("selinuxcon: %s",selinuxcon);
	freecon(selinuxcon);*/

	argv = cell_args->argv;
	cellname = cell_args->cellname;
	rootdir = cell_args->rootdir;


	ALOGD("Starting cell:");
	ALOGD("==============");
	ALOGD("start_args");
	ALOGD("----------");
	ALOGD("noopt: %d", start_args->noopt);
	ALOGD("uts_ns: %d", start_args->uts_ns);
	ALOGD("ipc_ns: %d", start_args->ipc_ns);
	ALOGD("user_ns: %d", start_args->user_ns);
	ALOGD("net_ns: %d", start_args->net_ns);
	ALOGD("pid_ns: %d", start_args->pid_ns);
	ALOGD("mount_ns: %d", start_args->mount_ns);
	ALOGD("mnt_rootfs: %d", start_args->mnt_rootfs);
	ALOGD("mnt_tmpfs: %d", start_args->mnt_tmpfs);
	ALOGD("newpts: %d", start_args->newpts);
	ALOGD("newcgrp: %d", start_args->newcgrp);
	ALOGD("share_dalvik_cache: %d", start_args->share_dalvik_cache);
	ALOGD("sdcard_branch: %d", start_args->sdcard_branch);
	ALOGD("open_console: %d", start_args->open_console);
	ALOGD("autoswitch: %d", start_args->autoswitch);
	ALOGD("pid_file: %s", start_args->pid_file);
	ALOGD("wait: %d", start_args->wait);
	ALOGD("\ncell_args");
	ALOGD("---------");
	ALOGD("cellname: %s", cell_args->cellname);
	ALOGD("rootdir: %s", cell_args->rootdir);
	ALOGD("init_pid: %d", cell_args->init_pid);
	ALOGD("restart_pid: %d", cell_args->restart_pid);
	ALOGD("argc: %d", cell_args->argc);
	ALOGD("cell_socket: %d", cell_args->cell_socket);

	/* reset out umask and sigmask for init */
	umask(0000);
	sigemptyset(&sigset);
	sigprocmask(SIG_SETMASK, &sigset, NULL);

	/* Make sure init doesn't kill CellD on bad cell errors */
	ret = setpgid(0, 0);
	if (ret < 0)
		ALOGE("error setting pgid: %s", strerror(errno));

	/* Close cell utility socket */
	close(cell_args->cell_socket);

	ALOGI("%s: do_child, mnt_rootfs:%d, rootdir=%s",
		  cellname, start_args->mnt_rootfs, rootdir);

	/* chroot... */
	if (chroot(rootdir)) {
		syserr = "chroot";
		goto out_err;
	}
	if (chdir("/")) {
		syserr = "chdir /";
		goto out_err;
	}

	ALOGD("%s: waiting for cgroup entry...", cellname);
	close(g_cgroup_pipe[1]);
	ret = read(g_cgroup_pipe[0], buf, sizeof(buf));
	close(g_cgroup_pipe[0]);
	if (ret == -1 || atoi(buf) < 1) {
		syserr = "cgroup entry";
		goto out_err;
	}

	/* check if we should remount devpts */
	if (start_args->newpts) {
		struct stat ptystat, ptsstat;
		argv++;
		ALOGD("%s: mounting /dev/pts", cellname);
		if (lstat("/dev/ptmx", &ptystat) < 0) {
			if (symlink("/dev/pts/ptmx", "/dev/ptmx") < 0) {
				syserr = "symlink /dev/pts/ptmx /dev/ptmx";
				goto out_err;
			}
			chmod("/dev/ptmx", 0666);
		} else if ((ptystat.st_mode & S_IFMT) != S_IFLNK) {
			syserr = "Error: /dev/ptmx must be a link to "
				 "/dev/pts/ptmx\n"
				 "do:\tchmod 666 /dev/pts/ptmx\n   "
				 "\trm /dev/ptmx\n   "
				 "\tln -s /dev/pts/ptmx /dev/ptmx\n";
			goto out_err;
		}

		/* create /dev/pts directory if doesn't exist */
		if (stat("/dev/pts", &ptsstat) < 0) {
			if (mkdir("/dev/pts", 0666) < 0) {
				syserr = "mkdir /dev/pts";
				goto out_err;
			}
		} else {
			/* if container had no /dev/pts mounted don't fret */
			umount2("/dev/pts", MNT_DETACH);
		}

		if (mount("pts", "/dev/pts", "devpts", 0, "ptmxmode=666,newinstance") < 0) {
			syserr = "mount -t devpts -o newinstance";
			goto out_err;
		}
	}

	close(cell_args->child_pipe[0]);
	buf[0] = 1;
	write(cell_args->child_pipe[1], buf, 1);
	close(cell_args->child_pipe[1]);

	ALOGD("%s: waiting for CellD...", cellname);
	close(cell_args->init_pipe[1]);
	ret = read(cell_args->init_pipe[0], buf, sizeof(buf));
	close(cell_args->init_pipe[0]);
	if (ret == -1 || atoi(buf) < 1) {
		syserr = "CellD communication";
		goto out_err;
	}
	ALOGD("%s: Starting init!", cellname);

	/* touch a file in / to indicate to /init that we're in a cell */
	int vmfd = open("/.cell",O_WRONLY|O_CREAT,0660);
	if(vmfd>=0){
		char value[PROPERTY_VALUE_MAX]; 
		property_get("persist.sys.exit", value, "1");
		if (strcmp(value, "0") == 0) {
			write(vmfd,"1",strlen("1"));
		}else{
			write(vmfd,"0",strlen("0"));
		}
		close(vmfd);
	}

	execve(cell_args->argv[0], cell_args->argv, NULL);
	syserr = "execve";

out_err:
	{
		int e = errno;
		ALOGE("ERROR{%s: errno=%d (%s)}", syserr, e, strerror(e));
	}
	return -1;
}

static int write_pid(char *pid_file, int pid)
{
	FILE *fp;

	if (!pid_file)
		return 0;

	fp = fopen(pid_file, "w");
	if (!fp)
		return -1;
	fprintf(fp, "%d", pid);
	fflush(fp);
	fclose(fp);
	return 0;
}

int do_share_dalvik_cache(char *root_path)
{
	char target[PATH_MAX];
	int ret = -1;

	ALOGI("Dalvik Cache: relocating %s/data/dalvik-cache...", root_path);

	snprintf(target, sizeof(target), "%s/data/dalvik-cache", root_path);
	mkdir(target, 0755);

	/* bind-mount the host's dalvik-cache directory into the cell */
	ret = mount("/data/dalvik-cache", target, "none", MS_BIND, 0);
	if (ret < 0)
		ALOGW("Couldn't share Dalvik cache");

	return (ret < 0) ? -1 : 0;
}

int mount_dev_tmpfs(char *root_path)
{
	char target[PATH_MAX];
	struct stat st;
	int ret = -1;
	
	snprintf(target, sizeof(target), "%s/dev", root_path);
	if (stat(target, &st) < 0) {
		/* try to create the directory */
		if (mkdir(target, 0755) < 0) {
			ALOGE("cannot create <root>/dev: %s", strerror(errno));
			return -1;
		}
	}

	ret = mount("tmpfs", target, "tmpfs", 0, "mode=0755");
	if (ret < 0) {
		ALOGE("unable to mount tmpfs: %s", strerror(errno));
		return -1;
	}

	return 0;
}

static int do_clone(struct cell_args *cell_args)
{
	struct cell_start_args *args = &cell_args->start_args;
	int pid;
	size_t stacksize = 4 * sysconf(_SC_PAGESIZE);
	void *childstack, *stack = malloc(stacksize);
	unsigned long flags;
	char buf[20];
	char veth_name[256];
	int cells_idx = 0;
	char *syserr;
	int e;

	if (!stack) {
		ALOGE("cannot allocate stack: %s", strerror(errno));
		return -1;
	}

	childstack = (char *)stack + stacksize;

	flags = SIGCHLD;
	if (args->uts_ns)
		flags |= CLONE_NEWUTS;
	if (args->ipc_ns)
		flags |= CLONE_NEWIPC;
	if (args->user_ns)
		flags |= CLONE_NEWUSER;
	if (args->net_ns)
		flags |= CLONE_NEWNET;
	if (args->pid_ns)
		flags |= CLONE_NEWPID;
	if (args->mount_ns)
		flags |= CLONE_NEWNS;

	pid = clone(do_child, childstack, flags, cell_args);

	free(stack);
	if (pid < 0) {
		ALOGE("clone: %s", strerror(errno));
		return -1;
	}
	if (gettimeofday(&cell_args->start_time, NULL) == -1)
		ALOGE("%s: gettimeofday failed: %s", __func__, strerror(errno));

	/*
	 * Put the new process in a cgroup if requested.
	 * Note that the child will block until we release it with a write
	 * into the global pipe. This ensures that all children of the cell's
	 * init process will inherit the cgroup (i.e. a child will _not_ be
	 * forked before we can put init into a cgroup)
	 */
	cell_args->init_pid = pid;
	do_newcgroup(cell_args);

	//cells_init_net(cell_args);

	snprintf(buf, sizeof(buf), "%d", pid);
	close(g_cgroup_pipe[0]);
	write(g_cgroup_pipe[1], buf, strlen(buf)+1);
	close(g_cgroup_pipe[1]);

	return pid;
}

/* Careful: When this is called, it's called from a different process.
 * That means, no signaling to celld's threads (ex: destroy_wifi) */
void tear_down_cell(struct cell_args *cell_args, struct pty_info *console_pty)
{
	struct cell_start_args *args = &cell_args->start_args;
	cleanup_cell_console(console_pty);

	if (unmount_all(cell_args->rootdir, args->mnt_rootfs) == -1)
		ALOGW("Couldn't unmount_all() on %s", cell_args->rootdir);
}

static void thread_exit_handler(int sig)
{
	pthread_exit(0);
}

/* sd is used for sending more detailed error messages to client.
 * console_pty is filled in after returning. pty_info.ptm will be -1 if no
 * console is requested. */
int cell_nsexec(int sd, struct cell_args *cell_args,
		char *name, struct pty_info *console_pty)
{
	struct cell_start_args *args = &cell_args->start_args;
	int pid = -1;
	char *rootdir = cell_args->rootdir;
	struct sigaction actions;
	int ret;

	/* Setup signal handler for SIGUSR2 (fake pthread_cancel) */
	memset(&actions, 0, sizeof(actions));
	sigemptyset(&actions.sa_mask);
	actions.sa_flags = 0;
	actions.sa_handler = thread_exit_handler;
	if (sigaction(SIGUSR2, &actions, NULL) < 0)
		ALOGE("sigaction(%s): %s", name, strerror(errno));

	/* pipe to synchronize child execution and entry into new cgroup */
	if (pipe(g_cgroup_pipe)) {
		ALOGE("pipe: %s", strerror(errno));
		send_msg(sd, "1 nsexec failed: pipe() failed");
		goto err_cleanup;
	}

	/* pipes to synchronize child start and CellD monitoring */
	if (pipe(cell_args->child_pipe) || pipe(cell_args->init_pipe)) {
		ALOGE("Can't create child/init pipes for '%s': %s",
			  name, strerror(errno));
		send_msg(sd, "1 nsexec failed: child/init pipe creating failed");
		goto err_cleanup;
	}

	if (args->mnt_rootfs) {
		if (mount_cell(name, args->sdcard_branch)) {
			ALOGE("couldn't mount '%s' rootfs: %d", name, errno);
			send_msg(sd, "1 nsexec failed: mount() rootfs failed");
			goto err_cleanup;
		}
	}

	/*if (args->mnt_tmpfs) {
		if (mount_dev_tmpfs(rootdir) < 0) {
			ALOGE("couldn't mount '%s' tmpfs: %d", name, errno);
			send_msg(sd, "1 nsexec failed: mount() tmpfs failed");
			goto err_cleanup;
		}
	}*/

	if (args->share_dalvik_cache)
		do_share_dalvik_cache(rootdir);

	if (args->open_console) {
		ALOGD("Opening console for '%s'", name);
		int ret = create_cell_console(rootdir, console_pty);
		if (ret < 0) {
			ALOGE("Couldn't open console in '%s'. "
				  "Continuing nsexec..", name);
			console_pty->ptm = -1;
		}
	} else
		console_pty->ptm = -1;

	cell_args->cell_socket = sd;
	ALOGI("Cloning '%s'", name);
	pid = do_clone(cell_args);

	if (pid == -1) {
		ALOGE("clone(%s) failed: tearing down cell", name);
		goto err_cleanup;
	}

	if(args->net_ns){
		config_host_net_work(pid);
	}

	write_pid(args->pid_file, pid);

	close(cell_args->child_pipe[1]);
	close(cell_args->init_pipe[0]);

	ALOGI("Successfully initialized '%s' with init PID %d", name, pid);
	return pid;

err_cleanup:
	tear_down_cell(cell_args, console_pty);
	return -1;
}
