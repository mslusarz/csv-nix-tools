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
 * - compute dates/times
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
#include <errno.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <proc/readproc.h>

#include "usr-grp-query.h"
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

	char user_time_ms;
	char system_time_ms;
	char cumulative_user_time_ms;
	char cumulative_system_time_ms;
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
	char cmdline;
	char cgroup;

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

	char euid_name;
	char egid_name;

	char ruid_name;
	char rgid_name;
	char suid_name;
	char sgid_name;
	char fuid_name;
	char fgid_name;

	char supgid_names;
};

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

static void
compute_visibility(char *cols, struct visible_columns *vis)
{
	memset(vis, 0, sizeof(*vis));

	const struct {
		const char *name;
		char *vis;
	} map[] = {
		/* always available */
		{ "tid", &vis->tid },
		{ "tgid", &vis->tgid },
		{ "euid", &vis->euid },
		{ "egid", &vis->egid },

		/* from stat or status */
		{ "ppid", &vis->ppid },
		{ "state", &vis->state },
		{ "cmd", &vis->cmd },
		{ "nlwp", &vis->nlwp },
		{ "wchan", &vis->wchan },

		/* from stat */
		{ "pgrp", &vis->pgrp },
		{ "session", &vis->session },

		{ "user_time_ms", &vis->user_time_ms },
		{ "system_time_ms", &vis->system_time_ms },
		{ "system_time_ms", &vis->cumulative_user_time_ms },
		{ "cumulative_system_time_ms", &vis->cumulative_system_time_ms },
		{ "start_time", &vis->start_time },

		{ "start_code", &vis->start_code },
		{ "end_code", &vis->end_code },
		{ "start_stack", &vis->start_stack },
		{ "kstk_esp", &vis->kstk_esp },
		{ "kstk_eip", &vis->kstk_eip },

		{ "priority", &vis->priority },
		{ "nice", &vis->nice },
		{ "rss", &vis->rss },
		{ "alarm", &vis->alarm },

		{ "rtprio", &vis->rtprio },
		{ "sched", &vis->sched },
		{ "vsize", &vis->vsize },
		{ "rss_rlim", &vis->rss_rlim },
		{ "flags", &vis->flags },
		{ "min_flt", &vis->min_flt },
		{ "maj_flt", &vis->maj_flt },
		{ "cmin_flt", &vis->cmin_flt },
		{ "cmaj_flt", &vis->cmaj_flt },

		{ "tty", &vis->tty },
		{ "tpgid", &vis->tpgid },
		{ "exit_signal", &vis->exit_signal },
		{ "processor", &vis->processor },

		/* from status */
		{ "pending_signals", &vis->pending_signals },
		{ "blocked_signals", &vis->blocked_signals },
		{ "ignored_signals", &vis->ignored_signals },
		{ "caught_signals", &vis->caught_signals },
		{ "pending_signals_per_task", &vis->pending_signals_per_task },

		{ "vm_size", &vis->vm_size },
		{ "vm_lock", &vis->vm_lock },
		{ "vm_rss", &vis->vm_rss },
		{ "vm_rss_anon", &vis->vm_rss_anon },
		{ "vm_rss_file", &vis->vm_rss_file },
		{ "vm_rss_shared", &vis->vm_rss_shared },
		{ "vm_data", &vis->vm_data },
		{ "vm_stack", &vis->vm_stack },
		{ "vm_swap", &vis->vm_swap },
		{ "vm_exe", &vis->vm_exe },
		{ "vm_lib", &vis->vm_lib },

		{ "ruid", &vis->ruid },
		{ "rgid", &vis->rgid },
		{ "suid", &vis->suid },
		{ "sgid", &vis->sgid },
		{ "fuid", &vis->fuid },
		{ "fgid", &vis->fgid },

		{ "supgid", &vis->supgid },

		/* from statm */
		{ "size", &vis->size },
		{ "resident", &vis->resident },
		{ "share", &vis->share },
		{ "trs", &vis->trs },
		{ "drs", &vis->drs },

		/* other */
		{ "cmdline", &vis->cmdline },
		{ "cgroup", &vis->cgroup },

		{ "oom_score", &vis->oom_score },
		{ "oom_adj", &vis->oom_adj },

		{ "ns_ipc", &vis->ns_ipc },
		{ "ns_mnt", &vis->ns_mnt },
		{ "ns_net", &vis->ns_net },
		{ "ns_pid", &vis->ns_pid },
		{ "ns_user", &vis->ns_user },
		{ "ns_uts", &vis->ns_uts },

		{ "sd_mach", &vis->sd_mach },
		{ "sd_ouid", &vis->sd_ouid },
		{ "sd_seat", &vis->sd_seat },
		{ "sd_sess", &vis->sd_sess },
		{ "sd_slice", &vis->sd_slice },
		{ "sd_unit", &vis->sd_unit },
		{ "sd_uunit", &vis->sd_uunit },

		{ "lxcname", &vis->lxcname },

		{ "environ", &vis->environ },

		/* always available */
		{ "euid_name", &vis->euid_name },
		{ "egid_name", &vis->egid_name },

		/* from status */
		{ "ruid_name", &vis->ruid_name },
		{ "rgid_name", &vis->rgid_name },
		{ "suid_name", &vis->suid_name },
		{ "sgid_name", &vis->sgid_name },
		{ "fuid_name", &vis->fuid_name },
		{ "fgid_name", &vis->fgid_name },

		{ "supgid_names", &vis->supgid_names },
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

		if (!found) {
			fprintf(stderr, "column %s not found\n", name);
			exit(2);
		}

		name = strtok(NULL, ",");
	}
}

