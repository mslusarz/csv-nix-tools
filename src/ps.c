/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

/*
 * TODO:
 * - threads?
 * - figure out what to do with these columns:
 * 	wchan, tty, tpgid, exit_signal, *signals*, alarm, kstk*, flags
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <proc/readproc.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "merge_utils.h"
#include "usr-grp-query.h"
#include "utils.h"

static const struct option opts[] = {
	{"columns",		required_argument,	NULL, 'c'},
	{"merge",		no_argument,		NULL, 'M'},
	{"table-name",		required_argument,	NULL, 'N'},
	{"pid",			required_argument,	NULL, 'p'},
	{"show",		no_argument,		NULL, 's'},
	{"show-full",		no_argument,		NULL, 'S'},
	{"as-table",		no_argument,		NULL, 'T'},
	{"version",		no_argument,		NULL, 'V'},
	{"help",		no_argument,		NULL, 'h'},
	{NULL,			0,			NULL, 0},
};

static size_t PageSize;

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-ps [OPTION]...\n");
	fprintf(out, "Print to standard output the list of system processes in the CSV format.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	describe_Columns(out);
	fprintf(out, "  -l                         use a longer listing format (can be used up to 4 times)\n");
	describe_Merge(out);
	describe_table_Name(out);
	fprintf(out, "  -p, --pid=pid1[,pid2...]   select processes from this list\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_as_Table(out, "proc");
	describe_help(out);
	describe_version(out);
}

static void
print_inside_quote(const char *str, size_t len)
{
	const char *comma = strnchr(str, ',', len);
	const char *nl = strnchr(str, '\n', len);
	const char *quot = strnchr(str, '"', len);
	if (!comma && !nl && !quot) {
		fwrite(str, 1, len, stdout);
		return;
	}
	if (!quot) {
		fwrite(str, 1, len, stdout);
		return;
	}

	do {
		size_t curlen = (uintptr_t)quot - (uintptr_t)str + 1;
		fwrite(str, 1, curlen, stdout);
		str += curlen;
		fputc('"', stdout);
		len -= curlen;
		quot = strnchr(str, '"', len);
	} while (quot);

	fwrite(str, 1, len, stdout);
}

static unsigned long
get_hz(void)
{
	static unsigned long hz = 0;
	if (hz)
		return hz;
	long ret = sysconf(_SC_CLK_TCK);
	if (ret <= 0) {
		fprintf(stderr,
			"unable to determine HZ value (%ld %d), assuming 100\n",
			ret, errno);
		hz = 100;
	} else {
		hz = (unsigned long)ret;
	}

	return hz;
}

static inline unsigned long long
time_in_ms(unsigned long long t)
{
	return t * 1000 / get_hz();
}

static struct timespec boottime;

#define NSECS_IN_SEC 1000000000

static void
get_start_time(proc_t *proc, struct timespec *ts)
{
	unsigned long long ms = time_in_ms(proc->start_time);

	memcpy(ts, &boottime, sizeof(boottime));
	ts->tv_sec += (long)(ms / 1000);
	ts->tv_nsec += (long)(1000000 * (ms % 1000));
	if (ts->tv_nsec >= NSECS_IN_SEC) {
		ts->tv_sec++;
		ts->tv_nsec -= NSECS_IN_SEC;
	}
}

static bool
subtract_timespecs(const struct timespec *ts_later,
		const struct timespec *ts_earlier,
		struct timespec *ts)
{
	if (ts_later->tv_sec < ts_earlier->tv_sec)
		return false;
	if (ts_later->tv_sec == ts_earlier->tv_sec &&
			ts_later->tv_nsec < ts_earlier->tv_nsec)
		return false;

	long long diff =
			(ts_later->tv_sec - ts_earlier->tv_sec) * NSECS_IN_SEC +
			ts_later->tv_nsec - ts_earlier->tv_nsec;
	assert(diff >= 0);
	ts->tv_sec = diff / NSECS_IN_SEC;
	ts->tv_nsec = diff % NSECS_IN_SEC;

	return true;
}

/* static point in time from which ages are calculated */
static struct timespec ref_time;

struct proc_data {
	proc_t *proc;

	struct timespec start_time_ts;
	struct timespec age_ts;
};

static void
print_tid(const void *p)
{
	const struct proc_data *pd = p;
	printf("%d", pd->proc->tid);
}

static void
print_tgid(const void *p)
{
	const struct proc_data *pd = p;
	printf("%d", pd->proc->tgid);
}

static void
print_euid(const void *p)
{
	const struct proc_data *pd = p;
	printf("%d", pd->proc->euid);
}

static void
print_egid(const void *p)
{
	const struct proc_data *pd = p;
	printf("%d", pd->proc->egid);
}

static void
print_ppid(const void *p)
{
	const struct proc_data *pd = p;
	printf("%d", pd->proc->ppid);
}

static void
print_state(const void *p)
{
	const struct proc_data *pd = p;
	printf("%c", pd->proc->state);
}

static void
print_cmd(const void *p)
{
	const struct proc_data *pd = p;
	csv_print_quoted(pd->proc->cmd, strlen(pd->proc->cmd));
}

static void
print_nlwp(const void *p)
{
	const struct proc_data *pd = p;
	printf("%d", pd->proc->nlwp);
}

static void
print_wchan(const void *p)
{
	const struct proc_data *pd = p;
	if (pd->proc->wchan == (unsigned KLONG)-1)
		printf("-1");
	else
		printf("0x%lx", pd->proc->wchan);
}

static void
print_pgrp(const void *p)
{
	const struct proc_data *pd = p;
	printf("%d", pd->proc->pgrp);
}

static void
print_session(const void *p)
{
	const struct proc_data *pd = p;
	printf("%d", pd->proc->session);
}

