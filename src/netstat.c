/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

/* see sock_diag(7) for explanation of what printed columns mean */

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <linux/inet_diag.h>
#include <linux/unix_diag.h>
#include <linux/sock_diag.h>

#include <libmnl/libmnl.h>

#include "merge_utils.h"
#include "utils.h"

/* protocols */
#define INET4_TCP (1ULL << 0)
#define INET4_UDP (1ULL << 1)
#define INET4_RAW (1ULL << 2)

#define INET6_TCP (1ULL << 5)
#define INET6_UDP (1ULL << 6)
#define INET6_RAW (1ULL << 7)

#define UNIX (1ULL << 9)

#define INET4 (INET4_TCP | INET4_UDP | INET4_RAW)
#define INET6 (INET6_TCP | INET6_UDP | INET6_RAW)
#define TCP (INET4_TCP | INET6_TCP)
#define UDP (INET4_UDP | INET6_UDP)
#define RAW (INET4_RAW | INET6_RAW)

#define INET (INET4 | INET6)
#define ALL (INET4 | INET6 | UNIX)

/* sources of information */
#define INET_MEMINFO   (1ULL << 10)
#define INET_INFO      (1ULL << 11)
#define INET_VEGASINFO (1ULL << 12)
#define INET_DCTCPINFO (1ULL << 12)
#define INET_BBRINFO   (1ULL << 12)
#define INET_CONG      (1ULL << 13)
#define INET_TOS       (1ULL << 14)
#define INET_TCLASS    (1ULL << 15)
#define INET_SKMEMINFO (1ULL << 16)
#define INET_SHUTDOWN  (1ULL << 17)

#define UNIX_NAME      (1ULL << 20)
#define UNIX_VFS       (1ULL << 21)
#define UNIX_PEER      (1ULL << 22)
#define UNIX_ICONS     (1ULL << 23)
#define UNIX_RQLEN     (1ULL << 24)
#define UNIX_MEMINFO   (1ULL << 25)
#define UNIX_SHUTDOWN  (1ULL << 26)
#if UNIX_DIAG_UID_AVAILABLE
#define UNIX_UID       (1ULL << 27)
#endif

#define REQUIRES_RESOLVING (1ULL << 31)