struct eval_col_data {
	int print;
	size_t visible_count;
	size_t count;
	int sources;
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
	d->sources |= source;

	if (d->visible_count < d->count)
		fputc(',', stdout);
}

enum { DEF = 0, STAT = 1, STATUS = 2, STAT_OR_STATUS = 4, STATM = 8,
	CMDLINE = 16, ENVIRON = 32, CGROUP = 64, OOM = 128, NS = 256,
	SD = 512, LXC = 1024 };

struct col_summary {
	size_t count;
	int sources;
};

static void
eval_visibility(const struct visible_columns *vis, bool print_header,
		struct col_summary *summary)
{
	struct eval_col_data d;
	d.print = 0;
	d.visible_count = 0;
	d.count = sizeof(*vis) + 1;
	d.sources = 0;

	do {
		/* always available */
		eval_col(vis->tid, "tid:int", &d, DEF);
		eval_col(vis->tgid, "tgid:int", &d, DEF);
		eval_col(vis->euid, "euid:int", &d, DEF);
		eval_col(vis->egid, "egid:int", &d, DEF);

		/* from stat or status */
		eval_col(vis->ppid, "ppid:int", &d, STAT_OR_STATUS);
		eval_col(vis->state, "state:string", &d, STAT_OR_STATUS);
		eval_col(vis->cmd, "cmd:string", &d, STAT_OR_STATUS);
		eval_col(vis->nlwp, "nlwp:int", &d, STAT_OR_STATUS);
		eval_col(vis->wchan, "wchan:int", &d, STAT_OR_STATUS);

		/* from stat */
		eval_col(vis->pgrp, "pgrp:int", &d, STAT);
		eval_col(vis->session, "session:int", &d, STAT);

		eval_col(vis->user_time_ms, "user_time_ms:int", &d, STAT);
		eval_col(vis->system_time_ms, "system_time_ms:int", &d, STAT);
		eval_col(vis->cumulative_user_time_ms, "cumulative_user_time_ms:int", &d, STAT);
		eval_col(vis->cumulative_system_time_ms, "cumulative_system_time_ms:int", &d, STAT);
		eval_col(vis->start_time, "start_time:int", &d, STAT);

		eval_col(vis->start_code, "start_code:int", &d, STAT);
		eval_col(vis->end_code, "end_code:int", &d, STAT);
		eval_col(vis->start_stack, "start_stack:int", &d, STAT);
		eval_col(vis->kstk_esp, "kstk_esp:int", &d, STAT);
		eval_col(vis->kstk_eip, "kstk_eip:int", &d, STAT);

		eval_col(vis->priority, "priority:int", &d, STAT);
		eval_col(vis->nice, "nice:int", &d, STAT);
		eval_col(vis->rss, "rss:int", &d, STAT);
		eval_col(vis->alarm, "alarm:int", &d, STAT);

		eval_col(vis->rtprio, "rtprio:int", &d, STAT);
		eval_col(vis->sched, "sched:int", &d, STAT);
		eval_col(vis->vsize, "vsize:int", &d, STAT);
		eval_col(vis->rss_rlim, "rss_rlim:int", &d, STAT);
		eval_col(vis->flags, "flags:int", &d, STAT);
		eval_col(vis->min_flt, "min_flt:int", &d, STAT);
		eval_col(vis->maj_flt, "maj_flt:int", &d, STAT);
		eval_col(vis->cmin_flt, "cmin_flt:int", &d, STAT);
		eval_col(vis->cmaj_flt, "cmaj_flt:int", &d, STAT);

		eval_col(vis->tty, "tty:int", &d, STAT);
		eval_col(vis->tpgid, "tpgid:int", &d, STAT);
		eval_col(vis->exit_signal, "exit_signal:int", &d, STAT);
		eval_col(vis->processor, "processor:int", &d, STAT);

		/* from status */
		eval_col(vis->pending_signals, "pending_signals:string", &d, STATUS);
		eval_col(vis->blocked_signals, "blocked_signals:string", &d, STATUS);
		eval_col(vis->ignored_signals, "ignored_signals:string", &d, STATUS);
		eval_col(vis->caught_signals, "caught_signals:string", &d, STATUS);
		eval_col(vis->pending_signals_per_task, "pending_signals_per_task:string", &d, STATUS);

		eval_col(vis->vm_size, "vm_size:int", &d, STATUS);
		eval_col(vis->vm_lock, "vm_lock:int", &d, STATUS);
		eval_col(vis->vm_rss, "vm_rss:int", &d, STATUS);
		eval_col(vis->vm_rss_anon, "vm_rss_anon:int", &d, STATUS);
		eval_col(vis->vm_rss_file, "vm_rss_file:int", &d, STATUS);
		eval_col(vis->vm_rss_shared, "vm_rss_shared:int", &d, STATUS);
		eval_col(vis->vm_data, "vm_data:int", &d, STATUS);
		eval_col(vis->vm_stack, "vm_stack:int", &d, STATUS);
		eval_col(vis->vm_swap, "vm_swap:int", &d, STATUS);
		eval_col(vis->vm_exe, "vm_exe:int", &d, STATUS);
		eval_col(vis->vm_lib, "vm_lib:int", &d, STATUS);

		eval_col(vis->ruid, "ruid:int", &d, STATUS);
		eval_col(vis->rgid, "rgid:int", &d, STATUS);
		eval_col(vis->suid, "suid:int", &d, STATUS);
		eval_col(vis->sgid, "sgid:int", &d, STATUS);
		eval_col(vis->fuid, "fuid:int", &d, STATUS);
		eval_col(vis->fgid, "fgid:int", &d, STATUS);

		eval_col(vis->supgid, "supgid:string", &d, STATUS);

		/* from tatm */
		eval_col(vis->size, "size:int", &d, STATM);
		eval_col(vis->resident, "resident:int", &d, STATM);
		eval_col(vis->share, "share:int", &d, STATM);
		eval_col(vis->trs, "trs:int", &d, STATM);
		eval_col(vis->drs, "drs:int", &d, STATM);

		/* other */
		eval_col(vis->cmdline, "cmdline:string[]", &d, CMDLINE);
		eval_col(vis->cgroup, "cgroup:string[]", &d, CGROUP);

		eval_col(vis->oom_score, "oom_score:int", &d, OOM);
		eval_col(vis->oom_adj, "oom_adj:int", &d, OOM);

		eval_col(vis->ns_ipc, "ns_ipc:int", &d, NS);
		eval_col(vis->ns_mnt, "ns_mnt:int", &d, NS);
		eval_col(vis->ns_net, "ns_net:int", &d, NS);
		eval_col(vis->ns_pid, "ns_pid:int", &d, NS);
		eval_col(vis->ns_user, "ns_user:int", &d, NS);
		eval_col(vis->ns_uts, "ns_uts:int", &d, NS);

		eval_col(vis->sd_mach, "sd_mach:string", &d, SD);
	        eval_col(vis->sd_ouid, "sd_ouid:string", &d, SD);
	        eval_col(vis->sd_seat, "sd_seat:string", &d, SD);
	        eval_col(vis->sd_sess, "sd_sess:string", &d, SD);
	        eval_col(vis->sd_slice, "sd_slice:string", &d, SD);
	        eval_col(vis->sd_unit, "sd_unit:string", &d, SD);
	        eval_col(vis->sd_uunit, "sd_uunit:string", &d, SD);

	        eval_col(vis->lxcname, "lxcname:string", &d, LXC);

		eval_col(vis->environ, "environ:string", &d, ENVIRON);

		eval_col(vis->euid_name, "euid_name:string", &d, DEF);
		eval_col(vis->egid_name, "egid_name:string", &d, DEF);

		eval_col(vis->ruid_name, "ruid_name:string", &d, STATUS);
		eval_col(vis->rgid_name, "rgid_name:string", &d, STATUS);
		eval_col(vis->suid_name, "suid_name:string", &d, STATUS);
		eval_col(vis->sgid_name, "sgid_name:string", &d, STATUS);
		eval_col(vis->fuid_name, "fuid_name:string", &d, STATUS);
		eval_col(vis->fgid_name, "fgid_name:string", &d, STATUS);

		eval_col(vis->supgid_names, "supgid_names:string", &d, STATUS);

		d.count = d.visible_count;
		d.visible_count = 0;
		if (print_header)
			d.print++;
	} while (d.print == 1);

