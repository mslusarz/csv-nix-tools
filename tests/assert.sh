#!/bin/bash

# Copyright 2019, Sebastian Pidek <sebastian.pidek@gmail.com>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#    * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#    * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in
#       the documentation and/or other materials provided with the
#       distribution.
#
#    * Neither the name of the copyright holder nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
correct_stdout=${1}
correct_stderr=${2}
candidate_stdout="${3}.stdout.candidate"
candidate_stderr="${3}.stderr.candidate"
show_detailed_differences=$4

diff -q "$correct_stdout" "$candidate_stdout" >/dev/null && diff -q "$correct_stderr" "$candidate_stderr" >/dev/null && echo "PASS" || echo "FAIL" && [[ ! -z $show_detailed_differences ]] && 
{
  diff -q "$correct_stdout" "$candidate_stdout";
  diff --suppress-common-lines "$correct_stdout" "$candidate_stdout"
} &&
{
  diff -q "$correct_stderr" "$candidate_stderr";
  diff --suppress-common-lines "$correct_stderr" "$candidate_stderr"
}
