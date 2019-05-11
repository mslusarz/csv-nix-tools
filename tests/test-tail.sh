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

source common.sh

show_detailed_differences=$1

TEST_CASE "empty input"
cat empty.txt | ../build/csv-tail 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh empty.csv error-no-typenames.txt "${test_name}" $show_detailed_differences

TEST_CASE "simple pass-through"
cat one-column-one-row.csv | ../build/csv-tail 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh one-column-one-row.csv empty.txt "${test_name}" $show_detailed_differences

TEST_CASE "--lines=0 case"
cat one-column-one-row.csv | ../build/csv-tail --lines=0 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh zero-lines.csv empty.txt "${test_name}" $show_detailed_differences

TEST_CASE "-n 0 case"
cat one-column-one-row.csv | ../build/csv-tail -n 0 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh zero-lines.csv empty.txt "${test_name}" $show_detailed_differences

TEST_CASE "--lines=1 case"
cat one-column-one-row.csv | ../build/csv-tail --lines=1 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh one-column-one-row.csv empty.txt "${test_name}" $show_detailed_differences

TEST_CASE "-n 1 case for csv-tail"
cat one-column-one-row.csv | ../build/csv-tail -n 1 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh one-column-one-row.csv empty.txt "${test_name}" $show_detailed_differences

TEST_CASE "--lines=2 but there's only 1 line in input"
cat one-column-one-row.csv | ../build/csv-tail --lines=2 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh one-column-one-row.csv empty.txt "${test_name}" $show_detailed_differences

TEST_CASE "-n 2 but there's only 1 line in input"
cat one-column-one-row.csv | ../build/csv-tail -n 2 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh one-column-one-row.csv empty.txt "${test_name}" $show_detailed_differences

TEST_CASE "--lines=2 but there's 3 lines in input"
cat 3-columns-3-rows.csv | ../build/csv-tail --lines=2 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh 3-columns-2-last-rows.csv empty.txt "${test_name}" $show_detailed_differences

TEST_CASE "-n 2 but there's 3 lines in input"
cat 3-columns-3-rows.csv | ../build/csv-tail -n 2 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh 3-columns-2-last-rows.csv empty.txt "${test_name}" $show_detailed_differences

TEST_CASE "--lines=3 and there's 3 lines in input"
cat 3-columns-3-rows.csv | ../build/csv-tail --lines=3 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh 3-columns-3-rows.csv empty.txt "${test_name}" $show_detailed_differences

TEST_CASE "-n 3 and there's 3 lines in input"
cat 3-columns-3-rows.csv | ../build/csv-tail -n 3 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh 3-columns-3-rows.csv empty.txt "${test_name}" $show_detailed_differences