	summary->count = d.count;
	summary->sources = d.sources;
}

struct visibility_info {
	struct visible_columns *cols;
	size_t count;
};

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

static long
get_hz(void)
{
	static long hz = 0;
	if (hz)
		return hz;
	hz = sysconf(_SC_CLK_TCK);
	if (hz <= 0) {
		fprintf(stderr,
			"unable to determine HZ value (%ld %d), assuming 100\n",
			hz, errno);
		hz = 100;
	}

	return hz;
}

static void
print_proc(proc_t *proc, struct visible_columns *vis,
		struct col_summary *summary)
{
	struct visibility_info visinfo = {vis, summary->count};
	struct print_ctx ctx = {0, &visinfo};

	/* always */
	if (vis->tid)
		cprint(&ctx, "%d", proc->tid);
	if (vis->tgid)
		cprint(&ctx, "%d", proc->tgid);
	if (vis->euid)
		cprint(&ctx, "%d", proc->euid);
	if (vis->egid)
		cprint(&ctx, "%d", proc->egid);

	/* stat, status */
	if (vis->ppid)
		cprint(&ctx, "%d", proc->ppid);
	if (vis->state)
		cprint(&ctx, "%c", proc->state);
	if (vis->cmd) {
		csv_print_quoted(proc->cmd, strlen(proc->cmd));
		cprint(&ctx, "");
	}
	if (vis->nlwp)
		cprint(&ctx, "%d", proc->nlwp);
	if (vis->wchan) {
		if (proc->wchan == -1)
			cprint(&ctx, "-1");
		else
			cprint(&ctx, "0x%lx", proc->wchan);
	}