static const struct option opts[] = {
	{"columns",		required_argument,	NULL, 'c'},
	{"merge",		no_argument,		NULL, 'M'},
	{"table-name",		required_argument,	NULL, 'N'},
//	{"sctp",		no_argument,		NULL, 'p'},
	{"resolve",		no_argument,		NULL, 'r'},
	{"show",		no_argument,		NULL, 's'},
	{"show-full",		no_argument,		NULL, 'S'},
	{"as-table",		no_argument,		NULL, 'T'},
	{"tcp",			no_argument,		NULL, 't'},
	{"udp",			no_argument,		NULL, 'u'},
//	{"udplite",		no_argument,		NULL, 'U'},
	{"raw",			no_argument,		NULL, 'w'},
	{"version",		no_argument,		NULL, 'V'},
	{"unix",		no_argument,		NULL, 'x'},
	{"help",		no_argument,		NULL, 'h'},
	{"inet4",		no_argument,		NULL, '4'},
	{"inet6",		no_argument,		NULL, '6'},
	{NULL,			0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-netstat [OPTION]...\n");
	fprintf(out, "Print to standard output the list of network sockets in the CSV format.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	describe_Columns(out);
	fprintf(out, "  -l                         use a longer listing format (can be used up to 3 times)\n");
	describe_Merge(out);
	describe_table_Name(out);
	fprintf(out, "  -r, --resolve              resolve IPs and ports\n");
	describe_Show(out);
	describe_Show_full(out);
//	fprintf(out, "  -p, --sctp                 print information only about SCTP sockets\n");
	describe_as_Table(out, "socket");
	fprintf(out, "  -t, --tcp                  print information only about TCP sockets\n");
	fprintf(out, "  -u, --udp                  print information only about UDP sockets\n");
//	fprintf(out, "  -U, --udplite              print information only about UDPLITE sockets\n");
	fprintf(out, "  -w, --raw                  print information only about RAW sockets\n");
	fprintf(out, "  -x, --unix                 print information only about UNIX sockets\n");
	fprintf(out, "  -4, --inet4                print information only about IPv4 sockets\n");
	fprintf(out, "  -6, --inet6                print information only about IPv6 sockets\n");
	describe_help(out);
	describe_version(out);
}

static inline int
attr_val_err(int type)
{
	fprintf(stderr, "mnl_attr_validate failed, type: %d, err: %s\n", type,
			strerror(errno));
#ifndef NDEBUG
	abort();
#endif
	return MNL_CB_ERROR;
}

static int
inet_data_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, INET_DIAG_MAX) < 0) {
//		fprintf(stderr, "unknown attribute %hu\n", attr->nla_type);
		return MNL_CB_OK;
	}

	switch(type) {
	case INET_DIAG_MEMINFO:
		if (mnl_attr_validate2(attr, MNL_TYPE_BINARY,
				sizeof(struct inet_diag_meminfo)) < 0) {
			return attr_val_err(type);
		}
		break;
	case INET_DIAG_INFO:
		if (mnl_attr_validate(attr, MNL_TYPE_BINARY) < 0) {
			return attr_val_err(type);
		}
		break;
	case INET_DIAG_VEGASINFO:
		/* not seen yet */
		raise(SIGTRAP);
		if (mnl_attr_validate2(attr, MNL_TYPE_BINARY,
				sizeof(struct tcpvegas_info)) < 0) {
			return attr_val_err(type);
		}
		break;
	case INET_DIAG_CONG:
		if (mnl_attr_validate(attr, MNL_TYPE_STRING) < 0) {
			return attr_val_err(type);
		}
		break;
	case INET_DIAG_TOS:
		if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
			return attr_val_err(type);
		break;
	case INET_DIAG_TCLASS:
		if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
			return attr_val_err(type);
		break;
	case INET_DIAG_SKMEMINFO:
		if (mnl_attr_validate2(attr, MNL_TYPE_BINARY,
				SK_MEMINFO_VARS * sizeof(uint32_t)) < 0) {
			return attr_val_err(type);
		}
		break;
	case INET_DIAG_SHUTDOWN:
		if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
			return attr_val_err(type);
		break;
	case INET_DIAG_DCTCPINFO:
		/* not seen yet */
		raise(SIGTRAP);
		/* requested by INET_DIAG_VEGASINFO? */
		if (mnl_attr_validate2(attr, MNL_TYPE_BINARY,
				sizeof(struct tcp_dctcp_info)) < 0) {
			return attr_val_err(type);
		}
		break;
	case INET_DIAG_PROTOCOL:
		if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
			return attr_val_err(type);
		break;
	case INET_DIAG_SKV6ONLY:
		if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
			return attr_val_err(type);
		break;
	case INET_DIAG_LOCALS:
		/* not seen yet */
		raise(SIGTRAP);
		/* variable size? */
		/* sctp only? */
		if (mnl_attr_validate(attr, MNL_TYPE_BINARY) < 0)
			return attr_val_err(type);
		break;
	case INET_DIAG_PEERS:
		/* not seen yet */
		raise(SIGTRAP);
		/* variable size? */
		/* sctp only? */
		if (mnl_attr_validate(attr, MNL_TYPE_BINARY) < 0)
			return attr_val_err(type);
		break;
	case INET_DIAG_PAD:
		/* not seen yet */
		raise(SIGTRAP);
		/* part of INET_DIAG_INFO? */
		if (mnl_attr_validate(attr, MNL_TYPE_U64) < 0)
			return attr_val_err(type);
		break;
	case INET_DIAG_MARK:
		/* requires NET_ADMIN */
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			return attr_val_err(type);
		break;
	case INET_DIAG_BBRINFO:
		/* not seen yet */
		raise(SIGTRAP);
		/* requested by INET_DIAG_VEGASINFO? */
		if (mnl_attr_validate2(attr, MNL_TYPE_BINARY,
				sizeof(struct tcp_bbr_info)) < 0) {
			return attr_val_err(type);
		}
		break;
	case INET_DIAG_CLASS_ID:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			return attr_val_err(type);
		break;
	case INET_DIAG_MD5SIG:
		/* requires NET_ADMIN? */
		/* not seen yet */
		raise(SIGTRAP);
		/* variable size? */
		/* part of INET_DIAG_INFO? */
		/* cnt * sizeof(struct tcp_diag_md5sig) */
		if (mnl_attr_validate(attr, MNL_TYPE_BINARY) < 0)
			return attr_val_err(type);
		break;
	default:
		fprintf(stderr, "unknown attr type: %d\n", type);
#ifndef NDEBUG
		abort();
#endif
		return MNL_CB_ERROR;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
dump_bin_attr(struct nlattr *tb)
{
	const char *m = mnl_attr_get_payload(tb);
	uint16_t len = mnl_attr_get_payload_len(tb);
	for (uint16_t i = 0; i < len; ++i)
		printf("%02hhx ", m[i]);
}

static const char *
family(uint8_t f)
{
	switch (f) {
		case AF_INET: return "INET4";
		case AF_INET6: return "INET6";
		case AF_UNIX: return "UNIX";
		default:
			fprintf(stderr, "unknown family: %hhu\n", f);
#ifndef NDEBUG
			abort();
#endif
			return "???";
	};
}

static const char *
socket_state(uint8_t state)
{
	static const char *states[] = {
		[0] = "???",
		[TCP_ESTABLISHED] = "ESTABLISHED",
		[TCP_SYN_SENT] = "SYN_SENT",
		[TCP_SYN_RECV] = "SYN_RECV",
		[TCP_FIN_WAIT1] = "FIN_WAIT1",
		[TCP_FIN_WAIT2] = "FIN_WAIT2",
		[TCP_TIME_WAIT] = "TIME_WAIT",
		[TCP_CLOSE] = "CLOSE",
		[TCP_CLOSE_WAIT] = "CLOSE_WAIT",
		[TCP_LAST_ACK] = "LAST_ACK",
		[TCP_LISTEN] = "LISTEN",
		[TCP_CLOSING] = "CLOSING",
	};

	if (state >= sizeof(states)/sizeof(states[0])) {
		fprintf(stderr, "unknown socket state: %hhu\n", state);
#ifndef NDEBUG
		abort();
#endif
		return "unknown";
	}

	return states[state];
}

static const char *
timer(uint8_t t)
{
	static const char *timer[] = {
		[0] = "not active",
		[1] = "retransmit",
		[2] = "keep-alive",
		[3] = "TIME_WAIT",
		[4] = "zero window probe",
	};

	if (t >= sizeof(timer)/sizeof(timer[0])) {
		fprintf(stderr, "unknown timer state: %hhu\n", t);
#ifndef NDEBUG
		abort();
#endif
		return "unknown";
	}

	return timer[t];
}

static const char *
protocol(uint8_t p)
{
	switch(p) {
	case IPPROTO_TCP: return "tcp";
	case IPPROTO_UDP: return "udp";
	case IPPROTO_RAW: return "raw";
	default:
		fprintf(stderr, "unknown protocol: %hhu\n", p);
#ifndef NDEBUG
		abort();
#endif
		return "???";
	}
}

struct sock_data {
	struct column_info *columns;
	size_t ncolumns;

	struct unix_diag_msg *unix_msg;
	struct nlattr *unix_attrs[__UNIX_DIAG_MAX + 1];

	struct inet_diag_msg *inet_msg;
	struct nlattr *inet_attrs[__INET_DIAG_MAX + 1];
	uint8_t protocol;

	struct csvmu_ctx *ctx;
};

static int
inet_data_cb(const struct nlmsghdr *nlh, void *data)
{
	struct inet_diag_msg *msg = mnl_nlmsg_get_payload(nlh);

	struct sock_data *sd = data;
	sd->unix_msg = NULL;
	sd->inet_msg = msg;
	memset(sd->inet_attrs, 0, sizeof(sd->inet_attrs));
	if (mnl_attr_parse(nlh, sizeof(*msg), inet_data_attr_cb,
			sd->inet_attrs) == MNL_CB_ERROR) {
		fprintf(stderr, "mnl_attr_parse failed\n");
		exit(2);
	}

	csvmu_print_row(sd->ctx, sd, sd->columns, sd->ncolumns);

	return MNL_CB_OK;
}

static int
unix_data_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, UNIX_DIAG_MAX) < 0) {
//		fprintf(stderr, "unknown attribute %hu\n", attr->nla_type);
		return MNL_CB_OK;
	}

	switch(type) {
	case UNIX_DIAG_NAME:
		if (mnl_attr_validate(attr, MNL_TYPE_BINARY) < 0) {
			return attr_val_err(type);
		}
		break;
	case UNIX_DIAG_VFS:
		if (mnl_attr_validate2(attr, MNL_TYPE_BINARY,
				sizeof(struct unix_diag_vfs)) < 0) {
			return attr_val_err(type);
		}
		break;
	case UNIX_DIAG_PEER:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0) {
			return attr_val_err(type);
		}
		break;
	case UNIX_DIAG_ICONS:
		if (mnl_attr_validate(attr, MNL_TYPE_BINARY) < 0) {
			return attr_val_err(type);
		}
		break;
	case UNIX_DIAG_RQLEN:
		if (mnl_attr_validate2(attr, MNL_TYPE_BINARY,
				sizeof(struct unix_diag_rqlen)) < 0)
			return attr_val_err(type);
		break;
	case UNIX_DIAG_MEMINFO:
		if (mnl_attr_validate2(attr, MNL_TYPE_BINARY,
				SK_MEMINFO_VARS * sizeof(uint32_t)) < 0) {
			return attr_val_err(type);
		}
		break;
	case UNIX_DIAG_SHUTDOWN:
		if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
			return attr_val_err(type);
		break;
