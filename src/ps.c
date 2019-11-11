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
 * - cmdline & cgroup -> flatten & add support for csv in csv
 * - --pid
 * - compute dates/times
 * - translate uids & gids
 * - better default list of columns (ps-like or top-like?)
 * - better column names
 * - verify units/mults (pages/mb/kb/b)
 * - figure out what to do about chrome insane cmdlist
 * - tree view?
 * - threads?
 * - figure out what to do with these columns:
 * 	wchan, tty, tpgid, exit_signal, *signals*, alarm, kstk*, flags
 */
#include <assert.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <proc/readproc.h>

#include "usr-grp.h"
#include "utils.h"

struct visible_columns {
	/* always available */
	char tid;
	char tgid;
	char euid;
	char egid;

	/* from stat or status */
	char ppid;
	char state;
	char cmd;
	char nlwp;
	char wchan;

	/* from stat */
	char pgrp;
	char session;

	char utime;
	char stime;
	char cutime;
	char cstime;
	char start_time;

	char start_code;
	char end_code;
	char start_stack;
	char kstk_esp;
	char kstk_eip;

	char priority;
	char nice;
	char rss;
	char alarm;

	char rtprio;
	char sched;
	char vsize;
	char rss_rlim;
	char flags;
	char min_flt;
	char maj_flt;
	char cmin_flt;
	char cmaj_flt;

	char tty;
	char tpgid;
	char exit_signal;
	char processor;

	/* from status */
	char pending_signals;
	char blocked_signals;
	char ignored_signals;
	char caught_signals;
	char pending_signals_per_task;

	char vm_size;
	char vm_lock;
	char vm_rss;
	char vm_rss_anon;
	char vm_rss_file;
	char vm_rss_shared;
	char vm_data;
	char vm_stack;
	char vm_swap;
	char vm_exe;
	char vm_lib;

	char ruid;
	char rgid;
	char suid;
	char sgid;
	char fuid;
	char fgid;

	char supgid;

	/* from statm */
	char size;
	char resident;
	char share;
	char trs;
	char drs;

	/* other */
	char ncmdline;
	char cmdline[100];

	char ncgroup;
	char cgroup[100];

	char oom_score;
	char oom_adj;

	char ns_ipc;
	char ns_mnt;
	char ns_net;
	char ns_pid;
	char ns_user;
	char ns_uts;

	char sd_mach;
        char sd_ouid;
        char sd_seat;
        char sd_sess;
        char sd_slice;
        char sd_unit;
        char sd_uunit;

        char lxcname;

	char environ;
};

struct visibility_info {
	struct visible_columns cols;
	size_t count;
};

static const struct option long_options[] = {
	{"fields",	required_argument,	NULL, 'f'},
	{"no-header",	no_argument,		NULL, 'H'},
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
	fprintf(out, "  -s, --show\n");
	fprintf(out, "      --no-header\n");
	fprintf(out, "      --help\n");
	fprintf(out, "      --version\n");
}

struct print_ctx {
	size_t printed;
	const struct visibility_info *visinfo;
};

static void
cprint(struct print_ctx *ctx, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);

	if (++ctx->printed < ctx->visinfo->count)
		fputc(',', stdout);
	else
		fputc('\n', stdout);
}

struct eval_col_data {
	int print;
	size_t visible_count;
	size_t count;
	int mask;
};

static void
eval_col(char vis, const char *str, struct eval_col_data *d, int source)
{
	if (!vis)
		return;
	(d->visible_count)++;
	if (!d->print)
		return;

	fputs(str, stdout);
	d->mask |= source;

	if (d->visible_count < d->count)
		fputc(',', stdout);
}

static size_t
get_array_size(char **arr, size_t *size)
{
	if (*size != SIZE_MAX)
		return *size;

	*size = 0;

	if (!arr)
		return *size;

	while (*arr) {
		arr++;
		(*size)++;
	}

	return *size;
}