	/* stat */
	if (vis->pgrp)
		cprint(&ctx, "%d", proc->pgrp);
	if (vis->session)
		cprint(&ctx, "%d", proc->session);

	if (vis->user_time_ms)
		cprint(&ctx, "%llu", proc->utime * 1000 / get_hz());
	if (vis->system_time_ms)
		cprint(&ctx, "%llu", proc->stime * 1000 / get_hz());
	if (vis->cumulative_user_time_ms)
		cprint(&ctx, "%llu", proc->cutime * 1000 / get_hz());
	if (vis->cumulative_system_time_ms)
		cprint(&ctx, "%llu", proc->cstime * 1000 / get_hz());
	if (vis->start_time)
		cprint(&ctx, "%llu", proc->start_time);

	if (vis->start_code)
		cprint(&ctx, "0x%lx", proc->start_code);
	if (vis->end_code)
		cprint(&ctx, "0x%lx", proc->end_code);
	if (vis->start_stack)
		cprint(&ctx, "0x%lx", proc->start_stack);
	if (vis->kstk_esp)
		cprint(&ctx, "0x%lx", proc->kstk_esp);
	if (vis->kstk_eip)
		cprint(&ctx, "0x%lx", proc->kstk_eip);

	if (vis->priority)
		cprint(&ctx, "%ld", proc->priority);
	if (vis->nice)
		cprint(&ctx, "%ld", proc->nice);
	if (vis->rss)
		cprint(&ctx, "%ld", proc->rss * PageSize);
	if (vis->alarm)
		cprint(&ctx, "%ld", proc->alarm);