#if UNIX_DIAG_UID_AVAILABLE
	case UNIX_DIAG_UID:
		/* not tested yet */
		raise(SIGTRAP);
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			return attr_val_err(type);
		break;
#endif
	default:
		fprintf(stderr, "unknown type: %d\n", type);
#ifndef NDEBUG
		abort();
#endif
		return MNL_CB_ERROR;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static const char *
unix_type(uint8_t t)
{
	switch(t) {
	case SOCK_DGRAM: return "DGRAM";
	case SOCK_PACKET: return "PACKET";
	case SOCK_STREAM: return "STREAM";
	case SOCK_SEQPACKET: return "SEQPACKET";
	default:
		fprintf(stderr, "unknown unix socket type: %hhu\n", t);
#ifndef NDEBUG
		abort();
#endif
		return "???";
	}
}

static int
unix_data_cb(const struct nlmsghdr *nlh, void *data)
{
	struct unix_diag_msg *msg = mnl_nlmsg_get_payload(nlh);

	struct sock_data *sd = data;
	sd->unix_msg = msg;
	sd->inet_msg = NULL;
	memset(sd->unix_attrs, 0, sizeof(sd->unix_attrs));
	if (mnl_attr_parse(nlh, sizeof(*msg), unix_data_attr_cb,
			sd->unix_attrs) == MNL_CB_ERROR) {
		fprintf(stderr, "mnl_attr_parse failed\n");
		exit(2);
	}

	csvmu_print_row(sd->ctx, sd, sd->columns, sd->ncolumns);

	return MNL_CB_OK;
}

static void
print_family(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->unix_msg)
		printf("%s", family(sd->unix_msg->udiag_family));
	else if (sd->inet_msg)
		printf("%s", family(sd->inet_msg->idiag_family));
}

static void
print_state(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->unix_msg && sd->unix_msg->udiag_type != SOCK_DGRAM)
		printf("%s", socket_state(sd->unix_msg->udiag_state));
	else if (sd->inet_msg && sd->protocol == IPPROTO_TCP)
		printf("%s", socket_state(sd->inet_msg->idiag_state));
}

static void
print_type(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->unix_msg) {
		printf("%s", unix_type(sd->unix_msg->udiag_type));
	} else if (sd->inet_msg) {
		/* no direct data for non-unix sockets, should we translate
		 * tcp to stream and udp to dgram? what about sctp and dccp?
		 * (at least sctp has 2 variants) */
	}
}

static void
print_inode(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->unix_msg)
		printf("%u", sd->unix_msg->udiag_ino);
	else if (sd->inet_msg)
		printf("%u", sd->inet_msg->idiag_inode);
}

static void
print_cookie0(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->unix_msg)
		printf("%u", sd->unix_msg->udiag_cookie[0]);
	else if (sd->inet_msg)
		printf("%u", sd->inet_msg->id.idiag_cookie[0]);
}

static void
print_cookie1(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->unix_msg)
		printf("%u", sd->unix_msg->udiag_cookie[1]);
	else if (sd->inet_msg)
		printf("%u", sd->inet_msg->id.idiag_cookie[1]);
}


static void
print_name_raw(const char *m, uint16_t len)
{
	if (len) {
		for (uint16_t i = 0; i < len - 1; ++i) {
			if (m[i] == '"')
				printf("\"\"");
			else if (m[i] == 0)
				putchar('@');
			else
				putchar(m[i]);
		}
	}

	if (m[len - 1]) {
		if (m[len - 1] == '"')
			printf("\"\"");
		else
			printf("%c", m[len - 1]);
	}
}

static void
print_name(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->unix_msg) {
		struct nlattr *attr = sd->unix_attrs[UNIX_DIAG_NAME];
		if (attr) {
			const char *m = mnl_attr_get_payload(attr);
			uint16_t len = mnl_attr_get_payload_len(attr);

			const char *comma = strnchr(m, ',', len);
			const char *nl = strnchr(m, '\n', len);
			const char *quot = strnchr(m, '"', len);

			if (!comma && !nl && !quot) {
				print_name_raw(m, len);
			} else {
				printf("\"");
				print_name_raw(m, len);
				printf("\"");
			}
		}
	}
}

static void
print_vfs_dev(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->unix_msg) {
		struct nlattr *attr = sd->unix_attrs[UNIX_DIAG_VFS];
		if (attr) {
			struct unix_diag_vfs *v = mnl_attr_get_payload(attr);

			printf("0x%x", v->udiag_vfs_dev);
		}
	}
}

static void
print_vfs_ino(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->unix_msg) {
		struct nlattr *attr = sd->unix_attrs[UNIX_DIAG_VFS];
		if (attr) {
			struct unix_diag_vfs *v = mnl_attr_get_payload(attr);

			printf("%u", v->udiag_vfs_ino);
		}
	}
}

static void
print_peer_inode(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->unix_msg) {
		struct nlattr *attr = sd->unix_attrs[UNIX_DIAG_PEER];
		if (attr)
			printf("%u", mnl_attr_get_u32(attr));
	}
}