static void
print_user_time_ms(const void *p)
{
	const struct proc_data *pd = p;
	printf("%llu", time_in_ms(pd->proc->utime));
}

static void
print_system_time_ms(const void *p)
{
	const struct proc_data *pd = p;
	printf("%llu", time_in_ms(pd->proc->stime));
}

static void
print_cumulative_user_time_ms(const void *p)
{
	const struct proc_data *pd = p;
	printf("%llu", time_in_ms(pd->proc->cutime));
}

static void
print_cumulative_system_time_ms(const void *p)
{
	const struct proc_data *pd = p;
	printf("%llu", time_in_ms(pd->proc->cstime));
}

static void
print_start_time_sec(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->start_time_ts.tv_sec);
}

static void
print_start_time_msec(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->start_time_ts.tv_nsec / 1000000);
}

static void
print_start_code(const void *p)
{
	const struct proc_data *pd = p;
	printf("0x%lx", pd->proc->start_code);
}

static void
print_end_code(const void *p)
{
	const struct proc_data *pd = p;
	printf("0x%lx", pd->proc->end_code);
}

static void
print_start_stack(const void *p)
{
	const struct proc_data *pd = p;
	printf("0x%lx", pd->proc->start_stack);
}

static void
print_kstk_esp(const void *p)
{
	const struct proc_data *pd = p;
	printf("0x%lx", pd->proc->kstk_esp);
}

static void
print_kstk_eip(const void *p)
{
	const struct proc_data *pd = p;
	printf("0x%lx", pd->proc->kstk_eip);
}

static void
print_priority(const void *p)
{
	const struct proc_data *pd = p;
	printf("%ld", pd->proc->priority);
}

static void
print_nice(const void *p)
{
	const struct proc_data *pd = p;
	printf("%ld", pd->proc->nice);
}

static void
print_long_in_pages(long l)
{
	assert(l >= 0);
	assert((size_t)l <= SIZE_MAX/PageSize);
	printf("%zu", (size_t)l * PageSize);
}

static void
print_rss_bytes(const void *p)
{
	const struct proc_data *pd = p;
	print_long_in_pages(pd->proc->rss);
}

static void
print_alarm(const void *p)
{
	const struct proc_data *pd = p;
	printf("%ld", pd->proc->alarm);
}

static void
print_rtprio(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->rtprio);
}

static void
print_sched(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->sched);
}

static void
print_vsize_bytes(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->vsize);
}

static void
print_rss_rlim(const void *p)
{
	const struct proc_data *pd = p;
	printf("%ld", pd->proc->rss_rlim);
}

static void
print_flags(const void *p)
{
	const struct proc_data *pd = p;
	printf("0x%lx", pd->proc->flags);
}

static void
print_min_flt(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->min_flt);
}

static void
print_maj_flt(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->maj_flt);
}

static void
print_cmin_flt(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->cmin_flt);
}

static void
print_cmaj_flt(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->cmaj_flt);
}

static void
print_tty_id(const void *p)
{
	const struct proc_data *pd = p;
	printf("0x%x", pd->proc->tty);
}

static void
print_tpgid(const void *p)
{
	const struct proc_data *pd = p;
	printf("%d", pd->proc->tpgid);
}

static void
print_exit_signal(const void *p)
{
	const struct proc_data *pd = p;
	printf("%d", pd->proc->exit_signal);
}

static void
print_processor(const void *p)
{
	const struct proc_data *pd = p;
	printf("%d", pd->proc->processor);
}

static void
print_pending_signals(const void *p)
{
	const struct proc_data *pd = p;
	csv_print_quoted(pd->proc->signal, strlen(pd->proc->signal));
}

static void
print_blocked_signals(const void *p)
{
	const struct proc_data *pd = p;
	csv_print_quoted(pd->proc->blocked, strlen(pd->proc->blocked));
}

static void
print_ignored_signals(const void *p)
{
	const struct proc_data *pd = p;
	csv_print_quoted(pd->proc->sigignore, strlen(pd->proc->sigignore));
}

static void
print_caught_signals(const void *p)
{
	const struct proc_data *pd = p;
	csv_print_quoted(pd->proc->sigcatch, strlen(pd->proc->sigcatch));
}

static void
print_pending_signals_per_task(const void *p)
{
	const struct proc_data *pd = p;
	csv_print_quoted(pd->proc->_sigpnd, strlen(pd->proc->_sigpnd));
}

static void
print_vm_size_bytes(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->vm_size * 1024);
}

static void
print_vm_size_KiB(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->vm_size);
}

static void
print_vm_lock_bytes(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->vm_lock * 1024);
}

static void
print_vm_rss_bytes(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->vm_rss * 1024);
}

static void
print_vm_rss_KiB(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->vm_rss);
}

static void
print_vm_rss_anon_bytes(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->vm_rss_anon * 1024);
}

static void
print_vm_rss_file_bytes(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->vm_rss_file * 1024);
}

static void
print_vm_rss_shared_bytes(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->vm_rss_shared * 1024);
}

static void
print_vm_data_bytes(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->vm_data * 1024);
}

static void
print_vm_stack_bytes(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->vm_stack * 1024);
}

static void
print_vm_swap_bytes(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->vm_swap * 1024);
}

static void
print_vm_exe_bytes(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->vm_exe * 1024);
}

static void
print_vm_lib_bytes(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->vm_lib * 1024);
}

static void
print_ruid(const void *p)
{
	const struct proc_data *pd = p;
	printf("%d", pd->proc->ruid);
}

static void
print_rgid(const void *p)
{
	const struct proc_data *pd = p;
	printf("%d", pd->proc->rgid);
}

static void
print_suid(const void *p)
{
	const struct proc_data *pd = p;
	printf("%d", pd->proc->suid);
}

static void
print_sgid(const void *p)
{
	const struct proc_data *pd = p;
	printf("%d", pd->proc->sgid);
}