	if (vis->rtprio)
		cprint(&ctx, "%lu", proc->rtprio);
	if (vis->sched)
		cprint(&ctx, "%lu", proc->sched);
	if (vis->vsize)
		cprint(&ctx, "%lu", proc->vsize);
	if (vis->rss_rlim)
		cprint(&ctx, "%ld", proc->rss_rlim);
	if (vis->flags)
		cprint(&ctx, "0x%lx", proc->flags);
	if (vis->min_flt)
		cprint(&ctx, "%lu", proc->min_flt);
	if (vis->maj_flt)
		cprint(&ctx, "%lu", proc->maj_flt);
	if (vis->cmin_flt)
		cprint(&ctx, "%lu", proc->cmin_flt);
	if (vis->cmaj_flt)
		cprint(&ctx, "%lu", proc->cmaj_flt);

	if (vis->tty)
		cprint(&ctx, "%d", proc->tty);
	if (vis->tpgid)
		cprint(&ctx, "%d", proc->tpgid);
	if (vis->exit_signal)
		cprint(&ctx, "%d", proc->exit_signal);
	if (vis->processor)
		cprint(&ctx, "%d", proc->processor);

	/* status */
	if (vis->pending_signals) {
		csv_print_quoted(proc->signal, strlen(proc->signal));
		cprint(&ctx, "");
	}
	if (vis->blocked_signals) {
		csv_print_quoted(proc->blocked, strlen(proc->blocked));
		cprint(&ctx, "");
	}
	if (vis->ignored_signals) {
		csv_print_quoted(proc->sigignore, strlen(proc->sigignore));
		cprint(&ctx, "");
	}
	if (vis->caught_signals) {
		csv_print_quoted(proc->sigcatch, strlen(proc->sigcatch));
		cprint(&ctx, "");
	}
	if (vis->pending_signals_per_task) {
		csv_print_quoted(proc->_sigpnd, strlen(proc->_sigpnd));
		cprint(&ctx, "");
	}

	if (vis->vm_size)
		cprint(&ctx, "%lu", proc->vm_size * 1024);
	if (vis->vm_lock)
		cprint(&ctx, "%lu", proc->vm_lock * 1024);
	if (vis->vm_rss)
		cprint(&ctx, "%lu", proc->vm_rss * 1024);
	if (vis->vm_rss_anon)
		cprint(&ctx, "%lu", proc->vm_rss_anon * 1024);
	if (vis->vm_rss_file)
		cprint(&ctx, "%lu", proc->vm_rss_file * 1024);
	if (vis->vm_rss_shared)
		cprint(&ctx, "%lu", proc->vm_rss_shared * 1024);
	if (vis->vm_data)
		cprint(&ctx, "%lu", proc->vm_data * 1024);
	if (vis->vm_stack)
		cprint(&ctx, "%lu", proc->vm_stack * 1024);
	if (vis->vm_swap)
		cprint(&ctx, "%lu", proc->vm_swap * 1024);
	if (vis->vm_exe)
		cprint(&ctx, "%lu", proc->vm_exe * 1024);
	if (vis->vm_lib)
		cprint(&ctx, "%lu", proc->vm_lib * 1024);