static void
print_icons(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->unix_msg) {
		/* inodes of pending connections (Incoming CONnectionS?) */
		struct nlattr *attr = sd->unix_attrs[UNIX_DIAG_ICONS];
		if (attr) {
			uint32_t *inodes = mnl_attr_get_payload(attr);
			uint16_t len = mnl_attr_get_payload_len(attr);
			assert(len % 4 == 0);
			len /= 4;

			if (len == 1) {
				printf("%d", inodes[0]);
			} else if (len > 1) {
				printf("\"");
				for (uint16_t i = 0; i < len - 1; ++i)
					printf("%d,", inodes[i]);
				printf("%d", inodes[len - 1]);
				printf("\"");
			}
		}
	}
}

static void
print_pending_conns(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->unix_msg) {
		struct nlattr *attr = sd->unix_attrs[UNIX_DIAG_RQLEN];
		if (attr && sd->unix_msg->udiag_state == TCP_LISTEN) {
			struct unix_diag_rqlen *r = mnl_attr_get_payload(attr);
			printf("%u", r->udiag_rqueue);
		}
	} else if (sd->inet_msg) {
		if (sd->inet_msg->idiag_state == TCP_LISTEN)
			printf("%u", sd->inet_msg->idiag_rqueue);
	}
}

static void
print_incoming_data(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->unix_msg) {
		struct nlattr *attr = sd->unix_attrs[UNIX_DIAG_RQLEN];
		if (attr && sd->unix_msg->udiag_state == TCP_ESTABLISHED) {
			struct unix_diag_rqlen *r = mnl_attr_get_payload(attr);
			printf("%u", r->udiag_rqueue);
		}
	} else if (sd->inet_msg) {
		if (sd->inet_msg->idiag_state != TCP_LISTEN)
			printf("%u", sd->inet_msg->idiag_rqueue);
	}
}

static void
print_backlog_length(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->unix_msg) {
		struct nlattr *attr = sd->unix_attrs[UNIX_DIAG_RQLEN];
		if (attr && sd->unix_msg->udiag_state == TCP_LISTEN) {
			struct unix_diag_rqlen *r = mnl_attr_get_payload(attr);
			printf("%u", r->udiag_wqueue);
		}
	} else if (sd->inet_msg) {
		if (sd->inet_msg->idiag_state == TCP_LISTEN)
			printf("%u", sd->inet_msg->idiag_wqueue);
	}
}

static void
print_outgoing_data(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->unix_msg) {
		struct nlattr *attr = sd->unix_attrs[UNIX_DIAG_RQLEN];
		if (attr && sd->unix_msg->udiag_state == TCP_ESTABLISHED) {
			struct unix_diag_rqlen *r = mnl_attr_get_payload(attr);
			printf("%u", r->udiag_wqueue);
		}
	} else if (sd->inet_msg) {
		if (sd->inet_msg->idiag_state != TCP_LISTEN)
			printf("%u", sd->inet_msg->idiag_wqueue);
	}
}

static void
print_meminfo(const void *p, int idx)
{
	const struct sock_data *sd = p;
	struct nlattr *attr = NULL;

	if (sd->unix_msg)
		attr = sd->unix_attrs[UNIX_DIAG_MEMINFO];
	else if (sd->inet_msg)
		attr = sd->inet_attrs[INET_DIAG_SKMEMINFO];

	if (attr) {
		uint32_t *m = mnl_attr_get_payload(attr);
		printf("%u", m[idx]);
	}
}

static void
print_rmem_alloc(const void *p)
{
	print_meminfo(p, SK_MEMINFO_RMEM_ALLOC);
}

static void
print_rcvbuf(const void *p)
{
	print_meminfo(p, SK_MEMINFO_RCVBUF);
}

static void
print_wmem_alloc(const void *p)
{
	print_meminfo(p, SK_MEMINFO_WMEM_ALLOC);
}

static void
print_sndbuf(const void *p)
{
	print_meminfo(p, SK_MEMINFO_SNDBUF);
}

static void
print_fwd_alloc(const void *p)
{
	print_meminfo(p, SK_MEMINFO_FWD_ALLOC);
}

static void
print_wmem_queued(const void *p)
{
	print_meminfo(p, SK_MEMINFO_WMEM_QUEUED);
}

static void
print_optmem(const void *p)
{
	print_meminfo(p, SK_MEMINFO_OPTMEM);
}

static void
print_backlog(const void *p)
{
	print_meminfo(p, SK_MEMINFO_BACKLOG);
}

static void
print_drops(const void *p)
{
	print_meminfo(p, SK_MEMINFO_DROPS);
}

static void
print_shutdown(const void *p)
{
	const struct sock_data *sd = p;
	struct nlattr *attr = NULL;

	if (sd->unix_msg)
		attr = sd->unix_attrs[UNIX_DIAG_SHUTDOWN];
	else if (sd->inet_msg)
		attr = sd->inet_attrs[INET_DIAG_SHUTDOWN];

	if (attr) {
		uint8_t u = mnl_attr_get_u8(attr);
		printf("%hhu", u);
	}
}

static void
print_protocol(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg)
		printf("%s", protocol(sd->protocol));
}

static void
print_ip(struct inet_diag_msg *inet, __be32 *nip)
{
	if (inet->idiag_family != AF_INET && inet->idiag_family != AF_INET6)
		abort();

	char buf[INET6_ADDRSTRLEN];

	const char *res = inet_ntop(inet->idiag_family, nip, buf, sizeof(buf));
	if (!res) {
		perror("inet_ntop");
		return;
	}

	printf("%s", res);
}


static void
print_src_ip(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg)
		print_ip(sd->inet_msg, sd->inet_msg->id.idiag_src);
}

static void
print_src_name(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct hostent *h = gethostbyaddr(sd->inet_msg->id.idiag_src,
				sd->inet_msg->idiag_family == AF_INET6 ? 16 : 4,
				sd->inet_msg->idiag_family);
		if (h)
			csv_print_quoted(h->h_name, strlen(h->h_name));
		else
			print_ip(sd->inet_msg, sd->inet_msg->id.idiag_src);
	}
}

static void
print_src_port(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg)
		printf("%hu", ntohs(sd->inet_msg->id.idiag_sport));
}

static void
print_src_port_name(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		uint16_t port = sd->inet_msg->id.idiag_sport;
		struct servent *serv = getservbyport(port,
				protocol(sd->protocol));
		if (serv)
			printf("%s", serv->s_name);
		else
			printf("%hu", ntohs(port));
	}
}

static void
print_dst_ip(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg)
		print_ip(sd->inet_msg, sd->inet_msg->id.idiag_dst);
}