static void
print_fuid(const void *p)
{
	const struct proc_data *pd = p;
	printf("%d", pd->proc->fuid);
}

static void
print_fgid(const void *p)
{
	const struct proc_data *pd = p;
	printf("%d", pd->proc->fgid);
}

static void
print_supgid(const void *p)
{
	const struct proc_data *pd = p;
	csv_print_quoted(pd->proc->supgid, strlen(pd->proc->supgid));
}

static void
print_size_bytes(const void *p)
{
	const struct proc_data *pd = p;
	print_long_in_pages(pd->proc->size);
}

static void
print_resident_bytes(const void *p)
{
	const struct proc_data *pd = p;
	print_long_in_pages(pd->proc->resident);
}

static void
print_share_bytes(const void *p)
{
	const struct proc_data *pd = p;
	print_long_in_pages(pd->proc->share);
}

static void
print_trs_bytes(const void *p)
{
	const struct proc_data *pd = p;
	print_long_in_pages(pd->proc->trs);
}

static void
print_drs_bytes(const void *p)
{
	const struct proc_data *pd = p;
	print_long_in_pages(pd->proc->drs);
}

static void
print_cmdline(const void *p)
{
	const struct proc_data *pd = p;
	putchar('"');

	char **cmd = pd->proc->cmdline;

	if (cmd) {
		while (*cmd) {
			print_inside_quote(*cmd, strlen(*cmd));
			cmd++;
			if (*cmd)
				putchar(',');
		}
	}

	putchar('"');
}

static void
print_command(const void *p)
{
	const struct proc_data *pd = p;
	putchar('"');

	char **cmd = pd->proc->cmdline;

	if (cmd) {
		while (*cmd) {
			print_inside_quote(*cmd, strlen(*cmd));
			cmd++;
			if (*cmd)
				putchar(' ');
		}
	} else {
		putchar('[');
		print_inside_quote(pd->proc->cmd, strlen(pd->proc->cmd));
		putchar(']');
	}

	putchar('"');
}

static void
print_cgroup(const void *p)
{
	const struct proc_data *pd = p;
	putchar('"');

	char **cgroup = pd->proc->cgroup;

	if (cgroup) {
		while (*cgroup) {
			print_inside_quote(*cgroup, strlen(*cgroup));
			cgroup++;
			if (*cgroup)
				putchar(',');
		}
	}

	putchar('"');
}

static void
print_oom_score(const void *p)
{
	const struct proc_data *pd = p;
	printf("%d", pd->proc->oom_score);
}

static void
print_oom_adj(const void *p)
{
	const struct proc_data *pd = p;
	printf("%d", pd->proc->oom_adj);
}

static void
print_ns_ipc(const void *p)
{
	const struct proc_data *pd = p;
	printf("0x%lx", pd->proc->ns[IPCNS]);
}

static void
print_ns_mnt(const void *p)
{
	const struct proc_data *pd = p;
	printf("0x%lx", pd->proc->ns[MNTNS]);
}

static void
print_ns_net(const void *p)
{
	const struct proc_data *pd = p;
	printf("0x%lx", pd->proc->ns[NETNS]);
}

static void
print_ns_pid(const void *p)
{
	const struct proc_data *pd = p;
	printf("0x%lx", pd->proc->ns[PIDNS]);
}

static void
print_ns_user(const void *p)
{
	const struct proc_data *pd = p;
	printf("0x%lx", pd->proc->ns[USERNS]);
}

static void
print_ns_uts(const void *p)
{
	const struct proc_data *pd = p;
	printf("0x%lx", pd->proc->ns[UTSNS]);
}

static void
print_sd_mach(const void *p)
{
	const struct proc_data *pd = p;
	csv_print_quoted(pd->proc->sd_mach, strlen(pd->proc->sd_mach));
}

static void
print_sd_ouid(const void *p)
{
	const struct proc_data *pd = p;
	csv_print_quoted(pd->proc->sd_ouid, strlen(pd->proc->sd_ouid));
}

static void
print_sd_seat(const void *p)
{
	const struct proc_data *pd = p;
	csv_print_quoted(pd->proc->sd_seat, strlen(pd->proc->sd_seat));
}

static void
print_sd_sess(const void *p)
{
	const struct proc_data *pd = p;
	csv_print_quoted(pd->proc->sd_sess, strlen(pd->proc->sd_sess));
}

static void
print_sd_slice(const void *p)
{
	const struct proc_data *pd = p;
	csv_print_quoted(pd->proc->sd_slice, strlen(pd->proc->sd_slice));
}

static void
print_sd_unit(const void *p)
{
	const struct proc_data *pd = p;
	csv_print_quoted(pd->proc->sd_unit, strlen(pd->proc->sd_unit));
}

static void
print_sd_uunit(const void *p)
{
	const struct proc_data *pd = p;
	csv_print_quoted(pd->proc->sd_uunit, strlen(pd->proc->sd_uunit));
}

static void
print_lxcname(const void *p)
{
	const struct proc_data *pd = p;
	csv_print_quoted(pd->proc->lxcname, strlen(pd->proc->lxcname));
}

static void
print_environ(const void *p)
{
	const struct proc_data *pd = p;
	if (pd->proc->environ)
		csv_print_quoted(pd->proc->environ[0], strlen(pd->proc->environ[0]));
}

static void
print_euid_name(const void *p)
{
	const struct proc_data *pd = p;
	printf("%s", get_user((uid_t)pd->proc->euid));
}

static void
print_egid_name(const void *p)
{
	const struct proc_data *pd = p;
	printf("%s", get_group((gid_t)pd->proc->egid));
}

static void
print_ruid_name(const void *p)
{
	const struct proc_data *pd = p;
	printf("%s", get_user((uid_t)pd->proc->ruid));
}

