/*
 * Copyright 2019, Marcin Ślusarz <marcin.slusarz@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * TODO:
 * - better default list of columns (ps-like or top-like?)
 * - better column names
 * - verify units/mults (pages/mb/kb/b)
 * - tree view?
 * - threads?
 * - figure out what to do with these columns:
 * 	wchan, tty, tpgid, exit_signal, *signals*, alarm, kstk*, flags
 */

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <proc/readproc.h>
#include <sys/time.h>

#include "usr-grp-query.h"
#include "utils.h"

static const struct option long_options[] = {
	{"fields",	required_argument,	NULL, 'f'},
	{"no-header",	no_argument,		NULL, 'H'},
	{"pid",		required_argument,	NULL, 'p'},
	{"show",	no_argument,		NULL, 's'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static size_t PageSize;

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-ps [OPTION]...\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -f, --fields=name1[,name2...]\n");
	fprintf(out, "  -p, --pid=pid1[,pid2...]\n");
	fprintf(out, "  -s, --show\n");
	fprintf(out, "      --no-header\n");
	fprintf(out, "      --help\n");
	fprintf(out, "      --version\n");
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
	ts->tv_sec += ms / 1000;
	ts->tv_nsec += 1000000 * (ms % 1000);
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

	unsigned long long diff =
			(ts_later->tv_sec - ts_earlier->tv_sec) * NSECS_IN_SEC +
			ts_later->tv_nsec - ts_earlier->tv_nsec;
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
	if (pd->proc->wchan == -1)
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
print_rss(const void *p)
{
	const struct proc_data *pd = p;
	printf("%ld", pd->proc->rss * PageSize);
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
print_vsize(const void *p)
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
print_tty(const void *p)
{
	const struct proc_data *pd = p;
	printf("%d", pd->proc->tty);
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
print_vm_size(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->vm_size * 1024);
}

static void
print_vm_lock(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->vm_lock * 1024);
}

static void
print_vm_rss(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->vm_rss * 1024);
}

static void
print_vm_rss_anon(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->vm_rss_anon * 1024);
}

static void
print_vm_rss_file(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->vm_rss_file * 1024);
}

static void
print_vm_rss_shared(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->vm_rss_shared * 1024);
}

static void
print_vm_data(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->vm_data * 1024);
}

static void
print_vm_stack(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->vm_stack * 1024);
}

static void
print_vm_swap(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->vm_swap * 1024);
}

static void
print_vm_exe(const void *p)
{
	const struct proc_data *pd = p;
	printf("%lu", pd->proc->vm_exe * 1024);
}

static void
print_vm_lib(const void *p)
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
print_size(const void *p)
{
	const struct proc_data *pd = p;
	printf("%ld", pd->proc->size * PageSize);
}

static void
print_resident(const void *p)
{
	const struct proc_data *pd = p;
	printf("%ld", pd->proc->resident * PageSize);
}

static void
print_share(const void *p)
{
	const struct proc_data *pd = p;
	printf("%ld", pd->proc->share * PageSize);
}

static void
print_trs(const void *p)
{
	const struct proc_data *pd = p;
	printf("%ld", pd->proc->trs * PageSize);
}

static void
print_drs(const void *p)
{
	const struct proc_data *pd = p;
	printf("%ld", pd->proc->drs * PageSize);
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
	printf("%s", get_user(pd->proc->euid));
}

static void
print_egid_name(const void *p)
{
	const struct proc_data *pd = p;
	printf("%s", get_group(pd->proc->egid));
}

static void
print_ruid_name(const void *p)
{
	const struct proc_data *pd = p;
	printf("%s", get_user(pd->proc->ruid));
}

static void
print_rgid_name(const void *p)
{
	const struct proc_data *pd = p;
	printf("%s", get_group(pd->proc->rgid));
}

static void
print_suid_name(const void *p)
{
	const struct proc_data *pd = p;
	printf("%s", get_user(pd->proc->suid));
}

static void
print_sgid_name(const void *p)
{
	const struct proc_data *pd = p;
	printf("%s", get_group(pd->proc->sgid));
}

static void
print_fuid_name(const void *p)
{
	const struct proc_data *pd = p;
	printf("%s", get_user(pd->proc->fuid));
}