static void
print_dst_name(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct hostent *h = gethostbyaddr(sd->inet_msg->id.idiag_dst,
				sd->inet_msg->idiag_family == AF_INET6 ? 16 : 4,
				sd->inet_msg->idiag_family);
		if (h)
			csv_print_quoted(h->h_name, strlen(h->h_name));
		else
			print_ip(sd->inet_msg, sd->inet_msg->id.idiag_dst);
	}
}

static void
print_dst_port(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg)
		printf("%hu", ntohs(sd->inet_msg->id.idiag_dport));
}

static void
print_dst_port_name(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		uint16_t port = sd->inet_msg->id.idiag_dport;
		struct servent *serv = getservbyport(port,
				protocol(sd->protocol));
		if (serv)
			printf("%s", serv->s_name);
		else
			printf("%hu", ntohs(port));
	}
}

static void
print_interface(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) // XXX translate it to interface name using rtnetlink
		printf("%u", sd->inet_msg->id.idiag_if);
}

static void
print_timer(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg)
		printf("%s", timer(sd->inet_msg->idiag_timer));
}

static void
print_retransmits(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg)
		printf("%hhu", sd->inet_msg->idiag_retrans);
}

static void
print_expires_ms(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg)
		printf("%u", sd->inet_msg->idiag_expires);
}

static void
print_uid(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg)
		printf("%u", sd->inet_msg->idiag_uid);

#if UNIX_DIAG_UID_AVAILABLE
	if (sd->unix_msg) {
		struct nlattr *attr = sd->unix_attrs[UNIX_DIAG_UID];

		if (attr) {
			uint8_t u = mnl_attr_get_u32(attr);
			printf("%u", u);
		}
	}
#endif
}

static void
print_rmem(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_MEMINFO];
		if (attr) {
			const struct inet_diag_meminfo *meminfo =
					mnl_attr_get_payload(attr);
			printf("%u", meminfo->idiag_rmem);
		}
	}
}

static void
print_wmem(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_MEMINFO];
		if (attr) {
			const struct inet_diag_meminfo *meminfo =
					mnl_attr_get_payload(attr);
			printf("%u", meminfo->idiag_wmem);
		}
	}
}

static void
print_fmem(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_MEMINFO];
		if (attr) {
			const struct inet_diag_meminfo *meminfo =
					mnl_attr_get_payload(attr);
			printf("%u", meminfo->idiag_fmem);
		}
	}
}

static void
print_tmem(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_MEMINFO];
		if (attr) {
			const struct inet_diag_meminfo *meminfo =
					mnl_attr_get_payload(attr);
			printf("%u", meminfo->idiag_tmem);
		}
	}
}

static void
print_vegas_enabled(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_VEGASINFO];
		if (attr) {
			const struct tcpvegas_info *m =
					mnl_attr_get_payload(attr);
			printf("%u", m->tcpv_enabled);
		}
	}
}

static void
print_vegas_rttcnt(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_VEGASINFO];
		if (attr) {
			const struct tcpvegas_info *m =
					mnl_attr_get_payload(attr);
			printf("%u", m->tcpv_rttcnt);
		}
	}
}

static void
print_vegas_rtt(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_VEGASINFO];
		if (attr) {
			const struct tcpvegas_info *m =
					mnl_attr_get_payload(attr);
			printf("%u", m->tcpv_rtt);
		}
	}
}

static void
print_vegas_minrtt(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_VEGASINFO];
		if (attr) {
			const struct tcpvegas_info *m =
					mnl_attr_get_payload(attr);
			printf("%u", m->tcpv_minrtt);
		}
	}
}

static void
print_cong(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_CONG];
		if (attr) {
			const char *m = mnl_attr_get_str(attr);
			printf("%s", m);
		}
	}
}

static void
print_tos(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_TOS];
		if (attr) {
			uint8_t u = mnl_attr_get_u8(attr);
			printf("%hhu", u);
		}
	}
}

static void
print_tclass(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_TCLASS];
		if (attr) {
			uint8_t u = mnl_attr_get_u8(attr);
			printf("%hhu", u);
		}
	}
}

static void
print_user_protocol(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_PROTOCOL];
		if (attr) {
			uint8_t u = mnl_attr_get_u8(attr);
			struct protoent *proto = getprotobynumber(u);
			if (proto)
				printf("%s", proto->p_name);
			else
				printf("%hhu", u);
		}
	}
}

static void
print_ipv6only(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_SKV6ONLY];
		if (attr) {
			uint8_t u = mnl_attr_get_u8(attr);
			printf("%hhu", u);
		}
	}
}

static void
print_locals(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_LOCALS];
		if (attr)
			dump_bin_attr(attr);
	}
}

static void
print_peers(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_PEERS];
		if (attr)
			dump_bin_attr(attr);
	}
}

static void
print_pad(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_PAD];
		if (attr) {
			uint64_t u = mnl_attr_get_u64(attr);
			printf("%lu", u);
		}
	}
}

static void
print_mark(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_MARK];
		if (attr) {
			uint32_t u = mnl_attr_get_u32(attr);
			printf("%u", u);
		}
	}
}

static void
print_dctcp_enabled(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_DCTCPINFO];
		if (attr) {
			const struct tcp_dctcp_info *m =
					mnl_attr_get_payload(attr);
			printf("%hu", m->dctcp_enabled);
		}
	}
}

static void
print_dctcp_ce_state(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_DCTCPINFO];
		if (attr) {
			const struct tcp_dctcp_info *m =
					mnl_attr_get_payload(attr);
			printf("%hu", m->dctcp_ce_state);
		}
	}
}

static void
print_dctcp_alpha(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_DCTCPINFO];
		if (attr) {
			const struct tcp_dctcp_info *m =
					mnl_attr_get_payload(attr);
			printf("%u", m->dctcp_alpha);
		}
	}
}

static void
print_dctcp_ab_ecn(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_DCTCPINFO];
		if (attr) {
			const struct tcp_dctcp_info *m =
					mnl_attr_get_payload(attr);
			printf("%u", m->dctcp_ab_ecn);
		}
	}
}

static void
print_dctcp_ab_tot(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_DCTCPINFO];
		if (attr) {
			const struct tcp_dctcp_info *m =
					mnl_attr_get_payload(attr);
			printf("%u", m->dctcp_ab_tot);
		}
	}
}

