# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2026, Marcin Ślusarz <marcin.slusarz@gmail.com>

import csv
from io import StringIO
import os
from pathlib import Path
import socket
import subprocess
import sys
import tempfile


def listing(*args, cwd=None, status=0, stderr=""):
    result = subprocess.run(
        [binary, *args], cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        universal_newlines=True, timeout=10,
    )
    assert result.returncode == status, f"{args}: exit {result.returncode}\n{result.stderr}"
    assert result.stderr == stderr, result.stderr
    return list(csv.DictReader(StringIO(result.stdout), strict=True))


def test_special_files():
    # A short path fits within the UNIX domain socket address limit.
    with tempfile.TemporaryDirectory(prefix="csv-ls-", dir="/tmp") as directory:
        path = Path(directory) / "server.sock"
        with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as server:
            server.bind(str(path))
            path.chmod(0o600)
            socket_row, device_row = listing(
                "--colors", "-c", "name,type_name,type_mode,rdev,name_color",
                path.name, "/dev/null", cwd=directory,
            )
            assert socket_row == {
                "name": path.name, "type_name": "sock", "type_mode": "srw------- ",
                "rdev:int": "0", "name_color": "fg=magenta",
            }, socket_row
            assert device_row["name"] == "/dev/null", device_row
            assert device_row["type_name"] == "chr", device_row
            assert device_row["type_mode"].startswith("c"), device_row
            assert device_row["rdev:int"] == hex(os.stat("/dev/null").st_rdev), device_row
            assert device_row["name_color"] == "fg=yellow", device_row


def test_proc_symlinks():
    # procfs reports a size of zero for exe even though readlink returns a path.
    rows = listing("-c", "name,symlink,symlink_info", "/proc/self/", "/proc/self/exe")
    for name in ("exe", "/proc/self/exe"):
        row, = (row for row in rows if row["name"] == name)
        assert row["symlink"] == os.path.realpath(binary), row
        assert row["symlink_info:int"] == "0", row


def test_unsorted():
    with tempfile.TemporaryDirectory(prefix="csv-ls-") as directory:
        for name in ("z", "B", "a", ".hidden"):
            (Path(directory) / name).touch()
        with os.scandir(directory) as entries:
            directory_order = [entry.name for entry in entries if not entry.name.startswith(".")]
        rows = listing("-U", cwd=directory)
        assert [row["name"] for row in rows] == directory_order, rows
        rows = listing(cwd=directory)
        assert [row["name"] for row in rows] == ["a", "B", "z"], rows


def test_directory_permissions():
    with tempfile.TemporaryDirectory(prefix="csv-ls-") as directory:
        tree = Path(directory) / "tree"
        blocked = tree / "blocked"
        blocked.mkdir(parents=True)
        (tree / "open").mkdir()
        (tree / "open" / "child").touch()
        blocked.chmod(0)
        try:
            if os.access(blocked, os.R_OK):
                sys.exit(77)
            rows = listing(
                "-R", "-c", "full_path", "tree", cwd=directory, status=1,
                stderr="listing directory 'tree/blocked' failed: Permission denied\n",
            )
            assert [row["full_path"] for row in rows] == [
                "tree/blocked", "tree/open", "tree/open/child",
            ], rows
        finally:
            blocked.chmod(0o700)

case, binary = sys.argv[1:]
{
    "special_files": test_special_files,
    "proc_symlinks": test_proc_symlinks,
    "unsorted": test_unsorted,
    "directory_permissions": test_directory_permissions,
}[case]()