static void
print_rgid_name(const void *p)
{
	const struct proc_data *pd = p;
	printf("%s", get_group((gid_t)pd->proc->rgid));
}

static void
print_suid_name(const void *p)
{
	const struct proc_data *pd = p;
	printf("%s", get_user((uid_t)pd->proc->suid));
}

static void
print_sgid_name(const void *p)
{
	const struct proc_data *pd = p;
	printf("%s", get_group((gid_t)pd->proc->sgid));
}

static void
print_fuid_name(const void *p)
{
	const struct proc_data *pd = p;
	printf("%s", get_user((uid_t)pd->proc->fuid));
}

static void
print_fgid_name(const void *p)
{
	const struct proc_data *pd = p;
	printf("%s", get_group((gid_t)pd->proc->fgid));
}

static void
print_supgid_names(const void *p)
{
	const struct proc_data *pd = p;
	if (strcmp(pd->proc->supgid, "-") == 0)
		return;

	char *gid_str = strtok(pd->proc->supgid, ",");

	putchar('"');
	while (gid_str) {
		gid_t gid;
		if (strtou_safe(gid_str, &gid, 0)) {
			fprintf(stderr, "gid '%s' is not a number\n", gid_str);
			abort();
		}
		printf("%s", get_group(gid));

		gid_str = strtok(NULL, ",");
		if (gid_str)
			putchar(',');
	}
	putchar('"');
}

static void
print_cpu_time_ms(const void *p)
{
	const struct proc_data *pd = p;
	printf("%llu", time_in_ms(pd->proc->utime + pd->proc->stime));
}

static void
print_cpu_time(const void *p)
{
	const struct proc_data *pd = p;
	unsigned long long time = time_in_ms(pd->proc->utime + pd->proc->stime);

	unsigned ms = time % 1000;
	time /= 1000;
	unsigned s = time % 60;
	time /= 60;
	unsigned m = time % 60;
	time /= 60;
	unsigned h = time % 24;
	time /= 24;
	unsigned long long d = time;

	if (d > 0)
		printf("%llud:%02uh:%02um:%02u.%03us", d, h, m, s, ms);
	else if (h > 0)
		printf("%uh:%02um:%02u.%03us", h, m, s, ms);
	else if (m > 0)
		printf("%um:%02u.%03us", m, s, ms);
	else if (s > 0)
		printf("%u.%03us", s, ms);
	else if (ms > 0)
		printf("0.%03us", ms);
	else
		printf("0s");
}

static void
print_start_time(const void *p)
{
	const struct proc_data *pd = p;
	print_timespec(&pd->start_time_ts, false);
}

static void
print_age_sec(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->age_ts.tv_sec);
}

static void
print_age_msec(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->age_ts.tv_nsec / 1000000);
}

static void
print_age(const void *p)
{
	const struct proc_data *pd = p;
	assert(pd->age_ts.tv_sec >= 0);
	unsigned long long age = (unsigned long long)pd->age_ts.tv_sec;
	unsigned ms = pd->age_ts.tv_nsec / 1000000;

	unsigned s = age % 60;
	age /= 60;
	unsigned m = age % 60;
	age /= 60;
	unsigned h = age % 24;
	age /= 24;
	unsigned long long d = age;

	if (d > 0)
		printf("%llud:%02uh:%02um:%02u.%03us", d, h, m, s, ms);
	else if (h > 0)
		printf("%uh:%02um:%02u.%03us", h, m, s, ms);
	else if (m > 0)
		printf("%um:%02u.%03us", m, s, ms);
	else
		printf("%u.%03us", s, ms);
}

static void
print_cpu_percent(const void *p)
{
	const struct proc_data *pd = p;
	unsigned long long cpu_time_ms =
			time_in_ms(pd->proc->utime + pd->proc->stime);
	assert(pd->age_ts.tv_sec >= 0);
	assert(pd->age_ts.tv_nsec >= 0);
	unsigned long long age_ms = (unsigned long long)pd->age_ts.tv_sec * 1000 +
			(unsigned long long)pd->age_ts.tv_nsec / 1000000;

	if (cpu_time_ms > age_ms)
		cpu_time_ms = age_ms;

	if (age_ms == 0)
		printf("0");
	else
		printf("%llu", 100 * cpu_time_ms / age_ms);
}

static void
estimate_boottime_once(void)
{
	/*
	 * It's impossible to get boot time with sub-second precision from
	 * the Linux kernel. Infer boot time from current process start_time
	 * and current time.
	 */

	struct timeval now;
	if (gettimeofday(&now, NULL)) {
		perror("gettimeofday");
		abort();
	}

	pid_t pids[2] = { getpid(), 0 };
	PROCTAB *pt = openproc(PROC_FILLSTAT | PROC_PID, pids);
	if (!pt) {
		perror("openproc");
		exit(2);
	}

	proc_t *proc = readproc(pt, NULL);

	unsigned long long ms = time_in_ms(proc->start_time);
	boottime.tv_sec = now.tv_sec - (long)(ms / 1000);
	long start_nsec = (long)(ms % 1000) * 1000000;
	long now_nsec = now.tv_usec * 1000;
	if (now_nsec >= start_nsec)
		boottime.tv_nsec = now_nsec - start_nsec;
	else {
		boottime.tv_nsec = NSECS_IN_SEC + now_nsec - start_nsec;
		boottime.tv_sec--;
	}

#ifndef NDEBUG
	/* verify */
	struct timespec start_time;
	get_start_time(proc, &start_time);

	struct timespec start_ts;
	start_ts.tv_sec = now.tv_sec;
	start_ts.tv_nsec = now.tv_usec * 1000;

	struct timespec diff;
	assert(subtract_timespecs(&start_ts, &start_time, &diff) == true);
	assert(diff.tv_sec == 0);
	assert(diff.tv_nsec == 0);
#endif

	freeproc(proc);

	closeproc(pt);

	ref_time.tv_sec = now.tv_sec;
	ref_time.tv_nsec = now.tv_usec * 1000;
}