static void
print_bbr_bw_lo(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_BBRINFO];
		if (attr) {
			const struct tcp_bbr_info *m =
					mnl_attr_get_payload(attr);
			printf("%u", m->bbr_bw_lo);
		}
	}
}

static void
print_bbr_bw_hi(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_BBRINFO];
		if (attr) {
			const struct tcp_bbr_info *m =
					mnl_attr_get_payload(attr);
			printf("%u", m->bbr_bw_hi);
		}
	}
}

static void
print_bbr_min_rtt(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_BBRINFO];
		if (attr) {
			const struct tcp_bbr_info *m =
					mnl_attr_get_payload(attr);
			printf("%u", m->bbr_min_rtt);
		}
	}
}

static void
print_bbr_pacing_gain(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_BBRINFO];
		if (attr) {
			const struct tcp_bbr_info *m =
					mnl_attr_get_payload(attr);
			printf("%u", m->bbr_pacing_gain);
		}
	}
}

static void
print_bbr_cwnd_gain(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_BBRINFO];
		if (attr) {
			const struct tcp_bbr_info *m =
					mnl_attr_get_payload(attr);
			printf("%u", m->bbr_cwnd_gain);
		}
	}
}

static void
print_class_id(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_CLASS_ID];
		if (attr) {
			uint32_t u = mnl_attr_get_u32(attr);
			printf("%u", u);
		}
	}
}

static void
print_md5sig(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_MD5SIG];
		if (attr)
			dump_bin_attr(attr);
	}
}

static void
print_info(const void *p)
{
	const struct sock_data *sd = p;
	if (sd->inet_msg) {
		struct nlattr *attr = sd->inet_attrs[INET_DIAG_INFO];
		if (attr) {
			/* XXX figure out how to interpret this yet */
			dump_bin_attr(attr);
		}
	}
}

#define ALL_STATES (\
		(1 << TCP_ESTABLISHED) | \
		(1 << TCP_SYN_SENT) | \
		(1 << TCP_SYN_RECV) | \
		(1 << TCP_FIN_WAIT1) | \
		(1 << TCP_FIN_WAIT2) | \
		(1 << TCP_TIME_WAIT) | \
		(1 << TCP_CLOSE) | \
		(1 << TCP_CLOSE_WAIT) | \
		(1 << TCP_LAST_ACK) | \
		(1 << TCP_LISTEN) | \
		(1 << TCP_CLOSING) )

static void
dump(struct mnl_socket *nl, uint8_t family, uint8_t protocol,
		struct column_info *columns, size_t ncolumns,
		struct csvmu_ctx *ctx)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	mnl_cb_t cb;

	unsigned portid = mnl_socket_get_portid(nl);

	struct sock_data sd;
	sd.columns = columns;
	sd.ncolumns = ncolumns;
	sd.protocol = protocol;

	sd.ctx = ctx;

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type	= SOCK_DIAG_BY_FAMILY;
	nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ROOT;
	nlh->nlmsg_seq = 0;

	if (family == AF_UNIX) {
		struct unix_diag_req *req;
		req = mnl_nlmsg_put_extra_header(nlh, sizeof(*req));
		req->sdiag_family = family;
		req->udiag_states = ALL_STATES;
		req->udiag_show = 0;

		for (size_t i = 0; i < ncolumns; ++i) {
			if (columns[i].data & UNIX_NAME)
				req->udiag_show |= UDIAG_SHOW_NAME;
			if (columns[i].data & UNIX_VFS)
				req->udiag_show |= UDIAG_SHOW_VFS;
			if (columns[i].data & UNIX_PEER)
				req->udiag_show |= UDIAG_SHOW_PEER;
			if (columns[i].data & UNIX_ICONS)
				req->udiag_show |= UDIAG_SHOW_ICONS;
			if (columns[i].data & UNIX_RQLEN)
				req->udiag_show |= UDIAG_SHOW_RQLEN;
			if (columns[i].data & UNIX_MEMINFO)
				req->udiag_show |= UDIAG_SHOW_MEMINFO;
#if UNIX_DIAG_UID_AVAILABLE
			if (columns[i].data & UNIX_UID)
				req->udiag_show |= UDIAG_SHOW_UID;
#endif
		}

		cb = unix_data_cb;
	} else {
		struct inet_diag_req_v2 *req;
		req = mnl_nlmsg_put_extra_header(nlh, sizeof(*req));
		req->sdiag_family = family;
		req->sdiag_protocol = protocol;
		req->idiag_states = ALL_STATES;

		req->idiag_ext = 0;
		for (size_t i = 0; i < ncolumns; ++i) {
			if (columns[i].data & INET_MEMINFO)
				req->idiag_ext |= 1 << (INET_DIAG_MEMINFO - 1);
			if (columns[i].data & INET_INFO)
				req->idiag_ext |= 1 << (INET_DIAG_INFO - 1);
			if (columns[i].data & INET_VEGASINFO)
				req->idiag_ext |= 1 << (INET_DIAG_VEGASINFO - 1);
			if (columns[i].data & INET_CONG)
				req->idiag_ext |= 1 << (INET_DIAG_CONG - 1);
			if (columns[i].data & INET_TOS)
				req->idiag_ext |= 1 << (INET_DIAG_TOS - 1);
			if (columns[i].data & INET_TCLASS)
				req->idiag_ext |= 1 << (INET_DIAG_TCLASS - 1);
			if (columns[i].data & INET_SKMEMINFO)
				req->idiag_ext |= 1 << (INET_DIAG_SKMEMINFO - 1);
			if (columns[i].data & INET_SHUTDOWN)
				req->idiag_ext |= 1 << (INET_DIAG_SHUTDOWN - 1);
		}

		cb = inet_data_cb;
	}

	if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0) {
		perror("mnl_socket_sendto");
		exit(2);
	}

	ssize_t ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	while (ret > 0) {
		ret = mnl_cb_run(buf, (size_t)ret, 0, portid, cb, &sd);
		if (ret <= 0)
			break;

		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	}

	if (ret == -1) {
		perror("mnl");
		exit(2);
	}
}