static void
print_fgid_name(const void *p)
{
	const struct proc_data *pd = p;
	printf("%s", get_group(pd->proc->fgid));
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
print_time_ms(const void *p)
{
	const struct proc_data *pd = p;
	printf("%llu", time_in_ms(pd->proc->utime + pd->proc->stime));
}

static void
print_time(const void *p)
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
	unsigned long long age = pd->age_ts.tv_sec;
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
calibrate_boottime(void)
{
	/*
	 * It's impossible to get boot time with sub-second precision from
	 * the Linux kernel. Infer boot time from current process start_time
	 * and current time.
	 */

	struct timeval start;
	if (gettimeofday(&start, NULL)) {
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
	boottime.tv_sec = start.tv_sec - ms / 1000;
	if (start.tv_usec * 1000 >= (ms % 1000) * 1000000)
		boottime.tv_nsec = start.tv_usec * 1000 - (ms % 1000) * 1000000;
	else {
		boottime.tv_nsec = NSECS_IN_SEC + start.tv_usec * 1000 - (ms % 1000) * 1000000;
		boottime.tv_sec--;
	}

#ifndef NDEBUG
	/* verify */
	struct timespec start_time;
	get_start_time(proc, &start_time);

	struct timespec start_ts;
	start_ts.tv_sec = start.tv_sec;
	start_ts.tv_nsec = start.tv_usec * 1000;

	struct timespec diff;
	assert(subtract_timespecs(&start_ts, &start_time, &diff) == true);
	assert(diff.tv_sec == 0);
	assert(diff.tv_nsec == 0);
#endif

	freeproc(proc);

	closeproc(pt);

	ref_time.tv_sec = start.tv_sec;
	ref_time.tv_nsec = start.tv_usec * 1000;
}

int
main(int argc, char *argv[])
{
	int opt;
	int longindex;
	char *cols = NULL;
	bool print_header = true;
	bool show = false;
	pid_t *pids = NULL;
	size_t npids = 0;

	calibrate_boottime();

	PageSize = sysconf(_SC_PAGESIZE);

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
		/* always available */
		{ true, 0, "tid",           TYPE_INT,    print_tid, DEF },
		{ true, 0, "tgid",          TYPE_INT,    print_tgid, DEF },
		{ true, 0, "euid",          TYPE_INT,    print_euid, DEF },
		{ true, 0, "egid",          TYPE_INT,    print_egid, DEF },

		/* from stat or status */
		{ true, 0, "ppid",          TYPE_INT,    print_ppid, STAT_OR_STATUS },
		{ true, 0, "state",         TYPE_STRING, print_state, STAT_OR_STATUS },
		{ true, 0, "cmd",           TYPE_STRING, print_cmd, STAT_OR_STATUS },
		{ true, 0, "nlwp",          TYPE_INT,    print_nlwp, STAT_OR_STATUS },
		{ true, 0, "wchan",         TYPE_INT,    print_wchan, STAT_OR_STATUS },

		/* from stat */
		{ true, 0, "pgrp",          TYPE_INT,    print_pgrp, STAT },
		{ true, 0, "session",       TYPE_INT,    print_session, STAT },

		{ true, 0, "user_time_ms",              TYPE_INT, print_user_time_ms, STAT },
		{ true, 0, "system_time_ms",            TYPE_INT, print_system_time_ms, STAT },
		{ true, 0, "cumulative_user_time_ms",   TYPE_INT, print_cumulative_user_time_ms, STAT },
		{ true, 0, "cumulative_system_time_ms", TYPE_INT, print_cumulative_system_time_ms, STAT },
		{ true, 0, "start_time_sec",            TYPE_INT, print_start_time_sec, STAT | START_TIME },
		{ true, 0, "start_time_msec",           TYPE_INT, print_start_time_msec, STAT | START_TIME },

		{ true, 0, "start_code",    TYPE_INT, print_start_code, STAT },
		{ true, 0, "end_code",      TYPE_INT, print_end_code, STAT },
		{ true, 0, "start_stack",   TYPE_INT, print_start_stack, STAT },
		{ true, 0, "kstk_esp",      TYPE_INT, print_kstk_esp, STAT },
		{ true, 0, "kstk_eip",      TYPE_INT, print_kstk_eip, STAT },

		{ true, 0, "priority",      TYPE_INT, print_priority, STAT },
		{ true, 0, "nice",          TYPE_INT, print_nice, STAT },
		{ true, 0, "rss",           TYPE_INT, print_rss, STAT },
		{ true, 0, "alarm",         TYPE_INT, print_alarm, STAT },

		{ true, 0, "rtprio",        TYPE_INT, print_rtprio, STAT },
		{ true, 0, "sched",         TYPE_INT, print_sched, STAT },
		{ true, 0, "vsize",         TYPE_INT, print_vsize, STAT },
		{ true, 0, "rss_rlim",      TYPE_INT, print_rss_rlim, STAT },
		{ true, 0, "flags",         TYPE_INT, print_flags, STAT },
		{ true, 0, "min_flt",       TYPE_INT, print_min_flt, STAT },
		{ true, 0, "maj_flt",       TYPE_INT, print_maj_flt, STAT },
		{ true, 0, "cmin_flt",      TYPE_INT, print_cmin_flt, STAT },
		{ true, 0, "cmaj_flt",      TYPE_INT, print_cmaj_flt, STAT },

		{ true, 0, "tty",           TYPE_INT, print_tty, STAT },
		{ true, 0, "tpgid",         TYPE_INT, print_tpgid, STAT },
		{ true, 0, "exit_signal",   TYPE_INT, print_exit_signal, STAT },
		{ true, 0, "processor",     TYPE_INT, print_processor, STAT },

		/* from status */
		{ true, 0, "pending_signals",          TYPE_STRING, print_pending_signals, STATUS },
		{ true, 0, "blocked_signals",          TYPE_STRING, print_blocked_signals, STATUS },
		{ true, 0, "ignored_signals",          TYPE_STRING, print_ignored_signals, STATUS },
		{ true, 0, "caught_signals",           TYPE_STRING, print_caught_signals, STATUS },
		{ true, 0, "pending_signals_per_task", TYPE_STRING, print_pending_signals_per_task, STATUS },

		{ true, 0, "vm_size",       TYPE_INT, print_vm_size, STATUS },
		{ true, 0, "vm_lock",       TYPE_INT, print_vm_lock, STATUS },
		{ true, 0, "vm_rss",        TYPE_INT, print_vm_rss, STATUS },
		{ true, 0, "vm_rss_anon",   TYPE_INT, print_vm_rss_anon, STATUS },
		{ true, 0, "vm_rss_file",   TYPE_INT, print_vm_rss_file, STATUS },
		{ true, 0, "vm_rss_shared", TYPE_INT, print_vm_rss_shared, STATUS },
		{ true, 0, "vm_data",       TYPE_INT, print_vm_data, STATUS },
		{ true, 0, "vm_stack",      TYPE_INT, print_vm_stack, STATUS },
		{ true, 0, "vm_swap",       TYPE_INT, print_vm_swap, STATUS },
		{ true, 0, "vm_exe",        TYPE_INT, print_vm_exe, STATUS },
		{ true, 0, "vm_lib",        TYPE_INT, print_vm_lib, STATUS },

		{ true, 0, "ruid",          TYPE_INT, print_ruid, STATUS },
		{ true, 0, "rgid",          TYPE_INT, print_rgid, STATUS },
		{ true, 0, "suid",          TYPE_INT, print_suid, STATUS },
		{ true, 0, "sgid",          TYPE_INT, print_sgid, STATUS },
		{ true, 0, "fuid",          TYPE_INT, print_fuid, STATUS },
		{ true, 0, "fgid",          TYPE_INT, print_fgid, STATUS },

		{ true, 0, "supgid",        TYPE_STRING, print_supgid, STATUS },

		/* from statm */
		{ true, 0, "size",          TYPE_INT, print_size, STATM },
		{ true, 0, "resident",      TYPE_INT, print_resident, STATM },
		{ true, 0, "share",         TYPE_INT, print_share, STATM },
		{ true, 0, "trs",           TYPE_INT, print_trs, STATM },
		{ true, 0, "drs",           TYPE_INT, print_drs, STATM },

		/* other */
		{ true, 0, "cmdline",       TYPE_STRING_ARR, print_cmdline, CMDLINE },
		{ true, 0, "cgroup",        TYPE_STRING_ARR, print_cgroup, CGROUP },

		{ true, 0, "oom_score",     TYPE_INT, print_oom_score, OOM },
		{ true, 0, "oom_adj",       TYPE_INT, print_oom_adj, OOM },

		{ true, 0, "ns_ipc",        TYPE_INT, print_ns_ipc, NS },
		{ true, 0, "ns_mnt",        TYPE_INT, print_ns_mnt, NS },
		{ true, 0, "ns_net",        TYPE_INT, print_ns_net, NS },
		{ true, 0, "ns_pid",        TYPE_INT, print_ns_pid, NS },
		{ true, 0, "ns_user",       TYPE_INT, print_ns_user, NS },
		{ true, 0, "ns_uts",        TYPE_INT, print_ns_uts, NS },

		{ true, 0, "sd_mach",       TYPE_STRING, print_sd_mach, SD },
		{ true, 0, "sd_ouid",       TYPE_STRING, print_sd_ouid, SD },
		{ true, 0, "sd_seat",       TYPE_STRING, print_sd_seat, SD },
		{ true, 0, "sd_sess",       TYPE_STRING, print_sd_sess, SD },
		{ true, 0, "sd_slice",      TYPE_STRING, print_sd_slice, SD },
		{ true, 0, "sd_unit",       TYPE_STRING, print_sd_unit, SD },
		{ true, 0, "sd_uunit",      TYPE_STRING, print_sd_uunit, SD },

		{ true, 0, "lxcname",       TYPE_STRING, print_lxcname, LXC },

		{ true, 0, "environ",       TYPE_STRING, print_environ, ENVIRON },

		{ true, 0, "euid_name",     TYPE_STRING, print_euid_name, DEF | USR_GRP },
		{ true, 0, "egid_name",     TYPE_STRING, print_egid_name, DEF | USR_GRP },

		{ true, 0, "ruid_name",     TYPE_STRING, print_ruid_name, STATUS | USR_GRP },
		{ true, 0, "rgid_name",     TYPE_STRING, print_rgid_name, STATUS | USR_GRP },
		{ true, 0, "suid_name",     TYPE_STRING, print_suid_name, STATUS | USR_GRP },
		{ true, 0, "sgid_name",     TYPE_STRING, print_sgid_name, STATUS | USR_GRP },
		{ true, 0, "fuid_name",     TYPE_STRING, print_fuid_name, STATUS | USR_GRP },
		{ true, 0, "fgid_name",     TYPE_STRING, print_fgid_name, STATUS | USR_GRP },

		{ true, 0, "supgid_names",  TYPE_STRING, print_supgid_names, STATUS | USR_GRP },

		{ true, 0, "time_ms",       TYPE_INT,    print_time_ms, STAT },
		{ true, 0, "time",          TYPE_STRING, print_time, STAT },

		{ true, 0, "start_time",    TYPE_STRING, print_start_time, STAT | START_TIME },
		{ true, 0, "age_sec",       TYPE_INT,    print_age_sec, STAT | START_TIME | AGE },
		{ true, 0, "age_msec",      TYPE_INT,    print_age_msec, STAT | START_TIME | AGE },
		{ true, 0, "age",           TYPE_STRING, print_age, STAT | START_TIME | AGE },
	};

	size_t ncolumns = ARRAY_SIZE(columns);

	while ((opt = getopt_long(argc, argv, "f:p:s", long_options,
			&longindex)) != -1) {
		switch (opt) {
			case 'f':
				cols = xstrdup_nofail(optarg);
				break;
			case 'H':
				print_header = false;
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
				break;
			case 'V':
				printf("git\n");
				return 0;
			case 0:
				switch (longindex) {
					case 0:
					default:
						usage(stderr);
						return 2;
				}
				break;
			case 'h':
			default:
				usage(stdout);
				return 2;
		}
	}

	if (cols) {
		int r = csvci_parse_cols(cols, columns, &ncolumns);

		free(cols);

		if (r)
			exit(2);
	} else {
		for (size_t i = 0; i < ncolumns; ++i)
			columns[i].order = i;
	}

	if (show)
		csv_show();

	if (print_header)
		csvci_print_header(columns, ncolumns);

	uint64_t sources = 0;
	for (size_t i = 0; i < ncolumns; ++i)
		sources |= columns[i].data;

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

		csvci_print_row(&pd, columns, ncolumns);

		freeproc(proc);
	}

	closeproc(pt);

	if (pids)
		free(pids);

	if (sources & USR_GRP)
		usr_grp_query_fini();

	return 0;
}