int
main(int argc, char *argv[])
{
	int opt;
	int longindex;
	char *cols = NULL;
	struct visible_columns vis;
	bool print_header = true;
	bool show = false;

	PageSize = sysconf(_SC_PAGESIZE);

	memset(&vis, 1, sizeof(vis));

	memset(&vis.cmdline, 0, sizeof(vis.cmdline));
	memset(&vis.cmdline, 1, 25);

	memset(&vis.cgroup, 0, sizeof(vis.cmdline));
	memset(&vis.cgroup, 1, 20);

	while ((opt = getopt_long(argc, argv, "f:s", long_options,
			&longindex)) != -1) {
		switch (opt) {
			case 'f':
				cols = xstrdup_nofail(optarg);
				break;
			case 'H':
				print_header = false;
				break;
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
		memset(&vis, 0, sizeof(vis));

		const struct {
			const char *name;
			char *vis;
		} map[] = {
				/* always available */
				{ "tid", &vis.tid },
				{ "tgid", &vis.tgid },
				{ "euid", &vis.euid },
				{ "egid", &vis.egid },

				/* from stat or status */
				{ "ppid", &vis.ppid },
				{ "state", &vis.state },
				{ "cmd", &vis.cmd },
				{ "nlwp", &vis.nlwp },
				{ "wchan", &vis.wchan },

				/* from stat */
				{ "pgrp", &vis.pgrp },
				{ "session", &vis.session },

				{ "utime", &vis.utime },
				{ "stime", &vis.stime },
				{ "cutime", &vis.cutime },
				{ "cstime", &vis.cstime },
				{ "start_time", &vis.start_time },

				{ "start_code", &vis.start_code },
				{ "end_code", &vis.end_code },
				{ "start_stack", &vis.start_stack },
				{ "kstk_esp", &vis.kstk_esp },
				{ "kstk_eip", &vis.kstk_eip },

				{ "priority", &vis.priority },
				{ "nice", &vis.nice },
				{ "rss", &vis.rss },
				{ "alarm", &vis.alarm },

				{ "rtprio", &vis.rtprio },
				{ "sched", &vis.sched },
				{ "vsize", &vis.vsize },
				{ "rss_rlim", &vis.rss_rlim },
				{ "flags", &vis.flags },
				{ "min_flt", &vis.min_flt },
				{ "maj_flt", &vis.maj_flt },
				{ "cmin_flt", &vis.cmin_flt },
				{ "cmaj_flt", &vis.cmaj_flt },

				{ "tty", &vis.tty },
				{ "tpgid", &vis.tpgid },
				{ "exit_signal", &vis.exit_signal },
				{ "processor", &vis.processor },

				/* from status */
				{ "pending_signals", &vis.pending_signals },
				{ "blocked_signals", &vis.blocked_signals },
				{ "ignored_signals", &vis.ignored_signals },
				{ "caught_signals", &vis.caught_signals },
				{ "pending_signals_per_task", &vis.pending_signals_per_task },

				{ "vm_size", &vis.vm_size },
				{ "vm_lock", &vis.vm_lock },
				{ "vm_rss", &vis.vm_rss },
				{ "vm_rss_anon", &vis.vm_rss_anon },
				{ "vm_rss_file", &vis.vm_rss_file },
				{ "vm_rss_shared", &vis.vm_rss_shared },
				{ "vm_data", &vis.vm_data },
				{ "vm_stack", &vis.vm_stack },
				{ "vm_swap", &vis.vm_swap },
				{ "vm_exe", &vis.vm_exe },
				{ "vm_lib", &vis.vm_lib },

				{ "ruid", &vis.ruid },
				{ "rgid", &vis.rgid },
				{ "suid", &vis.suid },
				{ "sgid", &vis.sgid },
				{ "fuid", &vis.fuid },
				{ "fgid", &vis.fgid },

				{ "supgid", &vis.supgid },

				/* from statm */
				{ "size", &vis.size },
				{ "resident", &vis.resident },
				{ "share", &vis.share },
				{ "trs", &vis.trs },
				{ "drs", &vis.drs },

				/* other */
				{ "oom_score", &vis.oom_score },
				{ "oom_adj", &vis.oom_adj },

				{ "ncmdline", &vis.ncmdline },
				{ "ncgroup", &vis.ncgroup },

				{ "ns_ipc", &vis.ns_ipc },
				{ "ns_mnt", &vis.ns_mnt },
				{ "ns_net", &vis.ns_net },
				{ "ns_pid", &vis.ns_pid },
				{ "ns_user", &vis.ns_user },
				{ "ns_uts", &vis.ns_uts },

				{ "sd_mach", &vis.sd_mach },
			        { "sd_ouid", &vis.sd_ouid },
			        { "sd_seat", &vis.sd_seat },
			        { "sd_sess", &vis.sd_sess },
			        { "sd_slice", &vis.sd_slice },
			        { "sd_unit", &vis.sd_unit },
			        { "sd_uunit", &vis.sd_uunit },

				{ "lxcname", &vis.lxcname },

				{ "environ", &vis.environ },
		};

		char *name = strtok(cols, ",");
		while (name) {
			int found = 0;
			for (size_t i = 0; i < sizeof(map) / sizeof(map[0]); ++i) {
				if (strcmp(name, map[i].name) == 0) {
					*map[i].vis = 1;
					found = 1;
					break;
				}
			}

			if (!found && strncmp(name, "cmdline", strlen("cmdline")) == 0) {
				int num = atoi(name + strlen("cmdline"));
				if (num >= sizeof(vis.cmdline)) {
					fprintf(stderr, "cmdline index above max\n");
					exit(2);
				}
				vis.cmdline[num] = 1;
				found = 1;
			}

			if (!found && strncmp(name, "cgroup", strlen("cgroup")) == 0) {
				int num = atoi(name + strlen("cgroup"));
				if (num >= sizeof(vis.cgroup)) {
					fprintf(stderr, "cgroup index above max\n");
					exit(2);
				}
				vis.cgroup[num] = 1;
				found = 1;
			}

			if (!found) {
				fprintf(stderr, "column %s not found\n", name);
				exit(2);
			}

			name = strtok(NULL, ",");
		}

		free(cols);
	}

	if (show)
		csv_show();

	enum { DEF = 0, STAT = 1, STATUS = 2, STAT_OR_STATUS = 4, STATM = 8,
		CMDLINE = 16, ENVIRON = 32, CGROUP = 64, OOM = 128, NS = 256,
		SD = 512, LXC = 1024 };

	struct eval_col_data d;
	d.print = 0;
	d.visible_count = 0;
	d.count = sizeof(vis) + 1;
	d.mask = 0;

	do {
		/* always available */
		eval_col(vis.tid, "tid:int", &d, DEF);
		eval_col(vis.tgid, "tgid:int", &d, DEF);
		eval_col(vis.euid, "euid:int", &d, DEF);
		eval_col(vis.egid, "egid:int", &d, DEF);

		/* from stat or status */
		eval_col(vis.ppid, "ppid:int", &d, STAT_OR_STATUS);
		eval_col(vis.state, "state:string", &d, STAT_OR_STATUS);
		eval_col(vis.cmd, "cmd:string", &d, STAT_OR_STATUS);
		eval_col(vis.nlwp, "nlwp:int", &d, STAT_OR_STATUS);
		eval_col(vis.wchan, "wchan:int", &d, STAT_OR_STATUS);

		/* from stat */
		eval_col(vis.pgrp, "pgrp:int", &d, STAT);
		eval_col(vis.session, "session:int", &d, STAT);

		eval_col(vis.utime, "utime:int", &d, STAT);
		eval_col(vis.stime, "stime:int", &d, STAT);
		eval_col(vis.cutime, "cutime:int", &d, STAT);
		eval_col(vis.cstime, "cstime:int", &d, STAT);
		eval_col(vis.start_time, "start_time:int", &d, STAT);

		eval_col(vis.start_code, "start_code:int", &d, STAT);
		eval_col(vis.end_code, "end_code:int", &d, STAT);
		eval_col(vis.start_stack, "start_stack:int", &d, STAT);
		eval_col(vis.kstk_esp, "kstk_esp:int", &d, STAT);
		eval_col(vis.kstk_eip, "kstk_eip:int", &d, STAT);

		eval_col(vis.priority, "priority:int", &d, STAT);
		eval_col(vis.nice, "nice:int", &d, STAT);
		eval_col(vis.rss, "rss:int", &d, STAT);
		eval_col(vis.alarm, "alarm:int", &d, STAT);

		eval_col(vis.rtprio, "rtprio:int", &d, STAT);
		eval_col(vis.sched, "sched:int", &d, STAT);
		eval_col(vis.vsize, "vsize:int", &d, STAT);
		eval_col(vis.rss_rlim, "rss_rlim:int", &d, STAT);
		eval_col(vis.flags, "flags:int", &d, STAT);
		eval_col(vis.min_flt, "min_flt:int", &d, STAT);
		eval_col(vis.maj_flt, "maj_flt:int", &d, STAT);
		eval_col(vis.cmin_flt, "cmin_flt:int", &d, STAT);
		eval_col(vis.cmaj_flt, "cmaj_flt:int", &d, STAT);

		eval_col(vis.tty, "tty:int", &d, STAT);
		eval_col(vis.tpgid, "tpgid:int", &d, STAT);
		eval_col(vis.exit_signal, "exit_signal:int", &d, STAT);
		eval_col(vis.processor, "processor:int", &d, STAT);

		/* from status */
		eval_col(vis.pending_signals, "pending_signals:int", &d, STATUS);
		eval_col(vis.blocked_signals, "blocked_signals:int", &d, STATUS);
		eval_col(vis.ignored_signals, "ignored_signals:int", &d, STATUS);
		eval_col(vis.caught_signals, "caught_signals:int", &d, STATUS);
		eval_col(vis.pending_signals_per_task, "pending_signals_per_task:int", &d, STATUS);

		eval_col(vis.vm_size, "vm_size:int", &d, STATUS);
		eval_col(vis.vm_lock, "vm_lock:int", &d, STATUS);
		eval_col(vis.vm_rss, "vm_rss:int", &d, STATUS);
		eval_col(vis.vm_rss_anon, "vm_rss_anon:int", &d, STATUS);
		eval_col(vis.vm_rss_file, "vm_rss_file:int", &d, STATUS);
		eval_col(vis.vm_rss_shared, "vm_rss_shared:int", &d, STATUS);
		eval_col(vis.vm_data, "vm_data:int", &d, STATUS);
		eval_col(vis.vm_stack, "vm_stack:int", &d, STATUS);
		eval_col(vis.vm_swap, "vm_swap:int", &d, STATUS);
		eval_col(vis.vm_exe, "vm_exe:int", &d, STATUS);
		eval_col(vis.vm_lib, "vm_lib:int", &d, STATUS);

		eval_col(vis.ruid, "ruid:int", &d, STATUS);
		eval_col(vis.rgid, "rgid:int", &d, STATUS);
		eval_col(vis.suid, "suid:int", &d, STATUS);
		eval_col(vis.sgid, "sgid:int", &d, STATUS);
		eval_col(vis.fuid, "fuid:int", &d, STATUS);
		eval_col(vis.fgid, "fgid:int", &d, STATUS);

		eval_col(vis.supgid, "supgid:string", &d, STATUS);

		/* from tatm */
		eval_col(vis.size, "size:int", &d, STATM);
		eval_col(vis.resident, "resident:int", &d, STATM);
		eval_col(vis.share, "share:int", &d, STATM);
		eval_col(vis.trs, "trs:int", &d, STATM);
		eval_col(vis.drs, "drs:int", &d, STATM);

		/* other */
		eval_col(vis.ncmdline, "ncmdline:int", &d, CMDLINE);
		for (int i = 0; i < sizeof(vis.cmdline); ++i) {
			assert(sizeof(vis.cmdline) - 1 == 99);
			char str[strlen("cmdline") + strlen("99") + strlen(":string") + 1];
			if (vis.cmdline[i] && d.print)
				sprintf(str, "cmdline%d:string", i);
			else
				str[0] = 0; /* doesn't matter*/

			eval_col(vis.cmdline[i], str, &d, CMDLINE);
		}

		eval_col(vis.ncgroup, "ncgroup:int", &d, CGROUP);
		for (int i = 0; i < sizeof(vis.cgroup); ++i) {
			assert(sizeof(vis.cgroup) - 1 == 99);
			char str[strlen("cgroup") + strlen("99") + strlen(":string") + 1];
			if (vis.cgroup[i] && d.print)
				sprintf(str, "cgroup%d:string", i);
			else
				str[0] = 0; /* doesn't matter*/

			eval_col(vis.cgroup[i], str, &d, CGROUP);
		}

		eval_col(vis.oom_score, "oom_score:int", &d, OOM);
		eval_col(vis.oom_adj, "oom_adj:int", &d, OOM);

		eval_col(vis.ns_ipc, "ns_ipc:int", &d, NS);
		eval_col(vis.ns_mnt, "ns_mnt:int", &d, NS);
		eval_col(vis.ns_net, "ns_net:int", &d, NS);
		eval_col(vis.ns_pid, "ns_pid:int", &d, NS);
		eval_col(vis.ns_user, "ns_user:int", &d, NS);
		eval_col(vis.ns_uts, "ns_uts:int", &d, NS);

		eval_col(vis.sd_mach, "sd_mach:string", &d, SD);
	        eval_col(vis.sd_ouid, "sd_ouid:string", &d, SD);
	        eval_col(vis.sd_seat, "sd_seat:string", &d, SD);
	        eval_col(vis.sd_sess, "sd_sess:string", &d, SD);
	        eval_col(vis.sd_slice, "sd_slice:string", &d, SD);
	        eval_col(vis.sd_unit, "sd_unit:string", &d, SD);
	        eval_col(vis.sd_uunit, "sd_uunit:string", &d, SD);

	        eval_col(vis.lxcname, "lxcname:string", &d, LXC);

		eval_col(vis.environ, "environ:string", &d, ENVIRON);

		d.count = d.visible_count;
		d.visible_count = 0;
		if (print_header)
			d.print++;
	} while (d.print == 1);

	if (print_header)
		printf("\n");

	struct visibility_info visinfo = {vis, d.count};

#if 0
	load_groups();
	load_users();
#endif

	int flags = 0;

	if (d.mask & STAT)
		flags |= PROC_FILLSTAT;
	if (d.mask & STATUS)
		flags |= PROC_FILLSTATUS;
	if (d.mask & STAT_OR_STATUS) {
		if ((flags & (PROC_FILLSTAT | PROC_FILLSTATUS)) == 0)
			/* stat seems easier to parse */
			flags |= PROC_FILLSTAT;
	}
	if (d.mask & STATM)
		flags |= PROC_FILLMEM;
	if (d.mask & CMDLINE)
		flags |= PROC_FILLCOM;
	if (d.mask & ENVIRON)
		flags |= PROC_FILLENV | PROC_EDITENVRCVT;
	if (d.mask & CGROUP)
		flags |= PROC_FILLCGROUP;
	if (d.mask & OOM)
		flags |= PROC_FILLOOM;
	if (d.mask & NS)
		flags |= PROC_FILLNS;
	if (d.mask & SD)
		flags |= PROC_FILLSYSTEMD;
	if (d.mask & LXC)
		flags |= PROC_FILL_LXC;

	pid_t l[] = {3209, 0};
	PROCTAB *pt = openproc(flags /*| PROC_PID*/, l);
	if (!pt) {
		perror("openproc");
		exit(2);
	}

	proc_t *proc;

	while ((proc = readproc(pt, NULL)) != NULL) {
		struct print_ctx ctx = {0, &visinfo};

		/* always */
		if (vis.tid)
			cprint(&ctx, "%d", proc->tid);
		if (vis.tgid)
			cprint(&ctx, "%d", proc->tgid);
		if (vis.euid)
			cprint(&ctx, "%d", proc->euid);
		if (vis.egid)
			cprint(&ctx, "%d", proc->egid);

		/* stat, status */
		if (vis.ppid)
			cprint(&ctx, "%d", proc->ppid);
		if (vis.state)
			cprint(&ctx, "%c", proc->state);
		if (vis.cmd) {
			csv_print_quoted(proc->cmd, strlen(proc->cmd));
			cprint(&ctx, "");
		}
		if (vis.nlwp)
			cprint(&ctx, "%d", proc->nlwp);
		if (vis.wchan)
			cprint(&ctx, "0x%lx", proc->wchan);

		/* stat */
		if (vis.pgrp)
			cprint(&ctx, "%d", proc->pgrp);
		if (vis.session)
			cprint(&ctx, "%d", proc->session);

		if (vis.utime)
			cprint(&ctx, "%llu", proc->utime);
		if (vis.stime)
			cprint(&ctx, "%llu", proc->stime);
		if (vis.cutime)
			cprint(&ctx, "%llu", proc->cutime);
		if (vis.cstime)
			cprint(&ctx, "%llu", proc->cstime);
		if (vis.start_time)
			cprint(&ctx, "%llu", proc->start_time);

		if (vis.start_code)
			cprint(&ctx, "0x%lx", proc->start_code);
		if (vis.end_code)
			cprint(&ctx, "0x%lx", proc->end_code);
		if (vis.start_stack)
			cprint(&ctx, "0x%lx", proc->start_stack);
		if (vis.kstk_esp)
			cprint(&ctx, "0x%lx", proc->kstk_esp);
		if (vis.kstk_eip)
			cprint(&ctx, "0x%lx", proc->kstk_eip);

		if (vis.priority)
			cprint(&ctx, "%ld", proc->priority);
		if (vis.nice)
			cprint(&ctx, "%ld", proc->nice);
		if (vis.rss)
			cprint(&ctx, "%ld", proc->rss * PageSize);
		if (vis.alarm)
			cprint(&ctx, "%ld", proc->alarm);

		if (vis.rtprio)
			cprint(&ctx, "%lu", proc->rtprio);
		if (vis.sched)
			cprint(&ctx, "%lu", proc->sched);
		if (vis.vsize)
			cprint(&ctx, "%lu", proc->vsize);
		if (vis.rss_rlim)
			cprint(&ctx, "0x%lx", proc->rss_rlim);
		if (vis.flags)
			cprint(&ctx, "0x%lx", proc->flags);
		if (vis.min_flt)
			cprint(&ctx, "%lu", proc->min_flt);
		if (vis.maj_flt)
			cprint(&ctx, "%lu", proc->maj_flt);
		if (vis.cmin_flt)
			cprint(&ctx, "%lu", proc->cmin_flt);
		if (vis.cmaj_flt)
			cprint(&ctx, "%lu", proc->cmaj_flt);

		if (vis.tty)
			cprint(&ctx, "%d", proc->tty);
		if (vis.tpgid)
			cprint(&ctx, "%d", proc->tpgid);
		if (vis.exit_signal)
			cprint(&ctx, "%d", proc->exit_signal);
		if (vis.processor)
			cprint(&ctx, "%d", proc->processor);

		/* status */
		if (vis.pending_signals) {
			csv_print_quoted(proc->signal, strlen(proc->signal));
			cprint(&ctx, "");
		}
		if (vis.blocked_signals) {
			csv_print_quoted(proc->blocked, strlen(proc->blocked));
			cprint(&ctx, "");
		}
		if (vis.ignored_signals) {
			csv_print_quoted(proc->sigignore, strlen(proc->sigignore));
			cprint(&ctx, "");
		}
		if (vis.caught_signals) {
			csv_print_quoted(proc->sigcatch, strlen(proc->sigcatch));
			cprint(&ctx, "");
		}
		if (vis.pending_signals_per_task) {
			csv_print_quoted(proc->_sigpnd, strlen(proc->_sigpnd));
			cprint(&ctx, "");
		}

		if (vis.vm_size)
			cprint(&ctx, "%lu", proc->vm_size * 1024);
		if (vis.vm_lock)
			cprint(&ctx, "%lu", proc->vm_lock * 1024);
		if (vis.vm_rss)
			cprint(&ctx, "%lu", proc->vm_rss * 1024);
		if (vis.vm_rss_anon)
			cprint(&ctx, "%lu", proc->vm_rss_anon * 1024);
		if (vis.vm_rss_file)
			cprint(&ctx, "%lu", proc->vm_rss_file * 1024);
		if (vis.vm_rss_shared)
			cprint(&ctx, "%lu", proc->vm_rss_shared * 1024);
		if (vis.vm_data)
			cprint(&ctx, "%lu", proc->vm_data * 1024);
		if (vis.vm_stack)
			cprint(&ctx, "%lu", proc->vm_stack * 1024);
		if (vis.vm_swap)
			cprint(&ctx, "%lu", proc->vm_swap * 1024);
		if (vis.vm_exe)
			cprint(&ctx, "%lu", proc->vm_exe * 1024);
		if (vis.vm_lib)
			cprint(&ctx, "%lu", proc->vm_lib * 1024);

		if (vis.ruid)
			cprint(&ctx, "%d", proc->ruid);
		if (vis.rgid)
			cprint(&ctx, "%d", proc->rgid);
		if (vis.suid)
			cprint(&ctx, "%d", proc->suid);
		if (vis.sgid)
			cprint(&ctx, "%d", proc->sgid);
		if (vis.fuid)
			cprint(&ctx, "%d", proc->fuid);
		if (vis.fgid)
			cprint(&ctx, "%d", proc->fgid);

		if (vis.supgid) {
			csv_print_quoted(proc->supgid, strlen(proc->supgid));
			cprint(&ctx, "");
		}

		/* statm */
		if (vis.size)
			cprint(&ctx, "%ld", proc->size * PageSize);
		if (vis.resident)
			cprint(&ctx, "%ld", proc->resident * PageSize);
		if (vis.share)
			cprint(&ctx, "%ld", proc->share * PageSize);
		if (vis.trs)
			cprint(&ctx, "%ld", proc->trs * PageSize);
		if (vis.drs)
			cprint(&ctx, "%ld", proc->drs * PageSize);

		/* other */
		size_t ncmdline = SIZE_MAX;
		if (vis.ncmdline)
			cprint(&ctx, "%lu", get_array_size(proc->cmdline, &ncmdline));

		for (int i = 0; i < sizeof(vis.cmdline); ++i) {
			if (vis.cmdline[i]) {
				get_array_size(proc->cmdline, &ncmdline);
				if (i < ncmdline)
					csv_print_quoted(proc->cmdline[i],
							strlen(proc->cmdline[i]));
				cprint(&ctx, "");
			}
		}

		size_t ncgroup = SIZE_MAX;
		if (vis.ncgroup)
			cprint(&ctx, "%lu", get_array_size(proc->cgroup, &ncgroup));

		for (int i = 0; i < sizeof(vis.cgroup); ++i) {
			if (vis.cgroup[i]) {
				get_array_size(proc->cgroup, &ncgroup);
				if (i < ncgroup)
					csv_print_quoted(proc->cgroup[i],
							strlen(proc->cgroup[i]));
				cprint(&ctx, "");
			}
		}

		if (vis.oom_score)
			cprint(&ctx, "%d", proc->oom_score);
		if (vis.oom_adj)
			cprint(&ctx, "%d", proc->oom_adj);

	        if (vis.ns_ipc)
			cprint(&ctx, "0x%lx", proc->ns[IPCNS]);
	        if (vis.ns_mnt)
			cprint(&ctx, "0x%lx", proc->ns[MNTNS]);
	        if (vis.ns_net)
			cprint(&ctx, "0x%lx", proc->ns[NETNS]);
	        if (vis.ns_pid)
			cprint(&ctx, "0x%lx", proc->ns[PIDNS]);
	        if (vis.ns_user)
			cprint(&ctx, "0x%lx", proc->ns[USERNS]);
	        if (vis.ns_uts)
			cprint(&ctx, "0x%lx", proc->ns[UTSNS]);

	        if (vis.sd_mach) {
			csv_print_quoted(proc->sd_mach, strlen(proc->sd_mach));
			cprint(&ctx, "");
	        }

	        if (vis.sd_ouid) {
			csv_print_quoted(proc->sd_ouid, strlen(proc->sd_ouid));
			cprint(&ctx, "");
	        }

	        if (vis.sd_seat) {
			csv_print_quoted(proc->sd_seat, strlen(proc->sd_seat));
			cprint(&ctx, "");
	        }

	        if (vis.sd_sess) {
			csv_print_quoted(proc->sd_sess, strlen(proc->sd_sess));
			cprint(&ctx, "");
	        }

	        if (vis.sd_slice) {
			csv_print_quoted(proc->sd_slice, strlen(proc->sd_slice));
			cprint(&ctx, "");
	        }

	        if (vis.sd_unit) {
			csv_print_quoted(proc->sd_unit, strlen(proc->sd_unit));
			cprint(&ctx, "");
	        }

	        if (vis.sd_uunit) {
			csv_print_quoted(proc->sd_uunit, strlen(proc->sd_uunit));
			cprint(&ctx, "");
	        }

	        if (vis.lxcname) {
			csv_print_quoted(proc->lxcname, strlen(proc->lxcname));
			cprint(&ctx, "");
	        }

	        if (vis.environ) {
			if (proc->environ)
				csv_print_quoted(proc->environ[0], strlen(proc->environ[0]));
			cprint(&ctx, "");
		}
	}

	closeproc(pt);

	free_users();
	free_groups();

	return 0;
}