int
main(int argc, char *argv[])
{
	int opt;
	char *cols = NULL;
	unsigned show_flags = SHOW_DISABLED;
	unsigned protocols = 0;
	bool merge = false;
	char *table = NULL;

	struct column_info columns[] = {
		{ true,  0, 0, "family",          TYPE_STRING, print_family,         ALL },
		{ true,  0, 0, "protocol",        TYPE_STRING, print_protocol,       INET },
		{ true,  0, 0, "src_ip",          TYPE_STRING, print_src_ip,         INET },
		{ true,  0, 0, "src_port",        TYPE_INT,    print_src_port,       INET },
		{ true,  0, 0, "dst_ip",          TYPE_STRING, print_dst_ip,         INET },
		{ true,  0, 0, "dst_port",        TYPE_INT,    print_dst_port,       INET },
		{ false, 0, 0, "src_name",        TYPE_STRING, print_src_name,       INET | REQUIRES_RESOLVING },
		{ false, 0, 0, "src_port_name",   TYPE_STRING, print_src_port_name,  INET | REQUIRES_RESOLVING },
		{ false, 0, 0, "dst_name",        TYPE_STRING, print_dst_name,       INET | REQUIRES_RESOLVING },
		{ false, 0, 0, "dst_port_name",   TYPE_STRING, print_dst_port_name,  INET | REQUIRES_RESOLVING },
		{ true,  0, 0, "state",           TYPE_STRING, print_state,          ALL },
		{ true,  0, 0, "name",            TYPE_STRING, print_name,           UNIX | UNIX_NAME },
		{ true,  0, 0, "inode",           TYPE_INT,    print_inode,          ALL },
		{ true,  0, 0, "peer_inode",      TYPE_INT,    print_peer_inode,     UNIX | UNIX_PEER },
		{ true,  0, 0, "type",            TYPE_STRING, print_type,           UNIX },
		{ true,  0, 0, "uid",             TYPE_INT,    print_uid,            INET
#if UNIX_DIAG_UID_AVAILABLE
| UNIX | UNIX_UID
#endif
		},
		{ true,  0, 0, "interface",       TYPE_INT,    print_interface,      INET },
		{ false, 0, 1, "pending_conns",   TYPE_INT,    print_pending_conns,  ALL | UNIX_RQLEN },
		{ false, 0, 1, "incoming_data",   TYPE_INT,    print_incoming_data,  ALL | UNIX_RQLEN },
		{ false, 0, 1, "backlog_length",  TYPE_INT,    print_backlog_length, ALL | UNIX_RQLEN },
		{ false, 0, 1, "outgoing_data",   TYPE_INT,    print_outgoing_data,  ALL | UNIX_RQLEN },
		{ false, 0, 1, "rmem_alloc",      TYPE_INT,    print_rmem_alloc,     ALL | INET_SKMEMINFO | UNIX_MEMINFO },
		{ false, 0, 1, "rcvbuf",          TYPE_INT,    print_rcvbuf,         ALL | INET_SKMEMINFO | UNIX_MEMINFO },
		{ false, 0, 1, "wmem_alloc",      TYPE_INT,    print_wmem_alloc,     ALL | INET_SKMEMINFO | UNIX_MEMINFO },
		{ false, 0, 1, "sndbuf",          TYPE_INT,    print_sndbuf,         ALL | INET_SKMEMINFO | UNIX_MEMINFO },
		{ false, 0, 1, "fwd_alloc",       TYPE_INT,    print_fwd_alloc,      ALL | INET_SKMEMINFO | UNIX_MEMINFO },
		{ false, 0, 1, "wmem_queued",     TYPE_INT,    print_wmem_queued,    ALL | INET_SKMEMINFO | UNIX_MEMINFO },
		{ false, 0, 1, "optmem",          TYPE_INT,    print_optmem,         ALL | INET_SKMEMINFO | UNIX_MEMINFO },
		{ false, 0, 1, "backlog",         TYPE_INT,    print_backlog,        ALL | INET_SKMEMINFO | UNIX_MEMINFO },
		{ false, 0, 1, "drops",           TYPE_INT,    print_drops,          ALL | INET_SKMEMINFO | UNIX_MEMINFO },
		{ false, 0, 1, "shutdown",        TYPE_INT,    print_shutdown,       ALL | INET_SHUTDOWN | UNIX_SHUTDOWN },
		{ false, 0, 1, "vfs_dev",         TYPE_INT,    print_vfs_dev,        UNIX | UNIX_VFS },
		{ false, 0, 1, "vfs_ino",         TYPE_INT,    print_vfs_ino,        UNIX | UNIX_VFS },
		{ false, 0, 1, "timer",           TYPE_STRING, print_timer,          INET },
		{ false, 0, 1, "retransmits",     TYPE_INT,    print_retransmits,    INET },
		{ false, 0, 1, "expires_ms",      TYPE_INT,    print_expires_ms,     INET },
		{ false, 0, 1, "rmem",            TYPE_INT,    print_rmem,           INET | INET_MEMINFO },
		{ false, 0, 1, "wmem",            TYPE_INT,    print_wmem,           INET | INET_MEMINFO },
		{ false, 0, 1, "fmem",            TYPE_INT,    print_fmem,           INET | INET_MEMINFO },
		{ false, 0, 1, "tmem",            TYPE_INT,    print_tmem,           INET | INET_MEMINFO },
		{ false, 0, 1, "ipv6only",        TYPE_INT,    print_ipv6only,       INET6 },
		{ false, 0, 1, "user_protocol",   TYPE_STRING, print_user_protocol,  RAW },
		{ false, 0, 2, "cookie0",         TYPE_INT,    print_cookie0,        ALL },
		{ false, 0, 2, "cookie1",         TYPE_INT,    print_cookie1,        ALL },
		{ false, 0, 2, "icons",           TYPE_INT_ARR,print_icons,          UNIX | UNIX_ICONS },
		{ false, 0, 2, "cong",            TYPE_STRING, print_cong,           INET | INET_CONG },
		{ false, 0, 2, "tos",             TYPE_INT,    print_tos,            INET | INET_TOS },
		{ false, 0, 2, "tclass",          TYPE_INT,    print_tclass,         INET | INET_TCLASS },
		{ false, 0, 2, "locals",          TYPE_STRING, print_locals,         INET },
		{ false, 0, 2, "peers",           TYPE_STRING, print_peers,          INET },
		{ false, 0, 2, "pad",             TYPE_INT,    print_pad,            INET },
		{ false, 0, 2, "mark",            TYPE_INT,    print_mark,           INET },
		{ false, 0, 2, "class_id",        TYPE_INT,    print_class_id,       INET | INET_TCLASS },
		{ false, 0, 2, "md5sig",          TYPE_STRING, print_md5sig,         INET },
		{ false, 0, 2, "vegas_enabled",   TYPE_INT,    print_vegas_enabled,  TCP | INET_VEGASINFO },
		{ false, 0, 2, "vegas_rttcnt",    TYPE_INT,    print_vegas_rttcnt,   TCP | INET_VEGASINFO },
		{ false, 0, 2, "vegas_rtt",       TYPE_INT,    print_vegas_rtt,      TCP | INET_VEGASINFO },
		{ false, 0, 2, "vegas_minrtt",    TYPE_INT,    print_vegas_minrtt,   TCP | INET_VEGASINFO },
		{ false, 0, 2, "dctcp_enabled",   TYPE_INT,    print_dctcp_enabled,  TCP | INET_DCTCPINFO },
		{ false, 0, 2, "dctcp_ce_state",  TYPE_INT,    print_dctcp_ce_state, TCP | INET_DCTCPINFO },
		{ false, 0, 2, "dctcp_alpha",     TYPE_INT,    print_dctcp_alpha,    TCP | INET_DCTCPINFO },
		{ false, 0, 2, "dctcp_ab_ecn",    TYPE_INT,    print_dctcp_ab_ecn,   TCP | INET_DCTCPINFO },
		{ false, 0, 2, "dctcp_ab_tot",    TYPE_INT,    print_dctcp_ab_tot,   TCP | INET_DCTCPINFO },
		{ false, 0, 2, "bbr_bw_lo",       TYPE_INT,    print_bbr_bw_lo,      TCP | INET_BBRINFO },
		{ false, 0, 2, "bbr_bw_hi",       TYPE_INT,    print_bbr_bw_hi,      TCP | INET_BBRINFO },
		{ false, 0, 2, "bbr_min_rtt",     TYPE_INT,    print_bbr_min_rtt,    TCP | INET_BBRINFO },
		{ false, 0, 2, "bbr_pacing_gain", TYPE_INT,    print_bbr_pacing_gain,TCP | INET_BBRINFO },
		{ false, 0, 2, "bbr_cwnd_gain",   TYPE_INT,    print_bbr_cwnd_gain,  TCP | INET_BBRINFO },
		{ false, 0, 3, "info",            TYPE_STRING, print_info,           INET | INET_INFO },
	};

	size_t ncolumns = ARRAY_SIZE(columns);
	size_t level = 0;

	while ((opt = getopt_long(argc, argv, "c:lMN:prSsTtUuwx46", opts,
			NULL)) != -1) {
		switch (opt) {
			case 'c':
				cols = xstrdup_nofail(optarg);
				break;
			case 'l':
				level++;
				for (size_t i = 0; i < ncolumns; ++i)
					if (columns[i].level <= level)
						if (!(columns[i].data & REQUIRES_RESOLVING))
							columns[i].vis = true;
				break;
			case 'M':
				merge = true;
				if (!table)
					table = xstrdup_nofail("socket");
				break;
			case 'N':
				table = xstrdup_nofail(optarg);
				break;
			case 'p':
				fprintf(stderr,
					"SCTP not supported yet (patches welcomed)\n");
				exit(2);
				/* protocols |= SCTP; */
				break;
			case 'r':
				for (size_t i = 0; i < ncolumns; ++i)
					if (columns[i].data & REQUIRES_RESOLVING)
						columns[i].vis = true;
				break;
			case 's':
				show_flags |= SHOW_SIMPLE;
				break;
			case 'S':
				show_flags |= SHOW_FULL;
				break;
			case 'T':
				table = xstrdup_nofail("socket");
				break;
			case 't':
				protocols |= TCP;
				break;
			case 'U':
				fprintf(stderr,
					"UDPLITE not supported yet (patches welcomed)\n");
				exit(2);
				/* protocols |= UDPLITE; */
				break;
			case 'u':
				protocols |= UDP;
				break;
			case 'w':
				protocols |= RAW;
				break;
			case 'V':
				printf("git\n");
				return 0;
			case 'x':
				protocols |= UNIX;
				break;
			case '4':
				protocols |= INET4;
				break;
			case '6':
				protocols |= INET6;
				break;
			case 'h':
			default:
				usage(stdout);
				return 2;
		}
	}

	if (!protocols)
		protocols = ALL;

	if (cols) {
		csvci_parse_cols_nofail(cols, columns, &ncolumns);

		free(cols);
	} else {
		for (size_t i = 0; i < ncolumns; ++i)
			columns[i].vis = columns[i].vis &&
				(columns[i].data & protocols);

		csvci_set_columns_order(columns, &ncolumns);
	}

	csv_show(show_flags);

	struct csvmu_ctx ctx;
	ctx.table = table;
	ctx.merge = merge;

	csvmu_print_header(&ctx, columns, ncolumns);

	struct mnl_socket *nl;

	nl = mnl_socket_open(NETLINK_INET_DIAG);
	if (nl == NULL) {
		perror("mnl_socket_open");
		exit(2);
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		perror("mnl_socket_bind");
		exit(2);
	}

	static const struct {
		unsigned flags;
		uint8_t family;
		uint8_t protocol;
	} protos[] = {
			{ INET4_TCP, AF_INET,  IPPROTO_TCP },
			{ INET6_TCP, AF_INET6, IPPROTO_TCP },

			{ INET4_UDP, AF_INET,  IPPROTO_UDP },
			{ INET6_UDP, AF_INET6, IPPROTO_UDP },

			{ INET4_RAW, AF_INET,  IPPROTO_RAW },
			{ INET6_RAW, AF_INET6, IPPROTO_RAW },

			/* not tested */
#if 0
			{ INET4_DCCP, AF_INET,  IPPROTO_DCCP },
			{ INET6_DCCP, AF_INET6, IPPROTO_DCCP },

			{ INET4_SCTP, AF_INET,  IPPROTO_SCTP },
			{ INET6_SCTP, AF_INET6, IPPROTO_SCTP },

			{ INET4_UDPLITE, AF_INET,  IPPROTO_UDPLITE },
			{ INET6_UDPLITE, AF_INET6, IPPROTO_UDPLITE },
#endif
			{ UNIX, AF_UNIX, 0 },
	};

	for (size_t i = 0; i < ARRAY_SIZE(protos); ++i) {
		if (protocols & protos[i].flags) {
			dump(nl, protos[i].family, protos[i].protocol, columns,
					ncolumns, &ctx);
		}
	}

	free(ctx.table);

	mnl_socket_close(nl);

	return 0;
}
