# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2026, Marcin Ślusarz <marcin.slusarz@gmail.com>

import csv
import os
from contextlib import ExitStack
from io import StringIO
from pathlib import Path
import socket
import subprocess
import sys
import tempfile


def netstat(*args, input_csv="", env=None):
    result = subprocess.run(
        [binary, *args],
        input=input_csv,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        env=env,
        universal_newlines=True,
        timeout=10,
    )
    assert result.returncode == 0, f"{args}: exit {result.returncode}\n{result.stderr}"
    assert not result.stderr, result.stderr
    reader = csv.DictReader(StringIO(result.stdout), strict=True)
    return reader.fieldnames, list(reader)


def inode(sock):
    return str(os.fstat(sock.fileno()).st_ino)


def check_fields(row, **expected):
    actual = {field: row[field] for field in expected}
    assert actual == expected, f"expected {expected}, got {actual}"


def check_pending(row, *clients):
    check_fields(row, **{"pending_conns:int": str(len(clients))})
    # The kernel sends 32-bit inode numbers, which icons prints as signed.
    pending = [int(value) & 0xFFFFFFFF for value in row["icons:int[]"].split(",")]
    assert sorted(pending) == sorted(int(inode(client)) for client in clients), row


def check_merge(listener_inode, path, table, *options):
    headers, entries = netstat(
        "--unix", *options, "--merge", "--columns", "inode,name",
        input_csv="_table,note\nexisting,keep me\n",
    )
    inode_column = f"{table}.inode:int"
    name_column = f"{table}.name"
    assert headers == ["_table", "note", inode_column, name_column], headers
    check_fields(entries[0], _table="existing", note="keep me", **{inode_column: "", name_column: ""})
    merged_listener, = (row for row in entries if row[inode_column] == listener_inode)
    check_fields(merged_listener, _table=table, note="", **{name_column: str(path)})


def test_unix():
    # A short directory keeps the name within the sockaddr_un path limit.
    with tempfile.TemporaryDirectory(prefix="csv-netstat-", dir="/tmp") as directory, \
            ExitStack() as stack:
        def open_unix(kind=socket.SOCK_STREAM):
            return stack.enter_context(socket.socket(socket.AF_UNIX, kind))

        path = Path(directory) / 'comma,quote"and\nnewline'
        listener = open_unix()
        listener.bind(str(path))
        listener.listen(4)
        listener.settimeout(5)
        listener_inode = inode(listener)

        abstract = open_unix(socket.SOCK_DGRAM)
        abstract_name = Path(directory).name
        abstract.bind("\0" + abstract_name)

        columns = (
            "inode,name,type,state,vfs_dev,vfs_ino,peer_inode,icons,"
            "pending_conns,backlog_length,incoming_data,shutdown"
        )

        def rows():
            _, entries = netstat("--unix", "--columns", columns)
            return {row["inode:int"]: row for row in entries}

        first = open_unix()
        first.connect(str(path))
        entries = rows()
        check_fields(
            entries[listener_inode],
            name=str(path),
            type="STREAM",
            state="LISTEN",
            **{
                "vfs_ino:int": str(path.stat().st_ino & 0xFFFFFFFF),
                "backlog_length:int": "4",
            },
        )
        assert int(entries[listener_inode]["vfs_dev:int"], 16) > 0
        check_pending(entries[listener_inode], first)
        check_fields(entries[inode(abstract)], name="@" + abstract_name, type="DGRAM")

        second = open_unix()
        second.connect(str(path))
        check_pending(rows()[listener_inode], first, second)

        accepted = stack.enter_context(listener.accept()[0])
        first.sendall(b"ready")
        first.shutdown(socket.SHUT_WR)
        entries = rows()
        check_fields(
            entries[inode(accepted)],
            state="ESTABLISHED",
            **{"peer_inode:int": inode(first), "incoming_data:int": "5"},
        )
        check_fields(entries[inode(first)], **{"peer_inode:int": inode(accepted), "shutdown:int": "2"})

        check_merge(listener_inode, path, "socket")
        check_merge(listener_inode, path, "socket", "--as-table")
        check_merge(listener_inode, path, "fixture", "--table-name", "fixture")


def test_resolve(resolver):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as listener, \
            socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client:
        listener.bind(("127.0.0.1", 0))
        listener.listen(1)
        listener.settimeout(5)
        client.bind(("127.0.0.2", 0))
        client.settimeout(5)
        client.connect(listener.getsockname())
        accepted, _ = listener.accept()
        with accepted:
            listener_port = str(listener.getsockname()[1])
            client_port = str(client.getsockname()[1])
            env = dict(os.environ, CSV_NETSTAT_TEST_LISTENER_PORT=listener_port)
            env["LD_PRELOAD"] = " ".join(filter(None, [resolver, env.get("LD_PRELOAD")]))
            _, entries = netstat("--tcp", "--resolve", env=env)
            rows = {row["inode:int"]: row for row in entries}
            check_fields(
                rows[inode(client)],
                family="INET4", protocol="tcp", state="ESTABLISHED",
                src_ip="127.0.0.2", src_name="127.0.0.2", src_port_name=client_port,
                dst_ip="127.0.0.1", dst_name="fixture-loopback", dst_port_name="fixture-service",
                **{"src_port:int": client_port, "dst_port:int": listener_port},
            )
            check_fields(
                rows[inode(accepted)],
                src_name="fixture-loopback", src_port_name="fixture-service",
                dst_name="127.0.0.2", dst_port_name=client_port,
            )


case, binary, resolver = sys.argv[1:]
if case == "unix":
    test_unix()
elif case == "resolve":
    test_resolve(resolver)
else:
    sys.exit(f"unknown test case: {case}")