	if (vis->ruid)
		cprint(&ctx, "%d", proc->ruid);
	if (vis->rgid)
		cprint(&ctx, "%d", proc->rgid);
	if (vis->suid)
		cprint(&ctx, "%d", proc->suid);
	if (vis->sgid)
		cprint(&ctx, "%d", proc->sgid);
	if (vis->fuid)
		cprint(&ctx, "%d", proc->fuid);
	if (vis->fgid)
		cprint(&ctx, "%d", proc->fgid);

	if (vis->supgid) {
		csv_print_quoted(proc->supgid, strlen(proc->supgid));
		cprint(&ctx, "");
	}

	/* statm */
	if (vis->size)
		cprint(&ctx, "%ld", proc->size * PageSize);
	if (vis->resident)
		cprint(&ctx, "%ld", proc->resident * PageSize);
	if (vis->share)
		cprint(&ctx, "%ld", proc->share * PageSize);
	if (vis->trs)
		cprint(&ctx, "%ld", proc->trs * PageSize);
	if (vis->drs)
		cprint(&ctx, "%ld", proc->drs * PageSize);

	/* other */
	if (vis->cmdline) {
		fputc('"', stdout);

		char **cmd = proc->cmdline;

		if (cmd) {
			while (*cmd) {
				print_inside_quote(*cmd, strlen(*cmd));
				cmd++;
				if (*cmd)
					fputc(',', stdout);
			}
		}

		fputc('"', stdout);
		cprint(&ctx, "");
	}

	if (vis->cgroup) {
		fputc('"', stdout);

		char **cgroup = proc->cgroup;

		if (cgroup) {
			while (*cgroup) {
				print_inside_quote(*cgroup, strlen(*cgroup));
				cgroup++;
				if (*cgroup)
					fputc(',', stdout);
			}
		}

		fputc('"', stdout);
		cprint(&ctx, "");
	}

	if (vis->oom_score)
		cprint(&ctx, "%d", proc->oom_score);
	if (vis->oom_adj)
		cprint(&ctx, "%d", proc->oom_adj);

        if (vis->ns_ipc)
		cprint(&ctx, "0x%lx", proc->ns[IPCNS]);
        if (vis->ns_mnt)
		cprint(&ctx, "0x%lx", proc->ns[MNTNS]);
        if (vis->ns_net)
		cprint(&ctx, "0x%lx", proc->ns[NETNS]);
        if (vis->ns_pid)
		cprint(&ctx, "0x%lx", proc->ns[PIDNS]);
        if (vis->ns_user)
		cprint(&ctx, "0x%lx", proc->ns[USERNS]);
        if (vis->ns_uts)
		cprint(&ctx, "0x%lx", proc->ns[UTSNS]);

        if (vis->sd_mach) {
		csv_print_quoted(proc->sd_mach, strlen(proc->sd_mach));
		cprint(&ctx, "");
        }

        if (vis->sd_ouid) {
		csv_print_quoted(proc->sd_ouid, strlen(proc->sd_ouid));
		cprint(&ctx, "");
        }

        if (vis->sd_seat) {
		csv_print_quoted(proc->sd_seat, strlen(proc->sd_seat));
		cprint(&ctx, "");
        }

        if (vis->sd_sess) {
		csv_print_quoted(proc->sd_sess, strlen(proc->sd_sess));
		cprint(&ctx, "");
        }

        if (vis->sd_slice) {
		csv_print_quoted(proc->sd_slice, strlen(proc->sd_slice));
		cprint(&ctx, "");
        }

        if (vis->sd_unit) {
		csv_print_quoted(proc->sd_unit, strlen(proc->sd_unit));
		cprint(&ctx, "");
        }

