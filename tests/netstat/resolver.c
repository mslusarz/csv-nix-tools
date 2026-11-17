/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2026, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

/* Keep netstat's lookups local and independent of the host's NSS data. */
struct hostent *
gethostbyaddr(const void *address, socklen_t length, int family)
{
	static char name[] = "fixture-loopback";
	static struct hostent result = { .h_name = name };
	const struct in_addr loopback = { .s_addr = htonl(INADDR_LOOPBACK) };

	if (family == AF_INET && length == sizeof(loopback) &&
			memcmp(address, &loopback, sizeof(loopback)) == 0)
		return &result;

	return NULL;
}

struct servent *
getservbyport(int port, const char *protocol)
{
	static char name[] = "fixture-service";
	static struct servent result = { .s_name = name };
	const char *listener_port = getenv("CSV_NETSTAT_TEST_LISTENER_PORT");

	if (listener_port && strcmp(protocol, "tcp") == 0 &&
			ntohs((in_port_t)port) == strtoul(listener_port, NULL, 10))
		return &result;

	return NULL;
}