/* missing sub-second precision */
static long long
get_coarse_boottime()
{
	unsigned long long boottime = ULLONG_MAX;
	FILE *fp = fopen("/proc/stat", "r");
	if (!fp) {
		fprintf(stderr,
			"unable to retrieve boot time from the kernel (%d, %s)\n",
			errno, strerror(errno));
		return 0;
	}

	char *buf = NULL;
	size_t bufsize = 0;

	ssize_t nread;
	while ((nread = getline(&buf, &bufsize, fp)) >= 0) {
		if ((size_t)nread < strlen("btime ") + 1)
			continue;
		if (strncmp(buf, "btime ", strlen("btime ")) != 0)
			continue;

		if (buf[nread - 1] == '\n')
			buf[nread - 1] = 0;

		if (strtoull_safe(buf + strlen("btime "), &boottime, 0)) {
			fprintf(stderr,
				"unable to retrieve boot time from the kernel (strtoull failed)\n");
			boottime = 0;
		}

		break;
	}

	free(buf);
	fclose(fp);

	if (boottime == ULLONG_MAX) {
		fprintf(stderr,
			"unable to retrieve boot time from the kernel (btime not found)\n");
		boottime = 0;
	}

	if (boottime > LLONG_MAX) {
		fprintf(stderr, "kernel returned impossible boot time, ignoring\n");
		boottime = 0;
	}

	return (long long)boottime;
}

static void
estimate_boottime()
{
	/*
	 * Precise (< 1ms) estimation is costly (at least 40ms, but can take
	 * much longer if system is loaded), so look up cached time first
	 * (mtime of ~/.cache/csv-ps).
	 */
	char *path = NULL;
	const char *cache = getenv("XDG_CACHE_HOME");
	if (cache) {
		if (csv_asprintf(&path, "%s/csv-ps", cache) < 0) {
			perror("asprintf");
			return;
		}
	} else {
		const char *home = getenv("HOME");
		if (!home)
			return;
		if (csv_asprintf(&path, "%s/.cache", home) < 0) {
			perror("asprintf");
			return;
		}

		struct stat statbuf;
		if (stat(path, &statbuf)) {
			perror("stat $HOME/.cache");
			free(path);
			return;
		}
		free(path);

		if (csv_asprintf(&path, "%s/.cache/csv-ps", home) < 0) {
			perror("asprintf");
			return;
		}
	}

	long long coarse_bootime_sec = get_coarse_boottime();

	struct stat st;
	if (stat(path, &st) == 0) {
		long long diff = ((st.st_mtim.tv_sec - boottime.tv_sec) * NSECS_IN_SEC
				+ st.st_mtim.tv_nsec - boottime.tv_nsec);

		bool ok = true;

		/*
		 * If the difference between cached boot time and estimated
		 * boot time is positive, then the cached boot time is wrong
		 * (it was probably estimated while the system was under load)
		 *
		 * If coarse_boottime is available, then it must match seconds
		 * part of the cached boot time.
		 *
		 * If coarse_boottime is not available, then the difference
		 * of more than one second means that the cached boot time was
		 * probably calculated in a previous boot.
		 *
		 * Otherwise assume cached boot time is correct.
		 */
		if (diff > 0) {
			ok = false;
		} else if (coarse_bootime_sec != 0) {
			if (st.st_mtim.tv_sec != coarse_bootime_sec)
				ok = false;
		} else {
			if (-diff > NSECS_IN_SEC)
				ok = false;
		}

		if (ok) {
			boottime.tv_sec = st.st_mtim.tv_sec;
			boottime.tv_nsec = st.st_mtim.tv_nsec;

			goto end;
		}
	}

	/*
	 * If coarse_boottime is available and doesn't match seconds of
	 * the estimated boot time, then set the estimated boot time to the last
	 * nanosecond of coarse_boottime.
	 */
	if (coarse_bootime_sec && coarse_bootime_sec != boottime.tv_sec) {
		boottime.tv_sec = coarse_bootime_sec;
		boottime.tv_nsec = NSECS_IN_SEC - 1;
	}

// #define DEBUG_BOOTTIME 1

#if DEBUG_BOOTTIME
	struct timespec orig = boottime;
#endif
	/*
	 * fork multiple times, estimate boot time in each subprocess, transfer
	 * it to the parent process using a pipe and take the minimum value
	 */
	int iter = 0;
	for (int i = 0; i < 50 && iter < 200; ++i, ++iter) {
		int fd[2];
		if (pipe(fd)) {
			perror("pipe");
			goto end;
		}

		pid_t p = fork();
		if (p == 0) {
			estimate_boottime_once();

			(void)close(fd[0]);

			if (write(fd[1], &boottime, sizeof(boottime)) < 0)
				perror("write pipe");

			(void)close(fd[1]);

			exit(0);
		} else if (p > 0) {
			struct timespec t;

			(void)close(fd[1]);

			ssize_t r = read(fd[0], &t, sizeof(t));

			if (r == sizeof(t)) {
				/* find the minimum time */
				if (t.tv_sec < boottime.tv_sec ||
					(t.tv_sec == boottime.tv_sec &&
					 t.tv_nsec < boottime.tv_nsec)) {
					memcpy(&boottime, &t, sizeof(boottime));
					i = 0;
				}
			} else {
				fprintf(stderr, "read pipe: %ld %d %s\n", r,
						errno, strerror(errno));
			}

			(void)close(fd[0]);

			if (waitpid(p, NULL, 0) == -1)
				perror("waitpid");
		} else {
			perror("fork");
			goto end;
		}
	}

#if DEBUG_BOOTTIME
	fprintf(stderr, "loops: %d\n", iter);
	fprintf(stderr, "diff: %ldus\n",
			((orig.tv_sec - boottime.tv_sec) * NSECS_IN_SEC
			+ orig.tv_nsec - boottime.tv_nsec) / 1000);
	fprintf(stderr, "%ld.%ld\n", boottime.tv_sec, boottime.tv_nsec);
#endif

	/* it doesn't matter if directory already exists */
	if (mkdir(path, 0700))
		if (errno != EEXIST)
			perror("mkdir .cache/csv-ps");

	/* store boot time as mtime of ~/.cache/csv-ps */
	struct timespec times[2];
	memcpy(&times[0], &ref_time, sizeof(times[0]));
	memcpy(&times[1], &boottime, sizeof(times[1]));
	if (utimensat(AT_FDCWD, path, times, 0)) {
		fprintf(stderr, "utimensat %s failed: %s\n", path,
				strerror(errno));
	}
end:
	free(path);
}