        if (vis->sd_uunit) {
		csv_print_quoted(proc->sd_uunit, strlen(proc->sd_uunit));
		cprint(&ctx, "");
        }

        if (vis->lxcname) {
		csv_print_quoted(proc->lxcname, strlen(proc->lxcname));
		cprint(&ctx, "");
        }

        if (vis->environ) {
		if (proc->environ)
			csv_print_quoted(proc->environ[0], strlen(proc->environ[0]));
		cprint(&ctx, "");
	}

	if (vis->euid_name)
		cprint(&ctx, "%s", get_user(proc->euid));
	if (vis->egid_name)
		cprint(&ctx, "%s", get_group(proc->egid));

	if (vis->ruid_name)
		cprint(&ctx, "%s", get_user(proc->ruid));
	if (vis->rgid_name)
		cprint(&ctx, "%s", get_group(proc->rgid));
	if (vis->suid_name)
		cprint(&ctx, "%s", get_user(proc->suid));
	if (vis->sgid_name)
		cprint(&ctx, "%s", get_group(proc->sgid));
	if (vis->fuid_name)
		cprint(&ctx, "%s", get_user(proc->fuid));
	if (vis->fgid_name)
		cprint(&ctx, "%s", get_group(proc->fgid));

	if (vis->supgid_names) {
		if (strcmp(proc->supgid, "-") != 0) {
			char *gid_str = strtok(proc->supgid, ",");

			fputc('"', stdout);
			while (gid_str) {
				gid_t gid;
				if (strtou_safe(gid_str, &gid, 0)) {
					fprintf(stderr,
						"gid '%s' is not a number\n",
						gid_str);
					abort();
				}
				fprintf(stdout, "%s", get_group(gid));

				gid_str = strtok(NULL, ",");
				if (gid_str)
					fputc(',', stdout);
			}
			fputc('"', stdout);
		}

		cprint(&ctx, "");
	}
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
	pid_t *pids = NULL;
	size_t npids = 0;

	PageSize = sysconf(_SC_PAGESIZE);

	memset(&vis, 1, sizeof(vis));

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
		compute_visibility(cols, &vis);
		free(cols);
	}

	if (show)
		csv_show();

	struct col_summary summary;
	eval_visibility(&vis, print_header, &summary);

	if (print_header)
		printf("\n");

	if (vis.euid_name || vis.egid_name ||
			vis.ruid_name || vis.rgid_name ||
			vis.suid_name || vis.sgid_name ||
			vis.fuid_name || vis.fgid_name ||
			vis.supgid_names)
		usr_grp_query_init();

	int flags = 0;

	if (summary.sources & STAT)
		flags |= PROC_FILLSTAT;
	if (summary.sources & STATUS)
		flags |= PROC_FILLSTATUS;
	if (summary.sources & STAT_OR_STATUS) {
		if ((flags & (PROC_FILLSTAT | PROC_FILLSTATUS)) == 0)
			/* stat seems easier to parse */
			flags |= PROC_FILLSTAT;
	}
	if (summary.sources & STATM)
		flags |= PROC_FILLMEM;
	if (summary.sources & CMDLINE)
		flags |= PROC_FILLCOM;
	if (summary.sources & ENVIRON)
		flags |= PROC_FILLENV | PROC_EDITENVRCVT;
	if (summary.sources & CGROUP)
		flags |= PROC_FILLCGROUP;
	if (summary.sources & OOM)
		flags |= PROC_FILLOOM;
	if (summary.sources & NS)
		flags |= PROC_FILLNS;
	if (summary.sources & SD)
		flags |= PROC_FILLSYSTEMD;
	if (summary.sources & LXC)
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
		print_proc(proc, &vis, &summary);
		freeproc(proc);
	}

	closeproc(pt);

	if (pids)
		free(pids);

	if (vis.euid_name || vis.egid_name ||
			vis.ruid_name || vis.rgid_name ||
			vis.suid_name || vis.sgid_name ||
			vis.fuid_name || vis.fgid_name ||
			vis.supgid_names)
		usr_grp_query_fini();

	return 0;
}