int
main(int argc, char *argv[])
{
	int opt;
	char *cols = NULL;
	bool show = false;
	bool show_full;
	pid_t *pids = NULL;
	size_t npids = 0;
	bool merge = false;
	char *table = NULL;

	estimate_boottime_once();

	long sysconf_ret = sysconf(_SC_PAGESIZE);
	if (sysconf_ret < 0) {
		perror("sysconf");
		exit(2);
	}

	PageSize = (size_t)sysconf_ret;

	enum {
		DEF            = 0,
		STAT           = 1 << 0,
		STATUS         = 1 << 1,
		STAT_OR_STATUS = 1 << 2,
		STATM          = 1 << 3,
		CMDLINE        = 1 << 4,
		ENVIRON        = 1 << 5,
		CGROUP         = 1 << 6,
		OOM            = 1 << 7,
		NS             = 1 << 8,
		SD             = 1 << 9,
		LXC            = 1 << 10,
		USR_GRP        = 1 << 11,
		START_TIME     = 1 << 12,
		AGE            = 1 << 13
	};

	struct column_info columns[] = {
		{ true,  0, 0, "euid_name",     TYPE_STRING, print_euid_name, DEF | USR_GRP },
		{ true,  0, 0, "pid",           TYPE_INT,    print_tgid, DEF },
		{ true,  0, 0, "ppid",          TYPE_INT,    print_ppid, STAT_OR_STATUS },
		{ true,  0, 0, "%cpu",          TYPE_INT,    print_cpu_percent, STAT | AGE },
		{ true,  0, 0, "vm_size_KiB",   TYPE_INT,    print_vm_size_KiB, STATUS },
		{ true,  0, 0, "vm_rss_KiB",    TYPE_INT,    print_vm_rss_KiB, STATUS },
		{ true,  0, 0, "tty_id",        TYPE_INT,    print_tty_id, STAT },
		{ true,  0, 0, "state",         TYPE_STRING, print_state, STAT_OR_STATUS },
		{ true,  0, 0, "start_time",    TYPE_STRING, print_start_time, STAT | START_TIME },
		{ true,  0, 0, "cpu_time",      TYPE_STRING, print_cpu_time, STAT },
		{ true,  0, 0, "processor",     TYPE_INT,    print_processor, STAT },
		{ true,  0, 0, "command",       TYPE_STRING, print_command, STAT_OR_STATUS | CMDLINE },

		{ false, 0, 1, "euid",          TYPE_INT,    print_euid, DEF },
		{ false, 0, 1, "tid",           TYPE_INT,    print_tid, DEF },
		{ false, 0, 1, "cpu_time_ms",   TYPE_INT,    print_cpu_time_ms, STAT },

		{ false, 0, 1, "age",           TYPE_STRING, print_age, STAT | START_TIME | AGE },

		{ false, 0, 1, "egid_name",     TYPE_STRING, print_egid_name, DEF | USR_GRP },
		{ false, 0, 1, "ruid_name",     TYPE_STRING, print_ruid_name, STATUS | USR_GRP },
		{ false, 0, 1, "rgid_name",     TYPE_STRING, print_rgid_name, STATUS | USR_GRP },
		{ false, 0, 1, "suid_name",     TYPE_STRING, print_suid_name, STATUS | USR_GRP },
		{ false, 0, 1, "sgid_name",     TYPE_STRING, print_sgid_name, STATUS | USR_GRP },
		{ false, 0, 1, "fuid_name",     TYPE_STRING, print_fuid_name, STATUS | USR_GRP },
		{ false, 0, 1, "fgid_name",     TYPE_STRING, print_fgid_name, STATUS | USR_GRP },
		{ false, 0, 1, "supgid_names",  TYPE_STRING, print_supgid_names, STATUS | USR_GRP },

		{ false, 0, 2, "egid",          TYPE_INT,    print_egid, DEF },
		{ false, 0, 2, "ruid",          TYPE_INT, print_ruid, STATUS },
		{ false, 0, 2, "rgid",          TYPE_INT, print_rgid, STATUS },
		{ false, 0, 2, "suid",          TYPE_INT, print_suid, STATUS },
		{ false, 0, 2, "sgid",          TYPE_INT, print_sgid, STATUS },
		{ false, 0, 2, "fuid",          TYPE_INT, print_fuid, STATUS },
		{ false, 0, 2, "fgid",          TYPE_INT, print_fgid, STATUS },
		{ false, 0, 2, "supgid",        TYPE_STRING, print_supgid, STATUS },

		{ false, 0, 2, "age_sec",       TYPE_INT,    print_age_sec, STAT | START_TIME | AGE },
		{ false, 0, 2, "age_msec",      TYPE_INT,    print_age_msec, STAT | START_TIME | AGE },

		{ false, 0, 2, "vm_size_B",     TYPE_INT, print_vm_size_bytes, STATUS },
		{ false, 0, 2, "vm_lock_B",     TYPE_INT, print_vm_lock_bytes, STATUS },
		{ false, 0, 2, "vm_rss_B",      TYPE_INT, print_vm_rss_bytes, STATUS },
		{ false, 0, 2, "vm_rss_anon_B", TYPE_INT, print_vm_rss_anon_bytes, STATUS },
		{ false, 0, 2, "vm_rss_file_B", TYPE_INT, print_vm_rss_file_bytes, STATUS },
		{ false, 0, 2, "vm_rss_shared_B", TYPE_INT, print_vm_rss_shared_bytes, STATUS },
		{ false, 0, 2, "vm_data_B",     TYPE_INT, print_vm_data_bytes, STATUS },
		{ false, 0, 2, "vm_stack_B",    TYPE_INT, print_vm_stack_bytes, STATUS },
		{ false, 0, 2, "vm_swap_B",     TYPE_INT, print_vm_swap_bytes, STATUS },
		{ false, 0, 2, "vm_exe_B",      TYPE_INT, print_vm_exe_bytes, STATUS },
		{ false, 0, 2, "vm_lib_B",      TYPE_INT, print_vm_lib_bytes, STATUS },

		{ false, 0, 2, "priority",      TYPE_INT, print_priority, STAT },
		{ false, 0, 2, "nice",          TYPE_INT, print_nice, STAT },

		{ false, 0, 3, "cmd",           TYPE_STRING, print_cmd, STAT_OR_STATUS },
		{ false, 0, 3, "nlwp",          TYPE_INT,    print_nlwp, STAT_OR_STATUS },
		{ false, 0, 3, "wchan",         TYPE_INT,    print_wchan, STAT_OR_STATUS },

		{ false, 0, 3, "pgrp",          TYPE_INT,    print_pgrp, STAT },
		{ false, 0, 3, "session",       TYPE_INT,    print_session, STAT },

		{ false, 0, 3, "user_time_ms",              TYPE_INT, print_user_time_ms, STAT },
		{ false, 0, 3, "system_time_ms",            TYPE_INT, print_system_time_ms, STAT },
		{ false, 0, 3, "cumulative_user_time_ms",   TYPE_INT, print_cumulative_user_time_ms, STAT },
		{ false, 0, 3, "cumulative_system_time_ms", TYPE_INT, print_cumulative_system_time_ms, STAT },
		{ false, 0, 3, "start_time_sec",            TYPE_INT, print_start_time_sec, STAT | START_TIME },
		{ false, 0, 3, "start_time_msec",           TYPE_INT, print_start_time_msec, STAT | START_TIME },

		{ false, 0, 3, "rss_B",         TYPE_INT, print_rss_bytes, STAT },
		{ false, 0, 3, "rtprio",        TYPE_INT, print_rtprio, STAT },
		{ false, 0, 3, "sched",         TYPE_INT, print_sched, STAT },
		{ false, 0, 3, "min_flt",       TYPE_INT, print_min_flt, STAT },
		{ false, 0, 3, "maj_flt",       TYPE_INT, print_maj_flt, STAT },
		{ false, 0, 3, "cmin_flt",      TYPE_INT, print_cmin_flt, STAT },
		{ false, 0, 3, "cmaj_flt",      TYPE_INT, print_cmaj_flt, STAT },
		{ false, 0, 3, "tpgid",         TYPE_INT, print_tpgid, STAT },

		{ false, 0, 3, "size_B",        TYPE_INT, print_size_bytes, STATM },
		{ false, 0, 3, "resident_B",    TYPE_INT, print_resident_bytes, STATM },
		{ false, 0, 3, "share_B",       TYPE_INT, print_share_bytes, STATM },
		{ false, 0, 3, "trs_B",         TYPE_INT, print_trs_bytes, STATM },
		{ false, 0, 3, "drs_B",         TYPE_INT, print_drs_bytes, STATM },

		{ false, 0, 3, "cmdline",       TYPE_STRING_ARR, print_cmdline, CMDLINE },

		{ false, 0, 4, "alarm",         TYPE_INT, print_alarm, STAT },

		{ false, 0, 4, "start_code",    TYPE_INT, print_start_code, STAT },
		{ false, 0, 4, "end_code",      TYPE_INT, print_end_code, STAT },
		{ false, 0, 4, "start_stack",   TYPE_INT, print_start_stack, STAT },
		{ false, 0, 4, "kstk_esp",      TYPE_INT, print_kstk_esp, STAT },
		{ false, 0, 4, "kstk_eip",      TYPE_INT, print_kstk_eip, STAT },

		{ false, 0, 4, "vsize_B",       TYPE_INT, print_vsize_bytes, STAT },
		{ false, 0, 4, "rss_rlim",      TYPE_INT, print_rss_rlim, STAT },
		{ false, 0, 4, "flags",         TYPE_INT, print_flags, STAT },

		{ false, 0, 4, "exit_signal",   TYPE_INT, print_exit_signal, STAT },

		{ false, 0, 4, "pending_signals",          TYPE_STRING, print_pending_signals, STATUS },
		{ false, 0, 4, "blocked_signals",          TYPE_STRING, print_blocked_signals, STATUS },
		{ false, 0, 4, "ignored_signals",          TYPE_STRING, print_ignored_signals, STATUS },
		{ false, 0, 4, "caught_signals",           TYPE_STRING, print_caught_signals, STATUS },
		{ false, 0, 4, "pending_signals_per_task", TYPE_STRING, print_pending_signals_per_task, STATUS },

		{ false, 0, 4, "oom_score",     TYPE_INT, print_oom_score, OOM },
		{ false, 0, 4, "oom_adj",       TYPE_INT, print_oom_adj, OOM },

		{ false, 0, 4, "ns_ipc",        TYPE_INT, print_ns_ipc, NS },
		{ false, 0, 4, "ns_mnt",        TYPE_INT, print_ns_mnt, NS },
		{ false, 0, 4, "ns_net",        TYPE_INT, print_ns_net, NS },
		{ false, 0, 4, "ns_pid",        TYPE_INT, print_ns_pid, NS },
		{ false, 0, 4, "ns_user",       TYPE_INT, print_ns_user, NS },
		{ false, 0, 4, "ns_uts",        TYPE_INT, print_ns_uts, NS },

		{ false, 0, 4, "sd_mach",       TYPE_STRING, print_sd_mach, SD },
		{ false, 0, 4, "sd_ouid",       TYPE_STRING, print_sd_ouid, SD },
		{ false, 0, 4, "sd_seat",       TYPE_STRING, print_sd_seat, SD },
		{ false, 0, 4, "sd_sess",       TYPE_STRING, print_sd_sess, SD },
		{ false, 0, 4, "sd_slice",      TYPE_STRING, print_sd_slice, SD },
		{ false, 0, 4, "sd_unit",       TYPE_STRING, print_sd_unit, SD },
		{ false, 0, 4, "sd_uunit",      TYPE_STRING, print_sd_uunit, SD },

		{ false, 0, 4, "lxcname",       TYPE_STRING, print_lxcname, LXC },

		{ false, 0, 4, "environ",       TYPE_STRING, print_environ, ENVIRON },
		{ false, 0, 4, "cgroup",        TYPE_STRING_ARR, print_cgroup, CGROUP },
	};

	size_t ncolumns = ARRAY_SIZE(columns);
	size_t level = 0;

	while ((opt = getopt_long(argc, argv, "c:lMN:p:sST", opts, NULL)) != -1) {
		switch (opt) {
			case 'c':
				cols = xstrdup_nofail(optarg);
				break;
			case 'l':
				level++;
				for (size_t i = 0; i < ncolumns; ++i)
					if (columns[i].level <= level)
						columns[i].vis = true;
				break;
			case 'M':
				merge = true;
				if (!table)
					table = xstrdup_nofail("proc");
				break;
			case 'N':
				table = xstrdup_nofail(optarg);
				break;
			case 'p': {
				char *pid = strtok(optarg, ",");
				while (pid) {
					pids = xrealloc_nofail(pids, npids + 2,
							sizeof(pids[0]));

					if (strtoi_safe(pid, &pids[npids], 0))
						exit(2);
					npids++;

					pid = strtok(NULL, ",");
				}
				pids[npids] = 0;

				break;
			}
			case 's':
				show = true;
				show_full = false;
				break;
			case 'S':
				show = true;
				show_full = true;
				break;
			case 'T':
				table = xstrdup_nofail("proc");
				break;
			case 'V':
				printf("git\n");
				return 0;
			case 'h':
			default:
				usage(stdout);
				return 2;
		}
	}

	if (cols) {
		csvci_parse_cols_nofail(cols, columns, &ncolumns);

		free(cols);
	} else {
		csvci_set_columns_order(columns, &ncolumns);
	}

	uint64_t sources = 0;
	for (size_t i = 0; i < ncolumns; ++i)
		sources |= columns[i].data;

	/* estimate_boottime may fork, which means we have to call it before
	 * anything goes to stdout, as stdout can be flushed in each process */
	if (sources & START_TIME)
		estimate_boottime();

	if (show)
		csv_show(show_full);

	struct csvmu_ctx ctx;
	ctx.table = table;
	ctx.merge = merge;

	csvmu_print_header(&ctx, columns, ncolumns);

	if (sources & USR_GRP)
		usr_grp_query_init();

	int flags = 0;

	if (sources & STAT)
		flags |= PROC_FILLSTAT;
	if (sources & STATUS)
		flags |= PROC_FILLSTATUS;
	if (sources & STAT_OR_STATUS) {
		if ((flags & (PROC_FILLSTAT | PROC_FILLSTATUS)) == 0)
			/* stat seems easier to parse */
			flags |= PROC_FILLSTAT;
	}
	if (sources & STATM)
		flags |= PROC_FILLMEM;
	if (sources & CMDLINE)
		flags |= PROC_FILLCOM;
	if (sources & ENVIRON)
		flags |= PROC_FILLENV | PROC_EDITENVRCVT;
	if (sources & CGROUP)
		flags |= PROC_FILLCGROUP;
	if (sources & OOM)
		flags |= PROC_FILLOOM;
	if (sources & NS)
		flags |= PROC_FILLNS;
	if (sources & SD)
		flags |= PROC_FILLSYSTEMD;
	if (sources & LXC)
		flags |= PROC_FILL_LXC;
	if (npids)
		flags |= PROC_PID;

	PROCTAB *pt = openproc(flags, pids);
	if (!pt) {
		perror("openproc");
		exit(2);
	}

	proc_t *proc;

	while ((proc = readproc(pt, NULL)) != NULL) {
		struct proc_data pd;
		pd.proc = proc;

		if (sources & START_TIME)
			get_start_time(proc, &pd.start_time_ts);
		if (sources & AGE) {
			if (!subtract_timespecs(&ref_time, &pd.start_time_ts,
					&pd.age_ts)) {
				/* if process was created after csv-ps, then
				 * pretend it was created at the same time */
				pd.age_ts.tv_sec = 0;
				pd.age_ts.tv_nsec = 0;
			}
		}

		csvmu_print_row(&ctx, &pd, columns, ncolumns);

		freeproc(proc);
	}

	closeproc(pt);

	free(ctx.table);
	free(pids);

	if (sources & USR_GRP)
		usr_grp_query_fini();

	return 0;
}
